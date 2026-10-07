@ GT Advance 3 - race frame state machine (per-frame race/AI control FSM)
@ Region: file offset 0x00AA40-0x00AD84 (VMA 0x0800AA40-0x0800AD84).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/ai_collect.s.
@
@ _0800AA40 runs every frame while a race scene is resident (reached via
@ the manager tick dispatch, src/foundation_subsys.c). Each
@ pass it:
@   1. reads car-index globals u16[0x03002772]/u16[0x03002778] and the
@      per-car halfword records at 0x0300288A/AA/CA (stride 10) through
@      accessors _080026F8/_08002730,
@   2. computes two derived values (_0800A9E0 min-of-grid, _0800A9A0),
@   3. resets the scene event ring (_08023FE4, see ai_raceevt.s),
@   4. switches on the global race phase u16[0x0300273C] (work-area base
@      0x03001780 + 0xFBC), 8-way table at _0800AAFC:
@      case 0 _0800AB1C: item-box spawner tick (_0800B4A8) + place-change
@                        events (38/43/49) + finish bookkeeping,
@      case 1 _0800AC2E: countdown phase (events 21/25/35, _08024144),
@      case 2 _0800AC90: GO!/start (_0800B89C + events),
@      case 3 _0800ACC0: running (event 21 only),
@      case 5 _0800AC68: lap/progress check via _0802581C/_08025E1C/
@                        _08025DBC then events 21/35 + _0800B9F0,
@      case 7 _0800ACCA: results transition (events 21/25/35/_08024144),
@      cases 4/6 -> shared tail.

.thumb

@ ----------------------------------------------------------------------------
.type _0800AA40, %function
_0800AA40:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	sub sp, #12
	ldr r4, _0800AADC_lit @ =0x03001780
	ldr r0, _0800AAE0_lit @ =0x00000FF2
	adds r6, r4, r0       @ r6 = 0x03002772 (car index global A)
	movs r1, #0
	ldrsh r0, [r6, r1]
	lsls r2, r0, #3
	ldr r3, _0800AAE4_lit @ =0x00000FF8
	adds r5, r4, r3       @ r5 = 0x03002778 (car index global B)
	movs r7, #0
	ldrsh r1, [r5, r7]
	lsls r3, r1, #1
	adds r3, r3, r2
	ldr r7, _0800AAE8_lit @ =0x0000100A
	adds r2, r4, r7       @ record array base 0x0300288A
	adds r2, r2, r3
	movs r7, #0
	ldrsh r2, [r2, r7]
	str r2, [sp, #8]      @ slot field for selected car
	ldr r2, _0800AAEC_lit @ =0x0000102A
	adds r2, r2, r4       @ 0x030027AA
	mov r8, r2
	add r8, r3
	mov r7, r8
	movs r2, #0
	ldrsh r7, [r7, r2]
	mov r8, r7            @ r8 = [0x030027AA + idx*10]
	ldr r7, _0800AAF0_lit @ =0x0000101A
	adds r7, r7, r4       @ 0x0300279A
	mov r9, r7
	add r9, r3
	mov r2, r9
	movs r3, #0
	ldrsh r2, [r2, r3]
	mov r9, r2            @ r9 = [0x0300279A + idx*10]
	bl sub_080026F8       @ get car halfword field set A
	str r0, [sp, #0]
	movs r7, #0
	ldrsh r0, [r6, r7]
	movs r2, #0
	ldrsh r1, [r5, r2]
	bl sub_08002730       @ get car halfword field set B
	mov sl, r0
	movs r3, #0
	ldrsh r0, [r6, r3]
	movs r7, #0
	ldrsh r1, [r5, r7]
	bl sub_0800A9E0       @ min-of-4x11 grid query for (a,b)
	str r0, [sp, #4]
	movs r1, #0
	ldrsh r0, [r6, r1]
	movs r2, #0
	ldrsh r1, [r5, r2]
	bl sub_0800A9A0       @ second grid aggregate for (a,b)
	adds r5, r0, #0
	bl _08023FE4          @ reset scene event ring (ai_raceevt.s)
	ldr r3, _0800AAF4_lit @ =0x00000FBC
	adds r4, r4, r3       @ r4 = 0x0300273C race phase
	movs r7, #0
	ldrsh r0, [r4, r7]
	cmp r0, #7
	bls _0800AAD2
	b _0800AD70           @ out of range -> shared tail
_0800AAD2:
	lsls r0, r0, #2
	ldr r1, _0800AAF8_lit @ =_0800AAFC (jump table base)
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	.align 2, 0

@ literal pool of _0800AA40 prologue
_0800AADC_lit:
	.word 0x03001780
_0800AAE0_lit:
	.word 0x00000FF2
_0800AAE4_lit:
	.word 0x00000FF8
_0800AAE8_lit:
	.word 0x0000100A
_0800AAEC_lit:
	.word 0x0000102A
_0800AAF0_lit:
	.word 0x0000101A
_0800AAF4_lit:
	.word 0x00000FBC

@ phase jump table; base word below points here (entries = case handlers)
_0800AAF8_lit:
	.word _0800AAFC
_0800AAFC:
	.word _0800AB1C @ case 0
	.word _0800AC2E @ case 1
	.word _0800AC90 @ case 2
	.word _0800ACC0 @ case 3
	.word _0800AD70 @ case 4 (tail)
	.word _0800AC68 @ case 5
	.word _0800AD70 @ case 6 (tail)
	.word _0800ACCA @ case 7

@ ----------------------------------------------------------------------------
@ case 0: collection manager + place change + award checks
_0800AB1C:
	movs r0, #21
	movs r1, #0
	bl _08023FF8         @ fire scene event 21
	ldr r1, _0800AB74_lit @ =0x03001780
	ldr r2, _0800AB78_lit @ =0x000010E5
	adds r0, r1, r2      @ byte 0x03002865
	ldrb r0, [r0, #0]
	subs r0, #1
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #2
	bls _0800AB38
	b _0800AD32
_0800AB38:
	ldr r3, [sp, #8]
	cmp r3, #10
	bne _0800AB50
	ldr r7, _0800AB7C_lit @ =0x00000FF8
	adds r0, r1, r7
	ldrh r0, [r0, #0]
	cmp r0, #3
	bne _0800AB50
	ldr r0, _0800AB80_lit @ =0x00001074
	adds r1, r1, r0
	movs r0, #1
	strh r0, [r1, #0]
_0800AB50:
	bl _0800B4A8         @ collection manager tick
	ldr r1, [sp, #8]
	cmp r1, #2
	bne _0800AB60
	mov r2, r8
	cmp r2, #0
	beq _0800AB6A
_0800AB60:
	cmp r5, sl
	ble _0800AB84
	mov r3, r8
	cmp r3, #1
	bne _0800AB84
_0800AB6A:
	movs r0, #43
	movs r1, #0
	bl _08023FF8         @ event 43 (place changed)
	b _0800AC10
	.align 2, 0
_0800AB74_lit:
	.word 0x03001780
_0800AB78_lit:
	.word 0x000010E5
_0800AB7C_lit:
	.word 0x00000FF8
_0800AB80_lit:
	.word 0x00001074

@ (fallthrough target from case 0 when no place change)
_0800AB84:
	ldr r7, [sp, #8]
	cmp r7, #10
	bne _0800AB90
	mov r0, r9
	cmp r0, #0
	beq _0800ABA0
_0800AB90:
	ldr r0, _0800ABC0_lit @ =0x03001780
	ldr r1, [sp, #4]
	ldr r2, [sp, #0]
	cmp r1, r2
	ble _0800ABE8
	mov r3, r9
	cmp r3, #1
	bne _0800ABE8
_0800ABA0:
	ldr r0, _0800ABC0_lit @ =0x03001780
	ldr r7, _0800ABC4_lit @ =0x00001074
	adds r2, r0, r7
	movs r3, #0
	ldrsh r1, [r2, r3]
	cmp r1, #0
	bne _0800ABE8
	subs r7, #124        @ -> 0x00000FF8
	adds r0, r0, r7
	ldrh r0, [r0, #0]
	cmp r0, #3
	bne _0800ABC8
	movs r0, #1
	strh r0, [r2, #0]
	b _0800ABF2
	.align 2, 0
_0800ABC0_lit:
	.word 0x03001780
_0800ABC4_lit:
	.word 0x00001074

@ finish/lap event broadcast (shared by both branches above)
_0800ABC8:
	movs r0, #38
	movs r1, #0
	bl _08023FF8         @ event 38
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #31
	bl sub_08004CD4
	b _0800AD70
_0800ABE8:
	ldr r1, _0800AC0C_lit @ =0x00001074
	adds r0, r0, r1
	ldrh r0, [r0, #0]
	cmp r0, #1
	bne _0800AC10
_0800ABF2:
	movs r0, #38
	movs r1, #0
	bl _08023FF8
	bl sub_08004CC4
	movs r0, #49
	bl sub_08004CD4
	bl sub_0800B0BC
	b _0800AD70
	.align 2, 0
_0800AC0C_lit:
	.word 0x00001074

@ common exit of case 0: broadcast 13/14/31/15 then tail
_0800AC10:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #31
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	b _0800AD70

@ ----------------------------------------------------------------------------
@ case 1: countdown phase
_0800AC2E:
	movs r0, #21
	movs r1, #0
	bl _08023FF8
	ldr r0, _0800AC60_lit @ =0x03001780
	ldr r2, _0800AC64_lit @ =0x00000FC8
	adds r0, r0, r2      @ u16 0x03002748
	ldrh r0, [r0, #0]
	cmp r0, #1
	bne _0800AC50
	movs r0, #1
	bl sub_0800279C
	movs r0, #25
	movs r1, #0
	bl _08023FF8
_0800AC50:
	movs r0, #35
	movs r1, #0
	bl _08023FF8
	bl sub_08024144
	b _0800AD70
	.align 2, 0
_0800AC60_lit:
	.word 0x03001780
_0800AC64_lit:
	.word 0x00000FC8

@ ----------------------------------------------------------------------------
@ case 5: lap / zone-progress check
_0800AC68:
	ldr r1, _0800AC84_lit @ =0x03001780
	ldr r3, _0800AC88_lit @ =0x000010E5
	adds r5, r1, r3      @ byte 0x03002865
	ldrb r0, [r5, #0]
	subs r0, #1
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #2
	bhi _0800AD26
	ldr r7, _0800AC8C_lit @ =0x00000576
	adds r0, r1, r7      @ u16 0x03001CF6? (base+0x576)
	movs r1, #0
	ldrsh r0, [r0, r1]
	b _0800AD00
	.align 2, 0
_0800AC84_lit:
	.word 0x03001780
_0800AC88_lit:
	.word 0x000010E5
_0800AC8C_lit:
	.word 0x00000576

@ ----------------------------------------------------------------------------
@ case 2: GO! / race start
_0800AC90:
	movs r0, #21
	movs r1, #0
	bl _08023FF8
	bl sub_0800B89C
	ldr r0, _0800ACB8_lit @ =0x03001780
	ldr r2, _0800ACBC_lit @ =0x00000FC8
	adds r0, r0, r2
	ldrh r0, [r0, #0]
	cmp r0, #1
	bne _0800AD32
	movs r0, #1
	bl sub_0800279C
	movs r0, #25
	movs r1, #0
	bl _08023FF8
	b _0800AD32
	.align 2, 0
_0800ACB8_lit:
	.word 0x03001780
_0800ACBC_lit:
	.word 0x00000FC8

@ ----------------------------------------------------------------------------
@ case 3: race running heartbeat
_0800ACC0:
	movs r0, #21
	movs r1, #0
	bl _08023FF8
	b _0800AD32

@ ----------------------------------------------------------------------------
@ case 7: results / post-race transition
_0800ACCA:
	ldr r4, _0800ACE0_lit @ =0x03001780
	ldr r3, _0800ACE4_lit @ =0x00001078
	adds r0, r4, r3      @ u16 0x030027F8
	movs r7, #0
	ldrsh r0, [r0, r7]
	cmp r0, #1
	beq _0800ACE8
	cmp r0, #2
	beq _0800AD44
	b _0800AD70
	.align 2, 0
_0800ACE0_lit:
	.word 0x03001780
_0800ACE4_lit:
	.word 0x00001078

@ sub-case value==1: zone occupancy check for tracked car
_0800ACE8:
	ldr r0, _0800AD3C_lit @ =0x000010E5
	adds r5, r4, r0      @ byte 0x03002865
	ldrb r0, [r5, #0]
	subs r0, #1
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #2
	bhi _0800AD26
	ldr r1, _0800AD40_lit @ =0x00000576
	adds r0, r4, r1
	movs r2, #0
	ldrsh r0, [r0, r2]
_0800AD00:
	bl sub_0802581C
	lsls r0, r0, #16
	asrs r4, r0, #16
	adds r0, r4, #0
	bl sub_08025E1C
	lsls r0, r0, #24
	lsrs r0, r0, #24
	movs r1, #0
	ldrsb r1, [r5, r1]
	cmp r0, r1
	bge _0800AD26
	adds r0, r4, #0
	bl sub_08025DBC
	movs r0, #1
	bl sub_0800279C
_0800AD26:
	movs r0, #21
	movs r1, #0
	bl _08023FF8
	bl sub_0800B9F0
_0800AD32:
	movs r0, #35
	movs r1, #0
	bl _08023FF8
	b _0800AD70
	.align 2, 0
_0800AD3C_lit:
	.word 0x000010E5
_0800AD40_lit:
	.word 0x00000576

@ sub-case value==2
_0800AD44:
	movs r0, #21
	movs r1, #0
	bl _08023FF8
	ldr r3, _0800AD80_lit @ =0x00000FC8
	adds r0, r4, r3
	ldrh r0, [r0, #0]
	cmp r0, #1
	bne _0800AD64
	movs r0, #1
	bl sub_0800279C
	movs r0, #25
	movs r1, #0
	bl _08023FF8
_0800AD64:
	movs r0, #35
	movs r1, #0
	bl _08023FF8
	bl sub_08024144
_0800AD70:
	add sp, #12
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0
_0800AD80_lit:
	.word 0x00000FC8
