// ============================================================================
// menu_ff78_i.c — reconstructed C for asm/menu_ff78.s (5 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_0800121A8 (0x0800121A8) — builder + conditional 7BFC + F778 tail.
//   sub_080015A0C (0x080015A0C) — table-driven 15930/159AC + 7C68 tail.
//   sub_080015AB0 (0x080015AB0) — twin of 15A0C (tables B, s0=112).
//   sub_080014A08 (0x080014A08) — dual gated 7B18 pairs.
//   sub_080015154 (0x080015154) — (ev, a1, a2, rec) 12-way dispatcher.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) void Sub_08007C68(void *a, u32 b, u32 c, int d, u32 e, u32 f, u32 g, u32 h, u32 i) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) void *Sub_08024BF0(void) { return 0; }
__attribute__((weak)) int Sub_0802581C(int v) { return v; }
__attribute__((weak)) void Sub_080011E10(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011CFC(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011B48(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011E58(void *a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_080011CBC(void *a) { (void)a; }
__attribute__((weak)) void Sub_080012574(volatile void *a) { (void)a; }
__attribute__((weak)) void Sub_0800F778(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800DBE8(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800DC7C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_080015930(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800159AC(void *a, u32 b, u32 c, u32 d) { (void)a; (void)b; (void)c; (void)d; }
__attribute__((weak)) void Sub_080014C70(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800D854(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800D8E4(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014F60(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014D94(void *a, int b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080014E3C(void *a, int b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080014EA0(void *a, int b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080014F14(void *a, int b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080014C88(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014C84(void) { }
#else
#ifndef __APPLE__
extern void sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
#define Sub_08007B18 sub_08007B18
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
#endif
extern void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void Sub_08007C68(void *a, u32 b, u32 c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void _08007C68(void *a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i); // faithful strong body
extern void *Sub_08024BF0(void);
extern int Sub_0802581C(int v);
extern void Sub_080011E10(void *a);
extern void Sub_080011784(void *a);
extern void Sub_080011CFC(void *a);
extern void Sub_080011B48(void *a);
extern void Sub_080011E58(void *a, u32 b);
extern void Sub_080011CBC(void *a);
extern void Sub_080012574(volatile void *a);
extern void Sub_0800F778(u32 a, u32 b, u32 c);
extern void Sub_0800D97C(void *a, int b);
extern void Sub_0800DBE8(void *a);
extern void Sub_0800DC7C(void *a, int b);
extern void Sub_080015930(void *a, u32 b, u32 c);
extern void Sub_0800159AC(void *a, u32 b, u32 c, u32 d); // 4-arg: asm/menu_ff78.s body uses r0-r3 only
extern void Sub_080014C70(void *a, void *b);
extern void Sub_0800D854(void *a);
extern void Sub_0800D8E4(void *a);
extern void Sub_080014F60(void *a);
extern void Sub_080014D94(void *a, int b, u32 c);
extern void Sub_080014E3C(void *a, int b, u32 c);
extern void Sub_080014EA0(void *a, int b, u32 c);
extern void Sub_080014F14(void *a, int b, u32 c);
extern void Sub_080014C88(void *a);
extern void Sub_080014C84(void);
#endif

// ----------------------------------------------------------------------------
// sub_0800121A8 — (rec): 24BF0 ptr; 11784/11E10/11CFC/11B48;
//   11E58(rec, u16[rec+180]);
//   if u16[p]!=0 and u16[p+2]==s16[rec+172]:
//     7BFC(rec+32,u32[rec+444],u32[rec+448],208,120,5,1,1,0);
//   11CBC; F778(168,76,u32[WA+0x5E4+(s16)2581C(s16[rec+172])*72]);
//   D97C(rec+224,15); DBE8(rec+64); 12574(rec).
void MenuFF78_121A8(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *p = (volatile u8 *)Sub_08024BF0();
    Sub_080011784(rec_);
    Sub_080011E10(rec_);
    Sub_080011CFC(rec_);
    Sub_080011B48(rec_);
    Sub_080011E58(rec_, *(volatile u16 *)(uintptr_t)(rec + 180));
    if (*(volatile u16 *)(uintptr_t)p != 0 &&
        *(volatile u16 *)(uintptr_t)(p + 2) ==
            *(volatile s16 *)(uintptr_t)(rec + 172)) {
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 444);
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 448);
        Sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)r1, (int)r2,
                     208, 120, 5, 1, 1, 0);
    }
    Sub_080011CBC(rec_);
    {
        s16 i = *(volatile s16 *)(uintptr_t)(rec + 172);
        s16 v = (s16)Sub_0802581C((int)i);
        u32 k = (u32)(s32)v;
        u32 off = ((k << 3) + k) << 3; // *72
        u32 t = *(volatile u32 *)(uintptr_t)(0x03001780u + 0x5E4u + off);
        Sub_0800F778(168, 76, t);
    }
    Sub_0800D97C((void *)(uintptr_t)(rec + 224), 15);
    Sub_0800DBE8((void *)(uintptr_t)(rec + 64));
    Sub_080012574(rec);
}
#ifndef __APPLE__
void _0800121A8(void *a) __attribute__((alias("MenuFF78_121A8")));
void sub_0800121A8(void *a) __attribute__((alias("MenuFF78_121A8")));
#endif

// ----------------------------------------------------------------------------
// Shared shape of sub_080015A0C / 15AB0 — (rec):
//   tbl = TBL; s = s16[rec+144];
//   15930(rec, u32[tbl+4+s*8], u32[rec+164]);
//   159AC(rec, u32[tbl+s*8], u32[tbl+4+s*8], u16[TBL2+s*2],
//     u32[rec+164]);
//   D97C(rec+140,15); tail-leaf(rec+16);
//   if s16[rec+136]==1:
//     7C68(rec+8, u32[rec+152], u32[rec+156], 20, S0,5,1,0,0).
// 15A0C: TBL=0x080CB818, TBL2=0x080CB840, tail=DC7C(...,1), S0=126.
// 15AB0: TBL=0x080CB84C, TBL2=0x080CB874, tail=DBE8(...), S0=112.
static void MenuFF78_15Ax(void *rec_, u32 tbl, u32 tbl2, int use_dc7c, u32 s0) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 s = *(volatile s16 *)(uintptr_t)(rec + 144);
    u32 off = ((u32)(s32)s) << 3;
    Sub_080015930(rec_, *(volatile u32 *)(uintptr_t)(tbl + 4 + off),
                  *(volatile u32 *)(uintptr_t)(rec + 164));
    {
        u32 r1 = *(volatile u32 *)(uintptr_t)(tbl + off);
        u32 r2 = *(volatile u32 *)(uintptr_t)(tbl + 4 + off);
        u32 r3 = *(volatile u16 *)(uintptr_t)(tbl2 + (((u32)(s32)s) << 1));
        // u32[rec+164] is read by the CALLEE itself (switch at +164); the ROM
        // caller stores one dead word at [sp] that the body never reads.
        Sub_0800159AC(rec_, r1, r2, r3);
    }
    Sub_0800D97C((void *)(uintptr_t)(rec + 140), 15);
    if (use_dc7c)
        Sub_0800DC7C((void *)(uintptr_t)(rec + 16), 1);
    else
        Sub_0800DBE8((void *)(uintptr_t)(rec + 16));
    if (*(volatile s16 *)(uintptr_t)(rec + 136) == 1) {
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 152);
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 156);
        _08007C68((void *)(uintptr_t)(rec + 8), r1, r2, 20, s0, 5, 1, 0, 0); // R1 C body (was Sub_ veneer)
    }
}

void MenuFF78_15A0C(void *rec_) { MenuFF78_15Ax(rec_, 0x080CB818u, 0x080CB840u, 1, 126); }
#ifndef __APPLE__
void _080015A0C(void *a) __attribute__((alias("MenuFF78_15A0C")));
void Sub_080015A0C(void *a) __attribute__((alias("MenuFF78_15A0C")));
void sub_080015A0C(void *a) __attribute__((alias("MenuFF78_15A0C")));
#endif

void MenuFF78_15AB0(void *rec_) { MenuFF78_15Ax(rec_, 0x080CB84Cu, 0x080CB874u, 0, 112); }
#ifndef __APPLE__
void _080015AB0(void *a) __attribute__((alias("MenuFF78_15AB0")));
void Sub_080015AB0(void *a) __attribute__((alias("MenuFF78_15AB0")));
void sub_080015AB0(void *a) __attribute__((alias("MenuFF78_15AB0")));
#endif

// ----------------------------------------------------------------------------
// sub_080014A08 — (rec): dual gated 7B18 pairs over rec+68.
//   if s16[rec+156] > 0: r2 = s16[rec+154];
//     r2==0: 7B18(rec+68,14,16,48,2,1,0,0);
//     r2==1: 7B18(rec+68,11,16,48,2,1,0,0).
//   if s16[rec+156] < s16[rec+158]: r2 = s16[rec+154];
//     r2==0: 7B18(rec+68,16,216,48,2,1,0,0);
//     r2==1: 7B18(rec+68,15,216,48,2,1,0,0).
//
// Two source decisions are load-bearing for the byte match. Neither is a
// register `__asm__` pin — this body needs none, and the GNU extension is
// not used here.
//
// 1. The three s16 reads are deliberately NOT volatile. A `volatile s16`
//    read makes agbcc fold the load to `ldrh / lsls #16 / asrs #16` (three
//    instructions); the ROM has the indexed two-instruction form
//    `movs r1,#0` + `ldrsh r0,[r0,r1]` at 0x8014a10, `movs r3,#0` +
//    `ldrsh r2,[r0,r3]` at 0x8014a1c, and `movs r3,#0` + `ldrsh r1,[r0,r3]`
//    at 0x8014a68. The pointee qualifier is the whole axis, not the
//    spelling: a struct-typed `rec[0].f156` with an explicit element index
//    (the idiom in src/ai_line_more.c:70) still folded, because the
//    qualifier, not the pointer type, is what agbcc reacts to. See
//    src/car_tick_dispatch.c:115 and docs/matching_workflow.md:1916.
// 2. The two-arm dispatch is a `switch`, not `if/else if`. agbcc lays the
//    two cases out as hoisted tests — `cmp r2,#0 / beq`, `cmp r2,#1 / beq`,
//    `b end`, then both bodies — which is exactly 0x8014a20..0x8014a28.
//    `if/else if` instead emits `bne` to a forward skip, and additionally
//    folds `rec+68` against the still-live `rec+154` address node into
//    `subs r0,#86`, which the ROM does not have. See
//    docs/matching_workflow.md:1882 for the same `? :` / `if/else` class.
//
// Semantics are unchanged: a `switch` with `case 0` / `case 1` and no
// `default` selects the same arm as the `if/else if` it replaced.
void MenuFF78_14A08(void *rec_) {
    u8 *rec = (u8 *)rec_;
    if (*(s16 *)(uintptr_t)(rec + 156) > 0) {
        s16 r2 = *(s16 *)(uintptr_t)(rec + 154);
        switch (r2) {
        case 0:
            Sub_08007B18((void *)(uintptr_t)(rec + 68), 14, 16, 48, 2, 1, 0, 0);
            break;
        case 1:
            Sub_08007B18((void *)(uintptr_t)(rec + 68), 11, 16, 48, 2, 1, 0, 0);
            break;
        }
    }
    if (*(s16 *)(uintptr_t)(rec + 156) < *(s16 *)(uintptr_t)(rec + 158)) {
        s16 r2 = *(s16 *)(uintptr_t)(rec + 154);
        switch (r2) {
        case 0:
            Sub_08007B18((void *)(uintptr_t)(rec + 68), 16, 216, 48, 2, 1, 0, 0);
            break;
        case 1:
            Sub_08007B18((void *)(uintptr_t)(rec + 68), 15, 216, 48, 2, 1, 0, 0);
            break;
        }
    }
}
#ifndef __APPLE__
void _080014A08(void *a) __attribute__((alias("MenuFF78_14A08")));
void Sub_080014A08(void *a) __attribute__((alias("MenuFF78_14A08")));
void sub_080014A08(void *a) __attribute__((alias("MenuFF78_14A08")));
#endif

// ----------------------------------------------------------------------------
// sub_080015154 — (ev, a1, a2, rec): ev-1 indexes a 12-entry table.
//   0->14C88(rec), 1->14C70(rec,a1), 4->D854(rec+8)+D8E4(rec+112),
//   5->14F60(rec),
//   6->[u16[rec+12]!=0] 14D94(rec,(u16)a1,(u16)a2), then on s16[rec+134]:
//     0->14E3C, 1->14EA0, 2->14F14 (each (rec,(u16)a1,(u16)a2)),
//   11->14C84(rec), else no-op.
void MenuFF78_15154(int ev, u32 a1, u32 a2, void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int idx = ev - 1;
    if (idx < 0 || idx > 11) return;
    switch (idx) {
    case 0:
        Sub_080014C88(rec_);
        break;
    case 1:
        Sub_080014C70(rec_, (void *)(uintptr_t)a1);
        break;
    case 4:
        Sub_0800D854((void *)(uintptr_t)(rec + 8));
        Sub_0800D8E4((void *)(uintptr_t)(rec + 112));
        break;
    case 5:
        Sub_080014F60(rec_);
        break;
    case 6: {
        u16 r5 = (u16)a1, r6 = (u16)a2;
        if (*(volatile u16 *)(uintptr_t)(rec + 12) != 0) {
            Sub_080014D94(rec_, (int)r5, (u32)r6);
            s16 s = *(volatile s16 *)(uintptr_t)(rec + 134);
            if (s == 0)
                Sub_080014E3C(rec_, (int)r5, (u32)r6);
            else if (s == 1)
                Sub_080014EA0(rec_, (int)r5, (u32)r6);
            else if (s == 2)
                Sub_080014F14(rec_, (int)r5, (u32)r6);
        }
        break;
    }
    case 11:
        Sub_080014C84();
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _080015154(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_15154")));
void sub_080015154(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_15154")));
#endif
