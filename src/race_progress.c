#include "gtadv/race_progress.h"
#include "gtadv/memory.h"
#include "gba/types.h"

// Reference: asm/race_progress.s 0x0800AD84-0x0800B0BC
// Three routines commit/advance/per-frame tail with WA=0x03001780.
// Evidence: literal pools at _0800AD84_lit_0..7 and _0800AE78 pools, exact halfword/byte widths,
// and calls to sub_08025CF4 and sub_0800279C.
// Contracts use AI lane: Ai_GridGet(type,row,col) returns u8 0..3, Ai_GridSet, EventPost.

// Packed-collection-grid accessors. `_08025CF4` takes (a, b, c) = the values of
// WA+0x0FF2 / WA+0x0FF8 / WA+0x103A and packs lane 44*a + 11*b + c; the setter
// `_08025C84` is 4-arg (same three plus the 2-bit value) — the earlier 3-arg
// `Ai_GridSet` decl here silently dropped the value (see the ROM call signatures).
extern int  Ai_GridGet(int type, int row, int col);   // _08025CF4
extern void _08025C84(int a, int b, int c, int value); // grid set (code_25930.c)
// asm/race_progress.s:87-88 — the ROM loads r0 = 1 immediately before the call
// (`movs r0, #1; bl sub_0800279C`), so the argument is part of the contract.
extern void Event_Post1(int v); // sub_0800279C
extern void CounterClear(void); // _08004CC4 subsystem LIFO reset
extern int  _0800AA20(void);   // shared racer tick (car_tick_dispatch.c)
extern void Ai_RaceFsm(void);  // _0800AA40 per-frame race FSM (ai_race_leaves.c)
extern void MenuProgressUpdate_0BD40(void);            // scene_record_dispatch.c
extern void MenuRecordApply_0800BE20(void *rec);       // menus.c
extern void MenuPlaceUpdate_0BE74(void);               // scene_record_dispatch.c

// _0800AD84 — commit the selected grid cell into the two lane tables.
// MEASURED 2026-10-22: a full C lift reaches 237/244 (candidate length exact)
// and stops on the indexed-halfword *scratch* register at three sites. The ROM
// takes r7 at each (`movs r7,#0` for the 0x0FF2 load's zero index, the second
// 0x0FF6 load's zero index, and the 0x100A base materialisation); agbcc takes
// r4 / r6 / r0 there. The rest of the body, including the 0x103A counter
// derived as `0x0FF2 + 72` (the ROM has no 0x103A pool word) and the
// `row*2 + col*8` accumulation order, reproduces exactly with
//   register u8 *wa asm("r2"), *rowc asm("r3"), *colc asm("r5"), *rowm asm("r6");
//   register u32 colo asm("r6"); register s32 idx asm("r1");
// plus liveness pins ("=r"/"r" bracketing) on r6/r7/r0 around the first
// 0x0FF6 load. Pinning r4 at the 0x0FF2 load or r6/r0 at the second 0x0FF6
// load moves the *destination* instead of the scratch, and pinning r0+r4
// together makes agbcc abort (internal compiler error), so the three r7
// scratches are the wall. See findings/indexed_halfword_scratch.md.
__attribute__((naked)) void Race_CommitCell(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r9\n"
        "mov r6, r8\n"
        "push {r6, r7}\n"
        "ldr r2, .Lad84_lit_0\n"
        "ldr r0, .Lad84_lit_1\n"
        "adds r3, r2, r0\n"
        "movs r4, #0\n"
        "ldrsh r1, [r3, r4]\n"
        "lsls r1, r1, #1\n"
        "ldr r6, .Lad84_lit_2\n"
        "adds r5, r2, r6\n"
        "movs r7, #0\n"
        "ldrsh r0, [r5, r7]\n"
        "lsls r0, r0, #3\n"
        "adds r1, r1, r0\n"
        "ldr r0, .Lad84_lit_3\n"
        "adds r0, r0, r2\n"
        "mov r9, r0\n"
        "add r1, r9\n"
        "adds r6, #72\n"
        "adds r4, r2, r6\n"
        "ldrh r0, [r4, #0]\n"
        "strh r0, [r1, #0]\n"
        "movs r7, #0\n"
        "ldrsh r1, [r3, r7]\n"
        "lsls r1, r1, #1\n"
        "movs r6, #0\n"
        "ldrsh r0, [r5, r6]\n"
        "lsls r0, r0, #3\n"
        "adds r1, r1, r0\n"
        "ldr r7, .Lad84_lit_4\n"
        "adds r7, r7, r2\n"
        "mov r8, r7\n"
        "add r1, r8\n"
        "ldrh r0, [r4, #0]\n"
        "strh r0, [r1, #0]\n"
        "ldrh r0, [r3, #0]\n"
        "ldr r1, .Lad84_lit_5\n"
        "adds r6, r2, r1\n"
        "strh r0, [r6, #0]\n"
        "ldrh r1, [r5, #0]\n"
        "ldr r3, .Lad84_lit_6\n"
        "adds r0, r2, r3\n"
        "strh r1, [r0, #0]\n"
        "ldr r0, .Lad84_lit_7\n"
        "adds r7, r2, r0\n"
        "ldrb r0, [r7, #0]\n"
        "subs r0, #1\n"
        "lsls r0, r0, #24\n"
        "lsrs r0, r0, #24\n"
        "cmp r0, #2\n"
        "bhi .Lad84_done\n"
        "movs r1, #0\n"
        "ldrsh r0, [r5, r1]\n"
        "movs r2, #0\n"
        "ldrsh r1, [r6, r2]\n"
        "movs r3, #0\n"
        "ldrsh r2, [r4, r3]\n"
        "bl _08025CF4\n"
        "lsls r0, r0, #24\n"
        "lsrs r0, r0, #24\n"
        "movs r3, #0\n"
        "ldrsb r3, [r7, r3]\n"
        "cmp r0, r3\n"
        "bge .Lad84_store\n"
        "movs r7, #0\n"
        "ldrsh r0, [r5, r7]\n"
        "movs r2, #0\n"
        "ldrsh r1, [r6, r2]\n"
        "movs r7, #0\n"
        "ldrsh r2, [r4, r7]\n"
        "bl _08025C84\n"
        "movs r0, #1\n"
        "bl Event_Post1\n"
        ".Lad84_store:\n"
        "movs r0, #0\n"
        "ldrsh r1, [r6, r0]\n"
        "lsls r1, r1, #1\n"
        "movs r2, #0\n"
        "ldrsh r0, [r5, r2]\n"
        "lsls r0, r0, #3\n"
        "adds r1, r1, r0\n"
        "add r1, r8\n"
        "ldrh r0, [r4, #0]\n"
        "strh r0, [r1, #0]\n"
        "ldrh r2, [r4, #0]\n"
        "adds r2, #1\n"
        "strh r2, [r4, #0]\n"
        "movs r3, #0\n"
        "ldrsh r0, [r6, r3]\n"
        "lsls r0, r0, #1\n"
        "movs r4, #0\n"
        "ldrsh r1, [r5, r4]\n"
        "lsls r1, r1, #3\n"
        "adds r0, r0, r1\n"
        "add r0, r9\n"
        "strh r2, [r0, #0]\n"
        ".Lad84_done:\n"
        "pop {r3, r4}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".Lad84_lit_0: .word 0x03001780\n"
        ".Lad84_lit_1: .word 0x00000FF6\n"
        ".Lad84_lit_2: .word 0x00000FF2\n"
        ".Lad84_lit_3: .word 0x00000FFA\n"
        ".Lad84_lit_4: .word 0x0000100A\n"
        ".Lad84_lit_5: .word 0x00000FF8\n"
        ".Lad84_lit_6: .word 0x00000FF4\n"
        ".Lad84_lit_7: .word 0x000010E5\n"
        ".syntax divided\n"
    );
}

