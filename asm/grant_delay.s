@ GT Advance 3 - car grant-after-delay helper (_0800B82C)
@ Region: file offset 0x00B82C-0x00B89C (VMA 0x0800B82C-0x0800B89C).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/ai_collect.s (grant flow).
@
@ _0800B82C(id in r0, delay in r1): grants collection car `id` once the
@ race-frame counter reaches `delay`. Exactly fills the gap between the
@ converted ai_collect.s (ends 0x00B82C) and go_start.s (starts 0x00B89C);
@ all six static callers are the `bl sub_0800B82C` sites inside go_start.s
@ (@0xB8AE/B8B6/B8C4/B8CC/B8DA/B8E2); zero literal refs into the span.
@ Shared grant shape with the inline part-2 grant inside _0800B4A8:
@   1. if word[wa+0x10F8] < delay            -> return (race-frame gate)
@   2. if sub_08025FAC(id) != 0              -> return (already owned)
@   3. u16[wa+0x1058] = 1                    (new-car flag)
@   4. u16[wa+0x1060 + 2*sub_08024C90(id)] = 1 (catalog seen-flag)
@   5. u8[wa+0x103E + s16[wa+0x103C]] = id   (award log append)
@   6. sub_08025F78(id)                      (ACQUIRE bitmask set)
@   7. _08023FF8(28, 0)                      (scene event: car acquired)
@   8. u16[wa+0x103C]++                      (awarded count)

.thumb

@ ----------------------------------------------------------------------------
sub_0800B82C:
_0800B82C:
	push {r4, r5, r6, lr}
	adds r5, r0, #0       @ id
	ldr r6, _0800B88C_lit @ =0x03001780
	ldr r2, _0800B890_lit @ =0x000010F8
	adds r0, r6, r2
	ldr r0, [r0, #0]      @ race-frame counter word[wa+0x10F8]
	cmp r0, r1            @ vs delay arg
	bcc _0800B886         @ counter < delay -> out
	adds r0, r5, #0
	bl sub_08025FAC       @ already owned?
	lsls r0, r0, #16
	cmp r0, #0
	bne _0800B886
	ldr r1, _0800B894_lit @ =0x00001058
	adds r0, r6, r1
	movs r4, #1
	strh r4, [r0, #0]     @ new-car flag
	adds r0, r5, #0
	bl sub_08024C90       @ catalog index of car
	lsls r0, r0, #1
	movs r2, #131
	lsls r2, r2, #5       @ 0x1060
	adds r1, r6, r2
	adds r0, r0, r1
	strh r4, [r0, #0]     @ seen-flag[idx] = 1
	ldr r0, _0800B898_lit @ =0x0000103C
	adds r4, r6, r0
	movs r1, #0
	ldrsh r0, [r4, r1]    @ awarded count
	subs r2, #34          @ 0x1060 -> 0x103E
	adds r1, r6, r2
	adds r0, r0, r1
	strb r5, [r0, #0]     @ award-log[count] = car id byte
	adds r0, r5, #0
	bl sub_08025F78       @ ACQUIRE car
	movs r0, #28
	movs r1, #0
	bl _08023FF8          @ scene event 28 (car acquired)
	ldrh r0, [r4, #0]
	adds r0, #1
	strh r0, [r4, #0]     @ count++
_0800B886:
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0
_0800B88C_lit: .word 0x03001780
_0800B890_lit: .word 0x000010F8
_0800B894_lit: .word 0x00001058
_0800B898_lit: .word 0x0000103C
