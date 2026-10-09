#include "gtadv/sound.h"
#include "gba/types.h"


void SoundHWMode_CF7C(u32 mode){
    u32 m = (mode << 24) >> 24; // lsls #24 / lsrs #24 u8 at 0x02CF7C/0x02CF7E
    register u32 c asm("r1") = m; // adds r1,r0,#0 at 0x02CF80; re-checked as cmp r1,#3 at 0x02CF90
    (void)c;
    switch (m) {
    case 1: {
        volatile u8 *r1 = (volatile u8 *)0x04000063u;
        *r1 = 8;
        r1 += 2;
        *r1 = 128;
        break;
    }
    case 2: {
        volatile u8 *r1 = (volatile u8 *)0x04000069u;
        *r1 = 8;
        r1 += 4;
        *r1 = 128;
        break;
    }
    case 3: {
        volatile u8 *r1 = (volatile u8 *)0x04000070u;
        *r1 = 0;
        break;
    }
    default: {
        volatile u8 *r1 = (volatile u8 *)0x04000079u;
        *r1 = 8;
        r1 += 4;
        *r1 = 128;
        break;
    }
    }
}
#ifndef __APPLE__
void _0802CF7C(u32 m) __attribute__((alias("SoundHWMode_CF7C")));
void sub_0802CF7C(u32 m) __attribute__((alias("SoundHWMode_CF7C")));
#endif
