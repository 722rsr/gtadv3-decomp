@ GT Advance 3 - car-catalog accessor cluster (collection system)
@ Region: file offset 0x024C3C-0x024E54 (VMA 0x08024C3C-0x08024E54).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/ai_collect.s ("Collection system").
@
@ All car ids are collection ids 0..97 (+ special 98); _080022E4 is the
@ shared bounds-checked id mapper (returns -1 for id>97). The three
@ per-car pointer tables give each id's record in a different database:
@   _08024C3C -> ROM 0x080CC640
@   _08024C58 -> ROM 0x080CD7C4
@   _08024C74 -> ROM 0x080CC948
@ Group lists live behind the pointer array at ROM 0x080CC518 (11 groups,
@ u32 ids, -1 terminated); _08025FAC(id) tests ownership (bitmask at
@ IWRAM 0x030017A0).
@
@ The bare labels _08024D78/_08024DD8/_08024DDC/_08024E00/_08024E24 each carry
@ a `.type..., %function` line below. Nothing in the corpus branches to them,
@ so without those lines tools/coverage.py's asm_vmas cannot see the entries and
@ each PREDECESSOR's span runs through them (e.g. _08024D74 read 44B instead of
@ 4B). `.type` emits zero bytes; the spans are unchanged in the ROM.

.thumb

@ ----------------------------------------------------------------------------
@ _08024C3C(id) — per-car pointer, database A (ROM 0x080CC640)
_08024C3C:
sub_08024C3C:
	push {lr}
	lsls r0, r0, #16
	asrs r0, r0, #16
	bl sub_080022E4      @ map/validate id
	lsls r0, r0, #16     @ ret*4 via <<16 then >>14
	ldr r1, _08024C54_lit @ =0x080CC640
	asrs r0, r0, #14
	adds r0, r0, r1
	ldr r0, [r0, #0]
	pop {r1}
	bx r1
	.align 2, 0
_08024C54_lit: .word 0x080CC640

@ ----------------------------------------------------------------------------
@ _08024C58(id) — per-car pointer, database B (ROM 0x080CD7C4)
_08024C58:
sub_08024C58:
	push {lr}
	lsls r0, r0, #16
	asrs r0, r0, #16
	bl sub_080022E4
	lsls r0, r0, #16
	ldr r1, _08024C70_lit @ =0x080CD7C4
	asrs r0, r0, #14
	adds r0, r0, r1
	ldr r0, [r0, #0]
	pop {r1}
	bx r1
	.align 2, 0
_08024C70_lit: .word 0x080CC7C4

@ ----------------------------------------------------------------------------
@ _08024C74(id) — per-car pointer, database C (ROM 0x080CC948)
_08024C74:
	push {lr}
	lsls r0, r0, #16
	asrs r0, r0, #16
	bl sub_080022E4
	lsls r0, r0, #16
	ldr r1, _08024C8C_lit @ =0x080CC948
	asrs r0, r0, #14
	adds r0, r0, r1
	ldr r0, [r0, #0]
	pop {r1}
	bx r1
	.align 2, 0
_08024C8C_lit: .word 0x080CC948


@ ----------------------------------------------------------------------------
@ _08024C90(id) — index of id in the flat 11-group catalog (0x080CC518);
@ returns group*? no: walks every list head, returns flat slot or 0.
_08024C90:
sub_08024C90:
	push {r4, r5, r6, lr}
	adds r5, r0, #0
	movs r2, #0          @ flat slot counter
	movs r6, #1
	negs r6, r6          @ -1 terminator / sentinel
	ldr r4, _08024CB4_lit @ =0x080CC518
_08024C9C:
	ldr r1, [r4, #0]     @ group list ptr
	ldr r0, [r1, #0]
	cmp r0, r6           @ empty list?
	beq _080024CC0_next
	movs r3, #1
	negs r3, r3
_080024CA8:
	ldr r0, [r1, #0]
	cmp r0, r5           @ found id?
	bne _080024CB8_step
	adds r0, r2, #0      @ return flat slot
	b _08024CCA
	.align 2, 0
_08024CB4_lit: .word 0x080CC518
_080024CB8_step:
	adds r1, #4
	ldr r0, [r1, #0]
	cmp r0, r3           @ until -1
	bne _080024CA8
_080024CC0_next:
	adds r4, #4          @ next group ptr
	adds r2, #1
	cmp r2, #10          @ 11 groups total
	ble _08024C9C
	movs r0, #0
_08024CCA:
	pop {r4, r5, r6}
	pop {r1}
	bx r1
	.align 2, 0

@ ----------------------------------------------------------------------------
@ _08024CD0(id, g) — raw index of id within group g's list (no owner test)
_08024CD0:
	push {r4, lr}
	adds r4, r0, #0
	movs r3, #0          @ index
	ldr r0, _08024CF4_lit @ =0x080CC518
	lsls r1, r1, #2
	adds r1, r1, r0
	ldr r1, [r1, #0]     @ list ptr
	ldr r0, [r1, #0]
	movs r2, #1
	negs r2, r2
	cmp r0, r2
	beq _080024D02_none
_080024CE8:
	ldr r0, [r1, #0]
	cmp r4, r0
	bne _080024CF8_step
	adds r0, r3, #0
	b _080024D04_ret
	.align 2, 0
_08024CF4_lit: .word 0x080CC518
_080024CF8_step:
	adds r3, #1
	adds r1, #4
	ldr r0, [r1, #0]
	cmp r0, r2
	bne _080024CE8
_080024D02_none:
	movs r0, #0
_080024D04_ret:
	pop {r4}
	pop {r1}
	bx r1
	.align 2, 0

@ ----------------------------------------------------------------------------
@ _08024D0C(id, g) — like _08024CD0 but only counts OWNED entries before
@ the match (garage ordering).
_08024D0C:
	push {r4, r5, r6, lr}
	adds r6, r0, #0
	movs r5, #0
	ldr r0, _08024D1C_lit @ =0x080CC518
	lsls r1, r1, #2
	adds r1, r1, r0
	ldr r4, [r1, #0]
	b _080024D38_test
	.align 2, 0
_08024D1C_lit: .word 0x080CC518
_080024D20_loop:
	ldr r0, [r4, #0]
	bl sub_08025FAC      @ owned?
	cmp r0, #0
	beq _080024D36_skip
	ldr r0, [r4, #0]
	cmp r6, r0           @ match?
	bne _080024D34_next
	adds r0, r5, #0
	b _080024D44_ret
_080024D34_next:
	adds r5, #1
_080024D36_skip:
	adds r4, #4
_080024D38_test:
	ldr r1, [r4, #0]
	movs r0, #1
	negs r0, r0
	cmp r1, r0
	bne _080024D20_loop
	movs r0, #0
_080024D44_ret:
	pop {r4, r5, r6}
	pop {r1}
	bx r1
	.align 2, 0

@ ----------------------------------------------------------------------------
@ _08024D4C(g) — group list pointer
_08024D4C:
	ldr r1, _08024D58_lit @ =0x080CC518
	lsls r0, r0, #2
	adds r0, r0, r1
	ldr r0, [r0, #0]
	bx lr
	.align 2, 0
_08024D58_lit: .word 0x080CC518

@ ----------------------------------------------------------------------------
@ _08024D5C(id) — group list that contains id (via flat index)
_08024D5C:
sub_08024D5C:
	push {r4, lr}
	ldr r4, _08024D70_lit @ =0x080CC518
	bl _08024C90         @ flat slot
	lsls r0, r0, #2
	adds r0, r0, r4
	ldr r0, [r0, #0]
	pop {r4}
	pop {r1}
	bx r1
	.align 2, 0
_08024D70_lit: .word 0x080CC614

@ ----------------------------------------------------------------------------
@ _08024D74 — constant 97 (car-id upper bound)
_08024D74:
	movs r0, #97
	bx lr

@ ----------------------------------------------------------------------------
@ _08024D78(g) — number of cars in group g. The.type line matters: nothing
@ in the corpus branches here, so without it coverage.py's asm_vmas cannot see
@ this entry and _08024D74's span ran through to 0x08024DA0 (44B) instead of 4B.
	.type _08024D78, %function
_08024D78:
	ldr r1, _08024D9C_lit @ =0x080CC518
	lsls r0, r0, #2
	adds r0, r0, r1
	ldr r1, [r0, #0]
	movs r2, #0
	ldr r0, [r1, #0]
	movs r3, #1
	negs r3, r3
	cmp r0, r3
	beq _08024D96_ret
_080024D8C:
	adds r2, #1
	adds r1, #4
	ldr r0, [r1, #0]
	cmp r0, r3
	bne _080024D8C
_08024D96_ret:
	adds r0, r2, #0
	bx lr
	.align 2, 0
_08024D9C_lit: .word 0x080CC518

@ ----------------------------------------------------------------------------
@ _08024DA0(g) — number of OWNED cars in group g
_08024DA0:
	push {r4, r5, r6, lr}
	ldr r1, _08024DD4_lit @ =0x080CC518
	lsls r0, r0, #2
	adds r0, r0, r1
	ldr r4, [r0, #0]
	movs r5, #0
	ldr r0, [r4, #0]
	movs r1, #1
	negs r1, r1
	cmp r0, r1
	beq _08024DCC_ret
	adds r6, r1, #0
_080024DB8:
	ldr r0, [r4, #0]
	bl sub_08025FAC
	cmp r0, #0
	beq _080024DC4_skip
	adds r5, #1
_080024DC4_skip:
	adds r4, #4
	ldr r0, [r4, #0]
	cmp r0, r6
	bne _080024DB8
_08024DCC_ret:
	adds r0, r5, #0
	pop {r4, r5, r6}
	pop {r1}
	bx r1
	.align 2, 0
_08024DD4_lit: .word 0x080CC518

@ ----------------------------------------------------------------------------
@ _08024DD8 — constant 11 (group count)
	.type _08024DD8, %function
_08024DD8:
	movs r0, #11
	bx lr

@ ----------------------------------------------------------------------------
@ _08024DDC(g) — bind sprite template set A for group g
	.type _08024DDC, %function
_08024DDC:
	push {lr}
	ldr r2, _08024DF8_lit @ =0x083D7BE8
	ldr r1, _08024DFC_lit @ =0x080CC1E4
	lsls r0, r0, #2
	adds r0, r0, r1
	ldr r1, [r0, #0]
	adds r0, r2, #0
	bl sub_08007498
	bl sub_0800748C
	pop {r1}
	bx r1
	.align 2, 0
_08024DF8_lit: .word 0x083D7BE8
_08024DFC_lit: .word 0x080CC1E4

@ ----------------------------------------------------------------------------
@ _08024E00(g) — bind sprite template set B for group g
	.type _08024E00, %function
_08024E00:
	push {lr}
	ldr r2, _08024E1C_lit @ =0x083D7BE8
	ldr r1, _08024E20_lit @ =0x080CC1E4
	lsls r0, r0, #2
	adds r0, r0, r1
	ldr r1, [r0, #0]
	adds r0, r2, #0
	bl sub_08007498
	bl sub_0800748C
	pop {r1}
	bx r1
	.align 2, 0
_08024E1C_lit: .word 0x083D7BE8
_08024E20_lit: .word 0x080CC1E4

@ ----------------------------------------------------------------------------
@ _08024E24(a, b) — store halfword then clear four words via _08024F34
	.type _08024E24, %function
_08024E24:
	push {lr}
	strh r1, [r0, #0]
	adds r0, #4
	bl sub_08024F34
	pop {r0}
	bx r0
	.align 2, 0

@ ----------------------------------------------------------------------------
@ _08024E34 — count owned cars over ids 1..98
_08024E34:
	push {r4, r5, lr}
	movs r5, #0
	movs r4, #1
_080024E3A:
	adds r0, r4, #0
	bl sub_08025FAC
	cmp r0, #0
	beq _080024E46_skip
	adds r5, #1
_080024E46_skip:
	adds r4, #1
	cmp r4, #98
	ble _080024E3A
	adds r0, r5, #0
	pop {r4, r5}
	pop {r1}
	bx r1

@ End-of-region anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x08024c3c-0x08024e54), has no `.include`, and ends at
@ 0x08024e54 -- the same address the promoted body at 0x08024e34 ends on
@ (0x08024e54). The boundary is therefore unambiguous and the anchor is safe.
@ Without it promotion_screen refuses the body with "no end marker in ai_catalog.s".
ai_catalog_end:
