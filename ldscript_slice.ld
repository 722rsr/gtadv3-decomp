/*
 * GT Advance 3 (A2GE) — linker script for an independent code slice.
 *
 * Builds a partial cartridge image from reconstructed source only: no
 * build/rom.o, no .incbin of baserom.gba.  The image starts at the
 * cartridge base so every assembled VMA is also its final ROM address,
 * which is what makes the slice's byte comparison against baserom.gba a
 * meaningful test of the source rather than of the link.
 */

ENTRY(crt0)

SECTIONS
{
    . = 0x08000000;

    .text : {
        *(.text*)
        *(.rodata*)
        *(.data*)
    }

    /DISCARD/ : {
        *(.ARM.attributes)
        *(.comment)
        *(.note*)
    }
}
