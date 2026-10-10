// ============================================================================
// menu_ff78_t.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080014F60 (0x080014F60) — (rec) emit machine over s16[rec+134].
//   sub_0800141AC (0x0800141AC) — (rec) big constructor + grid loops.
//
// NOTE on 14F60: for s = s16[rec+134] outside {0,1,2}, the (r6, sl) pair
// is caller-register residue in the ROM (never written on that path) and
// is only consumed when s is outside {2,3} as well, i.e. s<0 or s>3.
// The +134 cell is 0 on construction (14C88) and clamped 0..3 by its only
// mutator (14D94), and 14F60's sole caller is 15154-case5, so that path is
// unreachable; the C uses 0 there.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; }
__attribute__((weak)) void Sub_0800DAB8(void *a) { (void)a; }
__attribute__((weak)) void Sub_08007614(void *a, int b, int c, int d) { (void)a; (void)b; (void)c; (void)d; }
__attribute__((weak)) void Sub_0800798C(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_08007A58(void *a) { (void)a; }
__attribute__((weak)) void Sub_080075E8(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_08007ABC(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800D77C(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_08002B214(int v) { (void)v; }
__attribute__((weak)) void *Sub_08004B68(void) { return 0; }
__attribute__((weak)) u32 Sub_0800572C(int v) { (void)v; return 0; }
__attribute__((weak)) void Sub_08003954(int a, int b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800DBE8(void *a) { (void)a; }
__attribute__((weak)) void Sub_080013F94(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014078(void *a) { (void)a; }
__attribute__((weak)) int Sub_08025D64(int a, int b) { (void)a; return 0; }
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
extern void Sub_0800DAB8(void *a);
extern void Sub_08007614(void *a, int b, int c, int d);
extern void Sub_0800798C(void *a, void *b);
extern void Sub_08007A58(void *a);
extern void Sub_080075E8(void *a, int b, int c);
extern void Sub_08007ABC(u32 a, u32 b, u32 c);
extern void Sub_0800D77C(void *a, int b, int c);
extern void Sub_08002B214(int v);
extern void *Sub_08004B68(void);
extern u32 Sub_0800572C(int v);
extern void Sub_08003954(int a, int b, u32 c);
extern void Sub_0800D97C(void *a, int b);
extern void Sub_0800DBE8(void *a);
extern void Sub_080013F94(void *a);
extern void Sub_080014078(void *a);
extern int Sub_08025D64(int a, int b);
#endif

// ----------------------------------------------------------------------------
// sub_080014F60 — (rec):
//   If u16[rec+134] != 3: 7B18(rec, u16[CB7B8+s*2], u32[CB788+s*8],
//     u32[CB788+4+s*8], 3,1,0,0) with s = s16[rec+134].
//   (r6, sl) = s==1 ? (s16[138], 8) : s==0 ? (s16[136], 12) :
//     s==2 ? (s16[140], 2) : (0, 0) [unreachable residue, see NOTE].
//   r7 = rec+136, r8 = rec+138, r5 = rec+140, r9 = rec+134.
//   If (u16)(u16[134]-2) > 1 (s outside {2,3}):
//     if r6 > 1: 7B18(rec, u16[CB7BE+s16[130]*2],
//       u32[CB7A0+s*8], 104, 5,1,0,0).
//     if r6 < sl: 7B18(rec, u16[CB7BE+4+s16[130]*2],
//       u32[CB7A0+4+s*8], 160, 5,1,0,0).
//   s16[r5=+140]: 1->7B18(rec,13,112,112,6,1,0,0);
//     2->7B18(rec,12,136,112,6,1,0,0).
//   If u16[134]==3: 7B18(rec,18,200,128,7,1,0,0).
//   03954(144,52,s16[r7]); 03954(144,84,s16[r8]);
//   D97C(rec+128,15); DBE8(rec+8).
void MenuFF78_14F60(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 s = *(volatile s16 *)(uintptr_t)(rec + 134);
    if (*(volatile u16 *)(uintptr_t)(rec + 134) != 3) {
        u16 e = *(volatile u16 *)(uintptr_t)(0x080CB7B8u + (u32)(s32)s * 2);
        u32 w0 = *(volatile u32 *)(uintptr_t)(0x080CB788u + (u32)(s32)s * 8);
        u32 w1 = *(volatile u32 *)(uintptr_t)(0x080CB788u + 4 + (u32)(s32)s * 8);
        Sub_08007B18(rec_, (int)e, (int)w0, (int)w1, 3, 1, 0, 0);
    }
    int r6 = 0, sl = 0; // residue default; real values set below
    if (s == 1) {
        r6 = (int)*(volatile s16 *)(uintptr_t)(rec + 138);
        sl = 8;
    } else if (s == 0) {
        r6 = (int)*(volatile s16 *)(uintptr_t)(rec + 136);
        sl = 12;
    } else if (s == 2) {
        r6 = (int)*(volatile s16 *)(uintptr_t)(rec + 140);
        sl = 2;
    }
    if ((u16)((u16)s - 2) > 1) {
        if (r6 > 1) {
            s16 c = *(volatile s16 *)(uintptr_t)(rec + 130);
            u16 e = *(volatile u16 *)(uintptr_t)(0x080CB7BEu + (u32)(s32)c * 2);
            u32 w = *(volatile u32 *)(uintptr_t)(0x080CB7A0u + (u32)(s32)s * 8);
            Sub_08007B18(rec_, (int)e, (int)w, 104, 5, 1, 0, 0);
        }
        if (r6 < sl) {
            s16 c = *(volatile s16 *)(uintptr_t)(rec + 130);
            u16 e = *(volatile u16 *)(uintptr_t)(0x080CB7BEu + 4 + (u32)(s32)c * 2);
            u32 w = *(volatile u32 *)(uintptr_t)(0x080CB7A0u + 4 + (u32)(s32)s * 8);
            Sub_08007B18(rec_, (int)e, (int)w, 160, 5, 1, 0, 0);
        }
    }
    {
        s16 t = *(volatile s16 *)(uintptr_t)(rec + 140);
        if (t == 1) {
            Sub_08007B18(rec_, 13, 112, 112, 6, 1, 0, 0);
        } else if (t == 2) {
            Sub_08007B18(rec_, 12, 136, 112, 6, 1, 0, 0);
        }
    }
    if (*(volatile u16 *)(uintptr_t)(rec + 134) == 3)
        Sub_08007B18(rec_, 18, 200, 128, 7, 1, 0, 0);
    Sub_08003954(144, 52,
                 (u32)(s32)*(volatile s16 *)(uintptr_t)(rec + 136));
    Sub_08003954(144, 84,
                 (u32)(s32)*(volatile s16 *)(uintptr_t)(rec + 138));
    Sub_0800D97C((void *)(uintptr_t)(rec + 128), 15);
    Sub_0800DBE8((void *)(uintptr_t)(rec + 8));
}
#ifndef __APPLE__
void _080014F60(void *a) __attribute__((alias("MenuFF78_14F60")));
void Sub_080014F60(void *a) __attribute__((alias("MenuFF78_14F60")));
void sub_080014F60(void *a) __attribute__((alias("MenuFF78_14F60")));
#endif

// ----------------------------------------------------------------------------
// sub_0800141AC — (rec) big constructor:
//   2B214(52); 07770(0, 0x082F0FF0, 2, 0, 4, 1);
//   u32[104]=6; u32[116]=0; u32[128]=1; DAB8(rec+32);
//   7614(tile,1,0,3); 798C(0x082D7660,rec+16); 75E8(tm,0,10);
//   75E8(tm,1,11); 7A58(rec+16); 798C(tm,rec+8); 7A58(rec+8);
//   75E8(tm,0,5); 798C(0x082F3150,rec+24).
//   4 outer (i = 0..3) x 6 inner (k = 0..5):
//     u32[rec+248+i*72+k*12] = 0572C(2); u32[rec+252+...] = 9;
//     7ABC(u32[rec+28], 9, u32[cell]).
//   798C(0x082F36D8,rec);
//   7614(tm2,0,0,7); 7614(tm2,0,3,6); 7614(tm2,0,1,8); 7614(tm2,0,2,9).
//   6x (k = 0..5): u32[rec+176+k*12] = 0572C(12);
//     u32[rec+180+k*12] = (u32)(s32)s16[CB714+k*2+22*s16[0x03002776]];
//     7ABC(u32[rec+4], val, u32[rec+176+k*12]).
//   u32[rec+148] = u32[rec+32]; u32[rec+32] = 0; s16[rec+36] = 1;
//   D77C(rec+52,0,-32); D77C(rec+44,0,160);
//   s16[rec+40]=6; s16[rec+42]=5; s16[rec+166]=5; u32[rec+168]=2.
//   car==17: s16[156]=u16[WA+0x103A];
//   else: s16[156]=u16[WA+0xFFA+s16[WA+0xFF6]*2+s16[WA+0xFF2]*8].
//   s16[158] = (s16)25D64(s16[WA+0xFF2], s16[WA+0xFF6]);
//   upper-clamp s16[158], s16[156] at 10.
//   13F94(rec).
//   5x (d = -1..3): r0 = clamp(s16[160]+d, 0, 10);
//     u32[rec+180+(d+1)*12] = (u32)(s32)s16[CB714+r0*2+22*s16[WA+0xFF6]];
//     7ABC(u32[rec+4], val, u32[rec+176+(d+1)*12]).
//   14078(rec).
void MenuFF78_141AC(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    void *tile = (void *)(uintptr_t)0x082A798Cu;
    void *tm = (void *)(uintptr_t)0x082D7660u;
    u32 new_var;
    Sub_08002B214(52);
    _08007770(0, (void *)(uintptr_t)0x082F0FF0u, 2, 0, 4, 1); // R2 C body (was Sub_ veneer)
    *(volatile u32 *)(uintptr_t)(rec + 104) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 116) = 0;
    *(volatile u32 *)(uintptr_t)(rec + 128) = 1;
    Sub_0800DAB8((void *)(uintptr_t)(rec + 32));
    Sub_08007614(tile, 1, 0, 3);
    Sub_0800798C(tm, (void *)(uintptr_t)(rec + 16));
    new_var = 176;
    Sub_080075E8(tm, 0, 10);
    Sub_080075E8(tm, 1, 11);
    Sub_08007A58((void *)(uintptr_t)(rec + 16));
    Sub_0800798C(tm, rec_);
    Sub_08007A58(rec_);
    Sub_080075E8(tm, 0, 5);
    Sub_0800798C((void *)(uintptr_t)0x082F3150u,
                 (void *)(uintptr_t)(rec + 24));
    for (u32 i = 0; i <= 3; i++) {
        for (u32 k = 0; k < 6; k++) {
            u32 cell = (u32)(uintptr_t)(rec + 248 + i * 72 + k * 12);
            u32 cell2 = (u32)(uintptr_t)(rec + 252 + i * 72 + k * 12);
            *(volatile u32 *)(uintptr_t)cell = Sub_0800572C(2);
            *(volatile u32 *)(uintptr_t)cell2 = 9;
            Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 28), 9,
                         *(volatile u32 *)(uintptr_t)cell);
        }
    }
    {
        void *tm2 = (void *)(uintptr_t)0x082F36D8u;
        Sub_0800798C(tm2, rec_);
        Sub_08007614(tm2, 0, 0, 7);
        Sub_08007614(tm2, 0, 3, 6);
        Sub_08007614(tm2, 0, 1, 8);
        Sub_08007614(tm2, 0, 2, 9);
    }
    for (u32 k = 0; k <= 5; k++) {
        u32 c4 = (u32)(uintptr_t)(rec + new_var + k * 12);
        u32 c5 = (u32)(uintptr_t)(rec + 180 + k * 12);
        *(volatile u32 *)(uintptr_t)c4 = Sub_0800572C(12);
        int t = (int)*(volatile s16 *)(uintptr_t)(
            0x080CB714u + k * 2 +
            (u32)(22 * (int)*(volatile s16 *)(uintptr_t)0x03002776u));
        *(volatile u32 *)(uintptr_t)c5 = (u32)t;
        Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 4), (u32)t,
                     *(volatile u32 *)(uintptr_t)c4);
    }
    *(volatile u32 *)(uintptr_t)(rec + 148) =
        *(volatile u32 *)(uintptr_t)(rec + 32);
    *(volatile u32 *)(uintptr_t)(rec + 32) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 36) = 1;
    Sub_0800D77C((void *)(uintptr_t)(rec + 52), 0, -32);
    Sub_0800D77C((void *)(uintptr_t)(rec + 44), 0, 160);
    *(volatile s16 *)(uintptr_t)(rec + 40) = 6;
    *(volatile s16 *)(uintptr_t)(rec + 42) = 5;
    *(volatile s16 *)(uintptr_t)(rec + 166) = 5;
    *(volatile u32 *)(uintptr_t)(rec + 168) = 2;
    {
        u16 car = *(volatile u16 *)(uintptr_t)((volatile u8 *)Sub_08004B68() + 2);
        if (car == 17) {
            *(volatile s16 *)(uintptr_t)(rec + 156) =
                *(volatile u16 *)(uintptr_t)(wa + 0x103Au);
        } else {
            u32 addr = 0x03001780u + 0xFFAu +
                (u32)((s32)*(volatile s16 *)(uintptr_t)(wa + 0xFF6u) * 2) +
                (u32)((s32)*(volatile s16 *)(uintptr_t)(wa + 0xFF2u) * 8);
            *(volatile s16 *)(uintptr_t)(rec + 156) =
                *(volatile u16 *)(uintptr_t)addr;
        }
    }
    *(volatile s16 *)(uintptr_t)(rec + 158) = (s16)Sub_08025D64(
        (int)*(volatile s16 *)(uintptr_t)(wa + 0xFF2u),
        (int)*(volatile s16 *)(uintptr_t)(wa + 0xFF6u));
    if (*(volatile s16 *)(uintptr_t)(rec + 158) > 9)
        *(volatile s16 *)(uintptr_t)(rec + 158) = 10;
    if (*(volatile s16 *)(uintptr_t)(rec + 156) > 9)
        *(volatile s16 *)(uintptr_t)(rec + 156) = 10;
    Sub_080013F94(rec_);
    for (int d = -1; d <= 3; d++) {
        int r0 = (int)*(volatile s16 *)(uintptr_t)(rec + 160) + d;
        if (r0 < 0) r0 = 0;
        if (r0 > 9) r0 = 10;
        u32 r2off = (u32)(d + 1) * 12;
        int t = (int)*(volatile s16 *)(uintptr_t)(
            0x080CB714u + (u32)r0 * 2 +
            (u32)(22 * (int)*(volatile s16 *)(uintptr_t)(wa + 0xFF6u)));
        *(volatile u32 *)(uintptr_t)(rec + 180 + r2off) = (u32)t;
        Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 4), (u32)t,
                     *(volatile u32 *)(uintptr_t)(rec + new_var + r2off));
    }
    Sub_080014078(rec_);
}
#ifndef __APPLE__
void _0800141AC(void *a) __attribute__((alias("MenuFF78_141AC")));
void Sub_0800141AC(void *a) __attribute__((alias("MenuFF78_141AC")));
void sub_0800141AC(void *a) __attribute__((alias("MenuFF78_141AC")));
#endif
