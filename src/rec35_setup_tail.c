#include "gba/types.h"
#include <stdint.h>

// Reconstructed from asm/rec35_runtime.s 0x0801845C-0x08018540 (4 funcs, rec35
// award-grid fill + conditional cascade tail). The neighboring 0x08018230-
// 0x080183D8 dispatcher family was reconstructed concurrently into
// src/rec35_runtime.c and is NOT duplicated here.
// Instruction-by-instruction transcription using literal pools and the
// callee spellings that bind exact behavior on ARM (see below).

// ---------------------------------------------------------------------------
// _08001845C: rows 0..3 x cols 0..10: 25C84(0,row,col,1);
//   then 25FF0(0),(1),(2); then slots 0..31: 25DBC(slot,1).
// (Award-grid zone fill; called from the 18540-cascade 0x212 arm.)
extern void Sub_08025C84(int a, int b, int c, int d); // 0x08025C84
extern void sub_08025C84(int a, int b, int c, int d);
extern void Sub_08025FF0(int a); // 0x08025FF0
extern void Sub_08025DBC(int a, int b); // 0x08025DBC
void Rec35_GridFill_1845C(void) {
    int r5, r4;
    for (r5 = 0; r5 <= 3; r5++) {
        for (r4 = 0; r4 <= 10; r4++)
            sub_08025C84(0, r5, r4, 1);
    }
    Sub_08025FF0(0);
    Sub_08025FF0(1);
    Sub_08025FF0(2);
    for (r4 = 0; r4 <= 31; r4++)
        Sub_08025DBC(r4, 1);
}
#ifndef __APPLE__
void _08001845C(void) __attribute__((alias("Rec35_GridFill_1845C")));
#endif

// ---------------------------------------------------------------------------
// _0800184A8: const fills via 25EC0: (0..2,3),(3..7,1).
// (Called from the 18540-cascade 0x242 arm.)
extern void _08025EC0(int a, int b); // 0x08025EC0; asm/ai_grid_more4.s spells it this way
void Rec35_ConstFill_184A8(void) {
    _08025EC0(0, 3);
    _08025EC0(1, 3);
    _08025EC0(2, 3);
    _08025EC0(3, 1);
    _08025EC0(4, 1);
    _08025EC0(5, 1);
    _08025EC0(6, 1);
    _08025EC0(7, 1);
}
// The body is 70 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800184A8(void) __attribute__((alias("Rec35_ConstFill_184A8")));
#endif

// ---------------------------------------------------------------------------
// _080018508: rows 0..3 x cols 0..10: 25C84(0,row,col,3);
//   then u16[0x03001780+0xFF0]=1 (pool 0x03001780 + (255<<4)).
// (Called from the 18540-cascade 580 arm.)
// The pool word is 0x03001780 *unbiased* and the +0xFF0 is added at runtime
// (`movs r1,#255; lsls r1,r1,#4; adds r0,r0,r1`). agbcc folds
// `0x03001780 + 0x0FF0` into a single 0x03002770 constant, so the base has to
// be named through an absolute symbol and indexed as a u16 array, which is what
// reproduces the two-step form.
extern u16 Rec35_WA18[];
void Rec35_GridFill3_18508(void) {
    int r5, r4;
    for (r5 = 0; r5 <= 3; r5++) {
        for (r4 = 0; r4 <= 10; r4++)
            sub_08025C84(0, r5, r4, 3);
    }
    Rec35_WA18[255 * 8] = 1;
    // The defining shim MUST be inside this body. At file scope agbcc emits it
    // ahead of the function's own `.section.text.Rec35_GridFill3_18508`
    // directive, so it lands in the PRECEDING section -- and the per-body splice
    // extracts only the function's section, leaving `Rec35_WA18` undefined at
    // link while the reference inside the spliced body survives. Verified in
    // the generated.s: file scope put the shim at line 85 and the section
    // directive at 89. A label plus an absolute assignment emits no bytes, so
    // it is exempt from the splicer's ownership check for the same reason the
    // byte-free alias case is.
    __asm__(".globl Rec35_WA18\nRec35_WA18 = 0x03001780\n");
}
#ifndef __APPLE__
void _080018508(void) __attribute__((alias("Rec35_GridFill3_18508")));
#endif

// ---------------------------------------------------------------------------
// _080018540: conditional cascade on (u16 r1 & 0x3FF) with bit tests on
// (u16 r2), over the ctx record (r6=r0):
//   04B68(ctx) [discarded]; r4=r1&0x3FF; if (r2) [r6+16]=0;
//   [r6+16]++; if (>150) { 04BFC(1); 04EC0(1); }
//   0x222+bit32 -> 0274C(1)+2B368(1)+18434
//   0x212+bit16 -> 0274C(1)+2B368(1)+1845C+241C8
//   0x242+bit64 -> 0274C(1)+2B368(1)+184A8
//   0x282+bit128 -> 0274C(1)+2B368(1)+184F0
//   580(145<<2)+bit4 -> 0274C(1)+2B368(1)+18508+241C8
//   644(161<<2)+bit4 -> 0274C(1)+2B368(1)+u16[WA+0x1086]=1+0B0BC+
//                       04BFC(0)+04EC0(1)
//   else bit8 -> 2B368(1)+04BFC(0)+04EC0(1)
// Pools: 0x3FF/0x222/0x212/0x242/0x282 immediates, 0x03001780+0x1086.
// Called from the 186F0-dispatcher slot7 (r0=r3, r1/r2 u16-truncated).
extern void *Sub_08004B68(void *a);
extern void _08004BFC(int v);
extern void Sub_08004EC0(int v); // 0x08004EC0
extern void Sub_0802B214(int v);
extern void Sub_0800274C(int a); // 0x0800274C
extern void _080018434(void);
extern void _0800184F0(void);
extern void Sub_080241C8(void); // 0x080241C8
extern void Sub_0800B0BC(void); // 0x0800B0BC
void Rec35_Cascade_18540(void *ctx, int a, int b) {
    volatile u8 *r6 = (volatile u8 *)ctx;
    u32 r4 = (u32)(u16)a;
    u32 r5 = (u32)(u16)b;
    (void)Sub_08004B68(ctx);
    r4 &= 0x3FFu;
    if (r5 != 0)
        *(volatile u16 *)(r6 + 16) = 0;
    u16 n = (u16)(*(volatile u16 *)(r6 + 16) + 1);
    *(volatile u16 *)(r6 + 16) = n;
    if (n > 150) { // (n<<16) > (150<<19), both non-negative
        _08004BFC(1);
        Sub_08004EC0(1);
    }
    if (r4 == 0x222u) {
        if ((r5 & 32u) == 0) return;
        Sub_0800274C(1);
        Sub_0802B214(1);
        _080018434();
    } else if (r4 == 0x212u) {
        if ((r5 & 16u) == 0) return;
        Sub_0800274C(1);
        Sub_0802B214(1);
        Rec35_GridFill_1845C();
        Sub_080241C8();
    } else if (r4 == 0x242u) {
        if ((r5 & 64u) == 0) return;
        Sub_0800274C(1);
        Sub_0802B214(1);
        Rec35_ConstFill_184A8();
    } else if (r4 == 0x282u) {
        if ((r5 & 128u) == 0) return;
        Sub_0800274C(1);
        Sub_0802B214(1);
        _0800184F0();
    } else if (r4 == 580u) { // 145<<2
        if ((r5 & 4u) == 0) return;
        Sub_0800274C(1);
        Sub_0802B214(1);
        Rec35_GridFill3_18508();
        Sub_080241C8();
    } else if (r4 == 644u) { // 161<<2
        if ((r5 & 4u) == 0) return;
        Sub_0800274C(1);
        Sub_0802B214(1);
        *(volatile u16 *)(volatile u8 *)(0x03001780 + 0x1086) = 1;
        Sub_0800B0BC();
        _08004BFC(0);
        Sub_08004EC0(1);
    } else {
        if ((r5 & 8u) == 0) return;
        Sub_0802B214(1);
        _08004BFC(0);
        Sub_08004EC0(1);
    }
}
#ifndef __APPLE__
void _080018540(void *c, int a, int b) __attribute__((alias("Rec35_Cascade_18540")));
#endif
