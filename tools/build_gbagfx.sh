#!/bin/sh
# Fetch and build gbagfx (GBA graphics and compression converter from pret).
# Usage: tools/build_gbagfx.sh [dest_dir]   (default: tools/gbagfx)
set -e

DEST="${1:-tools/gbagfx}"
if [ -x "$DEST/gbagfx" ]; then
    echo "gbagfx already built at $DEST/gbagfx"
    exit 0
fi

mkdir -p "$DEST/src"
curl -sL https://github.com/pret/pokeemerald/archive/refs/heads/master.tar.gz | \
    tar -xz --strip-components=3 -C "$DEST/src" "pokeemerald-master/tools/gbagfx"

EXTRA_CFLAGS=""
EXTRA_LDFLAGS=""
if [ -d "/opt/homebrew/include" ]; then
    EXTRA_CFLAGS="-I/opt/homebrew/include"
fi
if [ -d "/opt/homebrew/lib" ]; then
    EXTRA_LDFLAGS="-L/opt/homebrew/lib"
fi

make -e CFLAGS="-Wall -Wextra -Werror -Wno-sign-compare -std=c11 -O2 -DPNG_SKIP_SETJMP_CHECK $EXTRA_CFLAGS" \
     LDFLAGS="$EXTRA_LDFLAGS" -C "$DEST/src" >/dev/null

cp "$DEST/src/gbagfx" "$DEST/gbagfx"
echo "built $DEST/gbagfx"
