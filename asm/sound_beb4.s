@ GT Advance 3 - sound driver: per-frame channel walker
@ Region: file offset 0x02BEB4-0x02C110 (VMA _0802BEB4-0x0802C110).
@ Pure Thumb channel dispatcher (604 B code + trampoline/pad).
@ Shares pool at 0x02C110-0x02C118 with pump (08061754 / 03007FF0 / Smsh).
@ Disassembled via arm-none-eabi-objdump force-thumb; byte-exact.
@ Sole runtime pointer: literal word 0x0802BEB5 at 0x02CC30 (sound_alloc).
@ Caller: none BL (runtime-dispatched)

.thumb
.type sub_0802BEB4, %function
sub_0802BEB4:
_0802BEB4:
	ldr r2, lit_0802C118
	ldr	r3, [r0, #52]
	cmp	r2, r3
	beq.n	_0802BEBE
	bx	lr
_0802BEBE:
	adds	r3, #1
	str	r3, [r0, #52]
	push	{r0, lr}
	ldr	r3, [r0, #56]
	cmp	r3, #0
	beq.n	_0802BED0
	ldr	r0, [r0, #60]
	bl	0x0802c10c
_0802BED0:
	pop	{r0}
	push	{r4, r5, r6, r7}
	mov	r4, r8
	mov	r5, r9
	mov	r6, sl
	mov	r7, fp
	push	{r4, r5, r6, r7}
	adds	r7, r0, #0
	ldr	r0, [r7, #4]
	cmp	r0, #0
	bge.n	_0802BEE8
	b.n	_0802C0FC
_0802BEE8:
	ldr r0, lit_0802C114
	ldr	r0, [r0, #0]
	mov	r8, r0
	adds	r0, r7, #0
	bl	0x0802cd58
	ldr	r0, [r7, #4]
	cmp	r0, #0
	bge.n	_0802BEFC
	b.n	_0802C0FC
_0802BEFC:
	ldrh	r0, [r7, #34]
	ldrh	r1, [r7, #32]
	adds	r0, r0, r1
	b.n	_0802C04C
_0802BF04:
	ldrb	r6, [r7, #8]
	ldr	r5, [r7, #44]
	movs	r3, #1
	movs	r4, #0
_0802BF0C:
	ldrb	r0, [r5, #0]
	movs	r1, #128
	tst	r1, r0
	bne.n	_0802BF16
	b.n	_0802C028
_0802BF16:
	mov	sl, r3
	orrs	r4, r3
	mov	fp, r4
	ldr	r4, [r5, #32]
	cmp	r4, #0
	beq.n	_0802BF4A
_0802BF22:
	ldrb	r1, [r4, #0]
	movs	r0, #199
	tst	r0, r1
	beq.n	_0802BF3E
	ldrb	r0, [r4, #16]
	cmp	r0, #0
	beq.n	_0802BF44
	subs	r0, #1
	strb	r0, [r4, #16]
	bne.n	_0802BF44
	movs	r0, #64
	orrs	r1, r0
	strb	r1, [r4, #0]
	b.n	_0802BF44
_0802BF3E:
	adds	r0, r4, #0
	bl	0x0802c89c
_0802BF44:
	ldr	r4, [r4, #52]
	cmp	r4, #0
	bne.n	_0802BF22
_0802BF4A:
	ldrb	r3, [r5, #0]
	movs	r0, #64
	tst	r0, r3
	beq.n	_0802BFC8
	adds	r0, r5, #0
	bl	0x0802c8b0
	movs	r0, #128
	strb	r0, [r5, #0]
	movs	r0, #2
	strb	r0, [r5, #15]
	movs	r0, #64
	strb	r0, [r5, #19]
	movs	r0, #22
	strb	r0, [r5, #25]
	movs	r0, #1
	adds	r1, r5, #6
	strb	r0, [r1, #30]
	b.n	_0802BFC8
_0802BF70:
	ldr	r2, [r5, #64]
	ldrb	r1, [r2, #0]
	cmp	r1, #128
	bcs.n	_0802BF7C
	ldrb	r1, [r5, #7]
	b.n	_0802BF86
_0802BF7C:
	adds	r2, #1
	str	r2, [r5, #64]
	cmp	r1, #189
	bcc.n	_0802BF86
	strb	r1, [r5, #7]
_0802BF86:
	cmp	r1, #207
	bcc.n	_0802BF9C
	mov	r0, r8
	ldr	r3, [r0, #56]
	adds	r0, r1, #0
	subs	r0, #207
	adds	r1, r7, #0
	adds	r2, r5, #0
	bl	0x0802c10c
	b.n	_0802BFC8
_0802BF9C:
	cmp	r1, #176
	bls.n	_0802BFBE
	adds	r0, r1, #0
	subs	r0, #177
	strb	r0, [r7, #10]
	mov	r3, r8
	ldr	r3, [r3, #52]
	lsls	r0, r0, #2
	ldr	r3, [r3, r0]
	adds	r0, r7, #0
	adds	r1, r5, #0
	bl	0x0802c10c
	ldrb	r0, [r5, #0]
	cmp	r0, #0
	beq.n	_0802C024
	b.n	_0802BFC8
_0802BFBE:
	ldr r0, lit_0802C110
	subs	r1, #128
	adds	r1, r1, r0
	ldrb	r0, [r1, #0]
	strb	r0, [r5, #1]
_0802BFC8:
	ldrb	r0, [r5, #1]
	cmp	r0, #0
	beq.n	_0802BF70
	subs	r0, #1
	strb	r0, [r5, #1]
	ldrb	r1, [r5, #25]
	cmp	r1, #0
	beq.n	_0802C024
	ldrb	r0, [r5, #23]
	cmp	r0, #0
	beq.n	_0802C024
	ldrb	r0, [r5, #28]
	cmp	r0, #0
	beq.n	_0802BFEA
	subs	r0, #1
	strb	r0, [r5, #28]
	b.n	_0802C024
_0802BFEA:
	ldrb	r0, [r5, #26]
	adds	r0, r0, r1
	strb	r0, [r5, #26]
	adds	r1, r0, #0
	subs	r0, #64
	lsls	r0, r0, #24
	bpl.n	_0802BFFE
	lsls	r2, r1, #24
	asrs	r2, r2, #24
	b.n	_0802C002
_0802BFFE:
	movs	r0, #128
	subs	r2, r0, r1
_0802C002:
	ldrb	r0, [r5, #23]
	muls	r0, r2
	asrs	r2, r0, #6
	ldrb	r0, [r5, #22]
	eors	r0, r2
	lsls	r0, r0, #24
	beq.n	_0802C024
	strb	r2, [r5, #22]
	ldrb	r0, [r5, #0]
	ldrb	r1, [r5, #24]
	cmp	r1, #0
	bne.n	_0802C01E
	movs	r1, #12
	b.n	_0802C020
_0802C01E:
	movs	r1, #3
_0802C020:
	orrs	r0, r1
	strb	r0, [r5, #0]
_0802C024:
	mov	r3, sl
	mov	r4, fp
_0802C028:
	subs	r6, #1
	ble.n	_0802C034
	movs	r0, #80
	adds	r5, r5, r0
	lsls	r3, r3, #1
	b.n	_0802BF0C
_0802C034:
	ldr	r0, [r7, #12]
	adds	r0, #1
	str	r0, [r7, #12]
	cmp	r4, #0
	bne.n	_0802C046
	movs	r0, #128
	lsls	r0, r0, #24
	str	r0, [r7, #4]
	b.n	_0802C0FC
_0802C046:
	str	r4, [r7, #4]
	ldrh	r0, [r7, #34]
	subs	r0, #150
_0802C04C:
	strh	r0, [r7, #34]
	cmp	r0, #150
	bcc.n	_0802C054
	b.n	_0802BF04
_0802C054:
	ldrb	r2, [r7, #8]
	ldr	r5, [r7, #44]
_0802C058:
	ldrb	r0, [r5, #0]
	movs	r1, #128
	tst	r1, r0
	beq.n	_0802C0F2
	movs	r1, #15
	tst	r1, r0
	beq.n	_0802C0F2
	mov	r9, r2
	adds	r0, r7, #0
	adds	r1, r5, #0
	bl	0x0802ce20
	ldr	r4, [r5, #32]
	cmp	r4, #0
	beq.n	_0802C0E8
_0802C076:
	ldrb	r1, [r4, #0]
	movs	r0, #199
	tst	r0, r1
	bne.n	_0802C086
	adds	r0, r4, #0
	bl	0x0802c89c
	b.n	_0802C0E2
_0802C086:
	ldrb	r0, [r4, #1]
	movs	r6, #7
	ands	r6, r0
	ldrb	r3, [r5, #0]
	movs	r0, #3
	tst	r0, r3
	beq.n	_0802C0A4
	bl	0x0802c160
	cmp	r6, #0
	beq.n	_0802C0A4
	ldrb	r0, [r4, #29]
	movs	r1, #1
	orrs	r0, r1
	strb	r0, [r4, #29]
_0802C0A4:
	ldrb	r3, [r5, #0]
	movs	r0, #12
	tst	r0, r3
	beq.n	_0802C0E2
	ldrb	r1, [r4, #8]
	movs	r0, #8
	ldrsb	r0, [r5, r0]
	adds	r2, r1, r0
	bpl.n	_0802C0B8
	movs	r2, #0
_0802C0B8:
	cmp	r6, #0
	beq.n	_0802C0D6
	mov	r0, r8
	ldr	r3, [r0, #48]
	adds	r1, r2, #0
	ldrb	r2, [r5, #9]
	adds	r0, r6, #0
	bl	0x0802c10c
	str	r0, [r4, #32]
	ldrb	r0, [r4, #29]
	movs	r1, #2
	orrs	r0, r1
	strb	r0, [r4, #29]
	b.n	_0802C0E2
_0802C0D6:
	adds	r1, r2, #0
	ldrb	r2, [r5, #9]
	ldr	r0, [r4, #36]
	bl	0x0802c420
	str	r0, [r4, #32]
_0802C0E2:
	ldr	r4, [r4, #52]
	cmp	r4, #0
	bne.n	_0802C076
_0802C0E8:
	ldrb	r0, [r5, #0]
	movs	r1, #240
	ands	r0, r1
	strb	r0, [r5, #0]
	mov	r2, r9
_0802C0F2:
	subs	r2, #1
	ble.n	_0802C0FC
	movs	r0, #80
	adds	r5, r5, r0
	bgt.n	_0802C058
_0802C0FC:
	ldr r0, lit_0802C118
	str	r0, [r7, #52]
	pop	{r0, r1, r2, r3, r4, r5, r6, r7}
	mov	r8, r0
	mov	r9, r1
	mov	sl, r2
	mov	fp, r3
	pop	{r3}
_0802C10C:
	bx	r3
	.short 0x0000
