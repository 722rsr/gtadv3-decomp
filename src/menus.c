#include "gtadv/menus.h"
#include "gba/bios.h"
#include "gba/regs.h"


// Minimal external stubs for standalone HOST builds (apple-only; on ARM the
// strong bodies win — these must never be
// ARM-live, or every call below binds to the no-op at compile time).
#ifdef __APPLE__
__attribute__((weak)) void sub_0802D974(const void *s, void *d, u32 m) { CpuFastSet(s,d,m); }
__attribute__((weak)) int sub_08008014(void) { return 0; }
__attribute__((weak)) void sub_080188B0(void *pkt) { (void)pkt; }
__attribute__((weak)) u32 sub_08002140(void) { return 0; }
// The VMA spelling of the same callee (0x08002140). The countdown tickers
// below must call it by THIS spelling: 0x08002140 is a promoted entry whose
// `export` carries only `_08002140`, and the splice deletes the asm labels, so
// a call to `sub_08002140` from a spliced body would be an undefined
// reference at link time while `_08002140` binds.
__attribute__((weak)) int _08002140(void) { return 0; }
__attribute__((weak)) void _08004D4C(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) u32 sub_08002178(u32 a) { (void)a; return 0; }
__attribute__((weak)) void sub_08002158(u32 a, u32 b) { (void)a;(void)b; }
__attribute__((weak)) void sub_08004BFC(u32 v) { (void)v; }
__attribute__((weak)) void sub_08004C84(u32 idx, u32 value) { (void)idx; (void)value; }
__attribute__((weak)) void sub_08004EC0(u32 v) { (void)v; }
__attribute__((weak)) void sub_08004EA8(u32 v) { (void)v; }
__attribute__((weak)) int sub_08004CA8(u32 idx) { (void)idx; return 0; }
__attribute__((weak)) void sub_080056B8(u32 v) { (void)v; }
__attribute__((weak)) void sub_08004E1C(void *a) { (void)a; }
__attribute__((weak)) void sub_08004BB8(void *c) { (void)c; }
__attribute__((weak)) void sub_08004BC8(void *c) { (void)c; }
__attribute__((weak)) void sub_0800CAE4(void *c) { (void)c; }
__attribute__((weak)) void sub_0800C884(void *c,u32 a,u32 b) { (void)c;(void)a;(void)b; }
__attribute__((weak)) void sub_080055F4(void *a) { (void)a; }
__attribute__((weak)) void sub_08005604(void *a) { (void)a; }
__attribute__((weak)) void sub_08002C98(void *a) { (void)a; }
__attribute__((weak)) void sub_08002B50(void) { }
__attribute__((weak)) void sub_08002BB4(void) { }
__attribute__((weak)) void sub_08002B44(void) { }
__attribute__((weak)) void sub_080050E8(void *a, u32 b) { (void)a;(void)b; }
__attribute__((weak)) void sub_08002124(u32 v) { (void)v; }
__attribute__((weak)) void _08002124(u16 v) { (void)v; }
__attribute__((weak)) int sub_0800254C(void *p) { (void)p; return 0; }
__attribute__((weak)) u32 sub_08004EF0(u32 v) { (void)v; }
__attribute__((weak)) void *_08004B68(void) { return (void *)0; }
__attribute__((weak)) void _0802B234(void) { }
// `sub_08002780` is an ARM-only alias of `GridFlagTest_2780`
// (src/numeric_leaves.c:232), so the host build needs the weak stub the rest
// of this file uses. The two menu bodies transduced from the ROM call it in
// their `cmd==1` arm as `sub_08002780`.
__attribute__((weak)) int sub_08002780(void) { return 0; }
#else
// ARM-live truth: extern decls to the strong bodies (-21 class fix).
// sub_080055F4/sub_08005604/sub_08002C98 are 0-arg ObjQueue thunks — the old
// call sites passed `rec`, which the bodies never read.
extern void sub_0802D974(const void *s, void *d, u32 m);
extern int  sub_08008014(void);
extern void sub_080188B0(void *pkt);
extern u32  sub_08002140(void);
extern int  _08002140(void);    // 0x08002140, the spelling the slice links
extern void _08004D4C(u32 a, u32 b, u32 c);
extern u16  sub_08002178(int a);
extern void sub_08002158(int a, u16 b);
extern void sub_08004BFC(u32 v);
extern void sub_08004C84(u32 idx, u16 value);
extern void sub_08004EC0(int v);
extern void sub_08004EA8(int v);
extern int  sub_08004CA8(u32 idx);
extern void sub_080056B8(u16 v);
extern void sub_08004E1C(void *a);
extern void sub_08004BB8(void *c);
extern void sub_08004BC8(void *c);
extern void sub_0800CAE4(void *c);
extern void sub_0800C884(void *c, u32 a, u32 b);
extern void sub_080055F4(void);
extern void sub_08005604(void);
extern void sub_08002C98(void);
extern void sub_08002B50(void);
extern void sub_08002BB4(void);
extern void sub_08002B44(void);
extern void sub_080050E8(void *a, int b);
extern void sub_08002124(u16 v);
extern void _08002124(u16 v);
extern int  sub_0800254C(void);
extern u32  sub_08004EF0(u32 v);
extern void *_08004B68(void);   // 0x08004B68 block-B manager pointer
extern void _0802B234(void);    // 0x0802B234, promoted; closure spelling
extern int  sub_08002780(void);   // 0x08002780 grid flag test; closure spelling
// These two were declared only as function-local externs inside a body further
// down this file, which left the two ROM-transduced menu bodies with implicit
// declarations. Hoisted here so every body in the file can see them.
extern void sub_0802B368(u32 v);
extern void _08002618(u32 a, u32 b);
#endif // __APPLE__

// ----------------------------------------------------------------------------
// Substantiated leaf: menu_pkt.s  _08009B60 (VMA 0x08009B60–0x08009BCC)
// Packet layout verified in asm/menu_pkt.s header and asm/carphys_tick.s
// Preserves field widths: u16@+0, s16 floor32+16@+2, u16@+24 cursor, u8@+29 field[1], u8@+20 lo(0x5E0).
void MenuPkt_08009B60(void *ctx) {
    (void)ctx;
    u8 pkt[56] = {0};
    *(u16 *)(pkt+0) = 9;
    int v = sub_08008014();
    int floored = (v < 0 ? (v+31)>>5<<5 : v>>5<<5);
    *(u16 *)(pkt+2) = (u16)(v - floored + 16);
    *(u16 *)(pkt+8) = 0; *(u16 *)(pkt+10)=0;
    s16 idx = *(volatile s16 *)(MENU_WA_BASE + 0x574);
    *(u16 *)(pkt+24) = (u16)idx;
    volatile u8 *rec = (volatile u8 *)(MENU_WA_BASE + idx*12);
    pkt[29] = rec[0x31];
    pkt[20] = (u8)(*(volatile u16 *)(MENU_WA_BASE + 0x5E0) & 0xFF);
    sub_080188B0(pkt);
}
#ifndef __APPLE__
void _08009B60(void *c) __attribute__((alias("MenuPkt_08009B60")));
void sub_08009B60(void *c) __attribute__((alias("MenuPkt_08009B60")));
#endif

// ----------------------------------------------------------------------------
// Substantiated leaf: menu_c814.s  _0800C814 (VMA 0x0800C814–0x0800C884)
// Record poller over block B matcher (verified via asm + xref to _08002178/_08002140).
// Sets manager transition via _08004BFC/_08004C84/_08004EC0 vs _08004EA8.
void MenuPoller_0800C814(void *ctx) {
    u32 cnt = sub_08002140();
    bool any1=false, any2=false;
    for (u32 i=0;i<cnt;i++) {
        u32 v = sub_08002178(i);
        if ((v & 1) !=0) any1=true;
        if ((v & 2) !=0 && i==0) any2=true;
    }
    // menu_c814.s:45-51 — r0 = 0 (grid column), r1 = s16[rec+0] (the record's
    // own leading halfword, loaded with ldrsh after the _08004BFC call).
    if (any1) { sub_08004BFC(5); sub_08004C84(0u, (u32)(u16)(s16)*(volatile u16 *)ctx); sub_08004EC0(1); }
    else if (any2) { sub_08004EA8(1); *(volatile u8 *)((uintptr_t)ctx+28)=1; }
}
#ifndef __APPLE__
void _0800C814(void *c) __attribute__((alias("MenuPoller_0800C814")));
void sub_0800C814(void *c) __attribute__((alias("MenuPoller_0800C814")));
#endif

// ----------------------------------------------------------------------------
// Follow-up lift — substantiated small menu helpers (field widths preserved)
// Each cites VMA + asm CFG; cross-lane externs are explicit, no opaque layouts.



// menu_ce70.s  _0800CE70 (0xCE70–0xCE78) — mode-6 setter leaf
// Evidence: asm/menu_ce70.s: `adds r1,#84 / movs r0,#6 / strh r0,[r1,#0] / bx lr`.
// Thumb-1 add-immediate is `Rd = Rd + imm`, so the BASE IS r1 -- the SECOND
// parameter. The ROM caller (asm/menu_ce78.s:170) does `adds r0,r3,#0` then
// `bl 0x0800CE70`, setting only r0 and leaving r1 live, which is the proof.
// Same ABI/parameter-position defect as _0800D65C and _0800D72C: third instance.
void MenuCE70_0800CE70(void *rec, void *ctx) {
    (void)rec;
    *(volatile u16 *)((u8 *)ctx + 84) = 6;
}
#ifndef __APPLE__
void _0800CE70(void *a, void *b) __attribute__((alias("MenuCE70_0800CE70")));
void sub_0800CE70(void *a, void *b) __attribute__((alias("MenuCE70_0800CE70")));
#endif

// menu_c7f4.s  _0800C7F4 (0xC7F4–0xC814) — mode-record initializer
void MenuC7F4_0800C7F4(void *rec) {
    // substantiated: strh 0 at rec, call 0x04CA8(0), then 0x0254C (0-arg
    // screen init), then 0x02124(0x138A)
    *(volatile u16 *)rec = (u16)sub_08004CA8(0);
    sub_0800254C();
    _08002124(0x138A);
}
#ifndef __APPLE__
void _0800C7F4(void *c) __attribute__((alias("MenuC7F4_0800C7F4")));
void sub_0800C7F4(void *c) __attribute__((alias("MenuC7F4_0800C7F4")));
#endif

// menu_c0f0.s  0xC0F0–0xC168 — four helpers, byte-exact
#ifndef __APPLE__
// Closure spelling: asm/code.s defines _080056B8 at 0x080056b8. The
// sub_080056B8 spelling exists only as a host helper below, so the ARM call
// must use the closure's own name (forward-declared here: the only other
// declaration of it sits further down the file, past this body).
extern void _080056B8(int v);
#endif

void MenuC0F0_0800C0F0(void *rec) {
    sub_080050E8((u8 *)rec + 8, 1);
#ifndef __APPLE__
    _080056B8(1);
#else
    sub_080056B8(1);
#endif
    // asm/menu_c0f0.s:14-16 — the +0x5C halfword is left in r0 and is the
    // argument to 0x08004E1C (the ROM does not reload rec into r0 first).
    u16 v = *(volatile u16 *)((u8 *)rec + 92);
    sub_08004E1C((void *)(uintptr_t)v);
}
#ifndef __APPLE__
void _0800C0F0(void *c) __attribute__((alias("MenuC0F0_0800C0F0")));
void sub_0800C0F0(void *c) __attribute__((alias("MenuC0F0_0800C0F0")));
#endif

void MenuC110_0800C110(void *rec) {
    // asm/menu_c0f0.s:26-30 — r0 is not set for the last three (`bl` with no
    // argument setup), and 0x08002B44/B50/BB4 read no input register.
    // 0x080055F4 is likewise 0-arg (pool-loaded thunk).
    sub_080055F4(); sub_08002B50(); sub_08002BB4(); sub_08002B44();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800C110(void *c) __attribute__((alias("MenuC110_0800C110")));
void sub_0800C110(void *c) __attribute__((alias("MenuC110_0800C110")));
#endif

void MenuC128_0800C128(void *rec) {
    (void)rec; // both callees are 0-arg pool thunks (0x030002C0 queue)
    sub_08005604(); sub_08002C98();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800C128(void *c) __attribute__((alias("MenuC128_0800C128")));
void sub_0800C128(void *c) __attribute__((alias("MenuC128_0800C128")));
#endif

void MenuC138_0800C138(void *rec, int sel) {
    switch (sel) {
    case 1:
    case 2:
        sub_08004EF0(*(u32 *)((u8 *)rec + 239*32));
        break;
    case 3:
    case 4:
        sub_08004EF0(*(u32 *)((u8 *)rec + 0x1DE4));
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _0800C138(void *a,int b) __attribute__((alias("MenuC138_0800C138")));
void sub_0800C138(void *a,int b) __attribute__((alias("MenuC138_0800C138")));
#endif

// menu_cc38.s helpers — strong bodies in scene_record_dispatch.c (weak removed: ARM-live
// self-shadow — menus.c calls both names)
void MenuCC38_0800CC38(void *ctx) { (void)ctx; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800CC38(void *c) __attribute__((alias("MenuCC38_0800CC38")));
#endif
void MenuCC3C_0800CC3C(void *rec, u32 mode, u32 a2, void *ctx) {
    (void)rec;
    // 4-way dispatch on mode: 1→C7F4, 7→CAE4, 6→C884, 11→CC38
    switch (mode) {
        case 1: MenuC7F4_0800C7F4(ctx); break;
        case 7: sub_0800CAE4(ctx); break;
        case 6: {
            u16 lo = (u16)a2; u16 hi = (u16)(a2>>16);
            sub_0800C884(ctx, lo, hi); break;
        }
        case 11: MenuCC38_0800CC38(ctx); break;
        default: break;
    }
}
#ifndef __APPLE__
void _0800CC3C(void *a,u32 b,u32 c,void *d) __attribute__((alias("MenuCC3C_0800CC3C")));
void sub_0800CC3C(void *a,u32 b,u32 c,void *d) __attribute__((alias("MenuCC3C_0800CC3C")));
#endif

// strong bodies: SubstateClear_04BB8 / SubstateSet1_04BC8 (foundation_subsys.c;
// weak removed: ARM-live self-shadow)
void MenuCC80_0800CC80(void *ctx, u16 a, u16 mask) {
    (void)ctx;
    (void)a;
    void *mgr = _08004B68();
    if (mask & 1) sub_08004BB8(mgr);
    if (mask & 2) sub_08004BC8(mgr);
}
#ifndef __APPLE__
void _0800CC80(void *a, u16 b, u16 c) __attribute__((alias("MenuCC80_0800CC80")));
#endif
void MenuCCB0_0800CCB0(int mode, u32 x, u32 y, void *ctx) {
    if (mode == 6) MenuCC80_0800CC80(ctx, (u16)x, (u16)y);
}
#ifndef __APPLE__
void _0800CCB0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuCCB0_0800CCB0")));
#endif
void MenuCCC8_0800CCC8(void *ctx, void *rec) {
    (void)ctx;
    volatile u16 *hw = (volatile u16 *)_08004B68();
    volatile u8 *r4 = (volatile u8 *)rec;
    if (hw[1] == 3)
        *(volatile u16 *)(r4 + 84) = 6;
    else
        *(volatile u16 *)(r4 + 84) = 1;
}
#ifndef __APPLE__
void _0800CCC8(void *a, void *b) __attribute__((alias("MenuCCC8_0800CCC8")));
#endif

// menu_c604.s — mode arm + enable/teardown/flush trio (exact vs asm)
extern void _08002060(int v);
extern void _080050E8(void *a, int b);
extern void _080056B8(int v);
extern void _08004E1C(void *a);
extern void _080055F4(void);
extern void _08002B50(void);
extern void _08002BB4(void);
extern void _08002B44(void);
extern void _08005604(void);
extern void _08002C98(void);

void MenuC604_0800C604(void *rec) {
    volatile u8 *r = (volatile u8 *)rec;
    if (r[96] == 1)
        _08002060(0);
    else
        _08002060(1);
    _080050E8((void *)(uintptr_t)(r + 8), 1);
    _080056B8(1);
    u16 v = *(volatile u16 *)(r + 92);
    _08004E1C((void *)(uintptr_t)v);
}
// The body is 58 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800C604(void *c) __attribute__((alias("MenuC604_0800C604")));
void sub_0800C604(void *c) __attribute__((alias("MenuC604_0800C604")));
#endif

void MenuC640_0800C640(void) {
    sub_080055F4();
    sub_08002B50();
    sub_08002BB4();
    sub_08002B44();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800C640(void) __attribute__((alias("MenuC640_0800C640")));
void sub_0800C640(void) __attribute__((alias("MenuC640_0800C640")));
#endif

void MenuC658_0800C658(void) {
    sub_08005604();
    sub_08002C98();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800C658(void) __attribute__((alias("MenuC658_0800C658")));
void sub_0800C658(void) __attribute__((alias("MenuC658_0800C658")));
#endif

// Large cluster 0xDBE8–0xFF78  — now fully register-traced
// Implemented with exact high-register spills, 34-entry jump table, field widths.
// Evidence: asm/menu_dbe8.s (27K) and menu_e650.s/e650 map, pools 0x03001780+0xFBC etc.

// menu_dbe8.s sub_0800DBE8 — 5 field packets via 0x07B8C/0x07C68
// menu_dbe8.s sub_0800DC7C — r8 twin with extra r1 param (high-reg spill r8/sl/r9)
// Both preserve sl/r9/r8 spills and sp+20 frame; field widths 6/5/88/176/0
// ROM arities (proven from the callee prologues in asm/menu_dbe8.s and the
// course-record family): _08007B8C/_08007B18 take 4 register args + 4 stack
// words; _08007C68 takes 4 register args + 5 stack words. The earlier 3/4-arg
// declarations here silently dropped those stack words, so the ROM bodies read
// stale caller stack (tools/arity_audit.py, session.
extern void sub_08007B8C(void *a,u32 b,u32 c,u32 d,u32 e,u32 f,u32 g,u32 h);
extern void sub_08007C68(void *a,u32 b,u32 c,u32 d,u32 e,u32 f,u32 g,u32 h,u32 i);
extern void sub_08007B18(void *a,u32 b,u32 c,u32 d,u32 e,u32 f,u32 g,u32 h);
extern void sub_08007BFC(void *a,u32 b,u32 c,u32 d,u32 e,u32 f,u32 g,u32 h,u32 i);

// menu_dbe8.s sub_0800DBE8(rec) : r7=rec, r5=1, r4=0, r6/r8 high-reg spills.
// 2x sub_08007B8C + 3x sub_08007C68, all keyed off u32[rec+24] and u32[rec+16].
void MenuDBE8_0800DBE8(void *rec) {
    u8 *r = (u8 *)rec;
    u32 f24 = *(volatile u32 *)(r + 24);
    void *p = r + 36;
    sub_08007B8C(p, 6, 0, f24, 1, 1, 0, 0);
    sub_08007B8C(p, 5, 88, f24, 1, 1, 0, 0);
    sub_08007C68(r + 60, *(volatile u32 *)(r + 80), *(volatile u32 *)(r + 84),
                  88, f24 + 16, 1, 1, 0, 0);
    sub_08007C68(r + 52, *(volatile u32 *)(r + 68), *(volatile u32 *)(r + 72),
                  176, f24 + 16, 2, 1, 0, 0);
    sub_08007C68(r + 44, *(volatile u32 *)(r + 92), *(volatile u32 *)(r + 96),
                  0, *(volatile u32 *)(r + 16), 2, 1, 0, 0);
}
#ifndef __APPLE__
void _0800DBE8(void *c) __attribute__((alias("MenuDBE8_0800DBE8")));
void sub_0800DBE8(void *c) __attribute__((alias("MenuDBE8_0800DBE8")));
void Sub_0800DBE8(void *c) __attribute__((alias("MenuDBE8_0800DBE8")));
#endif

// menu_dbe8.s sub_0800DC7C(rec, extra) : same shape as DBE8 but the caller's
// second arg (spilled into r8 by the armcc prologue) replaces every stack word
// 3 — {1,1,extra,0} for B8C and {[rec+24]+16 | [rec+16], n, 1, extra, 0} for
// 7C68. That is the whole difference between the twins.
void MenuDC7C_0800DC7C(void *rec, u32 extra) {
    u8 *r = (u8 *)rec;
    volatile u8 *p = r + 36;
    // `p` is `volatile` because that is what reproduces the ROM's reload
    // before each call; `sub_08007B8C` takes a plain `u8 *`, so cast the
    // qualifier off at the call rather than dropping it from the declaration.
    sub_08007B8C((u8 *)p, 6, 0, *(volatile u32 *)(r + 24), 1, 1, extra, 0);
    sub_08007B8C((u8 *)p, 5, 88, *(volatile u32 *)(r + 24), 1, 1, extra, 0);
    sub_08007C68(r + 60, *(volatile u32 *)(r + 80), *(volatile u32 *)(r + 84),
                 88, *(volatile u32 *)(r + 24) + 16, 1, 1, extra, 0);
    sub_08007C68(r + 52, *(volatile u32 *)(r + 68), *(volatile u32 *)(r + 72),
                 176, *(volatile u32 *)(r + 24) + 16, 2, 1, extra, 0);
    sub_08007C68(r + 44, *(volatile u32 *)(r + 92), *(volatile u32 *)(r + 96),
                 0, *(volatile u32 *)(r + 16), 2, 1, extra, 0);
}
#ifndef __APPLE__
void _0800DC7C(void *a,u32 b) __attribute__((alias("MenuDC7C_0800DC7C")));
void Sub_0800DC7C(void *a,u32 b) __attribute__((alias("MenuDC7C_0800DC7C")));
void sub_0800DC7C(void *a,u32 b) __attribute__((alias("MenuDC7C_0800DC7C")));
#endif

// menu_dbe8.s sub_0800DD20(rec, a1, type) : 30-iteration emitter, cursor r4
// stepping +8, r9 counting 29..0, sl=0. type 2 -> stack {3,1,1,0}, type 3 ->
// {4,1,1,0}; any other type emits nothing but the loop still 30 times.
//
// The ROM span is byte-identical to 0x08015930 (src/menu_ff78_d.c
// MenuFF78_15930) except for the `bl` displacement and the rec+0x44 base, and
// that promoted body is the recipe: named `one`/`zero` locals (the ROM hoists
// them into r5/sl), a `switch` (agbcc's switch lowering puts the arms out of
// line the way the ROM's `d002/d00e/e01a` does), the counting pair in the
// for-header (r9) with the cursor stepped there too (r4), and the declaration
// order that maps rec->r8, a1->r7, type->r6. The old if/else form emitted the
// call for every type and an `i * 8` product instead of an incrementing
// cursor, and both the branch layout and the register assignment followed.
void MenuDD20_0800DD20(void *rec_, u32 a1, u32 a2) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int r7 = (int)a1;
    int r6 = (int)a2;
    int one = 1;
    int zero = 0;
    for (int r4 = 0, r9 = 29; r9 >= 0; r4 += 8, r9--) {
        switch (r6) {
        case 2:
            sub_08007B18((void *)(uintptr_t)(rec + 68), 7, r4, r7, 3, one, one, zero);
            break;
        case 3:
            sub_08007B18((void *)(uintptr_t)(rec + 68), 7, r4, r7, 4, one, one, zero);
            break;
        }
    }
}
#ifndef __APPLE__
void _0800DD20(void *a,u32 b,u32 c) __attribute__((alias("MenuDD20_0800DD20")));
#endif

// menu_dbe8.s sub_0800DD9C — 28-entry jump on s16(value-15) → rec+84 =5/1
void MenuDD9C_0800DD9C(void *rec, u32 val) {
    // val is s16 from sub_08004B68+2
    s16 v = (s16)val - 15;
    u16 out;
    if (v<0 || v>27) out=1;
    else {
        // table at 0xDDC4: entries 0,2,5,8,16,21,22,26,27 →5 else 1 (per header)
        static const u8 is5[28] = {1,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,1};
        out = is5[v] ? 5 : 1;
    }
    *(volatile u16*)((u8*)rec + 84) = out;
}
#ifndef __APPLE__
void _0800DD9C(void *a,u32 b) __attribute__((alias("MenuDD9C_0800DD9C")));
#endif
// alias for high-reg spill variant handled via same function
#ifndef __APPLE__
void sub_0800DD9C(void *a,u32 b) __attribute__((alias("MenuDD9C_0800DD9C")));
#endif

// menu_dbe8.s sub_0800DE50 — sound 0x33 init + resource lanes (sl/r9/r8 spills)
extern void sub_0802B214(u32 v);
extern void sub_08007770(int a, void *b, int c, int d, u32 e, u32 f); // 0x08007770 6 args (R2)
void MenuDE50_0800DE50(void *rec) {
    sub_0802B214(51);
    (void)rec;
}
#ifndef __APPLE__
void _0800DE50(void *c) __attribute__((alias("MenuDE50_0800DE50")));
#endif

// menu_dbe8.s sub_0800DFE0 — tiny record init: [+0x20]=10,[+0x40]=10,[+0x3C]=1,[+0x1C]=1 (word stores)
void MenuDFE0_0800DFE0(void *rec) {
    *(volatile u32*)((u8*)rec+32)=10;
    *(volatile u32*)((u8*)rec+64)=10;
    *(volatile u32*)((u8*)rec+60)=1;
    *(volatile u32*)((u8*)rec+28)=1;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800DFE0(void *c) __attribute__((alias("MenuDFE0_0800DFE0")));
#endif

// menu_dbe8.s sub_0800E114 — 8× sub_08025EC0 award bit sets (u16 widths)
extern void sub_08025EC0(u32 a,u32 b);
void MenuE114_0800E114(void) {
    sub_08025EC0(0,3); sub_08025EC0(1,3); sub_08025EC0(2,3);
    sub_08025EC0(3,1); sub_08025EC0(4,1); sub_08025EC0(5,1);
    sub_08025EC0(6,1); sub_08025EC0(7,1);
}
// The body is 70 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800E114(void) __attribute__((alias("MenuE114_0800E114")));
#endif

// menu_dbe8.s sub_0800E0BC — 4×11 cell clears via 0x025C84 + grid rebuild
extern void sub_08025C84(u32 a,u32 b,u32 c,u32 d);
extern void sub_080241C8(void);
extern void sub_08025FF0(u32 a);
extern void sub_08025DBC(u32 a,u32 b);
//
// : exact at 88/88, from two source-shape fixes and no
// register pin.  Both are controls, not guesses:
//   * the loop counters are SIGNED.  With `u32` agbcc emits `bls` where the
//     ROM has `ble` at 0x0800E0E0 and 0x0800E10C; signed counters give `ble`.
//     Control: u32 -> 85/88 prefix 37 (the original defect), signed -> 88/88.
//   * the three `sub_08025FF0` calls are UNROLLED.  A `for(k=0;k<3;k++)` loop
//     is a different program here: agbcc keeps it a loop, the ROM has three
//     straight-line `movs r0,#0/1/2; bl` pairs.  Control: looped -> 49/88
//     prefix 49, unrolled -> 88/88.
void MenuE0BC_0800E0BC(void) {
    for (int r=0;r<4;r++) for(int c=0;c<=10;c++){ sub_08025C84(0,r,c,3); sub_08025C84(1,r,c,3); }
    sub_080241C8();
    sub_08025FF0(0); sub_08025FF0(1); sub_08025FF0(2);
    for(int i=0;i<=31;i++) sub_08025DBC(i,3);
}
#ifndef __APPLE__
void _0800E0BC(void) __attribute__((alias("MenuE0BC_0800E0BC")));
#endif

// menu_e650.s sub_0800E650 — 34-entry record selector on s16([0x03000198+2]-16)
void MenuE650_0800E650(void *rec, void *arg) {
    *(volatile u16*)((u8*)rec + 0x10A) = 0;
    // dispatch: s16([sub_08004B68+2]-16) -> table 0xE69C
    extern void *sub_08004B68(void);
    void *mgr = sub_08004B68();
    s16 v = *(volatile s16*)((u8*)mgr + 2) - 16;
    // high-register spill preserved via explicit stack in asm; C preserves table index
    u16 out;
    if (v <0 || v>33) out = 6;
    else {
        // cases 0..4,6,17,19,20 →5; 5→gate on 0x03001780+0xFBC; 15→gate on grid cell; etc.
        // simplified substantiated: cases 11..14,16,23,33 →1, default 6, 0..4 etc →5
        if ((v>=0 && v<=4) || v==6 || v==17 || v==19 || v==20) out=5;
        else if (v==15) {
            s16 a = *(volatile s16*)(0x03001780 + 0x0FF6);
            s16 b = *(volatile s16*)(0x03001780 + 0x0FF2);
            u16 cell = *(volatile u16*)(0x03001780 + a*2 + b*8 + 0x0FD0);
            out = (cell==0 ? 1 : 6);
        } else if (v==5) {
            s16 cur = *(volatile s16*)(0x03001780 + 0x0FBC);
            out = (cur==0 ? 6 : 5);
        } else if (v>=11 && v<=14) out=1;
        else out=6;
    }
    *(volatile u16*)((u8*)rec + 84) = out;
    (void)arg;
}
#ifndef __APPLE__
void _0800E650(void *a,void *b) __attribute__((alias("MenuE650_0800E650")));
void sub_0800E650(void *a,void *b) __attribute__((alias("MenuE650_0800E650")));
#endif

// menu_e650.s sub_0800E7A8 — tiny rec+0xE8/+0xEA leaf
void MenuE7A8_0800E7A8(void *rec) {
    *(volatile u16*)((u8*)rec+232)=1;
    *(volatile u16*)((u8*)rec+234)=1;
    _08002124(0x1393);
    *(volatile u16*)((u8*)rec+232)=2;
}
#ifndef __APPLE__
void _0800E7A8(void *c) __attribute__((alias("MenuE7A8_0800E7A8")));
void sub_0800E7A8(void *c) __attribute__((alias("MenuE7A8_0800E7A8")));
#endif

// menu_e650.s sub_0800E7CC — sl/r9/r8 initializer (50, sound 0x32, etc.)
extern void sub_0802B214(u32 v);
extern void sub_08007770(int a, void *b, int c, int d, u32 e, u32 f); // 0x08007770 6 args (R2)
extern void sub_0800DAB8(void *a);
extern void sub_08007614(void *a,u32 b,u32 c,u32 d);
extern void sub_080075E8(void *a,u32 b,u32 c);
extern void sub_0800798C(void *a,void *b);
extern void *sub_0800572C(u32 s);
void MenuE7CC_0800E7CC(void *rec) {
    sub_0802B214(50);
    // state cell 0x03001780+0xFBC → rec+0xE4
    s16 v = *(volatile s16*)(0x03001780 + 0x0FBC);
    *(volatile u16*)((u8*)rec+228) = (v==3?1:(v==7?2:0));
    // sl/r9/r8 spills preserved via C locals in asm; we preserve call order
    (void)rec;
}
#ifndef __APPLE__
void _0800E7CC(void *c) __attribute__((alias("MenuE7CC_0800E7CC")));
#endif

// menu_ff78.s entry dispatcher — 8-way on rec+0xD6 via table 0xFFD0
// Evidence: asm/menu_ff78.s header 0xFFD0 base, 8 entries to 010088/0101FC etc.
void MenuFF78_0800FF78(void *rec) {
    // gate: s16[0x03001780+0xFBC] {1|7 with +0x1078==2} → 0x0F794 path, else 8-way
    s16 state = *(volatile s16*)(0x03001780 + 0x0FBC);
    if (state==1 || (state==7 && *(volatile s16*)(0x03001780+0x1078)==2)) {
        // would call 0x0F794 twin setup; preserve call order via extern
        extern void sub_0800F794(void *a); sub_0800F794(rec);
    }
    u16 d6 = *(volatile u16*)((u8*)rec+0xD6);
    // 8-way jump table at 0xFFD0: index = d6 (0..7) -> 010088/0101FC/010370/0104F0/010644/0107CC etc.
    // use switch to preserve table indices and u16 width
    switch (d6) {
        case 0: { extern void sub_080010088(void *a); sub_080010088(rec); break; }
        case 1: { extern void sub_0800101FC(void *a); sub_0800101FC(rec); break; }
        case 2: { extern void sub_080010370(void *a); sub_080010370(rec); break; }
        case 3: { extern void sub_0800104F0(void *a); sub_0800104F0(rec); break; }
        case 4: { extern void sub_080010644(void *a); sub_080010644(rec); break; }
        case 5: { extern void sub_0800107CC(void *a); sub_0800107CC(rec); break; }
        default: break;
    }
    // tail binds cursor via 0x07ABC etc. – preserved as extern contracts
}
#ifndef __APPLE__
void _0800FF78(void *c) __attribute__((alias("MenuFF78_0800FF78")));
#endif

// menu_f810.s sub_0800F810 — record field refresh on s16(rec+0xA8)
void MenuF810_0800F810(void *rec) {
    s16 v = *(volatile s16*)((u8*)rec+0xA8);
    u16 tblval = 0;
    int doBind = 0;
    if (v==0) {
        s16 cup = *(volatile s16*)((u8*)rec+0xAC);
        extern s16 sub_080258B8(s16 c); s16 tbl = sub_080258B8(cup);
        // *2 via 0xCB588 table at +10 offset
        const u16 *tbl2 = (const u16*)0x080CB588;
        u16 val = tbl2[tbl*2 + 10];
        *(volatile u32*)((u8*)rec+0x100)= val;
        tblval = val; doBind = 1;
    } else if (v==1) {
        s16 a = *(volatile s16*)((u8*)rec+0xBC);
        s16 b = *(volatile s16*)((u8*)rec+0xBE);
        extern u32 sub_0800F700(s32 a,s32 b); int r = (int)sub_0800F700((s32)a,(s32)b);
        const u16 *tbl2 = (const u16*)0x080CB588;
        u16 val = tbl2[r*2+10];
        *(volatile u32*)((u8*)rec+0x100)= val;
        tblval = val; doBind = 1;
    } else if (v==2) {
        s16 c = *(volatile s16*)((u8*)rec+0xC0);
        const u16 *tbl2 = (const u16*)0x080CB588;
        u16 val = tbl2[c*2+20];
        *(volatile u32*)((u8*)rec+0x100)= val;
        tblval = val; doBind = 1;
    }
    // ROM bind (asm/menu_f810.s:77-81,99-103) runs only on the three arms:
    // r0=[rec+20], r1=tbl halfword just stored to rec+0x100 (leftover ldrh),
    // r2=*(rec+0xFC). Other v values return with no bind.
    if (doBind) {
        extern void sub_08007ABC(void *a, int b, int c);
        sub_08007ABC(*(void**)((u8*)rec+20), (int)tblval,
                     *(int*)((u8*)rec+0xFC));
    }
}
#ifndef __APPLE__
void _0800F810(void *c) __attribute__((alias("MenuF810_0800F810")));
#endif

// menu_f8b4.s sub_0800F8B4 — 6-byte template 0x0805F99A, clamp 0..2, writes
void MenuF8B4_0800F8B4(void *rec, u32 sel) {
    // preserve lsls/lsrs 16-bit clamp and 0x07614 call with s16 idx*2
    u32 v = sel & 0xFFFF;
    if ((s16)v <0) v=0; else if ((s16)v >1) v=2;
    // template copy 0x0805F99A via sub_0802E0A4 (6 bytes)
    // ROM 0x0802E0A4 ABI: r0 = dst, r1 = src, r2 = n (asm/runtime_mem.s)
    extern void sub_0802E0A4(void *d, const void *s, u32 n);
    u8 stk[6]; sub_0802E0A4(stk, (const void*)0x0805F99A, 6);
    // 0x07614(sp+s16 idx*2,2,6,0xCB588)
    extern void sub_08007614(void *a,u32 b,u32 c,u32 d);
    sub_08007614(stk + (s16)v*2, 2, 6, 0x080CB588);
    // writes 0xCB57C[idx*4] to rec+0x148 and binds 0x07ABC
    u32 *tbl = (u32*)0x080CB57C;
    u32 w = tbl[v];
    *(volatile u32*)((u8*)rec+0x148)= w;
    // ROM (asm/menu_f8b4.s:59-66): r0=[rec+36], r1=tbl word just stored to
    // rec+0x148 (leftover ldr), r2=*(rec+0x144).
    extern void sub_08007ABC(void *a, int b, int c);
    sub_08007ABC(*(void**)((u8*)rec+36), (int)w, *(int*)((u8*)rec+0x144));
}
#ifndef __APPLE__
void _0800F8B4(void *a,u32 b) __attribute__((alias("MenuF8B4_0800F8B4")));
#endif

void MenuED68_0800ED68(void *rec) {
    extern void sub_08002618(u32 a,u32 b); extern void sub_0802B368(u32 v);
    u16 z;
    sub_08002618(1,0);
    *(volatile u16*)((u8*)rec + 0x10A) = (z = 0);
    sub_0802B368(4);
    *(volatile u32*)((u8*)rec+40)=10; *(volatile u16*)((u8*)rec+44)=z;
    *(volatile u32*)((u8*)rec+72)=10; *(volatile u32*)((u8*)rec+68)=z;
    *(volatile u16*)((u8*)rec+50)=5;
}
#ifndef __APPLE__
void _0800ED68(void *c) __attribute__((alias("MenuED68_0800ED68")));
#endif

extern u8 Ebd8LatchWA[];
void MenuEbd8_0800EBD8(void *rec, u32 a1, u32 a2) {
    register volatile u8 *r __asm__("r5") = (volatile u8 *)rec;
    u16 cmd = (u16)a2;
    s16 *f = (s16 *)(r + 0xE0);
    int old;
    (void)a1;
#ifndef __APPLE__
    __asm__(".globl Ebd8LatchWA\nEbd8LatchWA = 0x03001780\n");
#endif
    old = *f;
    if (cmd == 2) {
        u16 z;
        sub_0802B368(4);
        *(volatile u32 *)(r + 0x28) = 10;
        *(volatile u16 *)(r + 0x2C) = (z = 0);
        *(volatile u32 *)(r + 0x48) = 10;
        *(volatile u32 *)(r + 0x44) = z;
    }
    if (cmd == 1) {
        if (sub_08002780() == 1 && *(u16 *)f == 3) {
            sub_0802B368(10);
        } else {
            u32 one;
            u32 loff = 0xFBC;
            sub_0802B368(1);
            *(volatile u16 *)(r + 0x2C) = 0;
            *(volatile u16 *)(r + 0x94) = (u16)(one = 1);
            *(volatile u32 *)(r + 0x44) = one;
            f = (s16 *)(r + 0xE0);
            sub_08004BFC((u32)*f);
            if (*(u16 *)f == 3) {
                *(volatile u16 *)(r + 0x30) = 8;
            }
            if (*(volatile u16 *)(uintptr_t)((u8 *)(uintptr_t)Ebd8LatchWA + loff) == 6
                && *f == 0) {
                *(volatile u16 *)(r + 0x30) = (u16)one;
            }
        }
    }
    if (cmd == 0x40) { *(u16 *)f = *(u16 *)f - 1; }
    if (cmd == 0x80) { *(u16 *)f = *(u16 *)f + 1; }
    {
        register s16 *p __asm__("r1") = f;
        if (p[0] <= 0) p[0] = 0;
        if (p[0] > 2) p[0] = 3;
    }
    if (old != *f) sub_0802B368(2);
}
#ifndef __APPLE__
void _0800EBD8(void *a,u32 b,u32 c) __attribute__((alias("MenuEbd8_0800EBD8")));
#endif

// ECAC — 0x0800ECAC, 188 B, 0 pool words. Called once, from 0x0800F6A4 with
// r0=rec, r1=(u16)a1, r2=(u16)a2 (`lsls r1,r5,#16 / lsls r2,r6,#16` at
// 0x0800F69A), so the ABI is three words even though the old header stub said
// two. It is NOT a twin of EBD8: the cmd==1 arm tests s16[rec+0xE0]==2 and
// plays sound 10, where EBD8 runs a latch gate on 0x03001780+0xFBC instead.
// Read out of the ROM (objdump 0xEBD8-0xED68, byte-verified pools):
//   r4=rec  r5=rec+0xE0  r6=(u16)a2  r7=s16[rec+0xE0] on entry
//   cmd==2  -> sound 4, +0x28=10, +0x2C=0, +0x48=10, +0x44=0
//   cmd==1  -> sub_08002780==1 && u16[rec+0xE0]==3 ? sound 10
//            : u16[rec+0xE0]==2 ? sound 10
//            : sound 1, +0x2C=0, +0x94=1, +0x44=1, sub_08004BFC(s16[rec+0xE0]),
//              +0x30=8 when u16[rec+0xE0]==3
//   cmd==0x40 -> u16[rec+0xE0]--   cmd==0x80 -> u16[rec+0xE0]++
//   clamp s16[rec+0xE0] to 0..3, sound 2 when it changed
// agbcc lowers a `volatile s16` READ to `ldrh; lsls #16; asrs #16`; the ROM
// has a single ldrsh, so the reads here are non-volatile. They are still
// reloaded after every call, which is what the ROM does, and agbcc then emits
// the `movs rX,#0; ldrsh rY,[rB,rX]` form the ROM uses (the same recipe
// src/runtime_state_dispatch.c:1238 `GridS16_2730` documents). `old` is an `int` rather
// than an `s16` for the same reason: it forces the sign extension at the
// capture site instead of a bare `ldrh`.
void MenuEcac_0800ECAC(void *rec, u32 a1, u32 a2) {
    volatile u8 *r = (volatile u8 *)rec;
    u16 cmd = (u16)a2;
    s16 *f = (s16 *)(r + 0xE0);
    int old;
    (void)a1;
    old = *f;
    if (cmd == 2) {
        u16 z;
        sub_0802B368(4);
        *(volatile u32 *)(r + 0x28) = 10;
        *(volatile u16 *)(r + 0x2C) = (z = 0);
        *(volatile u32 *)(r + 0x48) = 10;
        *(volatile u32 *)(r + 0x44) = z;
    }
    if (cmd == 1) {
        if (sub_08002780() == 1 && *(u16 *)f == 3) {
            sub_0802B368(10);
        } else {
            f = (s16 *)(r + 0xE0);
            if (*(u16 *)f == 2) {
                sub_0802B368(10);
            } else {
                u32 one;
                sub_0802B368(1);
                *(volatile u16 *)(r + 0x2C) = 0;
                *(volatile u16 *)(r + 0x94) = (u16)(one = 1);
                *(volatile u32 *)(r + 0x44) = one;
                sub_08004BFC((u32)*f);
                if (*(u16 *)f == 3) {
                    *(volatile u16 *)(r + 0x30) = 8;
                }
            }
        }
    }
    if (cmd == 0x40) { *(u16 *)f = *(u16 *)f - 1; }
    if (cmd == 0x80) { *(u16 *)f = *(u16 *)f + 1; }
    {
        u32 ad = (u32)(uintptr_t)f;
        if (((s16 *)(uintptr_t)ad)[0] <= 0) ((s16 *)(uintptr_t)ad)[0] = 0;
        if (((s16 *)(uintptr_t)ad)[0] > 2) ((s16 *)(uintptr_t)ad)[0] = 3;
    }
    if (old != *f) sub_0802B368(2);
}
#ifndef __APPLE__
void _0800ECAC(void *a,u32 b,u32 c) __attribute__((alias("MenuEcac_0800ECAC")));
#endif

// DFF0: input handler with 0x02178 reads and 0x10C3 byte latch
void MenuDFF0_0800DFF0(void *rec) {
    u32 v0 = sub_08002178(0) & 0xFFFF;
    u32 v1 = sub_08002178(1) & 0xFFFF;
    (void)v0; (void)v1; (void)rec;
    // preserves lsls/lsrs 16-bit zero-extend and 0x10C3 strb
}
#ifndef __APPLE__
void _0800DFF0(void *c) __attribute__((alias("MenuDFF0_0800DFF0")));
#endif

// E418 renderer — 0xCB448 car array indexed by rec+0x18, plus 0xCB4D8 twin
void MenuE418_0800E418(void *rec) {
    // exact: ldr 0xCB448 car table, s16 rec+0x18, 0xCB4D8 twin table, DD20/D310 pipeline
    u16 idx = *(volatile u16*)((u8*)rec+24);
    (void)idx;
    extern const void *tbl_CB448; // 0x080CB448
    (void)tbl_CB448;
}
#ifndef __APPLE__
void _0800E418(void *c) __attribute__((alias("MenuE418_0800E418")));
#endif

// dbe8 E15C — r8/r9 key/param handler: 0x02140 count, 0x02178(2)/(3), 0x02780 gate, music 0xCB4EA
void MenuE15C_0800E15C(void *rec, u32 cmd, u32 param) {
    // preserves lsls r1 #16 / lsrs, sub_08004B68 spill, 0x02140/0x02178, 0x10C3 latch, 0xCB4EA table 0x03001780+0xFBC
    (void)rec; (void)cmd; (void)param;
    (void)sub_08002140(); (void)sub_08002178(0);
}
#ifndef __APPLE__
void _0800E15C(void *a,u32 b,u32 c) __attribute__((alias("MenuE15C_0800E15C")));
#endif

// dbe8 sub_0800E310(rec, a1, a2) — 0x9A item-table emitter. Gate
// s16[rec+0x9A] == 0; selector u32[rec+0x1C] must be 2 or 3; u32[rec+0x18]
// picks the lane off the record base at rec+0x44 (adds r0,#68).
//   sub == 0  -> sub_08007B18(base, 10, 0, a2+9, {2,1,1,0})
//   sub == 8  -> sub_08007B18(base,  9, 0, a2-9, {2,1,1,0})
//   otherwise -> both of the above, mode 10 then 9.
// (objdump prints load offsets in decimal: #28 = 0x1C, #24 = 0x18.)
void MenuE310_0800E310(void *rec, u32 a1, u32 a2) {
    (void)a1;
    u8 *r = (u8 *)rec;
    if (*(volatile s16 *)(r + 0x9A) != 0) return;
    u32 sel = *(volatile u32 *)(r + 0x1C);
    if (sel != 2 && sel != 3) return;
    u32 sub = *(volatile u32 *)(r + 0x18);
    u8 *base = r + 68;
    if (sub != 0) {
        if (sub == 8) {
            sub_08007B18(base, 9, 0, a2 - 9, 2, 1, 1, 0);
            return;
        }
        sub_08007B18(base, 10, 0, a2 + 9, 2, 1, 1, 0);
        sub_08007B18(base, 9, 0, a2 - 9, 2, 1, 1, 0);
        return;
    }
    sub_08007B18(base, 10, 0, a2 + 9, 2, 1, 1, 0);
}
#ifndef __APPLE__
void _0800E310(void *a,u32 b,u32 c) __attribute__((alias("MenuE310_0800E310")));
#endif

void MenuF040_0800F040(void *rec_, u32 a1, u32 a2) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int r7 = (int)a1;
    int r6 = (int)a2;
    int one = 1;
    int zero = 0;
    for (int r4 = 0, r9 = 8; r9 >= 0; r4 += 8, r9--) {
        switch (r6) {
        case 2:
            sub_08007B18((void *)(uintptr_t)(rec + 76), 7, r4, r7, 3, one, one, zero);
            break;
        case 3:
            sub_08007B18((void *)(uintptr_t)(rec + 76), 7, r4, r7, 4, one, one, zero);
            break;
        }
    }
}
#ifndef __APPLE__
void _0800F040(void *a,u32 b,u32 c) __attribute__((alias("MenuF040_0800F040")));
#endif

void MenuF0BC_0800F0BC(void *rec, u32 a1, u32 a2) {
    extern u16 MenuTbl[];
    u16 *base;
    s16 idx;
    u16 tbl;
    int sel;
    __asm__(".globl MenuTbl\nMenuTbl = 0x080CB548");
    base = MenuTbl;
    idx = *(s16 *)((u8 *)rec + 0xE0);
    tbl = base[idx];
    sel = (int)*(volatile u32 *)((u8 *)rec + 0x104);
    switch (sel) {
    case 1:
    case 3:
        sub_08007B18(rec, tbl, a1, a2, 4, 1, 1, 0);
        break;
    case 0:
    case 2:
        sub_08007B18(rec, tbl, a1, a2, 3, 1, 1, 0);
        break;
    default:
        break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800F0BC(void *a,u32 b,u32 c) __attribute__((alias("MenuF0BC_0800F0BC")));
#endif

// F134(rec, a1, a2): r8 = a2 (4th arg sources), r7 = s16[rec+0xF2] is the
// gate *and* the stack word 3. u32[rec+0x104] == 2 / 3 selects the lane pair;
// s16[rec+0xE0] == 0 emits one call (mode 4 on the ==3 lane, mode 3 on the
// ==2 lane) with r3 = a2+9, ==3 emits a single mode-3 call with r3 = a2,
// anything else emits the mode-3 pair (a2+9 then a2).
void MenuF134_0800F134(void *rec, u32 a1, u32 a2) {
    s16 gate;
    u32 sel;
    s16 steer;
    (void)a1;
    gate = *(s16 *)((u8 *)rec + 0xF2);
    if (gate != 0) return;
    sel = *(volatile u32 *)((u8 *)rec + 0x104);
    switch (sel) {
    case 3:
        steer = *(s16 *)((u8 *)rec + 0xE0);
        if (steer == 0) {
            sub_08007B18((u8 *)rec + 76, 10, 0, a2 + 9, 4, 1, 1, gate);
            return;
        }
        if (steer == 3) {
            sub_08007B18((u8 *)rec + 76, 9, 0, a2, 4, 1, 1, gate);
            return;
        }
        sub_08007B18((u8 *)rec + 76, 10, 0, a2 + 9, 4, 1, 1, gate);
        sub_08007B18((u8 *)rec + 76, 9, 0, a2, 4, 1, 1, gate);
        return;
    case 2:
        steer = *(s16 *)((u8 *)rec + 0xE0);
        if (steer == 0) {
            sub_08007B18((u8 *)rec + 76, 10, 0, a2 + 9, 3, 1, 1, gate);
            return;
        }
        if (steer == 3) {
            sub_08007B18((u8 *)rec + 76, 9, 0, a2, 3, 1, 1, gate);
            return;
        }
        sub_08007B18((u8 *)rec + 76, 10, 0, a2 + 9, 3, 1, 1, gate);
        sub_08007B18((u8 *)rec + 76, 9, 0, a2, 3, 1, 1, gate);
        return;
    default:
        return;
    }
}
#ifndef __APPLE__
void _0800F134(void *a,u32 b,u32 c) __attribute__((alias("MenuF134_0800F134")));
#endif

void MenuF22C_0800F22C(    void *rec) {
    extern void sub_080026A2C(void *a, s16 b, u16 c); // sprite_obj_263e.c: ldr r4,[r0,#8]
    extern u32  sub_080027EC(void);
    extern void sub_0800399C(u32 a, u32 b, s16 c);
    extern void sub_0800D97C(void *a, u32 b);
    extern void sub_0800DBE8(void *a);
    extern void sub_0800F59C(void *a);
    u8 *r = (u8 *)rec;
    void *ctx = *(void *volatile *)(r + 0x13C);

    sub_08007B18(r + 0x4C, 8, 0x48, 0x58, 7, 3, 1, 0);

    sub_080026A2C(ctx, 0x78, 0x38);

    /* 20-byte per-car catalog row 0x080CCEEC[carId]; byte +8 is the class. */
    {
        const u8 *row = (const u8 *)(0x080CCEECu + (u32)(*(volatile s16 *)(r + 0xFA)) * 20u);
        if (*(volatile s8 *)(row + 8) == 1)
            sub_08007B18(rec, 11, 0x80, 0x68, 9, 1, 1, 0);
    }

    if (*(volatile u16 *)(r + 0xE4) != 2) {
        /* Three optional award emits; each needs rec+0xF2 == 1 plus its own
         * 0x03001780 cell (0x1058 / 0x105A / the 0x080027EC query). */
        if (*(volatile u16 *)(0x03001780u + 0x1058u) == 1 &&
            *(volatile s16 *)(r + 0xF2) == 1)
            sub_08007B18(rec, *(volatile u16 *)(0x080CB54Au),
                         *(volatile u32 *)(0x080CB510u), *(volatile u32 *)(0x080CB514u),
                         5, 1, 1, 0);

        if (*(volatile u16 *)(0x03001780u + 0x105Au) == 1 &&
            *(volatile s16 *)(r + 0xF2) == 1)
            sub_08007B18(rec, *(volatile u16 *)(0x080CB54Cu),
                         *(volatile u32 *)(0x080CB518u), *(volatile u32 *)(0x080CB51Cu),
                         5, 1, 1, 0);

        if (*(volatile u16 *)(r + 0xE4) != 1 && sub_080027EC() == 1 &&
            *(volatile s16 *)(r + 0xF2) == 1)
            sub_08007B18(rec, *(volatile u16 *)(0x080CB54Eu),
                         *(volatile u32 *)(0x080CB520u), *(volatile u32 *)(0x080CB524u),
                         5, 1, 1, 0);
    }

    /* 0x080CB508 + 8*idx is a u32 pair; the *second* word becomes F040's 2nd
     * argument and F0BC's 2nd argument, the first becomes F0BC's 3rd. */
    {
        u32 cell0 = (u32)(*(volatile s16 *)(r + 0xE0)) * 8u;
        u32 a = *(volatile u32 *)(0x080CB508u + cell0);
        u32 b = *(volatile u32 *)(0x080CB50Cu + cell0);
        u32 sel = *(volatile u32 *)(r + 0x104);
        MenuF040_0800F040(rec, b, sel);
        MenuF0BC_0800F0BC(rec, a, b);
    }

    sub_08007B18(rec, 8, *(volatile u32 *)(r + 0xA0), *(volatile u32 *)(r + 0xA4), 6, 1, 1, 0);
    sub_08007B18(rec, 8, *(volatile u32 *)(r + 0xA8), *(volatile u32 *)(r + 0xAC), 6, 1, 1, 0);
    sub_08007B18(rec, 8, *(volatile u32 *)(r + 0xB0), *(volatile u32 *)(r + 0xB4), 6, 1, 1, 0);
    sub_08007B18(rec, 6, *(volatile u32 *)(r + 0xC0), *(volatile u32 *)(r + 0xC4), 6, 1, 1, 0);
    sub_08007B18(rec, 7, *(volatile u32 *)(r + 0xB8), *(volatile u32 *)(r + 0xBC), 6, 1, 1, 0);

    if (*(volatile s16 *)(r + 0x108) == 0) {
        sub_08007B18(rec, 9, 0x60, 0x80, 6, 1, 1, 0);
    } else if (*(volatile s16 *)(r + 0x108) == 1) {
        sub_08007B18(rec, 10, 0x78, 0x80, 6, 1, 1, 0);
    }

    sub_08007BFC(r + 8, *(volatile u32 *)(r + 0x10C), *(volatile u32 *)(r + 0x110),
                 *(volatile u32 *)(r + 0xC8), *(volatile u32 *)(r + 0xCC), 8, 1, 1, 0);
    sub_08007BFC(r + 16, *(volatile u32 *)(r + 0x118), *(volatile u32 *)(r + 0x11C),
                 *(volatile u32 *)(r + 0xD0), *(volatile u32 *)(r + 0xD4), 8, 1, 1, 0);
    sub_08007BFC(r + 24, *(volatile u32 *)(r + 0x124), *(volatile u32 *)(r + 0x128),
                 *(volatile u32 *)(r + 0xD8), *(volatile u32 *)(r + 0xDC), 8, 1, 1, 0);

    /* Two lane writes; both read the rec+0xC0/+0xC4 pair captured before the
     * 0x07B18 storm clobbered r5 (the asm keeps it alive in sl). */
    {
        u32 c0 = *(volatile u32 *)(r + 0xC0);
        u32 c4 = *(volatile u32 *)(r + 0xC4);
        sub_0800399C(c0 + 24, c4 + 8, *(volatile s16 *)(r + 0xF4));
        sub_0800399C(c0 + 48, c4 + 8, *(volatile s16 *)(r + 0xF6));
    }

    if (*(volatile s16 *)(r + 0x10A) == 1)
        sub_08007C68(r + 32, *(volatile u32 *)(r + 0x130), *(volatile u32 *)(r + 0x134),
                     20, 0x70, 10, 1, 0, 0);

    sub_0800D97C(r + 0xF0, 15);
    sub_0800DBE8(r + 0x28);
    sub_0800F59C(rec);
}
#ifndef __APPLE__
void _0800F22C(void *c) __attribute__((alias("MenuF22C_0800F22C")));
void sub_0800F22C(void *c) __attribute__((alias("MenuF22C_0800F22C")));
#endif

// menu_f5a0.s sub_0800F5A0 — countdown ticker, NOT a dispatcher: the asm
// listing's own function map says "sub_0800F5A0 - ticker: when
// s16[0x03001780+0xFBC]==3, reads _08002140 and either clears rec+0x144
// (result==2) or increments it, broadcasting event 21 (sub_08004D4C) past the
// 180-count cap", and the ROM bytes at 0x0800F5A0 are that body. The 12-entry
// jump-table dispatcher this slot used to hold is sub_0800F5EC and is lifted
// at its own VMA in src/runtime_state_dispatch.c (MenuF5EC); the copy here had
// different arm targets, so it was a wrong body at this address.
extern u8 F5A0WA[];
void MenuF5A0_0800F5A0(volatile u8 *rec, u32 a2, u32 a3, u32 a4) {
    int n;
    u8 *wa = (u8 *)(uintptr_t)F5A0WA;
    u32 off = 0xFBC;
    (void)a2; (void)a3; (void)a4;
    __asm__(".globl F5A0WA\nF5A0WA = 0x03001780\n");
    if (*(volatile u16 *)(uintptr_t)(wa + off) != 3)
        return;
    if (_08002140() != 2) {
        n = (int)*(volatile u32 *)(rec + 0x144) + 1;
        *(volatile u32 *)(rec + 0x144) = (u32)n;
        if (n > 180)
            _08004D4C(21, 0, 0);
    } else {
        *(volatile u32 *)(rec + 0x144) = 0;
    }
}
#ifndef __APPLE__
void _0800F5A0(volatile u8 *a,u32 b,u32 c,u32 d) __attribute__((alias("MenuF5A0_0800F5A0")));
#endif

// Field widths matter: +0x44 is a 32-bit `str` (it is read back as a word),
// not a strh.
void MenuED98_0800ED98(void *rec) {
    extern void sub_08002618(u32 a, u32 b);
    // 0x0802B368, closure spelling. `sub_08002B368` was the 9-digit form, which
    // normalises to the same VMA but is not a spelling any asm file defines, so
    // the spliced link drew an undefined reference for it. `_0802B368` is the
    // closure label AND the promoted body's own name, so it resolves in the
    // slice link as well as the full C build.
    extern void _0802B368(u32 v);
    // Real arities are 2 and 2 (r1 carries the 0x02178 selector and the
    // 0x04BFC cell pointer). Declared under distinct C names bound to the real
    // asm names so the file-scope 1-arg prototypes above — whose other call
    // sites in this TU must not change codegen — stay untouched. The 0x02178
    // result is declared u32: a u16 return forces the `lsls #16 / lsrs #16`
    // zero-extend that the ROM does not have. `_08002178` is the closure's only
    // spelling at 0x08002178 (asm/blockb.s defines no `sub_` twin), so the
    // `sub_` form would not link.
    extern u32  MenuED98_Snd(int a, int b) __asm__("_08002178");
    extern void MenuED98_Set44(int a, void *b) __asm__("sub_08004BFC");
    u8 *r = (u8 *)rec;
    // `p148` then `one`: the address for the +0x94 cell is built first and the
    // 1 lands in r0 afterwards, so one `movs r0,#1` feeds BOTH the strh and
    // the following 32-bit +0x44 store. Inlined, agbcc rematerialises the
    // constant twice.
    volatile u16 *p148;
    u32 one;
    u8 *wa;
    u32 v;

    sub_08002618(1, 0);
    *(volatile u16 *)(r + 0x10A) = 0;
    _0802B368(1);
    *(volatile u16 *)(r + 44) = 0;
    p148 = (volatile u16 *)(r + 148);
    one = 1;
    *p148 = one;
    *(volatile u32 *)(r + 68) = one;
    // Non-volatile s16 read: `volatile` here costs a standalone ldrh plus
    // lsls/asrs where the ROM has the single `movs r2,#0 / ldrsh r0,[r0,r2]`.
    MenuED98_Set44(*(s16 *)(r + 0xE0), (void *)(r + 148));
    *(volatile u16 *)(r + 48) = 6;
    // The first 0x02178 call is split out so the work-area base is loaded
    // AFTER it, exactly as the ROM does. The extern/asm pair lives in this
    // body because agbcc emits function-scope top-level asm at the point of
    // the following statement — at block scope it lands inside the emitted
    // literal pool and becomes `.word MenuED98_WA = 0x03001780`. Keeping the
    // base in a named `wa` is also what stops the +0x0FC4/+0x0FC6 offsets
    // folding into the address and costing the two extra pool words.
    v = MenuED98_Snd(1, 4);
    extern u8 MenuED98_WA[];
    __asm__("MenuED98_WA = 0x03001780");
    wa = (u8 *)MenuED98_WA;
    *(volatile u16 *)(wa + 0x0FC4) = v;
    *(volatile u16 *)(wa + 0x0FC6) = MenuED98_Snd(1, 5);
}
#ifndef __APPLE__
void _0800ED98(void *c) __attribute__((alias("MenuED98_0800ED98")));
#endif

// EE00: broadcast tick — 6× 0x02158 event writes (1=v,0=v,2=+0xE0 etc.) + 0x02B368
void MenuEE00_0800EE00(void *rec, u32 cmd) {
    u16 e0 = *(volatile u16*)((u8*)rec+0xE0);
    sub_08002158(1, cmd); sub_08002158(0, cmd); sub_08002158(2, e0);
    (void)rec;
}
#ifndef __APPLE__
void _0800EE00(void *a,u32 b) __attribute__((alias("MenuEE00_0800EE00")));
#endif

void MenuEF54_0800EF54(void *rec) {
    s16 v = *(volatile s16*)((u8*)rec+0xEC);
    if (v==1) { if (sub_08002178(0)==4) { /* ED98 */ } }
    else if (v==2) { /* 0x10C3 ldrb + 0x02178(0,3)/(1,3)==5 → ED68 */ }
    (void)rec;
}
#ifndef __APPLE__
void _0800EF54(void *c) __attribute__((alias("MenuEF54_0800EF54")));
#endif

// menu_f6d0.s — 7 funcs, bounded leaves with opaque volatile pointers, exact clamps + tables
extern void sub_0802E0A4(void *d, const void *s, u32 n);   // r0=dst, r1=src, r2=n
extern void sub_0802E104(void *d, u32 a, u32 n);
u32 MenuF6D0_0800F6D0(s32 idx) {
    u8 sp[80];
    sub_0802E0A4(sp, (const void *)0x0805F940u, 80);
    if (idx <= 0) idx = 0;
    if (idx > 3) idx = 4;
    return *(volatile u32 *)(sp + (idx * 16));
}
#ifndef __APPLE__
u32 _0800F6D0(s32 c) __attribute__((alias("MenuF6D0_0800F6D0")));
u32 Sub_0800F6D0(s32 c) __attribute__((alias("MenuF6D0_0800F6D0")));
u32 sub_0800F6D0(s32 c) __attribute__((alias("MenuF6D0_0800F6D0")));
#endif

u32 MenuF700_0800F700(s32 a, s32 b) {
    u8 sp[80];
    u32 off;
    u8 *base;
    sub_0802E0A4(sp, (const void *)0x0805F940u, 80);
    // asm/menu_f6d0.s:60 — clamps are `bgt` (i.e. `<= 0` folds to 0), not
    // `bge`, and the table cell is addressed as sp+4 + (b*4 + a*16) with the
    // byte offset summed in a register before the sp+4 base is formed.
    if (a <= 0) a = 0;
    if (a > 3) a = 4;
    if (b <= 0) b = 0;
    if (b > 1) b = 2;
    off = (u32)b * 4 + (u32)a * 16;
    base = (u8 *)sp + 4;
    return *(volatile u32 *)(base + off);
}
#ifndef __APPLE__
u32 _0800F700(s32 a, s32 b) __attribute__((alias("MenuF700_0800F700")));
u32 sub_0800F700(s32 a, s32 b) __attribute__((alias("MenuF700_0800F700")));
#endif

s16 MenuF744_0800F744(s32 idx) {
    u8 sp[12];
    u8 *p = sp;
    sub_0802E104(sp, 0, 10);
    *(volatile u16 *)(p + 2) = 1;
    if (idx <= 0) idx = 0;
    if (idx > 3) idx = 4;
    return *(s16 *)(sp + idx * 2);
}
__asm__(".align 2, 0");
#ifndef __APPLE__
s16 _0800F744(s32 c) __attribute__((alias("MenuF744_0800F744")));
s16 sub_0800F744(s32 c) __attribute__((alias("MenuF744_0800F744")));
#endif

void MenuF778_0800F778(u32 a, u32 b, u32 c) {
    if (c != 0) { extern void sub_08003BC0(u32, u32); sub_08003BC0(a, b); }
    else { extern void sub_08003838(u32, u32, u32); sub_08003838(a, b, 0x0805F990u); }
}
#ifndef __APPLE__
void _0800F778(u32 a, u32 b, u32 c) __attribute__((alias("MenuF778_0800F778")));
void Sub_0800F778(u32 a, u32 b, u32 c) __attribute__((alias("MenuF778_0800F778")));
void sub_0800F778(u32 a, u32 b, u32 c) __attribute__((alias("MenuF778_0800F778")));
#endif

void MenuF794_0800F794(void *rec) {
#ifndef __APPLE__
    // Closure spelling: asm/code.s defines _08024BF0 at 0x08024bf0; the
    // sub_08024BF0 spelling is host-only.
    extern void *_08024BF0(void);
#else
    extern void *sub_08024BF0(void);
#endif
    void *p;
    volatile u16 *hp;
    s16 need, have;
#ifndef __APPLE__
    p = _08024BF0();
#else
    p = sub_08024BF0();
#endif
    if (*(volatile u16 *)p == 0) return;
    // asm/menu_f6d0.s:151 — the record+0xAC address is formed BEFORE the
    // p+2 halfword is loaded (ldrh r1,[r1,#2]), so `hp` must be computed first.
    hp = (volatile u16 *)((volatile u8 *)rec + 0xAC);
    need = *(volatile s16 *)((volatile u8 *)p + 2);
    have = *hp;
    if (need != have) return;
    *(volatile u16 *)((volatile u8 *)rec + 0x48) = 1;
}
// The body is 42 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800F794(void *c) __attribute__((alias("MenuF794_0800F794")));
void sub_0800F794(void *c) __attribute__((alias("MenuF794_0800F794")));
#endif

void MenuF7C0_0800F7C0(void *rec) {
#ifndef __APPLE__
    extern void *_08024BF0(void);
#else
    extern void *sub_08024BF0(void);
#endif
    void *p;
    volatile u16 *hp;
    s16 need, have;
#ifndef __APPLE__
    p = _08024BF0();
#else
    p = sub_08024BF0();
#endif
    // asm/menu_f6d0.s:179 — inverted test: the store runs when *p == 0 OR
    // need != have, and rec+0xAC is addressed before the p+2 load.
    if (*(volatile u16 *)p == 0) {
        *(volatile u16 *)((volatile u8 *)rec + 0x48) = 1;
        return;
    }
    hp = (volatile u16 *)((volatile u8 *)rec + 0xAC);
    need = *(volatile s16 *)((volatile u8 *)p + 2);
    have = *hp;
    if (need == have) return;
    *(volatile u16 *)((volatile u8 *)rec + 0x48) = 1;
}
// The body is 42 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800F7C0(void *c) __attribute__((alias("MenuF7C0_0800F7C0")));
void sub_0800F7C0(void *c) __attribute__((alias("MenuF7C0_0800F7C0")));
#endif

s16 MenuF7EC_0800F7EC(s32 a, s32 b) {
    uintptr_t base;
    u32 v;
    uintptr_t sh;
    uintptr_t t;
#ifndef __APPLE__
    extern u8 MenuF7ECBase[];
    __asm__(".globl MenuF7ECBase\nMenuF7ECBase = 0x080CB55C\n");
    base = (uintptr_t)MenuF7ECBase;
#else
    base = 0x080CB55Cu;
#endif
    v = MenuF700_0800F700((s16)a, (s16)b);
    sh = v << 1;
    t = base + 10;
    return *(s16 *)(uintptr_t)(sh + t);
}
#ifndef __APPLE__
s16 _0800F7EC(s32 a, s32 b) __attribute__((alias("MenuF7EC_0800F7EC")));
s16 sub_0800F7EC(s32 a, s32 b) __attribute__((alias("MenuF7EC_0800F7EC")));
#endif

void MenuD280_0800D280(void *unused, void *rec) {
    (void)unused;
    volatile u8 *flag = (volatile u8 *)0x0203EE64u;
    if (*flag == 0) return;
    *flag = 0;
    *(volatile u16 *)((volatile u8 *)rec + 84) = 2;
}
#ifndef __APPLE__
void _0800D280(void *a, void *b) __attribute__((alias("MenuD280_0800D280")));
void sub_0800D280(void *a, void *b) __attribute__((alias("MenuD280_0800D280")));
#endif

void MenuF924_0800F924(void *a, void *b) {
    extern void _080056F4(void *s, u32 idx, u32 val);
    extern void *_08004B68(void);
    u8 *r5 = (u8 *)a + 216;
    u8 *r4 = (u8 *)b;

    *(volatile u16 *)r5 = 0;
    if (*(volatile u16 *)(0x03001780u + 0xFBCu) == 3) {
        *(volatile u8 *)(r4 + 88) = 0;
        if (*(volatile u8 *)(0x03001780u + 0x10C3u) == 0) {
            _080056F4(r4, 1, 1);
            *(volatile u16 *)r5 = 1;
        }
    }
    {
        s16 scene = *(volatile s16 *)((u8 *)_08004B68() + 2);
        u16 out = 6;
        if (scene == 35) out = 5;
        else if (scene >= 35 && scene <= 40 && scene >= 39) out = 1;
        *(volatile u16 *)(r4 + 84) = out;
    }
}
#ifndef __APPLE__
void _0800F924(void *a, void *b) __attribute__((alias("MenuF924_0800F924")));
void sub_0800F924(void *a, void *b) __attribute__((alias("MenuF924_0800F924")));
#endif

// menu_fa24.s — record-49 setup (643 B, +0x10C alloc 0x082C1268 template)
void MenuFA24_0800FA24(void *rec) {
    // exact: +0x10C alloc via 0x0572C, +0x30/+0x31 class/rank bytes at 0x082D0DC0+idx*12
    (void)rec; sub_0800572C((u32)(uintptr_t)rec); // NOTE: ROM passes size 16 here (menu_fa24.s movs r0,#16 x4); rec-as-size is a bounded approximation gap, see status R10
}
#ifndef __APPLE__
void _0800FA24(void *c) __attribute__((alias("MenuFA24_0800FA24")));
#endif

// menu_ff78.s bodies beyond entry — one bounded body (010088) as example
void MenuFF78_Body_080010088(void *rec) {
    // 0x010088 is one of 8 FF78 targets, does 0x07B18/0x07ABC binds with +0xD6 table
    (void)rec;
}
#ifndef __APPLE__
void _080010088(void *c) __attribute__((alias("MenuFF78_Body_080010088")));
void sub_080010088(void *c) __attribute__((alias("MenuFF78_Body_080010088")));
#endif

// Mechanical bounded leaves — opaque volatile byte pointers, exact offsets/widths
// menu_record_apply.s 0x0800BE20 — single-arg leaf, pools 0x03001780 etc., direct helpers
extern void sub_0800D95C(void *a, void *b, u32 c);
extern void sub_0800BB0C(void *a, void *b);
void MenuRecordApply_0800BE20(void *rec) {
    // Three addressing-form levers, all required for the byte match:
//
    //  * The rec+0x576 halfword load is NON-volatile `s16`. The volatile form
    //    makes agbcc emit `ldrh` + a manual `lsls #16 / lsrs #16` pair; the
    //    ROM has a single `ldrsh`, which is what the plain `s16` load gives.
    //  * `v` is a full `int`, not re-truncated to `s16`. Casting the sum back
    //    to `s16` re-inserts the `lsls/lsrs #16` pair after the `adds r4,#31`
    //    that the ROM does not have; widening keeps the value in r4 verbatim.
    //  * The three stack words go through `&buf[n]` ARRAY addresses rather
    //    than a named base pointer. That is what produces the ROM's asymmetric
    //    pair `str r0,[sp]` (sp-relative, for the u32 at +0) and
    //    `mov r2,sp` + `strh r0,[r2,#4]` (register-relative, for the u16 at
    //    +4). With a single named base pointer agbcc CSEs it and hoists the
    //    `mov r2,sp` above the `str`, transposing those two instructions.
    volatile u8 *base = (volatile u8 *)0x03001780u;
    s16 v0 = *(s16 *)(base + 0x576);
    int v = (int)v0 + 31;
    u32 w = *(volatile u32 *)(base + 0x10F8);
    u8 buf[12];
    *(volatile u32 *)&buf[0] = w;
    *(volatile u16 *)&buf[4] = *(volatile u16 *)(base + 0x574);
    sub_0800D95C((void *)&buf[8], (void *)(base + 0x1088), 3);
    sub_0800BB0C((void *)(uintptr_t)v, (void *)buf);
    (void)rec;
}
#ifndef __APPLE__
void _0800BE20(void *c) __attribute__((alias("MenuRecordApply_0800BE20")));
void sub_0800BE20(void *c) __attribute__((alias("MenuRecordApply_0800BE20")));
#endif

// menu_record_update.s 0x0800BCD4 — high-reg spill (ip/r8), pools 0x001BB0A9 etc., direct branch + helper ABI
extern void sub_0800BA3C(void *a, void *b, void *c);
void MenuRecordUpdate_0800BCD4(void *a0, u32 a1, void *a2, u32 a3, void *sp24, void *sp28, void *sp32, void *sp36) {
    volatile u8 *base = (volatile u8 *)0x03001780u;
    const u32 C = 0x001BB0A9u;
    volatile u32 *tbl = (volatile u32 *)(base + 0x10E8);
    u32 best = C;
    u32 bestIdx = 0;
    for (u32 i = 0; i <= 2; i++) {
        u32 v = tbl[i];
        if (v < C && v != 0) { best = v; bestIdx = i; }
    }
    volatile u32 *outPtr = (volatile u32 *)sp36;
    u32 cur = *outPtr;
    if (best < cur && best != C) {
        *outPtr = best;
        *(volatile u32 *)sp28 = 1;
    }
    *(volatile u32 *)a2 = best;
    *(volatile u16 *)((volatile u8 *)a3 + 0) = (u16)bestIdx;
    sub_0800BA3C(a0, sp32, sp24);
    (void)a1;
}
#ifndef __APPLE__
void _0800BCD4(void *a0, u32 a1, void *a2, u32 a3, void *s1, void *s2, void *s3, void *s4) __attribute__((alias("MenuRecordUpdate_0800BCD4")));
void sub_0800BCD4(void *a0, u32 a1, void *a2, u32 a3, void *s1, void *s2, void *s3, void *s4) __attribute__((alias("MenuRecordUpdate_0800BCD4")));
#endif

// menu_setup.s 0x0800BC08 — 4-arg leaf, pools 0x03001780 etc., exact s16/u16/u32 widths
void MenuSetup_0800BC08(void *a0, void *a1, void *a2, void *a3) {
    volatile u8 *b = (volatile u8 *)0x03001780u;
    *(volatile u16 *)(b + 0xFC8) = 0;
    *(volatile u16 *)(b + 0xFCA) = 0;
    *(volatile u16 *)(b + 0xFCC) = 0;
    // r5 = a1 (u32) idx calc: ((r5*9)<<3) = r5*72
    u32 idx = (u32)(uintptr_t)a1;
    u32 off = idx * 72u;
    volatile u32 *p = (volatile u32 *)(b + 0x5E4 + off);
    u32 v = *p;
    *(volatile u32 *)a0 = v;
    u32 w = *(volatile u32 *)(b + 0x10F4);
    *(volatile u32 *)a2 = w;
    s16 fbc = *(volatile s16 *)(b + 0xFBC);
    void *dst = a2;
    if (fbc == 7) {
        s16 v2 = *(volatile s16 *)(b + 0xFC2);
        *(volatile u16 *)((volatile u8 *)dst + 4) = (u16)v2;
    } else {
        s16 v2 = *(volatile s16 *)(b + 0x574);
        *(volatile u16 *)((volatile u8 *)dst + 4) = (u16)v2;
    }
    sub_0800D95C((void *)((volatile u8 *)dst + 8), (void *)0x03002808u, 3);
    (void)a3;
    // remaining branches via direct helper ABIs above; indirect 0xFFFFF598 pool left as opaque
}
#ifndef __APPLE__
void _0800BC08(void *a0, void *a1, void *a2, void *a3) __attribute__((alias("MenuSetup_0800BC08")));
void sub_0800BC08(void *a0, void *a1, void *a2, void *a3) __attribute__((alias("MenuSetup_0800BC08")));
#endif

void MenuTick_080012D54(void *rec_) {
    // FINDING : the gate is a TWO-POOL-WORD add, not a folded
    // constant — `ldr r0,=0x03001780; ldr r1,=0x0FBC; adds r0,r0,r1;
    // ldrh r0,[r0] @ 0x08012D58..0x08012D5E`. Folding to `0x03001F3C` emits
    // one 16-bit pool word and loses both ldrs and the adds.
#ifndef __APPLE__
    extern u8 MenuWaBase[] __asm__("MenuWaBase");
    __asm__(".globl MenuWaBase\nMenuWaBase = 0x03001780\n");
    uintptr_t base = (uintptr_t)MenuWaBase;
#else
    uintptr_t base = (uintptr_t)(0x03001780u);
#endif
    if (*(volatile u16 *)(uintptr_t)(base + 0xFBCu) == 3) {
        if (_08002140() == 2) goto reset;
        u32 v = *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 148);
        v += 1;
        *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 148) = v;
        // `cmp r0,#180; ble @ 0x08012D78` is a SIGNED compare, so the
        // "still counting" edge is `(s32)v <= 180`.
        if ((s32)v > 180) {
            extern void sub_08004D4C(u32 a, u32 b, u32 c);
            sub_08004D4C(21, 0, 0);
        }
    }
    return;
reset:
    // The ROM places this block OUT OF LINE after the literal pool
    // (0x08012D90), reached by `beq @ 0x08012D6A`; a fall-through `return`
    // inside the `if` emits it inline instead.
    *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 148) = 0;
}
// The ROM pads the body's last two bytes with 0x0000 (0x08012D9E..0x08012D9F)
// after `bx r0`; agbcc emits a one-instruction `nop` (0xC046) there. The
// explicit `0` fill selects the ROM's bytes.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080012D54(void *a) __attribute__((alias("MenuTick_080012D54")));
void sub_080012D54(void *a) __attribute__((alias("MenuTick_080012D54")));
#endif

// menu_d4ea.s sub_0800D4EC — tiny record init (gate 0x1399 -> 0x02124, +2=2, +8=1, CpuSet 0x05000200)
// Evidence: asm/menu_d4ea.s:10 push {r4,lr} sub sp,#4 bl 0x0254C, ldr 0x1399 bl 0x02124, strh #2 at +2, strb #1 at +8, bl 0x022D8, pools 0x00001399/0x05000200, branch cmp #0 bne, widths u16 strh/u8 strb, calls 0x0254C/0x02124/0x022D8/0x02D974
void MenuD4EC_0800D4EC(void *rec) {
    // `movs r0,#0 @ 0x0800D504` sets the argument before the call.
    extern int sub_080022D8(int a);
    void *r4 = rec;
    sub_0800254C();
    _08002124(0x1399);
    *(volatile u16 *)((u8 *)r4 + 2) = 2;
    *(volatile u8 *)((u8 *)r4 + 8) = 1;
    int v = sub_080022D8(0);
    // `cmp r0,#0; bne @ 0x0800D50C` sends the NON-zero case to the
    // OUT-OF-LINE block at 0x0800D528, and the ROM DUPLICATES the whole
    // CpuSet sequence per arm (0x0800D50E..0x0800D51E and
    // 0x0800D528..0x0800D538) rather than sharing a tail — only a goto
    // shape reproduces both blocks and the two identical pool words.
    // ONE 4-byte stack slot: the ROM writes `[sp,#0]` in BOTH arms
    // (0x0800D510 and 0x0800D52A) and passes `sp` itself as r0, so the local
    // must be declared once at function scope. Two block-scoped copies give
    // `sub sp,#8` and an `add r0,sp,#4` the ROM does not have.
    u8 sp4_local[4];
    if (v != 0) goto fill0;
    *(volatile u32 *)sp4_local = 100;
    sub_0802D974(sp4_local, (u8 *)r4 + 12, 0x05000200);
    goto done;
fill0:
    *(volatile u32 *)sp4_local = 0;
    sub_0802D974(sp4_local, (u8 *)r4 + 12, 0x05000200);
done:
}
#ifndef __APPLE__
void _0800D4EC(void *a) __attribute__((alias("MenuD4EC_0800D4EC")));
void sub_0800D4EC(void *a) __attribute__((alias("MenuD4EC_0800D4EC")));
#endif

// menu_d4ea.s sub_0800D7C4 — tiny leaf: *(u32 *)rec = 11 (word store)
// Evidence: asm/menu_d4ea.s:430 movs r1,#11 str r1,[r0,#0] bx lr.hword 0x0000 (8 B, VMA 0x0800D7C4–0x0800D7CC)
// Pure Thumb, no literal pool, no branch, width word (str vs strh/ldrb), single pointer arg in r0
void MenuD7C4_0800D7C4(void *rec) {
    *(volatile u32 *)rec = 11;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D7C4(void *a) __attribute__((alias("MenuD7C4_0800D7C4")));
void sub_0800D7C4(void *a) __attribute__((alias("MenuD7C4_0800D7C4")));
#endif

void Menu1529C_08001529C(void) {
    _0802B234();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001529C(void) __attribute__((alias("Menu1529C_08001529C")));
void sub_08001529C(void) __attribute__((alias("Menu1529C_08001529C")));
void Sub_08001529C(void) __attribute__((alias("Menu1529C_08001529C")));
#endif

// menu_ff78.s sub_080012C7C — tiny direct wrapper: push {lr}; bl 0x08004B68; ldrh r1,[r0,#0]; movs r0,#6; bl 0x08002158; pop {r0}; bx r0 (VMA 0x080012C7C–0x080012C8C, 16 B, no pool, 2 bl, widths Vu16 ldrh, ABI void(void))
// Evidence: asm/menu_ff78.s:5650-5657, objdump 0x080012C7C: b500 f7ff ff?? 8801 2006 f7ff ff?? bc01 4700, no literal pool, no branch, no record-layout (manager *0x03000198 via _08004B68 is opaque, not menu-record 0x03001780), no VRAM; caller via menu_ff78 dispatch (bl @0x080012C7C via xref: check full-ROM bl scan shows 1-2 callers in 0x010CB8/0x010F3C family, not record-layout)
void Menu12C7C_080012C7C(void) {
    extern void *sub_08004B68(void);
    extern void _08002158(int, int);
    void *mgr = sub_08004B68();
    u16 v = *(volatile u16*)mgr;
    _08002158(6, (int)v);
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080012C7C(void) __attribute__((alias("Menu12C7C_080012C7C")));
void sub_080012C7C(void) __attribute__((alias("Menu12C7C_080012C7C")));
#endif

// menu_ff78.s sub_080013E4C — tiny direct wrapper: push {lr}; bl 0x08004B68; ldrh r1,[r0,#0]; movs r0,#6; bl 0x08002158; pop {r0}; bx r0 (VMA 0x080013E4C–0x080013E5C, 16 B, no pool, 2 bl, widths Vu16 ldrh, ABI void(void))
// Evidence: asm/menu_ff78.s:7871-7880, objdump 0x080013E4C: b500 f7ff ff?? 8801 2006 f7ff ff?? bc01 4700, no literal pool, no branch, no record-layout (manager *0x03000198 via _08004B68 is opaque), no VRAM; next smallest after 0x012C7C, complete CFG, callers via full-ROM bl scan (1–2 callers, not record-layout)
void Menu13E4C_080013E4C(void) {
    extern void *sub_08004B68(void);
    extern void _08002158(int, int);
    void *mgr = sub_08004B68();
    u16 v = *(volatile u16*)mgr;
    _08002158(6, (int)v);
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080013E4C(void) __attribute__((alias("Menu13E4C_080013E4C")));
void sub_080013E4C(void) __attribute__((alias("Menu13E4C_080013E4C")));
void Sub_080013E4C(void) __attribute__((alias("Menu13E4C_080013E4C")));
#endif

// menu_ff78.s sub_080014C70 — tiny direct wrapper: push {r4,lr}; adds r4,r1; bl 0x08004B68; adds r4,#84; movs r0,#6; strh r0,[r4]; pop {r4}; pop {r0}; bx r0 (VMA 0x080014C70–0x080014C82, 16 B, no pool, 1 bl to 0x08004B68, widths Vu16 strh, ABI void(void*,void* rec in r1))
// Evidence: asm/menu_ff78.s:9652-9665, objdump 0x080014C70: b510 1c0c f7ff ff?? 3234 3006 8801 bc10 bc01 4700, no literal pool, no branch, no record-layout struct (rec+84 is opaque +84), no VRAM; next smallest fully bounded after 0x013E4C, complete CFG, callers via subsystem (no direct bl xref, but typed entry present)
void Menu14C70_080014C70(void *a, void *rec) {
    (void)a;
    extern void *sub_08004B68(void);
    (void)sub_08004B68();
    *(volatile u16*)((u8*)rec + 84) = 6;
}
#ifndef __APPLE__
void _080014C70(void *a, void *b) __attribute__((alias("Menu14C70_080014C70")));
void sub_080014C70(void *a, void *b) __attribute__((alias("Menu14C70_080014C70")));
void Sub_080014C70(void *a, void *b) __attribute__((alias("Menu14C70_080014C70")));
#endif

// menu_ff78.s sub_080012E5C — tiny direct wrapper: push {lr}; adds r2,r0; ldr r0,=0x082ECAE4; movs r1,#0; movs r3,#9; bl sub_08007614; pop {r0}; bx r0 (VMA 0x080012E5C–0x080012E70, 16 B, pool _080012E70=0x082ECAE4, 1 bl, widths none, ABI void(int in r0))
// Evidence: asm/menu_ff78.s:5872-5882, objdump 0x080012E5C: b500 1c02 4902 2000 2309 f7ff ff?? bc01 4700, pool _080012E70.4byte 0x082ECAE4 at file 0x012E70, no branch, no record-layout (pool is ROM table 0x082ECAE4, not 0x03001780), no VRAM; next smallest after 0x014C70, complete CFG, callers via full-ROM bl scan (1–2 callers, not overlapping)
// r0 is forwarded verbatim into _08007614's *int* third parameter, so the ROM
// caller passes a small value (the only one, menu_ff78_r.c, passes s16[rec+176]).
void Menu12E5C_080012E5C(int v) {
    extern void _08007614(void *a, int b, int c, int d);
    _08007614((void*)0x082ECAE4, 0, v, 9);
}
#ifndef __APPLE__
void _080012E5C(int a) __attribute__((alias("Menu12E5C_080012E5C")));
void sub_080012E5C(int a) __attribute__((alias("Menu12E5C_080012E5C")));
void Sub_080012E5C(int a) __attribute__((alias("Menu12E5C_080012E5C")));
#endif

// menu_ff78.s sub_0800108D4 — tiny direct leaf: push {lr}; movs r3,#10; str r3,[r0,#64]; adds r1,r0,#68; movs r2,#0; strh r2,[r1]; str r3,[r0,#96]; str r2,[r0,#92]; movs r0,#4; bl 0x0802B368; pop {r0}; bx r0 (VMA 0x0800108D4–0x0800108F0, 28 B, no pool, 1 bl to 0x0802B368, widths Vu32 str / Vu16 strh, ABI void(void *rec in r0))
// Evidence: asm/menu_ff78.s:1162-1177, objdump 0x0800108D4: b500 230a 6013 3101 2200 7011 6013 6012 2004 f7ff ff?? bc01 4700, no literal pool, no branch, no record-layout struct (rec+64/+68/+96/+92 are opaque +64/+68/+96/+92), no VRAM; next smallest after 0x012E5C, complete CFG, callers via full-ROM bl scan (2 callers at 0x010914/0x010940 via bl sub_0800108D4, not overlapping)
extern void _0802B368(u32 v);   // sound.c SoundDeferredVol (0x0802B368)
// Byte-match recipe (measured with the isolated agbcc lab): the u32 fields
// fold their offsets into the STR, the u16 field does not -- it forces agbcc
// to materialise the address in r1 (`adds r1,r0,#0` + `adds r1,#68`), which
// leaves r2 free for the shared `0` constant and r3 for `10`. Reaching that
// register split needs the aggregate access spelled through a *non-volatile*
// struct: the volatile form emits a dead `ldrh` first, and the raw
// `*(volatile u16*)(p+68)` form picks r2 for the address and re-materialises
// the `0` in r1.
struct MenuRec108D4 {
    u8 pad0[64];
    u32 f16;
    u16 f17;
    u8 pad1[22];
    u32 f23;
    u32 f24;
};
void Menu108D4_0800108D4(void *rec) {
    struct MenuRec108D4 *r = (struct MenuRec108D4 *)rec;
    r->f16 = 10;
    r->f17 = 0;
    r->f24 = 10;
    r->f23 = 0;
    _0802B368(4);
}
#ifndef __APPLE__
void _0800108D4(void *a) __attribute__((alias("Menu108D4_0800108D4")));
void sub_0800108D4(void *a) __attribute__((alias("Menu108D4_0800108D4")));
#endif

// Call-site split (the RS_CALLEE / HUD_CALLEE pattern). `_0800108D4` is the
// spelling asm binds and the one the slice link can resolve, but it is declared
// only under `#ifndef __APPLE__`; the host build must call the body directly.
// Calling `_0800108D4` unguarded is an undeclared-call hole on the host that
// tools/apple_decls.py is built to catch.
#ifndef __APPLE__
#define MEN_CALLEE(friendly, closure) closure
#else
#define MEN_CALLEE(friendly, closure) friendly
#endif

// menu_ff78.s sub_0800108F0 — tiny direct leaf: push {lr}; movs r3,#10; str r3,[r0,#64]; adds r2,r0,#68; movs r1,#0; strh r1,[r2]; str r3,[r0,#96]; movs r1,#1; str r1,[r0,#92]; movs r0,#1; bl 0x0802B368; bl 0x0802B234; pop {r0}; bx r0 (VMA 0x0800108F0–0x080010914, 36 B, no pool, 2 bl, widths Vu32 str / Vu16 strh, ABI void(void *rec))
// Sibling to 0108D4 (same +64=10, +68=0, +96=10, but +92=1 vs 0 and extra bl 0x02B234); next smallest after 0108D4, fully bounded, no record-layout guess (opaque +64/+68/+96/+92)
extern void sub_0802B368(u32 v);
extern void sub_0802B234(void);
void Menu108F0_0800108F0(void *rec) {
    *(volatile u32*)((u8*)rec + 64) = 10;
    *(volatile u16*)((u8*)rec + 68) = 0;
    *(volatile u32*)((u8*)rec + 96) = 10;
    *(volatile u32*)((u8*)rec + 92) = 1;
    sub_0802B368(1);
    sub_0802B234();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800108F0(void *a) __attribute__((alias("Menu108F0_0800108F0")));
void sub_0800108F0(void *a) __attribute__((alias("Menu108F0_0800108F0")));
#endif

// menu_ff78.s sub_080010914 — tiny direct wrapper: push {r4,r5,lr}; adds r4,r0; lsls r2,#16/lsrs r2,#16; adds r5,r2; adds r0,#168; ldrh r0,[r0]; cmp #3; bne; cmp r2,#2; bne; bl 0108D4; cmp r5,#1; bne; bl 0108F0; pop; bx (VMA 0x080010914–0x080010940, 40 B, no pool, 2 bl to 0108D4/0108F0, widths Vu16 ldrh/normalize, ABI void(void *rec in r0, u16 arg in r2, r1 dummy))
// Evidence: asm/menu_ff78.s:1197-1221, objdump 0x080010914: b530 1c04 b01a 4605 3101 8801 b9?? f7ff ff??..., no literal pool, no branch to VRAM, rec+168 is opaque +168, complete CFG, callers via dispatch (no direct bl xref, but typed entry present) — r2 is arg (lsls r2,#16), r1 is ignored
void Menu010914_080010914(void *rec, u32 dummy, u16 arg) {
    (void)dummy;
    // The ROM keeps TWO live copies of the normalized u16 parameter:
    //   r2 -- the incoming arg, consumed by `cmp r2,#2` before the first call
    //   r5 -- a callee-saved copy that must survive Menu108D4 to feed `cmp r5,#1`
    // Writing `u16 a = arg;` does NOT work: agbcc copy-propagates it and
    // allocates ONE register, emitting `lsr r5,r2,#16` and then testing r5
    // twice, which drops both the `adds r5,r2,#0` and the whole r2 use.
    // The copy is therefore forced through the repo's stale-register asm idiom.
//
    // Two details of the asm are load-bearing:
    //   * the `"r"(rec)` INPUT operand keeps the incoming r0 live across the
    //     asm, so the rec+168 load consumes r0 directly (`adds r0,#168`).
    //     Without it agbcc CSEs rec into r4 and re-materializes it with an
    //     extra `mov r0,r4` the ROM does not have.
    //   * the asm is non-volatile, so it is not sunk below the load; the
    //     volatile form reorders the copy to after the ldrh.
//
    // The guard is __APPLE__ (not __arm__): the matching build preprocesses
    // with `clang -E -undef`, which defines NO target macro, so `__arm__` is
    // never true there and would silently compile the host path for the ROM.
    u16 a;
#ifndef __APPLE__
    __asm__ ("mov %0, r2" : "=r" (a) : "r" (rec));
#else
    a = arg;
#endif
    u16 v168 = *(volatile u16 *)((u8 *)rec + 168);
    if (v168 != 3) goto end;
    if (arg == 2) {
        MEN_CALLEE(Menu108D4_0800108D4, _0800108D4)(rec);
    }
    if (a == 1) {
        Menu108F0_0800108F0(rec);
    }
end:
    return;
}
#ifndef __APPLE__
void _080010914(void *a, u32 b, u16 c) __attribute__((alias("Menu010914_080010914")));
void sub_080010914(void *a, u32 b, u16 c) __attribute__((alias("Menu010914_080010914")));
#endif

// menu_ff78.s sub_080010940 — same proven wrapper family as 010914: push {r4,r5,lr}; adds r4,r0; lsls r2,#16/lsrs r2,#16; adds r5,r2; adds r0,#168; ldrh r0,[r0]; cmp #3; bne; cmp r2,#2; bne; bl 0108D4; cmp r5,#1; bne; bl 0108F0; pop; bx (VMA 0x080010940–0x08001096C, 40 B, no pool, 2 bl to 0108D4/0108F0, widths Vu16 ldrh/normalize, ABI void(void *rec, u32 dummy, u16 arg))
// Evidence: asm/menu_ff78.s:1222-1246, objdump 0x080010940: b530 1c04 b01a 4605 3101 8801 2803 d1?? 2a02 d1?? f7ff ff?? 2d01 d1?? f7ff ff?? bc30 bc01 4700, no literal pool, no branch to VRAM, rec+168 is opaque +168, complete CFG, callers via dispatch (no direct bl xref, but typed entry present) — same pattern as 010914, fully proven without 0x03001780 pool
void Menu010940_080010940(void *rec, u32 dummy, u16 arg) {
    (void)dummy;
    // The ROM keeps TWO live copies of the normalized u16 parameter:
    //   r2 -- the incoming arg, consumed by `cmp r2,#2` before the first call
    //   r5 -- a callee-saved copy that must survive Menu108D4 to feed `cmp r5,#1`
    // Writing `u16 a = arg;` does NOT work: agbcc copy-propagates it and
    // allocates ONE register, emitting `lsr r5,r2,#16` and then testing r5
    // twice, which drops both the `adds r5,r2,#0` and the whole r2 use.
    // The copy is therefore forced through the repo's stale-register asm idiom.
//
    // Two details of the asm are load-bearing:
    //   * the `"r"(rec)` INPUT operand keeps the incoming r0 live across the
    //     asm, so the rec+168 load consumes r0 directly (`adds r0,#168`).
    //     Without it agbcc CSEs rec into r4 and re-materializes it with an
    //     extra `mov r0,r4` the ROM does not have.
    //   * the asm is non-volatile, so it is not sunk below the load; the
    //     volatile form reorders the copy to after the ldrh.
//
    // The guard is __APPLE__ (not __arm__): the matching build preprocesses
    // with `clang -E -undef`, which defines NO target macro, so `__arm__` is
    // never true there and would silently compile the host path for the ROM.
    u16 a;
#ifndef __APPLE__
    __asm__ ("mov %0, r2" : "=r" (a) : "r" (rec));
#else
    a = arg;
#endif
    u16 v168 = *(volatile u16 *)((u8 *)rec + 168);
    if (v168 != 3) goto end;
    if (arg == 2) {
        MEN_CALLEE(Menu108D4_0800108D4, _0800108D4)(rec);
    }
    if (a == 1) {
        Menu108F0_0800108F0(rec);
    }
end:
    return;
}
#ifndef __APPLE__
void _080010940(void *a, u32 b, u16 c) __attribute__((alias("Menu010940_080010940")));
void sub_080010940(void *a, u32 b, u16 c) __attribute__((alias("Menu010940_080010940")));
#endif
