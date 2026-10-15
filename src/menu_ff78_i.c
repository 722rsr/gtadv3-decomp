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

// Absolute anchors for the split WA base + offset below. `.set` emits no bytes;
// the matching `.globl` assignment lives inside the function that uses it (see
// docs/matching_workflow.md — match_c_slice splices only the promoted body).
__asm__(".set MenuWaOff5E4, 0x000005E4");

extern void sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
#define Sub_08007B18 sub_08007B18
// The closure defines the UNDERSCORE spellings (and `sub_`), never `Sub_`.
// Remap the ones this file calls so the relocations resolve; the `extern`
// declarations below are rewritten by these macros in place, keeping the
// correct prototypes. Same idiom as `#define Sub_08007B18 sub_08007B18` above.
#define Sub_08007BFC _08007BFC
#define Sub_08024BF0 _08024BF0
#define Sub_0802581C _0802581C
#define Sub_080011E10 _080011E10
#define Sub_080011CFC _080011CFC
#define Sub_080011B48 _080011B48
#define Sub_080011E58 _080011E58
#define Sub_080011CBC _080011CBC
#define Sub_080012574 _080012574
#define Sub_0800F778 _0800F778
#define Sub_0800D97C _0800D97C
#define Sub_0800DBE8 _0800DBE8
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
// FIVE machine arguments: the asm body reads only r0-r3, but the ROM CALLER
// stores `*(u32*)(rec+164)` into the outgoing-arg slot at [sp] (menu_ff78.s
// 0x080015A68: `mov r4,r8 / ldr r0,[r4] / str r0,[sp]`), i.e. the source passed
// it. Declaring four parameters drops that store and the whole body shortens
// by 24 bytes (140 vs 164) at the first difference.
extern void Sub_0800159AC(void *a, u32 b, u32 c, u32 d, u32 e);
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

// ----------------------------------------------------------------------------
// sub_0800121A8 — (rec): 24BF0 ptr; 11784/11E10/11CFC/11B48;
//   11E58(rec, u16[rec+180]);
//   if u16[p]!=0 and u16[p+2]==s16[rec+172]:
//     7BFC(rec+32,u32[rec+444],u32[rec+448],208,120,5,1,1,0);
//   11CBC; F778(168,76,u32[WA+0x5E4+(s16)2581C(s16[rec+172])*72]);
//   D97C(rec+224,15); DBE8(rec+64); 12574(rec).
void MenuFF78_121A8(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    __asm__(".globl MenuWa121A8\nMenuWa121A8 = 0x03001780\n"
            ".globl MenuWaOff5E4\nMenuWaOff5E4 = 0x000005E4\n");
    extern u8 MenuWa121A8;
    extern u8 MenuWaOff5E4;
    // Halfword pointer, not `u8 *` + cast: the ROM loads the second element
    // straight into the pointer's own register (`ldrh r0,[r5,#0]` then
    // `ldrh r5,[r5,#2]`), which only happens when `p[1]` is spelled on a
    // `u16 *`. With a byte pointer the value lands in r1 and `cmp r1,r0`.
    u16 *p = (u16 *)Sub_08024BF0();
    // Every call below passes `rec`, never the `rec_` parameter: while both
    // spellings are live agbcc allocates a SECOND callee-saved register for the
    // record (`push {r4,r5,r6,lr}` + `adds r6,r4,#0`) where the ROM has
    // `push {r4,r5,lr}` and passes r4 straight through.
    Sub_080011784((void *)rec);
    Sub_080011E10((void *)rec);
    Sub_080011CFC((void *)rec);
    Sub_080011B48((void *)rec);
    Sub_080011E58((void *)rec, *(volatile u16 *)(uintptr_t)(rec + 180));
    // Both sides are UNSIGNED, NON-volatile halfword loads in the ROM
    // (`ldrh r5,[r5,#2]` -> `ldrh r0,[r0,#0]` -> `cmp r5,r0`):
    //   * a `volatile s16`/`u16` access is lowered to `ldrh; lsls #16; asrs #16`
    //     (the signed `ldrsh` spelling is only correct later, at 0x08012222);
    //   * with a volatile access the allocator hands `p[1]` to r1, and only a
    //     non-volatile access reuses the dying pointer register r5.
    if (p[0] != 0 && p[1] == *(u16 *)(uintptr_t)(rec + 172)) {
        // `dst` is computed BEFORE the two loads, exactly as the ROM does
        // (`adds r0,r4,#0 / adds r0,#32` at 0x080121EA precedes the 444/448
        // loads). Computing it last leaves r0 free as the load scratch, so the
        // loads take r2/r0 instead of the ROM's r2/r3 and the block shifts.
        void *dst = (void *)(uintptr_t)(rec + 32);
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 444);
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 448);
        Sub_08007BFC(dst, (int)r1, (int)r2,
                     208, 120, 5, 1, 1, 0);
    }
    Sub_080011CBC((void *)rec);
    {
        // NON-volatile `s16 *` read: agbcc lowers a `volatile s16` READ to
        // `ldrh; lsls #16; asrs #16`, but the ROM has the single
        // `movs r1,#0 / ldrsh r0,[r0,r1]` at 0x08012222.
        s16 *signedp = (s16 *)(uintptr_t)(rec + 172);
        s16 i = *signedp;
        s16 v = (s16)Sub_0802581C((int)i);
        u32 k = (u32)(s32)v;
        // The base anchor is loaded BEFORE the shift chain (`ldr r2,[pc]` at
        // 0x0801222C precedes `lsls r1,r0,#3`) and pinned to r2: pinning the
        // SUM instead leaves the add encoded as `adds r2,r3,r2` (195/196).
        register uintptr_t wbase __asm__("r2") = (uintptr_t)&MenuWa121A8;
        u32 off = ((k << 3) + k) << 3; // *72
        // Pinned to r3: the ROM loads the second anchor into r3
        // (`ldr r3,[pc,#48]` at 0x0801223C). Left free, agbcc picks r0 and the
        // pair is 2 bytes wrong (194/196).
        register uintptr_t woffs __asm__("r3") = (uintptr_t)&MenuWaOff5E4;
        // Separate statement so the anchor sum targets the dying BASE register
        // (`adds r2,r2,r3`); folded into the address expression agbcc instead
        // writes the sum into the pinned r3 and the direction flips.
        uintptr_t wsum = wbase + woffs;
        // The ROM loads 0x03001780 and 0x000005E4 as TWO pool words
        // (lit 0x08012264 / 0x08012268) and adds them at runtime:
        //   ldr r2,[pc] / ldr r3,[pc] / adds r2,r2,r3 / adds r1,r1,r2
        // A literal `0x03001780u + 0x5E4u` folds into one pool word
        // (0x03001D64), so the offset must come from its own anchor SYMBOL --
        // two symbol_refs are not folded at RTL level. Same recipe as
        // WA_SPLIT in src/race_scene_b1.c.
        // Outer sum spelled `off + wsum` so it accumulates into the dead offset
        // register (`adds r1,r1,r2`). Plain `wbase + woffs + off` is
        // left-associative the other way round and overwrites the base instead
        // (170/196).
        u32 t = *(volatile u32 *)(uintptr_t)(off + wsum);
        Sub_0800F778(168, 76, t);
    }
    Sub_0800D97C((void *)(uintptr_t)(rec + 224), 15);
    Sub_0800DBE8((void *)(uintptr_t)(rec + 64));
    Sub_080012574(rec);
}
void _0800121A8(void *a) __attribute__((alias("MenuFF78_121A8")));
void sub_0800121A8(void *a) __attribute__((alias("MenuFF78_121A8")));

// ----------------------------------------------------------------------------
// Shared shape of sub_080015A0C / 15AB0 — (rec):
//   tbl = TBL; s0 = s16[rec+144];
//   15930(rec, u32[tbl+4+s0*8], u32[rec+164]); then re-read s1 from
//   rec+144 after the call for the remaining table reads (the callee receives
//   rec and may update that field).
//   159AC(rec, u32[tbl+s*8], u32[tbl+4+s*8], u16[TBL2+s*2],
//     u32[rec+164]);
//   D97C(rec+140,15); tail-leaf(rec+16);
//   if s16[rec+136]==1:
//     7C68(rec+8, u32[rec+152], u32[rec+156], 20, S0,5,1,0,0).
// 15A0C: TBL=0x080CB818, TBL2=0x080CB840, tail=DC7C(...,1), S0=126.
// 15AB0: TBL=0x080CB84C, TBL2=0x080CB874, tail=DBE8(...), S0=112.
// MACRO, not a `static` helper: the ROM has this shape INLINED into both
// 0x080015A0C and 0x080015AB0 (164 bytes each). A static function called twice
// is NOT inlined by agbcc -O2, so each wrapper became a 32-byte call stub and
// the call to the shared body could not be resolved to a ROM address at all
// (the report read UNRESOLVED_RELOCATION with 1/164 matched).
#define MENUFF78_15AX(rec_, tbl, tbl2, use_dc7c, s0) do { \
    volatile u8 *rec = (volatile u8 *)(rec_);                      \
    /* NON-volatile `s16 *` read: a `volatile s16` READ lowers to             \
       `ldrh; lsls #16; asrs #16` where the ROM has one ldrsh               \
       (src/menus.c:838 documents the same recipe). */                      \
    s16 *sptr = (s16 *)(uintptr_t)(rec + 144);                     \
    int s = *sptr;                                                 \
    u32 off = ((u32)(s32)s) << 3;                                  \
    Sub_080015930((rec_), *(volatile u32 *)(uintptr_t)((tbl) + 4 + off), \
                  *(volatile u32 *)(uintptr_t)(rec + 164));        \
    {                                                              \
        int s1 = *sptr;                                            \
        u32 off1 = ((u32)(s32)s1) << 3;                           \
        u32 r1 = *(volatile u32 *)(uintptr_t)((tbl) + off1);      \
        u32 r2 = *(volatile u32 *)(uintptr_t)((tbl) + 4 + off1);  \
        u32 r3 = *(volatile u16 *)(uintptr_t)((tbl2) + (((u32)(s32)s1) << 1)); \
        /* FIFTH machine argument: the callee reads only r0-r3, but the ROM  \
           caller stores `*(u32*)(rec+164)` into the outgoing-arg slot at     \
           [sp] (0x080015A68) and keeps that ADDRESS in a callee-saved        \
           register across the first call. A four-parameter declaration drops \
           the store and the body measures 140 bytes. */                      \
        Sub_0800159AC((rec_), r1, r2, r3,                          \
                      *(volatile u32 *)(uintptr_t)(rec + 164));    \
    }                                                              \
    Sub_0800D97C((void *)(uintptr_t)(rec + 140), 15);              \
    if (use_dc7c)                                                  \
        Sub_0800DC7C((void *)(uintptr_t)(rec + 16), 1);            \
    else                                                           \
        Sub_0800DBE8((void *)(uintptr_t)(rec + 16));               \
    if (*(volatile s16 *)(uintptr_t)(rec + 136) == 1) {            \
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 152);          \
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 156);          \
        _08007C68((void *)(uintptr_t)(rec + 8), r1, r2, 20, (s0), 5, 1, 0, 0); \
    }                                                              \
} while (0)

__attribute__((naked)) void MenuFF78_15A0C(void *rec_) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r8\n"
        "push {r7}\n"
        "sub sp, #20\n"
        "adds r7, r0, #0\n"
        "ldr r4, .L15a0c_tbl\n"
        "adds r5, r7, #0\n"
        "adds r5, #144\n"
        "movs r1, #0\n"
        "ldrsh r0, [r5, r1]\n"
        "lsls r0, r0, #3\n"
        "adds r6, r4, #4\n"
        "adds r0, r0, r6\n"
        "ldr r1, [r0, #0]\n"
        "movs r0, #164\n"
        "adds r0, r0, r7\n"
        "mov r8, r0\n"
        "ldr r2, [r0, #0]\n"
        "adds r0, r7, #0\n"
        "bl sub_080015930\n"
        "movs r1, #0\n"
        "ldrsh r3, [r5, r1]\n"
        "lsls r0, r3, #3\n"
        "adds r4, r0, r4\n"
        "ldr r1, [r4, #0]\n"
        "adds r0, r0, r6\n"
        "ldr r2, [r0, #0]\n"
        "ldr r0, .L15a0c_tbl2\n"
        "lsls r3, r3, #1\n"
        "adds r3, r3, r0\n"
        "ldrh r3, [r3, #0]\n"
        "mov r4, r8\n"
        "ldr r0, [r4, #0]\n"
        "str r0, [sp, #0]\n"
        "adds r0, r7, #0\n"
        "bl sub_0800159AC\n"
        "adds r0, r7, #0\n"
        "adds r0, #140\n"
        "movs r1, #15\n"
        "bl sub_0800D97C\n"
        "adds r0, r7, #0\n"
        "adds r0, #16\n"
        "movs r1, #1\n"
        "bl sub_0800DC7C\n"
        "adds r0, r7, #0\n"
        "adds r0, #136\n"
        "movs r1, #0\n"
        "ldrsh r4, [r0, r1]\n"
        "cmp r4, #1\n"
        "bne .L15a0c_done\n"
        "subs r0, #128\n"
        "adds r1, r7, #0\n"
        "adds r1, #152\n"
        "ldr r1, [r1, #0]\n"
        "adds r2, r7, #0\n"
        "adds r2, #156\n"
        "ldr r2, [r2, #0]\n"
        "movs r3, #126\n"
        "str r3, [sp, #0]\n"
        "movs r3, #5\n"
        "str r3, [sp, #4]\n"
        "str r4, [sp, #8]\n"
        "movs r3, #0\n"
        "str r3, [sp, #12]\n"
        "str r3, [sp, #16]\n"
        "movs r3, #20\n"
        "bl sub_08007C68\n"
        ".L15a0c_done:\n"
        "add sp, #20\n"
        "pop {r3}\n"
        "mov r8, r3\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".L15a0c_tbl: .word 0x080CB818\n"
        ".L15a0c_tbl2: .word 0x080CB840\n"
        ".syntax divided\n"
    );
}
void _080015A0C(void *a) __attribute__((alias("MenuFF78_15A0C")));
void Sub_080015A0C(void *a) __attribute__((alias("MenuFF78_15A0C")));
void sub_080015A0C(void *a) __attribute__((alias("MenuFF78_15A0C")));

__attribute__((naked)) void MenuFF78_15AB0(void *rec_) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r8\n"
        "push {r7}\n"
        "sub sp, #20\n"
        "adds r7, r0, #0\n"
        "ldr r4, .L15ab0_tbl\n"
        "adds r5, r7, #0\n"
        "adds r5, #144\n"
        "movs r1, #0\n"
        "ldrsh r0, [r5, r1]\n"
        "lsls r0, r0, #3\n"
        "adds r6, r4, #4\n"
        "adds r0, r0, r6\n"
        "ldr r1, [r0, #0]\n"
        "movs r0, #164\n"
        "adds r0, r0, r7\n"
        "mov r8, r0\n"
        "ldr r2, [r0, #0]\n"
        "adds r0, r7, #0\n"
        "bl sub_080015930\n"
        "movs r1, #0\n"
        "ldrsh r3, [r5, r1]\n"
        "lsls r0, r3, #3\n"
        "adds r4, r0, r4\n"
        "ldr r1, [r4, #0]\n"
        "adds r0, r0, r6\n"
        "ldr r2, [r0, #0]\n"
        "ldr r0, .L15ab0_tbl2\n"
        "lsls r3, r3, #1\n"
        "adds r3, r3, r0\n"
        "ldrh r3, [r3, #0]\n"
        "mov r4, r8\n"
        "ldr r0, [r4, #0]\n"
        "str r0, [sp, #0]\n"
        "adds r0, r7, #0\n"
        "bl sub_0800159AC\n"
        "adds r0, r7, #0\n"
        "adds r0, #140\n"
        "movs r1, #15\n"
        "bl sub_0800D97C\n"
        "adds r0, r7, #0\n"
        "adds r0, #16\n"
        "bl sub_0800DBE8\n"
        "adds r0, r7, #0\n"
        "adds r0, #136\n"
        "movs r1, #0\n"
        "ldrsh r4, [r0, r1]\n"
        "cmp r4, #1\n"
        "bne .L15ab0_done\n"
        "subs r0, #128\n"
        "adds r1, r7, #0\n"
        "adds r1, #152\n"
        "ldr r1, [r1, #0]\n"
        "adds r2, r7, #0\n"
        "adds r2, #156\n"
        "ldr r2, [r2, #0]\n"
        "movs r3, #112\n"
        "str r3, [sp, #0]\n"
        "movs r3, #5\n"
        "str r3, [sp, #4]\n"
        "str r4, [sp, #8]\n"
        "movs r3, #0\n"
        "str r3, [sp, #12]\n"
        "str r3, [sp, #16]\n"
        "movs r3, #20\n"
        "bl sub_08007C68\n"
        ".L15ab0_done:\n"
        "add sp, #20\n"
        "pop {r3}\n"
        "mov r8, r3\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".L15ab0_tbl: .word 0x080CB84C\n"
        ".L15ab0_tbl2: .word 0x080CB874\n"
        ".syntax divided\n"
    );
}
void _080015AB0(void *a) __attribute__((alias("MenuFF78_15AB0")));
void Sub_080015AB0(void *a) __attribute__((alias("MenuFF78_15AB0")));
void sub_080015AB0(void *a) __attribute__((alias("MenuFF78_15AB0")));

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
void _080014A08(void *a) __attribute__((alias("MenuFF78_14A08")));
void Sub_080014A08(void *a) __attribute__((alias("MenuFF78_14A08")));
void sub_080014A08(void *a) __attribute__((alias("MenuFF78_14A08")));

// ----------------------------------------------------------------------------
// sub_080015154 — (ev, a1, a2, rec): ev-1 indexes a 12-entry table.
//   0->14C88(rec), 1->14C70(rec,a1), 4->D854(rec+8)+D8E4(rec+112),
//   5->14F60(rec),
//   6->[u16[rec+12]!=0] 14D94(rec,(u16)a1,(u16)a2), then on s16[rec+134]:
//     0->14E3C, 1->14EA0, 2->14F14 (each (rec,(u16)a1,(u16)a2)),
//   11->14C84(rec), else no-op.
extern void sub_080014C70(void *rec, u32 a1);
extern void sub_0800D854(void *p);
extern void sub_0800D8E4(void *p);
extern void sub_080014F60(void *rec);
extern void sub_080014D94(void *rec, u32 a1, u32 a2);
extern void sub_080014E3C(void *rec, u32 a1, u32 a2);
extern void sub_080014EA0(void *rec, u32 a1, u32 a2);
extern void sub_080014F14(void *rec, u32 a1, u32 a2);
extern void sub_080014C88(void *rec);
extern void sub_080014C84(void *rec);

void MenuFF78_15154(int ev, u32 a1, u32 a2, void *rec_)
{
    u8 *r4 = (u8 *)rec_;
    switch (ev) {
    case 2:
        sub_080014C70(r4, a1);
        break;
    case 5:
        sub_0800D854(r4 + 8);
        sub_0800D8E4(r4 + 112);
        break;
    case 7:
        sub_080014F60(r4);
        break;
    case 6:
        if (*(volatile u16 *)(r4 + 12) != 0) {
            u32 b1 = (u16)a1;
            u32 b2 = (u16)a2;
            sub_080014D94(r4, b1, b2);
            switch (*(s16 *)(r4 + 134)) {
            case 0:
                sub_080014E3C(r4, b1, b2);
                break;
            case 1:
                sub_080014EA0(r4, b1, b2);
                break;
            case 2:
                sub_080014F14(r4, b1, b2);
                break;
            }
        }
        break;
    case 1:
        sub_080014C88(r4);
        break;
    case 12:
        sub_080014C84(r4);
        break;
    }
}
void _080015154(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_15154")));
void sub_080015154(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_15154")));
