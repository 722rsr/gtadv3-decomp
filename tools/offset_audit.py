#!/usr/bin/env python3
"""offset_audit.py — flag transcribed struct offsets no ROM literal supports.

Why this exists
---------------
Thumb-1 can reach a structure offset in only two ways:

1. a single immediate form (`adds rN,#imm8` / `subs rN,#imm8`, 0..255; the
   `ldr/str` imm5*4 forms top out at 124), or
2. a **literal-pool word** the compiler emitted and then `add`/`ldr`s.

So any offset a *transcribed* C body applies to a base pointer in the
0x100..0xFFFF range must exist as an aligned little-endian u16 inside the
ROM's code region — otherwise the number was invented during transcription.
For example, the player's place byte is not at `wa + 0x2865`:
the ROM sites use the pool literal
`0x000010E5` (`0x03001780 + 0x10E5 = 0x03002865`). `0x000010E5` occurs six
times in the code region; `0x00002865` occurs **zero** times there — and
zero is what this tool reports.

What it accepts as "supported"
------------------------------
For an offset `O` seen in `src/`, the offset is counted as supported when the
code region contains **either**

* the offset-form word `O` (little-endian u16, i.e. `0x0000OOOO`), or
* the absolute VMA-form word `base + O + bit0` for one of the known
  work-area-ish bases (`--bases`, default `0x03001780` = WA, `0x03004E20`
  = race context, `0x03000008`/`0x030000E8` = blocks A/B, `0x03000610`
  = ghost manager, `0x030003E8` = subsystem instances, `0x03001D64` =
  car records, `0x030013D0` = save buffer #2).

That second form matters because a base-relative C offset is often reached in
ROM as an absolute VMA literal (`0x03002865` was present at file `0x6381b9`).

Caveats (documented, not hidden)
--------------------------------
* **Presence is not proof.** A numeric coincidence can "support" an offset;
  only *absence* is evidence. The tool is a **finder**, not an oracle — each
  hit needs an asm read before it is called a bug.
* Offsets that are *computed* (`wa + 0x100A + idx`) still need their
  immediate part (`0x100A`) to be a literal; the tool checks each `+ 0xNNNN`
  term independently, which is what we want.
* **Comment prose is ignored.** The sources carry the asm readings in
  comments (`// u32[rec+0x118] = 5`); a mention is not a reachable offset,
  so `//` and `/* */` text is stripped before matching. Use
  `--pattern` to scan comments deliberately (the `--offset` form bypasses
  the source scan entirely).
* Offsets that are really struct fields of a *different* base, or unit
  scalings, appear here too; filter with `--pattern`/`--exclude`.
* The code region default `0x2E158` is where the converted-code spans end
  (`asm/passthrough.inc`); data-tail coincidences (like the `0x00002865` at
  file `0x48F60`) are excluded on purpose. `--code-end 0` scans the whole ROM.

Usage
-----
    python3 tools/offset_audit.py                       # all of src/
    python3 tools/offset_audit.py --pattern 'wa \\+ 0x'
    python3 tools/offset_audit.py --offset 0x10E5 --offset 0x2865
    python3 tools/offset_audit.py --show-supported
"""

import argparse
import os
import re
import struct
import sys

ROM_END_OF_CODE = 0x2E158
DEFAULT_BASES = (
    0x03001780,  # WA (work area)
    0x03004E20,  # race context pointer target
    0x03000008,  # state block A
    0x030000E8,  # state block B
    0x03000610,  # ghost manager
    0x030003E8,  # subsystem instance array
    0x03001D64,  # car-record base
    0x030013D0,  # save buffer #2
)

# `+ 0x1234` / `+0x1234` after anything. Only the additive form reaches a
# higher structure cell; `- 0x` would be a different (signed) convention.
OFFSET_RE = re.compile(r"\+\s*0x([0-9A-Fa-f]+)\b")


def load_rom(path):
    with open(path, "rb") as fh:
        return fh.read()


def small_literals(rom, region_end):
    """Set of `0x0000xxxx` words in the code region (the offset literals)."""
    blob = rom[:region_end] if region_end else rom
    out = set()
    for i in range(0, len(blob) - 3, 2):
        w = struct.unpack_from("<I", blob, i)[0]
        if w and w <= 0xFFFF:
            out.add(w)
    return out


def word_hits(rom, value, limit=6, region_end=None):
    """File offsets (ROM VMA = off + 0x08000000) of `value` as an aligned u32."""
    blob = rom[:region_end] if region_end else rom
    needle = struct.pack("<I", value & 0xFFFFFFFF)
    out = []
    i = blob.find(needle)
    while i >= 0:
        out.append(i)
        if len(out) >= limit:
            break
        i = blob.find(needle, i + 1)
    return out


def strip_comments(text):
    """Blank out `//` and `/* */` comment text, keeping offsets (for line no.s).

    Comment prose is full of `rec+0x118`-style mentions (they are the docs),
    and a mention is not a reachable offset — the ROM never emits a literal
    for it. Without this the scan is mostly noise.
    """
    out = []
    depth = 0
    i = 0
    while i < len(text):
        two = text[i:i + 2]
        if depth == 0 and two == "//":
            break
        if two == "/*":
            depth += 1
            out.append("  ")
            i += 2
            continue
        if depth and two == "*/":
            depth -= 1
            out.append("  ")
            i += 2
            continue
        out.append(" " if depth else text[i])
        i += 1
    return "".join(out)


def collect_offsets(src_dir, pattern=None, exclude=None):
    """{offset: [(file, line, code)]} for every `+ 0xNNNN` in real code."""
    pat = re.compile(pattern) if pattern else OFFSET_RE
    ex = re.compile(exclude) if exclude else None
    found = {}
    for root, _dirs, files in os.walk(src_dir):
        for name in sorted(files):
            if not name.endswith((".c", ".h")):
                continue
            path = os.path.join(root, name)
            with open(path, "r", errors="replace") as fh:
                in_block = False
                for lineno, line in enumerate(fh, 1):
                    if ex and ex.search(line):
                        continue
                    code = strip_comments(line) if not in_block else ""
                    # `strip_comments` is line-local; track multi-line /* */
                    # blocks so their interior does not count as code either.
                    opens = line.count("/*")
                    closes = line.count("*/")
                    was_block = in_block
                    in_block = (opens > closes) if not in_block else (opens >= closes)
                    if was_block:
                        continue
                    for m in pat.finditer(code):
                        try:
                            off = int(m.group(1), 16)
                        except (IndexError, ValueError):
                            continue
                        if off < 0x100 or off > 0xFFFF:
                            continue
                        site = (path, lineno, code.strip())
                        bucket = found.setdefault(off, [])
                        if site not in bucket:  # one site per line
                            bucket.append(site)
    return found


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("rom", nargs="?", default="baserom.gba")
    ap.add_argument("--src", default="src", help="source directory (default src)")
    ap.add_argument("--code-end", type=lambda s: int(s, 0), default=ROM_END_OF_CODE,
                    help="end of the ROM code region; 0 = whole ROM")
    ap.add_argument("--pattern", help="override regex (must expose group 1 = hex offset)")
    ap.add_argument("--exclude", help="skip lines matching this regex")
    ap.add_argument("--offset", type=lambda s: int(s, 0), action="append",
                    help="report a specific offset (repeatable)")
    ap.add_argument("--bases", type=lambda s: int(s, 0), action="append",
                    help="absolute bases to accept (repeatable)")
    ap.add_argument("--show-supported", action="store_true",
                    help="also list offsets the ROM can produce")
    ap.add_argument("--top", type=int, default=0, help="limit hits listed")
    args = ap.parse_args()

    rom = load_rom(args.rom)
    region_end = args.code_end or None
    bases = args.bases or list(DEFAULT_BASES)

    if args.offset:
        offsets = {o: [(args.src, 0, "(requested)")] for o in args.offset}
    else:
        offsets = collect_offsets(args.src, args.pattern, args.exclude)

    lits = small_literals(rom, region_end)
    strong, weak, supported = [], [], []
    for off, sites in sorted(offsets.items()):
        lit = word_hits(rom, off, region_end=region_end)
        abs_hits = []
        for base in bases:
            abs_hits += word_hits(rom, (base + off) & 0xFFFFFFFF,
                                  region_end=region_end)
            abs_hits += word_hits(rom, ((base + off) | 1) & 0xFFFFFFFF,
                                  region_end=region_end)
        if lit or abs_hits:
            supported.append((off, sites, lit, abs_hits, None))
            continue
        # A nearby literal plus an 8-bit `adds`/`subs`, or a literal base plus
        # an `imm5`-scaled load/store, reaches the cell too: accept `d` up to
        # 255 below. Anything farther from every literal is a strong hit.
        near = None
        for d in range(0, 256):
            if (off - d) in lits:
                near = d
                break
        (weak if near is not None else strong).append(
            (off, sites, lit, abs_hits, near))

    print(f"rom            : {args.rom}")
    print(f"code region    : {'whole ROM' if not region_end else '0x0..%#x' % region_end}")
    print(f"offsets scanned: {len(offsets)} (0x100..0xFFFF, additive)")
    print(f"strong hits    : {len(strong)}   weak (nearby literal): {len(weak)}"
          f"   supported: {len(supported)}")
    print()

    def show(rows, marker):
        rows = rows[:args.top] if args.top else rows
        for off, sites, lit, abs_hits, near in rows:
            note = (f"   nearest literal -{near:#04x}" if near is not None
                    else "   no literal within 0xff")
            print(f"{marker} +{off:#06x}   {len(sites)} site(s)"
                  f"   offset-literal={len(lit)}"
                  f"   abs-literal={len(abs_hits)}{note}")
            for path, lineno, text in sites[:4]:
                loc = f"{path}:{lineno}" if lineno else path
                print(f"      {loc:<52.52}  {text[:78]}")
            if len(sites) > 4:
                print(f"      ... {len(sites) - 4} more")

    show(strong, "!!")
    print()
    print(f"-- weak ({len(weak)}) " + "-" * 40)
    show(weak, "~ ")
    if args.show_supported and supported:
        print()
        print(f"-- supported ({len(supported)}) " + "-" * 34)
        show(supported, "ok")

    if strong:
        print()
        print("STRONG hits have no ROM encoding at all (no direct literal, no")
        print("absolute form, nothing within 0xff) - read the asm before calling")
        print("one a bug: absence is evidence, presence is not proof.")
        return 1
    print()
    print("No strong hits: every scanned offset is producible from a ROM")
    print("literal (directly or with an 8-bit add).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
