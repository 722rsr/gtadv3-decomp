#!/usr/bin/env python3
"""arity_audit.py - detect C call sites that pass fewer arguments than the
ROM callee actually consumes.

Why this exists
---------------
The lifted C declares extern prototypes by hand.  When a hand-written prototype
is *shorter* than the real ROM callee, the C compiler emits fewer outgoing
stack words than the callee reads -- so the callee sees garbage for its high
arguments.  The ARM build cannot see this (the declaration is self-consistent)
and only a byte-compare or a driven runtime path catches it.  Several real bugs of this shape were found by hand in
`src/rec35_mid_region.c` (`_08007B18`, `_08007BFC`, `_08007C68`, `_08007770`,
`_08007570`, `_08002ED0`).

How the true arity is derived
-----------------------------
The ROM is ARM ADS/armcc Thumb-1 with a deterministic prologue:

    push {r4, r5, r6, r7, lr}   /  push {lr}          (low regs + lr)
    mov  r7, r8  /  mov r6, r9  ...                   (high-reg save dance)
    push {r5, r6, r7}                                 (high-reg saves)
    sub  sp, #N                                       (locals)

Every incoming stack argument sits at `[sp + disp]`, `[sp + disp + 4]`, ...
where `disp` is the total byte displacement of that prologue.  So

    stack_words = (max_read_offset - disp) / 4 + 1     (when max_read >= disp)
    arity       = 4 + stack_words

where `max_read_offset` is the largest `[sp, #N]` *load* offset in the body.
A function that never loads from `[sp, #N >= disp]` takes no stack arguments and
its arity cannot be pinned above 4 -- those are reported as `unknown`.

Usage
-----
    python3 tools/arity_audit.py                 # whole tree, under-arity only
    python3 tools/arity_audit.py --all           # include over-arity too
    python3 tools/arity_audit.py --symbol 08007BFC
    python3 tools/arity_audit.py --table         # ROM arity table for every fn
    python3 tools/arity_audit.py --rom-arity 08007570

Exit status is 1 when any under-arity call site is found, so it can be used as
a gate.

Limitations
-----------
* Only functions whose *own* prologue is intact are measured; a function whose
  body was split across two asm files without its prologue is reported unknown.
* `bl`-into-the-middle entries inherit the enclosing prologue, which is the
  intent (the interior label is reached with the same incoming frame).
* Argument counting is textual: macro-wrapped or multi-line calls whose text
  does not spell the callee name directly are skipped.
"""

import argparse
import os
import re
import sys
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASM = os.path.join(ROOT, "asm")
SRC = os.path.join(ROOT, "src")
INCLUDE = os.path.join(ROOT, "include")

VMA_RE = re.compile(r"^_?0*80([0-9A-Fa-f]{2,6})$")

# --- prologue recognition ---------------------------------------------------
PUSH_RE = re.compile(r"^\s*push\s*\{([^}]*)\}")
SUBSP_RE = re.compile(r"^\s*sub\s+sp,\s*#?(\d+)")
ADJSP_RE = re.compile(r"^\s*add\s+sp,\s*#?(\d+)")
# The armcc high-register save dance.  Disassemblers print r8-r12 either as
# `r8`..`r12` or as the aliases `sl`/`fp`/`ip`, so accept both spellings.
SAVEDANCE_RE = re.compile(
    r"^\s*mov\s+r[0-7],\s*(?:r(?:[89]|1[01])|sl|fp|ip|r12)\s*$")
LABEL_RE = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*):")
TYPE_RE = re.compile(r"^\s*\.type\s+([A-Za-z_][A-Za-z0-9_]*),\s*%function")
SPLOAD_RE = re.compile(r"^\s*(ldr|ldrb|ldrh|ldrsh|ldrsb|ldrsb)\s+r\d+,\s*\[sp,\s*#(\d+)\]")
SPANY_RE = re.compile(r"\[sp,\s*#(\d+)\]")
# armcc emits `mov rN, sp` + `ldr rM, [rN, #off]` for stack-word loads the
# literal `[sp, #off]` form cannot spell (e.g. _08002ED0's 10th word via
# `mov r0, sp` / `ldrh r0, [r0, #56]`). Only the adjacent pair is accepted:
# any arithmetic on rN between the copy and the load would break the
# equivalence, so a wider window would be unsound.
MOVSP_RE = re.compile(r"^\s*mov\s+(r\d+),\s*sp\s*$")
SPREGLOAD_RE = re.compile(r"^\s*(ldr|ldrb|ldrh|ldrsh|ldrsb)\s+r\d+,\s*\[(r\d+),\s*#(\d+)\]")


def parse_int(tok):
    tok = tok.strip()
    if tok.startswith("#"):
        tok = tok[1:]
    try:
        return int(tok, 0)
    except ValueError:
        return None


def prologue_disp(lines):
    """Total byte displacement of the leading push/sub sp sequence."""
    disp = 0
    for ln in lines:
        m = PUSH_RE.match(ln)
        if m:
            regs = [r.strip() for r in m.group(1).split(",") if r.strip()]
            if not regs:
                return None
            disp += 4 * len(regs)
            continue
        m = SUBSP_RE.match(ln)
        if m:
            disp += int(m.group(1))
            continue
        if SAVEDANCE_RE.match(ln):
            continue
        if not ln.strip():
            continue
        # first instruction that is not prologue
        return disp
    return None


def scan_asm(path):
    """Yield (name, disp, max_load_offset, max_any_offset) per function."""
    try:
        text = open(path, "r", errors="replace").read()
    except OSError:
        return
    lines = text.splitlines()

    def next_code(j):
        while j < len(lines) and not lines[j].strip():
            j += 1
        return lines[j] if j < len(lines) else ""

    # Function starts: a `.type NAME, %function` directive, or a bare VMA
    # label whose next instruction is a prologue.  Interior branch labels
    # (`_08007C26:` mid-body) are followed by ordinary instructions, so they
    # are correctly not treated as new functions -- otherwise the body would
    # be truncated before its incoming-stack-argument loads.
    starts = []
    for i, ln in enumerate(lines):
        m = TYPE_RE.match(ln)
        if m:
            starts.append((i, m.group(1)))
            continue
        m = LABEL_RE.match(ln)
        if m and VMA_RE.match(m.group(1)):
            nxt = next_code(i + 1)
            if PUSH_RE.match(nxt) or SUBSP_RE.match(nxt):
                starts.append((i, m.group(1)))
    seen = set()
    for idx, (i, name) in enumerate(starts):
        end = starts[idx + 1][0] if idx + 1 < len(starts) else len(lines)
        body = lines[i + 1:end]
        # Drop leading label-only lines.  Every converted function carries two
        # (`sub_0800798C:` then `_0800798C:`); without this the prologue scan
        # would stop immediately and report disp=0.
        while body and LABEL_RE.match(body[0]) and body[0].rstrip().endswith(":"):
            body = body[1:]
        if not body:
            continue
        disp = prologue_disp(body)
        if disp is None:
            continue
        max_load = None
        max_any = None
        for k, ln in enumerate(body):
            if TYPE_RE.match(ln) or LABEL_RE.match(ln):
                pass
            m = SPLOAD_RE.match(ln)
            if m:
                off = int(m.group(2))
                if max_load is None or off > max_load:
                    max_load = off
            m = MOVSP_RE.match(ln)
            if m and k + 1 < len(body):
                m2 = SPREGLOAD_RE.match(body[k + 1])
                if m2 and m2.group(2) == m.group(1):
                    off = int(m2.group(3))
                    if max_load is None or off > max_load:
                        max_load = off
            for m2 in SPANY_RE.finditer(ln):
                off = int(m2.group(1))
                if max_any is None or off > max_any:
                    max_any = off
        norm = norm_vma(name)
        key = (norm, disp)
        if key in seen:
            continue
        seen.add(key)
        yield norm, name, disp, max_load, max_any


def norm_vma(name):
    """Map sub_08007BFC / _08007BFC / _08007bfc to 08007bfc."""
    s = name.lstrip("_")
    if s.lower().startswith("sub_"):
        s = s[4:]
    s = s.lstrip("_")
    m = re.match(r"^0*([0-9A-Fa-f]{6,8})$", s)
    if not m:
        return None
    v = m.group(1).lower()
    return v[-6:].rjust(6, "0")


def rom_arity_table():
    """{norm_vma: arity} for every asm function whose arity is pinnable."""
    best = {}
    for fn in sorted(os.listdir(ASM)):
        if not fn.endswith(".s"):
            continue
        for norm, name, disp, max_load, max_any in scan_asm(os.path.join(ASM, fn)):
            if norm is None or disp is None:
                continue
            if disp <= 0 or max_load is None or max_load < disp:
                # disp == 0 means no prologue was recognised at all (raw / mixed-ISA
                # span, or a function split away from its prologue) -- the incoming
                # argument area is then unknown, so claim nothing.
                arity = None
            else:
                arity = 4 + (max_load - disp) // 4 + 1
            prev = best.get(norm)
            if prev is None or (prev[0] is None and arity is not None):
                names = set() if prev is None else set(prev[5])
                best[norm] = (arity, disp, max_load, max_any, name, names)
            elif prev is not None:
                best[norm][5].add(name)
    for norm, row in best.items():
        if row[4]:
            row[5].add(row[4])
    return best


HEX_RE = re.compile(r"([0-9A-Fa-f]{5,8})$")


def spellings(name):
    """Every identifier spelling the tree uses for one VMA symbol."""
    out = {name}
    m = HEX_RE.search(name)
    if not m:
        return out
    hexpart = m.group(1)
    for h in {hexpart, hexpart.lower(), hexpart.upper()}:
        for pre in ("", "_", "sub_", "Sub_", "sub_0", "Sub_0"):
            out.add(pre + h)
    # canonical 8-digit `sub_0800XXXX` / `_0800XXXX` forms
    pad = hexpart.rjust(8, "0")
    for h in {pad, pad.lower(), pad.upper()}:
        for pre in ("_", "sub_", "Sub_"):
            out.add(pre + h)
    return {s for s in out if s}


# --- C call site extraction -------------------------------------------------
COMMENT_RE = re.compile(r"//[^\n]*|/\*.*?\*/", re.S)
IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
CALL_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(")


def strip_comments_and_literals(text):
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            i = n if j < 0 else j
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        if c == '"':
            j = i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == '"':
                    break
                j += 1
            out.append('""')
            i = j + 1
            continue
        if c == "'":
            j = i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == "'":
                    break
                j += 1
            out.append("''")
            i = j + 1
            continue
        out.append(c)
        i += 1
    return "".join(out)


def count_args(text, open_paren):
    """Count top-level comma-separated arguments of a call at `open_paren`."""
    i = open_paren + 1
    depth = 0
    args = []
    cur = []
    n = len(text)
    while i < n:
        c = text[i]
        if c in "([{":
            depth += 1
            cur.append(c)
        elif c in ")]}":
            if depth == 0:
                break
            depth -= 1
            cur.append(c)
        elif c == "," and depth == 0:
            args.append("".join(cur))
            cur = []
        else:
            cur.append(c)
        i += 1
    args.append("".join(cur))
    args = [a.strip() for a in args]
    if len(args) == 1 and args[0] == "":
        return 0, args
    return len(args), args


FUNC_DEF_RE = re.compile(
    r"^\s*(?:[A-Za-z_][A-Za-z0-9_ \t\*]*?)\b([A-Za-z_][A-Za-z0-9_]*)\s*\(",
    re.M)


def iter_c_files(roots):
    for root in roots:
        if not os.path.isdir(root):
            continue
        for dirpath, _dirs, files in os.walk(root):
            for f in sorted(files):
                if f.endswith(".c") or f.endswith(".h"):
                    yield os.path.join(dirpath, f)


TYPE_WORDS = {
    "extern", "static", "inline", "void", "int", "unsigned", "signed",
    "char", "short", "long", "const", "volatile", "restrict", "register",
    "u8", "u16", "u32", "u64", "s8", "s16", "s32", "s64", "f32", "f64",
    "size_t", "ssize_t", "uintptr_t", "intptr_t", "bool", "float", "double",
    "__attribute__", "__attribute", "noreturn", "unused", "weak", "naked",
}
STMT_BOUNDARY = ";{}\n"


def is_declaration(prefix):
    """True when `prefix` (text since the statement start) is a type-only
    declaration head, e.g. `extern void  ` or `void *`."""
    p = prefix.strip()
    if not p:
        return False
    if not re.match(r"^[A-Za-z_0-9\s\*\[\]]*$", p):
        return False
    toks = re.findall(r"[A-Za-z_][A-Za-z0-9_]*", p)
    return bool(toks) and all(t in TYPE_WORDS for t in toks)


# A declaration wrapped in an ALL-CAPS function-like macro, e.g.
#   HOST_STUB(void _08007BFC(void *a, u8 b, ...));
# The inner identifier looks like a call to a naive scanner, so such sites are
# skipped explicitly.
MACRO_WRAP_RE = re.compile(r"[A-Z][A-Z0-9_]{2,}\s*\([^()]*$")


def find_call_sites(text):
    """Yield (name, line, n_args, args) for every *call* (not declaration)."""
    for m in CALL_RE.finditer(text):
        name = m.group(1)
        if name in ("if", "while", "for", "switch", "return", "sizeof",
                    "defined", "do"):
            continue
        # statement start = last boundary before the identifier
        j = m.start() - 1
        while j >= 0 and text[j] not in STMT_BOUNDARY:
            j -= 1
        prefix = text[j + 1:m.start()]
        if is_declaration(prefix):
            continue
        if MACRO_WRAP_RE.search(prefix):
            continue
        ln = text.count("\n", 0, m.start()) + 1
        n_args, args = count_args(text, m.end() - 1)
        yield name, ln, n_args, args


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--symbol", help="only this VMA (e.g. 08007BFC)")
    ap.add_argument("--all", action="store_true",
                    help="also report over-arity call sites")
    ap.add_argument("--table", action="store_true",
                    help="print the derived ROM arity table and exit")
    ap.add_argument("--rom-arity", help="print the derived arity for one VMA")
    ap.add_argument("--roots", nargs="*", default=[SRC, INCLUDE])
    args = ap.parse_args()

    table = rom_arity_table()

    if args.table:
        for k in sorted(table):
            a, disp, ml, ma = table[k][:4]
            print(f"{k}  arity={a if a is not None else 'unknown':>7}  "
                  f"disp={disp:>3}  max_load={ml}  max_any={ma}")
        return 0

    if args.rom_arity:
        a = table.get(norm_vma(args.rom_arity))
        if a is None:
            print(f"{args.rom_arity}: not found")
            return 1
        print(f"{args.rom_arity}: arity={a[0]} disp={a[1]} max_load={a[2]}")
        return 0

    want = norm_vma(args.symbol) if args.symbol else None

    # name -> arity for every spelling that maps to a VMA
    by_name = {}
    for k, row in table.items():
        a = row[0]
        for orig in (row[4],) + tuple(row[5]):
            for sp in spellings(orig):
                if sp not in by_name or (by_name[sp] is None and a is not None):
                    by_name[sp] = a
        by_name.setdefault(k, a)
        by_name.setdefault(k.upper(), a)

    under = defaultdict(list)
    over = defaultdict(list)
    for path in iter_c_files(args.roots):
        raw = open(path, errors="replace").read()
        text = strip_comments_and_literals(raw)
        rel = os.path.relpath(path, ROOT)
        for name, ln, n_args, _args in find_call_sites(text):
            if name not in by_name:
                continue
            arity = by_name[name]
            if arity is None:
                continue
            if want is not None and norm_vma(name) != want:
                continue
            if n_args < arity:
                under[name].append((rel, ln, n_args, arity))
            elif n_args > arity and args.all:
                over[name].append((rel, ln, n_args, arity))

    total = 0
    for name in sorted(under):
        rows = under[name]
        total += len(rows)
        print(f"\n### {name}: ROM arity {rows[0][3]}, {len(rows)} under-arity site(s)")
        for rel, ln, n_args, arity in rows:
            print(f"    {rel}:{ln}  passes {n_args} (< {arity})")
    if args.all:
        for name in sorted(over):
            rows = over[name]
            print(f"\n### {name}: ROM arity {rows[0][3]}, {len(rows)} over-arity site(s)")
            for rel, ln, n_args, arity in rows:
                print(f"    {rel}:{ln}  passes {n_args} (> {arity})")

    if total:
        print(f"\nUNDER-ARITY CALL SITES: {total}")
        return 1
    print("OK: no under-arity call sites against sharpened ROM arities")
    return 0


if __name__ == "__main__":
    sys.exit(main())
