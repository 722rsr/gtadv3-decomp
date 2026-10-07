#!/bin/sh
# Fetch and build gbafix (GBA ROM header and checksum fixer).
# Usage: tools/build_gbafix.sh [dest_dir]   (default: tools/gbafix)
set -e

DEST="${1:-tools/gbafix}"
if [ -x "$DEST/gbafix" ]; then
    echo "gbafix already built at $DEST/gbafix"
    exit 0
fi

mkdir -p "$DEST/src"
git clone --depth 1 https://github.com/devkitPro/gba-tools "$DEST/src"
gcc -O2 "$DEST/src/src/gbafix.c" -o "$DEST/gbafix"
echo "built $DEST/gbafix"
