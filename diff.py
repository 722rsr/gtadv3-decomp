#!/usr/bin/env python3
"""Run asm-differ for GT Advance 3.

Usage:
  python3 diff.py [options] <symbol_or_address>
  ./diff.py _08006C10
"""
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
DIFF_DIR = ROOT / "tools/asm-differ"
DIFF_SCRIPT = DIFF_DIR / "diff.py"

if not DIFF_SCRIPT.is_file():
    setup_script = ROOT / "tools/build_asm_differ.sh"
    if setup_script.is_file():
        subprocess.run([str(setup_script)], cwd=ROOT, check=True)
    else:
        print(f"Error: asm-differ not found at {DIFF_DIR}", file=sys.stderr)
        sys.exit(1)

sys.path.insert(0, str(DIFF_DIR))
import diff

if __name__ == "__main__":
    diff.main()
