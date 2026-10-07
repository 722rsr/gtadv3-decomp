#!/usr/bin/env python3
"""De-risk experiment: does agbcc byte-match ordinary game functions?

The runtime-provenance probe (`tools/era_runtime_probe.py`) closed the
compiler-*identity* question. This tests the follow-on question that actually
gates any migration to agbcc: does its *code generator* reproduce ordinary game
functions well enough to justify porting the C corpus?

Each candidate is a real ROM function reconstructed into C from its own
disassembly, compiled with agbcc at every optimization level, and byte-compared
against its ROM span. Only pool-free, straight-line leaves are used -- no
branches, no calls, no literal pool -- so every byte is attributable to codegen
alone and no relocation resolution is needed.

The count alone is not the interesting output. Each miss is also *classified*,
because a miss that is only register allocation or instruction ordering is a
source-shape problem the project has already shown it can absorb, whereas a miss
that selects different instructions is a codegen problem that no amount of C
reshaping fixes.

    python3 tools/derisk_probe.py
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ROM = ROOT / "baserom.gba"
WORK = ROOT / "build/era-corpus/derisk"
AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"  # matching compiler (docs/findings/compiler_split.md)
OPTS = ("-O", "-O1", "-O2", "-Os")
ROM_BASE = 0x08000000
TYPEDEFS = ("typedef unsigned char u8; typedef unsigned short u16; "
            "typedef unsigned int u32;\n")

# (name, ROM VMA, ROM span length, C reconstruction, what it does)
#
# The reconstructions are written to express the ROM's own operand order where
# that is observable. No `volatile`: the era work showed it forces
# read-modify-write stores the ROM never emits.
CANDIDATES = [
    ("08028250_decrement", 0x08028250, 8,
     "void f(void *p){ u32 *c = (u32 *)((u8 *)p + 4); *c = *c - 1; }",
     "decrement a u32 cell"),
    ("080055cc_reset", 0x080055CC, 10,
     "void f(void *p){ u16 *a = (u16 *)((u8 *)p + 2); *a = 0;"
     " *((u32 *)((u8 *)p + 12)) = *(u32 *)((u8 *)p + 8); }",
     "zero a u16, copy a u32 between fields"),
    ("08005f8c_deltasub", 0x08005F8C, 12,
     "void f(void *x, void *y){ u32 *a = (u32 *)x; u32 *b = (u32 *)y;"
     " u32 d = *a - *b; *a = d; *((u32 *)((u8 *)x + 4)) = d; }",
     "subtract two fields, store to two"),
    ("0802d92c_iterate", 0x0802D92C, 12,
     "void f(void *q, u32 i){ u8 *p = *(u8 **)((u8 *)q + 0x40);"
     " *((u8 *)q + 30) = p[0]; *((u32 *)((u8 *)q + 0x40)) = i + 1; }",
     "indexed walk: load ptr, load byte, store byte, bump index"),
    ("08004b90_setbit", 0x08004B90, 10,
     "void f(void *p){ u16 *c = (u16 *)((u8 *)p + 4); *c = 4 | *c; }",
     "OR a constant into a u16 field"),
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


def shape(insn: tuple[str, str]) -> tuple[str, str]:
    """Opcode plus operand *shape*, with registers and constants masked.

    Registers are masked so an allocation difference is not reported as a
    different instruction, but the *kind* of each operand is kept: a memory
    operand stays `[M]` whether the base is r0 or r2, while a register operand
    becomes `rX`. That distinction is load-bearing -- masking `[rX, #K]` down to
    the same token as a bare register would hide a real difference in what the
    instruction accesses.
    """
    mn, ops = insn
    ops = re.sub(r"\[r\d+(?:, #K)?\]", "[M]", ops)
    ops = re.sub(r"#\S+", "#K", ops)
    ops = re.sub(r"r\d+", "rX", ops)
    return (mn, ops)


def classify(rom_ins, cand_ins) -> str:
    """Say *how* a candidate differs, not merely that it does.

    Three buckets, in decreasing order of how much they matter for a port:

    * `register allocation` -- identical instruction sequence and identical
      operand *kinds*; only the registers chosen differ. This is the bucket the
      era-revision ladder already showed cannot be fixed by changing compiler
      revision, but is reachable by reshaping the C.
    * `instruction order` / `instruction selection` -- the compiler chose a
      different sequence of instructions. This is a codegen difference that no
      amount of source reshaping fixes, and is what would argue against the
      migration.
    """
    r, c = [shape(i) for i in rom_ins], [shape(i) for i in cand_ins]
    if [m for m, _ in r] == [m for m, _ in c] and r == c:
        return "register allocation"
    if [m for m, _ in r] == [m for m, _ in c]:
        return "operand kind"
    if sorted(m for m, _ in r) == sorted(m for m, _ in c):
        return "instruction order"
    return "instruction selection"


def compile_one(src: str, vma: int, opt: str, tag: str) -> bytes | None:
    WORK.mkdir(parents=True, exist_ok=True)
    c, i = WORK / f"{tag}{opt}.c", WORK / f"{tag}{opt}.i"
    s, o = WORK / f"{tag}{opt}.s", WORK / f"{tag}{opt}.o"
    e, b = WORK / f"{tag}{opt}.elf", WORK / f"{tag}{opt}.bin"
    c.write_text(TYPEDEFS + src + "\n", encoding="utf-8")
    with i.open("w", encoding="utf-8") as handle:
        if run(["clang", "-E", "-nostdinc", "-undef", str(c)], stdout=handle).returncode:
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
        print(f"derisk_probe: agbcc not built at {AGBCC}", file=sys.stderr)
        return 2
    if not ROM.exists():
        print(f"derisk_probe: no ROM at {ROM}", file=sys.stderr)
        return 2
    rom = ROM.read_bytes()

    print(f"agbcc vs ROM, pool-free leaves: {len(CANDIDATES)} candidates "
          f"x {len(OPTS)} optimization levels\n")
    print(f"{'candidate':<22}{'ROM':>4}  " + "".join(f"{o:>11}" for o in OPTS) + "   miss class")
    print("-" * 96)
    exact = 0
    classes: dict[str, int] = {}
    for name, vma, reflen, src, note in CANDIDATES:
        ref = rom[vma - ROM_BASE:vma - ROM_BASE + reflen]
        cells, miss = [], "-"
        for opt in OPTS:
            blob = compile_one(src, vma, opt, name)
            if blob is None:
                cells.append("FAIL")
                continue
            # The linker pads the section to 4 bytes; the ROM's padding is a
            # separate halfword, so compare only the routine's real span.
            if blob[:reflen] == ref:
                cells.append("EXACT")
                exact += 1
            else:
                prefix = 0
                for length in range(min(len(blob), reflen), 0, -1):
                    if ref[:length] == blob[:length]:
                        prefix = length
                        break
                cells.append(f"{len(blob)}B p{prefix}")
                if miss == "-":
                    miss = classify(disasm(ref, vma), disasm(blob[:reflen], vma))
        if miss != "-":
            classes[miss] = classes.get(miss, 0) + 1
        print(f"{name:<22}{reflen:>4}  " + "".join(f"{c:>11}" for c in cells) + f"   {miss}")
        print(f"{'':<26}({note})")

    total = len(CANDIDATES) * len(OPTS)
    print(f"\nbyte-exact compiles: {exact}/{total}")
    print("miss classification:", classes or "none")
    # The migration case only needs *some* byte-exact compiles on ordinary game
    # functions; a run with none is the result that would argue against it.
    return 0 if exact else 1


if __name__ == "__main__":
    raise SystemExit(main())
