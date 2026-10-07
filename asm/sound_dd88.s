@ sound/save continuation: sector retry/verify wrapper.
@ VMA 0x0802DD88-0x0802DDC6 (62 bytes), pure Thumb, no private pool.

	.thumb
	.type sub_0802DD88, %function
sub_0802DD88:
	push {r4, r5, r6, lr}
	adds r5, r1, #0
	lsls r0, #16
	lsrs r4, r0, #16
	movs r6, #0
	b .L_dd9a
.L_dd94:
	adds r0, r6, #1
	lsls r0, #24
	lsrs r6, r0, #24
.L_dd9a:
	cmp r6, #2
	bhi .L_ddbe
	adds r0, r4, #0
	adds r1, r5, #0
	bl sub_0802DC54
	lsls r0, #16
	lsrs r2, r0, #16
	cmp r2, #0
	bne .L_dd94
	adds r0, r4, #0
	adds r1, r5, #0
	bl sub_0802DD30
	lsls r0, #16
	lsrs r2, r0, #16
	cmp r2, #0
	bne .L_dd94
.L_ddbe:
	adds r0, r2, #0
	pop {r4, r5, r6}
	pop {r1}
	bx r1
