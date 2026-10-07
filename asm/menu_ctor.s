@ GT Advance 3 - menu/save scene constructor
@ Region: file offset 0x00C010-0x00C0F0 (VMA 0x0800C010-0x0800C0F0).
@ Pure Thumb with armcc high-register prologue/epilogue and private pools;
@ byte-exact.

.thumb
.type sub_0800C010, %function
sub_0800C010:
_0800C010:
	push {r4, r5, r6, lr}
	mov r6, r9
	mov r5, r8
	push {r5, r6}
	sub sp, #84
	adds r5, r0, #0
	mov r4, sp
	movs r0, #0
	strh r0, [r4, #0]
	movs r6, #1
	movs r0, #1
	mov r9, r0
	mov r1, r9
	strh r1, [r4, #2]
	add r0, sp, #4
	movs r1, #0
	movs r2, #64
	bl 0x0802E104
	strb r6, [r4, #4]
	movs r0, #24
	strb r0, [r4, #6]
	strb r6, [r4, #7]
	movs r2, #206
	lsls r2, r2, #4
	adds r0, r5, r2
	str r0, [sp, #16]
	add r3, sp, #68
	mov r8, r3
	mov r1, r8
	ldr r0, _0800C0D8
	ldmia r0!, {r2, r3, r4}
	stmia r1!, {r2, r3, r4}
	ldr r0, [r0, #0]
	str r0, [r1, #0]
	bl 0x080056FC
	movs r4, #231
	lsls r4, r4, #5
	adds r0, r5, r4
	movs r1, #16
	bl 0x080055D8
	adds r0, r5, #0
	adds r0, #92
	mov r1, r9
	strh r1, [r0, #0]
	adds r4, r5, #0
	adds r4, #8
	ldr r2, _0800C0DC
	mov r0, sp
	adds r1, r4, #0
	bl 0x0802D974
	adds r6, r5, #0
	adds r6, #76
	ldr r2, _0800C0E0
	mov r0, r8
	adds r1, r6, #0
	bl 0x0802D974
	movs r0, #2
	adds r1, r4, #0
	movs r2, #0
	bl 0x08004D4C
	adds r0, r4, #0
	movs r1, #0
	bl 0x080050E8
	adds r0, r6, #0
	bl 0x080032E0
	adds r0, r5, #0
	adds r0, #96
	bl 0x08003104
	ldr r0, _0800C0E4
	bl 0x08004DC8
	movs r2, #239
	lsls r2, r2, #5
	adds r4, r5, r2
	str r0, [r4, #0]
	ldr r0, _0800C0E8
	bl 0x08004DC8
	ldr r3, _0800C0EC
	adds r5, r5, r3
	str r0, [r5, #0]
	ldr r0, [r4, #0]
	bl 0x08004EF0
	add sp, #84
	pop {r3, r4}
	mov r8, r3
	mov r9, r4
	pop {r4, r5, r6}
	pop {r0}
	bx r0
_0800C0D8: .word 0x0805F6B4
_0800C0DC: .word 0x04000011
_0800C0E0: .word 0x04000004
_0800C0E4: .word 0x080CDFD0
_0800C0E8: .word 0x080CDFD8
_0800C0EC: .word 0x00001DE4
menu_ctor_end:
