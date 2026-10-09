// ============================================================================
// race_scene_a2.c — reconstructed C for asm/race_scene.s (10 functions,
// VMA order):
//
//   0x0801AAA0 0x0801BB14 0x0801BFAC 0x0801BFBC 0x0801C0BC
//   0x0801C0CC 0x0801C0E0 0x0801C27C 0x0801C340 0x0801C3AC
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

// External callees (strong lifts or trampolines).
HOST_STUB(void  _08002D974(const void *src, void *dst, u32 ctrl)); // 0x08002D974 CpuSet
HOST_STUB(void  _080024AC(void));                                  // 0x080024AC screen-tile init
HOST_STUB(void  _080055D8(u32 a, u16 b));                          // 0x080055D8
HOST_STUB(void  _08002B00(u32 a, u32 b));                          // 0x08002B00
HOST_STUB(void  _08002B1C(u32 a, u32 b));                          // 0x08002B1C
HOST_STUB(void  _08002B30(u32 a, u32 b));                          // 0x08002B30
HOST_STUB(void  _08001A204(void));                                 // 0x08001A204 course load
HOST_STUB(void  _080023AC(int slot, void *handler));               // 0x080023AC table install
HOST_STUB(void  _08019A2C(int v));                                 // 0x08019A2C
HOST_STUB(void  _08004324(int v));                                 // 0x08004324 camera store
HOST_STUB(void  _08025264(int v));                                 // 0x08025264 lineup builder
HOST_STUB(void *_080240D0(void));                                  // 0x080240D0 record-ptr getter
HOST_STUB(void  _08002B190(void));                                 // 0x08002B190
HOST_STUB(void  _080056B8(int v));                                 // 0x080056B8 queue gate
HOST_STUB(u32   _08018ACC(u32 mask));                              // 0x08018ACC flag test
HOST_STUB(void  _08001A768(void *a, int b, int c));                // 0x08001A768
HOST_STUB(void  _08001A294(void));                                 // 0x08001A294 race init FSM
HOST_STUB(void  _08002B1B8(void));                                 // 0x08002B1B8
HOST_STUB(void  _08002124(u16 v));                                 // 0x08002124 BlockB arm
HOST_STUB(void  _08002254(void));                                  // 0x08002254
HOST_STUB(void  _08018AA8(u32 mask, int set));                     // 0x08018AA8 flag op
HOST_STUB(void *_08004B68(void));                                  // 0x08004B68 scene ctx
HOST_STUB(void  _08004B80(void *p));                               // 0x08004B80
HOST_STUB(void  _08004B90(void *p));                               // 0x08004B90
HOST_STUB(void  _08004BB8(void *p));                               // 0x08004BB8
HOST_STUB(void  _08002B418(int a, int b));                         // 0x08002B418
HOST_STUB(void  _08002B1E4(int v));                                // 0x08002B1E4
HOST_STUB(int   _08001B9AC(void));                                 // 0x08001B9AC helper
HOST_STUB(void  _08027114(int v));                                 // 0x08027114
HOST_STUB(void  _08027210(int v));                                 // 0x08027210
HOST_STUB(void  _08002B3A4(void));                                 // 0x08002B3A4
HOST_STUB(void  _08002B44C(void));                                 // 0x08002B44C
HOST_STUB(void  _08002B30C(int v));                                // 0x08002B30C
HOST_STUB(void  _08002B234(void));                                 // 0x08002B234
HOST_STUB(void  _08024158(void));                                  // 0x08024158 ghost commit
HOST_STUB(int   _08005B5C(int v));                                 // 0x08005B5C abs
HOST_STUB(int   _08005CB4(void *v));                               // 0x08005CB4
HOST_STUB(int   _08005F44(int v));                                 // 0x08005F44 s16 table
HOST_STUB(int   _08002DE04(int n, int d));                         // 0x08002DE04 sdiv
HOST_STUB(void  _08002B500(int a, int b, int c));                  // 0x08002B500
HOST_STUB(int   _08002D9AC(u32 v));                                // 0x08002D9AC sqrt
// 0x0802D9AC (Sqrt) is spelled `sub_0802D9AC` by the promotion closure, and
// bios_wrappers.c already aliases that spelling to the real Sqrt body (one
//.thumb_set hop, zero bytes). Declared here so the call sites can use it;
// those sites are #ifndef __APPLE__-split because the alias does not exist on
// the host, where the HOST_STUB spelling above stays in force.
#ifndef __APPLE__
int sub_0802D9AC(u32 v);
#endif
HOST_STUB(void *_08006C10(void *a, int b));                        // 0x08006C10
HOST_STUB(void  _0800199B0(void));                                 // 0x0800199B0
HOST_STUB(void  _0800199CC(void));                                 // 0x0800199CC

// ---- shared anchors --------------------------------------------------------
#define WA     0x03001780u
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WA + (o)))
#define WA_S16(o) (*(volatile s16 *)(uintptr_t)(WA + (o)))
#define WA_S8(o)  (*(volatile s8  *)(uintptr_t)(WA + (o)))
#define WA_U32(o) (*(volatile u32 *)(uintptr_t)(WA + (o)))
#define RC_SLOT   0x03004E20u
#define RC()      ((volatile u8 *)(uintptr_t)(*(volatile u32 *)(uintptr_t)RC_SLOT))

// ----------------------------------------------------------------------------
// ---- 0x0801AAA0 — race init (event 1): zero racectx, VRAM fills, WA lanes,
// ---- 10-way setup dispatch on (s16)(u16[WA+0x10FC]-1) via table 0x0801AC28.
// Pools: {0x03004E20 slot, 0x03005880 racectx, CpuSet ctrls 0x0500015A /
// 0x05000200 / 0x05000400, WA+0x10BE, DISPCNT=0, _080024AC, _080055D8,
// _08002B00/1C/30, _08001A204, WA+0x1104/0x10C2, WA+0x10FC/0x10BC/0x1106 +
// handlers 0x08019E81/0x08019D31/0x08019BF9 via _080023AC(3), IE|=2,
// DISPSTAT|=16, _08019A2C(0), [0x03005760+12]=0x03004420, _08004324,
// lane offsets 0x10CA/0x10C8/0x1114/0x1DD4/0x1119/0x1DE4/0x10BD/0x1100/0x10FE/
// 0x1124/0x1DD6/0x1DE6/0x1129, IME bounce at tail}. Incoming regs dead.
// ----------------------------------------------------------------------------
void _08001AAA0(void) {
    volatile u8 *wa = (volatile u8 *)WA;
    volatile u8 *rc = (volatile u8 *)(uintptr_t)0x03005880u;
    u32 stk0 = 0, stk1 = 0, stk2 = 0;
    int idx;
    *(volatile u32 *)(uintptr_t)RC_SLOT = 0x03005880u;
    _08002D974(&stk0, (void *)0x03005880u, 0x0500015Au);
    WA_U8(0x10BE) = 0;
    *(volatile u16 *)(uintptr_t)0x04000000u = 0;
    _080024AC();
    _080055D8((u32)(uintptr_t)(rc + 172), 32);
    _08002D974(&stk1, (void *)0x0600E000u, 0x05000200u);
    _08002D974(&stk2, (void *)0x0600F000u, 0x05000400u);
    _08002B00(0x030035D0u, 512u);
    _08002B1C(0x03003A20u, 128u);
    _08002B30(0x03003C20u, 128u);
    _08001A204();
    WA_U8(0x10C2) = (*(volatile u16 *)(wa + 0x1104) == 2) ? 1 : 0;
    if (*(volatile u16 *)(wa + 0x10FC) == 10) {
        WA_U8(0x10BC) = 1;
        _080023AC(3, (void *)0x08019E81u);
    } else if (*(volatile u16 *)(wa + 0x1106) == 2) {
        WA_U8(0x10BC) = 1;
        _080023AC(3, (void *)0x08019D31u);
    } else {
        WA_U8(0x10BC) = 0;
        _080023AC(3, (void *)0x08019BF9u);
    }
    *(volatile u16 *)(uintptr_t)0x04000200u |= 2u;
    *(volatile u16 *)(uintptr_t)0x04000004u |= 16u;
    _08019A2C(0);
    *(volatile u32 *)(uintptr_t)(0x03005760u + 12) = 0x03004420u;
    _08004324(0x03005760);
    idx = (int)(s16)(*(volatile u16 *)(wa + 0x10FC) - 1);
    if (idx < 0 || idx > 9) {
        WA_U16(0x10CA) = 1; // default arm _08001AF90 (shared with cases 6,7)
        WA_U16(0x10C8) = 3;
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119);
        WA_U8(0x10BD) = 0;
    } else switch (idx + 1) { // table 0x0801AC28
    case 1: // _08001AC50
        WA_U16(0x10CA) = 8;
        WA_U16(0x10C8) = 3;
        if (WA_S8(0x1112) != 0)
            _08025264(3);
        else
            _08025264((int)WA_S16(0x1100)); // 136<<5
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119);
        WA_U8(0x10BD) = 1;
        break;
    case 2: // _08001AE84
        WA_U16(0x10CA) = 1;
        WA_U16(0x10C8) = 1;
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119);
        WA_U8(0x10BD) = 0; // via _08001AFC2 (r3=0)
        break;
    case 3: { // _08001ACD4 (record-conditional 16 B copy)
        void *p;
        WA_U16(0x10CA) = 1;
        WA_U16(0x10C8) = 3;
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119);
        p = _080240D0();
        if (*(volatile u16 *)p != 0 &&
            *(volatile u16 *)((volatile u8 *)p + 2) == WA_U16(0x10FE)) {
            volatile u8 *d = wa + 0x1124;
            volatile u8 *s = (volatile u8 *)p + 16;
            *(volatile u32 *)(d + 0) = *(volatile u32 *)(s + 0);
            *(volatile u32 *)(d + 4) = *(volatile u32 *)(s + 4);
            *(volatile u32 *)(d + 8) = *(volatile u32 *)(s + 8);
            *(volatile u32 *)(d + 12) = *(volatile u32 *)(s + 12);
            WA_U16(0x1DD6) = *(volatile u16 *)((volatile u8 *)p + 16);
            WA_S16(0x1DE6) = *(volatile s8 *)((volatile u8 *)p + 21);
            WA_U16(0x10CA) = 2;
        }
        WA_U8(0x10BD) = 0;
        break;
    }
    case 4: // _08001AD78
        WA_U16(0x10CA) = 8;
        WA_U16(0x10C8) = 3;
        _08025264((int)WA_S16(0x1100));
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119);
        WA_U8(0x10BD) = 0; // via _08001AF0E (r5=0)
        break;
    case 5: // _08001AE48
        WA_U16(0x10CA) = 1;
        WA_U16(0x10C8) = 3;
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119); // via _08001AFB2
        WA_U8(0x10BD) = 0;              // via _08001AFC2 (r3=0)
        break;
    case 6: // _08001AF90 (falls into _08001AFB2 with r6=0x10C8+81)
    case 7:
    default:
        WA_U16(0x10CA) = 1;
        WA_U16(0x10C8) = 3;
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119);
        WA_U8(0x10BD) = 0;
        break;
    case 8: // _08001ADD8 (extended copy +0x1124/0x1129)
        WA_U16(0x10CA) = 2;
        WA_U16(0x10C8) = 3;
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119);
        WA_U16(0x1DD6) = WA_U16(0x1124);
        WA_S16(0x1DE6) = WA_S8(0x1129);
        WA_U8(0x10BD) = 0; // via _08001AFC2 (r3=0)
        break;
    case 9: // _08001AED4 (flag op 512,1)
        WA_U16(0x10CA) = 1;
        WA_U16(0x10C8) = 5;
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119);
        _08018AA8(512, 1); // 128<<2
        WA_U8(0x10BD) = 0; // via _08001AF0E (r5=0)
        break;
    case 10: // _08001AF30 (flag op 512,1)
        WA_U16(0x10CA) = 1;
        WA_U16(0x10C8) = 10;
        WA_U16(0x1DD4) = WA_U16(0x1114);
        WA_S16(0x1DE4) = WA_S8(0x1119);
        _08018AA8(512, 1); // 128<<2
        WA_U8(0x10BD) = 0;
        break;
    }
    if (WA_U16(0x10FC) != 8) { // _08001AFC4 tail
        *(volatile u16 *)(uintptr_t)0x04000208u = 0;    // IME = 0
        *(volatile u16 *)(uintptr_t)0x04000200u &= 0xFF7Fu; // IE &= ~0x80
        *(volatile u16 *)(uintptr_t)0x04000200u &= 0xFFBFu; // IE &= ~0x40
        *(volatile u16 *)(uintptr_t)0x04000208u = 1;    // IME = 1
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801BB14 — race setup dispatcher on s16[racectx+66] via table
// ---- 0x0801BB3C (10 entries, default _08001BF9C). r3 = RC slot address at
// ---- every arm (set once at head, no call precedes the switch).
// Pools: {0x03004E20, WA+0x10FC/0x10C3/0x10C8/0x1114/0x1118/0x111A/0x1110/
// 0x10BD, 0x04000050/0x04000052, 0x04000000}.
// ----------------------------------------------------------------------------
void _08001BB14(void) {
    volatile u8 *rc = RC();
    int s = (int)*(volatile s16 *)(rc + 66);
    if (s < 0 || s > 9) { // _08001BF9C default
        void *p = _08004B68();
        _08004BB8(p);
        return;
    }
    switch (s) { // table 0x0801BB3C
    case 0: { // _08001BB64 (sound/init/course chain)
        int a, b;
        _08002B190();
        _080056B8(1);
        a = (int)_08018ACC(512);   // 128<<2
        b = (int)_08018ACC(0x8000); // 128<<8
        _08001A768((void *)(uintptr_t)rc, a, b);
        _08001A294();
        _08002B1B8();
        if (WA_U16(0x10FC) == 8) {
            _08018AA8(0x800, 1); // 128<<4
            _08002124(0x138F);
        } else {
            _08002254();
        }
        rc = RC(); // _08001BF8A tail
        *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1);
        return;
    }
    case 1: { // _08001BBCA (flag-gated +68 select, then scene pair)
        if (*(volatile u16 *)((volatile u8 *)WA + 0x10FC) == 8 &&
            _08018ACC(2) != 0) {
            if (WA_U8(0x10C3) != 0) {
                rc[124] = 1;
                if (rc[125] == 0)
                    return; // _08001BFA4 (no phase++)
                *(volatile u16 *)(rc + 68) = 30; // _08001BC02
            } else {
                if (rc[124] == 0)
                    return; // _08001BFA4
                rc[125] = 1; // _08001BC2C
                *(volatile u16 *)(rc + 68) = 31; // via _08001BC48
            }
        } else {
            *(volatile u16 *)(rc + 68) = 30; // _08001BC40
        }
        { // _08001BC4A
            void *p = _08004B68();
            _08004B80(p);
        }
        rc = RC();
        rc[129] = 1;
        *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1); // _08001BF8E
        return;
    }
    case 2: { // _08001BC64 (countdown lane, then cue/select)
        u16 u = *(volatile u16 *)(rc + 68);
        if ((s16)u > 0) {
            *(volatile u16 *)(rc + 68) = (u16)(u - 1);
            return; // _08001BFA4
        }
        if (WA_U16(0x10FC) == 10) {
            _08002B1E4(60);
            _08018AA8(0x40000, 1); // 128<<9
        } else {
            int lane = 0;
            if (WA_S8(0x111C) > 0) // (s8)[WA+0x1118+4]
                lane = 1;
            if (WA_S8(0x111A) > 1) // (s8)[WA+0x1118+2]
                lane = 2;
            _08002B418((int)(s16)(u16)(WA_U16(0x1114) - 1u), lane);
            if (_08018ACC(32) != 0 && WA_S8(0x1110) != 0)
                _08002B1E4(58);
        }
        if (WA_U8(0x10BD) != 0) { // _08001BCE8
            volatile u8 *q = RC();
            *(volatile u16 *)(q + 100) = 0;
            *(volatile u32 *)(q + 20) = 0x600u; // 192<<3
            _08018AA8(0x80000, 1);  // 128<<12
            _08018AA8(0x800000, 1); // 128<<19
            rc = RC(); // _08001BF8A tail
            *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1);
            return;
        }
        _08019A2C(0); // _08001BD34
        rc = RC();
        *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1);
        return;
    }
    case 3: { // _08001BD3C (flag gates, BLDCNT/DISP select, +76/+68 lanes)
        if (_08018ACC(2) == 0 && _08018ACC(0x80000) != 0) // 128<<12
            return; // _08001BFA4
        if (_08018ACC(64) == 0)
            return; // _08001BFA4
        if (WA_U16(0x10FC) == 10) {
            *(volatile u16 *)(uintptr_t)0x04000000u = 0x1741u; // DISPCNT
            _08018AA8(0x4000000, 1); // 128<<23
        } else {
            *(volatile u16 *)(uintptr_t)0x04000050u = 0x740u; // BLDCNT (232<<3)
            *(volatile u16 *)(uintptr_t)0x04000052u = 0x40Cu; // BLDALPHA
        }
        { // _08001BDA2 on (s16)[WA+0x10FC]
            int k = (int)WA_S16(0x10FC);
            if (k == 10) {
                volatile u8 *q = RC();
                *(volatile u16 *)(q + 76) = 0;
                *(volatile u16 *)(q + 68) = 2;
            } else if (k == 9) {
                volatile u8 *q = RC();
                *(volatile u16 *)(q + 76) = 1;
                *(volatile u16 *)(q + 68) = 0;
            } else {
                *(volatile u16 *)(RC() + 76) = 5;
            }
            rc = RC(); // _08001BE04
        }
        *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1); // _08001BF8E
        return;
    }
    case 4: { // _08001BE0C (limited-lane countdown / helper-gated engine path)
        int is10 = (WA_U16(0x10FC) == 10);
        if (is10) {
            u16 u = *(volatile u16 *)(rc + 114);
            if ((s16)u <= 7) {
                u16 v = *(volatile u16 *)(rc + 68);
                if ((s16)v > 0) {
                    *(volatile u16 *)(rc + 68) = (u16)(v - 1); // _08001BEF0
                    return;
                }
                *(volatile u16 *)(rc + 114) = (u16)(u + 1); // _08001BF02
                *(volatile u16 *)(rc + 68) = 2;
                return;
            }
        } else {
            if (_08001B9AC() == 0 && _08018ACC(2) == 0) // _08001BE40
                return; // _08001BFA4
        }
        // _08001BE54:
        if (_08018ACC(32) == 0)
            _08018AA8(8, 1);
        if (_08018ACC(2) != 0)
            goto bump4;
        if (_08018ACC(32) != 0)
            goto bump4;
        _08027114(2); // _08001BE7E
bump4:;
        rc = RC(); // _08001BF8A tail
        *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1);
        return;
    }
    case 5: // _08001BE86 (sound stop/start + scene pair)
        if (_08018ACC(2) == 0)
            return; // _08001BFA4
        _08018AA8(8, 0);
        _08027210(0);
        _08002B3A4();
        _08002B44C();
        if (_08018ACC(32) != 0)
            _08002B30C(3);
        else
            _08002B234();
        { // _08001BEBE
            void *p = _08004B68();
            _08004B90(p);
        }
        rc = RC();
        *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1);
        return;
    case 6: { // _08001BEC8 (mirror countdown / DISPCNT reset path)
        if (WA_U16(0x10FC) == 10) {
            u16 u = *(volatile u16 *)(rc + 114);
            if ((s16)u > 0) {
                u16 v = *(volatile u16 *)(rc + 68);
                if ((s16)v > 0) {
                    *(volatile u16 *)(rc + 68) = (u16)(v - 1); // _08001BEF0
                    return;
                }
                *(volatile u16 *)(rc + 114) = (u16)(u - 1); // _08001BF00
                *(volatile u16 *)(rc + 68) = 2;             // _08001BF02
                return;
            }
            _08018AA8(0x4000000, 0); // _08001BF0A (128<<23)
            *(volatile u16 *)(uintptr_t)0x04000000u = 0x141u; // DISPCNT
        }
        _08019A2C(1); // _08001BF1E
        rc = RC();
        *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1);
        return;
    }
    case 7: // _08001BF2C (gate 64, clear +129)
        if (_08018ACC(64) != 0)
            return; // _08001BFA4
        rc[129] = 0;
        *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1); // _08001BF8E
        return;
    case 8: // _08001BF48 (gates 16/512/0x8000, ghost commit path)
        if (_08018ACC(16) == 0 && _08018ACC(512) == 0) { // 128<<2
            if (_08018ACC(0x8000) == 0) { // 128<<8, _08001BF70
                rc = RC(); // _08001BF8A tail
                *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1);
                return;
            }
            _08024158();
            rc = RC();
            *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1);
            return;
        }
        *(volatile u16 *)(RC() + 66) = 0; // _08001BF5E
        return;
    case 9: { // _08001BF82 (scene pair + phase++)
        void *p = _08004B68();
        _08004BB8(p);
        rc = RC();
        *(volatile u16 *)(rc + 66) = (u16)(*(volatile u16 *)(rc + 66) + 1);
        return;
    }
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801BFAC — flag-lane store leaf (interior label, heavily BL-called:
// ---- 14 sites in _08001BFBC plus external callers). Pools: {0x03004E20}.
// ----------------------------------------------------------------------------
void _08001BFAC(int v) {
    *(volatile u16 *)(RC() + 120) = (u16)v;
}

// ----------------------------------------------------------------------------
// ---- 0x0801BFBC — key-bit dispatch: one _08001BFAC call per set bit, then
// ---- two direct leaves. Pools: none. Mask order follows ROM (128<<12 is
// ---- tested before 128<<11).
// ----------------------------------------------------------------------------
void _08001BFBC(int flags) {
    if (flags & 32)
        _08001BFAC(1);
    if (flags & 64)
        _08001BFAC(3);
    if (flags & 128)
        _08001BFAC(6);
    if (flags & 256) // 128<<1
        _08001BFAC(2);
    if (flags & 512) // 128<<2
        _08001BFAC(4);
    if (flags & 1024) // 128<<3
        _08001BFAC(7);
    if (flags & 2048) // 128<<4
        _08001BFAC(0);
    if (flags & 4096) // 128<<5
        _08001BFAC(9);
    if (flags & 131072) // 128<<10
        _08001BFAC(10);
    if (flags & 524288) // 128<<12
        _08001BFAC(11);
    if (flags & 262144) // 128<<11
        _08001BFAC(12);
    if (flags & 1048576) // 128<<13
        _08001BFAC(13);
    if (flags & 8388608) // 128<<16
        _08001BFAC(14);
    if (flags & 16777216) // 128<<17
        _08001BFAC(15);
    if (flags & 2097152) // 128<<14
        _0800199B0();
    if (flags & 4194304) // 128<<15
        _0800199CC();
}

// ----------------------------------------------------------------------------
// ---- 0x0801C0BC — lane store leaf (interior label). Pools: {0x03004E20}.
// ----------------------------------------------------------------------------
void _08001C0BC(void) {
    *(volatile u16 *)(RC() + 84) = 6;
}

// ----------------------------------------------------------------------------
// ---- 0x0801C0CC — scene triple store leaf (interior label, 4 BL callers).
// ---- Pools: {0x03004E20}. Third word is (s16)-extended then stored 32-bit.
// ----------------------------------------------------------------------------
void _08001C0CC(int a, int b, int c) {
    volatile u8 *ctx = RC();
    *(volatile u32 *)(ctx + 0) = (u32)a;
    *(volatile u32 *)(ctx + 4) = (u32)b;
    *(volatile u32 *)(ctx + 8) = (u32)(int)(s16)c;
}

// ----------------------------------------------------------------------------
// ---- 0x0801C0E0 — nearest-record scan + heading/scale solve (no incoming
// ---- args). Pools: {WA+0x10CA, 0x03004E80 base, stride 284 (142<<1)}.
// ---- Loop i=1..n-1 (n re-read each pass); best = nearest record with
// ---- |dx|,|dy|<=64 by (u16)sqrt; tail solves the _08002B500(21,.,.) pair.
// ----------------------------------------------------------------------------
void _08001C0E0(void) {
    volatile u8 *wa = (volatile u8 *)WA;
    volatile u8 *base = (volatile u8 *)(uintptr_t)0x03004E80u;
    int xy[2];
    void *best = NULL;
    u32 bestd = 256;
    int i;
    for (i = 1; ; i++) {
        s16 n = *(volatile s16 *)(wa + 0x10CA);
        volatile u8 *rec;
        int dx, dy;
        u32 d;
        if (i >= n)
            break;
        rec = base + (u32)i * 284u;
        dx = (int)(*(volatile u32 *)(base + 0) - *(volatile u32 *)(rec + 0)) >> 8;
        if (_08005B5C(dx) > 64)
            continue;
        dy = (int)(*(volatile u32 *)(base + 4) - *(volatile u32 *)(rec + 4)) >> 8;
        if (_08005B5C(dy) > 64)
            continue;
        d = (u32)_08002D9AC((u32)dx * (u32)dx + (u32)dy * (u32)dy) & 0xFFFFu;
        if ((int)d >= (int)bestd)
            continue;
        xy[0] = dx;
        xy[1] = dy;
        bestd = d;
        best = (void *)(uintptr_t)rec;
    }
    if (!best)
        return; // _08001C260
    {
        volatile u8 *brec = (volatile u8 *)best;
        int a = (int)(u16)_08005CB4(xy);
        int b = (int)(u16)_08005CB4((void *)(uintptr_t)(base + 272)); // 136<<1
        int c = (int)(u16)_08005CB4((void *)(uintptr_t)(brec + 272));
        u32 x0 = *(volatile u32 *)(base + 272);
        u32 y0 = *(volatile u32 *)(base + 276); // 138<<1
        u32 d0 = (u32)_08002D9AC(x0 * x0 + y0 * y0);
        int t1 = _08005F44((int)(s16)((s16)a - (s16)b));
        u32 p = (((u32)(u16)d0 * (u32)t1) << 4) >> 16; // LSRS form
        u32 x1 = *(volatile u32 *)(brec + 272);
        u32 y1 = *(volatile u32 *)(brec + 276);
        u32 d1 = (u32)_08002D9AC(x1 * x1 + y1 * y1);
        int t2 = _08005F44((int)(s16)((s16)a - (s16)c));
        int q = (int)(((u32)(u16)d1 * (u32)t2) << 4) >> 17; // ASRS form
        int v = (q - (int)(s16)p) >> 1;
        int w4 = v + 2000; // 250<<3
        int k = _08002DE04((int)(bestd * 192u), 92);
        int s = 192 - k;
        u32 w = (u32)(u16)s;
        if ((s16)s < 0)
            w = 0;
        _08002B500(21, (int)(s16)w, (int)(s16)w4);
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801C27C — record-pair solve leaf (r0=a, r1=b, r2=u16 dst).
// ---- Pools: {0x03004E20}. Writes u16[dst] then u16[racectx+96].
// ----------------------------------------------------------------------------
void _08001C27C(void *a0, void *b0, void *c0) {
    volatile u8 *a = (volatile u8 *)a0;
    volatile u8 *b = (volatile u8 *)b0;
    // r1: the u16 destination pointer. The ROM materialises `mov r1,r9` at
    // 0x0801C30C, i.e. AFTER the `subs` that forms the delta, and keeps r1 live
    // across the negative-clamp branch (0x0801C318 reuses it for the second
    // `strh`). Left unpinned, agbcc keeps the pointer in r0 and must re-emit
    // `mov r1,r9` inside the clamp arm. The `c = c0` copy is placed after the
    // subtraction so the `mov` sinks to 0x0801C30C.
    register volatile u8 *c __asm__("r1");
    int xy[2];
    int dist;
    // r0: `t` holds the _08005F44 result, and the ROM multiplies the scaled
    // product back into that same register (`muls r0,r6` at 0x0801C300). The
    // product must therefore be written back through `t`, not into a fresh
    // `v`; a separate destination costs a `mov` and lands the product in r6.
    register int t __asm__("r0");
    // `p`/`q` are s16, not int. The ROM sign-extends BOTH operands before the
    // subtract (lsls/asrs on r4 and r5 at 0x0801C2E8-EE). With int locals
    // agbcc's range analysis knows the [0,65535] range and proves
    // (s16)((s16)p-(s16)q) == (s16)(p-q), deleting all four mask pairs; this
    // retype restores them.
    s16 p, q;
    register u32 d1 __asm__("r6");
    xy[0] = (int)(*(volatile u32 *)(b + 4) - *(volatile u32 *)(a + 0)) >> 8;
    xy[1] = (int)(*(volatile u32 *)(b + 8) - *(volatile u32 *)(a + 4)) >> 8;
#ifndef __APPLE__
    dist = (int)(u16)sub_0802D9AC((u32)xy[0] * (u32)xy[0] + (u32)xy[1] * (u32)xy[1]);
#else
    dist = (int)(u16)_08002D9AC((u32)xy[0] * (u32)xy[0] + (u32)xy[1] * (u32)xy[1]);
#endif
    p = (s16)(u16)_08005CB4(xy);
    q = (s16)(u16)_08005CB4((void *)(uintptr_t)(b + 12));
    // The y1 load is in a NESTED block so it stays textually after the x1
    // square: the ROM loads b+16 at 0x08001C2D8, i.e. after
    // `adds r1,r0,#0 / muls r1,r0 / adds r0,r1,#0` (0x08001C2D2-D6), while
    // declaring both loads in one block makes agbcc schedule them adjacently
    // and hoist the second one up. Same arithmetic either way — this is load
    // placement, not a semantic change.
    {
        u32 x1 = *(volatile u32 *)(b + 12);
        u32 sq = x1 * x1;
        {
            u32 y1 = *(volatile u32 *)(b + 16);
#ifndef __APPLE__
            d1 = (u32)sub_0802D9AC(sq + y1 * y1);
#else
            d1 = (u32)_08002D9AC(sq + y1 * y1);
#endif
        }
    }
    t = _08005F44((int)(s16)(p - q));
    // Written back through `t` (pinned r0) and cast back to int so the final
    // `>> 17` is an arithmetic shift: the ROM uses `asrs r0,r0,#17` at
    // 0x0801C304, not the `lsrs` an unsigned right shift would give.
    t = (int)(((u32)(u16)d1 * (u32)t) << 4) >> 17;
    {
        // r0 again: the ROM reuses the now-dead `t` register for the delta
        // (`subs r0,r3,r0` at 0x0801C30A) and then for the s16 sign test
        // (`lsls r0,r0,#16` at 0x0801C310).
        register int dlt __asm__("r0");
        {
            // r3: the ROM writes the loaded halfword OVER the dying copy of
            // `b` (`mov r3,r8` / `ldrh r3,[r3,#38]` at 0x0801C306-0x0801C308),
            // so the address temp and the loaded value share one register.
            // Two pins to the same register work only because the ranges are
            // DISJOINT -- `bp` dies exactly where `h` is born -- and only in
            // separate declaration blocks (two pins in one block make the C89
            // transform bail with "budget exhausted"). See the worked case at
            // src/ai_line_leaves.c `Ai_LineEqual` (`register u16 ha
            // __asm__("r3")`), which closes the same coalescing residual.
            // Control: without the pins this is `ldrh r1,[r3,#38] /
            // subs r0,r1,r0`, 194/196, first difference +0x8C.
            register volatile u8 *bp __asm__("r3");
            bp = b;
            {
                register u16 h __asm__("r3");
                h = *(volatile u16 *)(bp + 38);
                dlt = (int)h - t;
            }
        }
        c = (volatile u8 *)c0;
        *(volatile u16 *)c = (u16)dlt;
        if ((s16)dlt < 0)
            *(volatile u16 *)c = 0;
    }
    if (dist > 255)
        dist = 255;
    {
        // agbcc expands a store's VALUE first and its ADDRESS second, so the
        // ROM's order (base loads, then `asrs`, then `adds r1,#96`, then
        // `strh`) needs the base materialised by its own statement and the
        // value in a named local. Inline `(RC + 96)` gave
        // `adds r1,#96` immediately after `ldr r1,[r0]` and 186/196.
        volatile u8 *rcp = RC();
        int v = 240 - (dist >> 1);
        *(volatile u16 *)(rcp + 96) = (u16)v;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801C340 — gate leaf over _08006C10(rec,-1) (single ptr arg, always
// ---- returns 0). Pools: {WA+0x10C8, 0x17FF}.
// ----------------------------------------------------------------------------
int _08001C340(void *p0) {
    volatile u8 *p = (volatile u8 *)p0;
    void *q = _08006C10((void *)(uintptr_t)p, -1);
    u32 w;
    int t;
    if (_08018ACC(0x800000) == 0) // 128<<16
        return 0;
    if (*(volatile s8 *)(p + 16) <= 0)
        return 0;
    if ((*(volatile u32 *)q & 2u) == 0)
        return 0;
    w = *(volatile u32 *)(p + 12);
    t = (int)(w + 0x800u); // 128<<4
    if (t < 0)
        t = (int)(w + 0x17FFu);
    t >>= 12;
    if ((int)WA_S16(0x10C8) > t)
        return 0;
    _08018AA8(0x4000, 1); // 128<<15
    return 0;
}

int _08001C3AC(void *a0, void *b0) {
    static const u16 kSet = 0xFFFFu;
#ifndef __APPLE__
    register u8 *b __asm__("r2") = (u8 *)b0;
    register int r __asm__("r1") = 1;
    register u16 keep __asm__("r4");
#else
    u8 *b = (u8 *)b0;
    int r = 1;
    u16 keep;
#endif
    if (a0 == 0) {
        int v;
        int nx;
        int ng;
        int o;
        u16 c;
        r = 8;
        v = (int)*(s16 *)(b + r);
        nx = ~v;
        ng = -nx;
        o = ng | nx;
        r = (int)((u32)o >> 31);
        c = kSet;
        *(u16 *)(b + 8) = c;
    } else {
#ifndef __APPLE__
        register u16 f __asm__("r3") = *(u16 *)((u8 *)a0 + 8);
#else
        u16 f = *(u16 *)((u8 *)a0 + 8);
#endif
        keep = *(u16 *)(b + 8);
        if (f == keep)
            r = 0;
        *(u16 *)(b + 8) = f;
    }
    return r;
}

// ROM entry alias.
#ifndef __APPLE__
void RaceScene_Init_AAA0(void) __attribute__((alias("_08001AAA0")));
// The spliced span for 0x08001BFBC starts at the asm label `sub_08001BFBC`,
// which the splice deletes, and other promoted C bodies call it under that
// spelling. agbcc only writes the `.thumb_set` for a spelling this TU
// aliases, so declare it here (rule 6).
void sub_08001BFBC(int flags) __attribute__((alias("_08001BFBC")));
#endif
