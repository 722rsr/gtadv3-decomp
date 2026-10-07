#!/usr/bin/env python3
"""ROM pointer scanner / density mapper for GBA ROMs.

Finds word-aligned 32-bit values that look like ROM addresses (0x08xxxxxx).
Useful for locating literal pools (=> nearby Thumb code), pointer tables
(=> resource directories), and mapping code vs data regions.

Subcommands:
    density ROM            Per-block percentage of words that are ROM pointers.
                           -b/--block sets block size (default 0x10000).
    table ROM              Dump contiguous runs of >= N consecutive ROM
                           pointers (-n, default 8) — likely pointer tables.

Examples:
    python3 tools/ptrscan.py density baserom.gba -b 0x20000
    python3 tools/ptrscan.py table baserom.gba -o docs/data/ptr_tables.txt
"""
import argparse
import struct


def iter_words(data):
    for off in range(0, len(data) - 3, 4):
        yield off, struct.unpack_from("<I", data, off)[0]


def cmd_density(args):
    with open(args.rom, "rb") as f:
        data = f.read()
    step = args.block
    print(f"block      romptr%")
    for base in range(0, len(data) - step, step):
        blk = data[base : base + step]
        total = len(blk) // 4
        hits = sum(
            1
            for _, w in iter_words(blk)
            if 0x08000000 <= w < 0x08800000
        )
        pct = 100.0 * hits / total
        bar = "#" * int(pct // 2)
        print(f"{base:08X}  {pct:5.1f}  {bar}")


def cmd_table(args):
    with open(args.rom, "rb") as f:
        data = f.read()
    runs = []
    run_start = None
    run_len = 0
    for off, w in iter_words(data):
        if 0x08000000 <= w < 0x08800000:
            if run_len == 0:
                run_start = off
            run_len += 1
        else:
            if run_len >= args.min:
                runs.append((run_start, run_len))
            run_len = 0
    if run_len >= args.min:
        runs.append((run_start, run_len))
    lines = ["# Contiguous ROM-pointer runs (candidate tables/literal pools)", "# offset  count"]
    lines += [f"0x{off:06X}  {n}" for off, n in runs]
    text = "\n".join(lines) + "\n"
    if args.output:
        with open(args.output, "w") as f:
            f.write(text)
        print(f"{len(runs)} runs -> {args.output}")
    else:
        print(text)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)

    d = sub.add_parser("density", help="pointer-density map per block")
    d.add_argument("rom")
    d.add_argument("-b", "--block", type=lambda x: int(x, 0), default=0x10000)

    t = sub.add_parser("table", help="dump contiguous pointer runs")
    t.add_argument("rom")
    t.add_argument("-n", "--min", type=int, default=8)
    t.add_argument("-o", "--output")

    args = p.parse_args()
    {"density": cmd_density, "table": cmd_table}[args.cmd](args)


if __name__ == "__main__":
    main()
