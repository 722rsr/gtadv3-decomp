@ SDK EEPROM sector reader.
@ VMA 0x0802DBA4-0x0802DC54 (176 bytes), pure Thumb with embedded pools.

	.thumb
	.type sub_0802DBA4, %function
sub_0802DBA4:
	push {r4, r5, r6, lr}
	sub sp, #136
	adds r5, r1, #0
	lsls r0, #16
	lsrs r3, r0, #16
	ldr r0, .L_dbbc
	ldr r0, [r0]
	ldrh r0, [r0, #4]
	cmp r3, r0
	bcc .L_dbc4
	ldr r0, .L_dbc0
	b .L_dc46
.L_dbbc:
	.word 0x0203FD54
.L_dbc0:
	.word 0x000080FF
.L_dbc4:
	ldr r0, .L_dc50
	adds r6, r0, #0
	ldr r0, [r0]
	ldrb r1, [r0, #8]
	lsls r0, r1, #1
	mov r4, sp
	adds r2, r0, r4
	adds r2, #2
	movs r4, #0
	cmp r4, r1
	bcs .L_dbee
.L_dbda:
	strh r3, [r2]
	subs r2, #2
	lsrs r3, #1
	adds r0, r4, #1
	lsls r0, #24
	lsrs r4, r0, #24
	ldr r0, [r6]
	ldrb r0, [r0, #8]
	cmp r4, r0
	bcc .L_dbda
.L_dbee:
	movs r0, #1
	strh r0, [r2]
	subs r2, #2
	strh r0, [r2]
	movs r4, #208
	lsls r4, #20
	ldr r0, .L_dc50
	ldr r0, [r0]
	ldrb r2, [r0, #8]
	adds r2, #3
	mov r0, sp
	adds r1, r4, #0
	bl sub_0802DB24
	adds r0, r4, #0
	mov r1, sp
	movs r2, #68
	bl sub_0802DB24
	add r2, sp, #8
	adds r5, #6
	movs r4, #0
	movs r6, #1
.L_dc1c:
	movs r1, #0
	movs r3, #0
.L_dc20:
	lsls r1, #17
	ldrh r0, [r2]
	ands r0, r6
	lsrs r1, #16
	orrs r1, r0
	adds r2, #2
	adds r0, r3, #1
	lsls r0, #24
	lsrs r3, r0, #24
	cmp r3, #15
	bls .L_dc20
	strh r1, [r5]
	subs r5, #2
	adds r0, r4, #1
	lsls r0, #24
	lsrs r4, r0, #24
	cmp r4, #3
	bls .L_dc1c
	movs r0, #0
.L_dc46:
	add sp, #136
	pop {r4, r5, r6}
	pop {r1}
	bx r1
	.short 0
.L_dc50:
	.word 0x0203FD54
