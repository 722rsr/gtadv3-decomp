@ isolated sound mixer voice-array clear.
@ VMA 0x0802BC4C-0x0802BC62 (22 bytes), pure Thumb, no pool.

	.thumb
	.type sub_0802BC4C, %function
sub_0802BC4C:
	mov r12, r4
	movs r1, #0
	movs r2, #0
	movs r3, #0
	movs r4, #0
	stmia r0!, {r1, r2, r3, r4}
	stmia r0!, {r1, r2, r3, r4}
	stmia r0!, {r1, r2, r3, r4}
	stmia r0!, {r1, r2, r3, r4}
	mov r4, r12
	bx lr
