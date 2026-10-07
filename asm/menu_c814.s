@ GT Advance 3 - menu record poller
@ Region: file offset 0x00C814-0x00C884 (VMA 0x0800C814-0x0800C884).
@ Pure Thumb; byte-exact transcription.

.thumb
.type sub_0800C814, %function
sub_0800C814:
_0800C814:
	push {r4, r5, r6, r7, lr}
	adds r5, r0, #0
	movs r6, #0
	movs r7, #0
	movs r4, #0
	b.n _0800C84A
_0800C820:
	adds r0, r4, #0
	movs r1, #1
	bl 0x08002178
	lsls r0, r0, #16
	lsrs r1, r0, #16
	movs r0, #1
	ands r0, r1
	cmp r0, #0
	beq.n _0800C836
	movs r6, #1
_0800C836:
	movs r2, #2
	adds r0, r1, #0
	ands r0, r2
	cmp r0, #0
	beq.n _0800C848
	cmp r4, #0
	bne.n _0800C846
	movs r7, #1
_0800C846:
	strh r2, [r5, #4]
_0800C848:
	adds r4, #1
_0800C84A:
	bl 0x08002140
	cmp r4, r0
	blt.n _0800C820
	cmp r6, #0
	beq.n _0800C86E
	movs r0, #5
	bl 0x08004BFC
	movs r0, #0
	ldrsh r1, [r5, r0]
	movs r0, #0
	bl 0x08004C84
	movs r0, #1
	bl 0x08004EC0
	b.n _0800C87C
_0800C86E:
	cmp r7, #0
	beq.n _0800C87C
	movs r0, #1
	bl 0x08004EA8
	movs r0, #1
	strb r0, [r5, #28]
_0800C87C:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
