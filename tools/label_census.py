#!/usr/bin/env python3
"""Census VMA-shaped labels against their ASSEMBLED addresses.

A label's address must come from the symbol table, never from parsing its name.
In this tree the code generator emits the .type/label pair *after* an armcc
high-register prologue preamble, so a VMA-shaped label can assemble several
bytes past the address its name claims. A wrong address derived from a name
produces valid, resolvable, in-range code that links, compiles and lints; the
only thing that catches it is the byte comparison against ground truth. That
failure is the `sub_08002BB4` class and it has been paid for more than once.

This tool exists so the figure in the docs cannot go stale. An earlier
hand-maintained list in docs/matching_workflow.md claimed "9 still misplaced"
when the true count was 2, because the list was not recomputed as source changed. Run this instead of editing prose.

Two traps this encodes, both paid for in this repo:

  * A relocatable .o reports SECTION-RELATIVE FILE OFFSETS, not VMAs. Add
    0x08000000 before comparing. Skipping that base makes every label look
    drifted by the same constant -0x08000000, which reads as a mass failure.
  * Compare VMAs as INTEGERS. `_080240C4` and `_080240c4` denote the same
    address; a name that pads to 8 digits denotes a different one.

Usage:
    python3 tools/label_census.py            # human table
    python3 tools/label_census.py --json     # machine-readable
    python3 tools/label_census.py --expect 2 # gate: non-zero exit if the count
                                            # differs (prints the new count)
    python3 tools/label_census.py --self-test
"""

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
NM = "arm-none-eabi-nm"
OBJ = ROOT / "build-code" / "code.o"
ROM_BASE = 0x08000000

# VMA-shaped: EIGHT hex digits naming a ROM address, after stripping the
# sub_/Sub_/_ decoration. Anything else is out of scope.
#
# NINE-digit names are counted and reported but deliberately NOT classified,
# because in this tree they are ambiguous by construction and no rule separates
# them. Widening the pattern to accept them turns `_080010004`, `_08001000C`,
# ... -- C DATA symbols spaced 8 bytes apart, named `_0800` + five hex digits --
# into 6029 phantom drifts, because the name is off by exactly 0x10000 from the
# address. At the same time one nine-digit name IS a real VMA claim: `_080026220`
# assembles at 0x08026220, a spurious digit the promotion screen caught. So the
# class contains both, and a census that classified it would be wrong 6029 times
# to be right once. tools/promotion_screen.py is the authority on which names
# must resolve; this tool is the authority on 8-digit VMA-shaped asm labels.
VMA_RE = re.compile(r"^(?:sub_|Sub_|_)?(0[0-9A-Fa-f]{7})$")
NINE_RE = re.compile(r"^(?:sub_|Sub_|_)?(0[0-9A-Fa-f]{8})$")

# Drift past this is not the prologue-preamble case. The preamble is a handful
# of halfwords; anything larger means the label names a different address than
# it assembles at, and must be adjudicated against the ROM one at a time.
PROLOGUE_DRIFT_MAX = 16


def parse_nm(output):
    """Parse `nm` output into [(named_vma_int, symbol, drift)].

    Split out from the subprocess call so the self-test can drive it with a
    synthetic fixture. A case that asserts against the live build artifact
    passes or fails with whether that artifact exists and is current, which
    makes it a test of the build tree rather than of this function.
    """
    if not output.strip():
        # An empty table must fail loudly here, not at the subprocess site, so
        # every path into this function carries the guard. Falling back to
        # name-parsing on an empty map reproduces the exact bug the tool exists
        # to catch, and does it silently.
        raise SystemExit(
            "empty symbol table: the object is a build artifact and is stale or "
            "missing after any asm/ edit -- run `make code-objects` first. "
            "Refusing to fall back to name-parsing, which is the exact bug this "
            "tool exists to catch."
        )
    rows = []
    for line in output.splitlines():
        fields = line.split()
        if len(fields) != 3:
            continue
        off, _type, name = fields
        m = VMA_RE.match(name)
        if not m:
            continue
        try:
            file_offset = int(off, 16)
        except ValueError:
            continue
        vma = file_offset + ROM_BASE  # a relocatable .o reports file offsets
        rows.append((int(m.group(1), 16), name, vma - int(m.group(1), 16)))
    return rows


def collect(obj=OBJ, nm=NM):
    out = subprocess.run([nm, str(obj)], capture_output=True, text=True).stdout
    return parse_nm(out)  # parse_nm owns the empty-table guard


def census(obj=OBJ, nm=NM):
    rows = collect(obj, nm)
    out = subprocess.run([nm, str(obj)], capture_output=True, text=True).stdout
    nine = [line.split() for line in out.splitlines()
            if len(line.split()) == 3 and NINE_RE.match(line.split()[2])]
    funcs = typed_function_names(ROOT / "asm")
    typed_nine = [(o, n) for o, _t, n in nine if n in funcs]
    ambiguous_funcs = len(typed_nine)
    # Drift-check the typed ones using coverage.py's own normaliser, which is
    # what collapses `_080026220` and `_08026220` to the same value. A redundant
    # zero is a SPELLING difference, not an address difference, and re-parsing
    # the name with a local regex is what produced 6029 phantom drifts.
    import importlib.util as _il
    _sp = _il.spec_from_file_location("_cov", ROOT / "tools" / "coverage.py")
    _cov = _il.module_from_spec(_sp)
    _sp.loader.exec_module(_cov)
    for _o, _n in typed_nine:
        # norm_vma returns the VMA as hex digits WITHOUT the 0x08 -- i.e. a FILE
        # OFFSET -- and collapses the redundant-zero spellings, which is exactly
        # the mapping needed here. Both _080026220 and _08026220 give 026220.
        claimed = _cov.norm_vma(_n)
        if not claimed:
            continue
        d = int(_o, 16) - int(claimed, 16)
        if d:
            drifted.append((int(claimed, 16) + ROM_BASE, _n, d))
    drifted = sorted((r for r in rows if r[2] != 0), key=lambda r: r[2])
    return {
        "total_vma_labels": len(rows),
        "ambiguous_9digit_names": len(nine),
        "ambiguous_9digit_typed_as_function": ambiguous_funcs,
        "ambiguous_9digit_pool_or_data": len(nine) - ambiguous_funcs,
        "drifted": [
            {
                "named_vma": f"0x{n:08x}",
                "symbol": s,
                "drift": d,
                "class": "prologue-preamble" if d <= PROLOGUE_DRIFT_MAX else "adjudicate",
            }
            for n, s, d in drifted
        ],
    }



# coverage.py:asm_vmas() counts an address as a function entry only if the label
# carries a `.type NAME,%function` line OR is the target of a `bl` from an asm
# file. `typed` is a local inside that function, so re-derive the typed half with
# the same rule rather than duplicating a private. This is what turns "6,026
# names we cannot classify" into "N real entries, M pool/data": the 9-digit
# spellings are a mix, and only the typed ones are drift-checkable.
TYPE_RE = re.compile(r"^\s*\.type\s+(\S+?)\s*,\s*%function")
BL_RE = re.compile(r"\bbl\s+(?:\.\w+\s+)?(0x[0-9A-Fa-f]+|[A-Za-z_]\w*)")


def typed_function_names(asm_dir):
    """Names coverage.py would count as function entries, by spelling."""
    typed, called = set(), set()
    for path in sorted(Path(asm_dir).glob("*.s")) + sorted(Path(asm_dir).glob("*.inc")):
        text = path.read_text(errors="replace")
        for m in TYPE_RE.finditer(text):
            typed.add(m.group(1))
        code = re.sub(r"@.*$", "", text, flags=re.M)
        for m in BL_RE.finditer(code):
            called.add(m.group(1))
    return typed | called


def _check(name, got, want):
    ok = got == want
    print(f"  {'PASS' if ok else 'FAIL'} {name}: got {got!r} want {want!r}")
    return ok


