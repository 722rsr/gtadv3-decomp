#!/usr/bin/env python3
"""Compare GCC-2.95-era cross compilers against a pinned ROM span.

GT Advance 3 uses Thumb-1 code generation with high-register save idioms,
interleaved literal pools, and zero alignment filler. This tool compares
era compilers at fixed ROM addresses, including padding and literal pools.

Two era toolchains are supported, both optional and both expected under the
gitignored `build/toolchains/` tree:

* `agbcc` / `old_agbcc` -- pret/agbcc, a GCC 2.95.3 fork built natively.
* DevKit Advance R5 Beta 3 -- GCC 3.2.2 (`cc1.exe` under Wine).  The driver
  cannot run under Wine because it passes Unix TMPDIR paths to the Windows
  `cc1`, so `cc1.exe` is invoked directly, and `libiconv-2.dll` must sit beside
  it.

The tool compiles a preprocessed C file with each available compiler and
optimization level, assembles and links the result at a given VMA, and reports
size, first differing byte, and whether pinned reference byte patterns (the
ROM's leaf functions) appear verbatim in the candidate.

    python3 tools/era_compiler_probe.py --self-test
    python3 tools/era_compiler_probe.py
    python3 tools/era_compiler_probe.py --source build/toolchains/agbcc/experiment/keypad.c \\
        --vma 0x08002430 --end 0x080024AC \\
        --leaf 0x08002488:0x08002494 --leaf 0x08002494:0x080024A0 \\
        --leaf 0x080024A0:0x080024AC --json build/toolchains/agbcc/era_results.json

Absent toolchains are reported as SKIP, never as failure, so the tool is safe
to run on a machine that only has the modern compiler.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_ROM = ROOT / "baserom.gba"
DEFAULT_WORK = ROOT / "build/toolchains/agbcc/era"
AGBCC_DIR = ROOT / "build/toolchains/agbcc"
DEVKIT_GLOB = "build/toolchains/devkitadv/**/lib/gcc-lib/arm-agb-elf/*/cc1.exe"
WINEPREFIX = ROOT / "build/toolchains/armcc/wineprefix"

AGBCC_OPT_LEVELS = ("-O", "-O1", "-O2", "-Os")
DEVKIT_OPT_LEVELS = ("-O1", "-O2", "-O3", "-Os")

# The ROM's non-leaf epilogue is `pop {rN}; pop {r0}; bx r0`, which the GCC 2.95
# back end only emits for Thumb when interworking is enabled; without it the
# same source produces `pop {rN, pc}`, a form the ROM never uses anywhere. That
# single flag also fixes the literal-pool distances and the r2/r3 allocation of
# the keypad cluster, so it is a build-flag fact about the original, not a
# per-function tuning knob.
DEFAULT_EXTRA_FLAGS = ("-mthumb-interwork",)


class ProbeError(RuntimeError):
    pass


def run(argv, **kwargs):
    kwargs.setdefault("capture_output", True)
    kwargs.setdefault("text", True)
    return subprocess.run([str(a) for a in argv], **kwargs)


def have(program: str) -> str | None:
    return shutil.which(program)


def find_agbcc() -> dict[str, Path]:
    found = {}
    for name in ("agbcc", "old_agbcc"):
        path = AGBCC_DIR / name
        if path.is_file() and os.access(path, os.X_OK):
            found[name] = path
    return found


def find_devkit_cc1() -> Path | None:
    matches = sorted(ROOT.glob(DEVKIT_GLOB))
    return matches[-1] if matches else None


def include_flags() -> list[str]:
    flags: list[str] = []
    for rel in ("include", "asm", "build/toolchains/agbcc/ginclude"):
        path = ROOT / rel
        if path.is_dir():
            flags += ["-I", str(path)]
    return flags


def preprocess(source: Path, out: Path, preprocessor: str) -> Path:
    """Produce a `.i` file that both era compilers can consume."""
    work = run(
        [preprocessor, "-E", "-nostdinc", "-undef", *include_flags(), "-x", "c", source]
    )
    if work.returncode != 0:
        raise ProbeError(f"preprocess failed for {source}:\n{work.stderr.strip()}")
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(work.stdout, encoding="utf-8")
    return out


def link_at(obj: Path, vma: int, out: Path) -> Path | None:
    elf = out.with_suffix(".elf")
    if run(["arm-none-eabi-ld", f"--section-start=.text={vma:#x}", "-o", elf, obj]).returncode:
        return None
    if run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text", elf, out]).returncode:
        return None
    return out if out.exists() else None


def assemble(asm: Path, obj: Path) -> bool:
    return run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-o", obj, asm]).returncode == 0


def build_agbcc(binary: Path, opt: str, source: Path, vma: int, work: Path, extra=()):
    asm = work / f"{binary.name}{opt}.s"
    obj = work / f"{binary.name}{opt}.o"
    unit = run([binary, opt, *extra, source, "-o", asm])
    if unit.returncode != 0 or not asm.exists():
        return None, (unit.stderr or unit.stdout).strip()
    if not assemble(asm, obj):
        return None, "assemble failed"
    return link_at(obj, vma, work / f"{binary.name}{opt}.bin"), None


def build_devkit(cc1: Path, opt: str, source: Path, vma: int, work: Path, extra=()):
    asm = work / f"gcc322{opt}.s"
    obj = work / f"gcc322{opt}.o"
    env = dict(os.environ, WINEDEBUG="-all")
    if WINEPREFIX.exists():
        env["WINEPREFIX"] = str(WINEPREFIX)
    if not have("wine"):
        return None, "wine not installed"
    unit = run(["wine", cc1, opt, "-mthumb", *extra, "-quiet", source, "-o", asm], env=env)
    if unit.returncode != 0 or not asm.exists():
        return None, (unit.stderr or unit.stdout).strip() or f"cc1 exit {unit.returncode}"
    if not assemble(asm, obj):
        return None, "assemble failed"
    return link_at(obj, vma, work / f"gcc322{opt}.bin"), None


def first_diff(reference: bytes, candidate: bytes) -> int | None:
    for index in range(min(len(reference), len(candidate))):
        if reference[index] != candidate[index]:
            return index
    return None


def summarize(reference: bytes, candidate: bytes) -> str:
    if candidate[: len(reference)] == reference:
        return f"MATCH ({len(reference)}/{len(reference)} bytes)"
    offset = first_diff(reference, candidate)
    if offset is None:
        return f"prefix match, length {len(candidate)} != {len(reference)}"
    return f"diff@+0x{offset:02X}"


def leaf_hits(candidate: bytes, leaves: list[tuple[str, bytes]]) -> str:
    return " ".join(f"{name}:{'Y' if blob in candidate else 'n'}" for name, blob in leaves)


def parse_leaf(spec: str, rom: bytes) -> tuple[str, bytes]:
    start, _, end = spec.partition(":")
    lo, hi = int(start, 16), int(end, 16)
    return f"{lo:08X}", rom[lo - 0x08000000 : hi - 0x08000000]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--source", type=Path, default=AGBCC_DIR / "experiment/keypad.c")
    parser.add_argument("--vma", type=lambda s: int(s, 16), default=0x08002430)
    parser.add_argument("--end", type=lambda s: int(s, 16), default=0x080024AC)
    parser.add_argument("--leaf", action="append", default=[], help="VMA:endVMA reference byte pattern")
    parser.add_argument("--work", type=Path, default=DEFAULT_WORK)
    parser.add_argument("--json", type=Path)
    parser.add_argument("--extra-flag", action="append", nargs="?", default=None,
                        help="compiler flag for every candidate (default: -mthumb-interwork). "
                             "Pass a bare --extra-flag to run with no extra flags, which is a "
                             "useful control: without interworking the epilogue is wrong.")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        return self_test()
    if not args.rom.exists():
        print(f"era_compiler_probe: ERROR: no ROM at {args.rom}", file=sys.stderr)
        return 2
    if not args.source.exists():
        print(
            f"era_compiler_probe: ERROR: no source at {args.source}\n"
            "Run tools/agbcc_c89_shim.py first if this is an experiment copy.",
            file=sys.stderr,
        )
        return 2

    args.rom = args.rom.resolve()
    args.source = args.source.resolve()
    args.work = args.work.resolve()
    rom = args.rom.read_bytes()
    reference = rom[args.vma - 0x08000000 : args.end - 0x08000000]
    leaves = [parse_leaf(spec, rom) for spec in args.leaf]

    if not have("clang"):
        print("era_compiler_probe: ERROR: clang is needed for preprocessing", file=sys.stderr)
        return 2
    args.work.mkdir(parents=True, exist_ok=True)
    unit = preprocess(args.source, args.work / (args.source.stem + ".i"), "clang")

    # A bare `--extra-flag` (nargs='?' yields None) means "run with no extra flags".
    extra = tuple(f for f in args.extra_flag if f) if args.extra_flag is not None else DEFAULT_EXTRA_FLAGS
    rows: list[dict] = []
    for name, binary in sorted(find_agbcc().items()):
        for opt in AGBCC_OPT_LEVELS:
            blob, note = build_agbcc(binary, opt, unit, args.vma, args.work, extra)
            rows.append(record(name, opt, blob, note, reference, leaves))
    cc1 = find_devkit_cc1()
    if cc1:
        for opt in DEVKIT_OPT_LEVELS:
            blob, note = build_devkit(cc1, opt, unit, args.vma, args.work, extra)
            rows.append(record("gcc3.2.2", opt, blob, note, reference, leaves))
    else:
        rows.append({"compiler": "gcc3.2.2", "opt": "-", "status": "SKIP",
                     "note": f"no cc1.exe matching {DEVKIT_GLOB}"})

    width = max(len(r["compiler"]) for r in rows) if rows else 10
    print(f"reference {args.vma:#010x}..{args.end:#010x} ({len(reference)} bytes) from {args.rom.name}")
    print(f"extra flags on every candidate: {' '.join(extra) if extra else '(none)'}")
    print(f"{'compiler':<{width}}  {'opt':<4}{'size':>6}  result")
    print("-" * (width + 22))
    for row in rows:
        if row.get("status") == "SKIP":
            print(f"{row['compiler']:<{width}}  {'-':<4}{'--':>6}  SKIP: {row['note']}")
            continue
        if row.get("status") == "FAIL":
            print(f"{row['compiler']:<{width}}  {row['opt']:<4}{'--':>6}  FAIL: {row['note']}")
            continue
        extra = f"  leaves[{row['leaves']}]" if leaves else ""
        print(f"{row['compiler']:<{width}}  {row['opt']:<4}{row['size']:>6}  {row['result']}{extra}")

    if args.json:
        args.json.write_text(json.dumps(
            {
                "rom": args.rom.name,
                "source": display_path(args.source),
                "span": {"vma": f"{args.vma:#010x}", "end": f"{args.end:#010x}", "bytes": len(reference)},
                "extra_flags": list(extra),
                "results": rows,
            }, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print(f"\nJSON: {args.json}")
    return 0


def display_path(path: Path) -> str:
    """Repo-relative when possible, absolute otherwise (toolchains are gitignored)."""
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def record(name: str, opt: str, blob: Path | None, note: str | None, reference: bytes, leaves):
    if blob is None:
        return {"compiler": name, "opt": opt, "status": "FAIL", "note": note or "no output"}
    data = blob.read_bytes()
    entry = {
        "compiler": name,
        "opt": opt,
        "status": "OK",
        "size": len(data),
        "result": summarize(reference, data),
        "sha256": hashlib.sha256(data).hexdigest(),
    }
    if leaves:
        entry["leaves"] = leaf_hits(data, leaves)
    return entry


def self_test() -> int:
    checks = 0
    reference = bytes(range(16))
    if first_diff(reference, reference) is not None:
        raise AssertionError("identical buffers must report no difference")
    if first_diff(reference, reference[:8]) is not None:
        raise AssertionError("a shorter buffer must be caught by the length check")
    if first_diff(reference, reference[:4] + b"\xff" + reference[5:]) != 4:
        raise AssertionError("first_diff must locate the exact byte")
    checks += 3
    if summarize(reference, reference) != "MATCH (16/16 bytes)":
        raise AssertionError("summarize must call an exact prefix a match")
    if summarize(reference, reference[:8]) != "prefix match, length 8 != 16":
        raise AssertionError("summarize must flag a truncated candidate")
    checks += 2
    hits = leaf_hits(b"\x00\x01\x02\x03\x04", [("A", b"\x01\x02"), ("B", b"\xff")])
    if hits != "A:Y B:n":
        raise AssertionError(f"leaf_hits wrong: {hits}")
    checks += 1
    rom = bytearray(0x100)
    rom[0x20:0x24] = b"\xde\xad\xbe\xef"
    name, blob = parse_leaf("0x08000020:0x08000024", bytes(rom))
    if name != "08000020" or blob != b"\xde\xad\xbe\xef":
        raise AssertionError("parse_leaf must slice the ROM by VMA")
    checks += 1
    print(f"era_compiler_probe self-test: {checks}/{checks} checks PASS")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ProbeError as error:
        print(f"era_compiler_probe: ERROR: {error}", file=sys.stderr)
        raise SystemExit(2)
