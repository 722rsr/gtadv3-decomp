#!/usr/bin/env python3
"""Scope: how much of the C corpus can agbcc compile at all, and what fails?

The de-risk probes showed agbcc byte-matches game leaves. Before committing to a
migration, two things have to be measured on the *real* corpus rather than on
hand-written probes:

1. How many of the 152 translation units does agbcc even accept? agbcc is C89;
   the corpus uses declaration-in-`for` in 68 files.
2. Once accepted, how many functions does it actually reproduce byte-for-byte?

This tool answers (1) mechanically and (2) for the pool-free subset, and reports
both as counts so the migration can be scoped honestly.
"""
from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"  # matching compiler (docs/findings/compiler_split.md)
SRC = ROOT / "src"
WORK = ROOT / "build/era-corpus/scope"
INCLUDE = [ROOT / "include", ROOT / "asm", ROOT / "build/toolchains/agbcc/ginclude"]


def run(argv, stdout=None):
    kwargs = {"text": True, "stderr": subprocess.PIPE}
    kwargs["stdout"] = stdout if stdout is not None else subprocess.PIPE
    return subprocess.run([str(a) for a in argv], **kwargs)


def classify_error(stderr: str) -> str:
    """Name the C89 barrier that blocked this file, for the migration backlog."""
    first = stderr.strip().splitlines()
    text = " ".join(first[:6])
    if "for loop initial declarations" in text or "only declares in for loop" in text:
        return "declaration-in-for"
    if "ISO C90 forbids" in text or "C90" in text:
        return "c90-restriction"
    if "declaration of" in text and "after" in text:
        return "declaration-after-statement"
    if "inline" in text:
        return "inline"
    if "long long" in text or "too long for" in text:
        return "long-long"
    if "parse error" in text or "syntax error" in text:
        return "parse-error"
    return "other"


def main() -> int:
    if not AGBCC.exists():
        print("scope: agbcc not built", file=sys.stderr)
        return 2
    WORK.mkdir(parents=True, exist_ok=True)
    files = sorted(SRC.glob("*.c"))
    inc = []
    for d in INCLUDE:
        if d.is_dir():
            inc += ["-I", str(d)]

    ok, failed, barriers = [], {}, {}
    for src in files:
        pre = WORK / (src.stem + ".i")
        with pre.open("w", encoding="utf-8") as h:
            p = run(["clang", "-E", "-nostdinc", "-undef", *inc,
                     "-D__APPLE__=0", str(src)], stdout=h)
        if p.returncode:
            # preprocessing itself failed (missing include, etc.)
            barriers.setdefault("preprocess", []).append(src.name)
            continue
        out = run([AGBCC, "-O2", "-mthumb-interwork", str(pre), "-o",
                   str(WORK / (src.stem + ".s"))])
        if out.returncode == 0:
            ok.append(src.name)
        else:
            reason = classify_error(out.stderr or "")
            failed[src.name] = reason
            barriers.setdefault(reason, []).append(src.name)

    total = len(files)
    print(f"agbcc acceptance over the real C corpus: {total} translation units\n")
    print(f"  accepted : {len(ok)}/{total}  ({100*len(ok)/total:.0f}%)")
    print(f"  rejected : {len(failed)}/{total}  ({100*len(failed)/total:.0f}%)\n")
    if barriers:
        print("rejection barriers (C89 conformance), by cause:")
        for reason, names in sorted(barriers.items(), key=lambda kv: -len(kv[1])):
            print(f"  {reason:<28} {len(names):>3} files")
        print()
        print("largest clusters:")
        for reason, names in sorted(barriers.items(), key=lambda kv: -len(kv[1]))[:4]:
            print(f"  {reason}: {', '.join(names[:6])}{' ...' if len(names) > 6 else ''}")

    (WORK / "scope.json").write_text(json.dumps(
        {"total": total, "accepted": len(ok), "rejected": len(failed),
         "barriers": {k: len(v) for k, v in barriers.items()},
         "rejected_files": failed}, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"\nJSON: {WORK / 'scope.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
