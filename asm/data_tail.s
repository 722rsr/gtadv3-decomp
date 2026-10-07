@ GT Advance 3 — cataloged ROM data / asset tail.
@
@ The data tail is the one remaining raw input in the cartridge.  It is
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
@ Tracking this span as a named, single data input (rather than an anonymous
@ tail) is also the anchor for the data/asset inventory work: the catalogued
@ ranges in docs/data/ (lz77_blobs.txt, mto_entries.txt, track_resources.txt)
@ all index into this span, as does the on-demand ASCII string inventory
@ (python3 tools/findstr.py baserom.gba).

    .incbin "baserom.gba", 0x02E158, 0x7D1EA8    @ -> end of ROM (0x800000)
