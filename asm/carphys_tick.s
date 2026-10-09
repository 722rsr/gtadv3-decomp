@ GT Advance 3 - scene-dispatch layer: car tick chain (records 27-30) +
@ the subsystem manager's record->record transition machinery (TASK 5A).
@ Regions: file offsets 0xA1D4-0xA204, 0xA204-0xA44C, 0xA62C-
@ 0xA668 (rec13 tick,), 0xA668-0xA9A0, 0xA9A0-0xAA20 (
@ grid helpers, converted in place), 0xAA20-0xAA40
@ (VMA 0x0800A1D4-, 0x0800AA20-0x0800AA40).
@
@ Disassembled via objdump/gbadisasm from baserom.gba; byte-exact.
@ Companions: asm/carphys_racer.s, src/foundation_subsys.c,
@             asm/carphys_tick.s.
@
@ Flow model (asm/carphys_tick.s): AgbMain's pump _08004B50 calls mgr->dispatch
@ = _0800A980(ctx), which switches on the context substate (+0x06):
@   0 -> _0800A668 per-record tick; 2 -> return 47; else -> _0800A94C.
@ The tick reads ctx record idx (+0), jumps through table _0800A688 and
@ RETURNS THE NEXT RECORD ID (step id): constant stubs implement fixed
@ chains (45->46->12->48->13...), call stubs delegate to per-record tick
@ fns, and the shared pop stub (_0800A8E0) pulls the next queued screen
@ off the manager byte-LIFO (_08004CC4 clear / _08004CD4 push / _08004CF0
@ pop). Record 7 sets r6=1 to re-queue itself; record 35 switches on the
@ global race phase halfword at 0x0300273C; records 37/38/47 are phase
@ loaders that enqueue record 13 when 0x03002804 ∈ {0,1}.

.thumb

@ ----------------------------------------------------------------------------
@ menu tick pocket VMA 0x08009BCC-0x08009F58 (file offsets
@ 0x009BCC-0x009F58), converted from raw passthrough incbin. Byte-exact
@ vs baserom (isolated assemble+link 908/908 bytes + make SHA gate).
@
@ Contents:
@   _08009BCC - record 51 tick (reset-flow gate)
@   _08009BF8 - record 0 tick (102-way key switch on ctx+0x2C)
@
@ Splice safety: xref.py finds exactly one BL caller each (_0800A774 /
@ _0800A75A, both already converted below in this file) and zero
@ literal-pool references to either entry; full-ROM word scan into
@ [0x9BCC,0x9F58) returns only the internal jump table + its base
@ literal. Every byte of the span is accounted for below.
@ NOTE: rec13's _08009B60 (ctx+0x2C==1 path -> id 50) sits at 0x9B60 and
@ ends (with its literal pool) exactly at 0x9BCC - it is NOT inside this
@ pocket; see asm/carphys_tick.s for its decoded semantics.

.thumb


.thumb

@ ----------------------------------------------------------------------------
@ _08009BCC - record 51 tick -> step id. Gate byte u8[wa+0x10B0]
@ (= u8[0x03002830]): when clear just pop the manager LIFO (passive
@ advance); when set, clear it, reset the LIFO counter and return 13 -
@ i.e. re-enter the boot flow at the continue/new-game fork.
_08009BCC:
	push {lr}
	ldr r0, r51_lit
	movs r2, #134			@ 0x86 << 5 = wa+0x10B0
	lsls r2, r2, #5
	adds r1, r0, r2
	ldrb r0, [r1, #0]
	cmp r0, #0
	bne _08009BE8
	bl sub_08004CF0			@ passive: step id = popped LIFO byte
	b _08009BF2
	movs r0, r0			@ pad halfword
	.align 2, 0
r51_lit:
	.word 0x03001780
_08009BE8:
	movs r0, #0
	strb r0, [r1, #0]		@ clear the reset gate
	bl sub_08004CC4			@ reset manager LIFO counter
	movs r0, #13			@ -> record 13 (continue/new-game fork)
_08009BF2:
	pop {r1}
	bx r1
	movs r0, r0			@ pad halfword

@ ----------------------------------------------------------------------------
@ _08009BF8(ctx) - record 0 tick -> step id. Switch on s32 key = ctx+0x2C,
@ 102-entry table below (keys 0..101; >101 -> stale r2=0).
@   key 0       : garage-record rebuild packet (see notes at handler)
@   key 1..5    : constant ids {1, 14, 4, 5, 8}
@   key 6..9    : racer-slot refresh packet (r4 = 0..3 sub-slot)
@   key 10..99  : constant id 0 (stale/no-op)
@   key 100     : constant id 6
@   key 101     : bl _08001F80(1); bl _0800D778; id 11
@ Handlers share one epilogue at _08009F24: bl _080188B0(sp packet),
@ then return 51.
_08009BF8:
	push {r4, r5, lr}
	sub sp, #64
	movs r4, #0
	movs r2, #0
	ldr r0, [r0, #44]		@ key = ctx+0x2C
	cmp r0, #101
	bls _08009C08			@ >101 -> stale: return r2 = 0
	b _08009F4C
_08009C08:
	lsls r0, r0, #2
	ldr r1, r0_lit
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	movs r0, r0			@ pad halfword
	.align 2, 0
r0_lit:
	.word rec0_table
	.align 2, 0
rec0_table:
	.word _08009DB0			@ key 0   garage-record rebuild packet
	.word _08009ECC			@ key 1   const 1
	.word _08009ED0			@ key 2   const 14
	.word _08009ED4			@ key 3   const 4
	.word _08009ED8			@ key 4   const 5
	.word _08009EDC			@ key 5   const 8
	.word _08009EE6			@ key 6   refresh packet, sub-slot 0
	.word _08009EE4			@ key 7   refresh packet, sub-slot 1
	.word _08009EE2			@ key 8   refresh packet, sub-slot 2
	.word _08009EE0			@ key 9   refresh packet, sub-slot 3
	.word _08009F4C			@ keys 10..99: no-op, id 0
	.rept 89
	.word _08009F4C
	.endr
	.word _08009F3C			@ key 100 const 6
	.word _08009F40			@ key 101 mode request + router, id 11

@ --- key 0: rebuild one garage record and post a UI packet --------------
@ idx = s16[wa+0x574]; record base = wa + idx*12 + 0x30 (12-byte garage
@ record array over IWRAM 0x03001780):
@   [0] = 0, [1] = u8[wa+0x1151], [2..9] = 0
@ then builds a 26-byte packet on the stack:
@   +0  = 0xFFFF          +2  = u16[wa+0x576]
@   +8  = u16[wa+0x113C]  +10 = u16[wa+0x113E]
@   +24 = bl 0x8002340(idx)      +28 = copy of the 12-byte record
@ zeroes sp+56, runs helper 0x802D974(dst=sp+56, src=sp, r2=0x0500000E),
@ falls into the shared epilogue (bl _080188B0(sp), return 51).
_08009DB0:
	ldr r4, ra_lit1			@ = 0x03001780
	ldr r0, ra_lit2			@ = 0x0574
	adds r5, r4, r0			@ r5 = &s16[wa+0x574]
	movs r2, #0
	ldrsh r1, [r5, r2]		@ idx
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2			@ idx*12
	adds r0, r0, r4
	adds r0, #48			@ &record[0]
	movs r2, #0
	strb r2, [r0, #0]
	movs r3, #0
	ldrsh r1, [r5, r3]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	ldr r3, ra_lit3			@ = 0x1151
	adds r1, r4, r3
	ldrb r1, [r1, #0]
	adds r0, #49			@ &record[1]
	strb r1, [r0, #0]
	movs r0, #0
	ldrsh r1, [r5, r0]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	adds r0, #50			@ &record[2]
	strb r2, [r0, #0]
	movs r3, #0
	ldrsh r1, [r5, r3]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	adds r0, #51			@ &record[3]
	strb r2, [r0, #0]
	movs r0, #0
	ldrsh r1, [r5, r0]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	adds r0, #52			@ &record[4]
	strb r2, [r0, #0]
	movs r3, #0
	ldrsh r1, [r5, r3]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	adds r0, #53			@ &record[5]
	strb r2, [r0, #0]
	movs r0, #0
	ldrsh r1, [r5, r0]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	adds r0, #54			@ &record[6]
	strb r2, [r0, #0]
	movs r3, #0
	ldrsh r1, [r5, r3]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	adds r0, #55			@ &record[7]
	strb r2, [r0, #0]
	movs r0, #0
	ldrsh r1, [r5, r0]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	adds r0, #56			@ &record[8]
	strb r2, [r0, #0]
	movs r3, #0
	ldrsh r1, [r5, r3]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	adds r0, #57			@ &record[9]
	strb r2, [r0, #0]
	str r2, [sp, #56]		@ zero sp+56
	add r0, sp, #56
	ldr r2, ra_lit4			@ = 0x0500000E
	mov r1, sp
	bl sub_0802D974			@ struct-init helper (raw region)
	mov r1, sp
	ldr r0, ra_lit5			@ = 0xFFFF
	strh r0, [r1, #0]		@ pkt+0
	ldr r2, ra_lit6			@ = 0x0576
	adds r0, r4, r2
	ldrh r0, [r0, #0]
	strh r0, [r1, #2]		@ pkt+2 = u16[wa+0x576]
	ldr r3, ra_lit7			@ = 0x113E
	adds r0, r4, r3
	ldrh r0, [r0, #0]
	strh r0, [r1, #10]		@ pkt+10
	ldr r2, ra_lit8			@ = 0x113C
	adds r0, r4, r2
	ldrh r0, [r0, #0]
	strh r0, [r1, #8]		@ pkt+8
	movs r3, #0
	ldrsh r0, [r5, r3]		@ idx
	bl 0x08002340			@ raw: id lookup
	mov r1, sp
	strh r0, [r1, #24]		@ pkt+24
	movs r0, #0
	ldrsh r1, [r5, r0]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r4
	add r1, sp, #28
	adds r0, #48
	ldmia r0!, {r2, r3, r4}		@ copy 12-byte record
	stmia r1!, {r2, r3, r4}		@ -> pkt+28
	b _08009F24
	movs r0, r0			@ pad halfword
	.align 2, 0
ra_lit1:
	.word 0x03001780
ra_lit2:
	.word 0x00000574
ra_lit3:
	.word 0x00001151
ra_lit4:
	.word 0x0500000E
ra_lit5:
	.word 0x0000FFFF
ra_lit6:
	.word 0x00000576
ra_lit7:
	.word 0x0000113E
ra_lit8:
	.word 0x0000113C

@ --- keys 1..5: constant ids --------------------------------------------
_08009ECC:
	movs r2, #1
	b _08009F4C

_08009ED0:
	movs r2, #14
	b _08009F4C

_08009ED4:
	movs r2, #4
	b _08009F4C

_08009ED8:
	movs r2, #5
	b _08009F4C

_08009EDC:
	movs r2, #8
	b _08009F4C

@ --- keys 6..9: racer-slot refresh packet --------------------------------
@ Three armcc-shared entry points add r4 += 1/2/3 so sub-slot r4 ends up
@ 0..3 for keys 6..9 respectively (key 6 enters straight at the body).
@ Packet: +0 = 10, +23 = sub-slot byte, +24 = bl 0x8002340(idx),
@ +28 = copy of garage record[idx] where idx = s16[wa+0x574];
@ zeroes sp+60, helper 0x802D974(sp+60 <- sp, 0x0500000E), shared tail.
_08009EE0:
	adds r4, #1
_08009EE2:
	adds r4, #1
_08009EE4:
	adds r4, #1
_08009EE6:
	movs r0, #0
	str r0, [sp, #60]
	add r0, sp, #60
	ldr r2, rb_lit1			@ = 0x0500000E
	mov r1, sp
	bl sub_0802D974
	mov r1, sp
	movs r0, #10
	strh r0, [r1, #0]		@ pkt+0 = 10
	mov r0, sp
	strb r4, [r0, #23]		@ pkt+23 = sub-slot
	ldr r5, rb_lit2			@ = 0x03001780
	ldr r0, rb_lit3			@ = 0x0574
	adds r4, r5, r0
	movs r1, #0
	ldrsh r0, [r4, r1]		@ idx
	bl 0x08002340
	mov r1, sp
	strh r0, [r1, #24]		@ pkt+24
	movs r2, #0
	ldrsh r1, [r4, r2]
	lsls r0, r1, #1
	adds r0, r0, r1
	lsls r0, r0, #2
	adds r0, r0, r5
	add r1, sp, #28
	adds r0, #48
	ldmia r0!, {r2, r3, r4}
	stmia r1!, {r2, r3, r4}
					@ fall through to shared epilogue
@ --- shared epilogue ------------------------------------------------------
@ Post the stack packet through the UI dispatcher used by _08009B60's
@ tail, then report step id 51.
_08009F24:
	mov r0, sp
	bl 0x080188B0			@ raw: UI packet consumer
	movs r2, #51
	b _08009F4C
	movs r0, r0			@ pad halfword
	.align 2, 0
rb_lit1:
	.word 0x0500000E
rb_lit2:
	.word 0x03001780
rb_lit3:
	.word 0x00000574

@ --- key 100 / key 101 ----------------------------------------------------
_08009F3C:
	movs r2, #6
	b _08009F4C

_08009F40:
	movs r0, #1
	bl 0x08001F80			@ raw: mode-request setter family
	bl 0x0800D778			@ raw: rec10 router helper
	movs r2, #11

@ --- common tail ----------------------------------------------------------
_08009F4C:
	adds r0, r2, #0
	add sp, #64
	pop {r4, r5}
	pop {r1}
	bx r1
	movs r0, r0			@ pad halfword

@############################################################################
@ TASK 5H - menu/results scene ROUTER ticks, VMA 0x08009F58-0x0800A1D4
@ (file offsets 0x009F58-0x00A1D4). Verified byte-exact vs baserom via
@ isolated assemble+link (636/636 bytes) before splicing here.
@############################################################################

@ ----------------------------------------------------------------------------
@ _08009F58 - record 2 (link test TST3-1): constant -> record 3 (TST4).
_08009F58:
	movs r0, #3
	bx lr

@ ----------------------------------------------------------------------------
@ _08009F5C - record 14 ROUTER (mode-select hub). key = _08004C0C:
@   0 -> 31, 1..4 -> 15, 5 -> 41, 6 -> 42, 7 -> 36, 8 -> 23.
_08009F5C:
	push {r4, lr}
	bl 0x08004C0C
	cmp r0, #8
	bhi _08009FAE
	lsls r0, r0, #2
	ldr r1, _r14_lit
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	.align 2, 0
_r14_lit:
	.word _r14_cases
_r14_cases:
	.word _08009F98			@ key 0 -> record 31
	.word _08009F9C			@ keys 1..4 -> record 15
	.word _08009F9C
	.word _08009F9C
	.word _08009F9C
	.word _08009FA0			@ key 5 -> record 41
	.word _08009FA4			@ key 6 -> record 42 (race scene)
	.word _08009FA8			@ key 7 -> record 36
	.word _08009FAC			@ key 8 -> record 23
_08009F98:
	movs r4, #31
	b _08009FAE
_08009F9C:
	movs r4, #15
	b _08009FAE
_08009FA0:
	movs r4, #41
	b _08009FAE
_08009FA4:
	movs r4, #42
	b _08009FAE
_08009FA8:
	movs r4, #36
	b _08009FAE
_08009FAC:
	movs r4, #23
_08009FAE:
	adds r0, r4, #0
	pop {r4}
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08009FB8 - record 23 ROUTER. key = s16[0x03002800]:
@   0 -> 24, 1 -> 34, 2 -> 37 (+ marks instance 37 field1 = 1), else stale.
_08009FB8:
	push {r4, lr}
	ldr r0, _r23_base
	movs r1, #132			@ 0x84 << 5 = 0x1080
	lsls r1, r1, #5
	adds r0, r0, r1
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #1
	beq _08009FE2
	cmp r0, #1
	bgt _08009FD8
	cmp r0, #0
	beq _08009FDE
	b _08009FF2
	.align 2, 0
_r23_base:
	.word 0x03001780
_08009FD8:
	cmp r0, #2
	beq _08009FE6
	b _08009FF2
_08009FDE:
	movs r4, #24
	b _08009FF2
_08009FE2:
	movs r4, #34
	b _08009FF2
_08009FE6:
	movs r4, #37
	movs r0, #37
	movs r1, #0
	movs r2, #1
	bl 0x08004C48
_08009FF2:
	adds r0, r4, #0
	pop {r4}
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _08009FFC - record 42 (race scene) ROUTER. key = _08004C0C:
@   <1 -> 15, ==2 -> 40, else stale.
_08009FFC:
	push {r4, lr}
	bl 0x08004C0C
	cmp r0, #1
	beq _0800A016
	cmp r0, #1
	bcc _0800A010
	cmp r0, #2
	beq _0800A014
	b _0800A016
_0800A010:
	movs r4, #15
	b _0800A016
_0800A014:
	movs r4, #40
_0800A016:
	adds r0, r4, #0
	pop {r4}
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ Constant routers.
_0800A020:				@ record 41 -> 44
	movs r0, #44
	bx lr

_0800A024:				@ record 32 -> 15
	movs r0, #15
	bx lr

@ ----------------------------------------------------------------------------
@ _0800A028 - record 31 ROUTER. idx = s16[0x03002776],
@ A = s16[0x03002772]; cell = s16[0x03002750 + idx*2 + A*8]:
@   cell == 0 -> store 1, return 32; else -> 15.
_0800A028:
	ldr r2, _r31_base
	ldr r1, _r31_o1
	adds r0, r2, r1
	movs r3, #0
	ldrsh r1, [r0, r3]
	lsls r1, r1, #1
	ldr r3, _r31_o2
	adds r0, r2, r3
	movs r3, #0
	ldrsh r0, [r0, r3]
	lsls r0, r0, #3
	adds r1, r1, r0
	movs r0, #253			@ 0xFD << 4 = 0xFD0
	lsls r0, r0, #4
	adds r2, r2, r0
	adds r1, r1, r2
	movs r2, #0
	ldrsh r0, [r1, r2]
	cmp r0, #0
	bne _0800A064
	movs r2, #32
	movs r0, #1
	strh r0, [r1, #0]
	b _0800A066
	.align 2, 0
_r31_base:
	.word 0x03001780
_r31_o1:
	.word 0x00000FF6
_r31_o2:
	.word 0x00000FF2
_0800A064:
	movs r2, #15
_0800A066:
	adds r0, r2, #0
	bx lr
	movs r0, r0

_0800A06C:				@ record 33 -> 15
	movs r0, #15
	bx lr

_0800A070:				@ record 34 -> 17
	movs r0, #17
	bx lr

@ ----------------------------------------------------------------------------
@ _0800A074 - record 15 POST-RACE ROUTER. key = ctx+0x2C:
@   <1 : switch u16[0x0300273C] (race phase), 8-way table below -
@        cases 0-4 -> phase-result handlers in the 0xA204+ pocket
@        ( region), case 5 -> stale-key tail, case 6 -> 17,
@        case 7 -> bl _0800B190 then 49.
@   ==1: u16[0x0300273C] == 7 ? 20 : 19
@   ==2: 22   ==3: 36   else: stale
_0800A074:
	push {lr}
	ldr r0, [r0, #44]
	cmp r0, #1
	beq _0800A0F4
	cmp r0, #1
	bcc _0800A08A
	cmp r0, #2
	beq _0800A110
	cmp r0, #3
	beq _0800A114
	b _0800A116
_0800A08A:
	ldr r1, _r15_base
	ldr r3, _r15_off
	adds r0, r1, r3
	movs r3, #0
	ldrsh r0, [r0, r3]
	adds r3, r1, #0
	cmp r0, #7
	bhi _0800A116
	lsls r0, r0, #2
	ldr r1, _r15_lit
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	.align 2, 0
_r15_base:
	.word 0x03001780
_r15_off:
	.word 0x00000FBC
_r15_lit:
	.word _r15_cases
	.align 2, 0
_r15_cases:
	.word _0800A0D0			@ phase 0 -> record 33
	.word _0800A0E8			@ phases 1..3 -> record 16
	.word _0800A0E8
	.word _0800A0E8
	.word _0800A116			@ phase 4 -> shared tail (key-stale)
	.word _0800A0F0			@ phase 5 -> record 17
	.word _0800A0D4			@ phase 6 -> bl _0800B190, record 49
	.word _0800A0DC			@ phase 7 -> sub-state 0x030027F8: 17 else 16
_0800A0D0:
	movs r2, #33
	b _0800A116
_0800A0D4:
	bl 0x0800B190
	movs r2, #49
	b _0800A116
_0800A0DC:
	ldr r1, _r15_o78
	adds r0, r3, r1
	movs r3, #0
	ldrsh r0, [r0, r3]
	cmp r0, #1
	beq _0800A0F0
_0800A0E8:
	movs r2, #16
	b _0800A116
	.align 2, 0
_r15_o78:
	.word 0x00001078
_0800A0F0:
	movs r2, #17
	b _0800A116
_0800A0F4:
	ldr r0, _r15_base2
	ldr r1, _r15_off2
	adds r0, r0, r1
	movs r2, #19
	ldrh r0, [r0, #0]
	cmp r0, #7
	bne _0800A116
	movs r2, #20
	b _0800A116
	movs r0, r0
	.align 2, 0
_r15_base2:
	.word 0x03001780
_r15_off2:
	.word 0x00000FBC
_0800A110:
	movs r2, #22
	b _0800A116
_0800A114:
	movs r2, #36
_0800A116:
	adds r0, r2, #0
	pop {r1}
	bx r1

@ ----------------------------------------------------------------------------
@ _0800A11C - record 22 tick: constant -> 26.
_0800A11C:
	movs r0, #26
	bx lr

_0800A120:				@ record 19 tick: constant -> 20
	movs r0, #20
	bx lr

@ ----------------------------------------------------------------------------
@ _0800A124 - record 20: drains the manager LIFO (once when race phase
@ == 7, twice otherwise) then routes to 15.
_0800A124:
	push {lr}
	ldr r0, _r20_base
	ldr r1, _r20_off
	adds r0, r0, r1
	ldrh r0, [r0, #0]
	cmp r0, #7
	bne _0800A140
	bl 0x08004CF0
	b _0800A148
	.align 2, 0
_r20_base:
	.word 0x03001780
_r20_off:
	.word 0x00000FBC
_0800A140:
	bl 0x08004CF0
	bl 0x08004CF0
_0800A148:
	movs r0, #15
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _0800A150 - record 16 (points/HUD family): switch race phase;
@ fires engine-command dispatcher _0800B190 (arg = s16[0x030027F8] on
@ some paths) and routes to 49, default returns 0 (re-tick).
_0800A150:
	push {lr}
	movs r1, #0
	ldr r0, _r16_base
	ldr r2, _r16_off
	adds r0, r0, r2
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #2
	beq _0800A17E
	cmp r0, #2
	bgt _0800A174
	cmp r0, #1
	beq _0800A190
	b _0800A196
	.align 2, 0
_r16_base:
	.word 0x03001780
_r16_off:
	.word 0x00000FBC
_0800A174:
	cmp r0, #3
	beq _0800A182
	cmp r0, #7
	beq _0800A186
	b _0800A196
_0800A17E:
	bl 0x0800B190
_0800A182:
	bl 0x0800B190
_0800A186:
	ldr r0, _r16_b2
	ldr r1, _r16_o2
	adds r0, r0, r1
	movs r2, #0
	ldrsh r0, [r0, r2]
_0800A190:
	bl 0x0800B190
	movs r1, #49
_0800A196:
	adds r0, r1, #0
	pop {r1}
	bx r1
	.align 2, 0
_r16_b2:
	.word 0x03001780
_r16_o2:
	.word 0x00001078

@ ----------------------------------------------------------------------------
@ Records 40 / 17 / 18: fire _0800B190 once, route to 49.
_0800A1A4:
	push {lr}
	bl 0x0800B190
	movs r0, #49
	pop {r1}
	bx r1

_0800A1B0:
	push {lr}
	bl 0x0800B190
	movs r0, #49
	pop {r1}
	bx r1

_0800A1BC:
	push {lr}
	bl 0x0800B190
	movs r0, #49
	pop {r1}
	bx r1

@ ----------------------------------------------------------------------------
@ _0800A1C8 - record 21 (menu scene): shared racer-style tick pump
@ (_0800AA20 pops one ring event / LIFO id); result = step id.
_0800A1C8:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0


@ ----------------------------------------------------------------------------
@ _0800A1D4 / _0800A1E0 / _0800A1EC / _0800A1F8 — racer tick thunks
@ (one per car slot; identical bodies). r0 = ctx from the caller stubs.
_0800A1D4:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

_0800A1E0:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

_0800A1EC:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

_0800A1F8:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ : per-record tick fns 0xA204-0xA44C converted (was raw pocket).
@ Veneer family + record-35 phase handlers; byte-exact vs baserom (SHA gate).
@
@ _0800AA20 = shared racer tick; its return value becomes the step id.
@ The two record-35 handlers switch on s16[0x030027FC] (race variant,
@ wa+0x107C) and enqueue LIFO programs {13,14,31,...}; their variant-2
@ case RE-SWITCHES on the global race phase u16[0x0300273C] (wa+0xFBC,
@ pool offset 0xFBC) to pick one extra queued id (16/17/18).
@ Unlike selectors _0800A44C/_0800A518 they fire no scene-ring events;
@ variant 0 calls engine-command dispatcher _0800B190 and returns 49.

@ rec38 tick: racer-tick veneer.
_0800A204:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

@ _0800A210: unreferenced twin of the veneer above (no BL/literal xrefs).
@ Entry label: no `.type %function` and no inbound `bl`, so this body was
@ invisible and inflated _0800A204's span. Zero bytes emitted.
	.type _0800A210, %function
_0800A210:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

@ rec36 tick: racer-tick veneer.
_0800A21C:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

@ rec37 tick: reset manager LIFO, enqueue 13, one racer tick.
_0800A228:
	push {lr}
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	bl _0800AA20
	pop {r1}
	bx r1

@ rec24 tick: racer-tick veneer.
_0800A23C:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

@ rec25 tick: racer-tick veneer.
_0800A248:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

@ _0800A254 - record-35 DEFAULT phase handler (stub _0800A8BA). Switch on
@ s16[0x030027FC]: 0 -> bl _0800B190, id 49; 1 -> {13,14,15};
@ 2 -> {13,14,15,+id by race phase}; 3 -> {13,14,15,19};
@ 4 -> {13,14,15,22}; >4 -> stale r1.
_0800A254:
	push {lr}
	ldr r0, _p35d_base
	ldr r2, _p35d_off
	adds r0, r0, r2
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #4
	bhi _0800A33E
	lsls r0, r0, #2
	ldr r1, _p35d_lit
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	movs r0, r0
	.align 2, 0
_p35d_base:
	.word 0x03001780
_p35d_off:
	.word 0x0000107C
_p35d_lit:
	.word _p35d_cases
_p35d_cases:
	.word _0800A290			@ variant 0: engine cmd, id 49
	.word _0800A298			@ variant 1: {13,14,15}
	.word _0800A2AC			@ variant 2: {13,14,15,+phase id}
	.word _0800A302			@ variant 3: {13,14,15,19}
	.word _0800A31C			@ variant 4: {13,14,15,22}
_0800A290:
	bl sub_0800B190
	movs r1, #49
	b _0800A33E
_0800A298:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #15
	b _0800A334
_0800A2AC:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	ldr r0, _p35d_b2
	ldr r1, _p35d_o2
	adds r0, r0, r1
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #1
	beq _0800A2F6
	cmp r0, #1
	bgt _0800A2E4
	cmp r0, #0
	beq _0800A2EE
	b _0800A338
	movs r0, r0
	.align 2, 0
_p35d_b2:
	.word 0x03001780
_p35d_o2:
	.word 0x00000FBC
_0800A2E4:
	cmp r0, #2
	beq _0800A2FA
	cmp r0, #5
	beq _0800A2F2
	b _0800A338
_0800A2EE:
	movs r0, #18
	b _0800A334
_0800A2F2:
	movs r0, #17
	b _0800A334
_0800A2F6:
	movs r0, #16
	b _0800A334
_0800A2FA:
	movs r0, #16
	bl sub_08004CD4
	b _0800A338
_0800A302:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	movs r0, #19
	b _0800A334
_0800A31C:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	movs r0, #22
_0800A334:
	bl sub_08004CD4
_0800A338:
	bl _0800AA20
	adds r1, r0, #0
_0800A33E:
	adds r0, r1, #0
	pop {r1}
	bx r1

@ _0800A344 - record-35 PHASE-0 handler (stub _0800A8A4). Same skeleton as
@ _0800A254 but the programs queue car id 31 instead of cmds 41/42/44:
@ 0 -> bl _0800B190, id 49; 1 -> {13,14,31,15};
@ 2 -> {13,14,31,15,+id by race phase}; 3 -> {...,19}; 4 -> {...,22}.
_0800A344:
	push {lr}
	ldr r0, _p350_base
	ldr r2, _p350_off
	adds r0, r0, r2
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #4
	bhi _0800A446
	lsls r0, r0, #2
	ldr r1, _p350_lit
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	movs r0, r0
	.align 2, 0
_p350_base:
	.word 0x03001780
_p350_off:
	.word 0x0000107C
_p350_lit:
	.word _p350_cases
_p350_cases:
	.word _0800A380			@ variant 0: engine cmd, id 49
	.word _0800A388			@ variant 1: {13,14,31,15}
	.word _0800A3A2			@ variant 2: {13,14,31,15,+phase id}
	.word _0800A3FE			@ variant 3: {13,14,31,15,19}
	.word _0800A41E			@ variant 4: {13,14,31,15,22}
_0800A380:
	bl sub_0800B190
	movs r1, #49
	b _0800A446
_0800A388:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #31
	bl sub_08004CD4
	movs r0, #15
	b _0800A43C
_0800A3A2:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #31
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	ldr r0, _p350_b2
	ldr r1, _p350_o2
	adds r0, r0, r1
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #1
	beq _0800A3F2
	cmp r0, #1
	bgt _0800A3E0
	cmp r0, #0
	beq _0800A3EA
	b _0800A440
	movs r0, r0
	.align 2, 0
_p350_b2:
	.word 0x03001780
_p350_o2:
	.word 0x00000FBC
_0800A3E0:
	cmp r0, #2
	beq _0800A3F6
	cmp r0, #5
	beq _0800A3EE
	b _0800A440
_0800A3EA:
	movs r0, #18
	b _0800A43C
_0800A3EE:
	movs r0, #17
	b _0800A43C
_0800A3F2:
	movs r0, #16
	b _0800A43C
_0800A3F6:
	movs r0, #16
	bl sub_08004CD4
	b _0800A440
_0800A3FE:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #31
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	movs r0, #19
	b _0800A43C
_0800A41E:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #31
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	movs r0, #22
_0800A43C:
	bl sub_08004CD4
_0800A440:
	bl _0800AA20
	adds r1, r0, #0
_0800A446:
	adds r0, r1, #0
	pop {r1}
	bx r1
@ ----------------------------------------------------------------------------
@ : race event selectors integrated from build/phys/race_selectors.ready.s
@ (transcribed). Span 0xA44C-0xA62C; trailing pad before _0800A62C.
@ GT Advance 3 - race event selectors (phase-keyed scene-command pushers)
@ Region: file offset 0xA44C-0xA62C (VMA 0x0800A44C-0x0800A62C).
@
@ Disassembled via objdump from baserom.gba; byte-exact.
@ Companion: asm/carphys_racer.s
@
@ Both selectors switch on s16[wa+0x107C] = u16[0x030027FC], the race
@ variant selected before the race starts. Each case pushes a short
@ "program" of byte commands onto the manager LIFO (_08004CD4 after
@ _08004CC4 reset) - these are the phase-change commands consumed by the
@ engine's soft-IRQ pump - optionally fires one scene-ring event
@ (_08023FF8), then runs one racer tick _0800AA20 and returns its step
@ id. Variant family A (wa+0x107C programs use cmd 42) and family B
@ (programs use cmds 41+44) are selected by different caller sites in
@ the menu/results module (0x801A616 / 0x801D226).
@
@ Program/event map:
@   _0800A44C: 0:sound-off,ret49  1:{13,14,42}        2:{13,14,42,15}+ev16
@              3:{13,14,42,15}+ev19            4:{13,14,42,15}+ev22
@   _0800A518: 0:sound-off,ret49  1:{13,14,41,44,15}
@              2:{13,14,41,44,15} + ev17 if u16[wa+0x1078]==1 / ev16 if ==2
@              3:{13,14,41,44,15}+ev20          4:{13,14,41,44,15}+ev22

.thumb

@ ----------------------------------------------------------------------------
@ _0800A44C -> step id — variant-A selector.
_0800A44C:
	push {lr}
	ldr r0, _0800A468 @ =0x03001780
	ldr r2, _0800A46C @ =0x0000107C
	adds r0, r0, r2
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #4
	bhi _0800A512
	lsls r0, r0, #2
	ldr r1, _0800A470 @ =_0800A474 (table base)
	adds r0, r0, r1
	ldr r0, [r0]
	mov pc, r0
	movs r0, r0
	.align 2, 0
_0800A468: .4byte 0x03001780
_0800A46C: .4byte 0x0000107C
_0800A470: .4byte _0800A474
@ --- selector A jump table ---
_0800A474:
	.4byte _0800A488   @ sel 0: sound off, ret 49
	.4byte _0800A490   @ sel 1: {13,14,42}
	.4byte _0800A4A8   @ sel 2: {13,14,42,15} + ev16
	.4byte _0800A4C8   @ sel 3: {13,14,42,15} + ev19
	.4byte _0800A4E8   @ sel 4: {13,14,42,15} + ev22

_0800A488:
	bl sub_0800B190      @ sound off
	movs r1, #49         @ 0x31
	b _0800A512

_0800A490:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #42         @ 0x2a
	bl sub_08004CD4
	b _0800A50C

_0800A4A8:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #42         @ 0x2a
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	movs r0, #16
	b _0800A506

_0800A4C8:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #42         @ 0x2a
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	movs r0, #19
	b _0800A506

_0800A4E8:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #42         @ 0x2a
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	movs r0, #22         @ 0x16
_0800A506:
	movs r1, #0
	bl _08023FF8         @ fire ring event
_0800A50C:
	bl _0800AA20         @ one racer tick; propagate its step id
	adds r1, r0, #0
_0800A512:
	adds r0, r1, #0
	pop {r1}
	bx r1

@ ----------------------------------------------------------------------------
@ _0800A518 -> step id — variant-B selector.
_0800A518:
	push {lr}
	ldr r0, _0800A534 @ =0x03001780
	ldr r2, _0800A538 @ =0x0000107C
	adds r0, r0, r2
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #4
	bls _0800A52A
	b _0800A624
_0800A52A:
	lsls r0, r0, #2
	ldr r1, _0800A53C @ =_0800A540 (table base)
	adds r0, r0, r1
	ldr r0, [r0]
	mov pc, r0
	.align 2, 0
_0800A534: .4byte 0x03001780
_0800A538: .4byte 0x0000107C
_0800A53C: .4byte _0800A540
@ --- selector B jump table ---
_0800A540:
	.4byte _0800A554   @ sel 0: sound off, ret 49
	.4byte _0800A55C   @ sel 1: {13,14,41,44,15}
	.4byte _0800A580   @ sel 2: {13,14,41,44,15} + ev17/ev16 by wa+0x1078
	.4byte _0800A5CE   @ sel 3: {13,14,41,44,15} + ev20
	.4byte _0800A5F4   @ sel 4: {13,14,41,44,15} + ev22

_0800A554:
	bl sub_0800B190      @ sound off
	movs r1, #49         @ 0x31
	b _0800A624

_0800A55C:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #41         @ 0x29
	bl sub_08004CD4
	movs r0, #44         @ 0x2c
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	b _0800A61E

_0800A580:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #41         @ 0x29
	bl sub_08004CD4
	movs r0, #44         @ 0x2c
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	ldr r0, _0800A5B8 @ =0x03001780
	ldr r1, _0800A5BC @ =0x00001078
	adds r0, r0, r1
	movs r2, #0
	ldrsh r0, [r0, r2]
	cmp r0, #1
	beq _0800A5C0
	cmp r0, #2
	beq _0800A5C4
	b _0800A61E
	movs r0, r0
	.align 2, 0
_0800A5B8: .4byte 0x03001780
_0800A5BC: .4byte 0x00001078
_0800A5C0:
	movs r0, #17         @ 0x11
	b _0800A618
_0800A5C4:
	movs r0, #16
	movs r1, #0
	bl _08023FF8
	b _0800A61E

_0800A5CE:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #41         @ 0x29
	bl sub_08004CD4
	movs r0, #44         @ 0x2c
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	movs r0, #20         @ 0x14
	b _0800A618

_0800A5F4:
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	movs r0, #14
	bl sub_08004CD4
	movs r0, #41         @ 0x29
	bl sub_08004CD4
	movs r0, #44         @ 0x2c
	bl sub_08004CD4
	movs r0, #15
	bl sub_08004CD4
	movs r0, #22         @ 0x16
_0800A618:
	movs r1, #0
	bl _08023FF8
_0800A61E:
	bl _0800AA20         @ one racer tick
	adds r1, r0, #0
_0800A624:
	adds r0, r1, #0
	pop {r1}
	bx r1
	movs r0, r0
@ ----------------------------------------------------------------------------
@ : record 13 tick converted (was raw pocket). Byte-exact vs baserom.
@
@ _0800A62C(ctx) - boot-flow decision point. key = s32 ctx+0x2C:
@   >1 -> return 0 (stale);
@   ==1 -> bl 0x08009B60(ctx), return 50;
@   ==0 -> consult request latch u16[0x03002806] (base 0x03001780 +
@          offset 0x1086 - NOT the "GT2" ASCII tag at 0x03002808):
@          ==1 -> clear it to 0, return 49; else return 14.
@ The latch is SET by UI-side code near 0x08018664 (strh 1 after sound
@ cmd _0802B368 + _0800B0BC/_08004BFC/_08004EC0 sequence) - see
@ asm/carphys_tick.s
@ ----------------------------------------------------------------------------
_0800A62C:
	push {lr}
	movs r1, #0
	ldr r0, [r0, #44]
	cmp r0, #1
	beq _0800A65C
	cmp r0, #1
	bcs _0800A662
	ldr r0, _p13_base
	ldr r1, _p13_off
	adds r2, r0, r1
	ldrh r0, [r2, #0]
	cmp r0, #1
	bne _0800A658
	movs r1, #49
	movs r0, #0
	strh r0, [r2, #0]
	b _0800A662
	.align 2, 0
_p13_base:
	.word 0x03001780
_p13_off:
	.word 0x00001086
_0800A658:
	movs r1, #14
	b _0800A662
_0800A65C:
	bl 0x08009B60
	movs r1, #50
_0800A662:
	adds r0, r1, #0
	pop {r1}
	bx r1

@ ----------------------------------------------------------------------------
@ _0800A668(ctx) - per-record tick dispatcher. Switches on ctx[+0]
@ (record idx, >51 -> pop stub) through the 52-word table below.
_0800A668:
	push {r4, r5, r6, lr}
	adds r5, r0, #0
	movs r6, #0
	movs r1, #0
	ldrsh r0, [r5, r1]
	cmp r0, #51
	bls _0800A678
	b _0800A8E0
_0800A678:
	lsls r0, r0, #2
	ldr r1, _lt_base
	adds r0, r0, r1
	ldr r0, [r0, #0]
	mov pc, r0
	movs r0, r0
	.align 2, 0
_lt_base:
	.word _tick_table

	.align 2, 0
_tick_table:
	.word _0800A758 @ rec0  bl 0x08009BF8
	.word _0800A8E0 @ rec1  (shared pop)
	.word _0800A762 @ rec2  bl 0x08009F58
	.word _0800A8E0 @ rec3
	.word _0800A8E0 @ rec4
	.word _0800A8E0 @ rec5
	.word _0800A8C2 @ rec6  const 9
	.word _0800A8CA @ rec7  self-requeue ? id10 : id8
	.word _0800A8E0 @ rec8
	.word _0800A8C6 @ rec9  const 7
	.word _0800A8D8 @ rec10 bl 0x0800D778, const 11
	.word _0800A8E0 @ rec11
	.word _0800A782 @ rec12 const 48
	.word _0800A78A @ rec13 bl _0800A62C
	.word _0800A794 @ rec14 bl 0x08009F5C
	.word _0800A7E2 @ rec15 bl 0x0800A074
	.word _0800A808 @ rec16 bl 0x0800A150
	.word _0800A81A @ rec17 bl 0x0800A1B0
	.word _0800A822 @ rec18 bl 0x0800A1BC
	.word _0800A7F6 @ rec19 bl 0x0800A120
	.word _0800A800 @ rec20 bl 0x0800A124
	.word _0800A82A @ rec21 bl 0x0800A1C8
	.word _0800A7EC @ rec22 bl 0x0800A11C
	.word _0800A79E @ rec23 bl 0x08009FB8
	.word _0800A86A @ rec24 bl 0x0800A23C
	.word _0800A872 @ rec25 bl 0x0800A248
	.word _0800A8E0 @ rec26
	.word _0800A832 @ rec27 bl _0800A1D4
	.word _0800A83A @ rec28 bl _0800A1E0
	.word _0800A842 @ rec29 bl _0800A1EC
	.word _0800A84A @ rec30 bl _0800A1F8
	.word _0800A7C6 @ rec31 bl 0x0800A028
	.word _0800A7D0 @ rec32 bl 0x0800A06C
	.word _0800A7D8 @ rec33 bl 0x0800A070
	.word _0800A8E0 @ rec34
	.word _0800A87A @ rec35 race-phase switch
	.word _0800A85A @ rec36 bl 0x0800A21C
	.word _0800A862 @ rec37 bl 0x0800A228
	.word _0800A852 @ rec38 bl _0800A204
	.word _0800A8E0 @ rec39
	.word _0800A812 @ rec40 bl 0x0800A1A4
	.word _0800A7B2 @ rec41 bl 0x0800A020
	.word _0800A7A8 @ rec42 bl 0x08009FFC
	.word _0800A8E0 @ rec43
	.word _0800A7BC @ rec44 bl 0x0800A024
	.word _0800A77A @ rec45 const 46 (boot -> 46)
	.word _0800A77E @ rec46 const 12
	.word _0800A8E0 @ rec47
	.word _0800A786 @ rec48 const 13
	.word _0800A76C @ rec49 bl 0x0800AF84
	.word _0800A8E0 @ rec50
	.word _0800A774 @ rec51 bl 0x08009BCC

@ --- record tick stubs -------------------------------------------------------
@ Call stubs: r0 = ctx; result = next record id. `b _0800A8EA` variants
@ join the tail past the r4 latch (r6==0 skips the self-push anyway).
_0800A758:
	adds r0, r5, #0
	bl _08009BF8
	adds r4, r0, #0
	b _0800A8EA

_0800A762:
	adds r0, r5, #0
	bl 0x08009F58
	adds r4, r0, #0
	b _0800A8EA

_0800A76C:
	adds r0, r5, #0
	bl 0x0800AF84
	b _0800A8E4

_0800A774:
	bl _08009BCC
	b _0800A8E4

@ constant chains: boot scene 45 -> 46 -> 12 -> 48 -> 13...
_0800A77A:
	movs r4, #46
	b _0800A8E6

_0800A77E:
	movs r4, #12
	b _0800A8E6

_0800A782:
	movs r4, #48
	b _0800A8E6

_0800A786:
	movs r4, #13
	b _0800A8E6

_0800A78A:
	adds r0, r5, #0
	bl _0800A62C
	adds r4, r0, #0
	b _0800A8EA

_0800A794:
	adds r0, r5, #0
	bl 0x08009F5C
	adds r4, r0, #0
	b _0800A8EA

_0800A79E:
	adds r0, r5, #0
	bl 0x08009FB8
	adds r4, r0, #0
	b _0800A8EA

_0800A7A8:
	adds r0, r5, #0
	bl 0x08009FFC
	adds r4, r0, #0
	b _0800A8EA

_0800A7B2:
	adds r0, r5, #0
	bl 0x0800A020
	adds r4, r0, #0
	b _0800A8EA

_0800A7BC:
	adds r0, r5, #0
	bl 0x0800A024
	adds r4, r0, #0
	b _0800A8EA

_0800A7C6:
	adds r0, r5, #0
	bl 0x0800A028
	adds r4, r0, #0
	b _0800A8EA

_0800A7D0:
	adds r0, r5, #0
	bl 0x0800A06C
	b _0800A8E4

_0800A7D8:
	adds r0, r5, #0
	bl 0x0800A070
	adds r4, r0, #0
	b _0800A8EA

_0800A7E2:
	adds r0, r5, #0
	bl 0x0800A074
	adds r4, r0, #0
	b _0800A8EA

_0800A7EC:
	adds r0, r5, #0
	bl 0x0800A11C
	adds r4, r0, #0
	b _0800A8EA

_0800A7F6:
	adds r0, r5, #0
	bl 0x0800A120
	adds r4, r0, #0
	b _0800A8EA

_0800A800:
	adds r0, r5, #0
	bl 0x0800A124
	b _0800A8E4

_0800A808:
	adds r0, r5, #0
	bl 0x0800A150
	adds r4, r0, #0
	b _0800A8EA

_0800A812:
	adds r0, r5, #0
	bl 0x0800A1A4
	b _0800A8E4

_0800A81A:
	adds r0, r5, #0
	bl 0x0800A1B0
	b _0800A8E4

_0800A822:
	adds r0, r5, #0
	bl 0x0800A1BC
	b _0800A8E4

_0800A82A:
	adds r0, r5, #0
	bl 0x0800A1C8
	b _0800A8E4

@ Records 27..30 tick stubs (jump-table targets 0x0800A832/3A/42/4A).
@ Each selects its own thunk then joins the common tail.
_0800A832:
	adds r0, r5, #0
	bl _0800A1D4
	b _0800A8E4

_0800A83A:
	adds r0, r5, #0
	bl _0800A1E0
	b _0800A8E4

_0800A842:
	adds r0, r5, #0
	bl _0800A1EC
	b _0800A8E4

_0800A84A:
	adds r0, r5, #0
	bl _0800A1F8
	b _0800A8E4

_0800A852:
	adds r0, r5, #0
	bl _0800A204
	b _0800A8E4

_0800A85A:
	adds r0, r5, #0
	bl _0800A21C
	b _0800A8E4

_0800A862:
	adds r0, r5, #0
	bl _0800A228
	b _0800A8E4

_0800A86A:
	adds r0, r5, #0
	bl _0800A23C
	b _0800A8E4

_0800A872:
	adds r0, r5, #0
	bl _0800A248
	b _0800A8E4

@ Record 35 stub: switch s16[0x0300273C] (global race phase):
@ 0 -> _0800A344, 3 -> _0800A44C, 7 -> _0800A518, default -> _0800A254.
_0800A87A:
	ldr r0, _rp_base
	ldr r1, _rp_off
	adds r0, r0, r1
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #3
	beq _0800A8B2
	cmp r0, #3
	bgt _0800A89C
	cmp r0, #0
	beq _0800A8A2
	b _0800A8BA
	movs r0, r0
	.align 2, 0
_rp_base:
	.word 0x03001780
_rp_off:
	.word 0x00000FBC
_0800A89C:
	cmp r0, #7
	beq _0800A8AA
	b _0800A8BA

_0800A8A2:
	adds r0, r5, #0
	bl _0800A344
	b _0800A8E4

_0800A8AA:
	adds r0, r5, #0
	bl _0800A518
	b _0800A8E4

_0800A8B2:
	adds r0, r5, #0
	bl _0800A44C
	b _0800A8E4

_0800A8BA:
	adds r0, r5, #0
	bl _0800A254
	b _0800A8E4

@ more constant stubs
_0800A8C2:
	movs r4, #9
	b _0800A8E6

_0800A8C6:
	movs r4, #7
	b _0800A8E6

@ Record 7 stub: sets r6=1 (self-requeue at tail), id = ctx+0x2C ? 10 : 8.
_0800A8CA:
	movs r6, #1
	ldr r0, [r5, #44]
	movs r4, #10
	cmp r0, #0
	bne _0800A8E6
	movs r4, #8
	b _0800A8E6

_0800A8D8:
	bl 0x0800D778
	movs r4, #11
	b _0800A8E6

@ Shared pop stub (records 1,3,4,5,8,11,26,34,39,43,47,50):
@ step id = popped manager LIFO byte (_08004CF0).
_0800A8E0:
	bl sub_08004CF0

@ Common tails. _0800A8E4 latches a returned value as the step id;
@ if r6 was set (record 7) the current record idx is pushed back onto
@ the manager LIFO before returning the id.
_0800A8E4:
	adds r4, r0, #0
_0800A8E6:
	cmp r6, #0
	beq _0800A8F2
_0800A8EA:
	movs r1, #0
	ldrsh r0, [r5, r1]
	bl sub_08004CD4
_0800A8F2:
	adds r0, r4, #0
	pop {r4, r5, r6}
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _0800A8FC - veneer to the shared racer tick (phase-loader path for rec 38).
_0800A8FC:
	push {lr}
	bl _0800AA20
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ _0800A908 - phase loader (record 37): when s16[0x03002804] ∈ {0,1},
@ reset the manager LIFO counter (_08004CC4) and enqueue record 13;
@ then return the popped id.
_0800A908:
	push {lr}
	ldr r0, _pl_base
	ldr r1, _pl_off
	adds r0, r0, r1
	movs r1, #0
	ldrsh r0, [r0, r1]
	cmp r0, #0
	beq _0800A926
	cmp r0, #1
	bne _0800A926
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
_0800A926:
	bl sub_08004CF0
	pop {r1}
	bx r1
	movs r0, r0
	.align 2, 0
_pl_base:
	.word 0x03001780
_pl_off:
	.word 0x00001084

@ ----------------------------------------------------------------------------
@ _0800A938 - phase loader (record 47): unconditionally reset LIFO,
@ enqueue 13, return popped id.
_0800A938:
	push {lr}
	bl sub_08004CC4
	movs r0, #13
	bl sub_08004CD4
	bl sub_08004CF0
	pop {r1}
	bx r1

@ ----------------------------------------------------------------------------
@ _0800A94C(ctx) - secondary dispatch on ctx[+0]: 38 -> racer veneer,
	.type _0800A94C, %function
_0800A94C:
	push {lr}
	movs r2, #0
	ldrsh r1, [r0, r2]
	cmp r1, #38
	beq _0800A966
	cmp r1, #38
	bgt _0800A960
	cmp r1, #37
	beq _0800A96C
	b _0800A978
_0800A960:
	cmp r1, #47
	beq _0800A972
	b _0800A978

_0800A966:
	bl _0800A8FC
	b _0800A97C

_0800A96C:
	bl _0800A908
	b _0800A97C

_0800A972:
	bl _0800A938
	b _0800A97C

_0800A978:
	bl sub_08004CF0
_0800A97C:
	pop {r1}
	bx r1

@ ----------------------------------------------------------------------------
@ _0800A980(ctx) - top-level scene dispatch, called by the scene pump
@ (_08004B50). Selects on the context substate halfword ctx+0x06:
@   0 -> per-record tick _0800A668 (returns next record id);
@   2 -> constant 47; else -> _0800A94C secondary dispatch.
@ Entry label: same reason as _0800A210 above. Zero bytes emitted.
	.type _0800A980, %function
_0800A980:
	push {lr}
	ldrh r1, [r0, #6]
	cmp r1, #0
	beq _0800A992
	cmp r1, #2
	beq _0800A998
	bl _0800A94C
	b _0800A99A
_0800A992:
	bl _0800A668
	b _0800A99A
_0800A998:
	movs r0, #47
_0800A99A:
	pop {r1}
	bx r1
	movs r0, r0

@ ----------------------------------------------------------------------------
@ : grid-aggregate helpers converted (were raw incbin pockets).
@ Span 0xA9A0-0xAA20, byte-exact vs baserom (make SHA256 gate).
@ Companion: asm/carphys_tick.s, asm/ai_collect.s (zone
@ grid). Splice safety: xref.py finds exactly four BL callers - two
@ converted (race FSM _0800AA40, asm/ai_racefsm.s @0xAAAC/0xAABA) and
@ two raw incbins (@0x242F2/0x24304) - and zero literal-pool refs.
@
@ Both helpers scan one row of a track-zone grid through the 2-bit cell
@ getter sub_08025CF4(type,row,col) (masks ROM 0x08060D48) and return
@ the minimum qualifying cell value: a cell participates only when its
@ u8 value v-1 <= 2 unsigned (v in {1,2,3}); the running minimum is
@ seeded at 3, so a row with no qualifying cell yields 3.
@   sub_0800A9A0(type,row): cols 0..2   (first grid block)
@   sub_0800A9E0(type,row): cols 3..10  (second grid block)
@ armcc high-register dance (r8 saved via r7 push/pop) + interworking
@ return; no literal pools.
	.thumb
	.type sub_0800A9A0, %function
sub_0800A9A0:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	mov r8, r0            @ type
	adds r7, r1, #0       @ row
	movs r6, #3           @ running min (seeded)
	movs r5, #0           @ last qualifying value
	movs r4, #0           @ col = 0..2
_0800A9B0:
	mov r0, r8
	adds r1, r7, #0
	adds r2, r4, #0
	bl sub_08025CF4       @ v = zone cell (type,row,col)
	lsls r0, r0, #24
	lsrs r1, r0, #24      @ r1 = (u8) v
	subs r0, r1, #1
	cmp r0, #2
	bhi _0800A9C6         @ v not in {1,2,3}: keep old candidate
	adds r5, r1, #0
_0800A9C6:
	cmp r5, r6
	bgt _0800A9CC
	adds r6, r5, #0       @ r6 = min(r6, r5)
_0800A9CC:
	adds r4, #1
	cmp r4, #2
	ble _0800A9B0
	adds r0, r6, #0       @ return min over block A (default 3)
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r1}
	bx r1
	movs r0, r0           @ pad halfword (0x0000)

	.thumb
	.type sub_0800A9E0, %function
sub_0800A9E0:
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	mov r8, r0            @ type
	adds r7, r1, #0       @ row
	movs r6, #3           @ running min (seeded)
	movs r5, #0           @ last qualifying value
	movs r4, #3           @ col = 3..10
_0800A9F0:
	mov r0, r8
	adds r1, r7, #0
	adds r2, r4, #0
	bl sub_08025CF4       @ v = zone cell (type,row,col)
	lsls r0, r0, #24
	lsrs r1, r0, #24      @ r1 = (u8) v
	subs r0, r1, #1
	cmp r0, #2
	bhi _0800AA06         @ v not in {1,2,3}: keep old candidate
	adds r5, r1, #0
_0800AA06:
	cmp r5, r6
	bgt _0800AA0C
	adds r6, r5, #0       @ r6 = min(r6, r5)
_0800AA0C:
	adds r4, #1
	cmp r4, #10
	ble _0800A9F0
	adds r0, r6, #0       @ return min over block B (default 3)
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r1}
	bx r1
	movs r0, r0           @ pad halfword (0x0000)

@ ----------------------------------------------------------------------------
@ _0800AA20 — shared racer tick body.
@ Calls sub_08024048(&out[2]) for a step id pair; when out[0] == -1 the
@ scene has no work left and the manager byte-stack is popped
@ (sub_08004CF0) to keep the pump in sync.
_0800AA20:
	push {lr}
	sub sp, #8
	mov r0, sp
	bl sub_08024048
	ldr r0, [sp]
	ldr r1, [sp, #4]
	movs r2, #1
	negs r2, r2
	cmp r0, r2
	bne _0800AA3A
	bl sub_08004CF0
_0800AA3A:
	add sp, #8
	pop {r1}
	bx r1
carphys_tick_end:
