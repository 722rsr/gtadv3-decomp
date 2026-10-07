@ GT Advance 3 - MTO sound driver: reset/init cluster
@ Region: file offset 0x02C4C4-0x02C53C (VMA 0x0802C4C4-0x0802C53C).
@ Disassembled via objdump from baserom.gba; byte-exact. Pure Thumb.
@ One function: bulk state reinitializer followed by a private literal
@ pool at 0x02C518-0x02C53B (9 words kept at their ROM offsets).
@ Companion: asm/sound_api.s
@
@ Callers (both raw): bl @0x0802B050, bl @0x0802B1C8 (sound API init path).
@ Callees (all raw passthrough): sub_0802D974 (ROM->mem copy helper),
@ sub_0802C8C4, sub_0802C780, sub_0802CA34 (per-block reset calls),
@ sub_0802CBBC (template record copier).

@ ----------------------------------------------------------------------------
@ sub_0802C4C4 - driver-wide reset/init.
@ 1) copies the routine image at 0x0802B91C (Thumb tag stripped from the
@    0x0802B91D literal by AND with ~2... i.e. bit0 mask -2) via
@    sub_0802D974(dst=0x03007000, src=r0, aux=0x0400E000);
@ 2) runs three single-arg reset calls over EWRAM blocks
@    0x0203E230 -> sub_0802C8C4, 0x0203EC40 -> sub_0802C780,
@    constant 0x0093F800 -> sub_0802CA34;
@ 3) walks 4 records of 12 B at ROM table 0x08061F74
@    ({u32 dst, u32 src, u8 size, pad, u16 hw}): per record calls
@    sub_0802CBBC(dst, src, size), stores lo(halfword@[rec+10]) to
@    [dst+11] and stamps [dst+24] = 0x020300EE.
	.type sub_0802C4C4, %function
sub_0802C4C4:
	push	{r4, r5, r6, lr}
	ldr	r0, [pc, #80]			@ 0x02C518
	movs	r1, #2
	negs	r1, r1				@ r1 = -2
	ands	r0, r1				@ strip Thumb tag -> 0x0802B91C
	ldr	r1, [pc, #76]			@ 0x02C51C
	ldr	r2, [pc, #76]			@ 0x02C520
	bl	sub_0802D974
	ldr	r0, [pc, #76]			@ 0x02C524
	bl	sub_0802C8C4
	ldr	r0, [pc, #72]			@ 0x02C528
	bl	sub_0802C780
	ldr	r0, [pc, #72]			@ 0x02C52C
	bl	sub_0802CA34
	ldr	r0, [pc, #68]			@ 0x02C530
	lsls	r0, r0, #16
	lsrs	r0, r0, #16			@ count = 0x400 records
	cmp	r0, #0
	beq	.Lc4c4_ret
	ldr	r5, [pc, #64]			@ 0x02C534: table base
	adds	r6, r0, #0
.Lc4c4_loop:
	ldr	r4, [r5, #0]			@ dst
	ldr	r1, [r5, #4]			@ src
	ldrb	r2, [r5, #8]			@ size
	adds	r0, r4, #0
	bl	sub_0802CBBC
	ldrh	r0, [r5, #10]
	strb	r0, [r4, #11]
	ldr	r0, [pc, #48]			@ 0x02C538
	str	r0, [r4, #24]
	adds	r5, #12
	subs	r6, #1
	cmp	r6, #0
	bne	.Lc4c4_loop
.Lc4c4_ret:
	pop	{r4, r5, r6}
	pop	{r0}
	bx	r0
	.word	0x0802B91D			@ 0x02C518: thumb-tagged ptr into mixed-ISA span
	.word	0x03007000			@ 0x02C51C
	.word	0x040000E0			@ 0x02C520
	.word	0x0203E230			@ 0x02C524
	.word	0x0203EC40			@ 0x02C528
	.word	0x0093F800			@ 0x02C52C
	.word	0x00000004			@ 0x02C530: record count (4)
	.word	0x08061F74			@ 0x02C534: template table base (4 x 12 B)
	.word	0x0203EE00			@ 0x02C538: stamp stored to [dst+24]
