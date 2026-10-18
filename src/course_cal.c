#include "gtadv/course_cal.h"

// Reference: asm/course_cal.s 0x080258B8-0x08025930
// Six Thumb s16 getters over ROM table 0x08060124, with 12-byte rows.
// The r2 table base and r1 row address retain the ROM's literal-load and
// add order. The empty barrier keeps the direct address live as a register
// value; each final signed-halfword read remains ordinary C.

s16 Course_GetCup(int idx) {
    register const u8 *base __asm__("r2") =
        (const u8 *)(uintptr_t)0x08060124u;
    register u32 row __asm__("r1") = (u32)idx * COURSE_CALENDAR_STRIDE;
    __asm__ volatile ("" : "+r" (base) : : "memory");
    row += (u32)(uintptr_t)base;
    return *(const s16 *)(uintptr_t)(row + 0);
}
s16 Course_GetSeqnum(int idx) {
    register const u8 *base __asm__("r2") =
        (const u8 *)(uintptr_t)0x08060124u;
    register u32 row __asm__("r1") = (u32)idx * COURSE_CALENDAR_STRIDE;
    __asm__ volatile ("" : "+r" (base) : : "memory");
    row += (u32)(uintptr_t)base;
    return *(const s16 *)(uintptr_t)(row + 8);
}
s16 Course_GetSong(int idx) {
    register const u8 *base __asm__("r2") =
        (const u8 *)(uintptr_t)0x08060124u;
    register u32 row __asm__("r1") = (u32)idx * COURSE_CALENDAR_STRIDE;
    __asm__ volatile ("" : "+r" (base) : : "memory");
    row += (u32)(uintptr_t)base;
    return *(const s16 *)(uintptr_t)(row + 2);
}
s16 Course_GetCourseDefault(int idx) {
    register const u8 *base __asm__("r2") =
        (const u8 *)(uintptr_t)0x08060124u;
    register u32 row __asm__("r1") = (u32)idx * COURSE_CALENDAR_STRIDE;
    __asm__ volatile ("" : "+r" (base) : : "memory");
    row += (u32)(uintptr_t)base;
    return *(const s16 *)(uintptr_t)(row + 4);
}
s16 Course_GetCourseVariant(int idx) {
    register const u8 *base __asm__("r2") =
        (const u8 *)(uintptr_t)0x08060124u;
    register u32 row __asm__("r1") = (u32)idx * COURSE_CALENDAR_STRIDE;
    __asm__ volatile ("" : "+r" (base) : : "memory");
    row += (u32)(uintptr_t)base;
    return *(const s16 *)(uintptr_t)(row + 6);
}
s16 Course_GetParam(int idx) {
    register const u8 *base __asm__("r2") =
        (const u8 *)(uintptr_t)0x08060124u;
    register u32 row __asm__("r1") = (u32)idx * COURSE_CALENDAR_STRIDE;
    __asm__ volatile ("" : "+r" (base) : : "memory");
    row += (u32)(uintptr_t)base;
    return *(const s16 *)(uintptr_t)(row + 10);
}

// Legacy aliases so existing BLs resolve (ARM only; alias unsupported on Darwin host)
s16 _080258B8(int idx) __attribute__((alias("Course_GetCup")));
s16 sub_080258B8(int idx) __attribute__((alias("Course_GetCup")));
s16 _080258CC(int idx) __attribute__((alias("Course_GetSeqnum")));
s16 _080258E0(int idx) __attribute__((alias("Course_GetSong")));
s16 _080258F4(int idx) __attribute__((alias("Course_GetCourseDefault")));
s16 _08025908(int idx) __attribute__((alias("Course_GetCourseVariant")));
s16 _0802591C(int idx) __attribute__((alias("Course_GetParam")));

// ROM entry alias.
s16 _0800258B8(int idx) __attribute__((alias("Course_GetCup")));
s16 _0800258CC(int idx) __attribute__((alias("Course_GetSeqnum")));
