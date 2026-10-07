@ GT Advance 3 - rec35 scene twin stage machines (legit menu/save transition)
@ Region: file offset 0x015ED0-0x016002 (VMA 0x08015ED0-0x08016002)
@
@ Disassembled via objdump from baserom.gba; byte-exact (make SHA gate).
@ Companion: asm/rec35_stage.s (save-transition chain),
@ asm/saveblock.s (guarded save wrappers).
@
@ Dispatched ONLY from event handler _080163B8 (bl @0x16432 -> machine A,
@ bl @0x1643A -> machine B), which is runtime-registered through rec35's
@ relocated template data (word 0x080163B9 @ ROM 0x080CB8A4) -- zero
@ static BL/literal callers of either machine anywhere else in the ROM.
@
@ Each machine switches on its scene phase halfword (A: s16[ctx+0x94],
@ B: s16[ctx+0x96]); phase >= 5 unreachable (cmp #4 / bhi):
@   phase 0 -> stamp 4 (pass-through/done)
@   phase 1 -> arm ctx+0x90=1, post helper events (_08015E28 / _08015E7C,
@              still raw, called numerically), _08002618(1,1), phase 2
@   phase 2 -> arm ctx+0x90=1, sound cmd _0802B3B8(3); on completion
@              ((res<<24)==0): ctx+20=1, guarded EEPROM save
@              (_08024BC0; machine B additionally _08024B70 full save
@              incl. ghost snapshot + session-reset hook), helper
@              re-run, _08002618(1,1), phase 3; else ctx+20=0
@   phase 3/4 -> shared pop epilogue
@
@ Table layout quirk (armcc): the `ldr r1,[pc,#N]` base literal IS
@ word[0] of the table block; dispatch slots are words [1..5] read at
@ base_literal + idx*4. Slot 0 therefore targets the first body directly.

.thumb

@ ----------------------------------------------------------------------------
@ _08015ED0(ctx) - stage machine A, phase cell s16[ctx+0x94]
_08015ED0:
	push {r4, r5, lr}
	adds r4, r0, #0
	adds r0, #0x94
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #4
	bhi.n _08015F60
	lsls r0, r0, #2
	ldr r1, _08015EE8 @ =0x08015EEC (table base literal)
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
.align 2, 0
_08015EE8:
	.4byte 0x08015EEC       @ base literal loaded by the ldr above
_08015EEC:
	.4byte 0x08015F00       @ slot 0 -> set-phase body
	.4byte 0x08015F0A       @ slot 1 -> helper-call body
	.4byte 0x08015F2A       @ slot 2 -> sound/save body
	.4byte 0x08015F60       @ slot 3 -> shared tail
	.4byte 0x08015F60       @ slot 4 -> shared tail
_08015F00:
	adds r1, r4, #0
	adds r1, #0x94
	movs r0, #4
	strh r0, [r1, #0]
	b.n _08015F60
_08015F0A:
	adds r1, r4, #0
	adds r1, #0x90
	movs r0, #1
	strh r0, [r1, #0]
	adds r0, r4, #0
	bl 0x08015E28           @ helper A (raw; numeric BL)
	movs r0, #1
	movs r1, #1
	bl 0x08002618           @ raw (numeric BL)
	adds r1, r4, #0
	adds r1, #0x94
	movs r0, #2
	strh r0, [r1, #0]
	b.n _08015F60
_08015F2A:
	adds r0, r4, #0
	adds r0, #0x90
	movs r5, #1
	strh r5, [r0, #0]
	movs r0, #3
	bl 0x0802B3B8            @ sound cmd 3 (converted; numeric exact VMA)
	lsls r0, r0, #24
	cmp r0, #0
	bne.n _08015F5C
	strh r5, [r4, #20]      @ save-done latch ctx+20 = 1
	bl 0x08024BC0            @ guarded save-only (saveblock.s; numeric exact VMA)
	adds r0, r4, #0
	bl 0x08015E28           @ helper A re-run (raw; numeric BL)
	movs r0, #1
	movs r1, #1
	bl 0x08002618
	adds r1, r4, #0
	adds r1, #0x94
	movs r0, #3
	strh r0, [r1, #0]
	b.n _08015F60
_08015F5C:
	movs r0, #0
	strh r0, [r4, #20]      @ completion pending: clear latch
_08015F60:
	pop {r4, r5}
	pop {r0}
	bx r0
.short 0x0000           @ armcc pad halfword

@ ----------------------------------------------------------------------------
@ _08015F68(ctx) - twin stage machine B, phase cell s16[ctx+0x96];
@ identical skeleton, helper B (_08015E7C) and an additional FULL guarded
@ save _08024B70 right before the save-only call in the phase-2 body.
_08015F68:
	push {r4, r5, lr}
	adds r4, r0, #0
	adds r0, #0x96
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #4
	bhi.n _08015FFC
	lsls r0, r0, #2
	ldr r1, _08015F80 @ =0x08015F84 (table base literal)
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
.align 2, 0
_08015F80:
	.4byte 0x08015F84       @ base literal loaded by the ldr above
_08015F84:
	.4byte 0x08015F98       @ slot 0 -> set-phase body
	.4byte 0x08015FA2       @ slot 1 -> helper-call body
	.4byte 0x08015FC2       @ slot 2 -> sound/save body
	.4byte 0x08015FFC       @ slot 3 -> shared tail
	.4byte 0x08015FFC       @ slot 4 -> shared tail
_08015F98:
	adds r1, r4, #0
	adds r1, #0x96
	movs r0, #4
	strh r0, [r1, #0]
	b.n _08015FFC
_08015FA2:
	adds r1, r4, #0
	adds r1, #0x90
	movs r0, #1
	strh r0, [r1, #0]
	adds r0, r4, #0
	bl 0x08015E7C           @ helper B (raw; numeric BL)
	movs r0, #1
	movs r1, #1
	bl 0x08002618
	adds r1, r4, #0
	adds r1, #0x96
	movs r0, #2
	strh r0, [r1, #0]
	b.n _08015FFC
_08015FC2:
	adds r0, r4, #0
	adds r0, #0x90
	movs r5, #1
	strh r5, [r0, #0]
	movs r0, #3
	bl 0x0802B3B8            @ sound cmd 3 (numeric exact VMA)
	lsls r0, r0, #24
	cmp r0, #0
	bne.n _08015FF8
	strh r5, [r4, #20]      @ save-done latch ctx+20 = 1
	bl 0x08024B70            @ guarded FULL save (saveblock.s; numeric exact VMA)
	bl 0x08024BC0            @ guarded save-only (saveblock.s; numeric exact VMA)
	adds r0, r4, #0
	bl 0x08015E7C           @ helper B re-run (raw; numeric BL)
	movs r0, #1
	movs r1, #1
	bl 0x08002618
	adds r1, r4, #0
	adds r1, #0x96
	movs r0, #3
	strh r0, [r1, #0]
	b.n _08015FFC
_08015FF8:
	movs r0, #0
	strh r0, [r4, #20]
_08015FFC:
	pop {r4, r5}
	pop {r0}
	bx r0
