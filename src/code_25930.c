// ============================================================================
// code_25930.c — C lift of asm/code_25930.s (VMA 0x08025930–0x08025CF4, 5 funcs)
//
// Car-collection grid builder + template copiers + grid-clear leaf.
// Transcribed instruction-for-instruction from the cited asm listing.
//
// Contents:
//   sub_080025930 (head at 0x080025930) — 32-row car-collection grid builder:
//     scans the 4x11 zone grid via _08025CF4 (types 3..10, rows 0..3),
//     matches u16[0x080600CC + row*2] ids, stores found car ids into the
//     0x0203F9B0 word array, then appends a tail block via _08025D64 /
//     _08025D90 and writes the count to 0x0203FA40.
//   0x080025A84 / 0x080025A8C / 0x080025A9C — small accessors over
//     0x0203FA40 / 0x0203F9B0 (word-array readers).
//   sub_080025AC0 — 2-entry presence scan via _08026004 over 0x0806010C
//     (8-B stride), building the 0x0203FA30 array; count -> 0x0203FA3C.
//   sub_080025B3C — memcpy-template 31x72 copier (0x08060370 template ->
//     0x03001780+0x5E4 grid, 0x0802E0A4 copies, 0xFFFFF700 stack pad).
//   sub_080025B88 — 2x72 template copier (0x08060C70 template ->
//     0x03001780+0xEE4).
//   sub_080025BC8 — car-record allocator: 84 B via _0800572C then
//     [out+4] = s16[0x080CD830 + id*12].
//   0x080025BF4-region leaf — [out+4] = s16[0x080CD830 + id*12] (no alloc).
//   sub_080025C0C — 182-B record allocator + s16 id store.
//   sub_080025C34 — 182-B record allocator + 20-B body copy.
//   sub_080025C64 — body copy without allocation (24-B header, 20-B body).
//   sub_080025C84 — bit-clear leaf over 0x03001780 (masks table 0x08060D48;
//     lane = carId*44 + row*11 + col, 4-B bitfield, DivRem by 4).
// ============================================================================

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) int  _08025CF4(int a, int b, int c) { (void)a; (void)b; (void)c; return 0; }
__attribute__((weak)) int  _08025D64(int a, int b) { (void)a; return 0; }
__attribute__((weak)) int  _08025D90(int a) { (void)a; return 0; }
__attribute__((weak)) int  _08026004(int a) { (void)a; return 0; }
__attribute__((weak)) int  _0800572C(int n) { (void)n; return 0; }
__attribute__((weak)) int  sub_0800572C(int n) { (void)n; return 0; }
__attribute__((weak)) void _0802E0A4(void *dst, const void *src, u32 n) { (void)dst; (void)src; (void)n; }
__attribute__((weak)) int  _0802D97C(int a, int b) { (void)b; return a; }
#else
extern int  _08025CF4(int a, int b, int c);   // 0x08025CF4 zone-grid get (ai_grid_leaves.c)
extern int  _08025D64(int a, int b);          // 0x08025D64 column count (ai_grid_leaves.c)
extern int  _08025D90(int a);                 // 0x08025D90 rows-full test (ai_grid_leaves.c)
extern int  _08026004(int a);                 // 0x08026004 owned flag (ai_award_leaves.c)
extern int  _0800572C(int n);                 // 0x0800572C save-heap alloc (save.c)
extern int  sub_0800572C(int n);              // 0x0800572C, closure spelling
// ROM ABI (asm/runtime_mem.s): r0 = dst, r1 = src, r2 = count; returns dst.
// The earlier (src, dst) declaration reversed every call in this file.
extern void _0802E0A4(void *dst, const void *src, u32 n); // 0x0802E0A4 runtime memcpy (foundation_runtime.c)
extern int  _0802D97C(int a, int b);          // 0x0802D97C DivRem (bios_wrappers.c)
#endif

#define GTADV_GRID_BASE ((volatile u8 *)(uintptr_t)0x03001780u)
extern volatile u8 C25GridBase[];

// ROM/IWRAM anchors (byte-exact pool words in the listing):
#define C25_GRID_OUT    ((volatile u32 *)(uintptr_t)0x0203F9B0)  // _0800259B0/_080025A1C/_080025A78/_080025A98/_080025AB0
#define C25_COUNT_OUT   ((volatile u32 *)(uintptr_t)0x0203FA40)  // _080025A74/_080025A88
#define C25_ZONES       0x0805FCACu                              // _0800259B4/_080025A7C/_080025A20
#define C25_ZONE_IDS    0x080600CCu                              // _0800259B8
#define C25_SPECIAL     ((volatile u32 *)(uintptr_t)0x0203FA30)  // _080025AF4/_080025B14/_080025B2C
#define C25_SPECIAL_CNT ((volatile u32 *)(uintptr_t)0x0203FA3C)  // _080025AF8/_080025B04
#define C25_SPECIAL_TBL 0x0806010Cu                              // _080025AF0
#define C25_CAR_BASE    ((volatile u8 *)(uintptr_t)0x03001780)
#define C25_GRID_OFF    0x5E4
#define C25_EE4_OFF     0xEE4
#define C25_TPL_370     0x08060370u
#define C25_TPL_C70     0x08060C70u
#define C25_REC_TBL     0x080CD830u
#define C25_MASKS       0x08060D48u

extern int C25_GridOut[];
extern const u8 C25_Zones[];
extern const u16 C25_ZoneIds[];
extern int C25_CountOut[];
#ifndef __APPLE__
__asm__(".globl C25_GridOut\nC25_GridOut = 0x0203F9B0\n");
__asm__(".globl C25_Zones\nC25_Zones = 0x0805FCAC\n");
__asm__(".globl C25_ZoneIds\nC25_ZoneIds = 0x080600CC\n");
__asm__(".globl C25_CountOut\nC25_CountOut = 0x0203FA40\n");
#else
static int C25_GridOut[32];
static const u8 C25_Zones[1024];
static const u16 C25_ZoneIds[32];
static int C25_CountOut[1];
#endif

// ----------------------------------------------------------------------------
// Head at 0x080025930 (no separate.type in the listing; the function begins
// right after the 0x080258xx cluster tail). 32-row scan.
void Code25930_BuildGrid(void) {
    int count = 0;
    register int latch __asm__("sl") = 0;
    {
        int base = (int)C25_GridOut;
        int zero = 0;
        int p = base + 124;

        do {
            *(int *)p = zero;
            p -= 4;
        } while (p >= base);
    }

    {
        register int row __asm__("r8") = 0;
        do {
            int zone_row = 0;
            register int next_row __asm__("r9");
            int zone_off;
            {
                register int tmp __asm__("r2") = 1;
                tmp += row;
                next_row = tmp;
            }
            zone_off = 0;
            for (zone_row = 0; zone_row <= 3; zone_off += 264, zone_row++) {
                register int zone_type __asm__("r5") = 3;
                register const u16 *zptr __asm__("r4");
                {
                    register const u8 *r4_base __asm__("r4") = C25_Zones;
                    register int r0 __asm__("r0") = zone_off + (int)r4_base;
                    zptr = (const u16 *)(r0 + 72);
                }
                do {
                    if (_08025CF4(0, zone_row, zone_type) > 0) {
                        if (_08025CF4(0, zone_row, zone_type) <= 3) {
                            register const u16 *zone_ids __asm__("r1") = C25_ZoneIds;
                            const s16 *idptr = (const s16 *)((row * 2) + (uintptr_t)zone_ids);
                            if (*zptr == *(const u16 *)idptr) {
                                register const int *r0_out __asm__("r0") = C25_GridOut;
                                register int r1_slot __asm__("r1") = count << 2;
                                r1_slot += (int)r0_out;
                                *(int *)r1_slot = *idptr;
                                count++;
                                latch = 1;
                                goto latch_check;
                            }
                        }
                    }
                    zptr = (const u16 *)((const u8 *)zptr + 24);
                    zone_type++;
                } while (zone_type <= 10);
            latch_check:
                if (latch == 1) {
                    latch = 0;
                    break;
                }
            }
            row = next_row;
        } while (row <= 31);
    }

    {
        register int r6 __asm__("r6") = 0;
        register const int *r1_out __asm__("r1") = C25_GridOut;
        register const u8 *r9 __asm__("r9") = C25_Zones;
        register const s16 *r8 __asm__("r8") = (const s16 *)(r9 + 72);
        register int r5 __asm__("r5") = 0;
        register int r0 __asm__("r0") = count << 2;
        register int *r4 __asm__("r4") = (int *)(r0 + (uintptr_t)r1_out);
        do {
            int cnt = _08025D64(0, r6);
            if ((unsigned)(cnt - 3) <= 7) {
                *r4++ = *(const s16 *)(((cnt * 24) + r5) + (uintptr_t)r9);
                count++;
            } else if (cnt <= 2) {
                if (_08025D90(0) == r6) {
                    register int val __asm__("r0");
                    __asm__("mov r2, %1\n\tmovs r3, #0\n\tldrsh %0, [r2, r3]" : "=r"(val) : "r"(r8) : "r2", "r3");
                    *r4++ = val;
                    count++;
                }
            }
            __asm__("movs r0, #132\n\tlsl r0, r0, #1\n\tadd %0, r0\n\tadd %1, %1, r0"
                    : "+r"(r8), "+r"(r5) : : "r0");
            r6++;
        } while (r6 <= 3);
    }

    {
        int *cnt = C25_CountOut;
        int n = count - 1;
        *cnt = n;
        if (n <= 0) {
            register int *r1 __asm__("r1") = C25_GridOut;
            register const u8 *r0 __asm__("r0") = C25_Zones;
            r0 += 72;
            *r1 = *(const s16 *)r0;
            *cnt = 0;
        }
    }
}
#ifndef __APPLE__
void _080025930(void) __attribute__((alias("Code25930_BuildGrid")));
void sub_080025930(void) __attribute__((alias("Code25930_BuildGrid")));
#endif

// ----------------------------------------------------------------------------
// 0x08025A80 — *0x0203FA40 as s16 (first word).
int Code25930_GetCount(void) {
    return *(const s16 *)0x0203FA40u;
}
#ifndef __APPLE__
int _080025A80(void) __attribute__((alias("Code25930_GetCount")));
int sub_080025A80(void) __attribute__((alias("Code25930_GetCount")));
int _08025A80(void) __attribute__((alias("Code25930_GetCount")));
int sub_08025A80(void) __attribute__((alias("Code25930_GetCount")));
int _080025A84(void) __attribute__((alias("Code25930_GetCount")));
int sub_080025A84(void) __attribute__((alias("Code25930_GetCount")));
#endif

// ----------------------------------------------------------------------------
// 0x08025A8C — word-array reader: s16[0x0203F9B0 + i*4].
extern const s16 C25_EntryTbl[];
int Code25930_GetEntry(int i) {
#ifndef __APPLE__
    __asm__(".globl C25_EntryTbl\nC25_EntryTbl = 0x0203F9B0\n");
    const s16 *base = C25_EntryTbl;
#else
    const s16 *base = (const s16 *)0x0203F9B0u;
#endif
    return *(const s16 *)((const u8 *)base + ((u32)i << 2));
}
#ifndef __APPLE__
int _080025A8C(int a) __attribute__((alias("Code25930_GetEntry")));
int sub_080025A8C(int a) __attribute__((alias("Code25930_GetEntry")));
int sub_08025A8C(int a) __attribute__((alias("Code25930_GetEntry")));
#endif

// ----------------------------------------------------------------------------
// 0x080025A9C — array index finder: first i where word[i]==val (s16 result).
int Code25930_FindEntry(int val) {
    int i = 0;
    volatile u32 *p = C25_GRID_OUT;
    for (; i <= 31; i++) {
        if ((u32)val == *p) {
            return (s16)i;
        }
        p++;
    }
    return 0;
}
#ifndef __APPLE__
int _080025A9C(int a) __attribute__((alias("Code25930_FindEntry")));
int sub_080025A9C(int a) __attribute__((alias("Code25930_FindEntry")));
#endif

// ----------------------------------------------------------------------------
// sub_080025AC0 — 2-entry presence scan building 0x0203FA30.
void Code25930_BuildSpecial(void) {
    int r6 = 0;
    int r4 = 0;
    const s16 *r5;
    volatile u32 *r7;
    r5 = (const s16 *)(uintptr_t)C25_SPECIAL_TBL;
    r7 = C25_SPECIAL;
    while (r4 <= 2) {
        if (_08026004(r4) == 1) {
            *r7++ = *r5;
            r6++;
        }
        r5 += 4;
        r4++;
    }
    *C25_SPECIAL_CNT = r6 - 1;
}
#ifndef __APPLE__
void _080025AC0(void) __attribute__((alias("Code25930_BuildSpecial")));
void sub_080025AC0(void) __attribute__((alias("Code25930_BuildSpecial")));
#endif

// ----------------------------------------------------------------------------
// 0x080025AF8-region leaves — *0x0203FA3C reader / indexed 0x0203FA30 /
// index finder over 0x0203FA30 (3-entry scan).
int Code25930_GetSpecialCount(void) {
    return *(const s16 *)0x0203FA3Cu;
}
#ifndef __APPLE__
int _080025AFC(void) __attribute__((alias("Code25930_GetSpecialCount")));
int sub_080025AFC(void) __attribute__((alias("Code25930_GetSpecialCount")));
#endif

extern const s16 C25_SpecialTbl[];
int Code25930_GetSpecial(int i) {
#ifndef __APPLE__
    __asm__(".globl C25_SpecialTbl\nC25_SpecialTbl = 0x0203FA30\n");
    const s16 *base = C25_SpecialTbl;
#else
    const s16 *base = (const s16 *)0x0203FA30u;
#endif
    return *(const s16 *)((const u8 *)base + ((u32)i << 2));
}
#ifndef __APPLE__
int _080025B08(int a) __attribute__((alias("Code25930_GetSpecial")));
int sub_080025B08(int a) __attribute__((alias("Code25930_GetSpecial")));
#endif

int Code25930_FindSpecial(int val) {
    int i = 0;
    volatile u32 *p = C25_SPECIAL;
    for (; i <= 2; i++) {
        if ((u32)val == *p) {
            return (s16)i;
        }
        p++;
    }
    return 0;
}
#ifndef __APPLE__
int _080025B18(int a) __attribute__((alias("Code25930_FindSpecial")));
int sub_080025B18(int a) __attribute__((alias("Code25930_FindSpecial")));
#endif

// ----------------------------------------------------------------------------
// sub_080025B3C — template copy: 31 x 72 B from 0x08060370 to
// 0x03001780+0x5E4, plus a 0x900-byte header copy from sp.
void Code25930_CopyTemplate370(void) {
    // add sp, 0xFFFFF700 = sp -= 0x900 : 0x900 bytes of stack scratch
    u8 scratch[0x900];
    extern u8 C25GridBaseB3C[];
    u8 *b;
    u8 *r4;
    u32 off;
    u32 r6;
    int i;
    __asm__(".globl C25GridBaseB3C\nC25GridBaseB3C = 0x03001780\n");
    _0802E0A4((void *)scratch, (const void *)(uintptr_t)C25_TPL_370, 0x900u);
    b = (u8 *)C25GridBaseB3C;
    r4 = scratch;
    off = C25_GRID_OFF;
    r6 = (u32)b + off;
    i = 31;
    for (; i >= 0; i--) {
        _0802E0A4((void *)(uintptr_t)r6, (const void *)r4, 72u);
        r4 += 72;
        r6 += 72;
    }
}
#ifndef __APPLE__
void _080025B3C(void) __attribute__((alias("Code25930_CopyTemplate370")));
void sub_080025B3C(void) __attribute__((alias("Code25930_CopyTemplate370")));
#endif

// ----------------------------------------------------------------------------
// sub_080025B88 — 216-B header + 2 x 72 B template copy from 0x08060C70 to
// 0x03001780+0xEE4.
void Code25930_CopyTemplateC70(void) {
    u8 scratch[216];
    extern u8 C25GridBaseC70[];
    u8 *b;
    u8 *r4;
    u32 off;
    u32 r6;
    int i;
    __asm__(".globl C25GridBaseC70\nC25GridBaseC70 = 0x03001780\n");
    _0802E0A4((void *)scratch, (const void *)(uintptr_t)C25_TPL_C70, 216u);
    b = (u8 *)C25GridBaseC70;
    r4 = scratch;
    off = C25_EE4_OFF;
    r6 = (u32)b + off;
    i = 2;
    for (; i >= 0; i--) {
        _0802E0A4((void *)(uintptr_t)r6, (const void *)r4, 72u);
        r4 += 72;
        r6 += 72;
    }
}
#ifndef __APPLE__
void _080025B88(void) __attribute__((alias("Code25930_CopyTemplateC70")));
void sub_080025B88(void) __attribute__((alias("Code25930_CopyTemplateC70")));
#endif

// ----------------------------------------------------------------------------
// sub_080025BC8 — allocate 84 B, store handle at [out+0], s16 id at [out+4].
extern const u8 C25RecTbl_BC8[];
void Code25930_Alloc84(void *out, int id) {
    volatile u32 *o = (volatile u32 *)out;
#ifndef __APPLE__
    int h = sub_0800572C(84);
#else
    int h = _0800572C(84);
#endif
    const u8 *base;
    u32 off;
    __asm__(".globl C25RecTbl_BC8\nC25RecTbl_BC8 = 0x080CD830\n");
    o[0] = (u32)h;
    base = (const u8 *)C25RecTbl_BC8;
    off = ((u32)id << 2) + (u32)id;
    off <<= 2;
    o[1] = (u32)*(const s16 *)(const void *)(base + off);
}
#ifndef __APPLE__
void _080025BC8(void *a, int b) __attribute__((alias("Code25930_Alloc84")));
void Sub_080025BC8(void *a, int b) __attribute__((alias("Code25930_Alloc84")));
void sub_080025BC8(void *a, int b) __attribute__((alias("Code25930_Alloc84")));
void _08025BC8(void *a, int b) __attribute__((alias("Code25930_Alloc84")));
void Sub_08025BC8(void *a, int b) __attribute__((alias("Code25930_Alloc84")));
void sub_08025BC8(void *a, int b) __attribute__((alias("Code25930_Alloc84")));
#endif

// ----------------------------------------------------------------------------
// 0x080025BF4-region leaf — [out+4] = s16[0x080CD830 + id*20] (no alloc).
extern const u8 C25RecTbl_BF0[];
void Code25930_StoreId(void *out, int id) {
    volatile u32 *o = (volatile u32 *)out;
    const u8 *base;
    u32 off;
    u32 k;
    __asm__(".globl C25RecTbl_BF0\nC25RecTbl_BF0 = 0x080CD830\n");
    base = (const u8 *)C25RecTbl_BF0;
    off = ((u32)id << 2) + (u32)id;
    off <<= 2;
    k = 0;
    o[1] = (u32)((const s16 *)(const void *)(base + off))[k];
}
#ifndef __APPLE__
void _080025BF0(void *a, int b) __attribute__((alias("Code25930_StoreId")));
void sub_080025BF0(void *a, int b) __attribute__((alias("Code25930_StoreId")));
void _080025BF4(void *a, int b) __attribute__((alias("Code25930_StoreId")));
void sub_080025BF4(void *a, int b) __attribute__((alias("Code25930_StoreId")));
void EventPost(void *a, int b) __attribute__((alias("Code25930_StoreId")));
void Event_0x08025BF0(void *a, int b) __attribute__((alias("Code25930_StoreId")));
void Sub_08025BF0(void *a, int b) __attribute__((alias("Code25930_StoreId")));
#endif

// ----------------------------------------------------------------------------
// sub_080025C0C — allocate 182 B, store handle at [out+0], s16 id at [out+4].
void Code25930_Alloc182(void *out, int id) {
    volatile u32 *o = (volatile u32 *)out;
    int h = _0800572C(182);
    o[0] = (u32)h;
    u32 off = ((u32)id << 2) + (u32)id;
    off <<= 2;
    volatile s16 *p = (volatile s16 *)(uintptr_t)(C25_REC_TBL + off);
    o[1] = (u32)*(volatile s16 *)p;
}
#ifndef __APPLE__
void _080025C0C(void *a, int b) __attribute__((alias("Code25930_Alloc182")));
void sub_080025C0C(void *a, int b) __attribute__((alias("Code25930_Alloc182")));
#endif

//   pin the base with `register u32 base __asm__("r1")`   32/40 prefix 14
//   same pin to r2 instead (control on which register)    32/40 prefix 14
//   pin the *result* read to r0, base unpinned            44 B span, 18/40
//   `const s16 *tbl` form, base pinned                     44 B span, 15/40
//   base pointer as its own statement, then `p += off`    32/40 prefix 14
//   `const u8 *tbl` bound first, then a second `q` local  32/40 prefix 14
//   one expression, base leftmost, stride inlined         32/40 prefix 14
//   base as a separate earlier statement AND chain inline 32/40 prefix 14
//   neutral control: same maths as three statements, not two   32/40 prefix 14
//   neutral control: reversed addition operands (`off + TBL`)  32/40 prefix 14
//   `u32 a = C25_REC_TBL;` read into its own variable     32/40 prefix 14
//
// Both neutral controls are the discriminator, and they came out NEGATIVE: a
// semantically neutral input that does not move the score is the doc's
// criterion for "the allocator is pinned here", not for "the miss is
// unreachable" -- see the _08022D74 entry for the contrast, where the same
// test came out positive and a register pin closed the body one instruction
// later. Here no register choice was involved at all: r1 was already the
// register the constant landed in. The decision was WHEN, not WHERE.
//
// The lever is the repo's own documented one, from src/course_cal.c:8-22,
// which fixes the byte-identical failure on the calendar table: the base must
// be a `symbol_ref` leaf, not a folded integer constant. A folded constant is
// rematerialisable, so the reload pass DELETES the insn that defined it and
// re-emits it as late as it can -- here the last insn before the `adds`. A
// `symbol_ref` is a real insn the reload pass cannot delete, so it stays where
// it is defined, and defining it first is what puts it at +0x0E.
//
// The absolute symbol must be declared and defined INSIDE the body: the
// per-body splice extracts only the function's brace-matched text, so a
// file-scope `__asm__` defining it is simply absent from the spliced section.
// C89 also requires every declaration to precede the `__asm__` statement, so
// `base` and `off` are declared (not initialised) at the top of the block. A
// mid-block `const u8 *base =...` next to the shim sends
// tools/agbcc_c89_transform.py into a nest/unwrap loop and the TU is dropped
// with "budget exhausted" -- measured, not assumed.
//
// `C25RecTbl` is a new absolute symbol, not a duplicate of anything: no asm
// label is defined at 0x080CD830, only five pool `.4byte` references to that
// address (asm/code_25930.s:361,372,393,419,440), and an absolute symbol
// assignment places no bytes.
extern const u8 C25RecTbl[];
void Code25930_Alloc182At(void *out, int id) {
    volatile u32 *o = (volatile u32 *)out;
#ifndef __APPLE__
    int h = sub_0800572C(182);
#else
    int h = _0800572C(182);
#endif
    const u8 *base;
    u32 off;
    __asm__(".globl C25RecTbl\nC25RecTbl = 0x080CD830\n");
    o[0] = (u32)h;
    base = (const u8 *)C25RecTbl;
    off = ((u32)id << 2) + (u32)id;
    off <<= 2;
    o[1] = (u32)*(const s16 *)(const void *)(base + off);   // ROM reads it with `ldrsh`
}
#ifndef __APPLE__
void _08025C08(void *a, int b) __attribute__((alias("Code25930_Alloc182At")));
void sub_08025C08(void *a, int b) __attribute__((alias("Code25930_Alloc182At")));
#endif

typedef struct { u32 w0, w1, w2, w3, w4; } C25Body20;
extern const u8 C25RecTbl_C30[];
// ----------------------------------------------------------------------------
// 0x08025C30 — allocate 182 B into [rec+20], s16 id at [rec+24],
// then copy 20 B body (ldmia/stmia pattern).
void Code25930_Alloc182Body(void *rec, int id) {
    volatile u32 *o = (volatile u32 *)rec;
#ifndef __APPLE__
    int h = sub_0800572C(182);
#else
    int h = _0800572C(182);
#endif
    const u8 *base;
    u32 off;
    const u8 *row;
    __asm__(".globl C25RecTbl_C30\nC25RecTbl_C30 = 0x080CD830\n");
    o[5] = (u32)h;
    base = (const u8 *)C25RecTbl_C30;
    off = ((u32)id << 2) + (u32)id;
    off <<= 2;
    row = base + off;
    o[6] = (u32)(s32)((const s16 *)(const void *)row)[0];
    *(C25Body20 *)o = *(const C25Body20 *)(const void *)row;
}
#ifndef __APPLE__
void _08025C30(void *a, int b) __attribute__((alias("Code25930_Alloc182Body")));
void sub_08025C30(void *a, int b) __attribute__((alias("Code25930_Alloc182Body")));
#endif

// ----------------------------------------------------------------------------
// sub_080025C64 — body copy without allocation (dst = r0):
//   [dst+24] = s16[0x080CD830 + id*20]; then copy 20 B body.
void Code25930_CopyBody(void *dst_, int id) {
    volatile u32 *o = (volatile u32 *)dst_;
    u32 off = ((u32)id << 2) + (u32)id;
    off <<= 2;
    volatile u32 *p = (volatile u32 *)(uintptr_t)(C25_REC_TBL + off);
    o[6] = (u32)*(volatile s16 *)p;   // [dst+24]
    o[7] = p[0]; o[8] = p[1]; o[9] = p[2];
    o[10] = p[3]; o[11] = p[4];
}
#ifndef __APPLE__
void _080025C64(void *a, int b) __attribute__((alias("Code25930_CopyBody")));
void sub_080025C64(void *a, int b) __attribute__((alias("Code25930_CopyBody")));
#endif

// ----------------------------------------------------------------------------
// sub_080025C84(layer, row, col, value) — packed 2-bit grid write
// (the ROM function is a *setter*, not a clear: it clears the 2-bit field then
// ORs `value & 3` back in). Index = 44*layer + 11*row + col, 4 cells per byte
// at `0x03001780 + index/4`, mask = table 0x08060D48[index & 3].
void Code25930_GridSet(int layer, int row, int col, int value) {
    u8 masks[4];
    int index;
    register int rem __asm__("r1");
    register int q __asm__("r0");
    volatile u8 *base;
    register volatile u8 *p __asm__("r5");
    register volatile u8 *mp __asm__("r0");
    register u8 cur __asm__("r1");
    register u8 t __asm__("r0");
    register u8 oldv __asm__("r2");
    int sh;
    _0802E0A4((void *)masks, (const void *)(uintptr_t)C25_MASKS, 4u);
    index = layer * 44 + row * 11 + col;
    value &= 3;                           // ROM masks the value right after the index
    rem = _0802D97C(index, 4);            // ROM r1 = index % 4 (first DivRem)
    __asm__(".globl C25GridBase\nC25GridBase = 0x03001780\n");
    base = (volatile u8 *)(uintptr_t)C25GridBase;
    q = index / 4;                        // cmp/bge/adds #3/asrs #2 is a signed /4
    p = base + q;
    mp = &masks[rem];
    cur = *p;
    cur = (u8)(cur & (u8)~(*mp));
    t = cur;
    *p = t;
    sh = _0802D97C(index, 4) * 2;         // ROM: second DivRem, then <<1
    *p = (u8)(((u8)(value << sh)) | (oldv = *p));
}
#ifndef __APPLE__
void _080025C84(int a, int b, int c, int d) __attribute__((alias("Code25930_GridSet")));
void sub_080025C84(int a, int b, int c, int d) __attribute__((alias("Code25930_GridSet")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void Sub_08025C84(int layer, int row, int col, int value) __attribute__((alias("Code25930_GridSet")));
int _08025A8C(int i) __attribute__((alias("Code25930_GetEntry")));
int _08025A9C(int val) __attribute__((alias("Code25930_FindEntry")));
int _08025AFC(void) __attribute__((alias("Code25930_GetSpecialCount")));
int _08025B08(int i) __attribute__((alias("Code25930_GetSpecial")));
int _08025B18(int val) __attribute__((alias("Code25930_FindSpecial")));
void _08025B3C(void) __attribute__((alias("Code25930_CopyTemplate370")));
void _08025B88(void) __attribute__((alias("Code25930_CopyTemplateC70")));
void _08025C84(int layer, int row, int col, int value) __attribute__((alias("Code25930_GridSet")));
void sub_08025C84(int layer, int row, int col, int value) __attribute__((alias("Code25930_GridSet")));
#endif
