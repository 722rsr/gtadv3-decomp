// ============================================================================
// menu_ff78_a.c — reconstructed C for asm/menu_ff78.s (7 functions):
//
//   sub_0800101FC (0x080101FC) — course-setup twin A (s16[rec+0xA8] = 3)
//   sub_080010370 (0x08010370) — course-setup twin B (grid rebuild + 0x576 pick)
//   sub_0800104F0 (0x080104F0) — course-setup twin C (special-array rebuild)
//   sub_080010644 (0x08010644) — course-setup twin D (BlockB arm 0x138D)
//   sub_0800107CC (0x080107CC) — course-setup twin E (99 seeds + WA mirrors)
//   sub_08001096C (0x0801096C) — mode-0/3 input handler (car select)
//   sub_0800109D4 (0x080109D4) — gate-dispatched input handler
//                                (0x08002158 event forwards + mode machine)
//
// The five setup twins share a common tail: car-catalog accessors fill
// s16 fields {0xAC,0xBC,0xBE,0xC0,0xC2}; rec+0x118/0x124 get phase tags;
// sub_0800F8B4(rec, s16[rec+0xC2]-1) rebinds the selector; u16 words from
// ROM table 0x080CB55C (columns +0/+10/+20, s16-indexed) are stored to
// rec+0x154/0x160/0x16C; then resource binds via _08007ABC.
//
// Transcribed instruction-for-instruction from asm/menu_ff78.s (objdump
// cross-checked). WA = 0x03001780 (car/records work area).
// ============================================================================

#include "gba/types.h"

// ---- callees ---------------------------------------------------------------
#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

HOST_STUB(void _08007ABC(void *rec, int id, int lane));     // 0x08007ABC resource bind
HOST_STUB(void _08002618(u32 a, u32 b));                    // 0x08002618 scene event
HOST_STUB(void _0800113A0(void *rec, int a1, int a2));      // menu_ff78.s (implementation incomplete)
HOST_STUB(void _080011214(void *rec, int a1, int a2));      // menu_ff78.s (implementation incomplete)

extern int  _080256D8(int a, int b, int c);   // car-catalog field hw0 (ai_line_leaves.c)
extern int  _080256F4(int a, int b, int c);   // car-catalog field hw2
extern int  _08025710(int a, int b, int c);   // car-catalog field hw1
extern s16  _0802572C(int a, int b, int c);   // car-catalog -> 0x08060124 course id
extern int  _08025CF4(int type, int row, int col); // zone-grid get
extern void _0800F8B4(void *rec, u32 sel);    // menus.c selector rebind
extern s16  _080258B8(int idx);               // course_cal.c Course_GetCup
extern s16  _0800F744(s32 c);                 // menus.c
extern void _0800F810(void *rec);             // menus.c record field refresh
extern int  _08025A9C(int val);               // code_25930.c FindEntry (0x0203F9B0)
extern int  _08025A8C(int i);                 // code_25930.c GetEntry
extern int  _08025AFC(void);                  // code_25930.c GetSpecialCount
extern int  _08025B08(int i);                 // code_25930.c GetSpecial
extern int  _08025B18(int val);               // code_25930.c FindSpecial
extern void _080025930(void);                 // code_25930.c BuildGrid
extern void _080025AC0(void);                 // code_25930.c BuildSpecial
extern int  _08025E1C(int id);                // ai_grid_leaves.c Ai_GridGetPacked
extern void _08002124(u16 v);                 // block_b.c BlockB_Arm
extern void _08002158(int id, u16 payload);   // block_b.c BlockB_ForwardEvent
extern u16  _08002178(int i);                 // block_b.c BlockB_DispatchIfLess
extern void _0800108D4(void *rec);            // menus.c Menu108D4
extern void _0800108F0(void *rec);            // menus.c Menu108F0
extern void _0802B368(u16 v);                 // sound.c SoundDeferredVol

// 0x08025A80 — s16[0x0203FA40] leaf (canonical body in code_25930.c).
int MenuFF78_25A80(void) {
    extern int _08025A80(void);
    return _08025A80();
}

// ---- shared anchors --------------------------------------------------------
#define WA        0x03001780u
#define WA_S16(o) (*(volatile s16 *)(uintptr_t)(WA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WA + (o)))
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WA + (o)))
#define CB55C     0x080CB55Cu   // u16 table (s16-indexed columns +0/+10/+20)

static inline u16 RD16(volatile u8 *rec, int off)  { return *(volatile u16 *)(rec + off); }
static inline void WR16(volatile u8 *rec, int off, u16 v) { *(volatile u16 *)(rec + off) = v; }
static inline u32  RD32(volatile u8 *rec, int off) { return *(volatile u32 *)(rec + off); }
static inline void WR32(volatile u8 *rec, int off, u32 v) { *(volatile u32 *)(rec + off) = v; }
static inline s16  RDS16(volatile u8 *rec, int off){ return *(volatile s16 *)(rec + off); }
static inline void PWR16(volatile u8 *p, u16 v)    { *(volatile u16 *)(uintptr_t)p = v; }
static inline u16  tbl16(int off, s16 idx) {
    return *(volatile u16 *)(uintptr_t)(CB55C + off + (int)idx * 2);
}

// ============================================================================
// sub_0800101FC — setup twin A: s16[rec+0xA8] = 3, catalog fill, binds.
// ============================================================================
void MenuFF78_101FC(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    WR16(rec, 168, 3);                       // s16[rec+0xA8] = 3
    int f;
    f = _080256D8(WA_S16(0xFF6), WA_S16(0x103A), 0);
    WR16(rec, 172, (u16)f);                  // s16[rec+0xAC]
    f = _080256F4(WA_S16(0xFF6), WA_S16(0x103A), 0);
    WR16(rec, 190, (u16)f);                  // s16[rec+0xBE]
    f = _0802572C(WA_S16(0xFF6), WA_S16(0x103A), 0);
    WR16(rec, 188, (u16)f);                  // s16[rec+0xBC]
    f = _08025710(WA_S16(0xFF6), WA_S16(0x103A), 0);
    WR16(rec, 192, (u16)f);                  // s16[rec+0xC0]
    f = _08025CF4(WA_S16(0xFF2), WA_S16(0xFF6), WA_S16(0x103A));
    WR16(rec, 194, (u16)f);                  // s16[rec+0xC2]
    WR32(rec, 280, 5);                       // u32[rec+0x118] = 5
    WR32(rec, 292, 2);                       // u32[rec+0x124] = 2
    s16 v = (s16)(u16)(RD16(rec, 194) - 1);  // ldrh/subs/lsls/asrs
    _0800F8B4(rec_, (u32)v);
    WR32(rec, 340, tbl16(0,  RDS16(rec, 172)));   // u32[rec+0x154]
    WR32(rec, 352, tbl16(10, RDS16(rec, 190)));   // u32[rec+0x160]
    WR32(rec, 364, tbl16(20, RDS16(rec, 192)));   // u32[rec+0x16C]
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 280), (int)RD32(rec, 276));
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 292), (int)RD32(rec, 288));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 352), (int)RD32(rec, 348));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 364), (int)RD32(rec, 360));
}
#ifndef __APPLE__
void _0800101FC(void *a) __attribute__((alias("MenuFF78_101FC")));
void sub_0800101FC(void *a) __attribute__((alias("MenuFF78_101FC")));
#endif

// ============================================================================
// sub_080010370 — setup twin B: rebuild grid, pick entry from s16[WA+0x576].
// ============================================================================
void MenuFF78_10370(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    WR32(rec, 280, 4);                       // u32[rec+0x118] = 4
    WR32(rec, 292, 2);                       // u32[rec+0x124] = 2
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 280), (int)RD32(rec, 276));
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 292), (int)RD32(rec, 288));
    _080025930();                            // rebuild car-collection grid
    WR16(rec, 168, 0);                       // s16[rec+0xA8] = 0
    int v = _08025A9C(WA_S16(0x576));
    WR16(rec, 180, (u16)v);                  // s16[rec+0xB4]
    v = _08025A8C(RDS16(rec, 180));
    WR16(rec, 172, (u16)v);                  // s16[rec+0xAC]
    v = MenuFF78_25A80();                    // 0x08025A80
    WR16(rec, 184, (u16)v);                  // s16[rec+0xB8]
    v = _080258B8(RDS16(rec, 172));
    WR16(rec, 188, (u16)v);                  // s16[rec+0xBC]
    v = _0800F744(RDS16(rec, 188));
    WR16(rec, 190, (u16)v);                  // s16[rec+0xBE]
    WR16(rec, 192, 0);                       // s16[rec+0xC0] = 0
    v = _08025E1C(RDS16(rec, 180));
    WR16(rec, 194, (u16)v);                  // s16[rec+0xC2]
    WR32(rec, 280, 4);                       // re-store 4
    WR32(rec, 292, 2);                       // re-store 2
    v = _080258B8(RDS16(rec, 172));
    WR32(rec, 340, tbl16(0, (s16)v));
    WR32(rec, 352, tbl16(10, RDS16(rec, 190)));
    WR32(rec, 364, tbl16(20, RDS16(rec, 192)));
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 280), (int)RD32(rec, 276));
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 292), (int)RD32(rec, 288));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 352), (int)RD32(rec, 348));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 364), (int)RD32(rec, 360));
    _0800F810(rec_);
}
#ifndef __APPLE__
void _080010370(void *a) __attribute__((alias("MenuFF78_10370")));
void sub_080010370(void *a) __attribute__((alias("MenuFF78_10370")));
#endif

// ============================================================================
// sub_0800104F0 — setup twin C: special-array rebuild + index pick.
// ============================================================================
void MenuFF78_104F0(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    _080025AC0();                            // rebuild 0x0203FA30 specials
    WR16(rec, 168, 0);                       // s16[rec+0xA8] = 0
    int v = _08025B18(WA_S16(0x576));
    WR16(rec, 180, (u16)v);                  // s16[rec+0xB4]
    v = _08025B08(RDS16(rec, 180));
    WR16(rec, 172, (u16)v);                  // s16[rec+0xAC]
    v = _08025AFC();
    WR16(rec, 184, (u16)v);                  // s16[rec+0xB8]
    v = _080258B8(RDS16(rec, 172));
    WR16(rec, 188, (u16)v);                  // s16[rec+0xBC]
    v = _0800F744(RDS16(rec, 188));
    WR16(rec, 190, (u16)v);                  // s16[rec+0xBE]
    WR16(rec, 192, 0);                       // s16[rec+0xC0] = 0
    WR16(rec, 194, 0);                       // s16[rec+0xC2] = 0
    WR32(rec, 280, 3);                       // u32[rec+0x118] = 3
    WR32(rec, 292, 1);                       // u32[rec+0x124] = 1
    s16 s = (s16)(u16)(RD16(rec, 194) - 1);
    _0800F8B4(rec_, (u32)s);
    v = _080258B8(RDS16(rec, 172));
    WR32(rec, 340, tbl16(0, (s16)v));
    WR32(rec, 352, tbl16(10, RDS16(rec, 190)));
    WR32(rec, 364, tbl16(20, RDS16(rec, 192)));
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 280), (int)RD32(rec, 276));
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 292), (int)RD32(rec, 288));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 352), (int)RD32(rec, 348));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 364), (int)RD32(rec, 360));
    _0800F810(rec_);
}
#ifndef __APPLE__
void _0800104F0(void *a) __attribute__((alias("MenuFF78_104F0")));
void sub_0800104F0(void *a) __attribute__((alias("MenuFF78_104F0")));
#endif

// ============================================================================
// sub_080010644 — setup twin D: BlockB arm 0x138D, grid rebuild, mode 4/2.
// ============================================================================
void MenuFF78_10644(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    _08002124(0x138D);
    WR16(rec, 218, 1);                       // s16[rec+0xDA] = 1
    _080025930();
    WR16(rec, 168, 0);                       // s16[rec+0xA8] = 0
    int v = _08025A9C(WA_S16(0x576));
    WR16(rec, 180, (u16)v);                  // s16[rec+0xB4]
    v = _08025A8C(RDS16(rec, 180));
    WR16(rec, 172, (u16)v);                  // s16[rec+0xAC]
    v = MenuFF78_25A80();                    // 0x08025A80
    WR16(rec, 184, (u16)v);                  // s16[rec+0xB8]
    v = _080258B8(RDS16(rec, 172));
    WR16(rec, 188, (u16)v);                  // s16[rec+0xBC]
    v = _0800F744(RDS16(rec, 188));
    WR16(rec, 190, (u16)v);                  // s16[rec+0xBE]
    WR16(rec, 192, 0);                       // s16[rec+0xC0] = 0
    WR16(rec, 194, 0);                       // s16[rec+0xC2] = 0
    WR16(rec, 182, RD16(rec, 172));          // s16[rec+0xB6] = u16[rec+0xAC]
    WR32(rec, 280, 4);                       // u32[rec+0x118] = 4
    WR32(rec, 292, 2);                       // u32[rec+0x124] = 2
    s16 s = (s16)(u16)(RD16(rec, 194) - 1);
    _0800F8B4(rec_, (u32)s);
    v = _080258B8(RDS16(rec, 172));
    WR32(rec, 340, tbl16(0, (s16)v));
    WR32(rec, 352, tbl16(10, RDS16(rec, 190)));
    WR32(rec, 364, tbl16(20, RDS16(rec, 192)));
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 280), (int)RD32(rec, 276));
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 292), (int)RD32(rec, 288));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 352), (int)RD32(rec, 348));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 364), (int)RD32(rec, 360));
    WR16(rec, 202, 30);                      // s16[rec+0xCA] = 30
    WR16(rec, 178, RD16(rec, 172));          // s16[rec+0xB2] = u16[rec+0xAC]
}
#ifndef __APPLE__
void _080010644(void *a) __attribute__((alias("MenuFF78_10644")));
void sub_080010644(void *a) __attribute__((alias("MenuFF78_10644")));
#endif

// ============================================================================
// sub_0800107CC — setup twin E: 99 seeds into 0xBC/0xBE/0xC0 + WA mirrors.
// ============================================================================
void MenuFF78_107CC(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    _080025930();
    WR16(rec, 168, 0);                       // s16[rec+0xA8] = 0
    int v = _08025A9C(WA_S16(0x576));
    WR16(rec, 180, (u16)v);                  // s16[rec+0xB4]
    v = _08025A8C(RDS16(rec, 180));
    WR16(rec, 172, (u16)v);                  // s16[rec+0xAC]
    WR16(rec, 176, WA_U16(0xFEC));           // s16[rec+0xB0] = u16[WA+0xFEC]
    WR16(rec, 200, WA_U16(0xFE8));           // s16[rec+0xC8] = u16[WA+0xFE8]
    WR16(rec, 198, WA_U16(0xFEA));           // s16[rec+0xC6] = u16[WA+0xFEA]
    v = MenuFF78_25A80();                    // 0x08025A80
    WR16(rec, 184, (u16)v);                  // s16[rec+0xB8]
    WR16(rec, 188, 99);                      // s16[rec+0xBC] = 99
    WR16(rec, 190, 99);                      // s16[rec+0xBE] = 99
    WR16(rec, 192, 99);                      // s16[rec+0xC0] = 99
    v = _08025E1C(RDS16(rec, 180));
    WR16(rec, 194, (u16)v);                  // s16[rec+0xC2]
    WR32(rec, 280, 4);                       // u32[rec+0x118] = 4
    WR32(rec, 292, 2);                       // u32[rec+0x124] = 2
    s16 s = (s16)(u16)(RD16(rec, 194) - 1);
    _0800F8B4(rec_, (u32)s);
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 280), (int)RD32(rec, 276));
    _08007ABC((void *)(uintptr_t)RD32(rec, 44), (int)RD32(rec, 292), (int)RD32(rec, 288));
    v = _080258B8(RDS16(rec, 172));
    WR32(rec, 340, tbl16(0, (s16)v));
    _08007ABC((void *)(uintptr_t)RD32(rec, 36), (int)RD32(rec, 340), (int)RD32(rec, 336));
    _0800F810(rec_);
}
#ifndef __APPLE__
void _0800107CC(void *a) __attribute__((alias("MenuFF78_107CC")));
void sub_0800107CC(void *a) __attribute__((alias("MenuFF78_107CC")));
#endif

// ============================================================================
// sub_08001096C — input handler: mode 0/3 start/select toggles, clamp 4.
// ============================================================================
void MenuFF78_1096C(void *rec_, int a1, int a2) {
    (void)a1;
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 r4 = (u16)a2;
    s16 v = RDS16(rec, 168);
    if (v == 0) {
        if (r4 == 2) _0800108D4(rec_);
        if (r4 == 1) {
            WR16(rec, 168, 3);
            _0802B368(1);
        }
    } else if (v == 3) {
        if (r4 == 2) {
            WR16(rec, 168, 0);
            _0802B368(4);
        }
        if (r4 == 1) {
            _0800108F0(rec_);
            WR16(rec, 220, r4);
        }
    }
    if (RDS16(rec, 168) > 4) WR16(rec, 168, 4);
}
#ifndef __APPLE__
void _08001096C(void *a, int b, int c) __attribute__((alias("MenuFF78_1096C")));
void sub_08001096C(void *a, int b, int c) __attribute__((alias("MenuFF78_1096C")));
#endif

// ============================================================================
// sub_0800109D4 — gate-dispatched input handler: forward events via
// _08002158, drive the _08002178 set/clear machine, mode machine 0/3/4,
// counter tick at rec+0xCA, and the tail dispatch to _0800113A0/_080011214.
// ============================================================================
void MenuFF78_109D4(void *rec_, int a1, int a2) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 arg1 = (u16)a1;
    u16 r9   = (u16)a2;
    volatile u8 *sl;    // rec+218 (0xDA)
    volatile u8 *r5p;   // rec+204 (0xCC)

    if (WA_U8(0x10C3) == 1) {
        _08002158(2, RD16(rec, 178));        // u16[rec+0xB2]
        r5p = rec + 218;
        _08002158(3, RD16(rec, 218));        // u16[rec+0xDA]
        _08002158(5, RD16(rec, 206));        // u16[rec+0xCE]
        _08002158(4, RD16(rec, 170));        // u16[rec+0xAA]
        sl = r5p;                            // sl = rec+218
        r5p = r5p - 14;                      // rec+204 (0xCC)
    } else {
        _08002158(2, RD16(rec, 172));        // u16[rec+0xAC]
        r5p = rec + 218;
        _08002158(3, RD16(rec, 218));
        _08002158(5, RD16(rec, 204));        // u16[rec+0xCC]
        _08002158(4, RD16(rec, 168));        // u16[rec+0xA8]  (r8 = rec+168 at entry)
        sl = r5p;
        r5p = rec + 204;
    }

    // L_A7C: latch requested mode into s16[rec+0xCE]
    if (r9 == 1) WR16(rec, 206, 1);
    if (r9 == 2) WR16(rec, 206, 2);

    int en = (WA_U8(0x10C3) == 0) ? 1 : 0;
    PWR16(rec + 172, _08002178(en));         // (en,2) -> rec+0xAC
    WA_U16(0xFEE) = _08002178(en);           // (en,3) -> WA+0xFEE
    PWR16(r5p, _08002178(en));               // (en,5) -> rec+0xCC
    WR16(rec, 168, _08002178(en));           // (en,4) -> rec+0xA8

    if (RDS16(rec, 172) == 0)
        WR16(rec, 172, RD16(rec, 182));      // s16[rec+0xAC] = u16[rec+0xB6]

    // counter tick at s16[rec+0xCA]; r6 becomes the constant 0 here
    u16 c = (u16)(RD16(rec, 202) - 1);
    WR16(rec, 202, c);
    if ((s32)((u32)c << 16) <= 0) {
        WR16(rec, 202, 0);
        u32 a = _08002178(0);                // (0,5)
        u32 b = _08002178(1);                // (1,5)
        if (a == b) {
            s16 v = RDS16(rec, 168);
            if (v == 0) {
                if (RD16(rec, 204) == 2) {
                    WR16(rec, 206, 0);
                    _08002618(1, 0);
                    WR16(rec, 216, 0);
                    _0800108D4(rec_);
                }
                if (RD16(rec, 204) == 1) {
                    WR16(rec, 206, 0);
                    WR16(rec, 170, 3);
                    _0802B368(1);
                }
            } else if (v == 3) {
                if (RD16(rec, 204) == 2) {
                    WR16(rec, 206, 0);
                    WR16(rec, 170, 0);
                    _0802B368(4);
                }
                s16 r4v = RDS16(rec, 204);
                if (r4v == 1) {
                    WR16(rec, 206, 0);
                    _08002618(1, 1);
                    WR16(rec, 216, (u16)r4v);
                    WR16(rec, 170, 4);
                    _0802B368(1);
                }
            } else if (v == 4) {
                if (r9 == 32) PWR16(sl, 0);
                if (r9 == 16) PWR16(sl, 1);
                if (RD16(rec, 204) == 2) {
                    WR16(rec, 206, 0);
                    WR16(rec, 170, 3);
                    _0802B368(4);
                    if (WA_U8(0x10C3) == 1) {
                        _08002618(1, 0);
                        WR16(rec, 216, 0);
                    }
                }
                if (RD16(rec, 204) == 1) {
                    u32 x = _08002178(0);    // (0,2)
                    u32 y = _08002178(1);    // (1,2)
                    if (x == y) {
                        WR16(rec, 206, 0);
                        _08002618(1, 0);
                        WR16(rec, 216, 0);
                        _0800108F0(rec_);
                    }
                }
            }
        }
    }

    // tail: clamp s16[rec+0xAA] to 4; dispatch when mode == 0
    if (RDS16(rec, 170) > 4) WR16(rec, 170, 4);
    if (RDS16(rec, 168) == 0) {
        int g = WA_U8(0x10C3);
        if (g == 0)      _0800113A0(rec_, (int)arg1, (int)r9);
        else if (g == 1) _080011214(rec_, (int)arg1, (int)r9);
    }
}
#ifndef __APPLE__
void _0800109D4(void *a, int b, int c) __attribute__((alias("MenuFF78_109D4")));
void sub_0800109D4(void *a, int b, int c) __attribute__((alias("MenuFF78_109D4")));
#endif
