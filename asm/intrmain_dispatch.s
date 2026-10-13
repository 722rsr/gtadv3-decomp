	.text
@ Region: file offset 0x000F84-0x001370 (VMA 0x08000F84-0x08001370).
@ Thumb IRQ/serial state machine, instruction stream shared with its C owner.
	.thumb
	.type _08000F84, %function
_08000F84:
	.include "intrmain_dispatch_body.inc"
intrmain_dispatch_end:
