#!/usr/bin/env python3
"""Shared foundation for the experimental matching strategies.

Each strategy needs the same five things before it can say anything
defensible, and building them separately is how they drift apart:

  * a **candidate adapter** that compiles scratch C89 with the pinned agbcc
    configuration and scores it against a real ROM span, reusing
    `corpus_match_probe`'s ELF/relocation/linking logic rather than
    re-deriving it;
  * a **contract record** stating what the target is (canonical ownership,
    caller ABI, allowed memory, return and live-in registers) and what the
    strategy cannot represent;
  * an **instruction signature** so two source spellings that emit the same
    instructions are not counted as two search results;
  * an **evidence writer** recording input hashes, tool identity, budgets and
    reproducible output paths;
  * a **bounded runner** with compile/time budgets, seeded comparisons and
    resume bookkeeping.

This module provides those and nothing else. It never edits `src/`, the
promotion manifest, or assembly, and it never reads reference bytes as
behavior: the ROM is the oracle, and only the ROM.

Two rules from the strategy document are load-bearing here:

  * *"Draft status separately from integrated probe status and independent-link
    acceptance. A summary must not collapse these into one success flag."*
    `Result` therefore has three separate status fields, and `accepted_bytes`
    is only ever set by the real link -- never here.
  * *"Missing optional tools must yield an explicit unsupported/dependency
    result; do not silently switch compiler or algorithm."*
    `DependencyMissing` and `Unsupported` are distinct statuses, not a
    generic failure.
"""
from __future__ import annotations

import hashlib
import json
import os
import re
import subprocess
import sys
import time
from dataclasses import dataclass, field, asdict
from datetime import datetime, timezone
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import corpus_match_probe as probe
import match_families as families
import agbcc_c89_transform as c89_transform

ROOT = probe.ROOT
ROM_BASE = probe.ROM_BASE

#: The path field of a `clang -E` line marker. Normalizing it is what makes
#: the translation-unit cache work at all: the marker records the *candidate's*
#: scratch path, which differs for every body, so keying on the raw
#: preprocessed text would mint a fresh address per body and the cache would
#: never hit. `corpus_match_probe` normalizes the same field for the same
#: reason (see its `_LINEMARKER_PATH`).
_LINEMARKER_PATH = re.compile(r'(?m)^(# \d+ ")[^"]*(")')

#: The pinned matching configuration. Changing this changes every strategy's
#: meaning, so it lives here once and every tool reports it in its evidence.
AGBCC_FLAGS = ("-O2", "-mthumb-interwork", "-ffunction-sections")

#: Fixed-width typedefs for scratch candidates. agbcc is a freestanding C89
#: compiler: it has no stdlib, so a body that writes `u32` must supply the
#: typedef itself. These widths are the project's, not a convenience set --
#: `s16` really is `short`, which on this target is 16 bits.
#: (name, declaration) pairs. The list form lets `with_preamble` supply only
#: what a candidate is actually missing.
#
#: These widths are the project's, not a convenience set -- `s16` really is
#: `short`, and on this target that is 16 bits.
_TYPEDEF_LINES = [
    ("u8", "typedef unsigned char u8;"),
    ("u16", "typedef unsigned short u16;"),
    ("u32", "typedef unsigned int u32;"),
    ("s8", "typedef signed char s8;"),
    ("s16", "typedef signed short s16;"),
    ("s32", "typedef signed int s32;"),
]
C89_TYPES = "\n".join(line for _, line in _TYPEDEF_LINES) + "\n"

TOOL_VERSION = 1
TOOL_ID = "experiment_kit"

EXPERIMENTS_ROOT = ROOT / "build/experiments"

# --------------------------------------------------------------------------
# Status vocabulary. Never collapse these: the whole point of the status set
# is that "no exact draft" and "we could not represent this contract" and
# "the solver gave up" are different facts with different follow-ups.
# --------------------------------------------------------------------------

EXACT_DRAFT = "EXACT_DRAFT"
NO_EXACT_DRAFT = "NO_EXACT_DRAFT"
UNSUPPORTED_CONTRACT = "UNSUPPORTED_CONTRACT"
DEPENDENCY_MISSING = "DEPENDENCY_MISSING"
SOLVER_TIMEOUT = "SOLVER_TIMEOUT"
BUDGET_EXHAUSTED = "BUDGET_EXHAUSTED"
TOOL_FAILURE = "TOOL_FAILURE"

#: Candidate-level status, deliberately NOT a terminal run status. One
#: candidate that would not compile says that candidate is invalid; it does
#: not make the run a failure, and it must not be counted as a tested
#: alternative either. Strategies use it to separate "we tried N shapes"
#: from "N shapes actually reached the oracle".
COMPILE_ERROR = "COMPILE_ERROR"

#: Every terminal outcome a strategy can reach. A run must always land in
#: exactly one of these; anything else is a bug in the tool, not a result.
TERMINAL_STATUSES = frozenset({
    EXACT_DRAFT, NO_EXACT_DRAFT, UNSUPPORTED_CONTRACT, DEPENDENCY_MISSING,
    SOLVER_TIMEOUT, BUDGET_EXHAUSTED, TOOL_FAILURE,
})

#: Statuses that mean "the strategy ran to completion and produced no exact
#: draft". That is a measured negative, not a failure to hide -- but
#: DEPENDENCY_MISSING and SOLVER_TIMEOUT are NOT in here, because neither
#: tested the hypothesis at all.
NEGATIVE_STATUSES = frozenset({
    NO_EXACT_DRAFT, UNSUPPORTED_CONTRACT, BUDGET_EXHAUSTED,
})


class Unsupported(Exception):
    """The contract cannot be represented by this strategy's model."""


class DependencyMissing(Exception):
    """An explicitly-configured optional external tool is not available."""


class BudgetExhausted(Exception):
    """A configured compile or time budget ran out."""


def digest_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def digest_path(path) -> str | None:
    """SHA-256 of a file, or None when it does not exist.

    A missing prerequisite hashes to None rather than to the hash of an empty
    file, so "the compiler is absent" can never be mistaken for "the compiler
    produced nothing".
    """
    try:
        return digest_bytes(Path(path).read_bytes())
    except OSError:
        return None


def git_revision() -> str | None:
    """Current git revision, or None outside a repository.

    Recorded in every run's provenance so a result can be traced back to the
    tree that produced it. Returns None rather than raising: a source tarball
    with no git metadata should still be able to run an experiment.
    """
    try:
        out = subprocess.run(["git", "-C", str(ROOT), "rev-parse", "HEAD"],
                             text=True, capture_output=True)
        return out.stdout.strip() or None
    except OSError:
        return None


def with_preamble(source: str) -> str:
    """Supply the fixed-width typedefs a self-contained candidate needs.

    Two failure modes, both hit during bring-up, and both of which look like
    a strategy bug when they are really a harness bug:

    * A synthesized body with no includes has no `u32`, so agbcc reports a
      syntax error at the function name. Adding the typedefs fixes it.
    * Real project C pulls in `gba/types.h`, which declares those same names.
      Adding them again is a redefinition error, and the candidate never
      compiles for a reason unrelated to the hypothesis under test.

    So the rule is: anything with an `#include` is left completely alone
    (its own closure supplies its types), and a self-contained body gets only
    the typedefs it has not already declared. Detection is on the typedef
    *declaration*, not on the bare word `typedef`, so an unrelated
    `typedef struct {…} rec;` still leaves `u32` missing.
    """
    if "#include" in source:
        return source
    declared = set(re.findall(r"\btypedef\b[^;{]*?\b(u8|u16|u32|s8|s16|s32)\b", source))
    missing = [line for name, line in _TYPEDEF_LINES if name not in declared]
    if not missing:
        return source
    return "\n".join(missing) + "\n\n" + source


# --------------------------------------------------------------------------
# Contract record
# --------------------------------------------------------------------------

@dataclass
class Contract:
    """What a target is, and what the model is allowed to assume about it.

    Every field is either ROM/caller evidence or an explicit operator choice.
    A strategy must not invent one: `unsupported` is the correct response to a
    target whose real contract falls outside the model, and the caller has to
    resolve the contract from ROM disassembly and caller ABI before asking.
    """

    name: str
    vma: int
    end: int
    #: Caller-selected, from ROM and caller evidence. Not inferred.
    return_type: str = "u32"
    #: "ordinary" | "volatile" -- hardware memory must not be reordered or
    #: dropped, so a strategy may not quietly treat it as ordinary.
    memory: str = "ordinary"
    #: Input registers actually live at entry.
    inputs: tuple[int, ...] = ()
    #: Human-readable justification, recorded verbatim into the evidence.
    abi_note: str = ""
    #: Set when the contract is known to be incomplete; forces UNSUPPORTED.
    unresolved: str | None = None

    def __post_init__(self):
        self.inputs = tuple(sorted(set(self.inputs)))

    @property
    def span(self) -> dict[int, int]:
        return {self.vma: self.end}

    @property
    def rom_bytes(self) -> int:
        return self.end - self.vma

    def file_offset(self) -> int:
        return self.vma - ROM_BASE

    def require_supported(self) -> None:
        """Fail closed when the contract itself is not trustworthy."""
        if self.unresolved:
            raise Unsupported(f"unresolved contract: {self.unresolved}")
        if self.rom_bytes <= 0:
            raise Unsupported(f"non-positive span at {self.vma:#x}")
        if self.return_type not in ("void", "u8", "s8", "u16", "s16", "u32", "s32"):
            raise Unsupported(f"unknown return type {self.return_type!r}")
        if self.memory not in ("ordinary", "volatile"):
            raise Unsupported(f"unknown memory class {self.memory!r}")
        if any(r > 3 for r in self.inputs):
            raise Unsupported(f"parameter register r{max(self.inputs)} exceeds the "
                              "r0-r3 argument contract; widen the ABI model first")

    def to_json(self) -> dict:
        data = asdict(self)
        data["inputs"] = list(self.inputs)
        data["vma"] = f"{self.vma:#010x}"
        data["end"] = f"{self.end:#010x}"
        data["file_offset"] = f"{self.file_offset():#x}"
        return data


def contract_for_span(name: str, vma: int, end: int, **kwargs) -> Contract:
    return Contract(name=name, vma=vma, end=end, **kwargs)


# --------------------------------------------------------------------------
# Bounded experiment runner
# --------------------------------------------------------------------------

@dataclass
class Budget:
    """Compile and wall-clock limits for one bounded experiment.

    `compiles` counts agbcc invocations that actually ran; cache hits do not
    consume it, because re-spending budget on a byte-identical recompile
    would make the cost measurement meaningless.
    """

    compiles: int = 64
    seconds: float = 900.0
    #: Reported, not enforced: strategies that cannot bound their own output
    #: honestly should set it and say so.
    note: str = ""

    def __post_init__(self):
        if self.compiles < 1:
            raise ValueError("budget.compiles must be >= 1")
        if self.seconds <= 0:
            raise ValueError("budget.seconds must be > 0")


class BudgetMeter:
    def __init__(self, budget: Budget):
        self.budget = budget
        self.started = time.monotonic()
        self.compiles = 0
        self.cache_hits = 0
        self.duplicate_outputs = 0

    @property
    def elapsed(self) -> float:
        return time.monotonic() - self.started

    def check(self) -> None:
        if self.compiles >= self.budget.compiles:
            raise BudgetExhausted(f"compile budget {self.budget.compiles} exhausted")
        if self.elapsed > self.budget.seconds:
            raise BudgetExhausted(f"time budget {self.budget.seconds:.0f}s exhausted")

    def spend(self) -> None:
        self.check()
        self.compiles += 1

    def remaining(self) -> float:
        return max(0.0, self.budget.seconds - self.elapsed)

    def to_json(self) -> dict:
        return {
            "compiles_allowed": self.budget.compiles,
            "compiles_used": self.compiles,
            "cache_hits": self.cache_hits,
            "duplicate_outputs": self.duplicate_outputs,
            "seconds_allowed": self.budget.seconds,
            "seconds_elapsed": round(self.elapsed, 3),
            "note": self.budget.note,
        }


# --------------------------------------------------------------------------
# Instruction signature
# --------------------------------------------------------------------------

def instruction_signature(blob: bytes, vma: int, work: Path | None = None) -> dict | None:
    """Normalized shape of emitted instructions, reusing the family index.

    Returns the same fingerprint structure `match_families` computes for ROM
    spans -- normalized register names, immediates collapsed to `#K`, branch
    topology, load/store widths kept distinct. Two source spellings that emit
    these identical tokens and edges are the same compiler output, and a search
    must not spend budget or novelty credit discovering that twice.

    The blob is laid out at its ROM offset before decoding, because
    `match_families.flow` reads `blob[pc - ROM_BASE]`. Handing it a bare
    emitted blob therefore only works when `vma == ROM_BASE`; for any real
    target address every lookup runs off the front of the buffer and the call
    raises `ValueError` rather than returning None. A strategy that treated
    that as "no signature" would silently record every candidate as distinct
    and defeat its own deduplication.

    Returns None when the blob cannot be decoded as bounded Thumb-1 control
    flow; the caller treats that as "not dedupable", never as "different".
    """
    if len(blob) < 4:
        return None
    # `match_families.flow` -- which `fingerprint` also calls -- indexes
    # `blob[pc - ROM_BASE]`, so the blob must be laid out at its ROM address;
    # 0-based addressing raises ValueError and silently yields no signature.
    offset = vma - ROM_BASE
    if offset < 0:
        return None
    image = bytearray(offset + len(blob))
    image[offset:] = blob
    image = bytes(image)
    with _temp_dir(work) as tmp:
        reachable = _reachable(image, vma, vma + len(blob))
        if not reachable:
            return None
        try:
            listing = families.disassemble(image, tmp, reachable)
        except (subprocess.CalledProcessError, OSError, ValueError):
            return None
        if not listing:
            return None
        signature = families.fingerprint(image, vma, vma + len(blob), listing)
        # An incomplete fingerprint is built from `?` tokens. Returning it
        # would let a strategy call two undecodable candidates identical, or
        # one of them novel, on evidence that was never decoded.
        if not signature.get("complete", False):
            return None
        return signature


def _reachable(image: bytes, start: int, end: int) -> dict:
    """Reachable {pc: halfword-size} map over [start, end) of a ROM_BASE-indexed image."""
    try:
        seen, _, _ = families.flow(image, start, end)
    except ValueError:
        return {}
    return {pc: item[1] for pc, item in seen.items()}


class _temp_dir:
    """Context manager for a scratch dir that may be caller-supplied."""

    def __init__(self, parent: Path | None):
        self.parent = parent
        self.path = None
        self._tmp = None

    def __enter__(self) -> Path:
        if self.parent is not None:
            self.parent.mkdir(parents=True, exist_ok=True)
            self.path = self.parent
        else:
            import tempfile
            self._tmp = tempfile.TemporaryDirectory(prefix="expkit-")
            self.path = Path(self._tmp.name)
        return self.path

    def __exit__(self, *exc):
        if self._tmp is not None:
            self._tmp.cleanup()
        return False


# --------------------------------------------------------------------------
# Candidate adapter
# --------------------------------------------------------------------------

@dataclass
class CompileResult:
    """What agbcc emitted for one scratch candidate."""

    ok: bool
    blob: bytes
    calls: list = field(default_factory=list)
    pools: list = field(default_factory=list)
    #: Name of the emitted `.text.*` section, needed to resolve relocations.
    section: str = ""
    error: str | None = None


class CandidateAdapter:
    """Compile scratch C89 and score it against a ROM span.

    This is the single oracle every strategy returns to. It reuses
    `corpus_match_probe`'s ELF parsing, relocation resolution and
    whole-span comparison, and adds what a *scratch* candidate needs that a
    translation unit does not: an explicit function/section name, caching keyed
    on every input that can change the answer, and per-candidate work
    directories that do not collide between strategies.

    Caching is content-addressed on the preprocessed text (so a header edit
    misses rather than serving a stale object), the compiler binary's hash,
    the exact flag list, the target VMA/span, the ROM hash and the resolved
    symbol table. An unresolved prerequisite hashes to None and forces a miss
    rather than a plausible-looking stale hit.
    """

    def __init__(self, work: Path, *, cache_dir: Path | None = None, meter: BudgetMeter | None = None):
        self.work = Path(work)
        self.work.mkdir(parents=True, exist_ok=True)
        self.cache_dir = Path(cache_dir) if cache_dir else self.work / "cache"
        self.cache_dir.mkdir(parents=True, exist_ok=True)
        self.meter = meter
        self.rom = probe.ROM.read_bytes()
        self.rom_sha256 = digest_bytes(self.rom)
        code_symbols = probe.load_code_symbols()
        self._resolver = probe.SymbolResolver(code_symbols, probe.alias_targets(),
                                               probe.scan_decl_hints())
        # Symbol *resolution* changes call encoding and pool words, so it is
        # part of the cache identity: re-resolving against a stale closure
        # would score a candidate against addresses the linker never used.
        self._symbol_key = digest_bytes(json.dumps(sorted(code_symbols.items())).encode())[:16]
        self._hits = 0
        self._misses = 0
        self._candidates = 0

    # -- prerequisites ---------------------------------------------------

    @staticmethod
    def toolchain_ready() -> tuple[bool, str | None]:
        if not probe.AGBCC.exists():
            return False, f"agbcc not built at {probe.AGBCC}"
        if not probe.ROM.exists():
            return False, f"no reference ROM at {probe.ROM}"
        return True, None

    def provenance(self) -> dict:
        return {
            "tool": TOOL_ID,
            "tool_version": TOOL_VERSION,
            "compiler": str(probe.AGBCC.relative_to(ROOT)),
            "compiler_sha256": digest_path(probe.AGBCC),
            "flags": list(AGBCC_FLAGS),
            "rom": str(probe.ROM.relative_to(ROOT)),
            "rom_sha256": self.rom_sha256,
            "symbols_sha256": self._symbol_key,
            "git_revision": git_revision(),
        }

    # -- compilation -----------------------------------------------------

    def _tu_key(self, preprocessed: str) -> str:
        """Content address for one translation unit's compiled object.

        Keyed on the **preprocessed** text, not the `.c` bytes. Every TU in
        this project `#include`s from `include/` and `asm/`, so keying on the
        `.c` would serve a pre-header-edit object after a header change -- and
        a stale object still scores cleanly, which is a confident wrong
        answer rather than a crash. `corpus_match_probe` documents the same
        trap for the same reason.

        Deliberately *not* keyed on the function name: a strategy sweeping
        several bodies of one TU compiles that TU once and links each body
        out of the single object, which is what makes a multi-body sweep
        affordable at all.

        Linemarkers are normalized first. `clang -E` emits line-marker
        headers carrying the *candidate's own* scratch path, which differs
        per body (c0000/, c0001/, ...). Keying on the raw text therefore
        gives every body of one TU a distinct address and the cache never
        hits -- measured during bring-up: 24 compiles for the 24 exact bodies
        of a single TU, where 1 was intended. Stripping the path leaves the
        content identity, which is the only thing the cache is for.
        `corpus_match_probe` normalizes the same field for the same reason.
        """
        normalized = _LINEMARKER_PATH.sub(r'\1<path>\2', preprocessed)
        parts = [
            normalized, str(self.rom_sha256),
            digest_path(probe.AGBCC) or "no-agbcc",
            "|".join(AGBCC_FLAGS), self._symbol_key, str(TOOL_VERSION),
        ]
        return hashlib.sha256("\x00".join(parts).encode()).hexdigest()[:32]

    def _preprocess(self, source: str, directory: Path, tag: str) -> tuple[Path | None, str | None]:
        """C89-transform (when needed) then `clang -E` one candidate body."""
        c_file = directory / f"{tag}.c"
        c_file.write_text(source, encoding="utf-8")

        # agbcc is a C89 compiler and the project's own sources are not: they
        # use declaration-after-statement, which agbcc rejects with a syntax
        # error at the *declaration*. Reuse the existing compiler-driven
        # transform rather than hand-rolling another one, so a candidate that
        # fails to compile fails for a reason the strategy actually caused.
        #
        # Only for sources carrying their own `#include`s: those are real
        # translation units in modern C, and this is exactly what the probe
        # does to them. A synthesized candidate is already C89 by
        # construction, and running the compiler-driven fixer over it would
        # fail closed on `unhandled` for input the strategy never malformed.
        compile_input = c_file
        if "#include" in source:
            transformed, stats = c89_transform.cached_transform_file(c_file)
            if stats.get("unhandled"):
                return None, f"C89 transform: {stats['unhandled']}"
            c89_file = directory / f"{tag}.c89.c"
            c89_file.write_text(transformed, encoding="utf-8")
            compile_input = c89_file

        i_file = directory / f"{tag}.i"
        with i_file.open("w", encoding="utf-8") as handle:
            preprocess = subprocess.run(
                ["clang", "-E", "-nostdinc", "-undef", *probe.inc_flags(), str(compile_input)],
                text=True, stdout=handle, stderr=subprocess.PIPE)
        if preprocess.returncode:
            return None, f"preprocess: {preprocess.stderr.strip()[:1500]}"
        return i_file, None

    def _object_for(self, i_file: Path, key: str, directory: Path, tag: str) -> tuple[Path | None, str | None]:
        """agbcc + as, memoized on the content address of the preprocessed text."""
        o_file = self.cache_dir / f"{key}.o"
        if o_file.exists():
            self._hits += 1
            if self.meter:
                self.meter.cache_hits += 1
            return o_file, None
        if self.meter is not None:
            self.meter.spend()
        s_file = directory / f"{tag}.s"
        built = subprocess.run([str(probe.AGBCC), *AGBCC_FLAGS, str(i_file), "-o", str(s_file)],
                               text=True, capture_output=True)
        if built.returncode:
            return None, f"agbcc: {(built.stderr or built.stdout).strip()[:1500]}"
        staged = subprocess.run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-o", str(o_file), str(s_file)],
                                text=True, capture_output=True)
        if staged.returncode:
            return None, f"as: {(staged.stderr or staged.stdout).strip()[:1500]}"
        (self.cache_dir / f"{key}.s").write_text(s_file.read_text(errors="replace"))
        self._misses += 1
        return o_file, None

    def compile(self, source: str, function: str, vma: int, *, index: int = 0) -> CompileResult:
        """Compile one scratch candidate and link its section at `vma`.

        A cache hit runs no compiler at all and does not spend compile
        budget: re-spending budget to rediscover an answer already in hand
        would make every cost measurement in the strategy document a fiction.
        """
        self._candidates += 1
        directory = self.work / f"c{index:04d}"
        directory.mkdir(parents=True, exist_ok=True)
        i_file, error = self._preprocess(with_preamble(source), directory, function)
        if i_file is None:
            return CompileResult(ok=False, blob=b"", error=error)
        key = self._tu_key(i_file.read_text(errors="replace"))
        o_file, error = self._object_for(i_file, key, directory, function)
        if o_file is None:
            return CompileResult(ok=False, blob=b"", error=error)

        sections = probe.section_map(o_file)
        key_name = f".text.{function}"
        if key_name not in sections:
            # Fail closed. Falling back to "some other .text section" would
            # score this body against a *different* function's bytes and
            # report a confident, meaningless number.
            available = ", ".join(sorted(sections)) or "none"
            return CompileResult(ok=False, blob=b"", error=(
                f"no section {key_name}; compiler emitted: {available}"))
        relocs = probe.relocations(o_file, key_name)
        blob, calls, pools = probe.link_function(sections[key_name], vma, relocs, key_name, self._resolver)
        return CompileResult(ok=True, blob=blob, calls=calls, pools=pools, section=key_name)

    # -- scoring ---------------------------------------------------------

    def score(self, compiled: CompileResult, contract: Contract) -> "Score":
        """Whole-span byte comparison against the target's ROM bytes."""
        if not compiled.ok:
            raise ValueError(compiled.error or "compile failed")
        previous = probe.WORK
        probe.WORK = self.work / "score"
        try:
            record = probe.compare_function(self.rom, contract.vma, contract.name, compiled.blob,
                                            contract.span, compiled.calls, compiled.pools)
        finally:
            probe.WORK = previous
        if record is None:
            return Score(status=NO_EXACT_DRAFT, detail="span smaller than probe minimum",
                         blob=compiled.blob)
        # Carried so callers that need a signature need not compile again.
        return Score.from_record(record, compiled)

    def evaluate(self, source: str, function: str, contract: Contract, *, index: int = 0) -> "Score":
        """Compile and score one candidate body.

        A compile failure is reported as `COMPILE_ERROR`, never as
        `NO_EXACT_DRAFT`. The distinction is the difference between "this
        source shape is not the answer" and "the harness could not build the
        question"; folding them together lets a broken pipeline manufacture
        negative evidence against a strategy that was never really tested.
        """
        compiled = self.compile(source, function, contract.vma, index=index)
        if not compiled.ok:
            return Score(status=COMPILE_ERROR, detail=compiled.error)
        return self.score(compiled, contract)

    def cache_stats(self) -> dict:
        return {"hits": self._hits, "misses": self._misses, "candidates": self._candidates}


#: `corpus_match_probe` status -> this module's terminal status. The probe's
#: `EXACT` means "these scratch bytes equal the ROM span", which in strategy
#: terms is an EXACT *draft*: integration and the independent link are still
#: required, and a summary must never imply otherwise.
PROBE_STATUS_MAP = {
    "EXACT": EXACT_DRAFT,
    "PARTIAL": NO_EXACT_DRAFT,
    "OVERSIZED": NO_EXACT_DRAFT,
    "NO_OVERLAP": NO_EXACT_DRAFT,
    "UNRESOLVED_RELOCATION": NO_EXACT_DRAFT,
}

@dataclass
class Score:
    """One candidate's measured result.

    `status` is the *draft* status only. `integrated_status` and
    `link_accepted` stay unset here because this module never integrates and
    never links; a strategy that reports them is overstepping.
    """

    status: str
    detail: str | None = None
    matched_bytes: int = 0
    rom_bytes: int = 0
    prefix: int | None = None
    first_diff: int | None = None
    cls: str | None = None
    calls: list = field(default_factory=list)
    pools: list = field(default_factory=list)
    signature: str | None = None
    #: The linked object's emitted bytes for this candidate. Carried here so a
    #: caller needing a signature does not compile a SECOND time: `evaluate`
    #: already produced these bytes, and novelty's mutant loop was paying for a
    #: duplicate `compile()` purely to recover them. Also lets the SEED's own
    #: signature be computed, which nothing did, so the baseline was missing
    #: from the novelty archive entirely.
    blob: bytes | None = None
    source: str | None = None
    #: Populated only by an integrator, never by a strategy tool.
    integrated_status: str | None = None
    link_accepted: bool = False
    accepted_bytes: int = 0

    @property
    def exact(self) -> bool:
        return self.status == EXACT_DRAFT

    @classmethod
    def from_record(cls, record: dict, compiled: CompileResult | None = None) -> "Score":
        """Translate one `corpus_match_probe` comparison record into a Score.

        The probe's status vocabulary is its own (`EXACT`, `PARTIAL`,
        `OVERSIZED`, `NO_OVERLAP`, `UNRESOLVED_RELOCATION`); the strategy
        vocabulary is this module's. Mapping happens here, once, and an
        unrecognized probe status is refused rather than passed through: a
        status string that reached `summary.md` untranslated would not be in
        `TERMINAL_STATUSES`, so the run could not be audited.
        """
        raw = record.get("status")
        mapped = PROBE_STATUS_MAP.get(raw)
        if mapped is None:
            raise ValueError(f"unrecognized probe status {raw!r}")
        return cls(
            status=mapped,
            matched_bytes=record.get("matched_bytes", 0),
            rom_bytes=record.get("rom_bytes", 0),
            prefix=record.get("prefix"),
            first_diff=record.get("first_diff"),
            cls=record.get("class"),
            calls=record.get("call_targets", []),
            pools=record.get("pool_words", []),
            blob=compiled.blob if compiled is not None else None,
        )

    def to_json(self) -> dict:
        data = asdict(self)
        # A whole candidate body in every JSONL row makes the evidence file
        # unreadable and inflates run directories; the .c file on disk is the
        # record of the source.
        data["source"] = None
        # Emitted bytes are not JSON-serialisable at all, so `asdict` carrying
        # them through would raise inside `json.dumps` and lose the row. The
        # object file under the run's probe directory is the record.
        data["blob"] = None
        return data


def summarize_status(scores: list[Score], *, exhausted: bool = False) -> str:
    """Fold per-candidate outcomes into one terminal run status.

    The fold is deliberately conservative. Candidates that never reached the
    oracle (compile errors) are excluded, because a run where every candidate
    failed to build has not measured anything, and calling it
    `NO_EXACT_DRAFT` would report a negative result for a strategy that was
    never executed. That case is `TOOL_FAILURE`, and it is the case that
    should send someone to fix the harness.
    """
    if any(s.exact for s in scores):
        return EXACT_DRAFT
    tested = [s for s in scores if s.status != COMPILE_ERROR]
    if not tested:
        return TOOL_FAILURE if scores else UNSUPPORTED_CONTRACT
    if exhausted:
        return BUDGET_EXHAUSTED
    return NO_EXACT_DRAFT


# --------------------------------------------------------------------------
# Evidence writer
# --------------------------------------------------------------------------

class EvidenceWriter:
    """One run directory with the layout the strategy document specifies.

        build/experiments/<strategy>/<run-id>/
          run.json  targets.json  contracts/  context/
          candidates/  probes/  diagnostics/  results.jsonl  summary.md

    `open()` clears stale winner markers before anything runs, so a failed
    rerun can never leave an earlier exact draft sitting in the new run's
    directory looking like its own result.
    """

    def __init__(self, strategy: str, run_id: str | None = None, *, root: Path | None = None,
                 extra: dict | None = None):
        self.strategy = strategy
        self.run_id = run_id or datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        base = Path(root) if root else EXPERIMENTS_ROOT / strategy
        self.dir = base / self.run_id
        self.extra = extra or {}
        self.results: list[dict] = []
        self._run: dict = {}

    def open(self) -> "EvidenceWriter":
        if self.dir.exists():
            for name in ("winner.c", "winner.json"):
                stale = self.dir / name
                if stale.exists():
                    stale.unlink()
        for sub in ("contracts", "context", "candidates", "probes", "diagnostics"):
            (self.dir / sub).mkdir(parents=True, exist_ok=True)
        (self.dir / "results.jsonl").write_text("")
        self._run = {
            "strategy": self.strategy,
            "run_id": self.run_id,
            "started_utc": datetime.now(timezone.utc).isoformat(timespec="seconds"),
            "tool": TOOL_ID,
            "tool_version": TOOL_VERSION,
            "git_revision": git_revision(),
            **self.extra,
        }
        (self.dir / "run.json").write_text(json.dumps(self._run, indent=2, sort_keys=True) + "\n")
        return self

    def write_json(self, name: str, payload) -> Path:
        path = self.dir / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
        return path

    def save_candidate(self, name: str, text: str) -> Path:
        path = self.dir / "candidates" / f"{name}.c"
        path.write_text(text)
        return path

    def save_contract(self, contract: Contract) -> Path:
        path = self.dir / "contracts" / f"{contract.name}.json"
        path.write_text(json.dumps(contract.to_json(), indent=2, sort_keys=True) + "\n")
        return path

    def record(self, payload: dict) -> None:
        self.results.append(payload)
        with (self.dir / "results.jsonl").open("a") as handle:
            handle.write(json.dumps(payload, sort_keys=True) + "\n")

    def declare_winner(self, name: str, text: str, payload: dict) -> Path:
        """Record an exact draft. Only call this on a measured EXACT score."""
        (self.dir / "winner.c").write_text(text)
        (self.dir / "winner.json").write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
        return self.dir / "winner.c"

    def finish(self, summary: dict) -> dict:
        """Write `summary.md` and finalize `run.json`.

        The summary states the draft status, the integrated status
        (`not run`) and the independent-link acceptance (`not run`) as three
        separate lines. Nothing here can promote a byte.
        """
        payload = dict(summary)
        payload.setdefault("draft_status", UNSUPPORTED_CONTRACT)
        payload.setdefault("integrated_status", "not run")
        payload.setdefault("link_accepted", "not run")
        payload["accepted_c_owned_bytes"] = 0
        payload["finished_utc"] = datetime.now(timezone.utc).isoformat(timespec="seconds")
        # `run.json` is the machine-readable record, so it carries every
        # SCALAR the run produced, using the same filter as the summary
        # renderer below. It previously copied only `draft_status` and
        # `integrated_status`, so the counters a strategy exists to produce --
        # `candidates`, `distinct_signatures`, `compiles_attempted` -- reached
        # `summary.md` only and `run.json` was a five-key stub. That is why a
        # run reporting "225244 candidates, 0 compiles" read like a slow
        # success until its summary was opened by hand.
        self._run.update({k: v for k, v in payload.items()
                          if not isinstance(v, (dict, list))})
        (self.dir / "run.json").write_text(json.dumps(self._run, indent=2, sort_keys=True) + "\n")

        rows = payload.get("results") or []
        lines = [
            f"# {self.strategy} — {self.run_id}",
            "",
            f"- draft status: `{payload['draft_status']}`",
            f"- integrated probe status: `{payload['integrated_status']}`",
            f"- independent-link acceptance: `{payload['link_accepted']}`",
            f"- accepted C-owned bytes: `0` (an experiment tool cannot promote)",
            "",
        ]
        for key, value in sorted(payload.items()):
            if key in ("results", "notes") or key in lines:
                continue
            if isinstance(value, (dict, list)):
                continue
            lines.append(f"- {key.replace('_', ' ')}: `{value}`")
        for note in payload.get("notes", []) or []:
            lines.append(f"- note: {note}")
        lines += ["", "## Candidates", ""]
        for row in rows:
            lines.append(
                f"- `{row.get('name', '?')}` — {row.get('status')} "
                f"({row.get('matched_bytes', 0)}/{row.get('rom_bytes', 0)} bytes)"
                + (f" — {row['detail']}" if row.get("detail") else "")
            )
        if not rows:
            lines.append("- none")
        (self.dir / "summary.md").write_text("\n".join(lines) + "\n")
        return payload

    def path(self, *parts: str) -> Path:
        return self.dir.joinpath(*parts)

    def relative(self, path: Path) -> str:
        try:
            return str(Path(path).relative_to(ROOT))
        except ValueError:
            return str(path)


# --------------------------------------------------------------------------
# Strategy scaffolding
def run_writer(strategy: str, out, *, extra: dict | None = None) -> EvidenceWriter:
    """An EvidenceWriter whose run directory IS `out`.

    `tool_header` documents `--out` as the "private run directory", so the
    named path must be the one that gets filled. Constructing
    `EvidenceWriter(strategy, root=out.parent)` does NOT do that: the writer
    is `<root>/<run_id>/`, and with no explicit run id it auto-generates a
    UTC timestamp. Measured: `--out build/experiments/recipe-pilot` wrote
    every file to `build/experiments/20261001T053354Z/` and left the named
    directory empty, so a caller checking its own `--out` for evidence sees
    nothing and no error either.

    `run_id="."` makes `base / "."` collapse to `base`, so the run directory is
    exactly `out`. Reruns overwrite, which is the documented behaviour:
    `open()` clears stale winner markers so a failed rerun cannot leave an
    earlier exact draft sitting in the directory as if it were its own result.
    """
    return EvidenceWriter(strategy, ".", root=Path(out), extra=extra)


def out_writer(strategy: str, out, *, extra: dict | None = None) -> EvidenceWriter:
    """`run_writer` when `--out` was given, else a default timestamped run dir.

    The one rule every strategy tool now follows: an explicit `--out` is
    honoured exactly, and the no-argument case keeps the automatic per-run
    timestamp under `build/experiments/<strategy>/`. Passing `out=None` gives
    the default; passing a path gives exactly that path.
    """
    if out is None:
        return EvidenceWriter(strategy, extra=extra)
    return run_writer(strategy, out, extra=extra)



# --------------------------------------------------------------------------
# Strategy scaffolding
# --------------------------------------------------------------------------

def tool_header(strategy: str, one_line: str, doc: str) -> None:
    """Standard CLI preamble shared by every strategy tool.

    Guarantees each tool takes `--out` (private run directory) and `--seed`,
    and that an uncaught strategy-level exception becomes an explicit status
    record rather than a traceback and a zero exit code.
    """
    ap = argparse.ArgumentParser(description=doc, formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog=one_line)
    ap.add_argument("--out", type=Path, help="private run directory (default build/experiments/<strategy>/<run>)")
    ap.add_argument("--seed", type=int, default=0, help="RNG seed; recorded in the evidence")
    ap.add_argument("--json", type=Path, help="copy the run summary here")
    return ap


def run_tool(strategy: str, body, *, ap, argv=None) -> int:
    """Dispatch a strategy body, mapping exceptions to explicit statuses.

    Exit codes: 0 the strategy ran and recorded a result (whatever that
    result is), 2 the strategy could not run at all. A negative hypothesis
    that was actually measured is exit 0 -- the measurement is the deliverable.
    """
    args = ap.parse_args(argv)
    try:
        return body(args)
    except DependencyMissing as exc:
        print(f"{strategy}: DEPENDENCY_MISSING: {exc}", file=sys.stderr)
        return 2
    except Unsupported as exc:
        print(f"{strategy}: UNSUPPORTED_CONTRACT: {exc}", file=sys.stderr)
        return 2
    except BudgetExhausted as exc:
        print(f"{strategy}: BUDGET_EXHAUSTED: {exc}", file=sys.stderr)
        return 0
    except (OSError, ValueError) as exc:
        print(f"{strategy}: TOOL_FAILURE: {exc}", file=sys.stderr)
        return 2


def require_toolchain() -> None:
    ok, reason = CandidateAdapter.toolchain_ready()
    if not ok:
        raise DependencyMissing(reason)


def optional_tool(argv: list[str], *, name: str, hint: str) -> str:
    """Locate an explicitly configured optional external executable.

    Per the strategy document a missing optional tool is an explicit
    unsupported/dependency result. It must never silently change the algorithm
    or the compiler, so this raises rather than degrading.
    """
    import shutil
    found = shutil.which(argv[0]) if len(argv) == 1 else (argv[0] if Path(argv[0]).exists() else None)
    if not found:
        raise DependencyMissing(f"{name} not available ({hint}); configure its path "
                                f"explicitly rather than falling back to another algorithm")
    return found


def self_test() -> int:
    """Regressions for the kit's own failure modes. No ROM or toolchain needed."""
    print("running experiment_kit self-test...")
    passed = total = 0

    def check(label: str, condition: bool) -> None:
        nonlocal passed, total
        total += 1
        if condition:
            passed += 1
            print(f"  [PASS] {label}")
        else:
            print(f"  [FAIL] {label}", file=sys.stderr)

    # Status vocabulary: the negative statuses must not include the ones that
    # mean "we could not tell", or a summary would read as a measured miss.
    check("exact draft is not a negative status", EXACT_DRAFT not in NEGATIVE_STATUSES)
    check("unsupported contract is a negative status", UNSUPPORTED_CONTRACT in NEGATIVE_STATUSES)
    check("dependency missing is not a negative status", DEPENDENCY_MISSING not in NEGATIVE_STATUSES)
    check("solver timeout is not a negative status", SOLVER_TIMEOUT not in NEGATIVE_STATUSES)
    check("all statuses are terminal", set(TERMINAL_STATUSES) >= {EXACT_DRAFT, NO_EXACT_DRAFT,
                                                                   UNSUPPORTED_CONTRACT,
                                                                   DEPENDENCY_MISSING, SOLVER_TIMEOUT,
                                                                   BUDGET_EXHAUSTED, TOOL_FAILURE})

    # A compile failure must not read as a measured miss, and the run-level
    # fold must refuse to claim a negative result from candidates that never
    # reached the oracle.
    check("compile error is not a terminal run status", COMPILE_ERROR not in TERMINAL_STATUSES)
    check("compile error is not a negative result", COMPILE_ERROR not in NEGATIVE_STATUSES)
    broken = [Score(status=COMPILE_ERROR), Score(status=COMPILE_ERROR)]
    check("all-compile-error folds to tool failure", summarize_status(broken) == TOOL_FAILURE)
    check("no candidates at all is unsupported", summarize_status([]) == UNSUPPORTED_CONTRACT)
    tested = [Score(status=COMPILE_ERROR), Score(status=NO_EXACT_DRAFT, matched_bytes=3)]
    check("a tested miss beside a compile error is a real negative",
          summarize_status(tested) == NO_EXACT_DRAFT)
    check("an exact draft wins the fold",
          summarize_status(tested + [Score(status=EXACT_DRAFT)]) == EXACT_DRAFT)
    check("budget exhaustion is reported when it cut the search short",
          summarize_status([Score(status=NO_EXACT_DRAFT)], exhausted=True) == BUDGET_EXHAUSTED)

    # The preamble rule: synthesized bodies need the typedefs, real
    # translation units bring their own and must not be redefined.
    check("synthesized body gains typedefs",
          "typedef unsigned int u32;" in with_preamble("u32 f(void){return 0;}"))
    check("source with includes is left untouched",
          with_preamble('#include <gba/types.h>\nu32 f(void){return 0;}')
          == '#include <gba/types.h>\nu32 f(void){return 0;}')
    check("an unrelated typedef does not suppress the preamble",
          "typedef unsigned int u32;" in with_preamble("typedef int rec;\nu32 f(void){return 0;}"))
    check("a self-declared type is not duplicated",
          with_preamble("typedef unsigned int u32;\nu32 f(void){return 0;}").count("u32;") == 1)
    check("preamble is idempotent",
          with_preamble(with_preamble("u32 f(void){return 0;}"))
          == with_preamble("u32 f(void){return 0;}"))

    # Contract fail-closed behavior: a target whose real contract the model
    # cannot honor must raise rather than be approximated.
    base = dict(name="t", vma=0x08000100, end=0x08000110)
    Contract(**base).require_supported()  # a well-formed contract must not raise
    check("unresolved contract refuses", raises(Unsupported, lambda: Contract(
        **base, unresolved="caller ABI unknown").require_supported()))
    check("r4 input refuses", raises(Unsupported, lambda: Contract(
        **base, inputs=(4,)).require_supported()))
    check("empty span refuses", raises(Unsupported, Contract(
        name="t", vma=0x08000100, end=0x08000100).require_supported))
    check("bad memory class refuses", raises(Unsupported, lambda: Contract(
        **base, memory="speculative").require_supported()))
    check("span math", (Contract(**base).rom_bytes, Contract(**base).file_offset()) == (0x10, 0x100))
    check("inputs normalize", Contract(**base, inputs=(2, 0, 2)).inputs == (0, 2))

    # Budget accounting: a cache hit must not spend compile budget, and the
    # limit must actually stop the search instead of being advisory.
    meter = BudgetMeter(Budget(compiles=2, seconds=1000))
    meter.spend()
    meter.cache_hits += 1
    check("cache hits do not spend budget", meter.compiles == 1 and meter.remaining() > 0)
    meter.spend()
    check("budget exhaustion raises", raises(BudgetExhausted, meter.check))
    # Deterministic: age the meter rather than relying on a 1 ms race.
    slow = BudgetMeter(Budget(compiles=9, seconds=1.0))
    slow.started -= 3600.0
    check("time budget raises", raises(BudgetExhausted, slow.check))
    check("a non-positive time budget is refused", raises(ValueError, lambda: Budget(seconds=0)))

    # Evidence writer: a rerun must clear a stale winner, and the summary must
    # report three separate statuses with zero accepted bytes.
    with _temp_dir(None) as tmp:
        writer = EvidenceWriter("selftest", "run1", root=tmp).open()
        writer.save_contract(Contract(**base))
        writer.record({"name": "c0", "status": EXACT_DRAFT, "matched_bytes": 16, "rom_bytes": 16})
        writer.declare_winner("c0", "u32 c0(void){return 0;}", {"status": EXACT_DRAFT})
        check("winner written", writer.path("winner.c").exists())
        again = EvidenceWriter("selftest", "run1", root=tmp).open()
        check("stale winner cleared on rerun", not again.path("winner.c").exists())
        check("stale results cleared on rerun", again.path("results.jsonl").read_text() == "")
        again.record({"name": "c1", "status": NO_EXACT_DRAFT, "matched_bytes": 4, "rom_bytes": 16})
        again.record({"name": "c2", "status": NO_EXACT_DRAFT, "matched_bytes": 0, "rom_bytes": 16})
        check("results.jsonl appends one json object per line",
              [json.loads(line)["name"] for line in
               again.path("results.jsonl").read_text().splitlines()] == ["c1", "c2"])
        payload = again.finish({"draft_status": NO_EXACT_DRAFT, "results": [
            {"name": "c1", "status": NO_EXACT_DRAFT, "matched_bytes": 4, "rom_bytes": 16}]})
        summary = again.path("summary.md").read_text()
        check("summary separates statuses",
              "draft status: `NO_EXACT_DRAFT`" in summary
              and "integrated probe status: `not run`" in summary
              and "independent-link acceptance: `not run`" in summary)
        check("summary claims no accepted bytes",
              payload["accepted_c_owned_bytes"] == 0 and "accepted C-owned bytes: `0`" in summary)

    # Digest must distinguish a missing prerequisite from an empty file.
    with _temp_dir(None) as tmp:
        empty = Path(tmp) / "empty"
        empty.write_bytes(b"")
        check("missing file hashes to None", digest_path(Path(tmp) / "nope") is None)
        check("empty file has a real digest", digest_path(empty) == digest_bytes(b""))

    # Score mapping: an EXACT probe status becomes EXACT_DRAFT, and a score
    # never claims integration.
    score = Score.from_record({"status": "EXACT", "matched_bytes": 8, "rom_bytes": 8})
    check("probe EXACT maps to EXACT_DRAFT", score.status == EXACT_DRAFT and score.exact)
    check("score never self-integrates", score.integrated_status is None and score.link_accepted is False)

    # The pinned flag set is the one the workflow uses. A strategy that drifts
    # it would silently change what every score in this project means.
    check("pinned flags", list(AGBCC_FLAGS) == ["-O2", "-mthumb-interwork", "-ffunction-sections"])

    # --- instruction signature -------------------------------------------
    # Regression: `match_families.flow` reads `blob[pc - ROM_BASE]`, so a bare
    # emitted blob only decoded at ROM_BASE. Every real target raised
    # ValueError instead of returning a signature, which would have made a
    # strategy record every candidate as distinct and defeat its own dedup.
    probe_blob = b"\x01\x20\x00\x21\x70\x47"          # movs r0,#1; movs r1,#1; bx lr
    low = instruction_signature(probe_blob, 0x08001234)
    high = instruction_signature(probe_blob, 0x08020000)
    check("a blob at a real ROM address yields a signature", low is not None)
    check("the same blob at another address gives the same signature",
          low is not None and high is not None
          and low["fingerprint"] == high["fingerprint"])
    check("an address past the code slice is not dedupable",
          instruction_signature(probe_blob, 0x0803F000) is None)
    check("a too-small blob is not dedupable",
          instruction_signature(b"\x00\x47", 0x08001234) is None)
    check("an undecodable blob is not dedupable",
          instruction_signature(b"\xff\xff\xff\xff\xff\xff", 0x08001234) is None)

    print(f"{passed}/{total} passed")
    return 0 if passed == total else 1


def raises(exc: type[BaseException], fn) -> bool:
    """Did calling `fn` raise `exc` (or a subclass)?

    The self-test uses this to assert that fail-closed paths actually fail.
    Passing the call object to `isinstance` instead would be a tautology, and
    a path that stopped raising would pass silently -- the exact class of bug
    this suite exists to catch.
    """
    try:
        fn()
    except exc:
        return True
    except BaseException:
        return False
    return False


def main() -> int:
    if "--self-test" in sys.argv:
        return self_test()
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--self-test", action="store_true")
    ap.parse_args()
    print(__doc__)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
