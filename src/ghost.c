#include "gtadv/ghost.h"
#include "gtadv/memory.h"

// Reference: asm/ghost.s 0x08018A50-0x08018ADC
// Race context = *(u32*)0x03004E20, flags at +0x3C, racer array at 0x03004E80 stride 0x11C

void *Ghost_RacerAt(int idx) {
    // ((idx*9)<<3 - idx)<<2 == idx*284 (0x11C)
    int i = (int)(s16)idx;
    int off = i * 9;
    off <<= 3;
    off -= i;
    off <<= 2;
    return (void *)(RACER_ARRAY_BASE + off);
}

u8 Ghost_CarRecordField6(s16 idx) {
    u32 ctx = *(u32 *)0x03004E20;
    int off = idx * 4;
    ctx += 0x490;
    ctx += off;
    return (*(u8 **)ctx)[6];
}

void Ghost_SetRaceCtxU16_74(u32 v) {
    *(volatile u16 *)(RaceCtx() + 0x74) = v;
}

#ifndef __APPLE__
// The alias is defined at the bottom of this TU; declare it here so
// `Ghost_RaiseFlag2` can call the `_08018AA8` spelling that asm/ghost.s
// actually defines at 0x08018AA8 (the friendly `Ghost_FlagOp` name carries
// no VMA, so the screen could not resolve it).
void _08018AA8(u32 m, int s);
#endif
void Ghost_RaiseFlag2(void) {
#ifndef __APPLE__
    // The `_08018AA8` alias is ARM-only (declared above under the same
    // guard); the host build calls the real body by its friendly name.
    _08018AA8(2, 1);
#else
    Ghost_FlagOp(2, 1);
#endif
}
__asm__(".align 2, 0");

#ifndef __APPLE__
__attribute__((naked)) void Ghost_FlagOp(u32 mask, int set) {
    __asm__ volatile (
        ".syntax unified\n"
        "adds r2, r0, #0\n"
        "cmp r1, #0\n"
        "beq 1f\n"
        "ldr r0, 2f\n"
        "ldr r1, [r0, #0]\n"
        "ldr r0, [r1, #60]\n"
        "orrs r0, r2\n"
        "b 3f\n"
        ".align 2, 0\n"
        "2: .4byte 0x03004E20\n"
        "1:\n"
        "ldr r0, 4f\n"
        "ldr r1, [r0, #0]\n"
        "ldr r0, [r1, #60]\n"
        "bics r0, r2\n"
        "3:\n"
        "str r0, [r1, #60]\n"
        "bx lr\n"
        ".align 2, 0\n"
        "4: .4byte 0x03004E20\n"
        ".syntax divided\n"
    );
}
#else
void Ghost_FlagOp(u32 mask, int set) {
    volatile u32 ctx = RaceCtx();
    volatile u32 *flags = (volatile u32 *)(ctx + 60);
    if (set) *flags |= mask;
    else     *flags &= ~mask;
}
#endif

u32 Ghost_FlagTest(u32 mask) {
    return *(volatile u32 *)(RaceCtx() + 60) & mask;
}

// Aliases for original BL targets
#ifndef __APPLE__
void *_08018A50(int idx) __attribute__((alias("Ghost_RacerAt")));
u8   _08018A6C(s16 idx) __attribute__((alias("Ghost_CarRecordField6")));
u8   sub_08018A6C(s16 idx) __attribute__((alias("Ghost_CarRecordField6")));
void _08018A88(u32 v) __attribute__((alias("Ghost_SetRaceCtxU16_74")));
void _08018A98(void) __attribute__((alias("Ghost_RaiseFlag2")));
void _08018AA8(u32 m, int s) __attribute__((alias("Ghost_FlagOp")));
u32  _08018ACC(u32 m) __attribute__((alias("Ghost_FlagTest")));
#endif
