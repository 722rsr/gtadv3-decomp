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
#include "gtadv/ram_blocks.h"


extern void _08002158(int a, int b);   // 0x08002158 block-B packet emit
extern u32  _08002178(int x, int hwIndex); // 0x08002178 block-B dispatch
extern void _08002618(int a, int b);   // 0x08002618
extern void _08007B18(void *a, int b, int c, int d, int e, int f, int g, int h); // 0x08007B18 (4 stack args)

#define WA        0x03001780u
#define GATE_10C3 0x10C3u

// ----------------------------------------------------------------------------
// 0x080023A34 sub_080023A34(rec, a, b) -- promoted from instruction
// splices to plain C (2026-10-16), byte-exact under old_agbcc
// (corpus_match_probe --c89 --require-exact), measured on the pinned
// toolchain. The spinning levers, all load-bearing:
//   * params are `int`, NOT `u16`: agbcc normalises u16 formals at function
//     ENTRY (before any statement), which would place the `lsls/lsrs r1`
//     pair above the ROM's `adds r5,r0,#0`. Writing `aN = (u16)a` as
//     ordered statements between fences restores the ROM order, and
//     `argB = (u16)b` into the r4 pin emits the ROM's rotate-truncate
//     `lsls r2,r2,#16 / lsrs r4,r2,#16` (the intermediate coalesces into
//     the input) and keeps the call's `adds r1,r4` a plain copy.
//   * homes r5 = rec, r4 = argB, r6 = rec+140 pins; the gate address stays
//     an UNPINNED allocno: flow.c early-returns on pinned hard regs, so a
//     pin would drop r7 out of `regs_ever_live` and lose the `push`; an
//     allocno reaches it (`adds r7,r0,r2`) and the push stays {r4-r7}.
//   * the gate address is spelled as `wa + gateOff` with wa/off as separate
//     dead-after-use locals: one expression `RAM_SYM + 0x10C3` folds in the
//     front end to a single pool word. The third gate read recomputes both
//     (`w2`/`o2` in r0/r1) because cse keeps the earlier values in callee-
//     saved regs across the calls.
//   * `result` needs a `"+r"` barrier after the call: without it agbcc
//     compares the call result in r0 directly and never emits the ROM's
//     `adds r4,r0,#0`; the barrier also frees r0 for the 142/144 address
//     copies. `z = 0` then lands in r4 after argB/result die.
//   * one `tmp` per site (pins r0/r0/r1/r0) rather than a single local:
//     the ROM builds each field address in a different register
//     (r0, r0, r1, r0).
// The two literal pools sit mid-body behind unconditional `b`s -- agbcc
// itself places them there; only their CONTENT needed the wa/off split.
// ------------------------------------------------------------------------------
void _080023A34(void *rec, int a, int b)
{
    register u8 *self __asm__("r5");
    register u32 argB __asm__("r4");
    u16 aN;
    u8 *rec140;
    volatile u8 *wa;
    u32 gateOff;
    volatile u8 *gate;
    register u8 *tmpA __asm__("r0");
    register u8 *tmpB __asm__("r0");
    register u8 *tmpC __asm__("r1");
    register u8 *tmpD __asm__("r0");
    int sel;
    register int result __asm__("r4");
    int z;
    u8 *w2;
    u32 o2;

    RAM_DEF_03001780;
    self = (u8 *)rec;
    __asm__ volatile ("" ::: "memory");
    aN = (u16)a;
    __asm__ volatile ("" ::: "memory");
    argB = (u16)b;
    __asm__ volatile ("" : "+r" (argB));
    rec140 = self;
    rec140 += 140;
    wa = RAM_SYM_03001780;
    gateOff = 0x10C3;
    gate = wa + gateOff;

    if (*gate == 1) {
        _08002158(0, aN);
        _08002158(1, argB);
        _08002158(2, *(u16 *)rec140);
        tmpA = self;
        tmpA += 232;
        _08002158(3, *tmpA);
    }

    if (*gate == 0) {
        _08002178(1, 0);
        sel = 1;
    } else {
        _08002178(0, 0);
        sel = 0;
    }
    result = _08002178(sel, 1);
    __asm__ volatile ("" : "+r" (result));

    if (result == 2) {
        tmpB = self;
        tmpB += 142;
        z = 0;
        *(u16 *)tmpB = z;
        w2 = RAM_SYM_03001780;
        o2 = 0x10C3;
        if (*(w2 + o2) == 0) {
            _08002618(1, 1);
            tmpC = self;
            tmpC += 144;
            *(u16 *)tmpC = 1;
        } else {
            _08002618(1, 0);
            tmpD = self;
            tmpD += 144;
            *(u16 *)tmpD = z;
        }
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
//     1, 3: stack={5,1,1,0}; _08007B18(rec, (int)w, a, b, 5, 1, 1, 0)
//     0, 2: stack={4,1,1,0}; _08007B18(rec, (int)w, a, b, 4, 1, 1, 0)
//     default: nothing
//   note: r0 arg for _08007B18 is rec (base, no +140 offset in this fn).
// ----------------------------------------------------------------------------
void _080023B60(void *rec, int a, int b)
{
    // Preserve the ROM's lookup setup: table base in r1, rec still in r0.
    register u32 table_base __asm__("r1") = 0x080CC190u;
    register u32 record_base __asm__("r0") = (u32)(uintptr_t)rec;
    __asm__ volatile ("" : "+r" (table_base), "+r" (record_base));
    u16 w = *(u16 *)(uintptr_t)(table_base + ((int)(s16) * (u16 *)(uintptr_t)(record_base + 140)) * 2);
    int sel = *(int *)((uintptr_t)rec + 168);

    switch (sel) {
    case 1:
    case 3:
        _08007B18(rec, (int)w, a, b, 5, 1, 1, 0);
        break;
    case 0:
    case 2:
        _08007B18(rec, (int)w, a, b, 4, 1, 1, 0);
        break;
    default:
        break;
    }
}
// The ROM span ends with 00 00; use zero fill instead of gas's Thumb nop.
__asm__(".align 2, 0");

// ROM entry alias.
void _08023A34(void *rec, int a, int b) __attribute__((alias("_080023A34")));

// ROM entry alias.
void _08023AE4(void *rec, int a, int b) __attribute__((alias("_080023AE4")));
void _08023B60(void *rec, int a, int b) __attribute__((alias("_080023B60")));
