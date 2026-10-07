@ menu mode/teardown leaves.
@ VMA 0x0800C604-0x0800C668 (100 bytes), pure Thumb, no pool.

	.thumb
	.type sub_0800C604, %function
sub_0800C604:
	push {r4, lr}
	adds r4, r0, #0
	adds r0, #96
	ldrb r0, [r0]
	cmp r0, #1
	bne .L_c618
	movs r0, #0
	bl 0x08002060
	b .L_c61e
.L_c618:
	movs r0, #1
	bl 0x08002060
.L_c61e:
	adds r0, r4, #0
	adds r0, #8
	movs r1, #1
	bl 0x080050E8
	movs r0, #1
	bl 0x080056B8
	adds r0, r4, #0
	adds r0, #92
	ldrh r0, [r0]
	bl 0x08004E1C
	pop {r4}
	pop {r0}
	bx r0
	.short 0

	.type sub_0800C640, %function
sub_0800C640:
	push {lr}
	bl 0x080055F4
	bl 0x08002B50
	bl 0x08002BB4
	bl 0x08002B44
	pop {r0}
	bx r0
	.short 0

	.type sub_0800C658, %function
sub_0800C658:
	push {lr}
	bl 0x08005604
	bl 0x08002C98
	pop {r0}
	bx r0
	.short 0

menu_c604_end:
