@ GT Advance 3 - AI/collection line score accessor
@ Region: file offset 0x025548-0x0255C4 (VMA 0x08025548-0x080255C4).
@ Pure Thumb; one catalog score sum with a private pool.

.thumb
.type sub_08025548, %function
sub_08025548:
_08025548:
	adds r2, r0, #0
	ldr r3, _080255C0
	movs r1, #2
	ldrsb r1, [r2, r1]
	lsls r1, r1, #3
	adds r1, r1, r3
	movs r0, #6
	ldrsb r0, [r2, r0]
	lsls r0, r0, #3
	adds r0, r0, r3
	adds r0, #34
	ldrb r1, [r1, #2]
	ldrb r0, [r0, #0]
	adds r0, r1, r0
	movs r1, #4
	ldrsb r1, [r2, r1]
	lsls r1, r1, #3
	adds r1, r1, r3
	adds r1, #66
	ldrb r1, [r1, #0]
	adds r0, r1, r0
	movs r1, #7
	ldrsb r1, [r2, r1]
	lsls r1, r1, #3
	adds r1, r1, r3
	adds r1, #98
	ldrb r1, [r1, #0]
	adds r0, r1, r0
	movs r1, #5
	ldrsb r1, [r2, r1]
	lsls r1, r1, #3
	adds r1, r1, r3
	adds r1, #130
	ldrb r1, [r1, #0]
	adds r0, r1, r0
	movs r1, #9
	ldrsb r1, [r2, r1]
	lsls r1, r1, #3
	adds r1, r1, r3
	adds r1, #162
	ldrb r1, [r1, #0]
	adds r0, r1, r0
	movs r1, #3
	ldrsb r1, [r2, r1]
	lsls r1, r1, #3
	adds r1, r1, r3
	adds r1, #194
	ldrb r1, [r1, #0]
	adds r0, r1, r0
	movs r1, #8
	ldrsb r1, [r2, r1]
	lsls r1, r1, #3
	adds r1, r1, r3
	adds r1, #226
	ldrb r1, [r1, #0]
	adds r0, r1, r0
	lsls r0, r0, #24
	asrs r0, r0, #24
	bx lr
	.short 0
_080255C0: .word 0x080CD6A8
