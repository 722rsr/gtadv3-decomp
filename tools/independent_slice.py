#!/usr/bin/env python3
"""Build and verify an independent link slice of the GT Advance 3 cartridge.

The reference `make` build reproduces the cartridge by assembling reconstructed
source and splicing the still-unknown remainder straight out of `baserom.gba`.
Neither the reference nor any removed hybrid build answers the question
this tool answers:

    can a contiguous range of the cartridge be produced from reconstructed
    source alone, with no `rom.o` and no `.incbin` of the reference ROM?

The answer for the code slice (file offsets 0x000000-0x02E158 — the whole
executable region: cartridge header, crt0/IntrMain, AgbMain, and every engine
region through the BIOS wrappers) is checked here end to end:

  1. Assemble `asm/code.s` -- a top level that includes only reconstructed
     sources.  There is no `asm/passthrough.inc` and therefore no `.incbin`; the
     closure is scanned to prove it.
  2. Read the object's undefined symbols.  Each one is a call that leaves the
     slice into code that has not been rebuilt yet.  Every such symbol must be
     recoverable from its own name (`sub_08004A2C` -> 0x08004A2C); the tool
     generates an explicit boundary stub for it so the link stays honest about
     what is still original.
  3. Reject the failure modes that would make the slice misleading: a symbol
     that resolves *inside* the slice range (a hole in coverage), a symbol
     outside the executable band, or any `.incbin` in the closure.
  4. Link at 0x08000000 and compare the emitted image with `baserom.gba` byte
     for byte over the slice range.
  5. Re-derive the header's integrity bytes from the emitted image, so a field
     edit that does not update the checksum fails here rather than silently.

`--write` refreshes the checked-in JSON/Markdown report; `--check` (what
`make independent-audit` runs) fails when the report no longer matches.
"""
from __future__ import annotations

import argparse
import difflib
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any, Iterable

ROOT = Path(__file__).resolve().parents[1]
ROM_BASE = 0x08000000
CODE_END = 0x2E158
EXPECTED_ROM_SHA256 = "f583ea5911c963774f8f26941c6cc4c5512d4586c2933fdd8829124041535c03"

AS = "arm-none-eabi-as"
LD = "arm-none-eabi-ld"
NM = "arm-none-eabi-nm"
OBJCOPY = "arm-none-eabi-objcopy"

INCLUDE_RE = re.compile(r'^\s*\.include\s+"([^"]+)"')
INCBIN_RE = re.compile(r'^\s*\.incbin\b')
# `sub_08004A2C`, `Sub_08004A2C` and `_08004A2C` are the corpus's three
# spellings of a VMA-derived symbol; `_08004A2C` may also carry leading zeros
# for 9-digit forms such as `_08002B190`.
VMA_NAME_RE = re.compile(r"^(?:_|sub_|Sub_)0*80([0-9a-fA-F]{4,7})$")

SLICE_NAME = "code"
SLICE_TOP = "asm/code.s"
SLICE_LDSCRIPT = "ldscript_slice.ld"
SLICE_START = 0x000000
SLICE_END = CODE_END

# The slice's own region map.  Adjacency is asserted so a file added to the top
# level without a matching row (or vice versa) is caught rather than reported.
SLICE_REGIONS: list[tuple[str, int, int, str]] = [
    ("asm/header.s", 0x000000, 0x0000C0, "cartridge header (fields + Nintendo logo)"),
    ("asm/boot.s", 0x0000C0, 0x0002C4, "crt0, IntrMain and IRQ hook plumbing"),
    ("asm/agbmain.s", 0x0002C4, 0x000A60, "AgbMain and boot-adjacent helpers"),
    (
        "asm/passthrough.inc",
        0x000A60,
        0x02E158,
        "every reconstructed engine region, in ROM order (generated manifest)",
    ),
]


class SliceError(RuntimeError):
    """A slice that cannot be built or would misrepresent what it proves."""


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def relpath(path: Path) -> str:
    return str(path.resolve().relative_to(ROOT.resolve()))


def run(command: list[str]) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    if result.returncode != 0:
        raise SliceError(
            "command failed: {}\n{}\n{}".format(
                " ".join(command), result.stdout.strip(), result.stderr.strip()
            )
        )
    return result


def active_lines(path: Path) -> Iterable[tuple[int, str]]:
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        stripped = line.lstrip()
        if stripped.startswith("@") or stripped.startswith("//"):
            continue
        yield number, line


def include_closure(root: Path) -> list[Path]:
    """Return the deterministic .s/.inc closure rooted at `root`."""
    pending = [root]
    seen: set[Path] = set()
    result: list[Path] = []
    while pending:
        path = pending.pop(0).resolve()
        if path in seen:
            continue
        seen.add(path)
        if not path.is_file():
            raise SliceError(f"included source does not exist: {relpath(path)}")
        result.append(path)
        for _, line in active_lines(path):
            match = INCLUDE_RE.match(line)
            if not match:
                continue
            name = match.group(1)
            candidates = [path.parent / name, ROOT / "asm" / name]
            target = next((c for c in candidates if c.is_file()), None)
            if target is None:
                raise SliceError(f"cannot resolve include {name!r} from {relpath(path)}")
            pending.append(target)
    return sorted(result, key=relpath)


def scan_for_incbin(closure: list[Path]) -> list[dict[str, Any]]:
    hits: list[dict[str, Any]] = []
    for path in closure:
        for number, line in active_lines(path):
            if INCBIN_RE.match(line):
                hits.append(
                    {"path": relpath(path), "line": number, "text": line.strip()}
                )
    return hits


def vma_from_name(name: str) -> int | None:
    match = VMA_NAME_RE.match(name)
    if not match:
        return None
    return ROM_BASE + int(match.group(1), 16)


def undefined_symbols(object_path: Path, nm: str) -> list[str]:
    result = run([nm, "-u", str(object_path)])
    names: list[str] = []
    for line in result.stdout.splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == "U":
            names.append(parts[1])
    return sorted(names)


def classify_boundary(
    names: list[str], slice_vma_start: int, slice_vma_end: int
) -> tuple[list[dict[str, Any]], list[str]]:
    """Map undefined symbols to VMAs and validate the external boundary.

    `slice_vma_start`/`slice_vma_end` are the slice's own address range, so a
    symbol that lands inside it is reported as a coverage hole rather than
    counted as a legitimate external call.

    Returns (boundary records, hard errors).  A symbol that cannot be derived
    from its name is an error rather than a silent guess: the boundary is
    evidence, and fabricated evidence is worse than none.
    """
    records: list[dict[str, Any]] = []
    errors: list[str] = []
    for name in names:
        address = vma_from_name(name)
        if address is None:
            errors.append(
                f"external symbol {name!r} does not embed a VMA and has no declared "
                f"target; add it to the slice's boundary map explicitly"
            )
            continue
        if slice_vma_start <= address < slice_vma_end:
            errors.append(
                f"external symbol {name!r} resolves to 0x{address:08X}, which is inside "
                f"the slice range 0x{slice_vma_start:08X}-0x{slice_vma_end:08X}: "
                f"the slice has a hole"
            )
            continue
        if not ROM_BASE <= address < ROM_BASE + CODE_END:
            errors.append(
                f"external symbol {name!r} resolves to 0x{address:08X}, outside the "
                f"executable band 0x{ROM_BASE:08X}-0x{ROM_BASE + CODE_END:08X}"
            )
            continue
        prefix = "sub_" if name.startswith("sub_") else ("Sub_" if name.startswith("Sub_") else "_")
        records.append(
            {
                "symbol": name,
                "address": f"0x{address:08X}",
                "spelling": prefix,
                "source": "vma-embedded-symbol-name",
            }
        )
    records.sort(key=lambda item: (int(item["address"], 16), item["symbol"]))
    return records, errors


def write_boundary_stub(path: Path, records: list[dict[str, Any]]) -> None:
    lines = [
        "@ Generated by tools/independent_slice.py -- do not edit.",
        "@",
        "@ Code outside the rebuilt slice that the slice still calls.  Each name",
        "@ is resolved to the original ROM VMA it stands for; the slice's source",
        "@ declares a symbolic branch/pool entry, so the stub keeps the emitted",
        "@ offset honest instead of hard-coding a numeric address per call site.",
        "@",
        "@ Rebuilding any of these targets shrinks this list.  When it is empty,",
        "@ the slice's code path is closed.",
        "",
    ]
    for record in records:
        # `.global` is required: a bare `.set` creates a file-local symbol that
        # the slice object cannot resolve against.
        lines.append(f"    .set {record['symbol']}, {record['address']}")
        lines.append(f"    .global {record['symbol']}")
    lines.append("")
    path.write_text("\n".join(lines), encoding="utf-8")


def assemble(
    as_: str, source: Path, inc: str, out: Path
) -> None:
    run([as_, "-mcpu=arm7tdmi", f"-I{ROOT / inc}", str(source), "-o", str(out)])


def link(ld: str, ldscript: Path, objects: list[Path], out: Path) -> None:
    run(
        [ld, "-T", str(ldscript)]
        + [str(obj) for obj in objects]
        + ["-o", str(out)]
    )


def header_facts(image: bytes) -> dict[str, Any]:
    """Re-derive the header's identity and integrity fields from the image."""
    if len(image) < 0xC0:
        raise SliceError("slice image is shorter than the cartridge header")
    complement = (-sum(image[0xA0:0xBD]) - 0x19) & 0xFF
    stored = image[0xBD]
    return {
        "title": image[0xA0:0xAC].split(b"\0", 1)[0].decode("ascii", "replace"),
        "game_code": image[0xAC:0xB0].decode("ascii", "replace"),
        "maker_code": image[0xB0:0xB2].decode("ascii", "replace"),
        "fixed_byte": f"0x{image[0xB2]:02X}",
        "software_version": f"0x{image[0xBC]:02X}",
        "complement_checksum": {
            "stored": f"0x{stored:02X}",
            "recomputed": f"0x{complement:02X}",
            "formula": "-(sum of bytes 0x0A0..0x0BC) - 0x19",
            "ok": stored == complement,
        },
        "global_checksum": {
            "stored": f"0x{image[0xBE] | (image[0xBF] << 8):04X}",
            "note": "retail image stores zero here rather than the halfword sum",
        },
    }


def region_evidence(image: bytes, rom: bytes) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for path, start, end, description in SLICE_REGIONS:
        built = image[start:end]
        reference = rom[start:end]
        rows.append(
            {
                "path": path,
                "start": f"0x{start:06X}",
                "end": f"0x{end:06X}",
                "vma_start": f"0x{ROM_BASE + start:08X}",
                "vma_end": f"0x{ROM_BASE + end:08X}",
                "size": end - start,
                "description": description,
                "sha256": sha256_bytes(built),
                "matches_baserom": built == reference,
            }
        )
    return rows


def check_region_adjacency() -> list[str]:
    errors: list[str] = []
    if not SLICE_REGIONS:
        return ["no slice regions declared"]
    if SLICE_REGIONS[0][1] != SLICE_START:
        errors.append("slice region map does not start at the slice start")
    if SLICE_REGIONS[-1][2] != SLICE_END:
        errors.append("slice region map does not end at the slice end")
    for previous, current in zip(SLICE_REGIONS, SLICE_REGIONS[1:]):
        if previous[2] != current[1]:
            errors.append(
                f"slice region map is not contiguous between {previous[0]} "
                f"(0x{previous[2]:06X}) and {current[0]} (0x{current[1]:06X})"
            )
    return errors


def build_and_verify(args: argparse.Namespace) -> dict[str, Any]:
    rom_path = (ROOT / args.rom) if not Path(args.rom).is_absolute() else Path(args.rom)
    if not rom_path.is_file():
        raise SliceError(f"reference ROM does not exist: {args.rom}")
    rom = rom_path.read_bytes()
    if hashlib.sha256(rom).hexdigest() != EXPECTED_ROM_SHA256:
        raise SliceError(
            f"{args.rom} is not the expected reference image "
            f"(SHA-256 {sha256_file(rom_path)})"
        )

    out_dir = (ROOT / args.out) if not Path(args.out).is_absolute() else Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)

    top = ROOT / SLICE_TOP
    closure = include_closure(top)

    # Structural guards run before anything is assembled: a slice that copies
    # reference bytes or that does not line up with its region map is not worth
    # linking, and failing fast keeps the diagnostic specific.
    errors = check_region_adjacency()
    incbins = scan_for_incbin(closure)
    for hit in incbins:
        errors.append(
            f"slice closure contains .incbin at {hit['path']}:{hit['line']}: "
            f"{hit['text']}"
        )
    if errors:
        raise SliceError("; ".join(errors))

    slice_object = out_dir / "code.o"
    assemble(args.as_, top, args.inc, slice_object)

    names = undefined_symbols(slice_object, args.nm)
    boundary, boundary_errors = classify_boundary(
        names, ROM_BASE + SLICE_START, ROM_BASE + SLICE_END
    )
    if boundary_errors:
        raise SliceError("; ".join(boundary_errors))

    boundary_source = out_dir / "external_boundary.s"
    write_boundary_stub(boundary_source, boundary)
    boundary_object = out_dir / "external_boundary.o"
    assemble(args.as_, boundary_source, args.inc, boundary_object)

    elf = out_dir / "code.elf"
    link(args.ld, ROOT / SLICE_LDSCRIPT, [slice_object, boundary_object], elf)

    binary = out_dir / "code.bin"
    run([args.objcopy, "-O", "binary", str(elf), str(binary)])
    image = binary.read_bytes()
    if len(image) < SLICE_END:
        raise SliceError(
            f"slice image is 0x{len(image):X} bytes, shorter than the declared "
            f"slice end 0x{SLICE_END:X}"
        )
    if len(image) > SLICE_END:
        # The declared end is CODE_END; the data tail is intentionally absent,
        # so an over-long image means the top level pulled in raw bytes.
        raise SliceError(
            f"slice image is 0x{len(image):X} bytes, longer than the declared "
            f"slice end 0x{SLICE_END:X}: the top level includes bytes it does not own"
        )
    image = image[:SLICE_END]

    built = image[SLICE_START:SLICE_END]
    reference = rom[SLICE_START:SLICE_END]
    if built != reference:
        first = next(i for i, (a, b) in enumerate(zip(built, reference)) if a != b)
        raise SliceError(
            f"slice is not byte-identical to {args.rom}: first difference at "
            f"file offset 0x{SLICE_START + first:06X} "
            f"(built 0x{built[first]:02X}, reference 0x{reference[first]:02X})"
        )

    facts = header_facts(image)
    if not facts["complement_checksum"]["ok"]:
        errors.append(
            "cartridge header complement checksum does not match the emitted fields "
            f"({facts['complement_checksum']['stored']} stored, "
            f"{facts['complement_checksum']['recomputed']} recomputed)"
        )

    report = {
        "schema": "gtadv3.independent_slice.v1",
        "generator": "tools/independent_slice.py",
        "generator_sha256": sha256_file(Path(__file__)),
        "slice": {
            "name": SLICE_NAME,
            "top_level": SLICE_TOP,
            "linker_script": SLICE_LDSCRIPT,
            "start": f"0x{SLICE_START:06X}",
            "end": f"0x{SLICE_END:06X}",
            "vma_start": f"0x{ROM_BASE + SLICE_START:08X}",
            "vma_end": f"0x{ROM_BASE + SLICE_END:08X}",
            "bytes": SLICE_END - SLICE_START,
            "sha256": sha256_bytes(built),
            "matches_baserom": built == reference,
            "incbin_count": len(incbins),
            "external_symbol_count": len(boundary),
        },
        "rom": {
            "path": args.rom,
            "sha256": EXPECTED_ROM_SHA256,
            "code_end": f"0x{CODE_END:06X}",
        },
        "source_closure": [
            {"path": relpath(path), "sha256": sha256_file(path)} for path in closure
        ],
        "regions": region_evidence(image, rom),
        "header": facts,
        "external_boundary": boundary,
        "errors": errors,
    }
    if errors:
        raise SliceError("; ".join(errors))
    return report


def render_markdown(report: dict[str, Any]) -> str:
    slice_info = report["slice"]
    facts = report["header"]
    lines = [
        f"# Independent link slice: {slice_info['name']}",
        "",
        "Generated by `tools/independent_slice.py`; machine-readable evidence is",
        "in `docs/data/independent_slice.json`.",
        "",
        "This slice is the **entire executable region** of the cartridge, produced",
        "from reconstructed source **without** linking `build/rom.o` and **without**",
        "a single `.incbin` of `baserom.gba`.  `asm/code.s` includes only",
        "`asm/header.s`, `asm/boot.s`, `asm/agbmain.s` and the generated region",
        "manifest `asm/passthrough.inc`; the cataloged data/asset tail lives in a",
        "separate file (`asm/data_tail.s`) that this top level never includes.",
        "",
        "`asm/code.s` is the *same* top level the reference `make` build and the",
        "independent slice consume, so the code region has one definition rather than",
        "two copies that could drift apart.",
        "",
        "## Result",
        "",
        "| Measure | Value |",
        "|---|---|",
        f"| Owned range | `{slice_info['start']}..{slice_info['end']}` "
        f"({slice_info['vma_start']}..{slice_info['vma_end']}) |",
        f"| Slice size | `{slice_info['bytes']}` bytes |",
        f"| Byte-identical to `baserom.gba` | `{'yes' if slice_info['matches_baserom'] else 'no'}` |",
        f"| Slice SHA-256 | `{slice_info['sha256']}` |",
        f"| `.incbin` in closure | `{slice_info['incbin_count']}` |",
        f"| External boundary symbols | `{slice_info['external_symbol_count']}` |",
        f"| Included source files | `{len(report['source_closure'])}` |",
        "",
        "The build is: assemble `asm/code.s` (which includes only",
        "reconstructed sources), resolve its out-of-slice calls from the generated",
        "boundary stub, link with `ldscript_slice.ld` at `0x08000000`, and compare",
        "the emitted image with the reference over the owned range.",
        "",
        "## Slice composition",
        "",
        "| Source | File offsets | VMA range | Size | Bytes match | Purpose |",
        "|---|---|---|---:|---|---|",
    ]
    for row in report["regions"]:
        match = "yes" if row["matches_baserom"] else "**no**"
        lines.append(
            f"| `{row['path']}` | `{row['start']}..{row['end']}` | "
            f"`{row['vma_start']}..{row['vma_end']}` | `{row['size']}` | {match} | "
            f"{row['description']} |"
        )

    lines.extend(
        [
            "",
            "## Cartridge header, re-derived from the emitted bytes",
            "",
            "| Field | Value |",
            "|---|---|",
            f"| Title | `{facts['title']}` |",
            f"| Game code | `{facts['game_code']}` |",
            f"| Maker code | `{facts['maker_code']}` |",
            f"| Fixed byte | `{facts['fixed_byte']}` |",
            f"| Software version | `{facts['software_version']}` |",
            f"| Header complement checksum | stored `{facts['complement_checksum']['stored']}`, "
            f"recomputed `{facts['complement_checksum']['recomputed']}` "
            f"(`{facts['complement_checksum']['formula']}`) |",
            f"| Global checksum | `{facts['global_checksum']['stored']}` — "
            f"{facts['global_checksum']['note']} |",
            "",
            "The complement checksum is recomputed from the assembled image, so a",
            "field edit that forgets to update `asm/header.s` fails the gate.",
            "",
            "## External boundary (still original code)",
            "",
        ]
    )
    if report["external_boundary"]:
        lines.extend(
            [
                "Every entry below is a call that leaves the rebuilt slice.  They are the",
                "next work items; the slice's own code path is otherwise closed.",
                "",
                "| Symbol | Target VMA |",
                "|---|---|",
            ]
        )
        for record in report["external_boundary"]:
            lines.append(f"| `{record['symbol']}` | `{record['address']}` |")
    else:
        lines.extend(
            [
                "**No out-of-slice references remain.**  Every branch, vector and",
                "function-pointer pool word in the executable region resolves to",
                "rebuilt source, so nothing in the code path leaves the rebuilt",
                "region.",
                "",
                "Caveat: this counts *code* references.  Literal pools still embed",
                "absolute data addresses (catalog tables, sprite templates, the",
                "course package) as constants.  Those are data inputs, not code",
                "delegation, and they are the subject of the data/asset inventory",
                "work rather than of this gate.",
            ]
        )
    lines.extend(
        [
            "",
            "## What this does and does not prove",
            "",
            f"* Proves: these {slice_info['bytes']} bytes can be emitted from reconstructed source with",
            "  no original executable bytes mixed in, and they are byte-identical to",
            "  the reference.",
            "* Proves: the slice's out-of-slice references are a *known, enumerated*",
            "  boundary rather than an invisible dependency on `rom.o`.",
            "* Does not prove: the data/asset tail (`0x02E158`-`0x7B04C4`) is still a",
            "  raw input; cataloging it into named data is the remaining data-gate work.",
            "",
        ]
    )
    return "\n".join(lines)


def self_test() -> None:
    assert vma_from_name("sub_08004A2C") == 0x08004A2C
    assert vma_from_name("_080016EC") == 0x080016EC
    assert vma_from_name("Sub_0802DDC8") == 0x0802DDC8
    assert vma_from_name("_08002B190") == 0x0802B190
    assert vma_from_name("AgbMain") is None
    assert vma_from_name("sub_0800") is None
    # Use a narrow window so the boundary rules are exercised independently of
    # how far the real slice currently reaches.
    window_start, window_end = ROM_BASE, ROM_BASE + 0x10
    records, errors = classify_boundary(["sub_08004A2C"], window_start, window_end)
    assert not errors and records[0]["address"] == "0x08004A2C"
    _, errors = classify_boundary(["_08000008"], window_start, window_end)
    assert errors and "inside the slice range" in errors[0]
    _, errors = classify_boundary(["sub_0807FFFF"], window_start, window_end)
    assert errors and "executable band" in errors[0]
    _, errors = classify_boundary(["SomeHelper"], window_start, window_end)
    assert errors and "does not embed a VMA" in errors[0]
    assert SLICE_END == CODE_END, "the slice is meant to cover the whole code region"
    assert not check_region_adjacency()
    print("independent_slice self-test: PASS")


def compare_report(path: Path, expected: str, label: str, errors: list[str]) -> None:
    if not path.is_file():
        errors.append(f"missing {label}: {relpath(path)}")
        return
    actual = path.read_text(encoding="utf-8")
    if actual == expected:
        return
    errors.append(f"{label} differs from the generated slice report: {relpath(path)}")
    diff = difflib.unified_diff(
        expected.splitlines(), actual.splitlines(), fromfile="generated", tofile=relpath(path), lineterm=""
    )
    for line in list(diff)[:40]:
        print(line)
    print("... slice report diff truncated ...")


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", default="baserom.gba", help="reference ROM (default: baserom.gba)")
    parser.add_argument("--out", default="build-slice", help="build directory (default: build-slice)")
    parser.add_argument("--inc", default="asm", help="assembler include directory (default: asm)")
    parser.add_argument("--as", dest="as_", default=AS)
    parser.add_argument("--ld", default=LD)
    parser.add_argument("--nm", default=NM)
    parser.add_argument("--objcopy", default=OBJCOPY)
    parser.add_argument("--json", default="docs/data/independent_slice.json")
    parser.add_argument("--markdown", default="docs/independent_slice.md")
    mode = parser.add_mutually_exclusive_group(required=False)
    mode.add_argument("--write", action="store_true", help="write the checked-in slice reports")
    mode.add_argument("--check", action="store_true", help="compare generated reports with checked-in reports")
    parser.add_argument("--self-test", action="store_true", help="run policy self-tests and exit")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv or sys.argv[1:])
    if args.self_test:
        self_test()
        return 0
    if not args.write and not args.check:
        print("independent-slice: ERROR: choose --write or --check", file=sys.stderr)
        return 2
    try:
        report = build_and_verify(args)
    except (OSError, SliceError) as error:
        print(f"independent-slice: ERROR: {error}", file=sys.stderr)
        return 1

    report_text = json.dumps(report, indent=2, sort_keys=True) + "\n"
    markdown_text = render_markdown(report)
    json_path = ROOT / args.json
    markdown_path = ROOT / args.markdown
    errors: list[str] = []
    if args.write:
        json_path.parent.mkdir(parents=True, exist_ok=True)
        markdown_path.parent.mkdir(parents=True, exist_ok=True)
        json_path.write_text(report_text, encoding="utf-8")
        markdown_path.write_text(markdown_text, encoding="utf-8")
        print(f"wrote {relpath(json_path)} and {relpath(markdown_path)}")
    else:
        compare_report(json_path, report_text, "JSON report", errors)
        compare_report(markdown_path, markdown_text, "Markdown report", errors)

    slice_info = report["slice"]
    print(
        "independent-slice: "
        f"{slice_info['name']} 0x{SLICE_START:06X}-0x{SLICE_END:06X} "
        f"({slice_info['bytes']} bytes) byte-identical={slice_info['matches_baserom']} "
        f"incbins={slice_info['incbin_count']} "
        f"external={slice_info['external_symbol_count']}"
    )
    if errors:
        for error in errors:
            print(f"independent-slice: ERROR: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
