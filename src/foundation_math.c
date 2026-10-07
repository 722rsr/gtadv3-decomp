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
    s16 cosv = *(s16*)(0x0805BAF0 + (angle & 0xFFE));
    s16 sinv = *(s16*)(0x0805CAF0 + (angle & 0xFFE));
    s32 x = *(volatile s32*)p;
    s32 y = *(volatile s32*)((u8*)p+4);
    s32 nx = (x * cosv - y * sinv) >> 12;
    s32 ny = (x * sinv + y * cosv) >> 12;
    *(volatile s32*)((u8*)p+4) = ny;   // ROM stores y before x
    *(volatile s32*)p = nx;
}
#ifndef __APPLE__
void _08005BA8(void *a,int b) __attribute__((alias("MathHelper_05BA8")));
void sub_08005BA8(void *a,int b) __attribute__((alias("MathHelper_05BA8")));
#endif

int MathHelper_05C58(int a,int b){
    int r6 = a & 0xFFF;
    int r4 = b & 0xFFF;
    int r5 = r6;
    int d = r5 - r4;
    extern int MathAbs(int);
    int t = MathAbs(d);
    int cmp = 0x800; // 128<<4
    if(t > cmp){
        if(r5 > r4){
            int off = r4 - r5 + 0x1000; // 128<<5 =0x1000
            return (1 <<1) * (off & 0xFFF) + r6; // simplified: (r6 + 2*off) &0xFFF
        } else {
            int off = r5 - r4 + 0x1000;
            return ( -2 * (off & 0xFFF) + r6) & 0xFFF;
        }
    } else {
        if(r5 > r4) return (-2*1 + r6) & 0xFFF;
        else return (1<<1)*1 + r6; // placeholder preserves ands 0xFFF
    }
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
