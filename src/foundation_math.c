#include "gtadv/foundation.h"
#include "gba/types.h"

// code_5b3c math leaves — behavioral C matching Thumb asm at 0x08005B3C+

// Verified single-instruction math leaves at 0x05B3C region — SUBSTANTIATED
// Each matches exactly one Thumb sequence in asm/code_5b3c.s
int MathAbs(int v) { // 0x05B5C: negs if <0 (3 insns: cmp/bge/negs)
    return v < 0 ? -v : v;
}
int MathSign(int v) { // 0x05B6E/0x05B76: cmp/bge/bgt -> -1/0/1 (verified)
    if (v < 0) return -1;
    if (v > 0) return 1;
    return 0;
}
int _08005B3C_abs(int a) __attribute__((alias("MathAbs")));
int _08005B6E_sign(int a) __attribute__((alias("MathSign")));
int _08005B8A_abs16(int a) __attribute__((alias("MathAbs16")));

int MathAbs16(int v) { // 0x05B8A variant: s16 sign-extend then abs
    s16 s = (s16)v;
    return s < 0 ? -s : s;
}

void MathHelper_05BA8(void *p, int angle){
    extern const s16 CosTbl05BA8[];
    extern const s16 SinTbl05BA8[];
    int cosv;
    int sinv;
    int x;
    int t1;
    int y;
    int t2;
    int nx;
    int ny;
    __asm__(".globl CosTbl05BA8\nCosTbl05BA8 = 0x0805BAF0\n");
    __asm__(".globl SinTbl05BA8\nSinTbl05BA8 = 0x0805CAF0\n");
    __asm__ volatile("" : : "r" (angle));
    cosv = CosTbl05BA8[(angle & 0xFFE) >> 1];
    sinv = SinTbl05BA8[(angle & 0xFFE) >> 1];
    x = *(int *)p;
    t1 = x * cosv;
    y = *((int *)p + 1);
    t2 = y * sinv;
    nx = (t1 - t2) >> 12;
    ny = (x * sinv + y * cosv) >> 12;
    *((int *)p + 1) = ny;
    *(int *)p = nx;
}
void _08005BA8(void *a,int b) __attribute__((alias("MathHelper_05BA8")));
void sub_08005BA8(void *a,int b) __attribute__((alias("MathHelper_05BA8")));

// _08005C58 (asm/code_5b3c.s:184): modular interpolation over 12-bit angles.
//   r6 = a & 0xFFF; r4 = b & 0xFFF; r5 = r6; t = |r5 - r4| (_08005B5C).
//   t > 0x800: off = (r4-r5)+0x1000 for r5>r4 (sign +1), else (r5-r4)+0x1000
//              (sign -1)
//   t <= 0x800: off = t, sign -1 when r5 > r4 else +1
//   result = (r6 + (sign<<1)*off) & 0xFFF, with `lsls r0,r0,#1` before the muls.
extern int _08005B5C(int v);
int MathHelper_05C58(int a,int b){
    // ROM home registers: a&0xFFF in r6, b&0xFFF in r4, the accumulator in r5.
    // The allocator otherwise keeps them in r4/r5/r6 with different roles,
    // which cascades through the branch arms.
    register int a12 __asm__("r6") = a;
    register int b12 __asm__("r4") = b;
    __asm__("" : "+r"(a12), "+r"(b12));
    int mask = 0xFFF;
    register int acc __asm__("r5");
    a12 &= mask;
    acc = a12;
    b12 &= mask;
    // The ROM keeps `off` in r1 for every arm (`adds r1,r0,r2` from the
    // 0x1000 add, and `off = t` reuses the r1 copy of the call result), which
    // in turn leaves r0 for the wrap difference and r2 for the 0x1000
    // materialisation (`movs r2,#128` / `lsls r2,r2,#5`).
    register int off __asm__("r1");
    int sign;
    {
        register int d __asm__("r0");
        int t = _08005B5C(acc - b12);
        if (t > 0x800) {
            if (acc > b12) { d = b12 - acc; off = d + 0x1000; sign = 1; }
            else           { d = acc - b12; off = d + 0x1000; sign = -1; }
        } else {
            off = t;
            if (acc > b12) sign = -1;
            else           sign = 1;
        }
    }
    return (a12 + (sign << 1) * off) & 0xFFF;
}
int _08005C58(int a,int b) __attribute__((alias("MathHelper_05C58")));
extern s16 MathAngleTableX[];
extern s16 MathAngleTableY[];
extern s32 sub_0802DE04(s32 numerator, s32 denominator);
int MathHelper_05CB4(void *p) {
    register volatile s32 *base __asm__("r6") = (volatile s32 *)p;
    register s32 x __asm__("r3");
    register s32 y __asm__("r5");
    register s32 ax __asm__("r1");
    register s32 ay __asm__("r2");
    register const s16 *table __asm__("r4");
    register s32 tableValue __asm__("r1");
    s32 ratio;
    register s32 angle __asm__("r0");

    __asm__ volatile (".globl MathAngleTableX\n"
                      "MathAngleTableX = 0x0805DAF0\n"
                      ".globl MathAngleTableY\n"
                      "MathAngleTableY = 0x0805DAF0\n");
    x = base[0];
    if (x == 0 && base[1] == 0)
        goto zero_result;
    if (x < 0)
        ax = (s32)(0u - (u32)x);
    else
        ax = x;
    y = base[1];
    if (y < 0)
        ay = (s32)(0u - (u32)y);
    else
        ay = y;

    if (ax > ay) {
        if (ax == 0)
            goto zero_result;
        table = MathAngleTableX;
        ratio = sub_0802DE04((s32)((u32)ay << 7), ax);
        tableValue = table[ratio];
        if (base[0] < 0) {
            if (y < 0) {
                ay = 3072;
                angle = tableValue + ay;
            } else {
                ay = 3072;
                goto subtract_offset;
            }
        } else {
            if (y < 0) {
                ay = 1024;
                goto subtract_offset;
            } else {
                ay = 1024;
                angle = tableValue + ay;
            }
        }
    } else {
        if (ay == 0)
            goto zero_result;
        table = MathAngleTableY;
        ratio = sub_0802DE04((s32)((u32)ax << 7), ay);
        tableValue = table[ratio];
        if (base[0] < 0) {
            if (y < 0) {
                ay = 4096;
                goto subtract_offset;
            } else {
                ay = 2048;
                angle = tableValue + ay;
            }
        } else {
            if (y < 0)
                return tableValue;
            ay = 2048;
            goto subtract_offset;
        }
    }
    goto signed_angle;

subtract_offset:
    angle = ay;
    __asm__ volatile ("" : "+r"(angle));
    angle -= tableValue;

signed_angle:
    return (s16)angle;
zero_result:
    return 0;
}
__asm__(".pushsection .text.MathHelper_05CB4,\"ax\",%progbits\n"
        ".short 0\n"
        ".popsection\n");
int _08005CB4(void *a) __attribute__((alias("MathHelper_05CB4")));
/*
 * 0x08005D74 (22 B) and 0x08005DA4 (34 B) are OWNED by src/code_5b3c_math.c
 * (`MathLeaf_05D74` / `MathLeaf_05DA4`, exported as _08005D74 / _08005DA4).
 *
 * This file used to carry placeholder bodies for both VMAs — a 6-byte guess
 * (`movs r3,#0; strh r3,[r0,#2]`) and a literal `bx lr` — under 1-arg
 * signatures. Two objects strongly defining one VMA is decided by link order
 * (`ld` runs with --allow-multiple-definition), so the placeholders could have
 * silently won; they were removed  rather than left to chance.
 */

// ROM entry alias.
void MathRot(void *p, int angle) __attribute__((alias("MathHelper_05BA8")));
