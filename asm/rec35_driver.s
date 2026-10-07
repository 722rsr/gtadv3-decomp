@ GT Advance 3 - rec35 runtime-registered scene event driver
@ Region: file offset 0x0163B8-0x0164D4 (VMA 0x080163B8-0x080164D4)
@
@ Disassembled via objdump from baserom.gba; byte-exact (make SHA gate).
@ Companion: asm/rec35_stage.s (save-transition chain),
@ asm/rec35_stage.s (the two stage machines this dispatches).
@
@ Registration: ZERO static BL/literal callers anywhere in the ROM. Sole
@ reference is VMA-form word 0x080163B9 at ROM 0x080CB8A4, inside rec35's
@ relocated ctor template block (ctor 0x08015BA1 + arg 0xB0 @0x080CB884/88;
@ src/foundation_subsys.c) -- registered through the runtime-
@ relocated constructor mechanism, same class as the guarded save wrappers.
@ The registration word lives OUTSIDE this span and stays raw/unmoved.
@
@ _080163B8(ev r0, arg_b r1, arg_c r2, ctx r3): 12-way event switch,
@ idx = ev-1, ev>12 -> shared epilogue. Table base literal IS word[0] of
@ the table block (armcc duplicate-target quirk, same as the stage
@ machines); dispatch slots are words [1..12] read at base_literal+idx*4.
@
@   ev 1 -> _08015CB4(ctx)                          (raw)
@   ev 2 -> _08015C9C(ctx, arg_b)                   (raw)
@   ev 3/4        no-op (slot -> epilogue)
@   ev 5 -> refresh ctx+0x10 via _0800D854 (raw marker),
@           ctx+0x78 via _0800D8E4 (raw marker); then switch
@           s16[ctx+0x8E]: 0 -> stage machine A _08015ED0 (rec35_stage.s),
@                          1 -> stage machine B _08015F68 (rec35_stage.s)
@   ev 6 -> save-done poll: u16[ctx+20]==0 -> epilogue; else split on
@           armed latch s16[ctx+0x90] (zero/nonzero) x twin selector
@           s16[ctx+0x8E] {0=A, 1=B}, forwarding (u16)arg_b/(u16)arg_c to
@           A0 _080160C4 / B0 _08016004 / A1 _08016164 / B1 _080161A0 (raw)
@   ev 7 -> _080162E0(ctx)                          (raw)
@   ev 8..11      no-op (slot -> epilogue)
@   ev12 -> _08015CB0(ctx)                          (raw)

.thumb

@ ----------------------------------------------------------------------------
@ Entry label: runtime-registered, so no `bl` in the closure; without
@ `.type %function` it was invisible and inflated _080163B4's span. Zero bytes.
	.type _080163B8, %function
_080163B8:
	push	{r4, r5, lr}
	adds	r5, r1, #0          @ arg_b
	adds	r4, r3, #0          @ ctx
	subs	r0, #1              @ ev-1
	cmp	r0, #11                 @ ev in 1..12?
	bls.n	_L080163C6
	b.n	_080164CC               @ default: epilogue
_L080163C6:
	lsls	r0, r0, #2
	ldr	r1, _080163D0       @ =0x080163D4 (table base literal = word[0])
	adds	r0, r0, r1
	ldr	r0, [r0, #0]
	mov	pc, r0
.align 2, 0
_080163D0:
	.4byte	0x080163D4          @ base literal loaded by the ldr above
	.4byte	0x080164BE          @ slot 0  (ev 1)  -> _08015CB4 call
	.4byte	0x08016404          @ slot 1  (ev 2)  -> _08015C9C call
	.4byte	0x080164CC          @ slot 2  (ev 3)  -> no-op
	.4byte	0x080164CC          @ slot 3  (ev 4)  -> no-op
	.4byte	0x0801640E          @ slot 4  (ev 5)  -> refresh + stage machines
	.4byte	0x08016448          @ slot 5  (ev 6)  -> save-done poll
	.4byte	0x08016440          @ slot 6  (ev 7)  -> _080162E0 call
	.4byte	0x080164CC          @ slot 7  (ev 8)  -> no-op
	.4byte	0x080164CC          @ slot 8  (ev 9)  -> no-op
	.4byte	0x080164CC          @ slot 9  (ev 10) -> no-op
	.4byte	0x080164CC          @ slot 10 (ev 11) -> no-op
	.4byte	0x080164C6          @ slot 11 (ev 12) -> _08015CB0 call
_L08016404:                     @ ev 2: _08015C9C(ctx, arg_b)
	adds	r0, r4, #0
	adds	r1, r5, #0
	bl	0x08015C9C              @ raw (numeric exact VMA)
	b.n	_080164CC
_L0801640E:                     @ ev 5: refresh + twin stage dispatch
	adds	r0, r4, #0
	adds	r0, #16             @ ctx+0x10
	bl	0x0800D854              @ raw passthrough marker sub_0800D854
	adds	r0, r4, #0
	adds	r0, #120            @ ctx+0x78
	bl	0x0800D8E4              @ raw passthrough marker sub_0800D8E4
	adds	r0, r4, #0
	adds	r0, #142            @ s16[ctx+0x8E] twin selector
	movs	r1, #0
	ldrsh	r0, [r0, r1]
	cmp	r0, #0
	beq.n	_L08016430
	cmp	r0, #1
	beq.n	_L08016438
	b.n	_080164CC
_L08016430:                     @ twin A
	adds	r0, r4, #0
	bl	0x08015ED0              @ stage machine A (rec35_stage.s)
	b.n	_080164CC
_L08016438:                     @ twin B
	adds	r0, r4, #0
	bl	0x08015F68              @ stage machine B (rec35_stage.s)
	b.n	_080164CC
_L08016440:                     @ ev 7: _080162E0(ctx)
	adds	r0, r4, #0
	bl	0x080162E0              @ raw (numeric exact VMA)
	b.n	_080164CC
_L08016448:                     @ ev 6: save-done latch poll + result routing
	ldrh	r0, [r4, #20]          @ u16[ctx+20] save-done latch
	cmp	r0, #0
	beq.n	_080164CC
	adds	r0, r4, #0
	adds	r0, #144            @ s16[ctx+0x90] armed latch
	movs	r1, #0
	ldrsh	r0, [r0, r1]
	cmp	r0, #0
	bne.n	_L0801648C
	adds	r0, r4, #0          @ armed==0 path: s16[ctx+0x8E] again
	adds	r0, #142
	movs	r1, #0
	ldrsh	r0, [r0, r1]
	cmp	r0, #0
	beq.n	_L0801646C
	cmp	r0, #1
	beq.n	_L0801647C
	b.n	_080164CC
_L0801646C:                     @ A0
	lsls	r1, r5, #16
	lsrs	r1, r1, #16         @ (u16)arg_b
	lsls	r2, r2, #16
	lsrs	r2, r2, #16         @ (u16)arg_c
	adds	r0, r4, #0
	bl	0x080160C4              @ raw (numeric exact VMA)
	b.n	_080164CC
_L0801647C:                     @ B0
	lsls	r1, r5, #16
	lsrs	r1, r1, #16
	lsls	r2, r2, #16
	lsrs	r2, r2, #16
	adds	r0, r4, #0
	bl	0x08016004              @ raw (numeric exact VMA)
	b.n	_080164CC
_L0801648C:                     @ armed!=0 path
	adds	r0, r4, #0
	adds	r0, #142
	movs	r1, #0
	ldrsh	r0, [r0, r1]
	cmp	r0, #0
	beq.n	_L0801649E
	cmp	r0, #1
	beq.n	_L080164AE
	b.n	_080164CC
_L0801649E:                     @ A1
	lsls	r1, r5, #16
	lsrs	r1, r1, #16
	lsls	r2, r2, #16
	lsrs	r2, r2, #16
	adds	r0, r4, #0
	bl	0x08016164              @ raw (numeric exact VMA)
	b.n	_080164CC
_L080164AE:                     @ B1
	lsls	r1, r5, #16
	lsrs	r1, r1, #16
	lsls	r2, r2, #16
	lsrs	r2, r2, #16
	adds	r0, r4, #0
	bl	0x080161A0              @ raw (numeric exact VMA)
	b.n	_080164CC
_L080164BE:                     @ ev 1: _08015CB4(ctx)
	adds	r0, r4, #0
	bl	0x08015CB4              @ raw (numeric exact VMA)
	b.n	_080164CC
_L080164C6:                     @ ev 12: _08015CB0(ctx)
	adds	r0, r4, #0
	bl	0x08015CB0              @ raw (numeric exact VMA)
_080164CC:                      @ shared epilogue (armcc interworking return)
	pop	{r4, r5}
	pop	{r0}
	bx	r0
.short	0x0000                  @ armcc pad halfword (@0x164D2)
