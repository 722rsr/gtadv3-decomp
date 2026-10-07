@ GT Advance 3 - sound voice flag updater
@ Region: file offset 0x02D4A8-0x02D510 (VMA 0x0802D4A8-0x0802D510).
@ Pure Thumb with a private Smsh sentinel pool; byte-exact.

.thumb
.type sub_0802D4A8, %function
sub_0802D4A8:
_0802D4A8:
	push {r4, r5, r6, r7, lr}
	mov r7, r9
	mov r6, r8
	push {r6, r7}
	adds r4, r0, #0
	lsls r1, r1, #16
	lsrs r7, r1, #16
	lsls r6, r2, #16
	ldr r3, [r4, #52]
	ldr r0, _0802D50C
	cmp r3, r0
	bne.n _0802D500
	adds r0, r3, #1
	str r0, [r4, #52]
	ldrb r2, [r4, #8]
	ldr r1, [r4, #44]
	movs r5, #1
	cmp r2, #0
	ble.n _0802D4FC
	movs r0, #128
	mov r8, r0
	lsrs r6, r6, #18
	movs r0, #3
	mov ip, r0
_0802D4D8:
	adds r0, r7, #0
	ands r0, r5
	cmp r0, #0
	beq.n _0802D4F2
	ldrb r3, [r1, #0]
	mov r0, r8
	ands r0, r3
	cmp r0, #0
	beq.n _0802D4F2
	strb r6, [r1, #19]
	mov r0, ip
	orrs r0, r3
	strb r0, [r1, #0]
_0802D4F2:
	subs r2, #1
	adds r1, #80
	lsls r5, r5, #1
	cmp r2, #0
	bgt.n _0802D4D8
_0802D4FC:
	ldr r0, _0802D50C
	str r0, [r4, #52]
_0802D500:
	pop {r3, r4}
	mov r8, r3
	mov r9, r4
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_0802D50C: .word 0x68736D53
