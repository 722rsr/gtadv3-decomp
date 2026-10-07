@ GT Advance 3 — EWRAM serial handlers (MultiSio) copied to 0x0203EE70
@ ROM sources: _08000AF4 (288 B, file 0xAF4) and _08000E80 (288 B, file 0xE80)
@ Each copied via CpuFastSet 0x04000048 = 72 words (288 B) by _080005C0 / _08000BF0
@ Handlers are Thumb ISRs for IntrMain slot0 combo Timer3+Serial (0xC0)
@ Stored at ROM but executed from EWRAM 0x0203EE70 via bx to 0x0203EE71 (Thumb)
@ gbadisasm cross-check: primary ISRs match objdump -M force-thumb exactly
@
@ Assembly include spans:
@   --converted 0x00AF4:0x00FA0:handlers.s   # whole span, one hole
@
@ The region is now fully reconstructed:
@   0x0AF4-0x0BF0  blob A — the 288-byte MultiSio burst ISR (copied to EWRAM)
@   0x0BF0-0x0CA0  sub_08000BF0 — serial installer / 32 KiB checksum + blob B
@                  copy.  Note it *starts inside* blob A's copy range: the
@                  CpuSet copy at 0x08000C24 takes 288 bytes from
@                  0x08000E80, while blob A's own copy (0x080005C0) takes
@                  0x08000AF4..0x08000C14, so 0x08000BF0..0x08000C14 is
@                  duplicated between the two images in the original ROM.
@   0x0CA0-0x0E80  sub_08000CA0 — block-B serial state stepper, 6-entry
@                  `mov pc` dispatch table at 0x08000CC0, ending in the
@                  shared 0x08000E72/0x08000E7A epilogue.
@   0x0E80-0x0F48  blob B — the 288-byte SIO-normal polling ISR
@   0x0F48-0x0FA0  secondary helper tail, a private literal pool, and the
@                  head of the 0x08000F84 handler (which continues into
@                  asm/code_fa0.s at 0x08000FA0 — the region split is a
@                  source-organisation boundary, not a function boundary).

.syntax unified
.cpu arm7tdmi
.thumb

	.type _08000AF4, %function
@ Blob A — _08000AF4 -> 0x0203EE70 (game-session MultiSio 32-bit burst)
_08000AF4:
	push {r4, r5, lr}
	sub sp, #8
	ldr r0, _08000BD4 @ =0x04000120
	ldr r1, [r0, #4]
	ldr r0, [r0]
	str r0, [sp]
	str r1, [sp, #4]
	ldr r2, _08000BD8 @ =0x0203EF90
	ldr r0, _08000BDC @ =0x04000128
	ldr r0, [r0]
	lsls r0, r0, #0x19
	lsrs r0, r0, #0x1f
	movs r5, #0
	strb r0, [r2, #7]
	mov r1, sp
	ldr r0, _08000BE0 @ =0x0000FEFE
	adds r4, r2, #0
	ldrh r1, [r1]
	cmp r1, r0
	bne _08000B54
	ldr r0, [r4, #0x18]
	cmp r0, #9
	ble _08000B54
	movs r0, #1
	rsbs r0, r0, #0
	str r0, [r4, #0x18]
	ldr r1, [r4, #0x28]
	ldr r0, [r4, #0x24]
	str r0, [r4, #0x28]
	str r1, [r4, #0x24]
	ldrb r0, [r4, #4]
	cmp r0, #0
	beq _08000B42
	ldr r1, [r4, #0x20]
	ldr r0, [r4, #0x1c]
	str r0, [r4, #0x20]
	str r1, [r4, #0x1c]
	strb r5, [r4, #4]
	str r5, [r4, #0x14]
_08000B42:
	ldr r3, _08000BE4 @ =0x04000208
	strh r5, [r3]
	ldr r2, _08000BE8 @ =0x03007FF8
	ldrh r0, [r2]
	movs r1, #0x80
	orrs r0, r1
	strh r0, [r2]
	movs r0, #1
	strh r0, [r3]
_08000B54:
	ldr r0, [r4, #0x14]
	cmp r0, #9
	bgt _08000B66
	ldr r2, _08000BDC @ =0x04000128
	ldr r1, [r4, #0x20]
	lsls r0, r0, #1
	adds r0, r0, r1
	ldrh r0, [r0]
	strh r0, [r2, #2]
_08000B66:
	ldr r0, [r4, #0x14]
	cmp r0, #0xa
	bgt _08000B70
	adds r0, #1
	str r0, [r4, #0x14]
_08000B70:
	ldr r0, [r4, #0x18]
	cmp r0, #0
	blt _08000B98
	ldr r1, [r4, #0x24]
	mov r2, sp
	lsls r0, r0, #1
	adds r1, r1, r0
	movs r3, #3
_08000B80:
	ldrh r0, [r2]
	strh r0, [r1]
	adds r2, #2
	adds r1, #0x18
	subs r3, #1
	cmp r3, #0
	bge _08000B80
	ldr r0, [r4, #0x18]
	cmp r0, #9
	bne _08000B98
	movs r0, #1
	strb r0, [r4, #5]
_08000B98:
	ldr r0, [r4, #0x18]
	cmp r0, #0xa
	bgt _08000BA2
	adds r0, #1
	str r0, [r4, #0x18]
_08000BA2:
	ldrb r2, [r4]
	cmp r2, #0
	beq _08000BAE
	ldr r1, _08000BEC @ =0x0400010E
	movs r0, #0
	strh r0, [r1]
_08000BAE:
	ldr r0, [r4, #0x14]
	cmp r0, #0xa
	bgt _08000BC8
	cmp r2, #0
	beq _08000BC8
	ldr r2, _08000BDC @ =0x04000128
	ldrh r0, [r2]
	movs r1, #0x80
	orrs r0, r1
	strh r0, [r2]
	ldr r1, _08000BEC @ =0x0400010E
	movs r0, #0xc0
	strh r0, [r1]
_08000BC8:
	movs r0, #1
	strb r0, [r4, #9]
	add sp, #8
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_08000BD4: .4byte 0x04000120
_08000BD8: .4byte 0x0203EF90
_08000BDC: .4byte 0x04000128
_08000BE0: .4byte 0x0000FEFE
_08000BE4: .4byte 0x04000208
_08000BE8: .4byte 0x03007FF8
_08000BEC: .4byte 0x0400010E
    .thumb
    .type sub_08000BF0, %function
sub_08000BF0:
_08000BF0:
	push	{r4, r5, r6, r7, lr}
	mov	r7, r8
	push	{r7}
	sub	sp, #4
	adds	r5, r0, #0
	adds	r7, r1, #0
	movs	r6, #0
	ldr r3, _08000C74
	strh	r6, [r3, #0]
	ldr r2, _08000C78
	ldrh	r1, [r2, #0]
	ldr r0, _08000C7C
	ands	r0, r1
	strh	r0, [r2, #0]
	movs	r0, #1
	mov	r8, r0
	strh	r0, [r3, #0]
	str	r6, [sp, #0]
	ldr r4, _08000C80
	ldr r2, _08000C84
	mov	r0, sp
	adds	r1, r4, #0
	bl 0x0802D974
	ldr r0, _08000C88
	ldr r1, _08000C8C
	ldr r2, _08000C90
	bl 0x0802D974
	ldr r1, _08000C94
	ldr r0, _08000C98
	str	r0, [r1, #0]
	str	r7, [r4, #4]
	movs	r0, #1
	negs	r0, r0
	str	r0, [r4, #8]
	cmp	r5, #0
	beq _08000C68
	ldr r0, _08000C9C
	str	r6, [r0, #0]
	mov	r2, r8
	strb	r2, [r4, #0]
	adds	r1, r7, #0
	movs	r2, #128
	lsls	r2, r2, #6
_08000C4A:
	ldmia	r1!, {r0}
	adds	r6, r6, r0
	subs	r2, #1
	cmp	r2, #0
	bne _08000C4A
	mvns	r0, r6
	str	r0, [r4, #12]
	ldr r1, _08000C94
	movs	r2, #128
	lsls	r2, r2, #5
	adds	r0, r2, #0
	strh	r0, [r1, #0]
	adds	r2, #1
	adds	r0, r2, #0
	strh	r0, [r1, #0]
_08000C68:
	add	sp, #4
	pop	{r3}
	mov	r8, r3
	pop	{r4, r5, r6, r7}
	pop	{r0}
	bx	r0
_08000C74: .4byte 0x04000208
_08000C78: .4byte 0x04000200
_08000C7C: .4byte 0x0000FF3F
_08000C80: .4byte 0x0203F150
_08000C84: .4byte 0x05000006
_08000C88: .4byte 0x08000E81
_08000C8C: .4byte 0x0203EE70
_08000C90: .4byte 0x04000048
_08000C94: .4byte 0x04000128
_08000C98: .4byte 0x00002003
_08000C9C: .4byte 0x0400010C

    .thumb
    .type sub_08000CA0, %function
sub_08000CA0:
_08000CA0:
	push	{r4, r5, r6, r7, lr}
	adds	r2, r0, #0
	ldr r0, _08000CBC
	ldrb	r1, [r0, #1]
	adds	r5, r0, #0
	cmp	r1, #4
	bls _08000CB0
	b _08000E72
_08000CB0:
	lsls	r0, r1, #2
	ldr r1, _08000CC0
	adds	r0, r0, r1
	ldr	r0, [r0, #0]
	mov	pc, r0
	movs	r0, r0
_08000CBC: .4byte 0x0203F150
_08000CC0: .4byte 0x08000CC4
_08000CC4: .4byte 0x08000CD8
_08000CC8: .4byte 0x08000CF0
_08000CCC: .4byte 0x08000D8C
_08000CD0: .4byte 0x08000DFE
_08000CD4: .4byte 0x08000E68
_08000CD8:
	ldr	r0, [r5, #0]
	ldr r1, _08000CEC
	ands	r0, r1
	cmp	r0, #0
	bne _08000CE4
	b _08000E72
_08000CE4:
	movs	r0, #1
	strb	r0, [r5, #1]
	b _08000E72
	movs	r0, r0
_08000CEC: .4byte 0x00FF00FF
_08000CF0:
	ldrb	r0, [r5, #0]
	cmp	r0, #1
	bne _08000D00
	ldrb	r2, [r5, #2]
	cmp	r2, #5
	bhi _08000CFE
	b _08000E72
_08000CFE:
	b _08000D0A
_08000D00:
	ldr r1, _08000D40
	movs	r3, #128
	lsls	r3, r3, #5
	adds	r0, r3, #0
	strh	r0, [r1, #0]
_08000D0A:
	ldr r0, _08000D44
	movs	r6, #0
	str	r6, [r0, #0]
	ldr r1, _08000D48
	movs	r0, #192
	strh	r0, [r1, #0]
	ldrb	r4, [r5, #0]
	cmp	r4, #1
	bne _08000D58
	ldr r2, _08000D40
	ldrh	r0, [r2, #0]
	movs	r1, #128
	orrs	r0, r1
	strh	r0, [r2, #0]
	ldr r1, _08000D4C
	ldr r0, _08000D50
	str	r0, [r1, #0]
	ldr r3, _08000D54
	strh	r6, [r3, #0]
	adds	r2, #216
	ldrh	r0, [r2, #0]
	movs	r1, #64
	orrs	r0, r1
	strh	r0, [r2, #0]
	strh	r4, [r3, #0]
	b _08000D78
	movs	r0, r0
_08000D40: .4byte 0x04000128
_08000D44: .4byte 0x04000120
_08000D48: .4byte 0x04000202
_08000D4C: .4byte 0x0400010C
_08000D50: .4byte 0x00C0F318
_08000D54: .4byte 0x04000208
_08000D58:
	ldr r2, _08000D84
	ldrh	r0, [r2, #0]
	movs	r3, #129
	lsls	r3, r3, #7
	adds	r1, r3, #0
	orrs	r0, r1
	strh	r0, [r2, #0]
	ldr r3, _08000D88
	strh	r6, [r3, #0]
	adds	r2, #216
	ldrh	r0, [r2, #0]
	movs	r1, #128
	orrs	r0, r1
	strh	r0, [r2, #0]
	movs	r0, #1
	strh	r0, [r3, #0]
_08000D78:
	movs	r0, #0
	strb	r0, [r5, #2]
	movs	r0, #2
	strb	r0, [r5, #1]
	b _08000E72
	movs	r0, r0
_08000D84: .4byte 0x04000128
_08000D88: .4byte 0x04000208
_08000D8C:
	ldr	r6, [r5, #8]
	adds	r4, r6, #0
	movs	r0, #128
	lsls	r0, r0, #6
	cmp	r6, r0
	ble _08000D9C
	adds	r4, r0, #0
	b _08000DA2
_08000D9C:
	cmp	r6, #0
	bge _08000DA2
	movs	r4, #0
_08000DA2:
	cmp	r2, #0
	beq _08000DA8
	str	r4, [r2, #0]
_08000DA8:
	ldrb	r0, [r5, #0]
	cmp	r0, #1
	beq _08000DEA
	ldr	r0, [r5, #20]
	cmp	r0, r4
	bge _08000DCE
	adds	r3, r5, #0
	ldr	r7, [r5, #4]
_08000DB8:
	ldr	r2, [r3, #20]
	lsls	r0, r2, #2
	adds	r0, r0, r7
	ldr	r1, [r3, #16]
	ldr	r0, [r0, #0]
	adds	r1, r1, r0
	str	r1, [r3, #16]
	adds	r2, #1
	str	r2, [r3, #20]
	cmp	r2, r4
	blt _08000DB8
_08000DCE:
	movs	r0, #128
	lsls	r0, r0, #6
	cmp	r6, r0
	ble _08000DF2
	ldr	r0, [r5, #12]
	ldr	r1, [r5, #16]
	adds	r0, r0, r1
	str	r0, [r5, #12]
	movs	r1, #1
	negs	r1, r1
	cmp	r0, r1
	bne _08000DEA
	movs	r0, #1
	strb	r0, [r5, #3]
_08000DEA:
	movs	r0, #128
	lsls	r0, r0, #6
	cmp	r6, r0
	bgt _08000DF8
_08000DF2:
	ldrb	r2, [r5, #2]
	cmp	r2, #140
	bne _08000E72
_08000DF8:
	movs	r0, #3
	strb	r0, [r5, #1]
	b _08000E72
_08000DFE:
	ldr r3, _08000E50
	movs	r4, #0
	strh	r4, [r3, #0]
	ldr r2, _08000E54
	ldrh	r1, [r2, #0]
	ldr r0, _08000E58
	ands	r0, r1
	strh	r0, [r2, #0]
	movs	r0, #1
	strh	r0, [r3, #0]
	ldr r1, _08000E5C
	movs	r3, #128
	lsls	r3, r3, #5
	adds	r0, r3, #0
	strh	r0, [r1, #0]
	movs	r0, #128
	lsls	r0, r0, #6
	str	r0, [r1, #0]
	adds	r0, #3
	str	r0, [r1, #0]
	subs	r2, #224
	movs	r0, #0
	movs	r1, #0
	str	r0, [r2, #0]
	str	r1, [r2, #4]
	ldrb	r0, [r5, #0]
	cmp	r0, #0
	beq _08000E3C
	ldr r1, _08000E60
	movs	r0, #0
	str	r0, [r1, #0]
_08000E3C:
	ldr r0, _08000E64
	movs	r1, #192
	strh	r1, [r0, #0]
	ldrb	r0, [r5, #0]
	cmp	r0, #0
	beq _08000E6E
	strb	r4, [r5, #2]
	movs	r0, #4
	strb	r0, [r5, #1]
	b _08000E72
_08000E50: .4byte 0x04000208
_08000E54: .4byte 0x04000200
_08000E58: .4byte 0x0000FF3F
_08000E5C: .4byte 0x04000128
_08000E60: .4byte 0x0400010C
_08000E64: .4byte 0x04000202
_08000E68:
	ldrb	r0, [r5, #2]
	cmp	r0, #2
	bls _08000E72
_08000E6E:
	movs	r0, #1
	b _08000E7A
_08000E72:
	ldrb	r0, [r5, #2]
	adds	r0, #1
	strb	r0, [r5, #2]
	movs	r0, #0
_08000E7A:
	pop	{r4, r5, r6, r7}
	pop	{r1}
	bx	r1

	.type _08000E80, %function
@ Blob B — _08000E80 -> 0x0203EE70 (mode 8/11 SIO-normal polling)
_08000E80:
	push {r4, r5, lr}
	ldr r2, _08000EA4 @ =0x04000120
	ldr r3, [r2]
	ldr r5, _08000EA8 @ =0x0203F150
	adds r4, r5, #0
	ldrb r0, [r5]
	cmp r0, #1
	beq _08000EB0
	ldr r0, _08000EAC @ =0x04000128
	ldrh r1, [r0]
	movs r2, #0x80
	orrs r1, r2
	strh r1, [r0]
	ldr r2, [r4, #8]
	cmp r2, #0
	bge _08000EFC
	b _08000EEA
	.align 2, 0
_08000EA4: .4byte 0x04000120
_08000EA8: .4byte 0x0203F150
_08000EAC: .4byte 0x04000128
_08000EB0:
	ldr r1, _08000EC4 @ =0x0400010E
	movs r0, #0
	strh r0, [r1]
	ldr r1, [r4, #8]
	cmp r1, #0
	bge _08000ECC
	ldr r0, _08000EC8 @ =0xFEFEFEFE
	str r0, [r2]
	b _08000F12
	.align 2, 0
_08000EC4: .4byte 0x0400010E
_08000EC8: .4byte 0xFEFEFEFE
_08000ECC:
	ldr r0, _08000EE0 @ =0x00001FFF
	cmp r1, r0
	bgt _08000EE4
	ldr r0, [r4, #4]
	lsls r1, r1, #2
	adds r1, r1, r0
	ldr r0, [r1]
	str r0, [r2]
	b _08000F12
	.align 2, 0
_08000EE0: .4byte 0x00001FFF
_08000EE4:
	ldr r0, [r4, #0xc]
	str r0, [r2]
	b _08000F12
_08000EEA:
	ldr r0, _08000EF8 @ =0xFEFEFEFE
	cmp r3, r0
	beq _08000F12
	subs r0, r2, #1
	str r0, [r5, #8]
	b _08000F12
	.align 2, 0
_08000EF8: .4byte 0xFEFEFEFE
_08000EFC:
	ldr r0, _08000F0C @ =0x00001FFF
	cmp r2, r0
_08000F00:
	bgt _08000F10
	ldr r1, [r4, #4]
	lsls r0, r2, #2
	adds r0, r0, r1
	str r3, [r0]
	b _08000F12
	.align 2, 0
_08000F0C: .4byte 0x00001FFF
_08000F10:
	str r3, [r4, #0xc]
_08000F12:
	ldr r1, [r4, #8]
	ldr r0, _08000F3C @ =0x00002002
	cmp r1, r0
	bgt _08000F34
	adds r0, r1, #1
	str r0, [r4, #8]
	ldrb r4, [r4]
	cmp r4, #1
	bne _08000F34
	ldr r2, _08000F40 @ =0x04000128
	ldrh r0, [r2]
	movs r1, #0x80
	orrs r0, r1
	strh r0, [r2]
	ldr r1, _08000F44 @ =0x0400010E
	movs r0, #0xc0
	strh r0, [r1]
_08000F34:
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_08000F3C: .4byte 0x00002002
_08000F40: .4byte 0x04000128
_08000F44: .4byte 0x0400010E

    @ 0xF48-0xFA0: secondary SIO helper tail, its private literal pool, and
    @ the head of the 0x08000F84 handler (continues at 0x08000FA0 in
    @ asm/code_fa0.s).
    .type sub_08000F48, %function
sub_08000F48:
_08000F48:
	adds	r2, r0, #0
	movs	r1, #0
	strb	r1, [r2, #30]
	strb	r1, [r2, #24]
	strb	r1, [r2, #29]
	adds	r3, r2, #0
	adds	r3, #74
	movs	r0, #15
	strb	r0, [r3, #0]
	adds	r0, r2, #0
	adds	r0, #72
	strb	r1, [r0, #0]
	strh	r1, [r2, #22]
	ldr r0, _08000F74
	strh	r1, [r0, #0]
	ldr r2, _08000F78
	ldr r3, _08000F7C
	adds	r0, r3, #0
	strh	r0, [r2, #0]
	ldr r0, _08000F80
	strh	r1, [r0, #0]
	bx	lr
_08000F74: .4byte 0x04000134
_08000F78: .4byte 0x04000128
_08000F7C: .4byte 0x00002003
_08000F80: .4byte 0x0400012A
.type _08000F84, %function
_08000F84:
	push	{r4, r5, r6, r7, lr}
	mov	r7, sl
	mov	r6, r9
	mov	r5, r8
	push	{r5, r6, r7}
	adds	r6, r0, #0
	bl 0x080014A4
	cmp	r0, #0
	beq _08000F9A
	b 0x08001360
_08000F9A:
	adds	r0, r6, #0
	adds	r0, #74
	ldrb	r1, [r0, #0]

@ End of handlers.s — verified to assemble byte-identical via arm-none-eabi-as -mcpu=arm7tdmi
