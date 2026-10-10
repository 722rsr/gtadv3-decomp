@ game-side EEPROM checksum, read, and write/verify wrappers.
@ VMA 0x08005860-0x08005988 (296 bytes), pure Thumb with private pools.

	.thumb
	.global sub_08005860
	.type sub_08005860, %function
sub_08005860:
	push {r4, lr}
	ldr r3, .L_5880
	adds r4, r0, #0
	movs r2, #0
	lsrs r1, #2
	cmp r2, r1
	bcs .L_5878
.L_586e:
	ldmia r4!, {r0}
	adds r3, r3, r0
	adds r2, #1
	cmp r2, r1
	bcc .L_586e
.L_5878:
	adds r0, r3, #0
	pop {r4}
	pop {r1}
	bx r1
.L_5880:
	.word 0x4E4D4D47

	.type sub_08005884, %function
sub_08005884:
	push {r4, r5, r6, lr}
	adds r6, r0, #0
	adds r5, r1, #0
	adds r0, r2, #7
	cmp r0, #0
	bge .L_5892
	adds r0, #7
.L_5892:
	asrs r4, r0, #3
	bl 0x080029D8
	bl 0x08002A68
	cmp r4, #0
	ble .L_58c2
.L_58a0:
	lsls r0, r6, #16
	lsrs r0, #16
	adds r1, r5, #0
	bl sub_0802DBA4
	lsls r0, #16
	lsrs r1, r0, #16
	cmp r1, #0
	beq .L_58b8
	ldr r0, .L_58cc
	bl 0x0800295C
.L_58b8:
	adds r6, #1
	adds r5, #8
	subs r4, #1
	cmp r4, #0
	bne .L_58a0
.L_58c2:
	bl 0x08002A0C
	pop {r4, r5, r6}
	pop {r0}
	bx r0
.L_58cc:
	.word 0x0805BAB0

	.type sub_080058D0, %function
sub_080058D0:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	adds r5, r0, #0
	mov r8, r1
	adds r2, #7
	lsrs r4, r2, #3
	movs r7, #0
	ldr r6, .L_5974
	strh r7, [r6]
	bl 0x080029D8
	bl 0x08002A68
	movs r0, #9
	bl 0x08002AF4
	adds r1, r0, #0
	movs r0, #0
	bl sub_0802DA20
	lsls r0, #16
	lsrs r1, r0, #16
	cmp r1, #0
	beq .L_5908
	ldr r0, .L_5978
	bl 0x0800295C
.L_5908:
	ldr r2, .L_597c
	ldrh r0, [r2]
	movs r1, #8
	orrs r0, r1
	strh r0, [r2]
	movs r0, #1
	strh r0, [r6]
	cmp r4, #0
	beq .L_5966
	adds r6, r4, #0
.L_591c:
	mov r1, r8
	lsls r0, r1, #16
	lsrs r4, r0, #16
	adds r0, r4, #0
	adds r1, r5, #0
	bl sub_0802DC54
	lsls r0, #16
	lsrs r1, r0, #16
	cmp r1, #0
	beq .L_593e
	adds r7, #1
	cmp r7, #10
	ble .L_5962
	ldr r0, .L_5980
	bl 0x0800295C
.L_593e:
	adds r0, r4, #0
	adds r1, r5, #0
	bl sub_0802DD30
	lsls r0, #16
	lsrs r1, r0, #16
	cmp r1, #0
	beq .L_595a
	adds r7, #1
	cmp r7, #10
	ble .L_5962
	ldr r0, .L_5984
	bl 0x0800295C
.L_595a:
	adds r5, #8
	movs r0, #1
	add r8, r0
	subs r6, #1
.L_5962:
	cmp r6, #0
	bne .L_591c
.L_5966:
	bl 0x08002A0C
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
.L_5974:
	.word 0x04000208
.L_5978:
	.word 0x0805BABC
.L_597c:
	.word 0x04000200
.L_5980:
	.word 0x0805BAD0
.L_5984:
	.word 0x0805BAE0
save_checksum_end:
