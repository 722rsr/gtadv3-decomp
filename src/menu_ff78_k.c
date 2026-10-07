// ============================================================================
// menu_ff78_k.c — reconstructed C for asm/menu_ff78.s (6 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080012854 (0x080012854) — full screen setup (07770/DAB8/7614/...).
//   sub_080013EB0 (0x080013EB0) — (ev, a1, a2, rec) 12-way dispatcher.
//   sub_080013F94 (0x080013F94) — (rec) s16[rec+160] clamp machine.
//   sub_080013FE4 (0x080013FE4) — (rec) 6-way jump table on s16[rec+166].
//   sub_080014AC4 (0x080014AC4) — setup + 14650/146D0/1479C/148D8 chain.
//   sub_080014C88 (0x080014C88) — full screen setup twin of 12854.
//
// NOTES:
// - 12854 stores 2 to u32[0x082A798C], a ROM word: the bus ignores the
//   store on hardware/emulator. Replicated as a volatile store.
// - 14C88's s16[rec+140] store on the WA+0x5E0==0 path uses r1 leftover
//   from "movs r1,#2" two stores earlier (no intervening call clobbers
//   r1: only strh/ldr/arith follow). Value is deterministically 2; the
//   nonzero path stores 1.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; }
__attribute__((weak)) void Sub_0800DAB8(void *a) { (void)a; }
__attribute__((weak)) void Sub_08007614(void *a, int b, int c, int d) { (void)a; (void)b; (void)c; (void)d; }
__attribute__((weak)) void Sub_0800798C(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_08007A58(void *a) { (void)a; }
__attribute__((weak)) void Sub_080075E8(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800D77C(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void *Sub_08004B68(void) { return 0; }
__attribute__((weak)) int Sub_08002124(u32 v) { (void)v; return 0; }
__attribute__((weak)) void Sub_08002B214(int v) { (void)v; }
__attribute__((weak)) void Sub_08002B234(void) {}
__attribute__((weak)) int Sub_08024C90(int v) { return v; }
__attribute__((weak)) void Sub_080013108(void *a) { (void)a; }
__attribute__((weak)) void Sub_080012ED0(void *a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800D854(void *a) { (void)a; }
__attribute__((weak)) void Sub_080012E9C(void *a) { (void)a; }
__attribute__((weak)) void Sub_080013E4C(void) { }
__attribute__((weak)) void Sub_080013B68(void *a) { (void)a; }
__attribute__((weak)) void Sub_080013E64(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800133E0(void) { }
__attribute__((weak)) void Sub_0800133E4(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080013850(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080012F14(void) { }
__attribute__((weak)) void Sub_0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800DBE8(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014650(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800146D0(void *a) { (void)a; }
__attribute__((weak)) void Sub_08001479C(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800148D8(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014A08(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014BCC(void) { }
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
extern void Sub_0800DAB8(void *a);
extern void Sub_08007614(void *a, int b, int c, int d);
extern void Sub_0800798C(void *a, void *b);
extern void Sub_08007A58(void *a);
extern void Sub_080075E8(void *a, int b, int c);
extern void Sub_0800D77C(void *a, int b, int c);
extern void *Sub_08004B68(void);
extern int Sub_08002124(u32 v);
extern void Sub_08002B214(int v);
extern void Sub_08002B234(void);
extern int Sub_08024C90(int v);
extern void Sub_080013108(void *a);
extern void Sub_080012ED0(void *a, u32 b);
extern void Sub_0800D854(void *a);
extern void Sub_080012E9C(void *a);
extern void Sub_080013E4C(void); // 0-arg ABI (asm/menu_ff78.s:7872 reads no input)
extern void Sub_080013B68(void *a);
extern void Sub_080013E64(void *a);
extern void Sub_0800133E0(void);
extern void Sub_0800133E4(void *a, u32 b, u32 c);
extern void Sub_080013850(void *a, u32 b, u32 c);
extern void Sub_080012F14(void);
extern void Sub_0800D97C(void *a, int b);
extern void Sub_0800DBE8(void *a);
extern void Sub_080014650(void *a, u32 b, u32 c);
extern void Sub_0800146D0(void *a);
extern void Sub_08001479C(void *a);
extern void Sub_0800148D8(void *a);
extern void Sub_080014A08(void *a);
extern void Sub_080014BCC(void);
#endif

// ----------------------------------------------------------------------------
// sub_080012854 — (rec) full screen setup.
//   2B214(49); if u16[WA+0xFBC]==3: 02124(0x1393).
//   07770(0, 0x082E7DD8, 1, 0, 4, 1);
//   u16[rec+80]=6; u32[rec+92]=10; u32[rec+104]=1; DAB8(rec+8);
//   tile = 0x082A798C: 7614(tile,1,0,3); 7614(tile,1,1,4); 7614(tile,1,2,5);
//   798C(tmpl,rec); 7A58(rec); 75E8(tmpl,0,6);
//   u32[rec+120]=rec+140; u32[rec+124]=rec+8; u32[rec+8]=0; s16[rec+12]=1;
//   D77C(rec+28,0,-32); D77C(rec+20,0,160);
//   s16[rec+16]=6; s16[rec+18]=5;
//   car==20: s16[rec+130]=u16[WA+0xFBE];
//   else: s16[rec+130]=(s16)24C90(s16[WA+0x574]);
//   s16[rec+128]=s16[rec+130]; u32[tile]=2 (ROM-ignored); u16[WA+0x1058]=0.
void MenuFF78_12854(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    void *tmpl = (void *)(uintptr_t)0x082E7DD8u;
    void *tile = (void *)(uintptr_t)0x082A798Cu;
    Sub_08002B214(49);
    if (*(volatile u16 *)(uintptr_t)(wa + 0xFBCu) == 3)
        Sub_08002124(0x1393u);
    _08007770(0, tmpl, 1, 0, 4, 1); // R2 C body (was Sub_ veneer)
    *(volatile u16 *)(uintptr_t)(rec + 80) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 92) = 10;
    *(volatile u32 *)(uintptr_t)(rec + 104) = 1;
    Sub_0800DAB8((void *)(uintptr_t)(rec + 8));
    Sub_08007614(tile, 1, 0, 3);
    Sub_08007614(tile, 1, 1, 4);
    Sub_08007614(tile, 1, 2, 5);
    Sub_0800798C(tmpl, rec_);
    Sub_08007A58(rec_);
    Sub_080075E8(tmpl, 0, 6);
    *(volatile u32 *)(uintptr_t)(rec + 120) = (u32)(uintptr_t)(rec + 140);
    *(volatile u32 *)(uintptr_t)(rec + 124) = (u32)(uintptr_t)(rec + 8);
    *(volatile u32 *)(uintptr_t)(rec + 8) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 12) = 1;
    Sub_0800D77C((void *)(uintptr_t)(rec + 28), 0, -32);
    Sub_0800D77C((void *)(uintptr_t)(rec + 20), 0, 160);
    *(volatile s16 *)(uintptr_t)(rec + 16) = 6;
    *(volatile s16 *)(uintptr_t)(rec + 18) = 5;
    {
        u16 car = *(volatile u16 *)(uintptr_t)((volatile u8 *)Sub_08004B68() + 2);
        if (car == 20) {
            *(volatile s16 *)(uintptr_t)(rec + 130) =
                *(volatile u16 *)(uintptr_t)(wa + 0xFBEu);
        } else {
            s16 w = *(volatile s16 *)(uintptr_t)(wa + 0x574u);
            *(volatile s16 *)(uintptr_t)(rec + 130) = (s16)Sub_08024C90((int)w);
        }
    }
    *(volatile s16 *)(uintptr_t)(rec + 128) =
        *(volatile s16 *)(uintptr_t)(rec + 130);
    *(volatile u32 *)(uintptr_t)0x082A798Cu = 2;
    *(volatile u16 *)(uintptr_t)(wa + 0x1058u) = 0;
}
#ifndef __APPLE__
void _080012854(void *a) __attribute__((alias("MenuFF78_12854")));
void sub_080012854(void *a) __attribute__((alias("MenuFF78_12854")));
#endif

// ----------------------------------------------------------------------------
// sub_080013EB0 — (ev, a1, a2, rec): ev-1 indexes a 12-entry table.
//   0->13108(rec), 1->12ED0(rec,a1),
//   4->D854(rec+32)+12E9C(rec)+[u16[WA+0xFBC]==3]13E4C(rec),
//   5->13B68(rec),
//   6->13E64(rec)+[u16[rec+36]!=0]133E0(rec,(u16)a1,(u16)a2)+
//     on s16[rec+186]: 0->133E4, 1->13850 (each (rec,(u16)a1,(u16)a2)),
//   11->12F14(rec), else no-op.
void MenuFF78_13EB0(int ev, u32 a1, u32 a2, void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int idx = ev - 1;
    if (idx < 0 || idx > 11) return;
    switch (idx) {
    case 0:
        Sub_080013108(rec_);
        break;
    case 1:
        Sub_080012ED0(rec_, a1);
        break;
    case 4:
        Sub_0800D854((void *)(uintptr_t)(rec + 32));
        Sub_080012E9C(rec_);
        if (*(volatile u16 *)(uintptr_t)(0x03001780u + 0xFBCu) == 3)
            Sub_080013E4C();
        break;
    case 5:
        Sub_080013B68(rec_);
        break;
    case 6: {
        u16 r5 = (u16)a1, r6 = (u16)a2;
        Sub_080013E64(rec_);
        if (*(volatile u16 *)(uintptr_t)(rec + 36) != 0) {
            Sub_0800133E0();
            s16 s = *(volatile s16 *)(uintptr_t)(rec + 186);
            if (s == 0)
                Sub_0800133E4(rec_, (u32)r5, (u32)r6);
            else if (s == 1)
                Sub_080013850(rec_, (u32)r5, (u32)r6);
        }
        break;
    }
    case 11:
        Sub_080012F14();
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _080013EB0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_13EB0")));
void sub_080013EB0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_13EB0")));
#endif

// ----------------------------------------------------------------------------
// sub_080013F94 — (rec) s16[rec+160] clamp machine:
//   if s16[160]-1 < s16[156]: s16[160] = s16[156].
//   if s16[160]+4 > s16[156]: s16[160] = s16[156]-3.
//   clamp 0..7.
void MenuFF78_13F94(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    if (*(volatile s16 *)(uintptr_t)(rec + 160) - 1 <
        *(volatile s16 *)(uintptr_t)(rec + 156))
        *(volatile s16 *)(uintptr_t)(rec + 160) =
            *(volatile s16 *)(uintptr_t)(rec + 156);
    if (*(volatile s16 *)(uintptr_t)(rec + 160) + 4 >
        *(volatile s16 *)(uintptr_t)(rec + 156))
        *(volatile s16 *)(uintptr_t)(rec + 160) =
            (s16)(*(volatile s16 *)(uintptr_t)(rec + 156) - 3);
    if (*(volatile s16 *)(uintptr_t)(rec + 160) < 0)
        *(volatile s16 *)(uintptr_t)(rec + 160) = 0;
    if (*(volatile s16 *)(uintptr_t)(rec + 160) > 7)
        *(volatile s16 *)(uintptr_t)(rec + 160) = 7;
}
#ifndef __APPLE__
void _080013F94(void *a) __attribute__((alias("MenuFF78_13F94")));
void Sub_080013F94(void *a) __attribute__((alias("MenuFF78_13F94")));
void sub_080013F94(void *a) __attribute__((alias("MenuFF78_13F94")));
#endif

// ----------------------------------------------------------------------------
// sub_080013FE4 — (rec) 6-way jump table on s16[rec+166] (else no-op):
//   0: [164]=0xFFD0,[166]=2; 1: [164]=48,[166]=3;
//   2: [164]+=12, if >0: [166]=4; 3: [164]-=12, if <0: [166]=4;
//   4: [164]=0,[166]=5; 5: no-op.
void MenuFF78_13FE4(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 s = *(volatile s16 *)(uintptr_t)(rec + 166);
    if (s < 0 || s > 5) return;
    switch (s) {
    case 0:
        *(volatile s16 *)(uintptr_t)(rec + 164) = (s16)0xFFD0;
        *(volatile s16 *)(uintptr_t)(rec + 166) = 2;
        break;
    case 1:
        *(volatile s16 *)(uintptr_t)(rec + 164) = 48;
        *(volatile s16 *)(uintptr_t)(rec + 166) = 3;
        break;
    case 2: {
        s16 v = (s16)(*(volatile s16 *)(uintptr_t)(rec + 164) + 12);
        *(volatile s16 *)(uintptr_t)(rec + 164) = v;
        if ((s32)((s32)v << 16) > 0)
            *(volatile s16 *)(uintptr_t)(rec + 166) = 4;
        break;
    }
    case 3: {
        s16 v = (s16)(*(volatile s16 *)(uintptr_t)(rec + 164) - 12);
        *(volatile s16 *)(uintptr_t)(rec + 164) = v;
        if ((s32)((s32)v << 16) < 0)
            *(volatile s16 *)(uintptr_t)(rec + 166) = 4;
        break;
    }
    case 4:
        *(volatile s16 *)(uintptr_t)(rec + 164) = 0;
        *(volatile s16 *)(uintptr_t)(rec + 166) = 5;
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _080013FE4(void *a) __attribute__((alias("MenuFF78_13FE4")));
void Sub_080013FE4(void *a) __attribute__((alias("MenuFF78_13FE4")));
void sub_080013FE4(void *a) __attribute__((alias("MenuFF78_13FE4")));
#endif

// ----------------------------------------------------------------------------
// sub_080014AC4 — (rec):
//   D97C(rec+152,15); r1 = s16[WA+0xFF2];
//   r1==0: 7B18(rec+8,4,16,32,5,1,0,0), r6 = rec+8;
//   r1==1: 7B18(rec+8,3,16,32,5,1,0,0), r6 = rec+8;
//   else: r6 = rec+8.
//   7B18(r6,8,s16[rec+160]*8+144,32,5,1,0,0);
//   14650(rec,128,104); 146D0(rec); 1479C(rec); 148D8(rec);
//   if u16[rec+36]==1:
//     7B18(r6,9,-40,48,5,1,0,0); 7B18(r6,9,216,48,5,1,0,0).
//   14A08(rec); DBE8(rec+32); 14BCC(rec).
void MenuFF78_14AC4(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    Sub_0800D97C((void *)(uintptr_t)(rec + 152), 15);
    {
        s16 r1 = *(volatile s16 *)(uintptr_t)(0x03001780u + 0xFF2u);
        if (r1 == 0) {
            Sub_08007B18((void *)(uintptr_t)(rec + 8), 4, 16, 32,
                         5, 1, 0, 0);
        } else if (r1 == 1) {
            Sub_08007B18((void *)(uintptr_t)(rec + 8), 3, 16, 32,
                         5, 1, 0, 0);
        }
    }
    {
        s16 w = *(volatile s16 *)(uintptr_t)(rec + 160);
        Sub_08007B18((void *)(uintptr_t)(rec + 8), 8,
                     (int)w * 8 + 144, 32, 5, 1, 0, 0);
    }
    Sub_080014650(rec_, 128, 104);
    Sub_0800146D0(rec_);
    Sub_08001479C(rec_);
    Sub_0800148D8(rec_);
    if (*(volatile u16 *)(uintptr_t)(rec + 36) == 1) {
        Sub_08007B18((void *)(uintptr_t)(rec + 8), 9, -40, 48, 5, 1, 0, 0);
        Sub_08007B18((void *)(uintptr_t)(rec + 8), 9, 216, 48, 5, 1, 0, 0);
    }
    Sub_080014A08(rec_);
    Sub_0800DBE8((void *)(uintptr_t)(rec + 32));
    Sub_080014BCC();
}
#ifndef __APPLE__
void _080014AC4(void *a) __attribute__((alias("MenuFF78_14AC4")));
void Sub_080014AC4(void *a) __attribute__((alias("MenuFF78_14AC4")));
void sub_080014AC4(void *a) __attribute__((alias("MenuFF78_14AC4")));
#endif

// ----------------------------------------------------------------------------
// sub_080014C88 — (rec) full screen setup twin of 12854:
//   07770(0, 0x082F8348, 3, 0, 4, 1);
//   u16[rec+80]=6; u32[rec+92]=11; u32[rec+104]=1; DAB8(rec+8);
//   tile = 0x082A798C: 7614(tile,1,0,3); 7614(tile,1,1,4);
//   798C(tmpl,rec); 7A58(rec);
//   75E8(tmpl,0,6); 75E8(tmpl,1,5); 75E8(tmpl,2,7);
//   u32[rec+120]=rec+144; u32[rec+124]=rec+8; u32[rec+8]=0; s16[rec+12]=1;
//   D77C(rec+28,0,-32); D77C(rec+20,0,160);
//   s16[rec+16]=6; s16[rec+18]=5; u32[rec+144]=2;
//   s16[rec+136]=1; s16[rec+138]=1;
//   s16[rec+140] = (s16[WA+0x5E0] != 0) ? 1 : 2 (stale-r1 proof, see NOTE);
//   s16[rec+134]=0; 2B234.
void MenuFF78_14C88(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    void *tmpl = (void *)(uintptr_t)0x082F8348u;
    void *tile = (void *)(uintptr_t)0x082A798Cu;
    _08007770(0, tmpl, 3, 0, 4, 1); // R2 C body (was Sub_ veneer)
    *(volatile u16 *)(uintptr_t)(rec + 80) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 92) = 11;
    *(volatile u32 *)(uintptr_t)(rec + 104) = 1;
    Sub_0800DAB8((void *)(uintptr_t)(rec + 8));
    Sub_08007614(tile, 1, 0, 3);
    Sub_08007614(tile, 1, 1, 4);
    Sub_0800798C(tmpl, rec_);
    Sub_08007A58(rec_);
    Sub_080075E8(tmpl, 0, 6);
    Sub_080075E8(tmpl, 1, 5);
    Sub_080075E8(tmpl, 2, 7);
    *(volatile u32 *)(uintptr_t)(rec + 120) = (u32)(uintptr_t)(rec + 144);
    *(volatile u32 *)(uintptr_t)(rec + 124) = (u32)(uintptr_t)(rec + 8);
    *(volatile u32 *)(uintptr_t)(rec + 8) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 12) = 1;
    Sub_0800D77C((void *)(uintptr_t)(rec + 28), 0, -32);
    Sub_0800D77C((void *)(uintptr_t)(rec + 20), 0, 160);
    *(volatile s16 *)(uintptr_t)(rec + 16) = 6;
    *(volatile s16 *)(uintptr_t)(rec + 18) = 5;
    *(volatile u32 *)(uintptr_t)(rec + 144) = 2;
    *(volatile s16 *)(uintptr_t)(rec + 136) = 1;
    *(volatile s16 *)(uintptr_t)(rec + 138) = 1;
    *(volatile s16 *)(uintptr_t)(rec + 140) =
        (*(volatile s16 *)(uintptr_t)(wa + 0x5E0u) != 0) ? 1 : 2;
    *(volatile s16 *)(uintptr_t)(rec + 134) = 0;
    Sub_08002B234();
}
#ifndef __APPLE__
void _080014C88(void *a) __attribute__((alias("MenuFF78_14C88")));
void Sub_080014C88(void *a) __attribute__((alias("MenuFF78_14C88")));
void sub_080014C88(void *a) __attribute__((alias("MenuFF78_14C88")));
#endif
