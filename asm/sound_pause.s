@ GT Advance 3 - MTO sound driver: pause-gate setters
@ Region: file offset 0x02C488-0x02C4C4 (VMA 0x0802C488-0x0802C4C4).
@ Disassembled via objdump from baserom.gba; byte-exact. Pure-Thumb
@ leaves over the voice/state record ([+0x34] == "Smsh" magic guard);
@ trailing private literal pools kept at their ROM offsets. Companion:
@ asm/sound_api.s

@ ----------------------------------------------------------------------------
@ _0802C488(state) - pause gate OFF: clear [+4] bit31 when the record
@ carries the "Smsh" magic at [+0x34]. Callers (all raw): bl @0x02C60C,
@ 0x02C66A, 0x02C6AA, 0x02C6C6.
	.type _0802C488, %function
_0802C488:
	adds	r2, r0, #0
	ldr	r3, [r2, #52]
	ldr	r0, [pc, #12]
	cmp	r3, r0
	bne	.Lc488_ret
	ldr	r0, [r2, #4]
	ldr	r1, [pc, #8]
	ands	r0, r1
	str	r0, [r2, #4]
.Lc488_ret:
	bx	lr
	.word	0x68736D53			@ 0x02C49C: "Smsh"
	.word	0x7FFFFFFF			@ 0x02C4A0: bit31 clear mask

@ ----------------------------------------------------------------------------
@ _0802C4A4(state, u16 v) - pause gate ON: when [+0x34] == "Smsh",
@ store u16(v) into fade cells [+0x24]/[+0x26] and 0x100 into [+0x28].
@ Sole caller (raw): bl @0x02C6E6.
	.type _0802C4A4, %function
_0802C4A4:
	adds	r2, r0, #0
	lsls	r1, r1, #16
	lsrs	r1, r1, #16
	ldr	r3, [r2, #52]
	ldr	r0, [pc, #16]
	cmp	r3, r0
	bne	.Lc4a4_ret
	strh	r1, [r2, #38]
	strh	r1, [r2, #36]
	movs	r0, #128
	lsls	r0, r0, #1
	strh	r0, [r2, #40]
.Lc4a4_ret:
	bx	lr
	.hword	0				@ 0x02C4BE: pad keeps pool at ROM offset
	.word	0x68736D53			@ 0x02C4C0: "Smsh"

@ ----------------------------------------------------------------------------
@ `_0802C4A4` is the LAST typed entry in this file, so it has no following
@ `.type` to use as a span end marker, and `tools/promotion_screen.py` refuses
@ it with "no end marker in sound_pause.s and no sound_pause_end: to fall back
@ on". This anchor emits NO bytes, so it cannot change the byte stream; it only
@ gives the last body a truthful bound. The file is self-terminated with no
@ `.include`, so EOF here is the region's real end: the final pool word above
@ occupies 0x02C4C0-0x02C4C3 and the region header puts the end at 0x02C4C4.
@ `make independent-slice` and `make ownership-map` must run in the same change
@ that touches a file, to refresh the pinned sha256.
sound_pause_end:
