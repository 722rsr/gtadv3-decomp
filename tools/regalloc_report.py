#!/usr/bin/env python3
"""Report agbcc's register allocation for one C function.

Register-choice residuals (same mnemonics, different register numbers) are a
gcc 2.95 local-alloc/global-alloc decision. This tool compiles the function's
TU with agbcc's RTL dumps enabled and reports, per pseudo register:

  * the hard register it finally got (``.greg`` dispositions),
  * its flags (``user var``, ``pointer``, preferred class) from ``.lreg``,
  * its live range as RTL insn uids, derived from the def and the
    ``REG_DEAD`` notes of the dump's insn stream,

and an annotated insn stream showing which pseudos die at each insn and which
are live around it. That answers "why did the result pseudo not take the dying
operand's register" without source-shape guesswork.

Discovery and compilation follow tools/corpus_match_probe.py exactly (same
C89 transform, preprocessor, and agbcc flags), so the allocation reported is
the one the matching probes see.

This is diagnostic evidence only; it grants no promotion authority.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

import corpus_match_probe as probe

ROOT = probe.ROOT
FLAGS = ["-O2", "-mthumb-interwork", "-ffunction-sections", "-da"]


def run(argv, **kwargs):
    return subprocess.run([str(a) for a in argv], check=True, **kwargs)


def function_source_path(name: str) -> tuple[Path, str]:
    """Resolve a body name (VMA alias aware) to its src file and C name."""
    aliases = probe.alias_targets()
    probe_name = next((alias for alias, target in aliases.items()
                       if target == name and probe.parse_vma_name(alias)), name)
    wanted = {name, probe_name}
    vma = probe.parse_vma_name(name)
    funcs = [f for f in probe.src_functions()
             if f[1] in wanted
             or f[1].lstrip("_") in {n.lstrip("_") for n in wanted}
             or (vma is not None and vma == f[2])]
    if not funcs:
        raise ValueError(f"function {name} not found in src/")
    # Same tie-break as corpus_match_probe: the strong body wins over a weak
    # stub, and a longer body wins over a shorter one at the same strength.
    best = max(funcs, key=lambda f: (0 if f[4] else 1, f[3]))
    return best[0], best[1]


def compile_with_dumps(path: Path, work: Path) -> Path:
    """C89-transform + preprocess + agbcc (with RTL dumps) into `work`."""
    transformed, stats = probe.cached_transform_file(path)
    if stats["unhandled"] is not None:
        raise ValueError(f"C89 transform: {stats['unhandled']}")
    unit = work / (path.stem + ".c89.c")
    unit.write_text(transformed, encoding="utf-8")
    pre = work / (path.stem + ".i")
    with pre.open("w", encoding="utf-8") as handle:
        run(["clang", "-E", "-nostdinc", "-undef", *probe.inc_flags(), str(unit)],
            stdout=handle)
    run([probe.AGBCC, *FLAGS, pre.name, "-o", (work / (path.stem + ".s")).name],
        capture_output=True, cwd=work)
    dumps = sorted(work.glob(pre.name + ".*"))
    wanted = [d for d in dumps if d.suffix in (".greg", ".lreg")]
    if not any(d.suffix == ".greg" for d in wanted):
        raise ValueError("agbcc produced no .greg dump (dump support missing?)")
    return pre


def function_section(text: str, name: str) -> str:
    """The `;; Function NAME` region of a dump file."""
    pattern = re.compile(r"^;; Function (\S+)\s*$", re.M)
    marks = [(m.start(), m.group(1)) for m in pattern.finditer(text)]
    for i, (start, fname) in enumerate(marks):
        if fname == name:
            end = marks[i + 1][0] if i + 1 < len(marks) else len(text)
            return text[start:end]
    raise ValueError(f"no `;; Function {name}` region in dump")


def parse_dump(greg: str, lreg: str, name: str):
    """Pseudo metadata, final hard registers, and per-insn death notes."""
    section = function_section(greg, name)
    lsection = function_section(lreg, name)

    pseudos = {}
    for m in re.finditer(
            r"Register (\d+) used (\d+) times across (\d+) insns in block (\d+)"
            r"(?:; set (\d+) times?)?(; user var)?(?:; pref ([A-Z_]+))?(; pointer)?",
            lsection):
        pseudos[int(m.group(1))] = {
            "refs": int(m.group(2)), "user": bool(m.group(6)),
            "pref": m.group(7), "pointer": bool(m.group(8)),
        }
    for m in re.finditer(r"Register (\d+), refs = (\d+), live_length = (\d+), size = (\d+)", section):
        pseudos.setdefault(int(m.group(1)), {}).update(
            refs=int(m.group(2)), live_length=int(m.group(3)))

    dispositions = {}
    m = re.search(r";; Register dispositions:\s*\n(.*?)(?:\n\n|\n;;)", section, re.S)
    if m:
        for n, hard in re.findall(r"(\d+) in (-?\d+)", m.group(1)):
            dispositions[int(n)] = int(hard)

    insns = []  # (uid, first_line, sets, dies, uses) from the PRE-allocation
    # stream (.lreg): pseudos are still numbered, so def/REG_DEAD positions
    # give real live ranges. The .greg stream is post-allocation and would
    # only show hard registers.
    for m in re.finditer(r"\((?:call_|jump_)?insn (\d+) ", lsection):
        uid = int(m.group(1))
        depth, i = 0, m.start()
        while i < len(lsection):
            if lsection[i] == "(":
                depth += 1
            elif lsection[i] == ")":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        block = lsection[m.start():i + 1]
        # The insn pattern ends at the template name ({movsi_insn}); anything
        # after it is REG_DEAD/REG_EQUAL notes, and REG_EQUAL expressions
        # mention pseudos that are not really used by the insn.
        head = re.split(r"\{[A-Za-z_][A-Za-z0-9_]*\}", block, maxsplit=1)[0]
        sets = [int(x) for x in re.findall(
            r"\(set\s+\(reg(?:/[^: ]*)*:\w+ (\d+)\)", head)]
        dies = [int(x) for x in re.findall(
            r"REG_(?:DEAD|UNUSED)\s+\(reg(?:/[^: ]*)*:\w+ (\d+)\)", block)]
        uses = sorted({int(x) for x in re.findall(
            r"\(reg(?:/[^: ]*)*:\w+ (\d+)\)", head)} - set(sets))
        first = block.splitlines()[0].strip()
        insns.append((uid, first, sets, dies, uses))
    return pseudos, dispositions, insns


def report(name: str, pseudos, dispositions, insns) -> str:
    def reg(p):
        hard = dispositions.get(p)
        return f"{p}(r{hard})" if hard is not None else str(p)
    # Live range per pseudo from def/REG_DEAD positions in the insn stream.
    first_def, last_use = {}, {}
    for uid, _first, sets, dies, uses in insns:
        for p in sets:
            first_def.setdefault(p, uid)
            last_use[p] = uid
        for p in dies + uses:
            last_use[p] = uid

    out = [f"## Register allocation: {name}", "",
           "| pseudo | hard reg | user var | refs | live len | live range (uid) |",
           "|---|---|---|---|---|---|"]
    for p in sorted(pseudos):
        meta = pseudos[p]
        hard = dispositions.get(p)
        rng = (f"{first_def.get(p, '?')}..{last_use.get(p, '?')}"
               if p in first_def or p in last_use else "?")
        out.append(f"| {p} | {hard if hard is not None else '-'} | "
                   f"{'yes' if meta.get('user') else ''} | {meta.get('refs', '?')} | "
                   f"{meta.get('live_length', '?')} | {rng} |")

    out += ["", "### Insn stream", "",
            "| uid | sets | dies here | live before | insn |", "|---|---|---|---|---|"]
    live = set()
    for uid, first, sets, dies, uses in sorted(insns):
        before = " ".join(reg(p) for p in sorted(live))
        out.append(f"| {uid} | {' '.join(reg(p) for p in sets) or ''} | "
                   f"{' '.join(reg(p) for p in dies) or ''} | {before} | "
                   f"`{first[:80]}` |")
        live |= set(sets) | set(uses)
        live -= set(dies)
    return "\n".join(out) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--function", required=True)
    ap.add_argument("--work-dir", type=Path,
                    default=ROOT / "build/regalloc")
    args = ap.parse_args()
    if not probe.AGBCC.is_file():
        print("regalloc_report: agbcc missing; run make toolchain", file=sys.stderr)
        return 1
    try:
        path, cname = function_source_path(args.function)
        work = args.work_dir / args.function
        work.mkdir(parents=True, exist_ok=True)
        pre = compile_with_dumps(path, work)
        greg = (work / (pre.name + ".greg")).read_text(encoding="utf-8")
        lreg = (work / (pre.name + ".lreg")).read_text(encoding="utf-8")
        pseudos, dispositions, insns = parse_dump(greg, lreg, cname)
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(f"regalloc_report: {exc}", file=sys.stderr)
        return 1
    print(f"source: {path.relative_to(ROOT)}  (dumps: {work})")
    print(report(cname, pseudos, dispositions, insns))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
