// ============================================================================
// course_resource_helpers.c — C lift of asm/course_resource_helpers.s
// (VMA 0x08007770–0x0800798C), the DMA3 VRAM/palette setup helpers.
//
// Transcribed instruction-for-instruction from the cited asm listing.
// All four functions program DMA3 channel registers (SAD/DAD/CNT at
// 0x040000D4/0xD8/0xDC) via the resource-record accessors.
//
// Resource-record accessors used here (all lifted or asm-bound):
//   _08007498(X, i) — seek child record i of X
//   _0800748C(X)    — resolve the record-array base of X
//   _080074A8(X)    — read u32 record attribute at +4
//   _08007658(X)    — 074A8 then >> 5
//   _080050D0(a, b) — palette/bank slot setup
//   _08005260(a, b, c) — tile blit setup
//   _080053DC(...)  — grid blit (event_dma_queue.c)
//   _080054A4(...)  — DMA3 row copy (event_dma_queue.c)
// ============================================================================

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) void *_08007498(void *x, int i) { (void)x; (void)i; return 0; }
__attribute__((weak)) void *_0800748C(void *x) { (void)x; return 0; }
__attribute__((weak)) u32  _080074A8(void *x) { (void)x; return 0; }
__attribute__((weak)) u32 _08007658(void *x) { (void)x; return 0; }
__attribute__((weak)) int  _080050D0(int a, int b) { (void)a; return 0; }
__attribute__((weak)) void _08005260(void *a, void *b, int c, int d) { (void)a; (void)b; (void)c; (void)d; }
__attribute__((weak)) void _080053DC(int a, const volatile u8 *b, int c, int d, int e, int f, int g, int h) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h; }
__attribute__((weak)) void _080054A4(int a, int b, int c, int d, int e) { (void)a;(void)b;(void)c;(void)d;(void)e; }
#else
extern void *_08007498(void *x, int i);       // 0x08007498 (course_resource.c)
extern void *_0800748C(void *x);              // 0x0800748C (course_resource.c)
extern u32  _080074A8(void *x);               // 0x080074A8 (course_resource.c)
extern u32 _08007658(void *x);               // 0x08007658 (course_resource.c)
extern int  _080050D0(int a, int b);          // 0x080050D0 (event_dma_queue.c)
extern void _08005260(void *a, void *b, int c, int d); // 0x08005260 4-arg (event_dma_queue.c)
extern void _080053DC(int a, const volatile u8 *b, int c, int d, int e, int f, int g, int h); // 0x080053DC
extern void _080054A4(int a, int b, int c, int d, int e);                       // 0x080054A4
#endif

// DMA3 channel register base (byte-exact pools _08007858/_080078E8/_08007920/_08007974):
#define CRH_DMA3 ((volatile u32 *)(uintptr_t)0x040000D4)

u32 Course_0x08007770(int ctx, void *X, int r2in, int r3in, u32 s0, u32 s1) {
    void *r4 = _08007498(X, r2in);                 // bl 07498 (r1=r2)
    void *r5 = _08007498(X, 0);                    // movs r1,#0
    void *r6 = _08007498(r4, 1);                   // movs r1,#1
    void *r7 = _08007498(r4, 2);                   // movs r1,#2
    u8 *r8 = (u8 *)_0800748C(r4);                  // bl 0748C
    int r9 = _080050D0((int)(uintptr_t)ctx, (int)_08007658(r5)); // bl 07658; mov r0,sl; bl 050D0
    void *r4b = _0800748C(r5);                     // bl 0748C (r0=r5)
    u32 r3dead = _080074A8(r5);                    // bl 074A8 (r0=r5) -> r3 for 05260
    _08005260((void *)(uintptr_t)(u32)ctx, r4b, r9, (int)r3dead); // bl 05260 (r0=sl, r1=r4, r2=r9, r3=r3)

    // 053DC src ptr = 0748C(r7); lanes from r8 halfwords.
    const volatile u8 *src = (const volatile u8 *)_0800748C(r7);
    u16 w0 = *(volatile u16 *)(r8 + 0);
    u16 w2 = *(volatile u16 *)(r8 + 2);
    int stk0 = (int)(w0 >> 3);
    int stk1 = (int)(w2 >> 3);
    _080053DC((int)(uintptr_t)ctx, src, r3in, (int)s0, stk0, stk1, r9, (int)s1);

    // Single DMA triple (pool _08007858 = 0x040000D4).
    volatile u32 *dma = CRH_DMA3;
    dma[0] = (u32)(uintptr_t)_0800748C(r6);
    dma[1] = 0x05000000u + (s1 << 5);              // 160<<19 = 0x05000000
    u32 cnt = _080074A8(r6);
    dma[2] = (cnt >> 2) | 0x84000000u;             // movs r1,#132; lsls #24
    (void)dma[2];

    // Row copy: 080054A4(slot=ctx, offset=r3in, row=s0, bytes=w0>>3, count=w2>>3).
    _080054A4((int)(uintptr_t)ctx, r3in, (int)s0, stk0, stk1);
    return (u32)r9;
}
#ifndef __APPLE__
u32 _08007770(int a, void *b, int c, int d, u32 e, u32 f) __attribute__((alias("Course_0x08007770")));
u32 sub_08007770(int a, void *b, int c, int d, u32 e, u32 f) __attribute__((alias("Course_0x08007770")));
#endif

// ----------------------------------------------------------------------------
// sub_0800785C — 4-DMA variant with sl = r0 (r0..r3 args):
//   r4 = 07498(r1, r2); r5 = 07498(r1, 0); r8 = 07498(r4, 1);
//   r9 = 050D0(r0, 07658(r5)); r4v = 0748C(r5); r3 = 074A8(r5);
//   05260(r0, r4v, r9); DMA triple = {0748C(r8), 0x05000000+(r3<<5), 074A8(r8)>>2|0x84000000}
void Course_0x0800785C(void *ctx, void *X, int rec_arg, int lane) {
    void *r4 = _08007498(X, rec_arg);
    void *r5 = _08007498(X, 0);
    void *r8 = _08007498(r4, 1);
    int r9 = _080050D0((int)(uintptr_t)ctx, (int)_08007658(r5));
    u32 r4v = (u32)(uintptr_t)_0800748C(r5);
    _08005260(ctx, (void *)(uintptr_t)r4v, r9, (int)_080074A8(r5)); // r3 = 074A8(r5)

    volatile u32 *dma = CRH_DMA3;
    dma[0] = (u32)(uintptr_t)_0800748C(r8);
    u32 w = (u32)lane << 5;
    dma[1] = 0x06000000u + w;                            // 160<<19
    u32 cnt = _080074A8(r8);
    dma[2] = (cnt >> 2) | 0x84000000u;
    (void)dma[2];
}
#ifndef __APPLE__
void _0800785C(void *a, void *b, int c, int d) __attribute__((alias("Course_0x0800785C")));
void sub_0800785C(void *a, void *b, int c, int d) __attribute__((alias("Course_0x0800785C")));
#endif

// ----------------------------------------------------------------------------
// sub_080078EC — 3-word DMA descriptor write:
//   r4 = 07498(r1, r2); r5 = 07498(r4, 0);
//   DMA = {0748C(r5), r3 (r6 arg), 074A8(r5)>>2 | 0x84000000}
void Course_0x080078EC(void *X, void *rec_arg, int lane) {
    void *r4 = _08007498(X, (int)(uintptr_t)rec_arg);
    void *r5 = _08007498(r4, 0);
    volatile u32 *dma = CRH_DMA3;
    dma[0] = (u32)(uintptr_t)_0800748C(r5);
    dma[1] = (u32)lane;
    u32 cnt = _080074A8(r5);
    dma[2] = (cnt >> 2) | 0x84000000u;
    (void)dma[2];
}
#ifndef __APPLE__
void _080078EC(void *a, void *b, int c) __attribute__((alias("Course_0x080078EC")));
void sub_080078EC(void *a, void *b, int c) __attribute__((alias("Course_0x080078EC")));
#endif

void *Course_0x08007924(void *X, int y) {
    void *r = _08007498(X, y);
    return _0800748C(_08007498(r, 2));
}
#ifndef __APPLE__
void *_08007924(void *a, int b) __attribute__((alias("Course_0x08007924")));
void *sub_08007924(void *a, int b) __attribute__((alias("Course_0x08007924")));
#endif

// ----------------------------------------------------------------------------
// sub_08007938 — 3-word DMA descriptor write (lane variant):
//   r4 = 07498(r1, r2); r6 = 07498(r4, 1);
//   DMA = {0748C(r6), (r3<<5) + 0x05000000, 074A8(r6)>>2 | 0x84000000}
void Course_0x08007938(void *X, void *rec_arg, int lane) {
    void *r4 = _08007498(X, (int)(uintptr_t)rec_arg);
    void *r6 = _08007498(r4, 1);
    volatile u32 *dma = CRH_DMA3;
    dma[0] = (u32)(uintptr_t)_0800748C(r6);
    u32 w = (u32)lane << 5;
    dma[1] = 0x05000000u + w;                            // 160<<19 = 0x05000000
    u32 cnt = _080074A8(r6);
    dma[2] = (cnt >> 2) | 0x84000000u;
    (void)dma[2];
}
#ifndef __APPLE__
void _08007938(void *a, void *b, int c) __attribute__((alias("Course_0x08007938")));
void sub_08007938(void *a, void *b, int c) __attribute__((alias("Course_0x08007938")));
#endif

// ROM entry alias.
#ifndef __APPLE__
u32 Sub_08007770(int ctx, void *X, int r2in, int r3in, u32 s0, u32 s1) __attribute__((alias("Course_0x08007770")));
#endif
