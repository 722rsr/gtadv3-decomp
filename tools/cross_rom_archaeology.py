#!/usr/bin/env python3
"""Cross-game / cross-version source archaeology.

Use a second GT-family ROM as a *hypothesis generator* about this project's
own code, never as authority. Two related games may share a routine, a record
layout or a helper; the differences between them can expose source constants,
optional features, data layouts, helper boundaries or alternative compilation
shapes. Sharing is a hypothesis to be scored, not a fact.

What this tool does, and deliberately does not, do:

  * Takes an explicit list of ROMs as `name=path`, each with its own address
    space, SHA-256, title and game code. Nothing is downloaded and no ROM is
    assumed present. A named-but-missing ROM raises `DependencyMissing`.
  * Builds a *conservative* code/data map for one bounded subsystem per ROM.
    A function boundary is only accepted when `match_families.flow` walks the
    span with zero failures; anything else stays UNKNOWN. It never treats every
    decodable byte as code.
  * Extends the family fingerprint (`match_families.fingerprint`) across ROMs:
    normalized instruction tokens, CFG edges, resolved callee offsets, literal
    roles and data-access widths.
  * Ranks correspondences with SEVERAL independent features and keeps EXACT
    matches and WEAK similarities in explicitly different confidence classes.
    No single scalar is ever reported as "a match".
  * Diffs what changed: constants, pool/literal values, branch conditions and
    call structure, and turns those into source hypotheses.
  * Emits research leads carrying source-ROM hash, VMA, file offset and the
    confidence evidence behind them.

HARD BOUNDARY (enforced in code, stated in every run's evidence):
  Another ROM's function prototype is NEVER authoritative ABI evidence for GT
  Advance 3, and VMA-shaped labels from another game NEVER enter this project's
  symbol closure. Any C reconstruction is validated against this project's own
  instructions, its own ABI and the unchanged agbcc byte oracle -- see
  `--validate-c`, which only ever scores against the project ROM.

stdlib only. No network. Drafts only; nothing here can promote a byte.
"""
from __future__ import annotations

import argparse
import difflib
import hashlib
import json
import re
import shutil
import struct
import subprocess
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import experiment_kit as kit
import match_families as families
import corpus_match_probe as probe

ROOT = probe.ROOT
ROM_BASE = probe.ROM_BASE
STRATEGY = "cross_rom_archaeology"

#: The project ROM. This is the only ROM any candidate may be scored against.
PROJECT = "gt3"


BOUNDARY_STATEMENT = (
    "another ROM's function prototype is NEVER authoritative ABI evidence for "
    "GT Advance 3; VMA-shaped labels from another game NEVER enter this "
    "project's symbol closure; C reconstruction is validated only against this "
    "project's own instructions, its own ABI and the unchanged agbcc oracle"
)

# Confidence classes. Deliberately NOT one scalar; the order is a set, not a
# ranking of strength -- WEAK is never collapsed into EXACT.
C_EXACT_BYTES = "EXACT_BYTES"          # byte-identical spans
C_EXACT_NORM = "EXACT_NORMALIZED"     # identical normalized tokens + CFG
C_WEAK = "WEAK_SIMILARITY"            # several features agree, not exact
C_NONE = "NO_CORRESPONDENCE"          # nothing above threshold
B_UNKNOWN = "UNKNOWN"                 # boundary not established

#: Feature thresholds. A WEAK match needs agreement across this many
#: INDEPENDENT features -- never one strong feature alone.
WEAK_FEATURE_FLOOR = 0.75
WEAK_MIN_FEATURES = 3
FEATURE_NAMES = ("tokens", "cfg", "size", "calls", "literals", "widths")

#: Built-in bounded subsystems, expressed as seed VMAs in the *project* ROM.
#: These are seeds, not claims: each is a real entry in this project's own
#: function map, and its span is re-established by reachability every run. A
#: subsystem with no verified entry points is absent rather than guessed -- an
#: invented seed that fails to walk cleanly only produces UNKNOWN noise.
SUBSYSTEMS = {
    "keypad": [0x08002430, 0x08002488, 0x08002494, 0x080024A0],   # KeypadPoll/Edge/Held/Repeat
    "keypad-poll": [0x08002430],                                  # KeypadPoll
    "menu": [0x0800CFE4, 0x0800D048, 0x0800C884],                 # MenuInit/RecordUpdater/RecordHandler
}


# ---------------------------------------------------------------------------
# ROM identity and address spaces
# ---------------------------------------------------------------------------


@dataclass
class Image:
    """One ROM and its own address space.

    `space` is the identity of the address space: (name, sha256). Every VMA is
    only ever resolved inside the space that produced it, so a VMA measured in
    one game can never silently be looked up in another game's map.
    """
    name: str
    path: Path
    sha256: str
    title: str
    game_code: str
    maker: int
    data: bytes = field(repr=False, default=b"")
    functions: dict = field(default_factory=dict)      # vma -> end, this space only
    provenance: str = "unknown"

    @property
    def space(self) -> tuple:
        return (self.name, self.sha256)

    def file_offset(self, vma: int) -> int:
        return vma - ROM_BASE

    def read(self, vma: int, end: int) -> bytes:
        """Bytes of THIS ROM. A VMA outside this ROM raises, it does not clamp."""
        if not ROM_BASE <= vma <= end <= ROM_BASE + len(self.data):
            raise ValueError(f"{self.name}: span {vma:#x}..{end:#x} outside this ROM")
        return self.data[self.file_offset(vma):self.file_offset(end)]

    def resolve(self, vma: int) -> int | None:
        """End of the function starting at `vma` in THIS space, else None.

        Returning None for an unknown VMA is what keeps address spaces from
        leaking: a VMA from another ROM is simply not in this dict.
        """
        return self.functions.get(vma)


def load_image(name: str, path_text: str) -> Image:
    """Open one ROM. A missing file is a DependencyMissing, never a skip."""
    path = Path(path_text).expanduser()
    if not path.is_absolute():
        path = (Path.cwd() / path).resolve()
    if not path.exists():
        raise kit.DependencyMissing(
            f"ROM {name!r} not found at {path}; supply --rom \"{name}=<path>\" "
            f"explicitly. Nothing is downloaded and no ROM is assumed present.")
    data = path.read_bytes()
    if len(data) < 0xC0 or data[0xB2] != 0x96:
        raise kit.Unsupported(f"{name}: {path} does not look like a GBA ROM "
                              f"(bad Nintendo logo byte {data[0xB2]:#x})")
    return Image(
        name=name,
        path=path,
        sha256=hashlib.sha256(data).hexdigest(),
        title=data[0xA0:0xAC].split(b"\0")[0].decode("ascii", "replace"),
        game_code=data[0xAC:0xB0].decode("ascii", "replace"),
        maker=data[0xB0],
        data=data,
    )


def family_prefix(image: Image) -> str:
    """The title/maker/game-code prefix family, recorded as evidence only."""
    return f"{image.title.split()[0] if image.title else '?'}/{image.game_code}"


# ---------------------------------------------------------------------------
# Conservative code/data map for a bounded subsystem
# ---------------------------------------------------------------------------

def _objdump() -> str:
    found = shutil.which("arm-none-eabi-objdump")
    if not found:
        raise kit.DependencyMissing(
            "arm-none-eabi-objdump not available; cross-ROM comparison needs "
            "the same disassembler this project uses. Configure its path "
            "explicitly rather than substituting another decoder.")
    return found


def _listing(image: Image, vma: int, end: int, work: Path) -> dict:
    """Disassemble the reachable instructions of one span, at real VMAs.

    Reuses `match_families.instruction_image` so pool words are zeroed at their
    original VMAs and cannot seed Thumb-2 IT state in a foreign ROM. The blob is
    truncated at the same CODE_END this project uses, so both ROMs are decoded
    by one configuration.
    """
    seen, _, _ = families.flow(image.data, vma, end)
    instructions = {pc: item[1] for pc, item in seen.items()}
    image_blob = image.data[:probe.CODE_END - ROM_BASE]
    path = work / f"{image.name}_{vma:08x}.bin"
    path.write_bytes(families.instruction_image(image_blob, instructions))
    result = subprocess.run(
        [_objdump(), "-D", "-b", "binary", "-m", "armv4t", "-M", "force-thumb",
         f"--adjust-vma={ROM_BASE}", str(path)],
        text=True, capture_output=True, check=True)
    listing = {}
    for line in result.stdout.splitlines():
        m = re.match(r"^\s*([0-9a-f]+):\s+(?:[0-9a-f]{4}\s+){1,2}\s*([a-z][a-z0-9.]*)\s*(.*?)\s*$", line)
        if m:
            listing[int(m[1], 16)] = (m[2], m[3].split("@")[0].strip())
    return listing


@dataclass
class Span:
    """One conservatively-mapped function body, or an UNKNOWN boundary."""
    image: str
    vma: int
    end: int | None
    boundary: str                       # "MAPPED" or "UNKNOWN"
    reason: str = ""
    listing: dict = field(default_factory=dict)
    flow: dict = field(default_factory=dict)      # pc -> (halfword, size, kind)
    edges: list = field(default_factory=list)
    pools: list = field(default_factory=list)
    tokens: list = field(default_factory=list)
    fingerprint: str = ""
    complete: bool = False
    limitations: list = field(default_factory=list)
    image_bytes: bytes = field(default_factory=bytes, repr=False)
    project_symbol: str | None = None
    callee_list: list = field(default_factory=list)

    @property
    def length(self) -> int | None:
        return None if self.end is None else self.end - self.vma

    @property
    def file_offset(self) -> int:
        return self.vma - ROM_BASE

    @property
    def callees(self) -> list:
        return self.callee_list

    def to_json(self) -> dict:
        return {
            "image": self.image, "vma": f"{self.vma:#010x}",
            "file_offset": f"{self.file_offset:#010x}",
            "end": None if self.end is None else f"{self.end:#010x}",
            "length": self.length, "boundary": self.boundary,
            "reason": self.reason, "fingerprint": self.fingerprint,
            "complete": self.complete, "limitations": self.limitations,
            "project_symbol": self.project_symbol,
            "callees": [f"{t:#010x}" for _, t in self.callee_list],
        }


def _half(image: Image, pc: int) -> int:
    off = pc - ROM_BASE
    if off < 0 or off + 2 > len(image.data):
        raise ValueError(f"{image.name}: instruction outside this ROM at {pc:#x}")
    return int.from_bytes(image.data[off:off + 2], "little")


def _callees(image: Image, flow: dict) -> list:
    """Absolute callee VMAs of BL instructions, from the shared flow walk.

    `flow` already classified each pc as call/branch/cond/return/next; this only
    resolves the 23-bit Thumb BL target, so there is still exactly one decoder.
    """
    out = []
    for pc, (_, size, kind) in sorted(flow.items()):
        if kind != "call":
            continue
        hi, lo = _half(image, pc), _half(image, pc + 2)
        target = pc + 4 + families.sign(lo & 0x7FF, 11) * 2 + ((hi & 0x7FF) << 12)
        out.append((pc, target))
    return out


def map_span(image: Image, vma: int, end_hint: int | None, work: Path) -> Span:
    """Map one bounded span conservatively.

    `end_hint` is an operator-chosen upper bound for the search, never an
    assertion of the boundary. A boundary is accepted only when the reachability
    walk is completely clean: every branch lands inside, no indirect transfer,
    no pool/reachable overlap, and the last reachable instruction is a return.
    Otherwise the span stays UNKNOWN.
    """
    if vma not in image.functions:
        return Span(image=image.name, vma=vma, end=None, boundary=B_UNKNOWN,
                    reason=f"no entry point for {vma:#x} in this ROM's own map "
                           f"(known entries: {len(image.functions)})")
    known_end = image.functions[vma]
    if known_end is None:
        # A seed that was supplied but did not walk cleanly. Present, but with
        # no established boundary: reported UNKNOWN, never sized by a guess.
        return Span(image=image.name, vma=vma, end=None, boundary=B_UNKNOWN,
                    reason=f"seed {vma:#x} was supplied but no clean walk to a "
                           f"return was found; the boundary is not established")
    end = min(known_end, end_hint) if end_hint else known_end
    if end <= vma:
        return Span(image=image.name, vma=vma, end=None, boundary=B_UNKNOWN,
                    reason=f"empty span at {vma:#x}")
    flow, edges, failures = families.flow(image.data, vma, end)
    if failures:
        return Span(image=image.name, vma=vma, end=None, boundary=B_UNKNOWN,
                    reason="; ".join(failures))
    if not flow:
        return Span(image=image.name, vma=vma, end=None, boundary=B_UNKNOWN,
                    reason="no reachable instruction")
    last = max(flow) + flow[max(flow)][1]
    if flow[max(flow)][2] != "return":
        return Span(image=image.name, vma=vma, end=None, boundary=B_UNKNOWN,
                    reason=f"span does not end in a return (last pc {max(flow):#x})")
    # A function's bytes include the literal pool words it loads. Excluding them
    # would understate the span and drop the very constants this strategy
    # exists to compare, so the pool extends the resolved end -- but only by
    # words this span actually loads, never by scanning for one.
    pools = _pool_words(image, flow)
    pool_end = max((int(entry["pool_vma"], 16) + 4 for entry in pools), default=0)
    resolved_end = max(last, pool_end)
    try:
        listing = _listing(image, vma, resolved_end, work)
    except Exception as exc:                      # decoder trouble = UNKNOWN
        return Span(image=image.name, vma=vma, end=None, boundary=B_UNKNOWN,
                    reason=f"disassembly failed: {exc}")
    fp = families.fingerprint(image.data, vma, resolved_end, listing)
    return Span(
        image=image.name, vma=vma, end=resolved_end, boundary="MAPPED",
        reason="clean Thumb-1 reachability, terminates in a return"
               + (f"; literal pool extended to {resolved_end:#010x}" if pool_end else ""),
        listing=listing, flow=flow, edges=fp["edges"], pools=pools,
        tokens=fp["tokens"], fingerprint=fp["fingerprint"],
        complete=fp["complete"], limitations=fp["limitations"],
        image_bytes=image.read(vma, resolved_end),
        callee_list=_callees(image, flow),
    )


def _pool_words(image: Image, flow: dict) -> list:
    """Literal pool words this span loads, with an explicit role."""
    out = []
    for pc, (h, size, _kind) in sorted(flow.items()):
        if h & 0xF800 != 0x4800:
            continue
        pool = ((pc + 4) & ~3) + (h & 255) * 4
        try:
            word = struct.unpack_from("<I", image.data, pool - ROM_BASE)[0]
        except struct.error:
            continue
        out.append({"pool_vma": f"{pool:#010x}", "load_pc": f"{pc:#010x}",
                    "value": f"{word:#010x}", "role": literal_role(word)})
    return out


def literal_role(word: int) -> str:
    """Classify a literal. 'POINTER' means it names an address, not a number."""
    if 0x02000000 <= word < 0x0A000000 or 0x08000000 <= word < 0x0A000000:
        return "POINTER"
    if word == 0:
        return "ZERO"
    if word <= 0xFF:
        return "U8"
    if word <= 0xFFFF:
        return "U16"
    return "CONST"


# ---------------------------------------------------------------------------
# Independent correspondence features
# ---------------------------------------------------------------------------

def _ratio(a: list, b: list) -> float:
    if not a and not b:
        return 1.0
    if not a or not b:
        return 0.0
    return difflib.SequenceMatcher(None, a, b).ratio()


def _jaccard(a: set, b: set) -> float:
    if not a and not b:
        return 1.0
    if not a or not b:
        return 0.0
    return len(a & b) / len(a | b)


def access_widths(tokens: list) -> list:
    """Data-access width per token: the shape of the data traffic, not its data."""
    table = {"ldr": "W", "str": "W", "ldrh": "H", "strh": "H",
             "ldrb": "B", "strb": "B", "ldrsb": "SB", "ldrsb2": "SB",
             "ldrh3": "H", "strh2": "H", "ldrb2": "B", "strb2": "B"}
    return [table.get(t.split(" ")[0], "-") for t in tokens]


def features(a: Span, b: Span) -> dict:
    """Six independent features. Each is separately reported and separately
    thresholdable; the class is decided by how many agree, not by a sum."""
    # `size` and `calls` compare scalars and are deliberately weak signals; they
    # exist so a strong shape match cannot hide behind a differing length.
    feats = {
        "tokens": _ratio(a.tokens, b.tokens),
        "cfg": 0.5 * _jaccard({tuple(e) for e in a.edges}, {tuple(e) for e in b.edges})
               + 0.5 * _ratio([tuple(e) for e in a.edges], [tuple(e) for e in b.edges]),
        "size": _ratio([a.length or 0], [b.length or 0]),
        "calls": _ratio([t - a.vma for _, t in a.callees],
                        [t - b.vma for _, t in b.callees]),
        "literals": _ratio([p["role"] for p in a.pools], [p["role"] for p in b.pools]),
        "widths": _ratio(access_widths(a.tokens), access_widths(b.tokens)),
    }
    return {k: round(v, 4) for k, v in feats.items()}





def classify(a: Span, b: Span, feats: dict) -> tuple:
    """Confidence class and the reason for it.

    EXACT requires structural identity (normalized tokens AND the CFG), or
    byte identity. WEAK requires >= WEAK_MIN_FEATURES independent features above
    threshold. No single feature can promote a pair on its own.
    """
    # Fail closed: an UNKNOWN boundary is never "identical" to anything. Two
    # unmapped spans both have length None and empty bytes, and comparing those
    # would manufacture a byte-exact match out of two absences.
    if a.boundary != "MAPPED" or b.boundary != "MAPPED":
        return (C_NONE, "boundary not established on "
                f"{a.image if a.boundary != 'MAPPED' else b.image}; an UNKNOWN span "
                f"cannot be compared, let alone matched")
    if not a.image_bytes or not b.image_bytes:
        return C_NONE, "one span has no bytes; refusing to call it identical"
    if a.length == b.length and a.image_bytes == b.image_bytes:
        return C_EXACT_BYTES, f"byte-identical {a.length}-byte span"
    if a.tokens and a.tokens == b.tokens and [list(e) for e in a.edges] == [list(e) for e in b.edges]:
        return C_EXACT_NORM, "identical normalized token sequence and identical CFG edges"
    strong = sorted(k for k, v in feats.items() if v >= WEAK_FEATURE_FLOOR)
    if len(strong) >= WEAK_MIN_FEATURES:
        return (C_WEAK,
                f"{len(strong)}/{len(FEATURE_NAMES)} independent features at or above "
                f"{WEAK_FEATURE_FLOOR}: {', '.join(strong)}")
    return C_NONE, f"only {len(strong)} feature(s) at or above {WEAK_FEATURE_FLOOR}"


# ---------------------------------------------------------------------------
# Diffing: what changed between two mapped spans
# ---------------------------------------------------------------------------

WIDTH_OPS = {"ldr": "word", "str": "word", "ldrh": "half", "strh": "half",
             "ldrb": "byte", "strb": "byte", "ldrsb": "byte"}


def _cond_of(token: str) -> str | None:
    m = re.match(r"^(?:cond\d+|branch|call|return)$", token)
    return token if m else None


def diff_spans(a: Span, b: Span) -> dict:
    """Changed constants, offsets, branch conditions and call structure."""
    ops = difflib.SequenceMatcher(None, a.tokens, b.tokens).get_opcodes()
    changed_tokens, changed_conds = [], []
    for tag, i1, i2, j1, j2 in ops:
        if tag == "equal":
            continue
        for k in range(max(i2 - i1, j2 - j1)):
            tok = a.tokens[i1 + k] if i1 + k < i2 else (b.tokens[j1 + k] if j1 + k < j2 else "")
            changed_tokens.append({"at": f"{a.vma + 2 * (i1 + k):#010x}", "a": tok,
                                   "b": b.tokens[j1 + k] if j1 + k < j2 else None,
                                   "op": tag})
            cond = _cond_of(tok)
            if cond:
                changed_conds.append({"kind": cond, "at": f"{a.vma + 2 * (i1 + k):#010x}"})

    literals = []
    for pa, pb in zip(a.pools, b.pools):
        va, vb = int(pa["value"], 16), int(pb["value"], 16)
        if va != vb:
            literals.append({
                "a": {"rom": a.image, "pool_vma": pa["pool_vma"], "value": pa["value"], "role": pa["role"]},
                "b": {"rom": b.image, "pool_vma": pb["pool_vma"], "value": pb["value"], "role": pb["role"]},
                "delta": vb - va,
            })
    return {
        "changed_instructions": changed_tokens,
        "changed_branch_conditions": changed_conds,
        "changed_literals": literals,
        "call_structure": {
            "a_callees": [f"{t:#010x}" for _, t in a.callees],
            "b_callees": [f"{t:#010x}" for _, t in b.callees],
            "call_count_delta": len(b.callees) - len(a.callees),
            "relative_call_offsets_equal":
                [t - a.vma for _, t in a.callees] == [t - b.vma for _, t in b.callees],
        },
        "length_delta": (b.length or 0) - (a.length or 0),
    }


def form_hypotheses(a: Span, b: Span, diff: dict, cls: str) -> list:
    """Turn the measured diff into explicit, checkable source hypotheses."""
    notes = []
    if cls == C_EXACT_BYTES:
        notes.append("spans are byte-identical: same instructions, same literals, "
                     "same addresses -- a shared source body is plausible, but "
                     "identity of bytes is not proof of a shared prototype.")
    if diff["length_delta"]:
        notes.append(f"length differs by {diff['length_delta']:+d} bytes: a source-level "
                     f"change (added/removed statement, inlining or an extra spill), "
                     f"not merely a relocation.")
    for lit in diff["changed_literals"]:
        if lit["a"]["role"] == "POINTER" == lit["b"]["role"]:
            notes.append(f"pointer literal moved {lit['delta']:+#x} "
                         f"({lit['a']['value']} -> {lit['b']['value']}); if the two games "
                         f"keep the same record layout this is a data relocation "
                         f"(different RAM region / build-time symbol), not a source change.")
        elif lit["a"]["role"] == lit["b"]["role"]:
            notes.append(f"same-role literal {lit['a']['role']} changed "
                         f"{lit['a']['value']} -> {lit['b']['value']} "
                         f"(delta {lit['delta']:+#x}): a plausible source constant "
                         f"difference (feature flag, size, or versioned table).")
        else:
            notes.append(f"literal changed role {lit['a']['role']} -> {lit['b']['role']} "
                         f"({lit['a']['value']} -> {lit['b']['value']}): the two bodies "
                         f"do not agree on what kind of thing is being named.")
    if diff["call_structure"]["call_count_delta"]:
        notes.append(f"call count differs by {diff['call_structure']['call_count_delta']:+d}: "
                     f"a helper was inlined, split or added on one side.")
    elif not diff["call_structure"]["relative_call_offsets_equal"]:
        notes.append("same call count but different relative callee offsets: the helper "
                     "boundary moved, so the neighbouring code was reorganised.")
    if diff["changed_branch_conditions"]:
        notes.append(f"{len(diff['changed_branch_conditions'])} branch condition(s) differ: "
                     f"the compared code is not a pure relocation.")
    if cls == C_NONE:
        notes.append("no correspondence above threshold: this pairing answers no "
                     "lifting question and must not be reported as shared code.")
    return notes


# ---------------------------------------------------------------------------
# Research leads
# ---------------------------------------------------------------------------

def _lead(a: Span, b: Span, cls: str, feats: dict, why: str, diff: dict,
          image_a: Image, image_b: Image) -> dict:
    """A research lead. Note what it is NOT: it carries no symbol for this
    project's closure and no ABI claim."""
    return {
        "confidence": cls,
        "why": why,
        "features": feats,
        "features_above_threshold": sorted(k for k, v in feats.items()
                                           if v >= WEAK_FEATURE_FLOOR),
        "project": {
            "image": a.image, "rom_sha256": image_a.sha256, "title": image_a.title,
            "game_code": image_a.game_code, "vma": f"{a.vma:#010x}",
            "file_offset": f"{a.file_offset:#010x}", "length": a.length,
        },
        "comparison": {
            "image": b.image, "rom_sha256": image_b.sha256, "title": image_b.title,
            "game_code": image_b.game_code, "vma": f"{b.vma:#010x}",
            "file_offset": f"{b.vma - ROM_BASE:#010x}", "length": b.length,
        },
        "diff": diff,
        "hypotheses": form_hypotheses(a, b, diff, cls),
        # The boundary, made machine-checkable rather than a comment.
        "symbol_closure": {
            "may_enter_project_closure": False,
            "project_symbol": None,
            "abi_evidence": "none -- the other game's prototype is not ABI evidence here",
            "reason": BOUNDARY_STATEMENT,
        },
    }


def _disambiguate(leads: list) -> list:
    """Decide whether each pairing is a settled correspondence or an ambiguity.

    Several bodies can normalize to the same token stream -- three 12-byte
    accessors that differ only in one immediate do exactly that. Reporting all
    of them as separate correspondences would overstate what was measured, so a
    pairing counts as UNAMBIGUOUS only when it STRICTLY beats every rival on
    both sides. A tie is not a disambiguation: two bodies that score identically
    have not been told apart by anything measured here, and calling one of them
    "the" counterpart would invent an ordering the evidence does not contain.
    Ambiguous leads are kept and labelled, never dropped and never presented as
    established.
    """
    def strength(lead) -> tuple:
        return (lead["confidence"], sum(lead["features"].values()))

    rivals_project, rivals_other = {}, {}
    for lead in leads:
        for table, key in ((rivals_project, (lead["project"]["vma"],)),
                           (rivals_other, (lead["comparison"]["image"],
                                           lead["comparison"]["vma"]))):
            table.setdefault(key, []).append(lead)
    for lead in leads:
        p_key = (lead["project"]["vma"],)
        o_key = (lead["comparison"]["image"], lead["comparison"]["vma"])
        mine = strength(lead)
        # Strictly stronger on both sides, or there is a rival at the same score.
        top_p = max(strength(other) for other in rivals_project[p_key])
        top_o = max(strength(other) for other in rivals_other[o_key])
        unique = (mine > top_p or len(rivals_project[p_key]) == 1) and \
                 (mine > top_o or len(rivals_other[o_key]) == 1)
        tied_project = sorted(other["comparison"]["vma"]
                              for other in rivals_project[p_key]
                              if other is not lead and strength(other) >= mine)
        tied_other = sorted(other["project"]["vma"]
                            for other in rivals_other[o_key]
                            if other is not lead and strength(other) >= mine)
        lead["disambiguation"] = {
            "unambiguous": unique,
            "tied_project_alternatives": tied_project,
            "tied_comparison_alternatives": tied_other,
            "note": ("strictly best on both sides: a single correspondence"
                     if unique else
                     "ambiguous: this pairing is not strictly better than "
                     f"{', '.join(tied_project or tied_other)} -- the shape evidence "
                     f"does not single out one counterpart, so identifying it needs "
                     f"a discriminating caller, data consumer or literal"),
        }
    leads.sort(key=lambda lead: (not lead["disambiguation"]["unambiguous"],
                                 lead["confidence"] != C_EXACT_BYTES,
                                 lead["confidence"] != C_EXACT_NORM,
                                 -sum(lead["features"].values())))
    return leads


# ---------------------------------------------------------------------------
# Run
# ---------------------------------------------------------------------------

def _project_names() -> dict:
    """vma -> this project's own name, read from the project's own corpus report."""
    report = ROOT / "build/era-corpus/ready-report.json"
    if not report.exists():
        return {}
    data = json.loads(report.read_text())
    out = {}
    for row in data.get("results", []):
        vma = row.get("vma")
        if vma:
            out[int(vma, 16)] = row.get("alias_of") or row.get("name")
    return out


def parse_rom_specs(specs: list) -> list:
    if not specs:
        raise kit.DependencyMissing(
            "no --rom given; this tool never assumes a second ROM exists. Pass "
            '--rom "<name>=<path>" for the project ROM and one other ROM.')
    out = []
    for spec in specs:
        if "=" not in spec:
            raise ValueError(f'--rom wants "name=path", got {spec!r}')
        name, path = spec.split("=", 1)
        if not name or not path:
            raise ValueError(f'--rom wants "name=path", got {spec!r}')
        if name == PROJECT:
            out.append(("project", load_image(name, path)))
        else:
            out.append(("other", load_image(name, path)))
    return out


