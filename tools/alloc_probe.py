#!/usr/bin/env python3
"""Global-alloc oracle: why did agbcc assign the registers it did?

Compiles a translation unit with `-dg` (global_alloc's debug dump) and prints,
for one function:

  * the sorted allocation order with each allocno's computed priority --
    `floor_log2(refs) * refs / live_length * size`, ties by allocno number
    (the formula docs/findings/register_shaping_levers.md measured),
  * the conflict set of every allocno that still needs a hard register,
    in hard-register names,
  * the final register dispositions,
  * a short RTL context line per allocno so source variables can be mapped
    to their register numbers.

Use this instead of guessing at barrier counts: place a temp's priority
relative to another's and read off the chain consequence directly.

Usage:
    python3 tools/alloc_probe.py src/course_collision.c --function Course_CollisionLoop
    python3 tools/alloc_probe.py src/foo.c --function _08007110 --c89 --rtl
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(Path(__file__).resolve().parent))

from agbcc_options import options_for as agbcc_options  # noqa: E402

AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"
INCLUDE = [ROOT / "include", ROOT / "asm", ROOT / "build/toolchains/agbcc/ginclude"]

REG_NAMES = {10: "sl", 11: "fp", 12: "ip", 13: "sp", 14: "lr", 15: "pc"}


def reg_name(n: int) -> str:
    return REG_NAMES.get(n, f"r{n}")


def inc_flags() -> list[str]:
    out: list[str] = []
    for d in INCLUDE:
        if d.is_dir():
            out += ["-I", str(d)]
    return out


def run(argv) -> subprocess.CompletedProcess:
    return subprocess.run([str(a) for a in argv], text=True, stderr=subprocess.PIPE)


def floor_log2(n: int) -> int:
    return n.bit_length() - 1


def priority(refs: int, live_length: int, size: int) -> float:
    if live_length <= 0 or refs <= 0:
        return 0.0
    return floor_log2(refs) * refs * size / live_length


SORTED_RE = re.compile(
    r"Register (\d+), refs = (\d+), live_length = (\d+), size = (\d+)")
ALLOC_RE = re.compile(r";; (\d+) regs to allocate: ([\d ]+)")
CONF_RE = re.compile(r";; (\d+) conflicts: ([\d ]+)")
DISP_RE = re.compile(r"(\d+) in (\d+)")


def function_section(greg: str, function: str) -> str | None:
    start = greg.find(f";; Function {function}\n")
    if start < 0:
        return None
    nxt = greg.find(";; Function ", start + 1)
    return greg[start:nxt if nxt > 0 else None]


def rtl_context(section: str, regno: int, limit: int = 110) -> str:
    """First RTL mention of a pseudo, collapsed to one readable line."""
    pat = re.compile(rf"\(reg(/v)?:SI {regno}(?=[ )\n])")
    m = pat.search(section)
    if not m:
        return "(not referenced in body RTL)"
    # Walk back to the start of the enclosing insn/expression line.
    line_start = section.rfind("\n", 0, m.start()) + 1
    text = section[line_start:line_start + limit * 2]
    text = re.sub(r"\s+", " ", text).strip()
    return text[:limit]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("source", type=Path, help="translation unit (src/...)")
    ap.add_argument("--function", required=True, help="function name as in the dump")
    ap.add_argument("--c89", action="store_true",
                    help="apply the C89 transform before compiling (as the probe does)")
    ap.add_argument("--rtl", action="store_true",
                    help="also print each allocno's first RTL context line")
    ap.add_argument("--work-dir", type=Path, default=Path("build/alloc_probe"))
    args = ap.parse_args()

    if not AGBCC.exists():
        print(f"alloc_probe: agbcc not built at {AGBCC}", file=sys.stderr)
        return 2

    work = ROOT / args.work_dir
    work.mkdir(parents=True, exist_ok=True)

    source = args.source if args.source.is_absolute() else ROOT / args.source
    compile_path = source
    if args.c89:
        from agbcc_c89_transform import cached_transform_file
        transformed, stats = cached_transform_file(source)
        if stats["unhandled"] is not None:
            print(f"alloc_probe: C89 transform unhandled: {stats['unhandled']}",
                  file=sys.stderr)
            return 2
        compile_path = work / source.name
        compile_path.write_text(transformed, encoding="utf-8")

    pre = work / (source.stem + ".i")
    out_s = work / (source.stem + ".s")
    with pre.open("w", encoding="utf-8"):
        r = run(["clang", "-E", "-nostdinc", "-undef", *inc_flags(), compile_path],
                )
        if r.returncode:
            # run() pipes stdout? No: stdout defaults to inherit for clang here.
            print(f"alloc_probe: preprocess failed:\n{r.stderr}", file=sys.stderr)
            return 2
        # clang wrote to stdout only if not redirected; redo with capture.
    # Preprocess properly with stdout capture:
    r = subprocess.run(["clang", "-E", "-nostdinc", "-undef", *inc_flags(),
                        str(compile_path)], text=True, capture_output=True)
    if r.returncode:
        print(f"alloc_probe: preprocess failed:\n{r.stderr}", file=sys.stderr)
        return 2
    pre.write_text(r.stdout, encoding="utf-8")

    options = agbcc_options(source)
    r = run([AGBCC, *options, "-dg", str(pre), "-o", str(out_s)])
    greg = pre.with_suffix(".i.greg")
    if not greg.exists():
        greg = Path(str(pre) + ".greg")
    if r.returncode or not greg.exists():
        print(f"alloc_probe: agbcc compile failed:\n{r.stdout}\n{r.stderr}",
              file=sys.stderr)
        return 2

    section = function_section(greg.read_text(errors="replace"), args.function)
    if section is None:
        print(f"alloc_probe: function {args.function} not in {greg}", file=sys.stderr)
        return 2

    # Sorted order as printed, with the priority we can independently compute.
    order: list[tuple[int, int, int, int]] = []
    for m in SORTED_RE.finditer(section):
        reg, refs, ll, size = (int(g) for g in m.groups())
        order.append((reg, refs, ll, size))

    alloc_m = ALLOC_RE.search(section)
    need = set(int(x) for x in alloc_m.group(2).split()) if alloc_m else set()
    conflicts = {int(m.group(1)): [int(x) for x in m.group(2).split()]
                 for m in CONF_RE.finditer(section)}
    dispositions = {int(a): int(b) for a, b in DISP_RE.findall(section)}

    def hard(regs: list[int]) -> str:
        return " ".join(reg_name(x) for x in regs)

    print(f"== {args.function}: allocation order (priority = floor_log2(refs)"
          f" * refs / live_length * size) ==")
    prev = None
    for reg, refs, ll, size in order:
        p = priority(refs, ll, size)
        flag = " <== ALLOCATE" if reg in need else ""
        note = ""
        if prev is not None and p > prev + 1e-9:
            note = "  [priority not monotonic?!]"
        disp = dispositions.get(reg)
        disp_s = f" -> {reg_name(disp)}" if disp is not None else ""
        print(f"  reg {reg:>3}: refs={refs:<3} ll={ll:<4} pri={p:<7.3f} "
              f"assigned{disp_s}{flag}{note}")
        prev = p

    print("\n== conflicts of allocatable regs (hard regs they cannot use) ==")
    for reg in sorted(need):
        c = conflicts.get(reg, [])
        free = [x for x in range(16) if x not in c]
        print(f"  reg {reg}: conflicts {hard(c)}")
        print(f"           free at its turn: {hard(free)}")

    print("\n== final dispositions ==")
    for hreg in sorted(set(dispositions.values())):
        print(f"  {reg_name(hreg)} <- pseudo {dispositions and [p for p, d in dispositions.items() if d == hreg]}")

    if args.rtl:
        print("\n== RTL context per allocatable reg ==")
        for reg in sorted(need):
            print(f"  reg {reg}: {rtl_context(section, reg)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
