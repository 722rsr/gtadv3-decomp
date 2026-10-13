#!/usr/bin/env python3
"""Link the matched C/ASM code and registered data into a whole ROM image."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
ROM_SIZE = 0x800000
CODE_OBJECT = Path("build/matching-slice/all/code.o")
DATA_OBJECT = Path("build-code/data.o")
OUT_DIR = Path("build/full-rom-match")
ROM_OUTPUT = OUT_DIR / "gtadv3.gba"
REPORT_OUTPUT = OUT_DIR / "report.json"


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def first_difference(actual: bytes, expected: bytes) -> int | None:
    for offset, (left, right) in enumerate(zip(actual, expected)):
        if left != right:
            return offset
    if len(actual) != len(expected):
        return min(len(actual), len(expected))
    return None


def run(command: list[str], root: Path = ROOT) -> None:
    result = subprocess.run(command, cwd=root, text=True, capture_output=True)
    if result.returncode:
        output = (result.stdout + result.stderr).strip()
        raise RuntimeError(f"{' '.join(command)} failed:\n{output}")


def self_test() -> int:
    cases = [
        ("identical images", b"abcdef", b"abcdef", None),
        ("first differing byte", b"abXdef", b"abcdef", 2),
        ("actual image is short", b"abc", b"abcdef", 3),
        ("actual image is long", b"abcdefg", b"abcdef", 6),
    ]
    failures = []
    for label, actual, expected, wanted in cases:
        got = first_difference(actual, expected)
        if got != wanted:
            failures.append(f"{label}: got {got!r}, expected {wanted!r}")
    if failures:
        print("full_rom_match self-test: FAIL", file=sys.stderr)
        print("\n".join(failures), file=sys.stderr)
        return 1
    print(f"full_rom_match self-test: PASS ({len(cases)} cases)")
    return 0


def pinned_sha256(root: Path) -> str:
    for line in (root / "baserom.sha256").read_text().splitlines():
        fields = line.split()
        if len(fields) == 2 and fields[1] == "baserom.gba":
            return fields[0]
    raise ValueError("baserom.sha256 has no baserom.gba digest")


def link_and_compare(root: Path = ROOT) -> dict:
    code = root / CODE_OBJECT
    data = root / DATA_OBJECT
    reference_path = root / "baserom.gba"
    for path in (code, data, reference_path):
        if not path.is_file():
            raise ValueError(f"required full-ROM match input is missing: {path.relative_to(root)}")

    out_dir = root / OUT_DIR
    out_dir.mkdir(parents=True, exist_ok=True)
    elf = out_dir / "gtadv3.elf"
    map_file = out_dir / "gtadv3.map"
    run([
        "arm-none-eabi-ld", "-T", "ldscript.ld", "-Map", str(map_file),
        str(code.relative_to(root)), str(data.relative_to(root)), "-o", str(elf),
    ], root)
    run([
        "arm-none-eabi-objcopy", "-O", "binary", str(elf.relative_to(root)),
        str(ROM_OUTPUT),
    ], root)

    actual = ROM_OUTPUT.read_bytes()
    expected = reference_path.read_bytes()
    expected_sha = pinned_sha256(root)
    actual_sha = sha256(expected)
    if actual_sha != expected_sha:
        raise ValueError(
            f"reference ROM SHA-256 is {actual_sha}; expected pinned {expected_sha}"
        )
    if len(actual) != ROM_SIZE:
        raise ValueError(f"linked ROM is {len(actual)} bytes; expected {ROM_SIZE}")
    difference = first_difference(actual, expected)
    if difference is not None:
        expected_byte = expected[difference] if difference < len(expected) else None
        actual_byte = actual[difference] if difference < len(actual) else None
        raise ValueError(
            f"full linked ROM differs at offset 0x{difference:06X}: "
            f"got {actual_byte!r}, expected {expected_byte!r}"
        )

    report = {
        "schema": "gtadv3.full_rom_match.v1",
        "result": "pass",
        "rom_bytes": len(actual),
        "rom_sha256": sha256(actual),
        "reference_sha256": expected_sha,
        "byte_identical": True,
        "inputs": {
            "code_object": str(CODE_OBJECT),
            "code_object_sha256": sha256(code.read_bytes()),
            "data_object": str(DATA_OBJECT),
            "data_object_sha256": sha256(data.read_bytes()),
            "padding": "source-generated zero fill in asm/data_tail.s",
            "data_note": (
                "Registered data inputs are byte-verified; some payloads remain "
                "verbatim or are extracted from the user-supplied reference ROM."
            ),
        },
    }
    (root / REPORT_OUTPUT).write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
    print(
        f"PASS full {len(actual):,}-byte C/ASM + registered-data ROM link: "
        "byte-identical to the pinned reference"
    )
    print(f"     report: {REPORT_OUTPUT}")
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        return self_test()
    try:
        link_and_compare()
    except (OSError, ValueError, RuntimeError, subprocess.CalledProcessError) as exc:
        print(f"full_rom_match: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
