@ GT Advance 3 - sound voice pan/volume envelope apply
@ Region: file offset 0x02C160-0x02C190 (VMA 0x0802C160-0x0802C190).
@ Pure Thumb; ARMCC outline leaf with no literal pool.

.thumb
.type sub_0802C160, %function
sub_0802C160:
_0802C160:
	ldrb r1, [r4, #18]
	movs r0, #20
	ldrsb r2, [r4, r0]
	movs r3, #128
	adds r3, r3, r2
	muls r3, r1
	ldrb r0, [r5, #16]
	muls r0, r3
	asrs r0, r0, #14
	cmp r0, #255
	bls.n _0802C178
	movs r0, #255
_0802C178:
	strb r0, [r4, #2]
	movs r3, #127
	subs r3, r3, r2
	muls r3, r1
	ldrb r0, [r5, #17]
	muls r0, r3
	asrs r0, r0, #14
	cmp r0, #255
	bls.n _0802C18C
	movs r0, #255
_0802C18C:
	strb r0, [r4, #3]
	bx lr

sound_voice_apply_end:
