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
extern void _080038C8(int a, u32 b, const void *c);      // 0x080038C8 obj lane
extern void _080039C0(int a, u32 b, int c);              // 0x080039C0 obj measure
extern s8   _08025500(int a);                            // 0x08025500 Ai_LineGet1
extern s8   _08025518(int a);                            // 0x08025518 Ai_LineGet2
extern s8   _08025530(int a);                            // 0x08025530 Ai_LineGet3
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
int _08009830(u16 a, u16 b) __attribute__((alias("CourseLeaves_09830")));
int sub_08009830(u16 a, u16 b) __attribute__((alias("CourseLeaves_09830")));

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
              (u32)(s32)(s16)*(s16 *)(blk + 16), 0u, 0u, 0u);

    u32 r8 = 0, r5 = 0;
    // Keep the shared zero in r5 before initializing the row position and
    // pointers. The ROM reuses it for all three stack arguments each call.
    __asm__("" : : "r"(r5));
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
void _08009900(void) __attribute__((alias("CourseLeaves_09900")));
void sub_08009900(void) __attribute__((alias("CourseLeaves_09900")));

// ----------------------------------------------------------------------------
// sub_080099D0 — results HUD builder: positions from blk[4]*16+16,
// labels from 0x080CB190/0x080CB19C/0x080CB1A8 tables and IWRAM s8 rows
// (0x03001780+0x1114/0x111A/0x111D), drawn via 038C8/039C0.
__attribute__((naked)) void CourseLeaves_099D0(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r8\n"
        "push {r7}\n"
        "sub sp, #68\n"
        "ldr r0, .Lcr099d0_08009b04\n"
        "ldr r0, [r0, #0]\n"
        "ldr r1, [r0, #4]\n"
        "lsls r1, r1, #4\n"
        "adds r1, #16\n"
        "ldr r2, .Lcr099d0_08009b08\n"
        "movs r0, #0\n"
        "bl 0x080038C8\n"
        "movs r4, #0\n"
        "add r7, sp, #52\n"
        "ldr r6, .Lcr099d0_08009b0c\n"
        "movs r5, #16\n"
        ".Lcr099d0_080099f2:\n"
        "ldmia r6!, {r2}\n"
        "movs r0, #20\n"
        "adds r1, r5, #0\n"
        "bl 0x080038C8\n"
        "ldr r0, .Lcr099d0_08009b04\n"
        "ldr r0, [r0, #0]\n"
        "lsls r1, r4, #1\n"
        "adds r0, #20\n"
        "adds r0, r0, r1\n"
        "movs r1, #0\n"
        "ldrsh r2, [r0, r1]\n"
        "movs r0, #120\n"
        "adds r1, r5, #0\n"
        "bl 0x080039C0\n"
        "adds r5, #16\n"
        "adds r4, #1\n"
        "cmp r4, #2\n"
        "ble .Lcr099d0_080099f2\n"
        "ldr r4, .Lcr099d0_08009b10\n"
        "ldr r2, .Lcr099d0_08009b14\n"
        "adds r4, r4, r2\n"
        "movs r1, #0\n"
        "ldrsh r0, [r4, r1]\n"
        "bl sub_08025500\n"
        "strh r0, [r7, #0]\n"
        "movs r2, #0\n"
        "ldrsh r0, [r4, r2]\n"
        "bl sub_08025518\n"
        "strh r0, [r7, #2]\n"
        "movs r1, #0\n"
        "ldrsh r0, [r4, r1]\n"
        "bl sub_08025530\n"
        "strh r0, [r7, #4]\n"
        "movs r4, #0\n"
        "adds r6, r7, #0\n"
        "movs r5, #80\n"
        "ldr r2, .Lcr099d0_08009b18\n"
        "mov r8, r2\n"
        ".Lcr099d0_08009a48:\n"
        "mov r0, r8\n"
        "adds r0, #4\n"
        "mov r8, r0\n"
        "subs r0, #4\n"
        "ldmia r0!, {r2}\n"
        "movs r0, #20\n"
        "adds r1, r5, #0\n"
        "bl 0x080038C8\n"
        "movs r1, #0\n"
        "ldrsh r2, [r6, r1]\n"
        "movs r0, #120\n"
        "adds r1, r5, #0\n"
        "bl 0x080039C0\n"
        "adds r6, #2\n"
        "adds r5, #16\n"
        "adds r4, #1\n"
        "cmp r4, #2\n"
        "ble .Lcr099d0_08009a48\n"
        "ldr r1, .Lcr099d0_08009b10\n"
        "ldr r2, .Lcr099d0_08009b1c\n"
        "adds r0, r1, r2\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "strh r0, [r7, #0]\n"
        "adds r2, #4\n"
        "adds r0, r1, r2\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "strh r0, [r7, #2]\n"
        "subs r2, #2\n"
        "adds r0, r1, r2\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "strh r0, [r7, #4]\n"
        "adds r2, #3\n"
        "adds r0, r1, r2\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "strh r0, [r7, #6]\n"
        "subs r2, #4\n"
        "adds r0, r1, r2\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "strh r0, [r7, #8]\n"
        "adds r2, #5\n"
        "adds r0, r1, r2\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "strh r0, [r7, #10]\n"
        "adds r2, #1\n"
        "adds r0, r1, r2\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "strh r0, [r7, #12]\n"
        "ldr r0, .Lcr099d0_08009b20\n"
        "adds r1, r1, r0\n"
        "movs r0, #0\n"
        "ldrsb r0, [r1, r0]\n"
        "strh r0, [r7, #14]\n"
        "adds r6, r7, #0\n"
        "ldr r7, .Lcr099d0_08009b24\n"
        "movs r5, #16\n"
        "movs r4, #7\n"
        ".Lcr099d0_08009ad8:\n"
        "ldmia r7!, {r2}\n"
        "movs r0, #130\n"
        "adds r1, r5, #0\n"
        "bl 0x080038C8\n"
        "movs r1, #0\n"
        "ldrsh r2, [r6, r1]\n"
        "movs r0, #220\n"
        "adds r1, r5, #0\n"
        "bl 0x080039C0\n"
        "adds r6, #2\n"
        "adds r5, #16\n"
        "subs r4, #1\n"
        "cmp r4, #0\n"
        "bge .Lcr099d0_08009ad8\n"
        "add sp, #68\n"
        "pop {r3}\n"
        "mov r8, r3\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".Lcr099d0_08009b04: .word 0x030003E4\n"
        ".Lcr099d0_08009b08: .word 0x0805F6B0\n"
        ".Lcr099d0_08009b0c: .word 0x080CB190\n"
        ".Lcr099d0_08009b10: .word 0x03001780\n"
        ".Lcr099d0_08009b14: .word 0x00001114\n"
        ".Lcr099d0_08009b18: .word 0x080CB19C\n"
        ".Lcr099d0_08009b1c: .word 0x0000111A\n"
        ".Lcr099d0_08009b20: .word 0x0000111D\n"
        ".Lcr099d0_08009b24: .word 0x080CB1A8\n"
        ".syntax divided\n"
    );
}
void _080099D0(void) __attribute__((alias("CourseLeaves_099D0")));
void sub_080099D0(void) __attribute__((alias("CourseLeaves_099D0")));
