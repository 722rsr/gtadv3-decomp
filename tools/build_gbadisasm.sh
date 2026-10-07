#!/bin/sh
# Fetch and build gbadisasm (GBA -> gas disassembler used by asm/ workflow).
# Usage: tools/build_gbadisasm.sh [dest_dir]   (default: tools/gbadisasm)
#
# The binary is NOT committed (repo rule: no binaries in git). Run this
# once per machine before converting new code regions:
#   tools/gbadisasm/gbadisasm <rom|slice> -l 0x8000000 -c <config>
set -e
DEST="${1:-tools/gbadisasm}"
if [ -x "$DEST/gbadisasm" ]; then
    echo "gbadisasm already built at $DEST/gbadisasm"
    exit 0
fi
mkdir -p "$DEST"
git clone --depth 1 https://github.com/jiangzhengwenjz/gbadisasm "$DEST/src"
python3 "$(dirname "$0")/gbadisasm_patch.py" "$DEST/src"
make -C "$DEST/src" >/dev/null
cp "$DEST/src/gbadisasm" "$DEST/gbadisasm"
echo "built $DEST/gbadisasm"
