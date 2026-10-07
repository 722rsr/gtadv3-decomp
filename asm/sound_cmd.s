@ GT Advance 3 - MTO sound driver: command commit helper
@ Region: file offset 0x02CB20-0x02CB84 (VMA 0x0802CB20-0x0802CB84).
@ Pure Thumb with a local pool; byte-exact.

.thumb
.type sub_0802CB20, %function
sub_0802CB20:
_0802CB20:
	push {lr}
	sub sp, #4
	ldr r0, _0802CB6C
	ldr r2, [r0, #0]
	ldr r1, [r2, #0]
	ldr r3, _0802CB70
	adds r0, r1, r3
	cmp r0, #1
	bhi.n _0802CB66
	adds r0, r1, #0
	adds r0, #10
	str r0, [r2, #0]
	ldr r3, _0802CB74
	ldr r0, [r3, #0]
	movs r1, #128
	lsls r1, r1, #18
	ands r0, r1
	cmp r0, #0
	beq.n _0802CB4A
	ldr r0, _0802CB78
	str r0, [r3, #0]
_0802CB4A:
	ldr r1, _0802CB7C
	movs r3, #128
	lsls r3, r3, #3
	adds r0, r3, #0
	strh r0, [r1, #0]
	movs r0, #0
	str r0, [sp, #0]
	movs r0, #212
	lsls r0, r0, #2
	adds r1, r2, r0
	ldr r2, _0802CB80
	mov r0, sp
	bl 0x0802D974
_0802CB66:
	add sp, #4
	pop {r0}
	bx r0
_0802CB6C: .word 0x03007FF0
_0802CB70: .word 0x978C92AD
_0802CB74: .word 0x040000C4
_0802CB78: .word 0x84400004
_0802CB7C: .word 0x040000C6
_0802CB80: .word 0x0500018C

@ Region end 0x0802CB84. The next function's VMA labels live in sound_stop.s,
@ so this file cannot spell them; this anchor gives tools/match_c_slice.py a
@ unique end marker when this body is C-owned. Emits no bytes.
sound_cmd_end:
