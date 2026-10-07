// ============================================================================
// menu_d1b4.c — C lift of asm/menu_d1b4.s (VMA 0x0800D1B4–0x0800D280, 5 funcs)
//
// Garage cursor-box handlers. Every function is transcribed
// instruction-for-instruction from the cited asm listing. No speculative
// behavior beyond the asm.
//
// Cursor-box record layout (from field offsets in the asm):
//   +4  u16 x position (reset to 120)
//   +6  u16 y position (reset to 60)
//
// Runtime dispatch note: a descriptor table at ROM 0x0CB3F8 carries the
// Thumb pointer 0x0800D245 for sub_0800D244 ({fn,size,data} records) —
// the dispatcher is invoked with (ev, a1, a2, rec) via the loader.
//
// Event dispatcher shape matches _0800D17C (events 1/2/6/7):
//   ev1 -> reset cursor box to 120x60
//   ev2 -> no-op stub
//   ev6 -> redraw frame via _080038A4
//   ev7 -> dpad delta mover
// ============================================================================

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) void _080038A4(int a, int b, void *c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void _08004EA8(int v) { (void)v; }
#else
extern void _080038A4(int a, int b, void *c);   // 0x080038A4 (frame draw)
extern void _08004EA8(int v);                   // 0x08004EA8 (manager store+step)
#endif

// ROM frame templates (byte-exact pool words in the listing):
#define D1B4_FRAME_60   ((void *)(uintptr_t)0x0805F8A4)   // _0800D1E4
#define D1B4_FRAME_MOVE ((void *)(uintptr_t)0x0805F8AC)   // _0800D1E8

// ----------------------------------------------------------------------------
// sub_0800D1B4 — reset cursor box to 120 x 60.
void MenuD1B4_Reset(void *rec) {
    volatile u8 *r = (volatile u8 *)rec;
    *(volatile u16 *)(r + 4) = 120;
    *(volatile u16 *)(r + 6) = 60;
}
#ifndef __APPLE__
void _0800D1B4(void *a) __attribute__((alias("MenuD1B4_Reset")));
void sub_0800D1B4(void *a) __attribute__((alias("MenuD1B4_Reset")));
#endif

static inline s16 *slot_d1c0(volatile u8 *p, s16 off) {
    return (s16 *)(p + off);
}
void MenuD1B4_Draw(void *rec) {
    volatile u8 *r4 = (volatile u8 *)rec;
    _080038A4(60, 60, D1B4_FRAME_60);
    s16 x = *slot_d1c0(r4, 4);
    s16 y = *slot_d1c0(r4, 6);
    _080038A4(x, y, D1B4_FRAME_MOVE);
}
#ifndef __APPLE__
void _0800D1C0(void *a) __attribute__((alias("MenuD1B4_Draw")));
void sub_0800D1C0(void *a) __attribute__((alias("MenuD1B4_Draw")));
#endif

// ----------------------------------------------------------------------------
// sub_0800D1EC — dpad delta mover (ev7).
//   r3 = rec, r5 = arg2 (keys2), r4 = dx = 0, r2 = dy = 0:
//     keys(arg1) & 32  -> dx -= 1
//     keys(arg1) & 16  -> dx += 1
//     keys(arg1) & 64  -> dy -= 1
//     keys(arg1) & 128 -> dy += 1
//   u16[rec+4] += dx; u16[rec+6] += dy
//   keys2 & 2 -> _08004EA8(1)
//
//  * The 2 trailing bytes are the section-alignment filler, not unreachable
//    code.  The body is 82 bytes, so under `-ffunction-sections` its section
//    is padded to 84 and gas closes a Thumb *code* section with the 2-byte
//    nop filler (0x46c0) where the ROM holds `00 00`.  A file-scope
//    `.align 2, 0` after the body emits the explicit `0` fill instead.  This
//    is the same one-liner `_0800A9A0`/`_0800A9E0` use; see the note below
//    the body and src/car_tick_dispatch.c for the full mechanism.
//  * The other 4 are one register pair, and they DO need a source-shape fix.
//    ROM `ldrh r1,[r3,#4]; adds r0,r1,r4` against candidate
//    `ldrh r0,[r3,#4]; adds r0,r0,r4`: unpinned agbcc coalesces the loaded
//    halfword with the sum into r0.  22-shape sweep only varied
//    TYPES and the order of the volatile accesses, never the register
//    ALLOCATION of the two pseudos, so it could not reach this: two GCC local
//    register variables per field -- load pinned to r1, sum pinned to r0 --
//    match both pairs.  With the load pinned alone the sum stays in r1
//    (78/84, first diff +0x34), and swapping the add's operand order instead
//    of pinning the sum does not help (78/84).  The mask chain, both stores,
//    the call and the tail matched throughout.
void MenuD1B4_Move(void *rec, int keys, int keys2) {
    volatile u8 *r3 = (volatile u8 *)rec;
    int r4 = 0;
    int r2 = 0;

    if (keys & 32)
        r4 -= 1;
    if (keys & 16)
        r4 += 1;
    if (keys & 64)
        r2 -= 1;
    if (keys & 128)
        r2 += 1;

    register u16 x __asm__("r1") = *(volatile u16 *)(r3 + 4);
    register u16 s __asm__("r0") = (u16)(x + r4);
    *(volatile u16 *)(r3 + 4) = s;
    register u16 y __asm__("r1") = *(volatile u16 *)(r3 + 6);
    register u16 t __asm__("r0") = (u16)(y + r2);
    *(volatile u16 *)(r3 + 6) = t;

    if (keys2 & 2)
        _08004EA8(1);
}
// Two GCC local register variables per field. The ROM keeps the LOAD and the
// SUM in different registers -- `ldrh r1,[r3,#4]` then `adds r0,r1,r4`
// (0x0800D21E/0x0800D220) -- where unpinned agbcc coalesces both into r0. Pin
// the load to r1 and the sum to r0 and both pairs match; the intervening
// store is then `strh r0,[r3,#4]`, exactly as in the ROM. Pinning the load
// alone leaves the sum in r1 (78/84, first diff +0x34); swapping the add's
// operands instead of pinning the sum does not help (78/84).
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800D1EC(void *a, int b, int c) __attribute__((alias("MenuD1B4_Move")));
void sub_0800D1EC(void *a, int b, int c) __attribute__((alias("MenuD1B4_Move")));
#endif

// ----------------------------------------------------------------------------
// sub_0800D240 — no-op stub (ev2).
void MenuD1B4_Noop(void *rec) {
    (void)rec;
}
#ifndef __APPLE__
void _0800D240(void *a) __attribute__((alias("MenuD1B4_Noop")));
void sub_0800D240(void *a) __attribute__((alias("MenuD1B4_Noop")));
#endif

void MenuD1B4_Dispatch(int ev, int a1, int a2, void *rec) {
    switch ((u32)ev) {
    case 2:
        MenuD1B4_Noop(rec);
        break;
    case 1:
        MenuD1B4_Reset(rec);
        break;
    case 7:
        MenuD1B4_Draw(rec);
        break;
    case 6:
        MenuD1B4_Move(rec, a1, a2);
        break;
    }
}
#ifndef __APPLE__
void _0800D244(int a, int b, int c, void *d) __attribute__((alias("MenuD1B4_Dispatch")));
void sub_0800D244(int a, int b, int c, void *d) __attribute__((alias("MenuD1B4_Dispatch")));
#endif
