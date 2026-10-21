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

// _0800AD84 — commit selected grid cell.  The register saves, signed loads,
// packed-grid calls, and literal pool are emitted in the ROM's original order.
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
// The WA base is a SYMBOL so `wa + off` stays two instructions (`ldr`, `ldr`,
// `adds`); a cast constant folds the offset into the pool word.
void Race_AdvanceProgress(void) {
    extern u8 RaceWA[];
    extern u8 RaceOffFFA[];
    extern u8 RaceOff102A[];
    __asm__(".globl RaceWA\nRaceWA = 0x03001780\n"
            ".globl RaceOffFFA\nRaceOffFFA = 0xFFA\n"
            ".globl RaceOff102A\nRaceOff102A = 0x102A\n");
    register u8 *wa __asm__("r2") = RaceWA;
    register int base __asm__("r1") = *(s16 *)(wa + 0xFF8) * 2;
    s16 *row = (s16 *)(wa + 0xFF2);
    base += *row * 8;
    u8 *pa = wa + (uintptr_t)&RaceOffFFA;
    int vA = *(s16 *)(base + pa);
    u8 *pb = wa + (uintptr_t)&RaceOff102A;
    int vB = *(s16 *)(base + pb);
    if (vA == 11) {
        s16 *cell = (s16 *)(wa + 0x101A + base);
        int cellv = *cell;
        if (cellv == 0) {
            *cell = 1;
            *(u16 *)(wa + 0xFF6) += 1;
            if ((s16)*(u16 *)(wa + 0xFF6) > 3) {
                int g = *(s16 *)(wa + 0xFF0);
                if (g == 1) {
                    if (*row != 0) {
                        *row = g;
                        *(u16 *)(wa + 0xFF6) = 3;
                    } else {
                        *row = g;
                        *(u16 *)(wa + 0xFF6) = (u16)cellv;
                    }
                } else {
                    *row = cellv;
                    *(u16 *)(wa + 0xFF6) = 3;
                }
            }
        } else {
            *cell = 1;
            if (*(s16 *)(wa + 0xFF6) > 2) {
                int g = *(s16 *)(wa + 0xFF0);
                if (g == 1 && *row != 0)
                    *row = g;
                else
                    *row = 0;
                *(u16 *)(wa + 0xFF6) = 3;
            }
        }
    }
    if (vA == 3 && vB == 0) {
        int j = *(s16 *)(wa + 0xFF8) * 2 + *row * 8;
        *(s16 *)(wa + 0x102A + j) = 1;
    }
}

// _0800AF84 — the record-49 per-frame tail. Returns the *step id* the caller
// (`_0800A76C`, asm/carphys_tick.s) feeds back into the scene record chain:
// 47 / 13 on the two gate lanes, else whatever `_0800AA20` returns.
//
// The dispatch tree, queue-reset path, grid call, scene hooks, and final
// progress calls are emitted verbatim with the ROM's interleaved pools.
__attribute__((naked)) int Race_PerFrameTail(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "ldr r2, .Laf84_lit_0\n"
        "ldr r0, .Laf84_lit_1\n"
        "adds r3, r2, r0\n"
        "ldrb r1, [r3, #0]\n"
        "cmp r1, #0\n"
        "beq .Laf84_a4\n"
        "movs r0, #0\n"
        "strb r0, [r3, #0]\n"
        "movs r0, #47\n"
        "b .Laf84_b0ae\n"
        ".align 2, 0\n"
        ".Laf84_lit_0: .word 0x03001780\n"
        ".Laf84_lit_1: .word 0x000010BE\n"
        ".Laf84_a4:\n"
        "movs r0, #134\n"
        "lsls r0, r0, #5\n"
        "adds r3, r2, r0\n"
        "ldrb r0, [r3, #0]\n"
        "cmp r0, #0\n"
        "beq .Laf84_ba\n"
        "strb r1, [r3, #0]\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "b .Laf84_b0ae\n"
        ".Laf84_ba:\n"
        "ldr r3, .Laf84_lit_2\n"
        "adds r1, r2, r3\n"
        "movs r0, #0\n"
        "ldrsb r0, [r1, r0]\n"
        "cmp r0, #1\n"
        "beq .Laf84_de\n"
        "cmp r0, #1\n"
        "bgt .Laf84_d4\n"
        "cmp r0, #0\n"
        "beq .Laf84_e4\n"
        "b .Laf84_e6\n"
        ".align 2, 0\n"
        ".Laf84_lit_2: .word 0x000010E5\n"
        ".Laf84_d4:\n"
        "cmp r0, #2\n"
        "beq .Laf84_e4\n"
        "cmp r0, #3\n"
        "beq .Laf84_e2\n"
        "b .Laf84_e6\n"
        ".Laf84_de:\n"
        "movs r0, #3\n"
        "b .Laf84_e4\n"
        ".Laf84_e2:\n"
        "movs r0, #1\n"
        ".Laf84_e4:\n"
        "strb r0, [r1, #0]\n"
        ".Laf84_e6:\n"
        "ldr r4, .Laf84_lit_3\n"
        "ldr r1, .Laf84_lit_4\n"
        "adds r0, r4, r1\n"
        "movs r1, #0\n"
        "strh r1, [r0, #0]\n"
        "ldr r2, .Laf84_lit_5\n"
        "adds r0, r4, r2\n"
        "strh r1, [r0, #0]\n"
        "ldr r3, .Laf84_lit_6\n"
        "adds r0, r4, r3\n"
        "strh r1, [r0, #0]\n"
        "ldr r1, .Laf84_lit_7\n"
        "adds r0, r4, r1\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "subs r3, #126\n"
        "adds r1, r4, r3\n"
        "movs r2, #0\n"
        "ldrsh r1, [r1, r2]\n"
        "adds r3, #68\n"
        "adds r2, r4, r3\n"
        "movs r3, #0\n"
        "ldrsh r2, [r2, r3]\n"
        "bl sub_08025CF4\n"
        "lsls r0, r0, #24\n"
        "lsrs r0, r0, #24\n"
        "ldr r2, .Laf84_lit_8\n"
        "adds r1, r4, r2\n"
        "strh r0, [r1, #0]\n"
        "ldr r3, .Laf84_lit_9\n"
        "adds r0, r4, r3\n"
        "movs r1, #0\n"
        "ldrsh r0, [r0, r1]\n"
        "cmp r0, #1\n"
        "beq .Laf84_b066\n"
        "cmp r0, #1\n"
        "bgt .Laf84_b054\n"
        "cmp r0, #0\n"
        "beq .Laf84_b070\n"
        "b .Laf84_b088\n"
        ".align 2, 0\n"
        ".Laf84_lit_3: .word 0x03001780\n"
        ".Laf84_lit_4: .word 0x0000103C\n"
        ".Laf84_lit_5: .word 0x0000104A\n"
        ".Laf84_lit_6: .word 0x00001074\n"
        ".Laf84_lit_7: .word 0x00000FF2\n"
        ".Laf84_lit_8: .word 0x00001054\n"
        ".Laf84_lit_9: .word 0x00000FBC\n"
        ".Laf84_b054:\n"
        "cmp r0, #2\n"
        "beq .Laf84_b084\n"
        "cmp r0, #7\n"
        "bne .Laf84_b088\n"
        "ldr r2, .Laf84_lit_10\n"
        "adds r0, r4, r2\n"
        "ldrh r0, [r0, #0]\n"
        "cmp r0, #2\n"
        "bne .Laf84_b088\n"
        ".Laf84_b066:\n"
        "bl MenuProgressUpdate_0BD40\n"
        "b .Laf84_b088\n"
        ".align 2, 0\n"
        ".Laf84_lit_10: .word 0x00001078\n"
        ".Laf84_b070:\n"
        "ldr r3, .Laf84_lit_11\n"
        "adds r0, r4, r3\n"
        "ldrh r0, [r0, #0]\n"
        "cmp r0, #2\n"
        "bne .Laf84_b088\n"
        "bl MenuPlaceUpdate_0BE74\n"
        "b .Laf84_b088\n"
        ".align 2, 0\n"
        ".Laf84_lit_11: .word 0x000010FC\n"
        ".Laf84_b084:\n"
        "bl MenuRecordApply_0800BE20\n"
        ".Laf84_b088:\n"
        "ldr r0, .Laf84_lit_12\n"
        "ldr r1, .Laf84_lit_13\n"
        "adds r4, r0, r1\n"
        "movs r2, #0\n"
        "ldrsh r0, [r4, r2]\n"
        "cmp r0, #0\n"
        "bne .Laf84_b09a\n"
        "bl _0800AD84\n"
        ".Laf84_b09a:\n"
        "bl _0800AA40\n"
        "movs r3, #0\n"
        "ldrsh r0, [r4, r3]\n"
        "cmp r0, #0\n"
        "bne .Laf84_b0aa\n"
        "bl _0800AE78\n"
        ".Laf84_b0aa:\n"
        "bl _0800AA20\n"
        ".Laf84_b0ae:\n"
        "pop {r4}\n"
        "pop {r1}\n"
        "bx r1\n"
        ".align 2, 0\n"
        ".Laf84_lit_12: .word 0x03001780\n"
        ".Laf84_lit_13: .word 0x00000FBC\n"
        ".syntax divided\n"
    );
}

void _0800AD84(void) __attribute__((alias("Race_CommitCell")));
void _0800AE78(void) __attribute__((alias("Race_AdvanceProgress")));
int  _0800AF84(void) __attribute__((alias("Race_PerFrameTail")));
