// ============================================================================
// Scene and record dispatch, including the race FSM at 0x0800AA40 and
// course phase machine at 0x08008CA0. Assembly owners:
//
//   asm/menu_progress_update.s   0x0800BD40 (results progress updater)
//   asm/menu_place.s             0x0800BE74 (placement/tier updater)
//   asm/menu_dab8.s              0x0800DAB8 (menu obj initializer, 10-entry)
//   asm/menu_d470.s              0x0800D470 (menu event dispatcher, 11-entry)
//   asm/menu_d3a4.s              0x0800D3A4 (menu selection handler)
//   asm/menu_d048.s              0x0800D048 (menu record updater)
//   asm/menu_ctor2.s             0x0800C1E4 (menu record-1 constructor)
//   asm/menu_ctor.s              0x0800C010 (menu/save scene constructor)
//   asm/menu_cae4.s              0x0800CAE4 (menu detail renderer)
//   asm/menu_c884.s              0x0800C884 (menu record handler)
//   asm/menu_c4d0.s              0x0800C4D0 (menu record-20 constructor)
//   asm/course_resource_leaf_more2.s 0x08007B8C (record emitter twin)
//   asm/course_resource_leaf_more.s  0x08007B18 (record emitter)
//   asm/course_proximity.s       0x08007210 (proximity search)
//   asm/code_25264.s             0x08025264 (lineup builder)
//   asm/code_23628.s             0x08023628 (race scene ctor)
//   asm/ai_racefsm.s             0x0800AA40 (per-frame race FSM, 8-way)
//   asm/course_dispatch_8aac.s   0x08008CA0 (record-49 phase machine, 9-way)
//
// Transcribed instruction-for-instruction from the asm listings (all pure
// Thumb, byte-exact via the make SHA gate).

#include "gba/types.h"

// ----------------------------------------------------------------------------
// Callees (HOST_STUB = weak on Apple host builds, extern on ARM; strong
// bodies win on the ARM link).
// ----------------------------------------------------------------------------
#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

HOST_STUB(void  _0800BC04(void));                                    // bx-lr stub @0x0800BC04
HOST_STUB(void  _0800BC08(void *a0, void *a1, void *a2, void *a3));
HOST_STUB(void  _0800BCD4(void *a0, void *a1, void *a2, void *a3, void *s1, void *s2, void *s3, void *s4));
HOST_STUB(int   _08002581C(int a));
HOST_STUB(int   _080025750(int a, int b, int c));
HOST_STUB(u32   _080025E98(u32 a, int b, int c));
HOST_STUB(void  _080025E70(u32 a, int b, int c, u32 d));
HOST_STUB(int   _080025E1C(int a));
HOST_STUB(void  _080025DBC(int a, int b));
HOST_STUB(int   _08002DE04(int n, int d));
HOST_STUB(u32   _08002DF6C(u32 n, u32 d));
HOST_STUB(void  _08002DE98(void));                                   // no-op leaf (mov pc,lr)
HOST_STUB(int   _08002D978(int a, int b));                           // swi Div (quotient)
HOST_STUB(int   _08002D97C(int a, int b));                           // swi DivRem (remainder)
HOST_STUB(int   _08002BE8(int dummy));                               // counter bump, s16 ret
HOST_STUB(void  _08002C48(int a, u16 b));
HOST_STUB(void  _08002D98(u32 a, u32 b, u32 c, u32 d, u32 e));
#ifndef __APPLE__
extern void  _0802E104(void *a, int b, u32 c);
extern void  sub_0802D974(const void *src, void *dst, u32 ctrl);
#define _08002E104 _0802E104
#define _08002D974 sub_0802D974
#else
HOST_STUB(void  _08002E104(void *a, int b, u32 c));                  // memset-like
HOST_STUB(void  _08002D974(const void *src, void *dst, u32 ctrl));   // CpuSet
#endif
HOST_STUB(void  _08004C84(u32 a, u32 b));                            // trampoline -> ROM
HOST_STUB(void *_08007498(void *x, int i));
HOST_STUB(void *_0800748C(void *x));
HOST_STUB(void *_08002BFC(int a));
HOST_STUB(void  _08002C34(int idx, void *node));
HOST_STUB(void  _0800798C(void *a, void *b));
HOST_STUB(void  _08007A58(void *p));
HOST_STUB(void  _080075E8(void *a, int b, int c));
HOST_STUB(void  _08007614(void *a, int b, int c, int d));
HOST_STUB(void  _08007ABC(void *a, void *b, int c));
HOST_STUB(int   _0800572C(int v));
HOST_STUB(void  _080056FC(void));
HOST_STUB(void  _080055D8(u32 a, u16 b));
HOST_STUB(void  _08004D4C(u32 a, u32 b, u32 c));
HOST_STUB(void  _080050E8(void *a, int b));
#ifndef __APPLE__
extern void  sub_080032E0(volatile u8 *p);
extern void  sub_08003104(void *handler);
#define _080032E0 sub_080032E0
#define _08003104 sub_08003104
#else
HOST_STUB(void  _080032E0(volatile u8 *p));
HOST_STUB(void  _08003104(void *handler));
#endif
HOST_STUB(void *_08004DC8(void *a));
HOST_STUB(void  _08004EF0(void *a));
HOST_STUB(void *_08004B68(void));
HOST_STUB(void  _08004BFC(int v));
HOST_STUB(void  _08004EA8(int v));
HOST_STUB(void  _08004EC0(int v));
HOST_STUB(u16   _080024A0(void));
HOST_STUB(int   _0800A9E0(int t, int r));
HOST_STUB(int   _0800A9A0(int t, int r));HOST_STUB(u32   _08002140(void));
HOST_STUB(u16   _08002178(int i));
HOST_STUB(void  _08002158(int a, int b));
HOST_STUB(void  _080016D0(int v));
HOST_STUB(u16   _08001E8C(int a, int b));
HOST_STUB(u16   _08001EA4(int a));
HOST_STUB(int   _08001E30(void));
HOST_STUB(void  _08001E48(int a, int b));
HOST_STUB(int   _08001F8C(void));  // 32-bit contract: ROM caller does a bare `cmp r6,r0` (no zero-extend), as in garage_records.c/_080028EB0
HOST_STUB(void  _08001F80(u16 v));
HOST_STUB(void  _08003940(int a, u32 b));
HOST_STUB(u16   _08002044(void));
HOST_STUB(void  _08003F18(u32 a, u32 b, const volatile u8 *c, int d));
HOST_STUB(u16   _0800206C(int a));
HOST_STUB(u16   _08001E5C(int a, int b));
HOST_STUB(void  _08003978(int a, int b, int c));
extern void sub_08003838(int a, u32 b, const volatile u8 *c);
extern void sub_08003954(int a, u32 b, int c);
HOST_STUB(void  _080026A4C(void *obj, int slot, int attr));
HOST_STUB(void  _08026938(void *obj, int slot));
HOST_STUB(int   _080024E54(s16 idx));
HOST_STUB(int   _080022E4(int id));
HOST_STUB(void  _0800C814(void *c));
HOST_STUB(void  _0800C454(void));
HOST_STUB(void  _0800C4AC(void));
HOST_STUB(void  _0800D298(void *rec));
HOST_STUB(void  _0800D2AC(void *rec));
HOST_STUB(void  _0800D2B0(void *rec));
HOST_STUB(void  _0800D95C(void *dst, const void *src, int n));
HOST_STUB(void  _0800DAB8(void *p));
HOST_STUB(void  _0800D77C(void *a, int b, int c));
HOST_STUB(void  _0800B4A8(void));
HOST_STUB(void  _0800B89C(void));
HOST_STUB(void  _0800B9F0(void));
HOST_STUB(void  _0800B0BC(void));
HOST_STUB(void  _080023FF8(int a, int b));
HOST_STUB(void  _080023FE4(void));
HOST_STUB(void  _08004CC4(void));
HOST_STUB(void  _08004CD4(int v));
HOST_STUB(void  _0800279C(int v));
HOST_STUB(s16   _080026F8(int a, int b));
HOST_STUB(s16   _08002730(int a, int b));
HOST_STUB(u32   _08024144(void));
HOST_STUB(int   _08005B5C(int v));
HOST_STUB(int   _08008014(void));
// The three spellings below are DROPPED-DIGIT TYPOS of the documented car-
// catalog getters : 0x08025518/25530/254E8 = Ai_LineGet2/3/0
// (s8[0x080CCEEC + 20*carId + 13/12/0]). The 0x080054E8/5518/5530 VMAs are
// mid-instruction interiors of DMA/CpuSet loops with no ROM callers — the
// veneers they produced were phantoms of this typo.
HOST_STUB(s8    _08025518(int a));
HOST_STUB(s8    _08025530(int a));
HOST_STUB(s8    _080254E8(int a));
HOST_STUB(void  _08002B214(int v));
HOST_STUB(void  _080025BC8(void *a, int b));
HOST_STUB(void  _08002124(u16 v));
HOST_STUB(u32   _08003130(void));
HOST_STUB(int   _08003D18(int a, volatile u8 *b));
HOST_STUB(void  _0800313C(u32 a));
HOST_STUB(void  _08008768(void *a, int b));
HOST_STUB(void  _080088B0(void *a, int b));
HOST_STUB(void  _0800861C(void *a, int b));
HOST_STUB(void  _080089FC(void *a));
HOST_STUB(void  _08005BA8(void *a, int b));
HOST_STUB(void  _08005DA4(void *a, void *b));
HOST_STUB(void  _080053DC(int a, const volatile u8 *b, int c, int d, int e, int f, int g, int h));
HOST_STUB(void  _080054A4(int a, int b, int c, int d, int e));
HOST_STUB(int   _08005E14(void *a, void *b, void *c));
HOST_STUB(void *_08006C10(void *state, int d));
HOST_STUB(void  _08007BFC(void *a, u8 b, u8 c, u32 d, u32 e, u32 f, u32 g));
HOST_STUB(void  _0800D280(void *a, void *b));                        // fixed 2-arg ABI
// Trampoline decls for ROM-exact callees (leads // 0x0800XXXX hints).
extern void *RomEmit8AAC(void *r7);          // 0x08008AAC
extern void *RomEmit8BB8(void *r7);          // 0x08008BB8
extern void *RomEmit8768(void *r7, int w);   // 0x08008768
extern void *RomEmit88B0(void *r7, int w);   // 0x080088B0
extern void *RomEmit861C(void *r7, int w);   // 0x0800861C
extern void *RomEmit89FC(void *r7);          // 0x080089FC
extern u32 RomLeaf7658(void *a);             // 0x08007658
extern void sub_08004CC4(void);
extern void sub_08004CD4(int v);
HOST_STUB(void  _08007658(void *a));
HOST_STUB(int   _080050D0(int a, int b));
HOST_STUB(void  _08005260(void *a, void *b, int c, int d)); // 4-arg (r3 = transfer size)
HOST_STUB(u32   _080074A8(void *x));

// ============================================================================
// Faithful div-remainder cores (sound_divmod.s 0x0802DFE4, armcc ROR
// shift-and-subtract; sound_aeabi_idiv_core.s 0x0802DE9C, same core on
// magnitudes with sign fixup). r0 = exact remainder for all game-range
// inputs; the ROR-assembly + rounding tail is replicated verbatim so even
// the huge-quotient corners match the ROM bit-for-bit. Differential-tested
// on host against an independent Python transcription of the asm.
// ============================================================================
static u32 ror32(u32 v, u32 n) {
    n &= 31u;
    return n ? (v >> n) | (v << ((32u - n) & 31u)) : v;
}

// Unsigned remainder core shared by both functions (mirrors 2E014..2E060).
static u32 divrem_core(u32 r0, u32 r1) {
    u32 r2, r3 = 1u, r4, ip;
    u32 r4s = 1u << 28;
    while (!(r1 >= r4s || r1 >= r0)) { r1 <<= 4; r3 <<= 4; }
    while (!(r1 >= r4s || r1 >= r0)) { r1 <<= 1; r3 <<= 1; }
    for (;;) {
        r2 = 0u;
        if (r0 >= r1) r0 -= r1;
        r4 = r1 >> 1; if (r0 >= r4) { r0 -= r4; r2 |= ror32(r3, 1); }
        r4 = r1 >> 2; if (r0 >= r4) { r0 -= r4; r2 |= ror32(r3, 2); }
        r4 = r1 >> 3; if (r0 >= r4) { r0 -= r4; r2 |= ror32(r3, 3); }
        ip = r3;
        if (r0 == 0u) break;
        r3 >>= 4; if (r3 == 0u) break;
        r1 >>= 4;
        (void)ip;
    }
    if ((r2 & 0xE0000000u) == 0u) return r0;
    r3 = ip;
    if (r2 & ror32(r3, 3)) r0 += r1 >> 3;
    if (r2 & ror32(r3, 2)) r0 += r1 >> 2;
    if (r2 & ror32(r3, 1)) r0 += r1 >> 1;
    return r0;
}

// _08002DFE4: unsigned remainder. den==0 routes through the _08002DE98
// no-op leaf then returns 0; num<den early-returns num (mov pc,lr).
u32 DivRemU_02DFE4(u32 num, u32 den) {
    if (den == 0u) { _08002DE98(); return 0u; }
    if (num < den) return num;
    return divrem_core(num, den);
}
#ifndef __APPLE__
u32 _08002DFE4(u32 n, u32 d) __attribute__((alias("DivRemU_02DFE4")));
u32 sub_08002DFE4(u32 n, u32 d) __attribute__((alias("DivRemU_02DFE4")));
#endif

// _08002DE9C: signed remainder. den==0 -> DE98 then 0; magnitudes through
// the core; negate iff the original num was negative (bpl/negs).
s32 DivRemS_02DE9C(s32 num, s32 den) {
    u32 r0 = (u32)num, r1 = (u32)den;
    if ((s32)r1 == 0) { _08002DE98(); return 0; }
    if ((s32)r1 < 0) r1 = 0u - r1;
    u32 saved = r0;
    if ((s32)r0 < 0) r0 = 0u - r0;
    if (r0 >= r1) r0 = divrem_core(r0, r1);
    if ((s32)saved < 0) r0 = 0u - r0;
    return (s32)r0;
}
#ifndef __APPLE__
s32 _08002DE9C(s32 a, s32 b) __attribute__((alias("DivRemS_02DE9C")));
s32 sub_08002DE9C(s32 a, s32 b) __attribute__((alias("DivRemS_02DE9C")));
#endif

// ============================================================================
// The faithful _08002ED0 placement writer now lives globally as EmitPlace_02ED0
// (src/foundation_runtime.c, _/sub_/Sub_ aliases); the emitter transcriptions
// below call it directly (10 args each).
// ============================================================================
extern void _08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j);

void CourseEmit7B18(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) {
    u8 *r4 = (u8 *)rec;
    void *arr = _0800748C(_08007498(*(void **)(r4 + 4), kind));
    u8 *a = (u8 *)arr;
    u32 base = (u32)(u16)(a[2] | ((u32)a[3] << 8)) + (u32)(u16)(r4[0] | ((u32)r4[1] << 8));
    u32 cnt = a[7];
    u8 *ent = a + 8;
    for (u32 i = 0; i < cnt; i++, ent += 4) {
        u32 x = (u32)ent[2] + (u32)dx;
        u32 y = (u32)ent[3] + (u32)dy;
        u32 z = (u32)ent[0] + base;
        _08002ED0((void *)(uintptr_t)x, (int)y, (int)z, (int)s0, (int)s1,
                   (int)ent[1], 1, (int)s2, (int)s3, 1);
    }
}

// Likewise the 0x08007B8C ROM body (see CourseEmit7B18 above).
void CourseEmit7B8C(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) {
    u8 *r4 = (u8 *)rec;
    void *arr = _0800748C(_08007498(*(void **)(r4 + 4), kind));
    u8 *a = (u8 *)arr;
    u32 r7 = (u32)(u16)(a[2] | ((u32)a[3] << 8)) + (u32)(u16)(r4[0] | ((u32)r4[1] << 8));
    u32 cnt = a[7];
    u8 *ent = a + 8;
    for (u32 i = 0; i < cnt; i++, ent += 4) {
        u32 x = (u32)ent[2] + (u32)dx;
        u32 y = (u32)ent[3] + (u32)dy;
        u32 z = (u32)ent[0] + r7;
        _08002ED0((void *)(uintptr_t)x, (int)y, (int)z, (int)s0, (int)s1,
                   (int)ent[1], 1, (int)s2, (int)s3, 0);
    }
}

// 7BFC variant: x=b2+r3-in, y=b3+caller-sp+0, z=b0+r1-in(kind),
// 2ED0 r3=caller-sp+4, stack=(caller-sp+8, b1, 1, caller-sp+12,
// caller-sp+16, 1).
static void CourseIter7BFC_Static(void *rec, int kind, int dx3,
                                  u32 s0, u32 s1, u32 s2, u32 s3, u32 s4) {
    u8 *r4 = (u8 *)rec;
    void *arr = _0800748C(_08007498(*(void **)(r4 + 4), kind));
    u8 *a = (u8 *)arr;
    u32 cnt = a[7];
    u8 *ent = a + 8;
    for (u32 i = 0; i < cnt; i++, ent += 4) {
        u32 x = (u32)ent[2] + (u32)dx3;
        u32 y = (u32)ent[3] + s0;
        u32 z = (u32)ent[0] + (u32)kind;
        _08002ED0((void *)(uintptr_t)x, (int)y, (int)z, (int)s1, (int)s2,
                   (int)ent[1], 1, (int)s3, (int)s4, 1);
    }
}

// ============================================================================
// menu_progress_update.s 0x0800BD40 — results progress updater. Seat fill via
// _0800BC08, conditional record apply via _0800BCD4, 12-byte award copy on
// the sv==1 path, WA+0x10F4 clear otherwise, WA+0xFCE latch at the tail.
// ============================================================================
void MenuProgressUpdate_0BD40(void) {
    u8 stk[60];
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    *(u32 *)(stk + 52) = 0u;
    *(u32 *)(stk + 56) = 0u;
    u8 *r4 = stk + 40;
    *(u16 *)r4 = 0u;
    _0800BC04();
    int r5 = (int)(s16)(int)_08002581C((int)(s16)(*(u16 *)(wa + 0x576)));
    u8 *r7 = stk + 44, *r6 = stk + 28;
    _0800BC08(r7, (void *)(uintptr_t)(s32)r5, stk + 16, r6);
    u8 *flagcell = wa + 0x10E4;
    int domain = (*(volatile s8 *)flagcell != 0);
    if (domain) {
        _0800BCD4((void *)(uintptr_t)(s32)r5, r7, stk + 48, r4,
                  stk + 52, stk + 56, stk + 16, r6);
        domain = (*(volatile s8 *)flagcell != 0);
        if (domain) {
            *(u16 *)(wa + 0xFC8) = *(u16 *)(stk + 52);
            u16 v56 = *(u16 *)(stk + 56);
            *(u16 *)(wa + 0xFCA) = v56;
            if ((int)(s16)v56 == 1) {
                u32 *src = (u32 *)(stk + 28);
                u32 *dst = (u32 *)(wa + (s32)r5 * 72 + 1568);
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
            }
        }
    }
    if (!domain) *(u32 *)(wa + 0x10F4) = 0u;
    *(u16 *)(wa + 0xFCE) = *(u16 *)r4;
}
#ifndef __APPLE__
void _0800BD40(void) __attribute__((alias("MenuProgressUpdate_0BD40")));
void sub_0800BD40(void) __attribute__((alias("MenuProgressUpdate_0BD40")));
#endif

// ============================================================================
// menu_place.s 0x0800BE74 — placement/tier updater. Signed (sl) vs unsigned
// (r4) mixed-radix decomposition of two related cells (//3 exact), tiered on
// the unsigned difference, then grid-record resolve via _080025E98/_080025E70
// and tier store to WA+0x10E5. (Proves _08002DE9C/_08002DFE4 = remainders:
// quotient readings degenerate the decomposition.)
// ============================================================================
void MenuPlaceUpdate_0BE74(void) {
    u8 stk[16];
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    *(u32 *)(stk + 12) = 0u;
    (void)_08002581C((int)(s16)(*(u16 *)(wa + 0x576)));
    int aval = _080025750((int)(s16)(*(u16 *)(wa + 0xFF2)),
                          (int)(s16)(*(u16 *)(wa + 0xFF6)),
                          (int)(s16)(*(u16 *)(wa + 0x103A)));
    *(u32 *)(stk + 0) = *(u32 *)(wa + 0x10F4);
    *(u16 *)(stk + 4) = *(u16 *)(wa + 0x574);
    _0800D95C(stk + 8, wa + 0x1088, 3);
    u32 sl = (u32)_08002DE04(aval, 18000) * 6000u;
    sl += (u32)_08002DE04(DivRemS_02DE9C(aval, 18000), 300) * 100u;
    sl += (u32)_08002DE04(DivRemS_02DE9C(aval, 300), 3);
    u32 bval = *(u32 *)(stk + 0);
    u32 r4 = (u32)_08002DF6C(bval, 18000u) * 6000u;
    r4 += (u32)_08002DF6C(DivRemU_02DFE4(bval, 18000u), 300u) * 100u;
    r4 += (u32)_08002DF6C(DivRemU_02DFE4(bval, 300u), 3u);
    u32 diff = sl - r4;
    u32 tier = 0u;
    if (*(volatile s8 *)(wa + 0x10E4) != 0) {
        if (diff <= 99u) tier = 1u;
        else if (diff - 100u <= 99u) tier = 2u;
        else tier = 3u;
    }
    int g2 = (int)(s16)(*(u16 *)(wa + 0xFF2));
    int g6 = (int)(s16)(*(u16 *)(wa + 0xFF6));
    int gv = (int)(s16)(*(u16 *)(wa + 0x103A));
    u32 row0 = *(u32 *)(stk + 0);
    if (_080025E98((u32)(s32)g2, g6, gv) > row0) {
        _080025E70((u32)(s32)g2, g6, gv, row0);
    } else if (_080025E98((u32)(s32)g2, g6, gv) == 0u) {
        _080025E70((u32)(s32)g2, g6, gv, row0);
    }
    *(wa + 0x10E5) = (u8)tier;
}
#ifndef __APPLE__
void _0800BE74(void) __attribute__((alias("MenuPlaceUpdate_0BE74")));
void sub_0800BE74(void) __attribute__((alias("MenuPlaceUpdate_0BE74")));
#endif

// ============================================================================
// menu_dab8.s 0x0800DAB8 — menu object initializer. Resource attach via the
// _0800798C/_08007A58/_080075E8 family, save-alloc triple, 10-entry phase
// table on s16[WA+0xFBC], then three _08007ABC bindings.
// ============================================================================
// ROM 0x0800DAB8: the ten case blocks each end in `movs r0,#k / b 0x0800DADE`
// and the single store `str r0,[r6,#72]` sits at 0x0800DADE, which the
// `default` (0x0800DAB0 `bhi.n`) jumps PAST. So the C stores per case and
// has no `cls >= 0` test, and case 5 is `if/else if/else`, not a ternary
// (`cmp r0,#1 / beq / cmp r0,#2 / beq` at 0x0800DAB2). Written that way the
// prologue becomes byte-exact (only r4/r5/r6 saved) but the s16 address
// `ldr r0,[pc] / ldr r1,[pc] / adds r0,r0,r1` (0x0800DABE) is 4 bytes that
// agbcc will not generate from any constant spelling - it folds to one
// literal - so the body stays short and every later byte shifts. Kept the
// scoring form until that address form is reachable.
void MenuObjInit_0DAB8(void *rec) {
    u8 *r6 = (u8 *)rec;
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    u8 *tmpl = (u8 *)(uintptr_t)0x082A798Cu;
    u8 *r4 = r6 + 0x24;
    _0800798C(tmpl, r4);
    _08007A58(r4);
    _080075E8(tmpl, 4, 1);
    _080075E8(tmpl, 3, 2);
    _0800798C((void *)(uintptr_t)0x082A95E4u, r6 + 0x34);
    *(u32 *)(r6 + 0x44) = (u32)_0800572C(0x10);
    int cls = -1;
    switch ((int)(s16)(*(u16 *)(wa + 0xFBC))) {
    case 0: cls = 7; break;
    case 1: cls = 6; break;
    case 2: cls = 8; break;
    case 3: cls = 5; break;
    case 4: cls = 1; break;
    case 5: {
        int sub = (int)(s16)(*(u16 *)(wa + 0x1078));
        cls = (sub == 1) ? 0xB : (sub == 2) ? 0xA : 2;
        break;
    }
    case 6: cls = 3; break;
    case 7: cls = 0; break;
    case 8: cls = 4; break;
    case 9: cls = 9; break;
    default: break;
    }
    if (cls >= 0) *(u32 *)(r6 + 0x48) = (u32)cls;
    _08007ABC((void *)(uintptr_t)*(u32 *)(r6 + 0x38),
              (void *)(uintptr_t)*(u32 *)(r6 + 0x48),
              (int)*(u32 *)(r6 + 0x44));
    _0800798C((void *)(uintptr_t)0x082AB030u, r6 + 0x3C);
    int a26 = _0800572C(0x26);
    *(u32 *)(r6 + 0x50) = (u32)a26;
    _08007ABC((void *)(uintptr_t)*(u32 *)(r6 + 0x40),
              (void *)(uintptr_t)*(u32 *)(r6 + 0x54), a26);
    _0800798C((void *)(uintptr_t)0x082B283Cu, r6 + 0x2C);
    int a3c = _0800572C(0x3C);
    *(u32 *)(r6 + 0x5C) = (u32)a3c;
    _08007ABC((void *)(uintptr_t)*(u32 *)(r6 + 0x30),
              (void *)(uintptr_t)*(u32 *)(r6 + 0x60), a3c);
}
#ifndef __APPLE__
void _0800DAB8(void *p) __attribute__((alias("MenuObjInit_0DAB8")));
void Sub_0800DAB8(void *p) __attribute__((alias("MenuObjInit_0DAB8")));
void sub_0800DAB8(void *p) __attribute__((alias("MenuObjInit_0DAB8")));
void Course_0x0800DAB8(void *p) __attribute__((alias("MenuObjInit_0DAB8"))); /* trampoline elimination: friendly-name spelling used by rec35_init.c */
#endif

// ============================================================================
// menu_d470.s 0x0800D470 — menu event dispatcher (ev 1-based; 11-entry table
// with holes falling through to the tail).
// ============================================================================
void MenuSelectHandler_0D3A4(void *rec, u32 a, u32 b);  // fwd (defined below)
void MenuEventDispatch_0D470(int ev, u32 a, u32 b, void *rec) {
    u8 *r3 = (u8 *)rec;
    register u32 r4 __asm__("r4") = a;
    switch (ev - 1) {
    case 1:
        _0800D280(r3, (void *)(uintptr_t)r4);
        break;
    case 0:
        _0800D298(r3);
        break;
    case 5:
        MenuSelectHandler_0D3A4(r3, (u16)r4, (u16)b);
        break;
    case 4:
        _0800D2AC(r3);
        break;
    case 6:
        _0800D2B0(r3);
        break;
    case 10:
        break;
    default:
        break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D470(int e, u32 a, u32 b, void *r) __attribute__((alias("MenuEventDispatch_0D470")));
void sub_0800D470(int e, u32 a, u32 b, void *r) __attribute__((alias("MenuEventDispatch_0D470")));
#endif

// ============================================================================
// menu_d3a4.s 0x0800D3A4 — menu selection handler: token handshake, per-slot
// key scan with cursor walk on bits 6/7, transition requests on bits 0/1,
// paint invocations on exit.
// ============================================================================
void MenuSelectHandler_0D3A4(void *rec, u32 a, u32 b) {
    u8 *r5 = (u8 *)rec;
    u16 r8 = (u16)a, r7 = (u16)b;
    _080016D0(123);
    *(u16 *)(r5 + 6) = _08001EA4(2);
    for (int r6 = 0; r6 < _08001F8C(); r6++) {   // signed compare: ROM branches `blt`, not `bcc`
        (void)_08001E8C((int)r6, 0);
        u16 key = _08001E8C((int)r6, 1);
        if (_08001E30()) {
            if (key & 0x40) {
                if (*(u16 *)(r5 + 4) != 0u) *(u16 *)(r5 + 4) -= 1u;
            }
            if (key & 0x80) {
                if (*(u16 *)(r5 + 4) <= 2u) *(u16 *)(r5 + 4) += 1u;
            }
        }
        if (key & 1u) {
            _08004EC0(1);
            _08004BFC(*(u16 *)(r5 + 6));
            goto d438;
        }
        if (key & 2u) {
            _08004EA8(1);
            goto d438;
        }
    }
d438:
    if (r7 & 0x100u) {
        _08004EC0(1);
        _08004BFC(0);
    }
    _08001E48(0, r8);
    _08001E48(1, r7);
    _08001E48(2, *(u16 *)(r5 + 4));
}
#ifndef __APPLE__
void _0800D3A4(void *r, u32 a, u32 b) __attribute__((alias("MenuSelectHandler_0D3A4")));
void sub_0800D3A4(void *r, u32 a, u32 b) __attribute__((alias("MenuSelectHandler_0D3A4")));
#endif

// ============================================================================
// menu_d048.s 0x0800D048 — menu record updater: key-bit lanes adjust the
// +6/+8 cursors with clamp/relayout, persisting to WA+0x574/0x1151 on bit0.
// (Second _08026A4C call passes s16[rec+8] explicitly = the value ROM's r2
// still holds: the compiled Sprite_SetTenTwelve preserves r2, as does ROM.)
// ============================================================================
void MenuRecordUpdater_0D048(void *rec, u32 dummy, u32 key) {
    (void)dummy;
    u8 *r4 = (u8 *)rec;
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    u16 r5 = (u16)key;
    (void)_08004B68();
    int r8 = (int)(s16)(*(u16 *)(r4 + 6));
    int r7 = 0, r6 = 0;
    int r9 = (int)(s16)(*(u16 *)(r4 + 8));
    if (r5 & 2u) _08004EA8(1);
    if (r5 & 1u) {
        *(u16 *)(wa + 0x574) = *(u16 *)(r4 + 6);
        *(wa + 0x1151) = (u8)*(u16 *)(r4 + 8);
        _08004EA8(1);
    }
    if (r5 & 32u) r7--;
    if (r5 & 16u) r7++;
    if (r5 & 64u) r6--;
    if (r5 & 128u) r6++;
    int v8 = (int)(s16)(*(u16 *)(r4 + 8)) + r6;
    *(u16 *)(r4 + 8) = (u16)v8;
    if ((s16)(u16)v8 < 0) *(u16 *)(r4 + 8) = 4u;
    if ((int)(s16)(*(u16 *)(r4 + 8)) > 4) *(u16 *)(r4 + 8) = 0u;
    int v6 = (int)(s16)(*(u16 *)(r4 + 6)) + r7;
    *(u16 *)(r4 + 6) = (u16)v6;
    if ((s16)(u16)v6 < 0) *(u16 *)(r4 + 6) = 0u;
    if ((int)(s16)(*(u16 *)(r4 + 6)) > 96) *(u16 *)(r4 + 6) = 96u;
    if ((int)(s16)(*(u16 *)(r4 + 6)) != r8) {
        void *obj = *(void **)(r4 + 28);
        int s6 = (int)(s16)(*(u16 *)(r4 + 6));
        int s8 = (int)(s16)(*(u16 *)(r4 + 8));
        _080026A4C(obj, s6, s8);
        _08026938(obj, s6);
    }
    if ((int)(s16)(*(u16 *)(r4 + 8)) != r9) {
        void *obj = *(void **)(r4 + 28);
        _080026A4C(obj, (int)(s16)(*(u16 *)(r4 + 6)),
                   (int)(s16)(*(u16 *)(r4 + 8)));
    }
}
#ifndef __APPLE__
void _0800D048(void *r, u32 d, u32 k) __attribute__((alias("MenuRecordUpdater_0D048")));
void sub_0800D048(void *r, u32 d, u32 k) __attribute__((alias("MenuRecordUpdater_0D048")));
#endif

// 16-byte template the two menu constructors copy onto the stack; a whole
// struct assignment is what agbcc expands to `ldmia/stmia r0!,{r2,r3,r4}`.
typedef struct { u32 w[4]; } MenuCtor_Tmpl;

// ============================================================================
// menu_ctor2.s 0x0800C1E4 — menu record-1 constructor: stack context init,
// template attach, manager/queue wiring, two resource lookups.
// ============================================================================
void MenuCtor2_0C1E4(void *rec) {
    u8 *r5 = (u8 *)rec;
    u8 stk[84];
    u8 *r4 = stk;
    // Two values of 1, and the ROM keeps them in two DIFFERENT hard registers:
    // `one` in r6 (`strb r6,[r4,#4]` at +0x26 and `strb r6,[r4,#9]` at +0x2C,
    // both after the `_08002E104` call) and the halfword constant in r9
    // (`mov r1,r9 / strh r1,[r4,#2]` at +0x18 and `mov r1,r9 / strh r1,[r0,#0]`
    // at +0x56). That split is what produces the prologue
    // `mov r6,r9 / mov r5,r8 / push {r5,r6}`.
//
    // `one` is pinned to r6 with a register variable, and the pin is what
    // closes the body. Unpinned, agbcc emits the two set-ups in the opposite
    // order -- `movs r0,#1 / mov r9,r0` then `movs r6,#1` -- and the body is
    // 219/224 at prefix 19 with a 5-byte inversion at +0x12; the reload pass
    // places the r9 constant's set-up at the head of the block that holds its
    // first use, ahead of the expand-time local init in that same block. No
    // source *shape* moves the init: the two constants' types (u8/u16/u32/s32/
    // const), both-or-either being a named local, declaration order, explicit
    // nesting, the byte stores moved ahead of the halfword store, and an
    // assign-after-declare form were all measured at 219/224 prefix 19. The pin
    // changes the allocation, not the order, and the allocation is what the
    // reload keys on.
//
    // The pin and the named `k1` are NOT independent -- each alone leaves the
    // body at 219/224:
    //   pin `one` only, literal `1u` in the halfword store -> prefix 24 but
    //     only 66/224 matched: the constant's uses get copy-propagated into
    //     the scratch (`strh r0,[r4,#2]`, `mov r4,r9 / strh r4,[r0,#0]`)
    //     instead of `mov r1,r9`.
    //   named `k1` only, `one` a plain local             -> 219/224 prefix 19.
    // Naming the halfword constant `k1` keeps its pseudo a separate live
    // value, so the use-site reload picks r1 and emits the ROM's `mov r1,r9`.
//
    // `k1` is a plain local on purpose. Pinning it too (`register u16 k1
    // __asm__("r9")`) costs 4 bytes -- the set-up collapses to a single
    // `movs` and the span becomes 220, not 224 (prefix 22). Two `register
    //... __asm__` declarations cannot share one declaration block at all:
    // tools/agbcc_c89_transform.py gives up with "budget exhausted" and drops
    // the whole TU.
//
    // `one` stays declared AFTER the zero store, and that position is still
    // load-bearing: declared first, agbcc issues the `movs r6,#1` ahead of the
    // `strh` and the body diverges at +0x0E (215/224, prefix 14).
    *(u16 *)(r4 + 0) = 0u;
    register u8 one __asm__("r6");
    one = 1u;
    u16 k1 = 1u;
    *(u16 *)(r4 + 2) = k1;
    _08002E104(stk + 4, 0, 64u);
    *(r4 + 4) = one;
    *(r4 + 6) = 16u;
    *(r4 + 9) = one;
    *(void **)(stk + 16) = r5 + 0xCE4;
    u8 *r8 = stk + 68;
    *(MenuCtor_Tmpl *)(uintptr_t)r8 = *(const MenuCtor_Tmpl *)(uintptr_t)0x0805F6C4u;
    _080056FC();
    _080055D8((u32)(uintptr_t)(r5 + 0x1CE4), 16u);
    *(u16 *)(r5 + 92) = 1u;
    u8 *rr4 = r5 + 8;
    _08002D974(stk, rr4, 0x04000011u);
    u8 *rr6 = r5 + 76;
    _08002D974(r8, rr6, 0x04000004u);
    _08004D4C(2u, (u32)(uintptr_t)rr4, 0u);
    _080050E8(rr4, 0);
    _080032E0(rr6);
    _08003104(r5 + 100);
    *(void **)(r5 + 0x1DE4) = _08004DC8((void *)(uintptr_t)0x080CDFD0u);
    *(void **)(r5 + 0x1DE8) = _08004DC8((void *)(uintptr_t)0x080CDFD8u);
}
#ifndef __APPLE__
void _0800C1E4(void *r) __attribute__((alias("MenuCtor2_0C1E4")));
void sub_0800C1E4(void *r) __attribute__((alias("MenuCtor2_0C1E4")));
#endif

// ============================================================================
// menu_ctor.s 0x0800C010 — menu/save scene constructor (twin of C1E4 with
// its own template, offsets, and a trailing _08004EF0 publish).
// ============================================================================
// The ROM copies the 16-byte template with `ldmia r0!,{r2,r3,r4} / stmia r1!,
// {r2,r3,r4}` (0x0800C04C), which is agbcc's block move for a whole-struct
// assignment; four scalar word copies do not produce it. The two `1` stores
// and the `*(u16*)(r5+92) = 1` store come out of one hoisted constant, which is
// why the prologue saves r8 AND r9 (`mov r6,r9 / mov r5,r8 / push {r5,r6}`).
void MenuCtor_0C010(void *rec) {
    u8 *r5 = (u8 *)rec;
    u8 stk[84];
    u8 *r4 = stk;
    // Two values of 1 in two different hard registers, exactly as in
    // MenuCtor2_0C1E4 above, and the same closing shape applies unchanged: the
    // r6 pin on `one` plus a *named* halfword constant `k1`. The ROM here is
    // `+0x12 movs r6,#1 / +0x14 movs r0,#1 / +0x16 mov r9,r0 / +0x18 mov r1,r9
    // / +0x1A strh r1,[r4,#2]`, with `one` read back at +0x36 `strb r6,[r4,#4]`
    // and +0x3C `strb r6,[r4,#7]`, and r9 again at the `(u16*)(r5+92)` store.
//
    // Both levers are required and neither is sufficient, measured here
    // independently of the twin: pin only (literal `1u` in the halfword store)
    // reaches prefix 24 with the constant copy-propagated into the scratch
    // instead of `mov r1,r9`; named `k1` only (plain `u8 one`) stays at
    // 219/224 prefix 19. See the full control list on MenuCtor2_0C1E4.
    *(u16 *)(r4 + 0) = 0u;
    register u8 one __asm__("r6");
    one = 1u;
    u16 k1 = 1u;
    *(u16 *)(r4 + 2) = k1;
    _08002E104(stk + 4, 0, 64u);
    *(r4 + 4) = one;
    *(r4 + 6) = 24u;
    *(r4 + 7) = one;
    *(void **)(stk + 16) = r5 + 0xCE0;
    u8 *r8 = stk + 68;
    *(MenuCtor_Tmpl *)(uintptr_t)r8 = *(const MenuCtor_Tmpl *)(uintptr_t)0x0805F6B4u;
    _080056FC();
    _080055D8((u32)(uintptr_t)(r5 + 0x1CE0), 16u);
    *(u16 *)(r5 + 92) = 1u;
    u8 *rr4 = r5 + 8;
    _08002D974(stk, rr4, 0x04000011u);
    u8 *rr6 = r5 + 76;
    _08002D974(r8, rr6, 0x04000004u);
    _08004D4C(2u, (u32)(uintptr_t)rr4, 0u);
    _080050E8(rr4, 0);
    _080032E0(rr6);
    _08003104(r5 + 96);
    // `rr` is formed AFTER the first _08004DC8 call: the ROM computes
    // `adds r4,r5,r2` at 0x0800C0B4, past the `bl` at 0x0800C0AC. Declaring it
    // before the call sinks the address computation ahead of the call instead.
    void *a = _08004DC8((void *)(uintptr_t)0x080CDFD0u);
    u8 *rr = r5 + 0x1DE0;
    *(void **)rr = a;
    *(void **)(r5 + 0x1DE4) = _08004DC8((void *)(uintptr_t)0x080CDFD8u);
    _08004EF0(*(void **)rr);
}
#ifndef __APPLE__
void _0800C010(void *r) __attribute__((alias("MenuCtor_0C010")));
void sub_0800C010(void *r) __attribute__((alias("MenuCtor_0C010")));
#endif

// ============================================================================
// menu_c4d0.s 0x0800C4D0 — menu record-20 constructor: explicit stack-context
// bytes, twin memset blocks, template 0x0805F6D4, C454/C4AC pair, triple
// resource lookup.
// ============================================================================
void MenuCtor20_0C4D0(void *rec) {
    u8 *r5 = (u8 *)rec;
    u8 stk[84];
    u8 *r0 = stk;
    *(u16 *)(r0 + 0) = 0u;
    *(u16 *)(r0 + 2) = 1u;
    r0[4] = 1u; r0[5] = 0u; r0[6] = 29u; r0[7] = 0u;
    r0[8] = 1u; r0[9] = 1u; r0[10] = 0u; r0[11] = 0u;
    *(u16 *)(r0 + 12) = 0u;
    *(u16 *)(r0 + 14) = 0u;
    *(void **)(stk + 16) = r5 + 0xCE4;
    r0[20] = 0u; r0[21] = 3u; r0[22] = 31u; r0[23] = 0u;
    r0[24] = 0u; r0[25] = 1u; r0[26] = 0u; r0[27] = 0u;
    *(u16 *)(r0 + 28) = 0u;
    *(u16 *)(r0 + 30) = 0u;
    *(void **)(stk + 32) = r5 + 0x1CE4;
    _08002E104(stk + 36, 0, 16u);
    _08002E104(stk + 52, 0, 16u);
    u8 *r8 = stk + 68;
    u32 *t = (u32 *)(uintptr_t)0x0805F6D4u;
    u32 *d = (u32 *)r8;
    d[0] = t[0]; d[1] = t[1]; d[2] = t[2]; d[3] = t[3];
    *(r5 + 96) = 1u;
    _080056FC();
    _080055D8((u32)(uintptr_t)(r5 + 0x24E4), 16u);
    *(u16 *)(r5 + 92) = 1u;
    u8 *rr4 = r5 + 8;
    _08002D974(stk, rr4, 0x04000011u);
    u8 *rr6 = r5 + 76;
    _08002D974(r8, rr6, 0x04000004u);
    _08004D4C(2u, (u32)(uintptr_t)rr4, 0u);
    _080050E8(rr4, 0);
    _080032E0(rr6);
    _0800C454();
    _0800C4AC();
    _08003104(r5 + 100);
    *(void **)(r5 + 0x25E4) = _08004DC8((void *)(uintptr_t)0x080CDFD0u);
    *(void **)(r5 + 0x25E8) = _08004DC8((void *)(uintptr_t)0x080CDFD8u);
    *(void **)(r5 + 0x25EC) = _08004DC8((void *)(uintptr_t)0x080CDFE0u);
}
#ifndef __APPLE__
void _0800C4D0(void *r) __attribute__((alias("MenuCtor20_0C4D0")));
void sub_0800C4D0(void *r) __attribute__((alias("MenuCtor20_0C4D0")));
#endif

// ============================================================================
// menu_cae4.s 0x0800CAE4 — menu/results detail renderer: template blit,
// four-column car lanes, template word lane, indexed text lookups.
// ============================================================================
void MenuDetailRender_0CAE4(void *rec) {
    u8 *r7 = (u8 *)rec;
    u8 stk[56];
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    u32 *t28 = (u32 *)(uintptr_t)0x0805F850u;
    u32 *d28 = (u32 *)(stk + 28);
    d28[0] = t28[0]; d28[1] = t28[1]; d28[2] = t28[2];
    d28[3] = t28[3]; d28[4] = t28[4]; d28[5] = t28[5];
    d28[6] = t28[6];
    _08003940(4u, (u32)(uintptr_t)0x0805F86Cu);
    _08003F18(20u, 20u, (const volatile u8 *)(uintptr_t)0x0805F87Cu, (int)(u16)_08002044());
    for (int r6 = 0; r6 <= 3; r6++) {
        int x = r6 * 50 + 50;
        sub_08003954(x, 30u, (int)_0800206C(r6));
        sub_08003954(x, 40u, (int)_08001E5C(r6, 0));
        sub_08003954(x, 50u, (int)_08002178(r6));
    }
    for (int i = 0; i <= 6; i++)
        sub_08003838(100, (u32)(70 + 10 * i),
                     (const volatile u8 *)(uintptr_t)d28[i]);
    int idx = (int)(s16)(*(u16 *)r7);
    sub_08003838(90, (u32)(idx * 10 + 70),
                 (const volatile u8 *)(uintptr_t)0x0805F888u);
    u32 *vec = (u32 *)(uintptr_t)0x080CB2C0u;
    sub_08003838(144, 70u,
                 (const volatile u8 *)(uintptr_t)vec[(int)(s16)(*(u16 *)(wa + 0x576))]);
    sub_08003954(160, 80u, (int)(s16)(*(u16 *)(wa + 0x574)));
    u32 *vec2 = (u32 *)(uintptr_t)0x080CB3B8u;
    sub_08003838(184, 70u,
                 (const volatile u8 *)(uintptr_t)vec2[(int)(s16)(*(u16 *)(wa + 0x113C))]);
    u32 *vec3 = (u32 *)(uintptr_t)0x080CB3C4u;
    sub_08003838(184, 80u,
                 (const volatile u8 *)(uintptr_t)vec3[(int)(s16)(*(u16 *)(wa + 0x113E))]);
    sub_08003954(160, 130u, (int)*(s8 *)(r7 + 29));
    _08002158(0, *(u16 *)r7);
}
#ifndef __APPLE__
void _0800CAE4(void *r) __attribute__((alias("MenuDetailRender_0CAE4")));
void sub_0800CAE4(void *r) __attribute__((alias("MenuDetailRender_0CAE4")));
#endif

// ============================================================================
// menu_c884.s 0x0800C884 — menu record handler: key-scan gate (provably
// always r6=0 — transcribed literally regardless), continue-path packet,
// bit-lane transitions, cursor walks, WA counter lanes, final HSL dispatch
// to the _08002158 packet sink.
// ============================================================================
void MenuRecordHandler_0C884(void *rec, u32 a1, u32 a2) {
    u8 *r5 = (u8 *)rec;
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    u16 r9 = (u16)a1, r7 = (u16)a2;
    u16 sl = _080024A0();
    int r8 = 0, r6 = 1;
    (void)_08004B68();
    if (_08002140() > 1u) {
        for (u32 r4 = 0; r4 < _08002140(); r4++) {
            if (_08002178((int)r4) != 5u) r6 = 0;
            if (((u32)_08002178((int)r4) << 16) != 0u) r6 = 0;
        }
    } else {
        r6 = 0;
    }
    if (r6 == 0) {
        if (r7 == 1u && *(u16 *)(r5 + 0) == 6u) {
            _08004BFC((int)(s16)(*(u16 *)r5) + (int)*(s8 *)(r5 + 29));
            _08004EC0(1);
            goto c982;
        }
        if (r7 & 1u) {
            int w = (int)(s16)(*(u16 *)r5);
            if (w != 5 && w != 1) {
                if (r9 & 0x200u) _08004BFC(100);
                else if (r9 & 0x100u) _08004BFC(101);
                else _08004BFC(w);
                _08004C84(0u, (u32)(u16)w);
                _08001F80(_08002044());
                _08004EC0(1);
            }
        }
    } else {
        if ((int)(s16)(*(u16 *)(r5 + 4)) > 0)
            *(u16 *)(r5 + 4) = (u16)((u32)*(u16 *)(r5 + 4) - 1u);
        else
            _0800C814(r5);
        goto c990;
    }
c982:
    if (r7 & 2u) _08004EA8(1);
c990:
    if (r7 & 64u) {
        if ((int)(s16)(*(u16 *)r5) > 0)
            *(u16 *)r5 = (u16)((u32)*(u16 *)r5 - 1u);
    }
    if (r7 & 128u) {
        if ((int)(s16)(*(u16 *)r5) <= 5)
            *(u16 *)r5 = (u16)((u32)*(u16 *)r5 + 1u);
    }
    if (sl & 32u) r8--;
    if (sl & 16u) r8++;
    if (r7 & 512u) {
        if ((int)(s16)(*(u16 *)r5) == 0) {
            u16 *cell = (u16 *)(wa + 0x113C);
            *cell = (u16)((u32)*cell + 1u);
            if ((int)(s16)*cell > 2) *cell = 0u;
        }
    }
    if (r7 & 256u) {
        if ((int)(s16)(*(u16 *)r5) == 0) {
            u16 *cell = (u16 *)(wa + 0x113E);
            *cell = (u16)((u32)*cell + 1u);
            if ((int)(s16)*cell > 2) *cell = 0u;
        }
    }
    switch ((int)(s16)(*(u16 *)r5)) {
    case 0: {
        u16 *cell = (u16 *)(wa + 0x576);
        *cell = (u16)((u32)*cell + (u32)r8);
        if ((int)(s16)*cell < 0) *cell = 0u;
        break;
    }
    case 1: {
        u16 *cell = (u16 *)(wa + 0x574);
        *cell = (u16)((u32)*cell + (u32)r8);
        if ((int)(s16)*cell < 0) *cell = 0u;
        if ((int)(s16)*cell > 96) *cell = 96u;
        break;
    }
    case 6: {
        u8 nv = (u8)((u32)*(r5 + 29) + (u32)r8);
        *(r5 + 29) = nv;
        if ((s8)nv < 0) *(r5 + 29) = 0u;
        if ((s8)*(r5 + 29) > 4) *(r5 + 29) = 4u;
        break;
    }
    default: break;
    }
    _08002158(0, r9);
    _08002158(1, r7);
    _08002158(2, *(u16 *)r5);
    _08002158(3, *(r5 + 28));
}
#ifndef __APPLE__
void _0800C884(void *r, u32 a, u32 b) __attribute__((alias("MenuRecordHandler_0C884")));
void sub_0800C884(void *r, u32 a, u32 b) __attribute__((alias("MenuRecordHandler_0C884")));
#endif

// ============================================================================
// course_resource_leaf_more.s 0x08007B18 / more2.s 0x08007B8C — record
// emitters over the course-record array (kind feeds Course_Seek; r1-in to
// the seek is the only use of the kind slot... precisely: kind -> 07498).
// The bodies themselves are defined above, next to the emitter helpers they
// share; only the VMA aliases live here.
// ============================================================================
#ifndef __APPLE__
void _08007B18(void *r, int k, int x, int y, u32 a, u32 b, u32 c, u32 d)
    __attribute__((alias("CourseEmit7B18")));
void _08007B8C(void *r, int k, int x, int y, u32 a, u32 b, u32 c, u32 d)
    __attribute__((alias("CourseEmit7B8C")));
#endif

// ============================================================================
// menu_ff78.s 0x080011784 — record filler dispatcher based on s16[rec+168].
// ROM call sites here are `bl 0x08007B18` (0x117CC/0x117E6/0x11802/0x1182E/
// 0x11848), so the body calls the strong CourseEmit7B18 rather than the
// file-local CourseEmit7B18_Static: a file-local callee has no symbol in the
// scoped link closure, which is what produced UNRESOLVED_RELOCATION (5 sites).
// ============================================================================
void Sub_080011784(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 selector = *(volatile s16 *)(uintptr_t)(rec + 168);

    switch (selector) {
    case 0: {
        u16 val = *(volatile u16 *)(uintptr_t)(rec + 214);
        if (val >= 5 && val <= 6) {
            CourseEmit7B18(rec_, 6, 10, 40, 4, 1, 1, selector);
        }
        break;
    }
    case 1:
        CourseEmit7B18(rec_, 7, 10, 56, 4, selector, selector, 0);
        break;
    case 2:
        CourseEmit7B18(rec_, 8, 10, 72, 4, 1, 1, 0);
        break;
    case 3: {
        s16 sub_selector = *(volatile s16 *)(uintptr_t)(rec + 226);
        if (sub_selector == 0) {
            CourseEmit7B18(rec_, 11, 10, 88, 4, 1, 1, sub_selector);
        } else if (sub_selector == 1) {
            CourseEmit7B18(rec_, 5, 10, 88, 4, sub_selector, sub_selector, 0);
        }
        break;
    }
    default:
        break;
    }
}
#ifndef __APPLE__
void sub_080011784(void *a) __attribute__((alias("Sub_080011784")));
void _080011784(void *a) __attribute__((alias("Sub_080011784")));
#else
void sub_080011784(void *a) { Sub_080011784(a); }
void _080011784(void *a) { Sub_080011784(a); }
#endif

// ============================================================================
// Static faithful _08007770 course-resource setup (course_resource_helpers.s
// 0x08007770). Kept as a file-local copy of the now-faithful
// Course_0x08007770: 6 machine arguments; source 053DC is
// 0748C(r7) (not r3in), DMA base 0x05000000.
// ============================================================================
static void CourseSetup7770_Static(void *ctx, void *X, int r2in, int r3in,
                                   u32 s0, u32 s1) {
    void *r4 = _08007498(X, r2in);
    void *r5 = _08007498(X, 0);
    void *r6 = _08007498(r4, 1);
    void *r7 = _08007498(r4, 2);
    u8 *r8 = (u8 *)_0800748C(r4);
    int r9 = _080050D0((int)(uintptr_t)ctx, (int)RomLeaf7658(r5));
    void *r4b = _0800748C(r5);
    _08005260(ctx, r4b, r9, (int)_080074A8(r5)); // r3 = 074A8(r5) per ROM asm
    const volatile u8 *src = (const volatile u8 *)_0800748C(r7);
    u16 r8w0 = *(volatile u16 *)(r8 + 0);
    u16 r8w2 = *(volatile u16 *)(r8 + 2);
    int stk0 = (int)(u16)(r8w0 >> 3);
    int stk1 = (int)(u16)(r8w2 >> 3);
    _080053DC((int)(uintptr_t)ctx, src, r3in, (int)s0, stk0, stk1, r9, (int)s1);
    volatile u32 *dma = (volatile u32 *)(uintptr_t)0x040000D4u;
    dma[0] = (u32)(uintptr_t)_0800748C(r6);
    dma[1] = 0x05000000u + (s1 << 5);
    dma[2] = (_080074A8(r6) >> 2) | 0x84000000u;
    (void)dma[2];
    _080054A4((int)(uintptr_t)ctx, r3in, (int)s0, stk0, stk1);
}

// projects the query delta (05DA4), clamps to the 2-attempt entry list and
// runs the half-plane + tile-range acceptance test (05E14), publishing the
// hit into the out record. Returns 1 on accept, 0 otherwise.
// ============================================================================
int CourseProximity_07210(void *state, void *out) {
    u8 *r7 = (u8 *)out;
    u8 *r8 = (u8 *)_08006C10(state, 0);
    if (*(u32 *)(r7 + 8) == 0u && *(u32 *)(r7 + 12) == 0u) return 0;
    s32 v24 = *(s32 *)(r7 + 0) - *(s32 *)(r8 + 12);
    s32 v28 = *(s32 *)(r7 + 4) - *(s32 *)(r8 + 16);
    s32 v32 = *(s32 *)(r7 + 8);
    s32 v36 = *(s32 *)(r7 + 12);
    u8 tmp[12], arg[16], slot[8];
    *(s32 *)(arg + 0) = v24; *(s32 *)(arg + 4) = v28;
    *(s32 *)(arg + 8) = v32; *(s32 *)(arg + 12) = v36;
    _08005DA4(tmp, arg);
    s32 v52 = v24 + v32;
    s32 r5 = v52;
    s32 v56 = v28 + v36;
    s32 r6 = v56;
    if (r5 > v24) r5 = v24;
    if (r6 > v28) r6 = v28;
    if (v52 < v24) v52 = v24;
    if (v56 < v28) v56 = v28;
    for (int attempt = 0; attempt < 2; attempt++) {
        u32 off = attempt ? 28u : 0u;
        u8 *ent = r8 + 40 + off;
        if ((((u32)_08005E14(slot, tmp, ent)) << 24) != 0u) {
            s32 dx = v24 - *(s32 *)(r8 + 32 + off);
            s32 dy = v28 - *(s32 *)(r8 + 36 + off);
            *(s32 *)(r7 + 0) = dx;
            *(s32 *)(r7 + 4) = dy;
            u32 prod = (u32)dx * (u32)*(s32 *)ent
                     + (u32)dy * (u32)*(s32 *)(ent + 4);
            s32 m1 = (s32)prod;
            s32 r3 = *(s32 *)slot;
            s32 s44 = *(s32 *)(slot + 4);
            if (m1 < 0) {
                if (r5 <= r3) continue;
                if (r6 <= s44) continue;
                if (v52 < r3) continue;
                if (v56 < s44) continue;
                s32 shr3 = r3 >> 8;
                s32 shr1 = s44 >> 8;
                if ((int)(s16)(*(u16 *)(ent + 12)) > shr3) continue;
                if ((int)(s16)(*(u16 *)(ent + 14)) > shr1) continue;
                if ((int)(s16)(*(u16 *)(ent + 16)) < shr3) continue;
                if ((int)(s16)(*(u16 *)(ent + 18)) < shr1) continue;
            }
            *(s32 *)(r7 + 0) = r3 + *(s32 *)(r8 + 12);
            *(s32 *)(r7 + 4) = s44 + *(s32 *)(r8 + 16);
            *(s32 *)(r7 + 8) = *(s32 *)ent;
            *(s32 *)(r7 + 12) = *(s32 *)(ent + 4);
            return 1;
        }
    }
    return 0;
}
#ifndef __APPLE__
int _08007210(void *s, void *o) __attribute__((alias("CourseProximity_07210")));
int sub_08007210(void *s, void *o) __attribute__((alias("CourseProximity_07210")));
#endif

// ============================================================================
// code_25264.s 0x08025264 — lineup builder: collects eligible car ids
// (skipping the one-make-cup reserves and unmapped ids), shuffles by
// signed-remainder indexing (another DivRemS consumer), insertion-ranks by
// catalog scores, and publishes picks + DivRem categories.
// ============================================================================
// ROM 0x080253DC and 0x08025430 call `bl 0x08025518` / `bl 0x08025530`
// DIRECTLY from the 0x08025264 body -- there is no helper function in the ROM
// for the score sum. A file-local `line_score` helper was therefore the wrong
// shape: it emitted a TU-local `bl line_score` that the scoped link closure
// cannot resolve (2 sites, UNRESOLVED_RELOCATION) in place of the two real
// `bl`s. Removed; both use sites now call the ROM callees directly.

void AiLineupBuilder_25264(int a0) {
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    u16 r4 = (u16)a0;
    u8 stk[196];
    u8 frame[16];
    _08002E104(stk, 0, 196u);
    _08002E104(frame, 0, 16u);
    int look = _080024E54((s16)r4);
    u8 *r6 = (u8 *)(uintptr_t)(u32)look;
    u16 r7 = 0u;
    for (; (s16)(*(u16 *)r6) != -1; r6 += 2) {
        u16 v = *(u16 *)r6;
        int vs = (s16)v;
        if (vs == 86 || vs == 56 || vs == 55 || vs == 31 || vs == 14
                || vs == 98 || vs == 54)
            continue;
        if ((s16)_080022E4(vs) == -1) continue;
        *(u16 *)(stk + (u16)r7 * 2u) = v;
        r7 = (u16)(r7 + 1u);
    }
    int cnt = (s16)r7;
    int total = (int)(s16)(*(u16 *)(wa + 0x10CA)) - 1;
    if (cnt < total) {
        int r8 = cnt;
        int rv = cnt;
        do {
            int idx = DivRemS_02DE9C(rv, r8);
            *(u16 *)(stk + (u16)rv * 2u) = *(u16 *)(stk + (u16)idx * 2u);
            rv++;
        } while ((s16)(u16)rv < total);
    }
    int n = (s16)r7;
    if (n > 0) {
        int r5 = n;
        u16 r6c = 0u;
        do {
            int q = DivRemS_02DE9C(_08005B5C((int)_08008014()), r5);
            int i = (s16)r6c;
            u16 a = *(u16 *)(stk + (u16)i * 2u);
            u16 b = *(u16 *)(stk + (u16)q * 2u);
            *(u16 *)(stk + (u16)i * 2u) = b;
            *(u16 *)(stk + (u16)q * 2u) = a;
            r6c = (u16)(i + 1);
        } while ((s16)r6c < r5);
    }
    *(u16 *)(frame + 0) = *(u16 *)(stk + 0);
    total = (int)(s16)(*(u16 *)(wa + 0x10CA)) - 1;
    for (int r6o = 1; (s16)r6o < total;) {
        int lim = (s16)r6o;
        int best = 0;
        if (best < lim) {
            for (;;) {
                int bj = (s16)best;
                int bestscore = (int)_08025518((s16)(*(u16 *)(frame + (u16)bj * 2u)))
                              + (int)_08025530((s16)(*(u16 *)(frame + (u16)bj * 2u)));
                u16 cand = *(u16 *)(stk + (u16)lim * 2u);
                if ((int)_08025518((s16)cand) + (int)_08025530((s16)cand) >= bestscore) {
                    if (lim > bj) {
                        for (int k = lim; k > bj; k--)
                            *(u16 *)(frame + (u16)k * 2u) =
                                *(u16 *)(frame + (u16)(k - 1) * 2u);
                    }
                    *(u16 *)(frame + (u16)bj * 2u) = cand;
                    break;
                }
                best++;
                if ((s16)best >= lim) {
                    *(u16 *)(frame + (u16)best * 2u) = cand;
                    break;
                }
            }
        }
        total = (int)(s16)(*(u16 *)(wa + 0x10CA)) - 1;
        r6o = (u16)(lim + 1);
        if (!((s16)r6o < total)) break;
    }
    for (int oi = 1;;) {
        int total3 = (int)(s16)(*(u16 *)(wa + 0x10CA));
        if (oi >= total3) break;
        int off = (s16)oi * 2;
        u16 pick = *(u16 *)(frame + (off - 2));
        *(u16 *)(wa + 0x1DD4 + (u16)off) = pick;
        int sv = _08005B5C((int)_08008014());
        int cat = (int)_080254E8((int)(s16)pick);
        *(u16 *)(wa + 0x1DE4 + (u16)off) = (u16)_08002D97C(sv, cat);
        oi++;
    }
}
#ifndef __APPLE__
void _08025264(int a) __attribute__((alias("AiLineupBuilder_25264")));
void sub_08025264(int a) __attribute__((alias("AiLineupBuilder_25264")));
#endif

// ============================================================================
// code_23628.s 0x08023628 — race scene constructor: sound select, twin
// resource setups, record wiring, award-grid alloc triple, car-record copy
// loop (35×72 B via CpuSet), tail flags.
// ============================================================================
void RaceSceneCtor_23628(void *rec) {
    u8 *r7 = (u8 *)rec;
    _08002B214(51);
    u8 *r8 = (u8 *)(uintptr_t)0x0839F5FCu;
    CourseSetup7770_Static((void *)(uintptr_t)0u, r8, 0, 0, 4u, 1u);
    u8 *r5 = (u8 *)(uintptr_t)0x082B7410u;
    CourseSetup7770_Static((void *)(uintptr_t)1u, r5, 3, 0, 0u, 3u);
    *(u32 *)(r7 + 88) = 6u;
    *(u32 *)(r7 + 100) = 21u;
    *(u32 *)(r7 + 112) = 1u;
    u8 *sl = r7 + 16;
    _0800DAB8(sl);
    _08007614((void *)(uintptr_t)0x082A798Cu, 1, 0, 4);
    _08007614((void *)(uintptr_t)0x082A798Cu, 1, 1, 5);
    _0800798C(r8, r7);
    _08007A58(r7);
    _0800798C(r5, r7 + 8);
    _080075E8(r5, 0, 6);
    u8 *r4 = r7 + 196;
    _080025BC8(r4, 5);
    _08007ABC((void *)(uintptr_t)*(u32 *)(r7 + 12),
              (void *)(uintptr_t)*(u32 *)(r7 + 200), *(u32 *)(r4 + 0));
    int a8 = _0800572C(8);
    *(u32 *)(r7 + 208) = (u32)a8;
    *(u32 *)(r7 + 212) = 22u;
    _08007ABC((void *)(uintptr_t)*(u32 *)(r7 + 12), (void *)(uintptr_t)22u,
              (u32)a8);
    int a8b = _0800572C(8);
    *(u32 *)(r7 + 220) = (u32)a8b;
    *(u32 *)(r7 + 224) = 21u;
    _08007ABC((void *)(uintptr_t)*(u32 *)(r7 + 12), (void *)(uintptr_t)21u,
              (u32)a8b);
    *(void **)(r7 + 128) = r4 - 28;
    *(void **)(r7 + 132) = sl;
    *(u32 *)(r7 + 16) = 0u;
    *(u16 *)(r7 + 20) = 1u;
    _0800D77C(r7 + 36, 0, -32);
    _0800D77C(r7 + 28, 0, 160);
    *(u16 *)(r7 + 24) = 6u;
    *(u16 *)(r7 + 26) = 5u;
    *(void **)(r7 + 148) = (void *)(uintptr_t)0x02024EC0u;
    *(void **)(r7 + 152) = (void *)(uintptr_t)0x02020000u;
    u32 dst = 0x02024EC0u;
    for (int r3 = 0; r3 <= 34; r3++) {
        u8 *src = (u8 *)(uintptr_t)(0x03001D64u + (u32)r3 * 72u);
        for (int k = 0; k < 5; k++) {
            _08002D974(src, (void *)(uintptr_t)dst, 0x04000003u);
            dst += 12u;
            src += 12;
        }
        _08002D974((void *)(uintptr_t)(0x03001DA0u + (u32)r3 * 72u),
                   (void *)(uintptr_t)dst, 0x04000003u);
        dst += 12u;
    }
    *(u32 *)(r7 + 168) = 2u;
    *(u16 *)(r7 + 142) = 0u;
    *(u16 *)(r7 + 156) = 0u;
    *(u16 *)(r7 + 158) = 0u;
    *(u16 *)(r7 + 160) = 0u;
    *(u16 *)(r7 + 146) = 0u;
    _08002124(0x138D);
    *(u32 *)(r7 + 240) = 1u;
}
#ifndef __APPLE__
void _08023628(void *r) __attribute__((alias("RaceSceneCtor_23628")));
void sub_08023628(void *r) __attribute__((alias("RaceSceneCtor_23628")));
#endif

// ============================================================================
// ai_racefsm.s 0x0800AA40 — per-frame race/AI control FSM: car-index
// sampling, grid aggregates, event-ring reset, 8-way phase dispatch
// (countdown / GO / running / lap-check / results). Sub-dispatch notes:
// slot32==10 gate, place-change events 38/43/49, award packets.
// ============================================================================
// Shared lap/progress tail of race-FSM cases 5 and 7-sub1 (AD00..AD32).
// NOTE: the place byte is wa+0x10E5 (== 0x03002865); every ROM site reaches
// it as pool literal 0x000010E5 added to the 0x03001780 base (e.g. the
// 0x0800AB78 and 0x0800AD2C literals). Do NOT write wa+0x2865 here.
static void race_lap_tail(void) {
    _080023FF8(21, 0);
    _0800B9F0();
    _080023FF8(35, 0);
}
static void race_progress_check(int c, u8 *wa) {
    int r4 = (int)(s16)_08002581C(c);
    int ru = (int)(u8)_080025E1C(r4);
    int rb = (int)(s8)*(wa + 0x10E5);
    if (ru < rb) {
        _080025DBC(r4, rb);
        _0800279C(1);
    }
    race_lap_tail();
}
void RaceFSM_AA40(void) {
    s32 stk0, stk4, slot32;
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    int a = (int)(s16)(*(u16 *)(wa + 0xFF2));
    int b = (int)(s16)(*(u16 *)(wa + 0xFF8));
    int idx = b * 2 + a * 8;
    slot32 = (s32)(s16)(*(u16 *)(wa + 0x100A + idx));
    int r8v = (int)(s16)(*(u16 *)(wa + 0x102A + idx));
    int r9v = (int)(s16)(*(u16 *)(wa + 0x101A + idx));
    stk0 = (s32)_080026F8(a, b);
    s32 sl = (s32)_08002730(a, b);
    stk4 = (s32)_0800A9E0(a, b);
    int r5 = _0800A9A0(a, b);
    _080023FE4();
    int phase = (int)(s16)(*(u16 *)(wa + 0xFBC));
    if ((u32)phase > 7u) return;
    switch (phase) {
    case 0: {
        _080023FF8(21, 0);
        u8 place = *(wa + 0x10E5);
        if ((u8)(place - 1u) > 2u) break;
        if (slot32 == 10 && *(u16 *)(wa + 0xFF8) == 3u)
            *(u16 *)(wa + 0x1074) = 1u;
        _0800B4A8();
        if (slot32 == 2) {
            if (r8v == 0) goto ev43;
        } else if (r5 > sl && r8v == 1) {
            goto ev43;
        }
        if (slot32 == 10) {
            if (r9v == 0) goto finish_a;
        } else if (stk4 > stk0 && r9v == 1) {
            goto finish_a;
        }
        if (*(u16 *)(wa + 0x1074) != 0u) goto common0;
        if (*(u16 *)(wa + 0xFF8) != 3u) goto ev38a;
        *(u16 *)(wa + 0x1074) = 1u;
        goto finish_b;
ev38a:
        _080023FF8(38, 0);
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(31);
        return;
finish_a:
        if (*(u16 *)(wa + 0x1074) != 1u) goto common0;
finish_b:
        _080023FF8(38, 0);
        sub_08004CC4();
        sub_08004CD4(49);
        _0800B0BC();
        return;
ev43:
        _080023FF8(43, 0);
common0:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(31);
        sub_08004CD4(15);
        return;
    }
    case 1:
        _080023FF8(21, 0);
        if (*(u16 *)(wa + 0xFC8) == 1u) {
            _0800279C(1);
            _080023FF8(25, 0);
        }
        _080023FF8(35, 0);
        _08024144();
        return;
    case 5: {
        u8 place = *(wa + 0x10E5);
        if ((u8)(place - 1u) > 2u) {
            race_lap_tail();
            return;
        }
        race_progress_check((int)(s16)(*(u16 *)(wa + 0x576)), wa);
        return;
    }
    case 2:
        _080023FF8(21, 0);
        _0800B89C();
        if (*(u16 *)(wa + 0xFC8) == 1u) {
            _0800279C(1);
            _080023FF8(25, 0);
        }
        _080023FF8(35, 0);
        return;
    case 3:
        _080023FF8(21, 0);
        _080023FF8(35, 0);
        return;
    case 7: {
        int v = (int)(s16)(*(u16 *)(wa + 0x1078));
        if (v == 1) {
            u8 place = *(wa + 0x10E5);
            if ((u8)(place - 1u) > 2u) {
                race_lap_tail();
                return;
            }
            race_progress_check((int)(s16)(*(u16 *)(wa + 0x576)), wa);
            return;
        }
        if (v == 2) {
            _080023FF8(21, 0);
            if (*(u16 *)(wa + 0xFC8) == 1u) {
                _0800279C(1);
                _080023FF8(25, 0);
            }
            _080023FF8(35, 0);
            _08024144();
            return;
        }
        return;
    }
    default:
        return;
    }
}
#ifndef __APPLE__
void _0800AA40(void) __attribute__((alias("RaceFSM_AA40")));
void sub_0800AA40(void) __attribute__((alias("RaceFSM_AA40")));
void Ai_RaceFsm(void) __attribute__((alias("RaceFSM_AA40"))); /* trampoline elimination: friendly-name spelling used by race_progress.c */
#endif

// Trampoline decls for the six course-record emitters used by 8CA0. The
// existing C bodies are hollow/partial (writes voided, void returns), so the
// phase machine calls the ROM bodies directly. All take <=2 reg args with
// r3 dead on entry (verified per body) — trampoline-safe (only r3 dies).
extern void *RomEmit8AAC(void *r7);          // 0x08008AAC
extern void *RomEmit8BB8(void *r7);          // 0x08008BB8
extern void *RomEmit8768(void *r7, int w);   // 0x08008768
extern void *RomEmit88B0(void *r7, int w);   // 0x080088B0
extern void *RomEmit861C(void *r7, int w);   // 0x0800861C
extern void *RomEmit89FC(void *r7);          // 0x080089FC
extern u32 RomLeaf7658(void *a);             // 0x08007658

static u8 *phase_case148(u8 *r7);
static u8 *phase_case3(u8 *r7);
static u8 *phase_case5(u8 *r7);

// Case 5 short burst + random/award blocks, flowing into the shared
// [32]/[28]/[80]/[24] tail (941E) which no other case reaches.
static u8 *phase_case5(u8 *r7) {
    u8 *cc = *(u8 **)(uintptr_t)0x030003E0u;
    *(u16 *)(r7 + 0) = 2u;
    *(u16 *)(r7 + 2) = 0x40CDu;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                             | (u32)(u16)(*(u16 *)(cc + 48)));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 220u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52)) + 24u))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 227u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52))
                             + (u16)((u16)(*(u16 *)(cc + 10)) * 2u)))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    u16 lane = *(u16 *)(cc + 38);
    CourseEmit7B18(cc + 88, 14, 184, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 208, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 224, 4, lane, 1u, 0u, 0u);
    int c26 = (int)(s16)*(u16 *)(cc + 26);
    if (c26 > 0) {
        int rr = _08002BE8(0);
        u16 nc26 = (u16)(*(u16 *)(cc + 26) - 1u);
        *(u16 *)(cc + 26) = nc26;
        int r5s = (s32)rr;
        u16 trow = *(u16 *)(uintptr_t)(0x080CB154u + (u32)(u16)(nc26 & 15u) * 2u);
        _08002C48(r5s, trow);
        int c18 = (int)(s16)*(u16 *)(cc + 18);
        if (c18 > 9) {
            *(u16 *)(r7 + 0) = 0x386u;
            *(u16 *)(r7 + 2) = (u16)(((u32)(u16)r5s << 9) | 0x4044u);
            int dq = _08002DE04(c18, 10);
            *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 40))
                                           + (u32)(dq * 4))
                                     | ((u32)(u16)(*(u16 *)(cc + 38)) << 12));
            r7 += 8;
        }
        *(u16 *)(r7 + 0) = 0x386u;
        *(u16 *)(r7 + 2) = (u16)(((u32)(u16)r5s << 9) | 0x4050u);
        int c18b = (int)(s16)*(u16 *)(cc + 18);
        int dm = DivRemS_02DE9C(c18b, 10);
        *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 40))
                                       + (u32)(dm * 4))
                                 | ((u32)(u16)(*(u16 *)(cc + 38)) << 12));
        r7 += 8;
    } else {
        int c18 = (int)(s16)*(u16 *)(cc + 18);
        if (c18 > 9) {
            *(u16 *)(r7 + 0) = 142u;
            *(u16 *)(r7 + 2) = 0x404Cu;
            int dq = _08002DE04(c18, 10);
            *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 40))
                                           + (u32)(dq * 4))
                                     | ((u32)(u16)(*(u16 *)(cc + 38)) << 12));
            r7 += 8;
        }
        *(u16 *)(r7 + 0) = 142u;
        *(u16 *)(r7 + 2) = 0x4058u;
        int c18b = (int)(s16)*(u16 *)(cc + 18);
        int dr = DivRemS_02DE9C(c18b, 10);
        *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 40))
                                       + (u32)(dr * 4))
                                 | ((u32)(u16)(*(u16 *)(cc + 38)) << 12));
        r7 += 8;
    }
    // [crec+32] conditional ingen record
    {
        u16 c32 = *(u16 *)(cc + 32);
        if (c32 != 0u && c32 <= 16u) {
            *(u16 *)(r7 + 0) = 1152u;
            *(u16 *)(r7 + 2) = 0x40A0u;
            *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                                          | (u32)(u16)(*(u16 *)(cc + 42)));
            r7 += 8;
            *(u16 *)(uintptr_t)0x04000050u = 1856u;
            *(u16 *)(uintptr_t)0x04000052u =
                (u16)(((16u - (u32)c32) << 8) | (u32)c32);
        }
    }
    // [crec+28] countdown block
    {
        u16 c28u = *(u16 *)(cc + 28);
        if ((int)(s16)c28u > 0) {
            *(u16 *)(cc + 28) = (u16)(c28u - 1u);
            u8 nb79 = (u8)(*(cc + 79) + 1u);
            *(cc + 79) = nb79;
            if (nb79 > 15u) *(cc + 79) = 1u;
            _08007614((void *)(uintptr_t)0x0828A35Cu, 1, (int)*(cc + 79),
                      (int)*(u16 *)(cc + 58));
            u8 *entry = (u8 *)(uintptr_t)(0x080CB074u
                        + (u32)(s32)((s32)(s16)(*(u16 *)(cc + 30)) * 8));
            int e2 = (int)(s16)*(u16 *)(entry + 2);
            int c28 = (int)(s16)*(u16 *)(cc + 28);
            int d = 60 - c28;
            int dd = (d + (int)((u32)d >> 31)) >> 1;
            CourseIter7BFC_Static(cc + 96, (int)*(u16 *)(cc + 62),
                                  (int)(s16)*(u16 *)(entry + 0), e2 - dd,
                                  (u32)*(u16 *)(cc + 58), 0u, 0u, 0u);
        }
    }
    // [cc+80/84] stamp + [24] tail switch
    *(u32 *)(cc + 80) = 168u;
    *(u32 *)(cc + 84) = 80u;
    {
        int w24 = (int)(s16)(*(u16 *)(cc + 24));
        if (w24 == 2) {
            u8 nb = (u8)(*(cc + 79) + 1u);
            *(cc + 79) = nb;
            if (nb > 15u) *(cc + 79) = 1u;
            _08007614((void *)(uintptr_t)0x0828A35Cu, 1, (int)*(cc + 79),
                      (int)*(u16 *)(cc + 58));
            r7 = (u8 *)RomEmit89FC(r7);
        } else if (w24 == 1) {
            u16 c22 = *(u16 *)(cc + 22);
            *(u16 *)(cc + 22) = (u16)(c22 - 1u);
            int idx = (int)(s16)*(u16 *)(cc + 22);
            int tv = (int)(s16)*(u16 *)(uintptr_t)(0x080CB0CCu + (u32)(u16)idx * 2u);
            *(u32 *)(cc + 80) = (u32)(tv + 168);
            *(u32 *)(cc + 84) = 80u;
            if ((s32)((u32)(u16)(c22 - 1u) << 16) <= 0)
                *(u16 *)(cc + 24) = 2u;
            r7 = (u8 *)RomEmit8AAC(r7);
        } else if (w24 == 3) {
            u16 c22 = *(u16 *)(cc + 22);
            *(u16 *)(cc + 22) = (u16)(c22 - 1u);
            int idx = (int)(s16)*(u16 *)(cc + 22);
            int tv = (int)(s16)*(u16 *)(uintptr_t)(0x080CB0EEu
                       + (u32)(u16)(16 - idx) * 2u);
            *(u32 *)(cc + 80) = (u32)(tv + 168);
            *(u32 *)(cc + 84) = 80u;
            if ((s32)((u32)(u16)(c22 - 1u) << 16) <= 0)
                *(u16 *)(cc + 24) = 0u;
            r7 = (u8 *)RomEmit8BB8(r7);
        }
    }
    return r7;
}

void CoursePhaseMachine_08CA0(void) {
    u8 tmp[80];
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    u8 *r7 = (u8 *)(uintptr_t)_08003130();
    u8 *crec = *(u8 **)(uintptr_t)0x030003E0u;
    int t = (int)(s16)(*(u16 *)(crec + 2));
    int n = _08003D18(_08002DE04(100 * t, 160), tmp + 20);
    if (n > 0) {
        u8 *line = tmp + 35 - (u32)n;
        int col = 175 - 7 * n;
        u16 v = 0x808Fu;
        do {
            *(u16 *)(r7 + 0) = v;
            *(u16 *)(r7 + 2) = (u16)((u32)col & 0x1FFu);
            int d = (int)*line - 48;
            int e = (d << 1) + (int)(u16)(*(u16 *)(crec + 52));
            int f = (int)(u16)(*(u16 *)(crec + 54));
            *(u16 *)(r7 + 4) = (u16)(e | (f << 12));
            r7 += 8;
            line++;
            col += 7;
            n--;
        } while (n != 0);
    }
    *(u16 *)(r7 + 0) = 0x808Fu;
    *(u16 *)(r7 + 2) = 202u;
    {
        int g = (int)(u16)(*(u16 *)(crec + 6));
        int h = (g << 1) + (int)(u16)(*(u16 *)(crec + 52));
        int k = (int)(u16)(*(u16 *)(crec + 54));
        *(u16 *)(r7 + 4) = (u16)(h | (k << 12));
    }
    r7 += 8;
    *(u32 *)(tmp + 36) = *(u32 *)(uintptr_t)0x0805F634u;
    *(u32 *)(tmp + 40) = *(u32 *)(uintptr_t)(0x0805F634u + 4u);
    *(u32 *)(tmp + 44) = *(u32 *)(uintptr_t)0x0805F63Cu;
    *(u32 *)(tmp + 48) = *(u32 *)(uintptr_t)(0x0805F63Cu + 4u);
    *(u32 *)(tmp + 52) = *(u32 *)(uintptr_t)0x0805F644u;
    *(u32 *)(tmp + 56) = *(u32 *)(uintptr_t)(0x0805F644u + 4u);
    *(u32 *)(tmp + 60) = *(u32 *)(uintptr_t)0x0805F64Cu;
    *(u32 *)(tmp + 64) = *(u32 *)(uintptr_t)(0x0805F64Cu + 4u);
    int c0 = (int)(s16)(*(u16 *)(crec + 0));
    if (c0 < 0) c0 += 3;
    u32 t14 = (u32)c0 << 14;
    s32 s14 = (s32)t14 >> 16;
    u32 ang = t14 >> 16;
    if (s14 <= 111) ang = 112u;
    ang = (ang << 20) >> 20;
    _08005BA8(tmp + 36, (int)ang);
    _08005BA8(tmp + 68, (int)ang);
    _08005BA8(tmp + 52, (int)ang);
    _08005BA8(tmp + 60, (int)ang);
    u32 w52 = *(u32 *)(tmp + 52);
    *(u32 *)(tmp + 72) = (u32)(u16)(w52 + 199u);
    u32 w56 = *(u32 *)(tmp + 56);
    u32 w60 = *(u32 *)(tmp + 60);
    u32 w64 = *(u32 *)(tmp + 64);
    *(u32 *)(tmp + 76) = (u32)(u16)(w64 + 138u);
    int rnd = _08002BE8(0);
    u16 r9b = (u16)rnd;
    u32 p68 = *(u32 *)(tmp + 68);
    int sx = (int)(s16)*(u16 *)(uintptr_t)(p68 + 4);
    *(u32 *)(tmp + 0) = (u32)(s32)sx;
    _08002D98((u32)r9b, (u32)(s32)(s16)(u16)(*(u32 *)(tmp + 36)),
              (u32)(s32)(s16)(u16)(*(u32 *)(tmp + 40)),
              (u32)(s32)(s16)(u16)(*(u32 *)(tmp + 44)), (u32)(s32)sx);
    u32 f2a = ((u32)(s32)(s16)(u16)(*(u32 *)(tmp + 72)) & 0x1FFu)
            | (((u32)r9b << 9) | 0x4000u);
    u32 recA0 = ((u32)(u16)(w56 + 134u) & 255u) | 256u;
    *(u16 *)(r7 + 0) = (u16)recA0;
    *(u16 *)(r7 + 2) = (u16)f2a;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(crec + 38)) << 12)
                           | (u32)(u16)(*(u16 *)(crec + 44)));
    r7 += 8;
    u32 r9s = (u32)r9b << 9;
    *(u16 *)(r7 + 0) = (u16)(((u32)(u16)(*(u32 *)(tmp + 76)) & 255u) | 256u);
    *(u16 *)(r7 + 2) = (u16)(((u32)(s32)(s16)(u16)w60 & 0x1FFu) | r9s);
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(crec + 38)) << 12)
                           | (u32)(u16)(*(u16 *)(crec + 46)));
    r7 += 8;
    int ph = (int)(s16)(*(u16 *)(wa + 0x10FC));
    if ((u32)ph <= 8u) {
        switch (ph) {
        case 1:
        case 4:
        case 8:
            r7 = phase_case148(r7);
            break;
        case 2: {
            u8 *cc = *(u8 **)(uintptr_t)0x030003E0u;
            r7 = (u8 *)RomEmit88B0(r7, (int)*(u32 *)(cc + 68));
            break;
        }
        case 3:
            r7 = phase_case3(r7);
            break;
        case 5:
            r7 = phase_case5(r7);
            break;
        default:
            break;
        }
    }
    // [crec+32]/[crec+28]/[80]/[24] tail runs only inside case 5
    // (phase_case5 above); all paths converge on the _0800313C tail.
    _0800313C((u32)(uintptr_t)r7);
}
#ifndef __APPLE__
void _08008CA0(void) __attribute__((alias("CoursePhaseMachine_08CA0")));
void sub_08008CA0(void) __attribute__((alias("CoursePhaseMachine_08CA0")));
#endif

// Case 1/4/8 record burst + counter gate (8ED8). Returns tail cursor.
static u8 *phase_case148(u8 *r7) {
    u8 *cc = *(u8 **)(uintptr_t)0x030003E0u;
    r7 = (u8 *)RomEmit8768(r7, (int)*(u32 *)(cc + 64));
    r7 = (u8 *)RomEmit88B0(r7, (int)*(u32 *)(cc + 68));
    *(u16 *)(r7 + 0) = 2u;
    *(u16 *)(r7 + 2) = 0x40A7u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                             | (u32)(u16)(*(u16 *)(cc + 50)));
    r7 += 8;
    *(u16 *)(r7 + 0) = 2u;
    *(u16 *)(r7 + 2) = 0x40CDu;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                             | (u32)(u16)(*(u16 *)(cc + 48)));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 220u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52)) + 24u))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 227u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52))
                             + (u16)((u16)(*(u16 *)(cc + 10)) * 2u)))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    u16 lane = *(u16 *)(cc + 38);
    CourseEmit7B18(cc + 88, 12, 0, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 64, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 80, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 96, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 104, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 13, 120, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 168, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 14, 184, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 208, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 224, 4, lane, 1u, 0u, 0u);
    u16 c4 = *(u16 *)(cc + 4);
    if ((int)(s16)c4 <= 0) return r7;
    u16 c4n = (u16)(c4 - 1u);
    *(u16 *)(cc + 4) = c4n;
    if ((s32)(s16)c4n > 63
        && _08002D97C(_08002D978((int)(s16)c4n, 8), 2) == 0)
        return r7;
    return (u8 *)RomEmit861C(r7, (int)*(u32 *)(cc + 72));
}

// Case 3 variant (90A4): conditional head emits, 8-burst, same counter gate.
static u8 *phase_case3(u8 *r7) {
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    u8 *cc = *(u8 **)(uintptr_t)0x030003E0u;
    if (*(cc + 78) == 0u
        || _08002D97C(_08002D978((int)*(u32 *)(wa + 0x10D8), 3), 2) != 0)
        r7 = (u8 *)RomEmit8768(r7, (int)*(u32 *)(cc + 64));
    u16 c34 = *(u16 *)(cc + 34);
    if (c34 != 0u) {
        u16 n34 = (u16)(c34 - 1u);
        *(u16 *)(cc + 34) = n34;
        if (_08002D97C(_08002D978((int)(s16)n34, 3), 2) != 0)
            r7 = (u8 *)RomEmit88B0(r7, (int)*(u32 *)(cc + 72));
    } else {
        r7 = (u8 *)RomEmit88B0(r7, (int)*(u32 *)(cc + 68));
    }
    *(u16 *)(r7 + 0) = 2u;
    *(u16 *)(r7 + 2) = 0x40CDu;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                             | (u32)(u16)(*(u16 *)(cc + 48)));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 220u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52)) + 24u))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 227u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52))
                             + (u16)((u16)(*(u16 *)(cc + 10)) * 2u)))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    u16 lane = *(u16 *)(cc + 38);
    CourseEmit7B18(cc + 88, 12, 0, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 64, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 80, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 96, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 104, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 14, 184, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 208, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 224, 4, lane, 1u, 0u, 0u);
    u16 c4 = *(u16 *)(cc + 4);
    if ((int)(s16)c4 <= 0) return r7;
    u16 c4n = (u16)(c4 - 1u);
    *(u16 *)(cc + 4) = c4n;
    if ((s32)(s16)c4n > 63
        && _08002D97C(_08002D978((int)(s16)c4n, 8), 2) == 0)
        return r7;
    return (u8 *)RomEmit861C(r7, (int)*(u32 *)(cc + 72));
}

// __APPEND_MARKER__

// ROM entry alias.
#ifndef __APPLE__
s32 _0802DE9C(s32 num, s32 den) __attribute__((alias("DivRemS_02DE9C")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void sub_08007B8C(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) __attribute__((alias("CourseEmit7B8C")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void Sub_08007B18(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) __attribute__((alias("CourseEmit7B18")));
void sub_08007B18(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) __attribute__((alias("CourseEmit7B18")));
#endif

// 0x0800BC04 — 2B `bx lr` no-op (menu_bc04.s tail slot; caller at
// :355 is the same sequence, which the ROM proves does nothing).
void MenuNoop_BC04(void) {}
#ifndef __APPLE__
void _0800BC04(void) __attribute__((alias("MenuNoop_BC04")));
void sub_0800BC04(void) __attribute__((alias("MenuNoop_BC04")));
#endif
