@ GT Advance 3 - MTO sound driver: song-bank claim / channel-swap cluster
@ Region: file offset 0x02B488-0x02B66C (VMA 0x0802B488-0x0802B66C).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/sound_api.s
@
@ Song directory at ROM 0x08061FA4, 8 bytes per song:
@   entry+0 u32 pointer -> runtime song-state struct (+2 = priority
@           byte; bit0 = droppable)
@   entry+4 u16 gate - must equal 3 before a claim succeeds
@   entry+6 u16 (unused here)
@ Master block 0x0203EE50 channel ownership:
@   +0x0F u8 channel-C owner song id (0xFF free)
@   +0x10 u8 channel-D owner song id (0xFF free)
@   +0x04/+0x06 s16 command words written by the claimer
@   +0x11 u8 volume-changed request (set on swap)
@ Wrappers _0802B64C/_0802B65C forward to the sequencer start/pause
@ primitives sub_0802C548 / sub_0802C614.

.thumb

@ ----------------------------------------------------------------------------
@ _0802B488(id, val, mode) — scaled volume + pan apply for one channel.
@ mode==1: vol = round-down(val * (id/32 + 110) / 256); pan = id*100/130
@ (via sub_0802DE04). else: vol = (val+1)/2; same pan path.
_0802B488:
	push {r4, r5, lr}
	lsls r0, r0, #16
	lsrs r3, r0, #16      @ id
	lsls r1, r1, #16
	lsrs r1, r1, #16      @ val
	lsls r2, r2, #16
	asrs r2, r2, #16      @ mode
	cmp r2, #1
	bne _0802B4B4
	lsls r0, r1, #16
	asrs r0, r0, #16
	lsls r2, r3, #16
	asrs r1, r2, #21      @ id/32 (arithmetic)
	adds r1, #110         @ 0x6E
	muls r0, r1
	adds r5, r2, #0       @ keep id<<16 for later
	cmp r0, #0
	bge _0802B4AE
	adds r0, #255         @ floor toward -inf
_0802B4AE:
	lsls r0, r0, #8
	lsrs r1, r0, #16      @ product >> 8
	b _0802B4C2
_0802B4B4:
	lsls r0, r1, #16
	asrs r1, r0, #16
	lsrs r0, r0, #31      @ val & 1
	adds r1, r1, r0
	lsls r1, r1, #15
	lsrs r1, r1, #16      @ (val+1)/2
	lsls r5, r3, #16
_0802B4C2:
	ldr r4, _0802B4F8_lit @ =0x03001764
	ldr r0, [r4, #0]
	ldrb r0, [r0, #13]    @ master+0xD level byte
	lsls r2, r1, #16
	asrs r2, r2, #16
	movs r1, #255
	bl sub_0802B718       @ volume apply
	ldr r1, _0802B4FC_lit @ word below
	adds r0, r5, r1
	asrs r0, r0, #16      @ pan source term
	movs r1, #100
	muls r0, r1
	movs r1, #130
	bl sub_0802DE04       @ *100/130 helper
	adds r2, r0, #0
	ldr r0, [r4, #0]
	ldrb r0, [r0, #13]
	lsls r2, r2, #16
	asrs r2, r2, #16
	movs r1, #255
	bl sub_0802B74C       @ pan apply
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_0802B4F8_lit:
	.word 0x03001764
_0802B4FC_lit:
	.word 0xEE6C0000      @ as stored; combined with id<<16 then >>16

@ ----------------------------------------------------------------------------
@ _0802B500(idx, w4, w6) — SONG LOAD / CHANNEL CLAIM.
@ Bank = 0x08061FA4 + idx*0x2000. Requires u16[bank+4] == 3.
@ Arbitrates against current C/D owners by priority byte [bank+2];
@ winners get master+0xF/+0x10 := idx and command words into
@ master+0x04/+0x06.
_0802B500:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	lsls r0, r0, #16
	lsrs r4, r0, #16      @ idx
	mov sl, r4
	lsls r1, r1, #16
	lsrs r5, r1, #16      @ w4
	mov r9, r5
	lsls r2, r2, #16
	lsrs r6, r2, #16      @ w6
	mov r8, r6
	ldr r7, _0802B540_lit @ =0x08061FA4
	lsls r0, r4, #16
	asrs r0, r0, #13      @ idx*8 -> wait: <<16>>13 = idx*8
	adds r2, r0, r7
	ldrh r0, [r2, #4]
	cmp r0, #3            @ bank gate word == 3?
	bne _0802B59E
	ldr r1, _0802B544_lit @ =0x03001764
	mov ip, r1
	ldr r3, [r1, #0]      @ master
	ldrb r0, [r3, #15]    @ C owner
	cmp r0, #255
	bne _0802B55C
	ldrb r0, [r3, #16]    @ D owner
	cmp r0, #255
	bne _0802B548
	strb r4, [r3, #16]    @ claim D := idx
	b _0802B586
	.align 2, 0
_0802B540_lit:
	.word 0x08061FA4
_0802B544_lit:
	.word 0x03001764
_0802B548:
	ldrb r1, [r3, #16]    @ D owner id
	lsls r0, r1, #3
	adds r0, r0, r7
	ldr r1, [r0, #0]      @ D bank ptr
	ldr r0, [r2, #0]      @ new bank ptr
	ldrb r1, [r1, #2]     @ D priority
	ldrb r0, [r0, #2]     @ new priority
	cmp r1, r0
	bcc _0802B59E         @ D >= new ? drop request
	b _0802B582
_0802B55C:
	ldrb r1, [r3, #15]    @ C owner id
	lsls r0, r1, #3
	adds r0, r0, r7
	ldr r1, [r0, #0]
	ldr r0, [r2, #0]
	ldrb r2, [r0, #2]
	ldrb r1, [r1, #2]
	cmp r1, r2
	bcc _0802B59E         @ C >= new ? drop
	ldrb r0, [r3, #16]    @ D owner
	cmp r0, #255
	beq _0802B58E
	ldrb r1, [r3, #16]
	lsls r0, r1, #3
	adds r0, r0, r7
	ldr r0, [r0, #0]
	ldrb r0, [r0, #2]
	cmp r0, r2
	bcc _0802B59E
_0802B582:
	strb r4, [r3, #16]    @ D := idx
	mov r1, ip
_0802B586:
	ldr r0, [r1, #0]
	strh r5, [r0, #4]     @ master+4 := w4
	strh r6, [r0, #6]     @ master+6 := w6
	b _0802B59E
_0802B58E:
	mov r0, sl
	strb r0, [r3, #16]    @ D := idx (from saved copy)
	mov r1, ip
	ldr r0, [r1, #0]
	mov r1, r9
	strh r1, [r0, #4]
	mov r1, r8
	strh r1, [r0, #6]
_0802B59E:
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0

@ ----------------------------------------------------------------------------
@ _0802B5AC — channel swap / teardown.
@ If D owns a song whose bank has bit0 set and C differs: stop old C,
@ promote D->C, start it. If D empty but C plays a bit0 bank (and probe
@ over stream banks fails): pause C and free it. Always clears D owner.
_0802B5AC:
	push {r4, r5, lr}
	ldr r4, _0802B5D8_lit @ =0x03001764
	ldr r2, [r4, #0]
	ldrb r0, [r2, #16]    @ D owner
	cmp r0, #255
	beq _0802B5FE
	ldr r1, _0802B5DC_lit @ =0x08061FA4
	adds r3, r0, #0
	lsls r0, r3, #3
	adds r0, r0, r1
	ldr r1, [r0, #0]      @ D song struct
	movs r5, #1
	adds r0, r5, #0
	ldrb r1, [r1, #2]
	ands r0, r1           @ struct.prio & 1 ?
	cmp r0, #0
	beq _0802B5E0
	ldrb r0, [r2, #15]    @ C owner
	cmp r3, r0
	beq _0802B63A
	b _0802B5E2
	.hword 0
_0802B5D8_lit:
	.word 0x03001764
_0802B5DC_lit:
	.word 0x08061FA4
_0802B5E0:
	ldrb r0, [r2, #15]
_0802B5E2:
	cmp r0, #255
	beq _0802B5EA
	bl sub_0802B65C       @ pause old C
_0802B5EA:
	ldr r1, [r4, #0]
	ldrb r0, [r1, #16]
	strb r0, [r1, #15]    @ C := D
	ldr r0, [r4, #0]
	strb r5, [r0, #17]    @ volume re-apply request
	ldr r0, [r4, #0]
	ldrb r0, [r0, #15]
	bl sub_0802B64C       @ start new C
	b _0802B63A
_0802B5FE:
	ldrb r0, [r2, #15]
	cmp r0, #255
	beq _0802B63A
	ldr r1, _0802B61C_lit @ =0x08061FA4
	adds r2, r0, #0
	lsls r0, r2, #3
	adds r0, r0, r1
	ldr r1, [r0, #0]
	movs r0, #1
	ldrb r1, [r1, #2]
	ands r0, r1
	cmp r0, #0
	beq _0802B620
	adds r0, r2, #0
	b _0802B630
_0802B61C_lit:
	.word 0x08061FA4
_0802B620:
	movs r0, #3
	bl sub_0802B3B8       @ any stream active?
	lsls r0, r0, #24
	cmp r0, #0
	bne _0802B63A
	ldr r0, [r4, #0]
	ldrb r0, [r0, #15]
_0802B630:
	bl sub_0802B65C       @ pause
	ldr r1, [r4, #0]
	movs r0, #255
	strb r0, [r1, #15]    @ free C
_0802B63A:
	ldr r0, _0802B648_lit @ =0x03001764
	ldr r1, [r0, #0]
	movs r0, #255
	strb r0, [r1, #16]    @ clear D owner
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_0802B648_lit:
	.word 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B64C(ch) — start wrapper -> sequencer sub_0802C548
_0802B64C:
sub_0802B64C:
	push {lr}
	lsls r0, r0, #16
	lsrs r0, r0, #16
	bl sub_0802C548
	pop {r0}
	bx r0
	.hword 0

@ ----------------------------------------------------------------------------
@ _0802B65C(ch) — pause wrapper -> sequencer sub_0802C614
_0802B65C:
sub_0802B65C:
	push {lr}
	lsls r0, r0, #16
	lsrs r0, r0, #16
	bl sub_0802C614
	pop {r0}
	bx r0
	.hword 0

	.align 2, 0

@ End-of-region anchor for the splicer: this file declares exactly one
@ `@ Region:` (0x02B488-0x02B66C), and the promoted body at 0x0802b65c ends on
@ 0x0802b66c, exactly that boundary. Without it the body is refused as
@ 'no end marker'.
sound_bank_end:
