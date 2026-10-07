@ GT Advance 3 - MTO sound driver: stop all active streams
@ Region: file offset 0x02CD18-0x02CD58 (VMA 0x0802CD18-0x0802CD58).
@ Pure Thumb with a private magic pool; byte-exact.

.thumb
.type sub_0802CD18, %function
sub_0802CD18:
_0802CD18:
	push {r4, r5, r6, lr}
	adds r6, r0, #0
	ldr r1, [r6, #52]
	ldr r0, _0802CD54
	cmp r1, r0
	bne.n _0802CD4E
	adds r0, r1, #1
	str r0, [r6, #52]
	ldr r0, [r6, #4]
	movs r1, #128
	lsls r1, r1, #24
	orrs r0, r1
	str r0, [r6, #4]
	ldrb r4, [r6, #8]
	ldr r5, [r6, #44]
	cmp r4, #0
	ble.n _0802CD4A
_0802CD3A:
	adds r0, r6, #0
	adds r1, r5, #0
	bl 0x0802C11C
	subs r4, #1
	adds r5, #80
	cmp r4, #0
	bgt.n _0802CD3A
_0802CD4A:
	ldr r0, _0802CD54
	str r0, [r6, #52]
_0802CD4E:
	pop {r4, r5, r6}
	pop {r0}
	bx r0
_0802CD54: .word 0x68736D53
