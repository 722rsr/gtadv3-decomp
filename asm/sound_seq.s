@ GT Advance 3 - MTO sound driver: stream-bank command walkers
@ Region: file offset 0x02B66C-0x02B7B4 (VMA 0x0802B66C-0x0802B7B4).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/sound_api.s
@
@ Song directory ROM 0x08061FA4 (stride 8): entry+4 u16 gate = index
@ into the bank-descriptor table below. Table ROM 0x08061F74 (stride
@ 12), filled at runtime by reset sub_0802C4C4:
@   idx 0: state 0x0203ED40, channels 0x0203E000, nch 4
@   idx 1: state 0x0203ED80, channels 0x0203E140, nch 1
@   idx 2: state 0x0203EDC0, channels 0x0203E190, nch 1
@   idx 3: state 0x0203EE10, channels 0x0203E1E0, nch 1
@ Handlers iterate channels (stride 0x50) of the selected song's state
@ block, applying updates to channels whose select bitmask (arg1) has
@ the corresponding bit set AND whose flag byte [+0] has bit7 set.
@ Each channel handler guards with the 0x68736D53 ("Smsh") lock at state+0x34:
@ apply only when unlocked, set magic+1 during iteration, restore at end.

.thumb

@ ----------------------------------------------------------------------------
@ _0802B66C(id, selmask, w_pan, w_vol, w_x) — combined multi-op applier.
@ No static BL callers (runtime-dispatched).
@ w_pan != 0            -> sub_0802D510(state, selmask, w_pan)
@ w_vol != 0x100        -> sub_0802D4A8(state, selmask, w_vol)
@ w_x   != 0            -> sub_0802D584(state, selmask, (s8)w_x)
@ Entry labels for the two runtime-dispatched sequence bodies below: reached by
@ table dispatch, not by `bl`, so neither had `.type %function`, both were
@ invisible, and each inflated its predecessor's span. Zero bytes each.
	.type _0802B66C, %function
_0802B66C:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	ldr r4, [sp, #32]     @ stack arg w_x
	lsls r0, r0, #16
	lsrs r5, r0, #16      @ id
	mov r8, r5
	lsls r1, r1, #16
	lsrs r6, r1, #16      @ selmask
	mov r9, r6
	lsls r3, r3, #16
	lsrs r3, r3, #16      @ w_vol
	mov sl, r3
	lsls r4, r4, #16
	lsrs r4, r4, #16      @ w_x
	lsls r2, r2, #16
	asrs r3, r2, #16      @ (s16)w_pan
	cmp r3, #0
	beq _0802B6B4
	ldr r2, _0802B66C_lit_tab   @ =0x08061F74
	ldr r0, _0802B66C_lit_dir   @ =0x08061FA4
	lsls r1, r5, #3
	adds r1, r1, r0
	ldrh r7, [r1, #4]     @ gate idx
	lsls r0, r7, #1
	adds r1, r7, #0
	adds r0, r0, r1
	lsls r0, r0, #2       @ idx*12
	adds r0, r0, r2
	ldr r0, [r0, #0]      @ state block
	adds r1, r6, #0
	adds r2, r3, #0
	bl sub_0802D510       @ pan-style op
_0802B6B4:
	mov r0, sl
	lsls r3, r0, #16
	movs r0, #128
	lsls r0, r0, #17      @ 0x01000000
	cmp r3, r0            @ w_vol == 0x100 ?
	beq _0802B6DC
	ldr r2, _0802B66C_lit_tab
	ldr r0, _0802B66C_lit_dir
	lsls r1, r5, #3
	adds r1, r1, r0
	ldrh r5, [r1, #4]
	lsls r0, r5, #1
	adds r0, r0, r5
	lsls r0, r0, #2
	adds r0, r0, r2
	ldr r0, [r0, #0]
	adds r1, r6, #0
	lsrs r2, r3, #16
	bl sub_0802D4A8       @ volume op (vol>>2)
_0802B6DC:
	cmp r4, #0
	beq _0802B700
	ldr r2, _0802B66C_lit_tab
	ldr r0, _0802B66C_lit_dir
	mov r7, r8
	lsls r1, r7, #3
	adds r1, r1, r0
	ldrh r3, [r1, #4]
	lsls r0, r3, #1
	adds r0, r0, r3
	lsls r0, r0, #2
	adds r0, r0, r2
	ldr r0, [r0, #0]
	mov r1, r9
	lsls r2, r4, #24
	asrs r2, r2, #24      @ (s8)w_x
	bl sub_0802D584       @ third op
_0802B700:
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.hword 0
_0802B66C_lit_tab:
	.word 0x08061F74
_0802B66C_lit_dir:
	.word 0x08061FA4

@ ----------------------------------------------------------------------------
@ sub_0802B718(id, selmask, val) — VOLUME walker -> sub_0802D4A8.
@ Writes val>>2 to each selected active channel +0x13, sets flag |=3.
@ Static callers: 0x0802B11C/0x0802B160/0x0802B174 (sound_api fade
@ engine re-apply), 0x0802B4CE (sound_bank _0802B488).
sub_0802B718:
_0802B718:
	push {r4, r5, lr}
	lsls r0, r0, #16
	ldr r4, _0802B718_lit_tab   @ =0x08061F74
	ldr r3, _0802B718_lit_dir   @ =0x08061FA4
	lsrs r0, r0, #13      @ id*8
	adds r0, r0, r3
	ldrh r5, [r0, #4]     @ gate idx
	lsls r3, r5, #1
	adds r3, r3, r5
	lsls r3, r3, #2
	adds r3, r3, r4
	ldr r0, [r3, #0]      @ state block
	lsls r1, r1, #16
	lsrs r1, r1, #16      @ val
	lsls r2, r2, #16
	lsrs r2, r2, #16      @ chsel
	bl sub_0802D4A8
	pop {r4, r5}
	pop {r0}
	bx r0
	.hword 0
_0802B718_lit_tab:
	.word 0x08061F74
_0802B718_lit_dir:
	.word 0x08061FA4

@ ----------------------------------------------------------------------------
@ sub_0802B74C(id, selmask, val) — PAN walker -> sub_0802D510.
@ Writes (s8)val to channel +0x0B and val to +0x0D, sets flag |=0xC.
@ Static callers: 0x0802B182 (sound_api), 0x0802B4EE (sound_bank).
sub_0802B74C:
_0802B74C:
	push {r4, r5, lr}
	lsls r0, r0, #16
	ldr r4, _0802B74C_lit_tab
	ldr r3, _0802B74C_lit_dir
	lsrs r0, r0, #13
	adds r0, r0, r3
	ldrh r5, [r0, #4]
	lsls r3, r5, #1
	adds r3, r3, r5
	lsls r3, r3, #2
	adds r3, r3, r4
	ldr r0, [r3, #0]
	lsls r1, r1, #16
	lsrs r1, r1, #16
	lsls r2, r2, #16
	asrs r2, r2, #16
	bl sub_0802D510
	pop {r4, r5}
	pop {r0}
	bx r0
	.hword 0
_0802B74C_lit_tab:
	.word 0x08061F74
_0802B74C_lit_dir:
	.word 0x08061FA4

@ ----------------------------------------------------------------------------
@ _0802B780(id, selmask, val) — third-op walker -> sub_0802D584.
@ Writes (u8)val to channel +0x15, sets flag |=3. No static BL callers.
	.type _0802B780, %function
_0802B780:
	push {r4, r5, lr}
	lsls r0, r0, #16
	ldr r4, _0802B780_lit_tab
	ldr r3, _0802B780_lit_dir
	lsrs r0, r0, #13
	adds r0, r0, r3
	ldrh r5, [r0, #4]
	lsls r3, r5, #1
	adds r3, r3, r5
	lsls r3, r3, #2
	adds r3, r3, r4
	ldr r0, [r3, #0]
	lsls r1, r1, #16
	lsrs r1, r1, #16
	lsls r2, r2, #24
	asrs r2, r2, #24
	bl sub_0802D584
	pop {r4, r5}
	pop {r0}
	bx r0
	.hword 0
_0802B780_lit_tab:
	.word 0x08061F74
_0802B780_lit_dir:
	.word 0x08061FA4

@ Synthetic end anchor at the region boundary 0x0802B7B4.
sound_seq_end:
