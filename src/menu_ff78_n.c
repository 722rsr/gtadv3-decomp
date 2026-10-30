// ============================================================================
// menu_ff78_n.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the closure spellings the asm defines (Sub_/sub_/
// _080...), so behavior is identical by construction.
//
//   sub_08001479C (0x08001479C) — (rec) 6-iteration grid sample + 7B18.
//   sub_0800148D8 (0x0800148D8) — (rec) 6-iteration record emit + 7BFC.
// ============================================================================

#include "gba/types.h"

extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void _08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern int Sub_08025CF4(int a, int b, int c);
extern int sub_08025CF4(int a, int b, int c);
extern int Sub_0802572C(int a, int b, int c); // host fallback
extern s16 _0802572C(int a, int b);             // ROM call is the 2-arg wrapper
extern u8 Menu1479C_WA[];
extern s16 Menu1479C_SYM[];
extern s16 Menu1479C_T1[];
extern s16 Menu1479C_T2[];

// ----------------------------------------------------------------------------
// sub_08001479C — (rec):
//   base = (0 <= s16[rec+166] <= 3) ? s16[rec+164] : 0.
//   for r9 in -1..4:
//     r4 = clamp(s16[rec+160]+r9, 0, s16[rec+158]).
//     sl = 25CF4(s16[WA+0xFF2], s16[0x03002776], r4).
//     r2 = clamp((s16)2572C(s16[0x03002776], r4), 0, 4); the ROM call is
//     the two-argument wrapper, which ignores the incoming r2 value.
//     skip = (u16[rec+36]==0 && (r9==-1 || r9==4)); if skip continue.
//     r1 = s16[0x080CB76C + r2*2];
//     7B18(rec+16, 10, r1, base-24+iter*48, 10,1,0,0)
//       where iter*48 accumulates from -48 (sp32) — i.e. arg = base-24
//       on the r9=-1 pass, +48 each pass.
//     if (sl-1) <= 2 unsigned (sl in 1..3):
//       r1 = s16[0x080CB776 + (sl-1)*2];
//       7B18(rec+16, 11, r1, base+iter*48, 11,1,0,0)
//       with iter*48 from -24 (sp28).
//
// Two indexed reads still need their zero scratch held off the register agbcc
// would otherwise reuse, so each is bracketed by an output-only empty asm +
// an input-only one (no bytes; see docs/findings/menu_ff78_1479c_lift.md): r1
// around the rec+164 read gives the ROM's `movs r3,#0`, and r3 around the T2
// table read gives its `movs r2,#0`. The brackets trade off against the pins:
// with all 32 pins in place the rec+158 read and the skip test's `cmp` copy
// needed brackets too, and re-measuring removals one trick at a time dropped
// 13 pins and 7 empty asm statements (the removed pins changed the live set
// enough to hand those two scratches the ROM's register on their own). The
// remainder is maximal: every one of the 24 left fails alone and in pairs.
// The preheader still needs `offB = z - 47` with `i = z` live in r0: a plain
// -48 reloads i from r9 instead of folding to `subs r0,#47`.
void MenuFF78_1479C(void *rec_) {
    __asm__(".globl Menu1479C_WA\nMenu1479C_WA = 0x03001780\n"
            ".globl Menu1479C_SYM\nMenu1479C_SYM = 0x03002776\n"
            ".globl Menu1479C_T1\nMenu1479C_T1 = 0x080CB76C\n"
            ".globl Menu1479C_T2\nMenu1479C_T2 = 0x080CB776\n");
    u8 *rec;
    s32 i;
    volatile s32 base;
    u8 * volatile p158;
    u8 * volatile p160;
    volatile s32 offA;
    volatile s32 offB;
    s16 tag;
    s32 lim;
    s32 m;
    s32 idx;
    s32 mt;
    s32 res;
    register s32 v __asm__("r4");
    register s32 pos __asm__("r5");
    u8 *p16;
    register u8 *pq __asm__("r1");
    register u8 *pl __asm__("r1");
    s32 ic3;
    u8 *q;
    register s32 ob __asm__("r3");
    register s32 oa __asm__("r3");
    register s32 inc __asm__("r2");
    s32 ic;
    register s32 raw __asm__("r0");
    register s32 bt __asm__("r2");

    rec = (u8 *)rec_;
    base = 0;
    tag = *(s16 *)(rec + 166);
    if (tag >= 0) {
        if (tag <= 3) {
            register s32 hold __asm__("r1");

            __asm__ volatile("" : "=r"(hold));
            bt = *(s16 *)(rec + 164);
            __asm__ volatile("" : : "r"(hold));
            base = bt;
        }
    }
    {
        s32 z;
        register u8 *t1 __asm__("r1");
        register u8 *t2 __asm__("r2");
        register s32 t3 __asm__("r3");
        z = -1;
        i = z;
        t1 = rec + 160;
        p160 = t1;
        t2 = rec + 158;
        p158 = t2;
        t3 = -24;
        offA = t3;
        offB = z - 47;
    }
    do {
        pq = p160;
        raw = *(s16 *)pq;
        ic3 = i;
        v = raw + ic3;
        if (v < 0)
            v = 0;
        pl = p158;
        lim = *(s16 *)pl;
        if (v > lim)
            v = lim;
        {
            register u8 *wa __asm__("r3");
            register s16 *cell __asm__("r0");
            register s16 *sym __asm__("r3");
            wa = (u8 *)(uintptr_t)Menu1479C_WA;
            cell = (s16 *)(uintptr_t)(wa + 0xFF2u);
            res = sub_08025CF4(*cell,
                               *(sym = (s16 *)(uintptr_t)Menu1479C_SYM), v);
        }
        {
            register s16 *sym2 __asm__("r3");
            m = (s16)_0802572C(*(sym2 = (s16 *)(uintptr_t)Menu1479C_SYM), v);
        }
        if (m < 0)
            m = 0;
        if (m > 4)
            m = 4;
        q = rec;
        {
            u16 s36;

            s36 = *(u16 *)(q + 36);
            if (s36 != 0 || (i != -1 && i != 4)) {
                s16 *t1;
                p16 = rec + 16;
                t1 = (s16 *)(uintptr_t)Menu1479C_T1;
                mt = m * 2;
                sub_08007B18(p16, *(s16 *)((u8 *)(uintptr_t)mt + (uintptr_t)t1),
                             ((pos = base + 24), (ob = offB), ob + pos),
                             64, 10, 1, 0, 0);
                idx = res - 1;
                if ((u32)idx <= 2) {
                    s16 *t2;
                    register s32 hold3 __asm__("r3");
                    s32 tv;

                    t2 = (s16 *)(uintptr_t)Menu1479C_T2;
                    __asm__ volatile("" : "=r"(hold3));
                    tv = *(s16 *)((u8 *)(uintptr_t)(idx * 2) + (uintptr_t)t2);
                    __asm__ volatile("" : : "r"(hold3));
                    sub_08007B18(p16, tv,
                                 ((oa = offA), oa + pos), 64, 11, 1, 0, 0);
                }
            }
        }
        offA += 48;
        {
            register s32 tb __asm__("r1");
            tb = offB + 48;
            offB = tb;
        }
        inc = 1;
        __asm__ volatile("" : "+r"(inc));
        i += inc;
        ic = i;
    } while (ic <= 4);
}
void _08001479C(void *a) __attribute__((alias("MenuFF78_1479C")));
void Sub_08001479C(void *a) __attribute__((alias("MenuFF78_1479C")));
void sub_08001479C(void *a) __attribute__((alias("MenuFF78_1479C")));

// ----------------------------------------------------------------------------
// sub_0800148D8 — (rec): 6-iteration record emit.
//   base = (0 <= s16[rec+166] <= 3) ? s16[rec+164] : 0.
//   for i in -1..4:
//     v = s16[rec+160] + i.
//     skip when u16[rec+36]==0 and (i==-1 || i==4).
//     if v == s16[rec+156]: v = max(v,0);
//       7BFC(rec, u32[176+j*12], u32[180+j*12], i*48+base+24, 48,8,1,0,0).
//     else: v = clamp(v,0,10); v<=2 -> mode 6, (u32)(v-3)<=7 -> mode 7.
// The t r0 pin is the one this spelling needs: an unpinned local cannot be
// written in place, so the +180 table word becomes `adds r2,r0,r2 /
// ldr r2,[r2]` in place of the ROM's `adds r0,r0,r2 / ldr r2,[r0]` (298 of
// 304 bytes align).  base r8 and off r2 are kept from the road taken; each can
// be dropped and the body still measures exact.  See
// docs/findings/menu_ff78_148d8_lift.md for the sweep.
// Naming `base + 24` is load-bearing: inline, combine folds the +24 into the
// scale (`adds r3,#24 / add r3,r8`).  pos is pinned to r0, so that assignment
// has to stay in the argument list -- as its own statement the pin's live range
// covers the r0-pinned table temp, whose `adds r0,#180` then clobbers the
// offset and the arm passes rec+180 instead of base+24.
// The empty `r(v)` read keeps the then-arm's dead `v < 0` clamp, which agbcc
// otherwise deletes (the ROM keeps it).
void MenuFF78_148D8(void *rec_) {
    u8 *rec = (u8 *)rec_;
    register s32 base __asm__("r8");
    s32 i;
    s32 v;

    base = 0;
    if (*(s16 *)(rec + 166) >= 0) {
        if (*(s16 *)(rec + 166) <= 3)
            base = *(s16 *)(rec + 164);
    }
    for (i = -1; i <= 4; i++) {
        v = *(s16 *)(rec + 160) + i;
        if (*(u16 *)(rec + 36) == 0 && (i == -1 || i == 4))
            continue;
        if (v == *(s16 *)(rec + 156)) {
            if (v < 0)
                v = 0;
            __asm__ volatile("" : : "r"(v));
            {
                register u8 *t __asm__("r0");
                u32 a1;
                u32 a2;
                register s32 pos __asm__("r0");
                register s32 off __asm__("r2");

                off = (i + 1) * 12;
                t = rec + 176;
                a1 = *(u32 *)(t + off);
                t = rec + 180;
                a2 = *(u32 *)(t + off);
                _08007BFC(rec, a1, a2, i * 48 + (pos = base + 24),
                          48, 8, 1, 0, 0);
            }
        } else {
            if (v < 0)
                v = 0;
            if (v >= 10)
                v = 10;
            if (v <= 2) {
                register u8 *t __asm__("r0");
                u32 a1;
                u32 a2;
                register s32 pos __asm__("r0");
                register s32 off __asm__("r2");

                off = (i + 1) * 12;
                t = rec + 176;
                a1 = *(u32 *)(t + off);
                t = rec + 180;
                a2 = *(u32 *)(t + off);
                _08007BFC(rec, a1, a2, i * 48 + (pos = base + 24),
                          48, 6, 1, 0, 0);
            } else if ((u32)(v - 3) <= 7u) {
                register u8 *t __asm__("r0");
                u32 a1;
                u32 a2;
                register s32 pos __asm__("r0");
                register s32 off __asm__("r2");

                off = (i + 1) * 12;
                t = rec + 176;
                a1 = *(u32 *)(t + off);
                t = rec + 180;
                a2 = *(u32 *)(t + off);
                _08007BFC(rec, a1, a2, i * 48 + (pos = base + 24),
                          48, 7, 1, 0, 0);
            }
        }
    }
}
void _0800148D8(void *a) __attribute__((alias("MenuFF78_148D8")));
void Sub_0800148D8(void *a) __attribute__((alias("MenuFF78_148D8")));
void sub_0800148D8(void *a) __attribute__((alias("MenuFF78_148D8")));
