#!/usr/bin/env python3
"""Recompute per-asm-cluster C-lift coverage (evidence-backed, source-level).

asm side: unique function VMAs per asm file, from `.type NAME, %function`
  lines (spellings `_0800XXXX` / `sub_0800XXXX` / old-style aliases normalized
  to bare lowercase 6-hex VMA "0800xxx").

C side (source of truth = src/*.c, NOT build-c objects — aliases at the same
  address plus generated alias/trampoline passes make nm counts unreliable):
  - function definitions: NAME(...) {  (brace-open, C control keywords excluded)
  - alias declarations:  NAME(...) __attribute__((alias("TARGET")))
  - weak definitions:    ... __attribute__((weak)) ... NAME(...)
A VMA counts as covered iff it resolves (through alias chains, cycles
guarded) to a defined function that is NOT a placeholder:
  - name contains "TODO" (Rec35Helper_TODO, Code22D20_TODO, ...)
  - declared weak (weak no-op fallbacks are documented non-lifts)
Truthful caveat: a real `T` body marked "For-brevity/simplified" in a comment
still counts here, exactly like the prior coverage audits.

Usage: python3 tools/coverage.py [--src-dir src] [--asm-dir asm] [--top 20]
       python3 tools/coverage.py --file menu_ff78.s   # per-VMA detail for one asm file
"""
import argparse
import glob
import os
import re
import sys

