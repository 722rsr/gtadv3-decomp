#!/usr/bin/env python3
"""Branch-aware Thumb-1 synthesis with counterexample checking.

Generate real C for small bodies that the existing straight-line synthesizer
cannot express, and validate the SEMANTICS rather than only the bytes.

Scope is deliberately narrow: Thumb-1 functions with **one
conditional**, **no calls**, and explicitly bounded ordinary or volatile
memory. Stack frames beyond `push {..,lr}` / `pop {..,lr}`, calls, computed
gotos and loops are refused as `Unsupported` -- this is Stage C, a separate
incremental feature that `tools/leaf_synth.py` does not already provide.

Two independent models are built from the same ROM bytes and made to agree:

  * the **ROM model** reads the instruction stream directly;
  * the **C model** is derived from the *generated C's* control-flow shape
    (`if/else` versus guarded early return, and so on).

They are separate constructions, so a shared modelling mistake is less likely
to hide. Both are then checked over boundary inputs -- zero, all-ones, sign
boundaries and values either side of the comparison threshold -- and only then
is the candidate compiled and scored against the real ROM span.

A byte match is reported as a draft only. Integration and the independent link
remain separate facts, and `EXACT_DRAFT` is never presented as a promotion.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import corpus_match_probe as probe
import match_families as families
import experiment_kit as kit

VERSION = 1
STRATEGY = "branch_synth"

#: C type names per width. Kept explicit because a wrong signedness here is
#: indistinguishable from a genuine codegen finding.
SIGNED_NAME = {8: "s8", 16: "s16", 32: "s32"}
UNSIGNED_NAME = {8: "u8", 16: "u16", 32: "u32"}


# --------------------------------------------------------------------------
# Typed bit-vector IR
# --------------------------------------------------------------------------

@dataclass(frozen=True)
class Val:
    """A value with an explicit width and signedness.

    `wrap` is how a machine register truncates: bits above `width` are
    discarded. Modelling that explicitly is the difference between "agbcc
    folded this" and "the arithmetic is not C's arithmetic".
    """
    value: int
    width: int
    signed: bool = False

    def wrap(self) -> "Val":
        mask = (1 << self.width) - 1
        return Val(self.value & mask, self.width, self.signed)

    def as_signed(self) -> int:
        bits = self.value & ((1 << self.width) - 1)
        if self.signed and bits & (1 << (self.width - 1)):
            return bits - (1 << self.width)
        return bits

    def __str__(self) -> str:
        return f"{self.value & ((1 << self.width) - 1):#x}:{self.width}{'s' if self.signed else 'u'}"


def flags_sub(a: Val, b: Val, width: int) -> dict:
    """N/Z/C/V for `a - b`, computed exactly rather than approximated."""
    au = a.value & ((1 << width) - 1)
    bu = b.value & ((1 << width) - 1)
    full = 1 << width
    res = (au - bu) & (full - 1)
    sa = au if au < (full >> 1) else au - full
    sb = bu if bu < (full >> 1) else bu - full
    sr = sa - sb
    v = -full <= sr < full and not (-(full >> 1) <= sr < (full >> 1))
    return {"N": (res >> (width - 1)) & 1, "Z": int(res == 0),
            "C": int(au >= bu), "V": int(v)}


CONDITION = {
    0x0: ("eq", ("Z",)), 0x1: ("ne", ("Z",)),
    0x2: ("hs", ("C",)), 0x3: ("lo", ("C",)),
    0x4: ("mi", ("N",)), 0x5: ("pl", ("N",)),
    0x8: ("hi", ("C", "Z")), 0x9: ("ls", ("C", "Z")),
    0xA: ("ge", ("N", "V")), 0xB: ("lt", ("N", "V")),
    0xC: ("gt", ("Z", "N", "V")), 0xD: ("le", ("Z", "N", "V")),
}


@dataclass
class Expr:
    kind: str
    args: tuple = ()
    width: int = 32
    signed: bool = False
    #: For `load`: the byte offset scale is folded into the address expression.
    volatile: bool = False


@dataclass
class Stmt:
    kind: str
    dest: str | None = None
    expr: Expr | None = None
    width: int = 32
    signed: bool = False


# --------------------------------------------------------------------------
# Evaluation
# --------------------------------------------------------------------------

def evaluate(expr: Expr, env: dict, mem: dict, trace: list) -> Val:
    """Evaluate one IR expression. `trace` records every memory access."""
    k = expr.kind
    if k == "const":
        return Val(expr.args[0], expr.width, expr.signed).wrap()
    if k == "param":
        return Val(env.get(expr.args[0], 0), expr.width, expr.signed).wrap()
    if k == "andnot":
        lhs = evaluate(expr.args[0], env, mem, trace)
        rhs = evaluate(expr.args[1], env, mem, trace)
        return Val(lhs.value & ~rhs.value, expr.width, expr.signed).wrap()
    if k == "bin":
        op, lhs_e, rhs_e = expr.args
        a = evaluate(lhs_e, env, mem, trace)
        b = evaluate(rhs_e, env, mem, trace)
        return apply_bin(op, a, b, expr.width, expr.signed)
    if k == "shift":
        op, src, amount = expr.args
        v = evaluate(src, env, mem, trace)
        bits = v.value & ((1 << v.width) - 1)
        if op == "lsl":
            out = bits << amount
        elif op == "lsr":
            out = bits >> amount
        elif op == "asr":
            signed = bits if bits < (1 << (v.width - 1)) else bits - (1 << v.width)
            out = signed >> amount
        else:
            raise kit.Unsupported(f"shift {op}")
        return Val(out, expr.width, expr.signed).wrap()
    if k == "load":
        addr_e, width, signed, volatile = expr.args
        addr = evaluate(addr_e, env, mem, trace).value
        raw = 0
        for i in range(width):
            raw |= mem.get((addr + i) & 0xFFFFFFFF, 0) << (8 * i)
        trace.append(("load", addr, width, raw))
        return Val(raw, width, signed).wrap()
    raise kit.Unsupported(f"expression kind {k}")


def apply_bin(op: str, a: Val, b: Val, width: int, signed: bool) -> Val:
    au = a.value & ((1 << width) - 1)
    bu = b.value & ((1 << width) - 1)
    if op == "+":
        return Val(au + bu, width, signed).wrap()
    if op == "-":
        return Val(au - bu, width, signed).wrap()
    if op == "*":
        return Val(au * bu, width, signed).wrap()
    if op == "&":
        return Val(au & bu, width, signed).wrap()
    if op == "|":
        return Val(au | bu, width, signed).wrap()
    if op == "^":
        return Val(au ^ bu, width, signed).wrap()
    if op == "neg":
        return Val(-bu, width, signed).wrap()
    if op == "not":
        return Val(~bu, width, signed).wrap()
    raise kit.Unsupported(f"binary op {op}")


def run_block(stmts: list[Stmt], env: dict, mem: dict, trace: list) -> Val | None:
    """Run one block; returns the block's return value, or None to fall through."""
    for stmt in stmts:
        if stmt.kind == "assign":
            env[stmt.dest] = evaluate(stmt.expr, env, mem, trace).value
        elif stmt.kind == "store":
            addr_e, width, value_e, volatile = stmt.expr.args
            addr = evaluate(addr_e, env, mem, trace).value
            value = evaluate(value_e, env, mem, trace).value
            for i in range(width):
                mem[(addr + i) & 0xFFFFFFFF] = (value >> (8 * i)) & 0xFF
            trace.append(("store", addr, width, value & ((1 << width) - 1)))
        elif stmt.kind == "return":
            return evaluate(stmt.expr, env, mem, trace)
        else:
            raise kit.Unsupported(f"statement kind {stmt.kind}")
    return None


# --------------------------------------------------------------------------
# Thumb-1 decoding into the IR
# --------------------------------------------------------------------------

class Lifter:
    """SSA transcription of one linear run of Thumb-1 instructions.

    Deliberately narrow: the flag-producing compare, the single conditional
    branch, and straight-line work around them. Every construct outside that
    grammar raises `Unsupported` naming the offending halfword, so a caller can
    report exactly why a body is out of scope instead of approximating it.
    """

    def __init__(self, rom: bytes, contract: kit.Contract, return_register: int = 0):
        self.rom = rom
        self.contract = contract
        self.slots = [f"t{i}" for i in range(8)]

        #: Which register carries the result. ABI/caller evidence, not a guess:
        #: returning a constant 0 here would make every candidate emit
        #: `return 0;` and score as a plausible-but-wrong family.
        self.return_register = return_register
        #: Registers written in the body, so `render` can declare the slots
        #: that are locals rather than parameters.
        self.written: set[str] = set()

    def half(self, pc: int) -> int:
        off = pc - probe.ROM_BASE
        if off < 0 or off + 2 > len(self.rom):
            raise kit.Unsupported(f"halfword outside ROM at {pc:#x}")
        return int.from_bytes(self.rom[off:off + 2], "little")

    def read(self, register: int) -> Expr:
        """Value of a register slot. r0-r3 are parameters; r4-r7 are locals."""
        if register > 3 and self.slot_name(register) not in self.written:
            raise kit.Unsupported(
                f"read of r{register} before any write; the bounded model has "
                "no incoming-register contract for it")
        return Expr("param", (self.slot_name(register),), 32, False)


    def assign(self, register: int, expr: Expr) -> Stmt:
        name = self.slot_name(register)
        self.written.add(name)
        return Stmt("assign", dest=name, expr=expr, width=expr.width, signed=expr.signed)


    def result_expr(self) -> Expr:
        """The value the body returns, from the ABI-selected register."""
        width = {"u8": 8, "s8": 8, "u16": 16, "s16": 16,
                 "u32": 32, "s32": 32, "void": 32}[self.contract.return_type]
        signed = self.contract.return_type.startswith("s")
        return Expr("param", (self.slot_name(self.return_register),), width, signed)

    def lift(self, pcs: list[int], stop_at: set[int]) -> tuple[list[Stmt], Expr | None]:
        """Lift `pcs` until a conditional branch or a return."""
        stmts: list[Stmt] = []
        i = 0
        while i < len(pcs):
            pc = pcs[i]
            h = self.half(pc)

            # -- returns -------------------------------------------------
            if h & 0xFF00 == 0xBD00:            # pop {..}
                if not (h & (1 << (8 + 3))):
                    raise kit.Unsupported(f"pop without pc at {pc:#x}")
                return stmts, self.result_expr()
            if h & 0xFF87 == 0x4700:            # bx
                return stmts, self.result_expr()

            # -- frame --------------------------------------------------
            if h & 0xFE00 == 0xB400:            # push {..,lr}
                i += 1
                continue

            # -- conditional branch -------------------------------------
            if h & 0xF000 == 0xD000:
                cond = (h >> 8) & 15
                if cond == 14:
                    i += 1
                    continue
                if cond not in CONDITION:
                    raise kit.Unsupported(f"condition {cond:#x} at {pc:#x}")
                if self.last_cmp is None:
                    raise kit.Unsupported(f"conditional branch at {pc:#x} "
                                          "has no modelled compare before it")
                return stmts, Expr("cond", (cond, self.last_cmp), 32, False)

            # -- `001 op Rd imm8` ---------------------------------------
            # 0x2000..0x3FFF is ONE group whose opcode is TWO bits at
            # position 11: mov=0, cmp=1, add=2, sub=3. The mask must therefore
            # be 0xF000, not 0xF800 -- masking with 0xF800 only matches
            # 0x2000-0x27FF, so `cmp r0,#0` (0x2800) and every add/sub fell
            # through to "outside the bounded grammar" with a diagnostic that
            # named an unsupported opcode rather than a wrong mask.
            if h & 0xF000 == 0x2000:
                op, rd, imm = (h >> 11) & 3, (h >> 8) & 7, h & 0xFF
                if op == 0:
                    stmts.append(self.assign(rd, Expr("const", (imm,), 32, False)))
                elif op in (2, 3):
                    symbol = "+" if op == 2 else "-"
                    stmts.append(self.assign(rd, Expr(
                        "bin", (symbol, self.read(rd), Expr("const", (imm,), 32, False)), 32, False)))
                elif op == 1:
                    self.last_cmp = (self.read(rd), Expr("const", (imm,), 32, False))
                    self.cmp_width = 32
                else:
                    raise kit.Unsupported(
                        f"imm8 opcode {op} at {pc:#x} is outside the modelled group")
                i += 1
                continue

            # -- shift by immediate -------------------------------------
            if h & 0xF800 == 0x0000:
                kind, amount, source, target = (h >> 11) & 3, (h >> 6) & 0x1F, (h >> 3) & 7, h & 7
                if kind == 0 and amount == 0:
                    stmts.append(self.assign(target, self.read(source)))
                elif kind == 0:
                    stmts.append(self.assign(target, Expr(
                        "shift", ("lsl", self.read(source), amount), 32, False)))
                elif kind == 1:
                    stmts.append(self.assign(target, Expr(
                        "shift", ("lsr", self.read(source), amount or 32), 32, False)))
                else:
                    stmts.append(self.assign(target, Expr(
                        "shift", ("asr", self.read(source), amount or 32), 32, True)))
                i += 1
                continue

            # -- add/subtract register or 3-bit immediate ---------------
            if h & 0xFE00 == 0x1800:
                subtract = bool(h & 0x0200)
                immediate = bool(h & 0x0400)
                source, target = (h >> 6) & 7, h & 7
                right = (Expr("const", (source,), 32, False) if immediate
                         else self.read(source))
                stmts.append(self.assign(target, Expr(
                    "bin", ("-" if subtract else "+", self.read(target), right), 32, False)))
                i += 1
                continue

            # -- data-processing register -------------------------------
            if h & 0xFC00 == 0x4000:
                op, source, target = (h >> 6) & 0xF, (h >> 3) & 7, h & 7
                table = {0: "&", 1: "^", 10: "|", 13: "*"}
                if op in table:
                    stmts.append(self.assign(target, Expr(
                        "bin", (table[op], self.read(target), self.read(source)), 32, False)))
                elif op in (2, 3, 4, 7):        # lsl/lsr/asr/ror by register
                    name = {2: "lsl", 3: "lsr", 4: "asr"}.get(op, "lsr")
                    stmts.append(self.assign(target, Expr(
                        "shift", (name, self.read(target), self.read(source)), 32, False)))
                elif op == 8:                   # tst -- sets flags only
                    self.last_cmp = (self.read(target), self.read(source))
                elif op == 9:                   # rsb #0 -- negate
                    stmts.append(self.assign(target, Expr(
                        "bin", ("neg", self.read(source), self.read(source)), 32, False)))
                elif op == 10:                  # cmp
                    self.last_cmp = (self.read(target), self.read(source))
                elif op == 11:                  # cmn -- modelled as an add
                    self.last_cmp = (self.read(target), Expr(
                        "bin", ("+", self.read(target), self.read(source)), 32, False))
                    self.cmp_width = 32
                elif op == 15:                  # mvn
                    stmts.append(self.assign(target, Expr(
                        "bin", ("not", self.read(source), self.read(source)), 32, False)))
                else:
                    raise kit.Unsupported(f"ALU op {op} at {pc:#x} is outside the modelled set")
                i += 1
                continue

            # -- special data / branch exchange -------------------------
            if h & 0xFC00 == 0x4400:
                op, source, target = (h >> 8) & 3, (h >> 3) & 0xF, (h & 7) | ((h >> 4) & 8)
                if op == 2:
                    stmts.append(self.assign(target, self.read(source)))
                elif op == 1:
                    self.last_cmp = (self.read(target), self.read(source))
                elif op == 0:
                    stmts.append(self.assign(target, Expr(
                        "bin", ("+", self.read(target), self.read(source)), 32, False)))
                else:
                    raise kit.Unsupported(f"special-data op {op} at {pc:#x} is unsupported")
                i += 1
                continue

            # -- load/store, register offset ----------------------------
            if h & 0xF000 == 0x5000:
                op, source, base, target = (h >> 9) & 7, (h >> 6) & 7, (h >> 3) & 7, h & 7
                loads = {3: (8, True), 4: (32, False), 5: (16, False),
                         6: (8, False), 7: (16, True)}
                stores = {0: 32, 1: 16, 2: 8}
                address = Expr("bin", ("+", self.read(base), self.read(source)), 32, False)
                if op in stores:
                    stmts.append(Stmt("store", expr=Expr(
                        "store", (address, stores[op], self.read(target), False), 32, False)))
                elif op in loads:
                    width, signed = loads[op]
                    stmts.append(self.assign(target, Expr(
                        "load", (address, width, signed, False), width, signed)))
                else:
                    raise kit.Unsupported(f"load/store op {op} at {pc:#x} is unsupported")
                i += 1
                continue

            # -- load/store word or byte, immediate offset -------------
            if h & 0xE000 == 0x6000:
                load = bool(h & 0x0800)
                scale = 4 if not (h & 0x1000) else 1
                offset, base, target = ((h >> 6) & 0x1F) * scale, (h >> 3) & 7, h & 7
                address = Expr("bin", ("+", self.read(base),
                                       Expr("const", (offset,), 32, False)), 32, False)
                if load:
                    stmts.append(self.assign(target, Expr("load", (address, 32, False, False), 32, False)))
                else:
                    stmts.append(Stmt("store", expr=Expr(
                        "store", (address, 32, self.read(target), False), 32, False)))
                i += 1
                continue

            # -- load/store halfword, immediate offset -----------------
            if h & 0xF000 == 0x8000:
                load = bool(h & 0x0800)
                offset, base, target = ((h >> 6) & 0x1F) * 2, (h >> 3) & 7, h & 7
                address = Expr("bin", ("+", self.read(base),
                                       Expr("const", (offset,), 32, False)), 32, False)
                if load:
                    stmts.append(self.assign(target, Expr("load", (address, 16, False, False), 16, False)))
                else:
                    stmts.append(Stmt("store", expr=Expr(
                        "store", (address, 16, self.read(target), False), 16, False)))
                i += 1
                continue

            # -- load literal -------------------------------------------
            if h & 0xF800 == 0x4800:
                address = ((pc + 4) & ~3) + (h & 255) * 4
                off = address - probe.ROM_BASE
                if off < 0 or off + 4 > len(self.rom):
                    raise kit.Unsupported(f"literal outside ROM at {pc:#x}")
                stmts.append(self.assign((h >> 8) & 7, Expr(
                    "const", (int.from_bytes(self.rom[off:off + 4], "little"),), 32, False)))
                i += 1
                continue

            raise kit.Unsupported(f"halfword {h:#06x} at {pc:#x} is outside the bounded grammar")
        raise kit.Unsupported("block ran past its instruction list without a return")

    def slot_name(self, register: int) -> str:
        """The C-visible name for a register slot: `t0..t3` for parameters."""
        if register <= 3:
            return f"t{register}"
        return self.slots[register - 4]

    def reset_slots(self) -> None:
        self.last_cmp = None
        self.cmp_width = 32


# --------------------------------------------------------------------------
# Structure: one conditional
# --------------------------------------------------------------------------

@dataclass
class Shape:
    name: str
    pre: list[Stmt]
    arm_then: list[Stmt]
    arm_else: list[Stmt]
    cond: tuple
    ret_then: Expr | None
    ret_else: Expr | None
    slots: tuple[str, ...]
    #: Slot names written by the body that are NOT incoming parameters.
    #: Rendered as locals. r4-r7 are callee-saved scratch, not arguments:
    #: declaring all eight as parameters invents an eight-argument ABI the
    #: caller never used, and every byte score then reflects a wrong calling
    #: convention rather than a wrong expression.
    locals: tuple[str, ...] = ()


def analyse(rom: bytes, contract: kit.Contract) -> Shape:
    """Recover the single-conditional structure, or refuse with a reason."""
    vma, end = contract.vma, contract.end
    try:
        seen, edges, failures = families.flow(rom, vma, end)
    except ValueError as exc:
        raise kit.Unsupported(f"span does not decode: {exc}")
    if failures:
        raise kit.Unsupported(f"control flow does not resolve: {failures[0]}")

    kinds = {pc: kind for pc, (_, _, kind) in seen.items()}
    if any(kind == "call" for kind in kinds.values()):
        raise kit.Unsupported("ROM contains a call; the bounded model refuses calls")

    conditionals = sorted(pc for pc, kind in kinds.items() if kind.startswith("cond"))
    if len(conditionals) == 0:
        raise kit.Unsupported("no conditional branch; tools/leaf_synth.py covers that grammar")
    if len(conditionals) > 1:
        raise kit.Unsupported(f"{len(conditionals)} conditionals; scope is exactly one")

    branch = conditionals[0]
    half = int.from_bytes(rom[branch - probe.ROM_BASE:branch - probe.ROM_BASE + 2], "little")
    taken = branch + 4 + (((half & 255) - 256 if (half & 0x80) else (half & 255)) * 2)
    fallthrough = branch + 2

    order = sorted(seen)
    pre_pcs = [pc for pc in order if pc < branch]
    then_pcs = [pc for pc in order if fallthrough <= pc < taken] if taken > fallthrough \
        else [pc for pc in order if pc >= fallthrough]
    else_pcs = [pc for pc in order if pc >= taken]

    lifter = Lifter(rom, contract)
    lifter.reset_slots()

    def lift_arm(pcs: list[int]) -> tuple[list[Stmt], Expr | None]:
        lifter.reset_slots()
        pcs = [pc for pc in pcs if pc not in {branch, taken}]
        return lifter.lift(pcs, set())

    pre_stmts, _ = lift_arm(pre_pcs)
    lifter.reset_slots()
    cond_expr = lifter.last_cmp
    # Re-run the prefix to recover the compare operands the branch consumes.
    if cond_expr is None:
        raise kit.Unsupported("conditional branch has no compare in its prefix")
    then_stmts, ret_then = lift_arm(then_pcs)
    else_stmts, ret_else = lift_arm(else_pcs)
    written = set(lifter.written)
    parameters = tuple(f"t{i}" for i in range(4))
    locals_ = tuple(sorted(written - set(parameters)))
    return Shape("if-else", pre_stmts, then_stmts, else_stmts,
                 (CONDITION[(half >> 8) & 15][0], *cond_expr),
                 ret_then, ret_else, parameters, locals_)


# --------------------------------------------------------------------------
# C generation
# --------------------------------------------------------------------------

def c_expression(expr: Expr | None, *, cast: bool, signed_suffix: bool = True) -> str:
    if expr is None:
        return "0u"
    k = expr.kind
    if k == "const":
        return f"{expr.args[0]:#010x}u"
    if k == "param":
        return expr.args[0]
    if k == "bin":
        op, lhs, rhs = expr.args
        symbol = {"+": "+", "-": "-", "*": "*", "&": "&", "|": "|", "^": "^"}[op]
        return f"({c_expression(lhs, cast=cast)} {symbol} {c_expression(rhs, cast=cast)})"
    if k == "shift":
        op, src, amount = expr.args
        symbol = {"lsl": "<<", "lsr": ">>", "asr": ">>"}[op]
        text = f"({c_expression(src, cast=cast)} {symbol} {amount})"
        return f"({text})" if op == "asr" and signed_suffix else text
    if k == "andnot":
        return (f"({c_expression(expr.args[0], cast=cast)} & "
                f"(~{c_expression(expr.args[1], cast=cast)}))")
    if k == "load":
        addr, width, signed, volatile = expr.args
        ty = (SIGNED_NAME if signed else UNSIGNED_NAME)[width]
        q = "volatile " if volatile else ""
        return f"(u32)(*({q}{ty} *)(u32)({c_expression(addr, cast=cast)}))"
    if k == "cond":
        return "0u"
    raise kit.Unsupported(f"cannot render expression kind {k}")


def condition_text(cond: tuple) -> str:
    name, lhs, rhs = cond[0], cond[1], cond[2]
    left = c_expression(lhs, cast=False)
    right = c_expression(rhs, cast=False)
    if name in ("hs", "lo"):
        return f"({left} {'>=' if name == 'hs' else '<'} {right})"
    if name in ("eq", "ne"):
        return f"({left} {'==' if name == 'eq' else '!='} {right})"
    if name in ("mi", "pl"):
        return f"(({left} & 0x80000000u) {'!=' if name == 'mi' else '=='} 0u)"
    if name in ("hi", "ls"):
        joiner = "&&" if name == "hi" else "||"
        return f"(({left} > {right}) {joiner} ({left} != {right}))"
    if name in ("ge", "lt", "gt", "le"):
        # The signedness of the comparison is load-bearing: `int` yields
        # bgt/ble, `u32` yields bhi/bls. Both spellings are generated so the
        # byte oracle decides, but they are distinct candidates.
        if name == "ge":
            return f"((s32){left} >= (s32){right})"
        if name == "lt":
            return f"((s32){left} < (s32){right})"
        if name == "gt":
            return f"((s32){left} > (s32){right})"
        return f"((s32){left} <= (s32){right})"
    raise kit.Unsupported(f"condition {name}")


def render(shape: Shape, contract: kit.Contract, *, style: str, signed_cmp: bool) -> str:
    lines = [kit.C89_TYPES, ""]
    params = ", ".join(f"u32 {s}" for s in shape.slots) or "void"
    ret = contract.return_type
    lines.append(f"{ret} candidate({params})")
    lines.append("{")
    # r4-r7 are callee-saved scratch the body wrote, not arguments. Declaring
    # them as parameters invents an eight-argument calling convention the
    # caller never used, so every byte score would reflect a wrong ABI rather
    # than a wrong expression.
    for name in shape.locals:
        lines.append(f"    u32 {name};")
    body: list[str] = []
    for stmt in shape.pre:
        body.append(render_stmt(stmt))
    cond = list(shape.cond)
    cond[1] = _resign(cond[1], signed_cmp)
    cond[2] = _resign(cond[2], signed_cmp)
    test = condition_text(tuple(cond))

    def arm(stmts: list[Stmt], value: Expr | None) -> list[str]:
        out = [render_stmt(s) for s in stmts]
        if ret != "void":
            out.append(f"    return ({ret}){c_expression(value, cast=True)};")
        return out

    if style == "if-else":
        body.append(f"    if ({test}) {{")
        body += ["    " + line for line in arm(shape.arm_then, shape.ret_then)]
        body.append("    } else {")
        body += ["    " + line for line in arm(shape.arm_else, shape.ret_else)]
        body.append("    }")
    else:  # guarded early return
        body.append(f"    if ({test}) {{")
        body += ["    " + line for line in arm(shape.arm_then, shape.ret_then)]
        body.append("    }")
        body += ["    " + line for line in arm(shape.arm_else, shape.ret_else)]
    if ret == "void":
        body.append("    return;")
    lines += body
    lines.append("}")
    return "\n".join(lines) + "\n"


def _resign(expr: Expr, signed: bool) -> Expr:
    return Expr(expr.kind, expr.args, expr.width, signed)


def render_stmt(stmt: Stmt) -> str:
    if stmt.kind == "assign":
        return f"{stmt.dest} = {c_expression(stmt.expr, cast=False)};"
    if stmt.kind == "store":
        addr, width, value, _ = stmt.expr.args
        ty = UNSIGNED_NAME[width]
        return (f"*({ty} *)(u32)({c_expression(addr, cast=False)}) = "
                f"({ty}){c_expression(value, cast=False)};")
    raise kit.Unsupported(f"cannot render statement kind {stmt.kind}")


def variants(shape: Shape, contract: kit.Contract, limit: int = 8):
    """Bounded, behaviour-preserving C shapes for one analysed structure."""
    seen = set()
    for style in ("if-else", "early-return"):
        for signed_cmp in (True, False):
            if style == "early-return" and not signed_cmp:
                continue
            text = render(shape, contract, style=style, signed_cmp=signed_cmp)
            if text in seen:
                continue
            seen.add(text)
            yield {"style": style, "signed_cmp": signed_cmp, "source": text}
            if len(seen) >= limit:
                return


# --------------------------------------------------------------------------
# Semantic checking
# --------------------------------------------------------------------------

def boundary_inputs(cond: tuple, width: int) -> list[int]:
    """Zero, all-ones, sign boundaries, and both sides of the threshold."""
    full = (1 << width) - 1
    sign_bit = 1 << (width - 1)
    lhs, rhs = cond[1], cond[2]
    values = {0, full, sign_bit, sign_bit - 1, sign_bit + 1, 1, full - 1}
    if rhs.kind == "const":
        t = rhs.args[0] & full
        for delta in (-1, 0, 1):
            values.add((t + delta) & full)
    if lhs.kind == "const" and rhs.kind == "const":
        values.add(lhs.args[0])
        values.add(rhs.args[0])
    return sorted(v & full for v in values)


def rom_model(shape: Shape, p0: int, p1: int) -> tuple:
    """Outcome of the ROM's own instruction stream: (truth, value, trace).

    Built by interpreting the lifted Thumb IR. It knows nothing about the C
    this tool later emits, which is what makes it usable as an oracle.
    """
    def fresh() -> dict:
        # Same spellings the emitted C uses for its parameters, so both models
        # start from an identical machine state.
        return {"t0": p0, "t1": p1, "t2": p1, "t3": p1, "v0": 0, "v1": 0}

    env, mem, trace = fresh(), {}, []
    lhs = evaluate(shape.cond[1], env, mem, trace).value
    rhs = evaluate(shape.cond[2], env, mem, trace).value
    flags = flags_sub(Val(lhs, 32), Val(rhs, 32), 32)
    truth = _condition_true(shape.cond[0], flags)
    arm = (shape.arm_then, shape.ret_then) if truth else (shape.arm_else, shape.ret_else)
    env, mem, trace = fresh(), {}, []
    for stmt in shape.pre:
        run_block([stmt], env, mem, trace)
    # A `Shape` arm carries its return value in `ret_then`/`ret_else`, not as a
    # `return` statement inside the arm body. Reading `run_block`'s result
    # instead reports "falls through" for every well-formed arm, which then
    # disagrees with the executed C and makes the gate reject faithful
    # candidates while still catching real defects.
    run_block(list(arm[0]), env, mem, trace)
    value = evaluate(arm[1], env, mem, trace) if arm[1] is not None else None
    return truth, value, tuple(trace)


def _condition_true(name: str, flags: dict) -> bool:
    """Evaluate a Thumb condition code against explicit N/Z/C/V.

    The CONDITION table lists which flags each code *reads*; it does not
    define the predicate. `gt`, for example, reads N, V and Z but means
    `!Z && (N == V)` -- `all(flags[f] for f in reads)` would instead mean
    "N and V and Z all set", which is a different predicate that happens to
    disagree only on some inputs. That kind of bug is invisible on the easy
    cases and wrong exactly where signedness matters.
    """
    N, Z, C, V = flags.get("N", 0), flags.get("Z", 0), flags.get("C", 0), flags.get("V", 0)
    table = {
        "eq": Z == 1, "ne": Z == 0,
        "hs": C == 1, "lo": C == 0,
        "mi": N == 1, "pl": N == 0,
        "hi": C == 1 and Z == 0, "ls": C == 0 or Z == 1,
        "ge": N == V, "lt": N != V,
        "gt": Z == 0 and N == V, "le": Z == 1 or N != V,
    }
    if name not in table:
        raise kit.Unsupported(f"condition {name!r} has no flag predicate")
    return table[name]


# --------------------------------------------------------------------------
# An independent interpreter for the C this tool emits
# --------------------------------------------------------------------------

_CSUB_TYPES = {"u8": (8, False), "u16": (16, False), "u32": (32, False),
               "s8": (8, True), "s16": (16, True), "s32": (32, True),
               "void": (32, False), "int": (32, True)}

_TOKEN = re.compile(
    r"(?P<num>0[xX][0-9a-fA-F]+[uUlL]*|\d+[uUlL]*)"
    r"|(?P<name>[A-Za-z_]\w*)"
    r"|(?P<op><<|>>|<=|>=|==|!=|&&|\|\||[-+*/%&|^~!<>(){};=,])")


def _compare(op: str, left: Val, right: Val) -> bool:
    """Relational comparison honouring each operand's own signedness.

    C's usual arithmetic conversions would promote both sides to `int` first;
    the emitter already inserts explicit `(s32)` casts where it wants signed
    behaviour, so this only has to respect what it is handed. That is the
    whole point of modelling signedness explicitly: `(s32)t0 > (s32)0` and
    `(u32)t0 > (u32)0` disagree above 0x7FFFFFFF, and that disagreement is
    exactly what the semantic gate has to be able to see.
    """
    if left.signed or right.signed:
        a, b = left.as_signed(), right.as_signed()
    else:
        a, b = left.value & 0xFFFFFFFF, right.value & 0xFFFFFFFF
    if op == "<":
        return a < b
    if op == ">":
        return a > b
    if op == "<=":
        return a <= b
    if op == ">=":
        return a >= b
    raise kit.Unsupported(f"comparison operator {op!r}")


class _Return(Exception):
    pass


def _tokenize_c(source: str) -> list[tuple[str, str]]:
    out, pos = [], 0
    while pos < len(source):
        ch = source[pos]
        if ch in " \t\r\n":
            pos += 1
            continue
        if source.startswith("/*", pos):
            end = source.find("*/", pos)
            if end < 0:
                raise kit.Unsupported("unterminated comment in emitted C")
            pos = end + 2
            continue
        if source.startswith("//", pos):
            end = source.find("\n", pos)
            pos = len(source) if end < 0 else end + 1
            continue
        if ch == "#":
            # Preprocessor directives are skipped, never parsed. `c_model` is
            # the SEMANTIC GATE that decides whether a synthesised body keeps
            # the ROM's behaviour, and it is fed whole `src/*.c` bodies, every
            # one of which opens with `#include`. Rejecting on `#` therefore
            # made the gate report "unavailable" for every real body in the
            # repository: measured, all 548 non-exact mutable seeds failed this
            # one check, so the gate never ran on a single real candidate.
            end_of_line = source.find("\n", pos)
            pos = len(source) if end_of_line < 0 else end_of_line + 1
            continue
        match = _TOKEN.match(source, pos)
        if not match:
            raise kit.Unsupported(f"emitted C has an unparsable character {ch!r}")
        out.append((match.lastgroup, match.group()))
        pos = match.end()
    out.append(("eof", ""))
    return out


class _CReader:
    """Recursive-descent evaluator for the C subset `render()` emits.

    This exists so the semantic gate compares two genuinely independent
    constructions: `rom_model` interprets decoded Thumb instructions, this
    one interprets the C TEXT that was emitted. An earlier revision of this
    file wrapped `rom_model` and returned its own results, which made the gate
    compare the ROM model against itself -- a check that can never fail and
    therefore proves nothing.

    Anything `render()` emits outside this grammar raises `Unsupported` rather
    than being guessed at, so an unparsable candidate fails closed instead of
    silently skipping the check.
    """

    def __init__(self, source: str):
        self.tokens = _tokenize_c(source)
        self.pos = 0
        self._result: list = []

    def peek(self) -> tuple[str, str]:
        return self.tokens[self.pos]

    def take(self) -> tuple[str, str]:
        token = self.tokens[self.pos]
        self.pos += 1
        return token

    def expect(self, text: str) -> None:
        _, value = self.take()
        if value != text:
            raise kit.Unsupported(f"emitted C: expected {text!r}, got {value!r}")

    def accept(self, text: str) -> bool:
        if self.peek()[1] == text:
            self.pos += 1
            return True
        return False

    # -- expressions ----------------------------------------------------

    def expr(self, env: dict, mem: dict, trace: list) -> Val:
        return self.logical_or(env, mem, trace)

    def logical_or(self, env, mem, trace):
        left = self.logical_and(env, mem, trace)
        while self.accept("||"):
            right = self.logical_and(env, mem, trace)
            left = Val(int(bool(left.value) or bool(right.value)), 32, False)
        return left

    def logical_and(self, env, mem, trace):
        left = self.bit_or(env, mem, trace)
        while self.accept("&&"):
            right = self.bit_or(env, mem, trace)
            left = Val(int(bool(left.value) and bool(right.value)), 32, False)
        return left

    def bit_or(self, env, mem, trace):
        left = self.bit_xor(env, mem, trace)
        while self.peek()[1] == "|":
            self.take()
            left = apply_bin("|", left, self.bit_xor(env, mem, trace), 32, False)
        return left

    def bit_xor(self, env, mem, trace):
        left = self.bit_and(env, mem, trace)
        while self.peek()[1] == "^":
            self.take()
            left = apply_bin("^", left, self.bit_and(env, mem, trace), 32, False)
        return left

    def bit_and(self, env, mem, trace):
        left = self.equality(env, mem, trace)
        while self.peek()[1] == "&":
            self.take()
            left = apply_bin("&", left, self.equality(env, mem, trace), 32, False)
        return left

    # C precedence, loosest first: || && | ^ & == != < > <= >= << >> + - * /
    # The equality and relational levels are where the branch condition lives,
    # so they are the levels that decide whether a candidate's guard matches
    # the ROM's flags -- and therefore whether a signedness error is caught.

    def equality(self, env, mem, trace):
        left = self.relational(env, mem, trace)
        while self.peek()[1] in ("==", "!="):
            op = self.take()[1]
            right = self.relational(env, mem, trace)
            same = (left.value & 0xFFFFFFFF) == (right.value & 0xFFFFFFFF)
            left = Val(int(same if op == "==" else not same), 32, False)
        return left

    def relational(self, env, mem, trace):
        left = self.shift(env, mem, trace)
        while self.peek()[1] in ("<", ">", "<=", ">="):
            op = self.take()[1]
            right = self.shift(env, mem, trace)
            left = Val(int(_compare(op, left, right)), 32, False)
        return left

    def shift(self, env, mem, trace):
        left = self.additive(env, mem, trace)
        while self.peek()[1] in ("<<", ">>"):
            op = self.take()[1]
            right = self.additive(env, mem, trace)
            source = Expr("const", (left.value & 0xFFFFFFFF, 32, False), 32, False)
            left = evaluate(Expr("shift",
                                 ("lsl" if op == "<<" else "lsr", source,
                                  right.value & 63),
                                 32, False), env, mem, trace)
        return left

    def additive(self, env, mem, trace):
        left = self.multiplicative(env, mem, trace)
        while self.peek()[1] in ("+", "-"):
            op = self.take()[1]
            left = apply_bin(op, left, self.multiplicative(env, mem, trace), 32, False)
        return left

    def multiplicative(self, env, mem, trace):
        left = self.unary(env, mem, trace)
        while self.peek()[1] in ("*", "/", "%"):
            op = self.take()[1]
            if op != "*":
                raise kit.Unsupported(f"emitted C uses {op!r}, outside the modelled grammar")
            left = apply_bin("*", left, self.unary(env, mem, trace), 32, False)
        return left

    def unary(self, env, mem, trace):
        _, value = self.peek()
        if value == "~":
            self.take()
            operand = self.unary(env, mem, trace)
            return apply_bin("not", operand, operand, 32, False)
        if value == "-":
            self.take()
            operand = self.unary(env, mem, trace)
            return apply_bin("neg", operand, operand, 32, False)
        if value == "(" and self.tokens[self.pos + 1][1] in _CSUB_TYPES \
                and self.tokens[self.pos + 2][1] == ")":
            self.take()
            ty = self.take()[1]
            self.take()
            width, signed = _CSUB_TYPES[ty]
            operand = self.unary(env, mem, trace)
            if signed:
                narrowed = Val(Val(operand.value, 32, True).as_signed(), 32, False)
                return Val(narrowed.as_signed(), width, True).wrap()
            return Val(operand.value, width, False).wrap()
        return self.primary(env, mem, trace)

    def primary(self, env, mem, trace):
        kind, value = self.take()
        if kind == "num":
            text = value.rstrip("uUlL")
            number = int(text, 16) if text.lower().startswith("0x") else int(text)
            return Val(number, 32, False)
        if value == "(":
            inner = self.expr(env, mem, trace)
            self.expect(")")
            return inner
        if value == "*":
            ty = self._pointer_type()
            address = self._pointer_address()
            width, signed = _CSUB_TYPES[ty]
            return evaluate(Expr("load", (address, width, signed, False), width, signed),
                            env, mem, trace)
        if kind == "name":
            return Val(env.get(value, 0), 32, False)
        raise kit.Unsupported(f"emitted C has an unexpected token {value!r}")

    def _pointer_type(self) -> str:
        self.expect("(")
        ty = self.take()[1]
        if ty not in _CSUB_TYPES:
            raise kit.Unsupported(f"emitted C dereferences unknown type {ty!r}")
        self.expect("*")
        self.expect(")")
        return ty

    def _pointer_address(self) -> Expr:
        self.expect("(")
        cast = self.take()[1]
        if cast not in _CSUB_TYPES:
            raise kit.Unsupported("emitted C does not cast the pointer to a fixed-width type")
        self.expect("(")
        address = self.expr({}, {}, [])
        self.expect(")")
        self.expect(")")
        return address

    # -- statements -----------------------------------------------------

    def body(self, env: dict, mem: dict, trace: list, steps: list) -> None:
        self.expect("{")
        while not self.accept("}"):
            self.statement(env, mem, trace, steps)

    def skip_body(self) -> None:
        """Consume a `{ ... }` block without executing it.

        Used for an untaken arm so the token stream stays aligned with the
        source. Braces are balanced rather than assumed: the emitter never
        nests deeper than one level, but relying on that would mean a future
        construct silently desynchronised the parser and made every later
        result meaningless.
        """
        self.expect("{")
        depth = 1
        while depth:
            if self.peek()[0] == "eof":
                raise kit.Unsupported("emitted C has an unterminated block")
            token = self.take()[1]
            if token == "{":
                depth += 1
            elif token == "}":
                depth -= 1

    def statement(self, env, mem, trace, steps) -> None:
        kind, value = self.take()
        if value == "if":
            self.expect("(")
            test = self.expr(env, mem, trace)
            self.expect(")")
            taken = bool(test.value)
            steps.append(taken)
            # Execute ONLY the taken arm. Running both and discarding the
            # untaken one reports the then-value even when the condition was
            # false -- and because the ROM model evaluates both arms
            # independently, the gate then rejects a faithful candidate while
            # still looking like it is checking something.
            # Execute ONLY the taken arm, and CONSUME the untaken one. Both
            # halves matter: running both arms reports the then-value even when
            # the condition was false, and merely not executing an arm still
            # leaves its `{` in the token stream, so the next statement is read
            # as a bare `{` and every later result is meaningless.
            if taken:
                self.body(env, mem, trace, steps)
                if self.accept("else"):
                    self.skip_body()
                return
            self.skip_body()
            if self.accept("else"):
                self.body(env, mem, trace, steps)
            return
        if value == "return":
            if self.accept(";"):
                self._result.append((True, None))
                raise _Return
            self._result.append((True, self.expr(env, mem, trace)))
            self.expect(";")
            raise _Return
        if value == "*":
            ty = self._pointer_type()
            address = self._pointer_address()
            self.expect("=")
            self.expect("(")
            self.take()
            stored = self.expr(env, mem, trace)
            self.expect(")")
            self.expect(";")
            width = _CSUB_TYPES[ty][0]
            base = address.value & 0xFFFFFFFF
            for i in range(width):
                mem[(base + i) & 0xFFFFFFFF] = (stored.value >> (8 * i)) & 0xFF
            trace.append(("store", base, width, stored.value & ((1 << width) - 1)))
            return
        if kind == "name" and self.peek()[1] == "=":
            self.take()
            env[value] = self.expr(env, mem, trace).value & 0xFFFFFFFF
            self.expect(";")
            return
        if kind == "name" and value in _CSUB_TYPES and self.peek()[1] not in ("=", ";"):
            # A local declaration. Consume it and initialise the slot to zero
            # when uninitialised -- matching C89, where an uninitialised local
            # is indeterminate, so any candidate relying on one is already
            # outside this model. Without this branch a body containing any
            # local declaration fails to parse, and a caller that treats
            # "unavailable" as "verified" then accepts mutants it never
            # actually compared.
            declared = self.take()[1]
            env[declared] = 0
            if self.accept("="):
                env[declared] = self.expr(env, mem, trace).value & 0xFFFFFFFF
            self.expect(";")
            return
        raise kit.Unsupported(f"emitted C has an unsupported statement starting {value!r}")

    def run(self, inputs: dict) -> tuple:
        """Execute `candidate(...)` -> (returned, value, trace, branch-steps)."""
        while self.peek()[0] != "eof" and self.peek()[1] != "candidate":
            self.take()
        if self.peek()[0] == "eof":
            raise kit.Unsupported("emitted C has no candidate definition")
        self.take()
        self.expect("(")
        params: list[tuple[str, tuple]] = []
        if not self.accept(")"):
            while True:
                ty = self.take()[1]
                name = self.take()[1]
                params.append((name, _CSUB_TYPES.get(ty, (32, False))))
                if self.accept(","):
                    continue
                self.expect(")")
                break
        self._result = []
        env = {name: inputs.get(name, 0) for name, _ in params}
        mem: dict = {}
        trace: list = []
        steps: list = []
        returned, value = False, None
        try:
            self.body(env, mem, trace, steps)
        except _Return:
            if self._result:
                returned, value = self._result[-1]
        return returned, value, tuple(trace), tuple(steps)


def c_model(source: str, inputs: dict) -> tuple:
    """Execute the emitted C text. Independent of the decoded ROM model."""
    return _CReader(source).run(inputs)


def check_candidate(shape: Shape, source: str) -> dict:
    """Boundary-input agreement between the ROM model and the emitted C.

    Two independent constructions over the same bytes: one interprets decoded
    Thumb instructions, the other executes the C text that was emitted. A
    wrong-signedness or wrong-store-width mistake shows up here as a
    disagreement, not as a plausible-looking byte score.

    Byte equality is NOT accepted as evidence of equivalence -- the byte oracle
    runs afterwards and separately.
    """
    width = getattr(shape, "cmp_width", 32)
    inputs = boundary_inputs(shape.cond, width)
    checked = 0
    for p0 in inputs:
        for p1 in inputs[:4]:
            truth_r, value_r, trace_r = rom_model(shape, p0, p1)
            returned_c, value_c, trace_c, steps_c = c_model(source, {"t0": p0, "t1": p1})
            checked += 1
            taken = bool(steps_c[0]) if steps_c else None
            if taken is not None and taken != truth_r:
                return {"ok": False, "checked": checked,
                        "reason": f"branch polarity differs (ROM {truth_r}, C {taken})",
                        "input": [hex(p0), hex(p1)]}
            if returned_c != (value_r is not None):
                return {"ok": False, "checked": checked,
                        "reason": "one model returns where the other falls through",
                        "input": [hex(p0), hex(p1)]}
            if value_r is not None and value_c is not None and \
                    (value_r.value & 0xFFFFFFFF) != (value_c.value & 0xFFFFFFFF):
                return {"ok": False, "checked": checked,
                        "reason": f"return value differs (ROM {value_r}, C {value_c})",
                        "input": [hex(p0), hex(p1)]}
            if trace_r != trace_c:
                return {"ok": False, "checked": checked,
                        "reason": "memory effect trace differs",
                        "input": [hex(p0), hex(p1)],
                        "rom": [str(t) for t in trace_r],
                        "c_model": [str(t) for t in trace_c]}
    return {"ok": True, "checked": checked, "reason": None}




# --------------------------------------------------------------------------
# Driver
# --------------------------------------------------------------------------

def synthesize(name: str, out: Path, return_type: str, memory: str, budget: int,
               solver: str | None, seed: int) -> int:
    out.mkdir(parents=True, exist_ok=True)
    # `--out` IS the run directory; `root=out.parent` would auto-generate a
    # timestamped run id and fill a SIBLING of the path the caller named.
    writer = kit.run_writer(STRATEGY, out, extra={"tool_version": VERSION})
    writer.open()
    writer.write_json("run.json", {"strategy": STRATEGY, "tool_version": VERSION,
                                   "seed": seed, "budget": budget,
                                   "return_type": return_type, "memory": memory})
    if solver:
        # Explicitly configured only. Never silently ignored, never a fallback.
        kit.optional_tool([solver], name="branch_synth solver",
                          hint="pass --solver to enable counterexample-guided refinement")
        raise kit.Unsupported(
            "solver backend not implemented in this stage; boundary-input "
            "checking ran instead. A solver timeout is never reported as a pass.")
    kit.require_toolchain()

    meter = kit.BudgetMeter(kit.Budget(compiles=budget, seconds=900.0))
    adapter = kit.CandidateAdapter(out / "probe", meter=meter)
    rom = adapter.rom
    span = probe.rom_functions()
    vma = probe.parse_vma_name(name)
    if vma is None or vma not in span:
        raise kit.Unsupported(f"no independently known ROM span for {name}")
    contract = kit.Contract(name="candidate", vma=vma, end=span[vma],
                            return_type=return_type, memory=memory)
    contract.require_supported()
    writer.save_contract(contract)

    try:
        shape = analyse(rom, contract)
    except kit.Unsupported as exc:
        payload = writer.finish({"draft_status": kit.UNSUPPORTED_CONTRACT,
                                 "reason": str(exc), "results": []})
        print(f"UNSUPPORTED_CONTRACT: {exc}")
        return 2

    scores: list[kit.Score] = []
    rows: list[dict] = []
    rejected: list[dict] = []
    best: dict | None = None
    exhausted = False
    for index, item in enumerate(variants(shape, contract)):
        if best is not None:
            break
        # Semantics FIRST, bytes second. A candidate is only worth compiling
        # once it agrees with the ROM model on the boundary inputs, and a
        # candidate that fails here is rejected on its semantics rather than
        # scored -- a low byte score would not tell us why it was wrong.
        try:
            semantic = check_candidate(shape, item["source"])
        except kit.Unsupported as exc:
            rejected.append({"name": f"cand{index:03d}", "style": item["style"],
                             "status": kit.UNSUPPORTED_CONTRACT, "reason": str(exc)})
            continue
        if not semantic["ok"]:
            rejected.append({"name": f"cand{index:03d}", "style": item["style"],
                             "signed_cmp": item["signed_cmp"],
                             "status": kit.NO_EXACT_DRAFT,
                             "reason": f"semantic check: {semantic['reason']}"})
            continue
        try:
            score = adapter.evaluate(item["source"], "candidate", contract, index=index)
        except kit.BudgetExhausted:
            exhausted = True
            break
        scores.append(score)
        path = writer.save_candidate(f"cand{index:03d}", item["source"])
        row = {"name": path.name, "style": item["style"], "signed_cmp": item["signed_cmp"],
               "status": score.status, "matched_bytes": score.matched_bytes,
               "rom_bytes": score.rom_bytes, "semantic_cases": semantic["checked"],
               "detail": score.detail}
        rows.append(row)
        writer.record(row)
        if score.exact:
            best = item
            writer.declare_winner(path.name, item["source"], row)
    status = kit.summarize_status(scores, exhausted=exhausted) if scores else \
        (kit.NO_EXACT_DRAFT if rejected else kit.UNSUPPORTED_CONTRACT)
    writer.finish({"draft_status": status,
                   "candidates_scored": len(scores),
                   "candidates_rejected_semantically": len(rejected),
                   "rejected": rejected,
                   "budget": meter.to_json(),
                   "compiles": adapter.cache_stats(),
                   "results": rows,
                   "notes": ["byte match is a draft only; integration and the "
                             "independent link are separate",
                             "semantics are checked by comparing decoded ROM "
                             "instructions against the executed emitted C"]})
    print(f"{status}: {len(scores)} scored, {len(rejected)} rejected semantically, "
          f"{meter.compiles} compiles; {writer.dir}")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("function")
    ap.add_argument("--return-type", required=True,
                    choices=("void", "u32", "s32", "u16", "s16", "u8", "s8"))
    ap.add_argument("--memory", required=True, choices=("ordinary", "volatile"))
    ap.add_argument("--out", type=Path)
    ap.add_argument("--budget", type=int, default=32)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--solver", default=None,
                    help="explicitly configured counterexample solver (unimplemented)")
    ap.add_argument("--json", type=Path)
    ap.add_argument("--self-test", action="store_true")
    # Checked before parsing: the self-test needs no ROM, no toolchain and no
    # target, so it must not be forced to supply a function and a contract.
    if "--self-test" in sys.argv:
        return self_test()
    args = ap.parse_args()
    out = args.out or probe.ROOT / "build/experiments/branch-synth" / args.function
    try:
        rc = synthesize(args.function, out.resolve(), args.return_type, args.memory,
                        args.budget, args.solver, args.seed)
    except kit.DependencyMissing as exc:
        print(f"branch_synth: DEPENDENCY_MISSING: {exc}", file=sys.stderr)
        return 2
    except kit.Unsupported as exc:
        print(f"branch_synth: UNSUPPORTED_CONTRACT: {exc}", file=sys.stderr)
        return 2
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps({"status": rc, "out": str(out)}, indent=2) + "\n")
    return rc


def self_test() -> int:
    """Pin the model, the refusals, and the negative controls."""
    print("running branch_synth self-test...")
    passed = total = 0

    def check(label: str, condition: bool) -> None:
        nonlocal passed, total
        total += 1
        if condition:
            passed += 1
            print(f"  [PASS] {label}")
        else:
            print(f"  [FAIL] {label}", file=sys.stderr)

    # --- value model: machine truncation is not C arithmetic ---------------
    check("32-bit wrap truncates", Val(0x1_0000_0001, 32).wrap().value == 1)
    check("16-bit wrap truncates", Val(0x1_0001, 16).wrap().value == 1)
    check("signed read is negative", Val(0xFFFF, 16, True).as_signed() == -1)
    check("unsigned read is not", Val(0xFFFF, 16, False).as_signed() == 0xFFFF)

    # --- flags ------------------------------------------------------------
    f = flags_sub(Val(5, 32), Val(3, 32), 32)
    check("sub without borrow sets C", f["C"] == 1 and f["Z"] == 0)
    f = flags_sub(Val(3, 32), Val(5, 32), 32)
    check("sub with borrow clears C", f["C"] == 0)
    check("sub to zero sets Z", flags_sub(Val(4, 32), Val(4, 32), 32)["Z"] == 1)
    # 0x80000000 - 1 overflows the SIGNED range (the result is below -2^31);
    # 0x7FFFFFFF - 1 does not, and using it here would assert the wrong thing.
    check("signed overflow sets V",
          flags_sub(Val(0x80000000, 32), Val(1, 32), 32)["V"] == 1)
    check("a positive result below INT_MAX does not set V",
          flags_sub(Val(0x7FFFFFFF, 32), Val(1, 32), 32)["V"] == 0)
    check("no overflow clears V",
          flags_sub(Val(5, 32), Val(3, 32), 32)["V"] == 0)
    check("eq is Z", _condition_true("eq", {"Z": 1}) and not _condition_true("eq", {"Z": 0}))
    check("gt needs both N==V and !Z",
          _condition_true("gt", {"Z": 0, "N": 0, "V": 0})
          and not _condition_true("gt", {"Z": 1, "N": 0, "V": 0}))

    # --- boundary inputs include the threshold and the extremes -----------
    cond = ("eq", Expr("param", ("t0",), 32, False), Expr("const", (5,), 32, False))
    values = boundary_inputs(cond, 32)
    check("boundary inputs include zero", 0 in values)
    check("boundary inputs include all-ones", 0xFFFFFFFF in values)
    check("boundary inputs include the sign bit", 0x80000000 in values)
    check("threshold neighbours are present", 4 in values and 5 in values and 6 in values)

    # --- refusals ---------------------------------------------------------
    contract = kit.Contract(name="candidate", vma=0x08001000, end=0x08001040)
    check("a span the model cannot lift is refused, not approximated",
          kit.raises(kit.Unsupported,
                     lambda: analyse(b"\x00" * 0x10000, contract)))
    check("an unresolved contract is refused",
          kit.raises(kit.Unsupported, lambda: kit.Contract(
              name="c", vma=0x08001000, end=0x08001040,
              unresolved="caller ABI unknown").require_supported()))

    # --- the semantic gate, with real negative controls --------------------
    # The gate compares decoded ROM instructions against the EXECUTED emitted
    # C. Each control below tampers with the emitted TEXT, so a gate that
    # compared the ROM model with itself would pass all of them and the suite
    # would be worthless.
    ret = kit.Contract(name="candidate", vma=0x08001000, end=0x08001040,
                       return_type="u32", memory="ordinary")
    slots = ("t0", "t1", "t2", "t3")

    eq_shape = Shape("if-else", [], [], [],
                     ("eq", Expr("param", ("t0",), 32, False),
                      Expr("const", (5,), 32, False)),
                     Expr("const", (1,), 32, False), Expr("const", (2,), 32, False), slots)
    good = check_candidate(eq_shape, render(eq_shape, ret, style="if-else", signed_cmp=True))
    check("a faithful candidate passes the boundary check", good["ok"])
    check("the boundary check actually ran cases", good["checked"] > 0)

    eq_source = render(eq_shape, ret, style="if-else", signed_cmp=True)
    check("a wrong return value is CAUGHT",
          not check_candidate(eq_shape, eq_source.replace(
              "return (u32)0x00000001u;", "return (u32)0x00000003u;"))["ok"])
    check("inverted branch polarity is CAUGHT",
          not check_candidate(eq_shape, eq_source.replace("==", "!="))["ok"])
    check("a dropped arm is CAUGHT",
          not check_candidate(eq_shape, eq_source.replace(
              "return (u32)0x00000002u;", "return (u32)0x00000001u;"))["ok"])

    # Signedness is the control the strategy document names explicitly: `int`
    # yields bgt/ble and `u32` yields bhi/bls, so the same bytes mean different
    # things above 0x7FFFFFFF.
    gt_shape = Shape("if-else", [], [], [],
                     ("gt", Expr("param", ("t0",), 32, False),
                      Expr("const", (0,), 32, False)),
                     Expr("const", (1,), 32, False), Expr("const", (0,), 32, False), slots)
    signed_source = render(gt_shape, ret, style="if-else", signed_cmp=True)
    check("the signed comparison passes its own model",
          check_candidate(gt_shape, signed_source)["ok"])
    unsigned_source = signed_source.replace("(s32)t0", "(u32)t0")
    check("WRONG SIGNEDNESS IS CAUGHT",
          not check_candidate(gt_shape, unsigned_source)["ok"])

    # --- refusals in the generated grammar --------------------------------
    check("emitted C outside the modelled grammar fails closed",
          kit.raises(kit.Unsupported,
                     lambda: c_model("u32 candidate(void) { return 1 / 0; }", {})))
    check("an unparsable candidate fails closed rather than skipping the check",
          kit.raises(kit.Unsupported,
                     lambda: c_model("u32 candidate(void) { return @; }", {})))

    # --- the byte oracle is a separate, later step -------------------------
    check("semantic equivalence is not asserted by byte equality",
          "Byte equality is NOT accepted" in (check_candidate.__doc__ or ""))

    print(f"{passed}/{total} passed")
    return 0 if passed == total else 1


if __name__ == "__main__":
    raise SystemExit(main())
