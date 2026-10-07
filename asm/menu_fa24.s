@ GT Advance 3 - menu record setup + record state dispatcher
@ Region: file offset 0x00FA24-0x00FF78 (VMA 0x0800FA24-0x0800FF78).
@ Pure Thumb, ARMCC; continuation of the menu span (menu_f924.s ends at
@ 0xFA24; next raw resumes at 0xFF78).
@
@ Function map:
@   sub_0800FA24 - record setup: stamps rec+0x88/0x94/0xA0 = {6,2,1},
@                  attaches resources (sub_0800DAB8 at rec+0x40,
@                  sub_080075E8 pair, sub_0800798C at rec / rec+0x20,
@                  sub_08007A58, sub_08007614), then allocs/sends
@                  (sub_0800572C x N + sub_08007ABC x N) into rec+0x150..
@                  rec+0x1C0 cells driven by rec+0xD6 variant; tail sets
@                  rec+0x40=0, rec+0x44=1, fills rec+0x54 (0x20) and
@                  rec+0x4C (0xA0), rec+0x48=1, rec+0x4A=5.
@   sub_0800FE90 - record state dispatcher: 8-entry jump table @0xFEB8 on
@                  s16[0x03001780+0xFBC]; cases pick a u16 for rec+0xD6 and
@                  call sub_08007770(0x082D9EF8, rec, 1, N, 0, 4, 1).
@
@ Transcribed from baserom.gba via objdump; byte-exact (make SHA gate).
@ Unconverted targets keep numeric `bl 0x0800XXXX` form. Pools at
@ original offsets.

	.type sub_0800FA24, %function
sub_0800FA24:
_0800FA24:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	sub sp, #12
	adds r7, r0, #0
	adds r1, r7, #0
	adds r1, #136           @ 0x88
	movs r0, #6
	str r0, [r1, #0]
	adds r1, #12
	movs r0, #2
	str r0, [r1, #0]
	adds r1, #12
	movs r0, #1
	str r0, [r1, #0]
	adds r0, r7, #0
	adds r0, #64            @ 0x40
	bl sub_0800DAB8
	ldr r0, _0800FD98
	movs r1, #0
	movs r2, #3
	bl sub_080075E8
	ldr r4, _0800FD9C
	adds r0, r4, #0
	adds r1, r7, #0
	bl sub_0800798C
	adds r0, r7, #0
	bl sub_08007A58
	adds r0, r4, #0
	movs r1, #0
	movs r2, #4
	bl sub_080075E8
	ldr r4, _0800FDA0
	adds r1, r7, #0
	adds r1, #32            @ 0x20
	adds r0, r4, #0
	bl sub_0800798C
	adds r0, r4, #0
	movs r1, #0
	movs r2, #5
	bl sub_080075E8
	adds r0, r4, #0
	movs r1, #2
	movs r2, #0
	movs r3, #6
	bl sub_08007614
	movs r0, #16
	bl sub_0800572C
	movs r1, #168           @ 0xa8
	lsls r1, r1, #1
	adds r1, r1, r7
	mov sl, r1
	str r0, [r1, #0]
	movs r2, #170           @ 0xaa
	lsls r2, r2, #1
	adds r6, r7, r2
	ldr r3, _0800FDA4
	ldrh r0, [r3, #0]
	str r0, [r6, #0]
	movs r0, #16
	bl sub_0800572C
	movs r1, #174           @ 0xae
	lsls r1, r1, #1
	adds r1, r1, r7
	mov r9, r1
	str r0, [r1, #0]
	movs r2, #176           @ 0xb0
	lsls r2, r2, #1
	adds r5, r7, r2
	ldr r3, _0800FDA4
	ldrh r0, [r3, #10]
	str r0, [r5, #0]
	movs r0, #16
	bl sub_0800572C
	movs r1, #180           @ 0xb4
	lsls r1, r1, #1
	adds r1, r1, r7
	mov r8, r1
	str r0, [r1, #0]
	movs r2, #182           @ 0xb6
	lsls r2, r2, #1
	adds r4, r7, r2
	ldr r3, _0800FDA4
	ldrh r0, [r3, #20]
	str r0, [r4, #0]
	ldr r0, [r7, #36]       @ 0x24
	ldr r1, [r6, #0]
	mov r3, sl
	ldr r2, [r3, #0]
	bl sub_08007ABC
	ldr r0, [r7, #36]       @ 0x24
	ldr r1, [r5, #0]
	mov r3, r9
	ldr r2, [r3, #0]
	bl sub_08007ABC
	ldr r0, [r7, #36]       @ 0x24
	ldr r1, [r4, #0]
	mov r3, r8
	ldr r2, [r3, #0]
	bl sub_08007ABC
	adds r0, r7, #0
	adds r0, #214           @ 0xd6
	str r0, [sp, #8]
	ldrh r1, [r0, #0]
	cmp r1, #1
	beq _0800FB90
	movs r0, #16
	bl sub_0800572C
	movs r2, #186           @ 0xba
	lsls r2, r2, #1
	adds r2, r2, r7
	mov sl, r2
	str r0, [r2, #0]
	movs r3, #188           @ 0xbc
	lsls r3, r3, #1
	adds r6, r7, r3
	ldr r1, _0800FDA4
	ldrh r0, [r1, #0]
	str r0, [r6, #0]
	movs r0, #16
	bl sub_0800572C
	movs r2, #192           @ 0xc0
	lsls r2, r2, #1
	adds r2, r2, r7
	mov r9, r2
	str r0, [r2, #0]
	movs r3, #194           @ 0xc2
	lsls r3, r3, #1
	adds r5, r7, r3
	ldr r1, _0800FDA4
	ldrh r0, [r1, #10]
	str r0, [r5, #0]
	movs r0, #16
	bl sub_0800572C
	movs r2, #198           @ 0xc6
	lsls r2, r2, #1
	adds r2, r2, r7
	mov r8, r2
	str r0, [r2, #0]
	movs r3, #200           @ 0xc8
	lsls r3, r3, #1
	adds r4, r7, r3
	ldr r1, _0800FDA4
	ldrh r0, [r1, #20]
	str r0, [r4, #0]
	ldr r0, [r7, #36]       @ 0x24
	ldr r1, [r6, #0]
	mov r3, sl
	ldr r2, [r3, #0]
	bl sub_08007ABC
	ldr r0, [r7, #36]       @ 0x24
	ldr r1, [r5, #0]
	mov r3, r9
	ldr r2, [r3, #0]
	bl sub_08007ABC
	ldr r0, [r7, #36]       @ 0x24
	ldr r1, [r4, #0]
	mov r3, r8
	ldr r2, [r3, #0]
	bl sub_08007ABC
_0800FB90:
	ldr r1, [sp, #8]
	ldrh r0, [r1, #0]
	cmp r0, #0
	blt _0800FBC6
	cmp r0, #1
	ble _0800FBA4
	cmp r0, #6
	bgt _0800FBC6
	cmp r0, #3
	blt _0800FBC6
_0800FBA4:
	movs r0, #16
	bl sub_0800572C
	movs r2, #162           @ 0xa2
	lsls r2, r2, #1
	adds r3, r7, r2
	str r0, [r3, #0]
	movs r0, #164           @ 0xa4
	lsls r0, r0, #1
	adds r2, r7, r0
	ldr r0, _0800FDA8
	ldr r1, [r0, #0]
	str r1, [r2, #0]
	ldr r0, [r7, #36]       @ 0x24
	ldr r2, [r3, #0]
	bl sub_08007ABC
_0800FBC6:
	adds r0, r7, #0
	adds r0, #214           @ 0xd6
	ldrh r1, [r0, #0]
	adds r5, r0, #0
	cmp r1, #2
	beq _0800FBD6
	cmp r1, #7
	bne _0800FBF8
_0800FBD6:
	movs r0, #16
	bl sub_0800572C
	movs r1, #222           @ 0xde
	lsls r1, r1, #1
	adds r2, r7, r1
	str r0, [r2, #0]
	movs r3, #224           @ 0xe0
	lsls r3, r3, #1
	adds r1, r7, r3
	movs r0, #20
	str r0, [r1, #0]
	ldr r0, [r7, #36]       @ 0x24
	ldr r2, [r2, #0]
	movs r1, #20
	bl sub_08007ABC
_0800FBF8:
	ldr r0, _0800FDAC
	movs r1, #0
	movs r2, #12
	bl sub_080075E8
	ldr r0, _0800FDB0
	adds r1, r7, #0
	adds r1, #56            @ 0x38
	bl sub_0800798C
	ldr r0, _0800FDB4
	adds r1, r7, #0
	adds r1, #40            @ 0x28
	bl sub_0800798C
	movs r0, #48             @ 0x30
	bl sub_0800572C
	movs r2, #138           @ 0x8a
	lsls r2, r2, #1
	adds r1, r7, r2
	str r0, [r1, #0]
	movs r3, #140           @ 0x8c
	lsls r3, r3, #1
	adds r0, r7, r3
	movs r6, #3
	str r6, [r0, #0]
	ldr r0, [r7, #44]       @ 0x2c
	ldr r2, [r1, #0]
	movs r1, #3
	bl sub_08007ABC
	movs r0, #48             @ 0x30
	bl sub_0800572C
	movs r2, #144           @ 0x90
	lsls r2, r2, #1
	adds r1, r7, r2
	str r0, [r1, #0]
	movs r3, #146           @ 0x92
	lsls r3, r3, #1
	adds r0, r7, r3
	str r6, [r0, #0]
	ldr r0, [r7, #44]       @ 0x2c
	ldr r2, [r1, #0]
	movs r1, #3
	bl sub_08007ABC
	ldrh r0, [r5, #0]
	cmp r0, #1
	bne _0800FCBE
	movs r0, #32
	bl sub_0800572C
	movs r1, #150           @ 0x96
	lsls r1, r1, #1
	adds r2, r7, r1
	str r0, [r2, #0]
	movs r3, #152           @ 0x98
	lsls r3, r3, #1
	adds r1, r7, r3
	movs r0, #6
	str r0, [r1, #0]
	ldr r0, [r7, #44]       @ 0x2c
	ldr r2, [r2, #0]
	movs r1, #6
	bl sub_08007ABC
	movs r0, #14
	bl sub_0800572C
	movs r1, #156           @ 0x9c
	lsls r1, r1, #1
	adds r4, r7, r1
	str r0, [r4, #0]
	ldr r1, _0800FDB8
	ldr r2, _0800FDBC
	adds r0, r1, r2
	movs r3, #0
	ldrsh r0, [r0, r3]
	adds r2, #68            @ 0x44
	adds r1, r1, r2
	movs r3, #0
	ldrsh r1, [r1, r3]
	bl sub_080256BC
	lsls r0, r0, #16
	asrs r0, r0, #16
	bl sub_08024C3C
	adds r1, r0, #0
	movs r2, #158           @ 0x9e
	lsls r2, r2, #1
	adds r0, r7, r2
	str r1, [r0, #0]
	ldr r0, [r7, #60]       @ 0x3c
	ldr r2, [r4, #0]
	bl sub_08007ABC
_0800FCBE:
	ldr r4, _0800FDC0
	adds r1, r7, #0
	adds r1, #24            @ 0x18
	adds r0, r4, #0
	bl sub_0800798C
	adds r0, r4, #0
	movs r1, #0
	movs r2, #10
	bl sub_080075E8
	movs r0, #12
	bl sub_0800572C
	adds r4, r7, #0
	adds r4, #228           @ 0xe4
	str r0, [r4, #0]
	movs r0, #0
	bl sub_080258A8
	adds r1, r0, #0
	adds r0, r7, #0
	adds r0, #232           @ 0xe8
	lsls r1, r1, #16
	asrs r1, r1, #16
	str r1, [r0, #0]
	ldr r0, [r7, #28]       @ 0x1c
	ldr r2, [r4, #0]
	bl sub_08007ABC
	ldr r4, _0800FDC4
	adds r1, r7, #0
	adds r1, #16            @ 0x10
	adds r0, r4, #0
	bl sub_0800798C
	movs r0, #32
	bl sub_0800572C
	adds r2, r7, #0
	adds r2, #252           @ 0xfc
	str r0, [r2, #0]
	movs r3, #128           @ 0x80
	lsls r3, r3, #1
	adds r1, r7, r3
	movs r0, #2
	str r0, [r1, #0]
	ldr r0, [r7, #20]       @ 0x14
	ldr r2, [r2, #0]
	movs r1, #2
	bl sub_08007ABC
	adds r0, r4, #0
	movs r1, #1
	movs r2, #8
	bl sub_080075E8
	adds r0, r4, #0
	movs r1, #0
	movs r2, #9
	bl sub_080075E8
	ldrh r5, [r5, #0]
	cmp r5, #1
	bne _0800FD42
	b _0800FE4C
_0800FD42:
	ldr r5, _0800FDC8
	adds r1, r7, #0
	adds r1, #48            @ 0x30
	adds r0, r5, #0
	bl sub_0800798C
	adds r0, r5, #0
	movs r1, #0
	movs r2, #11
	bl sub_080075E8
	ldr r0, _0800FDB8
	ldr r1, _0800FDCC
	adds r0, r0, r1
	ldrb r0, [r0, #0]
	cmp r0, #1
	bne _0800FDD0
	movs r2, #204           @ 0xcc
	lsls r2, r2, #1
	adds r4, r7, r2
	adds r0, r4, #0
	movs r1, #11
	bl 0x08025BC8
	ldr r0, [r7, #52]       @ 0x34
	movs r3, #206           @ 0xce
	lsls r3, r3, #1
	adds r1, r7, r3
	ldr r1, [r1, #0]
	ldr r2, [r4, #0]
	bl sub_08007ABC
	movs r0, #0
	str r0, [sp, #0]
	str r6, [sp, #4]
	movs r0, #1
	adds r1, r5, #0
	movs r2, #1
	movs r3, #0
	bl sub_08007770
	b _0800FE08
	.align 2, 0
_0800FD98: .4byte 0x082A798C
_0800FD9C: .4byte 0x082D9EF8
_0800FDA0: .4byte 0x082D7660
_0800FDA4: .4byte 0x080CB55C
_0800FDA8: .4byte 0x080CB57C
_0800FDAC: .4byte 0x082C4228
_0800FDB0: .4byte 0x082C5040
_0800FDB4: .4byte 0x082DD8FC
_0800FDB8: .4byte 0x03001780
_0800FDBC: .4byte 0x00000FF6
_0800FDC0: .4byte 0x082DFBDC
_0800FDC4: .4byte 0x082E4FCC
_0800FDC8: .4byte 0x082B7410
_0800FDCC: .4byte 0x000010C3
_0800FDD0:
	movs r0, #204           @ 0xcc
	lsls r0, r0, #1
	adds r4, r7, r0
	adds r0, r4, #0
	movs r1, #5
	bl 0x08025BC8
	ldr r0, [r7, #52]       @ 0x34
	movs r2, #206           @ 0xce
	lsls r2, r2, #1
	adds r1, r7, r2
	ldr r1, [r1, #0]
	ldr r2, [r4, #0]
	bl sub_08007ABC
	movs r0, #0
	str r0, [sp, #0]
	str r6, [sp, #4]
	movs r0, #1
	adds r1, r5, #0
	movs r2, #2
	movs r3, #0
	bl sub_08007770
	adds r1, r7, #0
	adds r1, #216           @ 0xd8
	movs r0, #1
	strh r0, [r1, #0]
_0800FE08:
	movs r0, #8
	bl sub_0800572C
	movs r3, #210           @ 0xd2
	lsls r3, r3, #1
	adds r2, r7, r3
	str r0, [r2, #0]
	movs r0, #212           @ 0xd4
	lsls r0, r0, #1
	adds r1, r7, r0
	movs r0, #22
	str r0, [r1, #0]
	ldr r0, [r7, #52]       @ 0x34
	ldr r2, [r2, #0]
	movs r1, #22
	bl sub_08007ABC
	movs r0, #8
	bl sub_0800572C
	movs r1, #216           @ 0xd8
	lsls r1, r1, #1
	adds r2, r7, r1
	str r0, [r2, #0]
	movs r3, #218           @ 0xda
	lsls r3, r3, #1
	adds r1, r7, r3
	movs r0, #21
	str r0, [r1, #0]
	ldr r0, [r7, #52]       @ 0x34
	ldr r2, [r2, #0]
	movs r1, #21
	bl sub_08007ABC
_0800FE4C:
	movs r0, #0
	str r0, [r7, #64]       @ 0x40
	adds r0, r7, #0
	adds r0, #68            @ 0x44
	movs r4, #1
	strh r4, [r0, #0]
	adds r0, #16
	movs r2, #32
	negs r2, r2
	movs r1, #0
	bl sub_0800D77C
	adds r0, r7, #0
	adds r0, #76            @ 0x4c
	movs r1, #0
	movs r2, #160           @ 0xa0
	bl sub_0800D77C
	adds r0, r7, #0
	adds r0, #72            @ 0x48
	strh r4, [r0, #0]
	adds r1, r7, #0
	adds r1, #74            @ 0x4a
	movs r0, #5
	strh r0, [r1, #0]
	add sp, #12
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0
	.type sub_0800FE90, %function
sub_0800FE90:
_0800FE90:
	push {r4, lr}
	sub sp, #8
	adds r3, r0, #0
	ldr r1, _0800FEB0
	ldr r4, _0800FEB4
	adds r0, r1, r4
	movs r4, #0
	ldrsh r0, [r0, r4]
	adds r4, r1, #0
	cmp r0, #7
	bhi _0800FF54
	lsls r0, r0, #2
	ldr r1, _0800FEB8
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	.align 2, 0
_0800FEB0: .4byte 0x03001780
_0800FEB4: .4byte 0x00000FBC
_0800FEB8: .4byte 0x0800FEBC   @ jump table base (points at first entry)
@ 8-entry jump table: idx = s16[0x03001780+0xFBC]
_0800FEBC: .4byte _0800FEDC
_0800FEC0: .4byte _0800FF0A
_0800FEC4: .4byte _0800FF14
_0800FEC8: .4byte _0800FF1E
_0800FECC: .4byte _0800FF54
_0800FED0: .4byte _0800FF00
_0800FED4: .4byte _0800FF54
_0800FED8: .4byte _0800FF28
_0800FEDC:
	movs r2, #4
	ldr r1, _0800FEF4
	adds r0, r4, r1
	movs r4, #0
	ldrsh r0, [r0, r4]
	cmp r0, #2
	bgt _0800FEF8
	adds r1, r3, #0
	adds r1, #214           @ 0xd6
	movs r0, #1
	b _0800FF52
	.align 2, 0
_0800FEF4: .4byte 0x0000103A
_0800FEF8:
	adds r1, r3, #0
	adds r1, #214           @ 0xd6
	movs r0, #0
	b _0800FF52
_0800FF00:
	movs r2, #3
	adds r1, r3, #0
	adds r1, #214           @ 0xd6
	movs r0, #5
	b _0800FF52
_0800FF0A:
	movs r2, #4
	adds r1, r3, #0
	adds r1, #214           @ 0xd6
	movs r0, #2
	b _0800FF52
_0800FF14:
	movs r2, #4
	adds r1, r3, #0
	adds r1, #214           @ 0xd6
	movs r0, #3
	b _0800FF52
_0800FF1E:
	movs r2, #4
	adds r0, r3, #0
	adds r0, #214           @ 0xd6
	strh r2, [r0, #0]
	b _0800FF54
_0800FF28:
	ldr r1, _0800FF3C
	adds r0, r4, r1
	movs r4, #0
	ldrsh r0, [r0, r4]
	cmp r0, #1
	beq _0800FF40
	cmp r0, #2
	beq _0800FF4A
	b _0800FF54
	.align 2, 0
_0800FF3C: .4byte 0x00001078
_0800FF40:
	movs r2, #3
	adds r1, r3, #0
	adds r1, #214           @ 0xd6
	movs r0, #6
	b _0800FF52
_0800FF4A:
	movs r2, #4
	adds r1, r3, #0
	adds r1, #214           @ 0xd6
	movs r0, #7
_0800FF52:
	strh r0, [r1, #0]
_0800FF54:
	ldr r1, _0800FF74
	lsls r2, r2, #16
	asrs r2, r2, #16
	movs r0, #4
	str r0, [sp, #0]
	movs r0, #1
	str r0, [sp, #4]
	movs r0, #0
	movs r3, #0
	bl sub_08007770
	add sp, #8
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0800FF74: .4byte 0x082D9EF8
