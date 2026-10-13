#!/usr/bin/env python3
"""Automate decompilation progress updates, reports, treemaps, and README documentation.

Performs all steps required to synchronize and verify progress across:
  1. docs/data/code_data_ownership.json (ROM ownership partition)
  2. docs/data/independent_slice.json (independent slice manifest)
  3. objdiff.json (per-function objdiff configuration)
  4. docs/data/report.json (objdiff v2 report consumed by decomp.dev)
  5. docs/data/report-inputs.json (CI integrity snapshot)
  6. README.md (function count, byte totals, and treemap image link)
  7. build/treemap/ (local 3D cushion treemap PNGs and interactive HTML)
  8. make ci-check (full CI gate verification)

Usage:
  python3 tools/update_progress.py [options]
  make update-progress
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import time
import unittest

ROOT = Path(__file__).resolve().parents[1]
README_PATH = ROOT / "README.md"
MANIFEST_PATH = ROOT / "tools/matching_slice_functions.json"
REPORT_PATH = ROOT / "docs/data/report.json"
STAMP_PATH = ROOT / "docs/data/report-inputs.json"
OWNERSHIP_PATH = ROOT / "docs/data/code_data_ownership.json"
SLICE_PATH = ROOT / "docs/data/independent_slice.json"
OBJDIFF_PATH = ROOT / "objdiff.json"

TREEMAP_IMG_URL_BASE = "https://decomp.dev/722rsr/gtadv3-decomp.png"


def run_cmd(cmd: list[str], cwd: Path = ROOT, check: bool = True) -> subprocess.CompletedProcess:
    """Run a subprocess command and display it clearly on failure."""
    res = subprocess.run(cmd, cwd=cwd, text=True, capture_output=True)
    if check and res.returncode != 0:
        err_msg = (res.stderr.strip() or res.stdout.strip())
        print(f"Error running {' '.join(cmd)}:\n{err_msg}", file=sys.stderr)
        res.check_returncode()
    return res


def compute_manifest_stats(manifest_file: Path = MANIFEST_PATH) -> tuple[int, int]:
    """Return (count, total_bytes) of selected C functions in the manifest."""
    manifest = json.loads(manifest_file.read_text())
    count = len(manifest)
    total_bytes = sum(int(entry["end_vma"], 16) - int(start, 16) for start, entry in manifest.items())
    return count, total_bytes


def content_progress(source_code_bytes: int, matched_code_bytes: int,
                     matched_data_bytes: int, content_end_bytes: int,
                     source_code_exact: bool) -> tuple[int, float]:
    """Return reconstructed non-padding content bytes and their percentage.

    `matched_code_bytes` counts C-owned functions only. The independent source
    slice also contains the cartridge header, startup/IRQ assembly, and other
    non-function bytes, so use its full extent only when its byte-identical
    proof is present. Bytes after the ownership report's content end are
    verified zero padding, not missing decompilation.
    """
    code_bytes = source_code_bytes if source_code_exact else matched_code_bytes
    reconstructed = code_bytes + matched_data_bytes
    percent = reconstructed / content_end_bytes * 100 if content_end_bytes else 0.0
    return reconstructed, percent


def format_readme(content: str, manifest_count: int, manifest_bytes: int,
                  code_size: int = 188760, cache_bust: bool = False,
                  timestamp: int | None = None) -> tuple[str, bool]:
    """Update progress metrics and treemap image URL in README markdown text."""
    changed = False

    # 1. Update manifest function count and byte count
    manifest_pattern = re.compile(
        r"(The manifest selects \*\*)([0-9,]+)( C functions / )([0-9,]+)( bytes\*\* within the\s+\*\*)([0-9,]+)(-byte\*\* executable slice\.)"
    )

    def _replace_stats(m: re.Match) -> str:
        new_stats = (f"{m.group(1)}{manifest_count:,}{m.group(3)}"
                     f"{manifest_bytes:,}{m.group(5)}{code_size:,}{m.group(7)}")
        return new_stats

    new_content, count = manifest_pattern.subn(_replace_stats, content)
    if count == 0:
        # Fallback pattern if formatting differs slightly
        fallback_pattern = re.compile(
            r"The manifest selects \*\*[0-9,]+ C functions / [0-9,]+ bytes\*\* within the"
        )
        replacement = f"The manifest selects **{manifest_count:,} C functions / {manifest_bytes:,} bytes** within the"
        new_content, count = fallback_pattern.subn(replacement, content)

    if new_content != content:
        changed = True
        content = new_content

    # 2. Update Treemap Image Link if cache busting requested
    if cache_bust:
        ts = timestamp or int(time.time())
        img_pattern = re.compile(
            r"(\[!\[GT Advance 3 Decompilation Treemap\]\(https:\/\/decomp\.dev\/722rsr\/gtadv3-decomp\.png)(?:\?[^)]+)?(\)\]\(https:\/\/decomp\.dev\/722rsr\/gtadv3-decomp\))"
        )
        new_img_content, img_count = img_pattern.subn(rf"\g<1>?t={ts}\g<2>", content)
        if new_img_content != content:
            changed = True
            content = new_img_content

    return content, changed


def update_ownership_and_slice(root: Path = ROOT, verbose: bool = True) -> None:
    """Regenerate ownership map and independent slice inventory."""
    if verbose:
        print("[1/6] Refreshing ROM code/data ownership and slice manifests...")
    run_cmd([sys.executable, "tools/ownership_map.py", "--write"], cwd=root)
    run_cmd([sys.executable, "tools/independent_slice.py", "--write"], cwd=root)


def update_objdiff_config(root: Path = ROOT, verbose: bool = True) -> None:
    """Regenerate objdiff.json configuration."""
    if verbose:
        print("[2/6] Refreshing objdiff configuration...")
    run_cmd([sys.executable, "tools/objdiff_build.py", "--configure"], cwd=root)


def refresh_decomp_report(root: Path = ROOT, verbose: bool = True) -> None:
    """Run full matching verification and update decomp.dev report snapshot."""
    if verbose:
        print("[3/6] Running verification gate & refreshing decomp.dev report...")
    run_cmd([sys.executable, "tools/decomp_report.py", "--refresh"], cwd=root)


def update_readme_file(root: Path = ROOT, cache_bust: bool = False,
                       verbose: bool = True) -> tuple[int, int, bool]:
    """Read latest manifest stats and update README.md if changed."""
    if verbose:
        print("[4/6] Updating README.md statistics...")
    readme_file = root / "README.md"
    orig_text = readme_file.read_text()
    func_count, func_bytes = compute_manifest_stats(root / "tools/matching_slice_functions.json")

    # Read code denominator from report if available
    code_size = 188760
    if (root / REPORT_PATH).exists():
        try:
            report_data = json.loads((root / REPORT_PATH).read_text())
            code_size = int(report_data.get("measures", {}).get("total_code", code_size))
        except Exception:
            pass

    updated_text, changed = format_readme(
        orig_text, func_count, func_bytes, code_size=code_size, cache_bust=cache_bust
    )
    if changed:
        readme_file.write_text(updated_text)
        if verbose:
            print(f"      README.md updated: {func_count:,} C functions / {func_bytes:,} bytes")
    elif verbose:
        print(f"      README.md is already up to date ({func_count:,} functions / {func_bytes:,} bytes)")
    return func_count, func_bytes, changed


def generate_treemaps(root: Path = ROOT, verbose: bool = True) -> bool:
    """Generate local 3D cushion treemap PNG images and interactive HTML viewers."""
    if verbose:
        print("[5/6] Generating local treemap visualizations...")
    try:
        import numpy  # noqa: F401
        from PIL import Image  # noqa: F401
    except ImportError:
        if verbose:
            print("      Pillow/numpy not available in Python environment; skipping local PNG treemap.")
        return False

    out_dir = root / "build/treemap"
    out_dir.mkdir(parents=True, exist_ok=True)
    run_cmd([
        sys.executable, "tools/treemap.py",
        "--mode", "independent",
        "--out", "build/treemap/treemap_independent.png",
        "--html", "build/treemap/independent.html",
    ], cwd=root)
    run_cmd([
        sys.executable, "tools/treemap.py",
        "--mode", "subsystem",
        "--out", "build/treemap/treemap_subsystem.png",
        "--html", "build/treemap/subsystem.html",
    ], cwd=root)
    if verbose:
        print("      Treemaps generated at build/treemap/treemap_independent.png and subsystem.png")
    return True


def run_ci_verification(root: Path = ROOT, verbose: bool = True) -> None:
    """Verify that ROM-free CI checks and snapshot fingerprints pass."""
    if verbose:
        print("[6/6] Verifying CI checks (make ci-check)...")
    run_cmd(["make", "ci-check"], cwd=root)
    if verbose:
        print("      make ci-check PASS -- all integrity gates and self-tests passed.")


def print_summary(root: Path = ROOT) -> None:
    """Display a concise summary of current decompilation progress."""
    report_file = root / REPORT_PATH
    if not report_file.exists():
        print("Summary unavailable: report.json does not exist.")
        return

    report = json.loads(report_file.read_text())
    measures = report.get("measures", {})

    func_count, func_bytes = compute_manifest_stats(root / MANIFEST_PATH)
    matched_code = int(measures.get("matched_code", func_bytes))
    total_code = int(measures.get("total_code", 188760))
    code_pct = float(measures.get("matched_code_percent", (matched_code / total_code) * 100))

    matched_data = int(measures.get("matched_data", 0))
    total_data = int(measures.get("total_data", 8199848))
    data_pct = float(measures.get("matched_data_percent", (matched_data / total_data) * 100))

    total_matched = matched_code + matched_data
    rom_size = total_code + total_data
    rom_pct = (total_matched / rom_size) * 100 if rom_size else 0.0

    slice_bytes = total_code
    slice_exact = False
    slice_file = root / SLICE_PATH
    if slice_file.is_file():
        try:
            slice_info = json.loads(slice_file.read_text()).get("slice", {})
            slice_bytes = int(slice_info.get("bytes", total_code))
            slice_exact = slice_info.get("matches_baserom") is True
        except (OSError, ValueError, TypeError):
            pass
    content_end = total_code + matched_data
    ownership_file = root / OWNERSHIP_PATH
    if ownership_file.is_file():
        try:
            content_end = int(json.loads(ownership_file.read_text())
                              .get("rom", {}).get("content_end", content_end), 16)
        except (OSError, ValueError, TypeError):
            pass
    content_bytes, content_pct = content_progress(
        slice_bytes, matched_code, matched_data, content_end, slice_exact)

    print("\n" + "=" * 65)
    print("           GT ADVANCE 3 DECOMPILATION PROGRESS")
    print("=" * 65)
    print(f"  Code Progress:")
    print(f"    Manifest Functions: {func_count:,}")
    print(f"    Matched Executable: {matched_code:,} / {total_code:,} bytes ({code_pct:.2f}%)")
    print(f"  Data Progress:")
    print(f"    Verified Data:      {matched_data:,} / {total_data:,} bytes ({data_pct:.2f}%)")
    print(f"  Non-padding Content:")
    print(f"    Source-backed:      {content_bytes:,} / {content_end:,} bytes ({content_pct:.2f}%)")
    print(f"  Overall Binary:")
    print(f"    C-matched + data:   {total_matched:,} / {rom_size:,} bytes ({rom_pct:.2f}%)")
    print("-" * 65)
    print("  Artifacts ready for commit:")
    print("    • docs/data/code_data_ownership.json")
    print("    • docs/data/independent_slice.json")
    print("    • docs/data/report.json          (decomp.dev objdiff v2 report)")
    print("    • docs/data/report-inputs.json   (CI integrity fingerprint)")
    print("    • objdiff.json")
    print("    • README.md")
    print("-" * 65)
    print(f"  Recommended commit command:")
    print(f"    git add docs/data/report.json docs/data/report-inputs.json \\")
    print(f"            docs/data/code_data_ownership.json docs/data/independent_slice.json \\")
    print(f"            objdiff.json README.md")
    print(f'    git commit -m "Progress: {func_count:,} C functions / {matched_code:,} bytes ({code_pct:.2f}%)"')
    print("=" * 65 + "\n")


class SelfTests(unittest.TestCase):
    def test_content_progress_excludes_verified_padding(self):
        content_bytes, percent = content_progress(
            source_code_bytes=188760,
            matched_code_bytes=188136,
            matched_data_bytes=7873388,
            content_end_bytes=8062148,
            source_code_exact=True,
        )
        self.assertEqual(content_bytes, 8062148)
        self.assertEqual(percent, 100.0)

    def test_unverified_slice_falls_back_to_c_matched_bytes(self):
        content_bytes, percent = content_progress(188760, 188136, 7873388,
                                                   8062148, False)
        self.assertEqual(content_bytes, 8061524)
        self.assertLess(percent, 100.0)

    def test_readme_formatter(self):
        sample = """# Header
