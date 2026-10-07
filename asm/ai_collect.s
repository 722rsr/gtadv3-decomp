@ GT Advance 3 - car-collection manager tick (grant/award logic)
@ Region: file offset 0x00B4A8-0x00B82C (VMA 0x0800B4A8-0x0800B82C).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@
@ Runs every frame from race-FSM case 0 (asm/ai_racefsm.s _0800AB1C).
@ NOT an item-box system: GT Advance 3 has no on-track pickups. The ids
@ here are CAR COLLECTION ids (0..98; ~97 real cars + specials 97/98):
@   presence bitmask  u8[13] at 0x030017A0   (_08025F78 set/_08025FAC test)
@   respawn counters  2-bit x8 at 0x03001CF0 (_08025F20 get)
@   slot claims       u8[3] at 0x03001CF8    (_08026020 claim)
@   car group lists   ROM 0x080CC518 (11 ptr-array lists, u32 ids, -1 term)
@ Work-area bookkeeping (base 0x03001780): +103C awarded-count, +103E
@ last-award id map, +1046 selected id, +1048 counter, +1058/+105A flags,
@   +1060 per-catalog-index flag array, +1076 award latch, +FF0 event gate.
@
@ Flow: part 1 reads descriptor triples T[A][B][C..C+2] from layout area
@ 0x080CCACC ({s16 id, s16 tag} records; A=[0x03002772], B=[0x03002778],
@ C=[0x0300288A+A*8+B*2]) gated by tag limits (+10E5 tier vs +1054), then
@ updates selection state and fires scene event 27. Part 2 grants record
@ T2[C] = {id, tag} when unowned and tag <= tier (+10E5), firing event 28.
@ Four 4x11 scans over the 2-bit zone grid (sub_08025CF4, rows 0..3 x
@ cols 0..10 = 44 cells; label _0800B652 = join point after part 2's
@ three skip branches, reached from B5F8/B604/B612 only) award bonus
@ cars on full coverage: all 44 type-0 cells nonzero -> 94 (slot 0),
@ all type-1 nonzero -> 95 (slot 1), all type-1 ==3 -> 96 (slot 2),
@ each firing event 28; finally all type-0 ==3 fires scene event 29
@ through the once-per-session gate at +FF0.

.thumb

@ ----------------------------------------------------------------------------
_0800B4A8:
	push {r4, r5, r6, r7, lr}
	mov r7, r9
	mov r6, r8
	push {r6, r7}
	ldr r7, _0800B54C_lit @ =0x03001780
	ldr r0, _0800B550_lit @ =0x00000FF8
	adds r5, r7, r0       @ r5 = 0x03002778 (index B)
	movs r2, #0
	ldrsh r1, [r5, r2]
	lsls r3, r1, #1
	ldr r6, _0800B554_lit @ =0x00000FF2
	adds r4, r7, r6       @ r4 = 0x03002772 (index A)
	movs r2, #0
	ldrsh r0, [r4, r2]
	lsls r2, r0, #3
	adds r3, r3, r2
	adds r6, #24          @ 0x100A -> record array base
	adds r2, r7, r6
	adds r3, r3, r2       @ &rec[A][B] (stride 10)
	movs r6, #0
	ldrsh r2, [r3, r6]
	mov r8, r2            @ C = record selector
	bl sub_08024F4C       @ T[A][B][C].id
	lsls r0, r0, #16
	asrs r6, r0, #16
	movs r1, #0
	ldrsh r0, [r4, r1]
	movs r2, #0
	ldrsh r1, [r5, r2]
	mov r2, r8
	bl sub_08024F74       @ T[A][B][C+1].id (view1 stride 8B)
	lsls r0, r0, #16
	asrs r0, r0, #16
	mov r9, r0
	movs r3, #0
	ldrsh r0, [r4, r3]
	movs r2, #0
	ldrsh r1, [r5, r2]
	mov r2, r8
	bl sub_08024F9C       @ T[A][B][C+2].id
	lsls r0, r0, #16
	asrs r1, r0, #16
	ldr r3, _0800B558_lit @ =0x00001054
	adds r0, r7, r3
	movs r3, #0
	ldrsh r2, [r0, r3]    @ limit word
	movs r0, #1
	negs r0, r0
	cmp r6, r0            @ id == -1 ?
	beq _0800B5C4
	ldr r3, _0800B55C_lit @ =0x000010E5
	adds r0, r7, r3
	ldrb r0, [r0, #0]     @ player tier byte
	lsls r0, r0, #24
	asrs r0, r0, #24
	cmp r0, r1            @ tier < id3 ?
	blt _0800B5C4
	cmp r2, r1            @ limit >= id3 ?
	bge _0800B5C4
	cmp r6, #2
	bhi _0800B58C
@ --- small-id path ---
	ldr r0, _0800B560_lit @ =0x00001046
	adds r4, r7, r0
	strh r6, [r4, #0]     @ selection := id
	adds r0, r6, #0
	bl sub_08025F20       @ counter[id&7]
	adds r0, #1
	ldr r1, _0800B564_lit @ =0x00001048
	adds r2, r7, r1
	strh r0, [r2, #0]
	lsls r0, r0, #16
	asrs r0, r0, #16
	cmp r0, #3
	ble _0800B568
	movs r0, #3
	strh r0, [r2, #0]     @ clamp 3
	b _0800B5C4
	.align 2, 0
_0800B54C_lit: .word 0x03001780
_0800B550_lit: .word 0x00000FF8
_0800B554_lit: .word 0x00000FF2
_0800B558_lit: .word 0x00001054
_0800B55C_lit: .word 0x000010E5
_0800B560_lit: .word 0x00001046
_0800B564_lit: .word 0x00001048

_0800B568:
	ldr r3, _0800B588_lit @ =0x0000105A
	adds r1, r7, r3
	movs r0, #1
	strh r0, [r1, #0]     @ active flag
	movs r6, #0
	ldrsh r0, [r4, r6]
	movs r3, #0
	ldrsh r1, [r2, r3]
	bl sub_08025EC0       @ apply(id, counter)
	movs r0, #27
	movs r1, #0
	bl _08023FF8          @ scene event 27 (lineup changed)
	b _0800B5C4
	.align 2, 0
_0800B588_lit: .word 0x0000105A

@ --- large-id path: compare counter against next descriptor ---
_0800B58C:
	adds r0, r6, #0
	bl sub_08025F20
	lsls r0, r0, #16
	asrs r0, r0, #16
	cmp r0, r9
	bge _0800B5C4
	ldr r0, _0800B804_lit @ =0x00001046
	adds r3, r7, r0
	strh r6, [r3, #0]
	ldr r1, _0800B808_lit @ =0x00001048
	adds r2, r7, r1
	mov r6, r9
	strh r6, [r2, #0]
	adds r0, #20          @ 0x1046 -> 0x105A
	adds r1, r7, r0
	movs r0, #1
	strh r0, [r1, #0]
	movs r1, #0
	ldrsh r0, [r3, r1]
	movs r3, #0
	ldrsh r1, [r2, r3]
	bl sub_08025EC0
	movs r0, #27
	movs r1, #0
	bl _08023FF8

@ ----------------------------------------------------------------------------
@ part 2: conditional grant of T2-view record {id, tag}
_0800B5C4:
	ldr r7, _0800B80C_lit @ =0x03001780
	ldr r6, _0800B810_lit @ =0x00000FF2
	adds r5, r7, r6
	movs r1, #0
	ldrsh r0, [r5, r1]
	ldr r2, _0800B814_lit @ =0x00000FF8
	adds r4, r7, r2
	movs r3, #0
	ldrsh r1, [r4, r3]
	mov r2, r8
	bl sub_08024FC4       @ rec[C].id  (view2, half strides)
	lsls r0, r0, #16
	asrs r6, r0, #16
	movs r1, #0
	ldrsh r0, [r5, r1]
	movs r2, #0
	ldrsh r1, [r4, r2]
	mov r2, r8
	bl sub_08024FE8       @ rec[C].tag
	lsls r0, r0, #16
	asrs r4, r0, #16
	movs r0, #1
	negs r0, r0
	cmp r6, r0            @ id == -1 ?
	beq _0800B652
	adds r0, r6, #0
	bl sub_08025FAC       @ already owned?
	lsls r0, r0, #16
	cmp r0, #0
	bne _0800B652
	ldr r3, _0800B818_lit @ =0x000010E5
	adds r0, r7, r3
	ldrb r0, [r0, #0]
	lsls r0, r0, #24
	asrs r0, r0, #24
	cmp r0, r4            @ tier < tag ?
	blt _0800B652
	ldr r1, _0800B81C_lit @ =0x00001058
	adds r0, r7, r1
	movs r4, #1
	strh r4, [r0, #0]     @ new-car flag
	adds r0, r6, #0
	bl sub_08024C90       @ catalog index of car
	lsls r0, r0, #1
	movs r2, #131
	lsls r2, r2, #5       @ 0x1060
	adds r1, r7, r2
	adds r0, r0, r1
	strh r4, [r0, #0]     @ seen-flag[idx] = 1
	ldr r3, _0800B820_lit @ =0x0000103C
	adds r4, r7, r3
	movs r1, #0
	ldrsh r0, [r4, r1]
	subs r2, #34          @ 0x1060 -> 0x103E
	adds r1, r7, r2
	adds r0, r0, r1
	strb r6, [r0, #0]     @@ award-log[count] = car id byte
	adds r0, r6, #0
	bl sub_08025F78       @ ACQUIRE car
	movs r0, #28
	movs r1, #0
	bl _08023FF8          @ scene event 28 (car acquired)
	ldrh r0, [r4, #0]
	adds r0, #1
	strh r0, [r4, #0]     @ count++

@ ----------------------------------------------------------------------------
@ progress-grid award scans (grid = 2-bit cells type<4 row<4 col<=10)
_0800B652:
	movs r6, #0           @ count(type0 nonzero<=3)
	movs r7, #0           @ count(type1 nonzero<=3)
	movs r3, #0
	mov r8, r3            @ count(type1 ==3 over rows 1..3)
	movs r5, #0
_0800B65C:
	movs r4, #0
_0800B65E:
	movs r0, #0
	adds r1, r5, #0
	adds r2, r4, #0
	bl sub_08025CF4
	lsls r0, r0, #24
	cmp r0, #0
	beq _0800B682
	movs r0, #0
	adds r1, r5, #0
	adds r2, r4, #0
	bl sub_08025CF4
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #3
	bhi _0800B682
	adds r6, #1
_0800B682:
	movs r0, #1
	adds r1, r5, #0
	adds r2, r4, #0
	bl sub_08025CF4
	lsls r0, r0, #24
	cmp r0, #0
	beq _0800B6A6
	movs r0, #1
	adds r1, r5, #0
	adds r2, r4, #0
	bl sub_08025CF4
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #3
	bhi _0800B6A6
	adds r7, #1
_0800B6A6:
	adds r4, #1
	cmp r4, #10
	ble _0800B65E
	adds r5, #1
	cmp r5, #3
	ble _0800B65C
	cmp r6, #43           @ all 44 type-0 cells populated?
	ble _0800B6F8
	movs r0, #94
	bl sub_08025FAC       @ owned?
	lsls r0, r0, #16
	cmp r0, #0
	bne _0800B6F8
	movs r0, #0
	bl sub_08026020       @ claim slot 0
	ldr r1, _0800B80C_lit @ =0x03001780
	ldr r6, _0800B824_lit @ =0x00001076
	adds r2, r1, r6
	movs r0, #1
	strh r0, [r2, #0]
	ldr r0, _0800B820_lit @ =0x0000103C
	adds r4, r1, r0
	movs r2, #0
	ldrsh r0, [r4, r2]
	ldr r3, _0800B828_lit @ =0x0000103E
	adds r1, r1, r3
	adds r0, r0, r1
	movs r1, #94
	strb r1, [r0, #0]
	movs r0, #94
	bl sub_08025F78
	movs r0, #28
	movs r1, #0
	bl _08023FF8
	ldrh r0, [r4, #0]
	adds r0, #1
	strh r0, [r4, #0]

_0800B6F8:
	cmp r7, #43           @ all type-1 cells populated?
	ble _0800B73E
	movs r0, #95
	bl sub_08025FAC
	lsls r0, r0, #16
	cmp r0, #0
	bne _0800B73E
	movs r0, #1
	bl sub_08026020       @ slot 1
	ldr r1, _0800B80C_lit
	ldr r6, _0800B824_lit
	adds r2, r1, r6
	movs r0, #1
	strh r0, [r2, #0]
	ldr r0, _0800B820_lit
	adds r4, r1, r0
	movs r2, #0
	ldrsh r0, [r4, r2]
	ldr r3, _0800B828_lit
	adds r1, r1, r3
	adds r0, r0, r1
	movs r1, #95
	strb r1, [r0, #0]
	movs r0, #95
	bl sub_08025F78
	movs r0, #28
	movs r1, #0
	bl _08023FF8
	ldrh r0, [r4, #0]
	adds r0, #1
	strh r0, [r4, #0]

_0800B73E:
	movs r5, #0
_0800B740:
	movs r4, #0
	adds r6, r5, #1
_0800B744:
	movs r0, #1
	adds r1, r5, #0
	adds r2, r4, #0
	bl sub_08025CF4
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #3
	bne _0800B75A
	movs r0, #1
	add r8, r0            @ count type1==3 rows 1..3
_0800B75A:
	adds r4, #1
	cmp r4, #10
	ble _0800B744
	adds r5, r6, #0
	cmp r5, #3
	ble _0800B740
	mov r1, r8
	cmp r1, #43           @ rows 1..3 all maxed?
	ble _0800B7AE
	movs r0, #96
	bl sub_08025FAC
	lsls r0, r0, #16
	cmp r0, #0
	bne _0800B7AE
	movs r0, #2
	bl sub_08026020       @ slot 2
	ldr r1, _0800B80C_lit
	ldr r3, _0800B824_lit
	adds r2, r1, r3
	movs r0, #1
	strh r0, [r2, #0]
	ldr r6, _0800B820_lit
	adds r4, r1, r6
	movs r2, #0
	ldrsh r0, [r4, r2]
	subs r3, #56          @ 0x1076 -> 0x103E
	adds r1, r1, r3
	adds r0, r0, r1
	movs r1, #96
	strb r1, [r0, #0]
	movs r0, #96
	bl sub_08025F78
	movs r0, #28
	movs r1, #0
	bl _08023FF8
	ldrh r0, [r4, #0]
	adds r0, #1
	strh r0, [r4, #0]

_0800B7AE:
	movs r6, #0
	movs r5, #0
_0800B7B2:
	movs r4, #0
_0800B7B4:
	adds r7, r5, #1
_0800B7B6:
	movs r0, #0
	adds r1, r5, #0
	adds r2, r4, #0
	bl sub_08025CF4
	lsls r0, r0, #24
	lsrs r0, r0, #24
	cmp r0, #3
	bne _0800B7CA
	adds r6, #1           @ count type0==3
_0800B7CA:
	adds r4, #1
	cmp r4, #10
	ble _0800B7B6
	adds r5, r7, #0
	cmp r5, #3
	ble _0800B7B2
	cmp r6, #43           @ all type-0 cells == 3?
	ble _0800B7F6
	ldr r0, _0800B80C_lit @ =0x03001780
	movs r6, #255
	lsls r6, r6, #4       @ 0x0FF0
	adds r1, r0, r6
	movs r2, #0
	ldrsh r0, [r1, r2]
	cmp r0, #0
	bne _0800B7F6
	movs r0, #1
	strh r0, [r1, #0]     @ one-shot gate
	movs r0, #29
	movs r1, #0
	bl _08023FF8          @ scene event 29
_0800B7F6:
	pop {r3, r4}
	mov r8, r3
	mov r9, r4
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0
_0800B804_lit: .word 0x00001046
_0800B808_lit: .word 0x00001048
_0800B80C_lit: .word 0x03001780
_0800B810_lit: .word 0x00000FF2
_0800B814_lit: .word 0x00000FF8
_0800B818_lit: .word 0x000010E5
_0800B81C_lit: .word 0x00001058
_0800B820_lit: .word 0x0000103C
_0800B824_lit: .word 0x00001076
_0800B828_lit: .word 0x0000103E
