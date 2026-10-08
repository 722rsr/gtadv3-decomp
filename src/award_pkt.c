#include "gtadv/award.h"
#include "gtadv/ai_grid.h"
#include "gtadv/memory.h"
#include "gba/types.h"

// Reference: asm/award_pkt.s 0x0800B0BC-0x0800B190
// 56-byte stack packet -> _080188B0

__attribute__((weak)) void UiPacket_Consume(void *pkt){ (void)pkt; } // _080188B0 — weak stub

void Award_BuildTierPacket(void) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    /* No `{0}`: the ROM reserves 56 bytes (`sub sp,#56`) but never zeroes
       them — only the fields below are written. Zero-init pulls in a
       `memset` the ROM has no `bl` for (UNRESOLVED) and adds 22 bytes. */
    u8 pkt[56];
    typedef struct { u32 w[3]; } rec12;
    // pkt[0]=10
    *(u16 *)(pkt+0) = 10;
    // cursor idx = u16[wa+0x574]
    u16 cursor = *(volatile u16 *)(wa + 0x574);
    *(u16 *)(pkt+24) = cursor;
    // field[0] signed byte
    s16 idx = *(volatile s16 *)(wa + 0x574);
    int off = idx * 12; // idx*12
    s8 field0 = *(volatile s8 *)(wa + 0x30 + off);
    *(s16 *)(pkt+26) = (s16)field0;
    /* 12-byte block copy (`ldmia`/`stmia`), not a byte loop: the ROM moves
       three words at once. A byte loop costs 14 extra bytes (OVERSIZED). */
    *(rec12 *)(pkt + 28) = *(volatile rec12 *)(wa + 0x30 + off);

    int cnt0=0,cnt1=0;
    for (int r=0;r<=3;r++) for (int c=0;c<=10;c++) {
        if ((Ai_GridGet(0,r,c)&0xFF)==3) cnt0++;
        if ((Ai_GridGet(1,r,c)&0xFF)==3) cnt1++;
    }
    s16 layer = *(volatile s16 *)(wa + 0xFF2);
    u8 tier;
    if (layer==0) tier = (cnt0>43)?1:0;
    else          tier = (cnt1>43)?3:2;
    pkt[23]=tier;
    UiPacket_Consume(pkt);
}

#ifndef __APPLE__
void _0800B0BC(void) __attribute__((alias("Award_BuildTierPacket")));
void Sub_0800B0BC(void) __attribute__((alias("Award_BuildTierPacket")));
#endif
