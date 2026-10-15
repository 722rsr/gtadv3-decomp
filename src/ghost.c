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

// The alias is defined at the bottom of this TU; declare it here so
// `Ghost_RaiseFlag2` can call the `_08018AA8` spelling that asm/ghost.s
// actually defines at 0x08018AA8 (the friendly `Ghost_FlagOp` name carries
// no VMA, so the screen could not resolve it).
void _08018AA8(u32 m, int s);
void Ghost_RaiseFlag2(void) {
    // The `_08018AA8` alias is ARM-only (declared above under the same
    // guard); the host build calls the real body by its friendly name.
    _08018AA8(2, 1);
}
__asm__(".align 2, 0");

void Ghost_FlagOp(u32 mask, int set) {
    if (set) {
        u32 ctx = *(volatile u32 *)0x03004E20;
        volatile u32 *fp = (volatile u32 *)(ctx + 60);
        u32 v = *fp;
        v |= mask;
        *fp = v;
    } else {
        u32 ctx = *(volatile u32 *)0x03004E20;
        volatile u32 *fp = (volatile u32 *)(ctx + 60);
        u32 v = *fp;
        v &= ~mask;
        *fp = v;
    }
}

u32 Ghost_FlagTest(u32 mask) {
    return *(volatile u32 *)(RaceCtx() + 60) & mask;
}

// Aliases for original BL targets
void *_08018A50(int idx) __attribute__((alias("Ghost_RacerAt")));
u8   _08018A6C(s16 idx) __attribute__((alias("Ghost_CarRecordField6")));
u8   sub_08018A6C(s16 idx) __attribute__((alias("Ghost_CarRecordField6")));
void _08018A88(u32 v) __attribute__((alias("Ghost_SetRaceCtxU16_74")));
void _08018A98(void) __attribute__((alias("Ghost_RaiseFlag2")));
void _08018AA8(u32 m, int s) __attribute__((alias("Ghost_FlagOp")));
u32  _08018ACC(u32 m) __attribute__((alias("Ghost_FlagTest")));
