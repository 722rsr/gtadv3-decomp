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
//   _080020784 (0x08020784) — scene setup C (03954 path + 7BFC + DBE8), C lift of the 20690 twin
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

#define HOST_STUB(sig) extern sig

HOST_STUB(int   _08002580C(int x));                                  // 0x08002580C table lookup
HOST_STUB(int   _08002581C(int x));                                  // 0x08002581C lineup probe
HOST_STUB(int   _08002584C(int x));                                  // 0x08002584C table lookup
HOST_STUB(int   _08002587C(int x));                                  // 0x08002587C lineup probe

// The nine-digit spellings above are host-only stubs; asm/ai_line_tail.s binds
// the EIGHT-digit labels `_0802581C:` / `_0802587C:`, which src/ai_line_more.c
// exports as aliases of Ai_LineFindCourse / Ai_LineFindIn6010C. The target
// build must therefore call the closure spelling, while the Apple host build
// keeps the stubbed nine-digit names.
#define RSE_CALLEE(friendly, closure) closure
extern int _0802581C(int x);   // 0x08002581C lineup probe
extern int _0802587C(int x);   // 0x08002587C lineup probe
extern void sub_0802B368(int v); // closure spelling for 0x0802B368
// Closure spellings used by the plain-C bodies in this TU (the matching build
// resolves them through the splicer; the host/equivalence build needs the
// prototypes so the bodies type-check without implicit declarations).
extern s16  _0802584C(int x);                                    // 0x0802584C table lookup
extern void sub_08007770(int a, int b, int c, int d, int e, int f); // 0x08007770 6 machine args
extern int  sub_080258A8(int idx);                               // 0x080258A8 numeric_leaves
extern void sub_08007ABC(int a, int b, int c);                   // 0x08007ABC resource bind
HOST_STUB(s16   _0800258A8(int idx));                                // 0x0800258A8 numeric_leaves
HOST_STUB(s16   _0800258B8(int x));                                  // 0x0800258B8 course_cal cup
HOST_STUB(s16   _0800258CC(int x));                                  // 0x0800258CC course_cal seq
extern s16 _080258B8(int x);  // closure spelling of 0x0800258B8
extern s16 _080258CC(int x);  // closure spelling of 0x0800258CC
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
HOST_STUB(void  _080020878(void *a));                                // 0x080020878 bx-lr (race_scene.c)
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
    // Keep the short result in r1 until rec+180 is formed in r0; promotion to
    // s32 then emits the ROM's sign extension immediately before the store.
    register s16 t asm("r1") = _0800258A8((int)RD16(rec, 26));
    register volatile u8 *t_slot asm("r0") = rec + 180;
    register s32 t32 asm("r1") = t;
    *(volatile s32 *)t_slot = t32;
    _08007ABC((void *)R32(rec, 12), (void *)(uintptr_t)(u32)t32, (int)R32(rec, 176));
    // The ROM preloads 1, clears it for lane <= 31, shifts the 0/1 flag by
    // 16, then copies it to r5. The shifted value is only tested for zero, so
    // it preserves the flag while matching the callee-saved guard across the
    // later calls.
    int lane = RD16(rec, 28);
    u16 h = 1;
    if (lane <= 31)
        h = 0;
    register int shifted asm("r0") = h << 16;
    asm volatile("" : "+r"(shifted));
    int hi = shifted;
    if (!hi) {
        int u = (int)(s16)RSE_CALLEE(_0800258B8, _080258B8)((int)RD16(rec, 26));
        // This is the ROM's r1 result / r0 destination sequence. Reusing r1
        // for the s16 promotion also supplies the helper's second argument.
        register s32 w asm("r1") = _08001F9EC(u);
        register volatile u8 *w_slot asm("r0") = rec + 192;
        register s32 w16 asm("r1") = (s16)w;
        *(volatile s32 *)w_slot = w16;
        _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)w16, (int)R32(rec, 188));
    } else {
        // The ROM keeps rec+192 in r1, stores the constant through it, then
        // decrements the same pointer to fetch rec+188 for the helper.
        register volatile u8 *const_slot asm("r1") = rec + 192;
        *(volatile s32 *)const_slot = 13;
        _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)13,
                  (int)*(volatile s32 *)(const_slot - 4));
    }
    if (!hi) {
        int u2 = (int)(s16)RSE_CALLEE(_0800258CC, _080258CC)((int)RD16(rec, 26));
        // Same store-and-call register pattern as the preceding nonconstant
        // branch, with the second lane's +204/+200 fields.
        register s32 w2 asm("r1") = _08001F9FC(u2);
        register volatile u8 *w2_slot asm("r0") = rec + 204;
        register s32 w2_16 asm("r1") = (s16)w2;
        *(volatile s32 *)w2_slot = w2_16;
        _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)(u32)w2_16, (int)R32(rec, 200));
    } else {
        // Reuse rec+204 as the base for the rec+200 helper field.
        register volatile u8 *const_slot2 asm("r1") = rec + 204;
        *(volatile s32 *)const_slot2 = 14;
        _08007ABC((void *)R32(rec, 20), (void *)(uintptr_t)14,
                  (int)*(volatile s32 *)(const_slot2 - 4));
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

void sub_08001FF24(void *rec0) __attribute__((alias("_08001FF24")));

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

void sub_08001FF7C(void *rec0) __attribute__((alias("_08001FF7C")));

struct AsmLift_Struct0 { u8 _pad0[12]; s32 field_12; u8 _pad1[4]; s32 field_20; u8 _pad2[2]; u16 field_26; union { u16 uhalf; s16 shalf; } field_28; s32 field_32; u16 field_36; u8 _pad3[22]; s32 field_60; s32 field_64; u8 _pad4[108]; s32 field_176; s32 field_180; u8 _pad5[4]; s32 field_188; s32 field_192; u8 _pad6[4]; s32 field_200; s32 field_204; };
void _08001FFD4(void *rec0, int a1, int a2) {
    struct AsmLift_Struct0 *a0 = (struct AsmLift_Struct0 *)rec0;
    u16 q2;
    u16 zero;
    u16 v0;
    s32 v1;
    s32 v2;
    s16 v3;
    s16 v4;
    s16 v5;
    s32 v6;
    s32 v7;
    v0 = a1;
    q2 = (u16)a2;
    v1 = a0->field_26;
    if (a0->field_28.shalf <= 31) {
        v6 = 0;
    } else {
        v6 = 1;
    }
    if ((u16)(q2 - 1) <= 1) {
        sub_0802B368(4);
        a0->field_32 = 10;
        a0->field_36 = 0;
        a0->field_64 = 10;
        a0->field_60 = 0;
    }
    if ((32 & v0) != 0 && *(s16 *)((u8 *)a0 + 152 + 0) <= 0) {
        a0->field_28.uhalf = a0->field_28.uhalf - 1;
        *(u16 *)((u8 *)a0 + 152) = 10;
    }
    if ((16 & v0) != 0 && *(s16 *)((u8 *)a0 + 152 + 0) <= 0) {
        a0->field_28.uhalf = a0->field_28.uhalf + 1;
        *(u16 *)((u8 *)a0 + 152) = 10;
    }
    v2 = *(u16 *)((u8 *)a0 + 152) - 1;
    __asm__ volatile("" : "+r"(v2));
    zero = 0;
    *(u16 *)((u8 *)a0 + 152) = v2;
    if (v2 << 16 <= 0) *(u16 *)((u8 *)a0 + 152) = zero;
    if (a0->field_28.shalf > 33) a0->field_28.uhalf = 34;
    if (a0->field_28.shalf < 0) a0->field_28.uhalf = zero;
    if (a0->field_28.shalf <= 31) {
        a0->field_26 = _08002580C(a0->field_28.shalf);
        v7 = 0;
    } else {
        register u16 h1 __asm__("r1");
        s32 f;
        __asm__ volatile("" : "=r"(h1));
        f = a0->field_28.shalf;
        __asm__ volatile("" : : "r"(h1));
        a0->field_26 = _0802584C(f - 32);
        v7 = 1;
    }
    if (v6 != (s16)v7) {
        _080050B4(0, 1);
        if ((s16)v7 == 0) {
            sub_08007770(0, 137643088, 3, 0, 4, 1);
        } else {
            sub_08007770(0, 137643088, 4, 0, 4, 1);
        }
    }
    if (a0->field_26 != v1) {
        sub_0802B368(3);
        v3 = sub_080258A8(((s16 *)a0)[13]);
        a0->field_180 = v3;
        sub_08007ABC(a0->field_12, v3, a0->field_176);
        if (v7 << 16 == 0) {
            v4 = _08001F9EC((s16)_080258B8(((s16 *)a0)[13]));
            a0->field_192 = v4;
            sub_08007ABC(a0->field_20, v4, a0->field_188);
        } else {
            *(s32 *)((u8 *)a0 + 192) = 13;
            sub_08007ABC(a0->field_20, 13, *(s32 *)((u8 *)a0 + 192 - 4));
        }
        if (v7 << 16 == 0) {
            v5 = _08001F9FC((s16)_080258CC(((s16 *)a0)[13]));
            a0->field_204 = v5;
            sub_08007ABC(a0->field_20, v5, a0->field_200);
        } else {
            *(s32 *)((u8 *)a0 + 204) = 14;
            sub_08007ABC(a0->field_20, 14, *(s32 *)((u8 *)a0 + 204 - 4));
        }
    }
}
__asm__(".align 2, 0");


// ---- 0x08020198 — key handler: byte-map integrate + car-record writeback + pools {0x03001D64, 0x03001DA0, 0x080CBC74, 0x080CBE74 x2, WA, 0x62A} ----
void _080020198(void *rec0, int a1, int a2) {
    volatile u8 *rec = (volatile u8 *)rec0;
    int key = (int)(u16)a1;
    int sel = (int)(u16)a2;
    u8 raw[4];
    u8 map[4];
    volatile u8 *area;
    u32 rowOff;
    u32 areaBase;
    if (R16(rec, 160) == 1) {
        register int s28 __asm__("r0");
        register int v __asm__("r2");
        s28 = (int)*(s16 *)(uintptr_t)(rec + 28);
        rowOff = (u32)s28 * 72u;
        v = (int)*(s16 *)(uintptr_t)(rec + 156);
        areaBase = 0x03001D64u + (u32)(v * 3) * 4u;
    } else {
        register int s28 __asm__("r0");
        // Zero-cost liveness pin ("=r" before, "r" after): keeps r3 out of
        // the extendhisi2 scratch set so the constant-index load takes r1.
        register int hold __asm__("r3");
        __asm__ volatile("" : "=r"(hold));
        s28 = (int)*(s16 *)(uintptr_t)(rec + 28);
        __asm__ volatile("" : : "r"(hold));
        rowOff = (u32)s28 * 72u;
        areaBase = 0x03001DA0u;
    }
    rowOff += areaBase;
    area = (volatile u8 *)(uintptr_t)(rowOff + 8u);
    __asm__ volatile("" : "+r"(area));
    {
        u8 *mapPtr = map;
        volatile u16 *tbl = (volatile u16 *)0x080CBC74u;
        {
            register u8 idx0 __asm__("r2") = R8(area, 0);
            register volatile u16 *lookup0 __asm__("r0") = tbl + idx0;
            __asm__ volatile("" : "+r"(lookup0));
            mapPtr[0] = (u8)*lookup0;
        }
        {
            register u8 idx1 __asm__("r3") = R8(area, 1);
            register volatile u16 *lookup1 __asm__("r0") = tbl + idx1;
            __asm__ volatile("" : "+r"(lookup1));
            mapPtr[1] = (u8)*lookup1;
        }
        {
            register u8 idx2 __asm__("r2") = R8(area, 2);
            register volatile u16 *lookup2 __asm__("r0") = tbl + idx2;
            __asm__ volatile("" : "+r"(lookup2));
            mapPtr[2] = (u8)*lookup2;
        }
    }
    _0800D95C((void *)raw, (const void *)(uintptr_t)area, 3);
    // Same liveness pin: the ROM's scratch is r3, so exclude r2.
    register int hold2 __asm__("r2");
    __asm__ volatile("" : "=r"(hold2));
    int c164 = (int)*(s16 *)(uintptr_t)(rec + 164);
    __asm__ volatile("" : : "r"(hold2));
    if (c164 <= 2) {
        if (sel == 1) {
            RSE_CALLEE(_08002B368, sub_0802B368)(1);
            W16I(rec, 164, R16(rec, 164) + 1);
        }
        if (sel == 2) {
            if (*(s16 *)(uintptr_t)(rec + 164) != 0)
                RSE_CALLEE(_08002B368, sub_0802B368)(4);
            W16I(rec, 164, R16(rec, 164) - 1);
        }
    } else {
        if (sel == 1) {
            RSE_CALLEE(_08002B368, sub_0802B368)(1);
            W32(rec, 32, 10);
            W16(rec, 36, 0);
            W32(rec, 64, 10);
            W32(rec, 60, (u32)sel); // r5 (==1 here)
        }
        if (sel == 2) {
            W16I(rec, 164, sel); // ==2
            RSE_CALLEE(_08002B368, sub_0802B368)(4);
        }
    }
    if (*(s16 *)(uintptr_t)(rec + 164) < 0)
        W16(rec, 164, 0);
    if (*(s16 *)(uintptr_t)(rec + 164) > 3)
        W16(rec, 164, 3);
    if (*(s16 *)(uintptr_t)(rec + 164) <= 2) {
        int key64 = key & 64;
        volatile u8 *lane = rec + 152;
        if (key64 != 0 && *(s16 *)(uintptr_t)lane <= 0) {
            RSE_CALLEE(_08002B368, sub_0802B368)(2);
            int idx = (int)*(s16 *)(uintptr_t)(rec + 164);
            map[idx] = (u8)(map[idx] - 1u);
            __asm__ volatile("" : : : "memory");
            u32 v = (u32)map[(int)RD16(rec, 164)];
            if (v == 0)
                map[(int)RD16(rec, 164)] = 53;
            else if (v > 53)
                map[(int)RD16(rec, 164)] = 1;
            __asm__ volatile("" : : : "memory");
            int rawIdx = (int)RD16(rec, 164);
            register u8 *rawEntry __asm__("r2") = raw + rawIdx;
            register u8 *mapEntry __asm__("r1") = map + rawIdx;
            __asm__ volatile("" : "+r"(rawEntry), "+r"(mapEntry) : : "memory");
            rawEntry[0] = *(volatile u8 *)((*(volatile u32 *)0x080CBE74u) + mapEntry[0]);
            W16(lane, 0, 10);
        }
        if ((key & 128) && *(s16 *)(uintptr_t)lane <= 0) {
            RSE_CALLEE(_08002B368, sub_0802B368)(2);
            int idx = (int)*(s16 *)(uintptr_t)(rec + 164);
            map[idx] = (u8)(map[idx] + 1u);
            __asm__ volatile("" : : : "memory");
            u32 v = (u32)map[(int)RD16(rec, 164)];
            if (v == 0)
                map[(int)RD16(rec, 164)] = 53;
            else if (v > 53)
                map[(int)RD16(rec, 164)] = 1;
            __asm__ volatile("" : : : "memory");
            int rawIdx = (int)RD16(rec, 164);
            register u8 *rawEntry __asm__("r2") = raw + rawIdx;
            register u8 *mapEntry __asm__("r1") = map + rawIdx;
            __asm__ volatile("" : "+r"(rawEntry), "+r"(mapEntry) : : "memory");
            rawEntry[0] = *(volatile u8 *)((*(volatile u32 *)0x080CBE74u) + mapEntry[0]);
            W16(lane, 0, 10);
        }
        {
            int t = (int)R16(lane, 0) - 1;
            W16I(lane, 0, t);
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
        register volatile u8 *wa __asm__("r2") = (volatile u8 *)WA;
        __asm__ volatile("" : "+r"(wa));
        register volatile u8 *dst __asm__("r0");
        register int offset __asm__("r3");
        dst = (volatile u8 *)(uintptr_t)(
            (u32)(int)*(s16 *)(uintptr_t)(rec + 28) * 72u + (u32)(uintptr_t)wa);
        __asm__ volatile("" : "+r"(dst));
        {
            register u8 value __asm__("r1") = raw[0];
            __asm__ volatile("" : "+r"(value) : : "memory");
            offset = 197;
            __asm__ volatile("" : "+r"(offset));
            offset <<= 3;
            dst += offset;
            *dst = value;
        }
        dst = (volatile u8 *)(uintptr_t)(
            (u32)(int)*(s16 *)(uintptr_t)(rec + 28) * 72u + (u32)(uintptr_t)wa);
        __asm__ volatile("" : "+r"(dst));
        {
            register u8 value __asm__("r1") = raw[1];
            __asm__ volatile("" : "+r"(value) : : "memory");
            offset++;
            dst += offset;
            *dst = value;
        }
        {
            register int row __asm__("r1");
            row = *(s16 *)(uintptr_t)(rec + 28);
            // Hold the preceding r3 offset through this signed read so agbcc
            // uses r0 for the register offset, matching 0x080203B8.
            __asm__ volatile("" : : "r"(offset));
            dst = (volatile u8 *)(uintptr_t)(
                (u32)row * 72u + (u32)(uintptr_t)wa);
        }
        __asm__ volatile("" : "+r"(dst));
        {
            register u8 value __asm__("r1") = raw[2];
            __asm__ volatile("" : "+r"(value) : : "memory");
            register int finalOffset __asm__("r2") = 0x62A;
            __asm__ volatile("" : "+r"(finalOffset));
            dst += finalOffset;
            *dst = value;
        }
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
// Promoted from instruction splices to plain C (2026-10-16): every splice
// was a register-shaping lever that pins reproduce directly, measured on the
// pinned old_agbcc (corpus_match_probe --c89 --require-exact):
//   * rec/area are pinned to the ROM's r8/r2 and their initializers emit the
//     prologue copies (`mov r8,r0 / adds r2,r3,#0`) in declaration order.
//     `bReg` must stay UNPINNED: it is the body's real global allocno (live
//     across the loop's calls), it takes r7, and that is what sets
//     regs_ever_live[7] -> `push {r4,r5,r6,r7,lr}` plus the r9->r7 / r8->r6
//     high-save staging. Pinning it to r7 instead drops the save
//     (`push {r4,r5,r6,lr}`, 111/128, measured).
//   * `modeReg` is pinned to r0 so the stack-arg read (`ldr r0,[sp,#32]`)
//     lands after the prologue copies at the dispatch, not scheduled early
//     into a scratch (the r7 save above is also what makes the slot #32).
//   * the case-1 setup declares k/currentField/cur/cursor in ROM order:
//     `movs r6,#0`, then `movs r0,#164 / add r0,r8 / mov r9,r0` (the
//     high-reg `add` form falls out of adding through the r8 pin), then the
//     r5/r4 copies.
//   * `*(s16 *)base` is deliberately NOT volatile: the ROM's
//     `mov r1,r9 / movs r2,#0 / ldrsh r0,[r1,r2]` is extendhisi2's
//     zero-scratch index form; the base copy is pinned to r1 (an unpinned
//     base reloads into r0 and rotates the scratch to r1), and a volatile
//     halfword would emit `ldrh;lsls;asrs` instead.
//   * the call block is duplicated in both arms on purpose: the ROM has two
//     copies of `ldrb r2,[r5] / adds r0,r4,#0 / adds r1,r7,#0 / movs r3,#4 /
//     bl`, so a shared tail would drop one.
void _080020498(void *rec0, int a, int b, void *area0, int mode) {
    register volatile u8 *rec __asm__("r8") = rec0;
    int bReg = b;
    register volatile u8 *area __asm__("r2") = area0;
    register int modeReg __asm__("r0") = mode;
    switch (modeReg) {
    case 0:
        _08001FADC(a, bReg, (void *)(uintptr_t)area, 3, 4);
        break;
    case 1: {
        register int k __asm__("r6") = 0;
        register volatile u8 *currentField __asm__("r9") = rec + 164;
        register volatile u8 *cur __asm__("r5") = area;
        register int cursor __asm__("r4") = a;
        do {
            register volatile u8 *base __asm__("r1") = currentField;
            register s32 current __asm__("r0") = *(s16 *)base;
            if (current == k) {
                if (R16(rec, 174) == 1)
                    _08001FB58(cursor, bReg, (int)*cur, 4);
            } else {
                _08001FB58(cursor, bReg, (int)*cur, 4);
            }
            cur++;
            cursor += 8;
            k++;
        } while (k <= 2);
        break;
    }
    }
}
__asm__(".align 2, 0");

// ---- 0x08020518 — scene setup A (lane select on idx<=31, 3x 7BFC + DBE8) ----
// Twin of 0x08020690/0x08020784 (same lane call, transform block and
// _080020498 tail) with two differences: the lane test is `idx > 31` (not the
// mode select) and the tail emits THREE 7BFC lanes before DBE8. Load-bearing
// spellings, each measured on the pinned compiler:
//  * `rec` is UNPINNED so the allocator lands it in r7: the ROM reads it with
//    the indexed form `movs rN,#28 / ldrsh rX,[r7,rN]`, which a pinned r7 never
//    takes (docs/findings/indexed_halfword_scratch.md).
//  * `rowIndex` and `row` are shared by both arms (one pseudo across the
//    if/else); with block-local copies the else arm gives the loaded value r0
//    and the row r2 instead of the ROM's r1/r0. The `"+r"` on rowIndex also
//    stops local-alloc coalescing cse's copy into the test's register, which
//    is what keeps arm 1's `adds r1,r0,#0`.
//  * the else arm must not reuse the test's loaded value (the ROM reloads
//    `[rec+28]` after the branch), hence the memory clobber there; its second
//    `index` read wants scratch r3 where agbcc takes the freed r0, hence the
//    r0/r2 liveness bracket (docs/findings/indexed_halfword_scratch.md).
void _080020518(void *rec0) {
    extern u8 RaceSceneE_Wa_20518[];
    extern u8 RaceSceneE_Offset_20518[];
    volatile u8 *rec = (volatile u8 *)rec0;
    register uintptr_t wa __asm__("r4");
    uintptr_t table;
    uintptr_t row;
    int rowIndex;
    int zero;

    __asm__(".globl RaceSceneE_Wa_20518\nRaceSceneE_Wa_20518 = 0x03001780");
    __asm__(".globl RaceSceneE_Offset_20518\nRaceSceneE_Offset_20518 = 0x000005E4");

    _0800D97C((void *)((volatile u8 *)rec0 + 172), 15);
    if (RD16(rec, 28) <= 31) {
        zero = 0;
        wa = (uintptr_t)RaceSceneE_Wa_20518;
        {
            rowIndex = RD16(rec, 28);
            __asm__ volatile ("" : "+r" (rowIndex));
            row = (uintptr_t)(rowIndex * 72);
            register uintptr_t offset __asm__("r1") = (uintptr_t)RaceSceneE_Offset_20518;
            __asm__ volatile ("" : "+r" (offset) : : "memory");
            table = wa + offset;
            __asm__ volatile ("" : "+r" (table));
            row += table;
            _08003B9C(56, 74, *(volatile u32 *)row);
        }
        {
            int index = RD16(rec, 28);
            s16 value = *(s16 *)(uintptr_t)(wa + (uintptr_t)(index * 72) + 1512u);
            int transformed = _08002370(value);
            _08001FA64(128, 78, transformed, 3);
        }
        {
            int index = RD16(rec, 28);
            uintptr_t area = (uintptr_t)(index * 72) + table + 8u;
            _080020498((void *)rec, 184, 78, (void *)area, zero);
        }
    } else {
        __asm__ volatile ("" : : : "memory");
        zero = 0;
        wa = (uintptr_t)RaceSceneE_Wa_20518;
        {
            rowIndex = RD16(rec, 28);
            __asm__ volatile ("" : "+r" (rowIndex));
            row = (uintptr_t)(rowIndex * 72);
            register uintptr_t offset __asm__("r1") = (uintptr_t)RaceSceneE_Offset_20518;
            __asm__ volatile ("" : "+r" (offset) : : "memory");
            table = wa + offset;
            __asm__ volatile ("" : "+r" (table));
            row += table;
            _08003954(120, 74, *(volatile u32 *)row);
        }
        {
            register int hold0 __asm__("r0");
            register int hold2 __asm__("r2");
            int index;
            s16 value;
            int transformed;
            __asm__ volatile ("" : "=r" (hold0), "=r" (hold2));
            index = RD16(rec, 28);
            __asm__ volatile ("" : : "r" (hold0), "r" (hold2));
            value = *(s16 *)(uintptr_t)(wa + (uintptr_t)(index * 72) + 1512u);
            transformed = _08002370(value);
            _08001FA64(128, 78, transformed, 3);
        }
        {
            int index = RD16(rec, 28);
            uintptr_t area = (uintptr_t)(index * 72) + table + 8u;
            _080020498((void *)rec, 184, 78, (void *)area, zero);
        }
    }

    _0800203E8((void *)rec);
    {
        volatile u8 *p8 = rec + 8;
        u32 v1 = R32(rec, 176);
        u32 v2 = R32(rec, 180);
        _08007BFC((void *)p8, v1, v2, 24, 33, 6, 1, 0, 0);
    }
    {
        volatile u8 *p16 = rec + 16;
        u32 v3 = R32(rec, 188);
        u32 v4 = R32(rec, 192);
        _08007BFC((void *)p16, v3, v4, 136, 33, 6, 1, 0, 0);
        v3 = R32(rec, 200);
        v4 = R32(rec, 204);
        _08007BFC((void *)p16, v3, v4, 184, 33, 6, 1, 0, 0);
    }
    _0800DBE8((void *)(rec + 32));
    _080020878((void *)rec);
}

// ---- 0x08020690 — scene setup B (03B9C path, mode-1 area select) + pools {WA+0x5E4, 0x03001D64} ----
// EXACT 244/244. The zero word is an unpinned temp whose liveness is extended
// past the tail calls by the closing "+r" barrier: that makes it a global
// allocno, which global-alloc places in r7, and marked-home r7 restores the
// ROM's push {r4,r5,r6,r7,lr}. table wins r6 on the allocno priority. The
// else arm materialises 78 as movs/lsls/adds in r2 (see the note there), and
// the trailing bx-lr stub takes the record in r0 like every ROM call site.
void _080020690(void *rec0) {
    register volatile u8 *rec __asm__("r5");
    extern u8 RaceSceneE_Wa_20690[];
    extern u8 RaceSceneE_Offset_20690[];
    extern u8 RaceSceneE_AltWa_20690[];
    __asm__(".globl RaceSceneE_Wa_20690\nRaceSceneE_Wa_20690 = 0x03001780");
    __asm__(".globl RaceSceneE_Offset_20690\nRaceSceneE_Offset_20690 = 0x000005E4");
    __asm__(".globl RaceSceneE_AltWa_20690\nRaceSceneE_AltWa_20690 = 0x03001D64");

    register uintptr_t wa __asm__("r4");
    int zero;
    uintptr_t table;

    rec = (volatile u8 *)rec0;
    _0800D97C((void *)((volatile u8 *)rec0 + 172), 5);
    zero = 0;
    wa = (uintptr_t)RaceSceneE_Wa_20690;
    {
        int rowIndex = RD16(rec, 28);
        uintptr_t row = (uintptr_t)(rowIndex * 72);
        register uintptr_t offset __asm__("r1") = (uintptr_t)RaceSceneE_Offset_20690;
        __asm__ volatile ("" : "+r" (offset) : : "memory");
        table = wa + offset;
        __asm__ volatile ("" : "+r" (table));
        row += table;
        _08003B9C(56, 74, *(volatile u32 *)row);
    }

    {
        int index = RD16(rec, 28);
        s16 value = *(s16 *)(uintptr_t)(wa + (uintptr_t)(index * 72) + 1512u);
        int transformed = _08002370(value);
        _08001FA64(128, 78, transformed, 3);
    }

    if (zero == RD16(rec, 156) && RD16(rec, 160) == 1) {
        int index = RD16(rec, 28);
        uintptr_t area = (uintptr_t)(index * 72) + table + 8u;
        _080020498((void *)rec, 184, 78, (void *)area, 1);
    } else {
        /* The ROM synthesizes 78 as (2 << 4) + 46 in r2 rather than
         * loading the immediate directly; the barriers keep agbcc from
         * folding the constant arithmetic into a single movs. */
        register u32 idParts __asm__("r2");
        idParts = 2u;
        __asm__ volatile ("" : "+r" (idParts));
        idParts <<= 4;
        __asm__ volatile ("" : "+r" (idParts));
        idParts += 46u;
        int index = RD16(rec, 28);
        uintptr_t area = (uintptr_t)(index * 72) + (uintptr_t)RaceSceneE_AltWa_20690;
        area += 8;
        _080020498((void *)rec, 184, (int)idParts, (void *)area, 0);
    }

    _08007BFC((void *)(rec + 8),
              *(volatile u32 *)(rec + 176), *(volatile u32 *)(rec + 180),
              24, 33, 6, 1, 0, 0);
    _0800DBE8((void *)(rec + 32));
    _080020878((void *)rec);
    __asm__ volatile ("" : "+r" (zero));
}

// ---- 0x08020784 — scene setup C (03954 path, mode-1 area select) + pools {WA+0x5E4, 0x03001D64} ----
// Same body as 0x08020690 with the text lane swapped (0x08003954, id 120); every
// register-shaping note there applies verbatim.
void _080020784(void *rec0) {
    register volatile u8 *rec __asm__("r5");
    extern u8 RaceSceneE_Wa_20784[];
    extern u8 RaceSceneE_Offset_20784[];
    extern u8 RaceSceneE_AltWa_20784[];
    __asm__(".globl RaceSceneE_Wa_20784\nRaceSceneE_Wa_20784 = 0x03001780");
    __asm__(".globl RaceSceneE_Offset_20784\nRaceSceneE_Offset_20784 = 0x000005E4");
    __asm__(".globl RaceSceneE_AltWa_20784\nRaceSceneE_AltWa_20784 = 0x03001D64");

    register uintptr_t wa __asm__("r4");
    int zero;
    uintptr_t table;

    rec = (volatile u8 *)rec0;
    _0800D97C((void *)((volatile u8 *)rec0 + 172), 5);
    zero = 0;
    wa = (uintptr_t)RaceSceneE_Wa_20784;
    {
        int rowIndex = RD16(rec, 28);
        uintptr_t row = (uintptr_t)(rowIndex * 72);
        register uintptr_t offset __asm__("r1") = (uintptr_t)RaceSceneE_Offset_20784;
        __asm__ volatile ("" : "+r" (offset) : : "memory");
        table = wa + offset;
        __asm__ volatile ("" : "+r" (table));
        row += table;
        _08003954(120, 74, *(volatile u32 *)row);
    }

    {
        int index = RD16(rec, 28);
        s16 value = *(s16 *)(uintptr_t)(wa + (uintptr_t)(index * 72) + 1512u);
        int transformed = _08002370(value);
        _08001FA64(128, 78, transformed, 3);
    }

    if (zero == RD16(rec, 156) && RD16(rec, 160) == 1) {
        int index = RD16(rec, 28);
        uintptr_t area = (uintptr_t)(index * 72) + table + 8u;
        _080020498((void *)rec, 184, 78, (void *)area, 1);
    } else {
        register u32 idParts __asm__("r2");
        idParts = 2u;
        __asm__ volatile ("" : "+r" (idParts));
        idParts <<= 4;
        __asm__ volatile ("" : "+r" (idParts));
        idParts += 46u;
        int index = RD16(rec, 28);
        uintptr_t area = (uintptr_t)(index * 72) + (uintptr_t)RaceSceneE_AltWa_20784;
        area += 8;
        _080020498((void *)rec, 184, (int)idParts, (void *)area, 0);
    }

    _08007BFC((void *)(rec + 8),
              *(volatile u32 *)(rec + 176), *(volatile u32 *)(rec + 180),
              24, 33, 6, 1, 0, 0);
    _0800DBE8((void *)(rec + 32));
    _080020878((void *)rec);
    __asm__ volatile ("" : "+r" (zero));
}

// ---- 0x0802087C — 12-way event dispatcher (ev-1 over table 0x08020894) ----
void _08002087C(int ev, int b, int c, void *rec_) {
    int idx = ev - 1;
    if ((u32)idx > 11u)
        return;

    // agbcc emits case bodies in source order. Keep the ROM's table targets:
    // 1, 4, 6, 5, 0, 11; the other six slots go to the common exit.
    switch (idx) {
    case 1:
        _08001FBAC((int)(uintptr_t)rec_, (void *)(uintptr_t)b);
        break;
    case 4:
        _0800D854((void *)((volatile u8 *)rec_ + 32));
        break;
    case 6: {
        s16 state = *(s16 *)((volatile u8 *)rec_ + 24);
        switch (state) {
        case 0: _080020518(rec_); break;
        case 2: _080020690(rec_); break;
        case 3: _080020784(rec_); break;
        }
        break;
    }
    case 5: {
        register s32 state __asm__("r0");
        if (*(volatile u16 *)((volatile u8 *)rec_ + 36) == 0)
            break;
        state = *(s16 *)((volatile u8 *)rec_ + 24);
        if (state == 0) {
            _08001FFD4(rec_, (u16)b, (u16)c);
        } else {
            // `state < 0` keeps `cmp #0; blt` (fold-const only rewrites
            // comparisons with a positive constant), matching 0x080208AA.
            if (state < 0)
                break;
            // The final gate is the {2,3} switch's leaf range tests:
            // emit_case_nodes emits `GT high -> default` then
            // `LT low -> default` for a childless range node, giving the
            // ROM's `cmp #3; bgt` / `cmp #2; blt` pair (0x080208AE/B2).
            // An if-chain cannot: `state < 2` folds into `cmp #1; ble`.
            switch (state) {
            case 2:
            case 3:
                _080020198(rec_, (u16)b, (u16)c);
                break;
            }
        }
        break;
    }
    case 0:
        _08001FC14(rec_);
        break;
    case 11:
        _08001FBD8(rec_);
        break;
    default:
        break;
    }
}
__asm__(".align 2, 0");

void _080020958(void *unused, void *rec_) {
    (void)unused;
    extern u8 RaceSceneE_Wa_20958[] __asm__("RaceSceneE_Wa_20958");
    __asm__(".globl RaceSceneE_Wa_20958\nRaceSceneE_Wa_20958 = 0x03001780\n");
    uintptr_t base = (uintptr_t)RaceSceneE_Wa_20958;
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
void sub_080020958(void *unused, void *rec_) __attribute__((alias("_080020958")));
void sub_08020958(void *unused, void *rec_) __attribute__((alias("_080020958")));


// ---- 0x080209A0 — big record setup: templates, car walk, 8-way table 0x08020B70 + pools below ----
// Pools: 0x1393, WA+0xFBC, 0x08346910, 0x082A798C, 0x08342B60, 0x08348B50,
//   WA+0x574 / WA+0xFC2 select, 0x080CBEA0, table 0x08020B70, 0x080CBF30,
//   0x080CBEB0, 0x0203F990, 0x08349754, 0x080CBEF0, 0x0203F980.
__attribute__((naked)) void _0800209A0(void *rec0) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, sl\n"
        "\tmov\tr6, r9\n"
        "\tmov\tr5, r8\n"
        "\tpush\t{r5, r6, r7}\n"
        "\tsub\tsp, #16\n"
        "\tadds\tr7, r0, #0\n"
        "\tmovs\tr0, #49\n"
        "\tbl 0x0802B214\n"
        "\tldr r0, .Lrace_0800209a0_0800209D4\n"
        "\tldr r1, .Lrace_0800209a0_0800209D8\n"
        "\tadds\tr4, r0, r1\n"
        "\tldrh\tr2, [r4, #0]\n"
        "\tcmp\tr2, #3\n"
        "\tbne .Lrace_0800209a0_0800209C6\n"
        "\tldr r0, .Lrace_0800209a0_0800209DC\n"
        "\tbl 0x08002124\n"
        ".Lrace_0800209a0_0800209C6:\n"
        "\tldrh\tr4, [r4, #0]\n"
        "\tcmp\tr4, #7\n"
        "\tbne .Lrace_0800209a0_0800209E0\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #144\n"
        "\tmovs\tr0, #1\n"
        "\tb .Lrace_0800209a0_0800209E6\n"
        ".Lrace_0800209a0_0800209D4: .word 0x03001780\n"
        ".Lrace_0800209a0_0800209D8: .word 0x00000FBC\n"
        ".Lrace_0800209a0_0800209DC: .word 0x00001393\n"
        ".Lrace_0800209a0_0800209E0:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #144\n"
        "\tmovs\tr0, #0\n"
        ".Lrace_0800209a0_0800209E6:\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmov\tsl, r1\n"
        "\tldr r5, .Lrace_0800209a0_080020AC0\n"
        "\tmovs\tr3, #4\n"
        "\tmov\tr9, r3\n"
        "\tstr\tr3, [sp, #0]\n"
        "\tmovs\tr6, #1\n"
        "\tstr\tr6, [sp, #4]\n"
        "\tmovs\tr0, #0\n"
        "\tadds\tr1, r5, #0\n"
        "\tmovs\tr2, #1\n"
        "\tmovs\tr3, #0\n"
        "\tbl sub_08007770\n"
        "\tmovs\tr4, #6\n"
        "\tmov\tr8, r4\n"
        "\tstr\tr4, [r7, #104]\n"
        "\tstr\tr4, [r7, #116]\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #128\n"
        "\tstr\tr6, [r0, #0]\n"
        "\tsubs\tr0, #96\n"
        "\tbl sub_0800DAB8\n"
        "\tldr r4, .Lrace_0800209a0_080020AC4\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #2\n"
        "\tmovs\tr2, #6\n"
        "\tbl sub_080075E8\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #9\n"
        "\tbl sub_080075E8\n"
        "\tldr r4, .Lrace_0800209a0_080020AC8\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r7, #0\n"
        "\tbl sub_0800798C\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #4\n"
        "\tbl sub_080075E8\n"
        "\tadds\tr0, r7, #0\n"
        "\tbl sub_08007A58\n"
        "\tadds\tr4, r7, #0\n"
        "\tadds\tr4, #8\n"
        "\tadds\tr0, r5, #0\n"
        "\tadds\tr1, r4, #0\n"
        "\tbl sub_0800798C\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_08007A58\n"
        "\tadds\tr0, r5, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #3\n"
        "\tbl sub_080075E8\n"
        "\tldr r5, .Lrace_0800209a0_080020ACC\n"
        "\tadds\tr4, #8\n"
        "\tadds\tr0, r5, #0\n"
        "\tadds\tr1, r4, #0\n"
        "\tbl sub_0800798C\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_08007A58\n"
        "\tadds\tr0, r5, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #5\n"
        "\tbl sub_080075E8\n"
        "\tadds\tr4, #168\n"
        "\tmov\tr5, r9\n"
        "\tstr\tr5, [r4, #0]\n"
        "\tmovs\tr0, #0\n"
        "\tstr\tr0, [r7, #32]\n"
        "\tstrh\tr6, [r7, #36]\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #52\n"
        "\tmovs\tr2, #32\n"
        "\tnegs\tr2, r2\n"
        "\tmovs\tr1, #0\n"
        "\tbl sub_0800D77C\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #44\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #160\n"
        "\tbl sub_0800D77C\n"
        "\tmov\tr6, r8\n"
        "\tstrh\tr6, [r7, #40]\n"
        "\tmovs\tr0, #5\n"
        "\tstrh\tr0, [r7, #42]\n"
        "\tmov\tr9, r4\n"
        "\tmov\tr0, sl\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tcmp\tr0, #1\n"
        "\tbne .Lrace_0800209a0_080020AD8\n"
        "\tldr r0, .Lrace_0800209a0_080020AD0\n"
        "\tldr r1, .Lrace_0800209a0_080020AD4\n"
        "\tadds\tr0, r0, r1\n"
        "\tb .Lrace_0800209a0_080020ADE\n"
        "\tmovs\tr0, r0\n"
        ".Lrace_0800209a0_080020AC0: .word 0x08346910\n"
        ".Lrace_0800209a0_080020AC4: .word 0x082A798C\n"
        ".Lrace_0800209a0_080020AC8: .word 0x08342B60\n"
        ".Lrace_0800209a0_080020ACC: .word 0x08348B50\n"
        ".Lrace_0800209a0_080020AD0: .word 0x03001780\n"
        ".Lrace_0800209a0_080020AD4: .word 0x00000FC2\n"
        ".Lrace_0800209a0_080020AD8:\n"
        "\tldr r0, .Lrace_0800209a0_080020B60\n"
        "\tldr r2, .Lrace_0800209a0_080020B64\n"
        "\tadds\tr0, r0, r2\n"
        ".Lrace_0800209a0_080020ADE:\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #138\n"
        "\tstrh\tr1, [r0, #0]\n"
        "\tstr\tr0, [sp, #12]\n"
        "\tldr\tr3, [sp, #12]\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r3, r4]\n"
        "\tbl 0x080022E4\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #140\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tldr r2, .Lrace_0800209a0_080020B60\n"
        "\tldr\tr5, [sp, #12]\n"
        "\tmovs\tr6, #0\n"
        "\tldrsh\tr1, [r5, r6]\n"
        "\tlsls\tr0, r1, #1\n"
        "\tadds\tr0, r0, r1\n"
        "\tlsls\tr0, r0, #2\n"
        "\tadds\tr0, r0, r2\n"
        "\tadds\tr0, #49\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tlsls\tr0, r0, #24\n"
        "\tasrs\tr0, r0, #24\n"
        "\tadds\tr5, r7, #0\n"
        "\tadds\tr5, #136\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tadds\tr3, r7, #0\n"
        "\tadds\tr3, #172\n"
        "\tldr r1, .Lrace_0800209a0_080020B68\n"
        "\tmovs\tr0, #252\n"
        "\tlsls\tr0, r0, #4\n"
        "\tadds\tr4, r2, r0\n"
        "\tmovs\tr6, #0\n"
        "\tldrsh\tr0, [r4, r6]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r0, r1\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r0, r1]\n"
        "\tstr\tr0, [r3, #0]\n"
        "\tldr\tr3, [sp, #12]\n"
        "\tmovs\tr6, #0\n"
        "\tldrsh\tr1, [r3, r6]\n"
        "\tlsls\tr0, r1, #1\n"
        "\tadds\tr0, r0, r1\n"
        "\tlsls\tr0, r0, #2\n"
        "\tadds\tr0, r0, r2\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #152\n"
        "\tadds\tr0, #48\n"
        "\tldmia\tr0!, {r2, r3, r6}\n"
        "\tstmia\tr1!, {r2, r3, r6}\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tstr\tr5, [sp, #8]\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #148\n"
        "\tcmp\tr0, #7\n"
        "\tbhi .Lrace_0800209a0_080020BE0\n"
        "\tlsls\tr0, r0, #2\n"
        "\tldr r1, .Lrace_0800209a0_080020B6C\n"
        "\tadds\tr0, r0, r1\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tmov\tpc, r0\n"
        ".Lrace_0800209a0_080020B60: .word 0x03001780\n"
        ".Lrace_0800209a0_080020B64: .word 0x00000574\n"
        ".Lrace_0800209a0_080020B68: .word 0x080CBEA0\n"
        ".Lrace_0800209a0_080020B6C: .word 0x08020B70\n"
        ".Lrace_0800209a0_080020B70: .word 0x08020B90\n"
        ".Lrace_0800209a0_080020B74: .word 0x08020B9A\n"
        ".Lrace_0800209a0_080020B78: .word 0x08020BA4\n"
        ".Lrace_0800209a0_080020B7C: .word 0x08020BAE\n"
        ".Lrace_0800209a0_080020B80: .word 0x08020BB8\n"
        ".Lrace_0800209a0_080020B84: .word 0x08020BC2\n"
        ".Lrace_0800209a0_080020B88: .word 0x08020BCC\n"
        ".Lrace_0800209a0_080020B8C: .word 0x08020BD6\n"
        ".Lrace_0800209a0_080020B90:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #148\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #154\n"
        "\tb .Lrace_0800209a0_080020BDE\n"
        ".Lrace_0800209a0_080020B9A:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #148\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #158\n"
        "\tb .Lrace_0800209a0_080020BDE\n"
        ".Lrace_0800209a0_080020BA4:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #148\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #156\n"
        "\tb .Lrace_0800209a0_080020BDE\n"
        ".Lrace_0800209a0_080020BAE:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #148\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #159\n"
        "\tb .Lrace_0800209a0_080020BDE\n"
        ".Lrace_0800209a0_080020BB8:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #148\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #157\n"
        "\tb .Lrace_0800209a0_080020BDE\n"
        ".Lrace_0800209a0_080020BC2:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #148\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #161\n"
        "\tb .Lrace_0800209a0_080020BDE\n"
        ".Lrace_0800209a0_080020BCC:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #148\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #155\n"
        "\tb .Lrace_0800209a0_080020BDE\n"
        ".Lrace_0800209a0_080020BD6:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #148\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #160\n"
        ".Lrace_0800209a0_080020BDE:\n"
        "\tstr\tr0, [r1, #0]\n"
        ".Lrace_0800209a0_080020BE0:\n"
        "\tldr\tr0, [r1, #0]\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tlsls\tr0, r0, #24\n"
        "\tasrs\tr0, r0, #24\n"
        "\tmovs\tr2, #142\n"
        "\tadds\tr2, r2, r7\n"
        "\tmov\tr8, r2\n"
        "\tmovs\tr4, #0\n"
        "\tstrh\tr0, [r2, #0]\n"
        "\tldr r3, .Lrace_0800209a0_080020C74\n"
        "\tmovs\tr5, #0\n"
        "\tldrsh\tr2, [r2, r5]\n"
        "\tldr r0, .Lrace_0800209a0_080020C78\n"
        "\tmovs\tr6, #252\n"
        "\tlsls\tr6, r6, #4\n"
        "\tadds\tr6, r6, r0\n"
        "\tmov\tsl, r6\n"
        "\tmovs\tr0, #0\n"
        "\tldrsh\tr1, [r6, r0]\n"
        "\tlsls\tr0, r1, #2\n"
        "\tadds\tr0, r0, r1\n"
        "\tadds\tr0, r0, r2\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r0, r3\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r0, r1]\n"
        "\tmov\tr2, r9\n"
        "\tstr\tr0, [r2, #0]\n"
        "\tldr r3, .Lrace_0800209a0_080020C7C\n"
        "\tmov\tr9, r3\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #24\n"
        "\tmov\tr0, r9\n"
        "\tbl sub_0800798C\n"
        "\tmovs\tr0, #16\n"
        "\tbl sub_08005758\n"
        "\tldr r5, .Lrace_0800209a0_080020C80\n"
        "\tstrh\tr0, [r5, #2]\n"
        "\tstrh\tr4, [r5, #0]\n"
        "\tmovs\tr0, #16\n"
        "\tbl sub_0800572C\n"
        "\tadds\tr6, r7, #0\n"
        "\tadds\tr6, #192\n"
        "\tstr\tr0, [r6, #0]\n"
        "\tadds\tr4, r7, #0\n"
        "\tadds\tr4, #196\n"
        "\tmov\tr1, sl\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tlsls\tr0, r0, #3\n"
        "\tldr r3, .Lrace_0800209a0_080020C84\n"
        "\tadds\tr0, r0, r3\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r0, r1]\n"
        "\tstr\tr0, [r4, #0]\n"
        "\tmov\tr0, r9\n"
        "\tmovs\tr1, #28\n"
        "\tmovs\tr2, #8\n"
        "\tbl sub_080075E8\n"
        "\tmov\tr2, r8\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r2, r3]\n"
        "\tcmp\tr1, #0\n"
        "\tbne .Lrace_0800209a0_080020C88\n"
        "\tldr\tr0, [r7, #28]\n"
        "\tldr\tr1, [r4, #0]\n"
        "\tldr\tr2, [r6, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tb .Lrace_0800209a0_080020CD6\n"
        ".Lrace_0800209a0_080020C74: .word 0x080CBF30\n"
        ".Lrace_0800209a0_080020C78: .word 0x03001780\n"
        ".Lrace_0800209a0_080020C7C: .word 0x08349754\n"
        ".Lrace_0800209a0_080020C80: .word 0x0203F990\n"
        ".Lrace_0800209a0_080020C84: .word 0x080CBEB0\n"
        ".Lrace_0800209a0_080020C88:\n"
        "\tcmp\tr1, #0\n"
        "\tblt .Lrace_0800209a0_080020CD6\n"
        "\tcmp\tr1, #3\n"
        "\tbgt .Lrace_0800209a0_080020CD6\n"
        "\tlsls\tr1, r1, #1\n"
        "\tmov\tr4, sl\n"
        "\tmovs\tr6, #0\n"
        "\tldrsh\tr0, [r4, r6]\n"
        "\tlsls\tr0, r0, #3\n"
        "\tadds\tr1, r1, r0\n"
        "\tldr r0, .Lrace_0800209a0_080020D18\n"
        "\tadds\tr1, r1, r0\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r1, r2]\n"
        "\tmovs\tr3, #2\n"
        "\tldrsh\tr2, [r5, r3]\n"
        "\tmovs\tr0, #16\n"
        "\tstr\tr0, [sp, #0]\n"
        "\tmov\tr0, r9\n"
        "\tmovs\tr3, #16\n"
        "\tbl sub_08007570\n"
        "\tldr r2, .Lrace_0800209a0_080020D1C\n"
        "\tmov\tr4, r8\n"
        "\tmovs\tr5, #0\n"
        "\tldrsh\tr0, [r4, r5]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tmov\tr6, sl\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r6, r3]\n"
        "\tlsls\tr1, r1, #3\n"
        "\tadds\tr0, r0, r1\n"
        "\tadds\tr0, r0, r2\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr1, [r0, r4]\n"
        "\tmov\tr0, r9\n"
        "\tmovs\tr2, #7\n"
        "\tbl sub_080075E8\n"
        ".Lrace_0800209a0_080020CD6:\n"
        "\tldr r4, .Lrace_0800209a0_080020D20\n"
        "\tmovs\tr5, #252\n"
        "\tlsls\tr5, r5, #4\n"
        "\tadds\tr4, r4, r5\n"
        "\tmovs\tr6, #0\n"
        "\tldrsh\tr0, [r4, r6]\n"
        "\tbl 0x08025F20\n"
        "\tldr r2, .Lrace_0800209a0_080020D24\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r4, r3]\n"
        "\tlsls\tr1, r1, #1\n"
        "\tadds\tr1, r1, r2\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #204\n"
        "\tldr\tr4, [sp, #12]\n"
        "\tmovs\tr5, #0\n"
        "\tldrsh\tr1, [r4, r5]\n"
        "\tldr\tr6, [sp, #8]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr2, [r6, r3]\n"
        "\tbl sub_0800D9A4\n"
        "\tadd\tsp, #16\n"
        "\tpop\t{r3, r4, r5}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tmov\tsl, r5\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".Lrace_0800209a0_080020D18: .word 0x080CBEB0\n"
        ".Lrace_0800209a0_080020D1C: .word 0x080CBEF0\n"
        ".Lrace_0800209a0_080020D20: .word 0x03001780\n"
        ".Lrace_0800209a0_080020D24: .word 0x0203F980\n"
        ".syntax divided\n"
    );
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

void sub_080020D28(void *rec0, int a, int b, int gate, int h5) __attribute__((alias("_080020D28")));

// ---- 0x08020E04 — key/counter handler: lane tick, clamp, rebind + pools {WA, 0x0203F980, 0x08349754, 0x080CBEF0, 0x080CBEB0, 0x080CBF30} ----
// NOTE: b (r1-in) is dead; kept for ABI shape.
__attribute__((naked)) void _080020E04(void *rec0, int b, int key) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, r9\n"
        "\tmov\tr6, r8\n"
        "\tpush\t{r6, r7}\n"
        "\tadds\tr4, r0, #0\n"
        "\tlsls\tr2, r2, #16\n"
        "\tlsrs\tr6, r2, #16\n"
        "\tmov\tr9, r6\n"
        "\tadds\tr5, r4, #0\n"
        "\tadds\tr5, #142\n"
        "\tldrh\tr0, [r5, #0]\n"
        "\tmov\tr8, r0\n"
        "\tcmp\tr6, #2\n"
        "\tbne .Lrace_080020e04_080020E32\n"
        "\tmovs\tr0, #4\n"
        "\tbl 0x0802B368\n"
        "\tmovs\tr1, #10\n"
        "\tstr\tr1, [r4, #32]\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r4, #36]\n"
        "\tstr\tr1, [r4, #64]\n"
        "\tstr\tr0, [r4, #60]\n"
        ".Lrace_080020e04_080020E32:\n"
        "\tcmp\tr6, #1\n"
        "\tbne .Lrace_080020e04_080020E64\n"
        "\tmovs\tr0, #1\n"
        "\tbl 0x0802B368\n"
        "\tmovs\tr1, #10\n"
        "\tstr\tr1, [r4, #32]\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r4, #36]\n"
        "\tstr\tr1, [r4, #64]\n"
        "\tstr\tr0, [r4, #60]\n"
        "\tldr r2, .Lrace_080020e04_080020F38\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #138\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r0, r1]\n"
        "\tlsls\tr1, r0, #1\n"
        "\tadds\tr1, r1, r0\n"
        "\tlsls\tr1, r1, #2\n"
        "\tadds\tr1, r1, r2\n"
        "\tadds\tr1, #48\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #152\n"
        "\tldmia\tr0!, {r2, r3, r7}\n"
        "\tstmia\tr1!, {r2, r3, r7}\n"
        ".Lrace_080020e04_080020E64:\n"
        "\tcmp\tr6, #32\n"
        "\tbne .Lrace_080020e04_080020E6E\n"
        "\tldrh\tr0, [r5, #0]\n"
        "\tsubs\tr0, #1\n"
        "\tstrh\tr0, [r5, #0]\n"
        ".Lrace_080020e04_080020E6E:\n"
        "\tmov\tr0, r9\n"
        "\tcmp\tr0, #16\n"
        "\tbne .Lrace_080020e04_080020E7A\n"
        "\tldrh\tr0, [r5, #0]\n"
        "\tadds\tr0, #1\n"
        "\tstrh\tr0, [r5, #0]\n"
        ".Lrace_080020e04_080020E7A:\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r5, r1]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lrace_080020e04_080020E86\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r5, #0]\n"
        ".Lrace_080020e04_080020E86:\n"
        "\tldr r1, .Lrace_080020e04_080020F3C\n"
        "\tldr r0, .Lrace_080020e04_080020F38\n"
        "\tmovs\tr2, #252\n"
        "\tlsls\tr2, r2, #4\n"
        "\tadds\tr6, r0, r2\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r6, r3]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r0, r1\n"
        "\tmovs\tr7, #0\n"
        "\tldrsh\tr1, [r5, r7]\n"
        "\tldrh\tr2, [r0, #0]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r0, r3]\n"
        "\tcmp\tr1, r0\n"
        "\tblt .Lrace_080020e04_080020EA8\n"
        "\tstrh\tr2, [r5, #0]\n"
        ".Lrace_080020e04_080020EA8:\n"
        "\tldrh\tr7, [r5, #0]\n"
        "\tcmp\tr7, r8\n"
        "\tbeq .Lrace_080020e04_080020EF8\n"
        "\tmovs\tr0, #2\n"
        "\tbl 0x0802B368\n"
        "\tldr r0, .Lrace_080020e04_080020F40\n"
        "\tldr r3, .Lrace_080020e04_080020F44\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r5, r2]\n"
        "\tlsls\tr1, r1, #1\n"
        "\tmovs\tr7, #0\n"
        "\tldrsh\tr2, [r6, r7]\n"
        "\tlsls\tr2, r2, #3\n"
        "\tadds\tr1, r1, r2\n"
        "\tadds\tr1, r1, r3\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r1, r2]\n"
        "\tmovs\tr2, #7\n"
        "\tbl sub_080075E8\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r5, r3]\n"
        "\tcmp\tr0, #0\n"
        "\tbne .Lrace_080020e04_080020EF8\n"
        "\tadds\tr2, r4, #0\n"
        "\tadds\tr2, #196\n"
        "\tldr r1, .Lrace_080020e04_080020F48\n"
        "\tmovs\tr5, #0\n"
        "\tldrsh\tr0, [r6, r5]\n"
        "\tlsls\tr0, r0, #3\n"
        "\tadds\tr0, r0, r1\n"
        "\tmovs\tr7, #0\n"
        "\tldrsh\tr1, [r0, r7]\n"
        "\tstr\tr1, [r2, #0]\n"
        "\tldr\tr0, [r4, #28]\n"
        "\tsubs\tr2, #4\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        ".Lrace_080020e04_080020EF8:\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #148\n"
        "\tldr\tr1, [r0, #0]\n"
        "\tadds\tr2, r4, #0\n"
        "\tadds\tr2, #142\n"
        "\tldrh\tr0, [r2, #0]\n"
        "\tstrb\tr0, [r1, #0]\n"
        "\tadds\tr4, #184\n"
        "\tldr r3, .Lrace_080020e04_080020F4C\n"
        "\tmovs\tr0, #0\n"
        "\tldrsh\tr2, [r2, r0]\n"
        "\tldr r0, .Lrace_080020e04_080020F38\n"
        "\tmovs\tr1, #252\n"
        "\tlsls\tr1, r1, #4\n"
        "\tadds\tr0, r0, r1\n"
        "\tmovs\tr5, #0\n"
        "\tldrsh\tr1, [r0, r5]\n"
        "\tlsls\tr0, r1, #2\n"
        "\tadds\tr0, r0, r1\n"
        "\tadds\tr0, r0, r2\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r0, r3\n"
        "\tmovs\tr7, #0\n"
        "\tldrsh\tr0, [r0, r7]\n"
        "\tstr\tr0, [r4, #0]\n"
        "\tpop\t{r3, r4}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".Lrace_080020e04_080020F38: .word 0x03001780\n"
        ".Lrace_080020e04_080020F3C: .word 0x0203F980\n"
        ".Lrace_080020e04_080020F40: .word 0x08349754\n"
        ".Lrace_080020e04_080020F44: .word 0x080CBEF0\n"
        ".Lrace_080020e04_080020F48: .word 0x080CBEB0\n"
        ".Lrace_080020e04_080020F4C: .word 0x080CBF30\n"
        ".syntax divided\n"
    );
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

void sub_080020F50(void *rec0, int sel) __attribute__((alias("_080020F50")));

// agbcc closes a section with the 2-byte Thumb NOP `c0 46`; the 198-byte ROM
// span ends `47 00 00 00` (`bx r0` then two zero bytes), so the 200-byte
// section's trailing pad must be 0 rather than gas's default Thumb nop fill.
__asm__(".align 2, 0");

// ---- 0x08021018 — repeat emitter: kind 6 x (u/2), kind 5 on odd ----
void _080021018(void *rec0, int a, int b, int c) {
    register volatile u8 *rec __asm__("r9") = (volatile u8 *)rec0;
    int a_slot = a;
    register int breg __asm__("sl") = b;
    // u = clamp((s8)c, 0, 32): identity 1..31, 32 above, 0 at/below 0.
    register u32 byte __asm__("r1") = (u8)c;
    register s32 signedC __asm__("r3") = (s8)c;
    if (signedC > 31)
        byte = 32;
    if ((s32)(byte << 24) <= 0)
        byte = 0;
    register u32 shifted __asm__("r0") = byte << 24;
    __asm__ volatile("" : "+r" (shifted));
    register s32 u __asm__("r1") = (s32)(s8)(shifted >> 24);
    register s32 n0 __asm__("r0") = (u + (s32)(shifted >> 31)) >> 1;
    __asm__ volatile("" : "+r" (n0));
    register s32 n __asm__("r6") = n0;
    register s32 odd __asm__("r8") = (s8)(u - (n << 1));
    register int remaining __asm__("r5") = 0;
    __asm__ volatile("" : "+r" (remaining));
    if (remaining < n) {
        /* These are the four stack arguments consumed by 7B18. Pinning the
         * current outgoing area to SP lets C place them before the register
         * arguments without reproducing the store instructions in asm. */
        register volatile u32 *outgoing __asm__("sp");
        int one = 1;
        register int row __asm__("r4") = a_slot;
        remaining = n;
        do {
            outgoing[0] = 9;
            outgoing[1] = one;
            outgoing[2] = one;
            outgoing[3] = 0;
            __asm__ volatile ("" : "+r" (rec));
            register volatile u8 *row_record __asm__("r0") = rec + 8;
            ((void (*)(void *, int, int, int))_08007B18)(
                (void *)row_record, 6, row, breg);
            row += 8;
            remaining--;
        } while (remaining != 0);
        // The ROM ends the `if` body with `adds r5,r6,#0` (remaining = n) at
        // 0x08021084 and its `bge` at 0x08021060 jumps PAST it to 0x08021086
        // (`mov r0,r8`), so the restore belongs INSIDE the `if`. With it
        // outside, agbcc retargets the `bge` onto the shared store (target
        // 0x08021084 -> 0x08021082) and the tail reads r5 = n instead of 0.
        remaining = n;
    }
    if (odd == 1) {
        // The second call site uses ordinary C and already matches the ROM;
        // retain its distinct argument expressions and register allocation.
        _08007B18((void *)(uintptr_t)(rec + 8), 5, a_slot + 8 * remaining,
                  breg, 9, (u32)odd, (u32)odd, 0);
    }
}
__asm__(".align 2, 0");

// ---- 0x080210BC — scene assembly: 7B18/D28/F50/1018 chain ----
// EXACT 396/396. Four spellings carry the body; the rest is forced call setup.
//  * WA+0xFC0 gate: same idiom as _080020F50 (symbol in a pointer VARIABLE, so
//    agbcc emits the two-word `Wa` + `movs #252 / lsls #4` add instead of
//    folding the offset into one pool word).
//  * the gate is two gotos, not `idx >= 0 && idx <= 2`: the connected range
//    test collapses into one unsigned `cmp #2 / bhi`, while the ROM keeps the
//    signed `cmp #2 / bgt` + `cmp #0 / blt` pair.
//  * `s` is ONE s8 shared by all three 1018 blocks, so it takes a callee-saved
//    home (r4), accumulates the sum there, sign-extends in place and copies to
//    r3 at each call. A per-block local instead lands the sign-extend directly
//    in r3 and drops the three `adds r3,r4,#0`.
//  * `rec+16` is bound before `R32(rec,184)` is read: as a single expression
//    agbcc derives rec+16 from the loaded pointer (`subs r0,#168`) instead of
//    forming it first, which the ROM does.
void _0800210BC(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    volatile u8 *wa;
    int idx;
    s8 s;

    _08007B18((void *)(uintptr_t)(rec + 68), 8, 112, 64, 6, 3, 1, 0);
    _080026A2C((void *)R32(rec, 204), 160, 32);
    __asm__(".globl RaceSceneWa210BC\nRaceSceneWa210BC = 0x03001780\n");
    wa = (volatile u8 *)RaceSceneWa210BC;
    idx = (int)RD16(wa, 0xFC0);
    if (idx > 2)
        goto alt;
    if (idx < 0)
        goto alt;
    _08007B18((void *)(uintptr_t)(rec + 8), 2, 0, 32, 3, 3, 1, 0);
    goto join;
alt:
    _08007B18((void *)(uintptr_t)(rec + 8), 4, 0, 32, 3, 3, 1, 0);
join:
    _08007B18((void *)(uintptr_t)(rec + 8), 3, 0, 56, 3, 3, 1, 0);
    _080020D28((void *)(uintptr_t)rec, 24, 56, (int)RD16(rec, 142), 7);
    _08007B18((void *)(uintptr_t)rec, R32(rec, 172), 32, 40, 4, 1, 1, 0);
    {
        volatile u8 *p16 = rec + 16;
        _08007B18((void *)(uintptr_t)p16, R32(rec, 184), 8, 96, 5, 1, 1, 0);
    }
    _080020F50((void *)(uintptr_t)rec, (int)R16(rec, 142));
    s = (s8)(_08025500((int)RD16(rec, 138)) + _08025548((const void *)(rec + 152)));
    _080021018((void *)(uintptr_t)rec, 103, 104, s);
    s = (s8)(_08025518((int)RD16(rec, 138)) + _080255C4((const void *)(rec + 152)));
    _080021018((void *)(uintptr_t)rec, 103, 120, s);
    s = (s8)(_08025530((int)RD16(rec, 138)) + _08025640((const void *)(rec + 152)));
    _080021018((void *)(uintptr_t)rec, 103, 136, s);
    _0800D97C((void *)(uintptr_t)(rec + 164), 15);
    ((void (*)(void *))_080021248)((void *)(uintptr_t)rec);
    _0800DBE8((void *)(uintptr_t)(rec + 32));
}
__asm__(".align 2, 0");

// ---- 0x08021260 — FBC-gated frame counter (event 21 past 180) + pools {WA+0xFBC} ----
void _080021260(void *rec_) {
    extern u8 RaceSceneWa21260[] __asm__("RaceSceneWa21260");
    __asm__(".globl RaceSceneWa21260\nRaceSceneWa21260 = 0x03001780\n");
    uintptr_t base = (uintptr_t)RaceSceneWa21260;
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
// ONE hop.
void sub_0800212AC(int ev, int b, int c, void *rec0) __attribute__((alias("_0800212AC")));

// agbcc closes a section with the 2-byte Thumb NOP `c0 46`; the ROM ends this
// span on two zero bytes. The explicit align makes the fill `0` instead, which
// is the last two bytes of the 176-byte span (174/176 -> EXACT).
__asm__(".align 2, 0");
