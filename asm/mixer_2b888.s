	.text
	@ Region: file offset 0x02B888-0x02BC28 (VMA 0x0802B888-0x0802BC28).
	.arm
	.type sub_0802B888, %function
sub_0802B888:
_0802B888:
	ldrmi	sl, [r0, -r0, lsl #4]
	umull	r2, r3, r0, r1
	add	r0, r3, #0
	bx	lr
	.thumb
	.type sub_0802B898, %function
sub_0802B898:
	ldr	r0, [pc, #104]	@ (0x802b904)
	ldr	r0, [r0, #0]
	ldr	r2, [pc, #104]	@ (0x802b908)
	ldr	r3, [r0, #0]
	cmp	r2, r3
	beq.n	0x802b8a6
	bx	lr
	adds	r3, #1
	str	r3, [r0, #0]
	.type _0802B8AA, %function
_0802B8AA:
	.include "mixer_2b8aa_body.inc"
mixer_2b888_end:
