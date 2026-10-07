@ GT Advance 3 - MTO sound driver: per-stream fade/update tick
@ Region: file offset 0x02CD58-0x02CE20 (VMA 0x0802CD58-0x0802CE20).
@ Pure Thumb with a private sentinel pool; byte-exact.

.thumb
.type sub_0802CD58, %function
sub_0802CD58:
_0802CD58:
	push {r4, r5, r6, r7, lr}
	adds r6, r0, #0
	ldrh r1, [r6, #36]
	cmp r1, #0
	beq.n _0802CE1A
	ldrh r0, [r6, #38]
	subs r0, #1
	strh r0, [r6, #38]
	ldr r3, _0802CD98
	adds r2, r3, #0
	lsls r0, r0, #16
	lsrs r3, r0, #16
	cmp r3, #0
	bne.n _0802CE1A
	strh r1, [r6, #38]
	ldrh r1, [r6, #40]
	movs r0, #2
	ands r0, r1
	cmp r0, #0
	beq.n _0802CD9C
	adds r0, r1, #0
	adds r0, #16
	strh r0, [r6, #40]
	ands r0, r2
	cmp r0, #255
	bls.n _0802CDEE
	movs r0, #128
	lsls r0, r0, #1
	strh r0, [r6, #40]
	strh r3, [r6, #36]
	b.n _0802CDEE
	.short 0
_0802CD98: .word 0x0000FFFF
_0802CD9C:
	adds r0, r1, #0
	subs r0, #16
	strh r0, [r6, #40]
	ands r0, r2
	lsls r0, r0, #16
	cmp r0, #0
	bgt.n _0802CDEE
	ldrb r5, [r6, #8]
	ldr r4, [r6, #44]
	cmp r5, #0
	ble.n _0802CDCE
_0802CDB2:
	adds r0, r6, #0
	adds r1, r4, #0
	bl 0x0802C11C
	movs r0, #1
	ldrh r7, [r6, #40]
	ands r0, r7
	cmp r0, #0
	bne.n _0802CDC6
	strb r0, [r4, #0]
_0802CDC6:
	subs r5, #1
	adds r4, #80
	cmp r5, #0
	bgt.n _0802CDB2
_0802CDCE:
	movs r0, #1
	ldrh r1, [r6, #40]
	ands r0, r1
	cmp r0, #0
	beq.n _0802CDE2
	ldr r0, [r6, #4]
	movs r1, #128
	lsls r1, r1, #24
	orrs r0, r1
	b.n _0802CDE6
_0802CDE2:
	movs r0, #128
	lsls r0, r0, #24
_0802CDE6:
	str r0, [r6, #4]
	movs r0, #0
	strh r0, [r6, #36]
	b.n _0802CE1A
_0802CDEE:
	ldrb r5, [r6, #8]
	ldr r4, [r6, #44]
	cmp r5, #0
	ble.n _0802CE1A
	movs r3, #128
	movs r7, #0
	movs r2, #3
_0802CDFC:
	ldrb r1, [r4, #0]
	adds r0, r3, #0
	ands r0, r1
	cmp r0, #0
	beq.n _0802CE12
	ldrh r7, [r6, #40]
	lsrs r0, r7, #2
	strb r0, [r4, #19]
	adds r0, r1, #0
	orrs r0, r2
	strb r0, [r4, #0]
_0802CE12:
	subs r5, #1
	adds r4, #80
	cmp r5, #0
	bgt.n _0802CDFC
_0802CE1A:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
