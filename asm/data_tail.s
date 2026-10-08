@ GT Advance 3 — cataloged ROM data / asset tail.
@
@ The data tail contains private ROM inputs and reviewed generated regions. It is
@ *data*, not executable code: 0x02E158 is exactly where reconstructed code
@ ends (see docs/code_data_ownership.md — zero raw executable bytes remain),
@ and everything up to the documented content end 0x7B04C4 is asset/resource
@ content, followed by verified zero padding.
@
@ This file exists so the code passthrough (asm/passthrough.inc) contains no
@ `.incbin` at all.  That is what lets an independent link slice assemble the
@ entire executable region from reconstructed source with nothing copied out
@ of baserom.gba — the reference `make` build still pulls this span in, in
@ ROM order, immediately after asm/passthrough.inc.
@
@ The generated palettes, course chain, LUTs, and compressed surface/variant
@ streams are rebuilt from private editable inputs (compressed streams kept
@ verbatim like normal .lz data); only the remaining ranges are copied from
@ the reference.
@ The data/asset inventories remain the layout authority: the catalogued
@ ranges in docs/data/ (lz77_blobs.txt, mto_entries.txt, track_resources.txt)
@ all index into this span, as does the on-demand ASCII string inventory
@ (python3 tools/findstr.py baserom.gba).

@ Early data tail with the MTO directory.
@ File offsets 0x02E158..0x0CE438; generator and hash: tools/data_regions.json.
    .global early_data_start
early_data_start:
    .incbin "build/data/early_data.bin"
    .global early_data_end
early_data_end:

@ MTO group 0: 63 course records, each header + variable payload.
@ File offsets 0xCE438..0x16BA2C; generator and hash: tools/data_regions.json.
    .global mto_group0_data_start
mto_group0_data_start:
    .incbin "build/data/mto_group0.bin"
    .global mto_group0_data_end
mto_group0_data_end:

@ MTO group 1: 63 surface-map records, headers plus LZ77 payloads kept verbatim.
@ File offsets 0x16BA2C..0x23CEFC; generator and hash: tools/data_regions.json.
    .global surface_map_data_start
surface_map_data_start:
    .incbin "build/data/surface_map.bin"
    .global surface_map_data_end
surface_map_data_end:

@ MTO group 2: eight theme-texture records, headers plus LZ77 payloads kept verbatim.
@ File offsets 0x23CEFC..0x252DC4; generator and hash: tools/data_regions.json.
    .global theme_texture_data_start
theme_texture_data_start:
    .incbin "build/data/theme_texture.bin"
    .global theme_texture_data_end
theme_texture_data_end:

@ MTO group 3: eight LUT records, each header + 256 payload bytes.
@ File offsets 0x252DC4..0x253604; generator and hash: tools/data_regions.json.
    .global theme_lut_a_data_start
theme_lut_a_data_start:
    .incbin "build/data/theme_lut_a.bin"
    .global theme_lut_a_data_end
theme_lut_a_data_end:

@ MTO group 4: eight palette records, each header + 320 payload bytes.
@ File offsets 0x253604..0x254044; generator and hash: tools/data_regions.json.
    .global theme_palette_data_start
theme_palette_data_start:
    .incbin "build/data/theme_palette.bin"
    .global theme_palette_data_end
theme_palette_data_end:

@ MTO group 5: seven surface-tile records, headers plus LZ77 payloads kept verbatim.
@ File offsets 0x254044..0x25A118; generator and hash: tools/data_regions.json.
    .global surface_tile_data_start
surface_tile_data_start:
    .incbin "build/data/surface_tile.bin"
    .global surface_tile_data_end
surface_tile_data_end:

@ MTO group 6: seven records, each header + sixteen palette halfwords.
@ File offsets 0x25A118..0x25A230; generator and hash: tools/data_regions.json.
    .global variant_palette_data_start
variant_palette_data_start:
    .incbin "build/data/variant_palette.bin"
    .global variant_palette_data_end
variant_palette_data_end:

@ MTO group 7: seven variant-overlay records, headers plus LZ77 payloads kept verbatim.
@ File offsets 0x25A230..0x25AF78; generator and hash: tools/data_regions.json.
    .global variant_overlay_data_start
variant_overlay_data_start:
    .incbin "build/data/variant_overlay.bin"
    .global variant_overlay_data_end
variant_overlay_data_end:

@ MTO group 8: 26 big-graphics records, headers plus LZ77 payloads kept verbatim.
@ File offsets 0x25AF78..0x284E2C; generator and hash: tools/data_regions.json.
    .global big_gfx_data_start
big_gfx_data_start:
    .incbin "build/data/big_gfx.bin"
    .global big_gfx_data_end
big_gfx_data_end:

@ MTO group 9: 26 small-table records, headers plus 0x20/0x40/0x80 payloads.
@ File offsets 0x284E2C..0x2856DC; generator and hash: tools/data_regions.json.
    .global scenery_table_data_start
scenery_table_data_start:
    .incbin "build/data/scenery_table.bin"
    .global scenery_table_data_end
scenery_table_data_end:

@ MTO group 10: 26 minimap records, headers plus LZ77 payloads kept verbatim.
@ File offsets 0x2856DC..0x28A35C; generator and hash: tools/data_regions.json.
    .global minimap_data_start
minimap_data_start:
    .incbin "build/data/minimap.bin"
    .global minimap_data_end
minimap_data_end:

@ Post-chain asset data.
@ File offsets 0x28A35C..0x3D7DA4; generator and hash: tools/data_regions.json.
    .global mid_gap_data_start
mid_gap_data_start:
    .incbin "build/data/mid_gap.bin"
    .global mid_gap_data_end
mid_gap_data_end:

@ LZ77 blob field.
@ File offsets 0x3D7DA4..0x799640; generator and hash: tools/data_regions.json.
    .global blob_field_data_start
blob_field_data_start:
    .incbin "build/data/blob_field.bin"
    .global blob_field_data_end
blob_field_data_end:

@ Tail end to documented content end.
@ File offsets 0x799640..0x7B04C4; generator and hash: tools/data_regions.json.
    .global tail_end_data_start
tail_end_data_start:
    .incbin "build/data/tail_end.bin"
    .global tail_end_data_end
tail_end_data_end:

    .incbin "baserom.gba", 0x7B04C4, 0x4FB3C    @ verified zero padding to end of ROM (0x800000)
