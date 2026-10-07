#!/usr/bin/env python3
"""Compare two RAM-watch capture directories (tools/ramwatch.py).

  python3 tools/ramdiff.py caps/orig caps/rebuilt

Snapshot files are named snap_XXXXXXXX.bin (frame number) and contain
the raw body of one snapshot (header stripped):

  v1 legacy: IWRAM (32 KiB @0x03000000) + EWRAM (256 KiB @0x02000000)
             = 294912 B total
  v2 live:   IWRAM + EWRAM + VRAM (96 KiB @0x06000000)
             + PAL (1 KiB @0x05000000) + OAM (1 KiB @0x07000000)
             = 395264 B total

Reports:
  * frames present in only one capture
  * every diverging frame, the first N differing regions per reported
    frame mapped back to bus addresses, and a per-domain divergence
    histogram across all diverging frames
Exit status: 0 identical, 1 differences, 2 usage/IO errors.
"""
import sys
from pathlib import Path

REGIONS_V2 = [
    ("IWRAM", 0x8000, 0x03000000),
    ("EWRAM", 0x40000, 0x02000000),
    ("VRAM", 0x18000, 0x06000000),
    ("PAL", 0x400, 0x05000000),
    ("OAM", 0x400, 0x07000000),
]
LEGACY = [("IWRAM", 0x8000, 0x03000000), ("EWRAM", 0x40000, 0x02000000)]
MAX_REGIONS = 8
MAX_FRAME_LIST = 40


def layout(size):
    if size == LEGACY[0][1] + LEGACY[1][1]:
        return LEGACY
    total = sum(s for _n, s, _b in REGIONS_V2)
    if size == total:
        return REGIONS_V2
    return None


def load(d):
    out = {}
    for p in Path(d).glob("snap_*.bin"):
        frame = int(p.stem.split("_")[1])
        out[frame] = p.read_bytes()
    return out


def regions(diffs):
    runs = []
    for i in diffs:
        if runs and i == runs[-1][1] + 1:
            runs[-1][1] = i
        else:
            runs.append([i, i])
    return runs


def make_mapper(lay):
    bounds = []
    off = 0
    for name, size, base in lay:
        bounds.append((off, off + size, name, base))
        off += size

    def addr(o):
        for s, e, name, base in bounds:
            if s <= o < e:
                return f"{name} 0x{base + o - s:08X}"
        return f"off 0x{o:X}"

    def domain(o):
        for s, e, name, _base in bounds:
            if s <= o < e:
                return name
        return "??"

    return addr, domain


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(2)
    a, b = load(sys.argv[1]), load(sys.argv[2])
    only_a = sorted(set(a) - set(b))
    only_b = sorted(set(b) - set(a))
    if only_a:
        print(f"frames only in {sys.argv[1]}: {only_a[:8]}")
    if only_b:
        print(f"frames only in {sys.argv[2]}: {only_b[:8]}")

    common = sorted(set(a) & set(b))
    if not common:
        print("no common frames")
        sys.exit(1)

    lay = layout(len(a[common[0]]))
    if lay is None or len({len(x) for x in a.values()} |
                          {len(x) for x in b.values()}) != 1:
        print("snapshot files have inconsistent sizes; cannot compare")
        sys.exit(2)
    fmt = "v2 (IWRAM+EWRAM+VRAM+PAL+OAM)" if len(lay) > 2 else \
          "v1 legacy (IWRAM+EWRAM)"
    print(f"format {fmt}, {len(common)} common frames "
          f"({common[0]}..{common[-1]})")

    addr, domain_of = make_mapper(lay)
    bad_frames = []
    domain_hist = {}
    first_detail = None
    for f in common:
        da, db = a[f], b[f]
        if len(da) != len(db):
            print(f"frame {f}: size mismatch {len(da)} vs {len(db)}")
            bad_frames.append(f)
            continue
        if da == db:  # fast path: whole-payload compare before indexing
            continue
        diffs = [i for i in range(len(da)) if da[i] != db[i]]
        if not diffs:
            continue
        bad_frames.append(f)
        for s, e in regions(diffs):
            key = domain_of(s)
            domain_hist[key] = domain_hist.get(key, 0) + (e - s + 1)
        if first_detail is None:
            first_detail = (f, da, db, diffs)

    if not bad_frames:
        print(f"IDENTICAL across {len(common)} frames "
              f"({common[0]}..{common[-1]})")
        sys.exit(0)

    shown = bad_frames[:MAX_FRAME_LIST]
    tail = "" if len(bad_frames) <= MAX_FRAME_LIST else \
        f" … (+{len(bad_frames) - MAX_FRAME_LIST} more)"
    print(f"{len(bad_frames)} diverging frame(s): {shown}{tail}")
    hist = ", ".join(f"{k}: {v} B" for k, v in sorted(domain_hist.items(),
                                                      key=lambda kv: -kv[1]))
    print(f"divergent bytes by domain: {hist}")

    f, da, db, diffs = first_detail
    print(f"FIRST at frame {f}:")
    for s, e in regions(diffs)[:MAX_REGIONS]:
        vals_a = " ".join(f"{x:02X}" for x in da[s:min(e + 1, s + 16)])
        vals_b = " ".join(f"{x:02X}" for x in db[s:min(e + 1, s + 16)])
        print(f"  {addr(s)}-({e - s + 1} B)")
        print(f"    orig: {vals_a}")
        print(f"    rbld: {vals_b}")
    sys.exit(1)


if __name__ == "__main__":
    main()
