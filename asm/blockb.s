@ GT Advance 3 - state block B / per-frame control cluster
@ Region: file offset 0x0020E8-0x002254 (VMA 0x080020E8-0x08002254)
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/agbmain.s (main loop).
@
@ Block B lives at IWRAM 0x030000E8; pointer slot is IWRAM 0x030003F4.
@ Field map (+ = block B offset):
@   +0x00 u8   active flag (set by _08002124, cleared on reset/_080021EC)
@   +0x01 u8   "done/ack" flag (cleared by _080021EC when helper
@              _08001F3C reports nonzero)
@   +0x04 u16  match counter (written by _080021A0; read as s16 limit)
@   +0x06 u16  param halfword (set with +0 by _08002124; broadcast to
@              block A +0x04 by _080021EC via _080016D0)
@   +0x08 s16  expected count compared against sub_08002044
@
@ The getters/setters here are called all over the engine's relocated
@ ctor region (0x800C806+, see src/foundation_subsys.c).

.thumb

@ ----------------------------------------------------------------------------
@ _080020E8(v) — write v to block B +0x08 (expected count).
@ AgbMain calls this with v=2 right after registering the block.
_080020E8:
	ldr r1, _080020F0 @ =0x030000F4
	ldr r1, [r1]
	strh r0, [r1, #8]
	bx lr
	.align 2, 0
_080020F0: .4byte 0x030000F4

@ ----------------------------------------------------------------------------
@ _080020F4 — register block B: slot 0x030003F4 <- 0x030000E8, reset.
@ Called once per frame from AgbMain's loop.
_080020F4:
	push {lr}
	ldr r1, _08002104 @ =0x030000F4
	ldr r0, _08002108 @ =0x030000E8
	str r0, [r1]
	bl _0800210C
	pop {r0}
	bx r0
	.align 2, 0
_08002104: .4byte 0x030000F4
_08002108: .4byte 0x030000E8

@ ----------------------------------------------------------------------------
@ _0800210C — reset block B (+0 byte, +4 halfword, +1 byte -> 0).
@ Callers: AgbMain loop and _080020F4.
_0800210C:
	ldr r1, _08002120 @ =0x030000F4
	ldr r0, [r1]
	movs r2, #0
	strb r2, [r0, #0]
	ldr r0, [r1]
	movs r1, #0
	strh r2, [r0, #4]
	strb r1, [r0, #1]
	bx lr
	movs r0, r0
	.align 2, 0
_08002120: .4byte 0x030000F4

@ ----------------------------------------------------------------------------
@ _08002124(param) — arm block B: active=1, +6=param, request mode 1.
@ Heavily used engine API (callers 0x800C806, 0x800D29C, 0x800D4F8,...).
_08002124:
	push {lr}
	ldr r3, _0800213C @ =0x030000F4
	ldr r2, [r3]
	movs r1, #1
	strb r1, [r2, #0]
	ldr r1, [r3]
	strh r0, [r1, #6]
	bl _08001818
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0800213C: .4byte 0x030000F4

@ ----------------------------------------------------------------------------
@ _08002140 -> value — if active: s16 at +0x04, else 0.
@ Heavily used engine API.
_08002140:
	movs r2, #0
	ldr r0, _08002154 @ =0x030000F4
	ldr r1, [r0]
	ldrb r0, [r1, #0]
	cmp r0, #0
	beq.n _08002150
	movs r0, #4
	ldrsh r2, [r1, r0]
_08002150:
	adds r0, r2, #0
	bx lr
	.align 2, 0
_08002154: .4byte 0x030000F4

@ ----------------------------------------------------------------------------
@ _08002158(id, payload) — if not acked (+1 == 0): forward to event
@ dispatcher _08001E48(id, payload).
@ Heavily used engine API.
_08002158:
	push {lr}
	adds r2, r0, #0
	lsls r1, r1, #16
	lsrs r1, r1, #16
	ldr r0, _08002174 @ =0x030000F4
	ldr r0, [r0]
	ldrb r0, [r0, #1]
	cmp r0, #0
	bne.n _08002170
	adds r0, r2, #0
	bl _08001E48
_08002170:
	pop {r0}
	bx r0
	.align 2, 0
_08002174: .4byte 0x030000F4

@ ----------------------------------------------------------------------------
@ _08002178(x) -> u16 — if x < s16(+0x04): _08001E5C(x), else 0.
@ Heavily used engine API.
_08002178:
	push {r4, lr}
	adds r2, r0, #0
	movs r3, #0
	ldr r0, _0800219C @ =0x030000F4
	ldr r0, [r0]
	movs r4, #4
	ldrsh r0, [r0, r4]
	cmp r2, r0
	bge.n _08002194
	adds r0, r2, #0
	bl _08001E5C
	lsls r0, r0, #16
	lsrs r3, r0, #16
_08002194:
	adds r0, r3, #0
	pop {r4}
	pop {r1}
	bx r1
	.align 2, 0
_0800219C: .4byte 0x030000F4

@ ----------------------------------------------------------------------------
@ _080021A0 — count entries matching block B +0x06 among the first
@ s16(+0x08) items reported by sub_08002044/sub_0800206C; store to +0x04.
@ Caller: _080021EC (and _08002204 fallthrough path).
_080021A0:
	push {r4, r5, lr}
	movs r5, #0
	bl _08002044
	ldr r1, _080021B8 @ =0x030000F4
	ldr r1, [r1]
	movs r2, #8
	ldrsh r1, [r1, r2]
	cmp r0, r1
	bne.n _080021DC
	movs r4, #0
	b.n _080021D4
	.align 2, 0
_080021B8: .4byte 0x030000F4
_080021BC:
	adds r0, r4, #0
	bl _0800206C
	ldr r1, _080021E8 @ =0x030000F4
	ldr r1, [r1]
	lsls r0, r0, #16
	lsrs r0, r0, #16
	ldrh r1, [r1, #6]
	cmp r0, r1
	bne.n _080021D2
	adds r5, #1
_080021D2:
	adds r4, #1
_080021D4:
	bl _08002044
	cmp r4, r0
	blt.n _080021BC
_080021DC:
	ldr r0, _080021E8 @ =0x030000F4
	ldr r0, [r0]
	strh r5, [r0, #4]
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_080021E8: .4byte 0x030000F4

@ ----------------------------------------------------------------------------
@ _080021EC — block B frame processor (AgbMain idle-path call).
@ if active: copy +6 into block A +0x04 (via _080016D0) and run the
@ matcher; then if acked and _08001F3C != 0, clear the ack flag.
_080021EC:
	push {r4, lr}
	ldr r4, _08002228 @ =0x030000F4
	ldr r1, [r4]
	ldrb r0, [r1, #0]
	cmp r0, #0
	beq.n _08002208
	ldrb r0, [r1, #1]
	cmp r0, #0
	bne.n _08002210
	ldrh r0, [r1, #6]
	bl _080016D0
	bl _080021A0
_08002208:
	ldr r0, [r4]
	ldrb r0, [r0, #1]
	cmp r0, #0
	beq.n _08002220
_08002210:
	bl _08001F3C
	cmp r0, #0
	beq.n _08002220
	ldr r0, _08002228 @ =0x030000F4
	ldr r1, [r0]
	movs r0, #0
	strb r0, [r1, #1]
_08002220:
	pop {r4}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08002228: .4byte 0x030000F4

@ ----------------------------------------------------------------------------
@ _0800222C(x) -> u16 — sibling of _08002178 using _08001E8C instead
@ of _08001E5C. No direct BL callers found in ROM (pointer/tail use).
@ Entry label. The inventory admits a bare label only on `.type %function` or
@ an inbound `bl`; this body has neither, so it was invisible and inflated
@ _080021EC's span. Zero bytes emitted.
	.type _0800222C, %function
_0800222C:
	push {r4, lr}
	adds r2, r0, #0
	movs r3, #0
	ldr r0, _08002250 @ =0x030000F4
	ldr r0, [r0]
	movs r4, #4
	ldrsh r0, [r0, r4]
	cmp r2, r0
	bge.n _08002248
	adds r0, r2, #0
	bl _08001E8C
	lsls r0, r0, #16
	lsrs r3, r0, #16
_08002248:
	adds r0, r3, #0
	pop {r4}
	pop {r1}
	bx r1
	.align 2, 0
_08002250: .4byte 0x030000F4

blockb_end:
