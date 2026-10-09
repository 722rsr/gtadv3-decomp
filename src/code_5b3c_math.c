// ============================================================================
// code_5b3c_math.c — C lift of asm/code_5b3c.s (VMA 0x08005B3C–0x08005F2C),
// the 12 pure-math leaves not already substantiated in foundation_math.c.
//
// Every function is transcribed from the cited asm listing (labels = bare
// VMAs; control flow, widths, call ABI preserved). This file replaces the
// earlier incomplete MathHelper_05D74 / MathHelper_05DA4 bodies in
// foundation_math.c with exact transcription (aliases are removed there; see
// the header note in this file's companion doc).
//
// NOTE on 0x08005B3C: this region is a passthrough-declared start, so until
// asm/code_5b3c.s carried its own label at that address the inventory saw one
// 32-byte span running through the real 16-byte body AND through the
// function that starts at 0x08005B4C — a body nothing branches to, hence
// invisible. The bare `_08005B3C:` region marker plus a `.type`/label pair at
// 0x08005B4C (both zero bytes) split them. Measured with
// tools/corpus_match_probe.py --c89 (see each entry for its own numbers).

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) int DivSI(int num, int den) { return den ? num / den : 0; }
__attribute__((weak)) void _08005BA8(void *p, int angle) { (void)p; (void)angle; }
// 0x0802DE04 is the signed EABI idiv, at asm/sound_aeabi_idiv.s / src/sound_extra.c
__attribute__((weak)) int  sub_0802DE04(int n, int d) { return d ? n / d : 0; }
__attribute__((weak)) void sub_08005DA4(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) int  sub_08005E14(void *a, void *b, void *c)
                                       { (void)a; (void)b; (void)c; return 0; }
#else
extern int  DivSI(int num, int den);          // sound_support.c (zero-guard == _08002DE98 no-op)
extern int  sub_0802DE04(int n, int d);       // 0x0802DE04 signed EABI divide (asm/sound_aeabi_idiv.s)
#endif

// ROM sin/cos tables (byte-exact pool words in the listing):
#define C5B3C_COS_TBL  ((volatile s16 *)(uintptr_t)0x0805BAF0)   // _08005BE4/_08005C30
#define C5B3C_SIN_TBL  ((volatile s16 *)(uintptr_t)0x0805CAF0)   // _08005BEC/_08005C38
#define C5B3C_ATAN_TBL ((volatile s16 *)(uintptr_t)0x0805DAF0)   // _08005D04/_08005D48
#define C5B3C_ANGLE_MASK 0x00000FFEu

// ----------------------------------------------------------------------------
// Timer-queue getters (asm/save_timer_queue.s pool 0x030003D4 -> 0x030003B0;
// 4 slots x 4 B: [0]=busy u8, [1]=delay u8, [2]=payload u16; +32 armed count,
// +34 fired count; fired-event words collect at +16).
s16 TimerFiredCount_05B3C(void) {
    u8 *q = *(u8 *volatile *)0x030003D4u;
    return *(s16 *)(q + 34);
}
#ifndef __APPLE__
s16 _08005B3C(void) __attribute__((alias("TimerFiredCount_05B3C")));
s16 sub_08005B3C(void) __attribute__((alias("TimerFiredCount_05B3C")));
#endif

// 0x08005B4C — 16B (body 10 + pool 4): u16[queue + 18 + i*4] = payload
u16 TimerPayload_05B4C(u32 i) {
    u8 *q = *(u8 *volatile *)0x030003D4u;
    u8 *e = q + i * 4;
    return *(u16 *)(e + 18);
}
#ifndef __APPLE__
u16 _08005B4C(u32 i) __attribute__((alias("TimerPayload_05B4C")));
u16 sub_08005B4C(u32 i) __attribute__((alias("TimerPayload_05B4C")));
#endif

// 0x08005B64 — 22 B body + 2 B inter-function align pad = 24 B span
// (cmp #0; bge; movs #1; negs; b; cmp #0; bgt; movs #0; b; movs #1; bx lr),
// i.e. the -1/0/+1 sign ternary. It is bl-called from asm/garage_26f50.s and
// carries no label at all, which is why `asm_vmas` dropped it: a VMA needs a
// label AND (a `.type` or an inbound `bl`).
int TimerSign_05B64(int v) {
    if (v < 0)
        return -1;
    if (v > 0)
        return 1;
    return 0;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
int _08005B64(int v) __attribute__((alias("TimerSign_05B64")));
int sub_08005B64(int v) __attribute__((alias("TimerSign_05B64")));
#endif

// ----------------------------------------------------------------------------
// 0x08005B7C — 16 B s16 abs leaf, 4-byte aligned between the two align pads.
//   ROM: `lsls/asrs #16` (widen the short parameter); `cmp #0; bge end`;
//   `negs r0,r0` then `lsls/asrs #16` again (the result is stored back into
//   the SAME short local, so the truncation pair is inside the branch); one
//   shared `bx lr` epilogue.
int MathAbs_05B7C(s16 v) {
    return v < 0 ? (s16)-v : v;
}
#ifndef __APPLE__
int _08005B7C(s16 v) __attribute__((alias("MathAbs_05B7C")));
int sub_08005B7C(s16 v) __attribute__((alias("MathAbs_05B7C")));
#endif

// 0x08005B8C — 28 B s16 sign leaf: -1 / 0 / +1 over a sign-extended short.
//   ROM: `lsls/asrs #16; cmp #0; bge L; movs #1; negs; b end;`
//        `L: cmp #0; bgt L2; movs #0; b end; L2: movs #1; end: bx lr`.
// The two compares are on the widened value, so the parameter is `s16`.
int MathSign_05B8C(s16 v) {
    if (v < 0)
        return -1;
    if (v > 0)
        return 1;
    return 0;
}
// The 28-byte span ends with the 2-byte inter-function align pad the ROM holds
// as `00 00`; `as` would close the section with the Thumb nop `46 c0`.
__asm__(".align 2, 0");
#ifndef __APPLE__
int _08005B8C(s16 v) __attribute__((alias("MathSign_05B8C")));
int sub_08005B8C(s16 v) __attribute__((alias("MathSign_05B8C")));
#endif

// ----------------------------------------------------------------------------
// 0x08005B5C — abs leaf. (cmp r0,#0; bge; negs r0; bx lr)
// NOTE: identical body exists as MathAbs in foundation_math.c; aliased here so
// the coverage gap at 0x08005B5C closes against the same evidence-backed body.
int MathLeaf_05B5C(int v) {
    return v < 0 ? -v : v;
}
#ifndef __APPLE__
int _08005B5C(int v) __attribute__((alias("MathLeaf_05B5C")));
int sub_08005B5C(int v) __attribute__((alias("MathLeaf_05B5C")));
#endif

// ----------------------------------------------------------------------------
// 0x08005D74 — vector constructor + rotate (matches sub_08005BA8 math):
//   [r0+0] = 0; [r0+4] = -r1; then s16 angle (r2) and rotate.
//
// Mechanism: a QI pseudo that cannot be coalesced onto the SI source must be
// given its own register, and the only register available for the second call
// argument is r1 — so the QI->SI conversion becomes a genuine
// register-to-register sign-extend, which on Thumb-1 IS the lsls/asrs pair.
// This is a GNU extension, so the unit is not strict C89; the project's C89
// transform processes it unchanged and the C89-equivalence gate still passes.
//
// Best measured: prefix 22 / 24 (first_diff 22), matched 22. EXACT is NOT
// reachable and the remaining 2 bytes prove why: the real body is 22 bytes
// and the ROM's 0x08005D8A-0x08005D8B are the 2 bytes of inter-function link
// padding (`00 00`) that align 0x08005D8C, whereas agbcc emits no trailing
// nop and GNU `as` pads the 22-byte `.text.MathLeaf_05D74` section to its
// declared 4-byte alignment with the Thumb NOP `46c0` (verified in the object
// file: section size 0x18, last word 0x46c0). No C body can make `as` emit
// `00 00` there, so 22 is the ceiling. It is still promotable: 22 + 2 == 24
// and the ROM tail is `00 00`.
#ifndef __APPLE__
extern void sub_08005BA8(void *a, int b);   // closure spelling, 0x08005BA8
#else
extern void _08005BA8(void *a, int b);     // host weak stub in runtime_record_helpers.c
#endif
void MathLeaf_05D74(void *p, int y, int angle) {
    volatile s32 *r = (volatile s32 *)p;
    r[0] = 0;
    r[1] = -y;
    // The ROM does not inline the rotate here: it tail-calls the shared
    // 0x08005BA8 body with r0 still the incoming pointer and r1 the s16 angle.
    register s16 t __asm__("r2") = (s16)angle;
    int a = t;   // ordinary local, deliberately NOT a second pin (see above)
    // Declared AND called under the guard: guarding only the declaration just
    // moves the implicit declaration to the other side, and
    // -Wimplicit-function-declaration is an error under `-Werror`.
    // The closure spells 0x08005BA8 both ways; `sub_` is preferred because the
    // `_` form is a local `t` symbol no separate C object can resolve.
#ifndef __APPLE__
    sub_08005BA8(p, a);
#else
    _08005BA8(p, a);
#endif
}
__asm__(".align 2, 0");

#ifndef __APPLE__
void _08005D74(void *a, int b, int c) __attribute__((alias("MathLeaf_05D74")));
void sub_08005D74(void *a, int b, int c) __attribute__((alias("MathLeaf_05D74")));
#endif

// ----------------------------------------------------------------------------
// 0x08005D8C — vec2 difference: out = c - b (both 2 x s32).
void MathLeaf_05D8C(void *out, void *b, void *c) {
    volatile u8 *o = (volatile u8 *)out;
    volatile u8 *vb = (volatile u8 *)b;
    volatile u8 *vc = (volatile u8 *)c;
    *(volatile s32 *)(o + 0) = *(volatile s32 *)(vc + 0) - *(volatile s32 *)(vb + 0);
    *(volatile s32 *)(o + 4) = *(volatile s32 *)(vc + 4) - *(volatile s32 *)(vb + 4);
}
#ifndef __APPLE__
void _08005D8C(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05D8C")));
void sub_08005D8C(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05D8C")));
#endif

// ----------------------------------------------------------------------------
// 0x08005DA4 — perpendicular projection of v=(b0,b1):
//   out+0 = v1; out+4 = -v0; out+8 = b0*v1 + b1*(-v0)
void MathLeaf_05DA4(void *out, void *b) {
    volatile s32 *o = (volatile s32 *)out;
    const volatile s32 *v = (const volatile s32 *)b;
    // ROM 0x08005DA4 order, statement for statement:
    //   r4=[b+12]; [o+0]=r4; r3=-[b+8]; [o+4]=r3;
    //   r2=[b+0]*r4; r1=[b+4]*r3; [o+8]=r2+r1
    // `a`/`c` stay named so r4/r3 survive into the muls instead of being
    // reloaded; moving the three stores to the end diverges at +0x08.
    s32 a = v[3];
    *o = a;
    s32 c = -v[2];
    o[1] = c;
    o[2] = v[0] * a + v[1] * c;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08005DA4(void *a, void *b) __attribute__((alias("MathLeaf_05DA4")));
void sub_08005DA4(void *a, void *b) __attribute__((alias("MathLeaf_05DA4")));
#endif

// ----------------------------------------------------------------------------
// 0x08005DC4 — cross-projected perpendicular:
//   out+0 = c1 - b1; out+4 = b0 - c0; out+8 = b0*(c1-b1) + c0*(b1-c1)
void MathLeaf_05DC4(void *out, void *b, void *c) {
    volatile s32 *o = (volatile s32 *)out;
    // Plain (non-volatile) sources: the ROM loads [c+4], [b+4], [b+0] and
    // [c+0] exactly once each and keeps [b+4] in r4 across the whole body,
    // which only CSE can do -- `volatile` forces a re-`ldr` per mention and
    // spills into a fifth register.
    const s32 *p = (const s32 *)b;
    const s32 *q = (const s32 *)c;
    // ROM 0x08005DC4 order:
    //   r3=[c+4]-r4([b+4]); [o+0]=r3; r2=[b+0]-[c+0]; [o+4]=r2;
    //   r1=[b+0]*r3; r2=r2*r4; [o+8]=r1+r2
    s32 x = q[1] - p[1];
    *o = x;
    s32 y = p[0] - q[0];
    o[1] = y;
    o[2] = p[0] * x + y * p[1];
}
#ifndef __APPLE__
void _08005DC4(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05DC4")));
void sub_08005DC4(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05DC4")));
#endif

// ----------------------------------------------------------------------------
// 0x08005DE4 — s16 variant of 05D8C:
//   out+0 = (s16)[c+2] - (s16)[b+2]; out+4 = (s16)[b+0] - (s16)[c+0]
//   out+8 = (s16)[b+0] * that + (s16)[b+2] * (out+4)
// Byte-exact (48/48). Two measured levers, not a return type:
//   * the ROM loads are bare `ldrsh reg,[base,idx]` with no widen pair, so the
//     sources are `const s16 *` — a volatile sub-word lvalue makes agbcc emit
//     `ldrh` + `lsls/asrs` (the P3 defect). Indexing halfwords (vb[1], not
//     *(s16*)(vb+2)) is what yields the ROM's register-offset form.
//   * each store must sit between the two differences it depends on: the ROM
//     stores out+0 before computing the out+4 difference. Moving all three
//     stores at the end puts both stores after both subs and diverges at +0xC.
void MathLeaf_05DE4(void *out, void *b, void *c) {
    volatile u32 *o = (volatile u32 *)out;
    const s16 *vb = (const s16 *)b;
    const s16 *vc = (const s16 *)c;
    s32 d0 = vc[1] - vb[1];
    o[0] = d0;
    s32 d1 = vb[0] - vc[0];
    o[1] = d1;
    o[2] = vb[0] * d0 + vb[1] * d1;
}
#ifndef __APPLE__
void _08005DE4(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05DE4")));
void sub_08005DE4(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05DE4")));
#endif

// ----------------------------------------------------------------------------
// 0x08005E14 — 2-D line intersection.
//   b = {x, y, dx, dy} (s32[4]); c = {x, y, dx, dy} (s32[4]):
//   det = b.dx*c.dy - c.dx*b.dy
//   if det == 0: return 0
//   num_t = c.dy*b.len - b.dy*c.len
//   num_u = b.dx*c.len - c.dx*b.len
//   out+0 = DivSI(num_t, det); out+4 = DivSI(num_u, det); return 1
int MathLeaf_05E14(void *out, void *b, void *c) {
    volatile u8 *rb = (volatile u8 *)b;
    volatile u8 *rc = (volatile u8 *)c;
    s32 bx = *(volatile s32 *)(rb + 0);
    s32 cdy = *(volatile s32 *)(rc + 4);
    s32 p = bx * cdy;
    s32 by = *(volatile s32 *)(rb + 4);
    s32 cx = *(volatile s32 *)(rc + 0);
    s32 det = p - cx * by;
    if (det == 0)
        return 0;
    {
        s32 blen = *(volatile s32 *)(rb + 8);
        s32 t = cdy * blen;
        s32 clen = *(volatile s32 *)(rc + 8);
        *(volatile s32 *)out = sub_0802DE04(t - by * clen, det);
        *(volatile s32 *)((volatile u8 *)out + 4) = sub_0802DE04(bx * clen - cx * blen, det);
    }
    return 1;
}
#ifndef __APPLE__
int _08005E14(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05E14")));
int sub_08005E14(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05E14")));
#endif

int MathLeaf_05E7C(void *out, void *a, void *b) {
    volatile s32 *o = (volatile s32 *)out;
    register volatile s32 *vb __asm__("r4") = (volatile s32 *)b;
    volatile s32 *fr = (volatile s32 *)a;
    s32 frame[7];
    volatile s32 *p;
    frame[0] = fr[0];
    frame[1] = fr[1];
    frame[2] = vb[0];
    frame[3] = vb[1];
    p = frame + 4;
    // Called through the CLOSURE spellings, not the friendly C names: this body
    // is promoted, so its two calls must resolve in the spliced link, which
    // holds asm/code.s plus this section and no C object. `sub_08005DA4` and
    // `sub_08005E14` are the labels asm/code_5b3c.s defines at 0x08005DA4 and
    // 0x08005E14; `MathLeaf_05DA4` / `MathLeaf_05E14` are C bodies with no
    // assembly label at all. Both address the same code, so the `bl`
    // displacements -- and therefore the body -- are unchanged.
    sub_08005DA4((void *)p, (void *)frame);
    sub_08005E14((void *)o, (void *)vb, (void *)p);
    {
        s32 dx = o[0] - *(volatile s32 *)a;
        o[0] = dx;
        s32 dy = o[1] - *((volatile s32 *)((volatile u8 *)a + 4));
        o[1] = dy;
        return dx * dx + dy * dy;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
int _08005E7C(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05E7C")));
int sub_08005E7C(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05E7C")));
#endif

// ----------------------------------------------------------------------------
// 0x08005EDC — parallel test / intersection gate.
// ROM: out->r6, b->r4; frameA[4]@sp = {a[0], a[1], b[1], -b[0]}; r5=sp+28;
// 5DA4(r5, sp); frameB = {b[0], b[1], b[0], b[1]} via r1-hold pattern;
// r4=sp+16 (b dies); 5DA4(r4, sp); gate = 5E14(out, r5, r4); return (s8)gate.
int MathLeaf_05EDC(void *out, void *a, void *b) {
#ifndef __APPLE__
    register void *o __asm__("r6") = out;
    register s32 *bb __asm__("r4") = (s32 *)b;
#else
    void *o = out;
    s32 *bb = (s32 *)b;
#endif
    s32 *aa = (s32 *)a;
    s32 w[4];
    s32 ob[3];
    s32 oa[3];
    w[0] = aa[0];
    w[1] = aa[1];
    w[2] = bb[1];
    w[3] = -bb[0];
    {
#ifndef __APPLE__
        register s32 *r5 __asm__("r5");
        register s32 *r4b __asm__("r4");
#else
        s32 *r5;
        s32 *r4b;
#endif
        r5 = oa;
        MathLeaf_05DA4(r5, w);
        {
            s32 t1 = bb[0];
            w[0] = t1;
            s32 t0 = bb[1];
            w[1] = t0;
            w[2] = t1;
            w[3] = t0;
        }
        r4b = ob;
        MathLeaf_05DA4(r4b, w);
        {
            int hit = MathLeaf_05E14(o, r5, r4b);
            return (int)(s8)hit;
        }
    }
}
#ifndef __APPLE__
int _08005EDC(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05EDC")));
int sub_08005EDC(void *a, void *b, void *c) __attribute__((alias("MathLeaf_05EDC")));
#endif
