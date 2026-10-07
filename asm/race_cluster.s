@ GT Advance 3 - race caller/dispatcher cluster
@ Region: file offset 0x01A008-0x01A204 (VMA 0x0801A008-0x0801A204).
@ Exact ARMCC Thumb transcription; pools, jump table, and pads kept at original ROM offsets.
@ Four leaf resource callers plus the 8-way dispatcher that fans out to them and
@ to the course-load helpers.  Pools are word-aligned with.align 2.

	.thumb

	.type sub_0801A008, %function
sub_0801A008:
	push {r4, r5, r6, lr}
	ldr r4, _0801A034
	movs r5, #0
	ldr r0, _0801A038
	ldr r2, _0801A03C
	adds r1, r0, r2
	movs r2, #0
	ldrsh r0, [r1, r2]
	cmp r5, r0
	bge _0801A068
	adds r6, r1, #0
_0801A01E:
	movs r0, #32
	bl 0x08018ACC
	cmp r0, #0
	beq _0801A040
	movs r2, #3
	cmp r5, #0
	bne _0801A048
	movs r2, #2
	b _0801A048
	.align 2, 0
_0801A034:
	.word 0x03004E80
_0801A038:
	.word 0x03001780
_0801A03C:
	.word 0x000010CA
_0801A040:
	movs r2, #5
	cmp r5, #0
	bne _0801A048
	movs r2, #1
_0801A048:
	ldrh r1, [r4, #26]
	adds r0, r5, #0
	bl 0x080263C4
	movs r2, #140
	lsls r2, r2, #1
	adds r1, r4, r2
	str r0, [r1]
	adds r5, #1
	movs r0, #142
	lsls r0, r0, #1
	adds r4, r4, r0
	movs r1, #0
	ldrsh r0, [r6, r1]
	cmp r5, r0
	blt _0801A01E
_0801A068:
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.short 0

	.type sub_0801A070, %function
sub_0801A070:
	push {r4, r5, lr}
	ldr r4, _0801A0B8
	movs r0, #32
	bl 0x08018ACC
	movs r2, #1
	cmp r0, #0
	beq _0801A082
	movs r2, #2
_0801A082:
	ldrh r1, [r4, #26]
	movs r0, #0
	bl 0x080263C4
	movs r5, #140
	lsls r5, r5, #1
	adds r1, r4, r5
	str r0, [r1]
	movs r0, #142
	lsls r0, r0, #1
	adds r4, r4, r0
	movs r0, #32
	bl 0x08018ACC
	movs r2, #4
	cmp r0, #0
	beq _0801A0A6
	movs r2, #3
_0801A0A6:
	ldrh r1, [r4, #26]
	movs r0, #1
	bl 0x080263C4
	adds r1, r4, r5
	str r0, [r1]
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_0801A0B8:
	.word 0x03004E80

	.type sub_0801A0BC, %function
sub_0801A0BC:
	push {r4, lr}
	ldr r4, _0801A0E4
	movs r0, #32
	bl 0x08018ACC
	movs r2, #1
	cmp r0, #0
	beq _0801A0CE
	movs r2, #2
_0801A0CE:
	ldrh r1, [r4, #26]
	movs r0, #0
	bl 0x080263C4
	movs r2, #140
	lsls r2, r2, #1
	adds r1, r4, r2
	str r0, [r1]
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0801A0E4:
	.word 0x03004E80

	.type sub_0801A0E8, %function
sub_0801A0E8:
	push {r4, lr}
	ldr r4, _0801A100
	ldr r0, _0801A104
	ldr r1, _0801A108
	adds r0, r0, r1
	ldrh r0, [r0]
	cmp r0, #10
	bne _0801A10C
	ldrh r1, [r4, #26]
	movs r0, #0
	movs r2, #0
	b _0801A118
_0801A100:
	.word 0x03004E80
_0801A104:
	.word 0x03001780
_0801A108:
	.word 0x000010FC
_0801A10C:
	movs r0, #32
	bl 0x08018ACC
	ldrh r1, [r4, #26]
	movs r0, #0
	movs r2, #1
_0801A118:
	bl 0x080263C4
	movs r2, #140
	lsls r2, r2, #1
	adds r1, r4, r2
	str r0, [r1]
	pop {r4}
	pop {r0}
	bx r0
	.short 0

	.type sub_0801A12C, %function
sub_0801A12C:
	push {r4, r5, r6, r7, lr}
	ldr r0, _0801A158
	ldr r1, _0801A15C
	bl 0x0802620C
	bl 0x08026220
	ldr r0, _0801A160
	ldr r1, _0801A164
	adds r0, r0, r1
	ldrh r0, [r0]
	subs r0, #1
	lsls r0, r0, #16
	asrs r0, r0, #16
	cmp r0, #7
	bhi _0801A19E
	lsls r0, r0, #2
	ldr r1, _0801A168
	adds r0, r0, r1
	ldr r0, [r0]
	mov pc, r0

	.align 2, 0
_0801A158:
	.word 0x03004E30
_0801A15C:
	.word 0x02010000
_0801A160:
	.word 0x03001780
_0801A164:
	.word 0x000010FC
_0801A168:
	.word _0801A16C
_0801A16C:
	.word 0x0801A18C
	.word 0x0801A19E
	.word 0x0801A192
	.word 0x0801A18C
	.word 0x0801A198
	.word 0x0801A19E
	.word 0x0801A19E
	.word 0x0801A192

_0801A18C:
	bl sub_0801A008
	b _0801A1A2
_0801A192:
	bl sub_0801A070
	b _0801A1A2
_0801A198:
	bl sub_0801A0BC
	b _0801A1A2
_0801A19E:
	bl sub_0801A0E8
_0801A1A2:
	movs r4, #0
	ldr r2, _0801A1F0
	ldr r3, _0801A1F4
	adds r1, r2, r3
	movs r3, #0
	ldrsh r0, [r1, r3]
	cmp r4, r0
	bge _0801A1EA
	adds r7, r2, #0
	adds r6, r1, #0
	movs r5, #0
_0801A1B8:
	ldr r0, _0801A1F8
	adds r0, r5, r0
	ldrh r0, [r0, #26]
	lsls r1, r4, #1
	ldr r3, _0801A1FC
	adds r2, r7, r3
	adds r1, r1, r2
	movs r2, #0
	ldrsh r1, [r1, r2]
	bl 0x08026520
	lsls r1, r4, #5
	ldr r2, _0801A200
	adds r1, r1, r2
	movs r2, #8
	bl 0x0802D970
	movs r3, #142
	lsls r3, r3, #1
	adds r5, r5, r3
	adds r4, #1
	movs r1, #0
	ldrsh r0, [r6, r1]
	cmp r4, r0
	blt _0801A1B8
_0801A1EA:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0

	.align 2, 0
_0801A1F0:
	.word 0x03001780
_0801A1F4:
	.word 0x000010CA
_0801A1F8:
	.word 0x03004E80
_0801A1FC:
	.word 0x00001DE4
_0801A200:
	.word 0x03005780
