@ AI/car-lineup record helpers.
@ VMA 0x08024FE8-0x08025028 (64 bytes), pure Thumb with private pools.

	.thumb
	.type sub_08024FE8, %function
sub_08024FE8:
	push {r4, lr}
	ldr r4, .L_5008
	lsls r2, #2
	movs r3, #44
	muls r1, r3
	adds r2, r2, r1
	movs r1, #176
	muls r0, r1
	adds r2, r2, r0
	adds r2, r2, r4
	movs r1, #2
	ldrsh r0, [r2, r1]
	pop {r4}
	pop {r1}
	bx r1
	.short 0
.L_5008:
	.word 0x080CCD8C

	.type sub_0802500C, %function
sub_0802500C:
	adds r3, r0, #0
	movs r2, #0
.L_5010:
	ldr r0, [r1]
	cmp r3, r0
	bcs .L_501a
	adds r0, r2, #0
	b .L_5024
.L_501a:
	adds r1, #12
	adds r2, #1
	cmp r2, #4
	ble .L_5010
	movs r0, #5
.L_5024:
	bx lr
	.short 0

@ End anchor for the 0x0802500C slice entry. sub_0802500C is the last body in
@ this region, so there is no following `.type` line to bound its span and
@ tools/promotion_screen.py:has_end_anchor finds nothing to fall back on.
@ A label emits no bytes; this sits at the region's last byte, 0x08025028.
ai_line_helpers_end:
