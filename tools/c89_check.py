#!/usr/bin/env python3
"""Verify every maintained C translation unit compiles with pinned old_agbcc.

The source contract is C89: `src/*.c` is handed to the matching compiler
directly, with no C99-to-C89 rewrite step. This gate preprocesses and compiles
every maintained TU and reports the compiler's own diagnostic for any file that
violates the contract. It is intentionally independent of the byte-match probe,
which only compiles TUs that own at least one selected function.

    python3 tools/c89_check.py
    python3 tools/c89_check.py --self-test
"""
from __future__ import annotations

import argparse
import concurrent.futures
import json
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from agbcc_options import options_for  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"
SRC = ROOT / "src"
INCLUDE = [ROOT / "include", ROOT / "asm", ROOT / "build/toolchains/agbcc/ginclude"]


def inc_flags() -> list[str]:
    return [flag for directory in INCLUDE if directory.is_dir()
            for flag in ("-I", str(directory))]


def run(argv: list[str], stdout=None) -> subprocess.CompletedProcess:
    return subprocess.run([str(arg) for arg in argv], text=True,
                          stdout=stdout if stdout is not None else subprocess.PIPE,
                          stderr=subprocess.PIPE)


def compile_one(source: Path, work: Path) -> tuple[bool, str, str]:
    """Preprocess and compile one file directly; `(ok, stage, diagnostic)`."""
    pre = work / f"{source.stem}.i"
    obj = work / f"{source.stem}.s"
    with pre.open("w", encoding="utf-8") as handle:
        preprocess = run(["clang", "-E", "-nostdinc", "-undef", *inc_flags(), str(source)],
                         stdout=handle)
    if preprocess.returncode:
        return False, "preprocess", (preprocess.stderr or "").strip()
    result = run([AGBCC, *options_for(source.name, function_sections=False),
                  str(pre), "-o", str(obj)])
    if result.returncode:
        return False, "agbcc", (result.stderr or "").strip()
    return True, "OK", ""


def check(files: list[Path]) -> list[dict]:
    def one(work: Path, path: Path) -> dict:
        ok, stage, diagnostic = compile_one(path, work)
        return {"source": str(path.relative_to(ROOT)), "ok": ok,
                "stage": stage, "diagnostic": diagnostic[:2000]}

    with tempfile.TemporaryDirectory(prefix="gtadv-c89-check-") as td:
        work = Path(td)
        workers = min(len(files), 16) or 1
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            return list(pool.map(lambda path: one(work, path), files))


def self_test() -> int:
    """Prove the gate accepts C89 and rejects the C99 form it exists to catch."""
    if not AGBCC.exists():
        print("c89_check self-test: SKIP (agbcc not built)")
        return 0
    with tempfile.TemporaryDirectory(prefix="gtadv-c89-gate-") as td:
        work = Path(td)
        c89 = work / "c89_fixture.c"
        c89.write_text("int f(int n) { int i; int c = 0;\n"
                       "    for (i = 0; i < n; i++) c += i;\n"
                       "    return c; }\n", encoding="utf-8")
        c99 = work / "c99_fixture.c"
        c99.write_text("int f(int n) { int c = 0;\n"
                       "    for (int i = 0; i < n; i++) c += i;\n"
                       "    return c; }\n", encoding="utf-8")
        ok_c89, _, diag_c89 = compile_one(c89, work)
        ok_c99, _, _ = compile_one(c99, work)
        if not ok_c89:
            print(f"c89_check self-test: C89 fixture rejected: {diag_c89}", file=sys.stderr)
            return 1
        if ok_c99:
            print("c89_check self-test: C99 fixture accepted; the gate cannot "
                  "detect a C99 regression", file=sys.stderr)
            return 1
        print("c89_check self-test: C89 fixture accepted, C99 fixture rejected PASS")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--json", type=Path)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()

    if args.self_test:
        return self_test()
    if not AGBCC.exists():
        print("c89_check: agbcc not built", file=sys.stderr)
        return 2
    files = sorted(SRC.glob("*.c"))
    records = check(files)
    failures = [record for record in records if not record["ok"]]
    print(f"C89 source gate: {len(records) - len(failures)}/{len(records)} "
          "translation units compile directly with old_agbcc")
    for record in failures:
        first = record["diagnostic"].splitlines()
        print(f"  {record['source']:<32} [{record['stage']}] "
              f"{first[0] if first else 'no diagnostic'}")
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps({"total": len(records),
                                         "accepted": len(records) - len(failures),
                                         "files": records}, indent=2,
                                        sort_keys=True) + "\n", encoding="utf-8")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
