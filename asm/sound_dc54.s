@ SDK EEPROM sector writer/verification setup.
@ VMA 0x0802DC54-0x0802DD30 (220 bytes), pure Thumb with embedded pools.

	.thumb
	.type sub_0802DC54, %function
sub_0802DC54:
	push {r4, r5, lr}
	sub sp, #164
	adds r5, r1, #0
	lsls r0, #16
	lsrs r4, r0, #16
	ldr r0, .L_dc6c
	ldr r0, [r0]
	ldrh r0, [r0, #4]
	cmp r4, r0
	bcc .L_dc74
	ldr r0, .L_dc70
	b .L_dd18
.L_dc6c:
	.word 0x0203FD54
.L_dc70:
	.word 0x000080FF
.L_dc74:
	ldr r0, .L_dcb4
	ldr r0, [r0]
	ldrb r0, [r0, #8]
	lsls r0, #1
	mov r1, sp
	adds r3, r0, r1
	adds r3, #132
	movs r0, #0
	strh r0, [r3]
	subs r3, #2
	movs r1, #0
.L_dc8a:
	ldrh r2, [r5]
	adds r5, #2
	movs r0, #0
.L_dc90:
	strh r2, [r3]
	subs r3, #2
	lsrs r2, #1
	adds r0, #1
	lsls r0, #24
	lsrs r0, #24
	cmp r0, #15
	bls .L_dc90
	adds r0, r1, #1
	lsls r0, #24
	lsrs r1, r0, #24
	cmp r1, #3
	bls .L_dc8a
	movs r1, #0
	ldr r0, .L_dcb4
	adds r2, r0, #0
	ldr r0, [r0]
	b .L_dcc6
.L_dcb4:
	.word 0x0203FD54
.L_dcb8:
	strh r4, [r3]
	subs r3, #2
	lsrs r4, #1
	adds r0, r1, #1
	lsls r0, #24
	lsrs r1, r0, #24
	ldr r0, [r2]
.L_dcc6:
	ldrb r0, [r0, #8]
	cmp r1, r0
	bcc .L_dcb8
	movs r0, #0
	strh r0, [r3]
	subs r3, #2
	movs r0, #1
	strh r0, [r3]
	movs r1, #208
	lsls r1, #20
	ldr r0, .L_dd20
	ldr r0, [r0]
	ldrb r2, [r0, #8]
	adds r2, #67
	mov r0, sp
	bl sub_0802DB24
	ldr r0, .L_dd24
	bl sub_0802DA58
	movs r4, #0
.L_dcf0:
	movs r1, #208
	lsls r1, #20
	movs r3, #1
	ldr r2, .L_dd28
.L_dcf8:
	ldrh r0, [r1]
	ands r0, r3
	cmp r0, #0
	bne .L_dd12
	ldrb r0, [r2]
	cmp r0, #0
	beq .L_dcf8
	ldrh r0, [r1]
	movs r1, #1
	ands r0, r1
	cmp r0, #0
	bne .L_dd12
	ldr r4, .L_dd2c
.L_dd12:
	bl sub_0802DAE0
	adds r0, r4, #0
.L_dd18:
	add sp, #164
	pop {r4, r5}
	pop {r1}
	bx r1
.L_dd20:
	.word 0x0203FD54
.L_dd24:
	.word 0x080C428C
.L_dd28:
	.word 0x0300176C
.L_dd2c:
	.word 0x0000C001
