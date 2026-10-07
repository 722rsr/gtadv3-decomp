#!/usr/bin/env python3
"""Triage the unpromoted backlog into a ranked, dispatchable work queue.

Discovery only: this tool grants no promotion authority. It reads the corpus
report plus the ROM's own prologue/epilogue bytes and the C source text, and
sorts each unpromoted body by its *likely root cause* and cheapest first move.

Why this exists: `promotion_screen.py` answers "which near-matches may be
staged", and `match_families.py`/`leaf_synth.py` give a user verified recipes.
Neither answers "of the bodies that still need work, which are cheap, which
are the same defect, and which are known-dead". That is the triage layer, and
without it each session re-derives it by hand and re-runs dead searches.

Usage:
    python3 tools/lift_scout.py                 # ranked queue, human table
    python3 tools/lift_scout.py --json build/lift-scout/queue.json
    python3 tools/lift_scout.py --source src/rec35_mid_region.c --top 40
    python3 tools/lift_scout.py --class P1      # only one priority band
    python3 tools/lift_scout.py --self-test
"""
from __future__ import annotations

import argparse
import collections
import json
from pathlib import Path
import re
import sys
import tempfile

ROOT = Path(__file__).resolve().parent.parent
ROM = ROOT / "baserom.gba"
ROM_BASE = 0x08000000
MANIFEST = ROOT / "tools/matching_slice_functions.json"
DEFAULT_REPORT = ROOT / "build/era-corpus/ready-report.json"

# `coverage.asm_vmas()` is the repo's authority on what counts as a function
# entry. Import it rather than re-deriving the rule: coverage.py:229-237
# records that getting the `bl` operand pattern wrong already bit this tree
# once, and a local copy of that regex is one commit away from repeating it.
sys.path.insert(0, str(Path(__file__).resolve().parent))
import coverage  # noqa: E402

# Priority bands. Lower sorts first. P0 is essentially free; P9 is either
# expensive transcription or measured-inexpressible and should not be dispatched
# without a specific reason.
BANDS = {
    "P0-span-gap": 0,
    # Sorts LAST on purpose. A band whose MOVES text opens "Do NOT spend the
    # week here" must not lead the default queue. See the note in
    # asm_entry_kinds: these are diagnostic, not dispatchable.
    "P0-untyped-bl": 98,
    "P1-register-choice": 1,
    "P1-statement-order": 1,
    "P1-epilogue": 1,
    "P2-narrow-param": 2,
    "P3-volatile-load": 3,
    "P4-pool-order": 4,
    "P6-switch-shape": 5,
    "P5-register-alloc": 6,
    "P9-frame-limit": 9,
    "P9-true-lift": 9,
    "P9-unresolved-reloc": 9,
}

# Human next move per class. Kept short: this is what a user reads first.
MOVES = {
    "P0-span-gap":
        "No `.type VMA, %function` and no `bl` target anywhere in asm/, so the "
        "inventory never saw this VMA and gave it a span running to the next "
        "known entry. Add the typed entry and a bare label (both emit zero "
        "bytes), then `make independent-slice && make ownership-map` to refresh "
        "the pinned sha256. Confirm with tools/label_census.py, which resolves "
        "address drift through the assembled object; lift_scout's check is "
        "name-based, so it is a lead, not proof. Sizes cannot substitute: "
        "prefix 0 is evidence AGAINST a gap, not for one.",
    "P0-untyped-bl":
        "This VMA has no `.type` but IS reached by a real `bl`, and "
        "coverage.py's inventory counts BL targets as entries -- so its span is "
        "already measured and adding `.type` changes nothing. Do NOT spend the "
        "comparison here. Check whether the bytes this body seems to 'absorb' belong "
        "to an INTERIOR function with no label at all: that one is invisible to "
        "the inventory and its bytes are charged to its PREDECESSOR's span. "
        "Use tools/label_census.py to find it before touching any C.",
    "P1-epilogue":
        "Compare the ROM tail halfword against the declared return type. A ROM "
        "`02 bc` (pop {{r1}}) with a `void` declaration is fixed by the "
        "declaration, not by a register pin.",
    "P1-register-choice":
        "The candidate already IS the right code: mnemonics, operands, branch "
        "targets and the literal pool all agree, and only the register NUMBERS "
        "differ. This is the cheapest class in the backlog -- measured "
        ", all three unpromoted bodies in it were within 6 bytes of "
        "EXACT. Do NOT re-shape the expression; you will only move the "
        "mismatch. Diff the two listings instruction by instruction and change "
        "the LIVE RANGE that decides which hard register agbcc picks. The one "
        "idiom seen twice: agbcc gives a `volatile` sub-word load and its "
        "widened/doubled value the SAME register, where the ROM splits them "
        "(byte in r1, widened in r0, pool base in r2) -- so keep the loaded "
        "value alive in a named local past the widening.",
    "P1-statement-order":
        "The right instructions in the wrong order. Reorder the source "
        "statements into the ROM's order and the body can fall out exactly: "
        "measured , `_08001F918` (54/68 -> 68/68) needed only the "
        "pool load emitted BEFORE the literal `8`, and two bodies in one "
        "translation unit cleared 5-9 bytes each from argument-forwarding "
        "order alone. agbcc creates incoming-parameter pseudos first, then "
        "locals in DECLARATION order, so declaration order among locals is a "
        "real lever. Do not change a type or a qualifier first.",
    "P2-narrow-param":
        "A sub-word SCALAR parameter appears in the declaration. This band is "
        "a SYNTACTIC correlate, NOT a measured prologue difference: it reads "
        "declaration text and never compares the candidate's prologue against "
        "the ROM's. Measured  on all 11 bodies then in this band, "
        "the ROM carried the lsls/lsrs zero-extend AND SO DID THE CANDIDATE, "
        "so widening the parameter -- the remedy this band used to prescribe "
        "-- would DELETE four correct bytes. Diff the two prologues first. If "
        "the ROM has the extend, this is register allocation or statement "
        "order, not parameter width.",
    "P3-volatile-load":
        "Sub-word load lands in the address register plus a following copy. "
        "Drop `volatile` on the lvalue and re-probe before touching arithmetic.",
    "P4-pool-order":
        "Pool load ordered after the shift pair feeding its address. Try an "
        "assembler-resolved absolute symbol for the base (see "
        "docs/findings/track_car_26180_pool_order.md).",
    "P5-register-alloc":
        "Right size, diverges late: destination-register choice. Reorder the "
        "expression or split into named locals declared in ROM build order.",
    "P9-frame-limit":
        "agbcc emits a push/pop frame the frameless ROM lacks. Often measured "
        "inexpressible; only worth a user after a specific hypothesis.",
    "P9-true-lift":
        "Large size delta with near-zero prefix: genuine transcription, "
        "expensive. Dispatch with a full context packet, not a guess.",
    "P9-unresolved-reloc":
        "Call/pool relocation does not resolve. Ownership or spelling work "
        "before any codegen work.",
    "P6-switch-shape":
        "The C tests one variable against several literals in an if/else-if "
        "chain. agbcc emits that as a LINEAR test; the ROM has a balanced tree "
        "that reuses one `cmp` as both `beq` and `bgt`. Rewrite as `switch (v) "
        "{ case A: ... case B: ... }` over the same cases. Two corollaries worth "
        "trying in the same pass: (1) the compared value's SIGNEDNESS changes the "
        "branch -- `int` gives `bgt`/`ble`, `u32` gives `bhi`/`bls`; (2) "
        "agbcc emits switch blocks in SOURCE order of the case labels, not "
        "case-value order, so reordering the labels moves them to the ROM's "
        "addresses. If the body is not a multiple of 4 bytes, also try a "
        "file-scope `__asm__(\".align 2, 0\")`: gas closes a Thumb code section "
        "with `nop` (0x46c0) where the ROM holds `00 00`.",
}

NARROW = re.compile(r"\b(u8|u16|s8|s16|uint8_t|uint16_t|int8_t|int16_t|"
                    r"unsigned char|unsigned short|short|char)\b")


def narrow_scalar_params(sig: str) -> list[str]:
    """Parameter sub-strings that are SCALAR sub-word values.

    A pointer parameter is word-sized whatever it points at, so `u8 *rec` is
    not a narrow parameter. `NARROW` alone matches the `u8` in it, which
    banded bodies whose parameters are ALL pointers and told a user to widen
    a parameter that is already a word. Measured : 4 of the 11
    bodies then in `P2-narrow-param` were banded on the pointee alone --
    `_08001F918(volatile u8 *rec)`, `_0802D5EC(u8 *voice)`, and two taking
    only word-sized parameters.
    """
    i = sig.find("(")
    j = sig.rfind(")")
    if i == -1 or j <= i:
        return []
    out: list[str] = []
    depth = 0
    cur = ""
    for ch in sig[i + 1:j]:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    out.append(cur)
    return [p for p in out if "*" not in p]


VOLATILE_SUBWORD = re.compile(
    r"\bvolatile\s+\(?\s*\*?\s*(u8|u16|s8|s16|uint8_t|uint16_t|int8_t|int16_t)\b")

# The RIGHT side must be a literal. A bare `if (a == b)` compares two
# variables, which is a pointer/length test, not a dispatch chain -- matching
# it would send a user to rewrite a pointer compare as a `switch`. An
# all-caps macro counts: the tree spells case constants like `PLAYER_RIVAL`.
IFEQ_CHAIN = re.compile(
    r"\bif\s*\(\s*([A-Za-z_]\w*)\s*==\s*"
    r"(?:-?\d+|0[xX][0-9A-Fa-f]+|[A-Z][A-Z0-9_]*)\b")


def ifeq_chain(body: str) -> tuple[str, int] | None:
    """(variable, count) when one variable is `==`-compared 2+ times.

    That is the textual shape of an if/else-if chain, which is what a `switch`
    replaces. Counting DISTINCT variables matters: two unrelated `if (a == 1)`
    and `if (b == 2)` in one body is not a chain.
    """
    if not body:
        return None
    counts: dict[str, int] = {}
    for m in IFEQ_CHAIN.finditer(body):
        counts[m.group(1)] = counts.get(m.group(1), 0) + 1
    for var, n in counts.items():
        if n >= 2:
            return var, n
    return None


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8"))


def read_rom() -> bytes:
    if not ROM.exists():
        raise SystemExit(f"missing reference ROM at {ROM}")
    return ROM.read_bytes()


def halfwords(rom: bytes, vma: int, count: int = 2) -> list[int]:
    """Thumb halfwords at `vma`, little-endian. Empty if out of range."""
    out = []
    for i in range(count):
        off = vma - ROM_BASE + 2 * i
        if 0 <= off + 2 <= len(rom):
            out.append(int.from_bytes(rom[off:off + 2], "little"))
    return out


def rom_shape(rom: bytes, vma: int, size: int) -> dict:
    """The two signal instructions: prologue push mask and epilogue bx target."""
    head = halfwords(rom, vma, 1)
    tail = halfwords(rom, vma + size - 2, 1)
    return {
        "push": head[0] if head else None,
        # 0x4770 = bx lr, 0x4700 = bx r0, 0x4708 = bx r1
        "bx": (tail[0] & 0xFF78) if tail else None,
        "pop": (tail[0] >> 8) if tail else None,
    }


ATTR_GROUP = re.compile(r"__attribute__\s*\(\(.*?\)\)|__extension__|"
                        r"__asm__\s*\(\"[^\"]*\"\)|__asm__\s*\([^)]*\)")


def source_body(src: Path, name: str) -> str:
    """Signature -- RETURN TYPE INCLUDED -- plus body text of `name`, or ''
    when not found textually.

    Brace-matched, not blank-line-delimited: cutting at the next blank line
    runs the slice into whatever follows the body (the alias block, or the next
    function), so an alias declaration and a real body become indistinguishable
    to every source-derived rule downstream.

    Two more text traps, both measured against the live 899-row queue.

    (1) `\bNAME\s*\(` matches the first MENTION of the name, not its
    declaration: 124 of 899 rows hit a `//` comment line or a `HOST_STUB(...)`
    line, and the "signature" came back as the comment's own parenthetical --
    `//   _08001FE24 (0x0801FE24) --- record setup` reads as a signature
    `(_08001FE24 (0x0801FE24)` and the declaration at :178 was never reached.
    (2) Cutting the signature at the NAME dropped the return type. A signature
    is `<return type> NAME(params)`, so starting at NAME leaves
    `returns_void` structurally False for every body in the repo: the P1 band
    could then only ever print "declared value-returning", on all 724 bodies
    whose source really says `void`, and its "declared void, ROM tail bx r1"
    branch was unreachable in production.
    """
    if not src.exists():
        return ""
    text = src.read_text(encoding="utf-8", errors="replace")
    pat = re.compile(r"\b" + re.escape(name) + r"\s*\(")
    m = mbol = None
    for cand in pat.finditer(text):
        bol = text.rfind("\n", 0, cand.start()) + 1
        before = text[bol:cand.start()]
        if before.lstrip().startswith(("//", "*", "/*")):
            continue
        # A `HOST_STUB(...)` wrapper puts the macro name on the SAME line, so
        # only the text in front of the name is evidence. A wider window reaches
        # back into the wrapper line and rejects the real definition below it.
        if re.search(r"\bHOST_STUB\s*\(", before):
            continue
        m, mbol = cand, bol
        break
    if m is None:
        return ""
    # Walk back to the start of the DECLARATION, dropping whatever a previous
    # statement left on the same line. `head` stays a suffix of `raw`, so its
    # length is the offset back to the declaration's first character.
    raw = text[mbol:m.start()]
    head = raw
    for sep in ("}", ";", "{", "#"):
        j = head.rfind(sep)
        if j != -1:
            head = head[j + 1:]
    head = head.lstrip()
    if not head or "//" in head or "/*" in head:
        # A bare call site or a trailing comment: no return type in front of the
        # name. Decline rather than report a property nobody can read.
        return ""
    start = m.start() - len(head)
    i = text.index("(", m.start())
    depth = 0
    sig_end = -1
    for j in range(i, min(len(text), i + 4000)):
        if text[j] == "(":
            depth += 1
        elif text[j] == ")":
            depth -= 1
            if depth == 0:
                sig_end = j
                break
    if sig_end == -1:
        return ""
    sig = text[start:sig_end + 1]
    k = text.find("{", sig_end)
    if k == -1:
        # An alias declaration: a signature with no body. Return it as-is so a
        # caller can tell "no body here" from "body found and empty".
        return sig
    depth = 0
    for n in range(k, min(len(text), k + 8000)):
        if text[n] == "{":
            depth += 1
        elif text[n] == "}":
            depth -= 1
            if depth == 0:
                return sig + " " + text[k:n + 1]
    return sig


def returns_void(sig: str) -> bool:
    """Whether the DECLARATION in `sig` returns `void`.

    Read from the text in front of the parameter list, which `source_body` now
    carries: the return type is the only place `void` can appear there, so a
    `void` PARAMETER cannot produce a false positive. `static`, `extern` and
    `__attribute__((...))` are tolerated, and a whole-signature wrapping macro
    (`HOST_STUB(void f(void));`) is unwrapped first.

    The comparison is on the WHOLE type, not on the word `void`: `void *` is a
    value-returning function, and `\bvoid\b` called it a void return. That
    mis-read is live in this repo -- `void *_08005FE0(void *a, int b)` at
    src/course_resource.c:610 was classified "declared void" against a ROM that
    returns through r1, which is the exact claim the band is supposed to make
    correctly.
    """
    head = ATTR_GROUP.sub(" ", sig)
    m = re.match(r"\s*[A-Z][A-Z0-9_]*\s*\(", head)
    if m:
        depth, end = 0, -1
        for j in range(m.end() - 1, len(head)):
            if head[j] == "(":
                depth += 1
            elif head[j] == ")":
                depth -= 1
                if depth == 0:
                    end = j
                    break
        if end == len(head) - 1:
            head = head[m.end():end]
    k = head.find("(")
    decl = head[:k] if k != -1 else head
    nm = re.search(r"[A-Za-z_]\w*\s*$", decl)      # drop the function name
    if nm:
        decl = decl[:nm.start()]
    decl = re.sub(r"\b(?:static|extern|inline|__inline__)\b", " ", decl)
    return decl.strip() == "void"


def vma_from_name(name: str) -> int | None:
    """VMA embedded in a closure label, or None.

    The 8-digit spelling `_0802BD74` already carries the whole VMA, so it is
    read as-is. The 9-digit corpus spelling `_080025AC0` repeats the `0800`
    prefix, so it is `0x0800` plus the low five hex digits -- which is how the
    tree writes a VMA whose sixth digit is zero.
    """
    m = re.search(r"([0-9A-Fa-f]{8,9})$", name)
    if not m:
        return None
    h = m.group(1).upper()
    if len(h) == 8:
        return int(h, 16)
    if len(h) == 9:
        return 0x08000000 + int(h[4:], 16)
    return None


def asm_entry_kinds() -> tuple[set[int], set[int]]:
    """(typed, bl_reached) VMAs across asm/.

    The ENTRY set is delegated to `coverage.py:asm_vmas()` rather than
    re-derived from a local regex. That module is the repo's authority on what
    counts as a function entry, and its `BL_RE` matches BOTH `bl SYMBOL` and
    the numeric `bl 0x0800XXXX` form. A hand-rolled name-only pattern here
    matched a small minority of real call sites: coverage.py:229-237 records
    that exact mistake already biting this tree once (carphys_tick.s has 220
    bare labels and 2 `.type` lines, so `0x08009FFC` and `0x0800A1A4` were both
    present in asm/ and invisible to the inventory).

    Returns (typed, bl_reached):
      * typed       -- a real `.type NAME, %function` exists for this VMA
      * bl_reached  -- an entry, but ONLY because something `bl`s it

    The BL half is derived as `entries - typed`. `asm_vmas` returns the union
    `typed | called`, and it only records a label actually DEFINED in some
    region, so the subtraction is exact: an entry is either declared
    %function or bl-targeted, and a body may be both.

    CONVERSION TRAP: `asm_vmas` yields SIX-CHAR LOWERCASE HEX STRINGS
    ("01fbac"), not ints -- `coverage.norm_vma` strips the 0x80 prefix and
    zero-fills to six. `int(v, 16)` alone puts every VMA at 0x01FBAC instead of
    0x0801FBAC, so `vma in entries` is False for every body, the split never
    fires, and the band silently reverts to the version this function exists to
    replace. Always `ROM_BASE + int(v, 16)`.

    Which case is which:
      * BL-reached -- already an inventory entry, so its span is measured to
        the next known entry, not guessed. Adding `.type` changes nothing.
        `_08021374` is this case: bare label at asm/carphys_racer.s:90, no
        `.type`, `bl _08021374` at :1218.
      * reached by neither -- the inventory never saw it, so its bytes are
        charged to the PREDECESSOR's span. The predecessor is what the report
        flags and the offender is an interior address with no label at all.
    """
    entries: set[int] = set()
    for vmas in coverage.asm_vmas(str(ROOT / "asm")).values():
        for v in vmas:
            entries.add(ROM_BASE + int(v, 16))

    typed: set[int] = set()
    tpat = re.compile(r"^\s*\.type\s+([A-Za-z_][A-Za-z0-9_]*)\s*,\s*%function", re.M)
    for path in sorted((ROOT / "asm").glob("*.s")):
        for name in tpat.findall(path.read_text(encoding="utf-8", errors="replace")):
            vma = vma_from_name(name)
            if vma is not None:
                typed.add(vma)
    return typed, entries - typed


def signature_of(body: str) -> str:
    """Signature INCLUDING the return type AND the parameter list -- the
    narrow-parameter rule reads the parameters, and cutting at the first `(`
    silently loses them.

    Attribute groups are blanked first. `static __attribute__((unused)) void
    f(void *a)` puts an unbalanced `((` in front of the parameter list, so
    scanning for the first `(` stops inside the attribute and returns
    `static __attribute__((unused))` -- losing the return type AND the
    parameters, which is exactly what `returns_void` and the narrow-parameter
    rule read.
    """
    work = ATTR_GROUP.sub(" ", body)
    i = work.find("(")
    if i == -1:
        return work
    depth = 0
    for j in range(i, min(len(work), i + 2000)):
        if work[j] == "(":
            depth += 1
        elif work[j] == ")":
            depth -= 1
            if depth == 0:
                return work[:j + 1]
    return work[:i]


def _classify_once(rec: dict, shape: dict, body: str, typed: bool = True,
                   reached: bool = False) -> tuple[str, str]:
    """One pass, first match wins. Evidence must be a measured fact, not a guess."""
    rom = rec["rom_bytes"]
    cand = rec["cand_bytes"]
    delta = cand - rom
    prefix = rec.get("prefix") or 0
    ratio = prefix / rom if rom else 0.0
    first_diff = rec.get("first_diff")
    sig = signature_of(body)
    src = rec.get("source") or ""

    # P9 unresolved relocation outranks everything: it is a spelling problem.
    if rec["status"] == "UNRESOLVED_RELOCATION":
        return "P9-unresolved-reloc", f"status UNRESOLVED_RELOCATION, cand {cand}"

    # P0 span gap, decided from asm rather than from sizes. The inventory
    # derives a span as "start to the next known entry", so a VMA with no
    # `.type NAME, %function` of its own gets a span that absorbs its
    # neighbour. Sizes cannot detect this: prefix 0 is evidence AGAINST a gap
    # (an inflated span still matches the victim's own leading bytes), and a
    # size rule firing on prefix==0 catches every large untranscribed function
    # instead. Check the asm directly.
    if not typed:
        # Split the untyped cases: a BL-reached body is ALREADY an inventory
        # entry, so its span is measured and adding `.type` fixes nothing.
        if reached:
            return "P0-untyped-bl", ("no `.type` but reached by `bl`, so the "
                                     "span is already measured; look for an "
                                     "INTERIOR unlabelled function absorbing "
                                     "bytes, not this VMA")
        return "P0-span-gap", ("no `.type` and no `bl` target: invisible to the "
                               "inventory, so its bytes inflate the "
                               "PREDECESSOR's span")



    # NOTE ON PREFIX: do NOT read a low prefix as "not a mechanical defect".
    # The cheap defects sit at or near the HEAD of the body -- a narrow
    # parameter's zero-extend pair occupies bytes 0-4, a volatile sub-word
    # load usually lands early -- so each shifts everything after it and
    # yields a LOW prefix. The race twins closed this repo sat at 8/44 = 0.18,
    # below any threshold worth setting. High prefix identifies only defects
    # at the tail that leave allocation unchanged. So every mechanical band
    # below carries its own prefix in the evidence, and a user confirms or
    # rejects the hypothesis with one probe rather than trusting the label.
    # P9 frame: ROM is frameless (no push) but candidate is far too long and
    # aligned, or the body is a leaf whose candidate still carries a frame.
    if shape.get("push") is not None and (shape["push"] & 0xFF00) != 0xB500:
        if delta > 0 and ratio >= 0.8:
            return "P9-frame-limit", (f"ROM prologue {shape['push']:#06x} has no "
                                     f"push, candidate +{delta} B with "
                                     f"{prefix}/{rom} prefix")

    # P1 epilogue: ROM returns through r1 (a value) but the declaration says
    # void, or the reverse. BOTH sides are read: `returns_void` from the
    # declaration `source_body` returned, `rom_valued` from the ROM's own tail
    # halfword. The declaration side used to be `re.match(r"\s*void\s+", sig)`
    # against a signature `source_body` had cut at the function NAME, so it was
    # False for all 899 queued bodies -- the "declared void" branch below was
    # unreachable and every P1 row could only print "declared value-returning".
    if body:
        void_decl = returns_void(sig)
        rom_valued = shape.get("bx") == 0x4708
        if void_decl and rom_valued:
            return "P1-epilogue", (f"declared void, ROM tail bx r1 "
                                   f"(halfword {shape.get('bx'):#06x}); "
                                   f"prefix {prefix}/{rom}, delta {delta:+d} "
                                   f"-- a return-type flip reserves r0 and can "
                                   f"shift the whole body, so verify before "
                                   f"assuming the tail is the only delta")
        if not void_decl and shape.get("bx") == 0x4700 and delta == 0:
            return "P1-epilogue", (f"declared value-returning, ROM tail bx r0; "
                                   f"prefix {prefix}/{rom}")

    # The report's OWN miss classification is the strongest signal available,
    # because it is produced by comparing the two byte streams rather than by
    # reading the declaration. Both of these classes mean the candidate is
    # already the right code, so the lever is narrow and the body is close.
    # Measured : all 3 unpromoted bodies in
    # "identical mnemonics (register/operand choice)" were within 6 bytes of
    # EXACT, and two of them stopped on the SAME repeatable idiom.
    report_class = rec.get("class") or ""
    if "register/operand choice" in report_class and first_diff not in (None, 0):
        return "P1-register-choice", (
            f"report class `identical mnemonics`: every mnemonic, operand, "
            f"branch target and pool word already agrees and only REGISTER "
            f"NUMBERS differ; {rom - (rec.get('matched_bytes') or 0)} of {rom} "
            f"bytes matched, first difference +{first_diff} -- this is the "
            f"cheapest class in the backlog, do NOT re-shape the expression")
    if report_class == "instruction order" and first_diff not in (None, 0):
        return "P1-statement-order", (
            f"report class `instruction order`: the right instructions in the "
            f"wrong order; {rom - (rec.get('matched_bytes') or 0)} of {rom} "
            f"bytes matched, first difference +{first_diff} -- reorder the "
            f"source statements to the ROM's order before touching types")

    # SIZE GATE for the declaration/qualifier bands. Changing a parameter width
    # or dropping a `volatile` swaps a few instructions and roughly PRESERVES
    # length -- measured on every body this repo closed with such a fix:
    # +4, 0, 0, 0, 0. A body off by 100+ bytes is missing chunks, not a load
    # form, and it diverges at byte ~0 precisely because it is untranscribed.
    # Without this gate the regex fires on any MMIO-heavy body and P3 claimed
    # bodies off by -180 and -128 bytes. P1 is deliberately exempt: a
    # return-type flip reserves r0 and can reshape a body at delta 0.
    length_preserving = abs(delta) <= 8

    # P2 narrow param. WHAT IS MEASURED: the declaration contains a sub-word
    # SCALAR parameter, the size is preserved, and the divergence is not at
    # byte 0. That is all. It is NOT a measured prologue difference -- this
    # branch never reads the ROM's bytes, so the "the ROM lacks the extend"
    # story the band used to tell is unfounded, and its remedy (widen the
    # parameter) deletes the extend when the ROM in fact HAS it. All 11 bodies
    # measured on  carried the extend on both sides. Pointer
    # parameters are excluded: `u8 *p` is a word, and matching its pointee
    # banded 4 of those 11 on nothing.
    narrow_scalar = [p for p in narrow_scalar_params(sig) if NARROW.search(p)]
    if (length_preserving and narrow_scalar
            and first_diff not in (None, 0)):
        shown = "; ".join(p.strip() for p in narrow_scalar)[:60]
        return "P2-narrow-param", (f"sub-word scalar parameter `{shown}`; "
                                   f"prefix {prefix}/{rom}, delta {delta:+d} "
                                   f"-- DECLARATION ONLY, not a measured "
                                   f"prologue difference; diff the ROM and "
                                   f"candidate prologues before changing a "
                                   f"parameter width")

    # P3 volatile SUB-WORD load. The qualifier matters only on a load narrower
    # than a word: that is where agbcc emits a standalone QImode load plus a
    # widen, or pins the load to the address register. A bare `volatile`
    # elsewhere in the body is not this defect, and matching on it put 328
    # bodies / 70 KB in this band.
    if (length_preserving and VOLATILE_SUBWORD.search(body)
            and first_diff not in (None, 0)):
        return "P3-volatile-load", (f"volatile sub-word lvalue in "
                                    f"`{VOLATILE_SUBWORD.search(body).group(0)}`; "
                                    f"prefix {prefix}/{rom}, delta {delta:+d}")

    # P6 switch shape.  showed the dominant lever is
    # STRUCTURE, not qualifiers: an if/else-if chain over one variable emits a
    # linear test, while the ROM has a balanced tree that reuses a single `cmp`
    # as both `beq` and `bgt`. agbcc only produces that tree from a `switch`.
    # Seven of sixteen bodies needed this rewrite, which no existing
    # band could see.
    chain = ifeq_chain(body)
    if chain and first_diff not in (None, 0):
        var, n = chain
        return "P6-switch-shape", (f"`if ({var} == ...)` compared {n}x -- an "
                                   f"if/else-if chain emits a linear test; a "
                                   f"switch over the same cases emits the "
                                   f"ROM's balanced tree; prefix {prefix}/{rom}, "
                                   f"delta {delta:+d}")

    # P4 pool load ordered after the shift pair that feeds its address.
    if "0x0300" in body and first_diff not in (None, 0) and 0 < ratio < 0.9:
        return "P4-pool-order", f"IWRAM literal with prefix {prefix}/{rom}"

    # P5 right-size, diverges late: register/operand choice.
    if delta == 0 and ratio >= 0.5:
        return "P5-register-alloc", f"exact size, {prefix}/{rom} prefix, first_diff +{first_diff}"

    if rec["status"] in ("NO_OVERLAP", "OVERSIZED") or ratio < 0.35:
        return "P9-true-lift", f"prefix {prefix}/{rom}, delta {delta:+d}"

    return "P9-true-lift", f"prefix {prefix}/{rom}, delta {delta:+d}"


def classify(rec: dict, shape: dict, body: str, typed: bool = True,
             reached: bool = False) -> tuple[str, str]:
    """(class, evidence) with any co-occurring signal named in the evidence.

    The rules are first-match-wins, so a body needing two fixes is reported
    under one band only. showed that is the NORMAL case: the structural
    defect coexisted with the qualifier one on every body P3 fired on, and the
    switch rewrite was the load-bearing fix on 2 of 4 wins. Reordering would
    push P6 ahead of P2/P3 and make the qualifier bands nearly unreachable,
    which hides work rather than reporting it -- so the primary class stands and
    the co-occurrence is stated outright. That is also the honest calibration:
    no single band closed a body in .
    """
    cls, why = _classify_once(rec, shape, body, typed=typed, reached=reached)
    if cls in ("P2-narrow-param", "P3-volatile-load", "P4-pool-order",
               "P0-span-gap", "P0-untyped-bl", "P1-epilogue"):
        chain = ifeq_chain(body)
        if chain:
            var, n = chain
            why += (f"; ALSO an if/else-if chain (`if ({var} == ...)` x{n}) -- "
                    f"the switch rewrite was load-bearing on 2 of 4 "
                    f"wins, so expect to need it as well")
    return cls, why


def build(report: dict, manifest: set[str], rom: bytes, typed_vmas: set[int],
          bl_reached: set[int], only_source: str | None,
          band: str | None) -> list[dict]:
    rows = []
    for rec in report["results"]:
        if rec["vma"] in manifest:
            continue
        src_rel = rec.get("source") or ""
        if only_source and only_source not in src_rel:
            continue
        shape = rom_shape(rom, int(rec["vma"], 16), rec["rom_bytes"])
        # The corpus report has NO `c_name` field. Its keys are `name` (the VMA
        # alias, e.g. `_08001E338`) and `alias_of` (the real body, e.g.
        # `Race_Scene_Leaf_E338`). Looking up `name` makes source_body match the
        # one-line `__attribute__((alias(...)))` declaration instead of the body,
        # and every source-derived rule then reads an attribute line. 1371 of
        # 1556 records carry alias_of; the rest genuinely have no body.
        body = source_body(ROOT / src_rel, rec.get("alias_of") or rec["name"])
        vma = int(rec["vma"], 16)
        cls, why = classify(rec, shape, body, typed=vma in typed_vmas,
                           reached=vma in bl_reached)
        if band and not cls.startswith(band):
            continue
        rows.append({
            "name": rec["name"], "vma": rec["vma"], "source": src_rel,
            "rom_bytes": rec["rom_bytes"], "cand_bytes": rec["cand_bytes"],
            "prefix": rec.get("prefix") or 0,
            "delta": rec["cand_bytes"] - rec["rom_bytes"],
            "status": rec["status"], "cls": cls,
            "band": BANDS[cls], "why": why,
            "calls": len(rec.get("call_targets") or []),
            "rom_push": shape.get("push"), "rom_bx": shape.get("bx"),
        })
    # Rank: cheap band first, then biggest prize inside the band, then most
    # aligned (least total work) first.
    rows.sort(key=lambda r: (r["band"], -r["rom_bytes"], -r["prefix"]))
    return rows


def print_table(rows: list[dict], top: int) -> None:
    counts = collections.Counter(r["cls"] for r in rows)
    print(f"unpromoted bodies queued: {len(rows)}")
    for cls, n in sorted(counts.items(), key=lambda kv: BANDS[kv[0]]):
        prize = sum(r["rom_bytes"] for r in rows if r["cls"] == cls)
        print(f"  {cls:22s} {n:4d} bodies  {prize:7d} bytes")
    print()
    shown = rows[:top]
    hdr = f"{'BAND':22s} {'VMA':12s} {'ROM':>5s} {'PFX':>4s} {'+/-':>5s}  NAME"
    print(hdr)
    print("-" * len(hdr))
    for r in shown:
        print(f"{r['cls']:22s} {r['vma']:12s} {r['rom_bytes']:5d} "
              f"{r['prefix']:4d} {r['delta']:+5d}  {r['name']}")
    if rows:
        print()
        first = rows[0]["cls"]
        print(f"next move for the top band ({first}):")
        for line in _wrap(MOVES[first], 88):
            print("  " + line)


def _wrap(text: str, width: int) -> list[str]:
    words, out, cur = text.split(), [], ""
    for w in words:
        if len(cur) + len(w) + 1 > width:
            out.append(cur)
            cur = w
        else:
            cur = (cur + " " + w).strip()
    if cur:
        out.append(cur)
    return out


def self_test() -> int:
    """Pin the classifier on synthetic records. Never reads the live build."""
    checks = []

    def check(name, got, want):
        checks.append((name, got == want, got, want))

    def rec(**kw):
        base = dict(name="X", vma="0x08001000", source="src/x.c", rom_bytes=48,
                    cand_bytes=48, prefix=48, first_diff=None,
                    status="EXACT", call_targets=[])
        base.update(kw)
        return base

    frameless = {"push": 0x1c01, "bx": 0x4770, "pop": 0x00}
    voidr0 = {"push": 0xb530, "bx": 0x4700, "pop": 0x30}
    valued = {"push": 0xb500, "bx": 0x4708, "pop": 0x01}

    # P0 is decided by the absence of a typed asm entry, NOT by sizes. The
    # negative controls below pin the exact confusion that produced a
    # 250-body bogus band: prefix==0 satisfies any "first_diff 0 and size is
    # nowhere near" rule, yet prefix 0 is evidence AGAINST a span gap, because
    # an inflated span still matches the victim's own leading bytes.
    wild = rec(rom_bytes=112, cand_bytes=20, prefix=0, first_diff=0,
               status="NO_OVERLAP")
    check("span-gap-untyped", classify(wild, frameless, "", typed=False)[0],
          "P0-span-gap")
    check("span-gap-typed-is-not-gap",
          classify(wild, frameless, "", typed=True)[0] == "P0-span-gap", False)
    # An UNTYPED entry that is BL-reached is still an inventory entry:
    # coverage.py:asm_vmas() counts typed labels UNION BL targets, so its span
    # is already measured and the old "add the typed entry" advice was a dead
    # end for it. `_08021374` is the real instance (bare label at
    # asm/carphys_racer.s:90, no `.type`, `bl _08021374` at :1218).
    check("bl-reached-untyped-is-not-a-span-gap",
          classify(wild, frameless, "", typed=False, reached=True)[0],
          "P0-untyped-bl")
    check("unreached-untyped-still-a-span-gap",
          classify(wild, frameless, "", typed=False, reached=False)[0],
          "P0-span-gap")
    # IFEQ_CHAIN must need a LITERAL on the right. `if (a == b)` twice is a
    # pair of pointer/length tests, and a user sent to rewrite it as a
    # `switch` would break a correct body.
    check("ifeq-two-variable-compares-is-not-a-chain",
          ifeq_chain("void X(void *a){ int p = 0, q = 0, r = 0;"
                     " if (p == q) r = 1; if (p == r) r = 2; }"), None)
    check("ifeq-literal-compares-is-a-chain",
          ifeq_chain("void X(void *a){ int v = 0;"
                     " if (v == 0) a = 1; else if (v == 2) a = 2; }")[1], 2)
    # Co-occurrence must be visible. First-match-wins would report only P3 on a
    # body that showed needed BOTH, so the evidence has to say so.
    _, why_both = classify(rec(rom_bytes=44, cand_bytes=44, prefix=8,
                               first_diff=8, status="OVERSIZED"), frameless,
                           "void X(void *a){ volatile u16 v = 1;"
                           " if (v == 0) v = 1; else if (v == 1) v = 2; }")
    check("cooccurrence-is-reported", "ALSO" in why_both, True)
    check("prefix-zero-is-not-a-gap",
          classify(rec(rom_bytes=1412, cand_bytes=1136, prefix=0, first_diff=0,
                       status="NO_OVERLAP"), frameless, "", typed=True)[0],
          "P9-true-lift")
    # a promoted-shaped body must never be classified as a gap
    check("exact-is-not-gap",
          classify(rec(prefix=48, first_diff=None), voidr0,
                   "void X(void *a){ }")[0] in
          ("P5-register-alloc", "P9-true-lift"), True)
    # name -> VMA normalisation, including the 9-digit corpus spelling
    check("vma-8-digit", vma_from_name("_0802BD74"), 0x0802BD74)
    check("vma-9-digit", vma_from_name("_080025AC0"), 0x08025AC0)
    check("vma-sub_9-digit", vma_from_name("sub_080025AC0"), 0x08025AC0)
    check("vma-none", vma_from_name("Race_Leaf_E338"), None)
    # declared void but ROM returns a value
    check("epilogue-void-vs-valuable",
          classify(rec(rom_bytes=20, cand_bytes=20, prefix=20, first_diff=8,
                       status="PARTIAL"), valued, "void X(void *a){ }")[0],
          "P1-epilogue")
    # sub-word parameter. The declaration is value-returning, so the ROM tail
    # must be bx r1 -- pairing it with a bx r0 ROM would (correctly) report an
    # epilogue defect and never reach the narrow-parameter rule.
    check("narrow-param",
          classify(rec(rom_bytes=48, cand_bytes=48, prefix=40, first_diff=4,
                       status="PARTIAL"), valued,
                   "u32 X(void *a, u16 b){ return 0; }")[0],
          "P2-narrow-param")
    # A POINTER parameter is word-sized whatever it points at, so `u8 *rec` is
    # not a narrow parameter. Before the scalar filter, NARROW matched the
    # pointee and banded bodies whose parameters are all pointers -- 4 of the
    # 11 measured in this band on  -- telling a user to widen a
    # parameter that is already a word.
    check("pointer-to-narrow-is-not-p2",
          classify(rec(rom_bytes=48, cand_bytes=48, prefix=40, first_diff=4,
                       status="PARTIAL"), valued,
                   "u32 X(volatile u8 *rec){ return 0; }")[0]
          == "P2-narrow-param", False)
    # and the positive control for the filter: a scalar sub-word parameter
    # still bands, so the fix did not disable the rule it was meant to sharpen.
    check("scalar-narrow-still-p2",
          classify(rec(rom_bytes=48, cand_bytes=48, prefix=40, first_diff=4,
                       status="PARTIAL"), valued,
                   "u32 X(volatile u8 *rec, u16 v){ return 0; }")[0],
          "P2-narrow-param")
    check("narrow-scalar-params-parsing",
          [p.strip() for p in narrow_scalar_params("void f(a, u8 *b, u16 c)")],
          ["a", "u16 c"])
    # The two measured-class bands. These are keyed on the report's own
    # comparison of the two byte streams, not on the declaration, so they must
    # outrank the syntactic bands: a body whose only defect is a register
    # number must not be sent to a user told to widen a parameter.
    # `frameless`, not `valued`: a `valued` ROM tail with a `void` declaration
    # is claimed by P1-epilogue, which is checked first and is the more precise
    # rule. These cases must isolate the measured-class bands.
    check("report-class-register-choice",
          classify(rec(rom_bytes=28, cand_bytes=28, prefix=24, first_diff=2,
                       status="PARTIAL", matched_bytes=26,
                       **{"class": "identical mnemonics "
                                   "(register/operand choice)"}),
                   frameless, "void X(volatile u8 *rec){ return 0; }")[0],
          "P1-register-choice")
    check("report-class-statement-order",
          classify(rec(rom_bytes=68, cand_bytes=68, prefix=60, first_diff=4,
                       status="PARTIAL", matched_bytes=54,
                       **{"class": "instruction order"}),
                   frameless, "void X(volatile u8 *rec){ return 0; }")[0],
          "P1-statement-order")
    # Negative control: the SAME declaration with no report class must NOT
    # claim a measured band, so these cannot fire on a report that simply
    # lacks the field. It falls through to P3, which is a syntactic correlate
    # and says so.
    check("report-class-absent-not-measured-band",
          classify(rec(rom_bytes=28, cand_bytes=28, prefix=24, first_diff=2,
                       status="PARTIAL", matched_bytes=26),
                   frameless, "void X(volatile u8 *rec){ return 0; }")[0],
          "P3-volatile-load")
    # and a genuinely unclassified near-miss still lands where it used to.
    check("report-class-absent-still-routes",
          classify(rec(rom_bytes=336, cand_bytes=156, prefix=2, first_diff=2,
                       status="OVERSIZED", matched_bytes=2),
                   frameless,
                   "void X(void *a){ volatile u16 v = 1; v++; }")[0],
          "P9-true-lift")
    # the mirror case: value-returning declaration against a void ROM tail.
    # Pins that the epilogue rule and the narrow-parameter rule do not overlap.
    check("epilogue-value-vs-void",
          classify(rec(rom_bytes=48, cand_bytes=48, prefix=40, first_diff=4,
                       status="PARTIAL"), voidr0,
                   "u32 X(void *a, u16 b){ return 0; }")[0],
          "P1-epilogue")
    # volatile body
    check("volatile-load",
          classify(rec(rom_bytes=44, cand_bytes=44, prefix=8, first_diff=8,
                       status="OVERSIZED"), frameless,
                   "void X(void *a){ volatile u16 v = 1; v++; }")[0],
          "P3-volatile-load")
    # Positive control for the size gate: the race twins closed at delta +4
    # (44 ROM -> 48 candidate), which must still classify as the volatile fix.
    check("volatile-length-preserving",
          classify(rec(rom_bytes=44, cand_bytes=48, prefix=8, first_diff=8,
                       status="OVERSIZED"), frameless,
                   "void X(void *a){ volatile u16 v = 1; v++; }")[0],
          "P3-volatile-load")
    # Negative control for the SIZE GATE: a qualifier change preserves length,
    # so a volatile sub-word body off by 180 bytes is missing chunks, not a
    # load form. Without this the band claimed _0800E650 (336 B, pfx 2,
    # delta -180) and _080020518 (376 B, delta -128).
    check("volatile-large-delta-not-p3",
          classify(rec(rom_bytes=336, cand_bytes=156, prefix=2, first_diff=2,
                       status="OVERSIZED"), frameless,
                   "void X(void *a){ volatile u16 v = 1; v++; }")[0],
          "P9-true-lift")
    check("narrow-large-delta-not-p2",
          classify(rec(rom_bytes=212, cand_bytes=200, prefix=2, first_diff=2,
                       status="PARTIAL"), frameless,
                   "u32 X(void *a, u16 b){ return 0; }")[0],
          "P9-true-lift")
    # P1 is exempt from the gate: a return-type flip can reshape a delta-0 body
    # by reserving r0, which is _0800DC7C's whole situation.
    check("epilogue-exempt-from-size-gate",
          classify(rec(rom_bytes=164, cand_bytes=164, prefix=2, first_diff=2,
                       status="PARTIAL"), valued,
                   "void X(void *a){ }")[0],
          "P1-epilogue")
    # P6 switch shape: the dominant lever, which no band could see.
    check("switch-chain-detected",
          classify(rec(rom_bytes=112, cand_bytes=112, prefix=22, first_diff=22,
                       status="PARTIAL"), frameless,
                   "void X(void *a){ int v = 0; if (v == 0) { a = 1; }"
                   " else if (v == 1) { a = 2; } }")[0],
          "P6-switch-shape")
    # Negative control: two DIFFERENT variables compared once each is not a
    # chain, and must not be reported as one.
    check("two-vars-not-a-chain",
          classify(rec(rom_bytes=112, cand_bytes=112, prefix=22, first_diff=22,
                       status="PARTIAL"), frameless,
                   "void X(void *a){ int p = 0, q = 0;"
                   " if (p == 0) a = 1; if (q == 2) a = 2; }")[0],
          "P9-true-lift")
    # Negative control: a single `==` is not a chain either.
    check("single-eq-not-a-chain",
          ifeq_chain("void X(void *a){ if (v == 0) a = 1; }"), None)
    # Negative control for the tightened P3: volatile on a WORD lvalue is not
    # this defect. Matching on a bare `volatile` put 328 bodies / 70 KB here.
    check("volatile-word-is-not-subword",
          classify(rec(rom_bytes=44, cand_bytes=44, prefix=8, first_diff=8,
                       status="OVERSIZED"), frameless,
                   "void X(void *a){ volatile u32 v = 1; v++; }")[0],
          "P9-true-lift")
    # A pointer to a volatile sub-word is the SAME defect: the dereference is a
    # volatile sub-word lvalue, and that is the shape the race twins had
    # (`*(volatile u16 *)((volatile u8 *)a + 182)`). Positive control.
    check("volatile-pointer-is-subword",
          classify(rec(rom_bytes=44, cand_bytes=44, prefix=8, first_diff=8,
                       status="OVERSIZED"), frameless,
                   "void X(void *a){ volatile u8 *c = 0; c++; }")[0],
          "P3-volatile-load")
    # unresolved relocation wins over everything
    check("unresolved-wins",
          classify(rec(status="UNRESOLVED_RELOCATION"), valued,
                   "void X(void *a){ }")[0],
          "P9-unresolved-reloc")
    # frameless ROM plus aligned oversized candidate is the frame signature
    check("frame-limit",
          classify(rec(rom_bytes=20, cand_bytes=24, prefix=20, first_diff=2,
                       status="OVERSIZED"), frameless,
                   "void X(void *a){ volatile u16 v = 1; v++; }")[0],
          "P9-frame-limit")
    # band ordering: P0 sorts before P9
    check("band-order", BANDS["P0-span-gap"] < BANDS["P9-true-lift"], True)

    # source_body must distinguish the real body from its VMA alias. The corpus
    # report's `name` is the alias, so looking that up matches the one-line
    # attribute declaration and every source-derived rule then reads an
    # attribute line instead of a body. Pin both directions on a real file.
    with tempfile.TemporaryDirectory() as td:
        sample = Path(td) / "sample.c"
        sample.write_text(
            "void Race_Leaf_E338(void *a) {\n"
            "    volatile u16 v = *(volatile u16 *)a;\n"
            "    if (v != 1) return;\n"
            "}\n"
            "#ifndef __APPLE__\n"
            "void _08001E338(void *a) __attribute__((alias(\"Race_Leaf_E338\")));\n"
            "#endif\n", encoding="utf-8")
        body_txt = source_body(sample, "Race_Leaf_E338")
        alias_txt = source_body(sample, "_08001E338")
        check("source_body-finds-body", "volatile u16 v" in body_txt, True)
        check("source_body-body-has-no-attribute",
              "__attribute__" in body_txt, False)
        # The fixture deliberately has NO blank line between the body's `}` and
        # the alias block, so this proves brace-matching rather than luck.
        check("source_body-alias-has-no-body",
              "volatile u16 v" in alias_txt, False)
        # and the two must classify differently, which is the bug's whole cost
        r_vol = rec(rom_bytes=44, cand_bytes=44, prefix=8, first_diff=8,
                    status="OVERSIZED")
        check("alias-lookup-would-miss-volatile",
              classify(r_vol, frameless, alias_txt)[0], "P9-true-lift")
        check("body-lookup-finds-volatile",
              classify(r_vol, frameless, body_txt)[0], "P3-volatile-load")

    # ---- NEGATIVE CONTROL: the P1 "declared value-returning" evidence ----
    # gave two P1 bodies to a user; both were already `void` in source
    # and the candidate already emitted the void-form epilogue. `source_body`
    # cut the signature at the function NAME, so `sig` never carried a return
    # type and the `declared void` branch was structurally unreachable: across
    # all 899 queued bodies the band printed "declared value-returning" on the
    # 724 whose source really says void. The fixture reproduces all three
    # text traps: a comment that MENTIONS the name with its own parenthetical,
    # a HOST_STUB wrapper, and a real `void` definition below both.
    with tempfile.TemporaryDirectory() as td:
        p1 = Path(td) / "p1.c"
        p1.write_text(
            "//   _0800C814 (VMA 0x0800C814-0x0800C8A0) -- prose, not a decl\n"
            "HOST_STUB(void _0800C814(void *a));\n"
            "void _0800C814(void *a) {\n"
            "    *(volatile u32 *)a = 4;\n"
            "}\n"
            "int _0800C880(void *a) {\n"
            "    return *(int *)a;\n"
            "}\n"
            "static __attribute__((unused)) void _0800C8AC(u8 sel) {\n"
            "    *(volatile u8 *)0 = sel;\n"
            "}\n"
            "int _0800C8D8(void *a, void *b) {\n"
            "    *(volatile u32 *)a = 1;\n"
            "    *(volatile u32 *)b = 2;\n"
            "    return 0;\n"
            "}\n"
            "void *_0800C8F0(void *a, int b) {\n"
            "    return (void *)((u8 *)a + b);\n"
            "}\n", encoding="utf-8")
        void_body = source_body(p1, "_0800C814")
        check("source_body-keeps-return-type", "void _0800C814(" in void_body, True)
        check("source_body-skips-comment-and-host-stub",
              "*(volatile u32 *)a = 4;" in void_body
              and "VMA 0x0800C814" not in void_body
              and "HOST_STUB" not in void_body, True)
        r_p1 = rec(rom_bytes=48, cand_bytes=48, prefix=8, first_diff=8,
                   status="PARTIAL")
        # the defect itself: a ROM that returns through r1 against a source that
        # says void must be REPORTED as exactly that.
        check("epilogue-void-declared-in-source",
              "declared void" in classify(r_p1, valued, void_body)[1], True)
        # and the mirror must stay honest in the other direction.
        int_body = source_body(p1, "_0800C880")
        check("epilogue-value-not-called-void",
              "declared void" in classify(r_p1, valued, int_body)[1], False)
        check("epilogue-value-declared-value-returning",
              "declared value-returning" in
              classify(r_p1, voidr0, int_body)[1], True)
        # a leading attribute group must not eat the parameter list -- it sits
        # in front of the return type and is full of unbalanced parens.
        attr_body = source_body(p1, "_0800C8AC")
        check("signature_of-attribute-keeps-params",
              "u8 sel" in signature_of(attr_body), True)
        check("epilogue-attributed-void-declared",
              "declared void" in classify(r_p1, valued, attr_body)[1], True)
        # a `void` PARAMETER is not a `void` RETURN: the reverse direction.
        two_void = source_body(p1, "_0800C8D8")
        check("epilogue-void-parameter-is-not-void-return",
              "declared void" in classify(r_p1, valued, two_void)[1], False)
        # `void *` is a VALUE return. The old `\bvoid\b` test called it void and
        # claimed "declared void" for `_08005FE0` (src/course_resource.c:610).
        ptr_body = source_body(p1, "_0800C8F0")
        check("epilogue-void-pointer-is-not-void-return",
              "declared void" in classify(r_p1, valued, ptr_body)[1], False)

    ok = sum(1 for _, good, _, _ in checks if good)
    for name, good, got, want in checks:
        if not good:
            print(f"FAIL {name}: got {got!r} want {want!r}")
    print(f"lift_scout self-test: {ok}/{len(checks)}")
    return 0 if ok == len(checks) else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    ap.add_argument("--json", type=Path)
    ap.add_argument("--source", help="substring filter on the C source path")
    ap.add_argument("--class", dest="band", help="only this priority band, e.g. P1")
    ap.add_argument("--top", type=int, default=30)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        return self_test()

    report = load_json(args.report)
    manifest = set(load_json(MANIFEST))
    rom = read_rom()
    typed_vmas, bl_reached = asm_entry_kinds()
    rows = build(report, manifest, rom, typed_vmas, bl_reached, args.source,
                 args.band)
    print_table(rows, args.top)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps({
            "source": "build/era-corpus report + baserom prologue/epilogue bytes",
            "total": len(rows),
            "bands": dict(collections.Counter(r["cls"] for r in rows)),
            "bytes_by_band": {
                cls: sum(r["rom_bytes"] for r in rows if r["cls"] == cls)
                for cls in sorted({r["cls"] for r in rows})},
            "queue": rows,
        }, indent=2) + "\n", encoding="utf-8")
        print(f"\nJSON: {args.json}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
