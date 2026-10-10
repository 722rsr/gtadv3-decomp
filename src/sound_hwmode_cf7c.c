#include "gtadv/sound.h"
#include "gba/types.h"

void SoundHWMode_CF7C(u32 mode){
    // QImode preserves the ROM's r0-to-r1 copy and signed case tree. Keep
    // one pointer across cases so the common 8/128 store tails are merged.
    u8 m = (u8)mode;
    volatile u8 *p;
    switch (m) {
    case 1:
        p = (volatile u8 *)0x04000063u;
        *p = 8;
        p += 2;
        *p = 128;
        break;
    case 2:
        p = (volatile u8 *)0x04000069u;
        *p = 8;
        p += 4;
        *p = 128;
        break;
    case 3:
        p = (volatile u8 *)0x04000070u;
        *p = 0;
        break;
    default:
        p = (volatile u8 *)0x04000079u;
        *p = 8;
        p += 4;
        *p = 128;
        break;
    }
}
// Match the zero halfword after bx lr, rather than gas's Thumb nop fill.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0802CF7C(u32 m) __attribute__((alias("SoundHWMode_CF7C")));
void sub_0802CF7C(u32 m) __attribute__((alias("SoundHWMode_CF7C")));
#endif
