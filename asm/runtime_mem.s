@ GT Advance 3 - ARMCC memory helpers
@ Region: file offset 0x02E0A4-0x02E158 (VMA 0x0802E0A4-0x0802E158).
@ Pure Thumb; byte-exact transcriptions of memcpy and memset.

.thumb
.type sub_0802E0A4, %function
sub_0802E0A4:
_0802E0A4:
	push {r4, r5, lr}
	adds r5, r0, #0
	adds r4, r5, #0
	adds r3, r1, #0
	cmp r2, #15
	bls.n _0802E0E4
	adds r0, r3, #0
	orrs r0, r5
	movs r1, #3
	ands r0, r1
	cmp r0, #0
	bne.n _0802E0E4
	adds r1, r5, #0
_0802E0BE:
	ldmia r3!, {r0}
	stmia r1!, {r0}
	ldmia r3!, {r0}
	stmia r1!, {r0}
	ldmia r3!, {r0}
	stmia r1!, {r0}
	ldmia r3!, {r0}
	stmia r1!, {r0}
	subs r2, #16
	cmp r2, #15
	bhi.n _0802E0BE
	cmp r2, #3
	bls.n _0802E0E2
_0802E0D8:
	ldmia r3!, {r0}
	stmia r1!, {r0}
	subs r2, #4
	cmp r2, #3
	bhi.n _0802E0D8
_0802E0E2:
	adds r4, r1, #0
_0802E0E4:
	subs r2, #1
	movs r0, #1
	negs r0, r0
	cmp r2, r0
	beq.n _0802E0FE
_0802E0EE:
	adds r1, r0, #0
_0802E0F0:
	ldrb r0, [r3, #0]
	strb r0, [r4, #0]
	adds r3, #1
	adds r4, #1
	subs r2, #1
	cmp r2, r1
	bne.n _0802E0F0
_0802E0FE:
	adds r0, r5, #0
	pop {r4, r5, pc}
	.short 0

.type sub_0802E104, %function
sub_0802E104:
_0802E104:
	push {r4, r5, lr}
	adds r5, r0, #0
	adds r4, r1, #0
	adds r3, r5, #0
	cmp r2, #3
	bls.n _0802E14A
	movs r0, #3
	ands r0, r5
	cmp r0, #0
	bne.n _0802E14A
	adds r1, r5, #0
	movs r0, #255
	ands r4, r0
	lsls r3, r4, #8
	orrs r3, r4
	lsls r0, r3, #16
	orrs r3, r0
	cmp r2, #15
	bls.n _0802E13E
_0802E12A:
	stmia r1!, {r3}
	stmia r1!, {r3}
	stmia r1!, {r3}
	stmia r1!, {r3}
	subs r2, #16
	cmp r2, #15
	bhi.n _0802E12A
	b.n _0802E13E
_0802E13A:
	stmia r1!, {r3}
	subs r2, #4
_0802E13E:
	cmp r2, #3
	bhi.n _0802E13A
	adds r3, r1, #0
	b.n _0802E14A
_0802E146:
	strb r4, [r3, #0]
	adds r3, #1
_0802E14A:
	adds r0, r2, #0
	subs r2, #1
	cmp r0, #0
	bne.n _0802E146
	adds r0, r5, #0
	pop {r4, r5, pc}
	.short 0
