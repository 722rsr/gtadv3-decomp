// ============================================================================
// race_scene_b2.c — C lift of asm/race_scene.s 0x0801C3E0 (race-frame fn).
//
//   0x0801C3E0 — the large race-frame fn (~1672 asm lines). Single BL caller:
//   sub_08001D224 (race-scene event fn, asm/race_scene.s:5614). Contains the
//   GHOST OVER latch: racectx flag 0x1000 is set at label _08001CE24 via
//   Ghost_FlagOp(0x1000, 1) when [scratch+32] bit 0x10000 is set while flag 32 is
//   clear (mask 0x10000 = movs #0x80 + lsls #9; scratch+32 = u32[0x03004F00]
//   via the _08002A764 copier); with flag 32 set the same lane raises
//   flag 2 + _08002A86C (asm/ghost.s S4: setter _08001CE24).
//   "GHOST OVER" text drawn from
//   ROM pool 0x0805FBD0 at the frame tail).
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

HOST_STUB(void  _08002E0A4(void *d, const void *s, u32 n));        // 0x08002E0A4 memcpy
HOST_STUB(void  _0802DDD0(void *a, void *b, void *c));            // 0x0802DDD0 bx-r2 veneer
HOST_STUB(void  _0802DDCC(void *a, void *b));                     // 0x0802DDCC bx-r1 veneer
HOST_STUB(void  _08018AA8(u32 mask, int set));                     // 0x08018AA8 flag set/clear
HOST_STUB(int   _08018ACC(u32 mask));                              // 0x08018ACC flag test
HOST_STUB(void  _08001B498(int v));                                // 0x08001B498 (race_scene_a1.c)
HOST_STUB(int   _08001B730(void *a, void *b));                     // 0x08001B730 (race_scene_a1.c)
HOST_STUB(void  _08001BFBC(u32 v));                                // 0x08001BFBC
HOST_STUB(void  _08001BFAC(int v));                                // 0x08001BFAC
HOST_STUB(void  _08001C0BC(void));                                 // 0x08001C0BC
HOST_STUB(void  _08001C0CC(u32 a, u32 b, u32 c));                  // 0x08001C0CC interior leaf (see NOTE)
HOST_STUB(void  _08001C0E0(void));                                 // 0x08001C0E0
HOST_STUB(void  _08001C27C(void *a, void *b, void *c));            // 0x08001C27C
HOST_STUB(void  _08001C340(void *p));                              // 0x08001C340
HOST_STUB(void  Sub_0800279D8(void *a, int b, int c, int d, int e)); // 0x0800279D8 exact ROM, 5 machine words
HOST_STUB(void  _08002A86C(void *rec, u32 bits, u32 flag));        // 0x08002A86C (garage_records.c)
HOST_STUB(int   _08005B5C(int v));                                 // 0x08005B5C abs
HOST_STUB(void  _08005BA8(void *a, int b));                        // 0x08005BA8 rotate
HOST_STUB(int   _08005CB4(void *a));                               // 0x08005CB4 angle
HOST_STUB(void *_08006C10(void *a, int d));                        // 0x08006C10 row lookup
HOST_STUB(void *_08006C60(void *a));                               // 0x08006C60 package getter
HOST_STUB(void  _08002B488(int a, int b, int c));                  // 0x08002B488 sound select
HOST_STUB(void  _08002B500(int a, int b, int c));                  // 0x08002B500 sound select
HOST_STUB(s32   _08002DE9C(s32 a, s32 b));                         // 0x08002DE9C signed divrem
HOST_STUB(void  _080038C8(int a, u32 b, const void *c));           // 0x080038C8 obj text lane
HOST_STUB(void  _08004260(int a, int b, int c));                   // 0x08004260 camera init
HOST_STUB(void  _080046D0(void));                                  // 0x080046D0
HOST_STUB(void  _08006650(void));                                  // 0x08006650 course stream
HOST_STUB(void  _08008CA0(void));                                  // 0x08008CA0 phase machine
HOST_STUB(void  _080080A4(int a));                                 // 0x080080A4 (course_records.c)
HOST_STUB(void  _0800808C(u16 a));                                 // 0x0800808C (course_records.c)
HOST_STUB(void  _08008074(u16 a));                                 // 0x08008074 (course_records.c)
HOST_STUB(void  _080080CC(u32 a));                                 // 0x080080CC (course_records.c)
HOST_STUB(void  _080080D8(u32 a));                                 // 0x080080D8 (course_records.c)
HOST_STUB(void  _080080E4(u32 a));                                 // 0x080080E4 (course_records.c)
HOST_STUB(void  _080080F4(u32 a));                                 // 0x080080F4 (course_records.c)
HOST_STUB(void  _08008104(void));                                  // 0x08008104 (course_records.c)
HOST_STUB(int   _08008114(u16 a));                                 // 0x08008114 (course_records.c)
HOST_STUB(void  _08008134(u16 a));                                 // 0x08008134 (course_records.c)
HOST_STUB(void  _080191BC(int v));                                 // 0x080191BC
HOST_STUB(void  _0801992C(void));                                  // 0x0801992C
HOST_STUB(void  _08026088(int a, int b, int c));                   // 0x08026088
HOST_STUB(void  _08026B80(int a));                                 // 0x08026B80 (sprite_obj_263e0.c)
HOST_STUB(void  _08026D48(int a));                                 // 0x08026D48 (sprite_obj_263e0.c, ignores arg)
HOST_STUB(void  _08027120(void));                                  // 0x08027120 unlabeled entry (see NOTE)
HOST_STUB(void  _08019010(void));                                  // 0x08019010 unlabeled entry (see NOTE)
HOST_STUB(void  _080278F0(void));                                  // 0x080278F0 unlabeled entry (see NOTE)
HOST_STUB(void  _08027828(void *p));                               // 0x08027828 unlabeled entry (see NOTE)
HOST_STUB(void  _0801986C(void));                                  // 0x0801986C (race_setup_18adc.s)
HOST_STUB(void  _08018C44(void));                                  // 0x08018C44 unlabeled entry (see NOTE)


// NOTE on _08001C0CC: interior BL entry at asm/race_scene.s:3522 (no
// `.type`; it sits between sub_08001BFBC's `bx lr`+pool and
// sub_08001C0E0). Body: racectx[0]=a, racectx[4]=b,
// racectx[8]=(s32)(s16)c; returns void. Called below at three sites with
// (x-sum, y-sum, masked-sum).

// ---- shared anchors (copied from race_scene_a1.c) --------------------------
#define WORK_AREA 0x03001780u
#define CTX       0x03004E20u
#define RACERS    0x03004E80u
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WORK_AREA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WORK_AREA + (o)))
#define WA_U32(o) (*(volatile u32 *)(uintptr_t)(WORK_AREA + (o)))

// ----------------------------------------------------------------------------
// ---- 0x0801C3E0 — race-frame fn ----
// Pools (in order): WA+0x10FC, WA, CTX, 0x54C, 0x4FC, 0x080CBB28,
// 0x03004EA4, 0x534, 0x080CBB2C, WA+0x10CA, 0x080CBB2C, 0x03004EA4,
// CTX, 0xF1B2F, WA, WA+0x10FC, WA+0x1108, CTX(0x03004E80 slot), 0x4FC,
// 0x080CBB28, CTX, 0x4E4, 0xFFF, CTX, 0x080CB94C, 0x0805CAF0, 0x0FFE,
// 0xFFFFFCE0, WA, WA+0x10FC, CTX, 0xD48, WA+0x10DC, CTX, 0xFFF, WA,
// WA+0x10FC, 0x080CBB28-table family, 0x0805FBC0, WA, WA+0x10FC, WA,
// WA+0x10FC, WA+0x10CA, 0x080CBB28, 0x03004FC0, WA, WA+0x10FC, WA+0x10C8,
// WA, WA+0x10FC, WA+0x110C, WA+0x10C3, jump table 0x0801CFFC, CTX,
// WA+0x1108, CTX, CTX, 0x03005760/0x03005DF0/0x03005E30, WA, WA+0x10FC,
// WA+0x10B8, CTX, WA, WA+0x10DC, 0x0805FBD0 ("GHOST OVER").
// ----------------------------------------------------------------------------
void _08001C3E0(void) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA;
    volatile u8 *racers = (volatile u8 *)RACERS;
    volatile u8 *r7rec;
    u32 frame[2];                       // [sp,#128] x/y (r8)
    u8 tmp04[52] ALIGN(4);              // [sp,#4] veneer-copy buf
    u16 h56;                            // [sp,#56] u16 buf (r9)
    u8 tmp60[16] ALIGN(4);              // [sp,#60] 14-B table buf
    u8 tmp76[52] ALIGN(4);              // [sp,#76] veneer-copy buf
    u32 racectx;
    int i;

    _08018AA8(0x100u, 0);
    _08018AA8(0x8000000u, 0);
    // ---- prologue scatter: WA+0x10FC == 8 → record copies + veneer lane --
    if (WA_U16(0x10FC) == 8u) {
        void *tbl;
        racectx = *(volatile u32 *)(uintptr_t)CTX;
        _08002E0A4((void *)(racectx + 0x4C8u),
                   (const void *)(racectx + 0x650u), 52u);
        for (i = 0; i < 7; i++)         // 28-B copy [racectx+0x54C]->[+0x4FC]
            ((volatile u32 *)(racectx + 0x4FCu))[i] =
                ((volatile u32 *)(racectx + 0x54Cu))[i];
        {
            u32 src = *(volatile u32 *)(racectx + 0x534u);
            for (i = 0; i < 7; i++)     // 28-B copy [src]->[racectx+0x54C]
                ((volatile u32 *)(racectx + 0x54Cu))[i] =
                    ((volatile u32 *)src)[i];
        }
        tbl = *(void *volatile *)(uintptr_t)0x080CBB2Cu;
        ((void (*)(void *))(uintptr_t)tbl)((void *)(uintptr_t)0x03004EA4u);
        tbl = *(void *volatile *)(uintptr_t)0x080CBB28u;
        ((void (*)(void *, void *))(uintptr_t)tbl)((void *)(uintptr_t)0x03004EA4u,
                                                    (void *)tmp04);
        _08001C340(*(void **)(void *)(tmp04 + 28));
    } else {
        // 0x03004EA4 + i*284 walk over s16[WA+0x10CA] (count reloaded per
        // iter, as ROM does via its r6 count pointer).
        for (i = 0; i < (s32)*(volatile s16 *)(wa + 0x10CAu); i++) {
            void *tbl = *(void *volatile *)(uintptr_t)0x080CBB2Cu;
            ((void (*)(void *))(uintptr_t)tbl)(
                (void *)(uintptr_t)(0x03004EA4u + (u32)i * 284u));
        }
    }
    // ---- flag-128 counter lane ----
    if (_08018ACC(128u) != 0) {
        u32 nv;
        racectx = *(volatile u32 *)(uintptr_t)CTX;
        nv = *(volatile u32 *)(racectx + 24u) + 5u;
        *(volatile u32 *)(racectx + 24u) = nv;
        // NOTE: `ble` = signed: fires only when (s32)nv > 0xF1B2F.
        if ((s32)nv > (s32)0xF1B2Fu)
            _08001B498(2);
        if (WA_U16(0x10FC) == 2u) {
            u32 lim = WA_U32(0x1108);
            u32 cur = *(volatile u32 *)(racectx + 24u);
            // NOTE: subs+bgt = signed diff test.
            if ((s32)(lim - cur) <= 0) {
                *(volatile u32 *)(racectx + 24u) = lim;
                _08001B498(2);
            }
        }
    }
    // ---- car-record lane select (r7) ----
    if (WA_U16(0x10FC) == 8u) {
        racectx = *(volatile u32 *)(uintptr_t)CTX;
        r7rec = (volatile u8 *)(racectx + 0x4C8u);
        *(volatile u32 *)(r7rec + 28u) = (u32)(racectx + 0x4FCu);
    } else {
        void *tbl = *(void *volatile *)(uintptr_t)0x080CBB28u;
        racectx = *(volatile u32 *)(uintptr_t)CTX;
        ((void (*)(void *, void *))(uintptr_t)tbl)((void *)(racers + 36u),
                                                    (void *)(racectx + 0x4C8u));
        r7rec = (volatile u8 *)(racectx + 0x4C8u);
        {
            u32 src = *(volatile u32 *)(racectx + 0x4E4u);
            for (i = 0; i < 7; i++)     // 28-B copy [src]->[racectx+0x4FC]
                ((volatile u32 *)(racectx + 0x4FCu))[i] =
                    ((volatile u32 *)src)[i];
        }
    }
    // ---- racers snapshot (_08001C598) ----
    *(volatile u32 *)(racers + 0u) = *(volatile u32 *)(r7rec + 4u);
    *(volatile u32 *)(racers + 4u) = *(volatile u32 *)(r7rec + 8u);
    *(volatile u32 *)(racers + 8u) = *(volatile u32 *)(r7rec + 24u);
    *(volatile u16 *)(racers + 22u) =
        *(volatile u16 *)(*(volatile u32 *)(r7rec + 28u) + 18u);
    *(volatile u8 *)(racers + 30u) =
        (u8)((*(volatile u32 *)(r7rec + 32u) >> 6) & 1u);
    *(volatile u32 *)(racers + 272u) = *(volatile u32 *)(r7rec + 12u);
    *(volatile u32 *)(racers + 276u) = *(volatile u32 *)(r7rec + 16u);
    // ---- flag-32 dispatch: set → row/collect lane, clear → 0xFF lane ----
    if (_08018ACC(32u) != 0) {
        void *row = _08006C60(*(void **)(void *)(r7rec + 28u));
        racectx = *(volatile u32 *)(uintptr_t)CTX;
        h56 = *(volatile u16 *)(r7rec + 38u);
        *(volatile u16 *)(racectx + 96u) = 255u;
        if (row != (void *)0 && _08018ACC(0x10000u) != 0) {
            _08018AA8(0x20000u, 1);
            _08001C27C(row, (void *)r7rec, (void *)&h56);
            if (WA_U16(0x10FC) == 10u) {
                *(volatile u32 *)(racectx + 20u) = 200u;
                *(volatile u32 *)(racers + 8u) =
                    (*(volatile u32 *)(racers + 8u) + 200u) & 0xFFFu;
            }
            {
                s32 dx = (s32)(*(volatile u32 *)(r7rec + 4u) -
                               *(volatile u32 *)(row + 0u));
                s32 dy = (s32)(*(volatile u32 *)(r7rec + 8u) -
                               *(volatile u32 *)(row + 4u));
                u32 px, py, m;
                int ang;
                frame[0] = (u32)(dx >> 4) + *(volatile u32 *)(racectx + 12u);
                frame[1] = (u32)(dy >> 4) + *(volatile u32 *)(racectx + 16u);
                px = *(volatile u32 *)(row + 0u) + frame[0];
                py = *(volatile u32 *)(row + 4u) + frame[1];
                // NOTE: r1 = frame[0] and r2 = racectx are also live at
                // this call; the 1-arg strong body reads the frame only.
                ang = _08005CB4((void *)frame);
                m = ((u32)ang + *(volatile u32 *)(racectx + 20u)) & 0xFFFu;
                _08001C0CC(px, py, m);
            }
        } else {
            // ---- _08001C690: flag-0x20000 countdown + curvature lane ----
            // (reached when row == 0 OR flag 0x10000 is clear).
            if (_08018ACC(0x20000u) != 0) {
                _08018AA8(0x20000u, 0);
                racectx = *(volatile u32 *)(uintptr_t)CTX;
                {
                    u16 t = (u16)(*(volatile u16 *)(racectx + 98u) + 1u);
                    *(volatile u16 *)(racectx + 98u) = t;
                    if ((s32)(s16)t > 3)
                        *(volatile u16 *)(racectx + 98u) = 1u;
                }
            }
            {
                s32 k;
                int mag, s44;
                racectx = *(volatile u32 *)(uintptr_t)CTX;
                k = (s32)*(volatile s16 *)(racectx + 98u);
                // ROM table 0x080CB94C indexed by s16[racectx+98]*2.
                *(volatile u32 *)(racectx + 20u) =
                    (u32)(s32)*(volatile s16 *)(uintptr_t)
                        (0x080CB94Cu + (u32)k * 2u);
                mag = _08005B5C((s32)*(volatile s16 *)(uintptr_t)
                    // ROM table 0x0805CAF0 on ([r7+24]-[r7+20])&0xFFE.
                    (0x0805CAF0u + ((*(volatile u32 *)(r7rec + 24u) -
                                     *(volatile u32 *)(r7rec + 20u)) & 0xFFEu)));
                s44 = (s32)*(volatile s16 *)(r7rec + 44u);
                // NOTE: ROM uses word stores (str of the sign-extended
                // s16) for this cell; all accesses here are u32.
                *(volatile u32 *)(racers + 236u) = (u32)s44;
                if ((s32)(s16)mag > 800) {
                    // NOTE: 0xFFFFFCE0 pool word = -800 (signed addend).
                    s32 adj = (((s32)(s16)mag - 800) >> 4);
                    if (s44 < 0) {
                        s32 nv = s44 + adj;
                        *(volatile u32 *)(racers + 236u) = (u32)nv;
                        if (nv > 0)
                            *(volatile u32 *)(racers + 236u) = 0u;
                    } else {
                        s32 nv = s44 - adj;
                        *(volatile u32 *)(racers + 236u) = (u32)nv;
                        if (nv < 0)
                            *(volatile u32 *)(racers + 236u) = 0u;
                    }
                }
            }
            // ---- WA+0x10FC == 10: race timer + countdown lanes ----
            if (WA_U16(0x10FC) == 10u) {
                racectx = *(volatile u32 *)(uintptr_t)CTX;
                *(volatile u32 *)(racectx + 12u) = 0xD48u;
                {
                    s32 c98 = (s32)*(volatile s16 *)(racectx + 98u);
                    if (c98 == 0) {
                        u32 base = WA_U32(0x10DC);
                        *(volatile u32 *)(racectx + 20u) =
                            (base << 4) - base;   // 15x, via lsls#4+subs
                    } else if (c98 == 2) {
                        *(volatile u32 *)(racers + 8u) =
                            (*(volatile u32 *)(racers + 8u) + 128u) & 0xFFFu;
                    }
                }
            }
            // ---- shared frame recompute (_08001C794; A1 skips this) ----
            {
                u32 m1, m2;
                racectx = *(volatile u32 *)(uintptr_t)CTX;
                frame[0] = *(volatile u32 *)(racectx + 12u);
                frame[1] = *(volatile u32 *)(racectx + 16u) + 11200u; // 175<<6
                m1 = (*(volatile u32 *)(r7rec + 20u) +
                      *(volatile u32 *)(racectx + 20u)) & 0xFFFu;
                _08005BA8((void *)frame, (int)m1);
                m2 = (*(volatile u32 *)(r7rec + 20u) +
                      *(volatile u32 *)(racectx + 20u)) & 0xFFFu;
                _08001C0CC(*(volatile u32 *)(r7rec + 4u) + frame[0],
                           *(volatile u32 *)(r7rec + 8u) + frame[1], m2);
            }
        }
        // ---- _08001C7D6 gate (A1 branches here; A2 falls through) ----
        if (WA_U16(0x10FC) == 10u)
            goto frame_tail;
        _08002B488((s32)(s16)h56,
                   (s32)*(volatile s16 *)(racectx + 96u),
                   (s32)*(volatile s16 *)(r7rec + 42u));
        goto event_section;
    } else {
        // ---- _08001C810: 0xFF lane ----
        int f1;
        racectx = *(volatile u32 *)(uintptr_t)CTX;
        *(volatile u16 *)(racectx + 96u) = 255u;
        f1 = _08018ACC(1u);
        if (f1 != 0) {
            u32 m;
            frame[0] = *(volatile u32 *)(racectx + 12u);
            frame[1] = *(volatile u32 *)(racectx + 16u) + 11200u; // 175<<6
            // NOTE: s16[racectx+78]<<3 stored as a word (lsls+str).
            *(volatile u32 *)(racectx + 20u) =
                (u32)(s32)*(volatile s16 *)(racectx + 78u) << 3;
            m = (*(volatile u32 *)(r7rec + 20u) +
                 *(volatile u32 *)(racectx + 20u)) & 0xFFFu;
            _08005BA8((void *)frame, (int)m);
            m = (*(volatile u32 *)(r7rec + 20u) +
                 *(volatile u32 *)(racectx + 20u)) & 0xFFFu;
            _08001C0CC(*(volatile u32 *)(r7rec + 4u) + frame[0],
                       *(volatile u32 *)(r7rec + 8u) + frame[1], m);
            goto event_section;
        }
        // ---- _08001C87C curvature lane (r9 = table u16) ----
        {
            u32 tabsel = (*(volatile u32 *)(r7rec + 24u) -
                          *(volatile u32 *)(r7rec + 20u)) & 0xFFEu;
            // ROM table 0x0805CAF0; kept in r9 (u16).
            u16 curv = *(volatile u16 *)(uintptr_t)(0x0805CAF0u + tabsel);
            s32 c5 = (s32)(s16)curv;
            racectx = *(volatile u32 *)(uintptr_t)CTX;
            // NOTE: word store of the sign-extended s16 (str), like path A.
            *(volatile u32 *)(racers + 236u) =
                (u32)(s32)*(volatile s16 *)(r7rec + 44u);
            if (_08005B5C(c5) > 800) {
                s32 cur = (s32)*(volatile u32 *)(racers + 236u);
                if (cur < 0) {
                    // NOTE: 0xFFFFFCE0 pool word = -800.
                    s32 nv = cur + ((_08005B5C(c5) + (s32)0xFFFFFCE0) >> 4);
                    *(volatile u32 *)(racers + 236u) = (u32)nv;
                    if (nv > 0)
                        goto curv_zero;
                } else {
                    // NOTE: 0xFFFFFCE0 pool word = -800.
                    s32 nv = cur - ((_08005B5C(c5) + (s32)0xFFFFFCE0) >> 4);
                    *(volatile u32 *)(racers + 236u) = (u32)nv;
                    if (nv < 0)
                        goto curv_zero;
                }
            }
            goto curv_done;
curv_zero:
            // NOTE: stores r6 = the flag-1 test result, which is 0 on this
            // path (branched on beq above); kept as the variable would be
            // if the compiler had not folded it. Written via the f1 local.
            *(volatile u32 *)(racers + 236u) = (u32)f1;
curv_done:;
        }
        // ---- WA+0x10FC == 5: lean/switch block (_08001C8F0) ----
        if (WA_U16(0x10FC) == 5u) {
            s8 lean;
            s32 c9;
            racectx = *(volatile u32 *)(uintptr_t)CTX;
            lean = *(volatile s8 *)(racectx + 127u);
            // NOTE: curv reloaded into r4 here as s16(r9).
            c9 = (s32)(s16)*(volatile u16 *)(uintptr_t)
                (0x0805CAF0u + ((*(volatile u32 *)(r7rec + 24u) -
                                 *(volatile u32 *)(r7rec + 20u)) & 0xFFEu));
            if ((s32)lean * c9 < 0)
                *(volatile u8 *)(racectx + 127u) = 0u;
            {
                int f10 = _08018ACC(0x1000000u);
                if (f10 != 0) {
                    if (c9 < 0) {
                        *(volatile u8 *)(racectx + 127u) = 255u;
                    } else if (c9 > 0) {
                        *(volatile u8 *)(racectx + 127u) = 1u;
                    } else {
                        *(volatile u8 *)(racectx + 127u) = 0u;
                    }
                } else {
                    if (_08005B5C(c9) > 0x6D6) {
                        u32 obj;
                        s32 e18, e22;
                        racectx = *(volatile u32 *)(uintptr_t)CTX;
                        obj = *(volatile u32 *)(racectx + 0x4E4u);
                        e18 = (s32)*(volatile s16 *)(obj + 18u);
                        e22 = (s32)*(volatile s16 *)(obj + 22u);
                        if (e18 >= e22 &&
                            (s32)*(volatile s16 *)(racectx + 94u) > 0 &&
                            (s32)*(volatile s16 *)(r7rec + 40u) > 79 &&
                            (s32)*(volatile s8 *)(racectx + 127u) * c9 <= 0) {
                            u32 nv56;
                            int dir;
                            nv56 = *(volatile u32 *)(racectx + 56u) +
                                   (u32)(s32)*(volatile s16 *)(r7rec + 40u);
                            *(volatile u32 *)(racectx + 56u) = nv56;
                            if (nv56 > 0x1388u) {
                                dir = (c9 >= 0) ? 1 : -1;
                                *(volatile u8 *)(racectx + 127u) =
                                    (u8)dir;
                                // NOTE: stores r6 = the flag-0x1000000
                                // result, 0 here (branched on beq); f10.
                                *(volatile u32 *)(racectx + 56u) = (u32)f10;
                                _0801992C();
                            }
                        } else {
                            racectx = *(volatile u32 *)(uintptr_t)CTX;
                            *(volatile u32 *)(racectx + 56u) = 0u;
                        }
                    } else {
                        racectx = *(volatile u32 *)(uintptr_t)CTX;
                        *(volatile u32 *)(racectx + 56u) = 0u;
                    }
                }
            }
            if ((s32)*(volatile s16 *)(r7rec + 40u) <= 79) {
                racectx = *(volatile u32 *)(uintptr_t)CTX;
                *(volatile u16 *)(racectx + 94u) =
                    (u16)(*(volatile u16 *)(racectx + 94u) - 2u);
            }
        }
        // ---- shared frame recompute (_08001C9F4) ----
        {
            u32 m;
            racectx = *(volatile u32 *)(uintptr_t)CTX;
            frame[0] = *(volatile u32 *)(racectx + 12u);
            frame[1] = *(volatile u32 *)(racectx + 16u) + 11200u; // 175<<6
            m = (*(volatile u32 *)(r7rec + 20u) +
                 *(volatile u32 *)(racectx + 20u)) & 0xFFFu;
            _08005BA8((void *)frame, (int)m);
            m = (*(volatile u32 *)(r7rec + 20u) +
                 *(volatile u32 *)(racectx + 20u)) & 0xFFFu;
            _08001C0CC(*(volatile u32 *)(r7rec + 4u) + frame[0],
                       *(volatile u32 *)(r7rec + 8u) + frame[1], m);
        }
        _08002B488((s32)*(volatile s16 *)(r7rec + 38u), 255,
                   (s32)*(volatile s16 *)(r7rec + 42u));
    }
event_section:
    // ---- _08001CA44: flag-1 lane + 6-way switch (table 0x0801CAAC) ----
    if (WA_U16(0x10FC) == 10u)
        goto frame_tail;
    if (_08018ACC(1u) == 0) {
        if (_08018ACC(2u) == 0) {
            u32 f40000;
            u8 sel49;
            racectx = *(volatile u32 *)(uintptr_t)CTX;
            // ROM table 0x080CBB58[u8[r7+49]] (word table).
            sel49 = *(volatile u8 *)(r7rec + 49u);
            f40000 = *(volatile u32 *)(uintptr_t)
                (0x080CBB58u + (u32)sel49 * 4u);
            _08018AA8(0x40000u, (int)f40000);
            {
                u32 c4 = *(volatile u32 *)(r7rec + 4u);
                u32 c8 = *(volatile u32 *)(r7rec + 8u);
                u32 c24m = *(volatile u32 *)(r7rec + 24u) & 0xFFFu;
                u32 flg32 = *(volatile u32 *)(r7rec + 32u);
                s32 t40 = (s32)*(volatile s16 *)(r7rec + 40u);
                // NOTE: site priority is flag-ordered, each gated on
                // s16[r7+40] > 10; every fallthrough lands at _08001CCD4.
                if ((flg32 & 0x1000u) != 0) {
                    // ---- _08001CB40 (site A) ----
                    if (t40 > 10) {
                        _08002B500(18,
                                  (s32)*(volatile s16 *)(racectx + 96u), 0);
                        _08018AA8(0x100u, 1);
                        Sub_0800279D8((void *)c4, (int)c8, (int)c24m, 1,
                                      3);
                    }
                } else if ((flg32 & 0x800u) != 0) {
                    // ---- _08001CB8C (site B) ----
                    if (t40 > 10) {
                        _08002B500(17,
                                  (s32)*(volatile s16 *)(racectx + 96u), 0);
                        _08018AA8(0x100u, 1);
                        Sub_0800279D8((void *)c4, (int)c8, (int)c24m, 1,
                                      2);
                    }
                } else if ((flg32 & 0x80000u) != 0) {
                    // ---- _08001CBDC (site C) ----
                    if (t40 > 10) {
                        _08002B500(18,
                                  (s32)*(volatile s16 *)(racectx + 96u), 0);
                        Sub_0800279D8((void *)c4, (int)c8, (int)c24m, 1,
                                      5);
                        _08018AA8(0x8000000u, 1);
                        {
                            u16 c112 =
                                *(volatile u16 *)(racectx + 112u);
                            if ((s32)(s16)c112 <= 29) {
                                *(volatile u16 *)(racectx + 112u) =
                                    (u16)(c112 + 1u);
                                _08018AA8(0x100u, 1);
                            }
                        }
                    }
                } else {
                    // ---- _08001CC34: WA+0x10BC byte lane ----
                    // NOTE: r4 == WA base on this path (proven: set at the
                    // _08001CA44 head, untouched by the A/B site blocks,
                    // which always exit to _08001CCD4).
                    // NOTE: all three stores below write 0: r1 holds the
                    // &0x80000 test result (0, beq taken) and r3 holds the
                    // byte itself (0, beq taken). The byte value is purely
                    // a branch condition; it is never stored.
                    u8 b10BC = WA_U8(0x10BC);
                    racectx = *(volatile u32 *)(uintptr_t)CTX;
                    if (b10BC != 0u) {
                        *(volatile u16 *)(racectx + 112u) = 0u;
                        if (t40 > 10)
                            Sub_0800279D8((void *)c4, (int)c8,
                                          (int)c24m, 1, 4);
                        if ((flg32 & 0x400u) != 0) {
                            _08002B500(16,
                                      (s32)*(volatile s16 *)(racectx + 96u),
                                      0);
                        }
                    } else if ((flg32 & 0x400u) != 0) {
                        *(volatile u16 *)(racectx + 112u) = 0u;
                        _08002B500(16,
                                  (s32)*(volatile s16 *)(racectx + 96u), 0);
                        Sub_0800279D8((void *)c4, (int)c8, (int)c24m, 1,
                                      1);
                    } else {
                        *(volatile u16 *)(racectx + 112u) = 0u;
                    }
                }
            }
        }
    } else {
        racectx = *(volatile u32 *)(uintptr_t)CTX;
        *(volatile u16 *)(racectx + 78u) =
            (u16)(*(volatile u16 *)(racectx + 78u) + 1u);
        if ((s32)(s16)*(volatile u16 *)(racectx + 78u) > 200)
            _08018AA8(2u, 1);
        if (_08018ACC(32u) == 0) {
            s32 sw = (s32)*(volatile s16 *)(racectx + 88u);
            // NOTE: `bhi` = unsigned: cases 0..5, else skip.
            if ((u32)sw <= 5u) {
                // Jump table 0x0801CAAC.
                switch (sw) {
                case 0: _080191BC(4); break;
                case 1: _080191BC(9); break;
                case 2: _080191BC(8); break;
                case 3: _080191BC(7); break;
                case 4: _080191BC(5); break;
                case 5: _080191BC(6); break;
                }
            }
        }
        if (_08018ACC(0x800000u) == 0)
            goto frame_tail;
        _08001B498(5);
        goto frame_tail;
    }
    // ---- _08001CCD4: flag-0x20000 lane ----
    if ((*(volatile u32 *)(r7rec + 32u) & 0x20000u) != 0) {
        _08018AA8(0x2000u, 1);
        _08018AA8(0x8000000u, 1);
    } else {
        _08018AA8(0x2000u, 0);
    }
    // ---- _08001CD06: flag-0x380 lane ----
    if ((*(volatile u32 *)(r7rec + 32u) & 0x380u) != 0) {
        racectx = *(volatile u32 *)(uintptr_t)CTX;
        _08002B500(19, (s32)*(volatile s16 *)(racectx + 96u), 0);
    }
    // ---- _08001CD24: flag-32 lane ----
    if ((*(volatile u32 *)(r7rec + 32u) & 32u) != 0) {
        _08018AA8(0x400000u, 1);
        _08001BFAC(8);
    }
    // ---- _08001CD3E: flag-0x8000 lane ----
    if ((*(volatile u32 *)(r7rec + 32u) & 0x8000u) != 0)
        _08018AA8(0x400000u, 1);
    // ---- _08001CD54: flag-0x40000 lane ----
    if ((*(volatile u32 *)(r7rec + 32u) & 0x40000u) != 0 &&
        _08018ACC(0x8000000u) == 0)
        _08001C0BC();
    // ---- _08001CD70: racectx+84 countdown + table-beep lane ----
    racectx = *(volatile u32 *)(uintptr_t)CTX;
    if ((s32)*(volatile s16 *)(racectx + 84u) > 0) {
        s32 slot;
        _08002E0A4((void *)tmp60, (const void *)(uintptr_t)0x0805FBC0u, 14u);
        // NOTE: count reloaded after the copy, as ROM does.
        slot = (s32)*(volatile s16 *)(racectx + 84u);
        if ((s32)*(volatile s16 *)(void *)(tmp60 + slot * 2) != 0) {
            _08018AA8(0x400u, 1);
            _08002B500(20, (s32)*(volatile s16 *)(racectx + 96u), 0);
        }
        *(volatile u16 *)(racectx + 84u) =
            (u16)(*(volatile u16 *)(racectx + 84u) - 1u);
    }
    // ---- _08001CDC2: WA+0x10FC == 3 bypass / C0E0 lane ----
    if (WA_U16(0x10FC) != 3u) {
        if (_08018ACC(0x200u) != 0 && _08018ACC(32u) == 0)
            _08001C0E0();
    }
    // ---- _08001CDE8: GHOST OVER latch lane ----
    // NOTE: mask is 0x10000 (movs r1,#0x80 + lsls #9 at 0x08001CDEA..EE,
    // ROM bytes 80 21 49 02) — not 0x200 (that mask belongs to the
    // _08001CDC2 bypass test two lanes up).
    if ((*(volatile u32 *)(r7rec + 32u) & 0x10000u) != 0) {
        if (_08018ACC(32u) == 0) {
            // _08001CE24: GHOST OVER latch — Ghost_FlagOp(0x1000, 1).
            _08018AA8(0x1000u, 1);
        } else {
            _08018AA8(2u, 1);
            _08002A86C((void *)(racers + 36u), 4u, 1u);
        }
    }
    // ---- _08001CE2E: WA+0x10FC == 3 + count-2 copy lane ----
    if (WA_U16(0x10FC) == 3u && *(volatile u16 *)(wa + 0x10CAu) == 2u) {
        void *tbl = *(void *volatile *)(uintptr_t)0x080CBB28u;
        ((void (*)(void *, void *))(uintptr_t)tbl)((void *)(uintptr_t)0x03004FC0u,
                                                    (void *)tmp76);
        if ((*(volatile u32 *)(tmp76 + 32u) & 0x10000u) != 0)
            _08002A86C((void *)(uintptr_t)0x03004FC0u, 4u, 1u);
    }
    // ---- _08001CE6A: row-again + draft block ----
    {
        void *row2 = _08006C10(*(void **)(void *)(r7rec + 28u), -1);
        u32 st28 = *(volatile u32 *)(r7rec + 28u);
        if ((s32)*(volatile s8 *)(void *)(st28 + 16u) > 0) {
            _08001BFBC(*(volatile u32 *)row2);
            if (_08001B730(*(void **)(void *)(r7rec + 28u), row2) != 0) {
                s16 idx = *(volatile s16 *)(racectx + 64u);
                u32 slot = racectx + 28u + (u32)(s32)idx * 4u;
                // NOTE: full-width subs+lsls (no 16-bit truncation).
                u32 prev = racectx + 28u +
                           (u32)((s32)idx - 1) * 4u;
                s32 q, delta;
                // NOTE: r8/r9 are repurposed here (racectx+64 / racectx);
                // the [sp,#128] frame is dead (last use was the 9F4 call).
                q = _08002DE9C((s32)*(volatile u32 *)(racectx + 24u), 3);
                *(volatile u32 *)slot =
                    *(volatile u32 *)(racectx + 24u) - (u32)q;
                delta = (s32)*(volatile u32 *)slot -
                        (s32)*(volatile u32 *)prev;
                if ((s32)idx < (s32)*(volatile s16 *)(wa + 0x10C8u)) {
                    // ---- _08001CF48: draft path ----
                    _080080E4((u32)delta);
                    _08002B500(9, 255, 0);
                    *(volatile u16 *)(racectx + 64u) =
                        (u16)(*(volatile u16 *)(racectx + 64u) + 1u);
                } else {
                    s32 wafc = (s32)*(volatile s16 *)(wa + 0x10FCu);
                    if (wafc == 2) {
                        _08001B498(1);
                    } else if (wafc == 8) {
                        _08001B498(4);
                    } else if (wafc == 3) {
                        if (*(volatile u32 *)(racectx + 24u) <
                            WA_U32(0x1108))
                            (void)_08008104();
                        _08001B498(0);
                    } else {
                        _08001B498(0);
                    }
                }
                // ---- _08001CF64: best-delta latch (bge = signed) ----
                if (WA_U16(0x10FC) == 3u) {
                    s32 lim = (s32)WA_U32(0x110Cu);
                    if (delta < lim) {
                        WA_U32(0x110C) = (u32)delta;
                        _080080F4((u32)delta);
                    }
                }
            }
        }
    }
    // ---- _08001CF82: flag-0x800000 + WA+0x10C3 lane ----
    if (_08018ACC(0x800000u) != 0 &&
        (WA_U8(0x10C3) == 0u || _08018ACC(0x400000u) == 0))
        _08001B498(5);
frame_tail:
    // ---- _08001CFAC ----
    (void)_080080A4((s32)*(volatile s16 *)(r7rec + 40u));
    _0800808C(*(volatile u8 *)(r7rec + 48u));
    _08008074(*(volatile u16 *)(r7rec + 38u));
    {
        // NOTE: (s16)(u16[WA+0x10FC]-1); `bls` = unsigned: slots 0..8,
        // all else → epilogue. Jump table 0x0801CFFC (slot i = val i+1).
        s32 sel = (s32)(s16)(u16)(WA_U16(0x10FC) - 1u);
        if ((u32)sel > 8u)
            goto epilogue;
        switch (sel) {
        case 0:   // val 1 → _08001D020
        case 3:   // val 4 → _08001D020
        case 7: { // val 8 → _08001D020
            (void)_08008114(*(volatile u16 *)(racers + 14u));
            racectx = *(volatile u32 *)(uintptr_t)CTX;
            _08008134(*(volatile u16 *)(racectx + 64u));
            _080080CC(*(volatile u32 *)(racectx + 24u));
            goto shared_d070;
        }
        case 2: { // val 3 → _08001D050
            racectx = *(volatile u32 *)(uintptr_t)CTX;
            _08008134(*(volatile u16 *)(racectx + 64u));
            _080080CC(*(volatile u32 *)(racectx + 24u));
            goto shared_d070;
        }
        case 1: { // val 2 → _08001D08C
            u32 rc = *(volatile u32 *)(uintptr_t)CTX;
            u32 diff = WA_U32(0x1108) - *(volatile u32 *)(rc + 24u);
            _080080D8(diff + 2u);
            goto epilogue;
        }
        case 4:   // val 5 → _08001D0B0
            racectx = *(volatile u32 *)(uintptr_t)CTX;
            _08008134(*(volatile u16 *)(racectx + 64u));
            goto epilogue;
        case 8:   // val 9 → _08001D0C4 (ble = signed)
            racectx = *(volatile u32 *)(uintptr_t)CTX;
            if ((s32)*(volatile u32 *)(racectx + 24u) > (s32)0x4650)
                _08018AA8(2u, 1);
            goto epilogue;
        case 5:   // val 6 → _08001D0D8
        case 6:   // val 7 → _08001D0D8
        default:
            goto epilogue;
        }
    }
shared_d070:
    // ---- _08001D070: indexed-gap store (r2 = racectx on both entries) ----
    {
        u32 c = (u32)(s32)*(volatile s16 *)(racectx + 64u);
        u32 cell = *(volatile u32 *)(racectx + 28u + ((c - 1u) * 4u));
        _080080D8(*(volatile u32 *)(racectx + 24u) - cell);
    }
epilogue:
    // ---- _08001D0D8 ----
    {
        u32 car = *(volatile u32 *)(uintptr_t)CTX;
        // NOTE: strh widths on the +8/+56 snapshot lanes (low halfword of
        // the u32 source); all other lanes are word stores.
        *(volatile u32 *)(uintptr_t)(0x03005760u + 0u) =
            *(volatile u32 *)(car + 0u);
        *(volatile u32 *)(uintptr_t)(0x03005760u + 4u) =
            *(volatile u32 *)(car + 4u);
        *(volatile u16 *)(uintptr_t)(0x03005760u + 8u) =
            (u16)*(volatile u32 *)(car + 8u);
        *(volatile u32 *)(uintptr_t)(0x03005DF0u + 24u) =
            *(volatile u32 *)(car + 0u);
        *(volatile u32 *)(uintptr_t)(0x03005DF0u + 28u) =
            *(volatile u32 *)(car + 4u);
        *(volatile u16 *)(uintptr_t)(0x03005DF0u + 56u) =
            (u16)*(volatile u32 *)(car + 8u);
        *(volatile u32 *)(uintptr_t)(0x03005E30u + 8u) =
            *(volatile u32 *)(car + 0u);
        *(volatile u32 *)(uintptr_t)(0x03005E30u + 12u) =
            *(volatile u32 *)(car + 4u);
        *(volatile u32 *)(uintptr_t)(0x03005E30u + 16u) =
            *(volatile u32 *)(car + 8u);
        _08004260(*(volatile u32 *)(car + 0u),
                  *(volatile u32 *)(car + 4u),
                  *(volatile u32 *)(car + 8u));
        _08006650();
        _080046D0();
        racectx = car;
        if (WA_U16(0x10FC) == 10u)
            goto frame_d1cc;
        if (_08018ACC(32u) != 0) {
            // ---- _08001D18E ----
            if (*(volatile u16 *)(wa + 0x10B8u) != 0u) {
                // NOTE: ldrh (u16) compare vs 9.
                if (*(volatile u16 *)(wa + 0x10FCu) != 9u)
                    _08026088(0, 0, 11);
                else
                    _08026088(0, 0, 10);
            }
        } else {
            s32 s90 = (s32)*(volatile s16 *)(racectx + 90u);
            if (s90 >= 0) {
                // ---- _08001D17C (falls into _08001D188) ----
                _08026088(0, 0, (int)s90);
                _08008CA0();
            } else {
                if ((s32)*(volatile s16 *)(wa + 0x10FCu) != 0) {
                    s32 s120 = (s32)*(volatile s16 *)(racectx + 120u);
                    // NOTE: r0 feeds _080026D48: -1 on the skip path, or
                    // _080026B80's (void) exit value on the call path.
                    // The strong body ignores its arg, so both are
                    // behaviorally exact; the taken constant is kept.
                    if (s120 == -1) {
                        _08026D48(-1);
                    } else {
                        _08026B80((int)s120);
                        _08026D48(0);
                    }
                }
                _08008CA0();
            }
        }
        // ---- _08001D1BA ----
        _08027120();
        _08019010();
        _080278F0();
        _08027828((void *)(uintptr_t)0x03005DF0u);
frame_d1cc:
        // ---- _08001D1CC ----
        _0801986C();
        _08018C44();
        WA_U32(0x10DC) = WA_U32(0x10DC) + 1u;
        if (_08018ACC(0x20u) != 0)
            // ROM pool 0x0805FBD0 = "GHOST OVER".
            _080038C8(32, 32u, (const void *)(uintptr_t)0x0805FBD0u);
    }
}
