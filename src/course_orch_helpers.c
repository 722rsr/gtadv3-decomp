// ============================================================================
// course_orch_helpers.c — implementations for four course_orch.s functions
// (asm/course_orch.s, VMA 0x080061B8 / 0x0800628C / 0x08006468 / 0x080064EC).
//
// The 0x061B8 and 0x064EC implementations are compiler-generated C lifts;
// the other two preserve the remaining assembly bodies. All four are
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
extern void *_08006590(void *base, int grp, int idx);      // 0x08006590 (course_orch.c)
extern void  sub_0802D984(const void *src, void *dst);     // LZ77UnCompVram (bios_wrappers.c)
extern void  sub_0802D988(const void *src, void *dst);     // LZ77UnCompWram (bios_wrappers.c)

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
    // ROM table, not hardware: plain halfword reads emit the ROM's `ldrh`
    // / `ldrsh` (the volatile forms zero/sign-extend with extra shifts and
    // also defeat the indexed `ldrsh [r5, r0]` addressing used below).
    // The two parameter spills are pinned in declaration order so the
    // prologue copies `out` before `idx` (`adds r4,r0 / adds r3,r1`).
    register u8 *o __asm__("r4") = (u8 *)out;
    register int i __asm__("r3") = idx;
    u8 *p = (u8 *)(uintptr_t)_08006590(base, 0, i);

    *(u16 *)(o + 0x28) = *(u16 *)(p + 2);
    *(u16 *)(o + 0x2A) = *(u16 *)(p + 4);
    *(u16 *)(o + 0x2C) = *(u16 *)(p + 6);
    *(u16 *)(o + 0x2E) = *(u16 *)(p + 8);
    *(u16 *)(o + 0x30) = *(u16 *)(p + 10);

    // r3 keeps p+12 through the whole body (the tail reloads p[14] from it).
    {
        u8 *q = p + 12;
        *(u32 *)(o + 0x00) = (u32)(uintptr_t)q;
        {
            u32 acc = (u32)(uintptr_t)(p + 28);
            *(u32 *)(o + 0x04) = acc;

            {
                s16 nA = *(s16 *)(p + 2);
                acc += (u32)((nA << 2) + nA) * 4;   // nA*20
                *(u32 *)(o + 0x08) = acc;

                {
                    s16 nB = *(s16 *)(p + 4);
                    acc += (u32)((nB << 1) + nB) * 4;   // nB*12
                    *(u32 *)(o + 0x0C) = acc;

                    {
                        s16 nC = *(s16 *)(p + 6);
                        acc += (u32)((nC << 1) + nC) * 4;   // nC*12
                        *(u32 *)(o + 0x10) = acc;

                        {
                            s16 nD = *(s16 *)(p + 8);
                            acc += (u32)(nD << 3);              // nD*8
                            *(u32 *)(o + 0x14) = acc;

                            acc += 60;
                            *(u32 *)(o + 0x18) = acc;

                            // Tail flag: the ROM reuses the narrowed byte register for the 0/1
                            // result (`cmp; bhi zero; movs r0,#1; b; zero: movs r0,#0`), so the
                            // branches assign back into `f` itself -- a separate flag local takes
                            // r1 and lets the compiler hoist one materialization above the compare.
                            {
                                u8 f = (u8)(q[2] - 6);
                                if (f <= 1)
                                    f = 1;
                                else
                                    f = 0;
                                *(u16 *)(o + 0x32) = f;
                            }
                        }
                    }
                }
            }
        }
    }
}
void _080064EC(void *a, int b, void *c) __attribute__((alias("CourseOrch_080064EC")));
void sub_080064EC(void *a, int b, void *c) __attribute__((alias("CourseOrch_080064EC")));
// Content is 134 bytes (2 mod 4); the ROM pads the span with `00 00` where
// gas defaults to `c0 46`.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// _080061B8(arg0 unused, idx, base) — surface VARIANT gfx (groups 5/6/7).
// Direct C lift; register pins preserve the compiler's byte-exact allocation.
//   variant r5 from the s16[0x0203F758] pair:
//     [+2]==0: [+0]==1 -> 1; [+0]==2 -> 2; else 0
//     [+2]==1: [+0]==0 -> 3; [+0]==1 -> 4; [+0]==2 -> 5; else 0
//     [+2]==2: 6
//     else: 0
//   g5[v] LZ77 -> VRAM 0x06008000; g6[v] DMA3 -> pal 0x050001E0 (16 hw);
//   g7[v] LZ77 -> VRAM 0x0600E000; DMA fill 0x0600E180 with 0 halfwords
//   (0xC0 words, ctrl 0x810000C0).
void CourseOrch_080061B8(void *unused, int idx, void *base)
{
    register void *resourceBase __asm__("r6") = base;
    register int image __asm__("r5") = 0;
    register s16 *variant __asm__("r1");
    register volatile u32 *dma __asm__("r4");
    int group;
    int form;
    void *src;
    volatile u16 zero;

    __asm__("" : "+r"(resourceBase));
    __asm__("" : "+r"(image));
    __asm__ volatile ("" : : : "r1");
    variant = (s16 *)(uintptr_t)0x0203F758;
    group = variant[1];

    if (group == 1)
        goto group1;
    if (group > 1)
        goto group2;
    if (group == 0)
        goto group0;
    goto fetch;

group2:
    if (group == 2)
        goto group2_set;
    goto fetch;

group0:
    form = variant[0];
    if (form == 1)
        goto group0_set1;
    if (form <= 1)
        goto fetch;
    if (form == 2)
        goto group0_set2;
    goto fetch;
group0_set1:
    image = 1;
    goto fetch;
group0_set2:
    image = 2;
    goto fetch;

group1:
    form = variant[0];
    if (form == 1)
        goto group1_set4;
    if (form > 1)
        goto group1_high;
    if (form == 0)
        goto group1_set3;
    goto fetch;
group1_high:
    if (form == 2)
        goto group1_set5;
    goto fetch;
group1_set3:
    image = 3;
    goto fetch;
group1_set4:
    image = 4;
    goto fetch;
group1_set5:
    image = 5;
    goto fetch;

group2_set:
    image = 6;

fetch:

    src = _08006590(resourceBase, 5, image);
    sub_0802D984(src, (void *)(uintptr_t)0x06008000);
    src = _08006590(resourceBase, 6, image);
    dma = CO_DMA3;
    dma[0] = (u32)(uintptr_t)src;
    dma[1] = 0x050001E0;
    dma[2] = 0x80000010;
    (void)dma[2];

    src = _08006590(resourceBase, 7, image);
    sub_0802D984(src, (void *)(uintptr_t)0x0600E000);
    zero = 0;
    dma[0] = (u32)(uintptr_t)&zero;
    dma[1] = 0x0600E180;
    dma[2] = 0x810000C0;
    (void)dma[2];
}
void _080061B8(void *a, int b, void *c) __attribute__((alias("CourseOrch_080061B8")));
void sub_080061B8(void *a, int b, void *c) __attribute__((alias("CourseOrch_080061B8")));

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
void CourseOrch_0800628C(void *hdr, int idx_unused, void *base)
{
    register u8 *resourceBase __asm__("r6");
    register u8 *ewram __asm__("r8");
    register s32 theme __asm__("r4");
    register s32 mode __asm__("r5");
    register s32 paletteSlot;
    register u32 baseValue __asm__("r1");
    register s32 form __asm__("r0");
    u8 *src;
    volatile u32 *dma;

    (void)idx_unused;
    resourceBase = (u8 *)base;
    __asm__ volatile("" : "+r" (resourceBase));
    baseValue = 128;
    __asm__ volatile("" : "+r" (baseValue));
    baseValue <<= 18;
    ewram = (u8 *)baseValue;
    __asm__ volatile("" : "+r" (ewram));
    {
        register u8 *payload __asm__("r0");
        payload = *(u8 **)hdr;
        __asm__ volatile("" : "+r" (payload));
        theme = payload[3];
        __asm__ volatile("" : "+r" (theme));
    }
    paletteSlot = 0;
    {
        register s16 *cell __asm__("r0");
        cell = (s16 *)CO_VAR;
        __asm__ volatile("" : "+r" (cell));
        mode = cell[1];
        __asm__ volatile("" : "+r" (mode));
        form = cell[0];
        __asm__ volatile("" : "+r" (form));
    }

    if (form == 1)
        goto form_one;
    if (form > 1)
        goto form_high;
    if (form == 0)
        goto form_zero;
    goto mode_adjust;

form_high:
    if (form == 2)
        goto form_two;
    goto mode_adjust;

form_zero:
    if (mode == 1)
        paletteSlot = 2;
    goto mode_adjust;

form_one:
    paletteSlot = 1;
    if (mode == 1)
        paletteSlot = 3;
    goto mode_adjust;

form_two:
    if (mode == 1)
        paletteSlot = 1;
    if (theme == 2) theme = 8;
    if (theme == 3) theme = 9;
    if (theme == 4) theme = 10;
    if (theme == 5) theme = 11;
    if (theme == 6) theme = 12;

mode_adjust:
    if (mode == 2) theme += 13;

fetch:
    src = (u8 *)_08006590(resourceBase, 8, theme);
    sub_0802D984(src, (void *)(uintptr_t)0x0600A000u);

    src = (u8 *)_08006590(resourceBase, 9, theme);
    dma = CO_DMA3;
    dma[0] = (u32)(uintptr_t)src + ((u32)paletteSlot << 5);
    dma[1] = 0x050001C0u;
    dma[2] = 0x80000010u;
    (void)dma[2];

    {
        register u8 *srcDirect __asm__("r0");
        register u8 *srcCopy __asm__("r2");
        srcDirect = (u8 *)_08006590(resourceBase, 10, theme);
        srcCopy = srcDirect;
        __asm__ volatile("" : "+r" (srcDirect), "+r" (srcCopy));
        if (mode == 2) {
        register volatile u16 *tile __asm__("r4");
        register s32 count __asm__("r3");
        register s32 maskLow __asm__("r7");
        register s32 maskHigh __asm__("r6");
        register s32 bias __asm__("r5");
        tile = (volatile u16 *)ewram;
        __asm__ volatile("" : "+r" (tile));
        sub_0802D988(srcDirect, ewram);
        maskLow = 0x3FF;
        maskHigh = 252;
        maskHigh <<= 8;
        count = 176;
        count <<= 2;
        {
            register s32 seed __asm__("r0");
            seed = 128;
            seed <<= 1;
            __asm__ volatile("" : "+r" (seed));
            bias = seed;
        }
        do {
            register s32 value __asm__("r0");
            register s32 low __asm__("r1");
            register s32 high __asm__("r2");
            value = *tile;
            low = maskLow;
            low &= value;
            high = maskHigh;
            high &= value;
            low = bias + low;
            low |= high;
            *tile = (u16)low;
            count--;
            tile++;
        } while (count != 0);
        {
            register s32 fillValue __asm__("r0");
            register s32 count2 __asm__("r3");
            register s32 fillSeed __asm__("r1");
            fillSeed = 128;
            fillSeed <<= 1;
            __asm__ volatile("" : "+r" (fillSeed));
            fillValue = fillSeed;
            count2 = 144;
            count2 <<= 2;
            do {
                *tile = (u16)fillValue;
                count2--;
                tile++;
            } while (count2 != 0);
        }
        {
            register volatile u32 *dmaPort __asm__("r2");
            register u32 control __asm__("r7");
            register u8 *source0 __asm__("r1");
            register u8 *target0 __asm__("r5");
            register u8 *target1 __asm__("r6");
            register s32 row __asm__("r3");
            row = 0;
            dmaPort = CO_DMA3;
            control = 0x80000020u;
            tile = (volatile u16 *)ewram;
            tile += 32;
            source0 = ewram;
            target1 = (u8 *)0x0600F800u;
            target0 = (u8 *)0x0600F000u;
            do {
                dmaPort[0] = (u32)(uintptr_t)source0;
                dmaPort[1] = (u32)(uintptr_t)target0;
                dmaPort[2] = control;
                (void)dmaPort[2];
                dmaPort[0] = (u32)(uintptr_t)tile;
                dmaPort[1] = (u32)(uintptr_t)target1;
                dmaPort[2] = control;
                (void)dmaPort[2];
                tile += 64;
                source0 += 128;
                target1 += 64;
                target0 += 64;
                row++;
            } while (row <= 19);
        }
    } else {
        register volatile u16 *tile __asm__("r4");
        register s32 count __asm__("r3");
        register s32 maskLow __asm__("r7");
        register s32 maskHigh __asm__("r6");
        register s32 bias __asm__("r5");
        tile = (volatile u16 *)ewram;
        __asm__ volatile("" : "+r" (tile));
        sub_0802D988(srcCopy, ewram);
        maskLow = 0x3FF;
        maskHigh = 252;
        maskHigh <<= 8;
        count = 128;
        count <<= 2;
        {
            register s32 seed __asm__("r0");
            seed = 128;
            seed <<= 1;
            __asm__ volatile("" : "+r" (seed));
            bias = seed;
        }
        do {
            register s32 value __asm__("r0");
            register s32 low __asm__("r1");
            register s32 high __asm__("r2");
            value = *tile;
            low = maskLow;
            low &= value;
            high = maskHigh;
            high &= value;
            low = bias + low;
            low |= high;
            *tile = (u16)low;
            count--;
            tile++;
        } while (count != 0);
        {
            register s32 fillValue __asm__("r0");
            register s32 count2 __asm__("r3");
            register s32 fillSeed __asm__("r1");
            fillSeed = 128;
            fillSeed <<= 1;
            __asm__ volatile("" : "+r" (fillSeed));
            fillValue = fillSeed;
            count2 = 192;
            count2 <<= 2;
            do {
                *tile = (u16)fillValue;
                count2--;
                tile++;
            } while (count2 != 0);
        }
        {
            register volatile u32 *dmaPort __asm__("r2");
            register u32 control __asm__("r7");
            register u8 *source0 __asm__("r1");
            register u8 *target0 __asm__("r5");
            register u8 *target1 __asm__("r6");
            register s32 row __asm__("r3");
            row = 0;
            dmaPort = CO_DMA3;
            control = 0x80000020u;
            tile = (volatile u16 *)ewram;
            tile += 32;
            source0 = ewram;
            target1 = (u8 *)0x0600F800u;
            target0 = (u8 *)0x0600F000u;
            do {
                dmaPort[0] = (u32)(uintptr_t)source0;
                dmaPort[1] = (u32)(uintptr_t)target0;
                dmaPort[2] = control;
                (void)dmaPort[2];
                dmaPort[0] = (u32)(uintptr_t)tile;
                dmaPort[1] = (u32)(uintptr_t)target1;
                dmaPort[2] = control;
                (void)dmaPort[2];
                tile += 64;
                source0 += 128;
                target1 += 64;
                target0 += 64;
                row++;
            } while (row <= 19);
        }
        }
    }
}

void _0800628C(void *a, int b, void *c) __attribute__((alias("CourseOrch_0800628C")));
void sub_0800628C(void *a, int b, void *c) __attribute__((alias("CourseOrch_0800628C")));

// ----------------------------------------------------------------------------
// _08006468(out, idx, base) — theme TILES + PALETTE + LUT (groups 2/3/4).
//   cup tier c = payload[1] (g0 payload+13);
//   if u16[0x0300287C] == 10 -> c = 7
//   else if u16[0x0203F758] == 2 -> {2->5, 3->6}
//   g2[c] LZ77 -> VRAM 0x06000000; g4[c] DMA3 -> pal 0x05000000 (160 hw);
//   g3[c] raw ROM ptr kept at out+0x24.
void CourseOrch_08006468(void *out, int idx, void *base) {
    // Same two levers as the sibling 0x080064EC directly above, plus one:
    // - the parameter spills are pinned IN DECLARATION ORDER (r6 <- out,
    //   r5 <- base) because agbcc otherwise copies r2 first, and `c` is
    //   pinned to r4 so `payload[1]` lands in the tier register directly
    //   (`ldrb r4,[r0,#1]`) instead of loading r0 and copying.
    // - the 0x0300287C tier cell is WORK_AREA+0x10FC from TWO pool words
    //   (`ldr r0,=0x03001780` / `ldr r1,=0x10FC` / `adds`): the folded
    //   constant pools as the single word 0x0300287C and loses the adds.
    //   Both operands are assembler absolute symbols, as in
    //   src/course_orch_helpers.c's other absolute-symbol users.
    // - the seeker's result stays in r0 for the whole body: agbcc's local
    //   alloc otherwise parks it in r2 (`adds r2,r0,#0` at +0x40) and lets
    //   the DMA3 pool load claim r0, which pushes the SAD store from r0/r1
    //   to r2/r0, costing the +0x54..+0x58 pair.
    extern u8 CoWaBase[], CoWaOff10FC[];
    register volatile u8 *o __asm__("r6");
    register volatile u8 *b __asm__("r5");
    register u32 c __asm__("r4");
    register u32 wabase __asm__("r0");
    register void *src __asm__("r0");
    volatile u8 *payload;
    __asm__(".globl CoWaBase\nCoWaBase = 0x03001780\n");
    __asm__(".globl CoWaOff10FC\nCoWaOff10FC = 0x10FC\n");
    o = (volatile u8 *)out;
    b = (volatile u8 *)base;
    payload = (volatile u8 *)(uintptr_t)*(volatile u32 *)o;
    c = payload[1];

    // The base must pool into r0 and the offset into r1, and the two tier
    // rewrites are two INDEPENDENT ifs, not an else-if chain: the ROM's
    // second arm falls through from `movs r4,#5` (`cmp r4,#2 / bne +2 /
    // movs r4,#5 / cmp r4,#3`), while an else-if emits an extra `b` over
    // the first body. The rewrite is equivalent here (c is 5, never 3,
    // when the second test runs).
    wabase = (u32)(uintptr_t)CoWaBase;
    if (*(volatile u16 *)(uintptr_t)(wabase + (u32)(uintptr_t)CoWaOff10FC) == 10) {
        c = 7;
    } else if (*(volatile u16 *)(uintptr_t)CO_VAR == 2) {
        if (c == 2)
            c = 5;
        if (c == 3)
            c = 6;
    }

    src = _08006590((void *)(uintptr_t)b, 2, (int)c);
    sub_0802D984(src, (void *)(uintptr_t)0x06000000u);

    src = _08006590((void *)(uintptr_t)b, 4, (int)c);
    // One shared base pseudo: through the CO_DMA3 macro each indexed store
    // recomputes a fresh address constant, and agbcc emits `adds r1,#4`
    // before the second store instead of the ROM's [r1,#4] / [r1,#8].
    {
        volatile u32 *dma = CO_DMA3;
        dma[0] = (u32)(uintptr_t)src;
        dma[1] = 0x05000000u;
        dma[2] = 0x800000A0u;
        (void)dma[2];
    }

    src = _08006590((void *)(uintptr_t)b, 3, (int)c);
    *(volatile u32 *)(o + 0x24) = (u32)(uintptr_t)src;
}
void _08006468(void *a, int b, void *c) __attribute__((alias("CourseOrch_08006468")));
void sub_08006468(void *a, int b, void *c) __attribute__((alias("CourseOrch_08006468")));