VMA_RE = re.compile(r"(0+80[0-9a-fA-F]{4,})")
TYPE_RE = re.compile(r"^\s*\.type\s+([A-Za-z_][A-Za-z0-9_]*)\s*,\s*%function")
FUNC_RE = re.compile(
    r"^\s*(?:static\s+|const\s+|unsigned\s+|signed\s+|volatile\s+)*"
    # GNU attribute prefixes (naked/weak/always_inline/...) before the type
    r"(?:__attribute__\s*\(\s*\([^)]*\)\s*\)\s*)*"
    # Some declarations place `static` after the attribute, as in
    # `__attribute__((naked)) static u32 Entry(...)`.
    r"(?:static\s+|const\s+|unsigned\s+|signed\s+|volatile\s+)*"
    r"(?:u8|u16|u32|s8|s16|s32|uint8_t|uint16_t|uint32_t|uint64_t|"
    r"int8_t|int16_t|int32_t|int64_t|uintptr_t|intptr_t|size_t|"
    r"int|void|bool|char|long\s+long|long|short)\s*\*?\s*"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*\("
)
ALIAS_RE = re.compile(
    r"\b(?:u8|u16|u32|s8|s16|s32|uint8_t|uint16_t|uint32_t|uint64_t|"
    r"int8_t|int16_t|int32_t|int64_t|uintptr_t|intptr_t|size_t|"
    r"int|void|bool|char|long|short|unsigned|signed)\b"
    # bridges must not cross ';' (next decl), '{}/}' (fn body), or '#' (preproc)
    # so a definition can't merge with the following line's alias decl.
    r"[^;{}#]*?([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}#]*?\)\s*"
    r"__attribute__\s*\(\s*\(\s*alias\s*\(\s*\"([^\"]+)\"[^;\"]*\)\s*\)\s*\)\s*;"
)
WEAK_RE = re.compile(r"__attribute__\s*\(\s*\(\s*weak")
KEYWORDS = {
    "if", "while", "for", "switch", "return", "sizeof", "else", "do",
    "case", "default",
}


def norm_vma(token: str):
    """'sub_08025C84' / '_0800A60' / '08004A2C' -> '0800a60' or None."""
    m = VMA_RE.search(token)
    if not m:
        return None
    v = m.group(1).lstrip("0").lower()
    if not v.startswith("80"):
        return None
    v = v[2:]
    if len(v) < 4 or len(v) > 6:
        return None
    return v.zfill(6)


def strip_comments(text):
    """Remove /*...*/ and //... comments so alias scanning can't seed matches
    from prose that mentions type keywords (e.g. 'u8 widths' in a header comment)."""
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    return text


def strip_host_ifdefs(text):
    """Drop `#ifdef __APPLE__` … `#else`/`#endif` host-only stubs so they
    cannot overwrite ARM real bodies when scanning source for coverage.

    Keeps the `#else` (non-Apple) branch when present; otherwise drops the
    whole Apple block. Nested ifdefs inside a dropped Apple block are ignored
    (depth counter). Other `#if`/`#ifdef` lines are left alone.
    """
    out = []
    i = 0
    lines = text.splitlines(keepends=True)
    n = len(lines)
    while i < n:
        line = lines[i]
        if re.match(r"^\s*#\s*ifdef\s+__APPLE__\b", line):
            depth = 1
            i += 1
            apple_body = []
            else_body = None
            while i < n and depth > 0:
                cur = lines[i]
                if re.match(r"^\s*#\s*if(n?def)?\b", cur):
                    depth += 1
                    if else_body is None:
                        apple_body.append(cur)
                    else:
                        else_body.append(cur)
                elif re.match(r"^\s*#\s*endif\b", cur):
                    depth -= 1
                    if depth == 0:
                        i += 1
                        break
                    if else_body is None:
                        apple_body.append(cur)
                    else:
                        else_body.append(cur)
                elif depth == 1 and re.match(r"^\s*#\s*else\b", cur):
                    else_body = []
                else:
                    if else_body is None:
                        apple_body.append(cur)
                    else:
                        else_body.append(cur)
                i += 1
            # Prefer the non-Apple branch; if none, drop the Apple stubs.
            if else_body is not None:
                out.extend(else_body)
            continue
        out.append(line)
        i += 1
    return "".join(out)


def attr_applies_to_defn(lines, idx):
    """True iff a weak/naked attribute belongs to the function at lines[idx].

    Looks only at the definition line itself and immediately preceding
    attribute-only lines that are NOT themselves another function definition
    (the old 3-line window falsely tagged the next fn as weak).
    """
    if WEAK_RE.search(lines[idx]):
        return True
    j = idx - 1
    while j >= 0:
        s = lines[j].strip()
        if not s or s.startswith("//") or s.startswith("/*") or s.startswith("*"):
            j -= 1
            continue
        # A prior function definition (with or without its own attribute) ends
        # the search — its weak attr does not apply to us.
        if FUNC_RE.match(lines[j]) and FUNC_RE.match(lines[j]).group(1) not in KEYWORDS:
            break
        if s.startswith("__attribute__"):
            # Pure attribute line for the upcoming definition (no type/name yet).
            if "(" in s and ")" in s and not re.search(
                r"\)\s*(?:__attribute__|$)", s
            ):
                # Heuristic: attribute-only lines look like
                # `__attribute__((weak))` with no trailing declarator.
                pass
            if re.match(r"^__attribute__\s*\(\s*\([^)]*\)\s*\)\s*$", s):
                if WEAK_RE.search(lines[j]):
                    return True
                j -= 1
                continue
            # Attribute glued onto a prior definition — stop.
            break
        break
    return False


def macro_defined_functions(text):
    """Return function names emitted by simple function-like C macros.

    The source corpus has ABI veneer bodies generated by a macro whose body
    declares ``void name(void)``. The C compiler expands these definitions,
    but this source inventory intentionally does not run a preprocessor. Detect
    that narrow, structural pattern and record each invocation's first
    argument as a real function definition.
    """
    lines = text.splitlines()
    generated = set()
    i = 0
    while i < len(lines):
        line = lines[i]
        if not re.match(r"^\s*#\s*define\s+", line):
            i += 1
            continue
        parts = [line.rstrip()]
        while parts[-1].endswith("\\") and i + 1 < len(lines):
            parts[-1] = parts[-1][:-1]
            i += 1
            parts.append(lines[i].strip())
        definition = " ".join(parts)
        match = re.match(
            r"^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\s*"
            r"\(([^)]*)\)\s*(.*)$",
            definition,
        )
        i += 1
        if not match:
            continue
        macro, params_text, body = match.groups()
        params = [param.strip() for param in params_text.split(",")]
        if not params or not re.search(
            r"\b(?:u8|u16|u32|s8|s16|s32|int|void|bool|char|long|short)\s+"
            + re.escape(params[0]) + r"\s*\(",
            body,
        ):
            continue
        invocation = re.compile(
            r"\b" + re.escape(macro) +
            r"\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,"
        )
        for source_line in lines:
            if source_line.lstrip().startswith("#"):
                continue
            call = invocation.search(source_line)
            if call:
                generated.add(call.group(1))
    return generated


def parse_c(src_dir):
    """Return (defined: {name: 'real'|'todo'|'weak'}, aliases: {name: target})."""
    defined, aliases = {}, {}
    for path in sorted(glob.glob(os.path.join(src_dir, "*.c"))):
        with open(path, "r", errors="replace") as f:
            text = f.read()
        text = strip_host_ifdefs(text)
        lines = text.splitlines(keepends=True)
        # alias attributes may wrap across lines; scan comment-stripped text
        for m in ALIAS_RE.finditer(strip_comments(text)):
            aliases[m.group(1)] = m.group(2)
        for i, line in enumerate(lines):
            fm = FUNC_RE.match(line)
            if not fm or fm.group(1) in KEYWORDS:
                continue
            # declaration (ends with ';' before any '{') is not a definition
            rest = "".join(lines[i:i + 4])
            if "{" not in rest.split(";")[0] and ";" in rest.split("{")[0]:
                continue
            name = fm.group(1)
            if "_TODO" in name:
                defined[name] = "todo"
            elif attr_applies_to_defn(lines, i):
                defined[name] = "weak"
            else:
                # Prefer a real body over a later weak host stub with the same
                # name (should be rare after strip_host_ifdefs).
                if defined.get(name) == "real":
                    continue
                defined[name] = "real"
        for name in macro_defined_functions(text):
            if "_TODO" not in name:
                defined[name] = "real"
    return defined, aliases


def resolve(name, defined, aliases, depth=0):
    """Follow alias chains to the underlying definition kind.

    Names are matched case-insensitively on the VMA-bearing part:
    gas/asm spellings mix `_0800A60` and `_0800a60`.
    """
    if depth > 8:
        return "cycle"
    key = name
    if key not in defined and key not in aliases:
        # case-insensitive retry for VMA-shaped names
        v = norm_vma(name)
        if v:
            for k in defined:
                if norm_vma(k) == v:
                    key = k
                    break
            else:
                for k in aliases:
                    if norm_vma(k) == v:
                        key = k
                        break
    if key in aliases:
        return resolve(aliases[key], defined, aliases, depth + 1)
    return defined.get(key, "missing")


LABEL_RE = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*):(.*)$")
DATA_DIRS = (".4byte", ".2byte", ".byte", ".short", ".long", ".word", ".hword", ".align", ".incbin", ".ascii")


# The operand is EITHER a symbolic name OR a numeric `0x0800XXXX` target. This
# used to be `[A-Za-z_][A-Za-z0-9_]*` alone, which starts with a letter or
# underscore and therefore can NEVER match `0x08009FFC` -- so every numeric `bl`
# was invisible to the inventory, and the `if v: called.add(v)` branch below
# ("direct VMA-shaped target") was dead code. The symptom is a label that is
# present in asm/ and bl-ed from four places, yet absent from rom_functions():
# `0x08009FFC` and `0x0800A1A4` in carphys_tick.s are both labelled (lines 493
# and 768) and both invisible, because that file has 220 bare `^_0800XXXX:`
# labels and only 2 `.type` lines, so the typed|called union dropped them.
BL_RE = re.compile(r"\bbl\s+([A-Za-z_][A-Za-z0-9_]*|0x[0-9A-Fa-f]+)")
B_RE = re.compile(r"\bb(?:\.[a-z]+)?\s+([A-Za-z_][A-Za-z0-9_]*)")


def asm_vmas(asm_dir):
    """Function VMAs per file.

    A label counts as a function entry when it has hard call/declare
    evidence: a `.type NAME,%function` line, or it is the target of a `bl`
    from any asm file (interior loop labels are only ever `b` targets).
    Pool/data labels are excluded.
    """
    typed = set()      # VMAs declared %function
    called = set()     # VMAs targeted by bl
    defined = {}       # vma -> file that defines the label
    files = []
    for path in sorted(glob.glob(os.path.join(asm_dir, "*.s"))) + sorted(glob.glob(os.path.join(asm_dir, "*.inc"))):
        name = os.path.basename(path)
        if name == "rom.s":
            continue
        files.append((name, path))
        with open(path, "r", errors="replace") as f:
            text = f.read()
        lines = text.splitlines()
        # strip comments so pool words / remarks can't look like code refs
        code = re.sub(r"@.*$", "", text, flags=re.M)
        for line in lines:
            m = TYPE_RE.match(line)
            if m:
                v = norm_vma(m.group(1))
                if v:
                    typed.add(v)
            m = LABEL_RE.match(line)
            if m:
                v = norm_vma(m.group(1))
                if v:
                    defined.setdefault(v, name)
        for m in BL_RE.finditer(code):
            pass  # bl targets resolved below via defined labels
        # map bl target labels to VMAs using the defined-label set
        for m in BL_RE.finditer(code):
            lab = m.group(1)
            # direct VMA-shaped target (numeric bl 0x0800XXXX)
            v = norm_vma(lab)
            if v:
                called.add(v)
                continue
            # symbolic: resolve later (second pass)
            sym_targets.setdefault(lab, name)

    # second pass: resolve symbolic bl labels to the file/label they hit
    label_to_vma = {}
    for name, path in files:
        with open(path, "r", errors="replace") as f:
            for line in f:
                m = LABEL_RE.match(line)
                if m:
                    v = norm_vma(m.group(1))
                    if v:
                        label_to_vma.setdefault(m.group(1), v)
    for lab in sym_targets:
        v = label_to_vma.get(lab)
        if v:
            called.add(v)

    per_file = {}
    for v in typed | called:
        fname = defined.get(v)
        if fname:
            per_file.setdefault(fname, set()).add(v)
    return per_file


sym_targets = {}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src-dir", default="src")
    ap.add_argument("--asm-dir", default="asm")
    ap.add_argument("--top", type=int, default=25)
    ap.add_argument("--file", help="list per-VMA detail for one asm file")
    args = ap.parse_args()

    defined, aliases = parse_c(args.src_dir)
    per_file = asm_vmas(args.asm_dir)
    all_asm = set().union(*per_file.values()) if per_file else set()

    # index every C-side name by the VMA it embeds: _08002430/sub_08002430
    # /Sub_08002430 all map to vma 002430
    names_by_vma = {}
    for name in list(defined) + list(aliases):
        v = norm_vma(name)
        if v:
            names_by_vma.setdefault(v, set()).add(name)

    def covered(v):
        for name in names_by_vma.get(v, ()):
            if resolve(name, defined, aliases) == "real":
                return True
        return False

    covered_all = {v for v in all_asm if covered(v)}

    if args.file:
        vmas = sorted(per_file.get(args.file, ()), key=lambda x: int(x, 16))
        print(f"{args.file}: {len(vmas)} function VMAs")
        for v in vmas:
            mark = "OK  " if v in covered_all else "GAP "
            kind = resolve(v, defined, aliases)
            print(f"  {mark} 0x08{v}  ({kind})")
        return 0

    total = len(all_asm)
    ncov = len(covered_all)
    ntodo = sum(1 for k in defined.values() if k == "todo")
    nweak = sum(1 for k in defined.values() if k == "weak")
    print(f"asm unique function VMAs:      {total}")
    print(f"evidence-backed covered:       {ncov} ({ncov * 100 // total}%)")
    print(f"C definitions: real={sum(1 for k in defined.values() if k == 'real')} "
          f"todo={ntodo} weak={nweak}  alias chains={len(aliases)}")
    print()
    rows = []
    for name, vmas in per_file.items():
        cov = sum(1 for v in vmas if v in covered_all)
        rows.append((len(vmas) - cov, len(vmas), cov, name))
    rows.sort(reverse=True)
    print(f"{'gap':>5} {'tot':>4} {'cov':>4}  asm file")
    shown = 0
    for gap, tot, cov, name in rows:
        if gap == 0 or shown >= args.top:
            break
        print(f"{gap:>5} {tot:>4} {cov:>4}  {name}")
        shown += 1
    full = sum(1 for r in rows if r[0] == 0)
    print(f"\nFULL clusters: {full} / {len(rows)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
