#include "gtadv/sound.h"
#include "gba/types.h"


void SoundHWMode_CF7C(u32 mode){
    u32 m = (mode << 24) >> 24; // lsls #24 / lsrs #24 u8 at 0x02CF7C/0x02CF7E
    if(m == 2){
        // beq at 0x02CF82 -> 0x02CFA4: ldr r1,=0x04000069 @0x02CFA8 pool, b 0x02CFBA -> movs #8 strb [r1] (u8), adds #4, movs #128 strb [r1] (u8), bx lr
        volatile u8 *r1 = (volatile u8*)(uintptr_t)*(volatile u32*)0x0802CFA8u;
        *r1 = 8;
        r1 += 4;
        *r1 = 128;
        return;
    } else if(m == 3){
        // bgt at 0x02CF86 -> 0x02CF90, cmp #3 beq at 0x02CF90 -> 0x02CFAC: ldr r1,=0x04000070 @0x02CFB4 pool, movs #0, b 0x02CFC2 -> strb [r1] (u8), bx lr
        volatile u8 *r1 = (volatile u8*)(uintptr_t)*(volatile u32*)0x0802CFB4u;
        *r1 = 0;
        return;
    } else if(m == 1){
        // cmp #1 beq at 0x02CF90 -> 0x02CF96: ldr r1,=0x04000063 @0x02CFA0 pool, movs #8 strb [r1] (u8), adds #2, b 0x02CFC0 -> movs #128 strb [r1] (u8)
        volatile u8 *r1 = (volatile u8*)(uintptr_t)*(volatile u32*)0x0802CFA0u;
        *r1 = 8;
        r1 += 2;
        *r1 = 128;
        return;
    } else {
        // else at 0x02CFB8: ldr r1,=0x04000079 @0x02CFC8 pool, b 0x02CFBA -> same 8/128 as m==2 but base 0x04000079
        volatile u8 *r1 = (volatile u8*)(uintptr_t)*(volatile u32*)0x0802CFC8u;
        *r1 = 8;
        r1 += 4;
        *r1 = 128;
        return;
    }
}
#ifndef __APPLE__
void _0802CF7C(u32 m) __attribute__((alias("SoundHWMode_CF7C")));
void sub_0802CF7C(u32 m) __attribute__((alias("SoundHWMode_CF7C")));
#endif
