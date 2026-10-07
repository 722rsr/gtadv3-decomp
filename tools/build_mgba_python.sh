#!/bin/sh
# Build libmgba + Python bindings headless (for `ramwatch.py run`).
#
# Includes two compatibility fixes for macOS and current toolchains:
#   1. cmake >= 4 needs -DCMAKE_POLICY_VERSION_MINIMUM=3.5 for mgba 0.10.x
#   2. mgba 0.10.5 packaging bug: EReaderScan/EReaderAnchorList/BlockList
#      are #ifdef USE_FFMPEG in src but declared unconditionally in the
#      internal header -> cffi emits refs that break dlopen when built
#      without FFmpeg. Fixed by linking tools/mgba_python_shim.c (abort
#      stubs; the API is unreachable during emulation) into the extension.
#
# Result: python package under
#   build/mgba-build/python/lib.macosx-*-cpython-*/
# Run:  export PYTHONPATH="$PWD"/build/mgba-build/python/lib.*-cpython-*/
#   (exact path is printed at the end)
set -e
cd "$(dirname "$0")/.."
VER=0.10.5

[ -d build/mgba-src ] || git clone --depth 1 --branch "$VER" --recursive \
    https://github.com/mgba-emu/mgba.git build/mgba-src

cp tools/mgba_python_shim.c build/mgba-src/src/platform/python/ereadershim.c
B=build/mgba-src/src/platform/python/_builder.py
grep -q ereadershim.c "$B" || sed -i '' \
    's/"vfs-py.c", "core.c", "log.c", "sio.c"/"vfs-py.c", "core.c", "log.c", "sio.c", "ereadershim.c"/' \
    "$B"

if [ ! -x build/pyenv/bin/python ]; then
    python3 -m venv build/pyenv
    build/pyenv/bin/pip -q install setuptools cffi cached-property
fi

PYINC=$(build/pyenv/bin/python -c "import sysconfig;print(sysconfig.get_paths()['include'])")
PYLIB=$(build/pyenv/bin/python -c "import sysconfig;print(sysconfig.get_config_var('LIBDIR'))")

cmake -S build/mgba-src -B build/mgba-build \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DBUILD_PYTHON=ON -DBUILD_QT=OFF -DBUILD_SDL=OFF -DBUILD_SUITE=OFF \
    -DUSE_FFMPEG=OFF -DUSE_MINIUPNPC=OFF -DUSE_ELF=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DPYTHON_EXECUTABLE="$PWD/build/pyenv/bin/python" \
    -DPython_EXECUTABLE="$PWD/build/pyenv/bin/python" \
    -DPYTHON_INCLUDE_DIR="$PYINC" \
    -DPYTHON_LIBRARY="$PYLIB/libpython3.11.dylib"
cmake --build build/mgba-build -j "$(sysctl -n hw.ncpu)"

MOD=$(echo build/mgba-build/python/lib.*-cpython-*/)
echo
echo "OK. Use:"
echo "  export PYTHONPATH=\"\$PWD/$MOD\""
