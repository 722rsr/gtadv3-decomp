@ GT Advance 3 — the executable region, as one shared top level.
@
@ This file owns file offsets 0x000000-0x02E158: every executable byte in the
@ cartridge.  It is the single definition of the code region, consumed by
@ three builds:
@
@   `make`                 asm/rom.s includes it, then the data tail
@   `make independent-slice` tools/independent_slice.py assembles it alone
@   `make c`               the hybrid C link links it as build-code/code.o
@
@ It is deliberately *not* the whole ROM: it never includes asm/data_tail.s,
@ so no `.incbin` of baserom.gba appears anywhere in its include closure.
@ tools/independent_slice.py enforces that, and additionally checks that no
@ out-of-slice reference is left dangling.
@
@ Owned region (file offsets; VMA = +0x08000000):
@   0x000000-0x0000C0  cartridge header          asm/header.s
@   0x0000C0-0x0002C4  crt0 / IntrMain           asm/boot.s
@   0x0002C4-0x000A60  AgbMain + boot helpers    asm/agbmain.s
@   0x000A60-0x02E158  all engine code           asm/passthrough.inc
@
@ asm/passthrough.inc is the generated region manifest shared by every build.
@ Since the cataloged data/asset tail was moved out into asm/data_tail.s, the
@ manifest itself contains no raw bytes, so reusing it here gives all three
@ builds one source of truth for region order.  The data tail is *not* part
@ of the code region.
@
@ What the code region proves: 0x02E158 is a hard boundary in the cartridge,
@ and everything below it can be produced by an independent link that never
@ sees build/rom.o and never copies reference bytes.  What it does not prove:
@ the data/asset tail (0x02E158-0x7B04C4) is still a declared raw data input.

    .syntax unified
    .cpu arm7tdmi

    .text

@@ 0x000000: cartridge header — branch into crt0, Nintendo logo, identity fields
    .include "header.s"

@@ 0x0000C0: crt0, IntrMain and the IRQ hook plumbing
    .include "boot.s"

@@ 0x0002C4: AgbMain and the boot-adjacent helpers
    .include "agbmain.s"

@@ 0x000A60-0x02E158: every reconstructed code region, in ROM order
    .include "passthrough.inc"
