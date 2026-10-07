// ============================================================================
// course_resource_more.c — C lift of asm/course_resource_more.s
// (VMA 0x0800798C–0x08007ABC, 4 funcs)
//
// Course-record value-tag placement helpers. Each walks the record array
// behind a course-record cursor record and, for every entry whose u32 type
// word is 4, calls the placement helper _08007ABC(base, i, value) with a
// running u16 cursor that accumulates each placed record's u16 width
// (fetched via _0800748C + ldrh [r0,#0]).
//
// Transcribed instruction-for-instruction from the cited asm listing.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void _0802D974(const void *a, void *b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) int   _08007484(void *rec) { (void)rec; return 0; }
__attribute__((weak)) int   _08007488(void *rec) { (void)rec; return 0; }
__attribute__((weak)) void *_08007498(void *base, int i) { (void)base; (void)i; return 0; }
__attribute__((weak)) void *_0800748C(void *rec) { (void)rec; return 0; }
__attribute__((weak)) void  _08007ABC(void *base, int i, int v) { (void)base; (void)i; (void)v; }
__attribute__((weak)) void *sub_0800572C(u32 s) { (void)s; return 0; }
#endif

// asm callees — all already exact; bound via extern decl + VMA hint.
extern void  _0802D974(const void *src, void *dst, u32 ctrl); // CpuSet (bios_wrappers.c)
extern int   _08007484(void *rec);        // 0x08007484 base accessor
extern int   _08007488(void *rec);        // 0x08007488 count accessor
extern void *_08007498(void *base, int i);// 0x08007498 record-array elt
extern void *_0800748C(void *rec);        // 0x0800748C record data ptr
extern void  _08007ABC(void *base, int i, int value); // EventBind
// 0x0800572C save-alloc cursor. Closure spelling -- the screen exports it -- and
// the signature MUST match the definition in src/save.c:813 (`void *(u32)`).
// Declaring it `int`/`int` here is undefined behaviour even though both are one
// word and the emitted bytes are unaffected.
extern void *sub_0800572C(u32 s);

// ----------------------------------------------------------------------------
// 0x0800798C sub_0800798C(rec, out)
//   push {r4,r5,lr}; sub sp,#4; r5=rec, r4=out
//   [sp]=0
//   CpuSet(&zero, out, 0x05000002)           @ src=sp, dst=out (r1 is already
//                                           @ `out` from the entry, so agbcc
//                                           @ reuses it and emits NO r1 setup),
//                                           @ ctrl = pool word 0x05000002
//                                           @ loaded straight into r2. Passing
//                                           @ the constant as arg2 instead costs
//                                           @ `mov r2,#0` (2 B) and shifts the pool
//                                           @ word to +40, i.e. 44 B vs 40.
//   out[+4] = _08007484(rec)
// ----------------------------------------------------------------------------
void _0800798C(void *rec, void *out)
{
    u32 zero = 0;
    _0802D974(&zero, out, (u32)0x05000002u);
    *(int *)((uintptr_t)out + 4) = _08007484(rec);
}
#ifndef __APPLE__
void Course_0x0800798C(void *a, void *b) __attribute__((alias("_0800798C"))); /* trampoline elimination: friendly-name spelling used by rec35_init.c */
void sub_0800798C(void *a, void *b) __attribute__((alias("_0800798C")));
void Sub_0800798C(void *a, void *b) __attribute__((alias("_0800798C")));
#endif

// ----------------------------------------------------------------------------
// 0x080079B4 sub_080079B4(rec, cursor)
//   push {r4,r5,r6,r7,lr}; r7=r8; push {r7}   @ r8 saved on the stack, reused
//                                            @ as the loop bound `n`
//   r4=cursor (r1, full 32 bits), r7=_08007484(rec) base, r8=_08007488(base)
//   r6 = cursor, r5 = i
//   loop i in [0,count):
//     e=_08007498(base,i); if [e]==4:
//       _08007ABC(base, i, r6)
//       r6 += u16[_0800748C(e)]
//   (returns; cursor lane written back inside _08007ABC call chain)
// `cursor` is u32, not u16: the ROM copies r1 into r4 whole at 0x080079BA
// (`adds r4, r1, #0`), so a u16 parameter would force the `lsls/lsrs`
// narrowing pair into the prologue and cost four bytes there.
// `_08007488` takes the BASE, not `rec`: the ROM leaves the result of
// _08007484 in r0 and calls straight into 0x0800748C with no intervening
// `mov`, so `rec` never has to stay live and r5 is free to be the counter.
// The accumulate goes through a named temporary with the loaded width on the
// left; written inline, agbcc folds it to `adds r6, r6, r0` instead of the
// ROM's `adds r6, r0, r6`.
// ----------------------------------------------------------------------------
void _080079B4(void *rec, u32 cursor)
{
    int base = _08007484(rec);
    int n = _08007488((void *)(uintptr_t)base);
    int i;
    u32 cur = cursor;

    for (i = 0; i < n; i++) {
        int *e = (int *)_08007498((void *)(uintptr_t)base, i);
        if (e[0] != 4)
            continue;
        _08007ABC((void *)(uintptr_t)base, i, (int)cur);
        {
            u32 t = (u32)*(u16 *)_0800748C((void *)e) + cur;
            cur = t;
        }
    }
}

