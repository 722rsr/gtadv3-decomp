#include "gtadv/ai_grid.h"
#include "gba/types.h"
#include "gba/bios.h"

extern void _0802E0A4(void *dst, const void *src, u32 n);
extern int  _0802D97C(int idx,int bits);

// helpers for packed unpack
static inline int packed_shift(int idx,int bits){ return _0802D97C(idx,bits); }
// copy the 4-byte mask table 0x08060D48 ({03,0C,30,C0}) to scratch
static inline void bios_unpack(const void *src, void *dst, u32 n){ _0802E0A4(dst, src, n); }

// _08025CF4(type,row,col) -> u8 0..3
// The instruction-level lift lives in src/runtime_state_dispatch.c (AiGridGet_25CF4,
// which owns the _08025CF4/sub_08025CF4 aliases); this entry point re-exports
// it so the derived leaves below share one implementation instead of the
// earlier hand-approximation that lived here.
#ifdef __APPLE__
// Host-only fallback: the standalone host test does not link runtime_state_dispatch.c,
// so keep a self-contained (equivalent) body here. Must stay bit-exact with
// AiGridGet_25CF4 — including the final `>> (idx & 3) * 2` field extract
// (objdump 0x08025D14–0x08025D18: `ands r0,r1` then `lsls r1,r2,#1`/`asrs`).
int Ai_GridGet(int type,int row,int col){
    if (col > 10 || col < 0) return 0;
    u8 tmp[4];
    bios_unpack((const void*)(uintptr_t)AI_GRID_MASK_TBL, tmp, 4u);
    int idx = 44*type + 11*row + col;
    int rem = packed_shift(idx, 4);
    volatile u8 *base = (volatile u8*)(uintptr_t)AI_GRID_BASE;
    int off = (idx < 0) ? ((idx + 3) >> 2) : (idx >> 2);
    u8 v = base[off];
    return (int)(u8)((u8)(v & tmp[rem]) >> (rem * 2));
}
#else
extern int AiGridGet_25CF4(int type, int row, int col);
int Ai_GridGet(int type,int row,int col){
    return AiGridGet_25CF4(type, row, col);
}
#endif

// _08025D64(type,row) — count consecutive set cells in column? Actually asm counts col 0..10 where GridGet !=0
//
// Same unrotated-loop shape as the sibling 0x08025D90 below, for the same
// reason: a `for` header makes agbcc ROTATE this loop (guard to the bottom,
// `b.n` at the top) which scores 18/44 with the first difference at +0x0A,
// while the ROM keeps the guard at +0x0A/+0x0C, the body from +0x0E, and the
// backward `b.n` at +0x22.
int Ai_GridCountCol(int type,int row){
    int cnt=0;
    int col=0;
    int v;
col_loop:
    if(col > 10) goto col_done;
#ifndef __APPLE__
    // The slice links no C object, so a promoted body may only call a spelling
    // the closure defines. `Ai_GridGet` is a friendly name with no VMA and no
    // alias declaration of its own, so the screen reads it as "no VMA and not a
    // promoted export" and blocks 0x08025D64 at 44/44 EXACT. The closure spells
    // this address `_08025CF4` (asm/ai_grid.s:8, C-owned at
    // src/runtime_state_dispatch.c:1019); both denote 0x08025CF4, so the `bl` is
    // unchanged. The host build has no VMA-named symbol: it keeps the name.
    extern int _08025CF4(int type, int row, int col);
    v = _08025CF4(type,row,col);
#else
    v = Ai_GridGet(type,row,col);
#endif
    if((v<<24)==0) goto col_done;
    cnt++;
    col++;
    goto col_loop;
col_done: ;
    return cnt;
}
#ifndef __APPLE__
int _08025D64(int a,int b) __attribute__((alias("Ai_GridCountCol")));
int Sub_08025D64(int a,int b) __attribute__((alias("Ai_GridCountCol")));
#endif

// _08025D90(type) — rows fully occupied clamped ≤3
//
// The loop is spelled with an explicit `goto` rather than `for`/`while`, and
// that is load-bearing, not a style preference. agbcc recognises
// `for (row=0; row<=3; ++row)` as a countable loop and ROTATES it: the guard
// `cmp r4,#3 / bgt` moves to the bottom, `full++/row++` becomes the loop body,
// and control enters past the body with `b.n`. That scores 22/44 with the
// first difference at +0x08. The ROM keeps the UNrotated shape -- guard at
// +0x08/+0x0A, body from +0x0C, `b.n` back to the guard at +0x1C -- and so
// does the ROM's own callee 0x08025D64, which is the identical loop over
// col 0..10 (`cmp r4,#10 / bgt` at 0x08025D6E, `b.n` back at 0x08025D86). Two
// unrotated loops in one call chain, so this is the shape agbcc emits for the
// original source, not a fluke of one body.
//
// Writing the same two exits as `goto` yields a CFG agbcc's loop pass does not
// match, the rotation does not happen, and the body is 44/44 EXACT.
int Ai_GridRowsFull(int type){
    int full=0;
    int row=0;
    int c;
row_loop:
    if(row > 3) goto row_done;
#ifndef __APPLE__
    // Same rule as the callee above. `Ai_GridCountCol` does resolve to VMA
    // 0x08025D64 through its own alias declaration, but the closure spells that
    // address `_08025D64`, so the screen reports "closure defines
    // _08025D64/sub_08025D64 at 0x08025d64 (rename)" and blocks 0x08025D90.
    // `_08025D64` is a C-owned alias of the function called here (line 67), so
    // the `bl` target and its encoding are unchanged. Host keeps the name.
    c = _08025D64(type,row);
#else
    c = Ai_GridCountCol(type,row);
#endif
    if(c != 11) goto row_done;
    full++;
    row++;
    goto row_loop;
row_done: ;
    if(full>3) full=3;
    return full;
}
#ifndef __APPLE__
int _08025D90(int a) __attribute__((alias("Ai_GridRowsFull")));
#endif

void Ai_GridSetPacked(int type, int val){
    u8 tmp[4];
    register int v __asm__("r4") = val;
    register volatile u8 *cell __asm__("r5");
    _0802E0A4((void *)tmp, (const void *)(uintptr_t)AI_GRID_MASK_TBL, 4u);
    v &= 3;
    register int sh __asm__("r2") = packed_shift(type, 4);
    register volatile u8 *base __asm__("r0") = (volatile u8 *)(uintptr_t)AI_GRID_BASE;
    register int idx __asm__("r1") = type;
    if (type < 0) idx = type + 3;
    idx >>= 2;
    base += 24;
    cell = (volatile u8 *)(idx + (uintptr_t)base);
    register u8 *t __asm__("r1") = tmp;
    register volatile u8 *mp __asm__("r0") = t + sh;
    register u32 cur __asm__("r1") = *cell;
    register u32 mask __asm__("r0") = *mp;
#ifndef __APPLE__
    __asm__("bic %0, %1\n\tadd %1, %0, #0" : "+r"(cur), "+r"(mask));
    *cell = mask;
#else
    *cell = cur & ~mask;
#endif
    int sh2 = packed_shift(type, 4);
    v <<= (sh2 << 1);
    v |= *cell;
    *cell = v;
}
#ifndef __APPLE__
void _08025DBC(int a,int b) __attribute__((alias("Ai_GridSetPacked")));
void sub_08025DBC(int a,int b) __attribute__((alias("Ai_GridSetPacked")));
void Sub_08025DBC(int a,int b) __attribute__((alias("Ai_GridSetPacked")));
#endif

// _08025E1C(type) -> 0..3.  Byte-exact (84/84, whole body + pool).
//
// Four source shapes are load-bearing here.  Each was found by a probe run
// that moved the score, and each is worth 4-22 bytes:
//   * the unpack call must be written as a direct `_0802E0A4(dst, src, n)`
//     rather than through the `bios_unpack` wrapper. The wrapper is
//     `static inline`, so the RTL is nominally identical, but agbcc then
//     emits `mov r0,sp` before `ldr r1,[pc]`; the ROM has the literal load
//     first (`4911 ldr r1,[pc,#68]` / `4668 mov r0,sp`). Worth 4 -> 24
//     leading matching bytes.
//   * the index must be written inline (`idx >> 2`) rather than assigned
//     back into `idx`. The assignment makes GCC shift *in place* in r4
//     (`asrs r4, r4, #2`); the ROM shifts into a fresh register
//     (`asrs r0, r4, #2`) and adds the cell base into it.
//   * `base` must be a local pointer incremented by a statement of its own
//     (`base += 24;`), placed AFTER the sign fixup.  Written inline as
//     `(u8 *)AI_GRID_BASE + 24 + i4`, agbcc constant-folds the +24 into the
//     literal pool (0x03001798) and drops the ROM's runtime `3118 adds
//     r1,#24`; the pool word then differs too.  The split statement keeps
//     0x03001780 in the pool, which also lets the literal load hoist above
//     `cmp r4,#0` the way the ROM does (`490a ldr r1,[pc,#40]` at +0x24).
//     recorded this as an unreachable constant-folding difference
//     after trying pointer, integer, struct-header, live-across-call and
//     both-association-order spellings; the statement form was not one of
//     them.  51 -> 73.
//   * the two reads need distinct pointers and the v read has to come first:
//     `cell` is formed, then `mp = tmp + sh`, then `*cell`, then `*mp`.
//     That is the only order that gives the ROM's tail -- `mov r3,sp` /
//     `adds r1,r3,r5` (the mask address, into the register the base freed)
//     / `ldrb r0,[r0]` (v, reusing its own base register) / `ldrb r1,[r1]`
//     -- together with the ROM's head register choice (index in r0, base in
//     r1).  73 -> 84.
int Ai_GridGetPacked(int type){
    u8 tmp[4];
    _0802E0A4((void *)tmp, (const void *)(uintptr_t)AI_GRID_MASK_TBL, 4u);
    int sh = packed_shift(type, 4);
    int sh2 = packed_shift(type, 4);
    int idx = type;
    volatile u8 *base = (volatile u8 *)(uintptr_t)AI_GRID_BASE;
    if (idx < 0) idx += 3;
    int i4 = idx >> 2;
    base += 24;
    volatile u8 *cell = base + i4;
    volatile u8 *mp = tmp + sh;
    u8 v = *cell;
    u8 mask = *mp;
    return (u8)((v & mask) >> (sh2 << 1));
}
#ifndef __APPLE__
int _08025E1C(int a) __attribute__((alias("Ai_GridGetPacked")));
int Sub_08025E1C(int a) __attribute__((alias("Ai_GridGetPacked")));
#endif

#ifndef __APPLE__
__attribute__((naked)) void Ai_GridWriteRecord(u32 a,int b,int c, u32 val){
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "ldr r5, 1f\n"
        "lsls r4, r1, #1\n"
        "adds r4, r4, r1\n"
        "adds r4, r4, r2\n"
        "lsls r4, r4, #2\n"
        "lsls r1, r0, #1\n"
        "adds r1, r1, r0\n"
        "lsls r1, r1, #4\n"
        "adds r4, r4, r1\n"
        "movs r0, #176\n"
        "lsls r0, r0, #3\n"
        "adds r5, r5, r0\n"
        "adds r4, r4, r5\n"
        "str r3, [r4, #0]\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        "1: .word 0x03001780\n"
        ".syntax divided\n"
    );
}
#else
void Ai_GridWriteRecord(u32 a,int b,int c, u32 val){
    volatile u8 *base = (volatile u8*)(uintptr_t)AI_GRID_BASE;
    u32 off = 1408 + (u32)(a*48) + (u32)((b*3 + c)*4);
    *(volatile u32*)(base + off) = val;
}
#endif
#ifndef __APPLE__
void _08025E70(u32 a,int b,int c, u32 d) __attribute__((alias("Ai_GridWriteRecord")));
#endif

#ifndef __APPLE__
__attribute__((naked)) u32 Ai_GridReadRecord(u32 a,int b,int c){
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "ldr r4, 1f\n"
        "lsls r3, r1, #1\n"
        "adds r3, r3, r1\n"
        "adds r3, r3, r2\n"
        "lsls r3, r3, #2\n"
        "lsls r1, r0, #1\n"
        "adds r1, r1, r0\n"
        "lsls r1, r1, #4\n"
        "adds r3, r3, r1\n"
        "movs r0, #176\n"
        "lsls r0, r0, #3\n"
        "adds r4, r4, r0\n"
        "adds r3, r3, r4\n"
        "ldr r0, [r3, #0]\n"
        "pop {r4}\n"
        "pop {r1}\n"
        "bx r1\n"
        ".align 2, 0\n"
        "1: .word 0x03001780\n"
        ".syntax divided\n"
    );
}
#else
u32 Ai_GridReadRecord(u32 a,int b,int c){
    volatile u8 *base = (volatile u8*)(uintptr_t)AI_GRID_BASE;
    u32 off = 1408 + (u32)(a*48) + (u32)((b*3 + c)*4);
    return *(volatile u32*)(base+off);
}
#endif
#ifndef __APPLE__
u32 _08025E98(u32 a,int b,int c) __attribute__((alias("Ai_GridReadRecord")));
#endif


// _08025EC0(id,val) halfword packed
void Ai_GridHalfwordSet(int id, int val){
    u8 tmp[4];
    register int masked __asm__("r4") = id;
    register int v __asm__("r6") = val;
    register volatile u16 *cell __asm__("r5");
    _0802E0A4((void *)tmp, (const void *)(uintptr_t)AI_GRID_MASK_TBL, 4u);
    masked &= 7;
    register int sh __asm__("r2") = packed_shift(masked, 4);
    register uintptr_t base __asm__("r1");
    register u32 off __asm__("r3");
    register int i4 __asm__("r0");
#ifndef __APPLE__
    /* Absolute definition in-body: the splicer retains only the function's
       section, so a file-scope asm definition is omitted from the link. */
    extern u8 AiGridHalfBase[];
    __asm__(".globl AiGridHalfBase\nAiGridHalfBase = 0x03001780\n");
    base = (uintptr_t)AiGridHalfBase;
#else
    base = (uintptr_t)0x03001780;
#endif
    i4 = (masked / 4) << 1;
    off = 174;
    off = off << 3;
    base += off;
    cell = (volatile u16 *)(i4 + base);
    register u8 *t __asm__("r1") = tmp;
    register volatile u8 *mp __asm__("r0") = t + sh;
    register u32 cur __asm__("r3") = *cell;
    register u32 mask __asm__("r0") = *mp;
#ifndef __APPLE__
    __asm__("bic %0, %1\n\tadd %1, %0, #0" : "+r"(cur), "+r"(mask));
    *cell = mask;
#else
    *cell = cur & ~mask;
#endif
    int sh2 = packed_shift(masked, 4);
    v <<= (sh2 << 1);
    v |= *cell;
    *cell = v;
}
#ifndef __APPLE__
void _08025EC0(int a,int b) __attribute__((alias("Ai_GridHalfwordSet")));
void sub_08025EC0(int a,int b) __attribute__((alias("Ai_GridHalfwordSet")));
void Sub_08025EC0(int a,int b) __attribute__((alias("Ai_GridHalfwordSet")));
#endif

// _08025F20(id) -> s16>>shift & mask.  Byte-exact (88/88, whole body + pool).
//
// Four source shapes are load-bearing here; each was found by a probe run
// that moved the score:
//   * the IWRAM base is named as an absolute SYMBOL_REF
//     (`AiGridHalfBase = 0x03001780`), not the `AI_GRID_BASE` CONST_INT.
//     A CONST_INT base folds into the literal pool (the pool word becomes
//     0x03001CF0 and the body shrinks to 80-84 bytes); the SYMBOL_REF keeps
//     0x03001780 in the pool and forces the runtime `movs r3,#174 /
//     lsls r3,r3,#3 / adds r1,r1,r3` pair.
//   * the base is an integer (`uintptr_t`), not a pointer. `i4 + base` with
//     a pointer base is canonicalized to base-first and emits
//     `adds r0,r1,r0`; the integer add keeps the source order and emits the
//     ROM's `adds r0,r0,r1` (same precedent as Ai_AwardLeafGet's off + base).
//   * the doubled index is one expression, `i4 = (masked / 4) << 1`. Split
//     across two statements (`i4 = masked / 4; i4 <<= 1`, let alone a
//     `>> 2` shift), agbcc lowers the divide to `lsrs`; the single
//     expression keeps the ROM's arithmetic `asrs r0,r4,#2`.
//   * the return is one expression over the dereference and `tmp[sh]`.
//     Naming the value/mask locals reorders the tail (mask address before
//     the cell load, zero taken in r3, final shift kept in r2); the
//     expression form gives the ROM's load-first tail with the shift
//     amount in r1 (`lsls r1,r2,#1 / asrs r0,r1`).
extern u8 AiGridHalfBase[] __asm__("AiGridHalfBase");
int Ai_GridHalfwordGet(int id){
    u8 tmp[4];
    _0802E0A4((void *)tmp, (const void *)(uintptr_t)AI_GRID_MASK_TBL, 4u);
    int masked = id & 7;
    int sh = packed_shift(masked, 4);
    int sh2 = packed_shift(masked, 4);
    __asm__(".globl AiGridHalfBase\nAiGridHalfBase = 0x03001780\n");
    register uintptr_t base asm("r1");
    register u32 off asm("r3");
    register int i4 asm("r0");
    base = (uintptr_t)AiGridHalfBase;
    i4 = (masked / 4) << 1;
    off = 174;
    off = off << 3;
    base += off;
    return (*(s16 *)(i4 + base) & tmp[sh]) >> (sh2 << 1);
}
#ifndef __APPLE__
int _08025F20(int a) __attribute__((alias("Ai_GridHalfwordGet")));
#endif

#ifndef __APPLE__
int _080025E1C(int id) __attribute__((alias("Sub_08025E1C")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void _080025DBC(int type,int val) __attribute__((alias("Ai_GridSetPacked")));
void _080025E70(u32 a,int b,int c, u32 val) __attribute__((alias("Ai_GridWriteRecord")));
u32 _080025E98(u32 a,int b,int c) __attribute__((alias("Ai_GridReadRecord")));
int _080025F20(int id) __attribute__((alias("Ai_GridHalfwordGet")));
#endif
