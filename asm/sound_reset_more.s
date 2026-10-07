@ GT Advance 3 - MTO sound driver: command/reset helpers
@ Region: file offset 0x02CA34-0x02CB20 (VMA 0x0802CA34-0x0802CB20).
@ Two pure-Thumb functions with local pools; byte-exact.

.thumb
.type sub_0802CA34, %function
sub_0802CA34:
_0802CA34:
	push {r4, r5, lr}
	adds r3, r0, #0
	ldr r0, _0802CAC0
	ldr r5, [r0, #0]
	ldr r1, [r5, #0]
	ldr r0, _0802CAC4
	cmp r1, r0
	bne.n _0802CABA
	adds r0, r1, #1
	str r0, [r5, #0]
	movs r4, #255
	ands r4, r3
	cmp r4, #0
	beq.n _0802CA56
	movs r0, #127
	ands r4, r0
	strb r4, [r5, #5]
_0802CA56:
	movs r4, #240
	lsls r4, r4, #4
	ands r4, r3
	cmp r4, #0
	beq.n _0802CA76
	lsrs r0, r4, #8
	strb r0, [r5, #6]
	movs r4, #12
	adds r0, r5, #0
	adds r0, #80
	movs r1, #0
_0802CA6C:
	strb r1, [r0, #0]
	subs r4, #1
	adds r0, #64
	cmp r4, #0
	bne.n _0802CA6C
_0802CA76:
	movs r4, #240
	lsls r4, r4, #8
	ands r4, r3
	cmp r4, #0
	beq.n _0802CA84
	lsrs r0, r4, #12
	strb r0, [r5, #7]
_0802CA84:
	movs r4, #176
	lsls r4, r4, #16
	ands r4, r3
	cmp r4, #0
	beq.n _0802CAA2
	movs r0, #192
	lsls r0, r0, #14
	ands r0, r4
	lsrs r4, r0, #14
	ldr r2, _0802CAC8
	ldrb r1, [r2, #0]
	movs r0, #63
	ands r0, r1
	orrs r0, r4
	strb r0, [r2, #0]
_0802CAA2:
	movs r4, #240
	lsls r4, r4, #12
	ands r4, r3
	cmp r4, #0
	beq.n _0802CAB6
	bl 0x0802CB20
	adds r0, r4, #0
	bl 0x0802C990
_0802CAB6:
	ldr r0, _0802CAC4
	str r0, [r5, #0]
_0802CABA:
	pop {r4, r5}
	pop {r0}
	bx r0
_0802CAC0: .word 0x03007FF0
_0802CAC4: .word 0x68736D53
_0802CAC8: .word 0x04000089

.type sub_0802CACC, %function
sub_0802CACC:
_0802CACC:
	push {r4, r5, r6, r7, lr}
	ldr r0, _0802CB18
	ldr r6, [r0, #0]
	ldr r1, [r6, #0]
	ldr r0, _0802CB1C
	cmp r1, r0
	bne.n _0802CB12
	adds r0, r1, #1
	str r0, [r6, #0]
	movs r5, #12
	adds r4, r6, #0
	adds r4, #80
	movs r0, #0
_0802CAE6:
	strb r0, [r4, #0]
	subs r5, #1
	adds r4, #64
	cmp r5, #0
	bgt.n _0802CAE6
	ldr r4, [r6, #28]
	cmp r4, #0
	beq.n _0802CB0E
	movs r5, #1
	movs r7, #0
_0802CAFA:
	lsls r0, r5, #24
	lsrs r0, r0, #24
	ldr r1, [r6, #44]
	bl 0x0802DDCC
	strb r7, [r4, #0]
	adds r5, #1
	adds r4, #64
	cmp r5, #4
	ble.n _0802CAFA
_0802CB0E:
	ldr r0, _0802CB1C
	str r0, [r6, #0]
_0802CB12:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_0802CB18: .word 0x03007FF0
_0802CB1C: .word 0x68736D53
