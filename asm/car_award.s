@ GT Advance 3 - car-collection acquisition routine (new-car award)
@ Region: file offset 0xB990-0xB9F0 (VMA 0x0800B990-0x0800B9F0).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/ai_collect.s
@
@ void sub_0800B990(u8 id)
@ Awards collection car `id` when not yet owned:
@   early-out while presence test sub_08025FAC(id) != 0;
@   u16[0x030011D0] = 1                       (collection-changed flag);
@   u16[0x030011D8 + 2*_08024C90(id)] = 1     (flat catalog slot mark);
@   byte[0x030011B6 + s16[0x030011B4]] = id   (append to owned-car list),
@   then s16[0x030011B4]++                    (advance write cursor);
@   grant event sub_08025F78(id); scene event 28 "car acquired" via
@   _08023FF8(28, 0).
@ All state lives in the 0x03001780 save/work area (+0x103C/+0x103E/
@ +0x1058/+0x1060). Sole callers: tiered bonus grants in raw
@ sub_0800B9F0 (BLs @0x0800BA12/BA1C/BA26/BA30 for cars {51,78,89,26}
@ once the 32-car census crosses 7/15/23/31).

.thumb

sub_0800B990:
_0800B990:
	push {r4, r5, r6, lr}
	adds r6, r0, #0        @ r6 = car id
	bl sub_08025FAC        @ presence test(id)
	lsls r0, r0, #16
	cmp r0, #0
	bne _0800B9DE          @ already owned -> epilogue
	ldr r4, _0800B9E4_lit_0 @ =0x03000178 work area
	ldr r1, _0800B9E4_lit_1 @ =0x00001058
	adds r0, r4, r1        @ 0x030011D0
	movs r5, #1
	strh r5, [r0]          @ changed flag = 1
	adds r0, r6, #0
	bl _08024C90           @ flat catalog index of id
	lsls r0, r0, #1        @ *2 -> halfword slot
	movs r2, #131          @ 0x83
	lsls r2, r2, #5        @ 0x1060
	adds r1, r4, r2        @ 0x030011D8
	adds r0, r0, r1
	strh r5, [r0]          @ catalog slot = 1
	ldr r0, _0800B9E4_lit_2 @ =0x0000103C
	adds r5, r4, r0        @ &s16[0x030011B4] list cursor
	movs r1, #0
	ldrsh r0, [r5, r1]     @ cursor
	subs r2, #34           @ 0x1060 - 0x22 = 0x103E
	adds r4, r4, r2        @ 0x030011B6 owned-list base
	adds r0, r0, r4
	strb r6, [r0]          @ list[cursor] = id
	adds r0, r6, #0
	bl sub_08025F78        @ grant event(id)
	movs r0, #28
	movs r1, #0
	bl _08023FF8           @ scene event 28: car acquired
	ldrh r0, [r5]
	adds r0, #1
	strh r0, [r5]          @ cursor++
_0800B9DE:
	pop {r4, r5, r6}
	pop {r0}
	bx r0

	.align 2, 0
_0800B9E4_lit_0:
	.word 0x03001780
_0800B9E4_lit_1:
	.word 0x00001058
_0800B9E4_lit_2:
	.word 0x0000103C