def run(args) -> int:
    images = parse_rom_specs(args.rom)
    project = next((im for role, im in images if role == "project"), None)
    if project is None:
        raise kit.DependencyMissing(
            f'the project ROM must be present as --rom "{PROJECT}=<path>"; a cross-ROM '
            f"comparison with no project side has no address space to compare against")
    others = [im for role, im in images if role == "other"]
    if not others:
        raise kit.DependencyMissing(
            "no comparison ROM given; cross-ROM archaeology needs a second ROM. Pass "
            '--rom "<name>=<path>".')

    for _role, image in images:
        # Every ROM's address space starts empty and is filled only from its own
        # bytes. Nothing is copied between spaces.
        image.functions = {}
    project.functions = dict(probe.rom_functions())
    # --- seeds, resolved per ROM and never mixed ---------------------------
    # A bare `--vma` seeds the PROJECT's space. `--vma other=...` seeds ONLY
    # that ROM's space. A foreign address never enters the project's seed list,
    # even when the two games happen to share a VMA.
    project_seeds = list(SUBSYSTEMS.get(args.subsystem, []))
    foreign_seeds = {other.name: [] for other in others}
    for item in args.vma:
        rom_name, _, vma_text = item.partition("=")
        if not rom_name:
            rom_name, vma_text = PROJECT, item
        try:
            vma = int(vma_text, 16)
        except ValueError:
            raise ValueError(f"--vma {item!r} is not a hex address "
                             f"(did you mean \"<rom>={item}\"?)") from None
        if rom_name == PROJECT:
            project_seeds.append(vma)
        elif rom_name in foreign_seeds:
            foreign_seeds[rom_name].append(vma)
        else:
            raise ValueError(f"--vma {item!r} names ROM {rom_name!r}, which was not "
                             f"passed to --rom (have: {sorted(foreign_seeds)}, "
                             f"{PROJECT!r})")
    if args.vma_rom and args.vma_rom not in foreign_seeds:
        raise ValueError(f"--vma-rom {args.vma_rom!r} names a ROM that was not "
                         f"passed to --rom (have: {sorted(foreign_seeds)})")
    if args.subsystem not in SUBSYSTEMS and not project_seeds:
        raise ValueError(f"unknown subsystem {args.subsystem!r} and no project --vma "
                         f"seeds; known subsystems: {', '.join(sorted(SUBSYSTEMS))}")
    project_seeds = sorted(set(project_seeds))

    project.functions = dict(probe.rom_functions())
    for other in others:
        # A foreign ROM has no trusted function map. Its entries come only from
        # seeds measured in THAT ROM's own bytes. A seed that does not walk
        # cleanly is recorded as a None boundary -- present and UNKNOWN, which
        # is not the same as absent, and never guessed at.
        for seed in sorted(set(foreign_seeds[other.name])):
            walked, _edges, failures = families.flow(other.data, seed,
                                                    seed + args.max_span)
            if not failures and walked and walked[max(walked)][2] == "return":
                other.functions[seed] = max(walked) + walked[max(walked)][1]
                print(f"[seed] {other.name} {seed:#010x}: walks cleanly to a return "
                      f"at {other.functions[seed]:#010x}")
            else:
                other.functions[seed] = None
                print(f"[seed] {other.name} {seed:#010x}: no clean walk to a return "
                      f"within {args.max_span:#x} -- boundary stays UNKNOWN")

    out_dir = Path(args.out) if args.out else None
    work = (out_dir or (ROOT / "build/cross-rom/work")) / "objdump"
    work.mkdir(parents=True, exist_ok=True)

    names = _project_names()
    # `--out` IS the run directory, and an explicit `--run-id` names a child
    # of it. Passing `root=out_dir` with an auto-generated run id instead wrote
    # to a timestamped CHILD of the path the caller named, leaving it empty.
    run_dir = None
    if out_dir is not None:
        run_dir = out_dir / args.run_id if args.run_id else out_dir
    writer = kit.out_writer(
        STRATEGY, run_dir,
        extra={"subsystem": args.subsystem, "boundary": BOUNDARY_STATEMENT,
               "run_id": args.run_id, "roms": []})
    writer.open()

    rom_records = []
    for role, image in images:
        record = {"role": role, "name": image.name, "path": str(image.path),
                  "sha256": image.sha256, "title": image.title,
                  "game_code": image.game_code, "maker": f"{image.maker:#04x}",
                  "family_prefix": family_prefix(image),
                  "address_space": list(image.space),
                  "bytes": len(image.data), "mapped_entries": len(image.functions)}
        rom_records.append(record)
        writer.write_json(f"roms/{image.name}.json", record)
    writer._run["roms"] = rom_records

    # --- map the project side -------------------------------------------
    project_spans = []
    for vma in project_seeds:
        span = map_span(project, vma, None, work)
        span.project_symbol = names.get(vma)
        project_spans.append(span)
        print(f"[map] {PROJECT} {span.vma:#010x} {span.boundary}"
              + (f" len={span.length}" if span.end else f" -- {span.reason}")
              + (f" ({span.project_symbol})" if span.project_symbol else ""))

    # --- map the comparison sides ---------------------------------------
    other_spans = []
    for other in others:
        for vma in sorted(other.functions):
            span = map_span(other, vma, None, work)
            span.project_symbol = None
            other_spans.append(span)
            print(f"[map] {other.name} {span.vma:#010x} {span.boundary}"
                  + (f" len={span.length}" if span.end else f" -- {span.reason}"))
        if not other.functions:
            print(f"[map] {other.name}: no seed walked cleanly; every candidate in this "
                  f"space is UNKNOWN (boundaries are not guessed)")

    # --- compare ----------------------------------------------------------
    leads = []
    for a in project_spans:
        if a.boundary != "MAPPED":
            continue
        for b in other_spans:
            if b.boundary != "MAPPED":
                continue
            feats = features(a, b)
            cls, why = classify(a, b, feats)
            if cls == C_NONE:
                continue
            diff = diff_spans(a, b)
            image_b = next(o for o in others if o.name == b.image)
            leads.append(_lead(a, b, cls, feats, why, diff, project, image_b))
    # --- disambiguate -----------------------------------------------------
    leads = _disambiguate(leads)

    counts = Counter(lead["confidence"] for lead in leads)
    payload = {
        "subsystem": args.subsystem,
        "roms": rom_records,
        "boundary": BOUNDARY_STATEMENT,
        "project_seeds": [f"{v:#010x}" for v in project_seeds],
        "foreign_seeds": {name: [f"{v:#010x}" for v in vs]
                          for name, vs in foreign_seeds.items()},
        "project_spans": [s.to_json() for s in project_spans],
        "other_spans": [s.to_json() for s in other_spans],
        "lead_count": len(leads),
        "confidence_counts": {k: counts.get(k, 0) for k in
                              (C_EXACT_BYTES, C_EXACT_NORM, C_WEAK)},
        "leads": leads,
        "unmapped": [s.to_json() for s in project_spans + other_spans
                     if s.boundary != "MAPPED"],
    }
    writer.write_json("archaeology.json", payload)
    writer.write_json("research_leads.json", {"leads": leads, "boundary": BOUNDARY_STATEMENT})

    # Optional: validate a candidate C body against THIS project's own bytes.
    validation = None
    if args.validate_c:
        validation = validate_candidate(args, project, names)
        writer.write_json("validation.json", validation)

    draft_status = kit.NO_EXACT_DRAFT
    summary = {
        "subsystem": args.subsystem,
        "project_rom": f"{project.name} {project.game_code}",
        "comparison_roms": ", ".join(f"{o.name} {o.game_code}" for o in others),
        "project_seed_spans": len(project_spans),
        "comparison_seed_spans": len(other_spans),
        "unmapped_spans": len(payload["unmapped"]),
        "leads_total": len(leads),
        "leads_exact_bytes": counts.get(C_EXACT_BYTES, 0),
        "leads_exact_normalized": counts.get(C_EXACT_NORM, 0),
        "leads_weak": counts.get(C_WEAK, 0),
        "leads_unambiguous": sum(1 for lead in leads
                                 if lead["disambiguation"]["unambiguous"]),
        "leads_ambiguous": sum(1 for lead in leads
                               if not lead["disambiguation"]["unambiguous"]),
        "validation_status": (validation or {}).get("status", "not run"),
        "draft_status": draft_status,
        "integrated_status": "not run",
        "link_accepted": "not run",
        "notes": [
            BOUNDARY_STATEMENT,
            "no cross-ROM label was written to this project's symbol table; "
            "leads carry hashes, VMAs and file offsets only",
            "a WEAK_SIMILARITY lead is a hypothesis, never a match: it is only "
            "reported when several independent features agree",
        ],
    }
    writer.record({"kind": "archaeology", "status": draft_status,
                   "leads": len(leads),
                   "counts": {k: counts.get(k, 0) for k in
                              (C_EXACT_BYTES, C_EXACT_NORM, C_WEAK)}})
    writer.finish(summary)

    print_report(payload)
    print(f"\n{STRATEGY}: leads={len(leads)} "
          f"(exact_bytes={counts.get(C_EXACT_BYTES, 0)} "
          f"exact_normalized={counts.get(C_EXACT_NORM, 0)} "
          f"weak={counts.get(C_WEAK, 0)}) -> {writer.path('research_leads.json')}")
    print(f"{STRATEGY}: boundary enforced -- {BOUNDARY_STATEMENT}")
    if args.json:
        Path(args.json).write_text(json.dumps(
            {"summary": summary, "archaeology": payload, "validation": validation},
            indent=2, sort_keys=True) + "\n")
        print(f"{STRATEGY}: json -> {args.json}")
    return 0



def print_report(payload: dict) -> None:
    print("\n=== ROMs (separate address spaces, explicit hashes) ===")
    for rec in payload["roms"]:
        print(f"  {rec['role']:7s} {rec['name']:6s} {rec['game_code']} "
              f"{rec['title']!r} sha256={rec['sha256'][:16]}… "
              f"entries={rec['mapped_entries']}")
    print("\n=== Mapped spans ===")
    for span in payload["project_spans"] + payload["other_spans"]:
        end = span["end"] or "?"
        print(f"  {span['image']:6s} vma={span['vma']} end={end} "
              f"len={span['length']} {span['boundary']}"
              + (f" -- {span['reason']}" if span["reason"] else ""))
    print("\n=== Correspondences (ranked; class is decided by several features) ===")
    if not payload["leads"]:
        print("  no correspondence above threshold")
    for lead in payload["leads"]:
        dis = lead["disambiguation"]
        mark = "UNAMBIGUOUS" if dis["unambiguous"] else "AMBIGUOUS"
        print(f"  [{lead['confidence']}/{mark}] project {lead['project']['vma']} "
              f"(off {lead['project']['file_offset']}) <-> "
              f"{lead['comparison']['image']} {lead['comparison']['vma']} "
              f"(off {lead['comparison']['file_offset']})")
        print("      features: " + "  ".join(f"{k}={v}" for k, v in lead["features"].items()))
        print(f"      why: {lead['why']}")
        print(f"      disambiguation: {dis['note']}")
        for note in lead["hypotheses"]:
            print(f"      hypothesis: {note}")
        print(f"      symbol closure: may_enter_project_closure="
              f"{lead['symbol_closure']['may_enter_project_closure']}")


def validate_candidate(args, project: Image, names: dict) -> dict:
    """Compile a scratch C body and score it against THIS project's own span.

    This is the only path by which a comparison can influence a byte: the
    foreign ROM contributes nothing but the hypothesis. agbcc and this project's
    ROM are the oracle.
    """
    if not args.vma:
        raise ValueError("--validate-c needs --vma to name this project's own span")
    vma = int(args.vma[0].split("=", 1)[-1], 16)
    if vma not in project.functions:
        raise kit.Unsupported(f"{vma:#x} is not a known entry in the project ROM")
    end = project.functions[vma]
    source = Path(args.validate_c).read_text()
    contract = kit.contract_for_span(f"crossrom_{vma:08x}", vma, end,
                                     return_type="unknown",
                                     abi_note="scored against this project's own "
                                              "instructions and agbcc; the other ROM's "
                                              "prototype is NOT ABI evidence")
    kit.require_toolchain()
    with kit._temp_dir(None) as scratch:
        adapter = kit.CandidateAdapter(scratch, meter=kit.BudgetMeter(kit.Budget(compiles=8)))
        score = adapter.evaluate(source, contract.name, contract, index=0)
    return {"vma": f"{vma:#010x}", "end": f"{end:#010x}", "status": score.status,
            "matched_bytes": score.matched_bytes, "rom_bytes": score.rom_bytes,
            "detail": score.detail, "oracle": "project ROM + unchanged agbcc",
            "note": "a comparison ROM never supplied the ABI or the expected bytes"}


# ---------------------------------------------------------------------------
# Self-test
# ---------------------------------------------------------------------------

def _tiny(rom_id: str, code_at: int, body: bytes) -> Image:
    """A synthetic ROM image in its own address space, for tests only."""
    data = bytearray(b"\x00" * 0x400)
    data[code_at:code_at + len(body)] = body
    image = Image(name=rom_id, path=Path(f"<synthetic {rom_id}>"),
                  sha256=hashlib.sha256(bytes(data)).hexdigest(),
                  title=f"SYNTH {rom_id}", game_code=f"SY{rom_id[0].upper()}",
                  maker=0x00, data=bytes(data))
    return image


def self_test() -> int:
    print(f"running {STRATEGY} self-test...")
    passed = total = 0

    def check(name: str, ok: bool, detail: str = "") -> None:
        nonlocal passed, total
        total += 1
        if ok:
            passed += 1
            print(f"  PASS {name}")
        else:
            print(f"  FAIL {name}{': ' + detail if detail else ''}")

    # 1. a missing ROM is DependencyMissing, not a crash and not a skip
    check("missing ROM raises DependencyMissing",
          kit.raises(kit.DependencyMissing,
                     lambda: load_image("ghost", "/nonexistent/ghost.gba")))
    check("missing ROM with no --rom at all raises DependencyMissing",
          kit.raises(kit.DependencyMissing, lambda: parse_rom_specs([])))
    check("malformed --rom spec is a usage error, not a silent skip",
          kit.raises(ValueError, lambda: parse_rom_specs(["no-equals-sign"])))

    # 2. address spaces do not leak: a VMA from ROM B is not resolvable in ROM A
    a_img = _tiny("A", 0x100, b"")
    b_img = _tiny("B", 0x300, b"")
    a_img.functions = {0x08000100: 0x08000110}
    b_img.functions = {0x08000300: 0x08000310}
    check("spaces are distinct identities",
          a_img.space != b_img.space and a_img.space[0] == "A" and b_img.space[0] == "B")
    check("a VMA from ROM B does not resolve in ROM A's map",
          a_img.resolve(0x08000300) is None)
    check("a VMA from ROM B does not resolve as a span in ROM A",
          map_span(a_img, 0x08000300, None, Path("/tmp")).boundary == B_UNKNOWN)
    check("a VMA resolves only inside its own space",
          a_img.resolve(0x08000100) == 0x08000110 and
          b_img.resolve(0x08000100) is None)
    check("reading a span outside this ROM raises rather than clamps",
          kit.raises(ValueError, lambda: a_img.read(0x08000900, 0x08000910)))
    check("reading a span this ROM does cover returns bytes",
          a_img.read(0x08000100, 0x08000110) == b"\x00" * 16)

    # 3. an unresolvable control-flow boundary is UNKNOWN, never guessed
    #    `bx r3` with no matching pop is an indirect transfer flow() rejects.
    indirect = _tiny("I", 0x100, b"\x18\x47\x00\xbf\x00\xbf\x00\xbf")
    indirect.functions = {0x08000100: 0x08000108}
    bad = map_span(indirect, 0x08000100, None, Path("/tmp"))
    check("indirect transfer leaves the boundary UNKNOWN",
          bad.boundary == B_UNKNOWN and bad.end is None,
          f"got {bad.boundary}/{bad.end}")
    check("UNKNOWN boundary carries a reason", "indirect transfer" in bad.reason, bad.reason)
    # A span that does not terminate in a return is also UNKNOWN.
    noexit = _tiny("N", 0x100, b"\x01\x20\x02\xe0\x00\x00")
    noexit.functions = {0x08000100: 0x08000106}
    bad2 = map_span(noexit, 0x08000100, None, Path("/tmp"))
    check("span not ending in a return stays UNKNOWN", bad2.boundary == B_UNKNOWN)
    # An entry this ROM's own map does not contain is UNKNOWN, never invented.
    check("a VMA with no entry in this ROM's own map is UNKNOWN",
          map_span(a_img, 0x08000900, None, Path("/tmp")).boundary == B_UNKNOWN)

    # 4. exact normalized match and weak similarity are DIFFERENT classes
    #    `movs r0,#1; bx lr` -- a complete, returning Thumb-1 body.
    leaf = bytes.fromhex("01207047")
    wa = ROOT / "build/cross-rom/work/self-test"
    wa.mkdir(parents=True, exist_ok=True)
    sa_img = _tiny("A", 0x100, leaf)
    sb_img = _tiny("B", 0x200, leaf)
    sa_img.functions = {0x08000100: 0x08000104}
    sb_img.functions = {0x08000200: 0x08000204}
    sa = map_span(sa_img, 0x08000100, None, wa)
    sb = map_span(sb_img, 0x08000200, None, wa)
    if shutil.which("arm-none-eabi-objdump") is None:
        check("synthetic spans map cleanly in both spaces", False,
              "arm-none-eabi-objdump missing; the mapping path is untested")
        feats = {}
    else:
        check("synthetic spans map cleanly in both spaces",
              sa.boundary == "MAPPED" and sb.boundary == "MAPPED",
              f"{sa.boundary}/{sa.reason} {sb.boundary}/{sb.reason}")
        check("the same body maps at a different VMA in another space",
              sb.boundary == "MAPPED" and sa.vma != sb.vma)
        feats = features(sa, sb)
        cls, _why = classify(sa, sb, feats)
        check("identical bodies are EXACT_BYTES",
              cls == C_EXACT_BYTES, f"{cls} {feats}")
        # Same shape, one different immediate: normalization keeps the token
        # stream equal, but the bytes are NOT identical.
        sb2_img = _tiny("B", 0x200, bytes.fromhex("02207047"))
        sb2_img.functions = {0x08000200: 0x08000204}
        sb2 = map_span(sb2_img, 0x08000200, None, wa)
        f2 = features(sa, sb2)
        cls2, why2 = classify(sa, sb2, f2)
        check("a changed immediate is not EXACT_BYTES",
              cls2 != C_EXACT_BYTES, f"{cls2} {f2}")
        check("a changed immediate lands in EXACT_NORMALIZED, a class distinct "
              "from EXACT_BYTES",
              cls2 == C_EXACT_NORM, f"{cls2} {f2} {why2}")
        # A longer body: different tokens and a different CFG, so not exact.
        sb3_img = _tiny("B", 0x200, bytes.fromhex("092001207047"))
        sb3_img.functions = {0x08000200: 0x08000206}
        sb3 = map_span(sb3_img, 0x08000200, None, wa)
        f3 = features(sa, sb3)
        cls3, _why3 = classify(sa, sb3, f3)
        check("a longer body is neither EXACT_BYTES nor EXACT_NORMALIZED",
              cls3 not in (C_EXACT_BYTES, C_EXACT_NORM), f"{cls3} {f3}")
        check("the four confidence classes are distinct values",
              len({C_EXACT_BYTES, C_EXACT_NORM, C_WEAK, C_NONE}) == 4)
        # Regression: the span must include the literal pool word the function
        # loads. Excluding it silently understated the function and dropped the
        # very constants this strategy exists to compare.
        pool_img = _tiny("PL", 0x100, bytes.fromhex("00487047c0350003"))
        pool_img.functions = {0x08000100: 0x08000108}
        pooled = map_span(pool_img, 0x08000100, None, wa)
        check("a mapped span includes the literal pool words it loads",
              pooled.boundary == "MAPPED" and pooled.end == 0x08000108,
              f"{pooled.boundary} end={pooled.end and hex(pooled.end)} {pooled.reason}")
        check("the loaded pool word is reported with its role",
              [(p["value"], p["role"]) for p in pooled.pools] == [("0x030035c0", "POINTER")],
              str(pooled.pools))

    # 4b. regression: an UNKNOWN boundary must never compare as a match. Two
    #     unmapped spans share length None and empty bytes, which is an absence.
    u1 = map_span(_tiny("U1", 0x100, b""), 0x08000900, None, wa)
    u2 = map_span(_tiny("U2", 0x100, b""), 0x08000900, None, wa)
    check("two UNKNOWN spans do not classify as a byte-exact match",
          classify(u1, u2, features(u1, u2))[0] == C_NONE)
    check("an UNKNOWN span paired with a MAPPED one is not a match",
          classify(sa, u1, features(sa, u1))[0] == C_NONE)
    check("a zero-length span is never called byte-identical",
          classify(Span("Z", 0x08000100, 0x08000100, "MAPPED"),
                   Span("Z", 0x08000200, 0x08000200, "MAPPED"), {})[0] == C_NONE)

    # 5. ranking uses more than one independent feature
    check("six independent features are computed",
          set(FEATURE_NAMES) == {"tokens", "cfg", "size", "calls", "literals", "widths"})
    check("every feature is independently reported",
          bool(feats) and all(0.0 <= v <= 1.0 for v in feats.values()), str(feats))
    check("a single strong feature cannot promote a WEAK match",
          WEAK_MIN_FEATURES >= 3 and
          _classify_with({"tokens": 1.0, "cfg": 0.0, "size": 0.0, "calls": 0.0,
                          "literals": 0.0, "widths": 0.0}) == C_NONE)
    check("two strong features still cannot promote a WEAK match",
          _classify_with({"tokens": 1.0, "cfg": 1.0, "size": 0.0, "calls": 0.0,
                          "literals": 0.0, "widths": 0.0}) == C_NONE)
    check("agreement across several features yields WEAK_SIMILARITY",
          _classify_with({"tokens": 0.9, "cfg": 0.8, "size": 0.95, "calls": 0.0,
                          "literals": 0.0, "widths": 0.0}) == C_WEAK)

    # 6. cross-ROM labels never enter the project symbol closure. This is
    #    checked on the REAL _lead() output, not on a hand-written stub.
    if sa.boundary == "MAPPED" and sb.boundary == "MAPPED":
        f_real = features(sa, sb)
        c_real, w_real = classify(sa, sb, f_real)
        real_lead = _lead(sa, sb, c_real, f_real, w_real,
                          diff_spans(sa, sb), sa_img, sb_img)
        sc = real_lead["symbol_closure"]
        check("a real lead may not enter the project symbol closure",
              sc["may_enter_project_closure"] is False and sc["project_symbol"] is None,
              str(sc))
        check("a real lead offers no ABI evidence from the other ROM",
              "not ABI evidence" in sc["abi_evidence"], sc["abi_evidence"])
        check("a real lead carries the source ROM hash, VMA and file offset",
              bool(real_lead["comparison"]["rom_sha256"])
              and real_lead["comparison"]["vma"] == f"{sb.vma:#010x}"
              and real_lead["comparison"]["file_offset"] == f"{sb.vma - ROM_BASE:#010x}")
        check("a real lead states the boundary as machine-readable evidence",
              BOUNDARY_STATEMENT in sc["reason"])
        check("a foreign span never acquires this project's symbol",
              sb.project_symbol is None and sa.project_symbol is None)
        check("a lead built by the real path carries no disambiguation verdict "
              "until one is assigned",
              "disambiguation" not in real_lead)
    # Three bodies that normalize identically must be reported as AMBIGUOUS,
    # not as three settled correspondences.
    trio = []
    for idx, imm in enumerate((0x01, 0x02, 0x03)):
        offset = 0x100 + 0x40 * idx
        img = _tiny(f"T{idx}", offset, bytes.fromhex(f"{imm:02x}207047"))
        base = ROM_BASE + offset
        img.functions = {base: base + 4}
        trio.append(map_span(img, base, None, wa))
    if all(s.boundary == "MAPPED" for s in trio):
        trio_leads = []
        for s in trio:
            f_s = features(sa, s)
            c_s, w_s = classify(sa, s, f_s)
            trio_leads.append(_lead(sa, s, c_s, f_s, w_s, diff_spans(sa, s),
                                    sa_img, sb_img))
        _disambiguate(trio_leads)
        # One project body against three identical foreign bodies: nothing
        # measured here singles out a counterpart, so ALL THREE must be
        # ambiguous. Picking the first would invent an ordering.
        check("mutually indistinguishable bodies are all reported ambiguous",
              all(not lead["disambiguation"]["unambiguous"] for lead in trio_leads),
              str([lead["disambiguation"]["unambiguous"] for lead in trio_leads]))
        check("an ambiguous lead names its tied alternatives",
              all(lead["disambiguation"]["tied_project_alternatives"]
                  for lead in trio_leads),
              str([lead["disambiguation"]["tied_project_alternatives"]
                   for lead in trio_leads]))
        # Control: with a single candidate on each side, the pairing is settled.
        solo = _lead(sa, sb, *classify(sa, sb, features(sa, sb))[:1],
                     features(sa, sb), "solo", diff_spans(sa, sb), sa_img, sb_img)
        check("a lone candidate on each side is unambiguous",
              _disambiguate([solo])[0]["disambiguation"]["unambiguous"],
              str(solo["disambiguation"]))
    else:
        check("mutually indistinguishable bodies are all reported ambiguous", False,
              "synthetic trio did not map: " + "; ".join(s.reason for s in trio))
    check("the boundary statement names both prohibitions",
          "NEVER authoritative ABI evidence" in BOUNDARY_STATEMENT
          and "NEVER enter this project's symbol closure" in BOUNDARY_STATEMENT)
    # No code path copies a foreign ROM's entries into the project map, and no
    # VMA-shaped foreign label is ever formed from a foreign VMA.
    body = (ROOT / "tools" / f"{STRATEGY}.py").read_text()
    check("no code path merges a foreign ROM's entries into the project map",
          not re.search(r"project\.functions\.update\(\s*(?:other|o)\b", body)
          and "project.functions = dict(probe.rom_functions())" in body)
    check("no foreign VMA-shaped label is ever constructed",
          not re.search(r'"_0*8[0-9A-Fa-f]{6}"', body) and
          not re.search(r"f\"_[0-9]*\{", body))

    # 7. literal roles are classified, not guessed
    check("pointer literals are recognised", literal_role(0x030035C0) == "POINTER")
    check("IWRAM pointers are recognised", literal_role(0x03000F50) == "POINTER")
    check("byte constants are recognised", literal_role(0x2A) == "U8")
    check("word constants are recognised", literal_role(0x00012345) == "CONST")

    print(f"{passed}/{total} passed")
    return 0 if passed == total else 1


def _classify_with(feats: dict) -> str:
    """Classify from a feature vector alone -- the negative control for ranking."""
    strong = [k for k, v in feats.items() if v >= WEAK_FEATURE_FLOOR]
    return C_WEAK if len(strong) >= WEAK_MIN_FEATURES else C_NONE


def main() -> int:
    if "--self-test" in sys.argv:
        return self_test()
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=BOUNDARY_STATEMENT)
    ap.add_argument("--rom", action="append", default=[], metavar="NAME=PATH",
                    help='ROM to analyze, explicit name and path. The project ROM '
                         f'must be named "{PROJECT}". Repeatable. Nothing is downloaded.')
    ap.add_argument("--subsystem", default="keypad",
                    help=f"bounded subsystem to map: {', '.join(sorted(SUBSYSTEMS))}, "
                         f"or a name combined with --vma seeds")
    ap.add_argument("--vma", action="append", default=[], metavar="[NAME=]VMA",
                    help="seed entry point; NAME=vma seeds one specific ROM's own space. "
                         "Repeatable.")
    ap.add_argument("--vma-rom", default="", metavar="NAME",
                    help="apply bare --vma seeds to this ROM only")
    ap.add_argument("--max-span", type=lambda s: int(s, 0), default=0x400,
                    help="upper bound on a seed's span search (default 0x400)")
    ap.add_argument("--validate-c", type=Path, metavar="FILE",
                    help="scratch C body to score against the PROJECT ROM's own span "
                         "with the unchanged agbcc oracle")
    ap.add_argument("--run-id", default=None, help="evidence run id")
    ap.add_argument("--out", type=Path, help="private run directory")
    ap.add_argument("--json", type=Path, help="copy the full result here")
    return kit.run_tool(STRATEGY, run, ap=ap, argv=sys.argv[1:])


if __name__ == "__main__":
    raise SystemExit(main())
