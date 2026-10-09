#include "gtadv/foundation.h"
#include "gba/types.h"
#include "gba/regs.h"
#include "gba/bios.h"

// EWRAM serial handlers — ROM 0x08000AF4/0x08000E80 copied to 0x0203EE70 via CpuFastSet 72 words (0x04000048)
// Executed via IntrMain slot0 combo 0xC0 (Timer3+Serial). Behavioral C mirrors asm/handlers.s

// State at 0x0203EF90 (see asm/agbmain.s / asm/handlers.s)
typedef struct { vu8 _00; vu8 fsm; vu8 _02; vu8 _03; vu8 ready; vu8 c5; vu8 flags; vu8 siocnt6; vu8 _08; vu8 latch9; vu8 _0A[4]; vu32 cnt14; vu32 cnt18; vu32 ptr1C; vu32 ptr20; vu32 ptr24; vu32 ptr28; } EwramSess;
#define SESS ((EwramSess*)0x0203EF90)

// Blob A — _08000AF4 (MultiSio 32-bit burst, timer3+serial)
void EWRAM_Handler_BlobA(void){
    u32 snapshot[2];
    {
        volatile u32 *p = (volatile u32 *)(uintptr_t)0x04000120;
        snapshot[1] = p[1];
        snapshot[0] = p[0];
    }
    vu8 bit6;
    {
        u32 v = *(volatile u32 *)(uintptr_t)0x04000128;
        bit6 = (u8)((v >> 6) & 1); // lsl 25 then lsr 31 extracts SIOCNT bit 6
        SESS->siocnt6 = bit6;
    }
    // Compare stack snapshot vs 0xFEFE
    {
        u16 v = (u16)snapshot[0]; // low half of SIODATA32 snapshot
        if(v == 0xFEFE){
            if((s32)SESS->cnt18 > 9){
                SESS->cnt18 = 0xFFFFFFFFu;
                vu32 a = SESS->ptr28, b = SESS->ptr24;
                SESS->ptr28 = b; SESS->ptr24 = a;
                if(SESS->ready){
                    vu32 c = SESS->ptr20, d = SESS->ptr1C;
                    SESS->ptr20 = d; SESS->ptr1C = c;
                    SESS->ready = 0;
                    SESS->cnt14 = 0;
                }
                REG_IME = 0;
                *(vu16*)0x03007FF8 |= 0x80;
                REG_IME = 1;
            }
        }
    }
    if((s32)SESS->cnt14 <= 9){
        vu16 *p = (vu16*)(SESS->ptr20 + (SESS->cnt14<<1));
        *(vu16*)0x0400012A = *p;
    }
    if((s32)SESS->cnt14 <= 10) SESS->cnt14++;
    if((s32)SESS->cnt18 >= 0){
        vu16 *dst = (vu16*)(SESS->ptr24 + (SESS->cnt18<<1));
        vu16 *src = (vu16*)snapshot;
        for(int i=0;i<4;i++) dst[i*12] = src[i];
        if(SESS->cnt18==9) SESS->c5 = 1;
    }
    if((s32)SESS->cnt18 <= 10) SESS->cnt18++;
    if(SESS->_00) REG_TM3CNT_H = 0;
    if((s32)SESS->cnt14 <= 10 && SESS->_00){
        REG_SIOCNT |= 0x80;
        REG_TM3CNT_H = 0xC0;
    }
    SESS->latch9 = 1;
}
#ifndef __APPLE__
void _08000AF4(void) __attribute__((alias("EWRAM_Handler_BlobA")));
#endif

// Blob B — _08000E80 (SIO-normal polling, 0x0203F150 state)
typedef struct { vu8 f00; vu8 _01[3]; vu32 p04; vu32 _08; vu32 cnt08; vu32 _0C; } EwramF150;
#define F150 ((EwramF150*)0x0203F150)
void EWRAM_Handler_BlobB(void){
    u32 siodata = REG_SIODATA32;
    vu8 mode = F150->f00;
    if(mode==1){
        REG_TM3CNT_H = 0;
        if((s32)F150->cnt08 < 0){
            *(vu32*)0x04000120 = 0xFEFEFEFE;
        } else if((s32)F150->cnt08 <= 0x1FFF){
            *(vu32*)0x04000120 = *(vu32*)(F150->p04 + (F150->cnt08<<2));
        } else {
            *(vu32*)0x04000120 = F150->_0C;
        }
    } else {
        REG_SIOCNT |= 0x80;
        if((s32)F150->cnt08 < 0){
            if(siodata != 0xFEFEFEFE) F150->cnt08--;
        } else {
            if((s32)F150->cnt08 <= 0x1FFF) *(vu32*)(F150->p04 + (F150->cnt08<<2)) = siodata;
            else F150->_0C = siodata;
        }
    }
    s32 c = (s32)F150->cnt08;
    if(c <= 0x2002){
        F150->cnt08 = c+1;
        if(mode==1){
            REG_SIOCNT |= 0x80;
            REG_TM3CNT_H = 0xC0;
        }
    }
}
#ifndef __APPLE__
void _08000E80(void) __attribute__((alias("EWRAM_Handler_BlobB")));
#endif

void InstallEWRAMHandlers(int checksumMode, u32 inputBase){
    volatile u16 *ime = (volatile u16 *)(uintptr_t)0x04000208;
    volatile u16 *ie = (volatile u16 *)(uintptr_t)0x04000200;
    volatile u16 *siocnt = (volatile u16 *)(uintptr_t)0x04000128;
    volatile u32 *f150 = (volatile u32 *)(uintptr_t)0x0203F150;
    u32 zero = 0;
    u32 sum = 0;

    extern void _0802D974(const void *src, void *dst, u32 mode);

    *ime = 0;
    *ie = (u16)(*ie & 0xFF3Fu);
    *ime = 1;
    *(volatile u16 *)(uintptr_t)0x04000134 = 0;
    *siocnt = 0x2000;
    *siocnt = (u16)(*siocnt | 0x4003u);

    _0802D974(&zero, (void *)(uintptr_t)0x0203F150, 0x05000006u);
    _0802D974((const void *)(uintptr_t)0x08000E81,
              (void *)(uintptr_t)0x0203EE70, 0x04000048u);
    f150[1] = inputBase;
    f150[2] = 0xFFFFFFFFu;
    *(volatile u32 *)(uintptr_t)0x04000128 = 0x2003u;

    if (checksumMode != 0) {
        volatile u32 *words = (volatile u32 *)(uintptr_t)inputBase;
        *(volatile u32 *)(uintptr_t)0x0400010C = 0;
        *(volatile u8 *)(uintptr_t)0x0203F150 = 1;
        for (u32 i = 0; i < 0x20000u; i++)
            sum += words[i];
        *(volatile u32 *)(uintptr_t)0x0203F15C = ~sum;
        *siocnt = 0x1000;
        *siocnt = 0x1001;
    }
}
#ifndef __APPLE__
void _08000BF0(int checksumMode, u32 inputBase)
    __attribute__((alias("InstallEWRAMHandlers")));
#endif
