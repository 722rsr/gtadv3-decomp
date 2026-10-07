#include "gtadv/course_cal.h"

// Reference: asm/course_cal.s 0x080258B8-0x08025930
// Six pure Thumb s16 getters over ROM table 0x08060124 stride 12.
// Behavioral eq: ((idx*3)*4) = idx*12 with lsls+adds; ldrsh with offset.
// Original literals all point to same base 0x08060124.
//
// The base must be a SYMBOL_REF leaf, not a folded integer constant, or the
// literal load sinks to its point of use. With `COURSE_CALENDAR_BASE`
// (a `(const u8 *)0x08060124` cast) the reload pass deletes the constant's
// defining insn and re-materialises it as late as GCC can -- the last insn
// before the `adds` -- because r0 dies at `adds r1,r1,r0`. That yields
//
//     lsls r1,r0,#1; adds r1,r1,r0; lsls r1,r1,#2
//     ldr  r0,=base; adds r1,r1,r0; movs r2,#0; ldrsh r0,[r1,r2]
//
// The absolute symbol must be declared INSIDE each body: the per-body splice
// extracts only the function's brace-matched text, so a file-scope `__asm__`
// that defines the symbol is simply absent from the spliced section. C89 also
// requires the declarations to precede it.
extern const u8 CourseCalTable[];

s16 Course_GetCup(int idx) {
    const u8 *base;
    u32 row;
    __asm__(".globl CourseCalTable\nCourseCalTable = 0x08060124\n");
    base = (const u8 *)CourseCalTable;
    row = (u32)idx * COURSE_CALENDAR_STRIDE;
    return *((const s16 *)(base + row + 0));
}
s16 Course_GetSeqnum(int idx) {
    const u8 *base;
    u32 row;
    __asm__(".globl CourseCalTable\nCourseCalTable = 0x08060124\n");
    base = (const u8 *)CourseCalTable;
    row = (u32)idx * COURSE_CALENDAR_STRIDE;
    return *((const s16 *)(base + row + 8));
}
s16 Course_GetSong(int idx) {
    const u8 *base;
    u32 row;
    __asm__(".globl CourseCalTable\nCourseCalTable = 0x08060124\n");
    base = (const u8 *)CourseCalTable;
    row = (u32)idx * COURSE_CALENDAR_STRIDE;
    return *((const s16 *)(base + row + 2));
}
s16 Course_GetCourseDefault(int idx) {
    const u8 *base;
    u32 row;
    __asm__(".globl CourseCalTable\nCourseCalTable = 0x08060124\n");
    base = (const u8 *)CourseCalTable;
    row = (u32)idx * COURSE_CALENDAR_STRIDE;
    return *((const s16 *)(base + row + 4));
}
s16 Course_GetCourseVariant(int idx) {
    const u8 *base;
    u32 row;
    __asm__(".globl CourseCalTable\nCourseCalTable = 0x08060124\n");
    base = (const u8 *)CourseCalTable;
    row = (u32)idx * COURSE_CALENDAR_STRIDE;
    return *((const s16 *)(base + row + 6));
}
s16 Course_GetParam(int idx) {
    const u8 *base;
    u32 row;
    __asm__(".globl CourseCalTable\nCourseCalTable = 0x08060124\n");
    base = (const u8 *)CourseCalTable;
    row = (u32)idx * COURSE_CALENDAR_STRIDE;
    return *((const s16 *)(base + row + 10));
}

// Legacy aliases so existing BLs resolve (ARM only; alias unsupported on Darwin host)
#ifndef __APPLE__
s16 _080258B8(int idx) __attribute__((alias("Course_GetCup")));
s16 sub_080258B8(int idx) __attribute__((alias("Course_GetCup")));
s16 _080258CC(int idx) __attribute__((alias("Course_GetSeqnum")));
s16 _080258E0(int idx) __attribute__((alias("Course_GetSong")));
s16 _080258F4(int idx) __attribute__((alias("Course_GetCourseDefault")));
s16 _08025908(int idx) __attribute__((alias("Course_GetCourseVariant")));
s16 _0802591C(int idx) __attribute__((alias("Course_GetParam")));
#endif

// ROM entry alias.
#ifndef __APPLE__
s16 _0800258B8(int idx) __attribute__((alias("Course_GetCup")));
s16 _0800258CC(int idx) __attribute__((alias("Course_GetSeqnum")));
#endif
