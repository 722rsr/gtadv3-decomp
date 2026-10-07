#!/usr/bin/env python3
"""Find corpus spans that swallow a real function boundary.

`corpus_match_probe._rom_functions_scan()` gives every ROM VMA the extent of the
NEXT KNOWN LABEL. That is right only if the label set is complete. When asm/ is
missing a label, one span silently covers two or more functions -- and every
number derived from it is wrong: the byte denominator, the match percentage, and
therefore which bodies a comparison is told to prioritise.

`0x08005BA8` is the known case: asm/ has no label at 0x08005BF0, so the span
runs to 0x08005C58 and reads 176 bytes for a 72-byte function. The score reports
21/176 where the achievable maximum is 21/72, and a body can sit permanently
"unpromotable" for a reason that has nothing to do with the C.

TWO DETECTORS, because neither alone is enough
---------------------------------------------
1. PROLOGUE SCAN (`scan_prologues`). Every even offset whose first halfword is a
   Thumb `push` that saves `lr`. Needs no precondition, so it is strictly more
   complete than gating on branch targets, but a `push {r4,lr}` can sit
   mid-function, so every hit needs adjudicating.

2. SHAPE SCAN (`entry_at`). The case that motivated the second detector is
   `0x08004560`: a bare label followed by `bx lr` and one padding byte
   (`asm/runtime_2aac.s:3553`). It carries NO `.type` line and NOTHING branches
   to it, so `coverage.py`'s `asm_vmas()` never admits it as an entry and
   `sub_08004508` is measured 92 bytes wide instead of 88. Detector 1 cannot see
   it either -- a bare `bx lr` leaf has no prologue to scan for. A detector that
   keys on `.type` presence, or on `bl` reachability, is blind to exactly this
   shape, which is why the previous revision of this tool reported
   "0 spans swallow an unlabelled entry" on a tree that contains one.

   The shape scan therefore keys on ADDRESS DISTANCE and INSTRUCTION SHAPE. A
   label `a`, with the next closure label at `n`, starts its own function iff all
   three gates hold:

     G1  `a` starts a decodable instruction: 2-byte aligned, not alignment
         padding, not inside a literal-pool word, not the tail of a 32-bit
         instruction;
     G2  the nearest real instruction BEFORE `a` is an exit -- `bx lr`,
         `pop {..,pc}`, `pop {rX}; bx rX`, or an unconditional transfer
         (`b`/`b.n`/`bl`/`blx`/`bx rN`). G2 walks BACKWARDS, because the previous
         closure label is often the pool word that terminates the function
         above, so a forward scan reads data as code;
     G3  `a`'s own body reaches ITS OWN return before `n`, with no pool word
         and no undefined instruction in between.

   G2 separates `0x08004560` (real -- the epilogue is `pop {r1}; bx r1` at
   0x08004554, then one pad byte and the two literals `_08004558`/`_0800455C`)
   from shared-tail labels such as `0x08005B62` (not real -- `0x08005B60 negs
   r0, r0` falls straight into the `bx lr`). G1 separates a real entry from a
   literal-pool word whose low halfword happens to be `0x4770`: `0x08002818`,
   `0x0800282C` and `0x08002840` are all `.4byte 0x080C4770`
   (`asm/code_279c.s:67,76,85`), not three tiny functions.

POOL DETECTION IS NOT A HEURISTIC
---------------------------------
objdump annotates a pc-relative literal load with the address of the word it
reads: `ldr r0, [pc, #64]  @ (0x4558)`. Those four bytes are DATA. Reading them
as instructions is not a rare corner -- it is exactly what hid sub_08004508's
real epilogue behind a bogus `movs r6, r1` at 0x0800455E -- so every scan here
skips them, and a candidate that starts inside one is rejected outright.

The ABS-branch-target detector from the first version is kept only as a count.
It is a LOWER BOUND on real entries (it finds only entries something calls) and
it is the reason this tool originally reported a clean tree.

Usage:
    python3 tools/span_audit.py                 # human table
    python3 tools/span_audit.py --json
    python3 tools/span_audit.py --self-test
"""

import bisect
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
NM = "arm-none-eabi-nm"
OBJDUMP = "arm-none-eabi-objdump"
CODE_O = ROOT / "build-code" / "code.o"
REPORT = ROOT / "build" / "era-corpus" / "ready-report.json"
DIS_CACHE = ROOT / "build" / "scratch" / "span_audit_dis.txt"
ROM_BASE = 0x08000000
SLICE_HI = 0x2E158   # the 188760-byte independent slice

# gas names an ABS symbol "*ABS*0x<addr>". The `*ABS*` prefix is part of the
# NAME COLUMN, not the type column: `nm` reports these as type `a` with the
# marker embedded in the name. Two ways to get this wrong, both of which report
# a CLEAN TREE -- the worst possible failure for a tool whose whole job is to
# find things:
#   * matching bare hex digits, which misses the `*ABS*` prefix entirely;
#   * demanding exactly 8 digits, when ROM addresses here are 7 (`*ABS*0x8000a60`).
# So: 7 or 8, both cases, anchored.
ABS_RE = re.compile(r"^\*ABS\*0x([0-9a-fA-F]{7,8})$")

# `   4518:\t4810      \tldr\tr0, [pc, #64]\t@ (0x4558)`
INSN_RE = re.compile(r"^\s*([0-9a-f]+):\t([^\t]*)\t(.*)$")
# objdump's own annotation naming the literal word a pc-relative load reads.
POOLREF_RE = re.compile(r"\[pc,\s*#-?\d+\]\s*@\s*\((0x[0-9a-f]+)\)")
# UNCONDITIONAL transfers only. `bne.n` and friends fall THROUGH, so a rule that
# counted them would call every interior branch target an entry; `bic`, `bfc`
# and `bfi` merely start with `b`.
UNCOND_RE = re.compile(r"^(?:b|bl|blx)(?:\.[nw])?$|^bx\s+\w+$")
REG_BX_RE = re.compile(r"^bx (r\d+|sl|sb|fp|ip|sp)$")


# Addresses the prologue scan flags that are NOT function entries, each with the
# evidence that settled it. A prologue is strong evidence, not proof: an
# `if (f) g();` the compiler inlined produces `push {r0,lr}` with no `pop`/`bx`
# of its own, and the scan cannot tell that from an entry.
#
# Do not add an address here to make a number look better. Each one needs the
# same evidence, and the audit still prints anything it has no record for.
KNOWN_NON_ENTRIES = {
    0x0802BEC2: (
        "NOT an entry: an inlined `if (f56) f60();`. The instruction before it "
        "is `str r3, [r0, #52]` at 0x0802BEC0 -- neither an epilogue nor a pool "
        "word. The block falls THROUGH into _0802BED0's `pop {r0}`, the "
        "continuation of sub_0802BEB4, and never emits an epilogue of its own. "
        "Corroborated three ways: no 32-bit literal anywhere in baserom.gba "
        "equals 0x0802BEC2 or +1, no bl targets it, and no branch targets it. "
        "_0802BEB4 is genuinely a 600-byte function, so leaving its span whole "
        "is correct."
    ),
}


def parse_symbols(nm_output):
    """[(vma, name, type)] from `nm` output, VMAs corrected by the ROM base.

    A relocatable .o reports section-relative FILE OFFSETS; skipping the base
    makes every symbol look shifted by -0x08000000, which reads as a mass
    failure rather than the constant it is.
    """
    rows = []
    for line in nm_output.splitlines():
        f = line.split()
        if len(f) != 3:
            continue
        off, typ, name = f
        try:
            rows.append((int(off, 16) + ROM_BASE, name, typ))
        except ValueError:
            continue
    return rows


def _is_thumb_prologue(rom, off):
    """True if a Thumb function prologue starts at file offset `off`.

    A branch target is NOT evidence of a function entry: any interior label in
    a function is a branch target, so counting them flags every large function
    as mis-measured. A PUSH that saves the link register is much stronger -- it
    is how this ROM marks an entry.

    Thumb PUSH is `1011 010M reglist`; the bit-8 halfword is `0xB5xx` with
    `lr` (bit 3 of the low byte) set. Reading the first HALFWORD and requiring
    an even file offset keeps this aligned: a misaligned read yields a garbage
    decode, which is how a mis-anchored objdump produces `vmaxnm.f16`.
    """
    if off % 2 or off + 2 > len(rom):
        return False
    hw = rom[off] | (rom[off + 1] << 8)
    if (hw & 0xFE00) != 0xB400:   # 1011 010x xxxx xxxx
        return False
    return bool(hw & 0x0100)       # bit 8 of the low halfword: lr in the list


def scan_prologues(rom, lo=0, hi=None):
    """Every even offset in the text where a Thumb function prologue starts.

    This replaces a branch-target precondition that was WRONG and made the tool
    blind to the case it was built for. Requiring the missing address to also be
    a numeric `bl` operand only finds entries that are CALLED. 0x08005BF0 and
    0x08005C3C are entries nothing branches to -- reached by fallthrough or a
    table -- so they emit no ABS symbol and the original detector could not see
    them even though the whole tool exists because of them.

    A prologue is also not proof of a function: a `push {r4,lr}` can sit
    mid-function. But scanning for them needs no precondition, so it is
    strictly more complete than gating on branch targets, and every hit is
    checkable with one disassembly.
    """
    hi = len(rom) if hi is None else hi
    return [off for off in range(lo, min(hi, len(rom)) - 1, 2)
            if _is_thumb_prologue(rom, off)]


# --------------------------------------------------------------------------
# Disassembly. Bounded to the slice, and cached: this tool is run far more
# often than baserom.gba changes.
# --------------------------------------------------------------------------

def disassemble(rom_path=ROOT / "baserom.gba", hi=SLICE_HI, cache=DIS_CACHE):
    """{offset: (bytes, mnemonic, operands, text, raw)} for the Thumb text.

    `text` has objdump's `@ ...` annotation stripped; `raw` keeps it, because
    the annotation is the only reliable statement of which bytes are literal
    pool.
    """
    rom_path = Path(rom_path)
    if cache is not None and Path(cache).exists() \
            and Path(cache).stat().st_mtime >= rom_path.stat().st_mtime:
        text = Path(cache).read_text()
    else:
        text = subprocess.run(
            [OBJDUMP, "-D", "-b", "binary", "-m", "armv4t", "-M",
             "force-thumb", "--start-address=0", f"--stop-address=0x{hi:x}",
             str(rom_path)],
            capture_output=True, text=True).stdout
        if cache is not None:
            Path(cache).parent.mkdir(parents=True, exist_ok=True)
            Path(cache).write_text(text)
    out = {}
    for line in text.splitlines():
        m = INSN_RE.match(line)
        if m is None:
            continue
        raw = m.group(3).strip()
        mnem_ops = raw.split("@")[0].strip().replace("\t", " ")
        parts = mnem_ops.split(" ", 1)
        out[int(m.group(1), 16)] = (
            m.group(2).replace(" ", ""),
            parts[0],
            parts[1].strip() if len(parts) > 1 else "",
            mnem_ops,
            raw,
        )
    return out


def pool_slots(dis):
    """Every BYTE of every pc-relative literal word.

    Only the four annotated bytes count. A 0x4770 in the padding that follows a
    pool word is code, not data, and calling it data would hide a real entry.
    """
    out = set()
    for e in dis.values():
        m = POOLREF_RE.search(e[4])
        if m:
            a = int(m.group(1), 16)
            out.update(range(a, a + 4))
    return out


def _text(dis, off):
    e = dis.get(off)
    return e[3] if e else None


def is_padding(dis, off):
    """`nop` / `movs r0, r0` -- the alignment filler gas emits between blocks."""
    t = _text(dis, off)
    return t is not None and (t == "nop" or t == "movs r0, r0")


def is_epilogue(dis, off, pool):
    """True if the non-pool instruction at `off` returns to the caller."""
    t = _text(dis, off)
    if t is None or is_padding(dis, off) or any(off + k in pool for k in range(4)):
        return False
    if t == "bx lr" or (t.startswith("pop ") and "pc" in t):
        return True
    m = REG_BX_RE.match(t)
    if m:                              # `pop {rX}; bx rX` is the GBA return idiom
        p = _text(dis, off - 2)
        return bool(p and p.startswith("pop ")
                    and re.search(r"\b" + m.group(1) + r"\b", p))
    return False


def predecessor(dis, off, pool, floor=0):
    """The nearest real (non-padding, non-pool, defined) instruction before `off`.

    Backwards on purpose. The previous closure label is frequently the literal
    pool word that ends the function above -- for `0x08004560` it is
    `_0800455C`, one of sub_08004508's two literals -- so a forward scan from it
    reads data as code and reports garbage.
    """
    cur = off - 2
    while cur >= floor:
        if any(cur + k in pool for k in range(4)):
            cur -= 2
            continue
        e = dis.get(cur)
        if e is None or is_padding(dis, cur) \
                or ".word" in e[4] or "UNDEFINED" in e[4]:
            cur -= 2
            continue
        return cur
    return None


def entry_at(dis, off, nxt, pool, floor=0):
    """Is `off` a real function entry that owns the bytes up to `nxt`?

    Returns `(g1, g2, g3)`; all three must hold. Offsets are FILE OFFSETS, the
    unit `disassemble` and `scan_prologues` both use.
    """
    e0 = dis.get(off)
    g1 = (e0 is not None and not is_padding(dis, off)
          and not any(off + k in pool for k in range(4))
          and ".word" not in e0[4] and "UNDEFINED" not in e0[4])

    p = predecessor(dis, off, pool, floor)
    pt = _text(dis, p) if p is not None else ""
    # An unconditional transfer never falls through, so whatever follows it
    # starts something new. That covers `b`/`bl`, and the `bx rN` veneer table
    # at 0x0802DDC8..0x0802DDFF (asm/sound_veneer.s:55-76).
    g2 = bool(pt) and (is_epilogue(dis, p, pool)
                       or bool(UNCOND_RE.match(pt)))

    body_last, bad = None, False
    cur = off
    while cur < nxt:
        if any(cur + k in pool for k in range(4)):
            bad = True
            break
        e = dis.get(cur)
        if e is None or ".word" in e[4] or "UNDEFINED" in e[4]:
            bad = True
            break
        if not is_padding(dis, cur):
            body_last = e[3]
        cur += len(e[0]) // 2
    g3 = (not bad and body_last is not None
          and (body_last == "bx lr"
               or (body_last.startswith("pop ") and "pc" in body_last)))
    return g1, g2, g3


def _spans(report_path):
    rep = json.loads(Path(report_path).read_text())
    results = rep["results"] if isinstance(rep, dict) else rep
    spans = []
    for r in results:
        try:
            start = int(r["vma"], 16)
            end = start + int(r["rom_bytes"])
        except (KeyError, ValueError):
            continue
        spans.append((start, end, r))
    return spans


def audit(report_path=REPORT, obj=CODE_O, rom_path=ROOT / "baserom.gba"):
    spans = _spans(report_path)

    out = subprocess.run([NM, str(obj)], capture_output=True, text=True).stdout
    syms = parse_symbols(out)
    labelled = sorted({v for v, name, _t in syms if not ABS_RE.match(name)})
    labelled_set = set(labelled)
    abs_targets = set()
    for _v, name, _t in syms:
        m = ABS_RE.match(name)
        if m:
            # gas emits the operand's FULL 32-bit value, so the ROM base is
            # already inside the name. Adding it again lands past the end of
            # the ROM and silently finds nothing.
            abs_targets.add(int(m.group(1), 16))

    rom = Path(rom_path).read_bytes()
    # scan_prologues returns FILE OFFSETS; spans are VMAs. Convert once, here,
    # or every comparison silently fails and the tool reports a clean tree.
    prologues = [off + ROM_BASE for off in scan_prologues(rom, 0, SLICE_HI)]

    dis = disassemble(rom_path)
    pool = pool_slots(dis)
    next_label = {}
    for i, v in enumerate(labelled):
        next_label[v] = (labelled[i + 1] if i + 1 < len(labelled)
                         else v + 0x10000)

    bad, excused = [], []
    for start, end, r in spans:
        # A missing label is an address that looks like an entry and carries no
        # label. No requirement that anything branches to it.
        missing = sorted(a for a in prologues
                         if start < a < end and a not in labelled_set)
        excused.extend(a for a in missing if a in KNOWN_NON_ENTRIES)
        missing = [a for a in missing if a not in KNOWN_NON_ENTRIES]
        # Second detector: a label with no `.type` and no `bl` whose shape is a
        # complete function. Detector 1 above cannot see these at all.
        lo = bisect.bisect_right(labelled, start)
        hi = bisect.bisect_left(labelled, end)
        shape = []
        for a in labelled[lo:hi]:
            if entry_at(dis, a - ROM_BASE, next_label[a] - ROM_BASE, pool) \
                    == (True, True, True):
                shape.append(a)
        if not missing and not shape:
            continue
        bad.append({
            "vma": r["vma"],
            "name": r.get("name", "?"),
            "rom_bytes": r["rom_bytes"],
            "matched_bytes": r.get("matched_bytes"),
            "source": r.get("source", "?"),
            "missing_labels": [f"0x{a:08x}" for a in missing],
            "shape_labels": [f"0x{a:08x}" for a in shape],
            "branched_to": sum(1 for a in missing + shape if a in abs_targets),
        })
    bad.sort(key=lambda d: -(len(d["missing_labels"]) + len(d["shape_labels"])))
    return {
        "spans_checked": len(spans),
        "prologues_in_text": len(prologues),
        "abs_targets": len(abs_targets),
        "pool_bytes": len(pool),
        "mis_measured": bad,
        "unlabelled_entries": sum(len(d["missing_labels"]) + len(d["shape_labels"])
                                  for d in bad),
        "adjudicated_non_entries": [
            {"addr": f"0x{a:08x}", "why": KNOWN_NON_ENTRIES[a]}
            for a in sorted(set(excused))
        ],
    }


def _check(name, got, want):
    ok = got == want
    print(f"  {'PASS' if ok else 'FAIL'} {name}: got {got!r} want {want!r}")
    return ok


def self_test():
    """Synthetic fixtures, then one labelled live-artifact regression.

    The fixtures are hand-written so they cannot be satisfied by the build tree.
    The block at the end is the only part that reads real artifacts; it is the
    regression for the blind spot detector 2 exists to cover, and it carries its
    own negative controls.
    """
    ok = True
    ok &= _check("VMAs get the ROM base added",
                 parse_symbols("0002bb8 t _08002BB8")[0][0], 0x08002BB8)
    ok &= _check("an ABS symbol is recognised as a branch target",
                 bool(ABS_RE.match("*ABS*0x080055f4")), True)
    ok &= _check("a real name is not",
                 bool(ABS_RE.match("sub_08005BA8")), False)
    ok &= _check("a 7-digit ABS name IS matched (ROM addrs are 7 digits)",
                 bool(ABS_RE.match("*ABS*0x8000a60")), True)
    ok &= _check("a non-ABS name is not matched",
                 bool(ABS_RE.match("*ABS*")), False)
    # gas emits the operand's FULL 32-bit value, so the ROM base is already
    # inside the name. Adding ROM_BASE again lands past the end of the ROM and
    # finds nothing -- the failure mode this assertion exists to prevent.
    ok &= _check("no ROM base is added to an ABS name",
                 int(ABS_RE.match("*ABS*0x80055f4").group(1), 16), 0x080055F4)
    ok &= _check("adding the base anyway escapes the ROM",
                 int(ABS_RE.match("*ABS*0x80055f4").group(1), 16) + ROM_BASE,
                 0x100055F4)

    # The detector's core predicate, on a synthetic span set.
    spans = [(0x08005BA8, 0x08005C58)]
    interior = sorted(a for a in {0x08005BF0, 0x08005C3C}
                      if spans[0][0] < a < spans[0][1])
    ok &= _check("two interior entries are found", len(interior), 2)
    ok &= _check("the span start is not counted", 0x08005BA8 in interior, False)
    ok &= _check("the span end is not counted", 0x08005C58 in interior, False)

    # The prologue test, on real Thumb encodings.
    #   0xB570 = push {r4,r5,r6,lr}   (0xB4xx family, lr bit set)
    #   0xB510 = push {r4,lr}
    #   0x4700 = bx lr   -- an epilogue, NOT a prologue
    #   0xB400 = push without lr      -- a data push, not an entry
    rom = bytes([0x70, 0xB5, 0x10, 0xB5, 0x00, 0x47, 0x00, 0xB4])
    ok &= _check("push {r4,r5,r6,lr} is a prologue", _is_thumb_prologue(rom, 0), True)
    ok &= _check("push {r4,lr} is a prologue", _is_thumb_prologue(rom, 2), True)
    ok &= _check("bx lr is NOT a prologue", _is_thumb_prologue(rom, 4), False)
    ok &= _check("push without lr is NOT a prologue", _is_thumb_prologue(rom, 6), False)
    ok &= _check("a misaligned read is refused", _is_thumb_prologue(rom, 1), False)
    # Unit boundary: scan_prologues yields FILE OFFSETS, spans are VMAs. Mixing
    # them reports a clean tree rather than an error, which is how the first
    # version of this tool found 0 of 1249 prologues.
    found = scan_prologues(rom, 0, len(rom))
    ok &= _check("scan_prologues yields offsets, not VMAs",
                 [o for o in found if o < 0x1000], [0, 2])
    ok &= _check("a read past the end is refused", _is_thumb_prologue(rom, 8), False)

    # A span with no interior entry must be clean -- the negative direction.
    clean = (0x08005BA8, 0x08005BF0)
    ok &= _check("a correctly bounded span reports nothing",
                 [a for a in {0x08005BF0, 0x08005C3C}
                  if clean[0] < a < clean[1]], [])

    # ---- shape detector -----------------------------------------------------
    # One hand-built disassembly exercising every gate. It reproduces the
    # sub_08004508 shape exactly: a `ldr [pc, #..]` naming a pool word, then the
    # `pop {r1}; bx r1` return, then padding, then the pool, then two bare
    # `bx lr` leaves, then a shared tail.
    def I(hexbytes, mnem, ops, raw):
        return (hexbytes, mnem, ops, f"{mnem} {ops}".strip(), raw)

    dis = {
        0x00: I("b510", "push", "{r4, lr}", "push\t{r4, lr}"),
        0x02: I("4812", "ldr", "r0, [pc, #8]",
                "ldr\tr0, [pc, #8]\t@ (0x0c)"),
        0x04: I("bc01", "pop", "{r1}", "pop\t{r1}"),
        0x06: I("4708", "bx", "r1", "bx\tr1"),
        0x08: I("0000", "movs", "r0, r0", "movs\tr0, r0"),
        0x0A: I("0000", "movs", "r0, r0", "movs\tr0, r0"),
        # pool word 0x0C..0x0F; its low halfword decodes as `bx lr`
        0x0C: I("4770", "bx", "lr", "bx\tlr"),
        0x0E: I("080c", "lsrs", "r4, r1, #32", "lsrs\tr4, r1, #32"),
        0x10: I("4770", "bx", "lr", "bx\tlr"),     # entry under test
        0x12: I("0000", "movs", "r0, r0", "movs\tr0, r0"),
        0x14: I("4770", "bx", "lr", "bx\tlr"),     # the next leaf entry
        0x16: I("0000", "movs", "r0, r0", "movs\tr0, r0"),
        0x18: I("4248", "negs", "r0, r0", "negs\tr0, r0"),
        0x1A: I("4770", "bx", "lr", "bx\tlr"),     # shared tail, NOT an entry
        0x1C: I("0000", "movs", "r0, r0", "movs\tr0, r0"),
    }
    pool = pool_slots(dis)
    ok &= _check("a pc-relative literal slot is found",
                 sorted(pool), [0x0C, 0x0D, 0x0E, 0x0F])
    ok &= _check("padding after a pool word is NOT pool",
                 any(a in pool for a in (0x10, 0x12, 0x14)), False)
    ok &= _check("the back-scan skips pad+pool and finds the return",
                 predecessor(dis, 0x10, pool), 0x06)
    ok &= _check("`pop {r1}; bx r1` is an epilogue",
                 is_epilogue(dis, 0x06, pool), True)
    ok &= _check("`bx lr` is an epilogue", is_epilogue(dis, 0x10, pool), True)
    ok &= _check("a pool word is never an epilogue",
                 is_epilogue(dis, 0x0C, pool), False)
    ok &= _check("`bic` is not an unconditional transfer",
                 bool(UNCOND_RE.match("bic r0, r1")), False)
    ok &= _check("`bne.n` is not an unconditional transfer",
                 bool(UNCOND_RE.match("bne.n 0x20")), False)
    ok &= _check("`bx r5` (a veneer) IS an unconditional transfer",
                 bool(UNCOND_RE.match("bx r5")), True)
    ok &= _check("a leaf entry behind return+pad+pool is detected",
                 entry_at(dis, 0x10, 0x14, pool), (True, True, True))
    ok &= _check("the next leaf entry is detected too",
                 entry_at(dis, 0x14, 0x18, pool), (True, True, True))
    ok &= _check("NEGATIVE: a shared tail reached by fallthrough is NOT an entry",
                 entry_at(dis, 0x1A, 0x1E, pool)[1], False)
    ok &= _check("NEGATIVE: a pool word is NOT an entry",
                 entry_at(dis, 0x0C, 0x10, pool)[0], False)
    ok &= _check("NEGATIVE: a label sitting on padding is NOT an entry",
                 entry_at(dis, 0x12, 0x14, pool)[0], False)
    ok &= _check("a label whose body never returns owns nothing",
                 entry_at(dis, 0x18, 0x1A, pool)[2], False)

    # ---- live regression ----------------------------------------------------
    # The blind spot: `0x08004560` is a bare label + `bx lr` + one pad byte
    # (asm/runtime_2aac.s:3553) with NO `.type` and no `bl`, so neither a
    # `.type`-keyed rule nor the prologue scan sees it, and sub_08004508 is
    # measured 92 bytes instead of 88. This is the only block here that reads
    # the build tree.
    rom_p, obj_p = ROOT / "baserom.gba", CODE_O
    if rom_p.exists() and obj_p.exists():
        ldis = disassemble(rom_p)
        lpool = pool_slots(ldis)
        ok &= _check("REGRESSION: _08004560 passes all three gates",
                     entry_at(ldis, 0x4560, 0x4564, lpool), (True, True, True))
        ok &= _check("REGRESSION: and sub_08004508's return is its predecessor",
                     "0x%08x" % (predecessor(ldis, 0x4560, lpool) + ROM_BASE),
                     "0x08004554")
        # NEGATIVE CONTROL, live: three literal-pool words whose low halfword is
        # 0x4770. asm/code_279c.s:67,76,85 spells all three `.4byte 0x080C4770`.
        # A rule keyed on instruction shape alone promotes every one of them.
        for a, nxt in ((0x2818, 0x282C), (0x282C, 0x2840), (0x2840, 0x2854)):
            ok &= _check(f"NEGATIVE: the 0x080C4770 pool word at 0x{a + ROM_BASE:08X}"
                         " is not an entry", entry_at(ldis, a, nxt, lpool)[0], False)
        # NEGATIVE CONTROL, live: a shared tail. `0x08005B60 negs r0, r0` falls
        # straight into the `bx lr` at 0x08005B62, so that label is interior.
        ok &= _check("NEGATIVE: the shared tail at 0x08005B62 is not an entry",
                     entry_at(ldis, 0x5B62, 0x5B64, lpool)[1], False)
        # NEGATIVE CONTROL, live: `resource_wrap_end` at 0x080263DE is two bytes
        # of alignment padding after a `pop {r1}; bx r1`. G2 PASSES -- the
        # preceding instruction really is a return -- and G1/G3 must still
        # refuse it, because the "function" would own nothing but padding.
        ok &= _check("NEGATIVE: resource_wrap_end (padding only) is not an entry",
                     entry_at(ldis, 0x263DE, 0x263E0, lpool),
                     (False, True, False))
    else:
        print("  SKIP live regression (baserom.gba or build-code/code.o absent)")

    print(f"span_audit self-test: {'PASS' if ok else 'FAIL'}")
    return 0 if ok else 1


def main(argv):
    if "--self-test" in argv:
        return self_test()
    rep = audit()
    if "--json" in argv:
        print(json.dumps(rep, indent=1))
        return 0
    print(f"  {rep['spans_checked']} spans checked, "
          f"{rep['prologues_in_text']} prologues in the text, "
          f"{rep['pool_bytes']} pool bytes")
    print(f"  {len(rep['mis_measured'])} spans swallow an unlabelled entry; "
          f"{rep['unlabelled_entries']} entries in total")
    for e in rep.get("adjudicated_non_entries", []):
        print(f"  (adjudicated non-entry {e['addr']}: {e['why'][:70]}...)")
    for d in rep["mis_measured"][:25]:
        print(f"  {d['vma']} {d['name']:<14} {d['matched_bytes']}/"
              f"{d['rom_bytes']}  {d['source']}")
        if d["missing_labels"]:
            print(f"      no-prologue labels: {', '.join(d['missing_labels'])}")
        if d["shape_labels"]:
            print(f"      shape-only labels: {', '.join(d['shape_labels'])}"
                  f"  (branched to: {d['branched_to']})")
    if len(rep["mis_measured"]) > 25:
        print(f"  ... and {len(rep['mis_measured']) - 25} more")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
