// ============================================================================
// menu_ff78_q.c — reconstructed C for asm/menu_ff78.s (1 function).
//
// Body transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080014488 (0x080014488) — (rec, u16 a1, u16 ev) event machine.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08002B368(u32 v) { (void)v; }
__attribute__((weak)) void Sub_08007ABC(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080013F94(void *a) { (void)a; }
#else
extern void Sub_08002B368(u32 v);
extern void Sub_08007ABC(u32 a, u32 b, u32 c);
extern void Sub_080013F94(void *a);
#endif

// ----------------------------------------------------------------------------
// sub_080014488 — (rec, u16 a1, u16 ev):
//   entry = u16[rec+156]; entry2 = u16[rec+160].
//   ev==2: 2B368(4); u32[32]=10; s16[36]=0; u32[64]=10; u32[60]=0.
//   ev==1: 2B368(1); s16[36]=0; s16[140]=1; u16[WA+0x103A]=u16[156].
//   If s16[rec+166]==5:
//     A (a1&32): if s16[162]<=0: r3=u16[156]; r1=s16[156];
//       if r1>0: u16[156]=r3-1; and if r1==s16[160]: s16[166]=0.
//       s16[162]=7.
//     B (a1&16): if s16[162]<=0: r3=u16[156]; r1=s16[156];
//       if r1 < s16[158]: t=r1; if t==s16[160]+3: u16[156]=r3+1,
//       s16[166]=1; else u16[156]=r3+1. s16[162]=7.
//   s16[162]--; if <=0: s16[162]=0.
//   Cell: if s16[156]<0: =0. If s16[156] > s16[158]: s16[156]=u16[158].
//   13F94(rec). If u16[156] != entry: 2B368(3).
//   If entry2 == u16[160]: return.
//   Else 5-iteration table fill (d = -1..3):
//     r0 = clamp(s16[160]+d, 0, 10); u32[rec+180+(d+1)*12] =
//       (u32)(s32)s16[0x080CB714 + r0*2 + 22*s16[0x03002776]];
//     7ABC(u32[rec+4], s16val, u32[rec+176+(d+1)*12]).
void MenuFF78_14488(void *rec_, u32 a1_, u32 ev_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 a1 = (u16)a1_;
    u16 ev = (u16)ev_;
    u16 entry = *(volatile u16 *)(uintptr_t)(rec + 156);
    u16 entry2 = *(volatile u16 *)(uintptr_t)(rec + 160);
    if (ev == 2) {
        Sub_08002B368(4);
        *(volatile u32 *)(uintptr_t)(rec + 32) = 10;
        *(volatile s16 *)(uintptr_t)(rec + 36) = 0;
        *(volatile u32 *)(uintptr_t)(rec + 64) = 10;
        *(volatile u32 *)(uintptr_t)(rec + 60) = 0;
    }
    if (ev == 1) {
        Sub_08002B368(1);
        *(volatile s16 *)(uintptr_t)(rec + 36) = 0;
        *(volatile s16 *)(uintptr_t)(rec + 140) = (s16)ev;
        *(volatile u16 *)(uintptr_t)(0x03001780u + 0x103Au) =
            *(volatile u16 *)(uintptr_t)(rec + 156);
    }
    if (*(volatile s16 *)(uintptr_t)(rec + 166) == 5) {
        if ((a1 & 32) != 0 && *(volatile s16 *)(uintptr_t)(rec + 162) <= 0) {
            u16 r3 = *(volatile u16 *)(uintptr_t)(rec + 156);
            s16 r1 = *(volatile s16 *)(uintptr_t)(rec + 156);
            if (r1 > 0) {
                *(volatile u16 *)(uintptr_t)(rec + 156) = (u16)(r3 - 1);
                if (r1 == *(volatile s16 *)(uintptr_t)(rec + 160))
                    *(volatile s16 *)(uintptr_t)(rec + 166) = 0;
            }
            *(volatile s16 *)(uintptr_t)(rec + 162) = 7;
        }
        if ((a1 & 16) != 0 && *(volatile s16 *)(uintptr_t)(rec + 162) <= 0) {
            u16 r3 = *(volatile u16 *)(uintptr_t)(rec + 156);
            s16 r1 = *(volatile s16 *)(uintptr_t)(rec + 156);
            if (r1 < *(volatile s16 *)(uintptr_t)(rec + 158)) {
                int t = (int)r1;
                if (t == (int)*(volatile s16 *)(uintptr_t)(rec + 160) + 3) {
                    *(volatile u16 *)(uintptr_t)(rec + 156) = (u16)(r3 + 1);
                    *(volatile s16 *)(uintptr_t)(rec + 166) = 1;
                } else {
                    *(volatile u16 *)(uintptr_t)(rec + 156) = (u16)(r3 + 1);
                }
            }
            *(volatile s16 *)(uintptr_t)(rec + 162) = 7;
        }
    }
    {
        s16 c = (s16)(*(volatile s16 *)(uintptr_t)(rec + 162) - 1);
        *(volatile s16 *)(uintptr_t)(rec + 162) = (c <= 0) ? 0 : c;
    }
    {
        if (*(volatile s16 *)(uintptr_t)(rec + 156) < 0)
            *(volatile s16 *)(uintptr_t)(rec + 156) = 0;
        if (*(volatile s16 *)(uintptr_t)(rec + 156) >
            *(volatile s16 *)(uintptr_t)(rec + 158))
            *(volatile s16 *)(uintptr_t)(rec + 156) =
                *(volatile u16 *)(uintptr_t)(rec + 158);
    }
    Sub_080013F94(rec_);
    if (*(volatile u16 *)(uintptr_t)(rec + 156) != entry)
        Sub_08002B368(3);
    if (entry2 == *(volatile u16 *)(uintptr_t)(rec + 160))
        return;
    for (int d = -1; d <= 3; d++) {
        int r0 = (int)*(volatile s16 *)(uintptr_t)(rec + 160) + d;
        if (r0 < 0) r0 = 0;
        if (r0 > 9) r0 = 10;
        u32 r2off = (u32)(d + 1) * 12;
        int t = (int)*(volatile s16 *)(uintptr_t)(
            0x080CB714u + (u32)r0 * 2 +
            (u32)(22 * (int)*(volatile s16 *)(uintptr_t)0x03002776u));
        *(volatile u32 *)(uintptr_t)(rec + 180 + r2off) = (u32)t;
        Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 4), (u32)t,
                     *(volatile u32 *)(uintptr_t)(rec + 176 + r2off));
    }
}
#ifndef __APPLE__
void _080014488(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_14488")));
void Sub_080014488(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_14488")));
void sub_080014488(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_14488")));
#endif
