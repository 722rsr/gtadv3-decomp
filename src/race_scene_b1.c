// ============================================================================
// race_scene_b1.c — reconstructed C for asm/race_scene.s (18 functions,
// VMA order):
//
//   0x08001D224 0x08001D264 0x08001D280 0x08001D288 0x08001D654 0x08001D750
//   0x08001D7B8 0x08001D858 0x08001D910 0x08001D950 0x08001D974 0x08001D9CC
//   0x08001DA10 0x08001DA70 0x08001DB0C 0x08001DB4C 0x08001DBC0 0x08001DC20
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"
extern void _08003838(int a, u32 b, const volatile u8 *c);  /* slice-closure spelling */
// Pool-word anchor for the 0x080CBB9C s16 table read by _08001DC20 (pool word
// at 0x0801DC88, verified 0x080CBB9C in baserom.gba). A plain integer literal
// there is a CONST_INT, and agbcc's reload will NOT put one in a register the
// ROM uses: it materialises the constant into the register that the previous
// reload left free, which is r3, and spends r0 on the `final`-pass scratch that
// `*extendhisi2_insn` carries for the zero index. A CONST holding a SYMBOL_REF
// is the shape reload always gives a fresh pool word AND the register the ROM
// picked, so naming the table this way is load-bearing, not cosmetic. The
// symbol is defined with `.set` (an assembler absolute, emitting no bytes) and
// is file-local, so nothing is added to the link.
__asm__(".set TblDC20, 0x080CBB9C");
extern s16 TblDC20;

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
// Closure spelling for 0x08007770. A HOST_STUB is NOT a call-site binding: it
// expands to `extern` on the ROM build, so `Sub_08007770` there names a symbol
// the assembled closure does NOT define -- `arm-none-eabi-nm -n
// build-code/code.o` reports only `sub_08007770` (asm/course_resource_helpers.s)
// and `_08007770` at 0x08007770, both file-local -- and the slice link fails
// with an undefined reference the moment this body is staged. The digit count
// is load-bearing: 7 hex digits, as nm reports them.
extern void sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
#endif

// This macro MUST sit at file scope, outside the __APPLE__ split above. Defined
// inside a branch it is undefined on the other build, and tools/apple_decls.py
// reports "call to undeclared 'RSB_CALLEE' under __APPLE__" -- same idiom and
// same reason as D1_CALLEE in src/race_scene_d1.c.
#ifndef __APPLE__
#define RSB_CALLEE(friendly, closure) closure
#else
#define RSB_CALLEE(friendly, closure) friendly
#endif

// External callees (strong lifts or trampolines).
HOST_STUB(void _08001BB14(void));                                              // 0x08001BB14 race setup
HOST_STUB(void _08001B8DC(void));                                              // 0x08001B8DC scene leaf
HOST_STUB(void _08001C3E0(void));                                              // 0x08001C3E0 race frame
HOST_STUB(void _08001D218(void));                                              // 0x08001D218 record wrapper (race_scene.c)
HOST_STUB(u32  _08018ACC(u32 mask));                                           // 0x08018ACC flag test
HOST_STUB(void _08018AA8(u32 mask, int flag));                                 // 0x08018AA8 flag op
HOST_STUB(void _08018ADC(void));                                               // 0x08018ADC scene leaf
HOST_STUB(void _08009648(void));                                               // 0x08009648 scene leaf
HOST_STUB(int  _08009674(void));                                               // 0x08009674 record query
HOST_STUB(void _08009680(u32 v));                                              // 0x08009680 record select
HOST_STUB(void _08001B130(int v));                                             // 0x08001B130 flag wrapper (race_scene.c)
HOST_STUB(void _08019A84(void));                                               // 0x08019A84 award leaf
HOST_STUB(void _08005604(void));                                               // 0x08005604 scene helper
HOST_STUB(void _08002D3C(void));                                               // 0x08002D3C block copy + VRAM fill
HOST_STUB(void _08002B5AC(void));                                              // 0x08002B5AC sound stop
HOST_STUB(void _080024AC(void));                                               // 0x080024AC keypad flush
HOST_STUB(u16  _08002044(void));                                               // 0x08002044 lineup count
HOST_STUB(void _08002158(int id, u16 payload));                                // 0x08002158 block-B forward
HOST_STUB(u16  _08002178(int x));                                              // 0x08002178 block-B dispatch-if-less
HOST_STUB(void _08002618(int a, int b));                                       // 0x08002618 scene event
HOST_STUB(void _08002B368(u16 v));                                             // 0x08002B368 sound cue
HOST_STUB(void _08002E0A4(void *dst, const void *src, u32 n));                 // 0x08002E0A4 memcpy
HOST_STUB(void _0802E0A4(void *dst, const void *src, u32 n));                   // 0x0802E0A4 memcpy (closure spelling)
HOST_STUB(void _08007570(void *a, int b, int c, int d, int units)); // 0x08007570 5-arg: r0-r3 + 1 stack word (32-byte units)
HOST_STUB(void _08002ED0(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j)); // 0x08002ED0 10-arg place
HOST_STUB(void _08007614(void *a, int b, int c, int d));                       // 0x08007614 lane emit
HOST_STUB(void _08007770(int a, void *b, int c, int d, u32 e, u32 f));         // 0x08007770 template setup
HOST_STUB(void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f));        // 0x08007770 exact ROM body
HOST_STUB(void _08007ABC(void *a, void *b, int c));                            // 0x08007ABC resource bind
HOST_STUB(int  _08005758(int v));                                              // 0x08005758 IWRAM alloc
HOST_STUB(int  _08002DE04(int n, int d));                                      // 0x08002DE04 quotient
HOST_STUB(s32  _08002DE9C(s32 a, s32 b));                                      // 0x08002DE9C signed remainder
HOST_STUB(int  _08002A764(void *src, void *dst));                              // 0x08002A764 state copier (garage_records.c)
HOST_STUB(void _080056F4(void *a, int b, int c));                              // 0x080056F4 record helper
HOST_STUB(void _08003838(int a, u32 b, const volatile u8 *c));                 // 0x08003838 obj lane (ROM word 0x0805FC1C)
HOST_STUB(void _08003B78(int a, u32 b, u32 c));                                // 0x08003B78 obj digits
HOST_STUB(void _08003954(int a, int b, u32 c));                                // 0x08003954 text lane

