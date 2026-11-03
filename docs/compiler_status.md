# Compiler and source verification

The matching compiler is `build/toolchains/agbcc/old_agbcc`, provisioned from
the pinned `pret/agbcc` revision by `make toolchain`. The normal flags are
`-O2 -mthumb-interwork -ffunction-sections`. The companion `agbcc` binary is
a comparison tool; its framing of conditional leaves differs from the ROM.
See [compiler behavior](findings/compiler_split.md).

## Runtime provenance

The ROM matches all 724 bytes of the following Thumb runtime routines.
VMAs are shown below; file offsets are VMA minus `0x08000000`.

| Routine | ROM VMA | Bytes |
| --- | --- | ---: |
| `_call_via_rX` | `0x0802DDC8` | 60 |
| `__udivsi3` | `0x0802DF6C` | 120 |
| `__umodsi3` | `0x0802DFE4` | 192 |
| `__divsi3` | `0x0802DE04` | 146 |
| `__modsi3` | `0x0802DE9C` | 206 |

These routines are handwritten runtime assembly, not compiler output. They
support runtime provenance independently of reconstructed C source shape.
The negative control checks that vanilla GCC 2.95.3's mode-switching
`_interwork_call_via_rX` veneer is absent. Both runtime variants share the
division routines, so those alone do not distinguish the toolchains.

Reproduce with `make era-runtime-probe`. The recorded comparison is
[`data/era_runtime_probe.json`](data/era_runtime_probe.json).
Third-party source and its license are documented in
[`tools/third_party/gcc-2.95.3/README.md`](../tools/third_party/gcc-2.95.3/README.md).

## Single compiler and source provenance

The project has one C compiler: `old_agbcc`. Every maintained C translation
unit is compiled by it. The build does not invoke ARMCC or any second C
compiler; `asm/` is assembled by GNU `as`. The runtime probe identifies the
linked compiler runtime as the pinned agbcc variant: the division and
interworking routines match the pret/agbcc `lib1thumb.asm`, and vanilla GCC
2.95.3's mode-switching `_interwork_call_via_rX` veneer is absent.

Some bodies are not agbcc C output. They contain `tst`, `mov pc, lr`,
`pop {pc}`, a bare `push {rN}` with a `bl`, an `.arm` island, `swi`, or a
fall-through tail, and they stay `asm` or `partial` (C with an explicit
instruction splice). Those bodies are hand-written assembly or splices, not
evidence of a second compiler, and they must not trigger a per-body compiler
switch. `tools/backlog_census.py --lift-screen` is the pre-lift gate; the
marker list and the six `tst` bodies are in
[matching workflow](matching_workflow.md#bodies-that-are-not-agbcc-c-output-asmsplice-only).

Older notes use "ARMCC" as shorthand for ARM-style instruction idioms (the
high-register save dance, `tst`, `mov pc, lr` returns). It is not a measured
provenance claim. Use "non-agbcc assembly" unless the body has been compared
against a specific toolchain.

A proposal to change the C compiler requires the AGENTS.md exception:
concrete provenance evidence, agreement on scope, and a full corpus regression.
One body that cannot be produced by `old_agbcc` is not evidence, and a compiler
patch that only helps one body is a red flag.

## Source language and matching

Maintained C is C89 in the dialect accepted by the pinned `old_agbcc`. There is
no C99-to-C89 rewrite step between `src/` and the compiler: the checked-in
translation unit is the compiler input. GNU extensions that the original build
already relies on -- `__asm__` statements, `__attribute__`, named register
variables, and `//` comments -- remain part of that dialect; the language gate
is what `old_agbcc` accepts, not ISO C89 with extensions disabled.

`tools/c89_check.py` is the source-language gate: it preprocesses and compiles
every `src/*.c` directly with `old_agbcc` and reports the compiler's diagnostic
for any C99 construct. Its self-test proves the gate accepts a C89 fixture and
rejects a `for`-declaration. `tools/c_source.py` holds the comment/string mask
and delimiter helpers shared by source-aware tools.

`tools/corpus_match_probe.py` compiles functions, resolves calls and pool
relocations at their ROM addresses, and compares the resulting spans. The
independent slice link verifies selected C in its actual placement, including
alignment and exports. A successful isolated probe is not sufficient to claim
independent source ownership.

The [verification guide](matching_workflow.md) describes the checks.
The [completion criteria](decompilation-roadmap.md) distinguish this executable
slice from a complete independent C ROM.

## Compiler diagnostics

- `tools/era_compiler_probe.py`: era-toolchain comparisons at fixed ROM spans.
- `tools/derisk_probe.py`: pool-free instruction selection comparisons.
- `tools/derisk_pool_probe.py`: literal-pool placement at actual ROM addresses.
- `tools/compiler_microscope.py`: RTL and register-allocation diagnostics.
