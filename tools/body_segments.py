#!/usr/bin/env python3
"""Split a ROM body into labelled segments so each one can become C on its own.

A 3-4 kB body cannot be lifted in one sitting, and one monolithic asm block gives
nowhere to stand: the first converted statement would have to reproduce the whole
function, because the compiler only owns the prologue while the body is a single
statement. This tool cuts the span at its real control-flow boundaries and emits
every piece as its own labelled asm block:

    seg_08001C4A8: ;                          <- C `goto` target
        __asm__ volatile ("seg_08001C4A8:"    <- asm `b`/`beq` target
            "\\tldr r0, seg_08001CFE8\\n"
            ...
            : : : "cc", "memory");

Both labels sit at the same address, so a block swaps for C in place: delete the
instruction string, write the statements under the C label, and every neighbour
still reaches it -- `b`/`beq` through the assembler symbol, `goto` through the C
label. Segments are emitted in ROM order and literal pools and jump tables keep
their offsets, so the span stays byte-identical after each swap.

Not every segment can be swapped: agbcc keeps a block's asm only while C can
still reach it. A block that ends in an unconditional `b` or a `return` leaves
the following asm unreachable *from C* when converted, and the compiler deletes
it (measured on `_08001C3E0`: one 2-byte `b` swapped for `goto` removed 233
instructions of 30 other blocks, while a `cond` block's swap removed only its
own two). Each segment therefore carries a verdict -- `yes`, `asm-only` or
`data` -- in the listing and the JSON, and the scaffold marks it per block.

Usage:
    python3 tools/body_segments.py --function _08001C3E0
    python3 tools/body_segments.py --vma 0x0801C3E0 --len 0xE38 --json build/segments/c3e0.json
    python3 tools/body_segments.py --function _08001C3E0 --out-dir build/segments/c3e0
    python3 tools/body_segments.py --function _08001C3E0 --check

`--out-dir` writes `segments.c` (the scaffold), `segments.s` (the same segments as
plain assembly) and `segments.json` (the map). `--check` does two round trips:
it assembles and links the asm fragment at the body's own VMA, then compiles the
C scaffold with the pinned agbcc and links that too, comparing both with the ROM
span. A segmentation that would not survive either trip fails loudly instead of
producing a plausible-looking scaffold.

`--self-test` needs no ROM: it segments a synthetic body and asserts the boundary
rules (branch target, terminator, `bl`, pool word, jump table, padding).
"""
from __future__ import annotations

import argparse
import json
import re
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

import objdump2gas

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_ROM = 'baserom.gba'
MANIFEST = 'tools/matching_slice_functions.json'
VMA_BASE = 0x08000000
# In-span labels are `seg_` + the repo's `0800<file offset>` spelling (the same
# tail `_0800XXXX` labels carry), so one name works as a C identifier and as the
# assembler symbol the neighbouring blocks branch to.
LABEL = 'seg_0800'

FALLTHROUGH, COND, BRANCH, CALL, RETURN, INDIRECT = (
    'fallthrough', 'cond', 'branch', 'call', 'return', 'indirect')


@dataclass
class Segment:
    """One swappable unit of a body.

    `start`/`end` are file offsets (VMA = +0x08000000). `lines` are gas text with
    no label: the segment's own label is emitted from `start`, so a segment that
    has been rewritten in C keeps its assembler symbol for its neighbours.
    """
    start: int
    end: int
    kind: str                       # 'code' | 'data' | 'pad'
    lines: list[str] = field(default_factory=list)
    insns: int = 0
    succ: list[tuple[str, int | None]] = field(default_factory=list)

    @property
    def vma(self) -> int:
        return self.start + VMA_BASE

    @property
    def name(self) -> str:
        return seg_label(self.start)


def seg_label(offset: int) -> str:
    return f'{LABEL}{offset:04X}'


def edge_target(kind: str, target: int | None) -> str:
    return '-' if target is None else f'0x{target + VMA_BASE:08X}'


# --------------------------------------------------------------------- decode
def classify(text: str) -> tuple[str, int | None]:
    """Classify one objdump instruction by how it ends a basic block.

    Returns (kind, target_vma). `call` and `plain` do not end a block; a
    conditional branch ends it but also falls through, so the caller records both
    edges.
    """
    t = text.strip().replace('\t', ' ')
    parts = t.split(None, 1)
    if not parts:
        return 'plain', None
    mnem = parts[0].replace('.n', '')
    rest = parts[1] if len(parts) > 1 else ''
    operand = rest.split(',')[0].strip()
    if mnem in ('bl', 'blx'):
        return 'call', int(rest, 16) if rest.startswith('0x') else None
    if mnem == 'bx':
        return 'return', None
    if mnem.startswith('b') and rest.startswith('0x'):
        return ('branch' if mnem == 'b' else 'cond'), int(rest, 16)
    if operand == 'pc':                 # mov/add/ldr into pc, pop {..., pc}
        return 'indirect', None
    if mnem == 'pop' and re.search(r'\bpc\b', rest):
        return 'indirect', None
    return 'plain', None


def listing(rom: bytes, start: int, end: int):
    """objdump listing of a span: (instrs, order, garbage_wide) as objdump2gas."""
    blob = rom[start:end]
    with tempfile.NamedTemporaryFile(suffix='.bin', delete=False) as fh:
        fh.write(blob)
        path = fh.name
    try:
        proc = subprocess.run(
            ['arm-none-eabi-objdump', '-D', '-b', 'binary', '-m', 'armv4t',
             '-M', 'force-thumb', f'--adjust-vma={start + VMA_BASE:#x}', path],
            capture_output=True, text=True)
        out = proc.stdout
    finally:
        Path(path).unlink(missing_ok=True)
    if not out.strip():
        raise SystemExit('body_segments: objdump produced no output '
                         '(is arm-none-eabi-objdump on PATH?)')
    with tempfile.NamedTemporaryFile('w', suffix='.lst', delete=False) as fh:
        fh.write(out)
        lst = fh.name
    try:
        return objdump2gas.parse_listing(lst)
    finally:
        Path(lst).unlink(missing_ok=True)


# ------------------------------------------------------------------ segments
def build(rom: bytes, start: int, end: int, instrs, order, pool) -> list[Segment]:
    """Segment [start, end) into code, data and pad pieces, in address order.

    Seeds are the span start, every in-span branch/call target and every data
    word that points into the span (a jump table's case addresses are reachable
    only through `mov pc, rN`, so nothing else can discover them). A walk stops at
    the next seed, at a pool word, or after a terminator; whatever no walk covered
    becomes a data run or `.hword` padding.
    """
    data_words = set(pool)
    seeds = {start}
    for word in sorted(data_words):
        if word % 4 or word + 4 > end:
            continue
        value = struct.unpack_from('<I', rom, word)[0]
        if start <= value - VMA_BASE < end:
            seeds.add(value - VMA_BASE)
    for vma in order:
        f = vma - VMA_BASE
        if not start <= f < end or f in data_words:
            continue
        kind, target = classify(instrs[vma][1])
        if kind in ('branch', 'cond', 'call') and target is not None:
            if start <= target - VMA_BASE < end:
                seeds.add(target - VMA_BASE)
        if kind == 'cond' and start <= f + 2 < end:
            seeds.add(f + 2)        # a conditional branch falls through

    segments: list[Segment] = []
    covered: set[int] = set()
    for seed in sorted(seeds):
        if seed in covered or seed in data_words:
            continue
        seg = _walk(seed, start, end, instrs, seeds, data_words)
        segments.append(seg)
        covered.update(range(seg.start, seg.end, 2))

    addr = start
    while addr < end:
        if addr in covered:
            addr += 2
            continue
        if addr in data_words and addr % 4 == 0:
            stop = addr
            while stop in data_words and stop + 4 <= end:
                stop += 4
            segments.append(_data_segment(rom, addr, stop, data_words))
            covered.update(range(addr, stop, 2))
            addr = stop
            continue
        stop = addr
        while stop < end and stop not in covered and stop not in data_words:
            stop += 2
        segments.append(_pad_segment(rom, addr, stop))
        covered.update(range(addr, stop, 2))
        addr = stop
    segments.sort(key=lambda seg: seg.start)
    return segments


def _walk(seed, start, end, instrs, seeds, data_words) -> Segment:
    """Walk one basic block from `seed`; stop at the next boundary."""
    seg = Segment(start=seed, end=seed, kind='code')
    addr = seed
    while addr < end:
        if addr != seed and (addr in seeds or addr in data_words or
                             addr + VMA_BASE not in instrs):
            seg.succ.append((FALLTHROUGH, addr))
            break
        raw, text = instrs.get(addr + VMA_BASE, (None, None))
        if raw is None:
            seg.succ.append((FALLTHROUGH, addr))
            break
        step = 4 if len(raw) >= 2 else 2
        kind, target = classify(text)
        seg.lines.append(objdump2gas.gas_text(
            objdump2gas.strip_comment(text), addr + VMA_BASE, start, end,
            set(), set(), prefix=LABEL))
        seg.insns += 1
        if kind in ('plain', 'call'):
            if kind == 'call':
                seg.succ.append((CALL, None if target is None
                                 else target - VMA_BASE))
            addr += step
            continue
        seg.succ.append((kind, None if target is None else target - VMA_BASE))
        if kind == 'cond':
            seg.succ.append((FALLTHROUGH, addr + step))
        addr += step
        break
    seg.end = max(addr, seg.start + 2)
    return seg


def _data_segment(rom, start, end, data_words) -> Segment:
    seg = Segment(start=start, end=end, kind='data')
    for addr in range(start, end, 4):
        value = struct.unpack_from('<I', rom, addr)[0]
        prefix = seg_label(addr) + ': ' if addr in data_words else '\t'
        seg.lines.append(f'{prefix}.4byte {value:#010x}')
    return seg


def _pad_segment(rom, start, end) -> Segment:
    seg = Segment(start=start, end=end, kind='pad')
    for addr in range(start, end, 2):
        seg.lines.append(f'\t.hword {struct.unpack_from("<H", rom, addr)[0]:#06x}')
    return seg


# -------------------------------------------------------------------- output
def span_table(segments: list[Segment]) -> str:
    width = max((len(seg.name) for seg in segments), default=8)
    rows = [f'  {"segment":<{width}}  {"range":<26} kind  insn bytes  c    '
            f'successors']
    for i, seg in enumerate(segments):
        edges = ' '.join(f'{kind}->{edge_target(kind, target)}'
                         for kind, target in seg.succ)
        verdict = swap_verdict(segments, i)[0]
        rows.append(f'  {seg.name:<{width}}  '
                    f'0x{seg.vma:08X}-0x{seg.vma + seg.end - seg.start:08X}  '
                    f'{seg.kind:<4} {seg.insns:>4} {seg.end - seg.start:>5}  '
                    f'{"yes" if verdict == "yes" else "--":<4} {edges}')
    return '\n'.join(rows)


# ------------------------------------------------------------------- swapping
def swap_verdict(segments: list[Segment], index: int) -> tuple[str, str]:
    """Can segment `index` become C where it stands? ('yes'|'asm-only'|'data', why).

    agbcc keeps a segment's asm in place only while C control flow can still
    reach it. A converted segment that ends in an unconditional `b` or a `return`
    leaves everything up to the next C label unreachable *from C*, and the
    compiler then deletes those asm statements outright -- measured on
    `_08001C3E0`: swapping one 2-byte `b` for `goto` removed 233 instructions
    belonging to 30 other segments, while swapping a `cond` segment removed only
    its own two. So a segment is convertible when it falls through (C keeps the
    sequential edge), or when nothing lies between it and its target.
    """
    seg = segments[index]
    if seg.kind != 'code':
        return ('data', f'{seg.kind} bytes, not code')
    kinds = {kind for kind, _ in seg.succ}
    if FALLTHROUGH in kinds:
        return ('yes', '')
    nxt = segments[index + 1].start if index + 1 < len(segments) else None
    target = next((target for kind, target in seg.succ
                   if kind in (BRANCH, RETURN, INDIRECT) and target is not None),
                  None)
    if nxt is None:
        return ('yes', 'last segment: nothing follows it to be pruned')
    if kind := next(iter(kinds & {BRANCH, RETURN, INDIRECT}), None):
        if kind == BRANCH and target == nxt:
            return ('yes', 'target is the next segment: nothing in between')
        return ('asm-only',
                'ends in ' + ('an unconditional branch' if kind == BRANCH else
                              'a return' if kind == RETURN else
                              'an indirect jump') +
                '; a C goto/return here makes every asm statement up to the next '
                'C label unreachable, and agbcc deletes them')
    return ('asm-only', 'unknown terminator')


def swap_candidates(segments: list[Segment], limit: int) -> list[Segment]:
    """Smallest convertible code segments with at most one inbound edge."""
    inbound: dict[int, int] = {seg.start: 0 for seg in segments}
    for seg in segments:
        for kind, target in seg.succ:
            if kind in (COND, BRANCH, FALLTHROUGH) and target in inbound:
                inbound[target] += 1
    easy = [seg for i, seg in enumerate(segments)
            if seg.kind == 'code' and inbound.get(seg.start, 0) <= 1
            and swap_verdict(segments, i)[0] == 'yes']
    easy.sort(key=lambda seg: (seg.end - seg.start, seg.start))
    return easy[:limit]


def c_identifier(name: str) -> str:
    """A C function name for the scaffold: `--function` may be any VMA spelling."""
    ident = re.sub(r'\W', '_', name)
    return ident if not ident[:1].isdigit() else '_' + ident


def scaffold_c(segments: list[Segment], name: str, span: tuple[int, int]) -> str:
    """The body as one function, one asm block per segment.

    Emitted as a complete translation unit so it can be compiled and compared as
    it stands (`--check` does that); the segments are the only thing in it, so
    the compiler has nothing to allocate and adds no code of its own.
    """
    code = sum(1 for seg in segments if seg.kind == 'code')
    data = sum(1 for seg in segments if seg.kind == 'data')
    pad = sum(1 for seg in segments if seg.kind == 'pad')
    swappable = sum(1 for i in range(len(segments))
                    if swap_verdict(segments, i)[0] == 'yes')
    out = [
        f'/* {name} 0x{span[0] + VMA_BASE:08X}-0x{span[1] + VMA_BASE:08X} '
        f'({span[1] - span[0]} B): {len(segments)} segments '
        f'({code} code, {data} data, {pad} pad) from tools/body_segments.py.',
        ' *',
        ' * Both labels of a segment sit at the same address: the C label is the',
        ' * target of a `goto` from a converted neighbour, the asm label is the',
        ' * target of the `b`/`beq` still living inside an asm block.',
        ' *',
        ' * To convert a segment, keep its two label statements and replace the',
        ' * instruction string with C.',
        ' *',
        ' * While every block is asm the compiler emits no code of its own, so the',
        ' * blocks declare no registers -- there is nothing to keep alive across',
        ' * them. Once a block becomes C, the remaining blocks must declare what',
        ' * they touch: the callee-saved registers they use and r0-r3 if they call.',
        ' *',
        ' * Never move a segment: pool words are addressed by their distance from the',
        ' * ldr sites.',
        ' *',
        f' * {swappable} of the {code} code segments can become C where they stand',
        ' * (`c: yes` below). The rest end in an unconditional branch, a return or',
        ' * an indirect jump: a C `goto`/`return` there makes the asm up to the',
        ' * next C label unreachable from C, and agbcc deletes it.',
        ' */',
        f'void {c_identifier(name)}(void) {{',
    ]
    for i, seg in enumerate(segments):
        edges = ', '.join(f'{kind}->{edge_target(kind, target)}'
                          for kind, target in seg.succ)
        verdict, why = swap_verdict(segments, i)
        out.append(f'/* {seg.kind}, {seg.end - seg.start} B, {seg.insns} insns'
                   + (f', {edges}' if edges else '')
                   + f'; c: {verdict}' + (f' -- {why}' if why else '') + ' */')
        out.append(f'{seg_label(seg.start)}: ;')
        out.append(f'    __asm__ volatile ("{seg_label(seg.start)}:\\n"')
        # objdump prints unified mnemonics; agbcc restores divided syntax for the
        # instructions it emits itself, so every block brackets its own mode.
        out.append('        "\\t.syntax unified\\n"')
        for line in seg.lines:
            escaped = line.replace('\\', '\\\\').replace('"', '\\"')
            out.append(f'        "{escaped}\\n"')
        out.append('        "\\t.syntax divided\\n"')
        out.append('        : : : "cc", "memory");')
        out.append('')
    out.append('}')
    return '\n'.join(out) + '\n'


def scaffold_asm(segments: list[Segment], name: str, span: tuple[int, int]) -> str:
    out = [
        f'@ {name} 0x{span[0] + VMA_BASE:08X}-0x{span[1] + VMA_BASE:08X} '
        f'({span[1] - span[0]} B) in {len(segments)} segments, from '
        f'tools/body_segments.py',
        '@ Assembled and byte-compared by `body_segments.py --check`; the segment',
        '@ labels are the same ones the C scaffold uses.',
        '',
        '\t.syntax unified',
        '\t.thumb',
        '',
    ]
    for seg in segments:
        out.append(f'@ --- {seg.kind} 0x{seg.vma:08X} '
                   f'({seg.end - seg.start} B, {seg.insns} insns)')
        out.append(f'{seg_label(seg.start)}:')
        out.extend(seg.lines)
    return '\n'.join(out) + '\n'


AGBCC = ROOT / 'build/toolchains/agbcc/old_agbcc'
INCLUDE = (ROOT / 'include', ROOT / 'asm', ROOT / 'build/toolchains/agbcc/ginclude')
# agbcc ends every function with its own return, so a body whose last segment
# already holds the ROM's `pop {pc}`/`bx r0` gains these bytes *after* the span.
COMPILER_TAIL = (b'', b'\x70\x47', b'\x70\x47\xc0\x46')


def _build(steps: list[list[str]], blob: Path, label: str) -> bytes | None:
    """Run as/ld/objcopy; the bytes of the linked span, or None on failure."""
    for step in steps:
        proc = subprocess.run(step, capture_output=True, text=True)
        if proc.returncode:
            print(proc.stdout + proc.stderr, file=sys.stderr)
            print(f'body_segments --check: {label}: {Path(step[0]).name} failed',
                  file=sys.stderr)
            return None
    return blob.read_bytes()


def _compare(produced: bytes | None, want: bytes, span: tuple[int, int],
             label: str, tail_ok: bool) -> bool:
    """Report one round trip; `tail_ok` allows the compiler's trailing return."""
    if produced is None:
        return False
    head, tail = produced[:len(want)], produced[len(want):]
    if head == want and (not tail_ok or tail in COMPILER_TAIL):
        note = '' if not tail else f' (+{len(tail)} B compiler return after the span)'
        print(f'body_segments --check: PASS {label} -- {len(want)}/{len(want)} B '
              f'identical at 0x{span[0] + VMA_BASE:08X}{note}')
        return True
    first = next((i for i, (a, b) in enumerate(zip(produced, want)) if a != b), None)
    at = 'length' if first is None else f'0x{span[0] + VMA_BASE + first:08X}'
    print(f'body_segments --check: FAIL {label} -- {len(produced)} B produced vs '
          f'{len(want)} B expected, first difference at {at}', file=sys.stderr)
    return False


def _link_bytes(work: Path, obj: Path, span: tuple[int, int]) -> tuple[list, Path]:
    elf, blob = work / 'body.elf', work / 'body.bin'
    steps = [
        ['arm-none-eabi-ld', f'-Ttext=0x{span[0] + VMA_BASE:08X}',
         '-e', '0', '-o', str(elf), str(obj)],
        ['arm-none-eabi-objcopy', '-O', 'binary', '--only-section=.text',
         str(elf), str(blob)],
    ]
    return steps, blob


def check(segments: list[Segment], rom: bytes, span: tuple[int, int]) -> bool:
    """Assemble + link the fragment at the span's VMA and compare with the ROM."""
    want = rom[span[0]:span[1]]
    with tempfile.TemporaryDirectory() as work:
        work = Path(work)
        src, obj = work / 'segments.s', work / 'segments.o'
        src.write_text(scaffold_asm(segments, 'span', span), encoding='utf-8')
        steps = [['arm-none-eabi-as', '-mcpu=arm7tdmi', '-o', str(obj), str(src)]]
        steps += _link_bytes(work, obj, span)[0]
        produced = _build(steps, work / 'body.bin', 'asm')
    ok = _compare(produced, want, span, 'asm', tail_ok=False)
    return ok and check_c(segments, rom, span)


def check_c(segments: list[Segment], rom: bytes, span: tuple[int, int]) -> bool:
    """Compile the C scaffold with the pinned agbcc and compare the span too.

    This is the half the swap actually depends on: the asm listing proves the
    instructions, but only the compiler proves that the labels, the pool
    references and the block order survive a real translation unit. A body of
    pure asm has nothing for the allocator to do, so the compiled span must be
    identical instruction for instruction.
    """
    if not AGBCC.exists():
        print('body_segments --check: SKIP c -- no pinned agbcc at '
              f'{AGBCC.relative_to(ROOT)} (run make toolchain)', file=sys.stderr)
        return True
    want = rom[span[0]:span[1]]
    src_text = scaffold_c(segments, f'0x{span[0] + VMA_BASE:08X}', span)
    with tempfile.TemporaryDirectory() as work:
        work = Path(work)
        src, pre = work / 'body.c', work / 'body.i'
        asm, obj = work / 'body.s', work / 'body.o'
        src.write_text(src_text, encoding='utf-8')
        inc = []
        for directory in INCLUDE:
            if directory.is_dir():
                inc += ['-I', str(directory)]
        steps = [
            ['clang', '-E', '-nostdinc', '-undef', *inc, str(src)],
        ]
        proc = subprocess.run(steps[0], capture_output=True, text=True)
        if proc.returncode:
            print(proc.stdout + proc.stderr, file=sys.stderr)
            print('body_segments --check: c: clang -E failed', file=sys.stderr)
            return False
        pre.write_text(proc.stdout, encoding='utf-8')
        steps = [
            [str(AGBCC), '-O2', '-mthumb-interwork', '-ffunction-sections',
             str(pre), '-o', str(asm)],
            ['arm-none-eabi-as', '-mcpu=arm7tdmi', '-o', str(obj), str(asm)],
        ]
        steps += _link_bytes(work, obj, span)[0]
        produced = _build(steps, work / 'body.bin', 'c')
    ok_c = _compare(produced, want, span, 'c  ', tail_ok=True)
    return ok_c


# --------------------------------------------------------------------- spans
def span_from_args(args) -> tuple[int, int]:
    """(start, end) as file offsets from --function/--vma and --len/--manifest."""
    if args.function:
        vma = parse_vma(args.function)
        if vma is None:
            raise SystemExit(f'body_segments: no VMA in {args.function!r}')
    elif args.vma:
        vma = int(args.vma, 0)
        if vma < VMA_BASE:
            vma += VMA_BASE
    else:
        raise SystemExit('body_segments: pass --function NAME or --vma ADDR')
    start = vma - VMA_BASE
    if args.len:
        return start, start + int(args.len, 0)
    entry = manifest().get(f'{vma:#010x}')
    if entry is None:
        raise SystemExit(f'body_segments: no manifest span for 0x{vma:08X}; '
                         'pass --len')
    return start, int(entry['end_vma'], 16) - VMA_BASE


def manifest() -> dict:
    path = ROOT / MANIFEST
    if not path.exists():
        return {}
    return json.loads(path.read_text(encoding='utf-8'))


def parse_vma(name: str) -> int | None:
    """`_08001C3E0`, `0x0801C3E0` and `_08001c3e0` all name 0x0801C3E0."""
    match = re.search(r'0*80([0-9A-Fa-f]{4,7})(?![0-9A-Fa-f])', name)
    if not match:
        return None
    return VMA_BASE + int(match.group(1), 16)


# ----------------------------------------------------------------- self-test
def self_test() -> int:
    """Segment a synthetic body; assert the boundary rules without a ROM."""
    start = 0x10000
    body = b''.join(struct.pack('<H', hw) for hw in [
        0x2001,          # 0x00 movs r0, #1
        0xF000, 0xF809,  # 0x02 bl (external: a call, not a split)
        0x2800,          # 0x06 cmp r0, #0
        0xD004,          # 0x08 beq.n  -> 0x08010014
        0xE003,          # 0x0A b.n    -> 0x08010014
        0x46C0,          # 0x0C unreachable pad
        0x46C0,          # 0x0E unreachable pad
        0x46C0,          # 0x10 unreachable pad
        0x46C0,          # 0x12 unreachable pad
        0x4802,          # 0x14 ldr r0, [pc, #8] -> 0x08010020
        0x4687,          # 0x16 mov pc, r0
        0x46C0,          # 0x18 unreachable
        0x46C0,          # 0x1A unreachable
        0x46C0,          # 0x1C unreachable
        0x46C0,          # 0x1E unreachable
        0x0014, 0x0801,  # 0x20 .word 0x08010014 (pool + jump-table case address)
    ])
    rom = bytes(start) + body     # file offsets are absolute, as in the real tool
    instrs = {
        0x10000 + VMA_BASE: (['2001'], 'movs\tr0, #1'),
        0x10002 + VMA_BASE: (['f000', 'f809'], 'bl\t0x80080f4'),
        0x10006 + VMA_BASE: (['2800'], 'cmp\tr0, #0'),
        0x10008 + VMA_BASE: (['d004'], 'beq.n\t0x8010014'),
        0x1000A + VMA_BASE: (['e003'], 'b.n\t0x8010014'),
        0x1000C + VMA_BASE: (['46c0'], 'movs\tr0, r0'),
        0x1000E + VMA_BASE: (['46c0'], 'movs\tr0, r0'),
        0x10010 + VMA_BASE: (['46c0'], 'movs\tr0, r0'),
        0x10012 + VMA_BASE: (['46c0'], 'movs\tr0, r0'),
        0x10014 + VMA_BASE: (['4802'], 'ldr\tr0, [pc, #8]\t@ (0x8010020)'),
        0x10016 + VMA_BASE: (['4687'], 'mov\tpc, r0'),
        0x10018 + VMA_BASE: (['46c0'], 'movs\tr0, r0'),
        0x1001A + VMA_BASE: (['46c0'], 'movs\tr0, r0'),
        0x1001C + VMA_BASE: (['46c0'], 'movs\tr0, r0'),
        0x1001E + VMA_BASE: (['46c0'], 'movs\tr0, r0'),
    }
    segments = build(rom, start, start + len(body), instrs, sorted(instrs),
                     {0x10020})
    by_start = {seg.start: seg for seg in segments}
    first, branch, tail = (by_start.get(0x10000), by_start.get(0x1000A),
                           by_start.get(0x10014))
    table = by_start.get(0x10020)
    checks = [
        ('branch target starts a segment', tail is not None),
        ('conditional branch ends its block',
         first is not None and first.end == 0x1000A),
        ('bl does not split the block', first is not None and first.insns == 4),
        ('cond/branch/fallthrough edges recorded',
         first is not None and [(COND, 0x10014), (FALLTHROUGH, 0x1000A)] ==
         [edge for edge in first.succ if edge[0] in (COND, FALLTHROUGH)] and
         branch is not None and (BRANCH, 0x10014) in branch.succ),
        ('call edge kept out of control flow',
         first is not None and (CALL, 0x80F4) in first.succ),
        ('mov pc ends its block as indirect',
         tail is not None and tail.end == 0x10018 and (INDIRECT, None) in tail.succ),
        ('pool word is a labelled data segment',
         table is not None and table.kind == 'data' and
         table.lines[0].startswith(seg_label(0x10020) + ':')),
        ('unreachable halfwords are padding',
         by_start.get(0x1000C) is not None and
         by_start[0x1000C].kind == 'pad' and by_start[0x1000C].end == 0x10014),
        ('labels are C identifiers and match the asm renderer',
         re.fullmatch(r'seg_0800[0-9A-F]{4,5}', first.name) is not None and
         objdump2gas.gas_text('b.n\t0x8010014', 0x801000C, 0x10000, 0x10024,
                              set(), set(), prefix=LABEL) == f'b {seg_label(0x10014)}'),
        ('segments cover the span exactly once',
         sum(seg.end - seg.start for seg in segments) == len(body)),
        ('fallthrough and cond segments are C-swappable',
         first is not None and swap_verdict(segments, segments.index(first))[0]
         == 'yes'),
        ('unconditional branch is asm-only when asm follows it',
         branch is not None and
         swap_verdict(segments, segments.index(branch))[0] == 'asm-only'),
        ('indirect jump and data are not C',
         tail is not None and
         swap_verdict(segments, segments.index(tail))[0] == 'asm-only' and
         swap_verdict(segments, segments.index(table))[0] == 'data'),
    ]
    failed = [name for name, ok in checks if not ok]
    for name, ok in checks:
        print(f'  {"PASS" if ok else "FAIL"} {name}')
    print(f'body_segments self-test: {len(checks) - len(failed)}/{len(checks)} '
          f'checks {"PASS" if not failed else "FAIL"}')
    return 1 if failed else 0


# ---------------------------------------------------------------------- main
def main(argv=None) -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--function', help='VMA-shaped function name, e.g. _08001C3E0')
    ap.add_argument('--vma', help='body start VMA (or file offset)')
    ap.add_argument('--len', help='body length in bytes (default: the manifest span)')
    ap.add_argument('--rom', default=DEFAULT_ROM)
    ap.add_argument('--json', help='write the segment map here')
    ap.add_argument('--out-dir', help='write segments.c, segments.s and segments.json')
    ap.add_argument('--check', action='store_true',
                    help='reassemble the fragments and compare with the ROM span')
    ap.add_argument('--max-swap', type=int, default=5,
                    help='how many easy swap candidates to list (default 5)')
    ap.add_argument('--self-test', action='store_true')
    args = ap.parse_args(argv)

    if args.self_test:
        return self_test()

    start, end = span_from_args(args)
    rom = Path(args.rom).read_bytes()
    instrs, order, _garbage = listing(rom, start, end)
    pool = objdump2gas.detect_pools(rom, start, end)
    segments = build(rom, start, end, instrs, order, pool)

    name = args.function or f'0x{start + VMA_BASE:08X}'
    code = [seg for seg in segments if seg.kind == 'code']
    data = [seg for seg in segments if seg.kind == 'data']
    pad = [seg for seg in segments if seg.kind == 'pad']
    swappable = sum(1 for i in range(len(segments))
                    if swap_verdict(segments, i)[0] == 'yes')
    print(f'{name} 0x{start + VMA_BASE:08X}-0x{end + VMA_BASE:08X} '
          f'({end - start} B): {len(segments)} segments -- {len(code)} code, '
          f'{len(data)} data, {len(pad)} pad; '
          f'{sum(seg.insns for seg in segments)} instructions, '
          f'{sum(seg.end - seg.start for seg in code)} B of code; '
          f'{swappable} segments can become C in place')
    print(span_table(segments))

    easy = swap_candidates(segments, args.max_swap)
    if easy:
        print('\nsmallest segments with at most one inbound edge (start here):')
        for seg in easy:
            edges = ' '.join(f'{kind}->{edge_target(kind, target)}'
                             for kind, target in seg.succ)
            print(f'  {seg.name}  0x{seg.vma:08X}  {seg.end - seg.start:>3} B  '
                  f'{seg.insns:>2} insns  {edges}')

    report = {
        'function': name,
        'start_vma': f'0x{start + VMA_BASE:08X}',
        'end_vma': f'0x{end + VMA_BASE:08X}',
        'bytes': end - start,
        'label_prefix': LABEL,
        'segments': [
            {
                'name': seg.name,
                'start_vma': f'0x{seg.vma:08X}',
                'end_vma': f'0x{seg.vma + seg.end - seg.start:08X}',
                'kind': seg.kind,
                'insns': seg.insns,
                'bytes': seg.end - seg.start,
                'successors': [
                    {'kind': kind, 'target_vma': None if target is None
                     else f'0x{target + VMA_BASE:08X}'}
                    for kind, target in seg.succ
                ],
                'c_swap': swap_verdict(segments, i)[0],
                'c_swap_reason': swap_verdict(segments, i)[1],
            }
            for i, seg in enumerate(segments)
        ],
    }
    if args.json:
        Path(args.json).parent.mkdir(parents=True, exist_ok=True)
        Path(args.json).write_text(json.dumps(report, indent=1) + '\n',
                                   encoding='utf-8')
        print(f'\nwrote {args.json}')
    if args.out_dir:
        out = Path(args.out_dir)
        out.mkdir(parents=True, exist_ok=True)
        (out / 'segments.c').write_text(scaffold_c(segments, name, (start, end)),
                                        encoding='utf-8')
        (out / 'segments.s').write_text(scaffold_asm(segments, name, (start, end)),
                                        encoding='utf-8')
        (out / 'segments.json').write_text(json.dumps(report, indent=1) + '\n',
                                           encoding='utf-8')
        print(f'wrote {out}/segments.c, segments.s, segments.json')
    if args.check:
        return 0 if check(segments, rom, (start, end)) else 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
