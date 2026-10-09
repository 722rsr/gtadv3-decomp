#include "gtadv/menus.h"
#include "gba/bios.h"

// ============================================================================
// menu_d4ea.s + menu_d8e4.s C lift — pause/stage state machines + transition
// leaves (VMAs 0x0800D4EA–0x0800D9A4; sub_0800D4EC / sub_0800D7C4 are already
// lifted in src/menus.c and are called here via their _0800 spellings).
//
// Every function below is transcribed instruction-for-instruction from the
// cited asm listing (labels = bare VMAs; all control flow preserved).
//
// Dispatcher table-decoding note (applies to sub_0800D664 / sub_0800D854):
// objdump2gas prints the literal-pool base word and the jump table as one
// contiguous.word run under the pool label. `ldr rN, pool` loads the FIRST
// word (= table base); entries live at base+4*i. So for D664: base
// 0x0800D67C, entries {D6B2,D6A8,D6D8,D6D8,D6CA,D6BA,D6D2,D6D8×4} for ids
// 1..11; for D854: base 0x0800D86C, entries {D8A8,D8B0,D8B8,D8DE×7,D8C0,
// D8C8,D8D0,D8D8,D8DE} for states 0..14.
//
// Shared block-B manager ABI (proven: block_b.c aliases + menus.c weak stubs;
// _0800 spellings bind strong lifted/asm symbols on ARM):
//   _08002140     -> s16 match count (0 when inactive)
//   _08002178(i,1)  -> u16 bit flags for match slot i (r1 dead in asm)
//   _080022D8(slot)-> "was match live" latch (int; r0 = slot, asm/idle.s:658)
//   _0800226C(rec12, 0x800) / _08002298(rec12)  -> d544 lane arms
//   _080022C0 -> pending?   _080022CC -> measure value (r0 -> r2 arg)
//   _08004EA8(v)/_08004EC0(v) -> manager store+step (r0 = arg)
//   _08004BFC(v)/_08004ED8(v) -> manager step (r0 set, unused by callee)
// Menu record layout (r4/r5 base, from asm field offsets):
//   +0 u16 state; +2 u16 sub-state; +4 u16 last-input; +8 u8 flag;
//   +10 u8 flag; +12 obj-record (0x05000200 palette fill target);
//   +16/+24 s32 y/x animation counters; +28/+32 u32 mode fields.
// ============================================================================

extern void _0800D4EC(void *rec);   // menus.c MenuD4EC_0800D4EC
extern void _0800D7C4(void *rec);   // menus.c MenuD7C4_0800D7C4 (*rec = 11)

#ifdef __APPLE__
__attribute__((weak)) void _0800D4EC(void *rec) { (void)rec; }
__attribute__((weak)) void _0800D7C4(void *rec) { *(volatile u32 *)rec = 11; }
__attribute__((weak)) void _0800226C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void _08002298(void *a) { (void)a; }
__attribute__((weak)) int  _080022C0(void) { return 0; }
__attribute__((weak)) int  _080022CC(void) { return 1; }
__attribute__((weak)) int  _080022D8(int slot) { (void)slot; return 0; }
__attribute__((weak)) void _08004EA8(int v) { (void)v; }
__attribute__((weak)) void _08004EC0(int v) { (void)v; }
__attribute__((weak)) void _08004BFC(int v) { (void)v; }
__attribute__((weak)) void _08004ED8(int v) { (void)v; }
__attribute__((weak)) u32  _08002140(void) { return 0; }
__attribute__((weak)) u16  _08002178(int x, int y) { (void)x; (void)y; return 0; }
__attribute__((weak)) void _08003F18(u32 a, u32 b, const volatile u8 *c, int d) { (void)a; (void)b; (void)c; (void)d; }
#else
// Strong callees on ARM: aliases from (block_b.c / course_resource.c /
// runtime_hud.c / menus.c) or the exact bodies.
extern int  _080022C0(void);
extern int  _080022CC(void);
extern int  _080022D8(int slot);          // r0 is the slot (asm/idle.s:658)
extern void _0800226C(void *a, int b);
extern void _08002298(void *a);
extern void _08004EA8(int v);
extern void _08004EC0(int v);
extern void _08004BFC(int v);
extern void _08004ED8(int v);
extern u32  _08002140(void);
extern u16  _08002178(int x, int y);         // r1 is dead in asm/blockb.s:126
extern void _08003978(int a, u32 b, int c);
extern void _08003940(int a, u32 b);
extern void _08003F18(u32 a, u32 b, const volatile u8 *c, int d);
extern void _08007664(void *a, void *b, int c);
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D544 (VMA 0x0800D544, 0x58 B) — pause-lane tick.
// state 0: match-live ? _0800226C(rec+12, 0x800) : _08002298(rec+12);
//   then u16[rec+0]++.
// state 1: if _080022C0 != 0 -> u16[rec+0]++ (asm 0x0800D57E `bne` ->
//   0x0800D590); else _08003978(70, 70, _080022CC) then RETURN WITHOUT
//   ticking (asm 0x0800D58E `b 0x0800D596`, the shared epilogue).
// other states return without ticking (asm 0x0800D554 `b 0x0800D596`).
// The state dispatch is a `switch`, not if/else-if: the ROM tests with two
// FORWARD `beq`s and a fallthrough `b` to the default, which is agbcc's switch
// compare-chain. if/else-if emits `bne` with the first arm in the fallthrough.
void MenuStage_0800D544(void *rec) {
    volatile u8 *r4 = (volatile u8 *)rec;
    s16 st = *(s16 *)(r4 + 0);
    switch (st) {
    case 0:
        if (_080022D8(0) != 0) {
            _0800226C((void *)(r4 + 12), 0x800);
        } else {
            _08002298((void *)(r4 + 12));
        }
        break;
    case 1:
        if (_080022C0() == 0) {
            int meas = _080022CC();
            _08003978(70, 70, meas);
            return;
        }
        break;
    default:
        return;
    }
    *(volatile u16 *)(r4 + 0) = (u16)(*(volatile u16 *)(r4 + 0) + 1);
}
#ifndef __APPLE__
void _0800D544(void *a) __attribute__((alias("MenuStage_0800D544")));
void sub_0800D544(void *a) __attribute__((alias("MenuStage_0800D544")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D59C (VMA 0x0800D59C, 8 B) — push{lr}/bl D544/pop tail
// veneer.
void MenuStage_0800D59C(void *rec) { _0800D544(rec); }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D59C(void *a) __attribute__((alias("MenuStage_0800D59C")));
void sub_0800D59C(void *a) __attribute__((alias("MenuStage_0800D59C")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D5A8 (VMA 0x0800D5A8, 0x3A B) — stage intro banner.
// asm: sub sp,#12; copy 12-byte ROM template 0x0805F920 to the stack buffer
// (dead — no callee reads caller stack; preserved for fidelity); then
// _08003940(4, 0x0805F92C) [song 4 + table], r4 = 0x0805F938,
// r3 = _08002140, then _08003F18(50, 30, r4, r3).
// The ROM copies the 12-byte block with a single `ldmia r0!,{r2,r3,r4}` /
// `stmia r1!,{r2,r3,r4}` pair (source in r0, destination `sp` in r1), i.e. a
// struct-style block copy through one base pointer. Three separately-addressed
// word reads emit three `ldr [pc]`/`ldr [r0]`/`str [sp]` triples and three
// extra pool words, which is the bug this shape fixes.
typedef struct { u32 w[3]; } MenuStageT12;
void MenuStage_0800D5A8(void *rec) {
    MenuStageT12 tmpl;
    // The ROM block copy reads the template through r0 and writes `sp`
    // through r1 (`ldr r0,_0800D5D8 / ldmia r0! / stmia r1!`), the reverse of
    // agbcc's default `dest=r0, src=r1`. Pinning the two pointer pseudos is
    // what reproduces the ROM's role assignment; it also lets agbcc reuse the
    // post-increment r0/r1 halfword for the following call pool load.
    register const MenuStageT12 *src __asm__("r0") =
        (const MenuStageT12 *)(uintptr_t)0x0805F920u;
    register MenuStageT12 *dst __asm__("r1") = &tmpl;
    *dst = *src;
    (void)tmpl; // dead stack copy in asm (ldmia/stmia to sp, never read)
    _08003940(4, (u32)0x0805F92C);
    _08003F18(50u, 30u, (const volatile u8 *)(uintptr_t)0x0805F938u, (int)_08002140());
    (void)rec;
}
#ifndef __APPLE__
void _0800D5A8(void *a) __attribute__((alias("MenuStage_0800D5A8")));
void sub_0800D5A8(void *a) __attribute__((alias("MenuStage_0800D5A8")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D5E4 (VMA 0x0800D5E4, 0x68 B) — input poll.
// for i in 0.._08002140-1: f = (u16)_08002178(i,1);
//   f&1 -> r6=1;  f&2 -> { if (i==0) r7=1; u16[rec+4]=2; u8[rec+10]=1; }
// r6 -> _08004BFC(1); _08004EC0(1);  else r7 -> _08004EA8(1); u8[rec+8]=r6.
// The call MUST pass the second argument: the ROM sets `movs r1, #1`
// (asm/menu_d4ea.s:147) even though asm/blockb.s:126-144 never reads r1
// (it is a dead store, the slot index reaching _08001E5C as r2 = r0). A
// 1-argument C call omits the `movs r1, #1` and loses 2 bytes. Declaring the
// parameter is the honest prototype for this call site; src/block_b.c's
// 1-argument `_08002178` alias definition is unchanged and unaffected, since
// the callee never reads r1.
void MenuStage_0800D5E4(void *rec) {
    volatile u8 *r5 = (volatile u8 *)rec;
    int r6 = 0, r7 = 0;
    u32 i = 0;
    while ((s32)i < (s32)_08002140()) {
        u16 f = (u16)(_08002178((int)i, 1) & 0xFFFF);
        if ((f & 1) != 0) r6 = 1;
        if ((f & 2) != 0) {
            if (i == 0) r7 = 1;
            *(volatile u16 *)(r5 + 4) = 2;
            *(volatile u8 *)(r5 + 10) = 1;
        }
        i++;
    }
    if (r6 != 0) {
        _08004BFC(1);
        _08004EC0(1);
    } else if (r7 != 0) {
        _08004EA8(1);
        *(volatile u8 *)(r5 + 8) = (u8)r6;
    }
}
// The body is byte-correct through `bx r0` (102/104, prefix 0x66); the last two
// bytes of the inventory span are the ROM's `00 00` inter-function pad
// (asm/menu_d4ea.s:192 `.hword 0x0000`). Under -ffunction-sections gas closes a
// Thumb code section with the 2-byte nop filler 0x46c0 instead. This
// file-scope `.align 2, 0` lands after the body's `.size` -- still inside the
// body's own section -- and pads with the explicit `0` fill.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D5E4(void *a) __attribute__((alias("MenuStage_0800D5E4")));
void sub_0800D5E4(void *a) __attribute__((alias("MenuStage_0800D5E4")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D64C (VMA 0x0800D64C, 4 B) — `bx lr` no-op leaf
// (dispatcher slot; args pass through unused).
void MenuStage_0800D64C(void *a, u16 b, u16 c) { (void)a; (void)b; (void)c; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D64C(void *a, u16 b, u16 c) __attribute__((alias("MenuStage_0800D64C")));
void sub_0800D64C(void *a, u16 b, u16 c) __attribute__((alias("MenuStage_0800D64C")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D650 (VMA 0x0800D650, 8 B) — `movs r0,#1; bl 0x08004ED8`
// wrapper (r0 = 1 set but unused by callee ABI).
void MenuStage_0800D650(void) { _08004ED8(1); }
#ifndef __APPLE__
void _0800D650(void) __attribute__((alias("MenuStage_0800D650")));
void sub_0800D650(void) __attribute__((alias("MenuStage_0800D650")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D65C (VMA 0x0800D65C, 8 B) — `adds r1,#88; movs r0,#0;
// strb r0,[r1]; bx lr`
// — the dispatcher at 0x0800D6A8 passes BOTH args: `adds r0,r3; adds r1,r4`, and
// the body zeroes u8[r1+88]. Thumb-1 add-immediate is `Rd = Rd + imm`, so r1 is
// the second parameter, not a fresh register — a one-parameter form allocates
// the address to r0 and the constant to r1 (reversed).
void MenuStage_0800D65C(void *rec, void *ctx) { *(volatile u8 *)((u8 *)ctx + 88) = 0; }
#ifndef __APPLE__
void _0800D65C(void *a, void *b) __attribute__((alias("MenuStage_0800D65C")));
void sub_0800D65C(void *a, void *b) __attribute__((alias("MenuStage_0800D65C")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D664 (VMA 0x0800D664, 0x7A B) — 11-entry event
// dispatcher (jump table 0x0800D67C, ids 1..11; guard `subs #1; cmp #10; bhi`).
// Register ABI: r0 = id, r1 = param1 (->r4), r2 = p2, r3 = rec.
// ids: 1 -> _0800D4EC(rec); 2 -> _0800D65C(rec, param1) (0x0800D6A8 passes
//      both: `adds r0,r3; adds r1,r4`); 3,4 -> nop;
//      5 -> _0800D59C(rec); 6 -> _0800D64C(rec, (u16)param1, (u16)p2);
//      7 -> _0800D5A8(rec); 8..11 -> nop (shared epilogue 0x0800D6D8).
void MenuStage_0800D664(u32 id, void *param1, u32 p2, void *rec) {
    switch (id) {
    case 2:
#ifndef __APPLE__
        _0800D65C(rec, param1);
#else
        MenuStage_0800D65C(rec, param1);
#endif
        break;
    case 1:
        _0800D4EC(rec);
        break;
    case 6:
#ifndef __APPLE__
        _0800D64C(rec, (u16)(uintptr_t)param1, (u16)p2);
#else
        MenuStage_0800D64C(rec, (u16)(uintptr_t)param1, (u16)p2);
#endif
        break;
    case 5:
#ifndef __APPLE__
        _0800D59C(rec);
#else
        MenuStage_0800D59C(rec);
#endif
        break;
    case 7:
#ifndef __APPLE__
        _0800D5A8(rec);
#else
        MenuStage_0800D5A8(rec);
#endif
        break;
    case 11:
        break;
    default:
        break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D664(u32 a, void *b, u32 c, void *d) __attribute__((alias("MenuStage_0800D664")));
void sub_0800D664(u32 a, void *b, u32 c, void *d) __attribute__((alias("MenuStage_0800D664")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D6E0 (VMA 0x0800D6E0, 0x24 B) — resource init:
// _08007664(0, 0x08292B40, 5); u32[rec+0] = 300; u32[rec+4] = 0.
void MenuStage_0800D6E0(void *rec) {
    _08007664(0, (void *)0x08292B40, 5);
    *(volatile u32 *)rec = 150 * 2;
    *((volatile u32 *)rec + 1) = 0;
}
#ifndef __APPLE__
void _0800D6E0(void *a) __attribute__((alias("MenuStage_0800D6E0")));
void sub_0800D6E0(void *a) __attribute__((alias("MenuStage_0800D6E0")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D704 (VMA 0x0800D704, 0x24 B) — wait-timer tick:
// s32[rec+0] > 0 -> decrement; else if s32[rec+4]==0 -> set 1, _08004EC0(1).
// Derived from `rec` directly, with no `volatile u8 *` base local: a named
// byte-pointer local makes agbcc materialise a SECOND base register for the
// rec+4 lvalue (`add r2, r1, #0`; `ldr r0, [r2, #4]`), but deriving both
// offsets from the parameter lets it hoist the single `adds r1, r0, #0` the
// ROM has and use `[r1]` / `[r1, #4]`. The accesses stay `volatile s32 *`.
void MenuStage_0800D704(void *rec) {
    s32 t = *(volatile s32 *)rec;
    if (t > 0) {
        *(volatile s32 *)rec = t - 1;
    } else if (*(volatile s32 *)((u8 *)rec + 4) == 0) {
        *(volatile s32 *)((u8 *)rec + 4) = 1;
        _08004EC0(1);
    }
}
// The body is byte-correct through `bx r0` (34/36, prefix 0x22); the last two
// bytes of the inventory span are the ROM's `00 00` inter-function pad
// (asm/menu_d4ea.s:314 `.hword 0x0000`). Same file-scope `.align 2, 0` idiom
// as MenuStage_0800D5E4 above: it lands after the body's `.size`, inside the
// body's own section, and pads with the explicit `0` fill.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D704(void *a) __attribute__((alias("MenuStage_0800D704")));
void sub_0800D704(void *a) __attribute__((alias("MenuStage_0800D704")));
#endif

// menu_d4ea.s sub_0800D728 (VMA 0x0800D728, 4 B) — `bx lr` no-op leaf.
void MenuStage_0800D728(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D728(void *a, u32 b, u32 c) __attribute__((alias("MenuStage_0800D728")));
void sub_0800D728(void *a, u32 b, u32 c) __attribute__((alias("MenuStage_0800D728")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D72C (VMA 0x0800D72C, 8 B) — `adds r1,#84; movs r0,#1;
// strh r0,[r1]; bx lr`. Thumb-1 add-immediate is `Rd = Rd + imm`, so the base
// is the SECOND parameter (r1), not a fresh register; the ROM caller at
// 0x0800D74E does `adds r0,r3; bl` and leaves r1 = the dispatcher's param1,
// so the callee is (rec, ctx) and u16[ctx+84] = 1.
void MenuStage_0800D72C(void *rec, void *ctx) { *(volatile u16 *)((u8 *)ctx + 84) = 1; }
#ifndef __APPLE__
void _0800D72C(void *a, void *b) __attribute__((alias("MenuStage_0800D72C")));
void sub_0800D72C(void *a, void *b) __attribute__((alias("MenuStage_0800D72C")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D734 (VMA 0x0800D734, 0x48 B) — event dispatcher
// {1:_0800D6E0(rec), 2:_0800D72C(rec,param1), 5:_0800D704(rec),
//  6:_0800D728(rec,(u16)param1,(u16)p2)}; ids 0/3/4 and >6 no-op.
// Guard note: 0x0800D774 holds an unreachable double epilogue
// (`pop {r0}; bx r0` then `bx lr`) — equivalent to the shared return here.
// Register ABI: r0 = id, r1 = param1, r2 = p2, r3 = rec.
// Arms are laid out 2,1,5,6 in the ROM, and agbcc follows the source order:
// listing the cases in that order reproduces the chain
//   cmp #2 / beq 0x1a; cmp #2 / bhi 0x10; cmp #1 / beq 0x22; b end
//   cmp #5 / beq 0x2a; cmp #6 / beq 0x32; b end
// and the four arms at +0x1a/+0x22/+0x2a/+0x32 (same recipe as the promoted
// 0x0800CFA0 dispatcher).
// Two levers beyond the case order, both measured:
//  * `p1` is pinned to r1. The ROM has NO `push {r4}`: r1 still holds param1
//    when the case-2 arm runs (its `bl` is the first branch taken), so nothing
//    needs a callee-saved copy. Unpinned, agbcc keeps param1 live across the
//    chain in r4 and emits `push {r4,lr}` / `add r4,r1` (84 B candidate).
//  * the case-6 narrowing is a u16 CAST, not `& 0xFFFF`: the mask form emits
//    `ldr r1,=0xFFFF / and` pairs where the ROM has `lsls #16 / lsrs #16`.
void MenuStage_0800D734(u32 id, void *param1, u32 p2, void *rec) {
    register void *p1 __asm__("r1") = param1;
    switch (id) {
    case 2: MenuStage_0800D72C(rec, p1); break;
    case 1: MenuStage_0800D6E0(rec); break;
    case 5: MenuStage_0800D704(rec); break;
    case 6: MenuStage_0800D728(rec, (u32)(u16)(uintptr_t)p1, (u32)(u16)p2); break;
    default: break;
    }
}
#ifndef __APPLE__
void _0800D734(u32 a, void *b, u32 c, void *d) __attribute__((alias("MenuStage_0800D734")));
void sub_0800D734(u32 a, void *b, u32 c, void *d) __attribute__((alias("MenuStage_0800D734")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D77C (VMA 0x0800D77C, 0xC B) — pair store leaf
// (u32[rec+0]=a, u32[rec+4]=b; callers in rec35_init.c/car_physics_core.c/
// rec35_runtime.c pass (r7+28, 0, 160) etc.).
void MenuStage_0800D77C(void *rec, u32 a, u32 b) {
    *(volatile u32 *)rec = a;
    *((volatile u32 *)rec + 1) = b;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D77C(void *a, u32 b, u32 c) __attribute__((alias("MenuStage_0800D77C")));
void Sub_0800D77C(void *a, u32 b, u32 c) __attribute__((alias("MenuStage_0800D77C")));
void sub_0800D77C(void *a, u32 b, u32 c) __attribute__((alias("MenuStage_0800D77C")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D784 (VMA 0x0800D784, 0x10 B) — state setter:
// u16[rec+4]=0, u32[rec+0]=1.
void MenuStage_0800D784(void *rec) {
    *(volatile u16 *)((u8 *)rec + 4) = 0;
    *(volatile u32 *)rec = 1;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D784(void *a) __attribute__((alias("MenuStage_0800D784")));
void sub_0800D784(void *a) __attribute__((alias("MenuStage_0800D784")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D790 (VMA 0x0800D790, 0x24 B) — slide-up tick:
// y(rec+16) -= 2, x(rec+24) += 4; snap x=0, y=144, state(rec+0) = 2 when
// y <= 144 OR x >= 0 (asm 0x0800D7A0 `ble` -> 0x0800D7A6, the snap block,
// then 0x0800D7A4 `blt` -> 0x0800D7B2, the epilogue). The second test really
// is `x >= 0`: a negative x falls through past the snap and returns. Writing
// the operand order as the ROM has it (`x >= 0`, not `x < 0`) is what makes
// agbcc emit `blt` to the epilogue instead of `bge` to the snap block.
void MenuStage_0800D790(void *rec) {
    volatile u8 *r2 = (volatile u8 *)rec;
    s32 y = *(volatile s32 *)(r2 + 16) - 2;
    *(volatile s32 *)(r2 + 16) = y;
    s32 x = *(volatile s32 *)(r2 + 24) + 4;
    *(volatile s32 *)(r2 + 24) = x;
    if (y <= 144 || x >= 0) {
        *(volatile s32 *)(r2 + 24) = 0;
        *(volatile s32 *)(r2 + 16) = 144;
        *(volatile s32 *)r2 = 2;
    }
}
#ifndef __APPLE__
void _0800D790(void *a) __attribute__((alias("MenuStage_0800D790")));
void sub_0800D790(void *a) __attribute__((alias("MenuStage_0800D790")));
#endif

// menu_d4ea.s sub_0800D7B4 (VMA 0x0800D7B4, 0x10 B) — u16[rec+4]=1,
// u32[rec+0]=3.
void MenuStage_0800D7B4(void *rec) {
    *(volatile u16 *)((u8 *)rec + 4) = 1;
    *(volatile u32 *)rec = 3;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D7B4(void *a) __attribute__((alias("MenuStage_0800D7B4")));
void sub_0800D7B4(void *a) __attribute__((alias("MenuStage_0800D7B4")));
#endif

// menu_d4ea.s sub_0800D7C0 (VMA 0x0800D7C0, 4 B) — `bx lr` no-op leaf.
void MenuStage_0800D7C0(void) { }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D7C0(void) __attribute__((alias("MenuStage_0800D7C0")));
void sub_0800D7C0(void) __attribute__((alias("MenuStage_0800D7C0")));
#endif

// (sub_0800D7C4 already lifted in src/menus.c: *(u32*)rec = 11)

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D7CC (VMA 0x0800D7CC, 0x34 B) — countdown tick:
// s32[rec+32]--; on <=0: state(rec+0)=12; dispatch rec+28 mode:
//   0 -> _08004EA8(s16[rec+10])   1 -> _08004EC0(s16[rec+8])
// The two s16 reads are NON-volatile: the ROM has `movs r2, #K; ldrsh r0,
// [r1, r2]`, and only a plain s16 lvalue gets that. A `volatile s16` read
// expands to `ldrh` + `lsl #16` + `asr #16` (2 extra bytes each) and forces
// agbcc to allocate a separate base register for rec+8. rec+10/rec+8 are
// EWRAM record fields, not MMIO, so dropping volatile costs no hardware
// contract; the surrounding word accesses keep theirs.
void MenuStage_0800D7CC(void *rec) {
    volatile u8 *r1 = (volatile u8 *)rec;
    s32 c = *(volatile s32 *)(r1 + 32) - 1;
    *(volatile s32 *)(r1 + 32) = c;
    if (c > 0) return;
    *(volatile s32 *)r1 = 12;
    s32 m = *(volatile s32 *)(r1 + 28);
    switch (m) {
    case 0:
        _08004EA8((int)*(s16 *)(r1 + 10));
        break;
    case 1:
        _08004EC0((int)*(s16 *)(r1 + 8));
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _0800D7CC(void *a) __attribute__((alias("MenuStage_0800D7CC")));
void sub_0800D7CC(void *a) __attribute__((alias("MenuStage_0800D7CC")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D800 (VMA 0x0800D800, 0x24 B) — slide-down tick:
// y(rec+16) += 2, x(rec+24) -= 4; when !(y <= 159) || x <= -32 snap
// x=-32, y=160 (state NOT changed).
void MenuStage_0800D800(void *rec) {
    volatile u8 *r2 = (volatile u8 *)rec;
    s32 y = *(volatile s32 *)(r2 + 16) + 2;
    *(volatile s32 *)(r2 + 16) = y;
    s32 x = *(volatile s32 *)(r2 + 24) - 4;
    *(volatile s32 *)(r2 + 24) = x;
    if (!(y <= 159) || x <= -32) {
        *(volatile s32 *)(r2 + 24) = -32;
        *(volatile s32 *)(r2 + 16) = 160;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D800(void *a) __attribute__((alias("MenuStage_0800D800")));
void sub_0800D800(void *a) __attribute__((alias("MenuStage_0800D800")));
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D828 (VMA 0x0800D828, 0x2C B) — mode-14 commit:
// dispatch rec+28 mode (0 -> _08004EA8(s16[rec+10]),
// 1 -> _08004EC0(s16[rec+8])), then state(rec+0) = 14.
// Non-volatile s16 reads + `switch` dispatch, as in MenuStage_0800D7CC above.
void MenuStage_0800D828(void *rec) {
    volatile u8 *r4 = (volatile u8 *)rec;
    s32 m = *(volatile s32 *)(r4 + 28);
    switch (m) {
    case 0:
        _08004EA8((int)*(s16 *)(r4 + 10));
        break;
    case 1:
        _08004EC0((int)*(s16 *)(r4 + 8));
        break;
    default:
        break;
    }
    *(volatile s32 *)r4 = 14;
}
#ifndef __APPLE__
void _0800D828(void *a) __attribute__((alias("MenuStage_0800D828")));
void sub_0800D828(void *a) __attribute__((alias("MenuStage_0800D828")));
#endif

// Call-site split for promoted bodies. `promotion_screen.py` only accepts a
// promoted body whose call targets the assembly closure defines, and the closure
// spells these `sub_0800D7xx`. On the host build the aliases do not exist
// (clang rejects `__attribute__((alias))`), so the friendly name is used there.
#ifndef __APPLE__
#define MS_CALLEE(friendly, closure) closure
#else
#define MS_CALLEE(friendly, closure) friendly
#endif

// ----------------------------------------------------------------------------
// menu_d4ea.s sub_0800D854 (VMA 0x0800D854, 0x8A B) — 15-entry state
// dispatcher (jump table 0x0800D86C, states 0..14; guard `cmp #14; bhi`):
// {0:_0800D784, 1:_0800D790, 2:_0800D7B4, 10:_0800D7C4(menus.c),
//  11:_0800D7CC, 12:_0800D800, 13:_0800D828}; states 3..9,14 nop.
void MenuStage_0800D854(void *rec) {
    u32 st = *(volatile u32 *)rec;
    // No explicit `st > 14` guard: the switch's own range check (min 0, max 14)
    // *is* the ROM's `cmp r0,#14; bhi`, and it only spans 0..14 when state 14 is
    // a real case label. With max 13 agbcc emits a second `cmp r0,#13; bhi` and
    // a 14-entry table, shifting everything after +0x0A.
    switch (st) {
    case 0: MenuStage_0800D784(rec); break;
    case 1: MS_CALLEE(MenuStage_0800D790, sub_0800D790)(rec); break;
    case 2: MenuStage_0800D7B4(rec); break;
    case 10: _0800D7C4(rec); break;
    case 11: MS_CALLEE(MenuStage_0800D7CC, sub_0800D7CC)(rec); break;
    case 12: MS_CALLEE(MenuStage_0800D800, sub_0800D800)(rec); break;
    case 13: MS_CALLEE(MenuStage_0800D828, sub_0800D828)(rec); break;
    case 14: break;          // -> 0x0800D8DE shared epilogue
    default: break;          // states 3..9 -> 0x0800D8DE shared epilogue
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D854(void *a) __attribute__((alias("MenuStage_0800D854")));
void Sub_0800D854(void *a) __attribute__((alias("MenuStage_0800D854")));
void sub_0800D854(void *a) __attribute__((alias("MenuStage_0800D854")));
void Menu_D854(void *a) __attribute__((alias("MenuStage_0800D854"))); // rec35_runtime.c spelling
#endif

// ============================================================================
// menu_d8e4.s (VMA 0x0800D8E4–0x0800D9A4) — menu stage transition family.
// rec: u16 +0 (counter A), u16 +2 (counter B), u16 +4 (phase/visible),
// u32 +8 (peer state word ptr), u32 +12 (peer record ptr).
// ============================================================================

// ----------------------------------------------------------------------------
// menu_d8e4.s sub_0800D8E4 (VMA 0x0800D8E4, 0x78 B) — transition engine.
// phase 1: u16[+2]=2, u16[+0]=3, *u32[+8] = 1 (r3 = phase), u16[+4]=2
//          (r0 = 2 at the shared store label 0x0800D958).
// phase 2: if (--u16[+0] wraps <= 0 && --u16[+2] wraps <= 0) u16[+4]=4;
//          flip *u32[+8] 0<->1 (other values untouched, `bne` skips store);
//          u16[+0]=3. (+4 store label NOT reached: `b 0x0800D95A`.)
// phase 4: *u32[+12] = {+0:10, +32:10, +28:1}; *u32[+8] = 1; u16[+4]=3.
// phases 0/3: no-op (all comparisons miss, straight to `bx lr`).
// The ROM opens with `movs r0,#4; ldrsh r3,[r2,r0]`: a NON-volatile signed
// halfword read. A `volatile s16` lvalue would emit `ldrh;lsls;asrs` instead.
// The dispatch is a `switch` (agbcc's split-at-2 compare tree: ==2, >2 -> 3/4,
// else ==1), not an if/else chain. Phase 2's decrement is 16-bit: `ldrh;subs;
// strh` then the sign test is the shifted-word `lsls #16;cmp #0;bgt`
// (equivalent to `(s16)v > 0`); the peer word flips 0<->1 through a SINGLE
// shared store, so the two arms assign a value and `goto` past the store on
// any other value.
void MenuStage_0800D8E4(void *rec_) {
    u8 *rec = (u8 *)rec_;
    s16 ph = *(s16 *)(rec + 4);
    switch (ph) {
    case 1:
        *(u16 *)(rec + 2) = 2;
        *(u16 *)(rec + 0) = 3;
        *(u32 *)*(u32 **)(rec + 8) = (u32)ph;
        *(u16 *)(rec + 4) = 2;
        break;
    case 3:
        break;
    case 2: {
        int a = *(u16 *)(rec + 0) - 1;
        *(u16 *)(rec + 0) = a;
        // `bgt _0800D95A` exits when counter A is still positive: the second
        // counter, the peer flip AND the +0=3 reload all live inside this arm.
        if ((a << 16) <= 0) {
            int b = *(u16 *)(rec + 2) - 1;
            *(u16 *)(rec + 2) = b;
            if ((b << 16) <= 0) {
                *(u16 *)(rec + 4) = 4;
            }
            {
                u32 *peer = *(u32 **)(rec + 8);
                u32 pv = *peer;
                // `if (pv != 0) { if (pv != 1) skip; pv = 0; } else pv = 1;`
                // is what puts the `pv = 1` arm out of line (ROM `beq` to it)
                // and the `pv = 0` arm inline, sharing one store.
                if (pv != 0) {
                    if (pv != 1) goto after_peer;
                    pv = 0;
                } else {
                    pv = 1;
                }
                *peer = pv;
            }
        after_peer:
            *(u16 *)(rec + 0) = 3;
        }
        break;
    }
    case 4: {
        u32 *p = *(u32 **)(rec + 12);
        p[0] = 10;
        p[8] = 10;
        p[7] = 1;
        *(u32 *)*(u32 **)(rec + 8) = 1;
        *(u16 *)(rec + 4) = 3;
        break;
    }
    default:
        break;
    }
}
#ifndef __APPLE__
void _0800D8E4(void *a) __attribute__((alias("MenuStage_0800D8E4")));
void Sub_0800D8E4(void *a) __attribute__((alias("MenuStage_0800D8E4")));
void sub_0800D8E4(void *a) __attribute__((alias("MenuStage_0800D8E4")));
void Menu_D8E4(void *a) __attribute__((alias("MenuStage_0800D8E4"))); // rec35_runtime.c spelling
#endif

// ----------------------------------------------------------------------------
// menu_d8e4.s sub_0800D95C (VMA 0x0800D95C, 0x16 B) — byte-copy leaf
// (dst=r0, src=r1, n=r2; byte loop `subs/bne`; n<=0 returns immediately).
void memcpy_0800D95C(u8 *dst, const u8 *src, int n) {
    if (n <= 0) return;
    do {
        *dst = *src;
        dst++;
        src++;
    } while (--n != 0);
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D95C(void *a, const void *b, int c) __attribute__((alias("memcpy_0800D95C")));
void sub_0800D95C(void *a, const void *b, int c) __attribute__((alias("memcpy_0800D95C")));
#endif

// ----------------------------------------------------------------------------
// menu_d8e4.s sub_0800D974 (VMA 0x0800D974, 8 B) — u16[rec+0] = 0 leaf.
void MenuStage_0800D974(void *rec) { *(volatile u16 *)rec = 0; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D974(void *a) __attribute__((alias("MenuStage_0800D974")));
void sub_0800D974(void *a) __attribute__((alias("MenuStage_0800D974")));
#endif

// ----------------------------------------------------------------------------
// menu_d8e4.s sub_0800D97C (VMA 0x0800D97C, 0x26 B) — rec+0--; when the u16
// decrement wraps negative: u16[rec+0] = r1 (reload value) and toggle
// s16[rec+2] {0->1, 1->0, other unchanged (bne skips store)}.
// Byte-match recipe (isolated agbcc lab): agbcc emits NO u16 truncation when the
// store's right-hand side is an *expression* and the value reaching the
// `volatile u16` MEM comes straight from a `ldrh` -- routing the decrement
// through a `u16` variable (`v = v - 1; store v`) inserts an `lsls/lsrs` mask
// the ROM does not have. The `s16[rec+2]` read must go through a plain
// (non-volatile) `s16 *` to get one `ldrsh r0,[r2,r1]` instead of `ldrh` plus a
// sign-extend pair.
void MenuStage_0800D97C(void *rec, int reload) {
    volatile u8 *r2 = (volatile u8 *)rec;
    u16 v;
    s16 s;
    v = *(volatile u16 *)(r2 + 0);
    *(volatile u16 *)(r2 + 0) = v - 1;
    v = v - 1;
    if ((s16)v < 0) {
        *(volatile u16 *)(r2 + 0) = reload;
        s = *(s16 *)(r2 + 2);
        if (s == 0) {
            *(volatile u16 *)(r2 + 2) = 1;
        } else if (s == 1) {
            *(volatile u16 *)(r2 + 2) = 0;
        }
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D97C(void *a, int b) __attribute__((alias("MenuStage_0800D97C")));
void Sub_0800D97C(void *a, int b) __attribute__((alias("MenuStage_0800D97C")));
void sub_0800D97C(void *a, int b) __attribute__((alias("MenuStage_0800D97C")));
#endif
