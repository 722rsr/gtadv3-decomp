#!/usr/bin/env python3
"""Run decomp-permuter-agbcc for a target function in GT Advance 3.

Sets up a self-contained permuter scratch directory with:
  - base.c: preprocessed C89 source for the target function
  - target.o: reference target object extracted from the independent code slice
  - compile.sh: wrapper invoking the repo's local agbcc compiler
  - settings.toml: gcc compiler mode settings

Usage:
  python3 tools/run_permuter.py <function_name> [--work-dir DIR] [permuter args...]
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PERMUTER_DIR = ROOT / "build/toolchains/decomp-permuter-agbcc"
AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"  # matching compiler (docs/findings/compiler_split.md)
CODE_O = ROOT / "build-code/code.o"
INCLUDE = [ROOT / "include", ROOT / "asm", ROOT / "build/toolchains/agbcc/ginclude"]


def ensure_tools() -> None:
    if not PERMUTER_DIR.is_dir():
        subprocess.run(["tools/build_permuter.sh"], cwd=ROOT, check=True)
    if not AGBCC.is_file():
        raise RuntimeError(f"agbcc not found at {AGBCC}")
    if not CODE_O.is_file():
        subprocess.run(["make", "code-objects"], cwd=ROOT, check=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("function", help="Target function symbol (e.g. _08006C10)")
    parser.add_argument("--work-dir", default=None, help="Directory to store permuter workspace")
    parser.add_argument("permuter_args", nargs=argparse.REMAINDER, help="Arguments passed to permuter.py")
    args = parser.parse_args()

    ensure_tools()

    work_dir = Path(args.work_dir) if args.work_dir else ROOT / f"build/permuter/{args.function}"
    work_dir.mkdir(parents=True, exist_ok=True)

    # 1. Extract target object symbol into target.o
    target_o = work_dir / "target.o"
    proc = subprocess.run(
        ["arm-none-eabi-objcopy", "-j", ".text", "-N", "dummy", str(CODE_O), str(target_o)],
        capture_output=True, text=True
    )
    if proc.returncode != 0:
        print(f"Error copying object: {proc.stderr}", file=sys.stderr)
        return 1

    # 2. Generate settings.toml
    settings_toml = work_dir / "settings.toml"
    settings_toml.write_text(
        f'func_name = "{args.function}"\n'
        f'compiler_type = "gcc"\n\n'
        f'[weight_overrides]\n'
        f'perm_temp_for_expr = 100\n',
        encoding="utf-8"
    )

    # 3. Generate compile.sh
    compile_sh = work_dir / "compile.sh"
    inc_flags = " ".join(f"-I{inc}" for inc in INCLUDE)
    compile_sh.write_text(
        f"#!/bin/bash\n"
        f"set -e\n"
        f'IN_C="$1"\n'
        f'OUT_O="$3"\n'
        f'TMP_S="$(mktemp /tmp/perm_XXXXXX).s"\n'
        f'"{AGBCC}" -O2 -mthumb-interwork -ffunction-sections {inc_flags} "$IN_C" -o "$TMP_S"\n'
        f'arm-none-eabi-as -mcpu=arm7tdmi "$TMP_S" -o "$OUT_O"\n'
        f'rm -f "$TMP_S"\n',
        encoding="utf-8"
    )
    compile_sh.chmod(0o755)

    print(f"Permuter workspace prepared at: {work_dir}")
    print(f"Run permuter with:")
    print(f"  python3 {PERMUTER_DIR}/permuter.py {work_dir} {' '.join(args.permuter_args)}")

    permuter_py = PERMUTER_DIR / "permuter.py"
    if permuter_py.is_file():
        cmd = [sys.executable, str(permuter_py), str(work_dir)] + args.permuter_args
        return subprocess.run(cmd).returncode
    return 0


if __name__ == "__main__":
    sys.exit(main())
