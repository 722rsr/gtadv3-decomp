@ GT Advance 3 - award-tier UI packet builder (post-race progress cluster tail).
@ Region: file offset 0x00B0BC-0x00B190 (VMA 0x0800B0BC-0x0800B190), one whole
@ function: prologue push{r4-r7,lr}/mov r7,r8/push{r7}/sub sp,#56 at 0x0800B0BC,
@ epilogue add sp,#56 / pop{r3}->r8 / pop{r4-r7} / pop{r0} / bx r0 ending at
@ 0x0800B190 (= start of the raw region owned by the next task). Embedded
@ private literal pool kept in place mid-function at 0x0800B15C-0x0800B167
@ ({wa, 0x574, 0xFF2}); both over-pool branches preserved.
@
@ Splice safety: xref.py finds exactly two BL callers - bl @0x0800AC04 inside
@ converted ai_racefsm.s (posts step/event id 49 via _08004CD4, then calls),
@ and raw bl @0x01866E (after sound cmd _0802B368(1) + strh 1, see the
@ carphys_tick.s note on the _0802B368+_0800B0BC sequence). Zero external
@ literal refs into the span as function pointers: full-ROM scan hits are two
@ data coincidences (lit-shaped word @0x039DA4 = 0x0800B165 points into this
@ function's own pool mid-word from a coordinate table; word @0x65a5b4 =
@ 0x0800B0CD lands mid-instruction inside an asset blob). All branches and
@ all four ldr-literal references are internal.
@
@ Semantics: sibling of the type-9 packet builder _08009B60 (menu_pkt.s).
@ Builds a 56-byte stack packet and hands it to the shared UI packet consumer
@ _080188B0 (raw region):
@   +0   u16 10                       packet type 10
@   +24  u16  u16[wa+0x574]           garage-record cursor idx
@   +26  s16  sxt8(u8[wa+idx*12+0x30]) field [0] of the cursor 12-byte
@                                     garage record
@   +28..39     12-byte copy of the whole cursor garage record body
@               (ldmia/stmia {r2,r3,r4})
@   +23  u8   award tier byte: layer = s16[wa+0xFF2];
@             layer == 0 -> cnt0 > 43 ? 1 : 0
@             layer != 0 -> cnt1 > 43 ? 3 : 2
@             where cntN counts cells == 3 over rows 0..3 x cols 0..10 of
@             zone grid type N via sub_08025CF4(type,row,col) - the same
@             44-cell layers and >43 threshold the award grants in
@             _0800B4A8 (ai_collect.s / asm/ai_collect.s SS3e).

.thumb

@ ----------------------------------------------------------------------------
@ sub_0800B0BC - post type-10 UI packet with garage record + award tier.
sub_0800B0BC:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	sub sp, #56			@ 56-byte packet frame
	mov r1, sp
	movs r0, #10			@ pkt[0] = 10
	strh r0, [r1, #0]
	ldr r3, lit_0800B15C		@ wa = 0x03001780
	ldr r0, lit_0800B160		@ 0x574
	adds r2, r3, r0			@ &u16[wa+0x574]
	ldrh r0, [r2, #0]
	strh r0, [r1, #24]		@ pkt[+24] = cursor idx
	mov r4, sp
	movs r0, #0
	ldrsh r1, [r2, r0]		@ idx = s16[wa+0x574]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2			@ idx * 12
	adds r0, r0, r3
	adds r0, #48			@ wa + idx*12 + 0x30
	ldrb r0, [r0, #0]
	lsls r0, r0, #24
	asrs r0, r0, #24		@ sign-extend field [0]
	strh r0, [r4, #26]		@ pkt[+26]
	movs r4, #0
	ldrsh r1, [r2, r4]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2			@ idx * 12
	adds r0, r0, r3
	add r1, sp, #28
	adds r0, #48			@ wa + idx*12 + 0x30
	ldmia r0!, {r2, r3, r4}		@ copy the 12-byte garage record
	stmia r1!, {r2, r3, r4}		@ body -> pkt[+28..+39]
	movs r0, #0
	mov r8, r0			@ cnt0 (grid type 0)
	movs r7, #0			@ cnt1 (grid type 1)
	movs r5, #0			@ row
_award_row_loop:			@ 0x0800B108
	movs r4, #0
	adds r6, r5, #1
_award_col_loop:			@ 0x0800B10C
	movs r0, #0			@ grid type 0
	adds r1, r5, #0
	adds r2, r4, #0
	bl sub_08025CF4
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #3
	bne _award_skip0
	movs r1, #1
	add r8, r1
_award_skip0:
	movs r0, #1			@ grid type 1
	adds r1, r5, #0
	adds r2, r4, #0
	bl sub_08025CF4
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #3
	bne _award_skip1
	adds r7, #1
_award_skip1:
	adds r4, #1
	cmp r4, #10
	ble _award_col_loop
	adds r5, r6, #0
	cmp r5, #3
	ble _award_row_loop
	ldr r0, lit_0800B15C		@ wa
	ldr r2, lit_0800B164		@ 0xFF2
	adds r0, r0, r2			@ &s16[wa+0xFF2] (current layer)
	movs r3, #0
	ldrsh r1, [r0, r3]
	cmp r1, #0
	bne _award_layer_nz
	mov r4, r8			@ cnt0
	cmp r4, #43
	ble _award_tier_zero
	mov r1, sp
	movs r0, #1			@ layer 0 complete-ish -> tier 1
	b _award_store_tier
	.align 2, 0
lit_0800B15C:
	.word 0x03001780		@ work-area base (wa)
lit_0800B160:
	.word 0x00000574		@ garage-record cursor u16[wa+0x574]
lit_0800B164:
	.word 0x00000FF2		@ current award-layer s16[wa+0xFF2]
_award_tier_zero:			@ 0x0800B168
	mov r0, sp
	strb r1, [r0, #23]		@ pkt[+23] = 0
	b _award_send
_award_layer_nz:			@ 0x0800B16E
	cmp r7, #43
	ble _award_tier_two
	mov r1, sp
	movs r0, #3			@ layer 1 all-3 -> tier 3
	b _award_store_tier
_award_tier_two:
	mov r1, sp
	movs r0, #2			@ otherwise tier 2
_award_store_tier:			@ 0x0800B17C
	strb r0, [r1, #23]
_award_send:				@ 0x0800B17E
	mov r0, sp
	bl 0x080188B0			@ raw region: shared UI packet consumer
	add sp, #56
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0				@ armcc interworking return

@ end of pocket: next VMA 0x0800B190 (raw region owned by the follow-up task)
