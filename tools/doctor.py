#!/usr/bin/env python3
"""Check this checkout's toolchain, ROM, and optional matching helpers.

The setup guide spans several commands and the matching tools fail one at a
time when a piece is missing. This collects the checks in one place and says
what to run next. It is intentionally read-only: it never installs packages,
builds a compiler, or rewrites a report.

Checks are split into three levels:

  PASS  present and usable
  WARN  optional; the named tools still work without it
  FAIL  blocks a required gate

Usage:
    python3 tools/doctor.py
    python3 tools/doctor.py --json build/doctor.json
    python3 tools/doctor.py --strict       # WARN also exits non-zero

The ROM hash is the only check that reads private input. Everything else is a
path or a version probe, so this is safe to run in CI without a ROM (the hash
check reports SKIP there).
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ROM = ROOT / "baserom.gba"
SHA = ROOT / "baserom.sha256"
AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"


def _run(command: list[str], timeout: int = 20) -> tuple[int, str]:
    try:
        result = subprocess.run(command, capture_output=True, text=True,
                                timeout=timeout)
        return result.returncode, (result.stdout + result.stderr).strip()
    except (OSError, subprocess.SubprocessError) as exc:
        return 127, str(exc)


def _which(name: str) -> str | None:
    return shutil.which(name)


def _expect_rom() -> str:
    if not ROM.exists():
        return ""
    for line in SHA.read_text(encoding="utf-8").splitlines():
        fields = line.split()
        if len(fields) == 2 and fields[1].lstrip("*") == "baserom.gba":
            return fields[0]
    return ""


def checks() -> list[dict]:
    rows: list[dict] = []

    def add(name: str, level: str, detail: str, hint: str = "") -> None:
        rows.append({"name": name, "level": level, "detail": detail, "hint": hint})

    version = sys.version_info
    if version >= (3, 11):
        add("python", "PASS", f"{version.major}.{version.minor}.{version.micro}")
    else:
        add("python", "FAIL", f"{version.major}.{version.minor}.{version.micro}",
            "install Python 3.11 or newer")

    for name in ("git", "make", "clang", "shasum"):
        path = _which(name)
        add(name, "PASS" if path else "FAIL", path or "not found",
            f"install {name}" if not path else "")

    arm = ("as", "ld", "objcopy", "objdump", "nm", "readelf")
    for suffix in arm:
        name = f"arm-none-eabi-{suffix}"
        path = _which(name)
        add(name, "PASS" if path else "FAIL", path or "not found",
            "install binutils-arm-none-eabi" if not path else "")

    if importlib.util.find_spec("elftools") is not None:
        add("pyelftools", "PASS", "installed")
    else:
        add("pyelftools", "FAIL", "not importable",
            "python3 -m pip install -r requirements.txt")

    expected = _expect_rom()
    if not expected:
        add("baserom.sha256", "FAIL", "no baserom.gba entry",
            "restore the pinned hash file")
    elif not ROM.exists():
        add("baserom.gba", "SKIP", "not present",
            "supply your own US ROM as baserom.gba")
    else:
        digest = hashlib.sha256(ROM.read_bytes()).hexdigest()
        if digest == expected:
            add("baserom.gba", "PASS", digest[:16] + "…")
        else:
            add("baserom.gba", "FAIL", digest[:16] + "…",
                "the ROM hash does not match baserom.sha256")

    if AGBCC.exists() and os.access(AGBCC, os.X_OK):
        add("agbcc", "PASS", str(AGBCC.relative_to(ROOT)))
    elif (ROOT / "build/toolchains/agbcc").exists():
        add("agbcc", "FAIL", "tree exists but old_agbcc is not executable",
            "run make toolchain")
    else:
        add("agbcc", "FAIL", "not provisioned", "run make toolchain")

    for path, hint in (
        (ROOT / "build-code/code.o", "make code-objects"),
        (ROOT / "build/era-corpus/ready-report.json", "make matching-ready"),
    ):
        if path.exists():
            add(path.name, "PASS", str(path.relative_to(ROOT)))
        else:
            add(path.name, "WARN", "missing", hint)

    npm = _which("npm")
    node = _which("node")
    if npm:
        add("npm", "PASS", npm)
        if node:
            add("node", "PASS", node)
        else:
            add("node", "WARN", "npm is present but node is not",
                "install Node.js to use asmlift seeds")
        sys.path.insert(0, str(ROOT / "tools"))
        try:
            import asmlift_seed

            executable = asmlift_seed.asmlift_path()
        except Exception:  # noqa: BLE001 - doctor must survive a broken tool
            executable = None
        if executable:
            add("asmlift", "PASS", f"{executable} ({asmlift_seed._version(executable)})")
        else:
            add("asmlift", "WARN", "not installed",
                "python3 tools/asmlift_seed.py --install")
    else:
        add("npm", "WARN", "not found",
            "install Node.js to use asmlift seed generation")

    objdiff = _which("objdiff-cli")
    add("objdiff-cli", "PASS" if objdiff else "WARN",
        objdiff or "not found (optional viewer)",
        "" if objdiff else "https://github.com/encounter/objdiff/releases")

    if importlib.util.find_spec("PIL") is not None:
        add("Pillow", "PASS", "installed")
    else:
        add("Pillow", "WARN", "not importable (PNG treemaps only)",
            "python3 -m pip install pillow")
    return rows


def render(rows: list[dict]) -> str:
    width = max((len(row["name"]) for row in rows), default=10)
    lines = []
    for row in rows:
        lines.append(f"{row['level']:4s} {row['name']:<{width}}  {row['detail']}")
        if row["hint"]:
            lines.append(f"     {'':<{width}}  -> {row['hint']}")
    counts = {}
    for row in rows:
        counts[row["level"]] = counts.get(row["level"], 0) + 1
    lines.append("")
    lines.append("  ".join(f"{key}={counts.get(key, 0)}"
                           for key in ("PASS", "WARN", "SKIP", "FAIL")))
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--json", type=Path)
    ap.add_argument("--strict", action="store_true",
                    help="treat WARN as a failure")
    args = ap.parse_args(argv)
    rows = checks()
    print(render(rows))
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps({"checks": rows}, indent=2) + "\n",
                             encoding="utf-8")
        print(f"JSON: {args.json}")
    failed = [row for row in rows if row["level"] == "FAIL"]
    warned = [row for row in rows if row["level"] == "WARN"]
    if failed or (args.strict and warned):
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
