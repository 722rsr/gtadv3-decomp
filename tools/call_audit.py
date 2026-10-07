#!/usr/bin/env python3
"""Static pre-flight: can every PROMOTED body's C call resolve in the slice link?

`export_audit.py` answers the same question for branch operands in the closure
TEXT. It cannot answer it for a promoted body's own C call sites, and that is
where the gap bites: a callee is promoted, its `sub_` twin's assembly label sat
inside the spliced span and is therefore GONE, and nothing in the retained asm
branches to it -- so `export_audit` sees no reference at all while the linker
draws `undefined reference to sub_08004BFC`. `promotion_screen.resolve()` sees
the label in the source text and calls the callee resolvable, because "the
closure defines this name" and "the closure still defines this name after the
splice" are different questions and only the second one is asked at link time.

So this tool computes the post-splice world directly. The deletion rule is
TEXTUAL, not address-based, and that distinction is the whole reason it is a
separate tool:

  * `replace_body` deletes from the start marker's line to the end marker's line,
    EXCLUSIVE of the end. A differently-spelled twin sitting ABOVE the start
    marker therefore SURVIVES. 0x08011cbc's entry starts at `_080011CBC:` while
    `sub_080011CBC:` and its `.type` line sit above it, so `bl sub_080011CBC`
    from retained asm still binds -- an address-based rule would call the label
    deleted, because its address is inside the span, and report a false
    positive. `match_c_slice` relies on exactly this ("a leftover label above
    the replacement").
  * The deleted text is the owner's file, `.include`s expanded IN PLACE. An
    entry owned by `asm/passthrough.inc` therefore deletes the labels of the
    converted file it includes, and an entry owned by that converted file
    deletes only its own lines.

  deleted  = closure labels whose flattened position lies in [start, end)
  provided = every closure label outside every deleted span, plus each entry's
             own body name, `c_name` and `export` list, plus names the closure
             defines with `.set`/`.globl` (absolute aliases, never deleted)

and then reports every call target of every promoted body that is in neither
set.

    python3 tools/call_audit.py            # report
    python3 tools/call_audit.py --self-test

Known limits, in the spirit of `export_audit.py`: the linker remains the
authority. This reads the source manifest and the compiled report, so it cannot
see a spelling agbcc invents, and it treats an `export` name as provided
without checking that agbcc actually wrote a `.thumb_set` for it -- the splicer
refuses such a name loudly rather than inventing a binding, so the failure mode
is still a link error and never a silent retarget.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "tools/matching_slice_functions.json"
REPORT = ROOT / "build/era-corpus/ready-report.json"
OBJ = ROOT / "build-code" / "code.o"
ROM_BASE = 0x08000000

# Kept in step with `promotion_screen._bare` and `match_c_slice.replace_body`:
# a label line, with an optional trailing `@` comment. A DATA label
# (`_080003B0: .4byte 0x087B04C4`) must keep failing, or a constant-pool word
# would be treated as a spliced function entry.
LABEL = re.compile(r"^([A-Za-z_][A-Za-z_0-9]*):\s*(?:@.*)?$")
INCLUDE = re.compile(r'^\s*\.include\s+"([^"]+)"')
# A name the closure defines absolutely: `.set _call_via_r2, _0802DDD0` in
# asm/sound_veneer.s. agbcc emits that spelling for a call through r2, and it
# is defined by the alias, not by a label of its own.
ABS_DEF = re.compile(r"^\s*\.(?:set|globl)\s+([A-Za-z_][A-Za-z_0-9]*)")


def _bare(line: str) -> str:
    """`line` with any trailing `@` comment removed, for marker comparison."""
    s = line.strip()
    at = s.find("@")
    return s[:at].rstrip() if at != -1 else s


def flatten(start: Path) -> tuple[list[tuple[Path, int]], set[Path], dict]:
    """The closure's lines with `.include` expanded in place, plus its files.

    Returns `(order, files, pos)` where `order` is a list of `(owning file, local
    line index)` in assembly order and `pos[(file, local)]` is that line's
    position in the flattened stream. `pos` is not derivable from a file base
    offset: a file with an `.include` in it has local indices that stop matching
    stream positions from the include onward, and `asm/passthrough.inc` includes
    ~200 files. Getting that wrong truncates every span owned by such a file to
    the length of its own pre-include prefix, which silently UNDER-reports
    deletions -- the direction that loses findings.
    """
    order: list[tuple[Path, int]] = []
    pos: dict[tuple[Path, int], int] = {}
    files: set[Path] = set()

    def walk(path: Path) -> None:
        if not path.exists() or path in files:
            return
        files.add(path)
        for i, line in enumerate(path.read_text(errors="replace").splitlines()):
            pos[(path, i)] = len(order)
            order.append((path, i, line))
            if m := INCLUDE.match(line):
                walk(path.parent / m.group(1))

    walk(start)
    return order, files, pos


def marker_index(lines: list[str], marker: str) -> int | None:
    """The single line index of `marker`, or None. The splicer's own notion."""
    hits = [i for i, line in enumerate(lines) if _bare(line) == marker]
    return hits[0] if len(hits) == 1 else None


