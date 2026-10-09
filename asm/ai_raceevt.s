@ GT Advance 3 - race-scene event handler + scene event queue (AI/race control)
@ Region: file offset 0x023ED0-0x024048 (VMA 0x08023ED0-0x08024048).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/ai_collect.s.
@
@ This cluster belongs to subsystem record 42 of the registry
@ (src/foundation_subsys.c): ctor/handler _08023ED0, instance
@ size 0xF4 allocated from the scene heap (base 0x030035D0). The engine
@ broadcast _08004D4C invokes it every frame with AgbMain's event ids;
@ the 12-entry switch (table at 0x08023EF0) maps:
@   ev1 -> _08023FCE (raw; teardown)
@   ev2 -> case below: init helper chain _08022CB4(ctx,a)
@   ev5 -> free ctx+0x10/_0800D8E4(ctx+0x78) + _080235F4(ctx)
@   ev6 -> _08023E7C pause/timeout watchdog, then per-phase gameplay
@          update by u16[ctx+0x8E]: {0:_0802381C,1:_08023958,2:_08023A34}
@   ev7 -> per-frame flow update by same phase halfword:
@          {0:_08023BD4 countdown,1:_08023D4C running,2:_08023E0C done}
@   ev12 -> _08022D20(ctx)
@
@ The queue helpers manage the 10-slot scene event ring at IWRAM
@ 0x030005B0 ({u32 ev, arg, next=-1} records; head cell [0x030005B0]):
@   _08023FE4 reset (called each frame by race FSM _0800AA40),
@   _08023FF8 push (used all over the race code as "fire scene event").
@ Pop side lives raw at _08024048 (marker sub_08024048); the shared
@ racer tick _0800AA20 pops one entry per frame.

.thumb

@ ----------------------------------------------------------------------------
@ _08023ED0(ev, a, b, ctx) — record-42 event handler / ctor.
@ Switch on ev-1 via table _08023EF0; default exits.
@ Entry label: reached through the switch table, not by `bl`; without
@ `.type %function` it was invisible and inflated _08023E7C's span. Zero bytes.
	.type _08023ED0, %function
_08023ED0:
	push {r4, r5, r6, lr}
	adds r5, r1, #0
	adds r6, r2, #0
	adds r4, r3, #0
	subs r0, #1
	cmp r0, #11
	bls _08023EE0
	b _08023FDC
_08023EE0:
	lsls r0, r0, #2
	ldr r1, _08023EE8_lit @ =_08023EF0
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	.align 2, 0
_08023EE8_lit: .word _08023EF0

@ jump table: entry[i] = handler for event i+1
_08023EF0:
	.word _08023FCE @ ev1
	.word _08023F20 @ ev2
	.word _08023FDC @ ev3 (exit)
	.word _08023FDC @ ev4 (exit)
	.word _08023F2A @ ev5
	.word _08023F76 @ ev6
	.word _08023F42 @ ev7
	.word _08023FDC @ ev8
	.word _08023FDC @ ev9
	.word _08023FDC @ ev10
	.word _08023FDC @ ev11
	.word _08023FD6 @ ev12

@ ev2: run init helper over (ctx, a)
_08023F20:
	adds r0, r4, #0
	adds r1, r5, #0
	bl sub_08022CB4
	b _08023FDC

@ ev5: rechild objects, then scene hook
_08023F2A:
	adds r0, r4, #0
	adds r0, #16
	bl sub_0800D854
	adds r0, r4, #0
	adds r0, #120
	bl sub_0800D8E4
	adds r0, r4, #0
	bl sub_080235F4
	b _08023FDC

@ ev7: per-frame flow update, switch on phase halfword [ctx+0x8E]
_08023F42:
	adds r0, r4, #0
	adds r0, #142
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #1
	beq _08023F66
	cmp r0, #1
	bgt _08023F58
	cmp r0, #0
	beq _08023F5E
	b _08023FDC
_08023F58:
	cmp r0, #2
	beq _08023F6E
	b _08023FDC
_08023F5E:
	adds r0, r4, #0
	bl sub_08023BD4 @ phase 0: pre-race countdown flow
	b _08023FDC
_08023F66:
	adds r0, r4, #0
	bl sub_08023D4C @ phase 1: race running flow (HUD/params)
	b _08023FDC
_08023F6E:
	adds r0, r4, #0
	bl sub_08023E0C @ phase 2: finished flow
	b _08023FDC

@ ev6: pause/timeout watchdog then per-phase gameplay update
@ (input/AI/state commands carry a=u16 arg1, b=u16 arg2)
_08023F76:
	adds r0, r4, #0
	bl sub_08023E7C
	ldrh r0, [r4, #20]
	cmp r0, #0
	beq _08023FDC
	adds r0, r4, #0
	adds r0, #142
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #1
	beq _08023FAE
	cmp r0, #1
	bgt _08023F98
	cmp r0, #0
	beq _08023F9E
	b _08023FDC
_08023F98:
	cmp r0, #2
	beq _08023FBE
	b _08023FDC
_08023F9E:
	lsls r1, r5, #16
	lsrs r1, r1, #16
	lsls r2, r6, #16
	lsrs r2, r2, #16
	adds r0, r4, #0
	bl sub_0802381C @ phase 0 gameplay step
	b _08023FDC
_08023FAE:
	lsls r1, r5, #16
	lsrs r1, r1, #16
	lsls r2, r6, #16
	lsrs r2, r2, #16
	adds r0, r4, #0
	bl sub_08023958 @ phase 1 gameplay step
	b _08023FDC
_08023FBE:
	lsls r1, r5, #16
	lsrs r1, r1, #16
	lsls r2, r6, #16
	lsrs r2, r2, #16
	adds r0, r4, #0
	bl sub_08023A34 @ phase 2 gameplay step
	b _08023FDC

@ ev1: teardown via scene hook
_08023FCE:
	adds r0, r4, #0
	bl sub_08023628
	b _08023FDC

@ ev12: final cleanup hook
_08023FD6:
	adds r0, r4, #0
	bl sub_08022D20
_08023FDC:
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0

@ ----------------------------------------------------------------------------
@ _08023FE4 — reset scene event ring at 0x030005B0: count=0,
@ first record's next = -1 (empty), second cell cleared.
_08023FE4:
	ldr r0, _08023FF4_lit @ =0x030005B0
	movs r2, #0
	str r2, [r0, #0]
	movs r1, #1
	negs r1, r1
	str r1, [r0, #4]
	str r2, [r0, #8]
	bx lr
	.align 2, 0
_08023FF4_lit: .word 0x030005B0

@ ----------------------------------------------------------------------------
@ _08023FF8(ev, arg) — append (ev, arg) to the scene event ring.
@ Walks up to 10 records from base+4 while next != -1, writes the new
@ tail {ev, arg}, terminates with next=-1 and clears the slot after.
_08023FF8:
	push {r4, r5, r6, lr}
	adds r5, r0, #0
	adds r6, r1, #0
	movs r4, #0
	ldr r0, _08024044_lit @ =0x030005B0
	ldr r1, [r0, #4]
	movs r2, #1
	negs r2, r2
	adds r3, r0, #0
	cmp r1, r2
	beq _0802401E
	adds r1, r3, #4
_08024010:
	adds r1, #8
	adds r4, #1
	cmp r4, #9
	bgt _0802401E
	ldr r0, [r1, #0]
	cmp r0, r2
	bne _08024010
_0802401E:
	lsls r1, r4, #3
	adds r2, r3, #4
	adds r0, r1, r2
	str r5, [r0, #0]
	adds r3, #8
	adds r1, r1, r3
	str r6, [r1, #0]
	adds r0, r4, #1
	lsls r0, r0, #3
	adds r2, r0, r2
	movs r1, #1
	negs r1, r1
	str r1, [r2, #0]
	adds r0, r0, r3
	movs r1, #0
	str r1, [r0, #0]
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0
_08024044_lit: .word 0x030005B0

@ Byte-neutral end anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x023ED0-0x024048) and has no `.include`; the body at
@ 0x08023FF8 ends on 0x024048, exactly that boundary, so the anchor is safe.
@ Without it the body is refused as 'no end marker in ai_raceevt.s'.
ai_raceevt_end:
