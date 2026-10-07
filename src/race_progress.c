#include "gtadv/race_progress.h"
#include "gtadv/memory.h"
#include "gba/types.h"

// Reference: asm/race_progress.s 0x0800AD84-0x0800B0BC
// Three routines commit/advance/per-frame tail with WA=0x03001780.
// Evidence: literal pools at _0800AD84_lit_0..7 and _0800AE78 pools, exact halfword/byte widths,
// and calls to sub_08025CF4 (Ai_GridGet) and sub_0800279C.
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

// _0800AD84 — commit selected grid cell
void Race_CommitCell(void) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    // ROM: r3 cell = WA+0x0FF6 (the cursor column), r5 cell = WA+0x0FF2 (row)
    s16 col = *(volatile s16 *)(wa + 0x0FF6);
    s16 row = *(volatile s16 *)(wa + 0x0FF2);
    s16 committedCol = col;   // == WA+0x0FF8 after the store below
    // dst1 = WA+0x0FFA + col*2 + row*8, dst2 = WA+0x100A + col*2+row*8, src = WA+0x103A
    volatile u16 *src = (volatile u16 *)(wa + 0x103A);
    volatile u16 *dst1 = (volatile u16 *)(wa + 0x0FFA + (u32)(u16)col*2u + (u32)(u16)row*8u);
    volatile u16 *dst2 = (volatile u16 *)(wa + 0x100A + (u32)(u16)col*2u + (u32)(u16)row*8u);
    *dst1 = *src;
    *dst2 = *src;
    *(volatile s16 *)(wa + 0x0FF8) = col;
    *(volatile s16 *)(wa + 0x0FF4) = row;
    committedCol = *(volatile s16 *)(wa + 0x0FF8);   // ROM re-reads the cell
    s8 tier = *(volatile s8 *)(wa + 0x10E5);
    u32 tu = (u32)(u8)(tier - 1);
    if (tu > 2) return;
    // ROM passes the three coordinate cells *and* the lane value:
    //   r0 = s16[WA+0x0FF2], r1 = s16[WA+0x0FF8], r2 = s16[WA+0x103A]
    int g = Ai_GridGet(row, committedCol, (int)(s16)*src);
    g = (g << 24) >> 24;
    if (g < (int)tier) {
        // r3 = tier: raise the packed cell to at least the tier
        _08025C84((int)row, (int)committedCol, (int)(s16)*src, (int)tier);
        Event_Post1(1);
    }
    // `store` label: dst2 keeps the OLD value, the cell and dst1 take value+1
    *dst2 = *src;
    u16 nv = (u16)(*src + 1);
    *src = nv;
    *dst1 = nv;
}

// _0800AE78 — advance committed state
void Race_AdvanceProgress(void) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    s16 col = *(volatile s16 *)(wa + 0x0FF8);
    s16 row = *(volatile s16 *)(wa + 0x0FF2);
    volatile u16 *cellA = (volatile u16 *)(wa + 0x0FFA + (u32)col*2 + (u32)row*8);
    volatile u16 *cellB = (volatile u16 *)(wa + 0x102A + (u32)col*2 + (u32)row*8);
    u16 vA = *cellA;
    u16 vB = *cellB;
    if (vA == 11) {
        volatile u16 *zero = (volatile u16 *)(wa + 0x0FFA + (u32)col*2 + (u32)row*8 -16);
        if (*zero == 0) {
            *zero = 1;
            volatile u16 *cnt = (volatile u16 *)(wa + 0x0FF6);
            u16 c = *cnt + 1;
            *cnt = c;
            if (c > 3) {
                s16 layerRow = *(volatile s16 *)(wa + 0x0FF2 -42); // WA+0x0FC8 approx
                if (layerRow == 1) {
                    if (row == 0) *cellA = 1;
                } else {
                    *cellA = *zero;
                }
                *(volatile u16 *)(wa + 0x0FF6) = 3;
            }
        } else {
            *zero = 1;
            volatile u16 *cnt2 = (volatile u16 *)(wa + 0x0FF6);
            if (*(volatile s16 *)cnt2 > 2) {
                *(volatile s16 *)(wa + 0x0FF2) = 0;
                *(volatile s16 *)(wa + 0x0FF6) = 3;
            }
        }
    }
    if (vA == 3 && vB == 0) {
        *(volatile u16 *)(wa + 0x102A + (u32)col*2 + (u32)row*8) = 1;
    }
}

// _0800AF84 — per-frame tail dispatcher
// _0800AF84 — the record-49 per-frame tail. Returns the *step id* the caller
// (`_0800A76C`, asm/carphys_tick.s) feeds back into the scene record chain:
// 47 / 13 on the two gate lanes, else whatever `_0800AA20` returns.
int Race_PerFrameTail(void) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    u8 f1 = *(volatile u8 *)(wa + 0x10BE);
    if (f1) { *(volatile u8 *)(wa + 0x10BE) = 0; return 47; }
    u8 f2 = *(volatile u8 *)(wa + 0x10C0);
    if (f2) { *(volatile u8 *)(wa + 0x10C0) = 0; CounterClear(); return 13; }
    // tier byte: 1 <-> 3 swap only; 0/2 are stored back unchanged, anything
    // else (negative or > 3) is left alone
    s8 tier = *(volatile s8 *)(wa + 0x10E5);
    if (tier == 1) *(volatile s8 *)(wa + 0x10E5) = 3;
    else if (tier == 0 || tier == 2) *(volatile s8 *)(wa + 0x10E5) = tier;
    else if (tier == 3) *(volatile s8 *)(wa + 0x10E5) = 1;
    // Clear three halfwords at WA+0x103C/0x104A/0x1074
    *(volatile u16 *)(wa + 0x103C) = 0;
    *(volatile u16 *)(wa + 0x104A) = 0;
    *(volatile u16 *)(wa + 0x1074) = 0;
    // _08025CF4(s16[WA+0x0FF2], s16[WA+0x0FF6], s16[WA+0x103A]) -> u8
    int g = Ai_GridGet(*(volatile s16 *)(wa + 0x0FF2),
                       *(volatile s16 *)(wa + 0x0FF6),
                       (int)(s16)*(volatile u16 *)(wa + 0x103A));
    *(volatile u16 *)(wa + 0x1054) = (u16)(g & 0xFF);
    // phase dispatch
    s16 cur = *(volatile s16 *)(wa + 0x0FBC);
    if (cur == 1) {
        MenuProgressUpdate_0BD40();
    } else if (cur > 1) {
        // ROM passes r0 = cur to 0x0800BE20; that callee ignores it
        if (cur == 2) MenuRecordApply_0800BE20((void *)(uintptr_t)(s32)cur);
        else if (cur == 7 && *(volatile u16 *)(wa + 0x1078) == 2)
            MenuProgressUpdate_0BD40();
    } else if (cur == 0 && *(volatile u16 *)(wa + 0x10FC) == 2) {
        MenuPlaceUpdate_0BE74();
    }
    if (*(volatile s16 *)(wa + 0x0FBC) == 0) Race_CommitCell();
    Ai_RaceFsm();
    // re-read: both callees may advance the phase
    if (*(volatile s16 *)(wa + 0x0FBC) == 0) Race_AdvanceProgress();
    return _0800AA20();
}

#ifndef __APPLE__
void _0800AD84(void) __attribute__((alias("Race_CommitCell")));
void _0800AE78(void) __attribute__((alias("Race_AdvanceProgress")));
int  _0800AF84(void) __attribute__((alias("Race_PerFrameTail")));
#endif
