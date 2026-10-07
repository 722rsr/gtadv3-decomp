#!/usr/bin/env python3
"""Route a matching target to the documented next move.

Targets must be stratified *by problem* rather than by match ratio. Routing
otherwise lives in prose, so a campaign either sends everything to the newest
tool or falls back to blind source permutation. This tool makes the routing
mechanical and evidence-based: given a target's measured state, it names the
next move, the reason, and the exact command.

It is a router, not a matcher. It never edits source, never compiles, and
never promotes. Its only outputs are a recommendation, its evidence, and the
command a user would run.

Two rules it enforces, both from the workflow's hard-won lessons:

  * A body whose real problem is assembly metadata or unresolved ownership is
    routed to **manual preflight**, not to a codegen strategy. The P0 bands
    and the unresolved-relocation band look like compiler problems and are
    not; sending a user to shape C for them wastes the comparison.
  * Eligibility is checked, not assumed. `branch_synth` refuses calls and
    stack use; `novelty_search` needs a faithful seed; `recipe_miner` needs two
    independently verified examples. A router that recommends a tool whose
    preconditions fail sends the user to re-derive the refusal.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass, field, asdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import corpus_match_probe as probe
import match_families as families
import lift_scout

ROOT = probe.ROOT
ROM_BASE = probe.ROM_BASE

DEFAULT_REPORT = ROOT / "build/era-corpus/ready-report.json"
DEFAULT_INDEX = ROOT / "build/matching-assist/families.json"

MANUAL = "manual-preflight"

# --------------------------------------------------------------------------
# Per-band routing. The band taxonomy is `lift_scout`'s, which is the
# measured one; this table only decides which documented move each band
# warrants. Keeping it as data (rather than branching inline) is what lets the
# self-test assert the whole routing table at once.
# --------------------------------------------------------------------------

@dataclass(frozen=True)
class Route:
    strategy: str
    reason: str
    #: Extra conditions the user must satisfy; surfaced, not enforced here.
    requires: tuple[str, ...] = ()


BAND_ROUTES: dict[str, Route] = {
    "P0-span-gap": Route(
        MANUAL,
        "assembly metadata, not codegen: the function inventory never saw this "
        "VMA, so the span is wrong and any byte score computed against it is "
        "meaningless. Fix the inventory first.",
        ("add the typed entry and a bare label (zero bytes emitted)",
         "make independent-slice && make ownership-map",
         "confirm with tools/label_census.py"),
    ),
    "P0-untyped-bl": Route(
        MANUAL,
        "diagnostic band, not a dispatch target: a BL-reached body is already "
        "an inventory entry and its span is measured. The real suspect is an "
        "INTERIOR function with no label at all, whose bytes inflate this "
        "body's span. Do not spend the comparison here.",
        ("run tools/label_census.py to find an unlabelled interior entry",),
    ),
    "P9-unresolved-reloc": Route(
        MANUAL,
        "call/pool relocation does not resolve, so the candidate was never "
        "linked at a real address. Ownership and spelling first; codegen "
        "shaping cannot fix an unresolved symbol.",
        ("python3 tools/call_audit.py", "python3 tools/export_audit.py"),
    ),
    "P1-epilogue": Route(
        MANUAL,
        "the ROM tail returns through a different register than the declaration "
        "claims. This is a declaration defect; a codegen strategy cannot see "
        "it.",
        ("read the ROM tail halfword and fix the declared return type",),
    ),
    "P4-pool-order": Route(
        "compiler_microscope.py",
        "pool load ordered after the shift pair feeding its address: a "
        "literal-folding decision inside the compiler. Diagnose the pass "
        "before changing source shape again.",
        ("docs/findings/track_car_26180_pool_order.md records the solved case",
         "two controlled variants, one source feature changed"),
    ),
    "P1-register-choice": Route(
        MANUAL,
        "the report's own comparison already agrees on every mnemonic, "
        "operand, branch target and pool word: only the register NUMBERS "
        "differ, so this is a gcc-2.95 `local_alloc` decision and no C source "
        "spelling reaches it. Measured : re-shaping the expression "
        "moved the mismatch without closing it, and a parameter-width change "
        "cannot apply at all when the body has no parameter. The lever that "
        "closed two of these in one comparison is a GNU local register-variable pin.",
        ("pin the divergent value: `register u32 g __asm__(\"r3\");` -- the "
         "construct already used at src/ai_line_leaves.c and "
         "src/code_22d20.c for this exact class",
         "one pin per body: tools/agbcc_c89_transform.py cannot handle two in "
         "one declaration block",
         "prove the pin is load-bearing by removing it and reproducing the "
         "original residual byte for byte"),
    ),
    "P1-statement-order": Route(
        MANUAL,
        "the right instructions in the wrong order, so the fix is the order of "
        "the source statements and not their types. agbcc creates incoming-"
        "parameter pseudos first and then locals in DECLARATION order, so "
        "declaration order among locals is a real lever rather than a guess.",
        ("reorder the statements into the ROM's order and re-probe",
         "do not change a type or a qualifier first: that moves the mismatch "
         "instead of closing it"),
    ),
    "P2-narrow-param": Route(
        MANUAL,
        "the declaration names a sub-word SCALAR parameter, and that is ALL "
        "this band measures -- it reads declaration text and never compares "
        "the candidate's prologue against the ROM's. The story this band used "
        "to tell, that the candidate carries an lsls/lsrs extend the ROM "
        "lacks, is unfounded: measured  on all 11 bodies then in "
        "the band, BOTH sides carried the extend, so widening the parameter "
        "would delete four correct bytes.",
        ("diff the two prologues before changing anything",
         "if the ROM has the extend, this is register allocation or statement "
         "order, not parameter width"),
    ),
    "P3-volatile-load": Route(
        "compiler_microscope.py",
        "sub-word load lands in the address register plus a copy: a qualifier "
        "and load-form decision.",
        ("test the volatile qualifier as a single controlled variant",),
    ),
    "P5-register-alloc": Route(
        "novelty_search.py",
        "right size, diverges late: the destination-register choice. Byte-"
        "similarity search stagnates here, and this is exactly the plateau "
        "novelty-assisted selection is for.",
        ("a faithful seed body is required; a stub is refused",),
    ),
    "P6-switch-shape": Route(
        "branch_synth.py",
        "the ROM's branch topology is a balanced tree reusing one compare, "
        "which a linear C if/else chain cannot express. This is a control-flow "
        "shape problem.",
        ("the switch rewrite may fix it without synthesis",),
    ),
    "P9-frame-limit": Route(
        "compiler_microscope.py",
        "agbcc emits a frame the frameless ROM lacks. Frequently measured "
        "inexpressible, so diagnose the decision before assuming a source fix.",
        ("worth a user only with a specific hypothesis",),
    ),
    "P9-true-lift": Route(
        "branch_synth.py",
        "genuine transcription, not a shaping defect. Synthesis is the "
        "documented route for bodies beyond the straight-line grammar.",
        ("dispatch with a full context packet",),
    ),
}

#: Bands that a codegen strategy must never be dispatched at, because the real
#: problem is not codegen. Kept as a set so the self-test can assert that no
#: route table entry contradicts it.
NON_CODEGEN_BANDS = frozenset({"P0-span-gap", "P0-untyped-bl", "P9-unresolved-reloc", "P1-epilogue"})


# --------------------------------------------------------------------------
# Measured target state
# --------------------------------------------------------------------------

@dataclass
class TargetState:
    """Everything the router is allowed to reason from. All of it measured."""

    name: str
    vma: int
    end: int
    rom_bytes: int = 0
    #: From the shared corpus report; None when the body is absent from it.
    probe_status: str | None = None
    prefix: int | None = None
    cand_bytes: int | None = None
    #: The report's OWN miss classification, and the C source it was scored
    #: from. `band_for` must pass both through, or it silently disagrees with
    #: the `lift_scout` classifier it exists to mirror.
    report_class: str | None = None
    source: str = ""
    matched_bytes: int | None = None
    band: str | None = None
    band_reason: str = ""
    #: ROM control-flow facts.
    conditionals: int = 0
    calls: int = 0
    pool_loads: int = 0
    instructions: int = 0
    flow_failures: tuple[str, ...] = ()
    #: Family evidence.
    family_members: int = 0
    family_verified: int = 0
    has_faithful_source: bool = False

    @property
    def match_ratio(self) -> float | None:
        if not self.rom_bytes or self.prefix is None:
            return None
        return round(self.prefix / self.rom_bytes, 4)

    def to_json(self) -> dict:
        data = asdict(self)
        data["vma"] = f"{self.vma:#010x}"
        data["end"] = f"{self.end:#010x}"
        data["file_offset"] = f"{self.vma - ROM_BASE:#x}"
        data["flow_failures"] = list(self.flow_failures)
        return data


def rom_features(rom: bytes, vma: int, end: int) -> dict:
    """Control-flow facts measured from the target's own bytes.

    Uses `match_families.flow`, which is deliberately conservative: an
    unresolved transfer, an out-of-span branch or an unsupported opcode is
    reported as a failure rather than guessed. Those failures are load-bearing
    here -- they are what makes a target ineligible for a synthesis strategy,
    so they must not be smoothed away.
    """
    try:
        seen, _, failures = families.flow(rom, vma, end)
    except ValueError as exc:
        return {"conditionals": 0, "calls": 0, "pool_loads": 0, "instructions": 0,
                "flow_failures": [str(exc)]}
    return {
        "conditionals": sum(1 for _, _, kind in seen.values() if kind.startswith("cond")),
        "calls": sum(1 for _, _, kind in seen.values() if kind == "call"),
        "pool_loads": sum(1 for h, _, _ in seen.values() if h & 0xF800 == 0x4800),
        "instructions": len(seen),
        "flow_failures": failures,
    }


def state_from_report(rec: dict, rom: bytes) -> TargetState:
    """Build a TargetState from one `corpus_match_probe` result record."""
    vma = int(rec["vma"], 16)
    size = rec.get("rom_bytes", 0)
    state = TargetState(
        name=rec.get("name", "?"),
        vma=vma,
        end=vma + size,
        rom_bytes=size,
        probe_status=rec.get("status"),
        prefix=rec.get("prefix"),
        cand_bytes=rec.get("cand_bytes"),
        report_class=rec.get("class"),
        source=rec.get("source", ""),
        matched_bytes=rec.get("matched_bytes"),
    )
    for key, value in rom_features(rom, vma, state.end).items():
        setattr(state, key, tuple(value) if key == "flow_failures" else value)
    return state


def family_evidence(index: dict | None, target: TargetState) -> tuple[int, int]:
    """(members, verified) for the target's ROM instruction family.

    `verified` counts members already EXACT or promoted -- those are the only
    examples a recipe may be mined from, because an unverified sibling proves
    nothing about what agbcc does.
    """
    if not index:
        return 0, 0
    fingerprint = None
    for rec in index.get("records", []):
        try:
            if int(rec["vma"], 16) == target.vma:
                fingerprint = rec.get("fingerprint")
                break
        except (KeyError, ValueError):
            continue
    if fingerprint is None:
        return 0, 0
    members = [r for r in index.get("records", [])
               if r.get("fingerprint") == fingerprint and r.get("complete")]
    verified = [r for r in members if r.get("status") == "EXACT" or r.get("promoted")]
    return len(members), len(verified)


# --------------------------------------------------------------------------
# Routing
# --------------------------------------------------------------------------

@dataclass
class Decision:
    strategy: str
    reason: str
    requires: tuple[str, ...] = ()
    alternatives: list[str] = field(default_factory=list)
    #: Reasons a plausible strategy was NOT chosen, so a user does not
    #: re-derive the refusal on its own.
    declined: list[str] = field(default_factory=list)

    def to_json(self) -> dict:
        data = asdict(self)
        data["requires"] = list(self.requires)
        return data


def route(state: TargetState) -> Decision:
    """Choose the documented next move from measured evidence alone."""
    # 1. Non-codegen problems win over everything. A byte score computed
    #    against a wrong span, or against unresolved symbols, is not evidence
    #    about codegen, and no strategy below can see those faults.
    band = state.band
    if band in NON_CODEGEN_BANDS:
        return Decision(BAND_ROUTES[band].strategy, BAND_ROUTES[band].reason,
                        BAND_ROUTES[band].requires)

    alternatives: list[str] = []
    declined: list[str] = []

    # 2. Unresolved control flow is a *synthesis* eligibility fact, not a
    #    global veto. A bounded model genuinely cannot be trusted against a
    #    span whose boundaries the ROM has not fixed -- but a source-level
    #    strategy (microscope, miner, novelty) never models the ROM, and a
    #    large body with calls fails `flow` routinely. Vetoing globally was
    #    measured to route all 903 queued bodies to manual: a plausible
    #    output with no dispatch behind it.
    synth_eligible = (state.conditionals >= 1 and state.calls == 0
                      and state.instructions > 0 and not state.flow_failures)

    base = BAND_ROUTES.get(band or "", None)

    # 3. A repeated family with independently verified siblings is the one
    #    case where mining beats shaping, regardless of band.
    if state.family_verified >= 2:
        return Decision(
            "recipe_miner.py",
            f"the ROM instruction family has {state.family_verified} verified "
            f"members out of {state.family_members}. A mined template is "
            "cheaper and more likely to hit the allocator than open-ended "
            "expression shaping.",
            ("examples are re-verified now, never reused from a stale probe",),
        )
    if state.family_members >= 2:
        declined.append("recipe_miner.py: family has "
                        f"{state.family_verified} verified members, needs >= 2")

    # 4. Synthesis eligibility, decided above, decides whether the bounded
    #    model is dispatched or refused -- and the refusal states which
    #    measured fact caused it, so the user does not re-derive it.
    if base and base.strategy == "branch_synth.py":
        if synth_eligible:
            return Decision(base.strategy, base.reason, base.requires)
        if state.flow_failures:
            declined.append(
                f"branch_synth.py: {len(state.flow_failures)} control-flow "
                f"failure(s) in span (first: {state.flow_failures[0]}); the "
                "bounded model needs a resolved boundary")
        elif state.calls:
            declined.append("branch_synth.py: ROM contains "
                            f"{state.calls} call(s); the bounded model refuses calls")
        else:
            declined.append("branch_synth.py: ROM is straight-line; "
                            "tools/leaf_synth.py covers this grammar")
        # Declined: return the manual transcription route explicitly. Falling
        # through to the generic band branch below would re-recommend
        # `branch_synth.py` for the body it just refused, which is how a
        # router talks a user straight into re-deriving the refusal.
        return Decision(
            MANUAL,
            _fallback_reason(state.band, state),
            ("python3 tools/match_context.py " + state.name),
            declined=declined,
        )

    if base:
        return Decision(base.strategy, base.reason, base.requires,
                        _alternatives_for(base.strategy), declined)

    # 5. Unknown band: report the measured facts rather than guess.
    return Decision(
        MANUAL,
        "no band classification is available for this target, so no strategy "
        "is recommended. Dispatch with a full context packet.",
        declined,
    )


def _alternatives_for(strategy: str) -> list[str]:
    return {
        "branch_synth.py": ["leaf_synth.py"],
        "compiler_microscope.py": ["novelty_search.py"],
        "novelty_search.py": ["compiler_microscope.py", "egraph_search.py"],
        "recipe_miner.py": [],
    }.get(strategy, [])


#: Why a band's body must be handled by hand when the bounded synthesis model
#: is ineligible. The band still reaches the synthesis check first -- a small
#: P6 body with one conditional really is a synthesis target -- but when the
#: model declines, the documented lever is the one stated here. For P6 that is
#: the if/else-chain to `switch` rewrite: measured across the band, its bodies
#: run 728-1176 bytes with up to 32 calls, so the one-conditional/no-call
#: synthesis scope does not apply and the lever is a source rewrite.
MANUAL_FALLBACK: dict[str, str] = {
    "P6-switch-shape": (
        "the ROM's dispatch tree is large and call-bearing, outside the "
        "one-conditional/no-call synthesis scope. The documented lever is a "
        "source rewrite, not synthesis: agbcc emits an if/else chain as a "
        "linear test, so rewrite the same cases as `switch (v) { case A: ... }`. "
        "The compared value's SIGNEDNESS changes the branch (int gives "
 "bgt/ble, u32 gives bhi/bls), and agbcc emits case blocks in SOURCE order, "
 "so reordering the labels moves them to the ROM's addresses."),
    "P9-true-lift": (
        "genuine transcription that the bounded synthesis model cannot "
        "express. Lift it by hand from the ROM, not by permutation."),
}


def _fallback_reason(band: str | None, state: TargetState) -> str:
    reason = MANUAL_FALLBACK.get(band or "")
    if reason:
        return reason
    return (f"genuine transcription that the bounded synthesis model cannot "
            f"express: {state.conditionals} conditional(s), {state.calls} "
            f"call(s), {len(state.flow_failures)} unresolved control-flow "
            "site(s). Lift it by hand from the ROM, not by permutation.")


def command_for(decision: Decision, state: TargetState) -> list[str]:
    """The exact command a user would run for this decision.

    Contract flags the router has no evidence for are left as explicit
    placeholders rather than filled in. `--return-type` and `--memory` are
    caller-selected facts read off the ROM and its callers; inventing a value
    here would convert a missing ABI decision into a confident wrong one, and
    the resulting score would look like evidence about the strategy.
    """
    base = [sys.executable, "tools"]
    if decision.strategy == MANUAL:
        return base + ["match_context.py", state.name,
                       "--out", f"build/experiments/router/{state.name}"]
    if decision.strategy == "recipe_miner.py":
        argv = base + ["recipe_miner.py", "--target", state.name]
    else:
        argv = base + [decision.strategy, state.name]
        if decision.strategy == "branch_synth.py":
            argv += ["--return-type", "<ROM + caller evidence>",
                     "--memory", "<ordinary|volatile>"]
        elif decision.strategy == "compiler_microscope.py":
            argv += ["<variant-a.c>", "<variant-b.c>"]
        elif decision.strategy == "novelty_search.py":
            argv += ["--seed-file", "<faithful seed .c>"]
        elif decision.strategy == "egraph_search.py":
            argv += ["--return-type", "<ROM + caller evidence>"]
        elif decision.strategy == "cross_rom_archaeology.py":
            argv = base + ["cross_rom_archaeology.py", "--subsystem",
                           "<input|save|menu records>", "--vma", f"{state.vma:#010x}"]
    argv += ["--out", f"build/experiments/experiments/{state.name}"]
    return argv


def band_for(state: TargetState, rom: bytes, *, typed: set[int], reached: set[int]) -> str | None:
    """Run `lift_scout`'s own classifier so the router cannot drift from it.

    The record and body must carry the SAME inputs `lift_scout` was given.
    This used to pass an empty body, an empty `source` and a null `class`,
    which made every band that reads declaration text or the report's own miss
    classification unreachable from here -- P2, P3, P4, P6 and both
    measured-class bands, i.e. most of the actionable ones. The router then
    agreed with nothing it claimed to mirror.
    """
    rec = {"name": state.name, "vma": f"{state.vma:#010x}",
           "status": state.probe_status or "NO_OVERLAP",
           "rom_bytes": state.rom_bytes, "cand_bytes": state.cand_bytes or 0,
           "prefix": state.prefix if state.prefix is not None else 0,
           "first_diff": state.prefix, "source": state.source,
           "class": state.report_class,
           "matched_bytes": state.matched_bytes}
    shape = lift_scout.rom_shape(rom, state.vma, state.rom_bytes)
    body = ""
    if state.source:
        try:
            body = lift_scout.source_body(lift_scout.ROOT / state.source,
                                          state.name) or ""
        except Exception:
            body = ""
    try:
        cls, _why = lift_scout.classify(rec, shape, body, typed=state.vma in typed,
                                        reached=state.vma in reached)
    except Exception:
        return None
    return cls


# --------------------------------------------------------------------------
# Driver
# --------------------------------------------------------------------------

def load_index(path: Path) -> dict | None:
    """Load the family index, refusing a stale one rather than trusting it."""
    if not path.exists():
        return None
    try:
        return families.load_index(path)
    except (ValueError, OSError):
        return None


def route_named(name: str, report: dict, rom: bytes, index: dict | None,
                typed: set[int], reached: set[int]) -> Decision | None:
    """Locate one named body in the report and route it. None when absent."""
    wanted = probe.parse_vma_name(name)
    for rec in report.get("results", []):
        try:
            if rec.get("name") != name and (wanted is None or int(rec["vma"], 16) != wanted):
                continue
        except (KeyError, ValueError):
            continue
        state = state_from_report(rec, rom)
        state.family_members, state.family_verified = family_evidence(index, state)
        state.band = band_for(state, rom, typed=typed, reached=reached)
        if state.band:
            state.band_reason = BAND_ROUTES.get(state.band, Route("", "")).reason
        return route(state)
    return None


def route_queue(queue: dict, report: dict, rom: bytes, index: dict | None,
                typed: set[int], reached: set[int], top: int) -> list[dict]:
    """Route a whole `lift_scout` queue, cheapest band first."""
    by_vma = {int(r["vma"], 16): r for r in report.get("results", [])
              if isinstance(r.get("vma"), str)}
    rows = []
    for row in queue.get("queue", []):
        try:
            vma = int(row["vma"], 16)
        except (KeyError, ValueError):
            continue
        rec = by_vma.get(vma)
        state = state_from_report(rec, row) if rec else TargetState(
            name=row.get("name", "?"), vma=vma, end=vma + row.get("rom_bytes", 0),
            rom_bytes=row.get("rom_bytes", 0), prefix=row.get("prefix"))
        state.family_members, state.family_verified = family_evidence(index, state)
        # `lift_scout.build()` stores the class name in `cls` and its sort
        # RANK in `band`. Reading `band` here would look up an integer in a
        # table keyed by class name, miss every row, and route the entire
        # queue to manual-preflight -- a plausible-looking output with no
        # dispatch behind it.
        state.band = row.get("cls")
        decision = route(state)
        rows.append({"name": state.name, "vma": f"{vma:#010x}",
                     "rom_bytes": state.rom_bytes, "band": state.band,
                     "decision": decision.strategy, "reason": decision.reason,
                     "declined": decision.declined})
    rows.sort(key=lambda r: (lift_scout.BANDS.get(r["band"], 50), -r["rom_bytes"]))
    return rows[:top]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--function", action="append", help="canonical body name or VMA; repeatable")
    ap.add_argument("--queue", type=Path, default=None,
                    help="route a lift_scout --json queue instead of named bodies")
    ap.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    ap.add_argument("--index", type=Path, default=DEFAULT_INDEX)
    ap.add_argument("--top", type=int, default=25)
    ap.add_argument("--json", type=Path)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()

    if args.self_test:
        return self_test()

    if not probe.ROM.exists():
        print("strategy_router: no reference ROM", file=sys.stderr)
        return 2
    if not args.report.exists():
        print(f"strategy_router: no corpus report at {args.report}; "
              "run `make matching-ready` first", file=sys.stderr)
        return 2

    rom = probe.ROM.read_bytes()
    report = json.loads(args.report.read_text())
    index = load_index(args.index)
    typed, reached = lift_scout.asm_entry_kinds()

    out_rows = []
    if args.queue:
        if not args.queue.exists():
            print(f"strategy_router: no queue at {args.queue}", file=sys.stderr)
            return 2
        out_rows = route_queue(json.loads(args.queue.read_text()), report, rom, index,
                               typed, reached, args.top)
        header = f"{'DECISION':22s} {'BAND':20s} {'ROM':>5s}  NAME"
        print(header)
        print("-" * len(header))
        for row in out_rows:
            print(f"{row['decision']:22s} {str(row['band']):20s} "
                  f"{row['rom_bytes']:5d}  {row['name']}")
    else:
        if not args.function:
            ap.error("give --function NAME or --queue PATH")
        for name in args.function:
            decision = route_named(name, report, rom, index, typed, reached)
            if decision is None:
                print(f"{name}: not in {args.report}")
                continue
            print(f"{name}: {decision.strategy}")
            print(f"  why: {decision.reason}")
            for item in decision.requires:
                print(f"  requires: {item}")
            for item in decision.declined:
                print(f"  not chosen: {item}")
            print()
            out_rows.append({"name": name, "decision": decision.strategy,
                             "reason": decision.reason})

    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps({
            "source": str(args.report), "rom_sha256": probe.digest(probe.ROM)
            if hasattr(probe, "digest") else None,
            "family_index": str(args.index) if index else None,
            "routes": out_rows,
        }, indent=2) + "\n", encoding="utf-8")
        print(f"JSON: {args.json}")
    return 0


def self_test() -> int:
    """Pin the routing table and every eligibility rule. No ROM/toolchain."""
    print("running strategy_router self-test...")
    passed = total = 0

    def check(label: str, condition: bool) -> None:
        nonlocal passed, total
        total += 1
        if condition:
            passed += 1
            print(f"  [PASS] {label}")
        else:
            print(f"  [FAIL] {label}", file=sys.stderr)

    def state(**kw) -> TargetState:
        base = dict(name="t", vma=0x08001000, end=0x08001020, rom_bytes=0x20,
                    instructions=8)
        base.update(kw)
        return TargetState(**base)

    # Every band lift_scout can emit must have a route, or the router would
    # silently fall through to manual for a body it could have dispatched.
    missing = sorted(set(lift_scout.BANDS) - set(BAND_ROUTES))
    check("every lift_scout band has a route", missing == [])
    stray = sorted(set(BAND_ROUTES) - set(lift_scout.BANDS))
    check("no route refers to a band lift_scout does not emit", stray == [])
    check("non-codegen bands never route to a codegen strategy",
          all(BAND_ROUTES[b].strategy == MANUAL for b in NON_CODEGEN_BANDS))

    # The faults that masquerade as codegen problems.
    check("span gap refuses synthesis",
          route(state(band="P0-span-gap")).strategy == MANUAL)
    check("unresolved reloc refuses synthesis",
          route(state(band="P9-unresolved-reloc")).strategy == MANUAL)
    check("untyped-bl is not dispatchable",
          route(state(band="P0-untyped-bl")).strategy == MANUAL)
    check("unresolved control flow refuses synthesis",
          route(state(band="P9-true-lift", flow_failures=("x",))).strategy == MANUAL)
    check("a wrong span outranks everything else",
          route(state(band="P0-span-gap", family_verified=5)).strategy == MANUAL)

    # Verified family beats the band: mining is cheaper than shaping.
    check("two verified siblings route to the miner",
          route(state(band="P5-register-alloc", family_verified=2,
                      family_members=4)).strategy == "recipe_miner.py")
    check("one verified sibling is not enough",
          route(state(band="P5-register-alloc", family_verified=1,
                      family_members=4)).strategy != "recipe_miner.py")
    check("an unverified family is declined with a reason",
          any("recipe_miner.py" in d for d in
              route(state(band="P5-register-alloc", family_verified=0,
                          family_members=3)).declined))

    # Synthesis eligibility is measured, not assumed.
    check("one conditional and no calls routes to synthesis",
          route(state(band="P9-true-lift", conditionals=1, calls=0)).strategy
          == "branch_synth.py")
    declined = route(state(band="P9-true-lift", conditionals=1, calls=3)).declined
    check("a call declines synthesis with a reason",
          any("calls" in d for d in declined))
    check("a straight-line body declines branch_synth with a reason",
          any("straight-line" in d for d in
              route(state(band="P9-true-lift", conditionals=0)).declined))
    check("an unclassified band goes to manual rather than guessing",
          route(state(band=None)).strategy == MANUAL)

    # Pool order is the documented solved case and must reach the microscope.
    check("pool order routes to the microscope",
          route(state(band="P4-pool-order")).strategy == "compiler_microscope.py")
    check("switch shape routes to synthesis",
          route(state(band="P6-switch-shape", conditionals=1,
                      calls=0)).strategy == "branch_synth.py")

    # A decision never names an unmeasured ABI fact as if it were known.
    argv = " ".join(command_for(route(state(band="P9-true-lift", conditionals=1)),
                               state(band="P9-true-lift", conditionals=1)))
    check("synthesis command leaves the return type to the user",
          "<ROM + caller evidence>" in argv)
    # The complement matters more: the router must never emit a concrete
    # contract value it did not measure, or a missing ABI decision becomes a
    # confident wrong one and the score reads as evidence.
    check("router never invents a concrete return type",
          not re.search(r"--return-type\s+(?!<)\S", argv))
    check("router never invents a concrete memory class",
          not re.search(r"--memory\s+(?!<)\S", argv))
    check("every band with a manual fallback states a real lever",
          _fallback_reason("P6-switch-shape", state()) !=
          _fallback_reason("P9-true-lift", state()))

    # Decision/TargetState round-trip through JSON without loss of the facts
    # a reviewer needs.
    payload = state(band="P4-pool-order", flow_failures=("a", "b")).to_json()
    check("state json is vma-shaped", payload["vma"] == "0x08001000")
    check("state json keeps flow failures as a list", payload["flow_failures"] == ["a", "b"])
    check("decision json is serializable", json.dumps(route(state()).to_json()) != "")

    # Router must not claim a match ratio it did not measure.
    check("absent probe record has no ratio", state(prefix=None).match_ratio is None)
    check("measured ratio is prefix over rom",
          state(prefix=10, rom_bytes=20).match_ratio == 0.5)

    print(f"{passed}/{total} passed")
    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())
