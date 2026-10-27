#!/usr/bin/env python3
"""Find bodies that contain a Thumb instruction idiom.

Some instructions are only reachable from one source spelling (a bound held in a
variable keeps `cmp #128` uncanonicalised, for example), so knowing which bodies
contain an idiom is how a lift target is chosen. The spans and the
decompiled/not-decompiled classification come from
`docs/data/source_quality.json`, so this reports exactly the bodies that are
still assembly.

Words that are the target of a PC-relative load are literal-pool data, not
instructions: a pool word such as 0x4200xxxx decodes as `tst` and would
otherwise report a body as containing an instruction it never executes.

Usage:
    python3 tools/thumb_idiom_scan.py                  # cmp #128 + bcs/bcc class
    python3 tools/thumb_idiom_scan.py --idiom tst      # bodies containing `tst`
    python3 tools/thumb_idiom_scan.py --exclude tst    # ...and no `tst`
    python3 tools/thumb_idiom_scan.py --all            # include lifted bodies
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ROM = ROOT / "baserom.gba"
QUALITY = ROOT / "docs" / "data" / "source_quality.json"
ROM_BASE = 0x08000000
NOT_DECOMPILED = {"asm", "c_fakematch"}


def is_cmp_imm(hw: int, imm: int | None = None) -> bool:
    return (hw & 0xF800) == 0x2800 and (imm is None or hw & 0xFF == imm)


def is_cond_branch(hw: int, cond: int | None = None) -> bool:
    return (hw & 0xF000) == 0xD000 and (hw & 0x0F00) != 0x0E00 and \
        (cond is None or (hw >> 8) & 0xF == cond)


def is_tst(hw: int) -> bool:
    return (hw & 0xFFC0) == 0x4200


# name -> list of predicates that must hold for consecutive halfwords
IDIOMS = {
    "cmp128": [lambda h: is_cmp_imm(h, 0x80),
               lambda h: is_cond_branch(h, 0x2)],   # bcs
    "cmp128any": [lambda h: is_cmp_imm(h, 0x80),
                  lambda h: is_cond_branch(h)],      # any conditional branch
    "tst": [is_tst],
}


def pool_words(rom: bytes, off: int, size: int) -> set[int]:
    """Byte offsets (relative to `off`) of words loaded PC-relative."""
    data = set()
    for i in range(0, size & ~1, 2):
        hw = struct.unpack_from("<H", rom, off + i)[0]
        if (hw & 0xF800) == 0x4800:           # ldr rX, [pc, #imm]
            target = ((off + i + 4) & ~3) + (hw & 0xFF) * 4
            for k in range(4):
                data.add(target - off + k)
    return data


def scan(rom: bytes, off: int, size: int, rules) -> int:
    skip = pool_words(rom, off, size)
    hws = [struct.unpack_from("<H", rom, off + i)[0]
           for i in range(0, size & ~1, 2) if i not in skip]
    return sum(1 for i in range(len(hws) - len(rules) + 1)
               if all(rule(hws[i + k]) for k, rule in enumerate(rules)))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--idiom", default="cmp128", choices=sorted(IDIOMS))
    ap.add_argument("--exclude", default=None, choices=sorted(IDIOMS))
    ap.add_argument("--all", action="store_true", help="include lifted bodies")
    ap.add_argument("--limit", type=int, default=25)
    args = ap.parse_args()

    rom = ROM.read_bytes()
    funcs = json.loads(QUALITY.read_text())["functions"]
    rules = IDIOMS[args.idiom]
    excl = IDIOMS[args.exclude] if args.exclude else None

    rows = []
    for vma, info in funcs.items():
        if not args.all and info["class"] not in NOT_DECOMPILED:
            continue
        off = int(vma, 16) - ROM_BASE
        size = int(info["size"])
        n = scan(rom, off, size, rules)
        if n and not (excl and scan(rom, off, size, excl)):
            rows.append((size, n, info["name"], off))

    rows.sort()
    for size, n, name, off in rows[:args.limit]:
        print(f"0x{off + ROM_BASE:08x} {size:5d} B  x{n}  {name}")
    print(f"-- {len(rows)} bodies contain `{args.idiom}`"
          + (f" and no `{args.exclude}`" if args.exclude else ""), file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
