@ GT Advance 3 - collection-line bit probe
@ Region: file offset 0x025214-0x025248 (VMA 0x08025214-0x08025248).
@ Pure Thumb; byte-exact transcription.

.thumb
.type sub_08025214, %function
sub_08025214:
_08025214:
	adds r3, r0, #0
	cmp r3, #0
	bge.n _0802521C
	adds r0, r3, #7
_0802521C:
	asrs r0, r0, #3
	ldr r1, _0802523C
	adds r1, r0, r1
	ldr r2, _08025240
	lsls r0, r0, #3
	subs r0, r3, r0
	adds r0, r0, r2
	ldrb r1, [r1, #0]
	ldrb r0, [r0, #0]
	ands r1, r0
	adds r0, r1, #0
	cmp r0, #0
	bne.n _08025244
	movs r0, #0
	b.n _08025246
	.short 0
_0802523C: .word 0x030015E8
_08025240: .word 0x080C4768
_08025244:
	movs r0, #1
_08025246:
	bx lr

ai_line_probe_end:
