@ AI/car-lineup row update helper.
@ VMA 0x08025130-0x08025198 (104 bytes), pure Thumb with one private pool.

	.thumb
	.type sub_08025130, %function
sub_08025130:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	sub sp, #4
	mov r8, r0
	adds r2, r1, #0
	movs r6, #0
.L_513e:
	movs r5, #0
	adds r7, r6, #1
	b .L_5146
.L_5144:
	adds r5, #1
.L_5146:
	cmp r5, #4
	bgt .L_516e
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
	beq .L_5144
	ldr r0, .L_5194
	str r0, [r4]
.L_516e:
	adds r6, r7, #0
	cmp r6, #4
	ble .L_513e
	adds r4, r2, #0
	movs r5, #4
.L_5178:
	mov r0, r8
	adds r1, r4, #0
	bl sub_08025028
	adds r4, #12
	subs r5, #1
	cmp r5, #0
	bge .L_5178
	add sp, #4
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
.L_5194:
	.word 0x7FFFFFFF
ai_line_update_end:
