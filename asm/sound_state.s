@ GT Advance 3 - MTO sound driver: state initializer
@ Region: file offset 0x02C8C4-0x02C990 (VMA 0x0802C8C4-0x0802C990).
@ Pure Thumb with one complete literal pool; byte-exact.

.thumb
.type sub_0802C8C4, %function
sub_0802C8C4:
_0802C8C4:
	push {r4, r5, lr}
	sub sp, #4
	adds r5, r0, #0
	movs r3, #0
	str r3, [r5, #0]
	ldr r2, _0802C95C
	ldr r0, [r2, #0]
	movs r1, #128
	lsls r1, r1, #18
	ands r0, r1
	cmp r0, #0
	beq.n _0802C8E0
	ldr r0, _0802C960
	str r0, [r2, #0]
_0802C8E0:
	ldr r1, _0802C964
	movs r2, #128
	lsls r2, r2, #3
	adds r0, r2, #0
	strh r0, [r1, #0]
	subs r1, #66
	movs r0, #143
	strh r0, [r1, #0]
	subs r1, #2
	ldr r2, _0802C968
	adds r0, r2, #0
	strh r0, [r1, #0]
	ldr r2, _0802C96C
	ldrb r1, [r2, #0]
	movs r0, #63
	ands r0, r1
	movs r1, #64
	orrs r0, r1
	strb r0, [r2, #0]
	ldr r1, _0802C970
	movs r2, #212
	lsls r2, r2, #2
	adds r0, r5, r2
	str r0, [r1, #0]
	adds r1, #4
	ldr r0, _0802C974
	str r0, [r1, #0]
	ldr r0, _0802C978
	str r5, [r0, #0]
	str r3, [sp, #0]
	ldr r2, _0802C97C
	mov r0, sp
	adds r1, r5, #0
	bl 0x0802D974
	movs r0, #8
	strb r0, [r5, #6]
	movs r0, #15
	strb r0, [r5, #7]
	ldr r0, _0802C980
	str r0, [r5, #56]
	ldr r0, _0802C984
	str r0, [r5, #40]
	str r0, [r5, #44]
	str r0, [r5, #48]
	str r0, [r5, #60]
	ldr r4, _0802C988
	adds r0, r4, #0
	bl 0x0802BCB4
	str r4, [r5, #52]
	movs r0, #128
	lsls r0, r0, #11
	bl 0x0802C990
	ldr r0, _0802C98C
	str r0, [r5, #0]
	add sp, #4
	pop {r4, r5}
	pop {r0}
	bx r0
	.short 0x0000
_0802C95C: .word 0x040000C4
_0802C960: .word 0x84400004
_0802C964: .word 0x040000C6
_0802C968: .word 0x00000B0E
_0802C96C: .word 0x04000089
_0802C970: .word 0x040000BC
_0802C974: .word 0x040000A0
_0802C978: .word 0x03007FF0
_0802C97C: .word 0x05000260
_0802C980: .word 0x0802C191
_0802C984: .word 0x0802D96D
_0802C988: .word 0x0203EBB0
_0802C98C: .word 0x68736D53

@ Unique end marker so the body owned by this file can be C-owned.
@ Emits no bytes.
sound_state_end:
