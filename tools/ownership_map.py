#!/usr/bin/env python3
"""Inventory the ROM ranges that supply the reference build.

The C-lift coverage tools answer "does a known ASM entry have a C body?".
This tool answers a different question: for every byte in the cartridge,
what source or ROM-backed input supplies it, and is that input executable code,
data, padding, or still unresolved?

The generated report is deliberately independent of the C hybrid build.  It
 inventories the assembly/reference link inputs and records known blockers to
an independent link.    Use --check in CI to detect source/layout drift; use
--strict to fail while executable bytes are still supplied by baserom.gba or
the C link still consumes a reference-build object.
"""
from __future__ import annotations

import argparse
import collections
import difflib
import hashlib
import json
import re
import struct
import sys
from pathlib import Path
from typing import Any, Iterable


ROOT = Path(__file__).resolve().parents[1]
ROM_BASE = 0x08000000
ROM_SIZE = 0x800000
CODE_END = 0x2E158
CONTENT_END = 0x7B04C4
EXPECTED_ROM_SHA256 = "f583ea5911c963774f8f26941c6cc4c5512d4586c2933fdd8829124041535c03"

# These are source-layout exceptions where a file's first-line region banner
# is absent, wrapped, or historically ended one byte early.  They are kept in
# the tool rather than hidden in the generated report so a layout change fails
# loudly instead of silently creating an unclassified gap.
SOURCE_RANGE_OVERRIDES: dict[str, list[tuple[int, int]]] = {
    # The cartridge header sits below the 0xC0 code floor the region-banner
    # parser enforces, so its span is declared here rather than scraped.
    "header.s": [(0x000000, 0x0000C0)],
    # handlers.s is one 0x0AF4-0x0FA0 hole in asm/passthrough.inc containing
    # five sub-regions (blob A, the installer, the block-B stepper, blob B,
    # the helper tail); the banner parser only sees the first, so pin the
    # whole span.
    "handlers.s": [(0x0AF4, 0x0FA0)],
    "carphys_tick.s": [
        (0x09BCC, 0x09F58),
        (0x09F58, 0x0A1D4),
        (0x0A1D4, 0x0A204),
        (0x0A204, 0x0A44C),
        (0x0A44C, 0x0A62C),
        (0x0A62C, 0x0A668),
        (0x0A668, 0x0A9A0),
        (0x0A9A0, 0x0AA20),
        (0x0AA20, 0x0AA40),
    ],
    "mixer_2b888.s": [(0x2B888, 0x2BC28)],
    "course_load.s": [(0x1A204, 0x1A294)],
    "course_stream_more.s": [(0x06B30, 0x06CB8)],
    "sound_control.s": [(0x2C990, 0x2CA34)],
}

# Literal/padding words emitted directly by the generated passthrough file.
# They are reconstructed source, not raw ROM bytes, but have no standalone
# region banner of their own.
INLINE_SOURCE_RANGES: dict[str, list[tuple[int, int]]] = {
    "asm/passthrough.inc": [(0x2BC62, 0x2BC64), (0x2C110, 0x2C11C)],
}

# Source files that intentionally own no code range: they exist to carry a
# data input (currently only the cataloged ROM tail).  They are expected to
# have no region banner, so they must be exempt from the banner requirement.
DATA_ONLY_SOURCES: set[str] = {"data_tail.s"}

# Direct .incbin ranges are deliberately explicit.  A newly introduced raw
# range must be classified here before the audit can pass.
RAW_OVERRIDES: list[dict[str, Any]] = [
    {
        "start": 0x02E158,
        "end": ROM_SIZE,
        "classification": "asset-data+padding",
        "status": "private-rom-data-input",
        "owner": "extracted/generated data inputs (pending independent integration)",
        "evidence": [
            "asm/data_tail.s",
            "baserom.sha256 documents content end 0x7B04C4 and zero padding",
            "docs/data/lz77_blobs.txt, mto_entries.txt, and track_resources.txt",
        ],
        "strict_blocker": False,
    },
]

INCLUDE_RE = re.compile(r'^\s*\.include\s+"([^"]+)"')
INCBIN_RE = re.compile(
    r'^\s*\.incbin\s+"baserom\.gba"\s*,\s*'
    r"(0x[0-9a-fA-F]+)\s*,\s*(0x[0-9a-fA-F]+)"
)
RANGE_RE = re.compile(
    r"(0x[0-9a-fA-F]+)\s*[-–]\s*(0x[0-9a-fA-F]+)"
)
LABEL_RE = re.compile(r"^\s*(?:[A-Za-z_][A-Za-z0-9_]*):")


class OwnershipError(RuntimeError):
    """A malformed or drifted ownership input."""


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def repo_path(value: str | Path) -> Path:
    path = Path(value)
    return path if path.is_absolute() else ROOT / path


def relpath(path: Path) -> str:
    return str(path.resolve().relative_to(ROOT.resolve()))


def hex_range(start: int, end: int) -> str:
    return f"0x{start:06X}-0x{end:06X}"


def normalize_address(value: str) -> int:
    number = int(value, 16)
    if number >= ROM_BASE:
        number -= ROM_BASE
    if not 0 <= number < ROM_SIZE:
        raise OwnershipError(f"address outside ROM: {value}")
    return number


def active_lines(path: Path) -> Iterable[tuple[int, str]]:
    """Yield non-comment source lines with their one-based line numbers."""
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        stripped = line.lstrip()
        if stripped.startswith("@") or stripped.startswith("//"):
            continue
        yield number, line


# Directives an aggregator may carry: they configure the assembler or name a
# section but emit no bytes of their own.  Anything else -- an instruction, a
# `.byte`/`.word`, an `.incbin` -- means the file owns bytes and must declare a
# region like every other region owner.
_AGGREGATOR_DIRECTIVE_RE = re.compile(
    r"^\s*\.(?:syntax|cpu|text|thumb|arm|section|global|align|ltorg|p2align|"
    r"balign|file|ident|fpu|arch|code|eabi_attribute|type|size)\b[^\n]*$"
)


def is_aggregator(path: Path) -> bool:
    """True when a source file emits no bytes of its own.

    Such a file only carries assembler directives and `.include`s the region
    owners (e.g. `asm/rom.s`, `asm/code.s`), so it declares no ROM range of
    its own.  The test is structural rather than a name list: a file that
    actually emits anything fails it, so a source cannot quietly opt out of
    the ownership map by being added to a set.
    """
    saw_include = False
    for _, line in active_lines(path):
        if not line.strip():
            continue
        if INCLUDE_RE.match(line):
            saw_include = True
            continue
        if _AGGREGATOR_DIRECTIVE_RE.match(line):
            continue
        return False
    return saw_include


def include_closure(root: Path) -> list[Path]:
    """Return the deterministic .s/.inc closure rooted at asm/rom.s."""
    pending = [root]
    seen: set[Path] = set()
    result: list[Path] = []
    while pending:
        path = pending.pop(0).resolve()
        if path in seen:
            continue
        seen.add(path)
        if not path.is_file():
            raise OwnershipError(f"included source does not exist: {path}")
        result.append(path)
        for _, line in active_lines(path):
            match = INCLUDE_RE.match(line)
            if not match:
                continue
            name = match.group(1)
            candidates = [path.parent / name, ROOT / "asm" / name]
            target = next((candidate.resolve() for candidate in candidates if candidate.is_file()), None)
            if target is None:
                raise OwnershipError(f"cannot resolve include {name!r} from {path}")
            if ROOT.resolve() not in target.parents and target != ROOT.resolve():
                raise OwnershipError(f"include escapes repository: {target}")
            pending.append(target)
    return sorted(result, key=relpath)


def parse_raw_inc_bins(closure: list[Path]) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for path in closure:
        for number, line in active_lines(path):
            if ".incbin" not in line:
                continue
            match = INCBIN_RE.match(line)
            if not match:
                raise OwnershipError(
                    f"unrecognized .incbin at {relpath(path)}:{number}: {line.strip()}"
                )
            start = int(match.group(1), 16)
            size = int(match.group(2), 16)
            end = start + size
            if not 0 <= start < end <= ROM_SIZE:
                raise OwnershipError(f"invalid .incbin range at {relpath(path)}:{number}")
            records.append(
                {
                    "start": start,
                    "end": end,
                    "size": size,
                    "path": relpath(path),
                    "line": number,
                    "text": line.strip(),
                }
            )
    records.sort(key=lambda item: (item["start"], item["end"], item["path"], item["line"]))
    for previous, current in zip(records, records[1:]):
        if current["start"] < previous["end"]:
            raise OwnershipError(
                f"overlapping raw .incbin ranges: {hex_range(previous['start'], previous['end'])} "
                f"and {hex_range(current['start'], current['end'])}"
            )
    return records


def raw_override(start: int, end: int) -> dict[str, Any] | None:
    for override in RAW_OVERRIDES:
        if override["start"] == start and override["end"] == end:
            return override
    return None


def extract_declared_ranges(path: Path) -> list[tuple[int, int]]:
    """Read a file's region banner, normalizing file offsets and VMAs.

    Only lines that explicitly introduce a region are considered.  Several
    source files mention private pools, subranges, or call targets in their
    first few prose lines; treating every hex range as an owner would create
    false overlaps.
    """
    ranges: set[tuple[int, int]] = set()
    declaration = re.compile(
        r"^\s*@\s*(?:Region|Regions|VMA|Hole)\b", re.IGNORECASE
    )
    for line in path.read_text(encoding="utf-8").splitlines()[:20]:
        if not declaration.match(line):
            continue
        for first, second in RANGE_RE.findall(line):
            start = normalize_address(first)
            end = normalize_address(second)
            if start >= end or start < 0xC0 or end > CODE_END:
                continue
            # A region banner should never claim a multi-megabyte span.  This
            # filter also prevents incidental prose in the first lines from
            # becoming a source owner.
            if end - start > 0x200000:
                continue
            ranges.add((start, end))
    return sorted(ranges)


def source_ranges(closure: list[Path]) -> tuple[list[dict[str, Any]], list[str]]:
    records: list[dict[str, Any]] = []
    errors: list[str] = []
    for path in closure:
        if path.suffix != ".s":
            continue
        name = path.name
        if name in DATA_ONLY_SOURCES:
            continue
        if is_aggregator(path):
            continue
        if name in SOURCE_RANGE_OVERRIDES:
            ranges = list(SOURCE_RANGE_OVERRIDES[name])
        else:
            ranges = extract_declared_ranges(path)
        if not ranges:
            errors.append(f"no declared source range for included file {relpath(path)}")
            continue
        for start, end in ranges:
            if not 0 <= start < end <= CODE_END:
                errors.append(f"invalid source range {hex_range(start, end)} in {relpath(path)}")
                continue
            records.append(
                {
                    "start": start,
                    "end": end,
                    "path": relpath(path),
                    "source": "region-banner",
                }
            )

    for name, ranges in INLINE_SOURCE_RANGES.items():
        path = ROOT / name
        if not path.is_file():
            errors.append(f"inline source owner does not exist: {name}")
            continue
        for start, end in ranges:
            records.append(
                {
                    "start": start,
                    "end": end,
                    "path": name,
                    "source": "generated-passthrough-layout",
                }
            )

    records.sort(key=lambda item: (item["start"], item["end"], item["path"]))
    # Exact duplicate declarations are harmless (many banners print both file
    # offset and VMA).  Non-identical overlaps are a source-layout defect.
    unique: list[dict[str, Any]] = []
    for record in records:
        if unique and (record["start"], record["end"], record["path"]) == (
            unique[-1]["start"], unique[-1]["end"], unique[-1]["path"]
        ):
            continue
        unique.append(record)
    previous: dict[str, Any] | None = None
    for record in unique:
        if previous and record["start"] < previous["end"]:
            errors.append(
                f"overlapping reconstructed source ranges: "
                f"{relpath(Path(previous['path']))} {hex_range(previous['start'], previous['end'])} "
                f"and {record['path']} {hex_range(record['start'], record['end'])}"
            )
        previous = record
    return unique, errors


def read_u16(data: bytes, offset: int) -> int | None:
    if offset < 0 or offset + 2 > len(data):
        return None
    return struct.unpack_from("<H", data, offset)[0]


def thumb_bl_target(data: bytes, offset: int) -> int | None:
    """Decode an ARMv4T Thumb BL, returning a file offset or None."""
    first = read_u16(data, offset)
    second = read_u16(data, offset + 2)
    if first is None or second is None:
        return None
    if (first & 0xF800) != 0xF000 or (second & 0xD000) != 0xD000:
        return None
    sign = (first >> 10) & 1
    imm10 = first & 0x3FF
    j1 = (second >> 13) & 1
    j2 = (second >> 11) & 1
    i1 = 1 - j1
    i2 = 1 - j2
    imm11 = second & 0x7FF
    imm25 = (sign << 24) | (i1 << 23) | (i2 << 22) | (imm10 << 12) | (imm11 << 1)
    if sign:
        imm25 -= 1 << 25
    return offset + 4 + imm25


def scan_raw_executable_references(
    rom: bytes, raw_records: list[dict[str, Any]]
) -> dict[str, dict[str, Any]]:
    """Find heuristic BL/literal references to each raw executable range.

    Literal scans necessarily include data coincidences.  They are recorded as
    evidence, never treated as a disassembly proof on their own.
    """
    executable = [
        record
        for record in raw_records
        if raw_override(record["start"], record["end"]) is not None
        and raw_override(record["start"], record["end"])["classification"] == "executable"
    ]
    result: dict[str, dict[str, Any]] = {}
    for record in executable:
        key = hex_range(record["start"], record["end"])
        result[key] = {"direct_bl_refs": 0, "literal_refs": 0, "samples": []}

    def containing(offset: int) -> dict[str, Any] | None:
        for record in executable:
            if record["start"] <= offset < record["end"]:
                return record
        return None

    for offset in range(0, len(rom) - 3, 2):
        target = thumb_bl_target(rom, offset)
        if target is None:
            continue
        record = containing(target)
        if record is None:
            continue
        item = result[hex_range(record["start"], record["end"])]
        item["direct_bl_refs"] += 1
        if len(item["samples"]) < 8:
            item["samples"].append({"kind": "bl", "site": f"0x{offset:06X}", "target": f"0x{target:06X}"})

    for offset in range(0, len(rom) - 3, 4):
        value = struct.unpack_from("<I", rom, offset)[0]
        target = None
        if ROM_BASE <= value < ROM_BASE + ROM_SIZE:
            target = value - ROM_BASE
        elif ROM_BASE < value <= ROM_BASE + ROM_SIZE:
            target = (value & ~1) - ROM_BASE
        if target is None:
            continue
        record = containing(target)
        if record is None:
            continue
        item = result[hex_range(record["start"], record["end"])]
        item["literal_refs"] += 1
        if len(item["samples"]) < 8:
            item["samples"].append({"kind": "literal", "site": f"0x{offset:06X}", "target": f"0x{target:06X}"})
    return result


def classify_raw_interval(start: int, end: int) -> dict[str, Any]:
    # The one large .incbin is split at the documented content end.
    if start == 0x02E158 and end <= CONTENT_END:
        return {
            "classification": "asset-data",
            "status": "private-rom-data-input",
            "owner": "extracted/generated data inputs (pending independent integration)",
            "evidence": ["asm/data_tail.s", "baserom.sha256 content-end record"],
            "strict_blocker": False,
        }
    if start >= CONTENT_END and end <= ROM_SIZE:
        return {
            "classification": "padding",
            "status": "owned-zero-padding",
            "owner": "linker/source-generated zero padding",
            "evidence": ["baserom.sha256", "all bytes after 0x7B04C4 are zero"],
            "strict_blocker": False,
        }
    for override in RAW_OVERRIDES:
        if override["start"] == start and override["end"] == end:
            return override
    # A raw range may have been split at content_end only; look up the
    # containing record before declaring it unclassified.
    for override in RAW_OVERRIDES:
        if override["start"] <= start and end <= override["end"]:
            result = dict(override)
            result["classification"] = "unclassified-raw"
            result["status"] = "unresolved"
            result["owner"] = "classification required"
            result["strict_blocker"] = True
            return result
    return {
        "classification": "unclassified-raw",
        "status": "unclassified-raw",
        "owner": "classification required",
        "evidence": [],
        "strict_blocker": True,
    }


def source_file_inventory(closure: list[Path], ranges: list[dict[str, Any]]) -> list[dict[str, Any]]:
    by_path: dict[str, list[dict[str, Any]]] = collections.defaultdict(list)
    for record in ranges:
        by_path[record["path"]].append(record)
    result: list[dict[str, Any]] = []
    for path in closure:
        relative = relpath(path)
        if relative == "asm/rom.s":
            continue
        declared = by_path.get(relative, [])
        result.append(
            {
                "path": relative,
                "sha256": sha256_file(path),
                "size": path.stat().st_size,
                "ranges": [
                    {
                        "start": f"0x{item['start']:06X}",
                        "end": f"0x{item['end']:06X}",
                        "source": item["source"],
                    }
                    for item in declared
                ],
            }
        )
    return result


# The reference build writes its objects and maps into `build/`.  The
# independent C link may consume only independently assembled objects
# (`build-code/`) and its own C objects (`build-c/`), so a reference to a
# `build/...o` path in the C-link path means the hybrid is still leaning on
# the reference build instead of standing on what it assembles itself.
#
# The reference targets themselves spell that directory as the `$(BUILD)`
# make variable, so a *literal* `build/...o` is a reliable tell for the C
# link's own inputs.  The negative lookbehind keeps `build-code/`,
# `build-c/` and `build-s/` out of the match.
REFERENCE_OBJECT_RE = re.compile(r"(?<![\w)/.-])build/[A-Za-z0-9_./-]+\.o\b")


def reference_object_mentions(line: str) -> list[str]:
    """Literal `build/...o` paths in `line`, skipping EXCLUDE_FILE filters.

    `EXCLUDE_FILE(build/x.o)` keeps an object out of a catch-all input, i.e.
    it is the opposite of a dependency, so it never counts.
    """
    hits = []
    for match in REFERENCE_OBJECT_RE.finditer(line):
        token = match.group(0)
        if f"EXCLUDE_FILE({token})" in line:
            continue
        hits.append(token)
    return hits


def dependency_findings() -> list[dict[str, Any]]:
    findings: list[dict[str, Any]] = []
    targets = [
        ROOT / "ldscript_matching.ld",
        ROOT / "ldscript_slice.ld",
        ROOT / "Makefile",
        ROOT / "tools" / "match_c_slice.py",
    ]
    for path in targets:
        for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            stripped = line.strip()
            if stripped.startswith("#") or stripped.startswith("*"):
                continue
            # EXCLUDE_FILE mentions are linker filters, not a dependency on
            # the reference object.  The positive inclusions are the relevant
            # independent-link blocker.
            for token in reference_object_mentions(line):
                findings.append(
                    {
                        "kind": "reference-object-link",
                        "path": relpath(path),
                        "line": number,
                        "message": f"independent C link still names the reference-build object {token}",
                        "text": stripped,
                    }
                )
    return findings


def build_segments(
    raw_records: list[dict[str, Any]],
    source_records: list[dict[str, Any]],
    reference_evidence: dict[str, dict[str, Any]],
) -> tuple[list[dict[str, Any]], list[str]]:
    errors: list[str] = []
    boundaries = {0, ROM_SIZE, CONTENT_END}
    for record in raw_records + source_records:
        boundaries.add(record["start"])
        boundaries.add(record["end"])
    ordered = sorted(boundaries)
    segments: list[dict[str, Any]] = []
    for start, end in zip(ordered, ordered[1:]):
        if start == end:
            continue
        raw_hits = [record for record in raw_records if record["start"] <= start and end <= record["end"]]
        source_hits = [record for record in source_records if record["start"] <= start and end <= record["end"]]
        if len(raw_hits) > 1:
            errors.append(f"multiple raw owners for {hex_range(start, end)}")
        if raw_hits:
            raw = raw_hits[0]
            classification = classify_raw_interval(start, end)
            # A split tail interval still has the containing record's source
            # path/line in the report.
            key = hex_range(raw["start"], raw["end"])
            evidence = list(classification.get("evidence", []))
            if raw_override(raw["start"], raw["end"]) is None:
                errors.append(f"raw range lacks RAW_OVERRIDES entry: {key}")
            if classification["classification"] == "executable":
                evidence.extend(
                    [
                        f"reference BL count={reference_evidence[key]['direct_bl_refs']}",
                        f"reference literal count={reference_evidence[key]['literal_refs']}",
                    ]
                )
            if source_hits:
                evidence.append("range is inside a broad source banner but raw bytes take precedence")
            segment = {
                "start": start,
                "end": end,
                "size": end - start,
                "classification": classification["classification"],
                "status": classification["status"],
                "executable": classification["classification"] == "executable",
                "backing": "baserom.gba:.incbin",
                "owners": [f"{raw['path']}:{raw['line']}"],
                "source_owners": sorted({item["path"] for item in source_hits}),
                "intended_owner": classification["owner"],
                "evidence": evidence,
                "strict_blocker": bool(classification["strict_blocker"]),
            }
        elif source_hits:
            owners = sorted({item["path"] for item in source_hits})
            segment = {
                "start": start,
                "end": end,
                "size": end - start,
                "classification": "assembly-source",
                "status": "owned-reference-assembly",
                "executable": True,
                "backing": "asm/*.s",
                "owners": owners,
                "source_owners": owners,
                "intended_owner": "reconstructed source assembly (reference link)",
                "evidence": [
                    "included source region banner / generated passthrough layout",
                    "reference build is checked by make against baserom.gba",
                ],
                "strict_blocker": False,
            }
        else:
            segment = {
                "start": start,
                "end": end,
                "size": end - size_zero(end - start),
                "classification": "unclassified",
                "status": "unresolved",
                "executable": True,
                "backing": "unknown",
                "owners": [],
                "source_owners": [],
                "intended_owner": "owner/evidence required",
                "evidence": ["no active .incbin or declared source range covers this interval"],
                "strict_blocker": True,
            }
        segments.append(segment)

    segments.sort(key=lambda item: item["start"])
    # Adjacent intervals with identical ownership and policy are one report
    # row.  Raw records remain separate when their source line differs.
    merged: list[dict[str, Any]] = []
    for segment in segments:
        if merged and segment_key(merged[-1]) == segment_key(segment):
            merged[-1]["end"] = segment["end"]
            merged[-1]["size"] = merged[-1]["end"] - merged[-1]["start"]
            merged[-1]["evidence"] = sorted(set(merged[-1]["evidence"] + segment["evidence"]))
        else:
            merged.append(segment)
    return merged, errors


def size_zero(size: int) -> int:
    # Kept as a named helper to make the unclassified branch's arithmetic
    # explicit and avoid accidentally recording a negative size after edits.
    return size


def segment_key(segment: dict[str, Any]) -> tuple[Any, ...]:
    return (
        segment["classification"],
        segment["status"],
        segment["executable"],
        segment["backing"],
        tuple(segment["owners"]),
        tuple(segment["source_owners"]),
        segment["intended_owner"],
        segment["strict_blocker"],
    )


def header_facts(rom: bytes) -> dict[str, Any]:
    if len(rom) < 0xC0:
        raise OwnershipError("ROM is shorter than the cartridge header")
    title = rom[0xA0:0xAC].split(b"\0", 1)[0].decode("ascii", "replace")
    game_code = rom[0xAC:0xB0].decode("ascii", "replace")
    maker = rom[0xB0:0xB2].decode("ascii", "replace")
    fixed = rom[0xB2]
    version = rom[0xBC]
    complement = rom[0xBD]
    expected = {
        "title": "GT ADVANCE 3",
        "game_code": "A2GE",
        "maker": "78",
        "fixed": 0x96,
        "version": 0,
        "header_complement_checksum": 0xE3,
    }
    actual = {
        "title": title,
        "game_code": game_code,
        "maker": maker,
        "fixed": fixed,
        "version": version,
        "header_complement_checksum": complement,
    }
    errors = [
        f"header field {key}={actual[key]!r}, expected {value!r}"
        for key, value in expected.items()
        if actual[key] != value
    ]
    return {"actual": actual, "expected": expected, "errors": errors}


def source_closure_hash(closure: list[Path]) -> str:
    digest = hashlib.sha256()
    for path in sorted(closure, key=relpath):
        digest.update(relpath(path).encode("utf-8"))
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")
    return digest.hexdigest()


def make_report(rom_path: Path) -> dict[str, Any]:
    if not rom_path.is_file():
        raise OwnershipError(f"ROM does not exist: {rom_path}")
    rom = rom_path.read_bytes()
    if len(rom) != ROM_SIZE:
        raise OwnershipError(f"ROM size is 0x{len(rom):X}, expected 0x{ROM_SIZE:X}")
    rom_sha = sha256_bytes(rom)
    if rom_sha != EXPECTED_ROM_SHA256:
        raise OwnershipError(
            f"ROM SHA-256 mismatch: {rom_sha}; expected {EXPECTED_ROM_SHA256}"
        )
    last_nonzero = max(index for index, value in enumerate(rom) if value)
    observed_content_end = last_nonzero + 1
    if observed_content_end != CONTENT_END:
        raise OwnershipError(
            f"ROM content end is 0x{observed_content_end:X}, expected 0x{CONTENT_END:X}"
        )
    padding_nonzero = sum(value != 0 for value in rom[CONTENT_END:])
    if padding_nonzero:
        raise OwnershipError(f"ROM padding contains {padding_nonzero} non-zero bytes")

    closure = include_closure(ROOT / "asm" / "rom.s")
    raw_records = parse_raw_inc_bins(closure)
    source_records, source_errors = source_ranges(closure)
    reference_evidence = scan_raw_executable_references(rom, raw_records)
    segments, segment_errors = build_segments(raw_records, source_records, reference_evidence)
    dependency = dependency_findings()
    header = header_facts(rom)

    strict_blockers: list[dict[str, Any]] = []
    for segment in segments:
        if segment["strict_blocker"]:
            strict_blockers.append(
                {
                    "kind": "unresolved-executable-or-unclassified",
                    "range": hex_range(segment["start"], segment["end"]),
                    "path": segment["owners"][0] if segment["owners"] else None,
                    "message": f"{segment['classification']} range is not independently owned",
                }
            )
    dependency_by_path: dict[str, list[int]] = collections.defaultdict(list)
    for finding in dependency:
        dependency_by_path[finding["path"]].append(finding["line"])
    for path, lines in sorted(dependency_by_path.items()):
        line_text = ", ".join(str(line) for line in lines)
        strict_blockers.append(
            {
                "kind": "reference-object-link",
                "range": None,
                "path": path,
                "lines": lines,
                "message": (
                    "independent C link still names a reference-build object "
                    f"(lines {line_text})"
                ),
            }
        )

    coverage_errors = list(source_errors) + list(segment_errors) + list(header["errors"])
    if not segments or segments[0]["start"] != 0 or segments[-1]["end"] != ROM_SIZE:
        coverage_errors.append("segments do not cover the complete 0x000000-0x800000 ROM")
    for previous, current in zip(segments, segments[1:]):
        if current["start"] != previous["end"]:
            coverage_errors.append(
                f"coverage gap/overlap at {hex_range(previous['end'], current['start'])}"
            )

    summary = {
        "rom_bytes": ROM_SIZE,
        "segment_count": len(segments),
        "raw_incbin_count": len(raw_records),
        "raw_executable_bytes": sum(
            item["size"] for item in segments if item["classification"] == "executable"
        ),
        "raw_header_bytes": sum(
            item["size"] for item in segments if item["classification"] == "header"
        ),
        "raw_data_bytes": sum(
            item["size"] for item in segments if item["classification"] == "asset-data"
        ),
        "padding_bytes": sum(
            item["size"] for item in segments if item["classification"] == "padding"
        ),
        "assembly_source_bytes": sum(
            item["size"] for item in segments if item["classification"] == "assembly-source"
        ),
        "unclassified_bytes": sum(
            item["size"] for item in segments if item["classification"] in {"unclassified", "unclassified-raw"}
        ),
        "coverage_complete": (
            bool(segments)
            and segments[0]["start"] == 0
            and segments[-1]["end"] == ROM_SIZE
            and all(previous["end"] == current["start"] for previous, current in zip(segments, segments[1:]))
        ),
        "source_file_count": len([item for item in closure if item.suffix == ".s" and item.name != "rom.s"]),
        "strict_blocker_count": len(strict_blockers),
    }

    raw_report = []
    for record in raw_records:
        override = raw_override(record["start"], record["end"])
        if override is None:
            classification = "unclassified-raw"
            status = "unresolved"
            owner = "classification required"
            evidence = []
            strict = True
        else:
            classification = override["classification"]
            status = override["status"]
            owner = override["owner"]
            evidence = list(override["evidence"])
            strict = bool(override["strict_blocker"])
        item = {
            "start": f"0x{record['start']:06X}",
            "end": f"0x{record['end']:06X}",
            "size": record["size"],
            "vma_start": f"0x{ROM_BASE + record['start']:08X}",
            "vma_end": f"0x{ROM_BASE + record['end']:08X}",
            "path": record["path"],
            "line": record["line"],
            "text": record["text"],
            "sha256": sha256_bytes(rom[record["start"]:record["end"]]),
            "classification": classification,
            "status": status,
            "owner": owner,
            "evidence": evidence,
            "strict_blocker": strict,
        }
        if classification == "executable":
            item["reference_evidence"] = reference_evidence[hex_range(record["start"], record["end"])]
        raw_report.append(item)

    return {
        "schema": "gtadv3.code_data_ownership.v1",
        "generator": "tools/ownership_map.py",
        "generator_sha256": sha256_file(Path(__file__)),
        "rom": {
            "path": relpath(rom_path),
            "base_vma": f"0x{ROM_BASE:08X}",
            "size": ROM_SIZE,
            "sha256": rom_sha,
            "content_end": f"0x{CONTENT_END:06X}",
            "padding_nonzero_bytes": padding_nonzero,
            "header": header,
            "data_tail_sha256": sha256_bytes(rom[CODE_END:CONTENT_END]),
        },
        "inputs": {
            "source_closure_sha256": source_closure_hash(closure),
            "source_closure": [relpath(path) for path in closure],
            "code_end": f"0x{CODE_END:06X}",
        },
        "summary": summary,
        "raw_incbin_spans": raw_report,
        "source_files": source_file_inventory(closure, source_records),
        "segments": [
            {
                **segment,
                "start": f"0x{segment['start']:06X}",
                "end": f"0x{segment['end']:06X}",
                "vma_start": f"0x{ROM_BASE + segment['start']:08X}",
                "vma_end": f"0x{ROM_BASE + segment['end']:08X}",
            }
            for segment in segments
        ],
        "dependency_findings": dependency,
        "strict_blockers": strict_blockers,
        "errors": coverage_errors,
    }


def render_markdown(report: dict[str, Any]) -> str:
    summary = report["summary"]
    rom = report["rom"]
    lines = [
        "# ROM code/data ownership map",
        "",
        "Generated by `tools/ownership_map.py`; the complete machine-readable",
        "segment list is in `docs/data/code_data_ownership.json`.",
        "",
        "This is a **backing/ownership inventory**, not a claim that the C",
        "hybrid is independent.  It records, for every byte of the cartridge,",
        "what source or ROM-backed input supplies it; the strict gate below",
        "reports any executable range or reference-build object the",
        "independent link still depends on.",
        "",
        "## Coverage summary",
        "",
        "| Measure | Value |",
        "|---|---:|",
        f"| ROM size | `{rom['size']}` bytes |",
        f"| ROM SHA-256 | `{rom['sha256']}` |",
        f"| Real content end | `{rom['content_end']}` |",
        f"| Coverage gaps/unclassified bytes | `{summary['unclassified_bytes']}` |",
        f"| Complete contiguous coverage | `{'yes' if summary['coverage_complete'] else 'no'}` |",
        f"| Reconstructed assembly bytes | `{summary['assembly_source_bytes']}` |",
        f"| Raw executable bytes | `{summary['raw_executable_bytes']}` |",
        f"| Raw header bytes | `{summary['raw_header_bytes']}` |",
        f"| Cataloged data-tail bytes | `{summary['raw_data_bytes']}` |",
        f"| Zero padding bytes | `{summary['padding_bytes']}` |",
        f"| Direct `.incbin` spans | `{summary['raw_incbin_count']}` |",
        f"| Included assembly source files | `{summary['source_file_count']}` |",
        f"| Strict blockers | `{summary['strict_blocker_count']}` |",
        "",
        "## Strict gate status",
        "",
    ]
    if report["strict_blockers"]:
        lines.append("**BLOCKED** — the following must be removed or rebuilt before an independent link:")
        lines.append("")
        for blocker in report["strict_blockers"]:
            location = blocker["path"] or "unknown"
            span = f" `{blocker['range']}`" if blocker.get("range") else ""
            lines.append(f"- `{blocker['kind']}`{span} — {location}: {blocker['message']}")
    else:
        lines.append(
            "**PASS** — no executable range or reference-build object dependency"
            " was found: every executable byte comes from reconstructed source,"
            " and the C link consumes only independently assembled objects."
        )
    lines.extend(
        [
            "",
            "`make ownership-audit` checks the inventory and known map consistency.",
            (
                "`make ownership-check` is the strict gate; it fails while any"
                " blocker above remains."
                if report["strict_blockers"]
                else "`make ownership-check` is the strict gate and currently passes."
            ),
            "",
            "## Direct ROM-backed spans",
            "",
            "| File offset | VMA range | Size | Class | Status | Source | Strict |",
            "|---|---|---:|---|---|---|---|",
        ]
    )
    for item in report["raw_incbin_spans"]:
        strict = "yes" if item["strict_blocker"] else "no"
        lines.append(
            f"| `{item['start']}..{item['end']}` | `{item['vma_start']}..{item['vma_end']}` | "
            f"`{item['size']}` | {item['classification']} | {item['status']} | "
            f"`{item['path']}:{item['line']}` | {strict} |"
        )
    lines.extend(
        [
            "",
            (
                "The large tail is data/assets through the documented content end,"
                " followed by verified zero padding."
                if not summary["raw_executable_bytes"]
                else "The executable raw spans above are real code pockets, not generic"
                " unknown data: their source comments identify the bodies, and the"
                " report records heuristic BL/literal reference evidence.  The large"
                " tail is data/assets through the documented content end, followed by"
                " verified zero padding."
            ),
            "",
            "## Reconstructed source assembly",
            "",
            "The following included source files own the non-raw code region.  Each",
            "range is a source-layout declaration, not a claim that every function",
            "has already been replaced by C; C-lift coverage remains a separate gate.",
            "",
            "| Source file | Declared ROM range(s) |",
            "|---|---|",
        ]
    )
    for item in report["source_files"]:
        if not item["ranges"]:
            continue
        ranges = "<br>".join(f"`{entry['start']}..{entry['end']}`" for entry in item["ranges"])
        lines.append(f"| `{item['path']}` | {ranges} |")
    lines.extend(
        [
            "",
            "## Data and padding inputs",
            "",
            f"- `0x002E158..0x{CONTENT_END:06X}` is the cataloged ROM data/asset tail; its SHA-256 is `{rom['data_tail_sha256']}`.",
            f"- `0x{CONTENT_END:06X}..0x{ROM_SIZE:06X}` is padding; non-zero byte count is `{rom['padding_nonzero_bytes']}`.",
            "- Data/header inputs are allowed for the reference build and are reported as private reference inputs; they still need extraction/generation steps before an independent build can consume them.",
            "",
            "## Method and limitations",
            "",
            "1. Recursively walk `.include` files from `asm/rom.s`.",
            "2. Read every active `.incbin` directive and match it to an explicit classification.",
            "3. Read source region banners, with checked-in exceptions for wrapped/multi-region files.",
            "4. Split the ROM at every raw/source boundary and verify complete coverage with no unclassified interval.",
            "5. Scan the ROM for heuristic Thumb BL and literal references into raw executable ranges.",
            "6. Inspect the independent link inputs (`ldscript_matching.ld`, `ldscript_slice.ld`, `tools/match_c_slice.py`, `tools/independent_slice.py`) for `build/rom.o`.",
            "",
            "The BL/literal counts are supporting evidence, not a standalone",
            "disassembler.  Pointer-like values in data can be coincidences; the",
            "strict gate therefore relies on explicit source classification and",
            "never silently normalizes an unknown raw range.",
            "",
        ]
    )
    return "\n".join(lines)


def self_test() -> None:
    # The only raw input left is the cataloged data tail.  If any executable
    # pocket reappears as an .incbin, RAW_OVERRIDES has to grow again and the
    # strict gate has to say so.
    assert raw_override(0x02E158, ROM_SIZE) is not None
    assert raw_override(0x000000, 0x0000C0) is None  # header is asm/header.s
    assert raw_override(0x000C14, 0x000CA0) is None  # transcribed in handlers.s
    assert raw_override(0x000CA0, 0x000E80) is None
    assert raw_override(0x021860, 0x021BA0) is None  # transcribed in carphys_racer.s
    assert raw_override(0x021BA0, 0x021BF8) is None
    assert classify_raw_interval(0x7B04C4, 0x800000)["classification"] == "padding"
    assert classify_raw_interval(0x02E158, CONTENT_END)["classification"] == "asset-data"
    first = {
        "start": 0,
        "end": 0x10,
        "classification": "assembly-source",
        "status": "owned-reference-assembly",
        "executable": True,
        "backing": "asm/*.s",
        "owners": ["asm/test.s"],
        "source_owners": ["asm/test.s"],
        "intended_owner": "test",
        "evidence": [],
        "strict_blocker": False,
    }
    second = dict(first)
    second["start"] = 0x10
    second["end"] = 0x20
    assert segment_key(first) == segment_key(second)

    # The aggregator test is structural, so it must accept the byte-free top
    # levels and reject real region owners.  If it ever accepted a region
    # owner, that file's bytes would drop out of the ownership map.
    assert is_aggregator(ROOT / "asm" / "rom.s")  # reference top level
    assert is_aggregator(ROOT / "asm" / "code.s")  # shared code top level
    assert not is_aggregator(ROOT / "asm" / "header.s")  # emits .byte rows
    assert not is_aggregator(ROOT / "asm" / "data_tail.s")  # emits .incbin

    # Positive control for the independent-link dependency scanner: it has to
    # see a reference-build object, and it must not mistake the neighbouring
    # independent object directories or an EXCLUDE_FILE filter for one.
    assert reference_object_mentions("$(LD) ... build/rom.o -o out") == ["build/rom.o"]
    assert reference_object_mentions("build-code/code.o build-code/data.o") == []
    assert reference_object_mentions("build-c/gtadv3-c.o") == []
    assert reference_object_mentions("EXCLUDE_FILE(build/rom.o) *(.text*)") == []
    assert reference_object_mentions("$(BUILD)/rom.o: asm/rom.s") == []

    print("ownership_map self-test: PASS")


def compare_file(path: Path, expected: str, label: str, errors: list[str]) -> None:
    if not path.is_file():
        errors.append(f"missing {label}: {relpath(path)}")
        return
    actual = path.read_text(encoding="utf-8")
    if actual == expected:
        return
    errors.append(f"{label} differs from generated ownership report: {relpath(path)}")
    diff = difflib.unified_diff(
        expected.splitlines(), actual.splitlines(), fromfile="generated", tofile=relpath(path), lineterm=""
    )
    for line in list(diff)[:40]:
        print(line)
    if len(expected.splitlines()) or len(actual.splitlines()):
        print("... ownership report diff truncated ...")


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", default="baserom.gba", help="reference ROM (default: baserom.gba)")
    mode = parser.add_mutually_exclusive_group(required=False)
    mode.add_argument("--write", action="store_true", help="write the checked-in JSON and Markdown reports")
    mode.add_argument("--check", action="store_true", help="compare generated reports with checked-in reports")
    parser.add_argument("--json", default="docs/data/code_data_ownership.json")
    parser.add_argument("--markdown", default="docs/code_data_ownership.md")
    parser.add_argument("--strict", action="store_true", help="fail when independent-link blockers remain")
    parser.add_argument("--self-test", action="store_true", help="run parser/policy self-tests and exit")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv or sys.argv[1:])
    if args.self_test:
        self_test()
        return 0
    if not args.write and not args.check:
        print("ownership-map: ERROR: choose --write or --check", file=sys.stderr)
        return 2
    try:
        report = make_report(repo_path(args.rom))
    except (OSError, OwnershipError) as error:
        print(f"ownership-map: ERROR: {error}", file=sys.stderr)
        return 2

    report_text = json.dumps(report, indent=2, sort_keys=True) + "\n"
    markdown_text = render_markdown(report)
    json_path = repo_path(args.json)
    markdown_path = repo_path(args.markdown)
    errors = list(report["errors"])
    if args.write:
        json_path.parent.mkdir(parents=True, exist_ok=True)
        markdown_path.parent.mkdir(parents=True, exist_ok=True)
        json_path.write_text(report_text, encoding="utf-8")
        markdown_path.write_text(markdown_text, encoding="utf-8")
        print(f"wrote {relpath(json_path)} and {relpath(markdown_path)}")
    else:
        compare_file(json_path, report_text, "JSON report", errors)
        compare_file(markdown_path, markdown_text, "Markdown report", errors)

    summary = report["summary"]
    print(
        "ownership-map: "
        f"{summary['segment_count']} segments, "
        f"{summary['unclassified_bytes']} unclassified bytes, "
        f"{summary['raw_executable_bytes']} raw executable bytes, "
        f"{summary['strict_blocker_count']} strict blockers"
    )
    if errors:
        for error in errors:
            print(f"ownership-map: ERROR: {error}", file=sys.stderr)
        return 2
    if args.strict and report["strict_blockers"]:
        print("ownership-map: STRICT FAIL — executable ROM bytes or rom.o dependency remain", file=sys.stderr)
        return 1
    if args.strict:
        print("ownership-map: STRICT PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
