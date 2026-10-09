#include "gtadv/foundation.h"
#include "gba/types.h"
#include "gba/regs.h"

// VBlank soft-IRQ kicker — asm/softirq.s 0x08000A60 (84B, proven via objdump)
// EWRAM 0x0203EF90 layout per asm/agbmain.s
typedef struct { u8 pend; u8 fsm; u8 _02; u8 _03; u8 ready; u8 _05; u8 flags; u8 siocnt6; u8 _08; u8 latch; u8 _0A[10]; u32 cnt14; u32 cnt18; u32 ptr1C; u32 ptr20; u32 ptr24; u32 ptr28; } SoftState;
#define ST ((SoftState*)0x0203EF90)

void SoftIrqKicker(void){
    if(ST->pend != 0){
        if(ST->fsm != 0){
            register u32 flagMask __asm__("r0") = 0x80;
            flagMask &= ST->flags;
            if(flagMask != 0){
            register u32 minusOne __asm__("r0") = 1;
            minusOne = 0 - minusOne;
            ST->cnt18 = minusOne;
            {
                register u32 a __asm__("r1") = ST->ptr28;
                register u32 b __asm__("r0") = ST->ptr24;
                ST->ptr28 = b;
                ST->ptr24 = a;
            }
            if(ST->ready){
                register u32 a __asm__("r1") = ST->ptr20;
                register u32 b __asm__("r0") = ST->ptr1C;
                ST->ptr20 = b;
                ST->ptr1C = a;
                ST->ready = 0;
                ST->cnt14 = 0;
            }
            {
                register volatile u16 *sio __asm__("r2") =
                    (volatile u16 *)(uintptr_t)0x04000128;
                u32 v = *(volatile u32 *)(uintptr_t)sio;
                ST->siocnt6 = (u8)((v << 25) >> 31);
                register u32 fill __asm__("r0") = 0xFEFE;
                *(sio + 1) = fill;
                *sio = (u16)(*sio | 0x80u);
            }
            REG_TM3CNT_H = 0xC0;
            }
        }
    } else {
        if(ST->latch == 0){
            REG_IME = 0;
            *(vu16*)0x03007FF8 |= 0x80;
            REG_IME = 1;
        }
        ST->latch = 0;
    }
}
#ifndef __APPLE__
void _08000A60(void) __attribute__((alias("SoftIrqKicker")));
// The closure spells 0x08000A60 ONLY `sub_08000A60` (asm/softirq.s:28) -- the
// `SoftIrqPump` alias this line used to carry has no VMA in its spelling, so
// tools/promotion_screen.py could not tell which ROM address the boot.s call
// reaches and blocked _08000268. A `sub_` name in a C TU is a local `t` there
// too, so the screen is satisfied either way -- which is exactly why the
// spelling has to be DEFINED here, in the owning TU, and not merely used at
// the call site. Aliased to the REAL BODY (SoftIrqKicker, above) in one hop:
// gcc emits `.thumb_set sub_08000A60, _08000A60` for an alias-of-an-alias and
// the slice link splices only the body's own section.
void sub_08000A60(void) __attribute__((alias("SoftIrqKicker")));
#endif
