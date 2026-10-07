#include "gtadv/foundation.h"
#include "gba/types.h"
#include "gba/regs.h"

// VBlank soft-IRQ kicker — asm/softirq.s 0x08000A60 (84B, proven via objdump)
// EWRAM 0x0203EF90 layout per asm/agbmain.s
typedef struct { vu8 pend; vu8 fsm; vu8 _02; vu8 _03; vu8 ready; vu8 _05; vu8 flags; vu8 siocnt6; vu8 _08; vu8 latch; vu8 _0A[10]; vu32 cnt14; vu32 cnt18; vu32 ptr1C; vu32 ptr20; vu32 ptr24; vu32 ptr28; } SoftState;
#define ST ((SoftState*)0x0203EF90)

void SoftIrqKicker(void){
    if(ST->pend==0){
        if(ST->latch==0){
            REG_IME = 0;
            *(vu16*)0x03007FF8 |= 0x80;
            REG_IME = 1;
        }
        ST->latch = 0;
        return;
    }
    if(ST->fsm==0) return;
    if((ST->flags & 0x80)==0) return;
    ST->cnt18 = 0xFFFFFFFFu;
    {
        u32 a = ST->ptr28, b = ST->ptr24;
        ST->ptr28 = b; ST->ptr24 = a;
    }
    if(ST->ready){
        u32 a = ST->ptr20, b = ST->ptr1C;
        ST->ptr20 = b; ST->ptr1C = a;
        ST->ready = 0;
        ST->cnt14 = 0;
    }
    {
        u32 v = *(vu32*)0x04000128;
        ST->siocnt6 = (u8)((v>>6)&1);
    }
    *(vu16*)0x0400012A = 0xFEFE;
    REG_SIOCNT |= 0x80;
    REG_TM3CNT_H = 0xC0;
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
