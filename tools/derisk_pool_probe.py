#!/usr/bin/env python3
"""Widened de-risk: pool-using leaves -- does literal-pool placement match?

The first de-risk pass used pool-free leaves, which sidesteps the hardest part
of matching this ROM: literal-pool placement. The pool word must land at exactly
the offset the `ldr rN,[pc,#N]` encodes, and the distance depends on where the
compiler decides to place the pool *and* what else it puts there.

Each candidate here is reconstructed from its own disassembly into C and compiled
at its real ROM VMA, so the `ldr`/`pool` distance is part of the byte comparison
rather than something the harness normalises away.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ROM = ROOT / "baserom.gba"
WORK = ROOT / "build/era-corpus/derisk-pool"
AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"  # matching compiler (docs/findings/compiler_split.md)
OPTS = ("-O", "-O1", "-O2", "-Os")
ROM_BASE = 0x08000000
TYPEDEFS = ("typedef unsigned char u8; typedef unsigned short u16; "
            "typedef unsigned int u32;\n")

# (name, ROM VMA, ROM span length, C source, note)
#
# The pool word is an absolute RAM address in the ROM, and it is the address that
# appears in the *pool*, not the pool's own location -- read from the ROM here
# and used as a C constant. Compiling at the real VMA makes the pool's distance
# from the `ldr` part of the byte comparison rather than something normalised away.
CANDIDATES = [
    ("08001f8c_getheld", 0x08001F8C, 12,
     "u16 f(void){ return *(u16 *)(*(u32 *)0x030000E4 + 10); }",
     "u16 getter through a pool pointer"),
    ("08002044_get08", 0x08002044, 12,
     "u16 f(void){ return *(u16 *)(*(u32 *)0x030000E4 + 12); }",
     "u16 getter, different field"),
    ("080016d0_set04", 0x080016D0, 12,
     "void f(u16 v){ u32 *p = *(u32 *)0x030000E4; *(u16 *)((u8 *)p + 4) = v; }",
     "u16 setter through a pool pointer"),
    ("08001f80_set10", 0x08001F80, 12,
     "void f(u16 v){ u32 *p = *(u32 *)0x030000E4; *(u16 *)((u8 *)p + 10) = v; }",
     "u16 setter, different field"),
    ("08004b9c_substate", 0x08004B9C, 16,
     "u32 f(void){ u32 *q = *(u32 *)0x03000198; return *(u16 *)(*(u32 *)((u8 *)q + 8) + 4) & 1; }",
     "bit test two levels into a pool pointer"),
    ("08002b44_zero", 0x08002B44, 12,
     "void f(void){ *(u16 *)0x03000158 = 0; }",
     "zero a u16 at a pool address"),
    ("08002af4_index", 0x08002AF4, 12,
     "void *f(u32 i){ return (void *)((i * 4) + 0x0203F170); }",
     "pool base + scaled index"),
    ("08001cb4_plusd0", 0x08001CB4, 16,
     "u32 f(void){ u32 *q = *(u32 *)0x030000E4; return *(u32 *)((u8 *)q + 0xD0); }",
     "u32 getter two levels into a pool pointer"),
    ("08004748_triple", 0x08004748, 16,
     "void f(u32 a, u32 b, u16 c){ u32 *p = (u32 *)0x03000188;"
     " *(u32 *)((u8 *)p + 0) = a; *(u32 *)((u8 *)p + 4) = b;"
     " *(u16 *)((u8 *)p + 8) = c; }",
     "three stores at three widths"),
    ("08004ef0_store5c", 0x08004EF0, 12,
     "void f(u32 v){ u32 *q = *(u32 *)0x030001A0; u32 *p = *(u32 *)((u8 *)q + 8);"
     " *(u32 *)((u8 *)p + 0x5C) = v; }",
     "u32 store two levels into a pool pointer"),
    ("08002be8_tally", 0x08002BE8, 18,
     "int f(void){ u16 *c = (u16 *)0x03000158; int t = *c; *c = t + 1; return t; }",
     "post-increment a u16, return it sign-extended"),
    ("08002060_setbyte", 0x08002060, 12,
     "void f(u8 v){ u32 *p = *(u32 *)0x030000E4; *((u8 *)p + 2) = v; }",
     "u8 store through a pool pointer"),
]


def run(argv, stdout=None):
    kwargs = {"text": True, "stderr": subprocess.PIPE}
    kwargs["stdout"] = stdout if stdout is not None else subprocess.PIPE
    return subprocess.run([str(a) for a in argv], **kwargs)


def disasm(blob: bytes, vma: int) -> list[tuple[str, str]]:
    WORK.mkdir(parents=True, exist_ok=True)
    tmp = WORK / "chunk.bin"
    tmp.write_bytes(blob)
    out = run(["arm-none-eabi-objdump", "-D", "-b", "binary", "-m", "arm", "-M",
               "force-thumb", f"--adjust-vma={vma:#x}", str(tmp)]).stdout
    ins = []
    for line in out.splitlines():
        parts = line.split("\t")
        if len(parts) >= 3 and ":" in parts[0]:
            ins.append((parts[2].split("@")[0].strip(),
                        parts[3].split("@")[0].strip() if len(parts) > 3 else ""))
    return ins


def shape(insn):
    mn, ops = insn
    ops = re.sub(r"\[r\d+(?:, #K)?\]", "[M]", ops)
    ops = re.sub(r"\[pc(?:, #K)?\]", "[PC]", ops)
    ops = re.sub(r"#\S+", "#K", ops)
    ops = re.sub(r"r\d+", "rX", ops)
    return (mn, ops)


def pool_distances(ins):
    """Every `ldr rX,[pc,#N]` in the routine, in order.

    The immediate is the word offset from the instruction to the pool, so this
    is the number that says whether agbcc placed the pool where the ROM does.
    objdump writes the operand as `[pc, #4]`, optionally followed by a `@ (addr)`
    comment that the caller has already stripped.
    """
    out = []
    for mn, ops in ins:
        m = re.search(r"\[pc,\s*#(-?\d+)\]", ops)
        if mn.startswith("ldr") and m:
            out.append(int(m.group(1)))
    return out


def classify(rom_ins, cand_ins):
    r, c = [shape(i) for i in rom_ins], [shape(i) for i in cand_ins]
    if r == c:
        return "register allocation"
    if [m for m, _ in r] == [m for m, _ in c]:
        return "operand kind"
    if sorted(m for m, _ in r) == sorted(m for m, _ in c):
        return "instruction order"
    return "instruction selection"


def compile_one(src, vma, opt, tag):
    WORK.mkdir(parents=True, exist_ok=True)
    c, i = WORK / f"{tag}{opt}.c", WORK / f"{tag}{opt}.i"
    s, o = WORK / f"{tag}{opt}.s", WORK / f"{tag}{opt}.o"
    e, b = WORK / f"{tag}{opt}.elf", WORK / f"{tag}{opt}.bin"
    c.write_text(TYPEDEFS + src + "\n", encoding="utf-8")
    with i.open("w", encoding="utf-8") as h:
        if run(["clang", "-E", "-nostdinc", "-undef", str(c)], stdout=h).returncode:
            return None
    if run([AGBCC, opt, "-mthumb-interwork", str(i), "-o", s]).returncode:
        return None
    if run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-o", o, s]).returncode:
        return None
    if run(["arm-none-eabi-ld", f"--section-start=.text={vma:#x}", "-o", e, o]).returncode:
        return None
    if run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text", e, b]).returncode:
        return None
    return b.read_bytes() if b.exists() else None


def main() -> int:
    if not AGBCC.exists():
        print("derisk_pool: agbcc not built", file=sys.stderr)
        return 2
    rom = ROM.read_bytes()
    print(f"pool-using leaves: {len(CANDIDATES)} candidates x {len(OPTS)} levels\n")
    print(f"{'candidate':<20}{'ROM':>4}  " + "".join(f"{o:>13}" for o in OPTS) + "   miss class")
    print("-" * 100)
    exact = 0
    pool_ok = 0
    classes: dict[str, int] = {}
    for name, vma, reflen, src, note in CANDIDATES:
        ref = rom[vma - ROM_BASE:vma - ROM_BASE + reflen]
        cells, miss = [], "-"
        ref_pool = pool_distances(disasm(ref, vma))
        for opt in OPTS:
            blob = compile_one(src, vma, opt, name)
            if blob is None:
                cells.append("FAIL"); continue
            if blob[:reflen] == ref:
                cells.append("EXACT"); exact += 1
            else:
                p = 0
                for L in range(min(len(blob), reflen), 0, -1):
                    if ref[:L] == blob[:L]:
                        p = L; break
                cells.append(f"{len(blob)}B p{p}")
                if miss == "-":
                    miss = classify(disasm(ref, vma), disasm(blob[:reflen], vma))
            if pool_distances(disasm(blob, vma)) == ref_pool:
                pool_ok += 1
        if miss != "-":
            classes[miss] = classes.get(miss, 0) + 1
        print(f"{name:<20}{reflen:>4}  " + "".join(f"{x:>13}" for x in cells) + f"   {miss}")
        print(f"{'':<24}({note}; ROM pool dist {ref_pool})")
    total = len(CANDIDATES) * len(OPTS)
    print(f"\nbyte-exact compiles:      {exact}/{total}")
    print(f"pool distances identical: {pool_ok}/{total}")
    print("miss classification:", classes or "none")
    return 0 if exact else 1


if __name__ == "__main__":
    raise SystemExit(main())
