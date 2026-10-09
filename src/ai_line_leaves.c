#include "gtadv/ai_line.h"
#include "gba/types.h"

// ---- ai_line_accessors trio + accessor4 ----

struct LineRec {
    s8 b[20];
};

// _080254E8 — 24B: ldr r2,=0x080CCEEC / lsls+asrs #16 / *20 / ldrsb [+0] / bx lr.
// LINK-CLEAN second-address-node cancel. The table base is a function-local
// pointer and gets a second address node via `+ t[0] - t[0]`, which forces agbcc
// to hold 0x080CCEEC in a register and hoists the literal load to the entry where
// the ROM has it (0x80254e8 ldr r2,[pc,#16]). Nothing at file scope is added, so
// this introduces no unresolved reference for the slice link. Without the cancel
// agbcc sinks the load to just before the `adds` (0x80254f2 ldr r0,[pc,#8]) and
// the body scores 12/24 with prefix 0.
s8 Ai_LineGet0(int id){ const s8 *t=(const s8*)(uintptr_t)0x080CCEECu; int v=(s16)id; return t[v*20]+t[0]-t[0]; }
#ifndef __APPLE__
s8 _080254E8(int a) __attribute__((alias("Ai_LineGet0")));
s8 Sub_080254E8(int a) __attribute__((alias("Ai_LineGet0")));
#endif
s8 Ai_LineGet1(int id){ const struct LineRec *t=(const struct LineRec *)(uintptr_t)0x080CCEECu; int v=(s16)id; return t[v].b[14]+t->b[0]-t->b[0]; }
#ifndef __APPLE__
s8 _08025500(int a) __attribute__((alias("Ai_LineGet1")));
s8 Sub_08025500(int a) __attribute__((alias("Ai_LineGet1")));
s8 sub_08025500(int a) __attribute__((alias("Ai_LineGet1")));
#endif
s8 Ai_LineGet2(int id){ const struct LineRec *t=(const struct LineRec *)(uintptr_t)0x080CCEECu; int v=(s16)id; return t[v].b[13]+t->b[0]-t->b[0]; }
#ifndef __APPLE__
s8 _08025518(int a) __attribute__((alias("Ai_LineGet2")));
s8 Sub_08025518(int a) __attribute__((alias("Ai_LineGet2")));
s8 sub_08025518(int a) __attribute__((alias("Ai_LineGet2")));
#endif
s8 Ai_LineGet3(int id){ const struct LineRec *t=(const struct LineRec *)(uintptr_t)0x080CCEECu; int v=(s16)id; return t[v].b[12]+t->b[0]-t->b[0]; }
#ifndef __APPLE__
s8 _08025530(int a) __attribute__((alias("Ai_LineGet3")));
s8 Sub_08025530(int a) __attribute__((alias("Ai_LineGet3")));
s8 sub_08025530(int a) __attribute__((alias("Ai_LineGet3")));
#endif

// _08025214 — 52 B bit probe: `id` -> byte `id>>3` of the collection bitfield at
// 0x030015E8 AND byte `id&7` of the mask table at 0x080C4768, as 0/1.
//
// Four things are load-bearing, each pinned by a register or an address node:
//
//  * `id` is PINNED to r3. It has to survive the divide (which clobbers r0) to
//    reach `subs r0, r3, r0`, and r0 is the return register, so the allocator
//    will not leave the value there by itself.
//  * The divide is spelled `if (a < 0) dv = a + 7; dv >>= 3` rather than `a / 8`.
//    Both lower to the same rounding-toward-zero sequence, but `a / 8` lets agbcc
//    fold the copy of `id` away and test `cmp r0, #0` on the incoming argument.
//    The explicit form forces `cmp r3, #0` against the pinned copy.
//  * Both table bases are absolute SYMBOL_REFs, not integer constants, and each
//    is written back through its own pointer variable
//    (`bm = base; bm = (u8 *)((uintptr_t)bm + (uintptr_t)idx);`). That is what
//    makes the literal load COALESCE with the pointer it feeds: the ROM has
//    `adds r1, r0, r1` (destination == base register) where a plain `base + idx`
//    emits a separate base register and a three-operand add instead.
//  * `bit` is PINNED to r0. `bit` is computed after `idx` is consumed, and r0 is
//    the natural accumulator for `lsls r0,r0,#3`, but agbcc's local allocator
//    otherwise prefers the (still-unread) r3, giving `sub r3, r3, r0` and
//    `add r2, r2, r3`. Pinning it also keeps the final `mt` pointer in r0 for the
//    second `ldrb`, which is where the ROM has it.
//
// Without the symbol refs the pool words fold into the pointer adds and the
// base register allocation shifts; the body then scores 41/52 with the first
// difference at +0x02 (the `cmp`).
int Ai_LineProbe(int id){
    register int a __asm__("r3");
    int dv;
    int idx;
    register int bit __asm__("r0");
    u8 *b2;
    u8 *bm;
    u8 *mt;
    extern u8 AiLineColBM[] __asm__("AiLineColBM");
    extern u8 AiLineMaskTbl[] __asm__("AiLineMaskTbl");
    __asm__(".globl AiLineColBM\nAiLineColBM = 0x030015E8\n");
    __asm__(".globl AiLineMaskTbl\nAiLineMaskTbl = 0x080C4768\n");
    a = id;
    dv = a;
    if (a < 0) dv = a + 7;
    idx = dv >> 3;
    bm = AiLineColBM;
    bm = (u8*)((uintptr_t)bm + (uintptr_t)idx);
    b2 = AiLineMaskTbl;
    bit = a - idx * 8;
    mt = (u8*)((uintptr_t)b2 + (uintptr_t)bit);
    if (*bm & *mt) return 1;
    return 0;
}
#ifndef __APPLE__
int _08025214(int a) __attribute__((alias("Ai_LineProbe")));
#endif

// _08025248 write via 0x0802D974
void Ai_LineWrite(int id){
    u32 v=0;
    extern void sub_0802D974(const void*,void*,u32);
    sub_0802D974(&v, (void*)(uintptr_t)id, 0x05000006u);
}
#ifndef __APPLE__
void _08025248(int a) __attribute__((alias("Ai_LineWrite")));
// `sub_08025248` is the spelling the closure labels at 0x08025248
// (asm/ai_line_write.s:7, plus the `.type` line above it in passthrough.inc:530),
// and the promotion's start marker is that same `.type` line -- so the splice
// deletes the label and the entry has to re-export it. One hop, to the real
// body: an alias chain through a second alias is two `.thumb_set` hops and the
// intermediate is not in the slice link.
void sub_08025248(int a) __attribute__((alias("Ai_LineWrite")));
#endif

// _08024FE8 helper
s16 Ai_LineHelperLoad(int a, int b, int c) {
    register const u8 *base __asm__("r4");
    base = (const u8 *)0x080CCD8C;
    __asm__("" : "+r"(base));
    c <<= 2;
    c += b * 44;
    c += a * 176;
    c += (u32)base;
    return ((const s16 *)c)[1];
}
#ifndef __APPLE__
s16 _080024FE8(int a, int b, int c) __attribute__((alias("Ai_LineHelperLoad")));
s16 sub_080024FE8(int a, int b, int c) __attribute__((alias("Ai_LineHelperLoad")));
s16 _08024FE8(int a, int b, int c) __attribute__((alias("Ai_LineHelperLoad")));
s16 sub_08024FE8(int a, int b, int c) __attribute__((alias("Ai_LineHelperLoad")));
#endif

// _0802500C find slot < = id? actually >?
int Ai_LineFindSlot(int id, const void *baseVoid){
    const u32 *base = (const u32*)baseVoid;
    int idx = 0;
    do {
        if ((u32)id >= *base) {
            base += 3;
            idx++;
        } else {
            return idx;
        }
    } while (idx <= 4);
    return 5;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
int _08002500C(int a, const void *b) __attribute__((alias("Ai_LineFindSlot")));
int sub_08002500C(int a, const void *b) __attribute__((alias("Ai_LineFindSlot")));
int _0802500C(int a, const void *b) __attribute__((alias("Ai_LineFindSlot")));
int sub_0802500C(int a, const void *b) __attribute__((alias("Ai_LineFindSlot")));
#endif

// _08025084 variant with <= ?
int Ai_LineFindSlot2(int id, const void *baseVoid){
    // Twin of _0802500C with the OPPOSITE sense: the ROM's `bls` is ARM's
    // UNSIGNED less-or-equal (BLS is unsigned; BLE is the signed <=), so the
    // advance arm is `(u32)id <= *base`. `id < (int)*base` compiles to the
    // signed `blt`, one halfword off at +0x08.
    const u32 *base = (const u32*)baseVoid;
    int idx = 0;
    do {
        if ((u32)id <= *base) {
            base += 3;
            idx++;
        } else {
            return idx;
        }
    } while (idx <= 4);
    return 5;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
int _080025084(int a, const void *b) __attribute__((alias("Ai_LineFindSlot2")));
int sub_080025084(int a, const void *b) __attribute__((alias("Ai_LineFindSlot2")));
int _08025084(int a, const void *b) __attribute__((alias("Ai_LineFindSlot2")));
int sub_08025084(int a, const void *b) __attribute__((alias("Ai_LineFindSlot2")));
#endif

// _080250FC equality of 12-byte records (id + halfword + masked word)
//
// What the residual was: ROM `ldrh r3,[r3,#4] / ldrh r4,[r4,#4] / cmp r3,r4`
// at +0x1C reuses each DEAD POINTER register as the halfword destination;
// candidate `ldrh r0,[r3,#4] / ldrh r4,[r4,#4] / cmp r0,r4` allocates a fresh
// register for the first and reuses the pointer for the second. Note the
// second one already coalesced -- so the allocator was never forbidden from
// doing this, only unwilling to do it twice. Two bytes, one allocation.
//
// The caller's spelling (`extern int _080250FC(const void*,const void*)` in
// _08025198/_08025130 below) is unchanged and still resolves, and the
// `Ai_LineEqual` alias is untouched.
int Ai_LineEqual(const void *a,const void *b){
    const u32 *pa=(const u32*)a, *pb=(const u32*)b;
    if(pa[0]!=pb[0]) return 0;
    if( (pa[2] & 0x00FFFFFF) != (pb[2] & 0x00FFFFFF) ) return 0;
    // PIN (GNU ext; see the group note). Forces the ROM register r3, the
    // destination of `ldrh r3,[r3,#4]` at +0x1C. `pa` is r3 and dies at that
    // load, so this coalesces into `pa` rather than displacing it. Without
    // the pin: `ldrh r0,[r3,#4] / cmp r0,r4`, 50/52, first diff +0x1C.
    register u16 ha __asm__("r3") = ((const u16*)pa)[2];
    if( ha != ((u16*)pb)[2]) return 0;
    return 1;
}
#ifndef __APPLE__
int _080250FC(const void *a,const void *b) __attribute__((alias("Ai_LineEqual")));
#endif

struct LineEntry {
    u32 w[3];
};

// _08025028 copy insert with shift
void Ai_LineCopyInsert(void *dst, const void *src){
    struct LineEntry *d = (struct LineEntry *)dst;
    const struct LineEntry *s = (const struct LineEntry *)src;
    int pos = Ai_LineFindSlot((int)s->w[0], d);
    int i;
    if (pos == 5)
        return;
    for (i = 3; i >= pos; i--) {
        d[i + 1] = d[i];
    }
    d[pos] = *s;
}
#ifndef __APPLE__
void _08025028(void *a,const void *b) __attribute__((alias("Ai_LineCopyInsert")));
void sub_08025028(void *a,const void *b) __attribute__((alias("Ai_LineCopyInsert")));
#endif

// _080250A0 variant uses slot2 logic? same but with <=
void Ai_LineCmpInsert(void *dst, const void *src){
    struct LineEntry *d = (struct LineEntry *)dst;
    const struct LineEntry *s = (const struct LineEntry *)src;
    int pos = Ai_LineFindSlot2((int)s->w[0], d);
    int i;
    if (pos == 5)
        return;
    for (i = 3; i >= pos; i--) {
        d[i + 1] = d[i];
    }
    d[pos] = *s;
}
#ifndef __APPLE__
void _080250A0(void *a,const void *b) __attribute__((alias("Ai_LineCmpInsert")));
void sub_080250A0(void *a,const void *b) __attribute__((alias("Ai_LineCmpInsert")));
#endif

// _080251FC timing helper
// The ROM body is `push {lr}; ldr r1,=0x030015E8; ldr r2,=0x04000002; bl
// 0x0802D974; pop {r1}; bx r1` -- r0 is never written, so the first CpuSet
// argument IS the caller's incoming r0 verbatim. That is a real parameter,
// not an indeterminate value: taking it as `const void *src` models the
// contract exactly and lets the register allocator pass it through in r0 with
// no `movs` to undo. Modelling it instead as a read of an uninitialised local
// also matched the bytes, but only by relying on undefined behaviour and on a
// `#pragma GCC diagnostic ignored "-Wuninitialized"` to hide the warning.
// A build that is green because the diagnostic was suppressed is not a
// build that is verified.
void Ai_LineTimingHelper(const void *src){
    extern void _0802D974(const void*,void*,u32);
    _0802D974(src, (void*)(uintptr_t)0x030015E8u, (u32)0x04000002u);
}
#ifndef __APPLE__
void _080251FC(const void *src) __attribute__((alias("Ai_LineTimingHelper")));
#endif

// _08025198 — 100B, no pool, high-reg r8, stack slot for r2 preservation
// Mechanically translated: outer r6=0..4, inner r5=0..4, compare src+r5*12 (3 words, 12 B) vs dst+r6*12 via _080250FC (returns 0/1 via lsls #24), if equal store 0 at src+r5*12 (u32 via str) and leave the inner loop, then 5× _080250A0(dst, src+i*12) walking a u8* forward.
// Two shape facts the ROM fixes: the loop counters are `int`, not s16 (s16 buys
// lsls #16/asrs #16 pairs the ROM has nowhere), and the store ends the inner
// loop -- the `str` falls straight into the outer latch at +0x3E, so the inner
// iteration is a `break`, not an `if`.
void _08025198(void *a, void *b){
    volatile u8 *dst = (volatile u8*)a;
    volatile u8 *src = (volatile u8*)b;
    extern int _080250FC(const void*,const void*);
#ifndef __APPLE__
    extern void sub_080250A0(void*,const void*);   // 0x080250A0, the closure spelling
#else
    extern void _080250A0(void*,const void*);      // host wrapper
#endif
    for(int r6=0; r6<=4; ++r6){
        for(int r5=0; r5<=4; ++r5){
            volatile u8 *pSrc = src + (r5*3)*4;
            volatile u8 *pDst = dst + (r6*3)*4;
            int eq = _080250FC((const void*)pSrc, (const void*)pDst);
            if((eq<<24)!=0){
                *(volatile u32*)pSrc = 0; // u32 width via str
                break;
            }
        }
    }
    {
        volatile u8 *p = src;
        for(int n=4; n>=0; --n){
#ifndef __APPLE__
            sub_080250A0((void*)dst, (const void*)p);
#else
            _080250A0((void*)dst, (const void*)p);
#endif
            p += 12;
        }
    }
}
// _08025130 — 104B, pool 0x7FFFFFFF at.L_5194, same loops but store sentinel and use _08025028
// Mechanically translated: the _08025198 shape verbatim with the 0x7FFFFFFF sentinel in place of the plain 0 store and _08025028 in place of _080250A0 in the trailing walk. Same `int` counters, same `break` after the store, same pointer walk.
void _08025130(void *a, void *b){
    volatile u8 *dst = (volatile u8*)a;
    volatile u8 *src = (volatile u8*)b;
    extern int _080250FC(const void*,const void*);
#ifndef __APPLE__
    extern void sub_08025028(void*,const void*);   // 0x08025028, the closure spelling
#else
    extern void _08025028(void*,const void*);      // host wrapper
#endif
    for(int r6=0; r6<=4; ++r6){
        for(int r5=0; r5<=4; ++r5){
            volatile u8 *pSrc = src + (r5*3)*4;
            volatile u8 *pDst = dst + (r6*3)*4;
            int eq = _080250FC((const void*)pSrc, (const void*)pDst);
            if((eq<<24)!=0){
                *(volatile u32*)pSrc = 0x7FFFFFFFu; // sentinel u32 via ldr 0x7FFFFFFF pool
                break;
            }
        }
    }
    {
        volatile u8 *p = src;
        for(int n=4; n>=0; --n){
#ifndef __APPLE__
            sub_08025028((void*)dst, (const void*)p);
#else
            _08025028((void*)dst, (const void*)p);
#endif
            p += 12;
        }
    }
}

#ifndef __APPLE__
extern const u8 LineScoreTable[];
__asm__(".globl LineScoreTable\nLineScoreTable = 0x080CD6A8\n");
#endif

// score sums over 0x080CD6A8 table
int Ai_LineScoreSum(const void *rec){
    const s8 *r = (const s8 *)rec;
#ifndef __APPLE__
    const u8 *tbl = LineScoreTable;
#else
    const u8 *tbl = (const u8 *)(uintptr_t)0x080CD6A8u;
#endif
    const u8 *p2 = tbl + r[2] * 8;
    const u8 *p6 = tbl + r[6] * 8 + 34;
    register int v2 __asm__("r1") = p2[2];
    __asm__("" : "+r"(v2));
    register int v6 __asm__("r0") = *p6;
    register int sum __asm__("r0");
#ifndef __APPLE__
    __asm__("add %0, %1, %2" : "=r"(sum) : "r"(v2), "r"(v6));
#else
    sum = v2 + v6;
#endif
    sum += (tbl + r[4] * 8)[66];
    sum += (tbl + r[7] * 8)[98];
    sum += (tbl + r[5] * 8)[130];
    sum += (tbl + r[9] * 8)[162];
    sum += (tbl + r[3] * 8)[194];
    sum += (tbl + r[8] * 8)[226];
    return (s8)sum;
}
#ifndef __APPLE__
__asm__(".align 2, 0");
int _08025548(const void *a) __attribute__((alias("Ai_LineScoreSum")));
int Sub_08025548(const void *a) __attribute__((alias("Ai_LineScoreSum")));
#endif


// _080256BC family: 8-byte record at 0x0805FCAC — each reads a different halfword.
// All four ROM bodies build the index from r0 and r1 only (a*33 + b*3, then <<3);
// r2 is never read, so the third argument is accepted but never used. Two shapes are
// load-bearing. (1) The two multiply terms must be separate named locals declared in
// ROM order (`b3` then `a33`) and summed as `(a33+b3)`: inline `(a*33+b*3)*8` makes
// agbcc emit a*33 first, which both reorders the body and picks different registers.
// (2) The `+t[0]-t[0]` cancel gives the table base a second address node, holding
// 0x0805FCAC in r3 and hoisting the literal load to the entry; without it the base
// folds to 0x0805FCAC+N and the pool word is wrong. The halfword offset then stays
// in the load (movs r2,#N / ldrsh r0,[r1,r2]) instead of being folded into it.
int Ai_CatalogFieldTmpA(int a,int b,int c){ const u8 *t=(const u8*)(uintptr_t)0x0805FCACu; int b3=b*3; int a33=a*33; const s16 *p=(const s16*)(t+(a33+b3)*8); return p[10]+(int)t[0]-(int)t[0]; } // byte offset +20
int Ai_CatalogFieldTmpB(int a,int b,int c){ const u8 *t=(const u8*)(uintptr_t)0x0805FCACu; int b3=b*3; int a33=a*33; const s16 *p=(const s16*)(t+(a33+b3)*8); return p[0]+(int)t[0]-(int)t[0]; }  // byte offset +0
int Ai_CatalogFieldTmpC(int a,int b,int c){ const u8 *t=(const u8*)(uintptr_t)0x0805FCACu; int b3=b*3; int a33=a*33; const s16 *p=(const s16*)(t+(a33+b3)*8); return p[2]+(int)t[0]-(int)t[0]; }  // byte offset +4
int Ai_CatalogFieldTmpD(int a,int b,int c){ const u8 *t=(const u8*)(uintptr_t)0x0805FCACu; int b3=b*3; int a33=a*33; const s16 *p=(const s16*)(t+(a33+b3)*8); return p[1]+(int)t[0]-(int)t[0]; }  // byte offset +2
#ifndef __APPLE__
int _080256BC(int a,int b,int c) __attribute__((alias("Ai_CatalogFieldTmpA")));
int sub_080256BC(int a,int b,int c) __attribute__((alias("Ai_CatalogFieldTmpA")));
int _080256D8(int a,int b,int c) __attribute__((alias("Ai_CatalogFieldTmpB")));
int sub_080256D8(int a,int b,int c) __attribute__((alias("Ai_CatalogFieldTmpB")));
int _080256F4(int a,int b,int c) __attribute__((alias("Ai_CatalogFieldTmpC")));
int sub_080256F4(int a,int b,int c) __attribute__((alias("Ai_CatalogFieldTmpC")));
int _08025710(int a,int b,int c) __attribute__((alias("Ai_CatalogFieldTmpD")));
int sub_08025710(int a,int b,int c) __attribute__((alias("Ai_CatalogFieldTmpD")));
#endif
