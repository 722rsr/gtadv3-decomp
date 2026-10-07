@ GT Advance 3 - sound voice parameter leaves
@ Region: file offset 0x02D584-0x02D6F4 (VMA 0x0802D584-0x0802D6F4).
@ Three pure-Thumb functions with private Smsh pools; byte-exact.

.thumb
.type sub_0802D584, %function
sub_0802D584:
_0802D584:
	push {r4, r5, r6, r7, lr}
	mov r7, r9
	mov r6, r8
	push {r6, r7}
	adds r4, r0, #0
	lsls r1, r1, #16
	lsrs r7, r1, #16
	lsls r2, r2, #24
	lsrs r6, r2, #24
	ldr r3, [r4, #52]
	ldr r0, _0802D5E8
	cmp r3, r0
	bne.n _0802D5DC
	adds r0, r3, #1
	str r0, [r4, #52]
	ldrb r2, [r4, #8]
	ldr r1, [r4, #44]
	movs r5, #1
	cmp r2, #0
	ble.n _0802D5D8
	movs r0, #128
	mov r8, r0
	movs r0, #3
	mov ip, r0
_0802D5B4:
	adds r0, r7, #0
	ands r0, r5
	cmp r0, #0
	beq.n _0802D5CE
	ldrb r3, [r1, #0]
	mov r0, r8
	ands r0, r3
	cmp r0, #0
	beq.n _0802D5CE
	strb r6, [r1, #21]
	mov r0, ip
	orrs r0, r3
	strb r0, [r1, #0]
_0802D5CE:
	subs r2, #1
	adds r1, #80
	lsls r5, r5, #1
	cmp r2, #0
	bgt.n _0802D5B4
_0802D5D8:
	ldr r0, _0802D5E8
	str r0, [r4, #52]
_0802D5DC:
	pop {r3, r4}
	mov r8, r3
	mov r9, r4
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_0802D5E8: .word 0x68736D53

.type sub_0802D5EC, %function
sub_0802D5EC:
_0802D5EC:
	adds r1, r0, #0
	movs r2, #0
	movs r0, #0
	strb r0, [r1, #26]
	strb r0, [r1, #22]
	ldrb r0, [r1, #24]
	cmp r0, #0
	bne.n _0802D600
	movs r0, #12
	b.n _0802D602
_0802D600:
	movs r0, #3
_0802D602:
	ldrb r2, [r1, #0]
	orrs r0, r2
	strb r0, [r1, #0]
	bx lr
	.short 0

.type sub_0802D60C, %function
sub_0802D60C:
_0802D60C:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	adds r6, r0, #0
	lsls r1, r1, #16
	lsrs r1, r1, #16
	mov sl, r1
	lsls r2, r2, #24
	lsrs r2, r2, #24
	mov r8, r2
	ldr r1, [r6, #52]
	ldr r0, _0802D67C
	cmp r1, r0
	bne.n _0802D66C
	adds r0, r1, #1
	str r0, [r6, #52]
	ldrb r5, [r6, #8]
	ldr r4, [r6, #44]
	movs r7, #1
	cmp r5, #0
	ble.n _0802D668
	mov r9, r8
_0802D63C:
	mov r0, sl
	ands r0, r7
	cmp r0, #0
	beq.n _0802D65E
	movs r0, #128
	ldrb r1, [r4, #0]
	ands r0, r1
	cmp r0, #0
	beq.n _0802D65E
	mov r0, r8
	strb r0, [r4, #23]
	mov r1, r9
	cmp r1, #0
	bne.n _0802D65E
	adds r0, r4, #0
	bl 0x0802D5EC
_0802D65E:
	subs r5, #1
	adds r4, #80
	lsls r7, r7, #1
	cmp r5, #0
	bgt.n _0802D63C
_0802D668:
	ldr r0, _0802D67C
	str r0, [r6, #52]
_0802D66C:
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
_0802D67C: .word 0x68736D53

.type sub_0802D680, %function
sub_0802D680:
_0802D680:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	adds r6, r0, #0
	lsls r1, r1, #16
	lsrs r1, r1, #16
	mov sl, r1
	lsls r2, r2, #24
	lsrs r2, r2, #24
	mov r8, r2
	ldr r1, [r6, #52]
	ldr r0, _0802D6F0
	cmp r1, r0
	bne.n _0802D6E0
	adds r0, r1, #1
	str r0, [r6, #52]
	ldrb r5, [r6, #8]
	ldr r4, [r6, #44]
	movs r7, #1
	cmp r5, #0
	ble.n _0802D6DC
	mov r9, r8
_0802D6B0:
	mov r0, sl
	ands r0, r7
	cmp r0, #0
	beq.n _0802D6D2
	movs r0, #128
	ldrb r1, [r4, #0]
	ands r0, r1
	cmp r0, #0
	beq.n _0802D6D2
	mov r0, r8
	strb r0, [r4, #25]
	mov r1, r9
	cmp r1, #0
	bne.n _0802D6D2
	adds r0, r4, #0
	bl 0x0802D5EC
_0802D6D2:
	subs r5, #1
	adds r4, #80
	lsls r7, r7, #1
	cmp r5, #0
	bgt.n _0802D6B0
_0802D6DC:
	ldr r0, _0802D6F0
	str r0, [r6, #52]
_0802D6E0:
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
_0802D6F0: .word 0x68736D53

sound_d584_end:
