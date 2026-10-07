@ GT Advance 3 - ARM EABI signed divide helper
@ Region: file offset 0x02DE04-0x02DE96 (VMA 0x0802DE04-0x0802DE96).
@ Pure Thumb; byte-exact transcription of __aeabi_idiv.

.thumb
.type sub_0802DE04, %function
sub_0802DE04:
_0802DE04:
	cmp r1, #0
	beq.n _0802DE8C
	push {r4}
	adds r4, r0, #0
	eors r4, r1
	mov ip, r4
	movs r3, #1
	movs r2, #0
	cmp r1, #0
	bpl.n _0802DE1A
	negs r1, r1
_0802DE1A:
	cmp r0, #0
	bpl.n _0802DE20
	negs r0, r0
_0802DE20:
	cmp r0, r1
	bcc.n _0802DE7E
	movs r4, #1
	lsls r4, r4, #28
_0802DE28:
	cmp r1, r4
	bcs.n _0802DE36
	cmp r1, r0
	bcs.n _0802DE36
	lsls r1, r1, #4
	lsls r3, r3, #4
	b.n _0802DE28
_0802DE36:
	lsls r4, r4, #3
_0802DE38:
	cmp r1, r4
	bcs.n _0802DE46
	cmp r1, r0
	bcs.n _0802DE46
	lsls r1, r1, #1
	lsls r3, r3, #1
	b.n _0802DE38
_0802DE46:
	cmp r0, r1
	bcc.n _0802DE4E
	subs r0, r0, r1
	orrs r2, r3
_0802DE4E:
	lsrs r4, r1, #1
	cmp r0, r4
	bcc.n _0802DE5A
	subs r0, r0, r4
	lsrs r4, r3, #1
	orrs r2, r4
_0802DE5A:
	lsrs r4, r1, #2
	cmp r0, r4
	bcc.n _0802DE66
	subs r0, r0, r4
	lsrs r4, r3, #2
	orrs r2, r4
_0802DE66:
	lsrs r4, r1, #3
	cmp r0, r4
	bcc.n _0802DE72
	subs r0, r0, r4
	lsrs r4, r3, #3
	orrs r2, r4
_0802DE72:
	cmp r0, #0
	beq.n _0802DE7E
	lsrs r3, r3, #4
	beq.n _0802DE7E
	lsrs r1, r1, #4
	b.n _0802DE46
_0802DE7E:
	adds r0, r2, #0
	mov r4, ip
	cmp r4, #0
	bpl.n _0802DE88
	negs r0, r0
_0802DE88:
	pop {r4}
	mov pc, lr
_0802DE8C:
	push {lr}
	bl sub_0802DE98
	movs r0, #0
	pop {pc}
