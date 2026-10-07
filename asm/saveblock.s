@ GT Advance 3 - EEPROM save-block accessors + fixups (game-side layer)
@ Region: file offset 0x0024568-0x0024C3C (VMA 0x08024568-0x08024C3C)
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@
@ RAM map established by the init clears below (BIOS CpuSet, word mode):
@   0x03000620  block #1 slot index (u32 cleared; only low byte used)
@   0x03000628  block #1 bitfield array, 16 B = 128 flags
@   0x03000638  block #1 body, 3480 B (grid rows via _080246F4)
@   0x030013D0  block #2 buffer, 508 B (see layout in asm/saveblock.s)
@   0x030015CC  block #2 slot index
@ Working area mirrored into block #2 on save:
@   0x03001780  head (first 0x2E bytes copied verbatim)
@   0x03001CF0  u32 -> buf+0x30        0x03001CF4 s16 cur record -> buf+0x34
@   0x03001CF6  u16 -> buf+0x36        0x03001CF8 u32 -> buf+0x38
@   0x03001CFC  100 B blob -> buf+0x3C (1 word manual + 96 via _0802E0A4)
@   0x03001D60  u16 -> buf+0xA0
@   0x03001D64  car/garage records, stride 72; 35 packed (#0..#34) to
@               buf+0xA4, stride 8; records #32..#34 seeded at init from
@               ROM template 0x08060C70 via _08025B88 -> IWRAM 0x03002664.
@ EEPROM payload for block #2 = 444 B (0x1BC = through last entry);
@ buf+0x1BC..0x1FB (64 B tail) is never persisted.
@ Garage bitfields (per-record, stride 12 from 0x03001780 + s16@0x03001CF4*12,
@ fields at +0x31..+0x39) pack into buf+0x2E.. via ROM mask tables
@ 0x080CC1A8 (halfwords) / 0x080CC1D8.

.thumb

@ ----------------------------------------------------------------------------
@ _08024568 - save-system init. Selects EEPROM device, zeroes the four
@ regions above (CpuSet word-mode), picks block #2 slot index via _0800580C,
@ loads block #2, seeds something at 0x03002808 and runs an unlock-audit
@ loop over _08025CF4 (count>43 => flag halfword at 0x03002770).
@ Caller: boot path (via relocated ctor dispatch; no static BL found).
_08024568:
	push	{r4, r5, r6, r7, lr}
	sub	sp, #16
	movs	r4, #0
	movs	r0, #1
	bl	_080057D0
	str	r4, [sp]                @ staging zero word for CpuSet src
	ldr	r1, _08024604           @ =0x030013D0 block #2 buffer
	ldr	r2, _08024608           @ =0x0500007F CpuSet: 127 words, word mode
	mov	r0, sp
	bl	sub_0802D974            @ BIOS CpuSet - zero 508 B
	movs	r0, #222                @ 444 = 0x1BC byte offset in EEPROM
	lsls	r0, r0, #1
	bl	_0800580C
	ldr	r1, _0802460C           @ =0x030015CC block #2 slot index
	strb	r0, [r1]
	str	r4, [sp, #4]
	add	r0, sp, #4
	ldr	r1, _08024610           @ =0x03000620 block #1 slot index
	ldr	r2, _08024614           @ =0x05000001 1 word
	bl	sub_0802D974
	str	r4, [sp, #8]
	add	r0, sp, #8
	ldr	r1, _08024618           @ =0x03000628 block #1 bitfields
	ldr	r2, _0802461C           @ =0x05000004 4 words = 16 B
	bl	sub_0802D974
	str	r4, [sp, #12]
	add	r0, sp, #12
	ldr	r1, _08024620           @ =0x03000638 block #1 body
	ldr	r2, _08024624           @ =0x05000366 870 words = 3480 B
	bl	sub_0802D974
	bl	sub_0802446C            @ unknown (pre-load hook?)
	bl	_08024BD8               @ guarded load of block #2
	ldr	r0, _08024628           @ =0x03002808
	ldr	r1, _0802462C           @ =0x0805FC8C ROM data
	movs	r2, #3
	bl	sub_0800D95C
	movs	r7, #0                  @ unlock-audit pass counter
	movs	r5, #0
_080245C6:
	movs	r4, #0
	adds	r6, r5, #1
_080245CA:
	movs	r0, #0
	adds	r1, r5, #0
	adds	r2, r4, #0
	bl	sub_08025CF4
	cmp	r0, #3
	bne	_080245DA
	adds	r7, #1
_080245DA:
	adds	r4, #1
	cmp	r4, #10
	ble	_080245CA
	adds	r5, r6, #0
	cmp	r5, #3
	ble	_080245C6
	cmp	r7, #43                 @ 0x2B
	ble	_080245F6
	ldr	r0, _08024630           @ =0x03001780
	movs	r1, #255
	lsls	r1, r1, #4              @ 0xFF0
	adds	r0, r0, r1              @ -> 0x03002770
	movs	r1, #1
	strh	r1, [r0]
_080245F6:
	bl	sub_080241C8
	add	sp, #16
	pop	{r4, r5, r6, r7}
	pop	{r0}
	bx	r0
	movs	r0, r0
	.align 2, 0
_08024604: .4byte 0x030013D0
_08024608: .4byte 0x0500007F
_0802460C: .4byte 0x030015CC
_08024610: .4byte 0x03000620
_08024614: .4byte 0x05000001
_08024618: .4byte 0x03000628
_0802461C: .4byte 0x05000004
_08024620: .4byte 0x03000638
_08024624: .4byte 0x05000366
_08024628: .4byte 0x03002808
_0802462C: .4byte 0x0805FC8C
_08024630: .4byte 0x03001780

@ ----------------------------------------------------------------------------
@ _08024634 -> 1 - load block #1: idx=[0x03000620], dst=0x03000628.
@ Wrapper around SDK _08005988; return value forced to 1.
@ Entry labels for the four saveblock leaves below. All are reached through a
@ descriptor table rather than a `bl`, so none had `.type %function`, all four
@ were invisible, and _08024568's span ran through all of them. Zero bytes each.
	.type _08024634, %function
_08024634:
	push	{lr}
	ldr	r0, _08024648           @ =0x03000620
	ldrb	r0, [r0]
	ldr	r1, _0802464C           @ =0x03000628
	bl	sub_08005988
	movs	r0, #1
	pop	{r1}
	bx	r1
	movs	r0, r0
	.align 2, 0
_08024648: .4byte 0x03000620
_0802464C: .4byte 0x03000628

@ ----------------------------------------------------------------------------
@ _08024650 - save block #1: idx=[0x03000620], src=0x03000628.
	.type _08024650, %function
_08024650:
	push	{lr}
	ldr	r0, _08024660           @ =0x03000620
	ldrb	r0, [r0]
	ldr	r1, _08024664           @ =0x03000628
	bl	sub_080059F0
	pop	{r0}
	bx	r0
	.align 2, 0
_08024660: .4byte 0x03000620
_08024664: .4byte 0x03000628

@ --- no-op stub --------------------------------------------------------------
	@ The `.type` line is what `asm_vmas` needs to call this a function ENTRY, and
	@ without it 0x08024650's span ran through this body: its inventory entry
	@ ended at 0x0802466c (28 bytes) where the real body ends at 0x08024668 (24 --
	@ its two pool words follow the `bx` directly, with no trailing pad). A
	@ correct body then reads PARTIAL forever against a span 4 bytes too long,
	@ and `promotable` never sees it. A `bl` cannot supply the evidence
	@ instead: nothing branches to this stub, which is exactly why it was
	@ invisible. Zero bytes emitted.
	.type _08024668, %function
_08024668:
	bx	lr
	movs	r0, r0

@ ----------------------------------------------------------------------------
@ _0802466C(bit, set) - set/clear bit `bit` of the 128-bit array at
@ 0x03000628. Bit masks come from the ROM byte table at 0x080C4768.
	.type _0802466C, %function
_0802466C:
	push	{r4, r5, lr}
	adds	r5, r0, #0
	cmp	r1, #0
	beq	_08024694               @ clear path
	ldr	r3, _0802468C           @ =0x03000628
	asrs	r4, r5, #3
	adds	r2, r4, r3              @ &arr[bit>>3]
	ldr	r1, _08024690           @ =0x080C4768 mask table
	movs	r0, #7
	ands	r0, r5
	adds	r0, r0, r1
	ldrb	r1, [r0]                @ mask byte
	ldrb	r2, [r2]                @ current byte
	orrs	r1, r2
	b	_080246A8
	movs	r0, r0
	.align 2, 0
_0802468C: .4byte 0x03000628
_08024690: .4byte 0x080C4768
_08024694:
	ldr	r3, _080246B4           @ =0x03000628
	asrs	r4, r5, #3
	adds	r2, r4, r3
	ldr	r1, _080246B8           @ =0x080C4768
	movs	r0, #7
	ands	r0, r5
	adds	r0, r0, r1
	ldrb	r1, [r2]
	ldrb	r0, [r0]
	bics	r1, r0
_080246A8:
	adds	r0, r4, r3
	strb	r1, [r0]
	pop	{r4, r5}
	pop	{r0}
	bx	r0
	movs	r0, r0
	.align 2, 0
_080246B4: .4byte 0x03000628
_080246B8: .4byte 0x080C4768

@ ----------------------------------------------------------------------------
@ _080246BC(bit) -> mask - test bit `bit` of the 0x03000628 array.
@ Returns (byte & mask), i.e. nonzero iff set.
_080246BC:
	ldr	r1, _080246D4           @ =0x03000628
	asrs	r2, r0, #3
	adds	r2, r2, r1
	ldr	r3, _080246D8           @ =0x080C4768
	movs	r1, #7
	ands	r1, r0
	adds	r1, r1, r3
	ldrb	r0, [r2]
	ldrb	r1, [r1]
	ands	r0, r1
	bx	lr
	movs	r0, r0
	.align 2, 0
_080246D4: .4byte 0x03000628
_080246D8: .4byte 0x080C4768

@ ----------------------------------------------------------------------------
@ _080246DC -> u16 - getter for block #1 bitfield-area +0x0C.
_080246DC:
	ldr	r0, _080246E4           @ =0x03000628
	ldrh	r0, [r0, #12]
	bx	lr
	movs	r0, r0
	.align 2, 0
_080246E4: .4byte 0x03000628

@ ----------------------------------------------------------------------------
@ _080246E8(v) - setter for block #1 bitfield-area +0x0C.
_080246E8:
	ldr	r1, _080246F0           @ =0x03000628
	strh	r0, [r1, #12]
	bx	lr
	movs	r0, r0
	.align 2, 0
_080246F0: .4byte 0x03000628

@ ----------------------------------------------------------------------------
@ _080246F4(row, col) -> ptr - address into block #1 body 0x03000638:
@ 0x03000638 + row*60 + col*12 (rows of 5 12-byte cells).
_080246F4:
	adds	r2, r0, #0
	lsls	r0, r2, #4
	subs	r0, r0, r2              @ row*15
	lsls	r0, r0, #2              @ row*60
	lsls	r2, r1, #1
	adds	r2, r2, r1              @ col*3
	lsls	r2, r2, #2              @ col*12
	ldr	r1, _0802470C           @ =0x03000638
	adds	r2, r2, r1
	adds	r0, r0, r2
	bx	lr
	movs	r0, r0
	.align 2, 0
_0802470C: .4byte 0x03000638

@ --- no-op stubs (handler-table fillers?) -----------------------------------
	.type _08024710, %function
_08024710:
	bx	lr
	movs	r0, r0

	.type _08024714, %function
_08024714:
	bx	lr
	movs	r0, r0

	.type _08024718, %function
_08024718:
	bx	lr
	movs	r0, r0

@ ----------------------------------------------------------------------------
@ _0802471C - bulk-save pair: slot 0 <=- 0x03003574 and 0x03003594
@ (both via SDK _080059F0). Purpose unknown (ghost/replay?).
	.type _0802471C, %function
_0802471C:
	push	{r4, lr}
	bl	sub_0802417C
	ldr	r4, _0802473C           @ =0x03003574
	movs	r0, #0
	adds	r1, r4, #0
	bl	sub_080059F0
	adds	r4, #32                 @ -> 0x03003594
	movs	r0, #0
	adds	r1, r4, #0
	bl	sub_080059F0
	pop	{r4}
	pop	{r0}
	bx	r0
	.align 2, 0
_0802473C: .4byte 0x03003574

@ ----------------------------------------------------------------------------
@ _08024740(val, k, part) - if val==1, OR mask table halfword `k` into the
@ packed garage halfword at 0x030013D0+0x2E+k*2. Mask table 0x080CC1A8.
_08024740:
	adds	r3, r2, #0
	cmp	r0, #1
	bne	_08024762
	ldr	r0, _08024764           @ =0x030013D0
	lsls	r2, r1, #1
	adds	r0, #46                 @ +0x2E
	adds	r2, r2, r0              @ &buf[0x2E + k*2]
	ldr	r0, _08024768           @ =0x080CC1A8
	lsls	r1, r3, #1
	adds	r1, r1, r0              @ &mask[k]
	ldrh	r0, [r2]
	ldrh	r3, [r1]
	bics	r0, r3
	strh	r0, [r2]
	ldrh	r1, [r1]
	orrs	r0, r1
	strh	r0, [r2]
_08024762:
	bx	lr
	.align 2, 0
_08024764: .4byte 0x030013D0
_08024768: .4byte 0x080CC1A8

@ ----------------------------------------------------------------------------
@ _0802476C(val, k) -> 0/1 - test mask bit `k` in packed halfword
@ buf[0x2E + val*2] (mask table 0x080CC1A8). Note swapped roles vs setter.
_0802476C:
	ldr	r2, _0802478C           @ =0x030013D0
	lsls	r0, r0, #1
	adds	r2, #46
	adds	r0, r0, r2
	ldr	r2, _08024790           @ =0x080CC1A8
	lsls	r1, r1, #1
	adds	r1, r1, r2
	ldrh	r1, [r1]
	ldrh	r0, [r0]
	ands	r1, r0
	lsls	r1, r1, #16
	asrs	r1, r1, #16
	negs	r0, r1
	orrs	r0, r1
	lsrs	r0, r0, #31
	bx	lr
	.align 2, 0
_0802478C: .4byte 0x030013D0
_08024790: .4byte 0x080CC1A8

@ ----------------------------------------------------------------------------
@ _08024794 - pack the current garage record's part fields into block #2
@ buffer at +0x2E..+0x39. Current record index = s16 at 0x03001CF4;
@ record i base = 0x03001780 + i*12; part bytes at +0x31..+0x39 map to
@ mask slots k = 0..5 via _08024740 (+0x30->k0, +0x35->k1, +0x33->k2,
@ +0x37->k3, +0x38->k4, +0x39->k5), then fixed-bit extraction for
@ +0x32/+0x36/+0x34/+0x31 through tables 0x080CC1C8 (hw) / 0x080CC1D8 (byte).
_08024794:
	push	{r4, r5, lr}
	ldr	r2, _08024878           @ =0x03001780
	ldr	r1, _0802487C           @ =0x00000574
	adds	r0, r2, r1              @ -> 0x03001CF4
	movs	r3, #0
	ldrsh	r1, [r0, r3]            @ current record index
	movs	r0, #0
	ldr	r4, _08024880           @ =0x030013D0
	strh	r0, [r4, #46]           @ packed = 0
	lsls	r0, r1, #1
	adds	r0, r0, r1
	lsls	r0, r0, #2              @ idx*12
	adds	r5, r0, r2              @ record base
	adds	r0, r5, #0
	adds	r0, #48                 @ +0x30
	ldrb	r0, [r0]
	lsls	r0, r0, #24
	asrs	r0, r0, #24
	movs	r1, #0
	movs	r2, #0
	bl	_08024740               @ apply(rec[+0x30], k=0)
	adds	r0, r5, #0
	adds	r0, #53                 @ +0x35
	ldrb	r0, [r0]
	lsls	r0, r0, #24
	asrs	r0, r0, #24
	movs	r1, #0
	movs	r2, #1
	bl	_08024740
	adds	r0, r5, #0
	adds	r0, #51                 @ +0x33
	ldrb	r0, [r0]
	lsls	r0, r0, #24
	asrs	r0, r0, #24
	movs	r1, #0
	movs	r2, #2
	bl	_08024740
	adds	r0, r5, #0
	adds	r0, #55                 @ +0x37
	ldrb	r0, [r0]
	lsls	r0, r0, #24
	asrs	r0, r0, #24
	movs	r1, #0
	movs	r2, #3
	bl	_08024740
	adds	r0, r5, #0
	adds	r0, #56                 @ +0x38
	ldrb	r0, [r0]
	lsls	r0, r0, #24
	asrs	r0, r0, #24
	movs	r1, #0
	movs	r2, #4
	bl	_08024740
	adds	r0, r5, #0
	adds	r0, #57                 @ +0x39
	ldrb	r0, [r0]
	lsls	r0, r0, #24
	asrs	r0, r0, #24
	movs	r1, #0
	movs	r2, #5
	bl	_08024740
	ldr	r2, _08024884           @ =0x080CC1C8
	ldrh	r1, [r4, #46]           @ packed
	ldrh	r0, [r2, #4]
	bics	r1, r0
	strh	r1, [r4, #46]
	adds	r0, r5, #0
	adds	r0, #50                 @ +0x32
	ldrb	r0, [r0]
	lsls	r0, r0, #24
	asrs	r0, r0, #24
	lsls	r0, r0, #4
	orrs	r0, r1                  @ rec[+0x32] << 4
	ldrh	r1, [r2, #6]
	bics	r0, r1
	strh	r0, [r4, #46]
	adds	r1, r5, #0
	adds	r1, #54                 @ +0x36
	ldrb	r1, [r1]
	lsls	r1, r1, #24
	asrs	r1, r1, #24
	lsls	r1, r1, #6
	orrs	r1, r0                  @ rec[+0x36] << 6
	ldrh	r2, [r2, #8]
	bics	r1, r2
	strh	r1, [r4, #46]
	adds	r0, r5, #0
	adds	r0, #52                 @ +0x34
	ldrb	r0, [r0]
	lsls	r0, r0, #24
	asrs	r0, r0, #24
	lsls	r0, r0, #8
	orrs	r0, r1                  @ rec[+0x34] << 8
	ldr	r1, _08024888           @ =0x080CC1D8
	ldrh	r1, [r1]                @ byte-table entry as halfword
	bics	r0, r1
	strh	r0, [r4, #46]
	adds	r1, r5, #0
	adds	r1, #49                 @ +0x31
	ldrb	r1, [r1]
	lsls	r1, r1, #24
	asrs	r1, r1, #24
	orrs	r0, r1                  @ rec[+0x31] raw
	strh	r0, [r4, #46]
	pop	{r4, r5}
	pop	{r0}
	bx	r0
	movs	r0, r0
	.align 2, 0
_08024878: .4byte 0x03001780
_0802487C: .4byte 0x00000574
_08024880: .4byte 0x030013D0
_08024884: .4byte 0x080CC1C8
_08024888: .4byte 0x080CC1D8

@ ----------------------------------------------------------------------------
@ _0802488C - inverse of _08024794: read packed garage halfword
@ buf+0x2E back into the current record's part bytes.
_0802488C:
	push	{r4, r5, lr}
	ldr	r5, _08024938           @ =0x03001780
	ldr	r1, _0802493C           @ =0x00000574
	adds	r0, r5, r1              @ -> 0x03001CF4
	movs	r3, #0
	ldrsh	r4, [r0, r3]            @ current record index
	movs	r0, #0
	movs	r1, #0
	bl	_0802476C               @ test(0, k=0)
	lsls	r1, r4, #1
	adds	r1, r1, r4
	lsls	r1, r1, #2              @ idx*12
	adds	r4, r1, r5              @ record base
	adds	r1, r4, #0
	adds	r1, #48                 @ +0x30
	strb	r0, [r1]
	movs	r0, #0
	movs	r1, #1
	bl	_0802476C
	adds	r1, r4, #0
	adds	r1, #53                 @ +0x35
	strb	r0, [r1]
	movs	r0, #0
	movs	r1, #2
	bl	_0802476C
	adds	r1, r4, #0
	adds	r1, #51                 @ +0x33
	strb	r0, [r1]
	movs	r0, #0
	movs	r1, #3
	bl	_0802476C
	adds	r1, r4, #0
	adds	r1, #55                 @ +0x37
	strb	r0, [r1]
	movs	r0, #0
	movs	r1, #4
	bl	_0802476C
	adds	r1, r4, #0
	adds	r1, #56                 @ +0x38
	strb	r0, [r1]
	movs	r0, #0
	movs	r1, #5
	bl	_0802476C
	adds	r1, r4, #0
	adds	r1, #57                 @ +0x39
	strb	r0, [r1]
	ldr	r2, _08024940           @ =0x030013D0
	ldr	r1, _08024944           @ =0x080CC1C8
	ldrh	r0, [r1, #4]
	ldrh	r3, [r2, #46]           @ packed
	ands	r0, r3
	lsrs	r0, r0, #4
	adds	r3, r4, #0
	adds	r3, #50                 @ +0x32
	strb	r0, [r3]
	ldrh	r0, [r1, #6]
	ldrh	r3, [r2, #46]
	ands	r0, r3
	lsrs	r0, r0, #6
	adds	r3, r4, #0
	adds	r3, #54                 @ +0x36
	strb	r0, [r3]
	ldrh	r0, [r1, #8]
	ldrh	r3, [r2, #46]
	ands	r0, r3
	lsrs	r0, r0, #8
	adds	r1, r4, #0
	adds	r1, #52                 @ +0x34
	strb	r0, [r1]
	ldr	r0, _08024948           @ =0x080CC1D8
	ldrb	r1, [r0]
	ldrh	r2, [r2, #46]
	ands	r1, r2
	adds	r0, r4, #0
	adds	r0, #49                 @ +0x31
	strb	r1, [r0]
	pop	{r4, r5}
	pop	{r0}
	bx	r0
	movs	r0, r0
	.align 2, 0
_08024938: .4byte 0x03001780
_0802493C: .4byte 0x00000574
_08024940: .4byte 0x030013D0
_08024944: .4byte 0x080CC1C8
_08024948: .4byte 0x080CC1D8

@ ----------------------------------------------------------------------------
@ _0802494C - pack 35 car/garage records (#0..#34; base 0x03001D64,
@ stride 72; fields +0 u32, +4 u16 low byte, +8, +9, +10) into block #2
@ buffer entries at 0x03001474 + i*8 ({u32; b0; b1; b2; b3} — all four
@ bytes stored).
_0802494C:
	push	{r4, r5, lr}
	movs	r4, #0
	ldr	r0, _080249A0           @ =0x030013D0
	mov	ip, r0
	mov	r3, ip
	adds	r3, #168                @ r3 = 0x03001478 (entry +4 bytes)
	ldr	r2, _080249A4           @ =0x03001780
	ldr	r1, _080249A8           @ =0x000005E4
	adds	r5, r2, r1              @ -> 0x03001D64 record base
_0802495E:
	lsls	r1, r4, #3
	mov	r0, ip
	adds	r0, #164                @ -> 0x03001474
	adds	r1, r1, r0              @ &entry[i]
	ldr	r0, [r5, #0]
	str	r0, [r1, #0]            @ entry.u32 = rec[+0]
	movs	r1, #189
	lsls	r1, r1, #3              @ 0x5E8
	adds	r0, r2, r1              @ rec+4 (u16)
	ldrh	r0, [r0, #0]
	strb	r0, [r3, #0]            @ entry.b0 = low byte only
	adds	r1, #4                  @ 0x5EC
	adds	r0, r2, r1              @ rec+8
	ldrb	r0, [r0, #0]
	strb	r0, [r3, #1]            @ entry.b1
	adds	r1, #1                  @ 0x5ED
	adds	r0, r2, r1              @ rec+9
	ldrb	r0, [r0, #0]
	strb	r0, [r3, #2]            @ entry.b2
	adds	r1, #1                  @ 0x5EE
	adds	r0, r2, r1              @ rec+10
	ldrb	r0, [r0, #0]
	strb	r0, [r3, #3]            @ entry.b3
	adds	r3, #8
	adds	r2, #72
	adds	r5, #72
	adds	r4, #1
	cmp	r4, #34
	ble	_0802495E
	pop	{r4, r5}
	pop	{r0}
	bx	r0
	movs	r0, r0
	.align 2, 0
_080249A0: .4byte 0x030013D0
_080249A4: .4byte 0x03001780
_080249A8: .4byte 0x000005E4

@ ----------------------------------------------------------------------------
@ _080249AC - inverse of _0802494C: scatter 35 buffer entries back over
@ the record array. Asymmetries preserved from original:
@ entry.b0 is written back as a halfword (sign-extended byte!) at rec+4,
@ b3 goes to rec+10, and rec+11 is zeroed.
_080249AC:
	push	{r4, r5, r6, r7, lr}
	movs	r4, #0
	ldr	r0, _08024A08           @ =0x030013D0
	mov	ip, r0
	movs	r6, #0
	mov	r3, ip
	adds	r3, #168                @ r3 walks entry+4 bytes
	ldr	r2, _08024A0C           @ =0x03001780
	ldr	r1, _08024A10           @ =0x000005E4
	adds	r5, r2, r1              @ record base 0x03001D64
_080249C0:
	lsls	r0, r4, #3
	mov	r1, ip
	adds	r1, #164                @ 0x03001474
	adds	r0, r0, r1              @ &entry[i]
	ldr	r0, [r0, #0]
	str	r0, [r5, #0]            @ rec[+0] = entry.u32
	movs	r1, #0
	ldrsb	r1, [r3, r1]            @ sign-extend entry.b0
	movs	r7, #189
	lsls	r7, r7, #3              @ 0x5E8
	adds	r0, r2, r7
	strh	r1, [r0, #0]            @ rec+4 (halfword)
	ldrb	r1, [r3, #1]
	adds	r7, #4                  @ 0x5EC
	adds	r0, r2, r7
	strb	r1, [r0, #0]            @ rec+8
	ldrb	r1, [r3, #2]
	adds	r7, #1                  @ 0x5ED
	adds	r0, r2, r7
	strb	r1, [r0, #0]            @ rec+9
	ldrb	r1, [r3, #3]
	adds	r7, #1                  @ 0x5EE
	adds	r0, r2, r7
	strb	r1, [r0, #0]            @ rec+10
	ldr	r1, _08024A14           @ =0x000005EF
	adds	r0, r2, r1
	strb	r6, [r0, #0]            @ rec+11 = 0
	adds	r3, #8
	adds	r2, #72
	adds	r5, #72
	adds	r4, #1
	cmp	r4, #34
	ble	_080249C0
	pop	{r4, r5, r6, r7}
	pop	{r0}
	bx	r0
	.align 2, 0
_08024A08: .4byte 0x030013D0
_08024A0C: .4byte 0x03001780
_08024A10: .4byte 0x000005E4
_08024A14: .4byte 0x000005EF

@ ----------------------------------------------------------------------------
@ _08024A18 - pre-save fixup for block #2: assemble the save image at
@ 0x030013D0 from the working area (head mirror, cursor/money cells,
@ garage pack, 100-byte blob, tail halfword, 34-entry pack).
_08024A18:
	push	{r4, r5, r6, lr}
	ldr	r4, _08024A88           @ =0x030013D0
	ldr	r5, _08024A8C           @ =0x03001780
	adds	r1, r4, #0
	adds	r0, r5, #0
	ldmia	r0!, {r2, r3, r6}
	stmia	r1!, {r2, r3, r6}
	ldmia	r0!, {r2, r3, r6}
	stmia	r1!, {r2, r3, r6}
	ldmia	r0!, {r2, r3}
	stmia	r1!, {r2, r3}
	ldmia	r0!, {r2, r3, r6}
	stmia	r1!, {r2, r3, r6}
	ldrh	r0, [r0, #0]            @ 44+2 = 46 bytes (0x2E) head mirror
	strh	r0, [r1, #0]
	movs	r6, #174
	lsls	r6, r6, #3              @ 0x570
	adds	r0, r5, r6              @ -> 0x03001CF0
	ldr	r0, [r0, #0]
	str	r0, [r4, #48]           @ buf+0x30
	ldr	r1, _08024A90           @ =0x00000574
	adds	r0, r5, r1              @ -> 0x03001CF4
	ldrh	r0, [r0, #0]
	strh	r0, [r4, #52]           @ buf+0x34
	ldr	r2, _08024A94           @ =0x00000576
	adds	r0, r5, r2              @ -> 0x03001CF6
	ldrh	r0, [r0, #0]
	strh	r0, [r4, #54]           @ buf+0x36
	bl	_08024794               @ pack garage -> buf+0x2E..
	movs	r3, #175
	lsls	r3, r3, #3              @ 0x578
	adds	r0, r5, r3              @ -> 0x03001CF8
	ldr	r0, [r0, #0]
	str	r0, [r4, #56]           @ buf+0x38
	adds	r0, r4, #0
	adds	r0, #60                 @ -> buf+0x3C
	adds	r6, #12                 @ 0x57C
	adds	r1, r5, r6              @ -> 0x03001CFC
	ldr	r2, [r1, #0]
	str	r2, [r4, #60]           @ first word manual
	movs	r2, #96
	bl	sub_0802E0A4            @ copy 96 more (blob -> buf+0x3C..0x9B)
	movs	r0, #188
	lsls	r0, r0, #3              @ 0x5E0
	adds	r5, r5, r0              @ -> 0x03001D60
	ldrh	r0, [r5, #0]
	adds	r4, #160                @ -> buf+0xA0
	strh	r0, [r4, #0]
	bl	_0802494C               @ pack 34 records -> buf+0xA4..
	pop	{r4, r5, r6}
	pop	{r0}
	bx	r0
	movs	r0, r0
	.align 2, 0
_08024A88: .4byte 0x030013D0
_08024A8C: .4byte 0x03001780
_08024A90: .4byte 0x00000574
_08024A94: .4byte 0x00000576

@ ----------------------------------------------------------------------------
@ _08024A98 - post-load fixup for block #2: distribute the save image
@ back over the working area (inverse of _08024A18).
_08024A98:
	push	{r4, r5, r6, lr}
	ldr	r5, _08024B04           @ =0x03001780
	ldr	r4, _08024B08           @ =0x030013D0
	adds	r1, r5, #0
	adds	r0, r4, #0
	ldmia	r0!, {r2, r3, r6}
	stmia	r1!, {r2, r3, r6}
	ldmia	r0!, {r2, r3, r6}
	stmia	r1!, {r2, r3, r6}
	ldmia	r0!, {r2, r3}
	stmia	r1!, {r2, r3}
	ldmia	r0!, {r2, r3, r6}
	stmia	r1!, {r2, r3, r6}
	ldrh	r0, [r0, #0]
	strh	r0, [r1, #0]            @ 46-byte head mirror back
	movs	r6, #174
	lsls	r6, r6, #3              @ 0x570
	adds	r1, r5, r6              @ -> 0x03001CF0
	ldr	r0, [r4, #48]
	str	r0, [r1, #0]
	ldrh	r1, [r4, #52]
	ldr	r2, _08024B0C           @ =0x00000574
	adds	r0, r5, r2
	strh	r1, [r0, #0]
	ldrh	r1, [r4, #54]
	ldr	r3, _08024B10           @ =0x00000576
	adds	r0, r5, r3
	strh	r1, [r0, #0]
	bl	_0802488C               @ unpack garage
	adds	r6, #8                  @ 0x578
	adds	r1, r5, r6              @ -> 0x03001CF8
	ldr	r0, [r4, #56]
	str	r0, [r1, #0]
	ldr	r1, _08024B14           @ =0x0000057C
	adds	r0, r5, r1              @ -> 0x03001CFC
	adds	r1, r4, #0
	adds	r1, #60                 @ buf+0x3C
	ldr	r2, [r4, #60]
	str	r2, [r0, #0]            @ first word manual
	movs	r2, #96
	bl	sub_0802E0A4            @ 96 bytes back (overlapping fwd copy)
	adds	r4, #160                @ buf+0xA0
	ldrh	r0, [r4, #0]
	movs	r2, #188
	lsls	r2, r2, #3              @ 0x5E0
	adds	r5, r5, r2              @ -> 0x03001D60
	strh	r0, [r5, #0]
	bl	_080249AC               @ scatter 34 records
	pop	{r4, r5, r6}
	pop	{r0}
	bx	r0
	.align 2, 0
_08024B04: .4byte 0x03001780
_08024B08: .4byte 0x030013D0
_08024B0C: .4byte 0x00000574
_08024B10: .4byte 0x00000576
_08024B14: .4byte 0x0000057C

@ --- constant-return stubs (op handlers) ------------------------------------
	.type _08024B18, %function
_08024B18:
sub_08024B18:
	movs	r0, #1
	bx	lr

	.type _08024B1C, %function
_08024B1C:
sub_08024B1C:
	movs	r0, #1
	bx	lr

	.type _08024B20, %function
_08024B20:
sub_08024B20:
	movs	r0, #1
	bx	lr

@ ----------------------------------------------------------------------------
@ _08024B24 -> 1 - load block #2 (idx=[0x030015CC], dst=0x030013D0);
@ on success run the post-load fixup.
_08024B24:
	push	{lr}
	ldr	r0, _08024B44           @ =0x030015CC
	ldrb	r0, [r0]
	ldr	r1, _08024B48           @ =0x030013D0
	bl	sub_08005988
	lsls	r0, r0, #24
	lsrs	r0, r0, #24
	cmp	r0, #1
	bne	_08024B3C
	bl	_08024A98
_08024B3C:
	movs	r0, #1
	pop	{r1}
	bx	r1
	movs	r0, r0
	.align 2, 0
_08024B44: .4byte 0x030015CC
_08024B48: .4byte 0x030013D0

@ --- no-op stubs -------------------------------------------------------------
@ These two are REAL 4-byte leaf entries (`bx lr` + the 2-byte align filler),
@ not padding and not part of _08024B24. They carry labels but no `.type`, so
@ tools/coverage.py's census -- which counts typed entry labels plus labels with
@ real BL targets -- never saw them, and _08024B24's span ran on to swallow
@ both. That inflated a 40-byte body to a 48-byte span and made an otherwise
@ promotable body score PARTIAL 40/48 with its neighbours' `bx lr` counted as
@ a miss. Byte-neutral: two markers, no ROM byte changes.
.type _08024B4C, %function
_08024B4C:
	bx	lr
	movs	r0, r0

.type _08024B50, %function
_08024B50:
	bx	lr
	movs	r0, r0

@ ----------------------------------------------------------------------------
@ _08024B54 - save block #2 (pre-save fixup first).
_08024B54:
	push	{lr}
	bl	_08024A18
	ldr	r0, _08024B68           @ =0x030015CC
	ldrb	r0, [r0]
	ldr	r1, _08024B6C           @ =0x030013D0
	bl	sub_080059F0
	pop	{r0}
	bx	r0
	.align 2, 0
_08024B68: .4byte 0x030015CC
_08024B6C: .4byte 0x030013D0

@ ----------------------------------------------------------------------------
@ _08024B70 - guarded full save op: IRQ-off (0x0802B234/0x0802B190),
@ zero the buffer, reload working area, run 0x0802446C hook, rebuild the
@ image, write to EEPROM, reload again, IRQ-on.
_08024B70:
	push	{r4, lr}
	sub	sp, #4
	bl	sub_0802B234
	bl	sub_0802B190
	movs	r0, #0
	str	r0, [sp]
	ldr	r4, _08024BB4           @ =0x030013D0
	ldr	r2, _08024BB8           @ =0x0500007F CpuSet 127 words
	mov	r0, sp
	adds	r1, r4, #0
	bl	sub_0802D974            @ zero buffer
	bl	_08024A98
	bl	sub_0802446C
	bl	_08024A18
	ldr	r0, _08024BBC           @ =0x030015CC
	ldrb	r0, [r0]
	adds	r1, r4, #0
	bl	sub_080059F0
	bl	_08024A98
	bl	sub_0802B1B8
	add	sp, #4
	pop	{r4}
	pop	{r0}
	bx	r0
	movs	r0, r0
	.align 2, 0
_08024BB4: .4byte 0x030013D0
_08024BB8: .4byte 0x0500007F
_08024BBC: .4byte 0x030015CC

@ ----------------------------------------------------------------------------
@ _08024BC0 / _08024BD8 - guarded save-only / load-only ops.
_08024BC0:
	push	{lr}
	bl	sub_0802B234
	bl	sub_0802B190
	bl	_08024B54
	bl	sub_0802B1B8
	pop	{r0}
	bx	r0
	movs	r0, r0

_08024BD8:
	push	{lr}
	bl	sub_0802B234
	bl	sub_0802B190
	bl	_08024B24
	bl	sub_0802B1B8
	pop	{r0}
	bx	r0
	movs	r0, r0

@ --- veneers into 0x080240D0 -------------------------------------------------
_08024BF0:
	push	{lr}
	bl	sub_080240D0
	pop	{r1}
	bx	r1
	movs	r0, r0

_08024BFC:
	push	{lr}
	bl	sub_080240D0
	pop	{r1}
	bx	r1
	movs	r0, r0

@ --- no-op stubs -------------------------------------------------------------
_08024C08:
	bx	lr
	movs	r0, r0

_08024C0C:
	bx	lr
	movs	r0, r0

_08024C10:
	bx	lr
	movs	r0, r0

_08024C14:
	bx	lr
	movs	r0, r0

_08024C18:
	bx	lr
	movs	r0, r0

@ ----------------------------------------------------------------------------
@ _08024C1C - guarded op calling the return-1 stubs with 0 and 1
@ (placeholder/no-op sequence).
_08024C1C:
	push	{lr}
	bl	sub_0802B234
	bl	sub_0802B190
	movs	r0, #0
	bl	_08024B18
	movs	r0, #1
	bl	_08024B18
	bl	sub_0802B1B8
	pop	{r0}
	bx	r0
	movs	r0, r0

saveblock_end:
