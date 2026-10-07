#!/bin/sh
# build_agbcc.sh -- provision the matching compiler into build/toolchains/agbcc/.
#
# Both compilers come from one pinned pret/agbcc tree, cloned in place so the
# probes that read source out of it keep working:
#
#   build/toolchains/agbcc/old_agbcc     the matching compiler (gcc `old` target)
#   build/toolchains/agbcc/agbcc         the comparison baseline
#   build/toolchains/agbcc/ginclude/     headers the tools pass via -I
#   build/toolchains/agbcc/gcc/, libgcc/, libc/   sources several probes read
#
# `old_agbcc` is the matching compiler (see docs/findings/compiler_split.md): -O2 -mthumb-interwork under pret/agbcc's
# stock-2.95 framing. `agbcc` is the baseline that split was measured against.
# Both are required -- tools/corpus_match_probe.py invokes `old_agbcc` while
# the era probes compare against `agbcc`.
#
# Requirements: git, a host C and C++ compiler, make, and an ARM assembler and
# archiver (binutils-arm-none-eabi -- `apt install build-essential
# binutils-arm-none-eabi git` on Debian/Ubuntu, `brew install gcc-arm-embedded`
# on macOS).
#
# The build is serial on purpose: pret/agbcc's own pinned commit is titled
# "Avoid multithreaded make where that doesn't work", and its build.sh runs
# `make -j1`. Expect several minutes.
#
# Usage:
#   tools/build_agbcc.sh           build if absent, otherwise report and exit
#   tools/build_agbcc.sh --force   rebuild from a fresh clone
#   tools/build_agbcc.sh --check   verify only; never build; non-zero if absent
#   tools/build_agbcc.sh --verify  also re-run the runtime-provenance oracle
#
# `--verify` also needs the oracle's second tree, which this script does not
# provision because it is not part of pret/agbcc: the vanilla GCC 2.95.3
# `lib1thumb.asm`, vendored in the repository at
# tools/third_party/gcc-2.95.3/lib1thumb.asm.  Should that file be missing, the
# agbcc tree is still checked in full, the baseline is reported SKIP, and
# verify_runtime below reports the case as INCOMPLETE (exit 3) rather than as a
# build failure.
#
# The pin is the tree the current compiler evidence was measured on. Bumping it
# requires rechecking docs/findings/compiler_split.md and the runtime fixture.
# Re-run `make matching-ready` after changing the pin.

set -e

AGBCC_REPO=https://github.com/pret/agbcc.git
AGBCC_COMMIT=da598c1d918402c42c0c0d7128ba14567f3175e9

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
DEST=$ROOT/build/toolchains/agbcc

force=0
check_only=0
verify=0
for arg in "$@"; do
    case $arg in
        --force) force=1 ;;
        --check) check_only=1 ;;
        --verify) verify=1 ;;
        -h|--help) awk 'NR>1 && /^set -e$/{exit} NR>1{print}' "$0"; exit 0 ;;
        *) echo "build_agbcc: unknown argument: $arg" >&2; exit 2 ;;
    esac
done

have_tree() {
    [ -x "$DEST/old_agbcc" ] && [ -x "$DEST/agbcc" ] && [ -d "$DEST/ginclude" ]
}

report() {
    if [ -d "$DEST/.git" ]; then
        head=$(git -C "$DEST" rev-parse HEAD 2>/dev/null || echo unknown)
        echo "build_agbcc: tree HEAD $head"
        if [ "$head" != "$AGBCC_COMMIT" ]; then
            echo "build_agbcc: WARNING: not the pinned commit $AGBCC_COMMIT" >&2
            echo "build_agbcc:          re-run with --force before trusting matching results" >&2
        fi
    else
        echo "build_agbcc: WARNING: $DEST is not a git checkout; cannot confirm the pin" >&2
    fi
    if command -v shasum >/dev/null 2>&1; then
        shasum -a 256 "$DEST/old_agbcc" "$DEST/agbcc"
    else
        sha256sum "$DEST/old_agbcc" "$DEST/agbcc"
    fi
}

# The oracle compares the ROM's runtime routines against two era toolchains:
# the pret/agbcc tree provisioned above, and the vanilla GCC 2.95.3 baseline
# that this script does not provision.  Without the baseline every agbcc routine
# is still checked and the baseline is reported SKIP, but `make era-runtime-probe`
# exits non-zero by design.  Say so plainly, and report the incomplete case with
# its own status, so `make: *** [era-runtime-probe] Error 1` is never misread as
# a broken toolchain build.
BASELINE=$ROOT/tools/third_party/gcc-2.95.3/lib1thumb.asm

verify_runtime() {
    if [ ! -f "$ROOT/baserom.gba" ]; then
        echo "build_agbcc: --verify skipped (baserom.gba is absent)" >&2
        return 0
    fi
    if [ ! -f "$BASELINE" ]; then
        echo "build_agbcc: NOTE: the oracle also needs the vendored GCC 2.95.3 baseline at" >&2
        echo "build_agbcc:       tools/third_party/gcc-2.95.3/lib1thumb.asm, which has been" >&2
        echo "build_agbcc:       deleted or moved.  Every agbcc routine is still checked; the" >&2
        echo "build_agbcc:       baseline is reported SKIP and the oracle exits non-zero." >&2
    fi
    echo "build_agbcc: re-running the runtime-provenance oracle (make era-runtime-probe)"
    if make -C "$ROOT" era-runtime-probe; then
        return 0
    fi
    if [ ! -f "$BASELINE" ]; then
        echo "build_agbcc: --verify INCOMPLETE: the agbcc tree was checked, the vendored" >&2
        echo "build_agbcc:          GCC 2.95.3 baseline was not (absent, see above).  The" >&2
        echo "build_agbcc:          toolchain built successfully; restore that file and re-run." >&2
        return 3
    fi
    return 1
}

if have_tree && [ "$force" -eq 0 ]; then
    echo "build_agbcc: already provisioned at $DEST (use --force to rebuild)"
    report
    [ "$verify" -eq 1 ] && verify_runtime
    exit 0
fi

if [ "$check_only" -eq 1 ]; then
    echo "build_agbcc: missing or incomplete at $DEST" >&2
    echo "build_agbcc: run tools/build_agbcc.sh to provision it" >&2
    exit 1
fi

for tool in git make python3 arm-none-eabi-as arm-none-eabi-ar; do
    command -v "$tool" >/dev/null 2>&1 || {
        echo "build_agbcc: required tool not found on PATH: $tool" >&2
        exit 1
    }
done

# Temporary compiler checkouts live under the ignored build/scratch/ directory.
scratch=$ROOT/build/scratch/agbcc-build
rm -rf "$scratch"
mkdir -p "$(dirname "$scratch")"

if [ "$force" -eq 1 ]; then
    rm -rf "$DEST"
fi

if [ -d "$DEST" ] && [ ! -d "$DEST/.git" ]; then
    echo "build_agbcc: $DEST exists but is not a git checkout; use --force to replace it" >&2
    exit 1
fi

if [ ! -d "$DEST/.git" ]; then
    echo "build_agbcc: cloning $AGBCC_REPO"
    git clone --quiet "$AGBCC_REPO" "$scratch"
    git -C "$scratch" checkout --quiet "$AGBCC_COMMIT"
    mkdir -p "$(dirname "$DEST")"
    mv "$scratch" "$DEST"
else
    echo "build_agbcc: reusing existing checkout at $DEST"
    head=$(git -C "$DEST" rev-parse HEAD 2>/dev/null || echo unknown)
    if [ "$head" != "$AGBCC_COMMIT" ]; then
        echo "build_agbcc: checking out pinned commit $AGBCC_COMMIT"
        git -C "$DEST" checkout --quiet "$AGBCC_COMMIT"
    fi
fi

echo "build_agbcc: building (serial; several minutes)"
(cd "$DEST" && ./build.sh)

have_tree || {
    echo "build_agbcc: build finished but old_agbcc/agbcc/ginclude are missing" >&2
    exit 1
}

echo "build_agbcc: provisioned $DEST"
report
[ "$verify" -eq 1 ] && verify_runtime
exit 0
