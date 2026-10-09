@ GT Advance 3 - block-A idle-path cluster (per-mode handlers + accessors)
@ Region: file offset 0x001988-0x0020E8 (VMA 0x08001988-0x080020E8)
@
@ Disassembled via gbadisasm/objdump from baserom.gba; byte-exact.
@ Companion: asm/blocka.s.
@
@ _08001B7C is the per-frame mode dispatcher called from AgbMain: it
@ maintains the +0xB0/+0xC0 work areas, then switches on requested mode
@ via a 10-entry jump table (base 0x1BE4). Handlers write the next
@ requested mode into +0x12 and a 0x03Ex/0x03Fx token into +0x04
@ (via _080016D0), forming the inter-mode handshake protocol.
@
@ The rest are accessors over block A shared with the relocated ctor
@ region (0x800Cxxx) and the save/menu engines: record getters, the
@ +0xC0 event-payload writer (_08001E48), mode setters, predicates.

.thumb

@ ----------------------------------------------------------------------------
@ _08001988 — mode-3 handler. Reads inverted-select record head; when
@ it equals 0x03ED runs the clock commit: payload -> [+0x2C], token ->
@ [+0x16], optional sub_080295C(0x0802D0E1, 0), divide-by-12 rounding
@ (sub_0802DFE4), CpuFastSet copy into [[slot]][+0x20] buffer, bumps
@ [+0x1A]; always re-arms +0x04=0x03FC and fires event 0 with [+0x1A],
@ then mirrors min(+0x16,+0x1A) through sub_0802DE04 into st+0xD0.
_08001988:
	push {r4, r5, lr}
	bl _08001680
	lsls r0, r0, #16
	ldr r1, _080019D8 @ =0x03ED0000
	cmp r0, r1
	bne.n _08001A32
	movs r0, #0
	bl _08001698
	lsls r0, r0, #16
	lsrs r1, r0, #16
	ldr r5, _080019DC @ =0x030000E4
	ldr r4, [r5]
	movs r2, #0x1a
	ldrsh r0, [r4, r2]
	cmp r1, r0
	bne.n _08001A32
	cmp r1, #0
	bne.n _080019E4
	movs r0, #1
	bl _08001698
	ldr r1, [r5]
	lsls r0, r0, #16
	lsrs r0, r0, #16
	str r0, [r1, #0x2c]
	movs r0, #2
	bl _08001698
	ldr r1, [r5]
	strh r0, [r1, #0x16]
	lsls r0, r0, #16
	cmp r0, #0
	bne.n _08001A28
	ldr r0, _080019E0 @ =0x0802E1D0
	movs r1, #0
	bl sub_0800295C
	b.n _08001A28
	.align 2, 0
_080019D8: .4byte 0x03ED0000
_080019DC: .4byte 0x030000E4
_080019E0: .4byte 0x0802E1D0
_080019E4:
	movs r2, #0x16
	ldrsh r0, [r4, r2]
	cmp r1, r0
	bne.n _08001A02
	ldr r0, [r4, #0x2c]
	movs r1, #12
	bl sub_0802DFE4
	adds r5, r0, #0
	cmp r5, #0
	bne.n _080019FC
	movs r5, #12
_080019FC:
	movs r0, #5
	strh r0, [r4, #0x12]
	b.n _08001A04
_08001A02:
	movs r5, #12
_08001A04:
	bl _080016B8
	adds r0, #4
	ldr r4, _08001A70 @ =0x030000E4
	ldr r1, [r4]
	ldr r1, [r1, #0x20]
	lsrs r2, r5, #31
	adds r2, r5, r2
	lsls r2, r2, #10
	lsrs r2, r2, #11
	bl sub_0802D974
	ldr r2, [r4]
	lsrs r1, r5, #1
	lsls r1, r1, #1
	ldr r0, [r2, #0x20]
	adds r0, r0, r1
	str r0, [r2, #0x20]
_08001A28:
	ldr r0, _08001A70 @ =0x030000E4
	ldr r1, [r0]
	ldrh r0, [r1, #0x1a]
	adds r0, #1
	strh r0, [r1, #0x1a]
_08001A32:
	movs r0, #0xfc
	lsls r0, r0, #2
	bl _080016D0
	ldr r4, _08001A70 @ =0x030000E4
	ldr r0, [r4]
	ldrh r1, [r0, #0x1a]
	movs r0, #0
	bl _08001E48
	ldr r4, [r4]
	movs r1, #0x1a
	ldrsh r0, [r4, r1]
	movs r2, #0x16
	ldrsh r1, [r4, r2]
	cmp r0, r1
	ble.n _08001A56
	adds r0, r1, #0
_08001A56:
	adds r4, #0xd0
	.short 0x0300        @ literal-pool residue emitted by armcc
	bl sub_0802DE04
	str r0, [r4]
	cmp r0, #0
	bge.n _08001A68
	movs r0, #0
	str r0, [r4]
_08001A68:
	pop {r4, r5}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08001A70: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001A74 — mode-4 handler: re-arm +0x04=0x03EE; if s16[+0x14] > 0
@ decrement it, else request mode 1.
_08001A74:
	push {lr}
	ldr r0, _08001A90 @ =0x000003EE
	bl _080016D0
	ldr r0, _08001A94 @ =0x030000E4
	ldr r1, [r0]
	ldrh r2, [r1, #0x14]
	movs r3, #0x14
	ldrsh r0, [r1, r3]
	cmp r0, #0
	ble.n _08001A98
	subs r0, r2, #1
	strh r0, [r1, #0x14]
	b.n _08001A9C
	.align 2, 0
_08001A90: .4byte 0x000003EE
_08001A94: .4byte 0x030000E4
_08001A98:
	movs r0, #1
	strh r0, [r1, #0x12]
_08001A9C:
	pop {r0}
	bx r0

@ ----------------------------------------------------------------------------
@ _08001AA0 — mode-5 handler: if inverted head == 0x03EE set
@ +0x12=1; always re-arm +0x04=0x03F1.
_08001AA0:
	push {lr}
	bl _08001680
	lsls r0, r0, #16
	ldr r1, _08001AC0 @ =0x03EE0000
	cmp r0, r1
	bne.n _08001AB6
	ldr r0, _08001AC4 @ =0x030000E4
	ldr r1, [r0]
	movs r0, #1
	strh r0, [r1, #0x12]
_08001AB6:
	ldr r0, _08001AC8 @ =0x000003F1
	bl _080016D0
	pop {r0}
	bx r0
	.align 2, 0
_08001AC0: .4byte 0x03EE0000
_08001AC4: .4byte 0x030000E4
_08001AC8: .4byte 0x000003F1

@ ----------------------------------------------------------------------------
@ _08001ACC — mode-6 handler: count records whose head != 0x03F4;
@ if none matched request mode 7 with +0x14=3; re-arm +0x04=0x03F2.
_08001ACC:
	push {r4, r5, r6, lr}
	movs r5, #1
	movs r4, #1
	ldr r0, _08001B18 @ =0x030000E4
	ldr r1, [r0]
	adds r6, r0, #0
	ldrh r1, [r1, #0xa]
	cmp r5, r1
	bge.n _08001AFC
_08001ADE:
	adds r0, r4, #0
	bl _080015F4
	lsls r0, r0, #16
	movs r1, #0xfd
	lsls r1, r1, #18
	cmp r0, r1
	bne.n _08001AF0
	adds r5, #1
_08001AF0:
	adds r4, #1
	ldr r0, _08001B18 @ =0x030000E4
	ldr r0, [r0]
	ldrh r0, [r0, #0xa]
	cmp r4, r0
	blt.n _08001ADE
_08001AFC:
	ldr r1, [r6]
	ldrh r0, [r1, #0xa]
	cmp r5, r0
	bne.n _08001B0C
	movs r0, #7
	strh r0, [r1, #0x12]
	movs r0, #3
	strh r0, [r1, #0x14]
_08001B0C:
	ldr r0, _08001B1C @ =0x000003F2
	bl _080016D0
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0
_08001B18: .4byte 0x030000E4
_08001B1C: .4byte 0x000003F2

@ ----------------------------------------------------------------------------
@ _08001B20 — mode-7 handler: if s16[+0x14] > 0 decrement, else
@ request mode 8; re-arm +0x04=0x03F3.
_08001B20:
	push {lr}
	ldr r0, _08001B38 @ =0x030000E4
	ldr r1, [r0]
	ldrh r2, [r1, #0x14]
	movs r3, #0x14
	ldrsh r0, [r1, r3]
	cmp r0, #0
	ble.n _08001B3C
	subs r0, r2, #1
	strh r0, [r1, #0x14]
	b.n _08001B40
	movs r0, r0
	.align 2, 0
_08001B38: .4byte 0x030000E4
_08001B3C:
	movs r0, #8
	strh r0, [r1, #0x12]
_08001B40:
	ldr r0, _08001B4C @ =0x000003F3
	bl _080016D0
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08001B4C: .4byte 0x000003F3

@ ----------------------------------------------------------------------------
@ _08001B50 — mode-8 handler: if inverted head == 0x03F3 request
@ mode 0xB; re-arm +0x04=0x03FD.
_08001B50:
	push {lr}
	bl _08001680
	lsls r0, r0, #16
	ldr r1, _08001B74 @ =0x03F30000
	cmp r0, r1
	bne.n _08001B66
	ldr r0, _08001B78 @ =0x030000E4
	ldr r1, [r0]
	movs r0, #0xb
	strh r0, [r1, #0x12]
_08001B66:
	movs r0, #0xfd
	lsls r0, r0, #2
	bl _080016D0
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08001B74: .4byte 0x03F30000
_08001B78: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001B7C — per-frame mode dispatcher (AgbMain idle-path call).
@ Special modes {8,11}: if sub_08000CA0(st+0xD0) nonzero, request mode 1.
@ Then snapshot record word -> st+0x30, wipe 16 KB of IWRAM state
@ starting at st+0x30 (two CpuFastSet fills), and dispatch on requested
@ mode through the switch table below.
_08001B7C:
	push {r4, lr}
	sub sp, #4
	ldr r4, _08001BD4 @ =0x030000E4
	ldr r0, [r4]
	ldrh r0, [r0, #0x10]
	bl _08001744
	cmp r0, #0
	beq.n _08001BA0
	ldr r0, [r4]
	adds r0, #0xd0
	bl sub_08000CA0
	cmp r0, #0
	beq.n _08001BA0
	ldr r1, [r4]
	movs r0, #1
	strh r0, [r1, #0x12]
_08001BA0:
	ldr r4, _08001BD4 @ =0x030000E4
	ldr r1, [r4]
	adds r0, r1, #0
	adds r0, #0x30
	adds r1, #0x70
	ldr r2, _08001BD8 @ =0x04000010
	bl sub_0802D974
	movs r0, #0
	str r0, [sp, #0]
	ldr r1, [r4]
	adds r1, #0x30
	ldr r2, _08001BDC @ =0x05000010
	mov r0, sp
	bl sub_0802D974
	ldr r0, [r4]
	ldrh r0, [r0, #0x10]
	subs r0, #1
	cmp r0, #9
	bhi.n _08001C40
	lsls r0, r0, #2
	ldr r1, _08001BE0 @ =0x08001BE4
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	.align 2, 0
_08001BD4: .4byte 0x030000E4
_08001BD8: .4byte 0x04000010
_08001BDC: .4byte 0x05000010
@ --- switch table: base pointer + 10 entries, index = mode - 1 ---
_08001BE0:
	.4byte 0x08001BE4   @ table base (loaded by the ldr above)
_08001BE4:
	.4byte 0x08001C0C   @ mode 1: +0x04 = 0x03EB
	.4byte 0x08001C18   @ mode 2: clock/count handler
	.4byte 0x08001C1E   @ mode 3
	.4byte 0x08001C24   @ mode 4: countdown chain
	.4byte 0x08001C2A   @ mode 5
	.4byte 0x08001C30   @ mode 6
	.4byte 0x08001C36   @ mode 7
	.4byte 0x08001C40   @ mode 8: no-op
	.4byte 0x08001C40   @ mode 9: no-op
	.4byte 0x08001C3C   @ mode 10: -> _08001B50 (inverted-head check)
_08001C0C:
	ldr r0, _08001C14 @ =0x000003EB
	bl _080016D0
	b.n _08001C40
	.align 2, 0
_08001C14: .4byte 0x000003EB
_08001C18:
	bl _08001854
	b.n _08001C40
_08001C1E:
	bl _08001988
	b.n _08001C40
_08001C24:
	bl _08001A74
	b.n _08001C40
_08001C2A:
	bl _08001AA0
	b.n _08001C40
_08001C30:
	bl _08001ACC
	b.n _08001C40
_08001C36:
	bl _08001B20
	b.n _08001C40
_08001C3C:
	bl _08001B50
_08001C40:
	add sp, #4
	pop {r4}
	pop {r0}
	bx r0

@ ----------------------------------------------------------------------------
@ _08001C48 — AgbMain idle-path call: refresh the +0xB0/+0xC0 work
@ area (4 KB CpuFastSet fill from +0xB0's head word; zero-fill when
@ countdown +0x08 hits 0, else decrement). When current mode is a
@ game-driven mode, publish +0x04 into +0xC0 and call
@ _08000848(st+0xB0, 0).
_08001C48:
	push {r4, lr}
	sub sp, #4
	ldr r4, _08001C78 @ =0x030000E4
	ldr r1, [r4]
	adds r0, r1, #0
	adds r0, #0xc0
	adds r1, #0xb0
	ldr r2, _08001C7C @ =0x04000004
	bl sub_0802D974
	ldr r1, [r4]
	ldrh r0, [r1, #8]
	movs r3, #8
	ldrsh r2, [r1, r3]
	cmp r2, #0
	bne.n _08001C84
	str r2, [sp, #0]
	adds r1, #0xc0
	ldr r2, _08001C80 @ =0x05000004
	mov r0, sp
	bl sub_0802D974
	b.n _08001C88
	movs r0, r0
	.align 2, 0
_08001C78: .4byte 0x030000E4
_08001C7C: .4byte 0x04000004
_08001C80: .4byte 0x05000004
_08001C84:
	subs r0, #1
	strh r0, [r1, #8]
_08001C88:
	ldr r4, _08001CB0 @ =0x030000E4
	ldr r0, [r4]
	ldrh r0, [r0, #0x10]
	bl _08001730
	cmp r0, #0
	beq.n _08001CA8
	ldr r0, [r4]
	ldrh r2, [r0, #4]
	adds r1, r0, #0
	adds r1, #0xc0
	strh r2, [r1, #0]
	adds r0, #0xb0
	movs r1, #0
	bl _08000848
_08001CA8:
	add sp, #4
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_08001CB0: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001CB4 -> u32 — value stored at st+0xD0.
@ Callers: 0x8001BBC4-region engines (save/menu).
_08001CB4:
	ldr r0, _08001CC0 @ =0x030000E4
	ldr r0, [r0]
	adds r0, #0xd0
	ldr r0, [r0]
	bx lr
	movs r0, r0
	.align 2, 0
_08001CC0: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001CC4 -> s16 — st+0xD0 halved (round toward zero).
_08001CC4:
	ldr r0, _08001CD4 @ =0x030000E4
	ldr r0, [r0]
	adds r0, #0xd0
	ldr r0, [r0]
	lsrs r1, r0, #31
	adds r0, r0, r1
	asrs r0, r0, #1
	bx lr
	.align 2, 0
_08001CD4: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001CD8 — AgbMain idle-path call: the "input/ticker" processor.
@ Clears +0xC; requires a game-driven mode. Reads session word
@ (_0800070C(st+0x30)) into +0xD8 and pumps the soft-IRQ queue
@ (_080006A4). Flag logic over +0xD8 bits: bit14/15 mismatch detection
@ bumps +0xE and broadcasts event 21 at wrap (60); bit15 gates extra
@ pump passes; bit9 latches +0x01; bits 1/2/4/8 each bump +0xC
@ (consumed by block B's matcher via _08002044).
_08001CD8:
	push {r4, r5, lr}
	ldr r4, _08001D5C @ =0x030000E4
	ldr r1, [r4]
	movs r0, #0
	strh r0, [r1, #0xc]
	ldrh r0, [r1, #0x10]
	bl _08001730
	cmp r0, #0
	bne.n _08001CEE
	b.n _08001E0A
_08001CEE:
	ldr r0, [r4]
	adds r0, #0x30
	bl _0800070C
	ldr r1, [r4]
	adds r1, #0xd8
	str r0, [r1]
	bl _080006A4
	ldr r1, [r4]
	ldrb r0, [r1, #2]
	cmp r0, #0
	beq.n _08001D66
	movs r3, #0
	adds r0, r1, #0
	adds r0, #0xd8
	ldr r2, [r0]
	movs r0, #0xc0
	lsls r0, r0, #6
	ands r0, r2
	cmp r0, #0
	bne.n _08001D36
	movs r0, #0x80
	lsls r0, r0, #8
	ands r0, r2
	cmp r0, #0
	beq.n _08001D32
	lsls r1, r2, #28
	lsrs r1, r1, #28
	lsls r0, r2, #20
	lsrs r0, r0, #28
	cmp r1, r0
	beq.n _08001D32
	movs r3, #1
_08001D32:
	cmp r3, #0
	beq.n _08001D60
_08001D36:
	ldr r0, _08001D5C @ =0x030000E4
	ldr r1, [r0]
	ldrh r0, [r1, #0xe]
	adds r0, #1
	strh r0, [r1, #0xe]
	lsls r0, r0, #16
	lsrs r0, r0, #16
	cmp r0, #60
	bls.n _08001D66
	movs r0, #0
	strh r0, [r1, #0xe]
	adds r0, r1, #0
	adds r0, #0xd8
	ldr r1, [r0]
	movs r0, #21
	movs r2, #0
	bl sub_08004D4C
	b.n _08001D66
	.align 2, 0
_08001D5C: .4byte 0x030000E4
_08001D60:
	ldr r0, _08001E10 @ =0x030000E4
	ldr r0, [r0]
	strh r3, [r0, #0xe]
_08001D66:
	ldr r4, _08001E10 @ =0x030000E4
	ldr r1, [r4]
	adds r0, r1, #0
	adds r0, #0xd8
	ldr r0, [r0]
	movs r5, #0x80
	ands r0, r5
	cmp r0, #0
	beq.n _08001D82
	ldrb r0, [r1, #1]
	cmp r0, #0
	bne.n _08001DB0
	bl _080006A4
_08001D82:
	ldr r1, [r4]
	ldrb r0, [r1, #1]
	cmp r0, #0
	bne.n _08001DB0
	adds r0, r1, #0
	adds r0, #0xd8
	ldr r0, [r0]
	ands r0, r5
	cmp r0, #0
	beq.n _08001D9A
	bl _080006A4
_08001D9A:
	ldr r2, [r4]
	adds r0, r2, #0
	adds r0, #0xd8
	ldr r0, [r0]
	movs r1, #0x80
	lsls r1, r1, #1
	ands r0, r1
	cmp r0, #0
	beq.n _08001DB0
	movs r0, #1
	strb r0, [r2, #1]
_08001DB0:
	ldr r3, _08001E10 @ =0x030000E4
	ldr r2, [r3]
	adds r0, r2, #0
	adds r0, #0xd8
	ldr r0, [r0]
	movs r1, #1
	ands r0, r1
	cmp r0, #0
	beq.n _08001DC8
	ldrh r0, [r2, #0xc]
	adds r0, #1
	strh r0, [r2, #0xc]
_08001DC8:
	ldr r2, [r3]
	adds r0, r2, #0
	adds r0, #0xd8
	ldr r0, [r0]
	movs r1, #2
	ands r0, r1
	cmp r0, #0
	beq.n _08001DDE
	ldrh r0, [r2, #0xc]
	adds r0, #1
	strh r0, [r2, #0xc]
_08001DDE:
	ldr r2, [r3]
	adds r0, r2, #0
	adds r0, #0xd8
	ldr r0, [r0]
	movs r1, #4
	ands r0, r1
	cmp r0, #0
	beq.n _08001DF4
	ldrh r0, [r2, #0xc]
	adds r0, #1
	strh r0, [r2, #0xc]
_08001DF4:
	ldr r2, [r3]
	adds r0, r2, #0
	adds r0, #0xd8
	ldr r0, [r0]
	movs r1, #8
	ands r0, r1
	cmp r0, #0
	beq.n _08001E0A
	ldrh r0, [r2, #0xc]
	adds r0, #1
	strh r0, [r2, #0xc]
_08001E0A:
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_08001E10: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001E14(idx) -> u32 — raw descriptor-table entry word
@ (0x0802E190[idx][sel], sel = st[+6]). Callers outside the window.
_08001E14:
	ldr r2, _08001E28 @ =0x0802E190
	lsls r0, r0, #2
	ldr r1, _08001E2C @ =0x030000E4
	ldr r1, [r1]
	ldrh r1, [r1, #6]
	lsls r1, r1, #4
	adds r0, r0, r1
	adds r0, r0, r2
	ldr r0, [r0]
	bx lr
	.align 2, 0
_08001E28: .4byte 0x0802E190
_08001E2C: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001E30 -> bool — true when st[+6] == 0 (normal select active).
_08001E30:
	movs r1, #0
	ldr r0, _08001E44 @ =0x030000E4
	ldr r0, [r0]
	ldrh r0, [r0, #6]
	cmp r0, #0
	bne.n _08001E3E
	movs r1, #1
_08001E3E:
	adds r0, r1, #0
	bx lr
	movs r0, r0
	.align 2, 0
_08001E44: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001E48(id, val) — write u16 payload into the +0xC0 event area at
@ slot id (offset (id+1)*2). The block-B dispatcher (_08002158) forwards
@ events here; AgbMain's mode handlers read them back.
@ Callers: 16 outside the window (engine-wide API).
_08001E48:
	ldr r2, _08001E58 @ =0x030000E4
	ldr r2, [r2]
	adds r0, #1
	lsls r0, r0, #1
	adds r2, #0xc0
	adds r2, r2, r0
	strh r1, [r2, #0]
	bx lr
	.align 2, 0
_08001E58: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001E5C(idx0, idx1) -> u16 — record halfword getter (current select).
@ Heavily used engine API (12 external callers).
_08001E5C:
	push {r4, r5, lr}
	ldr r4, _08001E84 @ =0x0802E190
	lsls r0, r0, #2
	ldr r2, _08001E88 @ =0x030000E4
	ldr r3, [r2]
	ldrh r5, [r3, #6]
	lsls r2, r5, #4
	adds r0, r0, r2
	adds r0, r0, r4
	ldr r0, [r0]
	adds r1, #1
	lsls r1, r1, #1
	lsls r0, r0, #4
	adds r1, r1, r0
	adds r3, #0x70
	adds r3, r3, r1
	ldrh r0, [r3]
	pop {r4, r5}
	pop {r1}
	bx r1
	.align 2, 0
_08001E84: .4byte 0x0802E190
_08001E88: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001E8C(rec_off, hw_idx) -> u16 — direct indexed halfword:
@ st[+0x70 + rec_off*16 + (hw_idx+1)*2]. Sibling of _08002178/_0800222C.
_08001E8C:
	ldr r2, _08001EA0 @ =0x030000E4
	ldr r2, [r2]
	adds r1, #1
	lsls r1, r1, #1
	lsls r0, r0, #4
	adds r1, r1, r0
	adds r2, #0x70
	adds r2, r2, r1
	ldrh r0, [r2]
	bx lr
	.align 2, 0
_08001EA0: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001EA4(hw_idx) -> u16 — st[+0x70 + (hw_idx+1)*2].
_08001EA4:
	ldr r1, _08001EB4 @ =0x030000E4
	ldr r1, [r1]
	adds r0, #1
	lsls r0, r0, #1
	adds r1, #0x70
	adds r1, r1, r0
	ldrh r0, [r1]
	bx lr
	.align 2, 0
_08001EB4: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001EB8 — decrement block A +0x08 countdown. A real entry with its own
@ `subs r0,#1; strh r0,[r1,#8]; bx lr` and 0x030000E4 pool (0x1eb8-0x1ec7),
@ and likewise invisible: nothing in asm/ `bl`s it, so `_08001EA4`'s 20-byte
@ body was scored against a 36-byte span running through it.
@ C owner: Idle_DecScratch08 (src/idle_dispatch.c).
	.type _08001EB8, %function
_08001EB8:
	ldr r1, _08001EC4 @ =0x030000E4
	ldr r1, [r1]
	subs r0, r0, #1
	strh r0, [r1, #8]
	bx lr
	movs r0, r0
	.align 2, 0
_08001EC4: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001EC8(arg_a, arg_b) — session setup (mode 2 side effects):
@ +0x12=2, [+0x2C]=arg_b, +0x16 = sub_0802DF6C(arg_b+11, 12) (fallback
@ 0 -> sub_080295C(0x0802D0E1, 0)), clear +0x18/+0x1C, [+0x24]=arg_a.
@ Callers: engine ctors (relocated region).
_08001EC8:
	push {r4, r5, r6, r7, lr}
	adds r5, r0, #0
	adds r0, r1, #0
	ldr r6, _08001F00 @ =0x030000E4
	ldr r4, [r6]
	movs r7, #0
	movs r1, #2
	strh r1, [r4, #0x12]
	str r0, [r4, #0x2c]
	adds r0, #11
	movs r1, #12
	bl sub_0802DF6C
	strh r0, [r4, #0x16]
	lsls r0, r0, #16
	cmp r0, #0
	bne.n _08001EF2
	ldr r0, _08001F04 @ =0x0802E1D0
	movs r1, #0
	bl sub_0800295C
_08001EF2:
	ldr r0, [r6]
	strh r7, [r0, #0x18]
	strh r7, [r0, #0x1c]
	str r5, [r0, #0x24]
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0
_08001F00: .4byte 0x030000E4
_08001F04: .4byte 0x0802E1D0

@ ----------------------------------------------------------------------------
@ _08001F08(arg) — mode-3 setup: +0x12=3, +0x1A=0, +0x16=0x1000,
@ [+0x20]=arg.
_08001F08:
	ldr r1, _08001F20 @ =0x030000E4
	ldr r2, [r1]
	movs r3, #0
	movs r1, #3
	strh r1, [r2, #0x12]
	strh r3, [r2, #0x1a]
	movs r1, #0x80
	lsls r1, r1, #5
	strh r1, [r2, #0x16]
	str r0, [r2, #0x20]
	bx lr
	movs r0, r0
	.align 2, 0
_08001F20: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001F24 / _08001F30 — thunks onto _08001F3C.
_08001F24:
	push {lr}
	bl _08001F3C
	pop {r1}
	bx r1
	movs r0, r0

.type _08001F30, %function
_08001F30:
	push {lr}
	bl _08001F3C
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08001F3C -> bool — current mode == 1.
@ Called from block B's frame processor (_080021EC).
	.type _08001F3C, %function
_08001F3C:
	movs r1, #0
	ldr r0, _08001F50 @ =0x030000E4
	ldr r0, [r0]
	ldrh r0, [r0, #0x10]
	cmp r0, #1
	bne.n _08001F4A
	movs r1, #1
_08001F4A:
	adds r0, r1, #0
	bx lr
	movs r0, r0
	.align 2, 0
_08001F50: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001F54(idx) -> pointer — record address:
@ st + 0x70 + [0x0802E190[idx][sel]] * 16.
	.type _08001F54, %function
_08001F54:
	push {r4, lr}
	ldr r3, _08001F78 @ =0x0802E190
	lsls r0, r0, #2
	ldr r1, _08001F7C @ =0x030000E4
	ldr r2, [r1]
	ldrh r4, [r2, #6]
	lsls r1, r4, #4
	adds r0, r0, r1
	adds r0, r0, r3
	ldr r0, [r0]
	lsls r0, r0, #4
	adds r0, #0x70
	adds r2, r2, r0
	adds r0, r2, #0
	pop {r4}
	pop {r1}
	bx r1
	movs r0, r0
	.align 2, 0
_08001F78: .4byte 0x0802E190
_08001F7C: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001F80(v) / _08001F8C — write/read block A +0x0A (record count).
.type _08001F80, %function
_08001F80:
	ldr r1, _08001F88 @ =0x030000E4
	ldr r1, [r1]
	strh r0, [r1, #0xa]
	bx lr
	.align 2, 0
_08001F88: .4byte 0x030000E4

_08001F8C:
	ldr r0, _08001F94 @ =0x030000E4
	ldr r0, [r0]
	ldrh r0, [r0, #0xa]
	bx lr
	.align 2, 0
_08001F94: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001F98(arg) — request mode 6 with +0x24 = arg (mode-8 hook arg
@ lives at +0x24; see commit processor). A real entry (0x1f98-0x1faf, with its
@ own `push {r4,lr}` / `pop {r4}; pop {r0}` epilogue and pool) and likewise
@ invisible -- nothing in asm/ `bl`s it -- so `_08001F8C`'s 12-byte body was
@ scored against a 40-byte span running through it.
	.type _08001F98, %function
_08001F98:
	push {r4, lr}
	adds r4, r0, #0
	movs r0, #6
	bl _08001724
	ldr r0, _08001FB0 @ =0x030000E4
	ldr r0, [r0]
	str r4, [r0, #0x24]
	pop {r4}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08001FB0: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001FB4(arg) — request mode 10 with +0x28 = arg.
_08001FB4:
	push {r4, lr}
	adds r4, r0, #0
	movs r0, #10
	bl _08001724
	ldr r0, _08001FCC @ =0x030000E4
	ldr r0, [r0]
	str r4, [r0, #0x28]
	pop {r4}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08001FCC: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08001FD0(m) -> bool — true unless m in {6..8} ("race modes").
@ _08001FE0(m) -> bool — true unless m in {10, 11} ("special modes").
_08001FD0:
	movs r1, #1
	cmp r0, #8
	bgt.n _08001FDC
	cmp r0, #6
	blt.n _08001FDC
	movs r1, #0
_08001FDC:
	adds r0, r1, #0
	bx lr

_08001FE0:
	movs r1, #1
	cmp r0, #11
	bgt.n _08001FEC
	cmp r0, #10
	blt.n _08001FEC
	movs r1, #0
_08001FEC:
	adds r0, r1, #0
	bx lr

@ ----------------------------------------------------------------------------
@ _08001FF0 -> bool — race-ish test over current AND requested mode.
@ _08002014 -> bool — special-mode test over both.
@ `_08001FF0` is bl-called from C only, so like its neighbours it had a bare
@ label and no `.type`; the inventory admits nothing else, and `_08001FE0`'s
@ 16-byte body was measured against a 52-byte span that swallowed it and its
@ pool word. `.type` emits no bytes.
	.type _08001FF0, %function
_08001FF0:
	push {r4, r5, lr}
	ldr r5, _08002010 @ =0x030000E4
	ldr r0, [r5]
	ldrh r0, [r0, #0x10]
	bl _08001FD0
	adds r4, r0, #0
	ldr r0, [r5]
	ldrh r0, [r0, #0x12]
	bl _08001FD0
	orrs r4, r0
	adds r0, r4, #0
	pop {r4, r5}
	pop {r1}
	bx r1
	.align 2, 0
_08002010: .4byte 0x030000E4

_08002014:
	push {r4, r5, lr}
	ldr r5, _08002034 @ =0x030000E4
	ldr r0, [r5]
	ldrh r0, [r0, #0x10]
	bl _08001FE0
	adds r4, r0, #0
	ldr r0, [r5]
	ldrh r0, [r0, #0x12]
	bl _08001FE0
	orrs r4, r0
	adds r0, r4, #0
	pop {r4, r5}
	pop {r1}
	bx r1
	.align 2, 0
_08002034: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _08002038 -> u16 — current mode.          _08002044 -> u16 — +0x0C.
@ _08002050 -> u16 — +0xD8 parsed word.     _08002060(f) — set +0x02 flag.
@ A FUNCTION ENTRY, and `asm_vmas` only admits a bare label with hard
@ evidence -- a `.type` line or an inbound `bl`. Nothing in asm/ branches here,
@ so it was absent from the inventory, and `_08002014`'s span ran on to
@ `_08002044` instead: 36 real bytes scored 36/48. The body is a complete
@ `bx lr` leaf with its own pool word, and src/idle_accessors.c:183 already
@ aliases it, so the C has always known about it. Declaring it restores the
@ 36-byte span. This file has 146 labels and only 10 inbound `bl`s, so the
@ same gap almost certainly hides more entries -- see the census in
@ docs/matching_workflow.md.
	.type _08002038, %function
_08002038:
	ldr r0, _08002040 @ =0x030000E4
	ldr r0, [r0]
	ldrh r0, [r0, #0x10]
	bx lr
	.align 2, 0
_08002040: .4byte 0x030000E4

_08002044:
	ldr r0, _0800204C @ =0x030000E4
	ldr r0, [r0]
	ldrh r0, [r0, #0xc]
	bx lr
	.align 2, 0
_0800204C: .4byte 0x030000E4

@ Same gap one entry further on. `_08002044` is 12 bytes (0x2044-0x204f:
@ ldr/ldr/ldrh/bx lr plus its own 0x030000E4 pool) and the ROM has a real
@ function boundary at 0x08002050, but nothing in asm/ `bl`s it and it carried
@ no `.type`, so the inventory never saw it and `_08002044`'s span swallowed
@ it: 12 candidate bytes against a 28-byte span. `.type` emits no bytes, so
@ `_08002044` drops to its true 12-byte extent and `_08002050` gets a 16-byte
@ span of its own (0x2050-0x205f, complete with its `movs r0,r0` align pad).
@ Both entries already have C owners in src/idle_accessors.c
@ (Idle_GetCounterC / Idle_GetD8Word, aliased at lines 229-230).
	.type _08002050, %function
_08002050:
	ldr r0, _0800205C @ =0x030000E4
	ldr r0, [r0]
	adds r0, #0xd8
	ldrh r0, [r0]
	bx lr
	movs r0, r0
	.align 2, 0
_0800205C: .4byte 0x030000E4

_08002060:
	ldr r1, _08002068 @ =0x030000E4
	ldr r1, [r1]
	strb r0, [r1, #2]
	bx lr
	.align 2, 0
_08002068: .4byte 0x030000E4

@ ----------------------------------------------------------------------------
@ _0800206C(idx) -> u16 — u16 form of _080015F4 (block B matcher uses
@ this to compare candidate records).
_0800206C:
	push {lr}
	bl _080015F4
	lsls r0, r0, #16
	lsrs r0, r0, #16
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _0800207C(dst_ptr, src, len) — arena bookkeeping helper: CpuFastSet
@ zero-fill of src for len-derived word count (ctrl 0x02800000), then
@ record src/len into the 2-word handle at dst_ptr.
@ Caller: save/menu engine.
_0800207C:
	push {r4, r5, r6, lr}
	sub sp, #4
	adds r6, r0, #0
	adds r5, r1, #0
	adds r4, r2, #0
	movs r0, #0
	str r0, [sp, #0]
	cmp r4, #0
	bge.n _08002090
	adds r2, r4, #3
_08002090:
	lsls r2, r2, #9
	lsrs r2, r2, #11
	movs r0, #160
	lsls r0, r0, #19
	orrs r2, r0
	mov r0, sp
	adds r1, r5, #0
	bl sub_0802D974
	str r5, [r6, #0]
	str r4, [r6, #4]
	add sp, #4
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _080020B0(handle, amt) -> old_head — consume amt bytes from a 2-word
@ arena handle ([0]=head, [4]=remaining); warns via
@ sub_080295C(0x0802E1DC, deficit) on underrun. Caller: menu/save.
_080020B0:
	push {r4, r5, r6, lr}
	adds r4, r0, #0
	adds r5, r1, #0
	ldr r6, [r4]
	cmp r5, #0
	bne.n _080020C0
	movs r0, #0
	b.n _080020DC
_080020C0:
	ldr r1, [r4, #4]
	cmp r1, r5
	bge.n _080020CE
	ldr r0, _080020E4 @ =0x0802E1DC
	subs r1, r5, r1
	bl sub_0800295C
_080020CE:
	ldr r0, [r4]
	adds r0, r0, r5
	str r0, [r4]
	ldr r0, [r4, #4]
	subs r0, r0, r5
	str r0, [r4, #4]
	adds r0, r6, #0
_080020DC:
	pop {r4, r5, r6}
	pop {r1}
	bx r1
	.align 2, 0
_080020E4: .4byte 0x0802E1DC

@ End-of-region anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x08001988-0x080020e8) and has no `.include`; the promoted body at
@ 0x080020B0 ends on 0x080020e8, exactly that boundary, so the anchor is safe.
@ Without it the body is refused as 'no end marker in asm/idle.s'.
idle_end:
