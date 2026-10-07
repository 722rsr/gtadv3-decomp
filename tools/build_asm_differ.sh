#!/bin/sh
# Fetch asm-differ (side-by-side assembly diffing for GBA Thumb).
# Usage: tools/build_asm_differ.sh [dest_dir]
set -e

DEST="${1:-tools/asm-differ}"
if [ -d "$DEST/.git" ]; then
    echo "asm-differ already present at $DEST"
else
    mkdir -p "$DEST"
    git clone --depth 1 https://github.com/simonlindholm/asm-differ "$DEST"
    echo "fetched $DEST"
fi

echo "Activate your Python virtual environment, then install asm-differ and its dependencies:"
printf '  python3 -m pip install "%s"\n' "$DEST"
