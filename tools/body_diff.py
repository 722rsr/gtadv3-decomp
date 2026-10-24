#!/usr/bin/env python3
"""Instruction-level diff of one ROM body: reference vs the C build.

`make` only answers "the whole image matches". That is the right gate but a
terrible debugging loop: a body that is off by one register prints nothing
about *which* register. This tool disassembles the same address range out of
`baserom.gba` and the freshly linked image, aligns the two streams and prints
only the rows around the first disagreement, so a spelling can be steered from
the listing.

Usage:
    python3 tools/body_diff.py --vma 0x08027308 --len 0x158
    python3 tools/body_diff.py --vma 0x08027308        # range auto-detected
    python3 tools/body_diff.py --vma 0x08027308 --len 0x158 --all
    python3 tools/body_diff.py --vma 0x08007110 --cand build/probe/_08007110_cand.bin

With no `--len` it scans the two images for differing bytes, which only works
when the change under test is the only edit in the tree (the usual case for a
single-body lift). `--all` prints every row instead of the first mismatch
window. Exit status is 0 when the range is byte-identical.

`--cand` compares the ROM span against a bare function blob (the probe's
`<name>_cand.bin` in its `--work-dir`) instead of `build/gtadv3.gba`, so one
body can be inspected without relinking the whole image.
"""
import argparse
import subprocess
import sys
import tempfile

MASK = 0x00FFFFFF


def load(path):
    with open(path, 'rb') as fh:
        return fh.read()


def rd4(rom, a):
    return int.from_bytes(rom[a & MASK:a & MASK + 4], 'little')


def objdump(rom, start, end):
    blob = rom[start & MASK:(start & MASK) + (end - start)]
    with tempfile.NamedTemporaryFile(suffix='.bin', delete=False) as fh:
        fh.write(blob)
        path = fh.name
    cmd = ['arm-none-eabi-objdump', '-D', '-b', 'binary', '-m', 'armv4t',
           '-M', 'force-thumb', f'--adjust-vma={start:#x}', path]
    out = subprocess.run(cmd, capture_output=True, text=True).stdout
    rows = []
    for line in out.splitlines():
        parts = [p for p in line.rstrip().split('\t') if p.strip()]
        if len(parts) < 2:
            continue
        head = parts[0].strip().rstrip(':')
        try:
            addr = int(head, 16)
        except ValueError:
            continue
        rows.append((addr, ' '.join(parts[2:]) if len(parts) > 2 else '', parts[1]))
    return rows


def changed_span(ref, cand, lo, hi):
    """First/last differing byte in [lo, hi), or None."""
    first = last = None
    for i in range(lo, hi):
        if i >= len(ref) or i >= len(cand):
            break
        if ref[i] != cand[i]:
            if first is None:
                first = i
            last = i
    if first is None:
        return None
    return ((first & ~1) | 0x08000000, ((last & ~1) + 2) | 0x08000000)


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument('--vma', required=True, type=lambda s: int(s, 0),
                    help='function address, e.g. 0x08027308')
    ap.add_argument('--len', type=lambda s: int(s, 0), default=None)
    ap.add_argument('--base', default='baserom.gba')
    ap.add_argument('--build', default='build/gtadv3.gba')
    ap.add_argument('--cand', default=None,
                    help='bare candidate blob for this body, instead of --build')
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--window', type=int, default=6,
                    help='rows of context each side of the first mismatch')
    args = ap.parse_args(argv)

    ref, cand = load(args.base), load(args.build)
    vma = args.vma
    if args.cand is not None:
        cand = load(args.cand)
        if args.len is None:
            args.len = len(cand)
        # A bare blob is addressed from 0, so slide it onto the ROM VMA.
        cand = b'\0' * (vma & MASK) + cand
    if args.len is not None:
        start, end = vma, vma + args.len
    else:
        span = changed_span(ref, cand, vma & MASK, min(len(ref), len(cand)))
        if span is None:
            start, end = vma, vma + 0x400
        else:
            start, end = span
            if start < vma:
                start = vma
    if start >= end:
        print(f'body_diff: empty range {start:#x}..{end:#x}', file=sys.stderr)
        return 2

    a_rows = objdump(ref, start, end)
    b_rows = objdump(cand, start, end)
    bmap = {addr: (txt, raw) for addr, txt, raw in b_rows}

    first = None
    for addr, txt, raw in a_rows:
        other = bmap.get(addr)
        if other is None or other[0] != txt:
            first = addr
            break

    lo = 0
    hi = len(a_rows)
    if first is not None and not args.all:
        idx = next(i for i, r in enumerate(a_rows) if r[0] == first)
        lo = max(0, idx - args.window)
        hi = min(len(a_rows), idx + args.window + 1)

    differs = 0
    for i, (addr, txt, raw) in enumerate(a_rows):
        other = bmap.get(addr, ('<missing>', ''))
        same = other[0] == txt
        if not same:
            differs += 1
        if not (lo <= i < hi):
            continue
        mark = '  ' if same else '!!'
        print(f'{mark} {addr:08x}  ref: {txt}')
        if not same:
            print(f'   {addr:08x}  c++: {other[0]}')
            print(f'   {addr:08x}  ref bytes {raw}   c++ bytes {other[1]}')

    if differs == 0:
        print(f'body_diff: {start:#x}..{end:#x} identical '
              f'({(end - start)} bytes, {len(a_rows)} instructions)')
        return 0
    print(f'body_diff: {differs}/{len(a_rows)} instructions differ '
          f'over {start:#x}..{end:#x}')
    if first is None:
        print('body_diff: streams agree; the difference is in the literal pool')
    return 1


if __name__ == '__main__':
    sys.exit(main())
