// ============================================================================
// race_scene_d2.c — reconstructed C for asm/race_scene.s (5 functions):
//
//   _08001EE2C (0x0801EE2C) — key/counter handler: setup latches + lane walk
//   _08001EFAC (0x0801EFAC) — big scene assembly: 7B18/7BFC chains + grid loops
//   _08001FA64 (0x0801FA64) — zero-terminated byte-stream 2ED0 emitter
//   _08001FADC (0x0801FADC) — counted byte-stream 2ED0 emitter (hidden s0 mode)
//   _08001FC14 (0x0801FC14) — scene setup: FBC switch + class branch + ctor chain
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

HOST_STUB(void _08002B368(u16 v));                                              // 0x08002B368 sound cue
HOST_STUB(void _08002B214(int v));                                              // 0x08002B214 sound wake
HOST_STUB(void *_08004B68(void));                                               // 0x08004B68 scene mgr
HOST_STUB(void _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h)); // 0x08007B18 emit
HOST_STUB(void Sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i)); // 0x08007BFC exact ROM
HOST_STUB(void _08002ED0(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j)); // 0x08002ED0 10 machine args
HOST_STUB(void _08007770(int a, void *b, int c, int d, u32 e, u32 f));         // 0x08007770 6 machine args
HOST_STUB(void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f));      // 0x08007770 exact ROM body
HOST_STUB(void _080075E8(void *a, int b, int c));                               // 0x080075E8 emit lane
HOST_STUB(void _0800798C(void *a, void *b));                                    // 0x0800798C resource setup
HOST_STUB(void _08007A58(void *p));                                             // 0x08007A58 resource init
HOST_STUB(void _08007ABC(void *a, void *b, int c));                             // 0x08007ABC resource bind
HOST_STUB(int _0800572C(int v));                                                // 0x0800572C IWRAM bump
HOST_STUB(s16 _0800258A8(int idx));                                             // 0x0800258A8 table lookup
HOST_STUB(int _080025F20(int a));                                               // 0x080025F20 grid count
HOST_STUB(void _080026A2C(void *a, s16 b, u16 c));                              // 0x080026A2C sprite XY
HOST_STUB(void _0800DBE8(void *p));                                             // 0x0800DBE8 record setup
HOST_STUB(void _0800DAB8(void *p));                                             // 0x0800DAB8 obj init
HOST_STUB(void _0800D77C(void *a, int b, int c));                               // 0x0800D77C slide setup
HOST_STUB(int _08001F9EC(int x));                                               // 0x08001F9EC s16 table 0x080CBE78
HOST_STUB(int _08001F9FC(int x));                                               // 0x08001F9FC s16 table 0x080CBE82
HOST_STUB(void _08001FA0C(void));                                               // 0x08001FA0C 64-lane template init
HOST_STUB(void _08001FE24(void *rec));                                          // 0x08001FE24 record setup (race_scene_e.c)
HOST_STUB(void _08001FF24(void *rec));                                          // 0x08001FF24 record setup (race_scene_e.c)
HOST_STUB(void _08001FF7C(void *rec));                                          // 0x08001FF7C record setup (race_scene_e.c)

// ---- shared anchors --------------------------------------------------------
#define WA 0x03001780u
static inline u8  R8(volatile u8 *p, u32 o)  { return *(volatile u8 *)(p + o); }
static inline s8  S8(volatile u8 *p, u32 o)  { return *(volatile s8 *)(p + o); }
static inline u16 R16(volatile u8 *p, u32 o) { return *(volatile u16 *)(p + o); }
static inline s16 S16(volatile u8 *p, u32 o) { return *(volatile s16 *)(p + o); }
static inline u32 R32(volatile u8 *p, u32 o) { return *(volatile u32 *)(p + o); }
static inline void W8(volatile u8 *p, u32 o, u8 v)   { *(volatile u8 *)(p + o) = v; }
static inline void W16(volatile u8 *p, u32 o, u16 v) { *(volatile u16 *)(p + o) = v; }
static inline void W32(volatile u8 *p, u32 o, u32 v) { *(volatile u32 *)(p + o) = v; }

// ----------------------------------------------------------------------------
// ---- 0x08001EE2C — key/counter handler: setup latches + lane walk ----
// Pools: {WA 0x03001780, table 0x080CBC24}. r0=rec, r1 dead, r2=(u16)c.
// Entry snapshots: r7=u16[rec+142], r8=u16[rec+136] (compared at the tail).
// ----------------------------------------------------------------------------
void _08001EE2C(void *rec0, u16 b, u16 c) {
    volatile u8 *rec = (volatile u8 *)rec0;
    u16 r7 = R16(rec, 142);
    int r8 = (int)R16(rec, 136);
    (void)b;
    if (c == 2) {
        W32(rec, 24, 10);
        W16(rec, 28, 0);
        W32(rec, 56, 10);
        W32(rec, 52, 0);
        _08002B368(4);
    }
    if (c == 1) {
        int s = (int)S16(rec, 136);
        if (s <= 7) {
            if (R16(rec, (u32)(144 + 2 * s)) != 0) {
                W32(rec, 24, 10);
                W16(rec, 28, 0);
                W32(rec, 56, 10);
                W32(rec, 52, 1);
                W16(rec, 138, 4);
                *(volatile u16 *)(uintptr_t)(WA + 0xFC0u) =
                    *(volatile u16 *)(uintptr_t)(0x080CBC24u + (u32)(2 * (int)S16(rec, 136)));
                _08002B368(1);
            } else {
                _08002B368(10);
            }
        } else if (s == 8) {
            W32(rec, 24, 10);
            W16(rec, 28, 0);
            W32(rec, 56, 10);
            W32(rec, 52, 0);
            _08002B368(1);
        }
    }
    if (c == 32) {
        if (R16(rec, 136) == 8) {
            W16(rec, 136, 3);
        } else {
            int v = (int)S16(rec, 136) - 4;
            if (v >= 0)
                W16(rec, 136, (u16)v);
        }
    }
    if (c == 16) {
        if (R16(rec, 136) == 8) {
            W16(rec, 136, 7);
        } else {
            int v = (int)S16(rec, 136) + 4;
            if (v <= 7)
                W16(rec, 136, (u16)v);
        }
    }
    if (c == 64) {
        if (R16(rec, 136) == 8) {
            int t = (int)S16(rec, 142);
            if (t == 0)
                W16(rec, 136, 3);
            else if (t == 1)
                W16(rec, 136, 7);
        } else {
            int v = (int)R16(rec, 136) - 1;
            W16(rec, 136, (u16)v);
            if ((s16)(u16)v == 3)
                W16(rec, 136, 4);
        }
    }
    if (c == 128) {
        int v = (int)R16(rec, 136) + 1;
        W16(rec, 136, (u16)v);
        if ((s16)(u16)v == 4)
            W16(rec, 136, 8);
    }
    if (R16(rec, 136) <= 3)
        W16(rec, 142, 0);
    if ((u16)((int)R16(rec, 136) - 4) <= 3u)
        W16(rec, 142, 1);
    if (S16(rec, 136) <= 0)
        W16(rec, 136, 0);
    if (S16(rec, 136) > 7)
        W16(rec, 136, 8);
    if (R16(rec, 142) != r7)
        _08002B368(3);
    else if (R16(rec, 136) != (u16)r8)
        _08002B368(2);
}

// ----------------------------------------------------------------------------
// ---- 0x08001EFAC — big scene assembly: 7B18/7BFC chains + grid loops ----
// Pools: {0x080CBBE4 (8-byte entry table), WA 0x03001780 (s8 thresholds)}.
// r0=rec only. armcc high-register frame (r5=3, r6=1, sl=0 staging the
// shared (3,1,1,0) stack tail of the lane emits).
// ----------------------------------------------------------------------------
void _08001EFAC(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    /* Head: template emit + sprite XY + eight lane emits on rec+16. */
    _08007B18((void *)(rec + 60), 8, 72, 88, 6, 3, 1, 0);
    _080026A2C((void *)(uintptr_t)R32(rec, 268), 120, 56);
    _08007B18((void *)(rec + 16), 2, 8, 40, 3, 1, 1, 0);
    _08007B18((void *)(rec + 16), 8, 0, 64, 3, 1, 1, 0);
    _08007B18((void *)(rec + 16), 6, 0, 88, 3, 1, 1, 0);
    _08007B18((void *)(rec + 16), 5, 16, 112, 3, 1, 1, 0);
    _08007B18((void *)(rec + 16), 3, 192, 40, 3, 1, 1, 0);
    _08007B18((void *)(rec + 16), 7, 200, 64, 3, 1, 1, 0);
    _08007B18((void *)(rec + 16), 4, 200, 88, 3, 1, 1, 0);
    _08007B18((void *)(rec + 16), 1, 192, 112, 3, 1, 1, 0);
    /* Eight 9-arg record emits on rec+8 (d switches 48 -> 168 halfway). */
    Sub_08007BFC((void *)(rec + 8), R32(rec, 160), R32(rec, 164), 48, 40, R32(rec, 168), 1, 1, 0);
    Sub_08007BFC((void *)(rec + 8), R32(rec, 172), R32(rec, 176), 48, 64, R32(rec, 180), 1, 1, 0);
    Sub_08007BFC((void *)(rec + 8), R32(rec, 184), R32(rec, 188), 48, 88, R32(rec, 192), 1, 1, 0);
    Sub_08007BFC((void *)(rec + 8), R32(rec, 196), R32(rec, 200), 48, 112, R32(rec, 204), 1, 1, 0);
    Sub_08007BFC((void *)(rec + 8), R32(rec, 208), R32(rec, 212), 168, 40, R32(rec, 216), 1, 1, 0);
    Sub_08007BFC((void *)(rec + 8), R32(rec, 220), R32(rec, 224), 168, 64, R32(rec, 228), 1, 1, 0);
    Sub_08007BFC((void *)(rec + 8), R32(rec, 232), R32(rec, 236), 168, 88, R32(rec, 240), 1, 1, 0);
    Sub_08007BFC((void *)(rec + 8), R32(rec, 244), R32(rec, 248), 168, 112, R32(rec, 252), 1, 1, 0);
    /* Field-gated emit. NOTE: ROM is two branches around a reloaded compare
       (u16[rec+136]==8 -> second lane; else first lane, reload, bne past the
       second); memory is untouched between, so this if/else is exact. */
    if (R16(rec, 136) == 8) {
        _08007B18((void *)rec, R16(rec, 138), 104, 128, 5, 1, 1, 0);
    } else {
        int s = (int)S16(rec, 136);
        _08007B18((void *)rec, 10,
                   *(volatile u32 *)(uintptr_t)(0x080CBBE4u + (u32)(8 * s)),
                   *(volatile u32 *)(uintptr_t)(0x080CBBE4u + 4u + (u32)(8 * s)),
                   4, 1, 1, 0);
    }
    /* Pass 1: r5 = 0..2, x (r6) = 40, 32, 24; sl = rec+130 throughout.
       NOTE: the body also stores rec+24 to [sp,#20] once; no callee reads
       caller stack word 5 (7B18 takes 4, 7BFC takes 5 at [sp,#0..16]), so
       it is a dead armcc slot with no C counterpart. */
    for (int r5 = 0, r6 = 40; r5 <= 2; r5++, r6 -= 8) {
        if (r5 < _080025F20(0)) {
            int v = (int)*(volatile s8 *)(uintptr_t)(WA + 12u * (u32)S16(rec, 130) + 50u);
            if (r5 < v)
                _08007B18((void *)rec, 11, r6, 48, 3, 1, 1, 0);
            else
                _08007B18((void *)rec, 12, r6, 48, 3, 1, 1, 0);
        }
        if (r5 < _080025F20(1)) {
            int v = (int)*(volatile s8 *)(uintptr_t)(WA + 12u * (u32)S16(rec, 130) + 54u);
            if (r5 < v)
                _08007B18((void *)rec, 11, r6, 72, 3, 1, 1, 0);
            else
                _08007B18((void *)rec, 12, r6, 72, 3, 1, 1, 0);
        }
        if (r5 < _080025F20(2)) {
            int v = (int)*(volatile s8 *)(uintptr_t)(WA + 12u * (u32)S16(rec, 130) + 52u);
            if (r5 < v)
                _08007B18((void *)rec, 11, r6, 96, 3, 1, 1, 0);
            else
                _08007B18((void *)rec, 12, r6, 96, 3, 1, 1, 0);
        }
        if (r5 < _080025F20(3)) {
            int v = (int)*(volatile s8 *)(uintptr_t)(WA + 12u * (u32)S16(rec, 130) + 55u);
            if (r5 < v)
                _08007B18((void *)rec, 11, r6, 120, 3, 1, 1, 0);
            else
                _08007B18((void *)rec, 12, r6, 120, 3, 1, 1, 0);
        }
    }
    /* Pass 2: r5 = 0..2, x (r6) = 192, 200, 208. */
    for (int r5 = 0, r6 = 192; r5 <= 2; r5++, r6 += 8) {
        if (r5 < _080025F20(4)) {
            int v = (int)*(volatile s8 *)(uintptr_t)(WA + 12u * (u32)S16(rec, 130) + 53u);
            if (r5 < v)
                _08007B18((void *)rec, 11, r6, 48, 3, 1, 1, 0);
            else
                _08007B18((void *)rec, 12, r6, 48, 3, 1, 1, 0);
        }
        if (r5 < _080025F20(5)) {
            int v = (int)*(volatile s8 *)(uintptr_t)(WA + 12u * (u32)S16(rec, 130) + 57u);
            if (r5 < v)
                _08007B18((void *)rec, 11, r6, 72, 3, 1, 1, 0);
            else
                _08007B18((void *)rec, 12, r6, 72, 3, 1, 1, 0);
        }
        if (r5 < _080025F20(6)) {
            int v = (int)*(volatile s8 *)(uintptr_t)(WA + 12u * (u32)S16(rec, 130) + 51u);
            if (r5 < v)
                _08007B18((void *)rec, 11, r6, 96, 3, 1, 1, 0);
            else
                _08007B18((void *)rec, 12, r6, 96, 3, 1, 1, 0);
        }
        if (r5 < _080025F20(7)) {
            int v = (int)*(volatile s8 *)(uintptr_t)(WA + 12u * (u32)S16(rec, 130) + 56u);
            if (r5 < v)
                _08007B18((void *)rec, 11, r6, 120, 3, 1, 1, 0);
            else
                _08007B18((void *)rec, 12, r6, 120, 3, 1, 1, 0);
        }
    }
    _0800DBE8((void *)(rec + 24));
}

void _08001FA64(void *dst0, int b, const u8 *tbl0, int d) {
    volatile u8 *dst = (volatile u8 *)dst0;
    int bb = b;
    int dd = d;
    const u8 *tbl = (const u8 *)tbl0;
    const s16 *tab2;
    if (*tbl == 0)
        return;
    tab2 = (const s16 *)0x0203F8F2u;
    for (;;) {
        int s = (int)*(const s16 *)(0x080CBC74u + 2u * (u32)*tbl);
        if (s != -1) {
            int idx = s - 1;
            int w = (int)*(const s16 *)((const u8 *)tab2 + 2u * (u32)idx);
            _08002ED0((void *)dst, bb, w, dd, 1, 0, 1, 0, 0, 1);
            tbl += 1;
            dst += 8;
        }
        if (*tbl == 0)
            break;
    }
}

// asm/race_scene.s:10606-10608 defines BOTH `sub_08001FA64:` and
// `_08001FA64:` on this one body, and asm/race_scene.s:12006,12054,12174,
// 12289 call it as `bl sub_08001FA64`. A promoted entry's export must
// therefore carry both spellings; the `sub_` twin is a direct alias of the
// C body with the same signature (same pattern as _08001F918).
#ifndef __APPLE__
void sub_08001FA64(void *dst0, int b, const u8 *tbl0, int d) __attribute__((alias("_08001FA64")));
#endif

void _08001FADC(void *dst0, int b, const u8 *tbl0, int n, int mode) {
    const u8 *tbl = (const u8 *)tbl0;
    volatile u8 *dst;
    const s16 *tab2;
    int i;
    int one;
    int zero;
    if (n <= 0)
        return;
    tab2 = (const s16 *)0x0203F8F2u;
    dst = (volatile u8 *)dst0;
    one = 1;
    zero = 0;
    for (i = n; i != 0; i--) {
        int s = (int)*(const s16 *)(0x080CBC74u + 2u * (u32)*tbl);
        if (s != -1) {
            int idx = s - 1;
            int w = (int)*(const s16 *)((const u8 *)tab2 + 2u * (u32)idx);
            _08002ED0((void *)dst, b, w, mode, one, zero, one, zero, zero, one);
            tbl += 1;
        }
        dst += 8;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x08001FC14 — scene setup: FBC switch + class branch + ctor chain ----
// Pools: {WA+0xFBC (s16 switch key), 0x08344450 (template), 0x082A798C,
// 0x082DFBDC, WA+0x576, 0x08345F84}. r0=rec only.
// FBC table at 0x0801FC40 (mov pc): idx 0/1/4/5/7 -> FC60 (kind 3),
// idx 2 -> FC7C (kind 4), idx 3/6 -> FC90 (empty tail), idx >7 -> FC90.
// Class branch on s16[_08004B68+0]: 24 -> wake+kind7 lane, 25 ->
// kind13 lane (+3 into rec+24 when WA+0xFBC==2), else straight to ctor.
// Tail switch on s16[rec+24] is a ROM cmp-chain (0/2/3), not a table.
// ----------------------------------------------------------------------------
void _08001FC14(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    volatile u8 *wa = (volatile u8 *)WA;
    int fbc = (int)S16(wa, 0xFBC);
    int alloc;
    int w;
    if (fbc >= 0 && fbc <= 7) {
        switch (fbc) {
        case 0:
        case 1:
        case 4:
        case 5:
        case 7:
            Sub_08007770(0, (void *)(uintptr_t)0x08344450u, 3, 0, 4, 1);
            break;
        case 2:
            Sub_08007770(0, (void *)(uintptr_t)0x08344450u, 4, 0, 4, 1);
            break;
        default:
            break;
        }
    }
    W32(rec, 104, 6);
    {
        s16 cls = S16((volatile u8 *)_08004B68(), 0);
        if (cls == 24) {
            _08002B214(51);
            W32(rec, 116, 7);
            W32(rec, 128, 1);
            W16(rec, 24, 0);
        } else if (cls == 25) {
            W32(rec, 116, 13);
            W32(rec, 128, 0);
            W16(rec, 24, 2);
            if (S16(wa, 0xFBC) == 2)
                W16(rec, 24, 3);
        }
    }
    _0800DAB8((void *)(rec + 32));
    _080075E8((void *)(uintptr_t)0x082A798Cu, 0, 7);
    _0800798C((void *)(uintptr_t)0x08344450u, (void *)rec);
    _08007A58((void *)rec);
    _0800798C((void *)(uintptr_t)0x082DFBDCu, (void *)(rec + 8));
    _080075E8((void *)(uintptr_t)0x082DFBDCu, 0, 6);
    alloc = _0800572C(12);
    W32(rec, 176, (u32)alloc);
    w = (int)_0800258A8((int)S16(wa, 0x576));
    W32(rec, 180, (u32)(s32)(s16)w);
    _08007ABC((void *)(uintptr_t)R32(rec, 12), (void *)(uintptr_t)(u32)(s32)(s16)w, (int)R32(rec, 176));
    _0800798C((void *)(uintptr_t)0x08345F84u, (void *)(rec + 16));
    alloc = _0800572C(6);
    W32(rec, 188, (u32)alloc);
    w = _08001F9EC(0);
    W32(rec, 192, (u32)(s32)(s16)w);
    _08007ABC((void *)(uintptr_t)R32(rec, 20), (void *)(uintptr_t)(u32)(s32)(s16)w, (int)R32(rec, 188));
    _0800798C((void *)(uintptr_t)0x08345F84u, (void *)(rec + 200));
    alloc = _0800572C(1);
    W32(rec, 200, (u32)alloc);
    w = _08001F9FC(0);
    W32(rec, 204, (u32)(s32)(s16)w);
    _08007ABC((void *)(uintptr_t)R32(rec, 20), (void *)(uintptr_t)(u32)(s32)(s16)w, (int)R32(rec, 200));
    _08001FA0C();
    W32(rec, 32, 0);
    W16(rec, 36, 1);
    _0800D77C((void *)(rec + 52), 0, -32);
    _0800D77C((void *)(rec + 44), 0, 160);
    W16(rec, 40, 6);
    W16(rec, 42, 5);
    {
        int sel = (int)S16(rec, 24);
        if (sel == 0)
            _08001FE24((void *)rec);
        else if (sel == 2)
            _08001FF24((void *)rec);
        else if (sel == 3)
            _08001FF7C((void *)rec);
    }
}
