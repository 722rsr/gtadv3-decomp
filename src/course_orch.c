#include "gtadv/course_orch.h"
#include "gba/bios.h"

// Reference: asm/course_orch.s + course_load.s
// Each function mirrors the Thumb listing; register-built addresses kept as constants.

extern void *_08006590(void *base,int grp,int idx);
extern void _0802D984(void *src, void *dst); // LZ77UnCompVram
extern void _0802D988(void *src, void *dst); // LZ77UnCompWram
extern void _0802D974(const void *src, void *dst, u32 ctrl); // CpuFastSet/ CpuSet wrapper

// Call-site split for the five helpers _08006138 dispatches to. The closure
// spells every one of them by VMA (verified `nm -n build-code/code.o`:
// _080064EC/_08006468/_080061B8/_0800628C/_08006574 all resolve), and
// promotion_screen derives rule 6 from closure branch targets, so a friendly
// name here reads as "no VMA and not a promoted export" and the probe reports
// UNRESOLVED. Every closure spelling below also has a real C definition:
// _080064EC/_08006468/_080061B8/_0800628C are aliases in course_orch_helpers.c,
// _08006574 is the Course_SurfaceLoad alias at the foot of this file.
// Same shape as RS_CALLEE (src/race_scene.c), FF_CALLEE (menu_ff78_f.c),
// MS_CALLEE (src/menu_stage.c) and R35_CALLEE (rec35_mid_region.c).
#ifndef __APPLE__
#define CO_CALLEE(friendly, closure) closure
extern void _080064EC(void *out, int idx, void *base); // Course_ParseHeader
extern void _08006468(void *out, int idx, void *base); // Course_ThemeGfx
extern void _080061B8(void *out, int idx, void *base); // Course_VariantGfx
extern void _0800628C(void *out, int idx, void *base); // Course_BigGfx
extern void _08006574(void *a, int b, void *c);        // Course_SurfaceLoad
extern s16 _08025908(int idx);                        // calendar variant
extern s16 _080258F4(int idx);                        // calendar default
extern void _080060EC(void *src);                // Course_VariantCopy
#else
#define CO_CALLEE(friendly, closure) friendly
// Host-build declarations. The `_080258F4` / `_08025908` aliases that the ARM
// build calls live in src/course_cal.c under `#ifndef __APPLE__` (aliases are
// unsupported on Darwin), so the Apple branch needs the friendly names the
// split expands to. Both are real C definitions there.
extern s16 Course_GetCourseDefault(int idx);
extern s16 Course_GetCourseVariant(int idx);
#endif

void Course_VariantCopy(void *src) {
    // _080060EC: CpuFastSet(src, 0x0203F758, 4)
    _0802D974(src, (void *)0x0203F758, 4);
}
void *Course_Group0Payload12(int idx, void *base) {
    // _08006100: seek group0 idx at base, +12
    void *p = _08006590(base, 0, idx);
    return (u8 *)p + 12;
}
__asm__(".align 2, 0");
void Course_VariantZero(void) {
    // _08006114: stack-staged zero HALFWORD, CpuSet (svc 11) copy 1 word to 0x0203F758 ctrl 0x01000004
    volatile u16 zero = 0;
    _0802D974((const void *)&zero, (void *)0x0203F758, 0x01000004);
}
void Course_ParseHeader(void *out, int idx, void *base) {
    // _080064EC: parse group0 header into out struct fields +00..+32 flag
    u8 *payload = (u8 *)_08006590(base, 0, idx);
    // Offsets payload-relative as per track_formats.doc
    s16 nA = *(s16 *)(payload+2);
    s16 nB = *(s16 *)(payload+4);
    s16 nC = *(s16 *)(payload+6);
    s16 nD = *(s16 *)(payload+8);
    s16 c5 = *(s16 *)(payload+10);
    u8 *secA = payload+28;
    *(void **)out = payload+12;
    *(void **)((u8 *)out+4) = secA;
    // Compute section ends via strides
    u8 *afterA = secA + nA*20;
    u8 *afterB = afterA + nB*12;
    u8 *afterC = afterB + nC*12;
    u8 *afterD = afterC + nD*8;
    *(void **)((u8 *)out+8) = afterA;
    *(void **)((u8 *)out+12)= afterB;
    *(void **)((u8 *)out+16)= afterC;
    *(void **)((u8 *)out+20)= afterD;
    *(void **)((u8 *)out+24)= afterD+60;
    *(s16 *)((u8 *)out+28)= nA;
    *(s16 *)((u8 *)out+42)= nB;
    *(s16 *)((u8 *)out+44)= nC;
    *(s16 *)((u8 *)out+46)= nD;
    *(s16 *)((u8 *)out+48)= c5;
    u8 v = *(payload+14); // byte
    u8 diff = (u8)(v-6);
    *(u16 *)((u8 *)out+50)= (diff <=1) ? 1 : 0;
}
void Course_SurfaceLoad(void *a, int b, void *c) {
    // _08006574 (ROM 0x08006574): `adds r3,r1,#0 / adds r0,r2,#0 / movs r1,#1 /
    // adds r2,r3,#0 / bl 0x08006590`. So it reads r1 (arg2) and r2 (arg3) and
    // ignores r0: `_08006590(c, 1, b)`, then LZ77UnCompWram(src, 0x02000000).
    // The caller at _08006138:0x619E passes r0=out, r1=cid, r2=base, so arg2 is
    // the course id (an int) and arg3 is the base pointer. The previous
    // signature declared arg2 as `void *`, which is why the orchestrator's
    // `(out, cid, base)` call failed to type-check. Arg1 is unused.
    (void)a;
    void *src = _08006590(c, 1, b);
    _0802D988(src, (void *)0x02000000);
}
// _08006574 ends mid-word at 0x0800658E; the ROM pads the gap up to the next
// function with 00 00, so force a zero-fill alignment instead of the nop agbcc
// would otherwise emit (same one-liner used before Course_Math_AbsRoundAvg in
// src/course_records.c).
__asm__(".align 2, 0");
void *Course_SeekRes(void *base,int grp,int idx) {
    return _08006590(base,grp,idx);
}
int Course_AttrLookup(int x,int y,int v) {
    // _080065D4: adj = x*65536 + 0xFFF30000; if adj<0 return 10; else table row*3 +k
    s32 adj = (x<<16) + (s32)0xFFF30000;
    if (adj < 0) return 10;
    // Table at 0x080C8FE4, s16 row = hi(adj)
    s16 row = (s16)(adj>>16);
    int k = 2;
    if ((u16)x==0) {
        int s = (-(s32)v) | (s32)v;
        k = (s>>31) & 1;
    }
    const u8 *tbl = (const u8 *)0x080C8FE4;
    s32 idx = row*3 + k;
    return tbl[idx];
}

void Course_ThemeGfx(void *out,int idx,void *base) {
    // _08006468
    (void)out;(void)idx;(void)base;
    // Stub: would seek groups 2/3/4 and DMA/LZ77 to VRAM/PAL
}
void Course_VariantGfx(void *out,int idx,void *base){
    (void)out;(void)idx;(void)base;
}
void Course_BigGfx(void *out,int idx,void *base){
    (void)out;(void)idx;(void)base;
}
void Course_Orchestrator(void *out,int course,void *base){
    // _08006138: 128 bytes = 0x08006138..0x080061B3 + pool 0x0203F760.
    // Every one of the five tail calls reloads the SAME (out, cid, base)
    // triple, so all five take three args in that order.
    // The work-area base must stay an unresolved SYMBOL_REF: agbcc folds a
    // literal 0x03001780 + 0x10C1 into one constant (`.word 0x03002841`),
    // but the ROM keeps the pair as two pool words plus `adds r0,r0,r2`, so
    // the offset needs its own absolute anchor too. Declared INSIDE the body
    // because the slice link splices only this section, so a file-scope one
    // never reaches code.o (same reason as RS_WA in src/race_scene.c).
#ifndef __APPLE__
    extern u8 COWa[] __asm__("COWa");
    extern u32 COOff[] __asm__("COOff");
    __asm__(".globl COWa\nCOWa = 0x03001780\n.globl COOff\nCOOff = 0x10C1\n");
#define CO_WA COWa
#define CO_OFF COOff
#else
#define CO_WA ((volatile u8 *)(uintptr_t)0x03001780u)
#define CO_OFF 0x10C1u
#endif
    int cid;
    // agbcc reserves the argument registers r0-r2 for the seven `bl`s and hands
    // the first long-lived pseudo the next free register, so `course` lands in
    // r3 (`add r3,r1,#0` in the prologue) and the COWa+COOff scratch falls into
    // r1. The ROM keeps `course` in r1 and scratches in r2. Pinning the
    // parameter to r1 with a local register variable reproduces both.
#ifndef __APPLE__
    register int cor1 __asm__("r1");
    cor1 = course;
#define CO_CRS cor1
#else
#define CO_CRS course
#endif
    // The ROM's `cmp r0,#0 / beq` leaves the plain `cid = course` path as the
    // fall-through and puts the calendar pair out of line, so this is written
    // as two explicit transfers rather than an if/else (agbcc lays an if/else
    // the other way round and emits `bne`).
    if (*(volatile u8 *)((u32)(uintptr_t)(CO_OFF) + (u32)(CO_WA)) == 0) goto calendar;
    cid = CO_CRS;
    goto tail;
calendar:
    // Each arm reads the course id straight out of r1 and calls immediately,
    // with the sel==2 call inline and the sel!=2 call reached by `bne` across
    // the 0x0203F758 pool word, exactly as the ROM lays it out. The arms stay
    // in separate blocks, and `CO_CRS` is the r1 pin above rather than a fresh
    // copy, so neither adds a prologue move the ROM does not have.
    if (*(volatile u16 *)0x0203F758 != 2) goto cal_default;
    cid = (s16)CO_CALLEE(Course_GetCourseVariant, _08025908)(CO_CRS);
    goto tail;
cal_default:
    cid = (s16)CO_CALLEE(Course_GetCourseDefault, _080258F4)(CO_CRS);
tail:
    CO_CALLEE(Course_ParseHeader, _080064EC)(out, cid, base);
    CO_CALLEE(Course_ThemeGfx, _08006468)(out, cid, base);
    CO_CALLEE(Course_VariantGfx, _080061B8)(out, cid, base);
    CO_CALLEE(Course_BigGfx, _0800628C)(out, cid, base);
    CO_CALLEE(Course_SurfaceLoad, _08006574)(out, cid, base);
    *(volatile void **)0x0203F760 = out;
#undef CO_WA
#undef CO_OFF
#undef CO_CRS
}
void Course_LoadEntry(void) { // _0801A204: push {r4-r6,lr}, sub #28, pools 0x0805FB98/0x05000002/0x03001780/0x030039D0
    // The ROM's stack slots OVERLAP. Twelve halfwords are copied from
    // 0x0805FB98 to sp+0 and again to sp+8, a zero word is staged at sp+16 and
    // CpuSet-filled (ctrl 0x05000002, count 2) into the 8-byte variant cell at
    // sp+20 -- so sp+0..sp+19 is ONE 20-byte block whose tail word doubles as
    // the fill source, not three independent locals. Declaring `u16 tmpl0[6]`,
    // `u16 tmpl1[6]`, `u32 zero`, `u32 variant[2]` instead gave a 36-byte frame
    // and `push {r4,r5,lr}` against the ROM's `push {r4,r5,r6,lr}` + frame 28.
    //   movs r0,#0; str r0,[sp,#16]; add r0,sp,#16; add r5,sp,#20;
    //   ldr r2,=0x05000002; adds r1,r5,#0; bl _0802D974
    // `memset` is a compiler INTRINSIC, not a closure symbol: agbcc lowers a
    // `{0}` aggregate initialiser to `bl memset`, which has no ROM counterpart.
    // Spelling it as the ROM's own scalar zero word is the fix.
    // The work-area base and both offsets must stay unresolved SYMBOL_REF
    // anchors (declared inside the body because the slice link splices only
    // this section): agbcc folds `0x03001780 + 0x1106` into one constant, and
    // the ROM keeps the pair as two pool words plus `adds r0,r4,r1`.
#ifndef __APPLE__
    extern void _08006138(void *out, int idx, void *base); // Course_Orchestrator
    extern u32 LTSrc[] __asm__("LTSrc");
    extern u32 LWa[] __asm__("LWa");
    extern u32 LSel[] __asm__("LSel");
    extern u32 LRaw[] __asm__("LRaw");
    __asm__(".globl LTSrc\nLTSrc = 0x0805FB98\n.globl LWa\nLWa = 0x03001780\n.globl LSel\nLSel = 0x1106\n.globl LRaw\nLRaw = 0x10FE\n");
#define LE_T (void *)LTSrc
#define LE_WA (u32)LWa
#define LE_SEL (u32)LSel
#define LE_RAW (u32)LRaw
#else
#define LE_T ((void *)(uintptr_t)0x0805FB98u)
#define LE_WA 0x03001780u
#define LE_SEL 0x1106u
#define LE_RAW 0x10FEu
#endif
    u32 tmpl[5];    // sp+0..sp+19; tmpl+2 == sp+8 is the second template copy
    u16 variant[4]; // sp+20..sp+27: the cell CpuSet fills, then the two
                    // selectors strh into variant[1] and variant[0]
    extern void _0802E0A4(void*,void*,int); // CpuSet 6
    extern void _0802D974(const void*,void*,u32);
    _0802E0A4(tmpl, LE_T, 6);
    _0802E0A4(tmpl + 2, LE_T, 6);
    tmpl[4] = 0;                              // movs r0,#0; str r0,[sp,#16]
    _0802D974(&tmpl[4], variant, 0x05000002); // 32-bit fill, count 2 -> sp+20
    s16 courseSel = *(s16 *)(LE_WA + LE_SEL);
    variant[1] = ((u16 *)tmpl)[courseSel];   // lsls #1; add r0,sp; ldrh
    s16 timeSel = *(s16 *)(LE_WA + (LE_SEL - 2u)); // subs r1,#2; adds r0,r4,r1
    variant[0] = ((u16 *)(tmpl + 2))[timeSel];
    Course_VariantZero();                              // bl 0x08006114
    CO_CALLEE(Course_VariantCopy, _080060EC)(variant); // bl 0x080060EC, r0 = r5
    s16 rawIdx = *(s16 *)(LE_WA + LE_RAW);
    CO_CALLEE(Course_Orchestrator, _08006138)((void*)0x030039D0, rawIdx, NULL);
    extern void _08006618(void); // StreamInit
    extern void _08008290(void *a);
    _08006618();
    _08008290((void*)0x030039D0); // sprite/OAM setup, pool 0x030039D0
#undef LE_T
#undef LE_WA
#undef LE_SEL
#undef LE_RAW
}
#ifndef __APPLE__
void _0801A204(void) __attribute__((alias("Course_LoadEntry")));
#endif

// Aliases (ARM only)
#ifndef __APPLE__
void _080060EC(void *a) __attribute__((alias("Course_VariantCopy")));
void *_08006100(int a,void *b) __attribute__((alias("Course_Group0Payload12")));
void _08006114(void) __attribute__((alias("Course_VariantZero")));
void _08006138(void *a,int b,void *c) __attribute__((alias("Course_Orchestrator")));
void _08006574(void *a,int b,void *c) __attribute__((alias("Course_SurfaceLoad")));
void *_08006590(void *a,int b,int c) __attribute__((alias("Course_SeekRes")));
int _080065D4(int a,int b,int c) __attribute__((alias("Course_AttrLookup")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void RaceScene_Leaf_A204(void) __attribute__((alias("Course_LoadEntry")));
void _08001A204(void) __attribute__((alias("Course_LoadEntry")));
#endif
