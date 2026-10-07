@ GT Advance 3 - MTO sound driver: state init + tick entries + public API
@ Region: file offset 0x2B04C-0x2B488 (VMA 0x0802B04C-0x0802B488).
@
@ Disassembled via gbadisasm/objdump from baserom.gba; byte-exact.
@
@ Master state block lives at EWRAM 0x0203EE50; its pointer is kept at
@ IWRAM 0x03001764 (every accessor reloads it). Field map of the master
@ block (offsets verified from this cluster):
@   +0x00 u16 fade target value
@   +0x02 u16 current fade value
@   +0x04 s16 channel-C apply value     +0x06 s16 channel-D apply value
@   +0x08 u8 flags: 01=driver enabled, 04=fade-down active,
@                   08=fade-up active, 10=target-reached latch,
@                   20=pause gate
@   +0x09 u8 master volume level (0xFF = muted/none)
@   +0x0A u8 secondary volume level
@   +0x0C u8 fade step
@   +0x0D u8 song speed byte (from ROM table 0x080613B8, stride 0x62)
@   +0x0E u8 flags2: bit6 = re-apply request (consumed by the tick),
@                    bits 2-5 aux gates cleared on fade arm
@   +0x0F u8 channel-C level            +0x10 u8 channel-D level
@   +0x11 u8 "volume changed" flag (set by setters, consumed per tick)
@
@ Four music/SFX stream slots sit right below the block at EWRAM
@ 0x0203ED40 / 0x0203ED80 / 0x0203EDC0 / 0x0203EE10 (stride 0x40);
@ _0802B3B8 probes word[bank+4] for activity.
@
@ Tick wiring (asm/agbmain.s): VBlank IRQ -> _0802B07C (sample pump
@ sub_0802BE78 when enabled); VCounter IRQ -> _0802B098 (sequencer
@ tick: service sub_0802C53C, advance master fade, apply ch C/D).

.thumb

