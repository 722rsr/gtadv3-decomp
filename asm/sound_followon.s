@ GT Advance 3 - sound stream start/compare/stop helpers
@ Region: file offset 0x02C548-0x02C614 (VMA 0x0802C548-0x0802C614).
@ Three pure-Thumb functions with private ROM pools; byte-exact.

.thumb

@ _0802C548(ch) - copy the selected stream's current record to the mixer.
.type sub_0802C548, %function
sub_0802C548:
_0802C548:
	push {lr}
	lsls r0, r0, #16
	ldr r2, _0802C56C
	ldr r1, _0802C570
	lsrs r0, r0, #13
	adds r0, r0, r1
	ldrh r3, [r0, #4]
	lsls r1, r3, #1
	adds r1, r1, r3
	lsls r1, r1, #2
	adds r1, r1, r2
	ldr r2, [r1, #0]
	ldr r1, [r0, #0]
	adds r0, r2, #0
	bl 0x0802CC34
	pop {r0}
	bx r0
	.align 2, 0
_0802C56C: .word 0x08061F74
_0802C570: .word 0x08061FA4

@ The `.type` lines above are byte-neutral metadata, but WITHOUT them
@ coverage.py cannot see this file's entries: a label counts as a function
@ entry only when a `.type NAME, %function` line names it or something `bl`s it
@ by name. Nothing branches to sub_0802C574 by name -- it is reached through a
@ pointer table -- so sub_0802C548's span ran straight past it and swallowed all
@ 76 bytes of the next body, and a byte-perfect 44-byte candidate read
@ PARTIAL 44/120.
@ _0802C574(ch) - replace a stream record when its source changes.
.type sub_0802C574, %function
sub_0802C574:
_0802C574:
	push {lr}
	lsls r0, r0, #16
	ldr r2, _0802C5A0
	ldr r1, _0802C5A4
	lsrs r0, r0, #13
	adds r0, r0, r1
	ldrh r3, [r0, #4]
	lsls r1, r3, #1
	adds r1, r1, r3
	lsls r1, r1, #2
	adds r1, r1, r2
	ldr r1, [r1, #0]
	ldr r3, [r1, #0]
	ldr r2, [r0, #0]
	cmp r3, r2
	beq.n _0802C5A8
	adds r0, r1, #0
	adds r1, r2, #0
	bl 0x0802CC34
	b.n _0802C5BC
	.short 0x0000
_0802C5A0: .word 0x08061F74
_0802C5A4: .word 0x08061FA4
_0802C5A8:
	ldr r2, [r1, #4]
	ldrh r0, [r1, #4]
	cmp r0, #0
	beq.n _0802C5B4
	cmp r2, #0
	bge.n _0802C5BC
_0802C5B4:
	adds r0, r1, #0
	adds r1, r3, #0
	bl 0x0802CC34
_0802C5BC:
	pop {r0}
	bx r0

@ _0802C5C0(ch) - stop a stream when its source changes or is invalid.
.type sub_0802C5C0, %function
sub_0802C5C0:
_0802C5C0:
	push {lr}
	lsls r0, r0, #16
	ldr r2, _0802C5EC
	ldr r1, _0802C5F0
	lsrs r0, r0, #13
	adds r0, r0, r1
	ldrh r3, [r0, #4]
	lsls r1, r3, #1
	adds r1, r1, r3
	lsls r1, r1, #2
	adds r1, r1, r2
	ldr r1, [r1, #0]
	ldr r3, [r1, #0]
	ldr r2, [r0, #0]
	cmp r3, r2
	beq.n _0802C5F4
	adds r0, r1, #0
	adds r1, r2, #0
	bl 0x0802CC34
	b.n _0802C610
	.short 0x0000
_0802C5EC: .word 0x08061F74
_0802C5F0: .word 0x08061FA4
_0802C5F4:
	ldr r2, [r1, #4]
	ldrh r0, [r1, #4]
	cmp r0, #0
	bne.n _0802C606
	adds r0, r1, #0
	adds r1, r3, #0
	bl 0x0802CC34
	b.n _0802C610
_0802C606:
	cmp r2, #0
	bge.n _0802C610
	adds r0, r1, #0
	bl 0x0802C488
_0802C610:
	pop {r0}
	bx r0

@ End of region 0x0802C548-0x0802C614. sub_0802C5C0's span ends exactly here,
@ so this anchor is the precise end marker, not a bounded-region guess.
sound_followon_end:
