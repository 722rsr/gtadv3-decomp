@ sound device/state setup leaves.
@ VMA 0x0802DA20-0x0802DB24 (260 bytes), pure Thumb with private pools.

	.thumb
	.type sub_0802DA20, %function
sub_0802DA20:
	adds r2, r1, #0
	lsls r0, #24
	lsrs r1, r0, #24
	cmp r1, #3
	bhi .L_da54
	ldr r0, .L_da44
	strb r1, [r0]
	ldr r1, .L_da48
	ldrb r0, [r0]
	lsls r0, #2
	ldr r3, .L_da4c
	adds r0, r0, r3
	str r0, [r1]
	ldr r0, .L_da50
	str r0, [r2]
	movs r0, #0
	b .L_da56
	.short 0
.L_da44:
	.word 0x03001768
.L_da48:
	.word 0x03001770
.L_da4c:
	.word 0x04000100
.L_da50:
	.word 0x0802D9FD
.L_da54:
	movs r0, #1
.L_da56:
	bx lr

	.type sub_0802DA58, %function
sub_0802DA58:
	push {r4, r5, r6, r7, lr}
	mov r7, r9
	mov r6, r8
	push {r6, r7}
	ldr r2, .L_dac4
	ldr r1, .L_dac8
	mov r9, r1
	ldrh r1, [r1]
	strh r1, [r2]
	movs r6, #0
	mov r2, r9
	strh r6, [r2]
	ldr r3, .L_dacc
	mov r8, r3
	ldr r5, [r3]
	strh r6, [r5, #2]
	ldr r3, .L_dad0
	ldr r4, .L_dad4
	ldrb r1, [r4]
	movs r2, #8
	adds r7, r2, #0
	lsls r7, r1
	adds r1, r7, #0
	strh r1, [r3]
	subs r3, #2
	ldrb r1, [r4]
	lsls r2, r1
	ldrh r1, [r3]
	orrs r1, r2
	strh r1, [r3]
	ldr r1, .L_dad8
	strb r6, [r1]
	ldr r2, .L_dadc
	ldrh r1, [r0]
	strh r1, [r2]
	adds r0, #2
	ldrh r1, [r0]
	strh r1, [r5]
	adds r1, r5, #2
	mov r2, r8
	str r1, [r2]
	ldrh r0, [r0, #2]
	strh r0, [r5, #2]
	str r5, [r2]
	movs r0, #1
	mov r3, r9
	strh r0, [r3]
	pop {r3, r4}
	mov r8, r3
	mov r9, r4
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
.L_dac4:
	.word 0x03001774
.L_dac8:
	.word 0x04000208
.L_dacc:
	.word 0x03001770
.L_dad0:
	.word 0x04000202
.L_dad4:
	.word 0x03001768
.L_dad8:
	.word 0x0300176C
.L_dadc:
	.word 0x0300176A

	.type sub_0802DAE0, %function
sub_0802DAE0:
	ldr r3, .L_db10
	movs r1, #0
	strh r1, [r3]
	ldr r2, .L_db14
	ldr r0, [r2]
	strh r1, [r0]
	adds r0, #2
	str r0, [r2]
	strh r1, [r0]
	subs r0, #2
	str r0, [r2]
	ldr r2, .L_db18
	ldr r0, .L_db1c
	ldrb r0, [r0]
	movs r1, #8
	lsls r1, r0
	ldrh r0, [r2]
	bics r0, r1
	strh r0, [r2]
	ldr r0, .L_db20
	ldrh r0, [r0]
	strh r0, [r3]
	bx lr
	.short 0
.L_db10:
	.word 0x04000208
.L_db14:
	.word 0x03001770
.L_db18:
	.word 0x04000200
.L_db1c:
	.word 0x03001768
.L_db20:
	.word 0x03001774
