@ SDK serial/EEPROM sector setup helper.
@ VMA 0x0802DB24-0x0802DBA4 (128 bytes), pure Thumb with private pools.

	.thumb
	.type sub_0802DB24, %function
sub_0802DB24:
	push {r4, r5, r6, lr}
	lsls r2, #16
	lsrs r2, #16
	ldr r4, .L_db84
	ldrh r3, [r4]
	adds r6, r3, #0
	movs r3, #0
	strh r3, [r4]
	ldr r5, .L_db88
	ldrh r4, [r5]
	ldr r3, .L_db8c
	ands r4, r3
	ldr r3, .L_db90
	ldr r3, [r3]
	ldrh r3, [r3, #6]
	orrs r4, r3
	strh r4, [r5]
	ldr r3, .L_db94
	str r0, [r3]
	ldr r0, .L_db98
	str r1, [r0]
	ldr r1, .L_db9c
	movs r0, #128
	lsls r0, #24
	orrs r2, r0
	str r2, [r1]
	adds r1, #2
	movs r2, #128
	lsls r2, #8
	adds r0, r2, #0
	ldrh r1, [r1]
	ands r0, r1
	cmp r0, #0
	beq .L_db78
	ldr r2, .L_dba0
	movs r0, #128
	lsls r0, #8
	adds r1, r0, #0
.L_db70:
	ldrh r0, [r2]
	ands r0, r1
	cmp r0, #0
	bne .L_db70
.L_db78:
	ldr r0, .L_db84
	strh r6, [r0]
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.short 0
.L_db84:
	.word 0x04000208
.L_db88:
	.word 0x04000204
.L_db8c:
	.word 0x0000F8FF
.L_db90:
	.word 0x0203FD54
.L_db94:
	.word 0x040000D4
.L_db98:
	.word 0x040000D8
.L_db9c:
	.word 0x040000DC
.L_dba0:
	.word 0x040000DE
