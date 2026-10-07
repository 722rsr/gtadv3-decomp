@ GT Advance 3 - MTO sound driver: per-voice flag scan
@ Region: file offset 0x02C738-0x02C780 (VMA 0x0802C738-0x0802C780).
@ Pure Thumb, no local pool; byte-exact.

.thumb
.type sub_0802C738, %function
sub_0802C738:
_0802C738:
	push {r4, r5, r6, r7, lr}
	ldrb r5, [r0, #8]
	ldr r4, [r0, #44]
	cmp r5, #0
	ble.n _0802C77A
	movs r7, #128
_0802C744:
	ldrb r1, [r4, #0]
	adds r0, r7, #0
	ands r0, r1
	cmp r0, #0
	beq.n _0802C772
	movs r6, #64
	adds r0, r6, #0
	ands r0, r1
	cmp r0, #0
	beq.n _0802C772
	adds r0, r4, #0
	bl 0x0802C8B0
	strb r7, [r4, #0]
	movs r0, #2
	strb r0, [r4, #15]
	strb r6, [r4, #19]
	movs r0, #22
	strb r0, [r4, #25]
	adds r1, r4, #0
	adds r1, #36
	movs r0, #1
	strb r0, [r1, #0]
_0802C772:
	subs r5, #1
	adds r4, #80
	cmp r5, #0
	bgt.n _0802C744
_0802C77A:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
