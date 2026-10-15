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
#define CO_CALLEE(friendly, closure) closure
extern void _080064EC(void *out, int idx, void *base); // Course_ParseHeader
extern void _08006468(void *out, int idx, void *base); // Course_ThemeGfx
extern void _080061B8(void *out, int idx, void *base); // Course_VariantGfx
extern void _0800628C(void *out, int idx, void *base); // Course_BigGfx
extern void _08006574(void *a, int b, void *c);        // Course_SurfaceLoad
extern s16 _08025908(int idx);                        // calendar variant
extern s16 _080258F4(int idx);                        // calendar default
extern void _080060EC(void *src);                // Course_VariantCopy

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
// _08006574: the loader uses the incoming course id and resource base, then
// leaves the ROM's two-byte tail word in the function span before _08006590.
void Course_SurfaceLoad(void *a, int b, void *c) {
    void *payload = _08006590(c, 1, b);
    (void)a;
    _0802D988(payload, (void *)0x02000000u);
}
__asm__(".align 2, 0");
void *Course_SeekRes(void *base, int grp, int idx) {
    register u32 groupLimit __asm__("r4") = (u32)grp;
    register u32 indexOrCursor __asm__("r3") = (u32)idx;
    register u32 directory __asm__("r5");
    register u32 groupCount __asm__("r1");
    register u32 group __asm__("r2");
    register u32 recordIndex __asm__("r6");
    register u32 tableAddress __asm__("r0");
    u32 offset;

    if (base == 0)
        base = (void *)0x080CE020u;
    directory = (u32)base;
    groupCount = 0;
    group = 0;
    recordIndex = indexOrCursor;
    recordIndex += 13u;
    if (groupCount < groupLimit) {
        indexOrCursor = directory;
        do {
            groupCount += *(const u32 *)(uintptr_t)(indexOrCursor + 8u);
            indexOrCursor += 4u;
            group++;
        } while (group < groupLimit);
    }
    groupCount += recordIndex;
    tableAddress = groupCount << 2;
    tableAddress = tableAddress + directory;
    offset = *(const u32 *)(uintptr_t)tableAddress;
    offset = (offset >> 2) << 2;
    return (void *)(uintptr_t)(directory + offset + 8u);
}
int Course_AttrLookup(int x, int y, int v) {
    // The ROM derives the row from x, but tests zero-extended y to select
    // either the special sign-of-v column (0/1) or column 2.
    register u32 selector __asm__("r3") = (u16)y;
    u8 value = (u8)v;
    register s32 adjusted __asm__("r0") =
        (s32)(((u32)x << 16) + 0xFFF30000u);
    register s32 row __asm__("r1") = (u32)adjusted >> 16;
    register s32 sign __asm__("r0");
    register const u8 *table __asm__("r2");
    register s32 selector16 __asm__("r1");
    register u32 index __asm__("r0");

    if (adjusted < 0)
        return 10;
    if (selector == 0) {
        sign = -(s32)value;
        sign |= (s32)value;
        selector = (u32)sign >> 31;
    } else {
        selector = 2;
    }
    table = (const u8 *)(uintptr_t)0x080C8FE4u;
    // Keep the table literal materialized before the two signed index casts.
    __asm__ volatile ("" : : "r" (table));
    row = (s16)row;
    index = (u32)(row * 3);
    selector16 = (s16)selector;
    index = index + selector16;
    index = index + (u32)(uintptr_t)table;
    return *(const u8 *)(uintptr_t)index;
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
    extern u8 COWa[] __asm__("COWa");
    extern u32 COOff[] __asm__("COOff");
    __asm__(".globl COWa\nCOWa = 0x03001780\n.globl COOff\nCOOff = 0x10C1\n");
#define CO_WA COWa
#define CO_OFF COOff
    int cid;
    // agbcc reserves the argument registers r0-r2 for the seven `bl`s and hands
    // the first long-lived pseudo the next free register, so `course` lands in
    // r3 (`add r3,r1,#0` in the prologue) and the COWa+COOff scratch falls into
    // r1. The ROM keeps `course` in r1 and scratches in r2. Pinning the
    // parameter to r1 with a local register variable reproduces both.
    register int cor1 __asm__("r1");
    cor1 = course;
#define CO_CRS cor1
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
    u32 tmpl[5];    // sp+0..sp+19; tmpl+2 == sp+8 is the second template copy
    u16 variant[4]; // sp+20..sp+27: the cell CpuSet fills, then the two
                    // selectors strh into variant[1] and variant[0]
    // Four pins, each a register the ROM holds live across a region agbcc
    // would otherwise re-derive or re-allocate:
    //   t2  = sp+8   (r6; the reason the prologue saves r6 at all, and the
    //                 pointer the ROM advances in place: `adds r6,r6,r0`)
    //   vr  = sp+20  (r5; CpuSet destination and both strh targets)
    //   wbase = 0x03001780 (r4; one pool word reused for the two 0x1104/0x1106
    //                 selector reads AND the +0x10FE event read, where the ROM
    //                 destroys it: `adds r4,r4,r1`)
    //   selv  = 0x1106 (r1; the ROM edits it in place -- `subs r1,#2` -- rather
    //                 than forming a second constant)
    // Unpinned, agbcc recomputes `add r0,sp,#8` inline (losing r6), parks the
    // cell in r4, loads LSel before the base (swapping both pool words) and
    // folds the second selector offset into `subs r0,#2`. All four selectors
    // stay INLINE in their index expressions as well: as separate `s16`
    // locals agbcc forms both 0x03002884/86 addresses before either read.
    register u16 *t2 __asm__("r6");
    register u16 *vr __asm__("r5");
    register u32 wbase __asm__("r4");
    register u32 selv __asm__("r1");
    extern void _0802E0A4(void*,void*,int); // CpuSet 6
    extern void _0802D974(const void*,void*,u32);
    _0802E0A4(tmpl, LE_T, 6);
    t2 = (u16 *)(uintptr_t)(tmpl + 2);
    _0802E0A4(t2, LE_T, 6);
    tmpl[4] = 0;                              // movs r0,#0; str r0,[sp,#16]
    {   // `add r0,sp,#16` precedes `add r5,sp,#20` in the ROM, so the fill
        // source's address must be its own named value; an inline &tmpl[4]
        // lets agbcc schedule the cell pointer first.
        u32 *zsrc = &tmpl[4];
        vr = variant;
        _0802D974(zsrc, vr, 0x05000002);      // 32-bit fill, count 2 -> sp+20
    }
    wbase = LE_WA;
    selv  = LE_SEL;
    {   // Both selector addresses are formed in r0 (`adds r0,r4,r1`) and each
        // ldrsh overwrites the address register; the zero index lands in r2 in
        // both reads.
        //
        // Neither shape is pinnable: `register u32 saddr __asm__("r0")` emits
        // IDENTICAL code (verified by direct experiment), because local-alloc
        // never consults such a pin -- it walks the instruction's own
        // suggestion set first and only then the lowest free register. For the
        // second read, where `selv` holds its last value, that walk lands on r1:
        // the add's suggestion set is {wbase=r4, selv=r1}, r4 is live and r1
        // dies in the instruction, so r1 wins and the load reads [r1,r2] --
        // 2 bytes off the ROM's [r0,r2]. (The zero index is decided the same
        // way, and r1 being free there is also why it is materialised in r1.)
        //
        // The fix is to keep r1 live across the second address computation with
        // a zero-instruction asm use of `selv`: the suggestion pass then fails
        // and the fallback picks r0 for the address, and the zero is pushed to
        // r2 in both reads. Same lever as the `x-y` guards elsewhere, but this
        // one is placed after the load so it changes no instruction. Full
        // write-up: docs/findings/local_alloc_register_choice.md.
        u32 saddr;
        u32 zi;
        saddr = wbase + selv;
        zi = 0;
        vr[1] = ((u16 *)tmpl)[*(s16 *)(uintptr_t)(saddr + zi)]; // lsls #1; add r0,sp; ldrh
        selv  = selv - 2u;                        // subs r1,#2
        saddr = wbase + selv;
        zi = 0;
        t2 = t2 + *(s16 *)(uintptr_t)(saddr + zi);
        __asm__("" : : "r"(selv));                // keeps r1 live to here
    }
    vr[0] = *t2;                              // ldrh [r6]; strh [r5]
    Course_VariantZero();                              // bl 0x08006114
    CO_CALLEE(Course_VariantCopy, _080060EC)(vr);      // bl 0x080060EC, r0 = r5
    {
        void *orc = (void *)0x030039D0;       // ldr r0,=0x030039D0
        s16 rawIdx;
        selv = LE_RAW;                        // ldr r1,=0x10FE
        wbase = wbase + selv;                 // adds r4,r4,r1
        rawIdx = *(s16 *)(uintptr_t)wbase;
        CO_CALLEE(Course_Orchestrator, _08006138)(orc, rawIdx, NULL);
    }
    extern void _08006618(void); // StreamInit
    // The ROM sets up NO argument for this call: 0x08008290 overwrites r0
    // from its own pool before any use (`ldr r4,[pc,#88]` / `ldr r2,[pc,#92]`
    // / `adds r0,r4,#0` at 0x08008294-98), so it reads no incoming r0 and
    // takes no parameter -- its C body is `(void)rec` for the same reason.
    // Passing the 0x030039D0 constant anyway costs the `adds r0,r4,#0` that
    // the ROM does not have.
    extern void _08008290(void);
    _08006618();
    _08008290(); // sprite/OAM setup
#undef LE_T
#undef LE_WA
#undef LE_SEL
#undef LE_RAW
}
void _0801A204(void) __attribute__((alias("Course_LoadEntry")));

// Aliases (ARM only)
void _080060EC(void *a) __attribute__((alias("Course_VariantCopy")));
void *_08006100(int a,void *b) __attribute__((alias("Course_Group0Payload12")));
void _08006114(void) __attribute__((alias("Course_VariantZero")));
void _08006138(void *a,int b,void *c) __attribute__((alias("Course_Orchestrator")));
void _08006574(void *a,int b,void *c) __attribute__((alias("Course_SurfaceLoad")));
void *_08006590(void *a,int b,int c) __attribute__((alias("Course_SeekRes")));
int _080065D4(int a,int b,int c) __attribute__((alias("Course_AttrLookup")));

// ROM entry alias.
void RaceScene_Leaf_A204(void) __attribute__((alias("Course_LoadEntry")));
void _08001A204(void) __attribute__((alias("Course_LoadEntry")));
