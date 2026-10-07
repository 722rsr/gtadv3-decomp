@ GT Advance 3 - menu placement/tier updater
@ Region: file offset 0x00BE74-0x00C010 (VMA 0x0800BE74-0x0800C010).
@ Pure Thumb with armcc high-register prologue/epilogue and private pools;
@ byte-exact.

.thumb
.type sub_0800BE74, %function
sub_0800BE74:
_0800BE74:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	sub sp, #16
	movs r0, #0
	str r0, [sp, #12]
	ldr r6, _0800BF58
	ldr r1, _0800BF5C
	adds r0, r6, r1
	movs r2, #0
	ldrsh r0, [r0, r2]
	bl 0x0802581C
	ldr r3, _0800BF60
	adds r0, r6, r3
	movs r1, #0
	ldrsh r0, [r0, r1]
	ldr r2, _0800BF64
	adds r1, r6, r2
	movs r3, #0
	ldrsh r1, [r1, r3]
	ldr r3, _0800BF68
	adds r2, r6, r3
	movs r3, #0
	ldrsh r2, [r2, r3]
	bl 0x08025750
	adds r4, r0, #0
	ldr r1, _0800BF6C
	adds r0, r6, r1
	ldr r0, [r0, #0]
	str r0, [sp, #0]
	mov r1, sp
	ldr r2, _0800BF70
	adds r0, r6, r2
	ldrh r0, [r0, #0]
	strh r0, [r1, #4]
	add r0, sp, #8
	ldr r3, _0800BF74
	adds r1, r6, r3
	movs r2, #3
	bl 0x0800D95C
	ldr r0, _0800BF78
	mov r9, r0
	adds r0, r4, #0
	mov r1, r9
	bl 0x0802DE04
	ldr r2, _0800BF7C
	adds r1, r0, #0
	muls r1, r2
	mov sl, r1
	adds r0, r4, #0
	mov r1, r9
	bl 0x0802DE9C
	movs r3, #150
	lsls r3, r3, #1
	mov r8, r3
	mov r1, r8
	bl 0x0802DE04
	movs r7, #100
	muls r0, r7
	add sl, r0
	adds r0, r4, #0
	mov r1, r8
	bl 0x0802DE9C
	movs r1, #3
	bl 0x0802DE04
	add sl, r0
	ldr r5, [sp, #0]
	adds r0, r5, #0
	mov r1, r9
	bl 0x0802DF6C
	ldr r1, _0800BF7C
	adds r4, r0, #0
	muls r4, r1
	adds r0, r5, #0
	mov r1, r9
	bl 0x0802DFE4
	mov r1, r8
	bl 0x0802DF6C
	muls r0, r7
	adds r4, r4, r0
	adds r0, r5, #0
	mov r1, r8
	bl 0x0802DFE4
	movs r1, #3
	bl 0x0802DF6C
	adds r4, r4, r0
	mov r2, sl
	subs r1, r2, r4
	ldr r3, _0800BF80
	adds r6, r6, r3
	movs r0, #0
	ldrsb r0, [r6, r0]
	cmp r0, #0
	beq.n _0800BFE4
	cmp r1, #99
	bhi.n _0800BF84
	movs r0, #1
	str r0, [sp, #12]
	b.n _0800BF9A
	_0800BF58: .word 0x03001780
	_0800BF5C: .word 0x00000576
	_0800BF60: .word 0x00000FF2
	_0800BF64: .word 0x00000FF6
	_0800BF68: .word 0x0000103A
	_0800BF6C: .word 0x000010F4
	_0800BF70: .word 0x00000574
	_0800BF74: .word 0x00001088
	_0800BF78: .word 0x00004650
	_0800BF7C: .word 0x00001770
	_0800BF80: .word 0x000010E4
_0800BF84:
	adds r0, r1, #0
	subs r0, #100
	cmp r0, #99
	bhi.n _0800BF92
	movs r1, #2
	str r1, [sp, #12]
	b.n _0800BF9A
_0800BF92:
	cmp r1, #199
	ble.n _0800BF9A
	movs r2, #3
	str r2, [sp, #12]
_0800BF9A:
	ldr r2, _0800C000
	ldr r3, _0800C004
	adds r6, r2, r3
	movs r1, #0
	ldrsh r0, [r6, r1]
	adds r3, #4
	adds r5, r2, r3
	movs r3, #0
	ldrsh r1, [r5, r3]
	ldr r3, _0800C008
	adds r4, r2, r3
	movs r3, #0
	ldrsh r2, [r4, r3]
	bl 0x08025E98
	ldr r1, [sp, #0]
	cmp r0, r1
	bhi.n _0800BFD2
	movs r1, #0
	ldrsh r0, [r6, r1]
	movs r2, #0
	ldrsh r1, [r5, r2]
	movs r3, #0
	ldrsh r2, [r4, r3]
	bl 0x08025E98
	cmp r0, #0
	bne.n _0800BFE4
_0800BFD2:
	movs r1, #0
	ldrsh r0, [r6, r1]
	movs r2, #0
	ldrsh r1, [r5, r2]
	movs r3, #0
	ldrsh r2, [r4, r3]
	ldr r3, [sp, #0]
	bl 0x08025E70
_0800BFE4:
	ldr r0, _0800C000
	ldr r1, _0800C00C
	adds r0, r0, r1
	mov r2, sp
	ldrb r2, [r2, #12]
	strb r2, [r0, #0]
	add sp, #16
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_0800C000: .word 0x03001780
_0800C004: .word 0x00000FF2
_0800C008: .word 0x0000103A
_0800C00C: .word 0x000010E5
