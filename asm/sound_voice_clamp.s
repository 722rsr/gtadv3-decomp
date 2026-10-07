@ GT Advance 3 - MTO sound driver: voice parameter clamp
@ Region: file offset 0x02CFCC-0x02D034 (VMA 0x0802CFCC-0x0802D034).
@ Pure Thumb without a literal pool; byte-exact.

.thumb
.type sub_0802CFCC, %function
sub_0802CFCC:
_0802CFCC:
	push {r4, lr}
	adds r1, r0, #0
	ldrb r0, [r1, #2]
	lsls r2, r0, #24
	lsrs r4, r2, #24
	ldrb r3, [r1, #3]
	lsls r0, r3, #24
	lsrs r3, r0, #24
	cmp r4, r3
	bcc.n _0802CFEC
	lsrs r0, r2, #25
	cmp r0, r3
	bcc.n _0802CFF8
	movs r0, #15
	strb r0, [r1, #27]
	b.n _0802D006
_0802CFEC:
	lsrs r0, r0, #25
	cmp r0, r4
	bcc.n _0802CFF8
	movs r0, #240
	strb r0, [r1, #27]
	b.n _0802D006
_0802CFF8:
	movs r0, #255
	strb r0, [r1, #27]
	ldrb r2, [r1, #3]
	ldrb r3, [r1, #2]
	adds r0, r2, r3
	lsrs r0, r0, #4
	b.n _0802D016
_0802D006:
	ldrb r2, [r1, #3]
	ldrb r3, [r1, #2]
	adds r0, r2, r3
	lsrs r0, r0, #4
	strb r0, [r1, #10]
	cmp r0, #15
	bls.n _0802D018
	movs r0, #15
_0802D016:
	strb r0, [r1, #10]
_0802D018:
	ldrb r2, [r1, #6]
	ldrb r3, [r1, #10]
	adds r0, r2, #0
	muls r0, r3
	adds r0, #15
	asrs r0, r0, #4
	strb r0, [r1, #25]
	ldrb r0, [r1, #28]
	ldrb r2, [r1, #27]
	ands r0, r2
	strb r0, [r1, #27]
	pop {r4}
	pop {r0}
	bx r0
@ End-of-region anchor for the splicer: this file declares exactly one
@ `@ Region:` (0x02CFCC-0x02D034) and no `.include`, and the body at
@ 0x0802cfcc is 104 bytes, so it ends on 0x0802d034 -- exactly that boundary.
@ Without it promotion_screen refuses the body with "no end marker in
@ sound_voice_clamp.s", even though the body is byte-exact at 104/104.
sound_voice_clamp_end:
