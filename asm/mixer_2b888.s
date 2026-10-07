	.text
	.arm
	.type sub_0802B888, %function
sub_0802B888:
_0802B888:
	ldrmi	sl, [r0, -r0, lsl #4]
	umull	r2, r3, r0, r1
	add	r0, r3, #0
	bx	lr
	.thumb
	.type sub_0802B898, %function
sub_0802B898:
	ldr	r0, [pc, #104]	@ (0x802b904)
	ldr	r0, [r0, #0]
	ldr	r2, [pc, #104]	@ (0x802b908)
	ldr	r3, [r0, #0]
	cmp	r2, r3
	beq.n	0x802b8a6
	bx	lr
	adds	r3, #1
	str	r3, [r0, #0]
	.type _0802B8AA, %function
_0802B8AA:
	push	{r4, r5, r6, r7, lr}
	mov	r1, r8
	mov	r2, r9
	mov	r3, sl
	mov	r4, fp
	push	{r0, r1, r2, r3, r4}
	sub	sp, #24
	ldrb	r1, [r0, #12]
	cmp	r1, #0
	beq.n	0x802b8ca
	ldr	r2, [pc, #80]	@ (0x802b910)
	ldrb	r2, [r2, #0]
	cmp	r2, #160	@ 0xa0
	bcs.n	0x802b8c8
	adds	r2, #228	@ 0xe4
	adds	r1, r1, r2
	str	r1, [sp, #20]
	ldr	r3, [r0, #32]
	cmp	r3, #0
	beq.n	0x802b8da
	ldr	r0, [r0, #36]	@ 0x24
	bl	0x802bc46
	ldr	r0, [sp, #24]
	ldr	r3, [r0, #40]	@ 0x28
	bl	0x802bc46
	ldr	r0, [sp, #24]
	ldr	r3, [r0, #16]
	mov	r8, r3
	ldr	r5, [pc, #44]	@ (0x802b914)
	adds	r5, r5, r0
	ldrb	r4, [r0, #4]
	subs	r7, r4, #1
	bls.n	0x802b8fa
	ldrb	r1, [r0, #11]
	subs	r1, r1, r7
	mov	r2, r8
	muls	r2, r1
	adds	r5, r5, r2
	str	r5, [sp, #8]
	ldr	r6, [pc, #24]	@ (0x802b918)
	ldr	r3, [pc, #12]	@ (0x802b90c)
	bx	r3
	movs	r0, r0
	ldrb	r0, [r6, #31]
	lsls	r0, r0, #12
	ldr	r3, [r2, #84]	@ 0x54
	ldr	r3, [r6, #4]
	strb	r1, [r0, #0]
	lsls	r0, r0, #12
	movs	r6, r0
	lsls	r0, r0, #16
	lsls	r0, r2, #13
	movs	r0, r0
	lsls	r0, r6, #24
	movs	r0, r0
	ldrb	r3, [r0, #5]
	cmp	r3, #0
	beq.n	0x802b968
	add	r1, pc, #4	@ (adr r1, 0x802b928)
	bx	r1
	movs	r0, r0
	.arm
	cmp	r4, #2
	addeq	r7, r0, #848	@ 0x350
	addne	r7, r5, r8
	mov	r4, r8
	ldrsb	r0, [r5]
	ldrsb	r1, [r7], #1
	add	r0, r0, r1
	mul	r1, r0, r3
	asr	r0, r1, #8
	tst	r0, #128	@ 0x80
	addne	r0, r0, #1
	strb	r0, [r5], #1
	subs	r4, r4, #1
	bgt	0x802b938
	add	r0, pc, #31
	bx	r0
	.thumb
	movs	r0, #0
	mov	r1, r8
	lsrs	r1, r1, #3
	bcc.n	0x802b972
	stmia	r5!, {r0}
	lsrs	r1, r1, #1
	bcc.n	0x802b97a
	stmia	r5!, {r0}
	stmia	r5!, {r0}
	stmia	r5!, {r0}
	stmia	r5!, {r0}
	stmia	r5!, {r0}
	stmia	r5!, {r0}
	subs	r1, #1
	bgt.n	0x802b97a
	ldr	r4, [sp, #24]
	ldr	r0, [r4, #24]
	mov	ip, r0
	ldrb	r0, [r4, #6]
	adds	r4, #80	@ 0x50
_0802B990:
	str	r0, [sp, #4]
	ldr	r3, [r4, #36]	@ 0x24
	ldr	r0, [sp, #20]
	cmp	r0, #0
	beq.n	0x802b9b0
	ldr	r1, [pc, #16]	@ (0x802b9ac)
	ldrb	r1, [r1, #0]
	cmp	r1, #160	@ 0xa0
	bcs.n	0x802b9a4
	adds	r1, #228	@ 0xe4
	cmp	r1, r0
	bcc.n	0x802b9b0
	b.n	0x802bc32
	movs	r0, r0
	movs	r6, r0
	lsls	r0, r0, #16
	ldrb	r6, [r4, #0]
	movs	r0, #199	@ 0xc7
	tst	r0, r6
	bne.n	0x802b9ba
	b.n	0x802bc28
	movs	r0, #128	@ 0x80
	tst	r0, r6
	beq.n	0x802b9ea
	movs	r0, #64	@ 0x40
	tst	r0, r6
	bne.n	0x802b9fa
	movs	r6, #3
	strb	r6, [r4, #0]
	adds	r0, r3, #0
	adds	r0, #16
	str	r0, [r4, #40]	@ 0x28
	ldr	r0, [r3, #12]
	str	r0, [r4, #24]
	movs	r5, #0
	strb	r5, [r4, #9]
	str	r5, [r4, #28]
	ldrb	r2, [r3, #3]
	movs	r0, #192	@ 0xc0
	tst	r0, r2
	beq.n	0x802ba42
	movs	r0, #16
	orrs	r6, r0
	strb	r6, [r4, #0]
	b.n	0x802ba42
	ldrb	r5, [r4, #9]
	movs	r0, #4
	tst	r0, r6
	beq.n	0x802ba00
	ldrb	r0, [r4, #13]
	subs	r0, #1
	strb	r0, [r4, #13]
	bhi.n	0x802ba50
	movs	r0, #0
	strb	r0, [r4, #0]
	b.n	0x802bc28
	movs	r0, #64	@ 0x40
	tst	r0, r6
	beq.n	0x802ba20
	ldrb	r0, [r4, #7]
	muls	r5, r0
	lsrs	r5, r5, #8
	ldrb	r0, [r4, #12]
	cmp	r5, r0
	bhi.n	0x802ba50
	ldrb	r5, [r4, #12]
	cmp	r5, #0
	beq.n	0x802b9fa
	movs	r0, #4
	orrs	r6, r0
	strb	r6, [r4, #0]
	b.n	0x802ba50
	movs	r2, #3
	ands	r2, r6
	cmp	r2, #2
	bne.n	0x802ba3e
	ldrb	r0, [r4, #5]
	muls	r5, r0
	lsrs	r5, r5, #8
	ldrb	r0, [r4, #6]
	cmp	r5, r0
	bhi.n	0x802ba50
	adds	r5, r0, #0
	beq.n	0x802ba12
	subs	r6, #1
	strb	r6, [r4, #0]
	b.n	0x802ba50
	cmp	r2, #3
	bne.n	0x802ba50
	ldrb	r0, [r4, #4]
	adds	r5, r5, r0
	cmp	r5, #255	@ 0xff
	bcc.n	0x802ba50
	movs	r5, #255	@ 0xff
	subs	r6, #1
	strb	r6, [r4, #0]
	strb	r5, [r4, #9]
	ldr	r0, [sp, #24]
	ldrb	r0, [r0, #7]
	adds	r0, #1
	muls	r0, r5
	lsrs	r5, r0, #4
	ldrb	r0, [r4, #2]
	ldrb	r1, [r4, #3]
	adds	r0, r0, r1
	muls	r0, r5
	lsrs	r0, r0, #9
	strb	r0, [r4, #10]
	movs	r0, #16
	ands	r0, r6
	str	r0, [sp, #16]
	beq.n	0x802ba80
	adds	r0, r3, #0
	adds	r0, #16
	ldr	r1, [r3, #8]
	adds	r0, r0, r1
	str	r0, [sp, #12]
	ldr	r0, [r3, #12]
	subs	r0, r0, r1
	str	r0, [sp, #16]
	ldr	r5, [sp, #8]
	ldr	r2, [r4, #24]
	ldr	r3, [r4, #40]	@ 0x28
	add	r0, pc, #4	@ (adr r0, 0x802ba8c)
	bx	r0
	movs	r0, r0
	.arm
	str	r8, [sp]
	ldrb	sl, [r4, #10]
	lsl	sl, sl, #16
	ldrb	r0, [r4, #1]
	tst	r0, #8
	beq	0x802bb94
	cmp	r2, #4
	ble	0x802bb00
	subs	r2, r2, r8
	movgt	lr, #0
	bgt	0x802bad0
	mov	lr, r8
	add	r2, r2, r8
	sub	r8, r2, #4
	sub	lr, lr, r8
	ands	r2, r2, #3
	moveq	r2, #4
	ldr	r6, [r5]
	ldrsb	r0, [r3], #1
	mul	r1, sl, r0
	bic	r1, r1, #16711680	@ 0xff0000
	add	r6, r1, r6, ror #8
	adds	r5, r5, #1073741824	@ 0x40000000
	bcc	0x802bad4
	str	r6, [r5], #4
	subs	r8, r8, #4
	bgt	0x802bad0
	adds	r8, r8, lr
	beq	0x802bc14
	ldr	r6, [r5]
	ldrsb	r0, [r3], #1
	mul	r1, sl, r0
	bic	r1, r1, #16711680	@ 0xff0000
	add	r6, r1, r6, ror #8
	subs	r2, r2, #1
	beq	0x802bb64
	adds	r5, r5, #1073741824	@ 0x40000000
	bcc	0x802bb04
	str	r6, [r5], #4
	subs	r8, r8, #4
	bgt	0x802baa4
	b	0x802bc14
	ldr	r0, [sp, #24]
	cmp	r0, #0
	beq	0x802bb58
	ldr	r3, [sp, #20]
	rsb	r9, r2, #0
	adds	r2, r0, r2
	bgt	0x802bbe8
	sub	r9, r9, r0
	b	0x802bb48
	pop	{r4, ip}
	mov	r2, #0
	b	0x802bb74
	ldr	r2, [sp, #16]
	cmp	r2, #0
	ldrne	r3, [sp, #12]
	bne	0x802bb1c
	strb	r2, [r4]
	lsr	r0, r5, #30
	bic	r5, r5, #-1073741824	@ 0xc0000000
	rsb	r0, r0, #3
	lsl	r0, r0, #3
	ror	r6, r6, r0
	str	r6, [r5], #4
	b	0x802bc1c
	push	{r4, ip}
	ldr	lr, [r4, #28]
	ldr	r1, [r4, #32]
	mul	r4, ip, r1
	ldrsb	r0, [r3]
	ldrsb	r1, [r3, #1]!
	sub	r1, r1, r0
	ldr	r6, [r5]
	mul	r9, lr, r1
	add	r9, r0, r9, asr #23
	mul	ip, sl, r9
	bic	ip, ip, #16711680	@ 0xff0000
	add	r6, ip, r6, ror #8
	add	lr, lr, r4
	lsrs	r9, lr, #23
	beq	0x802bbf4
	bic	lr, lr, #1065353216	@ 0x3f800000
	subs	r2, r2, r9
	ble	0x802bb34
	subs	r9, r9, #1
	addeq	r0, r0, r1
	ldrsbne	r0, [r3, r9]!
	ldrsb	r1, [r3, #1]!
	sub	r1, r1, r0
	adds	r5, r5, #1073741824	@ 0x40000000
	bcc	0x802bbb4
	str	r6, [r5], #4
	subs	r8, r8, #4
	bgt	0x802bbb0
	sub	r3, r3, #1
	pop	{r4, ip}
	str	lr, [r4, #28]
	str	r2, [r4, #24]
	str	r3, [r4, #40]	@ 0x28
	ldr	r8, [sp]
	add	r0, pc, #1
	bx	r0
