#!/usr/bin/env python3
"""objdump2gas_mixed.py - convert a raw span that mixes ARM and Thumb code into
byte-exact gas .s. Reuses tools/objdump2gas.py's Thumb pipeline and adds ARM
decoding for the ARM-mode segments.

Usage:
    # 1. produce force-thumb and arm-mode listings of the whole span:
    dd if=baserom.gba of=/tmp/span.bin bs=1 skip=$((START)) count=$((END-START))
    arm-none-eabi-objdump -D -b binary -m arm -M force-thumb \\
        --adjust-vma=$((0x08000000+START)) /tmp/span.bin > /tmp/span.t.lst
    arm-none-eabi-objdump -D -b binary -m arm --adjust-vma=$((0x08000000+START)) \\
        /tmp/span.bin > /tmp/span.a.lst
    # 2. convert, giving mode boundaries (address:mode, mode in {thumb,arm}):
    python3 tools/objdump2gas_mixed.py --start 0xSTART --end 0xEND \\
        --tlst /tmp/span.t.lst --alst /tmp/span.a.lst \\
        --modes 0xSTART:thumb,0xADDR1:arm,0xADDR2:thumb,... --out asm/span.s

The result must assemble byte-identical; `make`'s SHA gate arbitrates.
"""
import argparse
import os
import re
import struct
import sys

import objdump2gas as o2g

ROM_DEFAULT = "baserom.gba"


def arm_ldr_pc_targets(data, start, end):
    """ARM `ldr rN,[pc,#imm]` targets within [start,end)."""
    out = {}
    for f in range(start, end - 3, 4):
        w = struct.unpack_from("<I", data, f)[0]
        # 0xE59F0000 (P=1,U=1) or 0xE51F0000 (P=1,U=0): ldr rn,[pc,imm]
        if (w & 0xFFF00000) in (0xE5900000, 0xE5100000):
            rn = (w >> 16) & 0xF
            if rn == 15:
                imm = w & 0xFFF
                if (w >> 23) & 1:
                    tgt = f + 8 + imm
                else:
                    tgt = f + 8 - imm
                out[tgt] = f
    return out


def arm_bl_targets(data, start, end):
    """ARM BL (0xEBxxxxxx, cond 1110) sites/targets within span."""
    sites = {}
    for f in range(start, end - 3, 4):
        w = struct.unpack_from("<I", data, f)[0]
        if (w & 0xFF000000) == 0xEB000000:
            imm24 = w & 0xFFFFFF
            if imm24 & 0x800000:
                imm24 -= 0x1000000
            tgt = f + 8 + (imm24 << 2)
            if start <= tgt < end:
                sites[f] = tgt
    return sites


def parse_modes(modes_arg, start, end):
    """Return sorted list of (addr, mode) switches including (start, default)."""
    segs = [(start, "thumb")]
    for part in modes_arg.split(","):
        part = part.strip()
        if not part:
            continue
        a, m = part.split(":")
        segs.append((int(a, 0), m.strip().lower()))
    segs.sort()
    return segs


def mode_at(segs, addr):
    md = "thumb"
    for a, m in segs:
        if a <= addr:
            md = m
        else:
            break
    return md


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--start", required=True)
    ap.add_argument("--end", required=True)
    ap.add_argument("--tlst", required=True, help="force-thumb objdump listing")
    ap.add_argument("--alst", required=True, help="arm-mode objdump listing")
    ap.add_argument("--modes", required=True,
                   help="comma list addr:mode switches, e.g. 0x2B888:thumb,0x2B88C:arm")
    ap.add_argument("--out", required=True)
    ap.add_argument("--rom", default=ROM_DEFAULT)
    ap.add_argument("--asm-dir", default="asm")
    ap.add_argument("--symbols", default="",
                   help="comma list addr:name of extra exported symbols to "
                        "define at those addresses (e.g. 0x2B898:sub_0802B898)")
    args = ap.parse_args()

    start = int(args.start, 0)
    end = int(args.end, 0)
    data = open(args.rom, "rb").read()
    segs = parse_modes(args.modes, start, end)

    t_instrs, t_order, t_garbage = o2g.parse_listing(args.tlst)
    a_instrs, a_order, a_garbage = o2g.parse_listing(args.alst)

    # Pool detection
    pool = set()
    # Thumb pools over thumb segments only
    thumb_pool = o2g.detect_pools(data, start, end)
    # keep only those whose address is in thumb mode
    for f in thumb_pool:
        if mode_at(segs, f) == "thumb":
            pool.add(f)
    # ARM ldr-pc pools
    for tgt, src in arm_ldr_pc_targets(data, start, end).items():
        if start <= tgt < end and mode_at(segs, tgt) == "arm":
            pool.add(tgt)

    # Function entries
    func_set = set()
    # Thumb segments: reuse detection
    t_bl_sites = set()
    for a in t_order:
        if start + 0x08000000 <= a < end + 0x08000000:
            t_bl_sites.add(a - 0x08000000)
    t_funcs, t_blspan = o2g.detect_func_entries(
        t_instrs, t_order, data, start, end, pool, t_bl_sites, t_garbage)
    for f in t_funcs:
        if mode_at(segs, f) == "thumb":
            func_set.add(f)
    # ARM segments: entries = segment starts + ARM BL targets in span
    arm_bl = arm_bl_targets(data, start, end)
    for f in range(start, end, 2):
        if mode_at(segs, f) == "arm" and f in arm_bl.values():
            func_set.add(f)
    # segment starts that are arm mode
    for a, m in segs:
        if m == "arm" and start <= a < end:
            func_set.add(a)

    # labels
    labels = set(func_set) | pool
    # branch/BL targets in span (both modes)
    # Thumb branches
    for a in t_order:
        if not (start + 0x08000000 <= a < end + 0x08000000):
            continue
        f = a - 0x08000000
        m = re.match(r"^(b[a-z]*\.?n?)\s+(0x[0-9a-f]+)", t_instrs[a][1])
        if m and m.group(1).startswith("b") and m.group(1) != "bl":
            tgt = int(m.group(2), 16) - 0x08000000
            if start <= tgt < end:
                labels.add(tgt)
        m = re.match(r"^bl\s+(0x[0-9a-f]+)", t_instrs[a][1])
        if m and m.group(1):
            tgt = int(m.group(1), 16) - 0x08000000
            if start <= tgt < end:
                labels.add(tgt)
    # ARM branches/bl
    for a in a_order:
        if not (start + 0x08000000 <= a < end + 0x08000000):
            continue
        f = a - 0x08000000
        txt = a_instrs[a][1]
        m = re.match(r"^(b|bl|bx|blx)\s+(0x[0-9a-f]+)", txt)
        if m and m.group(2):
            tgt = int(m.group(2), 16) - 0x08000000
            if start <= tgt < end:
                labels.add(tgt)
    # jump-table pool targets
    for w in sorted(pool):
        if w % 4:
            continue
        val = struct.unpack_from("<I", data, w)[0]
        f = val - 0x08000000
        if start <= f < end:
            labels.add(f)

    out = []
    out.append("@ GT Advance 3 - mixed ARM/Thumb span (generated by "
               "tools/objdump2gas_mixed.py)")
    out.append(f"@ Region: file offset 0x{start:06X}-0x{end:06X}")
    out.append("@ byte-exact (make SHA gate). Mode switches: " + args.modes)
    out.append("")

    extra_syms = {}
    for part in args.symbols.split(","):
        part = part.strip()
        if not part:
            continue
        a, nm = part.split(":")
        extra_syms[int(a, 0)] = nm.strip()  # a is a file offset

    cur_mode = None
    emitted = set()
    addr = start + 0x08000000
    end_addr = end + 0x08000000

    def gtext(mode, raw, text, f):
        if mode == "thumb":
            return o2g.gas_text(o2g.strip_comment(text), addr, start, end,
                                func_set, set())
        # ARM: minimal transforms
        m = re.match(r"^(b|bl|bx|blx)\s+(0x[0-9a-f]+)$", text.strip())
        if m:
            op = m.group(1)
            tgt = int(m.group(2), 16)
            tf = tgt - 0x08000000
            if start <= tf < end:
                if tf in func_set:
                    return f"{op} sub_0800{tf:04X}"
                return f"{op} _0800{tf:04X}"
            return f"{op} 0x{tgt:08X}"
        m = re.match(r"^ldr\s+(r\d+),\s*\[pc,\s*#\d+\]\s*@\s*\(0x([0-9a-f]+)\)",
                     text)
        if m:
            tgt = int(m.group(2), 16)
            return f"ldr {m.group(1)}, _0800{tgt - 0x08000000:04X}"
        return text.strip()

    while addr < end_addr:
        f = addr - 0x08000000
        md = mode_at(segs, f)
        if md != cur_mode:
            out.append(f"\t.{md}")
            cur_mode = md
        if f in pool and f % 4 == 0:
            val = struct.unpack_from("<I", data, f)[0]
            if f not in emitted:
                out.append(f"_0800{f:04X}: .4byte 0x{val:08X}")
                emitted.add(f)
            addr += 4
            continue
        if f in func_set:
            if f not in emitted:
                out.append(f"\t.type sub_0800{f:04X}, %function")
                out.append(f"sub_0800{f:04X}:")
                out.append(f"_0800{f:04X}:")
                emitted.add(f)
        if f in extra_syms:
            out.append(f"\t.type {extra_syms[f]}, %function")
            out.append(f"{extra_syms[f]}:")
            emitted.add(f)
        elif f in labels and f not in emitted:
            out.append(f"_0800{f:04X}:")
            emitted.add(f)
        instrs = t_instrs if md == "thumb" else a_instrs
        raw, text = instrs.get(addr, (None, None))
        if raw is None:
            # padding / unknown: emit halfword
            hw = struct.unpack_from("<H", data, f)[0]
            out.append(f"\t.hword 0x{hw:04X}")
            addr += 2
            continue
        t = gtext(md, raw, text, f)
        out.append(f"\t{t}")
        step = 4 if len(raw) >= 2 and md == "thumb" else (4 if md == "arm" else 2)
        addr += step

    with open(args.out, "w") as fh:
        fh.write("\n".join(out) + "\n")
    print(f"wrote {args.out}: {len(out)} lines ({end - start} B span), "
          f"funcs={len(func_set)} pools={len(pool)}")


if __name__ == "__main__":
    main()