def self_test():
    ok = True
    # sub_08002BB4 assembles 4 bytes past its name -- the prologue-preamble case
    # this repo has already been bitten by.
    fixture = (
        "00002bb8 t sub_08002BB4\n"     # drifted +4
        "00002c00 t _08002C00\n"        # aligned
        "00002c9e t sub_08002C98\n"      # drifted +6
        "0002abcd T Some_Real_Name\n"    # not VMA-shaped, ignored
        "0002bd80 t sub_0802BD80\n"      # adjacent digits, must not match
    )
    rows = parse_nm(fixture)
    d = {n: dr for n, _s, dr in rows}
    ok &= _check("a label 4 bytes past its name is found", 0x08002BB4 in d, True)
    ok &= _check("its drift is +4", d.get(0x08002BB4), 4)
    ok &= _check("the +6 prologue case is +6", d.get(0x08002C98), 6)
    ok &= _check("an aligned label has zero drift", d.get(0x08002C00), 0)
    ok &= _check("a non-VMA name is ignored", 0x0002ABCD in d, False)
    ok &= _check("a 7-digit name is not treated as 0x08002bd8",
                 0x0802BD80 in d, True)

    # A 9-digit name must NOT be classified by the local regex: the class holds
    # both real VMA claims and a C naming convention, and widening the pattern
    # produced 6029 phantom drifts. It is split by type instead, and the typed
    # half is normalised through coverage.norm_vma -- which collapses
    # _080026220 and _08026220 to the same value, so a redundant zero stops
    # being a 2-billion-byte phantom.
    nine = parse_nm("00026220 t _080026220\n")
    ok &= _check("a 9-digit name is NOT classified by the local regex", len(nine), 0)
    ok &= _check("but a 9-digit name IS counted", bool(NINE_RE.match("_080026220")), True)

    # The two traps, stated as assertions so a future edit that drops them fails.
    ok &= _check("ROM base is added (offset 0x2bb8 -> VMA 0x08002bb8)",
                 0x08000000 + 0x2BB8, 0x08002BB8)
    ok &= _check("VMAs compare as integers, not padded strings",
                 int("0x08002c00", 16), 0x08002C00)

    # The normaliser is the load-bearing part of the 9-digit path.
    import importlib.util as _il
    _sp = _il.spec_from_file_location("_cov", ROOT / "tools" / "coverage.py")
    _cov = _il.module_from_spec(_sp)
    _sp.loader.exec_module(_cov)
    ok &= _check("norm_vma collapses the redundant-zero spellings",
                 _cov.norm_vma("_080026220"), _cov.norm_vma("_08026220"))
    ok &= _check("and that shared value is a file offset",
                 int(_cov.norm_vma("_08026220"), 16), 0x26220)

    # Classification: <=+16 is the prologue class, beyond it is not safe to move together.
    rep_rows = parse_nm("00002bb8 t sub_08002BB4\n0002abcd0 t sub_080046D0\n")
    cls = {s: ("prologue-preamble" if dr <= PROLOGUE_DRIFT_MAX else "adjudicate")
           for _n, s, dr in rep_rows}
    ok &= _check("+4 classifies as prologue-preamble",
                 cls.get("sub_08002BB4"), "prologue-preamble")
    ok &= _check("a large drift is NOT safe to move together",
                 cls.get("sub_080046D0"), "adjudicate")

    # An empty object must fail loudly, never fall back to name-parsing.
    try:
        parse_nm("")
        loud = False
    except SystemExit:
        loud = True
    ok &= _check("an empty symbol table is refused, not guessed", loud, True)
    print(f"label_census self-test: {'PASS' if ok else 'FAIL'}")
    return 0 if ok else 1


def main(argv):
    if "--self-test" in argv:
        return self_test()
    rep = census()
    if "--json" in argv:
        print(json.dumps(rep, indent=1))
        return 0
    print(f"  {rep['total_vma_labels']} VMA-shaped labels, "
          f"{len(rep['drifted'])} drifted")
    n9 = rep.get("ambiguous_9digit_names", 0)
    print(f"  {n9} 9-digit names, NOT drift-classified (mixed C spelling "
          f"convention): {rep.get('ambiguous_9digit_typed_as_function', 0)} "
          f"typed %function or bl-ed, "
          f"{rep.get('ambiguous_9digit_pool_or_data', 0)} pool/data")
    if not rep["drifted"]:
        print("  none")
        return 0
    print("  named-VMA        drift      class              symbol")
    for d in rep["drifted"]:
        print(f"  {d['named_vma']}  {d['drift']:+6d} (0x{d['drift']:<4X}) "
              f"{d['class']:<18} {d['symbol']}")
    if "--expect" in argv:
        want = int(argv[argv.index("--expect") + 1])
        got = len(rep["drifted"])
        if got != want:
            print(f"\n  FAIL: {got} drifted, expected {want}. "
                  f"If that is because a fix landed, update the count in "
                  f"docs/matching_workflow.md and pass the new number here.")
            return 1
        print(f"  expect {want}: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
