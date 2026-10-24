#!/usr/bin/env python3
"""Score alternative C spellings of ONE function body against the ROM.

This automates the loop the matching docs describe by hand: write a body,
probe it, read the diff, change one spelling, re-probe. Each variant file is a
complete replacement for the span between two markers in the real TU; the
harness swaps it in, probes, and always restores the original bytes before the
next variant (and on exit). Nothing is staged or promoted.

    python3 tools/lab_sweep.py --file src/foo.c --function _08026B80 \\
        --start 'void Sprite_MenuEvent(int ev) {' \\
        --end 'void _08026B80(int a)' variants/*.c

Prints one line per variant: matched bytes, status, candidate length, first
difference offset, then the best variant.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def score(function: str, work: Path) -> dict:
    out = work / "score.json"
    subprocess.run(
        [sys.executable, str(ROOT / "tools/corpus_match_probe.py"),
         "--c89", "--function", function, "--work-dir", str(work / "probe"),
         "--json", str(out)],
        cwd=ROOT, capture_output=True, text=True)
    if not out.exists():
        return {}
    data = json.loads(out.read_text())
    for row in data.get("results", []):
        if row["name"] == function:
            return row
    return {}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--file", required=True)
    ap.add_argument("--function", required=True)
    ap.add_argument("--start", required=True)
    ap.add_argument("--end", required=True)
    ap.add_argument("variants", nargs="+")
    args = ap.parse_args()

    path = ROOT / args.file
    original = path.read_text()
    start = original.index(args.start)
    end = original.index(args.end, start)

    best = None
    try:
        with tempfile.TemporaryDirectory(dir=ROOT / "build", prefix="sweep-") as td:
            work = Path(td)
            for vpath in args.variants:
                body = Path(vpath).read_text()
                path.write_text(original[:start] + body + "\n" + original[end:])
                row = score(args.function, work)
                print("%5d  %-22s cand=%-5s first=%-6s %s" % (
                    row.get("matched_bytes", 0), row.get("status", "?"),
                    row.get("cand_bytes"), row.get("first_diff"), vpath))
                key = (row.get("matched_bytes", 0), -abs(
                    (row.get("cand_bytes") or 0) - (row.get("rom_bytes") or 0)))
                if best is None or key > best[0]:
                    best = (key, vpath)
    finally:
        path.write_text(original)
    print("best:", best[1] if best else None)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
