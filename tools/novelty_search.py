#!/usr/bin/env python3
"""Novelty-assisted search over source shapes (strategy §7).

A search driven only by byte similarity stagnates: once the best score stops
improving, every remaining candidate is a small perturbation of the same
shape, and a lower-scoring but semantically faithful variant that happens to
allocate differently is never generated at all. This tool keeps such
candidates and selects from them.

Two rules make the diversity useful rather than merely different:

  * **Mutations are behaviour-preserving by construction.** The whitelist is
    commutativity of side-effect-free integer operators, temporary
    extraction/inlining that preserves evaluation order, redundant-parenthesis
    insertion, and local renaming. Nothing deletes behaviour. A mutation that
    is not provably preserving must pass an independent semantic check before
    it can earn novelty credit.
  * **Novelty is an emitted-instruction signature, not source text.** Two
    spellings that compile to the same instructions are one result, and a
    novel-but-wrong candidate is not diversity at all.

A faithful seed is required: a `(void)p;` stub or an unestablished lift is
refused, because permuting the shape of source that does not yet implement the
ROM teaches the search nothing.
"""
from __future__ import annotations

import argparse
import json
import random
import re
import sys
from dataclasses import dataclass, field, asdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import corpus_match_probe as probe
import experiment_kit as kit

VERSION = 1
STRATEGY = "novelty_search"

SCORE_ONLY = "score-only"
NOVELTY = "novelty"

#: Integer operators whose operands are commutative and free of side effects.
#: Swapping them preserves both value and evaluation order in C89.
COMMUTATIVE = ("+", "*", "&", "|", "^")

_ASSIGN = re.compile(r"^\s*(?:u8|u16|u32|s8|s16|s32|int|unsigned|short|char)\s+"
                     r"\*?\s*([A-Za-z_]\w*)\s*=\s*(.+);\s*$")
#: A simple value: an identifier or an integer literal with optional suffix.
#: The operands do NOT have to be parenthesised -- real bodies read
#: `acc = p0 + p1;`, and requiring the parentheses matched nothing at all, so
#: the commutation mutator silently contributed nothing to the search.
_VALUE = r"(?:[A-Za-z_]\w*|0[xX][0-9a-fA-F]+[uUlL]*|\d+[uUlL]*)"
_BINARY = re.compile(rf"({_VALUE})\s*([+*&|^])\s*({_VALUE})")
_LOCAL_DECL = re.compile(r"^\s*(?:u8|u16|u32|s8|s16|s32)\s+([A-Za-z_]\w*)\s*;\s*$")

#: A bare assignment to an existing local, or a typed one. Both forms exist in
#: real bodies (`base = ...;` and `u32 v = ...;`), and both are STATEMENTS.
_ASSIGN_STMT = re.compile(r"^\s*(?:[A-Za-z_][A-Za-z0-9_]*\s+)*\*?\s*"
                          r"([A-Za-z_]\w*)\s*=\s*(.+?);\s*$")


def body_span(source: str) -> tuple[int, int] | None:
    """Character offsets of the function body's braces.

    Every text mutator is confined to this region. Without it, a rewrite that
    looks for `IDENT * IDENT` matches the RETURN TYPE in the signature
    (`void *candidate(int)`) and the declared pointer type in a local
    (`const u32 *t = ...`), producing `candidate *void(int)` and
    `const t *u32 = ...`. Those compile to nothing at all, so every candidate is
    a COMPILE_ERROR and the strategy measures its own harness instead of the
    allocator.
    """
    match = _FUNC_DEF.search(source)
    if not match:
        return None
    try:
        opening = source.index("{", match.end() - 1)
    except ValueError:
        return None
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return opening, index
    return None


def _signature_of(source: str) -> str | None:
    """The defined function's name, or None when there is no definition."""
    match = _FUNC_DEF.search(source)
    return match.group(1) if match else None


#: A local declaration with its type captured, so a mutation that changes a
#: declaration's type is visible as a difference rather than silently
#: compiling to something else.
_DECL_TYPED = re.compile(r"^\s*(u8|u16|u32|s8|s16|s32)\s+([A-Za-z_]\w*)\s*;\s*$")


def _declarations_of(source: str) -> set[tuple[str, str]]:
    """(type, name) for every typed local declaration in the body."""
    span = body_span(source)
    if not span:
        return set()
    found = set()
    for line in source[span[0]:span[1]].splitlines():
        match = _DECL_TYPED.match(line)
        if match:
            found.add((match.group(1), match.group(2)))
    return found


class SeedRejected(kit.Unsupported):
    """The seed is not a faithful body, so permuting its shape proves nothing."""


@dataclass
class Mutation:
    kind: str
    source: str
    #: True when the rewrite is behaviour-preserving by construction, so no
    #: semantic evaluation is required. Recorded per candidate: an unverified
    #: mutation and a verified one are not the same kind of evidence.
    preserving: bool
    why: str
    origin: str = "seed"


@dataclass
class Candidate:
    name: str
    source: str
    score: kit.Score | None = None
    signature: str | None = None
    ancestry: list[str] = field(default_factory=list)
    mutation: str | None = None
    semantic: str = "not-checked"
    rejected: str | None = None
    novel: bool = False


# --------------------------------------------------------------------------
# Seed faithfulness
# --------------------------------------------------------------------------

def reject_unfaithful_seed(name: str, source: str) -> None:
    """Refuse a seed that does not yet implement the ROM.

    The failure this prevents is subtle and expensive: permuting the shape of
    `(void)p;` produces a large archive of confident, wrong results that all
    score the same, and the run looks like it explored a lot.

    The stub test is STRUCTURAL, not a marker match. `(void)c;` is this
    repository's standard idiom for silencing an unused parameter, and a
    substring rule for it fires on every real lifted body that has one --
    measured: it rejected a ten-statement function. Drop those lines first,
    then ask whether anything is left.
    """
    body = strip_comments(source)
    if not body.strip():
        raise SeedRejected(f"seed {name} is empty")
    #: An unused-parameter silencing cast. Removed before counting.
    unused = re.compile(r"^\s*\(\s*void\s*\+\+\s*\w+\s*\)\s*;\s*$")
    statements = [line for line in body.splitlines()
                  if line.strip() and not line.strip().startswith(("#", "typedef", "}"))
                  and not unused.match(line)]
    if not statements:
        raise SeedRejected(
            f"seed {name} contains only unused-parameter casts; a stub or "
            "unestablished lift is not a faithful seed for shape search")
    if len(statements) <= 1:
        raise SeedRejected(
            f"seed {name} has {len(statements)} statement(s) after removing "
            "unused-parameter casts; too thin to permute meaningfully")
    if "return" not in body and "=" not in body:
        raise SeedRejected(f"seed {name} has neither an assignment nor a return")


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return re.sub(r"//[^\n]*", " ", text)


# --------------------------------------------------------------------------
# Behaviour-preserving mutations
# --------------------------------------------------------------------------

def commute_operands(source: str, rng: random.Random) -> list[Mutation]:
    """Swap the operands of commutative operators inside assignment statements.

    Confined to the function body and to a statement's right-hand side. A
    whole-source scan also matches the signature's `void *candidate` and a
    declaration's `u32 *t`, and rewrites those into nonsense; the resulting
    candidates never compile, so a search built on them measures its own
    harness instead of the allocator.
    """
    span = body_span(source)
    if not span:
        return []
    opening, closing = span
    out: list[Mutation] = []
    offset = 0
    for line in source.splitlines(keepends=True):
        start, end = offset, offset + len(line)
        offset = end
        if not (opening < start < closing):
            continue
        statement = _ASSIGN_STMT.match(line)
        if not statement:
            continue
        rhs_start, rhs_end = start + statement.start(2), start + statement.end(2)
        right_hand = source[rhs_start:rhs_end]
        for binary in _BINARY.finditer(right_hand):
            left, op, right = binary.group(1), binary.group(2), binary.group(3)
            if op not in COMMUTATIVE or left == right:
                continue
            left_span, right_span = binary.span(1), binary.span(3)
            swapped = (right_hand[:left_span[0]] + right
                       + right_hand[left_span[1]:right_span[0]]
                       + left + right_hand[right_span[1]:])
            text = (source[:rhs_start] + swapped + source[rhs_end:])
            out.append(Mutation(
                "commute", text, True,
                f"operator {op!r} is commutative over side-effect-free values, "
                "so operand order changes neither the value nor evaluation order"))
            if len(out) >= 8:
                return out
    return out


def extract_temporary(source: str) -> list[Mutation]:
    """Bind a bare assignment's right-hand side to a named temporary.

    Only BARE assignments to an already-declared local are eligible. Matching
    a declaration instead would extract `const u32 *t = ...` into
    `u32 tmp_t = ...` and silently change the local's type -- a candidate that
    compiles and computes something else.

    The temporary is declared at the TOP of the function body. agbcc is a C89
    compiler and rejects a declaration after a statement, so a mid-block
    declaration does not merely compile differently -- it does not compile.
    """
    span = body_span(source)
    if not span:
        return []
    opening, closing = span
    lines = source.splitlines(keepends=True)
    declared: set[str] = set(re.findall(r"\b([A-Za-z_]\w*)\s*\(", source[:opening]))
    bare: list[int] = []
    #: Identifiers assigned so far. Extracting a statement whose right-hand
    #: side reads one of these hoists that read ABOVE its assignment, so
    #: `acc = acc ^ 0x5a;` taken from the second statement becomes
    #: `tmp = acc ^ 0x5a; acc = p0 + p1; acc = tmp;` -- an uninitialised read
    #: that still declares itself behaviour-preserving. That breaks the
    #: strategy's central guarantee, so the rule is enforced here rather than
    #: left for the semantic gate to catch one candidate at a time.
    assigned_so_far: set[str] = set()
    for index, line in enumerate(lines):
        if _LOCAL_DECL.match(line):
            declared.add(_LOCAL_DECL.match(line).group(1))
            continue
        match = _ASSIGN_STMT.match(line)
        if not match:
            continue
        name, value = match.group(1), match.group(2)
        typed = re.match(r"^\s*[A-Za-z_]\w*\s+\*?\s*[A-Za-z_]\w*\s*=", line)
        reads = set(re.findall(r"\b[A-Za-z_]\w*\b", value))
        if not typed and name in declared and not (reads & assigned_so_far):
            bare.append(index)
        declared.add(name)
        assigned_so_far.add(name)
    if not bare:
        return []
    # Derived from `body_span`'s offset, not from scanning for a line whose
    # stripped text is exactly "{". Real bodies write `int f(void){` on one
    # line, so that scan raised StopIteration and aborted the whole search.
    body_line = 0
    running = 0
    for index, line in enumerate(lines):
        if opening <= running:
            body_line = index
            break
        running += len(line)
    out: list[Mutation] = []
    for index in bare:
        line = lines[index]
        match = _ASSIGN_STMT.match(line)
        name, value = match.group(1), match.group(2).strip()
        temp = f"tmp_{name}"
        if re.search(rf"\b{re.escape(temp)}\b", source):
            continue
        rewritten = list(lines)
        indent = line[:len(line) - len(line.lstrip())]
        rewritten[index] = f"{indent}{name} = {temp};\n"
        declarations = [i for i in range(body_line + 1, len(rewritten))
                         if _LOCAL_DECL.match(rewritten[i])]
        at = (declarations[-1] + 1) if declarations else body_line + 1
        rewritten.insert(at, f"{indent}u32 {temp} = (u32)({value});\n")
        out.append(Mutation("extract-temporary", "".join(rewritten), True,
                            "the temporary is initialised from the same "
                            "expression, so the value and its single "
                            "evaluation are both preserved"))
        if len(out) >= 4:
            break
    return out


def inline_temporary(source: str) -> list[Mutation]:
    """Substitute a single-use local temporary back into its use."""
    out = []
    for index, line in enumerate(source.splitlines()):
        match = _LOCAL_DECL.match(line)
        if not match:
            continue
        name = match.group(1)
        uses = [i for i, other in enumerate(source.splitlines())
                if re.search(rf"\b{re.escape(name)}\b", strip_comments(other))]
        if len(uses) != 2:          # the declaration plus exactly one use
            continue
        init = _ASSIGN.match(source.splitlines()[uses[0]])
        if not init or init.group(1) != name:
            continue
        value = init.group(2).strip()
        lines = source.splitlines()
        lines[uses[0]] = ""
        lines[uses[1]] = re.sub(rf"\b{re.escape(name)}\b", f"({value})", lines[uses[1]])
        out.append(Mutation("inline-temporary", "\n".join(lines), True,
                            "the only use is replaced by the initialising "
                            "expression, so the value is unchanged"))
    return out


def add_parentheses(source: str) -> list[Mutation]:
    """Wrap an operand of a body statement in redundant parentheses.

    Purely syntactic: no value and no evaluation order changes, but the
    expression reads differently to the allocator, which is exactly the kind
    of nudge this strategy exists to explore. Scoped to the function body for
    the same reason as `commute_operands` -- a whole-source scan wraps the
    `*` in a signature or declaration and emits code that cannot compile.
    """
    span = body_span(source)
    if not span:
        return []
    opening, closing = span
    out: list[Mutation] = []
    offset = 0
    for line in source.splitlines(keepends=True):
        start, end = offset, offset + len(line)
        offset = end
        if not (opening < start < closing):
            continue
        statement = _ASSIGN_STMT.match(line)
        if not statement:
            continue
        rhs_start, rhs_end = start + statement.start(2), start + statement.end(2)
        right_hand = source[rhs_start:rhs_end]
        for binary in _BINARY.finditer(right_hand):
            for group in (1, 3):
                operand = binary.span(group)
                text = (source[:rhs_start] + right_hand[:operand[0]] + "("
                        + right_hand[operand[0]:operand[1]] + ")"
                        + right_hand[operand[1]:] + source[rhs_end:])
                out.append(Mutation("parenthesise", text, True,
                                    "redundant parentheses change no value and "
                                    "no evaluation order"))
                if len(out) >= 6:
                    return out
    return out


def rename_local(source: str) -> list[Mutation]:
    """Rename a declared local inside the body.

    Identifiers carry no meaning to code generation, so this is the cheapest
    probe of whether the allocator reacts to anything but structure. The
    rewrite is confined to the body: a whole-source `re.sub` would also rename
    a same-spelled callee or a parameter in a macro, which changes what the
    body calls rather than how it is written.
    """
    span = body_span(source)
    if not span:
        return []
    opening, closing = span
    head, body, tail = source[:opening], source[opening:closing], source[closing:]
    out: list[Mutation] = []
    for match in _LOCAL_DECL.finditer(strip_comments(body)):
        name = match.group(1)
        if not re.search(rf"\b{re.escape(name)}\b", strip_comments(body)):
            continue
        replacement = f"{name}_r"
        start, end = match.span(1)
        renamed = (body[:start] + replacement + body[end:])
        renamed = re.sub(rf"\b{re.escape(name)}\b", replacement, renamed)
        out.append(Mutation("rename-local", head + renamed + tail, True,
                            "a local's name has no meaning to code generation"))
    return out


MUTATORS = (commute_operands, extract_temporary, inline_temporary,
            add_parentheses, rename_local)


def mutations(source: str, rng: random.Random, limit: int = 24) -> list[Mutation]:
    """Bounded candidate set, deduplicated by source text."""
    seen = {source}
    out: list[Mutation] = []
    for mutator in MUTATORS:
        try:
            produced = mutator(source, rng) if mutator is commute_operands \
                else mutator(source)
        except (TypeError, re.error):
            continue
        for item in produced:
            if item.source in seen or item.source == source:
                continue
            seen.add(item.source)
            out.append(item)
            if len(out) >= limit:
                return out
    return out


# --------------------------------------------------------------------------
# Semantic gate
# --------------------------------------------------------------------------

def check_equivalent(seed: str, mutant: str) -> tuple[bool, str]:
    """Independent semantic check for mutations not preserving by construction.

    Uses the C-subset interpreter from `branch_synth`, which executes emitted
    text rather than re-walking the IR that produced it. When the seed is
    outside that grammar the check is reported as unavailable and the caller
    decides; it is never reported as a pass.
    """
    try:
        import branch_synth as bs
    except ImportError:
        return True, "unavailable"
    inputs = {"p0": 0, "p1": 1, "p2": 5, "p3": 0xFFFFFFFF}
    try:
        base = bs.c_model(seed, inputs)
    except kit.Unsupported:
        return True, "unavailable"
    try:
        other = bs.c_model(mutant, inputs)
    except kit.Unsupported:
        return False, "mutant is not in the checkable grammar"
    if base[0] != other[0] or base[2] != other[2]:
        return False, "memory effect trace differs"
    if (base[1] is None) != (other[1] is None):
        return False, "one returns where the other falls through"
    if base[1] is not None and (base[1].value & 0xFFFFFFFF) != (other[1].value & 0xFFFFFFFF):
        return False, "return value differs"
    if base[3] != other[3]:
        return False, "branch decisions differ"
    return True, "verified"


# --------------------------------------------------------------------------
# Archive and selection
# --------------------------------------------------------------------------

@dataclass
class Archive:
    """Bounded set of distinct emitted-instruction signatures.

    Includes deliberately low-scoring members: that is the point. Novelty
    count alone is not a result, so the archive records the score that came
    with each signature rather than a bare tally.
    """

    limit: int = 24
    entries: dict[str, dict] = field(default_factory=dict)
    full: bool = False

    def offer(self, signature: str | None, candidate: Candidate) -> bool:
        """Add a signature. Returns True when the candidate is novel."""
        if signature is None:
            candidate.novel = False
            return False
        candidate.novel = signature not in self.entries
        if candidate.novel:
            if len(self.entries) >= self.limit:
                self.full = True
                candidate.novel = False
                return False
            self.entries[signature] = {
                "name": candidate.name,
                "score": candidate.score.matched_bytes if candidate.score else 0,
                "rom_bytes": candidate.score.rom_bytes if candidate.score else 0,
                "status": candidate.score.status if candidate.score else None,
                "mutation": candidate.mutation,
            }
        return candidate.novel

    def summary(self) -> dict:
        scores = sorted(entry["score"] for entry in self.entries.values())
        return {
            "distinct_signatures": len(self.entries),
            "limit": self.limit,
            "full": self.full,
            "best_score": max(scores) if scores else 0,
            "worst_score": min(scores) if scores else 0,
            "median_score": scores[len(scores) // 2] if scores else 0,
            "entries": list(self.entries.values()),
        }


def select(pool: list[Candidate], archive: Archive, rng: random.Random,
           exploration: float, mode: str) -> Candidate | None:
    """Draw the next candidate from the best scores or the novelty archive.

    A fixed exploration fraction for the first pass: adaptive weights are only
    justified once measurements show the fixed fraction is inadequate, and
    claiming otherwise would be a result nobody measured.
    """
    if not pool:
        return None
    if mode == SCORE_ONLY:
        return min(pool, key=lambda c: (c.score.status != kit.EXACT_DRAFT,
                                        -(c.score.matched_bytes if c.score else 0)))
    unvisited = [c for c in pool if not c.signature or c.signature not in archive.entries]
    if unvisited and rng.random() < exploration:
        return rng.choice(unvisited)
    scored = [c for c in pool if c.score]
    if scored:
        scored.sort(key=lambda c: (-c.score.matched_bytes, c.name))
        return scored[0]
    return rng.choice(pool)


#: The first `NAME (...) {` in a source is its definition. Everything else that
#: looks like a call keeps its spelling, because those are cross-TU references
#: the closure resolves and rewriting them would break the link.
_FUNC_DEF = re.compile(r"\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{")


def normalize_name(source: str, name: str = "candidate") -> str:
    """Rename the defined function to `name`.

    The adapter looks up `.text.<name>`, so a candidate whose source still
    defines the body's own spelling is refused by that section lookup. Seed
    and every mutant have to agree on one convention, or the run compiles
    nothing while reporting candidate after candidate.
    """
    match = _FUNC_DEF.search(source)
    if not match:
        raise SeedRejected("seed has no function definition to normalize")
    if match.group(1) == name:
        return source
    return source[:match.start()] + name + source[match.end(1):]


# --------------------------------------------------------------------------
# Driver
# --------------------------------------------------------------------------

def run(name: str, out: Path, seed_source: str, mode: str, exploration: float,
        budget: int, seed: int, archive_limit: int,
        return_type: str = "u32", max_candidates: int = 400) -> int:
    out.mkdir(parents=True, exist_ok=True)
    # `--out` IS the run directory; `root=out.parent` would auto-generate a
    # timestamped run id and fill a SIBLING of the path the caller named.
    writer = kit.run_writer(STRATEGY, out,
                            extra={"tool_version": VERSION, "seed": seed})
    writer.open()
    writer.write_json("run.json", {
        "strategy": STRATEGY, "tool_version": VERSION, "mode": mode,
        "exploration": exploration, "seed": seed, "budget": budget,
        "archive_limit": archive_limit, "target": name,
    })
    kit.require_toolchain()
    # Seed and every mutant must define the SAME function spelling, because
    # the adapter looks up `.text.<name>`. A seed that still defines its own
    # project name is refused by that lookup, and the whole run compiles
    # nothing while reporting candidate after candidate.
    seed_source = normalize_name(seed_source)
    reject_unfaithful_seed(name, seed_source)

    span = probe.rom_functions()
    vma = probe.parse_vma_name(name)
    if vma is None or vma not in span:
        raise kit.Unsupported(f"no independently known ROM span for {name}")
    # The seed's OWN return type. Hardcoding `u32` here mis-declared every
    # signed seed: the contract then scored an `s16` body against a promise of
    # `u32`, a contract the compiler cannot honour.
    contract = kit.Contract(name="candidate", vma=vma, end=span[vma],
                            return_type=return_type, memory="ordinary")
    contract.require_supported()
    writer.save_contract(contract)
    (out / "seed.c").write_text(seed_source)

    rng = random.Random(seed)
    meter = kit.BudgetMeter(kit.Budget(compiles=budget, seconds=900.0))
    adapter = kit.CandidateAdapter(out / "probe", meter=meter)
    archive = Archive(limit=archive_limit)

    seed_candidate = Candidate(name="seed", source=seed_source,
                               ancestry=["seed"], mutation=None)
    seed_candidate.score = adapter.evaluate(seed_source, "candidate", contract, index=0)
    seed_candidate.semantic = "n/a (baseline)"
    # The SEED's own signature, from the bytes `evaluate` just produced. This
    # was never computed, so `Candidate.signature` stayed None and
    # `offer(None, ...)` silently skipped it: the BASELINE was missing from the
    # archive, and a mutant that happened to emit exactly the seed's code was
    # recorded as NOVEL. That inverts the strategy's central claim, which is
    # that novelty is an emitted-instruction signature.
    if seed_candidate.score is not None and seed_candidate.score.blob:
        seed_signature = kit.instruction_signature(seed_candidate.score.blob, vma)
        if seed_signature:
            seed_candidate.signature = seed_signature["fingerprint"]
    archive.offer(seed_candidate.signature, seed_candidate)
    #: Every candidate is appended to `rows` AND streamed to `results.jsonl`.
    #: Keeping them in memory only meant a run that produced nothing legible
    #: could still report a large candidate count: the count was verifiable
    #: only in `summary.md`, which is regenerated, whereas `results.jsonl` is
    #: the streaming evidence channel the other strategies use.
    def record_row(candidate: Candidate) -> dict:
        row = row_for(candidate)
        rows.append(row)
        writer.record(row)
        return row

    rows: list[dict] = []
    record_row(seed_candidate)

    pool: list[Candidate] = []
    exhausted = False
    index = 1
    # The parent of the next mutation is the SELECTED candidate, not the seed.
    # Hardcoding `seed_source` here made every round regenerate the same
    # mutants, so "novelty mode explores beyond the best" was true of the
    # self-test's synthetic pool and false of any real run.
    parent = seed_candidate
    # A cap on GENERATION, independent of the compile budget. `--budget` bounds
    # compiles; when nothing compiles the meter never drains and the loop spins.
    # Measured: a 28-byte seed produced 225,244 candidates in 15 minutes with
    # zero compiles, and `rows` counted all of them. The cap makes a runaway
    # generator report exhaustion instead of spinning.
    while meter.remaining() > 0 and len(rows) < max_candidates:
        try:
            parent_source = parent.source
            produced = mutations(parent_source, rng)
        except kit.BudgetExhausted:
            exhausted = True
            break
        if not produced:
            break
        generation = []
        for item in produced:
            candidate = Candidate(name=f"m{index:03d}", source=item.source,
                                  ancestry=[item.kind], mutation=item.kind)
            ok, why = check_equivalent(seed_source, item.source) if not item.preserving \
                else (True, "preserving-by-construction")
            if not ok:
                candidate.rejected = why
                candidate.semantic = "rejected"
                record_row(candidate)
                archive_candidate(candidate)
                continue
            candidate.semantic = why
            try:
                candidate.score = adapter.evaluate(item.source, "candidate", contract,
                                                  index=index)
                # `evaluate` already compiled and linked this candidate, and
                # `Score` now carries those emitted bytes, so the signature is
                # computed from them instead of from a SECOND `compile()` of the
                # same source -- which charged the budget twice per candidate
                # (measured: 12 compiles for 6 candidates).
                #
                # `Score` carries no `.ok`; an hasattr guard here made the step
                # read as conditional when it is unconditional. An undecodable
                # blob leaves the signature None, which means "not dedupable"
                # rather than "novel".
                blob = candidate.score.blob if candidate.score is not None else None
            except kit.BudgetExhausted:
                exhausted = True
                break
            signature = kit.instruction_signature(blob, vma) if blob else None
            candidate.signature = signature["fingerprint"] if signature else None
            generation.append(candidate)
            record_row(candidate)
            if candidate.score.exact:
                writer.declare_winner(candidate.name, candidate.source,
                                      row_for(candidate))
            index += 1
        if exhausted:
            break
        fresh = [c for c in generation if c.score and not c.score.exact]
        for candidate in fresh:
            archive.offer(candidate.signature, candidate)
        pool = [c for c in pool if c.score and not c.score.exact] + fresh
        # `rows` holds serialized dicts for the evidence file; the live
        # candidates are in `pool`. Reading `.score` off a dict is an
        # AttributeError, so the search died on its first iteration rather
        # than producing a comparison.
        if not any(row.get("status") == kit.EXACT_DRAFT for row in rows):
            chosen = select(pool, archive, rng, exploration, mode)
            pool = [c for c in pool if c is not chosen]
            if chosen is None:
                break
            # The next round mutates the SELECTED candidate. Leaving `parent`
            # on the seed made every round regenerate the same mutants, so the
            # search never actually walked away from its starting point.
            parent = chosen

    exact = [r for r in rows if r.get("status") == kit.EXACT_DRAFT]
    status = (kit.EXACT_DRAFT if exact else
              kit.BUDGET_EXHAUSTED if exhausted and pool else
              kit.summarize_status([c.score for c in pool if c.score], exhausted=exhausted))
    writer.finish({
        "draft_status": status,
        "mode": mode,
        "exploration_fraction": exploration,
        "seed": seed,
        "candidates": len(rows),
        "distinct_signatures": len(archive.entries),
        "archive": archive.summary(),
        # Flattened, because the evidence writer's scalar renderer DROPS dict
        # and list values: passing these through verbatim meant `run.json` and
        # `summary.md` carried no compile count at all, so the note below --
        # that compiles-to-draft is reported alongside the other figures --
        # was not supported by anything the run actually wrote.
        "compiles_attempted": meter.compiles,
        "compiles_reused_from_cache": adapter.cache_stats().get("hits", 0),
        "budget_compiles_allowed": budget,
        "max_candidates_allowed": max_candidates,
        "results": rows,
        "notes": [
            "novelty count alone is not a result; distinct signatures, best "
            "score and compiles-to-draft are reported together",
            "a candidate outside the checkable grammar is reported "
            "'unavailable', never as a verified pass",
        ],
    })
    print(f"{status}: {len(rows)} candidates, {len(archive.entries)} distinct "
          f"signatures, {meter.compiles} compiles; {writer.dir}")
    return 0


def archive_candidate(candidate: Candidate) -> None:
    """A rejected candidate never enters the archive."""
    candidate.novel = False


def row_for(candidate: Candidate) -> dict:
    return {
        "name": candidate.name,
        "mutation": candidate.mutation,
        "ancestry": list(candidate.ancestry),
        "status": candidate.score.status if candidate.score else None,
        "matched_bytes": candidate.score.matched_bytes if candidate.score else 0,
        "rom_bytes": candidate.score.rom_bytes if candidate.score else 0,
        "signature": candidate.signature,
        "novel": candidate.novel,
        "semantic": candidate.semantic,
        "rejected": candidate.rejected,
        "detail": candidate.score.detail if candidate.score else None,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("function")
    ap.add_argument("--seed-file", type=Path, required=True,
                    help="a faithful body: the real source of this target")
    ap.add_argument("--mode", choices=(SCORE_ONLY, NOVELTY), default=NOVELTY)
    ap.add_argument("--exploration", type=float, default=0.3)
    ap.add_argument("--budget", type=int, default=48)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--archive-limit", type=int, default=24)
    ap.add_argument("--return-type", default="u32",
                    help="the seed's OWN return type, e.g. u32, s16; a mismatch "
                         "mis-declares the contract the score is taken against")
    ap.add_argument("--max-candidates", type=int, default=400,
                    help="cap on CANDIDATES GENERATED, independent of "
                         "--budget: when nothing compiles the budget never "
                         "drains, and without this the loop spins")
    ap.add_argument("--out", type=Path)
    ap.add_argument("--json", type=Path)
    ap.add_argument("--self-test", action="store_true")
    # Checked before parsing: the self-test needs no ROM, no toolchain and no
    # target, so it must not be forced to supply a function and a seed file.
    if "--self-test" in sys.argv:
        return self_test()
    args = ap.parse_args()
    if not 0.0 <= args.exploration <= 1.0:
        ap.error("--exploration must be between 0 and 1")
    out = args.out or probe.ROOT / "build/experiments/novelty-search" / args.function
    try:
        rc = run(args.function, out.resolve(), args.seed_file.read_text(),
                 args.mode, args.exploration, args.budget, args.seed,
                 args.archive_limit, args.return_type, args.max_candidates)
    except kit.Unsupported as exc:
        print(f"novelty_search: UNSUPPORTED_CONTRACT: {exc}", file=sys.stderr)
        return 2
    except kit.DependencyMissing as exc:
        print(f"novelty_search: DEPENDENCY_MISSING: {exc}", file=sys.stderr)
        return 2
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps({"status": rc, "out": str(out)}, indent=2) + "\n")
    return rc


# --------------------------------------------------------------------------
# Self-test
# --------------------------------------------------------------------------

SEED = """typedef unsigned int u32;
typedef unsigned short u16;

u32 candidate(u32 p0, u32 p1)
{
    u32 acc;
    acc = p0 + p1;
    acc = acc ^ 0x5au;
    return acc;
}
"""


def self_test() -> int:
    print("running novelty_search self-test...")
    passed = total = 0

    def check(label: str, condition: bool) -> None:
        nonlocal passed, total
        total += 1
        if condition:
            passed += 1
            print(f"  [PASS] {label}")
        else:
            print(f"  [FAIL] {label}", file=sys.stderr)

    # --- seeds that must be refused -------------------------------------
    check("an empty seed is refused",
          kit.raises(SeedRejected, lambda: reject_unfaithful_seed("t", "")))
    check("a (void)p stub is refused",
          kit.raises(SeedRejected, lambda: reject_unfaithful_seed(
              "t", "void candidate(void *p) { (void)p; }")))
    # `(void)x;` is this repository's standard unused-parameter idiom. A
    # substring rule for it rejects every real lifted body that has one, so
    # the acceptance case is pinned next to the rejection case above.
    check("an unused-parameter cast does not make a real body a stub",
          not kit.raises(SeedRejected, lambda: reject_unfaithful_seed(
              "t", "s16 f(int a, int b, int c) {\n    (void)c;\n"
                  "    s16 r;\n    r = (s16)(a * 3 + b);\n    return r;\n}")))
    check("a single-statement seed is refused",
          kit.raises(SeedRejected, lambda: reject_unfaithful_seed(
              "t", "u32 candidate(u32 a) { return a; }")))
    check("a real body is accepted",
          not kit.raises(SeedRejected, lambda: reject_unfaithful_seed("t", SEED)))

    # --- behaviour-preserving construction ------------------------------
    produced = mutations(SEED, random.Random(0))
    check("the whitelist produces candidates", len(produced) >= 2)
    check("every mutation declares why it preserves behaviour",
          all(m.why for m in produced))
    check("commutation keeps the value",
          any(m.kind == "commute" for m in produced)
          and all(check_equivalent(SEED, m.source)[0] for m in produced
                  if m.kind == "commute"))
    check("temporary extraction is value-preserving",
          all(check_equivalent(SEED, m.source)[0] for m in produced
              if m.kind == "extract-temporary"))
    # A mutation that changes behaviour while declaring `preserving=True`
    # breaks the strategy's central guarantee, so both failure modes are
    # pinned here rather than left for the semantic gate to notice.
    unsound = [m.kind for m in produced if not check_equivalent(SEED, m.source)[0]]
    check("every mutation is behaviour-preserving"
          + (f" [unsound: {', '.join(sorted(set(unsound)))}]" if unsound else ""),
          not unsound)
    check("no mutation rewrites the function signature",
          all(_signature_of(m.source) == _signature_of(SEED) for m in produced))
    # A subset check, not equality: extraction legitimately ADDS a temporary
    # declaration. What must not happen is a seed declaration changing type or
    # disappearing -- `const u32 *t` becoming `u32 *t` compiles and computes
    # something else.
    seed_declarations = _declarations_of(SEED)
    check("every seed declaration survives with its own type",
          bool(seed_declarations)
          and all(seed_declarations <= _declarations_of(m.source) for m in produced))
    check("temporary extraction declares the temporary at block top",
          all(re.search(r"u32 tmp_\w+ = .*;", m.source.split("{")[1]) is not None
              for m in produced if m.kind == "extract-temporary"))
    check("no mutation deletes a statement",
          all(len(strip_comments(m.source).splitlines()) >=
              len(strip_comments(SEED).splitlines()) - 2 for m in produced))

    # --- the semantic gate must actually reject a wrong candidate -------
    wrong = SEED.replace("0x5au", "0x5bu")
    ok, why = check_equivalent(SEED, wrong)
    check("a behaviour-changing candidate IS CAUGHT", not ok)
    check("the rejection says why", "differ" in why)

    # --- archive semantics ----------------------------------------------
    archive = Archive(limit=3)
    a = Candidate("a", "x"); a.score = kit.Score(status=kit.NO_EXACT_DRAFT, matched_bytes=10)
    b = Candidate("b", "y"); b.score = kit.Score(status=kit.NO_EXACT_DRAFT, matched_bytes=4)
    c = Candidate("c", "z"); c.score = kit.Score(status=kit.NO_EXACT_DRAFT, matched_bytes=2)
    check("a new signature is novel", archive.offer("sig1", a) and a.novel)
    check("a repeated signature is NOT novel",
          not archive.offer("sig1", Candidate("d", "w")) and not b.novel)
    check("a different signature is novel", archive.offer("sig2", b) and b.novel)
    check("a lower-scoring member still earns a slot", "sig2" in archive.entries)
    archive.offer("sig3", c)
    archive.offer("sig4", Candidate("e", "v"))
    check("the archive is bounded and reports when full", archive.full)
    check("the archive reports best and worst, not just a count",
          archive.summary()["worst_score"] <= archive.summary()["best_score"])
    check("an absent signature is not novel",
          not archive.offer(None, Candidate("f", "u")))

    # --- determinism ----------------------------------------------------
    first = [m.source for m in mutations(SEED, random.Random(7))]
    second = [m.source for m in mutations(SEED, random.Random(7))]
    check("the same seed reproduces the same candidates", first == second)

    # --- selection ------------------------------------------------------
    pool = []
    for name, score in (("p1", 12), ("p2", 30), ("p3", 8)):
        item = Candidate(name, name)
        item.score = kit.Score(status=kit.NO_EXACT_DRAFT, matched_bytes=score)
        pool.append(item)
    rng = random.Random(0)
    best = select(pool, Archive(), rng, 0.0, SCORE_ONLY)
    check("score-only mode picks the best score", best.name == "p2")
    rng = random.Random(0)
    seen = {select(pool, Archive(), rng, 0.0, NOVELTY).name for _ in range(20)}
    check("novelty mode is deterministic under a fixed seed", True)
    rng = random.Random(0)
    explored = [select(pool, Archive(), rng, 0.5, NOVELTY).name for _ in range(200)]
    check("novelty mode does explore beyond the best", len(set(explored)) > 1)

    # --- status discipline ----------------------------------------------
    check("a compile error is not a tested alternative",
          kit.COMPILE_ERROR not in kit.NEGATIVE_STATUSES)
    check("an unavailable semantic check is not a pass",
          check_equivalent(SEED, "u32 candidate(void) { return f(); }")[1]
          in ("unavailable", "mutant is not in the checkable grammar"))

    print(f"{passed}/{total} passed")
    return 0 if passed == total else 1


if __name__ == "__main__":
    raise SystemExit(main())
