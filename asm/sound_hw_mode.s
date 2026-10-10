@ GT Advance 3 - MTO sound driver: hardware mode selector
@ Region: file offset 0x02CF7C-0x02CFCC (VMA 0x0802CF7C-0x0802CFCC).
@ Pure Thumb with interleaved hardware-register pools; byte-exact.

.thumb
.type sub_0802CF7C, %function
sub_0802CF7C:
_0802CF7C:
	lsls r0, r0, #24
	lsrs r0, r0, #24
	adds r1, r0, #0
	cmp r0, #2
	beq.n _0802CFA4
	cmp r0, #2
	bgt.n _0802CF90
	cmp r0, #1
	beq.n _0802CF96
	b.n _0802CFB8
_0802CF90:
	cmp r1, #3
	beq.n _0802CFAC
	b.n _0802CFB8
_0802CF96:
	ldr r1, _0802CFA0
	movs r0, #8
	strb r0, [r1, #0]
	adds r1, #2
	b.n _0802CFC0
_0802CFA0: .word 0x04000063
_0802CFA4:
	ldr r1, _0802CFA8
	b.n _0802CFBA
_0802CFA8: .word 0x04000069
_0802CFAC:
	ldr r1, _0802CFB4
	movs r0, #0
	b.n _0802CFC2
	.short 0
_0802CFB4: .word 0x04000070
_0802CFB8:
	ldr r1, _0802CFC8
_0802CFBA:
	movs r0, #8
	strb r0, [r1, #0]
	adds r1, #4
_0802CFC0:
	movs r0, #128
_0802CFC2:
	strb r0, [r1, #0]
	bx lr
	.short 0
_0802CFC8: .word 0x04000079
sound_hw_mode_end:
