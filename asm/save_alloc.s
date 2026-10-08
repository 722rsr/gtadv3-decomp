@ IWRAM bump allocator behind the SPRMAN OVER diagnostic.
@ VMA 0x0800572C-0x08005758 (44 bytes), pure Thumb plus private pool.

	.thumb
	.type sub_0800572C, %function
sub_0800572C:
	push {r4, lr}
	ldr r2, .L_5750
	ldrh r4, [r2]
	adds r0, r4, r0
	strh r0, [r2]
	lsls r0, #16
	movs r1, #0x80
	lsls r1, #19
	cmp r0, r1
	bls .L_5748
	ldr r0, .L_5754
	ldrh r1, [r2]
	bl 0x0800295C
.L_5748:
	adds r0, r4, #0
	pop {r4}
	pop {r1}
	bx r1
.L_5750:
	.word 0x030002D8
.L_5754:
	.word 0x0805BAA4

save_alloc_end:
