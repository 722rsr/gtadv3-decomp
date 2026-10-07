@ GT Advance 3 - car physics: racer event layer (subsystem records 27-30)
@ Region: file offset 0x2135C-0x21C94 (VMA 0x0802135C-0x08021C94).
@ Fully transcribed: no.incbin remains in this hole.  The ev-7 per-car
@ dispatch helper _08021BA0 and the racer handler span 0x21860-0x21BA0 it
@ dispatches into were the last raw executable bytes in the cartridge.
@
@ Disassembled via objdump/gbadisasm from baserom.gba; byte-exact.
@
@ Event ids reach the racer through the engine broadcast _08004D4C
@ (AgbMain main loop). The per-subsystem dispatcher _08021BF8 is the
@ ctor recorded in the subsystem registry (records 27..30, instance
@ size 0x124); it subtracts 1 from the event id before its 12-entry
@ switch, so table slot i serves event i+1:
@   ev1  -> _08021374 (car init/frame update: visuals/palette + state)
@   ev2  -> _0802135C (mark ctx scene-RAM word at [ctx]+0x54+1)
@   ev3,4-> default
@   ev5  -> sub_0800D854(ctx+0x38) + sub_0800D8E4(ctx+0xA0)
@   ev6  -> _08021828 race-start countdown latch (gated by ctx+0x3C)
@   ev7  -> _08021BA0 (per-car-id dispatch, raw)
@   ev12 -> _08021370 (no-op return)

.thumb

@ ----------------------------------------------------------------------------
@ _0802135C(ev=2) — ctx = r1; ctx_scene = _08004B68; set u16 at
@ ctx_scene+0x54 to 1. Called with (a=event, b=payload) by _08004D4C.
_0802135C:
	push {r4, lr}
	adds r4, r1, #0
	bl sub_08004B68
	adds r4, #0x54
	movs r0, #1
	strh r0, [r4, #0]
	pop {r4}
	pop {r0}
	bx r0

@ ----------------------------------------------------------------------------
@ _08021370(ev=12) — no-op event sink.
_08021370:
	bx lr

	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08021374(ev=1) — car init/frame update (registry records 27..30).
@ Flow:
@  1. reads the registry substate idx (27..30 via _08004B68), latches
@     car id 0..3 into u16[ctx+0xC0] (sl mirror; default paths skip the
@     store but still aim sl at ctx+0xC0);
@  2. mirrors work-area halfwords wa+0x1046/wa+0x1048 into ctx+0xBC/
@     ctx+0xBE (per-car index cells consumed by the id-0 case and by
@     table lookups);
@  3. initialises inst fields: [0x80]=6, [0x8C]=16, [0x98]=0,
@     sub-object at +0x38 via sub_0800DAB8;
@  4. binds sprite/palette resources: palettes sub_080075E8(base,idx,n)
@     from ROM banks 0x082A798C/0x082C4228/0x08368B0C(x4)/0x08342B60,
@     resource structs sub_0800798C(base,ctx+off)+sub_08007A58 activate
@     for +24/+32/+40/+8/+16/+0, object slots from sub_0800572C(7/14/
@     65)/sub_08005758(16) stored at +0xDC/+0xE0/+0xF4/+0xF8/+0x100/
@     +0x104 and bound with sub_08007ABC(handle,id,slot);
@     s16[ctx+0xEC] = tbl[0x080CBFAC][s16[0xBE]*2 + s16[0xBC]*8];
@  5. latches: u16[0x3C]=1, word[0x38]=0, [0xA8]=&[0xC8], [0xAC]=&+0x38,
@     word[0xC8]=2, wipes +0x44(160 B)/+0x4C(-32) via sub_0800D77C,
@     u16[0x40]=6, u16[0x42]=5, then sub_0800279C(1);
@  6. per-car-id switch on s16[ctx+0xC0]:
@     id0: sub_08007770(0,ctx+38,5,0); EWRAM 0x0203F990 state {+0=0,
@          +2=sub_08005758 result}; bounds-check s16[ctx+0xBE] in 1..3
@          then two lookups over tables 0x080CBFEC/0x080CC02C indexed
@          [id*2 + s16[ctx+0xBC]*8] feeding sub_08007570/sub_080075E8
@          against base 0x08349754;
@     id1: sub_08007770(...,4,...); decrement u16[wa+0x103C] lap-style
@          counter (clamp 0); byte-table wa+0x103E[count] -> ctx+0xB6,
@          sub_080022E4(s8) -> ctx+0xB8, u16[ctx+0xB4]=0,
@          sub_0800D9A4(ctx+0xCC,0,s16[0xB6]); object getters
@          sub_08024C3C/_08024D5C/_08024C58(s16[0xB6]) -> slots
@          +0xF8/+0xE0(+bind [ctx+36])/+0xDC... wait: results stored to
@          [sp24]=+0xF8, [sp20]=+0xE0, [sp32]=+0x104 with binds on
@          [ctx+36]/[ctx+28]/[ctx+44];
@     id2: only sub_08007770(0,ctx+38,7,0);
@     id3: decrement u16[wa+0x104A]; halfword-table wa+0x104C[cnt]->
@          ctx+0xC2; sub_08007770(...,6,...); resource 0x082DFBDC ->
@          ctx+48 (+palette 0..3); slot sub_0800572C(12) -> ctx+0x10C;
@          sub_080258A8(s16[0xC2]) -> ctx+0x110 (sign-extended);
@          sub_08007ABC([ctx+52], val, [ctx+0x10C]); s16[0xC2]->
@          ctx+0x11C; sub_08026F30(v,4)->ctx+0x118; sub_08026F50(
@          [ctx+0x11C],4).
@ Pools: 0x80215BC(12w), 0x8021680(4w), 0x8021740(3w), 0x8021818(4w).
_08021374:
	push {r4, r5, r6, r7, lr}
	mov r7, sl
	mov r6, r9
	mov r5, r8
	push {r5, r6, r7}
	sub sp, #36
	adds r7, r0, #0
	bl sub_08004B68
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #28
	beq.n _080213BC
	cmp r0, #28
	bgt.n _0802139E
	cmp r0, #27
	beq.n _080213AE
	movs r2, #192            @ 0xc0 - default: sl aims at id cell only
	adds r2, r2, r7
	mov sl, r2
	b.n _080213E8
_0802139E:
	cmp r0, #29
	beq.n _080213CA
	cmp r0, #30
	beq.n _080213D8
	movs r3, #192            @ 0xc0
	adds r3, r3, r7
	mov sl, r3
	b.n _080213E8
_080213AE:
	movs r0, #66             @ 0x42
	bl sub_0802B1E4
	adds r1, r7, #0
	adds r1, #192            @ 0xc0
	movs r0, #0              @ car id 0
	b.n _080213E4
_080213BC:
	movs r0, #66             @ 0x42
	bl sub_0802B1E4
	adds r1, r7, #0
	adds r1, #192            @ 0xc0
	movs r0, #1              @ car id 1
	b.n _080213E4
_080213CA:
	movs r0, #66             @ 0x42
	bl sub_0802B1E4
	adds r1, r7, #0
	adds r1, #192            @ 0xc0
	movs r0, #2              @ car id 2
	b.n _080213E4
_080213D8:
	movs r0, #66             @ 0x42
	bl sub_0802B1E4
	adds r1, r7, #0
	adds r1, #192            @ 0xc0
	movs r0, #3              @ car id 3
_080213E4:
	strh r0, [r1, #0]
	mov sl, r1               @ sl = &u16[ctx+0xC0]
_080213E8:
	ldr r4, _080215BC @ =0x03001780
	ldr r5, _080215C0 @ =0x00001046
	adds r0, r4, r5
	ldrh r0, [r0, #0]
	adds r6, r7, #0
	adds r6, #188            @ 0xbc
	str r6, [sp, #8]
	strh r0, [r6, #0]
	ldr r1, _080215C4 @ =0x00001048
	adds r0, r4, r1
	ldrh r0, [r0, #0]
	adds r2, r7, #0
	adds r2, #190            @ 0xbe
	str r2, [sp, #12]
	strh r0, [r2, #0]
	adds r0, r7, #0
	adds r0, #128            @ 0x80
	movs r3, #6
	str r3, [r0, #0]
	adds r0, #12             @ 0x8c
	movs r4, #16
	str r4, [r0, #0]
	adds r0, #12             @ 0x98
	movs r5, #0
	str r5, [r0, #0]
	movs r6, #56             @ 0x38
	adds r6, r6, r7
	mov r9, r6
	mov r0, r9
	bl sub_0800DAB8
	ldr r0, _080215C8 @ =0x082A798C
	movs r1, #2
	movs r2, #5
	bl sub_080075E8
	ldr r0, _080215CC @ =0x082C4228
	movs r1, #0
	movs r2, #6
	bl sub_080075E8
	ldr r0, _080215D0 @ =0x082C4458
	adds r1, r7, #0
	adds r1, #24
	bl sub_0800798C
	movs r0, #7
	bl sub_0800572C
	adds r1, r7, #0
	adds r1, #220            @ 0xdc
	str r1, [sp, #16]
	str r0, [r1, #0]
	adds r2, r7, #0
	adds r2, #224            @ 0xe0
	str r2, [sp, #20]
	movs r3, #2
	str r3, [r2, #0]
	ldr r0, [r7, #28]
	ldr r2, [r1, #0]
	movs r1, #2
	bl sub_08007ABC
	ldr r0, _080215D4 @ =0x082C5040
	adds r1, r7, #0
	adds r1, #32
	bl sub_0800798C
	movs r0, #14
	bl sub_0800572C
	movs r4, #244            @ 0xf4
	adds r4, r4, r7
	mov r8, r4
	str r0, [r4, #0]
	adds r5, r7, #0
	adds r5, #248            @ 0xf8
	str r5, [sp, #24]
	movs r6, #5
	str r6, [r5, #0]
	ldr r0, [r7, #36]
	ldr r2, [r4, #0]
	movs r1, #5
	bl sub_08007ABC
	ldr r0, _080215D8 @ =0x082D0DC0
	adds r1, r7, #0
	adds r1, #40             @ 0x28
	bl sub_0800798C
	movs r0, #7
	bl sub_0800572C
	movs r1, #128            @ 0x80
	lsls r1, r1, #1          @ x256 -> 0x100
	adds r1, r7, r1
	str r1, [sp, #28]
	str r0, [r1, #0]
	movs r2, #130            @ 0x82
	lsls r2, r2, #1          @ x256 -> 0x104
	adds r2, r7, r2
	str r2, [sp, #32]
	movs r0, #50             @ 0x32
	str r0, [r2, #0]
	ldr r0, [r7, #44]
	ldr r2, [r1, #0]
	movs r1, #50             @ 0x32
	bl sub_08007ABC
	ldr r6, _080215DC @ =0x08368B0C
	adds r0, r6, #0
	adds r1, r7, #0
	bl sub_0800798C
	adds r0, r7, #0
	bl sub_08007A58
	adds r0, r6, #0
	movs r1, #0
	movs r2, #3
	bl sub_080075E8
	adds r0, r6, #0
	movs r1, #1
	movs r2, #4
	bl sub_080075E8
	adds r0, r6, #0
	movs r1, #2
	movs r2, #7
	bl sub_080075E8
	adds r0, r6, #0
	movs r1, #3
	movs r2, #10
	bl sub_080075E8
	ldr r0, _080215E0 @ =0x0836EF7C
	adds r1, r7, #0
	adds r1, #8
	bl sub_0800798C
	movs r0, #65             @ 0x41
	bl sub_0800572C
	adds r3, r7, #0
	adds r3, #232            @ 0xe8
	str r0, [r3, #0]
	movs r4, #236            @ 0xec
	adds r4, r4, r7
	mov ip, r4
	ldr r2, _080215E4 @ =0x080CBFAC
	ldr r5, [sp, #12]
	movs r1, #0
	ldrsh r0, [r5, r1]
	lsls r0, r0, #1
	ldr r4, [sp, #8]
	movs r5, #0
	ldrsh r1, [r4, r5]
	lsls r1, r1, #3
	adds r0, r0, r1
	adds r0, r0, r2
	movs r2, #0
	ldrsh r1, [r0, r2]
	mov r4, ip
	str r1, [r4, #0]
	ldr r0, [r7, #12]
	ldr r2, [r3, #0]
	bl sub_08007ABC
	ldr r5, _080215E8 @ =0x08342B60
	adds r4, r7, #0
	adds r4, #16
	adds r0, r5, #0
	adds r1, r4, #0
	bl sub_0800798C
	adds r0, r4, #0
	bl sub_08007A58
	adds r0, r5, #0
	movs r1, #0
	movs r2, #9
	bl sub_080075E8
	adds r0, r7, #0
	adds r0, #168            @ 0xa8
	adds r4, #184            @ -> ctx+0xc8
	str r4, [r0, #0]
	adds r0, #4              @ 0xac
	mov r5, r9
	str r5, [r0, #0]
	movs r0, #0
	str r0, [r7, #56]        @ [ctx+0x38] = 0
	movs r1, #1
	mov r9, r1
	mov r2, r9
	strh r2, [r7, #60]       @ u16[ctx+0x3c] = 1
	adds r0, r7, #0
	adds r0, #76             @ 0x4c
	movs r2, #32
	negs r2, r2              @ -32
	movs r1, #0
	bl sub_0800D77C
	adds r0, r7, #0
	adds r0, #68             @ 0x44
	movs r1, #0
	movs r2, #160            @ 0xa0
	bl sub_0800D77C
	adds r0, r7, #0
	adds r0, #64             @ 0x40
	movs r3, #6
	strh r3, [r0, #0]
	adds r0, #2              @ 0x42
	movs r5, #5
	strh r5, [r0, #0]
	movs r0, #2
	str r0, [r4, #0]         @ [ctx+0xc8] = 2
	movs r0, #1
	bl sub_0800279C
	mov r2, sl
	movs r3, #0
	ldrsh r1, [r2, r3]       @ car id
	cmp r1, #1
	beq.n _08021690
	cmp r1, #1
	bgt.n _080215EC
	cmp r1, #0
	beq.n _080215FA
	b.n _08021806
	movs r0, r0
	.align 2, 0
_080215BC: .4byte 0x03001780
_080215C0: .4byte 0x00001046
_080215C4: .4byte 0x00001048
_080215C8: .4byte 0x082A798C
_080215CC: .4byte 0x082C4228
_080215D0: .4byte 0x082C4458
_080215D4: .4byte 0x082C5040
_080215D8: .4byte 0x082D0DC0
_080215DC: .4byte 0x08368B0C
_080215E0: .4byte 0x0836EF7C
_080215E4: .4byte 0x080CBFAC
_080215E8: .4byte 0x08342B60

_080215EC:
	cmp r1, #2
	bne.n _080215F2
	b.n _0802174C
_080215F2:
	cmp r1, #3
	bne.n _080215F8
	b.n _08021762
_080215F8:
	b.n _08021806

@ --- car id 0: player-car state block + opponent-table bind ---
_080215FA:
	movs r0, #4
	str r0, [sp, #0]
	mov r4, r9
	str r4, [sp, #4]
	movs r0, #0
	adds r1, r6, #0          @ r6 still = ctx+0x38
	movs r2, #5
	movs r3, #0
	bl sub_08007770
	movs r0, #16
	bl sub_08005758
	ldr r3, _08021680 @ =0x0203F990
	strh r0, [r3, #2]
	movs r5, #0
	strh r5, [r3, #0]
	ldr r6, [sp, #12]        @ &u16[ctx+0xbe]
	movs r0, #0
	ldrsh r1, [r6, r0]
	cmp r1, #0
	bne.n _08021628
	b.n _08021806
_08021628:
	cmp r1, #0
	bge.n _0802162E
	b.n _08021806
_0802162E:
	cmp r1, #3
	ble.n _08021634
	b.n _08021806
_08021634:
	ldr r4, _08021684 @ =0x08349754
	ldr r2, _08021688 @ =0x080CBFEC
	lsls r1, r1, #1
	ldr r5, [sp, #8]
	movs r6, #0
	ldrsh r0, [r5, r6]
	lsls r0, r0, #3
	adds r1, r1, r0
	adds r1, r1, r2
	movs r0, #0
	ldrsh r1, [r1, r0]
	movs r5, #2
	ldrsh r2, [r3, r5]
	movs r6, #16
	str r6, [sp, #0]
	adds r0, r4, #0
	movs r3, #16
	bl sub_08007570
	ldr r2, _0802168C @ =0x080CC02C
	ldr r1, [sp, #12]
	movs r3, #0
	ldrsh r0, [r1, r3]
	lsls r0, r0, #1
	ldr r5, [sp, #8]
	movs r6, #0
	ldrsh r1, [r5, r6]
	lsls r1, r1, #3
	adds r0, r0, r1
	adds r0, r0, r2
	movs r2, #0
	ldrsh r1, [r0, r2]
	adds r0, r4, #0
	movs r2, #8
	bl sub_080075E8
	b.n _08021806
	movs r0, r0
	.align 2, 0
_08021680: .4byte 0x0203F990
_08021684: .4byte 0x08349754
_08021688: .4byte 0x080CBFEC
_0802168C: .4byte 0x080CC02C

@ --- car id 1: lap countdown + object spawns ---
_08021690:
	movs r0, #4
	str r0, [sp, #0]
	mov r3, r9
	str r3, [sp, #4]
	movs r0, #0
	adds r1, r6, #0
	movs r2, #4
	movs r3, #0
	bl sub_08007770
	ldr r4, _08021740 @ =0x03001780
	ldr r5, _08021744 @ =0x0000103C
	adds r1, r4, r5
	ldrh r0, [r1, #0]
	subs r0, #1
	strh r0, [r1, #0]
	lsls r0, r0, #16
	cmp r0, #0
	bgt.n _080216BA
	movs r6, #0
	strh r6, [r1, #0]
_080216BA:
	movs r2, #0
	ldrsh r0, [r1, r2]
	ldr r3, _08021740 @ =0x03001780
	ldr r4, _08021748 @ =0x0000103E
	adds r1, r3, r4
	adds r0, r0, r1
	ldrb r0, [r0, #0]
	lsls r0, r0, #24
	asrs r0, r0, #24         @ s8 table value
	adds r4, r7, #0
	adds r4, #182            @ 0xb6
	strh r0, [r4, #0]
	movs r5, #0
	ldrsh r0, [r4, r5]
	bl sub_080022E4
	adds r1, r7, #0
	adds r1, #184            @ 0xb8
	strh r0, [r1, #0]
	adds r0, r7, #0
	adds r0, #180            @ 0xb4
	movs r6, #0
	strh r6, [r0, #0]
	adds r0, #24             @ -> ctx+0xcc
	movs r2, #0
	ldrsh r1, [r4, r2]
	movs r2, #0
	bl sub_0800D9A4
	movs r3, #0
	ldrsh r0, [r4, r3]
	bl sub_08024C3C
	adds r1, r0, #0
	ldr r5, [sp, #24]        @ &ctx+0xf8
	str r1, [r5, #0]
	ldr r0, [r7, #36]
	mov r6, r8               @ r8 = &ctx+0xf4
	ldr r2, [r6, #0]
	bl sub_08007ABC
	movs r1, #0
	ldrsh r0, [r4, r1]
	bl sub_08024D5C
	adds r1, r0, #0
	ldr r2, [sp, #20]        @ &ctx+0xe0
	str r1, [r2, #0]
	ldr r0, [r7, #28]
	ldr r3, [sp, #16]        @ &ctx+0xdc
	ldr r2, [r3, #0]
	bl sub_08007ABC
	movs r5, #0
	ldrsh r0, [r4, r5]
	bl sub_08024C58
	adds r1, r0, #0
	ldr r6, [sp, #32]        @ &ctx+0x104
	str r1, [r6, #0]
	ldr r0, [r7, #44]
	ldr r3, [sp, #28]        @ &ctx+0x100
	ldr r2, [r3, #0]
	bl sub_08007ABC
	b.n _08021806
	movs r0, r0
	.align 2, 0
_08021740: .4byte 0x03001780
_08021744: .4byte 0x0000103C
_08021748: .4byte 0x0000103E

@ --- car id 2: config pass only ---
_0802174C:
	movs r0, #4
	str r0, [sp, #0]
	mov r4, r9
	str r4, [sp, #4]
	movs r0, #0
	adds r1, r6, #0
	movs r2, #7
	movs r3, #0
	bl sub_08007770
	b.n _08021806

@ --- car id 3: countdown + trailing objects ---
_08021762:
	ldr r5, _08021818 @ =0x03001780
	ldr r0, _0802181C @ =0x0000104A
	adds r1, r5, r0
	ldrh r0, [r1, #0]
	subs r0, #1
	strh r0, [r1, #0]
	lsls r0, r0, #16
	cmp r0, #0
	bgt.n _08021778
	movs r2, #0
	strh r2, [r1, #0]
_08021778:
	movs r3, #0
	ldrsh r0, [r1, r3]
	lsls r0, r0, #1
	ldr r4, _08021818 @ =0x03001780
	ldr r5, _08021820 @ =0x0000104C
	adds r1, r4, r5
	adds r0, r0, r1
	ldrh r0, [r0, #0]
	adds r5, r7, #0
	adds r5, #194            @ 0xc2
	strh r0, [r5, #0]
	movs r0, #4
	str r0, [sp, #0]
	mov r0, r9
	str r0, [sp, #4]
	movs r0, #0
	adds r1, r6, #0
	movs r2, #6
	movs r3, #0
	bl sub_08007770
	ldr r4, _08021824 @ =0x082DFBDC
	adds r1, r7, #0
	adds r1, #48
	adds r0, r4, #0
	bl sub_0800798C
	adds r0, r4, #0
	movs r1, #0
	movs r2, #3
	bl sub_080075E8
	movs r0, #12
	bl sub_0800572C
	movs r1, #134            @ 0x86
	lsls r1, r1, #1          @ x256 -> 0x10c
	adds r4, r7, r1
	str r0, [r4, #0]
	movs r2, #0
	ldrsh r0, [r5, r2]
	bl sub_080258A8
	adds r1, r0, #0
	movs r3, #136            @ 0x88
	lsls r3, r3, #1          @ x256 -> 0x110
	adds r0, r7, r3
	lsls r1, r1, #16
	asrs r1, r1, #16
	str r1, [r0, #0]
	ldr r0, [r7, #52]
	ldr r2, [r4, #0]
	bl sub_08007ABC
	movs r6, #142            @ 0x8e
	lsls r6, r6, #1          @ x256 -> 0x11c
	adds r4, r7, r6
	movs r1, #0
	ldrsh r0, [r5, r1]
	str r0, [r4, #0]
	movs r1, #4
	bl sub_08026F30
	movs r2, #140            @ 0x8c
	lsls r2, r2, #1          @ x256 -> 0x118
	adds r1, r7, r2
	str r0, [r1, #0]
	ldr r1, [r4, #0]
	movs r2, #4
	bl sub_08026F50
_08021806:
	add sp, #36
	pop {r3, r4, r5}
	mov r8, r3
	mov r9, r4
	mov sl, r5
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08021818: .4byte 0x03001780
_0802181C: .4byte 0x0000104A
_08021820: .4byte 0x0000104C
_08021824: .4byte 0x082DFBDC

@ ----------------------------------------------------------------------------
@ _08021828(a=ev6, b, c) — race-start latch (table slot 5). t = u16(c) - 1; when <= 1:
@ sub_0802B368(1), [ctx+0xC8] = 1 (word), u16[ctx+0x3C] = 0,
@ u16[ctx+0xA4] = 1, u16[ctx+0x40] = 1.
@ The ctor gates this on u16[ctx+0x3C] != 0.
_08021828:
	push {r4, lr}
	adds r4, r0, #0
	lsls r2, r2, #16
	ldr r0, _0802185C @ =0xFFFF0000
	adds r2, r2, r0
	lsrs r2, r2, #16
	cmp r2, #1
	bhi _08021854
	movs r0, #1
	bl sub_0802B368
	adds r0, r4, #0
	adds r0, #0xC8
	movs r1, #1
	str r1, [r0]
	movs r0, #0
	strh r0, [r4, #0x3c]
	adds r0, r4, #0
	adds r0, #0xa4
	strh r1, [r0]
	subs r0, #100
	strh r1, [r0]
_08021854:
	pop {r4}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_0802185C: .4byte 0xFFFF0000

@ ----------------------------------------------------------------------------
@ 0x21860-0x21BA0 — the shared cursor/countdown stepper plus the four
@ per-car-id racer handlers that _08021BA0 below dispatches into on
@ s16[rec+0xC0]: 0 -> _080021860's sibling _0800218F8, 1 -> _0800219D0,
@ 2 -> _080021AFC, 3 -> _080021B38.  All five are pure Thumb.
@
@ Note on provenance: this span was the last raw executable pocket in the
@ ROM.  tools/objdump2gas.py mis-decoded two function prologues here as
@ "push<und>" / "pushls" because the preceding literal pool left objdump
@ with a wide line; both are plain `push {r4, r5, r6, r7, lr}` in the ROM
@ (bytes F0 B5 at 0x218F8 and 0x219D0).  Verified by the `make` SHA gate.

	.type sub_080021860, %function
sub_080021860:
_080021860:
	push	{r4, r5, r6, r7, lr}
	sub	sp, #24
	adds	r6, r0, #0
	adds	r7, r1, #0
	adds	r4, r3, #0
	ldr r3, _0800218E8
	ldr	r0, [r3, #0]
	subs	r0, #1
	str	r0, [r3, #0]
	cmp	r0, #0
	bgt _080021882
	ldr r0, _0800218EC
	ldrh	r1, [r0, #0]
	adds	r1, #1
	strh	r1, [r0, #0]
	movs	r0, #5
	str	r0, [r3, #0]
_080021882:
	cmp	r4, #0
	beq _0800218DE
	cmp	r4, #0
	blt _0800218DE
	cmp	r4, #3
	bgt _0800218DE
	ldr r5, _0800218EC
	movs	r1, #0
	ldrsh	r0, [r5, r1]
	cmp	r0, #15
	ble _08002189C
	movs	r0, #0
	strh	r0, [r5, #0]
_08002189C:
	ldr r0, _0800218F0
	ldr r3, _0800218F4
	lsls	r1, r4, #1
	lsls	r2, r2, #3
	adds	r1, r1, r2
	adds	r1, r1, r3
	movs	r2, #0
	ldrsh	r1, [r1, r2]
	movs	r3, #2
	ldrsh	r2, [r5, r3]
	movs	r4, #0
	ldrsh	r3, [r5, r4]
	lsls	r3, r3, #4
	movs	r4, #16
	str	r4, [sp, #0]
	bl 0x08007570
	movs	r0, #2
	ldrsh	r2, [r5, r0]
	movs	r1, #1
	str	r1, [sp, #0]
	movs	r0, #2
	str	r0, [sp, #4]
	str	r1, [sp, #8]
	movs	r0, #0
	str	r0, [sp, #12]
	str	r0, [sp, #16]
	str	r1, [sp, #20]
	adds	r0, r6, #0
	adds	r1, r7, #0
	ldr	r3, [sp, #44]
	bl 0x08002ED0
_0800218DE:
	add	sp, #24
	pop	{r4, r5, r6, r7}
	pop	{r0}
	bx	r0
	movs	r0, r0
_0800218E8: .4byte 0x030005A8
_0800218EC: .4byte 0x0203F990
_0800218F0: .4byte 0x08349754
_0800218F4: .4byte 0x080CBFEC

_0800218F8:
	push	{r4, r5, r6, r7, lr}
	mov	r7, r8
	push	{r7}
	sub	sp, #20
	adds	r7, r0, #0
	movs	r0, #7
	str	r0, [sp, #0]
	movs	r4, #1
	str	r4, [sp, #4]
	str	r4, [sp, #8]
	movs	r6, #0
	str	r6, [sp, #12]
	adds	r0, r7, #0
	movs	r1, #9
	movs	r2, #8
	movs	r3, #56
	bl 0x08007B18
	adds	r0, r7, #0
	adds	r0, #8
	adds	r1, r7, #0
	adds	r1, #232
	ldr	r1, [r1, #0]
	adds	r2, r7, #0
	adds	r2, #236
	ldr	r2, [r2, #0]
	movs	r3, #93
	str	r3, [sp, #0]
	movs	r3, #6
	str	r3, [sp, #4]
	str	r4, [sp, #8]
	str	r4, [sp, #12]
	str	r6, [sp, #16]
	movs	r3, #88
	bl 0x08007BFC
	ldr r1, _0800219C8
	movs	r0, #188
	adds	r0, r0, r7
	mov	r8, r0
	movs	r2, #0
	ldrsh	r0, [r0, r2]
	lsls	r0, r0, #1
	adds	r0, r0, r1
	movs	r3, #0
	ldrsh	r1, [r0, r3]
	movs	r5, #3
	str	r5, [sp, #0]
	str	r4, [sp, #4]
	str	r4, [sp, #8]
	str	r6, [sp, #12]
	adds	r0, r7, #0
	movs	r2, #40
	movs	r3, #64
	bl 0x08007B18
	str	r5, [sp, #0]
	str	r4, [sp, #4]
	str	r4, [sp, #8]
	str	r6, [sp, #12]
	adds	r0, r7, #0
	movs	r1, #8
	movs	r2, #40
	movs	r3, #96
	bl 0x08007B18
	adds	r0, r7, #0
	adds	r0, #16
	ldr r2, _0800219CC
	mov	r5, r8
	movs	r3, #0
	ldrsh	r1, [r5, r3]
	lsls	r1, r1, #1
	adds	r1, r1, r2
	movs	r5, #0
	ldrsh	r1, [r1, r5]
	movs	r2, #9
	str	r2, [sp, #0]
	str	r4, [sp, #4]
	str	r4, [sp, #8]
	str	r6, [sp, #12]
	movs	r2, #64
	movs	r3, #72
	bl 0x08007B18
	mov	r0, r8
	movs	r1, #0
	ldrsh	r2, [r0, r1]
	adds	r0, r7, #0
	adds	r0, #190
	movs	r5, #0
	ldrsh	r3, [r0, r5]
	movs	r0, #8
	str	r0, [sp, #0]
	movs	r0, #48
	movs	r1, #88
	bl sub_080021860
	add	sp, #20
	pop	{r3}
	mov	r8, r3
	pop	{r4, r5, r6, r7}
	pop	{r0}
	bx	r0
_0800219C8: .4byte 0x080CBF9C
_0800219CC: .4byte 0x080CBF8C

_0800219D0:
	push	{r4, r5, r6, r7, lr}
	sub	sp, #20
	adds	r5, r0, #0
	adds	r0, #92
	movs	r1, #5
	str	r1, [sp, #0]
	movs	r1, #3
	str	r1, [sp, #4]
	movs	r6, #1
	str	r6, [sp, #8]
	movs	r7, #0
	str	r7, [sp, #12]
	movs	r1, #8
	movs	r2, #56
	movs	r3, #104
	bl 0x08007B18
	adds	r0, r5, #0
	adds	r0, #204
	ldr	r0, [r0, #0]
	movs	r1, #104
	movs	r2, #72
	bl 0x08026A2C
	ldr r2, _080021AF8
	adds	r0, r5, #0
	adds	r0, #182
	movs	r3, #0
	ldrsh	r1, [r0, r3]
	lsls	r0, r1, #2
	adds	r0, r0, r1
	lsls	r0, r0, #2
	adds	r0, r0, r2
	ldrb	r0, [r0, #8]
	cmp	r0, #1
	bne _080021A2E
	movs	r0, #10
	str	r0, [sp, #0]
	str	r6, [sp, #4]
	str	r6, [sp, #8]
	str	r7, [sp, #12]
	adds	r0, r5, #0
	movs	r1, #15
	movs	r2, #112
	movs	r3, #120
	bl 0x08007B18
_080021A2E:
	movs	r4, #4
	str	r4, [sp, #0]
	str	r6, [sp, #4]
	str	r6, [sp, #8]
	str	r7, [sp, #12]
	adds	r0, r5, #0
	movs	r1, #13
	movs	r2, #152
	movs	r3, #72
	bl 0x08007B18
	str	r4, [sp, #0]
	str	r6, [sp, #4]
	str	r6, [sp, #8]
	str	r7, [sp, #12]
	adds	r0, r5, #0
	movs	r1, #14
	movs	r2, #152
	movs	r3, #88
	bl 0x08007B18
	str	r4, [sp, #0]
	str	r6, [sp, #4]
	str	r6, [sp, #8]
	str	r7, [sp, #12]
	adds	r0, r5, #0
	movs	r1, #14
	movs	r2, #152
	movs	r3, #112
	bl 0x08007B18
	adds	r0, r5, #0
	adds	r0, #24
	adds	r1, r5, #0
	adds	r1, #220
	ldr	r1, [r1, #0]
	adds	r2, r5, #0
	adds	r2, #224
	ldr	r2, [r2, #0]
	movs	r3, #80
	str	r3, [sp, #0]
	movs	r4, #6
	str	r4, [sp, #4]
	str	r6, [sp, #8]
	str	r6, [sp, #12]
	str	r7, [sp, #16]
	movs	r3, #155
	bl 0x08007BFC
	adds	r0, r5, #0
	adds	r0, #32
	adds	r1, r5, #0
	adds	r1, #244
	ldr	r1, [r1, #0]
	adds	r2, r5, #0
	adds	r2, #248
	ldr	r2, [r2, #0]
	movs	r3, #96
	str	r3, [sp, #0]
	str	r4, [sp, #4]
	str	r6, [sp, #8]
	str	r6, [sp, #12]
	str	r7, [sp, #16]
	movs	r3, #155
	bl 0x08007BFC
	adds	r0, r5, #0
	adds	r0, #40
	movs	r2, #128
	lsls	r2, r2, #1
	adds	r1, r5, r2
	ldr	r1, [r1, #0]
	movs	r3, #130
	lsls	r3, r3, #1
	adds	r2, r5, r3
	ldr	r2, [r2, #0]
	movs	r3, #120
	str	r3, [sp, #0]
	str	r4, [sp, #4]
	str	r6, [sp, #8]
	str	r6, [sp, #12]
	str	r7, [sp, #16]
	movs	r3, #155
	bl 0x08007BFC
	movs	r0, #7
	str	r0, [sp, #0]
	str	r6, [sp, #4]
	str	r6, [sp, #8]
	str	r7, [sp, #12]
	adds	r0, r5, #0
	movs	r1, #10
	movs	r2, #8
	movs	r3, #56
	bl 0x08007B18
	add	sp, #20
	pop	{r4, r5, r6, r7}
	pop	{r0}
	bx	r0
	movs	r0, r0
_080021AF8: .4byte 0x080CCEEC

_080021AFC:
	push	{r4, lr}
	sub	sp, #16
	adds	r4, r0, #0
	adds	r0, #196
	movs	r1, #5
	bl 0x0800D97C
	adds	r0, r4, #0
	adds	r0, #198
	movs	r2, #0
	ldrsh	r1, [r0, r2]
	cmp	r1, #1
	bne _080021B2E
	movs	r0, #7
	str	r0, [sp, #0]
	str	r1, [sp, #4]
	str	r1, [sp, #8]
	movs	r0, #0
	str	r0, [sp, #12]
	adds	r0, r4, #0
	movs	r1, #16
	movs	r2, #28
	movs	r3, #88
	bl 0x08007B18
_080021B2E:
	add	sp, #16
	pop	{r4}
	pop	{r0}
	bx	r0
	movs	r0, r0

	.type sub_080021B38, %function
sub_080021B38:
_080021B38:
	push	{r4, r5, r6, lr}
	sub	sp, #20
	adds	r6, r0, #0
	movs	r0, #7
	str	r0, [sp, #0]
	movs	r4, #1
	str	r4, [sp, #4]
	str	r4, [sp, #8]
	movs	r5, #0
	str	r5, [sp, #12]
	adds	r0, r6, #0
	movs	r1, #17
	movs	r2, #8
	movs	r3, #32
	bl 0x08007B18
	adds	r0, r6, #0
	adds	r0, #48
	movs	r2, #134
	lsls	r2, r2, #1
	adds	r1, r6, r2
	ldr	r1, [r1, #0]
	movs	r3, #136
	lsls	r3, r3, #1
	adds	r2, r6, r3
	ldr	r2, [r2, #0]
	movs	r3, #60
	str	r3, [sp, #0]
	movs	r3, #3
	str	r3, [sp, #4]
	str	r4, [sp, #8]
	str	r4, [sp, #12]
	str	r5, [sp, #16]
	movs	r3, #80
	bl 0x08007BFC
	movs	r1, #140
	lsls	r1, r1, #1
	adds	r0, r6, r1
	ldr	r0, [r0, #0]
	str	r4, [sp, #0]
	str	r4, [sp, #4]
	movs	r1, #80
	movs	r2, #72
	movs	r3, #4
	bl 0x08026FC4
	add	sp, #20
	pop	{r4, r5, r6}
	pop	{r0}
	bx	r0
	.hword 0x0000

@ ----------------------------------------------------------------------------
@ _08021BA0 — ev-7 per-car-id handler (table slot 7 of the ctor below).
@ Dispatches on s16[rec+0xC0] to the 0x21860-0x21BA0 handlers above, then
@ runs the shared tail: the panel object update _0800DBE8(rec+0x38).
_08021BA0:
	push	{r4, lr}
	adds	r4, r0, #0
	adds	r0, #176
	movs	r1, #15
	bl 0x0800D97C
	adds	r0, r4, #0
	adds	r0, #192
	movs	r1, #0
	ldrsh	r0, [r0, r1]
	cmp	r0, #1
	beq _080021BD4
	cmp	r0, #1
	bgt _080021BC2
	cmp	r0, #0
	beq _080021BCC
	b _080021BEA
_080021BC2:
	cmp	r0, #2
	beq _080021BDC
	cmp	r0, #3
	beq _080021BE4
	b _080021BEA
_080021BCC:
	adds	r0, r4, #0
	bl _0800218F8
	b _080021BEA
_080021BD4:
	adds	r0, r4, #0
	bl _0800219D0
	b _080021BEA
_080021BDC:
	adds	r0, r4, #0
	bl _080021AFC
	b _080021BEA
_080021BE4:
	adds	r0, r4, #0
	bl _080021B38
_080021BEA:
	adds	r0, r4, #0
	adds	r0, #56
	bl 0x0800DBE8
	pop	{r4}
	pop	{r0}
	bx	r0

@ ----------------------------------------------------------------------------
@ _08021BF8(a=event, b, c, d=ctx) — racer subsystem event dispatcher /
@ registry ctor (records 27..30 share it; registry arg = instance size
@ 0x124). Switch table has 12 entries; default exits.
@ Entry label: registry-constructed, so no `bl` in the closure; without
@ `.type %function` it was invisible and inflated _08021BA0's span. Zero bytes.
	.type _08021BF8, %function
_08021BF8:
	push {r4, r5, lr}
	adds r5, r1, #0
	adds r4, r3, #0
	subs r0, #1
	cmp r0, #11
	bhi _08021C8C
	lsls r0, r0, #2
	ldr r1, _08021C10 @ =0x08021C14 (table base)
	adds r0, r0, r1
	ldr r0, [r0]
	mov pc, r0
	movs r0, r0
	.align 2, 0
_08021C10: .4byte 0x08021C14
@ --- event switch table ---
_08021C14:
	.4byte 0x08021C7E   @ slot 0 = ev1: _08021374 (car init/frame update)
	.4byte 0x08021C44   @ slot 1 = ev2: mark scene word
	.4byte 0x08021C8C   @ slot 2 = ev3: -
	.4byte 0x08021C8C   @ slot 3 = ev4: -
	.4byte 0x08021C4E   @ slot 4 = ev5: init ctx+0x38 / ctx+0xA0 blocks
	.4byte 0x08021C68   @ slot 5 = ev6: race-start latch (gated by ctx+0x3C)
	.4byte 0x08021C60   @ slot 6 = ev7: _08021BA0 (per-car dispatch)
	.4byte 0x08021C8C   @ slot 7 = ev8: -
	.4byte 0x08021C8C   @ slot 8 = ev9: -
	.4byte 0x08021C8C   @ slot 9 = ev10: -
	.4byte 0x08021C8C   @ slot 10 = ev11: -
	.4byte 0x08021C86   @ slot 11 = ev12: no-op
_08021C44:
	adds r0, r4, #0
	adds r1, r5, #0
	bl _0802135C
	b.n _08021C8C
_08021C4E:
	adds r0, r4, #0
	adds r0, #0x38
	bl sub_0800D854
	adds r0, r4, #0
	adds r0, #0xA0
	bl sub_0800D8E4
	b.n _08021C8C
_08021C60:
	adds r0, r4, #0
	bl _08021BA0
	b.n _08021C8C
_08021C68:
	ldrh r0, [r4, #0x3c]
	cmp r0, #0
	beq.n _08021C8C
	lsls r1, r5, #16
	lsrs r1, r1, #16
	lsls r2, r2, #16
	lsrs r2, r2, #16
	adds r0, r4, #0
	bl _08021828
	b.n _08021C8C
_08021C7E:
	adds r0, r4, #0
	bl _08021374
	b.n _08021C8C
_08021C86:
	adds r0, r4, #0
	bl _08021370
_08021C8C:
	pop {r4, r5}
	pop {r0}
	bx r0
	movs r0, r0

@ Region end 0x08021C94. The next function's VMA labels live in
@ carphys_racer_tail.s, so this file cannot spell them; the anchor gives
@ tools/match_c_slice.py a unique end marker when this body is C-owned.
@ Emits no bytes.
carphys_racer_end:
