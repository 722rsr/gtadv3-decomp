@ GT Advance 3 - menu record/award update helper
@ Region: file offset 0x00BCD4-0x00BD40 (VMA 0x0800BCD4-0x0800BD40).
@ Pure Thumb with armcc high-register epilogue and private pools; byte-exact.

.thumb
.type sub_0800BCD4, %function
sub_0800BCD4:
_0800BCD4:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	mov ip, r0
	adds r5, r2, #0
	mov r8, r3
	ldr r7, [sp, #36]
	ldr r3, _0800BD34
	movs r4, #0
	movs r1, #0
	ldr r0, _0800BD38
	ldr r6, _0800BD3C
	adds r2, r0, r6
_0800BCEE:
	ldr r0, [r2, #0]
	cmp r0, r3
	bcs.n _0800BCFE
	cmp r0, #0
	beq.n _0800BCFE
	adds r3, r0, #0
	lsls r0, r1, #16
	lsrs r4, r0, #16
_0800BCFE:
	adds r2, #4
	adds r1, #1
	cmp r1, #2
	ble.n _0800BCEE
	ldr r0, [r7, #0]
	cmp r3, r0
	bge.n _0800BD1A
	ldr r0, _0800BD34
	cmp r3, r0
	beq.n _0800BD1A
	str r3, [r7, #0]
	movs r1, #1
	ldr r0, [sp, #28]
	str r1, [r0, #0]
_0800BD1A:
	str r3, [r5, #0]
	mov r0, r8
	strh r4, [r0, #0]
	mov r0, ip
	ldr r1, [sp, #32]
	ldr r2, [sp, #24]
	bl 0x0800BA3C
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_0800BD34: .word 0x001BB0A9
_0800BD38: .word 0x03001780
_0800BD3C: .word 0x000010E8