// ---- shared anchors --------------------------------------------------------
#define WA        0x03001780u
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WA + (o)))
#define WA_U32(o) (*(volatile u32 *)(uintptr_t)(WA + (o)))
// Two-pool-word address anchors. The ROM does NOT fold `WA + off` into one
// literal: it emits `ldr rX,[pc]` for the base, `ldr rY,[pc]` for the offset and
// `adds`, sometimes reusing the base register across several sites. A plain
// `(uintptr_t)(WA + off)` is a single folded CONST_INT and collapses to one
// `ldr rX,[pc] / ldr rX,[rX]`. A CONST holding a SYMBOL_REF is the shape reload
// always gives a fresh pool word (same reason TblDC20 works), so naming both
// halves this way reproduces the split. `.set` emits no bytes, but it leaves a
// FILE-LOCAL absolute (`a` in nm), and a local symbol has no storage for the
// slice linker to bind: `_08001DB0C` failed the link with
//   undefined reference to `WABaseB1' / `WAOff10F8'.
// The `.set` gives the compiler the constant; the `.globl` + assignment makes
// the symbol resolvable at link. This is the same pairing TblDC20 already uses
// inside `_08001D5E4` further down this file. Absolute assignments emit no
// bytes, so the measured EXACT scores are unchanged.
// Declaration order is load-bearing: the pool emits base (0x03001780) before
// offset (0x000010F8), which is the ROM's order at 0x0801DB40/0x0801DB44.
__asm__(".set WAOff10F8, 0x000010F8");
extern u8 WAOff10F8;
__asm__(".set WAOffFBC, 0x00000FBC");
extern u8 WAOffFBC;
__asm__(".set WAOff10C3, 0x000010C3");
extern u8 WAOff10C3;
__asm__(".set WABaseB1, 0x03001780");
extern u8 WABaseB1;
// The `.globl` + assignment must be emitted INSIDE a function that uses these
// anchors, via WA_SPLIT_DECL below -- not at file scope. `match_c_slice.py`
// splices only the single promoted body's own section out of the generated TU
// `.s`, so a file-scope definition never reaches `code.o` and the link fails
// with "undefined reference to `WABaseB1'". `_0802D9FC`'s `B16FlagVMA` in
// src/runtime_state_dispatch.c links for exactly this reason. Absolute assignments emit no
// bytes, so the measured EXACT scores are unchanged.
#ifndef __APPLE__
#define WA_SPLIT_DECL \
    __asm__(".globl WABaseB1\nWABaseB1 = 0x03001780\n" \
            ".globl WAOff10F8\nWAOff10F8 = 0x000010F8\n" \
            ".globl WAOffFBC\nWAOffFBC = 0x00000FBC\n" \
            ".globl WAOff10C3\nWAOff10C3 = 0x000010C3\n")
#else
#define WA_SPLIT_DECL
#endif

#define WA_SPLIT(o) ((uintptr_t)o + (uintptr_t)&WABaseB1)

// ----------------------------------------------------------------------------
// ---- 0x08001D224 — race event leaf: setup + flag-gated frame/dispatch ----
// Pools: {0x03004E20} (inline at _08001D258). No args, no return.
// Flow: _08001BB14; if (racectx[129] == 0) return; if (flag 0x80000)
// _08001B8DC; flag 4 ? _08001D218 : _08001C3E0.
// ----------------------------------------------------------------------------
void _08001D224(void) {
    _08001BB14();
    if (*(volatile u8 *)(*(volatile u32 *)(uintptr_t)0x03004E20u + 129u) == 0)
        return;
    if (_08018ACC(0x80000u) != 0)
        _08001B8DC();
    if (_08018ACC(4u) == 0)
        _08001C3E0();
    else
        _08001D218();
}

// ----------------------------------------------------------------------------
// ---- 0x08001D264 — halfword bit set/clear helper ----
// Interior label of sub_08001D224 (parent.type spans 0x08001D224–0x08001D288;
// reference: the label to its `bx lr` as its own function). No pools.
// r0 = u16 mask (via lsls/lsrs #16), r2 = flag: nonzero sets, zero clears.
void _08001D264(void *ptr_, int mask, int flag) {
    volatile u16 *ptr = (volatile u16 *)ptr_;
    register u32 m __asm__("r0") = (u32)(u16)mask;
    register u16 out __asm__("r0");
    if (flag) {
        out = (u16)(m | (u32)*ptr);
    } else {
        register u16 v __asm__("r1") = *ptr;
        v &= (u16)~m;
        out = v;
    }
    *ptr = out;
}
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// ---- 0x08001D280 — halfword masked test helper ----
// Interior label of sub_08001D224 (parent.type spans 0x08001D224–0x08001D288;
// reference: the label to its `bx lr` as its own function). No pools.
// Returns u16[ptr] & mask (ldrh zero-extends; ands keeps the value).
// ----------------------------------------------------------------------------
u16 _08001D280(const void *ptr_, u16 mask) {
    return (u16)(*(volatile u16 *)ptr_ & mask);
}
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// ---- 0x08001D288 — race scene event (car-record scatter + flag machine) ----
// Pools: {0x080CBB28, 0x03004EA4, 0x03003A10, WA+0x10C3, 0x03004E20,
//   0x03004E80(via -36), 0x03005770, WA+0x10C3, 0x0300577C, 0x04000128,
//   0x03004E20, WA+0x10BE, 0x040000DE, 0x040000D4, 0x03004420,
//   0x04000020, 0xA2600008}. No args, no return.
// Shape: flag-0x800 gate; indirect state copy; flag-lane setup over
// 0x03003A1C; two-iteration key loop (idx 0..1) with the seven-scan
// scatter; post-loop race+106/108 management; DMA3/IRQ tail.
// ----------------------------------------------------------------------------
void _08001D288(void) {
    if (_08018ACC(0x800u) != 0) {
        u32 lane[13];
        volatile u16 *dst = (volatile u16 *)(uintptr_t)0x03003A10u;
        volatile u16 *flagbase;
        _08002A764((void *)(uintptr_t)0x03004EA4u, lane);
        {
            s32 w1 = (s32)lane[1];
            dst[0] = (u16)(w1 >> 16);
            dst[1] = (u16)w1;
        }
        {
            s32 w2 = (s32)lane[2];
            dst[2] = (u16)(w2 >> 16);
            dst[3] = (u16)w2;
        }
        {
            int t = _08009674();
            u32 v = (((u32)lane[6] >> 4) & 0xFFu) | ((u32)t << 8);
            dst[4] = (u16)v;
        }
        dst[5] = *(volatile u16 *)((volatile u8 *)(uintptr_t)lane[7] + 18u);
        dst[6] = 0x5200u; // 164 << 7
        flagbase = dst + 6; // 0x03003A1C
        _08001D264((void *)(uintptr_t)flagbase, 1u, (int)_08018ACC(0x200000u));
        _08001D264((void *)(uintptr_t)flagbase, 4u, (int)(lane[8] & 64u));
        _08001D264((void *)(uintptr_t)flagbase, 2u, (int)_08018ACC(0x400000u));
        _08001D264((void *)(uintptr_t)flagbase, 16u,
                   (*(volatile u16 *)(uintptr_t)0x03004E8Eu == 1) ? 1 : 0);
        {
            volatile u8 *race = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
            _08001D264((void *)(uintptr_t)flagbase, 8u,
                       (WA_U8(0x10C3u) == 0) ? (int)race[124] : (int)race[125]);
        }
        {
            volatile u16 *cell = (volatile u16 *)(uintptr_t)0x03003A10u;
            volatile u8 *racer = (volatile u8 *)(uintptr_t)0x03004E80u;
            volatile u8 *raceptr = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
            (void)raceptr;
            for (int idx = 0; idx <= 1; idx++) {
                u32 key = *(volatile u32 *)(uintptr_t)0x04000128u;
                if (((key >> 4) & 3u) == (u32)idx) {
                    _08002158(0, cell[0]);
                    _08002158(1, cell[1]);
                    _08002158(2, cell[2]);
                    _08002158(3, cell[3]);
                    _08002158(4, cell[4]);
                    _08002158(5, cell[5]);
                    _08002158(6, cell[6]);
                    continue;
                }
                if ((int)_08002044() > 1) {
                    volatile u8 *race = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
                    volatile u16 *scan = (volatile u16 *)(uintptr_t)0x03005770u;
                    u16 last;
                    *(volatile u16 *)(race + 106u) = 0;
                    scan[0] = _08002178(1);
                    scan[1] = _08002178(1);
                    scan[2] = _08002178(1);
                    scan[3] = _08002178(1);
                    scan[4] = _08002178(1);
                    scan[5] = _08002178(1);
                    last = _08002178(1);
                    scan[6] = last;
                    if ((last & 0xFFu) == 82u) {
                        *(volatile u32 *)(racer + 284u) =
                            ((u32)scan[0] << 16) | (u32)scan[1];
                        *(volatile u32 *)(racer + 288u) =
                            ((u32)scan[2] << 16) | (u32)scan[3];
                        *(volatile u32 *)(racer + 292u) =
                            (u32)*(volatile u8 *)((volatile u8 *)scan + 8u) << 4;
                        *(volatile u16 *)(racer + 306u) = scan[5];
                        {
                            u16 t = _08001D280((const void *)(uintptr_t)0x0300577Cu, 4u);
                            *(racer + 314u) = (u8)t;
                        }
                        race = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
                        if (WA_U8(0x10C3u) == 0) {
                            u16 t = _08001D280((const void *)(uintptr_t)0x0300577Cu, 8u);
                            race[124] = (t != 0u) ? 1 : 0;
                        } else {
                            u16 t = _08001D280((const void *)(uintptr_t)0x0300577Cu, 8u);
                            race[125] = (t != 0u) ? 1 : 0;
                        }
                        if (_08001D280((const void *)(uintptr_t)0x0300577Cu, 16u) != 0u)
                            race[130] = 1;
                    }
                    race = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
                    *(volatile u16 *)(race + 106u) += 1u;
                } else {
                    volatile u8 *race = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
                    *(volatile u16 *)(race + 106u) += 1u;
                }
                {
                    // ---- flag/sound tail of the scatter iteration ----
                    if (_08001D280((const void *)(uintptr_t)0x0300577Cu, 2u) != 0u) {
                        _08018AA8(0x800000u, 1);
                        continue;
                    }
                    if (_08001D280((const void *)(uintptr_t)0x0300577Cu, 1u) != 0u) {
                        if (_08018ACC(0x200000u) != 0u) {
                            if ((*(volatile u8 *)(uintptr_t)0x04000128u & 48u) == 0u)
                                continue;
                            _08018AA8(0x200000u, 0);
                            continue;
                        }
                        if (_08018ACC(0x100000u) == 0u) {
                            _08018AA8(0x100000u, 1);
                            continue;
                        }
                        {
                            int p = _08009674();
                            u32 q = (u32)*(volatile u16 *)(uintptr_t)0x03005778u >> 8;
                            _08009680(q);
                            if (p != (int)q)
                                _08002B368(2u);
                            continue;
                        }
                    }
                    if (_08018ACC(0x200000u) != 0u)
                        continue;
                    if (_08018ACC(0x100000u) == 0u)
                        continue;
                    if (_08018ACC(2u) != 0u)
                        continue;
                    _08001B130((int)((u32)*(volatile u16 *)(uintptr_t)0x03005778u >> 8));
                }
            }
        }
        {
            volatile u8 *race = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
            if (*(volatile s16 *)(race + 106u) > 60) {
                u16 c;
                *(volatile u16 *)(race + 106u) = 1;
                c = (u16)(*(volatile u16 *)(race + 108u) + 1u);
                *(volatile u16 *)(race + 108u) = c;
                if ((s16)c > 4) {
                    _08018AA8(2u, 1);
                    WA_U8(0x10BEu) = 1;
                }
            }
        }
    }
    *(volatile u16 *)(uintptr_t)0x040000DEu = 0;
    _08002D3C();
    if (_08018ACC(4u) == 0u) {
        _08018ADC();
        _08009648();
    }
    if (_08018ACC(64u) != 0u)
        _08019A84();
    _08005604();
    *(volatile u32 *)(uintptr_t)0x040000D4u = 0x03004420u;
    *(volatile u32 *)(uintptr_t)0x040000D8u = 0x04000020u;
    *(volatile u32 *)(uintptr_t)0x040000DCu = 0xA2600008u;
}

// ----------------------------------------------------------------------------
// ---- 0x08001D654 — IRQ/DMA teardown + display reset leaf ----
// Pools: {WA+0x10FC, WA+0x10C0 (134 << 5), 0x04000208, 0x04000200,
//   0xFFFD, 0x04000004, 0xFFEF, 0x040000D4, 0x040000DE}. No args, no return.
// ----------------------------------------------------------------------------
void _08001D654(void) {
    if (WA_U16(0x10FCu) == 10u)
        WA_U8(0x10C0u) = 1;
    _08002B5AC();
    *(volatile u16 *)(uintptr_t)0x04000208u = 0;
    *(volatile u16 *)(uintptr_t)0x04000200u =
        (u16)(*(volatile u16 *)(uintptr_t)0x04000200u & 0xFFFDu);
    *(volatile u16 *)(uintptr_t)0x04000004u =
        (u16)(*(volatile u16 *)(uintptr_t)0x04000004u & 0xFFEFu);
    *(volatile u16 *)(uintptr_t)0x04000208u = 1;
    *(volatile u16 *)(uintptr_t)0x040000DEu = 0;
    *(volatile u32 *)(uintptr_t)0x040000D4u = 0;
    *(volatile u32 *)(uintptr_t)0x040000D8u = 0;
    *(volatile u32 *)(uintptr_t)0x040000DCu = 0;
    *(volatile u16 *)(uintptr_t)0x04000000u = 0;
    _080024AC();
}

// ----------------------------------------------------------------------------
// ---- 0x08001D750 — mode-gated table select (7 halfwords) ----
// Pools: {0x0805FBDC} (14 bytes via _0802E0A4). Later range matches
// override earlier ones; default h[0]; mode > 49 takes h[6].
// Returns the sign-extended selection (lsls/asrs #16).
// ----------------------------------------------------------------------------
int _08001D750(int mode) {
    u16 h[7];
    u16 r;
    _0802E0A4(h, (const void *)(uintptr_t)0x0805FBDCu, 14u);
    r = h[0];
    if ((u32)(mode - 10) <= 4u)
        r = h[1];
    if ((u32)(mode - 15) <= 4u)
        r = h[2];
    if ((u32)(mode - 20) <= 4u)
        r = h[3];
    if ((u32)(mode - 25) <= 4u)
        r = h[4];
    if ((u32)(mode - 30) <= 19u)
        r = h[5];
    if (mode > 49)
        r = h[6];
    return (s16)r;
}

// ----------------------------------------------------------------------------
// ---- 0x08001D7B8 — centered-lane solve over a 12-word template ----
// Pools: {0x0805FBEC} (48 bytes via 4x ldmia/stmia triples). Each stage
// replaces r with center - t[i]/2 (C trunc division matches the
// bias-and-asrs halving); default r = 0. Stores (r, extra) to out.
//
// The template copy is a WHOLE-STRUCT assignment, not a loop: agbcc expands
// `T x = *(const T *)addr` into `output_block_move`, which with src=r0/dst=r1
// and r4/r5/r6/ip live picks the three free transfer registers r2,r3,r7 and
// emits four unrolled `ldmia r0!,{r2,r3,r7} / stmia r1!,{r2,r3,r7}` pairs --
// exactly the ROM. A `for (i<12) t[i]=src[i]` loop instead emits a 12-iteration
// `ldmia r1!,{r0} / stmia r3!,{r0}` loop. That change alone: 6/160 -> 13/160.
//
// The three `__asm__` register pins (out=r5, center=r4, extra=r6) are
// LOAD-BEARING and the removal control is on the record: deleting all three
// drops 38/160 -> 13/160, candidate 156 -> 152, and the transfer registers
// collapse from the ROM's `{r2,r3,r7}` back to `{r5,r6,r7}` (because with
// center/extra left in r2/r3 the block move no longer has to vacate them).
// `extra` must sit in r6 precisely because r7 is a transfer register.
typedef struct { s32 w[12]; } RSB7B8_Tmpl;

unsigned long long _08001D7B8(void *out_, int mode, int center, int extra) {
    register volatile u8 *out __asm__("r5") = (volatile u8 *)out_;
    register int m __asm__("r12") = mode;
    register int ctr __asm__("r4") = center;
    register int ext __asm__("r6") = extra;
    register int r __asm__("r2");
    register int e __asm__("r3");
    register unsigned long long ret_val __asm__("r0");
    {
        RSB7B8_Tmpl t = *(const RSB7B8_Tmpl *)(uintptr_t)0x0805FBECu;
        r = 0;
        e = 0;
        // The ROM initializes r3 here even though `ext` replaces it before
        // either output store. Keep that otherwise-dead value through an asm
        // input so agbcc retains the `movs r3,#0` in the same position.
        __asm__ volatile("" : "+r" (e));
        if ((u32)m <= 14u) {
            register int x0 __asm__("r0") = t.w[0];
            register int s0 __asm__("r1") = (int)((u32)x0 >> 31);
            r = ctr - ((x0 + s0) >> 1);
        }
        if ((u32)(m - 15) <= 4u) {
            register int x2 __asm__("r0") = t.w[2];
            register int s2 __asm__("r1") = (int)((u32)x2 >> 31);
            r = ctr - ((x2 + s2) >> 1);
        }
        if ((u32)(m - 20) <= 4u) {
            register int x4 __asm__("r0") = t.w[4];
            register int s4 __asm__("r1") = (int)((u32)x4 >> 31);
            r = ctr - ((x4 + s4) >> 1);
        }
        if ((u32)(m - 25) <= 4u) {
            register int x6 __asm__("r0") = t.w[6];
            register int s6 __asm__("r1") = (int)((u32)x6 >> 31);
            r = ctr - ((x6 + s6) >> 1);
        }
        if ((u32)(m - 30) <= 19u) {
            register int x8 __asm__("r0") = t.w[8];
            register int s8 __asm__("r1") = (int)((u32)x8 >> 31);
            r = ctr - ((x8 + s8) >> 1);
        }
        {
            // The final ROM comparison copies the selector from ip into r7.
            register int over __asm__("r7") = m;
            if (over > 49) {
                register int xa __asm__("r0") = t.w[10];
                register int sa __asm__("r1") = (int)((u32)xa >> 31);
                r = ctr - ((xa + sa) >> 1);
            }
        }
    }
    e = ext;
    *(volatile u32 *)(out + 0u) = (u32)r;
    *(volatile u32 *)(out + 4u) = (u32)e;
    __asm__("add r0, r5, #0" : "=r" (ret_val) : "r" (out));
    return ret_val;
}
// ----------------------------------------------------------------------------
// ---- 0x08001D858 — record-lane reset leaf ----
// Pools: {WA+0xFCE, WA+0x576, WA+0xFCA, WA+0xFC8, WA+0x10E5, WA+0x10E4}.
// Seeds rec+154/172/180/182/184/186/146 from work-area cells; both
// 8-bit cells feed s8-extended stores.
// ----------------------------------------------------------------------------
void _08001D858(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 w0 = WA_U16(0xFCEu);
    s16 w8;
    s8 e5;
    s8 e4;
    *(volatile u16 *)(rec + 154u) = WA_U16(0x576u);
    *(volatile u16 *)(rec + 172u + (u32)w0 * 2u) = 0;
    *(volatile u16 *)(rec + 182u) = 0;
    w8 = *(volatile s16 *)(uintptr_t)(WA + 0xFC8u);
    *(volatile u16 *)(rec + 180u) = (w8 == 1) ? 1u : 0u;
    *(volatile u16 *)(rec + 184u) = (w8 == 1) ? 1u : 0u;
    e5 = *(volatile s8 *)(uintptr_t)(WA + 0x10E5u);
    *(volatile s16 *)(rec + 146u) = e5;
    *(volatile u16 *)(rec + 186u) = ((u16)(e5 - 1) <= 2u) ? 1u : 0u;
    e4 = *(volatile s8 *)(uintptr_t)(WA + 0x10E4u);
    if (e4 == 0) {
        *(volatile u16 *)(rec + 182u) = 0;
        *(volatile u16 *)(rec + 184u) = 0;
        *(volatile u16 *)(rec + 186u) = 0;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x08001D910 — attr/param lane (obj digits vs obj lane) ----
// Pools: {0x0805FC1C}. a3 selects the digits path; a4 (caller stack word)
// gates it: nonzero a4 with nonzero a3 returns without emitting.
// ----------------------------------------------------------------------------
void _08001D910(int a0, int a1, u32 a2, int a3, int a4) {
    if (a3 != 0) {
        if (a4 != 0)
            return;
        if (a2 != 0) {
            _08003B78(a0, (u32)a1, a2);
            return;
        }
        _08003838(a0, (u32)a1, (const volatile u8 *)(uintptr_t)0x0805FC1Cu);
        return;
    }
    if (a2 != 0) {
        _08003B78(a0, (u32)a1, a2);
        return;
    }
    _08003838(a0, (u32)a1, (const volatile u8 *)(uintptr_t)0x0805FC1Cu);
}

// The spliced span for 0x08001D910 starts at the asm label `sub_08001D910`,
// which the splice deletes while other assembly still branches to it. agbcc
// only writes the `.thumb_set` for a spelling this TU aliases (rule 6), so the
// manifest entry's `export` list is only satisfiable with this declaration.
#ifndef __APPLE__
void sub_08001D910(int a0, int a1, u32 a2, int a3, int a4)
    __attribute__((alias("_08001D910")));
#endif

// ----------------------------------------------------------------------------
// ---- 0x08001D950 — text-lane twin (stack-gated) ----
// No pools. a3 == 0 always emits; nonzero a3 emits unless a4 is nonzero.
// r1/r2 pass through to the callee untouched.
// ----------------------------------------------------------------------------
void _08001D950(int a0, int a1, u32 a2, int a3, int a4) {
    if (a3 != 0) {
        if (a4 != 0)
            return;
        _08003954(a0, a1, a2);
        return;
    }
    _08003954(a0, a1, a2);
}
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// ---- 0x08001D974 — paired-record reset ----
// Pools: {WA+0xFBC, WA+0x10C3}. Clears u16[a+160]; when the FBC cell is 3
// and the 10C3 gate byte is clear, clears b+88 and runs _080056F4(b,1,1)
// to relatch a+160; always sets u16[b+84] = 1.
//
// EXACT 84/84: keep the incoming record pointers in r5/r4, the IWRAM base
// and offsets in r2/r1, and the computed addresses in r0. The empty asm ties
// prevent agbcc from folding WA+offset into one literal. Pinning the final
// b+84 address in r1 preserves the ROM's two-step pointer advance and pool
// position.
// ----------------------------------------------------------------------------
void _08001D974(void *a_, void *b_) {
    WA_SPLIT_DECL;
#ifndef __APPLE__
    register volatile u8 *pa __asm__("r5") = (volatile u8 *)a_;
    register volatile u8 *pb __asm__("r4") = (volatile u8 *)b_;
    register uintptr_t waBase __asm__("r2");
    register uintptr_t waOff __asm__("r1");
    register uintptr_t waAddr __asm__("r0");
    register u32 zero __asm__("r0");
    pa += 160u;
    zero = 0;
    *(volatile u16 *)pa = (u16)zero;
    waBase = (uintptr_t)&WABaseB1;
    waOff = (uintptr_t)&WAOffFBC;
    __asm__("" : "+r" (waBase), "+r" (waOff));
    waAddr = waBase + waOff;
    __asm__("" : "+r" (waAddr));
    if (*(volatile u16 *)waAddr == 3u) {
        register volatile u8 *p88 __asm__("r1") = pb + 88u;
        zero = 0;
        *p88 = (u8)zero;
        waOff = (uintptr_t)&WAOff10C3;
        __asm__("" : "+r" (waBase), "+r" (waOff));
        waAddr = waBase + waOff;
        __asm__("" : "+r" (waAddr));
        if (*(volatile u8 *)waAddr == 0u) {
            _080056F4((void *)(uintptr_t)pb, 1, 1);
            *(volatile u16 *)pa = 1;
        }
    }
    register volatile u8 *p84 __asm__("r1") = pb + 84u;
    *(volatile u16 *)p84 = 1;
#else
    volatile u8 *pa = (volatile u8 *)a_;
    volatile u8 *pb = (volatile u8 *)b_;
    *(volatile u16 *)(pa + 160u) = 0;
    if (*(volatile u16 *)WA_SPLIT((uintptr_t)&WAOffFBC) == 3u) {
        *(pb + 88u) = 0;
        if (*(volatile u8 *)WA_SPLIT((uintptr_t)&WAOff10C3) == 0u) {
            _080056F4((void *)(uintptr_t)pb, 1, 1);
            *(volatile u16 *)(pa + 160u) = 1;
        }
    }
    *(volatile u16 *)(pb + 84u) = 1;
#endif
}

// ----------------------------------------------------------------------------
// ---- 0x08001D9CC — dual alloc + template emit ----
// Pools: {0x0203F8F0 (+2 working cursor), 0x080CBBA2, 0x08336CA0}.
// Two iterations: slot[i*8] = (u16)_08005758(16), then the 8-arg emit
// over (tab[i], (s16)slot, 0, stack 16).
// ----------------------------------------------------------------------------
void _08001D9CC(void) {
#ifndef __APPLE__
    // ROM keeps the ascending two-iteration counter in r6.
    register int i __asm__("r6") = 0;
    // Keep the unadjusted literal in r0: ROM loads 0x0203F8F0 then adds 2.
    register volatile s16 *slotBase __asm__("r0") =
        (volatile s16 *)(uintptr_t)0x0203F8F0u;
    register volatile s16 *slot __asm__("r4");
#else
    int i = 0;
    volatile s16 *slot = (volatile s16 *)(uintptr_t)0x0203F8F0u + 1;
#endif
    volatile u16 *tab;
#ifndef __APPLE__
    __asm__ volatile("" : "+r"(slotBase));
    slot = slotBase + 1;
#endif
    tab = (volatile u16 *)(uintptr_t)0x080CBBA2u;
    do {
        *slot = (s16)_08005758(16);
        u16 entry = *tab;
        s16 id = *(s16 *)(void *)slot;
        _08007570((void *)(uintptr_t)0x08336CA0u, entry, id, 0, 16);
        slot += 4;
        tab += 1;
        i++;
    } while (i <= 1);
}

// ----------------------------------------------------------------------------
// ---- 0x08001DA10 — divmod-indexed dual template emit ----
// Pools: {0x080CBBA2, 0x0203F8F0, 0x08336CA0}. Splits v into
// (rem(quot(v,10),10), rem(v,10)), looks each up as a byte-doubled table
// index, and emits over the advancing 8-byte lane (s16 at +2).
// ----------------------------------------------------------------------------
void _08001DA10(int v) {
    int q = _08002DE04(v, 10);
    int w[2];
    u32 base = 0x0203F8F0u;
    w[0] = _08002DE9C(q, 10);
    w[1] = _08002DE9C(v, 10);
    for (int i = 0; i < 2; i++) {
        u16 t = *(volatile u16 *)(uintptr_t)(0x080CBBA2u + (u32)(w[i] * 2));
        _08007570((void *)(uintptr_t)0x08336CA0u, t,
                  *(volatile s16 *)(uintptr_t)(base + 2u), 0, 16);
        base += 8u;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x08001DA70 — gated 10-arg place (single vs double emit) ----
// Pools: {0x0203F8F0} twice (one per path, 0x0801DABC / 0x0801DB08). Signed
// a2 > 9 takes the double path: lane cursor +8, address cursor +24 per
// iteration, 2 iterations. Otherwise ONE emit at a0 - 16 over the lane word at
// base+10 — but the ROM still wraps it in the same countdown loop, initialised
// to 0, so it runs exactly once (`movs r5,#0` / `subs` / `cmp #0` / `bge`).
// Stack words are (1,2,1,0,0,1) in both paths.
void _08001DA70(int a0, int a1, u32 a2, int a3) {
    if ((s32)a2 > 9) {
        u32 lane = 0x0203F8F0u;
        int cur = a0 - 32;
        int i;
        for (i = 1; i >= 0; i--) {
            _08002ED0((void *)(uintptr_t)(u32)cur, a1,
                      *(volatile s16 *)(uintptr_t)(lane + 2u),
                      a3, 1u, 2u, 1u, 0u, 0u, 1u);
            lane += 8u;
            cur += 24;
        }
    } else {
        u32 base = 0x0203F8F0u;
        u32 lane = base + 8u;
        int cur = a0 - 16;
        int i;
        for (i = 0; i >= 0; i--) {
            _08002ED0((void *)(uintptr_t)(u32)cur, a1,
                      *(volatile s16 *)(uintptr_t)(lane + 2u),
                      a3, 1u, 2u, 1u, 0u, 0u, 1u);
            lane += 8u;
        }
    }
}

// ----------------------------------------------------------------------------
// ---- 0x08001DB0C — alloc/emit/divmod chain + twin lane setup ----
// Pools: {WA+0x10F8, 0x08336CA0}. No live entry args (r0 clobbered first).
// ----------------------------------------------------------------------------
void _08001DB0C(void) {
    WA_SPLIT_DECL;
    _08001D9CC();
    _08001DA10((int)*(volatile u32 *)WA_SPLIT((uintptr_t)&WAOff10F8));
    _08007614((void *)(uintptr_t)0x08336CA0u, 0, 0, 10);
    _08007614((void *)(uintptr_t)0x08336CA0u, 0, 0, 11);
}

// ----------------------------------------------------------------------------
// ---- 0x08001DB4C — gate leaf: scene event + template variant ----
// Pools: {WA+0x10C3, 0x082B7410}. Gate clear: event (1,1), rec+160 = 1,
// template (1, tbl, 2, 0, 0, 3); gate set: event (1,0), rec+160 = 0,
// template (1, tbl, 1, 0, 0, 3).
//
// The lever that closed it is the ORDER of two independent reloads plus giving
// each of them its own name. ROM builds the else arm as
//   adds r0,r5,#0 / adds r0,#160 / movs r2,#0 / strh r2,[r0] / str r2,[sp]
// -- address in r0, ONE zero in r2 feeding BOTH the strh and the 6th stack
// argument. Passing a bare literal `0` for that argument is actively wrong:
// agbcc emits a SECOND `movs r0,#0` and the body drops to 90/116 with the first
// difference moving into the if arm's branch at +0x34. So the two zeros must
// share one local (`u32 z`), which is what puts them in r2; and the address
// must be computed BEFORE `z` is assigned, which is what puts it in r0 instead
// of r1. Naming the address pointer `p` is load-bearing for that second half:
// with the store written inline the compiler emits
//   movs r2,#0 / adds r0,r5,#0 / adds r0,#160 / strh r2,[r0] / str r2,[sp]
// -- every mnemonic and operand right, 110/116, with only the two-instruction
//   ORDER swap costing the remaining 6 bytes.
void _08001DB4C(void *rec_) {
    WA_SPLIT_DECL;
    volatile u8 *rec = (volatile u8 *)rec_;
    register u32 gate __asm__("r4") = *(volatile u8 *)WA_SPLIT((uintptr_t)&WAOff10C3);
    if (gate == 0u) {
        _08002618(1, 1);
        *(volatile u16 *)(rec + 160u) = 1;
        RSB_CALLEE(Sub_08007770, sub_08007770)(1, (void *)(uintptr_t)0x082B7410u, 2, 0, gate, 3u);
    } else {
        u32 z;
        volatile u16 *p;
        _08002618(1, 0);
        p = (volatile u16 *)(rec + 160u);
        z = 0u;
        *p = z;
        RSB_CALLEE(Sub_08007770, sub_08007770)(1, (void *)(uintptr_t)0x082B7410u, 1, 0, z, 3u);
    }
}

// ----------------------------------------------------------------------------
// ---- 0x08001DBC0 — clamped lane select + countdown cascade ----
// Pools: {0x08336CA0, 0x083397D8}. Clamps v to [0,15]... precisely:
// v <= 0 -> 0, v > 14 -> 15; emits both templates; ticks u16[rec+164]
// down with a reload of 1, cascading into u16[rec+162]++ (zeroed past 15).
// ----------------------------------------------------------------------------
void _08001DBC0(void *rec_, int v) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u16 *lane;
    volatile u16 *prev;
    u32 t;
    if (v <= 0)
        v = 0;
    if (v > 14)
        v = 15;
    _08007614((void *)(uintptr_t)0x08336CA0u, 0, v, 11);
    _08007614((void *)(uintptr_t)0x083397D8u, 2, v, 5);
    lane = (volatile u16 *)(rec + 164u);
    t = (u32)*lane - 1u;
    *lane = (u16)t;
    if ((s32)(t << 16) <= 0) {
        u32 nv;
        *lane = 1;
        prev = (volatile u16 *)(rec + 162u);
        nv = (u32)*prev + 1u;
        *prev = (u16)nv;
        if ((s16)nv > 15)
            *prev = 0;
    }
}
#ifndef __APPLE__
// The closure spells this VMA `sub_08001DBC0` (asm/race_scene.s) and the entry
// exports it, so the owning TU must define that exact name. One hop to the real
// body. `asm/race_scene.s` is not in the closure that `asm/code.s` roots, so no
// assembly can supply it -- and the link failure is loud, which is the good
// outcome; the alternative is a ROM veneer the byte oracle cannot see.
void sub_08001DBC0(void *rec_, int v) __attribute__((alias("_08001DBC0")));
#endif

// ----------------------------------------------------------------------------
// ---- 0x08001DC20 — index clamp + lane bind ----
// Pools: {0x0805FC26 (3 halfwords via _08002E0A4), 0x082D7660,
//   0x080CBB9C}. Clamps the u16 input to {0,1,2} ((s16)<=0 -> 0,
//   (s16)>1 -> 2), emits the slot, stores the sign-extended table word
//   to rec+256, and binds ([rec+36], word, [rec+252]).
void _08001DC20(void *rec_, int sel_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 slots[3];
    int v = (u16)sel_;
    s16 w;
    s16 w2;
    register volatile u32 *dst __asm__("r2");
#ifndef __APPLE__
    __asm__(".globl TblDC20\nTblDC20 = 0x080CBB9C\n");
#endif
#ifndef __APPLE__
    _0802E0A4(slots, (const void *)(uintptr_t)0x0805FC26u, 6u);
#else
    _08002E0A4(slots, (const void *)(uintptr_t)0x0805FC26u, 6u);
#endif
    if ((s16)v <= 0)
        v = 0;
    if ((s16)v > 1)
        v = 2;
    {
        void *tpl = (void *)(uintptr_t)0x082D7660u;
        w = slots[(s16)v];
        _08007614(tpl, 2, w, 6);
    }
    {
        dst = (volatile u32 *)(rec + 256u);
        w2 = ((const s16 *)&TblDC20)[(s16)v];
        *dst = (u32)(s32)w2;
    }
    _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 36u),
              (void *)(intptr_t)(int)w2, (int)*(volatile u32 *)(rec + 252u));
}

// ROM entry alias.
#ifndef __APPLE__
void RaceScene_Event_1D224(void) __attribute__((alias("_08001D224")));
// The spliced span for 0x0801D224 starts at the asm label `sub_08001D224`,
// so the C must define that spelling for the splice's own glue to re-emit it.
void sub_08001D224(void) __attribute__((alias("_08001D224")));
void RaceScene_Event_288(void) __attribute__((alias("_08001D288")));
void RaceScene_Event_654(void) __attribute__((alias("_08001D654")));
// Same rule 6 need for 0x08001D750: the splice deletes the asm label
// `sub_08001D750` and promoted callers reference that spelling.
int sub_08001D750(int mode) __attribute__((alias("_08001D750")));
unsigned long long sub_08001D7B8(void *out, int mode, int center, int extra) __attribute__((alias("_08001D7B8")));
void sub_08001DC20(void *rec, int sel) __attribute__((alias("_08001DC20")));
#endif
