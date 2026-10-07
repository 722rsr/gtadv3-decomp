@ GT Advance 3 - menu record-handler update
@ Region: file offset 0x00C884-0x00CAE4 (VMA 0x0800C884-0x0800CAE4).
@ Pure Thumb; byte-exact transcription.

.thumb
.type sub_0800C884, %function
sub_0800C884:
_0800C884:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	adds r5, r0, #0
	lsls r1, r1, #16
	lsrs r1, r1, #16
	mov r9, r1
	lsls r2, r2, #16
	lsrs r7, r2, #16
	bl 0x080024A0
	lsls r0, r0, #16
	lsrs r0, r0, #16
	mov sl, r0
	movs r0, #0
	mov r8, r0
	movs r6, #1
	bl 0x08004B68
	bl 0x08002140
	cmp r0, #1
	ble.n _0800C8E8
	movs r4, #0
	b.n _0800C8DE
_0800C8BA:
	adds r0, r4, #0
	movs r1, #2
	bl 0x08002178
	lsls r0, r0, #16
	lsrs r0, r0, #16
	cmp r0, #5
	beq.n _0800C8CC
	movs r6, #0
_0800C8CC:
	adds r0, r4, #0
	movs r1, #3
	bl 0x08002178
	lsls r0, r0, #16
	cmp r0, #0
	beq.n _0800C8DC
	movs r6, #0
_0800C8DC:
	adds r4, #1
_0800C8DE:
	bl 0x08002140
	cmp r4, r0
	blt.n _0800C8BA
	b.n _0800C8EA
_0800C8E8:
	movs r6, #0
_0800C8EA:
	cmp r6, #0
	beq.n _0800C906
	ldrh r1, [r5, #4]
	movs r2, #4
	ldrsh r0, [r5, r2]
	cmp r0, #0
	ble.n _0800C8FE
	subs r0, r1, #1
	strh r0, [r5, #4]
	b.n _0800C990
_0800C8FE:
	adds r0, r5, #0
	bl 0x0800C814
	b.n _0800C990
_0800C906:
	cmp r7, #1
	bne.n _0800C926
	ldrh r3, [r5, #0]
	cmp r3, #6
	bne.n _0800C926
	movs r1, #0
	ldrsh r0, [r5, r1]
	movs r1, #29
	ldrsb r1, [r5, r1]
	adds r0, r0, r1
	bl 0x08004BFC
	movs r0, #1
	bl 0x08004EC0
	b.n _0800C982
_0800C926:
	movs r0, #1
	ands r0, r7
	cmp r0, #0
	beq.n _0800C982
	movs r2, #0
	ldrsh r0, [r5, r2]
	cmp r0, #5
	beq.n _0800C982
	cmp r0, #1
	beq.n _0800C982
	movs r0, #128
	lsls r0, r0, #2
	mov r3, r9
	ands r0, r3
	cmp r0, #0
	beq.n _0800C94E
	movs r0, #100
	bl 0x08004BFC
	b.n _0800C96A
_0800C94E:
	movs r0, #128
	lsls r0, r0, #1
	mov r1, r9
	ands r0, r1
	cmp r0, #0
	beq.n _0800C962
	movs r0, #101
	bl 0x08004BFC
	b.n _0800C96A
_0800C962:
	movs r2, #0
	ldrsh r0, [r5, r2]
	bl 0x08004BFC
_0800C96A:
	movs r3, #0
	ldrsh r1, [r5, r3]
	movs r0, #0
	bl 0x08004C84
	bl 0x08002044
	bl 0x08001F80
	movs r0, #1
	bl 0x08004EC0
_0800C982:
	movs r0, #2
	ands r0, r7
	cmp r0, #0
	beq.n _0800C990
	movs r0, #1
	bl 0x08004EA8
_0800C990:
	movs r0, #64
	ands r0, r7
	cmp r0, #0
	beq.n _0800C9A6
	ldrh r1, [r5, #0]
	movs r2, #0
	ldrsh r0, [r5, r2]
	cmp r0, #0
	ble.n _0800C9A6
	subs r0, r1, #1
	strh r0, [r5, #0]
_0800C9A6:
	movs r0, #128
	ands r0, r7
	cmp r0, #0
	beq.n _0800C9BC
	ldrh r1, [r5, #0]
	movs r3, #0
	ldrsh r0, [r5, r3]
	cmp r0, #5
	bgt.n _0800C9BC
	adds r0, r1, #1
	strh r0, [r5, #0]
_0800C9BC:
	movs r0, #32
	mov r1, sl
	ands r0, r1
	cmp r0, #0
	beq.n _0800C9CC
	movs r2, #1
	negs r2, r2
	add r8, r2
_0800C9CC:
	movs r0, #16
	mov r3, sl
	ands r0, r3
	cmp r0, #0
	beq.n _0800C9DA
	movs r0, #1
	add r8, r0
_0800C9DA:
	movs r0, #128
	lsls r0, r0, #2
	ands r0, r7
	cmp r0, #0
	beq.n _0800CA02
	movs r1, #0
	ldrsh r2, [r5, r1]
	cmp r2, #0
	bne.n _0800CA02
	ldr r0, _0800CA3C
	ldr r3, _0800CA40
	adds r1, r0, r3
	ldrh r0, [r1, #0]
	adds r0, #1
	strh r0, [r1, #0]
	lsls r0, r0, #16
	asrs r0, r0, #16
	cmp r0, #2
	ble.n _0800CA02
	strh r2, [r1, #0]
_0800CA02:
	movs r0, #128
	lsls r0, r0, #1
	ands r0, r7
	cmp r0, #0
	beq.n _0800CA2A
	movs r0, #0
	ldrsh r2, [r5, r0]
	cmp r2, #0
	bne.n _0800CA2A
	ldr r0, _0800CA3C
	ldr r3, _0800CA44
	adds r1, r0, r3
	ldrh r0, [r1, #0]
	adds r0, #1
	strh r0, [r1, #0]
	lsls r0, r0, #16
	asrs r0, r0, #16
	cmp r0, #2
	ble.n _0800CA2A
	strh r2, [r1, #0]
_0800CA2A:
	movs r0, #0
	ldrsh r1, [r5, r0]
	cmp r1, #1
	beq.n _0800CA6C
	cmp r1, #1
	bgt.n _0800CA48
	cmp r1, #0
	beq.n _0800CA4E
	b.n _0800CAB4
_0800CA3C: .word 0x03001780
_0800CA40: .word 0x0000113C
_0800CA44: .word 0x0000113E
_0800CA48:
	cmp r1, #6
	beq.n _0800CA98
	b.n _0800CAB4
_0800CA4E:
	ldr r0, _0800CA64
	ldr r3, _0800CA68
	adds r2, r0, r3
	ldrh r0, [r2, #0]
	add r0, r8
	strh r0, [r2, #0]
	lsls r0, r0, #16
	cmp r0, #0
	bge.n _0800CAB4
	strh r1, [r2, #0]
	b.n _0800CAB4
_0800CA64: .word 0x03001780
_0800CA68: .word 0x00000576
_0800CA6C:
	ldr r0, _0800CA90
	ldr r2, _0800CA94
	adds r1, r0, r2
	ldrh r0, [r1, #0]
	add r0, r8
	strh r0, [r1, #0]
	lsls r0, r0, #16
	cmp r0, #0
	bge.n _0800CA82
	movs r0, #0
	strh r0, [r1, #0]
_0800CA82:
	movs r3, #0
	ldrsh r0, [r1, r3]
	cmp r0, #96
	ble.n _0800CAB4
	movs r0, #96
	strh r0, [r1, #0]
	b.n _0800CAB4
_0800CA90: .word 0x03001780
_0800CA94: .word 0x00000574
_0800CA98:
	ldrb r0, [r5, #29]
	add r0, r8
	strb r0, [r5, #29]
	lsls r0, r0, #24
	cmp r0, #0
	bge.n _0800CAA8
	movs r0, #0
	strb r0, [r5, #29]
_0800CAA8:
	movs r0, #29
	ldrsb r0, [r5, r0]
	cmp r0, #4
	ble.n _0800CAB4
	movs r0, #4
	strb r0, [r5, #29]
_0800CAB4:
	movs r0, #0
	mov r1, r9
	bl 0x08002158
	movs r0, #1
	adds r1, r7, #0
	bl 0x08002158
	ldrh r1, [r5, #0]
	movs r0, #2
	bl 0x08002158
	ldrb r1, [r5, #28]
	movs r0, #3
	bl 0x08002158
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.short 0
