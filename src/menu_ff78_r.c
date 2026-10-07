// ============================================================================
// menu_ff78_r.c — reconstructed C for asm/menu_ff78.s (1 function).
//
// Body transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080013108 (0x080013108) — (rec) full setup + DMA table fill.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; }
__attribute__((weak)) void Sub_0800DAB8(void *a) { (void)a; }
__attribute__((weak)) void Sub_080075E8(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800798C(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_08007A58(void *a) { (void)a; }
__attribute__((weak)) void Sub_08007ABC(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800D77C(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) int Sub_08002124(u32 v) { (void)v; return 0; }
__attribute__((weak)) void Sub_08002B214(int v) { (void)v; }
__attribute__((weak)) void Sub_080012E5C(int v) { (void)v; }
__attribute__((weak)) int Sub_080012E74(int v) { (void)v; return 0; }
__attribute__((weak)) void Sub_080012F98(void *a) { (void)a; }
__attribute__((weak)) void Sub_080013024(void *a) { (void)a; }
__attribute__((weak)) u32 Sub_0800572C(int v) { (void)v; return 0; }
__attribute__((weak)) int Sub_08024C3C(int v) { return v; }
__attribute__((weak)) int Sub_08024C58(int v) { return v; }
__attribute__((weak)) void Sub_0800D9A4(void *a, int b, int c) { (void)a; (void)b; (void)c; }
#else
extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
extern void Sub_0800DAB8(void *a);
extern void Sub_080075E8(void *a, int b, int c);
extern void Sub_0800798C(void *a, void *b);
extern void Sub_08007A58(void *a);
extern void Sub_08007ABC(u32 a, u32 b, u32 c);
extern void Sub_0800D77C(void *a, int b, int c);
extern int Sub_08002124(u32 v);
extern void Sub_08002B214(int v);
extern void Sub_080012E5C(int v);
extern int Sub_080012E74(int v);
extern void Sub_080012F98(void *a);
extern void Sub_080013024(void *a);
extern u32 Sub_0800572C(int v);
extern int Sub_08024C3C(int v);
extern int Sub_08024C58(int v);
extern void Sub_0800D9A4(void *a, int b, int c);
#endif

// ----------------------------------------------------------------------------
// sub_080013108 — (rec):
//   [u16[WA+0xFBC]==3] 02124(0x1393); s16[rec+138] = (u16==7) ? 0 : 1.
//   2B214(49); 07770(0, 0x082EE524, 3, 0, 4, 1);
//   u32[104]=6; u32[116]=4; u32[128]=1; DAB8(rec+32);
//   tile=0x082A798C: 75E8(tile,0,3); 75E8(tile,2,7);
//   798C(0x082EE524,rec); 75E8(tm,0,4); 75E8(tm,1,6); 75E8(tm,2,8);
//   7A58(rec); s16[138]: 0->12F98(rec), 1->13024(rec).
//   75E8(0x082C4228,0,5); 798C(0x082ECAE4,rec+8);
//   12E5C(s16[176]); u32[584]=0572C(16); u32[588]=(u32)12E74(s16[176]);
//   7ABC(u32[12], e74, u32[584]);
//   798C(0x082C5040,rec+16); u32[596]=0572C(14); u32[600]=5;
//   7ABC(u32[20],5,u32[596]);
//   798C(0x082D0DC0,rec+24); u32[608]=0572C(7); u32[612]=50;
//   7ABC(u32[28],50,u32[608]);
//   u32[32]=0; s16[36]=1; D77C(52,0,-32); D77C(44,0,160);
//   s16[40]=6; s16[42]=5;
//   D9A4(rec+580, s16[174], s16[172]);
//   u32[600]=24C3C(s16[174]); 7ABC(u32[20],u32[600],u32[596]);
//   u32[612]=24C58(s16[174]); 7ABC(u32[28],u32[612],u32[608]);
//   DMA: for k in 1..5: [0x040000D4] = (u32)(0x080CD7A8 +
//     (s8)u8[0x080CCEEC + s16[174]*20 + k]*2);
//     [..+4] = 0x05000280+k*2; [..+8] = 0x80000001 (+ dummy read).
void MenuFF78_13108(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    void *tmpl = (void *)(uintptr_t)0x082EE524u;
    void *tile = (void *)(uintptr_t)0x082A798Cu;
    if (*(volatile u16 *)(uintptr_t)(wa + 0xFBCu) == 3)
        Sub_08002124(0x1393u);
    *(volatile s16 *)(uintptr_t)(rec + 138) =
        (*(volatile u16 *)(uintptr_t)(wa + 0xFBCu) == 7) ? 0 : 1;
    Sub_08002B214(49);
    _08007770(0, tmpl, 3, 0, 4, 1); // R2 C body (was Sub_ veneer)
    *(volatile u32 *)(uintptr_t)(rec + 104) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 116) = 4;
    *(volatile u32 *)(uintptr_t)(rec + 128) = 1;
    Sub_0800DAB8((void *)(uintptr_t)(rec + 32));
    Sub_080075E8(tile, 0, 3);
    Sub_080075E8(tile, 2, 7);
    Sub_0800798C(tmpl, rec_);
    Sub_080075E8(tmpl, 0, 4);
    Sub_080075E8(tmpl, 1, 6);
    Sub_080075E8(tmpl, 2, 8);
    Sub_08007A58(rec_);
    {
        s16 s = *(volatile s16 *)(uintptr_t)(rec + 138);
        if (s == 0)
            Sub_080012F98(rec_);
        else if (s == 1)
            Sub_080013024(rec_);
    }
    Sub_080075E8((void *)(uintptr_t)0x082C4228u, 0, 5);
    Sub_0800798C((void *)(uintptr_t)0x082ECAE4u,
                 (void *)(uintptr_t)(rec + 8));
    Sub_080012E5C((int)*(volatile s16 *)(uintptr_t)(rec + 176));
    *(volatile u32 *)(uintptr_t)(rec + 584) = Sub_0800572C(16);
    *(volatile u32 *)(uintptr_t)(rec + 588) =
        (u32)Sub_080012E74((int)*(volatile s16 *)(uintptr_t)(rec + 176));
    Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 12),
                 *(volatile u32 *)(uintptr_t)(rec + 588),
                 *(volatile u32 *)(uintptr_t)(rec + 584));
    Sub_0800798C((void *)(uintptr_t)0x082C5040u,
                 (void *)(uintptr_t)(rec + 16));
    *(volatile u32 *)(uintptr_t)(rec + 596) = Sub_0800572C(14);
    *(volatile u32 *)(uintptr_t)(rec + 600) = 5;
    Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 20), 5,
                 *(volatile u32 *)(uintptr_t)(rec + 596));
    Sub_0800798C((void *)(uintptr_t)0x082D0DC0u,
                 (void *)(uintptr_t)(rec + 24));
    *(volatile u32 *)(uintptr_t)(rec + 608) = Sub_0800572C(7);
    *(volatile u32 *)(uintptr_t)(rec + 612) = 50;
    Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 28), 50,
                 *(volatile u32 *)(uintptr_t)(rec + 608));
    *(volatile u32 *)(uintptr_t)(rec + 32) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 36) = 1;
    Sub_0800D77C((void *)(uintptr_t)(rec + 52), 0, -32);
    Sub_0800D77C((void *)(uintptr_t)(rec + 44), 0, 160);
    *(volatile s16 *)(uintptr_t)(rec + 40) = 6;
    *(volatile s16 *)(uintptr_t)(rec + 42) = 5;
    Sub_0800D9A4((void *)(uintptr_t)(rec + 580),
                 (int)*(volatile s16 *)(uintptr_t)(rec + 174),
                 (int)*(volatile s16 *)(uintptr_t)(rec + 172));
    {
        int v = Sub_08024C3C((int)*(volatile s16 *)(uintptr_t)(rec + 174));
        *(volatile u32 *)(uintptr_t)(rec + 600) = (u32)v;
        Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 20), (u32)v,
                     *(volatile u32 *)(uintptr_t)(rec + 596));
    }
    {
        int v = Sub_08024C58((int)*(volatile s16 *)(uintptr_t)(rec + 174));
        *(volatile u32 *)(uintptr_t)(rec + 612) = (u32)v;
        Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 28), (u32)v,
                     *(volatile u32 *)(uintptr_t)(rec + 608));
    }
    {
        s16 i = *(volatile s16 *)(uintptr_t)(rec + 174);
        u32 row = (u32)(s32)i * 20;
        volatile u32 *dma = (volatile u32 *)(uintptr_t)0x040000D4u;
        for (u32 k = 1; k <= 5; k++) {
            s8 b = (s8)*(volatile u8 *)(uintptr_t)(0x080CCEECu + row + k);
            dma[0] = 0x080CD7A8u + (u32)((s32)b * 2);
            dma[1] = 0x05000280u + k * 2;
            dma[2] = 0x80000001u;
            (void)dma[2];
        }
    }
}
#ifndef __APPLE__
void _080013108(void *a) __attribute__((alias("MenuFF78_13108")));
void Sub_080013108(void *a) __attribute__((alias("MenuFF78_13108")));
void sub_080013108(void *a) __attribute__((alias("MenuFF78_13108")));
#endif
