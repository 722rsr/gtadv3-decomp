#!/usr/bin/env python3
"""Score a SaveUpdate_080241C8 draft body against the acceptance probe.

A draft file is the body of the function only:
    void SaveUpdate_080241C8(void) { ... }

splice() wraps it with the file-scope symbol block and puts it in src/save.c.
score() reports (first_diff, candidate_len, matched, status).
"""
from __future__ import annotations

import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SRC = ROOT / "src/save.c"
WORK = ROOT / "build/lift/save241c8"

SETS = "".join(
    f'__asm__(".set {n}, 0x{v}");\nextern u8 {n};\n'
    for n, v in [
        ("SaveWABase", "03001780"),
        ("SaveWAOffFF2", "00000FF2"),
        ("SaveWAOffFF6", "00000FF6"),
        ("SaveWAOffFFA", "00000FFA"),
        ("SaveWAOff101A", "0000101A"),
        ("SaveWAOff102A", "0000102A"),
        ("SaveWAOffNegFFA", "FFFFF006"),
        ("SaveWAOffNegFFA2", "FFFFF006"),
        ("SaveWAOffNegFF2", "FFFFF00E"),
    ]
)


ANCHOR = "// Zone-grid updater"


def splice(body: str) -> None:
    subprocess.run(["git", "checkout", "--", "src/save.c"], cwd=ROOT)
    t = SRC.read_text()
    if "__attribute__((naked)) void SaveUpdate_080241C8(void) {" in t:
        a = t.index("__attribute__((naked)) void SaveUpdate_080241C8(void) {")
    elif 'extern u8 SaveWAOffNegFF2;' in t:
        a = t.index('__asm__(".set SaveWABase')
    else:
        a = t.index("void SaveUpdate_080241C8(void) {")
    b = t.index('void _080241C8(void) __attribute__((alias("SaveUpdate_080241C8")));')
    SRC.write_text(t[:a] + SETS + "\n" + body + "\n\n" + t[b:])


def score(work: Path | None = None) -> tuple[int, int, int, str]:
    wd = work or (WORK / "probe-work")
    if wd.exists():
        shutil.rmtree(wd)
    out = subprocess.run(
        ["python3", "tools/corpus_match_probe.py", "--function",
         "SaveUpdate_080241C8", "--work-dir", str(wd)],
        cwd=ROOT, capture_output=True, text=True).stdout
    line = next((l for l in out.splitlines()
                 if "0x080241c8" in l and "bytes" in l), "")
    m = re.search(r":\s+([A-Z_]+)\s+(\d+)/(\d+) bytes, candidate (\d+) bytes, first difference (\S+)", line)
    if not m:
        skip = next((l for l in out.splitlines() if "SKIPPED" in l), "")
        return (-1, 0, 0, skip.strip() or "ERR")
    status, matched, span, candlen, first = m.groups()
    if first == "none":
        fd = 0x170
    elif first.startswith("+"):
        fd = int(first[1:], 16)
    else:
        fd = -1
    return (fd, int(candlen), int(matched), status)


def run_file(path: Path) -> tuple[int, int, int, str]:
    splice(path.read_text())
    return score()


if __name__ == "__main__":
    for p in sys.argv[1:]:
        fd, ln, mb, st = run_file(Path(p))
        print(f"{Path(p).name:24s} first=+0x{fd:02X} len={ln} matched={mb} {st}")
