@ AI/car-lineup row clear/update twin.
@ VMA 0x08025198-0x080251FC (100 bytes), pure Thumb, no private pool.

	.thumb
	.type sub_08025198, %function
sub_08025198:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	sub sp, #4
	mov r8, r0
	adds r2, r1, #0
	movs r6, #0
.L_51a6:
	movs r5, #0
	adds r7, r6, #1
	b .L_51ae
.L_51ac:
	adds r5, #1
.L_51ae:
	cmp r5, #4
	bgt .L_51d6
	lsls r0, r5, #1
	adds r0, r0, r5
	lsls r0, #2
	adds r4, r2, r0
	lsls r1, r6, #1
	adds r1, r1, r6
	lsls r1, #2
	add r1, r8
	adds r0, r4, #0
	str r2, [sp]
	bl sub_080250FC
	lsls r0, #24
	ldr r2, [sp]
	cmp r0, #0
	beq .L_51ac
	movs r0, #0
	str r0, [r4]
.L_51d6:
	adds r6, r7, #0
	cmp r6, #4
	ble .L_51a6
	adds r4, r2, #0
	movs r5, #4
.L_51e0:
	mov r0, r8
	adds r1, r4, #0
	bl sub_080250A0
	adds r4, #12
	subs r5, #1
	cmp r5, #0
	bge .L_51e0
	add sp, #4
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
ai_line_clear_end:
