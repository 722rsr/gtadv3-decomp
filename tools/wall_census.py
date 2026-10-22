#!/usr/bin/env python3
"""Separate the remaining asm-backed backlog into documented walls and real work.

`source_quality.py --list asm` orders the backlog by size, which invites picking
the biggest body first. Most of those big bodies are not "not yet attempted":
they carry an inline note recording a measured compiler wall (an agbcc pattern
that expands differently, a register pin the allocator will not reproduce, a
`tst` the backend lowers to `cmp`). Re-deriving one of those costs a whole
probe cycle and lands nothing.

This census reads the C text that already owns each leftover address and sorts
it by the evidence it carries:

  measured      a dated MEASURED note: someone probed this body
  unreachable   the note names a construct agbcc cannot emit
  regalloc      register pins/barriers present: allocation is the wall
  raw           naked body with no C at all: needs a full lift, not a nudge
  veneer        a 2-4 byte naked stub (`bx r3`, `swi n`): no C spelling exists
  unexplained   no note, no pins: the only rows worth opening first

`raw` is not a wall: a large naked transcription can always be rewritten from
scratch, it is just the most work per body, so it sorts last. Rows are ranked
by class and then by size (largest payoff first within a class), so the first
line of output is the best next target for the class you have budget for.
Nothing here compiles, edits, or promotes: it classifies the tree live (about
a second) and reads the wall notes out of the C files. `--snapshot` uses the
committed `docs/data/source_quality.json` instead, which is faster but can list
bodies that have since been promoted.

    python3 tools/wall_census.py                 # ranked census
    python3 tools/wall_census.py --class unexplained --top 10
    python3 tools/wall_census.py --json build/wall_census.json
"""
from __future__ import annotations

import argparse
import json
import re
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

ROOT = Path(__file__).resolve().parent.parent
SNAPSHOT = ROOT / "docs/data/source_quality.json"

# Ordered: the first class whose marker the evidence matches wins, so a dated
# measurement outranks the general "cannot emit" phrasing it usually contains.
CLASSES = (
    # Notes appear both as `MEASURED` and as `Measured 2026-10-13:`.
    ("measured", re.compile(r"\bmeasured\b", re.I)),
    ("unreachable", re.compile(
        r"unreachable|no spelling|cannot be (?:emitted|reached|expressed|"
        r"reproduced)|impossible|refuses|never emits|single instruction away",
        re.I)),
    ("regalloc", re.compile(
        r"register alloc|\bpins?\b|\bpinned\b|\bpseudo\b|\bpermuted?\b", re.I)),
)
RANK = {"unexplained": 0, "regalloc": 1, "unreachable": 2,
        "measured": 3, "raw": 4, "veneer": 5}
COMMENT_LINE = re.compile(r"^\s*(?:\*|//|/\*)")
COMMENT_MARKS = re.compile(r"^(?:\s|/\*|\*|//)+")
COMMENT_TAIL = re.compile(r"\s*\*/\s*$")


def load_snapshot(snapshot: bool) -> dict:
    """Return {vma: entry} for every function, classified live unless asked."""
    if not snapshot:
        import source_quality
        result = source_quality.classify(ROOT)
        entries = {}
        for vma, item in result["functions"].items():
            if isinstance(item, str):
                continue
            entries[int(vma, 0)] = item
        return entries
    data = json.loads(SNAPSHOT.read_text())
    return {int(k, 16): v for k, v in data["functions"].items()}


def mask_comments(text: str) -> str:
    """Blank every comment with spaces, keeping byte offsets aligned to `text`.

    `source_quality.functions` scans for braces, so it has to run on stripped
    text; a brace inside a comment otherwise swallows the rest of the file
    (`src/sound_seq_follow.c` parses as zero functions that way). Blanking
    instead of deleting keeps the offsets usable against the raw file, which
    is where the wall notes live.
    """
    import source_quality
    chars = list(text)
    n = len(chars)
    pos = 0
    for match in source_quality._COMMENT_TOKEN.finditer(text):
        i = match.start()
        if i < pos:
            continue
        token = match.group()
        if token == "\\\n":
            chars[i] = chars[i + 1] = " "
            pos = i + 2
        elif token == "//":
            end = text.find("\n", i + 2)
            end = n if end < 0 else end
            for k in range(i, end):
                chars[k] = " "
            pos = end
        elif token == "/*":
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            for k in range(i, end):
                if chars[k] != "\n":
                    chars[k] = " "
            pos = end
        else:  # a literal: already copied, skip its body
            j = i + 1
            while j < n and text[j] != token:
                j += 2 if text[j] == "\\" else 1
            pos = j + 1
    return "".join(chars)


def clean(lines: list[str]) -> list[str]:
    out = []
    for line in lines:
        line = COMMENT_TAIL.sub("", COMMENT_MARKS.sub("", line)).strip()
        if len(line) > 3:
            out.append(line)
    return out


def _notes(path: Path) -> dict[str, list[str]]:
    """{function name: wall-note lines} for one C file.

    The note block is the comment run immediately above the signature plus
    every comment inside the body: those two places are where a session
    records why a body is stuck.
    """
    import source_quality
    try:
        text = path.read_text(errors="replace")
    except OSError:
        return {}
    masked = mask_comments(text)
    out: dict[str, list[str]] = {}
    cursor = 0
    for name, prefix, body in source_quality.functions(masked):
        # Locate the definition by its head, not its body: a `{ }` body is a
        # single space and would match anywhere. The `{` is the head's last
        # character plus one, so offsets map straight back onto `text`.
        head = masked.find(prefix, cursor) if prefix else -1
        if head < 0:
            head = masked.find(name + "(", cursor)
            if head < 0:
                continue
        brace = head + len(prefix)
        if brace >= len(masked) or masked[brace] != "{":
            brace = masked.find("{", head)
            if brace < 0:
                continue
        start = brace + 1
        cursor = start
        above: list[str] = []
        for line in reversed(text[:head].splitlines()[-60:]):
            if not line.strip():
                if above:
                    break
                continue
            if not COMMENT_LINE.match(line):
                break
            above.append(line)
        # `prefix` spans from the previous statement boundary, so it already
        # holds the notes sitting between the last declaration and this
        # signature; the backward walk above only adds a block that a `;` cut
        # away. Both places, then the body, in source order.
        head_notes = [ln for ln in text[head:brace].splitlines()
                      if COMMENT_LINE.match(ln)]
        inside = [ln for ln in text[start:start + len(body)].splitlines()
                  if COMMENT_LINE.match(ln)]
        out[name] = clean(above[::-1] + head_notes + inside)
    return out


def classify(entry: dict, notes: list[str]) -> tuple[str, str]:
    """Return (wall class, the note line that decided it)."""
    joined = "\n".join(notes)
    for label, pattern in CLASSES:
        hit = pattern.search(joined)
        if hit:
            line = next((n for n in notes if pattern.search(n)), hit.group(0))
            return label, line
    detail = entry.get("detail") or {}
    if detail.get("pin") or detail.get("barrier"):
        pins = detail.get("pin", 0) + detail.get("barrier", 0)
        return "regalloc", f"{pins} register pin(s)/barrier(s) still in the body"
    if detail.get("naked") and 0 < entry.get("size", 99) <= 4:
        return "veneer", "2-4 byte stub; bx/swi has no C spelling"
    if detail.get("naked"):
        return "raw", "naked asm body; no C to nudge"
    if detail.get("insn"):
        return "raw", "asm dominates the body"
    return "unexplained", ""


def census(snapshot: bool = False) -> list[dict]:
    entries = load_snapshot(snapshot)
    wanted: dict[Path, list[tuple[int, dict]]] = {}
    for vma, entry in entries.items():
        if entry.get("class") not in ("asm", "partial"):
            continue
        wanted.setdefault(ROOT / entry["file"], []).append((vma, entry))

    rows = []
    for path, items in wanted.items():
        notes = _notes(path)
        for vma, entry in items:
            wall, note = classify(entry, notes.get(entry["name"], []))
            rows.append({
                "vma": vma,
                "class": entry["class"],
                "wall": wall,
                "size": entry["size"],
                "file": entry["file"],
                "name": entry["name"],
                "note": note,
            })
    rows.sort(key=lambda r: (RANK[r["wall"]], -r["size"], r["vma"]))
    return rows


def render(rows: list[dict]) -> str:
    out = [f"{'size':>6}  {'srcc':<7}  {'wall':<12}  target"]
    for r in rows:
        out.append(f"{r['size']:>6}  {r['class']:<7}  {r['wall']:<12}  "
                   f"0x{r['vma']:08x} {r['file']}:{r['name']}")
        if r["note"]:
            out.append(f"{'':>6}  {'':<7}  {'':<12}    {r['note'][:110]}")
    counts: dict[str, int] = {}
    for r in rows:
        counts[r["wall"]] = counts.get(r["wall"], 0) + 1
    total = sum(r["size"] for r in rows)
    tally = ", ".join(f"{k} {counts[k]}" for k in sorted(counts, key=RANK.get))
    out.append("")
    out.append(f"{len(rows)} leftover bodies, {total} bytes: {tally}")
    return "\n".join(out)


def self_test() -> None:
    """Pin the marker precedence and the note window on known shapes."""
    sample = (
        "/* file header */\n"
        "/* MEASURED (2026-10-16): agbcc cannot emit this. */\n"
        "void f(void) {\n"
        "    /* pins are load-bearing here */\n"
        "}\n"
        "// The ROM's `tst` is unreachable from agbcc.\n"
        "void g(void) { }\n"
        "void h(void) { }\n"
    )
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / "t.c"
        path.write_text(sample)
        notes = _notes(path)
    # A dated measurement also says "cannot emit"; measured still wins.
    assert classify({}, notes["f"])[0] == "measured"
    assert classify({}, notes["g"])[0] == "unreachable"
    assert classify({}, notes["h"])[0] == "unexplained"
    assert classify({"detail": {"pin": 1}}, [])[0] == "regalloc"
    assert classify({"detail": {"naked": True}, "size": 24}, [])[0] == "raw"
    assert classify({"detail": {"naked": True}, "size": 4}, [])[0] == "veneer"
    # Ranking must float the actionable row even when it is the smaller one.
    rows = [{"wall": "measured", "size": 900}, {"wall": "unexplained", "size": 20}]
    rows.sort(key=lambda r: (RANK[r["wall"]], -r["size"]))
    assert rows[0]["wall"] == "unexplained"
    print("wall_census self-test: PASS")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--class", dest="wall", choices=sorted(RANK),
                        help="restrict to one wall class")
    parser.add_argument("--top", type=int, default=0,
                        help="stop after N rows (0 = all)")
    parser.add_argument("--ascending", action="store_true",
                        help="smallest body first within each wall class")
    parser.add_argument("--snapshot", action="store_true",
                        help="use the committed classification snapshot")
    parser.add_argument("--json", type=Path, help="write the census as JSON")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        self_test()
        return

    rows = census(args.snapshot)
    if args.ascending:
        rows.sort(key=lambda r: (RANK[r["wall"]], r["size"], r["vma"]))
    if args.wall:
        rows = [r for r in rows if r["wall"] == args.wall]
    if args.json:
        args.json.write_text(json.dumps(rows, indent=1) + "\n")
    if args.top:
        rows = rows[:args.top]
    print(render(rows))


if __name__ == "__main__":
    main()
