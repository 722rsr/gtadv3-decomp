#!/usr/bin/env python3
"""Audit TODO/KEEP tracker rows against the naked function census in src/.

Checks coverage, stale or duplicate locations, totals, and per-file counts.
`--fix` updates unambiguous line numbers and reports ambiguous rows.

Usage: python3 tools/naked_doc_audit.py [--doc NAKED_TO_C.md] [--json]
       python3 tools/naked_doc_audit.py --fix
       python3 tools/naked_doc_audit.py --self-test
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

ROW_TODO = re.compile(r"^\| `([^`]+)` \| (\d+) \| `([^`]+)` \|")
# Group rows may include a display label between symbol and location.
ROW_KEEP = re.compile(r"^\| `([^`]+)`[^|]*\| `([^`]+):(\d+)(?:-(\d+))?`")
PERFILE = re.compile(r"^- `([^`]+)` — (\d+)$")
# Match the attribute, not prose that happens to say "naked".
NAKED = "__attribute__((naked))"

# symbol -> why a KEEP row may cite lines that carry no `naked` text.
ROW_EXCEPTIONS = {
    "Veneer_2DDC8..2DE00":
        "cites the 15 DEFINE_SOUND_VENEER expansion sites; the attribute lives "
        "on the multi-line #define at src/runtime_state_dispatch.c:901",
}


def parse_doc(text: str) -> dict:
    """TODO rows, KEEP rows and the per-file table, from the document text."""
    todo: list[tuple[str, int, str]] = []
    keep: list[tuple[str, str, int, int]] = []
    perfile: dict[str, int] = {}
    section = None
    for line in text.split("\n"):
        if line.startswith("## TODO"):
            section = "todo"
            continue
        if line.startswith("## KEEP"):
            section = "keep"
            continue
        if line.startswith("## ") or line.startswith("# "):
            section = None
            continue
        if section == "todo":
            m = ROW_TODO.match(line)
            if m:
                todo.append((m.group(1), int(m.group(2)), m.group(3)))
                continue
        elif section == "keep":
            m = ROW_KEEP.match(line)
            if m:
                keep.append((m.group(1), m.group(2), int(m.group(3)),
                             int(m.group(4) or m.group(3))))
                continue
        m = PERFILE.match(line)
        if m:
            perfile[m.group(1)] = int(m.group(2))
    return {"todo": todo, "keep": keep, "perfile": perfile}


def census(src_root: Path) -> dict:
    """Every `naked` line in src/, split into definitions and structural hits."""
    defs: list[tuple[str, int]] = []
    structural: list[tuple[str, int, str]] = []
    for path in sorted(src_root.rglob("*.c")):
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        rel = str(path.relative_to(ROOT))
        for kind, n in classify_lines(text):
            if kind == "def":
                defs.append((rel, n))
            else:
                structural.append((rel, n, kind))
    return {"defs": defs, "structural": structural}


def classify_lines(text: str) -> list[tuple[str, int]]:
    """Line numbers of the attribute, tagged `def` or the structural kind.

    A continued `#define` attribute is structural, not a function body.
    """
    out: list[tuple[str, int]] = []
    prev_continues = False
    for n, line in enumerate(text.split("\n"), 1):
        if NAKED in line:
            stripped = line.strip()
            if prev_continues:
                out.append(("#define continuation", n))
            elif stripped.startswith("#"):
                out.append(("#define", n))
            elif stripped.endswith(";"):
                out.append(("declaration", n))
            else:
                out.append(("def", n))
        prev_continues = line.rstrip().endswith("\\")
    return out


def audit(doc_text: str, cen: dict,
          exceptions: dict | None = None) -> dict:
    """The four rules.  Returns structured findings, never raises."""
    exceptions = ROW_EXCEPTIONS if exceptions is None else exceptions
    parsed = parse_doc(doc_text)
    hit_lines = {("def",) + p for p in cen["defs"]} | \
                {("struct",) + p[:2] for p in cen["structural"]}
    def_set = set(cen["defs"])

    cited: dict[tuple[str, int], list[str]] = {}
    stale_rows: list[tuple[str, int, str]] = []
    for rel, line, sym in parsed["todo"]:
        if (rel, line) in def_set:
            cited.setdefault((rel, line), []).append(f"TODO {sym}")
        else:
            stale_rows.append((rel, line, f"TODO {sym}"))
    for sym, rel, first, last in parsed["keep"]:
        span = range(first, max(first, last) + 1)
        found = [ln for ln in span if (rel, ln) in def_set] or \
                [ln for ln in span if ("struct", rel, ln) in hit_lines]
        if found:
            for ln in found:
                cited.setdefault((rel, ln), []).append(f"KEEP {sym}")
        elif sym not in exceptions:
            stale_rows.append((rel, first, f"KEEP {sym}"))

    uncovered = sorted(p for p in cen["defs"] if p not in cited)
    duplicated = sorted(p for p, rows in cited.items() if len(rows) > 1)
    used_exceptions = {sym for sym, _r, _f, _l in parsed["keep"]
                       if sym in exceptions}

    per_file_rows: dict[str, int] = {}
    for rel, _line, _sym in parsed["todo"]:
        per_file_rows[rel] = per_file_rows.get(rel, 0) + 1

    return {
        "todo_rows": len(parsed["todo"]),
        "keep_rows": len(parsed["keep"]),
        "naked_defs": len(cen["defs"]),
        "naked_raw": len(cen["defs"]) + len(cen["structural"]),
        "structural": cen["structural"],
        "uncovered": uncovered,
        "stale_rows": stale_rows,
        "duplicated": duplicated,
        "unused_exceptions": sorted(set(exceptions) - used_exceptions),
        "perfile_doc": parsed["perfile"],
        "perfile_rows": per_file_rows,
        "perfile_sum": sum(parsed["perfile"].values()),
    }


def violations(res: dict) -> list[str]:
    out = []
    if res["uncovered"]:
        out.append(f"COVERAGE: {len(res['uncovered'])} body(ies) have no row")
    if res["stale_rows"]:
        out.append(f"NO STALE: {len(res['stale_rows'])} row(s) cite no naked line")
    if res["duplicated"]:
        out.append(f"NO STALE: {len(res['duplicated'])} body(ies) carry two rows")
    if res["unused_exceptions"]:
        out.append(f"EXCEPTIONS: {len(res['unused_exceptions'])} entry(ies) no longer needed")
    want = res["todo_rows"] + res["keep_rows"]
    if want != res["naked_raw"]:
        out.append(f"TOTALS: rows({res['todo_rows']}+{res['keep_rows']})={want} "
                   f"!= naked hits {res['naked_raw']}")
    if res["perfile_sum"] != res["todo_rows"]:
        out.append(f"PER-FILE: table sums to {res['perfile_sum']} != "
                   f"{res['todo_rows']} TODO rows")
    for rel, count in sorted(res["perfile_rows"].items()):
        doc_count = res["perfile_doc"].get(rel)
        if doc_count is None:
            out.append(f"PER-FILE: {rel} missing from the table ({count} TODO rows)")
        elif doc_count != count:
            out.append(f"PER-FILE: {rel} table says {doc_count}, rows say {count}")
    for rel, count in sorted(res["perfile_doc"].items()):
        if rel not in res["perfile_rows"]:
            out.append(f"PER-FILE: {rel} in the table but has no TODO row")
    return out


def fix_doc(doc: Path, cen: dict) -> list[str]:
    """Rewrite stale TODO and KEEP row line numbers from the census.

    Re-anchor only when a symbol identifies one body or only one body remains.
    Leave ambiguous rows unchanged and report them.
    """
    lines = doc.read_text(encoding="utf-8").split("\n")
    parsed = parse_doc("\n".join(lines))
    def_set = set(cen["defs"])
    claimed = {(rel, line) for rel, line, _s in parsed["todo"] if (rel, line) in def_set}
    claimed |= {(rel, line) for _s, rel, first, last in parsed["keep"]
                for line in range(first, max(first, last) + 1) if (rel, line) in def_set}

    by_file: dict[str, list[int]] = {}
    for rel, line in cen["defs"]:
        by_file.setdefault(rel, []).append(line)
    texts: dict[str, list[str]] = {}

    changes: list[str] = []
    problems: list[str] = []
    for idx, (rel, line, sym) in enumerate(parsed["todo"]):
        if (rel, line) in def_set:
            continue
        if rel not in texts:
            src = sorted(Path(rel).parent.glob(Path(rel).name))
            body = src[0].read_text(encoding="utf-8", errors="replace").split("\n") \
                if src else []
            texts[rel] = body
        candidates = [n for n in by_file.get(rel, []) if (rel, n) not in claimed]
        named = [n for n in candidates
                 if sym in "\n".join(texts[rel][n - 1:n + 2])]
        pick = named[0] if len(named) == 1 else (candidates[0] if len(candidates) == 1 else None)
        if pick is None:
            problems.append(f"{sym}: cannot place unambiguously "
                            f"({len(candidates)} free body(ies) in {rel})")
            continue
        for i, text in enumerate(lines):
            if text.startswith(f"| `{rel}` | {line} | `{sym}` |"):
                lines[i] = text.replace(f"| {line} |", f"| {pick} |", 1)
                changes.append(f"{sym}: {rel}:{line} -> :{pick}")
                claimed.add((rel, pick))
                break
        else:
            problems.append(f"{sym}: row text not found for line {line}")
    # Structural KEEP locations are valid and should not be re-anchored.
    structural = {(rel, ln) for rel, ln, _kind in cen.get("structural", [])}
    for sym, rel, first, last in parsed["keep"]:
        if sym in ROW_EXCEPTIONS:
            continue
        width = max(first, last) - first
        span = range(first, last + 1) if last != first else (first,)
        if any((rel, ln) in def_set or (rel, ln) in structural for ln in span):
            continue
        if rel not in texts:
            src = sorted(Path(rel).parent.glob(Path(rel).name))
            body = src[0].read_text(encoding="utf-8", errors="replace").split("\n") \
                if src else []
            texts[rel] = body
        free = [n for n in by_file.get(rel, []) if (rel, n) not in claimed]
        named = [n for n in free if sym in "\n".join(texts[rel][n - 1:n + 2])]
        if width == 0:
            pick = named[0] if len(named) == 1 else (free[0] if len(free) == 1 else None)
        else:
            # Preserve the row width when locating a range.
            pick = next((n for n in (named or free)
                         if all((rel, n + k) in def_set
                                for k in range(1, width + 1))), None)
        if pick is None:
            problems.append(f"{sym}: cannot place KEEP row unambiguously "
                            f"({len(free)} free body(ies) in {rel})")
            continue
        old = (f"`{rel}:{first}-{last}`" if width else f"`{rel}:{first}`")
        new = (f"`{rel}:{pick}-{pick + width}`" if width else f"`{rel}:{pick}`")
        for i, text in enumerate(lines):
            if text.startswith(f"| `{sym}`") and old in text:
                lines[i] = text.replace(old, new, 1)
                changes.append(f"{sym}: {rel}:{first} -> :{pick}")
                for k in range(width + 1):
                    claimed.add((rel, pick + k))
                break
        else:
            problems.append(f"{sym}: KEEP row text not found for line {first}")
    if changes:
        doc.write_text("\n".join(lines), encoding="utf-8")
    for problem in problems:
        print(f"  unresolved: {problem}")
    return changes


def report(res: dict) -> None:
    print(f"TODO rows: {res['todo_rows']}   KEEP rows: {res['keep_rows']}   "
          f"naked bodies: {res['naked_defs']}   raw grep hits: {res['naked_raw']}")
    print(f"structural hits not carried by a row ({len(res['structural'])}):")
    for rel, line, kind in res["structural"]:
        print(f"  {kind:20s} {rel}:{line}")
    print(f"per-file table sum: {res['perfile_sum']}")


# Synthetic tracker fixtures exercise each audit rule.
GOOD_DOC = """
Totals: **2 TODO** + **1 listed KEEP**

## TODO — convert to C (2)

| File | Line | Symbol | Sig / note |
| `src/a.c` | 10 | `A_One` | `void A_One(void)` |
| `src/b.c` | 20 | `B_Two` | `void B_Two(void)` |

- `src/a.c` — 1
- `src/b.c` — 1

## KEEP — valid as straight machine code (do not convert)

| Symbol | Location | Why KEEP |
| `B_Three` | `src/b.c:30` | synthetic reason |
| `E_Decl` | `src/e.c:30-31` | declaration + definition pair |
"""

GOOD_CEN = {
    "defs": [("src/a.c", 10), ("src/b.c", 20), ("src/b.c", 30)],
    "structural": [("src/e.c", 30, "declaration")],
}


def _check(name, got, want):
    ok = got == want
    print(f"  {'PASS' if ok else 'FAIL'} {name}: got {got!r} want {want!r}")
    return ok


def self_test() -> int:
    ok = True
    res = audit(GOOD_DOC, GOOD_CEN, exceptions={})
    ok &= _check("a consistent tracker has no violations", violations(res), [])
    ok &= _check("its rows are counted", (res["todo_rows"], res["keep_rows"]), (2, 2))
    ok &= _check("its totals reconcile",
                 res["todo_rows"] + res["keep_rows"], res["naked_raw"])

    res = audit(GOOD_DOC.replace(
        "| `src/b.c` | 20 | `B_Two` | `void B_Two(void)` |\n", ""), GOOD_CEN,
        exceptions={})
    ok &= _check("a body with no row is caught",
                 any(v.startswith("COVERAGE") for v in violations(res)), True)

    res = audit(GOOD_DOC.replace("| `src/b.c` | 20 |", "| `src/b.c` | 18 |"),
                GOOD_CEN, exceptions={})
    ok &= _check("a TODO row pointing at a non-naked line is caught",
                 any(v.startswith("NO STALE") for v in violations(res)), True)

    res = audit(GOOD_DOC.replace(
        "| `src/b.c` | 20 | `B_Two` | `void B_Two(void)` |",
        "| `src/b.c` | 20 | `B_Two` | `void B_Two(void)` |\n"
        "| `src/b.c` | 20 | `B_TwoAlias` | `void B_Two(void)` |"), GOOD_CEN,
        exceptions={})
    ok &= _check("two rows on one body are caught",
                 any("two rows" in v for v in violations(res)), True)

    grown = dict(GOOD_CEN)
    grown["defs"] = GOOD_CEN["defs"] + [("src/d.c", 7)]
    res = audit(GOOD_DOC, grown, exceptions={})
    ok &= _check("an unlisted body fails the totals",
                 any(v.startswith("TOTALS") or v.startswith("COVERAGE")
                     for v in violations(res)), True)

    res = audit(GOOD_DOC.replace("- `src/b.c` — 1", "- `src/b.c` — 2"), GOOD_CEN,
                exceptions={})
    ok &= _check("a per-file count that disagrees with the rows is caught",
                 any(v.startswith("PER-FILE") for v in violations(res)), True)
    res = audit(GOOD_DOC.replace("- `src/b.c` — 1\n", ""), GOOD_CEN, exceptions={})
    ok &= _check("a file missing from the table is caught",
                 any("missing from the table" in v for v in violations(res)), True)
    res = audit(GOOD_DOC + "- `src/ghost.c` — 1\n", GOOD_CEN, exceptions={})
    ok &= _check("a table entry with no rows is caught",
                 any("in the table but has no TODO row" in v for v in violations(res)), True)

    # A KEEP location must identify a body or an explicit exception.
    res = audit(GOOD_DOC.replace("`src/e.c:30-31`", "`src/e.c:40-41`"), GOOD_CEN,
                exceptions={})
    ok &= _check("a KEEP row citing no naked line is caught",
                 any(v.startswith("NO STALE") for v in violations(res)), True)
    res = audit(GOOD_DOC.replace("`src/e.c:30-31`", "`src/e.c:40-41`"), GOOD_CEN,
                exceptions={"E_Decl": "fixture"})
    ok &= _check("an exception covers the row it names",
                 any(v.startswith("NO STALE") for v in violations(res)), False)
    res = audit(GOOD_DOC, GOOD_CEN,
                exceptions={"E_Decl": "fixture", "NoSuchRow": "fixture"})
    ok &= _check("an exception naming no row is caught",
                 any(v.startswith("EXCEPTIONS") for v in violations(res)), True)

    # Check continuation-line classification.
    print("  -- fixture: --fix re-anchoring --")
    import tempfile
    SHIFTED_DOC = """## TODO — convert to C (2)

| File | Line | Symbol | Sig / note |
| `src/a.c` | 1 | `A_One` | `void A_One(void)` |
| `src/a.c` | 4 | `A_Two` | `void A_Two(void)` |

- `src/a.c` — 2

## KEEP — valid as straight machine code (do not convert)
"""
    # One row is current and the other has a stale line number.
    FIX_CEN = {"defs": [("src/a.c", 1), ("src/a.c", 3)], "structural": []}
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        (root / "src").mkdir()
        (root / "src" / "a.c").write_text(
            "__attribute__((naked)) void A_One(void) {\n}\n"
            "__attribute__((naked)) void A_Two(void) {\n}\n")
        doc = root / "doc.md"
        doc.write_text(SHIFTED_DOC)
        before = violations(audit(doc.read_text(), FIX_CEN, exceptions={}))
        ok &= _check("the shifted row is stale before the fix",
                     any(v.startswith("NO STALE") for v in before), True)
        changed = fix_doc(doc, FIX_CEN)
        ok &= _check("an unambiguous stale row is re-anchored",
                     changed, ["A_Two: src/a.c:4 -> :3"])
        ok &= _check("the rewritten row passes the audit",
                     violations(audit(doc.read_text(), FIX_CEN, exceptions={})), [])
        # Ambiguous rows must remain unresolved.
        doc.write_text(SHIFTED_DOC.replace("| `src/a.c` | 1 |", "| `src/a.c` | 9 |")
                                 .replace("| `src/a.c` | 4 |", "| `src/a.c` | 8 |"))
        changed = fix_doc(doc, FIX_CEN)
        ok &= _check("two stale rows over two bodies are NOT guessed", changed, [])

    print("  -- fixture: --fix re-anchoring KEEP rows --")
    KEEP_DOC = """## TODO — convert to C (1)

| File | Line | Symbol | Sig / note |
| `src/a.c` | 1 | `A_One` | `void A_One(void)` |

- `src/a.c` — 1

## KEEP — valid as straight machine code (do not convert)

| Symbol | Location | Why KEEP |
| `A_Two` | `src/a.c:4` | fixture row |
"""
    KEEP_CEN = {"defs": [("src/a.c", 1), ("src/a.c", 3)], "structural": []}
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        (root / "src").mkdir()
        (root / "src" / "a.c").write_text(
            "__attribute__((naked)) void A_One(void) {\n}\n"
            "__attribute__((naked)) void A_Two(void) {\n}\n")
        doc = root / "doc.md"
        doc.write_text(KEEP_DOC)
        before = violations(audit(doc.read_text(), KEEP_CEN, exceptions={}))
        ok &= _check("a shifted KEEP row is stale before the fix",
                     any(v.startswith("NO STALE") for v in before), True)
        changed = fix_doc(doc, KEEP_CEN)
        ok &= _check("an unambiguous stale KEEP row is re-anchored",
                     changed, ["A_Two: src/a.c:4 -> :3"])
        ok &= _check("the rewritten KEEP row passes the audit",
                     violations(audit(doc.read_text(), KEEP_CEN, exceptions={})), [])
        # Ambiguous KEEP rows must remain unresolved.
        (root / "src" / "a.c").write_text(
            "__attribute__((naked)) void A_One(void) {\n}\n"
            "__attribute__((naked)) void Bep(void) {\n}\n"
            "__attribute__((naked)) void Cep(void) {\n}\n")
        doc.write_text(KEEP_DOC + "| `A_Three` | `src/a.c:9` | fixture row |\n")
        changed = fix_doc(doc, {"defs": [("src/a.c", 1), ("src/a.c", 3),
                                         ("src/a.c", 5)], "structural": []})
        ok &= _check("unnamed stale KEEP rows are NOT guessed", changed, [])

    print("  -- fixture: line classification --")
    ok &= _check("a continuation line is structural",
                 classify_lines("#define M(x) \\\n"
                                "    __attribute__((naked)) void x(void) { \\\n"
                                "    }\n"),
                 [("#define continuation", 2)])
    ok &= _check("a one-line declaration is structural",
                 classify_lines("void f(void) __attribute__((naked));\n"
                                "void f(void) {\n"),
                 [("declaration", 1)])
    ok &= _check("a definition line is a body",
                 classify_lines("__attribute__((naked)) void f(void) {\n}\n"),
                 [("def", 1)])
    ok &= _check("the bug this replaces: a sticky flag lost the chain",
                 classify_lines("#define M \\\n"
                                "  __attribute__((naked)) void x(void) {\n"),
                 [("#define continuation", 2)])

    print(f"naked_doc_audit self-test: {'PASS' if ok else 'FAIL'}")
    return 0 if ok else 1



def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--doc", default=str(ROOT / "NAKED_TO_C.md"))
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--fix", action="store_true",
                    help="rewrite stale TODO row line numbers from the census")
    args = ap.parse_args()
    if args.self_test:
        return self_test()

    doc = Path(args.doc)
    if not doc.exists():
        print("naked_doc_audit: SKIP (no tracking doc present)")
        return 0
    cen = census(ROOT / "src")
    if args.fix:
        changed = fix_doc(doc, cen)
        for line in changed:
            print(f"  re-anchored {line}")
        if not changed:
            print("naked_doc_audit --fix: nothing to re-anchor")
        return 0
    res = audit(doc.read_text(encoding="utf-8", errors="replace"), cen)
    if args.json:
        print(json.dumps(res, indent=2))
        return 0
    report(res)
    bad = violations(res)
    if bad:
        print("\nnaked_doc_audit: FAIL")
        for v in bad:
            print(f"  {v}")
        for rel, line in res["uncovered"]:
            print(f"    uncovered body: {rel}:{line}")
        for rel, line, label in res["stale_rows"]:
            print(f"    stale {label}: {rel}:{line}")
        return 1
    print("\nnaked_doc_audit: PASS (coverage, staleness, totals and per-file counts agree)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
