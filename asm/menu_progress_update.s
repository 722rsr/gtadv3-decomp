@ GT Advance 3 - menu/results progress update
@ Region: file offset 0x00BD40-0x00BE20 (VMA 0x0800BD40-0x0800BE20).
@ Pure Thumb with armcc high-register prologue/epilogue and private pools;
@ byte-exact.

.thumb
.type sub_0800BD40, %function
sub_0800BD40:
_0800BD40:
	push {r4, r5, r6, r7, lr}
	mov r7, r9
	mov r6, r8
	push {r6, r7}
	sub sp, #60
	movs r0, #0
	str r0, [sp, #52]
	str r0, [sp, #56]
	add r4, sp, #40
	strh r0, [r4, #0]
	bl sub_0800BC04
	ldr r0, _0800BDE0
	mov r8, r0
	ldr r0, _0800BDE4
	add r0, r8
	movs r1, #0
	ldrsh r0, [r0, r1]
	bl 0x0802581C
	lsls r0, r0, #16
	asrs r5, r0, #16
	add r7, sp, #44
	add r6, sp, #28
	adds r0, r7, #0
	adds r1, r5, #0
	add r2, sp, #16
	adds r3, r6, #0
	bl 0x0800BC08
	ldr r3, _0800BDE8
	add r3, r8
	mov r9, r3
	movs r0, #0
	ldrsb r0, [r3, r0]
	cmp r0, #0
	beq.n _0800BDF4
	add r2, sp, #48
	add r0, sp, #52
	str r0, [sp, #0]
	add r0, sp, #56
	str r0, [sp, #4]
	add r0, sp, #16
	str r0, [sp, #8]
	str r6, [sp, #12]
	adds r0, r5, #0
	adds r1, r7, #0
	adds r3, r4, #0
	bl 0x0800BCD4
	mov r1, r9
	movs r0, #0
	ldrsb r0, [r1, r0]
	cmp r0, #0
	beq.n _0800BDF4
	ldr r1, [sp, #52]
	ldr r0, _0800BDEC
	add r0, r8
	strh r1, [r0, #0]
	ldr r0, [sp, #56]
	ldr r1, _0800BDF0
	add r1, r8
	strh r0, [r1, #0]
	lsls r0, r0, #16
	asrs r0, r0, #16
	mov r2, r8
	cmp r0, #1
	bne.n _0800BDFE
	lsls r1, r5, #3
	adds r1, r1, r5
	lsls r1, r1, #3
	adds r1, r1, r2
	movs r3, #196
	lsls r3, r3, #3
	adds r1, r1, r3
	adds r0, r6, #0
	ldmia r0!, {r3, r5, r6}
	stmia r1!, {r3, r5, r6}
	b.n _0800BDFE
	.short 0
_0800BDE0: .word 0x03001780
_0800BDE4: .word 0x00000576
_0800BDE8: .word 0x000010E4
_0800BDEC: .word 0x00000FC8
_0800BDF0: .word 0x00000FCA
_0800BDF4:
	ldr r2, _0800BE14
	ldr r5, _0800BE18
	adds r1, r2, r5
	movs r0, #0
	str r0, [r1, #0]
_0800BDFE:
	ldrh r1, [r4, #0]
	ldr r6, _0800BE1C
	adds r0, r2, r6
	strh r1, [r0, #0]
	add sp, #60
	pop {r3, r4}
	mov r8, r3
	mov r9, r4
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_0800BE14: .word 0x03001780
_0800BE18: .word 0x000010F4
_0800BE1C: .word 0x00000FCE
