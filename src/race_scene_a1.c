// ============================================================================
// race_scene_a1.c — reconstructed C for asm/race_scene.s (10 functions,
// VMA order):
//
//   0x0801A44C (interior tail of sub_08001A294, label _08001A44C to return)
//   0x0801A768 0x0801B068 0x0801B27C 0x0801B364 0x08001B498
//   0x0801B730 (interior of sub_08001B498, label _08001B730 to return)
//   0x0801B77C 0x0801B8DC 0x0801B9AC
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#define HOST_STUB(sig) extern sig

HOST_STUB(void  _08002D974(const void *src, void *dst, u32 ctrl)); // 0x08002D974 CpuSet
HOST_STUB(void  _08002E0A4(void *d, const void *s, u32 n));        // 0x08002E0A4 memcpy
HOST_STUB(void *_08002E104(void *a, int b, u32 c));                // 0x08002E104 memset-like
HOST_STUB(void  _08018AA8(u32 mask, int set));                     // 0x08018AA8 flag set/clear
HOST_STUB(int   _08018ACC(u32 mask));                              // 0x08018ACC flag test
HOST_STUB(int   _08002BE8(int dummy));                             // 0x08002BE8 counter bump, s16 ret
HOST_STUB(void  _08002C48(int a, u16 b));                          // 0x08002C48 table math
HOST_STUB(int   _08002DE04(int n, int d));                         // 0x08002DE04 signed divide
HOST_STUB(u32   _08002DF6C(u32 n, u32 d));                         // 0x08002DF6C unsigned divide, quotient in r0 (R4: NOT remainder)
HOST_STUB(void  _0802DDD0(void *a, void *b, void *c));            // 0x0802DDD0 bx-r2 veneer
HOST_STUB(void  _0802DDCC(void *a, void *b));                     // 0x0802DDCC veneer
HOST_STUB(void  _08004090(void *a, u32 b, void *c, u32 d, int e)); // 0x08004090 5 machine args (r3 select + caller stack word)
HOST_STUB(void  _0800390C(int a, u32 b, const volatile u8 *c));    // 0x0800390C
HOST_STUB(u16   _08026150(int a, int b));                          // 0x08026150 award leaf (strh ret)
HOST_STUB(void  _080261E8(int a, int b));                          // 0x080261E8 award leaf
HOST_STUB(void  _0802612C(void));                                  // 0x0802612C
HOST_STUB(int   _08002A86C(void *rec, int b, int c));              // 0x08002A86C 3-arg record claim
HOST_STUB(void  _080026FF4(void *a, int b, int c, int d));         // 0x080026FF4 record init
HOST_STUB(void  _080270D8(u32 a, u32 b, int c));                   // 0x080270D8
HOST_STUB(void  _08027114(int v));                                 // 0x08027114
HOST_STUB(void  _08027210(int v));                                 // 0x08027210
HOST_STUB(void  _08002B500(int a, int b, int c));                  // 0x08002B500 sound select
HOST_STUB(void  _08002B368(int v));                                // 0x08002B368 sound cue
HOST_STUB(void  _08002B44C(void));                                 // 0x08002B44C
HOST_STUB(void  _08002B234(void));                                 // 0x08002B234
HOST_STUB(void  _08002B3A4(void));                                 // 0x08002B3A4
HOST_STUB(void  _08002B460(void));                                 // 0x08002B460
HOST_STUB(void  _08002B25C(void));                                 // 0x08002B25C
HOST_STUB(void  _08002B474(void));                                 // 0x08002B474
HOST_STUB(void  _08002B280(void));                                 // 0x08002B280
HOST_STUB(void  _080191BC(int v));                                 // 0x080191BC
HOST_STUB(void  _08019AEC(int v));                                 // 0x08019AEC
HOST_STUB(void  _08019A2C(int v));                                 // 0x08019A2C
HOST_STUB(int   _080098C8(u16 a, u16 b));                          // 0x080098C8
HOST_STUB(void  _080096A4(void *p));                               // 0x080096A4
HOST_STUB(void  _0800821C(void));                                  // 0x0800821C timed reset
HOST_STUB(void *_080240C4(void));                                  // 0x080240C4 ghost rec A
HOST_STUB(void *_080240D0(void));                                  // 0x080240D0 ghost rec P
HOST_STUB(void  _080240F8(u32 v));                                 // 0x080240F8
HOST_STUB(void  _080240EC(u32 v));                                 // 0x080240EC
HOST_STUB(void  _08024104(u32 v));                                 // 0x08024104 strh recA+2, word arg
HOST_STUB(void  _08024110(const void *s));                         // 0x08024110
HOST_STUB(void  _08001A4AC(void));                                 // 0x08001A4AC 10-way dispatcher (asm)
HOST_STUB(void  _08001B130(int v));                                // 0x08001B130 (asm)
HOST_STUB(void  _08001B1A4(int v));                                // 0x08001B1A4 6-way dispatcher (asm)
HOST_STUB(void  _08001B410(void));                                 // 0x08001B410 B-handler (asm)

// ---- shared anchors --------------------------------------------------------
#define WORK_AREA 0x03001780u
#define CTX       0x03004E20u
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WORK_AREA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WORK_AREA + (o)))
#define WA_U32(o) (*(volatile u32 *)(uintptr_t)(WORK_AREA + (o)))

// ----------------------------------------------------------------------------
// ---- 0x0801A44C — interior tail of sub_08001A294: flag-byte fold ----
// No pools or calls; 96-byte body.
//   +00 1c03  adds r3, r0, #0; r3 = dst copy, reused as the store base
//   +02 7888  ldrb r0, [r1, #2]; then #6, #4 — IMMEDIATE offset
//...  strb r0, [r3, #0..2]
//   +0E 78da  ldrb r2, [r3, #3]; accumulator lives in r2 for all five bits
//   +10 2007  movs r0, #7; index materialised in r0
//   +12 5608  ldrsb r0, [r1, r0]; SIGNED byte at src+7 — REGISTER offset
//   +14 2800  cmp r0, #0 / +16 d001 beq / +18 2001 movs r0,#1 / +1A 4302 orrs r2,r0
//   +1C 70da  strb r2, [r3, #3]
//   repeated for (src index, bit) = (3,2) (8,4) (9,8) (5,16), ending bx lr at +5E.
//
// Two non-obvious levers, both required for byte-exactness:
//  1. The flag test reads through a *non-volatile* `const s8 *`, while the three
//     header copies read through a non-volatile `const u8 *`. agbcc folds a
//     VOLATILE byte load to the immediate form `ldrb r0,[r1,#7]` (dropping the
//     sign); the plain `const s8 *` subscript is what yields the ROM's
//     two-instruction `movs r0,#7` + `ldrsb r0,[r1,r0]` re-materialisation.
//     The `u8` view must stay plain or the three header copies lose their
//     immediate offsets too. Neither view needs volatile: this is a tuning
//     block, not hardware, and the five indices are distinct so nothing is CSE'd.
//  2. The accumulator is `register... __asm__("r2")`. Without the pin agbcc
//     reloads into r0 and emits `ldrb r0,[r3,#3]` + `adds r2,r0,#0`, which is
//     2 bytes too long per bit block (candidate 108 B vs ROM 96 B). The pin
//     makes the volatile reload land directly in the OR destination.
// ----------------------------------------------------------------------------
void _08001A44C(void *dst_, const void *src_) {
    volatile u8 *dst = (volatile u8 *)dst_;
    const u8 *src = (const u8 *)src_;
    const s8 *cp = (const s8 *)src;
    register u8 b __asm__("r2");
    dst[0] = src[2];
    dst[1] = src[6];
    dst[2] = src[4];
    b = dst[3];
    if (cp[7] != 0)
        b |= 1u;
    dst[3] = (u8)b;
    b = dst[3];
    if (cp[3] != 0)
        b |= 2u;
    dst[3] = (u8)b;
    b = dst[3];
    if (cp[8] != 0)
        b |= 4u;
    dst[3] = (u8)b;
    b = dst[3];
    if (cp[9] != 0)
        b |= 8u;
    dst[3] = (u8)b;
    b = dst[3];
    if (cp[5] != 0)
        b |= 16u;
    dst[3] = (u8)b;
}

// ----------------------------------------------------------------------------
// ---- 0x0801A768 — race-scene setup: fills, flag arms, record loop, ----
// ---- award-lane series, veneer pickup, ghost-gated 28-byte copy ----
// Pools: {0x0500015A, 0x03002864, 0x05000006, WA+0x10BD/0x10FE/0x10C2/
//   0x10FC/0x10CA/0x10DC/0x1114, 0x080CBAA4, 0x03004E80, 0x05000047,
//   0x0805FB9E, 0x0805FBA6, 0x080CBB28, 0x03004EA4, CTX, 0x534,
//   0x080CBB2C, 0x54C}.
// r0=a (record), r1=b (zero-path flag), r2=c (0x8000-arm flag).
// NOTE: r4 holds `a` until the WA+0x10CA loop, then is reused as the loop
// counter whose final value lands in WA+0x10DC (0 here — single pass over
// the loop writes nothing else to r4 before the store).
// ----------------------------------------------------------------------------
void _08001A768(void *a_, int b, int c) {
    volatile u8 *a = (volatile u8 *)a_;
    register int bFlag __asm__("r5") = b;
    register int cFlag __asm__("sl") = c;
    volatile u8 *wa = (volatile u8 *)WORK_AREA;
    u32 zero = 0;
    _08002D974(&zero, (void *)a, 0x0500015Au);
    if (bFlag != 0) {
        _08018AA8(32u, 1);
    } else {
        u32 z2 = (u32)bFlag;
        _08002D974(&z2, (void *)(uintptr_t)0x03002864u, 0x05000006u);
    }
    if (*(volatile u8 *)(wa + 0x10BD) != 0) {
        s16 v = *(s16 *)(uintptr_t)(wa + 0x10FE);
        if (v > 15) {
            u32 t = *(volatile u32 *)(uintptr_t)(0x080CBAA4u + (u32)(v - 16) * 4u);
            volatile u8 *tab = (volatile u8 *)(uintptr_t)t;
            u8 val = tab[0];
            if (val != 0) {
                volatile u8 *base = a + 1184;
                volatile u16 *ctr = (volatile u16 *)(a + 102);
                u32 ti = 1;
                do {
                    *(volatile u8 *)(base + *(volatile s16 *)(a + 102)) = val;
                    *ctr = (u16)(*ctr + 1u);
                    val = tab[ti++];
                } while (val != 0);
            }
            {
                s16 c2 = *(volatile s16 *)(a + 102);
                *(volatile u32 *)(a + 164) = (u32)(120 - (int)c2 * 4);
                *(volatile u32 *)(a + 168) = 60;
            }
        }
    }
    if (cFlag != 0)
        _08018AA8(0x8000u, 1);
    if (*(volatile u8 *)(wa + 0x10C2) != 0) {
        _08018AA8(0x1000000u, 1);
        _08018AA8(0x2000000u, 1);
    }
    {
        s16 w = *(volatile s16 *)(wa + 0x10FC);
        if (w == 3 || w == 5) {
            *(volatile u32 *)(a + 48) = 0;
            *(volatile u32 *)(a + 52) = 0;
            *(volatile u8 *)(a + 127) = 0;
        }
    }
    {
        s16 n = *(volatile s16 *)(wa + 0x10CA);
        for (int i = 0; i < (int)n; i++) {
            u32 z = 0;
            _08002D974(&z, (void *)(uintptr_t)(0x03004E80u + (u32)i * 284u), 0x05000047u);
        }
    }
    _0802612C();
    {
        s16 w8[4];
        s16 w16[4];
        _08002E0A4((void *)w8, (const void *)(uintptr_t)0x0805FB9Eu, 8);
        _08002E0A4((void *)w16, (const void *)(uintptr_t)0x0805FBA6u, 8);
        for (int k = 0; k < 4; k++) {
            u16 q = _08026150((int)w8[k], 128);
            *(volatile u16 *)(a + 132 + (u32)k * 2u) = q;
            _080261E8((int)w8[k], (int)w16[k]);
        }
    }
    *(volatile u16 *)(a + 86) = _08026150(4, 32);
    _080261E8(4, 2);
    *(volatile u16 *)(a + 140) = _08026150(7, 64);
    _080261E8(7, 0);
    *(volatile u16 *)(a + 142) = _08026150(8, 64);
    _080261E8(8, 1);
    (void)_08026150(5, 32);
    _080261E8(5, 3);
    (void)_08026150(6, 32);
    _080261E8(6, 4);
    (void)_08026150(10, 70);
    _080261E8(10, 11);
    (void)_08026150(11, 2);
    _080261E8(11, 12);
    (void)_08026150(9, 51);
    _080261E8(9, 13);
    (void)_08026150(12, 16);
    _080261E8(12, 9);
    (void)_08026150(13, 16);
    _080261E8(13, 10);
    _08001A4AC();
    *(volatile u16 *)(a + 64) = 1;
    WA_U32(0x10DC) = 0;
    {
        u32 ctxval = *(volatile u32 *)(uintptr_t)CTX;
        void *tbl = *(void *volatile *)(uintptr_t)0x080CBB28u;
        ((void (*)(void *, void *))(uintptr_t)tbl)(
            (void *)(uintptr_t)0x03004EA4u,
            (void *)(uintptr_t)(ctxval + 1304u));
        {
            u32 p1 = *(volatile u32 *)(uintptr_t)(ctxval + 0x534u);
            u32 p2 = *(volatile u32 *)(uintptr_t)p1;
            *(volatile u8 *)(uintptr_t)(ctxval + 126u) = *(volatile u8 *)(uintptr_t)p2;
        }
        if ((u16)(WA_U16(0x1114) - 94) <= (u16)2)
            _08018AA8(0x80000000u, 1);
        if (*(volatile u16 *)(wa + 0x10FC) == 8) {
            void *init_callback = *(void *volatile *)(uintptr_t)0x080CBB2Cu;
            ((void (*)(void *))(uintptr_t)init_callback)((void *)(uintptr_t)0x03004EA4u);
            ((void (*)(void *, void *))(uintptr_t)tbl)(
                (void *)(uintptr_t)0x03004EA4u,
                (void *)(uintptr_t)(ctxval + 1304u));
            {
                u32 *src = *(u32 *volatile *)(uintptr_t)(ctxval + 0x534u);
                volatile u32 *dst = (volatile u32 *)(uintptr_t)(ctxval + 0x54Cu);
                for (int i = 0; i < 7; i++)
                    dst[i] = src[i];
            }
        }
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801B068 — start/select arm + record init + racer walk ----
// Pools: {CTX, WA+0x10FE/0x10CA/0x10FC, 0x03004E80}.
// r0=a: nonzero → flag-4 set path (096A4 over [ctx]+860, u16[[ctx]+122]=5);
// zero → flag-4 clear path (026FF4 over ctx+0x41C, 284-stride 270D8 walk,
// 27210 select on u16[WA+0x10FC]==5).
//
// Four constraints, each carried by a distinct lever:
// - then-path: the ROM holds the CTX *address* (pool word 0x03004E20) in r4
//   from its pool load through both reloads, so the `ldr r0,[r4]` / `adds`
//   pair re-reads the cell rather than the memoised value. Left unpinned,
//   agbcc dereferences into r4 and keeps the pooled address in r0, which
//   also moves the u16 store's constant from r1 to r0 and its address off
//   r0 (4 instructions). Pin the cell to r4; re-dereference before +122.
// - else-path: WORK_AREA is reached as base+offset from TWO pool words
//   (`ldr r2,=0x03001780` / `ldr r3,=0x10FE` / `adds r1,r2,r3`), once per
//   offset, with the base live in r2 across the 026FF4/270D8 calls. A
//   folded `WORK_AREA + 0x10FE` pools as the single word 0x0300277E and
//   loses both the adds and the shared base. Each operand is therefore its
//   own assembler absolute symbol (docs/findings/track_car_26180_pool_order.md).
// - the two halfwords are NON-volatile s16 loads: agbcc lowers a volatile
//   signed halfword to ldrh+lsls+asrs, and a plain one to `movs rX,#0` +
//   `ldrsh rD,[rB,rX]`, which is the ROM's shape at +0x4A and +0x7C.
// - the walk re-reads its bound from *pca in the condition (guard via r4
//   before the body repurposes r4 as the 03004E80 runner, back-edge via the
//   r6 copy the allocator makes), so the bound must stay a memory read, not
//   a cached `s16 n` local. The r6 copy itself needs the source split into
//   `s16 *cnt = pca;` -- with the loop reading `*pca` directly agbcc shares
//   ONE test (entry `b` + back-edge `blt`) instead of the ROM's duplicated
//   guard+back-edge pair, which is 4 instructions of layout.
//
// Probe: PARTIAL 183/200, first difference +0x4A. The whole then-path
// (0x00-0x2C) and the whole walk (0x5C-0x86 on) are byte-identical; what is
// left is a 5-halfword reorder in the two-argument pair: the ROM emits
// `ldrsh r1,[r1,r3]` (a2) BEFORE `ldr r3,=0x10CA` / `adds r4,r2,r3` (pca),
// while agbcc emits `ldr r3,=0x10CA` / `adds r4,r2,r3` (pca) BEFORE the a2
// load and then has to give that load's zero index r2 instead of r3 (r3
// holds the pca offset by then).
//
// WHERE the reorder happens is verified, and it is NOT expansion: an `-dr`
// RTL dump of this TU (build/era-corpus/ready-work/race_scene_a1.i.rtl, the
// pre-reload dump) has the statements in exact source order --
//   insn 96 (set reg40 (zero_extend (mem:HI (reg39))))   @ a2 read
//   insn 98 (set reg33 reg40)
//   insn 101 (set r3 (mem/u *.LC3))                      @ off = WaOffCA
//   insn 103 (set reg32 (plus r2 r3))                    @ pca
//   insn 106 (set reg41 (zero_extend (mem:HI (reg32))))  @ a3 read
// so the a2 LOAD really does sink past the pca group, later in the pipeline
// (reload is the only pass left that can place it; `-fno-regmove`,
// `-fno-optimize-register-move`, `-fno-caller-saves`,
// `-fno-expensive-optimizations`, `-fno-inline` and `-fno-defer-pop` all
// leave the emitted order unchanged, so it is not any of those). The index
// register is the tell: the ROM reuses the dying r3 for BOTH loads' zero
// index, the candidate is only offered r2 at the a2 load because pca's
// `adds r4,r2,r3` has already killed wb. pret/agbcc carries no insn
// scheduler (`flag_schedule_insns` does not exist in its tree), so nothing
// at the C level can move an insn back -- an empty `asm` on the value only
// changes the load width (ldrh) or drops the guard.
// ----------------------------------------------------------------------------
void _08001B068(int a) {
    if (a != 0) {
        _08018AA8(4u, 1);
        {
            register u32 cell __asm__("r4");
            cell = CTX;
            _080096A4((void *)(uintptr_t)(*(volatile u32 *)(uintptr_t)cell + 860u));
            *(volatile u16 *)(uintptr_t)(*(volatile u32 *)(uintptr_t)cell + 122u) = 5;
        }
        _08002B460();
        _08002B25C();
    } else {
        extern u8 WaBase[], WaOffFE[], WaOffCA[], WaOffFC[];
        u32 rec;
        s16 *pca;
        s16 a2, a3;
        register u8 *wb __asm__("r2");
        register u32 off __asm__("r3");
        register int i __asm__("r5");
        __asm__(".globl WaBase\nWaBase = 0x03001780\n");
        __asm__(".globl WaOffFE\nWaOffFE = 0x10FE\n");
        __asm__(".globl WaOffCA\nWaOffCA = 0x10CA\n");
        __asm__(".globl WaOffFC\nWaOffFC = 0x10FC\n");
        _08018AA8(4u, 0);
        /* Argument order is also instruction order here: the ROM builds
         * r0 = [CTX]+0x41C before it pools the WORK_AREA base, so the
         * caller's first argument must be evaluated first. */
        /* The record offset must be a SYMBOL, not the literal 0x41C: a
         * pinned `off = 0x41C` is copy-propagated away and the pool load
         * lands in r2 (the ROM's r3 is only reachable while `off` still has
         * a live range). */
        { extern u8 RecOff41C[];
          __asm__(".globl RecOff41C\nRecOff41C = 0x41C\n");
          rec = *(volatile u32 *)(uintptr_t)CTX;
          off = (u32)(uintptr_t)RecOff41C;
          rec = rec + off; }
        wb = (u8 *)(uintptr_t)WaBase;
        off = (u32)(uintptr_t)WaOffFE;
        a2 = *(s16 *)(uintptr_t)((uintptr_t)wb + off);
        off = (u32)(uintptr_t)WaOffCA;
        pca = (s16 *)(uintptr_t)((uintptr_t)wb + off);
        a3 = *pca;
        _080026FF4((void *)(uintptr_t)rec, (int)a2, (int)a3, 0);
        /* Written as the rotated form the ROM carries EXACTLY: the entry
         * guard reads the bound through r4 (the register the bound pointer
         * holds from its pool add), then the 03004E80 runner claims r4 for
         * the body and the allocator copies the bound pointer to r6 at the
         * loop entry (`adds r6,r4,#0`), which is what the back-edge test
         * reads. A plain `for` here makes agbcc share one test instead of
         * duplicating it (entry `b` + back-edge `blt`) and the guard read
         * ends up in the same register as the tail. */
        i = 0;
        if (i < (int)*pca) {
            s16 *cnt = pca;
            u32 *q = (u32 *)0x03004E80u;
            do {
                _080270D8(q[0], q[1], i);
                q = (u32 *)(uintptr_t)((u8 *)q + 284u);
                i++;
            } while (i < (int)*cnt);
        }
        if (*(volatile u16 *)(uintptr_t)((uintptr_t)WaBase + (uintptr_t)WaOffFC) == 5)
            _08027210(0);
        else
            _08027210(1);
        _08002B474();
        _08002B280();
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801B27C — key-gated B068/flag driver + counter cell machine ----
// Pools: {WORK_AREA, 0x10C2, CTX}. r0 dead, r1 = u16 key (truncated on entry
// into r4, which stays live for the whole body).
// Flag masks are 0x100000 / 0x200000 for the key==8 arm and 0x1000000 /
// 0x2000000 for the counter-cell arm — each materialised as `movs r0,#128`
// plus a single lsls, which is what fixes the shift amounts at 13/14/17/18.
// The work-area byte is reached as base + 0x10C2 from TWO pool words, so it
// must be an extern object indexed by a constant, not a folded integer.
// The counter cell is at CTX+110, read back signed through a struct field so
// the load is `movs r2,#0 / ldrsh r1,[r1,r2]` rather than ldrh+lsls+asrs.
// ----------------------------------------------------------------------------
void _08001B27C(int unused, u16 key) {
    struct Cell { s16 v; };
    extern volatile u8 RaceWA_1B27C[];
    (void)unused;
    __asm__("RaceWA_1B27C = 0x03001780");
    if (_08018ACC(0x100000u) != 0 && _08018ACC(8u) != 0) {
        _08001B068(1);
    } else if (key == 8 && _08018ACC(8u) != 0) {
        _08018AA8(0x100000u, 1);
        _08018AA8(0x200000u, 1);
    }
    if (((u32)key & 4u) != 0 && RaceWA_1B27C[0x10C2u] != 0) {
        /* The ROM holds the CTX *address* (pool word 0x03004E20) in r0 from the
         * `ldr r0,=CTX` at 0x2D6 through to the reload `ldr r0,[r0]` at 0x308,
         * so r0 is live across the whole cell machine. Left to itself agbcc's
         * local-alloc gives the base the last free register (r2), which also
         * costs the ldrsh its dead-temp index register (r3, not r2) and pushes
         * the switch value into r0. The pin is the same `register T v
         * __asm__("rN")` lever src/agbmain_session.c:110 documents. */
        register volatile u32 *ctxp __asm__("r0") = (volatile u32 *)(uintptr_t)CTX;
        register struct Cell *cell __asm__("r1") =
            (struct Cell *)(uintptr_t)(*ctxp + 110u);
        s16 v;
        *(volatile u16 *)(uintptr_t)cell = (u16)(*(volatile u16 *)(uintptr_t)cell + 1u);
        v = cell->v;
        switch (v) {
        case 0:
        case 3:
            *(volatile u16 *)(uintptr_t)(*ctxp + 110u) = 0;
            _08018AA8(0x1000000u, 1);
            _08018AA8(0x2000000u, 1);
            break;
        case 1:
            _08018AA8(0x1000000u, 0);
            _08018AA8(0x2000000u, 1);
            break;
        case 2:
            _08018AA8(0x1000000u, 0);
            _08018AA8(0x2000000u, 0);
            break;
        default:
            break;
        }
    }
    if (((u32)key & 9u) != 0 && _08018ACC(32u) != 0)
        _08018AA8(2u, 1);
}
__asm__(".align 2, 0");

// The 0x0801B27C span carries a twin label pair in the closure
// (asm/race_scene.s:1803 `sub_08001B27C:` immediately followed by
// `_08001B27C:`), and retained asm still calls the `sub_` spelling at
// asm/race_scene.s:1994 (`bl sub_08001B27C`). The splice deletes both labels
// and re-emits only a `.thumb_set` agbcc actually wrote, so C must define the
// `sub_` spelling or the independent link fails with an undefined reference.
// Alias the real body in ONE hop (chaining through a second alias is a known
// link failure).
void sub_08001B27C(int unused, u16 key) __attribute__((alias("_08001B27C")));

// ----------------------------------------------------------------------------
// ---- 0x0801B364 — two-u16 event router (ev9 body: 1B1A4/1B27C) ----
// Pools: {WA+0x10FC, CTX}. r0/r1 truncated to u16 on entry.
// WA+0x10FC==8 with positive s16[[ctx]+106] returns at once; else
// ACC(4) picks B1A4 (flag-0x8000 clear or flag-0x4000 set → B130(0)+2;
// else ACC(0x4000) re-test then [ctx]+122 decrement or 098C8) vs B27C.
// ----------------------------------------------------------------------------
void _08001B364(u16 a, u16 b) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA;
    u32 ctxval = *(volatile u32 *)(uintptr_t)CTX;
    if (*(volatile u16 *)(wa + 0x10FC) != 8 ||
        *(volatile s16 *)(uintptr_t)(ctxval + 106u) <= 0) {
        if (_08018ACC(4u) != 0) {
            int take130 = 0;
            if (*(volatile u16 *)(wa + 0x10FC) == 8) {
                if (_08018ACC(0x8000u) != 0)
                    take130 = 1;
                else if (_08018ACC(0x4000u) != 0)
                    take130 = 1;
            }
            if (take130 != 0) {
                _08001B130(0);
                _08001B1A4(2);
            } else {
                /* L3C8 re-test (second ACC4000 call on the w==8 fallthrough) */
                int sel = 0;
                int t = _08018ACC(0x4000u);
                if (t == 0 && *(volatile u16 *)(wa + 0x10FC) != 8) {
                    if (*(volatile s16 *)(uintptr_t)(ctxval + 122u) > 0)
                        *(volatile u16 *)(uintptr_t)(ctxval + 122u) =
                            (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 122u) - 1u);
                    else
                        sel = _080098C8(a, b);
                }
                _08001B1A4(sel);
            }
        } else {
            _08001B27C((int)a, b);
        }
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801B498 — 6-way select dispatcher + case bodies ----
// Pools: {WA+0x10FC, CTX, 0x03004EA4, table 0x0801B550, WA+0x10FE/0x1114/
//   0x10F0}. r0=sel (u16 store to [[ctx]+88]).
// Guards: WA+0x10FC==8 && WA+0x10C3==0 && sel==5 && [[ctx]+128]==0 clears
// flag 1; ACC(1) nonzero returns; else flag 1/128/8 arms, two 2A86C claims
// (8/16), [[ctx]+78]=0, WA+0x10FC==5 with positive [ctx+48] timed reset,
// then sel 0..5 dispatches (switch = table 0x0801B550).
// ----------------------------------------------------------------------------
void _08001B498(int sel) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA;
    u32 ctxval = *(volatile u32 *)(uintptr_t)CTX;
    if (*(volatile u16 *)(wa + 0x10FC) == 8 &&
        *(volatile u8 *)(wa + 0x10C3) == 0 &&
        sel == 5 &&
        *(volatile u8 *)(uintptr_t)(ctxval + 128u) == 0)
        _08018AA8(1u, 0);
    if (_08018ACC(1u) != 0)
        return;
    *(volatile u16 *)(uintptr_t)(ctxval + 88u) = (u16)sel;
    _08018AA8(1u, 1);
    _08018AA8(128u, 0);
    _08018AA8(8u, 0);
    _08002A86C((void *)(uintptr_t)0x03004EA4u, 8, 0);
    _08002A86C((void *)(uintptr_t)0x03004EA4u, 16, 0);
    *(volatile u16 *)(uintptr_t)(ctxval + 78u) = 0;
    if (*(volatile u16 *)(wa + 0x10FC) == 5 &&
        *(volatile s32 *)(uintptr_t)(ctxval + 48u) > 0)
        _0800821C();
    if (sel < 0 || sel > 5)
        return;
    switch (sel) { /* jump table 0x0801B550 */
    case 0:
        _08002A86C((void *)(uintptr_t)0x03004EA4u, 4, 1);
        if (_08018ACC(32u) != 0)
            break;
        _08018AA8(512u, 1);
        _08002B44C();
        _08002B234();
        _08002B368(64);
        _08027114(-2);
        {
            s16 w = *(volatile s16 *)(wa + 0x10FC);
            if (w == 1 || w == 4)
                _08019AEC((int)*(volatile s16 *)(uintptr_t)(0x03004E80u + 14u));
            else
                _08019AEC(1);
        }
        if (*(volatile u16 *)(wa + 0x10FC) != 3)
            break;
        if (_08018ACC(0x1000u) != 0)
            break;
        {
            u8 *ra = (u8 *)_080240C4();
            u8 *rp = (u8 *)_080240D0();
            _08024104((u16)*(volatile s16 *)(wa + 0x10FE));
            _080240F8(*(volatile u32 *)(uintptr_t)(ctxval + 24u));
            _080240EC(*(volatile u32 *)(uintptr_t)(ctxval + 44u));
            _08024110((const void *)(wa + 0x1114));
            if (*(volatile u16 *)(void *)rp == 0 ||
                *(volatile u16 *)(void *)(ra + 2) != *(volatile u16 *)(void *)(rp + 2) ||
                *(volatile u32 *)(void *)(ra + 4) < *(volatile u32 *)(void *)(rp + 4))
                _08018AA8(0x8000u, 1);
        }
        break;
    case 1:
        _08002A86C((void *)(uintptr_t)0x03004EA4u, 2, 1);
        if (_08018ACC(32u) != 0)
            break;
        _08018AA8(512u, 1);
        _08002B44C();
        _08002B234();
        _08027114(-2);
        _08019AEC(1);
        _08002B368(64);
        break;
    case 2:
    case 3:
        _08002A86C((void *)(uintptr_t)0x03004EA4u, 2, 1);
        _08002B44C();
        _08002B234();
        _08027114(-1);
        break;
    case 4:
        _08002A86C((void *)(uintptr_t)0x03004EA4u, 4, 1);
        _08002B44C();
        _08002B234();
        _08002B368(64);
        *(volatile u16 *)(uintptr_t)(0x03004E80u + 14u) = 1;
        _08019AEC(1);
        break;
    case 5:
        _08002A86C((void *)(uintptr_t)0x03004EA4u, 2, 1);
        _08002B44C();
        _08002B234();
        _08002B3A4();
        *(volatile u16 *)(uintptr_t)(0x03004E80u + 14u) = 2;
        _08019AEC(2);
        WA_U32(0x10F0) = 0;
        *(volatile u8 *)(uintptr_t)(ctxval + 128u) = 1;
        break;
    }
}

// Spelling aliases for the 0x08001B498 event dispatcher (race_scene.c uses
// the Sub_ spelling).
void sub_08001B498(int v) __attribute__((alias("_08001B498")));
void Sub_08001B498(int v) __attribute__((alias("_08001B498")));

// ----------------------------------------------------------------------------
// ---- 0x0801B730 — interior of sub_08001B498: threshold test leaf ----
// No pools. r0=record (word at +12), r1=flag cell (bit 1 tested).
// Returns 1 when the flag bit is set and (empty marker byte at
// [[CTX]+126] or s16[[CTX]+64]-1 below the scaled +12 word).
// ----------------------------------------------------------------------------
int _08001B730(void *rec_, void *cell_) {
    struct Cell { s16 v; };
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u32 *cell = (volatile u32 *)cell_;
    u32 ctxval;
    int d;
    register int v __asm__("r0");
    register int base __asm__("r1");
    if ((*cell & 2u) == 0)
        return 0;
    ctxval = *(volatile u32 *)(uintptr_t)CTX;
    if (*(volatile u8 *)(uintptr_t)(ctxval + 126u) == 0)
        return 1;
    d = (int)((struct Cell *)(uintptr_t)(ctxval + 64u))->v - 1;
    base = (int)*(volatile u32 *)(uintptr_t)((uintptr_t)rec + 12u);
    v = base + 2048;
    if (v < 0)
        v = base + 0x17FF;
    v >>= 12;
    if (d >= v)
        return 0;
    return 1;
}

// ----------------------------------------------------------------------------
// ---- 0x0801B77C — grid paint: div-derived sweep over +0x4B4 column ----
// Pools: {CTX, 0x0805FBB0, 0x4B4}. No incoming args.
// Count = DivSI(s16[[ctx]+100]*s16[[ctx]+102], 80)-1; four BE8/C48 pairs
// feed the sp+4 select words; each live lane picks a select word via the
// DF6C remainder and calls 04090 (5 machine args: r3 select + stack 1)
// or 0390C. Tail writes the 0x10000/1536-scaled pair to [[ctx]+16/+20].
// ----------------------------------------------------------------------------
void _08001B77C(void) {
    u32 ctxval = *(volatile u32 *)(uintptr_t)CTX;
    s16 v100 = *(volatile s16 *)(uintptr_t)(ctxval + 100u);
    s16 v102 = *(volatile s16 *)(uintptr_t)(ctxval + 102u);
    int cnt0 = _08002DE04((int)((u32)(s32)v100 * (u32)(s32)v102), 80);
    u32 tmpl[4];
    u8 b36[2];
    int cnt;
    for (int i = 0; i < 4; i++)
        tmpl[i] = *(volatile u32 *)(uintptr_t)(0x0805FBB0u + (u32)i * 4u);
    _08002E104((void *)b36, 0, 2);
    if (_08018ACC(32u) != 0)
        goto tail;
    cnt = cnt0 - 1;
    {
        u32 rnd[4];
        for (int k = 0; k < 4; k++) {
            rnd[k] = (u32)(s32)(s16)_08002BE8(0);
            _08002C48((int)(s16)rnd[k], (u16)tmpl[k]);
        }
        if (cnt >= 0) {
            u32 r7 = (u32)cnt * 8u;
            int it = cnt;
            u8 *base = (u8 *)(uintptr_t)(ctxval + 0x4B4u);
            u8 *tbase = (u8 *)(uintptr_t)(ctxval + 1184u);
            u32 p164 = *(volatile u32 *)(uintptr_t)(ctxval + 164u);
            u32 p168 = *(volatile u32 *)(uintptr_t)(ctxval + 168u);
            while (it >= 0) {
                base[cnt] = 12;
                if (base[it] != 0)
                    base[it] = (u8)(base[it] - 1u);
                b36[0] = tbase[it];
                if (base[it] != 0) {
                    u32 quot = _08002DF6C((u32)base[it], 3u); // quotient: idx*4 into rnd
                    u32 sel = *(volatile u32 *)((volatile u8 *)rnd + ((quot << 24) >> 22));
                    _08004090((void *)(uintptr_t)(p164 + r7 - 4u), p168 - 8u,
                              (void *)b36, sel, 1);
                } else {
                    // race_scene.s:2505-2511 — r0 = p164+r7 (x), r1 = p168,
                    // r2 = &b36 (the string); see ObjLane_0390C.
                    _0800390C((int)(uintptr_t)(p164 + r7),
                              (u32)p168, (const volatile u8 *)b36);
                }
                r7 -= 8u;
                it--;
            }
        }
    }
tail:
    {
        u8 *ctxp = (u8 *)(uintptr_t)*(volatile u32 *)(uintptr_t)CTX;
        u16 w100 = *(volatile u16 *)(void *)(ctxp + 100);
        int q1 = _08002DE04((int)((u32)w100 << 16), 160);
        *(volatile u32 *)(void *)(ctxp + 16) = 0x10000u - (u32)q1;
        {
            s16 s100 = *(volatile s16 *)(void *)(ctxp + 100);
            int q2 = _08002DE04((int)s100 * 3 * 512, 160);
            int d2 = 1536 - q2;
            *(volatile u32 *)(void *)(ctxp + 20) = d2 < 0 ? 0u : (u32)d2;
        }
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801B8DC — +100/+104 state machine over B77C ----
// Pools: {CTX}. s16[[ctx]+100] ticks up while <= 159; s16[[ctx]+104]
// selects: 0 → ACC(32)-gated 19A2C(0); 1 → B77C + miteinander 79/ACC(32)
// gate to 19A2C(0); 2 → B77C + ACC(64)-gated u16[[ctx]+68]=40 +
// flag 0x100000; 3 → B77C + u16[[ctx]+68] tick-down or flag 0x8000.
// Every taken arm (except the pure tick-down) bumps u16[[ctx]+104].
// ----------------------------------------------------------------------------
void _08001B8DC(void) {
    u32 ctx = CTX;
    u32 ctxval = *(volatile u32 *)(uintptr_t)ctx;
    if (*(volatile s16 *)(uintptr_t)(ctxval + 100u) <= 159)
        *(volatile u16 *)(uintptr_t)(ctxval + 100u) =
            (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 100u) + 1u);
    {
        s16 st = *(volatile s16 *)(uintptr_t)(ctxval + 104u);
        int bump = 0;
        if (st == 1) {
            _08001B77C();
            if (*(volatile s16 *)(uintptr_t)(ctxval + 100u) > 79) {
                if (_08018ACC(32u) == 0)
                    _08019A2C(0);
                bump = 1;
            }
        } else if (st > 1) {
            if (st == 2) {
                _08001B77C();
                if (_08018ACC(64u) == 0 &&
                    *(volatile s16 *)(uintptr_t)(ctxval + 100u) > 159) {
                    *(volatile u16 *)(uintptr_t)(ctxval + 68u) = 40;
                    _08018AA8(0x100000u, 0);
                    bump = 1;
                }
            } else if (st == 3) {
                _08001B77C();
                if (*(volatile s16 *)(uintptr_t)(ctxval + 68u) > 0)
                    *(volatile u16 *)(uintptr_t)(ctxval + 68u) =
                        (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 68u) - 1u);
                else {
                    _08018AA8(0x8000u, 0);
                    bump = 1;
                }
            }
        } else if (st == 0) {
            /* L91A: both arms fall into L99A — the bump is unconditional */
            if (_08018ACC(32u) != 0)
                _08019A2C(0);
            bump = 1;
        }
        if (bump != 0)
            *(volatile u16 *)(uintptr_t)(ctxval + 104u) =
                (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 104u) + 1u);
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801B9AC — 6-way scene-state dispatcher (table 0x0801B9D4) ----
// Pools: {CTX}. No incoming args; returns out (1 only when case 0 finds
// non-positive s16[[ctx]+68]). s16[[ctx]+76] picks the arm; arms 1/2/3
// share the decrement-vs-sound shape (100/60/60 via B410 for case 1),
// arm 4 falls into the arm-5 store shape on empty, arm 5 stores 40.
// The LBB02 store path also ticks u16[[ctx]+76] down by one.
// ----------------------------------------------------------------------------
int _08001B9AC(void) {
    int out = 0;
    u32 ctxval = *(volatile u32 *)(uintptr_t)CTX;
    s16 st = *(volatile s16 *)(uintptr_t)(ctxval + 76u);
    if (st >= 0 && st <= 5) {
        switch (st) { /* jump table 0x0801B9D4 */
        case 0:
            if (_08018ACC(32u) == 0)
                _080191BC(3);
            if (*(volatile s16 *)(uintptr_t)(ctxval + 68u) > 0)
                *(volatile u16 *)(uintptr_t)(ctxval + 68u) =
                    (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 68u) - 1u);
            else
                out = 1;
            break;
        case 1:
            if (_08018ACC(32u) == 0)
                _080191BC(2);
            if (*(volatile s16 *)(uintptr_t)(ctxval + 68u) > 0) {
                *(volatile u16 *)(uintptr_t)(ctxval + 68u) =
                    (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 68u) - 1u);
            } else {
                _08002B500(8, 255, 0);
                _08001B410();
                *(volatile u16 *)(uintptr_t)(ctxval + 68u) = 100;
                *(volatile u16 *)(uintptr_t)(ctxval + 76u) =
                    (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 76u) - 1u);
            }
            break;
        case 2:
            if (_08018ACC(32u) == 0)
                _080191BC(1);
            if (*(volatile s16 *)(uintptr_t)(ctxval + 68u) > 0) {
                *(volatile u16 *)(uintptr_t)(ctxval + 68u) =
                    (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 68u) - 1u);
            } else {
                _08002B500(7, 255, 0);
                *(volatile u16 *)(uintptr_t)(ctxval + 68u) = 60;
                *(volatile u16 *)(uintptr_t)(ctxval + 76u) =
                    (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 76u) - 1u);
            }
            break;
        case 3:
            if (_08018ACC(32u) == 0)
                _080191BC(0);
            if (*(volatile s16 *)(uintptr_t)(ctxval + 68u) > 0) {
                *(volatile u16 *)(uintptr_t)(ctxval + 68u) =
                    (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 68u) - 1u);
            } else {
                _08002B500(6, 255, 0);
                *(volatile u16 *)(uintptr_t)(ctxval + 68u) = 60;
                *(volatile u16 *)(uintptr_t)(ctxval + 76u) =
                    (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 76u) - 1u);
            }
            break;
        case 4:
            if (*(volatile s16 *)(uintptr_t)(ctxval + 68u) > 0) {
                *(volatile u16 *)(uintptr_t)(ctxval + 68u) =
                    (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 68u) - 1u);
            } else {
                _08002B500(5, 255, 0);
                *(volatile u16 *)(uintptr_t)(ctxval + 68u) = 60;
                *(volatile u16 *)(uintptr_t)(ctxval + 76u) =
                    (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 76u) - 1u);
            }
            break;
        case 5:
            *(volatile u16 *)(uintptr_t)(ctxval + 68u) = 40;
            *(volatile u16 *)(uintptr_t)(ctxval + 76u) =
                (u16)(*(volatile u16 *)(uintptr_t)(ctxval + 76u) - 1u);
            break;
        }
    }
    return out;
}

// ROM entry alias.
void RaceScene_B364(u16 a, u16 b) __attribute__((alias("_08001B364")));
void Sub_0801B498(int sel) __attribute__((alias("_08001B498")));
