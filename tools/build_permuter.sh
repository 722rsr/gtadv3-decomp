#!/bin/sh
# Fetch WhenGryphonsFly/decomp-permuter-agbcc (GBA-tuned permuter for agbcc).
# Usage: tools/build_permuter.sh [dest_dir]
set -e

DEST="${1:-build/toolchains/decomp-permuter-agbcc}"
if [ -d "$DEST/.git" ]; then
    echo "decomp-permuter-agbcc already present at $DEST"
    exit 0
fi

mkdir -p "$(dirname "$DEST")"
git clone --depth 1 https://github.com/WhenGryphonsFly/decomp-permuter-agbcc "$DEST"
echo "fetched $DEST"
