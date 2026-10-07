@ GT Advance 3 - rec13 continue-path UI packet builder.
@ Region: file offset 0x9B60-0x9BCC (VMA 0x08009B60-0x08009BCC), code
@ 0x9B60-0x9BC3 + private literal pool 0x9BC4-0x9BCB. Converted from raw
@ passthrough incbin (hole split at 0x9BCC); byte-exact vs baserom via
@ make SHA256 gate. Companion: asm/carphys_tick.s
@
@ Splice safety: xref.py finds exactly one BL caller - bl @0xA65C inside
@ the already-converted rec13 tick _0800A62C (numeric target, resolves
@ through the linker); zero external literal refs into the span; full-ROM
@ aligned word scan into [0x9B60,0x9BCC) returns only two odd-aligned
@ asset-region coincidences (0x4C3168 / 0x6FCFD8).
@
@ Semantics: called by rec13 when ctx+0x2C == 1 (the "continue" edge;
@ caller then returns step id 50). Builds the same-shaped 32-byte stack
@ packet the garage screen pump (rec0 key-0 handler) posts, then hands it
@ to the shared UI packet consumer _080188B0 (raw region).
@
@ Packet layout (sp.. sp+55):
@   +0  u16  9                       packet type 9
@   +2  u16  (_08008014 <0 ? +31 : +0) >>5 <<5 + 16
@                                    result floored to multiple of 32, +16
@   +8/+10 u16 0, 0                  (zeroed via r4 = 0)
@   +20  u8   lo(u16[wa+0x5E0])
@   +24  u16  u16[wa+0x574]          garage record cursor idx
@   +29  u8   u8[wa + idx*12 + 0x31] field [1] of the 12-byte garage
@                                     record selected by the cursor
@   (rest uninitialized - consumer reads only the fields above)

.thumb

@ ----------------------------------------------------------------------------
@ _08009B60 - post type-9 UI packet for the cursor-selected garage record.
@ Caller: _0800A62C (rec13 tick) ctx+0x2C==1 path -> step id 50.
_08009B60:
	push {r4, lr}
	sub sp, #56			@ 32-byte packet frame
	mov r1, sp
	movs r4, #0
	movs r0, #9			@ pkt[0] = 9
	strh r0, [r1, #0]
	bl 0x08008014			@ raw region: some frame/cursor metric
	adds r1, r0, #0
	mov r2, sp
	cmp r1, #0			@ negative? bias so >>5 rounds toward
	bge _08009B7A			@ -inf (floor to multiple of 32)
	adds r0, #31
_08009B7A:
	asrs r0, r0, #5
	lsls r0, r0, #5			@ floor to multiple of 32
	subs r0, r1, r0
	adds r0, #16
	strh r0, [r2, #2]		@ pkt[+2]
	mov r0, sp
	strh r4, [r0, #8]		@ pkt[+8]  = 0
	strh r4, [r0, #10]		@ pkt[+10] = 0
	mov r3, sp
	ldr r2, lit_08009BC4		@ wa = 0x03001780
	ldr r0, lit_08009BC8		@ 0x574
	adds r1, r2, r0
	ldrh r0, [r1, #0]
	strh r0, [r3, #24]		@ pkt[+24] = u16[wa+0x574] (cursor)
	movs r0, #0
	ldrsh r1, [r1, r0]		@ idx = s16[wa+0x574]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2			@ idx * 12
	adds r0, r0, r2
	adds r0, #49			@ +0x31 = garage-record field [1]
	ldrb r0, [r0, #0]
	strb r0, [r3, #29]		@ pkt[+29]
	mov r1, sp
	movs r0, #188			@ 0xBC << 3 = wa+0x5E0
	lsls r0, r0, #3
	adds r2, r2, r0
	ldrh r0, [r2, #0]
	strb r0, [r1, #20]		@ pkt[+20] = lo(u16[wa+0x5E0])
	mov r0, sp
	bl 0x080188B0			@ raw region: shared UI packet consumer
	add sp, #56
	pop {r4}
	pop {r0}
	bx r0				@ armcc interworking return
	movs r0, r0			@ pad halfword

	.align 2, 0
lit_08009BC4:
	.word 0x03001780		@ work-area base (wa)
lit_08009BC8:
	.word 0x00000574		@ garage-record cursor u16[wa+0x574]

@ end of pocket: next VMA 0x08009BCC = menu tick pocket
