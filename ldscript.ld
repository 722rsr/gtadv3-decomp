/*
 * GT Advance 3 (A2GE) — ROM linker script.
 *
 * Everything lands in one output section at the cartridge base in input
 * order, so the binary image reproduces the original byte layout as long
 * as asm/rom.s lists segments in ROM order (it does — see region map).
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
