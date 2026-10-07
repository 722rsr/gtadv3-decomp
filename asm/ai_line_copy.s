@ AI/car-lineup record copy and selector helpers.
@ VMA 0x08025028-0x080250A0 (120 bytes), pure Thumb, no private pool.

	.thumb
	.type sub_08025028, %function
sub_08025028:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	mov r9, r0
	mov r8, r1
	ldr r0, [r1]
	mov r1, r9
	bl sub_0802500C
	mov ip, r0
	cmp r0, #5
	beq .L_5076
	movs r4, #3
	lsls r0, #1
	mov sl, r0
	cmp r4, ip
	blt .L_5068
	mov r3, r9
	adds r3, #48
	mov r2, r9
	adds r2, #36
.L_5056:
	adds r1, r3, #0
	adds r0, r2, #0
	ldmia r0!, {r5, r6, r7}
	stmia r1!, {r5, r6, r7}
	subs r3, #12
	subs r2, #12
	subs r4, #1
	cmp r4, ip
	bge .L_5056
.L_5068:
	mov r1, sl
	add r1, ip
	lsls r1, #2
	add r1, r9
	mov r0, r8
	ldmia r0!, {r2, r3, r4}
	stmia r1!, {r2, r3, r4}
.L_5076:
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0

	.type sub_08025084, %function
sub_08025084:
	adds r3, r0, #0
	movs r2, #0
.L_5088:
	ldr r0, [r1]
	cmp r3, r0
	bls .L_5092
	adds r0, r2, #0
	b .L_509c
.L_5092:
	adds r1, #12
	adds r2, #1
	cmp r2, #4
	ble .L_5088
	movs r0, #5
.L_509c:
	bx lr
	.short 0

@ End anchor for the 0x08025084 slice entry. sub_08025084 is the last body in
@ this region, so there is no following `.type` line to bound its span and
@ tools/promotion_screen.py:has_end_anchor finds nothing to fall back on.
@ A label emits no bytes; this sits at the region's last byte, 0x080250A0.
ai_line_copy_end:
