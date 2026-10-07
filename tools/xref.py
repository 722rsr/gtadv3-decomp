#!/usr/bin/env python3
"""Cross-reference scanner for GBA ROMs.

Finds every static reference to an address in a binary:
  * Thumb `bl` call sites whose computed target equals the address
  * literal-pool words holding the address (optionally with bit0 set,
    i.e. the interworking Thumb form)

Stdlib only. Offsets are FILE offsets; pass --vma to print bus addresses
(0x08xxxxxx) instead.

Usage:
  python3 tools/xref.py baserom.gba 0x8002430            # all refs to one addr
  python3 tools/xref.py baserom.gba 0x5BAB0 0x5BAE0      # several addrs
  python3 tools/xref.py baserom.gba --calls-only 0x8004D4C
  python3 tools/xref.py baserom.gba --lits-only  0x80CB298
"""
import argparse
import struct
import sys


def read_rom(path):
    with open(path, "rb") as f:
        return f.read()


def thumb_bl_targets(rom):
    """Yield (site_off, target_off) for every decodable Thumb BL pair.

    Decodes unconditionally; bogus pairs decode to garbage targets, so
    callers should filter on the target they care about.
    """
    n = len(rom)
    for off in range(0, n - 2, 2):
        h1 = rom[off] | (rom[off + 1] << 8)
        if (h1 & 0xF800) != 0xF000:
            continue
        h2 = rom[off + 2] | (rom[off + 3] << 8)
        if (h2 & 0xF800) != 0xF800:
            continue
        hi = h1 & 0x7FF
        lo = h2 & 0x7FF
        disp = (hi << 12) | (lo << 1)
        if disp & 0x400000:          # 23-bit signed
            disp -= 0x800000
        pc = off + 4                 # site VMA + 4 == file off + 4
        yield off, pc + disp


def scan(rom, targets, calls_only=False, lits_only=False):
    """Return {target_fileoff: {'calls': [sites], 'lits': [lit_offsets]}}."""
    want = set(targets)
    out = {t: {"calls": [], "lits": []} for t in want}

    if not lits_only:
        for site, tgt in thumb_bl_targets(rom):
            if tgt in out:
                out[tgt]["calls"].append(site)

    if not calls_only:
        # literal pools store bus-address (VMA) words: 0x08xxxxxx, with
        # bit0 set for Thumb interworking targets
        litvals = {}
        for t in want:
            base = t + 0x08000000
            for v in (base, base | 1):
                litvals.setdefault(v, []).append(t)
            # RAM literals (IWRAM 0x03xxxxxx / EWRAM 0x02xxxxxx) are stored
            # as-is, no bus-base offset; also tolerate Thumb bit0 form
            for v in (t, t | 1):
                litvals.setdefault(v, []).append(t)
        # scan words at 4-byte alignment (literal pools are word-aligned)
        for off in range(0, len(rom) - 3, 4):
            w = struct.unpack_from("<I", rom, off)[0]
            ts = litvals.get(w)
            if ts:
                for t in ts:
                    # skip self-referential nonsense inside header etc.
                    out[t]["lits"].append(off)
    return out


def fmt(off, vma):
    return f"0x{off + 0x08000000:08X}" if vma else f"0x{off:06X}"


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("rom")
    ap.add_argument("addresses", nargs="+",
                    help="file-offset addresses to xref (accepts 0x..)")
    ap.add_argument("--calls-only", action="store_true")
    ap.add_argument("--lits-only", action="store_true")
    ap.add_argument("--vma", action="store_true",
                    help="print bus addresses (0x08xxxxxx) instead of file offsets")
    args = ap.parse_args()

    rom = read_rom(args.rom)
    targets = []
    for a in args.addresses:
        v = int(a, 16) if a.lower().startswith("0x") else int(a)
        if v >= 0x08000000:          # tolerate VMA input
            v -= 0x08000000
        targets.append(v)

    res = scan(rom, targets, args.calls_only, args.lits_only)

    for t in targets:
        print(f"=== refs to {fmt(t, args.vma)} "
              f"(VMA 0x{t + 0x08000000:08X}) ===")
        calls = res[t]["calls"]
        lits = res[t]["lits"]
        if not calls and not lits:
            print("  (no references found)")
            continue
        if calls:
            print(f"  bl callers ({len(calls)}):")
            for s in sorted(calls):
                print(f"    bl @ {fmt(s, args.vma)}")
        if lits:
            shown = sorted(set(lits))
            print(f"  literal words ({len(shown)}):")
            for l in shown[:200]:
                print(f"    lit @ {fmt(l, args.vma)}")
            if len(shown) > 200:
                print(f"    ... {len(shown) - 200} more")


if __name__ == "__main__":
    main()
