@ GT Advance 3 — top-level ROM assembly (reference build).
@
@ Assembles the whole cartridge: the executable region (asm/code.s) followed
@ by the cataloged data/asset tail (asm/data_tail.s).  The two are separate
@ files so that the code region has exactly one definition, shared by the
@ reference build, the independent link slice (tools/independent_slice.py)
@ and the hybrid C link (build-code/code.o) — none of which copies code bytes
@ out of baserom.gba.
@
@ Layout rule: regions appear in exact file-offset order, and the reference
@ build must come out byte-identical to baserom.gba.  `make` fails loudly on
@ any hash mismatch, so every edit is verified by reflex.
@
@ The only raw byte source is the data/asset tail.  Everything else is
@ reconstructed source:
@
@   0x000000–0x0000C0  cartridge header            (asm/header.s)  ✅
@   0x0000C0–0x0002C4  crt0/IntrMain/IRQ plumbing  (asm/boot.s)    ✅
@   0x0002C4–0x000A60  AgbMain + boot helpers      (asm/agbmain.s) ✅
@   0x000A60–0x02E158  engine code                 (asm/passthrough.inc) ✅
@   0x02E158–0x800000  cataloged data/asset tail   (asm/data_tail.s) raw input

    .syntax unified
    .cpu arm7tdmi
    .include "macros/function.inc"

    .text

@@ 0x000000-0x02E158: the executable region (shared top level)
    .include "code.s"

@@ 0x02E158-0x800000: cataloged ROM data/asset tail (the only raw input)
    .include "data_tail.s"
