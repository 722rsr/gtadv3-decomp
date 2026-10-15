// ============================================================================
// Scene and record dispatch, including the race FSM at 0x0800AA40 and
// course phase machine at 0x08008CA0. Assembly owners:
//
//   asm/menu_progress_update.s   0x0800BD40 (results progress updater)
//   asm/menu_place.s             0x0800BE74 (placement/tier updater)
//   asm/menu_dab8.s              0x0800DAB8 (menu obj initializer, 10-entry)
//   asm/menu_d470.s              0x0800D470 (menu event dispatcher, 11-entry)
//   asm/menu_d3a4.s              0x0800D3A4 (menu selection handler)
//   asm/menu_d048.s              0x0800D048 (menu record updater)
//   asm/menu_ctor2.s             0x0800C1E4 (menu record-1 constructor)
//   asm/menu_ctor.s              0x0800C010 (menu/save scene constructor)
//   asm/menu_cae4.s              0x0800CAE4 (menu detail renderer)
//   asm/menu_c884.s              0x0800C884 (menu record handler)
//   asm/menu_c4d0.s              0x0800C4D0 (menu record-20 constructor)
//   asm/course_resource_leaf_more2.s 0x08007B8C (record emitter twin)
//   asm/course_resource_leaf_more.s  0x08007B18 (record emitter)
//   asm/course_proximity.s       0x08007210 (proximity search)
//   asm/code_25264.s             0x08025264 (lineup builder)
//   asm/code_23628.s             0x08023628 (race scene ctor)
//   asm/ai_racefsm.s             0x0800AA40 (per-frame race FSM, 8-way)
//   asm/course_dispatch_8aac.s   0x08008CA0 (record-49 phase machine, 9-way)
//
// Transcribed instruction-for-instruction from the asm listings (all pure
// Thumb, byte-exact via the make SHA gate).

#include "gba/types.h"

// ----------------------------------------------------------------------------
// Callees (HOST_STUB: extern declarations for unlifted callees provided by
// the assembly closure).
// ----------------------------------------------------------------------------
#define HOST_STUB(sig) extern sig

HOST_STUB(void  _0800BC04(void));                                    // bx-lr stub @0x0800BC04
HOST_STUB(void  _0800BC08(void *a0, void *a1, void *a2, void *a3));
HOST_STUB(void  _0800BCD4(void *a0, void *a1, void *a2, void *a3, void *s1, void *s2, void *s3, void *s4));
HOST_STUB(int   _08002581C(int a));
HOST_STUB(int   _080025750(int a, int b, int c));
HOST_STUB(u32   _080025E98(u32 a, int b, int c));
HOST_STUB(void  _080025E70(u32 a, int b, int c, u32 d));
HOST_STUB(int   _080025E1C(int a));
HOST_STUB(void  _080025DBC(int a, int b));
HOST_STUB(int   _08002DE04(int n, int d));
HOST_STUB(u32   _08002DF6C(u32 n, u32 d));
HOST_STUB(void  _08002DE98(void));                                   // no-op leaf (mov pc,lr)
HOST_STUB(int   _08002D978(int a, int b));                           // swi Div (quotient)
HOST_STUB(int   _08002D97C(int a, int b));                           // swi DivRem (remainder)
HOST_STUB(int   _08002BE8(int dummy));                               // counter bump, s16 ret
HOST_STUB(void  _08002C48(int a, u16 b));
HOST_STUB(void  _08002D98(u32 a, u32 b, u32 c, u32 d, u32 e));
extern void *_0802E104(void *a, int b, u32 c);
extern void  sub_0802D974(const void *src, void *dst, u32 ctrl);
#define _08002E104 _0802E104
#define _08002D974 sub_0802D974
HOST_STUB(void  _08004C84(u32 a, u32 b));                            // trampoline -> ROM
HOST_STUB(void *_08007498(void *x, int i));
HOST_STUB(void *_0800748C(void *x));
HOST_STUB(void *_08002BFC(int a));
HOST_STUB(void  _08002C34(int idx, void *node));
HOST_STUB(void  _0800798C(void *a, void *b));
HOST_STUB(void  _08007A58(void *p));
HOST_STUB(void  _080075E8(void *a, int b, int c));
HOST_STUB(void  _08007614(void *a, int b, int c, int d));
HOST_STUB(void  _08007ABC(void *a, void *b, int c));
HOST_STUB(int   _0800572C(int v));
HOST_STUB(void  _080056FC(void));
HOST_STUB(void  _080055D8(u32 a, u16 b));
HOST_STUB(void  _08004D4C(u32 a, u32 b, u32 c));
HOST_STUB(void  _080050E8(void *a, int b));
extern void  sub_080032E0(volatile u8 *p);
extern void  sub_08003104(void *handler);
#define _080032E0 sub_080032E0
#define _08003104 sub_08003104
HOST_STUB(void *_08004DC8(void *a));
HOST_STUB(void  _08004EF0(void *a));
HOST_STUB(void *_08004B68(void));
HOST_STUB(void  _08004BFC(int v));
HOST_STUB(void  _08004EA8(int v));
HOST_STUB(void  _08004EC0(int v));
HOST_STUB(u16   _080024A0(void));
HOST_STUB(int   _0800A9E0(int t, int r));
HOST_STUB(int   _0800A9A0(int t, int r));HOST_STUB(u32   _08002140(void));
HOST_STUB(u16   _08002178(int i));
HOST_STUB(void  _08002158(int a, int b));
HOST_STUB(void  _080016D0(int v));
HOST_STUB(u16   _08001E8C(int a, int b));
HOST_STUB(u16   _08001EA4(int a));
HOST_STUB(int   _08001E30(void));
HOST_STUB(void  _08001E48(int a, int b));
HOST_STUB(int   _08001F8C(void));  // 32-bit contract: ROM caller does a bare `cmp r6,r0` (no zero-extend), as in garage_records.c/_080028EB0
HOST_STUB(void  _08001F80(u16 v));
HOST_STUB(void  _08003940(int a, u32 b));
HOST_STUB(u16   _08002044(void));
HOST_STUB(void  _08003F18(u32 a, u32 b, const volatile u8 *c, int d));
HOST_STUB(u16   _0800206C(int a));
HOST_STUB(u16   _08001E5C(int a, int b));
HOST_STUB(void  _08003978(int a, int b, int c));
extern void sub_08003838(int a, u32 b, const volatile u8 *c);
extern void sub_08003954(int a, u32 b, int c);
HOST_STUB(void  _080026A4C(void *obj, int slot, int attr));
HOST_STUB(void  _08026938(void *obj, int slot));
HOST_STUB(int   _080024E54(s16 idx));
HOST_STUB(int   _080022E4(int id));
HOST_STUB(void  _0800C814(void *c));
HOST_STUB(void  _0800C454(void));
HOST_STUB(void  _0800C4AC(void));
HOST_STUB(void  _0800D298(void *rec));
HOST_STUB(void  _0800D2AC(void *rec));
HOST_STUB(void  _0800D2B0(void *rec));
HOST_STUB(void  _0800D95C(void *dst, const void *src, int n));
HOST_STUB(void  _0800DAB8(void *p));
HOST_STUB(void  _0800D77C(void *a, int b, int c));
HOST_STUB(void  _0800B4A8(void));
HOST_STUB(void  _0800B89C(void));
HOST_STUB(void  _0800B9F0(void));
HOST_STUB(void  _0800B0BC(void));
HOST_STUB(void  _080023FF8(int a, int b));
HOST_STUB(void  _080023FE4(void));
HOST_STUB(void  _08004CC4(void));
HOST_STUB(void  _08004CD4(int v));
HOST_STUB(void  _0800279C(int v));
HOST_STUB(s16   _080026F8(int a, int b));
HOST_STUB(s16   _08002730(int a, int b));
HOST_STUB(u32   _08024144(void));
HOST_STUB(int   _08005B5C(int v));
HOST_STUB(int   _08008014(void));
// The three spellings below are DROPPED-DIGIT TYPOS of the documented car-
// catalog getters : 0x08025518/25530/254E8 = Ai_LineGet2/3/0
// (s8[0x080CCEEC + 20*carId + 13/12/0]). The 0x080054E8/5518/5530 VMAs are
// mid-instruction interiors of DMA/CpuSet loops with no ROM callers — the
// veneers they produced were phantoms of this typo.
HOST_STUB(s8    _08025518(int a));
HOST_STUB(s8    _08025530(int a));
HOST_STUB(s8    _080254E8(int a));
HOST_STUB(void  _08002B214(int v));
HOST_STUB(void  _080025BC8(void *a, int b));
HOST_STUB(void  _08002124(u16 v));
HOST_STUB(u32   _08003130(void));
HOST_STUB(int   _08003D18(int a, volatile u8 *b));
HOST_STUB(void  _0800313C(u32 a));
HOST_STUB(void  _08008768(void *a, int b));
HOST_STUB(void  _080088B0(void *a, int b));
HOST_STUB(void  _0800861C(void *a, int b));
HOST_STUB(void *_080089FC(void *a));
HOST_STUB(void  _08005BA8(void *a, int b));
HOST_STUB(void  _08005DA4(void *a, void *b));
HOST_STUB(void  _080053DC(int a, const volatile u8 *b, int c, int d, int e, int f, int g, int h));
HOST_STUB(void  _080054A4(int a, int b, int c, int d, int e));
HOST_STUB(int   _08005E14(void *a, void *b, void *c));
HOST_STUB(void *_08006C10(void *state, int d));
HOST_STUB(void  _08007BFC(void *a, u8 b, u8 c, u32 d, u32 e, u32 f, u32 g));
HOST_STUB(void  _0800D280(void *a, void *b));                        // fixed 2-arg ABI
// Trampoline decls for ROM-exact callees (leads // 0x0800XXXX hints).
extern void *RomEmit8AAC(void *r7);          // 0x08008AAC
extern void *RomEmit8BB8(void *r7);          // 0x08008BB8
extern void *RomEmit8768(void *r7, int w);   // 0x08008768
extern void *RomEmit88B0(void *r7, int w);   // 0x080088B0
extern void *RomEmit861C(void *r7, int w);   // 0x0800861C
extern void *RomEmit89FC(void *r7);          // 0x080089FC
extern u32 RomLeaf7658(void *a);             // 0x08007658
extern void sub_08004CC4(void);
extern void sub_08004CD4(int v);
HOST_STUB(void  _08007658(void *a));
HOST_STUB(int   _080050D0(int a, int b));
HOST_STUB(void  _08005260(void *a, void *b, int c, int d)); // 4-arg (r3 = transfer size)
HOST_STUB(u32   _080074A8(void *x));
HOST_STUB(void  _08007770(int a, void *b, int c, int d, u32 e, u32 f)); // 0x08007770 course-resource setup (6 machine args)

// ============================================================================
// Faithful div-remainder cores (sound_divmod.s 0x0802DFE4, armcc ROR
// shift-and-subtract; sound_aeabi_idiv_core.s 0x0802DE9C, same core on
// magnitudes with sign fixup). r0 = exact remainder for all game-range
// inputs; the ROR-assembly + rounding tail is replicated verbatim so even
// the huge-quotient corners match the ROM bit-for-bit. Differential-tested
// on host against an independent Python transcription of the asm.
// ============================================================================
// NOT PROMOTABLE (ARMCC runtime, same class as _0802C390): the three bodies in
// the EABI division block (this pair plus _0802DE04 / _0802DF6C) return with
// `mov pc, lr` / `pop {pc}` and wrap their only callee in a lazy single-register
// frame (`push {r4}` / `push {r0}` after the early exits), while agbcc always
// saves lr in a packed `push {r4, lr}` at entry and returns `pop {r0}; bx r0`.
// Empirically 0 of 1396 promoted bodies carry the marker (build/scan_armcc.py);
// the block is byte-exact via asm (asm/sound_aeabi_idiv_core.s,
// asm/sound_divmod.s) and stays asm-owned.
//
// The core is a MACRO, not a static helper: the ROM has no callee for it --
// both division bodies carry their own inline copy
// (asm/sound_aeabi_idiv_core.s 0x0802DED6..0x0802DF54, asm/sound_divmod.s
// 0x08002E014..0x08002E096) -- so a static helper would emit a `bl` that the
// independent slice cannot resolve (STATIC_HELPER defect class).
#define ROR32(V, N) (((V) >> (N)) | ((V) << ((32u - (N)) & 31u)))

// DIVREM_CORE(R0V, R1V): unsigned shift/subtract core that consumes the
// magnitudes in place; the remainder is left in R0V. The quotient bits
// accumulate in _dc2 via word rotates, exactly as the ROM's
// `mov ip,r3 / movs r4,#N / rors r3,r4 / orrs r2,r3 / mov r3,ip`.
#define DIVREM_CORE(R0V, R1V)                                                 \
    do {                                                                      \
        u32 _dc2, _dc3 = 1u, _dc4, _dcip;                                     \
        u32 _dc4s = 1u << 28;                                                 \
        while (!((R1V) >= _dc4s || (R1V) >= (R0V))) {                         \
            (R1V) <<= 4; _dc3 <<= 4;                                          \
        }                                                                     \
        while (!((R1V) >= _dc4s || (R1V) >= (R0V))) {                         \
            (R1V) <<= 1; _dc3 <<= 1;                                          \
        }                                                                     \
        for (;;) {                                                            \
            _dc2 = 0u;                                                        \
            if ((R0V) >= (R1V)) (R0V) -= (R1V);                               \
            _dc4 = (R1V) >> 1;                                                \
            if ((R0V) >= _dc4) { (R0V) -= _dc4; _dc2 |= ROR32(_dc3, 1); }     \
            _dc4 = (R1V) >> 2;                                                \
            if ((R0V) >= _dc4) { (R0V) -= _dc4; _dc2 |= ROR32(_dc3, 2); }     \
            _dc4 = (R1V) >> 3;                                                \
            if ((R0V) >= _dc4) { (R0V) -= _dc4; _dc2 |= ROR32(_dc3, 3); }     \
            _dcip = _dc3;                                                     \
            if ((R0V) == 0u) break;                                           \
            _dc3 >>= 4; if (_dc3 == 0u) break;                                \
            (R1V) >>= 4;                                                      \
        }                                                                     \
        if ((_dc2 & 0xE0000000u) != 0u) {                                     \
            _dc3 = _dcip;                                                     \
            if (_dc2 & ROR32(_dc3, 3)) (R0V) += (R1V) >> 3;                   \
            if (_dc2 & ROR32(_dc3, 2)) (R0V) += (R1V) >> 2;                   \
            if (_dc2 & ROR32(_dc3, 1)) (R0V) += (R1V) >> 1;                   \
        }                                                                     \
    } while (0)

// _08002DFE4: unsigned remainder. den==0 routes through the _08002DE98
// no-op leaf then returns 0; num<den early-returns num (mov pc,lr).
__attribute__((naked)) u32 DivRemU_02DFE4(u32 num, u32 den) {
    __asm__ volatile (
        ".syntax unified\n"
        "cmp\tr1, #0\n"
        "beq .Ldiv__08002E09A\n"
        "movs\tr3, #1\n"
        "cmp\tr0, r1\n"
        "bcs .Ldiv__08002DFF0\n"
        "mov\tpc, lr\n"
        ".Ldiv__08002DFF0:\n"
        "push\t{r4}\n"
        "movs\tr4, #1\n"
        "lsls\tr4, r4, #28\n"
        ".Ldiv__08002DFF6:\n"
        "cmp\tr1, r4\n"
        "bcs .Ldiv__08002E004\n"
        "cmp\tr1, r0\n"
        "bcs .Ldiv__08002E004\n"
        "lsls\tr1, r1, #4\n"
        "lsls\tr3, r3, #4\n"
        "b .Ldiv__08002DFF6\n"
        ".Ldiv__08002E004:\n"
        "lsls\tr4, r4, #3\n"
        ".Ldiv__08002E006:\n"
        "cmp\tr1, r4\n"
        "bcs .Ldiv__08002E014\n"
        "cmp\tr1, r0\n"
        "bcs .Ldiv__08002E014\n"
        "lsls\tr1, r1, #1\n"
        "lsls\tr3, r3, #1\n"
        "b .Ldiv__08002E006\n"
        ".Ldiv__08002E014:\n"
        "movs\tr2, #0\n"
        "cmp\tr0, r1\n"
        "bcc .Ldiv__08002E01C\n"
        "subs\tr0, r0, r1\n"
        ".Ldiv__08002E01C:\n"
        "lsrs\tr4, r1, #1\n"
        "cmp\tr0, r4\n"
        "bcc .Ldiv__08002E02E\n"
        "subs\tr0, r0, r4\n"
        "mov\tip, r3\n"
        "movs\tr4, #1\n"
        "rors\tr3, r4\n"
        "orrs\tr2, r3\n"
        "mov\tr3, ip\n"
        ".Ldiv__08002E02E:\n"
        "lsrs\tr4, r1, #2\n"
        "cmp\tr0, r4\n"
        "bcc .Ldiv__08002E040\n"
        "subs\tr0, r0, r4\n"
        "mov\tip, r3\n"
        "movs\tr4, #2\n"
        "rors\tr3, r4\n"
        "orrs\tr2, r3\n"
        "mov\tr3, ip\n"
        ".Ldiv__08002E040:\n"
        "lsrs\tr4, r1, #3\n"
        "cmp\tr0, r4\n"
        "bcc .Ldiv__08002E052\n"
        "subs\tr0, r0, r4\n"
        "mov\tip, r3\n"
        "movs\tr4, #3\n"
        "rors\tr3, r4\n"
        "orrs\tr2, r3\n"
        "mov\tr3, ip\n"
        ".Ldiv__08002E052:\n"
        "mov\tip, r3\n"
        "cmp\tr0, #0\n"
        "beq .Ldiv__08002E060\n"
        "lsrs\tr3, r3, #4\n"
        "beq .Ldiv__08002E060\n"
        "lsrs\tr1, r1, #4\n"
        "b .Ldiv__08002E014\n"
        ".Ldiv__08002E060:\n"
        "movs\tr4, #14\n"
        "lsls\tr4, r4, #28\n"
        "ands\tr2, r4\n"
        "bne .Ldiv__08002E06C\n"
        "pop\t{r4}\n"
        "mov\tpc, lr\n"
        ".Ldiv__08002E06C:\n"
        "mov\tr3, ip\n"
        "movs\tr4, #3\n"
        "rors\tr3, r4\n"
        "tst\tr2, r3\n"
        "beq .Ldiv__08002E07A\n"
        "lsrs\tr4, r1, #3\n"
        "adds\tr0, r0, r4\n"
        ".Ldiv__08002E07A:\n"
        "mov\tr3, ip\n"
        "movs\tr4, #2\n"
        "rors\tr3, r4\n"
        "tst\tr2, r3\n"
        "beq .Ldiv__08002E088\n"
        "lsrs\tr4, r1, #2\n"
        "adds\tr0, r0, r4\n"
        ".Ldiv__08002E088:\n"
        "mov\tr3, ip\n"
        "movs\tr4, #1\n"
        "rors\tr3, r4\n"
        "tst\tr2, r3\n"
        "beq .Ldiv__08002E096\n"
        "lsrs\tr4, r1, #1\n"
        "adds\tr0, r0, r4\n"
        ".Ldiv__08002E096:\n"
        "pop\t{r4}\n"
        "mov\tpc, lr\n"
        ".Ldiv__08002E09A:\n"
        "push\t{lr}\n"
        "bl sub_0802DE98\n"
        "movs\tr0, #0\n"
        "pop\t{pc}\n"
        ".syntax divided\n"
    );
}
u32 _08002DFE4(u32 n, u32 d) __attribute__((alias("DivRemU_02DFE4")));
u32 sub_08002DFE4(u32 n, u32 d) __attribute__((alias("DivRemU_02DFE4")));
// _08002DE9C: signed remainder. den==0 -> DE98 then 0; magnitudes through
// the core; negate iff the original num was negative (bpl/negs).
__attribute__((naked)) s32 DivRemS_02DE9C(s32 num, s32 den) {
    __asm__ volatile (
        ".syntax unified\n"
        "movs r3, #1\n"
        "cmp r1, #0\n"
        "beq.n .Ldiv__0802DF60\n"
        "bpl.n .Ldiv__0802DEA6\n"
        "negs r1, r1\n"
        ".Ldiv__0802DEA6:\n"
        "push {r4}\n"
        "push {r0}\n"
        "cmp r0, #0\n"
        "bpl.n .Ldiv__0802DEB0\n"
        "negs r0, r0\n"
        ".Ldiv__0802DEB0:\n"
        "cmp r0, r1\n"
        "bcc.n .Ldiv__0802DF54\n"
        "movs r4, #1\n"
        "lsls r4, r4, #28\n"
        ".Ldiv__0802DEB8:\n"
        "cmp r1, r4\n"
        "bcs.n .Ldiv__0802DEC6\n"
        "cmp r1, r0\n"
        "bcs.n .Ldiv__0802DEC6\n"
        "lsls r1, r1, #4\n"
        "lsls r3, r3, #4\n"
        "b.n .Ldiv__0802DEB8\n"
        ".Ldiv__0802DEC6:\n"
        "lsls r4, r4, #3\n"
        ".Ldiv__0802DEC8:\n"
        "cmp r1, r4\n"
        "bcs.n .Ldiv__0802DED6\n"
        "cmp r1, r0\n"
        "bcs.n .Ldiv__0802DED6\n"
        "lsls r1, r1, #1\n"
        "lsls r3, r3, #1\n"
        "b.n .Ldiv__0802DEC8\n"
        ".Ldiv__0802DED6:\n"
        "movs r2, #0\n"
        "cmp r0, r1\n"
        "bcc.n .Ldiv__0802DEDE\n"
        "subs r0, r0, r1\n"
        ".Ldiv__0802DEDE:\n"
        "lsrs r4, r1, #1\n"
        "cmp r0, r4\n"
        "bcc.n .Ldiv__0802DEF0\n"
        "subs r0, r0, r4\n"
        "mov ip, r3\n"
        "movs r4, #1\n"
        "rors r3, r4\n"
        "orrs r2, r3\n"
        "mov r3, ip\n"
        ".Ldiv__0802DEF0:\n"
        "lsrs r4, r1, #2\n"
        "cmp r0, r4\n"
        "bcc.n .Ldiv__0802DF02\n"
        "subs r0, r0, r4\n"
        "mov ip, r3\n"
        "movs r4, #2\n"
        "rors r3, r4\n"
        "orrs r2, r3\n"
        "mov r3, ip\n"
        ".Ldiv__0802DF02:\n"
        "lsrs r4, r1, #3\n"
        "cmp r0, r4\n"
        "bcc.n .Ldiv__0802DF14\n"
        "subs r0, r0, r4\n"
        "mov ip, r3\n"
        "movs r4, #3\n"
        "rors r3, r4\n"
        "orrs r2, r3\n"
        "mov r3, ip\n"
        ".Ldiv__0802DF14:\n"
        "mov ip, r3\n"
        "cmp r0, #0\n"
        "beq.n .Ldiv__0802DF22\n"
        "lsrs r3, r3, #4\n"
        "beq.n .Ldiv__0802DF22\n"
        "lsrs r1, r1, #4\n"
        "b.n .Ldiv__0802DED6\n"
        ".Ldiv__0802DF22:\n"
        "movs r4, #14\n"
        "lsls r4, r4, #28\n"
        "ands r2, r4\n"
        "beq.n .Ldiv__0802DF54\n"
        "mov r3, ip\n"
        "movs r4, #3\n"
        "rors r3, r4\n"
        "tst r2, r3\n"
        "beq.n .Ldiv__0802DF38\n"
        "lsrs r4, r1, #3\n"
        "adds r0, r0, r4\n"
        ".Ldiv__0802DF38:\n"
        "mov r3, ip\n"
        "movs r4, #2\n"
        "rors r3, r4\n"
        "tst r2, r3\n"
        "beq.n .Ldiv__0802DF46\n"
        "lsrs r4, r1, #2\n"
        "adds r0, r0, r4\n"
        ".Ldiv__0802DF46:\n"
        "mov r3, ip\n"
        "movs r4, #1\n"
        "rors r3, r4\n"
        "tst r2, r3\n"
        "beq.n .Ldiv__0802DF54\n"
        "lsrs r4, r1, #1\n"
        "adds r0, r0, r4\n"
        ".Ldiv__0802DF54:\n"
        "pop {r4}\n"
        "cmp r4, #0\n"
        "bpl.n .Ldiv__0802DF5C\n"
        "negs r0, r0\n"
        ".Ldiv__0802DF5C:\n"
        "pop {r4}\n"
        "mov pc, lr\n"
        ".Ldiv__0802DF60:\n"
        "push {lr}\n"
        "bl sub_0802DE98\n"
        "movs r0, #0\n"
        "pop {pc}\n"
        ".short 0\n"
        ".syntax divided\n"
    );
}
s32 _08002DE9C(s32 a, s32 b) __attribute__((alias("DivRemS_02DE9C")));
s32 sub_08002DE9C(s32 a, s32 b) __attribute__((alias("DivRemS_02DE9C")));
// ============================================================================
// The faithful _08002ED0 placement writer now lives globally as EmitPlace_02ED0
// (src/foundation_runtime.c, _/sub_/Sub_ aliases); the emitter transcriptions
// below call it directly (10 args each).
// ============================================================================
extern void _08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j);

void CourseEmit7B18(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) {
    u8 *r4 = (u8 *)rec;
    void *arr = _0800748C(_08007498(*(void **)(r4 + 4), kind));
    u8 *a = (u8 *)arr;
    u32 t1 = (u32)*(u16 *)(r4 + 0);
    u32 t0 = (u32)*(u16 *)(a + 2);
    u32 base = t1 + t0;
    int i = 0;
    u8 *ent;
    u32 one;
    if (i >= (int)a[7]) goto done;
    one = 1;
    ent = a + 8;
    do {
        u32 x = (u32)ent[2] + (u32)dx;
        u32 y = (u32)ent[3] + (u32)dy;
        u32 z = (u32)ent[0] + base;
        _08002ED0((void *)(uintptr_t)x, (int)y, (int)z, (int)s0, (int)s1,
                   (int)ent[1], (int)one, (int)s2, (int)s3, (int)one);
        ent += 4;
        i++;
    } while (i < (int)a[7]);
done:;
}

// Likewise the 0x08007B8C ROM body (see CourseEmit7B18 above).
void CourseEmit7B8C(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) {
    u8 *r4 = (u8 *)rec;
    void *arr = _0800748C(_08007498(*(void **)(r4 + 4), kind));
    u8 *a = (u8 *)arr;
    u32 t1 = (u32)*(u16 *)(r4 + 0);
    u32 t0 = (u32)*(u16 *)(a + 2);
    u32 base = t1 + t0;
    int i = 0;
    u8 *ent;
    if (i >= (int)a[7]) goto done;
    ent = a + 8;
    do {
        u32 x = (u32)ent[2] + (u32)dx;
        u32 y = (u32)ent[3] + (u32)dy;
        u32 z = (u32)ent[0] + base;
        _08002ED0((void *)(uintptr_t)x, (int)y, (int)z, (int)s0, (int)s1,
                   (int)ent[1], 1, (int)s2, (int)s3, 0);
        ent += 4;
        i++;
    } while (i < (int)a[7]);
done:;
}

// 7BFC variant: x=b2+r3-in, y=b3+caller-sp+0, z=b0+r1-in(kind),
// 2ED0 r3=caller-sp+4, stack=(caller-sp+8, b1, 1, caller-sp+12,
// caller-sp+16, 1).
static inline void CourseIter7BFC_Static(void *rec, int kind, int dx3,
                                  u32 s0, u32 s1, u32 s2, u32 s3, u32 s4) {
    u8 *r4 = (u8 *)rec;
    void *arr = _0800748C(_08007498(*(void **)(r4 + 4), kind));
    u8 *a = (u8 *)arr;
    u32 cnt = a[7];
    u8 *ent = a + 8;
    for (u32 i = 0; i < cnt; i++, ent += 4) {
        u32 x = (u32)ent[2] + (u32)dx3;
        u32 y = (u32)ent[3] + s0;
        u32 z = (u32)ent[0] + (u32)kind;
        _08002ED0((void *)(uintptr_t)x, (int)y, (int)z, (int)s1, (int)s2,
                   (int)ent[1], 1, (int)s3, (int)s4, 1);
    }
}

// ============================================================================
// menu_progress_update.s 0x0800BD40 — results progress updater. Seat fill via
// _0800BC08, conditional record apply via _0800BCD4, 12-byte award copy on
// the sv==1 path, WA+0x10F4 clear otherwise, WA+0xFCE latch at the tail.
// ============================================================================
__attribute__((naked)) void MenuProgressUpdate_0BD40(void)
{
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r9\n"
        "mov r6, r8\n"
        "push {r6, r7}\n"
        "sub sp, #60\n"
        "movs r0, #0\n"
        "str r0, [sp, #52]\n"
        "str r0, [sp, #56]\n"
        "add r4, sp, #40\n"
        "strh r0, [r4, #0]\n"
        "bl 0x0800bc04\n"
        "ldr r0, .L0bd40_p0\n"
        "mov r8, r0\n"
        "ldr r0, .L0bd40_p1\n"
        "add r0, r8\n"
        "movs r1, #0\n"
        "ldrsh r0, [r0, r1]\n"
        "bl 0x0802581c\n"
        "lsls r0, r0, #16\n"
        "asrs r5, r0, #16\n"
        "add r7, sp, #44\n"
        "add r6, sp, #28\n"
        "adds r0, r7, #0\n"
        "adds r1, r5, #0\n"
        "add r2, sp, #16\n"
        "adds r3, r6, #0\n"
        "bl 0x0800bc08\n"
        "ldr r3, .L0bd40_p2\n"
        "add r3, r8\n"
        "mov r9, r3\n"
        "movs r0, #0\n"
        "ldrsb r0, [r3, r0]\n"
        "cmp r0, #0\n"
        "beq .L0bd40_clear\n"
        "add r2, sp, #48\n"
        "add r0, sp, #52\n"
        "str r0, [sp, #0]\n"
        "add r0, sp, #56\n"
        "str r0, [sp, #4]\n"
        "add r0, sp, #16\n"
        "str r0, [sp, #8]\n"
        "str r6, [sp, #12]\n"
        "adds r0, r5, #0\n"
        "adds r1, r7, #0\n"
        "adds r3, r4, #0\n"
        "bl 0x0800bcd4\n"
        "mov r1, r9\n"
        "movs r0, #0\n"
        "ldrsb r0, [r1, r0]\n"
        "cmp r0, #0\n"
        "beq .L0bd40_clear\n"
        "ldr r1, [sp, #52]\n"
        "ldr r0, .L0bd40_p3\n"
        "add r0, r8\n"
        "strh r1, [r0, #0]\n"
        "ldr r0, [sp, #56]\n"
        "ldr r1, .L0bd40_p4\n"
        "add r1, r8\n"
        "strh r0, [r1, #0]\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #16\n"
        "mov r2, r8\n"
        "cmp r0, #1\n"
        "bne .L0bd40_after_copy\n"
        "lsls r1, r5, #3\n"
        "adds r1, r1, r5\n"
        "lsls r1, r1, #3\n"
        "adds r1, r1, r2\n"
        "movs r3, #196\n"
        "lsls r3, r3, #3\n"
        "adds r1, r1, r3\n"
        "adds r0, r6, #0\n"
        "ldmia r0!, {r3, r5, r6}\n"
        "stmia r1!, {r3, r5, r6}\n"
        "b .L0bd40_after_copy\n"
        ".align 2, 0\n"
        ".L0bd40_p0: .4byte 0x03001780\n"
        ".L0bd40_p1: .4byte 0x00000576\n"
        ".L0bd40_p2: .4byte 0x000010e4\n"
        ".L0bd40_p3: .4byte 0x00000fc8\n"
        ".L0bd40_p4: .4byte 0x00000fca\n"
        ".L0bd40_clear:\n"
        "ldr r2, .L0bd40_p5\n"
        "ldr r5, .L0bd40_p6\n"
        "adds r1, r2, r5\n"
        "movs r0, #0\n"
        "str r0, [r1, #0]\n"
        ".L0bd40_after_copy:\n"
        "ldrh r1, [r4, #0]\n"
        "ldr r6, .L0bd40_p7\n"
        "adds r0, r2, r6\n"
        "strh r1, [r0, #0]\n"
        "add sp, #60\n"
        "pop {r3, r4}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".L0bd40_p5: .4byte 0x03001780\n"
        ".L0bd40_p6: .4byte 0x000010f4\n"
        ".L0bd40_p7: .4byte 0x00000fce\n"
        ".syntax divided\n"
    );
}
void _0800BD40(void) __attribute__((alias("MenuProgressUpdate_0BD40")));
void sub_0800BD40(void) __attribute__((alias("MenuProgressUpdate_0BD40")));

// ============================================================================
// menu_place.s 0x0800BE74 — placement/tier updater. Signed (sl) vs unsigned
// (r4) mixed-radix decomposition of two related cells (//3 exact), tiered on
// the unsigned difference, then grid-record resolve via _080025E98/_080025E70
// and tier store to WA+0x10E5. (Proves _08002DE9C/_08002DFE4 = remainders:
// quotient readings degenerate the decomposition.)
// ============================================================================
__attribute__((naked)) void MenuPlaceUpdate_0BE74(void) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Lmp_be74_0800be74:\n"
        "\tpush {r4, r5, r6, r7, lr}\n"
        "\tmov r7, sl\n"
        "\tmov r6, r9\n"
        "\tmov r5, r8\n"
        "\tpush {r5, r6, r7}\n"
        "\tsub sp, #16\n"
        "\tmovs r0, #0\n"
        "\tstr r0, [sp, #12]\n"
        "\tldr r6, .Lmp_be74_0800bf58\n"
        "\tldr r1, .Lmp_be74_0800bf5c\n"
        "\tadds r0, r6, r1\n"
        "\tmovs r2, #0\n"
        "\tldrsh r0, [r0, r2]\n"
        "\tbl 0x0802581C\n"
        "\tldr r3, .Lmp_be74_0800bf60\n"
        "\tadds r0, r6, r3\n"
        "\tmovs r1, #0\n"
        "\tldrsh r0, [r0, r1]\n"
        "\tldr r2, .Lmp_be74_0800bf64\n"
        "\tadds r1, r6, r2\n"
        "\tmovs r3, #0\n"
        "\tldrsh r1, [r1, r3]\n"
        "\tldr r3, .Lmp_be74_0800bf68\n"
        "\tadds r2, r6, r3\n"
        "\tmovs r3, #0\n"
        "\tldrsh r2, [r2, r3]\n"
        "\tbl 0x08025750\n"
        "\tadds r4, r0, #0\n"
        "\tldr r1, .Lmp_be74_0800bf6c\n"
        "\tadds r0, r6, r1\n"
        "\tldr r0, [r0, #0]\n"
        "\tstr r0, [sp, #0]\n"
        "\tmov r1, sp\n"
        "\tldr r2, .Lmp_be74_0800bf70\n"
        "\tadds r0, r6, r2\n"
        "\tldrh r0, [r0, #0]\n"
        "\tstrh r0, [r1, #4]\n"
        "\tadd r0, sp, #8\n"
        "\tldr r3, .Lmp_be74_0800bf74\n"
        "\tadds r1, r6, r3\n"
        "\tmovs r2, #3\n"
        "\tbl 0x0800D95C\n"
        "\tldr r0, .Lmp_be74_0800bf78\n"
        "\tmov r9, r0\n"
        "\tadds r0, r4, #0\n"
        "\tmov r1, r9\n"
        "\tbl 0x0802DE04\n"
        "\tldr r2, .Lmp_be74_0800bf7c\n"
        "\tadds r1, r0, #0\n"
        "\tmuls r1, r2\n"
        "\tmov sl, r1\n"
        "\tadds r0, r4, #0\n"
        "\tmov r1, r9\n"
        "\tbl 0x0802DE9C\n"
        "\tmovs r3, #150\n"
        "\tlsls r3, r3, #1\n"
        "\tmov r8, r3\n"
        "\tmov r1, r8\n"
        "\tbl 0x0802DE04\n"
        "\tmovs r7, #100\n"
        "\tmuls r0, r7\n"
        "\tadd sl, r0\n"
        "\tadds r0, r4, #0\n"
        "\tmov r1, r8\n"
        "\tbl 0x0802DE9C\n"
        "\tmovs r1, #3\n"
        "\tbl 0x0802DE04\n"
        "\tadd sl, r0\n"
        "\tldr r5, [sp, #0]\n"
        "\tadds r0, r5, #0\n"
        "\tmov r1, r9\n"
        "\tbl 0x0802DF6C\n"
        "\tldr r1, .Lmp_be74_0800bf7c\n"
        "\tadds r4, r0, #0\n"
        "\tmuls r4, r1\n"
        "\tadds r0, r5, #0\n"
        "\tmov r1, r9\n"
        "\tbl 0x0802DFE4\n"
        "\tmov r1, r8\n"
        "\tbl 0x0802DF6C\n"
        "\tmuls r0, r7\n"
        "\tadds r4, r4, r0\n"
        "\tadds r0, r5, #0\n"
        "\tmov r1, r8\n"
        "\tbl 0x0802DFE4\n"
        "\tmovs r1, #3\n"
        "\tbl 0x0802DF6C\n"
        "\tadds r4, r4, r0\n"
        "\tmov r2, sl\n"
        "\tsubs r1, r2, r4\n"
        "\tldr r3, .Lmp_be74_0800bf80\n"
        "\tadds r6, r6, r3\n"
        "\tmovs r0, #0\n"
        "\tldrsb r0, [r6, r0]\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmp_be74_0800bfe4\n"
        "\tcmp r1, #99\n"
        "\tbhi.n .Lmp_be74_0800bf84\n"
        "\tmovs r0, #1\n"
        "\tstr r0, [sp, #12]\n"
        "\tb.n .Lmp_be74_0800bf9a\n"
        "\t.Lmp_be74_0800bf58: .word 0x03001780\n"
        "\t.Lmp_be74_0800bf5c: .word 0x00000576\n"
        "\t.Lmp_be74_0800bf60: .word 0x00000FF2\n"
        "\t.Lmp_be74_0800bf64: .word 0x00000FF6\n"
        "\t.Lmp_be74_0800bf68: .word 0x0000103A\n"
        "\t.Lmp_be74_0800bf6c: .word 0x000010F4\n"
        "\t.Lmp_be74_0800bf70: .word 0x00000574\n"
        "\t.Lmp_be74_0800bf74: .word 0x00001088\n"
        "\t.Lmp_be74_0800bf78: .word 0x00004650\n"
        "\t.Lmp_be74_0800bf7c: .word 0x00001770\n"
        "\t.Lmp_be74_0800bf80: .word 0x000010E4\n"
        ".Lmp_be74_0800bf84:\n"
        "\tadds r0, r1, #0\n"
        "\tsubs r0, #100\n"
        "\tcmp r0, #99\n"
        "\tbhi.n .Lmp_be74_0800bf92\n"
        "\tmovs r1, #2\n"
        "\tstr r1, [sp, #12]\n"
        "\tb.n .Lmp_be74_0800bf9a\n"
        ".Lmp_be74_0800bf92:\n"
        "\tcmp r1, #199\n"
        "\tble.n .Lmp_be74_0800bf9a\n"
        "\tmovs r2, #3\n"
        "\tstr r2, [sp, #12]\n"
        ".Lmp_be74_0800bf9a:\n"
        "\tldr r2, .Lmp_be74_0800c000\n"
        "\tldr r3, .Lmp_be74_0800c004\n"
        "\tadds r6, r2, r3\n"
        "\tmovs r1, #0\n"
        "\tldrsh r0, [r6, r1]\n"
        "\tadds r3, #4\n"
        "\tadds r5, r2, r3\n"
        "\tmovs r3, #0\n"
        "\tldrsh r1, [r5, r3]\n"
        "\tldr r3, .Lmp_be74_0800c008\n"
        "\tadds r4, r2, r3\n"
        "\tmovs r3, #0\n"
        "\tldrsh r2, [r4, r3]\n"
        "\tbl 0x08025E98\n"
        "\tldr r1, [sp, #0]\n"
        "\tcmp r0, r1\n"
        "\tbhi.n .Lmp_be74_0800bfd2\n"
        "\tmovs r1, #0\n"
        "\tldrsh r0, [r6, r1]\n"
        "\tmovs r2, #0\n"
        "\tldrsh r1, [r5, r2]\n"
        "\tmovs r3, #0\n"
        "\tldrsh r2, [r4, r3]\n"
        "\tbl 0x08025E98\n"
        "\tcmp r0, #0\n"
        "\tbne.n .Lmp_be74_0800bfe4\n"
        ".Lmp_be74_0800bfd2:\n"
        "\tmovs r1, #0\n"
        "\tldrsh r0, [r6, r1]\n"
        "\tmovs r2, #0\n"
        "\tldrsh r1, [r5, r2]\n"
        "\tmovs r3, #0\n"
        "\tldrsh r2, [r4, r3]\n"
        "\tldr r3, [sp, #0]\n"
        "\tbl 0x08025E70\n"
        ".Lmp_be74_0800bfe4:\n"
        "\tldr r0, .Lmp_be74_0800c000\n"
        "\tldr r1, .Lmp_be74_0800c00c\n"
        "\tadds r0, r0, r1\n"
        "\tmov r2, sp\n"
        "\tldrb r2, [r2, #12]\n"
        "\tstrb r2, [r0, #0]\n"
        "\tadd sp, #16\n"
        "\tpop {r3, r4, r5}\n"
        "\tmov r8, r3\n"
        "\tmov r9, r4\n"
        "\tmov sl, r5\n"
        "\tpop {r4, r5, r6, r7}\n"
        "\tpop {r0}\n"
        "\tbx r0\n"
        ".Lmp_be74_0800c000: .word 0x03001780\n"
        ".Lmp_be74_0800c004: .word 0x00000FF2\n"
        ".Lmp_be74_0800c008: .word 0x0000103A\n"
        ".Lmp_be74_0800c00c: .word 0x000010E5\n"
        ".syntax divided\n"
    );
}

void _0800BE74(void) __attribute__((alias("MenuPlaceUpdate_0BE74")));
void sub_0800BE74(void) __attribute__((alias("MenuPlaceUpdate_0BE74")));

// ============================================================================
// menu_dab8.s 0x0800DAB8 — menu object initializer. Resource attach via the
// _0800798C/_08007A58/_080075E8 family, save-alloc triple, 10-entry phase
// table on s16[WA+0xFBC], then three _08007ABC bindings.
// ============================================================================
// The switch arms follow the ROM's case-block order, so the emitted labels
// also reproduce its non-monotonic 10-entry table. Every valid case branches
// to one store; the out-of-range default branches past it. Absolute symbols
// keep the two-word WA-plus-offset address sequence used by the ROM.
void MenuObjInit_0DAB8(void *rec) {
    extern u8 MenuObjWA_DAB8[] __asm__("MenuObjWA_DAB8");
    extern u8 MenuObjModeOffset_DAB8[] __asm__("MenuObjModeOffset_DAB8");
    extern u8 MenuObjSubmodeOffset_DAB8[] __asm__("MenuObjSubmodeOffset_DAB8");
    __asm__(".globl MenuObjWA_DAB8\nMenuObjWA_DAB8 = 0x03001780\n");
    __asm__(".globl MenuObjModeOffset_DAB8\nMenuObjModeOffset_DAB8 = 0x00000FBC\n");
    __asm__(".globl MenuObjSubmodeOffset_DAB8\nMenuObjSubmodeOffset_DAB8 = 0x00001078\n");
    u8 *r6 = (u8 *)rec;
    u8 *tmpl = (u8 *)(uintptr_t)0x082A798Cu;
    u8 *r4 = r6 + 0x24;
    _0800798C(tmpl, r4);
    _08007A58(r4);
    _080075E8(tmpl, 4, 1);
    _080075E8(tmpl, 3, 2);
    _0800798C((void *)(uintptr_t)0x082A95E4u, r6 + 0x34);
    *(u32 *)(r6 + 0x44) = (u32)_0800572C(0x10);
    register u32 cls __asm__("r0");
    u32 mode_addr = (u32)(uintptr_t)MenuObjWA_DAB8 + (u32)(uintptr_t)MenuObjModeOffset_DAB8;
    switch ((int)(s16)(*(u16 *)(uintptr_t)mode_addr)) {
    case 0: cls = 7; goto store_class;
    case 5: cls = 6; goto store_class;
    case 1: cls = 8; goto store_class;
    case 6: cls = 5; goto store_class;
    case 2: cls = 1; goto store_class;
    case 7: {
        u32 sub_addr = (u32)(uintptr_t)MenuObjWA_DAB8 + (u32)(uintptr_t)MenuObjSubmodeOffset_DAB8;
        int sub = (int)(s16)(*(u16 *)(uintptr_t)sub_addr);
        if (sub == 1) goto submode_one;
        if (sub == 2) goto submode_two;
        cls = 2;
        goto store_class;
    submode_one:
        cls = 0xB;
        goto store_class;
    submode_two:
        cls = 0xA;
        goto store_class;
    }
    case 3: cls = 3; goto store_class;
    case 8: cls = 0; goto store_class;
    case 4: cls = 4; goto store_class;
    case 9: cls = 9; goto store_class;
    default: goto after_class;
    }
store_class:
    *(u32 *)(r6 + 0x48) = cls;
after_class:
    _08007ABC((void *)(uintptr_t)*(u32 *)(r6 + 0x38),
              (void *)(uintptr_t)*(u32 *)(r6 + 0x48),
              (int)*(u32 *)(r6 + 0x44));
    _0800798C((void *)(uintptr_t)0x082AB030u, r6 + 0x3C);
    int a26 = _0800572C(0x26);
    *(u32 *)(r6 + 0x50) = (u32)a26;
    _08007ABC((void *)(uintptr_t)*(u32 *)(r6 + 0x40),
              (void *)(uintptr_t)*(u32 *)(r6 + 0x54), a26);
    _0800798C((void *)(uintptr_t)0x082B283Cu, r6 + 0x2C);
    int a3c = _0800572C(0x3C);
    *(u32 *)(r6 + 0x5C) = (u32)a3c;
    _08007ABC((void *)(uintptr_t)*(u32 *)(r6 + 0x30),
              (void *)(uintptr_t)*(u32 *)(r6 + 0x60), a3c);
}
void _0800DAB8(void *p) __attribute__((alias("MenuObjInit_0DAB8")));
void Sub_0800DAB8(void *p) __attribute__((alias("MenuObjInit_0DAB8")));
void sub_0800DAB8(void *p) __attribute__((alias("MenuObjInit_0DAB8")));
void Course_0x0800DAB8(void *p) __attribute__((alias("MenuObjInit_0DAB8"))); /* trampoline elimination: friendly-name spelling used by rec35_init.c */

// ============================================================================
// menu_d470.s 0x0800D470 — menu event dispatcher (ev 1-based; 11-entry table
// with holes falling through to the tail).
// ============================================================================
void MenuSelectHandler_0D3A4(void *rec, u32 a, u32 b);  // fwd (defined below)
void MenuEventDispatch_0D470(int ev, u32 a, u32 b, void *rec) {
    u8 *r3 = (u8 *)rec;
    register u32 r4 __asm__("r4") = a;
    switch (ev - 1) {
    case 1:
        _0800D280(r3, (void *)(uintptr_t)r4);
        break;
    case 0:
        _0800D298(r3);
        break;
    case 5:
        MenuSelectHandler_0D3A4(r3, (u16)r4, (u16)b);
        break;
    case 4:
        _0800D2AC(r3);
        break;
    case 6:
        _0800D2B0(r3);
        break;
    case 10:
        break;
    default:
        break;
    }
}
__asm__(".align 2, 0");
void _0800D470(int e, u32 a, u32 b, void *r) __attribute__((alias("MenuEventDispatch_0D470")));
void sub_0800D470(int e, u32 a, u32 b, void *r) __attribute__((alias("MenuEventDispatch_0D470")));

// ============================================================================
// menu_d3a4.s 0x0800D3A4 — menu selection handler: token handshake, per-slot
// key scan with cursor walk on bits 6/7, transition requests on bits 0/1,
// paint invocations on exit.
// ============================================================================
void MenuSelectHandler_0D3A4(void *rec, u32 a, u32 b) {
    u8 *r5 = (u8 *)rec;
    u16 r8 = (u16)a, r7 = (u16)b;
    _080016D0(123);
    *(u16 *)(r5 + 6) = _08001EA4(2);
    for (int r6 = 0; r6 < _08001F8C(); r6++) {   // signed compare: ROM branches `blt`, not `bcc`
        (void)_08001E8C((int)r6, 0);
        u16 key = _08001E8C((int)r6, 1);
        if (_08001E30()) {
            if (key & 0x40) {
                if (*(u16 *)(r5 + 4) != 0u) *(u16 *)(r5 + 4) -= 1u;
            }
            if (key & 0x80) {
                if (*(u16 *)(r5 + 4) <= 2u) *(u16 *)(r5 + 4) += 1u;
            }
        }
        if (key & 1u) {
            _08004EC0(1);
            _08004BFC(*(u16 *)(r5 + 6));
            goto d438;
        }
        if (key & 2u) {
            _08004EA8(1);
            goto d438;
        }
    }
d438:
    if (r7 & 0x100u) {
        _08004EC0(1);
        _08004BFC(0);
    }
    _08001E48(0, r8);
    _08001E48(1, r7);
    _08001E48(2, *(u16 *)(r5 + 4));
}
void _0800D3A4(void *r, u32 a, u32 b) __attribute__((alias("MenuSelectHandler_0D3A4")));
void sub_0800D3A4(void *r, u32 a, u32 b) __attribute__((alias("MenuSelectHandler_0D3A4")));

// ============================================================================
// menu_d048.s 0x0800D048 — menu record updater: key-bit lanes adjust the
// +6/+8 cursors with clamp/relayout, persisting to WA+0x574/0x1151 on bit0.
// (Second _08026A4C call passes s16[rec+8] explicitly = the value ROM's r2
// still holds: the compiled Sprite_SetTenTwelve preserves r2, as does ROM.)
// ============================================================================
__attribute__((naked)) void MenuRecordUpdater_0D048(void *rec, u32 dummy, u32 key) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r9\n"
        "mov r6, r8\n"
        "push {r6, r7}\n"
        "adds r4, r0, #0\n"
        "lsls r2, r2, #16\n"
        "lsrs r5, r2, #16\n"
        "bl 0x08004b68\n"
        "movs r1, #6\n"
        "ldrsh r0, [r4, r1]\n"
        "mov r8, r0\n"
        "movs r7, #0\n"
        "movs r6, #0\n"
        "movs r3, #8\n"
        "ldrsh r2, [r4, r3]\n"
        "mov r9, r2\n"
        "movs r0, #2\n"
        "ands r0, r5\n"
        "cmp r0, #0\n"
        "beq.n .Ld048_after_bit2\n"
        "movs r0, #1\n"
        "bl 0x08004ea8\n"
        ".Ld048_after_bit2:\n"
        "movs r0, #1\n"
        "ands r0, r5\n"
        "cmp r0, #0\n"
        "beq.n .Ld048_after_bit1\n"
        "ldr r1, 1f\n"
        "ldrh r2, [r4, #6]\n"
        "ldr r3, 2f\n"
        "adds r0, r1, r3\n"
        "strh r2, [r0, #0]\n"
        "ldrh r0, [r4, #8]\n"
        "ldr r2, 3f\n"
        "adds r1, r1, r2\n"
        "strb r0, [r1, #0]\n"
        "movs r0, #1\n"
        "bl 0x08004ea8\n"
        ".Ld048_after_bit1:\n"
        "movs r0, #32\n"
        "ands r0, r5\n"
        "cmp r0, #0\n"
        "beq.n .Ld048_bit16\n"
        "subs r7, #1\n"
        ".Ld048_bit16:\n"
        "movs r0, #16\n"
        "ands r0, r5\n"
        "cmp r0, #0\n"
        "beq.n .Ld048_bit64\n"
        "adds r7, #1\n"
        ".Ld048_bit64:\n"
        "movs r0, #64\n"
        "ands r0, r5\n"
        "cmp r0, #0\n"
        "beq.n .Ld048_bit128\n"
        "subs r6, #1\n"
        ".Ld048_bit128:\n"
        "movs r0, #128\n"
        "ands r0, r5\n"
        "cmp r0, #0\n"
        "beq.n .Ld048_cursor8\n"
        "adds r6, #1\n"
        ".Ld048_cursor8:\n"
        "ldrh r3, [r4, #8]\n"
        "adds r0, r3, r6\n"
        "movs r1, #0\n"
        "strh r0, [r4, #8]\n"
        "lsls r0, r0, #16\n"
        "cmp r0, #0\n"
        "bge.n .Ld048_cursor8_nonnegative\n"
        "movs r0, #4\n"
        "strh r0, [r4, #8]\n"
        ".Ld048_cursor8_nonnegative:\n"
        "movs r2, #8\n"
        "ldrsh r0, [r4, r2]\n"
        "cmp r0, #4\n"
        "ble.n .Ld048_cursor6\n"
        "strh r1, [r4, #8]\n"
        ".Ld048_cursor6:\n"
        "ldrh r3, [r4, #6]\n"
        "adds r0, r3, r7\n"
        "strh r0, [r4, #6]\n"
        "lsls r0, r0, #16\n"
        "cmp r0, #0\n"
        "bge.n .Ld048_cursor6_nonnegative\n"
        "strh r1, [r4, #6]\n"
        ".Ld048_cursor6_nonnegative:\n"
        "movs r1, #6\n"
        "ldrsh r0, [r4, r1]\n"
        "cmp r0, #96\n"
        "ble.n .Ld048_cursor6_done\n"
        "movs r0, #96\n"
        "strh r0, [r4, #6]\n"
        ".Ld048_cursor6_done:\n"
        "movs r2, #6\n"
        "ldrsh r1, [r4, r2]\n"
        "cmp r1, r8\n"
        "beq.n .Ld048_after_obj6\n"
        "ldr r0, [r4, #28]\n"
        "movs r3, #8\n"
        "ldrsh r2, [r4, r3]\n"
        "bl 0x08026a4c\n"
        "ldr r0, [r4, #28]\n"
        "movs r2, #6\n"
        "ldrsh r1, [r4, r2]\n"
        "bl 0x08026938\n"
        ".Ld048_after_obj6:\n"
        "movs r3, #8\n"
        "ldrsh r2, [r4, r3]\n"
        "cmp r2, r9\n"
        "beq.n .Ld048_done\n"
        "ldr r0, [r4, #28]\n"
        "movs r3, #6\n"
        "ldrsh r1, [r4, r3]\n"
        "bl 0x08026a4c\n"
        ".Ld048_done:\n"
        "pop {r3, r4}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        "1: .word 0x03001780\n"
        "2: .word 0x00000574\n"
        "3: .word 0x00001151\n"
        ".syntax divided\n"
    );
}
void _0800D048(void *r, u32 d, u32 k) __attribute__((alias("MenuRecordUpdater_0D048")));
void sub_0800D048(void *r, u32 d, u32 k) __attribute__((alias("MenuRecordUpdater_0D048")));

// 16-byte template the two menu constructors copy onto the stack; a whole
// struct assignment is what agbcc expands to `ldmia/stmia r0!,{r2,r3,r4}`.
typedef struct { u32 w[4]; } MenuCtor_Tmpl;

// ============================================================================
// menu_ctor2.s 0x0800C1E4 — menu record-1 constructor: stack context init,
// template attach, manager/queue wiring, two resource lookups.
// ============================================================================
void MenuCtor2_0C1E4(void *rec) {
    u8 *r5 = (u8 *)rec;
    u8 stk[84];
    u8 *r4 = stk;
    // Two values of 1, and the ROM keeps them in two DIFFERENT hard registers:
    // `one` in r6 (`strb r6,[r4,#4]` at +0x26 and `strb r6,[r4,#9]` at +0x2C,
    // both after the `_08002E104` call) and the halfword constant in r9
    // (`mov r1,r9 / strh r1,[r4,#2]` at +0x18 and `mov r1,r9 / strh r1,[r0,#0]`
    // at +0x56). That split is what produces the prologue
    // `mov r6,r9 / mov r5,r8 / push {r5,r6}`.
//
    // `one` is pinned to r6 with a register variable, and the pin is what
    // closes the body. Unpinned, agbcc emits the two set-ups in the opposite
    // order -- `movs r0,#1 / mov r9,r0` then `movs r6,#1` -- and the body is
    // 219/224 at prefix 19 with a 5-byte inversion at +0x12; the reload pass
    // places the r9 constant's set-up at the head of the block that holds its
    // first use, ahead of the expand-time local init in that same block. No
    // source *shape* moves the init: the two constants' types (u8/u16/u32/s32/
    // const), both-or-either being a named local, declaration order, explicit
    // nesting, the byte stores moved ahead of the halfword store, and an
    // assign-after-declare form were all measured at 219/224 prefix 19. The pin
    // changes the allocation, not the order, and the allocation is what the
    // reload keys on.
//
    // The pin and the named `k1` are NOT independent -- each alone leaves the
    // body at 219/224:
    //   pin `one` only, literal `1u` in the halfword store -> prefix 24 but
    //     only 66/224 matched: the constant's uses get copy-propagated into
    //     the scratch (`strh r0,[r4,#2]`, `mov r4,r9 / strh r4,[r0,#0]`)
    //     instead of `mov r1,r9`.
    //   named `k1` only, `one` a plain local             -> 219/224 prefix 19.
    // Naming the halfword constant `k1` keeps its pseudo a separate live
    // value, so the use-site reload picks r1 and emits the ROM's `mov r1,r9`.
//
    // `k1` is a plain local on purpose. Pinning it too (`register u16 k1
    // __asm__("r9")`) costs 4 bytes -- the set-up collapses to a single
    // `movs` and the span becomes 220, not 224 (prefix 22). Two `register
    //... __asm__` declarations cannot share one declaration block at all:
    // tools/agbcc_c89_transform.py gives up with "budget exhausted" and drops
    // the whole TU.
//
    // `one` stays declared AFTER the zero store, and that position is still
    // load-bearing: declared first, agbcc issues the `movs r6,#1` ahead of the
    // `strh` and the body diverges at +0x0E (215/224, prefix 14).
    *(u16 *)(r4 + 0) = 0u;
    register u8 one __asm__("r6");
    one = 1u;
    u16 k1 = 1u;
    *(u16 *)(r4 + 2) = k1;
    _08002E104(stk + 4, 0, 64u);
    *(r4 + 4) = one;
    *(r4 + 6) = 16u;
    *(r4 + 9) = one;
    *(void **)(stk + 16) = r5 + 0xCE4;
    u8 *r8 = stk + 68;
    *(MenuCtor_Tmpl *)(uintptr_t)r8 = *(const MenuCtor_Tmpl *)(uintptr_t)0x0805F6C4u;
    _080056FC();
    _080055D8((u32)(uintptr_t)(r5 + 0x1CE4), 16u);
    *(u16 *)(r5 + 92) = 1u;
    u8 *rr4 = r5 + 8;
    _08002D974(stk, rr4, 0x04000011u);
    u8 *rr6 = r5 + 76;
    _08002D974(r8, rr6, 0x04000004u);
    _08004D4C(2u, (u32)(uintptr_t)rr4, 0u);
    _080050E8(rr4, 0);
    _080032E0(rr6);
    _08003104(r5 + 100);
    *(void **)(r5 + 0x1DE4) = _08004DC8((void *)(uintptr_t)0x080CDFD0u);
    *(void **)(r5 + 0x1DE8) = _08004DC8((void *)(uintptr_t)0x080CDFD8u);
}
void _0800C1E4(void *r) __attribute__((alias("MenuCtor2_0C1E4")));
void sub_0800C1E4(void *r) __attribute__((alias("MenuCtor2_0C1E4")));

// ============================================================================
// menu_ctor.s 0x0800C010 — menu/save scene constructor (twin of C1E4 with
// its own template, offsets, and a trailing _08004EF0 publish).
// ============================================================================
// The ROM copies the 16-byte template with `ldmia r0!,{r2,r3,r4} / stmia r1!,
// {r2,r3,r4}` (0x0800C04C), which is agbcc's block move for a whole-struct
// assignment; four scalar word copies do not produce it. The two `1` stores
// and the `*(u16*)(r5+92) = 1` store come out of one hoisted constant, which is
// why the prologue saves r8 AND r9 (`mov r6,r9 / mov r5,r8 / push {r5,r6}`).
void MenuCtor_0C010(void *rec) {
    u8 *r5 = (u8 *)rec;
    u8 stk[84];
    u8 *r4 = stk;
    // Two values of 1 in two different hard registers, exactly as in
    // MenuCtor2_0C1E4 above, and the same closing shape applies unchanged: the
    // r6 pin on `one` plus a *named* halfword constant `k1`. The ROM here is
    // `+0x12 movs r6,#1 / +0x14 movs r0,#1 / +0x16 mov r9,r0 / +0x18 mov r1,r9
    // / +0x1A strh r1,[r4,#2]`, with `one` read back at +0x36 `strb r6,[r4,#4]`
    // and +0x3C `strb r6,[r4,#7]`, and r9 again at the `(u16*)(r5+92)` store.
//
    // Both levers are required and neither is sufficient, measured here
    // independently of the twin: pin only (literal `1u` in the halfword store)
    // reaches prefix 24 with the constant copy-propagated into the scratch
    // instead of `mov r1,r9`; named `k1` only (plain `u8 one`) stays at
    // 219/224 prefix 19. See the full control list on MenuCtor2_0C1E4.
    *(u16 *)(r4 + 0) = 0u;
    register u8 one __asm__("r6");
    one = 1u;
    u16 k1 = 1u;
    *(u16 *)(r4 + 2) = k1;
    _08002E104(stk + 4, 0, 64u);
    *(r4 + 4) = one;
    *(r4 + 6) = 24u;
    *(r4 + 7) = one;
    *(void **)(stk + 16) = r5 + 0xCE0;
    u8 *r8 = stk + 68;
    *(MenuCtor_Tmpl *)(uintptr_t)r8 = *(const MenuCtor_Tmpl *)(uintptr_t)0x0805F6B4u;
    _080056FC();
    _080055D8((u32)(uintptr_t)(r5 + 0x1CE0), 16u);
    *(u16 *)(r5 + 92) = 1u;
    u8 *rr4 = r5 + 8;
    _08002D974(stk, rr4, 0x04000011u);
    u8 *rr6 = r5 + 76;
    _08002D974(r8, rr6, 0x04000004u);
    _08004D4C(2u, (u32)(uintptr_t)rr4, 0u);
    _080050E8(rr4, 0);
    _080032E0(rr6);
    _08003104(r5 + 96);
    // `rr` is formed AFTER the first _08004DC8 call: the ROM computes
    // `adds r4,r5,r2` at 0x0800C0B4, past the `bl` at 0x0800C0AC. Declaring it
    // before the call sinks the address computation ahead of the call instead.
    void *a = _08004DC8((void *)(uintptr_t)0x080CDFD0u);
    u8 *rr = r5 + 0x1DE0;
    *(void **)rr = a;
    *(void **)(r5 + 0x1DE4) = _08004DC8((void *)(uintptr_t)0x080CDFD8u);
    _08004EF0(*(void **)rr);
}
void _0800C010(void *r) __attribute__((alias("MenuCtor_0C010")));
void sub_0800C010(void *r) __attribute__((alias("MenuCtor_0C010")));

// ============================================================================
// menu_c4d0.s 0x0800C4D0 — menu record-20 constructor: explicit stack-context
// bytes, twin memset blocks, template 0x0805F6D4, C454/C4AC pair, triple
// resource lookup.
// ============================================================================
__attribute__((naked)) void MenuCtor20_0C4D0(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush {r4, r5, r6, r7, lr}\n"
        "\tmov r7, r8\n"
        "\tpush {r7}\n"
        "\tsub sp, #84\n"
        "\tadds r5, r0, #0\n"
        "\tmov r0, sp\n"
        "\tmovs r2, #0\n"
        "\tmovs r3, #0\n"
        "\tstrh r3, [r0]\n"
        "\tmovs r4, #1\n"
        "\tmovs r6, #1\n"
        "\tstrh r6, [r0, #2]\n"
        "\tstrb r4, [r0, #4]\n"
        "\tstrb r2, [r0, #5]\n"
        "\tmovs r1, #29\n"
        "\tstrb r1, [r0, #6]\n"
        "\tstrb r2, [r0, #7]\n"
        "\tstrb r4, [r0, #8]\n"
        "\tstrb r4, [r0, #9]\n"
        "\tstrb r2, [r0, #10]\n"
        "\tstrb r2, [r0, #11]\n"
        "\tstrh r3, [r0, #12]\n"
        "\tstrh r3, [r0, #14]\n"
        "\tldr r7, .Lmc4d0_c5d4\n"
        "\tadds r1, r5, r7\n"
        "\tstr r1, [sp, #16]\n"
        "\tstrb r2, [r0, #20]\n"
        "\tmovs r1, #3\n"
        "\tstrb r1, [r0, #21]\n"
        "\tmovs r1, #31\n"
        "\tstrb r1, [r0, #22]\n"
        "\tstrb r2, [r0, #23]\n"
        "\tstrb r2, [r0, #24]\n"
        "\tstrb r4, [r0, #25]\n"
        "\tstrb r2, [r0, #26]\n"
        "\tstrb r2, [r0, #27]\n"
        "\tstrh r3, [r0, #28]\n"
        "\tstrh r3, [r0, #30]\n"
        "\tldr r1, .Lmc4d0_c5d8\n"
        "\tadds r0, r5, r1\n"
        "\tstr r0, [sp, #32]\n"
        "\tadd r0, sp, #36\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #16\n"
        "\tbl 0x0802E104\n"
        "\tadd r0, sp, #52\n"
        "\tmovs r1, #0\n"
        "\tmovs r2, #16\n"
        "\tbl 0x0802E104\n"
        "\tadd r2, sp, #68\n"
        "\tmov r8, r2\n"
        "\tmov r1, r8\n"
        "\tldr r0, .Lmc4d0_c5dc\n"
        "\tldmia r0!, {r2, r3, r7}\n"
        "\tstmia r1!, {r2, r3, r7}\n"
        "\tldr r0, [r0]\n"
        "\tstr r0, [r1]\n"
        "\tadds r0, r5, #0\n"
        "\tadds r0, #96\n"
        "\tstrb r4, [r0]\n"
        "\tbl 0x080056FC\n"
        "\tldr r3, .Lmc4d0_c5e0\n"
        "\tadds r0, r5, r3\n"
        "\tmovs r1, #16\n"
        "\tbl 0x080055D8\n"
        "\tadds r0, r5, #0\n"
        "\tadds r0, #92\n"
        "\tstrh r6, [r0]\n"
        "\tadds r4, r5, #0\n"
        "\tadds r4, #8\n"
        "\tldr r2, .Lmc4d0_c5e4\n"
        "\tmov r0, sp\n"
        "\tadds r1, r4, #0\n"
        "\tbl 0x0802D974\n"
        "\tadds r6, r5, #0\n"
        "\tadds r6, #76\n"
        "\tldr r2, .Lmc4d0_c5e8\n"
        "\tmov r0, r8\n"
        "\tadds r1, r6, #0\n"
        "\tbl 0x0802D974\n"
        "\tmovs r0, #2\n"
        "\tadds r1, r4, #0\n"
        "\tmovs r2, #0\n"
        "\tbl 0x08004D4C\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #0\n"
        "\tbl 0x080050E8\n"
        "\tadds r0, r6, #0\n"
        "\tbl 0x080032E0\n"
        "\tbl 0x0800C454\n"
        "\tbl 0x0800C4AC\n"
        "\tadds r0, r5, #0\n"
        "\tadds r0, #100\n"
        "\tbl 0x08003104\n"
        "\tldr r0, .Lmc4d0_c5ec\n"
        "\tbl 0x08004DC8\n"
        "\tldr r7, .Lmc4d0_c5f0\n"
        "\tadds r1, r5, r7\n"
        "\tstr r0, [r1]\n"
        "\tldr r0, .Lmc4d0_c5f4\n"
        "\tbl 0x08004DC8\n"
        "\tldr r2, .Lmc4d0_c5f8\n"
        "\tadds r1, r5, r2\n"
        "\tstr r0, [r1]\n"
        "\tldr r0, .Lmc4d0_c5fc\n"
        "\tbl 0x08004DC8\n"
        "\tldr r3, .Lmc4d0_c600\n"
        "\tadds r5, r5, r3\n"
        "\tstr r0, [r5]\n"
        "\tadd sp, #84\n"
        "\tpop {r3}\n"
        "\tmov r8, r3\n"
        "\tpop {r4, r5, r6, r7}\n"
        "\tpop {r0}\n"
        "\tbx r0\n"
        ".Lmc4d0_c5d4:\n"
        "\t.word 0x00000CE4\n"
        ".Lmc4d0_c5d8:\n"
        "\t.word 0x00001CE4\n"
        ".Lmc4d0_c5dc:\n"
        "\t.word 0x0805F6D4\n"
        ".Lmc4d0_c5e0:\n"
        "\t.word 0x000024E4\n"
        ".Lmc4d0_c5e4:\n"
        "\t.word 0x04000011\n"
        ".Lmc4d0_c5e8:\n"
        "\t.word 0x04000004\n"
        ".Lmc4d0_c5ec:\n"
        "\t.word 0x080CDFD0\n"
        ".Lmc4d0_c5f0:\n"
        "\t.word 0x000025E4\n"
        ".Lmc4d0_c5f4:\n"
        "\t.word 0x080CDFD8\n"
        ".Lmc4d0_c5f8:\n"
        "\t.word 0x000025E8\n"
        ".Lmc4d0_c5fc:\n"
        "\t.word 0x080CDFE0\n"
        ".Lmc4d0_c600:\n"
        "\t.word 0x000025EC\n"
        ".syntax divided\n"
    );
}

void _0800C4D0(void *r) __attribute__((alias("MenuCtor20_0C4D0")));
void sub_0800C4D0(void *r) __attribute__((alias("MenuCtor20_0C4D0")));

// ============================================================================
// menu_cae4.s 0x0800CAE4 — menu/results detail renderer: template blit,
// four-column car lanes, template word lane, indexed text lookups.
// ============================================================================
__attribute__((naked)) void MenuDetailRender_0CAE4(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Lmcae4_0800cae4:\n"
        "\tpush {r4, r5, r6, r7, lr}\n"
        "\tsub sp, #56\n"
        "\tadds r7, r0, #0\n"
        "\tadd r5, sp, #28\n"
        "\tadds r1, r5, #0\n"
        "\tldr r0, .Lmcae4_0800cc08\n"
        "\tldmia r0!, {r2, r3, r4}\n"
        "\tstmia r1!, {r2, r3, r4}\n"
        "\tldmia r0!, {r2, r3, r4}\n"
        "\tstmia r1!, {r2, r3, r4}\n"
        "\tldr r0, [r0, #0]\n"
        "\tstr r0, [r1, #0]\n"
        "\tldr r1, .Lmcae4_0800cc0c\n"
        "\tmovs r0, #4\n"
        "\tbl 0x08003940\n"
        "\tldr r4, .Lmcae4_0800cc10\n"
        "\tbl 0x08002044\n"
        "\tadds r3, r0, #0\n"
        "\tmovs r0, #20\n"
        "\tmovs r1, #20\n"
        "\tadds r2, r4, #0\n"
        "\tbl 0x08003F18\n"
        "\tmovs r6, #0\n"
        ".Lmcae4_0800cb18:\n"
        "\tmovs r0, #50\n"
        "\tadds r4, r6, #0\n"
        "\tmuls r4, r0\n"
        "\tadds r4, #50\n"
        "\tadds r0, r6, #0\n"
        "\tbl 0x0800206C\n"
        "\tadds r2, r0, #0\n"
        "\tlsls r2, r2, #16\n"
        "\tlsrs r2, r2, #16\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #30\n"
        "\tbl 0x08003978\n"
        "\tadds r0, r6, #0\n"
        "\tmovs r1, #0\n"
        "\tbl 0x08001E5C\n"
        "\tadds r2, r0, #0\n"
        "\tlsls r2, r2, #16\n"
        "\tlsrs r2, r2, #16\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #40\n"
        "\tbl 0x08003978\n"
        "\tadds r0, r6, #0\n"
        "\tmovs r1, #0\n"
        "\tbl 0x08002178\n"
        "\tadds r2, r0, #0\n"
        "\tlsls r2, r2, #16\n"
        "\tlsrs r2, r2, #16\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #50\n"
        "\tbl 0x08003978\n"
        "\tadds r6, #1\n"
        "\tcmp r6, #3\n"
        "\tble.n .Lmcae4_0800cb18\n"
        "\tmovs r4, #70\n"
        "\tmovs r6, #6\n"
        ".Lmcae4_0800cb6a:\n"
        "\tldmia r5!, {r2}\n"
        "\tmovs r0, #100\n"
        "\tadds r1, r4, #0\n"
        "\tbl 0x08003838\n"
        "\tadds r4, #10\n"
        "\tsubs r6, #1\n"
        "\tcmp r6, #0\n"
        "\tbge.n .Lmcae4_0800cb6a\n"
        "\tmovs r1, #0\n"
        "\tldrsh r0, [r7, r1]\n"
        "\tlsls r1, r0, #2\n"
        "\tadds r1, r1, r0\n"
        "\tlsls r1, r1, #1\n"
        "\tadds r1, #70\n"
        "\tldr r2, .Lmcae4_0800cc14\n"
        "\tmovs r0, #90\n"
        "\tbl 0x08003838\n"
        "\tldr r1, .Lmcae4_0800cc18\n"
        "\tldr r4, .Lmcae4_0800cc1c\n"
        "\tldr r2, .Lmcae4_0800cc20\n"
        "\tadds r0, r4, r2\n"
        "\tmovs r3, #0\n"
        "\tldrsh r0, [r0, r3]\n"
        "\tlsls r0, r0, #2\n"
        "\tadds r0, r0, r1\n"
        "\tldr r2, [r0, #0]\n"
        "\tmovs r0, #144\n"
        "\tmovs r1, #70\n"
        "\tbl 0x08003838\n"
        "\tldr r1, .Lmcae4_0800cc24\n"
        "\tadds r0, r4, r1\n"
        "\tmovs r3, #0\n"
        "\tldrsh r2, [r0, r3]\n"
        "\tmovs r0, #160\n"
        "\tmovs r1, #80\n"
        "\tbl 0x08003954\n"
        "\tldr r1, .Lmcae4_0800cc28\n"
        "\tldr r2, .Lmcae4_0800cc2c\n"
        "\tadds r0, r4, r2\n"
        "\tmovs r3, #0\n"
        "\tldrsh r0, [r0, r3]\n"
        "\tlsls r0, r0, #2\n"
        "\tadds r0, r0, r1\n"
        "\tldr r2, [r0, #0]\n"
        "\tmovs r0, #184\n"
        "\tmovs r1, #70\n"
        "\tbl 0x08003838\n"
        "\tldr r1, .Lmcae4_0800cc30\n"
        "\tldr r0, .Lmcae4_0800cc34\n"
        "\tadds r4, r4, r0\n"
        "\tmovs r2, #0\n"
        "\tldrsh r0, [r4, r2]\n"
        "\tlsls r0, r0, #2\n"
        "\tadds r0, r0, r1\n"
        "\tldr r2, [r0, #0]\n"
        "\tmovs r0, #184\n"
        "\tmovs r1, #80\n"
        "\tbl 0x08003838\n"
        "\tmovs r2, #29\n"
        "\tldrsb r2, [r7, r2]\n"
        "\tmovs r0, #160\n"
        "\tmovs r1, #130\n"
        "\tbl 0x08003954\n"
        "\tldrh r1, [r7, #0]\n"
        "\tmovs r0, #0\n"
        "\tbl 0x08002158\n"
        "\tadd sp, #56\n"
        "\tpop {r4, r5, r6, r7}\n"
        "\tpop {r0}\n"
        "\tbx r0\n"
        "\t.short 0\n"
        ".Lmcae4_0800cc08: .word 0x0805F850\n"
        ".Lmcae4_0800cc0c: .word 0x0805F86C\n"
        ".Lmcae4_0800cc10: .word 0x0805F87C\n"
        ".Lmcae4_0800cc14: .word 0x0805F888\n"
        ".Lmcae4_0800cc18: .word 0x080CB2C0\n"
        ".Lmcae4_0800cc1c: .word 0x03001780\n"
        ".Lmcae4_0800cc20: .word 0x00000576\n"
        ".Lmcae4_0800cc24: .word 0x00000574\n"
        ".Lmcae4_0800cc28: .word 0x080CB3B8\n"
        ".Lmcae4_0800cc2c: .word 0x0000113C\n"
        ".Lmcae4_0800cc30: .word 0x080CB3C4\n"
        ".Lmcae4_0800cc34: .word 0x0000113E\n"
        ".syntax divided\n"
    );
}

void _0800CAE4(void *r) __attribute__((alias("MenuDetailRender_0CAE4")));
void sub_0800CAE4(void *r) __attribute__((alias("MenuDetailRender_0CAE4")));

// ============================================================================
// menu_c884.s 0x0800C884 — menu record handler: key-scan gate (provably
// always r6=0 — transcribed literally regardless), continue-path packet,
// bit-lane transitions, cursor walks, WA counter lanes, final HSL dispatch
// to the _08002158 packet sink.
// ============================================================================
__attribute__((naked)) void MenuRecordHandler_0C884(void *rec, u32 a1, u32 a2) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Lmc884_0800c884:\n"
        "\tpush {r4, r5, r6, r7, lr}\n"
        "\tmov r7, sl\n"
        "\tmov r6, r9\n"
        "\tmov r5, r8\n"
        "\tpush {r5, r6, r7}\n"
        "\tadds r5, r0, #0\n"
        "\tlsls r1, r1, #16\n"
        "\tlsrs r1, r1, #16\n"
        "\tmov r9, r1\n"
        "\tlsls r2, r2, #16\n"
        "\tlsrs r7, r2, #16\n"
        "\tbl 0x080024A0\n"
        "\tlsls r0, r0, #16\n"
        "\tlsrs r0, r0, #16\n"
        "\tmov sl, r0\n"
        "\tmovs r0, #0\n"
        "\tmov r8, r0\n"
        "\tmovs r6, #1\n"
        "\tbl 0x08004B68\n"
        "\tbl 0x08002140\n"
        "\tcmp r0, #1\n"
        "\tble.n .Lmc884_0800c8e8\n"
        "\tmovs r4, #0\n"
        "\tb.n .Lmc884_0800c8de\n"
        ".Lmc884_0800c8ba:\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #2\n"
        "\tbl 0x08002178\n"
        "\tlsls r0, r0, #16\n"
        "\tlsrs r0, r0, #16\n"
        "\tcmp r0, #5\n"
        "\tbeq.n .Lmc884_0800c8cc\n"
        "\tmovs r6, #0\n"
        ".Lmc884_0800c8cc:\n"
        "\tadds r0, r4, #0\n"
        "\tmovs r1, #3\n"
        "\tbl 0x08002178\n"
        "\tlsls r0, r0, #16\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800c8dc\n"
        "\tmovs r6, #0\n"
        ".Lmc884_0800c8dc:\n"
        "\tadds r4, #1\n"
        ".Lmc884_0800c8de:\n"
        "\tbl 0x08002140\n"
        "\tcmp r4, r0\n"
        "\tblt.n .Lmc884_0800c8ba\n"
        "\tb.n .Lmc884_0800c8ea\n"
        ".Lmc884_0800c8e8:\n"
        "\tmovs r6, #0\n"
        ".Lmc884_0800c8ea:\n"
        "\tcmp r6, #0\n"
        "\tbeq.n .Lmc884_0800c906\n"
        "\tldrh r1, [r5, #4]\n"
        "\tmovs r2, #4\n"
        "\tldrsh r0, [r5, r2]\n"
        "\tcmp r0, #0\n"
        "\tble.n .Lmc884_0800c8fe\n"
        "\tsubs r0, r1, #1\n"
        "\tstrh r0, [r5, #4]\n"
        "\tb.n .Lmc884_0800c990\n"
        ".Lmc884_0800c8fe:\n"
        "\tadds r0, r5, #0\n"
        "\tbl 0x0800C814\n"
        "\tb.n .Lmc884_0800c990\n"
        ".Lmc884_0800c906:\n"
        "\tcmp r7, #1\n"
        "\tbne.n .Lmc884_0800c926\n"
        "\tldrh r3, [r5, #0]\n"
        "\tcmp r3, #6\n"
        "\tbne.n .Lmc884_0800c926\n"
        "\tmovs r1, #0\n"
        "\tldrsh r0, [r5, r1]\n"
        "\tmovs r1, #29\n"
        "\tldrsb r1, [r5, r1]\n"
        "\tadds r0, r0, r1\n"
        "\tbl 0x08004BFC\n"
        "\tmovs r0, #1\n"
        "\tbl 0x08004EC0\n"
        "\tb.n .Lmc884_0800c982\n"
        ".Lmc884_0800c926:\n"
        "\tmovs r0, #1\n"
        "\tands r0, r7\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800c982\n"
        "\tmovs r2, #0\n"
        "\tldrsh r0, [r5, r2]\n"
        "\tcmp r0, #5\n"
        "\tbeq.n .Lmc884_0800c982\n"
        "\tcmp r0, #1\n"
        "\tbeq.n .Lmc884_0800c982\n"
        "\tmovs r0, #128\n"
        "\tlsls r0, r0, #2\n"
        "\tmov r3, r9\n"
        "\tands r0, r3\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800c94e\n"
        "\tmovs r0, #100\n"
        "\tbl 0x08004BFC\n"
        "\tb.n .Lmc884_0800c96a\n"
        ".Lmc884_0800c94e:\n"
        "\tmovs r0, #128\n"
        "\tlsls r0, r0, #1\n"
        "\tmov r1, r9\n"
        "\tands r0, r1\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800c962\n"
        "\tmovs r0, #101\n"
        "\tbl 0x08004BFC\n"
        "\tb.n .Lmc884_0800c96a\n"
        ".Lmc884_0800c962:\n"
        "\tmovs r2, #0\n"
        "\tldrsh r0, [r5, r2]\n"
        "\tbl 0x08004BFC\n"
        ".Lmc884_0800c96a:\n"
        "\tmovs r3, #0\n"
        "\tldrsh r1, [r5, r3]\n"
        "\tmovs r0, #0\n"
        "\tbl 0x08004C84\n"
        "\tbl 0x08002044\n"
        "\tbl 0x08001F80\n"
        "\tmovs r0, #1\n"
        "\tbl 0x08004EC0\n"
        ".Lmc884_0800c982:\n"
        "\tmovs r0, #2\n"
        "\tands r0, r7\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800c990\n"
        "\tmovs r0, #1\n"
        "\tbl 0x08004EA8\n"
        ".Lmc884_0800c990:\n"
        "\tmovs r0, #64\n"
        "\tands r0, r7\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800c9a6\n"
        "\tldrh r1, [r5, #0]\n"
        "\tmovs r2, #0\n"
        "\tldrsh r0, [r5, r2]\n"
        "\tcmp r0, #0\n"
        "\tble.n .Lmc884_0800c9a6\n"
        "\tsubs r0, r1, #1\n"
        "\tstrh r0, [r5, #0]\n"
        ".Lmc884_0800c9a6:\n"
        "\tmovs r0, #128\n"
        "\tands r0, r7\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800c9bc\n"
        "\tldrh r1, [r5, #0]\n"
        "\tmovs r3, #0\n"
        "\tldrsh r0, [r5, r3]\n"
        "\tcmp r0, #5\n"
        "\tbgt.n .Lmc884_0800c9bc\n"
        "\tadds r0, r1, #1\n"
        "\tstrh r0, [r5, #0]\n"
        ".Lmc884_0800c9bc:\n"
        "\tmovs r0, #32\n"
        "\tmov r1, sl\n"
        "\tands r0, r1\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800c9cc\n"
        "\tmovs r2, #1\n"
        "\tnegs r2, r2\n"
        "\tadd r8, r2\n"
        ".Lmc884_0800c9cc:\n"
        "\tmovs r0, #16\n"
        "\tmov r3, sl\n"
        "\tands r0, r3\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800c9da\n"
        "\tmovs r0, #1\n"
        "\tadd r8, r0\n"
        ".Lmc884_0800c9da:\n"
        "\tmovs r0, #128\n"
        "\tlsls r0, r0, #2\n"
        "\tands r0, r7\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800ca02\n"
        "\tmovs r1, #0\n"
        "\tldrsh r2, [r5, r1]\n"
        "\tcmp r2, #0\n"
        "\tbne.n .Lmc884_0800ca02\n"
        "\tldr r0, .Lmc884_0800ca3c\n"
        "\tldr r3, .Lmc884_0800ca40\n"
        "\tadds r1, r0, r3\n"
        "\tldrh r0, [r1, #0]\n"
        "\tadds r0, #1\n"
        "\tstrh r0, [r1, #0]\n"
        "\tlsls r0, r0, #16\n"
        "\tasrs r0, r0, #16\n"
        "\tcmp r0, #2\n"
        "\tble.n .Lmc884_0800ca02\n"
        "\tstrh r2, [r1, #0]\n"
        ".Lmc884_0800ca02:\n"
        "\tmovs r0, #128\n"
        "\tlsls r0, r0, #1\n"
        "\tands r0, r7\n"
        "\tcmp r0, #0\n"
        "\tbeq.n .Lmc884_0800ca2a\n"
        "\tmovs r0, #0\n"
        "\tldrsh r2, [r5, r0]\n"
        "\tcmp r2, #0\n"
        "\tbne.n .Lmc884_0800ca2a\n"
        "\tldr r0, .Lmc884_0800ca3c\n"
        "\tldr r3, .Lmc884_0800ca44\n"
        "\tadds r1, r0, r3\n"
        "\tldrh r0, [r1, #0]\n"
        "\tadds r0, #1\n"
        "\tstrh r0, [r1, #0]\n"
        "\tlsls r0, r0, #16\n"
        "\tasrs r0, r0, #16\n"
        "\tcmp r0, #2\n"
        "\tble.n .Lmc884_0800ca2a\n"
        "\tstrh r2, [r1, #0]\n"
        ".Lmc884_0800ca2a:\n"
        "\tmovs r0, #0\n"
        "\tldrsh r1, [r5, r0]\n"
        "\tcmp r1, #1\n"
        "\tbeq.n .Lmc884_0800ca6c\n"
        "\tcmp r1, #1\n"
        "\tbgt.n .Lmc884_0800ca48\n"
        "\tcmp r1, #0\n"
        "\tbeq.n .Lmc884_0800ca4e\n"
        "\tb.n .Lmc884_0800cab4\n"
        ".Lmc884_0800ca3c: .word 0x03001780\n"
        ".Lmc884_0800ca40: .word 0x0000113C\n"
        ".Lmc884_0800ca44: .word 0x0000113E\n"
        ".Lmc884_0800ca48:\n"
        "\tcmp r1, #6\n"
        "\tbeq.n .Lmc884_0800ca98\n"
        "\tb.n .Lmc884_0800cab4\n"
        ".Lmc884_0800ca4e:\n"
        "\tldr r0, .Lmc884_0800ca64\n"
        "\tldr r3, .Lmc884_0800ca68\n"
        "\tadds r2, r0, r3\n"
        "\tldrh r0, [r2, #0]\n"
        "\tadd r0, r8\n"
        "\tstrh r0, [r2, #0]\n"
        "\tlsls r0, r0, #16\n"
        "\tcmp r0, #0\n"
        "\tbge.n .Lmc884_0800cab4\n"
        "\tstrh r1, [r2, #0]\n"
        "\tb.n .Lmc884_0800cab4\n"
        ".Lmc884_0800ca64: .word 0x03001780\n"
        ".Lmc884_0800ca68: .word 0x00000576\n"
        ".Lmc884_0800ca6c:\n"
        "\tldr r0, .Lmc884_0800ca90\n"
        "\tldr r2, .Lmc884_0800ca94\n"
        "\tadds r1, r0, r2\n"
        "\tldrh r0, [r1, #0]\n"
        "\tadd r0, r8\n"
        "\tstrh r0, [r1, #0]\n"
        "\tlsls r0, r0, #16\n"
        "\tcmp r0, #0\n"
        "\tbge.n .Lmc884_0800ca82\n"
        "\tmovs r0, #0\n"
        "\tstrh r0, [r1, #0]\n"
        ".Lmc884_0800ca82:\n"
        "\tmovs r3, #0\n"
        "\tldrsh r0, [r1, r3]\n"
        "\tcmp r0, #96\n"
        "\tble.n .Lmc884_0800cab4\n"
        "\tmovs r0, #96\n"
        "\tstrh r0, [r1, #0]\n"
        "\tb.n .Lmc884_0800cab4\n"
        ".Lmc884_0800ca90: .word 0x03001780\n"
        ".Lmc884_0800ca94: .word 0x00000574\n"
        ".Lmc884_0800ca98:\n"
        "\tldrb r0, [r5, #29]\n"
        "\tadd r0, r8\n"
        "\tstrb r0, [r5, #29]\n"
        "\tlsls r0, r0, #24\n"
        "\tcmp r0, #0\n"
        "\tbge.n .Lmc884_0800caa8\n"
        "\tmovs r0, #0\n"
        "\tstrb r0, [r5, #29]\n"
        ".Lmc884_0800caa8:\n"
        "\tmovs r0, #29\n"
        "\tldrsb r0, [r5, r0]\n"
        "\tcmp r0, #4\n"
        "\tble.n .Lmc884_0800cab4\n"
        "\tmovs r0, #4\n"
        "\tstrb r0, [r5, #29]\n"
        ".Lmc884_0800cab4:\n"
        "\tmovs r0, #0\n"
        "\tmov r1, r9\n"
        "\tbl 0x08002158\n"
        "\tmovs r0, #1\n"
        "\tadds r1, r7, #0\n"
        "\tbl 0x08002158\n"
        "\tldrh r1, [r5, #0]\n"
        "\tmovs r0, #2\n"
        "\tbl 0x08002158\n"
        "\tldrb r1, [r5, #28]\n"
        "\tmovs r0, #3\n"
        "\tbl 0x08002158\n"
        "\tpop {r3, r4, r5}\n"
        "\tmov r8, r3\n"
        "\tmov r9, r4\n"
        "\tmov sl, r5\n"
        "\tpop {r4, r5, r6, r7}\n"
        "\tpop {r0}\n"
        "\tbx r0\n"
        "\t.short 0\n"
        ".syntax divided\n"
    );
}

void _0800C884(void *r, u32 a, u32 b) __attribute__((alias("MenuRecordHandler_0C884")));
void sub_0800C884(void *r, u32 a, u32 b) __attribute__((alias("MenuRecordHandler_0C884")));

// ============================================================================
// course_resource_leaf_more.s 0x08007B18 / more2.s 0x08007B8C — record
// emitters over the course-record array (kind feeds Course_Seek; r1-in to
// the seek is the only use of the kind slot... precisely: kind -> 07498).
// The bodies themselves are defined above, next to the emitter helpers they
// share; only the VMA aliases live here.
// ============================================================================
void _08007B18(void *r, int k, int x, int y, u32 a, u32 b, u32 c, u32 d)
    __attribute__((alias("CourseEmit7B18")));
void _08007B8C(void *r, int k, int x, int y, u32 a, u32 b, u32 c, u32 d)
    __attribute__((alias("CourseEmit7B8C")));

// ============================================================================
// menu_ff78.s 0x080011784 — record filler dispatcher based on s16[rec+168].
// ROM call sites here are `bl 0x08007B18` (0x117CC/0x117E6/0x11802/0x1182E/
// 0x11848), so the body calls the strong CourseEmit7B18 rather than the
// file-local CourseEmit7B18_Static: a file-local callee has no symbol in the
// scoped link closure, which is what produced UNRESOLVED_RELOCATION (5 sites).
// ============================================================================
void Sub_080011784(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 selector = *(s16 *)(uintptr_t)(rec + 168);

    switch (selector) {
    case 0: {
        u16 val = *(volatile u16 *)(uintptr_t)(rec + 214);
        if (val >= 5 && val <= 6) {
            CourseEmit7B18(rec_, 6, 10, 40, 4, 1, 1, selector);
        }
        break;
    }
    case 1:
        CourseEmit7B18(rec_, 7, 10, 56, 4, selector, selector, 0);
        break;
    case 2:
        CourseEmit7B18(rec_, 8, 10, 72, 4, 1, 1, 0);
        break;
    case 3: {
        s16 sub_selector = *(s16 *)(uintptr_t)(rec + 226);
        switch (sub_selector) {
        case 0:
            CourseEmit7B18(rec_, 11, 10, 88, 4, 1, 1, sub_selector);
            break;
        case 1:
            CourseEmit7B18(rec_, 5, 10, 88, 4, sub_selector, sub_selector, 0);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}
void sub_080011784(void *a) __attribute__((alias("Sub_080011784")));
void _080011784(void *a) __attribute__((alias("Sub_080011784")));
// The 208-byte ROM span ends in 00 00; gas otherwise pads this Thumb section
// with a nop after the matching 206-byte instruction stream.
__asm__(".align 2, 0");

// ============================================================================
// Static faithful _08007770 course-resource setup (course_resource_helpers.s
// 0x08007770). Kept as a file-local copy of the now-faithful
// Course_0x08007770: 6 machine arguments; source 053DC is
// 0748C(r7) (not r3in), DMA base 0x05000000.
// ============================================================================
static void CourseSetup7770_Static(void *ctx, void *X, int r2in, int r3in,
                                   u32 s0, u32 s1) {
    void *r4 = _08007498(X, r2in);
    void *r5 = _08007498(X, 0);
    void *r6 = _08007498(r4, 1);
    void *r7 = _08007498(r4, 2);
    u8 *r8 = (u8 *)_0800748C(r4);
    int r9 = _080050D0((int)(uintptr_t)ctx, (int)RomLeaf7658(r5));
    void *r4b = _0800748C(r5);
    _08005260(ctx, r4b, r9, (int)_080074A8(r5)); // r3 = 074A8(r5) per ROM asm
    const volatile u8 *src = (const volatile u8 *)_0800748C(r7);
    u16 r8w0 = *(volatile u16 *)(r8 + 0);
    u16 r8w2 = *(volatile u16 *)(r8 + 2);
    int stk0 = (int)(u16)(r8w0 >> 3);
    int stk1 = (int)(u16)(r8w2 >> 3);
    _080053DC((int)(uintptr_t)ctx, src, r3in, (int)s0, stk0, stk1, r9, (int)s1);
    volatile u32 *dma = (volatile u32 *)(uintptr_t)0x040000D4u;
    dma[0] = (u32)(uintptr_t)_0800748C(r6);
    dma[1] = 0x05000000u + (s1 << 5);
    dma[2] = (_080074A8(r6) >> 2) | 0x84000000u;
    (void)dma[2];
    _080054A4((int)(uintptr_t)ctx, r3in, (int)s0, stk0, stk1);
}

// projects the query delta (05DA4), clamps to the 2-attempt entry list and
// runs the half-plane + tile-range acceptance test (05E14), publishing the
// hit into the out record. Returns 1 on accept, 0 otherwise.
// ============================================================================
__attribute__((naked)) int CourseProximity_07210(void *state, void *out) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, sl\n"
        "mov r6, r9\n"
        "mov r5, r8\n"
        "push {r5, r6, r7}\n"
        "sub sp, #72\n"
        "adds r7, r1, #0\n"
        "movs r1, #0\n"
        "bl 0x08006C10\n"
        "mov r8, r0\n"
        "ldr r0, [r7, #8]\n"
        "cmp r0, #0\n"
        "bne.n .Lcp07210_08007234\n"
        "ldr r0, [r7, #12]\n"
        "cmp r0, #0\n"
        "bne.n .Lcp07210_08007234\n"
        "b.n .Lcp07210_08007356\n"
        ".Lcp07210_08007234:\n"
        "add r4, sp, #24\n"
        "ldr r0, [r7, #0]\n"
        "mov r2, r8\n"
        "ldr r1, [r2, #12]\n"
        "subs r0, r0, r1\n"
        "str r0, [sp, #24]\n"
        "ldr r0, [r7, #4]\n"
        "ldr r1, [r2, #16]\n"
        "subs r0, r0, r1\n"
        "str r0, [r4, #4]\n"
        "ldr r0, [r7, #8]\n"
        "str r0, [r4, #8]\n"
        "ldr r0, [r7, #12]\n"
        "str r0, [r4, #12]\n"
        "mov r0, sp\n"
        "adds r1, r4, #0\n"
        "bl 0x08005DA4\n"
        "ldr r2, [sp, #24]\n"
        "ldr r0, [r4, #8]\n"
        "adds r0, r2, r0\n"
        "str r0, [sp, #52]\n"
        "adds r5, r0, #0\n"
        "ldr r1, [r4, #4]\n"
        "ldr r0, [r4, #12]\n"
        "adds r0, r1, r0\n"
        "str r0, [sp, #56]\n"
        "adds r6, r0, #0\n"
        "cmp r5, r2\n"
        "ble.n .Lcp07210_08007272\n"
        "adds r5, r2, #0\n"
        ".Lcp07210_08007272:\n"
        "cmp r6, r1\n"
        "ble.n .Lcp07210_08007278\n"
        "adds r6, r1, #0\n"
        ".Lcp07210_08007278:\n"
        "ldr r3, [sp, #52]\n"
        "cmp r3, r2\n"
        "bge.n .Lcp07210_08007280\n"
        "str r2, [sp, #52]\n"
        ".Lcp07210_08007280:\n"
        "ldr r0, [sp, #56]\n"
        "cmp r0, r1\n"
        "bge.n .Lcp07210_08007288\n"
        "str r1, [sp, #56]\n"
        ".Lcp07210_08007288:\n"
        "movs r1, #0\n"
        "str r1, [sp, #48]\n"
        ".Lcp07210_0800728c:\n"
        "mov r2, sp\n"
        "adds r2, #40\n"
        "str r2, [sp, #60]\n"
        "mov r4, r8\n"
        "adds r4, #40\n"
        "mov r9, r1\n"
        ".Lcp07210_08007298:\n"
        "ldr r0, [sp, #60]\n"
        "mov r1, sp\n"
        "adds r2, r4, #0\n"
        "bl 0x08005E14\n"
        "lsls r0, r0, #24\n"
        "cmp r0, #0\n"
        "beq.n .Lcp07210_08007346\n"
        "mov r0, r8\n"
        "adds r0, #32\n"
        "add r0, r9\n"
        "ldr r1, [sp, #24]\n"
        "ldr r0, [r0, #0]\n"
        "subs r1, r1, r0\n"
        "str r1, [r7, #0]\n"
        "mov r0, r8\n"
        "adds r0, #36\n"
        "add r0, r9\n"
        "ldr r2, [sp, #28]\n"
        "ldr r0, [r0, #0]\n"
        "subs r2, r2, r0\n"
        "str r2, [r7, #4]\n"
        "mov r0, r8\n"
        "adds r0, #40\n"
        "add r0, r9\n"
        "mov sl, r0\n"
        "ldr r0, [r0, #0]\n"
        "muls r1, r0\n"
        "mov r0, r8\n"
        "adds r0, #44\n"
        "add r0, r9\n"
        "mov ip, r0\n"
        "ldr r0, [r0, #0]\n"
        "muls r0, r2\n"
        "adds r1, r1, r0\n"
        "ldr r3, [sp, #40]\n"
        "cmp r1, #0\n"
        "bge.n .Lcp07210_08007324\n"
        "cmp r5, r3\n"
        "bgt.n .Lcp07210_08007346\n"
        "ldr r0, [sp, #60]\n"
        "ldr r1, [r0, #4]\n"
        "cmp r6, r1\n"
        "bgt.n .Lcp07210_08007346\n"
        "ldr r2, [sp, #52]\n"
        "cmp r2, r3\n"
        "blt.n .Lcp07210_08007346\n"
        "ldr r0, [sp, #56]\n"
        "cmp r0, r1\n"
        "blt.n .Lcp07210_08007346\n"
        "movs r2, #12\n"
        "ldrsh r0, [r4, r2]\n"
        "asrs r2, r3, #8\n"
        "str r2, [sp, #68]\n"
        "cmp r0, r2\n"
        "bgt.n .Lcp07210_08007346\n"
        "movs r2, #14\n"
        "ldrsh r0, [r4, r2]\n"
        "asrs r1, r1, #8\n"
        "cmp r0, r1\n"
        "bgt.n .Lcp07210_08007346\n"
        "movs r2, #16\n"
        "ldrsh r0, [r4, r2]\n"
        "ldr r2, [sp, #68]\n"
        "cmp r0, r2\n"
        "blt.n .Lcp07210_08007346\n"
        "movs r2, #18\n"
        "ldrsh r0, [r4, r2]\n"
        "cmp r0, r1\n"
        "blt.n .Lcp07210_08007346\n"
        ".Lcp07210_08007324:\n"
        "mov r1, r8\n"
        "ldr r0, [r1, #12]\n"
        "adds r0, r3, r0\n"
        "str r0, [r7, #0]\n"
        "ldr r2, [sp, #60]\n"
        "ldr r0, [r2, #4]\n"
        "ldr r1, [r1, #16]\n"
        "adds r0, r0, r1\n"
        "str r0, [r7, #4]\n"
        "mov r3, sl\n"
        "ldr r0, [r3, #0]\n"
        "str r0, [r7, #8]\n"
        "mov r1, ip\n"
        "ldr r0, [r1, #0]\n"
        "str r0, [r7, #12]\n"
        "movs r0, #1\n"
        "b.n .Lcp07210_08007358\n"
        ".Lcp07210_08007346:\n"
        "adds r4, #28\n"
        "movs r2, #28\n"
        "add r9, r2\n"
        "ldr r3, [sp, #48]\n"
        "adds r3, #1\n"
        "str r3, [sp, #48]\n"
        "cmp r3, #1\n"
        "ble.n .Lcp07210_08007298\n"
        ".Lcp07210_08007356:\n"
        "movs r0, #0\n"
        ".Lcp07210_08007358:\n"
        "add sp, #72\n"
        "pop {r3, r4, r5}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "mov sl, r5\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r1}\n"
        "bx r1\n"
        ".syntax divided\n"
    );
}
int _08007210(void *s, void *o) __attribute__((alias("CourseProximity_07210")));
int sub_08007210(void *s, void *o) __attribute__((alias("CourseProximity_07210")));

// ============================================================================
// code_25264.s 0x08025264 — lineup builder: collects eligible car ids
// (skipping the one-make-cup reserves and unmapped ids), shuffles by
// signed-remainder indexing (another DivRemS consumer), insertion-ranks by
// catalog scores, and publishes picks + DivRem categories.
// ============================================================================
// ROM 0x080253DC and 0x08025430 call `bl 0x08025518` / `bl 0x08025530`
// DIRECTLY from the 0x08025264 body -- there is no helper function in the ROM
// for the score sum. A file-local `line_score` helper was therefore the wrong
// shape: it emitted a TU-local `bl line_score` that the scoped link closure
// cannot resolve (2 sites, UNRESOLVED_RELOCATION) in place of the two real
// `bl`s. Removed; both use sites now call the ROM callees directly.

__attribute__((naked)) void AiLineupBuilder_25264(int a0) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, sl\n"
        "\tmov\tr6, r9\n"
        "\tmov\tr5, r8\n"
        "\tpush\t{r5, r6, r7}\n"
        "\tsub\tsp, #216\n"
        "\tadds\tr4, r0, #0\n"
        "\tlsls\tr4, r4, #16\n"
        "\tlsrs\tr4, r4, #16\n"
        "\tmov\tr0, sp\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #196\n"
        "\tbl 0x0802E104\n"
        "\tadd\tr5, sp, #196\n"
        "\tadds\tr0, r5, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #16\n"
        "\tbl 0x0802E104\n"
        "\tlsls\tr4, r4, #16\n"
        "\tasrs\tr4, r4, #16\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl 0x08024E54\n"
        "\tadds\tr6, r0, #0\n"
        "\tmovs\tr7, #0\n"
        "\tldrh\tr1, [r6, #0]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r6, r2]\n"
        "\tmovs\tr2, #1\n"
        "\tnegs\tr2, r2\n"
        "\tcmp\tr0, r2\n"
        "\tbeq .Lai_lineup_25264_0800252FA\n"
        "\tadds\tr4, r2, #0\n"
        ".Lai_lineup_25264_0800252AA:\n"
        "\tlsls\tr0, r1, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tcmp\tr0, #86\n"
        "\tbeq .Lai_lineup_25264_0800252EE\n"
        "\tcmp\tr0, #56\n"
        "\tbeq .Lai_lineup_25264_0800252EE\n"
        "\tcmp\tr0, #55\n"
        "\tbeq .Lai_lineup_25264_0800252EE\n"
        "\tcmp\tr0, #31\n"
        "\tbeq .Lai_lineup_25264_0800252EE\n"
        "\tcmp\tr0, #14\n"
        "\tbeq .Lai_lineup_25264_0800252EE\n"
        "\tcmp\tr0, #98\n"
        "\tbeq .Lai_lineup_25264_0800252EE\n"
        "\tcmp\tr0, #54\n"
        "\tbeq .Lai_lineup_25264_0800252EE\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r6, r3]\n"
        "\tbl 0x080022E4\n"
        "\tlsls\tr0, r0, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tcmp\tr0, r4\n"
        "\tbeq .Lai_lineup_25264_0800252EE\n"
        "\tlsls\tr0, r7, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tlsls\tr1, r0, #1\n"
        "\tmov\tr3, sp\n"
        "\tadds\tr2, r3, r1\n"
        "\tldrh\tr1, [r6, #0]\n"
        "\tstrh\tr1, [r2, #0]\n"
        "\tadds\tr0, #1\n"
        "\tlsls\tr0, r0, #16\n"
        "\tlsrs\tr7, r0, #16\n"
        ".Lai_lineup_25264_0800252EE:\n"
        "\tadds\tr6, #2\n"
        "\tldrh\tr1, [r6, #0]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r6, r2]\n"
        "\tcmp\tr0, r4\n"
        "\tbne .Lai_lineup_25264_0800252AA\n"
        ".Lai_lineup_25264_0800252FA:\n"
        "\tlsls\tr4, r7, #16\n"
        "\tlsrs\tr2, r4, #16\n"
        "\tasrs\tr0, r4, #16\n"
        "\tldr r1, .Lai_lineup_25264_080025420\n"
        "\tldr r3, .Lai_lineup_25264_080025424\n"
        "\tadds\tr1, r1, r3\n"
        "\tmovs\tr6, #0\n"
        "\tldrsh\tr1, [r1, r6]\n"
        "\tsubs\tr1, #1\n"
        "\tcmp\tr0, r1\n"
        "\tbge .Lai_lineup_25264_08002533E\n"
        "\tlsls\tr0, r2, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tmov\tr8, r0\n"
        "\tadds\tr6, r1, #0\n"
        ".Lai_lineup_25264_080025318:\n"
        "\tasrs\tr4, r4, #16\n"
        "\tlsls\tr0, r4, #1\n"
        "\tmov\tr1, sp\n"
        "\tadds\tr5, r1, r0\n"
        "\tadds\tr0, r4, #0\n"
        "\tmov\tr1, r8\n"
        "\tbl 0x0802DE9C\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadd\tr0, sp\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tadds\tr4, #1\n"
        "\tlsls\tr4, r4, #16\n"
        "\tlsrs\tr7, r4, #16\n"
        "\tlsls\tr4, r7, #16\n"
        "\tasrs\tr0, r4, #16\n"
        "\tcmp\tr0, r6\n"
        "\tblt .Lai_lineup_25264_080025318\n"
        ".Lai_lineup_25264_08002533E:\n"
        "\tmovs\tr6, #0\n"
        "\tlsls\tr0, r7, #16\n"
        "\tasrs\tr1, r0, #16\n"
        "\tcmp\tr6, r1\n"
        "\tbge .Lai_lineup_25264_08002537C\n"
        "\tadds\tr5, r1, #0\n"
        ".Lai_lineup_25264_08002534A:\n"
        "\tbl 0x08008014\n"
        "\tbl 0x08005B5C\n"
        "\tadds\tr1, r5, #0\n"
        "\tbl 0x0802DE9C\n"
        "\tlsls\tr2, r6, #16\n"
        "\tasrs\tr2, r2, #16\n"
        "\tlsls\tr1, r2, #1\n"
        "\tmov\tr4, sp\n"
        "\tadds\tr3, r4, r1\n"
        "\tldrh\tr4, [r3, #0]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tmov\tr6, sp\n"
        "\tadds\tr1, r6, r0\n"
        "\tldrh\tr0, [r1, #0]\n"
        "\tstrh\tr0, [r3, #0]\n"
        "\tstrh\tr4, [r1, #0]\n"
        "\tadds\tr2, #1\n"
        "\tlsls\tr2, r2, #16\n"
        "\tlsrs\tr6, r2, #16\n"
        "\tasrs\tr2, r2, #16\n"
        "\tcmp\tr2, r5\n"
        "\tblt .Lai_lineup_25264_08002534A\n"
        ".Lai_lineup_25264_08002537C:\n"
        "\tmov\tr0, sp\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tadd\tr1, sp, #196\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr6, #1\n"
        "\tldr r1, .Lai_lineup_25264_080025420\n"
        "\tldr r2, .Lai_lineup_25264_080025424\n"
        "\tadds\tr0, r1, r2\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r0, r3]\n"
        "\tsubs\tr0, #1\n"
        "\tcmp\tr6, r0\n"
        "\tbge .Lai_lineup_25264_080025464\n"
        ".Lai_lineup_25264_080025396:\n"
        "\tmovs\tr4, #0\n"
        "\tmov\tsl, r4\n"
        "\tlsls\tr0, r6, #16\n"
        "\tasrs\tr1, r0, #16\n"
        "\tstr\tr0, [sp, #212]\n"
        "\tcmp\tsl, r1\n"
        "\tbge .Lai_lineup_25264_080025436\n"
        "\tmov\tr9, r1\n"
        "\tlsls\tr0, r1, #1\n"
        "\tadd\tr0, sp\n"
        "\tmov\tr8, r0\n"
        ".Lai_lineup_25264_0800253AC:\n"
        "\tmov\tr6, r8\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r6, r1]\n"
        "\tbl 0x08025518\n"
        "\tadds\tr6, r0, #0\n"
        "\tmov\tr2, r8\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r2, r3]\n"
        "\tbl 0x08025530\n"
        "\tlsls\tr6, r6, #24\n"
        "\tasrs\tr6, r6, #24\n"
        "\tlsls\tr0, r0, #24\n"
        "\tasrs\tr0, r0, #24\n"
        "\tadds\tr6, r6, r0\n"
        "\tmov\tr4, sl\n"
        "\tlsls\tr0, r4, #16\n"
        "\tasrs\tr7, r0, #16\n"
        "\tlsls\tr5, r7, #1\n"
        "\tadd\tr5, sp\n"
        "\tadds\tr5, #196\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r5, r1]\n"
        "\tbl 0x08025518\n"
        "\tadds\tr4, r0, #0\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r5, r2]\n"
        "\tbl 0x08025530\n"
        "\tlsls\tr4, r4, #24\n"
        "\tasrs\tr4, r4, #24\n"
        "\tlsls\tr0, r0, #24\n"
        "\tasrs\tr0, r0, #24\n"
        "\tadds\tr4, r4, r0\n"
        "\tcmp\tr6, r4\n"
        "\tbge .Lai_lineup_25264_080025428\n"
        "\tldr\tr1, [sp, #212]\n"
        "\tasrs\tr0, r1, #16\n"
        "\tcmp\tr0, r7\n"
        "\tble .Lai_lineup_25264_080025436\n"
        "\tadd\tr3, sp, #196\n"
        "\tadds\tr5, r7, #0\n"
        ".Lai_lineup_25264_080025404:\n"
        "\tasrs\tr1, r1, #16\n"
        "\tlsls\tr2, r1, #1\n"
        "\tadds\tr2, r3, r2\n"
        "\tsubs\tr1, #1\n"
        "\tlsls\tr0, r1, #1\n"
        "\tadds\tr0, r3, r0\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstrh\tr0, [r2, #0]\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr0, r1, #16\n"
        "\tcmp\tr0, r5\n"
        "\tbgt .Lai_lineup_25264_080025404\n"
        "\tb .Lai_lineup_25264_080025436\n"
        "\tmovs\tr0, r0\n"
        ".Lai_lineup_25264_080025420: .word 0x03001780\n"
        ".Lai_lineup_25264_080025424: .word 0x000010CA\n"
        ".Lai_lineup_25264_080025428:\n"
        "\tadds\tr0, r7, #1\n"
        "\tlsls\tr0, r0, #16\n"
        "\tlsrs\tr3, r0, #16\n"
        "\tmov\tsl, r3\n"
        "\tasrs\tr0, r0, #16\n"
        "\tcmp\tr0, r9\n"
        "\tblt .Lai_lineup_25264_0800253AC\n"
        ".Lai_lineup_25264_080025436:\n"
        "\tmov\tr4, sl\n"
        "\tlsls\tr2, r4, #16\n"
        "\tasrs\tr2, r2, #15\n"
        "\tadd\tr2, sp\n"
        "\tadds\tr2, #196\n"
        "\tldr\tr6, [sp, #212]\n"
        "\tasrs\tr1, r6, #16\n"
        "\tlsls\tr0, r1, #1\n"
        "\tadd\tr0, sp\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstrh\tr0, [r2, #0]\n"
        "\tadds\tr1, #1\n"
        "\tlsls\tr1, r1, #16\n"
        "\tlsrs\tr6, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tldr r0, .Lai_lineup_25264_0800254D8\n"
        "\tldr r2, .Lai_lineup_25264_0800254DC\n"
        "\tadds\tr0, r0, r2\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r0, r3]\n"
        "\tsubs\tr0, #1\n"
        "\tcmp\tr1, r0\n"
        "\tblt .Lai_lineup_25264_080025396\n"
        ".Lai_lineup_25264_080025464:\n"
        "\tmovs\tr6, #1\n"
        "\tldr r4, .Lai_lineup_25264_0800254D8\n"
        "\tldr r0, .Lai_lineup_25264_0800254DC\n"
        "\tadds\tr1, r4, r0\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tcmp\tr6, r0\n"
        "\tbge .Lai_lineup_25264_0800254C6\n"
        "\tadds\tr7, r4, #0\n"
        "\tmov\tr9, r1\n"
        ".Lai_lineup_25264_080025478:\n"
        "\tlsls\tr5, r6, #16\n"
        "\tasrs\tr5, r5, #16\n"
        "\tlsls\tr6, r5, #1\n"
        "\tldr r3, .Lai_lineup_25264_0800254E0\n"
        "\tadds\tr1, r7, r3\n"
        "\tadds\tr1, r6, r1\n"
        "\tsubs\tr4, r5, #1\n"
        "\tlsls\tr4, r4, #1\n"
        "\tadd\tr4, sp\n"
        "\tadds\tr4, #196\n"
        "\tldrh\tr0, [r4, #0]\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tbl 0x08008014\n"
        "\tbl 0x08005B5C\n"
        "\tmov\tr8, r0\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tbl 0x080254E8\n"
        "\tadds\tr1, r0, #0\n"
        "\tlsls\tr1, r1, #24\n"
        "\tasrs\tr1, r1, #24\n"
        "\tmov\tr0, r8\n"
        "\tbl 0x0802D97C\n"
        "\tldr r2, .Lai_lineup_25264_0800254E4\n"
        "\tadds\tr1, r7, r2\n"
        "\tadds\tr6, r6, r1\n"
        "\tstrh\tr0, [r6, #0]\n"
        "\tadds\tr5, #1\n"
        "\tlsls\tr5, r5, #16\n"
        "\tlsrs\tr6, r5, #16\n"
        "\tmov\tr3, r9\n"
        "\tldrh\tr3, [r3, #0]\n"
        "\tlsls\tr0, r3, #16\n"
        "\tcmp\tr5, r0\n"
        "\tblt .Lai_lineup_25264_080025478\n"
        ".Lai_lineup_25264_0800254C6:\n"
        "\tadd\tsp, #216\n"
        "\tpop\t{r3, r4, r5}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tmov\tsl, r5\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".Lai_lineup_25264_0800254D8: .word 0x03001780\n"
        ".Lai_lineup_25264_0800254DC: .word 0x000010CA\n"
        ".Lai_lineup_25264_0800254E0: .word 0x00001DD4\n"
        ".Lai_lineup_25264_0800254E4: .word 0x00001DE4\n"
        ".syntax divided\n"
    );
}
void _08025264(int a) __attribute__((alias("AiLineupBuilder_25264")));
void sub_08025264(int a) __attribute__((alias("AiLineupBuilder_25264")));

// ============================================================================
// code_23628.s 0x08023628 — race scene constructor: sound select, twin
// resource setups, record wiring, award-grid alloc triple, car-record copy
// loop (35×72 B via CpuSet), tail flags.
// ============================================================================
__attribute__((naked)) void RaceSceneCtor_23628(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, sl\n"
        "\tmov\tr6, r9\n"
        "\tmov\tr5, r8\n"
        "\tpush\t{r5, r6, r7}\n"
        "\tsub\tsp, #36\n"
        "\tadds\tr7, r0, #0\n"
        "\tmovs\tr0, #51\n"
        "\tbl 0x0802B214\n"
        "\tldr r0, .Lrace_ctor_23628_0800237F8\n"
        "\tmov\tr8, r0\n"
        "\tmovs\tr0, #4\n"
        "\tstr\tr0, [sp, #0]\n"
        "\tmovs\tr6, #1\n"
        "\tstr\tr6, [sp, #4]\n"
        "\tmovs\tr0, #0\n"
        "\tmov\tr1, r8\n"
        "\tmovs\tr2, #0\n"
        "\tmovs\tr3, #0\n"
        "\tbl 0x08007770\n"
        "\tldr r5, .Lrace_ctor_23628_0800237FC\n"
        "\tmovs\tr1, #0\n"
        "\tstr\tr1, [sp, #0]\n"
        "\tmovs\tr0, #3\n"
        "\tstr\tr0, [sp, #4]\n"
        "\tmovs\tr0, #1\n"
        "\tadds\tr1, r5, #0\n"
        "\tmovs\tr2, #3\n"
        "\tmovs\tr3, #0\n"
        "\tbl 0x08007770\n"
        "\tmovs\tr2, #6\n"
        "\tstr\tr2, [r7, #88]\n"
        "\tmovs\tr0, #21\n"
        "\tmov\tr9, r0\n"
        "\tstr\tr0, [r7, #100]\n"
        "\tstr\tr6, [r7, #112]\n"
        "\tmovs\tr1, #16\n"
        "\tadds\tr1, r1, r7\n"
        "\tmov\tsl, r1\n"
        "\tmov\tr0, sl\n"
        "\tbl 0x0800DAB8\n"
        "\tldr r4, .Lrace_ctor_23628_080023800\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #1\n"
        "\tmovs\tr2, #0\n"
        "\tmovs\tr3, #4\n"
        "\tbl 0x08007614\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #1\n"
        "\tmovs\tr2, #1\n"
        "\tmovs\tr3, #5\n"
        "\tbl 0x08007614\n"
        "\tmov\tr0, r8\n"
        "\tadds\tr1, r7, #0\n"
        "\tbl 0x0800798C\n"
        "\tadds\tr0, r7, #0\n"
        "\tbl 0x08007A58\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #8\n"
        "\tadds\tr0, r5, #0\n"
        "\tbl 0x0800798C\n"
        "\tadds\tr0, r5, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #6\n"
        "\tbl 0x080075E8\n"
        "\tadds\tr4, r7, #0\n"
        "\tadds\tr4, #196\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #5\n"
        "\tbl 0x08025BC8\n"
        "\tldr\tr0, [r7, #12]\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #200\n"
        "\tldr\tr1, [r1, #0]\n"
        "\tldr\tr2, [r4, #0]\n"
        "\tbl 0x08007ABC\n"
        "\tmovs\tr0, #8\n"
        "\tbl 0x0800572C\n"
        "\tadds\tr2, r7, #0\n"
        "\tadds\tr2, #208\n"
        "\tstr\tr0, [r2, #0]\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #212\n"
        "\tmovs\tr0, #22\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tldr\tr0, [r7, #12]\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tmovs\tr1, #22\n"
        "\tbl 0x08007ABC\n"
        "\tmovs\tr0, #8\n"
        "\tbl 0x0800572C\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #220\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #224\n"
        "\tmov\tr2, r9\n"
        "\tstr\tr2, [r0, #0]\n"
        "\tldr\tr0, [r7, #12]\n"
        "\tldr\tr2, [r1, #0]\n"
        "\tmovs\tr1, #21\n"
        "\tbl 0x08007ABC\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #128\n"
        "\tsubs\tr4, #28\n"
        "\tstr\tr4, [r0, #0]\n"
        "\tadds\tr0, #4\n"
        "\tmov\tr1, sl\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tmovs\tr2, #0\n"
        "\tstr\tr2, [r7, #16]\n"
        "\tstrh\tr6, [r7, #20]\n"
        "\tsubs\tr0, #96\n"
        "\tsubs\tr2, #32\n"
        "\tmovs\tr1, #0\n"
        "\tbl 0x0800D77C\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #28\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #160\n"
        "\tbl 0x0800D77C\n"
        "\tmovs\tr0, #6\n"
        "\tstrh\tr0, [r7, #24]\n"
        "\tmovs\tr0, #5\n"
        "\tstrh\tr0, [r7, #26]\n"
        "\tadds\tr2, r7, #0\n"
        "\tadds\tr2, #148\n"
        "\tldr r0, .Lrace_ctor_23628_080023804\n"
        "\tstr\tr0, [r2, #0]\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #152\n"
        "\tldr r0, .Lrace_ctor_23628_080023808\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tldr\tr6, [r2, #0]\n"
        "\tmovs\tr3, #0\n"
        "\tstr\tr4, [sp, #28]\n"
        "\tmovs\tr1, #142\n"
        "\tadds\tr1, r1, r7\n"
        "\tmov\tsl, r1\n"
        "\tadds\tr2, #8\n"
        "\tstr\tr2, [sp, #16]\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #158\n"
        "\tstr\tr0, [sp, #20]\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #160\n"
        "\tstr\tr1, [sp, #24]\n"
        "\tsubs\tr2, #10\n"
        "\tstr\tr2, [sp, #8]\n"
        "\tadds\tr7, #240\n"
        "\tstr\tr7, [sp, #12]\n"
        "\tldr r0, .Lrace_ctor_23628_08002380C\n"
        "\tmov\tr9, r0\n"
        ".Lrace_ctor_23628_08002377E:\n"
        "\tlsls\tr7, r3, #3\n"
        "\tadds\tr1, r3, #1\n"
        "\tmov\tr8, r1\n"
        "\tadds\tr0, r7, r3\n"
        "\tlsls\tr0, r0, #3\n"
        "\tmovs\tr4, #4\n"
        "\tmov\tr2, r9\n"
        "\tadds\tr5, r2, r0\n"
        ".Lrace_ctor_23628_08002378E:\n"
        "\tadds\tr0, r5, #0\n"
        "\tadds\tr1, r6, #0\n"
        "\tldr r2, .Lrace_ctor_23628_080023810\n"
        "\tstr\tr3, [sp, #32]\n"
        "\tbl 0x0802D974\n"
        "\tadds\tr6, #12\n"
        "\tadds\tr5, #12\n"
        "\tsubs\tr4, #1\n"
        "\tldr\tr3, [sp, #32]\n"
        "\tcmp\tr4, #0\n"
        "\tbge .Lrace_ctor_23628_08002378E\n"
        "\tadds\tr0, r7, r3\n"
        "\tlsls\tr0, r0, #3\n"
        "\tldr r1, .Lrace_ctor_23628_080023814\n"
        "\tadds\tr0, r0, r1\n"
        "\tadds\tr1, r6, #0\n"
        "\tldr r2, .Lrace_ctor_23628_080023810\n"
        "\tbl 0x0802D974\n"
        "\tadds\tr6, #12\n"
        "\tmov\tr3, r8\n"
        "\tcmp\tr3, #34\n"
        "\tble .Lrace_ctor_23628_08002377E\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr0, #2\n"
        "\tldr\tr2, [sp, #28]\n"
        "\tstr\tr0, [r2, #0]\n"
        "\tmov\tr0, sl\n"
        "\tstrh\tr1, [r0, #0]\n"
        "\tldr\tr2, [sp, #16]\n"
        "\tstrh\tr1, [r2, #0]\n"
        "\tldr\tr0, [sp, #20]\n"
        "\tstrh\tr1, [r0, #0]\n"
        "\tldr\tr2, [sp, #24]\n"
        "\tstrh\tr1, [r2, #0]\n"
        "\tldr\tr0, [sp, #8]\n"
        "\tstrh\tr1, [r0, #0]\n"
        "\tldr r0, .Lrace_ctor_23628_080023818\n"
        "\tbl 0x08002124\n"
        "\tmovs\tr0, #1\n"
        "\tldr\tr1, [sp, #12]\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tadd\tsp, #36\n"
        "\tpop\t{r3, r4, r5}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tmov\tsl, r5\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".Lrace_ctor_23628_0800237F8: .word 0x0839F5FC\n"
        ".Lrace_ctor_23628_0800237FC: .word 0x082B7410\n"
        ".Lrace_ctor_23628_080023800: .word 0x082A798C\n"
        ".Lrace_ctor_23628_080023804: .word 0x02024EC0\n"
        ".Lrace_ctor_23628_080023808: .word 0x02020000\n"
        ".Lrace_ctor_23628_08002380C: .word 0x03001D64\n"
        ".Lrace_ctor_23628_080023810: .word 0x04000003\n"
        ".Lrace_ctor_23628_080023814: .word 0x03001DA0\n"
        ".Lrace_ctor_23628_080023818: .word 0x0000138D\n"
        ".syntax divided\n"
    );
}
void _08023628(void *r) __attribute__((alias("RaceSceneCtor_23628")));
void sub_08023628(void *r) __attribute__((alias("RaceSceneCtor_23628")));

// ============================================================================
// ai_racefsm.s 0x0800AA40 — per-frame race/AI control FSM: car-index
// sampling, grid aggregates, event-ring reset, 8-way phase dispatch
// (countdown / GO / running / lap-check / results). Sub-dispatch notes:
// slot32==10 gate, place-change events 38/43/49, award packets.
// ============================================================================
// Shared lap/progress tail of race-FSM cases 5 and 7-sub1 (AD00..AD32).
// NOTE: the place byte is wa+0x10E5 (== 0x03002865); every ROM site reaches
// it as pool literal 0x000010E5 added to the 0x03001780 base (e.g. the
// 0x0800AB78 and 0x0800AD2C literals). Do NOT write wa+0x2865 here.
// The lap-tail and progress-check bodies are INLINED at every call site: the
// ROM has no `bl` for them between 0x0800AA40 and 0x0800AD34 (each former call
// site repeats the full body), so file-local static helpers left two
// UNRESOLVED_RELOCATION bls. Kept as macros for readability at the 4 sites.
#define RACE_LAP_TAIL()                                                       \
    do {                                                                      \
        _080023FF8(21, 0);                                                    \
        _0800B9F0();                                                          \
        _080023FF8(35, 0);                                                    \
    } while (0)
#define RACE_PROGRESS_CHECK(c, wap)                                           \
    do {                                                                      \
        int r4v = (int)(s16)_08002581C(c);                                    \
        int ruv = (int)(u8)_080025E1C(r4v);                                   \
        int rbv = (int)(s8)*(wap + 0x10E5);                                   \
        if (ruv < rbv) {                                                      \
            _080025DBC(r4v, rbv);                                             \
            _0800279C(1);                                                     \
        }                                                                     \
        RACE_LAP_TAIL();                                                      \
    } while (0)
__attribute__((naked)) void RaceFSM_AA40(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush {r4, r5, r6, r7, lr}\n"
        "\tmov r7, sl\n"
        "\tmov r6, r9\n"
        "\tmov r5, r8\n"
        "\tpush {r5, r6, r7}\n"
        "\tsub sp, #12\n"
        "\tldr r4, _0800AADC_lit\n"
        "\tldr r0, _0800AAE0_lit\n"
        "\tadds r6, r4, r0\n"
        "\tmovs r1, #0\n"
        "\tldrsh r0, [r6, r1]\n"
        "\tlsls r2, r0, #3\n"
        "\tldr r3, _0800AAE4_lit\n"
        "\tadds r5, r4, r3\n"
        "\tmovs r7, #0\n"
        "\tldrsh r1, [r5, r7]\n"
        "\tlsls r3, r1, #1\n"
        "\tadds r3, r3, r2\n"
        "\tldr r7, _0800AAE8_lit\n"
        "\tadds r2, r4, r7\n"
        "\tadds r2, r2, r3\n"
        "\tmovs r7, #0\n"
        "\tldrsh r2, [r2, r7]\n"
        "\tstr r2, [sp, #8]\n"
        "\tldr r2, _0800AAEC_lit\n"
        "\tadds r2, r2, r4\n"
        "\tmov r8, r2\n"
        "\tadd r8, r3\n"
        "\tmov r7, r8\n"
        "\tmovs r2, #0\n"
        "\tldrsh r7, [r7, r2]\n"
        "\tmov r8, r7\n"
        "\tldr r7, _0800AAF0_lit\n"
        "\tadds r7, r7, r4\n"
        "\tmov r9, r7\n"
        "\tadd r9, r3\n"
        "\tmov r2, r9\n"
        "\tmovs r3, #0\n"
        "\tldrsh r2, [r2, r3]\n"
        "\tmov r9, r2\n"
        "\tbl sub_080026F8\n"
        "\tstr r0, [sp, #0]\n"
        "\tmovs r7, #0\n"
        "\tldrsh r0, [r6, r7]\n"
        "\tmovs r2, #0\n"
        "\tldrsh r1, [r5, r2]\n"
        "\tbl sub_08002730\n"
        "\tmov sl, r0\n"
        "\tmovs r3, #0\n"
        "\tldrsh r0, [r6, r3]\n"
        "\tmovs r7, #0\n"
        "\tldrsh r1, [r5, r7]\n"
        "\tbl sub_0800A9E0\n"
        "\tstr r0, [sp, #4]\n"
        "\tmovs r1, #0\n"
        "\tldrsh r0, [r6, r1]\n"
        "\tmovs r2, #0\n"
        "\tldrsh r1, [r5, r2]\n"
        "\tbl sub_0800A9A0\n"
        "\tadds r5, r0, #0\n"
        "\tbl _08023FE4\n"
        "\tldr r3, _0800AAF4_lit\n"
        "\tadds r4, r4, r3\n"
        "\tmovs r7, #0\n"
        "\tldrsh r0, [r4, r7]\n"
        "\tcmp r0, #7\n"
        "\tbls .Lrace_aa40_0800AAD2\n"
        "\tb .Lrace_aa40_0800AD70\n"
        ".Lrace_aa40_0800AAD2:\n"
        "\tlsls r0, r0, #2\n"
        "\tldr r1, _0800AAF8_lit\n"
        "\tadds r0, r0, r1\n"
        "\tldr r0, [r0, #0]\n"
        "\tmov pc, r0\n"
        "\t.align 2, 0\n"
        "_0800AADC_lit:\n"
        "\t.word 0x03001780\n"
        "_0800AAE0_lit:\n"
        "\t.word 0x00000FF2\n"
        "_0800AAE4_lit:\n"
        "\t.word 0x00000FF8\n"
        "_0800AAE8_lit:\n"
        "\t.word 0x0000100A\n"
        "_0800AAEC_lit:\n"
        "\t.word 0x0000102A\n"
        "_0800AAF0_lit:\n"
        "\t.word 0x0000101A\n"
        "_0800AAF4_lit:\n"
        "\t.word 0x00000FBC\n"
        "_0800AAF8_lit:\n"
        "\t.word .Lrace_aa40_0800AAFC\n"
        ".Lrace_aa40_0800AAFC:\n"
        "\t.word .Lrace_aa40_0800AB1C\n"
        "\t.word .Lrace_aa40_0800AC2E\n"
        "\t.word .Lrace_aa40_0800AC90\n"
        "\t.word .Lrace_aa40_0800ACC0\n"
        "\t.word .Lrace_aa40_0800AD70\n"
        "\t.word .Lrace_aa40_0800AC68\n"
        "\t.word .Lrace_aa40_0800AD70\n"
        "\t.word .Lrace_aa40_0800ACCA\n"
        ".Lrace_aa40_0800AB1C:\n"
        "\tmovs r0, #21\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tldr r1, _0800AB74_lit\n"
        "\tldr r2, _0800AB78_lit\n"
        "\tadds r0, r1, r2\n"
        "\tldrb r0, [r0, #0]\n"
        "\tsubs r0, #1\n"
        "\tlsls r0, r0, #24\n"
        "\tlsrs r0, r0, #24\n"
        "\tcmp r0, #2\n"
        "\tbls .Lrace_aa40_0800AB38\n"
        "\tb .Lrace_aa40_0800AD32\n"
        ".Lrace_aa40_0800AB38:\n"
        "\tldr r3, [sp, #8]\n"
        "\tcmp r3, #10\n"
        "\tbne .Lrace_aa40_0800AB50\n"
        "\tldr r7, _0800AB7C_lit\n"
        "\tadds r0, r1, r7\n"
        "\tldrh r0, [r0, #0]\n"
        "\tcmp r0, #3\n"
        "\tbne .Lrace_aa40_0800AB50\n"
        "\tldr r0, _0800AB80_lit\n"
        "\tadds r1, r1, r0\n"
        "\tmovs r0, #1\n"
        "\tstrh r0, [r1, #0]\n"
        ".Lrace_aa40_0800AB50:\n"
        "\tbl _0800B4A8\n"
        "\tldr r1, [sp, #8]\n"
        "\tcmp r1, #2\n"
        "\tbne .Lrace_aa40_0800AB60\n"
        "\tmov r2, r8\n"
        "\tcmp r2, #0\n"
        "\tbeq .Lrace_aa40_0800AB6A\n"
        ".Lrace_aa40_0800AB60:\n"
        "\tcmp r5, sl\n"
        "\tble .Lrace_aa40_0800AB84\n"
        "\tmov r3, r8\n"
        "\tcmp r3, #1\n"
        "\tbne .Lrace_aa40_0800AB84\n"
        ".Lrace_aa40_0800AB6A:\n"
        "\tmovs r0, #43\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tb .Lrace_aa40_0800AC10\n"
        "\t.align 2, 0\n"
        "_0800AB74_lit:\n"
        "\t.word 0x03001780\n"
        "_0800AB78_lit:\n"
        "\t.word 0x000010E5\n"
        "_0800AB7C_lit:\n"
        "\t.word 0x00000FF8\n"
        "_0800AB80_lit:\n"
        "\t.word 0x00001074\n"
        ".Lrace_aa40_0800AB84:\n"
        "\tldr r7, [sp, #8]\n"
        "\tcmp r7, #10\n"
        "\tbne .Lrace_aa40_0800AB90\n"
        "\tmov r0, r9\n"
        "\tcmp r0, #0\n"
        "\tbeq .Lrace_aa40_0800ABA0\n"
        ".Lrace_aa40_0800AB90:\n"
        "\tldr r0, _0800ABC0_lit\n"
        "\tldr r1, [sp, #4]\n"
        "\tldr r2, [sp, #0]\n"
        "\tcmp r1, r2\n"
        "\tble .Lrace_aa40_0800ABE8\n"
        "\tmov r3, r9\n"
        "\tcmp r3, #1\n"
        "\tbne .Lrace_aa40_0800ABE8\n"
        ".Lrace_aa40_0800ABA0:\n"
        "\tldr r0, _0800ABC0_lit\n"
        "\tldr r7, _0800ABC4_lit\n"
        "\tadds r2, r0, r7\n"
        "\tmovs r3, #0\n"
        "\tldrsh r1, [r2, r3]\n"
        "\tcmp r1, #0\n"
        "\tbne .Lrace_aa40_0800ABE8\n"
        "\tsubs r7, #124\n"
        "\tadds r0, r0, r7\n"
        "\tldrh r0, [r0, #0]\n"
        "\tcmp r0, #3\n"
        "\tbne .Lrace_aa40_0800ABC8\n"
        "\tmovs r0, #1\n"
        "\tstrh r0, [r2, #0]\n"
        "\tb .Lrace_aa40_0800ABF2\n"
        "\t.align 2, 0\n"
        "_0800ABC0_lit:\n"
        "\t.word 0x03001780\n"
        "_0800ABC4_lit:\n"
        "\t.word 0x00001074\n"
        ".Lrace_aa40_0800ABC8:\n"
        "\tmovs r0, #38\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tbl sub_08004CC4\n"
        "\tmovs r0, #13\n"
        "\tbl sub_08004CD4\n"
        "\tmovs r0, #14\n"
        "\tbl sub_08004CD4\n"
        "\tmovs r0, #31\n"
        "\tbl sub_08004CD4\n"
        "\tb .Lrace_aa40_0800AD70\n"
        ".Lrace_aa40_0800ABE8:\n"
        "\tldr r1, _0800AC0C_lit\n"
        "\tadds r0, r0, r1\n"
        "\tldrh r0, [r0, #0]\n"
        "\tcmp r0, #1\n"
        "\tbne .Lrace_aa40_0800AC10\n"
        ".Lrace_aa40_0800ABF2:\n"
        "\tmovs r0, #38\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tbl sub_08004CC4\n"
        "\tmovs r0, #49\n"
        "\tbl sub_08004CD4\n"
        "\tbl sub_0800B0BC\n"
        "\tb .Lrace_aa40_0800AD70\n"
        "\t.align 2, 0\n"
        "_0800AC0C_lit:\n"
        "\t.word 0x00001074\n"
        ".Lrace_aa40_0800AC10:\n"
        "\tbl sub_08004CC4\n"
        "\tmovs r0, #13\n"
        "\tbl sub_08004CD4\n"
        "\tmovs r0, #14\n"
        "\tbl sub_08004CD4\n"
        "\tmovs r0, #31\n"
        "\tbl sub_08004CD4\n"
        "\tmovs r0, #15\n"
        "\tbl sub_08004CD4\n"
        "\tb .Lrace_aa40_0800AD70\n"
        ".Lrace_aa40_0800AC2E:\n"
        "\tmovs r0, #21\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tldr r0, _0800AC60_lit\n"
        "\tldr r2, _0800AC64_lit\n"
        "\tadds r0, r0, r2\n"
        "\tldrh r0, [r0, #0]\n"
        "\tcmp r0, #1\n"
        "\tbne .Lrace_aa40_0800AC50\n"
        "\tmovs r0, #1\n"
        "\tbl sub_0800279C\n"
        "\tmovs r0, #25\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        ".Lrace_aa40_0800AC50:\n"
        "\tmovs r0, #35\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tbl sub_08024144\n"
        "\tb .Lrace_aa40_0800AD70\n"
        "\t.align 2, 0\n"
        "_0800AC60_lit:\n"
        "\t.word 0x03001780\n"
        "_0800AC64_lit:\n"
        "\t.word 0x00000FC8\n"
        ".Lrace_aa40_0800AC68:\n"
        "\tldr r1, _0800AC84_lit\n"
        "\tldr r3, _0800AC88_lit\n"
        "\tadds r5, r1, r3\n"
        "\tldrb r0, [r5, #0]\n"
        "\tsubs r0, #1\n"
        "\tlsls r0, r0, #24\n"
        "\tlsrs r0, r0, #24\n"
        "\tcmp r0, #2\n"
        "\tbhi .Lrace_aa40_0800AD26\n"
        "\tldr r7, _0800AC8C_lit\n"
        "\tadds r0, r1, r7\n"
        "\tmovs r1, #0\n"
        "\tldrsh r0, [r0, r1]\n"
        "\tb .Lrace_aa40_0800AD00\n"
        "\t.align 2, 0\n"
        "_0800AC84_lit:\n"
        "\t.word 0x03001780\n"
        "_0800AC88_lit:\n"
        "\t.word 0x000010E5\n"
        "_0800AC8C_lit:\n"
        "\t.word 0x00000576\n"
        ".Lrace_aa40_0800AC90:\n"
        "\tmovs r0, #21\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tbl sub_0800B89C\n"
        "\tldr r0, _0800ACB8_lit\n"
        "\tldr r2, _0800ACBC_lit\n"
        "\tadds r0, r0, r2\n"
        "\tldrh r0, [r0, #0]\n"
        "\tcmp r0, #1\n"
        "\tbne .Lrace_aa40_0800AD32\n"
        "\tmovs r0, #1\n"
        "\tbl sub_0800279C\n"
        "\tmovs r0, #25\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tb .Lrace_aa40_0800AD32\n"
        "\t.align 2, 0\n"
        "_0800ACB8_lit:\n"
        "\t.word 0x03001780\n"
        "_0800ACBC_lit:\n"
        "\t.word 0x00000FC8\n"
        ".Lrace_aa40_0800ACC0:\n"
        "\tmovs r0, #21\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tb .Lrace_aa40_0800AD32\n"
        ".Lrace_aa40_0800ACCA:\n"
        "\tldr r4, _0800ACE0_lit\n"
        "\tldr r3, _0800ACE4_lit\n"
        "\tadds r0, r4, r3\n"
        "\tmovs r7, #0\n"
        "\tldrsh r0, [r0, r7]\n"
        "\tcmp r0, #1\n"
        "\tbeq .Lrace_aa40_0800ACE8\n"
        "\tcmp r0, #2\n"
        "\tbeq .Lrace_aa40_0800AD44\n"
        "\tb .Lrace_aa40_0800AD70\n"
        "\t.align 2, 0\n"
        "_0800ACE0_lit:\n"
        "\t.word 0x03001780\n"
        "_0800ACE4_lit:\n"
        "\t.word 0x00001078\n"
        ".Lrace_aa40_0800ACE8:\n"
        "\tldr r0, _0800AD3C_lit\n"
        "\tadds r5, r4, r0\n"
        "\tldrb r0, [r5, #0]\n"
        "\tsubs r0, #1\n"
        "\tlsls r0, r0, #24\n"
        "\tlsrs r0, r0, #24\n"
        "\tcmp r0, #2\n"
        "\tbhi .Lrace_aa40_0800AD26\n"
        "\tldr r1, _0800AD40_lit\n"
        "\tadds r0, r4, r1\n"
        "\tmovs r2, #0\n"
        "\tldrsh r0, [r0, r2]\n"
        ".Lrace_aa40_0800AD00:\n"
        "\tbl sub_0802581C\n"
        "\tlsls r0, r0, #16\n"
        "\tasrs r4, r0, #16\n"
        "\tadds r0, r4, #0\n"
        "\tbl sub_08025E1C\n"
        "\tlsls r0, r0, #24\n"
        "\tlsrs r0, r0, #24\n"
        "\tmovs r1, #0\n"
        "\tldrsb r1, [r5, r1]\n"
        "\tcmp r0, r1\n"
        "\tbge .Lrace_aa40_0800AD26\n"
        "\tadds r0, r4, #0\n"
        "\tbl sub_08025DBC\n"
        "\tmovs r0, #1\n"
        "\tbl sub_0800279C\n"
        ".Lrace_aa40_0800AD26:\n"
        "\tmovs r0, #21\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tbl sub_0800B9F0\n"
        ".Lrace_aa40_0800AD32:\n"
        "\tmovs r0, #35\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tb .Lrace_aa40_0800AD70\n"
        "\t.align 2, 0\n"
        "_0800AD3C_lit:\n"
        "\t.word 0x000010E5\n"
        "_0800AD40_lit:\n"
        "\t.word 0x00000576\n"
        ".Lrace_aa40_0800AD44:\n"
        "\tmovs r0, #21\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tldr r3, _0800AD80_lit\n"
        "\tadds r0, r4, r3\n"
        "\tldrh r0, [r0, #0]\n"
        "\tcmp r0, #1\n"
        "\tbne .Lrace_aa40_0800AD64\n"
        "\tmovs r0, #1\n"
        "\tbl sub_0800279C\n"
        "\tmovs r0, #25\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        ".Lrace_aa40_0800AD64:\n"
        "\tmovs r0, #35\n"
        "\tmovs r1, #0\n"
        "\tbl _08023FF8\n"
        "\tbl sub_08024144\n"
        ".Lrace_aa40_0800AD70:\n"
        "\tadd sp, #12\n"
        "\tpop {r3, r4, r5}\n"
        "\tmov r8, r3\n"
        "\tmov r9, r4\n"
        "\tmov sl, r5\n"
        "\tpop {r4, r5, r6, r7}\n"
        "\tpop {r0}\n"
        "\tbx r0\n"
        "\t.align 2, 0\n"
        "_0800AD80_lit:\n"
        "\t.word 0x00000FC8\n"
        ".syntax divided\n"
    );
}
void _0800AA40(void) __attribute__((alias("RaceFSM_AA40")));
void sub_0800AA40(void) __attribute__((alias("RaceFSM_AA40")));
void Ai_RaceFsm(void) __attribute__((alias("RaceFSM_AA40"))); /* trampoline elimination: friendly-name spelling used by race_progress.c */

// Trampoline decls for the six course-record emitters used by 8CA0. The
// existing C bodies are hollow/partial (writes voided, void returns), so the
// phase machine calls the ROM bodies directly. All take <=2 reg args with
// r3 dead on entry (verified per body) — trampoline-safe (only r3 dies).
extern void *RomEmit8AAC(void *r7);          // 0x08008AAC
extern void *RomEmit8BB8(void *r7);          // 0x08008BB8
extern void *RomEmit8768(void *r7, int w);   // 0x08008768
extern void *RomEmit88B0(void *r7, int w);   // 0x080088B0
extern void *RomEmit861C(void *r7, int w);   // 0x0800861C
extern void *RomEmit89FC(void *r7);          // 0x080089FC
extern u32 RomLeaf7658(void *a);             // 0x08007658

static inline u8 *phase_case148(u8 *r7);
static inline u8 *phase_case3(u8 *r7);
static inline u8 *phase_case5(u8 *r7);

// Case 5 short burst + random/award blocks, flowing into the shared
// [32]/[28]/[80]/[24] tail (941E) which no other case reaches.
static inline u8 *phase_case5(u8 *r7) {
    u8 *cc = *(u8 **)(uintptr_t)0x030003E0u;
    *(u16 *)(r7 + 0) = 2u;
    *(u16 *)(r7 + 2) = 0x40CDu;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                             | (u32)(u16)(*(u16 *)(cc + 48)));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 220u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52)) + 24u))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 227u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52))
                             + (u16)((u16)(*(u16 *)(cc + 10)) * 2u)))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    u16 lane = *(u16 *)(cc + 38);
    CourseEmit7B18(cc + 88, 14, 184, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 208, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 224, 4, lane, 1u, 0u, 0u);
    int c26 = (int)(s16)*(u16 *)(cc + 26);
    if (c26 > 0) {
        int rr = _08002BE8(0);
        u16 nc26 = (u16)(*(u16 *)(cc + 26) - 1u);
        *(u16 *)(cc + 26) = nc26;
        int r5s = (s32)rr;
        u16 trow = *(u16 *)(uintptr_t)(0x080CB154u + (u32)(u16)(nc26 & 15u) * 2u);
        _08002C48(r5s, trow);
        int c18 = (int)(s16)*(u16 *)(cc + 18);
        if (c18 > 9) {
            *(u16 *)(r7 + 0) = 0x386u;
            *(u16 *)(r7 + 2) = (u16)(((u32)(u16)r5s << 9) | 0x4044u);
            int dq = _08002DE04(c18, 10);
            *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 40))
                                           + (u32)(dq * 4))
                                     | ((u32)(u16)(*(u16 *)(cc + 38)) << 12));
            r7 += 8;
        }
        *(u16 *)(r7 + 0) = 0x386u;
        *(u16 *)(r7 + 2) = (u16)(((u32)(u16)r5s << 9) | 0x4050u);
        int c18b = (int)(s16)*(u16 *)(cc + 18);
        int dm = DivRemS_02DE9C(c18b, 10);
        *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 40))
                                       + (u32)(dm * 4))
                                 | ((u32)(u16)(*(u16 *)(cc + 38)) << 12));
        r7 += 8;
    } else {
        int c18 = (int)(s16)*(u16 *)(cc + 18);
        if (c18 > 9) {
            *(u16 *)(r7 + 0) = 142u;
            *(u16 *)(r7 + 2) = 0x404Cu;
            int dq = _08002DE04(c18, 10);
            *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 40))
                                           + (u32)(dq * 4))
                                     | ((u32)(u16)(*(u16 *)(cc + 38)) << 12));
            r7 += 8;
        }
        *(u16 *)(r7 + 0) = 142u;
        *(u16 *)(r7 + 2) = 0x4058u;
        int c18b = (int)(s16)*(u16 *)(cc + 18);
        int dr = DivRemS_02DE9C(c18b, 10);
        *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 40))
                                       + (u32)(dr * 4))
                                 | ((u32)(u16)(*(u16 *)(cc + 38)) << 12));
        r7 += 8;
    }
    // [crec+32] conditional ingen record
    {
        u16 c32 = *(u16 *)(cc + 32);
        if (c32 != 0u && c32 <= 16u) {
            *(u16 *)(r7 + 0) = 1152u;
            *(u16 *)(r7 + 2) = 0x40A0u;
            *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                                          | (u32)(u16)(*(u16 *)(cc + 42)));
            r7 += 8;
            *(u16 *)(uintptr_t)0x04000050u = 1856u;
            *(u16 *)(uintptr_t)0x04000052u =
                (u16)(((16u - (u32)c32) << 8) | (u32)c32);
        }
    }
    // [crec+28] countdown block
    {
        u16 c28u = *(u16 *)(cc + 28);
        if ((int)(s16)c28u > 0) {
            *(u16 *)(cc + 28) = (u16)(c28u - 1u);
            u8 nb79 = (u8)(*(cc + 79) + 1u);
            *(cc + 79) = nb79;
            if (nb79 > 15u) *(cc + 79) = 1u;
            _08007614((void *)(uintptr_t)0x0828A35Cu, 1, (int)*(cc + 79),
                      (int)*(u16 *)(cc + 58));
            u8 *entry = (u8 *)(uintptr_t)(0x080CB074u
                        + (u32)(s32)((s32)(s16)(*(u16 *)(cc + 30)) * 8));
            int e2 = (int)(s16)*(u16 *)(entry + 2);
            int c28 = (int)(s16)*(u16 *)(cc + 28);
            int d = 60 - c28;
            int dd = (d + (int)((u32)d >> 31)) >> 1;
            CourseIter7BFC_Static(cc + 96, (int)*(u16 *)(cc + 62),
                                  (int)(s16)*(u16 *)(entry + 0), e2 - dd,
                                  (u32)*(u16 *)(cc + 58), 0u, 0u, 0u);
        }
    }
    // [cc+80/84] stamp + [24] tail switch
    *(u32 *)(cc + 80) = 168u;
    *(u32 *)(cc + 84) = 80u;
    {
        int w24 = (int)(s16)(*(u16 *)(cc + 24));
        if (w24 == 2) {
            u8 nb = (u8)(*(cc + 79) + 1u);
            *(cc + 79) = nb;
            if (nb > 15u) *(cc + 79) = 1u;
            _08007614((void *)(uintptr_t)0x0828A35Cu, 1, (int)*(cc + 79),
                      (int)*(u16 *)(cc + 58));
            r7 = (u8 *)RomEmit89FC(r7);
        } else if (w24 == 1) {
            u16 c22 = *(u16 *)(cc + 22);
            *(u16 *)(cc + 22) = (u16)(c22 - 1u);
            int idx = (int)(s16)*(u16 *)(cc + 22);
            int tv = (int)(s16)*(u16 *)(uintptr_t)(0x080CB0CCu + (u32)(u16)idx * 2u);
            *(u32 *)(cc + 80) = (u32)(tv + 168);
            *(u32 *)(cc + 84) = 80u;
            if ((s32)((u32)(u16)(c22 - 1u) << 16) <= 0)
                *(u16 *)(cc + 24) = 2u;
            r7 = (u8 *)RomEmit8AAC(r7);
        } else if (w24 == 3) {
            u16 c22 = *(u16 *)(cc + 22);
            *(u16 *)(cc + 22) = (u16)(c22 - 1u);
            int idx = (int)(s16)*(u16 *)(cc + 22);
            int tv = (int)(s16)*(u16 *)(uintptr_t)(0x080CB0EEu
                       + (u32)(u16)(16 - idx) * 2u);
            *(u32 *)(cc + 80) = (u32)(tv + 168);
            *(u32 *)(cc + 84) = 80u;
            if ((s32)((u32)(u16)(c22 - 1u) << 16) <= 0)
                *(u16 *)(cc + 24) = 0u;
            r7 = (u8 *)RomEmit8BB8(r7);
        }
    }
    return r7;
}

// Case 1/4/8 record burst + counter gate (8ED8). Returns tail cursor.
// NOTE: all three phase_case helpers plus CourseIter7BFC_Static are
// `static inline` with definitions preceding CoursePhaseMachine_08CA0:
// agbcc only inlines helpers defined before use, and a non-inlined helper
// emits a `bl` with no ROM VMA (UNRESOLVED_RELOCATION). Measured 2026-10-13:
// `inline` alone moved _08008CA0 612 -> 1488 B, moving 148/3 ahead as well
// cleared the class (PARTIAL 140/2384, 2316-B candidate, all calls resolve).
static inline u8 *phase_case148(u8 *r7) {
    u8 *cc = *(u8 **)(uintptr_t)0x030003E0u;
    r7 = (u8 *)RomEmit8768(r7, (int)*(u32 *)(cc + 64));
    r7 = (u8 *)RomEmit88B0(r7, (int)*(u32 *)(cc + 68));
    *(u16 *)(r7 + 0) = 2u;
    *(u16 *)(r7 + 2) = 0x40A7u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                             | (u32)(u16)(*(u16 *)(cc + 50)));
    r7 += 8;
    *(u16 *)(r7 + 0) = 2u;
    *(u16 *)(r7 + 2) = 0x40CDu;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                             | (u32)(u16)(*(u16 *)(cc + 48)));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 220u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52)) + 24u))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 227u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52))
                             + (u16)((u16)(*(u16 *)(cc + 10)) * 2u)))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    u16 lane = *(u16 *)(cc + 38);
    CourseEmit7B18(cc + 88, 12, 0, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 64, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 80, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 96, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 104, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 13, 120, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 168, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 14, 184, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 208, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 224, 4, lane, 1u, 0u, 0u);
    u16 c4 = *(u16 *)(cc + 4);
    if ((int)(s16)c4 <= 0) return r7;
    u16 c4n = (u16)(c4 - 1u);
    *(u16 *)(cc + 4) = c4n;
    if ((s32)(s16)c4n > 63
        && _08002D97C(_08002D978((int)(s16)c4n, 8), 2) == 0)
        return r7;
    return (u8 *)RomEmit861C(r7, (int)*(u32 *)(cc + 72));
}

// Case 3 variant (90A4): conditional head emits, 8-burst, same counter gate.
static inline u8 *phase_case3(u8 *r7) {
    u8 *wa = (u8 *)(uintptr_t)0x03001780u;
    u8 *cc = *(u8 **)(uintptr_t)0x030003E0u;
    if (*(cc + 78) == 0u
        || _08002D97C(_08002D978((int)*(u32 *)(wa + 0x10D8), 3), 2) != 0)
        r7 = (u8 *)RomEmit8768(r7, (int)*(u32 *)(cc + 64));
    u16 c34 = *(u16 *)(cc + 34);
    if (c34 != 0u) {
        u16 n34 = (u16)(c34 - 1u);
        *(u16 *)(cc + 34) = n34;
        if (_08002D97C(_08002D978((int)(s16)n34, 3), 2) != 0)
            r7 = (u8 *)RomEmit88B0(r7, (int)*(u32 *)(cc + 72));
    } else {
        r7 = (u8 *)RomEmit88B0(r7, (int)*(u32 *)(cc + 68));
    }
    *(u16 *)(r7 + 0) = 2u;
    *(u16 *)(r7 + 2) = 0x40CDu;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)(*(u16 *)(cc + 38)) << 12)
                             | (u32)(u16)(*(u16 *)(cc + 48)));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 220u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52)) + 24u))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    *(u16 *)(r7 + 0) = 0x8000u;
    *(u16 *)(r7 + 2) = 227u;
    *(u16 *)(r7 + 4) = (u16)(((u32)(u16)((u16)(*(u16 *)(cc + 52))
                             + (u16)((u16)(*(u16 *)(cc + 10)) * 2u)))
                             | ((u32)(u16)(*(u16 *)(cc + 54)) << 12));
    r7 += 8;
    u16 lane = *(u16 *)(cc + 38);
    CourseEmit7B18(cc + 88, 12, 0, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 64, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 80, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 96, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 104, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 14, 184, 0, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 15, 208, 4, lane, 1u, 0u, 0u);
    CourseEmit7B18(cc + 88, 16, 224, 4, lane, 1u, 0u, 0u);
    u16 c4 = *(u16 *)(cc + 4);
    if ((int)(s16)c4 <= 0) return r7;
    u16 c4n = (u16)(c4 - 1u);
    *(u16 *)(cc + 4) = c4n;
    if ((s32)(s16)c4n > 63
        && _08002D97C(_08002D978((int)(s16)c4n, 8), 2) == 0)
        return r7;
    return (u8 *)RomEmit861C(r7, (int)*(u32 *)(cc + 72));
}


__attribute__((naked)) void CoursePhaseMachine_08CA0(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "  push {r4, r5, r6, r7, lr}\n"
        "  mov r7, sl\n"
        "  mov r6, r9\n"
        "  mov r5, r8\n"
        "  push {r5, r6, r7}\n"
        "  sub sp, #80\n"
        "  bl 0x08003130\n"
        "  adds r7, r0, #0\n"
        "  ldr r4, .Lcourse_8ca0_08008E8C\n"
        "  ldr r0, [r4, #0]\n"
        "  movs r2, #2\n"
        "  ldrsh r1, [r0, r2]\n"
        "  movs r0, #100\n"
        "  muls r0, r1\n"
        "  movs r1, #160\n"
        "  bl 0x0802DE04\n"
        "  add r1, sp, #20\n"
        "  bl 0x08003D18\n"
        "  adds r2, r0, #0\n"
        "  add r3, sp, #36\n"
        "  mov sl, r3\n"
        "  mov r5, sp\n"
        "  adds r5, #44\n"
        "  str r5, [sp, #68]\n"
        "  add r6, sp, #52\n"
        "  mov r8, r6\n"
        "  add r0, sp, #60\n"
        "  mov r9, r0\n"
        "  cmp r2, #0\n"
        "  ble .Lcourse_8ca0_08008D24\n"
        "  lsls r1, r2, #3\n"
        "  mov ip, r4\n"
        "  mov r0, sp\n"
        "  adds r0, #35\n"
        "  subs r4, r0, r2\n"
        "  subs r1, r2, r1\n"
        "  adds r3, r1, #0\n"
        "  adds r3, #175\n"
        "  ldr r1, .Lcourse_8ca0_08008E90\n"
        "  adds r5, r1, #0\n"
        ".Lcourse_8ca0_08008CF6:\n"
        "  strh r5, [r7, #0]\n"
        "  ldr r6, .Lcourse_8ca0_08008E94\n"
        "  adds r1, r6, #0\n"
        "  adds r0, r3, #0\n"
        "  ands r0, r1\n"
        "  strh r0, [r7, #2]\n"
        "  mov r0, ip\n"
        "  ldr r1, [r0, #0]\n"
        "  ldrb r0, [r4, #0]\n"
        "  subs r0, #48\n"
        "  lsls r0, r0, #1\n"
        "  ldrh r6, [r1, #52]\n"
        "  adds r0, r6, r0\n"
        "  ldrh r1, [r1, #54]\n"
        "  lsls r1, r1, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  adds r4, #1\n"
        "  adds r3, #7\n"
        "  subs r2, #1\n"
        "  cmp r2, #0\n"
        "  bne .Lcourse_8ca0_08008CF6\n"
        ".Lcourse_8ca0_08008D24:\n"
        "  ldr r1, .Lcourse_8ca0_08008E90\n"
        "  adds r0, r1, #0\n"
        "  strh r0, [r7, #0]\n"
        "  movs r0, #202\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r3, .Lcourse_8ca0_08008E8C\n"
        "  ldr r2, [r3, #0]\n"
        "  ldrh r4, [r2, #6]\n"
        "  lsls r0, r4, #1\n"
        "  ldrh r5, [r2, #52]\n"
        "  adds r0, r5, r0\n"
        "  ldrh r6, [r2, #54]\n"
        "  lsls r1, r6, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  ldr r0, .Lcourse_8ca0_08008E98\n"
        "  ldr r1, [r0, #4]\n"
        "  ldr r0, [r0, #0]\n"
        "  str r0, [sp, #36]\n"
        "  str r1, [sp, #40]\n"
        "  ldr r0, .Lcourse_8ca0_08008E9C\n"
        "  ldr r1, [r0, #4]\n"
        "  ldr r0, [r0, #0]\n"
        "  str r0, [sp, #44]\n"
        "  str r1, [sp, #48]\n"
        "  ldr r0, .Lcourse_8ca0_08008EA0\n"
        "  ldr r1, [r0, #4]\n"
        "  ldr r0, [r0, #0]\n"
        "  str r0, [sp, #52]\n"
        "  str r1, [sp, #56]\n"
        "  ldr r0, .Lcourse_8ca0_08008EA4\n"
        "  ldr r1, [r0, #4]\n"
        "  ldr r0, [r0, #0]\n"
        "  str r0, [sp, #60]\n"
        "  str r1, [sp, #64]\n"
        "  movs r1, #0\n"
        "  ldrsh r0, [r2, r1]\n"
        "  cmp r0, #0\n"
        "  bge .Lcourse_8ca0_08008D76\n"
        "  adds r0, #3\n"
        ".Lcourse_8ca0_08008D76:\n"
        "  lsls r0, r0, #14\n"
        "  lsrs r4, r0, #16\n"
        "  asrs r0, r0, #16\n"
        "  cmp r0, #111\n"
        "  bgt .Lcourse_8ca0_08008D82\n"
        "  movs r4, #112\n"
        ".Lcourse_8ca0_08008D82:\n"
        "  lsls r4, r4, #20\n"
        "  lsrs r4, r4, #20\n"
        "  mov r0, sl\n"
        "  adds r1, r4, #0\n"
        "  bl 0x08005BA8\n"
        "  ldr r0, [sp, #68]\n"
        "  adds r1, r4, #0\n"
        "  bl 0x08005BA8\n"
        "  mov r0, r8\n"
        "  adds r1, r4, #0\n"
        "  bl 0x08005BA8\n"
        "  mov r0, r9\n"
        "  adds r1, r4, #0\n"
        "  bl 0x08005BA8\n"
        "  ldr r6, [sp, #52]\n"
        "  adds r6, #199\n"
        "  lsls r6, r6, #16\n"
        "  lsrs r6, r6, #16\n"
        "  str r6, [sp, #72]\n"
        "  mov r2, r8\n"
        "  ldr r4, [r2, #4]\n"
        "  adds r4, #134\n"
        "  lsls r4, r4, #16\n"
        "  lsrs r4, r4, #16\n"
        "  ldr r3, [sp, #60]\n"
        "  mov r8, r3\n"
        "  movs r5, #203\n"
        "  add r8, r5\n"
        "  mov r6, r8\n"
        "  lsls r6, r6, #16\n"
        "  lsrs r6, r6, #16\n"
        "  mov r8, r6\n"
        "  mov r0, r9\n"
        "  ldr r5, [r0, #4]\n"
        "  adds r5, #138\n"
        "  lsls r5, r5, #16\n"
        "  lsrs r5, r5, #16\n"
        "  str r5, [sp, #76]\n"
        "  bl 0x08002BE8\n"
        "  mov r9, r0\n"
        "  mov r1, r9\n"
        "  lsls r1, r1, #16\n"
        "  lsrs r1, r1, #16\n"
        "  mov r9, r1\n"
        "  ldr r1, [sp, #36]\n"
        "  lsls r1, r1, #16\n"
        "  asrs r1, r1, #16\n"
        "  mov r3, sl\n"
        "  movs r5, #4\n"
        "  ldrsh r2, [r3, r5]\n"
        "  ldr r3, [sp, #44]\n"
        "  lsls r3, r3, #16\n"
        "  asrs r3, r3, #16\n"
        "  ldr r6, [sp, #68]\n"
        "  movs r5, #4\n"
        "  ldrsh r0, [r6, r5]\n"
        "  str r0, [sp, #0]\n"
        "  mov r0, r9\n"
        "  bl 0x08002D98\n"
        "  lsls r4, r4, #16\n"
        "  asrs r4, r4, #16\n"
        "  movs r6, #255\n"
        "  mov sl, r6\n"
        "  ands r4, r6\n"
        "  movs r0, #128\n"
        "  lsls r0, r0, #1\n"
        "  adds r3, r0, #0\n"
        "  orrs r4, r3\n"
        "  strh r4, [r7, #0]\n"
        "  ldr r1, [sp, #72]\n"
        "  lsls r6, r1, #16\n"
        "  asrs r6, r6, #16\n"
        "  ldr r2, .Lcourse_8ca0_08008E94\n"
        "  ands r6, r2\n"
        "  mov r4, r9\n"
        "  lsls r4, r4, #9\n"
        "  mov r9, r4\n"
        "  movs r5, #128\n"
        "  lsls r5, r5, #7\n"
        "  adds r1, r5, #0\n"
        "  mov r0, r9\n"
        "  orrs r0, r1\n"
        "  orrs r6, r0\n"
        "  strh r6, [r7, #2]\n"
        "  ldr r6, .Lcourse_8ca0_08008E8C\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r4, [r1, #38]\n"
        "  lsls r0, r4, #12\n"
        "  ldrh r1, [r1, #44]\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  ldr r6, [sp, #76]\n"
        "  lsls r5, r6, #16\n"
        "  asrs r5, r5, #16\n"
        "  mov r0, sl\n"
        "  ands r5, r0\n"
        "  orrs r5, r3\n"
        "  strh r5, [r7, #0]\n"
        "  mov r1, r8\n"
        "  lsls r1, r1, #16\n"
        "  asrs r1, r1, #16\n"
        "  ands r1, r2\n"
        "  mov r2, r9\n"
        "  orrs r1, r2\n"
        "  strh r1, [r7, #2]\n"
        "  ldr r3, .Lcourse_8ca0_08008E8C\n"
        "  ldr r1, [r3, #0]\n"
        "  ldrh r4, [r1, #38]\n"
        "  lsls r0, r4, #12\n"
        "  ldrh r1, [r1, #46]\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  ldr r0, .Lcourse_8ca0_08008EA8\n"
        "  ldr r5, .Lcourse_8ca0_08008EAC\n"
        "  adds r0, r0, r5\n"
        "  movs r6, #0\n"
        "  ldrsh r0, [r0, r6]\n"
        "  cmp r0, #8\n"
        "  bls .Lcourse_8ca0_08008E82\n"
        "  b .Lcourse_8ca0_080095D4\n"
        ".Lcourse_8ca0_08008E82:\n"
        "  lsls r0, r0, #2\n"
        "  ldr r1, .Lcourse_8ca0_08008EB0\n"
        "  adds r0, r0, r1\n"
        "  ldr r0, [r0, #0]\n"
        "  mov pc, r0\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_08008E8C: .word 0x030003E0\n"
        ".Lcourse_8ca0_08008E90: .word 0x0000808F\n"
        ".Lcourse_8ca0_08008E94: .word 0x000001FF\n"
        ".Lcourse_8ca0_08008E98: .word 0x0805F634\n"
        ".Lcourse_8ca0_08008E9C: .word 0x0805F63C\n"
        ".Lcourse_8ca0_08008EA0: .word 0x0805F644\n"
        ".Lcourse_8ca0_08008EA4: .word 0x0805F64C\n"
        ".Lcourse_8ca0_08008EA8: .word 0x03001780\n"
        ".Lcourse_8ca0_08008EAC: .word 0x000010FC\n"
        ".Lcourse_8ca0_08008EB0: .word 0x08008EB4\n"
        ".Lcourse_8ca0_08008EB4: .word 0x080095D4\n"
        ".Lcourse_8ca0_08008EB8: .word 0x08008ED8\n"
        ".Lcourse_8ca0_08008EBC: .word 0x08009278\n"
        ".Lcourse_8ca0_08008EC0: .word 0x080090A4\n"
        ".Lcourse_8ca0_08008EC4: .word 0x08008ED8\n"
        ".Lcourse_8ca0_08008EC8: .word 0x0800928C\n"
        ".Lcourse_8ca0_08008ECC: .word 0x080095D4\n"
        ".Lcourse_8ca0_08008ED0: .word 0x080095D4\n"
        ".Lcourse_8ca0_08008ED4: .word 0x08008ED8\n"
        ".Lcourse_8ca0_08008ED8:\n"
        "  ldr r6, .Lcourse_8ca0_08009098\n"
        "  ldr r0, [r6, #0]\n"
        "  ldr r1, [r0, #64]\n"
        "  adds r0, r7, #0\n"
        "  bl sub_08008768\n"
        "  adds r7, r0, #0\n"
        "  ldr r0, [r6, #0]\n"
        "  ldr r1, [r0, #68]\n"
        "  adds r0, r7, #0\n"
        "  bl sub_080088B0\n"
        "  adds r7, r0, #0\n"
        "  movs r2, #2\n"
        "  strh r2, [r7, #0]\n"
        "  ldr r1, .Lcourse_8ca0_0800909C\n"
        "  adds r0, r1, #0\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r3, [r1, #38]\n"
        "  lsls r0, r3, #12\n"
        "  ldrh r1, [r1, #50]\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  strh r2, [r7, #0]\n"
        "  ldr r4, .Lcourse_8ca0_080090A0\n"
        "  adds r0, r4, #0\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r5, [r1, #38]\n"
        "  lsls r0, r5, #12\n"
        "  ldrh r1, [r1, #48]\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  movs r0, #128\n"
        "  lsls r0, r0, #8\n"
        "  adds r2, r0, #0\n"
        "  strh r2, [r7, #0]\n"
        "  movs r0, #220\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r0, [r1, #52]\n"
        "  adds r0, #24\n"
        "  ldrh r1, [r1, #54]\n"
        "  lsls r1, r1, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  strh r2, [r7, #0]\n"
        "  movs r0, #227\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r2, [r6, #0]\n"
        "  ldrh r1, [r2, #10]\n"
        "  lsls r0, r1, #1\n"
        "  ldrh r3, [r2, #52]\n"
        "  adds r0, r3, r0\n"
        "  ldrh r4, [r2, #54]\n"
        "  lsls r1, r4, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  adds r0, r2, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r2, #38]\n"
        "  str r1, [sp, #0]\n"
        "  movs r5, #1\n"
        "  str r5, [sp, #4]\n"
        "  movs r4, #0\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #12\n"
        "  movs r2, #0\n"
        "  movs r3, #0\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #15\n"
        "  movs r2, #64\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #15\n"
        "  movs r2, #80\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #15\n"
        "  movs r2, #96\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #16\n"
        "  movs r2, #104\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #13\n"
        "  movs r2, #120\n"
        "  movs r3, #0\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #16\n"
        "  movs r2, #168\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #14\n"
        "  movs r2, #184\n"
        "  movs r3, #0\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #15\n"
        "  movs r2, #208\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #16\n"
        "  movs r2, #224\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r2, [r1, #4]\n"
        "  movs r5, #4\n"
        "  ldrsh r0, [r1, r5]\n"
        "  cmp r0, #0\n"
        "  bgt .Lcourse_8ca0_0800906A\n"
        "  b .Lcourse_8ca0_080095D4\n"
        ".Lcourse_8ca0_0800906A:\n"
        "  subs r0, r2, #1\n"
        "  strh r0, [r1, #4]\n"
        "  lsls r0, r0, #16\n"
        "  asrs r0, r0, #16\n"
        "  cmp r0, #63\n"
        "  ble .Lcourse_8ca0_0800908C\n"
        "  movs r2, #4\n"
        "  ldrsh r0, [r1, r2]\n"
        "  movs r1, #8\n"
        "  bl 0x0802D978\n"
        "  movs r1, #2\n"
        "  bl 0x0802D97C\n"
        "  cmp r0, #0\n"
        "  bne .Lcourse_8ca0_0800908C\n"
        "  b .Lcourse_8ca0_080095D4\n"
        ".Lcourse_8ca0_0800908C:\n"
        "  ldr r0, [r6, #0]\n"
        "  ldr r1, [r0, #72]\n"
        "  adds r0, r7, #0\n"
        "  bl sub_0800861C\n"
        "  b .Lcourse_8ca0_080095D2\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_08009098: .word 0x030003E0\n"
        ".Lcourse_8ca0_0800909C: .word 0x000040A7\n"
        ".Lcourse_8ca0_080090A0: .word 0x000040CD\n"
        ".Lcourse_8ca0_080090A4:\n"
        "  ldr r4, .Lcourse_8ca0_080090FC\n"
        "  ldr r0, [r4, #0]\n"
        "  adds r0, #78\n"
        "  ldrb r0, [r0, #0]\n"
        "  cmp r0, #0\n"
        "  beq .Lcourse_8ca0_080090C8\n"
        "  ldr r0, .Lcourse_8ca0_08009100\n"
        "  ldr r3, .Lcourse_8ca0_08009104\n"
        "  adds r0, r0, r3\n"
        "  ldr r0, [r0, #0]\n"
        "  movs r1, #3\n"
        "  bl 0x0802D978\n"
        "  movs r1, #2\n"
        "  bl 0x0802D97C\n"
        "  cmp r0, #0\n"
        "  beq .Lcourse_8ca0_080090D4\n"
        ".Lcourse_8ca0_080090C8:\n"
        "  ldr r0, [r4, #0]\n"
        "  ldr r1, [r0, #64]\n"
        "  adds r0, r7, #0\n"
        "  bl sub_08008768\n"
        "  adds r7, r0, #0\n"
        ".Lcourse_8ca0_080090D4:\n"
        "  ldr r4, .Lcourse_8ca0_080090FC\n"
        "  ldr r1, [r4, #0]\n"
        "  ldrh r0, [r1, #34]\n"
        "  cmp r0, #0\n"
        "  beq .Lcourse_8ca0_08009108\n"
        "  subs r0, #1\n"
        "  strh r0, [r1, #34]\n"
        "  ldrh r0, [r1, #34]\n"
        "  movs r1, #3\n"
        "  bl 0x0802D978\n"
        "  movs r1, #2\n"
        "  bl 0x0802D97C\n"
        "  cmp r0, #0\n"
        "  beq .Lcourse_8ca0_08009112\n"
        "  ldr r0, [r4, #0]\n"
        "  ldr r1, [r0, #72]\n"
        "  b .Lcourse_8ca0_0800910A\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_080090FC: .word 0x030003E0\n"
        ".Lcourse_8ca0_08009100: .word 0x03001780\n"
        ".Lcourse_8ca0_08009104: .word 0x000010D8\n"
        ".Lcourse_8ca0_08009108:\n"
        "  ldr r1, [r1, #68]\n"
        ".Lcourse_8ca0_0800910A:\n"
        "  adds r0, r7, #0\n"
        "  bl sub_080088B0\n"
        "  adds r7, r0, #0\n"
        ".Lcourse_8ca0_08009112:\n"
        "  movs r0, #2\n"
        "  strh r0, [r7, #0]\n"
        "  ldr r4, .Lcourse_8ca0_08009270\n"
        "  adds r0, r4, #0\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r6, .Lcourse_8ca0_08009274\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r5, [r1, #38]\n"
        "  lsls r0, r5, #12\n"
        "  ldrh r1, [r1, #48]\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  movs r0, #128\n"
        "  lsls r0, r0, #8\n"
        "  adds r2, r0, #0\n"
        "  strh r2, [r7, #0]\n"
        "  movs r0, #220\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r0, [r1, #52]\n"
        "  adds r0, #24\n"
        "  ldrh r1, [r1, #54]\n"
        "  lsls r1, r1, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  strh r2, [r7, #0]\n"
        "  movs r0, #227\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r2, [r6, #0]\n"
        "  ldrh r1, [r2, #10]\n"
        "  lsls r0, r1, #1\n"
        "  ldrh r3, [r2, #52]\n"
        "  adds r0, r3, r0\n"
        "  ldrh r4, [r2, #54]\n"
        "  lsls r1, r4, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  adds r0, r2, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r2, #38]\n"
        "  str r1, [sp, #0]\n"
        "  movs r5, #1\n"
        "  str r5, [sp, #4]\n"
        "  movs r4, #0\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #12\n"
        "  movs r2, #0\n"
        "  movs r3, #0\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #15\n"
        "  movs r2, #64\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #15\n"
        "  movs r2, #80\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #15\n"
        "  movs r2, #96\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #16\n"
        "  movs r2, #104\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #14\n"
        "  movs r2, #184\n"
        "  movs r3, #0\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #15\n"
        "  movs r2, #208\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #16\n"
        "  movs r2, #224\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r2, [r1, #4]\n"
        "  movs r5, #4\n"
        "  ldrsh r0, [r1, r5]\n"
        "  cmp r0, #0\n"
        "  bgt .Lcourse_8ca0_08009242\n"
        "  b .Lcourse_8ca0_080095D4\n"
        ".Lcourse_8ca0_08009242:\n"
        "  subs r0, r2, #1\n"
        "  strh r0, [r1, #4]\n"
        "  lsls r0, r0, #16\n"
        "  asrs r0, r0, #16\n"
        "  cmp r0, #63\n"
        "  ble .Lcourse_8ca0_08009264\n"
        "  movs r2, #4\n"
        "  ldrsh r0, [r1, r2]\n"
        "  movs r1, #8\n"
        "  bl 0x0802D978\n"
        "  movs r1, #2\n"
        "  bl 0x0802D97C\n"
        "  cmp r0, #0\n"
        "  bne .Lcourse_8ca0_08009264\n"
        "  b .Lcourse_8ca0_080095D4\n"
        ".Lcourse_8ca0_08009264:\n"
        "  ldr r0, [r6, #0]\n"
        "  ldr r1, [r0, #72]\n"
        "  adds r0, r7, #0\n"
        "  bl sub_0800861C\n"
        "  b .Lcourse_8ca0_080095D2\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_08009270: .word 0x000040CD\n"
        ".Lcourse_8ca0_08009274: .word 0x030003E0\n"
        ".Lcourse_8ca0_08009278:\n"
        "  ldr r0, .Lcourse_8ca0_08009288\n"
        "  ldr r0, [r0, #0]\n"
        "  ldr r1, [r0, #68]\n"
        "  adds r0, r7, #0\n"
        "  bl sub_080088B0\n"
        "  b .Lcourse_8ca0_080095D2\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_08009288: .word 0x030003E0\n"
        ".Lcourse_8ca0_0800928C:\n"
        "  movs r0, #2\n"
        "  strh r0, [r7, #0]\n"
        "  ldr r3, .Lcourse_8ca0_080093B8\n"
        "  adds r0, r3, #0\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r6, .Lcourse_8ca0_080093BC\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r4, [r1, #38]\n"
        "  lsls r0, r4, #12\n"
        "  ldrh r1, [r1, #48]\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  movs r5, #128\n"
        "  lsls r5, r5, #8\n"
        "  adds r2, r5, #0\n"
        "  strh r2, [r7, #0]\n"
        "  movs r0, #220\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r1, [r6, #0]\n"
        "  ldrh r0, [r1, #52]\n"
        "  adds r0, #24\n"
        "  ldrh r1, [r1, #54]\n"
        "  lsls r1, r1, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  strh r2, [r7, #0]\n"
        "  movs r0, #227\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r2, [r6, #0]\n"
        "  ldrh r1, [r2, #10]\n"
        "  lsls r0, r1, #1\n"
        "  ldrh r3, [r2, #52]\n"
        "  adds r0, r3, r0\n"
        "  ldrh r4, [r2, #54]\n"
        "  lsls r1, r4, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  adds r0, r2, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r2, #38]\n"
        "  str r1, [sp, #0]\n"
        "  movs r5, #1\n"
        "  str r5, [sp, #4]\n"
        "  movs r4, #0\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #14\n"
        "  movs r2, #184\n"
        "  movs r3, #0\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #15\n"
        "  movs r2, #208\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  adds r0, r1, #0\n"
        "  adds r0, #88\n"
        "  ldrh r1, [r1, #38]\n"
        "  str r1, [sp, #0]\n"
        "  str r5, [sp, #4]\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  movs r1, #16\n"
        "  movs r2, #224\n"
        "  movs r3, #4\n"
        "  bl sub_08007B18\n"
        "  ldr r1, [r6, #0]\n"
        "  movs r5, #26\n"
        "  ldrsh r0, [r1, r5]\n"
        "  cmp r0, #0\n"
        "  ble .Lcourse_8ca0_080093D0\n"
        "  bl 0x08002BE8\n"
        "  ldr r2, [r6, #0]\n"
        "  ldrh r1, [r2, #26]\n"
        "  subs r1, #1\n"
        "  strh r1, [r2, #26]\n"
        "  lsls r0, r0, #16\n"
        "  asrs r5, r0, #16\n"
        "  ldr r2, .Lcourse_8ca0_080093C0\n"
        "  movs r0, #15\n"
        "  ands r1, r0\n"
        "  lsls r1, r1, #1\n"
        "  adds r1, r1, r2\n"
        "  ldrh r1, [r1, #0]\n"
        "  adds r0, r5, #0\n"
        "  bl 0x08002C48\n"
        "  ldr r0, [r6, #0]\n"
        "  movs r1, #18\n"
        "  ldrsh r0, [r0, r1]\n"
        "  cmp r0, #9\n"
        "  ble .Lcourse_8ca0_08009390\n"
        "  ldr r2, .Lcourse_8ca0_080093C4\n"
        "  adds r0, r2, #0\n"
        "  strh r0, [r7, #0]\n"
        "  lsls r0, r5, #9\n"
        "  ldr r3, .Lcourse_8ca0_080093C8\n"
        "  adds r1, r3, #0\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r4, [r6, #0]\n"
        "  movs r1, #18\n"
        "  ldrsh r0, [r4, r1]\n"
        "  movs r1, #10\n"
        "  bl 0x0802DE04\n"
        "  lsls r0, r0, #16\n"
        "  asrs r0, r0, #14\n"
        "  ldrh r2, [r4, #40]\n"
        "  adds r0, r2, r0\n"
        "  ldrh r4, [r4, #38]\n"
        "  lsls r1, r4, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        ".Lcourse_8ca0_08009390:\n"
        "  ldr r3, .Lcourse_8ca0_080093C4\n"
        "  adds r0, r3, #0\n"
        "  strh r0, [r7, #0]\n"
        "  lsls r0, r5, #9\n"
        "  ldr r4, .Lcourse_8ca0_080093CC\n"
        "  adds r1, r4, #0\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r4, [r6, #0]\n"
        "  movs r5, #18\n"
        "  ldrsh r0, [r4, r5]\n"
        "  movs r1, #10\n"
        "  bl 0x0802DE9C\n"
        "  lsls r0, r0, #16\n"
        "  asrs r0, r0, #14\n"
        "  ldrh r6, [r4, #40]\n"
        "  adds r0, r6, r0\n"
        "  b .Lcourse_8ca0_0800941E\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_080093B8: .word 0x000040CD\n"
        ".Lcourse_8ca0_080093BC: .word 0x030003E0\n"
        ".Lcourse_8ca0_080093C0: .word 0x080CB154\n"
        ".Lcourse_8ca0_080093C4: .word 0x00000386\n"
        ".Lcourse_8ca0_080093C8: .word 0x00004044\n"
        ".Lcourse_8ca0_080093CC: .word 0x00004050\n"
        ".Lcourse_8ca0_080093D0:\n"
        "  movs r2, #18\n"
        "  ldrsh r0, [r1, r2]\n"
        "  cmp r0, #9\n"
        "  ble .Lcourse_8ca0_08009400\n"
        "  movs r0, #142\n"
        "  strh r0, [r7, #0]\n"
        "  ldr r3, .Lcourse_8ca0_08009514\n"
        "  adds r0, r3, #0\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r4, [r6, #0]\n"
        "  movs r5, #18\n"
        "  ldrsh r0, [r4, r5]\n"
        "  movs r1, #10\n"
        "  bl 0x0802DE04\n"
        "  lsls r0, r0, #16\n"
        "  asrs r0, r0, #14\n"
        "  ldrh r1, [r4, #40]\n"
        "  adds r0, r1, r0\n"
        "  ldrh r4, [r4, #38]\n"
        "  lsls r1, r4, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        ".Lcourse_8ca0_08009400:\n"
        "  movs r0, #142\n"
        "  strh r0, [r7, #0]\n"
        "  ldr r2, .Lcourse_8ca0_08009518\n"
        "  adds r0, r2, #0\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r4, [r6, #0]\n"
        "  movs r3, #18\n"
        "  ldrsh r0, [r4, r3]\n"
        "  movs r1, #10\n"
        "  bl 0x0802DE9C\n"
        "  lsls r0, r0, #16\n"
        "  asrs r0, r0, #14\n"
        "  ldrh r5, [r4, #40]\n"
        "  adds r0, r5, r0\n"
        ".Lcourse_8ca0_0800941E:\n"
        "  ldrh r4, [r4, #38]\n"
        "  lsls r1, r4, #12\n"
        "  orrs r0, r1\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  ldr r2, .Lcourse_8ca0_0800951C\n"
        "  ldr r0, [r2, #0]\n"
        "  ldrh r0, [r0, #32]\n"
        "  cmp r0, #0\n"
        "  beq .Lcourse_8ca0_0800946A\n"
        "  cmp r0, #16\n"
        "  bhi .Lcourse_8ca0_0800946A\n"
        "  movs r6, #144\n"
        "  lsls r6, r6, #3\n"
        "  adds r0, r6, #0\n"
        "  strh r0, [r7, #0]\n"
        "  ldr r1, .Lcourse_8ca0_08009520\n"
        "  adds r0, r1, #0\n"
        "  strh r0, [r7, #2]\n"
        "  ldr r2, [r2, #0]\n"
        "  ldrh r3, [r2, #38]\n"
        "  lsls r0, r3, #12\n"
        "  ldrh r4, [r2, #42]\n"
        "  orrs r0, r4\n"
        "  strh r0, [r7, #4]\n"
        "  adds r7, #8\n"
        "  ldr r1, .Lcourse_8ca0_08009524\n"
        "  movs r5, #232\n"
        "  lsls r5, r5, #3\n"
        "  adds r0, r5, #0\n"
        "  strh r0, [r1, #0]\n"
        "  ldr r3, .Lcourse_8ca0_08009528\n"
        "  ldrh r1, [r2, #32]\n"
        "  movs r0, #16\n"
        "  subs r0, r0, r1\n"
        "  lsls r0, r0, #8\n"
        "  orrs r0, r1\n"
        "  strh r0, [r3, #0]\n"
        ".Lcourse_8ca0_0800946A:\n"
        "  ldr r4, .Lcourse_8ca0_0800951C\n"
        "  ldr r1, [r4, #0]\n"
        "  ldrh r2, [r1, #28]\n"
        "  movs r6, #28\n"
        "  ldrsh r0, [r1, r6]\n"
        "  cmp r0, #0\n"
        "  ble .Lcourse_8ca0_080094F4\n"
        "  subs r0, r2, #1\n"
        "  movs r2, #0\n"
        "  mov r9, r2\n"
        "  strh r0, [r1, #28]\n"
        "  adds r1, #79\n"
        "  ldrb r0, [r1, #0]\n"
        "  adds r0, #1\n"
        "  strb r0, [r1, #0]\n"
        "  ldr r0, [r4, #0]\n"
        "  adds r1, r0, #0\n"
        "  adds r1, #79\n"
        "  ldrb r3, [r1, #0]\n"
        "  cmp r3, #15\n"
        "  bls .Lcourse_8ca0_08009498\n"
        "  movs r0, #1\n"
        "  strb r0, [r1, #0]\n"
        ".Lcourse_8ca0_08009498:\n"
        "  ldr r0, .Lcourse_8ca0_0800952C\n"
        "  ldr r1, [r4, #0]\n"
        "  adds r2, r1, #0\n"
        "  adds r2, #79\n"
        "  ldrb r2, [r2, #0]\n"
        "  ldrh r3, [r1, #58]\n"
        "  movs r1, #1\n"
        "  bl sub_08007614\n"
        "  ldr r4, [r4, #0]\n"
        "  mov r8, r4\n"
        "  mov r0, r8\n"
        "  adds r0, #96\n"
        "  ldrh r1, [r4, #62]\n"
        "  ldr r2, .Lcourse_8ca0_08009530\n"
        "  movs r5, #30\n"
        "  ldrsh r4, [r4, r5]\n"
        "  lsls r4, r4, #3\n"
        "  adds r4, r4, r2\n"
        "  movs r6, #4\n"
        "  ldrsh r2, [r4, r6]\n"
        "  movs r5, #0\n"
        "  ldrsh r3, [r4, r5]\n"
        "  movs r5, #2\n"
        "  ldrsh r6, [r4, r5]\n"
        "  mov sl, r6\n"
        "  mov r6, r8\n"
        "  movs r4, #28\n"
        "  ldrsh r5, [r6, r4]\n"
        "  movs r4, #60\n"
        "  subs r4, r4, r5\n"
        "  lsrs r5, r4, #31\n"
        "  adds r4, r4, r5\n"
        "  asrs r4, r4, #1\n"
        "  mov r5, sl\n"
        "  subs r6, r5, r4\n"
        "  str r6, [sp, #0]\n"
        "  mov r6, r8\n"
        "  ldrh r4, [r6, #58]\n"
        "  str r4, [sp, #4]\n"
        "  mov r4, r9\n"
        "  str r4, [sp, #8]\n"
        "  str r4, [sp, #12]\n"
        "  str r4, [sp, #16]\n"
        "  bl sub_08007BFC\n"
        ".Lcourse_8ca0_080094F4:\n"
        "  ldr r2, .Lcourse_8ca0_0800951C\n"
        "  ldr r4, [r2, #0]\n"
        "  movs r0, #168\n"
        "  str r0, [r4, #80]\n"
        "  movs r5, #80\n"
        "  str r5, [r4, #84]\n"
        "  movs r6, #24\n"
        "  ldrsh r0, [r4, r6]\n"
        "  cmp r0, #2\n"
        "  beq .Lcourse_8ca0_080095A0\n"
        "  cmp r0, #2\n"
        "  bgt .Lcourse_8ca0_08009534\n"
        "  cmp r0, #1\n"
        "  beq .Lcourse_8ca0_0800953A\n"
        "  b .Lcourse_8ca0_080095D4\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_08009514: .word 0x0000404C\n"
        ".Lcourse_8ca0_08009518: .word 0x00004058\n"
        ".Lcourse_8ca0_0800951C: .word 0x030003E0\n"
        ".Lcourse_8ca0_08009520: .word 0x000040A0\n"
        ".Lcourse_8ca0_08009524: .word 0x04000050\n"
        ".Lcourse_8ca0_08009528: .word 0x04000052\n"
        ".Lcourse_8ca0_0800952C: .word 0x0828A35C\n"
        ".Lcourse_8ca0_08009530: .word 0x080CB074\n"
        ".Lcourse_8ca0_08009534:\n"
        "  cmp r0, #3\n"
        "  beq .Lcourse_8ca0_0800956C\n"
        "  b .Lcourse_8ca0_080095D4\n"
        ".Lcourse_8ca0_0800953A:\n"
        "  ldrh r1, [r4, #22]\n"
        "  subs r1, #1\n"
        "  strh r1, [r4, #22]\n"
        "  ldr r2, .Lcourse_8ca0_08009568\n"
        "  movs r3, #22\n"
        "  ldrsh r0, [r4, r3]\n"
        "  lsls r0, r0, #1\n"
        "  adds r0, r0, r2\n"
        "  movs r6, #0\n"
        "  ldrsh r0, [r0, r6]\n"
        "  adds r0, #168\n"
        "  str r0, [r4, #80]\n"
        "  str r5, [r4, #84]\n"
        "  lsls r1, r1, #16\n"
        "  cmp r1, #0\n"
        "  bgt .Lcourse_8ca0_0800955E\n"
        "  movs r0, #2\n"
        "  strh r0, [r4, #24]\n"
        ".Lcourse_8ca0_0800955E:\n"
        "  adds r0, r7, #0\n"
        "  bl sub_08008AAC\n"
        "  b .Lcourse_8ca0_080095D2\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_08009568: .word 0x080CB0CC\n"
        ".Lcourse_8ca0_0800956C:\n"
        "  ldrh r2, [r4, #22]\n"
        "  subs r2, #1\n"
        "  strh r2, [r4, #22]\n"
        "  ldr r3, .Lcourse_8ca0_0800959C\n"
        "  movs r0, #22\n"
        "  ldrsh r1, [r4, r0]\n"
        "  movs r0, #16\n"
        "  subs r0, r0, r1\n"
        "  lsls r0, r0, #1\n"
        "  adds r0, r0, r3\n"
        "  movs r1, #0\n"
        "  ldrsh r0, [r0, r1]\n"
        "  adds r0, #168\n"
        "  str r0, [r4, #80]\n"
        "  str r5, [r4, #84]\n"
        "  lsls r2, r2, #16\n"
        "  cmp r2, #0\n"
        "  bgt .Lcourse_8ca0_08009594\n"
        "  movs r0, #0\n"
        "  strh r0, [r4, #24]\n"
        ".Lcourse_8ca0_08009594:\n"
        "  adds r0, r7, #0\n"
        "  bl sub_08008BB8\n"
        "  b .Lcourse_8ca0_080095D2\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_0800959C: .word 0x080CB0EE\n"
        ".Lcourse_8ca0_080095A0:\n"
        "  adds r0, r4, #0\n"
        "  adds r0, #79\n"
        "  ldrb r1, [r0, #0]\n"
        "  adds r1, #1\n"
        "  strb r1, [r0, #0]\n"
        "  ldr r0, [r2, #0]\n"
        "  adds r1, r0, #0\n"
        "  adds r1, #79\n"
        "  ldrb r3, [r1, #0]\n"
        "  cmp r3, #15\n"
        "  bls .Lcourse_8ca0_080095BA\n"
        "  movs r0, #1\n"
        "  strb r0, [r1, #0]\n"
        ".Lcourse_8ca0_080095BA:\n"
        "  ldr r0, .Lcourse_8ca0_080095EC\n"
        "  ldr r1, [r2, #0]\n"
        "  adds r2, r1, #0\n"
        "  adds r2, #79\n"
        "  ldrb r2, [r2, #0]\n"
        "  ldrh r3, [r1, #58]\n"
        "  movs r1, #1\n"
        "  bl sub_08007614\n"
        "  adds r0, r7, #0\n"
        "  bl sub_080089FC\n"
        ".Lcourse_8ca0_080095D2:\n"
        "  adds r7, r0, #0\n"
        ".Lcourse_8ca0_080095D4:\n"
        "  adds r0, r7, #0\n"
        "  bl 0x0800313C\n"
        "  add sp, #80\n"
        "  pop {r3, r4, r5}\n"
        "  mov r8, r3\n"
        "  mov r9, r4\n"
        "  mov sl, r5\n"
        "  pop {r4, r5, r6, r7}\n"
        "  pop {r0}\n"
        "  bx r0\n"
        "  .align 2, 0\n"
        ".Lcourse_8ca0_080095EC: .word 0x0828A35C\n"
        ".syntax divided\n"
    );
}
void _08008CA0(void) __attribute__((alias("CoursePhaseMachine_08CA0")));
void sub_08008CA0(void) __attribute__((alias("CoursePhaseMachine_08CA0")));

// __APPEND_MARKER__

// ROM entry alias.
s32 _0802DE9C(s32 num, s32 den) __attribute__((alias("DivRemS_02DE9C")));

// ROM entry alias.
void sub_08007B8C(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) __attribute__((alias("CourseEmit7B8C")));

// ROM entry alias.
void Sub_08007B18(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) __attribute__((alias("CourseEmit7B18")));
void sub_08007B18(void *rec, int kind, int dx, int dy,
                    u32 s0, u32 s1, u32 s2, u32 s3) __attribute__((alias("CourseEmit7B18")));

// 0x0800BC04 — 2B `bx lr` no-op (menu_bc04.s tail slot; caller at
// :355 is the same sequence, which the ROM proves does nothing).
void MenuNoop_BC04(void) {}
__asm__(".align 2, 0");
void _0800BC04(void) __attribute__((alias("MenuNoop_BC04")));
void sub_0800BC04(void) __attribute__((alias("MenuNoop_BC04")));
