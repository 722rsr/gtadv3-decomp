@ GT Advance 3 - sound voice flag/pitch updater
@ Region: file offset 0x02D510-0x02D584 (VMA 0x0802D510-0x0802D584).
@ Pure Thumb with a private Smsh sentinel pool; byte-exact.

.thumb
.type sub_0802D510, %function
sub_0802D510:
_0802D510:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	adds r4, r0, #0
	lsls r1, r1, #16
	lsrs r1, r1, #16
	mov ip, r1
	lsls r2, r2, #16
	lsrs r6, r2, #16
	ldr r3, [r4, #52]
	ldr r0, _0802D580
	cmp r3, r0
	bne.n _0802D572
	adds r0, r3, #1
	str r0, [r4, #52]
	ldrb r2, [r4, #8]
	ldr r3, [r4, #44]
	movs r5, #1
	cmp r2, #0
	ble.n _0802D56E
	movs r0, #128
	mov r9, r0
	lsls r0, r6, #16
	asrs r7, r0, #24
	movs r0, #12
	mov r8, r0
_0802D548:
	mov r0, ip
	ands r0, r5
	cmp r0, #0
	beq.n _0802D564
	ldrb r1, [r3, #0]
	mov r0, r9
	ands r0, r1
	cmp r0, #0
	beq.n _0802D564
	strb r7, [r3, #11]
	strb r6, [r3, #13]
	mov r0, r8
	orrs r0, r1
	strb r0, [r3, #0]
_0802D564:
	subs r2, #1
	adds r3, #80
	lsls r5, r5, #1
	cmp r2, #0
	bgt.n _0802D548
_0802D56E:
	ldr r0, _0802D580
	str r0, [r4, #52]
_0802D572:
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_0802D580: .word 0x68736D53
sound_d510_end:
