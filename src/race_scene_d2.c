// ============================================================================
// race_scene_d2.c — reconstructed C for asm/race_scene.s (5 functions):
//
//   _08001EE2C (0x0801EE2C) — key/counter handler: setup latches + lane walk
//   _08001EFAC (0x0801EFAC) — big scene assembly: 7B18/7BFC chains + grid loops
//   _08001FA64 (0x0801FA64) — zero-terminated byte-stream 2ED0 emitter
//   _08001FADC (0x0801FADC) — counted byte-stream 2ED0 emitter (hidden s0 mode)
//   _08001FC14 (0x0801FC14) — scene setup: FBC switch + class branch + ctor chain
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#define HOST_STUB(sig) extern sig

#define D2_CALLEE(friendly, closure) closure
extern void *sub_08004B68(void);
extern void sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void sub_0802B214(int v);
extern int sub_0800572C(int v);
extern s16 sub_080258A8(int idx);

HOST_STUB(void _0802B368(u16 v));                                               // 0x0802B368 sound cue
HOST_STUB(void _08002B214(int v));                                              // 0x08002B214 sound wake
HOST_STUB(void *_08004B68(void));                                               // 0x08004B68 scene mgr
HOST_STUB(void _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h)); // 0x08007B18 emit
extern void sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i); // 0x08007BFC 9-arg emit
HOST_STUB(void _08002ED0(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j)); // 0x08002ED0 10 machine args
HOST_STUB(void _08007770(int a, void *b, int c, int d, u32 e, u32 f));         // 0x08007770 6 machine args
HOST_STUB(void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f));      // 0x08007770 exact ROM body
HOST_STUB(void _080075E8(void *a, int b, int c));                               // 0x080075E8 emit lane
HOST_STUB(void _0800798C(void *a, void *b));                                    // 0x0800798C resource setup
HOST_STUB(void _08007A58(void *p));                                             // 0x08007A58 resource init
HOST_STUB(void _08007ABC(void *a, void *b, int c));                             // 0x08007ABC resource bind
HOST_STUB(int _0800572C(int v));                                                // 0x0800572C IWRAM bump
HOST_STUB(s16 _0800258A8(int idx));                                             // 0x0800258A8 table lookup
HOST_STUB(int _080025F20(int a));                                               // 0x080025F20 grid count
HOST_STUB(void _080026A2C(void *a, s16 b, u16 c));                              // 0x080026A2C sprite XY
HOST_STUB(void _0800DBE8(void *p));                                             // 0x0800DBE8 record setup
HOST_STUB(void _0800DAB8(void *p));                                             // 0x0800DAB8 obj init
HOST_STUB(void _0800D77C(void *a, int b, int c));                               // 0x0800D77C slide setup
HOST_STUB(int _08001F9EC(int x));                                               // 0x08001F9EC s16 table 0x080CBE78
HOST_STUB(int _08001F9FC(int x));                                               // 0x08001F9FC s16 table 0x080CBE82
HOST_STUB(void _08001FA0C(void));                                               // 0x08001FA0C 64-lane template init
HOST_STUB(void _08001FE24(void *rec));                                          // 0x08001FE24 record setup (race_scene_e.c)
HOST_STUB(void _08001FF24(void *rec));                                          // 0x08001FF24 record setup (race_scene_e.c)
HOST_STUB(void _08001FF7C(void *rec));                                          // 0x08001FF7C record setup (race_scene_e.c)

// ---- shared anchors --------------------------------------------------------
#define WA 0x03001780u
// Symbol anchors keep the two constants pooled verbatim and added at run time
// (`ldr rX,=K` then `adds`), instead of folding K+delta into one pool word.
__asm__(".set D2WaBase, 0x03001780");
extern u8 D2WaBase;
__asm__(".set D2EntryTbl, 0x080CBBE4");
extern u8 D2EntryTbl;
#define D2_WA(o) ((u8 *)((uintptr_t)&D2WaBase + (uintptr_t)(o)))
#define D2_ENT(o) ((u8 *)((uintptr_t)&D2EntryTbl + (uintptr_t)(o)))
static inline u8  R8(volatile u8 *p, u32 o)  { return *(volatile u8 *)(p + o); }
static inline s8  S8(volatile u8 *p, u32 o)  { return *(volatile s8 *)(p + o); }
static inline u16 R16(volatile u8 *p, u32 o) { return *(volatile u16 *)(p + o); }
static inline s16 S16(volatile u8 *p, u32 o) { return *(volatile s16 *)(p + o); }
static inline u32 R32(volatile u8 *p, u32 o) { return *(volatile u32 *)(p + o); }
static inline void W8(volatile u8 *p, u32 o, u8 v)   { *(volatile u8 *)(p + o) = v; }
static inline void W16(volatile u8 *p, u32 o, u16 v) { *(volatile u16 *)(p + o) = v; }
static inline void W32(volatile u8 *p, u32 o, u32 v) { *(volatile u32 *)(p + o) = v; }

// ----------------------------------------------------------------------------
// ---- 0x08001EE2C — key/counter handler: setup latches + lane walk ----
// C, EXACT 384/384 on both gates (`make matching-slice`, `make full-rom-match`).
// Pools: {WA 0x03001780, lane table 0x080CBC24}. r0=rec, r1 dead, r2=(u16)c.
// Entry snapshots: prev142 = u16[rec+142], p136 = rec+136, prev136 = *p136.
//
// The measured levers, in the order they matter:
//  * `int c0` + a named `u16 k = (u16)c0;` keeps the narrowing after the rec
//    copy (`adds r4,r0` runs first). A `u16` parameter narrows ahead of every
//    statement and costs 4 bytes at +0x6.
//  * The second half re-reads the cell as `int v = *(s16 *)(rec + 136);` and
//    only then assigns `p`. Reading through `p` lets the allocator keep the
//    entry pointer and drop the ROM's fresh `adds r0,r4 / adds r0,#136` pair,
//    and an `s16` local keeps the value in HImode, emitting `ldrh` plus shifts
//    instead of the ROM's `ldrsh`.
//  * The two pool bases are pinned (`wa` r1, `tab` r2): unpinned, agbcc swaps
//    the two pool loads and the lookup block inverts. One `"+r"(wa)` laundry
//    is needed with them -- with neither variable laundered the loads reorder
//    and the body loses 8 bytes.
//  * In the `k == 1` arm the 10/0 materialisations interleave with their
//    stores; hoisting both to the top of the block swaps the first two and
//    costs 4 bytes.
//  * `*p = *p + n` (not a named sum) stores the halfword before the signed
//    narrow compare (`ldrh / add / strh / lsls / asrs / cmp`), the ROM's clamp.
//  * The `k == 64` arm's shared store is a `switch` on the signed field: its
//    two case bodies merge into one `strh` exactly as the ROM's do.
//  * `q` pinned to r3 is the ROM's tail copy of `p` (`adds r3,r2,#0`); the
//    clamps read and store through it. Unpinned, it coalesces into `p`.
//  * An empty asm after the `k == 32` arm and after the `k == 64` arm stops
//    cross-jump merging their stores into the later lanes' stores.
//  * The span ends in `00 00`, so the file-scope `.align 2, 0` after the body
//    supplies the zero fill instead of gas's Thumb `46 c0` nop.
// ----------------------------------------------------------------------------
void _08001EE2C(void *rec0, int b0, int c0) {
    u8 *rec;
    u16 k;
    u16 prev142;
    u16 prev136;
    u16 *p136;

    (void)b0;
    rec = (u8 *)rec0;
    k = (u16)c0;
    prev142 = *(u16 *)(rec + 142);
    p136 = (u16 *)(rec + 136);
    prev136 = *p136;

    if (k == 2) {
        u32 ten;
        u32 zero;
        ten = 10;
        __asm__ volatile ("" : "+r" (ten));
        *(u32 *)(rec + 24) = ten;
        zero = 0;
        __asm__ volatile ("" : "+r" (zero));
        *(u16 *)(rec + 28) = (u16)zero;
        *(u32 *)(rec + 56) = ten;
        *(u32 *)(rec + 52) = zero;
        _0802B368(4);
    }
    if (k == 1) {
        struct RecTab { u8 pad[144]; u16 tab[8]; };
        int v;
        v = *(s16 *)p136;
        if (v <= 7) {
            if (((struct RecTab *)rec)->tab[v] != 0) {
                u32 ten2;
                u32 zero2;
                ten2 = 10;
                *(u32 *)(rec + 24) = ten2;
                zero2 = 0;
                *(u16 *)(rec + 28) = (u16)zero2;
                *(u32 *)(rec + 56) = ten2;
                *(u32 *)(rec + 52) = k;
                *(u16 *)(rec + 138) = 4;
                {
                    register u8 *wa __asm__("r1") = (u8 *)0x03001780u;
                    register u16 *tab __asm__("r2") = (u16 *)0x080CBC24u;
                    int sv;
                    u16 val;
                    __asm__ volatile ("" : "+r" (wa));
                    sv = *(s16 *)p136;
                    val = tab[sv];
                    *(u16 *)(wa + 0xFC0) = val;
                }
                _0802B368(1);
            } else {
                _0802B368(10);
            }
        } else {
            if (v == 8) {
                u32 ten3;
                u32 zero3;
                ten3 = 10;
                *(u32 *)(rec + 24) = ten3;
                zero3 = 0;
                *(u16 *)(rec + 28) = (u16)zero3;
                *(u32 *)(rec + 56) = ten3;
                *(u32 *)(rec + 52) = zero3;
                _0802B368(1);
            }
        }
    }

    {
        int v = *(s16 *)(rec + 136);
        u16 *p = (u16 *)(rec + 136);
        if (k == 32) {
            if (*p == 8) {
                *p = 3;
            } else {
                v = v - 4;
                if (v >= 0) *p = (u16)v;
            }
        }
        __asm__ volatile ("");
        if (k == 16) {
            if (*p == 8) {
                *p = 7;
            } else {
                v = v + 4;
                if (v <= 7) *p = (u16)v;
            }
        }
        __asm__ volatile ("");
        {
            u16 *p142 = (u16 *)(rec + 142);
            if (k == 64) {
                if (*p == 8) {
                    switch (*(s16 *)p142) {
                    case 0: *p = 3; break;
                    case 1: *p = 7; break;
                    }
                } else {
                    *p = (u16)(*p - 1);
                    if (*(s16 *)p == 3) *p = 4;
                }
            }
            __asm__ volatile ("");
            if (k == 128) {
                *p = (u16)(*p + 1);
                if (*(s16 *)p == 4) *p = 8;
            }
            {
                register u16 *q __asm__("r3") = p;
                if (*q <= 3) *p142 = 0;
                if ((u16)(*q - 4) <= 3) *p142 = 1;
                if (*(s16 *)q <= 0) *q = 0;
                if (*(s16 *)q > 7) *q = 8;
                if (*p142 != prev142) {
                    _0802B368(3);
                } else {
                    if (*p != prev136) _0802B368(2);
                }
            }
        }
    }
}
// The ROM pads the 382-byte body with `00 00`; gas would close the section
// with the Thumb nop `46 c0`. A file-scope `.align 2, 0` lands after the
// body's `.size` in the same section and pads with the explicit zero fill.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// ---- 0x08001EFAC — big scene assembly: 7B18/7BFC chains + grid loops ----
// Pools: {0x080CBBE4 (8-byte entry table), WA 0x03001780 (s8 thresholds)}.
// r0=rec only. ROM high-register frame (r5=3, r6=1, sl=0 staging the
// shared (3,1,1,0) stack tail of the lane emits).
//
// C, EXACT 1448/1448 on both gates.
//
// The two grid preheaders are the load-bearing part (agbcc runs loop
// optimization twice, `flag_rerun_loop_opt`):
//  * The `u8 *wa = D2_WA(0);` at the head of each loop body is what puts the
//    D2WaBase pool load ahead of the row-base address in the movable list.
//    It is moved in pass 1 (move_movables lowers `threshold` by 3 per moved
//    move-insn), so the row base -- normally 13*4*4 = 208 >= insn_count --
//    drops to 10*4*4 = 160 and is deferred to the second pass. That pass
//    emits it after strength_reduce's 40/192 giv init, which is the ROM's
//    `3, 1, 40/0, sl` order. Without the named base the row base is first in
//    the list and hoists in pass 1, costing 28 bytes of preheader order.
//  * `(uintptr_t)(idx * 12) + (uintptr_t)wa` keeps the scaled index on the
//    left of the address add; the macro's symbol-first order emits
//    `adds r1,r2,r1` instead of the ROM's `adds r1,r1,r2` (6 bytes).
//  * The entry-table arm needs the "r" hint below (the ROM loads the pool
//    word into r3 before the index).
// ----------------------------------------------------------------------------
void _08001EFAC(void *rec0) {
    u8 *p;
    u8 *rec;
    u8 *tail;
    int j;

    __asm__(".globl D2WaBase\nD2WaBase = 0x03001780\n"
            ".globl D2EntryTbl\nD2EntryTbl = 0x080CBBE4\n");
    rec = (u8 *)rec0;
    _08007B18(rec + 60, 8, 72, 88, 6u, 3u, 1u, 0u);
    _080026A2C(*(void **)(rec + 268), 120, 56);

    _08007B18(rec + 16, 2, 8, 40, 3u, 1u, 1u, 0u);
    _08007B18(rec + 16, 8, 0, 64, 3u, 1u, 1u, 0u);
    _08007B18(rec + 16, 6, 0, 88, 3u, 1u, 1u, 0u);
    _08007B18(rec + 16, 5, 16, 112, 3u, 1u, 1u, 0u);
    _08007B18(rec + 16, 3, 192, 40, 3u, 1u, 1u, 0u);
    _08007B18(rec + 16, 7, 200, 64, 3u, 1u, 1u, 0u);
    _08007B18(rec + 16, 4, 200, 88, 3u, 1u, 1u, 0u);
    _08007B18(rec + 16, 1, 192, 112, 3u, 1u, 1u, 0u);

    sub_08007BFC(rec + 8, (int)*(u32 *)(rec + 160), (int)*(u32 *)(rec + 164), 48,
                 40, (int)*(u32 *)(rec + 168), 1, 1, 0);
    sub_08007BFC(rec + 8, (int)*(u32 *)(rec + 172), (int)*(u32 *)(rec + 176), 48,
                 64, (int)*(u32 *)(rec + 180), 1, 1, 0);
    sub_08007BFC(rec + 8, (int)*(u32 *)(rec + 184), (int)*(u32 *)(rec + 188), 48,
                 88, (int)*(u32 *)(rec + 192), 1, 1, 0);
    sub_08007BFC(rec + 8, (int)*(u32 *)(rec + 196), (int)*(u32 *)(rec + 200), 48,
                 112, (int)*(u32 *)(rec + 204), 1, 1, 0);
    sub_08007BFC(rec + 8, (int)*(u32 *)(rec + 208), (int)*(u32 *)(rec + 212), 168,
                 40, (int)*(u32 *)(rec + 216), 1, 1, 0);
    sub_08007BFC(rec + 8, (int)*(u32 *)(rec + 220), (int)*(u32 *)(rec + 224), 168,
                 64, (int)*(u32 *)(rec + 228), 1, 1, 0);
    sub_08007BFC(rec + 8, (int)*(u32 *)(rec + 232), (int)*(u32 *)(rec + 236), 168,
                 88, (int)*(u32 *)(rec + 240), 1, 1, 0);
    sub_08007BFC(rec + 8, (int)*(u32 *)(rec + 244), (int)*(u32 *)(rec + 248), 168,
                  112, (int)*(u32 *)(rec + 252), 1, 1, 0);

    p = rec + 136;
    if (*(u16 *)p != 8u) {
        /* The ROM loads the entry table's base into r3 before the index, so
           the symbol must already be in a register at this point. */
        __asm__ volatile("" : : "r"((u32)(uintptr_t)D2_ENT(0)));
        _08007B18(rec, 10,
                  (int)*(u32 *)(D2_ENT(0) + (s32)*(s16 *)p * 8),
                  (int)*(u32 *)(D2_ENT(0) + (s32)*(s16 *)p * 8 + 4),
                  4u, 1u, 1u, 0u);
    }
    if (*(u16 *)p == 8u) {
        _08007B18(rec, (int)*(u16 *)(rec + 138), 104, 128, 5u, 1u, 1u, 0u);
    }

    j = 0;
    tail = rec + 24;
    do {
        u8 *wa = D2_WA(0);
        if (j < _08025F20(0)) {
            int idx = (int)*(s16 *)(rec + 130);
            u8 *row = (u8 *)((uintptr_t)((u32)idx * 12u) + (uintptr_t)wa);
            if (j < (int)*(s8 *)(row + 50u))
                _08007B18(rec, 11, 40 - (j << 3), 48, 3u, 1u, 1u, 0u);
            else
                _08007B18(rec, 12, 40 - (j << 3), 48, 3u, 1u, 1u, 0u);
        }
        if (j < _08025F20(1)) {
            int idx = (int)*(s16 *)(rec + 130);
            u8 *row = (u8 *)((uintptr_t)((u32)idx * 12u) + (uintptr_t)wa);
            if (j < (int)*(s8 *)(row + 54u))
                _08007B18(rec, 11, 40 - (j << 3), 72, 3u, 1u, 1u, 0u);
            else
                _08007B18(rec, 12, 40 - (j << 3), 72, 3u, 1u, 1u, 0u);
        }
        if (j < _08025F20(2)) {
            int idx = (int)*(s16 *)(rec + 130);
            u8 *row = (u8 *)((uintptr_t)((u32)idx * 12u) + (uintptr_t)wa);
            if (j < (int)*(s8 *)(row + 52u))
                _08007B18(rec, 11, 40 - (j << 3), 96, 3u, 1u, 1u, 0u);
            else
                _08007B18(rec, 12, 40 - (j << 3), 96, 3u, 1u, 1u, 0u);
        }
        if (j < _08025F20(3)) {
            int idx = (int)*(s16 *)(rec + 130);
            u8 *row = (u8 *)((uintptr_t)((u32)idx * 12u) + (uintptr_t)wa);
            if (j < (int)*(s8 *)(row + 55u))
                _08007B18(rec, 11, 40 - (j << 3), 120, 3u, 1u, 1u, 0u);
            else
                _08007B18(rec, 12, 40 - (j << 3), 120, 3u, 1u, 1u, 0u);
        }
        j++;
    } while (j <= 2);

    j = 0;
    do {
        u8 *wa = D2_WA(0);
        if (j < _08025F20(4)) {
            int idx = (int)*(s16 *)(rec + 130);
            u8 *row = (u8 *)((uintptr_t)((u32)idx * 12u) + (uintptr_t)wa);
            if (j < (int)*(s8 *)(row + 53u))
                _08007B18(rec, 11, 192 + (j << 3), 48, 3u, 1u, 1u, 0u);
            else
                _08007B18(rec, 12, 192 + (j << 3), 48, 3u, 1u, 1u, 0u);
        }
        if (j < _08025F20(5)) {
            int idx = (int)*(s16 *)(rec + 130);
            u8 *row = (u8 *)((uintptr_t)((u32)idx * 12u) + (uintptr_t)wa);
            if (j < (int)*(s8 *)(row + 57u))
                _08007B18(rec, 11, 192 + (j << 3), 72, 3u, 1u, 1u, 0u);
            else
                _08007B18(rec, 12, 192 + (j << 3), 72, 3u, 1u, 1u, 0u);
        }
        if (j < _08025F20(6)) {
            int idx = (int)*(s16 *)(rec + 130);
            u8 *row = (u8 *)((uintptr_t)((u32)idx * 12u) + (uintptr_t)wa);
            if (j < (int)*(s8 *)(row + 51u))
                _08007B18(rec, 11, 192 + (j << 3), 96, 3u, 1u, 1u, 0u);
            else
                _08007B18(rec, 12, 192 + (j << 3), 96, 3u, 1u, 1u, 0u);
        }
        if (j < _08025F20(7)) {
            int idx = (int)*(s16 *)(rec + 130);
            u8 *row = (u8 *)((uintptr_t)((u32)idx * 12u) + (uintptr_t)wa);
            if (j < (int)*(s8 *)(row + 56u))
                _08007B18(rec, 11, 192 + (j << 3), 120, 3u, 1u, 1u, 0u);
            else
                _08007B18(rec, 12, 192 + (j << 3), 120, 3u, 1u, 1u, 0u);
        }
        j++;
    } while (j <= 2);

    _0800DBE8(tail);
}

void _08001FA64(void *dst0, int b, const u8 *tbl0, int d) {
    volatile u8 *dst = (volatile u8 *)dst0;
    int bb = b;
    int dd = d;
    const u8 *tbl = (const u8 *)tbl0;
    const s16 *tab2;
    if (*tbl == 0)
        return;
    tab2 = (const s16 *)0x0203F8F2u;
    for (;;) {
        int s = (int)*(const s16 *)(0x080CBC74u + 2u * (u32)*tbl);
        if (s != -1) {
            int idx = s - 1;
            int w = (int)*(const s16 *)((const u8 *)tab2 + 2u * (u32)idx);
            _08002ED0((void *)dst, bb, w, dd, 1, 0, 1, 0, 0, 1);
            tbl += 1;
            dst += 8;
        }
        if (*tbl == 0)
            break;
    }
}

// asm/race_scene.s:10606-10608 defines BOTH `sub_08001FA64:` and
// `_08001FA64:` on this one body, and asm/race_scene.s:12006,12054,12174,
// 12289 call it as `bl sub_08001FA64`. A promoted entry's export must
// therefore carry both spellings; the `sub_` twin is a direct alias of the
// C body with the same signature (same pattern as _08001F918).
void sub_08001FA64(void *dst0, int b, const u8 *tbl0, int d) __attribute__((alias("_08001FA64")));

void _08001FADC(void *dst0, int b, const u8 *tbl0, int n, int mode) {
    const u8 *tbl = (const u8 *)tbl0;
    volatile u8 *dst;
    const s16 *tab2;
    int i;
    int one;
    int zero;
    if (n <= 0)
        return;
    tab2 = (const s16 *)0x0203F8F2u;
    dst = (volatile u8 *)dst0;
    one = 1;
    zero = 0;
    for (i = n; i != 0; i--) {
        int s = (int)*(const s16 *)(0x080CBC74u + 2u * (u32)*tbl);
        if (s != -1) {
            int idx = s - 1;
            int w = (int)*(const s16 *)((const u8 *)tab2 + 2u * (u32)idx);
            _08002ED0((void *)dst, b, w, mode, one, zero, one, zero, zero, one);
            tbl += 1;
        }
        dst += 8;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x08001FC14 — scene setup: FBC switch + class branch + ctor chain ----
// Pools: {WA+0xFBC (s16 switch key), 0x08344450 (template), 0x082A798C,
// 0x082DFBDC, WA+0x576, 0x08345F84}. r0=rec only.
// FBC table at 0x0801FC40 (mov pc): idx 0/1/4/5/7 -> FC60 (kind 3),
// idx 2 -> FC7C (kind 4), idx 3/6 -> FC90 (empty tail), idx >7 -> FC90.
// Class branch on s16[_08004B68+0]: 24 -> wake+kind7 lane, 25 ->
// kind13 lane (+3 into rec+24 when WA+0xFBC==2), else straight to ctor.
// Tail switch on s16[rec+24] is a ROM cmp-chain (0/2/3), not a table.
// ----------------------------------------------------------------------------
void _08001FC14(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    register int fbc __asm__("r0") = (int)WA;
    register int fbcOffset __asm__("r1") = 0xFBC;
    register volatile u32 *p128 __asm__("r1");
    int alloc;
    int w;
    __asm__ volatile("" : "+r"(fbc), "+r"(fbcOffset));
    fbc += fbcOffset;
    fbcOffset = 0;
    fbc = *(s16 *)(uintptr_t)(fbc + fbcOffset);
    switch (fbc) {
    case 0:
    case 1:
    case 4:
    case 5:
    case 7:
        D2_CALLEE(Sub_08007770, sub_08007770)(0, (void *)(uintptr_t)0x08344450u, 3, 0, 4, 1);
        break;
    case 2:
        D2_CALLEE(Sub_08007770, sub_08007770)(0, (void *)(uintptr_t)0x08344450u, 4, 0, 4, 1);
        break;
    default:
        break;
    }
    W32(rec, 104, 6);
    {
        volatile u8 *clsBase = (volatile u8 *)D2_CALLEE(_08004B68, sub_08004B68)();
        register int cls __asm__("r0");
        register int clsIndex __asm__("r1");
        clsIndex = 0;
        cls = *(s16 *)(uintptr_t)(clsBase + clsIndex);
        switch (cls) {
        case 24:
            D2_CALLEE(_08002B214, sub_0802B214)(51);
            W32(rec, 116, 7);
            p128 = (volatile u32 *)(rec + 128);
            __asm__ volatile("" : "+r"(p128));
            *p128 = 1;
            W16(rec, 24, 0);
            break;
        case 25:
            W32(rec, 116, 13);
            p128 = (volatile u32 *)(rec + 128);
            __asm__ volatile("" : "+r"(p128));
            *p128 = 0;
            W16(rec, 24, 2);
            fbc = (int)WA;
            fbcOffset = 0xFBC;
            __asm__ volatile("" : "+r"(fbc), "+r"(fbcOffset));
            fbc += fbcOffset;
            if (*(volatile u16 *)(uintptr_t)fbc == 2)
                W16(rec, 24, 3);
            break;
        }
    }
    _0800DAB8((void *)(rec + 32));
    _080075E8((void *)(uintptr_t)0x082A798Cu, 0, 7);
    _0800798C((void *)(uintptr_t)0x08344450u, (void *)rec);
    _08007A58((void *)rec);
    _0800798C((void *)(uintptr_t)0x082DFBDCu, (void *)(rec + 8));
    _080075E8((void *)(uintptr_t)0x082DFBDCu, 0, 6);
    alloc = D2_CALLEE(_0800572C, sub_0800572C)(12);
    W32(rec, 176, (u32)alloc);
    fbc = (int)WA;
    fbcOffset = 0x576;
    __asm__ volatile("" : "+r"(fbc), "+r"(fbcOffset));
    fbc += fbcOffset;
    fbcOffset = 0;
    fbc = *(s16 *)(uintptr_t)(fbc + fbcOffset);
    w = D2_CALLEE(_0800258A8, sub_080258A8)(fbc);
    {
        volatile u32 *out180 = (volatile u32 *)(rec + 180);
        s16 value180 = (s16)w;
        *out180 = (u32)(s32)value180;
        _08007ABC((void *)(uintptr_t)R32(rec, 12), (void *)(uintptr_t)(u32)(s32)value180, (int)R32(rec, 176));
    }
    _0800798C((void *)(uintptr_t)0x08345F84u, (void *)(rec + 16));
    alloc = D2_CALLEE(_0800572C, sub_0800572C)(6);
    W32(rec, 188, (u32)alloc);
    w = _08001F9EC(0);
    {
        volatile u32 *out192 = (volatile u32 *)(rec + 192);
        s16 value192 = (s16)w;
        *out192 = (u32)(s32)value192;
        _08007ABC((void *)(uintptr_t)R32(rec, 20), (void *)(uintptr_t)(u32)(s32)value192, (int)R32(rec, 188));
    }
    _0800798C((void *)(uintptr_t)0x08345F84u, (void *)(rec + 200));
    alloc = D2_CALLEE(_0800572C, sub_0800572C)(1);
    W32(rec, 200, (u32)alloc);
    w = _08001F9FC(0);
    {
        volatile u32 *out204 = (volatile u32 *)(rec + 204);
        s16 value204 = (s16)w;
        *out204 = (u32)(s32)value204;
        _08007ABC((void *)(uintptr_t)R32(rec, 20), (void *)(uintptr_t)(u32)(s32)value204, (int)R32(rec, 200));
    }
    _08001FA0C();
    W32(rec, 32, 0);
    W16(rec, 36, 1);
    _0800D77C((void *)(rec + 52), 0, -32);
    _0800D77C((void *)(rec + 44), 0, 160);
    W16(rec, 40, 6);
    W16(rec, 42, 5);
    {
        register int sel __asm__("r0");
        register int selOffset __asm__("r1") = 24;
        sel = *(s16 *)(uintptr_t)(rec + selOffset);
        switch (sel) {
        case 0:
            _08001FE24((void *)rec);
            break;
        case 2:
            _08001FF24((void *)rec);
            break;
        case 3:
            _08001FF7C((void *)rec);
            break;
        }
    }
}
