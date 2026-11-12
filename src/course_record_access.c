// Course record and HUD accessors.

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
extern void sub_0802D974(const void *a, void *b, u32 c); // CpuSet
extern s16  _080261B0(int a);                            // 0x080261B0 Ai_AwardLeafGet
extern s16  _080261F8(int a);                            // 0x080261F8 award s16 read
extern void _08007A04(void *a, int b);                   // 0x08007A04 (defined in course_resource_more.c)
extern void _0800798C(void *a, void *b);                 // 0x0800798C (defined in course_resource_more.c)
extern void _08007614(void *a, int b, int c, int d);     // 0x08007614 lane emit
extern void _0802E0A4(void *dst, const void *src, u32 n);  // memcpy (ROM: r0=dst, r1=src, r2=n)
extern void _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h); // 0x08007B18 record build (4 stack args;)
extern void sub_080038C8(int a, u32 b, const void *c);   // 0x080038C8 obj lane
extern void sub_080039C0(int a, u32 b, int c);           // 0x080039C0 obj measure
extern s16  _08025500(int a);                            // 0x08025500 Ai_LineGet1 (call site stores 16 bits)
extern s16  _08025518(int a);                            // 0x08025518 Ai_LineGet2 (call site stores 16 bits)
extern s16  _08025530(int a);                            // 0x08025530 Ai_LineGet3 (call site stores 16 bits)
extern int  _080097E8(u16 a, u16 b);                     // 0x080097E8 (course_leaves.c)
extern void _08009748(u16 a, u16 b);                     // 0x08009748 (course_leaves.c)

// Anchors (byte-exact pools):
#define CL_SLOT   ((volatile u32 *)(uintptr_t)0x030003E4)


// 0x080096A4: initialize the course HUD record and copy its template fields.
void CourseLeaves_096A4(void *rec)
{
    void *new_record = rec;
    void **slot = (void **)0x030003E4u;
    u32 zero_storage;
    u32 second_offset;
    register volatile u8 *current __asm__("r2");
    register volatile u8 *work __asm__("r0");
    register volatile u8 *source __asm__("r1");
    register u32 offset __asm__("r3");
    register u32 zero_value __asm__("r0");
    extern u8 CourseLeaves096A4Wa[];
    void *template;

    __asm__(".globl CourseLeaves096A4Wa\nCourseLeaves096A4Wa = 0x03001780\n");
    __asm__ volatile ("" : "+r" (new_record) : : "memory");
    *slot = new_record;
    zero_value = 0;
    zero_storage = zero_value;
    sub_0802D974(&zero_storage, new_record, 0x0500000Bu);
    *(volatile u16 *)((u8 *)*slot + 14) = _080261B0(10);
    *(volatile u16 *)((u8 *)*slot + 16) = _080261F8(10);
    *(volatile u16 *)((u8 *)*slot + 18) = _080261F8(11);

    template = (void *)(uintptr_t)0x08291F7Cu;
    _0800798C(template, (u8 *)*slot + 36);
    _08007A04((u8 *)*slot + 36, *(s16 *)((u8 *)*slot + 14));
    _08007614(template, 0, 1, *(s16 *)((u8 *)*slot + 16));
    _08007614(template, 0, 0, *(s16 *)((u8 *)*slot + 18));
    current = *slot;
    *(volatile u16 *)(current + 20) = 1;
    work = (volatile u8 *)CourseLeaves096A4Wa;
    offset = 0x10C6u;
    __asm__ volatile ("" : "+r" (work), "+r" (offset) : : "memory");
    source = work + offset;
    *(volatile u16 *)(current + 22) = *(volatile u16 *)source;
    second_offset = 0x10C8u;
    __asm__ volatile ("" : "+r" (second_offset) : : "memory");
    *(volatile u16 *)(current + 24) = *(volatile u16 *)(work + second_offset);
}
void _080096A4(void *a) __attribute__((alias("CourseLeaves_096A4")));
void sub_080096A4(void *a) __attribute__((alias("CourseLeaves_096A4")));

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
        {
            u32 v = blk[1];
            if ((s32)v > 0) {
                blk[1] = v - 1;
                r3 = 1;
            }
        }
    }
    if (sel == 128) {
        blk = (volatile u32 *)(uintptr_t)*CL_SLOT;
        {
            u32 v = blk[1];
            if ((s32)v <= 1) {
                blk[1] = v + 1;
                r3 = 1;
            }
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
int _08009830(u16 a, u16 b) __attribute__((alias("CourseLeaves_09830")));
int sub_08009830(u16 a, u16 b) __attribute__((alias("CourseLeaves_09830")));

// ----------------------------------------------------------------------------
// sub_08009900 — lap-label HUD builder (labels at 0x0805F6A4/0x0805F6AA,
// 6-byte template rows; record built via 07B18; values from blk+36 fields).
void CourseLeaves_09900(void) {
    u8 rowA[8], rowB[8];
    _0802E0A4((void *)rowA, (const void *)(uintptr_t)0x0805F6A4u, 6u);
    _0802E0A4((void *)rowB, (const void *)(uintptr_t)0x0805F6AAu, 6u);

    {
        volatile u8 *blk = (volatile u8 *)(uintptr_t)*CL_SLOT;
        // ROM stack block: s0 = s16[blk+16], s1 = s2 = s3 = 0 (fix).
        _08007B18((void *)(uintptr_t)(blk + 36), 1, 88, 28,
                  (u32)(s32)(s16)*(s16 *)(blk + 16), 0u, 0u, 0u);

        {
            u32 r8 = 0, r5 = 0;
            // Keep the shared zero in r5 before initializing the row position and
            // pointers. The ROM reuses it for all three stack arguments each call.
            __asm__("" : : "r"(r5));
            {
                u32 r7 = 64;
                u8 *r6 = rowB, *r4 = rowA;
                for (;;) {
                    volatile u8 *b = (volatile u8 *)(uintptr_t)*CL_SLOT;
                    u32 mode = *(volatile u32 *)b;
                    if (mode == r8) {
                        s16 a0 = *(s16 *)r4;
                        s16 b0 = *(s16 *)r6;
                        _08007B18((void *)(uintptr_t)(b + 36), a0, b0, (int)r7,
                                  (u32)(s32)(s16)*(s16 *)(b + 16), r5, r5, r5);
                    } else {
                        s16 a0 = *(s16 *)r4;
                        s16 b0 = *(s16 *)r6;
                        _08007B18((void *)(uintptr_t)(b + 36), a0, b0, (int)r7,
                                  (u32)(s32)(s16)*(s16 *)(b + 18), r5, r5, r5);
                    }
                    r7 += 24;
                    r6 += 2;
                    r4 += 2;
                    r8++;
                    if ((s32)r8 > 2)
                        break;
                }
            }
        }
    }
}
void _08009900(void) __attribute__((alias("CourseLeaves_09900")));
void sub_08009900(void) __attribute__((alias("CourseLeaves_09900")));

// ----------------------------------------------------------------------------
// sub_080099D0 — results HUD builder: positions from blk[4]*16+16,
// labels from 0x080CB190/0x080CB19C/0x080CB1A8 tables and IWRAM s8 rows
// (0x03001780+0x1114/0x111A/0x111D), drawn via 038C8/039C0.
void CourseLeaves_099D0(void)
{
    extern u8 CourseLeaves_099D0_Wa[];
    u8 frame[52];
    s16 v[8];
    s16 *rows, *src, *q;
    const u32 *tab1, *tab2, *tab3;
    s8 b;
    register int i __asm__("r4");
    register int y __asm__("r5");
    u32 o;

    __asm__(".globl CourseLeaves_099D0_Wa\nCourseLeaves_099D0_Wa = 0x03001780\n");

    sub_080038C8(0, (*(u32 *)((u8 *)(uintptr_t)*CL_SLOT + 4) << 4) + 16,
               (const void *)0x0805F6B0u);

    i = 0;
    rows = v;
    tab1 = (const u32 *)0x080CB190u;
    y = 16;
    do {
        s16 *rowp;
        s32 idx;
        sub_080038C8(20, y, (const void *)*tab1++);
        rowp = (s16 *)(uintptr_t)*CL_SLOT;
        idx = i * 2;
        rowp = (s16 *)((u8 *)rowp + 20);
        sub_080039C0(120, y, *(s16 *)((u8 *)rowp + idx));
        y += 16;
        i++;
    } while (i <= 2);

    {
        register u8 *work __asm__("r4");
        register u32 off __asm__("r2");
        work = (u8 *)CourseLeaves_099D0_Wa;
        off = 0x1114u;
        __asm__ volatile ("" : "+r" (work), "+r" (off));
        src = (s16 *)(work + off);
    }
    {
        // The three reads use zero-index scratch registers r1/r2/r1. Bracket
        // each load with a dummy in the register it must avoid.
        register s32 val __asm__("r0");
        register s32 hold __asm__("r2");
        __asm__ volatile ("" : "=r" (hold));
        val = *src;
        __asm__ volatile ("" : : "r" (hold));
        rows[0] = _08025500(val);
    }
    {
        register s32 val __asm__("r0");
        register s32 hold __asm__("r1");
        __asm__ volatile ("" : "=r" (hold));
        val = *src;
        __asm__ volatile ("" : : "r" (hold));
        rows[1] = _08025518(val);
    }
    {
        register s32 val __asm__("r0");
        register s32 hold __asm__("r2");
        __asm__ volatile ("" : "=r" (hold));
        val = *src;
        __asm__ volatile ("" : : "r" (hold));
        rows[2] = _08025530(val);
    }

    i = 0;
    q = rows;
    y = 80;
    {
        register u32 t2 __asm__("r2");
        t2 = 0x080CB19Cu;
        __asm__ volatile ("" : "+r" (t2));
        tab2 = (const u32 *)t2;
    }
    do {
        sub_080038C8(20, y, (const void *)*tab2++);
        {
            register s32 val __asm__("r2");
            register s32 hold __asm__("r0");
            __asm__ volatile ("" : "=r" (hold));
            val = *q;
            __asm__ volatile ("" : : "r" (hold));
            sub_080039C0(120, y, (s16)val);
        }
        q++;
        y += 16;
        i++;
    } while (i <= 2);

    {
        register u8 *wb __asm__("r1");
        register u32 off __asm__("r2");
        register s8 bb __asm__("r0");
        wb = (u8 *)CourseLeaves_099D0_Wa;
        off = 0x111Au;
        bb = *(u8 *)(wb + off);
        rows[0] = bb;
        off += 4;
        bb = *(u8 *)(wb + off);
        rows[1] = bb;
        off -= 2;
        bb = *(u8 *)(wb + off);
        rows[2] = bb;
        off += 3;
        bb = *(u8 *)(wb + off);
        rows[3] = bb;
        off -= 4;
        bb = *(u8 *)(wb + off);
        rows[4] = bb;
        off += 5;
        bb = *(u8 *)(wb + off);
        rows[5] = bb;
        off += 1;
        bb = *(u8 *)(wb + off);
        rows[6] = bb;
        __asm__ volatile ("" : "+r" (off));
        {
            register s32 hold __asm__("r2");
            s32 last;
            __asm__ volatile ("" : "=r" (hold));
            last = *(s8 *)(wb + 0x111D);
            __asm__ volatile ("" : : "r" (hold));
            rows[7] = last;
        }
    }

    q = rows;
    tab3 = (const u32 *)0x080CB1A8u;
    y = 16;
    i = 7;
    do {
        sub_080038C8(130, y, (const void *)*tab3++);
        sub_080039C0(220, y, *q);
        q++;
        y += 16;
        i--;
    } while (i >= 0);
}
void _080099D0(void) __attribute__((alias("CourseLeaves_099D0")));
void sub_080099D0(void) __attribute__((alias("CourseLeaves_099D0")));
