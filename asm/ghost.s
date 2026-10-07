@ GT Advance 3 — race-context event-flag API + racer-array accessors
@ Region: file offset 0x0018A50-0x0018ADC (VMA 0x08018A50-0x08018ADC)
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@
@ Race context = *(u32*)0x03004E20 (pointer cell; struct lives in EWRAM).
@   +0x3C u32 event/flag word      — set/tested by this cluster
@   +0x74 u16                      — written by _08018A88
@   +0x490 u32[N] car-record ptrs  — indexed by _08018A6C
@ Racer array base 0x03004E80, stride 0x11C (284 B) per car (_08018A50).
@
@ Known flag bits in the +0x3C word (see asm/ghost.s):
@   0x00000002 set by _08018A98
@   0x00001000 "GHOST OVER" message latch (drawn by race update 0x801C3E0)

.thumb

@ ----------------------------------------------------------------------------
@ _08018A50(idx) — &racer_array[idx]  (base 0x03004E80, stride 0x11C)
_08018A50:
	adds r1, r0, #0
	lsls r1, r1, #16
	asrs r1, r1, #16
	lsls r0, r1, #3
	adds r0, r0, r1          @ x*9
	lsls r0, r0, #3          @ *72 -> x*72
	subs r0, r0, r1          @ -x  -> x*71
	lsls r0, r0, #2          @ *4  -> x*284 (0x11C)
	ldr r1, _08018A68        @ =0x03004E80
	adds r0, r0, r1
	bx lr
	movs r0, r0
	.align 2, 0
_08018A68: .4byte 0x03004E80

@ ----------------------------------------------------------------------------
@ _08018A6C(idx) — byte at car_records[idx]->+6
@ car_records = *(racectx + 0x490 + idx*4)
_08018A6C:
	ldr r1, _08018A84        @ =0x03004E20
	ldr r1, [r1, #0]         @ racectx
	lsls r0, r0, #16
	asrs r0, r0, #14         @ idx*4 (sign-safe)
	movs r2, #146
	lsls r2, r2, #3          @ 0x490
	adds r1, r1, r2
	adds r1, r1, r0
	ldr r0, [r1, #0]
	ldrb r0, [r0, #6]
	bx lr
	movs r0, r0
	.align 2, 0
_08018A84: .4byte 0x03004E20

@ ----------------------------------------------------------------------------
@ _08018A88(v) — *(u16*)(racectx + 0x74) = v
_08018A88:
	ldr r1, _08018A94        @ =0x03004E20
	ldr r1, [r1, #0]
	adds r1, #116            @ +0x74
	strh r0, [r1, #0]
	bx lr
	movs r0, r0
	.align 2, 0
_08018A94: .4byte 0x03004E20

@ ----------------------------------------------------------------------------
@ _08018A98 — raise racectx flag bit 0x2
_08018A98:
	push {lr}
	movs r0, #2
	movs r1, #1
	bl _08018AA8
	pop {r0}
	bx r0
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08018AA8(mask, set) — flags |= mask if set!=0 else flags &= ~mask
_08018AA8:
	adds r2, r0, #0
	cmp r1, #0
	beq _08018ABC
	ldr r0, _08018AB8        @ =0x03004E20
	ldr r1, [r0, #0]
	ldr r0, [r1, #60]        @ +0x3C flags
	orrs r0, r2
	b _08018AC4
	.align 2, 0
_08018AB8: .4byte 0x03004E20
_08018ABC:
	ldr r0, _08018AC8        @ =0x03004E20
	ldr r1, [r0, #0]
	ldr r0, [r1, #60]
	bics r0, r2
_08018AC4:
	str r0, [r1, #60]
	bx lr
	.align 2, 0
_08018AC8: .4byte 0x03004E20

@ ----------------------------------------------------------------------------
@ _08018ACC(mask) — return flags & mask  (nonzero = any bit pending)
_08018ACC:
	adds r1, r0, #0
	ldr r0, _08018AD8        @ =0x03004E20
	ldr r0, [r0, #0]
	ldr r0, [r0, #60]
	ands r0, r1
	bx lr
	.align 2, 0
_08018AD8: .4byte 0x03004E20

@ End-of-region anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x08018A50-0x08018ADC) and has no `.include`; the body at
@ 0x08018ACC is 8 bytes of code plus its 4-byte pool word and ends on
@ 0x08018ADC, exactly that boundary. Verified against baserom.gba: the span
@ reads 1c01 4802 6800 6bc0 4008 4770 | 4e20 0300. Unambiguous, so the anchor
@ is safe. Without it the body is refused as 'no end marker'.
ghost_end:
