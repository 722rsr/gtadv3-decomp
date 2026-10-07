#!/usr/bin/env python3
"""Extract printable ASCII strings from a binary (ROM triage helper).

Usage:
    python3 tools/findstr.py baserom.gba                 # whole file, min 6 chars
    python3 tools/findstr.py baserom.gba -l 8 -r MTO     # min 8 chars, filter 'MTO'
    python3 tools/findstr.py baserom.gba -e sjis         # also try Shift-JIS

Output: one line per string as `OFFSET  text` where OFFSET is both the file
offset and the VMA minus 0x08000000 (add 0x08000000 for the bus address).
"""
import argparse
import re

ASCII_RE = re.compile(rb"[\x20-\x7E]{%d,}")
# Shift-JIS ranges: half-width katakana + lead bytes 0x81-0x9F/0xE0-0xEF pairs
SJIS_RE = re.compile(rb"(?:[\x20-\x7E]|[\xA1-\xDF]|\x81-\x9F[\x40-\xFC]|\xE0-\xEF[\x40-\xFC]){%d,}")


def extract(data: bytes, minlen: int, encoding: str):
    if encoding == "ascii":
        rx = re.compile(rb"[\x20-\x7E]{%d,}" % minlen)
        return [(m.start(), m.group().decode("ascii")) for m in rx.finditer(data)]
    # sjis pass: keep only strings containing real multi-byte characters,
    # otherwise binary data decodes as noise (half-width katakana).
    out = []
    for m in re.finditer(rb"[\x20-\x7E\x81-\x9F\xE0-\xEF]{%d,}" % minlen, data):
        chunk = m.group()
        try:
            txt = chunk.decode("shift_jis")
        except UnicodeDecodeError:
            continue
        if any(ord(ch) > 0xFF or 0x2000 < ord(ch) < 0xFFEF and ord(ch) > 0x3000 for ch in txt):
            out.append((m.start(), txt))
    return out


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("rom")
    p.add_argument("-l", "--min-length", type=int, default=6)
    p.add_argument("-r", "--regex", help="only show strings matching this regex")
    p.add_argument("-e", "--encoding", choices=["ascii", "sjis"], default="ascii")
    p.add_argument("-o", "--output")
    args = p.parse_args()

    with open(args.rom, "rb") as f:
        data = f.read()
    results = extract(data, args.min_length, args.encoding)
    if args.regex:
        rx = re.compile(args.regex)
        results = [(off, s) for off, s in results if rx.search(s)]
    text = "\n".join(f"0x{off:06X}  {s!r}" for off, s in results)
    if args.output:
        with open(args.output, "w") as f:
            f.write(text + "\n")
        print(f"{len(results)} strings -> {args.output}")
    else:
        print(text)


if __name__ == "__main__":
    main()
