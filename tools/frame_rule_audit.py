#!/usr/bin/env python3
"""Check tracker rows for frames the matching compiler cannot emit.

Flags non-leaf bodies without an lr save and pushes of low registers without
lr. Other KEEP reasons need separate review.

Usage: python3 tools/frame_rule_audit.py [--doc NAKED_TO_C.md] [--json]
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

ROW_TODO = re.compile(r"^\| `([^`]+)` \| (\d+) \| `([^`]+)` \|")
ROW_KEEP = re.compile(r"^\| `([^`]+)` \| `([^`]+):(\d+)(?:-(\d+))?` \| (.+) \|")

FRAME_CITE = "thumb.c:785"


def naked_region(path: Path, start: int) -> str:
    """Body text of the naked function declared at or after `start`."""
    lines = path.read_text(encoding="utf-8", errors="replace").split("\n")
    i = start - 1
    while i < len(lines) and "naked" not in lines[i]:
        i += 1
    body = []
    while i < len(lines):
        if lines[i].strip() == "}":
            break
        body.append(lines[i])
        i += 1
    return "\n".join(body)


# `push {r4, r5}` and its escaped spelling `"push\t{r4, r5}\n"` both occur.
PUSH_TEXT = re.compile(r"push(?:\\t|\\n|\s)*\{([^}]*)\}")
HWORD = re.compile(r"\.hword\s+0x([0-9a-fA-F]+)")
# A `bl`/`blx` call.  `bx` is a return, not a call.
CALL = re.compile(r"(?:^|[\s\"])bl(?:x)?[\s\t]")


def push_lists(body: str) -> list[list[str]]:
    """Register lists of every `push`, textual or `.hword`-encoded."""
    lists = [[r.strip() for r in m.group(1).split(",") if r.strip()]
             for m in PUSH_TEXT.finditer(body)]
    # `PUSH {Rlist}` is one 16-bit Thumb halfword: `1011 010M RRRRRRRR`, so
    # 0xB5xx carries lr (the M bit) and 0xB4xx does not, and the low byte is the
    # register list.  Nothing else in Thumb starts with 0xB4/0xB5.
    for tok in (t.lower() for t in HWORD.findall(body)):
        if len(tok) != 4 or not tok.startswith("b"):
            continue
        val = int(tok, 16)
        if not 0xB400 <= val <= 0xB5FF:
            continue
        rlist = val & 0xFF
        regs = [f"r{r}" for r in range(8) if rlist & (1 << r)]
        if val & 0x0100:
            regs.append("lr")
        lists.append(regs)
    return lists


def has_call(body: str) -> bool:
    return CALL.search(body) is not None or re.search(r"\bmov\s+ip,\s*lr", body) is not None


def classify(rel: str, srcline: int) -> str | None:
    """'A', 'B', or None for a row's naked body."""
    src = ROOT / rel
    if not src.exists():
        return None
    body = naked_region(src, srcline)
    lists = push_lists(body)
    if not lists:
        return "A" if has_call(body) else None
    return "B" if all("lr" not in regs for regs in lists) else None


def audit(doc: Path) -> dict:
    """Blocked TODO rows (should be none) and frame-cited KEEP rows (the set)."""
    lines = doc.read_text(encoding="utf-8").split("\n")
    section = None
    out = {"todo_rows": 0, "keep_rows": 0, "todo_blocked": [], "keep_cited": [],
           "keep_other": []}
    for line in lines:
        if line.startswith("## TODO"):
            section = "todo"
            continue
        if line.startswith("## KEEP"):
            section = "keep"
            continue
        if line.startswith("## "):
            section = None
            continue
        if section == "todo":
            m = ROW_TODO.match(line)
            if not m:
                continue
            out["todo_rows"] += 1
            rel, srcline, sym = m.group(1), int(m.group(2)), m.group(3)
            shape = classify(rel, srcline)
            if shape:
                out["todo_blocked"].append((rel, srcline, sym, shape))
        elif section == "keep":
            m = ROW_KEEP.match(line)
            if not m:
                continue
            out["keep_rows"] += 1
            sym, rel, srcline, why = m.group(1), m.group(2), int(m.group(3)), m.group(5)
            shape = classify(rel, srcline)
            if not shape:
                continue
            entry = (rel, srcline, sym, shape, why)
            if FRAME_CITE in line:
                out["keep_cited"].append(entry)
            else:
                out["keep_other"].append(entry)
    return out


def _check(name, got, want):
    ok = got == want
    print(f"  {'PASS' if ok else 'FAIL'} {name}: got {got!r} want {want!r}")
    return ok


# Paired examples cover blocked and allowed frame shapes.
FIXTURES = [
    ("call with no push is shape A",
     '    "mov  ip, lr\\n"\n    "bl   0x0802BCE8\\n"\n    "bx   ip\\n"', "A"),
    ("escaped-tab push {r4, r5} with no lr is shape B",
     '    "push\\t{r4, r5}\\n"\n    "pop\\t{r4, r5}\\n"\n    "bx\\tlr\\n"', "B"),
    ("push {r4, r5, lr} is not blocked (that is agbcc's own frame)",
     '    "push\\t{r4, r5, lr}\\n"\n    "bl   0x0802BCE8\\n"', None),
    ("leaf with no frame at all is not blocked",
     '    "movs r0, #1\\n"\n    "bx   lr\\n"', None),
    ("`bx` alone is a return, not a call",
     '    "push {r4, lr}\\n"\n    "bx  lr\\n"', None),
    (("a bare push {lr} non-leaf is not blocked (mask is lr only)"),
     '    "push {lr}\\n"\n    "bl   0x0802BCE8\\n"', None),
]


def self_test() -> int:
    """Fixture shapes, then one live regression on the tracking doc."""
    ok = True
    for name, body, want in FIXTURES:
        lists = push_lists(body)
        if not lists:
            got = "A" if has_call(body) else None
        else:
            got = "B" if all("lr" not in regs for regs in lists) else None
        ok &= _check(name, got, want)

    # Support pushes encoded as Thumb halfwords.
    ok &= _check("`.hword`-encoded push is decoded",
                 push_lists('  ".hword 0xb5f0\\n"\n  ".hword 0x4657\\n"'),
                 [["r4", "r5", "r6", "r7", "lr"]])

    doc = ROOT / "NAKED_TO_C.md"
    if doc.exists():
        res = audit(doc)
        ok &= _check("no TODO row carries an unreachable frame",
                     res["todo_blocked"], [])
        ok &= _check("the frame rule is cited by exactly the blocked rows",
                     len(res["keep_cited"]), 16)
        ok &= _check("every cited KEEP row really has a blocked shape",
                     all(shape in ("A", "B")
                         for _r, _l, _s, shape, _w in res["keep_cited"]), True)
    else:
        print("  SKIP live regression (NAKED_TO_C.md absent)")

    print(f"frame_rule_audit self-test: {'PASS' if ok else 'FAIL'}")
    return 0 if ok else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--doc", default=str(ROOT / "NAKED_TO_C.md"))
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        return self_test()

    doc = Path(args.doc)
    if not doc.exists():
        print(f"frame_rule_audit: no such tracking doc: {doc}", file=sys.stderr)
        return 2
    res = audit(doc)

    if args.json:
        print(json.dumps(res, indent=2))
        return 0

    print(f"TODO rows: {res['todo_rows']}   KEEP rows: {res['keep_rows']}")
    print(f"\nKEEP rows citing the frame rule: {len(res['keep_cited'])}")
    for rel, srcline, sym, shape, _why in res["keep_cited"]:
        print(f"  shape {shape}  {rel}:{srcline}  {sym}")
    if res["keep_other"]:
        print(f"\nKEEP rows with the same shape but a different stated reason "
              f"({len(res['keep_other'])}) — review that the reason covers them:")
        for rel, srcline, sym, shape, why in res["keep_other"]:
            print(f"  shape {shape}  {rel}:{srcline}  {sym}\n      reason: {why}")
    print(f"\nTODO rows still carrying an unreachable frame: "
          f"{len(res['todo_blocked'])} (expected 0)")
    for rel, srcline, sym, shape, _why in res["todo_blocked"]:
        print(f"  shape {shape}  {rel}:{srcline}  {sym}")
    return 1 if res["todo_blocked"] else 0


if __name__ == "__main__":
    sys.exit(main())
