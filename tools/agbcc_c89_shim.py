#!/usr/bin/env python3
"""Generate C89-compatible source fixtures for era-compiler comparisons.

`agbcc` (pret/agbcc, a GCC 2.95.3 fork) is a C89 compiler and rejects C99
declaration-after-statement.  The repository's lifted C uses C99 freely, so the
compiler-matching harness cannot feed the real translation units to agbcc
unchanged.

This tool applies an explicit, single-occurrence-asserted patch table that only
moves declarations to the start of their enclosing block, or wraps the tail of a
block in a nested block.  Semantics are unchanged: no statement is reordered
across a call and no initializer is evaluated earlier than before.  Output goes
to a generated directory; the checked-in sources are never modified.

    python3 tools/agbcc_c89_shim.py --out build/toolchains/agbcc/experiment
    python3 tools/agbcc_c89_shim.py --check

`--check` verifies the pinned source hashes and that every patch still matches
exactly once, without writing anything.
"""
from __future__ import annotations

import argparse
import hashlib
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUT = ROOT / "build/toolchains/agbcc/experiment"

# Each entry pins the exact source revision it was written against, so a source
# edit cannot silently invalidate the experiment.
PATCHES: dict[str, dict] = {
    "src/keypad.c": {
        "expected_sha256": "ad4436227b61b81349376cd9a963def376c7d47348afbe3383f24f1a5a7959b2",
        "notes": "KeypadPoll: hoist the only C99 mid-block declaration (lastHeld).",
        "patches": [
            (
                "    volatile KeypadState *st = KEYPAD_STATE;\n",
                "    u16 lastHeld;\n    volatile KeypadState *st = KEYPAD_STATE;\n",
            ),
            (
                "    u16 lastHeld = st->lastHeld;\n",
                "    lastHeld = st->lastHeld;\n",
            ),
        ],
    },
}


class ShimError(RuntimeError):
    pass


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def apply_patches(source: str, spec: dict, name: str) -> str:
    text = source
    for index, (old, new) in enumerate(spec["patches"]):
        count = text.count(old)
        if count != 1:
            raise ShimError(
                f"{name}: patch {index} matched {count} times, expected exactly 1:\n"
                f"--- pattern ---\n{old}--- end ---"
            )
        text = text.replace(old, new)
    return text


def build(entry: str, spec: dict) -> tuple[str, str]:
    path = ROOT / entry
    source = path.read_text(encoding="utf-8")
    digest = sha256_text(source)
    expected = spec.get("expected_sha256")
    if expected and digest != expected:
        raise ShimError(
            f"{entry}: source hash mismatch\n  expected {expected}\n  actual   {digest}\n"
            "Re-derive the patch table for the current source before rerunning."
        )
    return source, apply_patches(source, spec, entry)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    args.out = args.out.expanduser().resolve()

    try:
        for entry, spec in PATCHES.items():
            source, patched = build(entry, spec)
            if args.check:
                print(
                    f"OK  {entry}: {len(spec['patches'])} patch(es) applicable, "
                    f"source {sha256_text(source)}"
                )
                continue
            out_path = args.out / Path(entry).name
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_text(patched, encoding="utf-8")
            print(
                f"{entry} -> {out_path.relative_to(ROOT)}\n"
                f"  patches {len(spec['patches'])}: {spec['notes']}\n"
                f"  source  {sha256_text(source)}\n"
                f"  shimmed {sha256_text(patched)}"
            )
        return 0
    except (OSError, ShimError) as error:
        print(f"agbcc_c89_shim: ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
