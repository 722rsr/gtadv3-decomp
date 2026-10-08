@ GT Advance 3 - sound voice volume-scale leaf
@ Region: file offset 0x02D480-0x02D4A8 (VMA 0x0802D480-0x0802D4A8).
@ Pure Thumb with a private Smsh sentinel pool; byte-exact.

.thumb
.type sub_0802D480, %function
sub_0802D480:
_0802D480:
	push {r4, lr}
	adds r2, r0, #0
	lsls r1, r1, #16
	lsrs r1, r1, #16
	ldr r3, [r2, #52]
	ldr r0, _0802D4A4
	cmp r3, r0
	bne.n _0802D49C
	strh r1, [r2, #30]
	ldrh r4, [r2, #28]
	adds r0, r1, #0
	muls r0, r4
	asrs r0, r0, #8
	strh r0, [r2, #32]
_0802D49C:
	pop {r4}
	pop {r0}
	bx r0
	.short 0
_0802D4A4: .word 0x68736D53

sound_d480_end:
