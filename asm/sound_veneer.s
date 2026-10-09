@ GT Advance 3 - armcc register-branch veneer pool (relocated-code calls)
@ Region: file offset 0x02DDC8-0x02DE04 (VMA 0x0802DDC8-0x0802DE04).
@ Pool of 15 four-byte veneer entry points: `bx rN` + halfword pad
@ (mov r8,r8), reached by `ldr rN, <ram_addr>; bl veneer[rN]` to execute code
@ relocated to RAM (src/foundation_subsys.c). Each slot has its own typed entry
@ so corpus matching and ownership use the four-byte veneer span.
@
@ Caller census (tools/xref.py, BL sites only):
@   bx r0 (here)   : _08000908 in agbmain.s sub_08008F4 (session probe
@                    0x0203F110; asm/agbmain.s).
@   bx r1 (_0802DDCC): 0x080041A8, 0x08004B58 (raw), _0801AA10,
@                    _0801C468, _0801C4C6 (raw scene code),
@                    _0802C8A2/_0802C8B6 (asm/sound_init.s),
@                    _0802CB00 (asm/sound_reset_more.s).
@   bx r2 (_0802DDD0): 15 callers incl. ghost-scene dispatch
@                    (_0801C448/_0801C474/_0801C57A, subsystem manager
@                    0x08004F54..0x08005004, sound tail asm/sound_d6f4.s
@                    x3); see asm/ghost.s/src/foundation_subsys.c.
@   bx r3          : none.
@   bx r4 (_0802DDD8): event broadcast _08004D6C/_08004D7E/_08004D96/
@                    _08004DB6 (asm/agbmain.s scene-manager API, handler ptr
@                    in r4).
@   bx r5 (_0802DDDC): _08004F98 (raw).
@   bx r6..bx lr   : zero static references.
@ Literal-pool scan over every entry and pad address: only two offset-form
@ data coincidences (words 0x0002DDDD @0x5E2D4C, 0x0002DDEF @0x79E30C,
@ both inside asset data, not VMA-form pointers).

	.thumb
	.type sub_0802DDC8, %function
sub_0802DDC8:
_0802DDC8:
	bx r0
	.hword 0x46C0		@ pad (mov r8, r8)
.type _0802DDCC, %function
_0802DDCC:
	bx r1
	.hword 0x46C0
.type _0802DDD0, %function
_0802DDD0:
	bx r2
	.hword 0x46C0
.type _0802DDD4, %function
_0802DDD4:
	bx r3
	.hword 0x46C0
.type _0802DDD8, %function
_0802DDD8:
	bx r4
	.hword 0x46C0
.type _0802DDDC, %function
_0802DDDC:
	bx r5
	.hword 0x46C0
.type _0802DDE0, %function
_0802DDE0:
	bx r6
	.hword 0x46C0
.type _0802DDE4, %function
_0802DDE4:
	bx r7
	.hword 0x46C0
.type _0802DDE8, %function
_0802DDE8:
	bx r8
	.hword 0x46C0
.type _0802DDEC, %function
_0802DDEC:
	bx r9
	.hword 0x46C0
.type _0802DDF0, %function
_0802DDF0:
	bx r10
	.hword 0x46C0
.type _0802DDF4, %function
_0802DDF4:
	bx r11
	.hword 0x46C0
.type _0802DDF8, %function
_0802DDF8:
	bx r12
	.hword 0x46C0
.type _0802DDFC, %function
_0802DDFC:
	bx r13
	.hword 0x46C0
.type _0802DE00, %function
_0802DE00:
	bx r14
	.hword 0x46C0

@ Synthetic end anchor for the final four-byte veneer slot, at 0x0802DE04.
sound_veneer_end:

@ agbcc EMITS the name `_call_via_r2` for an indirect call through r2, so there
@ is no C call site to rename -- the symbol only ever appears in compiler output.
@ `.set` makes it resolve to the same veneer the closure already has, which is
@ what `build_c.py`'s alias pass does for every other compiler-emitted name.
@ Without this the link fails with `undefined reference to _call_via_r2`, which
@ is how found it: promotion_screen accepted the name (the runtime table
@ and the closure agreed on 0x0802DDD0) and the linker then disagreed.
	.thumb
	.set _call_via_r2, _0802DDD0
@ 0x02CACC calls the voice function through r1. agbcc emits `_call_via_r1`
@ for that indirect call; bind it to the existing bx-r1 veneer at 0x0802DDCC.
.set _call_via_r1, _0802DDCC
@ 0x08003560 / 0x08003350 dispatch their template pointer from r9 and reach
@ `bx r9` here. Same situation as _call_via_r2 above: agbcc emits the name
@ `_call_via_r9` itself, there is no C call site to rename, and without the
@ `.set` the link fails with `undefined reference to _call_via_r9`.
	.set _call_via_r9, _0802DDEC
@ 0x08004D4C dispatches its four broadcast targets from r4 and reaches `bx r4`
@ here. Same situation as r2 and r9 above: agbcc emits `_call_via_r4` itself,
@ there is no C call site to rename, and without the `.set` the link fails.
@ Derived from the runtime table in tools/era_runtime_probe.py:22 -- the
@ `__aeabi_call_via_rX` pool is 60 bytes at 0x0802DDC8, so the veneers are 4
@ apart and `_call_via_rN` is at 0x0802DDC8 + 4*N. Both existing entries follow
@ it (r2 -> 0x0802DDD0, r9 -> 0x0802DDEC), giving r4 -> 0x0802DDD8, which is
@ the `bl` src/foundation_subsys.c already documents for this body.
	.set _call_via_r4, _0802DDD8
