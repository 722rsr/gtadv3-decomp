@ GT Advance 3 - delayed save/timer event drain
@ Region: file offset 0x005AE4-0x005B3C (VMA 0x08005AE4-0x08005B3C).
@ Pure Thumb; byte-exact transcription.

.thumb
.type sub_08005AE4, %function
sub_08005AE4:
_08005AE4:
	push {r4, r5, r6, lr}
	movs r3, #0
	ldr r0, _08005B0C
	ldr r2, [r0, #0]
	adds r5, r2, #0
	adds r5, #16
	adds r6, r0, #0
	movs r4, #3
_08005AF4:
	ldrb r0, [r2, #0]
	cmp r0, #0
	beq.n _08005B14
	ldrb r0, [r2, #1]
	adds r1, r0, #0
	cmp r1, #0
	bne.n _08005B10
	strb r1, [r2, #0]
	ldr r0, [r2, #0]
	stmia r5!, {r0}
	adds r3, #1
	b.n _08005B14
_08005B0C: .word 0x030003D4
_08005B10:
	subs r0, #1
	strb r0, [r2, #1]
_08005B14:
	subs r4, #1
	adds r2, #4
	cmp r4, #0
	bge.n _08005AF4
	ldr r0, [r6, #0]
	strh r3, [r0, #34]
	ldrh r2, [r0, #32]
	subs r1, r2, r3
	strh r1, [r0, #32]
	cmp r3, #0
	ble.n _08005B34
	movs r0, #11
	movs r1, #0
	movs r2, #0
	bl 0x08004D4C
_08005B34:
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.short 0
