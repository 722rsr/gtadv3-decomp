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
extern void *_08007498(void *x, int i);       // 0x08007498 (course_resource.c)
extern void *_0800748C(void *x);              // 0x0800748C (course_resource.c)
extern u32  _080074A8(void *x);               // 0x080074A8 (course_resource.c)
extern u32 _08007658(void *x);               // 0x08007658 (course_resource.c)
extern int  _080050D0(int a, int b);          // 0x080050D0 (event_dma_queue.c)
extern void _08005260(void *a, void *b, int c, int d); // 0x08005260 4-arg (event_dma_queue.c)
extern void _080053DC(int a, const volatile u8 *b, int c, int d, int e, int f, int g, int h); // 0x080053DC
extern void _080054A4(int a, int b, int c, int d, int e);                       // 0x080054A4

// DMA3 channel register base (byte-exact pools _08007858/_080078E8/_08007920/_08007974):
#define CRH_DMA3 ((volatile u32 *)(uintptr_t)0x040000D4)

__attribute__((naked)) u32 Course_0x08007770(int ctx, void *X, int r2in, int r3in, u32 s0, u32 s1) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, sl\n"
        "mov r6, r9\n"
        "mov r5, r8\n"
        "push {r5, r6, r7}\n"
        "sub sp, #28\n"
        "mov sl, r0\n"
        "adds r0, r1, #0\n"
        "adds r1, r2, #0\n"
        "str r3, [sp, #16]\n"
        "bl _08007498\n"
        "adds r4, r0, #0\n"
        "movs r1, #0\n"
        "bl _08007498\n"
        "adds r5, r0, #0\n"
        "adds r0, r4, #0\n"
        "movs r1, #1\n"
        "bl _08007498\n"
        "adds r6, r0, #0\n"
        "adds r0, r4, #0\n"
        "movs r1, #2\n"
        "bl _08007498\n"
        "adds r7, r0, #0\n"
        "adds r0, r4, #0\n"
        "bl _0800748C\n"
        "mov r8, r0\n"
        "adds r0, r5, #0\n"
        "bl _08007658\n"
        "adds r1, r0, #0\n"
        "mov r0, sl\n"
        "bl _080050D0\n"
        "mov r9, r0\n"
        "adds r0, r5, #0\n"
        "bl _0800748C\n"
        "adds r4, r0, #0\n"
        "adds r0, r5, #0\n"
        "bl _080074A8\n"
        "adds r3, r0, #0\n"
        "mov r0, sl\n"
        "adds r1, r4, #0\n"
        "mov r2, r9\n"
        "bl _08005260\n"
        "adds r0, r7, #0\n"
        "bl _0800748C\n"
        "adds r1, r0, #0\n"
        "mov r2, r8\n"
        "ldrh r2, [r2, #0]\n"
        "lsrs r0, r2, #3\n"
        "str r0, [sp, #0]\n"
        "mov r3, r8\n"
        "ldrh r3, [r3, #2]\n"
        "lsrs r0, r3, #3\n"
        "str r0, [sp, #4]\n"
        "mov r0, r9\n"
        "str r0, [sp, #8]\n"
        "ldr r2, [sp, #64]\n"
        "str r2, [sp, #12]\n"
        "mov r0, sl\n"
        "ldr r2, [sp, #16]\n"
        "ldr r3, [sp, #60]\n"
        "bl _080053DC\n"
        "ldr r4, .L07770_dma\n"
        "adds r0, r6, #0\n"
        "bl _0800748C\n"
        "str r0, [r4, #0]\n"
        "ldr r3, [sp, #64]\n"
        "lsls r3, r3, #5\n"
        "str r3, [sp, #20]\n"
        "movs r0, #160\n"
        "lsls r0, r0, #19\n"
        "adds r0, r3, r0\n"
        "str r0, [r4, #4]\n"
        "adds r0, r6, #0\n"
        "bl _080074A8\n"
        "lsrs r0, r0, #2\n"
        "movs r1, #132\n"
        "lsls r1, r1, #24\n"
        "orrs r0, r1\n"
        "str r0, [r4, #8]\n"
        "ldr r0, [r4, #8]\n"
        "mov r1, r8\n"
        "ldrh r1, [r1, #0]\n"
        "lsrs r3, r1, #3\n"
        "mov r2, r8\n"
        "ldrh r2, [r2, #2]\n"
        "lsrs r0, r2, #3\n"
        "str r0, [sp, #0]\n"
        "mov r0, sl\n"
        "ldr r1, [sp, #16]\n"
        "ldr r2, [sp, #60]\n"
        "bl _080054A4\n"
        "mov r0, r9\n"
        "add sp, #28\n"
        "pop {r3, r4, r5}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "mov sl, r5\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r1}\n"
        "bx r1\n"
        ".short 0\n"
        ".L07770_dma: .word 0x040000D4\n"
        ".syntax divided\n"
    );
}
u32 _08007770(int a, void *b, int c, int d, u32 e, u32 f) __attribute__((alias("Course_0x08007770")));
u32 sub_08007770(int a, void *b, int c, int d, u32 e, u32 f) __attribute__((alias("Course_0x08007770")));

// ----------------------------------------------------------------------------
// sub_0800785C — 4-arg DMA variant with sl = ctx (r0..r3 args):
//   r4 = 07498(X, rec); r5 = 07498(r4, 0); r8 = 07498(r4, 1);
//   r9 = 050D0(ctx, 07658(r5)); r4v = 0748C(r5); r3 = 074A8(r5);
//   05260(ctx, r4v, r9, r3); DMA triple = {0748C(r8), 0x05000000+(lane<<5),
//   074A8(r8)>>2|0x84000000}; returns the 050D0 result (r9), so the epilogue
//   is `pop {r1}; bx r1`, not the void `pop {r0}; bx r0`.
//   (The second lookup feeds off r4, as in the 078EC sibling -- not off X.
//   The DMA base is 160<<19 = 0x05000000, as in the 07770 sibling.)
int Course_0x0800785C(void *ctx, void *X, int rec_arg, int lane) {
    void *r4 = _08007498(X, rec_arg);
    void *r5 = _08007498(r4, 0);
    void *r8 = _08007498(r4, 1);
    int r9 = _080050D0((int)(uintptr_t)ctx, (int)_08007658(r5));
    u32 r4v = (u32)(uintptr_t)_0800748C(r5);
    _08005260(ctx, (void *)(uintptr_t)r4v, r9, (int)_080074A8(r5)); // r3 = 074A8(r5)

    volatile u32 *dma = CRH_DMA3;
    dma[0] = (u32)(uintptr_t)_0800748C(r8);
    u32 w = (u32)lane << 5;
    dma[1] = 0x05000000u + w;
    u32 cnt = _080074A8(r8);
    dma[2] = (cnt >> 2) | 0x84000000u;
    (void)dma[2];
    return r9;
}
int _0800785C(void *a, void *b, int c, int d) __attribute__((alias("Course_0x0800785C")));
int sub_0800785C(void *a, void *b, int c, int d) __attribute__((alias("Course_0x0800785C")));

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
void _080078EC(void *a, void *b, int c) __attribute__((alias("Course_0x080078EC")));
void sub_080078EC(void *a, void *b, int c) __attribute__((alias("Course_0x080078EC")));

void *Course_0x08007924(void *X, int y) {
    void *r = _08007498(X, y);
    return _0800748C(_08007498(r, 2));
}
void *_08007924(void *a, int b) __attribute__((alias("Course_0x08007924")));
void *sub_08007924(void *a, int b) __attribute__((alias("Course_0x08007924")));

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
void _08007938(void *a, void *b, int c) __attribute__((alias("Course_0x08007938")));
void sub_08007938(void *a, void *b, int c) __attribute__((alias("Course_0x08007938")));

// ROM entry alias.
u32 Sub_08007770(int ctx, void *X, int r2in, int r3in, u32 s0, u32 s1) __attribute__((alias("Course_0x08007770")));
