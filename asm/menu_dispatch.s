@ GT Advance 3 - menu record dispatch helper
@ Region: file offset 0x00C168-0x00C1E4 (VMA 0x0800C168-0x0800C1E4).
@ Pure Thumb with an inline 14-entry tail-call table; byte-exact.

.thumb
.type sub_0800C168, %function
sub_0800C168:
_0800C168:
	push {lr}
	adds r2, r1, #0
	subs r0, #1
	cmp r0, #13
	bhi.n _0800C1E0
	lsls r0, r0, #2
	ldr r1, _0800C17C
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
_0800C17C:
	.word 0x0800C180
	.word 0x0800C1B8
	.word 0x0800C1E0
	.word 0x0800C1C0
	.word 0x0800C1C8
	.word 0x0800C1E0
	.word 0x0800C1E0
	.word 0x0800C1E0
	.word 0x0800C1D0
	.word 0x0800C1E0
	.word 0x0800C1E0
	.word 0x0800C1E0
	.word 0x0800C1E0
	.word 0x0800C1E0
	.word 0x0800C1D8
_0800C1B8:
	adds r0, r3, #0
	bl 0x0800C010
	b.n _0800C1E0
_0800C1C0:
	adds r0, r3, #0
	bl 0x0800C0F0
	b.n _0800C1E0
_0800C1C8:
	adds r0, r3, #0
	bl 0x0800C110
	b.n _0800C1E0
_0800C1D0:
	adds r0, r3, #0
	bl 0x0800C128
	b.n _0800C1E0
_0800C1D8:
	adds r0, r3, #0
	adds r1, r2, #0
	bl 0x0800C138
_0800C1E0:
	pop {r0}
	bx r0

@ Region end 0x0800C1E4. The next function's VMA labels live in menu_ctor2.s,
@ so this file cannot spell them; the anchor gives tools/match_c_slice.py a
@ unique end marker when this body is C-owned. Emits no bytes.
menu_dispatch_end:
