#!/usr/bin/env python3
"""Check every src/*.c for calls to symbols that vanish in the host build.

The hybrid ARM build and the macOS host build see different symbol sets. A
VMA-shaped alias such as `sub_080040F8` is defined inside `#ifndef __APPLE__`,
because the host suite has no such symbol. A call to one therefore compiles
cleanly for ARM and becomes a **C89 implicit declaration** for the host -- and
a host link with `-undefined dynamic_lookup` leaves the symbol
unbound while the binary still links.

That combination makes the defect invisible by every gate the repo has:

* the ARM build, where the alias resolves.
* `make matching-ready` compiles for ARM and the C89 equivalence check compares
  ARM output against ARM output.

So the check is a one-line compile with `__APPLE__` defined and implicit
declarations promoted to errors, swept over *all* of `src/`. It costs about a
second and it is the only thing standing between a retarget to a closure
spelling and a silent host-build hole.

The right shape for such a call is the split, not a weak stub:

    #ifndef __APPLE__
        extern void sub_08004A0C(void);
        sub_08004A0C();
    #else
        extern void HeapReset(void);
        HeapReset();
    #endif

`HOST_STUB` is not a substitute. It expands to
`__attribute__((weak)) <decl>;` -- a declaration with no body -- so on Apple it
only silences the implicit-declaration warning; the call is still unbound
unless something else defines the symbol. The split makes the host call the
*real* body instead, which is what a test of that function wants.

`--record` rewrites the baseline file from the current sweep. Commit the result
when it shrinks; a sweep that reports the same set forever is a habit, not a
gate.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BASELINE = Path(__file__).with_name("apple_decls_baseline.json")
IMPLICIT = re.compile(r"call to undeclared function '([A-Za-z_0-9]+)'")

# A declaration the file itself cannot fix. `CpuFastSet` and the agbcc runtime
# helpers are declared in headers the host build does not see; `Div`/`DivRem`/
# `MathRot` are wrappers over them. These belong in their own task -- the point
# of the baseline is to hold them still, not to make them disappear here.
KNOWN_UNRELATED = {
    "CpuFastSet", "Div", "DivRem", "MathRot", "TimeStr_03E34",
}


def _rel(path: Path) -> str:
    """Repo-relative when possible; absolute for the self-test's scratch TUs."""
    try:
        return str(path.resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def sweep(files: list[Path]) -> list[tuple[str, str]]:
    """Return `(relpath, symbol)` for every undeclared call under `__APPLE__`."""
    def sweep_one(path: Path) -> list[tuple[str, str]]:
        proc = subprocess.run(
            ["clang", "-D__APPLE__", "-fsyntax-only", "-Iinclude", "-I.",
             "-Werror=implicit-function-declaration", str(path)],
            cwd=ROOT, capture_output=True, text=True,
        )
        return [(_rel(path), name) for name in IMPLICIT.findall(proc.stderr)]

    workers = min(os.cpu_count() or 4, len(files), 16)
    if len(files) <= 1:
        results = [sweep_one(f) for f in files]
    else:
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            results = list(pool.map(sweep_one, files))

    found: list[tuple[str, str]] = []
    for r in results:
        found.extend(r)
    return sorted(set(found))


def new_entries(found, baseline) -> list[tuple[str, str]]:
    """Entries absent from the baseline. A SET difference, never a count.

    Comparing sizes would pass when a new hole appears and an old one is fixed
    in the same run, so the swap case in the self-test holds the total fixed and
    still expects this to report. It lives here, not inside the test, so a
    control can disable the line that actually runs.
    """
    return sorted((p, s) for p, s in found if (p, s) not in set(baseline))


def fixed_entries(found, baseline) -> list[tuple[str, str]]:
    """Baseline entries the sweep no longer reports."""
    return sorted(set(baseline) - set(found))



def self_test() -> int:
    """Pin the sweep and the baseline filter on scratch TUs, never on `src/`.

    A control that injects a regression into a real `src/*.c` and restores it
    proves the same thing while risking a corrupted tracked file if it is
    interrupted. These fixtures live in a temp dir, so the control is safe to
    run at any time -- which is the only kind worth having.

    The filter is set membership, never arithmetic. A gate that only asks
    "did the number go down" passes when a new hole appears and an old one is
    fixed in the same run, so the swap case below deliberately holds the total
    size fixed and still expects NEW.
    """
    ok = 0
    total = 0

    def check(name: str, got, want) -> None:
        nonlocal ok, total
        total += 1
        if got == want:
            ok += 1
        else:
            print(f"FAIL {name}: got {got!r} want {want!r}", file=sys.stderr)

    with tempfile.TemporaryDirectory(prefix="apple-decls-") as tmp:
        work = Path(tmp)

        def tu(name: str, body: str) -> Path:
            p = work / name
            p.write_text(body, encoding="utf-8")
            return p

        # A call to a name nothing defines is the defect this tool exists for.
        bad = tu("bad.c", "void caller(void) { sub_080040F8(); }\n")
        check("an undeclared call is reported",
              sweep([bad]), [(str(bad), "sub_080040F8")])

        # A clean TU must stay silent, or the sweep reports noise and the
        # baseline is worthless.
        good = tu("good.c", "void callee(void) { }\nvoid caller(void) { callee(); }\n")
        check("a declared call is not reported", sweep([good]), [])

        # A TU that fails to parse for an unrelated reason is not a clean file;
        # only the undeclared-call shape may count.
        broken = tu("broken.c", "this is not C at all ((\n")
        check("an unrelated parse error reports no call", sweep([broken]), [])

        # The comparison is set membership, not a count. These call the same
        # functions `main()` uses, so a control that neuters the filter fails
        # here by name instead of silently pinning set-comprehension semantics.
        baseline = {("a.c", "old"), ("b.c", "kept")}
        grown = [("a.c", "old"), ("b.c", "kept"), ("c.c", "new")]
        check("a new entry is NEW", new_entries(grown, baseline), [("c.c", "new")])
        # Same total, different contents -- a count-only gate would pass this.
        swapped = [("a.c", "new"), ("b.c", "kept")]
        check("a same-size replacement is still NEW",
              new_entries(swapped, baseline), [("a.c", "new")])
        check("a fixed entry is reported as fixed",
              fixed_entries(swapped, baseline), [("a.c", "old")])

    print(f"apple_decls self-test: {ok}/{total}")
    return 0 if ok == total else 1



def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--record", action="store_true",
                    help="rewrite the baseline from the current sweep")
    ap.add_argument("--self-test", action="store_true",
                    help="pin the sweep and the baseline filter on scratch TUs")
    args = ap.parse_args()
    if args.self_test:
        return self_test()

    files = sorted((ROOT / "src").glob("*.c"))
    if not files:
        print("no src/*.c found", file=sys.stderr)
        return 1
    found = sweep(files)

    if args.record:
        BASELINE.write_text(json.dumps(found, indent=1) + "\n", encoding="utf-8")
        print(f"recorded {len(found)} undeclared host call(s) in {BASELINE.name}")
        return 0

    baseline = {(p, s) for p, s in json.loads(BASELINE.read_text())} \
        if BASELINE.exists() else set()

    # A symbol already in the baseline is pre-existing debt, not a regression.
    new = new_entries(found, baseline)
    fixed = fixed_entries(found, baseline)

    for path, sym in fixed:
        print(f"  fixed: {path}: {sym}")
    if fixed:
        print(f"{len(fixed)} baseline entr(ies) fixed -- re-run with --record")
    if new:
        for path, sym in new:
            hint = ""
            if sym in KNOWN_UNRELATED:
                hint = "  (known unrelated, see KNOWN_UNRELATED)"
            print(f"  NEW: {path}: call to undeclared '{sym}' under __APPLE__{hint}",
                  file=sys.stderr)
        print(f"FAIL {len(new)} host-build hole(s) the gates cannot see: an "
              f"alias retarget needs the #ifndef __APPLE__ split at the CALL "
              f"site, not just at the declaration", file=sys.stderr)
        return 1
    print(f"apple_decls: PASS ({len(found)} undeclared host call(s), all "
          f"pre-existing and recorded in {BASELINE.name})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