The manifest selects **1,252 C functions / 64,740 bytes** within the
**188,760-byte** executable slice. Other text.
[![GT Advance 3 Decompilation Treemap](https://decomp.dev/722rsr/gtadv3-decomp.png)](https://decomp.dev/722rsr/gtadv3-decomp)
"""
        updated, changed = format_readme(sample, 1342, 77676, 188760, cache_bust=False)
        self.assertTrue(changed)
        self.assertIn("**1,342 C functions / 77,676 bytes**", updated)
        self.assertIn("https://decomp.dev/722rsr/gtadv3-decomp.png", updated)

        # Idempotence
        updated2, changed2 = format_readme(updated, 1342, 77676, 188760, cache_bust=False)
        self.assertFalse(changed2)
        self.assertEqual(updated, updated2)

    def test_readme_cache_bust(self):
        sample = "[![GT Advance 3 Decompilation Treemap](https://decomp.dev/722rsr/gtadv3-decomp.png)](https://decomp.dev/722rsr/gtadv3-decomp)\n"
        updated, changed = format_readme(sample, 100, 1000, 188760, cache_bust=True, timestamp=1234567890)
        self.assertTrue(changed)
        self.assertIn("https://decomp.dev/722rsr/gtadv3-decomp.png?t=1234567890", updated)

    def test_manifest_stats(self):
        count, total = compute_manifest_stats(ROOT / "tools/matching_slice_functions.json")
        self.assertGreater(count, 1000)
        self.assertGreater(total, 50000)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--self-test", action="store_true", help="Run internal unit tests and exit.")
    parser.add_argument("--check", action="store_true", help="Check whether README and reports are up to date.")
    parser.add_argument("--skip-build", "--fast", action="store_true", help="Skip matching build gate; update files only.")
    parser.add_argument("--no-treemap", action="store_true", help="Skip local cushion treemap rendering.")
    parser.add_argument("--no-ci-check", action="store_true", help="Skip running make ci-check.")
    parser.add_argument("--cache-bust", action="store_true", help="Append cache-busting timestamp to README treemap image.")
    args = parser.parse_args()

    if args.self_test:
        suite = unittest.TestLoader().loadTestsFromTestCase(SelfTests)
        result = unittest.TextTestRunner(verbosity=2).run(suite)
        sys.exit(0 if result.wasSuccessful() else 1)

    if args.check:
        func_count, func_bytes = compute_manifest_stats(ROOT / "tools/matching_slice_functions.json")
        readme_text = README_PATH.read_text()
        expected_str = f"**{func_count:,} C functions / {func_bytes:,} bytes**"
        if expected_str not in readme_text:
            print(f"README.md is stale: expected '{expected_str}'", file=sys.stderr)
            sys.exit(1)
        # Check decomp_report
        res = subprocess.run([sys.executable, "tools/decomp_report.py"], cwd=ROOT)
        sys.exit(res.returncode)

    print("=== Updating GT Advance 3 Decompilation Progress ===")
    t0 = time.time()

    # Step 1: Update code/data ownership & independent slice manifests
    update_ownership_and_slice(ROOT)

    # Step 2: Refresh objdiff configuration
    update_objdiff_config(ROOT)

    # Step 3: Verify build and refresh decomp.dev report (unless --skip-build)
    if not args.skip_build:
        refresh_decomp_report(ROOT)
    else:
        print("[3/6] Skipping full build gate (--skip-build requested).")

    # Step 4: Update README statistics and badges
    update_readme_file(ROOT, cache_bust=args.cache_bust)

    # Step 5: Render local treemaps
    if not args.no_treemap:
        generate_treemaps(ROOT)
    else:
        print("[5/6] Skipping local treemap rendering (--no-treemap requested).")

    # Step 6: Verify CI checks
    if not args.no_ci_check:
        run_ci_verification(ROOT)
    else:
        print("[6/6] Skipping CI verification (--no-ci-check requested).")

    # Print summary
    print_summary(ROOT)
    elapsed = time.time() - t0
    print(f"Progress update complete in {elapsed:.1f}s.")


if __name__ == "__main__":
    main()
