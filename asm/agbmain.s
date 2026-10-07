@ GT Advance 3 - AgbMain + boot-adjacent helpers
@ Region: file offset 0x0002C4-0x000A60 (VMA 0x080002C4-0x08000A60)
@
@ Disassembled by gbadisasm from baserom.gba; verified byte-identical.
@ BL targets outside
@ this region are resolved by markers in asm/passthrough.inc.

.thumb

	.type _080002C4, %function
_080002C4:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, sb
	mov r5, r8
	push {r5, r6, r7}
	sub sp, #8
	ldr r0, _080003B0 @ =0x087B04C4
	cmp r0, #0
	bne _080002D8
	b _08000580
_080002D8:
	ldr r1, _080003B4 @ =0x04000204
	ldr r2, _080003B8 @ =0x00004014
	adds r0, r2, #0
	strh r0, [r1]
	movs r2, #0
	str r2, [sp]
	ldr r0, _080003BC @ =0x040000D4
	mov r3, sp
	str r3, [r0]
	movs r1, #0x80
	lsls r1, r1, #0x12
	str r1, [r0, #4]
	ldr r1, _080003C0 @ =0x85010000
	str r1, [r0, #8]
	ldr r1, [r0, #8]
	str r2, [sp]
	str r3, [r0]
	movs r1, #0xc0
	lsls r1, r1, #0x12
	str r1, [r0, #4]
	ldr r1, _080003C4 @ =0x85001F80
	str r1, [r0, #8]
	ldr r1, [r0, #8]
	str r2, [sp]
	str r3, [r0]
	movs r1, #0xe0
	lsls r1, r1, #0x13
	str r1, [r0, #4]
	ldr r1, _080003C8 @ =0x85000100
	str r1, [r0, #8]
	ldr r1, [r0, #8]
	add r1, sp, #4
	strh r2, [r1]
	str r1, [r0]
	movs r1, #0xa0
	lsls r1, r1, #0x13
	str r1, [r0, #4]
	ldr r1, _080003CC @ =0x81000200
	str r1, [r0, #8]
	ldr r0, [r0, #8]
	ldr r0, _080003D0 @ =0x0203EE50
	bl sub_0802B04C
	bl _08000290
	ldr r0, _080003D4 @ =0x030035D0
	movs r1, #0xc0
	lsls r1, r1, #6
	bl sub_08004A2C
	ldr r0, _080003D8 @ =0x080CB298
	bl sub_08004AF4
	bl sub_08005A58
	bl sub_08002694
	movs r0, #1
	bl _08000590
	bl sub_08002AAC
	ldr r1, _080003DC @ =0x03001780
	ldr r0, _080003E0 @ =0x00000FBC
	adds r2, r1, r0
	movs r0, #1
	strh r0, [r2]
	ldr r3, _080003E4 @ =0x000010C8
	adds r2, r1, r3
	movs r0, #3
	strh r0, [r2]
	ldr r7, _080003E8 @ =0x04000208
	adds r5, r1, #0
	ldr r0, _080003EC @ =0x000010D8
	adds r6, r5, r0
	ldr r1, _080003F0 @ =0x000010B8
	adds r1, r1, r5
	mov sl, r1
	movs r2, #0
	mov sb, r2
_08000378:
	movs r3, #0
	mov r8, r3
	bl sub_0802D9B0
	bl _080016EC
	bl _080020F4
	movs r0, #2
	bl _080020E8
	bl sub_08005A68
	bl _0800210C
	movs r0, #1
	movs r1, #0
	movs r2, #0
	bl sub_08004D4C
	movs r0, #3
	movs r1, #0
	movs r2, #0
	bl sub_08004D4C
	movs r0, #1
	strh r0, [r7]
	b _08000510
	.align 2, 0
_080003B0: .4byte 0x087B04C4
_080003B4: .4byte 0x04000204
_080003B8: .4byte 0x00004014
_080003BC: .4byte 0x040000D4
_080003C0: .4byte 0x85010000
_080003C4: .4byte 0x85001F80
_080003C8: .4byte 0x85000100
_080003CC: .4byte 0x81000200
_080003D0: .4byte 0x0203EE50
_080003D4: .4byte 0x030035D0
_080003D8: .4byte 0x080CB298
_080003DC: .4byte 0x03001780
_080003E0: .4byte 0x00000FBC
_080003E4: .4byte 0x000010C8
_080003E8: .4byte 0x04000208
_080003EC: .4byte 0x000010D8
_080003F0: .4byte 0x000010B8
_080003F4:
	bl _08002430
	bl _08001834
	movs r0, #4
	movs r1, #0
	movs r2, #0
	bl sub_08004D4C
	bl sub_08004E6C
	cmp r0, #1
	beq _08000414
	cmp r0, #2
	beq _08000424
	b _08000432
_08000414:
	bl sub_08004B68
	ldrh r1, [r0, #0xa]
	movs r0, #0xf
	movs r2, #0
	bl sub_08004D4C
	b _08000432
_08000424:
	bl sub_08004B68
	ldrh r1, [r0, #0xa]
	movs r0, #0x10
	movs r2, #0
	bl sub_08004D4C
_08000432:
	bl _08001B7C
	bl sub_08004B68
	bl sub_08004B74
	cmp r0, #0
	beq _0800045E
	bl _08002494
	adds r4, r0, #0
	lsls r4, r4, #0x10
	lsrs r4, r4, #0x10
	bl _08002488
	adds r2, r0, #0
	lsls r2, r2, #0x10
	lsrs r2, r2, #0x10
	movs r0, #6
	adds r1, r4, #0
	bl sub_08004D4C
_0800045E:
	bl sub_08005AE4
	bl _080021EC
	movs r0, #5
	movs r1, #0
	movs r2, #0
	bl sub_08004D4C
	bl _08001C48
	movs r0, #7
	movs r1, #0
	movs r2, #0
	bl sub_08004D4C
	bl sub_0802D9B0
	bl _08001CD8
	movs r0, #8
	movs r1, #0
	movs r2, #0
	bl sub_08004D4C
	bl sub_08004E6C
	cmp r0, #1
	beq _0800049E
	cmp r0, #2
	beq _080004AE
	b _080004BC
_0800049E:
	bl sub_08004B68
	ldrh r1, [r0, #0xa]
	movs r0, #0x11
	movs r2, #0
	bl sub_08004D4C
	b _080004BC
_080004AE:
	bl sub_08004B68
	ldrh r1, [r0, #0xa]
	movs r0, #0x12
	movs r2, #0
	bl sub_08004D4C
_080004BC:
	ldr r2, [r6]
	adds r2, #1
	str r2, [r6]
	movs r0, #1
	ands r0, r2
	ldr r3, _08000570 @ =0x000010B0
	adds r1, r5, r3
	strh r0, [r1]
	movs r0, #2
	ands r0, r2
	adds r3, #2
	adds r1, r5, r3
	strh r0, [r1]
	movs r0, #4
	ands r0, r2
	adds r3, #2
	adds r1, r5, r3
	strh r0, [r1]
	movs r0, #8
	ands r0, r2
	adds r3, #2
	adds r1, r5, r3
	strh r0, [r1]
	movs r0, #0x10
	ands r0, r2
	mov r1, sl
	strh r0, [r1]
	cmp r2, #0x20
	bls _08000510
	bl _08002488
	movs r4, #0xf
	adds r1, r4, #0
	ands r1, r0
	cmp r1, #0
	beq _08000510
	bl _08002494
	adds r1, r4, #0
	ands r1, r0
	cmp r1, #0xf
	beq _08000520
_08000510:
	bl sub_08004B9C
	cmp r0, #0
	bne _0800051A
	b _080003F4
_0800051A:
	mov r2, r8
	cmp r2, #0
	beq _0800054A
_08000520:
	mov r3, sb
	strh r3, [r7]
	ldr r2, _08000574 @ =0x04000200
	ldrh r0, [r2]
	ldr r3, _08000578 @ =0x0000FF7F
	adds r1, r3, #0
	ands r0, r1
	strh r0, [r2]
	ldrh r0, [r2]
	adds r3, #0x40
	adds r1, r3, #0
	ands r0, r1
	strh r0, [r2]
	movs r0, #1
	strh r0, [r7]
	ldr r0, _0800057C @ =0x03007FFA
	mov r1, sb
	strb r1, [r0]
	movs r0, #0xff
	bl sub_0802D994
_0800054A:
	bl sub_08004E6C
	cmp r0, #2
	bne _08000560
	bl sub_08004B68
	ldrh r1, [r0, #0xa]
	movs r0, #0x14
	movs r2, #0
	bl sub_08004D4C
_08000560:
	movs r0, #0xc
	movs r1, #0
	movs r2, #0
	bl sub_08004D4C
	bl sub_08004B50
	b _08000378
	.align 2, 0
_08000570: .4byte 0x000010B0
_08000574: .4byte 0x04000200
_08000578: .4byte 0x0000FF7F
_0800057C: .4byte 0x03007FFA
_08000580:
	add sp, #8
	pop {r3, r4, r5}
	mov r8, r3
	mov sb, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
_08000590:
	push {r4, r5, lr}
	sub sp, #4
	adds r4, r0, #0
	movs r0, #0
	str r0, [sp]
	ldr r5, _080005B8 @ =0x0203EE64
	ldr r2, _080005BC @ =0x05000001
	mov r0, sp
	adds r1, r5, #0
	bl sub_0802D974
	cmp r4, #0
	bne _080005AE
	movs r0, #1
	strb r0, [r5]
_080005AE:
	add sp, #4
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_080005B8: .4byte 0x0203EE64
_080005BC: .4byte 0x05000001
_080005C0:
	push {r4, r5, r6, r7, lr}
	mov r7, sb
	mov r6, r8
	push {r6, r7}
	sub sp, #4
	adds r4, r0, #0
	ldr r6, _0800066C @ =0x04000208
	movs r2, #0
	strh r2, [r6]
	ldr r5, _08000670 @ =0x04000200
	ldrh r1, [r5]
	ldr r0, _08000674 @ =0x0000FF3F
	ands r0, r1
	strh r0, [r5]
	movs r0, #1
	mov sb, r0
	strh r0, [r6]
	ldr r0, _08000678 @ =0x04000134
	strh r2, [r0]
	ldr r2, _0800067C @ =0x04000128
	movs r1, #0x80
	lsls r1, r1, #6
	adds r0, r1, #0
	strh r0, [r2]
	ldrh r0, [r2]
	ldr r3, _08000680 @ =0x00004003
	adds r1, r3, #0
	orrs r0, r1
	strh r0, [r2]
	movs r0, #0
	mov r8, r0
	str r0, [sp]
	ldr r7, _08000684 @ =0x0203EF90
	ldr r2, _08000688 @ =0x05000060
	mov r0, sp
	adds r1, r7, #0
	bl sub_0802D974
	ldr r0, _0800068C @ =_0800021C
	ldr r1, _08000690 @ =0x0203F110
	ldr r2, _08000694 @ =0x04000010
	bl sub_0802D974
	ldr r0, _08000698 @ =0x08000AF5
	ldr r1, _0800069C @ =0x0203EE70
	ldr r2, _080006A0 @ =0x04000048
	bl sub_0802D974
	lsls r4, r4, #4
	movs r0, #0xf
	ldrb r1, [r7, #2]
	ands r0, r1
	orrs r0, r4
	strb r0, [r7, #2]
	movs r0, #0xc
	str r0, [r7, #0x14]
	str r0, [r7, #0x18]
	adds r0, r7, #0
	adds r0, #0x30
	str r0, [r7, #0x1c]
	adds r0, #0x18
	str r0, [r7, #0x20]
	adds r0, #0x18
	str r0, [r7, #0x24]
	adds r0, #0x60
	str r0, [r7, #0x28]
	movs r3, #0x90
	lsls r3, r3, #1
	adds r0, r7, r3
	str r0, [r7, #0x2c]
	mov r0, r8
	strh r0, [r6]
	ldrh r0, [r5]
	movs r1, #0x80
	orrs r0, r1
	strh r0, [r5]
	mov r1, sb
	strh r1, [r6]
	add sp, #4
	pop {r3, r4}
	mov r8, r3
	mov sb, r4
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0
_0800066C: .4byte 0x04000208
_08000670: .4byte 0x04000200
_08000674: .4byte 0x0000FF3F
_08000678: .4byte 0x04000134
_0800067C: .4byte 0x04000128
_08000680: .4byte 0x00004003
_08000684: .4byte 0x0203EF90
_08000688: .4byte 0x05000060
_0800068C: .4byte _0800021C
_08000690: .4byte 0x0203F110
_08000694: .4byte 0x04000010
_08000698: .4byte 0x08000AF5
_0800069C: .4byte 0x0203EE70
_080006A0: .4byte 0x04000048
_080006A4:
	ldr r1, _080006B8 @ =0x0203EF90
	ldrb r0, [r1]
	cmp r0, #0
	beq.n _080006B4
	movs r0, #0x80
	ldrb r2, [r1, #6]
	orrs r0, r2
	strb r0, [r1, #6]
_080006B4:
	bx lr
	movs r0, r0
	.align 2, 0
_080006B8: .4byte 0x0203EF90
_080006BC:
	ldr r3, _080006F0 @ =0x04000208
	movs r0, #0
	strh r0, [r3]
	ldr r2, _080006F4 @ =0x04000200
	ldrh r1, [r2]
	ldr r0, _080006F8 @ =0x0000FF3F
	ands r0, r1
	strh r0, [r2]
	movs r0, #1
	strh r0, [r3]
	ldr r1, _080006FC @ =0x04000128
	ldr r2, _08000700 @ =0x00002003
	adds r0, r2, #0
	strh r0, [r1]
	subs r1, #0x1c
	ldr r0, _08000704 @ =0x0000ABFB
	str r0, [r1]
	adds r1, #0xf6
	movs r0, #0xc0
	strh r0, [r1]
	ldr r1, _08000708 @ =0x0203EF90
	movs r0, #0x7f
	ldrb r2, [r1, #6]
	ands r0, r2
	strb r0, [r1, #6]
	bx lr
	.align 2, 0
_080006F0: .4byte 0x04000208
_080006F4: .4byte 0x04000200
_080006F8: .4byte 0x0000FF3F
_080006FC: .4byte 0x04000128
_08000700: .4byte 0x00002003
_08000704: .4byte 0x0000ABFB
_08000708: .4byte 0x0203EF90
@ ----------------------------------------------------------------------------
@ _0800070C(buf) -> u16 — session-word reader (called from idle path
@ every frame with st+0x30). Samples SIOCNT once at entry; switches on
@ the soft-IRQ state machine byte at 0x0203EF90+1:
@   state 0: if link lines idle and [+0x14]==12, arm the serial timer
@            packet (IME dance, IE bit6/Timer3 reload 0xABFB) and
@            advance state; else just set state 1.
@   state 1: watchdog — bump retry counter +8 up to 7, then state 2.
@   state 2 / default: verify via _08008F4.
@ Finally composes the status halfword from state bytes (+2/+3/+6/+7/+8)
@ plus sampled SIOCNT bits, and returns it.
_0800070C:
	push {r4, r5, r6, r7, lr}
	mov ip, r0
	ldr r6, _08000728 @ =0x04000128
	ldr r7, [r6]
	ldr r0, _0800072C @ =0x0203EF90
	ldrb r1, [r0, #1]
	adds r5, r0, #0
	cmp r1, #1
	beq.n _08000794
	cmp r1, #1
	bgt.n _08000730
	cmp r1, #0
	beq.n _08000736
	b.n _080007DA
	.align 2, 0
_08000728: .4byte 0x04000128
_0800072C: .4byte 0x0203EF90
_08000730:
	cmp r1, #2
	beq.n _080007D4
	b.n _080007DA
_08000736:
	movs r1, #0x30
	adds r0, r7, #0
	ands r0, r1
	cmp r0, #0
	bne.n _08000790
	movs r0, #0x88
	adds r4, r7, #0
	ands r4, r0
	cmp r4, #8
	bne.n _080007DA
	movs r1, #4
	adds r0, r7, #0
	ands r0, r1
	lsls r0, r0, #24
	lsrs r1, r0, #24
	cmp r1, #0
	bne.n _08000790
	ldr r0, [r5, #0x14]
	cmp r0, #12
	bne.n _08000790
	ldr r3, _080007B8 @ =0x04000208
	strh r1, [r3, #0]          @ IME = 0 (r1 is known zero here)
	ldr r2, _080007BC @ =0x04000200
	ldrh r1, [r2, #0]
	ldr r0, _080007C0 @ =0x0000FF7F
	ands r0, r1
	strh r0, [r2, #0]
	ldrh r0, [r2, #0]
	movs r1, #0x40
	orrs r0, r1
	strh r0, [r2, #0]          @ IE: clear bit7, set bit6 (serial)
	movs r0, #1
	strh r0, [r3, #0]          @ IME = 1
	ldrb r1, [r6, #1]
	movs r0, #0x41
	negs r0, r0
	ands r0, r1
	strb r0, [r6, #1]          @ SIOCNT.H &= ~0x40
	ldr r1, _080007C4 @ =0x0400010C
	ldr r0, _080007C8 @ =0x0000ABFB
	str r0, [r1, #0]           @ TM3CNT_L reload = 0xABFB
	adds r1, #0xf6             @ -> 0x04000202 = IE
	movs r0, #0xc0
	strh r0, [r1, #0]          @ IE = Timer3 | Serial
	strb r4, [r5, #0]          @ record sampled SIOCNT & 0x88
_08000790:
	movs r0, #1
	strb r0, [r5, #1]
_08000794:
	ldr r1, _080007CC @ =0x0203EF90
	movs r0, #0xf0
	ldrb r2, [r1, #2]
	ands r0, r2
	cmp r0, #0
	beq.n _080007D4
	movs r0, #0x40
	ldrb r2, [r1, #6]
	ands r0, r2
	cmp r0, #0
	bne.n _080007D4
	ldrb r0, [r1, #8]
	cmp r0, #7
	bhi.n _080007D0
	adds r0, #1
	strb r0, [r1, #8]
	b.n _080007D4
	movs r0, r0
	.align 2, 0
_080007B8: .4byte 0x04000208
_080007BC: .4byte 0x04000200
_080007C0: .4byte 0x0000FF7F
_080007C4: .4byte 0x0400010C
_080007C8: .4byte 0x0000ABFB
_080007CC: .4byte 0x0203EF90
_080007D0:
	movs r0, #2
	strb r0, [r1, #1]
_080007D4:
	mov r0, ip
	bl sub_08008F4
_080007DA:
	ldr r2, _08000810 @ =0x0203EF90
	ldrb r0, [r2, #11]
	adds r0, #1
	strb r0, [r2, #11]
	ldrb r1, [r2, #6]
	movs r3, #16
	ands r3, r1
	ldrb r0, [r2, #3]
	orrs r3, r0
	movs r0, #32
	ands r0, r1
	orrs r3, r0
	movs r0, #0x40
	ands r0, r1
	orrs r3, r0
	ldrb r1, [r2, #2]
	lsrs r0, r1, #4
	lsls r0, r0, #8
	adds r5, r2, #0
	ldrb r2, [r5, #0]
	cmp r2, #8
	bne.n _08000814
	movs r1, #0x80
	orrs r1, r0
	orrs r1, r3
	b.n _08000818
	movs r0, r0
	.align 2, 0
_08000810: .4byte 0x0203EF90
_08000814:
	adds r1, r3, #0
	orrs r1, r0
_08000818:
	ldrb r0, [r5, #7]
	cmp r0, #0
	beq.n _08000824
	movs r0, #0x80
	lsls r0, r0, #5
	orrs r1, r0
_08000824:
	ldrb r5, [r5, #8]
	lsrs r0, r5, #3
	lsls r2, r0, #15
	lsls r0, r7, #26
	lsrs r0, r0, #30
	cmp r0, #3
	bls.n _0800083C
	movs r0, #0x80
	lsls r0, r0, #6
	orrs r0, r2
	orrs r0, r1
	b.n _08000840
_0800083C:
	adds r0, r1, #0
	orrs r0, r2
_08000840:
	pop {r4, r5, r6, r7}
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08000848(payload) — build a 10-halfword packet in [[0x0203EF90]+28]:
@ merge flag bits into header bytes (+0/+1) from the caller's arg bit0,
@ 0x0203F150 byte3 bit0, EF90 byte6 bit7, and nibble-xor of EF90
@ bytes 2/3; zero the checksum slot; CpuFastSet-copy 16 payload bytes
@ to packet+4; checksum = ~(sum of first 10 halfwords) - 12; raise the
@ "packet ready" byte at EF90+4. 🔎 link-session/serial packet builder.
_08000848:
	push {r4, r5, r6, r7, lr}
	movs r6, #0
	ldr r4, _080008E8 @ =0x0203EF90
	ldr r3, [r4, #0x1c]
	movs r2, #1
	ands r1, r2
	lsls r1, r1, #4
	movs r2, #17
	negs r2, r2
	ldrb r5, [r3, #1]
	ands r2, r5
	orrs r2, r1
	strb r2, [r3, #1]
	ldr r5, [r4, #0x1c]
	ldr r1, _080008EC @ =0x0203F150
	movs r3, #1
	adds r2, r3, #0
	ldrb r1, [r1, #3]
	ands r2, r1
	lsls r2, r2, #5
	movs r1, #0x21
	negs r1, r1
	ldrb r7, [r5, #1]
	ands r1, r7
	orrs r1, r2
	strb r1, [r5, #1]
	ldr r2, [r4, #0x1c]
	ldrb r5, [r4, #6]
	lsls r1, r5, #25
	lsrs r1, r1, #31
	ands r3, r1
	lsls r3, r3, #6
	movs r1, #0x41
	negs r1, r1
	ldrb r7, [r2, #1]
	ands r1, r7
	orrs r1, r3
	strb r1, [r2, #1]
	ldr r2, [r4, #0x1c]
	ldrb r1, [r4, #11]
	strb r1, [r2, #0]
	ldr r3, [r4, #0x1c]
	ldrb r2, [r4, #2]
	lsls r1, r2, #28
	lsrs r1, r1, #28
	ldrb r2, [r4, #3]
	eors r2, r1
	movs r1, #15
	ands r2, r1
	movs r1, #16
	negs r1, r1
	ldrb r5, [r3, #1]
	ands r1, r5
	orrs r1, r2
	strb r1, [r3, #1]
	ldr r1, [r4, #0x1c]
	strh r6, [r1, #2]
	ldr r1, [r4, #0x1c]
	adds r1, #4
	ldr r2, _080008F0 @ =0x04000004
	bl sub_0802D974            @ copy 4 words of payload (r0 = arg)
	movs r1, #0
	ldr r0, [r4, #0x1c]
_080008C8:
	ldrh r7, [r0]
	adds r6, r7, r6
	adds r0, #2
	adds r1, #1
	cmp r1, #9
	bls.n _080008C8
	ldr r0, [r4, #0x1c]
	mvns r1, r6
	subs r1, #12
	strh r1, [r0, #2]
	movs r0, #1
	strb r0, [r4, #4]
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_080008E8: .4byte 0x0203EF90
_080008EC: .4byte 0x0203F150
_080008F0: .4byte 0x04000004

@ ----------------------------------------------------------------------------
@ sub_08008F4(buf) — verify/receive side of the packet protocol.
@ For records 3..0 at [[0x0203EF90]+0x2C] (stride 24): sum 10 halfwords;
@ when the sum is -13 the record is marked valid (bit r3 in EF90+3),
@ its id nibble merged into EF90+6, and 16 bytes from buf+r3*16 are
@ filled into record+4 before the area is re-zeroed. Tail rebuilds the
@ EF90+2 status byte from accumulated flags. Called by _080007D4 above
@ (state 2/default). Uses sub_0802DDC8 (unconverted) for the initial
@ handshake probe.
sub_08008F4:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	sub sp, #12
	mov r9, r0
	ldr r0, _08000A14 @ =0x0203F110
	movs r4, #0
	str r4, [sp]
	bl sub_0802DDC8
	lsls r0, r0, #24
	ldr r2, _08000A18 @ =0x0203EF90
	strb r4, [r2, #3]
	movs r1, #0x41
	negs r1, r1
	ldrb r3, [r2, #6]
	ands r1, r3
	strb r1, [r2, #6]
	cmp r0, #0
	beq.n _080009A2
	movs r3, #3
	add r4, sp, #4
	mov r8, r4
	adds r5, r2, #0
	movs r6, #13
	negs r6, r6
	mov sl, r6
_0800092E:
	lsls r0, r3, #1
	adds r0, r0, r3
	lsls r0, r0, #3
	ldr r1, [r5, #0x2c]
	adds r7, r1, r0
	movs r0, #0
	movs r2, #0
	subs r1, r3, #1
	str r1, [sp, #8]
	adds r1, r7, #0
_08000942:
	ldrh r4, [r1, #0]
	adds r0, r4, r0
	adds r1, #2
	adds r2, #1
	cmp r2, #9
	bls.n _08000942
	lsls r0, r0, #16
	asrs r0, r0, #16
	adds r4, r7, #4
	cmp r0, sl
	bne.n _0800098E
	movs r0, #1
	lsls r0, r3
	ldrb r6, [r5, #3]
	orrs r0, r6
	strb r0, [r5, #3]
	ldrb r2, [r5, #6]
	lsls r1, r2, #28
	lsrs r1, r1, #28
	ldrb r6, [r7, #1]
	lsls r0, r6, #26
	lsrs r0, r0, #31
	lsls r0, r3
	orrs r0, r1
	movs r1, #15
	ands r0, r1
	movs r6, #16
	negs r6, r6
	adds r1, r6, #0
	ands r2, r1
	orrs r2, r0
	strb r2, [r5, #6]
	lsls r1, r3, #4
	add r1, r9
	adds r0, r4, #0
	ldr r2, _08000A1C @ =0x04000004
	bl sub_0802D974
_0800098E:
	movs r0, #0
	str r0, [sp, #4]
	mov r0, r8
	adds r1, r4, #0
	ldr r2, _08000A20 @ =0x05000004
	bl sub_0802D974
	ldr r3, [sp, #8]
	cmp r3, #0
	bge.n _0800092E
_080009A2:
	ldr r3, _08000A18 @ =0x0203EF90
	ldrb r2, [r3, #2]
	lsls r0, r2, #28
	lsrs r0, r0, #28
	ldrb r6, [r3, #3]
	orrs r0, r6
	movs r1, #15
	ands r0, r1
	movs r1, #16
	negs r1, r1
	ands r1, r2
	orrs r1, r0
	lsrs r2, r1, #4
	lsls r0, r1, #28
	lsrs r0, r0, #28
	orrs r2, r0
	lsls r2, r2, #4
	movs r4, #15
	ands r4, r1
	orrs r4, r2
	strb r4, [r3, #2]
	movs r1, #1
	adds r0, r1, #0
	ands r0, r6
	adds r5, r3, #0
	cmp r0, #0
	beq.n _08000A4E
	ldrb r0, [r5, #0]
	cmp r0, #8
	bne.n _08000A24
	movs r0, #3
	ands r0, r6
	cmp r0, #0
	beq.n _080009F8
	lsls r0, r6, #24
	lsrs r0, r0, #24
	lsrs r1, r4, #4
	cmp r0, r1
	bne.n _080009F8
	movs r0, #16
	ldrb r1, [r5, #6]
	orrs r0, r1
	strb r0, [r5, #6]
_080009F8:
	ldrb r4, [r5, #6]
	lsls r2, r4, #28
	lsrs r2, r2, #28
	movs r0, #14
	ldrb r6, [r5, #2]
	lsrs r3, r6, #4
	adds r1, r0, #0
	ands r1, r2
	ands r0, r3
	cmp r1, r0
	bne.n _08000A3A
	movs r0, #0x40
	orrs r0, r4
	b.n _08000A38
	.align 2, 0
_08000A14: .4byte 0x0203F110
_08000A18: .4byte 0x0203EF90
_08000A1C: .4byte 0x04000004
_08000A20: .4byte 0x05000004
_08000A24:
	ldrb r2, [r7, #1]
	lsls r0, r2, #25
	lsrs r0, r0, #31
	ands r1, r0
	lsls r1, r1, #6
	movs r0, #0x41
	negs r0, r0
	ldrb r3, [r5, #6]
	ands r0, r3
	orrs r0, r1
_08000A38:
	strb r0, [r5, #6]
_08000A3A:
	ldrb r7, [r7, #1]
	lsls r1, r7, #27
	lsrs r1, r1, #31
	lsls r1, r1, #5
	movs r0, #0x21
	negs r0, r0
	ldrb r4, [r5, #6]
	ands r0, r4
	orrs r0, r1
	strb r0, [r5, #6]
_08000A4E:
	ldrb r0, [r5, #3]
	add sp, #12
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r1}
	bx r1
