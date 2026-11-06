#!/usr/bin/env python3
"""Cross-product sweep of C shapes for one ROM body.

`shape_search.py` scores a directory of hand-written candidates. When a body is
stuck on a handful of independent spelling choices -- inline literal vs reused
temp, `&tbl[i]` vs `tbl + i`, a pointer local vs a cast expression -- the work
is not one more spellings, it is *every combination* of them. Writing that
matrix by hand wastes the thing the sweep needs most: rounds.

This tool expands a template over a knob matrix, scores every cell through
`shape_search`, and prints the sorted table plus the instruction map of the
best cells. The measurement is unchanged: `shape_search` compiles with the
pinned `old_agbcc` and compares against the ROM span, so a cell that reads
EXACT here is the shape to land in `src/`.

Template placeholders are `{{name}}`; the knob file is Python defining
`KNOBS = {"name": ["...", "..."]}`. Template text outside the placeholders is
copied verbatim, so the file stays a readable C draft.

    python3 tools/shape_matrix.py build/shape/t.c _080020E04 --knobs build/shape/k.py
    python3 tools/shape_matrix.py build/shape/t.c _080020E04 --knobs k.py --map 2
"""
from __future__ import annotations

import argparse
import itertools
import re
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(Path(__file__).resolve().parent))

import corpus_match_probe as probe  # noqa: E402
import shape_search as ss  # noqa: E402

PLACEHOLDER = re.compile(r"\{\{(\w+)\}\}")


def load_knobs(path: Path) -> dict[str, list[str]]:
    namespace: dict = {}
    exec(compile(path.read_text(encoding="utf-8"), str(path), "exec"), namespace)
    knobs = namespace.get("KNOBS")
    if not isinstance(knobs, dict) or not knobs:
        raise SystemExit(f"{path}: define KNOBS = {{'name': [values]}}")
    for name, values in knobs.items():
        if not values:
            raise SystemExit(f"{path}: knob {name!r} has no values")
    return knobs


def expand(template: str, knobs: dict[str, list[str]]) -> list[tuple[str, str]]:
    """(label, source) for every cell in the matrix."""
    names = [n for n in PLACEHOLDER.findall(template)]
    unknown = sorted(set(names) - set(knobs))
    if unknown:
        raise SystemExit("template uses knobs with no values: " + ", ".join(unknown))
    ordered = [n for n in knobs if n in names]
    cells: list[tuple[str, str]] = []
    combos = list(itertools.product(*(knobs[n] for n in ordered)))
    width = max(2, len(str(len(combos) - 1)))
    for index, combo in enumerate(combos):
        source = template
        for name, value in zip(ordered, combo):
            source = source.replace("{{%s}}" % name, value)
        cells.append((f"c{index:0{width}d}", source))
    return cells


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("template")
    ap.add_argument("target")
    ap.add_argument("--knobs", required=True)
    ap.add_argument("--fn", help="function name inside the candidates, if ambiguous")
    ap.add_argument("--map", type=int, default=0, metavar="N",
                    help="print the instruction map of the best N cells")
    ap.add_argument("--window", type=int, default=3)
    ap.add_argument("--json", help="write every record here")
    ap.add_argument("--keep", help="copy the winning cell here as <name>.c")
    args = ap.parse_args()

    template = Path(args.template).read_text(encoding="utf-8")
    knobs = load_knobs(Path(args.knobs))
    cells = expand(template, knobs)
    try:
        vma, name, span_end = ss.resolve_target(args.target)
    except ss.Unavailable as exc:
        print(f"shape_matrix: {exc}", file=sys.stderr)
        return 2
    rom = probe.ROM.read_bytes()

    records: list[dict] = []
    failures: list[tuple[str, str]] = []
    sources: dict[str, str] = {}
    with tempfile.TemporaryDirectory(dir=ROOT / "build", prefix="matrix-") as td:
        work = Path(td)
        for tag, source in cells:
            path = work / f"{tag}.c"
            path.write_text(source, encoding="utf-8")
            sources[tag] = source
            try:
                records.append(ss.score(path, vma, span_end, rom, work, args.fn))
            except (ValueError, ss.Unavailable) as exc:
                failures.append((tag, str(exc).splitlines()[0]))
        records.sort(key=lambda r: (r["status"] != "EXACT", -r["matched_bytes"],
                                    r["cand_bytes"] != r["rom_bytes"]))
        print(f"{name} at {vma:#010x}: {len(cells)} cells, "
              f"{len(records)} scored, {len(failures)} rejected")
        ss.render(records, failures, sys.stdout)
        for rec in records[:args.map]:
            print()
            print(f"--- {rec['source']}  frame={rec.get('frame')}  "
                  f"matched={rec['matched_bytes']}/{rec['rom_bytes']} "
                  f"({rec['status']})")
            for line in ss.diff_map(rec, rom, vma, limit=40):
                print("   " + line)
        if args.keep and records:
            best = records[0]
            tag = Path(best["candidate"]).stem
            out = Path(args.keep)
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_text(sources[tag], encoding="utf-8")
            print(f"\nkept {tag} -> {out}")
    if args.json:
        Path(args.json).parent.mkdir(parents=True, exist_ok=True)
        import json
        Path(args.json).write_text(json.dumps(records, indent=2) + "\n",
                                   encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
