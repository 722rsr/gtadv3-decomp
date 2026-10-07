#!/usr/bin/env python3
"""Dump and classify the 'MTO' resource directory at ROM 0x08060378.

Directory layout:
    struct mto_entry { u8 magic[4];   // "MTO\0" = 0x004F544D LE
                       u32 file_off;  // payload file offset
                       u32 size;      };  // payload byte size

Each payload is classified:
    LZ77     first byte 0x10 and decompresses cleanly to its header size
    THUMB    starts with a Thumb push prologue (0xB5xx / 0xB4xx)
    ASCII    >=90% printable bytes
    ZERO     all zero bytes
    BLOB     anything else
    BADMAGIC/OOB  entry itself is invalid

Stdlib only. Writes a TSV inventory; stdout gets the summary.
"""
import argparse
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lz77 import decompress  # noqa: E402

DIR_OFF = 0x08060378 - 0x08000000
STRIDE = 12
N_ANNOUNCED = 210


def classify(payload: bytes) -> str:
    if not any(payload):
        return "ZERO"
    if payload[0] == 0x10 and len(payload) >= 4:
        want = int.from_bytes(payload[1:4], "little")
        try:
            got = decompress(payload, 0)
            if len(got) == want:
                return "LZ77"
        except Exception:
            pass
    if len(payload) >= 2:
        hw = int.from_bytes(payload[0:2], "little")
        if hw & 0xFF00 in (0xB500, 0xB400):
            return "THUMB"
    printable = sum(1 for b in payload if 32 <= b < 127 or b in (9, 10, 13))
    if printable >= len(payload) * 0.9:
        return "ASCII"
    return "BLOB"


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("rom", nargs="?", default="baserom.gba")
    ap.add_argument("-o", "--out", default="docs/data/mto_entries.txt",
                    help="output TSV path (default docs/data/mto_entries.txt)")
    ap.add_argument("--dir-offset", type=lambda x: int(x, 0), default=DIR_OFF,
                    help="directory file offset (default 0x60378)")
    args = ap.parse_args()

    data = open(args.rom, "rb").read()
    rows = []
    counts = {}
    sizes = []
    n_bad_magic = n_oob = 0
    last_valid = None
    for i in range(N_ANNOUNCED):
        e = args.dir_offset + i * STRIDE
        magic = data[e:e + 4]
        off = int.from_bytes(data[e + 4:e + 8], "little")
        size = int.from_bytes(data[e + 8:e + 12], "little")
        if magic != b"MTO\x00":
            cls, n_bad_magic = "BADMAGIC", n_bad_magic + 1
        elif off + size > len(data):
            cls, n_oob = "OOB", n_oob + 1
        else:
            cls = classify(data[off:off + size])
            sizes.append(size)
            last_valid = i
        counts[cls] = counts.get(cls, 0) + 1
        rows.append((i, off, size, cls))

    with open(args.out, "w") as f:
        f.write("# MTO resource directory dump — baserom.gba\n")
        f.write(f"# dir @ file 0x{args.dir_offset:X} "
                f"(VMA 0x{args.dir_offset + 0x08000000:X}), "
                f"{STRIDE}-byte entries: magic 'MTO\\0', u32 file_off, u32 size\n"
                f"# announced={N_ANNOUNCED} valid={len(sizes)} "
                f"badmagic={n_bad_magic} oob={n_oob} last_valid_idx={last_valid}\n")
        f.write("# idx\toff\tvma\tsize\tclass\n")
        for i, off, size, cls in rows:
            f.write(f"{i}\t0x{off:X}\t0x{off + 0x08000000:X}\t"
                    f"{size}\t{cls}\n")

    print(f"wrote {args.out}")
    print(f"announced={N_ANNOUNCED} valid={len(sizes)} "
          f"badmagic={n_bad_magic} oob={n_oob} last_valid_idx={last_valid}")
    if sizes:
        print(f"size min={min(sizes)} max={max(sizes)} "
              f"sum=0x{sum(sizes):X} avg={sum(sizes)/len(sizes):.1f}")
    print("classes:", " ".join(f"{k}={v}" for k, v in sorted(counts.items())))


if __name__ == "__main__":
    main()
