@ GT Advance 3 - rec35 scene helper pair
@ Region: file offset 0x015E28-0x015ED0 (VMA 0x08015E28-0x08015ED0),
@ two 0x54-byte bodies incl. trailing armcc pad halfwords:
@   helper A 0x08015E28-0x08015E7C, helper B 0x08015E7C-0x08015ED0
@   (B abuts asm/rec35_stage.s exactly - no overlap).
@
@ Disassembled via objdump from baserom.gba; byte-exact (make SHA gate).
@ Companion: asm/rec35_stage.s (save-transition chain).
@
@ Callers (xref.py census, whole ROM): EXACTLY the four numeric BLs in
@ the converted twin stage machines -
@   _08015E28 <- bl @0x08015F14, bl @0x08015F46   (machine A)
@   _08015E7C <- bl @0x08015FAC, bl @0x08015FE2   (machine B)
@ Zero literal-pool refs (incl. bit0 interworking form); every branch
@ target landing inside the span originates inside it (own internal
@ branches) - no exterior interior references.
@
@ Semantics: helper(ctx) reads the machine's phase cell
@ (A: s16[ctx+0x94], B: s16[ctx+0x96]) and posts one event id into the
@ queue anchored at ctx+0xA0 via raw event poster _08025BF0(list, id):
@   phase == 1     -> id {A:9,  B:17}
@   2 <= p <= 3    -> id {A:10, B:18}   (cmp#1/blt exit, cmp#3/bgt exit)
@   otherwise      -> no-op
@ then binds the posted node: _08007ABC([ctx+12], [ctx+0xA4], [ctx+0xA0])
@ (exact converted alias sub_08007ABC; called numerically here to match
@ the authoritative rec35_stage.s style - identical encoding).
@ These ids match asm/carphys_tick.s stage-1/3 helper events feeding the sound-
@ completion latch that gates the guarded EEPROM saves.

.thumb

@ ----------------------------------------------------------------------------
@ _08015E28(ctx) - helper A: post event for machine-A phase cell s16[ctx+0x94]
_08015E28:
	push {r4, r5, lr}
	adds r5, r0, #0
	adds r0, #0x94
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #1
	bne.n _08015E52
	adds r4, r5, #0
	adds r4, #0xA0
	adds r0, r4, #0
	movs r1, #9
	bl sub_08025BF0            @ raw event poster (no alias; numeric BL)
	ldr r0, [r5, #12]
	adds r1, r5, #0
	adds r1, #0xA4
	ldr r1, [r1, #0]
	ldr r2, [r4, #0]
	bl 0x08007ABC            @ exact alias sub_08007ABC (numeric BL)
	b.n _08015E74
_08015E52:
	cmp r0, #1
	blt.n _08015E74
	cmp r0, #3
	bgt.n _08015E74
	adds r4, r5, #0
	adds r4, #0xA0
	adds r0, r4, #0
	movs r1, #10
	bl sub_08025BF0            @ raw event poster (numeric BL)
	ldr r0, [r5, #12]
	adds r1, r5, #0
	adds r1, #0xA4
	ldr r1, [r1, #0]
	ldr r2, [r4, #0]
	bl 0x08007ABC            @ exact alias sub_08007ABC (numeric BL)
_08015E74:
	pop {r4, r5}
	pop {r0}
	bx r0
	.short 0x0000           @ armcc pad halfword

@ ----------------------------------------------------------------------------
@ _08015E7C(ctx) - helper B: twin of A over machine-B phase cell s16[ctx+0x96],
@ event ids {17, 18}
_08015E7C:
	push {r4, r5, lr}
	adds r5, r0, #0
	adds r0, #0x96
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #1
	bne.n _08015EA6
	adds r4, r5, #0
	adds r4, #0xA0
	adds r0, r4, #0
	movs r1, #17
	bl sub_08025BF0            @ raw event poster (numeric BL)
	ldr r0, [r5, #12]
	adds r1, r5, #0
	adds r1, #0xA4
	ldr r1, [r1, #0]
	ldr r2, [r4, #0]
	bl 0x08007ABC            @ exact alias sub_08007ABC (numeric BL)
	b.n _08015EC8
_08015EA6:
	cmp r0, #1
	blt.n _08015EC8
	cmp r0, #3
	bgt.n _08015EC8
	adds r4, r5, #0
	adds r4, #0xA0
	adds r0, r4, #0
	movs r1, #18
	bl sub_08025BF0            @ raw event poster (numeric BL)
	ldr r0, [r5, #12]
	adds r1, r5, #0
	adds r1, #0xA4
	ldr r1, [r1, #0]
	ldr r2, [r4, #0]
	bl 0x08007ABC            @ exact alias sub_08007ABC (numeric BL)
_08015EC8:
	pop {r4, r5}
	pop {r0}
	bx r0
	.short 0x0000           @ armcc pad halfword
rec35_helper_end:
