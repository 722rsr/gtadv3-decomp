@ -------------------------------------------------------------
@ : _0802BC64 voice unlink (MTO sound driver)
@ Hole 0x02BC64-0x02BC84 (VMA 0x0802BC64-0x0802BC84), 0x20 bytes.
@
@ Semantics: unlinks the caller's node from the driver's doubly-linked
@ active-voice list. r0 = channel/voice object:
@   [obj+0x2C] anchor ptr (list-head struct; its +0x20 = first node),
@   [obj+0x30] cached prev node, [obj+0x34] cached next node;
@   nodes cross-link via +0x30(prev)/+0x34(next).
@ Splices the node out using the cached pair (prev->next := next,
@ else anchor->first := next; next->prev := prev), then clears
@ [obj+0x2C]. Leaf, pure Thumb, no pool.
@
@ Boundary proof :
@  - entry 0x0802BC64: handler-table pointer word 0x0802BC65 at ROM
@    0x061568 (sequencer opcode dispatch table 0x06154C..0x06156C);
@    preceding pad halfword 0000 at 0x02BC62 after _0802BC4C bx lr.
@  - end: bx lr at 0x0802BC82; next fn _0802BC84 starts 0x02BC84
@    (table ptr 0x0802BC85 x5 at 0x061540..0x061550).
@  - callers: exactly one BL (at 0x0802BC9E inside raw _0802BC84
@    free-all loop); otherwise runtime-dispatched via the table.
@  - no literal words anywhere reference the interior; full-ROM
@    branch-target scan into [0xBC66,0xBC82) finds only the three
@    internal branches below (exterior halfword hits at 0x2B936/
@    0x2BAD6/0x2BB4A/0x2BBF2 sit inside ARM-mode streams: the adr/bx
@    divide helper entered at 0x2B924 and the ARM mix loop
@    0x2BA8C-0x2BC28 - false positives).
@ -------------------------------------------------------------
	.thumb
	.type _0802BC64, %function
_0802BC64:
	ldr	r3, [r0, #0x2C]
	cmp	r3, #0
	beq	_L0802BC82
	ldr	r1, [r0, #0x34]
	ldr	r2, [r0, #0x30]
	cmp	r2, #0
	beq	_L0802BC76
	str	r1, [r2, #0x34]
	b	_L0802BC78
_L0802BC76:
	str	r1, [r3, #0x20]
_L0802BC78:
	cmp	r1, #0
	beq	_L0802BC7E
	str	r2, [r1, #0x30]
_L0802BC7E:
	movs	r1, #0
	str	r1, [r0, #0x2C]
_L0802BC82:
	bx	lr
@ Region end (VMA 0x0802BC84). Emits no bytes; gives the promotion screen an
@ end marker for the last function in this region instead of guessing.
sound_unlink_end:
