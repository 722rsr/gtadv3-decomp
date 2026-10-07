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

## Source generation and matching

`tools/agbcc_c89_transform.py` writes compiler-compatible copies under `build/`.
Maintained C source is not rewritten by the build. `tools/c89_equivalence.py`
compares modern ARM compilations of the original and transformed source,
including loadable sections and symbolic relocations. This is an empirical
equivalence check, not a general proof of text-transform semantics.

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
