#include "gtadv/foundation.h"
#include "gtadv/memory.h"
#include "gba/regs.h"
#include "gba/bios.h"   // Div/DivRem on the host __APPLE__ fallbacks below
void _080038A4(int a, u32 b, const volatile u8 *c);  /* alias of ObjLane_038A4, below */
extern void _08002C34(int a, void *b);   // foundation_runtime.c free-list push
extern int _08005790(u32 a);             // save.c allocator (returns the new cursor)
extern int _08005758(u32 a);   // save.c allocator (returns the new cursor)
extern void _08007538(void *a, int b, void *c);   // course_resource.c
extern void _08007614(void *a, int b, int c, int d);  // course_resource.c
#ifndef __APPLE__
// Closure spellings for the two measured helpers called below. The `_` twins
// are what the ROM closure defines at 0x08003CB4 / 0x0800295C; the friendly
// names exist only on the host, hence the split at each call site.
extern int sub_08003CB4(int val, volatile u8 *buf, int width);
// NOT `_08003CB4`: it is also a closure label, but nothing in src/ defines
// it, so a call under that name bypasses the C body.
extern void _0800295C(u32 a0, u32 a1);
#endif

// Forward declarations for the slice-closure `_` spellings; each is defined
// below as an alias of the same body as its `sub_` twin (promotion rule 1).
extern void sub_08003560(int a, u32 b, u32 c, u32 d, const volatile u8 *e);  /* defined below, alias of ObjList_03560 */
extern void sub_08003350(int a, u32 b, u32 c, u32 d, const volatile u8 *e, u16 f);  /* defined below, alias of ObjList_03350 */
extern void sub_0800364C(int a, u32 b, u32 c, u32 d, const volatile u8 *e);  /* defined below, alias of ObjCenter_0364C */
extern void sub_08003698(int a, u32 b, u32 c, u32 d, const volatile u8 *e);  /* defined below, alias of ObjCenter_03698 */
extern void sub_080036E0(int a, u32 b, u32 c, u32 d, const volatile u8 *e);  /* defined below, alias of ObjCenter_036E0 */
extern void sub_08003728(int a, u32 b, u32 c, u32 d, const volatile u8 *e);  /* defined below, alias of ObjCenter_03728 */
extern void sub_0800376C(int a, u32 b, u32 c, u32 d, int e, u32 f);  /* defined below, alias of ObjMeasure_0376C */
extern void sub_080037BC(int a, u32 b, u32 c, u32 d, int e, u32 f);  /* defined below, alias of ObjMeasure_037BC */
extern void sub_0800385C(int a, u32 b, const volatile u8 *c);  /* defined below, alias of ObjLane_0385C */
extern int sub_08003D4C(int a, volatile u8 *b);  /* defined below, alias of TimeStr_03D4C */
extern int sub_08003E34(int a, volatile u8 *b);  /* defined below, alias of TimeStr_03E34 */

// ----------------------------------------------------------------------------
// runtime_2aac.s tail lift — HUD/object families 0x08003104–0x08004A28.
//
// Transcribed 1:1 from asm/runtime_2aac.s (byte-exact asm at each cited VMA).
// AAPCS note: 5th+ parameters live on the stack; after the armcc high-register
// prologues here (20+12+40 = 72 bytes), [sp+72] = 5th param, [sp+76] = 6th, ….
// All mappings below follow the register/stack slots of the asm literally —
// field *meanings* are not asserted beyond what the asm shows.
//
// Anchors (pool words): IWRAM 0x03000134 (+6/+30/+22/+14 strh lanes, 32-byte
// records behind *(u32*)0x03000134), 0x03000138/13C/140/150/154, 0x03000160
// (lane table), 0x03000178 (sprite table {base,count}), 0x03000180/188
// (camera ctrl {x,y,u16 ang,ptr}), 0x0203F6B0/6C0/700/724/728, 0x03000198
// subsystem block (+132/136/140/144 alloc cursor).
// ROM data: 0x080C4BA0 stride/size table; 0x080C49A0 s16 advance table;
// templates 0x0802E210/E220/E230/E240 = 4-word fn-ptr tables (entries
// 0x0800315C-family |1); record strips 0x08032260/0x0802E260/0x0802FA60/
// 0x08030A60 and 0x080C53E4/0x080C5FE4; rotation table 0x08033260.
// ----------------------------------------------------------------------------

// HOST_STUB = weak on Apple host builds, extern on ARM. A host link with
// -undefined dynamic_lookup binds lazily, so a bare extern would leave
// the symbol silently undefined.
#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

#ifndef __APPLE__
extern int Div(int a, int b);            // sub_0802D978 (bios_wrappers.c)
extern int DivRem(int a, int b);         // sub_0802D97C
extern int MathAbs(int v);               // 0x08005B5C (foundation_math.c)
extern void MathRot(void *p, int angle); // 0x08005BA8
extern void CpuFastSet(const void *src, void *dst, u32 mode); // bios_wrappers.c (swi 0x0C)
extern int TimeStr_03D4C(int val, volatile u8 *buf);  // defined below in this file
extern int TimeStr_03E34(int val, volatile u8 *buf);
#endif

extern void *_08002BFC(int a);   // 16-byte object alloc (foundation_runtime.c)
void _08003838(int a, u32 b, const volatile u8 *c);  // ObjLane_03838, aliased below
int _08003E34(int a, volatile u8 *b);  // defining name of TimeStr_03E34, below
int TimeStr_03D18(int val, volatile u8 *buf);  // defined below in this file

// The ROM bodies at 0x080041E0 and 0x080041F8 issue `bl 0x0802DE04`, the
// signed EABI idiv, not the swi Div wrapper the other Div users in this file
// call. Only those two below are retargeted; every other Div user keeps Div.
HOST_STUB(int sub_0802DE04(int a, int b));   // 0x0802DE04 (asm/blocka.s)
HOST_STUB(void _08005BA8(void *p, int angle)); // 0x08005BA8 VMA spelling for slice link
HOST_STUB(int sub_080042C4(int a, int b, volatile u32 *c)); // 0x080042C4 VMA spelling for slice link
HOST_STUB(int sub_0802D978(int a, int b)); // 0x0802D978 swi 0x06 Div quotient
HOST_STUB(int sub_0802D97C(int a, int b)); // 0x0802D97C swi 0x06 DivRem remainder

// ----------------------------------------------------------------------------
// Entry-writer family 0x0800315C/3194/31D4/3210 — write three halfwords at
// obj = *(void**)req from the request block
//   req = {+0 obj, +4 f1, +8 f2, +12 f3, +16 src, +20 f5}
//   [obj+0] = 0x8000 | (u16)f2        (0x03210: 0x8400 | f2)
//   [obj+2] = (u16)f1
//   [obj+4] = ((u16)src[2] + 2*f3) | ((u16)src[0] + f5(+12 for 0x31D4)) << 12
//             (0x03194 additionally | 0x400)
// Guard: f3 < 0 skips the writes. Returns the obj pointer either way.
// The four bodies are spelled out separately rather than sharing a helper:
// each is 56/64/60/56 bytes and they differ in constant choice, register
// allocation and instruction ORDER (0x31D4 loads [src+2] straight into r2 and
// folds the +12 into r0 before the shift, where 0x3194 computes the src[0]
// half first). A shared helper cannot express that, so the family is four
// independent bodies.
//
// 0x0800315C / 0x08003194 / 0x08003210 are now EXACT (promoted,
// 176 B). Two further levers were needed on top of the pins:
//
//  1. The +4 field is read at WORD width. The ROM has `ldr r0,[r3,#4]` then
//     `strh r0,[r4,#2]`; a `u16` read compiles to `ldrh`, 2 bytes short.
//     `register u32 f1 __asm__("r0") = *(volatile u32 *)(p + 4);` storing
//     `(u16)f1` is the fix.
//  2. `v |= 0x400` and the halfword tag must be MATERIALISED IN ONE REGISTER
//     AND COPIED TO ANOTHER, as the ROM does (`movs r5,#128 / lsls r5,r5,#8 /
//     adds r0,r5,#0`). agbcc rematerialises the constant straight into the
//     destination and drops the copy, so the constant is pinned to its ROM
//     register and a non-emitting `__asm__("" : "+r" (tagc));` forbids the
//     rematerialisation. The copy then survives only because the pinned
//     source register is OVERWRITTEN between the copy and the use
//     (`tagv = tagc; tagc = the load; tagv |= tagc;`); without the overwrite
//     agbcc coalesces the copy away and the body is 2 bytes short. For the
//     0x400 OR both ends of the copy are themselves barred, since neither is
//     overwritten: `bitc` in r2, `bitv` in r0, a barrier on each.
//     The barrier buys the register but changes the following add's FORM
//     (ROM `adds r1,r2` 0x1851 vs 0x1889 with a barrier), so measure both.
//
// 0x080031D4 reaches 58/60 with both levers and stays unpromoted: its only
// miss is the src[2] load DESTINATION, where the ROM coalesces the dest onto
// the dying base (`ldrh r2,[r2,#2]`) and agbcc picks r0. Forcing r2 flips the
// add to 0x1889; `register volatile` costs a stack frame. Axis recorded, not
// settled.
void _0800315C(volatile u8 *req)
{
    register volatile u8 *p __asm__("r3") = req;
    register volatile u8 *o __asm__("r4") = *(volatile u8 **)(p + 0);
    register s32 f3 __asm__("r1") = *(volatile s32 *)(p + 12);
    if (f3 >= 0) {
        register volatile u8 *src __asm__("r2") = *(volatile u8 **)(p + 16);
        register u32 tagc __asm__("r5") = 0x8000u;
        register u32 tagv __asm__("r0");
        __asm__("" : "+r" (tagc));
        tagv = tagc;
        tagc = *(volatile u16 *)(p + 8);
        tagv |= tagc;
        *(volatile u16 *)(o + 0) = (u16)tagv;
        register u32 f1 __asm__("r0") = *(volatile u32 *)(p + 4);
        *(volatile u16 *)(o + 2) = (u16)f1;
        u32 v = (u32)(f3 << 1);
        v += *(volatile u16 *)(src + 2);
        register u32 idxv __asm__("r0") = *(volatile u32 *)(p + 20);
        idxv += *(volatile u16 *)(src + 0);
        v |= idxv << 12;
        *(volatile u16 *)(o + 4) = (u16)v;
    }
}
// Body is 54 bytes, so the -ffunction-sections section pads to 56; gas would
// close a Thumb code section with the 2-byte `nop` filler (0x46c0) where the
// ROM holds `00 00`. A file-scope `.align 2, 0` lands after the body's `.size`,
// still inside its own section, and pads with the explicit `0` fill.
__asm__(".align 2, 0");

// 0x08003210 — 56 B. Same shape as 0x0800315C with the 0x8400 halfword tag.
void _08003210(volatile u8 *req)
{
    register volatile u8 *p __asm__("r3") = req;
    register volatile u8 *o __asm__("r4") = *(volatile u8 **)(p + 0);
    register s32 f3 __asm__("r1") = *(volatile s32 *)(p + 12);
    if (f3 >= 0) {
        register volatile u8 *src __asm__("r2") = *(volatile u8 **)(p + 16);
        register u32 tagc __asm__("r5") = 0x8400u;
        register u32 tagv __asm__("r0");
        __asm__("" : "+r" (tagc));
        tagv = tagc;
        tagc = *(volatile u16 *)(p + 8);
        tagv |= tagc;
        *(volatile u16 *)(o + 0) = (u16)tagv;
        register u32 f1 __asm__("r0") = *(volatile u32 *)(p + 4);
        *(volatile u16 *)(o + 2) = (u16)f1;
        u32 v = (u32)(f3 << 1);
        v += *(volatile u16 *)(src + 2);
        register u32 idxv __asm__("r0") = *(volatile u32 *)(p + 20);
        idxv += *(volatile u16 *)(src + 0);
        v |= idxv << 12;
        *(volatile u16 *)(o + 4) = (u16)v;
    }
}
__asm__(".align 2, 0");

// 0x08003194 — 64 B. 0x8000 tag, no +12, ORs 0x400 into the size halfword.
void _08003194(volatile u8 *req)
{
    register volatile u8 *p __asm__("r3") = req;
    register volatile u8 *o __asm__("r4") = *(volatile u8 **)(p + 0);
    register s32 f3 __asm__("r1") = *(volatile s32 *)(p + 12);
    if (f3 >= 0) {
        register volatile u8 *src __asm__("r2") = *(volatile u8 **)(p + 16);
        register u32 tagc __asm__("r5") = 0x8000u;
        register u32 tagv __asm__("r0");
        __asm__("" : "+r" (tagc));
        tagv = tagc;
        tagc = *(volatile u16 *)(p + 8);
        tagv |= tagc;
        *(volatile u16 *)(o + 0) = (u16)tagv;
        register u32 f1 __asm__("r0") = *(volatile u32 *)(p + 4);
        *(volatile u16 *)(o + 2) = (u16)f1;
        u32 v = (u32)(f3 << 1);
        v += *(volatile u16 *)(src + 2);
        register u32 idxv __asm__("r0") = *(volatile u32 *)(p + 20);
        idxv += *(volatile u16 *)(src + 0);
        v |= idxv << 12;
        register u32 bitc __asm__("r2") = 0x400u;
        register u32 bitv __asm__("r0");
        __asm__("" : "+r" (bitc));
        bitv = bitc;
        __asm__("" : "+r" (bitv));
        v |= bitv;
        *(volatile u16 *)(o + 4) = (u16)v;
    }
}
__asm__(".align 2, 0");

// 0x080031D4 — 60 B. 0x8000 tag, ORs 0x400, and folds +12 into the pool word
// before the shift (so the src[0] halfword is never read at all).
void _080031D4(volatile u8 *req)
{
    register volatile u8 *p __asm__("r3") = req;
    register volatile u8 *o __asm__("r4") = *(volatile u8 **)(p + 0);
    register s32 f3 __asm__("r1") = *(volatile s32 *)(p + 12);
    if (f3 >= 0) {
        register volatile u8 *src __asm__("r2") = *(volatile u8 **)(p + 16);
        register u32 tagc __asm__("r5") = 0x8000u;
        register u32 tagv __asm__("r0");
        __asm__("" : "+r" (tagc));
        tagv = tagc;
        tagc = *(volatile u16 *)(p + 8);
        tagv |= tagc;
        *(volatile u16 *)(o + 0) = (u16)tagv;
        register u32 f1 __asm__("r0") = *(volatile u32 *)(p + 4);
        *(volatile u16 *)(o + 2) = (u16)f1;
        u32 v = (u32)(f3 << 1);
        v += *(volatile u16 *)(src + 2);
        register u32 idxv __asm__("r0") = *(volatile u32 *)(p + 20);
        idxv += 12;
        v |= idxv << 12;
        register u32 bitc __asm__("r2") = 0x400u;
        register u32 bitv __asm__("r0");
        __asm__("" : "+r" (bitc));
        bitv = bitc;
        __asm__("" : "+r" (bitv));
        v |= bitv;
        *(volatile u16 *)(o + 4) = (u16)v;
    }
}
__asm__(".align 2, 0");

#ifndef __APPLE__
void sub_0800315C(volatile u8 *a) __attribute__((alias("_0800315C")));
void sub_08003194(volatile u8 *a) __attribute__((alias("_08003194")));
void sub_080031D4(volatile u8 *a) __attribute__((alias("_080031D4")));
void sub_08003210(volatile u8 *a) __attribute__((alias("_08003210")));
#endif

// 0x08003148 — strlen (byte loop). Shared label with the bx-r0 entry above.
u32 strlength_03148(const volatile u8 *s)
{
    const volatile u8 *p = s;
    u32 n = 0;
    while (*p) {
        n++;
        p++;
    }
    return n;
}
#ifndef __APPLE__
u32 sub_08003148(const volatile u8 *s) __attribute__((alias("strlength_03148")));
#endif

// ----------------------------------------------------------------------------
// Lane emitters 0x08003248/270/298 — _08007538(0x087ACF70, k, a) then
// _08007614(0x087ACF70, k+3, 0, b) with k = 0/1/2.
static void lane_emit(int a, int b, int k)
{
    _08007538((void *)(uintptr_t)0x087ACF70, k, (void *)(uintptr_t)a);
    _08007614((void *)(uintptr_t)0x087ACF70, k + 3, 0, b);
}
void LaneEmit_0248(int a, int b) { lane_emit(a, b, 0); }
void LaneEmit_0270(int a, int b) { lane_emit(a, b, 1); }
void LaneEmit_0298(int a, int b) { lane_emit(a, b, 2); }
// 0x08000248/0x08000270/0x08000298 are the VMA addresses of these three
// bodies. The `sub_0800324x` names below were WRONG: 0x08003248/0x08003270/
// 0x08003298 are a different function entirely (Wrap_03248/Wrap_03270/
// Wrap_03298 in src/foundation_runtime.c, called from asm/runtime_2aac.s).
// An alias binds a name to a body, so declaring `sub_08003248` here both
// pointed the closure's spelling at the wrong body and denied the real owner
// the export its manifest entry needs. The `sub_` spellings are declared next
// to their true owners instead.
#ifndef __APPLE__
void _08000248(int a, int b) __attribute__((alias("LaneEmit_0248")));
void _08000270(int a, int b) __attribute__((alias("LaneEmit_0270")));
void _08000298(int a, int b) __attribute__((alias("LaneEmit_0298")));
#endif

// 0x080032C0 — _08007614(0x087ACF70, 3, u16[0x03000160] + a, b)
void LaneEmit_02C0(int a, int b)
{
    void *tbl = (void *)(uintptr_t)0x087ACF70;
    u16 off = *(volatile u16 *)(uintptr_t)0x03000160;
    a += off;
    _08007614(tbl, 3, b, a);
}
#ifndef __APPLE__
void sub_080032C0(int a, int b) __attribute__((alias("LaneEmit_02C0")));
#endif

// ----------------------------------------------------------------------------
// 0x080032E0 — lane driver. For lane 0..3 over 4-byte records at arg:
//   [lane*4 + 0x03000160 + 2] = _08005758(0x080C4BB0[lane])
//   [lane*4 + 0x03000160 + 0] = _08005790(rec[1])
//   lane 0/2/3: emit (u16[+2], u16[+0]) via 0x3248/0x3270/0x3298.
// (0x080C4BB0 = 0x080C4BA0 + 0x10, read at loop entry for each lane.)
void LaneDriver_02E0(volatile u8 *rec)
{
    for (int lane = 0; lane <= 3; lane++, rec += 4) {
        if (rec[0] == 0)
            continue;
        volatile u8 *cell = (volatile u8 *)(uintptr_t)(0x03000160 + lane * 4);
        u32 alloc_arg = *(volatile u32 *)(uintptr_t)(0x080C4BB0 + lane * 4);
        *(volatile u16 *)(cell + 2) = (u16)_08005758((int)alloc_arg);
        *(volatile u16 *)(cell + 0) = (u16)_08005790(rec[1]);
        if (lane == 1)
            continue;
        u16 a = *(volatile u16 *)(cell + 2);
        u16 b = *(volatile u16 *)(cell + 0);
        if (lane == 0)
            LaneEmit_0248(a, b);
        else if (lane == 2)
            LaneEmit_0270(a, b);
        else if (lane == 3)
            LaneEmit_0298(a, b);
    }
}
#ifndef __APPLE__
void sub_080032E0(volatile u8 *p) __attribute__((alias("LaneDriver_02E0")));
void _080032E0(volatile u8 *p) __attribute__((alias("LaneDriver_02E0")));
#endif

// ----------------------------------------------------------------------------
// Object-list builders 0x08003350/03400/034B0/03560/03FD4 — per-glyph loop.
//   f(sel, x, a2, a3, str, …):
//     tmpl[4] <- ROM template table; lane = table[sel] (0x080C4BA0);
//     req = {obj, x&0x1FF, a2, adv-1, 0x03000160+sel*4, a3};
//     for each byte c of str: adv = s16[0x080C49A0 + c*2] - 1;
//       if adv >= 0: obj = _08002BFC(0); [obj+12] = w5 (u16; 0x03400/0x03FD4
//       read w6 instead); req.obj = obj; tmpl[sel](req) through the ROM's
//       0x0802DDEC register-dispatch veneer (r9 = tmpl[sel], r8 = req);
//       _08002C34(0, obj);
//       x += lane.
typedef u32 (*tmpl_fn)(u32 *req); /* req = u32[6]: {obj, x&1FF, a2, adv, 0x03000160+sel*4, a3} — attr builders ldr rN,[r3,#N] word-wide */

// The former `static void obj_list(...)` helper is GONE, deleted not merely
// bypassed: agbcc emitted it once as a file-local `t` symbol that the probe's
// closure table cannot resolve (R_ARM_THM_CALL -> UNRESOLVED), so both
// wrappers read UNRESOLVED_RELOCATION and were unscorable. The ROM bodies at
// 0x08003400/0x080034B0 are self-contained modulo three real ROM `bl`s
// (0x08002BFC, the 0x0802DDEC register-dispatch veneer, 0x08002C34), so the
// loop is now spelled out in each body, on the proven ObjList_03350 frame.

typedef struct { u32 w[4]; } tmpl_tbl;

// ROM 0x08003350 (176 B). Same frame and the same 16-byte template block copy
// as 0x08003560, plus three facts of its own, all read off asm/runtime_2aac.s:
//  1. `req[1] = x & 0x1FF` — a POOL literal (0x000001FF) is loaded and `ands`
//     with the running x in r6; only this variant masks (0x08003560 stores r6
//     raw).
//  2. `add r0,sp,#76; ldrh r0,[r0,#0]; strh r0,[r4,#12]` writes the 6th
//     argument — read straight off the caller's stack slot, hence a `u16`
//     parameter that agbcc never materialises in a register.
//  3. `ldrh` lands in r1, so `adv` also lives in r1 and the branch is `cmp
//     r1,#0 / blt`; r0 is live across `bl sub_08002BFC` holding the mask, which
//     is why the allocator keeps the two apart.
//
// MEASURED  (U03350, probe-U03350/sweep.py): `rq` is deliberately
// UNPINNED. Pinning it `__asm__("r8")` scores 151/176; leaving agbcc to pick
// scores 158/176 and is what makes it hoist the `rq = req` into the preheader
// as the ROM's `mov r8,r4` (0x08003380) instead of emitting `adds r7,r1,#0;
// mov r8,r7` inside the loop. Pins on r4 for the req base were tried in ten
// shapes (extra `rb` node, all-stores-via-rq, rp-from-rb) and every one spills
// to 180 bytes or drops to ~55; do not re-try them. Declaration order is not a
// lever here either: all 17 permutations that keep `req` after `hdr` compile to
// the identical 158 bytes.
//
// Remaining delta is 18 bytes and is ONE root cause: agbcc gives the `req`
// base r1 where the ROM uses r4 (`add r4,sp,#16`). That single choice cascades
// into the 0x08003410-0x08003440 init stores and into the register numbers of
// the `0x080C49A0 + (*p << 1)` index (ROM r1/r2/r3, here r0/r1/r2) and of the
// w5 ldrh (ROM r0, here r3). Prologue, epilogue, frame (40 B) and the 0x33..0xAF
// loop body all match.
void ObjList_03350(int sel, u32 x, u32 a2, u32 a3, const volatile u8 *str, u16 w5)
{
    tmpl_tbl hdr;
    u32 req[6];
    u32 *rq;
    register u32 lane __asm__("r10");
    register u32 fnv  __asm__("r9");
    const volatile u8 *p = str;
    register int adv __asm__("r1");
    uintptr_t base;
    hdr = *(const tmpl_tbl *)(uintptr_t)0x0802E210u;
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    lane = *(volatile u32 *)(uintptr_t)(base + (u32)sel * 4);
    fnv = (u32)(uintptr_t)hdr.w[sel];
    req[4] = 0x03000160u + (u32)sel * 4;
    req[2] = a2;
    req[5] = a3;
    while (*p) {
        rq = req;
        adv = ((const s16 *)(uintptr_t)(0x080C49A0 + ((u32)*p << 1)))[0] - 1;
        req[1] = x & 0x1FF;
        req[3] = (u32)adv;
        if (adv >= 0) {
            u32 obj = (u32)(uintptr_t)_08002BFC(x & 0x1FF);
            *(volatile u16 *)(uintptr_t)(obj + 12) = w5;
            req[0] = obj;
            ((tmpl_fn)(uintptr_t)fnv)(rq);
            _08002C34(0, (void *)(uintptr_t)obj);
        }
        p++;
        x += lane;
    }
}

// ROM 0x08003400 (176 B) — inline obj_list, template table 0x0802E220.
// Per-body facts over 0x08003350 (same 176 B, 158/176 there):
//  * the `[obj+12]` halfword is read from `add r0,sp,#80` = entry-sp+8 = the
//    SEVENTH parameter (w6), not the sixth; 0x080034B0 uses `add r0,sp,#76`
//    = entry-sp+4 = the SIXTH (w5). That, plus the template base, is the whole
//    per-body difference — the two ROM streams are otherwise instruction-for-
//    instruction identical.
//  * `_08002BFC` is entered with r0 already holding `x & 0x1FF` (`ands r0,r6`
//    at 0x0800345C feeds the call directly), so the argument is spelled out
//    rather than passed as 0.
void ObjList_03400(int sel, u32 x, u32 a2, u32 a3, const volatile u8 *str, u32 w5, u32 w6)
{
    tmpl_tbl hdr;
    u32 req[6];
    u32 *rq;
    register u32 lane __asm__("r10");
    register u32 fnv  __asm__("r9");
    const volatile u8 *p = str;
    register int adv __asm__("r1");

    uintptr_t base;
    // NOTE: no `(void)w5;` here. With the cast present agbcc materialises the
    // unused sixth parameter and inflates the frame to 44 (`sub sp,#44`), which
    // is the FIRST difference (+0x0A) against the ROM's `sub sp,#40`.
    hdr = *(const tmpl_tbl *)(uintptr_t)0x0802E220u;
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    lane = *(volatile u32 *)(uintptr_t)(base + (u32)sel * 4);
    fnv = (u32)(uintptr_t)hdr.w[sel];
    req[4] = 0x03000160u + (u32)sel * 4;
    req[2] = a2;
    req[5] = a3;
    while (*p) {
        rq = req;
        adv = ((const s16 *)(uintptr_t)(0x080C49A0 + ((u32)*p << 1)))[0] - 1;
        req[1] = x & 0x1FF;
        req[3] = (u32)adv;
        if (adv >= 0) {
            u32 obj = (u32)(uintptr_t)_08002BFC(x & 0x1FF);
            *(volatile u16 *)(uintptr_t)(obj + 12) = w6;
            req[0] = obj;
            ((tmpl_fn)(uintptr_t)fnv)(rq);
            _08002C34(0, (void *)(uintptr_t)obj);
        }
        p++;
        x += lane;
    }
}
// ROM 0x080034B0 (176 B) — same stream as 0x08003400 with two constants
// changed: template base 0x0802E230 (pool word at 0x0800354C) and the
// `[obj+12]` source slot `add r0,sp,#76` = the sixth parameter (w5).
// Its 18-byte residual is the SAME single defect as 0x08003400's and is
// diagnosed in full above this body (`&req` in r1 where the ROM uses r4).
void _080034B0(int sel, u32 x, u32 a2, u32 a3, const volatile u8 *str, u32 w5, u32 w6)
{
    tmpl_tbl hdr;
    u32 req[6];
    u32 *rq;
    register u32 lane __asm__("r10");
    register u32 fnv  __asm__("r9");
    const volatile u8 *p = str;
    register int adv __asm__("r1");
    uintptr_t base;
    (void)w6;
    hdr = *(const tmpl_tbl *)(uintptr_t)0x0802E230u;
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    lane = *(volatile u32 *)(uintptr_t)(base + (u32)sel * 4);
    fnv = (u32)(uintptr_t)hdr.w[sel];
    req[4] = 0x03000160u + (u32)sel * 4;
    req[2] = a2;
    req[5] = a3;
    while (*p) {
        rq = req;
        adv = ((const s16 *)(uintptr_t)(0x080C49A0 + ((u32)*p << 1)))[0] - 1;
        req[1] = x & 0x1FF;
        req[3] = (u32)adv;
        if (adv >= 0) {
            u32 obj = (u32)(uintptr_t)_08002BFC(x & 0x1FF);
            *(volatile u16 *)(uintptr_t)(obj + 12) = w5;
            req[0] = obj;
            ((tmpl_fn)(uintptr_t)fnv)(rq);
            _08002C34(0, (void *)(uintptr_t)obj);
        }
        p++;
        x += lane;
    }
}
// Residual, measured  : 15 bytes in 164, in two
// independent clusters, both downstream of register choice and NOT of shape.
//  * 9 bytes at +0x33,+0x35,+0x36,+0x38,+0x3A,+0x3C,+0x3E,+0x40,+0x44 -- the
//    `&req` node. The ROM computes `add r4,sp,#16`; agbcc computes
//    `add r1,sp,#16` and then uses r1 for all three init stores (`mov r8,r1`
//    at +0x44), which also forces the 0x03000160 pool load into r3 and the
//    a2/a3 copy temps into r7/r0 instead of the ROM's r3/r7. This is the SAME
//    single defect as 0x08003400's, at the SAME offset, and it is unreachable
//    for the same reason: `mov r4,sp` + `stmia r4!` is the first hard register
//    agbcc assigns, so the address is rematerialised into a call-clobbered
//    scratch and a pin receives only a copy (`add r1,sp,#16; adds r4,r1,#0`,
//    measured 73/164 here). Forcing the node callee-saved DOES win the class --
//    agbcc gave it r6 and reused r6 in the block copy `ldmia r1!,{r2,r3,r6}`,
//    the same reuse-after-death the ROM performs -- but r6 is `x`'s slot, so
//    `x` moves r6 -> r7 and the copy list follows it: 130/164.
//  * 6 bytes at +0x4C,+0x4E,+0x51,+0x52,+0x55,+0x56 -- the advance lookup.
//    The ROM holds the character in r1 (`ldrb r1,[r5,#0]`) and uses r2/r3 for
//    the table base and the ldrsh offset; agbcc folds the character into r0 and
//    uses r1/r2. One temp where the ROM has two, because the ROM's `c*2` shift
//    is a separate destination register while agbcc coalesces the character
//    and the shift into r0. Untried: pinning a named `u32 c = *p;` so `c` is
//    live across the shift. Not a span, order or reload problem -- the caller
//    list, the `bx r9` veneer and all three pool words already resolve.

