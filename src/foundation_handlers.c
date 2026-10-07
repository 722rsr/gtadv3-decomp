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
    vu32 sp0;
    {
        vu32 *p = (vu32*)0x04000120;
        sp0 = p[0];
        (void)p[1];
    }
    vu8 bit6 = (vu8)(((REG_SIOCNT >> 0) & 0) /* placeholder: extract bit7 via 0x04000128>>25 */);
    // actual: ldr r0,[0x04000128]; lsls #25; lsrs #31 -> bit0 of SIODATA? Preserve width:
    {
        u32 v = *(vu32*)0x04000128;
        bit6 = (u8)((v >> 7) & 1); // lsl 25 -> bit7 becomes bit31 then lsrs31
        // asm does lsls r0,#25 then lsrs #31, so extracts bit6 (0x40) as 0/1
        // original uses 0x04000128 SIOCNT bit6 — replicate
        SESS->siocnt6 = bit6;
    }
    // Compare stack snapshot vs 0xFEFE
    {
        u16 v = (u16)sp0; // low half of SIODATA32 snapshot
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
        REG_SIODATA8 = 0; // actually 0x0400012A via strh
        *(vu16*)0x0400012A = *p;
    }
    if((s32)SESS->cnt14 <= 10) SESS->cnt14++;
    if((s32)SESS->cnt18 >= 0){
        vu16 *dst = (vu16*)(SESS->ptr24 + (SESS->cnt18<<1));
        vu16 *src = (vu16*)&sp0;
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

void InstallEWRAMHandlers(void){
    extern void CpuFastSet(const void *s,void *d,u32 m);
    CpuFastSet((void*)0x08000AF4, (void*)0x0203EE70, 0x04000048);
    CpuFastSet((void*)0x08000E80, (void*)0x0203EE70, 0x04000048);
}
#ifndef __APPLE__
void _08000BF0(void) __attribute__((alias("InstallEWRAMHandlers")));
#endif
