#!/usr/bin/env python3
"""Fast single-body lift loop: compile one C function out of its real TU.

The corpus probe is the acceptance gate, but it preprocesses and compiles a
whole source file per iteration. When a body is being steered register by
register that is the wrong loop: what is needed is *this* function's agbcc
output beside the ROM bytes, in a second.

This tool copies one function (and the file-scope declarations, typedefs and
`__asm__` symbol bindings that precede it) into a scratch TU under
`build/lift/`, compiles it with the same incantation the probe uses, and
disassembles both streams so the rows can be compared directly.

    python3 tools/lift_loop.py src/garage.c _08027234
    python3 tools/lift_loop.py src/garage.c _08027234 --all
    python3 tools/lift_loop.py src/garage.c _08027234 --asm
    python3 tools/lift_loop.py src/garage.c _08027234 --sweep a,b,wide

`--variant FILE...` grades whole-body replacement files inside the same scratch
TU, so composed spelling grids run at scratch-compile speed instead of full-TU
probe speed. The ranking orders by grade, aligned instruction count, length
drift, then file name.

`--sweep VAR[,VAR...]` inserts an `__asm__ volatile("" : "+r"(VAR))`
barrier at every statement boundary of the body, one (variable, boundary)
pair per iteration, and ranks the results. That barrier is the only lever that
promotes a pseudo to a *global* allocno, which is the precondition for r7 and
for a constant-copy spill; the placement decides how far the range stretches
and therefore its `allocno_compare` priority, so the worthwhile placements are
a sweep rather than a guess. The best variant is written to
`build/lift/sweep_best.c` for pasting back, and the winning placements to
`build/lift/sweep_best.barriers` with the statement each one follows.

`--spill VAR[,VAR...]` runs the same grid with an `asm("" : "+m"(VAR))`
barrier instead. agbcc gives a variable its stack home where that asm appears,
so a spill sweep orders the frame: on `_08002A90C` the placements `v/out` then
`extra` then `outp` reproduce the ROM's sp+0/sp+8/sp+16/sp+20 layout, which no
declaration order does. Use it when the frame offsets, not the registers, are
what differs.

`--sweep-rounds N` (default 1) stacks barriers: each round sweeps again on top
of the current winner, so placements that only pay off *together* are
reachable. `--beam K` carries the K best variants of a round into the next
instead of one; a single greedy step often has two equally-scored children and
only one of them can be extended. Bodies that needed two barriers (`_0800AF84`:
`p09/wa` then `p42/t`) are exactly the ones a one-shot sweep reports as "no
placement beat the baseline".

`--all` prints every aligned row instead of the first mismatch window.
`--asm` prints only the candidate listing (no ROM rows).

The ROM window is the ownership manifest's span for the VMA (`--len` overrides,
the candidate length is only the fallback for a body the manifest does not
list). It must never track the candidate: a body that grows otherwise runs the
comparison into the next function's prologue and reports those bytes as aligned.

The scratch TU keeps the whole file's prologue and the requested function, so
the surrounding declarations, includes and `__attribute__((alias))` lines are
the real ones. Nothing under `src/` is written; the scratch tree is ignored
build output. Exit status is 0 when the extracted range is byte-identical.
"""

from __future__ import annotations

import argparse
import difflib
import json
import os
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor, as_completed as as_complete
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import corpus_match_probe as probe  # noqa: E402
from agbcc_c89_transform import strip_noncode, transform_file  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / "build/lift"
ROM = ROOT / "baserom.gba"
MANIFEST = ROOT / "tools/matching_slice_functions.json"

_SPANS: dict[int, int] | None = None


def span_len(vma: int) -> int | None:
    """ROM span length for `vma` from the ownership manifest.

    The span must never be taken from the candidate: a body that grows by a few
    bytes otherwise drags the ROM window along with it, which silently scores
    the *next* function's prologue as if it were this body's tail and makes a
    grown variant read as "aligned" over instructions the ROM does not have
    here. `--len` overrides; otherwise the manifest's own end boundary wins and
    the candidate length is only the last resort (a body the manifest does not
    list).
    """
    global _SPANS
    if _SPANS is None:
        try:
            raw = json.loads(MANIFEST.read_text(encoding="utf-8"))
        except (OSError, ValueError):
            raw = {}
        _SPANS = {}
        for key, ent in raw.items():
            try:
                _SPANS[int(key, 16)] = int(ent["end_vma"], 16) - int(key, 16)
            except (KeyError, TypeError, ValueError):
                continue
    return _SPANS.get(vma)

FUNC_RE = re.compile(
    r"^(?P<ret>[A-Za-z_][\w \*]*?)\s+(?P<name>[A-Za-z_]\w*)\s*\((?P<args>[^;{]*)\)\s*\{",
    re.M,
)


def _brace_spans(text: str) -> list[tuple[int, int]]:
    """Top-level { ... } spans, skipping comments and literals."""
    out: list[tuple[int, int]] = []
    depth = 0
    start = None
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            i = n if j < 0 else j + 1
            continue
        if c in "\"'":
            q = c
            i += 1
            while i < n and text[i] != q:
                if text[i] == "\\":
                    i += 1
                i += 1
            i += 1
            continue
        if c == "{":
            if depth == 0:
                start = i
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0 and start is not None:
                out.append((start, i + 1))
                start = None
        i += 1
    return out


SIG_RE = re.compile(r"([A-Za-z_]\w*)\s*\([^;{}()]*\)\s*$")

ATTR_RE = re.compile(
    r"__attribute__\s*\(\s*\((?:[^()]|\([^()]*\))*\)\s*\)")


def blank_attrs(masked: str) -> str:
    """Space out `__attribute__((...))` so it cannot win the signature match."""
    return ATTR_RE.sub(lambda m: " " * (m.end() - m.start()), masked)


def build_tu(path: Path, name: str, want_span: bool = False, raw: bool = False):
    """The whole file with every function body but `name` blanked out.

    Keeping the file intact means typedefs, macros, `__asm__` symbol bindings
    and the include list are exactly the ones the real TU compiles with, so the
    scratch body sees the same types and constants. Only the other bodies are
    removed, and their declarations stay intact.

    `want_span` returns the kept body's `{ ... }` range as well, which is what
    the barrier sweep needs to know where it may insert.

    `raw` returns the file untransformed, so a caller can splice raw C into it
    and then run the C89 transform over the whole result. A `--variant` file is
    ordinary source, and splicing it into the already-transformed text would
    leave its declarations after a statement.
    """
    if raw:
        text = path.read_text(encoding="utf-8", errors="replace")
    else:
        # C89 needs every declaration ahead of the first statement; the project
        # applies that transform before agbcc, so the scratch TU must too.
        text, _stats = probe.cached_transform_file(path)
    # Comments and literals are blanked (same indices) so neither the brace scan
    # nor the signature lookback can mistake prose for code -- a doc comment
    # directly above a definition otherwise wins the greedy name match.
    masked = blank_attrs(strip_noncode(text))
    pieces = []
    prev = 0
    found = False
    span = None
    # Bodies before the kept one are shortened to `;`, so coordinates in the
    # joined TU drift; `delta` is the drift a body's original span has accrued.
    delta = 0
    for start, end in _brace_spans(masked):
        sig_start = max(0, start - 400)
        m = SIG_RE.search(masked, sig_start, start)
        if not m:
            continue
        if m.group(1) == name:
            found = True
            # The span covers the whole signature line: a `--variant` body
            # file is a whole replacement starting at the return type, and
            # SIG_RE's match begins at the name, after `void `. The sweep's
            # statement points ignore the extra text since a signature holds
            # no `;`/`{` to report a boundary from.
            line_start = masked.rfind("\n", 0, m.start()) + 1
            span = (line_start - delta, end - delta)
            continue
        # Replace the body with a declaration-only `;` so the signature stays.
        pieces.append(text[prev:start])
        pieces.append(";")
        prev = end
        delta += (end - start) - 1
    if not found:
        raise SystemExit(f"{name}: no top-level definition found")
    pieces.append(text[prev:])
    tu = "".join(pieces)
    return (tu, span) if want_span else tu


def compile_tu(tu: str, origin: Path, workdir: Path = WORK, quiet: bool = False) -> Path:
    """agbcc through the same preprocess/assemble pipeline the probe uses.

    `quiet` keeps a sweep's hundreds of parallel compiles from interleaving
    their diagnostics; the failing variant records them instead.
    """
    err = (lambda s: None) if quiet else sys.stderr.write
    workdir.mkdir(parents=True, exist_ok=True)
    src = workdir / "lift.c"
    src.write_text(tu, encoding="utf-8")
    pre = workdir / "lift.i"
    with pre.open("w", encoding="utf-8") as h:
        rc = subprocess.run(["clang", "-E", "-nostdinc", "-undef",
                             *probe.inc_flags(), str(src)],
                            stdout=h, stderr=subprocess.PIPE, text=True)
    if rc.returncode != 0:
        err(rc.stderr)
        raise SystemExit("clang -E failed: " + rc.stderr.strip().splitlines()[0][:120])
    sfile = workdir / "lift.s"
    rc = subprocess.run([str(probe.AGBCC), *probe.agbcc_options(origin),
                         str(pre), "-o", str(sfile)], capture_output=True, text=True)
    if rc.returncode != 0:
        err(rc.stderr)
        bad = [l for l in rc.stderr.splitlines() if "error" in l]
        raise SystemExit("agbcc failed: " + (bad[0][:120] if bad else "?"))
    err(rc.stderr)
    obj = workdir / "lift.o"
    rc = subprocess.run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-o", str(obj),
                         str(sfile)], capture_output=True, text=True)
    if rc.returncode != 0:
        sys.stderr.write(rc.stderr)
        raise SystemExit("as failed")
    return obj


def disasm(blob: bytes, base: int) -> list[str]:
    """Instruction rows of one blob, aligned on the first instruction.

    The scratch file must be unique per call: a sweep runs these in parallel,
    and a shared path lets one thread disassemble another's bytes (which reads
    as plausible scores for the wrong variant).
    """
    with tempfile.NamedTemporaryFile(suffix=".bin") as tmp:
        tmp.write(blob)
        tmp.flush()
        out = subprocess.run(
            ["arm-none-eabi-objdump", "-D", "-b", "binary", "-m", "arm",
             "-M", "force-thumb", f"--adjust-vma={base:#x}", tmp.name],
            capture_output=True, text=True).stdout
    rows = []
    for line in out.splitlines():
        # objdump separates address / hex bytes / mnemonic with TABS and puts a
        # space between a 4-byte instruction's two halfwords, so the byte field
        # must be delimited by the tabs rather than by a hex class -- matching
        # hex words silently dropped every `bl`, which made two different
        # bodies look identical.
        m = re.match(r"^\s*([0-9a-f]+):\t([^\t]*)\t(.*)$", line)
        if m:
            rows.append((int(m.group(1), 16), re.sub(r"\s+", " ", m.group(3).strip())))
    return rows


def norm(ins: str) -> str:
    t = re.sub(r"0x[0-9a-f]+", "X", ins)
    return re.sub(r"\s+", " ", t).strip()


LOWREG = re.compile(r"\br([0-7])\b")


def permuted(blob_a: bytes, blob_b: bytes) -> dict[int, int] | None:
    """Is `blob_b` the same program as `blob_a` under a low-register rename?

    Returns the observed rN->rM map when the two instruction streams are equal
    after substituting low registers, else None. This is the diagnostic that
    separates "the C is wrong" from "the C is right and only agbcc's
    allocation differs" -- the two need opposite next moves.
    """
    if len(blob_a) != len(blob_b):
        return None
    rows_a, rows_b = disasm(blob_a, 0), disasm(blob_b, 0)
    if len(rows_a) != len(rows_b):
        return None
    fwd: dict[int, int] = {}
    rev: dict[int, int] = {}
    for (pa, ia), (pb, ib) in zip(rows_a, rows_b):
        if pa != pb:
            return None
        # `bl` targets are absolute in the ROM blob and unresolved in the
        # scratch TU's object, so mask every 0x literal before comparing; `#imm`
        # operands stay distinct so a real operand difference still fails.
        ta = LOWREG.sub(lambda m: f"R{m.group(1)}", norm(ia))
        tb = LOWREG.sub(lambda m: f"R{m.group(1)}", norm(ib))
        if ta.count("R") != tb.count("R"):
            return None
        ra = [int(x) for x in re.findall(r"\bR([0-7])", ta)]
        rb = [int(x) for x in re.findall(r"\bR([0-7])", tb)]
        for x, y in zip(ra, rb):
            if fwd.setdefault(x, y) != y or rev.setdefault(y, x) != x:
                return None
        if re.sub(r"R[0-7]", "R", ta) != re.sub(r"R[0-7]", "R", tb):
            return None
    return fwd


def mask_calls(blob: bytes) -> bytes:
    """Zero the offset of every `bl`/`blx` so an unresolved call compares equal.

    The ROM blob has a resolved branch displacement; the scratch TU's object
    carries the assembler's placeholder because the callee lives in another
    object. Only the 11-bit offsets differ, so masking them lets the byte
    verdict say "identical apart from call targets" instead of "differs".
    """
    out = bytearray(blob)
    for i in range(0, len(out) - 3, 2):
        w = out[i] | (out[i + 1] << 8)
        if (w & 0xF800) == 0xF000:
            w2 = out[i + 2] | (out[i + 3] << 8)
            if (w2 & 0xF800) == 0xF800:
                # The displacement straddles both halfwords, so blank the pair.
                out[i] = out[i + 1] = out[i + 2] = out[i + 3] = 0
    return bytes(out)


def rom_slice(vma: int, size: int) -> bytes:
    off = vma - 0x08000000
    rom = ROM.read_bytes()
    # A VMA outside the image silently yielded an empty slice, and every
    # downstream score then read "0 aligned" as if the body were wrong. The
    # usual cause is passing a symbol's 9-digit spelling (`0x08002AD14`, as in
    # the `_08002AD14` label) instead of its 8-digit VMA (`0x802AD14`).
    if off < 0 or off + size > len(rom):
        raise SystemExit(
            f"vma {vma:#010x} + {size} B falls outside the {len(rom):#x} B ROM "
            f"(base {0x08000000:#010x}, offset {off:#x}); VMAs here are "
            f"8-digit ({vma & 0x0FFFFFFF | 0x08000000:#010x}), not the "
            f"9-digit symbol spelling")
    return rom[off:off + size]


def section_blob(obj: Path, function: str, workdir: Path = WORK) -> bytes:
    """The machine code of `function`'s own .text section."""
    section = subprocess.run(
        ["arm-none-eabi-objdump", "-h", str(obj)],
        capture_output=True, text=True).stdout
    sect = ".text." + function
    if sect not in section:
        cands = [l.split()[1] for l in section.splitlines()
                 if ".text." in l and "CODE" in l]
        if not cands:
            raise SystemExit("no text section in object")
        sect = cands[0]
    binpath = workdir / "lift.bin"
    rc = subprocess.run(["arm-none-eabi-objcopy", "-O", "binary",
                         "--only-section", sect, str(obj), str(binpath)],
                        capture_output=True, text=True)
    if rc.returncode != 0:
        sys.stderr.write(rc.stderr)
        raise SystemExit("objcopy failed")
    blob = binpath.read_bytes()
    # A function section can end in alignment padding (gas aligns the section,
    # and an appended `.align`/section tail lands after the symbol). objcopy
    # copies that padding, so the blob reads longer than the function and the
    # length check reports a phantom `LENGTH +N` on an otherwise exact body.
    # The symbol's own size is the authoritative extent.
    nm = subprocess.run(["arm-none-eabi-nm", "-S", "--defined-only", str(obj)],
                        capture_output=True, text=True).stdout
    for line in nm.splitlines():
        parts = line.split()
        if len(parts) == 4 and parts[3] == function:
            size = int(parts[1], 16)
            if 0 < size < len(blob):
                blob = blob[:size]
            break
    return blob


def align_score(ref: bytes, cand: bytes) -> int:
    """Instructions that line up with the ROM under difflib."""
    rows_a, rows_b = disasm(ref, 0), disasm(cand, 0)
    sm = difflib.SequenceMatcher(None, [norm(r[1]) for r in rows_a],
                                 [norm(r[1]) for r in rows_b])
    return sum(i2 - i1 for tag, i1, i2, _j1, _j2 in sm.get_opcodes() if tag == "equal")


def evaluate(tu: str, source: Path, function: str, vma: int, size: int,
             workdir: Path) -> dict:
    """Compile one variant and grade it against the ROM. Never raises on C errors."""
    try:
        obj = compile_tu(tu, source, workdir, quiet=True)
        blob = section_blob(obj, function, workdir)
    except SystemExit as exc:
        return {"kind": "error", "msg": str(exc), "size": 0, "aligned": -1,
                "blob": b"", "tu": tu}
    n = size or len(blob)
    ref = rom_slice(vma, n)
    # Every grade below has to be measured against the fixed span, so a body
    # that ran long cannot be "exact-calls" merely by matching its own length.
    sized = len(blob) == n
    if sized and blob == ref:
        kind = "exact"
    elif sized and mask_calls(blob) == mask_calls(ref):
        kind = "exact-calls"
    elif sized and permuted(ref, blob) is not None:
        kind = "permuted"
    else:
        kind = "differs"
    return {"kind": kind, "msg": "", "size": len(blob),
            "aligned": align_score(ref, blob), "blob": blob, "tu": tu}


def statement_points(tu: str, span: tuple[int, int]) -> list[int]:
    """Offsets inside the body just after each statement `;` and each `{`.

    These are the places a barrier can be inserted without splitting an
    expression. Comments and literals are masked so a `;` in prose is skipped.
    """
    body = strip_noncode(tu[span[0]:span[1]])
    pts = []
    for i, c in enumerate(body):
        if c == ";" or (c == "{" and i > 0):
            pts.append(span[0] + i + 1)
    return pts


BARRIER_SIG = '__asm__ volatile(""'


def _barrier_stmt(reg: str) -> str:
    return '\n__asm__ volatile("" : "+r"(%s));' % reg


def _spill_stmt(var: str) -> str:
    """Force `var` into a stack home at this point (the --spill lever)."""
    return '\n__asm__ volatile("" : "+m"(%s));' % var


def _preceding_stmt(tu: str, at: int) -> str:
    """One-line tail of the statement a barrier at `at` follows.

    Printed with the winner so the placement can be reproduced in the real
    source: the sweep runs on the C89-transformed TU, whose line numbers mean
    nothing to the file on disk, but the statement text does.
    """
    text = " ".join(tu[max(0, at - 160):at].split())
    return text[-78:]


def barrier_variants(tu: str, span: tuple[int, int], regs: list[str],
                     tag: str = "") -> list[tuple[str, str, tuple[int, str, str]]]:
    """(label, tu, (point, reg, preceding statement)) per barrier placement.

    A boundary that already carries a barrier is skipped: the barriered text
    adds its own `;`, so without this every stacked round would spend a
    compile re-testing placements that exist already.
    """
    out = []
    seen = set()
    for idx, at in enumerate(statement_points(tu, span)):
        if BARRIER_SIG in tu[max(0, at - 48):at]:
            continue
        for reg in regs:
            cand = tu[:at] + _barrier_stmt(reg) + tu[at:]
            if cand in seen:
                continue
            seen.add(cand)
            out.append((f"{tag}p{idx:02d}/{reg}", cand, (idx, reg, _preceding_stmt(tu, at))))
    return out


def spill_variants(tu: str, span: tuple[int, int], vars_: list[str],
                   tag: str = "") -> list[tuple[str, str, tuple[int, str, str]]]:
    """(label, tu, (point, var, preceding statement)) per spill placement.

    Same grid as `barrier_variants`, but the inserted statement is an `"+m"`
    barrier. agbcc gives a variable a stack home where that asm appears (and
    allocates the other homes around it), which is the lever for frame layout:
    the slot order follows the barrier order, not the declaration order.
    """
    out = []
    seen = set()
    for idx, at in enumerate(statement_points(tu, span)):
        window = tu[max(0, at - 64):at]
        if BARRIER_SIG in window:
            continue
        for var in vars_:
            if '"+m"(%s)' % var in window:
                continue
            cand = tu[:at] + _spill_stmt(var) + tu[at:]
            if cand in seen:
                continue
            seen.add(cand)
            out.append((f"{tag}p{idx:02d}/{var}", cand,
                        (idx, var, _preceding_stmt(tu, at))))
    return out


RANK = {"exact": 0, "exact-calls": 1, "permuted": 2, "differs": 3, "error": 9}


def _score(item, size: int) -> tuple:
    """Grade for comparing a variant with the baseline (no label tiebreak)."""
    _label, res, _meta = item
    return (RANK[res["kind"]], -res["aligned"],
            abs(res["size"] - size) if size else 0)


def _rank(item, size: int) -> tuple:
    """Sort key: the grade, then the shorter barrier chain (stable, and a
    shorter chain is the easier one to paste back into the source)."""
    return _score(item, size) + (len(item[2]), item[0])


def run_sweep(a, tu: str, span, vma: int, size: int,
              gen=barrier_variants) -> int:
    regs = [r.strip() for r in (a.spill or a.sweep).split(",") if r.strip()]
    rounds = max(1, a.sweep_rounds)
    beam = max(1, a.beam)
    points = ({int(p.strip().lstrip("p")) for p in a.points.split(",") if p.strip()}
              if a.points else None)
    base = evaluate(tu, a.source, a.function, vma, size, WORK / "sweep/base")
    base_item = ("baseline", base, [])
    print(f"{'variant':<12} {'kind':<11} {'size':>5} {'aligned':>7}  barriers")
    print(f"{'baseline':<12} {base['kind']:<11} {base['size']:>5} {base['aligned']:>7}")

    best = base_item
    frontier = [("", tu, [])]
    for rnd in range(1, rounds + 1):
        tag = "" if rnd == 1 else f"r{rnd}"
        variants = []
        seen = set()
        for _path, cur, meta in frontier:
            for label, cand, m in gen(cur, span, regs, tag):
                if points is not None and rnd == 1 and m[0] not in points:
                    continue
                if cand in seen:
                    continue
                seen.add(cand)
                variants.append((label, cand, meta + [m]))
        if not variants:
            break
        print(f"\nround {rnd}: {len(variants)} variants x {regs}"
              f" (frontier {len(frontier)})", file=sys.stderr)
        base_dir = WORK / "sweep" / f"r{rnd}"
        results = []
        with ThreadPoolExecutor(max_workers=a.jobs) as pool:
            futs = {pool.submit(evaluate, cand, a.source, a.function, vma, size,
                                base_dir / f"v{i:03d}"): (label, meta)
                    for i, (label, cand, meta) in enumerate(variants)}
            for fut in as_complete(futs):
                label, meta = futs[fut]
                try:
                    res = fut.result()
                except Exception as exc:  # noqa: BLE001 - one bad variant must not end the sweep
                    res = {"kind": "error", "msg": repr(exc), "size": 0,
                           "aligned": -1, "blob": b"", "tu": ""}
                results.append((label, res, meta))
        results.sort(key=lambda item: _rank(item, size))
        head = f"{'variant':<12} {'kind':<11} {'size':>5} {'aligned':>7}  barriers"
        print(head)
        for label, res, meta in results[:a.top]:
            chain = " + ".join(f"p{m[0]:02d}/{m[1]}" for m in meta)
            print(f"{label:<12} {res['kind']:<11} {res['size']:>5} {res['aligned']:>7}"
                  f"  {chain}")
        top = results[0]
        if _score(top, size) < _score(best, size):
            best = top
            if best[1]["kind"] == "exact":
                break
            frontier = [(label, res["tu"], meta) for label, res, meta in results[:beam]]
        else:
            print(f"\nround {rnd}: no variant on the frontier improved", file=sys.stderr)
            break

    label, res, meta = best
    if meta and res["tu"]:
        best_path = WORK / "sweep_best.c"
        best_path.write_text(res["tu"], encoding="utf-8")
        # The variant is already past the C89 transform, so its real TU origin
        # has to travel with it for `--raw-tu` to compile it the same way.
        (WORK / "sweep_best.origin").write_text(str(a.source.resolve()))
        lines = [f"# {a.function}: {res['kind']}, {res['size']} B, "
                 f"{res['aligned']} aligned",
                 "# barrier/statement, in insertion order -> ",
                 "#   build/lift/sweep_best.c (--raw-tu), then paste into the source"]
        for r_i, (idx, reg, stmt) in enumerate(meta, 1):
            lines.append(f"{r_i}. +r({reg})  after p{idx:02d}: ...{stmt}")
        (WORK / "sweep_best.barriers").write_text("\n".join(lines) + "\n",
                                                  encoding="utf-8")
        print(f"\nbest: {label} ({res['kind']}, {res['aligned']} aligned) -> {best_path}")
        for r_i, (idx, reg, stmt) in enumerate(meta, 1):
            print(f"  barrier {r_i}: +r({reg}) after p{idx:02d}: ...{stmt}")
    else:
        print("\nno placement beat the baseline")
    return 0 if res["kind"] == "exact" else 1


def print_listing(ref_bytes: bytes, cand_bytes: bytes, vma: int,
                  all_rows: bool = False, limit: int = 3) -> None:
    """Print the aligned ROM-vs-candidate listing (
    `!!` ROM rows, `++` candidate rows) for the first `limit` differing runs.

    Shared by the single-body path and `--variant --diff`, so a spelling grid
    does not need a second probe run to show what the bytes actually differ in.
    """
    ref = disasm(ref_bytes, vma)
    cand = disasm(cand_bytes, vma)
    sm = difflib.SequenceMatcher(None, [norm(r[1]) for r in ref],
                                 [norm(c[1]) for c in cand])
    shown = 0
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            if all_rows:
                for k in range(i2 - i1):
                    print(f"   {ref[i1 + k][0]:08x}  ref: {ref[i1 + k][1]}")
            continue
        if all_rows:
            for k in range(i1, i2):
                print(f"!! {ref[k][0]:08x}  ref: {ref[k][1]}")
            for k in range(j1, j2):
                print(f"++ {cand[k][0]:08x}  c++: {cand[k][1]}")
        else:
            shown += 1
            print(f"   rom {i1}:{i2}  ->  cand {j1}:{j2}")
            for k in range(i1, i2):
                print(f"!! {ref[k][0]:08x}  ref: {ref[k][1]}")
            for k in range(j1, j2):
                print(f"++ {cand[k][0]:08x}  c++: {cand[k][1]}")
            if limit and shown >= limit:
                break


def byte_score(blob: bytes, ref: bytes) -> tuple[int, int | None]:
    """Byte-match count in the probe's sense, with unresolved windows out.

    The linked scratch object leaves every `bl` displacement as the
    assembler's placeholder, so those byte pairs can never compare equal.
    Masking the BL displacement pairs (mask_calls) before the comparison
    reproduces the probe's decision to exclude unresolved windows, at
    ranking fidelity.
    """
    a, b = mask_calls(blob), mask_calls(ref)
    compared = min(len(a), len(b))
    diffs = [i for i in range(compared) if a[i] != b[i]]
    matched = sum(1 for i in range(compared) if a[i] == b[i])
    return matched, (diffs[0] if diffs else
                     (None if len(a) == len(b) else compared))


def first_diff(res: dict, vma: int, size: int) -> int | None:
    """Byte offset of the first candidate/ROM difference inside the span."""
    ref = rom_slice(vma, size)
    return next((i for i, (x, y) in enumerate(zip(res["blob"], ref)) if x != y),
                None)


def run_variants(a, tu: str, span: tuple[int, int], vma: int, size: int) -> int:
    """Grade whole-body replacement files in the scratch TU.

    Each `--variant FILE` is spliced over the kept body's span and graded like
    lab_sweep, but at the scratch TU's compile cost (a second or two instead of
    a full-TU probe). Ranking counts probe-sense matched bytes (`byte_score`),
    so the fast sweep ranks the way the acceptance gate will. The splice runs on
    the raw source and the whole file is then C89-transformed, so a variant may
    declare locals after statements like any other source file.
    """
    raw_tu, raw_span = build_tu(a.source, a.function, want_span=True, raw=True)
    results = []
    for vfile in a.variant:
        body = Path(vfile).read_text(encoding="utf-8")
        cand = raw_tu[:raw_span[0]] + body + raw_tu[raw_span[1]:]
        workdir = WORK / "variant" / Path(vfile).stem
        workdir.mkdir(parents=True, exist_ok=True)
        scratch = workdir / "variant_raw.c"
        scratch.write_text(cand, encoding="utf-8")
        cand, _stats = transform_file(scratch)
        res = evaluate(cand, a.source, a.function, vma, size, workdir)
        n = size or len(res["blob"])
        res["bytes"], res["first"] = byte_score(res["blob"],
                                                rom_slice(vma, n))
        if res["kind"] == "error":
            res["msg"] = res["msg"].splitlines()[0][:160] if res["msg"] else "?"
        results.append((vfile, res))
    results.sort(key=lambda it: (RANK[it[1]["kind"]], -it[1]["bytes"],
                                 abs(it[1]["size"] - size), it[0]))
    print(f"{'variant':<56} {'kind':<11} {'size':>5} {'bytes':>6}  first")
    for vfile, res in results:
        print(f"{Path(vfile).name:<56} {res['kind']:<11} {res['size']:>5} "
              f"{res['bytes']:>6}  {res['first'] if res['first'] is not None else '-'}"
              + (f"  {res['msg']}" if res["kind"] == "error" else ""))
    if not results:
        return 1
    best = results[0][1]
    if a.diff and best["kind"] != "exact":
        print(f"\n--- {Path(results[0][0]).name} ---")
        print_listing(rom_slice(vma, size), best["blob"], vma,
                      limit=a.limit)
    return 0 if best["kind"] == "exact" else 1


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("source", type=Path)
    ap.add_argument("function")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--asm", action="store_true")
    ap.add_argument("--len", type=lambda s: int(s, 0))
    ap.add_argument("--vma", type=lambda s: int(s, 0))
    ap.add_argument("--variant", nargs="+", metavar="FILE",
                    help="grade whole-body replacement files from a scratch TU")
    ap.add_argument("--sweep", metavar="VAR[,VAR...]",
                    help="insert a +r barrier on each variable at every statement boundary and rank")
    ap.add_argument("--spill", metavar="VAR[,VAR...]",
                    help="insert a \"+m\" barrier on each variable at every statement boundary "
                         "and rank (forces a stack home at that point: the frame-layout lever)")
    ap.add_argument("--points", metavar="P[,P...]",
                    help="restrict the sweep to these statement indices")
    ap.add_argument("--sweep-rounds", type=int, default=1,
                    help="stack this many barriers (each round sweeps on the winner)")
    ap.add_argument("--beam", type=int, default=1,
                    help="frontier width carried into the next sweep round")
    ap.add_argument("--jobs", type=int, default=0, help="parallel sweeps (0 = cpu count)")
    ap.add_argument("--top", type=int, default=10)
    ap.add_argument("--diff", action="store_true",
                    help="print the aligned listing for the best --variant body")
    ap.add_argument("--limit", type=int, default=3,
                    help="differing runs printed by --diff (0 = every run)")
    ap.add_argument("--raw-tu", action="store_true",
                    help="`source` is already a transformed TU (e.g. sweep_best.c)")
    a = ap.parse_args()
    if not a.jobs:
        a.jobs = max(1, min(8, (os.cpu_count() or 2)))

    vma = a.vma or 0
    if not vma:
        # The VMA name is the alias target's `_08027234` spelling, either as
        # the requested symbol itself or as an alias of it.
        src = a.source.read_text(encoding="utf-8", errors="replace")
        alias = re.search(
            r"\b_?(?:080)([0-9A-Fa-f]{5})\b[^;]*alias\(\"%s\"\)" % re.escape(a.function), src)
        if alias:
            vma = 0x08000000 + int(alias.group(1), 16)
        else:
            m = re.fullmatch(r"_?080([0-9A-Fa-f]{5,6})", a.function)
            if m:
                # The trailing hex digits are the offset inside ROM: the
                # base-prefixed spelling `_08002A90C` carries one extra zero
                # after the `080*` prefix, so never int() the whole name.
                vma = 0x08000000 + int(m.group(1), 16)
    if not vma:
        raise SystemExit("pass --vma (or use an _0802xxxx symbol name)")

    if a.variant:
        tu, span = build_tu(a.source, a.function, want_span=True)
        return run_variants(a, tu, span, vma,
                            a.len or span_len(vma) or 0)

    if a.sweep:
        tu, span = build_tu(a.source, a.function, want_span=True)
        return run_sweep(a, tu, span, vma, a.len or span_len(vma) or 0)

    if a.spill:
        tu, span = build_tu(a.source, a.function, want_span=True)
        return run_sweep(a, tu, span, vma, a.len or span_len(vma) or 0,
                         gen=spill_variants)

    tu = (a.source.read_text(encoding="utf-8") if a.raw_tu
          else build_tu(a.source, a.function))
    origin = a.source
    if a.raw_tu:
        side = WORK / "sweep_best.origin"
        if side.exists():
            origin = Path(side.read_text().strip())
    obj = compile_tu(tu, origin)
    blob = section_blob(obj, a.function)

    size = a.len or span_len(vma) or len(blob)
    ref = disasm(rom_slice(vma, size), vma)
    cand = disasm(blob, vma)

    if a.asm:
        for pc, ins in cand:
            print(f"{pc:08x}  {ins}")
        return 0

    same = 0
    sm = difflib.SequenceMatcher(None, [norm(r[1]) for r in ref],
                                 [norm(c[1]) for c in cand])
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            same += i2 - i1
    print(f"{a.function}: rom {len(ref)} insns / {size} B, cand {len(cand)} insns / "
          f"{len(blob)} B, {same} aligned equal")
    if len(blob) != size:
        print(f"LENGTH {len(blob) - size:+d} B against the {size} B span "
              f"({vma:#010x}-{vma + size:#010x}); the ROM window is fixed")
    sized = len(blob) == size
    if sized and blob == rom_slice(vma, size):
        print("BYTE-EXACT")
        return 0

    if sized and mask_calls(blob) == mask_calls(rom_slice(vma, size)):
        print("BYTE-EXACT apart from unresolved call targets")
        return 0

    m = permuted(rom_slice(vma, size), blob) if sized else None
    if m is not None:
        pairs = " ".join(f"r{k}->r{v}" for k, v in sorted(m.items()))
        print(f"PURE REGISTER PERMUTATION ({pairs}) -- the C is right; only "
              f"agbcc's allocation differs")
    else:
        print("not a pure register permutation -- at least one operation differs")

    print_listing(rom_slice(vma, size), blob, vma, a.all)
    return 1


if __name__ == "__main__":
    sys.exit(main())
