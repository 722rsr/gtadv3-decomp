#!/usr/bin/env python3
"""Compile ONE C snippet with the pinned agbcc and print what it emits.

The matching guide repeatedly asks a question that a whole-function probe
answers only slowly: *which spelling of this expression produces the ROM's
exact instruction pair?* The documented route is to hand-build a scratch TU
(`build/foldprobe.c`), compile it with `-O2 -mthumb-interwork`, and objdump it.
This tool is that loop, reproducible and with no leftover state:

    python3 tools/expr_probe.py --snippet 'int f(u8*a){return *(u16*)(a+148);}'
    python3 tools/expr_probe.py --file /tmp/spell.c --fn f
    python3 tools/expr_probe.py --self-test

A short prelude with the usual project typedefs is prepended unless
`--no-prelude` is given, so a snippet can be one function body. `--keep DIR`
leaves the `.i`/`.s`/`.o` behind for objdump or `-da` RTL inspection.

This is a *diagnostic*. It scores nothing against the ROM: an expression that
emits the instruction pair you wanted says nothing about whether the whole
body allocates the same registers, which is why the answer still has to be
re-measured in the real translation unit with `corpus_match_probe.py`.

Measured on the pinned compiler (2026-10-24, `--self-test` pins both): a
three-operand Thumb `add` keeps its source operand order when the destination
is a *pinned* register, but agbcc commutes it once the destination is
coalesced with one of the operands, so an accumulate (`t = t + x`) always
encodes the added value first. A ROM `adds r4, r4, r0` therefore cannot come
from `t += x`; it needs a distinct destination variable that the allocator
later places in the first operand's register.
"""
from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"
INCLUDE = [ROOT / "include", ROOT / "asm",
           ROOT / "build/toolchains/agbcc/ginclude"]
PRELUDE = ("typedef unsigned char u8;\ntypedef signed char s8;\n"
           "typedef unsigned short u16;\ntypedef short s16;\n"
           "typedef unsigned int u32;\ntypedef int s32;\n"
           "typedef volatile unsigned char vu8;\n"
           "typedef volatile unsigned short vu16;\n"
           "typedef volatile unsigned int vu32;\n"
           "typedef unsigned int uintptr_t;\n")

SECTION = re.compile(r"^Disassembly of section (\S+):")


class Unavailable(Exception):
    """The pinned toolchain is not present (or a stage failed)."""


def inc_flags() -> list[str]:
    out: list[str] = []
    for d in INCLUDE:
        if d.is_dir():
            out += ["-I", str(d)]
    return out


def build(source: str, work: Path) -> Path:
    """Preprocess, agbcc-compile and assemble `source`; return the object."""
    if not AGBCC.exists():
        raise Unavailable(f"missing pinned compiler: {AGBCC} (run make toolchain)")
    for tool in ("clang", "arm-none-eabi-as", "arm-none-eabi-objdump"):
        if shutil.which(tool) is None:
            raise Unavailable(f"missing {tool} on PATH")
    c_path = work / "snippet.c"
    i_path = work / "snippet.i"
    s_path = work / "snippet.s"
    o_path = work / "snippet.o"
    c_path.write_text(source, encoding="utf-8")
    steps = [
        ["clang", "-E", "-nostdinc", "-undef", *inc_flags(), str(c_path),
         "-o", str(i_path)],
        [str(AGBCC), "-O2", "-mthumb-interwork", "-ffunction-sections",
         str(i_path), "-o", str(s_path)],
        ["arm-none-eabi-as", "-mcpu=arm7tdmi", str(s_path), "-o", str(o_path)],
    ]
    for argv in steps:
        res = subprocess.run(argv, cwd=ROOT, capture_output=True, text=True)
        if res.returncode != 0:
            raise Unavailable(f"{Path(argv[0]).name} failed:\n"
                              f"{res.stderr.strip() or res.stdout.strip()}")
    return o_path


def parse_disassembly(text: str) -> dict[str, list[str]]:
    """Split `objdump -d` output into per-section instruction streams.

    Columns are tab separated and the byte column is a single 4-digit group
    for Thumb (`\t3094      \t`) versus eight space-separated pairs for ARM,
    so the split is on tabs rather than a byte-shaped pattern.
    """
    out: dict[str, list[str]] = {}
    current = None
    for line in text.splitlines():
        m = SECTION.match(line)
        if m:
            current = m.group(1)
            out[current] = []
            continue
        if current is None or "\t" not in line:
            continue
        parts = line.split("\t")
        head = parts[0].strip()
        if not head.endswith(":") or len(parts) < 3:
            continue
        try:
            offset = int(head[:-1], 16)
        except ValueError:
            continue
        operands = parts[3].rstrip() if len(parts) > 3 else ""
        insn = parts[2] + (" " + operands if operands else "")
        out[current].append(f"+0x{offset:02x}  {insn}")
    return out


def sections(obj: Path) -> dict[str, list[str]]:
    res = subprocess.run(["arm-none-eabi-objdump", "-d", str(obj)],
                         capture_output=True, text=True)
    if res.returncode != 0:
        raise Unavailable(f"objdump failed:\n{res.stderr.strip()}")
    return parse_disassembly(res.stdout)


def run(source: str, keep: Path | None = None) -> dict[str, list[str]]:
    with tempfile.TemporaryDirectory(dir=ROOT / "build", prefix="expr-") as td:
        work = Path(td)
        obj = build(source, work)
        result = sections(obj)
        if keep is not None:
            keep.mkdir(parents=True, exist_ok=True)
            for name in ("snippet.c", "snippet.i", "snippet.s", "snippet.o"):
                shutil.copy2(work / name, keep / name)
    return result


def render(result: dict[str, list[str]], only: str | None, out) -> None:
    shown = 0
    for section, insns in result.items():
        if not section.startswith(".text."):
            continue
        name = section[len(".text."):]
        if only and name != only:
            continue
        shown += 1
        print(f"{name}  ({len(insns)} instructions)", file=out)
        for insn in insns:
            print(f"    {insn}", file=out)
    if only and not shown:
        raise Unavailable(f"no function named {only!r} in the snippet")


def self_test() -> int:
    """Pin the two measured add shapes, and that a snippet really compiles."""
    failures = 0

    def check(what: str, ok: bool) -> None:
        nonlocal failures
        if not ok:
            failures += 1
        print(("PASS " if ok else "FAIL ") + what)

    if not AGBCC.exists():
        print(f"SKIP self-test: {AGBCC} missing (run make toolchain)")
        return 0
    pinned = run(PRELUDE + (
        "extern void use(int);\n"
        "int a1(u8 *p, u8 *o) {\n"
        "    register int base __asm__(\"r4\");\n"
        "    u16 scale = *(u16 *)(p + 148);\n"
        "    base = scale;\n"
        "    base = base + *(u16 *)(p + 126);\n"
        "    o[0] = (u8)base;\n"
        "    use(base);\n"
        "    return 0;\n"
        "}\n"))
    body = pinned.get(".text.a1", [])
    check("a pinned destination keeps the source operand order",
          any(i.endswith("adds r4, r2, r0") for i in body))
    accum = run(PRELUDE + (
        "extern void use(int);\n"
        "int a2(u8 *p, u8 *o) {\n"
        "    register int t __asm__(\"r4\");\n"
        "    t = *(u16 *)(p + 148);\n"
        "    o[0] = (u8)(t << 2);\n"
        "    t = t + *(u16 *)(p + 126);\n"
        "    use(t);\n"
        "    return 0;\n"
        "}\n"))
    body = accum.get(".text.a2", [])
    check("an accumulate commutes the added value first",
          any(i.endswith("adds r4, r0, r4") for i in body))
    check("a snippet with no prelude still compiles through the same path",
          ".text.f" in run("int f(void) { return 1; }\n", keep=None))
    print(f"expr_probe self-test: {'PASS' if not failures else 'FAIL'}")
    return 0 if not failures else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    src = ap.add_mutually_exclusive_group()
    src.add_argument("--snippet", help="C text to compile")
    src.add_argument("--file", help="file with C text ('-' for stdin)")
    ap.add_argument("--fn", help="show only this function")
    ap.add_argument("--no-prelude", action="store_true",
                    help="compile the snippet verbatim")
    ap.add_argument("--keep", help="copy .c/.i/.s/.o into this directory")
    ap.add_argument("--json", help="write the instruction streams here")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        return self_test()
    if args.snippet:
        text = args.snippet
    elif args.file:
        text = sys.stdin.read() if args.file == "-" else \
            Path(args.file).read_text(encoding="utf-8")
    else:
        ap.error("one of --snippet/--file/--self-test is required")
    source = text if args.no_prelude else PRELUDE + text
    try:
        result = run(source, Path(args.keep) if args.keep else None)
        if args.json:
            Path(args.json).parent.mkdir(parents=True, exist_ok=True)
            Path(args.json).write_text(json.dumps(result, indent=2) + "\n")
        render(result, args.fn, sys.stdout)
    except Unavailable as exc:
        print(f"expr_probe: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
