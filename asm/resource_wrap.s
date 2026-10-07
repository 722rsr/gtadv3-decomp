@ candidate: live resource-band wrapper.
@ VMA 0x080263C4-0x080263DE (26 bytes), pure Thumb, no private pool.

	.thumb
	.type sub_080263C4, %function
sub_080263C4:
	push {r4, r5, lr}
	adds r5, r1, #0
	adds r1, r2, #0
	bl 0x08026230
	adds r4, r0, #0
	adds r1, r5, #0
	bl 0x080262A4
	adds r0, r4, #0
	pop {r4, r5}
	pop {r1}
	bx r1

resource_wrap_end:
