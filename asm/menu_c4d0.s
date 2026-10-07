@ menu record-20 constructor.
@ VMA 0x0800C4D0-0x0800C604 (564 bytes), pure Thumb with private pools.

	.thumb
	.type sub_0800C4D0, %function
sub_0800C4D0:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	sub sp, #84
	adds r5, r0, #0
	mov r0, sp
	movs r2, #0
	movs r3, #0
	strh r3, [r0]
	movs r4, #1
	movs r6, #1
	strh r6, [r0, #2]
	strb r4, [r0, #4]
	strb r2, [r0, #5]
	movs r1, #29
	strb r1, [r0, #6]
	strb r2, [r0, #7]
	strb r4, [r0, #8]
	strb r4, [r0, #9]
	strb r2, [r0, #10]
	strb r2, [r0, #11]
	strh r3, [r0, #12]
	strh r3, [r0, #14]
	ldr r7, .L_c5d4
	adds r1, r5, r7
	str r1, [sp, #16]
	strb r2, [r0, #20]
	movs r1, #3
	strb r1, [r0, #21]
	movs r1, #31
	strb r1, [r0, #22]
	strb r2, [r0, #23]
	strb r2, [r0, #24]
	strb r4, [r0, #25]
	strb r2, [r0, #26]
	strb r2, [r0, #27]
	strh r3, [r0, #28]
	strh r3, [r0, #30]
	ldr r1, .L_c5d8
	adds r0, r5, r1
	str r0, [sp, #32]
	add r0, sp, #36
	movs r1, #0
	movs r2, #16
	bl 0x0802E104
	add r0, sp, #52
	movs r1, #0
	movs r2, #16
	bl 0x0802E104
	add r2, sp, #68
	mov r8, r2
	mov r1, r8
	ldr r0, .L_c5dc
	ldmia r0!, {r2, r3, r7}
	stmia r1!, {r2, r3, r7}
	ldr r0, [r0]
	str r0, [r1]
	adds r0, r5, #0
	adds r0, #96
	strb r4, [r0]
	bl 0x080056FC
	ldr r3, .L_c5e0
	adds r0, r5, r3
	movs r1, #16
	bl 0x080055D8
	adds r0, r5, #0
	adds r0, #92
	strh r6, [r0]
	adds r4, r5, #0
	adds r4, #8
	ldr r2, .L_c5e4
	mov r0, sp
	adds r1, r4, #0
	bl 0x0802D974
	adds r6, r5, #0
	adds r6, #76
	ldr r2, .L_c5e8
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
	bl 0x0800C454
	bl 0x0800C4AC
	adds r0, r5, #0
	adds r0, #100
	bl 0x08003104
	ldr r0, .L_c5ec
	bl 0x08004DC8
	ldr r7, .L_c5f0
	adds r1, r5, r7
	str r0, [r1]
	ldr r0, .L_c5f4
	bl 0x08004DC8
	ldr r2, .L_c5f8
	adds r1, r5, r2
	str r0, [r1]
	ldr r0, .L_c5fc
	bl 0x08004DC8
	ldr r3, .L_c600
	adds r5, r5, r3
	str r0, [r5]
	add sp, #84
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0

.L_c5d4:
	.word 0x00000CE4
.L_c5d8:
	.word 0x00001CE4
.L_c5dc:
	.word 0x0805F6D4
.L_c5e0:
	.word 0x000024E4
.L_c5e4:
	.word 0x04000011
.L_c5e8:
	.word 0x04000004
.L_c5ec:
	.word 0x080CDFD0
.L_c5f0:
	.word 0x000025E4
.L_c5f4:
	.word 0x080CDFD8
.L_c5f8:
	.word 0x000025E8
.L_c5fc:
	.word 0x080CDFE0
.L_c600:
	.word 0x000025EC
