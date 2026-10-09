# Tools

Run Python utilities with `python3 tools/NAME.py --help` for their options.
Most analysis utilities use the standard library. The matching build needs
`pyelftools`, Clang, ARM GCC/binutils, and the pinned `old_agbcc`; emulator
and visualization tools have additional dependencies noted below.
Install the required Python package with `python3 -m pip install -r requirements.txt`.
Outputs belong under ignored `build/` or `assets/`.

## Build and verification

| Tool | Purpose |
| --- | --- |
| `build_agbcc.sh` | Provision pinned `pret/agbcc`; `--check` verifies installation and `--verify` checks runtime provenance. |
| `agbcc_c89_transform.py` | Generate compiler-compatible C copies under `build/`. |
| `agbcc_c89_shim.py` | Generate small source fixtures for the era-compiler probe. |
| `c89_equivalence.py` | Compare original/transformed modern ARM objects and relocations. |
| `corpus_match_probe.py` | Score compiled functions against their ROM spans, resolving relocations. |
| `promotion_screen.py` | Check candidate ownership, spans, symbols, and alignment before selecting C sections. |
| `match_c_slice.py` | Link selected C sections into the independent executable slice; includes an instruction-mutation negative control. |
| `independent_slice.py` | Verify reconstructed assembly for the complete executable region without ROM-backed code. |
| `data_regions.py` | Extract private editable palette fields, generate registered data inputs, and verify the assembled data tail. See [data integration](../docs/data-integration.md). |
| `ownership_map.py` | Inventory source/data ownership and check the independent-link boundary. |
| `apple_decls.py` | Check host declarations separately from ARM aliases. |
| `call_audit.py` | Audit C callees against the assembled symbol closure. |
| `export_audit.py` | Check aliases and external references affected by section replacement. |
| `label_census.py` | Compare VMA-shaped label names with assembled addresses. |
| `span_audit.py` | Detect typed-entry and replacement-span inconsistencies. |
| `proto_audit.py` | Find cross-translation-unit prototype disagreements. |
| `arity_audit.py` | Compare ROM argument use with C call sites. |
| `offset_audit.py` | Check work-area offsets against ROM evidence and known bounds. |
| `coverage.py` | Inventory known assembly function entries and their C owners. |

```sh
make toolchain
make matching-ready
make fast-check
```

The [verification guide](../docs/matching_workflow.md) explains what each gate
proves. The reference build's hash is distinct from the independent C goal.
On a fresh checkout, run `matching-ready` first: `fast-check` uses its
generated corpus report and assembled symbol closure.

## Compiler and matching diagnostics

These optional diagnostics are separate from the core verification gates.
Run `make experimental-check` for the strategy-tool regression suite.
`make fast-check` retains the core static audits and matching-assistant tests;
neither target replaces `make matching-ready`.

| Tool | Purpose |
| --- | --- |
| `era_compiler_probe.py` | Compare era compilers on reference functions. |
| `era_runtime_probe.py` | Compare handwritten compiler runtimes with the ROM and a negative control. |
| `era_corpus_scope.py` | Classify source compatibility diagnostics across translation units. |
| `derisk_probe.py` | Probe pool-free leaf instruction selection. |
| `derisk_pool_probe.py` | Probe literal-pool placement at real ROM addresses. |
| `match_context.py` | Gather a function's source, ROM instructions, callers, and fresh matching examples. |
| `match_families.py` | Index instruction/control-flow siblings; reject stale indexes. |
| `lift_scout.py` | Classify unmatched functions by diagnostic evidence. |
| `strategy_router.py` | Select applicable diagnostics from measured function properties. |
| `experiment_kit.py` | Shared candidate compiler, byte scorer, contracts, and output handling. |
| `compiler_microscope.py` | Compare compiler RTL passes for controlled source variants. |
| `fixtures/agbcc_recipes/` | Reproduce selected upstream matching motifs; see the [recipe cookbook](../docs/findings/agbcc_matching_recipes.md) for applicability and local results. |
| `recipe_miner.py` | Derive typed templates from freshly verified exact examples. |
| `leaf_synth.py` | Bounded synthesis for straight-line Thumb leaves. |
| `branch_synth.py` | Synthesize call-free conditional bodies with semantic checks before byte scoring. |
| `egraph_search.py` | Enumerate width-aware integer rewrites with checked side conditions. |
| `novelty_search.py` | Search distinct emitted instruction shapes using permitted source mutations. |
| `cross_rom_archaeology.py` | Compare privately supplied GT-family ROMs without importing foreign symbols into the link. |
| `run_permuter.py` | Prepare and run decomp-permuter for a function. |
| `test_matching_assist.py` | Regression checks for family retrieval and leaf synthesis. |
| `test_experiment_strategies.py` | Regression checks for matching strategies and their shared constraints. |

```sh
python3 tools/corpus_match_probe.py --function NAME --c89 --require-all --require-exact
python3 tools/match_families.py --build
python3 tools/match_context.py NAME
python3 tools/leaf_synth.py _08024B18 --return-type u32 --memory ordinary --budget 32
```

A generated draft is not selected source. Recheck its ABI, memory accesses,
and output in the owning translation unit and independent link.
Strategy tools' `--out` names the output directory itself; a rerun replaces
that directory's generated results.

## agbcc permuter

Install the optional dependencies in your Python environment, then prepare a
fresh scratch workspace:

```sh
python3 -m pip install -r tools/requirements-permuter.txt
python3 tools/run_permuter.py _08009FB8 --prepare-only
```

The wrapper provisions a pinned `WhenGryphonsFly/decomp-permuter-agbcc`, refreshes
`build-code/code.o`, and runs a focused C89 corpus probe. It generates `base.c`,
`target.o`, `compile.sh`, and `settings.toml` under `build/permuter/FUNCTION/`.
Aliases are resolved to their actual C body; friendly names with a known VMA
alias work too. Wrapper options can follow the function name. Pass fork options
after `--`; for a new workspace and a baseline-only fork check:

```sh
python3 tools/run_permuter.py _08009FB8 --work-dir build/permuter/car-check -- --debug
```

To start an actual search, omit `--prepare-only` and `--debug`, and pass options
such as `-- -j 4 --best-only`. Searches run until interrupted or the fork's stop
condition is met. The wrapper enables `--stack-diffs`. Existing nonempty scratch
directories are refused to protect manual `PERM_GENERAL` edits and search output;
resume using the printed fork command **from the workspace directory**. The fork
writes debug files into its current directory.

Each candidate compiles with pinned `old_agbcc -O2 -mthumb-interwork
-ffunction-sections` inside the original, preprocessed translation unit. Frozen
`before.c` and `after.c` preserve declarations, attributes, aliases, globals and
helper bodies. Parser-only declarations in `PERM_PRETEND` do not replace this
compiler context. The scorer receives only the target function's resolved bytes
at its ROM address, including its pool; ARM code/data mapping symbols are kept.
`metadata.json` records compiler/ROM/source identities and symbol resolution;
`baseline.json` records the initial score and byte comparison. Preparation fails
if the fork's source round-trip changes the baseline or a relocation is unresolved.

The fork's parser supports a subset of GNU C. Ordinary asm statements are retained
as literal pragmas; targets containing named-register asm declarations, GNU
statement expressions, or attributed function declarations are explicitly refused.
Other parser failures are reported rather than producing a runnable workspace.
Changing source or compiler inputs requires a fresh workspace; searches use a
frozen snapshot, not live source edits. Run `python3 tools/test_run_permuter.py`
for adapter regressions (also included in `make experimental-check`).

Permuter scores are advisory: branch targets and some operand differences are
normalized by the fork. A zero score does not prove byte identity. Search output
contains the candidate body; review it in its owning source file, run the focused
`corpus_match_probe.py --c89 --require-all --require-exact`, and then the independent
link/`matching-ready` gate before promotion. No candidate is applied automatically.
Scratch objects and ROM-derived bytes remain private build artifacts.

## Optional assembly viewers

`build_asm_differ.sh` fetches asm-differ; it does not install Python packages.
From the repository root, activate the virtual environment from the setup guide
and install the fetched package, including its declared dependencies:

```sh
. .venv/bin/activate
tools/build_asm_differ.sh
python3 -m pip install ./tools/asm-differ
python3 diff.py --help
```

This follows asm-differ's [package installation support](https://github.com/simonlindholm/asm-differ#usage).
Repeat the install command if you change Python environments, even when the
source checkout already exists. Run `make` before comparing the reference ROM
and `build/gtadv3.gba` through this wrapper. Those are reference-build comparisons;
use the matching probes above to evaluate candidate C functions.

### objdiff

Install [objdiff 3.8.2 or newer](https://github.com/encounter/objdiff/releases),
complete the compiler/Python/ROM setup in the root README, and open this repository
in the objdiff GUI. The checked-in `objdiff.json` lists known C functions grouped
by source file, including nonmatches. Select a unit and build it; source/header/
assembly changes trigger rebuilds. Objects are generated on demand, so opening a
fresh checkout does not require compiling the entire corpus.

`tools/objdiff_build.py` is the configured build command. It refreshes the assembled
symbol closure, runs the selected function's C89/old_agbcc probe, resolves calls
and pools at the ROM VMA, and publishes `target.o` and `candidate.o` under
`build/objdiff/ADDRESS/`. Each diagnostic ELF contains one sized Thumb function,
including its literal pool and alignment tail, with ARM code/data mapping symbols.
The target bytes come from the hash-checked private ROM; candidate bytes come
from the current C translation unit. No permuter package is required.

Regenerate the config when function names, owners or boundaries change:

```sh
make objdiff-config
python3 tools/objdiff_build.py --check-config
```

The CLI's one-shot diff reads existing objects; invoke the configured build first:

```sh
python3 tools/objdiff_build.py build/objdiff/08009fb8/candidate.o
objdiff-cli diff -p . -u car_tick_helpers/_08009FB8 -o build/objdiff/car-diff.json
```

`comparison.json` beside the pair records the probe result and a strict resolved-byte
comparison. Unresolved relocations, compiler failures, or changed source ownership
fail the build and remove stale viewer objects. Not every inventory entry is
necessarily compilable; such failures remain visible instead of being represented
by reference bytes. The watcher excludes generated comparison/probe output to avoid
rebuild loops.

Objdiff percentages are diagnostic, not ownership or independent-link acceptance.
These ROM-addressed ELF views have resolved relocations; inspect the accompanying
probe call/pool records when investigating symbol targets. The authoritative
decomp.dev report remains separately generated by `make progress`, and promotions
still require the matching workflow above. Adapter regressions run with
`python3 tools/test_objdiff_build.py` or `make experimental-check`.

## ROM and asset analysis

| Tool | Purpose |
| --- | --- |
| `lz77.py` | Scan, decompress, and compress BIOS LZ77 data. |
| `ptrscan.py` | Find pointer-dense ranges and contiguous pointer tables. |
| `findstr.py` | Extract ASCII or Shift-JIS strings with offsets. |
| `xref.py` | Find Thumb calls and literal references to ROM/RAM addresses. |
| `refaudit.py` | Classify reference hits, distinguishing pointers from numeric data coincidences. |
| `mto_dump.py` | Inventory the MTO directory at `0x08060378`. |
| `track_dump.py` | Inventory course/resource records at `0x080CE020`. |
| `extract_assets.py` | Extract track resources, compressed data, sound tables, and PCM samples. |
| `asset_pipeline.py` | Generate editable data for supported families and verify complete owned spans. |
| `mini_dis.py` | Minimal Thumb decoder; cross-check significant results with ARM objdump. |
| `objdump2gas.py` | Convert Thumb listings to source assembly with pools and labels. |
| `objdump2gas_mixed.py` | Convert mixed ARM/Thumb spans. |
| `mk_passthrough.py` | Generate include/range manifests from file-offset symbol lists. |
| `treemap.py` | Render function ownership as PNG and interactive HTML; requires Pillow for PNG output. |
| `build_gbadisasm.sh`, `gbadisasm_patch.py` | Build and patch gbadisasm. |
| `build_asm_differ.sh` | Provision assembly comparison tooling. |
| `build_permuter.sh` | Provision decomp-permuter. |
| `build_gbagfx.sh`, `build_gbafix.sh` | Provision graphics and cartridge-header utilities. |

ROM VMA equals file offset plus `0x08000000`. Check a tool's address convention
before passing arguments. Literal coincidences alone do not prove a pointer.
Keep ROM-derived asset outputs private.

Gbadisasm can misclassify literal pools and assert on overlapping function
ranges. Small spans with explicit `arm_label`/`thumb_label` seeds are easier
to verify. Headerless slices need `-s` and the correct `-l` base VMA.
Objdump prints Thumb load/store displacements in decimal.

## Runtime comparisons

`build_mgba_python.sh` builds libmGBA bindings under `build/` and prints the
Python environment settings. `mgba_python_shim.c` supplies binding support.
For registers and selected memory at individual calls, use
[`function_trace.py`](function_trace.py); the
[function capture guide](../docs/function_captures.md) includes setup, an
idle-boot example, return-pairing limits, and optional live-emulator tests.
Use `ramwatch.py run` with identical input for both ROMs, then `ramdiff.py`
to compare captures. The capture format includes IWRAM, EWRAM, VRAM, palette
RAM, and OAM.

```sh
python3 tools/ramwatch.py run baserom.gba inputs.csv --outdir build/caps/reference --interval 30 --frames 3600
python3 tools/ramdiff.py build/caps/reference build/caps/rebuilt
```

The rebuilt capture must come from the actual independent ROM under test.
Runtime comparisons diagnose behavior; the complete-ROM hash remains the
final byte-identity gate.

## Third-party source

[`third_party/gcc-2.95.3/`](third_party/gcc-2.95.3/README.md) contains the
runtime source used by `era_runtime_probe.py`. Its GPL license and notices
remain intact and separate from CC0. See [NOTICE](../NOTICE).
