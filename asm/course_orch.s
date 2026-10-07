@ GT Advance 3 — course orchestrator cluster (track resource package users)
@ Region: file offset 0x0060EC-0x006650 (VMA 0x080060EC-0x08006650)
@
@ Disassembled via objdump from baserom.gba; byte-exact (make SHA gate).
@ Companion: tools/track_dump.py (resource package layout).
@
@ Resource package directory @ ROM 0x080CE020: {u16 magic=12345, u16 pad,
@   u32 count[11] = {63,63,8,8,8,7,7,7,26,26,26}, u32 offset[249]}
@ Record chain @ 0x080CE438..0x08028A35C: {u16 group, u16 idx, u32 size,
@   payload[size]} — seeker _08006590 returns payload addresses.
@ Groups: 0 course headers (raw) · 1 surface maps 256x256 (LZ77->0x02000000)
@         2 theme tiles (LZ77->VRAM) · 3 LUTs (raw ptr kept) · 4 palettes
@         5/6/7 surface-variant tiles/pal (LZ77/DMA/LZ77) · 8 big gfx
@         9 palettes · 10 minimap tilemaps (LZ77 -> 0x02000000)
@
@ Variant selector: word copy at EWRAM-halfwords 0x0303F758 ([+0]/[+2]),
@ installed by race init through _080060EC (CpuSet 1-word copy).
@ Course struct (parsed by _080064EC) lives at IWRAM 0x030039D0 when
@ called by race init (base arg NULL -> default directory).

.thumb

@ ----------------------------------------------------------------------------
@ _080060EC(src) — copy one word into the variant-selector cell 0x0303F758
_080060EC:
	push {lr}
	ldr r1, _080060FC        @ =0x0203F758
	movs r2, #4              @ CpuSet length: 1 word
	bl sub_0802D974          @ BIOS CpuSet(src=r0, dst=0x0303F758, ctrl=4)
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_080060FC: .4byte 0x0203F758

@ ----------------------------------------------------------------------------
@ _08006100(idx, base) — &group0[idx] payload + 12 (pre-section-A block)
.type _08006100, %function
_08006100:
	push {lr}
	adds r2, r0, #0          @ idx
	adds r0, r1, #0          @ base (0 -> default dir)
	movs r1, #0              @ group 0
	bl _08006590             @ seeker
	adds r0, #12
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08006114 — zero the variant cell 0x0303F758 (stack-staged fill word)
_08006114:
	push {lr}
	sub sp, #4
	mov r1, sp
	movs r0, #0
	strh r0, [r1, #0]
	ldr r1, _08006130        @ =0x0203F758
	ldr r2, _08006134        @ =0x01000004 (fill | 1 word)
	mov r0, sp
	bl sub_0802D974
	add sp, #4
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08006130: .4byte 0x0203F758
_08006134: .4byte 0x01000004

@ ----------------------------------------------------------------------------
@ _08006138(out, course, base) — COURSE LOAD ORCHESTRATOR
@   out: course struct (IWRAM 0x030039D0 from race init)
@   course: raw course id; resolved unless flag byte [0x03002841] != 0
@     (then used as-is), else mapped through championship id helpers:
@     u16[0x0303F758]==2 -> _08025908(course), else _080258F4(course)
@   Stores `out` pointer to 0x0203F760 for later consumers.
_08006138:
	push {r4, r5, r6, lr}
	adds r5, r0, #0
	adds r6, r2, #0
	ldr r0, _08006150        @ =0x03001780
	ldr r2, _08006154        @ =0x000010C1
	adds r0, r0, r2          @ -> 0x03002841 flag byte
	ldrb r0, [r0, #0]
	cmp r0, #0
	beq _08006158
	adds r4, r1, #0          @ flag set: use raw id
	b _08006176
	movs r0, r0
	.align 2, 0
_08006150: .4byte 0x03001780
_08006154: .4byte 0x000010C1
_08006158:
	ldr r0, _08006168        @ =0x0203F758
	ldrh r0, [r0, #0]
	cmp r0, #2
	bne _0800616C
	adds r0, r1, #0
	bl _08025908             @ championship id mapper (variant 2)
	b _08006172
	.align 2, 0
_08006168: .4byte 0x0203F758
_0800616C:
	adds r0, r1, #0
	bl _080258F4             @ championship id mapper (default)
_08006172:
	lsls r0, r0, #16
	asrs r4, r0, #16
_08006176:
	adds r0, r5, #0
	adds r1, r4, #0
	adds r2, r6, #0
	bl _080064EC             @ parse group-0 header into out
	adds r0, r5, #0
	adds r1, r4, #0
	adds r2, r6, #0
	bl _08006468             @ groups 2/3/4: theme tiles + palette + LUT
	adds r0, r5, #0
	adds r1, r4, #0
	adds r2, r6, #0
	bl _080061B8             @ groups 5/6/7: surface variant gfx + pal
	adds r0, r5, #0
	adds r1, r4, #0
	adds r2, r6, #0
	bl _0800628C             @ groups 8/9/10: big gfx, pal, minimap tiles
	adds r0, r5, #0
	adds r1, r4, #0
	adds r2, r6, #0
	bl _08006574             @ group 1: surface map -> EWRAM 0x02000000
	ldr r0, _080061B4        @ =0x0203F760
	str r5, [r0, #0]         @ remember struct ptr
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_080061B4: .4byte 0x0203F760

@ ----------------------------------------------------------------------------
@ _080061B8(arg0 unused, idx, base) — surface VARIANT graphics (groups 5/6/7)
@   variant v picked from s16[0x0303F758+0]/[+2] switch table (0..6);
@   g5[v] LZ77 -> VRAM 0x06008000; g6[v] DMA3 -> pal 0x050001E0 (16 hw);
@   g7[v] LZ77 -> VRAM 0x0600E000; final DMA zero-fills 0x0600E180..
_080061B8:
	push {r4, r5, r6, lr}
	sub sp, #4
	adds r6, r2, #0
	movs r5, #0
	ldr r1, [pc, #16]        @ (0x80061D4) =0x0203F758
	movs r2, #2
	ldrsh r0, [r1, r2]       @ [+2]
	cmp r0, #1
	beq _080061F8
	cmp r0, #1
	bgt _080061D8
	cmp r0, #0
	beq _080061DE
	b _0800621E
	.align 2, 0
_080061D4: .4byte 0x0203F758
_080061D8:
	cmp r0, #2
	beq _0800621C
	b _0800621E
_080061DE:
	movs r2, #0
	ldrsh r0, [r1, r2]       @ [+0]
	cmp r0, #1
	beq _080061F0
	cmp r0, #1
	ble _0800621E
	cmp r0, #2
	beq _080061F4
	b _0800621E
_080061F0:
	movs r5, #1
	b _0800621E
_080061F4:
	movs r5, #2
	b _0800621E
_080061F8:
	movs r2, #0
	ldrsh r0, [r1, r2]
	cmp r0, #1
	beq _08006214
	cmp r0, #1
	bgt _0800620A
	cmp r0, #0
	beq _08006210
	b _0800621E
_0800620A:
	cmp r0, #2
	beq _08006218
	b _0800621E
_08006210:
	movs r5, #3
	b _0800621E
_08006214:
	movs r5, #4
	b _0800621E
_08006218:
	movs r5, #5
	b _0800621E
_0800621C:
	movs r5, #6
_0800621E:
	adds r0, r6, #0
	movs r1, #5              @ group 5
	adds r2, r5, #0
	bl _08006590
	ldr r1, [pc, #68]        @ (0x8006270) =0x06008000
	bl sub_0802D984          @ LZ77UnCompVram
	adds r0, r6, #0
	movs r1, #6              @ group 6 (raw 32-byte palette)
	adds r2, r5, #0
	bl _08006590
	ldr r4, [pc, #56]        @ (0x8006274) =0x040000D4 (DMA3)
	str r0, [r4, #0]         @ SAD
	ldr r0, [pc, #56]        @ (0x8006278) =0x050001E0
	str r0, [r4, #4]         @ DAD
	ldr r0, [pc, #56]        @ (0x800627C) =0x80000010
	str r0, [r4, #8]         @ 32B count 16, enable; readback
	ldr r0, [r4, #8]
	adds r0, r6, #0
	movs r1, #7              @ group 7
	adds r2, r5, #0
	bl _08006590
	ldr r1, [pc, #44]        @ (0x8006280) =0x0600E000
	bl sub_0802D984          @ LZ77UnCompVram
	mov r1, sp               @ staged zero halfword
	movs r0, #0
	strh r0, [r1, #0]
	str r1, [r4, #0]
	ldr r0, [pc, #36]        @ (0x8006284) =0x0600E180
	str r0, [r4, #4]
	ldr r0, [pc, #36]        @ (0x8006288) =0x810000C0
	str r0, [r4, #8]         @ 32-bit x 0xC0 clear; readback
	ldr r0, [r4, #8]
	add sp, #4
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0
_08006270: .4byte 0x06008000
_08006274: .4byte 0x040000D4
_08006278: .4byte 0x050001E0
_0800627C: .4byte 0x80000010
_08006280: .4byte 0x0600E000
_08006284: .4byte 0x0600E180
_08006288: .4byte 0x810000C0

@ ----------------------------------------------------------------------------
@ _0800628C(hdr, idx unused, base) — big gfx + palettes + minimap (grp 8/9/10)
@   theme t = byte[g0payload+15] (cup tier hi byte); variant pair from
@   0x0303F758 -> r7 in {0..3}; if s16[758+2]==2 then idx += 13.
@   g8[t'] LZ77 -> VRAM 0x0600A000; g9[t'] palette DMA (+r7*32 slot);
@   g10[t'] LZ77 -> EWRAM 0x02000000 then tile-index fixup (+0x100 low
@   bits) and 20-entry DMA list streaming rows to BG maps 0x0600F000/
@   0x0600F800.
_0800628C:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	adds r6, r2, #0
	movs r1, #128
	lsls r1, r1, #18         @ 0x02000000
	mov r8, r1
	ldr r0, [r0, #0]         @ out+0 = g0 payload + 12
	ldrb r4, [r0, #3]        @ payload+15: cup-tier hi byte -> theme
	movs r7, #0
	ldr r0, [pc, #20]        @ (0x80062B8) =0x0203F758
	movs r1, #2
	ldrsh r5, [r0, r1]       @ [+2]
	movs r1, #0
	ldrsh r0, [r0, r1]       @ [+0]
	cmp r0, #1
	beq _080062CA
	cmp r0, #1
	bgt _080062BC
	cmp r0, #0
	beq _080062C2
	b _080062F8
	.align 2, 0
_080062B8: .4byte 0x0203F758
_080062BC:
	cmp r0, #2
	beq _080062D4
	b _080062F8
_080062C2:
	cmp r5, #1
	bne _080062F8
	movs r7, #2
	b _080062F8
_080062CA:
	movs r7, #1
	cmp r5, #1
	bne _080062F8
	movs r7, #3
	b _080062F8
_080062D4:
	cmp r5, #1
	bne _080062DA
	movs r7, #1
_080062DA:
	cmp r4, #2
	bne _080062E0
	movs r4, #8
_080062E0:
	cmp r4, #3
	bne _080062E6
	movs r4, #9
_080062E6:
	cmp r4, #4
	bne _080062EC
	movs r4, #10
_080062EC:
	cmp r4, #5
	bne _080062F2
	movs r4, #11
_080062F2:
	cmp r4, #6
	bne _080062F8
	movs r4, #12
_080062F8:
	cmp r5, #2
	bne _080062FE
	adds r4, #13
_080062FE:
	adds r0, r6, #0
	movs r1, #8              @ group 8
	adds r2, r4, #0
	bl _08006590
	adds r2, r0, #0
	ldr r1, [pc, #168]       @ (0x80063B4) =0x0600A000
	bl sub_0802D984          @ LZ77UnCompVram
	adds r0, r6, #0
	movs r1, #9              @ group 9 (raw palettes, 32B slots)
	adds r2, r4, #0
	bl _08006590
	adds r2, r0, #0
	ldr r1, [pc, #152]       @ (0x80063B8) =0x040000D4
	lsls r0, r7, #5          @ variant * 32
	adds r0, r2, r0
	str r0, [r1, #0]         @ SAD = palette slot
	ldr r0, [pc, #148]       @ (0x80063BC) =0x050001C0
	str r0, [r1, #4]         @ DAD = OBJ palette
	ldr r0, [pc, #148]       @ (0x80063C0) =0x80000010
	str r0, [r1, #8]         @ count 16, enable; readback
	ldr r0, [r1, #8]
	adds r0, r6, #0
	movs r1, #10             @ group 10 (minimap tiles)
	adds r2, r4, #0
	bl _08006590
	adds r2, r0, #0
	cmp r5, #2
	bne _080063D4
	mov r4, r8               @ ---- path A (variant pair == ?,2)
	mov r1, r8
	bl sub_0802D988          @ LZ77UnCompWram(src, 0x02000000)
	ldr r7, [pc, #124]       @ (0x80063C4) =0x000003FF
	movs r6, #252
	lsls r6, r6, #8          @ 0xFC00
	movs r3, #176
	lsls r3, r3, #2          @ 704 halfwords
	movs r0, #128
	lsls r0, r0, #1          @ 0x100
	adds r5, r0, #0
_08006356:                   @ tile-index fixup: low10 += 0x100
	ldrh r0, [r4, #0]
	adds r1, r7, #0
	ands r1, r0
	adds r2, r6, #0
	ands r2, r0
	adds r1, r5, r1
	orrs r1, r2
	strh r1, [r4, #0]
	subs r3, #1
	adds r4, #2
	cmp r3, #0
	bne _08006356
	movs r1, #128
	lsls r1, r1, #1          @ 0x100
	adds r0, r1, #0
	movs r3, #144
	lsls r3, r3, #2          @ 144 halfwords
_08006378:                   @ fill run with 0x100
	strh r0, [r4, #0]
	subs r3, #1
	adds r4, #2
	cmp r3, #0
	bne _08006378
	movs r3, #0
	ldr r2, [pc, #48]        @ (0x80063B8) =0x040000D4
	ldr r7, [pc, #64]        @ (0x80063C8) =0x80000020
	mov r4, r8
	adds r4, #64             @ src row cursor
	mov r1, r8
	ldr r6, [pc, #60]        @ (0x80063CC) =0x0600F800
	ldr r5, [pc, #60]        @ (0x80063D0) =0x0600F000
_08006392:                   @ 20-entry DMA list: even->BG0map, odd->BG1map
	str r1, [r2, #0]
	str r5, [r2, #4]
	str r7, [r2, #8]
	ldr r0, [r2, #8]
	str r4, [r2, #0]
	str r6, [r2, #4]
	str r7, [r2, #8]
	ldr r0, [r2, #8]
	adds r4, #128
	adds r1, #128
	adds r6, #64
	adds r5, #64
	adds r3, #1
	cmp r3, #19
	ble _08006392
	b _08006448
	movs r0, r0
	.align 2, 0
_080063B4: .4byte 0x0600A000
_080063B8: .4byte 0x040000D4
_080063BC: .4byte 0x050001C0
_080063C0: .4byte 0x80000010
_080063C4: .4byte 0x000003FF
_080063C8: .4byte 0x80000020
_080063CC: .4byte 0x0600F800
_080063D0: .4byte 0x0600F000
_080063D4:                   @ ---- path B
	mov r4, r8
	adds r0, r2, #0
	mov r1, r8
	bl sub_0802D988          @ LZ77UnCompWram(src, 0x02000000)
	ldr r7, [pc, #116]       @ (0x8006454) =0x000003FF
	movs r6, #252
	lsls r6, r6, #8          @ 0xFC00
	movs r3, #128
	lsls r3, r3, #2          @ 512 halfwords
	movs r0, #128
	lsls r0, r0, #1          @ 0x100
	adds r5, r0, #0
_080063EE:                   @ tile-index fixup (512 entries)
	ldrh r0, [r4, #0]
	adds r1, r7, #0
	ands r1, r0
	adds r2, r6, #0
	ands r2, r0
	adds r1, r5, r1
	orrs r1, r2
	strh r1, [r4, #0]
	subs r3, #1
	adds r4, #2
	cmp r3, #0
	bne _080063EE
	movs r1, #128
	lsls r1, r1, #1
	adds r0, r1, #0
	movs r3, #192
	lsls r3, r3, #2          @ 768 halfwords
_08006410:                   @ fill run with 0x100
	strh r0, [r4, #0]
	subs r3, #1
	adds r4, #2
	cmp r3, #0
	bne _08006410
	movs r3, #0
	ldr r2, [pc, #56]        @ (0x8006458) =0x040000D4
	ldr r7, [pc, #60]        @ (0x800645C) =0x80000020
	mov r4, r8
	adds r4, #64
	mov r1, r8
	ldr r6, [pc, #56]        @ (0x8006460) =0x0600F800
	ldr r5, [pc, #56]        @ (0x8006464) =0x0600F000
_0800642A:                   @ 20-entry DMA list
	str r1, [r2, #0]
	str r5, [r2, #4]
	str r7, [r2, #8]
	ldr r0, [r2, #8]
	str r4, [r2, #0]
	str r6, [r2, #4]
	str r7, [r2, #8]
	ldr r0, [r2, #8]
	adds r4, #128
	adds r1, #128
	adds r6, #64
	adds r5, #64
	adds r3, #1
	cmp r3, #19
	ble _0800642A
_08006448:
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08006454: .4byte 0x000003FF
_08006458: .4byte 0x040000D4
_0800645C: .4byte 0x80000020
_08006460: .4byte 0x0600F800
_08006464: .4byte 0x0600F000

@ ----------------------------------------------------------------------------
@ _08006468(out, idx, base) — theme TILES + PALETTE + LUT (groups 2/3/4)
@   cup tier c = byte[out+0 -> g0payload+13] (hi byte of header u16[+0xC]);
@   if u16[0x0300287C]==10 then c=7 else if u16[0x0203F758]==2 then
@   {2->5,3->6}; g2[c] LZ77 -> VRAM 0x06000000; g4[c] DMA3 -> pal
@   0x05000000 (160 hw = 320B); g3[c] raw ptr kept at out+0x24.
_08006468:
	push {r4, r5, r6, lr}
	adds r6, r0, #0
	adds r5, r2, #0
	ldr r0, [r6, #0]
	ldrb r4, [r0, #1]        @ g0 payload+13
	ldr r0, [pc, #16]        @ (0x8006484) =0x03001780
	ldr r1, [pc, #16]        @ (0x8006488) =0x000010FC
	adds r0, r0, r1          @ -> 0x0300287C
	ldrh r0, [r0, #0]
	cmp r0, #10
	bne _0800648C
	movs r4, #7
	b _080064A0
	movs r0, r0
	.align 2, 0
_08006484: .4byte 0x03001780
_08006488: .4byte 0x000010FC
_0800648C:
	ldr r0, [pc, #80]        @ (0x80064E0) =0x0203F758
	ldrh r0, [r0, #0]
	cmp r0, #2
	bne _080064A0
	cmp r4, #2
	bne _0800649A
	movs r4, #5
_0800649A:
	cmp r4, #3
	bne _080064A0
	movs r4, #6
_080064A0:
	adds r0, r5, #0
	movs r1, #2              @ group 2
	adds r2, r4, #0
	bl _08006590
	movs r1, #192
	lsls r1, r1, #19         @ 0x06000000
	bl sub_0802D984          @ LZ77UnCompVram
	adds r0, r5, #0
	movs r1, #4              @ group 4 (raw 320B palette)
	adds r2, r4, #0
	bl _08006590
	ldr r1, [pc, #36]        @ (0x80064E4) =0x040000D4
	str r0, [r1, #0]         @ SAD
	movs r0, #160
	lsls r0, r0, #19         @ 0x05000000
	str r0, [r1, #4]         @ DAD = BG palette
	ldr r0, [pc, #32]        @ (0x80064E8) =0x800000A0
	str r0, [r1, #8]         @ count 160, enable; readback
	ldr r0, [r1, #8]
	adds r0, r5, #0
	movs r1, #3              @ group 3 (raw LUT)
	adds r2, r4, #0
	bl _08006590
	str r0, [r6, #36]        @ out+0x24 keeps ROM ptr
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_080064E0: .4byte 0x0203F758
_080064E4: .4byte 0x040000D4
_080064E8: .4byte 0x800000A0

@ ----------------------------------------------------------------------------
@ _080064EC(out, idx, base) — GROUP-0 HEADER PARSER
@   out layout (see tools/track_dump.py):
@     +00 g0 payload+12   +04 section A base (+28)   +08 after A (nA*20)
@     +0C after B (nB*12) +10 after C (nC*12)        +14 after D (nD*8)
@     +18 +60 scratch     +1C nA +2A nB +2C nC +2E nD +30 c5
@     +32 flag = (byte[payload+14]-6 <= 1)
_080064EC:
	push {r4, r5, lr}
	adds r4, r0, #0
	adds r3, r1, #0
	adds r0, r2, #0
	movs r1, #0              @ group 0
	adds r2, r3, #0
	bl _08006590
	mov ip, r0               @ ip = payload
	ldrh r0, [r0, #2]
	strh r0, [r4, #40]       @ +0x28 = nA
	mov r1, ip
	ldrh r0, [r1, #4]
	strh r0, [r4, #42]       @ +0x2A = nB
	ldrh r0, [r1, #6]
	strh r0, [r4, #44]       @ +0x2C = nC
	ldrh r0, [r1, #8]
	strh r0, [r4, #46]       @ +0x2E = nD
	ldrh r0, [r1, #10]
	strh r0, [r4, #48]       @ +0x30 = c5
	mov r3, ip
	adds r3, #12
	str r3, [r4, #0]         @ +00
	adds r1, #28
	str r1, [r4, #4]         @ +04 section A
	mov r5, ip
	movs r0, #2
	ldrsh r2, [r5, r0]       @ nA
	lsls r0, r2, #2
	adds r0, r0, r2          @ *5
	lsls r0, r0, #2          @ *20
	adds r1, r1, r0
	str r1, [r4, #8]         @ +08 after A
	movs r0, #4
	ldrsh r2, [r5, r0]       @ nB
	lsls r0, r2, #1
	adds r0, r0, r2          @ *3
	lsls r0, r0, #2          @ *12
	adds r1, r1, r0
	str r1, [r4, #12]        @ +0C after B
	movs r0, #6
	ldrsh r2, [r5, r0]       @ nC
	lsls r0, r2, #1
	adds r0, r0, r2
	lsls r0, r0, #2
	adds r1, r1, r0
	str r1, [r4, #16]        @ +10 after C
	movs r2, #8
	ldrsh r0, [r5, r2]       @ nD
	lsls r0, r0, #3          @ *8
	adds r1, r1, r0
	str r1, [r4, #20]        @ +14 after D
	adds r1, #60
	str r1, [r4, #24]        @ +18
	ldrb r0, [r3, #2]        @ payload+14
	subs r0, #6
	lsls r0, r0, #24
	lsrs r0, r0, #24         @ (u8)(v-6)
	cmp r0, #1
	bhi _08006568
	movs r0, #1
	b _0800656A
_08006568:
	movs r0, #0
_0800656A:
	strh r0, [r4, #50]       @ +0x32 flag
	pop {r4, r5}
	pop {r0}
	bx r0
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08006574(idx, base) — SURFACE MAP loader: g1[idx] LZ77 -> EWRAM 0x02000000
_08006574:
	push {lr}
	adds r3, r1, #0
	adds r0, r2, #0
	movs r1, #1              @ group 1
	adds r2, r3, #0
	bl _08006590
	movs r1, #128
	lsls r1, r1, #18         @ 0x02000000
	bl sub_0802D988          @ LZ77UnCompWram
	pop {r0}
	bx r0
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08006590(base|NULL, group, index) — RESOURCE SEEKER
@   base==NULL -> default directory 0x080CE020 (literal pool below).
@   k = sum(count[0..group)) + index; return dir + align4(offset[k]) + 8.
@   Validated against all 249 records of the sequential chain.
_08006590:
	push {r4, r5, r6, lr}
	adds r4, r1, #0          @ group
	adds r3, r2, #0          @ index
	cmp r0, #0
	bne _0800659C
	ldr r0, [pc, #52]        @ (0x80065D0) default dir
_0800659C:
	adds r5, r0, #0          @ dir
	movs r1, #0              @ accumulated record count
	movs r2, #0              @ loop counter
	adds r6, r3, #0
	adds r6, #13             @ index+13
	cmp r1, r4
	bcs _080065B8
_080065AA:
	adds r3, r5, #0
_080065AC:
	ldr r0, [r3, #8]         @ count[group_i]
	adds r1, r1, r0
	adds r3, #4
	adds r2, #1
	cmp r2, r4
	bcc _080065AC
_080065B8:
	adds r1, r1, r6          @ +index+13
	lsls r0, r1, #2
	adds r0, r0, r5
	ldr r0, [r0, #0]         @ offset word
	lsrs r0, r0, #2
	lsls r0, r0, #2          @ align4
	adds r0, r5, r0
	adds r0, #8              @ skip record header
	pop {r4, r5, r6}
	pop {r1}
	bx r1
	movs r0, r0
	.align 2, 0
_080065D0: .4byte 0x080CE020

@ ----------------------------------------------------------------------------
@ _080065D4(s16 x, s16 y, u8 v) — attribute lookup in table @0x080C8FE4
@   adj = x*65536 + 0xFFF30000 (saturating-ish guard); returns 10 when
@   negative; else row = s16(hi(adj)), k = 2 (or sign trick -> 0/1 when
@   adj==0); result = table[row*3 + k].
_080065D4:
	lsls r1, r1, #16
	lsrs r3, r1, #16         @ u16(x)
	lsls r2, r2, #24
	lsrs r2, r2, #24         @ u8 v
	lsls r0, r0, #16         @ x << 16
	ldr r1, [pc, #12]        @ (0x80065EC) =0xFFF30000
	adds r0, r0, r1
	lsrs r1, r0, #16
	cmp r0, #0
	bge _080065F0
	movs r0, #10
	b _08006612
	.align 2, 0
_080065EC: .4byte 0xFFF30000
_080065F0:
	cmp r3, #0
	bne _080065FC
	negs r0, r2
	orrs r0, r2
	lsrs r3, r0, #31         @ k = sign bit (0/1)
	b _080065FE
_080065FC:
	movs r3, #2
_080065FE:
	ldr r2, [pc, #20]        @ (0x8006614) table base
	lsls r1, r1, #16
	asrs r1, r1, #16         @ s16 row
	lsls r0, r1, #1
	adds r0, r0, r1          @ row*3
	lsls r1, r3, #16
	asrs r1, r1, #16
	adds r0, r0, r1
	adds r0, r0, r2
	ldrb r0, [r0, #0]
_08006612:
	bx lr
	.align 2, 0
_08006614: .4byte 0x080C8FE4

@ ----------------------------------------------------------------------------
@ _08006618 — radar/minimap work-area init
@   64 iterations writing bytes 0x80,0x80 at 4-byte strides from
@   0x0203F770 (256-byte table region); clears 0x0203F870 (h),
@   0x0203F8A4 (h), 0x0203F8A8 (w).
_08006618:
	push {r4, r5, lr}
	ldr r3, [pc, #36]        @ (0x8006640) =0x0203F870
	ldr r4, [pc, #36]        @ (0x8006644) =0x0203F8A4
	ldr r5, [pc, #40]        @ (0x8006648) =0x0203F8A8
	movs r2, #128
	ldr r0, [pc, #40]        @ (0x800664C) =0x0203F770
	movs r1, #64
_08006626:
	strb r2, [r0, #0]
	strb r2, [r0, #1]
	adds r0, #4
	subs r1, #1
	cmp r1, #0
	bne _08006626
	strh r1, [r3, #0]
	strh r1, [r4, #0]
	str r1, [r5, #0]
	pop {r4, r5}
	pop {r0}
	bx r0
	movs r0, r0
	.align 2, 0
_08006640: .4byte 0x0203F870
_08006644: .4byte 0x0203F8A4
_08006648: .4byte 0x0203F8A8
_0800664C: .4byte 0x0203F770
@ Region end (VMA 0x08006650). Emits no bytes; gives the promotion screen an
@ end marker for the last function in this region instead of guessing.
course_orch_end:
