#include "gba/types.h"

// ai_line_more.s remaining: 255C4/25640 sums + 2581C family
// Provide faithful byte-for-byte behavior for host test (we just implement lookup sums)


// _080255C4 — first catalog sum (8 bytes aggregated)
int Ai_LineSumA(const void *rec){
    const s8 *r = (const s8 *)rec;
#ifndef __APPLE__
    extern const u8 LineScoreTable[];
    __asm__(".globl LineScoreTable\nLineScoreTable = 0x080CD6A8\n");
    const u8 *tbl = LineScoreTable;
#else
    const u8 *tbl = (const u8 *)(uintptr_t)0x080CD6A8u;
#endif
    const u8 *p2 = tbl + r[2] * 8;
    const u8 *p6 = tbl + r[6] * 8 + 33;
    register int v2 __asm__("r1") = p2[1];
    __asm__("" : "+r"(v2));
    register int v6 __asm__("r0") = *p6;
    register int sum __asm__("r0");
#ifndef __APPLE__
    __asm__("add %0, %1, %2" : "=r"(sum) : "r"(v2), "r"(v6));
#else
    sum = v2 + v6;
#endif
    sum += (tbl + r[4] * 8)[65];
    sum += (tbl + r[7] * 8)[97];
    sum += (tbl + r[5] * 8)[129];
    sum += (tbl + r[9] * 8)[161];
    sum += (tbl + r[3] * 8)[193];
    sum += (tbl + r[8] * 8)[225];
    return (s8)sum;
}
#ifndef __APPLE__
__asm__(".align 2, 0");
int _080255C4(const void *a) __attribute__((alias("Ai_LineSumA")));
int Sub_080255C4(const void *a) __attribute__((alias("Ai_LineSumA")));
#endif

// _08025640 sum with offsets 0,32,64,96,128,160,192,224
int Ai_LineSumB(const void *rec){
    const s8 *r = (const s8 *)rec;
#ifndef __APPLE__
    extern const u8 LineScoreTable[];
    __asm__(".globl LineScoreTable\nLineScoreTable = 0x080CD6A8\n");
    const u8 *tbl = LineScoreTable;
#else
    const u8 *tbl = (const u8 *)(uintptr_t)0x080CD6A8u;
#endif
    const u8 *p2 = tbl + r[2] * 8;
    const u8 *p6 = tbl + r[6] * 8 + 32;
    register int v2 __asm__("r1") = p2[0];
    __asm__("" : "+r"(v2));
    register int v6 __asm__("r0") = *p6;
    register int sum __asm__("r0");
#ifndef __APPLE__
    __asm__("add %0, %1, %2" : "=r"(sum) : "r"(v2), "r"(v6));
#else
    sum = v2 + v6;
#endif
    sum += (tbl + r[4] * 8)[64];
    sum += (tbl + r[7] * 8)[96];
    sum += (tbl + r[5] * 8)[128];
    sum += (tbl + r[9] * 8)[160];
    sum += (tbl + r[3] * 8)[192];
    sum += (tbl + r[8] * 8)[224];
    return (s8)sum;
}
#ifndef __APPLE__
__asm__(".align 2, 0");
int _08025640(const void *a) __attribute__((alias("Ai_LineSumB")));
int Sub_08025640(const void *a) __attribute__((alias("Ai_LineSumB")));
#endif

// _0802581C / _08002581C — find course index by scanning 0x080600CC
int Ai_LineFindCourse(int id){
    const s16 *tbl=(const s16*)(uintptr_t)0x080600CCu;
    for(int i=0;i<=31;++i) if(id==tbl[i]) return (s16)i;
    return 0;
}
#ifndef __APPLE__
int _0802581C(int a) __attribute__((alias("Ai_LineFindCourse")));
int Sub_0802581C(int a) __attribute__((alias("Ai_LineFindCourse")));
// The THIRD twin, lowercase `sub_`. `tools/matching_slice_functions.json`
// promotes 0x0802581c with `c_name: _0802581C` and `export: ["sub_0802581C"]`,
// and the slice link synthesises that pair itself, but a promoted C body
// calling `sub_0802581C` should not depend on the synthesis. One hop
// to the real body, same guard as its two siblings.
int sub_0802581C(int a) __attribute__((alias("Ai_LineFindCourse")));
#endif
// _08025848 -> constant 32
int Ai_LineConst32(void){ return 32; }
#ifndef __APPLE__
int _08025848(void) __attribute__((alias("Ai_LineConst32")));
#endif

// _0802584C/5C/6C lookups over 0x0806010C 8-byte records. Two source levers,
// each checked by compiling this TU with agbcc and diffing against
// baserom.gba; both are load-bearing and neither is cosmetic.
//   * Reading through a struct type keeps the halfword offset in the ldrsh
//     INDEX register (`mov r1,#2`), where the ROM has it. Written as
//     `t[idx*4+1]` on a plain `s16 *`, agbcc folds base+2 into the pool word
//     (0x0806010E) and the offset disappears from the code.
//   * The `+ t[0].fN - t[0].fN` cancel gives the table base a second,
//     independent address node, so its `ldr r1,=0x0806010C` is issued before
//     the `lsl r0,r0,#3` index shift. Without it agbcc sinks the literal load
//     to just before the `adds`, and the body's first two instructions come
//     out swapped (a 12/16 match that no wording of `t[idx*4]` can fix).
typedef struct { s16 f0, f1, f2, f3; } Ai_LineRec4;
#define AI_LINE_TAB4 ((const Ai_LineRec4 *)(uintptr_t)0x0806010Cu)
s16 Ai_LineLookupA(int idx){ const Ai_LineRec4 *t=AI_LINE_TAB4; return t[idx].f0+t[0].f1-t[0].f1; }
// f1 is the +2 field. With an s16 result agbcc truncates the promoted sum as
// `ldrh [r0,#2]; lsls #16; asrs #16` (three instructions) instead of the ROM's
// single `ldrsh`, and that three-instruction form appears for offset 2 alone
// (0, 4, 6, 8, 10, 12 and 14 all keep the one-instruction `ldrsh`). This leaf
// therefore returns int; nothing in src/ declares _0802585C, so no other TU's
// prototype depends on it, and _08002580C in this file is already `int`.
int Ai_LineLookupB(int idx){ const Ai_LineRec4 *t=AI_LINE_TAB4; return t[idx].f1+t[0].f0-t[0].f0; }
s16 Ai_LineLookupC(int idx){ const Ai_LineRec4 *t=AI_LINE_TAB4; return t[idx].f2+t[0].f2-t[0].f2; }
#ifndef __APPLE__
s16 _0802584C(int a) __attribute__((alias("Ai_LineLookupA")));
#endif
#ifndef __APPLE__
int _0802585C(int a) __attribute__((alias("Ai_LineLookupB")));
#endif
#ifndef __APPLE__
s16 _0802586C(int a) __attribute__((alias("Ai_LineLookupC")));
#endif

// _0802587C / _08002587C — same scan over 0x0806010C, stride 8, 3 entries.
// Identical mechanism to _0802581C above (no `(s16)id`, id on the left,
// `(s16)` on the return); only the table, the stride and the bound differ.
int Ai_LineFindIn6010C(int id){
    const s16 *t=(const s16*)(uintptr_t)0x0806010Cu;
    for(int i=0;i<=2;++i) if(id==t[i*4]) return (s16)i;
    return 0;
}
#ifndef __APPLE__
int _0802587C(int a) __attribute__((alias("Ai_LineFindIn6010C")));
#endif

// 257 family — exact pure-Thumb table arithmetic, opaque volatile
// Three source levers, all read off the ROM and all load-bearing:
//   * The table base is a named pointer, and the body gives it a SECOND
//     address node via a `+ t[0].x - t[0].x` cancel (as _0802580C above).
//     Two uses force agbcc to hold the constant in a register, which hoists
//     the literal load to the entry where the ROM has it (0x802572e:
//     ldr r4,[pc,#28]; 0x80257f0: ldr r3,[pc,#20]). In 2572C the base is
//     live across a call, so that register is callee-saved and the prologue
//     grows to `push {r4, lr}`. Written as one integral expression
//     `*(volatile s16*)(0x0805FCACu + off)` the constant is folded into a
//     pool word and materialised at its point of use instead, mid-body.
//   * The read goes through a struct type, which keeps the FIELD offset in
//     the ldrsh index register (movs r2,#0 / #22) and picks `ldrsh` over
//     the `ldrh` + separate `lsls/asrs` truncation a bare `*(volatile
//     s16*)` costs. It also supplies the 12-byte stride arithmetic
//     `lsls r1,r0,#1; adds r1,r1,r0; lsls r1,r1,#2` for 2572C.
//   * 257F0 computes the `b*3` term BEFORE the `a*33` term. agbcc walks a
//     sum left-to-right and accumulates into the first term's register, so
//     `(a*33 + b*3)*8` yields the ROM's roles reversed (r2 holding a*33,
//     r0 holding b*3). Assigning `s = b*3` before `q = a*33` and summing
//     `(q + s)` forces the order: r2 takes the b*3 temp while r0 still
//     holds a, and a*33 lands in b's now-dead r1, which stays the addend.
typedef struct { s16 f0; s8 pad[10]; } Ai_LineRec12;   /* 12-byte stride */
typedef struct { s8 pad[22]; s16 v; } Ai_LineRec22;    /* s16 field at 22 */
s16 Ai_Line2572C(int a,int b,int c){
#ifndef __APPLE__
    extern int _080256D8(int,int,int);   // 0x080256D8, the closure spelling (asm/ai_line_more.s:155)
#else
    extern int Ai_CatalogFieldTmpB(int,int,int);
#endif
    const Ai_LineRec12 *t;
    s16 v;
    t = (const Ai_LineRec12 *)(uintptr_t)0x08060124u;
#ifndef __APPLE__
    v = (s16)_080256D8(a,b,c);
#else
    v = (s16)Ai_CatalogFieldTmpB(a,b,c);
#endif
    return t[v].f0 + t[0].f0 - t[0].f0;
}
#ifndef __APPLE__
s16 _0802572C(int a,int b,int c) __attribute__((alias("Ai_Line2572C")));
s16 Sub_0802572C(int a,int b,int c) __attribute__((alias("Ai_Line2572C")));
#endif

int Ai_Line25750(int a, int b, int c) {
    register int b_reg __asm__("r4") = b;
    register int c_reg __asm__("r3") = c;
    if (a != 0) {
#ifndef __APPLE__
        extern const u8 Ai_Tbl5FCAC[];
        __asm__(".globl Ai_Tbl5FCAC\nAi_Tbl5FCAC = 0x0805FCAC\n");
        register const u8 *base __asm__("r2") = Ai_Tbl5FCAC;
#else
        register const u8 *base __asm__("r2") = (const u8 *)0x0805FCAC;
#endif
        register int c3 __asm__("r1") = c_reg * 3;
        register int b33 __asm__("r0") = b_reg * 33;
        int off = (b33 + c3) * 8;
        base += 16;
        return *(const int *)(off + (uintptr_t)base);
    } else {
#ifndef __APPLE__
        extern const u8 Ai_Tbl5FCAC[];
        __asm__(".globl Ai_Tbl5FCAC\nAi_Tbl5FCAC = 0x0805FCAC\n");
        register const u8 *base __asm__("r2") = Ai_Tbl5FCAC;
#else
        register const u8 *base __asm__("r2") = (const u8 *)0x0805FCAC;
#endif
        register int c3 __asm__("r1") = c_reg * 3;
        register int b33 __asm__("r0") = b_reg * 33;
        int off = (b33 + c3) * 8;
        base += 12;
        return *(const int *)(off + (uintptr_t)base);
    }
}
#ifndef __APPLE__
int _08025750(int a,int b,int c) __attribute__((alias("Ai_Line25750")));
#endif

s16 Ai_Line25790(int a, int b, int c) {
    register int b_reg __asm__("r4") = b;
    register int c_reg __asm__("r3") = c;
    if (a != 0) {
#ifndef __APPLE__
        extern const u8 Ai_Tbl5FCAC[];
        __asm__(".globl Ai_Tbl5FCAC\nAi_Tbl5FCAC = 0x0805FCAC\n");
        register const u8 *base __asm__("r2") = Ai_Tbl5FCAC;
#else
        register const u8 *base __asm__("r2") = (const u8 *)0x0805FCAC;
#endif
        register int c3 __asm__("r1") = c_reg * 3;
        register int b33 __asm__("r0") = b_reg * 33;
        int off = (b33 + c3) * 8;
        const u8 *p = (const u8 *)(off + (uintptr_t)base);
        return *(const s16 *)(p + 8);
    } else {
#ifndef __APPLE__
        extern const u8 Ai_Tbl5FCAC[];
        __asm__(".globl Ai_Tbl5FCAC\nAi_Tbl5FCAC = 0x0805FCAC\n");
        register const u8 *base __asm__("r2") = Ai_Tbl5FCAC;
#else
        register const u8 *base __asm__("r2") = (const u8 *)0x0805FCAC;
#endif
        register int c3 __asm__("r1") = c_reg * 3;
        register int b33 __asm__("r0") = b_reg * 33;
        int off = (b33 + c3) * 8;
        const u8 *p = (const u8 *)(off + (uintptr_t)base);
        return *(const s16 *)(p + 6);
    }
}
#ifndef __APPLE__
s16 _08025790(int a,int b,int c) __attribute__((alias("Ai_Line25790")));
#endif
// 0x080257D4: the +20 halfword of the 8-byte record at 0x0805FCAC. The ROM
// span is byte-identical to 0x080256BC (src/ai_line_leaves.c
// Ai_CatalogFieldTmpA, promoted EXACT), and that body's shape note applies
// verbatim -- two details are load-bearing and neither is a width:
//   * the two multiply terms must be separate named locals declared in ROM
//     order (`b3` then `a33`) and summed as `(a33+b3)`. Written inline as
//     `(a*33 + b*3)*8` agbcc emits a*33 first, which reorders the body and
//     lands different registers;
//   * the `+t[0]-t[0]` cancel gives the table base a second address node, so
//     0x0805FCAC stays in r3 and the literal load is hoisted to the entry.
//     Without it the base folds to 0x0805FCAC+20 and both the pool word and
//     the `movs r2,#20` in the load are wrong.
// The return type is int (race_dispatch.c already declares it that way); the
// ROM's `ldrsh` supplies the sign extension the callers rely on.
int Ai_Line257D4(int a,int b,int c){ const u8 *t=(const u8*)(uintptr_t)0x0805FCACu; int b3=b*3; int a33=a*33; const s16 *p=(const s16*)(t+(a33+b3)*8); return p[10]+(int)t[0]-(int)t[0]; } // byte offset +20
#ifndef __APPLE__
int _080257D4(int a,int b,int c) __attribute__((alias("Ai_Line257D4")));
#endif
s16 Ai_Line257F0(int a,int b,int c){
    const Ai_LineRec22 *t;
    const Ai_LineRec22 *p;
    int s, q;
    (void)c;
    t = (const Ai_LineRec22 *)(uintptr_t)0x0805FCACu;
    s = b*3;
    q = a*33;
    p = (const Ai_LineRec22 *)((const s8 *)t + (q + s)*8);
    return p->v + t[0].v - t[0].v;
}
#ifndef __APPLE__
s16 _080257F0(int a,int b,int c) __attribute__((alias("Ai_Line257F0")));
#endif
// Same second lever as the 0x0806010C lookups: the `+ t[0] - t[0]` cancel
// hoists `ldr r1,=0x080600CC` above the `lsl r0,r0,#1` index shift.
int _0802580C(int a) { const s16*t=(const s16*)(uintptr_t)0x080600CCu; return t[a*1]+t[0]-t[0]; }

#ifndef __APPLE__
int _08002580C(int a) __attribute__((alias("_0802580C")));
#endif

// ROM entry alias.
#ifndef __APPLE__
int _080025750(int a,int b,int c) __attribute__((alias("Ai_Line25750")));
s16 _08002584C(int idx) __attribute__((alias("Ai_LineLookupA")));
int _08002587C(int id) __attribute__((alias("Ai_LineFindIn6010C")));
#endif

// ROM entry alias.
#ifndef __APPLE__
int _08002581C(int id) __attribute__((alias("Ai_LineFindCourse")));
#endif