@ ----------------------------------------------------------------------------
@ _0802B04C(state) — driver init (AgbMain calls it with r0=0x0203EE50).
@ sub_0802C4C4 low-level reset; publish state ptr at 0x03001764;
@ [+8]=1 enable; levels [+9]/[+0xF]/[+0x10] |= 0xFF.
sub_0802B04C:
_0802B04C:
	push {r4, lr}
	adds r4, r0, #0
	bl sub_0802C4C4
	ldr r0, _0802B078 @ =0x03001764
	str r4, [r0]
	movs r0, #1
	strb r0, [r4, #8]
	movs r0, #255
	ldrb r1, [r4, #9]
	orrs r1, r0
	strb r1, [r4, #9]
	ldrb r1, [r4, #15]
	orrs r1, r0
	strb r1, [r4, #15]
	ldrb r1, [r4, #16]
	orrs r0, r1
	strb r0, [r4, #16]
	pop {r4}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0802B078: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B07C — VBlank-side entry: run sample pump when enabled.
sub_0802B07C:
_0802B07C:
	push {lr}
	ldr r0, _0802B094 @ =0x03001764
	ldr r1, [r0]
	movs r0, #1
	ldrb r1, [r1, #8]
	ands r0, r1
	cmp r0, #0
	beq _0802B090
	bl sub_0802BE78
_0802B090:
	pop {r0}
	bx r0
	.align 2, 0
_0802B094: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B098 — sequencer tick (VCounter slot).
sub_0802B098:
_0802B098:
	push {r4, r5, r6, r7, lr}
	ldr r5, _0802B108 @ =0x03001764
	ldr r1, [r5]
	movs r0, #1
	ldrb r1, [r1, #8]
	ands r0, r1
	cmp r0, #0
	beq _0802B186
	bl sub_0802C53C
	ldr r1, [r5]
	movs r6, #0
	movs r0, #0
	strb r0, [r1, #17]
	ldr r2, [r5]
	ldrb r3, [r2, #8]
	movs r0, #4
	ands r0, r3
	cmp r0, #0
	beq _0802B122
	movs r0, #32
	ands r0, r3
	cmp r0, #0
	bne _0802B164
	ldrh r1, [r2, #2]
	ldrb r4, [r2, #12]
	subs r0, r1, r4
	strh r0, [r2, #2]
	lsls r0, r0, #16
	ldrh r4, [r2, #0]
	lsls r1, r4, #16
	cmp r0, r1
	bgt _0802B114
	strh r4, [r2, #2]
	movs r0, #16
	orrs r0, r3
	strb r0, [r2, #8]
	ldr r1, [r5]
	movs r0, #251
	ldrb r7, [r1, #8]
	ands r0, r7
	strb r0, [r1, #8]
	ldr r1, [r5]
	movs r0, #64
	ldrb r2, [r1, #14]
	ands r0, r2
	cmp r0, #0
	beq _0802B10C
	bl sub_0802B234
	ldr r1, [r5]
	movs r0, #191
	ldrb r3, [r1, #14]
	ands r0, r3
	strb r0, [r1, #14]
	b _0802B164
	.align 2, 0
_0802B108: .4byte 0x03001764
_0802B10C:
	ldrb r0, [r1, #9]
	movs r4, #2
	ldrsh r2, [r1, r4]
	b _0802B11A
_0802B114:
	ldrb r0, [r2, #9]
	movs r7, #2
	ldrsh r2, [r2, r7]
_0802B11A:
	movs r1, #255
	bl sub_0802B718
	b _0802B164
_0802B122:
	movs r0, #8
	ands r0, r3
	cmp r0, #0
	beq _0802B164
	movs r0, #32
	ands r0, r3
	cmp r0, #0
	bne _0802B164
	ldrh r1, [r2, #2]
	ldrb r4, [r2, #12]
	adds r0, r1, r4
	strh r0, [r2, #2]
	lsls r0, r0, #16
	ldrh r4, [r2, #0]
	lsls r1, r4, #16
	cmp r0, r1
	blt _0802B156
	strh r4, [r2, #2]
	movs r0, #16
	orrs r0, r3
	strb r0, [r2, #8]
	ldr r1, [r5]
	movs r0, #247
	ldrb r7, [r1, #8]
	ands r0, r7
	strb r0, [r1, #8]
_0802B156:
	ldr r1, [r5]
	ldrb r0, [r1, #9]
	movs r3, #2
	ldrsh r2, [r1, r3]
	movs r1, #255
	bl sub_0802B718
_0802B164:
	ldr r4, _0802B18C @ =0x03001764
	ldr r1, [r4]
	ldrb r0, [r1, #15]
	cmp r0, #255
	beq _0802B186
	movs r7, #4
	ldrsh r2, [r1, r7]
	movs r1, #255
	bl sub_0802B718
	ldr r1, [r4]
	ldrb r0, [r1, #15]
	movs r3, #6
	ldrsh r2, [r1, r3]
	movs r1, #255
	bl sub_0802B74C
_0802B186:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0
_0802B18C: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B190 — sound OFF: cleanup sub_0802CB20, clear enable bit.
sub_0802B190:
_0802B190:
	push {r4, lr}
	ldr r4, _0802B1B4 @ =0x03001764
	ldr r1, [r4]
	movs r0, #1
	ldrb r1, [r1, #8]
	ands r0, r1
	cmp r0, #0
	beq _0802B1AE
	bl sub_0802CB20
	ldr r1, [r4]
	movs r0, #254
	ldrb r2, [r1, #8]
	ands r0, r2
	strb r0, [r1, #8]
_0802B1AE:
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0802B1B4: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B1B8 — sound ON: low-level init sub_0802C4C4 + sub_0802CB84,
@ set enable bit (no-op when already enabled).
sub_0802B1B8:
_0802B1B8:
	push {r4, lr}
	ldr r4, _0802B1E0 @ =0x03001764
	ldr r1, [r4]
	movs r0, #1
	ldrb r1, [r1, #8]
	ands r0, r1
	cmp r0, #0
	bne _0802B1DA
	bl sub_0802C4C4
	bl sub_0802CB84
	ldr r0, [r4]
	movs r1, #1
	ldrb r2, [r0, #8]
	orrs r1, r2
	strb r1, [r0, #8]
_0802B1DA:
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0802B1E0: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B1E4(vol) — master volume set: [+9]=vol, cur[+2]=0xFF,
@ change-flag [+0x11]=1, apply sub_0802B64C(vol), clear +8 bit2.
sub_0802B1E4:
_0802B1E4:
	push {r4, lr}
	ldr r4, _0802B210 @ =0x03001764
	ldr r1, [r4]
	strb r0, [r1, #9]
	ldr r1, [r4]
	movs r0, #255
	strh r0, [r1, #2]
	movs r0, #1
	strb r0, [r1, #17]
	ldr r0, [r4]
	ldrb r0, [r0, #9]
	bl sub_0802B64C
	ldr r1, [r4]
	movs r0, #251
	ldrb r2, [r1, #8]
	ands r0, r2
	strb r0, [r1, #8]
	pop {r4}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0802B210: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B214(vol) — conditional master-volume set (skip if unchanged).
sub_0802B214:
_0802B214:
	push {lr}
	lsls r0, r0, #16
	lsrs r1, r0, #16
	ldr r0, _0802B230 @ =0x03001764
	ldr r0, [r0]
	ldrb r0, [r0, #9]
	cmp r0, r1
	beq _0802B22A
	adds r0, r1, #0
	bl sub_0802B1E4
_0802B22A:
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0802B230: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B234 — stop/mute-all: apply current [+9] via sub_0802B65C,
@ [+9]=0xFF, +8 &= 0xD3 (keeps only enable|fade-up bits).
sub_0802B234:
_0802B234:
	push {r4, lr}
	ldr r4, _0802B258 @ =0x03001764
	ldr r0, [r4]
	ldrb r0, [r0, #9]
	bl sub_0802B65C
	ldr r1, [r4]
	movs r0, #255
	strb r0, [r1, #9]
	ldr r1, [r4]
	movs r0, #211
	ldrb r2, [r1, #8]
	ands r0, r2
	strb r0, [r1, #8]
	pop {r4}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0802B258: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B25C — pause: if [+9]!=0xFF run sub_0802C614, set +8 bit5.
sub_0802B25C:
_0802B25C:
	push {r4, lr}
	ldr r4, _0802B27C @ =0x03001764
	ldr r1, [r4]
	ldrb r0, [r1, #9]
	cmp r0, #255
	beq _0802B276
	bl sub_0802C614
	ldr r1, [r4]
	movs r0, #32
	ldrb r2, [r1, #8]
	orrs r0, r2
	strb r0, [r1, #8]
_0802B276:
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0802B27C: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B280 — resume: if [+9]!=0xFF run sub_0802C5C0, clear +8 bit5.
sub_0802B280:
_0802B280:
	push {r4, lr}
	ldr r4, _0802B2A0 @ =0x03001764
	ldr r1, [r4]
	ldrb r0, [r1, #9]
	cmp r0, #255
	beq _0802B29A
	bl sub_0802C5C0
	ldr r1, [r4]
	movs r0, #223
	ldrb r2, [r1, #8]
	ands r0, r2
	strb r0, [r1, #8]
_0802B29A:
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0802B2A0: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B2A4(target u16, step u8) — arm master fade toward target.
@ Clears +0xE bits {1..6} (&0xBF). cur[+2] >= target -> store
@ target/+step, +8|=4 (down); else +8|=8 (up). Either way clears +8
@ bit4 (done latch).
sub_0802B2A4:
_0802B2A4:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	lsls r0, r0, #16
	lsrs r3, r0, #16
	mov r8, r3
	lsls r1, r1, #24
	lsrs r5, r1, #24
	adds r6, r5, #0
	ldr r4, _0802B2E4 @ =0x03001764
	ldr r1, [r4]
	movs r0, #191
	ldrb r2, [r1, #14]
	ands r0, r2
	movs r7, #0
	mov ip, r7
	strb r0, [r1, #14]
	ldr r2, [r4]
	ldrb r0, [r2, #9]
	cmp r0, #255
	beq _0802B302
	lsls r1, r3, #16
	ldrh r7, [r2, #2]
	lsls r0, r7, #16
	cmp r0, r1
	blt _0802B2E8
	strh r3, [r2, #0]
	strb r5, [r2, #12]
	ldr r1, [r4]
	movs r0, #4
	b _0802B2F2
	movs r0, r0
	.align 2, 0
_0802B2E4: .4byte 0x03001764
_0802B2E8:
	mov r0, r8
	strh r0, [r2, #0]
	strb r6, [r2, #12]
	ldr r1, [r4]
	movs r0, #8
_0802B2F2:
	ldrb r2, [r1, #8]
	orrs r0, r2
	strb r0, [r1, #8]
	ldr r1, [r4]
	movs r0, #239
	ldrb r7, [r1, #8]
	ands r0, r7
	strb r0, [r1, #8]
_0802B302:
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0

@ ----------------------------------------------------------------------------
@ _0802B30C(step) — trigger fade-out to 0: _0802B2A4(0, step), then
@ set +0xE bit6 (re-apply request consumed by the tick).
sub_0802B30C:
_0802B30C:
	push {lr}
	adds r1, r0, #0
	lsls r1, r1, #24
	lsrs r1, r1, #24
	movs r0, #0
	bl sub_0802B2A4
	ldr r0, _0802B32C @ =0x03001764
	ldr r1, [r0]
	movs r0, #64
	ldrb r2, [r1, #14]
	orrs r0, r2
	strb r0, [r1, #14]
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0802B32C: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B330 -> s16 — remaining fade distance: 0 when muted or not
@ fading down; target-cur while fading down (+8&4); else 0.
sub_0802B330:
_0802B330:
	movs r3, #0
	ldr r0, _0802B340 @ =0x03001764
	ldr r1, [r0]
	ldrb r0, [r1, #9]
	cmp r0, #255
	bne _0802B344
	movs r0, #0
	b _0802B364
	.align 2, 0
_0802B340: .4byte 0x03001764
_0802B344:
	ldrb r2, [r1, #8]
	movs r0, #8
	ands r0, r2
	cmp r0, #0
	bne _0802B356
	movs r0, #4
	ands r0, r2
	cmp r0, #0
	beq _0802B360
_0802B356:
	ldrh r2, [r1, #0]
	ldrh r3, [r1, #2]
	subs r0, r2, r3
	lsls r0, r0, #16
	lsrs r3, r0, #16
_0802B360:
	lsls r0, r3, #16
	asrs r0, r0, #16
_0802B364:
	bx lr
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _0802B368(vol) — deferred volume request: change-flag [+0x11]=1 then
@ apply via sub_0802B64C. The engine-wide volume setter (151 callers).
sub_0802B368:
_0802B368:
	push {lr}
	lsls r0, r0, #16
	lsrs r0, r0, #16
	ldr r1, _0802B380 @ =0x03001764
	ldr r2, [r1]
	movs r1, #1
	strb r1, [r2, #17]
	bl sub_0802B64C
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0802B380: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B384(v) — secondary volume set: [+0xA]=v, change flag, apply.
sub_0802B384:
_0802B384:
	push {lr}
	lsls r0, r0, #16
	lsrs r0, r0, #16
	ldr r2, _0802B3A0 @ =0x03001764
	ldr r1, [r2]
	strb r0, [r1, #10]
	ldr r2, [r2]
	movs r1, #1
	strb r1, [r2, #17]
	bl sub_0802B64C
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0802B3A0: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B3A4 — apply secondary level byte[+0xA] via sub_0802B65C.
sub_0802B3A4:
_0802B3A4:
	push {lr}
	ldr r0, _0802B3B4 @ =0x03001764
	ldr r0, [r0]
	ldrb r0, [r0, #10]
	bl sub_0802B65C
	pop {r0}
	bx r0
	.align 2, 0
_0802B3B4: .4byte 0x03001764

@ ----------------------------------------------------------------------------
@ _0802B3B8(bank) -> bool — anything playing? Probes stream bank
@ {0:0x0203ED40, 1:0x0203ED80, 2:0x0203EDC0, other:0x0203EE10};
@ true when driver enabled and ([bank+4]&0xFFFF)!=0 and >= 0, or when
@ change-flag [+0x11]==1.
sub_0802B3B8:
_0802B3B8:
	adds r3, r0, #0
	ldr r2, _0802B3D0 @ =0x03001764
	ldr r1, [r2]
	movs r0, #1
	ldrb r1, [r1, #8]
	ands r0, r1
	cmp r0, #0
	beq _0802B414
	cmp r3, #0
	bne _0802B3D8
	ldr r0, _0802B3D4 @ =0x0203ED40
	b _0802B3F2
	.align 2, 0
_0802B3D0: .4byte 0x03001764
_0802B3D4: .4byte 0x0203ED40
_0802B3D8:
	cmp r3, #1
	bne _0802B3E4
	ldr r0, _0802B3E0 @ =0x0203ED80
	b _0802B3F2
	.align 2, 0
_0802B3E0: .4byte 0x0203ED80
_0802B3E4:
	cmp r3, #2
	bne _0802B3F0
	ldr r0, _0802B3EC @ =0x0203EDC0
	b _0802B3F2
	.align 2, 0
_0802B3EC: .4byte 0x0203EDC0
_0802B3F0:
	ldr r0, _0802B40C @ =0x0203EE10
_0802B3F2:
	ldr r1, [r0, #4]
	ldr r0, [r2]
	ldrb r0, [r0, #17]
	cmp r0, #1
	beq _0802B408
	ldr r0, _0802B410 @ =0x0000FFFF
	ands r0, r1
	cmp r0, #0
	beq _0802B414
	cmp r1, #0
	blt _0802B414
_0802B408:
	movs r0, #1
	b _0802B416
	.align 2, 0
_0802B40C: .4byte 0x0203EE10
_0802B410: .4byte 0x0000FFFF
_0802B414:
	movs r0, #0
_0802B416:
	bx lr

@ ----------------------------------------------------------------------------
@ _0802B418(a, b) — song-speed select: byte at ROM table
@ 0x080613B8 + a + b*0x62, plus 0x18, stored to [+0xD], applied via
@ sub_0802B64C.
sub_0802B418:
_0802B418:
	push {r4, r5, lr}
	ldr r4, _0802B444 @ =0x03001764
	ldr r5, [r4]
	ldr r3, _0802B448 @ =0x080613B8
	lsls r0, r0, #16
	asrs r0, r0, #16
	lsls r1, r1, #16
	asrs r1, r1, #16
	movs r2, #98
	muls r1, r2
	adds r0, r0, r1
	adds r0, r0, r3
	ldrb r0, [r0]
	adds r0, #24
	strb r0, [r5, #13]
	ldr r0, [r4]
	ldrb r0, [r0, #13]
	bl sub_0802B64C
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_0802B444: .4byte 0x03001764
_0802B448: .4byte 0x080613B8

@ ----------------------------------------------------------------------------
@ _0802B44C / _0802B460 — re-apply song speed byte[+0xD] via
@ sub_0802B65C. _0802B474 — same via sub_0802B64C.
sub_0802B44C:
_0802B44C:
	push {lr}
	ldr r0, _0802B45C @ =0x03001764
	ldr r0, [r0]
	ldrb r0, [r0, #13]
	bl sub_0802B65C
	pop {r0}
	bx r0
	.align 2, 0
_0802B45C: .4byte 0x03001764

sub_0802B460:
_0802B460:
	push {lr}
	ldr r0, _0802B470 @ =0x03001764
	ldr r0, [r0]
	ldrb r0, [r0, #13]
	bl sub_0802B65C
	pop {r0}
	bx r0
	.align 2, 0
_0802B470: .4byte 0x03001764

sub_0802B474:
_0802B474:
	push {lr}
	ldr r0, _0802B484 @ =0x03001764
	ldr r0, [r0]
	ldrb r0, [r0, #13]
	bl sub_0802B64C
	pop {r0}
	bx r0
	.align 2, 0
_0802B484: .4byte 0x03001764

@ End-of-region anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x0802b04c-0x0802b488), has no `.include`, and ends at
@ 0x0802b488 -- the same address the promoted body at 0x0802b474 ends on
@ (0x0802b488). The boundary is therefore unambiguous and the anchor is safe.
@ Without it promotion_screen refuses the body with "no end marker in sound_api.s".
sound_api_end:
