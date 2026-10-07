@ GT Advance 3 - award/collection grid copy twins
@ Region: file offset 0x00BA3C-0x00BC08 (VMA 0x0800BA3C-0x0800BC08).
@ Pure Thumb with armcc high-register prologues and private pools; byte-exact.

.thumb
.type sub_0800BA3C, %function
sub_0800BA3C:
_0800BA3C:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	sub sp, #76
	adds r4, r0, #0
	str r1, [sp, #64]
	str r2, [sp, #60]
	movs r0, #0
	mov r8, r0
	movs r2, #0
	ldr r1, _0800BB00
	mov r9, r1
	lsls r0, r4, #3
	adds r0, r0, r4
	lsls r0, r0, #3
	ldr r5, _0800BB04
	mov sl, r5
	mov ip, r2
	add r1, sl
	adds r0, r0, r1
	str r0, [sp, #72]
	mov r3, sp
_0800BA6C:
	lsls r6, r4, #3
	str r6, [sp, #68]
	mov r7, r8
	cmp r7, #1
	beq.n _0800BAA2
	ldr r0, [sp, #64]
	ldr r1, [r0, #0]
	ldr r5, [sp, #72]
	ldr r0, [r5, #0]
	cmp r1, r0
	bcs.n _0800BAA2
	adds r0, r3, #0
	ldr r1, [sp, #64]
	ldmia r1!, {r5, r6, r7}
	stmia r0!, {r5, r6, r7}
	ldr r6, _0800BB08
	strh r2, [r6, #0]
	lsls r0, r2, #16
	cmp r0, #0
	bne.n _0800BA9A
	movs r0, #1
	ldr r7, [sp, #60]
	str r0, [r7, #0]
_0800BA9A:
	adds r3, #12
	adds r2, #1
	movs r0, #1
	mov r8, r0
_0800BAA2:
	cmp r2, #4
	bgt.n _0800BABC
_0800BAA6:
	ldr r1, [sp, #68]
	adds r0, r1, r4
	lsls r0, r0, #3
	add r0, ip
	add r0, r9
	adds r1, r3, #0
	add r0, sl
	ldmia r0!, {r5, r6, r7}
	stmia r1!, {r5, r6, r7}
	adds r3, #12
	adds r2, #1
_0800BABC:
	movs r0, #12
	add ip, r0
	ldr r1, [sp, #72]
	adds r1, #12
	str r1, [sp, #72]
	cmp r2, #4
	ble.n _0800BA6C
_0800BACA:
	ldr r1, _0800BB00
	ldr r2, [sp, #68]
	adds r0, r2, r4
	lsls r0, r0, #3
	mov r3, sp
	adds r2, r0, r1
	ldr r5, _0800BB04
	mov r8, r5
	movs r4, #4
_0800BADC:
	mov r6, r8
	adds r1, r2, r6
	adds r0, r3, #0
	ldmia r0!, {r5, r6, r7}
	stmia r1!, {r5, r6, r7}
	adds r3, #12
	adds r2, #12
	subs r4, #1
	cmp r4, #0
	bge.n _0800BADC
	add sp, #76
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_0800BB00: .word 0x03001780
_0800BB04: .word 0x000005E4
_0800BB08: .word 0x0300274C

.type sub_0800BB0C, %function
sub_0800BB0C:
_0800BB0C:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	sub sp, #72
	str r0, [sp, #60]
	mov ip, r1
	movs r0, #0
	str r0, [sp, #64]
	ldr r1, _0800BBEC
	ldr r2, _0800BBF0
	adds r4, r1, r2
	strh r0, [r4, #0]
	ldr r5, _0800BBF4
	adds r0, r1, r5
	add r6, sp, #64
	ldrh r6, [r6, #0]
	strh r6, [r0, #0]
	ldr r7, _0800BBF8
	adds r3, r1, r7
	add r0, sp, #64
	ldrh r0, [r0, #0]
	strh r0, [r3, #0]
	movs r5, #0
	ldr r2, [sp, #60]
	lsls r0, r2, #3
	adds r0, r0, r2
	lsls r0, r0, #3
	ldr r6, _0800BBFC
	adds r6, r6, r1
	mov r8, r5
	adds r3, r0, r6
	mov r2, sp
	mov sl, r4
	mov r9, r1
	_0800BB54:
	ldr r7, [sp, #60]
	lsls r7, r7, #3
	str r7, [sp, #68]
	ldr r0, [sp, #64]
	cmp r0, #1
	beq.n _0800BB8A
	mov r4, ip
	ldr r1, [r4, #0]
	ldr r0, [r3, #0]
	cmp r1, r0
	bls.n _0800BB8A
	adds r0, r2, #0
	mov r1, ip
	ldmia r1!, {r4, r6, r7}
	stmia r0!, {r4, r6, r7}
	ldr r6, _0800BC00
	strh r5, [r6, #0]
	lsls r0, r5, #16
	cmp r0, #0
	bne.n _0800BB82
	movs r0, #1
	mov r7, sl
	strh r0, [r7, #0]
_0800BB82:
	adds r2, #12
	adds r5, #1
	movs r0, #1
	str r0, [sp, #64]
_0800BB8A:
	cmp r5, #4
	bgt.n _0800BBA8
_0800BB8E:
	ldr r1, [sp, #68]
	ldr r4, [sp, #60]
	adds r0, r1, r4
	lsls r0, r0, #3
	add r0, r8
	add r0, r9
	adds r1, r2, #0
	ldr r6, _0800BBFC
	adds r0, r0, r6
	ldmia r0!, {r4, r6, r7}
	stmia r1!, {r4, r6, r7}
	adds r2, #12
	adds r5, #1
_0800BBA8:
	movs r7, #12
	add r8, r7
	adds r3, #12
	cmp r5, #4
	ble.n _0800BB54
_0800BBB2:
	ldr r1, _0800BBEC
	ldr r2, [sp, #68]
	ldr r3, [sp, #60]
	adds r0, r2, r3
	lsls r0, r0, #3
	mov r3, sp
	adds r2, r0, r1
	ldr r4, _0800BBFC
	mov r8, r4
	movs r4, #4
_0800BBC6:
	mov r5, r8
	adds r1, r2, r5
	adds r0, r3, #0
	ldmia r0!, {r5, r6, r7}
	stmia r1!, {r5, r6, r7}
	adds r3, #12
	adds r2, #12
	subs r4, #1
	cmp r4, #0
	bge.n _0800BBC6
	add sp, #72
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
_0800BBEC: .word 0x03001780
_0800BBF0: .word 0x00000FC8
_0800BBF4: .word 0x00000FCA
_0800BBF8: .word 0x00000FCC
_0800BBFC: .word 0x000005E4
_0800BC00: .word 0x0300274C
	.type sub_0800BC04, %function
sub_0800BC04:
_0800BC04:
	.short 0x4770
	.short 0
award_twins_end:
