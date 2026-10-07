@ GT Advance 3 - sound sequence interpreter and field leaves
@ Region: file offset 0x02D6F4-0x02D974 (VMA 0x0802D6F4-0x0802D974).
@ Pure Thumb: interpreter, inline dispatch table, relocated-code dispatchers,
@ and cursor/field leaves. Runtime table entries are preserved byte-exactly.

.thumb
.type sub_0802D6F4, %function
sub_0802D6F4:
_0802D6F4:
	push {r4, r5, r6, lr}
	adds r4, r0, #0
	adds r6, r1, #0
	ldr r1, [r6, #64]
	ldrb r5, [r1, #0]
	adds r2, r1, #1
	str r2, [r6, #64]
	ldr r0, [r4, #24]
	ldrb r1, [r1, #1]
	adds r3, r1, r0
	adds r0, r2, #1
	str r0, [r6, #64]
	ldrb r2, [r2, #1]
	adds r0, #1
	str r0, [r6, #64]
	cmp r5, #17
	bls.n _0802D718
	b.n _0802D846
_0802D718:
	lsls r0, r5, #2
	ldr r1, _0802D724
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	.short 0
_0802D724:
	.word 0x0802D728
	.word 0x0802D770
	.word 0x0802D774
	.word 0x0802D77C
	.word 0x0802D784
	.word 0x0802D78E
	.word 0x0802D79C
	.word 0x0802D7AA
	.word 0x0802D7B2
	.word 0x0802D7BA
	.word 0x0802D7C2
	.word 0x0802D7CA
	.word 0x0802D7D2
	.word 0x0802D7DA
	.word 0x0802D7E8
	.word 0x0802D7F6
	.word 0x0802D804
	.word 0x0802D812
	.word 0x0802D820
_0802D770:
	strb r2, [r3, #0]
	b.n _0802D846
_0802D774:
	ldrb r1, [r3, #0]
	adds r0, r1, r2
	strb r0, [r3, #0]
	b.n _0802D846
_0802D77C:
	ldrb r1, [r3, #0]
	subs r0, r1, r2
	strb r0, [r3, #0]
	b.n _0802D846
_0802D784:
	ldr r0, [r4, #24]
	adds r0, r0, r2
	ldrb r0, [r0, #0]
	strb r0, [r3, #0]
	b.n _0802D846
_0802D78E:
	ldr r0, [r4, #24]
	adds r0, r0, r2
	ldrb r1, [r3, #0]
	ldrb r0, [r0, #0]
	adds r0, r1, r0
	strb r0, [r3, #0]
	b.n _0802D846
_0802D79C:
	ldr r0, [r4, #24]
	adds r0, r0, r2
	ldrb r1, [r3, #0]
	ldrb r0, [r0, #0]
	subs r0, r1, r0
	strb r0, [r3, #0]
	b.n _0802D846
_0802D7AA:
	ldrb r3, [r3, #0]
	cmp r3, r2
	beq.n _0802D82C
	b.n _0802D840
_0802D7B2:
	ldrb r3, [r3, #0]
	cmp r3, r2
	bne.n _0802D82C
	b.n _0802D840
_0802D7BA:
	ldrb r3, [r3, #0]
	cmp r3, r2
	bhi.n _0802D82C
	b.n _0802D840
_0802D7C2:
	ldrb r3, [r3, #0]
	cmp r3, r2
	bcs.n _0802D82C
	b.n _0802D840
_0802D7CA:
	ldrb r3, [r3, #0]
	cmp r3, r2
	bls.n _0802D82C
	b.n _0802D840
_0802D7D2:
	ldrb r3, [r3, #0]
	cmp r3, r2
	bcc.n _0802D82C
	b.n _0802D840
_0802D7DA:
	ldr r0, [r4, #24]
	adds r0, r0, r2
	ldrb r3, [r3, #0]
	ldrb r0, [r0, #0]
	cmp r3, r0
	beq.n _0802D82C
	b.n _0802D840
_0802D7E8:
	ldr r0, [r4, #24]
	adds r0, r0, r2
	ldrb r3, [r3, #0]
	ldrb r0, [r0, #0]
	cmp r3, r0
	bne.n _0802D82C
	b.n _0802D840
_0802D7F6:
	ldr r0, [r4, #24]
	adds r0, r0, r2
	ldrb r3, [r3, #0]
	ldrb r0, [r0, #0]
	cmp r3, r0
	bhi.n _0802D82C
	b.n _0802D840
_0802D804:
	ldr r0, [r4, #24]
	adds r0, r0, r2
	ldrb r3, [r3, #0]
	ldrb r0, [r0, #0]
	cmp r3, r0
	bcs.n _0802D82C
	b.n _0802D840
_0802D812:
	ldr r0, [r4, #24]
	adds r0, r0, r2
	ldrb r3, [r3, #0]
	ldrb r0, [r0, #0]
	cmp r3, r0
	bls.n _0802D82C
	b.n _0802D840
_0802D820:
	ldr r0, [r4, #24]
	adds r0, r0, r2
	ldrb r3, [r3, #0]
	ldrb r0, [r0, #0]
	cmp r3, r0
	bcs.n _0802D840
_0802D82C:
	ldr r0, _0802D83C
	ldr r2, [r0, #0]
	adds r0, r4, #0
	adds r1, r6, #0
	bl 0x0802DDD0
	b.n _0802D846
	.short 0
_0802D83C: .word 0x0203EBB4
_0802D840:
	ldr r0, [r6, #64]
	adds r0, #4
	str r0, [r6, #64]
_0802D846:
	pop {r4, r5, r6}
	pop {r0}
	bx r0

.type sub_0802D84C, %function
sub_0802D84C:
_0802D84C:
    @ GLOBAL for the same reason as sub_0802D034: src/sound_core.c takes its
    @ address into the dispatch table, and a local symbol cannot resolve it.
    .global sub_0802D84C
    .global _0802D84C
	push {lr}
	ldr r2, [r1, #64]
	ldrb r3, [r2, #0]
	adds r2, #1
	str r2, [r1, #64]
	ldr r2, _0802D868
	lsls r3, r3, #2
	adds r3, r3, r2
	ldr r2, [r3, #0]
	bl 0x0802DDD0
	pop {r0}
	bx r0
	.short 0
_0802D868: .word 0x08061788

.type sub_0802D86C, %function
sub_0802D86C:
_0802D86C:
	push {lr}
	ldr r2, _0802D87C
	ldr r2, [r2, #0]
	bl 0x0802DDD0
	pop {r0}
	bx r0
	.short 0
_0802D87C: .word 0x0203EBB0

.type sub_0802D880, %function
sub_0802D880:
_0802D880:
	push {r4, lr}
	ldr r2, [r1, #64]
	ldr r0, _0802D8B8
	ands r4, r0
	ldrb r0, [r2, #0]
	orrs r4, r0
	ldrb r0, [r2, #1]
	lsls r3, r0, #8
	ldr r0, _0802D8BC
	ands r4, r0
	orrs r4, r3
	ldrb r0, [r2, #2]
	lsls r3, r0, #16
	ldr r0, _0802D8C0
	ands r4, r0
	orrs r4, r3
	ldrb r0, [r2, #3]
	lsls r3, r0, #24
	ldr r0, _0802D8C4
	ands r4, r0
	orrs r4, r3
	str r4, [r1, #40]
	adds r2, #4
	str r2, [r1, #64]
	pop {r4}
	pop {r0}
	bx r0
	.short 0
_0802D8B8: .word 0xFFFFFF00
_0802D8BC: .word 0xFFFF00FF
_0802D8C0: .word 0xFF00FFFF
_0802D8C4: .word 0x00FFFFFF

.type sub_0802D8C8, %function
sub_0802D8C8:
_0802D8C8:
	ldr r0, [r1, #64]
	ldrb r2, [r0, #0]
	adds r0, r1, #0
	adds r0, #36
	strb r2, [r0, #0]
	ldr r0, [r1, #64]
	adds r0, #1
	str r0, [r1, #64]
	bx lr
	.short 0

.type sub_0802D8DC, %function
sub_0802D8DC:
_0802D8DC:
	ldr r0, [r1, #64]
	ldrb r2, [r0, #0]
	adds r0, r1, #0
	adds r0, #44
	strb r2, [r0, #0]
	ldr r0, [r1, #64]
	adds r0, #1
	str r0, [r1, #64]
	bx lr
	.short 0

.type sub_0802D8F0, %function
sub_0802D8F0:
_0802D8F0:
	ldr r0, [r1, #64]
	ldrb r0, [r0, #0]
	adds r2, r1, #0
	adds r2, #45
	strb r0, [r2, #0]
	ldr r0, [r1, #64]
	adds r0, #1
	str r0, [r1, #64]
	bx lr
	.short 0

.type sub_0802D904, %function
sub_0802D904:
_0802D904:
	ldr r0, [r1, #64]
	ldrb r0, [r0, #0]
	adds r2, r1, #0
	adds r2, #46
	strb r0, [r2, #0]
	ldr r0, [r1, #64]
	adds r0, #1
	str r0, [r1, #64]
	bx lr
	.short 0

.type sub_0802D918, %function
sub_0802D918:
_0802D918:
	ldr r0, [r1, #64]
	ldrb r0, [r0, #0]
	adds r2, r1, #0
	adds r2, #47
	strb r0, [r2, #0]
	ldr r0, [r1, #64]
	adds r0, #1
	str r0, [r1, #64]
	bx lr
	.short 0

.type sub_0802D92C, %function
sub_0802D92C:
_0802D92C:
	ldr r0, [r1, #64]
	ldrb r2, [r0, #0]
	strb r2, [r1, #30]
	adds r0, #1
	str r0, [r1, #64]
	bx lr
.type sub_0802D938, %function
sub_0802D938:
_0802D938:
	ldr r0, [r1, #64]
	ldrb r2, [r0, #0]
	strb r2, [r1, #31]
	adds r0, #1
	str r0, [r1, #64]
	bx lr

.type sub_0802D944, %function
sub_0802D944:
_0802D944:
	ldr r0, [r1, #64]
	ldrb r0, [r0, #0]
	adds r2, r1, #0
	adds r2, #38
	strb r0, [r2, #0]
	ldr r0, [r1, #64]
	adds r0, #1
	str r0, [r1, #64]
	bx lr
	.short 0

.type sub_0802D958, %function
sub_0802D958:
_0802D958:
	ldr r0, [r1, #64]
	ldrb r0, [r0, #0]
	adds r2, r1, #0
	adds r2, #39
	strb r0, [r2, #0]
	ldr r0, [r1, #64]
	adds r0, #1
	str r0, [r1, #64]
	bx lr
	.short 0

.type sub_0802D96C, %function
sub_0802D96C:
_0802D96C:
	bx lr
	.short 0

.type sub_0802D970, %function
sub_0802D970:
_0802D970:
	swi 0x0C
	bx lr
sound_d6f4_end:
