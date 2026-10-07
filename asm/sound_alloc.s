@ GT Advance 3 - MTO sound driver: stream allocation helper
@ Region: file offset 0x02CBBC-0x02CC34 (VMA 0x0802CBBC-0x0802CC34).
@ Pure Thumb with a private pool; byte-exact.

.thumb
.type sub_0802CBBC, %function
sub_0802CBBC:
_0802CBBC:
	push {r4, r5, r6, r7, lr}
	adds r7, r0, #0
	adds r6, r1, #0
	lsls r2, r2, #24
	lsrs r4, r2, #24
	cmp r4, #0
	beq.n _0802CC20
	cmp r4, #16
	bls.n _0802CBD0
	movs r4, #16
_0802CBD0:
	ldr r0, _0802CC28
	ldr r5, [r0, #0]
	ldr r1, [r5, #0]
	ldr r0, _0802CC2C
	cmp r1, r0
	bne.n _0802CC20
	adds r0, r1, #1
	str r0, [r5, #0]
	adds r0, r7, #0
	bl 0x0802C8B0
	str r6, [r7, #0x2C]
	strb r4, [r7, #8]
	movs r0, #128
	lsls r0, r0, #24
	str r0, [r7, #4]
	cmp r4, #0
	beq.n _0802CC04
	movs r1, #0
_0802CBF6:
	strb r1, [r6, #0]
	subs r0, r4, #1
	lsls r0, r0, #24
	lsrs r4, r0, #24
	adds r6, #80
	cmp r4, #0
	bne.n _0802CBF6
_0802CC04:
	ldr r0, [r5, #32]
	cmp r0, #0
	beq.n _0802CC14
	str r0, [r7, #56]
	ldr r0, [r5, #36]
	str r0, [r7, #60]
	movs r0, #0
	str r0, [r5, #32]
_0802CC14:
	str r7, [r5, #36]
	ldr r0, _0802CC30
	str r0, [r5, #32]
	ldr r0, _0802CC2C
	str r0, [r5, #0]
	str r0, [r7, #52]
_0802CC20:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
_0802CC28: .word 0x03007FF0
_0802CC2C: .word 0x68736D53
_0802CC30: .word 0x0802BEB5