// ----------------------------------------------------------------------------
// 0x08007A04 sub_08007A04(rec, value)
//   r5=rec, r4=value; r7=[rec+4] base, r8=_08007488(base)
//   [rec+0] = (u16)value
//   r6 = (u16)value
//   loop i in [0,count):  e=_08007498(base,i); if [e]==4:
//       _08007ABC(base, i, r6); r6 += u16[_0800748C(e)]
// `value` is u32, not u16: the ROM copies r1 into r4 whole at 0x08007A0C and
// only narrows it after the call (strh r4 then lsls/lsrs into r6), so a u16
// parameter would truncate at the prologue and cost the four bytes there.
// The accumulate goes through a named temporary with the loaded width on the
// left; written inline, agbcc folds it to `adds r6, r6, r0` instead of the
// ROM's `adds r6, r0, r6`.
// ----------------------------------------------------------------------------
void _08007A04(void *rec, u32 value)
{
    int base = *(int *)((uintptr_t)rec + 4);
    int n = _08007488((void *)(uintptr_t)base);
    int i;
    u32 cur;

    *(u16 *)((uintptr_t)rec + 0) = (u16)value;
    cur = (u32)(u16)value;

    for (i = 0; i < n; i++) {
        int *e = (int *)_08007498((void *)(uintptr_t)base, i);
        if (e[0] != 4)
            continue;
        _08007ABC((void *)(uintptr_t)base, i, (int)cur);
        {
            u32 t = (u32)*(u16 *)_0800748C((void *)e) + cur;
            cur = t;
        }
    }
}
#ifndef __APPLE__
// The closure spells this VMA `sub_08007A04` and the screen exports it, so the
// owning TU has to define that exact spelling. One hop to the real body: an
// alias-of-an-alias would not be in the slice link.
void sub_08007A04(void *rec, u32 value) __attribute__((alias("_08007A04")));
#endif

// ----------------------------------------------------------------------------
// 0x08007A58 sub_08007A58(rec)
//   r4=rec; r7=[rec+4] base, r8=_08007488(base)
//   r0 = sub_0800572C(0)            @ alloc/claim cursor slot
//   [rec+0] = (u16)r0            @ strh first, then narrow (0x08007A70/72/74)
//   r6 = (u16)r0
//   loop i in [0,count): e=_08007498(base,i); if [e]==4:
//       _08007ABC(base, i, r6)
//       sub_0800572C(u16[_0800748C(e)])   @ result discarded (0x08007A9E)
//       r6 += u16[_0800748C(e)]       @ the asm really does call _0800748C
//                                       twice, at 0x08007A98 and 0x08007AA4
// ----------------------------------------------------------------------------
void _08007A58(void *rec)
{
    int base = *(int *)((uintptr_t)rec + 4);
    int n = _08007488((void *)(uintptr_t)base);
    void *r0 = sub_0800572C(0);
    int i;
    u32 cur;

    *(u16 *)((uintptr_t)rec + 0) = (u16)(uintptr_t)r0;
    cur = (u32)(u16)(uintptr_t)r0;

    for (i = 0; i < n; i++) {
        int *e = (int *)_08007498((void *)(uintptr_t)base, i);
        if (e[0] != 4)
            continue;
        _08007ABC((void *)(uintptr_t)base, i, (int)cur);
        sub_0800572C((u32)*(u16 *)_0800748C((void *)e));
        {
            u32 t = (u32)*(u16 *)_0800748C((void *)e) + cur;
            cur = t;
        }
    }
}
#ifndef __APPLE__
void Course_0x08007A58(void *p) __attribute__((alias("_08007A58"))); /* trampoline elimination: friendly-name spelling used by rec35_init.c */
void sub_08007A58(void *p) __attribute__((alias("_08007A58")));
void Sub_08007A58(void *p) __attribute__((alias("_08007A58")));
#endif
