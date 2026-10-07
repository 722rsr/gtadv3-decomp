// ============================================================================
// code_23a34.c — C lift of asm/code_23a34.s (VMA 0x08023A34–0x08023BD4, 3 funcs)
//
// Course-record engine sound/key-frame emitters (gate cells at
// 0x03001780+0x10C3). Every function is transcribed instruction-for-
// instruction from the cited asm listing.
//
// note: _080023B02's `.4byte 0xE01AD00E` mid-listing is the objdump2gas
// rendering of the conditional-branch tail of `cmp r6,#3` (bne/bge pair),
// not data. In C this is the fall-through structure of the if/else chain.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void _08002158(int a, int b) { (void)a; (void)b; }
__attribute__((weak)) u16  _08002178(int a) { (void)a; return 0; }
__attribute__((weak)) void _08002618(int a, int b) { (void)a; (void)b; }
__attribute__((weak)) void _08007B18(void *a, int b, int c, int d, int e, int f, int g, int h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
#endif

extern void _08002158(int a, int b);   // 0x08002158 block-B packet emit
extern u16  _08002178(int a);          // 0x08002178 block-B read (1-arg)
extern void _08002618(int a, int b);   // 0x08002618
extern void _08007B18(void *a, int b, int c, int d, int e, int f, int g, int h); // 0x08007B18 (4 stack args)

#define WA        0x03001780u
#define GATE_10C3 0x10C3u

// ----------------------------------------------------------------------------
// 0x080023A34 sub_080023A34(rec, a_u16, b_u16)
//   r5=rec, r1=(u16), r4=(u16)r2, r6=rec+140, r7 = WA+0x10C3 gate byte
//   if (gate == 1):
//     _08002158(0, 0); _08002158(1, r4); _08002158(2, u16[rec+140]);
//     _08002158(3, u8[rec+232])
//   if (gate == 0): r0 = _08002178(1) else r0 = _08002178(0)
//   r4 = _08002178(1)
//   if (r4 != 2) return
//   [rec+142] = 0
//   if (gate == 0): _08002618(1, 1); [rec+144] = 1
//   else:            _08002618(1, 0); [rec+144] = 0
// ----------------------------------------------------------------------------
void _080023A34(void *rec, u16 a, u16 b)
{
    u8 gate = *(u8 *)(uintptr_t)(WA + GATE_10C3);
    int r4;

    (void)a;
    if (gate == 1) {
        _08002158(0, 0);
        _08002158(1, (int)b);
        _08002158(2, *(u16 *)((uintptr_t)rec + 140));
        _08002158(3, *(u8 *)((uintptr_t)rec + 232));
    }
    if (gate == 0)
        r4 = _08002178(1);
    else
        r4 = _08002178(0);
    r4 = _08002178(1);
    if (r4 != 2)
        return;

    *(u16 *)((uintptr_t)rec + 142) = 0;
    if (gate == 0) {
        _08002618(1, 1);
        *(u16 *)((uintptr_t)rec + 144) = 1;
    } else {
        _08002618(1, 0);
        *(u16 *)((uintptr_t)rec + 144) = 0;
    }
}

// ----------------------------------------------------------------------------
// 0x080023AE4 sub_080023AE4(rec, a, b)
//   30-iteration course-record placement, byte-identical in the ROM to
//   0x080161F0 (src/rec35_16002.c Rec35_PlaceRecords) apart from the call's
//   displacement, and that promoted body is the recipe followed here. Every
//   one of its shape choices is load-bearing and was measured there:
//     * the two `1` stack args and the `0` are hoisted into callee-saved
//       registers by the ROM (r5 = one, sl = zero), so they must be NAMED
//       locals, not literals;
//     * the dispatch is a `switch`, not an if/else chain: switch lowering
//       emits `cmp/beq; cmp/beq; b end; body; b end; body; end` (arms out of
//       line), which is the ROM's `d002 / d00e / e01a` layout;
//     * the declaration order (R, a1, md, one, zero, off) is what maps
//       rec->r8, a1->r7, md->r6, one->r5, off->r4 as the ROM does.
//   The old form here was an if/else with a ternary that also emitted a call
//   for every b, not only 2/3 -- wrong behavior as well as wrong bytes.
// ----------------------------------------------------------------------------
void _080023AE4(void *rec, int a, int b)
{
    u8 *R = (u8 *)rec;
    int a1 = a;
    int md = b;
    int one = 1;
    int zero = 0;
    int off = 0;

    for (int i = 29; i >= 0; i--) {
        switch (md) {
        case 2:
            _08007B18(R + 52, 7, off, a1, 4, one, one, zero);
            break;
        case 3:
            _08007B18(R + 52, 7, off, a1, 5, one, one, zero);
            break;
        default:
            break;
        }
        off += 8;
    }
}

// ----------------------------------------------------------------------------
// 0x080023B60 sub_080023B60(rec, a, b)
//   r3=rec, r4=a, r5=b
//   r0 = (s16)[rec+140] * 2 + 0x080CC190 → u16 w
//   r0 = [rec+168]
//   switch (r0):
//     1, 2, 3: stack={5,1,1,0}; _08007B18(rec, (int)w, a, b, 5, 1, 1, 0)
//     0:       stack={4,1,1,0}; _08007B18(rec, (int)w, a, b, 4, 1, 1, 0)
//     default: nothing
//   note: r0 arg for _08007B18 is rec (base, no +140 offset in this fn).
// ----------------------------------------------------------------------------
void _080023B60(void *rec, int a, int b)
{
    int sel = *(int *)((uintptr_t)rec + 168);
    int kind;

    if (sel == 1 || sel == 2 || sel == 3)
        kind = 5;
    else if (sel == 0)
        kind = 4;
    else
        return;

    {
        u16 w = *(u16 *)(uintptr_t)(0x080CC190u + ((int)(s16) * (u16 *)((uintptr_t)rec + 140)) * 2);
        _08007B18(rec, (int)w, a, b, kind, 1, 1, 0);
    }
}

// ROM entry alias.
#ifndef __APPLE__
void _08023A34(void *rec, u16 a, u16 b) __attribute__((alias("_080023A34")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void _08023AE4(void *rec, int a, int b) __attribute__((alias("_080023AE4")));
void _08023B60(void *rec, int a, int b) __attribute__((alias("_080023B60")));
#endif
