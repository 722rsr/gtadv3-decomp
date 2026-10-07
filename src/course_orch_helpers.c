// ============================================================================
// course_orch_helpers.c — C lift of the 4 remaining course_orch.s functions
// (asm/course_orch.s, VMA 0x080061B8 / 0x0800628C / 0x08006468 / 0x080064EC).
//
// Transcribed instruction-for-instruction from the asm listing. All four are
// resource-package loaders driven by the directory seeker _08006590
// (already lifted as Course_SeekRes in src/course_orch.c):
//   _080064EC — group-0 header parser -> IWRAM 0x030039D0 course struct
//   _08006468 — theme tiles + palette + LUT (groups 2/3/4)
//   _080061B8 — surface variant gfx + pal (groups 5/6/7)
//   _0800628C — big gfx + palettes + minimap tiles (groups 8/9/10)
//
// Variant cell: s16 pair at 0x0203F758 ([+0] course form, [+2] variant).
// ============================================================================

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) void *_08006590(void *base, int grp, int idx) { (void)base; (void)grp; (void)idx; return 0; }
__attribute__((weak)) void  sub_0802D984(const void *src, void *dst) { (void)src; (void)dst; }
__attribute__((weak)) void  sub_0802D988(const void *src, void *dst) { (void)src; (void)dst; }
#else
extern void *_08006590(void *base, int grp, int idx);      // 0x08006590 (course_orch.c)
extern void  sub_0802D984(const void *src, void *dst);     // LZ77UnCompVram (bios_wrappers.c)
extern void  sub_0802D988(const void *src, void *dst);     // LZ77UnCompWram (bios_wrappers.c)
#endif

#define CO_DMA3   ((volatile u32 *)(uintptr_t)0x040000D4)
#define CO_VAR    ((volatile u8 *)(uintptr_t)0x0203F758)
#define CO_WA     ((volatile u8 *)(uintptr_t)0x03001780)
#define CO_EWRAM  ((volatile u8 *)(uintptr_t)0x02000000)

// ----------------------------------------------------------------------------
// _080064EC(out, idx, base) — GROUP-0 HEADER PARSER.
//   payload = _08006590(base, 0, idx); out layout (tools/track_dump.py):
//     +0x28 nA=u16[p+2]  +0x2A nB=u16[p+4]  +0x2C nC=u16[p+6]
//     +0x2E nD=u16[p+8]  +0x30 c5=u16[p+10]
//     +0x00 payload+12   +0x04 payload+28
//     +0x08 +0x04 + nA*20   +0x0C +0x08 + nB*12
//     +0x10 +0x0C + nC*12   +0x14 +0x10 + nD*8
//     +0x18 +0x14 + 60
//     +0x32 flag = ((u8)(p[14]-6) <= 1)
void CourseOrch_080064EC(void *out, int idx, void *base) {
    volatile u8 *o = (volatile u8 *)out;
    volatile u8 *p = (volatile u8 *)(uintptr_t)_08006590(base, 0, idx);

    *(volatile u16 *)(o + 0x28) = *(volatile u16 *)(p + 2);
    *(volatile u16 *)(o + 0x2A) = *(volatile u16 *)(p + 4);
    *(volatile u16 *)(o + 0x2C) = *(volatile u16 *)(p + 6);
    *(volatile u16 *)(o + 0x2E) = *(volatile u16 *)(p + 8);
    *(volatile u16 *)(o + 0x30) = *(volatile u16 *)(p + 10);

    *(volatile u32 *)(o + 0x00) = (u32)(uintptr_t)(p + 12);
    *(volatile u32 *)(o + 0x04) = (u32)(uintptr_t)(p + 28);

    s16 nA = *(volatile s16 *)(p + 2);
    u32 r1 = (u32)(uintptr_t)(p + 28) + (u32)((nA << 2) + nA) * 4;   // nA*20
    *(volatile u32 *)(o + 0x08) = r1;

    s16 nB = *(volatile s16 *)(p + 4);
    r1 += (u32)((nB << 1) + nB) * 4;                                  // nB*12
    *(volatile u32 *)(o + 0x0C) = r1;

    s16 nC = *(volatile s16 *)(p + 6);
    r1 += (u32)((nC << 1) + nC) * 4;                                  // nC*12
    *(volatile u32 *)(o + 0x10) = r1;

    s16 nD = *(volatile s16 *)(p + 8);
    r1 += (u32)(nD << 3);                                             // nD*8
    *(volatile u32 *)(o + 0x14) = r1;

    r1 += 60;
    *(volatile u32 *)(o + 0x18) = r1;

    u8 f = (u8)(p[14] - 6);
    *(volatile u16 *)(o + 0x32) = (u16)(f <= 1 ? 1 : 0);
}
#ifndef __APPLE__
void _080064EC(void *a, int b, void *c) __attribute__((alias("CourseOrch_080064EC")));
void sub_080064EC(void *a, int b, void *c) __attribute__((alias("CourseOrch_080064EC")));
#endif

// ----------------------------------------------------------------------------
// _080061B8(arg0 unused, idx, base) — surface VARIANT gfx (groups 5/6/7).
//   variant r5 from the s16[0x0203F758] pair:
//     [+2]==0: [+0]==1 -> 1; [+0]==2 -> 2; else 0
//     [+2]==1: [+0]==0 -> 3; [+0]==1 -> 4; [+0]==2 -> 5; else 0
//     [+2]==2: 6
//     else: 0
//   g5[v] LZ77 -> VRAM 0x06008000; g6[v] DMA3 -> pal 0x050001E0 (16 hw);
//   g7[v] LZ77 -> VRAM 0x0600E000; DMA fill 0x0600E180 with 0 halfwords
//   (0xC0 words, ctrl 0x810000C0).
void CourseOrch_080061B8(void *unused, int idx, void *base) {
    (void)unused;
    s32 hi = *(volatile s16 *)(CO_VAR + 2);
    s32 lo = *(volatile s16 *)(CO_VAR + 0);
    u32 r5 = 0;
    if (hi == 0) {
        if (lo == 1)
            r5 = 1;
        else if (lo == 2)
            r5 = 2;
    } else if (hi == 1) {
        if (lo == 0)
            r5 = 3;
        else if (lo == 1)
            r5 = 4;
        else if (lo == 2)
            r5 = 5;
    } else if (hi == 2) {
        r5 = 6;
    }

    void *src = _08006590(base, 5, (int)r5);
    sub_0802D984(src, (void *)(uintptr_t)0x06008000u);

    src = _08006590(base, 6, (int)r5);
    CO_DMA3[0] = (u32)(uintptr_t)src;
    CO_DMA3[1] = 0x050001E0u;
    CO_DMA3[2] = 0x80000010u;
    (void)CO_DMA3[2];

    src = _08006590(base, 7, (int)r5);
    sub_0802D984(src, (void *)(uintptr_t)0x0600E000u);

    u16 zero = 0;
    CO_DMA3[0] = (u32)(uintptr_t)&zero;
    CO_DMA3[1] = 0x0600E180u;
    CO_DMA3[2] = 0x810000C0u;
    (void)CO_DMA3[2];
}
#ifndef __APPLE__
void _080061B8(void *a, int b, void *c) __attribute__((alias("CourseOrch_080061B8")));
void sub_080061B8(void *a, int b, void *c) __attribute__((alias("CourseOrch_080061B8")));
#endif

// ----------------------------------------------------------------------------
// _0800628C(hdr, idx unused, base) — big gfx + palettes + minimap (8/9/10).
//   theme t = payload[3] (g0 payload+15); variant r7 from the s16 pair:
//     [+0]==0: r5==1 -> 2 else 0
//     [+0]==1: r7=1; r5==1 -> 3
//     [+0]==2: if r5==1 -> 1; then t remap {2->8,3->9,4->10,5->11,6->12}
//     else 0
//   if s16[+2]==2: t += 13
//   g8[t] LZ77 -> VRAM 0x0600A000; g9[t] DMA3 -> 0x050001C0 + r7*32;
//   g10[t] LZ77 -> EWRAM 0x02000000, tile-index fixup (+0x100 in low 10
//   bits), fill run with 0x100, then 20-entry DMA row list to
//   0x0600F000/0x0600F800 (64 B rows, ctrl 0x80000020).
void CourseOrch_0800628C(void *hdr, int idx_unused, void *base) {
    (void)idx_unused;
    volatile u8 *payload = (volatile u8 *)(uintptr_t)*(volatile u32 *)((u8 *)hdr + 0);
    u32 t = payload[3];

    s32 hi = *(volatile s16 *)(CO_VAR + 2);
    s32 lo = *(volatile s16 *)(CO_VAR + 0);
    u32 r7 = 0;
    if (lo == 0) {
        if (hi == 1)
            r7 = 2;
    } else if (lo == 1) {
        r7 = 1;
        if (hi == 1)
            r7 = 3;
    } else if (lo == 2) {
        if (hi == 1)
            r7 = 1;
        // theme remap
        if (t == 2) t = 8;
        else if (t == 3) t = 9;
        else if (t == 4) t = 10;
        else if (t == 5) t = 11;
        else if (t == 6) t = 12;
    }
    if (hi == 2)
        t += 13;

    void *src = _08006590(base, 8, (int)t);
    sub_0802D984(src, (void *)(uintptr_t)0x0600A000u);

    src = _08006590(base, 9, (int)t);
    CO_DMA3[0] = (u32)(uintptr_t)src + (r7 << 5);
    CO_DMA3[1] = 0x050001C0u;
    CO_DMA3[2] = 0x80000010u;
    (void)CO_DMA3[2];

    src = _08006590(base, 10, (int)t);
    sub_0802D988(src, (void *)(uintptr_t)CO_EWRAM);

    // tile-index fixup + fill + DMA row list (path A/B differ only in counts)
    volatile u16 *r4 = (volatile u16 *)(uintptr_t)CO_EWRAM;
    u32 halfwords, fill_hw;
    if (hi == 2) {
        halfwords = 704;    // movs r3,#176; lsls #2
        fill_hw = 144;      // movs r3,#144; lsls #2
    } else {
        halfwords = 512;    // movs r3,#128; lsls #2
        fill_hw = 768;      // movs r3,#192; lsls #2
    }
    for (u32 i = 0; i < halfwords; i++) {
        u16 v = r4[i];
        u16 low10 = (u16)(v & 0x03FFu);
        u16 high = (u16)(v & 0xFC00u);
        r4[i] = (u16)(low10 + 0x0100u) | high;
    }
    for (u32 i = 0; i < fill_hw; i++) {
        r4[halfwords + i] = 0x0100u;
    }

    // 20-entry DMA list: even row -> 0x0600F000+, odd row -> 0x0600F800+
    volatile u8 *srcA = CO_EWRAM;         // r1
    volatile u8 *srcB = CO_EWRAM + 64;    // r4
    u32 dadA = 0x0600F000u;               // r5
    u32 dadB = 0x0600F800u;               // r6
    for (u32 i = 0; i <= 19; i++) {
        CO_DMA3[0] = (u32)(uintptr_t)srcA;
        CO_DMA3[1] = dadA;
        CO_DMA3[2] = 0x80000020u;
        (void)CO_DMA3[2];
        CO_DMA3[0] = (u32)(uintptr_t)srcB;
        CO_DMA3[1] = dadB;
        CO_DMA3[2] = 0x80000020u;
        (void)CO_DMA3[2];
        srcB += 128;
        srcA += 128;
        dadB += 64;
        dadA += 64;
    }
}
#ifndef __APPLE__
void _0800628C(void *a, int b, void *c) __attribute__((alias("CourseOrch_0800628C")));
void sub_0800628C(void *a, int b, void *c) __attribute__((alias("CourseOrch_0800628C")));
#endif

// ----------------------------------------------------------------------------
// _08006468(out, idx, base) — theme TILES + PALETTE + LUT (groups 2/3/4).
//   cup tier c = payload[1] (g0 payload+13);
//   if u16[0x0300287C] == 10 -> c = 7
//   else if u16[0x0203F758] == 2 -> {2->5, 3->6}
//   g2[c] LZ77 -> VRAM 0x06000000; g4[c] DMA3 -> pal 0x05000000 (160 hw);
//   g3[c] raw ROM ptr kept at out+0x24.
void CourseOrch_08006468(void *out, int idx, void *base) {
    volatile u8 *o = (volatile u8 *)out;
    volatile u8 *payload = (volatile u8 *)(uintptr_t)*(volatile u32 *)o;
    u32 c = payload[1];

    if (*(volatile u16 *)(uintptr_t)(CO_WA + 0x10FC) == 10) {
        c = 7;
    } else if (*(volatile u16 *)(uintptr_t)CO_VAR == 2) {
        if (c == 2)
            c = 5;
        else if (c == 3)
            c = 6;
    }

    void *src = _08006590(base, 2, (int)c);
    sub_0802D984(src, (void *)(uintptr_t)0x06000000u);

    src = _08006590(base, 4, (int)c);
    CO_DMA3[0] = (u32)(uintptr_t)src;
    CO_DMA3[1] = 0x05000000u;
    CO_DMA3[2] = 0x800000A0u;
    (void)CO_DMA3[2];

    src = _08006590(base, 3, (int)c);
    *(volatile u32 *)(o + 0x24) = (u32)(uintptr_t)src;
}
#ifndef __APPLE__
void _08006468(void *a, int b, void *c) __attribute__((alias("CourseOrch_08006468")));
void sub_08006468(void *a, int b, void *c) __attribute__((alias("CourseOrch_08006468")));
#endif
