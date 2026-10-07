@ menu/record dispatcher tranche.
@ VMA 0x0800C340-0x0800C454 (276 bytes), pure Thumb with private pools/table.

	.thumb
	.type sub_0800C340, %function
sub_0800C340:
	push {r4, r5, lr}
	adds r5, r0, #0
	adds r4, r1, #0
	movs r0, #0
	bl 0x08002060
	cmp r4, #1
	blt .L_c372
	cmp r4, #2
	bgt .L_c364
	ldr r1, .L_c360
	adds r0, r5, r1
	ldr r0, [r0]
	bl 0x08004EF0
	b .L_c372

.L_c360:
	.word 0x00001DE4

.L_c364:
	cmp r4, #4
	bgt .L_c372
	ldr r1, .L_c378
	adds r0, r5, r1
	ldr r0, [r0]
	bl 0x08004EF0

.L_c372:
	pop {r4, r5}
	pop {r0}
	bx r0

.L_c378:
	.word 0x00001DE8

	.type sub_0800C37C, %function
sub_0800C37C:
	push {lr}
	adds r0, #0x61
	ldrb r0, [r0]
	cmp r0, #0
	bne .L_c38c
	movs r0, #1
	bl 0x08004ED8
.L_c38c:
	pop {r0}
	bx r0

	.type sub_0800C390, %function
sub_0800C390:
	push {lr}
	movs r0, #1
	bl 0x08002060
	pop {r0}
	bx r0

	.type sub_0800C39C, %function
sub_0800C39C:
	push {lr}
	adds r2, r1, #0
	subs r0, #1
	cmp r0, #20
	bhi .L_c44e
	lsls r0, #2
	ldr r1, .L_c3b0
	adds r0, r0, r1
	ldr r0, [r0]
	mov pc, r0

.L_c3b0:
	.word .L_c3b4
.L_c3b4:
	.word .L_c408
	.word .L_c44e
	.word .L_c410
	.word .L_c418
	.word .L_c44e
	.word .L_c44e
	.word .L_c44e
	.word .L_c420
	.word .L_c44e
	.word .L_c44e
	.word .L_c44e
	.word .L_c44e
	.word .L_c428
	.word .L_c432
	.word .L_c44e
	.word .L_c44e
	.word .L_c44e
	.word .L_c44e
	.word .L_c448
	.word .L_c44e
	.word .L_c43c

.L_c408:
	adds r0, r3, #0
	bl 0x0800C1E4
	b .L_c44e
.L_c410:
	adds r0, r3, #0
	bl 0x0800C2C4
	b .L_c44e
.L_c418:
	adds r0, r3, #0
	bl 0x0800C2E4
	b .L_c44e
.L_c420:
	adds r0, r3, #0
	bl 0x0800C2FC
	b .L_c44e
.L_c428:
	adds r0, r3, #0
	adds r1, r2, #0
	bl 0x0800C30C
	b .L_c44e
.L_c432:
	adds r0, r3, #0
	adds r1, r2, #0
	bl 0x0800C340
	b .L_c44e
.L_c43c:
	lsls r1, r2, #16
	lsrs r1, #16
	adds r0, r3, #0
	bl 0x0800C37C
	b .L_c44e
.L_c448:
	adds r0, r3, #0
	bl 0x0800C390

.L_c44e:
	pop {r0}
	bx r0
	.short 0
