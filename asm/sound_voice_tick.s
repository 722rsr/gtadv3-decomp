@ GT Advance 3 - MTO sound driver: per-voice parameter tick
@ Region: file offset 0x02CE20-0x02CED4 (VMA 0x0802CE20-0x0802CED4).
@ Pure Thumb without a literal pool; byte-exact.

.thumb
.type sub_0802CE20, %function
sub_0802CE20:
_0802CE20:
	push {r4, lr}
	adds r2, r1, #0
	movs r0, #1
	ldrb r1, [r2, #0]
	ands r0, r1
	cmp r0, #0
	beq.n _0802CE84
	ldrb r3, [r2, #19]
	ldrb r1, [r2, #18]
	adds r0, r3, #0
	muls r0, r1
	lsrs r3, r0, #5
	ldrb r4, [r2, #24]
	cmp r4, #1
	bne.n _0802CE48
	movs r0, #22
	ldrsb r0, [r2, r0]
	adds r0, #128
	muls r0, r3
	lsrs r3, r0, #7
_0802CE48:
	movs r0, #20
	ldrsb r0, [r2, r0]
	lsls r0, r0, #1
	movs r1, #21
	ldrsb r1, [r2, r1]
	adds r1, r0, r1
	cmp r4, #2
	bne.n _0802CE5E
	movs r0, #22
	ldrsb r0, [r2, r0]
	adds r1, r0
_0802CE5E:
	movs r0, #128
	negs r0, r0
	cmp r1, r0
	bge.n _0802CE6A
	adds r1, r0, #0
	b.n _0802CE70
_0802CE6A:
	cmp r1, #127
	ble.n _0802CE70
	movs r1, #127
_0802CE70:
	adds r0, r1, #0
	adds r0, #128
	muls r0, r3
	lsrs r0, r0, #8
	strb r0, [r2, #16]
	movs r0, #127
	subs r0, r1
	muls r0, r3
	lsrs r0, r0, #8
	strb r0, [r2, #17]
_0802CE84:
	ldrb r1, [r2, #0]
	movs r0, #4
	ands r0, r1
	adds r3, r1, #0
	cmp r0, #0
	beq.n _0802CEC8
	movs r0, #14
	ldrsb r0, [r2, r0]
	ldrb r1, [r2, #15]
	muls r0, r1
	movs r1, #12
	ldrsb r1, [r2, r1]
	adds r1, r0
	lsls r1, r1, #2
	movs r0, #10
	ldrsb r0, [r2, r0]
	lsls r0, r0, #8
	adds r1, r0
	movs r0, #11
	ldrsb r0, [r2, r0]
	lsls r0, r0, #8
	adds r1, r0
	ldrb r0, [r2, #13]
	adds r1, r0, r1
	ldrb r0, [r2, #24]
	cmp r0, #0
	bne.n _0802CEC2
	movs r0, #22
	ldrsb r0, [r2, r0]
	lsls r0, r0, #4
	adds r1, r0
_0802CEC2:
	asrs r0, r1, #8
	strb r0, [r2, #8]
	strb r1, [r2, #9]
_0802CEC8:
	movs r0, #250
	ands r0, r3
	strb r0, [r2, #0]
	pop {r4}
	pop {r0}
	bx r0
