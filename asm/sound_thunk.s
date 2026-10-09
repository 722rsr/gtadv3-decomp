@ GT Advance 3 - MTO sound driver: VCounter tick thunk
@ Region: file offset 0x02C53C-0x02C548 (VMA 0x0802C53C-0x0802C548).
@ Disassembled via objdump from baserom.gba; byte-exact (
@. Companion: asm/sound_api.s

	.thumb

@ ----------------------------------------------------------------------------
@ sub_0802C53C — sequencer service entry, sole caller _0802B098 in
@ sound_api.s (@0x02B0A8, first act of every VCounter tick). Pure
@ trampoline into the software mixer render sub_0802B898 (raw blob
@ 0x02B898-0x02BE74): the per-frame "note service" IS a full mix pass.
@ Trailing halfword pad keeps the next function at its ROM offset.
.type sub_0802C53C, %function
sub_0802C53C:
	push {lr}
	bl sub_0802B898
	pop {r0}
	bx r0
	.hword 0

sound_thunk_end:
