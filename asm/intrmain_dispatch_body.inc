	push	{r4, r5, r6, r7, lr}
	mov	r7, sl
	mov	r6, r9
	mov	r5, r8
	push	{r5, r6, r7}
	adds	r6, r0, #0
	bl 0x080014A4
	cmp	r0, #0
	beq _08000F9A
	b 0x08001360
_08000F9A:
	adds	r0, r6, #0
	adds	r0, #74
	ldrb	r1, [r0, #0]
	mov	sl, r0
	cmp	r1, #15
	bls _08000FAE
	subs	r0, r1, #1
	mov	r1, sl
	strb	r0, [r1, #0]
	b _08001360
_08000FAE:
	adds	r1, r6, #0
	adds	r1, #72
	ldrb	r0, [r1, #0]
	cmp	r0, #0
	beq _08000FD8
	movs	r0, #0
	strb	r0, [r1, #0]
	ldr r0, _08000FD4
	ldrh	r0, [r0, #0]
	movs	r4, #252
	ands	r4, r0
	cmp	r4, #8
	beq _08000FD8
	adds	r0, r6, #0
	bl 0x08000F48
	movs	r0, #8
	eors	r0, r4
	b _08001362
_08000FD4: .4byte 0x04000128
_08000FD8:
	ldrb	r2, [r6, #24]
	cmp	r2, #223
	bls _0800102A
	adds	r0, r6, #0
	bl sub_080014B4
	adds	r4, r0, #0
	cmp	r4, #0
	beq _08000FEC
	b _08001362
_08000FEC:
	adds	r0, r6, #0
	adds	r0, #75
	ldrb	r0, [r0, #0]
	cmp	r0, #1
	bne _08001008
	ldrb	r0, [r6, #24]
	cmp	r0, #225
	bls _08001008
	adds	r0, r6, #0
	bl _080014A4
	cmp	r0, #0
	bne _08001008
	b _08001350
_08001008:
	adds	r0, r6, #0
	bl _080014A4
	cmp	r0, #0
	beq _08001014
	b _08001360
_08001014:
	ldrh	r0, [r6, #22]
	cmp	r0, #0
	bne _08001024
	adds	r0, r6, #0
	bl 0x08000F48
	movs	r0, #113
	b _08001362
_08001024:
	subs	r0, #1
	strh	r0, [r6, #22]
	b _08001360
_0800102A:
	ldrb	r0, [r6, #24]
	cmp	r0, #2
	bne _08001032
	b _08001168
_08001032:
	cmp	r0, #2
	bgt _08001040
	cmp	r0, #0
	beq _0800104E
	cmp	r0, #1
	beq _0800110C
	b _080012A4
_08001040:
	cmp	r0, #208
	bne _08001046
	b _080011B4
_08001046:
	cmp	r0, #209
	bne _0800104C
	b _08001250
_0800104C:
	b _080012A4
_0800104E:
	movs	r5, #14
	movs	r4, #3
	ldr r0, _08001094
	ldrh	r0, [r0, #6]
	adds	r1, r0, #0
	ldr r0, _08001098
	ldrb	r2, [r6, #30]
	adds	r7, r2, #0
	cmp	r1, r0
	bne _08001076
	adds	r3, r1, #0
	ldr r1, _0800109C
_08001066:
	asrs	r5, r5, #1
	subs	r1, #2
	subs	r4, #1
	cmp	r4, #0
	beq _08001076
	ldrh	r0, [r1, #0]
	cmp	r0, r3
	beq _08001066
_08001076:
	movs	r0, #14
	ands	r5, r0
	strb	r5, [r6, #29]
	movs	r4, #3
	ldr r0, _08001094
	ldrh	r0, [r0, #6]
	adds	r3, r0, #0
	asrs	r0, r2, #3
	movs	r1, #1
	ands	r0, r1
	cmp	r0, #0
	beq _080010A4
	ldr r0, _080010A0
	b _080010CA
	movs	r0, r0
_08001094: .4byte 0x04000120
_08001098: .4byte 0x0000FFFF
_0800109C: .4byte 0x04000126
_080010A0: .4byte 0x00007208
_080010A4:
	subs	r4, #1
	cmp	r4, #0
	beq _080010D0
	lsls	r0, r4, #1
	ldr r1, _080010F8
	adds	r0, r0, r1
	ldrh	r0, [r0, #0]
	adds	r3, r0, #0
	adds	r0, r2, #0
	asrs	r0, r4
	movs	r1, #1
	ands	r0, r1
	cmp	r0, #0
	beq _080010A4
	adds	r0, r1, #0
	lsls	r0, r4
	movs	r1, #228
	lsls	r1, r1, #7
	orrs	r0, r1
_080010CA:
	cmp	r3, r0
	beq _080010A4
	movs	r5, #0
_080010D0:
	adds	r0, r5, #0
	ands	r0, r7
	strb	r0, [r6, #30]
	cmp	r5, #0
	bne _080010E0
	movs	r0, #15
	mov	r2, sl
	strb	r0, [r2, #0]
_080010E0:
	mov	r1, sl
	ldrb	r0, [r1, #0]
	cmp	r0, #0
	bne _080010FC
	ldrb	r2, [r6, #29]
	ldrb	r0, [r6, #30]
	cmp	r2, r0
	beq _08001102
	adds	r0, r6, #0
	bl sub_080013BC
	b _0800110C
_080010F8: .4byte 0x04000120
_080010FC:
	subs	r0, #1
	mov	r1, sl
	strb	r0, [r1, #0]
_08001102:
	movs	r2, #196
	lsls	r2, r2, #7
	adds	r0, r2, #0
	ldrb	r1, [r6, #30]
	b _0800120C
_0800110C:
	adds	r1, r6, #0
	adds	r1, #73
	movs	r0, #0
	strb	r0, [r1, #0]
	movs	r4, #3
	adds	r7, r1, #0
	ldr r5, _08001160
_0800111A:
	lsls	r0, r4, #1
	ldr r2, _08001164
	adds	r0, r0, r2
	ldrh	r0, [r0, #0]
	adds	r3, r0, #0
	asrs	r0, r3, #8
	subs	r2, r4, #1
	cmp	r0, #114
	bne _08001144
	lsls	r0, r2, #1
	adds	r0, r0, r5
	strh	r3, [r0, #0]
	movs	r0, #255
	ands	r3, r0
	movs	r0, #1
	lsls	r0, r4
	cmp	r3, r0
	bne _08001144
	ldrb	r0, [r1, #0]
	orrs	r3, r0
	strb	r3, [r1, #0]
_08001144:
	adds	r4, r2, #0
	cmp	r4, #0
	bne _0800111A
	ldrb	r1, [r6, #29]
	ldrb	r2, [r7, #0]
	cmp	r1, r2
	bne _08001102
	movs	r0, #2
	strb	r0, [r6, #24]
	movs	r1, #194
	lsls	r1, r1, #7
	adds	r0, r1, #0
	ldrb	r1, [r7, #0]
	b _0800120C
_08001160: .4byte 0x03000000
_08001164: .4byte 0x04000120
_08001168:
	movs	r4, #3
	adds	r7, r6, #0
	adds	r7, #73
	adds	r5, r7, #0
	movs	r2, #1
	mov	ip, r2
	ldr r0, _080011AC
	mov	r9, r0
	ldr r1, _080011B0
	mov	r8, r1
_0800117C:
	ldrb	r3, [r5, #0]
	adds	r0, r3, #0
	asrs	r0, r4
	mov	r2, ip
	ands	r0, r2
	subs	r2, r4, #1
	cmp	r0, #0
	beq _080011A4
	lsls	r0, r4, #1
	add	r0, r8
	ldrh	r1, [r0, #0]
	lsls	r0, r2, #1
	add	r0, r9
	ldrh	r0, [r0, #0]
	cmp	r1, r0
	beq _080011A4
	mov	r0, ip
	lsls	r0, r4
	eors	r3, r0
	strb	r3, [r5, #0]
_080011A4:
	adds	r4, r2, #0
	cmp	r4, #0
	bne _0800117C
	b _08001308
_080011AC: .4byte 0x03000000
_080011B0: .4byte 0x04000120
_080011B4:
	movs	r5, #1
	movs	r4, #3
	adds	r7, r6, #0
	adds	r7, #73
	movs	r0, #25
	adds	r0, r0, r6
	mov	ip, r0
	ldr r1, _08001218
	mov	r8, r1
_080011C6:
	lsls	r0, r4, #1
	ldr r2, _0800121C
	adds	r0, r0, r2
	ldrh	r0, [r0, #0]
	adds	r3, r0, #0
	subs	r2, r4, #1
	mov	r1, ip
	adds	r0, r1, r2
	strb	r3, [r0, #0]
	ldrb	r1, [r7, #0]
	asrs	r1, r4
	movs	r0, #1
	ands	r1, r0
	cmp	r1, #0
	beq _080011FA
	asrs	r0, r3, #8
	subs	r0, #114
	cmp	r0, #1
	bls _080011EE
	b _08001356
_080011EE:
	lsls	r0, r2, #1
	add	r0, r8
	ldrh	r0, [r0, #0]
	cmp	r3, r0
	bne _080011FA
	movs	r5, #0
_080011FA:
	adds	r4, r2, #0
	cmp	r4, #0
	bne _080011C6
	cmp	r5, #0
	bne _08001220
	movs	r2, #198
	lsls	r2, r2, #7
	adds	r0, r2, #0
	ldrb	r1, [r6, #28]
_0800120C:
	orrs	r1, r0
	adds	r0, r6, #0
	bl sub_08001370
	b _08001362
	movs	r0, r0
_08001218: .4byte 0x03000000
_0800121C: .4byte 0x04000120
_08001220:
	movs	r0, #209
	strb	r0, [r6, #24]
	movs	r5, #17
	movs	r4, #3
	mov	r0, ip
	adds	r0, #2
_0800122C:
	ldrb	r1, [r0, #0]
	adds	r5, r1, r5
	subs	r0, #1
	subs	r4, #1
	cmp	r4, #0
	bne _0800122C
	strb	r5, [r6, #20]
	movs	r0, #255
	ands	r5, r0
	movs	r2, #200
	lsls	r2, r2, #7
	adds	r0, r2, #0
	orrs	r5, r0
	adds	r0, r6, #0
	adds	r1, r5, #0
	bl sub_08001370
	b _08001362
_08001250:
	movs	r4, #3
	adds	r7, r6, #0
	adds	r7, #73
	ldrb	r1, [r7, #0]
	ldr r2, _08001290
	movs	r5, #1
_0800125C:
	ldrh	r0, [r2, #0]
	adds	r3, r0, #0
	adds	r0, r1, #0
	asrs	r0, r4
	ands	r0, r5
	cmp	r0, #0
	beq _08001270
	asrs	r0, r3, #8
	cmp	r0, #115
	bne _08001356
_08001270:
	subs	r2, #2
	subs	r4, #1
	cmp	r4, #0
	bne _0800125C
	adds	r0, r6, #0
	bl 0x0802D98C
	adds	r4, r0, #0
	cmp	r4, #0
	bne _08001294
	movs	r0, #224
	strb	r0, [r6, #24]
	adds	r0, #176
	strh	r0, [r6, #22]
	b _08001360
	movs	r0, r0
_08001290: .4byte 0x04000126
_08001294:
	adds	r0, r6, #0
	bl 0x08000F48
	movs	r0, #30
	mov	r1, sl
	strb	r0, [r1, #0]
	movs	r0, #112
	b _08001362
_080012A4:
	movs	r4, #3
	adds	r7, r6, #0
	adds	r7, #73
	mov	ip, r7
	movs	r2, #1
	mov	r8, r2
_080012B0:
	mov	r0, ip
	ldrb	r5, [r0, #0]
	adds	r0, r5, #0
	asrs	r0, r4
	mov	r1, r8
	ands	r0, r1
	cmp	r0, #0
	beq _080012EA
	lsls	r0, r4, #1
	ldr r2, _08001304
	adds	r0, r0, r2
	ldrh	r0, [r0, #0]
	adds	r3, r0, #0
	asrs	r2, r3, #8
	ldrb	r0, [r6, #24]
	lsrs	r1, r0, #1
	movs	r0, #98
	subs	r0, r0, r1
	mov	r1, r8
	lsls	r1, r4
	cmp	r2, r0
	bne _080012E4
	movs	r0, #255
	ands	r3, r0
	cmp	r3, r1
	beq _080012EA
_080012E4:
	eors	r5, r1
	mov	r1, ip
	strb	r5, [r1, #0]
_080012EA:
	subs	r4, #1
	cmp	r4, #0
	bne _080012B0
	ldrb	r2, [r6, #24]
	cmp	r2, #196
	bne _08001308
	movs	r0, #14
	ldrb	r7, [r7, #0]
	ands	r0, r7
	strb	r0, [r6, #30]
	strb	r4, [r6, #24]
	b _08001102
	movs	r0, r0
_08001304: .4byte 0x04000120
_08001308:
	ldrb	r0, [r7, #0]
	cmp	r0, #0
	bne _08001318
	adds	r0, r6, #0
	bl 0x08000F48
	movs	r0, #80
	b _08001362
_08001318:
	ldrb	r0, [r6, #24]
	adds	r0, #2
	strb	r0, [r6, #24]
	lsls	r0, r0, #24
	lsrs	r0, r0, #24
	cmp	r0, #196
	bne _08001328
	b _08001102
_08001328:
	ldr	r0, [r6, #40]
	ldrb	r1, [r6, #24]
	adds	r0, r1, r0
	subs	r1, r0, #3
	ldrb	r1, [r1, #0]
	lsls	r1, r1, #8
	subs	r0, #4
	ldrb	r0, [r0, #0]
	orrs	r1, r0
	adds	r0, r6, #0
	bl sub_08001370
	adds	r4, r0, #0
	cmp	r4, #0
	bne _08001362
	adds	r0, r6, #0
	adds	r0, #75
	ldrb	r0, [r0, #0]
	cmp	r0, #1
	bne _08001360
_08001350:
	bl sub_080015B8
	b _08000FAE
_08001356:
	adds	r0, r6, #0
	bl 0x08000F48
	movs	r0, #96
	b _08001362
_08001360:
	movs	r0, #0
_08001362:
	pop	{r3, r4, r5}
	mov	r8, r3
	mov	r9, r4
	mov	sl, r5
	pop	{r4, r5, r6, r7}
	pop	{r1}
	bx	r1
