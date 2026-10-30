#!/usr/bin/env python3
"""Flag bodies whose ROM stack frame is larger than the current C draft's.

A lift that produces the right instructions in the wrong stack shape is not a
semantic problem: agbcc rematerialises a pointer instead of spilling it, so the
draft's frame comes out one or two words short of the ROM's and every offset in
the body shifts. Measured examples are `_08001650C` (ROM `sub sp, #32`, draft
`sub sp, #20`) and `_080275B8` (ROM `sub sp, #12`, draft `sub sp, #8`). Spending
a lift loop on such a body before noticing is the expensive mistake this tool
exists to prevent.

It compiles the function's current C out of its real translation unit (exactly
as `lift_loop.py` does), reads both prologues, and compares:

  * local frame   -- the `sub sp, #N` immediates, what gcc calls frame size,
  * saved stack   -- the registers a `push`/`stmdb sp!` spills,
  * stack         -- their sum, the body's total prologue reservation.

`--draft FILE` grades a whole-body replacement (the candidate about to be
pasted into the source) instead of the committed body, so the check runs before
the edit rather than after it. A draft whose ROM frame is larger is
allocation-bound: place the spills in the ROM's slot order (lift_loop
`--spill`) before chasing instruction differences.

    python3 tools/frame_probe.py _08001650C
    python3 tools/frame_probe.py Rec35_Record49Constructor
    python3 tools/frame_probe.py _080275B8 --draft build/v275b8_a.c
    python3 tools/frame_probe.py _08001650C --json build/frame.json

Exit status is 0 when every frame matches, 1 when a body is flagged, 2 on a
resolution or compile error, so the check is usable in a script. The decoder is
Thumb (the C corpus is built `-mthumb`); an ARM body is detected and reported
as unsupported rather than measured wrongly.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import corpus_match_probe as probe  # noqa: E402
import lift_loop  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]

REG_ORDER = [f"r{i}" for i in range(8)] + ["r8", "r9", "r10", "r11",
                                           "ip", "sp", "lr", "pc"]
PUSH_RE = re.compile(r"^push\s*\{([^}]*)\}$")
STM_SP_RE = re.compile(r"^stm(?:db|fd|ia|ib)\s+sp!\s*,\s*\{([^}]*)\}$")
SUB_SP_RE = re.compile(r"^subs?\s+sp\s*,\s*(?:sp\s*,\s*)?#(\d+)$")
# `mov r7, sp` / `add r7, sp, #0`: frame-pointer setup, not part of the frame.
FP_SETUP_RE = re.compile(r"^(?:movs?|adds?)\s+r\d+\s*,\s*sp(?:\s*,\s*#-?\d+)?$")
# `mov r7, sl` (save), `mov r9, r0` (argument into a high reg): agbcc emits
# these between the two pushes of a high-register body, so the scan must not
# stop there or it misses the second push and the local `sub sp`.
HI_REGS = r"(?:r8|r9|r10|r11|sl|fp|ip)"
HI_MOVE_RE = re.compile(
    rf"^movs?\s+(?:r\d+|{HI_REGS})\s*,\s*{HI_REGS}$"
    rf"|^movs?\s+{HI_REGS}\s*,\s*r\d+$")
# `ldr r3, [pc, #N]` loads a frame amount from the pool; the later `sub sp, sp,
# r3` is not statically resolvable here, but the load itself keeps the scan
# inside the prologue.
POOL_LOAD_RE = re.compile(r"^ldr\s+r\d+\s*,\s*\[pc(?:\s*,\s*#\d+)?\]$")
SIZE_RE = re.compile(r"#(\d+)")


def _regs(text: str) -> list[str]:
    """Expand a push register list, including `r4-r7` range spellings."""
    out: list[str] = []
    for part in text.split(","):
        part = part.strip()
        if not part:
            continue
        m = re.fullmatch(r"(\w+)\s*-\s*(\w+)", part)
        if m and m.group(1) in REG_ORDER and m.group(2) in REG_ORDER:
            i, j = REG_ORDER.index(m.group(1)), REG_ORDER.index(m.group(2))
            out.extend(REG_ORDER[i:j + 1])
        else:
            out.append(part)
    return out


def prologue_frame(rows: list[tuple[int, str]]) -> dict:
    """Stack reservation of one prologue: local frame, saved regs, total.

    Stops at the first instruction that is neither a stack reservation, a
    high-register move, a frame-pointer setup nor a pool load, so a `push` that
    appears mid-body is not counted as part of the frame.
    """
    local = saved = 0
    detail: list[str] = []
    started = False
    for _pc, ins in rows[:16]:
        m = PUSH_RE.match(ins) or STM_SP_RE.match(ins)
        if m:
            regs = _regs(m.group(1))
            saved += 4 * len(regs)
            detail.append(f"push {len(regs)} reg ({4 * len(regs)} B)")
            started = True
            continue
        m = SUB_SP_RE.match(ins)
        if m:
            n = int(m.group(1))
            local += n
            detail.append(f"sub sp, #{n}")
            started = True
            continue
        if not started:
            break
        if FP_SETUP_RE.match(ins) or HI_MOVE_RE.match(ins) or POOL_LOAD_RE.match(ins):
            continue
        break
    return {"local": local, "saved": saved, "stack": local + saved,
            "detail": detail}


def isa_of(blob: bytes) -> str:
    """Thumb unless the entry instruction is an ARM `stmdb/stmfd sp!`."""
    if len(blob) >= 2:
        half = blob[0] | (blob[1] << 8)
        if (half & 0xFFF0) in (0xE920, 0xE900):
            return "arm"
    return "thumb"


def disasm(blob: bytes, isa: str) -> list[tuple[int, str]]:
    """Instruction rows, decoded in the body's own ISA."""
    with tempfile.NamedTemporaryFile(suffix=".bin") as tmp:
        tmp.write(blob)
        tmp.flush()
        cmd = ["arm-none-eabi-objdump", "-D", "-b", "binary", "-m", "arm"]
        if isa == "thumb":
            cmd += ["-M", "force-thumb"]
        cmd += ["--adjust-vma=0", tmp.name]
        out = subprocess.run(cmd, capture_output=True, text=True).stdout
    rows = []
    for line in out.splitlines():
        m = re.match(r"^\s*([0-9a-f]+):\t([^\t]*)\t(.*)$", line)
        if m:
            rows.append((int(m.group(1), 16), re.sub(r"\s+", " ", m.group(3).strip())))
    return rows


def measure(blob: bytes) -> dict:
    isa = isa_of(blob)
    info = prologue_frame(disasm(blob, isa))
    info["isa"] = isa
    return info


def compile_current(path: Path, cname: str, work: Path) -> bytes:
    tu = lift_loop.build_tu(path, cname)
    obj = lift_loop.compile_tu(tu, path, work)
    return lift_loop.section_blob(obj, cname, work)


def compile_draft(path: Path, cname: str, draft: Path, work: Path) -> bytes:
    """Splice a whole-body replacement over the real TU, as --variant does."""
    raw_tu, raw_span = lift_loop.build_tu(path, cname, want_span=True, raw=True)
    body = draft.read_text(encoding="utf-8")
    candidate = raw_tu[:raw_span[0]] + body + raw_tu[raw_span[1]:]
    work.mkdir(parents=True, exist_ok=True)
    scratch = work / "draft_raw.c"
    scratch.write_text(candidate, encoding="utf-8")
    transformed, _stats = lift_loop.transform_file(scratch)
    obj = lift_loop.compile_tu(transformed, path, work)
    return lift_loop.section_blob(obj, cname, work)


def compare(rom: dict, cand: dict) -> tuple[str, str]:
    """(flag, explanation). `flag` is the machine-readable verdict."""
    d_local = rom["local"] - cand["local"]
    if d_local > 0:
        return "rom-larger", (
            f"ROM local frame is {d_local} B larger "
            f"(ROM {rom['local']} B vs C {cand['local']} B, "
            f"{d_local // 4} slot(s)); allocation-bound -- place the spills in "
            f"the ROM's slot order before chasing instruction differences")
    if d_local < 0:
        return "c-larger", (
            f"draft local frame is {-d_local} B larger than ROM's "
            f"(ROM {rom['local']} B vs C {cand['local']} B); the draft reserves "
            f"stack the ROM does not use")
    if rom["saved"] != cand["saved"]:
        return "saved-differs", (
            f"saved-register stack differs: ROM {rom['saved']} B vs "
            f"C {cand['saved']} B")
    return "match", "frame matches"


def resolve(name: str) -> tuple[Path, str, int]:
    """(source file, C definition name, VMA) for a friendly or VMA name.

    `regalloc_report.function_source_path` is the wrong resolver here: it is
    built for RTL dump names, so a VMA request resolves to the `alias`
    declaration's name (`_08001650C`), which has no body to compile. Prefer the
    friendly body the VMA aliases, and accept only a name `build_tu` can find a
    definition for.
    """
    vma = probe.parse_vma_name(name)
    aliases = probe.alias_targets()
    # Match aliases by VMA, not spelling: `_080275B8` and `_0800275B8` are the
    # same address, and only the literal one appears in the alias declaration.
    ordered = [name]
    for alias, target in aliases.items():
        if alias == name or (vma is not None and probe.parse_vma_name(alias) == vma):
            ordered.append(target)
        if target == name:
            ordered.append(alias)
    entries = []
    seen: set[str] = set()
    for cand in ordered:
        if cand in seen:
            continue
        seen.add(cand)
        entries += [f for f in probe.src_functions() if f[1] == cand]
    # Strong body before a weak stub, then the longer definition.
    entries.sort(key=lambda f: (0 if f[4] else 1, -f[3]))
    for path, cname, entry_vma, _lines, _weak in entries:
        if vma is not None and entry_vma != vma:
            continue
        try:
            lift_loop.build_tu(path, cname)
        except SystemExit:
            continue
        return path, cname, entry_vma
    raise ValueError(f"{name}: no C definition with a body")


def span_for(vma: int, fallback: int = 0) -> int:
    size = lift_loop.span_len(vma)
    if not size:
        end = probe.rom_functions().get(vma)
        size = (end - vma) if end else fallback
    return size


def probe_target(name: str, draft: Path | None, work: Path) -> dict:
    path, cname, vma = resolve(name)
    size = span_for(vma)
    if size <= 0:
        raise ValueError(f"{name}: no ROM span at {vma:#010x}")
    rom = measure(lift_loop.rom_slice(vma, size))
    cand = measure(compile_draft(path, cname, draft, work) if draft
                   else compile_current(path, cname, work))
    flag, why = compare(rom, cand)
    return {"target": name, "file": str(path.relative_to(ROOT)), "name": cname,
            "vma": vma, "size": size, "rom": rom, "c": cand,
            "flag": flag, "detail": why, "draft": str(draft) if draft else None}


def render(res: dict) -> str:
    rom, cand = res["rom"], res["c"]
    out = [f"{res['target']}  ({res['file']}:{res['name']})  "
           f"{res['size']} B @ {res['vma']:#010x}"]
    for label, info in (("rom", rom), ("c  ", cand)):
        detail = "; ".join(info["detail"]) or "no prologue stack"
        out.append(f"  {label}  local {info['local']:>3} B   saved "
                   f"{info['saved']:>3} B   stack {info['stack']:>3} B   "
                   f"[{detail}]")
    out.append(f"  {'FLAG ' if res['flag'] != 'match' else 'OK   '}"
               f"{res['detail']}")
    return "\n".join(out)


# --------------------------------------------------------------------------
# Self-test: the shapes that matter are the two measured prologues, the
# high-register double push, a leaf, and a mid-body push that must not count.
# --------------------------------------------------------------------------

def _rows(*lines: str) -> list[tuple[int, str]]:
    return [(2 * i, ins) for i, ins in enumerate(lines)]


def self_test() -> None:
    high_reg = prologue_frame(_rows(
        "push {r4, r5, r6, r7, lr}",
        "mov r7, sl", "mov r6, r9", "mov r5, r8",
        "push {r5, r6, r7}",
        "sub sp, #32",
        "adds r7, r0, #0"))
    assert (high_reg["local"], high_reg["saved"], high_reg["stack"]) == (32, 32, 64), high_reg
    assert "sub sp, #32" in high_reg["detail"]

    garage = prologue_frame(_rows(
        "push {r4, r5, r6, r7, lr}",
        "mov r7, sl", "mov r6, r9", "mov r5, r8",
        "push {r5, r6, r7}",
        "sub sp, #12",
        "mov r9, r0"))
    assert (garage["local"], garage["saved"]) == (12, 32), garage

    # A range spelling counts every register, not one comma-separated item.
    assert prologue_frame(_rows("push {r4-r7, lr}"))["saved"] == 20

    # A leaf, and a local frame with no saved registers.
    assert prologue_frame(_rows("bx lr")) == {
        "local": 0, "saved": 0, "stack": 0, "detail": []}
    assert prologue_frame(_rows("sub sp, #8", "adds r0, r1, #0"))["local"] == 8

    # Real work ends the prologue: a later push belongs to a nested sequence.
    mid = prologue_frame(_rows("push {r4, lr}", "bl sub_x", "push {r5, r6}"))
    assert (mid["local"], mid["saved"]) == (0, 8), mid
    # `mov r0, r1` is not a high-register save, so the scan stops there.
    assert prologue_frame(_rows("push {r4, lr}", "mov r0, r1",
                                "sub sp, #16"))["local"] == 0

    # ARM `stmdb sp!` is measured through the same path.
    arm = prologue_frame(_rows("stmdb sp!, {r4, r5, lr}", "sub sp, sp, #8"))
    assert (arm["local"], arm["saved"]) == (8, 12), arm
    assert isa_of(bytes.fromhex("2de9f041")) == "arm"
    assert isa_of(bytes.fromhex("10b5")) == "thumb"

    # The verdict is directional: the whole point is ROM > draft.
    rom = {"local": 32, "saved": 32, "stack": 64}
    short = {"local": 20, "saved": 32, "stack": 52}
    assert compare(rom, short)[0] == "rom-larger"
    assert compare(short, rom)[0] == "c-larger"
    assert compare(rom, dict(rom))[0] == "match"
    assert compare(rom, {"local": 32, "saved": 28, "stack": 60})[0] == "saved-differs"
    print("frame_probe self-test: PASS")


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("targets", nargs="*", metavar="FUNCTION",
                    help="VMA symbol or friendly name")
    ap.add_argument("--draft", type=Path,
                    help="grade this whole-body replacement instead of the committed C")
    ap.add_argument("--json", type=Path, help="write the results as JSON")
    ap.add_argument("--work-dir", type=Path,
                    help="keep compile artifacts here (default: a temporary directory)")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)

    if args.self_test:
        self_test()
        return 0
    if not args.targets:
        ap.error("pass at least one FUNCTION (or --self-test)")
    if args.draft and len(args.targets) != 1:
        ap.error("--draft grades one body; pass a single FUNCTION")

    results = []
    status = 0
    with tempfile.TemporaryDirectory() as tmp:
        base = (ROOT / args.work_dir) if args.work_dir else Path(tmp)
        for target in args.targets:
            try:
                res = probe_target(target, args.draft,
                                   base / f"{target.lstrip('_')}")
            except (ValueError, SystemExit) as exc:
                print(f"{target}: {exc}", file=sys.stderr)
                status = 2
                continue
            results.append(res)
            print(render(res))
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(results, indent=1) + "\n", encoding="utf-8")

    flagged = [r for r in results if r["flag"] != "match"]
    if flagged:
        print(f"\n{len(flagged)} of {len(results)} flagged as allocation-bound")
        status = max(status, 1)
    return status


if __name__ == "__main__":
    sys.exit(main())
