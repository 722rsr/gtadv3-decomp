#!/usr/bin/env python3
"""Exhaustive static reference audit for one or more ROM addresses.

Why this exists : `tools/xref.py` reports BL callers and
literal words, but a literal hit in the **asset/data tail** is almost
never a pointer. The uncompressed data region holds regular 12-byte
numeric tables `{u16 a, u16 b, u32 p, u32 q}` whose value fields sweep
smoothly through the exact numeric range of the low code clusters
(e.g. p/q = 0x2107, 0x205A, 0x2006, 0x1FB4, 0x1F99, 0x1F64 ...), so
"a word equals <code address>" is frequently a *sample value*, not a
reference. That mistake was made once (asm/ghost.s called those
samples "descriptor tables consumed by relocated code"); this tool
classifies every hit so it cannot be repeated silently.

For each target address it reports three reference forms:

  * Thumb `bl` call sites whose computed target equals the address
  * VMA-form literal words (`0x08xxxxxx == target`)
  * file-offset-form literal words (`target - 0x08000000`, with and
    without the bit0 Thumb tag -- how runtime-relocated code would
    address a body)

and classifies every hit site against the converted spans parsed from
`asm/passthrough.inc`:

  CONVERTED-CODE(region, file)  inside a converted `asm/*.s` span
  CODE-POCKET                   below `--code-end` but in no converted span
  DATA-TAIL                     at/after `--code-end` (assets + data)

DATA-TAIL hits are additionally fitted against a numeric table: if the
site lies inside a `{u16,u16,u32,u32}` (or `{u32,u32}`) run, the tool
prints the table base, stride, record index and field, plus the
neighbouring sample values, so "reference" vs "sample" is decidable at
a glance.

Usage:
  python3 tools/refaudit.py baserom.gba 0x08001F98
  python3 tools/refaudit.py baserom.gba 0x08001F98 0x08001FB4 --no-tables
  python3 tools/refaudit.py baserom.gba --calls-only 0x08001FB4
"""
import argparse
import struct
import sys


def read_rom(path):
    with open(path, "rb") as f:
        return f.read()


def parse_regions(asm_dir):
    """Return [(lo, hi, [file, ...])] from asm/passthrough.inc comments."""
    import os
    import re

    empty = [(0, 0x2E158, [])]
    path = os.path.join(asm_dir, "passthrough.inc")
    if not os.path.exists(path):
        return empty
    regions = []
    cur = None
    line_re = re.compile(r"converted region 0x0*([0-9A-Fa-f]+)-0x0*([0-9A-Fa-f]+)")
    inc_re = re.compile(r'\.include\s+"([^"]+)"')
    for line in open(path):
        m = line_re.search(line)
        if m:
            cur = (int(m.group(1), 16), int(m.group(2), 16), [])
            regions.append(cur)
            continue
        m = inc_re.search(line)
        if m and cur is not None:
            cur[2].append(m.group(1))
    return regions or empty


def bl_targets(rom):
    """Yield (site_file_off, target_vma) for every decodable Thumb BL."""
    for off in range(0, len(rom) - 4, 2):
        h1 = rom[off] | (rom[off + 1] << 8)
        if (h1 & 0xF800) != 0xF000:
            continue
        h2 = rom[off + 2] | (rom[off + 3] << 8)
        if (h2 & 0xF800) != 0xF800:
            continue
        s = ((h1 & 0x7FF) << 12) | ((h2 & 0x7FF) << 1)
        if h1 & 0x0400:
            s -= 1 << 23
        yield off, (0x08000000 + off + 4 + s) & 0xFFFFFFFF


def w16(rom, off):
    return struct.unpack_from("<H", rom, off)[0]


def w32(rom, off):
    return struct.unpack_from("<I", rom, off)[0]


TABLE_STRIDES = {
    12: ((0, 2), (4,), (8,)),   # {u16,u16, u32, u32}
    8: ((0, 2), (4,),),         # {u16,u16, u32}
    16: ((0, 2), (4,), (8,), (12,)),
}


def record_ok(rom, off, stride):
    pat = TABLE_STRIDES[stride]
    if off < 0 or off + stride > len(rom):
        return False
    for u16s in pat[0:1]:
        for p in u16s:
            v = w16(rom, off + p)
            if v == 0 or v >= 0x400:
                return False
    for w32s in pat[1:]:
        for p in w32s:
            v = w32(rom, off + p)
            if v == 0 or v >= 0x100000:
                return False
    return True


FIT_SPAN = 256  # cap record-run expansion so distinct tables do not merge


def fit_table(rom, off, min_run=6):
    """Best (stride, base, run) numeric-table run containing file offset off."""
    best = None
    for stride in TABLE_STRIDES:
        for phase in range(stride // 4):
            cand = off - phase * 4
            if not record_ok(rom, cand, stride):
                continue
            base = cand
            back = 0
            while back < FIT_SPAN and record_ok(rom, base - stride, stride):
                base -= stride
                back += 1
            run = 0
            while run < FIT_SPAN and record_ok(rom, base + run * stride, stride):
                run += 1
            if run >= min_run and (best is None or run > best[2]):
                best = (stride, base, run)
    return best


def classify(file_off, regions, code_end):
    for lo, hi, files in regions:
        if lo <= file_off < hi:
            return ("CONVERTED-CODE", files[0] if files else "?")
    if file_off < code_end:
        return ("CODE-POCKET", None)
    return ("DATA-TAIL", None)


def fmt_site(rom, file_off, regions, code_end, tables, value):
    kind, detail = classify(file_off, regions, code_end)
    align = file_off % 4
    out = "    file %#08x VMA %#010x  %-15s %-14s align=%d" % (
        file_off, 0x08000000 + file_off, kind, detail or "-", align)
    if tables and kind == "DATA-TAIL":
        fit = fit_table(rom, file_off)
        if fit:
            stride, base, run = fit
            rec = (file_off - base) // stride
            field = file_off - (base + rec * stride)
            pat = TABLE_STRIDES[stride]
            flat = [p for group in pat for p in group]
            idx = flat.index(field) if field in flat else -1
            neigh = [w32(rom, base + (rec + k) * stride + pat[1][0])
                     for k in (-2, -1, 0, 1, 2)
                     if record_ok(rom, base + (rec + k) * stride, stride)]
            out += "\n      -> numeric-table SAMPLE: stride %d, table file %#08x (%d records), rec %d field-off %d (u32 index %d); u32-field sweep %s" % (
                stride, base, run, rec, field, idx, [hex(v) for v in neigh])
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("rom")
    ap.add_argument("addresses", nargs="+", help="target VMA(s), 0x08XXXXXX")
    ap.add_argument("--asm-dir", default="asm",
                    help="where passthrough.inc lives (region map)")
    ap.add_argument("--code-end", type=lambda s: int(s, 0), default=0x2E158,
                    help="first file offset of the asset/data tail")
    ap.add_argument("--calls-only", action="store_true")
    ap.add_argument("--lits-only", action="store_true")
    ap.add_argument("--no-offset-form", action="store_true",
                    help="skip file-offset-form literal scan")
    ap.add_argument("--no-tables", action="store_true",
                    help="skip numeric-table coincidence fitting")
    a = ap.parse_args()

    rom = read_rom(a.rom)
    regions = parse_regions(a.asm_dir)
    if not a.asm_dir or not regions:
        regions = [(0, a.code_end, [])]

    bl_cache = None
    for addr_s in a.addresses:
        addr = int(addr_s, 0)
        print("=== refaudit %#010x ===" % addr)
        if not a.lits_only:
            if bl_cache is None:
                bl_cache = list(bl_targets(rom))
            hits = [(0x08000000 + o, t) for o, t in bl_cache if (t & ~1) == addr]
            print("  bl-callers (%d):" % len(hits))
            for site, t in hits:
                print(fmt_site(rom, site - 0x08000000, regions, a.code_end,
                               not a.no_tables, addr))
        if not a.calls_only:
            forms = [("vma-form", addr), ("offset-form(|1)", (addr - 0x08000000) | 1)]
            if not a.no_offset_form:
                forms.append(("offset-form", addr - 0x08000000))
            for name, val in forms:
                hits = [o for o in range(0, len(rom) - 4)
                        if w32(rom, o) == val]
                print("  %s %#010x (%d):" % (name, val, len(hits)))
                for o in hits:
                    print(fmt_site(rom, o, regions, a.code_end,
                                   not a.no_tables, val))
    return 0


if __name__ == "__main__":
    sys.exit(main())
