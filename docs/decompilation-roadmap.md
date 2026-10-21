# Build status and completion criteria

The ROM is byte-complete: reconstructed C/ASM and cataloged data link to the
exact US ROM, the executable entry census has no unresolved candidates, and
every owned ROM byte is accounted for. The decompilation is not complete: about
half of the executable bytes still come from assembly, either in `asm/` or as
naked/asm-dominated functions in `src/`. Byte completion and decompilation are
reported separately. Read the current [Objdiff report](data/report.json) and
the README's generated progress block with this ledger.

## Current evidence

- **Decompiled to C: under half of the code (see the README progress block).**
  `tools/source_quality.py` classifies each selected function. Only `c` (plain
  C) and `c_fakematch` (C whose only asm is empty barriers, register pins, or
  directives) count as decompiled. `asm` (naked or asm-dominated bodies) and
  `partial` (C with instructions spliced in through `asm()`) are byte-exact but
  not decompiled. This is the headline progress figure.
- **Function-entry census: complete.** An independent census checks the
  `.type`/`BL` inventory against every push-with-link-register pattern and
  function-shaped label: data-pool patterns and reviewed non-entries are
  accounted for, with 0 unresolved candidates and 0 swallowed entries. This
  shows the inventory is complete. It is not a decompilation measure.
- **Independent code link: 100%.** The selected sources (C, inline-asm C, and
  naked asm alike) splice into a complete 188,760-byte C/ASM slice that links
  without original executable bytes and compares exactly. The report records
  this in its `complete_*` measures. It is a build fact, not decompilation
  progress.
- **Whole-ROM hybrid link: PASS.** `make full-rom-match` combines the exact
  C/ASM code slice with the registered data object, emits the 326,460-byte
  zero-padding tail from source, and compares the entire 8 MiB output
  byte-for-byte with the pinned US ROM. This is a full-image link and match;
  it does not make ROM-derived/verbatim data payloads semantic source.
- **Reference ROM build:** the assembly/data reference build also compares
  exactly against the pinned US ROM. It remains a separate build path from the
  C-spliced full-image gate.
- **Data bytes:** all 7,873,388 non-padding content bytes round-trip through the
  registered data generators. Some payloads are retained verbatim or originate
  from ROM extraction, so this measures byte reproduction, not semantic asset
  reconstruction. The 326,460 padding bytes are excluded from the data
  denominator.
- **Runtime equivalence:** not run. It is a separate behavior check; exact
  whole-image identity already establishes the same ROM bytes.

The generated report in `docs/data/report.json` separates `matched_*` values
(functions decompiled to C) from `complete_*` values (linked units/bytes). Padding is
excluded from data progress. A per-function `fuzzy_match_percent` there is a
projection of the source class (100 for `c`/`c_fakematch`, 0 otherwise), not a
measurement: it must never be read back as match evidence, because the class it
encodes is what wrote it. Byte truth comes from the gates above -- `make
matching-ready` splices every selected body's *compiled* output and requires an
identical code slice, and `make full-rom-match` requires an identical image --
so a passing gate is the evidence that the C reproduces the ROM. The function count is cross-checked by the
independent census gate rather than treated as exhaustive merely because it
comes from `.type`/`BL` labels. Keep those definitions alongside any progress
number quoted elsewhere. The [ownership map](code_data_ownership.md),
[compiler documentation](compiler_status.md), and
[build verification guide](matching_workflow.md) document the underlying gates.

## Completion criteria

1. Every executable byte is classified as C-owned, documented source assembly,
   or non-function content. The independent prologue and shape scans account
   for all candidates and fail on any unexplained entry or span boundary.
   Keep exact byte matching separate from source form: a naked or
   asm-dominated body owns its bytes but is not decompiled.
2. The complete independent link obtains behavior from reconstructed source.
   It must not link `build/rom.o`, include original executable bytes through
   `.incbin`, or use ROM trampolines. Direct calls, callback tables, interrupt
   vectors, and relocated handlers must resolve to rebuilt code.
3. Each data or asset input has a named range, type, consumer, provenance, and
   extraction or generation step. Report byte reproduction separately from
   semantic decoding. Build rules distinguish data from executable bytes.
   Private ROM data may serve as an extraction input; its executable bytes
   cannot satisfy source ownership.
4. The complete hybrid ROM matches `baserom.sha256`. Isolated function matches,
   the executable-slice match, and the reference-build hash are separate
   checks and do not substitute for the whole-image result; `make
   full-rom-match` passes this gate. All four byte-completion criteria pass.

Runtime comparisons and semantic asset decoding remain optional follow-up
quality work. Runtime equivalence is not a prerequisite once the independently
linked full ROM is byte-identical; raw compressed assets remain separately
identified and byte-verified rather than being presented as semantically
decoded.

Readable names, subsystem boundaries, and explanations of hardware quirks are
part of source quality. Preserve ROM aliases, ABI signatures, access widths,
call order, and integer behavior when improving readability.
