// ============================================================================
// menu_ff78_o.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080015304 (0x080015304) — (rec) WA+0xFBC 8-way setup + tail.
//   sub_0800154E0 (0x0800154E0) — (rec, dead, ev) event handler.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; }
__attribute__((weak)) void Sub_0800DAB8(void *a) { (void)a; }
__attribute__((weak)) void Sub_08007614(void *a, int b, int c, int d) { (void)a; (void)b; (void)c; (void)d; }
__attribute__((weak)) void Sub_0800798C(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_08007A58(void *a) { (void)a; }
__attribute__((weak)) void Sub_080075E8(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_08007ABC(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800D77C(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) int Sub_08002124(u32 v) { (void)v; return 0; }
__attribute__((weak)) void Sub_08002B368(u32 v) { (void)v; }
__attribute__((weak)) int Sub_08024068(void) { return 0; }
__attribute__((weak)) void Sub_08001549C(void *a) { (void)a; }
__attribute__((weak)) void Sub_080025BC8(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800152A8(void *a) { (void)a; }
#else
extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
extern void Sub_0800DAB8(void *a);
extern void Sub_08007614(void *a, int b, int c, int d);
extern void Sub_0800798C(void *a, void *b);
extern void Sub_08007A58(void *a);
extern void Sub_080075E8(void *a, int b, int c);
extern void Sub_08007ABC(u32 a, u32 b, u32 c);
extern void Sub_0800D77C(void *a, int b, int c);
extern int Sub_08002124(u32 v);
extern void Sub_08002B368(u32 v);
extern int Sub_08024068(void);
extern void Sub_08001549C(void *a);
extern void Sub_080025BC8(void *a, int b);
extern void Sub_0800152A8(void *a);
#endif

// ----------------------------------------------------------------------------
// sub_080015304 — (rec): head = 8-way on s16[WA+0xFBC]:
//   0: 07770(0,tmpl,1,0,4,1), s16[rec+138]=2.
//   1,2,4,6: 07770(0,tmpl,0,0,4,1), s16[rec+138]=0.
//   3: 07770(0,tmpl,0,0,4,1), s16[rec+138]=1.
//   5,7: nothing. (tmpl = 0x082FA8E4.)
// Tail (always):
//   u16[rec+88]=6; u32[rec+100]=12; u32[rec+112]=0; DAB8(rec+16);
//   tile=0x082A798C: 7614(tile,1,0,3); 7614(tile,1,1,4);
//   798C(tmpl,rec); 7A58(rec); 798C(0x082B7410,rec+8); 75E8(tm2,0,5);
//   25BC8(rec+152,5);
//   7ABC(u32[rec+12],u32[rec+156],u32[rec+152]);
//   u32[rec+128]=rec+164; u32[rec+132]=rec+16; u32[rec+16]=0;
//   s16[rec+20]=1; D77C(rec+36,0,-32); D77C(rec+28,0,160);
//   s16[rec+24]=5; s16[rec+26]=6; u32[rec+164]=2;
//   s16[rec+148]=0; s16[rec+150]=0; 02124(0x1392);
//   if s16[rec+138]==1: 152A8(rec).
void MenuFF78_15304(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    void *tmpl = (void *)(uintptr_t)0x082FA8E4u;
    void *tile = (void *)(uintptr_t)0x082A798Cu;
    {
        s16 v = *(volatile s16 *)(uintptr_t)(wa + 0xFBCu);
        if (v >= 0 && v <= 7) {
            switch (v) {
            case 0:
                _08007770(0, tmpl, 1, 0, 4, 1);
                *(volatile s16 *)(uintptr_t)(rec + 138) = 2;
                break;
            case 3:
                _08007770(0, tmpl, 0, 0, 4, 1);
                *(volatile s16 *)(uintptr_t)(rec + 138) = 1;
                break;
            case 1: case 2: case 4: case 6:
                _08007770(0, tmpl, 0, 0, 4, 1);
                *(volatile s16 *)(uintptr_t)(rec + 138) = 0;
                break;
            default:
                break;
            }
        }
    }
    *(volatile u16 *)(uintptr_t)(rec + 88) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 100) = 12;
    *(volatile u32 *)(uintptr_t)(rec + 112) = 0;
    Sub_0800DAB8((void *)(uintptr_t)(rec + 16));
    Sub_08007614(tile, 1, 0, 3);
    Sub_08007614(tile, 1, 1, 4);
    Sub_0800798C(tmpl, rec_);
    Sub_08007A58(rec_);
    {
        void *tm2 = (void *)(uintptr_t)0x082B7410u;
        Sub_0800798C(tm2, (void *)(uintptr_t)(rec + 8));
        Sub_080075E8(tm2, 0, 5);
    }
    Sub_080025BC8((void *)(uintptr_t)(rec + 152), 5);
    Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 12),
                 *(volatile u32 *)(uintptr_t)(rec + 156),
                 *(volatile u32 *)(uintptr_t)(rec + 152));
    *(volatile u32 *)(uintptr_t)(rec + 128) = (u32)(uintptr_t)(rec + 164);
    *(volatile u32 *)(uintptr_t)(rec + 132) = (u32)(uintptr_t)(rec + 16);
    *(volatile u32 *)(uintptr_t)(rec + 16) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 20) = 1;
    Sub_0800D77C((void *)(uintptr_t)(rec + 36), 0, -32);
    Sub_0800D77C((void *)(uintptr_t)(rec + 28), 0, 160);
    *(volatile s16 *)(uintptr_t)(rec + 24) = 5;
    *(volatile s16 *)(uintptr_t)(rec + 26) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 164) = 2;
    *(volatile s16 *)(uintptr_t)(rec + 148) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 150) = 0;
    Sub_08002124(0x1392u);
    if (*(volatile s16 *)(uintptr_t)(rec + 138) == 1)
        Sub_0800152A8(rec_);
}
#ifndef __APPLE__
void _080015304(void *a) __attribute__((alias("MenuFF78_15304")));
void Sub_080015304(void *a) __attribute__((alias("MenuFF78_15304")));
void sub_080015304(void *a) __attribute__((alias("MenuFF78_15304")));
#endif

// ----------------------------------------------------------------------------
// sub_0800154E0 — (rec, dead, ev): r6 = rec+144, entry = u16[rec+144].
//   ev==2: 1549C(rec).
//   ev==1 on s = s16[rec+144]:
//     s==1: u16[WA+0x107C]=u16[rec+144]; 1549C(rec).
//     s==0: s16[rec+24]=ev; 2B368(1); u32[16]=10; s16[20]=0; u32[48]=10;
//       u32[44]=ev; u16[WA+0x107C]=u16[rec+144].
//     s in 2..4:
//       s==4 && u16[WA+0xFBC]==7: 2B368(10).
//       else: v=24068; r0 = (v==39) ? 1 : (u16[WA+0xFBC]==1 ? 7 : 5);
//         s16[rec+24]=r0; 2B368(1); u32[16]=10; s16[20]=0; u32[48]=10;
//         u32[44]=1; u16[WA+0x107C]=u16[rec+144].
//   tail on cell u16[r6]: ev==64: cell--; ev==128: cell++;
//   clamp s16 0..4; changed vs entry -> 2B368(2).
void MenuFF78_154E0(void *rec_, u32 dead, u32 ev_) {
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
        if (s == 1) {
            *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                *(volatile u16 *)(uintptr_t)cell;
            Sub_08001549C(rec_);
        } else if (s == 0) {
            *(volatile s16 *)(uintptr_t)(rec + 24) = (s16)ev;
            Sub_08002B368(1);
            *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
            *(volatile s16 *)(uintptr_t)(rec + 20) = 0;
            *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
            *(volatile u32 *)(uintptr_t)(rec + 44) = (u32)ev;
            *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                *(volatile u16 *)(uintptr_t)cell;
        } else if (s >= 2 && s <= 4) {
            if (s == 4 && *(volatile u16 *)(uintptr_t)(wa + 0xFBCu) == 7) {
                Sub_08002B368(10);
            } else {
                int r0;
                if (Sub_08024068() == 39)
                    r0 = 1;
                else if (*(volatile u16 *)(uintptr_t)(wa + 0xFBCu) == 1)
                    r0 = 7;
                else
                    r0 = 5;
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
    }
    if (ev == 64) {
        s16 c = *(volatile s16 *)(uintptr_t)cell;
        *(volatile s16 *)(uintptr_t)cell = (s16)(c - 1);
    }
    if (ev == 128) {
        s16 c = *(volatile s16 *)(uintptr_t)cell;
        *(volatile s16 *)(uintptr_t)cell = (s16)(c + 1);
    }
    if (*(volatile s16 *)(uintptr_t)cell < 0)
        *(volatile s16 *)(uintptr_t)cell = 0;
    if (*(volatile s16 *)(uintptr_t)cell > 4)
        *(volatile s16 *)(uintptr_t)cell = 4;
    if (*(volatile u16 *)(uintptr_t)cell != entry)
        Sub_08002B368(2);
}
#ifndef __APPLE__
void _0800154E0(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_154E0")));
void Sub_0800154E0(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_154E0")));
void sub_0800154E0(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_154E0")));
#endif
