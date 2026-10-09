// ============================================================================
// race_scene_c.c — reconstructed C for asm/race_scene.s (20 functions):
//
//   0x0801E1E0 0x0801E230 0x0801E2D0 0x0801E304 0x0801E3BC 0x0801E454
//   0x0801E4C0 0x0801E4F4 0x0801E528 0x0801E56C 0x0801E5B0 0x0801E68C
//   0x0801E6D8 0x0801E718 0x0801E764 0x0801E8B4 0x0801E92C 0x0801E9DC
//   0x0801EA28 0x0801EB48
//
// Already lifted in src/race_scene.c — NOT redefined here (called as
// externals): 0801E338, 0801E364, 0801E390, 0801E9D8, 0801EBA4.
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

// External callees (strong lifts or trampolines).
HOST_STUB(void _08002B368(u16 v));                                              // 0x08002B368 sound cue
// 0x0802b368 is a PROMOTED entry whose manifest `export` is exactly
// ['sub_0802B368'], so a spliced body must call that spelling. src/sound.c
// already provides it as a `#ifndef __APPLE__` alias of `_0802B368`.
HOST_STUB(void sub_0802B368(u16 v));
HOST_STUB(int _08024068(void));                                                 // 0x08024068 record-kind probe
HOST_STUB(void _08002158(int id, u16 payload));                                 // 0x08002158 block-B forward
HOST_STUB(u16 _08002178(int x));                                                // 0x08002178 block-B dispatch-if-less
HOST_STUB(void _08002618(int a, int b));                                        // 0x08002618 scene event
HOST_STUB(void _08007B18(void *r, int k, int x, int y, u32 a, u32 b, u32 c, u32 d)); // 0x08007B18 8-arg emit
HOST_STUB(void Sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i)); // 0x08007BFC exact ROM
HOST_STUB(void sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i)); // 0x08007BFC exact ROM (closure spelling)
HOST_STUB(void Sub_08007C68(void *a, u32 b, u32 c, int d, int e, int f, int g, int h, int i)); // 0x08007C68 exact ROM (kept for unconverted sites)
extern void _08007C68(void *a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i); // faithful strong body (course_records.c)
HOST_STUB(void _08001D910(int a0, int a1, u32 a2, int a3, int a4));              // 0x08001D910 attr/param lane
HOST_STUB(void _08001D950(int a0, int a1, u32 a2, int a3, int a4));              // 0x08001D950 attr/param lane twin
HOST_STUB(void _08001D974(void *a, void *b));                                   // 0x08001D974 paired-record reset
HOST_STUB(void _08001DBC0(void *rec, int v));                                   // 0x08001DBC0 clamped lane select
HOST_STUB(void _08001DC8C(void *rec));                                          // 0x08001DC8C phase switcher
HOST_STUB(void _08001D9C8(void));                                               // 0x08001D9C8 bx-lr stub (race_scene.c)
HOST_STUB(void _08001DA70(int a0, int a1, u32 a2, int a3));                     // 0x08001DA70 gated emit
HOST_STUB(void _08003B78(int a, u32 b, u32 c));                                 // 0x08003B78 obj digits
HOST_STUB(void _08003838(int a, u32 b, const volatile u8 *c));                  // 0x08003838 obj lane (ROM word 0x0805FC1C)
HOST_STUB(int  _08002140(void));                                                 // 0x08002140 block-B match count
HOST_STUB(void _08004D4C(u32 a, u32 b, u32 c));                                 // 0x08004D4C scene broadcast
HOST_STUB(void *_08004B68(void));                                               // 0x08004B68 scene header
HOST_STUB(void _0800D97C(void *a, int b));                                      // 0x0800D97C record rebind
HOST_STUB(void _0800DBE8(void *a));                                             // 0x0800DBE8 record setup
HOST_STUB(void _0800D854(void *a));                                             // 0x0800D854 record tick
HOST_STUB(void _08001E338(void *rec));                                          // 0x08001E338 (race_scene.c)
HOST_STUB(void _08001E364(void *rec));                                          // 0x08001E364 (race_scene.c)
HOST_STUB(void _08001E390(void *rec));                                          // 0x08001E390 (race_scene.c)
HOST_STUB(void _08001E9D8(void *rec));                                         // 0x08001E9D8 bx-lr stub (race_scene.c)

// ---- shared anchors --------------------------------------------------------
#define WA        0x03001780u
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WA + (o)))
#define WA_U32(o) (*(volatile u32 *)(uintptr_t)(WA + (o)))

// ----------------------------------------------------------------------------
// ---- 0x08001E1E0 — key-gated scene setup leaf + record-kind latch ----
// Pools: {0xFFFF0000} ((u16)(c-1) via (c<<16 + 0xFFFF0000)>>16).
// r1 dead. Sets u16[rec+48]=1 iff probe in {27,28,29,30,38,43}.
//
// Two measured facts make this 80/80. Both are load-bearing.
//  1. The membership test is a single `switch` over the SIX consecutive-claim
//     case values, not the if/else-if spelling. agbcc's if/else expansion
//     folds `v >= 27 && v <= 30` to `subs #27 / cmp #3 / bhi` and threads the
//     38/43 tests inline (measured 38/80); the switch keeps the ROM's
//     decision tree `cmp #38 / beq / cmp #38 / bgt / cmp #30 / bgt /
//     cmp #27 / blt / b` and puts both arms out of line.
//  2. The first parameter must be typed `volatile u8 *` in the SIGNATURE, not
//     cast from a `void *`. agbcc then emits `adds r4, r0, #0` as the
//     prologue's first instruction, ahead of the `lsls r2,r2,#16`; with the
//     `void *` parameter it is a `reg_equiv` copy materialised after the
//     shift (measured 74/80, first difference +0x2). The call site in this
//     file passes `rec_` unchanged, so the stronger type is call-compatible.
// ----------------------------------------------------------------------------
void _08001E1E0(volatile u8 *rec, u16 b, u16 c) {
    (void)b;
    if ((u16)(c - 1) > 1u)
        return;
    sub_0802B368(1);
    *(volatile u32 *)(rec + 40) = 10;
    *(volatile u16 *)(rec + 44) = 0;
    *(volatile u32 *)(rec + 72) = 10;
    *(volatile u32 *)(rec + 68) = 1;
    switch (_08024068()) {
    case 27: case 28: case 29: case 30: case 38: case 43:
        *(volatile u16 *)(rec + 48) = 1;
        break;
    }
}
// 78 B of body, two short of the section's 4-byte alignment; the ROM holds
// `00 00` where gas closes a Thumb code section with `46 c0` (nop). Same
// file-scope zero-fill trick as race_phase_vms.c's RaceVM_022438.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// ---- 0x08001E230 — gate-dispatched forward leaf + key-gated setup ----
// Pools: {WA+0x10C3} x2. r1 dead at entry but live at the 2178 call.
// NOTE: asm sets r1=4 before the _08002178 bl; the callee ignores r1
// (blockb.s proof), so the C call passes it as a dead second arg via a
// two-arg function-pointer cast. A single-arg call omits the `movs r1,#4`
// and drops to 158/160 with the tail correct but one pool missing.
// Three lowering facts, all load-bearing (same as _08001E1E0):
//  1. First param is `volatile u8 *` in the SIGNATURE: agbcc then emits
//     `adds r5,r0,#0` ahead of the `lsls r2,#16`; with `void *` it is a
//     reg_equiv copy after the shift (first diff +0x2).
//  2. WA byte via base symbol + offset (two pools, base then off), not a
//     folded single literal. The `wa` symbol idiom is what EB48 uses; each
//     use scopes its own `wa` local so the base dies before the call and no
//     extra callee-saved register is pushed.
//  3. The trailing s16 test is a PLAIN `s16` read (ldrsh); volatile emits
//     ldrh+lsls+asrs. The sel value is pinned to r0 so the two `movs r0`
//     arms keep the ROM's `b`-gap where the second pool pair lives; an
//     unpinned `int sel` preloads `movs r1,#0` and parks the pools at the end.
//  4. 158 B body + `00 00` ROM pad vs gas `46 c0`: file-scope align fills 0.
// ----------------------------------------------------------------------------
void _08001E230(volatile u8 *rec, u16 b, u16 c) {
    volatile u8 *r4;
    (void)b;
    {
        extern u8 RaceSceneCWaE230[];
        __asm__(".globl RaceSceneCWaE230\nRaceSceneCWaE230 = 0x03001780");
        const volatile u8 *wa = RaceSceneCWaE230;
        if (*(volatile u8 *)(uintptr_t)(wa + 0x10C3) == 1) {
            _08002158(4, *(volatile u16 *)(rec + 170));
            r4 = rec + 168;
        } else {
            r4 = rec + 168;
            _08002158(4, *(volatile u16 *)r4);
        }
    }
    {
        extern u8 RaceSceneCWaE230[];
        const volatile u8 *wa = RaceSceneCWaE230;
        register int sel __asm__("r0");
        if (*(volatile u8 *)(uintptr_t)(wa + 0x10C3) == 0)
            sel = 1;
        else
            sel = 0;
        *(volatile u16 *)r4 = ((u16 (*)(int, int))_08002178)(sel, 4);
    }
    if ((u16)(c - 1) <= 1u)
        *(volatile u16 *)(rec + 170) = 1;
    if (*(s16 *)r4 == 1) {
        _08002618(1, 0);
        *(volatile u16 *)(rec + 160) = 0;
        sub_0802B368(1);
        *(volatile u32 *)(rec + 40) = 10;
        *(volatile u16 *)(rec + 44) = 0;
        *(volatile u32 *)(rec + 72) = 10;
        *(volatile u32 *)(rec + 68) = 1;
    }
}
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// ---- 0x08001E2D0 — guarded 8-arg emit (12,144,112,4,1,1,0) ----
// No pools. Guard: (u16)(u16[rec+146]-1) <= 2.
// ----------------------------------------------------------------------------
void _08001E2D0(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    if ((u16)(*(volatile u16 *)(rec + 146) - 1) <= 2u)
        _08007B18(rec_, 12, 144, 112, 4, 1, 1, 0);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E304 — guarded 8-arg emit (13,160,72,3,1,1,0) ----
// No pools. Guard: (u16)(u16[rec+146]-1) <= 7.
// ----------------------------------------------------------------------------
void _08001E304(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    if ((u16)(*(volatile u16 *)(rec + 146) - 1) <= 7u)
        _08007B18(rec_, 13, 160, 72, 3, 1, 1, 0);
}

// The spliced spans for 0x08001E2D0/0x08001E304 start at the asm labels
// `sub_08001E2D0`/`sub_08001E304`, which the splice deletes, and other
// assembly still branches to them. agbcc only writes the `.thumb_set` for a
// spelling this TU aliases, so declare both here (rule 6).
#ifndef __APPLE__
void sub_08001E2D0(void *rec_) __attribute__((alias("_08001E2D0")));
void sub_08001E304(void *rec_) __attribute__((alias("_08001E304")));
// Same rule 6 need for 0x08001E528/0x08001E56C: two distinct spliced bodies,
// each starting at the asm label the splice deletes, each with promoted
// callers under that spelling.
void sub_08001E528(void *rec_) __attribute__((alias("_08001E528")));
void sub_08001E56C(void *rec_) __attribute__((alias("_08001E56C")));
// Same rule 6 need for 0x08001E9DC: the spliced span starts at the asm label
// `sub_08001E9DC` (asm/race_scene.s:8535) which the splice deletes, and the
// asm defines both spellings at that one address.
void sub_08001E9DC(void *rec_) __attribute__((alias("_08001E9DC")));
#endif

// ----------------------------------------------------------------------------
// ---- 0x08001E3BC — four attr/param lane calls over WA+0x10E8..0x10F4 ----
// Pools: {WA base 0x03001780 + 0x10E8/0x10EC/0x10F0/0x10F4} (base kept in r4).
// Stack word of each call = s16[rec+150] reloaded per call via r6 pointer.
// Four lowering facts, all load-bearing:
//  1. WA words via base symbol + offset (two pools), not folded literals.
//     The `wa` uintptr_t idiom is what E4C0/E4F4 use.
//  2. Lane selects are PLAIN `s16` reads (ldrsh with r1=0); volatile emits
//     ldrh+lsls+asrs.
//  3. Tail is a POINTER local (`adds r6,r5,#150`), reloaded per call; a value
//     local hoists to r4 and drops push {r6} (prefix 0).
//  4. First WA word and lane load precede the tail-pointer materialisation:
//     `w0`/`a0` locals then `tail` assignment puts `adds r6` after the first
//     `ldrsh r3` like the ROM; declaring `tail` first hoists `adds r6` above
//     the second pool load (134/152, first diff +0x8).
// ----------------------------------------------------------------------------
void _08001E3BC(volatile u8 *rec) {
#ifndef __APPLE__
    extern u8 RaceSceneCWa3BC[];
    __asm__(".globl RaceSceneCWa3BC\nRaceSceneCWa3BC = 0x03001780");
    uintptr_t wa = (uintptr_t)RaceSceneCWa3BC;
#else
    uintptr_t wa = (uintptr_t)WA;
#endif
    u32 w0 = *(volatile u32 *)(wa + 0x10E8u);
    int a0 = ((const s16 *)(rec + 172))[0];
    const s16 *tail = (const s16 *)(rec + 150);
    _08001D910(84, 36, w0, a0, tail[0]);
    _08001D910(84, 52, *(volatile u32 *)(wa + 0x10ECu), ((const s16 *)(rec + 174))[0], tail[0]);
    _08001D910(84, 68, *(volatile u32 *)(wa + 0x10F0u), ((const s16 *)(rec + 176))[0], tail[0]);
    _08001D910(84, 92, *(volatile u32 *)(wa + 0x10F4u), ((const s16 *)(rec + 180))[0], tail[0]);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E454 — four zeroed attr/param lane calls ----
// Pools: {WA base + 0x10E8/0x10EC/0x10F0/0x10F4} (base kept in r4, zero in r5).
// Reads nothing from r0, but every caller still re-materialises r0 = rec
// before the `bl`, so the lifted source passes rec and the body ignores it.
// WA words via base symbol + offset (two pools), not folded literals; the
// `wa` uintptr_t idiom puts the base in r4 and the shared zero in r5 like
// the ROM (`movs r5,#0` once, `str r5,[sp]` per call). Folded literals put
// the zero in r4 and drop push {r5} (prefix 0).
// ----------------------------------------------------------------------------
void _08001E454(void *rec_) {
    (void)rec_;
#ifndef __APPLE__
    extern u8 RaceSceneCWa454[];
    __asm__(".globl RaceSceneCWa454\nRaceSceneCWa454 = 0x03001780");
    uintptr_t wa = (uintptr_t)RaceSceneCWa454;
#else
    uintptr_t wa = (uintptr_t)WA;
#endif
    _08001D910(84, 36, *(volatile u32 *)(wa + 0x10E8u), 0, 0);
    _08001D910(84, 52, *(volatile u32 *)(wa + 0x10ECu), 0, 0);
    _08001D910(84, 68, *(volatile u32 *)(wa + 0x10F0u), 0, 0);
    _08001D910(84, 92, *(volatile u32 *)(wa + 0x10F4u), 0, 0);
}

// ----------------------------------------------------------------------------
// ---- 0x0801E4C0 — WA-gated obj digits/lane select ----
// Pools: {WA+0x10F4, 0x0805FC1C}. Takes no args.
void _08001E4C0(void) {
#ifndef __APPLE__
    extern u8 RaceSceneCWa4C0[];
    __asm__(".globl RaceSceneCWa4C0\nRaceSceneCWa4C0 = 0x03001780\n");
    extern u8 RaceSceneCTbl4C0[];
    __asm__(".globl RaceSceneCTbl4C0\nRaceSceneCTbl4C0 = 0x0805FC1C\n");
    uintptr_t wa = (uintptr_t)RaceSceneCWa4C0;
#else
    uintptr_t wa = (uintptr_t)WA;
#endif
    uintptr_t a = wa + 0x10F4u;
    u32 w = *(volatile u32 *)a;
    if (w != 0)
        _08003B78(84, 92, w);
    else
        _08003838(84, 92,
                  (const volatile u8 *)(uintptr_t)
#ifndef __APPLE__
                  RaceSceneCTbl4C0
#else
                  0x0805FC1Cu
#endif
                  );
}


// ----------------------------------------------------------------------------
// ---- 0x08001E4F4 — attr/param lane twin call (148,92,...) ----
// Pools: {WA+0x10F8}.
// ----------------------------------------------------------------------------
void _08001E4F4(volatile u8 *rec) {
#ifndef __APPLE__
    extern u8 RaceSceneCWa4F4[];
    __asm__(".globl RaceSceneCWa4F4\nRaceSceneCWa4F4 = 0x03001780\n");
    uintptr_t wa = (uintptr_t)RaceSceneCWa4F4;
#else
    uintptr_t wa = (uintptr_t)WA;
#endif
    uintptr_t a = wa + 0x10F8u;
    u32 w = *(volatile u32 *)a;
    _08001D950(148, 92, w,
               ((const s16 *)(rec + 180))[0],
               ((const s16 *)(rec + 150))[0]);
}


// ----------------------------------------------------------------------------
// ---- 0x08001E528 — guarded 9-arg record emit (...,184,40,6,1,1,0) ----
// No pools. Guard: (u16)(u16[rec+146]-1) <= 2.
// ----------------------------------------------------------------------------
void _08001E528(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    if ((u16)(*(volatile u16 *)(rec + 146) - 1) <= 2u)
        sub_08007BFC((void *)(rec + 32),
                     *(volatile u32 *)(rec + 252), *(volatile u32 *)(rec + 256),
                     184, 40, 6, 1, 1, 0);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E56C — guarded 9-arg record emit, twin of 0x08001E528 ----
// No pools. Body identical to _08001E528 (separate VMA, own callers).
// ----------------------------------------------------------------------------
void _08001E56C(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    if ((u16)(*(volatile u16 *)(rec + 146) - 1) <= 2u)
        sub_08007BFC((void *)(rec + 32),
                     *(volatile u32 *)(rec + 252), *(volatile u32 *)(rec + 256),
                     184, 40, 6, 1, 1, 0);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E5B0 — dual-path double record emit (high-reg prologue) ----
// No pools. v=(u16)(u16[rec+146]-1): v<=2 requires s16[rec+150]==1,
// v in 3..8 takes the second arm. NOTE: both arms emit the same two
// calls ((168,104,7,1,1,0) + (184,112,7,1,1,0)); only the armcc
// high-register staging differs, so the arms share one tail here.
// ----------------------------------------------------------------------------
void _08001E5B0(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 v = (u16)(*(volatile u16 *)(rec + 146) - 1);
    if (v <= 2u) {
        if (*(volatile s16 *)(rec + 150) != 1)
            return;
    } else if ((u16)(v - 3) > 5u) {
        return;
    }
    Sub_08007BFC((void *)(rec + 8),
                 *(volatile u32 *)(rec + 192), *(volatile u32 *)(rec + 196),
                 168, 104, 7, 1, 1, 0);
    Sub_08007BFC((void *)(rec + 8),
                 *(volatile u32 *)(rec + 204), *(volatile u32 *)(rec + 208),
                 184, 112, 7, 1, 1, 0);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E68C — scene composite: emits + lanes + D97C/DBE8/E9D8 tail ----
// No pools.
// ----------------------------------------------------------------------------
void _08001E68C(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    _08001E2D0(rec_);
    _08001E304(rec_);
    _08001E5B0(rec_);
    _08001E454(rec_);
    _08001E528(rec_);
    _08001DBC0(rec_, *(s16 *)(rec + 162));
    _0800D97C((void *)(rec + 148), 2);
    _0800DBE8((void *)(rec + 40));
    _08001E9D8(rec_);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E6D8 — scene composite over the E338/E364/E3BC family ----
// No pools.
// ----------------------------------------------------------------------------
void _08001E6D8(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    _08001E338(rec_);
    _08001E364(rec_);
    _08001E3BC(rec_);
    _08001DBC0(rec_, *(s16 *)(rec + 162));
    _0800D97C((void *)(rec + 148), 2);
    _0800DBE8((void *)(rec + 40));
    _08001E9D8(rec_);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E718 — scene composite, twin of 0x08001E68C ----
// No pools. Body identical to _08001E68C (separate VMA, own callers).
// ----------------------------------------------------------------------------
void _08001E718(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    _08001E2D0(rec_);
    _08001E304(rec_);
    _08001E5B0(rec_);
    _08001E454(rec_);
    _08001E528(rec_);
    _08001DBC0(rec_, *(s16 *)(rec + 162));
    _0800D97C((void *)(rec + 148), 2);
    _0800DBE8((void *)(rec + 40));
    _08001E9D8(rec_);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E764 — WA-count-gated double emit pairs + 5-word tail emit ----
// Pools: {WA+0x10F8} (read twice: DA70 arg, then the >9 compare).
// record+16 base; pair kind 56/132 when count>9 else 40/140; tail emit
// carries u32[rec+264/268/276/280] + (11,1,1,0).
// ----------------------------------------------------------------------------
void _08001E764(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
#ifndef __APPLE__
    extern u8 RaceSceneCWa764[];
    __asm__(".globl RaceSceneCWa764\nRaceSceneCWa764 = 0x03001780");
    uintptr_t wa = (uintptr_t)RaceSceneCWa764;
#else
    uintptr_t wa = (uintptr_t)WA;
#endif
    volatile u32 *p = (volatile u32 *)(wa + 0x10F8u);
    _08001E390(rec_);
    _08001DA70(120, 64, *p, 10);
    if (*p > 9u) {
        Sub_08007BFC((void *)(rec + 16),
                     *(volatile u32 *)(rec + 292), *(volatile u32 *)(rec + 296),
                     56, 72, 10, 1, 1, 0);
        Sub_08007BFC((void *)(rec + 16),
                     *(volatile u32 *)(rec + 304), *(volatile u32 *)(rec + 308),
                     132, 80, 10, 1, 1, 0);
    } else {
        Sub_08007BFC((void *)(rec + 16),
                     *(volatile u32 *)(rec + 292), *(volatile u32 *)(rec + 296),
                     40, 72, 10, 1, 1, 0);
        Sub_08007BFC((void *)(rec + 16),
                     *(volatile u32 *)(rec + 304), *(volatile u32 *)(rec + 308),
                     140, 80, 10, 1, 1, 0);
    }
    Sub_08007BFC((void *)(rec + 16),
                 *(volatile u32 *)(rec + 264), *(volatile u32 *)(rec + 268),
                 (int)*(volatile u32 *)(rec + 276), (int)*(volatile u32 *)(rec + 280),
                 11, 1, 1, 0);
    _08001DBC0(rec_, *(s16 *)(rec + 162));
    _0800D97C((void *)(rec + 148), 15);
    _0800DBE8((void *)(rec + 40));
    _08001E9D8(rec_);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E8B4 — E4C0 + E56C + guarded emit + DBC0/D97C/DBE8/E9D8 tail ----
// No pools. NOTE: the E56C call is load-bearing: the ROM emits
// `bl sub_08001E56C` between the E4C0 call and the guarded 7BFC emit.
// Inlining the 7BFC alone omits that call (112 B vs 120 B, prefix 20).
// 118 B body + `00 00` ROM pad vs gas `46 c0`: align fills 0.
// ----------------------------------------------------------------------------
void _08001E8B4(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    _08001E4C0();
    _08001E56C(rec_);
    if ((u16)(*(volatile u16 *)(rec + 146) - 1) <= 2u)
        sub_08007BFC((void *)(rec + 8),
                     *(volatile u32 *)(rec + 240), *(volatile u32 *)(rec + 244),
                     8, 112, 5, 1, 1, 0);
    _08001DBC0(rec_, *(s16 *)(rec + 162));
    _0800D97C((void *)(rec + 148), 15);
    _0800DBE8((void *)(rec + 40));
    _08001E9D8(rec_);
}
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// ---- 0x08001E92C — s8-gated emit + zeroed lanes + 9-arg packet emit ----
// Pools: {WA+0x10E4} (ldrb + sign-extend; nonzero = s8 nonzero).
// ----------------------------------------------------------------------------
void _08001E92C(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    if (*(volatile s8 *)(uintptr_t)(WA + 0x10E4) != 0)
        Sub_08007BFC((void *)(rec + 8),
                     *(volatile u32 *)(rec + 228), *(volatile u32 *)(rec + 232),
                     160, 32, 8, 1, 1, 0);
    _08001E454(rec_);
    if (*(volatile s16 *)(rec + 160) == 1)
        _08007C68((void *)(rec + 24),
                     (int)*(volatile u32 *)(rec + 216), (int)*(volatile u32 *)(rec + 220),
                     20, 112, 9, 1, 0, 0); // R1-proven C body (was Sub_ ROM veneer)
    _08001DBC0(rec_, *(s16 *)(rec + 162));
    _0800D97C((void *)(rec + 148), 15);
    _0800DBE8((void *)(rec + 40));
    _08001E9D8(rec_);
}

// ----------------------------------------------------------------------------
// ---- 0x08001E9DC — countdown ticker (rec+320, cap 180 -> event 21) ----
// Pools: {WA+0xFBC}. Ticks only while u16[WA+0xFBC]==3; _08002140==2
// resets the counter, otherwise it increments and broadcasts (21,0,0)
// past 180.
// ----------------------------------------------------------------------------
void _08001E9DC(void *rec_) {
    extern u8 RaceSceneCWaE9DC[];
    __asm__(".globl RaceSceneCWaE9DC\nRaceSceneCWaE9DC = 0x03001780");
    const volatile u8 *wa = RaceSceneCWaE9DC;
    u32 n;
    if (*(volatile u16 *)(uintptr_t)(wa + 0xFBC) != 3)
        return;
    // The ==2 arm is the OUT-OF-LINE block in the ROM (beq past the pool),
    // so it must be the else arm here, not the fallthrough.
    if (_08002140() != 2) {
        n = *(volatile u32 *)(rec_ + 320) + 1;
        *(volatile u32 *)(rec_ + 320) = n;
        // Signed test: the ROM's `cmp r0,#180 / ble` is a SIGNED branch, so
        // `n > 180u` emits `bls` and costs the last mismatching byte.
        if ((s32)n > 180)
            _08004D4C(21, 0, 0);
    } else {
        *(volatile u32 *)(rec_ + 320) = 0;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x08001EA28 — 12-way event dispatcher (table 0x0801EA48) ----
// Pools: {0x0801EA48, nested 0x0801EAA8}. ev is 1-based (idx = ev-1):
// ev1->DC8C, ev2->D974(rec,b), ev5->D854(rec+40), ev6->E9DC+key
// fan-out, ev7->nested s16[rec+156] dispatch (0:E6D8 1:E68C 2:E764
// 3:E8B4 4:E718 5:E92C), ev12->D9C8, all other slots break.
// NOTE: b/c are word-sized (D974 takes b as a record pointer); the
// E1E0/E230 arms truncate them to u16 exactly like the asm shifts.
// ----------------------------------------------------------------------------
void _08001EA28(int ev, u32 b, u32 c, void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int idx = ev - 1;
    if ((u32)idx > 11u)
        return;
    switch (idx) {
    case 0:
        _08001DC8C(rec_);
        break;
    case 1:
        _08001D974(rec_, (void *)(uintptr_t)b);
        break;
    case 4:
        _0800D854((void *)(rec + 40));
        break;
    case 5: {
        s16 s;
        _08001E9DC(rec_);
        if (*(volatile u16 *)(rec + 44) == 0)
            break;
        s = *(volatile s16 *)(rec + 156);
        if (s < 0)
            break;
        if (s <= 4)
            _08001E1E0(rec_, (u16)b, (u16)c);
        else if (s == 5)
            _08001E230(rec_, (u16)b, (u16)c);
        break;
    }
    case 6: {
        s16 s = *(volatile s16 *)(rec + 156);
        if ((u32)s > 5u)
            break;
        switch (s) {
        case 0: _08001E6D8(rec_); break;
        case 1: _08001E68C(rec_); break;
        case 2: _08001E764(rec_); break;
        case 3: _08001E8B4(rec_); break;
        case 4: _08001E718(rec_); break;
        case 5: _08001E92C(rec_); break;
        }
        break;
    }
    case 11:
        _08001D9C8();
        break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x08001EB48 — scene-header car-class writer ----
// Pools: {WA+0xFBC}. NOTE: incoming r0 dead (never read); r1 is the
// record. u8[rec+88]=0 while u16[WA+0xFBC]==3; u16[rec+84] = 5 for
// class {26,35}, 1 for {39}, else 6.
// ----------------------------------------------------------------------------
void _08001EB48(int a0, void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    extern u8 RaceSceneCWaEB48[];
    __asm__(".globl RaceSceneCWaEB48\nRaceSceneCWaEB48 = 0x03001780");
    const volatile u8 *wa = RaceSceneCWaEB48;
    s16 v;
    (void)a0;
    if (*(volatile u16 *)(uintptr_t)(wa + 0xFBC) == 3)
        *(volatile u8 *)(rec + 88) = 0;
    v = *(s16 *)((u8 *)_08004B68() + 2);
    switch (v) {
    case 26:
    case 35:
        *(volatile u16 *)(rec + 84) = 5;
        break;
    case 39:
        *(volatile u16 *)(rec + 84) = 1;
        break;
    default:
        *(volatile u16 *)(rec + 84) = 6;
        break;
    }
}
