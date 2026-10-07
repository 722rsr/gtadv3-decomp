// ============================================================================
// menu_ff78_p.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080015620 (0x080015620) — (rec, dead, ev) event handler.
//   sub_08001573C (0x08001573C) — (rec, dead, ev) event handler.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08002B368(u32 v) { (void)v; }
__attribute__((weak)) int Sub_08024068(void) { return 0; }
__attribute__((weak)) void Sub_08001549C(void *a) { (void)a; }
__attribute__((weak)) void Sub_08002158(int cmd, u32 val) { (void)cmd; (void)val; }
__attribute__((weak)) int Sub_08002178(u32 a) { (void)a; return 0; }
__attribute__((weak)) void Sub_08002618(u32 a, u32 b) { (void)a; (void)b; }
#else
extern void Sub_08002B368(u32 v);
extern int Sub_08024068(void);
extern void Sub_08001549C(void *a);
extern void Sub_08002158(int cmd, u32 val);
extern int Sub_08002178(u32 a);
extern void Sub_08002618(u32 a, u32 b);
#endif

// ----------------------------------------------------------------------------
// sub_080015620 — (rec, dead, ev): r6 = rec+144, entry = u16[rec+144].
//   ev==2: 1549C(rec).
//   ev==1 on s = s16[rec+144]:
//     s==0: s16[rec+24]=ev; 2B368(1); u32[16]=10; s16[20]=0; u32[48]=10;
//       u32[44]=ev; u16[WA+0x107C]=u16[rec+144].
//     s==1: u16[WA+0x107C]=u16[rec+144]; 1549C(rec).
//     s in 2..4: v=24068; r0 = (v==16) ? 5 : (v==39 ? 1 : v);
//       s16[rec+24]=r0; 2B368(1); u32[16]=10; s16[20]=0; u32[48]=10;
//       u32[44]=1; u16[WA+0x107C]=u16[rec+144].
//   tail on cell u16[r6]: ev==64: cell--, then if ==2: cell=1;
//   ev==128: cell++, then if ==2: cell=3.
//   clamp s16 0..4; changed (u16) vs entry -> 2B368(2).
void MenuFF78_15620(void *rec_, u32 dead, u32 ev_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    u16 ev = (u16)ev_;
    volatile u8 *cell = (volatile u8 *)(uintptr_t)(rec + 144);
    u16 entry = *(volatile u16 *)(uintptr_t)cell;
    if (ev == 2)
        Sub_08001549C(rec_);
    if (ev == 1) {
        s16 s = *(volatile s16 *)(uintptr_t)cell;
        if (s == 0) {
            *(volatile s16 *)(uintptr_t)(rec + 24) = (s16)ev;
            Sub_08002B368(1);
            *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
            *(volatile s16 *)(uintptr_t)(rec + 20) = 0;
            *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
            *(volatile u32 *)(uintptr_t)(rec + 44) = (u32)ev;
            *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                *(volatile u16 *)(uintptr_t)cell;
        } else if (s == 1) {
            *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                *(volatile u16 *)(uintptr_t)cell;
            Sub_08001549C(rec_);
        } else if (s >= 2 && s <= 4) {
            int v = Sub_08024068();
            int r0 = (v == 16) ? 5 : (v == 39 ? 1 : v);
            *(volatile s16 *)(uintptr_t)(rec + 24) = (s16)r0;
            Sub_08002B368(1);
            *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
            *(volatile s16 *)(uintptr_t)(rec + 20) = 0;
            *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
            *(volatile u32 *)(uintptr_t)(rec + 44) = 1;
            *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                *(volatile u16 *)(uintptr_t)cell;
        }
    }
    if (ev == 64) {
        s16 c = (s16)(*(volatile u16 *)(uintptr_t)cell - 1);
        *(volatile s16 *)(uintptr_t)cell = c;
        if (c == 2)
            *(volatile s16 *)(uintptr_t)cell = 1;
    }
    if (ev == 128) {
        s16 c = (s16)(*(volatile u16 *)(uintptr_t)cell + 1);
        *(volatile s16 *)(uintptr_t)cell = c;
        if (c == 2)
            *(volatile s16 *)(uintptr_t)cell = 3;
    }
    if (*(volatile s16 *)(uintptr_t)cell < 0)
        *(volatile s16 *)(uintptr_t)cell = 0;
    if (*(volatile s16 *)(uintptr_t)cell > 4)
        *(volatile s16 *)(uintptr_t)cell = 4;
    if (*(volatile u16 *)(uintptr_t)cell != entry)
        Sub_08002B368(2);
}
#ifndef __APPLE__
void _080015620(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_15620")));
void Sub_080015620(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_15620")));
void sub_080015620(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_15620")));
#endif

// ----------------------------------------------------------------------------
// sub_08001573C — (rec, dead, ev): entry = u16[rec+144].
//   Head: if u8[WA+0x10C3]==1: 02158(u16[150],4); 02158(u16[146],2);
//     02158(u16[146],3); else: 02158(u16[148],4); 02158(u16[144],2);
//     02158(u16[144],3). r1 = rec+146, sl = rec+148 either way.
//   ev==1: s16[rec+150]=1. ev==2: s16[rec+150]=2.
//   ev==64: u16[r1]--. ev==128: u16[r1]++. Clamp s16[r1] 0..4.
//   Gate = u8[WA+0x10C3]:
//     0: 02178(1,2)->s16[rec+144]; 02178(1,4)->s16[sl].
//     nonzero: 02178(0,2)->s16[rec+144]; 02178(0,4)->s16[sl].
//   If (u16)02178(gate,4) != (u16)02178(gate^1,4):
//     (recompute r4 = rec+144, r9 = u16[rec+144].)
//     slv = s16[sl];
//     slv==1: s16[rec+150]=0; 02618(1,0); s16[rec+136]=0;
//       q = s16[rec+144];
//       q==1: u16[WA+0x107C]=u16[rec+144]; 1549C(rec).
//       q==0: s16[rec+24]=slv; 2B368(1); u32[16]=10; s16[20]=0;
//         u32[48]=10; u32[44]=slv; u16[WA+0x107C]=u16[rec+144].
//       q in 2..4: 2B368(1); u32[16]=10; s16[20]=0 (then same tail:
//         u32[48]=10; u32[44]=slv; u16[WA+0x107C]=u16[rec+144]).
//     slv==2: s16[rec+150]=0; 02618(1,0); s16[rec+136]=0; 1549C(rec).
void MenuFF78_1573C(void *rec_, u32 dead, u32 ev_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    u16 ev = (u16)ev_;
    u16 entry = *(volatile u16 *)(uintptr_t)(rec + 144);
    u8 gate = *(volatile u8 *)(uintptr_t)(wa + 0x10C3u);
    if (gate == 1) {
        Sub_08002158(4, *(volatile u16 *)(uintptr_t)(rec + 150));
        Sub_08002158(2, *(volatile u16 *)(uintptr_t)(rec + 146));
        Sub_08002158(3, *(volatile u16 *)(uintptr_t)(rec + 146));
    } else {
        Sub_08002158(4, *(volatile u16 *)(uintptr_t)(rec + 148));
        Sub_08002158(2, *(volatile u16 *)(uintptr_t)(rec + 144));
        Sub_08002158(3, *(volatile u16 *)(uintptr_t)(rec + 144));
    }
    volatile u8 *r1 = (volatile u8 *)(uintptr_t)(rec + 146);
    volatile u8 *sl = (volatile u8 *)(uintptr_t)(rec + 148);
    if (ev == 1)
        *(volatile s16 *)(uintptr_t)(rec + 150) = 1;
    if (ev == 2)
        *(volatile s16 *)(uintptr_t)(rec + 150) = 2;
    if (ev == 64) {
        u16 c = *(volatile u16 *)(uintptr_t)r1;
        *(volatile u16 *)(uintptr_t)r1 = (u16)(c - 1);
    }
    if (ev == 128) {
        u16 c = *(volatile u16 *)(uintptr_t)r1;
        *(volatile u16 *)(uintptr_t)r1 = (u16)(c + 1);
    }
    if (*(volatile s16 *)(uintptr_t)r1 < 0)
        *(volatile s16 *)(uintptr_t)r1 = 0;
    if (*(volatile s16 *)(uintptr_t)r1 > 4)
        *(volatile s16 *)(uintptr_t)r1 = 4;
    {
        int g = (gate == 0) ? 1 : 0;
        int v0 = Sub_08002178((u32)g);
        *(volatile s16 *)(uintptr_t)(rec + 144) = (s16)(u16)v0;
        int v1 = Sub_08002178((u32)g);
        *(volatile s16 *)(uintptr_t)sl = (s16)(u16)v1;
        if (entry != *(volatile u16 *)(uintptr_t)(rec + 144))
            Sub_08002B368(2);
        int w0 = Sub_08002178(0);
        int w1 = Sub_08002178(1);
        if ((u16)w0 != (u16)w1) {
            s16 slv = *(volatile s16 *)(uintptr_t)sl;
            if (slv == 1) {
                *(volatile s16 *)(uintptr_t)(rec + 150) = 0;
                Sub_08002618(1, 0);
                *(volatile s16 *)(uintptr_t)(rec + 136) = 0;
                s16 q = *(volatile s16 *)(uintptr_t)(rec + 144);
                if (q == 1) {
                    *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                        *(volatile u16 *)(uintptr_t)(rec + 144);
                    Sub_08001549C(rec_);
                } else if (q == 0) {
                    *(volatile s16 *)(uintptr_t)(rec + 24) = slv;
                    Sub_08002B368(1);
                    *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
                    *(volatile s16 *)(uintptr_t)(rec + 20) = 0;
                    *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
                    *(volatile u32 *)(uintptr_t)(rec + 44) = (u32)(u16)slv;
                    *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                        *(volatile u16 *)(uintptr_t)(rec + 144);
                } else if (q >= 2 && q <= 4) {
                    Sub_08002B368(1);
                    *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
                    *(volatile s16 *)(uintptr_t)(rec + 20) = 0;
                    *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
                    *(volatile u32 *)(uintptr_t)(rec + 44) = (u32)(u16)slv;
                    *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                        *(volatile u16 *)(uintptr_t)(rec + 144);
                }
            } else if (slv == 2) {
                *(volatile s16 *)(uintptr_t)(rec + 150) = 0;
                Sub_08002618(1, 0);
                *(volatile s16 *)(uintptr_t)(rec + 136) = 0;
                Sub_08001549C(rec_);
            }
        }
    }
}
#ifndef __APPLE__
void _08001573C(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_1573C")));
void Sub_08001573C(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_1573C")));
void sub_08001573C(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_1573C")));
#endif
