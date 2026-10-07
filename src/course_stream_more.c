// ============================================================================
// course_stream_more.c — C lift of asm/course_stream_more.s remaining
// functions: 0x08006BC8 / 0x08006C10 / 0x08006C60 / 0x08006C94.
//
// Course-surface row state constructors + wrap-around column lookup +
// cross-product heading leaf. Transcribed instruction-for-instruction
// from the cited asm listing.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void *_08005F98(void) { return (void *)0; }
__attribute__((weak)) int   _0802DE9C(int a, int b) { (void)a; return 0; }
#else
extern void *_08005F98(void);   // 0x08005F98 surface package getter (surface_access.s)
extern int   _0802DE9C(int a, int b); // 0x0802DE9C signed divide core
#endif

// ----------------------------------------------------------------------------
// 0x08006BC8 sub_08006BC8(state, hdr) — surface row-state constructor:
//   r4=state, r5=hdr
//   pkg = _08005F98; r1 = pkg
//   r2 = u16[hdr+12]
//   if (s16)[hdr+12] >= 0: u16[state+10] = r2
//   else:                  u16[state+10] = 0
//   [state+0]  = [pkg+20]
//   [state+4]  = [pkg+24]
//   u16[state+8] = u16[pkg+48]
//   [state+12] = (s16)[hdr+16]
//   state[16]  = 0
//   u16[state+18] = u16[hdr+14]
//   [state+24] = 0
//   state[17]  = 0
//   u16[state+20] = 0
// ----------------------------------------------------------------------------
void _08006BC8(void *state, void *hdr)
{
    void *pkg = _08005F98();
    register u8  zb __asm__("r1");
    register u32 zw __asm__("r2");
    u16 p48;

    if ((s16) * (u16 *)((uintptr_t)hdr + 12) >= 0)
        *(u16 *)((uintptr_t)state + 10) = *(u16 *)((uintptr_t)hdr + 12);
    else
        *(u16 *)((uintptr_t)state + 10) = 0;

    *(u32 *)((uintptr_t)state + 0)  = *(u32 *)((uintptr_t)pkg + 20);
    *(u32 *)((uintptr_t)state + 4)  = *(u32 *)((uintptr_t)pkg + 24);
    p48 = *(u16 *)((uintptr_t)pkg + 48);
    zb = 0;
    zw = 0;
    *(u16 *)((uintptr_t)state + 8) = p48;
    *(s32 *)((uintptr_t)state + 12) = (s32)(s16) * (u16 *)((uintptr_t)hdr + 16);
    *(u8 *)((uintptr_t)state + 16) = zb;
    *(u16 *)((uintptr_t)state + 18) = *(u16 *)((uintptr_t)hdr + 14);
    *(u32 *)((uintptr_t)state + 24) = zw;
    *(u8 *)((uintptr_t)state + 17) = zb;
    *(u16 *)((uintptr_t)state + 20) = (u16)zw;
}

// asm/course_stream_more.s:90-91 defines BOTH `sub_08006BC8:` and `_08006BC8:` on
// this one body, so the C replacement must publish both spellings. The manifest
// `export` for this body records both; without the `sub_` form the underscore-form
// definition alone leaves the `sub_` symbol undefined at the slice link.
#ifndef __APPLE__
void sub_08006BC8(void *state, void *hdr) __attribute__((alias("_08006BC8")));
#endif

__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// 0x08006C10 sub_08006C10(state, col_delta) — wrapped column row lookup:
//   r4=state; r2 = (s16)[state+10] + col_delta
//   if u8[[state+0]] != 0:                       (raw column mode)
//     if r2 < 0: r2 += (s16)[state+8] repeatedly until >= 0
//     r2 = _0802DE9C(r2, (s16)[state+8])         (mod width)
//     goto done (shared tail)
//   if r2 < 0 or (s16)[state+8] <= r2: return 0  (list mode)
// done: return [state+4] + r2*88
//   (both paths share the *88 tail at 0x08006C4E)
// ----------------------------------------------------------------------------
void *_08006C10(void *state, int col_delta)
{
    int r2 = (int)(s16) * (u16 *)((uintptr_t)state + 10) + col_delta;

    if (*(u8 *)(*(uintptr_t *)((uintptr_t)state + 0)) != 0) {
        if (r2 < 0) {
            int w = (int)(s16) * (u16 *)((uintptr_t)state + 8);
            do {
                r2 += w;
            } while (r2 < 0);
        }
        r2 = _0802DE9C(r2, (int)(s16) * (u16 *)((uintptr_t)state + 8));
        goto done;
    }
    if (r2 < 0 || (int)(s16) * (u16 *)((uintptr_t)state + 8) <= r2)
        return (void *)0;
done:
    return (void *)(*(uintptr_t *)((uintptr_t)state + 4) + (uintptr_t)r2 * 88);
}
// The body is 78 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes `.text._08006C10` itself and fills a Thumb
// code section with `nop` (0x46c0), where the ROM holds `00 00`. This
// file-scope `.align` is emitted after `_08006C10`'s `.size` -- still inside
// its own section -- and pads with the `0` fill argument instead. No body byte
// changes; the two filler halfwords become 00 00. Assembling this same TU
// without -ffunction-sections also yields 00 00, which is the filler the
// original single-.text build produced, so this restores that byte for byte.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// 0x08006C60 sub_08006C60(state) — current-row package record getter:
//   row = _08006C10(state, 0)
//   if row == 0: return 0
//   if (s16)[row+10] < 0: return 0
//   pkg = _08005F98
//   return [pkg+8] + (s16)[row+10]*12      (r1 = 3*idx<<2 = idx*12)
// ----------------------------------------------------------------------------
void *_08006C60(void *state)
{
    void *row = _08006C10(state, 0);
    void *pkg;
    int idx;

    if (row == (void *)0)
        return (void *)0;
    if ((int)(s16) * (u16 *)((uintptr_t)row + 10) < 0)
        return (void *)0;
    pkg = _08005F98();
    idx = (int)(s16) * (u16 *)((uintptr_t)row + 10);
    return (void *)(*(uintptr_t *)((uintptr_t)pkg + 8) + (uintptr_t)idx * 12);
}
// Same 2-byte tail filler as _08006C10 above: the body is 50 bytes and gas
// would close the section with a Thumb `nop` where the ROM holds `00 00`.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// 0x08006C94 sub_08006C94(state, pos) — cross-product heading leaf:
//   r2=state, r3=[pos+0], [state+12] → r4 = r3 - [state+12]
//   r1=[pos+4], [state+16] → r5 = r1 - [state+16]
//   return (s16)[state+20]*r5 - (s16)[state+22]*r4
//
// WHY, in the compiler (build/toolchains/agbcc/gcc/local-alloc.c). This is not
// global_alloc at all — every one of the 13 pseudos here is block-local, so
// `local_alloc` assigns all of them and `global_alloc` then reports "0 regs to
// allocate". `block_alloc` calls `combine_regs` (local-alloc.c:1533) for each
// operand of each insn, and the tie fires when the condition at
// local-alloc.c:1666 holds,
//     (already_dead || find_regno_note (insn, REG_DEAD, ureg))
//     && reg_meets_class_p (sreg, qty_min_class[reg_qty[ureg]])
// — i.e. when an input DIES at the insn, the input and the output become one
// quantity and one hard register. `[pos+0]` dies at the first `subs`, so the
// difference and its left operand fuse: `subs r3, r3, r0`, one callee-save,
// `push {r4, lr}`.
//
// Defeating the tie is necessary but NOT sufficient, which is why and
// both stalled. `find_free_reg` (local-alloc.c:1868) picks the lowest
// hard reg not live over [born_index, dead_index) of the whole quantity. The
// untied difference has live range [insn 15, insn 28) and r3 is NOT live in
// that window (r3's only other use, `movs r3, #22` at insn 27, is reload's
// choice for the ldrsh scratch, assigned after local_alloc has finished). So an
// untied difference takes r3 anyway, and quantities are ordered by
// floor_log2(refs)*refs*size/live_length — dy (2*2/3) ahead of dx (2*2/8) —
// which cannot yield the ROM's dx=r4, dy=r5 at all. No statement order, no
// naming, no type and no arity change can move a quantity's position in that
// order past a value the RTL requires. The only C construct that addresses the
// decision itself is a local register variable, so the six live values are
// pinned.
//
// Every pin is load-bearing; perturbing one and recompiling:
//     dx r4 EXACT | r6 32/36 | r5  7/36 (28-byte body — dx and dy collide)
//     py r1 EXACT | r2 26/36 | r0 10/36 (24-byte body — collides with the
//                                  `state` pointer, which is live throughout)
//     px pinned EXACT | unpinned 34/36 (first diff +0x04)
//     sx pinned EXACT | unpinned 34/36 (first diff +0x06)
//
// The two differences MUST be pinned to different registers (r4/r5) and MUST
// stay live across the two multiplies: that is what makes the prologue
// `push {r4, r5, lr}` and the epilogue `pop {r4, r5}` / `pop {r1}` / `bx r1`.
// Nothing here changes the computation — the pinned locals are the same
// values the plain form leaves to the allocator, in the same order.
// ----------------------------------------------------------------------------
int _08006C94(void *state, void *pos)
{
    register int px __asm__("r3") = *(int *)((uintptr_t)pos + 0);
    register int sx __asm__("r0") = *(int *)((uintptr_t)state + 12);
    register int dx __asm__("r4") = px - sx;
    register int py __asm__("r1") = *(int *)((uintptr_t)pos + 4);
    register int sy __asm__("r0") = *(int *)((uintptr_t)state + 16);
    register int dy __asm__("r5") = py - sy;

    return (int)(s16) * (u16 *)((uintptr_t)state + 20) * dy
         - (int)(s16) * (u16 *)((uintptr_t)state + 22) * dx;
}

// ROM entry alias.
#ifndef __APPLE__
void * Sub_08006C10(void *state, int col_delta) __attribute__((alias("_08006C10")));
#endif
