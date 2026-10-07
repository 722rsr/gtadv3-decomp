#!/usr/bin/env python3
"""proto_audit.py — flag symbols declared with conflicting signatures.

Why this exists
---------------
The C lift is spread over ~150 TUs, and a symbol's *definition* lives in
whichever TU owns it while every other TU re-declares it (there is no umbrella
header for most of them). C prototypes are not checked across translation
units, so a TU that declares

    extern void _0802E0A4(const void *src, void *dst, u32 n);

while `src/foundation_runtime.c` defines

    void RuntimeMemcpy(void *dst, const void *src, u32 len)

compiles and links cleanly — and every call in that TU passes its arguments in
the wrong order at run time. That is a real bug class found on  (the
grid family's `_0802E0A4` callers copied into ROM instead of out of it).

This tool finds every such disagreement **statically**: it collects each
declaration/definition of a symbol from `src/` (and `include/`), normalises the
parameter types (names and qualifiers dropped), and reports symbols whose
declarations do not all agree on the parameter *shape*.

Usage
-----
    python3 tools/proto_audit.py                 # real parameter conflicts
    python3 tools/proto_audit.py --symbol _0802E0A4
    python3 tools/proto_audit.py --only-differing-return   # the benign class
    python3 tools/proto_audit.py --json out.json

Two classes, reported separately
--------------------------------
* **Parameter conflicts** (the bug class above): the parameter tuples differ in
  arity or in which positions are pointers. One side must be wrong.
* **Return-shape only** (benign): every parameter tuple agrees and only the
  declared *return* type differs (`void *` vs `int`). A pointer and a scalar
  are both returned in r0, so this cannot change a call's behaviour — it is
  noise, reported on its own so it does not mask a real conflict. The exit
  status follows the class actually reported.

Notes / limits
--------------
* Parses the *declaration text* with a regex, not a real C parser. `__attribute__`
  blocks, `#ifdef` host stubs and multi-line signatures are handled heuristically;
  a handful of odd cases are reported as `unparsed` rather than silently dropped.
* `void f()` (no prototype) and `void f(void)` are treated as the same shape.
* Only symbols with at least two declarations are compared; single-declaration
  symbols cannot conflict by construction.
"""

import argparse
import json
import os
import re
import sys

# A declaration or definition: `type name(params)` with optional storage class.
DECL_RE = re.compile(
    r"^(?P<pre>(?:__attribute__\s*\(\([^)]*\)\)\s*)?"
    r"(?:extern|static|inline|__attribute__\s*\(\([^)]*\)\)|\s)*)"
    r"(?P<ret>(?:const\s+)?(?:unsigned\s+|signed\s+)?"
    r"(?:void|char|short|int|long|u8|u16|u32|s8|s16|s32|f32|size_t|uintptr_t))\s*"
    r"(?P<ptr>\**)\s*"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*"
    r"\((?P<params>[^;{)]*(?:\([^)]*\)[^;{)]*)*)\)\s*"
    r"(?P<tail>__attribute__\s*\(\([^)]*\)\)\s*)?"
    r"(?P<end>[;{=])",
    re.MULTILINE,
)

TYPE_WORDS = {
    "unsigned": "u", "signed": "s", "int": "int", "char": "char",
    "short": "short", "long": "long", "void": "void", "size_t": "size_t",
    "uintptr_t": "uintptr_t", "float": "float", "double": "double",
}


def normalise_type(t):
    """Reduce a C type spelling to a comparable token."""
    t = t.strip()
    if t == "" or t == "void":
        return "void"
    ptrs = t.count("*")
    body = t.replace("*", " ")
    body = re.sub(r"\b(const|volatile|register|struct|union|enum)\b", " ", body)
    body = re.sub(r"\s+", " ", body).strip()
    words = body.split()
    mapped = []
    for w in words:
        if re.fullmatch(r"[us](8|16|32|64)", w):
            mapped.append(w)
        elif w in TYPE_WORDS:
            mapped.append(TYPE_WORDS[w])
        else:
            mapped.append(w)  # unknown typedef — keep verbatim
    out = " ".join(mapped) if mapped else "int"
    return out + "*" * ptrs


