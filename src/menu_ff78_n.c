// ============================================================================
// menu_ff78_n.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_08001479C (0x08001479C) — (rec) 6-iteration grid sample + 7B18.
//   sub_0800148D8 (0x0800148D8) — (rec) 6-iteration record emit + 7BFC.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) int Sub_08025CF4(int a, int b, int c) { (void)a; (void)b; (void)c; return 0; }
__attribute__((weak)) int Sub_0802572C(int a, int b, int c) { (void)a; (void)b; (void)c; return 0; }
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern int Sub_08025CF4(int a, int b, int c);
extern int Sub_0802572C(int a, int b, int c); // 3-arg ABI (r2 forwarded to 0x080256D8)
#endif

// ----------------------------------------------------------------------------
// sub_08001479C — (rec):
//   base = (0 <= s16[rec+166] <= 3) ? s16[rec+164] : 0.
//   for r9 in -1..4:
//     r4 = clamp(s16[rec+160]+r9, 0, s16[rec+158]).
//     sl = 25CF4(s16[WA+0xFF2], s16[0x03002776], r4).
//     r2 = clamp((s16)2572C(s16[0x03002776], r4), 0, 4).
//     skip = (u16[rec+36]==0 && (r9==-1 || r9==4)); if skip continue.
//     r1 = s16[0x080CB76C + r2*2];
//     7B18(rec+16, 10, r1, base-24+iter*48, 10,1,0,0)
//       where iter*48 accumulates from -48 (sp32) — i.e. arg = base-24
//       on the r9=-1 pass, +48 each pass.
//     if (sl-1) <= 2 unsigned (sl in 1..3):
//       r1 = s16[0x080CB776 + (sl-1)*2];
//       7B18(rec+16, 11, r1, base+iter*48, 11,1,0,0)
//       with iter*48 from -24 (sp28).
void MenuFF78_1479C(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    int base = 0;
    {
        s16 m = *(volatile s16 *)(uintptr_t)(rec + 166);
        if (m >= 0 && m <= 3)
            base = (int)*(volatile s16 *)(uintptr_t)(rec + 164);
    }
    int acc32 = -48, acc28 = -24;
    for (int r9 = -1; r9 <= 4; r9++) {
        int r4 = (int)*(volatile s16 *)(uintptr_t)(rec + 160) + r9;
        if (r4 < 0) r4 = 0;
        {
            int hi = (int)*(volatile s16 *)(uintptr_t)(rec + 158);
            if (r4 > hi) r4 = hi;
        }
        int sl = Sub_08025CF4(
            (int)*(volatile s16 *)(uintptr_t)(wa + 0xFF2u),
            (int)*(volatile s16 *)(uintptr_t)0x03002776u, r4);
        int r2 = (int)(s16)Sub_0802572C(
            (int)*(volatile s16 *)(uintptr_t)0x03002776u, r4, 0);
        if (r2 < 0) r2 = 0;
        if (r2 > 4) r2 = 4;
        int skip = (*(volatile u16 *)(uintptr_t)(rec + 36) == 0 &&
                    (r9 == -1 || r9 == 4));
        if (!skip) {
            int r1 = (int)*(volatile s16 *)(uintptr_t)(0x080CB76Cu +
                                                       (u32)r2 * 2);
            Sub_08007B18((void *)(uintptr_t)(rec + 16), 10, r1,
                         base + 24 + acc32, 10, 1, 0, 0);
            int s = sl - 1;
            if ((u32)s <= 2) {
                int t = (int)*(volatile s16 *)(uintptr_t)(0x080CB776u +
                                                          (u32)s * 2);
                Sub_08007B18((void *)(uintptr_t)(rec + 16), 11, t,
                             base + 24 + acc28, 11, 1, 0, 0);
            }
        }
        acc32 += 48;
        acc28 += 48;
    }
}
#ifndef __APPLE__
void _08001479C(void *a) __attribute__((alias("MenuFF78_1479C")));
void Sub_08001479C(void *a) __attribute__((alias("MenuFF78_1479C")));
void sub_08001479C(void *a) __attribute__((alias("MenuFF78_1479C")));
#endif

// ----------------------------------------------------------------------------
// sub_0800148D8 — (rec):
//   base = (0 <= s16[rec+166] <= 3) ? s16[rec+164] : 0.
//   for r5 in -1..4 (r4 = r5+1):
//     r1 = s16[rec+160] + r5.
//     gated = (u16[rec+36]==0 && (r5==-1 || r5==4)); if gated continue.
//     if r1 == s16[rec+156]:
//       r1 = max(r1, 0); pick u32[rec+176+r4*12], u32[rec+180+r4*12];
//       7BFC(rec, r1w, r2w, r5*48+base+24, 48,8,1,0,0).
//     else:
//       r1 = clamp(r1, 0, 10); pick same pair;
//       r1 <= 2: 7BFC(rec, w1, w2, r5*48+base+24, 48,6,1,0,0).
//       else (3..10): 7BFC(rec, w1, w2, r5*48+base+24, 48,7,1,0,0).
void MenuFF78_148D8(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int base = 0;
    {
        s16 m = *(volatile s16 *)(uintptr_t)(rec + 166);
        if (m >= 0 && m <= 3)
            base = (int)*(volatile s16 *)(uintptr_t)(rec + 164);
    }
    for (int r5 = -1; r5 <= 4; r5++) {
        int r4 = r5 + 1;
        int r1 = (int)*(volatile s16 *)(uintptr_t)(rec + 160) + r5;
        if (*(volatile u16 *)(uintptr_t)(rec + 36) == 0 &&
            (r5 == -1 || r5 == 4))
            continue;
        u32 r2off = (u32)r4 * 12;
        u32 w1 = *(volatile u32 *)(uintptr_t)(rec + 176 + r2off);
        u32 w2 = *(volatile u32 *)(uintptr_t)(rec + 180 + r2off);
        int r3 = r5 * 48 + base + 24;
        if (r1 == (int)*(volatile s16 *)(uintptr_t)(rec + 156)) {
            if (r1 < 0) r1 = 0;
            Sub_08007BFC(rec_, (int)w1, (int)w2, r3, 48, 8, 1, 0, 0);
        } else {
            if (r1 < 0) r1 = 0;
            if (r1 > 9) r1 = 10;
            if (r1 <= 2) {
                Sub_08007BFC(rec_, (int)w1, (int)w2, r3, 48, 6, 1, 0, 0);
            } else {
                Sub_08007BFC(rec_, (int)w1, (int)w2, r3, 48, 7, 1, 0, 0);
            }
        }
    }
}
#ifndef __APPLE__
void _0800148D8(void *a) __attribute__((alias("MenuFF78_148D8")));
void Sub_0800148D8(void *a) __attribute__((alias("MenuFF78_148D8")));
void sub_0800148D8(void *a) __attribute__((alias("MenuFF78_148D8")));
#endif
