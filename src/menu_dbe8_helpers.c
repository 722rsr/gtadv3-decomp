// ============================================================================
// menu_dbe8_helpers.c — C lift of the 4 remaining menu_dbe8.s functions:
//   sub_0800DE4C — no-op leaf (bx lr)
//   sub_0800E2E4 — param handler (range-gated sound + reset)
//   sub_0800E598 — no-op leaf (bx lr)
//   sub_0800E59C — 12-entry command dispatcher (table 0x0800E5B8, 0-based
//                  idx = cmd-1; cases 1/2/5/6/7/12 live, rest return)
//
// Transcribed instruction-for-instruction from asm/menu_dbe8.s.
// ============================================================================

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) void _0802B368(int v) { (void)v; }
__attribute__((weak)) void _08002618(int a, int b) { (void)a; (void)b; }
__attribute__((weak)) void _0800DD9C(void *a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void _0800D854(void *a) { (void)a; }
__attribute__((weak)) void _0800D8E4(void *a) { (void)a; }
__attribute__((weak)) void _0800E418(void *a) { (void)a; }
__attribute__((weak)) void _0800E15C(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void _0800DE50(void *a) { (void)a; }
__attribute__((weak)) void _0800DE4C(void *a) { (void)a; }
#else
extern void _0802B368(int v);               // 0x0802B368 sound entry
extern void _08002618(int a, int b);        // 0x08002618
extern void _0800DD9C(void *a, u32 b);      // 0x0800DD9C (menus.c)
extern void _0800D854(void *a);             // 0x0800D854 (menu_stage.c)
extern void _0800D8E4(void *a);             // 0x0800D8E4 (menu_stage.c)
extern void _0800E418(void *a);             // 0x0800E418 (menus.c)
extern void _0800E15C(void *a, u32 b, u32 c); // 0x0800E15C (menus.c)
extern void _0800DE50(void *a);             // 0x0800DE50 (menus.c)
extern void _0800DE4C(void *a);             // 0x0800DE4C (this file)
#endif

// ----------------------------------------------------------------------------
// sub_0800DE4C — no-op leaf.
void MenuDE4C_0800DE4C(void *rec) {
    (void)rec;
}
#ifndef __APPLE__
void _0800DE4C(void *a) __attribute__((alias("MenuDE4C_0800DE4C")));
void sub_0800DE4C(void *a) __attribute__((alias("MenuDE4C_0800DE4C")));
#endif

// ----------------------------------------------------------------------------
// sub_0800E2E4 — param handler:
//   r2 = (u16)((r2<<16) + 0xFFFF0000)>>16 = (u16)(param - 0x10000>>16 …)
//   (arithmetically: r2 = (u16)(param + 0xFFFF0000) = (u16)(param - 0x10000))
//   if r2 <= 1: _08002B368(4); _08002618(1, 0); u16[rec+20] = 0
void MenuE2E4_0800E2E4(void *rec, int a1, u32 param) {
    (void)a1;
    volatile u8 *r4 = (volatile u8 *)rec;
    u32 v = ((u32)param << 16) + 0xFFFF0000u;
    v >>= 16;
    if (v <= 1) {
        _0802B368(4);
        _08002618(1, 0);
        *(volatile u16 *)(r4 + 20) = 0;
    }
}
#ifndef __APPLE__
void _0800E2E4(void *a, int b, u32 c) __attribute__((alias("MenuE2E4_0800E2E4")));
void sub_0800E2E4(void *a, int b, u32 c) __attribute__((alias("MenuE2E4_0800E2E4")));
#endif

// ----------------------------------------------------------------------------
// sub_0800E598 — no-op leaf.
void MenuE598_0800E598(void) {
}
#ifndef __APPLE__
void _0800E598(void) __attribute__((alias("MenuE598_0800E598")));
void sub_0800E598(void) __attribute__((alias("MenuE598_0800E598")));
#endif

// ----------------------------------------------------------------------------
// sub_0800E59C — 12-entry command dispatcher (r0 = cmd, r3 = rec, r1 = arg,
// r2 = param). Jump table 0x0800E5B8, idx = cmd - 1:
//   1 -> DE50(rec)            2 -> DD9C(rec, arg)
//   5 -> D854(rec+32); D8E4(rec+136)
//   6 -> if u16[rec+36]!=0: (s16[rec+20]==0 ? E15C(rec,u16arg,u16param)
//                                           : E2E4(rec,u16arg,u16param))
//   7 -> E418(rec)
//   12 -> DE4C(rec)
//   else return.
void MenuE59C_0800E59C(int cmd, int arg, u32 param, void *rec) {
    volatile u8 *r4 = (volatile u8 *)rec;
    u32 idx = (u32)(cmd - 1);
    if (idx > 11)
        return;
    switch (idx) {
    case 0:
        _0800DE50(rec);
        break;
    case 1:
        _0800DD9C(rec, (u32)arg);
        break;
    case 4:
        _0800D854((void *)(r4 + 32));
        _0800D8E4((void *)(r4 + 136));
        break;
    case 5:
        if (*(volatile u16 *)(r4 + 36) != 0) {
            u32 a1 = ((u32)arg << 16) >> 16;
            u32 a2 = ((u32)param << 16) >> 16;
            if (((s16)*(volatile s16 *)(r4 + 20)) == 0)
                _0800E15C(rec, a1, a2);
            else
                _0800E2E4(rec, (int)a1, a2);
        }
        break;
    case 6:
        _0800E418(rec);
        break;
    case 11:
        _0800DE4C(rec);
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _0800E59C(int a, int b, u32 c, void *d) __attribute__((alias("MenuE59C_0800E59C")));
void sub_0800E59C(int a, int b, u32 c, void *d) __attribute__((alias("MenuE59C_0800E59C")));
#endif
