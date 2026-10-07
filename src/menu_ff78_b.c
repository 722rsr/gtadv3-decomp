// ============================================================================
// menu_ff78_b.c — reconstructed C for asm/menu_ff78.s (3 functions):
//
//   sub_080010CB8 (0x08010CB8) — car-select input handler (mode machine
//                                0..3 + cursor-apply mode machine 0/1/2)
//   sub_080011214 (0x08011214) — cursor-move handler (keymask bit gate)
//   sub_0800113A0 (0x080113A0) — apply-current-selection handler
//                                (0x109D4 tail dispatch, r1/r2 dead)
//
// Common epilogue shape: s16[rec+0xBC] = _080258B8(s16[rec+0xAC]);
// s16[rec+0xBE] = _0800F744(s16[rec+0xBC]); record placement via
// _080026F50(u32[rec+0xF0], s16[rec+0xAC], 7); u16[0x080CB55C + 2*s16[0xBC]]
// -> u32[rec+0x154]; _0800F7EC(s16[0xBC], s16[0xBE]) -> u32[rec+0x160];
// resource binds via _08007ABC; final u16[rec+0xB6] = u16[rec+0xAC].
//
// Transcribed instruction-for-instruction from asm/menu_ff78.s.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

HOST_STUB(void _08007ABC(void *rec, int id, int lane));     // 0x08007ABC resource bind

extern void _0800108D4(void *rec);            // menus.c Menu108D4
extern void _0800108F0(void *rec);            // menus.c Menu108F0
extern void _0802B368(u16 v);                 // sound.c SoundDeferredVol
extern s16  _0800F744(s32 c);                 // menus.c
extern u32  _0800F6D0(s32 c);                 // menus.c
extern u32  _0800F700(s32 a, s32 b);          // menus.c
extern s16  _0800F7EC(s32 a, s32 b);          // menus.c
extern void _0800F810(void *rec);             // menus.c record field refresh
extern s16  _080258B8(int idx);               // course_cal.c Course_GetCup
extern int  _08025A8C(int i);                 // code_25930.c GetEntry
extern int  _08025E1C(int id);                // ai_grid_leaves.c Ai_GridGetPacked
extern s16  _0800258A8(int idx);              // numeric_leaves.c 0x080CD7D0 lookup
extern u16  _08002178(int i);                 // block_b.c BlockB_DispatchIfLess
extern void _080026F50(u32 a, u32 b, u32 c, void *d); // garage.c Garage_PlaceRecord

#define CB55C 0x080CB55Cu

static inline u16 RD16(volatile u8 *rec, int off)  { return *(volatile u16 *)(rec + off); }
static inline void WR16(volatile u8 *rec, int off, u16 v) { *(volatile u16 *)(rec + off) = v; }
static inline u32  RD32(volatile u8 *rec, int off) { return *(volatile u32 *)(rec + off); }
static inline void WR32(volatile u8 *rec, int off, u32 v) { *(volatile u32 *)(rec + off) = v; }
static inline s16  RDS16(volatile u8 *rec, int off){ return *(volatile s16 *)(rec + off); }
static inline u16  tbl16(int off, s16 idx) {
    return *(volatile u16 *)(uintptr_t)(CB55C + off + (int)idx * 2);
}

// ============================================================================
// sub_080010CB8 — car-select input handler.
// ============================================================================
void MenuFF78_10CB8(void *rec_, int a1, int a2) {
    (void)a1;
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 r5 = (u16)a2;
    u16 r6 = RD16(rec, 168);                 // u16[rec+0xA8] at entry
    s16 v = RDS16(rec, 168);

    if (v == 0) {
        if (r5 == 2) _0800108D4(rec_);
        if (r5 == 1) {
            WR16(rec, 168, (u16)(r6 + 1));
            _0802B368(1);
            if (RD16(rec, 172) != RD16(rec, 176)) {
                WR16(rec, 198, 99);          // u16[rec+0xC6] = 99
                WR16(rec, 200, 99);          // u16[rec+0xC8] = 99
            }
            WR16(rec, 176, RD16(rec, 172));  // u16[rec+0xB0] = u16[rec+0xAC]
        }
    } else if (v == 1 || v == 2) {
        if (r5 == 2) {
            WR16(rec, 168, (u16)(r6 - 1));
            _0802B368(4);
        }
        if (r5 == 1) {
            WR16(rec, 168, (u16)(r6 + 1));
            _0802B368(1);
        }
    } else if (v == 3) {
        if (r5 == 2) {
            WR16(rec, 168, (u16)(r6 - 1));
            _0802B368(4);
        }
        if (r5 == 1) _0800108F0(rec_);
    }

    // common tail: clamp, skip if unchanged, then apply per new mode
    if (RDS16(rec, 168) > 4) WR16(rec, 168, 4);
    if (RD16(rec, 168) == r6) return;
    v = RDS16(rec, 168);
    if (v == 0) {
        WR16(rec, 190, 99);                  // u16[rec+0xBE] = 99
        WR16(rec, 192, 99);                  // u16[rec+0xC0] = 99
        _0800F810(rec_);
        return;
    } else if (v == 2) {
        // mode 2: rec+0xC0 pick from rec+0xC8 mirror / zero
        if (RD16(rec, 172) == RD16(rec, 176)) {
            u16 t = RD16(rec, 200);
            WR16(rec, 192, t);
            if ((s16)t == 99) WR16(rec, 192, 0);
        } else {
            WR16(rec, 192, 0);
        }
        WR32(rec, 364, tbl16(20, RDS16(rec, 192)));   // u32[rec+0x16C]
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 364), (int)RD32(rec, 360));
        WR32(rec, 376, RD16((volatile u8 *)(uintptr_t)CB55C, 20)); // u32[rec+0x178] = u16[0x080CB570]
        WR32(rec, 388, RD16((volatile u8 *)(uintptr_t)CB55C, 22)); // u32[rec+0x184] = u16[0x080CB572]
        WR32(rec, 400, RD16((volatile u8 *)(uintptr_t)CB55C, 24)); // u32[rec+0x190] = u16[0x080CB574]
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 376), (int)RD32(rec, 348));
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 388), (int)RD32(rec, 384));
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 400), (int)RD32(rec, 396));
        _0800F810(rec_);
        return;
    }

    // mode 1: rec+0xBE pick, record list fill loop
    if (RD16(rec, 172) == RD16(rec, 176)) {
        u16 t = RD16(rec, 198);
        WR16(rec, 190, t);
        if ((s16)t == 99) {
            s16 r4 = RDS16(rec, 188);
            u32 r0 = _0800F700(r4, _0800F744(0));
            WR16(rec, 190, (u16)r0);
        }
    } else {
        s16 r4 = RDS16(rec, 188);
        u32 r0 = _0800F700(r4, _0800F744(0));
        WR16(rec, 190, (u16)r0);
    }

    // list fill: for r6 = 0.. _0800F6D0(s16[rec+0xBC])-1
    for (u32 r6 = 0, r4 = 0; r6 < _0800F6D0(RDS16(rec, 188)); r6++, r4 += 12) {
        u32 r0 = _0800F700(RDS16(rec, 188), (s32)r6);
        u16 w = tbl16(10, (s16)r0);          // u16[0x080CB566 + 2*v]
        WR32(rec, 376 + (int)r4, w);         // u32[rec+0x178 + r4]
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)w, (int)RD32(rec, 372 + (int)r4));
    }
    if (RD16(rec, 190) != 99) {
        s16 r0 = _0800F7EC(RDS16(rec, 188), RDS16(rec, 190));
        WR32(rec, 352, (u32)(u16)(u32)(s16)r0);   // u32[rec+0x160] = (s16)result
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 352), (int)RD32(rec, 348));
    }
    _0800F810(rec_);
}
#ifndef __APPLE__
void _080010CB8(void *a, int b, int c) __attribute__((alias("MenuFF78_10CB8")));
void sub_080010CB8(void *a, int b, int c) __attribute__((alias("MenuFF78_10CB8")));
#endif

// ============================================================================
// sub_080011214 — cursor-move handler (keymask 0x20 = down, 0x10 = up).
// ============================================================================
void MenuFF78_11214(void *rec_, int a1, int a2) {
    (void)a2;
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 keys = (u16)a1;

    if (keys & 32) {
        if (RDS16(rec, 186) <= 0) {
            WR16(rec, 180, (u16)(RD16(rec, 180) - 1));   // u16[rec+0xB4]--
            WR16(rec, 186, 10);                          // s16[rec+0xBA] = 10
        }
    }
    if (keys & 16) {
        if (RDS16(rec, 186) <= 0) {
            WR16(rec, 180, (u16)(RD16(rec, 180) + 1));
            WR16(rec, 186, 10);
        }
    }
    // repeat counter
    u16 c = (u16)(RD16(rec, 186) - 1);
    WR16(rec, 186, c);
    if ((s32)((u32)c << 16) <= 0) WR16(rec, 186, 0);
    // clamp s16[rec+0xB4] against s16[rec+0xB8]
    if (RDS16(rec, 180) >= RDS16(rec, 184)) WR16(rec, 180, RD16(rec, 184));
    if (RDS16(rec, 180) < 0) WR16(rec, 180, 0);

    int v = _08025A8C(RDS16(rec, 180));
    WR16(rec, 178, (u16)v);                  // s16[rec+0xB2]

    if (_08002178(0) != _08002178(1)) return;    // (0,2) vs (1,2)

    if (RD16(rec, 182) != RD16(rec, 172)) {
        _0802B368(3);
        WR16(rec, 172, (u16)_08025A8C(RDS16(rec, 180)));   // u16[rec+0xAC]
        int f = _080258B8(RDS16(rec, 172));
        WR16(rec, 188, (u16)f);              // s16[rec+0xBC]
        f = _0800F744(RDS16(rec, 188));
        WR16(rec, 190, (u16)f);              // s16[rec+0xBE]
        WR16(rec, 194, 0);                   // s16[rec+0xC2] = 0
        s16 a8 = _0800258A8(RDS16(rec, 172));
        WR32(rec, 232, (u32)(u16)(u32)(s16)a8);            // u32[rec+0xE8]
        _08007ABC((void *)(uintptr_t)RD32(rec, 28), (int)RD32(rec, 228), 0);
        WR32(rec, 244, (u32)(u16)(u32)RDS16(rec, 172));    // u32[rec+0xF4] = s16[rec+0xAC]
        _080026F50(RD32(rec, 240), (u32)(u16)(u32)RDS16(rec, 172), 7, 0);
        WR32(rec, 340, tbl16(0, RDS16(rec, 188)));         // u32[rec+0x154]
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
        s16 e = _0800F7EC(RDS16(rec, 188), RDS16(rec, 190));
        WR32(rec, 352, (u32)(u16)(u32)(s16)e);             // u32[rec+0x160]
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 352), (int)RD32(rec, 348));
    }
    WR16(rec, 182, RD16(rec, 172));          // u16[rec+0xB6] = u16[rec+0xAC]
}
#ifndef __APPLE__
void _080011214(void *a, int b, int c) __attribute__((alias("MenuFF78_11214")));
void sub_080011214(void *a, int b, int c) __attribute__((alias("MenuFF78_11214")));
#endif

// ============================================================================
// sub_0800113A0 — apply-current-selection handler (0x109D4 tail; r1/r2 dead).
// ============================================================================
void MenuFF78_113A0(void *rec_, int a1, int a2) {
    (void)a1; (void)a2;
    volatile u8 *rec = (volatile u8 *)rec_;

    int f = _080258B8(RDS16(rec, 172));
    WR16(rec, 188, (u16)f);                  // s16[rec+0xBC]
    f = _0800F744(RDS16(rec, 188));
    WR16(rec, 190, (u16)f);                  // s16[rec+0xBE]

    if (_08002178(0) == _08002178(1)) {      // (0,2) vs (1,2)
        if (RD16(rec, 182) != RD16(rec, 172)) {
            s16 a8 = _0800258A8(RDS16(rec, 172));
            WR32(rec, 232, (u32)(u16)(u32)(s16)a8);        // u32[rec+0xE8]
            _08007ABC((void *)(uintptr_t)RD32(rec, 28), (int)RD32(rec, 228), 0);
            WR32(rec, 244, (u32)(u16)(u32)RDS16(rec, 172)); // u32[rec+0xF4]
            _080026F50(RD32(rec, 240), (u32)(u16)(u32)RDS16(rec, 172), 7, 0);
            WR32(rec, 340, tbl16(0, RDS16(rec, 188)));      // u32[rec+0x154]
            _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
            s16 e = _0800F7EC(RDS16(rec, 188), RDS16(rec, 190));
            WR32(rec, 352, (u32)(u16)(u32)(s16)e);          // u32[rec+0x160]
            _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 352), (int)RD32(rec, 348));
        }
        WR16(rec, 182, RD16(rec, 172));      // u16[rec+0xB6] = u16[rec+0xAC]
    }
}
#ifndef __APPLE__
void _0800113A0(void *a, int b, int c) __attribute__((alias("MenuFF78_113A0")));
void sub_0800113A0(void *a, int b, int c) __attribute__((alias("MenuFF78_113A0")));
#endif
