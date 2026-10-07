@ GT Advance 3 - menu record helpers
@ Region: file offset 0x00CCEC-0x00CE2C (VMA 0x0800CCEC-0x0800CE2C).
@ Pure Thumb; three ARMCC handlers with private pools.

.thumb
.type sub_0800CCEC, %function
sub_0800CCEC:
_0800CCEC:
	push {r4, r5, lr}
	adds r5, r0, #0
	bl 0x08004B68
	movs r0, #0
	bl 0x08004CA8
	strh r0, [r5, #0]
	ldr r0, _0800CD3C
	ldr r1, _0800CD40
	adds r0, r0, r1
	ldrh r0, [r0, #0]
	strh r0, [r5, #6]
	bl 0x08026948
	ldr r2, _0800CD44
	adds r4, r5, r2
	str r0, [r4, #0]
	movs r2, #6
	ldrsh r1, [r5, r2]
	movs r2, #0
	bl 0x08026A4C
	ldr r0, [r4, #0]
	movs r1, #1
	bl 0x08026A58
	ldr r0, [r4, #0]
	movs r1, #0
	bl 0x08026A60
	ldr r0, [r4, #0]
	movs r2, #6
	ldrsh r1, [r5, r2]
	bl 0x08026938
	pop {r4, r5}
	pop {r0}
	bx r0
	.short 0
_0800CD3C: .word 0x03001780
_0800CD40: .word 0x00000574
_0800CD44: .word 0x00001018

.type sub_0800CD48, %function
sub_0800CD48:
_0800CD48:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	adds r5, r0, #0
	lsls r2, r2, #16
	lsrs r4, r2, #16
	adds r6, r4, #0
	bl 0x08004B68
	movs r1, #6
	ldrsh r0, [r5, r1]
	mov r8, r0
	movs r7, #0
	movs r0, #2
	ands r0, r4
	cmp r0, #0
	beq.n _0800CD70
	movs r0, #1
	bl 0x08004EA8
_0800CD70:
	movs r0, #1
	ands r0, r4
	cmp r0, #0
	beq.n _0800CD88
	ldr r0, _0800CDE0
	ldrh r1, [r5, #6]
	ldr r2, _0800CDE4
	adds r0, r0, r2
	strh r1, [r0, #0]
	movs r0, #6
	bl 0x08004EC0
_0800CD88:
	movs r0, #32
	ands r0, r4
	cmp r0, #0
	beq.n _0800CD92
	subs r7, #1
_0800CD92:
	movs r0, #16
	ands r6, r0
	cmp r6, #0
	beq.n _0800CD9C
	adds r7, #1
_0800CD9C:
	ldrh r1, [r5, #6]
	adds r0, r1, r7
	strh r0, [r5, #6]
	lsls r0, r0, #16
	cmp r0, #0
	bge.n _0800CDAC
	movs r0, #0
	strh r0, [r5, #6]
_0800CDAC:
	movs r2, #6
	ldrsh r0, [r5, r2]
	cmp r0, #96
	ble.n _0800CDB8
	movs r0, #96
	strh r0, [r5, #6]
_0800CDB8:
	movs r0, #6
	ldrsh r1, [r5, r0]
	cmp r1, r8
	beq.n _0800CDD6
	ldr r2, _0800CDE8
	adds r4, r5, r2
	ldr r0, [r4, #0]
	movs r2, #0
	bl 0x08026A4C
	ldr r0, [r4, #0]
	movs r2, #6
	ldrsh r1, [r5, r2]
	bl 0x08026938
_0800CDD6:
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_0800CDE0: .word 0x03001780
_0800CDE4: .word 0x00000574
_0800CDE8: .word 0x00001018

.type sub_0800CDEC, %function
sub_0800CDEC:
_0800CDEC:
	push {r4, lr}
	adds r4, r0, #0
	ldr r2, _0800CE20
	movs r0, #96
	movs r1, #15
	bl 0x080038A4
	ldr r0, _0800CE24
	ldrh r0, [r0, #12]
	lsls r2, r0, #16
	asrs r2, r2, #19
	movs r0, #120
	movs r1, #32
	bl 0x08003978
	ldr r0, _0800CE28
	adds r4, r4, r0
	ldr r0, [r4, #0]
	movs r1, #120
	movs r2, #100
	bl 0x08026A20
	pop {r4}
	pop {r0}
	bx r0
	.short 0
_0800CE20: .word 0x0805F88C
_0800CE24: .word 0x030035C0
_0800CE28: .word 0x00001018
menu_ccec_end:
