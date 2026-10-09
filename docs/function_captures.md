# Function-entry and return captures

`tools/function_trace.py` records a bounded set of calls in a fresh headless
mGBA session. Use it to investigate argument values, stack state, returned
registers, and changes in selected RAM regions. This is runtime evidence for
one input scenario, not a proof of equivalence or an exact-match promotion gate.

## Setup and a reproducible boot capture

Build the existing pinned mGBA 0.10.5 Python bindings:

```sh
sh tools/build_mgba_python.sh
```

Export the exact `PYTHONPATH` printed by the build script, then use
`build/pyenv/bin/python` below. On the current macOS/Python 3.11 build:

```sh
export PYTHONPATH="$PWD/build/mgba-build/python/lib.macosx-10.9-universal2-cpython-311"
mkdir -p build/function-trace
printf 'frame,keymask\n0,0\n' > build/function-trace/idle.csv
build/pyenv/bin/python tools/function_trace.py \
    baserom.gba build/function-trace/idle.csv \
    --entry 0x08001744 --calls 8 --frames 120 \
    --region IWRAM --out build/function-trace/boot.jsonl
```

The function at `0x08001744` is reached during idle boot. Use a new output
filename for each run; existing trace files are never overwritten. Captures
contain private ROM-derived state and belong under ignored `build/`.

`--entry` accepts a numeric ROM address, including an odd Thumb function
pointer. Thumb is the default; use `--mode arm` for an aligned ARM entry.
Resolve names against the ELF for the ROM actually being tested, for example
with `arm-none-eabi-nm`. Do not assume a rebuilt ROM retains the reference's
function addresses.

Repeat `--region` to select any of `IWRAM`, `EWRAM`, `VRAM`, `PAL`, `OAM`, or a
smaller range such as `--region state:0x030015f0:0x40`. Only canonical RAM ranges
are accepted; peripheral registers and memory mirrors are excluded. With no
region arguments, only the stack is captured. `--stack-bytes` defaults to 64,
is clipped at the end of the containing RAM region, and can be set to zero.
Each event declares its actual address and length, including the stack range.
Snapshots are direct memory copies, not bus reads that might affect hardware.

Limits are `--calls 8`, `--frames 600`, and `--seconds 60` by default. The call
limit counts captured entries, including recursion. Frame/time limits are
checked at frame boundaries; the time limit is approximate to one emulated
frame. Execution finishes the current frame after the last captured return.
A call that has not returned when a limit is reached is recorded as incomplete.
Exit status is 0 for at least one completed call with no unresolved calls,
1 for no returns/incomplete calls/runtime capture errors, and 2 for setup or
argument errors. Read the summary to see whether the requested call count was
reached or the run ended at a limit.

## Trace format and pairing

The JSONL stream begins with `gtadv3.function-trace.v1` metadata: ROM/input
SHA-256, mGBA version, limits, selected regions, and input timing. It then emits
`entry`, `return`, optional `incomplete`/`error`, and a final `summary` record.
Entry/return records include:

- `call_id`: pairs a captured entry with its return, including nested calls.
- `address`: breakpoint address, immediately before that instruction executes.
- `registers`: unsigned 32-bit r0 through r15; r13 is SP, r14 is LR.
  r15 is mGBA's raw pipeline PC, **not** the breakpoint address.
- `cpsr`, completed `frame`, and global emulated `cycles`.
- `memory`: named address/size/hex snapshots; return records add `elapsed_cycles`.

At entry the tool saves LR, SP and CPU mode and arms a hardware breakpoint at
the LR continuation. It pairs that breakpoint only when the original SP and
return instruction set/CPU mode are restored. A LIFO stack handles recursive
calls sharing a return address; tail-entered calls sharing both continuation
and SP can complete together. Hardware breakpoints do not patch ROM bytes.

This follows ordinary ABI calls to the selected entry. It does not reconstruct
an arbitrary call graph. An internal jump back to the entry can look like
another call. Nonlocal unwinds, unusual stack restoration, or LR used as data
can leave calls unresolved. The tool does not search past an unresolved inner
call to manufacture an outer return. Entry snapshots precede the prologue;
return snapshots precede the caller's continuation instruction.

Cycle differences include interrupts, DMA and nested work during the call.
Memory differences can likewise include interrupt/DMA effects, and cannot
identify which instruction wrote a byte. Unchanged bytes do not prove that no
access occurred. This version captures boundaries, not memory-access watchpoints.

## Reproducing scenarios and comparing captures

The CSV format is the same as `ramwatch.py`: explicit `frame,keymask` changes,
with a mask held until the next row (including zero to release all keys).
Row zero sets keys before execution; row N applies after completed frame N.
Runs start from reset with mGBA's built-in BIOS emulation and fresh save state;
no external save or savestate is loaded. Use identical input and emulator
configuration for comparisons.

For repeated runs of the same ROM and settings, JSONL should compare exactly:

```sh
cmp build/function-trace/boot-a.jsonl build/function-trace/boot-b.jsonl
```

For reference versus rebuilt ROM, expect metadata hashes and possibly code
addresses to differ. Compare matched scenarios/calls, r0-r3 at entry, return
registers, restored SP, and the selected memory ranges. Do not treat an equal
call ordinal alone as proof that the same logical invocation was observed.
`ramdiff.py` compares frame snapshot directories; it does not parse this JSONL.

## Frame snapshot coverage correction

Both newly generated `ramwatch.py gen` Lua scripts and `ramwatch.py run` now
capture IWRAM, EWRAM, VRAM, PAL and OAM in the existing v2 order (395264 bytes).
The TCP collector still accepts legacy two-region v1 captures and writes a
`capture.json` manifest identifying their actual coverage. Headless captures
also record ROM/input hashes. The legacy wire header remains compatible;
its total payload size distinguishes v1 from v2.

Use fresh output directories and regenerate old Lua scripts. Frame snapshot
names now use the actual completed frame, including the terminal snapshot;
older headless runs mislabeled the final frame as `frames - 1`. Row-zero key
input is now honored by both paths. Old captures are consequently not assumed
to align with newly generated ones. Start GUI captures from reset to use the
same input schedule as the headless driver.

## Verification

```sh
python3 tools/test_function_trace.py
FUNCTION_TRACE_INTEGRATION=1 build/pyenv/bin/python tools/test_function_trace.py
```

The optional integration test assembles an original diagnostic ROM with three
recursive calls. It checks call pairing, argument/return values, changing RAM,
repeatability, an instruction-change negative control, missed-target failure,
and terminal frame snapshot layout. Pure tests cover recursion/mode guards,
ARM/tail-continuation pairing, region validation and fragmented legacy/v2 TCP
payloads. When `lua` is installed, a mock-emulator test executes the generated
Lua and checks payload order, frame-zero keys and key release. The live GUI
socket path has not been exercised by that test.

Verified locally on 2026-10-08 with a fresh mGBA 0.10.5 build: two idle-boot
runs of the supplied reference ROM each captured eight calls and returns at
`0x08001744`, with all five regions, and produced identical JSONL. Initial
observed calls took 26 cycles. These results do not establish runtime
equivalence of any independently rebuilt ROM.

Implementation uses the pinned release's
[native debugger API](https://github.com/mgba-emu/mgba/blob/0.10.5/include/mgba/debugger/debugger.h)
and [ARM breakpoint implementation](https://github.com/mgba-emu/mgba/blob/0.10.5/src/arm/debugger/debugger.c).
The Python `NativeDebugger` convenience wrapper has outdated breakpoint method
signatures in this release; the driver uses the CFFI declarations directly.
No development-version Lua breakpoint APIs are required.
