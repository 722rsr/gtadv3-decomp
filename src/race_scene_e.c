// ============================================================================
// race_scene_e.c — reconstructed C for asm/race_scene.s (20 functions):
//
//   _08001FE24 (0x0801FE24) — record setup, counter-lane variant
//   _08001FF24 (0x0801FF24) — record setup, lineup-probe variant
//   _08001FF7C (0x0801FF7C) — record setup, lineup-probe +32 variant
//   _08001FFD4 (0x0801FFD4) — key handler (countdown latch, lane tick, reselect)
//   _080020198 (0x08020198) — key handler (byte-map integrate, car-record writeback)
//   _0800203E8 (0x080203E8) — twin 7B18 emit on +174 (kinds 11/14, 15/16)
//   _080020498 (0x08020498) — 5-arg emit helper (mode 0: 01FADC; mode 1: 01FB58 loop)
//   _080020518 (0x08020518) — scene setup A (03B9C path + 3x 7BFC + DBE8)
//   _080020690 (0x08020690) — scene setup B (03B9C path + 7BFC + DBE8)
//   _080020784 (0x08020784) — scene setup C (03954 path + 7BFC + DBE8)
//   _08002087C (0x0802087C) — 12-way event dispatcher (table 0x08020894)
//   _080020958 (0x08020958) — FBC-gated +84 store (5/6 via 04B68 +2)
//   _0800209A0 (0x080209A0) — big record setup (templates, car walk, 8-way table)
//   _080020D28 (0x08020D28) — 5-arg emit helper (7BFC path / 7570+2ED0 path)
//   _080020E04 (0x08020E04) — key/counter handler (lane tick, clamp, rebind)
//   _080020F50 (0x08020F50) — twin 7B18 emit on +166 (kinds 11/14, 15/16)
//   _080021018 (0x08021018) — repeat emitter (kind 6 x n/2, kind 5 on odd)
//   _0800210BC (0x080210BC) — scene assembly (7B18/7BFC/D28/F50/1018 chain)
//   _080021260 (0x08021260) — FBC-gated frame counter (event 21 past 180)
//   _0800212AC (0x080212AC) — 12-way event dispatcher (table 0x080212C8)
//
// Transcribed instruction-for-instruction from asm/race_scene.s (objdump
// cross-checked). Strong `_0800XXXX` definitions so tools/coverage.py counts
// them; asm callers bind via the splicer's re-emitted aliases. No `sub_` aliases here.

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

HOST_STUB(int   _08002580C(int x));                                  // 0x08002580C table lookup
HOST_STUB(int   _08002581C(int x));                                  // 0x08002581C lineup probe
HOST_STUB(int   _08002584C(int x));                                  // 0x08002584C table lookup
HOST_STUB(int   _08002587C(int x));                                  // 0x08002587C lineup probe

// The nine-digit spellings above are host-only stubs; asm/ai_line_tail.s binds
// the EIGHT-digit labels `_0802581C:` / `_0802587C:`, which src/ai_line_more.c
// exports as aliases of Ai_LineFindCourse / Ai_LineFindIn6010C. The target
// build must therefore call the closure spelling, while the Apple host build
// keeps the stubbed nine-digit names.
#ifndef __APPLE__
#define RSE_CALLEE(friendly, closure) closure
extern int _0802581C(int x);   // 0x08002581C lineup probe
extern int _0802587C(int x);   // 0x08002587C lineup probe
#else
#define RSE_CALLEE(friendly, closure) friendly
#endif
HOST_STUB(s16   _0800258A8(int idx));                                // 0x0800258A8 numeric_leaves
HOST_STUB(int   _0800258B8(int x));                                  // 0x0800258B8 course_cal cup
HOST_STUB(int   _0800258CC(int x));                                  // 0x0800258CC course_cal seq
HOST_STUB(void  _08002124(u16 v));                                   // 0x08002124 BlockB arm
HOST_STUB(int   _08002140(void));                                    // 0x08002140 engine key
HOST_STUB(void *_08004B68(void));                                    // 0x08004B68 scene mgr
HOST_STUB(void  _08004D4C(u32 a, u32 b, u32 c));                     // 0x08004D4C event post
HOST_STUB(void  _080050B4(int a, u16 b));                            // 0x080050B4 OAM wrap
HOST_STUB(void  _08002B214(int v));                                  // 0x08002B214 sound wake
HOST_STUB(void  _08002B368(int v));                                  // 0x08002B368 sound cmd
HOST_STUB(void  _080075E8(void *a, int b, int c));                   // 0x080075E8 emit lane
HOST_STUB(void  _08007570(void *a, int b, int c, int d, int units)); // 0x08007570 5-arg (see header NOTE)
HOST_STUB(void  _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h)); // 0x08007B18 emit
HOST_STUB(void  _08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i)); // 0x08007BFC 9 machine args
HOST_STUB(void  _08007770(int a, void *b, int c, int d, u32 e, u32 f)); // 0x08007770 6 machine args
HOST_STUB(void  Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f)); // 0x08007770 exact ROM body
HOST_STUB(void  _08007ABC(void *a, void *b, int c));                 // 0x08007ABC resource bind
HOST_STUB(void  _0800798C(void *a, void *b));                        // 0x0800798C resource setup
HOST_STUB(void  _08007A58(void *p));                                 // 0x08007A58 resource init
HOST_STUB(int   _08005758(int v));                                   // 0x08005758 IWRAM alloc
HOST_STUB(int   _0800572C(int v));                                   // 0x0800572C IWRAM bump
HOST_STUB(int   _080022E4(int id));                                  // 0x080022E4 car-record map
HOST_STUB(int   _08002370(int v));                                   // 0x08002370 s16 transform
HOST_STUB(void  _08003954(int a, int b, u32 c));                     // 0x08003954 text lane
HOST_STUB(void  _08003B9C(int a, u32 b, u32 c));                     // 0x08003B9C text lane
HOST_STUB(void  _08001FA64(int a, int b, int c, int d));             // 0x08001FA64 emit loop
HOST_STUB(void  _08001FADC(int a, int b, void *c, int d, int e));    // 0x08001FADC 5 machine args (hidden s0 explicit)
HOST_STUB(void  _08001FB58(int a, int b, int c, int d));             // 0x08001FB58 emit leaf (r3 passes through to 2ED0)
// 0x08001FBAC takes the record SECOND: asm/race_scene.s:10780-10782 is
// `push {r4,lr}; adds r4, r1, #0` before the bl. The definition at
// src/race_scene_d1.c:619 is (int a0, void *rec_) and agrees. This weak stub
// had the pair reversed, so this call passed the record in r0.
HOST_STUB(void  _08001FBAC(int a0, void *rec));
HOST_STUB(void  _08001FBD8(void *rec));                              // 0x08001FBD8 record copy
HOST_STUB(void  _08001FC14(void *rec));                              // 0x08001FC14 FBC dispatch
HOST_STUB(int   _08001F9EC(int x));                                  // 0x08001F9EC s16 table 0x080CBE78
HOST_STUB(int   _08001F9FC(int x));                                  // 0x08001F9FC s16 table 0x080CBE82
HOST_STUB(void  _0800D95C(void *dst, const void *src, int n));       // 0x0800D95C memcpy
HOST_STUB(void  _0800D97C(void *p, int v));                          // 0x0800D97C record rebind
HOST_STUB(void  _0800D77C(void *a, int b, int c));                   // 0x0800D77C slide setup
HOST_STUB(void  _0800DAB8(void *p));                                 // 0x0800DAB8 obj init
HOST_STUB(void  _0800D854(void *p));                                 // 0x0800D854 record tick
HOST_STUB(void  _0800DBE8(void *p));                                 // 0x0800DBE8 record setup
HOST_STUB(void  _0800D9A4(void *a, int b, int c));                   // 0x0800D9A4 grid paint
HOST_STUB(void  _080020878(void));                                   // 0x080020878 bx-lr (race_scene.c)
HOST_STUB(void  _08002099C(void *a));                                // 0x08002099C bx-lr (race_scene.c)
HOST_STUB(void  _080021248(void));                                   // 0x080021248 bx-lr (race_scene.c)
HOST_STUB(void  _08002124C(void *a));                                // 0x08002124C leaf (race_scene.c)
HOST_STUB(s8    _08025500(int a));                                   // 0x08025500 line get 1
HOST_STUB(s8    _08025518(int a));                                   // 0x08025518 line get 2
HOST_STUB(s8    _08025530(int a));                                   // 0x08025530 line get 3
HOST_STUB(int   _08025548(const void *a));                           // 0x08025548 line sum
HOST_STUB(int   _080255C4(const void *a));                           // 0x080255C4 line sum A
HOST_STUB(int   _08025640(const void *a));                           // 0x08025640 line sum B
HOST_STUB(int   _08025F20(int a));                                   // 0x08025F20 grid hw get
HOST_STUB(void  _080026A2C(void *a, s16 b, u16 c));                  // 0x080026A2C sprite XY
HOST_STUB(void  _08002ED0(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j)); // 0x08002ED0 10 machine args

// Forward declarations of later-defined tranche lifts used below.
void _080020958(void *unused, void *rec);
void _0800209A0(void *rec);
void _080020E04(void *rec, int b, int key);
void _080020F50(void *rec, int sel);
void _080021018(void *rec, int a, int b, int c);
void _08001FFD4(void *rec, int a, int b);
void _080020198(void *rec, int a, int b);
void _080020498(void *rec, int a, int b, void *area, int mode);
void _080020518(void *rec);
void _080020690(void *rec);
void _080020784(void *rec);
void _080020D28(void *rec, int a, int b, int gate, int h5);

// Absolute-address symbols for _080020F50; defined inside that body because the
// per-body splice carries only brace-matched text from the function.
extern const u8 RaceSceneWaF50[];
extern const u8 RaceSceneTblF50[];
extern const u8 RaceSceneWaD28[];
extern const u8 RaceSceneTblD28[];
extern const u8 RaceSceneWa210BC[];

#define WA 0x03001780u
static inline u8  R8(volatile u8 *p, u32 o)  { return *(volatile u8 *)(p + o); }
static inline s8  S8(volatile u8 *p, u32 o)  { return *(volatile s8 *)(p + o); }
static inline u16 R16(volatile u8 *p, u32 o) { return *(volatile u16 *)(p + o); }
static inline s16 S16(volatile u8 *p, u32 o) { return *(volatile s16 *)(p + o); }
static inline u32 R32(volatile u8 *p, u32 o) { return *(volatile u32 *)(p + o); }
// Non-volatile s16 read: agbcc lowers this to a true indexed `ldrsh`, where a
// volatile s16 load becomes ldrh+lsls+asrs and truncates the sign.
static inline s16 RD16(volatile u8 *p, u32 o) { return *(s16 *)(p + o); }
static inline void W8(volatile u8 *p, u32 o, u8 v)   { *(volatile u8 *)(p + o) = v; }
static inline void W16(volatile u8 *p, u32 o, u16 v) { *(volatile u16 *)(p + o) = v; }
static inline void W32(volatile u8 *p, u32 o, u32 v) { *(volatile u32 *)(p + o) = v; }
// Store through a u16 lvalue from an int-typed value: the narrowing is the
// strh itself, so no lsls/lsrs pair is emitted (an explicit (u16) cast is).
static inline void W16I(volatile u8 *p, u32 o, int v) { *(volatile u16 *)(p + o) = v; }
static inline void WS(volatile u8 *p, u16 v) { *(volatile u16 *)p = v; }

// ---- 0x0801FE24 — record setup, counter-lane variant + pools {WA+0xFC8/0xFCA/0xFCC} ----
void _08001FE24(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    volatile u8 *wa;
    W16(rec, 28, 0);
    W16I(rec, 26, _08002580C(0));
    wa = (volatile u8 *)WA;
    WS(rec + 160, RD16(wa, 0xFC8));
    WS(rec + 162, RD16(wa, 0xFCA));
    WS(rec + 156, RD16(wa, 0xFCC));
    int t = (int)_0800258A8((int)RD16(rec, 26));
    W32(rec, 180, (u32)(s16)t);
    _08007ABC((void *)R32(rec, 12), (void *)(uintptr_t)(u32)t, (int)R32(rec, 176));
    // ROM: `movs r1,#1 / cmp r0,#31 / bgt / movs r1,#0` — the TRUE value is
    // preloaded and the branch SKIPS the false assignment, which is the shape
    // of `h = 1; if (x <= 31) h = 0;`, not of a `? :` ternary (agbcc inverts
    // a ternary's condition and lays the false arm inline). The following
    // `lsls r0,r1,#16 / adds r5,r0,#0` is the u16 -> int widening of `h`,
    // and r5 is the callee-saved home because `hi` is live across three bls.
    u16 h = 1;
    if (RD16(rec, 28) <= 31)
        h = 0;
    int hi = h;
    if (!hi) {
        int u = (int)(s16)_0800258B8((int)RD16(rec, 26));
        int w = _08001F9EC(u);
        W32(rec, 192, (u32)(s16)w);
        _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)w, (int)R32(rec, 188));
    } else {
        W32(rec, 192, 13);
        _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)R32(rec, 188), 13);
    }
    if (!hi) {
        int u2 = (int)(s16)_0800258CC((int)RD16(rec, 26));
        int w2 = _08001F9FC(u2);
        W32(rec, 204, (u32)(s16)w2);
        _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)w2, (int)R32(rec, 200));
    } else {
        W32(rec, 204, 14);
        _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)R32(rec, 200), 14);
    }
}

// ---- 0x0801FF24 — record setup, lineup-probe variant + pools {WA+0x576, 0xFC8/0xFCA/0xFCC} ----
void _08001FF24(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    volatile u8 *wa = (volatile u8 *)WA;
    W16(rec, 26, R16(wa, 0x576));
    W16I(rec, 28, RSE_CALLEE(_08002581C, _0802581C)((int)RD16(rec, 26)));
    WS(rec + 160, RD16(wa, 0xFC8));
    WS(rec + 162, RD16(wa, 0xFCA));
    WS(rec + 156, RD16(wa, 0xFCC));
}

#ifndef __APPLE__
void sub_08001FF24(void *rec0) __attribute__((alias("_08001FF24")));
#endif

// ---- 0x0801FF7C — record setup, lineup-probe +32 variant + pools {WA+0x576, 0xFC8/0xFCA/0xFCC} ----
void _08001FF7C(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    volatile u8 *wa = (volatile u8 *)WA;
    W16(rec, 26, R16(wa, 0x576));
    W16I(rec, 28, RSE_CALLEE(_08002587C, _0802587C)((int)RD16(rec, 26)) + 32);
    WS(rec + 160, RD16(wa, 0xFC8));
    WS(rec + 162, RD16(wa, 0xFCA));
    WS(rec + 156, RD16(wa, 0xFCC));
}

#ifndef __APPLE__
void sub_08001FF7C(void *rec0) __attribute__((alias("_08001FF7C")));
#endif

// ---- 0x0801FFD4 — key handler: countdown latch, lane tick, reselect + pools {0x08344450 x2} ----
void _08001FFD4(void *rec0, int a1, int a2) {
    volatile u8 *rec = (volatile u8 *)rec0;
    u16 r5 = (u16)a1;
    u16 r2 = (u16)a2;
    u16 r8 = R16(rec, 26);
    // ROM: `movs r1,#28 / ldrsh r0,[r4,r1] / movs r7,#1 / cmp r0,#31 / bgt /
    // movs r7,#0` — the TRUE value is preloaded and the branch SKIPS the false
    // assignment. A `? :` ternary makes agbcc invert the condition and lay the
    // false arm inline (`movs r7,#0 / cmp / ble / movs r7,#1`), and the
    // volatile S16 helper costs an ldrh + lsls + asrs where the ROM has one
    // ldrsh.
    int r7 = 1;
    if (RD16(rec, 28) <= 31)
        r7 = 0;
    if ((u16)(r2 - 1u) <= 1u) {
        _08002B368(4);
        W32(rec, 32, 10);
        W16(rec, 36, 0);
        W32(rec, 64, 10);
        W32(rec, 60, 0);
    }
    volatile u8 *lane = rec + 152;
    if ((r5 & 32u) && S16(lane, 0) <= 0) {
        W16(rec, 28, (u16)(R16(rec, 28) - 1u));
        W16(lane, 0, 10);
    }
    if ((r5 & 16u) && S16(lane, 0) <= 0) {
        W16(rec, 28, (u16)(R16(rec, 28) + 1u));
        W16(lane, 0, 10);
    }
    {
        int t = (int)R16(lane, 0) - 1;
        W16(lane, 0, (u16)t);
        if ((s16)(u16)t <= 0)
            W16(lane, 0, 0);
    }
    if (S16(rec, 28) > 33)
        W16(rec, 28, 34);
    if (S16(rec, 28) < 0)
        W16(rec, 28, 0);
    int newsel;
    if (S16(rec, 28) <= 31) {
        W16(rec, 26, (u16)_08002580C(0));
        newsel = 0;
    } else {
        W16(rec, 26, (u16)_08002584C((int)S16(rec, 28) - 32));
        newsel = 1;
    }
    if (r7 != newsel) {
        _080050B4(0, 1);
        if (newsel == 0)
            Sub_08007770(0, (void *)0x08344450u, 3, 0, 4, 1);
        else
            Sub_08007770(0, (void *)0x08344450u, 4, 0, 4, 1);
    }
    if (R16(rec, 26) != r8) {
        _08002B368(3);
        int t2 = (int)_0800258A8((int)S16(rec, 26));
        W32(rec, 180, (u32)t2);
        _08007ABC((void *)R32(rec, 12), (void *)(uintptr_t)(u32)t2, (int)R32(rec, 176));
        if (newsel == 0) {
            int u = (int)_0800258B8((int)S16(rec, 26));
            int w = _08001F9EC(u);
            W32(rec, 192, (u32)w);
            _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)w, (int)R32(rec, 188));
        } else {
            W32(rec, 192, 13);
            _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)R32(rec, 188), 13);
        }
        if (newsel == 0) {
            int u2 = (int)_0800258CC((int)S16(rec, 26));
            int w2 = _08001F9FC(u2);
            W32(rec, 204, (u32)w2);
            _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)w2, (int)R32(rec, 200));
        } else {
            W32(rec, 204, 14);
            _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)R32(rec, 200), 14);
        }
    }
}

// ---- 0x08020198 — key handler: byte-map integrate + car-record writeback + pools {0x03001D64, 0x03001DA0, 0x080CBC74, 0x080CBE74 x2, WA, 0x62A} ----
void _080020198(void *rec0, int a1, int a2) {
    volatile u8 *rec = (volatile u8 *)rec0;
    int key = (int)(u16)a1;
    int sel = (int)(u16)a2;
    u8 raw[4];
    u8 map[4];
    volatile u8 *area;
    if (R16(rec, 160) == 1) {
        int s28 = (int)S16(rec, 28);
        int v = (int)S16(rec, 156);
        area = (volatile u8 *)(0x03001D64u + (u32)s28 * 72u + (u32)(v * 3) * 4u) + 8;
    } else {
        area = (volatile u8 *)(0x03001DA0u + (u32)(int)S16(rec, 28) * 72u) + 8;
    }
    {
        volatile u16 *tbl = (volatile u16 *)0x080CBC74u;
        map[0] = (u8)tbl[R8(area, 0)];
        map[1] = (u8)tbl[R8(area, 1)];
        map[2] = (u8)tbl[R8(area, 2)];
    }
    _0800D95C((void *)raw, (const void *)(uintptr_t)area, 3);
    int c164 = (int)S16(rec, 164);
    if (c164 <= 2) {
        if (sel == 1) {
            _08002B368(1);
            W16(rec, 164, (u16)(R16(rec, 164) + 1u));
        }
        if (sel == 2) {
            if (S16(rec, 164) != 0)
                _08002B368(4);
            W16(rec, 164, (u16)(R16(rec, 164) - 1u));
        }
    } else {
        if (sel == 1) {
            _08002B368(1);
            W32(rec, 32, 10);
            W16(rec, 36, 0);
            W32(rec, 64, 10);
            W32(rec, 60, (u32)sel); // r5 (==1 here)
        }
        if (sel == 2) {
            W16(rec, 164, (u16)sel); // ==2
            _08002B368(4);
        }
    }
    if (S16(rec, 164) < 0)
        W16(rec, 164, 0);
    if (S16(rec, 164) > 3)
        W16(rec, 164, 3);
    if (S16(rec, 164) <= 2) {
        volatile u8 *lane = rec + 152;
        if ((key & 64) && S16(lane, 0) <= 0) {
            _08002B368(2);
            int idx = (int)S16(rec, 164);
            map[idx] = (u8)(map[idx] - 1u);
            int v = (int)map[idx];
            if (v == 0)
                map[idx] = 53;
            else if (v > 53)
                map[idx] = 1;
            raw[idx] = *(volatile u8 *)((*(volatile u32 *)0x080CBE74u) + map[idx]);
            W16(lane, 0, 10);
        }
        if ((key & 128) && S16(lane, 0) <= 0) {
            _08002B368(2);
            int idx = (int)S16(rec, 164);
            map[idx] = (u8)(map[idx] + 1u);
            int v = (int)map[idx];
            if (v == 0)
                map[idx] = 53;
            else if (v > 53)
                map[idx] = 1;
            raw[idx] = *(volatile u8 *)((*(volatile u32 *)0x080CBE74u) + map[idx]);
            W16(lane, 0, 10);
        }
        {
            int t = (int)R16(lane, 0) - 1;
            W16(lane, 0, (u16)t);
            if ((s16)(u16)t <= 0)
                W16(lane, 0, 0);
        }
    }
    if (R16(rec, 160) == 1) {
        area[0] = raw[0];
        area[1] = raw[1];
        area[2] = raw[2];
    }
    if (R16(rec, 162) == 1) {
        volatile u8 *wa = (volatile u8 *)WA;
        volatile u8 *cb = wa + (u32)(int)S16(rec, 28) * 72u;
        cb[1576] = raw[0];
        cb[1577] = raw[1];
        cb[0x62A] = raw[2]; // 0x62A == 1578
    }
}

// ---- 0x080203E8 — twin 7B18 emit on +174 (kinds 11/14 x16, 15/16 x128) ----
void _0800203E8(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    if (RD16(rec, 28) > 0) {
        int v = (int)RD16(rec, 174);
        switch (v) {
        case 0: _08007B18((void *)(uintptr_t)(rec + 68), 11, 16, 32, 7, 1, 0, 0); break;
        case 1: _08007B18((void *)(uintptr_t)(rec + 68), 14, 16, 32, 7, 1, 0, 0); break;
        }
    }
    if (RD16(rec, 28) <= 33) {
        int v = (int)RD16(rec, 174);
        switch (v) {
        case 0: _08007B18((void *)(uintptr_t)(rec + 68), 15, 128, 32, 7, 1, 0, 0); break;
        case 1: _08007B18((void *)(uintptr_t)(rec + 68), 16, 128, 32, 7, 1, 0, 0); break;
        }
    }
}
// `_0800203E8`'s body is 174 bytes, so its `-ffunction-sections` section is
// padded to the 176-byte ROM span. Under that flag gas closes a Thumb *code*
// section with the 2-byte `nop` filler (0x46c0), where the ROM holds
// `00 00` -- which is why this scored 174/176 with every instruction already
// correct (prefix 174, first difference +0xAE). A file-scope `.align 2, 0`
// is emitted after the body's `.size`, i.e. still inside the body's own
// section, and pads with the explicit `0` fill instead. 174/176 -> EXACT.
__asm__(".align 2, 0");

// ---- 0x080020498 — 5-arg emit helper: mode 0 -> 01FADC, mode 1 -> 01FB58 loop ----
void _080020498(void *rec0, int a, int b, void *area0, int mode) {
    volatile u8 *rec = (volatile u8 *)rec0;
    volatile u8 *area = (volatile u8 *)area0;
    if (mode == 0) {
        _08001FADC(a, b, (void *)(uintptr_t)area, 3, mode);
    } else if (mode == 1) {
        volatile u8 *cur = area;
        int cursor = a;
        for (int k = 0; k <= 2; k++) {
            // ROM: call unless ([rec+164]==k AND u16[rec+174]!=1)
            if ((int)S16(rec, 164) != k || R16(rec, 174) == 1)
                _08001FB58(cursor, b, (int)*cur, 4);
            cur++;
            cursor += 8;
        }
    }
}

// ---- 0x08020518 — scene setup A (03B9C path) + pools {WA+0x5E4, 0x080CBC74 via FA64} ----
void _080020518(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    _0800D97C((void *)(uintptr_t)(rec + 172), 15);
    int s28 = (int)S16(rec, 28);
    u32 ebb = WA + 0x5E4u + (u32)s28 * 72u;
    if (s28 <= 31)
        _08003B9C(56, 74, *(volatile u32 *)ebb);
    else
        _08003954(120, 74, *(volatile u32 *)ebb);
    {
        int t = (int)*(volatile s16 *)(ebb + 1512u);
        _08001FA64(128, 78, _08002370(t), 3);
    }
    _080020498(rec0, 184, 78, (void *)(ebb + 8u), 0);
    _0800203E8(rec0);
    _08007BFC((void *)(uintptr_t)(rec + 8), (int)R32(rec, 176), (int)R32(rec, 180), 24, 33, 6, 1, 0, 0);
    _08007BFC((void *)(uintptr_t)(rec + 16), (int)R32(rec, 188), (int)R32(rec, 192), 136, 33, 6, 1, 0, 0);
    _08007BFC((void *)(uintptr_t)(rec + 16), (int)R32(rec, 200), (int)R32(rec, 204), 184, 33, 6, 1, 0, 0);
    _0800DBE8((void *)(uintptr_t)(rec + 32));
    _080020878();
}

// ---- 0x08020690 — scene setup B (03B9C path, mode-1 area select) + pools {WA+0x5E4, 0x03001D64} ----
void _080020690(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    _0800D97C((void *)(uintptr_t)(rec + 172), 5);
    int s28 = (int)S16(rec, 28);
    u32 ebb = WA + 0x5E4u + (u32)s28 * 72u;
    _08003B9C(56, 74, *(volatile u32 *)ebb);
    {
        int t = (int)*(volatile s16 *)(ebb + 1512u);
        _08001FA64(128, 78, _08002370(t), 3);
    }
    if (S16(rec, 156) == 0 && S16(rec, 160) == 1)
        _080020498(rec0, 184, 78, (void *)(ebb + 8u), 1);
    else
        _080020498(rec0, 184, 78, (void *)(0x03001D64u + (u32)s28 * 72u + 8u), 0);
    _08007BFC((void *)(uintptr_t)(rec + 8), (int)R32(rec, 176), (int)R32(rec, 180), 24, 33, 6, 1, 0, 0);
    _0800DBE8((void *)(uintptr_t)(rec + 32));
    _080020878();
}

// ---- 0x08020784 — scene setup C (03954 path, mode-1 area select) + pools {WA+0x5E4, 0x03001D64} ----
void _080020784(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    _0800D97C((void *)(uintptr_t)(rec + 172), 5);
    int s28 = (int)S16(rec, 28);
    u32 ebb = WA + 0x5E4u + (u32)s28 * 72u;
    _08003954(120, 74, *(volatile u32 *)ebb);
    {
        int t = (int)*(volatile s16 *)(ebb + 1512u);
        _08001FA64(128, 78, _08002370(t), 3);
    }
    if (S16(rec, 156) == 0 && S16(rec, 160) == 1)
        _080020498(rec0, 184, 78, (void *)(ebb + 8u), 1);
    else
        _080020498(rec0, 184, 78, (void *)(0x03001D64u + (u32)s28 * 72u + 8u), 0);
    _08007BFC((void *)(uintptr_t)(rec + 8), (int)R32(rec, 176), (int)R32(rec, 180), 24, 33, 6, 1, 0, 0);
    _0800DBE8((void *)(uintptr_t)(rec + 32));
    _080020878();
}

// ---- 0x0802087C — 12-way event dispatcher (ev-1 over table 0x08020894) ----
void _08002087C(int ev, int b, int c, void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    int idx = ev - 1;
    if ((u32)idx > 11u)
        return;
    // Table 0x08020894: ev1->0942(FC14) ev2->08C4(FBAC) ev3/4->0950(nop)
    // ev5->08CE(D854) ev6->0908(key) ev7->08D8(s24) ev8-11->0950(nop) ev12->094A(FBD8)
    switch (idx) {
    case 0:
        _08001FC14(rec0);
        break;
    case 1:
        _08001FBAC(b, rec0);
        break;
    case 4:
        _0800D854((void *)(uintptr_t)(rec + 32));
        break;
    case 5: {
        int u1 = (int)(u16)b;
        int u2 = (int)(u16)c;
        if (R16(rec, 36) == 0)
            break;
        int s24 = (int)S16(rec, 24);
        if (s24 == 0)
            _08001FFD4(rec0, u1, u2);
        else if (s24 >= 2 && s24 <= 3)
            _080020198(rec0, u1, u2);
        break;
    }
    case 6: {
        int s24 = (int)S16(rec, 24);
        if (s24 == 0)
            _080020518(rec0);
        else if (s24 == 2)
            _080020690(rec0);
        else if (s24 == 3)
            _080020784(rec0);
        break;
    }
    case 11:
        _08001FBD8(rec0);
        break;
    default:
        break;
    }
}

void _080020958(void *unused, void *rec_) {
    (void)unused;
#ifndef __APPLE__
    extern u8 RaceSceneE_Wa_20958[] __asm__("RaceSceneE_Wa_20958");
    __asm__(".globl RaceSceneE_Wa_20958\nRaceSceneE_Wa_20958 = 0x03001780\n");
    uintptr_t base = (uintptr_t)RaceSceneE_Wa_20958;
#else
    uintptr_t base = (uintptr_t)(0x03001780u);
#endif
    if (*(volatile u16 *)(uintptr_t)(base + 0xFBCu) == 3) {
        *(volatile u8 *)(uintptr_t)((volatile u8 *)rec_ + 88) = 0;
    }
    u16 car_id = *(volatile u16 *)(uintptr_t)((volatile u8 *)_08004B68() + 2);
    if (car_id == 20) {
        *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 5;
    } else {
        *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 6;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_080020958(void *unused, void *rec_) __attribute__((alias("_080020958")));
void sub_08020958(void *unused, void *rec_) __attribute__((alias("_080020958")));
#endif


// ---- 0x080209A0 — big record setup: templates, car walk, 8-way table 0x08020B70 + pools below ----
// Pools: 0x1393, WA+0xFBC, 0x08346910, 0x082A798C, 0x08342B60, 0x08348B50,
//   WA+0x574 / WA+0xFC2 select, 0x080CBEA0, table 0x08020B70, 0x080CBF30,
//   0x080CBEB0, 0x0203F990, 0x08349754, 0x080CBEF0, 0x0203F980.
void _0800209A0(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    volatile u8 *wa = (volatile u8 *)WA;
    _08002B214(49);
    u16 fbc1 = R16(wa, 0xFBC);
    if (fbc1 == 3)
        _08002124(0x1393);
    u16 fbc2 = R16(wa, 0xFBC);
    W16(rec, 144, (fbc2 == 7) ? 1 : 0);
    volatile u8 *flag144 = rec + 144;
    Sub_08007770(0, (void *)0x08346910u, 1, 0, 4, 1);
    W32(rec, 104, 6);
    W32(rec, 116, 6);
    W32(rec, 128, 1);
    _0800DAB8((void *)(uintptr_t)(rec + 32));
    _080075E8((void *)0x082A798Cu, 2, 6);
    _080075E8((void *)0x082A798Cu, 0, 9);
    _0800798C((void *)0x08342B60u, (void *)(uintptr_t)rec);
    _080075E8((void *)0x08342B60u, 0, 4);
    _08007A58((void *)(uintptr_t)rec);
    _0800798C((void *)0x08346910u, (void *)(uintptr_t)(rec + 8));
    _08007A58((void *)(uintptr_t)(rec + 8));
    _080075E8((void *)0x08346910u, 0, 3);
    _0800798C((void *)0x08348B50u, (void *)(uintptr_t)(rec + 16));
    _08007A58((void *)(uintptr_t)(rec + 16));
    _080075E8((void *)0x08348B50u, 0, 5);
    W32(rec, 184, 4);
    W32(rec, 32, 0);
    W16(rec, 36, 1);
    _0800D77C((void *)(uintptr_t)(rec + 52), 0, -32);
    _0800D77C((void *)(uintptr_t)(rec + 44), 0, 160);
    W16(rec, 40, 6);
    W16(rec, 42, 5);
    u16 fl = R16(flag144, 0);
    // u16[rec+138] = u16[(flag144==1) ? WA+0xFC2 : WA+0x574]
    W16(rec, 138, *(volatile u16 *)(fl == 1 ? (WA + 0xFC2u) : (WA + 0x574u)));
    W16(rec, 140, (u16)_080022E4((int)S16(rec, 138)));
    int ci = (int)S16(rec, 138);
    // [rec+136] = (s8)u8[WA + 12*ci + 49]
    W16(rec, 136, (u16)(s8)R8(wa, (u32)((s32)ci * 12 + 49)));
    // u32[rec+172] = s16[0x080CBEA0 + 2*s16[WA+0xFC0]]
    W32(rec, 172, (u32)(s32)*(volatile s16 *)(0x080CBEA0u + (u32)((s32)S16(wa, 0xFC0) * 2)));
    {
        // 12-byte copy [WA+12*ci+48.. +59] -> [rec+152.. +163]
        u32 co = (u32)((s32)ci * 12);
        W32(rec, 152, R32(wa, co + 48u));
        W32(rec, 156, R32(wa, co + 52u));
        W32(rec, 160, R32(wa, co + 56u));
    }
    {
        // 8-way select on s16[WA+0xFC0] via table 0x08020B70; out of
        // range (unsigned >7, covers negatives) keeps u32[rec+148] as-is
        // via a self read+write (faithful to the ROM fallthrough).
        int w = (int)S16(wa, 0xFC0);
        volatile u8 **slot = (volatile u8 **)(rec + 148);
        if ((u32)w <= 7u) {
            switch (w) {
            case 0: *slot = rec + 154; break;
            case 1: *slot = rec + 158; break;
            case 2: *slot = rec + 156; break;
            case 3: *slot = rec + 159; break;
            case 4: *slot = rec + 157; break;
            case 5: *slot = rec + 161; break;
            case 6: *slot = rec + 155; break;
            default: *slot = rec + 160; break; // case 7
            }
        } else {
            volatile u8 *tmp = *slot;
            *slot = tmp;
        }
    }
    {
        volatile u8 *ptr = *(volatile u8 **)(rec + 148);
        W16(rec, 142, (u16)(s8)ptr[0]);
    }
    int s142 = (int)S16(rec, 142);
    int w2 = (int)S16(wa, 0xFC0);
    // u32[rec+184] = s16[0x080CBF30 + 2*(5*w2 + s142)]
    W32(rec, 184, (u32)(s32)*(volatile s16 *)(0x080CBF30u + (u32)(2 * (5 * w2 + s142))));
    _0800798C((void *)0x08349754u, (void *)(uintptr_t)(rec + 24));
    int av = _08005758(16);
    // NOTE: [+2] stored before [+0]; r4==0 here (carried from the table
    // prologue), so [0x0203F990] = 0.
    *(volatile u16 *)(0x0203F990u + 2u) = (u16)av;
    *(volatile u16 *)0x0203F990u = 0;
    W32(rec, 192, (u32)_0800572C(16));
    W32(rec, 196, (u32)(s32)*(volatile s16 *)(0x080CBEB0u + (u32)(8 * w2)));
    _080075E8((void *)0x08349754u, 28, 8);
    if (s142 == 0) {
        _08007ABC((void *)R32(rec, 28), (void *)R32(rec, 196), (int)R32(rec, 192));
    } else if (s142 >= 1 && s142 <= 3) {
        u32 a1 = 0x080CBEB0u + (u32)(2 * s142 + 8 * w2);
        int q1 = (int)*(volatile s16 *)a1;
        int q2 = (int)*(volatile s16 *)(0x0203F990u + 2u);
        _08007570((void *)0x08349754u, q1, q2, 16, 16);
        u32 a2 = 0x080CBEF0u + (u32)(2 * s142 + 8 * w2);
        _080075E8((void *)0x08349754u, 7, (int)*(volatile s16 *)a2);
    }
    int w3 = (int)S16(wa, 0xFC0);
    *(volatile u16 *)(0x0203F980u + (u32)(2 * w3)) = (u16)_08025F20(w3);
    _0800D9A4((void *)(uintptr_t)(rec + 204), (int)S16(rec, 138), (int)S16(rec, 136));
}

// ---- 0x08020D28 — 5-arg emit helper + pools {0x030005A4, 0x0203F990, 0x08349754, 0x080CBEB0, WA} ----
// NOTE: h5 is the ROM caller-stack word ([sp,#48] = caller sp+0); the sole
// caller (0210BC in this file) places 7 there, passed explicitly.
void _080020D28(void *rec0, int a, int b, int gate, int h5) {
    volatile u8 *rec = (volatile u8 *)rec0;
    u32 c = *(volatile u32 *)0x030005A4u - 1u;
    *(volatile u32 *)0x030005A4u = c;
    // The pool base is a BLOCK-LOCAL in each arm, not one shared `pool`: the
    // ROM keeps the counter pointer in r2 across the first `if` while the
    // first arm's 0x0203F990 sits in a short-lived r0. One shared variable
    // ties the two and drags the base into a callee-saved register, which
    // costs r6 here and shifts the counter pointer to r1.
    if ((s32)c <= 0) {
        volatile u8 *p = (volatile u8 *)0x0203F990u;
        *(volatile u16 *)p = *(volatile u16 *)p + 1u;
        *(volatile u32 *)0x030005A4u = 3;
    }
    if (gate == 0) {
        // arg0 materialised into a local so agbcc builds rec+24 into r0
        // FIRST; left inline it evaluates the rec+192 / rec+196 loads ahead
        // of it and runs a single running pointer (`adds r0,#192 / ldr r1 /
        // adds r0,#4 / ldr r2 / subs r0,#172`) the ROM does not have.
        void *p0 = (void *)(uintptr_t)(rec + 24);
        _08007BFC(p0, (int)R32(rec, 192), (int)R32(rec, 196), a, b, 8, 1, 1, 0);
    } else {
        // Nested, not `gate >= 0 && gate <= 3`. Once the first `if` has
        // established gate != 0, agbcc folds the `&&` pair into one unsigned
        // range test and drops the lower compare entirely (216-byte body
        // against the ROM's 220); the ROM keeps `cmp r5,#0 / blt` and
        // `cmp r5,#3 / bgt` as two branches to the same target.
        if (gate >= 0) {
            if (gate <= 3) {
                __asm__(".globl RaceSceneWaD28\nRaceSceneWaD28 = 0x03001780\n");
                __asm__(".globl RaceSceneTblD28\nRaceSceneTblD28 = 0x080CBEB0\n");
                volatile u8 *pool = (volatile u8 *)0x0203F990u;
                // NOTE: ldrsh — signed compare vs 15. RD16 (non-volatile) is
                // what yields a true `movs rX,#0 / ldrsh rY,[r6,rX]`; the
                // volatile S16 helper costs an ldrh + lsls + asrs triple.
                if ((int)RD16(pool, 0) > 15)
                    W16(pool, 0, 0);
                // Hoisted into locals, and the base added LAST: the ROM loads
                // 0x08349754 then 0x080CBEB0 up front, computes 2*gate, then
                // reads WA, and assembles `adds r2,r2,r1 / adds r2,r2,r3`.
                void *tbl = (void *)0x08349754u;
                u32 base = (uintptr_t)RaceSceneTblD28;
                int g2 = 2 * gate;
                // Symbol in a pointer VARIABLE: an inline `WA + 0xFC0` folds
                // into one pool word, the ROM adds 0xFC0 in a register.
                volatile u8 *wa = (volatile u8 *)RaceSceneWaD28;
                int w = (int)RD16(wa, 0xFC0);
                u32 addr = (u32)g2 + (u32)(8 * w) + base;
                int q1 = (int)RD16((volatile u8 *)addr, 0);
                int q2 = (int)RD16(pool, 2);
                int q3 = (int)RD16(pool, 0) << 4;
                _08007570(tbl, q1, q2, q3, 16);
                int r2s = (int)RD16(pool, 2);
                _08002ED0((void *)(u32)a, b, r2s, h5, 1, 2, 1, 0, 0, 1);
            }
        }
    }
}

#ifndef __APPLE__
void sub_080020D28(void *rec0, int a, int b, int gate, int h5) __attribute__((alias("_080020D28")));
#endif

// ---- 0x08020E04 — key/counter handler: lane tick, clamp, rebind + pools {WA, 0x0203F980, 0x08349754, 0x080CBEF0, 0x080CBEB0, 0x080CBF30} ----
// NOTE: b (r1-in) is dead; kept for ABI shape.
void _080020E04(void *rec0, int b, int key) {
    (void)b;
    volatile u8 *rec = (volatile u8 *)rec0;
    volatile u8 *wa = (volatile u8 *)WA;
    int k = (int)(u16)key;
    u16 r8 = R16(rec, 142);
    if (k == 2) {
        _08002B368(4);
        W32(rec, 32, 10);
        W16(rec, 36, 0);
        W32(rec, 64, 10);
        W32(rec, 60, 0);
    }
    if (k == 1) {
        _08002B368(1);
        W32(rec, 32, 10);
        W16(rec, 36, 0);
        W32(rec, 64, 10);
        W32(rec, 60, 0);
        {
            int s138 = (int)S16(rec, 138);
            u32 src = WA + (u32)((s32)s138 * 12) + 48u;
            volatile u8 *dst = rec + 152;
            *(volatile u32 *)(dst + 0) = *(volatile u32 *)src;
            *(volatile u32 *)(dst + 4) = *(volatile u32 *)(src + 4u);
            *(volatile u32 *)(dst + 8) = *(volatile u32 *)(src + 8u);
        }
    }
    if (k == 32)
        W16(rec, 142, (u16)(R16(rec, 142) - 1u));
    if (k == 16)
        W16(rec, 142, (u16)(R16(rec, 142) + 1u));
    if (R16(rec, 142) == 0)
        W16(rec, 142, 0);
    {
        volatile u8 *cell = wa + 0xFC0u;
        int s6 = (int)S16(cell, 0);
        u32 tbl = 0x0203F980u + (u32)(2 * s6);
        int s = (int)S16(rec, 142);
        // NOTE: signed lo, unsigned store word: clamp up to u16[tbl].
        if (s >= (int)*(volatile s16 *)tbl)
            W16(rec, 142, *(volatile u16 *)tbl);
    }
    if (R16(rec, 142) != r8) {
        _08002B368(2);
        int s142 = (int)S16(rec, 142);
        int s6 = (int)S16(wa, 0xFC0);
        int q = (int)*(volatile s16 *)(0x080CBEF0u + (u32)(2 * s142 + 8 * s6));
        _080075E8((void *)0x08349754u, 7, q);
        if (R16(rec, 142) == 0) {
            int q2 = (int)*(volatile s16 *)(0x080CBEB0u + (u32)(8 * s6));
            W32(rec, 196, (u32)q2);
            _08007ABC((void *)R32(rec, 28), (void *)(u32)q2, (int)R32(rec, 192));
        }
    }
    // u8[u32[rec+148]] = (u8)u16[rec+142]
    *(volatile u8 *)R32(rec, 148) = (u8)R16(rec, 142);
    {
        int s142 = (int)S16(rec, 142);
        int s6 = (int)S16(wa, 0xFC0);
        W32(rec, 184, (u32)(s32)*(volatile s16 *)(0x080CBF30u + (u32)(2 * (5 * s6 + s142))));
    }
}

// ---- 0x08020F50 — twin 7B18 emit on +166 (kinds 11/14 x8, 15/16 x64) ----
void _080020F50(void *rec0, int sel) {
    volatile u8 *rec = (volatile u8 *)rec0;
    u16 r5 = (u16)sel;
    if (r5 != 0) {
        int v = (int)RD16(rec, 166);
        switch (v) {
        case 0:
            _08007B18((void *)(uintptr_t)(rec + 68), 11, 8, 72, 3, 1, 1, 0);
            break;
        case 1:
            _08007B18((void *)(uintptr_t)(rec + 68), 14, 8, 72, 3, 1, 1, 0);
            break;
        }
    }
    {
        // The ROM loads the table base 0x0203F980 BEFORE the WA symbol, then
        // materializes 0xFC0 as `movs #252 / lsls #4` and adds it in-line. A
        // pointer VARIABLE holding the symbol keeps agbcc from folding
        // WA+0xFC0 into a single pool word. The symbol is defined in this body
        // so the per-body splice carries it.
        __asm__(".globl RaceSceneWaF50\nRaceSceneWaF50 = 0x03001780\n.globl RaceSceneTblF50\nRaceSceneTblF50 = 0x0203F980\n");
        volatile u8 *tbl = (volatile u8 *)RaceSceneTblF50;
        volatile u8 *wa = (volatile u8 *)RaceSceneWaF50;
        int w = (int)RD16(wa, 0xFC0);
        int t = (int)*(s16 *)(tbl + (u32)(2 * w));
        if ((int)r5 != t) {
            int v = (int)RD16(rec, 166);
            switch (v) {
            case 0:
                _08007B18((void *)(uintptr_t)(rec + 68), 15, 64, 72, 3, 1, 1, 0);
                break;
            case 1:
                _08007B18((void *)(uintptr_t)(rec + 68), 16, 64, 72, 3, 1, 1, 0);
                break;
            }
        }
    }
}

#ifndef __APPLE__
void sub_080020F50(void *rec0, int sel) __attribute__((alias("_080020F50")));
#endif

// agbcc closes a section with the 2-byte Thumb NOP `c0 46`; the 198-byte ROM
// span ends `47 00 00 00` (`bx r0` then two zero bytes), so the 200-byte
// section's trailing pad must be 0 rather than gas's default Thumb nop fill.
__asm__(".align 2, 0");

// ---- 0x08021018 — repeat emitter: kind 6 x (u/2), kind 5 on odd ----
void _080021018(void *rec0, int a, int b, int c) {
    volatile u8 *rec = (volatile u8 *)rec0;
    // u = clamp((s8)c, 0, 32): identity 1..31, 32 above, 0 at/below 0.
    int s8c = (int)(s8)(u8)c;
    int u;
    if (s8c <= 0)
        u = 0;
    else if (s8c <= 31)
        u = s8c;
    else
        u = 32;
    int n = u >> 1;
    for (int k = 0; k < n; k++)
        _08007B18((void *)(uintptr_t)(rec + 8), 6, a + 8 * k, b, 9, 1, 1, 0);
    if ((u & 1) != 0)
        _08007B18((void *)(uintptr_t)(rec + 8), 5, a + 8 * n, b, 9, 1, 1, 0);
}

// ---- 0x080210BC — scene assembly: 7B18/7BFC/D28/F50/1018 chain + pools below ----
// Pools: 0x08349754, WA+0xFC0 gate, 0x080CBEB0, 0x080CBEF0, 0x080CBF30,
//   0x0203F990 not touched here (see 0209A0/020D28).
void _0800210BC(void *rec0) {
    // Every call below passes `rec`, never `rec0`. While both spellings are
    // live agbcc allocates a SECOND callee-saved register for the record
    // (`mov r6,r8 / mov r9,r6` + `push {r6,r7}` instead of `mov r7,r8` +
    // `push {r7}`) and the 396-byte span becomes 400.
    volatile u8 *rec = (volatile u8 *)rec0;
    _08007B18((void *)(uintptr_t)(rec + 68), 8, 112, 64, 6, 3, 1, 0);
    _080026A2C((void *)R32(rec, 204), 160, 32);
    // Symbol in a pointer VARIABLE: an inline `WA + 0xFC0` folds into one
    // pool word, the ROM adds 0xFC0 into a register. RD16 (non-volatile)
    // gives the single ldrsh the ROM has.
    __asm__(".globl RaceSceneWa210BC\nRaceSceneWa210BC = 0x03001780\n");
    volatile u8 *wa = (volatile u8 *)RaceSceneWa210BC;
    // NOTE: single ldrsh in ROM; one C read feeds both compares.
    int fc0 = (int)RD16(wa, 0xFC0);
    // UNRESOLVED: the ROM keeps `cmp r0,#2 / bgt` AND `cmp r0,#0 / blt`, both
    // targeting the SAME block. agbcc folds the `&&` to one unsigned `bhi`;
    // every shape that keeps the second compare either emits the 4-arg call
    // twice (nested ifs, 432 bytes) or lands 8 bytes over (decision local,
    // 404). Left in the size-preserving form; this body is NOT promotable.
    if (fc0 >= 0 && fc0 <= 2)
        _08007B18((void *)(uintptr_t)(rec + 8), 2, 0, 32, 3, 3, 1, 0);
    else
        _08007B18((void *)(uintptr_t)(rec + 8), 4, 0, 32, 3, 3, 1, 0);
    _08007B18((void *)(uintptr_t)(rec + 8), 3, 0, 56, 3, 3, 1, 0);
    _080020D28((void *)(uintptr_t)rec, 24, 56, (int)S16(rec, 142), 7);
    _08007B18((void *)(uintptr_t)rec, (int)R32(rec, 172), 32, 40, 4, 1, 1, 0);
    _08007B18((void *)(uintptr_t)(rec + 16), (int)R32(rec, 184), 8, 96, 5, 1, 1, 0);
    _080020F50((void *)(uintptr_t)rec, (int)R16(rec, 138));
    {
        int r4 = (int)_08025500((int)S16(rec, 138));
        r4 += _08025548((const void *)(uintptr_t)(rec + 152));
        r4 = (int)(s8)r4;
        _080021018((void *)(uintptr_t)rec, 103, 104, r4);
    }
    {
        int r4 = (int)_08025518((int)S16(rec, 138));
        r4 += _080255C4((const void *)(uintptr_t)(rec + 152));
        r4 = (int)(s8)r4;
        _080021018((void *)(uintptr_t)rec, 103, 120, r4);
    }
    {
        int r4 = (int)_08025530((int)S16(rec, 138));
        r4 += _08025640((const void *)(uintptr_t)(rec + 152));
        r4 = (int)(s8)r4;
        _080021018((void *)(uintptr_t)rec, 103, 136, r4);
    }
    _0800D97C((void *)(uintptr_t)(rec + 164), 15);
    _080021248();
    _0800DBE8((void *)(uintptr_t)(rec + 32));
}

// ---- 0x08021260 — FBC-gated frame counter (event 21 past 180) + pools {WA+0xFBC} ----
void _080021260(void *rec_) {
#ifndef __APPLE__
    extern u8 RaceSceneWa21260[] __asm__("RaceSceneWa21260");
    __asm__(".globl RaceSceneWa21260\nRaceSceneWa21260 = 0x03001780\n");
    uintptr_t base = (uintptr_t)RaceSceneWa21260;
#else
    uintptr_t base = (uintptr_t)(0x03001780u);
#endif
    if (*(volatile u16 *)(uintptr_t)(base + 0xFBCu) == 3) {
        if (_08002140() == 2) goto reset;
        u32 v = *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 208);
        v += 1;
        *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 208) = v;
        if ((s32)v > 180) {
            _08004D4C(21, 0, 0);
        }
    }
    return;
reset:
    *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 208) = 0;
}
__asm__(".align 2, 0");

extern const u8 RaceSceneWa12AC[];

// ---- 0x080212AC — 12-way event dispatcher (ev-1 over table 0x080212C8) ----
// agbcc emits the switch arms in SOURCE order, so the arms below are written
// in the ROM's layout order — ev2(ev-1=1) ev5(4) ev7(6) ev6(5) ev1(0) ev12(11)
// — which is what lands them at 0x12F8/0x1302/0x1328/0x1330/0x1346/0x134E, the
// addresses the value-ordered table at 0x080212C8 points at:
// ev1->1346(09A0) ev2->12F8(0958) ev3/4->1354(nop)
// ev5->1302(D854+late-124C) ev6->1330(0E04) ev7->1328(10BC)
// ev8-11->1354(nop) ev12->134E(099C)
void _0800212AC(int ev, int b, int c, void *rec0) {
    // Every arm passes `rec`, never `rec0`. That is not cosmetic: while both
    // spellings of the record are live, agbcc splits them — the plain
    // argument copies read the callee-saved r4 the prologue built, but the
    // offset forms re-derive from the incoming r3 (`add r0,r3,#0` /
    // `ldrh r0,[r3,#36]`) where the ROM reads r4. Routing every arm through
    // the local coalesces the two and r4 wins. (A `register... __asm__("r4")`
    // pin is the obvious lever and is worse: it gives the local an allocno
    // separate from the parameter, so the prologue grows a second copy —
    // `push {r4,r5,r6,lr}` + `add r5,r3,#0; add r4,r5,#0` — and the span
    // stops matching at +0x0.)
    volatile u8 *rec = (volatile u8 *)rec0;
    int idx = ev - 1;
    // The ROM reads WA+0xFBC as `ldr r0,=0x03001780 / ldr r1,=0x00000FBC /
    // adds r0,r0,r1` — a symbol address plus an offset too large for an add
    // immediate, summed in-line. agbcc folds the expression form
    // (`*(u16 *)(Wa + 0xFBC)`) into ONE `Wa+0xFBC` pool word and only emits
    // the two-word add for a pointer VARIABLE holding the symbol, so the read
    // below goes through a local. The symbol is defined in this body: the
    // per-body splice takes brace-matched text only, so a file-scope
    // definition would never reach the section.
    __asm__(".globl RaceSceneWa12AC\nRaceSceneWa12AC = 0x03001780\n");
    if ((u32)idx > 11u)
        return;
    switch (idx) {
    case 1:
        // NOTE: r1-in (b) is the record here; r0-in is dead.
        _080020958((void *)(uintptr_t)rec, (void *)b);
        break;
    case 4:
        _0800D854((void *)(uintptr_t)(rec + 32));
        {
            // Scoped after the call on purpose: a base live across `bl` is
            // allocated a callee-saved register, and the ROM rebuilds the
            // address in r0 between the call and the load.
            volatile u8 *wa = (volatile u8 *)(const u8 *)RaceSceneWa12AC;
            u16 fbc = R16(wa, 0xFBC);
            if (fbc == 3)
                _08002124C((void *)(uintptr_t)rec);
        }
        break;
    case 6:
        _0800210BC((void *)(uintptr_t)rec);
        break;
    case 5:
        if (R16(rec, 36) != 0)
            _080020E04((void *)(uintptr_t)rec, (u16)b, (u16)c);
        break;
    case 0:
        _0800209A0((void *)(uintptr_t)rec);
        break;
    case 11:
        _08002099C((void *)(uintptr_t)rec);
        break;
    default:
        break;
    }
}

// The 0x080212AC span carries a twin label pair in the closure
// (asm/race_scene.s:13630-13631 declares `sub_0800212AC:` immediately followed
// by `_0800212AC:`), so the splice deletes the `sub_` spelling and re-emits only
// a `.thumb_set` agbcc actually wrote. C must therefore define it or the
// independent link fails with an undefined reference. Alias the real body in
// ONE hop. The guard is required: clang rejects `alias` attributes on darwin,
// and no ARM gate sees it -- only tools/apple_decls.py does.
#ifndef __APPLE__
void sub_0800212AC(int ev, int b, int c, void *rec0) __attribute__((alias("_0800212AC")));
#endif

// agbcc closes a section with the 2-byte Thumb NOP `c0 46`; the ROM ends this
// span on two zero bytes. The explicit align makes the fill `0` instead, which
// is the last two bytes of the 176-byte span (174/176 -> EXACT).
__asm__(".align 2, 0");
