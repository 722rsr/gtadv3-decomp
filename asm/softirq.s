@ GT Advance 3 - VBlank-side soft-IRQ / link-session transmit kicker
@ Region: file offset 0x000A60-0x000AF4 (VMA 0x08000A60-0x08000AF4)
@
@ Disassembled via objdump from baserom.gba; verified byte-identical.
@ Companions: asm/agbmain.s,
@             asm/agbmain.s
@
@ Callers: 0x08000268 VBlank IRQ handler (bl 0x0800026A) and wrapper
@ _08002388 in the unconverted pocket at 0x2254-0x2430 (bl 0x0800238A).
@
@ Operates on EWRAM 0x0203EF90 (link-session state, see asm/agbmain.s):
@   +0x00 u8  pending flag
@   +0x01 u8  FSM state (0 = idle -> nothing to do here)
@   +0x04 u8  packet-ready, raised by builder _08000848
@   +0x06 u8  merged flag bits (bit7 raised by main-loop pump _080006A4)
@   +0x07 u8  snapshot of SIOCNT bit6 (serial IRQ-enable) at kick time
@   +0x09 u8  idle-path latch (kept 0 while pending==0)
@   +0x14 u32 arming counter: enter hook _080005C0 seeds 12, reader
@             _0800070C arms only when ==12, cleared here on packet send
@   +0x18 u32 enter hook seeds 12; set to -1 here on every kick
@   +0x1C/+0x20 u32 ptr pair {EF90+0x30, EF90+0x48}, swapped when a
@             queued packet is transmitted (ping-pong buffers)
@   +0x24/+0x28 u32 ptr pair {EF90+0x60, EF90+0xC0}, swapped on every
@             kick (ping-pong buffers)

.thumb

sub_08000A60:
	push {r4, r5, lr}
	ldr r4, _08000ABC @ =0x0203EF90
	ldrb r5, [r4]
	cmp r5, #0
	beq _08000ACC                @ nothing pending -> idle dance below
	ldrb r0, [r4, #1]            @ session FSM state
	cmp r0, #0
	beq _08000AE6                @ state 0 -> return
	movs r0, #0x80
	ldrb r1, [r4, #6]
	ands r0, r1                  @ merged flags bit7?
	cmp r0, #0
	beq _08000AE6                @ pump _080006A4 has not raised it
	movs r0, #1
	negs r0, r0
	str r0, [r4, #0x18]          @ +0x18 = -1 (kick marker / no-timeout)
	ldr r1, [r4, #0x28]
	ldr r0, [r4, #0x24]
	str r0, [r4, #0x28]          @ swap +0x24 <-> +0x28
	str r1, [r4, #0x24]
	ldrb r0, [r4, #4]            @ queued packet?
	cmp r0, #0
	beq _08000A9C
	ldr r1, [r4, #0x20]
	ldr r0, [r4, #0x1C]
	str r0, [r4, #0x20]          @ swap +0x1C <-> +0x20
	str r1, [r4, #0x1C]
	movs r0, #0
	strb r0, [r4, #4]            @ consume packet-ready
	str r0, [r4, #0x14]          @ reset arming counter (was 12)
_08000A9C:
	ldr r2, _08000AC0 @ =0x04000128       @ SIOCNT
	ldr r0, [r2]                 @ word {SIOCNT, SIODATA8}
	lsls r0, r0, #25
	lsrs r0, r0, #31             @ extract SIOCNT bit6 (serial IRQ en)
	strb r0, [r4, #7]            @ snapshot -> EF90+7
	ldr r0, _08000AC4 @ =0x0000FEFE
	strh r0, [r2, #2]            @ SIODATA8 (0x0400012A) = 0xFEFE
	ldrh r0, [r2]
	movs r1, #0x80
	orrs r0, r1
	strh r0, [r2]                @ SIOCNT |= start bit (normal 8-bit)
	ldr r1, _08000AC8 @ =0x0400010E       @ TM3CNT_H
	movs r0, #0xc0
	strh r0, [r1]                @ Timer3 enable + IRQ (IntrMain slot 0)
	b _08000AE6

	.align 2, 0
_08000ABC: .4byte 0x0203EF90
_08000AC0: .4byte 0x04000128
_08000AC4: .4byte 0x0000FEFE
_08000AC8: .4byte 0x0400010E

@ Idle path (pending == 0): keep the serial wait-channel latch armed so
@ HALT can wake on the slot-0 (Timer3+Serial) combo interrupt.
_08000ACC:
	ldrb r0, [r4, #9]
	cmp r0, #0
	bne _08000AE4                @ latch set -> someone suppressed us
	ldr r3, _08000AEC @ =0x04000208       @ IME
	strh r5, [r3]                @ IME(halfword) = 0 (r5 == 0 here)
	ldr r2, _08000AF0 @ =0x03007FF8       @ BIOS IntrWait flag halfword
	ldrh r0, [r2]
	movs r1, #0x80
	orrs r0, r1
	strh r0, [r2]                @ flags |= 0x0080 (serial channel)
	movs r0, #1
	strh r0, [r3]                @ IME = 1
_08000AE4:
	strb r5, [r4, #9]            @ latch = 0
_08000AE6:
	pop {r4, r5}
	pop {r0}
	bx r0

	.align 2, 0
_08000AEC: .4byte 0x04000208
_08000AF0: .4byte 0x03007FF8
