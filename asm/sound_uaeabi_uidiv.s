@ GT Advance 3 - ARM EABI unsigned divide helper
@ Region: file offset 0x02DF6C-0x02DFE4 (VMA 0x0802DF6C-0x0802DFE4).
@ Pure Thumb; byte-exact transcription of __aeabi_uidiv.

.thumb
.type sub_0802DF6C, %function
sub_0802DF6C:
_0802DF6C:
	cmp r1, #0
	beq.n _0802DFDA
	movs r3, #1
	movs r2, #0
	push {r4}
	cmp r0, r1
	bcc.n _0802DFD4
	movs r4, #1
	lsls r4, r4, #28
_0802DF7E:
	cmp r1, r4
	bcs.n _0802DF8C
	cmp r1, r0
	bcs.n _0802DF8C
	lsls r1, r1, #4
	lsls r3, r3, #4
	b.n _0802DF7E
_0802DF8C:
	lsls r4, r4, #3
_0802DF8E:
	cmp r1, r4
	bcs.n _0802DF9C
	cmp r1, r0
	bcs.n _0802DF9C
	lsls r1, r1, #1
	lsls r3, r3, #1
	b.n _0802DF8E
_0802DF9C:
	cmp r0, r1
	bcc.n _0802DFA4
	subs r0, r0, r1
	orrs r2, r3
_0802DFA4:
	lsrs r4, r1, #1
	cmp r0, r4
	bcc.n _0802DFB0
	subs r0, r0, r4
	lsrs r4, r3, #1
	orrs r2, r4
_0802DFB0:
	lsrs r4, r1, #2
	cmp r0, r4
	bcc.n _0802DFBC
	subs r0, r0, r4
	lsrs r4, r3, #2
	orrs r2, r4
_0802DFBC:
	lsrs r4, r1, #3
	cmp r0, r4
	bcc.n _0802DFC8
	subs r0, r0, r4
	lsrs r4, r3, #3
	orrs r2, r4
_0802DFC8:
	cmp r0, #0
	beq.n _0802DFD4
	lsrs r3, r3, #4
	beq.n _0802DFD4
	lsrs r1, r1, #4
	b.n _0802DF9C
_0802DFD4:
	adds r0, r2, #0
	pop {r4}
	mov pc, lr
_0802DFDA:
	push {lr}
	bl sub_0802DE98
	movs r0, #0
	pop {pc}
