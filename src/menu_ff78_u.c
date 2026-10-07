// ============================================================================
// menu_ff78_u.c — reconstructed C for asm/menu_ff78.s (1 function).
//
// Body transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_0800133E4 (0x0800133E4) — (rec, u16 a1, u16 ev) big event machine.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08002B368(u32 v) { (void)v; }
__attribute__((weak)) void Sub_08007ABC(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800DA50(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) int Sub_080254E8(int v) { return v; }
__attribute__((weak)) int Sub_080022E4(int v) { return v; }
__attribute__((weak)) int Sub_08024C3C(int v) { return v; }
__attribute__((weak)) int Sub_08024C58(int v) { return v; }
__attribute__((weak)) void Sub_080026A4C(u32 a, int b, int c) { (void)a; (void)b; (void)c; }
#else
extern void Sub_08002B368(u32 v);
extern void Sub_08007ABC(u32 a, u32 b, u32 c);
extern void Sub_0800DA50(void *a, int b, int c);
extern int Sub_080254E8(int v);
extern int Sub_080022E4(int v);
extern int Sub_08024C3C(int v);
extern int Sub_08024C58(int v);
// ROM 0x08026A4C dereferences r0 (mgr ptr) and stores r1/r2 to halfwords:
extern void Sub_080026A4C(void *mgr, int b, int c);
#endif

// WA pair helper: u8[WA+s*12+49] = lo(u16[rec+172]),
// u8[WA+s*12+48] = lo(u16[rec+178]).
static void MenuFF78_WAPair(volatile u8 *rec, volatile u8 *wa, s16 s) {
    u32 off = ((u32)(s32)s << 1) + (u32)(s32)s; // *3
    off <<= 2; // *12
    *(volatile u8 *)(uintptr_t)(wa + off + 49) =
        (u8)*(volatile u16 *)(uintptr_t)(rec + 172);
    *(volatile u8 *)(uintptr_t)(wa + off + 48) =
        (u8)*(volatile u16 *)(uintptr_t)(rec + 178);
}

// ----------------------------------------------------------------------------
// sub_0800133E4 — (rec, u16 a1, u16 ev):
//   sp4 = u16[174], sp8 = u16[172], sp12 = u16[136] (entries).
//   ev==1:
//     s = s16[138];
//     s==1: 2B368(1); s16[186]=ev; WAPair(s16[174]); u32[144]=0.
//     s==0 on c = s16[WA+0xFC2]:
//       c==95: 2B368(1); s16[186]=ev; WAPair(s16[174]); u32[144]=0.
//       c==94||c==96: 2B368(1); WAPair(s16[174]);
//         u32[32]=10; s16[36]=0; u32[64]=10; s16[40]=5; u32[60]=1;
//         u16[WA+0xFBE]=u16[176]; u16[WA+0xFC2]=u16[174];
//         WAPair(s16[WA+0xFC2]).
//   ev==2: 2B368(4); u32[32]=10; s16[36]=0; u32[64]=10; u32[60]=0.
//   (a1&32) && s16[140]<=0: u16[136]--, s16[140]=5.
//   (a1&16) && s16[140]<=0: u16[136]++, s16[140]=5.
//   s16[140]--; <=0 -> 0.
//   ev==64: s16[172]--. ev==128: s16[172]++.
//   clamp s16[136] >= 0.
//   if u32[rec+188+s16[136]*4]==-1: u16[136]--.
//   if s16[172]<0: s16[172] = (s8)254E8(s16[174])-1.
//   if s16[172] > (s8)254E8(s16[174])-1: s16[172]=0.
//   u8[WA+s16[174]*12+49] = lo(u16[172]).
//   (s16)sp12 != s16[136] -> 2B368(3).
//   (s32)(s16)sp8 != s16[172] -> 2B368(2).
//   s16[136] != (s16)sp12... (see body) -> u16[174] =
//     u32[rec+188+s16[136]*4]; s16[180] = 22E4(s16[174]).
//   u16[174] != sp4 -> rebuild: s16[172] = (s8)u8[WA+s16[174]*12+49];
//     DA50(rec+580, s16[174], s16[172]);
//     u32[600]=24C3C(...); 7ABC(u32[20],u32[600],u32[596]);
//     u32[612]=24C58(...); 7ABC(u32[28],u32[612],u32[608]);
//     DMA x5 over s16[174] row (pals 82..8A).
//   s16[172] != (s32)(s16)sp8 -> 26A4C(u32[580], s16[180], s16[172]).
void MenuFF78_133E4(void *rec_, u32 a1_, u32 ev_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    u16 a1 = (u16)a1_;
    u16 ev = (u16)ev_;
    u16 sp4 = *(volatile u16 *)(uintptr_t)(rec + 174);
    u16 sp8 = *(volatile u16 *)(uintptr_t)(rec + 172);
    u16 sp12 = *(volatile u16 *)(uintptr_t)(rec + 136);
    if (ev == 1) {
        s16 s = *(volatile s16 *)(uintptr_t)(rec + 138);
        if (s == 1) {
            Sub_08002B368(1);
            *(volatile s16 *)(uintptr_t)(rec + 186) = (s16)ev;
            MenuFF78_WAPair(rec, wa,
                            *(volatile s16 *)(uintptr_t)(rec + 174));
            *(volatile u32 *)(uintptr_t)(rec + 144) = 0;
        } else if (s == 0) {
            s16 c = *(volatile s16 *)(uintptr_t)(wa + 0xFC2u);
            if (c == 95) {
                Sub_08002B368(1);
                *(volatile s16 *)(uintptr_t)(rec + 186) = (s16)ev;
                MenuFF78_WAPair(rec, wa,
                                *(volatile s16 *)(uintptr_t)(rec + 174));
                *(volatile u32 *)(uintptr_t)(rec + 144) = 0;
            } else if (c == 94 || c == 96) {
                Sub_08002B368(1);
                MenuFF78_WAPair(rec, wa,
                                *(volatile s16 *)(uintptr_t)(rec + 174));
                *(volatile u32 *)(uintptr_t)(rec + 32) = 10;
                *(volatile s16 *)(uintptr_t)(rec + 36) = 0;
                *(volatile u32 *)(uintptr_t)(rec + 64) = 10;
                *(volatile s16 *)(uintptr_t)(rec + 40) = 5;
                *(volatile u32 *)(uintptr_t)(rec + 60) = 1;
                *(volatile u16 *)(uintptr_t)(wa + 0xFBEu) =
                    *(volatile u16 *)(uintptr_t)(rec + 176);
                *(volatile u16 *)(uintptr_t)(wa + 0xFC2u) =
                    *(volatile u16 *)(uintptr_t)(rec + 174);
                MenuFF78_WAPair(rec, wa,
                                *(volatile s16 *)(uintptr_t)(wa + 0xFC2u));
            }
        }
    }
    if (ev == 2) {
        Sub_08002B368(4);
        *(volatile u32 *)(uintptr_t)(rec + 32) = 10;
        *(volatile s16 *)(uintptr_t)(rec + 36) = 0;
        *(volatile u32 *)(uintptr_t)(rec + 64) = 10;
        *(volatile u32 *)(uintptr_t)(rec + 60) = 0;
    }
    if ((a1 & 32) != 0 && *(volatile s16 *)(uintptr_t)(rec + 140) <= 0) {
        u16 c = *(volatile u16 *)(uintptr_t)(rec + 136);
        *(volatile u16 *)(uintptr_t)(rec + 136) = (u16)(c - 1);
        *(volatile s16 *)(uintptr_t)(rec + 140) = 5;
    }
    if ((a1 & 16) != 0 && *(volatile s16 *)(uintptr_t)(rec + 140) <= 0) {
        u16 c = *(volatile u16 *)(uintptr_t)(rec + 136);
        *(volatile u16 *)(uintptr_t)(rec + 136) = (u16)(c + 1);
        *(volatile s16 *)(uintptr_t)(rec + 140) = 5;
    }
    {
        s16 c = (s16)(*(volatile s16 *)(uintptr_t)(rec + 140) - 1);
        *(volatile s16 *)(uintptr_t)(rec + 140) = (c <= 0) ? 0 : c;
    }
    if (ev == 64) {
        s16 c = *(volatile s16 *)(uintptr_t)(rec + 172);
        *(volatile s16 *)(uintptr_t)(rec + 172) = (s16)(c - 1);
    }
    if (ev == 128) {
        s16 c = *(volatile s16 *)(uintptr_t)(rec + 172);
        *(volatile s16 *)(uintptr_t)(rec + 172) = (s16)(c + 1);
    }
    if (*(volatile s16 *)(uintptr_t)(rec + 136) < 0)
        *(volatile s16 *)(uintptr_t)(rec + 136) = 0;
    if (*(volatile u32 *)(uintptr_t)(rec + 188 +
            ((u32)(s32)*(volatile s16 *)(uintptr_t)(rec + 136) << 2)) ==
        0xFFFFFFFFu) {
        u16 c = *(volatile u16 *)(uintptr_t)(rec + 136);
        *(volatile u16 *)(uintptr_t)(rec + 136) = (u16)(c - 1);
    }
    {
        s16 v = *(volatile s16 *)(uintptr_t)(rec + 172);
        s16 w = *(volatile s16 *)(uintptr_t)(rec + 174);
        if (v < 0) {
            int t = (int)(s8)Sub_080254E8((int)w) - 1;
            *(volatile s16 *)(uintptr_t)(rec + 172) = (s16)t;
        }
    }
    {
        s16 v = *(volatile s16 *)(uintptr_t)(rec + 172);
        s16 w = *(volatile s16 *)(uintptr_t)(rec + 174);
        if (v > (int)(s8)Sub_080254E8((int)w) - 1)
            *(volatile s16 *)(uintptr_t)(rec + 172) = 0;
    }
    {
        s16 w = *(volatile s16 *)(uintptr_t)(rec + 174);
        u32 off = (((u32)(s32)w << 1) + (u32)(s32)w) << 2; // *12
        *(volatile u8 *)(uintptr_t)(wa + off + 49) =
            (u8)*(volatile u16 *)(uintptr_t)(rec + 172);
    }
    if ((s16)sp12 != *(volatile s16 *)(uintptr_t)(rec + 136))
        Sub_08002B368(3);
    if ((s32)(s16)sp8 != (int)*(volatile s16 *)(uintptr_t)(rec + 172))
        Sub_08002B368(2);
    if (*(volatile s16 *)(uintptr_t)(rec + 136) != (s16)sp12) {
        u32 v = *(volatile u32 *)(uintptr_t)(rec + 188 +
            ((u32)(s32)*(volatile s16 *)(uintptr_t)(rec + 136) << 2));
        *(volatile u16 *)(uintptr_t)(rec + 174) = (u16)v;
        *(volatile s16 *)(uintptr_t)(rec + 180) =
            (s16)Sub_080022E4(
                (int)*(volatile s16 *)(uintptr_t)(rec + 174));
    }
    if (*(volatile u16 *)(uintptr_t)(rec + 174) != sp4) {
        s16 w = *(volatile s16 *)(uintptr_t)(rec + 174);
        u32 off = (((u32)(s32)w << 1) + (u32)(s32)w) << 2; // *12
        s8 b = (s8)*(volatile u8 *)(uintptr_t)(wa + off + 49);
        *(volatile s16 *)(uintptr_t)(rec + 172) = (s16)b;
        Sub_0800DA50((void *)(uintptr_t)(rec + 580),
                     (int)*(volatile s16 *)(uintptr_t)(rec + 174),
                     (int)*(volatile s16 *)(uintptr_t)(rec + 172));
        {
            int v = Sub_08024C3C(
                (int)*(volatile s16 *)(uintptr_t)(rec + 174));
            *(volatile u32 *)(uintptr_t)(rec + 600) = (u32)v;
            Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 20), (u32)v,
                         *(volatile u32 *)(uintptr_t)(rec + 596));
        }
        {
            int v = Sub_08024C58(
                (int)*(volatile s16 *)(uintptr_t)(rec + 174));
            *(volatile u32 *)(uintptr_t)(rec + 612) = (u32)v;
            Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 28), (u32)v,
                         *(volatile u32 *)(uintptr_t)(rec + 608));
        }
        {
            s16 i = *(volatile s16 *)(uintptr_t)(rec + 174);
            u32 row = (u32)(s32)i * 20;
            volatile u32 *dma = (volatile u32 *)(uintptr_t)0x040000D4u;
            for (u32 k = 1; k <= 5; k++) {
                s8 bb = (s8)*(volatile u8 *)(uintptr_t)(0x080CCEECu + row + k);
                dma[0] = 0x080CD7A8u + (u32)((s32)bb * 2);
                dma[1] = 0x05000280u + k * 2;
                dma[2] = 0x80000001u;
                (void)dma[2];
            }
        }
    }
    if (*(volatile s16 *)(uintptr_t)(rec + 172) != (s32)(s16)sp8)
        Sub_080026A4C((void *)(uintptr_t)*(volatile u32 *)(uintptr_t)(rec + 580),
                      (int)*(volatile s16 *)(uintptr_t)(rec + 180),
                      (int)*(volatile s16 *)(uintptr_t)(rec + 172));
}
#ifndef __APPLE__
void _0800133E4(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_133E4")));
void Sub_0800133E4(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_133E4")));
void sub_0800133E4(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_133E4")));
#endif
