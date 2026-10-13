# Build status and completion criteria

The target is a byte-identical US ROM built from readable C and cataloged data.
The full independent C/hybrid ROM link is not complete. Read the current
[Objdiff report](data/report.json) together with this evidence ledger: the
percentages describe different axes, not one overall “decompiled” score.

## Current evidence

- **Exact C-function matching: 100%.** `matching-ready` currently reports
  1,674 / 1,674 identified C function candidates and 188,136 / 188,136 selected
  C-function bytes byte-exact. The ROM function inventory is based on source
  `.type` declarations and `BL` targets, so this is a lower-bound census, not
  proof that no unlabelled or indirect-call entry points remain. Of the matched
  candidates, 1,024 have at least three source statements and 650 are one-line
  stubs; this split is only a triage proxy, not a judgment of semantic
  completeness or readability.
- **Whole-slice C-byte ratio: 99.67% (diagnostic only).** This is 188,136 C
  function bytes / 188,760 bytes in the executable slice. The denominator also
  includes the cartridge header, startup, reconstructed assembly, and
  non-function bytes. It is not a C-function decompilation percentage and
  should not be used as the headline progress figure.
- **Independent code link: 100%.** The selected C replacements splice into a
  complete 188,760-byte C/ASM executable slice that links without original
  executable bytes and compares exactly. This proves the code slice, not the
  full ROM.
- **Reference ROM build:** the assembly/data reference build compares exactly
  against the pinned US ROM. The build still reads the final 326,460 bytes of
  verified zero padding from `baserom.gba`; it is therefore not a fully
  independent ROM build.
- **Data bytes:** all 7,873,388 non-padding content bytes round-trip through the
  registered data generators. Some payloads are retained verbatim or originate
  from ROM extraction, so this measures byte reproduction, not semantic asset
  reconstruction. The 326,460 padding bytes are excluded from the data
  denominator.
- **Full independent C/ASM ROM link:** not established.
- **Runtime equivalence:** not run.

The generated report in `docs/data/report.json` separates `matched_*` values
(exact comparisons) from `complete_*` values (linked units/bytes). Padding is
excluded from data progress, and report function counts are explicitly scoped
to the currently identified entry inventory. Keep those definitions alongside
any progress number quoted elsewhere. The [ownership map](code_data_ownership.md),
[compiler documentation](compiler_status.md), and
[build verification guide](matching_workflow.md) document the underlying gates.

## Completion criteria

1. Every executable byte is classified as C-owned, documented source assembly,
   or non-function content, and the known-function inventory is audited for
   missed entries. Do not equate a `.type`/call-target count with an exhaustive
   census. Keep the C function-match count separate from source-quality review;
   exact byte-matching stubs are not meaningful decompilation.
2. The complete independent link obtains behavior from reconstructed source.
   It must not link `build/rom.o`, include original executable bytes through
   `.incbin`, or use ROM trampolines. Direct calls, callback tables, interrupt
   vectors, and relocated handlers must resolve to rebuilt code.
3. Each data or asset input has a named range, type, consumer, provenance, and
   extraction or generation step. Report byte reproduction separately from
   semantic decoding. Build rules distinguish data from executable bytes.
   Private ROM data may serve as an extraction input; its executable bytes
   cannot satisfy source ownership.
4. The complete independent ROM matches `baserom.sha256`. Isolated function
   matches, the executable-slice match, and the reference-build hash are
   separate checks and do not substitute for this result.
5. Runtime comparisons run the rebuilt ROM and reference with identical
   scripted input, covering boot, menus, racing, finish, save/load, garage,
   and sound. Captures include IWRAM, EWRAM, VRAM, palette RAM, and OAM, with
   evidence that rebuilt code executed. Runtime traces diagnose behavior;
   byte identity remains the final output gate.

Readable names, subsystem boundaries, and explanations of hardware quirks are
part of source quality. Preserve ROM aliases, ABI signatures, access widths,
call order, and integer behavior when improving readability.
