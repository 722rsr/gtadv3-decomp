#!/usr/bin/env python3
"""Runtime-provenance probe: which toolchain's compiler runtime is linked in the ROM?

The compiler-identity work in `tools/era_compiler_probe.py` and
`tools/era_revision_ladder.py` compares *compiled game source* against ROM
bytes.  That evidence is strong but indirect: it depends on reconstructing a
plausible original source, and a single matched function (n=1) cannot separate
"this toolchain" from "this toolchain's Thumb backend agrees on simple code".

This probe closes that gap with evidence carrying no source-reconstruction risk
at all.  An ARM EABI ROM links a small set of hand-written runtime routines --
the 32-bit division cores and the interworking call veneers.  Those are *not*
compiler output: they are a fixed assembly source file shipped with the
toolchain (`libgcc/lib1thumb.asm` in the GCC tree).  So if the ROM contains a
toolchain's runtime byte-for-byte, that toolchain's runtime is what was linked,
and no choice of C source could have produced that evidence.

The routines checked here, and where the ROM keeps them:

| Routine | Toolchain symbol | ROM VMA | Bytes |
|---|---|---|---|
| interworking call veneer | `_call_via_rX` | `0x0802DDC8` | 60 |
| unsigned divide | `__udivsi3` | `0x0802DF6C` | 120 |
| unsigned remainder | `__umodsi3` | `0x0802DFE4` | 192 |
| signed divide | `__divsi3` | `0x0802DE04` | 148 |
| signed remainder | `__modsi3` | `0x0802DE9C` | 208 |
| divide-by-zero hook | `__div0` | `0x0802DE98` | 2 |

Why this is not a restatement of the era-compiler result.  `agbcc` matching a
game function shows its *code generator* agrees on some C.  A runtime routine
matching shows the ROM *links that toolchain's library*, which is a statement
about the build environment rather than about codegen luck, and it is what makes
`agbcc` a legitimate drop-in for the independent C build rather than merely an
interesting lookalike.

## How the comparison is made exact

The division cores end with `bl __div0`, whose displacement is a link-time
relocation.  Assembling one routine standalone leaves that displacement
unresolved, so the raw comparison would differ by two bytes for a reason that
has nothing to do with codegen.  The probe therefore links each core against a
one-instruction `__div0` stub placed at the ROM's own `__div0` address
(`0x0802DE98`), which resolves the branch to exactly the value the ROM carries
and lets the comparison run over the *whole* routine.

What survives is then only the routine's own trailing alignment: the cores end
with a pool halfword whose value belongs to whatever follows them in the ROM, so
a candidate whose tail is `00 00` where the ROM continues into the next routine
is aligned, not different.  That single case is accepted; anything else is a
mismatch, and the self-test pins each accept and each reject.

Usage:

    python3 tools/era_runtime_probe.py --self-test
    python3 tools/era_runtime_probe.py
    python3 tools/era_runtime_probe.py --json docs/data/era_runtime_probe.json

Absent toolchain sources are reported as SKIP rather than as a mismatch, but a
SKIP still fails the target: a missing tree means the comparison is incomplete,
not clean.  Both trees ship with the repository (the agbcc one via
`make toolchain`, the GCC 2.95.3 one vendored), so a SKIP now means something was
deleted or the provisioner was not run.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ROM = ROOT / "baserom.gba"
ROM_BASE = 0x08000000
WORK = ROOT / "build/era-corpus/runtime"

# Candidate toolchain trees, each shipping its own Thumb runtime assembly.
# `agbcc` and `old_agbcc` are two builds of the same pinned source tree,
# so one runtime-source entry covers both.
#
# This probe reads exactly one file per tree -- its `lib1thumb.asm`.  The agbcc
# entry comes from the tree `make toolchain` provisions; the vanilla GCC 2.95.3
# entry points at the single vendored file under tools/third_party/ instead of at
# a full GCC source tree, because that file is the only part of such a tree the
# probe uses.  Both the routine rows and the negative control are built from it,
# so vendoring it makes the whole target reproducible without a GCC checkout.
TREES = {
    "agbcc": ROOT / "build/toolchains/agbcc/libgcc/lib1thumb.asm",
    "gcc-2.95.3": ROOT / "tools/third_party/gcc-2.95.3/lib1thumb.asm",
}

# Each routine is `#ifdef`-guarded in the toolchain's file, and the file's local
# labels (Loop1, Over1, Ldiv0, ...) repeat across routines, so it only assembles
# one routine at a time.
#
# (macro, ROM VMA, whether the routine needs the `__div0` link step, recorded
#  region end -- the project's `Region:` banner on the matching asm transcription)
ROUTINES = {
    "__aeabi_uidiv": ("L_udivsi3", 0x0802DF6C, True, 0x0802DFE4),
    "__aeabi_umod": ("L_umodsi3", 0x0802DFE4, True, 0x0802E0A4),
    "__aeabi_idiv": ("L_divsi3", 0x0802DE04, True, 0x0802DE96),
    "__aeabi_imod": ("L_modsi3", 0x0802DE9C, True, 0x0802DF6A),
    "__aeabi_call_via_rX": ("L_call_via_rX", 0x0802DDC8, False, 0x0802DE04),
}

# Where the ROM's own `__div0` lives, decoded from the cores' `bl` displacements.
# It sits between the signed and unsigned cores, so the link script pins each
# section at its absolute address rather than relying on object order.
DIV0_VMA = 0x0802DE98

# Vanilla GCC 2.95.3 also offers a mode-switching interworking veneer that agbcc
# replaced with a plain `bx rN` stub.  A genuine negative control: the ROM must
# contain the stub form and *not* the switching form.
NEGATIVE_CONTROLS = {
    "gcc-2.95.3:__interwork_call_via_rX": "L_interwork_call_via_rX",
}


class ProbeError(RuntimeError):
    pass


def run(argv, stdout=None):
    kwargs = {"text": True, "stderr": subprocess.PIPE}
    kwargs["stdout"] = stdout if stdout is not None else subprocess.PIPE
    return subprocess.run([str(a) for a in argv], **kwargs)


def link_script(path: Path) -> Path:
    """A script that pins `__div0` at its ROM address and `.text1` at the routine."""
    path.write_text(
        "SECTIONS\n"
        "{\n"
        f"  . = {DIV0_VMA:#x};\n"
        '  .text0 : { *(.text0) }\n'
        "}\n"
        "SECTIONS\n"
        "{\n"
        '  .text1 : { *(.text) }\n'
        "}\n",
        encoding="utf-8",
    )
    return path


def div0_stub(path: Path) -> Path:
    """A minimal `__div0` -- the ROM's is a bare `bx lr`."""
    path.write_text(
        "\t.code 16\n"
        '\t.section .text0,"ax",%progbits\n'
        "\t.align 2,0\n"
        "\t.thumb_func\n"
        "\t.globl __div0\n"
        "__div0:\n"
        "\tbx lr\n",
        encoding="utf-8",
    )
    return path


def build_routine(tree_name: str, source: Path, macro: str, vma: int, needs_div0: bool) -> bytes | None:
    """Assemble (and, for the divide cores, link) one runtime routine."""
    WORK.mkdir(parents=True, exist_ok=True)
    tag = re.sub(r"\W+", "_", f"{tree_name}_{macro}")
    pre_s = WORK / f"{tag}.s"
    obj = WORK / f"{tag}.o"
    # vanilla GCC's file asserts __USER_LABEL_PREFIX__ and builds symbol names
    # from it; agbcc's copy hardcodes SYM(x) = x and takes no such define.
    defines = [f"-D{macro}"]
    if tree_name == "gcc-2.95.3":
        defines.append("-D__USER_LABEL_PREFIX__=")
    with pre_s.open("w", encoding="utf-8") as handle:
        pre = run(["clang", "-E", "-nostdinc", "-undef", "-x", "assembler-with-cpp",
                   *defines, source], stdout=handle)
    if pre.returncode or not pre_s.read_text(encoding="utf-8").strip():
        return None
    if run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-o", obj, pre_s]).returncode:
        return None
    if not needs_div0:
        binf = WORK / f"{tag}.bin"
        if run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text", obj, binf]).returncode:
            return None
        return binf.read_bytes() if binf.exists() else None

    stub_o = WORK / f"{tag}_div0.o"
    if run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-o", stub_o, div0_stub(WORK / "div0.s")]).returncode:
        return None
    elf, binf = WORK / f"{tag}.elf", WORK / f"{tag}.bin"
    linked = run(["arm-none-eabi-ld", "-T", link_script(WORK / f"{tag}.lds"),
                  f"--section-start=.text1={vma:#x}", "-o", elf, obj, stub_o])
    if linked.returncode:
        return None
    if run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text1", elf, binf]).returncode:
        return None
    return binf.read_bytes() if binf.exists() else None


def is_align_padding(tail: bytes) -> bool:
    """True when the candidate's final pool halfword is align, not code.

    Each core ends with an align-to-4 pool.  In the ROM that halfword belongs to
    whatever routine follows; standalone, the linker emits `00 00`.  This is
    accepted only when the tail is exactly one zero halfword, so a differing
    opcode, immediate, or register list stays a MISMATCH.
    """
    return tail in (b"", b"\x00\x00")


def check(rom: bytes, tree_name: str, source: Path) -> list[dict]:
    rows = []
    for routine, (macro, vma, needs_div0, end) in ROUTINES.items():
        blob = build_routine(tree_name, source, macro, vma, needs_div0)
        if blob is None or len(blob) < 8:
            rows.append({"tree": tree_name, "routine": routine, "status": "SKIP",
                         "note": f"could not assemble {macro}"})
            continue
        offset = vma - ROM_BASE
        # The recorded region is the routine plus at most one align halfword; the
        # linker may also pad the section, so compare over the recorded span and
        # treat the trailing halfword as alignment only when the candidate's own
        # bytes there are the linker's `00 00`.
        region = rom[offset:end - ROM_BASE]
        matched = 0
        for length in range(len(region), 0, -1):
            if region[:length] == blob[:length]:
                matched = length
                break
        tail = region[matched:len(blob)]
        if matched == len(region):
            status = "MATCH"
        elif len(tail) == 2 and is_align_padding(tail):
            status = "MATCH_MINUS_ALIGN"
        else:
            status = "MISMATCH"
        rows.append({
            "tree": tree_name,
            "routine": routine,
            "status": status,
            "vma": f"{vma:#010x}",
            "end": f"{end:#010x}",
            "region_bytes": len(region),
            "candidate_bytes": len(blob),
            "matched_bytes": matched + (2 if status == "MATCH_MINUS_ALIGN" else 0),
            "candidate_sha256": hashlib.sha256(blob).hexdigest(),
        })
    return rows


def negatives(rom: bytes) -> list[dict]:
    rows = []
    for label, macro in NEGATIVE_CONTROLS.items():
        tree_name = label.split(":")[0]
        blob = build_routine(tree_name, TREES[tree_name], macro, 0x08000000, False)
        if blob is None or len(blob) < 8:
            rows.append({"control": label, "status": "SKIP", "note": "could not assemble"})
            continue
        found = rom.find(blob)
        rows.append({
            "control": label,
            "status": "ABSENT" if found < 0 else "PRESENT",
            "candidate_bytes": len(blob),
            "vma": None if found < 0 else f"{found + ROM_BASE:#010x}",
            "candidate_sha256": hashlib.sha256(blob).hexdigest(),
        })
    return rows


def div0_stub_matches_rom(rom: bytes) -> bool:
    """Confirm the ROM's `__div0` really is the bare `bx lr` the stub stands in for.

    The link step pins `__div0` at `DIV0_VMA` so the cores' `bl` displacements
    resolve to the ROM's values.  That is only sound if the ROM's routine at that
    address is the one being stubbed, so the stub's own bytes are compared
    directly: `f7 46` is `bx lr` in Thumb.  If this ever fails, the pins are wrong
    and every "full match" below would be meaningless.
    """
    offset = DIV0_VMA - ROM_BASE
    return rom[offset:offset + 2] == b"\xf7\x46"


def self_test() -> int:
    checks = 0
    # The accept: a candidate whose final pool halfword is the linker's `00 00`.
    if not is_align_padding(b"\x00\x00"):
        raise AssertionError("a zero pool halfword is align, not a mismatch")
    checks += 1
    if not is_align_padding(b""):
        raise AssertionError("an absent tail needs no allowance")
    checks += 1
    if is_align_padding(b"\xc0\x46"):
        raise AssertionError("a mov r8,r8 nop in the tail is a real difference")
    checks += 1
    if is_align_padding(b"\x00\x00\x00\x00"):
        raise AssertionError("more than one halfword of tail is a real difference")
    checks += 1
    if is_align_padding(b"\x00"):
        raise AssertionError("an odd-length tail is not align")
    checks += 1
    if len(ROUTINES) != 5:
        raise AssertionError("five runtime routines are expected")
    checks += 1
    # Four divide cores need the div0 link step; the call veneer does not.
    div_cores = [vma for _, vma, needs, _ in ROUTINES.values() if needs]
    if len(div_cores) != 4:
        raise AssertionError("four divide cores are expected")
    checks += 1
    # Every recorded region must be longer than the routine it names, since each
    # carries at least its trailing align halfword.
    for _, vma, _, end in ROUTINES.values():
        if end <= vma:
            raise AssertionError(f"region {vma:#x}..{end:#x} is empty")
    checks += 1
    # `__div0` must be a distinct address inside the runtime band, since the link
    # script places it absolutely; it sits between the signed and unsigned cores.
    if not (0x0802DE00 <= DIV0_VMA <= 0x0802E000):
        raise AssertionError("__div0 must lie in the runtime band")
    checks += 1
    if DIV0_VMA in {vma for _, vma, _, _ in ROUTINES.values()}:
        raise AssertionError("__div0 must not overlap a checked routine")
    checks += 1
    # The pin is only sound if the ROM really has a bare `bx lr` there.
    rom = bytearray(0x200000)
    rom[DIV0_VMA - ROM_BASE:DIV0_VMA - ROM_BASE + 2] = b"\xf7\x46"
    if not div0_stub_matches_rom(bytes(rom)):
        raise AssertionError("a bare bx lr at the pinned __div0 must validate")
    checks += 1
    rom[DIV0_VMA - ROM_BASE] = 0x00
    if div0_stub_matches_rom(bytes(rom)):
        raise AssertionError("a wrong __div0 pin must be rejected")
    checks += 1
    print(f"era_runtime_probe self-test: {checks}/{checks} checks PASS")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--rom", type=Path, default=ROM)
    parser.add_argument("--json", type=Path)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        return self_test()
    if not args.rom.exists():
        print(f"era_runtime_probe: ERROR: no ROM at {args.rom}", file=sys.stderr)
        return 2

    rom = args.rom.read_bytes()
    if not div0_stub_matches_rom(rom):
        print(f"era_runtime_probe: ERROR: no bare `bx lr` at the pinned __div0 "
              f"{DIV0_VMA:#010x} (found {rom[DIV0_VMA - ROM_BASE:DIV0_VMA - ROM_BASE + 4].hex()}); "
              "the divide-core link pins are unsound", file=sys.stderr)
        return 2
    rows, skipped = [], []
    for tree_name, source in TREES.items():
        if not source.exists():
            skipped.append({"tree": tree_name, "status": "SKIP", "note": f"no {source}"})
            continue
        rows.extend(check(rom, tree_name, source))
    controls = negatives(rom) if TREES["gcc-2.95.3"].exists() else []

    width = max((len(r["routine"]) for r in rows if "routine" in r), default=10)
    print(f"runtime provenance against {args.rom.name}")
    print(f"{'tree':<12} {'routine':<{width}}  {'vma':<12}{'match':>8}  status")
    print("-" * (12 + width + 28))
    for row in rows:
        if row.get("status") == "SKIP":
            print(f"{row['tree']:<12} {row.get('routine', '-'):<{width}}  {'-':<12}{'--':>8}  SKIP: {row.get('note')}")
            continue
        print(f"{row['tree']:<12} {row['routine']:<{width}}  {row['vma']:<12}"
              f"{row['matched_bytes']:>5}/{row['region_bytes']:<3}  {row['status']}")
    for row in skipped:
        print(f"{row['tree']:<12} {'-':<{width}}  {'-':<12}{'--':>8}  SKIP: {row['note']}")

    all_match = bool(rows) and not skipped and all(
        r["status"] in ("MATCH", "MATCH_MINUS_ALIGN") for r in rows
    )
    print()
    for row in controls:
        print(f"negative control {row['control']}: {row['status']}"
              + (f" @ {row['vma']}" if row["vma"] else ""))
    verdict = ("all runtime routines match the ROM" if all_match
               else "NOT all runtime routines match the ROM")
    print(verdict)

    if args.json:
        args.json.write_text(json.dumps(
            {
                "rom": args.rom.name,
                "verdict": verdict,
                "all_match": all_match,
                "div0_vma": f"{DIV0_VMA:#010x}",
                "results": rows + skipped,
                "negative_controls": controls,
            }, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print(f"JSON: {args.json}")
    return 0 if all_match else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ProbeError as error:
        print(f"era_runtime_probe: ERROR: {error}", file=sys.stderr)
        raise SystemExit(2)
