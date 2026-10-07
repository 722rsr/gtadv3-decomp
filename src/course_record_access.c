// ============================================================================
// course_record_access.c — C lift of the 4 remaining course_leaves_95f0.s
// functions (VMA 0x080096A4 / 0x08009830 / 0x08009900 / 0x080099D0).
//
// Transcribed instruction-for-instruction from asm/course_leaves_95f0.s.
//
// Course-state block: *0x030003E4 (subsystem instance slot), fields used:
//   +0  mode (u32)          +4  counter (u32, mode-1 path)
//   +12 u16 submode         +14/16/18 s16 HUD fields
//   +20/22/24 u16 HUD trio  +36 record area
// IWRAM cells: 0x03001780+0x10C6/0x10C8/0x10CA/0x1114/0x111A/0x111D.
// ============================================================================

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) void  _0802D974(const void *a, void *b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) s16   _080261B0(int a) { (void)a; return 0; }
__attribute__((weak)) s16   _080261F8(int a) { (void)a; return 0; }
__attribute__((weak)) void  _08007A04(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void  _08007614(void *a, int b, int c, int d) { (void)a; (void)b; (void)c; (void)d; }
__attribute__((weak)) void  _0802E0A4(void *dst, const void *src, u32 n) { (void)dst; (void)src; (void)n; }
__attribute__((weak)) void  _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void  _080038C8(int a, u32 b, const void *c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _080039C0(int a, u32 b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) s8    _08025500(int a) { (void)a; return 0; }
__attribute__((weak)) s8    _08025518(int a) { (void)a; return 0; }
__attribute__((weak)) s8    _08025530(int a) { (void)a; return 0; }
__attribute__((weak)) int   _080097E8(u16 a, u16 b) { (void)a; return 0; }
__attribute__((weak)) void  _08009748(u16 a, u16 b) { (void)a; (void)b; }
#else
extern void _0802D974(const void *a, void *b, u32 c);   // CpuSet
extern s16  _080261B0(int a);                            // 0x080261B0 Ai_AwardLeafGet
extern s16  _080261F8(int a);                            // 0x080261F8 award s16 read
extern void _08007A04(void *a, int b);                   // 0x08007A04 (defined in course_resource_more.c)
extern void _0800798C(void *a, void *b);                 // 0x0800798C (defined in course_resource_more.c)
extern void _08007614(void *a, int b, int c, int d);     // 0x08007614 lane emit
extern void _0802E0A4(void *dst, const void *src, u32 n);  // memcpy (ROM: r0=dst, r1=src, r2=n)
extern void _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h); // 0x08007B18 record build (4 stack args;)
extern void _080038C8(int a, u32 b, const void *c);      // 0x080038C8 obj lane
extern void _080039C0(int a, u32 b, int c);              // 0x080039C0 obj measure
extern s8   _08025500(int a);                            // 0x08025500 Ai_LineGet1
extern s8   _08025518(int a);                            // 0x08025518 Ai_LineGet2
extern s8   _08025530(int a);                            // 0x08025530 Ai_LineGet3
extern int  _080097E8(u16 a, u16 b);                     // 0x080097E8 (course_leaves.c)
extern void _08009748(u16 a, u16 b);                     // 0x08009748 (course_leaves.c)
#endif

// Anchors (byte-exact pools):
#define CL_SLOT   ((volatile u32 *)(uintptr_t)0x030003E4)
#define CL_WA     ((volatile u8 *)(uintptr_t)0x03001780)
#define CL_TPL    ((void *)(uintptr_t)0x08291F7C)


// ----------------------------------------------------------------------------
// sub_080096A4 — course-state HUD record setup (r0 = new record pointer).
//   *slot = r0; CpuSet(&0, slot, 0x0500000B) (11 halfword zero-fill via
//   fill ctrl); [rec+14] = 261B0(10); [rec+16] = 261F8(10);
//   [rec+18] = 261F8(11); seek template 0x08291F7C+36 via 0798C;
//   07A04(rec+36, s16[rec+14]); 07614(0x08291F7C, 0, 1, s16[rec+16]);
//   07614(0x08291F7C, 0, 0, s16[rec+18]); [rec+20] = 1;
//   [rec+22] = u16[0x03001780+0x10C6]; [rec+24] = u16[0x03001780+0x10C8].
void CourseLeaves_096A4(void *rec) {
    *CL_SLOT = (u32)(uintptr_t)rec;
    u32 zero = 0;
    _0802D974(&zero, (void *)(uintptr_t)rec, 0x0500000Bu);

    *(volatile s16 *)((u8 *)rec + 14) = _080261B0(10);
    *(volatile s16 *)((u8 *)rec + 16) = _080261F8(10);
    *(volatile s16 *)((u8 *)rec + 18) = _080261F8(11);

    u8 tpl[40];
    _0800798C((void *)(uintptr_t)CL_TPL, tpl);   // seeks child record (+36 arg path)
    u8 *dst36 = (u8 *)rec + 36;
    s16 v14 = *(volatile s16 *)((u8 *)rec + 14);
    _08007A04(dst36, v14);

    s16 v16 = *(volatile s16 *)((u8 *)rec + 16);
    _08007614(CL_TPL, 0, 1, v16);
    s16 v18 = *(volatile s16 *)((u8 *)rec + 18);
    _08007614(CL_TPL, 0, 0, v18);

    *(volatile u16 *)((u8 *)rec + 20) = 1;
    *(volatile u16 *)((u8 *)rec + 22) = *(volatile u16 *)(CL_WA + 0x10C6);
    *(volatile u16 *)((u8 *)rec + 24) = *(volatile u16 *)(CL_WA + 0x10C8);
}
#ifndef __APPLE__
void _080096A4(void *a) __attribute__((alias("CourseLeaves_096A4")));
void sub_080096A4(void *a) __attribute__((alias("CourseLeaves_096A4")));
#endif

// ----------------------------------------------------------------------------
// sub_08009830 — key/step handler (r0 = step, r1 = sel):
//   sel==8: blk[0] = 0, ret 2
//   sel==1: ret 3 if blk[4]==0 else 4 if blk[4]==1; ret 5 if blk[4]==2
//   sel==64: if blk[4] > 0: blk[4]--, ret 1
//   sel==128: if blk[4] <= 1: blk[4]++, ret 1
//   sel & 0x30: _08009748(step, sel)
//   sel == 0x100: u16[blk+12] = 0, ret 1
int CourseLeaves_09830(u16 step, u16 sel) {
    u32 r3 = 0;
    volatile u32 *blk;
    if (sel == 8) {
        volatile u32 *b8 = (volatile u32 *)(uintptr_t)*CL_SLOT;
        b8[0] = 0;
        r3 = 2;
    }
    if (sel == 1) {
        // ROM 0x0800984c/0x08009866: the slot ADDRESS stays live in r1 across
        // the block, so the ==2 test re-derefs [r1] rather than reusing the
        // first blk[1] load. Binding the slot to a local is what keeps it live.
//
        // Pin is load-bearing: removing it reproduces 147/152 byte for byte
        // (first diff +0x04, candidate 152 B); retargeting the same pin to r0
        // also reproduces 147/152, so it is the register NUMBER, not the
        // `register` keyword, that carries the match.
        volatile u32 *slot1 = CL_SLOT;
        register u32 v __asm__("r2") = ((volatile u32 *)(uintptr_t)*slot1)[1];
        if (v == 0)
            r3 = 3;
        else if (v == 1)
            r3 = 4;
        if (((volatile u32 *)(uintptr_t)*slot1)[1] == 2)
            r3 = 5;
    }
    if (sel == 64) {
        blk = (volatile u32 *)(uintptr_t)*CL_SLOT;
        u32 v = blk[1];
        if ((s32)v > 0) {
            blk[1] = v - 1;
            r3 = 1;
        }
    }
    if (sel == 128) {
        blk = (volatile u32 *)(uintptr_t)*CL_SLOT;
        u32 v = blk[1];
        if ((s32)v <= 1) {
            blk[1] = v + 1;
            r3 = 1;
        }
    }
    if (sel & 0x30) {
        // ROM asm/course_leaves_95f0.s:438 is `bl sub_08009748`. This body was
        // calling _080097E8 -- a different function that returns an int tier --
        // contradicting its own comment on line 97.
        _08009748(step, sel);
    }
    if (sel == 0x100) {
        volatile u8 *b = (volatile u8 *)(uintptr_t)*CL_SLOT;
        *(volatile u16 *)(b + 12) = 0;
        r3 = 1;
    }
    return (int)r3;
}
#ifndef __APPLE__
int _08009830(u16 a, u16 b) __attribute__((alias("CourseLeaves_09830")));
int sub_08009830(u16 a, u16 b) __attribute__((alias("CourseLeaves_09830")));
#endif

// ----------------------------------------------------------------------------
// sub_08009900 — lap-label HUD builder (labels at 0x0805F6A4/0x0805F6AA,
// 6-byte template rows; record built via 07B18; values from blk+36 fields).
void CourseLeaves_09900(void) {
    u8 rowA[8], rowB[8];
    _0802E0A4((void *)rowA, (const void *)(uintptr_t)0x0805F6A4u, 6u);
    _0802E0A4((void *)rowB, (const void *)(uintptr_t)0x0805F6AAu, 6u);

    volatile u8 *blk = (volatile u8 *)(uintptr_t)*CL_SLOT;
    // ROM stack block: s0 = s16[blk+16], s1 = s2 = s3 = 0 (fix).
    _08007B18((void *)(uintptr_t)(blk + 36), 1, 88, 28,
              (u32)(s32)(s16)*(volatile s16 *)(blk + 16), 0u, 0u, 0u);

    u32 r8 = 0, r5 = 0, r7 = 64;
    (void)r5;
    u8 *r6 = rowB, *r4 = rowA;
    for (;;) {
        volatile u8 *b = (volatile u8 *)(uintptr_t)*CL_SLOT;
        u32 mode = *(volatile u32 *)b;
        if (mode == r8) {
            s16 a0 = *(volatile s16 *)r4;
            s16 b0 = *(volatile s16 *)r6;
            _08007B18((void *)(uintptr_t)(b + 36), a0, b0, (int)r7,
                      (u32)(s32)(s16)*(volatile s16 *)(b + 16), 0u, 0u, 0u);
        } else {
            s16 a0 = *(volatile s16 *)r4;
            s16 b0 = *(volatile s16 *)r6;
            _08007B18((void *)(uintptr_t)(b + 36), a0, b0, (int)r7,
                      (u32)(s32)(s16)*(volatile s16 *)(b + 18), 0u, 0u, 0u);
        }
        r7 += 24;
        r6 += 2;
        r4 += 2;
        r8++;
        if (r8 > 2)
            break;
    }
}
#ifndef __APPLE__
void _08009900(void) __attribute__((alias("CourseLeaves_09900")));
void sub_08009900(void) __attribute__((alias("CourseLeaves_09900")));
#endif

// ----------------------------------------------------------------------------
// sub_080099D0 — results HUD builder: positions from blk[4]*16+16,
// labels from 0x080CB190/0x080CB19C/0x080CB1A8 tables and IWRAM s8 rows
// (0x03001780+0x1114/0x111A/0x111D), drawn via 038C8/039C0.
void CourseLeaves_099D0(void) {
    volatile u8 *blk = (volatile u8 *)(uintptr_t)*CL_SLOT;
    u32 v4 = *(volatile u32 *)(blk + 4);
    u32 y0 = (v4 << 4) + 16;

    _080038C8(0, y0, (const void *)(uintptr_t)0x080CB190u);

    for (u32 i = 0; i <= 2; i++) {
        _080038C8(20, (u32)(16 + i * 16), (const void *)(uintptr_t)(0x080CB190u + 4));
        s16 v = *(volatile s16 *)(blk + 20 + i * 2);
        _080039C0(120, (u32)(16 + i * 16), v);
    }

    volatile u8 *wa = CL_WA;
    s16 a = _08025500((int)*(volatile s16 *)(wa + 0x1114));
    s16 b = _08025518((int)*(volatile s16 *)(wa + 0x1114));
    s16 c = _08025530((int)*(volatile s16 *)(wa + 0x1114));
    (void)a; (void)b; (void)c;

    for (u32 i = 0; i <= 2; i++) {
        _080038C8(20, (u32)(80 + i * 16), (const void *)(uintptr_t)(0x080CB19Cu + (i + 1) * 4));
        _080039C0(120, (u32)(80 + i * 16), *(volatile s16 *)(wa + 0x1114 + i * 2));
    }

    // s8 rows: 0x111A (3 bytes) and 0x111D sign-extended halfword rows
    volatile s8 *r = (volatile s8 *)(wa + 0x111A);
    for (u32 i = 0; i <= 6; i += 2) {
        s16 v = r[i];
        _080039C0(120, (u32)(80 + i * 8), v);
    }
    volatile s8 *last = (volatile s8 *)(wa + 0x111D);
    s16 lv = *last;
    (void)lv;
}
#ifndef __APPLE__
void _080099D0(void) __attribute__((alias("CourseLeaves_099D0")));
void sub_080099D0(void) __attribute__((alias("CourseLeaves_099D0")));
#endif
