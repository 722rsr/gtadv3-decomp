@ GT Advance 3 - resource wrapper helpers
@ Regions: file offset 0x026230-0x0263C4 (VMA 0x08026230-0x080263C4)
@ Exact ARMCC Thumb transcription; pools kept at original ROM offsets.
@ The live wrapper 0x080263C4 itself lives in resource_wrap.s and is not duplicated here.
@ Two pure-Thumb helpers plus private literal pools (word-aligned).

	.thumb

	.type sub_08026230, %function
sub_08026230:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	ldr r2, _0802629C
	lsls r0, r0, #3
	adds r0, #8
	ldr r2, [r2]
	adds r5, r2, r0
	str r1, [r5]
	ldr r0, [r2, #4]
	str r0, [r5, #4]
	adds r4, r0, #0
	ldr r0, _080262A0
	bl 0x08007498
	bl 0x0800748C
	adds r3, r0, #0
	movs r2, #0
	movs r0, #7
	mov ip, r0
	movs r1, #1
	mov r8, r1
	movs r6, #144
	lsls r6, r6, #4
_08026262:
	adds r1, r2, #0
	cmp r2, #0
	bge _0802626A
	adds r1, r2, #7
_0802626A:
	asrs r1, r1, #3
	adds r1, r3, r1
	adds r0, r2, #0
	mov r7, ip
	ands r0, r7
	ldrb r1, [r1]
	asrs r1, r0
	adds r0, r1, #0
	mov r1, r8
	ands r0, r1
	cmp r0, #0
	beq _08026284
	adds r4, r4, r6
_08026284:
	adds r2, #1
	cmp r2, #255
	ble _08026262
	ldr r0, _0802629C
	ldr r0, [r0]
	str r4, [r0, #4]
	adds r0, r5, #0
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r1}
	bx r1

	.align 2, 0
_0802629C:
	.word 0x03001670
_080262A0:
	.word 0x0879984C

	.type sub_080262A4, %function
sub_080262A4:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	sub sp, #8
	adds r5, r0, #0
	ldr r0, [r5, #4]
	mov r8, r0
	ldr r0, _080263BC
	bl 0x08007498
	str r0, [sp]
	ldr r4, _080263C0
	adds r0, r4, #0
	movs r1, #7
	bl 0x08007498
	bl 0x0800748C
	mov sl, r0
	ldr r1, [r5]
	adds r0, r4, #0
	bl 0x08007498
	bl 0x0800748C
	mov r9, r0
	ldr r0, [sp]
	movs r1, #0
	bl 0x08007498
	bl 0x0800748C
	movs r4, #128
	lsls r4, r4, #18
	adds r1, r4, #0
	bl 0x0802D988
	adds r3, r4, #0
	movs r4, #0
_080262F6:
	adds r0, r4, #0
	cmp r4, #0
	bge _080262FE
	adds r0, r4, #7
_080262FE:
	asrs r6, r0, #3
	mov r1, r9
	adds r0, r1, r6
	movs r5, #7
	ands r5, r4
	ldrb r0, [r0]
	asrs r0, r5
	movs r7, #1
	ands r0, r7
	cmp r0, #0
	beq _0802632A
	adds r0, r3, #0
	mov r1, r8
	movs r2, #144
	lsls r2, r2, #2
	str r3, [sp, #4]
	bl 0x0802D970
	movs r0, #144
	lsls r0, r0, #4
	add r8, r0
	ldr r3, [sp, #4]
_0802632A:
	mov r1, sl
	adds r0, r1, r6
	ldrb r0, [r0]
	asrs r0, r5
	ands r0, r7
	cmp r0, #0
	beq _0802633E
	movs r0, #144
	lsls r0, r0, #4
	adds r3, r3, r0
_0802633E:
	adds r4, #1
	cmp r4, #127
	ble _080262F6
	ldr r0, [sp]
	movs r1, #1
	bl 0x08007498
	bl 0x0800748C
	movs r4, #128
	lsls r4, r4, #18
	adds r1, r4, #0
	bl 0x0802D988
	adds r3, r4, #0
	movs r4, #128
_0802635E:
	adds r0, r4, #0
	cmp r4, #0
	bge _08026366
	adds r0, r4, #7
_08026366:
	asrs r6, r0, #3
	mov r1, r9
	adds r0, r1, r6
	movs r5, #7
	ands r5, r4
	ldrb r0, [r0]
	asrs r0, r5
	movs r7, #1
	ands r0, r7
	cmp r0, #0
	beq _08026392
	adds r0, r3, #0
	mov r1, r8
	movs r2, #144
	lsls r2, r2, #2
	str r3, [sp, #4]
	bl 0x0802D970
	movs r0, #144
	lsls r0, r0, #4
	add r8, r0
	ldr r3, [sp, #4]
_08026392:
	mov r1, sl
	adds r0, r1, r6
	ldrb r0, [r0]
	asrs r0, r5
	ands r0, r7
	cmp r0, #0
	beq _080263A6
	movs r0, #144
	lsls r0, r0, #4
	adds r3, r3, r0
_080263A6:
	adds r4, #1
	cmp r4, #255
	ble _0802635E
	add sp, #8
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0

	.align 2, 0
_080263BC:
	.word 0x083D7BE8
_080263C0:
	.word 0x0879984C
