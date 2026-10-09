// ============================================================================
// race_scene_d1.c — reconstructed C for asm/race_scene.s (15 functions):
//
//   0x0801EBA8 0x0801F568 0x0801F5B4 0x0801F668 0x0801F6A4 0x0801F764
//   0x0801F82C 0x0801F89C 0x0801F918 0x0801F9EC 0x0801F9FC 0x0801FA0C
//   0x0801FB58 0x0801FBAC 0x0801FBD8
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig

// Closure spellings for _08001F6A4's two external callees. A HOST_STUB is NOT a
// call-site binding: it expands to `extern` on the ROM build, so `Sub_08007770`
// there names a symbol the closure does not define, and the screen blocks the
// body on the rename. The digit count is load-bearing -- nm reports
// `sub_0802B214` (7 hex digits), and the 8-digit `sub_08002B214` normalises to
// the same address and still fails.
//
// The macro must sit OUTSIDE the __APPLE__ split above. Defining it inside the
// `#else` branch leaves it undefined on the host build, and tools/apple_decls.py
// catches that as a "call to undeclared 'D1_CALLEE' under __APPLE__" -- the
// same reason the externs for the closure spellings have to stay where they
// are, under the ROM branch.
extern void sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void sub_0802B214(int v);
#endif

// Closure spellings for _08001F6A4's two external callees. A HOST_STUB is NOT a
// call-site binding: it expands to `extern` on the ROM build, so `Sub_08007770`
// there names a symbol the closure does not define, and the screen blocks the
// body on the rename. The digit count is load-bearing -- nm reports
// `sub_0802B214` (7 hex digits), and the 8-digit `sub_08002B214` normalises to
// the same address and still fails.
//
// This macro MUST sit at file scope, below the __APPLE__ split above, not
// inside its `#else`. Defined inside the ROM branch it is undefined on the
// host build, and tools/apple_decls.py reports that as a "call to undeclared
// 'D1_CALLEE' under __APPLE__" -- the check exists for exactly this.
#ifndef __APPLE__
#define D1_CALLEE(friendly, closure) closure
#else
#define D1_CALLEE(friendly, closure) friendly
#endif

// External callees (strong lifts or trampolines).
HOST_STUB(void _08002B214(int v));                                             // 0x08002B214 sound wake
HOST_STUB(void sub_0802B368(int v));                                           // 0x0802B368 sound cue
HOST_STUB(void _08002124(u16 v));                                              // 0x08002124 BlockB arm
HOST_STUB(int _08002140(void));                                                 // 0x08002140 BlockB match count (word return: a u16 one forces a lsls/lsrs pair)
HOST_STUB(void *_08004B68(void));                                              // 0x08004B68 scene header
HOST_STUB(void _08004D4C(u32 a, u32 b, u32 c));                                // 0x08004D4C scene broadcast
HOST_STUB(void _080075E8(void *a, int b, int c));                              // 0x080075E8 emit lane
HOST_STUB(void _08007570(void *a, int b, int c, int d, int units)); // 0x08007570 5-arg: r0-r3 + 1 stack word (32-byte units)
HOST_STUB(void _08007770(int a, void *b, int c, int d, u32 e, u32 f));        // 0x08007770 6 machine args
HOST_STUB(void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f));     // 0x08007770 exact ROM body
HOST_STUB(void _08007ABC(void *a, void *b, int c));                            // 0x08007ABC resource bind
HOST_STUB(void _0800798C(void *a, void *b));                                   // 0x0800798C resource setup
HOST_STUB(void _08007A58(void *p));                                            // 0x08007A58 resource init
HOST_STUB(void _08007614(void *a, int b, int c, int d));                       // 0x08007614 lane emit
HOST_STUB(void _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h)); // 0x08007B18 8-arg emit
HOST_STUB(void _08002ED0(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j)); // 0x08002ED0 10 machine args
HOST_STUB(int _0800572C(int v));                                               // 0x0800572C IWRAM bump
HOST_STUB(int _08005758(int v));                                               // 0x08005758 IWRAM alloc
HOST_STUB(int _080022E4(int id));                                              // 0x080022E4 car-record map
HOST_STUB(int _080025F20(int a));                                              // 0x080025F20 grid hw get
HOST_STUB(void _0800D77C(void *a, int b, int c));                              // 0x0800D77C slide setup
HOST_STUB(void _0800D9A4(void *a, int b, int c));                              // 0x0800D9A4 grid paint
HOST_STUB(void _0800DAB8(void *p));                                            // 0x0800DAB8 obj init
HOST_STUB(void _0800D854(void *p));                                            // 0x0800D854 record tick
HOST_STUB(void _0800DBE8(void *p));                                            // 0x0800DBE8 record setup
HOST_STUB(void _0800D95C(void *dst, const void *src, int n));                  // 0x0800D95C memcpy
HOST_STUB(void _08001EB48(int a0, void *rec));                                 // 0x08001EB48 (race_scene_c.c)
HOST_STUB(void _08001EBA4(void));                                              // 0x08001EBA4 bx-lr stub (race_scene.c)
HOST_STUB(void _08001EE2C(void *rec, u16 b, u16 c));                           // 0x08001EE2C key handler
HOST_STUB(void _08001EFAC(void *rec));                                         // 0x08001EFAC emit scene
HOST_STUB(void _08001F554(void *a));                                           // 0x08001F554 BlockB forward leaf (race_scene.c)

// ---- shared anchors --------------------------------------------------------
#define WA        0x03001780u
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WA + (o)))
#define WA_U32(o) (*(volatile u32 *)(uintptr_t)(WA + (o)))

// ----------------------------------------------------------------------------
// ---- 0x0801EBA8 — twin setup scene (WA-gated arm, 8-lane flag/bitblt init,
// ---- 8-record alloc/bind loop, WA/record tail + D9A4 grid paint) ----
// Pools: {WA+0xFBC, 0x1393, 0x0833F35C, 0x082A798C, 0x083413B0, 0x080CBBC4,
// 0x080CBBD4, 0x08342B60, WA+0x105A, WA+0xFC2, WA+0x574, WA+0xFC0}.
// u16[rec+134] = (u16[WA+0xFBC]==7); flag lanes u16[rec+144+2i] =
// (_080025F20(i)!=0); per lane i: u32[rec+160+12i]=_0800572C(9),
// u32[rec+164+12i]=u16[0x080CBBC4/0x080CBBD4+2i] (flag==1 picks BBC4),
// u32[rec+168+12i]=4, then _08007ABC(u32[rec+12], lane, alloc).
// ----------------------------------------------------------------------------
void _08001EBA8(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int i;
    D1_CALLEE(_08002B214, sub_0802B214)(49);
    if (WA_U16(0xFBC) == 3)
        _08002124(0x1393);
    if (WA_U16(0xFBC) == 7)
        *(volatile u16 *)(rec + 134) = 1;
    else
        *(volatile u16 *)(rec + 134) = 0;
    D1_CALLEE(Sub_08007770, sub_08007770)(0, (void *)0x0833F35Cu, 2, 0, 4, 1);
    *(volatile u32 *)(rec + 96) = 6;
    *(volatile u32 *)(rec + 108) = 9;
    *(volatile u32 *)(rec + 120) = 1;
    _0800DAB8((void *)(rec + 24));
    _080075E8((void *)0x082A798Cu, 2, 6);
    _0800798C((void *)0x0833F35Cu, rec_);
    _080075E8((void *)0x0833F35Cu, 0, 3);
    _080075E8((void *)0x0833F35Cu, 1, 5);
    _08007A58(rec_);
    _08007614((void *)0x083413B0u, 0, 0, 4);
    for (i = 0; i <= 7; i++)
        *(volatile u16 *)(rec + 144 + 2u * (u32)i) = _080025F20(i) != 0 ? 1 : 0;
    for (i = 0; i < 8; i++) {
        int a = _0800572C(9);
        u16 f = *(volatile u16 *)(rec + 144 + 2u * (u32)i);
        u16 t = (f == 1) ? *(volatile u16 *)(uintptr_t)(0x080CBBC4u + 2u * (u32)i)
                         : *(volatile u16 *)(uintptr_t)(0x080CBBD4u + 2u * (u32)i);
        *(volatile u32 *)(rec + 160 + 12u * (u32)i) = (u32)a;
        *(volatile u32 *)(rec + 164 + 12u * (u32)i) = (u32)t;
        *(volatile u32 *)(rec + 168 + 12u * (u32)i) = 4;
        _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 12),
                  (void *)(uintptr_t)*(volatile u32 *)(rec + 164 + 12u * (u32)i),
                  (int)*(volatile u32 *)(rec + 160 + 12u * (u32)i));
    }
    _0800798C((void *)0x08342B60u, (void *)(rec + 16));
    _08007A58((void *)(rec + 16));
    *(volatile u32 *)(rec + 24) = 0;
    *(volatile u16 *)(rec + 28) = 1;
    _0800D77C((void *)(rec + 44), 0, -32);
    _0800D77C((void *)(rec + 36), 0, 160);
    *(volatile u16 *)(rec + 32) = 6;
    *(volatile u16 *)(rec + 34) = 5;
    WA_U16(0x105A) = 0;
    if (*(volatile u16 *)(rec + 134) == 1)
        *(volatile u16 *)(rec + 130) = WA_U16(0xFC2);
    else
        *(volatile u16 *)(rec + 130) = WA_U16(0x574);
    *(volatile u16 *)(rec + 132) = (u16)_080022E4((int)*(volatile s16 *)(rec + 130));
    {
        s16 v = *(volatile s16 *)(rec + 130);
        *(volatile u16 *)(rec + 128) =
            (u16)(s16)*(volatile s8 *)(uintptr_t)(WA + (u32)((s32)v * 12) + 49u);
    }
    *(volatile u16 *)(rec + 136) = WA_U16(0xFC0);
    *(volatile u16 *)(rec + 140) = 0;
    if (*(volatile u16 *)(rec + 136) <= 3)
        *(volatile u16 *)(rec + 142) = 0;
    if ((u16)(*(volatile u16 *)(rec + 136) - 4) <= 3u)
        *(volatile u16 *)(rec + 142) = 1;
    *(volatile u16 *)(rec + 138) = 5;
    _0800D9A4((void *)(rec + 268),
              (int)*(volatile s16 *)(rec + 130), (int)*(volatile s16 *)(rec + 128));
}

// ----------------------------------------------------------------------------
// ---- 0x0801F568 — countdown ticker (rec+272, cap 180 -> event 21) ----
// Pools: {0x03001780, 0x00000FBC} — the work-area base and the 0xFBC block
// offset are TWO literals, not the folded 0x0300273C. Twins _08001E9DC
// (rec+320, race_scene_c.c): ticks only while u16[WA+0xFBC]==3;
// _08002140==2 resets the counter, otherwise it increments and broadcasts
// (21,0,0) past 180. `rec` is the typed parameter rather than a cast copy of a
// `void *`: with a separate local, agbcc keeps a second copy and emits
// `push {r4,r5,lr}` + `adds r5, r4, #0` the ROM does not have.
// ----------------------------------------------------------------------------
void _08001F568(volatile u8 *rec) {
    int n;
    // The ROM keeps the work-area base and the 0xFBC block offset as TWO pool
    // words (`ldr r0,=0x03001780 / ldr r1,=0x00000FBC / adds r0,r0,r1`).
    // A single address expression folds the two into one 0x0300273C word, so
    // base and offset are held in locals: the constant address then becomes
    // register + constant and each gets its own pool word. The extern and its
    // definition both live in this body so the per-body splice carries the
    // reference and the definition together.
    extern u8 RaceSceneD1WA[];
    u8 *wa = (u8 *)(uintptr_t)RaceSceneD1WA;
    u32 off = 0xFBC;
    __asm__(".globl RaceSceneD1WA\nRaceSceneD1WA = 0x03001780");
    if (*(volatile u16 *)(uintptr_t)(wa + off) != 3)
        return;
    // `!= 2` rather than `== 2` + return: the ROM branches `beq` FORWARD past
    // the body to the reset block parked after the literal pool, which is the
    // layout `if (a) {...} else {...}` gives and `if (a) {...; return;}` does
    // not (that spelling emits `bne` over an inline block).
    if (_08002140() != 2) {
        n = (int)*(volatile u32 *)(rec + 272) + 1;
        *(volatile u32 *)(rec + 272) = (u32)n;
        if (n > 180)
            _08004D4C(21, 0, 0);
    } else {
        *(volatile u32 *)(rec + 272) = 0;
    }
}

#ifndef __APPLE__
void sub_08001F568(volatile u8 *rec) __attribute__((alias("_08001F568")));
#endif

// ----------------------------------------------------------------------------
// ---- 0x0801F5B4 — 12-way event dispatcher (table 0x0801F5D0) ----
// No pools (table words are code pointers). ev is 1-based (idx = ev-1):
// ev1->EBA8(rec), ev2->EB48(rec,b), ev5->D854(rec+24)+FBC-gated F554,
// ev6->F568+nonzero-key EE2C, ev7->EFAC(rec), ev12->EBA4 stub,
// all other slots break.
// NOTE: b/c are word-sized; only the ev6 arm truncates them to u16
// exactly like the asm shifts. The ev2 arm forwards b untruncated as
// EB48's record (r1), with rec in EB48's dead r0 slot.
// ----------------------------------------------------------------------------
void _08001F5B4(int ev, u32 b, u32 c, void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int idx = ev - 1;
    if ((u32)idx > 11u)
        return;
    switch (idx) {
    case 0:
        _08001EBA8(rec_);
        break;
    case 1:
        _08001EB48((int)(uintptr_t)rec_, (void *)(uintptr_t)b);
        break;
    case 4:
        _0800D854((void *)(rec + 24));
        if (WA_U16(0xFBC) == 3)
            _08001F554(rec_);
        break;
    case 5:
        _08001F568(rec_);
        if (*(volatile u16 *)(rec + 28) != 0)
            _08001EE2C(rec_, (u16)b, (u16)c);
        break;
    case 6:
        _08001EFAC(rec_);
        break;
    case 11:
        _08001EBA4();
        break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801F668 — scene-header class select into rec+84 (5/6) ----
// No pools. NOTE: incoming r0 dead (never read); r1 is the record
// (same shape as _08001EB48). u16[rec+84] = 5 for class {24,34,36,37}
// (35 excluded), else 6.
//
// Two lowering facts are load-bearing for byte-exactness here:
void _08001F668(int a0, void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 v;
    s16 *p;
    (void)a0;
    p = (s16 *)(void *)((u8 *)_08004B68() + 2);
    v = *p;
    switch (v) {
    case 24: case 34: case 36: case 37:
        *(volatile u16 *)(rec + 84) = 5;
        break;
    default:
        *(volatile u16 *)(rec + 84) = 6;
        break;
    }
}

// asm/race_scene.s:10093-10094 defines BOTH `sub_08001F668:` and `_08001F668:` on
// this one body, and asm/race_scene.s:10514 calls it as `bl sub_08001F668`. So the
// C replacement must publish BOTH spellings: without the `sub_` form the
// underscore-form definition alone leaves the asm caller undefined at the slice
// link. The manifest `export` for this body records both.
#ifndef __APPLE__
void sub_08001F668(int a0, void *rec_) __attribute__((alias("_08001F668")));
#endif

// ----------------------------------------------------------------------------
// ---- 0x0801F6A4 — record setup leaf (wake 51, template lanes, WA+0x1080
// ---- mirror into rec+8) ----
// Pools: {0x083433D8, 0x082A798C, WA+0x1080} (132<<5 = 4224).
// ----------------------------------------------------------------------------
void _08001F6A4(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 w;
    D1_CALLEE(_08002B214, sub_0802B214)(51);
    D1_CALLEE(Sub_08007770, sub_08007770)(0, (void *)0x083433D8u, 0, 0, 4, 1);
    *(volatile u32 *)(rec + 88) = 6;
    *(volatile u32 *)(rec + 100) = 5;
    *(volatile u32 *)(rec + 112) = 1;
    _0800DAB8((void *)(rec + 16));
    _08007614((void *)0x082A798Cu, 1, 0, 3);
    _08007614((void *)0x082A798Cu, 1, 1, 4);
    _0800798C((void *)0x083433D8u, rec_);
    _08007A58(rec_);
    *(volatile u32 *)(rec + 16) = 0;
    *(volatile u16 *)(rec + 20) = 1;
    _0800D77C((void *)(rec + 36), 0, -32);
    _0800D77C((void *)(rec + 28), 0, 160);
    *(volatile u16 *)(rec + 24) = 6;
    *(volatile u16 *)(rec + 26) = 5;
    *(volatile u32 *)(rec + 12) = 2;
    // +0x1080 is block 132 of the 32-byte-stride work area at 0x03001780.
    // Reading it through the type is what keeps agbcc from folding the block
    // index into one address constant; the ROM computes base + 132<<5. The
    // extern and its definition both live in this body so the per-body splice
    // carries the reference and the definition together.
    struct RaceSceneWA { u8 blocks[0x1080]; u16 lane_sel; };
    extern u8 RaceSceneWorkArea[];
    __asm__("RaceSceneWorkArea = 0x03001780");
    w = ((volatile struct RaceSceneWA *)(uintptr_t)RaceSceneWorkArea)->lane_sel;
    if (w <= 2) {
        *(volatile u16 *)(rec + 8) = w;
    } else {
        w = 0;
        *(volatile u16 *)(rec + 8) = w;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801F764 — key handler (countdown latch, lane tick, select echo) ----
// Pools: {WA+0x1080} (132<<5 = 4224, twice). NOTE: incoming r1 (b) dead;
// r2 (c) is the live key, truncated to u16 exactly like the asm shifts.
// c==2: sound 4 + full reset lanes, WA mirror = 3. c==1: gated on
// u16[rec+8] (<=2: sound 1 + select lanes incl. u16[rec+24]=8 when ==2;
// s16==3: sound 4 + reset lanes), then WA mirror = u16[rec+8].
// c==64/128: u16[rec+8]-/++. Clamp s16[rec+8] to 0..3; on change vs the
// entry snapshot, sound 2.
//
// Two lowering facts, both load-bearing:
//  1. First param is `volatile u8 *` in the SIGNATURE (adds r4,r0 before the
//     lsls, same as E1E0/E230).
//  2. Entry snapshot loads straight into r6 (`ldrh r6,[r4,#8]`); a plain u16
//     local emits `ldrh r0` + `adds r6,r0`.
//  3. WA mirror via base symbol + 132<<5 (ldr base, movs 132, lsls 5, adds),
//     not a folded single literal.
// ----------------------------------------------------------------------------
void _08001F764(volatile u8 *rec, u16 b, u16 c) {
    register u32 entry __asm__("r6") = *(volatile u16 *)(rec + 8);
    extern u8 RaceSceneD1Wa764[];
    __asm__(".globl RaceSceneD1Wa764\nRaceSceneD1Wa764 = 0x03001780");
    (void)b;
    if (c == 2) {
        sub_0802B368(4);
        {
            register int ten __asm__("r1") = 10;
            *(volatile u32 *)(rec + 16) = ten;
            {
                register int zero __asm__("r0") = 0;
                *(volatile u16 *)(rec + 20) = zero;
                *(volatile u32 *)(rec + 48) = ten;
                *(volatile u32 *)(rec + 44) = zero;
            }
        }
        {
            int blk = 132;
            *(volatile u16 *)(RaceSceneD1Wa764 + (blk << 5)) = 3;
        }
    }
    if (c == 1) {
        register u16 vv __asm__("r2") = *(volatile u16 *)(rec + 8);
        register u32 sh __asm__("r1") = (u32)vv << 16;
        if ((sh >> 16) <= 2) {
            sub_0802B368(1);
            *(volatile u32 *)(rec + 12) = 1;
            {
                register int ten __asm__("r1") = 10;
                *(volatile u32 *)(rec + 16) = ten;
                {
                    register int zero __asm__("r0") = 0;
                    *(volatile u16 *)(rec + 20) = zero;
                    *(volatile u32 *)(rec + 48) = ten;
                    *(volatile u32 *)(rec + 44) = 1;
                }
            }
            if (*(volatile u16 *)(rec + 8) == 2)
                *(volatile u16 *)(rec + 24) = 8;
        } else {
            register int signed_hi __asm__("r0") = (int)sh >> 16;
            if (signed_hi == 3) {
                sub_0802B368(4);
                {
                    register int ten __asm__("r1") = 10;
                    *(volatile u32 *)(rec + 16) = ten;
                    {
                        register int zero __asm__("r0") = 0;
                        *(volatile u16 *)(rec + 20) = zero;
                        *(volatile u32 *)(rec + 48) = ten;
                        *(volatile u32 *)(rec + 44) = zero;
                    }
                }
            }
        }
        {
            register u8 *wa __asm__("r0") = RaceSceneD1Wa764;
            register u16 tmp __asm__("r1") = *(volatile u16 *)(rec + 8);
            register int blk __asm__("r2") = 132;
            blk <<= 5;
            wa += blk;
            *(volatile u16 *)wa = tmp;
        }
    }
    if (c == 64)
        *(volatile u16 *)(rec + 8) = (u16)(*(volatile u16 *)(rec + 8) - 1);
    if (c == 128)
        *(volatile u16 *)(rec + 8) = (u16)(*(volatile u16 *)(rec + 8) + 1);
    if (*(s16 *)(uintptr_t)(rec + 8) < 0)
        *(volatile u16 *)(rec + 8) = 0;
    if (*(s16 *)(uintptr_t)(rec + 8) > 3)
        *(volatile u16 *)(rec + 8) = 3;
    {
        register u16 current __asm__("r4") = *(volatile u16 *)(rec + 8);
        if (entry != current)
            sub_0802B368(2);
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801F82C — mode-switched 8-arg emit (kind = u16 table select) ----
// Pools: {0x080CBC60}. sel = u16[0x080CBC60 + 2*s16[rec+8]] stays in r1
// across the mode switch; the switch picks the 5th stack word (4 for
// mode {1,3}, 3 for mode {0,2}): _08007B18(rec, sel, b, c, 4/3, 1, 0, 0).
// Three lowering facts are load-bearing for byte-exactness here:
//  * The table base is a SYMBOL. Spelled as a C constant, agbcc folds it into
//    the address and materialises `ldr r1,[pc]` at the point of use, i.e. one
//    slot after the `lsls`; as a symbol it is a separate literal the allocator
//    places at the ROM's offset, first instruction of the body. Same rule as
//    _08001F918 and _08001FA0C below.
//  * The mode test must be a `switch` over a SIGNED m. An `if/else if` chain
//    emits a linear test sequence, and a `u32 m` makes the balanced tree branch
//    on the low side first (`bcc`) instead of the ROM's `bgt`/`cmp #0` pair.
//  * The class order matters: cases 1/3 before 0/2 is what puts the mode-4
//    block first after the pool, where the ROM parks it.
// ----------------------------------------------------------------------------
// The 4th parameter is never read by the body (the ROM prologue saves only
// r3/r4/r5 = a0/b/c and reuses r3 as scratch), but _08001F918's call site
// passes one, and the ROM reloads u32[rec+12] into r3 there. Declaring the
// dead 4th parameter is what makes agbcc emit that load.
void _08001F82C(volatile u8 *rec, int b, int c, int d) {
    (void)d;
    extern u16 RaceSceneD1Tab[];
    __asm__(".globl RaceSceneD1Tab\nRaceSceneD1Tab = 0x080CBC60");
    s16 k = *(s16 *)(uintptr_t)(rec + 8);
    u16 sel = RaceSceneD1Tab[k];
    int m = (int)*(volatile u32 *)(rec + 12);
    switch (m) {
    case 1:
    case 3:
        _08007B18((void *)rec, (int)sel, b, c, 4, 1, 0, 0);
        break;
    case 0:
    case 2:
        _08007B18((void *)rec, (int)sel, b, c, 3, 1, 0, 0);
        break;
    default:
        break;
    }
}
// The body is 110 bytes, so its `-ffunction-sections` section is padded to 112.
// gas closes a Thumb code section with the 2-byte `nop` filler (0x46c0); the ROM
// pads with `00 00`. A file-scope `.align 2, 0` lands after the body's `.size`,
// i.e. still inside the body's own section, and pads with the explicit `0` fill.
__asm__(".align 2, 0");

#ifndef __APPLE__
void sub_08001F82C(volatile u8 *rec, int b, int c, int d) __attribute__((alias("_08001F82C")));
#endif

// ----------------------------------------------------------------------------
// ---- 0x0801F89C — 30-iteration kind-gated 8-arg emit (stride 8) ----
// No pools. for k = 0..232 step 8 (30 iters): mode==2 emits
// (rec+52, 7, k, v, 3, 1, 0, 0), mode==3 emits (..., 4,...) instead.
// Three lowering facts are load-bearing for byte-exactness here:
//  * The mode test is a `switch`. An `if/else if` chain emits
//    `cmp #2 / bne` skipping inline, where the ROM branches `beq` forward to
//    both case blocks with a trailing `b` to the loop increment.
//  * The loop increments k BEFORE n (`k += 8, n--`); the opposite order emits
//    the `n--` pair first.
//  * `rec` is the typed parameter and the two loop-invariant call arguments
//    are named locals declared before the counters — see the body.
// ----------------------------------------------------------------------------
void _08001F89C(volatile u8 *rec, int v, int mode) {
    // Declared (and so initialised) before k/n: the ROM's loop preheader
    // materialises the two loop-invariant call arguments first
    // (`movs r0,#1 / mov sl,r0 / movs r4,#0`) and only then the counters
    // (`movs r5,#0 / movs r0,#29 / mov r9,r0`).
    int one = 1;
    int zero = 0;
    int k = 0;
    int n = 29;
    for (; n >= 0; k += 8, n--) {
        switch (mode) {
        case 2:
            _08007B18((void *)(rec + 52), 7, k, v, 3, one, zero, zero);
            break;
        case 3:
            _08007B18((void *)(rec + 52), 7, k, v, 4, one, zero, zero);
            break;
        default:
            break;
        }
    }
}

#ifndef __APPLE__
void sub_08001F89C(volatile u8 *rec, int v, int mode) __attribute__((alias("_08001F89C")));
#endif

// ----------------------------------------------------------------------------
// ---- 0x0801F918 — paired-record emit + select emit + DBE8 tail ----
// Pools: {0x080CBC40} (8-byte entries: +0 u16 select, +4 s16 lane).
// e = table + 8*s16[rec+8]: F89C(rec, s16[e+4], u32[rec+12]),
// then F82C(rec, (s16)u16[e], s16[e+4]), then D854(rec+16).
// ----------------------------------------------------------------------------
void _08001F918(volatile u8 *rec) {
    extern u8 RaceSceneD1Tbl40[];
    __asm__(".globl RaceSceneD1Tbl40\nRaceSceneD1Tbl40 = 0x080CBC40");
    u8 *tbl = RaceSceneD1Tbl40;
    s16 k = *(s16 *)(uintptr_t)(rec + 8);
    u32 idx = (u32)((s32)k * 8);
    u8 *pa = tbl + idx;
    u16 a = *(u16 *)(uintptr_t)pa;
    u8 *tblb = tbl + 4u;
    s16 b = *(s16 *)(uintptr_t)(tblb + idx);
    _08001F89C((void *)rec, (int)b, (int)*(volatile u32 *)(rec + 12));
    _08001F82C((void *)rec, (int)(s16)a, (int)b, (int)*(volatile u32 *)(rec + 12));
    _0800DBE8((void *)(rec + 16));
}

// asm/race_scene.s:10453-10455 defines BOTH `sub_08001F918:` and
// `_08001F918:` on this one body, and asm/race_scene.s:10523 calls it as
// `bl sub_08001F918`. A promoted entry's export must therefore carry both
// spellings; the `sub_` twin is a direct alias of the C body with the same
// signature (same pattern as _08001F568/_08001F89C above).
#ifndef __APPLE__
void sub_08001F918(volatile u8 *rec) __attribute__((alias("_08001F918")));
#endif

// ----------------------------------------------------------------------------
// ---- 0x0801F9EC — s16 table lookup (table 0x080CBE78) ----
// No pools (table address is a code word). return s16[0x080CBE78 + 2*x].
// Two lowering facts, both load-bearing:
//  * The element read must go through a PLAIN `const s16 *`. agbcc will not
//    fold a volatile HImode load into LDRSH: `volatile` emits ldrh + lsls#16
//    + asrs#16 (8 bytes), which is the +4-byte OVERSIZED this body reported.
//    Dropping it gives the ROM's single `movs r1,#0; ldrsh r0,[r0,r1]`
//    (Thumb has no immediate-offset LDRSH, so the ROM materialises the zero
//    register itself). Same rule as _08001F668 above.
//  * The table base needs a SECOND address node to land in the right place.
//    Two steps, both measured: a bare `0x080CBE78u + (x<<1)` emits
//    lsls/ldrh/lsls#16/asrs#16 and sinks the literal after the shift. A
//    function-local `const s16 *t` alone fixes the SIZE and the load form
//    (16 B, 12/16) but still orders it `lsls` then `ldr r1,[pc]`, i.e. the
//    literal one slot too late. Adding the `+ t[0] - t[0]` second address
//    node -- algebraically a no-op, and it emits no extra instruction --
//    forces agbcc to materialise the base in r1 from the entry, which is
//    exactly where the ROM has its `ldr r1,[pc,#8]`. This is the same idiom
//    as Ai_LineGet0..3 in src/ai_line_leaves.c.
// ----------------------------------------------------------------------------
int _08001F9EC(int x) {
    const s16 *t = (const s16 *)(uintptr_t)0x080CBE78u;
    return t[x] + t[0] - t[0];
}

// ----------------------------------------------------------------------------
// ---- 0x0801F9FC — s16 table lookup (table 0x080CBE82) ----
// No pools (table address is a code word). return s16[0x080CBE82 + 2*x].
// Same lowering as _08001F9EC above; only the literal differs.
// ----------------------------------------------------------------------------
int _08001F9FC(int x) {
    const s16 *t = (const s16 *)(uintptr_t)0x080CBE82u;
    return t[x] + t[0] - t[0];
}

// ----------------------------------------------------------------------------
// ---- 0x0801FA0C — 64-lane alloc/template init + lane-emit tail ----
// Pools: {0x0203F8F0, 0x08344450}. Takes no args (r0 never read).
// 64 iters: u16[0x0203F8F2+2i] = (u16)_08005758(1), then
// _08007570(0x08344450, 0, (s16)lane, i, 1); tail lanes
// _08007614(0x08344450, 1, 0, 3) + (..., 1, 1, 4).
// ----------------------------------------------------------------------------
void _08001FA0C(void) {
    // `base[1 + i]`, not a pre-offset pointer: the ROM's lane base is the
    // literal 0x0203F8F0 walked by 2, and the store/load share one moving
    // pointer. RESIDUAL: agbcc still folds `0x0203F8F0 + 2` into a single
    // 0x0203F8F2 pool word, so the ROM's `ldr r0,[pc] / adds r4, r0, #2`
    // pair (2 bytes) and the pool word itself are still missing.
    u16 *base = (u16 *)(uintptr_t)0x0203F8F0u;
    int i;
    for (i = 0; i <= 63; i++) {
        base[1 + i] = (u16)_08005758(1);
        _08007570((void *)0x08344450u, 0,
                  (int)*(s16 *)(uintptr_t)&base[1 + i], i, 1);
    }
    _08007614((void *)0x08344450u, 1, 0, 3);
    _08007614((void *)0x08344450u, 1, 1, 4);
}

// ----------------------------------------------------------------------------
// ---- 0x0801FB58 — guarded 10-arg emit leaf (r3 passes through) ----
// Pools: {0x080CBC74, 0x0203F8F0}. idx = (u8)c: t =
// s16[0x080CBC74 + 2*idx]; t==-1 returns; else v =
// s16[0x0203F8F2 + 2*(t-1)] and _08002ED0(a, b, v, d, 1, 0, 1, 0, 0, 1).
// Three lowering facts, all load-bearing:
//  * The first table base is a SYMBOL declared between the two shifts
//    (`shifted = c<<24; base = Symbol; offset = shifted>>23`), so agbcc
//    loads it between `lsls r2,#24` and `lsrs r2,#23` (same idiom as
//    Ai_CatalogA/B/C in src/ai_catalog.c).
//  * The load width is PLAIN `s16` (no `volatile`): `volatile` emits
//    `ldrh`+`lsls #16`+`asrs #16`, plain gives the ROM's single `ldrsh`
//    with a zero register.
//  * The second address keeps `subs/lsls/adds #2/adds` by computing the
//    byte offset before bumping the pointer (`p = P2; u = t-1;
//    off = u*2; p += 1; v = *(p_addr + off)`): the `P2+1` fold that drops
//    `subs`/`adds #2` needs a constant base, and a symbol base keeps it.
//    Declaring `p` before `u` is what keeps `t` in r2 (and so the second
//    base in r1, index in r0); the reverse order swaps them.
// NOTE: incoming r3 (d) is never stored, only forwarded.
// ----------------------------------------------------------------------------
void _08001FB58(int a, int b, int c, int d) {
    u32 shifted;
    u32 base;
    u32 offset;
    s16 *p;
    int u;
    s16 t;
    s16 v;
    u32 off;
    extern u8 RaceSceneD1Tbl74[];
    extern s16 RaceSceneD1RamF8F0[];
    __asm__(".globl RaceSceneD1Tbl74\nRaceSceneD1Tbl74 = 0x080CBC74");
    __asm__(".globl RaceSceneD1RamF8F0\nRaceSceneD1RamF8F0 = 0x0203F8F0");
    shifted = (u32)c << 24;
    base = (u32)(uintptr_t)RaceSceneD1Tbl74;
    offset = shifted >> 23;
    t = *(s16 *)(uintptr_t)(offset + base);
    if (t == -1)
        return;
    p = RaceSceneD1RamF8F0;
    u = (int)t - 1;
    off = (u32)((u32)u * 2);
    p += 1;
    v = *(s16 *)(uintptr_t)((u32)(uintptr_t)p + off);
    _08002ED0((void *)(u32)a, b, (int)v, d, 1, 0, 1, 0, 0, 1);
}

// ----------------------------------------------------------------------------
// ---- 0x0801FBAC — scene-header class select into rec+84 (1/6) ----
// No pools. NOTE: incoming r0 dead (never read); r1 is the record
// (same shape as _08001F668). u16[rec+84] = 1 for class 27..30, else 6.
// Same two lowering facts as _08001F668 above, both load-bearing:
//  * The class word is read through a PLAIN `s16 *`. `volatile` cannot be
//    folded into LDRSH and emits ldrh + lsls#16 + asrs#16 (8 bytes), the
//    +4-byte OVERSIZED this body used to report.
//  * The 27..30 membership test must be a `switch` over the four consecutive
//    case values. `v >= 27 && v <= 30` is canonicalised by gcc-2.95's cse into
//    the range check `subs #27 / lsls #16 / lsrs #16 / cmp #3 / bhi`, and a
//    nested `if` spelling still folds the lower bound to `cmp #26 / ble`. The
//    switch keeps both compares (`cmp #30 / bgt`, `cmp #27 / blt`) because
//    the case tree emits them after the fold step.
// ----------------------------------------------------------------------------
void _08001FBAC(int a0, void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 v;
    s16 *p;
    (void)a0;
    p = (s16 *)(void *)((u8 *)_08004B68() + 2);
    v = *p;
    switch (v) {
    case 27:
    case 28:
    case 29:
    case 30:
        *(volatile u16 *)(rec + 84) = 1;
        break;
    default:
        *(volatile u16 *)(rec + 84) = 6;
        break;
    }
}
// The body is 42 bytes, so its `-ffunction-sections` section is padded to 44.
// gas closes a Thumb code section with the 2-byte `nop` filler (0x46c0), but the
// ROM pads with `00 00` (asm/race_scene.s writes it as `movs r0, r0`). A
// file-scope `.align 2, 0` is emitted after the body's `.size`, i.e. still
// inside the body's own section, and pads with the explicit `0` fill.
__asm__(".align 2, 0");

#ifndef __APPLE__
void sub_08001FBAC(int a0, void *rec_) __attribute__((alias("_08001FBAC")));
#endif

// ----------------------------------------------------------------------------
// ---- 0x0801FBD8 — car-record block copy via D95C ----
// Pools: {0x03002808, 0xFFFFF55C}. base = 0x03002808-0xAA4 = 0x03001D64
// (car-record base): _0800D95C(0x03002808,
// 0x03001D64 + 12*s16[rec+156] + 72*s16[rec+28] + 8, 3).
// ----------------------------------------------------------------------------
void _08001FBD8(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 a = *(volatile s16 *)(rec + 28);
    s16 b = *(volatile s16 *)(rec + 156);
    const void *src = (const void *)(uintptr_t)(0x03001D64u + (u32)((s32)b * 12) +
                                                (u32)((s32)a * 72) + 8u);
    _0800D95C((void *)(uintptr_t)0x03002808u, src, 3);
}
