#!/usr/bin/env python3
"""Recover asset payloads plus a deterministic generator.

Readable, editable data *plus* a generator that reproduces the EXACT stored
bytes. This is a separate acceptance track from C function promotion: nothing
here promotes a byte, and a data-only byte match is never C-owned executable
bytes.

What it does, in order:

1. Selects a small family inside the MTO track/resource package and records,
   for every entry, BOTH the ROM VMA and the file offset, the decoded type, the
   encoding, the alignment and the ownership. Ownership is *consumed*, never
   invented: record-level ownership comes from `tools/track_dump.py` and is
   cross-checked against the committed inventory `docs/data/track_resources.txt`;
   region-level ownership comes from `docs/data/code_data_ownership.json`.
2. Reuses `tools/lz77.py` for compressed data. The decoded payload is the
   semantic content; the LZ77 stream, the trailing pad and the 8-byte record
   header are the storage representation and are kept separate.
3. Keeps the decoded payload separate from its storage form and preserves
   everything a renderer would throw away: palette index order, duplicate and
   unused entries, the unused bit 15 of a BGR555 halfword, trailing pad.
4. Generates the stored bytes from the editable representation and compares
   EXACT bytes. A visual/rendered match is never evidence: the only oracle is
   `sha256(generated) == sha256(ROM span)`.
5. For a compressed family, sweeps encoder policies (minimum match length,
   match tie-breaking, greedy vs lazy parsing, hash-chain insertion, chain
   depth, window, grouping) and compares BOTH the encoded bytes AND the
   consumed span.
6. Checks the complete owned span -- record header, payload, trailing pad --
   plus the surrounding layout constraint (the next record must start exactly
   at our end). A span is never silently redefined to make a mismatch go away.
7. Extraction and generation rules exist only for families that already have a
   classification grade and an owning record; anything else is refused.

The round-trip capability of a compressor is NOT reconstruction. If no policy in
the sweep reproduces the stored stream, the tool reports UNRESOLVED GENERATOR,
keeps the named extraction path, and says so.

Measured on this ROM (48 LZ77 records across MTO groups 2, 5, 7 and 10):
`insert=all, tie=nearest, parse=greedy, chain=64` reproduces 32 of 48 stored
streams byte-exactly; 16 are not reproduced by any of the 32 swept policies and
are reported UNRESOLVED_GENERATOR; the repo's existing `tools/lz77.py
compress()` reproduces 0 of the 48. The decisive knob is hash-chain insertion:
`tools/lz77.py` inserts only the position each token starts at, so it misses the
nearer source positions inside its own matches.

Families, and what this ROM actually measures (every entry owns its 8-byte
record header as part of the compared span):

    family           grp  n   encoding   spans reproduced   owned bytes
    variant_palette    6   7   raw        7/7                 280/280
    theme_palette      4   8   raw        8/8                 2624/2624
    theme_lut_a        3   8   raw        8/8                 2112/2112
    scenery_table      9  26   raw       26/26                2224/2224
    variant_overlay    7   7   lz77       6/7                 3012/3400
    theme_texture      2   8   lz77       0/8                 0/89800
    surface_tile       5   7   lz77       0/7                 0/24788

The four raw families are the deliverable: 49 records, 7240 owned bytes, every
one regenerated from an editable text source. The three compressed families
are the honest part: the payload is recovered and round-trips for all 22
records, but 16 stored streams are not reproduced by any swept policy and are
reported UNRESOLVED_GENERATOR with the extraction path preserved.

CLI:
    python3 tools/asset_pipeline.py --family variant_palette
    python3 tools/asset_pipeline.py --family variant_palette --vma 0x0825A120
    python3 tools/asset_pipeline.py --generate RUN/editable/variant_palette
    python3 tools/asset_pipeline.py --check --generate RUN/editable/variant_palette
    python3 tools/asset_pipeline.py --self-test

Everything is written under the private run directory (default
`build/experiments/asset_pipeline/<run-id>/`, which is git-ignored). Extracted
binaries never leave it; the editable sources are text.
"""
from __future__ import annotations

import argparse
import json
import re
import struct
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import experiment_kit as kit
import lz77
import track_dump
from extract_assets import gba_bgr555_to_rgb888

ROOT = kit.ROOT
ROM_BASE = kit.ROM_BASE
ROM_DEFAULT = ROOT / "baserom.gba"

ONE_LINE = ("asset_pipeline.py --family NAME [--vma V] [--end V] "
            "[--generate FROM_EDITABLE] [--check] [--out DIR] [--json PATH]")

STRATEGY = "asset_pipeline"
TOOL_VERSION = 1

# --------------------------------------------------------------------------
# Status vocabulary.
#
# These are DATA statuses, deliberately not the kit's candidate statuses: a
# span is not a candidate body, and `EXACT_DRAFT` would invite the reading
# that a draft is one integration step from a promoted C function. It is not.
# The kit's `EvidenceWriter.finish()` still forces accepted_c_owned_bytes = 0.
# --------------------------------------------------------------------------

SPAN_REPRODUCED = "SPAN_REPRODUCED"
SPAN_MISMATCH = "SPAN_MISMATCH"
UNRESOLVED_GENERATOR = "UNRESOLVED_GENERATOR"
UNOWNED_SPAN = "UNOWNED_SPAN"
SHORT_READ = "SHORT_READ"
CLASSIFICATION_UNSUPPORTED = "CLASSIFICATION_UNSUPPORTED"
STALE_OWNERSHIP = "STALE_OWNERSHIP"

#: Worst-first. The family status is the first of these that any entry hit, so
#: a single unowned span can never be averaged away by seven good ones.
STATUS_PRECEDENCE = (
    UNOWNED_SPAN,
    SHORT_READ,
    STALE_OWNERSHIP,
    CLASSIFICATION_UNSUPPORTED,
    SPAN_MISMATCH,
    UNRESOLVED_GENERATOR,
    SPAN_REPRODUCED,
)


class AssetRefusal(Exception):
    """Base for the fail-closed refusals. Never degrades to a guess."""

    status = CLASSIFICATION_UNSUPPORTED


class UnownedSpan(AssetRefusal):
    status = UNOWNED_SPAN


class ShortRead(AssetRefusal):
    status = SHORT_READ


class StaleOwnership(AssetRefusal):
    status = STALE_OWNERSHIP


class ClassificationUnsupported(AssetRefusal):
    status = CLASSIFICATION_UNSUPPORTED


# --------------------------------------------------------------------------
# Families
# --------------------------------------------------------------------------


@dataclass(frozen=True)
class FamilySpec:
    """One reviewed family: which records, how stored, how decoded, how well
    the interpretation is actually supported.

    `grade` and `evidence` are the honest part. "u16 BGR555" for group 6 is
    grade B (a documented consumer DMAs the chunk to 0x050001E0); for group 4
    the same reading is grade C (role unproven). The generator does not care --
    it reproduces bytes either way -- but the editable source says which is
    which, so nobody later mistakes a guess for a finding.
    """

    name: str
    group: int
    editable: str                 # u16_palette | u8_table | lz77_payload
    decoded_type: str
    encoding: str                 # raw | lz77-0x10
    role: str
    grade: str
    evidence: tuple[str, ...]
    sizes: tuple[int, ...]        # allowed stored payload sizes
    dec_sizes: tuple[int, ...] = ()   # allowed decoded sizes (compressed only)


FAMILIES: dict[str, FamilySpec] = {
    "variant_palette": FamilySpec(
        name="variant_palette", group=6, editable="u16_palette",
        decoded_type="u16[16] little-endian; bits 0-14 BGR555, bit 15 preserved verbatim",
        encoding="raw", role="road/surface variant palette chunk", grade="B",
        evidence=("Resource group 6: palette chunk -> 0x050001E0 via DMA3",
                  "43 of 112 halfwords carry bit 15 set, so bit 15 is metadata, not colour"),
        sizes=(0x20,)),
    "theme_palette": FamilySpec(
        name="theme_palette", group=4, editable="u16_palette",
        decoded_type="u16[160] little-endian; bits 0-14 BGR555, bit 15 preserved verbatim",
        encoding="raw", role="per-theme 160-colour chunk (10 x 16)", grade="C",
        evidence=("Resource group 4: 0x140 each, role graded C",
                  "tools/extract_assets.py reads group 4 as GBA palettes",
                  "0 of 1280 halfwords carry bit 15; 828 distinct low-15 values"),
        sizes=(0x140,)),
    "theme_lut_a": FamilySpec(
        name="theme_lut_a", group=3, editable="u8_table",
        decoded_type="u8[256] attribute lookup",
        encoding="raw", role="per-theme attribute LUT", grade="C",
        evidence=("Resource group 3: theme LUT A, 256 B",
                  "values are small (11 distinct); a u16 reading would be a scale artefact"),
        sizes=(0x100,)),
    "scenery_table": FamilySpec(
        name="scenery_table", group=9, editable="u16_palette",
        decoded_type="u16[n] little-endian; 15-bit BGR555-consistent, role unproven",
        encoding="raw", role="scenery/car small table", grade="C",
        evidence=("Resource group 9: small tables, role graded C",
                  "three stored sizes (0x20/0x40/0x80); 0 bit-15 halfwords over 26 records"),
        sizes=(0x20, 0x40, 0x80)),
    "variant_overlay": FamilySpec(
        name="variant_overlay", group=7, editable="lz77_payload",
        decoded_type="u8[0x200] tile payload",
        encoding="lz77-0x10", role="road/surface variant tile overlay -> VRAM 0x0600E000",
        grade="B",
        evidence=("Resource group 7: more tiles -> VRAM 0x0600E000",
                  "every record starts with a BIOS-LZ77 type 0x10 header"),
        sizes=(0x17C, 0x180, 0x200, 0x204, 0x208), dec_sizes=(0x200, 0x800)),
    "theme_texture": FamilySpec(
        name="theme_texture", group=2, editable="lz77_payload",
        decoded_type="u8[0x4000] texture/tilemap payload",
        encoding="lz77-0x10", role="per-theme 128x128 texture", grade="C",
        evidence=("Resource group 2: LZ77, 0x4000 each, role graded C",
                  "all 8 records decode to exactly 0x4000 bytes; none is reproduced by any "
                  "swept encoder policy, so this family is an UNRESOLVED GENERATOR by "
                  "measurement, not by assumption"),
        sizes=(0x7A4, 0x2CC0, 0x300C, 0x3094, 0x3140, 0x3174, 0x3314, 0x33BC),
        dec_sizes=(0x4000,)),
    "surface_tile": FamilySpec(
        name="surface_tile", group=5, editable="lz77_payload",
        decoded_type="u8[n] 4bpp surface tile graphics",
        encoding="lz77-0x10", role="road surface tile graphics -> VRAM 0x06008000", grade="B",
        evidence=("Resource group 5: surface tile graphics -> 0x06008000",
                  "seven stored sizes, decoded 0x6a0..0x1c20; none reproduced by any swept "
                  "encoder policy, so this family is an UNRESOLVED GENERATOR by measurement"),
        sizes=(0x118, 0xAD8, 0xF48, 0xFD0, 0x11D4, 0x132C, 0x1094),
        dec_sizes=(0x6A0, 0x1380, 0x1BA0, 0x1BC0, 0x1C00, 0x1C20)),
}

#: Policy names in sweep order. `chain` is the hash-chain depth; the value only
#: matters when two candidates tie, which is precisely the axis §9 item 5 asks
#: about, so it is swept rather than fixed.
CANONICAL_POLICY = {"insert": "all", "tie": "nearest", "parse": "greedy", "chain": 64}

POLICY_SWEEP: tuple[dict, ...] = tuple(
    {"insert": ins, "tie": tie, "parse": parse, "chain": chain}
    for ins in ("all", "head")
    for tie in ("nearest", "farthest")
    for parse in ("greedy", "lazy")
    for chain in (1, 16, 64, 1024)
)


def policy_name(policy: dict) -> str:
    return (f"insert={policy['insert']},tie={policy['tie']},"
            f"parse={policy['parse']},chain={policy['chain']}")


def _policy_from_name(name: str) -> dict:
    """Inverse of `policy_name`, so a recorded sweep winner can be re-run."""
    policy = dict(CANONICAL_POLICY)
    for field_text in name.split(","):
        key, _, value = field_text.partition("=")
        policy[key] = int(value) if key == "chain" else value
    return policy



# --------------------------------------------------------------------------
# Ownership
# --------------------------------------------------------------------------

INVENTORY = ROOT / "docs/data/track_resources.txt"
OWNERSHIP_JSON = ROOT / "docs/data/code_data_ownership.json"


@dataclass(frozen=True)
class Record:
    """One owned MTO record. `vma` is the RECORD start (the 8-byte header is
    part of the owned span), `payload` is where the seeker returns."""

    file_offset: int
    group: int
    index: int
    size: int
    seq: int
    next_file_offset: int | None

    @property
    def vma(self) -> int:
        return self.file_offset + ROM_BASE

    @property
    def payload_file_offset(self) -> int:
        return self.file_offset + 8

    @property
    def payload_vma(self) -> int:
        return self.vma + 8

    @property
    def end(self) -> int:
        return self.vma + 8 + self.size

    @property
    def end_file_offset(self) -> int:
        return self.file_offset + 8 + self.size

    def owns(self, vma: int) -> bool:
        return self.vma <= vma < self.end

    def to_json(self) -> dict:
        return {
            "vma": f"{self.vma:#010x}",
            "end": f"{self.end:#010x}",
            "file_offset": f"{self.file_offset:#x}",
            "end_file_offset": f"{self.end_file_offset:#x}",
            "payload_vma": f"{self.payload_vma:#010x}",
            "payload_file_offset": f"{self.payload_file_offset:#x}",
            "group": self.group,
            "index": self.index,
            "size": self.size,
            "alignment": self.file_offset % 4,
            "next_record_vma": (f"{self.next_file_offset + ROM_BASE:#010x}"
                                if self.next_file_offset is not None else None),
        }


class Ownership:
    """Record-level ownership from the existing map, cross-checked against the
    committed inventory. A disagreement is a hard refusal, not a warning: the
    inventory is what a reviewer reads, and silently preferring one source would
    make the evidence unfalsifiable."""

    def __init__(self, rom: bytes, *, inventory: Path = INVENTORY,
                 ownership_json: Path = OWNERSHIP_JSON):
        self.rom = rom
        counts, offsets, rows = track_dump.read_package(rom)
        self.counts = counts
        self.directory_file_offset = track_dump.DIR_OFF
        records = []
        for seq, (pos, group, index, size, _payload) in enumerate(rows):
            nxt = rows[seq + 1][0] if seq + 1 < len(rows) else None
            records.append(Record(pos, group, index, size, seq, nxt))
        self.records = records
        self.by_start = {r.file_offset: r for r in records}
        self.inventory_rows = self._read_inventory(inventory)
        self.inventory_checked = self.inventory_rows is not None
        self._cross_check_inventory()
        self.segment = self._owning_segment(ownership_json)

    # -- inventory -----------------------------------------------------
    @staticmethod
    def _read_inventory(path: Path) -> dict | None:
        if not path.exists():
            return None
        rows = {}
        for line in path.read_text().splitlines():
            if not line or line.startswith("#"):
                continue
            cols = line.split("\t")
            if len(cols) < 6:
                continue
            rows[(int(cols[1], 16), int(cols[3]), int(cols[4]))] = int(cols[5], 16)
        return rows

    def _cross_check_inventory(self) -> None:
        if self.inventory_rows is None:
            return
        for rec in self.records:
            key = (rec.file_offset, rec.group, rec.index)
            if key not in self.inventory_rows:
                raise StaleOwnership(
                    f"record {rec.vma:#010x} (g{rec.group} i{rec.index}) is absent from "
                    f"{INVENTORY.relative_to(ROOT)}; regenerate the inventory before trusting it")
            if self.inventory_rows[key] != rec.size:
                raise StaleOwnership(
                    f"record {rec.vma:#010x} size {rec.size:#x} disagrees with the committed "
                    f"inventory ({self.inventory_rows[key]:#x})")

    def _owning_segment(self, path: Path) -> dict | None:
        if not path.exists():
            return None
        data = json.loads(path.read_text())
        for seg in data.get("segments", []):
            start = int(seg["start"], 16)
            end = int(seg["end"], 16)
            if start <= self.directory_file_offset < end:
                return {
                    "classification": seg.get("classification"),
                    "intended_owner": seg.get("intended_owner"),
                    "owners": seg.get("owners", []),
                    "status": seg.get("status"),
                    "start": f"{start:#x}",
                    "end": f"{end:#x}",
                }
        return None

    # -- resolution ----------------------------------------------------
    def group_records(self, group: int) -> list[Record]:
        return [r for r in self.records if r.group == group]

    def resolve(self, vma: int) -> Record:
        """The record that owns `vma`, or a refusal. Never a guess."""
        if vma < ROM_BASE:
            raise UnownedSpan(f"{vma:#x} is below the ROM base {ROM_BASE:#x}")
        for rec in self.records:
            if rec.owns(vma):
                return rec
        raise UnownedSpan(
            f"{vma:#010x} is inside no owned MTO record; the ownership map covers "
            f"{self.records[0].vma:#010x}..{self.records[-1].end:#010x} only")


# --------------------------------------------------------------------------
# Span primitives
# --------------------------------------------------------------------------


def read_span(rom: bytes, vma: int, end: int) -> bytes:
    """Exactly the bytes of [vma, end). A short read is a failure, not a
    truncated success -- a prefix comparison would happily call a 4 KiB read of
    a 4 KiB-3 record a match."""
    if end <= vma:
        raise ShortRead(f"empty or reversed span {vma:#010x}..{end:#010x}")
    off = vma - ROM_BASE
    if off < 0:
        raise ShortRead(f"span {vma:#010x} starts below the ROM base")
    if off + (end - vma) > len(rom):
        raise ShortRead(
            f"span {vma:#010x}..{end:#010x} needs {end - vma} bytes at file {off:#x}, "
            f"ROM holds {len(rom)}")
    return rom[off:off + (end - vma)]


@dataclass
class SpanCheck:
    """Result of comparing generated bytes against the complete owned span."""

    status: str
    vma: int
    end: int
    length: int
    matched_bytes: int = 0
    first_diff: int | None = None
    rom_sha: str = ""
    generated_sha: str = ""
    detail: str | None = None

    @property
    def exact(self) -> bool:
        return self.status == SPAN_REPRODUCED

    def to_json(self) -> dict:
        return {
            "status": self.status,
            "vma": f"{self.vma:#010x}",
            "end": f"{self.end:#010x}",
            "file_offset": f"{self.vma - ROM_BASE:#x}",
            "length": self.length,
            "matched_bytes": self.matched_bytes,
            "first_diff": self.first_diff,
            "rom_sha256": self.rom_sha,
            "generated_sha256": self.generated_sha,
            "detail": self.detail,
        }


def compare_span(rom: bytes, vma: int, end: int, generated: bytes) -> SpanCheck:
    """Whole-span byte comparison. Length is part of the verdict."""
    actual = read_span(rom, vma, end)
    length = end - vma
    result = SpanCheck(status=SPAN_MISMATCH, vma=vma, end=end, length=length,
                       rom_sha=kit.digest_bytes(actual),
                       generated_sha=kit.digest_bytes(generated))
    if len(generated) != length:
        result.detail = (f"generated {len(generated)} bytes for a {length}-byte owned span")
        return result
    if generated == actual:
        result.status = SPAN_REPRODUCED
        result.matched_bytes = length
        return result
    for k in range(length):
        if generated[k] != actual[k]:
            result.matched_bytes = k
            result.first_diff = k
            result.detail = (f"first difference at +{k:#x} "
                             f"(span {vma + k:#010x}): generated {generated[k]:#04x} "
                             f"rom {actual[k]:#04x}")
            return result
    result.status = SPAN_REPRODUCED
    result.matched_bytes = length
    return result


def check_layout(record: Record, chain_end_file_offset: int) -> str | None:
    """The surrounding layout constraint: this record's end must be exactly
    where the next record begins. A gap means the owned span is wrong, and
    reporting a mismatch against the wrong span is exactly how a real error
    gets redefined away."""
    if record.next_file_offset is None:
        if record.end_file_offset != chain_end_file_offset:
            return (f"last record ends at {record.end_file_offset:#x} but the chain ends at "
                    f"{chain_end_file_offset:#x}")
        return None
    if record.next_file_offset != record.end_file_offset:
        return (f"next record starts at {record.next_file_offset:#x}, owned span ends at "
                f"{record.end_file_offset:#x}")
    return None


# --------------------------------------------------------------------------
# BIOS-LZ77: token layer, so encoder policy is a real, comparable object
# --------------------------------------------------------------------------


def lz77_tokens(stream: bytes, *, limit: int | None = None) -> tuple[list[tuple[int, int | None]], int]:
    """Decode one BIOS-LZ77 stream to tokens; return (tokens, consumed bytes).

    `consumed` is the *stored* length, which §9 item 5 requires comparing and
    not just the encoded bytes: two encoders can emit the same length and
    completely different streams, and one can emit a shorter stream than the
    record holds.
    """
    if len(stream) < 4:
        raise ClassificationUnsupported(f"LZ77 stream truncated: {len(stream)} bytes")
    if stream[0] != 0x10:
        raise ClassificationUnsupported(f"bad LZ77 magic {stream[0]:#04x}")
    size = int.from_bytes(stream[1:4], "little")
    j = 4
    produced = 0
    tokens: list[tuple[int, int | None]] = []
    while produced < size:
        if j >= len(stream):
            raise ShortRead(
                f"LZ77 stream needs more than the {len(stream)} bytes it owns to produce "
                f"{size:#x} bytes (stopped at +{j:#x} having produced {produced:#x}); "
                f"refusing to read past the owned span")
        flags = stream[j]
        j += 1
        for bit in range(8):
            if produced >= size:
                break
            if flags & (0x80 >> bit):
                if j + 1 >= len(stream):
                    raise ShortRead(f"LZ77 match unit truncated at +{j:#x}")
                b, b2 = stream[j], stream[j + 1]
                j += 2
                length = ((b >> 4) & 0xF) + 3
                disp = ((b & 0xF) << 8) | b2
                if disp >= produced:
                    raise ClassificationUnsupported(
                        f"LZ77 displacement {disp:#x} at output {produced:#x} reaches before "
                        f"the start of the buffer")
                tokens.append((length, disp))
                produced += length
            else:
                if j >= len(stream):
                    raise ShortRead(f"LZ77 literal truncated at +{j:#x}")
                tokens.append((1, None))
                j += 1
                produced += 1
            if limit is not None and j > limit:
                raise ShortRead(f"LZ77 stream exceeds the {limit}-byte sweep limit")
    return tokens, j


def lz77_emit(payload: bytes, tokens: list[tuple[int, int | None]]) -> bytes:
    """Re-encode a token list. The inverse of `lz77_tokens` up to the token
    choices, which are exactly what an encoder policy decides."""
    out = bytearray(b"\x10" + len(payload).to_bytes(3, "little"))
    pos = 0
    idx = 0
    total = len(tokens)
    while idx < total:
        flags = 0
        units = bytearray()
        for bit in range(8):
            if idx >= total:
                break
            length, disp = tokens[idx]
            if disp is None:
                units.append(payload[pos])
            else:
                flags |= 0x80 >> bit
                units.append(((length - 3) << 4) | ((disp >> 8) & 0xF))
                units.append(disp & 0xFF)
            pos += length
            idx += 1
        out.append(flags)
        out.extend(units)
    return bytes(out)


def lz77_encode(payload: bytes, *, minlen: int = 3, maxlen: int = 18,
                window: int = 0x1000, chain: int = 64, tie: str = "nearest",
                insert: str = "all", parse: str = "greedy") -> bytes:
    """Parameterised BIOS-LZ77 encoder.

    The knobs are the ones §9 item 5 names. Two of them turned out to decide
    this ROM's streams:

    * `insert` -- whether every consumed position enters the hash chain or
      only the position a token starts at. `tools/lz77.py`'s compressor does
      the latter, which is why it reproduces 0 of the 48 MTO LZ77 records while
      `all` reproduces 32.
    * `tie` -- which source position wins when two candidates are equally
      long. Same token count, different bytes.
    """
    n = len(payload)
    head: dict[bytes, int] = {}
    prev = [-1] * n

    def insert_at(i: int) -> None:
        if i + minlen <= n:
            key = payload[i:i + minlen]
            prev[i] = head.get(key, -1)
            head[key] = i

    def find(i: int) -> tuple[int, int]:
        if i + minlen > n:
            return 0, 0
        key = payload[i:i + minlen]
        best = best_disp = 0
        j = head.get(key, -1)
        depth = 0
        limit = min(maxlen, n - i)
        while j != -1 and depth < chain:
            disp = i - j - 1
            if disp > window:
                break
            run = 0
            while run < limit and payload[j + run] == payload[i + run]:
                run += 1
            if run > best or (run == best and run >= minlen and
                              ((tie == "nearest" and disp < best_disp) or
                               (tie == "farthest" and disp > best_disp))):
                best, best_disp = run, disp
                if run == limit:
                    break
            j = prev[j]
            depth += 1
        return best, best_disp

    tokens: list[tuple[int, int | None]] = []
    i = 0
    while i < n:
        run, disp = find(i)
        if parse == "lazy" and run >= minlen and i + 1 < n:
            next_run, _ = find(i + 1)
            if next_run > run:
                tokens.append((1, None))
                insert_at(i)
                i += 1
                continue
        if run >= minlen:
            tokens.append((run, disp))
            if insert == "all":
                for k in range(i, i + run):
                    insert_at(k)
            else:
                insert_at(i)
            i += run
        else:
            tokens.append((1, None))
            insert_at(i)
            i += 1
    return lz77_emit(payload, tokens)


@dataclass
class SweepResult:
    """What the policy sweep measured for one compressed entry."""

    reproduced: bool
    policy: str | None
    candidates_tried: int
    original_stream: int
    best_stream: int | None
    best_policy: str | None
    repo_compressor_exact: bool
    repo_compressor_len: int
    complete: bool
    detail: str | None = None

    def to_json(self) -> dict:
        return {
            "reproduced": self.reproduced,
            "policy": self.policy,
            "candidates_tried": self.candidates_tried,
            "original_stream": self.original_stream,
            "best_stream": self.best_stream,
            "best_policy": self.best_policy,
            "repo_compressor_exact": self.repo_compressor_exact,
            "repo_compressor_len": self.repo_compressor_len,
            "sweep_complete": self.complete,
            "detail": self.detail,
        }


def sweep_policies(payload: bytes, stored: bytes, *, meter: kit.BudgetMeter | None = None,
                   policies=POLICY_SWEEP) -> SweepResult:
    """Try every policy; report bytes AND consumed span for each.

    The repo's own compressor is measured in the same pass, because "our new
    encoder is different" is only interesting next to "and the old one was
    wrong too, in a specific way".
    """
    original_stream = len(stored)
    tried = 0
    reproduced = False
    winner = None
    best_len = None
    best_policy = None
    complete = True
    for policy in policies:
        if meter is not None:
            try:
                meter.spend()
            except kit.BudgetExhausted:
                complete = False
                break
        encoded = lz77_encode(payload, **policy)
        tried += 1
        if best_len is None or len(encoded) < best_len:
            best_len, best_policy = len(encoded), policy_name(policy)
        if encoded == stored:
            reproduced = True
            winner = policy_name(policy)
            break
    repo = lz77.compress(payload)
    result = SweepResult(
        reproduced=reproduced, policy=winner, candidates_tried=tried,
        original_stream=original_stream, best_stream=best_len,
        best_policy=best_policy, repo_compressor_exact=(repo == stored),
        repo_compressor_len=len(repo), complete=complete)
    if not reproduced:
        result.detail = (
            f"no policy in {tried} reproduced the stored stream: original {original_stream} B, "
            f"best candidate {best_len} B via {best_policy}; the payload round-trips but the "
            f"encoder policy is unresolved")
    return result


# --------------------------------------------------------------------------
# Editable representation
#
# The decoded payload and the storage representation are separate objects with
# separate syntax. The payload arrays hold semantic values; the `#define` block
# holds the record header and the stored-span facts. Nothing in the payload
# arrays is derived from a rendering, so a palette that looks identical but has
# a different bit 15 still regenerates different bytes -- and gets caught.
# --------------------------------------------------------------------------

_DEFINE_RE = re.compile(r"^#define\s+([A-Z0-9_]+)\s+(0x[0-9a-fA-F]+|\d+)\s*$")
_ARRAY_HEAD_RE = re.compile(r"^static const u(8|16)\s+([A-Za-z0-9_]+)\[(\d*)\]\s*=\s*\{$")
_VALUE_RE = re.compile(r"0x[0-9a-fA-F]{1,4}|\d+")


@dataclass
class Editable:
    """One entry's editable form: semantic payload + storage metadata."""

    name: str
    defines: dict[str, int]
    arrays: dict[str, tuple[int, list[int]]]   # name -> (width, values)

    def u8(self, name: str) -> bytes:
        return bytes(self._values(name, 8))

    def u16(self, name: str) -> bytes:
        values = self._values(name, 16)
        return b"".join(struct.pack("<H", v) for v in values)

    def _values(self, name: str, width: int) -> list[int]:
        if name not in self.arrays:
            raise ClassificationUnsupported(f"editable source has no array {name!r}")
        got_width, values = self.arrays[name]
        if got_width != width:
            raise ClassificationUnsupported(
                f"array {name!r} is u{got_width}, expected u{width}")
        return values

    def define(self, name: str) -> int:
        if name not in self.defines:
            raise ClassificationUnsupported(f"editable source has no #define {name}")
        return self.defines[name]


def render_editable(spec: FamilySpec, entry: str, payload: bytes, *,
                    record: Record, storage: dict[str, int], provenance: str,
                    pad_values: bytes = b"") -> str:
    """Emit the editable source for one entry. Text only, always."""
    upper = entry.upper()
    lines = [
        "/* Generated by tools/asset_pipeline.py, "
        f"{STRATEGY} v{TOOL_VERSION}.",
        f" * family      : {spec.name} (MTO group {spec.group})",
        f" * entry       : {entry}",
        f" * record      : VMA {record.vma:#010x} .. {record.end:#010x} "
        f"(file {record.file_offset:#x}..{record.end_file_offset:#x}, {record.size:#x}-byte payload)",
        f" * encoding    : {spec.encoding}",
        f" * decoded type: {spec.decoded_type}",
        f" * role        : {spec.role} (classification grade {spec.grade})",
        f" * ownership   : {provenance}",
        " *",
        " * The arrays below are the DECODED payload: semantic values, in index",
        " * order, including duplicate and unused entries and any unused bit. The",
        " * #define block is the STORAGE representation: record header and stored",
        " * span facts. Regenerate with:",
        f" *   python3 tools/asset_pipeline.py --generate <dir>/{spec.name}",
        " * Bytes are the only oracle; a rendered or visual match proves nothing.",
        " */",
        "",
        "/* --- storage representation ------------------------------------- */",
    ]
    for key in ("GROUP", "INDEX", "SIZE", "ENCODED_SIZE", "PAD_LEN", "DECODED_SIZE"):
        if key in storage:
            lines.append(f"#define {upper}_{key} 0x{storage[key]:x}")
    lines.append("")
    if spec.editable == "u16_palette":
        count = len(payload) // 2
        lines.append(f"/* {count} colours; comment is a 24-bit render of bits 0-14 and is")
        lines.append(" * NOT authoritative -- the halfword below is, bit 15 included. */")
        lines.append(f"static const u16 {entry}[{count}] = {{")
        for row in range(0, count, 4):
            chunk = []
            for k in range(row, min(row + 4, count)):
                value = struct.unpack_from("<H", payload, k * 2)[0]
                r, g, b = gba_bgr555_to_rgb888(value)
                flag = "bit15" if value & 0x8000 else ""
                chunk.append(f"0x{value:04X}, /* {k:3d}: #{r:02X}{g:02X}{b:02X} {flag} */")
            lines.append("    " + " ".join(chunk))
        lines.append("};")
    elif spec.editable == "u8_table":
        lines.append(f"static const u8 {entry}[{len(payload)}] = {{")
        for row in range(0, len(payload), 16):
            chunk = ", ".join(f"0x{value:02X}" for value in payload[row:row + 16])
            lines.append(f"    {chunk},")
        lines.append("};")
    elif spec.editable == "lz77_payload":
        lines.append(f"static const u8 {entry}[{len(payload)}] = {{")
        for row in range(0, len(payload), 16):
            chunk = ", ".join(f"0x{value:02X}" for value in payload[row:row + 16])
            lines.append(f"    {chunk},")
        lines.append("};")
        pad_len = storage.get("PAD_LEN", 0)
        if pad_len:
            lines.extend([
                "/* Trailing bytes inside the owned span that the LZ77 stream does not",
                " * consume. Kept verbatim: their content is not assumed to be zero. */",
                f"static const u8 {entry}_pad[{pad_len}] = {{",
                "    " + ", ".join(f"0x{value:02X}" for value in pad_values) + ",",
                "};",
            ])
    else:  # pragma: no cover - registry is closed
        raise ClassificationUnsupported(f"unknown editable kind {spec.editable!r}")
    lines.append("")
    return "\n".join(lines)


def parse_editable(text: str) -> Editable:
    """Strictly parse the form `render_editable` emits.

    Strict on purpose. A lenient parser that skipped unrecognised lines would
    let a hand edit delete a palette entry -- or retype one -- and still
    regenerate a span that merely *looks* right. Every non-blank line must be a
    `#define`, an array head, array values, or the array terminator.
    """
    defines: dict[str, int] = {}
    arrays: dict[str, tuple[int, list[int]]] = {}
    stripped = re.sub(r"/\*.*?\*/", " ", text, flags=re.DOTALL)
    lines = stripped.splitlines()
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        i += 1
        if not line:
            continue
        if line.startswith("#define"):
            match = _DEFINE_RE.match(line)
            if not match:
                raise ClassificationUnsupported(f"unparseable #define line: {line!r}")
            if match.group(1) in defines:
                raise ClassificationUnsupported(f"#define {match.group(1)} appears twice")
            defines[match.group(1)] = int(match.group(2), 0)
            continue
        if line.startswith("static const"):
            head = _ARRAY_HEAD_RE.match(line)
            if not head:
                raise ClassificationUnsupported(f"unparseable array head: {line!r}")
            width = int(head.group(1))
            name = head.group(2)
            declared = head.group(3)
            if name in arrays:
                raise ClassificationUnsupported(f"array {name!r} defined twice")
            body: list[str] = []
            closed = False
            while i < len(lines):
                body.append(lines[i])
                closed = "};" in lines[i]
                i += 1
                if closed:
                    break
            if not closed:
                raise ClassificationUnsupported(f"array {name!r} is never terminated with ';}}'")
            blob = " ".join(body)
            blob = blob[:blob.rindex("};")]
            values = [int(token, 0) for token in _VALUE_RE.findall(blob)]
            leftovers = _VALUE_RE.sub("", blob).replace(",", " ").split()
            if leftovers:
                raise ClassificationUnsupported(
                    f"array {name!r} has non-numeric content: {leftovers[:4]}")
            if declared and int(declared) != len(values):
                raise ClassificationUnsupported(
                    f"array {name!r} declares [{declared}] but has {len(values)} values")
            limit = 0x100 if width == 8 else 0x10000
            bad = [v for v in values if not 0 <= v < limit]
            if bad:
                raise ClassificationUnsupported(
                    f"array {name!r} has out-of-range u{width} value(s) {bad[:4]}")
            arrays[name] = (width, values)
            continue
        raise ClassificationUnsupported(f"unexpected line in editable source: {line!r}")
    if not arrays:
        raise ClassificationUnsupported("editable source has no arrays")
    return Editable(name="", defines=defines, arrays=arrays)


# --------------------------------------------------------------------------
# Commit policy
# --------------------------------------------------------------------------


def is_text_file(path: Path) -> bool:
    try:
        data = path.read_bytes()
    except OSError:
        return False
    if b"\x00" in data:
        return False
    try:
        data.decode("utf-8")
    except UnicodeDecodeError:
        return False
    return True


def assert_commit_material(paths: list[Path]) -> None:
    """Committed output is source/text/build material only.

    A ROM-derived binary is a legitimate artifact -- it is what gets compared
    -- but it belongs in the ignored run directory, never in a tree a reviewer
    would commit. This refuses rather than warns.
    """
    for path in paths:
        if not path.is_file():
            raise ClassificationUnsupported(f"expected artifact is missing: {path}")
        if not is_text_file(path):
            raise ClassificationUnsupported(
                f"{path} is not text; ROM-derived binaries must stay inside the ignored run "
                f"directory and must not be proposed as committed material")


def is_git_ignored(path: Path) -> bool:
    try:
        result = subprocess.run(["git", "-C", str(ROOT), "check-ignore", "-q", str(path)],
                                capture_output=True)
    except OSError:
        return False
    return result.returncode == 0


# --------------------------------------------------------------------------
# Extraction / generation
# --------------------------------------------------------------------------


@dataclass
class EntryResult:
    name: str
    status: str
    record: Record
    payload_size: int
    decoded_size: int
    detail: str | None = None
    check: SpanCheck | None = None
    sweep: SweepResult | None = None
    layout: str | None = None
    editable_path: str | None = None

    def to_json(self) -> dict:
        data = {
            "name": self.name,
            "status": self.status,
            "record": self.record.to_json(),
            "payload_size": self.payload_size,
            "decoded_size": self.decoded_size,
            "detail": self.detail,
            "layout_ok": self.layout is None,
            "layout_detail": self.layout,
            "editable": self.editable_path,
            # The kit's summary renderer reads these two names; they are the
            # owned-span byte counts, not C function bytes.
            "matched_bytes": self.check.matched_bytes if self.check else 0,
            "rom_bytes": self.check.length if self.check else 0,
        }
        if self.check is not None:
            data["span"] = self.check.to_json()
        if self.sweep is not None:
            data["encoder_sweep"] = self.sweep.to_json()
        return data


def _provenance(own: Ownership) -> str:
    seg = own.segment
    if not seg:
        return f"MTO track package directory at {own.directory_file_offset:#x}"
    owners = ", ".join(seg.get("owners") or []) or seg.get("intended_owner", "unknown")
    return (f"MTO record chain (tools/track_dump.py, directory "
            f"{own.directory_file_offset:#x}); region owner {owners}")


def _entry_name(spec: FamilySpec, record: Record) -> str:
    return f"{spec.name}_{record.index:02d}"


def _classify(spec: FamilySpec, payload: bytes) -> bytes:
    """Validate the stored bytes against the reviewed classification, and
    return the decoded payload. Refuses rather than decoding something the
    family does not describe."""
    if len(payload) not in spec.sizes:
        raise ClassificationUnsupported(
            f"stored payload {len(payload):#x} bytes is not one of the reviewed sizes "
            f"{[hex(s) for s in spec.sizes]} for {spec.name}")
    if spec.encoding == "raw":
        if spec.editable == "u16_palette" and len(payload) % 2:
            raise ClassificationUnsupported(
                f"{spec.name} is a u16 family but the payload length {len(payload)} is odd")
        return payload
    tokens, consumed = lz77_tokens(payload)
    decoded = lz77.decompress(payload)
    if len(decoded) not in spec.dec_sizes:
        raise ClassificationUnsupported(
            f"decoded size {len(decoded):#x} is not one of the reviewed sizes "
            f"{[hex(s) for s in spec.dec_sizes]} for {spec.name}")
    if consumed > len(payload):
        raise ShortRead("LZ77 stream overruns its own record")
    return decoded


def _storage_facts(spec: FamilySpec, payload: bytes, decoded: bytes,
                   consumed: int) -> dict[str, int]:
    storage = {"GROUP": spec.group, "INDEX": 0, "SIZE": len(payload)}
    if spec.encoding != "raw":
        storage["DECODED_SIZE"] = len(decoded)
        storage["ENCODED_SIZE"] = consumed
        storage["PAD_LEN"] = len(payload) - consumed
    return storage


def generate_record_bytes(spec: FamilySpec, entry: str, editable: Editable,
                          *, policy: dict) -> tuple[bytes, dict]:
    """Editable form -> the complete stored record bytes, plus the facts the
    check needs. This is the generator: it emits the 8-byte storage header from
    the #define block, re-encodes the payload, appends the verbatim pad, and
    compares the consumed span against the recorded one."""
    group = editable.define(f"{entry.upper()}_GROUP")
    index = editable.define(f"{entry.upper()}_INDEX")
    size = editable.define(f"{entry.upper()}_SIZE")
    if group != spec.group:
        raise ClassificationUnsupported(
            f"editable source says group {group}, the family owns group {spec.group}")
    if size > 0xFFFF:
        raise ClassificationUnsupported(f"record size {size:#x} is not representable")
    header = struct.pack("<HHI", group, index, size)
    if spec.editable == "u16_palette":
        body = editable.u16(entry)
    elif spec.editable == "u8_table":
        body = editable.u8(entry)
    else:
        payload = editable.u8(entry)
        declared = editable.define(f"{entry.upper()}_DECODED_SIZE")
        if len(payload) != declared:
            raise ClassificationUnsupported(
                f"payload is {len(payload)} bytes, the storage record says {declared:#x}")
        stream = lz77_encode(payload, **policy)
        consumed = len(stream)
        recorded = editable.define(f"{entry.upper()}_ENCODED_SIZE")
        pad_len = editable.define(f"{entry.upper()}_PAD_LEN")
        pad = editable.u8(f"{entry}_pad") if pad_len else b""
        if len(pad) != pad_len:
            raise ClassificationUnsupported(
                f"pad array holds {len(pad)} bytes, the storage record says {pad_len}")
        if consumed + pad_len != size:
            raise ClassificationUnsupported(
                f"generated stream {consumed} + pad {pad_len} != stored size {size}; the "
                f"encoder policy is wrong for this entry, not the span")
        if recorded != consumed:
            raise ClassificationUnsupported(
                f"consumed span {consumed} disagrees with the recorded {recorded}; the "
                f"encoder policy is unresolved, not a near miss")
        body = stream + pad
    if len(body) != size:
        raise ClassificationUnsupported(
            f"generated {len(body)} payload bytes for a {size}-byte stored payload")
    facts = {"header": len(header), "payload": len(body)}
    return header + body, facts


def extract_family(rom: bytes, own: Ownership, spec: FamilySpec, *,
                   only: Record | None, writer: kit.EvidenceWriter,
                   meter: kit.BudgetMeter, chain_end: int) -> tuple[list[EntryResult], dict]:
    """Decoded payload -> editable sources, in the run directory."""
    out_dir = writer.path("editable", spec.name)
    out_dir.mkdir(parents=True, exist_ok=True)
    records = [only] if only is not None else own.group_records(spec.group)
    if not records:
        raise UnownedSpan(f"no MTO record belongs to group {spec.group}")
    provenance = _provenance(own)
    results: list[EntryResult] = []
    manifest_entries = []
    for record in records:
        name = _entry_name(spec, record)
        entry_path = out_dir / f"{name}.c"
        stored = read_span(rom, record.payload_vma, record.payload_vma + record.size)
        try:
            decoded = _classify(spec, stored)
        except AssetRefusal as exc:
            results.append(EntryResult(name=name, status=exc.status, record=record,
                                       payload_size=record.size, decoded_size=0,
                                       detail=str(exc)))
            writer.record({"name": name, "status": exc.status, "detail": str(exc),
                           "record": record.to_json()})
            continue
        storage = _storage_facts(spec, stored, decoded,
                                 lz77_tokens(stored)[1] if spec.encoding != "raw" else 0)
        storage["INDEX"] = record.index
        pad_values = stored[storage.get("ENCODED_SIZE", 0):] if spec.encoding != "raw" else b""
        text = render_editable(spec, name, decoded, record=record, storage=storage,
                               provenance=provenance, pad_values=pad_values)
        entry_path.write_text(text)
        assert_commit_material([entry_path])
        contract = kit.Contract(
            name=name, vma=record.vma, end=record.end, return_type="u32",
            memory="ordinary", inputs=(),
            abi_note=(f"{spec.encoding} record: 8-byte storage header + {record.size:#x}-byte "
                      f"payload; decoded {spec.decoded_type}; {provenance}"))
        contract.require_supported()
        writer.save_contract(contract)
        manifest_entries.append({
            "name": name, "file": entry_path.name, "index": record.index,
            "record": record.to_json(), "payload_size": record.size,
            "decoded_size": len(decoded),
            "sha256_decoded": kit.digest_bytes(decoded),
            "policy": policy_name(CANONICAL_POLICY) if spec.encoding != "raw" else None,
        })
        results.append(EntryResult(name=name, status="EXTRACTED", record=record,
                                   payload_size=record.size, decoded_size=len(decoded),
                                   editable_path=writer.relative(entry_path)))
        writer.record({"name": name, "status": "EXTRACTED",
                       "record": record.to_json(),
                       "payload_size": record.size, "decoded_size": len(decoded),
                       "editable": writer.relative(entry_path),
                       "sha256_decoded": kit.digest_bytes(decoded)})
    manifest = {
        "family": spec.name,
        "group": spec.group,
        "encoding": spec.encoding,
        "decoded_type": spec.decoded_type,
        "role": spec.role,
        "classification_grade": spec.grade,
        "evidence": list(spec.evidence),
        "ownership": provenance,
        "directory_file_offset": f"{own.directory_file_offset:#x}",
        "directory_vma": f"{own.directory_file_offset + ROM_BASE:#010x}",
        "inventory_cross_checked": own.inventory_checked,
        "generator_policy": policy_name(CANONICAL_POLICY) if spec.encoding != "raw" else None,
        "rom_sha256": kit.digest_bytes(rom),
        "tool": f"{Path(__file__).name} v{TOOL_VERSION}",
        "entries": manifest_entries,
    }
    manifest_path = out_dir / "family.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    assert_commit_material([manifest_path])
    # The raw payload is useful evidence but is a ROM-derived binary: it lives
    # only in the ignored run directory and is never proposed for commit.
    extracted = writer.path("extracted", spec.name)
    extracted.mkdir(parents=True, exist_ok=True)
    for record in records:
        (extracted / f"{_entry_name(spec, record)}.payload.bin").write_bytes(
            read_span(rom, record.payload_vma, record.payload_vma + record.size))
    return results, manifest


def verify_family(rom: bytes, own: Ownership, spec: FamilySpec, source: Path, *,
                  writer: kit.EvidenceWriter, meter: kit.BudgetMeter,
                  chain_end: int, sweep_bytes: int) -> list[EntryResult]:
    """Editable sources -> stored bytes -> compare against the complete owned
    span. This is the only thing that counts as success."""
    manifest_path = source / "family.json"
    if not manifest_path.exists():
        raise ClassificationUnsupported(f"no family.json under {source}")
    manifest = json.loads(manifest_path.read_text())
    if manifest.get("family") != spec.name:
        raise ClassificationUnsupported(
            f"{manifest_path} describes family {manifest.get('family')!r}, not {spec.name!r}")
    if manifest.get("rom_sha256") != kit.digest_bytes(rom):
        raise StaleOwnership(
            "the editable sources were extracted from a different ROM image; regenerate them")
    results = []
    for item in manifest["entries"]:
        name = item["name"]
        record_info = item["record"]
        vma = int(record_info["vma"], 16)
        end = int(record_info["end"], 16)
        try:
            record = own.resolve(vma)
        except AssetRefusal as exc:
            results.append(EntryResult(name=name, status=exc.status,
                                       record=Record(vma - ROM_BASE, spec.group,
                                                     item["index"], 0, -1, None),
                                       payload_size=0, decoded_size=0, detail=str(exc)))
            writer.record({"name": name, "status": exc.status, "detail": str(exc)})
            continue
        if record.vma != vma or record.end != end:
            raise StaleOwnership(
                f"{name}: editable sources claim span {vma:#010x}..{end:#010x}, the ownership "
                f"map now says {record.vma:#010x}..{record.end:#010x}")
        text = (source / item["file"]).read_text()
        editable = parse_editable(text)
        if spec.editable == "u16_palette":
            decoded_size = len(editable.u16(name))
        else:
            decoded_size = len(editable.u8(name))
        def attempt(policy: dict) -> SpanCheck:
            """Generate and compare; a policy that cannot even fill the stored
            size is a measured miss, not a crash -- the sweep still has to run."""
            try:
                blob, _facts = generate_record_bytes(spec, name, editable, policy=policy)
            except AssetRefusal as exc:
                return SpanCheck(status=SPAN_MISMATCH, vma=vma, end=end,
                                 length=end - vma,
                                 detail=f"generator produced no candidate: {exc}")
            return compare_span(rom, vma, end, blob)

        check = attempt(dict(CANONICAL_POLICY))
        layout = check_layout(record, chain_end)
        status = check.status
        sweep = None
        detail = check.detail
        if spec.encoding != "raw" and not check.exact:
            # The canonical policy failed. Sweep the encoder policy space
            # before concluding anything: "our encoder is wrong" and "no
            # encoder reproduces this" are different facts.
            stored = read_span(rom, record.payload_vma, record.payload_vma + record.size)
            decoded = _classify(spec, stored)
            pad_len = editable.define(f"{name.upper()}_PAD_LEN")
            stream = stored[:len(stored) - pad_len]
            if len(decoded) > sweep_bytes:
                status = UNRESOLVED_GENERATOR
                detail = (f"payload recovered ({len(decoded)} bytes) but the encoder sweep was "
                          f"skipped: it exceeds the {sweep_bytes}-byte --sweep-bytes limit. "
                          f"Not swept is not reproduced.")
            else:
                sweep = sweep_policies(decoded, stream, meter=meter,
                                       policies=POLICY_SWEEP)
                if sweep.reproduced:
                    check = attempt(_policy_from_name(sweep.policy))
                    status = check.status
                    detail = (f"canonical policy failed; the swept policy {sweep.policy} "
                              f"reproduces the stored stream"
                              + (f" ({check.detail})" if check.detail else ""))
                else:
                    status = UNRESOLVED_GENERATOR
                    detail = (f"payload recovered and round-trips ({len(decoded)} bytes) but no "
                              f"encoder policy reproduces the stored stream ({sweep.detail}); "
                              f"extraction path preserved: tools/lz77.py decompress of the "
                              f"owned record at {record.payload_vma:#010x}")
        if layout is not None and status in (SPAN_REPRODUCED, SPAN_MISMATCH):
            status = SPAN_MISMATCH
            detail = f"layout constraint violated: {layout}"
        result = EntryResult(name=name, status=status, record=record,
                             payload_size=record.size, decoded_size=decoded_size,
                             detail=detail, check=check, sweep=sweep, layout=layout,
                             editable_path=str(source / item["file"]))
        results.append(result)
        payload = {"name": name, "status": status, "record": record.to_json(),
                   "payload_size": record.size, "decoded_size": decoded_size,
                   "detail": detail, "span": check.to_json(),
                   "layout_ok": layout is None}
        if sweep is not None:
            payload["encoder_sweep"] = sweep.to_json()
        writer.record(payload)
    return results


def summarize(results: list[EntryResult]) -> str:
    """Worst-first fold. A single unowned span is not averaged away."""
    seen = {r.status for r in results}
    for status in STATUS_PRECEDENCE:
        if status in seen:
            return status
    return CLASSIFICATION_UNSUPPORTED if not results else SPAN_MISMATCH


# --------------------------------------------------------------------------
# CLI
# --------------------------------------------------------------------------


def _auto_int(text: str) -> int:
    return int(text, 0)


def build_parser() -> argparse.ArgumentParser:
    # `kit.tool_header` builds the same three shared options but references
    # `argparse` inside experiment_kit, which that module never imports, so
    # calling it raises NameError. The options are reproduced here so this tool
    # keeps the shared CLI contract; fix the kit and this can go back to it.
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=ONE_LINE)
    ap.add_argument("--out", type=Path,
                    help="private run directory (default build/experiments/"
                         f"{STRATEGY}/<run>)")
    ap.add_argument("--seed", type=int, default=0,
                    help="RNG seed; recorded in the evidence (this strategy is "
                         "deterministic, so the seed is provenance only)")
    ap.add_argument("--json", type=Path, help="copy the run summary here")
    ap.add_argument("--family", choices=sorted(FAMILIES), help="family to extract or verify")
    ap.add_argument("--vma", type=_auto_int, help="restrict to the record owning this VMA")
    ap.add_argument("--end", type=_auto_int,
                    help="assert the owned span ends here; disagreement is a refusal")
    ap.add_argument("--generate", type=Path, metavar="FROM_EDITABLE",
                    help="generate from an existing editable directory instead of extracting")
    ap.add_argument("--check", action="store_true",
                    help="verify only: do not extract; exit 1 unless every span is reproduced")
    ap.add_argument("--rom", type=Path, default=ROM_DEFAULT, help="ROM image")
    ap.add_argument("--sweep-bytes", type=_auto_int, default=0x4000,
                    help="skip encoder sweeps for payloads larger than this")
    ap.add_argument("--self-test", action="store_true", help="run the self-test and exit")
    return ap


def body(args) -> int:
    if args.self_test:
        return self_test()
    rom_path = Path(args.rom)
    if not rom_path.exists():
        raise kit.DependencyMissing(f"ROM image {rom_path} is missing")
    rom = rom_path.read_bytes()

    source = Path(args.generate) if args.generate else None
    if args.family is None and source is None:
        raise kit.Unsupported(
            f"--family is required (one of {', '.join(sorted(FAMILIES))}), or pass "
            f"--generate FROM_EDITABLE with a family.json that names the family")
    spec = FAMILIES[args.family] if args.family else None

    if args.out:
        out = Path(args.out)
        writer = kit.EvidenceWriter(STRATEGY, out.name, root=out.parent,
                                    extra={"family": args.family or "from-manifest",
                                           "seed": args.seed,
                                           "rom": str(rom_path),
                                           "rom_sha256": kit.digest_bytes(rom)})
    else:
        writer = kit.EvidenceWriter(STRATEGY, None, extra={
            "family": args.family or "from-manifest", "seed": args.seed,
            "rom": str(rom_path), "rom_sha256": kit.digest_bytes(rom)})
    writer.open()

    own = Ownership(rom)
    chain_end = own.records[-1].end_file_offset
    meter = kit.BudgetMeter(kit.Budget(
        compiles=len(POLICY_SWEEP) * 8 + 8, seconds=600.0,
        note="budget unit is one encoder-policy trial, not one agbcc compile"))

    try:
        if source is None:
            only = None
            if args.vma is not None or args.end is not None:
                only = _select_record(own, args)
            extracted, manifest = extract_family(rom, own, spec, only=only, writer=writer,
                                                 meter=meter, chain_end=chain_end)
            source = writer.path("editable", spec.name)
        else:
            manifest = json.loads((source / "family.json").read_text())
            spec = FAMILIES[manifest["family"]]
            if args.vma is not None or args.end is not None:
                _select_record(own, args)

        results = verify_family(rom, own, spec, source, writer=writer, meter=meter,
                                chain_end=chain_end, sweep_bytes=args.sweep_bytes)
    except AssetRefusal as exc:
        writer.record({"name": spec.name if spec else "?", "status": exc.status,
                       "detail": str(exc)})
        payload = writer.finish({"draft_status": exc.status, "results": [],
                                 "notes": [str(exc)]})
        _emit(args, writer, payload, [])
        print(f"{STRATEGY}: {exc.status}: {exc}", file=sys.stderr)
        return 2

    status = summarize(results)
    payload = writer.finish({
        "draft_status": status,
        "family": spec.name,
        "group": spec.group,
        "encoding": spec.encoding,
        "decoded_type": spec.decoded_type,
        "entries": len(results),
        "entries_reproduced": sum(1 for r in results if r.status == SPAN_REPRODUCED),
        "entries_unresolved_generator": sum(1 for r in results if r.status == UNRESOLVED_GENERATOR),
        "owned_bytes_checked": sum(r.record.size + 8 for r in results),
        "owned_bytes_reproduced": sum(r.record.size + 8 for r in results
                                      if r.status == SPAN_REPRODUCED),
        "generator_policy": policy_name(CANONICAL_POLICY) if spec.encoding != "raw" else "n/a (raw)",
        "run_dir": writer.relative(writer.dir),
        "commit_material": "text-only under the ignored run dir; "
                           f"git-ignored: {is_git_ignored(writer.dir)}",
        "results": [r.to_json() for r in results],
        "notes": _notes(results, spec),
        "budget": meter.to_json(),
    })
    _emit(args, writer, payload, results)
    for row in results:
        mark = "OK  " if row.status == SPAN_REPRODUCED else "FAIL"
        extra = ""
        if row.sweep is not None:
            extra = (f"  encoder: {row.sweep.policy or 'unresolved'} "
                     f"(repo lz77.compress exact={row.sweep.repo_compressor_exact}, "
                     f"original {row.sweep.original_stream} B, best {row.sweep.best_stream} B)")
        print(f"{mark} {row.name:24s} {row.record.vma:#010x}..{row.record.end:#010x} "
              f"file {row.record.file_offset:#07x}  {row.status}{extra}")
        if row.detail:
            print(f"       {row.detail}")
    print(f"{STRATEGY}: {status} "
          f"({payload['entries_reproduced']}/{payload['entries']} spans reproduced, "
          f"{payload['owned_bytes_reproduced']}/{payload['owned_bytes_checked']} owned bytes)")
    print(f"run: {writer.relative(writer.dir)}")
    if args.check and status != SPAN_REPRODUCED:
        return 1
    return 0


def _select_record(own: Ownership, args) -> Record:
    if args.vma is None:
        raise UnownedSpan("--end without --vma cannot be resolved to an owned span")
    record = own.resolve(args.vma)
    if args.vma not in (record.vma, record.payload_vma):
        raise UnownedSpan(
            f"{args.vma:#010x} is inside record {record.vma:#010x} but is not its start "
            f"({record.vma:#010x}) or payload start ({record.payload_vma:#010x}); refusing to "
            f"redefine the owned span around an interior address")
    if args.end is not None and args.end != record.end:
        raise UnownedSpan(
            f"--end {args.end:#010x} disagrees with the owned span end {record.end:#010x}; "
            f"refusing to redefine the span to make a mismatch disappear")
    return record


def _notes(results: list[EntryResult], spec: FamilySpec) -> list[str]:
    notes = [
        "STATUS SEMANTICS: this is a DATA status. SPAN_REPRODUCED means the generator "
        "reproduces the complete owned ROM span byte for byte. It is not a C draft, not a "
        "promotion, and contributes 0 C-owned executable bytes.",
        f"family classification grade {spec.grade}: {spec.role}",
    ]
    unresolved = [r.name for r in results if r.status == UNRESOLVED_GENERATOR]
    if unresolved:
        notes.append("UNRESOLVED GENERATOR for " + ", ".join(unresolved) +
                     ": payload recovered and round-trips, original stored stream not "
                     "reproduced. Extraction path preserved (tools/lz77.py decompress of the "
                     "owned record); the encoder policy is not claimed.")
    produced = [r.name for r in results if r.status == SPAN_REPRODUCED]
    if produced and spec.encoding != "raw":
        notes.append("reproduced with policy " + policy_name(CANONICAL_POLICY))
    repo_exact = [r.name for r in results
                  if r.sweep is not None and r.sweep.repo_compressor_exact]
    if results and any(r.sweep is not None for r in results):
        notes.append("tools/lz77.py compress() reproduced "
                     + (", ".join(repo_exact) if repo_exact else "none of the swept entries"))
    return notes


def _emit(args, writer: kit.EvidenceWriter, payload: dict, results) -> None:
    if args.json:
        path = Path(args.json)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(payload, indent=2, sort_keys=True, default=str) + "\n")


# --------------------------------------------------------------------------
# Self-test
# --------------------------------------------------------------------------


def self_test() -> int:
    print("running asset_pipeline self-test...")
    passed = total = skipped = 0

    def check(label: str, condition: bool) -> None:
        nonlocal passed, total
        total += 1
        if condition:
            passed += 1
            print(f"  [PASS] {label}")
        else:
            print(f"  [FAIL] {label}", file=sys.stderr)

    def skip(label: str, why: str) -> None:
        nonlocal skipped
        skipped += 1
        print(f"  [SKIP] {label} ({why})")

    spec = FAMILIES["variant_palette"]
    record = Record(0x25A118, 6, 0, 0x20, 157, 0x25A140)

    # ---- editable representation round-trips exactly ------------------
    payload = bytes([0x1F, 0x7C, 0x62, 0xE9, 0xA4, 0x6D, 0x00, 0x00])
    text = render_editable(spec, "variant_palette_00", payload, record=record,
                           storage={"GROUP": 6, "INDEX": 0, "SIZE": 8},
                           provenance="self-test", pad_values=b"")
    back = parse_editable(text)
    check("palette survives the editable text form", back.u16("variant_palette_00") == payload)

    tricky = struct.pack("<4H", 0x8000, 0x7C1F, 0x7C1F, 0x0001)
    tricky_text = render_editable(spec, "variant_palette_00", tricky, record=record,
                                  storage={"GROUP": 6, "INDEX": 0, "SIZE": 8},
                                  provenance="self-test", pad_values=b"")
    tricky_back = parse_editable(tricky_text)
    check("bit 15 and duplicate entries are preserved, not rendered away",
          tricky_back.u16("variant_palette_00") == tricky)
    check("a bit-15 palette renders to the same RGB as its low-15 twin but keeps its bytes",
          gba_bgr555_to_rgb888(0x8000 | 0x7C1F) == gba_bgr555_to_rgb888(0x7C1F)
          and tricky_back.u16("variant_palette_00") != struct.pack("<4H", 0, 0x7C1F, 0x7C1F, 1))

    table = bytes(range(256))
    table_spec = FAMILIES["theme_lut_a"]
    table_text = render_editable(table_spec, "theme_lut_a_00", table, record=record,
                                 storage={"GROUP": 3, "INDEX": 0, "SIZE": 256},
                                 provenance="self-test", pad_values=b"")
    check("byte table survives the editable text form",
          parse_editable(table_text).u8("theme_lut_a_00") == table)

    check("a wrong element count is refused",
          kit.raises(ClassificationUnsupported, lambda: parse_editable(
              table_text.replace("[256]", "[255]"))))
    check("a non-numeric value is refused",
          kit.raises(ClassificationUnsupported, lambda: parse_editable(
              table_text.replace("0x00, 0x01", "0x00, oops, 0x02"))))
    check("a deleted array is refused",
          kit.raises(ClassificationUnsupported, lambda: parse_editable(
              "/* only a comment */\n")))

    # ---- span checks -------------------------------------------------
    rom = bytes(0x40)
    check("a short read fails instead of comparing a prefix",
          kit.raises(ShortRead, lambda: read_span(rom, ROM_BASE + 0x30, ROM_BASE + 0x50)))
    check("an empty span is refused", kit.raises(ShortRead, lambda: read_span(rom, ROM_BASE, ROM_BASE)))
    check("a span below the ROM base is refused",
          kit.raises(ShortRead, lambda: read_span(rom, ROM_BASE - 0x10, ROM_BASE)))
    exact = compare_span(rom, ROM_BASE, ROM_BASE + 0x40, rom)
    check("an exact span is SPAN_REPRODUCED", exact.status == SPAN_REPRODUCED and exact.matched_bytes == 0x40)
    last_byte = bytearray(rom)
    last_byte[0x3F] = 0xFF
    tail_check = compare_span(rom, ROM_BASE, ROM_BASE + 0x40, bytes(last_byte))
    check("a difference in the final byte of the span is caught",
          tail_check.status == SPAN_MISMATCH and tail_check.first_diff == 0x3F)
    short_gen = compare_span(rom, ROM_BASE, ROM_BASE + 0x40, rom[:-1])
    check("a generator that emits fewer bytes than the span is not a match",
          short_gen.status == SPAN_MISMATCH
          and "generated 63 bytes for a 64-byte owned span" in (short_gen.detail or "")
          and short_gen.matched_bytes == 0)
    long_gen = compare_span(rom, ROM_BASE, ROM_BASE + 0x40, rom + b"\x00")
    check("a generator that emits more bytes than the span is not a match",
          long_gen.status == SPAN_MISMATCH and long_gen.matched_bytes == 0)

    # ---- ownership refusals -------------------------------------------
    rom_present = ROM_DEFAULT.exists()
    if rom_present:
        own = Ownership(ROM_DEFAULT.read_bytes())
        check("the committed inventory is cross-checked", own.inventory_checked
              and len(own.inventory_rows) == 249)
        record0 = own.records[0]
        check("ownership resolves a record start", own.resolve(record0.vma) is record0)
        check("ownership resolves a payload start", own.resolve(record0.payload_vma) is record0)
        check("an address outside every record is refused",
              kit.raises(UnownedSpan, lambda: own.resolve(0x08000000)))
        check("an address past the package is refused",
              kit.raises(UnownedSpan, lambda: own.resolve(own.records[-1].end + 4)))
        interior = record0.vma + 16
        check("an interior address resolves to its record, not a new span",
              own.resolve(interior) is record0)
        args_like = argparse.Namespace(vma=interior, end=None)
        check("an interior --vma is refused rather than redefining the span",
              kit.raises(UnownedSpan, lambda: _select_record(own, args_like)))
        bad_end = argparse.Namespace(vma=record0.vma, end=record0.end - 4)
        check("a --end that disagrees with the owned span is refused",
              kit.raises(UnownedSpan, lambda: _select_record(own, bad_end)))
        good = argparse.Namespace(vma=record0.vma, end=record0.end)
        check("the record's own span is accepted", _select_record(own, good) is record0)
        check("end without vma is refused",
              kit.raises(UnownedSpan, lambda: _select_record(own, argparse.Namespace(vma=None, end=0x1000))))
        check("an unknown family name is not in the registry",
              "no_such_family" not in FAMILIES)
        check("layout is contiguous across the whole chain",
              all(check_layout(r, own.records[-1].end_file_offset) is None for r in own.records))
        broken = Record(record0.file_offset, 6, 0, 0x20, 0, record0.file_offset + 0x40)
        check("a non-contiguous successor is reported as a layout violation",
              check_layout(broken, 0) is not None)
    else:
        skip("ownership cross-checks", "baserom.gba is absent")

    # ---- BIOS-LZ77: the round-trip / reproduction distinction ----------
    sample = bytes([0x00, 0xF0, 0x01, 0xF0, 0x02, 0xF0, 0x00, 0x07,
                    0xF0, 0x08, 0xF0, 0x09, 0xF0] * 8)
    tokens, consumed = lz77_tokens(lz77_encode(sample))
    check("token parse and emit are inverses", lz77_emit(sample, tokens)
          == lz77_encode(sample))
    check("the token layer reports the consumed span", consumed == len(lz77_encode(sample)))

    stored = lz77_encode(sample, insert="all", tie="nearest")
    reencoded = lz77_encode(sample, insert="head", tie="farthest")
    check("a re-encoded blob round-trips to the same payload",
          lz77.decompress(reencoded) == lz77.decompress(stored) == sample)
    check("a re-encoded blob that is not the stored bytes is REJECTED",
          compare_span(rom, ROM_BASE, ROM_BASE + len(stored), reencoded).status == SPAN_MISMATCH)
    sweep = sweep_policies(sample, stored)
    check("the sweep recovers the policy that produced a stored blob",
          sweep.reproduced and "tie=nearest" in (sweep.policy or ""))
    # Negative control for the sweep itself: a legitimate stream whose token
    # choices no policy in the space can produce. A sweep that "finds" this
    # would be reporting a policy it invented.
    # Same token choices, different payload: a perfectly valid stream that no
    # policy encoding `sample` can emit.
    other = bytearray(sample)
    other[0] ^= 0xFF
    tampered_tokens, _consumed = lz77_tokens(stored)
    tampered = lz77_emit(bytes(other), tampered_tokens)
    check("the tampered stream is still a decodable LZ77 stream",
          tampered != stored and len(lz77.decompress(tampered)) == len(sample)
          and lz77.decompress(tampered) != sample)
    check("the sweep finds no policy for a stream none of them produced",
          not sweep_policies(sample, tampered).reproduced)
    check("a corrupt stream is refused, not decoded",
          kit.raises(ClassificationUnsupported, lambda: lz77_tokens(b"\x11\x00\x02\x00abcd")))
    truncated = stored[:len(stored) - 3]
    check("a short read inside the stream fails rather than decoding a prefix",
          kit.raises(ShortRead, lambda: lz77_tokens(truncated)))
    check("a stream whose declared size outruns the span is refused",
          kit.raises(ShortRead, lambda: lz77_tokens(
              b"\x10" + (0x1000).to_bytes(3, "little") + b"\x00" * 8)))
    check("classification refuses a stored size the family does not describe",
          kit.raises(ClassificationUnsupported,
                     lambda: _classify(spec, b"\x00" * 0x24)))

    # ---- unresolved generator ------------------------------------------
    bad_spec = FAMILIES["variant_overlay"]
    bad_editable = Editable(
        name="x",
        defines={"X_GROUP": 7, "X_INDEX": 0, "X_SIZE": 0x10,
                 "X_DECODED_SIZE": len(sample), "X_ENCODED_SIZE": 0x10, "X_PAD_LEN": 0},
        arrays={"x": (8, list(sample))})
    check("a payload whose stored size disagrees is refused",
          kit.raises(ClassificationUnsupported,
                     lambda: generate_record_bytes(bad_spec, "x", bad_editable,
                                                   policy=CANONICAL_POLICY)))
    check("a payload whose consumed span disagrees is refused",
          kit.raises(ClassificationUnsupported,
                     lambda: generate_record_bytes(
                         bad_spec, "x",
                         Editable(name="x",
                                  defines={**bad_editable.defines, "X_SIZE": 0x40,
                                           "X_ENCODED_SIZE": 0x40, "X_PAD_LEN": 0},
                                  arrays=bad_editable.arrays),
                         policy=CANONICAL_POLICY)))
    mixed = [EntryResult("a", SPAN_REPRODUCED, record, 0x20, 0x20),
             EntryResult("b", UNRESOLVED_GENERATOR, record, 0x20, 0x200)]
    check("an unresolved generator outranks a reproduced sibling",
          summarize(mixed) == UNRESOLVED_GENERATOR)
    check("a mismatch outranks an unresolved generator",
          summarize(mixed + [EntryResult("c", SPAN_MISMATCH, record, 0x20, 0x20)])
          == SPAN_MISMATCH)
    check("an unowned span outranks everything",
          summarize(mixed + [EntryResult("d", UNOWNED_SPAN, record, 0, 0)]) == UNOWNED_SPAN)
    check("all reproduced folds to SPAN_REPRODUCED",
          summarize([EntryResult("a", SPAN_REPRODUCED, record, 0x20, 0x20)]) == SPAN_REPRODUCED)

    # ---- commit material ----------------------------------------------
    with kit._temp_dir(None) as tmp:
        tmp = Path(tmp)
        text_file = tmp / "editable.c"
        text_file.write_text(text)
        binary = tmp / "payload.bin"
        binary.write_bytes(payload)
        check("an editable source is text", is_text_file(text_file))
        check("a ROM-derived binary is not text", not is_text_file(binary))
        check("the commit policy refuses a binary artifact",
              kit.raises(ClassificationUnsupported,
                         lambda: assert_commit_material([binary])))
        assert_commit_material([text_file])
        check("the commit policy accepts a text artifact", True)
        check("the commit policy refuses a missing artifact",
              kit.raises(ClassificationUnsupported,
                         lambda: assert_commit_material([tmp / "nope.c"])))

    # ---- ROM-backed end-to-end -----------------------------------------
    # Drives the same extract -> generate -> compare path the CLI does, so the
    # suite cannot pass while the tool itself is broken.
    check("the default run directory is git-ignored",
          is_git_ignored(kit.EXPERIMENTS_ROOT))
    if rom_present:
        rom_image = ROM_DEFAULT.read_bytes()
        own_rom = Ownership(rom_image)
        chain_end = own_rom.records[-1].end_file_offset
        with kit._temp_dir(None) as tmp:
            for family in ("variant_palette", "theme_palette", "theme_lut_a", "scenery_table"):
                spec_rom = FAMILIES[family]
                recs = own_rom.group_records(spec_rom.group)
                writer = kit.EvidenceWriter("selftest", f"run_{family}",
                                            root=Path(tmp)).open()
                meter = kit.BudgetMeter(kit.Budget(compiles=512, seconds=600.0))
                extract_family(rom_image, own_rom, spec_rom, only=None, writer=writer,
                               meter=meter, chain_end=chain_end)
                results = verify_family(rom_image, own_rom, spec_rom,
                                        writer.path("editable", family), writer=writer,
                                        meter=meter, chain_end=chain_end,
                                        sweep_bytes=0x4000)
                owned = sum(r.record.size + 8 for r in results)
                ok = sum(1 for r in results if r.status == SPAN_REPRODUCED)
                check(f"{family}: all {len(recs)} owned spans regenerate byte-exactly",
                      ok == len(recs) and len(results) == len(recs))
                check(f"{family}: {owned} owned bytes checked, 8-byte record headers included",
                      owned > 0)
                check(f"{family}: every editable source is text",
                      bool(list(writer.path("editable", family).glob("*.c"))) and
                      all(is_text_file(path)
                          for path in writer.path("editable", family).glob("*")))
                check(f"{family}: the layout constraint holds for every owned span",
                      all(r.layout is None for r in results))

            overlay = FAMILIES["variant_overlay"]
            recs = own_rom.group_records(overlay.group)
            writer = kit.EvidenceWriter("selftest", "run_overlay", root=Path(tmp)).open()
            meter = kit.BudgetMeter(kit.Budget(compiles=4096, seconds=900.0))
            extract_family(rom_image, own_rom, overlay, only=None, writer=writer,
                           meter=meter, chain_end=chain_end)
            results = verify_family(rom_image, own_rom, overlay,
                                    writer.path("editable", "variant_overlay"),
                                    writer=writer, meter=meter, chain_end=chain_end,
                                    sweep_bytes=0x4000)
            reproduced = [r for r in results if r.status == SPAN_REPRODUCED]
            unresolved = [r for r in results if r.status == UNRESOLVED_GENERATOR]
            repo_exact = [r for r in results
                          if r.sweep is not None and r.sweep.repo_compressor_exact]
            check("the compressed family has at least one byte-exact reproduction",
                  len(reproduced) >= 1)
            check("the compressed family has entries no swept policy reproduces",
                  len(unresolved) >= 1)
            check("unreproduced compressed entries report UNRESOLVED GENERATOR, not success",
                  all("round-trips" in (r.detail or "") for r in unresolved))
            check("tools/lz77.py compress() reproduces none of the swept entries",
                  not repo_exact)
            check("every compressed entry is either reproduced or explicitly unresolved",
                  len(reproduced) + len(unresolved) == len(results) == len(recs))
            check("the compressed family summary never claims a full success",
                  summarize(results) == UNRESOLVED_GENERATOR)
    else:
        skip("ROM-backed end-to-end regeneration", "baserom.gba is absent")

    print(f"{passed}/{total} passed" + (f", {skipped} skipped" if skipped else ""))
    return 0 if passed == total else 1


def main() -> int:
    if "--self-test" in sys.argv:
        return self_test()
    return kit.run_tool(STRATEGY, body, ap=build_parser())


if __name__ == "__main__":
    raise SystemExit(main())
