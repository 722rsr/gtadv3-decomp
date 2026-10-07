@ adjacent IWRAM allocator/state helpers.
@ VMA 0x08005758-0x080057D0 (120 bytes), pure Thumb with private pools.

	.thumb
	.type sub_08005758, %function
sub_08005758:
	ldr r1, .L_5764
	ldrh r2, [r1, #2]
	subs r0, r2, r0
	strh r0, [r1, #2]
	ldrh r0, [r1, #2]
	bx lr
.L_5764:
	.word 0x030002D8

	.type sub_08005768, %function
sub_08005768:
	ldr r3, .L_577c
	adds r2, r3, #4
	adds r0, r0, r2
	ldrh r2, [r3]
	strb r2, [r0]
	ldrh r0, [r3]
	adds r1, r0, r1
	strh r1, [r3]
	bx lr
	.short 0
.L_577c:
	.word 0x030002D8

	.type sub_08005780, %function
sub_08005780:
	ldr r1, .L_578c
	adds r1, #4
	adds r0, r0, r1
	ldrb r0, [r0]
	bx lr
	.short 0
.L_578c:
	.word 0x030002D8

	.type sub_08005790, %function
sub_08005790:
	ldr r1, .L_57a0
	adds r1, #68
	ldrh r2, [r1]
	subs r0, r2, r0
	strh r0, [r1]
	ldrh r0, [r1]
	bx lr
	.short 0
.L_57a0:
	.word 0x030002D8

	.type sub_080057A4, %function
sub_080057A4:
	sub sp, #4
	movs r2, #0
	str r2, [sp]
	ldr r2, .L_57c8
	mov r3, sp
	str r3, [r2]
	lsls r0, #5
	ldr r3, .L_57cc
	adds r0, r0, r3
	str r0, [r2, #4]
	lsls r1, #3
	movs r0, #133
	lsls r0, #24
	orrs r1, r0
	str r1, [r2, #8]
	ldr r0, [r2, #8]
	add sp, #4
	bx lr
.L_57c8:
	.word 0x040000D4
.L_57cc:
	.word 0x06010000
save_alloc_more_end:
