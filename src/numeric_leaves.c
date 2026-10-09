#include "gtadv/car_physics_lane.h"
#include "gtadv/ghost.h"
#include "gba/types.h"
#include "gba/bios.h"

// Lane: numeric/table leaves — behavioral equivalence of code_24f*, code_258a8, code_26f*, 2730/279c
// Sources: asm/code_24f34.s, code_24f4c.s, code_24f74.s, code_24f9c.s, code_24fc4.s,
//          asm/code_258a8.s, asm/code_26f8.s, asm/code_2730.s, asm/code_279c.s,
//          asm/code_26f30.s, asm/code_2694.s
// All addresses verified against objdump literal pools; pools kept as absolute constants.

// ---- 24F34 : 10-byte clear (raw span 0x024F34-0x024F4C, no named symbol) ----
 // Substantiated: 10x strb r1 where r1=0 (asm/code_24f34.s:7-16).
void Code24F34_Clear10(void *dst) {
    volatile u8 *p = (volatile u8 *)dst;
    for (int i = 0; i < 10; i++) p[i] = 0;
}
// No alias – raw inline sequence has no BL target.

// ---- 24F4C family : ROM table 0x080CCACC stride helpers ----
// Base table at 0x080CCACC, entry size derived from asm multiplies.
// 24F4C offset 0, 24F74 offset 2, 24F9C offset 4 — same base formula:
//   addr = base + r2*8 + r1*88 + r0*352  (352 = 11*32; 88 = 11*8)
// Then load s16 at [addr + off]
//
// Two source shapes, both measured against the ROM:
//
// 1. The base is kept in its own named local and added LAST, through a named
//    `addr`. Folding it into the sum (`(u32)base +...`) makes agbcc
//    materialise the pool word into r3 and add it immediately --
//    `ldr r3,[pc,#28]; adds r2,r2,r3` at the top -- and the `push {r4,lr}`
//    frame never appears, because nothing is live across the arithmetic.
// 2. The load is `((const s16 *)p)[idx]`, NOT `*(const volatile s16 *)p`. In
//    this agbcc a `volatile s16` read expands to `ldrh` plus an `lsls/asrs`
//    pair, which is 2 bytes longer and is not what the ROM does; the plain
//    `s16` read is the single `ldrsh` the ROM has. The table is in ROM, so
//    dropping `volatile` changes no behaviour. The array-index form is also
//    what produces the ROM's `movs r1,#idx; ldrsh r0,[r2,r1]` -- a
//    byte-offset cast folds the index away and emits `ldrsh r0,[r2,#imm]`.
//
// MEASURED, and still NOT promotable: 4/40, 4/40, 4/40 and 2/36 (was
// 20/40 / 19/36 / 19/36 / 13/36). Everything above the final add now matches --
// the `push {r4,lr}` frame, the stride arithmetic in order, the
// `movs r1,#idx; ldrsh` tail, the `pop {r4}; pop {r1}; bx r1` epilogue and the
// pool word. What is left is ONE thing: the ROM holds the base in **r4**,
// loaded by `ldr r4,[pc,#32]` as the second instruction, and adds it with
// `adds r2,r2,r4` after the last stride multiply. agbcc cannot be made to do
// that here. Measured, not assumed:
//   * no `register... __asm__("r4")` local survives the constant-load sink;
//     the pin is honoured only when the variable is live across a *volatile
//     access*, which costs a load the ROM does not have;
//   * folding the base into the sum first gives the load at the top but in r3
//     and no frame at all, because nothing is live across the arithmetic;
//   * keeping it separate gives the late `adds` but sinks the pool load to its
//     use, and at that point r0 is free, so the allocator takes r0.
// So the byte score DROPPED while the body got structurally closer: the score
// is misleading here because the pool word moves with the load. Recorded rather
// than reverted, so the next attempt does not re-run these twenty variants.
static inline s16 table_24f_lookup(int r0, int r1, int r2, int idx) {
    register const u8 *base __asm__("r4");
    base = (const u8 *)0x080CCACC;
    __asm__("" : "+r"(base));
    r2 <<= 3;
    r2 += r1 * 88;
    r2 += r0 * 352;
    r2 += (u32)base;
    return ((const s16 *)r2)[idx];
}
s16 Code24F4C_TableLookup(int a0,int a1,int a2){ return table_24f_lookup(a0,a1,a2,0); }
s16 Code24F74_TableLookup(int a0,int a1,int a2){ return table_24f_lookup(a0,a1,a2,1); }
s16 Code24F9C_TableLookup(int a0,int a1,int a2){ return table_24f_lookup(a0,a1,a2,2); }
#ifndef __APPLE__
s16 _080024F4C(int a0,int a1,int a2) __attribute__((alias("Code24F4C_TableLookup")));
s16 sub_080024F4C(int a0,int a1,int a2) __attribute__((alias("Code24F4C_TableLookup")));
s16 _08024F4C(int a0,int a1,int a2) __attribute__((alias("Code24F4C_TableLookup")));
s16 sub_08024F4C(int a0,int a1,int a2) __attribute__((alias("Code24F4C_TableLookup")));

s16 _080024F74(int a0,int a1,int a2) __attribute__((alias("Code24F74_TableLookup")));
s16 sub_080024F74(int a0,int a1,int a2) __attribute__((alias("Code24F74_TableLookup")));
s16 _08024F74(int a0,int a1,int a2) __attribute__((alias("Code24F74_TableLookup")));
s16 sub_08024F74(int a0,int a1,int a2) __attribute__((alias("Code24F74_TableLookup")));

s16 _080024F9C(int a0,int a1,int a2) __attribute__((alias("Code24F9C_TableLookup")));
s16 sub_080024F9C(int a0,int a1,int a2) __attribute__((alias("Code24F9C_TableLookup")));
s16 _08024F9C(int a0,int a1,int a2) __attribute__((alias("Code24F9C_TableLookup")));
s16 sub_08024F9C(int a0,int a1,int a2) __attribute__((alias("Code24F9C_TableLookup")));
#endif

// ---- 24FC4 : base 0x080CCD8C, strides 4/44/176 ----
// asm: r2*4 + r1*44 + r0*176 + base; ldrsh [r2]
// Same two source shapes as the 24F4C family above, and for the same reasons:
// the base is added LAST through a named `addr` (so r4 stays live across the
// body and the `push {r4,lr}` frame appears), and the load is an `s16` array
// index rather than a `volatile s16` dereference.
s16 Code24FC4_TableLookup(int a0, int a1, int a2) {
    register const u8 *base __asm__("r4");
    base = (const u8 *)0x080CCD8C;
    __asm__("" : "+r"(base));
    a2 <<= 2;
    a2 += a1 * 44;
    a2 += a0 * 176;
    a2 += (u32)base;
    return *(const s16 *)a2;
}
#ifndef __APPLE__
s16 _080024FC4(int a0, int a1, int a2) __attribute__((alias("Code24FC4_TableLookup")));
s16 sub_080024FC4(int a0, int a1, int a2) __attribute__((alias("Code24FC4_TableLookup")));
s16 _08024FC4(int a0, int a1, int a2) __attribute__((alias("Code24FC4_TableLookup")));
s16 sub_08024FC4(int a0, int a1, int a2) __attribute__((alias("Code24FC4_TableLookup")));
#endif

// ---- 258A8 : simple s16 table at 0x080CD7D0 ----
// asm: lsls r0,#1; adds r0,r1 where r1=base; ldrsh
s16 Code258A8_SimpleLookup(int idx){
    const s16 *t = (const s16 *)0x080CD7D0;
    return t[idx] + t[0] - t[0];
}
#ifndef __APPLE__
s16 _0800258A8(int idx) __attribute__((alias("Code258A8_SimpleLookup")));
s16 _080258A8(int idx) __attribute__((alias("Code258A8_SimpleLookup")));
s16 sub_080258A8(int idx) __attribute__((alias("Code258A8_SimpleLookup")));
#endif

// ---- 26F8 : IWRAM s16 at 0x03001780+108C/109C helpers ----
// Reader at ROM 0x080026F8 (file offset 0x0026F8), 28 bytes including pools.
// The in-section absolute base and named displacement retain the ROM's two
// literal loads and base-plus-displacement addition before the grid offset.
// IWRAM work-area reads are ordinary s16 data, matching the ROM's ldrsh.
s16 Code26F8_GetIWRAM_S16(int r0,int r1){
#ifndef __APPLE__
    extern u8 GridRead_26F8_WA[];
    __asm__(".globl GridRead_26F8_WA\nGridRead_26F8_WA = 0x03001780\n");
    u32 base = (u32)(uintptr_t)GridRead_26F8_WA;
#else
    u32 base = 0x03001780u;
#endif
    u32 off = ((u32)r1 << 1) + ((u32)r0 << 3);
    u32 idx = 0;
    u32 disp = 0x108Cu;
    u32 addr = (u32)off + (base + disp);
    return ((s16 *)(addr + idx))[idx];
}
void Code26F8_SetIWRAM_S16(int r0,int r1,int r2){
    volatile s16 *p = (volatile s16 *)(0x03001780u + 0x109Cu + (r1*2) + (r0*8));
    *p = (s16)r2;
}
#ifndef __APPLE__
s16 _080026F8(int r0,int r1) __attribute__((alias("Code26F8_GetIWRAM_S16")));
s16 sub_080026F8(int r0,int r1) __attribute__((alias("Code26F8_GetIWRAM_S16")));
#endif


// 27xx : flags at 0x03001780+0x1056 / 0x105C
// _0800279C is EXACT 80/80: keep each branch's WA+0x105C calculation
// separate so agbcc retains the ROM's three duplicate literal pairs.
void Code279C_SetFlag0(int v){
#ifndef __APPLE__
    register u32 value4 __asm__("r4") = (u32)v;
    register uintptr_t cell __asm__("r0");
    register u32 offsetOrValue __asm__("r1");
    extern int _08002780(void);
    int gate = _08002780();
    if (gate == 0) {
        if (value4 == 0u) {
            cell = 0x03001780u;
            __asm__("" : "+r" (cell));
            offsetOrValue = 0x105Cu;
            __asm__("" : "+r" (offsetOrValue));
            cell += offsetOrValue;
            __asm__("" : "+r" (cell));
            *(volatile u16 *)cell = (u16)value4;
            return;
        }
        cell = 0x03001780u;
        __asm__("" : "+r" (cell));
        offsetOrValue = 0x105Cu;
        __asm__("" : "+r" (offsetOrValue));
        cell += offsetOrValue;
        __asm__("" : "+r" (cell));
        offsetOrValue = 1;
    } else {
        cell = 0x03001780u;
        __asm__("" : "+r" (cell));
        offsetOrValue = 0x105Cu;
        __asm__("" : "+r" (offsetOrValue));
        cell += offsetOrValue;
        __asm__("" : "+r" (cell));
        offsetOrValue = 0;
    }
    *(volatile u16 *)cell = (u16)offsetOrValue;
#else
    extern int _08002780(void);
    int gate = _08002780();
    volatile u16 *flag = (volatile u16 *)(0x03001780u + 0x105Cu);
    if (gate==0){
        if (v==0) *flag = 0;
        else {
            volatile u16 *p = (volatile u16 *)(0x03001780u + 0x105Cu);
            *p = 1;
            return;
        }
    } else {
        volatile u16 *p = (volatile u16 *)(0x03001780u + 0x105Cu);
        *p = 0;
    }
#endif
}
int Code279C_IsNonZero_105C(void){
    volatile s16 v = *(volatile s16 *)(0x03001780u + 0x105Cu);
    int t = -v | v;
    return (t >> 31) & 1; // asm: negs/orrs/lsrs #31 => 0 if zero else 1
}
int Code279C_Get_4770_Entry(int idx){
    // 0x080C4770 table, idx*8? asm: lsls #3? Actually _08002800: idx<<3? inspect 279c tail
    // Leaf at 0x08002818: lsls r0 #16, asrs #13 ( *4), adds base 0x080C4770, ldrsh [r0]
    return *(const volatile s16 *)((uintptr_t)0x080C4770 + (u32)(idx * 8));
}
#ifndef __APPLE__
void _0800279C(int v) __attribute__((alias("Code279C_SetFlag0")));
void Event_Post1(int v) __attribute__((alias("Code279C_SetFlag0")));
#endif

// ---- 26F30 : alloc helper (calls sub_0800572C and sub_08026F50) ----
extern void *sub_0800572C(int size);
extern void sub_08026F50(void *ptr, int a0, int a1);

void *Code26F30_Alloc(int a0, int a1) {
    void *ptr = sub_0800572C(64);
    sub_08026F50(ptr, a0, a1);
    return ptr;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void *_080026F30(int a0, int a1) __attribute__((alias("Code26F30_Alloc")));
void *sub_080026F30(int a0, int a1) __attribute__((alias("Code26F30_Alloc")));
void *_08026F30(int a0, int a1) __attribute__((alias("Code26F30_Alloc")));
void *sub_08026F30(int a0, int a1) __attribute__((alias("Code26F30_Alloc")));
#endif

void Code2694_InitWork(void){
    extern void _08024568(void);
    extern volatile u8 WorkArea_01780[];
    u32 zero = 0;
    register u8 *wa __asm__("r4");
    __asm__(".globl WorkArea_01780\nWorkArea_01780 = 0x03001780");
    wa = (u8 *)WorkArea_01780;
    CpuSet(&zero, (void *)wa, 0x0500078Du);
    wa += 0x5E0;
    *(volatile u16*)wa = 1;
    Ghost_BindSampleBuffers((void*)0x0203D600u, (void*)0x0203CC00u, 2560);
    _08024568();
}
#ifndef __APPLE__
void _08002694(void) __attribute__((alias("Code2694_InitWork")));
void ClearWorkArea(void) __attribute__((alias("Code2694_InitWork"))); /* trampoline elimination: friendly-name spelling used by foundation_agbmain.c */
void sub_0800279C(int v) __attribute__((alias("Code279C_SetFlag0")));
#endif

// ROM entry alias.
#ifndef __APPLE__
s16 Sub_080258A8(int idx) __attribute__((alias("Code258A8_SimpleLookup")));
#endif

// ----------------------------------------------------------------------------
// Round-six transcription ((consolidated/elsewhere) family; WA base 0x03001780).
// All four are small pool leaves over the same grid cell region the
// Code26F8/2730 helpers use.
//
// ROM 0x080026DC (file offset 0x0026DC), 28-byte store: u16[WA + 0x108C + a*8 + b*2] = c.
// ROM 0x08002714 (file offset 0x002714), 28-byte store: u16[WA + 0x109C + a*8 + b*2] = c (identical body,
//   different pool base; save.c's rebuild loop clears both grids).
void GridStore_26DC(int a, int b, int c) {
#ifndef __APPLE__
    extern u8 GridStore_26DC_WA[];
    __asm__(".globl GridStore_26DC_WA\nGridStore_26DC_WA = 0x03001780\n");
    u32 base = (u32)(uintptr_t)GridStore_26DC_WA;
#else
    u32 base = 0x03001780u;
#endif
    u32 off = ((u32)b << 1) + ((u32)a << 3);
    u32 disp = 0x108Cu;
    u32 addr = off + (base + disp);
    *(volatile u16 *)addr = (u16)c;
}
void GridStore_2714(int a, int b, int c) {
#ifndef __APPLE__
    extern u8 GridStore_2714_WA[];
    __asm__(".globl GridStore_2714_WA\nGridStore_2714_WA = 0x03001780\n");
    u32 base = (u32)(uintptr_t)GridStore_2714_WA;
#else
    u32 base = 0x03001780u;
#endif
    u32 off = ((u32)b << 1) + ((u32)a << 3);
    u32 disp = 0x109Cu;
    u32 addr = off + (base + disp);
    *(volatile u16 *)addr = (u16)c;
}
#ifndef __APPLE__
void _080026DC(int a, int b, int c) __attribute__((alias("GridStore_26DC")));
void sub_080026DC(int a, int b, int c) __attribute__((alias("GridStore_26DC")));
void Sub_080026DC(int a, int b, int c) __attribute__((alias("GridStore_26DC")));
void _08002714(int a, int b, int c) __attribute__((alias("GridStore_2714")));
void sub_08002714(int a, int b, int c) __attribute__((alias("GridStore_2714")));
void Sub_08002714(int a, int b, int c) __attribute__((alias("GridStore_2714")));
#endif

// ROM 0x08002780 (file offset 0x002780), 28 bytes: s16 v = s16[WA + 0x1056]; return ((-v) | v) >> 31 —
// 0 if the halfword is zero, else 1. (0x0800279C gates on it.)
int GridFlagTest_2780(void) {
#ifndef __APPLE__
    extern u8 GridFlagTest_2780_WA[];
    __asm__(".globl GridFlagTest_2780_WA\nGridFlagTest_2780_WA = 0x03001780\n");
    u32 base = (u32)(uintptr_t)GridFlagTest_2780_WA;
#else
    u32 base = 0x03001780u;
#endif
    s16 v = *(s16 *)(base + 0x1056u);
    return (int)(((u32)(-v) | (u32)v) >> 31);
}
#ifndef __APPLE__
int _08002780(void) __attribute__((alias("GridFlagTest_2780")));
int sub_08002780(void) __attribute__((alias("GridFlagTest_2780")));
#endif

// ROM 0x080027EC (file offset 0x0027EC), 28 bytes: same test over
// s16[WA + 0x105C]. Each flag reader defines its own absolute base inside
// its function section so the symbol survives independent-slice substitution.
int GridFlagTest_27EC(void) {
#ifndef __APPLE__
    extern u8 GridFlagTest_27EC_WA[];
    __asm__(".globl GridFlagTest_27EC_WA\nGridFlagTest_27EC_WA = 0x03001780\n");
    u32 base = (u32)(uintptr_t)GridFlagTest_27EC_WA;
#else
    u32 base = 0x03001780u;
#endif
    s16 v = *(s16 *)(base + 0x105Cu);
    return (int)(((u32)(-v) | (u32)v) >> 31);
}
#ifndef __APPLE__
int _080027EC(void) __attribute__((alias("GridFlagTest_27EC")));
int sub_080027EC(void) __attribute__((alias("GridFlagTest_27EC")));
#endif
