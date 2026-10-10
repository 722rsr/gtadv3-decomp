#include "gba/types.h"

// Closure targets from the six-entry garage event table at 0x08028188.
// These entries pass the record in r0 and the preserved selector in r1;
// several callees ignore that second machine argument.
extern void sub_08002804C(void *rec, int selector);
extern void sub_080028090(void *rec, int selector);
extern void _0800280AC(void *rec, int selector);
extern void _0800280C0(void *rec, int selector);
extern void sub_0800280D4(void *rec, int selector);
extern void sub_080028134(void *rec, int selector);
extern void sub_0800281F8(void *rec, int selector);
extern void sub_08002823C(void *rec, int selector);
extern void _080028250(void *rec, int selector);
extern void _080028258(void *rec, int selector);
extern void sub_080028260(void *rec, int selector);
extern void sub_0800282D0(void *rec, int selector);
extern void sub_080028730(void *rec, int selector);
extern void sub_080028734(void *rec);
extern void sub_080028740(void *rec);
extern void sub_080028828(void *rec);
extern void sub_080028834(void *rec);
extern void sub_080028888(void *rec, int p1, int p2);
extern void sub_0800288A0(void);
extern void sub_080028D9C(void *state);
extern void sub_0800291F4(void *state);
extern void sub_080028F40(void *state);
extern void sub_080028F14(int p1, int p2);
extern void sub_080029044(void *state);
extern void sub_080028EB0(void *state);
extern void sub_080029230(void);
extern void sub_080029208(void *state);
extern void sub_080029224(void *state);
extern void sub_080029218(void *state, int p1);

void GarageDispatch_28188(u32 selector, u32 value, u32 unused, void *rec) {
    (void)unused;
    switch (selector) {
    case 13: sub_08002804C(rec, (int)value); break;
    case 14: sub_080028090(rec, (int)value); break;
    case 15: _0800280AC(rec, (int)value); break;
    case 16: _0800280C0(rec, (int)value); break;
    case 17: sub_0800280D4(rec, (int)value); break;
    case 18: sub_080028134(rec, (int)value); break;
    default: break;
    }
}
// The ROM uses a zero halfword before the next 4-byte entry.
__asm__(".align 2, 0");

#ifndef __APPLE__
void _08028188(u32 selector, u32 value, u32 unused, void *rec)
    __attribute__((alias("GarageDispatch_28188")));
#endif

void GarageDispatch_2832C(u32 selector, u32 value, u32 unused, void *rec) {
    (void)unused;
    switch (selector) {
    case 13: sub_0800281F8(rec, (int)value); break;
    case 14: sub_08002823C(rec, (int)value); break;
    case 15: _080028250(rec, (int)value); break;
    case 16: _080028258(rec, (int)value); break;
    case 17: sub_080028260(rec, (int)value); break;
    case 18: sub_0800282D0(rec, (int)value); break;
    default: break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0802832C(u32 selector, u32 value, u32 unused, void *rec)
    __attribute__((alias("GarageDispatch_2832C")));
#endif

void GarageDispatch_288BC(u32 event, u32 p1, u32 p2, void *rec) {
    switch (event) {
        // Keep the ROM's block order: 2, 1, 6, 5, 7, 9, 12.
        case 2: sub_080028730(rec, (int)p1); break;
        case 1: sub_080028734(rec); break;
        case 6: sub_080028888(rec, (int)(u16)p1, (u16)p2); break;
        case 5: sub_080028828(rec); break;
        case 7: sub_080028834(rec); break;
        case 9: sub_080028740(rec); break;
        case 12: sub_0800288A0(); break;
        default: break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080288BC(u32 event, u32 p1, u32 p2, void *rec)
    __attribute__((alias("GarageDispatch_288BC")));
#endif

void GarageDispatch_29284(u32 event, u32 p1, u32 p2) {
    register u32 saved_p1 asm("r3") = p1;
    register u32 saved_p2 asm("r4") = p2;
    register void *state asm("r2") = (void *)0x030035D0u;
    switch (event) {
        // Keep the ROM's block order: 1, 4, 6, 5, 7, 8, 13, 21, 19, 12.
        case 1: sub_080028D9C(state); break;
        case 4: sub_0800291F4(state); break;
        case 6: sub_080028F14((int)(u16)saved_p1, (int)(u16)saved_p2); break;
        case 5: sub_080028F40(state); break;
        case 7: sub_080029044(state); break;
        case 8: sub_080028EB0(state); break;
        case 13: sub_080029208(state); break;
        case 21: sub_080029218(state, (int)(u16)saved_p1); break;
        case 19: sub_080029224(state); break;
        case 12: sub_080029230(); break;
        default: break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08029284(u32 event, u32 p1, u32 p2)
    __attribute__((alias("GarageDispatch_29284")));
#endif
