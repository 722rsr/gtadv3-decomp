#!/usr/bin/env python3
"""Prove a matched C function can replace source ASM in the independent code link.

This builds a temporary, source-only code slice. One original function body is
removed from a copied assembly input and replaced by agbcc assembly compiled
from the maintained C source. The complete executable region is then linked
at its real address and compared with baserom. The temporary tree lives under
ignored build/ and never consumes original executable bytes as source.
"""
from __future__ import annotations

import argparse
import collections
import contextlib
import io
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import corpus_match_probe as probe

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "tools/matching_slice_functions.json"

# A real `.incbin` directive, not the word in prose: several converted files
# document in a comment that their hole has no incbin left. Same pattern as
# independent_slice.INCBIN_RE.
INCBIN_RE = re.compile(r"^\s*\.incbin\b", re.M)


def command(argv: list[str | Path]) -> None:
    proc = subprocess.run([str(arg) for arg in argv], cwd=ROOT, capture_output=True, text=True)
    if proc.returncode:
        diagnostics = (proc.stdout + proc.stderr).splitlines()
        shown = diagnostics[:20]
        if len(diagnostics) > 20:
            shown.append(f"... {len(diagnostics) - 20} additional diagnostic lines omitted")
        raise RuntimeError(f"{' '.join(map(str, argv))}\n" + "\n".join(shown))


def asm_section(source: Path, names: list[str], owned: list[str] | None = None,
                entry_vma: int | None = None) -> str:
    """One agbcc function section, bounded to the function it declares.

    agbcc does not give every alias its own section: after a function's
    `.size`, the `.globl`/`.thumb_set` pairs for *every other alias of the same
    body* are emitted into whichever section is still open. Splicing the raw
    section therefore redefines VMAs this link does not own -- and points one
    at a twin ROM address. Keep the body plus the alias spellings it owns.
    `names` selects the section and names the body; `owned` widens
    the alias spellings the splice may re-export (the manifest's declared owner
    spelling, which the scorer's chosen `name` may not be), so a later promoted
    body can call this VMA under the spelling it was written with.
    """
    lines = source.read_text(encoding="utf-8").splitlines(keepends=True)
    for name in names:
        section_re = re.compile(r"^\s*\.section\s+\.text\." + re.escape(name) + r"(?:,|\s|$)")
        starts = [i for i, line in enumerate(lines) if section_re.match(line)]
        if len(starts) != 1:
            continue
        start = starts[0] + 1
        end = next((i for i in range(start, len(lines)) if re.match(r"^\s*\.section\s+", lines[i])), len(lines))
        body = lines[start:end]
        # Function-section alignment is an object-placement directive. In the
        # monolithic link the replacement starts at an already pinned VMA.
        while body and (body[0].lstrip().startswith(".align") or not body[0].strip()):
            body.pop(0)
        wanted = owned or names
        for i, line in enumerate(body):
            if re.match(r"^\s*\.size\s+" + re.escape(name) + r"\s*,", line):
                text = "".join(body[:i + 1] + _own_alias_glue(body[i + 1:], name, wanted))
                return _export_missing(text, lines, name, wanted, source, entry_vma)
        return _export_missing("".join(body), lines, name, wanted, source, entry_vma)
    raise RuntimeError(f"no unique agbcc function section for {names} in {source}")


def _assert_owns_only(text: str, body: str, names: list[str], source: Path) -> None:
    """A splice may define its body and its own VMA spelling, nothing else.

    An over-wide section silently redefines other ROM addresses; the assembler
    only catches that when the address still has an assembly label, so a
    collision against a *C-only* name would link and quietly change meaning.
    """
    defined = {m.group(1) for m in re.finditer(r"^\s*([A-Za-z_][A-Za-z_0-9]*):", text, re.M)}
    defined |= {m.group(1) for m in re.finditer(r"^\s*\.globl\s+([A-Za-z_][A-Za-z_0-9]*)", text, re.M)}
    defined |= {m.group(1) for m in re.finditer(r"^\s*\.thumb_set\s+([A-Za-z_][A-Za-z_0-9]*)", text, re.M)}
    # A name assigned to an absolute expression -- `.globl X` then
    # `X = 0x080CB68C` -- emits NO bytes in this section. It is a pure address
    # declaration, so it cannot be the over-wide-section hazard this check
    # exists to catch, and it is not something the manifest can declare either:
    # the screen builds `export` from closure *branch* targets, and this is a
    # data symbol the C must emit so a `SYMBOL_REF` leaf survives instead of a
    # folded integer (see docs/matching_workflow.md, "A C-emitted data symbol is
    # currently unstageable"). Exempt those, and only those: anything with a
    # real label still counts.
    byte_free = {m.group(1) for m in
                 re.finditer(r"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*=\s*[^=]", text, re.M)}
    labelled = {m.group(1) for m in re.finditer(r"^\s*([A-Za-z_][A-Za-z_0-9]*):", text, re.M)}
    extra = defined - {body} - set(names) - (byte_free - labelled)
    if extra:
        raise RuntimeError(f"splice for {names} would also define {sorted(extra)} in {source}")

def _export_missing(text: str, lines: list[str], body: str, wanted: list[str],
                  source: Path, entry_vma: int | None = None) -> str:
    """Re-emit an owned alias whose glue agbcc put in another section.

    agbcc emits each alias's `.globl`/`.thumb_set` pair into whichever section
    is still open, so a body can own a spelling whose pair landed much later in
    the file and is therefore outside the section being spliced. The binding is
    read back out of agbcc's own `.thumb_set`, never invented: if the compiled
    source does not alias this name to this body, the splice does not define it.
    """
    defined = {m.group(1) for m in re.finditer(r"^\s*\.globl\s+([A-Za-z_][A-Za-z_0-9]*)", text, re.M)}
    defined |= {m.group(1) for m in re.finditer(r"^\s*\.thumb_set\s+([A-Za-z_][A-Za-z_0-9]*)", text, re.M)}
    extra = []
    for name in wanted:
        if name == body or name in defined:
            continue
        if any(re.match(r"^\s*\.thumb_set\s+" + re.escape(name) + r"\s*,\s*" + re.escape(body) + r"\s*$", l)
               for l in lines):
            # The owning TU already aliases it; agbcc just put the pair in
            # another section. Re-emit the pair as-is -- this is the
            # evidence-based path and it needs no further check.
            extra += [f"\t.globl\t{name}\n", f"\t.thumb_set\t{name},{body}\n"]
            continue
        # Nothing in the TU binds this name, so the old behaviour was to skip
        # it and let the link fail -- at ~3 minutes a round, three times in one
        # comparison. Synthesising the pair fixes that, BUT ONLY when the name is
        # provably this body. Aliasing an arbitrary `export` name to the body
        # would be a silent retarget: a name denoting a different function
        # would link cleanly and move the call, and the only thing that would
        # notice is the byte diff thousands of bytes later. This repo has paid
        # for exactly that once. So the gate is name identity, and a name that
        # does not correspond is still refused loudly.
        if _same_vma(name, body) or (entry_vma is not None and _name_vma(name) == entry_vma):
            extra += [f"\t.globl\t{name}\n", f"\t.thumb_set\t{name},{body}\n"]
        else:
            raise RuntimeError(
                f"splice for {body} exports {name}, which no TU binds and whose "
                f"name does not denote {body}; refusing to invent the alias "
                f"(a mismatch here would move a call silently)")
    _assert_owns_only(text + "".join(extra), body, wanted, source)
    return text + "".join(extra)


def _same_vma(a: str, b: str) -> bool:
    """Do two C symbol names denote the same ROM address?

    Deliberately permissive about the `sub_` prefix and about a leading zero,
    because `norm_vma` absorbs one and the closure spells the same VMA both
    ways. Deliberately strict about everything else: if either name has no
    recognisable VMA, the answer is False, so the caller refuses.
    """
    va, vb = _name_vma(a), _name_vma(b)
    return va is not None and vb is not None and va == vb


def _name_vma(n: str) -> int | None:
    """The ROM VMA a C symbol name encodes, or None if it is a friendly name.

    Deliberately does NOT use `norm_vma`, which strips the ROM base and so
    collapses `sub_08000ED68` onto `_0800ED68` -- different addresses. That is
    fine for an inventory key and wrong for a gate that must not invent a
    binding.
    """
    # Use the repo's own convention rather than a private one. `norm_vma`
    # strips leading zeros, requires the result to start `80`, drops those two
    # digits and zfills to 6 -- so `sub_080018210`, `_08018210` and
    # `sub_08017414` all resolve, which a fixed digit-count bound got wrong
    # (`sub_080018210` really does assemble, at 0x08018210).
    #
    # That convention is lossy: `sub_08000ED68` and `_0800ED68` normalise to the
    # same value though they differ by 0x10000. Accepting it here is still
    # correct, because nm lookups, the screen and the inventory all apply the
    # same rule -- a gate stricter than the rest of the tooling would reject
    # names the rest of the pipeline considers identical. Being no MORE
    # permissive is the property that matters, and that is what the adversarial
    # self-test cases pin.
    m = re.search(r"(?:sub_)?_?([0-9A-Fa-f]{6,10})\b", n)
    if not m:
        return None
    v = m.group(1).lstrip("0").lower()
    if not v.startswith("80"):
        return None
    v = v[2:]
    if len(v) < 4 or len(v) > 6:
        return None
    return 0x08000000 | int(v.zfill(6), 16)


def _own_alias_glue(tail: list[str], body: str, names: list[str]) -> list[str]:
    """`.globl`/`.thumb_set` pairs that alias `body` under a name in `names`."""
    kept: list[str] = []
    i = 0
    while i < len(tail):
        glob = re.match(r"^\s*\.globl\s+([A-Za-z_][A-Za-z_0-9]*)\s*$", tail[i])
        thumb = re.match(r"^\s*\.thumb_set\s+([A-Za-z_][A-Za-z_0-9]*)\s*,\s*([A-Za-z_][A-Za-z_0-9]*)\s*$",
                         tail[i + 1]) if i + 1 < len(tail) else None
        if glob and thumb and thumb.group(1) == glob.group(1) and thumb.group(2) == body:
            if thumb.group(1) in names or glob.group(1) == body:
                kept += [tail[i], tail[i + 1]]
            i += 2
            continue
        if glob and glob.group(1) in names:
            kept.append(tail[i])
        i += 1
    return kept


def symbol_size(obj: Path, names: list[str]) -> int:
    from elftools.elf.elffile import ELFFile
    with obj.open("rb") as stream:
        elf = ELFFile(stream)
        symtab = elf.get_section_by_name(".symtab")
        for name in names:
            matches = symtab.get_symbol_by_name(name) or []
            for sym in matches:
                size = sym["st_size"]
                if size:
                    return size
    raise RuntimeError(f"no sized function symbol for {names} in {obj}")


def check_span(key: str, size: int, span_size: int) -> None:
    """The compiled body must fit the ROM span it is about to replace."""
    if size > span_size:
        raise RuntimeError(f"{key} compiled body {size} bytes exceeds ROM span {span_size}")


def splice_section(vma: int, body: str, span_size: int) -> str:
    """The `asm/*.s` text that replaces one ROM function, sized to `span_size`.

    A spliced entry has to occupy exactly its ROM span. The link is monolithic
    and sequential, so an entry that emits the wrong number of bytes moves every
    later address, and the byte comparison then reports the error thousands of
    bytes away from its cause.

    The pad is computed *by the assembler*, not by this tool. `symbol_size` is
    the extent agbcc's own object reports for the body, and agbcc measured it
    with the body starting on a four-byte boundary. The body aligns its own
    literal pool with `.align 2, 0`, and that aligns to the address the body
    actually assembles at -- so a body landing one word off emits a different
    number of bytes than that size, and `span_size - symbol_size` pads by the
    wrong amount. `.` is the assembler's own location counter, so
    `span_size - (. - label)` is the shortfall measured where the miscount
    happens, at whatever address the marker lands on.

    That was 0x08003248: a correct manifest entry, `EXACT 40/40` from the probe,
    `symbol_size` 40, and 38 bytes in the link because the entry before it had
    moved the body off the boundary the size was measured on. Reading the
    shortfall back out of the object cannot repair it, because the object's
    boundary is not the link's.

    agbcc emits divided Thumb syntax (`sub r1,r1,#1`), whereas the reconstructed
    source uses unified syntax (`subs`). Restore it around the whole body.

    The pad moves the location counter instead of filling with `.space`: gas
    reports a zero `.space` count as a "negative value" and warns about it, so
    every body that already fills its span would be noise on stderr, and it
    *ignores* a negative count outright. A backwards `.` is gas's `.org` path,
    so an over-long body fails the assembly instead of being quietly fitted to a
    span it does not fit, and the gap is zero-filled, which is what the ROM
    holds there.

    That refusal is not hypothetical: plenty of manifest entries have a body
    that exactly fills its span, and a marker landing one word off can make the
    body's own pool alignment *grow* by two bytes, so those bodies would need
    more room than the span has. No construction emits exactly `span_size` bytes
    for such an entry without cutting the body short, and cutting it short is
    what this gate exists to prevent -- so it stops. `check_span` catches the
    same overrun a step earlier whenever the object already knows the body is
    too big; this is the backstop for a body that only turns out too big at the
    address its marker put it at.
    """
    label = f".Lcs_body_{vma:08X}"
    pad = f".Lcs_pad_{vma:08X}"
    return ("    .syntax divided\n"
            f"{label}:\n"
            f"{body}"
            f"    .set {pad}, {span_size} - (. - {label})\n"
            f"    . = . + {pad}  @ inter-function alignment\n"
            "    .syntax unified\n")


def locate_region(original: str, start: str, end: str) -> tuple[int, int]:
    """The half-open text range `[start_at, end_at)` one splice replaces.

    Shared by `replace_body`, which does the replacing, and the `--spans` dry
    run, which only reports it -- so the geometry a maintainer eyeballs is the
    geometry the splice performs. Both markers must resolve exactly once and
    in order, or the entry is refused before any text is touched.
    """
    def marker(label: str) -> re.Pattern[str]:
        # A label may carry a trailing `@` comment -- the converted regions do
        # it constantly (`asm/carphys_tick.s:568` is
        # `_0800A06C:\t\t\t@ record 33 -> 15`). Demanding a bare `LABEL` line
        # made those bodies unspliceable even though the screen could find them,
        # so the screen offered a body this function then refused at splice
        # time. The `@`-only tail is deliberate: a DATA label
        # (`_080003B0: .4byte 0x087B04C4`) must still fail to match, or a
        # splice could bound itself on a constant-pool word.
        return re.compile(r"^\s*" + re.escape(label) + r"\s*(?:@.*)?$", re.M)
    starts = list(marker(start).finditer(original))
    ends = list(marker(end).finditer(original))
    if len(starts) != 1 or len(ends) != 1 or starts[0].start() >= ends[0].start():
        raise RuntimeError(f"ambiguous source markers: {start} .. {end}")
    return starts[0].start(), ends[0].start()


def replace_body(original: str, start: str, end: str, replacement: str) -> str:
    start_at, end_at = locate_region(original, start, end)
    return original[:start_at] + replacement + "\n" + original[end_at:]


def check_regions_disjoint(regions: list[tuple[str, int, int]], where: Path) -> None:
    """Refuse manifest entries whose spliced regions overlap in one file.

    `regions` is `(key, start_at, end_at)` per entry, located in the file's
    ORIGINAL text. Two entries splicing overlapping text is not recoverable at
    splice time: the first splice deletes the second's markers and it fails
    with "ambiguous source markers", naming neither the collision nor the
    other key -- or, where the overlap is only in marker lines, one entry
    silently rewrites the other's body. Locating every region up front and
    comparing them turns both into one named refusal before anything is
    touched.

    Regions that merely TOUCH are fine and common: one entry's end marker is
    the next entry's start marker, and each splice keeps everything from its
    end marker onward. Sorted by start, one adjacent sweep is complete: if any
    two regions overlap, the earlier one also overlaps the region immediately
    after it.
    """
    ordered = sorted(regions, key=lambda region: (region[1], region[2]))
    for a, b in zip(ordered, ordered[1:]):
        if b[1] < a[2]:
            raise RuntimeError(
                f"{where}: spliced regions overlap: {a[0]} covers text "
                f"[{a[1]}, {a[2]}) and {b[0]} covers [{b[1]}, {b[2]})")


def pad_only(rom: bytes, start_vma: int, end_vma: int, base: int = 0) -> bool:
    """Are the ROM bytes in `[start_vma, end_vma)` alignment padding?

    A manifest span may end before the probe's function inventory does, when the
    owning file's region ends there and the remainder is zero padding that the
    next file contributes. That case is legitimate; a span that reaches into
    real code is not. The length check matters as much as the zero test: a range
    running off the end of the image would slice short and read as all-zero.

    `base` is the ROM's address-0 offset (`probe.ROM_BASE` in production) and is
    a parameter so the rule can be pinned on a small synthetic buffer instead of
    a 128 MiB one.
    """
    if end_vma < start_vma:
        return False
    lo, hi = start_vma - base, end_vma - base
    if lo < 0 or hi > len(rom):
        return False
    return not any(rom[lo:hi])


def self_test() -> int:
    """Pin the span accounting on synthetic fixtures, never on a live build.

    The link is monolithic, so the only place a wrong span count surfaces is a
    byte comparison thousands of bytes downstream of its cause. That is why the
    accounting is checked here instead, at the point it is decided.

    Every fixture is assembled for real, so these cases pin the bytes the
    assembler emits rather than the text it was handed. Asserting against
    `build/matching-slice/all/` or a real `asm/*.s` label would make a case
    pass or fail depending on whether that artifact is current, and a fix that
    is not pinned is a fix that can be reverted by accident.
    """
    ok = 0
    total = 0

    def check(name: str, got, want) -> None:
        # `total` is counted, never hardcoded: adding a case then silently
        # breaks the exit status, and the next person "fixes" it by editing
        # the number instead of finding the broken case.
        nonlocal ok, total
        total += 1
        if got == want:
            ok += 1
        else:
            print(f"FAIL {name}: got {got!r} want {want!r}", file=sys.stderr)

    # `replace_body`'s marker regex must accept exactly the line shapes
    # `promotion_screen.pick_marker` accepts, or the screen offers a body that
    # the splicer then refuses -- the two are a matched pair, and widening one
    # alone moves the failure rather than fixing it. Pin BOTH directions: the
    # accept side is a trailing `@` comment on a real label, the reject side is
    # a `.4byte` data label, which must never bound a splice.
    annotated = ("head\n"
                 "_0800A06C:\t\t\t@ record 33 -> 15\n"
                 "movs r0, #15\n"
                 "bx lr\n"
                 "_0800A120:\t\t\t@ record 19\n")
    spliced = replace_body(annotated, "_0800A06C:", "_0800A120:", "REPLACED")
    check("splices across labels carrying trailing @ comments",
          "REPLACED" in spliced and "movs r0, #15" not in spliced, True)
    # The start marker here MUST exist, or the splice raises on the START and
    # this case passes for a harness reason while never testing the data label
    # at all -- which is exactly what it did on first write, and exactly what an
    # over-widened regex then failed to catch.
    pooled = "head:\n_080003B0: .4byte 0x087B04C4\nbx lr\n"
    try:
        replace_body(pooled, "head:", "_080003B0:", "REPLACED")
        check("data label refuses to bound a splice", "matched", "raised")
    except RuntimeError:
        check("data label refuses to bound a splice", "raised", "raised")

    # The `--spans` dry run must report the SAME region `replace_body`
    # replaces, so pin the shared locator: the region starts at its start
    # marker and stops at the START of its end marker, which the splice keeps.
    start_at, end_at = locate_region(annotated, "_0800A06C:", "_0800A120:")
    check("locate_region bounds the replaced text, keeping the end marker",
          annotated[start_at:end_at].startswith("_0800A06C:")
          and annotated[end_at:].startswith("_0800A120:"), True)
    # Two manifest entries may not splice overlapping text in one file. Pin
    # the accept side twice (truly disjoint AND the shared-boundary case the
    # real manifest relies on, one entry's end marker being the next entry's
    # start marker) and the refusals by their message -- a guard that refused
    # everything would pass every "it raises" case here.
    def disjoint(found):
        try:
            check_regions_disjoint(found, Path("synthetic.s"))
            return ""
        except RuntimeError as exc:
            return str(exc)
    check("disjoint splice regions are accepted",
          disjoint([("a", 0, 10), ("b", 20, 30)]), "")
    check("regions sharing a boundary are accepted",
          disjoint([("a", 0, 10), ("b", 10, 20)]), "")
    check("overlapping regions are refused, naming both keys and ranges",
          disjoint([("a", 0, 12), ("b", 10, 20)]),
          "synthetic.s: spliced regions overlap: a covers text [0, 12) "
          "and b covers [10, 20)")
    check("a region nested inside another is refused",
          "overlap" in disjoint([("a", 0, 30), ("b", 10, 20)]), True)
    check("duplicate regions are refused",
          "overlap" in disjoint([("a", 0, 10), ("b", 0, 10)]), True)
    check("entries handed over out of order are still compared",
          "overlap" in disjoint([("b", 10, 20), ("a", 0, 12)]), True)

    def guard_error(key: str, size: int, span_size: int) -> str:
        """The `size > span_size` guard's own message, or `""` if it let it pass."""
        try:
            check_span(key, size, span_size)
        except RuntimeError as exc:
            return str(exc)
        return ""

    with tempfile.TemporaryDirectory(prefix="mcs-selftest-") as tmp:
        work = Path(tmp)
        counter = [0]

        def emitted(body: str, span_size: int, skew: int) -> bytes:
            """Assemble a splice exactly as `main()` writes it, at a skewed address.

            `skew` is the body's address mod 4 and is all the body can observe
            of it: the only address-sensitive thing in an agbcc body is the pool's
            `.align 2, 0`, and a relocatable section addresses itself from zero.
            So the fixture sits the body at offset `skew` rather than
            materialising 0x08000000 bytes of section.

            The extent is bounded by a label the fixture puts immediately after
            the splice, the way the next function follows it in the real link.
            Measuring the object's file length instead folds in the assembler's
            end-of-section alignment fill, which a monolithic link never inserts
            between two functions -- and that fill is 2 bytes, exactly the size
            of the disagreement these cases are about.
            """
            from elftools.elf.elffile import ELFFile
            counter[0] += 1
            tag, end = f"case{counter[0]}", f"mcs_end_{counter[0]}"
            src, obj = work / f"{tag}.s", work / f"{tag}.o"
            src.write_text("    .syntax unified\n    .text\n"
                           + (f"    .space {skew}, 0\n" if skew else "")
                           + splice_section(0x08000000, body, span_size) + f"{end}:\n",
                           encoding="utf-8")
            command(["arm-none-eabi-as", "-mcpu=arm7tdmi", src, "-o", obj])
            with obj.open("rb") as stream:
                syms = ELFFile(stream).get_section_by_name(".symtab")
                stop = syms.get_symbol_by_name(end)[0]["st_value"]
            blob = work / f"{tag}.bin"
            command(["arm-none-eabi-objcopy", "-O", "binary", obj, blob])
            return blob.read_bytes()[skew:stop]

        def assembler_error(body: str, span_size: int, skew: int) -> str:
            """The assembler's diagnostic for a splice, or `""` if it accepted it."""
            try:
                emitted(body, span_size, skew)
            except RuntimeError as exc:
                return str(exc)
            return ""

        # agbcc's shape: instructions, `.align 2, 0` before the literal pool,
        # then `.size`. Five instructions is 10 bytes, so the pool alignment
        # contributes 2 bytes on a four-byte boundary and nothing one word off
        # -- the whole extent disagreement this gate has to survive. agbcc's
        # divided `nop` is `mov r8,r8`, so the fixture bytes below are `46c0`.
        pooled = ("    .globl\tPooled_0\n"
                  "    .type\t Pooled_0,function\n"
                  "    .thumb_func\n"
                  "Pooled_0:\n"
                  "\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n"
                  ".L1:\n"
                  "    .align\t2, 0\n"
                  ".L2:\n"
                  "    .word\t142266224\n"
                  ".L3:\n"
                  "    .size\t Pooled_0,.L3-Pooled_0\n")
        bare = ("    .globl\tBare_0\n"
                "    .type\t Bare_0,function\n"
                "    .thumb_func\n"
                "Bare_0:\n"
                "\tnop\n\tnop\n\tnop\n"
                "    .size\t Bare_0,.-Bare_0\n")
        nops, pool = bytes.fromhex("c046" * 5), bytes.fromhex("70cf7a08")
        aligned, skewed = 0, 2

        # A body whose section extent is smaller than the symbol size is padded
        # out to the span. `symbol_size` would report 16 for this body (10 + 2
        # of pool alignment + 4), and padding `20 - 16` reaches the span only on
        # a four-byte boundary; one word off the body stops at 14, so that
        # arithmetic leaves the entry 2 bytes short of the span it replaces.
        check("pooled body reaches its span on a word boundary",
              len(emitted(pooled, 20, aligned)), 20)
        check("pooled body reaches its span one word off",
              len(emitted(pooled, 20, skewed)), 20)
        check("span count does not depend on where the marker landed",
              len(emitted(pooled, 20, skewed)) - len(emitted(pooled, 20, aligned)), 0)
        # The padding goes after the body and the body's own bytes survive: on
        # the skewed boundary the pool lands unpacked, so 14 body bytes then 6
        # of zero fill; on the word boundary the pool keeps its own 2 bytes of
        # alignment and the fill is 4. Nothing is dropped to make the span up.
        check("pooled body keeps its bytes and is zero-padded after",
              emitted(pooled, 20, skewed), nops + pool + bytes(6))
        check("pooled body keeps its own pool alignment on a word boundary",
              emitted(pooled, 20, aligned), nops + bytes(2) + pool + bytes(4))

        # A body whose extent already equals the span gets no padding: the
        # emitted bytes are the body's and stop where the body stops.
        check("body filling its span is not padded", emitted(bare, 6, skewed), nops[:6])
        check("body filling its span is not padded on a word boundary",
              emitted(bare, 6, aligned), nops[:6])

        # A body that overruns the span is neither clamped nor truncated to fit.
        # The pad cannot move the location counter backwards, so the splice
        # fails the assembly outright: `size > span_size` catches this first
        # whenever the object already knows the body is too big, and this is
        # the backstop for a body that only turns out too big at the address
        # the marker put it at. Quietly fitting it would hide the defect
        # behind a passing gate.
        check("body overrunning its span is refused, not fitted",
              assembler_error(bare, 4, aligned) != "", True)
        check("body inside its span is not refused",
              assembler_error(bare, 6, aligned), "")

        # The `size > span_size` guard, pinned in both directions and by its
        # wording: a body that exactly fills its span is accepted, and a body
        # one byte over is refused with a message that names both sizes.
        check("span guard accepts a body that fits",
              guard_error("0x08003248", 40, 40), "")
        check("span guard refuses a body one byte over",
              guard_error("0x08003248", 42, 40),
              "0x08003248 compiled body 42 bytes exceeds ROM span 40")

        # `symbol_size` reads the body's extent, and the `.thumb_set` spellings
        # agbcc emits alongside it carry the same one. The splices pass the
        # alias spellings in `names` too, so this pins that an alias consulted
        # first agrees with the body rather than silently answering for it.
        alias_src, alias_obj = work / "alias.s", work / "alias.o"
        alias_src.write_text("    .syntax unified\n    .text\n" + pooled +
                             "    .globl\talias_of_Pooled_0\n"
                             "    .thumb_set\talias_of_Pooled_0,Pooled_0\n", encoding="utf-8")
        command(["arm-none-eabi-as", "-mcpu=arm7tdmi", alias_src, "-o", alias_obj])
        check("symbol_size of a body", symbol_size(alias_obj, ["Pooled_0"]), 16)
        check("symbol_size agrees with the body through its alias",
              symbol_size(alias_obj, ["alias_of_Pooled_0"]), 16)
        # A manifest span may legitimately stop at the owning file's region end
        # while the probe's inventory runs on into alignment padding. Pin both
        # directions, or the rule could pass by refusing everything: the
        # ordinary case (span == inventory) and a zero-pad overshoot must be
        # ACCEPTED, and an overshoot holding a real byte or running off the
        # image must be REFUSED. A synthetic buffer, not the 128 MiB ROM.
        rom = bytes([0x30, 0xB5, 0x00, 0x00, 0x47, 0x70, 0x00, 0x00,
                     0x12, 0x34, 0x56, 0x78])
        check("span ending exactly at the inventory is accepted",
              pad_only(rom, 0, 0), True)
        check("two-byte zero-pad overshoot is accepted",
              pad_only(rom, 6, 8), True)
        check("overshoot holding a real byte is refused",
              pad_only(rom, 6, 9), False)
        check("overshoot running off the image is refused, not read as all-zero",
              pad_only(rom, 10, 16), False)
        check("a backwards range is refused",
              pad_only(rom, 8, 6), False)
        # With the base applied the VMA-addressed range is read from the right
        # place and accepted; without it the same range is out of bounds and
        # refused, so this pair fails if the offset is dropped.
        check("the base offset is applied, so a VMA range is read at VMA-base",
              pad_only(rom, 0x08000006, 0x08000008, 0x08000000), True)
        check("and the same range is out of bounds when the base is dropped",
              pad_only(rom, 0x08000006, 0x08000008), False)
        # The splice-ownership rule, and the one exemption it grants. A name
        # assigned to an absolute expression (`.globl X` / `X = 0x080CB68C`)
        # emits no bytes, so it cannot be the over-wide-section hazard the rule
        # exists to catch, and no manifest field can declare it: the screen
        # builds `export` from closure *branch* targets, and this is a data
        # symbol the C must emit so a SYMBOL_REF leaf survives. Pin the
        # exemption AND the three refusals it must not swallow -- especially
        # "alias and a real label of the same name", which is the case a naive
        # `defined -= byte_free` would accept.
        def owns(text, body="_080012B34", names=("sub_080012B34",)):
            try:
                _assert_owns_only(text, body, list(names), Path("synthetic.s"))
                return True
            except RuntimeError:
                return False
        check("a byte-free absolute alias is exempt",
              owns(".globl _080CB68C\n_080CB68C = 0x080CB68C\n"
                   "sub_080012B34:\n bx lr\n"), True)
        check("a real label of that name is still refused",
              owns(".globl _080CB68C\n_080CB68C:\n .word 0\n"
                   "sub_080012B34:\n bx lr\n"), False)
        check("an alias that also has a real label is still refused",
              owns(".globl _X\n_X = 0x8000000\n_X:\n .word 0\n"
                   "sub_080012B34:\n bx lr\n"), False)
        check("a second real function label is still refused",
              owns("other_fn:\n bx lr\nsub_080012B34:\n bx lr\n"), False)
        # A `.thumb_set` DOES rebind a section, so it must never be treated as
        # byte-free. It has no `=` at the symbol position, so the alias regex
        # cannot see it -- pinned here so a future rewrite that matches on
        # `.globl` instead of on an assignment would fail.
        check("a .thumb_set rebind is still refused",
              owns(".thumb_set _X, _080012B34\nsub_080012B34:\n bx lr\n"), False)
        # And the assignment form is genuinely byte-free, in decimal as well as
        # hex -- the regex keys on `=` followed by a non-`=`, not on the literal.
        check("a decimal absolute assignment is exempt too",
              owns(".globl _X\n_X = 134565772\nsub_080012B34:\n bx lr\n"), True)
        # The discriminating case. A `.globl` with NO assignment defines
        # something real (whatever the assembler resolves it against), so it
        # must be refused; only a name *assigned* an absolute is byte-free.
        # Without this case a filter that keyed on `.globl` instead of on `=`
        # would pass every other case here and silently exempt real bindings.
        check("a bare .globl with no assignment is still refused",
              owns(".globl _X\nsub_080012B34:\n bx lr\n"), False)
        # The export-synthesis gate. An `export` name the owning TU never binds
        # is synthesised so the entry can link -- but ONLY when the name is
        # provably this body. Without that gate a name denoting a DIFFERENT
        # function gets aliased to the body, links cleanly, and moves the call;
        # the only thing that would notice is the byte diff much later. This
        # repo has paid for that exact silent retarget once.
        # NOTE the fixture: the section must be labelled with the BODY itself.
        # Labelling it `body_lbl` makes `_assert_owns_only` raise before the
        # gate is reached, so every case here would answer for a harness
        # reason -- the refusals especially, since a raise looks identical to a
        # correct refusal.
        def synth(name, body="_0800ED68", lines=()):
            try:
                out = _export_missing(f"{body}:\n bx lr\n", list(lines),
                                      body, [name], Path("synthetic.s"))
                return f".thumb_set\t{name},{body}" in out, ""
            except RuntimeError as e:
                return False, str(e)
        ok_twin, _ = synth("sub_0800ED68")
        check("an export twin of the body IS synthesised", ok_twin, True)
        # A 9-digit spelling IS the same address under the repo's own naming
        # convention: `sub_08000ED68` is how 0x0800ED68 is written with an extra
        # leading zero, and nm, the screen and the inventory all resolve it that
        # way. My first attempt rejected it, on the premise that a 9-digit form
        # is a different VMA -- which is how `sub_080018210` came to be refused
        # even though it really assembles, at 0x08018210. The adversarial cases
        # below are what the gate is for; this one is only the convention.
        ok_9, _ = synth("sub_08000ED68")
        check("a 9-digit spelling of the body's VMA is a twin", ok_9, True)
        ok_real, _ = synth("sub_080018210")
        check("a genuine 9-digit symbol resolves", _name_vma("sub_080018210"),
              0x08018210)
        # `_0800ED68` itself is the body, so the loop skips it via `name ==
        # body` and nothing is synthesised -- correct, and not worth a case.
        # The adversarial case: a name that denotes a different function.
        bad_ok, bad_why = synth("sub_08009999")
        check("an export naming a DIFFERENT function is refused", bad_ok, False)
        # ...and refused by the identity gate specifically, not by some other
        # check happening to fire first. Assert on the reason, as elsewhere.
        check("...with a reason that names the refusal",
              "refusing to invent the alias" in bad_why, True)
        check("...naming the body it refused to bind to",
              "_0800ED68" in bad_why, True)
        # A body whose `c_name` is a FRIENDLY name carries no VMA at all, so
        # comparing the export name against the body string can never work. The
        # entry's own VMA is threaded in for exactly this: `SubsysStoreConfig`
        # at 0x08004A2C was refused as "name does not denote" when the name is
        # plainly the body. Gated on the VMA, so it stays a provable identity.
        def synth_friendly(name, entry_vma=0x08004A2C):
            try:
                out = _export_missing("SubsysStoreConfig:\n bx lr\n", [],
                                      "SubsysStoreConfig", [name],
                                      Path("synthetic.s"), entry_vma)
                return ".thumb_set\tsub_08004A2C,SubsysStoreConfig" in out
            except RuntimeError:
                return False
        check("an export twin of a FRIENDLY-named body is synthesised via the entry VMA",
              synth_friendly("sub_08004A2C"), True)
        check("...but a different VMA still is refused",
              synth_friendly("sub_08009999"), False)
        name_ok, name_why = synth("Some_Friendly_Name")
        check("an unrecognisable export name is refused", name_ok, False)
        check("...by the same gate", "refusing to invent" in name_why, True)
        check("_same_vma: agrees on sub_/underscore, rejects a different VMA and a non-name",
              (_same_vma("sub_0800ED68", "_0800ED68"),
               _same_vma("sub_0800ED68", "_0800ED69"),
               _same_vma("nope", "_0800ED68")), (True, False, False))
        # TRAP 7's lever, pinned end to end. A body may emit its own INTERIOR
        # label (`.globl _0802BCF6` inside the function, `_0802BCF6:` at
        # body+2) so a promoted caller's `b.n` into the middle of this span
        # keeps a target after the splice. The section must carry the label
        # THROUGH at its interior offset and must NOT synthesise a
        # `.thumb_set` twin -- that would bind the name to the ENTRY and
        # silently move both callers two bytes forward. The same rule covers
        # a data label on a pool word (`_0802BCE4: .word 0x080614E0`), which
        # an outside `ldr` reads: the shape.
        inbody = work / "inbody.s"
        inbody.write_text(
            '    .section\t.text.SoundBCF4_FetchBE,"ax",%progbits\n'
            "    .align\t2, 0\n"
            "    .globl\tSoundBCF4_FetchBE\n"
            "    .type\t SoundBCF4_FetchBE,%function\n"
            "    .thumb_func\n"
            "SoundBCF4_FetchBE:\n"
            "\tpush\t{lr}\n"
            "    .globl\t_0802BCF6\n"
            "_0802BCF6:\n"
            "\tpop\t{r0}\n"
            "\tbx\tr0\n"
            "    .size\t SoundBCF4_FetchBE,.-SoundBCF4_FetchBE\n",
            encoding="utf-8")
        kept = asm_section(inbody, ["SoundBCF4_FetchBE"],
                           ["SoundBCF4_FetchBE", "_0802BCF6"], 0x0802BCF4)
        check("an in-body label the export declares survives at its interior offset",
              "\n_0802BCF6:\n" in kept and ".globl\t_0802BCF6" in kept, True)
        check("...and no .thumb_set twin is invented for it",
              ".thumb_set\t_0802BCF6" in kept, False)
        # The manifest `export` is what licenses the name: a label the entry
        # does not declare is the over-wide-section hazard, refused loudly.
        try:
            asm_section(inbody, ["SoundBCF4_FetchBE"], ["SoundBCF4_FetchBE"],
                        0x0802BCF4)
            check("an in-body label the export omits is refused", "matched", "raised")
        except RuntimeError:
            check("an in-body label the export omits is refused", "raised", "raised")
        # Without the in-body `.globl` the label is not exported at all -- and
        # synthesising the pair would misbind it to the entry, exactly the
        # two-byte retarget trap. Loud refusal, never an invented alias.
        bare = work / "inbody_bare.s"
        bare.write_text(
            inbody.read_text(encoding="utf-8").replace(
                "    .globl\t_0802BCF6\n", ""), encoding="utf-8")
        try:
            asm_section(bare, ["SoundBCF4_FetchBE"],
                        ["SoundBCF4_FetchBE", "_0802BCF6"], 0x0802BCF4)
            check("a bare in-body label is refused, never aliased to the entry",
                  "matched", "raised")
        except RuntimeError as exc:
            check("a bare in-body label is refused, never aliased to the entry",
                  "refusing to invent the alias" in str(exc), True)
        # The pool word an outside `ldr` reads -- the shape proper.
        pool = work / "inbody_pool.s"
        pool.write_text(
            '    .section\t.text.SoundBCCE_ValidateFrag,"ax",%progbits\n'
            "    .align\t2, 0\n"
            "    .globl\tSoundBCCE_ValidateFrag\n"
            "    .type\t SoundBCCE_ValidateFrag,%function\n"
            "    .thumb_func\n"
            "SoundBCCE_ValidateFrag:\n"
            "\tpush\t{r0}\n"
            "    .globl\t_0802BCE4\n"
            "_0802BCE4: .word 0x080614E0\n"
            "    .size\t SoundBCCE_ValidateFrag,.-SoundBCCE_ValidateFrag\n",
            encoding="utf-8")
        kept = asm_section(pool, ["SoundBCCE_ValidateFrag"],
                           ["SoundBCCE_ValidateFrag", "_0802BCE4"], 0x0802BCCE)
        check("a declared pool-word data label survives the splice",
              "_0802BCE4: .word 0x080614E0" in kept
              and ".thumb_set\t_0802BCE4" not in kept, True)
    print(f"match_c_slice self-test: {ok}/{total}")
    return 0 if ok == total else 1


def run_probe(tu_work: Path, report: Path, c_names: list[str]) -> dict:
    """Score one translation unit's bodies, in THIS process, into `tu_work`.

    `corpus_match_probe.main()` is the same entry point the old
    `[sys.executable, corpus_match_probe.py, ...]` subprocess ran, with the
    same flags, so the bytes it scores are the same bytes -- only the
    interpreter is shared. That matters because the probe is import-safe
    (`if __name__ == "__main__"`), and `--function` is already repeatable, so
    this is batching across process boundaries moved inside one process rather
    than a reimplementation of the scorer.

    `tu_work` is still per TU and must not be shared: `main()` writes
    `<stem>.s`/`<stem>.o` and the disassembly blobs there, and the splice below
    reads each body's section back out of the directory its own TU was
    compiled in. `main()` leaves the module-level work dir set to the last TU
    it ran; the caller re-sets it per key before reading anything.
    """
    argv = ["corpus_match_probe.py", "--c89", "--require-all",
            "--work-dir", str(tu_work), "--json", str(report)]
    for c_name in c_names:
        argv += ["--function", c_name]
    saved_argv = sys.argv
    out = io.StringIO()
    try:
        sys.argv = argv
        with contextlib.redirect_stdout(out):
            returncode = probe.main()
    finally:
        sys.argv = saved_argv
    if returncode:
        # The old code raised on the child's stdout+stderr. Keep that: the probe
        # is where a "cannot compile this TU" is explained, and a silent
        # non-zero here would look like a clean run.
        raise RuntimeError(out.getvalue() or f"corpus_match_probe failed for {c_names}")
    return json.loads(report.read_text(encoding="utf-8"))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("function", nargs="*", help="manifest key(s), e.g. _080028250")
    ap.add_argument("--all", action="store_true",
                    help="splice every manifest entry and compare the whole slice")
    ap.add_argument("--negative-control", action="store_true",
                    help="also prove a deliberately changed C instruction is rejected")
    ap.add_argument("--spans", action="store_true",
                    help="dry-run: resolve every selected entry's splice region "
                         "and check per-file disjointness, without compiling")
    ap.add_argument("--self-test", action="store_true",
                    help="pin the span accounting on synthetic fixtures")
    args = ap.parse_args()
    if args.self_test:
        return self_test()
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    if args.all and args.function:
        ap.error("--all cannot be combined with individual functions")
    if args.spans and args.negative_control:
        ap.error("--spans does not build, so it cannot run the negative control")
    if not args.all and not args.function and not args.spans:
        ap.error("pass a function, --all, or --spans")
    keys = sorted(manifest) if (args.all or (args.spans and not args.function)) else []
    if not args.all:
        for name in args.function:
            vma = probe.parse_vma_name(name)
            if vma is None:
                ap.error(f"invalid VMA: {name}")
            key = f"{vma:#010x}"
            if key not in manifest:
                ap.error(f"{key} has no entry in {MANIFEST.relative_to(ROOT)}")
            keys.append(key)
        keys = sorted(set(keys))
    if args.negative_control and "0x08028250" not in keys:
        ap.error("--negative-control requires the 0x08028250 manifest entry")
    spans = probe.rom_functions()
    # Splice geometry is validated up front, on the ORIGINAL text of each file,
    # before any scoring or splicing: every marker must resolve, the manifest
    # span must agree with the function inventory, and no two entries in one
    # file may splice overlapping text (`check_regions_disjoint`). At splice
    # time an overlap is unrecoverable and misreported -- the first splice
    # deletes the second's markers and it fails with "ambiguous source
    # markers", naming neither the collision nor the other key. This pass is
    # also the whole of `--spans`, so the dry run a maintainer eyeballs is the
    # same code the build is gated on.
    rom_bytes = probe.ROM.read_bytes()
    file_text: dict[Path, str] = {}
    regions: dict[Path, list[tuple[str, int, int]]] = collections.defaultdict(list)
    for key in keys:
        entry = manifest[key]
        vma = int(key, 16)
        # The manifest's span may legitimately end BEFORE the inventory's, when
        # the owning file's region ends there and the rest is alignment padding
        # that belongs to whatever the next file contributes. `promotion_screen`
        # already clamps exactly that case (`pad_only`), so demanding strict
        # equality here rejected the entry the screen had just approved. The
        # guard keeps its teeth: the span may not run past the inventory, and
        # the overshoot must be zero, so a manifest that reaches into real code
        # is still refused.
        inv_end = spans.get(vma)
        end_vma = int(entry["end_vma"], 16)
        if inv_end is None or end_vma > inv_end or not pad_only(rom_bytes, end_vma, inv_end, probe.ROM_BASE):
            raise RuntimeError(f"manifest span for {key} disagrees with function inventory")
        asm_path = ROOT / entry["asm_file"]
        original = file_text.get(asm_path)
        if original is None:
            original = asm_path.read_text(encoding="utf-8")
            file_text[asm_path] = original
        start_at, end_at = locate_region(original, entry["start_marker"], entry["end_marker"])
        regions[asm_path].append((key, start_at, end_at))
    for asm_path, found in sorted(regions.items()):
        check_regions_disjoint(found, asm_path.relative_to(ROOT))
    if args.spans:
        for asm_path in sorted(regions):
            found = regions[asm_path]
            print(f"{asm_path.relative_to(ROOT)} ({len(found)} entries)")
            text = file_text[asm_path]
            for key, start_at, end_at in found:
                entry = manifest[key]
                first = text.count("\n", 0, start_at) + 1
                last = text.count("\n", 0, end_at) + 1
                print(f"  {key}  lines {first}-{last}  "
                      f"{int(entry['end_vma'], 16) - int(key, 16)} bytes  "
                      f"{entry['start_marker']} .. {entry['end_marker']}")
        print(f"PASS {sum(len(found) for found in regions.values())} splice regions "
              f"in {len(regions)} files: every marker resolves and no two "
              f"regions in a file overlap")
        return 0
    work = ROOT / "build/matching-slice" / ("all" if args.all else "_".join(key[2:] for key in keys))
    work.mkdir(parents=True, exist_ok=True)
    replacements: dict[Path, str] = {}
    promoted: list[tuple[str, int, str]] = []
    # Score one TU per probe run, not one body per run, and run all of them in
    # this process. The per-process cost is the ROM read, the closure symbol
    # table over ~12k symbols, the src/ parse and the span map -- 0.86 s,
    # measured, paid once per `python3`. The old one-body-per-run shape paid it
    # 275 times for 56 TUs and that, not compilation, was the 13-minute link;
    # grouping by TU fixed the redundancy, and one interpreter removes the rest.
    src_by_name: dict[str, Path] = {}
    for src_path, name, _vma, _body, _weak in probe.src_functions():
        src_by_name.setdefault(name, Path(src_path))
    # `src_functions()` reports the VMA-shaped spelling of each body, so a
    # friendly c_name (`MenuStage_0800D77C`, aliased to `_0800D77C`) is not a
    # key here. Those still group, but only against themselves: the same VMA
    # appears as a one-line stub in a dozen TUs and the probe picks by rank, so
    # guessing the file up front could bind a body to the wrong work dir. An
    # isolated group is correct by construction and costs one run.
    groups: dict[Path, list[str]] = collections.defaultdict(list)
    for key in keys:
        c_name = manifest[key]["c_name"]
        if c_name in src_by_name:
            groups[src_by_name[c_name]].append(key)
            continue
        vma = probe.parse_vma_name(c_name)
        if vma is None or not any(f[2] == vma for f in probe.src_functions()):
            raise RuntimeError(f"{key}: c_name {c_name} has no definition in src/")
        groups[work / f"solo_{key[2:]}"].append(key)
    scored: dict[str, dict] = {}
    work_by_key: dict[str, Path] = {}
    ready_report_path = ROOT / "build/era-corpus/ready-report.json"
    ready_work_path = ROOT / "build/era-corpus/ready-work"
    ready_scored: dict[str, dict] = {}
    ready_mtime = 0.0
    if ready_report_path.is_file() and ready_work_path.is_dir():
        try:
            ready_mtime = ready_report_path.stat().st_mtime
            ready_data = json.loads(ready_report_path.read_text(encoding="utf-8"))
            for rec in ready_data.get("results", []):
                ready_scored[f"{int(rec['vma'], 16):#010x}"] = rec
        except Exception:
            ready_scored = {}

    for src_path, group in sorted(groups.items(), key=lambda kv: str(kv[0])):
        c_names = [manifest[k]["c_name"] for k in group]
        # Fast path: if ready-work and ready-report already contain fresh scored
        # artifacts for all keys in this group, reuse them directly.
        use_ready = False
        if ready_scored and all(k in ready_scored for k in group):
            group_fresh = True
            for k in group:
                rec = ready_scored[k]
                source = ROOT / rec["source"]
                if not source.is_file() or source.stat().st_mtime > ready_mtime:
                    group_fresh = False
                    break
                s_file = ready_work_path / f"{source.stem}.s"
                o_file = ready_work_path / f"{source.stem}.o"
                if not s_file.is_file() or not o_file.is_file():
                    group_fresh = False
                    break
            if group_fresh:
                use_ready = True

        if use_ready:
            for k in group:
                scored[k] = ready_scored[k]
                work_by_key[k] = ready_work_path
        else:
            report = work / f"{src_path.stem}_tu_report.json"
            tu_work = work / f"{src_path.stem}_tu_work"
            tu_report = run_probe(tu_work, report, c_names)
            for rec in tu_report["results"]:
                scored[f"{int(rec['vma'], 16):#010x}"] = rec
            for k in group:
                work_by_key[k] = tu_work
    for key in keys:
        vma = int(key, 16)
        entry = manifest[key]
        # The inventory agreement guard ran in the geometry pass above.
        if key not in scored:
            raise RuntimeError(f"expected one scored C body at {key}")
        rec = scored[key]
        probe.WORK = work_by_key[key]
        if any(not x.get("resolved", True) for x in rec["call_targets"] + rec["pool_words"]):
            raise RuntimeError(f"{key} has an unresolved relocation")
        source = ROOT / rec["source"]
        asm_output = probe.WORK / f"{source.stem}.s"
        obj = probe.WORK / f"{source.stem}.o"
        names = [rec["name"]]
        if rec.get("alias_of"):
            names.append(rec["alias_of"])
        # `c_name` is the spelling this entry promises the rest of the link;
        # `export` lists spellings other assembly still branches to, whose
        # labels this splice replaces (promotion rule 4).
        owned = list(names)
        for extra in (entry.get("c_name"), *entry.get("export", ())):
            if extra and extra not in owned:
                owned.append(extra)
        section = asm_section(asm_output, names, owned, vma)
        if args.negative_control and vma == 0x08028250:
            original_insn = "\tsub\tr1, r1, #1"
            if section.count(original_insn) != 1:
                raise RuntimeError("negative-control instruction anchor is not unique")
            section = section.replace(original_insn, "\tsub\tr1, r1, #2")
        size = symbol_size(obj, names)
        span_size = int(entry["end_vma"], 16) - vma
        check_span(key, size, span_size)
        if INCBIN_RE.search(section):
            raise RuntimeError(f"{key} compiled section contains .incbin")
        # Local compiler labels are TU-scoped in their own object. Give them a
        # VMA prefix before splicing several C bodies into one assembly section.
        section = re.sub(r"\.L([A-Za-z_0-9]+)", rf".Lmatch_{vma:08X}_\1", section)
        section = splice_section(vma, section, span_size)
        asm_path = ROOT / entry["asm_file"]
        original = replacements.get(asm_path, file_text[asm_path])
        changed = replace_body(original, entry["start_marker"], entry["end_marker"], section)
        if INCBIN_RE.search(changed):
            raise RuntimeError(f"{key} modified code source contains .incbin")
        replacements[asm_path] = changed
        promoted.append((key, span_size, rec["status"]))
    # Only the files this manifest splices are rewritten, and the assembler
    # searches this directory first. A stale override from a larger manifest
    # would otherwise be assembled in place of the real source, so a shrunken
    # manifest could report PASS against a previous run's bytes.
    override_dir = work / "asm"
    shutil.rmtree(override_dir, ignore_errors=True)
    override_dir.mkdir(parents=True, exist_ok=True)
    for asm_path, changed in replacements.items():
        (override_dir / asm_path.name).write_text(changed, encoding="utf-8")
    obj_out, elf_out, bin_out = work / "code.o", work / "code.elf", work / "code.bin"
    command(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-I", override_dir,
             "-I", ROOT / "asm", ROOT / "asm/code.s", "-o", obj_out])
    command(["arm-none-eabi-ld", "-T", ROOT / "ldscript_slice.ld", obj_out, "-o", elf_out])
    command(["arm-none-eabi-objcopy", "-O", "binary", elf_out, bin_out])
    built = bin_out.read_bytes()
    rom = rom_bytes[:probe.CODE_END - probe.ROM_BASE]
    if len(built) != len(rom):
        raise RuntimeError(f"code slice size {len(built)} != reference {len(rom)}")
    if built != rom:
        first = next(i for i, (a, b) in enumerate(zip(built, rom)) if a != b)
        if args.negative_control and 0x28250 <= first < 0x28258:
            print(f"PASS negative control: changed C instruction rejected at ROM offset 0x{first:06X}")
            return 0
        raise RuntimeError(f"C-owned code slice differs at ROM offset 0x{first:06X}: "
                           f"got {built[first]:02x}, expected {rom[first]:02x}")
    if args.negative_control:
        raise RuntimeError("negative control unexpectedly matched baserom")
    print(f"PASS {len(promoted)} C-owned functions / {sum(size for _, size, _ in promoted)} bytes "
          f"in an independent, byte-identical {len(built)}-byte code slice")
    for key, size, isolated_status in promoted:
        print(f"  {key}: {size} bytes ({isolated_status} isolated probe)")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as exc:
        print(f"match_c_slice: {exc}", file=sys.stderr)
        raise SystemExit(1)
