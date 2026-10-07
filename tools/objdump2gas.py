#!/usr/bin/env python3
"""objdump2gas.py - convert an objdump Thumb listing into byte-exact gas .s.

The proven pipeline for converting raw ROM spans into `asm/*.s` files that
link byte-identically under `make`'s SHA gate (used for the menu
span; gbadisasm aborts on this ROM's unclassified literal pools, see
tools/README.md "Known gbadisasm limitations").

Usage:
    # 1. produce the objdump listing for a raw span (file offsets!):
    dd if=baserom.gba of=/tmp/span.bin bs=1 skip=$((0xSTART)) count=$((END-START))
    arm-none-eabi-objdump -D -b binary -m arm -M force-thumb \
        --adjust-vma=$((0x08000000 + 0xSTART)) /tmp/span.bin > /tmp/span.lst
    # 2. convert:
    python3 tools/objdump2gas.py --start 0xSTART --end 0xEND \
        --lst /tmp/span.lst --out asm/span.s

Pipeline:
  1. Parse the objdump listing -> (vma, rawbytes, text) in address order.
  2. Detect POOL words inside the span:
       a. every `ldr rN,[pc,#imm]` target anywhere in the ROM that lands in
          the span (iterated to fixpoint),
       b. pointer runs: a word whose value is an even VMA into the span is a
          jump-table base; extend forward while consecutive words are too.
  3. Detect FUNCTION ENTRIES automatically (BL sites inside the span are
     real code by construction; use `--extra-bl-sites` for verified
     external callers -- a blind ROM-wide scan would also hit data-region
     coincidences like 0x3C4E8/0x3E630/0xD1AB8):
       a. `push` at a BL target into the span from a known code site,
       b. `push` whose previous non-pool instruction is a return (`bx`),
          an unconditional branch, an epilogue `pop`, or a `movs r0,r0` pad
          (i.e. not fall-through) -- this catches dispatchers buried after
          epilogues/pools,
       c. bare `bx lr` no-op stubs at BL targets (0x4770 single instruction).
  4. Walk the span linearly: pool words -> `.4byte`, function entries ->
     `.type sub_0800XXXX, %function` + `sub_`/`_080` labels, other labels ->
     `_0800XXXX`, instructions -> gas text (branches/BLs/ldr-pc resolved to
     labels, external BLs to converted symbols stay symbolic, everything
     else numeric).
  5. The result must assemble byte-identical; `make`'s SHA gate arbitrates.

Conventions (match the repo): labels are `sub_0800<file-offset-hex>` /
`_0800<file-offset-hex>`; file offsets, add 0x08000000 for VMA; pools at
original offsets; pad halfwords emitted as `.hword`.
"""
import argparse
import os
import re
import struct
import sys

ROM_DEFAULT = "baserom.gba"
ASM_DIR_DEFAULT = "asm"


def thumb_bl_targets(rom):
    """Yield (site_off, target_off) for every decodable Thumb BL pair.
    Same decoder as tools/xref.py (verified against objdump)."""
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
        pc = off + 4
        yield off, pc + disp


# ---------------------------------------------------------------- parse lst
def parse_listing(path):
    """Return (instrs, order, garbage_wide): vma -> (rawbytes_list, text),
    sorted vmas, and the set of offsets inside multi-halfword listing lines
    that are NOT `bl` (objdump decodes pool/data bytes as garbage ARM/VFP
    "instructions"; those offsets are not reliable Thumb instruction
    boundaries)."""
    instrs = {}
    order = []
    garbage_wide = set()
    for ln in open(path, encoding="utf-8", errors="replace"):
        m = re.match(r"^\s*([0-9a-f]+):\t(.*)$", ln)
        if not m:
            continue
        vma = int(m.group(1), 16)
        rest = m.group(2)
        parts = rest.split("\t")
        raw = parts[0].strip().split()
        text = "\t".join(parts[1:]) if len(parts) > 1 else parts[0]
        if vma not in instrs:
            instrs[vma] = (raw, text.strip())
            order.append(vma)
        if len(raw) >= 2 and not text.strip().startswith("bl"):
            for k in range(1, len(raw)):
                garbage_wide.add(vma + 2 * k)
            garbage_wide.add(vma)
    order.sort()
    return instrs, order, garbage_wide


# ------------------------------------------------------------- pool detect
def ldr_pc_targets(data, exclude_sources):
    """ldr rN,[pc,#imm] targets in the whole ROM, skipping sources in
    exclude_sources (data regions). Returns {target: [sources]}."""
    out = {}
    for off in range(0, len(data) - 3, 2):
        if off in exclude_sources:
            continue
        hw = data[off] | (data[off + 1] << 8)
        if (hw & 0xF800) == 0x4800:
            imm = (hw & 0xFF) * 4
            pc = ((off + 4) & ~3)
            tgt = pc + imm
            out.setdefault(tgt, []).append(off)
    return out


def pointer_run_words(data, start, end):
    """Word-aligned runs of even VMAs into the span (jump tables / pointer
    pools). Starts at any word whose value is an even VMA into span; extends
    forward while consecutive words are too."""
    run = set()
    for w in range(start & ~3, end - 4, 4):
        val = struct.unpack_from("<I", data, w)[0]
        f = val - 0x08000000
        if start <= f < end and (val & 1) == 0:
            a = w
            while a + 4 <= end:
                v2 = struct.unpack_from("<I", data, a)[0]
                f2 = v2 - 0x08000000
                if start <= f2 < end and (v2 & 1) == 0:
                    run.add(a)
                    a += 4
                else:
                    break
    return run


def detect_pools(data, start, end):
    """Literal pools / jump tables. Iterate to a fixpoint, RECOMPUTING from
    scratch each round so that "fake" ldr-pc decodes of pool data words never
    poison the pool permanently.

    Trap: a pool word holding an out-of-span pointer (e.g. 0x080C544C) is
    missed by pointer_run_words (in-span-only); its low halfword (0x4C54)
    decodes as `ldr r4,[pc,#imm]` and, if the fixpoint merely accumulated
    targets, would mark a real function entry as pool data. Recomputing with
    only trusted (non-pool) sources each round drops those fake targets once
    the data word itself is recognized as pool.
    """
    pool = set()
    for _ in range(8):
        cand = ldr_pc_targets(data, pool)
        new = {t for t in cand if start <= t < end}
        nxt = set(pointer_run_words(data, start, end)) | new
        if nxt == pool:
            break
        pool = nxt
    return pool


# -------------------------------------------------- function entry detect
def detect_func_entries(instrs, order, data, start, end, pool, bl_sites,
                        garbage_wide):
    """Automatic function-entry detection. Returns (funcs, bl_targets_span).

    bl_sites is the set of BL *site* file offsets that are known to be real
    code (by default the sites inside the span being converted; extend with
    `--extra-bl-sites` for verified external callers). Its targets inside
    the span form bl_targets_span.

    Rules (a push at address A is an entry unless it falls through from a
    regular instruction):
      a. A is a BL target into the span from a known code site;
      b. the previous *non-pool, non-garbage* instruction is a return
         (`bx rN`), an unconditional branch (`b`/`b.n`), an epilogue `pop`,
         or a pad (`movs r0, r0` = 0x46C0);
      c. bare `bx lr` (0x4770) single-instruction no-op stubs at BL targets;
      d. raw-byte pass: a `push` opcode (0xB4xx/0xB5xx) at an offset the
         listing missed (hidden inside a garbage multi-halfword line, e.g.
         objdump merged a pool word's tail halfword with the next
         function's prologue) that satisfies rule (b).
    """
    funcs = set()
    bl_targets_span = set()
    for site, tgt in thumb_bl_targets(data):
        if site not in bl_sites:
            continue
        if start <= tgt < end:
            bl_targets_span.add(tgt)

    push_mnems = ("push",)
    rets = ("bx", "b", "b.n", "pop {")

    def prev_nonpool(addr):
        """Previous instruction before addr, skipping pool words and
        garbage-wide-line offsets (not reliable instruction boundaries)."""
        p = addr - 2
        while p in pool or p in garbage_wide:
            p -= 2
        return instrs.get(p)

    def is_ret_or_pad(prev):
        pm = re.match(r"^([a-z][a-z0-9.]*)", prev[1])
        pname = pm.group(1) if pm else ""
        return pname.startswith(rets) or (
            pname == "movs" and "r0, r0" in prev[1])

    for a in order:
        if a < start + 0x08000000 or a >= end + 0x08000000:
            continue
        f = a - 0x08000000
        raw, text = instrs[a]
        m = re.match(r"^([a-z][a-z0-9.]*)", text)
        mnem = m.group(1) if m else ""
        if mnem in push_mnems:
            is_bl_tgt = f in bl_targets_span
            prev = prev_nonpool(a)
            is_prev_ret = prev is not None and is_ret_or_pad(prev)
            # a mid-function second push of the high-register dance is
            # preceded by `mov rN, r8` -> fall-through, correctly excluded
            if is_bl_tgt or is_prev_ret or prev is None:
                funcs.add(f)
        elif mnem == "bx" and f in bl_targets_span:
            # no-op stub: bare `bx lr` (0x4770) at a BL target
            hw = struct.unpack_from("<H", data, f)[0]
            if hw == 0x4770:
                funcs.add(f)

    # (d) raw-byte pass: push opcodes hidden in gaps of the listing
    for f in range(start, end - 1, 2):
        if f in funcs or (f + 0x08000000) in instrs:
            continue
        hw = struct.unpack_from("<H", data, f)[0]
        if 0xB400 <= hw <= 0xB5FF:
            prev = prev_nonpool(f + 0x08000000)
            if prev is None or is_ret_or_pad(prev):
                funcs.add(f)
    return funcs, bl_targets_span


# ----------------------------------------------------------- symbol tables
def converted_symbols(asm_dir):
    """sub_0800XXXX labels defined in the converted asm files."""
    out = set()
    if not os.path.isdir(asm_dir):
        return out
    for fn in os.listdir(asm_dir):
        if fn.endswith(".s"):
            for ln in open(os.path.join(asm_dir, fn), encoding="utf-8",
                           errors="replace"):
                m = re.match(r"^\s*(sub_0800[0-9A-Fa-f]+):", ln)
                if m:
                    out.add(m.group(1))
    return out


# ------------------------------------------------------- gas text helpers
def strip_comment(text):
    """Remove objdump `@ 0x..` suffix comments, keep pool refs `@ (0x..)`."""
    if "@" in text and "(" not in text:
        text = text.split("@")[0].rstrip()
    return text


def gas_text(text, vma, start, end, func_set, converted):
    """Convert one objdump instruction to gas, resolving labels."""
    # branch -> label (strip .n).  NOTE: exclude only the exact `bl`
    # mnemonic -- `ble`/`bls`/`blt` also start with "bl" but are
    # conditional branches.
    m = re.match(r"^(b[a-z]*\.?n?)\s+(0x[0-9a-f]+)$", text)
    if m and m.group(1).startswith("b") and m.group(1) != "bl":
        op = m.group(1).replace(".n", "")
        tgt = int(m.group(2), 16)
        f = tgt - 0x08000000
        if start <= f < end:
            return f"{op} _0800{f:04X}"
        return f"{op} 0x{tgt:08X}"
    # bl -> function symbol / interior label / numeric
    m = re.match(r"^bl\s+(0x[0-9a-f]+)$", text)
    if m:
        tgt = int(m.group(1), 16)
        f = tgt - 0x08000000
        if start <= f < end:
            if f in func_set:
                return f"bl sub_0800{f:04X}"
            return f"bl _0800{f:04X}"
        # Out-of-span targets: always emit numeric `bl 0x0800XXXX`.  Symbolifying
        # to a `sub_0800XXXX` defined in another .s file can mis-resolve under
        # the single-section link (the assembler/linker may land on a veneer or
        # a stale address).  Numeric form is what every other converted file
        # uses for external calls and assembles byte-exact.
        return f"bl 0x{f + 0x08000000:08X}"
    # ldr rN,[pc,#imm] -> pool label
    m = re.match(r"^ldr\s+(r\d+),\s*\[pc,\s*#\d+\]\s*@\s*\(0x([0-9a-f]+)\)",
                 text)
    if m:
        tgt = int(m.group(2), 16)
        return f"ldr {m.group(1)}, _0800{tgt - 0x08000000:04X}"
    return text


# ------------------------------------------------------------------ main
def main():
    ap = argparse.ArgumentParser(
        description="Convert an objdump Thumb listing into byte-exact gas.")
    ap.add_argument("--start", required=True, help="span start, file offset, "
                    "e.g. 0xFF78")
    ap.add_argument("--end", required=True, help="span end (exclusive), "
                    "file offset, e.g. 0x15CB4")
    ap.add_argument("--lst", required=True, help="objdump listing file")
    ap.add_argument("--out", default="/dev/stdout",
                    help="output .s path (default stdout)")
    ap.add_argument("--rom", default=ROM_DEFAULT, help="baserom path")
    ap.add_argument("--asm-dir", default=ASM_DIR_DEFAULT,
                    help="dir of converted asm files for symbol lookup")
    ap.add_argument("--title", default="converted span",
                    help="one-line title for the generated header")
    ap.add_argument("--extra-bl-sites", default="",
                    help="comma-separated file offsets of verified external "
                    "BL sites (code, not data) whose targets into the span "
                    "should be labeled; e.g. 0x164C8 for the rec35_driver "
                    "call into the menu tail")
    args = ap.parse_args()

    start = int(args.start, 0)
    end = int(args.end, 0)
    if start >= end:
        sys.exit("error: --start must be < --end")

    data = open(args.rom, "rb").read()
    instrs, order, garbage_wide = parse_listing(args.lst)
    pool = detect_pools(data, start, end)
    # BL sites inside the span are real code by construction; external ones
    # must be listed explicitly (a ROM-wide scan picks up data-region
    # coincidences like 0x3C4E8/0x3E630/0xD1AB8).
    bl_sites = set()
    for a in order:
        if start + 0x08000000 <= a < end + 0x08000000:
            bl_sites.add(a - 0x08000000)
    for s in args.extra_bl_sites.split(","):
        s = s.strip()
        if s:
            bl_sites.add(int(s, 0))
    func_set, bl_span = detect_func_entries(
        instrs, order, data, start, end, pool, bl_sites, garbage_wide)
    converted = converted_symbols(args.asm_dir)

    # labels to emit: branch/BL targets (in-span), function entries, pool
    # words, jump-table case targets
    branch_targets = set()
    bl_targets_in = set()
    for a in order:
        if not (start + 0x08000000 <= a < end + 0x08000000):
            continue
        f = a - 0x08000000
        m = re.match(r"^(b[a-z]*\.?n?)\s+(0x[0-9a-f]+)", instrs[a][1])
        if m and m.group(1).startswith("b") and m.group(1) != "bl":
            tgt = int(m.group(2), 16)
            tf = tgt - 0x08000000
            if start <= tf < end:
                branch_targets.add(tf)
        m = re.match(r"^bl\s+(0x[0-9a-f]+)", instrs[a][1])
        if m:
            tgt = int(m.group(1), 16)
            tf = tgt - 0x08000000
            if start <= tf < end:
                bl_targets_in.add(tf)
    jt_targets = set()
    for w in sorted(pool):
        if w % 4:
            continue
        val = struct.unpack_from("<I", data, w)[0]
        f = val - 0x08000000
        if start <= f < end:
            jt_targets.add(f)
    labels = branch_targets | bl_targets_in | bl_span | func_set | pool \
        | jt_targets

    out = []
    out.append(f"@ GT Advance 3 - {args.title}")
    out.append(f"@ Region: file offset 0x{start:06X}-0x{end:06X} "
               f"(VMA 0x{start + 0x08000000:08X}-0x{end + 0x08000000:08X}).")
    out.append("@ Pure Thumb, ARMCC. Generated by tools/objdump2gas.py from "
               "an objdump listing of")
    out.append("@ baserom.gba; byte-exact (make SHA gate). Unconverted "
               "targets keep numeric")
    out.append("@ `bl 0x0800XXXX` form. Pools at original offsets.")
    out.append("")

    emitted = set()
    addr = start + 0x08000000
    end_addr = end + 0x08000000
    while addr < end_addr:
        f = addr - 0x08000000
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
        elif f in labels:
            if f not in emitted:
                out.append(f"_0800{f:04X}:")
                emitted.add(f)
        raw, text = instrs.get(addr, (None, None))
        if raw is None:
            hw = struct.unpack_from("<H", data, f)[0]
            if f in func_set and 0xB400 <= hw <= 0xB5FF:
                # function start hidden in a garbage listing line:
                # decode the push prologue for readable text
                regs = [f"r{r}" for r in range(8) if hw & (1 << r)]
                if hw & 0x100:
                    regs.append("lr")
                out.append(f"\tpush\t{{" + ", ".join(regs) + "}")
            else:
                out.append(f"\t.hword 0x{hw:04X}")
            addr += 2
            continue
        t = gas_text(strip_comment(text), addr, start, end, func_set,
                     converted)
        out.append(f"\t{t}")
        step = 4 if len(raw) >= 2 else 2
        addr += step

    with open(args.out, "w", encoding="utf-8") as fh:
        fh.write("\n".join(out) + "\n")
    print(f"wrote {args.out}: {len(out)} lines, {len(out)} "
          f"({end - start} B span)")
    print(f"pools={len(pool)} funcs={len(func_set)} "
          f"labels={len(labels)} bl_targets_in_span={len(bl_span)}")


if __name__ == "__main__":
    main()
