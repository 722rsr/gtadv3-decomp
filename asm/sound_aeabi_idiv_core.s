@ GT Advance 3 - ARM EABI signed divide core
@ Region: file offset 0x02DE9C-0x02DF6A (VMA 0x0802DE9C-0x0802DF6A).
@ Pure Thumb; byte-exact transcription. The preceding raw pad/stub at
@ 0x0802DE96-0x0802DE9C is kept in passthrough.inc.

.thumb
.type sub_0802DE9C, %function
sub_0802DE9C:
_0802DE9C:
	movs r3, #1
	cmp r1, #0
	beq.n _0802DF60
	bpl.n _0802DEA6
	negs r1, r1
_0802DEA6:
	push {r4}
	push {r0}
	cmp r0, #0
	bpl.n _0802DEB0
	negs r0, r0
_0802DEB0:
	cmp r0, r1
	bcc.n _0802DF54
	movs r4, #1
	lsls r4, r4, #28
_0802DEB8:
	cmp r1, r4
	bcs.n _0802DEC6
	cmp r1, r0
	bcs.n _0802DEC6
	lsls r1, r1, #4
	lsls r3, r3, #4
	b.n _0802DEB8
_0802DEC6:
	lsls r4, r4, #3
_0802DEC8:
	cmp r1, r4
	bcs.n _0802DED6
	cmp r1, r0
	bcs.n _0802DED6
	lsls r1, r1, #1
	lsls r3, r3, #1
	b.n _0802DEC8
_0802DED6:
	movs r2, #0
	cmp r0, r1
	bcc.n _0802DEDE
	subs r0, r0, r1
_0802DEDE:
	lsrs r4, r1, #1
	cmp r0, r4
	bcc.n _0802DEF0
	subs r0, r0, r4
	mov ip, r3
	movs r4, #1
	rors r3, r4
	orrs r2, r3
	mov r3, ip
_0802DEF0:
	lsrs r4, r1, #2
	cmp r0, r4
	bcc.n _0802DF02
	subs r0, r0, r4
	mov ip, r3
	movs r4, #2
	rors r3, r4
	orrs r2, r3
	mov r3, ip
_0802DF02:
	lsrs r4, r1, #3
	cmp r0, r4
	bcc.n _0802DF14
	subs r0, r0, r4
	mov ip, r3
	movs r4, #3
	rors r3, r4
	orrs r2, r3
	mov r3, ip
_0802DF14:
	mov ip, r3
	cmp r0, #0
	beq.n _0802DF22
	lsrs r3, r3, #4
	beq.n _0802DF22
	lsrs r1, r1, #4
	b.n _0802DED6
_0802DF22:
	movs r4, #14
	lsls r4, r4, #28
	ands r2, r4
	beq.n _0802DF54
	mov r3, ip
	movs r4, #3
	rors r3, r4
	tst r2, r3
	beq.n _0802DF38
	lsrs r4, r1, #3
	adds r0, r0, r4
_0802DF38:
	mov r3, ip
	movs r4, #2
	rors r3, r4
	tst r2, r3
	beq.n _0802DF46
	lsrs r4, r1, #2
	adds r0, r0, r4
_0802DF46:
	mov r3, ip
	movs r4, #1
	rors r3, r4
	tst r2, r3
	beq.n _0802DF54
	lsrs r4, r1, #1
	adds r0, r0, r4
_0802DF54:
	pop {r4}
	cmp r4, #0
	bpl.n _0802DF5C
	negs r0, r0
_0802DF5C:
	pop {r4}
	mov pc, lr
_0802DF60:
	push {lr}
	bl sub_0802DE98
	movs r0, #0
	pop {pc}
