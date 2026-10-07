@ GT Advance 3 - keypad poller + edge/key/repeat getters
@ Region: file offset 0x002430-0x0024AC (VMA 0x08002430-0x080024AC)
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@
@ Keypad state lives at IWRAM 0x030035C0:
@   +0x00 u16 current keys   (~KEYINPUT)
@   +0x02 u16 edge keys      (cur & ~prev)
@   +0x04 u16 repeat keys    (fires while held, see below)
@   +0x06 u16 hold countdown (reloaded to 24 on any change)
@   +0x08 u16 last held value
@   +0x0A u16 frame counter  (++ per poll)
@
@ Repeat behaviour: on key change, +0x08/+0x04 = cur and countdown=24.
@ While held unchanged: countdown decrements; once it hits 0, +0x04 is
@ refreshed every other frame (frame counter bit 0).

.thumb

@ ----------------------------------------------------------------------------
@ _08002430 — poll KEYINPUT, update cur/edge/repeat state.
@ Called once per frame from AgbMain's idle path.
_08002430:
	push {r4, lr}
	ldr r0, _0800246C @ =0x04000130
	ldrh r0, [r0]
	mvns r0, r0
	lsls r0, r0, #16
	lsrs r2, r0, #16
	ldr r3, _08002470 @ =0x030035C0
	adds r0, r2, #0
	ldrh r1, [r3, #0]
	bics r0, r1
	movs r1, #0
	strh r0, [r3, #2]
	strh r2, [r3, #0]
	strh r1, [r3, #4]
	ldrh r1, [r3, #10]
	adds r1, #1
	strh r1, [r3, #10]
	ldrh r4, [r3, #8]
	cmp r2, r4
	bne.n _08002478
	ldrh r0, [r3, #6]
	cmp r0, #0
	bne.n _08002474
	movs r0, #1
	ands r1, r0
	cmp r1, #0
	beq.n _08002480
	strh r4, [r3, #4]
	b.n _08002480
	movs r0, r0
	.align 2, 0
_0800246C: .4byte 0x04000130
_08002470: .4byte 0x030035C0
_08002474:
	subs r0, #1
	b.n _0800247E
_08002478:
	strh r2, [r3, #8]
	strh r2, [r3, #4]
	movs r0, #24
_0800247E:
	strh r0, [r3, #6]
_08002480:
	pop {r4}
	pop {r0}
	bx r0
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08002488 -> u16 — edge keys (+0x02). AgbMain gates the soft-reset
@ combo on this (asm/agbmain.s 4.4).
_08002488:
	ldr r0, _08002490 @ =0x030035C0
	ldrh r0, [r0, #2]
	bx lr
	movs r0, r0
	.align 2, 0
_08002490: .4byte 0x030035C0

@ ----------------------------------------------------------------------------
@ _08002494 -> u16 — currently-held keys (+0x00).
_08002494:
	ldr r0, _0800249C @ =0x030035C0
	ldrh r0, [r0, #0]
	bx lr
	movs r0, r0
	.align 2, 0
_0800249C: .4byte 0x030035C0

@ ----------------------------------------------------------------------------
@ _080024A0 -> u16 — repeat keys (+0x04). Caller: 0x800C89A.
_080024A0:
	ldr r0, _080024A8 @ =0x030035C0
	ldrh r0, [r0, #4]
	bx lr
	movs r0, r0
	.align 2, 0
_080024A8: .4byte 0x030035C0

@ End-of-region anchor for the splicer. This file has exactly one
@ `@ Region:` (0x08002430-0x080024AC) and no `.include`, and the promoted body
@ at 0x080024A0 ends exactly at 0x080024AC, so the boundary is unambiguous and
@ an end marker is safe here. Without it `promotion_screen` refuses the body
@ with "no end marker in keypad.s", because the splicer needs a literal end
@ label to bound the replacement and will not infer one from a region comment.
keypad_end:
