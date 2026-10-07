#!/usr/bin/env python3
"""Bounded Thumb-1 leaf-to-C search with the project's exact agbcc byte oracle.

No source/manifest edits. ABI return contract and memory qualification must be
selected from ROM/caller evidence. Unsupported instructions fail closed.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import re
import subprocess
import sys

import corpus_match_probe as probe
from match_families import digest, assembled_modes, mode_at


class Unsupported(ValueError):
    pass


@dataclass
class Node:
    name: str
    expression: str
    memory: bool = False
    store: bool = False


@dataclass
class Lift:
    nodes: list[Node]
    result: str
    inputs: set[int]
    instructions: list[dict]


def lift(rom, start, end, memory='ordinary', max_instructions=32, modes=None):
    """SSA transcription of an intentionally small, flag-independent grammar.

    No stack, calls, branches, register lists, multiple-transfer, or writes to
    callee-saved registers. Memory widths and sign extension come from opcodes.
    """
    regs, inputs, nodes, evidence = {}, set(), [], []
    qualifier = 'volatile ' if memory == 'volatile' else ''
    def read(r):
        if r not in regs:
            if r > 3:
                raise Unsupported(f'live-in r{r} is not a parameter register')
            inputs.add(r)
            regs[r] = f'p{r}'
        return regs[r]
    def put(r, expression, mem=False):
        if r > 3:
            raise Unsupported(f'write to r{r} requires a wider ABI/stack model')
        name = f't{len(nodes)}'
        nodes.append(Node(name, expression, memory=mem))
        regs[r] = name
    def access(address, width, signed=False):
        ty = ('s' if signed else 'u') + str(width)
        return f'*({qualifier}{ty} *)({address})'
    pc = start
    while pc + 2 <= end and len(evidence) < max_instructions:
        if modes is not None and mode_at(modes, pc) != 't':
            raise Unsupported(f'address {pc:#x} is ARM/data in the assembled closure')
        off = pc - probe.ROM_BASE
        h = int.from_bytes(rom[off:off + 2], 'little')
        evidence.append({'vma': hex(pc), 'file_offset': hex(off), 'halfword': f'{h:04x}'})
        if h == 0x4770:  # bx lr
            return Lift(nodes, regs.get(0, 'p0'), inputs, evidence)
        if h & 0xF800 == 0x1800:  # add/sub register or 3-bit immediate
            rd, rs, operand = h & 7, (h >> 3) & 7, (h >> 6) & 7
            right = str(operand) + 'u' if h & 0x400 else read(operand)
            put(rd, f'({read(rs)} {"-" if h & 0x200 else "+"} {right})')
        elif h & 0xE000 == 0 and h & 0x1800 != 0x1800:
            op, amount, rs, rd = (h >> 11) & 3, (h >> 6) & 31, (h >> 3) & 7, h & 7
            val = read(rs)
            if op == 0:
                expr = val if amount == 0 else f'({val} << {amount})'
            elif op == 1:
                expr = '0u' if amount == 0 else f'({val} >> {amount})'
            else:
                expr = f'((u32)((s32){val} >> {amount or 31}))'
            put(rd, expr)
        elif h & 0xE000 == 0x2000:
            op, rd, imm = (h >> 11) & 3, (h >> 8) & 7, h & 255
            if op == 1:
                raise Unsupported('CMP/flags are outside the straight-line grammar')
            put(rd, f'{imm}u' if op == 0 else f'({read(rd)} {"+" if op == 2 else "-"} {imm}u)')
        elif h & 0xFC00 == 0x4000:
            op, rs, rd = (h >> 6) & 15, (h >> 3) & 7, h & 7
            right = read(rs)
            binary = {0: '&', 1: '^', 12: '|', 13: '*', 14: '&'}
            if op in binary:
                put(rd, f'({read(rd)} {binary[op]} {"~" if op == 14 else ""}{right})')
            elif op == 9:
                put(rd, f'(0u - {right})')
            elif op == 15:
                put(rd, f'(~{right})')
            else:
                raise Unsupported('flag-dependent, compare or register-shift ALU instruction')
        elif h & 0xFC00 == 0x4400:
            op, rs, rd = (h >> 8) & 3, (h >> 3) & 15, (h & 7) | ((h >> 4) & 8)
            if op not in (0, 2):
                raise Unsupported('high-register compare/branch')
            put(rd, read(rs) if op == 2 else f'({read(rd)} + {read(rs)})')
        elif h & 0xF800 == 0x4800:
            address = ((pc + 4) & ~3) + (h & 255) * 4
            offset = address - probe.ROM_BASE
            if offset < 0 or offset + 4 > len(rom):
                raise Unsupported('literal outside ROM')
            value = int.from_bytes(rom[offset:offset + 4], 'little')
            evidence[-1]['literal'] = {'vma': hex(address), 'value': hex(value)}
            put((h >> 8) & 7, f'0x{value:08X}u')
        elif h & 0xE000 == 0x6000 or h & 0xF000 == 0x8000:
            rd, rb, immediate = h & 7, (h >> 3) & 7, (h >> 6) & 31
            width = 16 if h & 0xF000 == 0x8000 else (8 if h & 0x1000 else 32)
            address = f'({read(rb)} + {immediate * (width // 8)}u)'
            lvalue = access(address, width)
            if h & 0x800:
                put(rd, f'(u32)({lvalue})', True)
            else:
                nodes.append(Node(f't{len(nodes)}', f'{lvalue} = (u{width}){read(rd)};', True, True))
        elif h & 0xF000 == 0x5000:
            op, ro, rb, rd = (h >> 9) & 7, (h >> 6) & 7, (h >> 3) & 7, h & 7
            width = (32, 16, 8, 8, 32, 16, 8, 16)[op]
            lvalue = access(f'({read(rb)} + {read(ro)})', width, op in (3, 7))
            if op >= 3:
                put(rd, f'(u32)({lvalue})', True)
            else:
                nodes.append(Node(f't{len(nodes)}', f'{lvalue} = (u{width}){read(rd)};', True, True))
        else:
            raise Unsupported(f'unsupported halfword {h:04x} at {pc:#x}')
        pc += 2
    raise Unsupported('no bx lr within span/instruction budget')


TYPES = '''typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
'''


def variants(model, return_type, limit=32):
    """Enumerate named SSA, local-order, and safe single-use expression shapes.

    Inline only pure single-use nodes. A lone memory operation may be inlined
    when there are no stores, so no memory ordering or access count is changed.
    """
    inputs = set(model.inputs)
    if return_type != 'void' and model.result == 'p0':
        inputs.add(0)
    argc = max(inputs, default=-1) + 1
    params = ', '.join(f'u32 p{i}' for i in range(argc)) or 'void'
    emitted = set()
    for inline in (False, True):
        for reverse in (False, True):
            for cast in (False, True):
                for pointer_params in (False, True):
                    nodes = [Node(n.name, n.expression, n.memory, n.store) for n in model.nodes]
                    result = model.result
                    if inline:
                        memory_count = sum(n.memory for n in nodes)
                        for n in list(nodes):
                            if n.store or (n.memory and memory_count != 1):
                                continue
                            uses = sum(len(re.findall(r'\b' + n.name + r'\b', x.expression)) for x in nodes if x != n)
                            uses += int(return_type != 'void' and result == n.name)
                            if uses != 1:
                                continue
                            for other in nodes:
                                if other != n:
                                    other.expression = re.sub(r'\b' + n.name + r'\b', '(' + n.expression + ')', other.expression)
                            if result == n.name:
                                result = n.expression
                            nodes.remove(n)
                    declarations = [n for n in nodes if not n.store]
                    if reverse:
                        declarations.reverse()
                    body = [f'    u32 {n.name};' for n in declarations]
                    body += ['    ' + (n.expression if n.store else f'{n.name} = {n.expression};') for n in nodes]
                    if return_type != 'void':
                        body.append(f'    return {"(" + return_type + ")" if cast else ""}{result};')
                    parameter_text = params
                    if pointer_params and argc:
                        parameter_text = ', '.join(f'void *p{i}' for i in range(argc))
                        body = [re.sub(r'\bp([0-3])\b', r'((u32)p\1)', line) for line in body]
                    source = TYPES + f'\n{return_type} candidate({parameter_text})\n{{\n' + '\n'.join(body) + '\n}\n'
                    if source in emitted:
                        continue
                    emitted.add(source)
                    yield source
                    if len(emitted) >= limit:
                        return


def synthesize(name, out, return_type, memory, budget):
    # Use a fresh scoped packet/report to establish canonical source and span.
    out.mkdir(parents=True, exist_ok=True)
    # A failed/unsupported rerun must never leave an earlier winner advertised.
    (out / 'winner.c').unlink(missing_ok=True)
    (out / 'summary.json').write_text(json.dumps({'status': 'STARTED', 'function': name}) + '\n')
    report = out / 'baseline.json'
    command = [sys.executable, str(probe.ROOT / 'tools/corpus_match_probe.py'), '--function', name,
               '--c89', '--require-all', '--work-dir', str(out / 'baseline'), '--json', str(report)]
    run = subprocess.run(command, text=True, capture_output=True)
    (out / 'baseline.log').write_text(run.stdout + run.stderr)
    if run.returncode:
        raise ValueError(f'baseline probe failed; see {out / "baseline.log"}')
    records = json.loads(report.read_text())['results']
    if len(records) != 1:
        raise ValueError('baseline must identify exactly one body')
    rec = records[0]
    start = int(rec['vma'], 16)
    end = start + rec['rom_bytes']
    rom = probe.ROM.read_bytes()
    summary = {'function': name, 'vma': rec['vma'], 'file_offset': hex(start - probe.ROM_BASE),
               'rom_sha256': digest(probe.ROM), 'return_type': return_type, 'memory': memory,
               'budget': budget, 'promotion': False,
               'abi_note': 'Word-sized input registers only; candidate parameter types require caller review.',
               'compiler': 'agbcc -O2 -mthumb-interwork -ffunction-sections',
               'compiler_sha256': digest(probe.AGBCC), 'results': []}
    try:
        model = lift(rom, start, end, memory, modes=assembled_modes(out))
    except Unsupported as exc:
        summary.update(status='UNSUPPORTED', reason=str(exc))
        (out / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
        print(f'UNSUPPORTED: {exc}; {out / "summary.json"}')
        return 2
    summary['instructions'] = model.instructions
    summary['input_registers'] = sorted(model.inputs | ({0} if return_type != 'void' and model.result == 'p0' else set()))
    resolver = probe.SymbolResolver(probe.load_code_symbols(), probe.alias_targets(), probe.scan_decl_hints())
    old_work = probe.WORK
    probe.WORK = out
    try:
        for number, source in enumerate(variants(model, return_type, budget)):
            directory = out / f'candidate-{number:03d}'
            directory.mkdir(exist_ok=True)
            c, assembly, obj = (directory / x for x in ('candidate.c', 'candidate.s', 'candidate.o'))
            c.write_text(source)
            commands = [[str(probe.AGBCC), '-O2', '-mthumb-interwork', '-ffunction-sections', str(c), '-o', str(assembly)],
                        ['arm-none-eabi-as', '-mcpu=arm7tdmi', '-o', str(obj), str(assembly)]]
            failed = False
            for command in commands:
                result = subprocess.run(command, text=True, capture_output=True)
                if result.returncode:
                    summary['results'].append({'candidate': number, 'status': 'COMPILE_ERROR', 'error': result.stderr})
                    failed = True
                    break
            if failed:
                continue
            sections = probe.section_map(obj)
            key = '.text.candidate'
            if key not in sections:
                raise ValueError('compiler emitted no candidate section')
            blob, calls, pools = probe.link_function(sections[key], start, probe.relocations(obj, key), key, resolver)
            score = probe.compare_function(rom, start, f'candidate_{number:03d}', blob, {start: end}, calls, pools)
            if score is None:
                score = {'status': 'UNSCORED', 'reason': 'span smaller than probe minimum'}
            score['candidate'] = number
            score['source'] = str(c)
            summary['results'].append(score)
            if score['status'] == 'EXACT':
                # Source stays a draft. Integration may change allocation/layout.
                (out / 'winner.c').write_text(source)
                break
    finally:
        probe.WORK = old_work
    summary['status'] = 'EXACT_DRAFT' if any(r['status'] == 'EXACT' for r in summary['results']) else 'NO_EXACT_DRAFT'
    (out / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(f"{summary['status']}: {len(summary['results'])} candidates; {out / 'summary.json'}")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('function')
    ap.add_argument('--out', type=Path)
    ap.add_argument('--return-type', choices=('void', 'u32', 's32', 'u16', 's16', 'u8', 's8'), required=True)
    ap.add_argument('--memory', choices=('ordinary', 'volatile'), required=True)
    ap.add_argument('--budget', type=int, default=32)
    args = ap.parse_args()
    if not 1 <= args.budget <= 256:
        ap.error('--budget must be between 1 and 256')
    out = args.out or probe.ROOT / 'build/matching-assist/synth' / args.function
    try:
        return synthesize(args.function, out.resolve(), args.return_type, args.memory, args.budget)
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(f'leaf_synth: {exc}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
