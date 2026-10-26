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

`--sweep REG[,REG...]` inserts an `__asm__ volatile("" : "+r"(REG))`
barrier at every statement boundary of the body, one (register, boundary)
pair per iteration, and ranks the results. That barrier is the only lever that
promotes a pseudo to a *global* allocno, which is the precondition for r7 and
for a constant-copy spill; the placement decides how far the range stretches
and therefore its `allocno_compare` priority, so the worthwhile placements are
a sweep rather than a guess. The best variant is written to
`build/lift/sweep_best.c` for pasting back.

`--all` prints every aligned row instead of the first mismatch window.
`--asm` prints only the candidate listing (no ROM rows).

The scratch TU keeps the whole file's prologue and the requested function, so
the surrounding declarations, includes and `__attribute__((alias))` lines are
the real ones. Nothing under `src/` is written; the scratch tree is ignored
build output. Exit status is 0 when the extracted range is byte-identical.
"""

from __future__ import annotations

import argparse
import difflib
import os
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor, as_completed as as_complete
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import corpus_match_probe as probe  # noqa: E402
from agbcc_c89_transform import strip_noncode  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / "build/lift"
ROM = ROOT / "baserom.gba"

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


def build_tu(path: Path, name: str, want_span: bool = False):
    """The whole file with every function body but `name` blanked out.

    Keeping the file intact means typedefs, macros, `__asm__` symbol bindings
    and the include list are exactly the ones the real TU compiles with, so the
    scratch body sees the same types and constants. Only the other bodies are
    removed, and their declarations stay intact.

    `want_span` returns the kept body's `{ ... }` range as well, which is what
    the barrier sweep needs to know where it may insert.
    """
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
            span = (start - delta, end - delta)
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
    return ROM.read_bytes()[off:off + size]


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
    return binpath.read_bytes()


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
    if blob == ref:
        kind = "exact"
    elif mask_calls(blob) == mask_calls(rom_slice(vma, len(blob))):
        kind = "exact-calls"
    elif permuted(rom_slice(vma, len(blob)), blob) is not None:
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


def barrier_variants(tu: str, span: tuple[int, int], regs: list[str]) -> list[tuple[str, str]]:
    """(label, tu) for every (register, insertion point) barrier placement."""
    out = []
    seen = set()
    for idx, at in enumerate(statement_points(tu, span)):
        for reg in regs:
            stmt = '\n__asm__ volatile("" : "+r"(%s));' % reg
            cand = tu[:at] + stmt + tu[at:]
            if cand in seen:
                continue
            seen.add(cand)
            out.append((f"p{idx:02d}/{reg}", cand))
    return out


def run_sweep(a, tu: str, span, vma: int, size: int) -> int:
    regs = [r.strip() for r in a.sweep.split(",") if r.strip()]
    variants = barrier_variants(tu, span, regs)
    if a.points:
        keep = {int(p.strip().lstrip("p")) for p in a.points.split(",") if p.strip()}
        variants = [v for v in variants if int(v[0].split("/")[0][1:]) in keep]
    print(f"sweep: {len(variants)} variants x {regs}", file=sys.stderr)
    base = evaluate(tu, a.source, a.function, vma, size, WORK / "sweep/base")
    results = []
    base_dir = WORK / "sweep"
    with ThreadPoolExecutor(max_workers=a.jobs) as pool:
        futs = {pool.submit(evaluate, cand, a.source, a.function, vma, size,
                            base_dir / f"v{i:03d}"): label
                for i, (label, cand) in enumerate(variants)}
        for fut in as_complete(futs):
            label = futs[fut]
            try:
                res = fut.result()
            except Exception as exc:  # noqa: BLE001 - one bad variant must not end the sweep
                res = {"kind": "error", "msg": repr(exc), "size": 0,
                       "aligned": -1, "blob": b"", "tu": ""}
            results.append((label, res))

    def key(item):
        rank = {"exact": 0, "exact-calls": 1, "permuted": 2, "differs": 3,
                "error": 9}[item[1]["kind"]]
        return (rank, -item[1]["aligned"], abs(item[1]["size"] - size) if size else 0)

    results.sort(key=key)
    pad = f"{'variant':<12} {'kind':<11} {'size':>5} {'aligned':>7}"
    print(pad)
    print(f"{'baseline':<12} {base['kind']:<11} {base['size']:>5} {base['aligned']:>7}")
    for label, res in results[:a.top]:
        extra = "" if not res["msg"] else "  " + res["msg"].splitlines()[0][:70]
        print(f"{label:<12} {res['kind']:<11} {res['size']:>5} {res['aligned']:>7}{extra}")

    if not results:
        return 1
    rank = {"exact": 0, "exact-calls": 1, "permuted": 2, "differs": 3, "error": 9}
    label, best = results[0]
    improved = rank[best["kind"]] < rank[base["kind"]] or (
        best["kind"] == base["kind"] and best["aligned"] > base["aligned"])
    if improved and best["tu"]:
        best_path = WORK / "sweep_best.c"
        best_path.write_text(best["tu"], encoding="utf-8")
        # The variant is already past the C89 transform, so its real TU origin
        # has to travel with it for `--raw-tu` to compile it the same way.
        (WORK / "sweep_best.origin").write_text(str(a.source.resolve()))
        print(f"\nbest: {label} -> {best_path}")
    else:
        print("\nno placement beat the baseline")
    return 0 if best["kind"] == "exact" else 1


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("source", type=Path)
    ap.add_argument("function")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--asm", action="store_true")
    ap.add_argument("--len", type=lambda s: int(s, 0))
    ap.add_argument("--vma", type=lambda s: int(s, 0))
    ap.add_argument("--sweep", metavar="REG[,REG...]",
                    help="insert a +r barrier at every statement boundary and rank")
    ap.add_argument("--points", metavar="P[,P...]",
                    help="restrict the sweep to these statement indices")
    ap.add_argument("--jobs", type=int, default=0, help="parallel sweeps (0 = cpu count)")
    ap.add_argument("--top", type=int, default=10)
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
            m = re.fullmatch(r"_?(080[0-9A-Fa-f]{5})", a.function)
            if m:
                vma = int(m.group(1), 16)
    if not vma:
        raise SystemExit("pass --vma (or use an _0802xxxx symbol name)")

    if a.sweep:
        tu, span = build_tu(a.source, a.function, want_span=True)
        return run_sweep(a, tu, span, vma, a.len or 0)

    tu = (a.source.read_text(encoding="utf-8") if a.raw_tu
          else build_tu(a.source, a.function))
    origin = a.source
    if a.raw_tu:
        side = WORK / "sweep_best.origin"
        if side.exists():
            origin = Path(side.read_text().strip())
    obj = compile_tu(tu, origin)
    blob = section_blob(obj, a.function)

    size = a.len or len(blob)
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
    if blob == rom_slice(vma, len(blob)):
        print("BYTE-EXACT")
        return 0

    if mask_calls(blob) == mask_calls(rom_slice(vma, len(blob))):
        print("BYTE-EXACT apart from unresolved call targets")
        return 0

    m = permuted(rom_slice(vma, len(blob)), blob)
    if m is not None:
        pairs = " ".join(f"r{k}->r{v}" for k, v in sorted(m.items()))
        print(f"PURE REGISTER PERMUTATION ({pairs}) -- the C is right; only "
              f"agbcc's allocation differs")
    else:
        print("not a pure register permutation -- at least one operation differs")

    shown = 0
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            if a.all:
                for k in range(i2 - i1):
                    print(f"   {ref[i1 + k][0]:08x}  ref: {ref[i1 + k][1]}")
            continue
        if a.all:
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
            if shown >= 3:
                break
    return 1


if __name__ == "__main__":
    sys.exit(main())