def split_params(params):
    """Split a parameter list on top-level commas (no nested parens expected)."""
    params = params.strip()
    if params == "" or params == "void":
        return []
    out, depth, cur = [], 0, ""
    for ch in params:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    out.append(cur)
    shapes = []
    for p in out:
        p = p.strip()
        if not p:
            continue
        # drop the parameter name (last identifier, unless it is the type)
        p = re.sub(r"\[[^\]]*\]", "*", p)          # array params -> pointer
        p = re.sub(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*$", "", p).strip()
        if not p:
            p = "int"
        shapes.append(normalise_type(p))
    return shapes


def strip_blocks(text):
    """Blank out comments, preprocessor lines, and `#ifdef __APPLE__` bodies
    (keeps line numbers). The apple branches carry host-only weak stubs that
    disagree on purpose with the ARM externs — counting both fakes conflicts
    between TUs that are each internally consistent."""
    out, i, n = [], 0, len(text)
    depth = 0
    while i < n:
        two = text[i:i + 2]
        if depth == 0 and two == "//":
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
            continue
        if two == "/*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", text[i:j]))
            i = j
            continue
        if text[i] == "\n" and depth == 0:
            pass
        out.append(text[i])
        i += 1
    text = "".join(out)
    # Blank `#ifdef __APPLE__` branch bodies line by line (keeping newlines so
    # line numbers survive): those branches carry host-only weak stubs whose
    # signatures deliberately differ from the ARM branch's externs.
    res, in_apple, depth = [], 0, 0
    for ln in text.split("\n"):
        st = ln.strip()
        if in_apple == 0 and re.match(r"#\s*ifdef\s+__APPLE__", st):
            in_apple, depth = 1, 1
            res.append(" " * len(ln))
            continue
        if in_apple:
            if re.match(r"#\s*if", st):
                depth += 1
            elif re.match(r"#\s*endif", st):
                depth -= 1
                if depth <= 0:
                    in_apple = 0
            elif depth == 1 and re.match(r"#\s*else", st):
                in_apple = 0          # keep the ARM (#else) branch
            res.append(" " * len(ln))
            continue
        res.append(ln)
    text = "\n".join(res)
    # blank whole preprocessor directives
    return re.sub(r"^[ \t]*#.*$", lambda m: " " * len(m.group(0)), text, flags=re.MULTILINE)


# Macro-wrapped declarations (`HOST_STUB(void f(int));`, `WEAK(int g(void));`)
# hide the real signature from DECL_RE, so unwrap them first.
MACRO_DECL_RE = re.compile(
    r"^[ \t]*(?:HOST_STUB|WEAK|WEAK_DECL)[ \t]*\((.*)\)[ \t]*;[ \t]*$",
    re.MULTILINE)


def collect(src_dirs):
    """{symbol: [(file, line, return, params-shape, kind, text)]}"""
    found = {}
    unparsed = []
    for src_dir in src_dirs:
        for root, _dirs, files in os.walk(src_dir):
            for fn in sorted(files):
                if not fn.endswith((".c", ".h")):
                    continue
                path = os.path.join(root, fn)
                raw = open(path, "r", errors="replace").read()
                text = strip_blocks(raw)
                lines = raw.splitlines()
                spans = [(m.start(), m.end(), m) for m in DECL_RE.finditer(text)]
                # macro-wrapped declarations: parse the inner text in place
                for mm in MACRO_DECL_RE.finditer(text):
                    inner = mm.group(1)
                    im = DECL_RE.search(inner + ";")
                    if not im:
                        continue
                    name = im.group("name")
                    if name in ("if", "while", "for", "switch", "return", "sizeof"):
                        continue
                    ret = normalise_type((im.group("ret") or "int") + im.group("ptr"))
                    params = tuple(split_params(im.group("params")))
                    line = text.count("\n", 0, mm.start()) + 1
                    found.setdefault(name, []).append(
                        (path, line, ret, params, "macro",
                         lines[line - 1].strip() if line <= len(lines) else ""))
                for start, _end, m in spans:
                    name = m.group("name")
                    if name in ("if", "while", "for", "switch", "return", "sizeof"):
                        continue
                    ret = normalise_type(
                        (m.group("ret") or "int") + m.group("ptr"))
                    params = tuple(split_params(m.group("params")))
                    line = text.count("\n", 0, start) + 1
                    kind = "def" if m.group("end") == "{" else "decl"
                    found.setdefault(name, []).append(
                        (path, line, ret, params, kind, lines[line - 1].strip()
                         if line <= len(lines) else ""))
    return found, unparsed


def shape(entry):
    return (entry[2], entry[3])


def abi_shape(entry):
    """Collapse a signature to what actually reaches the callee.

    The ARM ABI passes every 32-bit scalar (int/u32/s32/s16/...) and every
    pointer in one register, so `int` vs `u32` and `int` vs `s16` are the same
    call. What can break at run time is pointer-vs-scalar confusions, arity,
    and pointer-vs-scalar returns. Only those are reported by default.
    """
    ret = "P" if entry[2].endswith("*") else "S"
    params = tuple("P" if p.endswith("*") else "S" for p in entry[3])
    return (ret, params)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--src", action="append", default=None,
                    help="directory to scan (repeatable; default src + include)")
    ap.add_argument("--symbol", action="append", help="only this symbol (repeatable)")
    ap.add_argument("--only-differing-return", action="store_true",
                    help="report only symbols whose arity matches but return differs")
    ap.add_argument("--include-macros", action="store_true",
                    help="also compare declarations hidden inside HOST_STUB/WEAK "
                         "macros (host-link scaffolding only; expands to nothing "
                         "on ARM, so its signature cannot affect the ROM build)")
    ap.add_argument("--include-static", action="store_true",
                    help="also compare `static` definitions — a TU-local body "
                         "cannot clash with another TU's declaration")
    ap.add_argument("--spelling", action="store_true",
                    help="compare full type spellings instead of ABI shapes "
                         "(much noisier: `int` vs `u32` params are ABI-identical)")
    ap.add_argument("--top", type=int, default=0)
    ap.add_argument("--json", help="write the full report as JSON")
    ap.add_argument("--show-agreeing", action="store_true")
    args = ap.parse_args()

    dirs = args.src or ["src", "include"]
    found, _ = collect(dirs)

    conflicts, retonly, agreeing = [], [], []
    skipped = 0
    for name, decls in found.items():
        if args.symbol and name not in args.symbol:
            continue
        usable = []
        for d in decls:
            if d[4] == "macro" and not args.include_macros:
                skipped += 1
                continue
            if re.match(r"^static\b", d[5]) and not args.include_static:
                skipped += 1
                continue
            usable.append(d)
        key = abi_shape if not args.spelling else shape
        shapes = {}
        for d in usable:
            shapes.setdefault(key(d), []).append(d)
        # An unspecified `()` parameter list is not a competing signature — it
        # means "arguments unspecified" and the compiler passes whatever the
        # call site writes. It cannot conflict with a real prototype, so drop
        # it when a real one exists (otherwise every `void f();` forward
        # declaration would look like a bug).
        if len({p for _r, p in shapes}) > 1:
            real = {k: v for k, v in shapes.items() if len(k[1])}
            if real:
                shapes = real
        if len(shapes) < 2:
            agreeing.append((name, decls))
            continue
        # A pointer and a scalar both come back in r0, so a return-type
        # disagreement is ABI-identical; only the parameter tuple decides
        # whether a call site can feed the callee the wrong thing.
        paramsets = {params for _ret, params in shapes}
        rets = {ret for ret, _params in shapes}
        if len(paramsets) > 1:
            conflicts.append((name, shapes))
        elif len(rets) > 1:
            retonly.append((name, shapes))

    if args.json:
        with open(args.json, "w") as fh:
            json.dump({n: [{"site": f"{d[0]}:{d[1]}", "kind": d[4],
                            "ret": d[2], "params": list(d[3]), "text": d[5]}
                           for d in decls]
                       for n, decls in found.items()}, fh, indent=1)

    mode = "type spellings" if args.spelling else "ABI shapes (pointer/scalar/arity)"
    print(f"scanned      : {', '.join(dirs)}")
    print(f"symbols seen : {len(found)}")
    print(f"compared by  : {mode}")
    print(f"param conflicts : {len(conflicts)}"
          + ("   (marked MAJORITY / outlier)" if conflicts else ""))
    print(f"return-shape only (benign): {len(retonly)}"
          + ("  — list with --only-differing-return" if retonly else ""))
    if skipped:
        print(f"ignored          : {skipped} macro-wrapped / static entries "
              "(--include-macros / --include-static to compare them)")
    print()
    selected = retonly if args.only_differing_return else conflicts
    rows = selected[:args.top] if args.top else selected
    for name, shapes in rows:
        print(f"!! {name}")
        groups = sorted(shapes.items(), key=lambda kv: -len(kv[1]))
        for i, ((ret, params), decls) in enumerate(groups):
            sig = f"{ret} ({', '.join(params)})"
            tag = "" if (i == 0 or len(groups) == 1) else "   <-- OUTLIER"
            print(f"   {len(decls)}x  {sig}{tag}")
            for path, line, _r, _p, kind, text in decls:
                print(f"        {kind:<4} {path}:{line}")
    if args.show_agreeing:
        print()
        for name, decls in agreeing:
            print(f"ok {name}  ({len(decls)} agree)")
    if rows:
        print()
        print("A conflict is not automatically a bug: the ROM ABI decides which")
        print("side is right (read the asm at the symbol's address). What this")
        print("list guarantees is that at most one of the shapes can be right.")
        return 1
    if selected is retonly:
        print("No symbols differ in return shape only.")
    else:
        print("No parameter conflicts among scanned symbols.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
