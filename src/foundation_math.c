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
#ifndef __APPLE__
int _08005B3C_abs(int a) __attribute__((alias("MathAbs")));
#endif
#ifndef __APPLE__
int _08005B6E_sign(int a) __attribute__((alias("MathSign")));
#endif
#ifndef __APPLE__
int _08005B8A_abs16(int a) __attribute__((alias("MathAbs16")));
#endif

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
#ifndef __APPLE__
void _08005BA8(void *a,int b) __attribute__((alias("MathHelper_05BA8")));
void sub_08005BA8(void *a,int b) __attribute__((alias("MathHelper_05BA8")));
#endif

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
#ifndef __APPLE__
int _08005C58(int a,int b) __attribute__((alias("MathHelper_05C58")));
#endif
int MathHelper_05CB4(void *p){
    s32 x = *(s32*)p;
    s32 y = *(s32*)((u8*)p+4);
    if(x==0 && y==0) return 0;
    int ax = x<0 ? -x : x;
    int ay = y<0 ? -y : y;
    if(ax > ay){
        extern int __aeabi_idiv(int,int);
        int q = __aeabi_idiv(ay<<7, ax);
        s16 e = *(volatile s16*)(0x0805DAF0 + (q<<1));
        if(x<0) { if(y<0) e+=0xC00; else e = -e; }
        else if(y<0) e = 0x400 - e;
        return e;
    } else {
        if(ay==0) return 0;
        extern int __aeabi_idiv(int,int);
        int q = __aeabi_idiv(ax<<7, ay);
        s16 e = *(volatile s16*)(0x0805DAF0 + (q<<1));
        if(x<0) { if(y<0) e = 0x800 - e; else e += 0x400; }
        else if(y>=0) e = 0x400 - e;
        return e;
    }
}
#ifndef __APPLE__
int _08005CB4(void *a) __attribute__((alias("MathHelper_05CB4")));
#endif
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
#ifndef __APPLE__
void MathRot(void *p, int angle) __attribute__((alias("MathHelper_05BA8")));
#endif