// _0800AE78 — advance the committed grid/row state (WA = 0x03001780):
//   idx = s16[WA+0x0FF8]*2 + s16[WA+0x0FF2]*8   (byte offset; row cell 0x0FF2)
//   vA  = s16[WA+0x0FFA + idx]     vB = s16[WA+0x102A + idx]
//   vA == 11: the cell at WA+0x101A + idx is armed and the WA+0x0FF6 counter
//   advances; past its threshold the row cell is re-seeded from
//   s16[WA+0x0FF0]. Tail: vA == 3 && vB == 0 re-derives the index and arms
//   WA+0x102A.
//
// EXACT 268/268. Everything below that looks mechanical was measured:
//
//  * Pool. The ROM keeps the base and each displacement in its own pool word
//    (`ldr r2,=0x03001780; ldr r3,=0x00000FFA; adds r0,r2,r3`). A plain
//    `(uintptr_t)(0x03001780 + off)` is one folded CONST_INT and collapses to a
//    single `ldr`, so the base and the four displacements that own a word are
//    named as SYMBOL_REFs (the WA_SPLIT recipe from race_scene_b1.c). Each
//    `(wa + off)` is then built before `+ idx`: as one expression agbcc
//    re-associates to `(idx + wa) + off`, shares `idx + wa`, and synthesises one
//    displacement from another (`0x0FF2 + 8`, `0x0FF2 + 56`, -20 bytes).
//  * 0x101A / 0xFF0 have no pool word: the ROM derives them from the 0x102A
//    constant already in a register (`subs r7,#16`, `subs r7,#42`). Plain
//    constants get re-materialised (`movs #0xFF; lsls #4`, +2 bytes) even with
//    0x101A still live, so the chain lives in `o4` and is laundered with a
//    `"+r"` barrier first -- otherwise cse folds `o4 - 16` back to a literal.
//  * The two arms keep separate copies of the re-seed tail. With the constants
//    out of the way agbcc's cross-jump pass merges them (-8 bytes); the
//    left-alone spills (`e` into r8) cost more than they save, so the
//    distinguishing spelling is the `zero` temp in the second arm -- the ROM
//    has that `movs r2,#0` at the arm head and reuses it at `strh r2,[r4]`.
//  * The counter is `u32` + explicit narrowing: the ROM stores the wide value
//    (`strh`) and only then narrows (`lsls #16; asrs #16`) for the compare.
//  * The three empty-asm blocks are liveness pins for the signed-halfword
//    zero-index scratch (see findings/indexed_halfword_scratch.md). The `*pb`
//    and `vA` loads want r7/r5, which the allocator only reaches when r3/r5/r6
//    are live across them; without the pins it takes the lowest free register.
//  * The tail is a second set of pool words and uses the `RPOffFF2T` alias: one
//    symbol for both makes the tail's `wa+0xFF2` collapse onto the live `pb`.
//  * Register homes are the ROM's: left alone agbcc materialises every
//    displacement into r0 and adds the address into that same r0.
__asm__(".set RacePrWaB, 0x03001780");
extern u8 RacePrWaB;
__asm__(".set RPOffFF8, 0x00000FF8");
extern u8 RPOffFF8;
__asm__(".set RPOffFF2, 0x00000FF2");
extern u8 RPOffFF2;
__asm__(".set RPOffFF2T, 0x00000FF2");
extern u8 RPOffFF2T;
__asm__(".set RPOffFFA, 0x00000FFA");
extern u8 RPOffFFA;
__asm__(".set RPOffFF6, 0x00000FF6");
extern u8 RPOffFF6;
#define RACE_PR_DECL                                                     \
    __asm__(".globl RacePrWaB\nRacePrWaB = 0x03001780\n"                 \
            ".globl RPOffFF8\nRPOffFF8 = 0x00000FF8\n"                   \
            ".globl RPOffFF2\nRPOffFF2 = 0x00000FF2\n"                   \
            ".globl RPOffFF2T\nRPOffFF2T = 0x00000FF2\n"                 \
            ".globl RPOffFFA\nRPOffFFA = 0x00000FFA\n"                   \
            ".globl RPOffFF6\nRPOffFF6 = 0x00000FF6\n")

void Race_AdvanceProgress(void) {
    RACE_PR_DECL;
    register u32 wa __asm__("r2") = (u32)&RacePrWaB;
    register u32 w2 __asm__("r5");
    register u32 pa __asm__("r0");
    register u32 o1 __asm__("r1");
    register u32 o2 __asm__("r5");
    register u32 o3 __asm__("r3");
    register u32 o5 __asm__("r2");
    register u32 o6 __asm__("r0");
    u32 o4;
    register s16 *pb __asm__("r4");
    s16 *pe;
    register s16 *pcA __asm__("r2");
    register s16 *pcB __asm__("r3");
    register s32 idx __asm__("r1");
    register s32 c __asm__("r6");
    register s32 d __asm__("r12");
    s32 e, i2, zero;
    u32 n;

    o1 = (u32)&RPOffFF8;
    pa = wa + o1;
    idx = *(s16 *)pa * 2;
    o2 = (u32)&RPOffFF2;
    pb = (s16 *)(wa + o2);
    {
        register s32 h3 __asm__("r3");
        register s32 h5 __asm__("r5");
        register s32 h6 __asm__("r6");
        __asm__("" : "=r"(h3), "=r"(h5), "=r"(h6));
        idx += *pb * 8;
        __asm__("" : : "r"(h3), "r"(h5), "r"(h6));
    }
    o3 = (u32)&RPOffFFA;
    pa = wa + o3;
    {
        register s32 h3b __asm__("r3");
        __asm__("" : "=r"(h3b));
        c = *(s16 *)(uintptr_t)(idx + pa);
        __asm__("" : : "r"(h3b));
    }
    o4 = 0x102A;
    __asm__ volatile("" : "+r"(o4));
    pa = wa + o4;
    d = *(s16 *)(uintptr_t)(idx + pa);
    w2 = wa;
    if (c == 11) {
        o4 = o4 - 16u;
        pa = w2 + o4;
        __asm__("" : "+r"(o4), "+r"(pa));
        pe = (s16 *)(uintptr_t)(idx + pa);
        e = *pe;
        if (e == 0) {
            *pe = 1;
            o1 = (u32)&RPOffFF6;
            pcA = (s16 *)(w2 + o1);
            __asm__("" : "+r"(pcA));
            n = (u32)(u16)*pcA + 1u;
            *pcA = n;
            if ((s16)n > 3) {
                o4 = o4 - 42u;
                pa = w2 + o4;
                __asm__("" : "+r"(o4), "+r"(pa));
                if (*(s16 *)pa == 1) {
                    if (*pb == 0) {
                        *pb = 1;
                        *pcA = e;
                    } else {
                        *pb = 1;
                        *pcA = 3;
                    }
                } else {
                    *pb = 0;
                    *pcA = 3;
                }
            }
        } else {
            zero = 0;
            __asm__("" : "+r"(zero));
            *pe = 1;
            o6 = (u32)&RPOffFF6;
            pcB = (s16 *)(w2 + o6);
            __asm__("" : "+r"(pcB));
            if (*pcB > 2) {
                o4 = 0x0FF0;
                pa = w2 + o4;
                __asm__("" : "+r"(o4), "+r"(pa));
                if (*(s16 *)pa == 1 && *pb != 0)
                    *pb = 1;
                else
                    *pb = zero;
                *pcB = 3;
            }
        }
    }
    if (c == 3 && d == 0) {
        o1 = (u32)&RPOffFF8;
        pa = w2 + o1;
        i2 = *(s16 *)pa * 2;
        o3 = (u32)&RPOffFF2T;
        pa = w2 + o3;
        i2 += *(s16 *)pa * 8;
        __asm__ volatile("" : "+r"(o3));
        o5 = 0x102A;
        pa = w2 + o5;
        *(s16 *)(uintptr_t)(i2 + pa) = 1;
    }
}

// _0800AF84 — per-frame race tail (312 B): three one-shot flag cells, the
// 0x10E5 1<->3 swap, the grid commit into 0x1054, the mode dispatch at 0x0FBC,
// then the gate that runs Race_CommitCell/Ai_RaceFsm/Race_AdvanceProgress.
//
// The ROM pools the work-area base three separate times (0x0800AF9C,
// 0x0800B038, 0x0800B0B4) and each half reads it into a different register, so
// every half needs its own symbol. One shared anchor lets agbcc CSE the base
// into a single long-lived register and the body grows a `push {r4,r5,r6,lr}`.
// 0x0FF2 and 0x1074 are symbols for a different reason: as plain literals agbcc
// re-derives them from a displacement still in a register (`0x103C-74`,
// `0x104A+42`), which loses two pool words and 4 B. 0x103C / 0x104A pool
// themselves as literals, and 0x0FF6 / 0x103A are the ROM's own in-register
// derivations from the 0x1074 value (`subs #126`, `adds #68`).
//
// Register homes, all measured against the ROM listing:
//  * `wa` -> r2 and the two short-lived cell pointers -> r3 (0x10BE/0x10C0) and
//    r1 (0x10E5). Left to itself agbcc puts wa in r3 and every cell pointer in
//    r1; without the r1 pin on the third pointer it reuses wa's r2 as the sum's
//    destination (`adds r2,r2,r1`).
//  * the dispatch value is pinned to r0: the ROM's compare tree leaves it there
//    and the `case 2` call consumes it as the argument (`bl` with no `movs`).
//    As a free variable agbcc emits an extra `movs r0,#2`.
//  * `oc` (the 0x1074 displacement, kept across the store so the two grid reads
//    can derive from it) only takes r3 when it is a global allocno; the empty
//    `+r` barrier at the top of the body is that pin. Placing it after the
//    assignment instead dislodges the whole 0x103C/0x104A/0x1074 store block.
//  * `t` (the 0x0FF2 displacement) needs its barrier *after* the read, or agbcc
//    coalesces it into the sum's destination and the zero index moves to r1.
//  * the grid result is narrowed through a named u8 so the `lsls/lsrs` pair
//    lands before the 0x1054 pool load, as in the ROM.
__asm__(".set PftWa, 0x03001780");
extern u8 PftWa;
__asm__(".set PftWb, 0x03001780");
extern u8 PftWb;
__asm__(".set PftWc, 0x03001780");
extern u8 PftWc;
// 0x1074 and 0x0FF2 need a symbol of their own: as plain literals agbcc
// re-derives each from a displacement already in a register (`0x104A+42`,
// `0x103C-74`), which drops two pool words and 4 B of the body.
__asm__(".set PftO1074, 0x00001074");
extern u8 PftO1074;
__asm__(".set PftO0FF2, 0x00000FF2");
extern u8 PftO0FF2;
// Call-site split: the closure spells these by VMA because only the ROM label
// is in the matching slice, the friendly names exist as C aliases alone. Same
// shape as RS_CALLEE (race_scene.c) and FF_CALLEE (menu_ff78_f.c).
#define PFT_CALLEE(friendly, closure) closure
extern void sub_08004CC4(void); // ROM label spelling (asm/passthrough.inc)
extern int  _08025CF4(int a, int b, int c);
extern void _0800AA40(void);
extern void _0800BD40(void);
extern void _0800BE74(void);
extern void _0800BE20(void *c);
extern void _0800AD84(void);
extern void _0800AE78(void);

int Race_PerFrameTail(void) {
    __asm__(".globl PftWa\nPftWa = 0x03001780\n"
            ".globl PftWb\nPftWb = 0x03001780\n"
            ".globl PftWc\nPftWc = 0x03001780\n"
            ".globl PftO1074\nPftO1074 = 0x00001074\n"
            ".globl PftO0FF2\nPftO0FF2 = 0x00000FF2\n");
    register u8 *wa __asm__("r2");
    u8 *wbp;
    register u8 *p1 __asm__("r3");
    register u8 *p3 __asm__("r1");
    u32 wb, wc, o, oc, t;
    u8 rb;
    s16 *sp;
    s16 *gate;
    int v, m, a, b, c, r;

    wa = (u8 *)(uintptr_t)&PftWa;
    __asm__ volatile("" : "+r"(wa));
    __asm__ volatile("" : "+r"(oc));
    p1 = wa + 0x10BE;
    if (*p1 != 0) {
        *p1 = 0;
        return 47;
    }
    p1 = wa + 0x10C0;
    if (*p1 != 0) {
        *p1 = 0;
        PFT_CALLEE(CounterClear, sub_08004CC4)();
        return 13;
    }
    p3 = wa + 0x10E5;
    v = *(s8 *)p3;
    switch (v) {
    case 1:
        v = 3;
        break;
    case 3:
        v = 1;
        break;
    case 0:
    case 2:
        break;
    default:
        goto stepped;
    }
    *p3 = (u8)v;
stepped:
    wbp = (u8 *)(uintptr_t)&PftWb;
    wb = (u32)(uintptr_t)wbp;
    *(u16 *)(uintptr_t)(wbp + 0x103Cu) = 0;
    *(u16 *)(uintptr_t)(wb + 0x104Au) = 0;
    oc = (u32)(uintptr_t)&PftO1074;
    *(u16 *)(uintptr_t)(wb + oc) = 0;
    t = (u32)(uintptr_t)&PftO0FF2;
    sp = (s16 *)(uintptr_t)(wb + t);
    a = *sp;
    __asm__ volatile("" : "+r"(t));
    oc = oc - 126u;
    b = *(s16 *)(uintptr_t)(wb + oc);
    oc = oc + 68u;
    c = *(s16 *)(uintptr_t)(wb + oc);
    r = PFT_CALLEE(Ai_GridGet, _08025CF4)(a, b, c);
    rb = (u8)r;
    o = 0x1054u;
    *(u16 *)(uintptr_t)(wb + o) = rb;

    {
    register int m __asm__("r0");
    m = *(s16 *)(uintptr_t)(wb + 0x0FBCu);
    switch (m) {
    case 7:
        if (*(u16 *)(uintptr_t)(wb + 0x1078u) == 2)
            PFT_CALLEE(MenuProgressUpdate_0BD40, _0800BD40)();
        break;
    case 1:
        PFT_CALLEE(MenuProgressUpdate_0BD40, _0800BD40)();
        break;
    case 0:
        if (*(u16 *)(uintptr_t)(wb + 0x10FCu) == 2)
            PFT_CALLEE(MenuPlaceUpdate_0BE74, _0800BE74)();
        break;
    case 2:
        PFT_CALLEE(MenuRecordApply_0800BE20, _0800BE20)((void *)(uintptr_t)m);
        break;
    }
    }
    wc = (u32)(uintptr_t)&PftWc;
    gate = (s16 *)(uintptr_t)(wc + 0x0FBCu);
    if (*gate == 0)
        PFT_CALLEE(Race_CommitCell, _0800AD84)();
    PFT_CALLEE(Ai_RaceFsm, _0800AA40)();
    if (*gate == 0)
        PFT_CALLEE(Race_AdvanceProgress, _0800AE78)();
    return _0800AA20();
}

void _0800AD84(void) __attribute__((alias("Race_CommitCell")));
void _0800AE78(void) __attribute__((alias("Race_AdvanceProgress")));
int  _0800AF84(void) __attribute__((alias("Race_PerFrameTail")));
