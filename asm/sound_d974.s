@ sound BIOS/stack wrappers.
@ VMA 0x0802D974-0x0802D9B0 (60 bytes), pure Thumb with private pools.

	.thumb
	.type sub_0802D974, %function
sub_0802D974:
	swi 0x0B
	bx lr

	.type sub_0802D978, %function
sub_0802D978:
	swi 0x06
	bx lr

	.type sub_0802D97C, %function
sub_0802D97C:
	swi 0x06
	adds r0, r1, #0
	bx lr
	.short 0

	.type sub_0802D984, %function
sub_0802D984:
	swi 0x12
	bx lr

	.type sub_0802D988, %function
sub_0802D988:
	swi 0x11
	bx lr

	.type sub_0802D98C, %function
sub_0802D98C:
	movs r1, #1
	swi 0x25
	bx lr
	.short 0

	.type sub_0802D994, %function
sub_0802D994:
	ldr r3, .L_d9a4
	movs r2, #0
	strb r2, [r3]
	ldr r1, .L_d9a8
	mov sp, r1
	swi 1
	swi 0
	.short 0
.L_d9a4:
	.word 0x04000208
.L_d9a8:
	.word 0x03007F00

	.type sub_0802D9AC, %function
sub_0802D9AC:
	swi 8
	bx lr
sound_d974_end:
