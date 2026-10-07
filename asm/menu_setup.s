@ GT Advance 3 - menu/results setup initializer
@ Region: file offset 0x00BC08-0x00BCD4 (VMA 0x0800BC08-0x0800BCD4).
@ Pure Thumb with private IWRAM pools; byte-exact.

.thumb
.type sub_0800BC08, %function
sub_0800BC08:
_0800BC08:
	push {r4, r5, r6, r7, lr}
	adds r5, r1, #0
	adds r4, r2, #0
	adds r6, r3, #0
	ldr r3, _0800BC50
	ldr r2, _0800BC54
	adds r1, r3, r2
	movs r2, #0
	strh r2, [r1, #0]
	ldr r7, _0800BC58
	adds r1, r3, r7
	strh r2, [r1, #0]
	adds r7, #2
	adds r1, r3, r7
	strh r2, [r1, #0]
	lsls r1, r5, #3
	adds r1, r1, r5
	lsls r1, r1, #3
	ldr r7, _0800BC5C
	adds r2, r3, r7
	adds r1, r1, r2
	ldr r1, [r1, #0]
	str r1, [r0, #0]
	ldr r1, _0800BC60
	adds r0, r3, r1
	ldr r0, [r0, #0]
	str r0, [r4, #0]
	ldr r2, _0800BC64
	adds r0, r3, r2
	ldrh r0, [r0, #0]
	cmp r0, #7
	bne.n _0800BC6C
	ldr r7, _0800BC68
	adds r0, r3, r7
	b.n _0800BC70
	.short 0
_0800BC50: .word 0x03001780
_0800BC54: .word 0x00000FC8
_0800BC58: .word 0x00000FCA
_0800BC5C: .word 0x000005E4
_0800BC60: .word 0x000010F4
_0800BC64: .word 0x00000FBC
_0800BC68: .word 0x00000FC2
_0800BC6C:
	ldr r1, _0800BCA4
	adds r0, r3, r1
_0800BC70:
	ldrh r0, [r0, #0]
	strh r0, [r4, #4]
	adds r0, r4, #0
	adds r0, #8
	ldr r4, _0800BCA8
	adds r1, r4, #0
	movs r2, #3
	bl 0x0800D95C
	lsls r0, r5, #3
	adds r0, r0, r5
	lsls r0, r0, #3
	ldr r2, _0800BCAC
	adds r1, r4, r2
	adds r0, r0, r1
	ldr r0, [r0, #0]
	str r0, [r6, #0]
	adds r0, r4, #0
	subs r0, #204
	ldrh r0, [r0, #0]
	cmp r0, #7
	bne.n _0800BCB0
	adds r0, r4, #0
	subs r0, #198
	b.n _0800BCB4
	.short 0
_0800BCA4: .word 0x00000574
_0800BCA8: .word 0x03002808
_0800BCAC: .word 0xFFFFF598
_0800BCB0:
	ldr r7, _0800BCCC
	adds r0, r4, r7
_0800BCB4:
	ldrh r0, [r0, #0]
	strh r0, [r6, #4]
	adds r0, r6, #0
	adds r0, #8
	ldr r1, _0800BCD0
	movs r2, #3
	bl 0x0800D95C
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
_0800BCCC: .word 0xFFFFF4EC
_0800BCD0: .word 0x03002808
