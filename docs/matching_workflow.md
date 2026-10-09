# Build verification and byte matching

The matching build uses the pinned `old_agbcc` compiler with
`-O2 -mthumb-interwork -ffunction-sections`. See [setup](../README.md),
[compiler behavior](findings/compiler_split.md), and
[completion criteria](decompilation-roadmap.md).

## Verification commands

```sh
make toolchain
make matching-ready
git diff --check
```

`matching-ready` checks the reference ROM hash, generates compiler-compatible
C copies, checks original/transformed object equivalence, scores the C corpus,
and links the selected C functions into the independent executable slice.
It also runs an instruction-mutation negative control, runtime-provenance
comparison, and source/data ownership checks.

Generated source, objects, reports, captures, and extracted assets belong in
ignored output directories. Never commit the reference ROM or derived binaries.
The checked-in inventories contain metadata and hashes, not replacement code.

For a focused comparison:

```sh
python3 tools/corpus_match_probe.py --function NAME --c89 --require-all --require-exact
make fast-check
```

`fast-check` runs tool regressions and static audits. It does not replace the
independent link. On a fresh checkout, first run `matching-ready` to generate
the corpus report and assembled symbol closure used by these audits.
`make matching-slice` checks the selected C slice and its
negative control. Assembly edits also require `make independent-slice` and
`make ownership-map` to refresh source hashes before auditing the inventories.
Optional search and synthesis tools have a separate regression target,
`make experimental-check`; it is not part of `fast-check` or the ROM gate.

## Function ownership and relocation

Read the ROM instructions to establish parameter registers, return behavior,
access widths, and control flow. A source name or existing prototype is not
ABI evidence. File offset equals ROM VMA minus `0x08000000`.

Each ROM entry has one canonical C body. Alias spellings of that body do not
create separate ownership. The selected entries and their source paths live in
`tools/matching_slice_functions.json`.

Resolve symbols from the assembled include closure rooted at `asm/code.s`.
Parsing an address out of a VMA-shaped name does not establish that a symbol
exists. Inspect `arm-none-eabi-nm -n build-code/code.o` and the exact C spelling
in `src/`; preserve required ARM aliases and host definitions together.
A selected C export may be valid even if it is absent from the assembly object:
the C section provides it. `CpuSet` is one such example.

The splicer retains a function's section, not its entire translation unit.
Absolute symbols needed by a body must be defined inside that section, or in
retained assembly. A file-scope `__asm__(".set...")` is not sufficient.
An export list must preserve entry aliases without moving an interior label
to the start of its owner.

Branch targets and literal-pool references into a replaced span require special
care. Define an interior label at its exact offset inside the retained C
section, or keep the relevant source assembly. A first-statement inline label
can follow a compiler prologue and therefore land at entry +2 rather than +0.
A naked hardware-contract transcription can place a label precisely. Inspect
all external references, including `ldr` pool operands, before replacement;
static checks alone do not prove the linked references remain valid.

Check end labels in their owning source files. Inventing a missing
`<stem>_end` label or borrowing a neighboring region's boundary can replace
unrelated code.

## Span and alignment checks

Run `tools/span_audit.py` before diagnosing instruction selection. A missing
typed entry can make one function's span swallow another. Bare labels do not
necessarily define function entries; use `.type NAME, %function` where the
ROM establishes one. Do not infer boundaries from name spelling alone.

The probe's `prefix` is the contiguous matching run from the start.
`matched_bytes` counts equal bytes at any offsets, so it cannot establish
that a difference is confined to alignment. Use the predicate implemented by
`promotion_screen.alignment_only`, not a formula based on a total byte count.

For content length congruent to 2 modulo 4, gas may pad a function section
with `c0 46` while the ROM contains `00 00`. The ROM span includes padding:
testing the span length instead of content length reverses this diagnostic.
Compare both tails even when an earlier instruction also differs.
An in-section `__asm__(".align 2, 0");` can reproduce the zero padding.
Do not assume an alignment mismatch is unavoidable, or that every two-byte
residual is alignment-only.

`OVERSIZED` means only that the candidate exceeds the ROM span. A small
length difference does not imply a nearly matching instruction stream.
Establish the correct length and cause before tuning register allocation.
See [candidate-length diagnostics](findings/oversized_plus4_class.md).

## Compiler constraints

- A volatile signed halfword lvalue can emit `ldrh; lsl; asr` where a
  non-volatile read emits `mov rN,#0; ldrsh`. Distinguish a volatile pointer
  cell from a volatile pointee. IWRAM work areas are not hardware registers;
  preserve actual hardware volatility and access counts.
- Parameter widths follow the ROM. A narrow C parameter can introduce
  truncation absent from an already word-width calling convention.
- Named locals and their declaration positions can preserve load ordering.
  An inline expression may be folded or hoisted differently.
- A shared named constant can preserve one materialization used by a store
  and a call. Duplicating a literal can introduce a second instruction.
- GNU local register variables constrain a specific pseudo. Pinning a base
  pointer is not equivalent to pinning a loaded cell word. Document the
  required ROM register; verify each body's output independently.
- An empty read/write asm constraint such as `__asm__("" : "+r"(value))`
  can prevent rematerialization. Combined with a pinned register and later
  reuse, it preserves the source-to-return copy in the runtime entry writers
  at `0x0800315C`, `0x08003194`, and `0x08003210`.
- An assembler-resolved absolute base can preserve a pool load before address
  arithmetic. Keep its definition in the function section. See
  [the track-award example](findings/track_car_26180_pool_order.md).
- A `static` helper introduces a call unless the compiler actually inlines it.
  If the ROM has no such call, express the operation in the owning body.
- Switches, conditional expressions, and if/else chains can produce different
  instruction forms. Semantically equal C need not be byte-equivalent.

Compiler RTL dumps (`-da`) help distinguish instruction selection,
scheduling, and register allocation. A failed finite search establishes a
limit of the tested forms, not impossibility of every C representation.

For additional controlled experiments, see the
[newer agbcc recipe cookbook](findings/agbcc_matching_recipes.md): narrow-mode
locals, loop-note barriers, allocator diagnostics, and reproducible transfer
probes. It separates upstream observations from locally reproduced results.

## Selecting C sections

Use a fresh `build/era-corpus/ready-report.json` and its `ready-work/` objects.
Other corpus directories may contain stale output. Screen candidates with
`tools/promotion_screen.py`, then run `tools/call_audit.py` and
`tools/export_audit.py` before the independent link. The screen accepts exact
and verified alignment-only candidates; the link remains authoritative.

Do not edit source while its full verification is running. Isolated diagnostic
output can be selected with the probe's `--work-dir` and `--json` arguments.
Review the resulting manifest diff and rerun `make matching-ready` after
changing selected functions, exports, paths, or assembly boundaries.

## Optional matching diagnostics

```sh
python3 tools/match_families.py --build
python3 tools/match_context.py NAME
python3 tools/strategy_router.py --function NAME
```

Family matching compares instruction and branch patterns with consistent
register normalization, while retaining load/store widths. It is a retrieval
heuristic, not semantic-equivalence evidence. Fresh example probes validate
recipes against current source; stale indexes must be rebuilt.

`leaf_synth.py` supports bounded straight-line Thumb leaves and requires
ROM-backed choices for return type and memory qualification. It rejects
unsupported control flow, stack use, calls, and ambiguous live-ins. Its outputs
distinguish `EXACT_DRAFT`, `NO_EXACT_DRAFT`, and `UNSUPPORTED`.
Exit 0 means the search completed, not that it matched. Recheck a draft in its
actual translation unit and independent link before selecting it.

See the [tool index](../tools/README.md) for synthesis, disassembly, asset,
and runtime comparison utilities.
