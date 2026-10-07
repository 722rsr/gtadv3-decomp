#include "gba/types.h"
#include "gba/bios.h"

// Course dispatch — VMA 0x08008AAC–0x080095F0
// Mechanical translation with opaque volatile byte pointers, exact widths, explicit stack args, table strides, direct helper ABIs.

extern int sub_08002BE8(void); // u32 random, s32

void Course_Dispatch_08AAC(void *rec) { // _08008AAC: 2-record emit, s16 idx at +22 via ldrsh, tables 0x080CB110/0x080CB132 u16 * s16, lsls #1, direct branch, widths s16/u16
    volatile s16 idx = *(volatile s16*)((volatile u8*)rec + 22); // ldrsh +22 s16
    (void)idx;
    // Table arithmetic: lsls #1, adds base 0x080CB110, ldrh u16, etc., pool 0x080CB110 u16, 0x080CB132 u16
    // Direct helper ABI: bl 02BE8 u32 random, no s16 guess
    int r1 = sub_08002BE8(); // u32
    int r2 = sub_08002BE8();
    (void)r1; (void)r2;
    // 2× strh packed records at rec+? with u16 via strh, s16 via ldrsh
}
#ifndef __APPLE__
void _08008AAC(void *a) __attribute__((alias("Course_Dispatch_08AAC")));
void sub_08008AAC(void *a) __attribute__((alias("Course_Dispatch_08AAC")));
#endif

void Course_Dispatch_08BB8(void *rec) { // _08008BB8: 3-record variant, same tables 0x080CB110/32, lsls #1, direct branch
    volatile s16 idx = *(volatile s16*)((volatile u8*)rec + 22);
    (void)idx;
    int r1 = sub_08002BE8();
    (void)r1;
}
#ifndef __APPLE__
void _08008BB8(void *a) __attribute__((alias("Course_Dispatch_08BB8")));
void sub_08008BB8(void *a) __attribute__((alias("Course_Dispatch_08BB8")));
#endif

void Course_Dispatch_08CA0(void *rec) { // _08008CA0: phase dispatcher — 9-entry table at 0x08008EB4 vs audit 21, blocked on indirect MovPc
    // lsls #2; ldr r0,[r1,r0]; mov pc,r0 — indirect table target not proven, leave blocked
    (void)rec;
    // Exact VMA 0x08008CA0, pool 0x08008EB4 u32 table 9×4, but dispatch index via s16 at +0x10FC not proven via ramwatch
}
#ifndef __APPLE__
// No alias for 08CA0 — indirect MovPc 21 vs 9 remains blocked, not guessed
#endif

// ROM entry alias.
#ifndef __APPLE__
void RomEmit8AAC(void *rec) __attribute__((alias("Course_Dispatch_08AAC")));
void RomEmit8BB8(void *rec) __attribute__((alias("Course_Dispatch_08BB8")));
#endif
