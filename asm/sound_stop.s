@ GT Advance 3 - MTO sound driver: stop/clear current sequence state
@ Region: file offset 0x02CB84-0x02CBBC (VMA 0x0802CB84-0x0802CBBC).
@ Pure Thumb with a three-word local pool; byte-exact.

.thumb
.type sub_0802CB84, %function
sub_0802CB84:
_0802CB84:
	push {r4, lr}
	ldr r0, _0802CBB0
	ldr r2, [r0, #0]
	ldr r3, [r2, #0]
	ldr r0, _0802CBB4
	cmp r3, r0
	beq.n _0802CBA8
	ldr r1, _0802CBB8
	movs r4, #0xB6
	lsls r4, r4, #8
	adds r0, r4, #0
	strh r0, [r1, #0]
	ldrb r0, [r2, #4]
	movs r0, #0
	strb r0, [r2, #4]
	adds r0, r3, #0
	subs r0, #10
	str r0, [r2, #0]
_0802CBA8:
	pop {r4}
	pop {r0}
	bx r0
	.short 0
_0802CBB0: .word 0x03007FF0
_0802CBB4: .word 0x68736D53
_0802CBB8: .word 0x040000C6

@ End of region 0x0802CBBC. The last function here ends exactly at 0x0802CBBC, where
@ the next region file's prologue begins, so this anchor is the precise
@ end marker -- not a bounded-region guess. Without it the last body in
@ this file has no end marker to fall back on and the promotion screen
@ refuses it. Emits no bytes; the following file supplies the address.
sound_stop_end:
