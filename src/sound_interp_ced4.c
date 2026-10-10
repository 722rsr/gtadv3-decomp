#include "gtadv/sound.h"
#include "gba/types.h"

// Reference: asm/sound_interp.s, 0x0802CED4-0x0802CF7C.
// The mode-4 table returns an un-biased byte. Other modes decode adjacent
// packed pitch values (low nibble: s16 table index; high nibble: shift),
// interpolate with an 8-bit fraction, and add the 0x800 hardware bias.
// These are data-table addresses, not loads from the function's own code.
#ifdef __APPLE__
#define INTERP_REG(type, name, reg) type name
#else
#define INTERP_REG(type, name, reg) register type name __asm__(reg)
#endif

s32 SoundInterp_CED4(u32 a, u32 b, u32 c){
    u32 av = (u8)a;
    INTERP_REG(u32, bv, "r5") = (u8)b;
    INTERP_REG(s32, cv, "ip") = (u8)c;
    INTERP_REG(u8 *, tbl, "r3");
    INTERP_REG(s16 *, levels, "r4");
    INTERP_REG(s32, lo, "r6");
    INTERP_REG(s32, mask, "r2");
    INTERP_REG(s32, t, "r0");
    INTERP_REG(s32, v, "r1");
    if (av == 4) {
        if (bv <= 20) bv = 0;
        else {
            t = bv;
            t -= 21;
            bv = (u8)t;
            if (bv > 59) bv = 59;
        }
        // Keep the pool load in r0 before adding the r5 table index.
        t = 0x08061708;
        __asm__("" : "+r"(t));
        t = bv + t;
        return *(u8 *)(uintptr_t)t;
    }
    if (bv <= 35) {
        t = 0;
        cv = t;
        bv = 0;
    } else {
        t = bv;
        t -= 36;
        bv = (u8)t;
        if (bv > 130) {
            bv = 130;
            v = 255;
            cv = v;
        }
    }
    tbl = (u8 *)0x0806166C;
    t = bv + (uintptr_t)tbl;
    lo = *(u8 *)(uintptr_t)t;
    levels = (s16 *)0x080616F0;
    mask = 15;
    t = lo;
    t &= mask;
    t <<= 1;
    t += (uintptr_t)levels;
    v = *(s16 *)(uintptr_t)t;
    // Keep signed word-mode shifts after the byte loads (asrs, not lsrs).
    __asm__("" : "+r"(lo));
    t = lo >> 4;
    lo = v;
    lo >>= t;
    t = bv + 1;
    t += (uintptr_t)tbl;
    v = *(u8 *)(uintptr_t)t;
    t = v;
    t &= mask;
    t <<= 1;
    t += (uintptr_t)levels;
    t = *(s16 *)(uintptr_t)t;
    __asm__("" : "+r"(v));
    v >>= 4;
    t >>= v;
    t -= lo;
    return lo + ((t * cv) >> 8) + 0x800;
}
#ifndef __APPLE__
s32 _0802CED4(u32 a, u32 b, u32 c) __attribute__((alias("SoundInterp_CED4")));
s32 sub_0802CED4(u32 a, u32 b, u32 c) __attribute__((alias("SoundInterp_CED4")));
#endif
