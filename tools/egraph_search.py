#!/usr/bin/env python3
"""egraph_search.py -- justified-rewrite expression search.

Explores *equivalent expressions* rather than enumerating source spellings,
because agbcc lowers different equivalent forms differently and one of them
may be byte-exact for a target span.

This is deliberately NOT a full e-graph. It is a small, bounded rewrite-DAG
enumerator with canonical deduplication in pure Python: the question the plan
asks is whether the rewrite vocabulary yields useful *compiler* variation, and
that question does not need congruence closure to be answered. The label
"e-graph" is kept only in the strategy name; this docstring says what it is.

Scope, and why it is narrow:

  * Pure integer expressions with explicit width and signedness on every node.
    No memory, no calls, no hardware access. A seeded expression naming any of
    those is REFUSED (`kit.Unsupported`), never partially modelled.
  * Only *justified* rewrites. Every rewrite carries a side condition, the
    side condition is CHECKED, and an unsatisfied side condition BLOCKS the
    rewrite and is recorded as blocked. Mathematical integer identities do not
    automatically hold for C: `a - b == a + (-b)` is modular-arithmetic true and
    signed-C-UB true, and the difference between those two facts is the single
    most important correctness property in this file.
  * An external egg/egglog executable is optional. `--egglog PATH` records its
    version and, when it cannot be executed, raises `kit.DependencyMissing`. It
    never silently changes the algorithm.

Candidates are scored with `kit.CandidateAdapter.evaluate`/`score` against the
target's real ROM span. Deduplication is by COMPILED OUTPUT -- emitted bytes
plus `kit.instruction_signature` -- so two representatives that compile
identically consume one search slot.

This is a candidate generator for synthesis. It runs after a correct lift exists
but its expression form mismatches. It never edits src/, never edits
tools/matching_slice_functions.json, never edits asm/, and `EXACT_DRAFT` means
scratch bytes equal the ROM span -- integration and the independent link are
separate and are not performed here.

Usage:
    python3 tools/egraph_search.py NAME --return-type u32 [--seeded-expression FILE]
                              [--out DIR] [--max-nodes N] [--max-iters N]
                              [--max-extract N] [--egglog PATH] [--self-test]
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import experiment_kit as kit  # noqa: E402
import corpus_match_probe as probe  # noqa: E402

STRATEGY = "egraph-search"
ONE_LINE = ("egraph_search.py: bounded rewrite-DAG enumerator (not a full e-graph); every "
            "rewrite carries a checked side condition; dedup by compiled output.")

#: Type table: width in bits, signedness. Names are the project's C89 types so a
#: scratch body reads like src/.
TYPES: dict[str, tuple[int, bool]] = {
    "u8": (8, False), "u16": (16, False), "u32": (32, False),
    "s8": (8, True), "s16": (16, True), "s32": (32, True),
}

#: Name bound by a named-temporary wrapper. A local, never a parameter: the ABI
#: registers belong to the real leaf parameters only.
TMP_NAME = "t0"

#: Constructs outside the model. Naming one is a refusal, not a guess.
REFUSED_OPS = {
    "load": "memory read has no ordering/effect contract here",
    "store": "memory write has no ordering/effect contract here",
    "volatile": "volatile access: hardware visibility is unmodelled",
    "call": "call: no callee-ABI or clobber contract is modelled",
    "hw": "hardware register access is unmodelled",
    "asm": "inline asm is unmodelled",
}

BINARY_OPS = {"+", "-", "*", "&", "|", "^", "<<", ">>"}
UNARY_OPS = {"neg", "not"}
CAST_OPS = {"zext", "sext", "trunc", "as"}
COMMUTATIVE = {"+", "*", "&", "|", "^"}

# Node shapes (plain tuples: hashable, so canonical dedup is a dict key):
#   ("leaf", "var"|"const", type, name|value)
#   ("bin", op, type, lhs, rhs)
#   ("un",  op, type, arg)
#   ("cast", op, type, arg)
#   ("tmp", None, type, value, body)   named-temporary wrapper (extraction only)


# ---------------------------------------------------------------------------
# Side conditions and small helpers
# ---------------------------------------------------------------------------

@dataclass(frozen=True)
class Side:
    """A rewrite and the condition under which it is a *C* identity.

    `condition` is the human-readable justification recorded verbatim into the
    evidence. The rule function checks it mechanically; a rule whose condition
    does not hold emits nothing and records a `blocked` entry instead.
    """
    rule: str
    condition: str


def ctype(t: tuple[int, bool]) -> str:
    for name, spec in TYPES.items():
        if spec == t:
            return name
    return f"{'s' if t[1] else 'u'}{t[0]}"


def mask_of(t: tuple[int, bool]) -> int:
    return (1 << t[0]) - 1


def fits(value: int, t: tuple[int, bool]) -> bool:
    if not t[1]:
        return 0 <= value <= mask_of(t)
    lo, hi = -(1 << (t[0] - 1)), (1 << (t[0] - 1)) - 1
    return lo <= value <= hi


def wrap(value: int, t: tuple[int, bool]) -> int:
    m = mask_of(t)
    value &= m
    if t[1] and value >> (t[0] - 1):
        value -= m + 1
    return value


def etype(e):
    return e[2]


# ---------------------------------------------------------------------------
# Expression constructors / accessors
# ---------------------------------------------------------------------------

def var(name: str, t) -> tuple:
    return ("leaf", "var", t, name)


def const(value: int, t) -> tuple:
    return ("leaf", "const", t, wrap(value, t))


def binop(op: str, t, lhs, rhs) -> tuple:
    return ("bin", op, t, lhs, rhs)


def unop(op: str, t, arg) -> tuple:
    return ("un", op, t, arg)


def cast(op: str, t, arg) -> tuple:
    return ("cast", op, t, arg)


def tmp(value, body) -> tuple:
    """Named-temporary wrapper: bind `value` to `TMP_NAME`, then evaluate `body`.

    The name appears in `body`'s reference, not in the wrapper, so `to_c` needs
    no special case and the declaration is emitted from `value`.
    """
    return ("tmp", None, etype(value), value, body)


def children(e):
    if e[0] == "bin":
        return (e[3], e[4])
    if e[0] in ("un", "cast"):
        return (e[3],)
    if e[0] == "tmp":
        return (e[3], e[4])
    return ()


def walk(e):
    """Every node of e, including e, parents before children."""
    yield e
    for child in children(e):
        yield from walk(child)


def subterms(e):
    """Every distinct node of e, including e itself."""
    seen, out = set(), []
    for n in walk(e):
        if n not in seen:
            seen.add(n)
            out.append(n)
    return out


def size(e) -> int:
    return sum(1 for _ in walk(e))


def shape_key(e) -> tuple:
    """Structural key ignoring operand order -- extraction diversity accounting
    only, never correctness."""
    if e[0] == "leaf":
        return ("leaf", e[1], e[2])
    if e[0] == "bin":
        a, b = sorted((shape_key(e[3]), shape_key(e[4])), key=repr)
        return ("bin", e[1], e[2], a, b)
    if e[0] in ("un", "cast"):
        return (e[0], e[1], e[2], shape_key(e[3]))
    return ("tmp", shape_key(e[3]))


def const_value(e):
    if e[0] == "leaf" and e[1] == "const":
        return e[3]
    return None


def to_c(e) -> str:
    """Emit C for one node. Widths and signedness are explicit in the source so
    no promotion is left to the reader or the compiler."""
    if e[0] == "tmp":
        return to_c(e[4])
    if e[0] == "leaf":
        if e[1] == "var":
            return e[3]
        return f"{e[3]}u" if not e[2][1] else f"({e[3]})"
    name, t = ctype(e[2]), e[2]
    if e[0] == "bin":
        if e[1] in ("<<", ">>"):
            rhs = const_value(e[4])
            if rhs is None:
                raise kit.Unsupported(f"variable shift amount in {e!r}: a shift count >= width "
                                      "is undefined in C and this tool will not emit one")
            if not (0 <= rhs < t[0]):
                raise kit.Unsupported(f"shift amount {rhs} out of range for width {t[0]}")
            return f"({to_c(e[3])} {e[1]} {rhs})"
        return f"({to_c(e[3])} {e[1]} {to_c(e[4])})"
    if e[0] == "un":
        return f"({name})(-{to_c(e[3])})" if e[1] == "neg" else f"(~{to_c(e[3])})"
    return f"(({name}){to_c(e[3])})"


def variables(e) -> list[str]:
    out = []
    for n in walk(e):
        if n[0] == "leaf" and n[1] == "var" and n[3] not in out:
            out.append(n[3])
    return out


def eval_const(e):
    """Fold a fully constant expression, or return None."""
    v = const_value(e)
    if v is not None:
        return v
    if e[0] == "bin":
        a, b = eval_const(e[3]), eval_const(e[4])
        if a is None or b is None:
            return None
        op = e[1]
        try:
            if op == "+":
                return wrap(a + b, e[2])
            if op == "-":
                return wrap(a - b, e[2])
            if op == "*":
                return wrap(a * b, e[2])
            if op == "&":
                return wrap(a & b, e[2])
            if op == "|":
                return wrap(a | b, e[2])
            if op == "^":
                return wrap(a ^ b, e[2])
            if op == "<<":
                return wrap(a << b, e[2])
            if op == ">>":
                return wrap(a >> b, e[2])
        except (ValueError, OverflowError):
            return None
        return None
    if e[0] == "un":
        a = eval_const(e[3])
        return None if a is None else wrap(-a if e[1] == "neg" else ~a, e[2])
    if e[0] == "cast":
        a = eval_const(e[3])
        return None if a is None else wrap(a, e[2])
    return None


def exact_binary(op, a, b) -> int:
    va, vb = eval_const(a), eval_const(b)
    return {"+": lambda: va + vb, "-": lambda: va - vb, "*": lambda: va * vb,
            "&": lambda: va & vb, "|": lambda: va | vb, "^": lambda: va ^ vb,
            "<<": lambda: va << vb, ">>": lambda: va >> vb}[op]()


# ---------------------------------------------------------------------------
# Seeded-expression parser
# ---------------------------------------------------------------------------

_TOKEN = re.compile(r"[()]|[^\s()]+")
_IDENT = re.compile(r"[A-Za-z_]\w*\Z")


def parse_seeded(text: str) -> list[tuple]:
    """Parse the one-expression-per-line DSL. Refuses anything outside the model.

        (u32 + (u32 * a (u32 3)) (u32 & b (u32 255)))

    The type token is the declared result width AND signedness, and every node
    carries one. A bare `load`, `store`, `call`, `volatile`, `hw` or `asm`
    operator is refused by name: a memory model this experiment does not have
    cannot be approximated into existence.
    """
    exprs = []
    for lineno, raw in enumerate(text.splitlines(), 1):
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        toks = _TOKEN.findall(line)
        if not toks:
            continue
        try:
            exprs.append(_parse_tokens(toks, lineno)[0])
        except kit.Unsupported:
            raise
        except (ValueError, IndexError) as exc:
            raise kit.Unsupported(f"seed line {lineno}: {exc}") from None
    if not exprs:
        raise kit.Unsupported("seeded expression file contained no expression")
    return exprs


def _parse_type(tok: str) -> tuple[int, bool]:
    if tok not in TYPES:
        raise ValueError(f"unknown type {tok!r}; known: {', '.join(sorted(TYPES))}")
    return TYPES[tok]


def _parse_tokens(toks, lineno: int, pos: int = 0):
    if pos >= len(toks) or toks[pos] != "(":
        raise ValueError("expected '('")
    pos += 1
    if pos >= len(toks):
        raise ValueError("truncated expression")
    t = _parse_type(toks[pos])
    pos += 1
    if pos >= len(toks):
        raise ValueError("truncated expression: missing operator")
    op = toks[pos]
    pos += 1
    if op in REFUSED_OPS:
        raise kit.Unsupported(
            f"seed line {lineno}: {op!r} is outside this model's scope -- {REFUSED_OPS[op]}. "
            "Memory, calls and hardware access are not modelled and there is no fallback "
            "algorithm to substitute.")
    if op in BINARY_OPS:
        lhs, pos = _parse_tokens(toks, lineno, pos)
        rhs, pos = _parse_tokens(toks, lineno, pos)
        if pos >= len(toks) or toks[pos] != ")":
            raise ValueError(f"trailing tokens after binary {op}")
        _well_typed_bin(op, t, lhs, rhs)
        return binop(op, t, lhs, rhs), pos + 1
    if op in UNARY_OPS:
        arg, pos = _parse_tokens(toks, lineno, pos)
        if pos >= len(toks) or toks[pos] != ")":
            raise ValueError(f"trailing tokens after unary {op}")
        return unop(op, t, arg), pos + 1
    if op in CAST_OPS:
        arg, pos = _parse_tokens(toks, lineno, pos)
        if pos >= len(toks) or toks[pos] != ")":
            raise ValueError(f"trailing tokens after {op}")
        _well_typed_cast(op, t, arg)
        return cast(op, t, arg), pos + 1
    if pos >= len(toks) or toks[pos] != ")":
        raise ValueError(f"trailing tokens after leaf {op!r}")
    leaf = var(op, t) if _IDENT.match(op) else const(int(op, 0), t)
    return leaf, pos + 1


def _well_typed_bin(op, t, lhs, rhs):
    """Fail closed on a seeded expression whose declared types are incoherent.

    C's integer promotions are deliberately NOT modelled: a rewrite over
    expressions with implicit conversions is a rewrite over different values.
    Requiring every node to declare its own type removes the ambiguity instead
    of guessing at it.
    """
    lt, rt = etype(lhs), etype(rhs)
    if op in ("<<", ">>"):
        if rt[1]:
            raise ValueError(f"shift amount type {ctype(rt)} is signed; shift counts are unsigned")
        if rt[0] >= t[0]:
            raise ValueError(f"shift amount width {rt[0]} >= result width {t[0]}")
        return
    if lt[0] != t[0] or rt[0] != t[0]:
        raise ValueError(f"{op} operands must have the result width {t[0]} "
                         f"(got {ctype(lt)}, {ctype(rt)})")
    if lt[1] != t[1] or rt[1] != t[1]:
        raise ValueError(f"{op} operands must share the result signedness {ctype(t)}")


def _well_typed_cast(op, t, arg):
    """Fail closed on a cast whose declared direction contradicts its spelling.

    In C `(u32)x` sign-extends when `x` is signed and zero-extends when it is
    not. Accepting a `zext` of a signed operand, or a `sext` of an unsigned one,
    would let a rewrite reason about an extension the emitted C does not
    perform -- the rewrite vocabulary would be quietly unsound.
    """
    src = etype(arg)
    if op == "as":
        if src[0] != t[0]:
            raise ValueError(f"'as' is a same-width conversion, but {ctype(src)} -> {ctype(t)} "
                             "changes width; use zext, sext or trunc")
        return
    if op in ("zext", "sext"):
        if t[0] <= src[0]:
            raise ValueError(f"{op} must widen: {ctype(src)} -> {ctype(t)}")
        if (op == "zext") != (not src[1]):
            raise ValueError(f"{op} contradicts the source type {ctype(src)}: a C cast "
                             f"{ctype(src)} -> {ctype(t)} "
                             f"{'zero' if not src[1] else 'sign'}-extends")
        return
    if op == "trunc" and t[0] >= src[0]:
        raise ValueError(f"trunc must narrow: {ctype(src)} -> {ctype(t)}")


# ---------------------------------------------------------------------------
# Justified rewrites
# ---------------------------------------------------------------------------
#
# `_rules(node)` yields (new_expr, Side) only for side conditions that HOLD.
# `_rule_blocks(node)` yields (Side, reason) for conditions that are checkable
# here and FALSE -- recorded as blocked, never emitted. Nothing travels from one
# list to the other without its condition having been checked.


def _rules(node):
    """Yield (new_expr, Side) for every rule that fires at `node`."""
    t = etype(node)
    width, signed = t
    unsigned = not signed

    if node[0] == "bin":
        op, a, b = node[1], node[3], node[4]

        # constant folding, gated on representability (no signed wrap-around)
        if eval_const(node) is not None and not (signed and not fits(exact_binary(op, a, b), t)):
            yield const(eval_const(node), t), Side(
                "const-fold", f"both operands are constants and the exact {ctype(t)} result is "
                              "representable; signed overflow is refused, not wrapped")

        # commutativity: modular for unsigned, possibly-UB for signed
        if op in COMMUTATIVE and unsigned:
            yield binop(op, t, b, a), Side(
                "commute", f"{op} is commutative and {ctype(t)} arithmetic is modular, so operand "
                           "order cannot change the value")

        # a - b == a + (-b) modulo 2^W. Unsigned only.
        if op == "-" and unsigned:
            yield binop("+", t, a, unop("neg", t, b)), Side(
                "unsigned-sub-to-add", f"unsigned modular arithmetic at width {width}: a - b and "
                                       f"a + (-b) agree modulo 2^{width}; NOT valid for signed C, "
                                       "where the overflow is undefined")

        # x + 0 -> x
        zero = Side("add-zero", f"unsigned modular arithmetic at width {width}: x + 0 == x")
        if op == "+" and unsigned and const_value(b) == 0:
            yield a, zero
        if op == "+" and unsigned and const_value(a) == 0:
            yield b, zero

        # 2 * x == x << 1 modulo 2^W
        if op == "*" and unsigned and const_value(b) == 2:
            yield binop("<<", t, a, const(1, t)), Side(
                "mul-two-to-shift", f"unsigned modular arithmetic at width {width}: 2*x and x<<1 "
                                    "agree modulo 2^width; for signed x this rewrite can introduce "
                                    "undefined overflow")

        # (x & 0xFF) == (u8)x
        if op == "&" and unsigned and width > 8 and const_value(b) == 0xFF:
            yield cast("trunc", TYPES["u8"], a), Side(
                "mask-to-trunc", f"width {width} > 8, the mask is exactly 2^8-1 and the operand "
                                 "is unsigned, so masking is precisely an 8-bit truncation")

    if node[0] == "un" and node[1] == "neg" and unsigned:
        yield binop("-", t, const(0, t), node[3]), Side(
            "neg-to-sub", f"unsigned modular arithmetic at width {width}: -b and 0-b agree "
                          "modulo 2^width")

    if node[0] == "cast":
        op, src = node[1], etype(node[3])

        # (uW)x == (x & (2^W-1)) for an unsigned narrowing
        if op == "trunc" and unsigned and src[0] > width:
            yield binop("&", t, node[3], const(mask_of(t), t)), Side(
                "trunc-to-mask", f"the truncation narrows {ctype(src)} to width {width} and the "
                                 "source is unsigned, so only the low bits survive")

        inner = node[3]
        if inner[0] == "cast":
            # trunc_W(ext_W'(x)) -> x
            if op == "trunc" and src[0] > width and inner[1] in ("zext", "sext") \
                    and etype(inner[3])[0] == width and etype(inner[3])[1] == signed:
                yield inner[3], Side(
                    "ext-trunc-cancel", f"the truncation returns from width {src[0]} to {width} and "
                                       f"the extension agrees in signedness ({ctype(t)}), so the "
                                       "pair is the identity on x")
            # zext_32(trunc_8(x)) -> (x & 0xFF)
            if op == "zext" and unsigned and width > 8 and inner[1] == "trunc" \
                    and etype(inner[3])[0] == width and width == 8:
                yield binop("&", t, inner[3], const(0xFF, t)), Side(
                    "zext-of-trunc-to-mask", "zero-extending an 8-bit truncation equals masking "
                                             "with 0xFF at any wider unsigned width")
            # sext_W(sext_W'(x)) -> sext_W(x)
            if op == "sext" and signed and inner[1] == "sext" and etype(inner[3])[1] \
                    and width >= etype(inner)[0]:
                yield cast("sext", t, inner[3]), Side(
                    "sext-collapse", f"two sign extensions compose to one: the intermediate width "
                                     f"{etype(inner)[0]} is <= {width} and both extensions are signed")


def _rule_blocks(node):
    """(Side, reason) for every side condition that is checkable here and FALSE."""
    t = etype(node)
    out = []
    if node[0] != "bin":
        return out
    op = node[1]
    if op in COMMUTATIVE and t[1]:
        out.append((Side("commute", f"{op} is mathematically commutative, but reordering signed "
                                    f"{ctype(t)} arithmetic can introduce undefined overflow, so "
                                    "the rewrite is refused"),
                    f"{ctype(t)} is signed: reordering may overflow"))
    if op == "-" and t[1]:
        out.append((Side("unsigned-sub-to-add", "a - b == a + (-b) holds modulo 2^W; for "
                                                f"{ctype(t)} it can overflow, which is undefined"),
                    f"{ctype(t)} is signed"))
    if op == "*" and const_value(node[4]) == 2 and t[1]:
        out.append((Side("mul-two-to-shift", "2*x == x<<1 modulo 2^W; signed overflow is "
                                             "undefined, so the rewrite is refused"),
                    f"{ctype(t)} is signed"))
    if op in ("+", "-", "*") and eval_const(node) is not None and t[1]:
        exact = exact_binary(op, node[3], node[4])
        if not fits(exact, t):
            out.append((Side("const-fold", f"the exact {ctype(t)} result {exact} is not "
                                           "representable; folding it to a wrapped constant would "
                                           "change the value"),
                        f"signed overflow: {exact} does not fit in {ctype(t)}"))
    return out


def _replace(root, target, new) -> tuple:
    if root == target:
        return new
    if root[0] == "bin":
        return binop(root[1], root[2], _replace(root[3], target, new), _replace(root[4], target, new))
    if root[0] in ("un", "cast"):
        return root[0], root[1], root[2], _replace(root[3], target, new)
    if root[0] == "tmp":
        return tmp(_replace(root[3], target, new), root[4])
    return root


def rewrite_once(e) -> tuple[list, list]:
    """All single-site justified rewrites of `e`, as (applied, blocked).

    `applied` is [(new_expr, Side)]; `blocked` is [(Side, reason)].
    """
    applied, blocked = [], []
    for node in subterms(e):
        for new, side in _rules(node):
            applied.append((_replace(e, node, new), side))
        for side, reason in _rule_blocks(node):
            blocked.append((side, reason))
    return applied, blocked


# ---------------------------------------------------------------------------
# Bounded saturation over a rewrite DAG
# ---------------------------------------------------------------------------

@dataclass
class Limits:
    max_nodes: int = 400
    max_iters: int = 24
    max_extract: int = 16
    seconds: float = 900.0

    def to_json(self, used: dict, bound: str | None) -> dict:
        return {"max_nodes": self.max_nodes, "nodes_used": used.get("nodes", 0),
                "max_iters": self.max_iters, "iters_used": used.get("iters", 0),
                "max_extract": self.max_extract, "extract_used": used.get("extract", 0),
                "max_seconds": self.seconds, "seconds_used": round(used.get("seconds", 0.0), 3),
                "bound_hit": bound}


def saturate(seeds, limits: Limits, *, started: float | None = None):
    """Breadth-first rewrite-DAG enumerator with canonical deduplication.

    Not an e-graph: there is no congruence closure and no e-class analysis, only
    a bounded set of structurally canonicalised expressions. That is honest
    because the question being asked -- does the rewrite vocabulary produce
    *compiler variation* -- is answered by the compile step, not by graph theory.

    Returns (expressions_in_insertion_order, applied, blocked, used, bound).
    """
    started = time.monotonic() if started is None else started
    nodes: dict[tuple, tuple] = {}
    order: list[tuple] = []
    for seed in seeds:
        if seed not in nodes:
            nodes[seed] = seed
            order.append(seed)
    frontier = list(order)
    applied: list[dict] = []
    blocked: list[dict] = []
    bound = None
    iters = 0

    while frontier:
        if iters >= limits.max_iters:
            bound = "max_iters"
            break
        if len(nodes) >= limits.max_nodes:
            bound = "max_nodes"
            break
        if time.monotonic() - started > limits.seconds:
            bound = "max_seconds"
            break
        iters += 1
        fresh = []
        for expr in frontier:
            hit, missed = rewrite_once(expr)
            for new, side in hit:
                applied.append({"rule": side.rule, "side_condition": side.condition,
                                "from_expr": to_c(expr), "expr": to_c(new)})
                if new not in nodes and len(nodes) < limits.max_nodes:
                    nodes[new] = new
                    order.append(new)
                    fresh.append(new)
            for side, reason in missed:
                blocked.append({"rule": side.rule, "side_condition": side.condition,
                                "blocked_because": reason})
        frontier = fresh

    used = {"nodes": len(nodes), "iters": iters, "extract": 0,
            "seconds": time.monotonic() - started}
    return order, applied, blocked, used, bound


def extract(order, limit: int) -> list[tuple]:
    """Bounded set of structurally distinct representatives, plus named-temporary
    variants of the binary forms. This is a *diversity* filter; compiled-output
    dedup in `evaluate_representatives` decides how many search slots a group
    really costs."""
    picked, seen = [], set()
    for expr in order:
        key = shape_key(expr)
        if key in seen:
            continue
        seen.add(key)
        picked.append(expr)
        if len(picked) >= limit:
            break
    reps = list(picked)
    for expr in picked:
        if expr[0] != "bin" or len(reps) >= limit + max(0, limit // 2):
            continue
        variant = tmp(expr[3], binop(expr[1], expr[2], var(TMP_NAME, etype(expr[3])), expr[4]))
        if shape_key(variant) not in seen:
            seen.add(shape_key(variant))
            reps.append(variant)
    return reps


# ---------------------------------------------------------------------------
# Candidate emission and evaluation
# ---------------------------------------------------------------------------

def candidate_source(expr, function: str, return_type: str) -> str:
    """Render one representative as a scratch C89 body.

    A named temporary is a local declaration, never a parameter: the ABI
    registers belong to the real leaf parameters only.
    """
    types = {n: etype(x) for x in walk(expr)
             for n in ([x[3]] if x[0] == "leaf" and x[1] == "var" else [])}
    params = [n for n in variables(expr) if n != TMP_NAME]
    signature = ", ".join(f"{ctype(types[n])} {n}" for n in params)
    lines = [f"{return_type} {function}({signature}) {{"]
    for node in walk(expr):
        if node[0] == "tmp":
            lines.append(f"    {ctype(node[2])} {TMP_NAME} = {to_c(node[3])};")
    lines.append(f"    return {to_c(expr)};")
    lines.append("}")
    return "\n".join(lines)


def resolve_contract(name: str, return_type: str, abi_note: str) -> kit.Contract:
    """Map a ROM function name to its real span. Fails closed."""
    symbols = probe.load_code_symbols()
    spans = probe.rom_functions()
    vma = None
    for key in dict.fromkeys([name, name.lstrip("_"), "_" + name.lstrip("_"), name.upper()]):
        if key in symbols:
            vma = symbols[key]
            break
    if vma is None:
        raise kit.Unsupported(
            f"no ROM symbol named {name!r}; the target must be named exactly as it appears in "
            "tools/matching_slice_functions.json or asm/")
    end = spans.get(vma)
    if end is None:
        raise kit.Unsupported(f"{name} at {vma:#010x} has no recorded ROM span end; refusing to "
                              "score a candidate against a guessed span")
    return kit.Contract(name=name, vma=vma, end=end, return_type=return_type, abi_note=abi_note)


def _safe_to_c(expr):
    try:
        return to_c(expr)
    except kit.Unsupported as exc:
        return f"<unemittable: {exc}>"


def output_key(blob: bytes, vma: int):
    """Compiled-output identity: emitted bytes plus normalized instruction shape.

    The byte digest is always available. The instruction signature is best
    effort -- a blob that will not fingerprint yields `None` for that leg,
    which the caller treats as "not further distinguishable", never as
    "different". It is never allowed to raise: a fingerprint failure must not
    abort a measurement.
    """
    try:
        sig = kit.instruction_signature(blob, vma)
    except (ValueError, OSError, subprocess.SubprocessError):
        sig = None
    return (kit.digest_bytes(blob), None if sig is None else json.dumps(sig, sort_keys=True))


def self_test_span():
    """A real code span for the fingerprint leg of `output_key`, or None."""
    try:
        spans = probe.rom_functions()
    except (OSError, ValueError):  # pragma: no cover - defensive
        return None
    best = None
    for vma, end in sorted(spans.items()):
        size = end - vma
        if kit.ROM_BASE <= vma < kit.ROM_BASE + 0x40000 and 32 <= size <= 128:
            best = vma
            break
    return best


def evaluate_representatives(reps, adapter, contract, writer, *, max_slots: int):
    """Compile and score each representative, deduplicating by COMPILED OUTPUT.

    Two representatives that emit the same bytes with the same instruction
    signature are the same answer to this strategy's question; spending a second
    search slot on them would make every cost measurement a fiction.
    """
    rows: list[dict] = []
    seen_outputs: dict[tuple, str] = {}
    scores: list[kit.Score] = []
    slots = duplicates = 0
    for i, expr in enumerate(reps):
        if slots >= max_slots:
            break
        name = f"r{i:03d}"
        try:
            text = candidate_source(expr, f"cand_{name}", contract.return_type)
        except kit.Unsupported as exc:
            rows.append({"name": name, "status": kit.UNSUPPORTED_CONTRACT, "detail": str(exc),
                         "expr": _safe_to_c(expr), "consumed_slot": False})
            continue
        path = writer.save_candidate(name, text)
        compiled = adapter.compile(text, f"cand_{name}", contract.vma, index=i)
        if not compiled.ok:
            score = kit.Score(status=kit.COMPILE_ERROR, detail=compiled.error)
            scores.append(score)
            rows.append({"name": name, "status": score.status, "detail": score.detail,
                         "expr": _safe_to_c(expr), "path": writer.relative(path),
                         "consumed_slot": False})
            writer.record(rows[-1])
            continue
        key = output_key(compiled.blob, contract.vma)
        if key in seen_outputs:
            duplicates += 1
            rows.append({"name": name, "status": "DUPLICATE_OUTPUT",
                         "duplicate_of": seen_outputs[key], "blob_sha256": key[0],
                         "signature": key[1], "expr": _safe_to_c(expr),
                         "path": writer.relative(path), "consumed_slot": False})
            writer.record(rows[-1])
            continue
        seen_outputs[key] = name
        slots += 1
        score = adapter.score(compiled, contract)
        scores.append(score)
        row = {"name": name, "status": score.status, "matched_bytes": score.matched_bytes,
               "rom_bytes": score.rom_bytes, "prefix": score.prefix,
               "first_diff": score.first_diff, "detail": score.detail,
               "blob_sha256": key[0], "signature": key[1], "expr": _safe_to_c(expr),
               "path": writer.relative(path), "consumed_slot": True}
        writer.record(row)
        rows.append(row)
        if score.exact:
            writer.declare_winner(name, text, row)
            break
    return rows, scores, {"slots_used": slots, "duplicate_outputs": duplicates,
                          "distinct_compiled_outputs": len(seen_outputs)}


# ---------------------------------------------------------------------------
# Optional external egg/egglog
# ---------------------------------------------------------------------------

def egglog_revision(path: Path) -> dict:
    """Record an explicitly configured external e-graph engine, or refuse.

    Absence is `kit.DependencyMissing`, never a silent fall back to the Python
    enumerator: a run that asked for egg and silently got the enumerator would
    report a measurement of the wrong algorithm.
    """
    if not Path(path).exists():
        raise kit.DependencyMissing(
            f"--egglog {path} does not exist; configure it explicitly or omit the flag. This tool "
            "will not silently substitute its Python enumerator for a configured engine.")
    try:
        out = subprocess.run([str(path), "--version"], capture_output=True, text=True, timeout=30)
    except (OSError, subprocess.SubprocessError) as exc:
        raise kit.DependencyMissing(f"--egglog {path} could not be executed: {exc}") from None
    if out.returncode != 0:
        raise kit.DependencyMissing(f"--egglog {path} --version failed: "
                                    f"{(out.stderr or out.stdout).strip()[:200]}")
    return {"path": str(path), "version": (out.stdout or out.stderr).strip(),
            "protocol": "RECORDED, NOT DRIVEN: this build enumerates in pure Python. A driver must "
                        "speak a pinned revision-to-revision protocol; the recorded revision is in "
                        "run.json either way."}


# ---------------------------------------------------------------------------
# Run
# ---------------------------------------------------------------------------

DEFAULT_SEED = "(u32 + (u32 * (u32 a) (u32 3)) (u32 & (u32 b) (u32 255)))"


def body(args) -> int:
    limits = Limits(max_nodes=args.max_nodes, max_iters=args.max_iters,
                    max_extract=args.max_extract, seconds=args.seconds)
    engine = egglog_revision(Path(args.egglog)) if args.egglog else None
    kit.require_toolchain()

    contract = resolve_contract(args.name, args.return_type,
                                abi_note="pure-integer leaf; return type declared by the caller")
    contract.require_supported()

    if args.seeded_expression:
        seeds = parse_seeded(Path(args.seeded_expression).read_text())
        origin = str(args.seeded_expression)
    else:
        seeds = parse_seeded(DEFAULT_SEED)
        origin = "built-in default seed (no --seeded-expression given)"

    started = time.monotonic()
    writer = kit.EvidenceWriter(STRATEGY, args.run_id, root=args.out,
                                extra={"max_nodes": limits.max_nodes,
                                       "max_iters": limits.max_iters,
                                       "max_extract": limits.max_extract,
                                       "egglog": engine}).open()
    budget = kit.Budget(compiles=args.max_compiles, seconds=limits.seconds)
    meter = kit.BudgetMeter(budget)
    adapter = kit.CandidateAdapter(writer.dir / "work", meter=meter)
    writer.save_contract(contract)

    order, applied, blocked, used, bound = saturate(seeds, limits, started=started)
    reps = extract(order, limits.max_extract)
    used["extract"] = len(reps)
    writer.write_json("rewrites.json", {"applied": applied, "blocked": blocked})
    writer.write_json("graph.json", {"seed_origin": origin, "nodes": len(order),
                                     "bound_hit": bound,
                                     "expressions": [{"expr": _safe_to_c(e), "size": size(e)}
                                                     for e in order]})

    rows, scores, dedup = evaluate_representatives(
        reps, adapter, contract, writer, max_slots=args.max_slots)
    status = kit.summarize_status(scores, exhausted=bool(bound) or meter.compiles >= budget.compiles)
    payload = {
        "draft_status": status,
        "target": contract.name,
        "target_vma": f"{contract.vma:#010x}",
        "return_type": contract.return_type,
        "seed_origin": origin,
        "graph_nodes": len(order),
        "rewrites_applied": len(applied),
        "rewrites_blocked": len(blocked),
        "representatives": len(reps),
        "compile_slots_used": dedup["slots_used"],
        "duplicate_compiled_outputs": dedup["duplicate_outputs"],
        "distinct_compiled_outputs": dedup["distinct_compiled_outputs"],
        "limits": limits.to_json(used, bound),
        "budget": meter.to_json(),
        "adapter_cache": adapter.cache_stats(),
        "egglog": engine,
        "notes": [
            "EXACT_DRAFT means scratch bytes equal the ROM span; integration and the independent "
            "link were not run.",
            "This is a rewrite-DAG enumerator, not a full e-graph: no congruence closure.",
            f"Search was bounded by: {bound or 'none of the node/iteration/time limits'}.",
        ],
        "results": rows,
    }
    writer.finish(payload)
    print(f"{STRATEGY}: {contract.name} at {contract.vma:#010x} — {status}")
    print(f"  graph nodes {len(order)}; rewrites applied {len(applied)}, blocked {len(blocked)}")
    print(f"  representatives {len(reps)} -> {dedup['slots_used']} compile slots; "
          f"{dedup['duplicate_outputs']} duplicate compiled outputs; "
          f"{dedup['distinct_compiled_outputs']} distinct")
    print(f"  bounds: {payload['limits']}")
    print(f"  evidence: {writer.relative(writer.dir)}")
    return 0


class _Parsed:
    """Hand an already-parsed namespace to `kit.run_tool`."""

    def __init__(self, args):
        self._args = args

    def parse_args(self, _argv=None):  # pragma: no cover - trivial
        return self._args


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0], epilog=ONE_LINE,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("name", nargs="?", help="ROM function name, exactly as the repo spells it")
    ap.add_argument("--return-type", default="u32", help="declared return type (default u32)")
    ap.add_argument("--seeded-expression", type=Path,
                    help="file of seeded expressions in the '(u32 + a b)' DSL")
    ap.add_argument("--out", type=Path, help="private run directory")
    ap.add_argument("--run-id", help="run id (default: UTC timestamp)")
    ap.add_argument("--max-nodes", type=int, default=400, help="bound on distinct graph nodes")
    ap.add_argument("--max-iters", type=int, default=24, help="bound on rewrite iterations")
    ap.add_argument("--max-extract", type=int, default=16, help="bound on extracted representatives")
    ap.add_argument("--max-slots", type=int, default=12, help="bound on distinct compiled outputs")
    ap.add_argument("--max-compiles", type=int, default=64, help="agbcc invocations allowed")
    ap.add_argument("--seconds", type=float, default=900.0, help="wall-clock bound")
    ap.add_argument("--egglog", help="explicit path to an external egg/egglog executable")
    ap.add_argument("--seed", type=int, default=0,
                    help="recorded in the evidence; the search is deterministic and ignores it")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        return self_test()
    if not args.name:
        ap.error("NAME is required unless --self-test is given")
    if args.out and not args.run_id:
        args.run_id = "run"
    return kit.run_tool(STRATEGY, body, ap=_Parsed(args), argv=[])


# ---------------------------------------------------------------------------
# Self-test
# ---------------------------------------------------------------------------

def self_test() -> int:
    print("running egraph_search self-test...")
    passed = failed = skipped = total = 0

    def check(label, condition):
        nonlocal passed, failed, total
        total += 1
        if condition:
            passed += 1
            print(f"  [PASS] {label}")
        else:
            failed += 1
            print(f"  [FAIL] {label}", file=sys.stderr)

    def skip(label, why):
        nonlocal skipped, total
        total += 1
        skipped += 1
        print(f"  [SKIP] {label} ({why})")

    def rules_at(text):
        return rewrite_once(parse_seeded(text)[0])

    # -- parser and refusals ----------------------------------------------
    try:
        parsed = parse_seeded("(u32 + (u32 * (u32 a) (u32 3)) (u32 & (u32 b) (u32 255)))")
        ok_parse = len(parsed) == 1 and to_c(parsed[0]) == "((a * 3u) + (b & 255u))"
    except kit.Unsupported as exc:  # pragma: no cover - defensive
        ok_parse = False
    check("seeded expression parses and emits C", ok_parse)

    for bad_op in ("load", "store", "call", "volatile", "hw", "asm"):
        refused = False
        try:
            parse_seeded(f"(u32 + (u32 {bad_op} (u32 a)) (u32 1))")
        except kit.Unsupported:
            refused = True
        check(f"{bad_op!r} node is refused, not modelled", refused)

    check("incoherent operand widths are refused",
          kit.raises(kit.Unsupported, lambda: parse_seeded("(u32 + (u16 (u16 a)) (u32 1))")))
    check("a missing --egglog path is DependencyMissing, never a silent fallback",
          kit.raises(kit.DependencyMissing, lambda: egglog_revision(Path("/nonexistent/egglog"))))

    # -- side conditions: the core correctness property --------------------
    signed_fired = {side.rule for _, side in rules_at("(s32 * (s32 a) (s32 2))")[0]}
    signed_blocked = {side.rule for side, _ in rules_at("(s32 * (s32 a) (s32 2))")[1]}
    check("an unsigned-only rewrite is NOT applied to a signed expression",
          "mul-two-to-shift" not in signed_fired and "mul-two-to-shift" in signed_blocked)
    check("an unsigned-only rewrite is NOT applied to a signed subtraction",
          "unsigned-sub-to-add" not in {s.rule for _, s in rules_at("(s32 - (s32 a) (s32 b))")[0]}
          and "unsigned-sub-to-add" in {s.rule for s, _ in rules_at("(s32 - (s32 a) (s32 b))")[1]})

    ovf_applied, ovf_blocked = rules_at("(s8 + (s8 127) (s8 1))")
    check("signed overflow BLOCKS constant folding rather than wrapping it",
          "const-fold" not in {s.rule for _, s in ovf_applied}
          and any("signed overflow" in reason for _, reason in ovf_blocked))
    check("constant folding DOES fire when the signed result is representable",
          "const-fold" in {s.rule for _, s in rules_at("(s8 + (s8 100) (s8 27))")[0]})
    check("every unsigned modular identity preserves the value",
          _identity_preserved("(u32 * (u32 a) (u32 2))", "mul-two-to-shift",
                              lambda a, b: a * 2, lambda a, b: a << 1)
          and _identity_preserved("(u32 - (u32 a) (u32 b))", "unsigned-sub-to-add",
                                  lambda a, b: a - b, lambda a, b: a + (-b)))
    check("the unsigned side condition is load-bearing: 2*a overflows signed width",
          any(_overflows_signed(a) for a in (0x40000000, 0x7FFFFFFF, 0xFFFFFFFF, 0x80000000)))
    check("an unsigned modular rewrite is actually emitted (not vacuously absent)",
          {s.rule for _, s in rules_at("(u32 * (u32 a) (u32 2))")[0]} >= {"mul-two-to-shift"}
          and {s.rule for _, s in rules_at("(u32 - (u32 a) (u32 b))")[0]} >= {"unsigned-sub-to-add"})

    # Mixed seeds: the unsigned default exercises the applied rules, the signed
    # ones exercise the BLOCKED side conditions. A purely unsigned seed would
    # make the blocked-evidence check vacuously true.
    mixed = parse_seeded(DEFAULT_SEED + "\n(s32 - (s32 a) (s32 b))\n(s8 + (s8 127) (s8 1))")

    order, applied, blocked, used, bound = saturate(
        mixed, Limits(max_nodes=150, max_iters=6, max_extract=8, seconds=60.0))
    check("every emitted rewrite carries a non-empty side condition",
          bool(applied) and all(r.get("rule") and r.get("side_condition") for r in applied))
    check("every blocked rewrite records why it was blocked",
          bool(blocked) and all(r.get("side_condition") and r.get("blocked_because")
                                for r in blocked))
    check("evidence is JSON-serializable",
          json.loads(json.dumps({"applied": applied, "blocked": blocked})) is not None)

    # -- bounds -------------------------------------------------------------
    _, _, _, tiny_used, tiny_bound = saturate(
        parse_seeded(DEFAULT_SEED), Limits(max_nodes=1, max_iters=50, max_extract=2, seconds=60.0))
    check("the node bound actually stops the search",
          tiny_bound == "max_nodes" and tiny_used["nodes"] <= 1)
    _, _, _, it_used, it_bound = saturate(
        parse_seeded(DEFAULT_SEED), Limits(max_nodes=10**6, max_iters=2, max_extract=2, seconds=60.0))
    check("the iteration bound actually stops the search",
          it_bound == "max_iters" and it_used["iters"] <= 2)
    _, _, _, sec_used, sec_bound = saturate(
        parse_seeded(DEFAULT_SEED), Limits(max_nodes=10**6, max_iters=10**6, max_extract=2, seconds=0.0))
    check("the time bound actually stops the search",
          sec_bound == "max_seconds" and sec_used["seconds"] < 5.0)
    reported = Limits(max_nodes=1, max_iters=2, max_extract=3, seconds=4.0).to_json(
        tiny_used, tiny_bound)
    check("all four bounds are reported independently",
          all(k in reported for k in ("max_nodes", "nodes_used", "max_iters", "iters_used",
                                      "max_extract", "extract_used", "max_seconds",
                                      "seconds_used", "bound_hit")))

    # -- extraction ---------------------------------------------------------
    reps = extract(order, 6)
    check("extraction is bounded and structurally distinct",
          0 < len(reps) <= 9 and len({shape_key(x) for x in reps}) == len(reps))
    check("extraction includes a named-temporary variant", any(x[0] == "tmp" for x in reps))
    body_text = candidate_source(reps[0], "cand_t", "u32")
    check("a representative emits a C89 body with a return",
          body_text.startswith("u32 cand_t(") and "return" in body_text)
    temp_text = candidate_source([x for x in reps if x[0] == "tmp"][0], "cand_tmp", "u32")
    check("a named-temporary variant declares its temporary",
          "u32 t0 = " in temp_text or "u8 t0 = " in temp_text or "u16 t0 = " in temp_text
          or "s32 t0 = " in temp_text)

    # -- compiled-output dedup (needs the real toolchain) -------------------
    ok, reason = kit.CandidateAdapter.toolchain_ready()
    if not ok:
        skip("identical compiled outputs deduplicate to one slot", reason)
    else:
        span = self_test_span()
        if span is None:
            skip("identical compiled outputs deduplicate to one slot", "no code span available")
        else:
            with tempfile.TemporaryDirectory() as tmp:
                adapter = kit.CandidateAdapter(Path(tmp) / "work")
                blob_a = adapter.compile("u32 cand_a(u32 a) { return (a & 255u); }",
                                         "cand_a", span, index=0).blob
                blob_b = adapter.compile("u32 cand_b(u32 a) { return (a & 0xFFu); }",
                                         "cand_b", span, index=1).blob
                blob_c = adapter.compile("u32 cand_c(u32 a) { return (a * 3u); }",
                                         "cand_c", span, index=2).blob
                check("two different spellings that compile identically deduplicate to one slot",
                      bool(blob_a) and bool(blob_b)
                      and output_key(blob_a, span) == output_key(blob_b, span))
                check("a different compiled output is a distinct slot",
                      bool(blob_c) and output_key(blob_a, span) != output_key(blob_c, span))

    print(f"\n{passed}/{total} passed, {failed} failed, {skipped} skipped")
    return 0 if failed == 0 else 1


def _identity_preserved(text: str, rule: str, original, rewritten) -> bool:
    """The rule fires AND every sampled input keeps its value."""
    fired = [side for _, side in rewrite_once(parse_seeded(text)[0])[0] if side.rule == rule]
    if not fired:
        return False
    m = 0xFFFFFFFF
    samples = (0, 1, 2, 3, 5, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 12345)
    return all(original(a, b) & m == rewritten(a, b) & m
               for a in samples for b in samples[:4])


def _overflows_signed(a: int) -> bool:
    """True when 2*a is outside the signed 32-bit range: the negative control
    proving the unsigned side condition is load-bearing rather than decorative."""
    return not (-(1 << 31) <= 2 * a <= (1 << 31) - 1)


if __name__ == "__main__":
    sys.exit(main())
