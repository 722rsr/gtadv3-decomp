/*
 * GT Advance 3 (A2GE) — Matching object-based linker script.
 * Links discrete translation units and module objects in exact ROM order.
 */

ENTRY(crt0)

SECTIONS
{
    . = 0x08000000;

    .text : {
        *(.text.header)
        *(.text.crt0)
        *(.text*)
        *(.rodata*)
        *(.data*)
    }

    . = 0x0802E158;

    .data_tail : {
        *(.data_tail)
        build-code/data.o(.text* .rodata* .data*)
    }

    /DISCARD/ : {
        *(.ARM.attributes)
        *(.comment)
        *(.note*)
    }
}
