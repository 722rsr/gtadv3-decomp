@ save-slot descriptor appender.
@ VMA 0x0800580C-0x08005860 (84 bytes), pure Thumb with one private pool.

	.thumb
	.type _0800580C, %function
_0800580C:
	push {r4, lr}
	adds r3, r0, #0
	ldr r2, .L_585c
	ldr r4, [r2]
	lsls r0, r4, #3
	adds r1, r2, #0
	adds r1, #12
	adds r0, r0, r1
	ldr r1, [r2, #8]
	str r1, [r0]
	str r3, [r0, #4]
	adds r1, r3, #4
	ldr r0, [r2, #4]
	cmp r0, #2
	bgt .L_583e
	cmp r0, #1
	blt .L_583e
	adds r1, #7
	adds r0, r1, #0
	cmp r0, #0
	bge .L_583a
	adds r0, r3, #0
	adds r0, #18
	.L_583a:
	asrs r1, r0, #3
	lsls r1, r1, #3
	.L_583e:
	cmp r1, #0
	bge .L_5844
	adds r1, #7
	.L_5844:
	asrs r1, r1, #3
	ldr r0, [r2, #8]
	adds r0, r0, r1
	str r0, [r2, #8]
	ldr r0, [r2]
	adds r0, #1
	str r0, [r2]
	lsls r0, r4, #24
	lsrs r0, r0, #24
	pop {r4}
	pop {r1}
	bx r1
.L_585c:
	.word 0x03000320
save_desc_end:
