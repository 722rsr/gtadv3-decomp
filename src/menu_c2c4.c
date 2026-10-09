// ============================================================================
// menu_c2c4.c — C lift of two menu helper pockets:
//   asm/menu_c2c4.s (VMA 0x0800C2C4–0x0800C340, 4 funcs)
//   asm/menu_c340.s (VMA 0x0800C340–0x0800C454, 4 funcs)
//
// Transcribed instruction-for-instruction from the cited asm listings.
// ============================================================================

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) void _080050E8(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void _080056B8(int v) { (void)v; }
__attribute__((weak)) void _08004E1C(void *a) { (void)a; }
__attribute__((weak)) void _080055F4(void) {}
__attribute__((weak)) void sub_08002B50(void) {}
__attribute__((weak)) void sub_08002BB4(void) {}
__attribute__((weak)) void sub_08002B44(void) {}
__attribute__((weak)) void _08005604(void) {}
__attribute__((weak)) void sub_08002C98(void) {}
__attribute__((weak)) void _08004EF0(void *a) { (void)a; }
__attribute__((weak)) void _08002060(int v) { (void)v; }
__attribute__((weak)) void _08004ED8(int v) { (void)v; }
__attribute__((weak)) void _0800C1E4(void *a) { (void)a; }
#else
extern void _080050E8(void *a, int b);      // 0x080050E8 (event_dma_queue.c)
extern void _080056B8(int v);               // 0x080056B8 ObjQueueGate
extern void _08004E1C(void *a);             // 0x08004E1C Leaf_04E1C
extern void _080055F4(void);                // 0x080055F4 (event_dma_queue.c)
extern void sub_08002B50(void);                // 0x08002B50 Wrap_02B50
extern void sub_08002BB4(void);                // 0x08002BB4 Store_02BB4
extern void sub_08002B44(void);                // 0x08002B44 Store_02B44
extern void _08005604(void);                // 0x08005604 ObjQueueFlushMain
extern void sub_08002C98(void);                // 0x08002C98 ObjFlush_02C98
extern void _08004EF0(void *a);             // 0x08004EF0 (foundation_late.c)
extern void _08002060(int v);               // 0x08002060 (garage.c caller)
extern void _08004ED8(int v);               // 0x08004ED8
extern void _0800C1E4(void *a);             // 0x0800C1E4 (menus.c)
#endif

// Record offsets (byte-exact pools):
#define MC2_OFF_1DE4 0x1DE4
#define MC2_OFF_1DE8 0x1DE8

// ============================================================================
// menu_c2c4.s
// ============================================================================

// ----------------------------------------------------------------------------
// sub_0800C2C4 — slot enable + gate + leaf:
//   _080050E8(rec+8, 1); _080056B8(1); _08004E1C(u16[rec+92])
void MenuC2C4_Enable(void *rec) {
    volatile u8 *r = (volatile u8 *)rec;
    _080050E8((void *)(uintptr_t)(r + 8), 1);
    _080056B8(1);
    u16 v = *(volatile u16 *)(r + 92);
    _08004E1C((void *)(uintptr_t)v);
}
#ifndef __APPLE__
void _0800C2C4(void *a) __attribute__((alias("MenuC2C4_Enable")));
void sub_0800C2C4(void *a) __attribute__((alias("MenuC2C4_Enable")));
#endif

// ----------------------------------------------------------------------------
// sub_0800C2E4 — teardown chain: 055F4 -> 02B50 -> 02BB4 -> 02B44. The
// dispatcher passes rec in r0 (`adds r0,r3,#0`); the body ignores it.
void MenuC2C4_Teardown(void *rec) {
    (void)rec;
    _080055F4();
    sub_08002B50();
    sub_08002BB4();
    sub_08002B44();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800C2E4(void *a) __attribute__((alias("MenuC2C4_Teardown")));
void sub_0800C2E4(void *a) __attribute__((alias("MenuC2C4_Teardown")));
#endif

// ----------------------------------------------------------------------------
// sub_0800C2FC — flush chain: 05604 -> 02C98. Called with rec in r0 (ignored).
void MenuC2C4_Flush(void *rec) {
    (void)rec;
    _08005604();
    sub_08002C98();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800C2FC(void *a) __attribute__((alias("MenuC2C4_Flush")));
void sub_0800C2FC(void *a) __attribute__((alias("MenuC2C4_Flush")));
#endif

// ----------------------------------------------------------------------------
// sub_0800C30C — pointer-dispatch leaf:
//   r1 in 1..2 -> _08004EF0(*(u32**)(rec+0x1DE4))
//   r1 in 3..4 -> _08004EF0(*(u32**)(rec+0x1DE8))
void MenuC2C4_PtrDispatch(void *rec, int which) {
    volatile u8 *r = (volatile u8 *)rec;
    switch (which) {
    case 1:
    case 2:
        _08004EF0((void *)(uintptr_t)*(volatile u32 *)(r + MC2_OFF_1DE4));
        break;
    case 3:
    case 4:
        _08004EF0((void *)(uintptr_t)*(volatile u32 *)(r + MC2_OFF_1DE8));
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _0800C30C(void *a, int b) __attribute__((alias("MenuC2C4_PtrDispatch")));
void sub_0800C30C(void *a, int b) __attribute__((alias("MenuC2C4_PtrDispatch")));
#endif

// ============================================================================
// menu_c340.s
// ============================================================================

// ----------------------------------------------------------------------------
// sub_0800C340 — same pointer-dispatch with a leading _08002060(0) call.
void MenuC340_Dispatch(void *rec, int which) {
    volatile u8 *r = (volatile u8 *)rec;
    _08002060(0);
    switch (which) {
    case 1:
    case 2:
        _08004EF0((void *)(uintptr_t)*(volatile u32 *)(r + MC2_OFF_1DE4));
        break;
    case 3:
    case 4:
        _08004EF0((void *)(uintptr_t)*(volatile u32 *)(r + MC2_OFF_1DE8));
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _0800C340(void *a, int b) __attribute__((alias("MenuC340_Dispatch")));
void sub_0800C340(void *a, int b) __attribute__((alias("MenuC340_Dispatch")));
#endif

// ----------------------------------------------------------------------------
// sub_0800C37C — byte gate: if u8[rec+0x61] == 0 -> _08004ED8(1). The
// dispatcher (sub_0800C39C) reaches it with a zero-extended u16 second word
// (`lsls r1,#16; lsrs r1,#16`), so the parameter is declared even though the
// body ignores it; that keeps the call site's 4 bytes.
void MenuC340_ByteGate(void *rec, u16 arg) {
    volatile u8 *r = (volatile u8 *)rec;
    (void)arg;
    if (r[0x61] == 0)
        _08004ED8(1);
}
#ifndef __APPLE__
void _0800C37C(void *a, u16 b) __attribute__((alias("MenuC340_ByteGate")));
void sub_0800C37C(void *a, u16 b) __attribute__((alias("MenuC340_ByteGate")));
#endif

// ----------------------------------------------------------------------------
// sub_0800C390 — tail call _08002060(1). The dispatcher passes rec in r0
// (`adds r0,r3,#0`); the body ignores it.
void MenuC340_SetMode(void *rec) {
    (void)rec;
    _08002060(1);
}
#ifndef __APPLE__
void _0800C390(void *a) __attribute__((alias("MenuC340_SetMode")));
void sub_0800C390(void *a) __attribute__((alias("MenuC340_SetMode")));
#endif

// ----------------------------------------------------------------------------
// sub_0800C39C — 21-entry command dispatcher (r0 = id 1..21, r1 = arg, r3 = rec).
// The pool word at .L_c3B0 points at the real table .L_c3B4, so the switch
// index is `id-1` (0-based) and the case values are the table slots:
//   0->C1E4, 2->C2C4, 3->C2E4, 7->C2FC, 12->C30C(arg), 13->C340(arg),
//   18->C390, 20->C37C(u16 arg); every other slot returns.
void MenuC340_Command(int id, int arg, int a2, void *rec) {
    (void)a2;
    int idx = id - 1;
    if ((u32)idx > 20)
        return;
    switch (idx) {
    case 0: _0800C1E4(rec); break;
    case 2: MenuC2C4_Enable(rec); break;
    case 3: MenuC2C4_Teardown(rec); break;
    case 7: MenuC2C4_Flush(rec); break;
    case 12: MenuC2C4_PtrDispatch(rec, arg); break;
    case 13: MenuC340_Dispatch(rec, arg); break;
    case 20: MenuC340_ByteGate(rec, (u16)arg); break;
    case 18: MenuC340_SetMode(rec); break;
    default:        break;
    }
}
// Body is 182 bytes; the ROM's 184-byte span ends in `00 00`. Under
// -ffunction-sections gas closes a Thumb code section with the `46c0` nop;
// this file-scope `.align 2, 0` (after the body's `.size`, still inside its
// section) pads with the explicit `0` fill instead.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800C39C(int a, int b, int c, void *d) __attribute__((alias("MenuC340_Command")));
void sub_0800C39C(int a, int b, int c, void *d) __attribute__((alias("MenuC340_Command")));
#endif
