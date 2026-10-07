# ROM layout

The US reference ROM is identified by `baserom.sha256`. ROM addresses use
`0x08000000` as their base: file offset = ROM VMA - `0x08000000`.

The independent executable slice covers file offsets `0x000000–0x02E158`
(end exclusive, 188,760 bytes). `asm/code.s` defines its assembly closure;
the matching manifest selects C replacements within that closure. The
remaining ROM data is supplied privately by the reference ROM until each
data family has a complete reconstruction path.

## Ownership and inventories

- [Executable-slice verification](independent_slice.md).
- [Complete source/data ownership](code_data_ownership.md).
- [Course headers](data/course_headers.txt).
- [Course resource records](data/track_resources.txt).
- [MTO directory entries](data/mto_entries.txt).
- [Compressed blocks](data/lz77_blobs.txt).
- [Pointer-table candidates](data/ptr_tables.txt).

Inventories describe reference-ROM data; they are not substitutes for asset
generators or proof that a candidate pointer is used by executable code.
Extracted assets and generated ROMs belong in ignored local directories.

See [completion criteria](decompilation-roadmap.md) for the distinction
between the reference build, independent executable slice, and full C ROM.
