#!/usr/bin/env python3
"""Mine reusable C recipes from verified matches (strategy §4).

`match_families.py` already RETRIEVES examples whose C resembles a target's.
This tool goes one step further: it parses two or more independently verified
exact bodies, anti-unifies them into a parameterised template with typed holes,
and instantiates that template for a new target.

The parts that decide whether it is useful rather than merely clever:

  * **Only verified examples.** A recipe mined from a body that is not byte-
    exact teaches the allocator nothing. Examples are re-probed now; a stale
    probe is never reused.
  * **Holes are typed and must bind.** A difference between two examples
    becomes a hole with a role (constant, field offset, stride, symbol, or
    callee). A hole with no ROM evidence for the target stays `unbound`, and
    an unbound hole blocks instantiation rather than defaulting to zero.
  * **Parse, do not rewrite.** The supported C89 subset is tokenised and
    parsed. Anything outside it -- macros, structs, function pointers, goto,
    switch -- is REJECTED with a reason. Running a regex over arbitrary C
    would silently corrupt exactly the constructs nobody checked.

Signed and unsigned recipes never merge: `s16` and `u16` differ in the
returned value above 0x7FFF, so a template that treats them as interchangeable
produces a confidently wrong candidate.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass, field, asdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import corpus_match_probe as probe
import experiment_kit as kit

VERSION = 1
STRATEGY = "recipe_miner"

#: Supported C89 subset. Kept as data so the self-test can assert that every
#: type the parser claims to handle is one it actually handles.
TYPES = {
    "u8": (8, False), "u16": (16, False), "u32": (32, False),
    "s8": (8, True), "s16": (16, True), "s32": (32, True),
    "unsigned char": (8, False), "unsigned short": (16, False), "unsigned int": (32, False),
    "signed char": (8, True), "short": (16, True), "int": (32, True),
    "char": (8, False), "long": (32, False),
}

CALLABLE = ("void", "u8", "u16", "u32", "s8", "s16", "s32")
#: Source spelling -> the `experiment_kit.Contract` vocabulary, which accepts
#: only `u8/s8/u16/s16/u32/s32/void`. A recipe that faithfully kept what the
#: source said (`int`) was therefore refused with "unknown return type
#: ('int', 32, True)" -- the tuple because `Function.return_type` is
#: `(name, width, signed)`, the name because `int` is `s32` here. Rendering
#: keeps the source spelling; only the contract is canonicalised.
CONTRACT_TYPES = {
    "u8": "u8", "u16": "u16", "u32": "u32",
    "s8": "s8", "s16": "s16", "s32": "s32",
    "unsigned char": "u8", "unsigned short": "u16", "unsigned int": "u32",
    "signed char": "s8", "short": "s16", "int": "s32",
    "char": "u8", "long": "u32", "void": "void",
}


def contract_return_type(recipe: "Recipe") -> str:
    """The contract spelling of a recipe's return type, or an explicit refusal."""
    raw = recipe.return_type
    name = raw[0] if isinstance(raw, tuple) else raw
    if name not in CONTRACT_TYPES:
        raise kit.Unsupported(f"recipe return type {name!r} has no contract spelling")
    return CONTRACT_TYPES[name]


def source_return_type(recipe: "Recipe") -> str:
    """The spelling to RENDER in emitted C: the source's own when it is one."""
    raw = recipe.return_type
    name = raw[0] if isinstance(raw, tuple) else raw
    return name if name in TYPES else contract_return_type(recipe)


TYPEDEFS = "\n".join([
    "typedef unsigned char u8;",
    "typedef unsigned short u16;",
    "typedef unsigned int u32;",
    "typedef signed char s8;",
    "typedef signed short s16;",
    "typedef signed int s32;",
])
#: `"\n".join(<a single string>)` joins that string's CHARACTERS, so every
#: emitted instantiation began with one letter per line and could never
#: compile. A list literal is what this always meant.

#: Tokens the subset deliberately refuses. Encountering one is a rejection
#: with a name, never a silent mangling.
UNSUPPORTED = {
    "goto": "goto", "switch": "switch", "case": "switch", "default": "switch",
    "do": "loop", "while": "loop", "for": "loop", "continue": "loop", "break": "loop",
    "sizeof": "sizeof", "static": "storage class", "extern": "storage class",
    "register": "storage class", "volatile": "qualifier", "const": "qualifier",
    "struct": "aggregate", "union": "aggregate", "enum": "aggregate",
    "typedef": "declaration", "#": "preprocessor",
}


class ParseError(Exception):
    """The source is outside the supported subset; never rewritten."""

    def __init__(self, reason: str, detail: str = ""):
        super().__init__(f"{reason}{': ' + detail if detail else ''}")
        self.reason = reason
        self.detail = detail


# --------------------------------------------------------------------------
# Tokenizer
# --------------------------------------------------------------------------

@dataclass(frozen=True)
class Token:
    kind: str
    text: str
    pos: int


def tokenize(text: str) -> list[Token]:
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    out: list[Token] = []
    pos = 0
    while pos < len(text):
        ch = text[pos]
        if ch in " \t\r\n":
            pos += 1
            continue
        if ch == "#":
            # Preprocessor directives are not part of the supported subset and
            # are never parsed, but they MUST be skipped rather than rejected:
            # `extract_function` tokenizes the whole source file, and every
            # `src/*.c` here opens with `#include`, whose `#` is not in the
            # token pattern, so every real body in the repository was refused
            # with "unparsable character: '#'" -- while the inline self-test
            # snippets, having no includes, passed. Skipping the line is the
            # only safe reading: parsing a directive is what would corrupt the
            # body.
            end_of_line = text.find("\n", pos)
            pos = len(text) if end_of_line < 0 else end_of_line + 1
            continue
        # Kept ahead of the single-character operator class on purpose. Without
        # it `p++` tokenises as `p` `+` `+` and `a - -b` as a subtraction of a
        # negation: both then parse as valid expressions of a different
        # meaning, which is the silent mis-parse this tool exists to avoid.
        if (text.startswith("->", pos) or text.startswith("++", pos)
                or text.startswith("--", pos)):
            raise ParseError("unsupported operator", text[pos:pos + 2])
        match = re.match(r"[A-Za-z_]\w*|0[xX][0-9a-fA-F]+[UuLl]*|\d+[UuLl]*|&&|\|\||<<|>>|"
                         r"==|!=|<=|>=|[+\-*/%&|^~!<>=(){};,\[\].]", text[pos:])
        if not match:
            raise ParseError("unparsable character", repr(ch))
        token = match.group()
        if token in UNSUPPORTED:
            raise ParseError(UNSUPPORTED[token], token)
        # `token[0].isalpha() or token == "_"` classifies only a BARE underscore
        # as a name: `"_".isalpha()` is False, and `token == "_"` is False for
        # `_0802B65C`. So every VMA-shaped identifier in this repository was
        # typed `op`, and `parse_function` rejected it with "expected function
        # name" while naming the very token it had just read -- 30 of the
        # corpus's exact bodies, i.e. every body whose own name is VMA-shaped.
        kind = ("name" if (token[0].isalpha() or token[0] == "_")
                else "num" if token[0].isdigit() else "op")
        out.append(Token(kind, token, pos))
        pos += len(token)
    out.append(Token("eof", "", pos))
    return out


# --------------------------------------------------------------------------
# AST
# --------------------------------------------------------------------------

@dataclass
class Node:
    kind: str
    text: str = ""
    children: list["Node"] = field(default_factory=list)
    type_name: str = ""
    signed: bool = False
    width: int = 32


@dataclass
class Stmt:
    kind: str
    target: str = ""
    expr: Node | None = None
    width: int = 32
    signed: bool = False
    type_name: str = ""


@dataclass
class Function:
    name: str
    return_type: str
    params: list[str]
    #: Declared type per parameter, parallel to `params`. The parser used to
    #: discard the type, leaving an instantiation rendering `candidate(a, b)`
    #: with no types -- C that cannot compile, from a recipe the tool had just
    #: declared mined.
    param_types: list[str]
    body: list[Stmt]


# --------------------------------------------------------------------------
# Parser
# --------------------------------------------------------------------------

_BINARY_PRECEDENCE = {"*": 3, "/": 3, "%": 3, "+": 2, "-": 2,
                      "<<": 1, ">>": 1, "<": 1, ">": 1,
                      "<=": 1, ">=": 1, "==": 1, "!=": 1,
                      "&": 0, "^": 0, "|": 0}


class Parser:
    """Recursive-descent parser for the documented subset."""

    def __init__(self, tokens: list[Token]):
        self.tokens = tokens
        self.pos = 0

    def peek(self) -> Token:
        return self.tokens[self.pos]

    def take(self) -> Token:
        token = self.tokens[self.pos]
        self.pos += 1
        return token

    def expect(self, text: str) -> Token:
        token = self.take()
        if token.text != text:
            raise ParseError("expected token", f"{text!r}, found {token.text!r}")
        return token

    def accept(self, text: str) -> bool:
        if self.peek().text == text:
            self.pos += 1
            return True
        return False

    def _accept_void_params(self) -> bool:
        """Consume a `(void)` empty parameter list, reporting whether it did."""
        if self.peek().text == "void" and self.tokens[self.pos + 1].text == ")":
            self.take()
            self.expect(")")
            return True
        return False


    def parse_function(self, name: str) -> Function:
        return_type = self._type()
        actual = self.take()
        if actual.kind != "name":
            raise ParseError("expected function name", actual.text)
        # Identity, not just kind. The span cutter can resolve a name to a
        # DIFFERENT function -- a `sub_`/`_` twin, or an alias target -- and with
        # no check here that body parsed silently and was then mined as though
        # it were the requested target. The template would be built from real C
        # that is simply the wrong function, and nothing downstream could tell,
        # because every later stage keys on the name the CALLER asked for rather
        # than on the one that was actually parsed.
        if name and actual.text != name:
            raise ParseError(f"expected function {name}", actual.text)
        self.expect("(")
        params: list[str] = []
        # `(void)` is C89's spelling for "no parameters" and it is pervasive in
        # this repository. Without this, `_type()` consumes `void` as a
        # parameter type and `take()` then records `)` as the parameter NAME,
        # so the following `expect(")")` fails against `{`. That is how every
        # `int f(void) { ... }` body was rejected with "')', found '{'" -- it
        # looks like a span bug and is not; the span is correct.
        param_types: list[str] = []
        if not self.accept(")") and not self._accept_void_params():
            while True:
                param_types.append(self._type()[0])
                params.append(self.take().text)
                if self.accept(","):
                    continue
                self.expect(")")
                break
        return Function(actual.text, return_type, params, param_types,
                        self.parse_body())

    def _type(self) -> tuple[str, int, bool]:
        token = self.peek()
        if token.text in TYPES:
            self.take()
            return token.text, *TYPES[token.text]
        if token.text in CALLABLE:
            self.take()
            return token.text, 32, False
        if token.text == "unsigned" and self.tokens[self.pos + 1].text in ("char", "short", "int"):
            self.take()
            second = self.take()
            return f"unsigned {second.text}", *TYPES[f"unsigned {second.text}"]
        if token.text == "signed" and self.tokens[self.pos + 1].text in ("char", "short", "int"):
            self.take()
            second = self.take()
            return f"signed {second.text}", *TYPES[f"signed {second.text}"]
        raise ParseError("unsupported type", token.text)

    def parse_body(self) -> list[Stmt]:
        self.expect("{")
        body: list[Stmt] = []
        while not self.accept("}"):
            body.append(self.parse_stmt())
        return body

    def parse_stmt(self) -> Stmt:
        if self.peek().text == "return":
            self.take()
            if self.accept(";"):
                return Stmt("return", expr=None)
            expr = self.expr()
            self.expect(";")
            return Stmt("return", expr=expr, width=expr.width, signed=expr.signed,
                        type_name=expr.type_name)
        if self.accept("*"):
            # The store form is `*(TYPE *)(addr) = value;` -- the `(` comes
            # before the type. Parsing the type first reported "unsupported
            # type: (" for every store in the subset.
            self.expect("(")
            type_name, width, signed = self._type()
            self.expect("*")
            self.expect(")")
            address = self.expr()
            self.expect("=")
            self.expr()
            self.expect(";")
            return Stmt("store", expr=Node("deref", children=[address]), width=width,
                        signed=signed, type_name=type_name)
        # A bare assignment to an already-declared local carries no type. Real
        # bodies write `base = 0x030015F0u + idx;` after `u32 base;`, and
        # requiring a type here rejects most of them as "unsupported type"
        # rather than as what they are.
        if self.peek().kind == "name" and self.tokens[self.pos + 1].text == "=":
            name = self.take().text
            self.take()
            expr = self.expr()
            self.expect(";")
            return Stmt("assign", target=name, expr=expr, width=expr.width,
                        signed=expr.signed, type_name="")
        type_name, width, signed = self._type()
        name = self.take()
        if name.kind != "name":
            raise ParseError("expected declarator", name.text)
        if self.accept(";"):
            return Stmt("declare", target=name.text, width=width, signed=signed,
                        type_name=type_name)
        self.expect("=")
        expr = self.expr()
        self.expect(";")
        return Stmt("assign", target=name.text, expr=expr, width=width, signed=signed,
                    type_name=type_name)

    def expr(self, min_precedence: int = 0) -> Node:
        left = self.unary()
        while True:
            token = self.peek()
            precedence = _BINARY_PRECEDENCE.get(token.text)
            if precedence is None or precedence < min_precedence:
                return left
            self.take()
            right = self.expr(precedence + 1)
            width, signed = max(left.width, right.width), left.signed or right.signed
            left = Node("binary", token.text, [left, right], "", signed, width)

    def unary(self) -> Node:
        token = self.peek()
        if token.text in ("~", "!", "-", "+"):
            self.take()
            operand = self.unary()
            return Node("unary", token.text, [operand], "", operand.signed, operand.width)
        return self.primary()

    def primary(self) -> Node:
        token = self.take()
        if token.kind == "num":
            # C89: an integer constant with no suffix has type `int`, which is
            # SIGNED; only a `u`/`U` suffix makes it unsigned. Omitting this
            # stamped every plain literal `signed=False`, which propagated into
            # the hole record and into anti-unification's signedness comparison
            # -- so the fix belongs here, not in the renderer.
            signed = not re.search(r"[uU]", token.text)
            return Node("literal", token.text, width=32, signed=signed)
        if token.kind == "name":
            following = self.peek().text
            if following == "(":
                self.take()
                args = []
                if not self.accept(")"):
                    while True:
                        args.append(self.expr())
                        if self.accept(","):
                            continue
                        self.expect(")")
                        break
                return Node("call", token.text, args, width=32)
            return Node("name", token.text, width=32)
        if token.text == "(":
            following = self.tokens[self.pos]
            if following.text in TYPES or following.text in CALLABLE:
                type_name, width, signed = self._type()
                self.expect(")")
                operand = self.unary()
                return Node("cast", type_name, [operand], type_name, signed, width)
            inner = self.expr()
            self.expect(")")
            return inner
        if token.text == "*":
            self.expect("(")
            type_name, width, signed = self._type()
            self.expect("*")
            self.expect(")")
            address = self.unary()
            return Node("deref", children=[address], type_name=type_name, signed=signed,
                        width=width)
        raise ParseError("unexpected token", token.text)


def validate_symbols(function: Function) -> None:
    """Reject a body that references an identifier it never declares.

    Without a symbol table the parser happily accepts `base = TILE_BASE + idx`,
    which is the macro spelling -- and then mines a template full of holes
    named after macro constants that no ROM evidence can ever bind. Rejecting
    an undeclared name is the honest outcome: it is a construct outside the
    subset, not a value the miner failed to understand.
    """
    declared = set(function.params)
    for stmt in function.body:
        if stmt.target:
            declared.add(stmt.target)

    def walk(node: Node) -> None:
        if node.kind == "name" and node.text not in declared:
            raise ParseError("undeclared identifier", node.text)
        for child in node.children:
            walk(child)

    for stmt in function.body:
        if stmt.expr is not None:
            walk(stmt.expr)


def parse_source(source: str) -> Function:
    parser = Parser(tokenize(source))
    function = parser.parse_function("")
    if parser.peek().kind != "eof":
        raise ParseError("trailing text after the function",
                         parser.peek().text)
    validate_symbols(function)
    return function


def _definition_span(source: str, name: str) -> tuple[int, int] | None:
    """The span of one real DEFINITION of `name`, or None when there is none.

    One body can appear in a file several times: an
    `__attribute__((alias("...")))` stub, a forward declaration, and the
    definition. Taking the FIRST textual occurrence picked the alias stub,
    whose `"` the tokenizer refuses -- 107 of the corpus's exact bodies failed
    with "unparsable character" for exactly that reason. A candidate counts as
    a definition only when its parameter list is followed by `{`, not `;`.
    """
    for match in re.finditer(r"\b" + re.escape(name) + r"\s*\(", source):
        index = source.index("(", match.end() - 1)
        depth = 0
        closed = -1
        while index < len(source):
            if source[index] == "(":
                depth += 1
            elif source[index] == ")":
                depth -= 1
                if depth == 0:
                    closed = index
                    break
            index += 1
        if closed < 0:
            continue
        if not source[closed + 1:].lstrip().startswith("{"):
            continue                     # a declaration or an alias stub
        opening = source.index("{", closed)
        level = 0
        for position in range(opening, len(source)):
            if source[position] == "{":
                level += 1
            elif source[position] == "}":
                level -= 1
                if level == 0:
                    start = match.start()
                    # Walk back over the return type (identifiers and `*`), so
                    # a wrapped declaration is not cut mid-type.
                    while start > 0 and (source[start - 1].isalnum()
                                         or source[start - 1] in " *_"):
                        start -= 1
                    return start, position + 1
    return None


def extract_function(source: str, name: str) -> Function:
    """Parse exactly one definition out of a whole source file.

    Only the definition's own span is tokenised. The previous slice ran from
    the signature to END OF FILE, so on any multi-function file the parser
    consumed the first function and the trailing-text check rejected it --
    extraction had never worked on a real file, and only passed because every
    self-test fed a snippet where the function WAS the whole string. Restricting
    the slice also keeps file-scope `extern` declarations, which the tokenizer
    refuses by design, from rejecting an otherwise parseable body.
    """
    span = _definition_span(source, name)
    if span is None:
        raise ParseError("function not found in source", name)
    parser = Parser(tokenize(source[span[0]:span[1]]))
    function = parser.parse_function(name)
    if parser.peek().kind != "eof":
        raise ParseError("trailing text after the function", parser.peek().text)
    validate_symbols(function)
    return function


# --------------------------------------------------------------------------
# Anti-unification
# --------------------------------------------------------------------------

@dataclass
class Hole:
    name: str
    role: str
    width: int
    signed: bool
    #: Literal text seen in each example, retained as provenance.
    observed: list[str] = field(default_factory=list)
    bound: int | None = None

    def to_json(self) -> dict:
        return asdict(self)


@dataclass
class Recipe:
    name: str
    return_type: str
    params: list[str]
    template: list[dict]
    holes: dict[str, Hole]
    examples: list[dict] = field(default_factory=list)
    failure_cases: list[str] = field(default_factory=list)
    #: Declared type per parameter, parallel to `params`. Defaulted last so the
    #: positional construction in `bind()` stays valid.
    param_types: list[str] = field(default_factory=list)

    @property
    def unbound(self) -> list[str]:
        return sorted(h for h, hole in self.holes.items() if hole.bound is None)

    def to_json(self) -> dict:
        return {
            "name": self.name, "return_type": self.return_type, "params": self.params,
            "template": self.template,
            "holes": {k: v.to_json() for k, v in self.holes.items()},
            "examples": self.examples,
            "failure_cases": self.failure_cases,
            "unbound_holes": self.unbound,
        }


def node_key(node: Node) -> tuple:
    return (node.kind, node.text, node.type_name, node.signed,
            tuple(node_key(child) for child in node.children))


def anti_unify(nodes: list[Node], holes: dict[str, Hole], counter: list[int]) -> Node:
    """Structural anti-unification: like shapes merge, differing leaves become holes.

    Signedness is part of the shape key, never a hole. Two examples that


    disagree about `s16` versus `u16` do NOT merge -- they are different
    recipes, because the value above 0x7FFF differs.
    """
    first = nodes[0]
    for other in nodes[1:]:
        if (other.kind != first.kind or other.signed != first.signed
                or other.width != first.width
                or (first.kind not in ("literal", "name", "call") and other.text != first.text)):
            raise ValueError(f"structure differs at {first.kind}/{other.kind}")
    if first.kind == "literal":
        values = {node.text for node in nodes}
        if len(values) == 1:
            # `signed` must be carried through. The loop above has already
            # established that every node in this group agrees on it, so this
            # copy is the second literal-construction site and it was dropping
            # the flag. The four-byte pilot never reached it -- its two
            # examples had DIFFERENT constants (`1` vs `97`), so it took the
            # hole path below, which does pass `first.signed`.
            return Node("literal", first.text, width=first.width,
                        signed=first.signed)
        name = f"k{counter[0]}"
        counter[0] += 1
        holes[name] = Hole(name, "constant", first.width, first.signed,
                           observed=[node.text for node in nodes])
        return Node("hole", name, width=first.width, signed=first.signed)
    if first.kind == "name":
        names = {node.text for node in nodes}
        if len(names) == 1:
            return Node("name", first.text, width=first.width)
        name = f"v{counter[0]}"
        counter[0] += 1
        holes[name] = Hole(name, "local", first.width, first.signed,
                           observed=[node.text for node in nodes])
        return Node("hole", name, width=first.width, signed=first.signed)
    if first.kind == "call":
        names = {node.text for node in nodes}
        text = first.text if len(names) == 1 else None
        if text is None:
            name = f"f{counter[0]}"
            counter[0] += 1
            holes[name] = Hole(name, "callee", 32, False,
                               observed=sorted(names))
            text = name
        merged = [anti_unify([node.children[i] for node in nodes],
                             holes, counter)
                  for i in range(len(first.children))] if first.children else []
        return Node("call", text, merged, width=first.width)
    merged = [anti_unify([node.children[i] for node in nodes], holes, counter)
              for i in range(len(first.children))]
    return Node(first.kind, first.text, merged, first.type_name, first.signed, first.width)


def stmt_key(stmt: Stmt) -> tuple:
    return (stmt.kind, stmt.target, stmt.type_name, stmt.signed,
            node_key(stmt.expr) if stmt.expr else None)


def anti_unify_functions(examples: list[Function], name: str) -> Recipe:
    """Anti-unify two or more parsed bodies into one parameterised recipe."""
    if len(examples) < 2:
        raise ValueError("anti-unification needs at least two examples")
    returns = {example.return_type for example in examples}
    if len(returns) != 1:
        raise ValueError(f"examples disagree about the return type: {sorted(returns)}")
    params = [tuple(example.params) for example in examples]
    if len(set(params)) != 1:
        raise ValueError(f"examples disagree about parameters: {params}")
    lengths = {len(example.body) for example in examples}
    if len(lengths) != 1:
        raise ValueError("examples have different statement counts; "
                         "this miner only unifies structurally identical bodies")
    holes: dict[str, Hole] = {}
    counter = [0]
    template: list[dict] = []
    for index in range(len(examples[0].body)):
        statements = [example.body[index] for example in examples]
        keys = {stmt_key(stmt) for stmt in statements}
        kinds = {stmt.kind for stmt in statements}
        if len(kinds) != 1:
            raise ValueError(f"statement {index} differs in kind: {sorted(kinds)}")
        stmt = statements[0]
        entry = {"kind": stmt.kind, "target": stmt.target, "width": stmt.width,
                 "signed": stmt.signed, "type_name": stmt.type_name}
        if stmt.expr is not None:
            if len(keys) != 1:
                entry["expr"] = asdict(anti_unify([s.expr for s in statements], holes, counter))
            else:
                entry["expr"] = asdict(stmt.expr)
        template.append(entry)
    return Recipe(name=name, return_type=examples[0].return_type,
                  params=list(examples[0].params), template=template, holes=holes,
                  param_types=list(examples[0].param_types))


# --------------------------------------------------------------------------
# Instantiation
# --------------------------------------------------------------------------

def render_node(node: dict, holes: dict[str, Hole], *, names: dict[str, str]) -> str:
    kind = node["kind"]
    if kind == "literal":
        return node["text"]
    if kind == "name":
        return names.get(node["text"], node["text"])
    if kind == "hole":
        hole = holes.get(node["text"])
        if hole is None or hole.bound is None:
            raise kit.Unsupported(f"hole {node['text']!r} is unbound; instantiate refused")
        literal = f"{hole.bound:#x}" if hole.width <= 16 else f"{hole.bound:#010x}"
        suffix = "u" if not hole.signed else ""
        return f"({literal}{suffix})"
    if kind == "call":
        args = ", ".join(render_node(child, holes, names=names) for child in node["children"])
        return f"{names.get(node['text'], node['text'])}({args})"
    if kind == "cast":
        inner = render_node(node["children"][0], holes, names=names)
        return f"({node['type_name']}){inner}"
    if kind == "deref":
        inner = render_node(node["children"][0], holes, names=names)
        return f"*({node['type_name']} *)(u32)({inner})"
    if kind == "binary":
        left = render_node(node["children"][0], holes, names=names)
        right = render_node(node["children"][1], holes, names=names)
        return f"({left} {node['text']} {right})"
    if kind == "unary":
        inner = render_node(node["children"][0], holes, names=names)
        return f"({node['text']}{inner})"
    raise kit.Unsupported(f"cannot render node kind {kind!r}")


def instantiate(recipe: Recipe, names: dict[str, str] | None = None) -> str:
    """Render a bound recipe as compilable C89."""
    if recipe.unbound:
        raise kit.Unsupported(
            f"recipe has unbound holes {recipe.unbound}; refusing to default them")
    names = names or {}
    # The holes as recorded. A render-time signedness override used to live here,
    # which emitted correct C while `recipe.json` still claimed the observed
    # constant was unsigned -- a second source of truth contradicting the record.
    # Signedness now comes from the literal's suffix where the node is built.
    holes = recipe.holes
    # Every parameter, typed, with C89's `void` for none. This rendered only
    # `params[0]` and an empty string otherwise, so a two-parameter body emitted
    # `candidate()` -- a function the recipe had just mined, rendered as
    # something that cannot compile.
    if recipe.params:
        declared = ", ".join(f"{t} {n}" for t, n in
                             zip(recipe.param_types or ["u32"] * len(recipe.params),
                                 recipe.params))
    else:
        declared = "void"
    lines = [TYPEDEFS, "",
             f"{source_return_type(recipe)} candidate({declared})", "{"]
    names = {name: name for name in recipe.params} | names
    for entry in recipe.template:
        if entry["kind"] == "declare":
            lines.append(f"    {entry['type_name']} {entry['target']};")
            continue
        if entry["kind"] == "store":
            node = entry["expr"]
            value = render_node(node["children"][0], holes, names=names)
            type_name = node.get("type_name") or entry["type_name"]
            lines.append(f"    *({type_name} *)(u32)({value}) = "
                         f"({type_name}){render_node(node['children'][1] if len(node['children']) > 1 else node, holes, names=names)};")
            continue
        text = render_node(entry["expr"], holes, names=names)
        if entry["kind"] == "return":
            lines.append(f"    return {text};" if text else "    return;")
        else:
            lines.append(f"    {entry['target']} = {text};")
    lines.append("}")
    return "\n".join(lines) + "\n"


def bind(recipe: Recipe, binding: dict[str, int]) -> Recipe:
    """Return an independent copy with the named holes bound.

    The `Hole` objects must be copied, not shared. A shallow `dict()` copy
    leaves both recipes pointing at the same holes, so binding one instance
    silently fills the other's unbound holes too -- which turns "this recipe
    still needs a value" into "this recipe instantiates", for a target the
    binding was never checked against.
    """
    holes = {name: Hole(hole.name, hole.role, hole.width, hole.signed,
                        list(hole.observed), hole.bound)
             for name, hole in recipe.holes.items()}
    copy = Recipe(recipe.name, recipe.return_type, list(recipe.params),
                  [dict(entry) for entry in recipe.template], holes,
                  list(recipe.examples), list(recipe.failure_cases))
    for name, value in binding.items():
        if name not in copy.holes:
            raise ValueError(f"no hole named {name!r}")
        copy.holes[name].bound = value
    return copy


# --------------------------------------------------------------------------
# Driver
# --------------------------------------------------------------------------

def verified_examples(names: list[str]) -> list[dict]:
    """Re-probe each named body NOW. A stale exact is not evidence."""
    accepted = []
    for name in names:
        record = subprocess_probe(name)
        if record is not None:
            accepted.append(record)
    return accepted


def subprocess_probe(name: str) -> dict | None:
    """Fresh scoped EXACT probe for one body, or None when it does not hold."""
    import subprocess
    import tempfile
    with tempfile.TemporaryDirectory(prefix="recipe-miner-") as tmp:
        report = Path(tmp) / "report.json"
        result = subprocess.run(
            [sys.executable, str(probe.ROOT / "tools/corpus_match_probe.py"),
             "--function", name, "--c89", "--require-all", "--require-exact",
             "--work-dir", tmp, "--json", str(report)],
            text=True, capture_output=True)
        if result.returncode or not report.exists():
            return None
        results = json.loads(report.read_text())["results"]
        exact = [r for r in results if r["status"] == "EXACT"
                 and r["name"] == name and int(r["vma"], 16) == probe.parse_vma_name(name)]
        return exact[0] if len(exact) == 1 else None


def _bail(writer: "kit.EvidenceWriter", report: dict, reason: str):
    """Finish the run as UNSUPPORTED_CONTRACT and return the reason to main().

    Every refusal path used to `writer.finish({... "reason": r ...})` and then
    `return None, report`, handing back the INNER probe report, which has no
    `reason` key. main() prints `report.get('reason')`, so the user saw
    `UNSUPPORTED_CONTRACT: None` and was told nothing about why the run
    refused. The reason now travels on the record main() prints.
    """
    # The per-example verdicts go in as scalars so `summary.md` renders them;
    # without them the summary says only "fewer than two examples parsed" and
    # never which example failed, or at which stage.
    # Split by stage, because "not byte-exact" and "this tool cannot parse it"
    # are very different facts, and merging them printed a name in both the
    # verified and rejected lists at once.
    record = {"draft_status": kit.UNSUPPORTED_CONTRACT, "reason": reason,
              "results": [], "report": report,
              "examples usable": ", ".join(report.get("usable") or []) or "(none)",
              "failed fresh EXACT probe": ", ".join(
                  report.get("failed_probe") or []) or "(none)",
              "outside supported subset": "; ".join(
                  report.get("outside_subset") or []) or "(none)"}
    writer.finish(record)
    return None, {**report, "reason": reason}


def mine(names: list[str], out: Path) -> tuple[Recipe | None, dict]:
    writer = kit.run_writer(STRATEGY, out, extra={"tool_version": VERSION})
    writer.open()
    writer.write_json("run.json", {"strategy": STRATEGY, "tool_version": VERSION,
                                   "examples": names})
    # Built up front so every refusal path can say WHICH examples were rejected,
    # not merely that the run gave up.
    # Three buckets, not two. "Passed the fresh EXACT probe" and "this tool can
    # parse it" are different facts, and a single `rejected` list appended to
    # from both stages printed a name in `examples verified` AND
    # `examples rejected` at once -- which reads as a self-contradiction in the
    # one artifact an operator reads.
    report: dict = {"examples": names, "usable": [], "failed_probe": [],
                    "outside_subset": []}
    if len(names) < 2:
        return _bail(writer, report, f"needs at least two examples, got {len(names)}")
    records = verified_examples(names)
    for name in names:
        if not any(r["name"] == name for r in records):
            report["failed_probe"].append(name)
    if len(records) < 2:
        return _bail(writer, report, f"only {len(records)} example(s) verified now")

    parsed = []
    for record in records:
        text = (probe.ROOT / record["source"]).read_text(errors="replace")
        try:
            parsed.append(extract_function(text, record.get("alias_of") or record["name"]))
            report["usable"].append(record["name"])
        except ParseError as exc:
            report["outside_subset"].append(f"{record['name']}: {exc.reason}")
    if len(parsed) < 2:
        return _bail(writer, report,
                     "fewer than two examples parsed inside the supported subset")
    try:
        recipe = anti_unify_functions(parsed, "mined")
    except ValueError as exc:
        return _bail(writer, report, str(exc))
    recipe.examples = [{"name": r["name"], "source": r["source"], "vma": r["vma"],
                        "rom_bytes": r["rom_bytes"]} for r in records]
    writer.write_json("recipe.json", recipe.to_json())
    return recipe, report


def apply_to_target(recipe: Recipe, target: str, out: Path, budget: int,
                    binding: dict[str, int]) -> tuple[kit.Score | None, dict]:
    """Instantiate the recipe for a target and score it against the ROM."""
    span = probe.rom_functions()
    vma = probe.parse_vma_name(target)
    if vma is None or vma not in span:
        raise kit.Unsupported(f"no independently known ROM span for {target}")
    contract = kit.Contract(name="candidate", vma=vma, end=span[vma],
                            return_type=contract_return_type(recipe),
                            memory="ordinary")
    contract.require_supported()
    bound = bind(recipe, binding)
    source = instantiate(bound)
    (out / "instantiation.c").write_text(source)
    meter = kit.BudgetMeter(kit.Budget(compiles=budget, seconds=600.0))
    adapter = kit.CandidateAdapter(out / "probe", meter=meter)
    return adapter.evaluate(source, "candidate", contract), {"source": str(out / "instantiation.c")}


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--target", required=True, help="body to instantiate for")
    ap.add_argument("--example", action="append", default=[],
                    help="verified body to mine; repeatable, needs >= 2")
    ap.add_argument("--bind", action="append", default=[],
                    help="HOLE=VALUE, repeatable; unbound holes refuse instantiation")
    ap.add_argument("--out", type=Path)
    ap.add_argument("--budget", type=int, default=16)
    ap.add_argument("--json", type=Path)
    ap.add_argument("--self-test", action="store_true")
    # Checked before parsing: the self-test needs no ROM, no toolchain and no
    # target, so it must not be forced to supply `--target` and two examples.
    if "--self-test" in sys.argv:
        return self_test()
    args = ap.parse_args()
    out = args.out or probe.ROOT / "build/experiments/recipe-miner" / args.target
    out.mkdir(parents=True, exist_ok=True)
    try:
        recipe, report = mine(args.example, out)
        if recipe is None:
            print(f"recipe_miner: UNSUPPORTED_CONTRACT: {report.get('reason')}", file=sys.stderr)
            return 2
        binding = {}
        for item in args.bind:
            name, _, value = item.partition("=")
            binding[name] = int(value, 0)
        score, artifacts = apply_to_target(recipe, args.target, out, args.budget, binding)
        print(f"recipe {recipe.name}: {len(recipe.holes)} hole(s), "
              f"{len(recipe.unbound)} unbound; score {score.status if score else 'none'}")
        # The success path never called `finish()`, so a run that actually mined
        # and scored wrote no summary.md and left run.json holding only the
        # early stub. A pilot whose evidence directory has no summary is not a
        # result you can cite, and the refusal paths -- the ones that proved
        # nothing -- were the only ones writing one.
        kit.run_writer(STRATEGY, out, extra={"tool_version": VERSION}).finish({
            "draft_status": score.status if score else kit.TOOL_FAILURE,
            "recipe": recipe.name,
            "target": args.target,
            "holes": len(recipe.holes),
            "matched_bytes": score.matched_bytes if score else 0,
            "rom_bytes": score.rom_bytes if score else 0,
            "instantiation": artifacts.get("source", ""),
            "examples usable": ", ".join(report.get("usable") or []) or "(none)",
            "examples failed probe": ", ".join(report.get("failed_probe") or []) or "(none)",
            "outside supported subset": "; ".join(report.get("outside_subset") or []) or "(none)",
        })
        if args.json:
            args.json.parent.mkdir(parents=True, exist_ok=True)
            args.json.write_text(json.dumps({
                "recipe": recipe.to_json(), "report": report,
                "score": score.to_json() if score else None, "artifacts": artifacts,
            }, indent=2) + "\n")
        return 0
    except kit.Unsupported as exc:
        print(f"recipe_miner: UNSUPPORTED_CONTRACT: {exc}", file=sys.stderr)
        return 2
    except kit.DependencyMissing as exc:
        print(f"recipe_miner: DEPENDENCY_MISSING: {exc}", file=sys.stderr)
        return 2


# --------------------------------------------------------------------------
# Self-test
# --------------------------------------------------------------------------

A = """u32 helper_a(u32 idx)
{
    u32 base;
    base = 0x030015F0u + idx;
    return *(u16 *)((u32)base + 2);
}
"""

B = """u32 helper_b(u32 idx)
{
    u32 base;
    base = 0x03002000u + idx;
    return *(u16 *)((u32)base + 6);
}
"""

C_SAME_AS_A = A.replace("helper_a", "helper_c")

MACRO_BODY = """u32 helper_m(u32 idx)
{
    base = TILE_BASE + idx;
    return *(u16 *)((u32)base + OFF);
}
"""


