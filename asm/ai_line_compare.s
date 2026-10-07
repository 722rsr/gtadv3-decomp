@ AI/car-lineup copy and record comparison helpers.
@ VMA 0x080250A0-0x08025130 (144 bytes), pure Thumb with one private pool.

	.thumb
	.type sub_080250A0, %function
sub_080250A0:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	mov r9, r0
	mov r8, r1
	ldr r0, [r1]
	mov r1, r9
	bl sub_08025084
	mov ip, r0
	cmp r0, #5
	beq .L_50ee
	movs r4, #3
	lsls r0, #1
	mov sl, r0
	cmp r4, ip
	blt .L_50e0
	mov r3, r9
	adds r3, #48
	mov r2, r9
	adds r2, #36
.L_50ce:
	adds r1, r3, #0
	adds r0, r2, #0
	ldmia r0!, {r5, r6, r7}
	stmia r1!, {r5, r6, r7}
	subs r3, #12
	subs r2, #12
	subs r4, #1
	cmp r4, ip
	bge .L_50ce
.L_50e0:
	mov r1, sl
	add r1, ip
	lsls r1, #2
	add r1, r9
	mov r0, r8
	ldmia r0!, {r2, r3, r4}
	stmia r1!, {r2, r3, r4}
.L_50ee:
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0

	.type sub_080250FC, %function
sub_080250FC:
	push {r4, lr}
	adds r3, r0, #0
	adds r4, r1, #0
	ldr r1, [r3]
	ldr r0, [r4]
	cmp r1, r0
	bne .L_5128
	ldr r0, [r3, #8]
	ldr r2, .L_5124
	ands r0, r2
	ldr r1, [r4, #8]
	ands r1, r2
	cmp r0, r1
	bne .L_5128
	ldrh r3, [r3, #4]
	ldrh r4, [r4, #4]
	cmp r3, r4
	bne .L_5128
	movs r0, #1
	b .L_512a
.L_5124:
	.word 0x00FFFFFF
.L_5128:
	movs r0, #0
.L_512a:
	pop {r4}
	pop {r1}
	bx r1

@ End of region 0x080250A0-0x08025130. sub_080250FC's span ends exactly here,
@ so this anchor is the precise end marker, not a bounded-region guess.
ai_line_compare_end:
