@ GT Advance 3 - sound voice-list cleanup
@ Region: file offset 0x02C11C-0x02C160 (VMA 0x0802C11C-0x0802C160).
@ Pure Thumb; byte-exact transcription including the private root pool.

.thumb
.type sub_0802C11C, %function
sub_0802C11C:
_0802C11C:
	push {r4, r5, r6, lr}
	adds r5, r1, #0
	ldrb r1, [r5, #0]
	movs r0, #128
	tst r0, r1
	beq.n _0802C154
	ldr r4, [r5, #32]
	cmp r4, #0
	beq.n _0802C152
	movs r6, #0
_0802C130:
	ldrb r0, [r4, #0]
	cmp r0, #0
	beq.n _0802C14A
	ldrb r0, [r4, #1]
	movs r3, #7
	ands r0, r3
	beq.n _0802C148
	ldr r3, _0802C15C
	ldr r3, [r3, #0]
	ldr r3, [r3, #44]
	bl 0x0802C10C
_0802C148:
	strb r6, [r4, #0]
_0802C14A:
	str r6, [r4, #44]
	ldr r4, [r4, #52]
	cmp r4, #0
	bne.n _0802C130
_0802C152:
	str r4, [r5, #32]
_0802C154:
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.short 0
_0802C15C: .word 0x03007FF0
