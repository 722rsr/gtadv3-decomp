#!/usr/bin/env python3
"""Score many candidate C shapes for ONE ROM body, cheapest loop available.

The matching workflow settles a body by editing `src/` and re-running
`corpus_match_probe.py --function`, which re-parses the tree, rebuilds the
whole translation unit and re-links it for every spelling tried. When a body
is stuck on allocation -- the ROM spills six variables where the draft spills
two -- the work is a *sweep*: dozens of shapes differing in one declaration,
pin or block scope, each needing only a verdict. Editing the tree per shape
turns that into minutes of I/O per idea.

This tool keeps the same measurement and drops the tree round trip. Each
candidate is a self-contained C file holding one function; it is compiled
with the pinned `old_agbcc`, linked at the target's real VMA through
`corpus_match_probe.link_function`, and compared against the ROM span with
`corpus_match_probe.compare_function`. Nothing is scored by this tool's own
arithmetic: the numbers are the ones the corpus gate would produce, so a
shape that reads EXACT here is the shape to land in `src/`.

    python3 tools/shape_search.py Course_CollisionTest build/shape/*.c
    python3 tools/shape_search.py 0x08006E70 --dir build/shape --show best
    python3 tools/shape_search.py --self-test

A candidate that fails to compile is reported and the sweep continues: in a
search, a rejected spelling is data, not an abort. Output is sorted
exact-first, then by matched bytes, so the head of the table is the shape to
work from next.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(Path(__file__).resolve().parent))

import corpus_match_probe as probe  # noqa: E402

class Unavailable(Exception):
    """Toolchain or target resolution failed; not a candidate's fault."""


def resolve_target(target: str) -> tuple[int, str, int]:
    """(vma, display name, span end) for a name, `_0x...` spelling or hex VMA."""
    vma = probe.parse_vma_name(target)
    if vma is None and target.lower().startswith("0x"):
        vma = int(target, 16)
    if vma is None:
        raise Unavailable(f"cannot read a VMA out of {target!r}")
    spans = probe.rom_functions()
    if vma not in spans:
        near = sorted(spans, key=lambda v: abs(v - vma))[:3]
        raise Unavailable(f"{vma:#010x} is not a known function entry; nearest: "
                          + ", ".join(f"{v:#010x}" for v in near))
    return vma, target, spans[vma]


def compile_candidate(path: Path, work: Path) -> Path:
    """Preprocess, agbcc-compile and assemble one candidate; return its object."""
    if not probe.AGBCC.exists():
        raise Unavailable(f"missing pinned compiler: {probe.AGBCC} (run make toolchain)")
    for tool in ("clang", "arm-none-eabi-as"):
        if shutil.which(tool) is None:
            raise Unavailable(f"missing {tool} on PATH")
    stem = re.sub(r"[^A-Za-z0-9_]", "_", path.stem)
    tag = hashlib.sha256(path.read_bytes()).hexdigest()[:12]
    i_path = work / f"{stem}.i"
    s_path = work / f"{stem}.s"
    o_path = work / f"{stem}_{tag}.o"
    if o_path.exists():
        return o_path
    options = probe.agbcc_options(path.name)
    # Preprocess first: the TU cache is keyed on the *preprocessed* text, so a
    # header edit has to miss. clang is the cheap half; agbcc is what is cached.
    pre = subprocess.run(
        ["clang", "-E", "-nostdinc", "-undef", *probe.inc_flags(), str(path),
         "-o", str(i_path)], cwd=ROOT, capture_output=True, text=True)
    if pre.returncode != 0:
        raise ValueError(f"clang failed: {(pre.stderr or pre.stdout).strip()[:400]}")
    if probe.tu_cache_lookup(i_path, s_path, o_path, options) and o_path.exists():
        return o_path
    steps = [
        [str(probe.AGBCC), *options, "-mthumb-interwork",
         "-ffunction-sections", str(i_path), "-o", str(s_path)],
        ["arm-none-eabi-as", "-mcpu=arm7tdmi", str(s_path), "-o", str(o_path)],
    ]
    for argv in steps:
        res = subprocess.run([str(a) for a in argv], cwd=ROOT,
                             capture_output=True, text=True)
        if res.returncode != 0:
            raise ValueError(f"{Path(argv[0]).name} failed: "
                             f"{(res.stderr or res.stdout).strip()[:400]}")
    probe.tu_cache_store(i_path, s_path, o_path, options)
    return o_path


def pick_section(obj: Path, wanted: str | None) -> tuple[str, str]:
    """(section name, symbol name) for the candidate's one function."""
    secs = probe.section_map(obj)
    sizes = probe.symbol_sizes(obj)
    names = [s for s in secs if s.startswith(".text.") and s[6:] in sizes]
    if wanted:
        matches = [s for s in names if s[6:] == wanted]
        if not matches:
            raise ValueError(f"no function {wanted!r}; candidates define: "
                             + ", ".join(s[6:] for s in names) or "(none)")
        names = matches
    if len(names) != 1:
        raise ValueError(f"expected one sized function, found "
                         + ", ".join(s[6:] for s in names or ["(none)"]))
    return names[0], names[0][6:]


def frame_size(blob: bytes) -> int | None:
    """Bytes `sub sp, #N` reserves in the prologue, or None if unsized.

    The frame is the first thing a spilled shape changes, and it is a single
    immediate at a fixed place, so it reads as a column instead of a first
    diff that hides every later difference behind it.
    """
    for i in range(0, min(len(blob), 16) - 1, 2):
        halfword = int.from_bytes(blob[i:i + 2], "little")
        if halfword & 0xFF80 == 0xB080:
            return (halfword & 0x7F) * 4
    return None


def score(path: Path, vma: int, span_end: int, rom: bytes, work: Path,
          wanted: str | None = None, keep: Path | None = None) -> dict:
    obj = compile_candidate(path, work)
    section, symbol = pick_section(obj, wanted)
    sizes = probe.symbol_sizes(obj)
    resolver = probe.SymbolResolver(probe.load_code_symbols(),
                                    probe.alias_targets(), probe.scan_decl_hints())
    blob, calls, pools = probe.link_function(
        probe.section_map(obj)[section], vma,
        probe.relocations(obj, section), section, resolver)
    rec = probe.compare_function(rom, vma, f"{path.stem}__{symbol}", blob,
                                 {vma: span_end}, calls, pools,
                                 sizes.get(symbol))
    if rec is None:
        raise ValueError("comparison produced no record")
    rec["candidate"] = str(path)
    rec["source"] = f"{path.name}:{symbol}"
    rec["frame"] = frame_size(blob)
    if keep is not None:
        keep.mkdir(parents=True, exist_ok=True)
        for generated in sorted(work.glob(f"{path.stem}*")):
            shutil.copy2(generated, keep / generated.name)
        shutil.copy2(path, keep / f"{path.stem}.c")
    return rec


def insn_rows(blob: bytes, vma: int, tag: str) -> dict[int, str]:
    """`offset -> mnemonic` for one blob, keyed by absolute VMA."""
    rows: dict[int, str] = {}
    for line in probe.objdump_text(blob, vma, tag).splitlines():
        parts = line.split("\t")
        if len(parts) < 3:
            continue
        head = parts[0].strip()
        if not head.endswith(":"):
            continue
        try:
            off = int(head[:-1], 16)
        except ValueError:
            continue
        operands = parts[3].strip() if len(parts) > 3 else ""
        rows[off] = (parts[2] + " " + operands).strip()
    return rows


def diff_map(rec: dict, rom: bytes, vma: int, limit: int = 24) -> list[str]:
    """Instruction rows where the candidate and the ROM disagree.

    The point of a sweep is *which* slots a shape allocates, so the differences
    are listed at instruction granularity with both mnemonics on one line.
    """
    rom_blob = rom[vma - probe.ROM_BASE:rec["rom_bytes"] + vma - probe.ROM_BASE]
    rom_rows = insn_rows(rom_blob, vma, f"{rec['name']}_rom")
    cand_blob = (probe.WORK / f"{rec['name']}_cand.bin").read_bytes()
    cand_rows = insn_rows(cand_blob, vma, f"{rec['name']}_cand")
    out: list[str] = []
    for off in sorted(set(rom_rows) & set(cand_rows)):
        if rom_rows[off] != cand_rows[off]:
            out.append(f"+{off - vma:#05x}  rom: {rom_rows[off]:<28} "
                       f"c: {cand_rows[off]}")
    total = len(out)
    shown = out[:limit]
    if total > limit:
        shown.append(f"... {total - limit} more differing instructions")
    return shown


def listing(blob: bytes, vma: int, tag: str, at: int, halfwords: int = 4) -> list[str]:
    """Instructions around byte offset `at`, so a diff reads as a shape change."""
    text = probe.objdump_text(blob, vma, tag)
    rows: list[tuple[int, str]] = []
    for line in text.splitlines():
        parts = line.split("\t")
        if len(parts) < 3:
            continue
        head = parts[0].strip()
        if not head.endswith(":"):
            continue
        try:
            off = int(head[:-1], 16)
        except ValueError:
            continue
        operands = parts[3].rstrip() if len(parts) > 3 else ""
        rows.append((off, (parts[2] + " " + operands).strip()))
    rows = [(off, insn) for off, insn in rows if insn]
    if not rows:
        return []
    if at is None:
        keep = rows[:halfwords]
    else:
        index = next((i for i, (off, _) in enumerate(rows) if off >= at), len(rows) - 1)
        keep = rows[max(0, index - halfwords):index + halfwords]
    return [f"+0x{off:03x}  {insn}" for off, insn in keep]


def show(rec: dict, rom: bytes, vma: int, halfwords: int) -> None:
    first = rec.get("first_diff")
    print(f"--- ROM      vma={vma:#010x} len={rec['rom_bytes']}")
    for line in listing(rom[vma - probe.ROM_BASE:rec["rom_bytes"] + vma - probe.ROM_BASE],
                        vma, "shape_show_rom", first, halfwords):
        print("   " + line)
    print(f"--- CANDIDATE {rec['source']} len={rec['cand_bytes']} "
          f"matched={rec['matched_bytes']}/{rec['rom_bytes']}")
    blob = (probe.WORK / f"{rec['name']}_cand.bin").read_bytes()
    for line in listing(blob, vma, f"{rec['name']}_cand", first, halfwords):
        print("   " + line)


def render(records: list[dict], failures: list[tuple[str, str]], out) -> None:
    width = max([len(r["source"]) for r in records] + [8])
    print(f"{'candidate':<{width}}  {'status':<9} {'bytes':>9} {'frame':>5} {'matched':>8} "
          f"{'1st diff':>8}  class", file=out)
    for rec in records:
        diff = rec.get("first_diff")
        print(f"{rec['source']:<{width}}  {rec['status']:<9} "
              f"{rec['cand_bytes']:>4}/{rec['rom_bytes']:<4} "
              f"{str(rec.get('frame')):>5} {rec['matched_bytes']:>8} "
              f"{('+' + hex(diff)) if diff is not None else '-':>8}  "
              f"{rec.get('class') or ''}", file=out)
    for name, error in failures:
        print(f"{name:<{width}}  COMPILE   {error}", file=out)


def self_test() -> int:
    """Pin the tool end to end on a documented 4-byte ROM leaf.

    0x0800A020 is `movs r0,#44; bx lr` -- small enough that an exact and a
    near-miss shape are both one line, so the self-test measures the tool's
    own path (compile, link, compare, sort) rather than a transcription.
    """
    failures = 0

    def check(what: str, ok: bool) -> None:
        nonlocal failures
        if not ok:
            failures += 1
        print(("PASS " if ok else "FAIL ") + what)

    if not probe.AGBCC.exists():
        print(f"SKIP self-test: {probe.AGBCC} missing (run make toolchain)")
        return 0
    vma, _name, span_end = resolve_target("_0800A020")
    check("target resolves to its ROM span", span_end - vma == 4)
    rom = probe.ROM.read_bytes()
    with tempfile.TemporaryDirectory(dir=ROOT / "build", prefix="shape-self-") as td:
        work = Path(td)
        good = work / "good.c"
        good.write_text("int good(void) { return 44; }\n", encoding="utf-8")
        bad = work / "bad.c"
        bad.write_text("int bad(void) { return 15; }\n", encoding="utf-8")
        broken = work / "broken.c"
        broken.write_text("int broken(void) { return 44 }\n", encoding="utf-8")
        rec = score(good, vma, span_end, rom, work)
        check("an exact body scores EXACT over its whole span",
              rec["status"] == "EXACT" and rec["matched_bytes"] == rec["rom_bytes"])
        rec_bad = score(bad, vma, span_end, rom, work)
        check("a wrong constant is a PARTIAL with a first diff at +0",
              rec_bad["status"] == "PARTIAL" and rec_bad["first_diff"] == 0)
        try:
            score(broken, vma, span_end, rom, work)
            raised = False
        except ValueError as exc:
            raised = "failed" in str(exc)
        check("a candidate that does not compile raises a per-candidate error", raised)
    print(f"shape_search self-test: {'PASS' if not failures else 'FAIL'}")
    return 0 if not failures else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("target", nargs="?", help="function name or VMA, e.g. _08006E70")
    ap.add_argument("candidates", nargs="*", help="self-contained candidate .c files")
    ap.add_argument("--dir", help="score every *.c in this directory")
    ap.add_argument("--fn", help="function name inside the candidate, if ambiguous")
    ap.add_argument("--show", help="dump ROM vs candidate instructions for this "
                                   "candidate stem, or 'all'/'best'")
    ap.add_argument("--window", type=int, default=4,
                    help="instructions either side of the first diff (default 4)")
    ap.add_argument("--map", action="store_true",
                    help="list every differing instruction for --show")
    ap.add_argument("--head", type=int, metavar="N",
                    help="print the first N instructions of every candidate")
    ap.add_argument("--keep", help="copy candidate .c/.i/.s/.o into this directory")
    ap.add_argument("--json", help="write the records here")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        return self_test()
    if not args.target:
        ap.error("a target function name or VMA is required")

    paths = [Path(p) for p in args.candidates]
    if args.dir:
        paths += sorted(Path(args.dir).glob("*.c"))
    if not paths:
        ap.error("give candidate .c files or --dir")
    try:
        vma, name, span_end = resolve_target(args.target)
    except Unavailable as exc:
        print(f"shape_search: {exc}", file=sys.stderr)
        return 2
    rom = probe.ROM.read_bytes()
    records: list[dict] = []
    failures: list[tuple[str, str]] = []
    with tempfile.TemporaryDirectory(dir=ROOT / "build", prefix="shape-") as td:
        work = Path(td)
        keep = Path(args.keep) if args.keep else None
        for path in paths:
            try:
                records.append(score(path, vma, span_end, rom, work, args.fn, keep))
            except (ValueError, Unavailable) as exc:
                failures.append((path.stem, str(exc).splitlines()[0]))
    records.sort(key=lambda r: (r["status"] != "EXACT", -r["matched_bytes"],
                                r["cand_bytes"] != r["rom_bytes"]))
    print(f"{name} at {vma:#010x}, {span_end - vma} ROM bytes")
    render(records, failures, sys.stdout)
    if args.head:
        print()
        for rec in records:
            blob = (probe.WORK / f"{rec['name']}_cand.bin").read_bytes()
            rows = sorted(insn_rows(blob, vma, f"{rec['name']}_cand").items())
            print(f"{Path(rec['candidate']).stem}  frame={rec.get('frame')}: "
                  + " | ".join(f"{insn}" for _off, insn in rows[:args.head]))
    if args.json:
        Path(args.json).parent.mkdir(parents=True, exist_ok=True)
        Path(args.json).write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")
    if args.show and records:
        chosen = records[:1] if args.show == "best" else [
            r for r in records if Path(r["candidate"]).stem == args.show]
        if args.show == "all":
            chosen = records
        shown = [r for r in chosen if (probe.WORK / f"{r['name']}_cand.bin").exists()]
        for rec in shown:
            print()
            if args.map:
                print(f"--- {rec['source']}  frame={rec.get('frame')}  "
                      f"matched={rec['matched_bytes']}/{rec['rom_bytes']}")
                for line in diff_map(rec, rom, vma):
                    print("   " + line)
            else:
                show(rec, rom, vma, args.window)
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
