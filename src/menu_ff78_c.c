// ============================================================================
// menu_ff78_c.c — reconstructed C for asm/menu_ff78.s no-op `bx lr` stubs +
// screen-setup cursor handlers.
//
//   sub_080012574 (0x080012574) — bx lr no-op (4B)
//   sub_080012850 (0x080012850) — bx lr no-op (4B)
//   sub_080012F14 (0x080012F14) — bx lr no-op (4B)
//   sub_0800133E0 (0x0800133E0) — bx lr no-op (4B)
//   sub_080013E60 (0x080013E60) — bx lr no-op (4B)
//   sub_080014074 (0x080014074) — bx lr no-op (4B; tail of sub_080014072)
//   sub_0800141A8 (0x0800141A8) — bx lr no-op (4B)
//   sub_080014BCC (0x080014BCC) — bx lr no-op (4B)
//   sub_080014C84 (0x080014C84) — bx lr no-op (4B)
//   sub_080015CB0 (0x080015CB0) — bx lr no-op (4B; rec35_driver.s `bl 0x08015CB0`)
//
//   sub_080010F3C (0x080010F3C) — cursor-move handler w/ s16[rec+0xC2]=0
//                                  (mirrors 0x11214 but uses _08025A8C/0x258B8)
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

// ---- no-op stubs ----------------------------------------------------------
void _080012574(volatile void *rec) { (void)rec; }
void _080012850(void) { }
void _080012F14(void) { }
void _0800133E0(void) { }
void _080013E60(void) { }
void _080014074(void) { }
void _0800141A8(void) { }
void _080014BCC(void) { }
void _080014C84(void) { }
void _080015CB0(void) { }

#ifndef __APPLE__
void sub_080012574(volatile void *rec) __attribute__((alias("_080012574")));
void sub_080012850(void) __attribute__((alias("_080012850")));
void sub_080012F14(void) __attribute__((alias("_080012F14")));
void sub_0800133E0(void) __attribute__((alias("_0800133E0")));
void sub_080013E60(void) __attribute__((alias("_080013E60")));
void sub_080014074(void) __attribute__((alias("_080014074")));
void sub_0800141A8(void) __attribute__((alias("_0800141A8")));
void sub_080014BCC(void) __attribute__((alias("_080014BCC")));
void sub_080014C84(void) __attribute__((alias("_080014C84")));
void sub_080015CB0(void) __attribute__((alias("_080015CB0")));
void Sub_080012574(volatile void *rec) __attribute__((alias("_080012574")));
void Sub_080012850(void) __attribute__((alias("_080012850")));
void Sub_080012F14(void) __attribute__((alias("_080012F14")));
void Sub_0800133E0(void) __attribute__((alias("_0800133E0")));
void Sub_080013E60(void) __attribute__((alias("_080013E60")));
void Sub_080014074(void) __attribute__((alias("_080014074")));
void Sub_0800141A8(void) __attribute__((alias("_0800141A8")));
void Sub_080014BCC(void) __attribute__((alias("_080014BCC")));
void Sub_080014C84(void) __attribute__((alias("_080014C84")));
void Sub_080015CB0(void) __attribute__((alias("_080015CB0")));
#endif

// ---- sub_080010F3C: cursor-move handler ----------------------------------
extern int  _08025A8C(int i);            // code_25930.c GetEntry
extern s16  _080258B8(int idx);          // course_cal.c Course_GetCup
extern s16  _0800F744(s32 c);            // menus.c
extern int  _08025E1C(int id);           // ai_grid_leaves.c Ai_GridGetPacked
extern void _0800F810(void *rec);        // menus.c record field refresh
extern s16  _0800258A8(int idx);         // numeric_leaves.c Code258A8
extern void _08007ABC(void *rec, int id, int lane); // resource bind
extern void _0802B368(u16 v);            // sound.c SoundDeferredVol
extern void _080026F50(u32 a, u32 b, u32 c, void *d); // garage.c Garage_PlaceRecord
extern s16  _0800F7EC(s32 a, s32 b);     // menus.c

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
// sub_080010F3C — cursor-move handler with s16[rec+0xC2]=0 after apply.
// r0 = rec, r1 = keymask (16/32 = up/down). Same epilogue shape as 0x11214
// but uses _08025A8C (vs 0x11214's _08025A8C, identical) and adds a
// s16[rec+0xC2]=0 write + the 0xE8/0xF4 rec placement sequence.
// ============================================================================
void MenuFF78_10F3C(void *rec_, int a1, int a2) {
    (void)a2;
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 keys = (u16)a1;

    u16 r5 = RD16(rec, 180);                          // u16[rec+0xB4] saved
    if (keys & 32) {
        if (RDS16(rec, 186) <= 0) {
            WR16(rec, 180, (u16)(r5 - 1));
            WR16(rec, 186, 10);
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

    // bail if unchanged
    s16 r5s = (s16)r5;
    s16 cur = RDS16(rec, 180);
    if (r5s == cur) goto _end;
    if (cur == 99) goto _end;

    _0802B368(3);
    s16 v = (s16)_08025A8C(cur);
    WR16(rec, 172, (u16)v);                           // s16[rec+0xAC]
    s16 f = _080258B8(RDS16(rec, 172));
    WR16(rec, 188, (u16)f);                           // s16[rec+0xBC]
    s16 f2 = _0800F744(RDS16(rec, 188));
    WR16(rec, 190, (u16)f2);                          // s16[rec+0xBE]
    WR16(rec, 194, 0);                                // s16[rec+0xC2] = 0
    int ge = _08025E1C(cur);
    (void)ge;
    _0800F810(rec_);
    s16 a8 = _0800258A8(RDS16(rec, 172));
    WR32(rec, 232, (u32)(u16)(u32)(s16)a8);           // u32[rec+0xE8]
    _08007ABC((void *)(uintptr_t)RD32(rec, 28), (int)RD32(rec, 228), 0);
    WR32(rec, 244, (u32)(u16)(u32)RDS16(rec, 172));   // u32[rec+0xF4]
    _080026F50(RD32(rec, 240), (u32)(u16)(u32)RDS16(rec, 172), 7, 0);
    // u32[rec+0x154] from table column 0 (was: tbl16(0, RDS16(rec, 188)))
    WR32(rec, 340, tbl16(0, RDS16(rec, 188)));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
    s16 e = _0800F7EC(RDS16(rec, 188), RDS16(rec, 190));
    WR32(rec, 352, (u32)(u16)(u32)(s16)e);            // u32[rec+0x160]
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 352), (int)RD32(rec, 348));
_end:
    ;
}
#ifndef __APPLE__
void _080010F3C(void *a, int b, int c) __attribute__((alias("MenuFF78_10F3C")));
void sub_080010F3C(void *a, int b, int c) __attribute__((alias("MenuFF78_10F3C")));
#endif

// ============================================================================
// sub_0800110A0 — cursor-move handler w/ _0802584C (vs 0x10F3C's _08025A8C)
// and the _0800F8B4 selector rebind step. Same epilogue shape.
// r0 = rec, r1 = keymask (16/32 = up/down).
// ============================================================================
extern s16  _0802584C(int a);             // ai_line_more.c Ai_LineLookupA
extern void _0800F8B4(void *rec, u32 sel); // menus.c selector rebind
void MenuFF78_110A0(void *rec_, int a1, int a2) {
    (void)a2;
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 keys = (u16)a1;

    u16 r5 = RD16(rec, 180);                          // u16[rec+0xB4] saved
    if (keys & 32) {
        if (RDS16(rec, 186) <= 0) {
            WR16(rec, 180, (u16)(r5 - 1));
            WR16(rec, 186, 10);
        }
    }
    if (keys & 16) {
        if (RDS16(rec, 186) <= 0) {
            WR16(rec, 180, (u16)(RD16(rec, 180) + 1));
            WR16(rec, 186, 10);
        }
    }
    u16 c = (u16)(RD16(rec, 186) - 1);
    WR16(rec, 186, c);
    if ((s32)((u32)c << 16) <= 0) WR16(rec, 186, 0);

    if (RDS16(rec, 180) >= RDS16(rec, 184)) WR16(rec, 180, RD16(rec, 184));
    if (RDS16(rec, 180) < 0) WR16(rec, 180, 0);

    s16 r5s = (s16)r5;
    s16 cur = RDS16(rec, 180);
    if (r5s == cur) goto _end;
    if (cur == 99) goto _end;

    _0802B368(3);
    // r8 was loaded with r5 but here we just use the s16 r5 value as initial
    // Note: asm uses r5 (saved u16[rec+0xB4]) to compute r8 via ldrh/strh —
    // we use cur consistently.
    s16 v = _0802584C(cur);
    WR16(rec, 172, (u16)v);                           // s16[rec+0xAC]
    s16 f = _080258B8(RDS16(rec, 172));
    WR16(rec, 188, (u16)f);                           // s16[rec+0xBC]
    s16 f2 = _0800F744(RDS16(rec, 188));
    WR16(rec, 190, (u16)f2);                          // s16[rec+0xBE]
    WR16(rec, 194, 0);                                // s16[rec+0xC2] = 0
    _0800F810(rec_);
    s16 a8 = _0800258A8(RDS16(rec, 172));
    WR32(rec, 232, (u32)(u16)(u32)(s16)a8);           // u32[rec+0xE8]
    _08007ABC((void *)(uintptr_t)RD32(rec, 28), (int)RD32(rec, 228), 0);
    // ldrh r1, [r7] (s16[rec+0xC2]); subs r1, #1; _0800F8B4(rec, (s16)r1)
    s16 sel = (s16)((u16)RD16(rec, 194) - 1);
    _0800F8B4(rec_, (u32)(u16)(u32)sel);
    WR32(rec, 244, (u32)(u16)(u32)RDS16(rec, 172));   // u32[rec+0xF4]
    _080026F50(RD32(rec, 240), (u32)(u16)(u32)RDS16(rec, 172), 7, 0);
    WR32(rec, 340, tbl16(0, RDS16(rec, 188)));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
    s16 e = _0800F7EC(RDS16(rec, 188), RDS16(rec, 190));
    WR32(rec, 352, (u32)(u16)(u32)(s16)e);            // u32[rec+0x160]
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 352), (int)RD32(rec, 348));
_end:
    ;
}
#ifndef __APPLE__
void _0800110A0(void *a, int b, int c) __attribute__((alias("MenuFF78_110A0")));
void sub_0800110A0(void *a, int b, int c) __attribute__((alias("MenuFF78_110A0")));
#endif

// ============================================================================
// sub_080011484 — cursor-move handler w/ _0802581C (Ai_LineFindCourse) for
// s16[rec+0xC2]; otherwise similar epilogue (F8B4 rebind + 0xE8/0xF4 placement
// + 0x154/0x160/0x168 from 0x080CB55C, etc.).
// ============================================================================
extern int  _0802581C(int a);             // ai_line_more.c Ai_LineFindCourse
void MenuFF78_11484(void *rec_, int a1, int a2) {
    (void)a2;
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 keys = (u16)a1;

    u16 r6 = RD16(rec, 180);                          // u16[rec+0xB4] saved (r6 here, was r5 elsewhere)
    if (keys & 32) {
        if (RDS16(rec, 186) <= 0) {
            WR16(rec, 180, (u16)(r6 - 1));
            WR16(rec, 186, 10);
        }
    }
    if (keys & 16) {
        if (RDS16(rec, 186) <= 0) {
            WR16(rec, 180, (u16)(RD16(rec, 180) + 1));
            WR16(rec, 186, 10);
        }
    }
    u16 c = (u16)(RD16(rec, 186) - 1);
    WR16(rec, 186, c);
    if ((s32)((u32)c << 16) <= 0) WR16(rec, 186, 0);

    if (RDS16(rec, 180) >= RDS16(rec, 184)) WR16(rec, 180, RD16(rec, 184));
    if (RDS16(rec, 180) < 0) WR16(rec, 180, 0);

    s16 r6s = (s16)r6;
    s16 cur = RDS16(rec, 180);
    // unconditional tail (no early-bail in this variant): always do apply
    s16 v = (s16)_08025A8C(cur);
    WR16(rec, 172, (u16)v);                           // s16[rec+0xAC]
    s16 f = _080258B8(RDS16(rec, 172));
    WR16(rec, 188, (u16)f);                           // s16[rec+0xBC]
    // _0802581C(_080258B8(s16[rec+0xAC])) lsls asrs then _08025E1C
    s16 c2 = (s16)(s32)_0802581C((s32)(u16)RDS16(rec, 172));
    s16 c2b = (s16)(s32)_08025E1C((int)c2);
    WR16(rec, 194, (u16)c2b);                         // s16[rec+0xC2]
    (void)r6s;                                        // r6 used only for early-out check below
    if ((s16)r6 == cur || cur == 99) goto _rebind;
    _0802B368(3);
    _0800F810(rec_);
_rebind:
    {
        s16 sel = (s16)((u16)RD16(rec, 194) - 1);
        _0800F8B4(rec_, (u32)(u16)(u32)sel);
    }
    // 0xE8 = _080258A8(s16[rec+0xAC])
    s16 a8 = _0800258A8(RDS16(rec, 172));
    WR32(rec, 232, (u32)(u16)(u32)(s16)a8);
    _08007ABC((void *)(uintptr_t)RD32(rec, 28), (int)RD32(rec, 228), 0);
    // 0x154 = u16[0x080CB55C + 2 * (s16) _080258B8(s16[rec+0xAC])]
    // (note: asm uses lsls r0, r0, #1; ldr r1, _0800115DC -> asrs r0, #15;
    //  adds r0, r0, r1; ldrh r0, [r0] => r0 = u16[CB55C + 2*idx])
    {
        s16 ac = RDS16(rec, 172);
        s32 v2 = (s32)(u16)_080258B8(ac);
        u16 w = *(volatile u16 *)(uintptr_t)(CB55C + 2 * v2);
        WR32(rec, 340, w);                            // u32[rec+0x154]
    }
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
    WR32(rec, 244, (u32)(u16)(u32)RDS16(rec, 172));   // u32[rec+0xF4]
    _080026F50(RD32(rec, 240), (u32)(u16)(u32)RDS16(rec, 172), 7, 0);
}
#ifndef __APPLE__
void _080011484(void *a, int b, int c) __attribute__((alias("MenuFF78_11484")));
void sub_080011484(void *a, int b, int c) __attribute__((alias("MenuFF78_11484")));
#endif

// ============================================================================
// sub_0800115E0 — list-cursor handler w/ u16[rec+190] cursor over the record
// list. Uses _0800F744/F700/F6D0 for clamp + select logic. Same epilogue shape
// (s16[rec+212] = selected idx).
// ============================================================================
extern u32 _0800F6D0(s32 idx);            // menus.c
extern u32 _0800F700(s32 a, s32 b);       // menus.c
void MenuFF78_115E0(void *rec_, int a1, int a2) {
    (void)a1;
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 k = (u16)a2;
    u16 r8v = RD16(rec, 190);                            // u16[rec+190]
    if (k == 32) WR16(rec, 190, (u16)(r8v - 1));
    if (k == 16) WR16(rec, 190, (u16)(r8v + 1));

    // 99 → use _0800F744(0) selector path through _0800F700
    if (RD16(rec, 190) == 99) {
        s16 bc = RDS16(rec, 188);                         // s16[rec+0xBC]
        int x = (int)_0800F744(0);
        u32 r0 = _0800F700((s32)bc, (s32)x);
        WR16(rec, 190, (u16)r0);
        r8v = RD16(rec, 190);
    }
    s16 cur = RDS16(rec, 190);
    u32 f6d0 = _0800F6D0(RDS16(rec, 188));
    if (cur >= (s16)f6d0 && RD16(rec, 190) != 99) {
        WR16(rec, 190, (u16)(f6d0 - 1));
    }
    if (RDS16(rec, 190) < 0) WR16(rec, 190, 0);
    WR16(rec, 198, RD16(rec, 190));                       // u16[rec+198] = u16[rec+190]
    s16 r8s = (s16)r8v;
    if (r8s == RDS16(rec, 190)) goto _end;
    if (RDS16(rec, 190) == 99) goto _end;
    _0802B368(3);
    {
        s16 e = _0800F7EC(RDS16(rec, 188), RDS16(rec, 190));
        WR32(rec, 348, (u32)(u16)(u32)(s16)e);            // u32[rec+0x15C]
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 348), (int)RD32(rec, 344));
        _0800F810(rec_);
    }
_end:
    {
        // branch: u16[rec+188] == 1 -> _0800F700(..)-1, else _0800F700(..)
        u16 h188 = RD16(rec, 188);
        u32 r0;
        if (h188 == 1) {
            r0 = _0800F700((s32)(s16)h188, (s32)RDS16(rec, 190)) - 1;
        } else {
            r0 = _0800F700((s32)(s16)h188, (s32)RDS16(rec, 190));
        }
        WR16(rec, 212, (u16)r0);                          // u16[rec+0xD4]
    }
}
#ifndef __APPLE__
void _0800115E0(void *a, int b, int c) __attribute__((alias("MenuFF78_115E0")));
void sub_0800115E0(void *a, int b, int c) __attribute__((alias("MenuFF78_115E0")));
#endif

// ============================================================================
// sub_0800116DC — small cursor handler w/ u16[rec+192] (clamped 0..2, 99→0),
// mirrors to u16[rec+200], and the same epilogue 0x15C bind + s16[rec+212].
// ============================================================================
void MenuFF78_116DC(void *rec_, int a1, int a2) {
    (void)a1;
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 k = (u16)a2;
    if (k == 32) WR16(rec, 192, (u16)(RD16(rec, 192) - 1));
    if (k == 16) WR16(rec, 192, (u16)(RD16(rec, 192) + 1));
    u16 r3 = 0;
    if (RD16(rec, 192) == 99) {
        WR16(rec, 192, 0);
        r3 = 0;
    }
    s16 cur = RDS16(rec, 192);
    if (cur > 2 && cur != 99) WR16(rec, 192, 2);
    if (RDS16(rec, 192) < 0) WR16(rec, 192, 0);
    WR16(rec, 200, RD16(rec, 192));                      // u16[rec+200] = u16[rec+192]
    s16 r3s = (s16)r3;
    if (r3s == RDS16(rec, 192)) goto _end;
    if (RDS16(rec, 192) == 99) goto _end;
    _0802B368(3);
    {
        // u32[rec+0x164] = u16[0x080CB570 + 2 * s16[rec+192]]
        // (asm: ldrh r1, [r1, #0]; r1 += 20; r1 + r0*2 -> ldrh r1, [r0, #0];
        //  str r1, [r2, #0])
        u16 w = *(volatile u16 *)(uintptr_t)(CB55C + 20 + 2 * (int)RDS16(rec, 192));
        WR32(rec, 356, w);                                // u32[rec+0x164]
        _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 356), (int)RD32(rec, 352));
        _0800F810(rec_);
    }
_end:
    WR16(rec, 212, RD16(rec, 192));                       // u16[rec+0xD4]
}
#ifndef __APPLE__
void _0800116DC(void *a, int b, int c) __attribute__((alias("MenuFF78_116DC")));
void sub_0800116DC(void *a, int b, int c) __attribute__((alias("MenuFF78_116DC")));
#endif
