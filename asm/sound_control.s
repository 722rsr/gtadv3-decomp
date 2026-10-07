@ GT Advance 3 - MTO sound driver: control/reset helpers
@ Region: file offset 0x02C990-0x02CB20 (VMA 0x0802C990-0x0802CB20).
@ Two pure-Thumb functions with local pools; byte-exact.

.thumb
.type sub_0802C990, %function
sub_0802C990:
_0802C990:
	push {r4, r5, r6, lr}
	adds r2, r0, #0
	ldr r0, _0802CA10
	ldr r4, [r0, #0]
	movs r0, #240
	lsls r0, r0, #12
	ands r0, r2
	lsrs r2, r0, #16
	movs r6, #0
	strb r2, [r4, #8]
	ldr r1, _0802CA14
	subs r0, r2, #1
	lsls r0, r0, #1
	adds r0, r0, r1
	ldrh r5, [r0, #0]
	str r5, [r4, #16]
	movs r0, #198
	lsls r0, r0, #3
	adds r1, r5, #0
	bl 0x0802DE04
	strb r0, [r4, #11]
	ldr r0, _0802CA18
	muls r0, r5
	ldr r1, _0802CA1C
	adds r0, r0, r1
	ldr r1, _0802CA20
	bl 0x0802DE04
	adds r1, r0, #0
	str r1, [r4, #20]
	movs r0, #128
	lsls r0, r0, #17
	bl 0x0802DE04
	adds r0, #1
	asrs r0, r0, #1
	str r0, [r4, #24]
	ldr r0, _0802CA24
	strh r6, [r0, #0]
	ldr r4, _0802CA28
	ldr r0, _0802CA2C
	adds r1, r5, #0
	bl 0x0802DE04
	negs r0, r0
	strh r0, [r4, #0]
	bl 0x0802CB84
	ldr r1, _0802CA30
_0802C9F4:
	ldrb r0, [r1, #0]
	cmp r0, #159
	beq.n _0802C9F4
	ldr r1, _0802CA30
_0802C9FC:
	ldrb r0, [r1, #0]
	cmp r0, #159
	bne.n _0802C9FC
	ldr r1, _0802CA24
	movs r0, #128
	strh r0, [r1, #0]
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.short 0x0000
	.align 2, 0
_0802CA10: .word 0x03007FF0
_0802CA14: .word 0x08061654
_0802CA18: .word 0x00091D1B
_0802CA1C: .word 0x00001388
_0802CA20: .word 0x00002710
_0802CA24: .word 0x04000102
_0802CA28: .word 0x04000100
_0802CA2C: .word 0x00044940
_0802CA30: .word 0x04000006
