#include "gtadv/menus.h"
#include "gba/bios.h"
#include "gba/regs.h"


// Minimal external stubs for standalone HOST builds (apple-only; on ARM the
// strong bodies win — these must never be
// ARM-live, or every call below binds to the no-op at compile time).
// ARM-live truth: extern decls to the strong bodies (-21 class fix).
// sub_080055F4/sub_08005604/sub_08002C98 are 0-arg ObjQueue thunks — the old
// call sites passed `rec`, which the bodies never read.
extern void sub_0802D974(const void *s, void *d, u32 m);
extern int  sub_08008014(void);
// The ROM's 0x080188B0 is a raw region; the closure names it sub_0800188B0
// (asm/rec35_runtime.s). The 8-digit spelling does not resolve at link time.
#define sub_080188B0 sub_0800188B0
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
extern void *sub_08004B68(void); // closure spelling for promoted slice links
extern void _0802B234(void);    // 0x0802B234, promoted; closure spelling
extern int  sub_08002780(void);   // 0x08002780 grid flag test; closure spelling
// These two were declared only as function-local externs inside a body further
// down this file, which left the two ROM-transduced menu bodies with implicit
// declarations. Hoisted here so every body in the file can see them.
extern void sub_0802B368(u32 v);
extern void _08002618(u32 a, u32 b);

// The closure exports the VMA spellings for these ROM entries. Keep their
// one-argument C declarations here; _0800C814 casts them to the register
// signatures emitted by the original call sites.
extern u16 _08002178(int a);
extern void _08004C84(u32 idx, u16 value);

// ----------------------------------------------------------------------------
// Substantiated leaf: menu_pkt.s  _08009B60 (VMA 0x08009B60–0x08009BCC)
// Packet layout verified in asm/menu_pkt.s header and asm/carphys_tick.s
// Preserves field widths: u16@+0, s16 floor32+16@+2, u16@+24 cursor, u8@+29 field[1], u8@+20 lo(0x5E0).
//
// PROBE 87/108 at the ROM's own 108 bytes; the three residuals are all one
// instruction of ORDER and none of them changes the body's instructions:
//   * 0x04-0x07: the ROM has `mov r1,sp` BEFORE its `movs r4,#0`, agbcc emits
//     the zero local first. Declaring the zero later in the source is what
//     produces that here -- as a statement after the pkt[0] store the frame
//     base still comes first but the diff moves to 0x0A (85/108), and
//     `(z = 0)` at the store site drops `push {r4}` altogether (104 bytes).
//   * 0x10-0x22: the ROM keeps the call's result in r1 and the packet base in
//     r2 with `mov r2,sp` hoisted above the `cmp r1,#0`; agbcc does the mirror
//     (result in r2, base reloaded into r1 at the store). Pinning either of the
//     two with a register variable reallocates the whole body.
//   * 0x2A-0x2D: `mov r3,sp` before the two literal-pool loads vs after.
// Naming the frame with a `u16 *pk` pointer instead of an array does NOT fix
// the hoists: it makes pk live across the call, which costs a second
// callee-saved push and measures 100 bytes / 16 matched.
void MenuPkt_08009B60(void *ctx) {
    (void)ctx;
    // The WA base must stay a POOL WORD, not a folded offset: the ROM loads
    // 0x03001780 and 0x574 separately (asm/menu_pkt.s lit_08009BC4/lit_08009BC8)
    // and adds them at runtime, and it reuses that one base register for the
    // +0x574 cursor word and the +idx*12 garage record. The `MENU_WA_BASE + K`
    // spelling pools 0x03001CF4 instead.
    __asm__(".globl MenuWaB60\nMenuWaB60 = 0x03001780\n");
    extern u8 MenuWaB60;
    // NOT `= {0}`: the ROM never zeroes the 56-byte frame. It writes pkt[0],
    // pkt[+2], pkt[+8], pkt[+10], pkt[+20], pkt[+24] and pkt[+29] and leaves
    // the rest as garbage, so an aggregate initialiser only adds a `bl memset`
    // (an agbcc intrinsic with no ROM counterpart -> UNRESOLVED_RELOCATION)
    // and 8 extra bytes.
    u8 pkt[56];
    // Materialised once and live ACROSS the `bl`, so it lands in r4 and the
    // prologue is `push {r4,lr}` as the ROM has it.
    // Base of the FIRST store pinned to r1, and declared BEFORE `zero`: the
    // ROM's opening pair is `mov r1,sp / movs r4,#0 / movs r0,#9 /
    // strh r0,[r1,#0]`, so the pointer materialises first and the zero
    // constant (live across the `bl`) second. (r1 is free again by the `bl`,
    // where the return value is pinned back onto r1.)
    register u16 *pkt0 __asm__("r1") = (u16 *)pkt;
    u16 zero = 0;
    pkt0[0] = 9;
    // Pinned to r1: the ROM copies the return value into r1 (`adds r1,r0,#0`
    // at 0x08009B70) and keeps the packet pointer in r2 (`mov r2,sp`), so the
    // index compare is `cmp r1,#0`. Left free, agbcc hands `v` r2 and the
    // packet pointer r1 (87/108).
    register int v __asm__("r1") = sub_08008014();
    // Bias-then-shift, not the `? :` over two shifts: the ROM's `cmp r1,#0 /
    // bge` skips only `adds r0,#31` and BOTH paths share the `asrs r0,r0,#5`
    // at 0x08009B7A. The conditional-shift spelling duplicates the shift and
    // costs a branch.
    // Packet base in r2 from here on: the ROM's `mov r2,sp` sits between the
    // value copy and the `cmp r1,#0` (0x08009B72) and the store is
    // `strh r0,[r2,#2]`, i.e. the pointer is the BASE, not base+2.
    register u16 *pkt2 __asm__("r2") = (u16 *)pkt;
    int bias = (v < 0) ? v + 31 : v;
    // The floored value is pinned to r0 so the final chain is
    // `subs r0,r1,r0 / adds r0,#16 / strh r0,[r2,#2]`. Left as a named plain
    // local the extra pseudo changes which register agbcc hands to `v`
    // (87/108); left as an inline expression the chain accumulates into r1.
    register int floored __asm__("r0") = (bias >> 5) << 5;
    pkt2[1] = (u16)(v - floored + 16);
    *(u16 *)(pkt+8) = zero; *(u16 *)(pkt+10) = zero;
    // The base is loaded AFTER the `bl` (ROM 0x08009B8C), so it must not be a
    // function-wide variable -- live across the call it costs a callee-saved
    // register and the prologue becomes `push {r4,r5,lr}`.
    {
        // Same lever once more: the ROM materialises the +24 destination
        // (`mov r3,sp` at 0x08009B8A) BEFORE loading the WA base.
        register u16 *pkt24 __asm__("r3") = (u16 *)pkt;
        volatile u8 *wa = (volatile u8 *)((uintptr_t)&MenuWaB60);
        s16 *idxp;
        // Two SEPARATE reads of the same halfword: the ROM loads it with
        // `ldrh` for the packet field and again with `ldrsh` for the index.
        pkt24[12] = *(volatile u16 *)(wa + 0x574);
        // NON-volatile `s16 *` read: agbcc lowers a `volatile s16` READ to
        // `ldrh; lsls #16; asrs #16`, but the ROM has one
        // `movs r0,#0 / ldrsh r1,[r1,r0]` (the MenuEcac_0800ECAC recipe
        // above). `int` rather than `s16` forces the extension at the capture
        // site instead of a bare `ldrh`.
        idxp = (s16 *)(wa + 0x574);
        int idx = *idxp;
        pkt[29] = wa[0x31 + idx*12];
        pkt[20] = (u8)(*(volatile u16 *)(wa + 0x5E0) & 0xFF);
        sub_080188B0(pkt);
    }
}

void _08009B60(void *c) __attribute__((alias("MenuPkt_08009B60")));
void sub_08009B60(void *c) __attribute__((alias("MenuPkt_08009B60")));

// ----------------------------------------------------------------------------
// Substantiated leaf: menu_c814.s  _0800C814 (VMA 0x0800C814–0x0800C884)
// Record poller over block B matcher (verified via asm + xref to _08002178/_08002140).
// Sets manager transition via _08004BFC/_08004C84/_08004EC0 vs _08004EA8.
void MenuPoller_0800C814(void *ctx) {
    register volatile u8 *rec __asm__("r5") = (volatile u8 *)ctx;
    register int any1 __asm__("r6") = 0;
    int any2 = 0;
    register int i __asm__("r4") = 0;
    for (; i < (int)_08002140(); i++) {
        u16 v = (u16)((u16 (*)(int, int))(void *)_08002178)(i, 1);
        if ((v & 1u) != 0)
            any1 = 1;
        {
            register u32 two __asm__("r2") = 2;
            if ((v & two) != 0) {
                if (i == 0)
                    any2 = 1;
                *(volatile u16 *)(rec + 4) = (u16)two;
            }
        }
    }
    // menu_c814.s:45-51 — r0 = 0 (grid column), r1 = s16[rec+0] (the record's
    // own leading halfword, loaded with ldrsh after the _08004BFC call).
    if (any1) {
        sub_08004BFC(5);
        {
            register s32 signedKey __asm__("r1");
            signedKey = *(const s16 *)(uintptr_t)rec;
            ((void (*)(u32, s32))(void *)_08004C84)(0u, signedKey);
        }
        sub_08004EC0(1);
    } else if (any2) {
        sub_08004EA8(1);
        rec[28] = 1;
    }
}
__asm__(".align 2, 0");
void _0800C814(void *c) __attribute__((alias("MenuPoller_0800C814")));
void sub_0800C814(void *c) __attribute__((alias("MenuPoller_0800C814")));

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
void _0800CE70(void *a, void *b) __attribute__((alias("MenuCE70_0800CE70")));
void sub_0800CE70(void *a, void *b) __attribute__((alias("MenuCE70_0800CE70")));

// menu_c7f4.s  _0800C7F4 (0xC7F4–0xC814) — mode-record initializer
void MenuC7F4_0800C7F4(void *rec) {
    // substantiated: strh 0 at rec, call 0x04CA8(0), then 0x0254C (0-arg
    // screen init), then 0x02124(0x138A)
    *(volatile u16 *)rec = (u16)sub_08004CA8(0);
    sub_0800254C();
    _08002124(0x138A);
}
void _0800C7F4(void *c) __attribute__((alias("MenuC7F4_0800C7F4")));
void sub_0800C7F4(void *c) __attribute__((alias("MenuC7F4_0800C7F4")));

// menu_c0f0.s  0xC0F0–0xC168 — four helpers, byte-exact
// Closure spelling: asm/code.s defines _080056B8 at 0x080056b8. The
// sub_080056B8 spelling exists only as a host helper below, so the ARM call
// must use the closure's own name (forward-declared here: the only other
// declaration of it sits further down the file, past this body).
extern void _080056B8(int v);

void MenuC0F0_0800C0F0(void *rec) {
    sub_080050E8((u8 *)rec + 8, 1);
    _080056B8(1);
    // asm/menu_c0f0.s:14-16 — the +0x5C halfword is left in r0 and is the
    // argument to 0x08004E1C (the ROM does not reload rec into r0 first).
    u16 v = *(volatile u16 *)((u8 *)rec + 92);
    sub_08004E1C((void *)(uintptr_t)v);
}
void _0800C0F0(void *c) __attribute__((alias("MenuC0F0_0800C0F0")));
void sub_0800C0F0(void *c) __attribute__((alias("MenuC0F0_0800C0F0")));

void MenuC110_0800C110(void *rec) {
    // asm/menu_c0f0.s:26-30 — r0 is not set for the last three (`bl` with no
    // argument setup), and 0x08002B44/B50/BB4 read no input register.
    // 0x080055F4 is likewise 0-arg (pool-loaded thunk).
    sub_080055F4(); sub_08002B50(); sub_08002BB4(); sub_08002B44();
}
__asm__(".align 2, 0");
void _0800C110(void *c) __attribute__((alias("MenuC110_0800C110")));
void sub_0800C110(void *c) __attribute__((alias("MenuC110_0800C110")));

void MenuC128_0800C128(void *rec) {
    (void)rec; // both callees are 0-arg pool thunks (0x030002C0 queue)
    sub_08005604(); sub_08002C98();
}
__asm__(".align 2, 0");
void _0800C128(void *c) __attribute__((alias("MenuC128_0800C128")));
void sub_0800C128(void *c) __attribute__((alias("MenuC128_0800C128")));

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
void _0800C138(void *a,int b) __attribute__((alias("MenuC138_0800C138")));
void sub_0800C138(void *a,int b) __attribute__((alias("MenuC138_0800C138")));

// menu_cc38.s helpers — strong bodies in scene_record_dispatch.c (weak removed: ARM-live
// self-shadow — menus.c calls both names)
void MenuCC38_0800CC38(void *ctx) { (void)ctx; }
__asm__(".align 2, 0");
void _0800CC38(void *c) __attribute__((alias("MenuCC38_0800CC38")));
// True ABI from asm/menu_cc38.s: the switch selector is r0 (`cmp r0,#6`),
// and the ctx passed to every callee is r3 (`adds r0,r3,#0`); case 6
// zero-extends r1/r2 (the two 16-bit halves) and passes them as C884's
// second/third words. The old first-arg `rec`/second-arg `mode` shape put
// the selector in r1, which is why the body mismatched.
void MenuCC3C_0800CC3C(u32 mode, u32 a2lo, u32 a2hi, void *ctx) {
    // 4-way dispatch on mode: 1→C7F4, 7→CAE4, 6→C884, 11→CC38
    switch (mode) {
        case 1: MenuC7F4_0800C7F4(ctx); break;
        case 7: sub_0800CAE4(ctx); break;
        case 6: {
            u16 lo = (u16)a2lo; u16 hi = (u16)a2hi;
            sub_0800C884(ctx, lo, hi); break;
        }
        case 11: MenuCC38_0800CC38(ctx); break;
        default: break;
    }
}
void _0800CC3C(void *a,u32 b,u32 c,void *d) __attribute__((alias("MenuCC3C_0800CC3C")));
void sub_0800CC3C(void *a,u32 b,u32 c,void *d) __attribute__((alias("MenuCC3C_0800CC3C")));

// strong bodies: SubstateClear_04BB8 / SubstateSet1_04BC8 (foundation_subsys.c;
// weak removed: ARM-live self-shadow)
void MenuCC80_0800CC80(void *ctx, u16 a, u16 mask) {
    (void)ctx;
    (void)a;
    void *mgr = _08004B68();
    if (mask & 1) sub_08004BB8(mgr);
    if (mask & 2) sub_08004BC8(mgr);
}
void _0800CC80(void *a, u16 b, u16 c) __attribute__((alias("MenuCC80_0800CC80")));
void MenuCCB0_0800CCB0(int mode, u32 x, u32 y, void *ctx) {
    if (mode == 6) MenuCC80_0800CC80(ctx, (u16)x, (u16)y);
}
void _0800CCB0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuCCB0_0800CCB0")));
void MenuCCC8_0800CCC8(void *ctx, void *rec) {
    (void)ctx;
    volatile u16 *hw = (volatile u16 *)_08004B68();
    volatile u8 *r4 = (volatile u8 *)rec;
    if (hw[1] == 3)
        *(volatile u16 *)(r4 + 84) = 6;
    else
        *(volatile u16 *)(r4 + 84) = 1;
}
void _0800CCC8(void *a, void *b) __attribute__((alias("MenuCCC8_0800CCC8")));

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
void _0800C604(void *c) __attribute__((alias("MenuC604_0800C604")));
void sub_0800C604(void *c) __attribute__((alias("MenuC604_0800C604")));

void MenuC640_0800C640(void *rec) {
    sub_080055F4();
    sub_08002B50();
    sub_08002BB4();
    sub_08002B44();
    (void)rec;
}
__asm__(".align 2, 0");
void _0800C640(void *rec) __attribute__((alias("MenuC640_0800C640")));
void sub_0800C640(void *rec) __attribute__((alias("MenuC640_0800C640")));

void MenuC658_0800C658(void *rec) {
    sub_08005604();
    sub_08002C98();
    (void)rec;
}
__asm__(".align 2, 0");
void _0800C658(void *rec) __attribute__((alias("MenuC658_0800C658")));
void sub_0800C658(void *rec) __attribute__((alias("MenuC658_0800C658")));

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
    // `u32[rec+24]` is re-read at each call site (the ROM reloads `ldr r3,[r7,#24]`
    // before every packet); hoisting it into a local steals `r6` from `p`.
    void *p = r + 36;
    sub_08007B8C(p, 6, 0, *(volatile u32 *)(r + 24), 1, 1, 0, 0);
    sub_08007B8C(p, 5, 88, *(volatile u32 *)(r + 24), 1, 1, 0, 0);
    sub_08007C68(r + 60, *(volatile u32 *)(r + 80), *(volatile u32 *)(r + 84),
                  88, *(volatile u32 *)(r + 24) + 16, 1, 1, 0, 0);
    sub_08007C68(r + 52, *(volatile u32 *)(r + 68), *(volatile u32 *)(r + 72),
                  176, *(volatile u32 *)(r + 24) + 16, 2, 1, 0, 0);
    sub_08007C68(r + 44, *(volatile u32 *)(r + 92), *(volatile u32 *)(r + 96),
                  0, *(volatile u32 *)(r + 16), 2, 1, 0, 0);
}
void _0800DBE8(void *c) __attribute__((alias("MenuDBE8_0800DBE8")));
void sub_0800DBE8(void *c) __attribute__((alias("MenuDBE8_0800DBE8")));
void Sub_0800DBE8(void *c) __attribute__((alias("MenuDBE8_0800DBE8")));
// 146-byte body, one word short of the section's 4-byte alignment: gas pads
// with `nop` (0x46c0) where the ROM holds `00 00`.
__asm__(".align 2, 0");

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
void _0800DC7C(void *a,u32 b) __attribute__((alias("MenuDC7C_0800DC7C")));
void Sub_0800DC7C(void *a,u32 b) __attribute__((alias("MenuDC7C_0800DC7C")));
void sub_0800DC7C(void *a,u32 b) __attribute__((alias("MenuDC7C_0800DC7C")));

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
void _0800DD20(void *a,u32 b,u32 c) __attribute__((alias("MenuDD20_0800DD20")));

// menu_dbe8.s sub_0800DD9C (0x0800DD9C, 176 B) — jump-table dispatch:
//   push {r4, lr}; r4 = arg2 (the record, NOT arg1); *(u8*)(arg2 + 88) = 1;
//   r0 = SceneCtx() (bl 0x08004B68); v = (s16)*(u16*)(ctx + 2) - 15;
//   if v <= 27 (unsigned `bhi`, so any value below 0 also routes to default —
//   the ROM's `(s16)u16 - 15` index SKIPS the table for v<0 with no guard,
//   because the cast makes the compare unsigned): r0 = tab[v] (28 halfword
//   pairs at 0x0800DDC4), `mov pc, r0` into one of the two tails:
//     0x0800DE3C: r1 = r4 + 84, r0 = 1, strh, pop, bx
//     0x0800DE34: r1 = r4 + 84, r0 = 5, strh (shared tail at DDDA merges them)
//   so: *(u16*)(rec + 84) = tab[v] ? 5 : 1.
// NOTE the manifest's 176 B span includes the 28-entry table (112 B) plus the
// 4-byte entry pool at 0x0800DDC4.
void MenuDD9C_0800DD9C(void *arg1, volatile u8 *rec) {
    volatile u8 *base = (volatile u8 *)(rec + 88);
    *base = 1;
    extern void *_08004B68(void);
    // NON-volatile sh rte: the volatile spelling STOPS agbcc from reusing the
    // 0x0800DDC8 pool word, causing an extra ldr; and the read must be
    // written as `(s16)(u16) - 15` so agbcc emits ldrh/subs/lsls/asrs in the
    // ROM's order (subtract FIRST, then sign-extend; writing `(s16)(u16-x)`
    // inverts that and emits `subs` last).
    volatile u8 *ctx = (volatile u8 *)_08004B68();
    int v = (int)(s16)((int)*(u16 *)(uintptr_t)(ctx + 2) - 15);
    u16 out;
    // STORES IN EACH ARM, not one shared store: the ROM emits one full copy of
    // the address expression and the store in each arm (`adds r1,r4,#0 /
    // adds r1,#84 / movs r0,#5 / b`, then the same with `movs r0,#1`). The
    // store base lives in ARG2 (r4 via the prologue copy from r1), so
    // putting the store AFTER the switch with `out = N` instead tail-merges
    // both arms (`movs r1,#5 / movs r1,#1 / adds r0,#84 / strh r1`), which
    // is the last 2 bytes (174/176).
    switch (v) {
    case 0: case 2: case 5: case 8: case 16: case 21: case 22: case 26: case 27:
        *(volatile u16 *)(uintptr_t)(rec + 84) = 5;
        break;
    default:
        *(volatile u16 *)(uintptr_t)(rec + 84) = 1;
        break;
    } }
// The ROM's 174-byte body is followed by a zero halfword (00 00) before the
// next function at 0x0800DE50; the top-level asm must sit BEFORE the alias
// declarations or agbcc drops it and GAS pads the Thumb section with `46 c0`
// instead (same rule as Race_Setup_19200 in src/race_setup.c).
__asm__(".pushsection .text.MenuDD9C_0800DD9C,\"ax\",%progbits\n\t.short 0\n\t.popsection");
void _0800DD9C(void *a,u32 b) __attribute__((alias("MenuDD9C_0800DD9C")));
// alias for high-reg spill variant handled via same function
void sub_0800DD9C(void *a,u32 b) __attribute__((alias("MenuDD9C_0800DD9C")));

// menu_dbe8.s sub_0800DE50 — sound 0x33 init + resource lanes (sl/r9/r8 spills)
extern void sub_0802B214(u32 v);
extern void sub_08007770(int a, void *b, int c, int d, u32 e, u32 f); // 0x08007770 6 args (R2)
__attribute__((naked)) void MenuDE50_0800DE50(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "  push {r4, r5, r6, r7, lr}\n"
        "  mov r7, sl\n"
        "  mov r6, r9\n"
        "  mov r5, r8\n"
        "  push {r5, r6, r7}\n"
        "  sub sp, #8\n"
        "  adds r7, r0, #0\n"
        "  bl sub_08004B68\n"
        "  movs r0, #51\n"
        "  bl sub_0802B214\n"
        "  ldr r0, .Lmde50_0800df9c\n"
        "  ldr r1, .Lmde50_0800dfa0\n"
        "  adds r0, r0, r1\n"
        "  movs r1, #0\n"
        "  mov r9, r1\n"
        "  movs r4, #9\n"
        "  strh r4, [r0, #0]\n"
        "  ldr r5, .Lmde50_0800dfa4\n"
        "  movs r0, #4\n"
        "  str r0, [sp, #0]\n"
        "  movs r0, #1\n"
        "  mov r8, r0\n"
        "  str r0, [sp, #4]\n"
        "  movs r0, #0\n"
        "  adds r1, r5, #0\n"
        "  movs r2, #5\n"
        "  movs r3, #0\n"
        "  bl sub_08007770\n"
        "  ldr r6, .Lmde50_0800dfa8\n"
        "  mov r1, r9\n"
        "  str r1, [sp, #0]\n"
        "  movs r0, #3\n"
        "  str r0, [sp, #4]\n"
        "  movs r0, #1\n"
        "  adds r1, r6, #0\n"
        "  movs r2, #1\n"
        "  movs r3, #0\n"
        "  bl sub_08007770\n"
        "  str r4, [r7, #104]\n"
        "  movs r0, #22\n"
        "  str r0, [r7, #116]\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #128\n"
        "  mov r1, r8\n"
        "  str r1, [r0, #0]\n"
        "  movs r0, #32\n"
        "  adds r0, r0, r7\n"
        "  mov sl, r0\n"
        "  bl sub_0800DAB8\n"
        "  ldr r4, .Lmde50_0800dfac\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #1\n"
        "  movs r2, #0\n"
        "  movs r3, #3\n"
        "  bl sub_08007614\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #1\n"
        "  movs r2, #1\n"
        "  movs r3, #4\n"
        "  bl sub_08007614\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #1\n"
        "  movs r2, #2\n"
        "  movs r3, #5\n"
        "  bl sub_08007614\n"
        "  adds r0, r5, #0\n"
        "  adds r1, r7, #0\n"
        "  bl sub_0800798C\n"
        "  adds r0, r7, #0\n"
        "  bl sub_08007A58\n"
        "  adds r0, r5, #0\n"
        "  movs r1, #4\n"
        "  movs r2, #7\n"
        "  bl sub_080075E8\n"
        "  adds r0, r5, #0\n"
        "  movs r1, #3\n"
        "  movs r2, #6\n"
        "  bl sub_080075E8\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #8\n"
        "  adds r0, r6, #0\n"
        "  bl sub_0800798C\n"
        "  adds r0, r6, #0\n"
        "  movs r1, #0\n"
        "  movs r2, #8\n"
        "  bl sub_080075E8\n"
        "  adds r4, r7, #0\n"
        "  adds r4, #156\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #20\n"
        "  bl 0x08025C08\n"
        "  ldr r0, [r7, #12]\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #160\n"
        "  ldr r1, [r1, #0]\n"
        "  ldr r2, [r4, #0]\n"
        "  bl sub_08007ABC\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #144\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #28\n"
        "  str r0, [r1, #0]\n"
        "  adds r0, #120\n"
        "  mov r1, sl\n"
        "  str r1, [r0, #0]\n"
        "  mov r0, r9\n"
        "  str r0, [r7, #32]\n"
        "  mov r1, r8\n"
        "  strh r1, [r7, #36]\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #52\n"
        "  movs r2, #32\n"
        "  negs r2, r2\n"
        "  movs r1, #0\n"
        "  bl sub_0800D77C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #44\n"
        "  movs r1, #0\n"
        "  movs r2, #160\n"
        "  bl sub_0800D77C\n"
        "  movs r0, #6\n"
        "  strh r0, [r7, #40]\n"
        "  mov r0, r8\n"
        "  strh r0, [r7, #42]\n"
        "  movs r0, #0\n"
        "  bl 0x08004CA8\n"
        "  str r0, [r7, #24]\n"
        "  movs r0, #2\n"
        "  str r0, [r7, #28]\n"
        "  mov r1, r9\n"
        "  strh r1, [r7, #20]\n"
        "  movs r5, #0\n"
        "  movs r4, #0\n"
        ".Lmde50_0800df80:\n"
        "  adds r0, r4, #0\n"
        "  bl 0x08026068\n"
        "  cmp r0, #1\n"
        "  bne .Lmde50_0800df8c\n"
        "  adds r5, #1\n"
        ".Lmde50_0800df8c:\n"
        "  adds r4, #1\n"
        "  cmp r4, #2\n"
        "  ble .Lmde50_0800df80\n"
        "  cmp r5, #0\n"
        "  ble .Lmde50_0800dfb0\n"
        "  movs r0, #1\n"
        "  b .Lmde50_0800dfb2\n"
        "  .align 2, 0\n"
        ".Lmde50_0800df9c: .word 0x03001780\n"
        ".Lmde50_0800dfa0: .word 0x00000FBC\n"
        ".Lmde50_0800dfa4: .word 0x082B37D8\n"
        ".Lmde50_0800dfa8: .word 0x082B7410\n"
        ".Lmde50_0800dfac: .word 0x082A798C\n"
        ".Lmde50_0800dfb0:\n"
        "  movs r0, #0\n"
        ".Lmde50_0800dfb2:\n"
        "  strh r0, [r7, #18]\n"
        "  ldr r0, .Lmde50_0800dfd4\n"
        "  bl 0x08002124\n"
        "  ldr r0, .Lmde50_0800dfd8\n"
        "  ldr r1, .Lmde50_0800dfdc\n"
        "  adds r0, r0, r1\n"
        "  movs r1, #0\n"
        "  strb r1, [r0, #0]\n"
        "  add sp, #8\n"
        "  pop {r3, r4, r5}\n"
        "  mov r8, r3\n"
        "  mov r9, r4\n"
        "  mov sl, r5\n"
        "  pop {r4, r5, r6, r7}\n"
        "  pop {r0}\n"
        "  bx r0\n"
        "  .align 2, 0\n"
        ".Lmde50_0800dfd4: .word 0x0000138C\n"
        ".Lmde50_0800dfd8: .word 0x03001780\n"
        ".Lmde50_0800dfdc: .word 0x000010C3\n"
        ".syntax divided\n"
    );
}

void _0800DE50(void *c) __attribute__((alias("MenuDE50_0800DE50")));

// menu_dbe8.s sub_0800DFE0 — tiny record init: [+0x20]=10,[+0x40]=10,[+0x3C]=1,[+0x1C]=1 (word stores)
void MenuDFE0_0800DFE0(void *rec) {
    *(volatile u32*)((u8*)rec+32)=10;
    *(volatile u32*)((u8*)rec+64)=10;
    *(volatile u32*)((u8*)rec+60)=1;
    *(volatile u32*)((u8*)rec+28)=1;
}
__asm__(".align 2, 0");
void _0800DFE0(void *c) __attribute__((alias("MenuDFE0_0800DFE0")));

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
void _0800E114(void) __attribute__((alias("MenuE114_0800E114")));

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
void _0800E0BC(void) __attribute__((alias("MenuE0BC_0800E0BC")));

// menu_e650.s sub_0800E650 — 34-entry record selector on s16([sub_08004B68+2]-16)
// via the jump table at 0xE69C. Arm order in ROM (store-5, FBC gate, grid
// gate, store-1, store-6) is the source case order; the FBC/grid gates jump
// to the default arm when closed, and all arms share one strh tail (dst+out
// locals, dst assigned per arm).
// The WA base comes through the E650WA absolute symbol (same F5A0WA idiom):
// a visible 0x03001780 literal would let agbcc fold base+offset into one
// pool, but the ROM keeps them as two pools plus `adds`.
extern u8 E650WA[];
void MenuE650_0800E650(void *rec, void *arg) {
    register u16 *dst __asm__("r1");
    register int out __asm__("r0");
    void *mgr;
    int w;
    __asm__(".globl E650WA\nE650WA = 0x03001780\n");
    dst = (u16 *)((u8 *)rec + (133 << 1));
    out = 0;
    *dst = (u16)out;
    {
        u8 *w0 = E650WA;
        if (*(u16 *)(w0 + 0x0FBCu) == 3) {
            dst = (u16 *)((u8 *)arg + 88);
            out = 0;
            *(u8 *)dst = (u8)out;
        }
    }
    mgr = sub_08004B68();
    w = (int)(s16)(*(u16 *)((u8 *)mgr + 2) - 16);
    switch (w) {
    case 0: case 1: case 2: case 3: case 4:
    case 6: case 17: case 19: case 20:
        dst = (u16 *)((u8 *)arg + 84);
        out = 5;
        break;
    case 5: {
        u8 *w1 = E650WA;
        if (*(s16 *)(w1 + 0x0FBCu) == 0)
            goto def6;
        dst = (u16 *)((u8 *)arg + 84);
        out = 5;
        // Preserve the ROM's separate success block for case 5.
        __asm__ volatile("" ::: "r2");
        break;
    }
    case 15: {
        register u8 *w2 __asm__("r2") = E650WA;
        register int row __asm__("r1") = (int)*(s16 *)(w2 + 0x0FF6u) * 2;
        // Keep the ROM's second offset read separate from the first.
        __asm__ volatile("" ::: "memory");
        register int col __asm__("r0") = (int)*(s16 *)(w2 + 0x0FF2u);
        col = col * 8;
        row = row + col;
        w2 += (253 << 4);
        row = row + (int)w2;
        if (*(s16 *)row != 0)
            goto def6;
        dst = (u16 *)((u8 *)arg + 84);
        out = 1;
        // Preserve the ROM's separate success block for case 15.
        __asm__ volatile("" ::: "r3");
        break;
    }
    case 11: case 12: case 13: case 14:
    case 16: case 23: case 33:
        dst = (u16 *)((u8 *)arg + 84);
        out = 1;
        break;
    default:
    def6:
        dst = (u16 *)((u8 *)arg + 84);
        out = 6;
        break;
    }
    *dst = (u16)out;
}
__asm__(".align 2, 0");
void _0800E650(void *a,void *b) __attribute__((alias("MenuE650_0800E650")));
void sub_0800E650(void *a,void *b) __attribute__((alias("MenuE650_0800E650")));

// menu_e650.s sub_0800E7A8 — tiny rec+0xE8/+0xEA leaf
void MenuE7A8_0800E7A8(void *rec) {
    *(volatile u16*)((u8*)rec+232)=1;
    *(volatile u16*)((u8*)rec+234)=1;
    _08002124(0x1393);
    *(volatile u16*)((u8*)rec+232)=2;
}
void _0800E7A8(void *c) __attribute__((alias("MenuE7A8_0800E7A8")));
void sub_0800E7A8(void *c) __attribute__((alias("MenuE7A8_0800E7A8")));

// menu_e650.s sub_0800E7CC — sl/r9/r8 initializer (50, sound 0x32, etc.)
extern void sub_0802B214(u32 v);
extern void sub_08007770(int a, void *b, int c, int d, u32 e, u32 f); // 0x08007770 6 args (R2)
extern void sub_0800DAB8(void *a);
extern void sub_08007614(void *a,u32 b,u32 c,u32 d);
extern void sub_080075E8(void *a,u32 b,u32 c);
extern void sub_0800798C(void *a,void *b);
extern void *sub_0800572C(u32 s);
__attribute__((naked)) void MenuE7CC_0800E7CC(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "  push {r4, r5, r6, r7, lr}\n"
        "  mov r7, sl\n"
        "  mov r6, r9\n"
        "  mov r5, r8\n"
        "  push {r5, r6, r7}\n"
        "  sub sp, #8\n"
        "  adds r7, r0, #0\n"
        "  movs r0, #50\n"
        "  bl sub_0802B214\n"
        "  ldr r0, .Lme7cc_0800e7fc\n"
        "  ldr r1, .Lme7cc_0800e800\n"
        "  adds r0, r0, r1\n"
        "  movs r2, #0\n"
        "  ldrsh r0, [r0, r2]\n"
        "  cmp r0, #3\n"
        "  beq .Lme7cc_0800e804\n"
        "  cmp r0, #7\n"
        "  beq .Lme7cc_0800e80c\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #228\n"
        "  movs r0, #0\n"
        "  b .Lme7cc_0800e812\n"
        "  .align 2, 0\n"
        ".Lme7cc_0800e7fc: .word 0x03001780\n"
        ".Lme7cc_0800e800: .word 0x00000FBC\n"
        ".Lme7cc_0800e804:\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #228\n"
        "  movs r0, #1\n"
        "  b .Lme7cc_0800e812\n"
        ".Lme7cc_0800e80c:\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #228\n"
        "  movs r0, #2\n"
        ".Lme7cc_0800e812:\n"
        "  strh r0, [r1, #0]\n"
        "  mov r9, r1\n"
        "  mov r3, r9\n"
        "  movs r0, #0\n"
        "  ldrsh r2, [r3, r0]\n"
        "  cmp r2, #1\n"
        "  beq .Lme7cc_0800e848\n"
        "  cmp r2, #1\n"
        "  bgt .Lme7cc_0800e82a\n"
        "  cmp r2, #0\n"
        "  beq .Lme7cc_0800e82e\n"
        "  b .Lme7cc_0800e86e\n"
        ".Lme7cc_0800e82a:\n"
        "  cmp r2, #2\n"
        "  bne .Lme7cc_0800e86e\n"
        ".Lme7cc_0800e82e:\n"
        "  ldr r1, .Lme7cc_0800e844\n"
        "  movs r0, #4\n"
        "  str r0, [sp, #0]\n"
        "  movs r0, #1\n"
        "  str r0, [sp, #4]\n"
        "  movs r0, #0\n"
        "  movs r2, #1\n"
        "  movs r3, #0\n"
        "  bl sub_08007770\n"
        "  b .Lme7cc_0800e86e\n"
        "  .align 2, 0\n"
        ".Lme7cc_0800e844: .word 0x082C1268\n"
        ".Lme7cc_0800e848:\n"
        "  ldr r1, .Lme7cc_0800e9e8\n"
        "  movs r0, #4\n"
        "  str r0, [sp, #0]\n"
        "  str r2, [sp, #4]\n"
        "  movs r0, #0\n"
        "  movs r2, #2\n"
        "  movs r3, #0\n"
        "  bl sub_08007770\n"
        "  ldr r1, .Lme7cc_0800e9ec\n"
        "  movs r0, #0\n"
        "  str r0, [sp, #0]\n"
        "  movs r0, #3\n"
        "  str r0, [sp, #4]\n"
        "  movs r0, #1\n"
        "  movs r2, #2\n"
        "  movs r3, #0\n"
        "  bl sub_08007770\n"
        ".Lme7cc_0800e86e:\n"
        "  movs r1, #6\n"
        "  mov sl, r1\n"
        "  str r1, [r7, #112]\n"
        "  movs r0, #3\n"
        "  str r0, [r7, #124]\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #136\n"
        "  movs r2, #1\n"
        "  mov r8, r2\n"
        "  str r2, [r0, #0]\n"
        "  adds r6, r7, #0\n"
        "  adds r6, #40\n"
        "  adds r0, r6, #0\n"
        "  bl sub_0800DAB8\n"
        "  ldr r4, .Lme7cc_0800e9f0\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #1\n"
        "  movs r2, #0\n"
        "  movs r3, #3\n"
        "  bl sub_08007614\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #1\n"
        "  movs r2, #1\n"
        "  movs r3, #4\n"
        "  bl sub_08007614\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #1\n"
        "  movs r2, #2\n"
        "  movs r3, #5\n"
        "  bl sub_08007614\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #0\n"
        "  movs r2, #6\n"
        "  bl sub_080075E8\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #2\n"
        "  movs r2, #7\n"
        "  bl sub_080075E8\n"
        "  ldr r0, .Lme7cc_0800e9f4\n"
        "  movs r1, #0\n"
        "  movs r2, #8\n"
        "  bl sub_080075E8\n"
        "  ldr r0, .Lme7cc_0800e9f8\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #8\n"
        "  bl sub_0800798C\n"
        "  movs r0, #7\n"
        "  bl sub_0800572C\n"
        "  movs r3, #134\n"
        "  lsls r3, r3, #1\n"
        "  adds r1, r7, r3\n"
        "  str r0, [r1, #0]\n"
        "  movs r2, #136\n"
        "  lsls r2, r2, #1\n"
        "  adds r0, r7, r2\n"
        "  movs r5, #2\n"
        "  str r5, [r0, #0]\n"
        "  ldr r0, [r7, #12]\n"
        "  ldr r2, [r1, #0]\n"
        "  movs r1, #2\n"
        "  bl sub_08007ABC\n"
        "  ldr r0, .Lme7cc_0800e9fc\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #16\n"
        "  bl sub_0800798C\n"
        "  movs r0, #14\n"
        "  bl sub_0800572C\n"
        "  movs r3, #140\n"
        "  lsls r3, r3, #1\n"
        "  adds r2, r7, r3\n"
        "  str r0, [r2, #0]\n"
        "  movs r0, #142\n"
        "  lsls r0, r0, #1\n"
        "  adds r1, r7, r0\n"
        "  movs r0, #5\n"
        "  str r0, [r1, #0]\n"
        "  ldr r0, [r7, #20]\n"
        "  ldr r2, [r2, #0]\n"
        "  movs r1, #5\n"
        "  bl sub_08007ABC\n"
        "  ldr r0, .Lme7cc_0800ea00\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #24\n"
        "  bl sub_0800798C\n"
        "  movs r0, #7\n"
        "  bl sub_0800572C\n"
        "  movs r1, #146\n"
        "  lsls r1, r1, #1\n"
        "  adds r2, r7, r1\n"
        "  str r0, [r2, #0]\n"
        "  movs r3, #148\n"
        "  lsls r3, r3, #1\n"
        "  adds r1, r7, r3\n"
        "  movs r0, #50\n"
        "  str r0, [r1, #0]\n"
        "  ldr r0, [r7, #28]\n"
        "  ldr r2, [r2, #0]\n"
        "  movs r1, #50\n"
        "  bl sub_08007ABC\n"
        "  ldr r4, .Lme7cc_0800e9e8\n"
        "  adds r0, r4, #0\n"
        "  adds r1, r7, #0\n"
        "  bl sub_0800798C\n"
        "  adds r0, r7, #0\n"
        "  bl sub_08007A58\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #0\n"
        "  movs r2, #9\n"
        "  bl sub_080075E8\n"
        "  ldr r4, .Lme7cc_0800e9ec\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #32\n"
        "  adds r0, r4, #0\n"
        "  bl sub_0800798C\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #0\n"
        "  movs r2, #10\n"
        "  bl sub_080075E8\n"
        "  movs r0, #152\n"
        "  lsls r0, r0, #1\n"
        "  adds r4, r7, r0\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #5\n"
        "  bl 0x08025BC8\n"
        "  ldr r0, [r7, #36]\n"
        "  movs r2, #154\n"
        "  lsls r2, r2, #1\n"
        "  adds r1, r7, r2\n"
        "  ldr r1, [r1, #0]\n"
        "  ldr r2, [r4, #0]\n"
        "  bl sub_08007ABC\n"
        "  movs r3, #130\n"
        "  lsls r3, r3, #1\n"
        "  adds r1, r7, r3\n"
        "  str r5, [r1, #0]\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #152\n"
        "  str r1, [r0, #0]\n"
        "  adds r0, #4\n"
        "  str r6, [r0, #0]\n"
        "  movs r0, #0\n"
        "  str r0, [r7, #40]\n"
        "  mov r0, r8\n"
        "  strh r0, [r7, #44]\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #60\n"
        "  movs r2, #32\n"
        "  negs r2, r2\n"
        "  movs r1, #0\n"
        "  bl sub_0800D77C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #52\n"
        "  movs r1, #0\n"
        "  movs r2, #160\n"
        "  bl sub_0800D77C\n"
        "  ldr r0, .Lme7cc_0800ea04\n"
        "  ldr r1, .Lme7cc_0800ea08\n"
        "  adds r0, r0, r1\n"
        "  ldrh r0, [r0, #0]\n"
        "  cmp r0, #1\n"
        "  bne .Lme7cc_0800ea0c\n"
        "  movs r0, #8\n"
        "  strh r0, [r7, #48]\n"
        "  b .Lme7cc_0800ea10\n"
        "  .align 2, 0\n"
        ".Lme7cc_0800e9e8: .word 0x082C1268\n"
        ".Lme7cc_0800e9ec: .word 0x082B7410\n"
        ".Lme7cc_0800e9f0: .word 0x082A798C\n"
        ".Lme7cc_0800e9f4: .word 0x082C4228\n"
        ".Lme7cc_0800e9f8: .word 0x082C4458\n"
        ".Lme7cc_0800e9fc: .word 0x082C5040\n"
        ".Lme7cc_0800ea00: .word 0x082D0DC0\n"
        ".Lme7cc_0800ea04: .word 0x03001780\n"
        ".Lme7cc_0800ea08: .word 0x00000FBC\n"
        ".Lme7cc_0800ea0c:\n"
        "  mov r2, sl\n"
        "  strh r2, [r7, #48]\n"
        ".Lme7cc_0800ea10:\n"
        "  movs r2, #0\n"
        "  movs r0, #5\n"
        "  strh r0, [r7, #50]\n"
        "  movs r3, #133\n"
        "  lsls r3, r3, #1\n"
        "  adds r0, r7, r3\n"
        "  strh r2, [r0, #0]\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #224\n"
        "  strh r2, [r1, #0]\n"
        "  ldr r2, .Lme7cc_0800ea38\n"
        "  ldr r3, .Lme7cc_0800ea3c\n"
        "  adds r0, r2, r3\n"
        "  adds r6, r1, #0\n"
        "  ldrh r0, [r0, #0]\n"
        "  cmp r0, #7\n"
        "  bne .Lme7cc_0800ea44\n"
        "  ldr r1, .Lme7cc_0800ea40\n"
        "  adds r0, r2, r1\n"
        "  b .Lme7cc_0800ea48\n"
        "  .align 2, 0\n"
        ".Lme7cc_0800ea38: .word 0x03001780\n"
        ".Lme7cc_0800ea3c: .word 0x00000FBC\n"
        ".Lme7cc_0800ea40: .word 0x00000FC2\n"
        ".Lme7cc_0800ea44:\n"
        "  ldr r3, .Lme7cc_0800eb9c\n"
        "  adds r0, r2, r3\n"
        ".Lme7cc_0800ea48:\n"
        "  ldrh r1, [r0, #0]\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #250\n"
        "  strh r1, [r0, #0]\n"
        "  adds r5, r0, #0\n"
        "  movs r1, #0\n"
        "  ldrsh r0, [r5, r1]\n"
        "  bl sub_080022E4\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #252\n"
        "  movs r3, #0\n"
        "  strh r0, [r1, #0]\n"
        "  ldr r2, .Lme7cc_0800eba0\n"
        "  movs r0, #0\n"
        "  ldrsh r1, [r5, r0]\n"
        "  lsls r0, r1, #1\n"
        "  adds r0, r0, r1\n"
        "  lsls r0, r0, #2\n"
        "  adds r0, r0, r2\n"
        "  adds r0, #49\n"
        "  ldrb r0, [r0, #0]\n"
        "  lsls r0, r0, #24\n"
        "  asrs r0, r0, #24\n"
        "  adds r4, r7, #0\n"
        "  adds r4, #248\n"
        "  strh r0, [r4, #0]\n"
        "  movs r0, #0\n"
        "  ldrsh r1, [r5, r0]\n"
        "  lsls r0, r1, #1\n"
        "  adds r0, r0, r1\n"
        "  lsls r0, r0, #2\n"
        "  adds r0, r0, r2\n"
        "  adds r0, #48\n"
        "  movs r1, #0\n"
        "  ldrsb r1, [r0, r1]\n"
        "  movs r2, #132\n"
        "  lsls r2, r2, #1\n"
        "  adds r0, r7, r2\n"
        "  strh r1, [r0, #0]\n"
        "  strh r3, [r6, #0]\n"
        "  bl 0x08024D74\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #246\n"
        "  strh r0, [r1, #0]\n"
        "  bl 0x08024E34\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #244\n"
        "  strh r0, [r1, #0]\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #160\n"
        "  movs r1, #168\n"
        "  movs r2, #40\n"
        "  bl sub_0800D77C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #168\n"
        "  movs r1, #168\n"
        "  movs r2, #64\n"
        "  bl sub_0800D77C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #176\n"
        "  movs r1, #168\n"
        "  movs r2, #88\n"
        "  bl sub_0800D77C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #184\n"
        "  movs r1, #96\n"
        "  movs r2, #120\n"
        "  bl sub_0800D77C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #192\n"
        "  movs r1, #152\n"
        "  movs r2, #120\n"
        "  bl sub_0800D77C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #200\n"
        "  movs r1, #176\n"
        "  movs r2, #48\n"
        "  bl sub_0800D77C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #208\n"
        "  movs r1, #176\n"
        "  movs r2, #72\n"
        "  bl sub_0800D77C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #216\n"
        "  movs r1, #176\n"
        "  movs r2, #96\n"
        "  bl sub_0800D77C\n"
        "  movs r3, #158\n"
        "  lsls r3, r3, #1\n"
        "  adds r0, r7, r3\n"
        "  movs r2, #0\n"
        "  ldrsh r1, [r5, r2]\n"
        "  movs r3, #0\n"
        "  ldrsh r2, [r4, r3]\n"
        "  bl sub_0800D9A4\n"
        "  movs r1, #0\n"
        "  ldrsh r0, [r5, r1]\n"
        "  bl 0x08024C3C\n"
        "  adds r1, r0, #0\n"
        "  movs r2, #142\n"
        "  lsls r2, r2, #1\n"
        "  adds r0, r7, r2\n"
        "  str r1, [r0, #0]\n"
        "  ldr r0, [r7, #20]\n"
        "  movs r3, #140\n"
        "  lsls r3, r3, #1\n"
        "  adds r2, r7, r3\n"
        "  ldr r2, [r2, #0]\n"
        "  bl sub_08007ABC\n"
        "  movs r1, #0\n"
        "  ldrsh r0, [r5, r1]\n"
        "  bl 0x08024D5C\n"
        "  adds r1, r0, #0\n"
        "  movs r2, #136\n"
        "  lsls r2, r2, #1\n"
        "  adds r0, r7, r2\n"
        "  str r1, [r0, #0]\n"
        "  ldr r0, [r7, #12]\n"
        "  movs r3, #134\n"
        "  lsls r3, r3, #1\n"
        "  adds r2, r7, r3\n"
        "  ldr r2, [r2, #0]\n"
        "  bl sub_08007ABC\n"
        "  movs r1, #0\n"
        "  ldrsh r0, [r5, r1]\n"
        "  bl 0x08024C58\n"
        "  adds r1, r0, #0\n"
        "  movs r2, #148\n"
        "  lsls r2, r2, #1\n"
        "  adds r0, r7, r2\n"
        "  str r1, [r0, #0]\n"
        "  ldr r0, [r7, #28]\n"
        "  movs r3, #146\n"
        "  lsls r3, r3, #1\n"
        "  adds r2, r7, r3\n"
        "  ldr r2, [r2, #0]\n"
        "  bl sub_08007ABC\n"
        "  adds r1, r7, #0\n"
        "  adds r1, #236\n"
        "  movs r0, #2\n"
        "  strh r0, [r1, #0]\n"
        "  mov r0, r9\n"
        "  movs r2, #0\n"
        "  ldrsh r1, [r0, r2]\n"
        "  cmp r1, #1\n"
        "  beq .Lme7cc_0800ebb0\n"
        "  cmp r1, #1\n"
        "  bgt .Lme7cc_0800eba4\n"
        "  cmp r1, #0\n"
        "  beq .Lme7cc_0800eba8\n"
        "  b .Lme7cc_0800ebb6\n"
        "  .align 2, 0\n"
        ".Lme7cc_0800eb9c: .word 0x00000574\n"
        ".Lme7cc_0800eba0: .word 0x03001780\n"
        ".Lme7cc_0800eba4:\n"
        "  cmp r1, #2\n"
        "  bne .Lme7cc_0800ebb6\n"
        ".Lme7cc_0800eba8:\n"
        "  adds r0, r7, #0\n"
        "  bl sub_0800E7A4\n"
        "  b .Lme7cc_0800ebb6\n"
        ".Lme7cc_0800ebb0:\n"
        "  adds r0, r7, #0\n"
        "  bl sub_0800E7A8\n"
        ".Lme7cc_0800ebb6:\n"
        "  movs r3, #128\n"
        "  lsls r3, r3, #1\n"
        "  adds r0, r7, r3\n"
        "  movs r1, #0\n"
        "  strh r1, [r0, #0]\n"
        "  movs r2, #129\n"
        "  lsls r2, r2, #1\n"
        "  adds r0, r7, r2\n"
        "  strh r1, [r0, #0]\n"
        "  add sp, #8\n"
        "  pop {r3, r4, r5}\n"
        "  mov r8, r3\n"
        "  mov r9, r4\n"
        "  mov sl, r5\n"
        "  pop {r4, r5, r6, r7}\n"
        "  pop {r0}\n"
        "  bx r0\n"
        "  .align 2, 0\n"
        ".syntax divided\n"
    );
}

void _0800E7CC(void *c) __attribute__((alias("MenuE7CC_0800E7CC")));

// menu_ff78.s entry dispatcher — 8-way on rec+0xD6 via table 0xFFD0
// Evidence: asm/menu_ff78.s header 0xFFD0 base, 8 entries to 010088/0101FC etc.
__attribute__((naked)) void MenuFF78_0800FF78(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "adds r7, r0, #0\n"
        "ldr r1, .Lff78_ffc4\n"
        "ldr r2, .Lff78_ffc8\n"
        "adds r0, r1, r2\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "cmp r0, #1\n"
        "beq .Lff78_ff98\n"
        "cmp r0, #7\n"
        "bne .Lff78_ff9e\n"
        "ldr r2, .Lff78_ffcc\n"
        "adds r0, r1, r2\n"
        "ldrh r0, [r0, #0]\n"
        "cmp r0, #2\n"
        "bne .Lff78_ff9e\n"
        ".Lff78_ff98:\n"
        "adds r0, r7, #0\n"
        "bl sub_0800F794\n"
        ".Lff78_ff9e:\n"
        "movs r0, #52\n"
        "bl sub_0802B214\n"
        "adds r0, r7, #0\n"
        "bl sub_0800FE90\n"
        "adds r0, r7, #0\n"
        "bl sub_0800FA24\n"
        "adds r0, r7, #0\n"
        "adds r0, #214\n"
        "ldrh r0, [r0, #0]\n"
        "cmp r0, #7\n"
        "bhi .Lff78_1002a\n"
        "lsls r0, r0, #2\n"
        "ldr r1, .Lff78_ffd0\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "mov pc, r0\n"
        ".align 2, 0\n"
        ".Lff78_ffc4: .4byte 0x03001780\n"
        ".Lff78_ffc8: .4byte 0x00000FBC\n"
        ".Lff78_ffcc: .4byte 0x00001078\n"
        ".Lff78_ffd0: .4byte .Lff78_ffd4\n"
        ".Lff78_ffd4: .4byte .Lff78_fff4\n"
        ".4byte .Lff78_fffc\n"
        ".4byte .Lff78_10004\n"
        ".4byte .Lff78_1000c\n"
        ".4byte .Lff78_10014\n"
        ".4byte .Lff78_1001c\n"
        ".4byte .Lff78_1001c\n"
        ".4byte .Lff78_10024\n"
        ".Lff78_fff4:\n"
        "adds r0, r7, #0\n"
        "bl sub_080010088\n"
        "b .Lff78_1002a\n"
        ".Lff78_fffc:\n"
        "adds r0, r7, #0\n"
        "bl sub_0800101FC\n"
        "b .Lff78_1002a\n"
        ".Lff78_10004:\n"
        "adds r0, r7, #0\n"
        "bl sub_080010370\n"
        "b .Lff78_1002a\n"
        ".Lff78_1000c:\n"
        "adds r0, r7, #0\n"
        "bl sub_0800104F0\n"
        "b .Lff78_1002a\n"
        ".Lff78_10014:\n"
        "adds r0, r7, #0\n"
        "bl sub_080010644\n"
        "b .Lff78_1002a\n"
        ".Lff78_1001c:\n"
        "adds r0, r7, #0\n"
        "bl sub_0800107CC\n"
        "b .Lff78_1002a\n"
        ".Lff78_10024:\n"
        "adds r0, r7, #0\n"
        "bl sub_080010370\n"
        ".Lff78_1002a:\n"
        "adds r5, r7, #0\n"
        "adds r5, #244\n"
        "adds r1, r7, #0\n"
        "adds r1, #172\n"
        "movs r2, #0\n"
        "ldrsh r0, [r1, r2]\n"
        "str r0, [r5, #0]\n"
        "movs r2, #0\n"
        "ldrsh r0, [r1, r2]\n"
        "bl _080258A8\n"
        "adds r6, r7, #0\n"
        "adds r6, #232\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #16\n"
        "str r0, [r6, #0]\n"
        "ldr r0, [r5, #0]\n"
        "movs r1, #7\n"
        "bl _08026F30\n"
        "adds r4, r7, #0\n"
        "adds r4, #240\n"
        "str r0, [r4, #0]\n"
        "ldr r0, [r7, #28]\n"
        "ldr r1, [r6, #0]\n"
        "adds r2, r7, #0\n"
        "adds r2, #228\n"
        "ldr r2, [r2, #0]\n"
        "bl sub_08007ABC\n"
        "ldr r0, [r4, #0]\n"
        "ldr r1, [r5, #0]\n"
        "movs r2, #7\n"
        "bl _08026F50\n"
        "adds r0, r7, #0\n"
        "adds r0, #204\n"
        "movs r1, #0\n"
        "strh r1, [r0, #0]\n"
        "adds r0, #2\n"
        "strh r1, [r0, #0]\n"
        "adds r0, #14\n"
        "strh r1, [r0, #0]\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
}
void _0800FF78(void *c) __attribute__((alias("MenuFF78_0800FF78")));

// menu_f810.s sub_0800F810 — record field refresh. Reconstructed from the ROM
// listing, not the old shape:
//   * the selector read is a NON-volatile s16 (`movs r1,#0; ldrsh`).
//   * all cases use the 0x080CB588 table. Case 0 uses twice the sign-extended
//     sub_080258B8 result, case 1 uses twice sub_0800F700 plus 10 bytes, and
//     case 2 uses twice rec+0xC0 plus 20 bytes.
//   * cases 0 and 1 share the table load, WORD store to rec+0x100, and
//     sub_08007ABC(rec[+20], value, rec[+0xFC]) tail. Case 2 has its own tail.
void MenuF810_0800F810(void *rec) {
    extern s16 _080258B8(s16 c);
    extern u32 sub_0800F700(s32 a, s32 b);
    extern void sub_08007ABC(void *a, int b, int c);
    u8 *r4 = (u8 *)rec;
    s16 v = *(s16 *)(r4 + 0xA8);
    u8 *table;
    register u8 *dst asm("r2");
    s32 offset;
    register u32 val asm("r1");
    if (v == 1)
        goto case1;
    if (v > 1)
        goto above1;
    if (v == 0)
        goto case0;
    goto done;
above1:
    if (v == 2)
        goto case2;
    goto done;
case0: {
        s16 result = _080258B8(*(s16 *)(r4 + 0xAC));
        register u32 scale asm("r3") = 128;
        scale <<= 1;
        dst = r4 + scale;
        table = (u8 *)0x080CB588u;
        offset = (s32)((u32)result << 16) >> 15;
        goto common01;
    }
case1: {
        s32 result = (s32)sub_0800F700(*(s16 *)(r4 + 0xBC),
                                      *(s16 *)(r4 + 0xBE));
        register u32 scale asm("r3") = 128;
        scale <<= 1;
        dst = r4 + scale;
        table = (u8 *)0x080CB588u;
        asm volatile("" : "+r"(table) : : "memory");
        offset = result * 2;
        table += 10;
    }
common01:
    val = *(u16 *)(uintptr_t)(offset + (uintptr_t)table);
    *(u32 *)dst = val;
    sub_08007ABC(*(void **)(r4 + 20), (int)val, *(int *)(r4 + 0xFC));
    goto done;
case2: {
        register u32 scale asm("r0") = 128;
        scale <<= 1;
        dst = r4 + scale;
        table = (u8 *)0x080CB588u;
        asm volatile("" : "+r"(table) : : "memory");
        s32 index = *(s16 *)(r4 + 0xC0);
        offset = index * 2;
        table += 20;
        val = *(u16 *)(uintptr_t)(offset + (uintptr_t)table);
        *(u32 *)dst = val;
        sub_08007ABC(*(void **)(r4 + 20), (int)val, *(int *)(r4 + 0xFC));
    }
done:
    return;
}
void _0800F810(void *c) __attribute__((alias("MenuF810_0800F810")));

// menu_f8b4.s sub_0800F8B4 — 6-byte template 0x0805F99A, clamp 0..2, writes
// asm/menu_f8b4.s, reconstructed instruction-for-instruction. Order matters:
// the 6-byte template copy (0x0802E0A4) precedes the 0..2 clamp; the clamp is
// `if ((s16)v <= 0) v = 0;` (the `lsls;cmp #0;bgt` form, NOT an `<0`/`asrs`
// test), then `if ((s16)v > 1) v = 2;`. The 0x08007614 call is
// (0x082D7660, 2, (s16)stk[v*2], 6) — r0 is the pooled 0x082D7660 (not the
// 0x080CB588 the old comment guessed), r1=2, r2 is the ldrsh of the stack
// template at v*2, r3=6.
void MenuF8B4_0800F8B4(void *rec, u32 sel) {
    extern void sub_0802E0A4(void *d, const void *s, u32 n);
    extern void sub_08007614(void *a, u32 b, u32 c, u32 d);
    extern void sub_08007ABC(void *a, int b, int c);
    int v = (u16)sel;
    u8 stk[6];
    u32 w;
    sub_0802E0A4(stk, (const void *)0x0805F99Au, 6);
    if ((s16)v <= 0) v = 0;
    if ((s16)v > 1) v = 2;
    // ROM re-sign-extends the clamped index in place (`lsls r4,r4,#16;
    // asrs r4,r4,#16`) before using it as s16*2 and s16*4.
    // The ROM materializes the pooled 0x082D7660 into r0 BEFORE the
    // sign-extension, so name it in its own statement (agbcc evaluates call
    // arguments right-to-left, which would otherwise sink the pool load past
    // the `lsls/asrs` pair).
    void *addr = (void *)0x082D7660u;
    int idx = (s16)v;
    sub_08007614(addr, 2u,
                 (u32)*(s16 *)(stk + idx * 2), 6u);
    // ROM builds the rec+0x148 destination address first (`movs r0,#0xa4;
    // lsls r0,#1; adds r2,r5,r0`), then loads the table word.
    register volatile u32 *dst __asm__("r2") =
        (volatile u32 *)((u8 *)rec + 0x148);
    // Assembler-resolved absolute base keeps the pool load BEFORE the
    // `idx*4` scale (docs/findings/track_car_26180_pool_order.md): a folded
    // C constant is loaded at its first use, i.e. after the scale.
    {
        extern u8 MenuF8B4Tbl[];
        uintptr_t base;
        __asm__(".globl MenuF8B4Tbl\nMenuF8B4Tbl = 0x080CB57C\n");
        base = (uintptr_t)MenuF8B4Tbl;
        w = *(const u32 *)(base + (u32)idx * 4u);
    }
    *dst = w;
    sub_08007ABC(*(void **)((u8 *)rec + 36), (int)w,
                 *(int *)((u8 *)rec + 0x144));
}
void _0800F8B4(void *a,u32 b) __attribute__((alias("MenuF8B4_0800F8B4")));

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
void _0800ED68(void *c) __attribute__((alias("MenuED68_0800ED68")));

extern u8 Ebd8LatchWA[];
void MenuEbd8_0800EBD8(void *rec, u32 a1, u32 a2) {
    register volatile u8 *r __asm__("r5") = (volatile u8 *)rec;
    u16 cmd = (u16)a2;
    s16 *f = (s16 *)(r + 0xE0);
    int old;
    (void)a1;
    __asm__(".globl Ebd8LatchWA\nEbd8LatchWA = 0x03001780\n");
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
void _0800EBD8(void *a,u32 b,u32 c) __attribute__((alias("MenuEbd8_0800EBD8")));

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
    register volatile u8 *r asm("r4") = (volatile u8 *)rec;
    register s16 *f asm("r5");
    u16 cmd = (u16)a2;
    int old;
    (void)a1;
    f = (s16 *)(r + 0xE0);
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
            register volatile u8 *next_f __asm__("r0") = r + 0xE0;
            /* Match the ROM's r0 address temporary before copying to r5. */
            __asm__ volatile ("" : "+r" (next_f));
            f = (s16 *)next_f;
            if (*(u16 *)f != 2) {
                u32 one;
                sub_0802B368(1);
                *(volatile u16 *)(r + 0x2C) = 0;
                *(volatile u16 *)(r + 0x94) = (u16)(one = 1);
                *(volatile u32 *)(r + 0x44) = one;
                sub_08004BFC((u32)*f);
                if (*(u16 *)f == 3) {
                    *(volatile u16 *)(r + 0x30) = 8;
                }
            } else {
                sub_0802B368(10);
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
void _0800ECAC(void *a,u32 b,u32 c) __attribute__((alias("MenuEcac_0800ECAC")));

// DFF0: input handler with 0x02178 reads and 0x10C3 byte latch
__attribute__((naked)) void MenuDFF0_0800DFF0(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "adds r4, r0, #0\n"
        "movs r6, #0\n"
        "movs r7, #0\n"
        "movs r0, #0\n"
        "movs r1, #1\n"
        "bl 0x08002178\n"
        "lsls r0, r0, #16\n"
        "lsrs r5, r0, #16\n"
        "movs r0, #1\n"
        "movs r1, #1\n"
        "bl 0x08002178\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r5, #1\n"
        "bne .L0DFF0_second\n"
        "cmp r0, #1\n"
        "beq .L0DFF0_sound10\n"
        "movs r6, #1\n"
        "ldr r0, .L0DFF0_wa0\n"
        "ldr r1, .L0DFF0_latch0\n"
        "adds r0, r0, r1\n"
        "strb r6, [r0, #0]\n"
        "b .L0DFF0_after_mode\n"
        ".align 2, 0\n"
        ".L0DFF0_wa0: .word 0x03001780\n"
        ".L0DFF0_latch0: .word 0x000010C3\n"
        ".L0DFF0_second:\n"
        "cmp r0, #1\n"
        "bne .L0DFF0_after_mode\n"
        "ldr r0, .L0DFF0_wa1\n"
        "ldr r2, .L0DFF0_latch1\n"
        "adds r0, r0, r2\n"
        "strb r6, [r0, #0]\n"
        "movs r6, #1\n"
        "b .L0DFF0_after_mode\n"
        ".align 2, 0\n"
        ".L0DFF0_wa1: .word 0x03001780\n"
        ".L0DFF0_latch1: .word 0x000010C3\n"
        ".L0DFF0_sound10:\n"
        "movs r0, #10\n"
        "bl 0x0802B368\n"
        ".L0DFF0_after_mode:\n"
        "cmp r5, #2\n"
        "bne .L0DFF0_after_mode2\n"
        "movs r7, #1\n"
        "strh r5, [r4, #16]\n"
        ".L0DFF0_after_mode2:\n"
        "cmp r6, #0\n"
        "beq .L0DFF0_mode2\n"
        "movs r0, #1\n"
        "bl 0x0802B368\n"
        "movs r0, #0\n"
        "strh r0, [r4, #36]\n"
        "adds r1, r4, #0\n"
        "adds r1, #140\n"
        "movs r0, #1\n"
        "strh r0, [r1, #0]\n"
        "ldr r0, [r4, #24]\n"
        "bl 0x08004BFC\n"
        "ldr r1, [r4, #24]\n"
        "movs r0, #0\n"
        "bl 0x08004C84\n"
        "ldr r1, .L0DFF0_wa2\n"
        "ldr r2, .L0DFF0_music_tbl\n"
        "ldr r0, [r4, #24]\n"
        "lsls r0, r0, #1\n"
        "adds r0, r0, r2\n"
        "ldrh r0, [r0, #0]\n"
        "ldr r2, .L0DFF0_music_off\n"
        "adds r1, r1, r2\n"
        "strh r0, [r1, #0]\n"
        "b .L0DFF0_done\n"
        ".align 2, 0\n"
        ".L0DFF0_wa2: .word 0x03001780\n"
        ".L0DFF0_music_tbl: .word 0x080CB4EA\n"
        ".L0DFF0_music_off: .word 0x00000FBC\n"
        ".L0DFF0_mode2:\n"
        "cmp r7, #0\n"
        "beq .L0DFF0_done\n"
        "movs r0, #4\n"
        "bl 0x0802B368\n"
        "movs r0, #10\n"
        "str r0, [r4, #32]\n"
        "strh r6, [r4, #36]\n"
        "str r0, [r4, #64]\n"
        "str r6, [r4, #60]\n"
        "adds r1, r4, #0\n"
        "adds r1, #168\n"
        "movs r0, #1\n"
        "strb r0, [r1, #0]\n"
        ".L0DFF0_done:\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".syntax divided\n"
    );
}
void _0800DFF0(void *c) __attribute__((alias("MenuDFF0_0800DFF0")));

// E418 renderer — 0xCB448 car array indexed by rec+0x18, plus 0xCB4D8 twin
__attribute__((naked)) void MenuE418_0800E418(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "  push {r4, r5, r6, lr}\n"
        "  sub sp, #20\n"
        "  adds r4, r0, #0\n"
        "  ldrh r1, [r4, #18]\n"
        "  cmp r1, #1\n"
        "  bne .Lme418_0800e440\n"
        "  ldr r0, .Lme418_0800e4dc\n"
        "  ldr r2, [r0, #40]\n"
        "  adds r2, #8\n"
        "  ldr r3, [r0, #44]\n"
        "  movs r0, #6\n"
        "  str r0, [sp, #0]\n"
        "  str r1, [sp, #4]\n"
        "  str r1, [sp, #8]\n"
        "  movs r0, #0\n"
        "  str r0, [sp, #12]\n"
        "  adds r0, r4, #0\n"
        "  movs r1, #17\n"
        "  bl sub_08007B18\n"
        ".Lme418_0800e440:\n"
        "  ldr r0, .Lme418_0800e4e0\n"
        "  ldr r1, .Lme418_0800e4e4\n"
        "  adds r0, r0, r1\n"
        "  ldrh r0, [r0, #0]\n"
        "  cmp r0, #1\n"
        "  bne .Lme418_0800e476\n"
        "  ldr r0, .Lme418_0800e4dc\n"
        "  ldr r2, [r0, #40]\n"
        "  ldr r3, [r0, #44]\n"
        "  ldr r0, .Lme418_0800e4e8\n"
        "  ldrh r5, [r0, #10]\n"
        "  adds r0, r4, #0\n"
        "  adds r0, #154\n"
        "  movs r6, #0\n"
        "  ldrsh r1, [r0, r6]\n"
        "  cmp r1, #1\n"
        "  bne .Lme418_0800e476\n"
        "  movs r0, #5\n"
        "  str r0, [sp, #0]\n"
        "  str r1, [sp, #4]\n"
        "  str r1, [sp, #8]\n"
        "  movs r0, #0\n"
        "  str r0, [sp, #12]\n"
        "  adds r0, r4, #0\n"
        "  adds r1, r5, #0\n"
        "  bl sub_08007B18\n"
        ".Lme418_0800e476:\n"
        "  bl sub_080027EC\n"
        "  cmp r0, #1\n"
        "  bne .Lme418_0800e4a8\n"
        "  ldr r0, .Lme418_0800e4dc\n"
        "  ldr r2, [r0, #56]\n"
        "  ldr r3, [r0, #60]\n"
        "  ldr r0, .Lme418_0800e4e8\n"
        "  ldrh r5, [r0, #14]\n"
        "  adds r0, r4, #0\n"
        "  adds r0, #154\n"
        "  movs r6, #0\n"
        "  ldrsh r1, [r0, r6]\n"
        "  cmp r1, #1\n"
        "  bne .Lme418_0800e4a8\n"
        "  movs r0, #5\n"
        "  str r0, [sp, #0]\n"
        "  str r1, [sp, #4]\n"
        "  str r1, [sp, #8]\n"
        "  movs r0, #0\n"
        "  str r0, [sp, #12]\n"
        "  adds r0, r4, #0\n"
        "  adds r1, r5, #0\n"
        "  bl sub_08007B18\n"
        ".Lme418_0800e4a8:\n"
        "  ldr r2, .Lme418_0800e4dc\n"
        "  ldr r0, [r4, #24]\n"
        "  lsls r0, r0, #3\n"
        "  adds r1, r0, r2\n"
        "  ldr r6, [r1, #0]\n"
        "  adds r2, #4\n"
        "  adds r0, r0, r2\n"
        "  ldr r5, [r0, #0]\n"
        "  ldr r2, [r4, #28]\n"
        "  adds r0, r4, #0\n"
        "  adds r1, r5, #0\n"
        "  bl sub_0800DD20\n"
        "  ldr r1, .Lme418_0800e4e8\n"
        "  ldr r0, [r4, #24]\n"
        "  lsls r0, r0, #1\n"
        "  adds r0, r0, r1\n"
        "  ldrh r1, [r0, #0]\n"
        "  ldr r2, [r4, #28]\n"
        "  cmp r2, #1\n"
        "  beq .Lme418_0800e4f6\n"
        "  cmp r2, #1\n"
        "  bgt .Lme418_0800e4ec\n"
        "  cmp r2, #0\n"
        "  beq .Lme418_0800e500\n"
        "  b .Lme418_0800e540\n"
        "  .align 2, 0\n"
        ".Lme418_0800e4dc: .word 0x080CB448\n"
        ".Lme418_0800e4e0: .word 0x03001780\n"
        ".Lme418_0800e4e4: .word 0x00001076\n"
        ".Lme418_0800e4e8: .word 0x080CB4D8\n"
        ".Lme418_0800e4ec:\n"
        "  cmp r2, #2\n"
        "  beq .Lme418_0800e528\n"
        "  cmp r2, #3\n"
        "  beq .Lme418_0800e50e\n"
        "  b .Lme418_0800e540\n"
        ".Lme418_0800e4f6:\n"
        "  movs r0, #4\n"
        "  str r0, [sp, #0]\n"
        "  str r2, [sp, #4]\n"
        "  str r2, [sp, #8]\n"
        "  b .Lme418_0800e518\n"
        ".Lme418_0800e500:\n"
        "  movs r0, #3\n"
        "  str r0, [sp, #0]\n"
        "  movs r0, #1\n"
        "  str r0, [sp, #4]\n"
        "  str r0, [sp, #8]\n"
        "  str r2, [sp, #12]\n"
        "  b .Lme418_0800e51c\n"
        ".Lme418_0800e50e:\n"
        "  movs r0, #4\n"
        "  str r0, [sp, #0]\n"
        "  movs r0, #1\n"
        "  str r0, [sp, #4]\n"
        "  str r0, [sp, #8]\n"
        ".Lme418_0800e518:\n"
        "  movs r0, #0\n"
        "  str r0, [sp, #12]\n"
        ".Lme418_0800e51c:\n"
        "  adds r0, r4, #0\n"
        "  adds r2, r6, #0\n"
        "  adds r3, r5, #0\n"
        "  bl sub_08007B18\n"
        "  b .Lme418_0800e540\n"
        ".Lme418_0800e528:\n"
        "  movs r0, #3\n"
        "  str r0, [sp, #0]\n"
        "  movs r0, #1\n"
        "  str r0, [sp, #4]\n"
        "  str r0, [sp, #8]\n"
        "  movs r0, #0\n"
        "  str r0, [sp, #12]\n"
        "  adds r0, r4, #0\n"
        "  adds r2, r6, #0\n"
        "  adds r3, r5, #0\n"
        "  bl sub_08007B18\n"
        ".Lme418_0800e540:\n"
        "  adds r0, r4, #0\n"
        "  adds r1, r6, #0\n"
        "  adds r2, r5, #0\n"
        "  bl sub_0800E310\n"
        "  movs r0, #20\n"
        "  ldrsh r5, [r4, r0]\n"
        "  cmp r5, #1\n"
        "  bne .Lme418_0800e578\n"
        "  adds r0, r4, #0\n"
        "  adds r0, #8\n"
        "  adds r1, r4, #0\n"
        "  adds r1, #156\n"
        "  ldr r1, [r1, #0]\n"
        "  adds r2, r4, #0\n"
        "  adds r2, #160\n"
        "  ldr r2, [r2, #0]\n"
        "  movs r3, #56\n"
        "  str r3, [sp, #0]\n"
        "  movs r3, #8\n"
        "  str r3, [sp, #4]\n"
        "  str r5, [sp, #8]\n"
        "  movs r3, #0\n"
        "  str r3, [sp, #12]\n"
        "  str r3, [sp, #16]\n"
        "  movs r3, #12\n"
        "  bl sub_08007C68\n"
        ".Lme418_0800e578:\n"
        "  adds r0, r4, #0\n"
        "  adds r0, #152\n"
        "  movs r1, #15\n"
        "  bl sub_0800D97C\n"
        "  adds r0, r4, #0\n"
        "  adds r0, #32\n"
        "  bl sub_0800DBE8\n"
        "  adds r0, r4, #0\n"
        "  bl sub_0800E598\n"
        "  add sp, #20\n"
        "  pop {r4, r5, r6}\n"
        "  pop {r0}\n"
        "  bx r0\n"
        "  .align 2, 0\n"
        ".syntax divided\n"
    );
}

void _0800E418(void *c) __attribute__((alias("MenuE418_0800E418")));

// dbe8 E15C — r8/r9 key/param handler: 0x02140 count, 0x02178(2)/(3), 0x02780 gate, music 0xCB4EA
__attribute__((naked)) void MenuE15C_0800E15C(void *rec, u32 cmd, u32 param) {
    __asm__ volatile (
        ".syntax unified\n"
        "  push {r4, r5, r6, r7, lr}\n"
        "  mov r7, r9\n"
        "  mov r6, r8\n"
        "  push {r6, r7}\n"
        "  adds r4, r0, #0\n"
        "  lsls r1, r1, #16\n"
        "  lsrs r1, r1, #16\n"
        "  mov r9, r1\n"
        "  lsls r2, r2, #16\n"
        "  lsrs r7, r2, #16\n"
        "  bl sub_08004B68\n"
        "  movs r6, #1\n"
        "  bl 0x08002140\n"
        "  cmp r0, #1\n"
        "  ble .Lme15c_0800e1b6\n"
        "  movs r5, #0\n"
        "  movs r0, #168\n"
        "  adds r0, r0, r4\n"
        "  mov r8, r0\n"
        "  b .Lme15c_0800e1ac\n"
        ".Lme15c_0800e188:\n"
        "  adds r0, r5, #0\n"
        "  movs r1, #2\n"
        "  bl 0x08002178\n"
        "  lsls r0, r0, #16\n"
        "  lsrs r0, r0, #16\n"
        "  cmp r0, #6\n"
        "  beq .Lme15c_0800e19a\n"
        "  movs r6, #0\n"
        ".Lme15c_0800e19a:\n"
        "  adds r0, r5, #0\n"
        "  movs r1, #3\n"
        "  bl 0x08002178\n"
        "  lsls r0, r0, #16\n"
        "  cmp r0, #0\n"
        "  beq .Lme15c_0800e1aa\n"
        "  movs r6, #0\n"
        ".Lme15c_0800e1aa:\n"
        "  adds r5, #1\n"
        ".Lme15c_0800e1ac:\n"
        "  bl 0x08002140\n"
        "  cmp r5, r0\n"
        "  blt .Lme15c_0800e188\n"
        "  b .Lme15c_0800e1be\n"
        ".Lme15c_0800e1b6:\n"
        "  movs r6, #0\n"
        "  movs r1, #168\n"
        "  adds r1, r1, r4\n"
        "  mov r8, r1\n"
        ".Lme15c_0800e1be:\n"
        "  cmp r6, #0\n"
        "  beq .Lme15c_0800e1da\n"
        "  ldrh r1, [r4, #16]\n"
        "  movs r2, #16\n"
        "  ldrsh r0, [r4, r2]\n"
        "  cmp r0, #0\n"
        "  ble .Lme15c_0800e1d2\n"
        "  subs r0, r1, #1\n"
        "  strh r0, [r4, #16]\n"
        "  b .Lme15c_0800e278\n"
        ".Lme15c_0800e1d2:\n"
        "  adds r0, r4, #0\n"
        "  bl sub_0800DFF0\n"
        "  b .Lme15c_0800e278\n"
        ".Lme15c_0800e1da:\n"
        "  cmp r7, #1\n"
        "  bne .Lme15c_0800e25c\n"
        "  ldr r1, [r4, #24]\n"
        "  cmp r1, #6\n"
        "  beq .Lme15c_0800e24c\n"
        "  ldrh r0, [r4, #18]\n"
        "  cmp r0, #0\n"
        "  bne .Lme15c_0800e1ee\n"
        "  cmp r1, #5\n"
        "  beq .Lme15c_0800e1fc\n"
        ".Lme15c_0800e1ee:\n"
        "  bl 0x08002780\n"
        "  cmp r0, #1\n"
        "  bne .Lme15c_0800e204\n"
        "  ldr r0, [r4, #24]\n"
        "  cmp r0, #7\n"
        "  bne .Lme15c_0800e204\n"
        ".Lme15c_0800e1fc:\n"
        "  movs r0, #10\n"
        "  bl sub_0802B368\n"
        "  b .Lme15c_0800e25c\n"
        ".Lme15c_0800e204:\n"
        "  movs r0, #1\n"
        "  bl sub_0802B368\n"
        "  movs r0, #0\n"
        "  strh r0, [r4, #36]\n"
        "  adds r1, r4, #0\n"
        "  adds r1, #140\n"
        "  movs r0, #1\n"
        "  strh r0, [r1, #0]\n"
        "  ldr r0, [r4, #24]\n"
        "  bl 0x08004BFC\n"
        "  ldr r1, [r4, #24]\n"
        "  movs r0, #0\n"
        "  bl 0x08004C84\n"
        "  ldr r1, .Lme15c_0800e240\n"
        "  ldr r2, .Lme15c_0800e244\n"
        "  ldr r3, [r4, #24]\n"
        "  lsls r0, r3, #1\n"
        "  adds r0, r0, r2\n"
        "  ldrh r0, [r0, #0]\n"
        "  ldr r2, .Lme15c_0800e248\n"
        "  adds r1, r1, r2\n"
        "  strh r0, [r1, #0]\n"
        "  cmp r3, #7\n"
        "  bne .Lme15c_0800e25c\n"
        "  movs r0, #8\n"
        "  strh r0, [r4, #40]\n"
        "  b .Lme15c_0800e25c\n"
        "  .align 2, 0\n"
        ".Lme15c_0800e240: .word 0x03001780\n"
        ".Lme15c_0800e244: .word 0x080CB4EA\n"
        ".Lme15c_0800e248: .word 0x00000FBC\n"
        ".Lme15c_0800e24c:\n"
        "  movs r0, #10\n"
        "  bl sub_0802B368\n"
        "  movs r0, #1\n"
        "  movs r1, #1\n"
        "  bl 0x08002618\n"
        "  strh r7, [r4, #20]\n"
        ".Lme15c_0800e25c:\n"
        "  cmp r7, #2\n"
        "  bne .Lme15c_0800e278\n"
        "  movs r0, #4\n"
        "  bl sub_0802B368\n"
        "  movs r1, #10\n"
        "  str r1, [r4, #32]\n"
        "  movs r0, #0\n"
        "  strh r0, [r4, #36]\n"
        "  str r1, [r4, #64]\n"
        "  str r0, [r4, #60]\n"
        "  movs r0, #1\n"
        "  mov r1, r8\n"
        ".Lme15c_0800e276:\n"
        "  strb r0, [r1, #0]\n"
        ".Lme15c_0800e278:\n"
        "  cmp r7, #64\n"
        "  beq .Lme15c_0800e280\n"
        "  cmp r7, #32\n"
        "  bne .Lme15c_0800e28c\n"
        ".Lme15c_0800e280:\n"
        "  movs r0, #2\n"
        "  bl sub_0802B368\n"
        "  ldr r0, [r4, #24]\n"
        "  subs r0, #1\n"
        "  str r0, [r4, #24]\n"
        ".Lme15c_0800e28c:\n"
        "  cmp r7, #128\n"
        "  beq .Lme15c_0800e294\n"
        "  cmp r7, #16\n"
        "  bne .Lme15c_0800e2a0\n"
        ".Lme15c_0800e294:\n"
        "  movs r0, #2\n"
        "  bl sub_0802B368\n"
        "  ldr r0, [r4, #24]\n"
        "  adds r0, #1\n"
        "  str r0, [r4, #24]\n"
        ".Lme15c_0800e2a0:\n"
        "  ldr r0, [r4, #24]\n"
        "  cmp r0, #0\n"
        "  bge .Lme15c_0800e2aa\n"
        "  movs r0, #8\n"
        "  str r0, [r4, #24]\n"
        ".Lme15c_0800e2aa:\n"
        "  ldr r0, [r4, #24]\n"
        "  cmp r0, #8\n"
        "  ble .Lme15c_0800e2b4\n"
        "  movs r0, #0\n"
        "  str r0, [r4, #24]\n"
        ".Lme15c_0800e2b4:\n"
        "  movs r0, #0\n"
        "  mov r1, r9\n"
        "  bl 0x08002158\n"
        "  movs r0, #1\n"
        "  adds r1, r7, #0\n"
        "  bl 0x08002158\n"
        "  ldrh r1, [r4, #24]\n"
        "  movs r0, #2\n"
        "  bl 0x08002158\n"
        "  mov r2, r8\n"
        "  ldrb r1, [r2, #0]\n"
        "  movs r0, #3\n"
        "  bl 0x08002158\n"
        "  pop {r3, r4}\n"
        "  mov r8, r3\n"
        "  mov r9, r4\n"
        "  pop {r4, r5, r6, r7}\n"
        "  pop {r0}\n"
        "  bx r0\n"
        "  .align 2, 0\n"
        ".syntax divided\n"
    );
}

void _0800E15C(void *a,u32 b,u32 c) __attribute__((alias("MenuE15C_0800E15C")));

// dbe8 sub_0800E310(rec, a1, a2) — 0x9A item-table emitter. Gate
// s16[rec+0x9A] == 0; selector u32[rec+0x1C] must be 2 or 3; u32[rec+0x18]
// picks the lane off the record base at rec+0x44 (adds r0,#68).
//   sub == 0  -> sub_08007B18(base, 10, 0, a2+9, {2,1,1,0})
//   sub == 8  -> sub_08007B18(base,  9, 0, a2-9, {2,1,1,0})
//   otherwise -> both of the above, mode 10 then 9.
// (objdump prints load offsets in decimal: #28 = 0x1C, #24 = 0x18.)
__attribute__((naked)) void MenuE310_0800E310(void *rec, u32 a1, u32 a2) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r8\n"
        "push {r7}\n"
        "sub sp, #16\n"
        "adds r7, r2, #0\n"
        "adds r1, r0, #0\n"
        "adds r1, #154\n"
        "movs r3, #0\n"
        "ldrsh r2, [r1, r3]\n"
        "mov r8, r2\n"
        "cmp r2, #0\n"
        "bne.n .Le310_done\n"
        "ldr r6, [r0, #28]\n"
        "cmp r6, #2\n"
        "beq.n .Le310_case2\n"
        "cmp r6, #3\n"
        "bne.n .Le310_done\n"
        "ldr r1, [r0, #24]\n"
        "cmp r1, #0\n"
        "bne.n .Le310_case1_sub\n"
        "adds r0, #68\n"
        "adds r3, r7, #0\n"
        "adds r3, #9\n"
        "movs r1, #2\n"
        "str r1, [sp, #0]\n"
        "movs r1, #1\n"
        "str r1, [sp, #4]\n"
        "str r1, [sp, #8]\n"
        "str r2, [sp, #12]\n"
        "movs r1, #10\n"
        "b.n .Le310_call10\n"
        ".Le310_case1_sub:\n"
        "cmp r1, #8\n"
        "bne.n .Le310_case1_general\n"
        "adds r0, #68\n"
        "adds r3, r7, #0\n"
        "subs r3, #9\n"
        "movs r1, #2\n"
        "str r1, [sp, #0]\n"
        "movs r1, #1\n"
        "str r1, [sp, #4]\n"
        "str r1, [sp, #8]\n"
        "mov r1, r8\n"
        "str r1, [sp, #12]\n"
        "b.n .Le310_shared_call9\n"
        ".Le310_case1_general:\n"
        "adds r6, r0, #0\n"
        "adds r6, #68\n"
        "adds r3, r7, #0\n"
        "adds r3, #9\n"
        "movs r5, #2\n"
        "str r5, [sp, #0]\n"
        "movs r4, #1\n"
        "str r4, [sp, #4]\n"
        "str r4, [sp, #8]\n"
        "mov r2, r8\n"
        "str r2, [sp, #12]\n"
        "adds r0, r6, #0\n"
        "movs r1, #10\n"
        "movs r2, #0\n"
        "bl 0x08007b18\n"
        "adds r3, r7, #0\n"
        "subs r3, #9\n"
        "str r5, [sp, #0]\n"
        "str r4, [sp, #4]\n"
        "str r4, [sp, #8]\n"
        "mov r0, r8\n"
        "str r0, [sp, #12]\n"
        "adds r0, r6, #0\n"
        "b.n .Le310_shared_call9\n"
        ".Le310_case2:\n"
        "ldr r1, [r0, #24]\n"
        "cmp r1, #0\n"
        "bne.n .Le310_case2_sub\n"
        "adds r0, #68\n"
        "adds r3, r7, #0\n"
        "adds r3, #9\n"
        "str r6, [sp, #0]\n"
        "movs r1, #1\n"
        "str r1, [sp, #4]\n"
        "str r1, [sp, #8]\n"
        "mov r1, r8\n"
        "str r1, [sp, #12]\n"
        "movs r1, #10\n"
        "b.n .Le310_call10\n"
        ".Le310_case2_sub:\n"
        "cmp r1, #8\n"
        "bne.n .Le310_case2_general\n"
        "adds r0, #68\n"
        "adds r3, r7, #0\n"
        "subs r3, #9\n"
        "str r6, [sp, #0]\n"
        "movs r1, #1\n"
        "str r1, [sp, #4]\n"
        "str r1, [sp, #8]\n"
        "mov r2, r8\n"
        "str r2, [sp, #12]\n"
        ".Le310_shared_call9:\n"
        "movs r1, #9\n"
        ".Le310_call10:\n"
        "movs r2, #0\n"
        "bl 0x08007b18\n"
        "b.n .Le310_done\n"
        ".Le310_case2_general:\n"
        "adds r5, r0, #0\n"
        "adds r5, #68\n"
        "adds r3, r7, #0\n"
        "adds r3, #9\n"
        "str r6, [sp, #0]\n"
        "movs r4, #1\n"
        "str r4, [sp, #4]\n"
        "str r4, [sp, #8]\n"
        "mov r0, r8\n"
        "str r0, [sp, #12]\n"
        "adds r0, r5, #0\n"
        "movs r1, #10\n"
        "movs r2, #0\n"
        "bl 0x08007b18\n"
        "adds r3, r7, #0\n"
        "subs r3, #9\n"
        "str r6, [sp, #0]\n"
        "str r4, [sp, #4]\n"
        "str r4, [sp, #8]\n"
        "mov r1, r8\n"
        "str r1, [sp, #12]\n"
        "adds r0, r5, #0\n"
        "movs r1, #9\n"
        "movs r2, #0\n"
        "bl 0x08007b18\n"
        ".Le310_done:\n"
        "add sp, #16\n"
        "pop {r3}\n"
        "mov r8, r3\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".syntax divided\n"
    );
}
void _0800E310(void *a,u32 b,u32 c) __attribute__((alias("MenuE310_0800E310")));

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
void _0800F040(void *a,u32 b,u32 c) __attribute__((alias("MenuF040_0800F040")));

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
void _0800F0BC(void *a,u32 b,u32 c) __attribute__((alias("MenuF0BC_0800F0BC")));

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
void _0800F134(void *a,u32 b,u32 c) __attribute__((alias("MenuF134_0800F134")));

__attribute__((naked)) void MenuF22C_0800F22C(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "  push {r4, r5, r6, r7, lr}\n"
        "  mov r7, sl\n"
        "  mov r6, r9\n"
        "  mov r5, r8\n"
        "  push {r5, r6, r7}\n"
        "  sub sp, #24\n"
        "  adds r7, r0, #0\n"
        "  adds r0, #76\n"
        "  movs r1, #7\n"
        "  str r1, [sp, #0]\n"
        "  movs r1, #3\n"
        "  str r1, [sp, #4]\n"
        "  movs r1, #1\n"
        "  str r1, [sp, #8]\n"
        "  movs r5, #0\n"
        "  str r5, [sp, #12]\n"
        "  movs r1, #8\n"
        "  movs r2, #72\n"
        "  movs r3, #88\n"
        "  bl sub_08007B18\n"
        "  movs r1, #158\n"
        "  lsls r1, r1, #1\n"
        "  adds r0, r7, r1\n"
        "  ldr r0, [r0, #0]\n"
        "  movs r1, #120\n"
        "  movs r2, #56\n"
        "  bl 0x08026A2C\n"
        "  ldr r2, .Lmf22c_0800f42c\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #250\n"
        "  movs r3, #0\n"
        "  ldrsh r1, [r0, r3]\n"
        "  lsls r0, r1, #2\n"
        "  adds r0, r0, r1\n"
        "  lsls r0, r0, #2\n"
        "  adds r0, r0, r2\n"
        "  movs r1, #8\n"
        "  ldrsb r1, [r0, r1]\n"
        "  cmp r1, #1\n"
        "  bne .Lmf22c_0800f296\n"
        "  movs r0, #9\n"
        "  str r0, [sp, #0]\n"
        "  str r1, [sp, #4]\n"
        "  str r1, [sp, #8]\n"
        "  str r5, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  movs r1, #11\n"
        "  movs r2, #128\n"
        "  movs r3, #104\n"
        "  bl sub_08007B18\n"
        ".Lmf22c_0800f296:\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #228\n"
        "  adds r6, r0, #0\n"
        "  ldrh r4, [r6, #0]\n"
        "  cmp r4, #2\n"
        "  beq .Lmf22c_0800f348\n"
        "  ldr r0, .Lmf22c_0800f430\n"
        "  ldr r2, [r0, #8]\n"
        "  ldr r0, [r0, #12]\n"
        "  mov r8, r0\n"
        "  ldr r0, .Lmf22c_0800f434\n"
        "  ldrh r1, [r0, #2]\n"
        "  ldr r0, .Lmf22c_0800f438\n"
        "  ldr r3, .Lmf22c_0800f43c\n"
        "  adds r0, r0, r3\n"
        "  ldrh r0, [r0, #0]\n"
        "  cmp r0, #1\n"
        "  bne .Lmf22c_0800f2d8\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #242\n"
        "  movs r3, #0\n"
        "  ldrsh r4, [r0, r3]\n"
        "  cmp r4, #1\n"
        "  bne .Lmf22c_0800f2d8\n"
        "  movs r0, #5\n"
        "  str r0, [sp, #0]\n"
        "  str r4, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r5, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  mov r3, r8\n"
        "  bl sub_08007B18\n"
        ".Lmf22c_0800f2d8:\n"
        "  ldr r0, .Lmf22c_0800f430\n"
        "  ldr r2, [r0, #16]\n"
        "  ldr r3, [r0, #20]\n"
        "  ldr r0, .Lmf22c_0800f434\n"
        "  ldrh r1, [r0, #4]\n"
        "  ldr r0, .Lmf22c_0800f438\n"
        "  ldr r4, .Lmf22c_0800f440\n"
        "  adds r0, r0, r4\n"
        "  ldrh r0, [r0, #0]\n"
        "  cmp r0, #1\n"
        "  bne .Lmf22c_0800f30c\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #242\n"
        "  movs r5, #0\n"
        "  ldrsh r4, [r0, r5]\n"
        "  cmp r4, #1\n"
        "  bne .Lmf22c_0800f30c\n"
        "  movs r0, #5\n"
        "  str r0, [sp, #0]\n"
        "  str r4, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  movs r0, #0\n"
        "  str r0, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  bl sub_08007B18\n"
        ".Lmf22c_0800f30c:\n"
        "  ldrh r6, [r6, #0]\n"
        "  cmp r6, #1\n"
        "  beq .Lmf22c_0800f348\n"
        "  ldr r0, .Lmf22c_0800f430\n"
        "  ldr r5, [r0, #24]\n"
        "  ldr r6, [r0, #28]\n"
        "  ldr r0, .Lmf22c_0800f434\n"
        "  ldrh r4, [r0, #6]\n"
        "  bl sub_080027EC\n"
        "  cmp r0, #1\n"
        "  bne .Lmf22c_0800f348\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #242\n"
        "  movs r2, #0\n"
        "  ldrsh r1, [r0, r2]\n"
        "  cmp r1, #1\n"
        "  bne .Lmf22c_0800f348\n"
        "  movs r0, #5\n"
        "  str r0, [sp, #0]\n"
        "  str r1, [sp, #4]\n"
        "  str r1, [sp, #8]\n"
        "  movs r0, #0\n"
        "  str r0, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  adds r1, r4, #0\n"
        "  adds r2, r5, #0\n"
        "  adds r3, r6, #0\n"
        "  bl sub_08007B18\n"
        ".Lmf22c_0800f348:\n"
        "  ldr r2, .Lmf22c_0800f430\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #224\n"
        "  movs r3, #0\n"
        "  ldrsh r0, [r0, r3]\n"
        "  lsls r0, r0, #3\n"
        "  adds r1, r0, r2\n"
        "  ldr r6, [r1, #0]\n"
        "  adds r2, #4\n"
        "  adds r0, r0, r2\n"
        "  ldr r5, [r0, #0]\n"
        "  movs r0, #130\n"
        "  lsls r0, r0, #1\n"
        "  adds r4, r7, r0\n"
        "  ldr r2, [r4, #0]\n"
        "  adds r0, r7, #0\n"
        "  adds r1, r5, #0\n"
        "  bl sub_0800F040\n"
        "  ldr r3, [r4, #0]\n"
        "  adds r0, r7, #0\n"
        "  adds r1, r6, #0\n"
        "  adds r2, r5, #0\n"
        "  bl sub_0800F0BC\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #160\n"
        "  ldr r2, [r0, #0]\n"
        "  adds r0, #4\n"
        "  ldr r3, [r0, #0]\n"
        "  movs r1, #6\n"
        "  mov r8, r1\n"
        "  str r1, [sp, #0]\n"
        "  movs r6, #1\n"
        "  str r6, [sp, #4]\n"
        "  str r6, [sp, #8]\n"
        "  movs r4, #0\n"
        "  mov r9, r4\n"
        "  str r4, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  movs r1, #8\n"
        "  bl sub_08007B18\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #168\n"
        "  ldr r2, [r0, #0]\n"
        "  adds r0, #4\n"
        "  ldr r3, [r0, #0]\n"
        "  mov r5, r8\n"
        "  str r5, [sp, #0]\n"
        "  str r6, [sp, #4]\n"
        "  str r6, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  movs r1, #8\n"
        "  bl sub_08007B18\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #176\n"
        "  ldr r2, [r0, #0]\n"
        "  adds r0, #4\n"
        "  ldr r3, [r0, #0]\n"
        "  str r5, [sp, #0]\n"
        "  str r6, [sp, #4]\n"
        "  str r6, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  movs r1, #8\n"
        "  bl sub_08007B18\n"
        "  adds r5, r7, #0\n"
        "  adds r5, #192\n"
        "  ldr r2, [r5, #0]\n"
        "  adds r4, r7, #0\n"
        "  adds r4, #196\n"
        "  ldr r3, [r4, #0]\n"
        "  mov r0, r8\n"
        "  str r0, [sp, #0]\n"
        "  str r6, [sp, #4]\n"
        "  str r6, [sp, #8]\n"
        "  mov r1, r9\n"
        "  str r1, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  movs r1, #6\n"
        "  bl sub_08007B18\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #184\n"
        "  ldr r2, [r0, #0]\n"
        "  adds r0, #4\n"
        "  ldr r3, [r0, #0]\n"
        "  mov r0, r8\n"
        "  str r0, [sp, #0]\n"
        "  str r6, [sp, #4]\n"
        "  str r6, [sp, #8]\n"
        "  mov r1, r9\n"
        "  str r1, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  movs r1, #7\n"
        "  bl sub_08007B18\n"
        "  movs r2, #132\n"
        "  lsls r2, r2, #1\n"
        "  adds r0, r7, r2\n"
        "  movs r3, #0\n"
        "  ldrsh r0, [r0, r3]\n"
        "  mov sl, r5\n"
        "  str r4, [sp, #20]\n"
        "  cmp r0, #0\n"
        "  beq .Lmf22c_0800f444\n"
        "  cmp r0, #1\n"
        "  beq .Lmf22c_0800f45e\n"
        "  b .Lmf22c_0800f476\n"
        "  .align 2, 0\n"
        ".Lmf22c_0800f42c: .word 0x080CCEEC\n"
        ".Lmf22c_0800f430: .word 0x080CB508\n"
        ".Lmf22c_0800f434: .word 0x080CB548\n"
        ".Lmf22c_0800f438: .word 0x03001780\n"
        ".Lmf22c_0800f43c: .word 0x00001058\n"
        ".Lmf22c_0800f440: .word 0x0000105A\n"
        ".Lmf22c_0800f444:\n"
        "  mov r4, r8\n"
        "  str r4, [sp, #0]\n"
        "  str r6, [sp, #4]\n"
        "  str r6, [sp, #8]\n"
        "  mov r5, r9\n"
        "  str r5, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  movs r1, #9\n"
        "  movs r2, #96\n"
        "  movs r3, #128\n"
        "  bl sub_08007B18\n"
        "  b .Lmf22c_0800f476\n"
        ".Lmf22c_0800f45e:\n"
        "  mov r1, r8\n"
        "  str r1, [sp, #0]\n"
        "  str r0, [sp, #4]\n"
        "  str r0, [sp, #8]\n"
        "  mov r2, r9\n"
        "  str r2, [sp, #12]\n"
        "  adds r0, r7, #0\n"
        "  movs r1, #10\n"
        "  movs r2, #120\n"
        "  movs r3, #128\n"
        "  bl sub_08007B18\n"
        ".Lmf22c_0800f476:\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #8\n"
        "  movs r3, #134\n"
        "  lsls r3, r3, #1\n"
        "  adds r1, r7, r3\n"
        "  ldr r1, [r1, #0]\n"
        "  movs r4, #136\n"
        "  lsls r4, r4, #1\n"
        "  adds r2, r7, r4\n"
        "  ldr r2, [r2, #0]\n"
        "  adds r3, r7, #0\n"
        "  adds r3, #200\n"
        "  ldr r3, [r3, #0]\n"
        "  adds r4, r7, #0\n"
        "  adds r4, #204\n"
        "  ldr r4, [r4, #0]\n"
        "  str r4, [sp, #0]\n"
        "  movs r6, #8\n"
        "  str r6, [sp, #4]\n"
        "  movs r5, #1\n"
        "  str r5, [sp, #8]\n"
        "  str r5, [sp, #12]\n"
        "  movs r4, #0\n"
        "  mov r8, r4\n"
        "  str r4, [sp, #16]\n"
        "  bl sub_08007BFC\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #16\n"
        "  movs r2, #140\n"
        "  lsls r2, r2, #1\n"
        "  adds r1, r7, r2\n"
        "  ldr r1, [r1, #0]\n"
        "  movs r3, #142\n"
        "  lsls r3, r3, #1\n"
        "  adds r2, r7, r3\n"
        "  ldr r2, [r2, #0]\n"
        "  adds r3, r7, #0\n"
        "  adds r3, #208\n"
        "  ldr r3, [r3, #0]\n"
        "  adds r4, r7, #0\n"
        "  adds r4, #212\n"
        "  ldr r4, [r4, #0]\n"
        "  str r4, [sp, #0]\n"
        "  str r6, [sp, #4]\n"
        "  str r5, [sp, #8]\n"
        "  str r5, [sp, #12]\n"
        "  mov r4, r8\n"
        "  str r4, [sp, #16]\n"
        "  bl sub_08007BFC\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #24\n"
        "  movs r2, #146\n"
        "  lsls r2, r2, #1\n"
        "  adds r1, r7, r2\n"
        "  ldr r1, [r1, #0]\n"
        "  movs r3, #148\n"
        "  lsls r3, r3, #1\n"
        "  adds r2, r7, r3\n"
        "  ldr r2, [r2, #0]\n"
        "  adds r3, r7, #0\n"
        "  adds r3, #216\n"
        "  ldr r3, [r3, #0]\n"
        "  adds r4, r7, #0\n"
        "  adds r4, #220\n"
        "  ldr r4, [r4, #0]\n"
        "  str r4, [sp, #0]\n"
        "  str r6, [sp, #4]\n"
        "  str r5, [sp, #8]\n"
        "  str r5, [sp, #12]\n"
        "  mov r4, r8\n"
        "  str r4, [sp, #16]\n"
        "  bl sub_08007BFC\n"
        "  mov r5, sl\n"
        "  ldr r0, [r5, #0]\n"
        "  adds r0, #24\n"
        "  ldr r2, [sp, #20]\n"
        "  ldr r1, [r2, #0]\n"
        "  adds r1, #8\n"
        "  adds r2, r7, #0\n"
        "  adds r2, #244\n"
        "  movs r3, #0\n"
        "  ldrsh r2, [r2, r3]\n"
        "  bl 0x0800399C\n"
        "  ldr r0, [r5, #0]\n"
        "  adds r0, #48\n"
        "  ldr r4, [sp, #20]\n"
        "  ldr r1, [r4, #0]\n"
        "  adds r1, #8\n"
        "  adds r2, r7, #0\n"
        "  adds r2, #246\n"
        "  movs r5, #0\n"
        "  ldrsh r2, [r2, r5]\n"
        "  bl 0x0800399C\n"
        "  movs r1, #133\n"
        "  lsls r1, r1, #1\n"
        "  adds r0, r7, r1\n"
        "  movs r2, #0\n"
        "  ldrsh r4, [r0, r2]\n"
        "  cmp r4, #1\n"
        "  bne .Lmf22c_0800f572\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #32\n"
        "  movs r3, #152\n"
        "  lsls r3, r3, #1\n"
        "  adds r1, r7, r3\n"
        "  ldr r1, [r1, #0]\n"
        "  movs r5, #154\n"
        "  lsls r5, r5, #1\n"
        "  adds r2, r7, r5\n"
        "  ldr r2, [r2, #0]\n"
        "  movs r3, #112\n"
        "  str r3, [sp, #0]\n"
        "  movs r3, #10\n"
        "  str r3, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  mov r3, r8\n"
        "  str r3, [sp, #12]\n"
        "  str r3, [sp, #16]\n"
        "  movs r3, #20\n"
        "  bl sub_08007C68\n"
        ".Lmf22c_0800f572:\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #240\n"
        "  movs r1, #15\n"
        "  bl sub_0800D97C\n"
        "  adds r0, r7, #0\n"
        "  adds r0, #40\n"
        "  bl sub_0800DBE8\n"
        "  adds r0, r7, #0\n"
        "  bl sub_0800F59C\n"
        "  add sp, #24\n"
        "  pop {r3, r4, r5}\n"
        "  mov r8, r3\n"
        "  mov r9, r4\n"
        "  mov sl, r5\n"
        "  pop {r4, r5, r6, r7}\n"
        "  pop {r0}\n"
        "  bx r0\n"
        "  .2byte 0x0000\n"
        ".syntax divided\n"
    );
}

void _0800F22C(void *c) __attribute__((alias("MenuF22C_0800F22C")));
void sub_0800F22C(void *c) __attribute__((alias("MenuF22C_0800F22C")));

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
void _0800F5A0(volatile u8 *a,u32 b,u32 c,u32 d) __attribute__((alias("MenuF5A0_0800F5A0")));

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
void _0800ED98(void *c) __attribute__((alias("MenuED98_0800ED98")));

// EE00: broadcast tick — six event writes plus the state transition tail.
__attribute__((naked)) void MenuEE00_0800EE00(void *rec, u32 cmd) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r9\n"
        "mov r6, r8\n"
        "push {r6, r7}\n"
        "adds r5, r0, #0\n"
        "lsls r2, r2, #16\n"
        "lsrs r7, r2, #16\n"
        "adds r4, r5, #0\n"
        "adds r4, #224\n"
        "movs r1, #0\n"
        "ldrsh r0, [r4, r1]\n"
        "mov r9, r0\n"
        "movs r2, #236\n"
        "adds r2, r2, r5\n"
        "mov r8, r2\n"
        "ldrh r1, [r2, #0]\n"
        "movs r0, #1\n"
        "bl 0x08002158\n"
        "movs r0, #0\n"
        "adds r1, r7, #0\n"
        "bl 0x08002158\n"
        "ldrh r1, [r4, #0]\n"
        "movs r0, #2\n"
        "bl 0x08002158\n"
        "adds r6, r5, #0\n"
        "adds r6, #232\n"
        "ldrh r1, [r6, #0]\n"
        "movs r0, #3\n"
        "bl 0x08002158\n"
        "adds r4, #26\n"
        "ldrh r1, [r4, #0]\n"
        "movs r0, #4\n"
        "bl 0x08002158\n"
        "ldr r2, 1f\n"
        "movs r0, #0\n"
        "ldrsh r1, [r4, r0]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r2\n"
        "adds r0, #49\n"
        "movs r1, #0\n"
        "ldrsb r1, [r0, r1]\n"
        "lsls r1, r1, #16\n"
        "lsrs r1, r1, #16\n"
        "movs r0, #5\n"
        "bl 0x08002158\n"
        "bl 0x08004b68\n"
        "ldrh r1, [r0, #0]\n"
        "movs r0, #6\n"
        "bl 0x08002158\n"
        "cmp r7, #2\n"
        "bne.n .Lee00_after_first\n"
        "ldrh r1, [r6, #0]\n"
        "cmp r1, #4\n"
        "beq.n .Lee00_after_first\n"
        "movs r0, #5\n"
        "strh r0, [r6, #0]\n"
        "mov r2, r8\n"
        "strh r7, [r2, #0]\n"
        ".Lee00_after_first:\n"
        "adds r6, r5, #0\n"
        "adds r6, #224\n"
        "movs r0, #236\n"
        "adds r0, r0, r5\n"
        "mov r8, r0\n"
        "cmp r7, #1\n"
        "bne.n .Lee00_case_done\n"
        "movs r1, #0\n"
        "ldrsh r0, [r6, r1]\n"
        "cmp r0, #0\n"
        "bne.n .Lee00_case1_nonzero\n"
        "movs r0, #1\n"
        "movs r1, #1\n"
        "bl 0x08002618\n"
        "movs r2, #133\n"
        "lsls r2, r2, #1\n"
        "adds r0, r5, r2\n"
        "strh r7, [r0, #0]\n"
        "adds r1, r5, #0\n"
        "adds r1, #232\n"
        "movs r0, #4\n"
        "strh r0, [r1, #0]\n"
        "mov r0, r8\n"
        "strh r7, [r0, #0]\n"
        "b.n .Lee00_case_done\n"
        ".align 2, 0\n"
        "1: .word 0x03001780\n"
        ".Lee00_case1_nonzero:\n"
        "movs r0, #1\n"
        "movs r1, #0\n"
        "bl 0x08002618\n"
        "movs r1, #133\n"
        "lsls r1, r1, #1\n"
        "adds r0, r5, r1\n"
        "movs r4, #0\n"
        "strh r4, [r0, #0]\n"
        "movs r0, #1\n"
        "bl 0x0802b368\n"
        "strh r4, [r5, #44]\n"
        "adds r0, r5, #0\n"
        "adds r0, #148\n"
        "strh r7, [r0, #0]\n"
        "str r7, [r5, #68]\n"
        "adds r0, #84\n"
        "movs r4, #8\n"
        "strh r4, [r0, #0]\n"
        "movs r2, #0\n"
        "ldrsh r0, [r6, r2]\n"
        "bl 0x08004bfc\n"
        "ldrh r0, [r6, #0]\n"
        "cmp r0, #3\n"
        "bne.n .Lee00_case_done\n"
        "strh r4, [r5, #48]\n"
        ".Lee00_case_done:\n"
        "mov r1, r8\n"
        "ldrh r1, [r1, #0]\n"
        "cmp r1, #1\n"
        "beq.n .Lee00_clamp\n"
        "cmp r7, #64\n"
        "bne.n .Lee00_check128\n"
        "movs r0, #2\n"
        "bl 0x0802b368\n"
        "ldrh r0, [r6, #0]\n"
        "subs r0, #1\n"
        "strh r0, [r6, #0]\n"
        ".Lee00_check128:\n"
        "cmp r7, #128\n"
        "bne.n .Lee00_clamp\n"
        "movs r0, #2\n"
        "bl 0x0802b368\n"
        "ldrh r0, [r6, #0]\n"
        "adds r0, #1\n"
        "strh r0, [r6, #0]\n"
        ".Lee00_clamp:\n"
        "adds r1, r6, #0\n"
        "movs r2, #0\n"
        "ldrsh r0, [r1, r2]\n"
        "cmp r0, #0\n"
        "bgt.n .Lee00_clamp_high\n"
        "movs r0, #0\n"
        "strh r0, [r1, #0]\n"
        ".Lee00_clamp_high:\n"
        "movs r2, #0\n"
        "ldrsh r0, [r1, r2]\n"
        "cmp r0, #1\n"
        "ble.n .Lee00_compare\n"
        "movs r0, #2\n"
        "strh r0, [r1, #0]\n"
        ".Lee00_compare:\n"
        "movs r1, #0\n"
        "ldrsh r0, [r6, r1]\n"
        "cmp r9, r0\n"
        "beq.n .Lee00_done\n"
        "movs r0, #2\n"
        "bl 0x0802b368\n"
        ".Lee00_done:\n"
        "pop {r3, r4}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".syntax divided\n"
    );
}
void _0800EE00(void *a,u32 b) __attribute__((alias("MenuEE00_0800EE00")));

__attribute__((naked)) void MenuEF54_0800EF54(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "adds r5, r0, #0\n"
        "adds r1, r5, #0\n"
        "adds r1, #236\n"
        "movs r2, #0\n"
        "ldrsh r0, [r1, r2]\n"
        "cmp r0, #1\n"
        "beq.n .Lef54_case1\n"
        "cmp r0, #1\n"
        "bgt.n .Lef54_case2_test\n"
        "cmp r0, #0\n"
        "beq.n .Lef54_write2\n"
        "b.n .Lef54_done\n"
        ".Lef54_case2_test:\n"
        "cmp r0, #2\n"
        "beq.n .Lef54_case2\n"
        "b.n .Lef54_done\n"
        ".Lef54_case1:\n"
        "movs r0, #0\n"
        "movs r1, #3\n"
        "bl 0x08002178\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #4\n"
        "bne.n .Lef54_done\n"
        "movs r0, #1\n"
        "movs r1, #3\n"
        "bl 0x08002178\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #4\n"
        "bne.n .Lef54_done\n"
        "adds r0, r5, #0\n"
        "bl 0x0800ed98\n"
        "b.n .Lef54_done\n"
        ".Lef54_case2:\n"
        "ldr r0, 1f\n"
        "ldr r1, 2f\n"
        "adds r0, r0, r1\n"
        "ldrb r0, [r0, #0]\n"
        "cmp r0, #1\n"
        "bne.n .Lef54_case2_tail\n"
        "movs r0, #0\n"
        "movs r1, #3\n"
        "bl 0x08002178\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #5\n"
        "bne.n .Lef54_case2_after\n"
        "movs r0, #1\n"
        "movs r1, #3\n"
        "bl 0x08002178\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #5\n"
        "bne.n .Lef54_case2_after\n"
        "adds r0, r5, #0\n"
        "bl 0x0800ed68\n"
        ".Lef54_case2_after:\n"
        "movs r0, #1\n"
        "movs r1, #6\n"
        "bl 0x08002178\n"
        "adds r4, r0, #0\n"
        "lsls r4, r4, #16\n"
        "lsrs r4, r4, #16\n"
        "bl 0x08004b68\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "cmp r4, r0\n"
        "bne.n .Lef54_store2\n"
        "movs r0, #1\n"
        "movs r1, #3\n"
        "bl 0x08002178\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #4\n"
        "bne.n .Lef54_done\n"
        ".Lef54_store2:\n"
        "adds r1, r5, #0\n"
        "adds r1, #232\n"
        ".Lef54_write2:\n"
        "movs r0, #2\n"
        "strh r0, [r1, #0]\n"
        "b.n .Lef54_done\n"
        ".align 2, 0\n"
        "1: .word 0x03001780\n"
        "2: .word 0x000010c3\n"
        ".Lef54_case2_tail:\n"
        "adds r1, r5, #0\n"
        "adds r1, #232\n"
        "movs r0, #5\n"
        "strh r0, [r1, #0]\n"
        "movs r0, #0\n"
        "movs r1, #3\n"
        "bl 0x08002178\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #5\n"
        "bne.n .Lef54_done\n"
        "movs r0, #1\n"
        "movs r1, #3\n"
        "bl 0x08002178\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #5\n"
        "bne.n .Lef54_done\n"
        "adds r0, r5, #0\n"
        "bl 0x0800ed68\n"
        ".Lef54_done:\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".syntax divided\n"
    );
}
void _0800EF54(void *c) __attribute__((alias("MenuEF54_0800EF54")));

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
u32 _0800F6D0(s32 c) __attribute__((alias("MenuF6D0_0800F6D0")));
u32 Sub_0800F6D0(s32 c) __attribute__((alias("MenuF6D0_0800F6D0")));
u32 sub_0800F6D0(s32 c) __attribute__((alias("MenuF6D0_0800F6D0")));

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
u32 _0800F700(s32 a, s32 b) __attribute__((alias("MenuF700_0800F700")));
u32 sub_0800F700(s32 a, s32 b) __attribute__((alias("MenuF700_0800F700")));

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
s16 _0800F744(s32 c) __attribute__((alias("MenuF744_0800F744")));
s16 sub_0800F744(s32 c) __attribute__((alias("MenuF744_0800F744")));

void MenuF778_0800F778(u32 a, u32 b, u32 c) {
    if (c != 0) { extern void sub_08003BC0(u32, u32); sub_08003BC0(a, b); }
    else { extern void sub_08003838(u32, u32, u32); sub_08003838(a, b, 0x0805F990u); }
}
void _0800F778(u32 a, u32 b, u32 c) __attribute__((alias("MenuF778_0800F778")));
void Sub_0800F778(u32 a, u32 b, u32 c) __attribute__((alias("MenuF778_0800F778")));
void sub_0800F778(u32 a, u32 b, u32 c) __attribute__((alias("MenuF778_0800F778")));

void MenuF794_0800F794(void *rec) {
    // Closure spelling: asm/code.s defines _08024BF0 at 0x08024bf0; the
    // sub_08024BF0 spelling is host-only.
    extern void *_08024BF0(void);
    void *p;
    volatile u16 *hp;
    s16 need, have;
    p = _08024BF0();
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
void _0800F794(void *c) __attribute__((alias("MenuF794_0800F794")));
void sub_0800F794(void *c) __attribute__((alias("MenuF794_0800F794")));

void MenuF7C0_0800F7C0(void *rec) {
    extern void *_08024BF0(void);
    void *p;
    volatile u16 *hp;
    s16 need, have;
    p = _08024BF0();
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
void _0800F7C0(void *c) __attribute__((alias("MenuF7C0_0800F7C0")));
void sub_0800F7C0(void *c) __attribute__((alias("MenuF7C0_0800F7C0")));

s16 MenuF7EC_0800F7EC(s32 a, s32 b) {
    uintptr_t base;
    u32 v;
    uintptr_t sh;
    uintptr_t t;
    extern u8 MenuF7ECBase[];
    __asm__(".globl MenuF7ECBase\nMenuF7ECBase = 0x080CB55C\n");
    base = (uintptr_t)MenuF7ECBase;
    v = MenuF700_0800F700((s16)a, (s16)b);
    sh = v << 1;
    t = base + 10;
    return *(s16 *)(uintptr_t)(sh + t);
}
s16 _0800F7EC(s32 a, s32 b) __attribute__((alias("MenuF7EC_0800F7EC")));
s16 sub_0800F7EC(s32 a, s32 b) __attribute__((alias("MenuF7EC_0800F7EC")));

void MenuD280_0800D280(void *unused, void *rec) {
    (void)unused;
    volatile u8 *flag = (volatile u8 *)0x0203EE64u;
    if (*flag == 0) return;
    *flag = 0;
    *(volatile u16 *)((volatile u8 *)rec + 84) = 2;
}
void _0800D280(void *a, void *b) __attribute__((alias("MenuD280_0800D280")));
void sub_0800D280(void *a, void *b) __attribute__((alias("MenuD280_0800D280")));

// Promoted from instruction splices to plain C (2026-10-16), measured on
// the pinned old_agbcc (corpus_match_probe --c89 --require-exact):
//   * the two `add r0, r2, r1` splices are plain pointer arithmetic through
//     the r2-pinned base: `base + 0xFBCu` / `base + 0x10C3u` keep the base
//     in r2 and rematerialise each offset as its own pool word, which is
//     also the ROM's pool order (0x03001780, 0x00000FBC, 0x000010C3).
//   * `*(s16 *)((const u8 *)sceneBase + 2)` reaches the ROM's
//     `movs r1,#2 / ldrsh r0,[r0,r1]` through *extendhisi2_insn: a PLUS
//     with a constant emits `mov SCRATCH,#2; ldrsh DST,[BASE,SCRATCH]`,
//     and the scratch takes the just-freed r1. A volatile halfword would
//     emit `ldrh;lsls;asrs` instead.
//   * the scene==35 / range / else arms each assign outp and value, so the
//     ROM's three duplicated `adds r1,r4,#0 / adds r1,#84` blocks survive
//     (a shared store would tail-merge them) and the `&&` chain lowers to
//     the ROM's `cmp #35 blt / cmp #40 bgt / cmp #39 blt` decision tree.
void MenuF924_0800F924(void *a, void *b) {
    extern void _080056F4(void *s, u32 idx, u32 val);
    extern void *_08004B68(void);
    u8 *r4 = (u8 *)b;
    u8 *r5 = (u8 *)a;
    volatile u8 *globalBase = (volatile u8 *)(uintptr_t)0x03001780u;

    r5 += 216;
    {
        register u32 zero __asm__("r0") = 0;
        *(volatile u16 *)r5 = (u16)zero;
    }
    {
        register volatile u8 *base __asm__("r2") = globalBase;
        volatile u16 *statep = (volatile u16 *)(base + 0xFBCu);
        if (*statep == 3) {
            register volatile u8 *clearp __asm__("r1") = r4 + 88;
            register u8 clearValue __asm__("r0") = 0;
            *clearp = clearValue;
            register u32 flagOffset __asm__("r1") = 0x10C3u;
            register volatile u8 *flagp __asm__("r0") = base + flagOffset;
            if (*flagp == 0) {
                _080056F4(r4, 1, 1);
                *(volatile u16 *)r5 = 1;
            }
        }
    }
    {
        register void *sceneBase __asm__("r0") = _08004B68();
        register s32 scene __asm__("r0") = *(s16 *)((const u8 *)sceneBase + 2);
        register volatile u16 *outp __asm__("r1");
        register u16 value __asm__("r0");
        /* The ROM's dispatch is ONE switch: `cmp #35 beq` (smoothed to
           `bne` over body 5), then the case tree's
           `cmp #35 blt / cmp #40 bgt / cmp #39 blt` all targeting the
           default body. A switch is the only agbcc construct that emits
           `blt` with the ORIGINAL constant: fold-const rewrites every
           `x < C` / `x >= C` tree with positive C into `<=`/`>` with
           C-1, while stmt.c's emit_case_nodes emits the case bounds
           directly. Case 35 must be IN the switch (it makes node 35 the
           tree root, whose single-right-child path emits the leading
           `LT low -> default`); an `if (scene == 35)` ahead of a
           case-39/40-only switch loses it (measured). */
        switch (scene) {
        case 35:
            outp = (volatile u16 *)(r4 + 84);
            value = 5;
            goto store_scene;
        case 39:
        case 40:
            outp = (volatile u16 *)(r4 + 84);
            value = 1;
            goto store_scene;
        default:
            outp = (volatile u16 *)(r4 + 84);
            value = 6;
            goto store_scene;
        }
store_scene:
        *outp = value;
    }
}
__asm__(".pushsection .text.MenuF924_0800F924,\"ax\",%progbits\n\t.short 0\n\t.popsection");
void _0800F924(void *a, void *b) __attribute__((alias("MenuF924_0800F924")));
void sub_0800F924(void *a, void *b) __attribute__((alias("MenuF924_0800F924")));

// menu_fa24.s — record-49 setup (643 B, +0x10C alloc 0x082C1268 template)
__attribute__((naked)) void MenuFA24_0800FA24(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush {r4, r5, r6, r7, lr}\n"
        "\tmov r7, sl\n"
        "\tmov r6, r9\n"
        "\tmov r5, r8\n"
        "\tpush {r5, r6, r7}\n"
        "\tsub sp, #12\n"
        "\tadds r7, r0, #0\n"
        "\tadds r1, r7, #0\n"
        "\tadds r1, #136\n"
        "\tmovs r0, #6\n"
        "\tstr r0, [r1, #0]\n"
        "\tadds r1, #12\n"
        "\tmovs r0, #2\n"
        "\tstr r0, [r1, #0]\n"
        "\tadds r1, #12\n"
        "\tmovs r0, #1\n"
        "\tstr r0, [r1, #0]\n"
        "\tadds r0, r7, #0\n"
        "\tadds r0, #64\n"
        "\tbl sub_0800DAB8\n"
        "\tldr r0, .Lmfa24_0800fd98\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #3\n"
        "\tbl sub_080075E8\n"
        "\tldr r4, .Lmfa24_0800fd9c\n"
        "\tadds r0, r4, #0\n"
        "\tadds r1, r7, #0\n"
        "\tbl sub_0800798C\n"
        "\tadds r0, r7, #0\n"
        "\tbl sub_08007A58\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #4\n"
        "\tbl sub_080075E8\n"
        "\tldr r4, .Lmfa24_0800fda0\n"
        "\tadds r1, r7, #0\n"
        "\tadds r1, #32\n"
        "\tadds r0, r4, #0\n"
        "\tbl sub_0800798C\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #5\n"
        "\tbl sub_080075E8\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #2\n"
        "\tmovs r2, #0\n"
        "\tmovs r3, #6\n"
        "\tbl sub_08007614\n"
        "\tmovs r0, #16\n"
        "\tbl sub_0800572C\n"
        "\tmovs r1, #168\n"
        "\tlsls r1, r1, #1\n"
        "\tadds r1, r1, r7\n"
        "\tmov sl, r1\n"
        "\tstr r0, [r1, #0]\n"
        "\tmovs r2, #170\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r6, r7, r2\n"
        "\tldr r3, .Lmfa24_0800fda4\n"
        "\tldrh r0, [r3, #0]\n"
        "\tstr r0, [r6, #0]\n"
        "\tmovs r0, #16\n"
        "\tbl sub_0800572C\n"
        "\tmovs r1, #174\n"
        "\tlsls r1, r1, #1\n"
        "\tadds r1, r1, r7\n"
        "\tmov r9, r1\n"
        "\tstr r0, [r1, #0]\n"
        "\tmovs r2, #176\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r5, r7, r2\n"
        "\tldr r3, .Lmfa24_0800fda4\n"
        "\tldrh r0, [r3, #10]\n"
        "\tstr r0, [r5, #0]\n"
        "\tmovs r0, #16\n"
        "\tbl sub_0800572C\n"
        "\tmovs r1, #180\n"
        "\tlsls r1, r1, #1\n"
        "\tadds r1, r1, r7\n"
        "\tmov r8, r1\n"
        "\tstr r0, [r1, #0]\n"
        "\tmovs r2, #182\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r4, r7, r2\n"
        "\tldr r3, .Lmfa24_0800fda4\n"
        "\tldrh r0, [r3, #20]\n"
        "\tstr r0, [r4, #0]\n"
        "\tldr r0, [r7, #36]\n"
        "\tldr r1, [r6, #0]\n"
        "\tmov r3, sl\n"
        "\tldr r2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr r0, [r7, #36]\n"
        "\tldr r1, [r5, #0]\n"
        "\tmov r3, r9\n"
        "\tldr r2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr r0, [r7, #36]\n"
        "\tldr r1, [r4, #0]\n"
        "\tmov r3, r8\n"
        "\tldr r2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadds r0, r7, #0\n"
        "\tadds r0, #214\n"
        "\tstr r0, [sp, #8]\n"
        "\tldrh r1, [r0, #0]\n"
        "\tcmp r1, #1\n"
        "\tbeq .Lmfa24_0800fb90\n"
        "\tmovs r0, #16\n"
        "\tbl sub_0800572C\n"
        "\tmovs r2, #186\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r2, r2, r7\n"
        "\tmov sl, r2\n"
        "\tstr r0, [r2, #0]\n"
        "\tmovs r3, #188\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r6, r7, r3\n"
        "\tldr r1, .Lmfa24_0800fda4\n"
        "\tldrh r0, [r1, #0]\n"
        "\tstr r0, [r6, #0]\n"
        "\tmovs r0, #16\n"
        "\tbl sub_0800572C\n"
        "\tmovs r2, #192\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r2, r2, r7\n"
        "\tmov r9, r2\n"
        "\tstr r0, [r2, #0]\n"
        "\tmovs r3, #194\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r5, r7, r3\n"
        "\tldr r1, .Lmfa24_0800fda4\n"
        "\tldrh r0, [r1, #10]\n"
        "\tstr r0, [r5, #0]\n"
        "\tmovs r0, #16\n"
        "\tbl sub_0800572C\n"
        "\tmovs r2, #198\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r2, r2, r7\n"
        "\tmov r8, r2\n"
        "\tstr r0, [r2, #0]\n"
        "\tmovs r3, #200\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r4, r7, r3\n"
        "\tldr r1, .Lmfa24_0800fda4\n"
        "\tldrh r0, [r1, #20]\n"
        "\tstr r0, [r4, #0]\n"
        "\tldr r0, [r7, #36]\n"
        "\tldr r1, [r6, #0]\n"
        "\tmov r3, sl\n"
        "\tldr r2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr r0, [r7, #36]\n"
        "\tldr r1, [r5, #0]\n"
        "\tmov r3, r9\n"
        "\tldr r2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr r0, [r7, #36]\n"
        "\tldr r1, [r4, #0]\n"
        "\tmov r3, r8\n"
        "\tldr r2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        ".Lmfa24_0800fb90:\n"
        "\tldr r1, [sp, #8]\n"
        "\tldrh r0, [r1, #0]\n"
        "\tcmp r0, #0\n"
        "\tblt .Lmfa24_0800fbc6\n"
        "\tcmp r0, #1\n"
        "\tble .Lmfa24_0800fba4\n"
        "\tcmp r0, #6\n"
        "\tbgt .Lmfa24_0800fbc6\n"
        "\tcmp r0, #3\n"
        "\tblt .Lmfa24_0800fbc6\n"
        ".Lmfa24_0800fba4:\n"
        "\tmovs r0, #16\n"
        "\tbl sub_0800572C\n"
        "\tmovs r2, #162\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r3, r7, r2\n"
        "\tstr r0, [r3, #0]\n"
        "\tmovs r0, #164\n"
        "\tlsls r0, r0, #1\n"
        "\tadds r2, r7, r0\n"
        "\tldr r0, .Lmfa24_0800fda8\n"
        "\tldr r1, [r0, #0]\n"
        "\tstr r1, [r2, #0]\n"
        "\tldr r0, [r7, #36]\n"
        "\tldr r2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        ".Lmfa24_0800fbc6:\n"
        "\tadds r0, r7, #0\n"
        "\tadds r0, #214\n"
        "\tldrh r1, [r0, #0]\n"
        "\tadds r5, r0, #0\n"
        "\tcmp r1, #2\n"
        "\tbeq .Lmfa24_0800fbd6\n"
        "\tcmp r1, #7\n"
        "\tbne .Lmfa24_0800fbf8\n"
        ".Lmfa24_0800fbd6:\n"
        "\tmovs r0, #16\n"
        "\tbl sub_0800572C\n"
        "\tmovs r1, #222\n"
        "\tlsls r1, r1, #1\n"
        "\tadds r2, r7, r1\n"
        "\tstr r0, [r2, #0]\n"
        "\tmovs r3, #224\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r1, r7, r3\n"
        "\tmovs r0, #20\n"
        "\tstr r0, [r1, #0]\n"
        "\tldr r0, [r7, #36]\n"
        "\tldr r2, [r2, #0]\n"
        "\tmovs r1, #20\n"
        "\tbl sub_08007ABC\n"
        ".Lmfa24_0800fbf8:\n"
        "\tldr r0, .Lmfa24_0800fdac\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #12\n"
        "\tbl sub_080075E8\n"
        "\tldr r0, .Lmfa24_0800fdb0\n"
        "\tadds r1, r7, #0\n"
        "\tadds r1, #56\n"
        "\tbl sub_0800798C\n"
        "\tldr r0, .Lmfa24_0800fdb4\n"
        "\tadds r1, r7, #0\n"
        "\tadds r1, #40\n"
        "\tbl sub_0800798C\n"
        "\tmovs r0, #48\n"
        "\tbl sub_0800572C\n"
        "\tmovs r2, #138\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r1, r7, r2\n"
        "\tstr r0, [r1, #0]\n"
        "\tmovs r3, #140\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r0, r7, r3\n"
        "\tmovs r6, #3\n"
        "\tstr r6, [r0, #0]\n"
        "\tldr r0, [r7, #44]\n"
        "\tldr r2, [r1, #0]\n"
        "\tmovs r1, #3\n"
        "\tbl sub_08007ABC\n"
        "\tmovs r0, #48\n"
        "\tbl sub_0800572C\n"
        "\tmovs r2, #144\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r1, r7, r2\n"
        "\tstr r0, [r1, #0]\n"
        "\tmovs r3, #146\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r0, r7, r3\n"
        "\tstr r6, [r0, #0]\n"
        "\tldr r0, [r7, #44]\n"
        "\tldr r2, [r1, #0]\n"
        "\tmovs r1, #3\n"
        "\tbl sub_08007ABC\n"
        "\tldrh r0, [r5, #0]\n"
        "\tcmp r0, #1\n"
        "\tbne .Lmfa24_0800fcbe\n"
        "\tmovs r0, #32\n"
        "\tbl sub_0800572C\n"
        "\tmovs r1, #150\n"
        "\tlsls r1, r1, #1\n"
        "\tadds r2, r7, r1\n"
        "\tstr r0, [r2, #0]\n"
        "\tmovs r3, #152\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r1, r7, r3\n"
        "\tmovs r0, #6\n"
        "\tstr r0, [r1, #0]\n"
        "\tldr r0, [r7, #44]\n"
        "\tldr r2, [r2, #0]\n"
        "\tmovs r1, #6\n"
        "\tbl sub_08007ABC\n"
        "\tmovs r0, #14\n"
        "\tbl sub_0800572C\n"
        "\tmovs r1, #156\n"
        "\tlsls r1, r1, #1\n"
        "\tadds r4, r7, r1\n"
        "\tstr r0, [r4, #0]\n"
        "\tldr r1, .Lmfa24_0800fdb8\n"
        "\tldr r2, .Lmfa24_0800fdbc\n"
        "\tadds r0, r1, r2\n"
        "\tmovs r3, #0\n"
        "\tldrsh r0, [r0, r3]\n"
        "\tadds r2, #68\n"
        "\tadds r1, r1, r2\n"
        "\tmovs r3, #0\n"
        "\tldrsh r1, [r1, r3]\n"
        "\tbl sub_080256BC\n"
        "\tlsls r0, r0, #16\n"
        "\tasrs r0, r0, #16\n"
        "\tbl sub_08024C3C\n"
        "\tadds r1, r0, #0\n"
        "\tmovs r2, #158\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r0, r7, r2\n"
        "\tstr r1, [r0, #0]\n"
        "\tldr r0, [r7, #60]\n"
        "\tldr r2, [r4, #0]\n"
        "\tbl sub_08007ABC\n"
        ".Lmfa24_0800fcbe:\n"
        "\tldr r4, .Lmfa24_0800fdc0\n"
        "\tadds r1, r7, #0\n"
        "\tadds r1, #24\n"
        "\tadds r0, r4, #0\n"
        "\tbl sub_0800798C\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #10\n"
        "\tbl sub_080075E8\n"
        "\tmovs r0, #12\n"
        "\tbl sub_0800572C\n"
        "\tadds r4, r7, #0\n"
        "\tadds r4, #228\n"
        "\tstr r0, [r4, #0]\n"
        "\tmovs r0, #0\n"
        "\tbl sub_080258A8\n"
        "\tadds r1, r0, #0\n"
        "\tadds r0, r7, #0\n"
        "\tadds r0, #232\n"
        "\tlsls r1, r1, #16\n"
        "\tasrs r1, r1, #16\n"
        "\tstr r1, [r0, #0]\n"
        "\tldr r0, [r7, #28]\n"
        "\tldr r2, [r4, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr r4, .Lmfa24_0800fdc4\n"
        "\tadds r1, r7, #0\n"
        "\tadds r1, #16\n"
        "\tadds r0, r4, #0\n"
        "\tbl sub_0800798C\n"
        "\tmovs r0, #32\n"
        "\tbl sub_0800572C\n"
        "\tadds r2, r7, #0\n"
        "\tadds r2, #252\n"
        "\tstr r0, [r2, #0]\n"
        "\tmovs r3, #128\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r1, r7, r3\n"
        "\tmovs r0, #2\n"
        "\tstr r0, [r1, #0]\n"
        "\tldr r0, [r7, #20]\n"
        "\tldr r2, [r2, #0]\n"
        "\tmovs r1, #2\n"
        "\tbl sub_08007ABC\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #1\n"
        "\tmovs r2, #8\n"
        "\tbl sub_080075E8\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #9\n"
        "\tbl sub_080075E8\n"
        "\tldrh r5, [r5, #0]\n"
        "\tcmp r5, #1\n"
        "\tbne .Lmfa24_0800fd42\n"
        "\tb .Lmfa24_0800fe4c\n"
        ".Lmfa24_0800fd42:\n"
        "\tldr r5, .Lmfa24_0800fdc8\n"
        "\tadds r1, r7, #0\n"
        "\tadds r1, #48\n"
        "\tadds r0, r5, #0\n"
        "\tbl sub_0800798C\n"
        "\tadds r0, r5, #0\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #11\n"
        "\tbl sub_080075E8\n"
        "\tldr r0, .Lmfa24_0800fdb8\n"
        "\tldr r1, .Lmfa24_0800fdcc\n"
        "\tadds r0, r0, r1\n"
        "\tldrb r0, [r0, #0]\n"
        "\tcmp r0, #1\n"
        "\tbne .Lmfa24_0800fdd0\n"
        "\tmovs r2, #204\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r4, r7, r2\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #11\n"
        "\tbl 0x08025BC8\n"
        "\tldr r0, [r7, #52]\n"
        "\tmovs r3, #206\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r1, r7, r3\n"
        "\tldr r1, [r1, #0]\n"
        "\tldr r2, [r4, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tmovs r0, #0\n"
        "\tstr r0, [sp, #0]\n"
        "\tstr r6, [sp, #4]\n"
        "\tmovs r0, #1\n"
        "\tadds r1, r5, #0\n"
        "\tmovs r2, #1\n"
        "\tmovs r3, #0\n"
        "\tbl sub_08007770\n"
        "\tb .Lmfa24_0800fe08\n"
        "\t.align 2, 0\n"
        ".Lmfa24_0800fd98: .word 0x082A798C\n"
        ".Lmfa24_0800fd9c: .word 0x082D9EF8\n"
        ".Lmfa24_0800fda0: .word 0x082D7660\n"
        ".Lmfa24_0800fda4: .word 0x080CB55C\n"
        ".Lmfa24_0800fda8: .word 0x080CB57C\n"
        ".Lmfa24_0800fdac: .word 0x082C4228\n"
        ".Lmfa24_0800fdb0: .word 0x082C5040\n"
        ".Lmfa24_0800fdb4: .word 0x082DD8FC\n"
        ".Lmfa24_0800fdb8: .word 0x03001780\n"
        ".Lmfa24_0800fdbc: .word 0x00000FF6\n"
        ".Lmfa24_0800fdc0: .word 0x082DFBDC\n"
        ".Lmfa24_0800fdc4: .word 0x082E4FCC\n"
        ".Lmfa24_0800fdc8: .word 0x082B7410\n"
        ".Lmfa24_0800fdcc: .word 0x000010C3\n"
        ".Lmfa24_0800fdd0:\n"
        "\tmovs r0, #204\n"
        "\tlsls r0, r0, #1\n"
        "\tadds r4, r7, r0\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #5\n"
        "\tbl 0x08025BC8\n"
        "\tldr r0, [r7, #52]\n"
        "\tmovs r2, #206\n"
        "\tlsls r2, r2, #1\n"
        "\tadds r1, r7, r2\n"
        "\tldr r1, [r1, #0]\n"
        "\tldr r2, [r4, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tmovs r0, #0\n"
        "\tstr r0, [sp, #0]\n"
        "\tstr r6, [sp, #4]\n"
        "\tmovs r0, #1\n"
        "\tadds r1, r5, #0\n"
        "\tmovs r2, #2\n"
        "\tmovs r3, #0\n"
        "\tbl sub_08007770\n"
        "\tadds r1, r7, #0\n"
        "\tadds r1, #216\n"
        "\tmovs r0, #1\n"
        "\tstrh r0, [r1, #0]\n"
        ".Lmfa24_0800fe08:\n"
        "\tmovs r0, #8\n"
        "\tbl sub_0800572C\n"
        "\tmovs r3, #210\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r2, r7, r3\n"
        "\tstr r0, [r2, #0]\n"
        "\tmovs r0, #212\n"
        "\tlsls r0, r0, #1\n"
        "\tadds r1, r7, r0\n"
        "\tmovs r0, #22\n"
        "\tstr r0, [r1, #0]\n"
        "\tldr r0, [r7, #52]\n"
        "\tldr r2, [r2, #0]\n"
        "\tmovs r1, #22\n"
        "\tbl sub_08007ABC\n"
        "\tmovs r0, #8\n"
        "\tbl sub_0800572C\n"
        "\tmovs r1, #216\n"
        "\tlsls r1, r1, #1\n"
        "\tadds r2, r7, r1\n"
        "\tstr r0, [r2, #0]\n"
        "\tmovs r3, #218\n"
        "\tlsls r3, r3, #1\n"
        "\tadds r1, r7, r3\n"
        "\tmovs r0, #21\n"
        "\tstr r0, [r1, #0]\n"
        "\tldr r0, [r7, #52]\n"
        "\tldr r2, [r2, #0]\n"
        "\tmovs r1, #21\n"
        "\tbl sub_08007ABC\n"
        ".Lmfa24_0800fe4c:\n"
        "\tmovs r0, #0\n"
        "\tstr r0, [r7, #64]\n"
        "\tadds r0, r7, #0\n"
        "\tadds r0, #68\n"
        "\tmovs r4, #1\n"
        "\tstrh r4, [r0, #0]\n"
        "\tadds r0, #16\n"
        "\tmovs r2, #32\n"
        "\tnegs r2, r2\n"
        "\tmovs r1, #0\n"
        "\tbl sub_0800D77C\n"
        "\tadds r0, r7, #0\n"
        "\tadds r0, #76\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #160\n"
        "\tbl sub_0800D77C\n"
        "\tadds r0, r7, #0\n"
        "\tadds r0, #72\n"
        "\tstrh r4, [r0, #0]\n"
        "\tadds r1, r7, #0\n"
        "\tadds r1, #74\n"
        "\tmovs r0, #5\n"
        "\tstrh r0, [r1, #0]\n"
        "\tadd sp, #12\n"
        "\tpop {r3, r4, r5}\n"
        "\tmov r8, r3\n"
        "\tmov r9, r4\n"
        "\tmov sl, r5\n"
        "\tpop {r4, r5, r6, r7}\n"
        "\tpop {r0}\n"
        "\tbx r0\n"
        "\t.align 2, 0\n"
        ".syntax divided\n"
    );
}

void _0800FA24(void *c) __attribute__((alias("MenuFA24_0800FA24")));

// menu_ff78.s bodies beyond entry — one bounded body (010088) as example
__attribute__((naked)) void MenuFF78_Body_080010088(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, sl\n"
        "\tmov\tr6, r9\n"
        "\tmov\tr5, r8\n"
        "\tpush\t{r5, r6, r7}\n"
        "\tsub\tsp, #4\n"
        "\tadds\tr7, r0, #0\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #168\n"
        "\tmovs\tr0, #3\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tldr r5, .Lff10088_0800101e8\n"
        "\tldr r0, .Lff10088_0800101ec\n"
        "\tadds\tr6, r5, r0\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r6, r1]\n"
        "\tldr r2, .Lff10088_0800101f0\n"
        "\tadds\tr4, r5, r2\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r4, r3]\n"
        "\tbl 0x080256D8\n"
        "\tmovs\tr1, #172\n"
        "\tadds\tr1, r1, r7\n"
        "\tmov\tr8, r1\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r6, r2]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r4, r3]\n"
        "\tbl 0x080256F4\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #190\n"
        "\tstr\tr1, [sp, #0]\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r6, r2]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r4, r3]\n"
        "\tbl 0x0802572C\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #188\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r6, r1]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r4, r2]\n"
        "\tbl 0x08025710\n"
        "\tmovs\tr3, #192\n"
        "\tadds\tr3, r3, r7\n"
        "\tmov\tsl, r3\n"
        "\tstrh\tr0, [r3, #0]\n"
        "\tldr r0, .Lff10088_0800101f4\n"
        ".Lff10088_0800100f8: .word 0x2100182D\n"
        "\tldrsh\tr0, [r5, r1]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r6, r2]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr2, [r4, r3]\n"
        "\tbl 0x08025CF4\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #194\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr0, #140\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr4, r7, r0\n"
        "\tmovs\tr0, #4\n"
        "\tstr\tr0, [r4, #0]\n"
        "\tmovs\tr2, #146\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr2, r2, r7\n"
        "\tmov\tr9, r2\n"
        "\tmovs\tr0, #2\n"
        "\tstr\tr0, [r2, #0]\n"
        "\tldrh\tr1, [r1, #0]\n"
        "\tsubs\tr1, #1\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tadds\tr0, r7, #0\n"
        "\tbl sub_0800F8B4\n"
        "\tmov\tr3, r8\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r3, r1]\n"
        "\tbl 0x080258B8\n"
        "\tmovs\tr2, #170\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr5, r7, r2\n"
        "\tldr r2, .Lff10088_0800101f8\n"
        "\tlsls\tr0, r0, #16\n"
        "\tasrs\tr0, r0, #15\n"
        "\tadds\tr0, r0, r2\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstr\tr0, [r5, #0]\n"
        "\tmovs\tr3, #176\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr6, r7, r3\n"
        "\tldr\tr1, [sp, #0]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r1, r3]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr1, r2, #0\n"
        "\tadds\tr1, #10\n"
        "\tadds\tr0, r0, r1\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstr\tr0, [r6, #0]\n"
        "\tmovs\tr0, #182\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r0, r7\n"
        "\tmov\tr8, r0\n"
        "\tmov\tr1, sl\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r1, r3]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr2, #20\n"
        "\tadds\tr0, r0, r2\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tmov\tr1, r8\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tldr\tr0, [r7, #44]\n"
        "\tldr\tr1, [r4, #0]\n"
        "\tmovs\tr3, #138\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r7, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #44]\n"
        "\tmov\tr2, r9\n"
        "\tldr\tr1, [r2, #0]\n"
        "\tmovs\tr3, #144\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r7, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #36]\n"
        "\tldr\tr1, [r5, #0]\n"
        "\tmovs\tr3, #168\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r7, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #36]\n"
        "\tldr\tr1, [r6, #0]\n"
        "\tmovs\tr3, #174\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r7, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #36]\n"
        "\tmov\tr2, r8\n"
        "\tldr\tr1, [r2, #0]\n"
        "\tmovs\tr3, #180\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r7, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadd\tsp, #4\n"
        "\tpop\t{r3, r4, r5}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tmov\tsl, r5\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".Lff10088_0800101e8: .word 0x03001780\n"
        ".Lff10088_0800101ec: .word 0x00000FF6\n"
        ".Lff10088_0800101f0: .word 0x0000103A\n"
        ".Lff10088_0800101f4: .word 0x00000FF2\n"
        ".Lff10088_0800101f8: .word 0x080CB55C\n"
        ".syntax divided\n"
    );
}

void _080010088(void *c) __attribute__((alias("MenuFF78_Body_080010088")));
void sub_080010088(void *c) __attribute__((alias("MenuFF78_Body_080010088")));

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
void _0800BE20(void *c) __attribute__((alias("MenuRecordApply_0800BE20")));
void sub_0800BE20(void *c) __attribute__((alias("MenuRecordApply_0800BE20")));

// menu_record_update.s 0x0800BCD4 — high-reg spill (ip/r8), pools 0x001BB0A9 etc., direct branch + helper ABI
extern void sub_0800BA3C(void *a, void *b, void *c);
void MenuRecordUpdate_0800BCD4(void *a0, u32 a1, void *a2, u32 a3, void *sp24, void *sp28, void *sp32, void *sp36) {
    // Two assembler-resolved symbols keep the table base and its offset as
    // two separate pool words; folding the constants would emit one summed
    // word (0x03002868) and drop the runtime `adds r2, r0, r6`.
    extern u8 BcdWaBase[];
    extern u8 BcdWaOff10E8[];
    __asm__(".globl BcdWaBase\nBcdWaBase = 0x03001780\n"
            ".globl BcdWaOff10E8\nBcdWaOff10E8 = 0x10E8\n");
    const u32 C = 0x001BB0A9u;
    register void *arg0 __asm__("r12") = a0;
    register u32 *bestOut __asm__("r5") = (u32 *)a2;
    register u16 *indexOut __asm__("r8") = (u16 *)a3;
    u32 *currentPtr = (u32 *)sp36;
    register u32 best __asm__("r3") = C;
    register u16 bestIdx __asm__("r4") = 0;
    register s32 i __asm__("r1") = 0;
    register volatile u8 *tableBase __asm__("r0") = (volatile u8 *)BcdWaBase;
    register u32 tableOffset __asm__("r6") = (u32)(uintptr_t)BcdWaOff10E8;
    register volatile u32 *table __asm__("r2");
    table = (volatile u32 *)(uintptr_t)
        ((uintptr_t)tableBase + (uintptr_t)tableOffset);
    do {
        register u32 value __asm__("r0") = *table;
        if (value < best && value != 0) {
            best = value;
            {
                // The ROM's `lsls r0, r1, #16 / lsrs r4, r0, #16` needs an
                // intermediate that is NOT coalesced with bestIdx: a plain
                // `bestIdx = i` sinks the truncation into `strh`, and an
                // unpinned intermediate coalesces into r4. Pinning it to the
                // just-dead value register r0 reproduces the pair (measured
                // in build/scratch_trunc*.c).
                register u32 sh __asm__("r0") = (u32)i << 16;
                bestIdx = sh >> 16;
                __asm__ volatile ("" : "+r" (bestIdx));
            }
        }
        table++;
        i++;
    } while (i <= 2);
    register u32 current __asm__("r0") = *currentPtr;
    if ((s32)best < (s32)current && best != C) {
        *currentPtr = best;
        *(u32 *)sp28 = 1;
    }
    *bestOut = best;
    *indexOut = bestIdx;
    sub_0800BA3C(arg0, sp32, sp24);
    (void)a1;
}
void _0800BCD4(void *a0, u32 a1, void *a2, u32 a3, void *s1, void *s2, void *s3, void *s4) __attribute__((alias("MenuRecordUpdate_0800BCD4")));
void sub_0800BCD4(void *a0, u32 a1, void *a2, u32 a3, void *s1, void *s2, void *s3, void *s4) __attribute__((alias("MenuRecordUpdate_0800BCD4")));

// menu_setup.s 0x0800BC08 — 4-arg leaf, pools 0x03001780 etc., exact s16/u16/u32 widths
__attribute__((naked)) void MenuSetup_0800BC08(void *a0, void *a1, void *a2, void *a3) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "adds r5, r1, #0\n"
        "adds r4, r2, #0\n"
        "adds r6, r3, #0\n"
        "ldr r3, .Lbc08_wa\n"
        "ldr r2, .Lbc08_fc8\n"
        "adds r1, r3, r2\n"
        "movs r2, #0\n"
        "strh r2, [r1, #0]\n"
        "ldr r7, .Lbc08_fca\n"
        "adds r1, r3, r7\n"
        "strh r2, [r1, #0]\n"
        "adds r7, #2\n"
        "adds r1, r3, r7\n"
        "strh r2, [r1, #0]\n"
        "lsls r1, r5, #3\n"
        "adds r1, r1, r5\n"
        "lsls r1, r1, #3\n"
        "ldr r7, .Lbc08_5e4\n"
        "adds r2, r3, r7\n"
        "adds r1, r1, r2\n"
        "ldr r1, [r1, #0]\n"
        "str r1, [r0, #0]\n"
        "ldr r1, .Lbc08_10f4\n"
        "adds r0, r3, r1\n"
        "ldr r0, [r0, #0]\n"
        "str r0, [r4, #0]\n"
        "ldr r2, .Lbc08_fbc\n"
        "adds r0, r3, r2\n"
        "ldrh r0, [r0, #0]\n"
        "cmp r0, #7\n"
        "bne .Lbc08_b6c\n"
        "ldr r7, .Lbc08_fc2\n"
        "adds r0, r3, r7\n"
        "b .Lbc08_b70\n"
        ".short 0\n"
        ".align 2, 0\n"
        ".Lbc08_wa: .word 0x03001780\n"
        ".Lbc08_fc8: .word 0x00000FC8\n"
        ".Lbc08_fca: .word 0x00000FCA\n"
        ".Lbc08_5e4: .word 0x000005E4\n"
        ".Lbc08_10f4: .word 0x000010F4\n"
        ".Lbc08_fbc: .word 0x00000FBC\n"
        ".Lbc08_fc2: .word 0x00000FC2\n"
        ".Lbc08_b6c:\n"
        "ldr r1, .Lbc08_574\n"
        "adds r0, r3, r1\n"
        ".Lbc08_b70:\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r4, #4]\n"
        "adds r0, r4, #0\n"
        "adds r0, #8\n"
        "ldr r4, .Lbc08_2808\n"
        "adds r1, r4, #0\n"
        "movs r2, #3\n"
        "bl sub_0800D95C\n"
        "lsls r0, r5, #3\n"
        "adds r0, r0, r5\n"
        "lsls r0, r0, #3\n"
        "ldr r2, .Lbc08_f598\n"
        "adds r1, r4, r2\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "str r0, [r6, #0]\n"
        "adds r0, r4, #0\n"
        "subs r0, #204\n"
        "ldrh r0, [r0, #0]\n"
        "cmp r0, #7\n"
        "bne .Lbc08_bb0\n"
        "adds r0, r4, #0\n"
        "subs r0, #198\n"
        "b .Lbc08_bb4\n"
        ".short 0\n"
        ".Lbc08_574: .word 0x00000574\n"
        ".Lbc08_2808: .word 0x03002808\n"
        ".Lbc08_f598: .word 0xFFFFF598\n"
        ".Lbc08_bb0:\n"
        "ldr r7, .Lbc08_f4ec\n"
        "adds r0, r4, r7\n"
        ".Lbc08_bb4:\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r6, #4]\n"
        "adds r0, r6, #0\n"
        "adds r0, #8\n"
        "ldr r1, .Lbc08_2808_2\n"
        "movs r2, #3\n"
        "bl sub_0800D95C\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".short 0\n"
        ".Lbc08_f4ec: .word 0xFFFFF4EC\n"
        ".Lbc08_2808_2: .word 0x03002808\n"
        ".syntax divided\n"
    );
}
void _0800BC08(void *a0, void *a1, void *a2, void *a3) __attribute__((alias("MenuSetup_0800BC08")));
void sub_0800BC08(void *a0, void *a1, void *a2, void *a3) __attribute__((alias("MenuSetup_0800BC08")));

void MenuTick_080012D54(void *rec_) {
    // FINDING : the gate is a TWO-POOL-WORD add, not a folded
    // constant — `ldr r0,=0x03001780; ldr r1,=0x0FBC; adds r0,r0,r1;
    // ldrh r0,[r0] @ 0x08012D58..0x08012D5E`. Folding to `0x03001F3C` emits
    // one 16-bit pool word and loses both ldrs and the adds.
    extern u8 MenuWaBase[] __asm__("MenuWaBase");
    __asm__(".globl MenuWaBase\nMenuWaBase = 0x03001780\n");
    uintptr_t base = (uintptr_t)MenuWaBase;
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
void _080012D54(void *a) __attribute__((alias("MenuTick_080012D54")));
void sub_080012D54(void *a) __attribute__((alias("MenuTick_080012D54")));

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
void _0800D4EC(void *a) __attribute__((alias("MenuD4EC_0800D4EC")));
void sub_0800D4EC(void *a) __attribute__((alias("MenuD4EC_0800D4EC")));

// menu_d4ea.s sub_0800D7C4 — tiny leaf: *(u32 *)rec = 11 (word store)
// Evidence: asm/menu_d4ea.s:430 movs r1,#11 str r1,[r0,#0] bx lr.hword 0x0000 (8 B, VMA 0x0800D7C4–0x0800D7CC)
// Pure Thumb, no literal pool, no branch, width word (str vs strh/ldrb), single pointer arg in r0
void MenuD7C4_0800D7C4(void *rec) {
    *(volatile u32 *)rec = 11;
}
__asm__(".align 2, 0");
void _0800D7C4(void *a) __attribute__((alias("MenuD7C4_0800D7C4")));
void sub_0800D7C4(void *a) __attribute__((alias("MenuD7C4_0800D7C4")));

void Menu1529C_08001529C(void) {
    _0802B234();
}
__asm__(".align 2, 0");
void _08001529C(void) __attribute__((alias("Menu1529C_08001529C")));
void sub_08001529C(void) __attribute__((alias("Menu1529C_08001529C")));
void Sub_08001529C(void) __attribute__((alias("Menu1529C_08001529C")));

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
void _080012C7C(void) __attribute__((alias("Menu12C7C_080012C7C")));
void sub_080012C7C(void) __attribute__((alias("Menu12C7C_080012C7C")));

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
void _080013E4C(void) __attribute__((alias("Menu13E4C_080013E4C")));
void sub_080013E4C(void) __attribute__((alias("Menu13E4C_080013E4C")));
void Sub_080013E4C(void) __attribute__((alias("Menu13E4C_080013E4C")));

// menu_ff78.s sub_080014C70 — tiny direct wrapper: push {r4,lr}; adds r4,r1; bl 0x08004B68; adds r4,#84; movs r0,#6; strh r0,[r4]; pop {r4}; pop {r0}; bx r0 (VMA 0x080014C70–0x080014C82, 16 B, no pool, 1 bl to 0x08004B68, widths Vu16 strh, ABI void(void*,void* rec in r1))
// Evidence: asm/menu_ff78.s:9652-9665, objdump 0x080014C70: b510 1c0c f7ff ff?? 3234 3006 8801 bc10 bc01 4700, no literal pool, no branch, no record-layout struct (rec+84 is opaque +84), no VRAM; next smallest fully bounded after 0x013E4C, complete CFG, callers via subsystem (no direct bl xref, but typed entry present)
void Menu14C70_080014C70(void *a, void *rec) {
    (void)a;
    extern void *sub_08004B68(void);
    (void)sub_08004B68();
    *(volatile u16*)((u8*)rec + 84) = 6;
}
void _080014C70(void *a, void *b) __attribute__((alias("Menu14C70_080014C70")));
void sub_080014C70(void *a, void *b) __attribute__((alias("Menu14C70_080014C70")));
void Sub_080014C70(void *a, void *b) __attribute__((alias("Menu14C70_080014C70")));

// menu_ff78.s sub_080012E5C — tiny direct wrapper: push {lr}; adds r2,r0; ldr r0,=0x082ECAE4; movs r1,#0; movs r3,#9; bl sub_08007614; pop {r0}; bx r0 (VMA 0x080012E5C–0x080012E70, 16 B, pool _080012E70=0x082ECAE4, 1 bl, widths none, ABI void(int in r0))
// Evidence: asm/menu_ff78.s:5872-5882, objdump 0x080012E5C: b500 1c02 4902 2000 2309 f7ff ff?? bc01 4700, pool _080012E70.4byte 0x082ECAE4 at file 0x012E70, no branch, no record-layout (pool is ROM table 0x082ECAE4, not 0x03001780), no VRAM; next smallest after 0x014C70, complete CFG, callers via full-ROM bl scan (1–2 callers, not overlapping)
// r0 is forwarded verbatim into _08007614's *int* third parameter, so the ROM
// caller passes a small value (the only one, menu_ff78_r.c, passes s16[rec+176]).
void Menu12E5C_080012E5C(int v) {
    extern void _08007614(void *a, int b, int c, int d);
    _08007614((void*)0x082ECAE4, 0, v, 9);
}
void _080012E5C(int a) __attribute__((alias("Menu12E5C_080012E5C")));
void sub_080012E5C(int a) __attribute__((alias("Menu12E5C_080012E5C")));
void Sub_080012E5C(int a) __attribute__((alias("Menu12E5C_080012E5C")));

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
void _0800108D4(void *a) __attribute__((alias("Menu108D4_0800108D4")));
void sub_0800108D4(void *a) __attribute__((alias("Menu108D4_0800108D4")));

#define MEN_CALLEE(friendly, closure) closure

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
void _0800108F0(void *a) __attribute__((alias("Menu108F0_0800108F0")));
void sub_0800108F0(void *a) __attribute__((alias("Menu108F0_0800108F0")));

// menu_ff78.s sub_080010914 — tiny direct wrapper: push {r4,r5,lr}; adds r4,r0; lsls r2,#16/lsrs r2,#16; adds r5,r2; adds r0,#168; ldrh r0,[r0]; cmp #3; bne; cmp r2,#2; bne; bl 0108D4; cmp r5,#1; bne; bl 0108F0; pop; bx (VMA 0x080010914–0x080010940, 40 B, no pool, 2 bl to 0108D4/0108F0, widths Vu16 ldrh/normalize, ABI void(void *rec in r0, u16 arg in r2, r1 dummy))
// Evidence: asm/menu_ff78.s:1197-1221, objdump 0x080010914: b530 1c04 b01a 4605 3101 8801 b9?? f7ff ff??..., no literal pool, no branch to VRAM, rec+168 is opaque +168, complete CFG, callers via dispatch (no direct bl xref, but typed entry present) — r2 is arg (lsls r2,#16), r1 is ignored
void Menu010914_080010914(void *rec, u32 dummy, u16 arg) {
    (void)dummy;
    // Keep the second copy of the normalized argument in the callee-saved r5
    // across Menu108D4; the original remains available for the arg == 2 test.
    register u16 a __asm__("r5") = arg;
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
void _080010914(void *a, u32 b, u16 c) __attribute__((alias("Menu010914_080010914")));
void sub_080010914(void *a, u32 b, u16 c) __attribute__((alias("Menu010914_080010914")));

// menu_ff78.s sub_080010940 — same proven wrapper family as 010914: push {r4,r5,lr}; adds r4,r0; lsls r2,#16/lsrs r2,#16; adds r5,r2; adds r0,#168; ldrh r0,[r0]; cmp #3; bne; cmp r2,#2; bne; bl 0108D4; cmp r5,#1; bne; bl 0108F0; pop; bx (VMA 0x080010940–0x08001096C, 40 B, no pool, 2 bl to 0108D4/0108F0, widths Vu16 ldrh/normalize, ABI void(void *rec, u32 dummy, u16 arg))
// Evidence: asm/menu_ff78.s:1222-1246, objdump 0x080010940: b530 1c04 b01a 4605 3101 8801 2803 d1?? 2a02 d1?? f7ff ff?? 2d01 d1?? f7ff ff?? bc30 bc01 4700, no literal pool, no branch to VRAM, rec+168 is opaque +168, complete CFG, callers via dispatch (no direct bl xref, but typed entry present) — same pattern as 010914, fully proven without 0x03001780 pool
void Menu010940_080010940(void *rec, u32 dummy, u16 arg) {
    (void)dummy;
    // Keep the second copy of the normalized argument in the callee-saved r5
    // across Menu108D4; the original remains available for the arg == 2 test.
    register u16 a __asm__("r5") = arg;
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
void _080010940(void *a, u32 b, u16 c) __attribute__((alias("Menu010940_080010940")));
void sub_080010940(void *a, u32 b, u16 c) __attribute__((alias("Menu010940_080010940")));
