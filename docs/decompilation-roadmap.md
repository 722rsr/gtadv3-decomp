# Build status and completion criteria

The byte-complete decompilation gates pass: reconstructed C/ASM and cataloged
data link to the exact US ROM, the executable entry census has no unresolved
candidates, and every owned ROM byte is accounted for. Some compressed assets
remain verbatim or are extracted from the supplied ROM; that is a separate
semantic-asset task, not an executable-code or byte-match gap. Read the current
[Objdiff report](data/report.json) with this evidence ledger because matching,
linking, and asset semantics are separate measures.

## Current evidence

- **Exact function matching: 100%.** `matching-ready` currently reports
  1,674 / 1,674 identified C function candidates and 188,136 / 188,136 selected
  function bytes byte-exact. An independent census checks the 1,674
  `.type`/`BL` inventory against 1,249 push-with-link-register patterns and
  function-shaped labels: 39 data-pool patterns and 6 reviewed non-entries are
  accounted for, with 0 unresolved candidates and 0 swallowed entries. Of the
  matched candidates, 1,024 have at least three source statements and 650 have
  one source statement; this is only a triage proxy for readability, not a
  semantic completeness score. Some exact owners are documented assembly or
  inline assembly rather than high-level C.
- **Whole-slice C-byte ratio: 99.67% (diagnostic only).** This is 188,136 C
  function bytes / 188,760 bytes in the executable slice. The denominator also
  includes the cartridge header, startup, reconstructed assembly, and
  non-function bytes. It is not a C-function decompilation percentage and
  should not be used as the headline progress figure.
- **Independent code link and Objdiff byte match: 100%.** The selected C
  replacements splice into a complete 188,760-byte C/ASM slice that links
  without original executable bytes and compares exactly. The report credits
  its 624 source-owned non-function bytes as exact only after this gate passes;
  these are not missing functions or hand-waved C matches.
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
(exact comparisons) from `complete_*` values (linked units/bytes). Padding is
excluded from data progress. The function count is cross-checked by the
independent census gate rather than treated as exhaustive merely because it
comes from `.type`/`BL` labels. Keep those definitions alongside any progress
number quoted elsewhere. The [ownership map](code_data_ownership.md),
[compiler documentation](compiler_status.md), and
[build verification guide](matching_workflow.md) document the underlying gates.

## Completion criteria

1. Every executable byte is classified as C-owned, documented source assembly,
   or non-function content. The independent prologue and shape scans account
   for all candidates and fail on any unexplained entry or span boundary.
   Keep exact byte matching separate from source readability and semantics.
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
