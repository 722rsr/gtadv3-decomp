@ GT Advance 3 - MTO sound driver: table interpolation helper
@ Region: file offset 0x02CED4-0x02CF7C (VMA 0x0802CED4-0x0802CF7C).
@ Pure Thumb with private ROM-table pools; byte-exact.

.thumb
.type sub_0802CED4, %function
sub_0802CED4:
_0802CED4:
	push {r4, r5, r6, r7, lr}
	lsls r0, r0, #24
	lsrs r0, r0, #24
	lsls r1, r1, #24
	lsrs r5, r1, #24
	lsls r2, r2, #24
	lsrs r2, r2, #24
	mov ip, r2
	cmp r0, #4
	bne.n _0802CF0C
	cmp r5, #20
	bhi.n _0802CEF0
	movs r5, #0
	b.n _0802CEFE
_0802CEF0:
	adds r0, r5, #0
	subs r0, #21
	lsls r0, r0, #24
	lsrs r5, r0, #24
	cmp r5, #59
	bls.n _0802CEFE
	movs r5, #59
_0802CEFE:
	ldr r0, _0802CF08
	adds r0, r5, r0
	ldrb r0, [r0, #0]
	b.n _0802CF6E
	.short 0
_0802CF08: .word 0x08061708
_0802CF0C:
	cmp r5, #35
	bhi.n _0802CF18
	movs r0, #0
	mov ip, r0
	movs r5, #0
	b.n _0802CF2A
_0802CF18:
	adds r0, r5, #0
	subs r0, #36
	lsls r0, r0, #24
	lsrs r5, r0, #24
	cmp r5, #130
	bls.n _0802CF2A
	movs r5, #130
	movs r1, #255
	mov ip, r1
_0802CF2A:
	ldr r3, _0802CF74
	adds r0, r5, r3
	ldrb r6, [r0, #0]
	ldr r4, _0802CF78
	movs r2, #15
	adds r0, r6, #0
	ands r0, r2
	lsls r0, r0, #1
	adds r0, r0, r4
	movs r7, #0
	ldrsh r1, [r0, r7]
	asrs r0, r6, #4
	adds r6, r1, #0
	asrs r6, r0
	adds r0, r5, #1
	adds r0, r0, r3
	ldrb r1, [r0, #0]
	adds r0, r1, #0
	ands r0, r2
	lsls r0, r0, #1
	adds r0, r0, r4
	movs r2, #0
	ldrsh r0, [r0, r2]
	asrs r1, r1, #4
	asrs r0, r1
	subs r0, r0, r6
	mov r7, ip
	muls r7, r0
	adds r0, r7, #0
	asrs r0, r0, #8
	adds r0, r6, r0
	movs r1, #128
	lsls r1, r1, #4
	adds r0, r0, r1
_0802CF6E:
	pop {r4, r5, r6, r7}
	pop {r1}
	bx r1
_0802CF74: .word 0x0806166C
_0802CF78: .word 0x080616F0
