@ AI/car-lineup timing/serial helper.
@ VMA 0x080251FC-0x08025214 (24 bytes), pure Thumb with private pools.

	.thumb
	.type sub_080251FC, %function
sub_080251FC:
	push {lr}
	ldr r1, .L_520c
	ldr r2, .L_5210
	bl 0x0802D974
	pop {r0}
	bx r0
	.short 0
.L_520c:
	.word 0x030015E8
.L_5210:
	.word 0x04000002
ai_line_sio_end:
