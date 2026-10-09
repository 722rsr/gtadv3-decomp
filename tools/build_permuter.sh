#!/bin/sh
# Fetch WhenGryphonsFly/decomp-permuter-agbcc (GBA-tuned permuter for agbcc).
# Usage: tools/build_permuter.sh [dest_dir]
set -e

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
DEST="${1:-$ROOT/build/toolchains/decomp-permuter-agbcc}"
REV=1f7ef872b12f54db7678ff00e0346abc015410ae
if [ -d "$DEST/.git" ]; then
    if [ "$(git -C "$DEST" rev-parse HEAD)" != "$REV" ]; then
        echo "decomp-permuter-agbcc: expected $REV; existing checkout left unchanged: $DEST" >&2
        exit 1
    fi
    echo "decomp-permuter-agbcc $REV already present at $DEST"
    exit 0
fi

mkdir -p "$(dirname "$DEST")"
git clone --depth 1 https://github.com/WhenGryphonsFly/decomp-permuter-agbcc "$DEST"
git -C "$DEST" fetch --depth 1 origin "$REV"
git -C "$DEST" checkout --detach "$REV"
echo "fetched $DEST"
