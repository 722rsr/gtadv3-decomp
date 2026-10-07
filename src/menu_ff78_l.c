// ============================================================================
// menu_ff78_l.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080011958 (0x080011958) — (rec) s16[rec+168] {1,2} emit machine.
//   sub_080011B48 (0x080011B48) — (rec) u16[rec+214] 8-way emit machine.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) int Sub_0800F6D0(int v) { return v; }
__attribute__((weak)) void Sub_080011854(void *a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800118DC(void *a, u32 b) { (void)a; (void)b; }
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern int Sub_0800F6D0(int v);
extern void Sub_080011854(void *a, u32 b);
extern void Sub_0800118DC(void *a, u32 b);
#endif

// Table base for the record-13 emit tails (stride-8 {w0, w1} pairs).
#define FF78_TBL_B5B0 ((u32)0x080CB5B0u)

// ----------------------------------------------------------------------------
// sub_080011958 — (rec): dispatch on s16[rec+168].
//   ==1: F6D0 head (r0==3: 7B18(rec,10,0,112,3,1,1,0);
//     r0==2: 7B18(rec,12,0,112,3,1,1,0)); then for k in 0..F6D0-1
//     (limit re-evaluated each iteration):
//     7BFC(rec+32, u32[rec+372+k*12], u32[rec+376+k*12],
//       u32[sp20+k*4], 120,5,1,1,0) with sp20/24/28 = {8,32,56}.
//     Then 11854(rec, u16[rec+190]); if u16[rec+190] != 99:
//       7B18(rec,13,tbl[u16[rec+212]].w0,tbl[u16[rec+212]].w1,3,1,1,0).
//   ==2: 7B18(rec,10,0,112,3,1,1,0);
//     7BFC(rec+32,u32[372],u32[376],8,120,5,1,1,0);
//     7BFC(rec+32,u32[384],u32[388],32,120,5,1,1,0);
//     7BFC(rec+32,u32[396],u32[400],56,120,5,1,1,0);
//     118DC(rec,u16[rec+192]); if u16[rec+192] != 99: same 13-emit.
//   else: return.
void MenuFF78_11958(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 mode = *(volatile s16 *)(uintptr_t)(rec + 168);
    if (mode == 1) {
        int h = Sub_0800F6D0((int)*(volatile s16 *)(uintptr_t)(rec + 188));
        if (h == 3) {
            Sub_08007B18(rec_, 10, 0, 112, 3, 1, 1, 0);
        } else if (h == 2) {
            Sub_08007B18(rec_, 12, 0, 112, 3, 1, 1, 0);
        }
        {
            // kinds = {8,32,56} at sp+20: k is bounded by the F6D0 limit,
            // whose ROM table (0x0805F940 rows) yields at most 3, so k
            // never exceeds 2 and the 3-word array is never over-read.
            u32 kinds[3] = { 8, 32, 56 };
            u32 k = 0;
            for (;;) {
                int lim = Sub_0800F6D0(
                    (int)*(volatile s16 *)(uintptr_t)(rec + 188));
                if (!((int)k < lim)) break;
                u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 372 + k * 12);
                u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 376 + k * 12);
                Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)r1, (int)r2,
                             (int)kinds[k], 120, 5, 1, 1, 0);
                k++;
            }
        }
        Sub_080011854(rec_, *(volatile u16 *)(uintptr_t)(rec + 190));
        if (*(volatile u16 *)(uintptr_t)(rec + 190) != 99) {
            u16 ti = *(volatile u16 *)(uintptr_t)(rec + 212);
            u32 w0 = *(volatile u32 *)(uintptr_t)(FF78_TBL_B5B0 +
                                                  (u32)ti * 8);
            u32 w1 = *(volatile u32 *)(uintptr_t)(FF78_TBL_B5B0 + 4 +
                                                  (u32)ti * 8);
            Sub_08007B18(rec_, 13, (int)w0, (int)w1, 3, 1, 1, 0);
        }
    } else if (mode == 2) {
        Sub_08007B18(rec_, 10, 0, 112, 3, 1, 1, 0);
        {
            u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 372);
            u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 376);
            Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)r1, (int)r2,
                         8, 120, 5, 1, 1, 0);
        }
        {
            u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 384);
            u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 388);
            Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)r1, (int)r2,
                         32, 120, 5, 1, 1, 0);
        }
        {
            u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 396);
            u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 400);
            Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)r1, (int)r2,
                         56, 120, 5, 1, 1, 0);
        }
        Sub_0800118DC(rec_, *(volatile u16 *)(uintptr_t)(rec + 192));
        if (*(volatile u16 *)(uintptr_t)(rec + 192) != 99) {
            u16 ti = *(volatile u16 *)(uintptr_t)(rec + 212);
            u32 w0 = *(volatile u32 *)(uintptr_t)(FF78_TBL_B5B0 +
                                                  (u32)ti * 8);
            u32 w1 = *(volatile u32 *)(uintptr_t)(FF78_TBL_B5B0 + 4 +
                                                  (u32)ti * 8);
            Sub_08007B18(rec_, 13, (int)w0, (int)w1, 3, 1, 1, 0);
        }
    }
}
#ifndef __APPLE__
void _080011958(void *a) __attribute__((alias("MenuFF78_11958")));
void Sub_080011958(void *a) __attribute__((alias("MenuFF78_11958")));
void sub_080011958(void *a) __attribute__((alias("MenuFF78_11958")));
#endif

// ----------------------------------------------------------------------------
// sub_080011B48 — (rec): v = u16[rec+214]; v > 7 -> return.
//   v in {5,6}: 7BFC(rec+32,u32[336],u32[340],160,96,5,1,1,0);
//     +[u16[190]!=99] 7BFC(rec+32,u32[348],u32[352],184,...);
//     +[u16[192]!=99] 7BFC(rec+32,u32[360],u32[364],208,...).
//   else (0,1,2,3,4,7): r2 = (v<=1) ? u32[340] : u32[344];
//     7BFC(rec+32,u32[336],r2,160,...); 7BFC(...,u32[348],u32[352],184,...);
//     7BFC(...,u32[360],u32[364],208,...). (s = 96,5,1,1,0 throughout.)
void MenuFF78_11B48(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 v = *(volatile u16 *)(uintptr_t)(rec + 214);
    if (v > 7) return;
    if (v == 5 || v == 6) {
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 336);
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 340);
        Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)r1, (int)r2,
                     160, 96, 5, 1, 1, 0);
        if (*(volatile u16 *)(uintptr_t)(rec + 190) != 99) {
            u32 a = *(volatile u32 *)(uintptr_t)(rec + 348);
            u32 b = *(volatile u32 *)(uintptr_t)(rec + 352);
            Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)a, (int)b,
                         184, 96, 5, 1, 1, 0);
        }
        if (*(volatile u16 *)(uintptr_t)(rec + 192) != 99) {
            u32 a = *(volatile u32 *)(uintptr_t)(rec + 360);
            u32 b = *(volatile u32 *)(uintptr_t)(rec + 364);
            Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)a, (int)b,
                         208, 96, 5, 1, 1, 0);
        }
    } else {
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 336);
        u32 r2 = (v <= 1) ? *(volatile u32 *)(uintptr_t)(rec + 340)
                          : *(volatile u32 *)(uintptr_t)(rec + 344);
        Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)r1, (int)r2,
                     160, 96, 5, 1, 1, 0);
        {
            u32 a = *(volatile u32 *)(uintptr_t)(rec + 348);
            u32 b = *(volatile u32 *)(uintptr_t)(rec + 352);
            Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)a, (int)b,
                         184, 96, 5, 1, 1, 0);
        }
        {
            u32 a = *(volatile u32 *)(uintptr_t)(rec + 360);
            u32 b = *(volatile u32 *)(uintptr_t)(rec + 364);
            Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)a, (int)b,
                         208, 96, 5, 1, 1, 0);
        }
    }
}
#ifndef __APPLE__
void _080011B48(void *a) __attribute__((alias("MenuFF78_11B48")));
void Sub_080011B48(void *a) __attribute__((alias("MenuFF78_11B48")));
void sub_080011B48(void *a) __attribute__((alias("MenuFF78_11B48")));
#endif
