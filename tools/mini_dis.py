#!/usr/bin/env python3
"""Disassemble a ROM address range.

Prefers `arm-none-eabi-objdump` on a byte slice of baserom.gba: the local
windowing makes addresses exact and keeps objdump's own decoder (the previous
hand-rolled table mis-decoded `adds rd, rn, #imm`, the high-register ops and
every PC-relative literal, which made control flow hard to follow).

Usage:
    python3 tools/mini_dis.py 0x08021F88 0x0802205C        # [start, end)
    python3 tools/mini_dis.py 0x08021F88 --len 0xD4

`--pools` re-labels any `ldr rX, [pc, #n]` target word as `.word` data so the
literal pool stops looking like instruction garbage.
"""
import shutil
import struct
import subprocess
import sys
import tempfile

ROM = 'baserom.gba'
MASK = 0x00FFFFFF


def rom_bytes():
    with open(ROM, 'rb') as f:
        return f.read()


def rd2(rom, a):
    o = a & MASK
    return struct.unpack('<H', rom[o:o + 2])[0]


def rd4(rom, a):
    o = a & MASK
    return struct.unpack('<I', rom[o:o + 4])[0]


def objdump(start, end):
    rom = rom_bytes()
    size = end - start
    blob = rom[start & MASK:(start & MASK) + size]
    with tempfile.NamedTemporaryFile(suffix='.bin', delete=False) as fh:
        fh.write(blob)
        path = fh.name
    cmd = ['arm-none-eabi-objdump', '-D', '-b', 'binary', '-m', 'armv4t',
           '-M', 'force-thumb', f'--adjust-vma={start:#x}', path]
    out = subprocess.run(cmd, capture_output=True, text=True).stdout
    lines = []
    for line in out.splitlines():
        line = line.rstrip()
        if not line or line.endswith(':') or line.lstrip().startswith('...'):
            continue
        head = line.split('\t')[0].strip().rstrip(':')
        try:
            addr = int(head, 16)
        except ValueError:
            continue
        parts = [p for p in line.split('\t') if p.strip()]
        text = '\t'.join(p.strip() for p in parts[2:]) if len(parts) > 2 else ''
        lines.append((addr, text))
    return lines, rom


def pool_target(addr, text):
    """Resolve the literal a `ldr rX, [pc, #n]` reads.

    objdump >= 2.38 appends the resolved address as `@ (0x...)`; older ones do
    not, so fall back to `(addr + 4 & ~3) + n`. The resolved form has to win:
    parsing the operand alone trips over that trailing comment.
    """
    if ', [pc, #' not in text:
        return None
    for token in text.split():
        if token.startswith('(') and hexish(token.lstrip('(').rstrip(')')):
            return int(token.lstrip('(').rstrip(')'), 16)
    try:
        off = int(text.split('#')[1].split(']')[0], 0)
    except (IndexError, ValueError):
        return None
    return ((addr + 4) & ~3) + off


def hexish(s):
    try:
        int(s, 16)
    except ValueError:
        return False
    return s.startswith('0x')


def annotate_pools(lines, rom):
    """Mark words referenced by `ldr rX, [pc, #n]` as literal-pool data."""
    out = []
    pending = set()
    i = 0
    while i < len(lines):
        a, t = lines[i]
        if a in pending:
            val = rd4(rom, a)
            out.append((a, f'.word {val:#010x}'))
            pending.discard(a)
            i += 1
            # skip the raw halves of that word
            while i < len(lines) and lines[i][0] < a + 4:
                i += 1
            continue
        out.append((a, t))
        lit = pool_target(a, t)
        if lit is not None:
            pending.add(lit)
        i += 1
    return out


def main(argv):
    if len(argv) < 2 or argv[1] in ('-h', '--help'):
        print(__doc__)
        return 2
    start = int(argv[1], 0)
    if len(argv) > 2 and argv[2] == '--len':
        end = start + int(argv[3], 0)
        rest = argv[4:]
    else:
        end = int(argv[2], 0) if len(argv) > 2 else start + 2
        rest = argv[3:]
    pools = '--pools' in rest
    lines, rom = objdump(start, end)
    if not lines:
        print('mini_dis: objdump produced no output (is arm-none-eabi-objdump '
              'on PATH?)', file=sys.stderr)
        return 1
    if pools:
        lines = annotate_pools(lines, rom)
    for a, t in lines:
        print(f'{a:08x}: {t}')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