def spans_of(manifest: dict, order: list[tuple], pos: dict) -> list[tuple[int, int]]:
    """`(start, end)` flattened positions for every manifest entry.

    An entry whose markers cannot be located is SKIPPED, not treated as
    deleting everything: `replace_body` would raise on it, so the honest
    answer is "this tool cannot model that entry", and a wrong span here would
    report every label after it as deleted.
    """
    lines_of: dict[Path, list[str]] = {}
    for entry in order:
        path = entry[0]
        if path not in lines_of:
            lines_of[path] = path.read_text(errors="replace").splitlines()
    out: list[tuple[int, int]] = []
    for key, entry in manifest.items():
        owner = ROOT / entry["asm_file"]
        if owner not in lines_of:
            continue
        s = marker_index(lines_of[owner], entry["start_marker"])
        e = marker_index(lines_of[owner], entry["end_marker"])
        if s is None or e is None:
            continue
        if (owner, s) not in pos or (owner, e) not in pos:
            continue
        gs, ge = pos[(owner, s)], pos[(owner, e)]
        if gs >= ge:
            continue
        out.append((gs, ge))
    return out


def deleted_names(order: list[tuple], spans: list[tuple[int, int]], in_span: list[bool] | None = None) -> set[str]:
    """Closure label lines inside a spliced span."""
    lines_of: dict[Path, list[str]] = {}
    for entry in order:
        p = entry[0]
        if p not in lines_of:
            lines_of[p] = p.read_text(errors="replace").splitlines()
    if in_span is None:
        in_span = [False] * len(order)
        for s, e in spans:
            in_span[s:e] = [True] * (e - s)
    out: set[str] = set()
    for pos, entry in enumerate(order):
        if in_span[pos]:
            line = entry[2] if len(entry) > 2 else lines_of[entry[0]][entry[1]]
            m = LABEL.match(line)
            if m:
                out.add(m.group(1))
    return out


def closure_abs_names(files: set[Path]) -> set[str]:
    """Names the closure defines with `.set`/`.globl`, not by a label line."""
    out: set[str] = set()
    for path in files:
        for line in path.read_text(errors="replace").splitlines():
            if m := ABS_DEF.match(line):
                out.add(m.group(1))
    return out


def provided_names(manifest: dict, report: dict, order, spans, files) -> set[str]:
    """What the spliced link still defines once every splice is applied."""
    by_vma = {int(r["vma"], 16): r for r in report["results"]}
    provided: set[str] = set(closure_abs_names(files))
    for key, entry in manifest.items():
        rec = by_vma.get(int(key, 16))
        if rec:
            provided.add(rec["name"])
            if rec.get("alias_of"):
                provided.add(rec["alias_of"])
        if entry.get("c_name"):
            provided.add(entry["c_name"])
        provided.update(entry.get("export", ()))
    in_span = [False] * len(order)
    for s, e in spans:
        in_span[s:e] = [True] * (e - s)
    gone = deleted_names(order, spans, in_span)
    lines_of: dict[Path, list[str]] = {}
    for entry in order:
        p = entry[0]
        if p not in lines_of:
            lines_of[p] = p.read_text(errors="replace").splitlines()
    for pos, entry in enumerate(order):
        if not in_span[pos]:
            line = entry[2] if len(entry) > 2 else lines_of[entry[0]][entry[1]]
            m = LABEL.match(line)
            if m:
                provided.add(m.group(1))
    assert gone is not None
    return provided


def audit(manifest: dict, report: dict) -> tuple[list[tuple], int]:
    """(vma, name, missing symbol) for every promoted body with a dead call."""
    order, files, pos = flatten(ROOT / "asm/code.s")
    spans = spans_of(manifest, order, pos)
    provided = provided_names(manifest, report, order, spans, files)
    by_vma = {int(r["vma"], 16): r for r in report["results"]}
    bad: list[tuple] = []
    for key in sorted(manifest, key=lambda k: int(k, 16)):
        rec = by_vma.get(int(key, 16))
        if not rec:
            continue
        for call in rec.get("call_targets", ()):
            sym = call.get("symbol", "")
            if sym and sym not in provided:
                bad.append((key, rec["name"], sym))
    return bad, len(spans)


def self_test() -> int:
    """Pin the deletion rule on a synthetic closure, never on the live tree.

    The cases that matter, in order of how easy they are to get backwards: a
    label between the markers dies; a label exactly AT the end marker survives;
    a label ABOVE the start marker survives even though its address is inside
    the span; and a label in a file the owner `.include`s dies with it.
    """
    import tempfile
    ok = total = 0

    def check(name, got, want):
        nonlocal ok, total
        total += 1
        if got == want:
            ok += 1
        else:
            print(f"FAIL {name}: got {got!r} want {want!r}", file=sys.stderr)

    with tempfile.TemporaryDirectory() as tmp:
        d = Path(tmp)
        (d / "inc.s").write_text("mid:\n\tbx\tlr\ntail:\n\tbx\tlr\n", encoding="utf-8")
        (d / "code.s").write_text(
            "head:\n\tbx\tlr\nabove:\n"
            "\t.type body, %function\nbody:\n\ttwin:\n\tbx\tlr\n"
            '\t.include "inc.s"\n'
            "\t.type other, %function\nother:\n\tbx\tlr\nlast:\n", encoding="utf-8")
        order, files, pos = flatten(d / "code.s")
        man = {"0x08000000": {"c_name": "c", "asm_file": str(d / "code.s"),
                              "start_marker": ".type body, %function",
                              "end_marker": ".type other, %function",
                              "end_vma": "0x08000010"}}
        spans = spans_of(man, order, pos)
        check("one span modelled", len(spans), 1)
        gone = deleted_names(order, spans)
        check("the body label is deleted", "body" in gone, True)
        check("a twin ABOVE the start marker survives", "twin" in gone, False)
        check("a label above the start marker survives", "above" in gone, False)
        check("an included file's labels are deleted with it",
              gone & {"mid", "tail"}, {"mid", "tail"})
        check("the end marker itself survives", "other" in gone, False)
        # A second, later entry must not make the first one's `twin` disappear.
        # Its markers run the OTHER way round in the flattened stream (the end
        # marker is in the included file, before the start marker in stream
        # order), which is precisely the case `replace_body` refuses -- so the
        # span is skipped rather than inverted, and `other` stays undeleted.
        man2 = dict(man, **{"0x08000010": {"c_name": "d", "asm_file": str(d / "code.s"),
                                           "start_marker": ".type other, %function",
                                           "end_marker": "last:",
                                           "end_vma": "0x08000020"}})
        gone2 = deleted_names(order, spans_of(man2, order, pos))
        check("a forward adjoining entry takes the end marker", "other" in gone2, True)
        check("a forward adjoining entry still spares the twin", "twin" in gone2, False)
        man3 = dict(man, **{"0x08000010": {"c_name": "d", "asm_file": str(d / "code.s"),
                                           "start_marker": "other:",
                                           "end_marker": "last:",
                                           "end_vma": "0x08000020"}})
        gone3 = deleted_names(order, spans_of(man3, order, pos))
        check("a bare-label start marker works too", "other" in gone3, True)
        # An entry whose markers cannot be located is skipped, not guessed at.
        man3 = {"0x08000000": {"c_name": "c", "asm_file": str(d / "code.s"),
                               "start_marker": "no_such_marker:",
                               "end_marker": ".type other, %function",
                               "end_vma": "0x08000010"}}
        check("an unlocatable marker yields no span", len(spans_of(man3, order, pos)), 0)
    print(f"self-test: {ok}/{total} checks PASS")
    return 0 if ok == total else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        return self_test()
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    report = json.loads(REPORT.read_text(encoding="utf-8"))
    bad, nspans = audit(manifest, report)
    print(f"manifest entries: {len(manifest)} / spans modelled: {nspans}")
    if bad:
        print(f"promoted bodies with a call nothing provides: {len(bad)}")
        for key, name, sym in bad:
            print(f"  {key} {name}: {sym}  -> add it to the callee's `export`, "
                  f"or retarget the call to a spelling that survives")
    else:
        print("promoted bodies with a call nothing provides: 0")
    print("\n(the linker remains the authority; see this tool's Known limits)")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