def self_test() -> int:
    print("running recipe_miner self-test...")
    passed = total = 0

    def check(label: str, condition: bool) -> None:
        nonlocal passed, total
        total += 1
        if condition:
            passed += 1
            print(f"  [PASS] {label}")
        else:
            print(f"  [FAIL] {label}", file=sys.stderr)

    # --- the subset is really the subset --------------------------------
    parsed_a = parse_source(A)
    # `params` holds declarator NAMES; the declared type is not retained,
    # because the recipe renders a fresh signature from the examples' own.
    check("a supported body parses", parsed_a.params == ["idx"]
          and [s.kind for s in parsed_a.body] == ["declare", "assign", "return"])
    check("goto is REJECTED, not mangled",
          kit.raises(ParseError, lambda: parse_source(
              "u32 f(u32 a) { " + A.split("{", 1)[1].replace("return", "goto x;"))))
    check("a macro body is REJECTED, not mangled",
          kit.raises(ParseError, lambda: parse_source(MACRO_BODY)))
    check("volatile is REJECTED (qualifier outside the subset)",
          kit.raises(ParseError, lambda: parse_source(
              A.replace("u32 base;", "volatile u32 base;"))))
    check("a for loop is REJECTED",
          kit.raises(ParseError, lambda: parse_source(
              "u32 f(u32 a) { for (a = 0; a < 2; a++) a = 1; return a; }")))
    check("a structure declaration is REJECTED",
          kit.raises(ParseError, lambda: tokenize("struct rec { int a; }; int f(void);")))
    # A preprocessor line is SKIPPED, not rejected: every `src/*.c` here opens
    # with `#include`, and `extract_function` tokenizes the whole file, so
    # rejecting on `#` refused every real body in the repository while these
    # include-free snippets passed.
    check("a preprocessor directive is skipped, not parsed",
          [t.text for t in tokenize("#include <gba/types.h>\nu32 f(u32 a) { return a; }")
           if t.kind == "name"][:2] == ["u32", "f"])
    # `++`, `--` and `->` must be refused BEFORE the single-character operator
    # class sees them, and the guard is ADJACENCY-ONLY: `p++` is one token and
    # is refused, while `a - -a` tokenises as `a` `-` `-` `a` exactly as it
    # always has. Whether a spaced double minus is accepted is the parser's
    # question, not the tokenizer's, so it is deliberately not asserted here.
    check("an adjacent increment operator is REJECTED, not split into two ops",
          kit.raises(ParseError, lambda: tokenize("u32 f(void){ u32 p; p++; return p; }"))
          and kit.raises(ParseError, lambda: tokenize("u32 f(u32 p){ p--; return p; }"))
          and kit.raises(ParseError, lambda: tokenize("u32 f(void){ return 0; } ->"))
          )

    # --- anti-unification -----------------------------------------------
    parsed_b = parse_source(B)
    same = parse_source(C_SAME_AS_A)
    recipe = anti_unify_functions([parsed_a, parsed_b], "mined")
    check("differing constants become holes", len(recipe.holes) >= 2)
    check("identical bodies produce NO holes",
          len(anti_unify_functions([parsed_a, same], "same").holes) == 0)
    check("holes carry the values they abstracted over",
          any(len(hole.observed) >= 2 for hole in recipe.holes.values()))
    # A bare unsuffixed integer constant is `int` in C89, i.e. SIGNED. Both
    # literal-construction sites must carry that: with `signed` dropped they
    # both defaulted to unsigned, `5` and `5u` compared EQUAL, and a signed and
    # an unsigned literal merged silently -- the confusion this tool refuses to
    # merge recipes over. The copy site is the one the four-byte pilot never
    # reached, because its two examples had DIFFERENT constants.
    same_recipe = anti_unify_functions([parse_source("int g(void){ return 5; }"),
                                        parse_source("int g(void){ return 5; }")],
                                       "same-literal")
    check("a copied literal keeps its signedness",
          same_recipe.template[0]["expr"]["kind"] == "literal"
          and same_recipe.template[0]["expr"]["signed"] is True)
    check("a signed and an unsigned literal are NOT merged",
          kit.raises(ValueError, lambda: anti_unify_functions(
              [parse_source("int g(void){ return 5; }"),
               parse_source("int g(void){ return 5u; }")], "signedness")))
    check("a single example cannot be anti-unified",
          kit.raises(ValueError, lambda: anti_unify_functions([parsed_a], "one")))
    check("structurally different bodies are refused",
          kit.raises(ValueError, lambda: anti_unify_functions(
              [parsed_a, parse_source("u32 f(u32 a) { return a; }")], "diff")))
    check("examples that disagree on the return type are refused",
          kit.raises(ValueError, lambda: anti_unify_functions(
              [parsed_a, parse_source("s32 f(u32 a) { return a; }")], "ret")))

    # --- signedness must never merge ------------------------------------
    signed = A.replace("u16", "s16")
    try:
        merged = anti_unify_functions([parsed_a, parse_source(signed)], "signedness")
        merged_ok = True
    except ValueError:
        merged_ok = False
    check("u16 and s16 examples do NOT merge into one recipe", not merged_ok)

    # --- unbound holes block instantiation -------------------------------
    check("an unbound hole refuses instantiation",
          kit.raises(kit.Unsupported, lambda: instantiate(recipe)))
    bound = bind(recipe, {name: 4 for name, hole in recipe.holes.items()})
    text = instantiate(bound)
    check("a fully bound recipe instantiates", "candidate(" in text and "return" in text)
    check("binding an unknown hole is refused",
          kit.raises(ValueError, lambda: bind(recipe, {"nope": 1})))
    check("a partially bound recipe still refuses",
          kit.raises(kit.Unsupported, lambda: instantiate(
              bind(recipe, {name: 1 for name in list(recipe.holes)[:1]}))))

    # --- instantiation is honest about types -----------------------------
    check("instantiation casts through the recorded type",
          "(u16 *)" in text)
    check("the recipe records where it came from",
          recipe.examples == [] and recipe.name == "mined")

    print(f"{passed}/{total} passed")
    return 0 if passed == total else 1


if __name__ == "__main__":
    raise SystemExit(main())
