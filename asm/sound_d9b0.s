@ sound control/state continuation.
@ VMA 0x0802D9B0-0x0802DA20 (112 bytes), pure Thumb with private pools/data.

	.thumb
	.type sub_0802D9B0, %function
sub_0802D9B0:
	movs r2, #0
	swi 5
	bx lr
	.short 0
	@ 0x0802D9B8 real entry: the next word is not sub_0802D9B0's pool (that
	@ body has no ldr). Its bytes 00 04 00 0C are `lsls r0,#16; lsrs r0,#16`,
	@ the u16 normalise prologue that falls through into sub_0802D9BC below.
	@ Called via `bl 0x0802D9B8` from asm/code_57d0.s:29,34, and C-lifted as
	@ SoundD9BC_SelectorU16 in src/sound_deep.c. Transcribed as instructions
	@ (byte-identical to the.word) so the entry has a label the slice link
	@ can resolve.
	.type sub_0802D9B8, %function
sub_0802D9B8:
_0802D9B8:
	lsls r0, r0, #16
	lsrs r0, r0, #16

	.type sub_0802D9BC, %function
sub_0802D9BC:
	movs r2, #0
	cmp r0, #4
	bne .L_d9d4
	ldr r1, .L_d9cc
	ldr r0, .L_d9d0
	str r0, [r1]
	b .L_d9f0
	.short 0
.L_d9cc:
	.word 0x0203FD54
.L_d9d0:
	.word 0x080C4274
.L_d9d4:
	cmp r0, #64
	bne .L_d9e8
	ldr r1, .L_d9e0
	ldr r0, .L_d9e4
	str r0, [r1]
	b .L_d9f0
.L_d9e0:
	.word 0x0203FD54
.L_d9e4:
	.word 0x080C4280
.L_d9e8:
	ldr r1, .L_d9f4
	ldr r0, .L_d9f8
	str r0, [r1]
	movs r2, #1
.L_d9f0:
	adds r0, r2, #0
	bx lr
.L_d9f4:
	.word 0x0203FD54
.L_d9f8:
	.word 0x080C4274

	.type sub_0802D9FC, %function
sub_0802D9FC:
	ldr r1, .L_da18
	ldrh r0, [r1]
	cmp r0, #0
	beq .L_da16
	ldrh r0, [r1]
	subs r0, #1
	strh r0, [r1]
	lsls r0, #16
	cmp r0, #0
	bne .L_da16
	ldr r1, .L_da1c
	movs r0, #1
	strb r0, [r1]
.L_da16:
	bx lr
.L_da18:
	.word 0x0300176A
.L_da1c:
	.word 0x0300176C
sound_d9b0_end:
