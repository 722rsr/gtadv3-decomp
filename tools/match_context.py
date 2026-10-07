#!/usr/bin/env python3
"""Build a bounded, local context packet for one byte-matching C function.

The packet summarizes the function's source and ROM evidence. Its resolved bytes
come from corpus_match_probe, not from a model or an object similarity score.
Everything is written under ignored build/; no ROM bytes enter tracked files.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

import corpus_match_probe as probe
from agbcc_c89_transform import matching_delimiter, strip_noncode

ROOT = Path(__file__).resolve().parents[1]


def display_path(path: Path) -> str:
    """Repo-relative when under ROOT, else absolute.

    `--out` is the documented way to give a parallel user private output,
    and a user's private dir lives outside the repo (/tmp/...). An
    unconditional relative_to(ROOT) therefore raised ValueError for exactly
    the callers the workflow tells to use it, after all the probing work had
    already been done.
    """
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def function_source(path: Path, name: str, max_lines: int) -> str:
    content = path.read_text(encoding="utf-8", errors="replace")
    pattern = re.compile(r"\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*\{")
    for match in pattern.finditer(strip_noncode(content)):
        opening = content.find("{", match.start(), match.end())
        end = matching_delimiter(strip_noncode(content), opening, "{", "}")
        if end is None:
            continue
        start = content.rfind("\n", 0, match.start()) + 1
        snippet = content[start:end + 1]
        lines = snippet.splitlines()
        if len(lines) > max_lines:
            return "\n".join(lines[:max_lines]) + f"\n/* TRUNCATED: {len(lines)} source lines total */"
        return snippet
    return f"/* Body for {name} was not found by the source extractor; inspect {path}. */"


def linked_candidate(record: dict) -> bytes:
    source = ROOT / record["source"]
    obj = probe.WORK / f"{source.stem}.o"
    sections = probe.section_map(obj)
    keys = [f".text.{record['name']}"]
    if record.get("alias_of"):
        keys.append(f".text.{record['alias_of']}")
    resolver = probe.SymbolResolver(probe.load_code_symbols(), probe.alias_targets(),
                                    probe.scan_decl_hints())
    for key in keys:
        if key in sections:
            return probe.link_function(sections[key], int(record["vma"], 16),
                                       probe.relocations(obj, key), key, resolver)[0]
    raise RuntimeError(f"candidate section missing from {obj}: {keys}")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("function", help="VMA-shaped source name, e.g. _08006C10")
    ap.add_argument("--out", type=Path)
    ap.add_argument("--max-source-lines", type=int, default=180)
    ap.add_argument("--family-index", type=Path,
                    default=ROOT / "build/matching-assist/families.json")
    ap.add_argument("--no-recipes", action="store_true",
                    help="skip family lookup and fresh example probes")
    ap.add_argument("--recipe-count", type=int, default=3)
    args = ap.parse_args()
    if not 1 <= args.recipe_count <= 5:
        ap.error("--recipe-count must be between 1 and 5")
    out_dir = args.out or ROOT / "build/match" / args.function.lstrip("_")
    out_dir.mkdir(parents=True, exist_ok=True)
    report_path = out_dir / "report.json"
    work_dir = out_dir / "work"
    result = subprocess.run([sys.executable, str(ROOT / "tools/corpus_match_probe.py"),
                             "--function", args.function, "--c89", "--require-all",
                             "--work-dir", str(work_dir), "--json", str(report_path)],
                            cwd=ROOT, capture_output=True, text=True)
    if result.returncode:
        print(result.stdout + result.stderr, file=sys.stderr)
        return result.returncode
    report = json.loads(report_path.read_text(encoding="utf-8"))
    if len(report["results"]) != 1:
        print(f"expected one function, got {len(report['results'])}", file=sys.stderr)
        return 1
    rec = report["results"][0]
    probe.WORK = work_dir
    vma = int(rec["vma"], 16)
    reference = probe.ROM.read_bytes()[vma - probe.ROM_BASE:vma - probe.ROM_BASE + rec["rom_bytes"]]
    candidate = linked_candidate(rec)
    (out_dir / "reference.bin").write_bytes(reference)
    (out_dir / "candidate.bin").write_bytes(candidate)
    source_name = rec.get("alias_of") or rec["name"]
    snippet = function_source(ROOT / rec["source"], source_name, args.max_source_lines)
    first = rec["first_diff"]
    mismatch = "none" if first is None else f"+0x{first:X}"
    lines = [
        f"# Matching packet: {rec['name']} at {rec['vma']}", "",
        f"- Source: `{rec['source']}` (`{source_name}`)",
        f"- ROM span: {rec['rom_bytes']} bytes; candidate: {rec['cand_bytes']} bytes",
        f"- Result: **{rec['status']}**, {rec['matched_bytes']}/{rec['rom_bytes']} matching bytes; first difference {mismatch}",
        "- Compiler: `agbcc -O2 -mthumb-interwork -ffunction-sections`; calls and pools resolved at the ROM VMA",
        f"- Recheck: `python3 tools/corpus_match_probe.py --function {rec['name']} --c89 --require-all --json {display_path(report_path)}`",
        "- Exact gate: add `--require-exact` to the recheck command; a score or matching instructions alone is insufficient.",
        "", "## Calls and pool relocations", "",
    ]
    if rec["call_targets"] or rec["pool_words"]:
        lines += [f"- call +0x{x['offset']:X}: `{x['symbol']}` → `{x['target_vma'] or 'UNRESOLVED'}`"
                  for x in rec["call_targets"]]
        lines += [f"- pool +0x{x['offset']:X}: `{x['symbol']}` → `{x['target_value']}`"
                  for x in rec["pool_words"]]
    else:
        lines.append("- none")
    lines += ["", "## Current C", "", "```c", snippet, "```", "",
              "## ROM disassembly", "", "```asm",
              probe.objdump_text(reference, vma, f"{rec['name']}_context_ref").rstrip(), "```", "",
              "## Candidate disassembly", "", "```asm",
              probe.objdump_text(candidate, vma, f"{rec['name']}_context_cand").rstrip(), "```", "",
              "## Completion checks", "",
              "1. Inspect the ROM instructions, literal pools, ABI, and source call sites before changing C.",
              "2. Recompile and require the full linked function span to match at its real VMA.",
              "3. Preserve one VMA owner and run the reference and independent ownership gates after promotion.",
              "4. Record any remaining whole-ROM or data-tail limitation separately from this function result.", ""]
    if not args.no_recipes:
        from match_families import load_index, find_record, nearest, verified_recipes
        try:
            index = load_index(args.family_index)
            target = find_record(index, rec['name'])
            if (target['vma'], target['source'], target.get('alias_of')) != (rec['vma'], rec['source'], rec.get('alias_of')):
                raise ValueError('current probe disagrees with indexed source ownership')
            siblings = nearest(index, target, 5)
            lines += ["## ROM family suggestions", "",
                      "These are structural similarities. Inspect each target's ROM constants, widths and ABI.", ""]
            lines += [f"- `{r['name']}`: {r['similarity']:.0%} similarity; "
                      f"{'same fingerprint' if r['same_family'] else 'different fingerprint'}"
                      for r in siblings]
            if not target['complete']:
                lines += ["- Family lookup unsupported: " + '; '.join(target['limitations'])]
            lines += [""] + verified_recipes(index, target, out_dir, args.recipe_count)
        except (ValueError, OSError) as exc:
            notice = f"Family recipes unavailable: {exc}. Run python3 tools/match_families.py --build."
            print(notice, file=sys.stderr)
            lines += ["## Recipe lookup", "", notice, ""]
    context_path = out_dir / "context.md"
    context_path.write_text("\n".join(lines), encoding="utf-8")
    print(f"{rec['status']} {rec['matched_bytes']}/{rec['rom_bytes']} B; context: {display_path(context_path)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