void ObjList_03560(int sel, u32 x, u32 a2, u32 a3, const volatile u8 *str)
{
    tmpl_tbl hdr;
    u32 req[6];
    // The ROM keeps THREE live addresses for the same array: r4 for the three
    // init stores (`add r4,sp,#16`), r8 for the dispatch argument
    // (`mov r8,r4`... `mov r0,r8`), and r7 as the loop copy (`mov r7,r8`).
    // `rq` was once pinned to r8 here. Measured  :
    // the pin is DECORATION and costs 7 bytes -- agbcc picks r8 for `rq`
    // unprompted, and the pin only moved the copy past the `cmp`, inserting an
    // extra `adds r7,r1,#0` and shortening the `bne.n` span by 2. 142 -> 149.
    u32 *rq;
    // Pinned: the ROM keeps `lane` in sl (r10) and the template pointer in r9,
    // reaching the `bx r9` veneer at 0x0802DDEC. Unpinned, agbcc picks r9 for
    // `lane` and r8 for `fn` and emits `bl _call_via_r8` -- same shape, wrong
    // register, wrong veneer.
    register u32 lane __asm__("r10");
    register u32 fnv  __asm__("r9");
    // `p = str` is an INITIALISER, so it is the first executable statement: the
    // ROM loads the string pointer into r5 before the block copy
    // (`ldr r5,[sp,#72]` at 0x08003572). That makes r5 unavailable to the copy,
    // so agbcc loads through {r2,r3,r7} as the ROM does, and `a2` is parked in
    // r8 first (`mov r8,r2` -> `mov r3,r8` -> `str r3,[r4,#8]`) instead of
    // being stored straight out of r2, which that copy would clobber.
    const volatile u8 *p = str;
    int adv;
    uintptr_t base;

// The ROM loads the TEMPLATE address BEFORE the copy (0x08003576) and the
// LANE-table address AFTER it (0x08003580), so `hdr` is assigned first.
    hdr = *(const tmpl_tbl *)(uintptr_t)0x0802E240u;
// The lane-table pool load must stay AHEAD of `lsls r0,r0,#2`, which is what
// naming the base absolutely buys: a folded literal makes agbcc emit the load
// after the shift. Same guard and symbol spelling as the four neighbours above
// ( recipe).
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    lane = *(volatile u32 *)(uintptr_t)(base + (u32)sel * 4);
    fnv = (u32)(uintptr_t)hdr.w[sel];
    req[4] = 0x03000160u + (u32)sel * 4;
    req[2] = a2;
    req[5] = a3;
    while (*p) {
        rq = req;
        adv = ((const s16 *)(uintptr_t)(0x080C49A0 + ((u32)*p << 1)))[0] - 1;
        req[1] = x;
        req[3] = (u32)adv;
        if (adv >= 0) {
            // `bl sub_08002BFC` carries NO `movs r0,#0`: the ROM passes the
            // advance that is already in r0. ObjAlloc_02BFC is documented
            // `(void)a; // asm takes r0 but never reads it`, so this is the
            // same call, not a new argument.
            u32 obj = (u32)(uintptr_t)_08002BFC(adv);
            req[0] = obj;
            ((tmpl_fn)(uintptr_t)fnv)(rq);
            _08002C34(0, (void *)(uintptr_t)obj);
        }
        p++;
        x += lane;
    }
}
// 0x08003FD4 — template 0x0802E250; obj[+12] = w6; the template fn is called
// with two extra args (r1 = w6, r2 = u8 w7) which 0x315C-family fns ignore.
void ObjList_03FD4(int sel, u32 x, u32 a2, u32 a3, const volatile u8 *str,
                   u16 w5, u32 w6, u8 w7)
{
    u32 req[6];
    req[4] = 0x03000160u + (u32)sel * 4;
    req[5] = a3;
    req[2] = a2;
    u32 lane = *(volatile u32 *)(uintptr_t)(0x080C4BA0 + (u32)sel * 4);
    const volatile u8 *p = str;
    while (*p) {
        int adv = *(volatile s16 *)(uintptr_t)(0x080C49A0 + ((u32)*p << 1)) - 1;
        req[1] = x & 0x1FF;
        req[3] = (u32)adv;
        if (adv >= 0) {
            u32 obj = (u32)(uintptr_t)_08002BFC(0);
            *(volatile u16 *)(uintptr_t)(obj + 14) = w5;
            req[0] = obj;
            tmpl_fn fn = (tmpl_fn)(uintptr_t)((const u32 *)(uintptr_t)0x0802E250)[sel];
            fn(req);
            (void)w6; (void)w7; // passed in r1/r2 by the asm call site
            _08002C34(0, (void *)(uintptr_t)obj);
        }
        p++;
        x += lane;
    }
}
#ifndef __APPLE__
void _08003350(int a, u32 b, u32 c, u32 d, const volatile u8 *e, u16 f)
    __attribute__((alias("ObjList_03350")));
void sub_08003350(int a, u32 b, u32 c, u32 d, const volatile u8 *e, u16 f) __attribute__((alias("ObjList_03350")));
void sub_08003400(int a, u32 b, u32 c, u32 d, const volatile u8 *e, u32 f, u32 g) __attribute__((alias("ObjList_03400")));
void sub_080034B0(int a, u32 b, u32 c, u32 d, const volatile u8 *e, u32 f, u32 g) __attribute__((alias("_080034B0")));
void sub_08003560(int a, u32 b, u32 c, u32 d, const volatile u8 *e) __attribute__((alias("ObjList_03560")));
void sub_08003FD4(int a, u32 b, u32 c, u32 d, const volatile u8 *e, u16 f, u32 g, u8 h) __attribute__((alias("ObjList_03FD4")));
#endif

// ----------------------------------------------------------------------------
// Centering wrappers (x adjusted by per-char width, then delegated).
//   0x03604: x' = 120 - (table[sel]/2)*strlen(str); -> 0x03350(sel, x', a1, a2, str, 1)
//   0x0364C: x' = x - (table[sel]/2)*strlen(str);  -> 0x03350(sel, x', a2, a3, str, 1)
//   0x03698: x' = x - (table[sel]/2)*len;          -> 0x03560(sel, x', a2, a3, str)
//   0x036E0: x' = x - table[sel]*len;              -> 0x03350(sel, x', a2, a3, str, 1)
//   0x03728: x' = x - table[sel]*len;              -> 0x03560(sel, x', a2, a3, str)
static u32 table_at(u32 off) { return *(volatile u32 *)(uintptr_t)(0x080C4BA0 + off); }

void ObjCenter_03604(int sel, u32 a1, u32 a2, const volatile u8 *str)
{
    u32 half = table_at((u32)sel * 4) / 2;
    u32 x = 120 - half * strlength_03148(str);
    ObjList_03350(sel, x, a1, a2, str, 1);
}
// : the length loop is INLINED in all four ROM bodies --
// they contain no `bl` at all except the single delegate. The probe's
// UNRESOLVED_RELOCATION was the invented call to strlength_03148, so the fix
// is to write the loop, not to chase a relocation. The ROM's loop is
// `ldrb/cmp/beq` (first byte test) then `p++; x -= step; ldrb/cmp/bne`,
// which is exactly `while (*p) { p++; x -= step; }`: the decrement sits
// AFTER the pointer bump, and `step` is loop-invariant so agbcc keeps it in
// r1 across the loop exactly as the ROM does. Declaring `p` BEFORE the table
// read puts the ROM's `adds r2,r3` ahead of the pool load.
//
// The same rule kills the SECOND invented call: `table_at` is a `static`
// one-liner that agbcc does NOT inline, so it emitted a real `bl table_at`
// and the probe again read an unresolved relocation, now at +0x18. The pool
// load and the index scale are written out at each use site, which is what
// the ROM does (`ldr rX,[pc,#n]` then `lsls/adds/ldr`) and also what keeps
// 0x080C4BA0 in the literal pool instead of folded into a displacement.
// The divide-by-2 variant spells `(s32)v / 2`, which is the ROM's
// `lsrs r1,r0,#31; adds; asrs r1,r0,#1` round-up-for-negatives triple.
// The /2 is a SIGNED divide: the ROM's asrs only matches a signed C `/2`.
void ObjCenter_0364C(int sel, u32 x, u32 a2, u32 a3, const volatile u8 *str)
{
    const volatile u8 *p = str;
    uintptr_t base;
    u32 step;
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    step = (u32)((s32)*(volatile u32 *)(uintptr_t)(base + (u32)sel * 4) / 2);
    while (*p) { p++; x -= step; }
    sub_08003350(sel, x, a2, a3, str, 1);
}
void ObjCenter_03698(int sel, u32 x, u32 a2, u32 a3, const volatile u8 *str)
{
    const volatile u8 *p = str;
    uintptr_t base;
    u32 step;
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    step = (u32)((s32)*(volatile u32 *)(uintptr_t)(base + (u32)sel * 4) / 2);
    while (*p) { p++; x -= step; }
    sub_08003560(sel, x, a2, a3, str);
}
void ObjCenter_036E0(int sel, u32 x, u32 a2, u32 a3, const volatile u8 *str)
{
    const volatile u8 *p = str;
    uintptr_t base;
    u32 step;
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    step = *(volatile u32 *)(uintptr_t)(base + (u32)sel * 4);
    while (*p) { p++; x -= step; }
    sub_08003350(sel, x, a2, a3, str, 1);
}
void ObjCenter_03728(int sel, u32 x, u32 a2, u32 a3, const volatile u8 *str)
{
    const volatile u8 *p = str;
    uintptr_t base;
    u32 step;
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    step = *(volatile u32 *)(uintptr_t)(base + (u32)sel * 4);
    while (*p) { p++; x -= step; }
    sub_08003560(sel, x, a2, a3, str);
}
#ifndef __APPLE__
void sub_08003604(int a, u32 b, u32 c, const volatile u8 *d) __attribute__((alias("ObjCenter_03604")));
void sub_0800364C(int a, u32 b, u32 c, u32 d, const volatile u8 *e) __attribute__((alias("ObjCenter_0364C")));
void sub_08003698(int a, u32 b, u32 c, u32 d, const volatile u8 *e) __attribute__((alias("ObjCenter_03698")));
void sub_080036E0(int a, u32 b, u32 c, u32 d, const volatile u8 *e) __attribute__((alias("ObjCenter_036E0")));
void sub_08003728(int a, u32 b, u32 c, u32 d, const volatile u8 *e) __attribute__((alias("ObjCenter_03728")));
#endif

// ----------------------------------------------------------------------------
// Measured emitters 0x0800376C/37BC — itoa (0x03CB4, 16-byte buffer) then
// x -= digits * table[sel]; delegate with the buffer as the string.
//   0x0376C(sel, x, a2, a3, value, a5): -> 0x03350(sel, x', a2, a3, buf, a5)
//   0x037BC(sel, x, a2, a3, value, a5): -> 0x03400(sel, x', a2, a3, buf, 1, a5)
extern int Measure_03CB4(int val, volatile u8 *buf, int width);

// `table_at` is a DATA reference, not a call. The ROM at both entries is
// `ldr rX,[pc,#n]` (pool word 0x080C4BA0) / `lsls rY,r5,#2` / `adds` /
// `ldr rY,[rY]` -- there is no `bl` at all. `table_at` is a `static` one-liner
// that agbcc does NOT inline, so it emitted a real `bl table_at` and the probe
// reported UNRESOLVED_RELOCATION. Writing the pool load out, exactly as
// ObjCenter_0364C/03698/036E0/03728 above already do, resolves it.
#ifndef __APPLE__
#define RM_CALLEE(friendly, closure) closure
extern int sub_08003CB4(int val, volatile u8 *buf, int width);
extern void sub_08003350(int a, u32 b, u32 c, u32 d, const volatile u8 *e, u16 f);
extern void sub_08003400(int a, u32 b, u32 c, u32 d, const volatile u8 *e, u32 f, u32 g);
#else
#define RM_CALLEE(friendly, closure) friendly
#endif
void ObjMeasure_0376C(int sel, u32 x, u32 a2, u32 a3, int value, u32 a5)
{
    volatile u8 buf[16];
    uintptr_t base;
    u32 step;
    int digits;
    digits = RM_CALLEE(Measure_03CB4, sub_08003CB4)(value, buf, 16);
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    step = *(volatile u32 *)(uintptr_t)(base + (u32)sel * 4);
    x -= (u32)(digits * (int)step);
    RM_CALLEE(ObjList_03350, sub_08003350)(sel, x, a2, a3, buf, a5);
}
void ObjMeasure_037BC(int sel, u32 x, u32 a2, u32 a3, int value, u32 a5)
{
    volatile u8 buf[16];
    uintptr_t base;
    u32 step;
    int digits;
    digits = RM_CALLEE(Measure_03CB4, sub_08003CB4)(value, buf, 16);
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      base = (uintptr_t)HudLaneTbl; }
#else
    base = (uintptr_t)0x080C4BA0u;
#endif
    step = *(volatile u32 *)(uintptr_t)(base + (u32)sel * 4);
    x -= (u32)(digits * (int)step);
    RM_CALLEE(ObjList_03400, sub_08003400)(sel, x, a2, a3, buf, 1, a5);
}
#ifndef __APPLE__
void sub_0800376C(int a, u32 b, u32 c, u32 d, int e, u32 f) __attribute__((alias("ObjMeasure_0376C")));
void sub_080037BC(int a, u32 b, u32 c, u32 d, int e, u32 f) __attribute__((alias("ObjMeasure_037BC")));
#endif

// 0x08003810 — itoa(arg3) to a 16-byte buffer, then 0x03604(sel, x, a2, buf).
void ObjTwo_03810(int sel, u32 x, u32 a2, int value)
{
    volatile u8 buf[16];
    // ROM `bl 0x08003CB4` (asm/runtime_2aac.s). Prefer the `sub_` twin over the
    // bare `_` spelling: both are closure labels at 0x08003CB4, but only
    // `sub_08003CB4` is also an alias of a real body in this TU, so the call
    // reaches C. A bare `extern int _08003CB4(...)` with no definition
    // anywhere silently reaches the ROM body instead of C -- it lands on the
    // same address, so the byte oracle cannot see it and only the
    // export/call audits catch it. Unguarded this would be a
    // C89 implicit declaration on Apple, hence the call-site split.
#ifndef __APPLE__
    sub_08003CB4(value, buf, 16);
#else
    Measure_03CB4(value, buf, 16);
#endif
    // ROM `bl 0x08003604` (asm/runtime_2aac.s). The closure spelling is the
    // only one the slice can resolve; unguarded it is a silent C89 implicit
    // declaration on Apple, so the call site carries the __APPLE__ split.
#ifndef __APPLE__
    sub_08003604(sel, x, a2, buf);
#else
    ObjCenter_03604(sel, x, a2, buf);
#endif
}
#ifndef __APPLE__
void sub_08003810(int a, u32 b, u32 c, int d) __attribute__((alias("ObjTwo_03810")));
#endif

void ObjLane_03838(int a, u32 b, const volatile u8 *c)   { _08003350(0, a, b, 0, c, 1); }
void ObjLane_0385C(int a, u32 b, const volatile u8 *c)
{
    // The closure spelling on ARM; the host keeps the friendly name, which is
    // defined unconditionally here. An unguarded alias call is a silent C89
    // implicit declaration on Apple, and a host link with
    // `-undefined dynamic_lookup` binds lazily, so nothing reports it.
#ifndef __APPLE__
    sub_08003400(0, a, b, 0, c, 1, 1);
#else
    ObjList_03400(0, a, b, 0, c, 1, 1);
#endif
}
void _08003880(int a, u32 b, const volatile u8 *c, u32 d)
{
    /* asm passes 7 args (d lands in the dead 7th slot 0x080034B0 never reads):
       (sel=0, x=a, a2=b, a3=0, str=c@sp72, w5=1@sp76, d=dead@sp80) */
    _080034B0(0, a, b, 0, c, 1, d);
}
void ObjLane_038A4(int a, u32 b, const volatile u8 *c)   { _08003350(2, a, b, 0, c, 1); }
void ObjLane_038C8(int a, u32 b, const volatile u8 *c)   { _08003350(3, a, b, 0, c, 1); }
void ObjLane_038EC(int a, u32 b, const volatile u8 *c)   { sub_08003560(2, a, b, 0, c); }
void ObjLane_0390C(int a, u32 b, const volatile u8 *c)   { sub_08003560(3, a, b, 0, c); }
#ifndef __APPLE__
void sub_08003838(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_03838")));
void sub_0800385C(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_0385C")));
void sub_08003880(int a, u32 b, const volatile u8 *c, u32 d) __attribute__((alias("_08003880")));
void ObjLane_03880(int a, u32 b, const volatile u8 *c, u32 d) __attribute__((alias("_08003880")));
void sub_080038A4(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_038A4")));
void sub_080038C8(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_038C8")));
void sub_080038EC(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_038EC")));
void sub_0800390C(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_0390C")));
#endif

// 0x0800392C/3940 — 0x03604(sel=0/2, a, 0, b). The register shuffle in the
// ROM is `adds r2,r0; adds r3,r1; movs r0,#sel; adds r1,r2; movs r2,#0`
// (asm/runtime_2aac.s:1919-1949): the caller's r0 lands in a1 and its r1 in
// the STRING slot (r3), with a2 explicitly zeroed — this wrapper has no
// incoming x (it centres at 120). The previous `(sel, a, b, 0)` transposed the
// two, so every call read its string from address 0.
void ObjCenter_0392C(int a, u32 b)
{
#ifndef __APPLE__
    sub_08003604(0, a, 0, (const volatile u8 *)(uintptr_t)b);
#else
    ObjCenter_03604(0, a, 0, (const volatile u8 *)(uintptr_t)b);
#endif
}
void ObjCenter_03940(int a, u32 b)
{
#ifndef __APPLE__
    sub_08003604(2, a, 0, (const volatile u8 *)(uintptr_t)b);
#else
    ObjCenter_03604(2, a, 0, (const volatile u8 *)(uintptr_t)b);
#endif
}
#ifndef __APPLE__
void sub_0800392C(int a, u32 b) __attribute__((alias("ObjCenter_0392C")));
void sub_08003940(int a, u32 b) __attribute__((alias("ObjCenter_03940")));
// Declared HERE as well as in the alias block below: agbcc emits an
// alias's.globl/.thumb_set glue into whichever section is open where
// the declaration sits, so an alias 1000 lines from its body lands in a
// different section and the per-body splice never carries it.
void _08003940(int a, u32 b) __attribute__((alias("ObjCenter_03940")));
#endif

// 0x08003954–0x080039E4 — measured delegates (slots exactly as in asm).
void ObjMeasure_03954(int a, u32 b, int c) { sub_0800376C(0, a, b, 0, c, 1); }
__asm__(".align 2, 0");
void ObjMeasure_03978(int a, u32 b, int c) { sub_0800376C(2, a, b, 0, c, 1); }
__asm__(".align 2, 0");
void ObjMeasure_0399C(int a, u32 b, int c) { sub_080037BC(2, a, b, 0, c, 1); }
__asm__(".align 2, 0");
void ObjMeasure_039C0(int a, u32 b, int c) { sub_0800376C(3, a, b, 0, c, 1); }
__asm__(".align 2, 0");
void ObjMeasure_039E4(int a, u32 b, int c) { sub_0800376C(2, a, b, 0, c, 0); }
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_08003954(int a, u32 b, int c) __attribute__((alias("ObjMeasure_03954")));
// NOTE: the `_08003954` spelling is already defined further down this file. Do
// NOT add a second alias here — a duplicate makes agbcc reject the whole TU
// ("redefinition of `_08003954'"), which SKIPS this file and silently voids
// every body in it.
void sub_08003978(int a, u32 b, int c) __attribute__((alias("ObjMeasure_03978")));
void sub_0800399C(int a, u32 b, int c) __attribute__((alias("ObjMeasure_0399C")));
void sub_080039C0(int a, u32 b, int c) __attribute__((alias("ObjMeasure_039C0")));
void sub_080039E4(int a, u32 b, int c) __attribute__((alias("ObjMeasure_039E4")));
#endif

// 0x08003A08–0x08003AA8 — centering delegates.
void ObjCenter_03A08(int a, u32 b, const volatile u8 *c) { sub_0800364C(0, a, b, 0, c); }
__asm__(".align 2, 0");
void ObjCenter_03A28(int a, u32 b, const volatile u8 *c) { sub_0800364C(2, a, b, 0, c); }
__asm__(".align 2, 0");
void ObjCenter_03A48(int a, u32 b, const volatile u8 *c) { sub_0800364C(3, a, b, 0, c); }
__asm__(".align 2, 0");
void ObjCenter_03A68(int a, u32 b, const volatile u8 *c) { sub_08003698(3, a, b, 0, c); }
__asm__(".align 2, 0");
void ObjCenter_03A88(int a, u32 b, const volatile u8 *c) { sub_080036E0(3, a, b, 0, c); }
__asm__(".align 2, 0");
void ObjCenter_03AA8(int a, u32 b, const volatile u8 *c) { sub_08003728(3, a, b, 0, c); }
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_08003A08(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjCenter_03A08")));
void sub_08003A28(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjCenter_03A28")));
void sub_08003A48(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjCenter_03A48")));
void sub_08003A68(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjCenter_03A68")));
void sub_08003A88(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjCenter_03A88")));
void sub_08003AA8(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjCenter_03AA8")));
#endif

// 0x08003AC8/ADC — 0x03810(sel=0/2, a, 0, b)
void ObjTwo_03AC8(int a, int b)
{
#ifndef __APPLE__
    sub_08003810(0, a, 0, b);
#else
    ObjTwo_03810(0, a, 0, b);
#endif
}
void ObjTwo_03ADC(int a, int b)
{
#ifndef __APPLE__
    sub_08003810(2, a, 0, b);
#else
    ObjTwo_03810(2, a, 0, b);
#endif
}
#ifndef __APPLE__
void sub_08003AC8(int a, int b) __attribute__((alias("ObjTwo_03AC8")));
void sub_08003ADC(int a, int b) __attribute__((alias("ObjTwo_03ADC")));
void _08003ADC(int a, int b) __attribute__((alias("ObjTwo_03ADC")));
#endif

// 0x08003AF0 — request block from src: sel=src[8], x=s16[src+12], a2=s16[src+14],
// a3=src[10], str=*(const u8**)src, w5=1.
void ObjReq_03AF0(u8 *src)
{
    // s16 lanes as plain s16 reads — the ROM uses `ldrsh` (trap 5: a
    // volatile sub-word rval expands to ldrh+sign-extension instead).
    // Literal statement order matches the ROM's instruction order.
    u8 a0 = src[8];
    s32 x1 = *(s16 *)(src + 12);
    s32 x2 = *(s16 *)(src + 14);
    u8 a3 = src[10];
    const u32 *tab = (const u32 *)*(const u32 *)(uintptr_t)src;
    RM_CALLEE(ObjList_03350, sub_08003350)(a0, (u32)x1, (u32)x2, a3, (const volatile u8 *)tab, 1);
}
// Trap 6: content length 2 (mod 4); pad this function's section with `00 00`.
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_08003AF0(volatile u8 *a) __attribute__((alias("ObjReq_03AF0")));
#endif

// 0x08003B18 — advance: s16[0x080C49A0 + a*2] - 1. Plain s16 read: the
// ROM uses `ldrsh` (trap 5: volatile would expand to ldrh+sign-extension).
// The table base is a named symbol referenced before the shift so the pool
// load stays above it, as in the ROM.
int GlyphAdvance_03B18(int a)
{
    extern u8 GlyphAdvTbl[];
    u8 *tbl;
    __asm__(".globl GlyphAdvTbl\nGlyphAdvTbl = 0x080C49A0");
    tbl = GlyphAdvTbl;
    return *(s16 *)(tbl + ((u32)a << 1)) - 1;
}
// 0x08003B34 — first index i in 0..255 with s16[0x080C49A0 + i*2] == a+1, else 0
int GlyphIndex_03B34(int a)
{
    int want = a + 1;
    for (int i = 0; i <= 255; i++) {
        if (*(volatile s16 *)(uintptr_t)(0x080C49A0 + (u32)i * 2) == want)
            return i;
    }
    return 0;
}
#ifndef __APPLE__
int sub_08003B18(int a) __attribute__((alias("GlyphAdvance_03B18")));
int sub_08003B34(int a) __attribute__((alias("GlyphIndex_03B34")));
#endif

void ObjDigits_03B54(int a0, u32 a1, int val) { volatile u8 b[16]; sub_08003D4C(val, b); _080038A4(a0, a1, b); }
void ObjDigits_03B78(int a0, u32 a1, u32 val) { volatile u8 b[16]; sub_08003D4C(val, b); _08003838(a0, a1, b); }
void ObjDigits_03B9C(int a0, u32 a1, u32 val) { volatile u8 b[16]; sub_08003E34(val, b); _08003838(a0, a1, b); }
void ObjDigits_03BC0(int a0, u32 a1, int val) { volatile u8 b[16]; sub_08003D4C(val, b); sub_0800385C(a0, a1, b); }
// Call the ROM's own spelling, not the friendly alias: the corpus probe resolves
// R_ARM_THM_CALL against the ROM closure table, which has `_08003E34` and not
// `TimeStr_03E34`. The friendly name left the `bl` at +0x0E unresolved and cost
// its 4 bytes. Same convention as ObjDigits_03B9C above (`sub_08003E34`).
void ObjDigits_03BE4(int a0, u32 a1, int val, u32 a3) { volatile u8 b[16]; _08003E34(val, b); _08003880(a0, a1, b, a3); }
#ifndef __APPLE__
void sub_08003B54(int a, u32 b, int c) __attribute__((alias("ObjDigits_03B54")));
void sub_08003B78(int a, u32 b, u32 c) __attribute__((alias("ObjDigits_03B78")));
void _08003B78(int a, u32 b, u32 c) __attribute__((alias("ObjDigits_03B78")));
void sub_08003B9C(int a, u32 b, u32 c) __attribute__((alias("ObjDigits_03B9C")));
void _08003B9C(int a, u32 b, u32 c) __attribute__((alias("ObjDigits_03B9C")));
void sub_08003BC0(int a, u32 b, int c) __attribute__((alias("ObjDigits_03BC0")));
void sub_08003BE4(int a, u32 b, int c, u32 d) __attribute__((alias("ObjDigits_03BE4")));
#endif

// ----------------------------------------------------------------------------
// 0x08003C0C — right-aligned decimal.
// : the probe's UNRESOLVED_RELOCATION was a WRONG CALLEE,
// not an invented helper. The ROM's `bl` at +0x10 targets 0x08003D18
// (`TimeStr_03D18`, the right-to-left writer that ends at buf[15]), NOT
// 0x08003CB4 (`Measure_03CB4`), and it passes only TWO arguments: r0 = val,
// r1 = sp+8. The old three-argument `Measure_03CB4(val, base, 16)` call left
// the relocation dangling. The ROM frame is `sub sp,#24` with the 16-byte
// digit area at sp+8; declaring a plain `volatile u8 buf[16]` gives agbcc
// that exact frame, whereas `buf[24]` + `base = buf + 8` produced
// `sub sp,#32` and an extra callee-saved register (candidate 60 vs 84
// bytes). `buf + 15 - digits` is the ROM's
// `subs r0,#15; add r1,sp,#8; subs r0,r1,r0`.
void ObjDigitsRight_03C0C(int a0, u32 a1, int val)
{
    volatile u8 buf[16];
    uintptr_t tbl;
    int digits;
    digits = TimeStr_03D18(val, buf);
#ifndef __APPLE__
    { extern u8 HudLaneTbl[];
      __asm__(".globl HudLaneTbl\nHudLaneTbl = 0x080C4BA0\n");
      tbl = (uintptr_t)HudLaneTbl; }
#else
    tbl = (uintptr_t)0x080C4BA0u;
#endif
    a0 -= (int)(*(volatile u32 *)(uintptr_t)(tbl + 8) * digits);
    sub_08003350(2, (u32)a0, a1, 0, buf + 15 - digits, 1);
}
#ifndef __APPLE__
void sub_08003C0C(int a, u32 b, int c) __attribute__((alias("ObjDigitsRight_03C0C")));
#endif

void ObjFixed_03C58(int a0, u32 a1)
{
    volatile u8 buf[24];
    volatile u8 *base = buf + 8;
    base[0] = '0';
    base[1] = 0;
    a0 -= (int)table_at(8);
    ObjList_03350(2, (u32)a0, a1, 0, base, 1);
}
#ifndef __APPLE__
void sub_08003C58(int a, u32 b) __attribute__((alias("ObjFixed_03C58")));
#endif

// ----------------------------------------------------------------------------
// 0x08003C80 — writer: NUL at end+n, then n nibbles right-to-left, value
// shifted right 4 per digit. The high nibble arm is a BARE `adds r0,#65` on
// the unmasked nibble (ROM 0x08003CA0) — NOT 'A'+(nib-10), which would fold
// to `adds r0,#55`. So digits 10..15 store 75..80, and "hex" here is a
// misnomer. Do not "fix" this to 'A'..'F'; the ROM is the oracle.
// Returns void: the epilogue is `pop {r4}; pop {r0}; bx r0`, which clobbers
// r0 with the popped LR, so no value can survive it.
//
// Two locals below are pinned with GCC local register variables
// (`register T v __asm__("rN")`), a GNU extension. Each pin names a register
// the ROM actually uses and the instruction that forces it; agbcc's own
// allocation is the mirror image (mask in r4, value in r2), which costs every
// byte from +2 on. Each pin sits in its OWN declaration block: two pins in one
// block make tools/agbcc_c89_transform.py bail with "budget exhausted".
//   i  -> r3  `adds r3,r2,#0` (0x8003c84): a copy of n, decremented in place.
//   m  -> r2  `movs r2,#15` (0x8003c90, hoisted to the preheader) feeding the
//            single `ands r0,r2` (0x8003c96). Pinning it also keeps r2 out of
//            the allocator's early choices, which is what lets `end + n` keep
//            using the incoming r2 for `adds r1,r1,r2` (0x8003c86) instead of
//            CSE-ing to the r3 copy.
// Dropping either pin is measurable: without the r3 pin the body falls back to
// prefix 6 / 49 of 52, without the r2 pin to prefix 0x14 / 48 of 52. A third
// pin on `v -> r4` was tried and REMOVED: with the other two in place agbcc
// picks r4 for the shifted value by itself, and the emitted bytes are
// identical with and without it, so it is not load-bearing.
// `nib` is signed on purpose: the ROM branches on `bgt` (0x8003c9a), not the
// `bhi` an unsigned compare emits. 0..15 either way, so this is codegen only.
void HexWrite_03C80(int val, volatile u8 *end, int n)
{
    volatile u8 *p;
    u32 v = (u32)val;
    {
        register int i __asm__("r3") = n;
        p = end + n;
        *p = 0;
        // The `if` is load-bearing: it is the ROM's separate pre-loop guard
        // (`cmp r3,#0 / beq.n` at 0x8003c8c) with the loop itself bottom-tested.
        if (i != 0) {
            register u32 m __asm__("r2") = 0xF;
            do {
                p--;
                s32 nib = (s32)(v & m);
                *p = (u8)(nib <= 9 ? '0' + nib : 'A' + nib);
                v >>= 4;
            } while (--i != 0);
        }
    }
}
#ifndef __APPLE__
void sub_08003C80(int a, volatile u8 *b, int c) __attribute__((alias("HexWrite_03C80")));
#endif

// ----------------------------------------------------------------------------
// 0x08003CB4 — decimal itoa: '-' + negated value for negatives; digits
// counted by Div/10; left-to-right emit via DivRem; returns digit count.
// (Zero-padded tail reserved by width — asm reserves but does not pad.)
int Measure_03CB4(int val, volatile u8 *buf, int width)
{
    volatile u8 *p = buf;
    u32 digits = 0;
    u32 v;
    if (val < 0) {
        *p++ = '-';
        v = (u32)(-val);
        if (width > 0)
            width--;
    } else {
        v = (u32)val;
    }
    u32 t = v;
    do {
        t = (u32)Div((int)t, 10);
        digits++;
    } while (t > 0);
    if (width == 0)
        return (int)digits;
    u32 pow = 1;
    for (u32 i = 1; i < digits; i++)
        pow *= 10;
    for (u32 i = 0; i < digits; i++) {
        *p++ = (u8)('0' + DivRem((int)v, (int)pow) / (int)pow);
        pow /= 10;
    }
    *p = 0;
    return (int)digits;
}
#ifndef __APPLE__
int sub_08003CB4(int a, volatile u8 *b, int c) __attribute__((alias("Measure_03CB4")));
#endif

// ----------------------------------------------------------------------------
// 0x08003D18 — zero-padded itoa writing right-to-left ending at buf[15]
// (buf[15] = NUL first); returns digit count.
int TimeStr_03D18(int val, volatile u8 *buf)
{
    u32 digits = 0;
    volatile u8 *p = buf + 14;

    buf[15] = 0;
    {
        int v = val;
        do {
            *p = (u8)('0' + DivRem(v, 10));
            p--;
            v = Div(v, 10);
            digits++;
        } while (v > 0);
    }
    return (int)digits;
}
// The body is 50 bytes in a 4-aligned section (`.text.TimeStr_03D18` = 0x34).
// Under -ffunction-sections gas closes a Thumb code section with the 2-byte
// `nop` filler (0x46c0) where the ROM holds `00 00`. This file-scope
// `.align 2, 0` lands after the body's `.size`, i.e. still inside the body's
// own section, and pads with the explicit `0` fill instead. 50/52 -> EXACT.
__asm__(".align 2, 0");
#ifndef __APPLE__
int sub_08003D18(int a, volatile u8 *b) __attribute__((alias("TimeStr_03D18")));
int _08003D18(int a, volatile u8 *b) __attribute__((alias("TimeStr_03D18")));
#endif

// ----------------------------------------------------------------------------
// Time formatters 0x08003D4C (232 B) / 0x08003E34 (228 B) — INLINED per body.
// The former `static void time_fmt(...)` helper is deleted, not bypassed: agbcc
// emitted it once as a file-local `t` symbol the probe's closure table cannot
// resolve, so both wrappers read UNRESOLVED_RELOCATION and were unscorable.
// The ROM bodies are self-contained apart from `bl`s to real BIOS wrappers, so
// each body now spells its own stream out.
//
// What the disassembly actually says (the old shared helper got these wrong):
//  * Sign test is `ands r0,r6` against a 0x80000000 constant materialised as
//    `movs r0,#128; lsls r0,r0,#24` — i.e. `val & 0x80000000`, not `val < 0`.
//    That byte is live ACROSS the divide calls, so it lives in r8 (r9 for the
//    (val%300)/3 temp), which is what forces the `mov r7,r9 / mov r6,r8 /
//    push {r6,r7}` high-register prologue. It is finally stored at buf[8].
//  * Negative branch writes 8 chars buf[0..7] and a NUL at **buf[9]**
//    (`movs r0,#0; strb r0,[r7,#1]` with r7 = buf+8). It does NOT write
//    buf[8], and it is not a 9-byte copy from a table. Return value 0.
//  * Positive branch writes buf[0..8] and NO terminator. Return value is
//    r8 (`mov r0,r8; strb r0,[r7,#0]` is the last thing before the epilogue),
//    i.e. 0 or 0x80.
//  * `qa = Div(DivRem(val,300),3)` — DivRem leaves the REMAINDER in r0, so the
//    nesting divides (val%300), not (val/300). Same for 18000/300.
//  * Separators are literals, not a shared `sep` parameter: 0x3D4C writes 0x27
//    at buf[2] and 0x22 at buf[5]; 0x08003E34 writes '.' at both.
// Closure spellings, verified against `arm-none-eabi-nm build-code/code.o`:
//   0x0802D978 -> sub_0802D978 (Div),  0x0802D97C -> sub_0802D97C (DivRem).
//   The friendly `Div`/`DivRem` declared at the top of this file are NOT
//   symbols in code.o, so a call under those names would be a second
//   UNRESOLVED_RELOCATION.
int TimeStr_03D4C(int val, volatile u8 *buf)
{
    // Register assignment MEASURED: agbcc puts val in r7 and buf in r6 and emits
    // `adds r7,r0,#0; adds r6,r1,#0`, the reverse of the ROM's
    // `adds r6,r0,#0; adds r7,r1,#0` at +0x08. Pinning buf to r7 does NOT fix
    // it and costs 15 more bytes (35/232 -> 20/232, candidate 208 -> 252):
    // agbcc then reloads the parameter instead of keeping it in r6. The swap is
    // left as-is and recorded as the first difference.
    u8 *p = (u8 *)buf;
    register u32 flag __asm__("r8");
    register u32 qa __asm__("r9");
    u32 qb, qc;
    flag = 0x80000000u;
    flag = (u32)val & flag;
    if (flag) {
        *p++ = '-'; *p++ = '-'; *p++ = '\''; *p++ = '0';
        *p++ = '0'; *p++ = '"'; *p++ = '0'; *p = '0';
        p[1] = 0;
        return 0;
    }
    qa = (u32)sub_0802D978(sub_0802D97C(val, 300), 3);
    qb = (u32)sub_0802D978(sub_0802D97C(val, 18000), 300);
    qc = (u32)sub_0802D978(val, 18000);
    *p++ = (u8)('0' + sub_0802D978((int)qc, 10));
    *p++ = (u8)('0' + sub_0802D97C((int)qc, 10));
    *p++ = '\'';
    *p++ = (u8)('0' + sub_0802D978((int)qb, 10));
    *p++ = (u8)('0' + sub_0802D97C((int)qb, 10));
    *p++ = '"';
    *p++ = (u8)('0' + sub_0802D978((int)qa, 10));
    *p++ = (u8)('0' + sub_0802D97C((int)qa, 10));
    { u8 end = (u8)flag;
      *p = end;
      return (int)end; }
}
// ROM 0x08003E34 (228 B). Byte-verified against the ROM stream, not inferred:
// the NEGATIVE branch is  0x1A movs r1,#45 (0x2D '-')  0x24 movs r0,#39 (0x27
// '\'')  0x32 movs r0,#34 (0x22 '"')  0x3E movs r0,#0 + `strb r0,[r7,#1]`,
// which with p at buf+7 is buf[8]. So the eight stores are
//   - - ' - - " - -   with NUL at buf[8];
// and the POSITIVE branch's two separators are `movs r4,#36` = 0x24 '$'
// (0x8E), NOT 0x2E '.'. The sibling _08003D4C above was already corrected to
// '\'' / '"'; this body was left on '.' and on '0' placeholders.
// Measured  : the corrections COST score -- the
// smaller immediates fold into fewer instructions -- so this went 36 -> 19.
// ROM bytes win; the score is a proxy, the ROM is the oracle.
int _08003E34(int val, volatile u8 *buf)
{
    u8 *p = (u8 *)buf;
    register u32 flag __asm__("r8");
    register u32 qa __asm__("r9");
    u32 qb, qc;
    flag = 0x80000000u;
    flag = (u32)val & flag;
    if (flag) {
        *p++ = '-'; *p++ = '-'; *p++ = '\''; *p++ = '-';
        *p++ = '-'; *p++ = '"'; *p++ = '-'; *p = '-';
        p[1] = 0;
        return 0;
    }
    qa = (u32)sub_0802D978(sub_0802D97C(val, 300), 3);
    qb = (u32)sub_0802D978(sub_0802D97C(val, 18000), 300);
    qc = (u32)sub_0802D978(val, 18000);
    *p++ = (u8)('0' + sub_0802D978((int)qc, 10));
    *p++ = (u8)('0' + sub_0802D97C((int)qc, 10));
    *p++ = '$';
    *p++ = (u8)('0' + sub_0802D978((int)qb, 10));
    *p++ = (u8)('0' + sub_0802D97C((int)qb, 10));
    *p++ = '$';
    *p++ = (u8)('0' + sub_0802D978((int)qa, 10));
    *p++ = (u8)('0' + sub_0802D97C((int)qa, 10));
    { u8 end = (u8)flag;
      *p = end;
      return (int)end; }
}
#ifndef __APPLE__
int sub_08003D4C(int a, volatile u8 *b) __attribute__((alias("TimeStr_03D4C")));
int sub_08003E34(int a, volatile u8 *b) __attribute__((alias("_08003E34")));
int TimeStr_03E34(int a, volatile u8 *b) __attribute__((alias("_08003E34")));
#endif

// ----------------------------------------------------------------------------
// 0x08003F18 — indexed pair: len = strlen(str); emit str at (2, x); itoa(idx)
// into a 16-byte buffer; emit buf at (2, x + len*8).
void ObjIndexed_03F18(u32 x, u32 a1, const volatile u8 *str, int idx)
{
    u32 len = strlength_03148(str);
    ObjList_03350(2, x, a1, 0, str, 1);
    volatile u8 buf[16];
    Measure_03CB4(idx, buf, 16);
    ObjList_03350(2, x + len * 8, a1, 0, buf, 1);
}
#ifndef __APPLE__
void sub_08003F18(u32 a, u32 b, const volatile u8 *c, int d) __attribute__((alias("ObjIndexed_03F18")));
void _08003F18(u32 a, u32 b, const volatile u8 *c, int d) __attribute__((alias("ObjIndexed_03F18")));
#endif

// 0x08003F64 — hex: HexWrite(val, buf, n) then emit at (2, x).
void ObjHex_03F64(int a0, u32 a1, int val, int n)
{
    volatile u8 buf[16];
#ifndef __APPLE__
    sub_08003C80(val, buf, n);
#else
    HexWrite_03C80(val, buf, n);
#endif
    ObjLane_038A4(a0, a1, buf);
}
#ifndef __APPLE__
void sub_08003F64(int a, u32 b, int c, int d) __attribute__((alias("ObjHex_03F64")));
#endif

// ----------------------------------------------------------------------------
// 0x08003F88 — request-block entry writer with selectable tag:
//   tag = arg2 ? 0x8700 : 0x8300 (0x87/0x83 << 8)
//   [obj+0] = tag | f2; [obj+2] = (arg1 << 9) | f1;
//   [obj+4] = ((src[2] + 2*f3) | ((src[0] + f5) << 12))
void ObjAttr_03F88(volatile u8 *req, int a1, u32 sel)
{
    register volatile u8 *q __asm__("r3");
    register int b __asm__("r5");
    register volatile u8 *obj __asm__("r2");
    register s32 f3 __asm__("r1");
    register volatile u8 *src __asm__("r4");
    register u32 taghi __asm__("r6");
    register u32 tag __asm__("r0");
    register u32 w __asm__("r0");
    register u32 vhi __asm__("r0");
    register u16 s2 __asm__("r6");
    register u16 s0 __asm__("r4");
    u32 s;
    q = req;
    b = a1;
    s = (sel << 24) >> 24;
    obj = (volatile u8 *)(uintptr_t)*(volatile u32 *)(q + 0);
    f3 = *(volatile s32 *)(q + 12);
    if (f3 < 0)
        return;
    src = (volatile u8 *)(uintptr_t)*(volatile u32 *)(q + 16);
    taghi = s ? 0x8700u : 0x8300u;
    tag = taghi;
    __asm__ volatile ("" : "=r" (tag) : "0" (tag));
    tag |= *(volatile u16 *)(q + 8);
    *(volatile u16 *)(obj + 0) = (u16)tag;
    w = (u32)b << 9;
    w |= *(volatile u16 *)(q + 4);
    *(volatile u16 *)(obj + 2) = (u16)w;
    f3 <<= 1;
    s2 = *(volatile u16 *)(src + 2);
    f3 = s2 + f3;
    vhi = *(volatile u32 *)(q + 20);
    s0 = *(volatile u16 *)(src + 0);
    vhi = s0 + vhi;
    vhi <<= 12;
    *(volatile u16 *)(obj + 4) = (u16)((u32)f3 | vhi);
}
// Span tail pad: see the note above (trap 6 lever).
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_08003F88(volatile u8 *a, int b, u32 c) __attribute__((alias("ObjAttr_03F88")));
#endif

// ----------------------------------------------------------------------------
// Sprite manager — 0x03000178 {base, count}, 16-byte slots {s16 id, u16 used,
// u32 a, u32 b}.
// 0x080040BC — first slot with used == 0, else NULL.
// ROM reads the count once (`ldr r0,[r0,#4]`, kept in r3) and compares it
// signed (`bge`/`blt`), so the bound is a plain s32 local, not a volatile
// re-read per iteration.
volatile u8 *SpriteFind_040BC(void)
{
    u32 *t = (u32 *)(uintptr_t)0x03000178;
    volatile u8 *p = (volatile u8 *)(uintptr_t)t[0];
    s32 i;
    for (i = 0; i < (s32)t[1]; i++, p += 16) {
        if (*(volatile u16 *)(p + 2) == 0)
            return p;
    }
    return 0;
}
// 0x080040E4 — set {base, count} then 0x040F8.
void SpriteUpdate_040F8(void);
#ifndef __APPLE__
// The slice links no C object, so a promoted body may only call a spelling
// the assembly closure defines. The alias that defines it sits below.
void sub_080040F8(void);
#endif
void SpriteInit_040E4(u32 base, u32 count)
{
    volatile u32 *t = (volatile u32 *)(uintptr_t)0x03000178;
    t[0] = base;
    t[1] = count;
#ifndef __APPLE__
    sub_080040F8();
#else
    // The alias that defines `sub_080040F8` is ARM-only, so the host build
    // calls the friendly name. An undeclared call is a silent C89 implicit
    // declaration and a host link with `-undefined dynamic_lookup` binds lazily,
    // so nothing would report it.
    SpriteUpdate_040F8();
#endif
}
// 0x080040F8 — CpuSet one zero word to base (t[0]), ctrl
// ((count*4) & 0x1FFFFF) | (160<<19 = 0x05000000).
// `CpuSet` (swi 0x0B, veneer 0x0802D974), not `CpuFastSet` (0x0802D970): the
// ROM's `bl 0x0802d974` is the CpuSet wrapper, and 0x05xxxxxx is the 32-bit
// FILL control word, not a fast-set mode.
void SpriteUpdate_040F8(void)
{
    // `zero` first, then the `dst` local, is load-bearing twice over:
    // `u32 zero = 0` is what puts `movs r0,#0 / str r0,[sp]` at the TOP of the
    // ROM, and the separate `dst` local is what makes agbcc emit `ldr r1,[r0]`
    // at +0x0C and pass r1 straight through to the call. Without `dst` the
    // `t[0]` load sinks to the call site (+0x1C) and the body is 16 bytes out.
    u32 zero = 0;
    volatile u32 *t = (volatile u32 *)(uintptr_t)0x03000178;
    void *dst = (void *)(uintptr_t)t[0];
    u32 mode = ((t[1] << 2) & 0x001FFFFFu) | 0x05000000u;
    CpuSet(&zero, dst, mode);
}
#ifndef __APPLE__
volatile u8 *sub_080040BC(void) __attribute__((alias("SpriteFind_040BC")));
void sub_080040E4(u32 a, u32 b) __attribute__((alias("SpriteInit_040E4")));
void sub_080040F8(void) __attribute__((alias("SpriteUpdate_040F8")));
#endif

// 0x08004124 — for every slot with used != 0 and id == arg: used = 0.
void SpriteHideAll_04124(u16 id)
{
    volatile u32 *t = (volatile u32 *)(uintptr_t)0x03000178;
    volatile u8 *p = (volatile u8 *)(uintptr_t)t[0];
    s16 want = (s16)id;
    for (u32 i = 0; i < t[1]; i++, p += 16) {
        if (*(volatile u16 *)(p + 2) != 0 && *(volatile s16 *)p == want)
            *(volatile u16 *)(p + 2) = 0;
    }
}
#ifndef __APPLE__
void sub_08004124(u16 a) __attribute__((alias("SpriteHideAll_04124")));
#endif

// 0x08004158 — alloc: p = find; p.used = 1; p.id = arg0; p.a = arg1; p.b = arg2.
void SpriteAlloc_04158(u16 id, u32 a1, u32 a2)
{
    volatile u8 *p = SpriteFind_040BC();
    if (!p)
        return;
    *(volatile u16 *)(p + 2) = 1;
    *(volatile u16 *)p = id;
    *(volatile u32 *)(p + 4) = a1;
    *(volatile u32 *)(p + 8) = a2;
}
#ifndef __APPLE__
void sub_08004158(u16 a, u32 b, u32 c) __attribute__((alias("SpriteAlloc_04158")));
#endif

// 0x08004180 — free id: for each used slot with matching id, call the
// re0x0802DDCC (bx r1) with r0 = slot.b, r1 = slot.a.
void SpriteFree_04180(u16 id)
{
    volatile s32 *t = (volatile s32 *)(uintptr_t)0x03000178;
    volatile u8 *p = (volatile u8 *)(uintptr_t)t[0];
    for (s32 i = 0; i < t[1]; i++, p += 16) {
        if (*(volatile u16 *)(p + 2) != 0 && *(volatile s16 *)p == (s16)id) {
            typedef void (*rel_fn)(u32);
            ((rel_fn)(uintptr_t)*(volatile u32 *)(p + 4))
                (*(volatile u32 *)(p + 8));
        }
    }
}
#ifndef __APPLE__
void sub_08004180(u16 a) __attribute__((alias("SpriteFree_04180")));
#endif

// ----------------------------------------------------------------------------
// Camera family.
// 0x080041C8 — dst = 0x0203F6B0 {x, y}.
void CamCopy_041C8(volatile u32 *dst)
{
    volatile u32 *src = (volatile u32 *)(uintptr_t)0x0203F6B0;
    dst[0] = src[0];
    dst[1] = src[1];
}
// 0x080041D4 — return s16 0x0203F728.
s16 CamAngle_041D4(void) { return *(volatile s16 *)(uintptr_t)0x0203F728; }
// 0x080041E0 — s16(120 - (a0 * 200) / a1)
s16 CamY_041E0(int a0, int a1)
{
    return (s16)(120 - sub_0802DE04(a0 * 200, a1));
}
// 0x080041F8 — project: d = (a0,a1) - cam; rotate by s16 0x0203F728;
int CamProject_041F8(int a0, int a1, volatile u32 *dst)
{
    volatile u32 *cam = (volatile u32 *)(uintptr_t)0x0203F6B0;
    s32 pt[2] = { a0 - (s32)cam[0], a1 - (s32)cam[1] };
    _08005BA8(pt, *(s16 *)(uintptr_t)0x0203F728);
    if (pt[1] == 0)
        return 0;
    { s32 y = pt[1];
      dst[0] = (u32)(120 - sub_0802DE04(pt[0] * 200, y));
      dst[1] = (u32)y; }
    return 1;
}
__asm__(".align 2, 0");
// 0x0800424C — s16((a0 * 200) / a1)
s16 CamY_0424C(int a0, int a1)
{
#ifndef __APPLE__
    return (s16)sub_0802DE04(a0 * 200, a1);
#else
    return (s16)Div(a0 * 200, a1);
#endif
}
#ifndef __APPLE__
void sub_080041C8(volatile u32 *a) __attribute__((alias("CamCopy_041C8")));
s16 sub_080041D4(void) __attribute__((alias("CamAngle_041D4")));
s16 sub_080041E0(int a, int b) __attribute__((alias("CamY_041E0")));
int sub_080041F8(int a, int b, volatile u32 *c) __attribute__((alias("CamProject_041F8")));
s16 sub_0800424C(int a, int b) __attribute__((alias("CamY_0424C")));
#endif

// 0x08004260 — init: 0x0203F6C0 = {20,32,-20,32}; rotate both pairs by arg2;
// cam = {arg0, arg1}; u16 0x0203F728 = (0x800 - (s16)arg2) & 0xFFF.
void CamInit_04260(int a0, int a1, int ang)
{
    volatile u32 *tbl = (volatile u32 *)(uintptr_t)0x0203F6C0;
    tbl[0] = 20;
    tbl[1] = 32;
    tbl[2] = (u32)-20;
    tbl[3] = 32;
    s16 a = (s16)ang;
    MathRot((void *)tbl, a);
    MathRot((void *)(tbl + 2), a);
    volatile u32 *cam = (volatile u32 *)(uintptr_t)0x0203F6B0;
    cam[0] = (u32)a0;
    cam[1] = (u32)a1;
    *(volatile u16 *)(uintptr_t)0x0203F728 =
        (u16)(((0x800 - (u32)(u16)a) & 0xFFF) & 0xFFFF);
}
#ifndef __APPLE__
void sub_08004260(int a, int b, int c) __attribute__((alias("CamInit_04260")));
void _08004260(int a, int b, int c) __attribute__((alias("CamInit_04260")));
#endif

// ----------------------------------------------------------------------------
// 0x080042C4 — in-sector test: out = {dx, dy} where d = arg - cam; |dx|,
// |dy| must be <= 0x20000; cross = cell0*dy - cell1*dx must be <= 0 and
// dot = cell2*dy - cell3*dx >= 0 (cell = 0x0203F6C0). Returns 1 inside.
int CamSector_042C4(int a0, int a1, volatile u32 *out)
{
    volatile u32 *cam = (volatile u32 *)(uintptr_t)0x0203F6B0;
    volatile u32 *cell = (volatile u32 *)(uintptr_t)0x0203F6C0;
    s32 dx = a0 - (s32)cam[0];
    out[0] = (u32)dx;
    if (MathAbs(dx) > 0x20000)
        return 0;
    s32 dy = a1 - (s32)cam[1];
    out[1] = (u32)dy;
    if (MathAbs(dy) > 0x20000)
        return 0;
    s32 cross = (s32)cell[0] * dy - (s32)cell[1] * dx;
    if (cross > 0)
        return 0;
    s32 dot = (s32)cell[2] * dy - (s32)cell[3] * dx;
    if (dot < 0)
        return 0;
    return 1;
}
#ifndef __APPLE__
int sub_080042C4(int a, int b, volatile u32 *c) __attribute__((alias("CamSector_042C4")));
#endif

// 0x08004324 — *(u32*)0x03000180 = a0. A 4-byte leaf: `ldr r1,[pc,#4];
// str r0,[r1]`, pool word 0x03000180 at 0x0800432C, then `bx lr` + 2 pad.
// `src/race_scene_a2.c:130` calls it as `_08004324(0x03005760)`, so it is a
// real callable entry and the transcription is right.
//
// A 4-byte entry with no prologue is a legitimate tiny leaf, not evidence of a
// tail. "No push" and "not a function" are different claims.
void CamSet_04324(int a0) { *(volatile u32 *)(uintptr_t)0x03000180 = (u32)a0; }
#ifndef __APPLE__
void sub_08004324(int a) __attribute__((alias("CamSet_04324")));
void _08004324(int a) __attribute__((alias("CamSet_04324")));
#endif

// ----------------------------------------------------------------------------
// Track-strip projections 0x08004344/3B8/440/4C4/508/4760/4818 — shared gate
// shape: CamSector, rotate the delta by s16 0x0203F728, then a strip lookup
// with the probe range check. Only the strip table / glyph stride / probe
// bias / divisor differ per variant (all constants transcribed from pools).
//   result record writes: [out+36] s32, [out+40] s32, [out+44] s32, [out+60] ptr
// ROM-faithful trio (0x043B8/0x04440 — 1-arg: the garage/race callers preload
// only r0; the projection coords come from the record's own [rec+4]/[rec+8],
// which 0x080275B8 fills from its a0 just before the call):
//   gate: CamSector(rec+4, rec+8 -> pt), rotate by s16 0x0203F728,
//         probe = pt[1] + 0xFFFFDD9F, reject if probe >u 0x1549E
//   writes: [rec+44]=probe>>6 (arithmetic), [rec+36]=120 - Div(200*pt[0], probe),
//           [rec+40]=Div(0xED800, probe) - (s16[glyph]-0x30), [rec+60]=glyph cell
// 0x043B8: strip 0x0802E260, cell stride 12, glyph halfword at cell+4
// 0x04440: strip 0x0802FA60, cell stride 8,  glyph halfword at cell+2
// 0x04508: in-record variant, NO CamSector/rotate gate (always returns 1):
//          row = ([rec+4]>>8)*10 into strip 0x08030A60, glyph at cell+6 ->
//          [rec+16], [rec+12]=32, [rec+8]=[rec+4]>>6, [rec+0]=120-
//          Div(200*[rec+0],[rec+4]), [rec+4]=Div(0xED800,[rec+4])-(glyph-0x30)
// 0x04344: legacy 4-arg strip_digit model kept (entry VMA is an interior
//          address, prologue not yet dumped; no ROM or C callers).
// 0x044C4: strip 0x080C53E4 gate only (bias 0xFFFFEBAF, limit 0x1EBAE);
//          on CamSector failure writes {dx, dy} to out[0..1] and returns 0.
// 0x04760/0x04818: strip 0x080C53E4 / 0x080C5FE4, cam at 0x03000188,
//          |d| gate 192<<9, rotation angle (0x800 - u16[cam+8]) & 0xFFF.
// Shared 0x043B8/0x04440 shape, INLINED per body (ROM 0x080043B8-0x0800443C /
// 0x08004440-0x080044C0). The former `static strip_glyph` was the same defect
// class as `bg_rot`: a local `static` helper that agbcc emits once and that the
// probe cannot resolve (R_ARM_THM_CALL against a `t`-local `strip_glyph`), so
// both wrappers read UNRESOLVED_RELOCATION and were unscorable. The ROM bodies
// carry the helper's logic inline -- the only `bl`s are to real ROM functions
// (0x080042C4, 0x08005BA8, 0x0802DE04), so the C now spells them out per body.
//
// Instruction constraints for 0x043B8 and 0x04440, identical
// structure, 4 bytes over in code: the ROM reads both the rotation angle
// (0x0203F728) and the per-cell glyph halfword through the REGISTER-offset
// Thumb form -- `movs r2,#0; ldrsh r1,[r0,r2]` and `movs r2,#4 (resp #2);
// ldrsh r1,[rN,r2]` -- where agbcc expands a constant byte offset to
// `ldrh; lsls #16; asrs #16` (6 bytes each instead of 4). That +4 plus a
// -2 from the predicate at +0x10 (see the long note inside TrackDigit_043B8:
// `(ret << 24) == 0` IS emitted correctly and is required) is the whole 136 B.
// Neither half can be applied alone; see that note for what was measured.
//
// Closure spellings, verified against `arm-none-eabi-nm build-code/code.o`:
//   bl 0x080042C4 -> sub_080042C4   (NOT `CamSector_042C4`: no such symbol)
//   bl 0x08005BA8 -> _08005BA8      (VMA spelling, same convention as below)
//   bl 0x0802DE04 -> sub_0802DE04   (signed EABI idiv, not the swi Div wrapper)
// All three are file-local (`t`) text symbols in code.o; whether the probe's
// closure table sees them is reported per body below.
//
// Two facts the shared helper had wrong, both read off the disassembly:
//  1. the divisor of BOTH idivs is the raw rotated pt[1] (r5), not the
//     `probe = pt[1] + 0xFFFFDD9F` the old helper divided by -- `adds r1,r5,#0`
//     at 0x080043FC / 0x0800440C reloads r5 itself. The +0xFFFFDD9F offset is
//     used ONLY by the unsigned range guard (`adds r1,r5,r0; cmp r1,r0; bhi`).
//  2. `rec[11] = pt[1] >> 6` and the cell index `pt[1] >> 8` also come from r5,
//     i.e. from the raw value, not the offset one.
int TrackDigit_043B8(volatile u32 *rec)
{
    s32 pt[2];
    // (1) The 4-byte overshoot was `volatile`, not the register-offset form.
    //     agbcc expands a `volatile s16` read at a CONSTANT byte offset into
    //     `ldrh; lsls #16; asrs #16` (6 bytes) where the ROM has the cheap
    //     4-byte `movs rX,#imm; ldrsh rY,[rn,rX]`. A plain `const s16 *` read
    //     yields the single sign-extending load with no offset juggling at all:
    //     the two reads are now `*(const s16 *)(uintptr_t)0x0203F728` and
    //     `*(const s16 *)(cell + 4)`. 140 -> 136 and 136 -> 132 bytes, both
    //     OVERSIZED -> PARTIAL. Removal control: restoring `volatile` returns
    //     both to 140/136 and OVERSIZED. Load-bearing.
//
    // (2) The predicate. The ROM spells the test `lsls r0,r0,#24; cmp r0,#0`
    //     (4 bytes) on the `sub_080042C4` result, where `!camret` folds to a
    //     bare `cmp r0,#0` (2 bytes). `s32 camret =...; if ((camret << 24)
    //     == 0)` forces the shift to survive.
    s32 camret = sub_080042C4((int)rec[1], (int)rec[2], (volatile u32 *)pt);
    if ((camret << 24) == 0)
        return 0;
    _08005BA8(pt, *(const s16 *)(uintptr_t)0x0203F728);
    s32 v = pt[1];
    if ((u32)(v + (s32)0xFFFFDD9F) > 0x0001549Eu)
        return 0;
    rec[11] = (u32)(v >> 6);
    extern u8 CellTbl03B8[];
    __asm__("CellTbl03B8 = 0x0802E260");
    u32 cellidx = (u32)(v >> 8) * 12;
    const u8 *cell = (const u8 *)(uintptr_t)((u32)(uintptr_t)CellTbl03B8 + cellidx);
    rec[9] = (u32)(120 - sub_0802DE04(pt[0] * 200, v));
    {
        s32 d = sub_0802DE04(0x000ED800, v);
        s32 glyph = *(const s16 *)(cell + 4);
        glyph -= '0';
        rec[10] = (u32)(d - glyph);
    }
    rec[15] = (u32)(uintptr_t)cell;
    return 1;
}
// Span tail: gas closes the Thumb section with the 2-byte nop filler `c0 46`
// where the ROM holds `00 00`. An explicit `.align 2, 0` in the still-open
// section pads with `0` instead. No instruction change.
__asm__(".align 2, 0");
int TrackDigit_04440(volatile u32 *rec)
{
    s32 pt[2];
    s32 camret = sub_080042C4((int)rec[1], (int)rec[2], (volatile u32 *)pt);
    if ((camret << 24) == 0)
        return 0;
    _08005BA8(pt, *(const s16 *)(uintptr_t)0x0203F728);
    s32 v = pt[1];
    if ((u32)(v + (s32)0xFFFFDD9F) > 0x0001549Eu)
        return 0;
    rec[11] = (u32)(v >> 6);
    extern u8 CellTbl04440[];
    __asm__("CellTbl04440 = 0x0802FA60");
    u32 cellidx = (u32)(v >> 8) * 8;
    const u8 *cell = (const u8 *)(uintptr_t)((u32)(uintptr_t)CellTbl04440 + cellidx);
    rec[9] = (u32)(120 - sub_0802DE04(pt[0] * 200, v));
    {
        s32 d = sub_0802DE04(0x000ED800, v);
        s32 glyph = *(const s16 *)(cell + 2);
        glyph -= '0';
        rec[10] = (u32)(d - glyph);
    }
    rec[15] = (u32)(uintptr_t)cell;
    return 1;
}
// Span tail: same `.align 2, 0` filler as TrackDigit_043B8 above.
__asm__(".align 2, 0");
int TrackDigit_04508(volatile u32 *rec)
{
    s32 y = (s32)rec[1];                                           // +4
    const u8 *cell = (const u8 *)(uintptr_t)(0x08030A60 + (u32)((y >> 8) * 10));
    rec[4] = (u32)*(volatile s16 *)(cell + 6);                     // +16
    rec[3] = 32u;                                                  // +12
    rec[2] = (u32)(y >> 6);                                        // +8
    rec[0] = (u32)(120 - Div(200 * (s32)rec[0], y));               // +0
    rec[1] = (u32)(Div(0x000ED800, y) - (*(volatile s16 *)(cell + 4) - '0')); // +4
    rec[11] = (u32)(uintptr_t)cell;                                // +44
    return 1;
}
int TrackArrow_044C4(int a0, int a1, volatile u32 *out)
{
    s16 *ap;
    if (!((u8)sub_080042C4(a0, a1, out))) {
        out[0] = 0;
        out[1] = 0;
        return 0;
    }
    ap = (s16 *)(uintptr_t)0x0203F728;
    _08005BA8((void *)out, *ap);
    s32 probe = (s32)out[1] + (s32)0xFFFFEBAF;
    if ((u32)probe > 0x0001EBAE)
        return 0;
    return 1;
}
// 0x04760/0x04818 — one body in ROM (0x04818 = push preamble falling into
// 0x04760's register setup); both VMAs exported.
static int track_proj(volatile u32 *out, s32 sx, s32 sy, s32 sz, uintptr_t strip)
{
    volatile u32 *cam = (volatile u32 *)(uintptr_t)0x03000188;
    s32 dx = sx - (s32)cam[0];
    if (MathAbs(dx) > (192 << 9))
        return 0;
    s32 dy = sy - (s32)cam[1];
    if (MathAbs(dy) > (192 << 9))
        return 0;
    s32 ang = (s32)(0x800 - *(volatile u16 *)((u8 *)cam + 8)) & 0xFFF;
    s32 pt[2] = { dx, dy };
    MathRot(pt, ang);
    s32 u = pt[1] + (s32)0xFFFFE890;
    if ((u32)u > 0x00015BA8)
        return 0;
    s32 v = pt[0];
    // cross gates exactly as written in the asm (u' = u + 2*v as loaded)
    s32 up = u + 2 * v;
    if (Div(-(60 * up), u) + 0 < 0)
        return 0;
    if (-(60 * up) > 0)
        return 0;
    s32 col = u >> 8;
    const u8 *rec = (const u8 *)(uintptr_t)(strip + (u32)((col * 2 + col) * 2));
    *(volatile u16 *)((u8 *)out + 54) = *(volatile u16 *)rec;
    *(volatile s32 *)((u8 *)out + 48) = *(volatile s16 *)(rec + 2);
    *(volatile s32 *)((u8 *)out + 44) = col;
    *(volatile s32 *)((u8 *)out + 36) = 120 + Div(-(60 * up), col);
    *(volatile s32 *)((u8 *)out + 40) = *(volatile s16 *)(rec + 4);
    (void)sz;
    return 1;
}
int TrackProj_04760(volatile u32 *out, int a0, int a1, int a2)
{
    return track_proj(out, a0, a1, a2, 0x080C53E4);
}
int TrackProj_04818(volatile u32 *out, volatile u32 *src)
{
    return track_proj(out, (s32)src[0], (s32)src[1], (s32)src[2], 0x080C5FE4);
}
#ifndef __APPLE__
int sub_080043B8(volatile u32 *a) __attribute__((alias("TrackDigit_043B8")));
int _080043B8(volatile u32 *a) __attribute__((alias("TrackDigit_043B8")));
int sub_08004440(volatile u32 *a) __attribute__((alias("TrackDigit_04440")));
int _08004440(volatile u32 *a) __attribute__((alias("TrackDigit_04440")));
int sub_08004508(volatile u32 *a) __attribute__((alias("TrackDigit_04508")));
int _08004508(volatile u32 *a) __attribute__((alias("TrackDigit_04508")));
int sub_080044C4(int a, int b, volatile u32 *c) __attribute__((alias("TrackArrow_044C4")));
int _080044C4(int a, int b, volatile u32 *c) __attribute__((alias("TrackArrow_044C4")));
int sub_08004760(volatile u32 *a, int b, int c, int d) __attribute__((alias("TrackProj_04760")));
int sub_08004818(volatile u32 *a, volatile u32 *b) __attribute__((alias("TrackProj_04818")));
#endif

// ----------------------------------------------------------------------------
// BG rotation writers 0x08004564/45B8/4610/4670 — 107-entry 12-byte records
// at 0x08033260; ctrl = *(u32*)0x03000180 {x, y, u16 angle, ptr}; dest =
// ptr + 208*4.
//
// Per-body index and store order, read off the asm:
//   4564: idx = (angle<<20)>>23;            d0=rec0+x, d1=y-rec1, hw0=rec2, hw4=rec3
//   45B8: idx = (angle<<20)>>23 - 128;      d0=rec1+x, d1=rec0+y, hw0=-rec3, hw4=rec2
//   4610: idx = (angle<<20)>>23 + 0xFFFFFF00;  d0=x-rec0, d1=rec1+y, hw0=-rec2, hw4=-rec3
//   4670: idx = (angle<<20)>>23 + 0xFFFFFE80;  d0=rec1-x, d1=rec0-y, hw0=rec3, hw4=-rec2
//
// Record address = 0x08033260 + idx*1296. The asm's multiply chain is
//   lsls r0,r1,#2; adds r0,r0,r1; lsls r0,r0,#4; adds r0,r0,r1; lsls r0,r0,#4
// = 4 -> 5 -> 80 -> 81 -> 1296, i.e. 1296 = 108 * 12, exactly one frame of the
// 108 twelve-byte records the 107..0 countdown walks. (The stride constant was
// 85 here before; 85 matches nothing in the instruction stream.)
void BgRot_04564(void)
{
    volatile u32 *ctrl = (volatile u32 *)(uintptr_t)0x03000180;
    volatile u32 *src = (volatile u32 *)(uintptr_t)*ctrl;
    s32 x = (s32)src[0];
    s32 y = (s32)src[1];
    u32 angle = *(volatile u16 *)((u8 *)src + 8);
    s32 idx = (s32)((angle << 20) >> 23);
    s32 dest = (s32)src[3] + (208 << 2);
    extern u8 RSB[] __asm__("RSB");
    const u8 *rec;
    int n;
    __asm__(".globl RSB\nRSB = 0x08033260\n");
    rec = (const u8 *)(RSB + (u32)(idx * 1296));
    n = 107;
    do {
        volatile u8 *d = (volatile u8 *)(uintptr_t)dest;
        volatile u32 *dw = (volatile u32 *)(uintptr_t)dest;
        dw[2] = (u32)(x + *(volatile s32 *)(rec + 0));
        dw[3] = (u32)(y - *(volatile s32 *)(rec + 4));
        *(volatile u16 *)(d + 0) = *(volatile u16 *)(rec + 8);
        *(volatile u16 *)(d + 4) = *(volatile u16 *)(rec + 10);
        dest += 16;
        rec += 12;
        n--;
    } while (n >= 0);
}
void BgRot_045B8(void)
{
    volatile u32 *ctrl = (volatile u32 *)(uintptr_t)0x03000180;
    volatile u32 *src = (volatile u32 *)(uintptr_t)*ctrl;
    s32 x = (s32)src[0];
    s32 y = (s32)src[1];
    u32 angle = *(volatile u16 *)((u8 *)src + 8);
    s32 idx = (s32)((angle << 20) >> 23);
    s32 dest;
    extern u8 RSB045B8[] __asm__("RSB045B8");
    const u8 *rec;
    int n;
    __asm__(".globl RSB045B8\nRSB045B8 = 0x08033260\n");
    idx -= 128;
    dest = (s32)src[3] + (208 << 2);
    rec = (const u8 *)(RSB045B8 + (u32)(idx * 1296));
    n = 107;
    do {
        volatile u8 *d = (volatile u8 *)(uintptr_t)dest;
        volatile u32 *dw = (volatile u32 *)(uintptr_t)dest;
        dw[2] = (u32)(x + *(volatile s32 *)(rec + 4));
        dw[3] = (u32)(y + *(volatile s32 *)(rec + 0));
        register u32 negv __asm__("r6") = *(volatile u16 *)(rec + 10);
        *(volatile u16 *)(d + 0) = (u16)(-(s16)negv);
        *(volatile u16 *)(d + 4) = *(volatile u16 *)(rec + 8);
        dest += 16;
        rec += 12;
        n--;
    } while (n >= 0);
}
void BgRot_04610(void)
{
    volatile u32 *ctrl = (volatile u32 *)(uintptr_t)0x03000180;
    volatile u32 *src = (volatile u32 *)(uintptr_t)*ctrl;
    s32 x = (s32)src[0];
    s32 y = (s32)src[1];
    u32 angle = *(volatile u16 *)((u8 *)src + 8);
    s32 idx = (s32)((angle << 20) >> 23);
    s32 dest;
    extern u8 RSB04610[] __asm__("RSB04610");
    const u8 *rec;
    int n;
    __asm__(".globl RSB04610\nRSB04610 = 0x08033260\n");
    idx += (s32)0xFFFFFF00;
    dest = (s32)src[3] + (208 << 2);
    rec = (const u8 *)(RSB04610 + (u32)(idx * 1296));
    n = 107;
    do {
        volatile u8 *d = (volatile u8 *)(uintptr_t)dest;
        volatile u32 *dw = (volatile u32 *)(uintptr_t)dest;
        dw[2] = (u32)(x - *(volatile s32 *)(rec + 0));
        dw[3] = (u32)(y + *(volatile s32 *)(rec + 4));
        register u32 negv __asm__("r6") = *(volatile u16 *)(rec + 8);
        *(volatile u16 *)(d + 0) = (u16)(-(s16)negv);
        *(volatile u16 *)(d + 4) = (u16)(-(s16)*(volatile u16 *)(rec + 10));
        dest += 16;
        rec += 12;
        n--;
    } while (n >= 0);
}
void BgRot_04670(void)
{
    volatile u32 *ctrl = (volatile u32 *)(uintptr_t)0x03000180;
    volatile u32 *src = (volatile u32 *)(uintptr_t)*ctrl;
    s32 x = (s32)src[0];
    s32 y = (s32)src[1];
    u32 angle = *(volatile u16 *)((u8 *)src + 8);
    s32 idx = (s32)((angle << 20) >> 23);
    s32 dest;
    extern u8 RSB04670[] __asm__("RSB04670");
    const u8 *rec;
    int n;
    __asm__(".globl RSB04670\nRSB04670 = 0x08033260\n");
    idx += (s32)0xFFFFFE80;
    dest = (s32)src[3] + (208 << 2);
    rec = (const u8 *)(RSB04670 + (u32)(idx * 1296));
    n = 107;
    do {
        volatile u8 *d = (volatile u8 *)(uintptr_t)dest;
        volatile u32 *dw = (volatile u32 *)(uintptr_t)dest;
        dw[2] = (u32)(x - *(volatile s32 *)(rec + 4));
        dw[3] = (u32)(y - *(volatile s32 *)(rec + 0));
        *(volatile u16 *)(d + 0) = *(volatile u16 *)(rec + 10);
        register u32 negv __asm__("r6") = *(volatile u16 *)(rec + 8);
        *(volatile u16 *)(d + 4) = (u16)(-(s16)negv);
        dest += 16;
        rec += 12;
        n--;
    } while (n >= 0);
}
#ifndef __APPLE__
void sub_08004564(void) __attribute__((alias("BgRot_04564")));
void sub_080045B8(void) __attribute__((alias("BgRot_045B8")));
void sub_08004610(void) __attribute__((alias("BgRot_04610")));
void sub_08004670(void) __attribute__((alias("BgRot_04670")));
// The closure names these four addresses with BOTH spellings
// (`_08004564` and `sub_08004564`, asm/runtime_2aac.s:3547-3549, 3637-3639).
// Promoting 0x080046D0 needs the underscore form to resolve to the C body as
// well, and it must be an ALIAS, not a bare `extern` declaration: a bare
// declaration makes the call bind to the ROM body instead of C. The
// byte oracle cannot see that -- the call lands on the same address and the
// 188,760-byte comparison still matches -- so this alias is what keeps the
// call inside C.
void _08004564(void) __attribute__((alias("BgRot_04564")));
void _080045B8(void) __attribute__((alias("BgRot_045B8")));
void _08004610(void) __attribute__((alias("BgRot_04610")));
void _08004670(void) __attribute__((alias("BgRot_04670")));
#endif

// 0x080046D0 — strh 52 at [cam+10]; dispatch on (s16[cam+8]>>10)&3; then
// zero halfwords from 0x0203F700+34 down to 0x0203F700 and 0x0203F724 = 0.
// Three lifts were wrong against the ROM and are corrected here:
//  * the strh lands at +10, not +0 (0x080046D8 `strh r0, [r1, #10]`);
//  * the dispatch index is `s16 >> 10`, which agbcc renders as the 16-bit
//    shift idiom `lsls #16` / `asrs #26` at 0x080046DC/0x080046DE. `angle *
//    64 >> 26` on an s32 folds to `asrs #31` and puts the load in r0;
//  * the clear loop compares as a *signed* integer (`cmp r0,r1` / `bge`,
//    0x08004656/0x08004658). Comparing the u16 pointers makes agbcc rewrite
//    the bound to base-1 and branch `bhi`, which also changes the pool.
// One register pin below (a GCC local register variable, a GNU extension; see
// the DECISION note in docs/matching_workflow.md) takes this from 85/108 to
// 96/108.
void BgRotDispatch_046D0(void)
{
    volatile u32 *ctrl = (volatile u32 *)(uintptr_t)0x03000180;
    volatile u8 *src = (volatile u8 *)(uintptr_t)*ctrl;
    // The dispatch index needs the LOAD in r1 and the shift temp in r0, so it
    // takes two pins in two different declaration blocks. Pinning `angle` to
    // r1 alone makes the shift destructive; pinning the sign-extended copy
    // `mid` to r0 forces the two apart and yields the ROM's non-destructive
    // `lsls r0,r1,#16` / `asrs r1,r0,#26`. See the header note.
    register s16 angle __asm__("r1");

    *(volatile u16 *)(src + 10) = 52;
    angle = *(volatile s16 *)(src + 8);
    register s32 mid __asm__("r0") = (s32)angle << 16;
    switch ((mid >> 26) & 3) {
#ifndef __APPLE__
    // Promotion rule 6, the shape this file already uses at the top: the
    // closure spells these four addresses `_08004564`/`sub_08004564` etc
    // (asm/runtime_2aac.s:3547-3549, 3637-3639, 3686-3688), so a promoted body
    // must CALL that spelling -- the splice can only export a name agbcc
    // wrote, and the friendly `BgRot_*` name is one it does not write, giving
    // "undefined reference to `BgRot_04564'" at link. Under __APPLE__ the
    // `_` aliases do not exist, so the host build calls the friendly name.
    // The split has to be here at the CALL site, not only at the declaration:
    // an unguarded call to an `__APPLE__`-absent alias is a host-build hole
    // that neither the agbcc probe nor the byte oracle can see.
    case 0: _08004564(); break;
    case 1: _080045B8(); break;
    case 2: _08004610(); break;
    case 3: _08004670(); break;
#else
    case 0: BgRot_04564(); break;
    case 1: BgRot_045B8(); break;
    case 2: BgRot_04610(); break;
    case 3: BgRot_04670(); break;
#endif
    }
    {
    // `hi` is declared BEFORE `lo`, and that ORDER is load-bearing: it decides
    // which address constant agbcc materialises first, and the ROM loads
    // 0x0203F724 into r3 at 0x08004718 BEFORE 0x0203F700 into r1 at
    // 0x0800471A, with the pool words in that same order at 0x08004734/38.
    // Declared second, `lo` wins the race and both the two `ldr`s and the two
    // pool words swap (104/108, first diff +0x49).
    register volatile u16 *hi __asm__("r3") = (volatile u16 *)(uintptr_t)0x0203F724;
    volatile u16 *lo = (volatile u16 *)(uintptr_t)0x0203F700;
    // Pinned to r3 because agbcc rematerialises a single-use address constant:
    // unpinned it sinks the 0x0203F724 load to the store at the end of the
    // function instead of holding it live in r3 across the clear loop for its
    // one use (0x0800472C `strh r0,[r3]`). The pin reaches the ROM's shape
    // honestly. The alternative lever found -- giving `hi` a SECOND use
    // by duplicating the `*hi = 0` store, 91/108 -- is a fidelity lie, since
    // the ROM has one use. Forcing a definition point with an assignment
    // expression does not work either: agbcc still sinks the load to the use.
    // Hoisting the clear value into a named u16 is load-bearing too: it gives
    // the `movs r2,#0` a definition point BEFORE the loop, which is where the
    // ROM has it (0x0800471C). As the literal `0` in the store, agbcc sinks
    // the materialisation past `adds r0,r1,#0` / `adds r0,#34` to 0x08004720
    // (96/108). The `u16` width matters: a `u8`/`u32` value changes the store
    // or the sign of the clear.
    u16 z = 0;
    volatile u8 *p = (volatile u8 *)lo + 34;
    while ((s32)(uintptr_t)p >= (s32)(uintptr_t)lo) {
        *(volatile u16 *)(uintptr_t)p = z;
        p -= 2;
    }
    *(volatile u16 *)(uintptr_t)hi = 0;
    }
}
#ifndef __APPLE__
void sub_080046D0(void) __attribute__((alias("BgRotDispatch_046D0")));
void _080046D0(void) __attribute__((alias("BgRotDispatch_046D0")));
#endif

// ----------------------------------------------------------------------------
// Palette fade 0x080048D8 — pair over s16[0x03000188+8]: scaleB = 0x08005F44,
// scaleA = 0x08005F2C; for i in 0..95: a = word[0x080C4F64 + i*8],
// b = word[+4]; write at dst = ctrl[3]+0x400 + i*16:
//   [dst+8] = ctrl[0] + ((a*scaleB - b*scaleB) >> 12)   (asm: (a-b)*scaleB)
//   [dst+12] = ctrl[1] - ((a*scaleA + b*scaleA) >> 12)  (asm: (a+b)*scaleA)
//   [dst+0] = (a*scaleB) >> 19 (u16), [dst+4] = (b*scaleA) >> 19 (u16)
// then CpuFastSet zero at ctrl[3]+159*16 and the post-loop tail, mode 0x05000004.
extern s16 SaveTriggerA(u32 arg); // _08005F2C (save.c)
extern s16 SaveTriggerB(u32 arg); // _08005F44
void PaletteFade_048D8(void)
{
    volatile u32 *ctrl = (volatile u32 *)(uintptr_t)0x03000188;
    s16 sel = *(volatile s16 *)((u8 *)ctrl + 8);
    s16 scaleB = SaveTriggerB((u32)sel);
    s16 scaleA = SaveTriggerA((u32)sel);
    volatile u8 *dst = (volatile u8 *)(uintptr_t)(ctrl[3] + 0x400);
    for (int i = 0; i <= 95; i++, dst += 16) {
        const u32 *mx = (const u32 *)(uintptr_t)(0x080C4F64 + (u32)i * 8);
        s32 a = (s32)mx[0];
        s32 b = (s32)mx[1];
        *(volatile s32 *)((u8 *)dst + 8) =
            (u32)((s32)ctrl[0] + (((a - b) * scaleB) >> 12));
        *(volatile s32 *)((u8 *)dst + 12) =
            (u32)((s32)ctrl[1] - (((a + b) * scaleA) >> 12));
        *(volatile u16 *)((u8 *)dst + 0) = (u16)((a * scaleB) >> 19);
        *(volatile u16 *)((u8 *)dst + 4) = (u16)((b * scaleA) >> 19);
    }
    u32 zero = 0;
    CpuFastSet(&zero, (void *)(uintptr_t)(ctrl[3] + (159 << 4)), 0x05000004);
}
#ifndef __APPLE__
void sub_080048D8(void) __attribute__((alias("PaletteFade_048D8")));
#endif

// ----------------------------------------------------------------------------
// 0x080049C4 — subsystem-tail bump alloc (0x03000198+136 cursor/+144 free):
// n == 0 → 0; when n > free, Foundation_InitCommon(n - 0x0805BA60) first;
// zero the old cursor via CpuFastSet mode ((n<<9)>>11) | 0x050000A0.
// 0x08004A10 — snapshot: +132 → +136, +140 → +144.
extern void Foundation_InitCommon(u32 a, u32 b);
void *HeapBump_049C4(u32 n)
{
    volatile u8 *base = (volatile u8 *)(uintptr_t)0x03000198;
    volatile u32 *cur = (volatile u32 *)(base + 136);
    volatile u32 *rem = (volatile u32 *)(base + 144);
    u32 old = *cur;
    if (n == 0)
        return 0;
    if (n > *rem) {
        Foundation_InitCommon(n - *(volatile u32 *)(uintptr_t)0x0805BA60, 0);
    }
    *cur = old + n;
    *rem = *rem - n;
    u32 mode = ((n << 9) >> 11) | 0x050000A0u;
    u32 zero = 0;
    CpuFastSet(&zero, (void *)(uintptr_t)old, mode);
    return (void *)(uintptr_t)old;
}
void HeapSnap_04A10(void)
{
    volatile u8 *base = (volatile u8 *)(uintptr_t)0x03000198;
    *(volatile u32 *)(base + 136) = *(volatile u32 *)(base + 132);
    *(volatile u32 *)(base + 144) = *(volatile u32 *)(base + 140);
}
#ifndef __APPLE__
void *sub_080049C4(u32 n) __attribute__((alias("HeapBump_049C4")));
void sub_08004A10(void) __attribute__((alias("HeapSnap_04A10")));
#endif

// ----------------------------------------------------------------------------
// Stragglers — final runtime_2aac.s gaps (byte-verified against baserom).
// ----------------------------------------------------------------------------

// 0x08002BFC — 16-byte object bump allocator over *(0x03000154) with a
// free-list: if the refill counter *(0x03000150) > 0 it is decremented and the
// cursor bumped +16; otherwise CpuFastSet-wipe 0x0800 (2048 bytes at
// 0x0802E204) via sub_0800295C(a,b=0). Returns the pre-bump cursor.
// Shape notes (ROM, asm/runtime_2aac.s):
//   - the cursor is dereferenced BEFORE the counter pointer is formed
//     (`ldr r2,[pc]; ldr r4,[r2]; ldr r1,[pc]; ldr r0,[r1]`), so `ret` must
//     be read before `cnt` appears in the source;
//   - the counter is loaded once into a register and reused
//     (`cmp r0,#0; subs r0,#1; str r0,[r1]`), so it needs a local, not
//     `*cnt = *cnt - 1` (which re-loads);
//   - the guard is a SIGNED `ble` (0xdd0a), so the local must be signed:
//     with an unsigned counter agbcc emits `beq` instead.
void *ObjAlloc_02BFC(int a)
{
    (void)a; // asm takes r0 but never reads it
    volatile u32 *cur = (volatile u32 *)(uintptr_t)0x03000154;
    u32 ret = *cur;
    volatile int *cnt = (volatile int *)(uintptr_t)0x03000150;
    int c = *cnt;
    if (c > 0) {
        *cnt = c - 1;
        *cur = ret + 16;
    } else {
        // ROM `bl 0x0800295C` (asm/code_295c.s). Closure spelling on ARM, the
        // friendly name on the host — unguarded it is a silent C89 implicit
        // declaration on Apple, so the call site carries the __APPLE__ split.
#ifndef __APPLE__
        _0800295C((u32)(uintptr_t)0x0802E204, 0);
#else
        Foundation_InitCommon((u32)(uintptr_t)0x0802E204, 0);
#endif
    }
    return (void *)(uintptr_t)ret;
}
void *sub_08002BFC(int a) __attribute__((alias("ObjAlloc_02BFC")));
#ifndef __APPLE__
void *_08002BFC(int a) __attribute__((alias("ObjAlloc_02BFC")));
#endif

// 0x08002C98 — OAM flush: for each of *(0x03000144) entries, walk the linked
void ObjFlush_02C98(void)
{
    volatile u32 *head = (volatile u32 *)(uintptr_t)0x03000140;
    volatile u32 *shadowp = (volatile u32 *)(uintptr_t)0x03000134;
    volatile u32 *countp = (volatile u32 *)(uintptr_t)0x03000144;
    u32 n = *countp;
    volatile u8 *dst = (volatile u8 *)(uintptr_t)*shadowp;
    volatile u16 *keys = (volatile u16 *)(uintptr_t)0x030035C0;
    s16 kx = (s16)keys[6];
    int kb = kx >> 3;
    u32 mask = 0x1FFu;
    for (u32 i = 0; i < n; i++) {
        volatile u8 *node = (volatile u8 *)(uintptr_t)head[i];
        while (node) {
            u16 type = *(volatile u16 *)(node + 12);
            if (type == 1) {
                u16 a1 = *(volatile u16 *)(node + 2);
                int off = ((int)kb) & (int)mask;
                u16 hi = (u16)(((u32)off << 9) & (mask << 9));
                a1 = (u16)((a1 & ~(mask << 9)) | hi);
                *(volatile u16 *)(node + 2) = a1;
            }
            *(volatile u32 *)(dst + 0) = *(volatile u32 *)(node + 0);
            *(volatile u16 *)(dst + 4) = *(volatile u16 *)(node + 4);
            dst += 8;
            node = (volatile u8 *)(uintptr_t)*(volatile u32 *)(node + 8);
        }
    }
    CpuFastSet((const void *)(uintptr_t)*shadowp, (void *)(uintptr_t)0x03000000,
               (224u << 19) | (128u << 1) | 1u);
}
void sub_08002C98(void) __attribute__((alias("ObjFlush_02C98")));
#ifndef __APPLE__
void _08002C98(void) __attribute__((alias("ObjFlush_02C98")));
#endif

// 0x08003104 — tail dispatch: bx r0 (r0 = handler VMA|1). Callers (5 BL sites,
// e.g. @0x0800C0A6) pass record+96/100 addresses.
void TailDispatch_03104(void *handler)
{
    ((void (*)(void))(uintptr_t)handler)();
}
void sub_08003104(void *handler) __attribute__((alias("TailDispatch_03104")));
#ifndef __APPLE__
void _08003104(void *handler) __attribute__((alias("TailDispatch_03104")));
#endif

// 0x08004090 — centered-text wrapper: 5 args; e = flags byte (stack arg 5).
// Calls 0x08003FD4(sel=3, x=a, y=b, 0, str=c, 1, d, flags).
// Call-site split, the RS_CALLEE pattern (race_scene.c): 0x08003FD4 is NOT a
// promoted VMA, so its section is dropped from the slice object and the
// linker binds nothing for the friendly name. The closure spelling
// `sub_08003FD4` is what asm/runtime_2aac.s exports at that address, so the
// target build must call THAT or the promoted body fails to link.
#ifndef __APPLE__
#define HUD_CALLEE(friendly, closure) closure
extern void sub_08003FD4(int sel, u32 x, u32 a2, u32 a3,
                         const volatile u8 *str, u16 w5, u32 w6, u8 w7);
#else
#define HUD_CALLEE(friendly, closure) friendly
#endif
void TextCenter_04090(int a, int b, const volatile u8 *c, u32 d, u8 e)
{
    HUD_CALLEE(ObjList_03FD4, sub_08003FD4)(3, (u32)a, (u32)b, 0, c, 1, d, e);
}
void sub_08004090(int a, int b, const volatile u8 *c, u32 d, u8 e)
    __attribute__((alias("TextCenter_04090")));
#ifndef __APPLE__
void _08004090(int a, int b, const volatile u8 *c, u32 d, u8 e)
    __attribute__((alias("TextCenter_04090")));
#endif

void CamAux_0473C(u32 v)
{
    volatile u32 *p = (volatile u32 *)(uintptr_t)0x03000188;
    p[3] = v;
}
void sub_0800473C(u32 v) __attribute__((alias("CamAux_0473C")));
void _0800473C(u32 v) __attribute__((alias("CamAux_0473C")));

void CamSet_04748(u32 a, u32 b, u32 c)
{
    volatile u32 *p = (volatile u32 *)(uintptr_t)0x03000188;
    p[0] = a;
    p[1] = b;
    *(volatile u16 *)(p + 2) = (u16)c;
}
void sub_08004748(u32 a, u32 b, u32 c) __attribute__((alias("CamSet_04748")));
void _08004748(u32 a, u32 b, u32 c) __attribute__((alias("CamSet_04748")));

// ----------------------------------------------------------------------------
// Missing VMA spellings for the object-lane / centering / measured delegates
// above. The ROM call sites' C lifts spell these `_0800xxxx` (and two as
// `Sub_0800xxxx`), while the bodies here only exported the `sub_0800xxxx`
// form — so every such call reached the asm body instead of the C body.
// The aliases below bind them.
// Signatures repeat the owners' exactly (declaration type agreement is
// enforced by -Werror=attribute-alias).
// ----------------------------------------------------------------------------
#ifndef __APPLE__
void _08003838(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_03838")));
void _080038A4(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_038A4")));
void _080038C8(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_038C8")));
void _0800390C(int a, u32 b, const volatile u8 *c) __attribute__((alias("ObjLane_0390C")));
void _08003954(int a, u32 b, int c) __attribute__((alias("ObjMeasure_03954")));
void Sub_08003954(int a, u32 b, int c) __attribute__((alias("ObjMeasure_03954")));
void _08003978(int a, u32 b, int c) __attribute__((alias("ObjMeasure_03978")));
void Sub_08003978(int a, u32 b, int c) __attribute__((alias("ObjMeasure_03978")));
void _080039C0(int a, u32 b, int c) __attribute__((alias("ObjMeasure_039C0")));
void _0800392C(int a, u32 b) __attribute__((alias("ObjCenter_0392C")));
#endif
