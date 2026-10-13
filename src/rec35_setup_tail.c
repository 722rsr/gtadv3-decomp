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
extern void sub_08025FF0(int a); // 0x08025FF0
extern void sub_08025DBC(int a, int b); // 0x08025DBC
void Rec35_GridFill_1845C(void) {
    // The ROM materializes the constant 1 once (`movs r7,#1`) and passes it
    // via `adds r3,r7,#0` / `adds r1,r7,#0` at both use sites; a shared named
    // local keeps it in the callee-saved r7 instead of re-issuing `movs`.
    int one = 1;
    int r5, r4;
    for (r5 = 0; r5 <= 3; r5++) {
        for (r4 = 0; r4 <= 10; r4++)
            sub_08025C84(0, r5, r4, one);
    }
    sub_08025FF0(0);
    sub_08025FF0(1);
    sub_08025FF0(2);
    for (r4 = 0; r4 <= 31; r4++)
        sub_08025DBC(r4, one);
}
// The body is 74 bytes (2 mod 4); gas would close the section with a `c0 46`
// code nop while the ROM holds `00 00`. This file-scope zero-fill `.align`
// lands after the body's `.size`, still inside its own function section, so it
// pads with the 0 fill argument instead. No body byte changes.
__asm__(".align 2, 0");
void _08001845C(void) __attribute__((alias("Rec35_GridFill_1845C")));
// asm/rec35_runtime.s:4116 branches here by the `sub_08001845C` spelling.
void sub_08001845C(void) __attribute__((alias("Rec35_GridFill_1845C")));

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
void _0800184A8(void) __attribute__((alias("Rec35_ConstFill_184A8")));

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
void _080018508(void) __attribute__((alias("Rec35_GridFill3_18508")));

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
__attribute__((naked)) void Rec35_Cascade_18540(void *ctx, int a, int b) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, r6, lr}\n"
        "adds	r6, r0, #0\n"
        "lsls	r1, r1, #16\n"
        "lsrs	r4, r1, #16\n"
        "lsls	r2, r2, #16\n"
        "lsrs	r5, r2, #16\n"
        "bl 0x08004B68\n"
        "ldr r0, .L18540_59c\n"
        "ands	r4, r0\n"
        "cmp	r5, #0\n"
        "beq .L18540_55c\n"
        "movs	r0, #0\n"
        "strh	r0, [r6, #16]\n"
        ".L18540_55c:\n"
        "ldrh	r0, [r6, #16]\n"
        "adds	r0, #1\n"
        "strh	r0, [r6, #16]\n"
        "lsls	r0, r0, #16\n"
        "movs	r1, #150\n"
        "lsls	r1, r1, #19\n"
        "cmp	r0, r1\n"
        "ble .L18540_578\n"
        "movs	r0, #1\n"
        "bl 0x08004BFC\n"
        "movs	r0, #1\n"
        "bl 0x08004EC0\n"
        ".L18540_578:\n"
        "ldr r0, .L18540_5a0\n"
        "cmp	r4, r0\n"
        "bne .L18540_5a4\n"
        "movs	r0, #32\n"
        "ands	r0, r5\n"
        "cmp	r0, #0\n"
        "beq .L18540_5a4\n"
        "movs	r0, #1\n"
        "bl 0x0800274C\n"
        "movs	r0, #1\n"
        "bl 0x0802B368\n"
        "adds	r0, r6, #0\n"
        "bl 0x08018434\n"
        "b .L18540_6a2\n"
        "movs	r0, r0\n"
        ".L18540_59c: .4byte 0x000003FF\n"
        ".L18540_5a0: .4byte 0x00000222\n"
        ".L18540_5a4:\n"
        "ldr r0, .L18540_5cc\n"
        "cmp	r4, r0\n"
        "bne .L18540_5d0\n"
        "movs	r0, #16\n"
        "ands	r0, r5\n"
        "cmp	r0, #0\n"
        "beq .L18540_5d0\n"
        "movs	r0, #1\n"
        "bl 0x0800274C\n"
        "movs	r0, #1\n"
        "bl 0x0802B368\n"
        "adds	r0, r6, #0\n"
        "bl 0x0801845C\n"
        "bl 0x080241C8\n"
        "b .L18540_6a2\n"
        "movs	r0, r0\n"
        ".L18540_5cc: .4byte 0x00000212\n"
        ".L18540_5d0:\n"
        "ldr r0, .L18540_5f4\n"
        "cmp	r4, r0\n"
        "bne .L18540_5f8\n"
        "movs	r0, #64\n"
        "ands	r0, r5\n"
        "cmp	r0, #0\n"
        "beq .L18540_5f8\n"
        "movs	r0, #1\n"
        "bl 0x0800274C\n"
        "movs	r0, #1\n"
        "bl 0x0802B368\n"
        "adds	r0, r6, #0\n"
        "bl 0x080184A8\n"
        "b .L18540_6a2\n"
        "movs	r0, r0\n"
        ".L18540_5f4: .4byte 0x00000242\n"
        ".L18540_5f8:\n"
        "ldr r0, .L18540_61c\n"
        "cmp	r4, r0\n"
        "bne .L18540_620\n"
        "movs	r0, #128\n"
        "ands	r0, r5\n"
        "cmp	r0, #0\n"
        "beq .L18540_620\n"
        "movs	r0, #1\n"
        "bl 0x0800274C\n"
        "movs	r0, #1\n"
        "bl 0x0802B368\n"
        "adds	r0, r6, #0\n"
        "bl 0x080184F0\n"
        "b .L18540_6a2\n"
        "movs	r0, r0\n"
        ".L18540_61c: .4byte 0x00000282\n"
        ".L18540_620:\n"
        "movs	r0, #145\n"
        "lsls	r0, r0, #2\n"
        "cmp	r4, r0\n"
        "bne .L18540_648\n"
        "movs	r0, #4\n"
        "ands	r0, r5\n"
        "cmp	r0, #0\n"
        "beq .L18540_648\n"
        "movs	r0, #1\n"
        "bl 0x0800274C\n"
        "movs	r0, #1\n"
        "bl 0x0802B368\n"
        "adds	r0, r6, #0\n"
        "bl 0x08018508\n"
        "bl 0x080241C8\n"
        "b .L18540_6a2\n"
        ".L18540_648:\n"
        "movs	r0, #161\n"
        "lsls	r0, r0, #2\n"
        "cmp	r4, r0\n"
        "bne .L18540_688\n"
        "movs	r0, #4\n"
        "ands	r0, r5\n"
        "cmp	r0, #0\n"
        "beq .L18540_688\n"
        "movs	r0, #1\n"
        "bl 0x0800274C\n"
        "movs	r0, #1\n"
        "bl 0x0802B368\n"
        "ldr r0, .L18540_680\n"
        "ldr r1, .L18540_684\n"
        "adds	r0, r0, r1\n"
        "movs	r1, #1\n"
        "strh	r1, [r0, #0]\n"
        "bl 0x0800B0BC\n"
        "movs	r0, #0\n"
        "bl 0x08004BFC\n"
        "movs	r0, #1\n"
        "bl 0x08004EC0\n"
        "b .L18540_6a2\n"
        ".L18540_680: .4byte 0x03001780\n"
        ".L18540_684: .4byte 0x00001086\n"
        ".L18540_688:\n"
        "movs	r0, #8\n"
        "ands	r0, r5\n"
        "cmp	r0, #0\n"
        "beq .L18540_6a2\n"
        "movs	r0, #1\n"
        "bl 0x0802B368\n"
        "movs	r0, #0\n"
        "bl 0x08004BFC\n"
        "movs	r0, #1\n"
        "bl 0x08004EC0\n"
        ".L18540_6a2:\n"
        "pop	{r4, r5, r6}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        ".syntax divided\n"
    );
}
void _080018540(void *c, int a, int b) __attribute__((alias("Rec35_Cascade_18540")));
