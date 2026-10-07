#!/usr/bin/env python3
"""Dump the MTO track/resource package at ROM 0x080CE020.

Package layout:
  dir header: {u16 magic=12345, u16 pad, u32 count[11], u32 offset[total]}
  record:     {u16 group, u16 index, u32 size, payload[size]}   chained sequentially
  seeker fn:  _08006590 returns dir + align4(offset[k]) + 8  (= payload address)

Outputs (default under docs/data/):
  track_resources.txt   full record inventory (all groups)
  course_headers.txt    per-course group-0 header field map
"""
import argparse
import struct
import sys
from pathlib import Path

DIR_OFF = 0x0CE020          # file offset of package directory (VMA 0x080CE020)
NGROUPS = 11


def read_package(rom):
    magic, pad = struct.unpack_from('<HH', rom, DIR_OFF)
    if magic != 12345:
        sys.exit(f'bad magic {magic} at {DIR_OFF:#x}')
    counts = [struct.unpack_from('<I', rom, DIR_OFF + 8 + 4 * k)[0] for k in range(NGROUPS)]
    total = sum(counts)
    offsets = struct.unpack_from(f'<{total}I', rom, DIR_OFF + 8 + 4 * NGROUPS)
    # walk the sequential record chain starting at first payload - 8
    first = DIR_OFF + ((offsets[0] >> 2) << 2) + 8 - 8
    recs = []
    pos = first
    while True:
        g, i, size = struct.unpack_from('<HHI', rom, pos)
        recs.append((pos, g, i, size, pos + 8))
        if len(recs) == total:
            break
        pos += 8 + size
    return counts, offsets, recs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('rom')
    ap.add_argument('-o', '--outdir', default='docs/data')
    a = ap.parse_args()
    rom = Path(a.rom).read_bytes()
    outdir = Path(a.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    counts, offsets, recs = read_package(rom)

    lines = ['# MTO resource package — baserom.gba',
             f'# dir @ file {DIR_OFF:#x} (VMA {DIR_OFF + 0x08000000:#x}),'
             f' magic=12345, groups={NGROUPS}, records={sum(counts)}',
             '# group counts: ' + ' '.join(map(str, counts)),
             '# chain start ' + f'{recs[0][0]:#x}' + '  chain end '
             + f'{recs[-1][0] + 8 + recs[-1][3]:#x}',
             '# idx\tfile_off\tvma\tgroup\tindex\tsize\tlz77_dec']
    n = 0
    for (off, g, i, size, pay) in recs:
        dec = ''
        if rom[pay] == 0x10:
            dec = int.from_bytes(rom[pay + 1:pay + 4], 'little')
            dec = f'{dec:#x}'
        lines.append(f'{n}\t{off:#x}\t{off + 0x08000000:#x}\t{g}\t{i}\t{size:#x}\t{dec}')
        n += 1
    (outdir / 'track_resources.txt').write_text('\n'.join(lines) + '\n')

    lines = ['# Group-0 course header map — all 63 courses',
             '# fields are payload-relative; sections:',
             '#   A@+28 stride20*nA  B@.. stride12*nB  C@.. stride12*nC  D@.. stride8*nD',
             '#   tail = c5*88+52 bytes (52B block + c5 x 88B entries)',
             '# crs\tpayload_off\tmagic\tnA\tnB\tnC\tnD\tc5\th0C\th0E\th10\th12\th14\th16\tmapW\tmapH\tparsed\ttail']
    for ci in range(counts[0]):
        p = recs[ci][4]
        sz = recs[ci][3]
        hw = struct.unpack_from('<16H', rom, p)
        na, nb, nc, nd, c5 = hw[1], hw[2], hw[3], hw[4], hw[5]
        parsed = 28 + na * 20 + nb * 12 + nc * 12 + nd * 8
        lines.append(f'{ci}\t{p:#x}\t{hw[0]:#x}\t{na}\t{nb}\t{nc}\t{nd}\t{c5}'
                     f'\t{hw[6]}\t{hw[7]}\t{hw[8]}\t{hw[9]}\t{hw[10]}\t{hw[11]}'
                     f'\t{hw[10]}\t{hw[11]}\t{parsed}\t{sz - 8 - parsed}')
    (outdir / 'course_headers.txt').write_text('\n'.join(lines) + '\n')
    print(f'wrote {outdir}/track_resources.txt ({len(recs)} records), '
          f'{outdir}/course_headers.txt ({counts[0]} courses)')


if __name__ == '__main__':
    main()
