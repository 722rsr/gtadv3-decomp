# Build status and completion criteria

The target is a byte-identical US ROM built from readable C and cataloged
data. The full independent C ROM is not complete.

## Build status

The reference build (`make`) assembles reconstructed source, verified generated
data regions, and zero padding. It checks the resulting ROM against `baserom.sha256`.
Its matching hash does not establish a C-built ROM.

The independent executable slice covers ROM VMA `0x08000000–0x0802E158`
(file offsets `0x000000–0x02E158`), or 188,760 bytes. It contains reconstructed
assembly and the C functions selected by
[`matching_slice_functions.json`](../tools/matching_slice_functions.json).
The manifest selects 1,121 functions covering 53,864 bytes. `make matching-ready`
checks this mixed C/assembly slice against the reference and verifies that a
deliberately changed instruction fails the comparison.

The remaining work is C reconstruction and matching and use of the independent C
link for the complete ROM. All cataloged data through content end `0x7B04C4`
(7,873,388 bytes; padding excluded) now uses
[verified generator inputs](data-integration.md). See the
[ownership map](code_data_ownership.md), [compiler documentation](compiler_status.md),
and [build verification guide](matching_workflow.md).

## Completion criteria

1. Every executable region has a C owner or documented source assembly for a
   hardware contract C cannot express, such as startup, interrupt entry, BIOS
   calls, or register-branch veneers. A known-function count alone does not
   classify every byte or entry point.
2. The complete independent link obtains behavior from reconstructed source.
   It must not link `build/rom.o`, include original executable bytes through
   `.incbin`, or use ROM trampolines. Direct calls, callback tables, interrupt
   vectors, and relocated handlers must resolve to rebuilt code.
3. Each data or asset input has a named range, type, consumer, and extraction
   or generation step. Build rules distinguish data from executable bytes.
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
