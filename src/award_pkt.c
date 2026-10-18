#include "gtadv/award.h"
#include "gtadv/ai_grid.h"
#include "gtadv/memory.h"
#include "gba/types.h"

// Reference: asm/award_pkt.s 0x0800B0BC-0x0800B190.
// Builds a 56-byte type-10 packet and sends it to the ROM packet consumer.

__attribute__((weak)) void UiPacket_Consume(void *pkt){ (void)pkt; } // _080188B0 — weak stub

extern int sub_08025CF4(int type, int row, int col);
extern void sub_0800188B0(void *pkt);

void Award_BuildTierPacket(void)
{
    u8 packet[56];
    register volatile u8 *copySource __asm__("r0");
    register volatile u8 *copyDestination __asm__("r1");
    register s32 count0 __asm__("r8");
    s32 count1;
    register s32 nextRow __asm__("r6");
    s32 row;
    s32 col;
    s32 layer;
    register u8 tier __asm__("r0");
    register volatile u8 *tierDestination __asm__("r1");

    {
        register volatile u8 *packetBase __asm__("r1") = packet;
        *(volatile u16 *)(packetBase + 0) = 10;
        __asm__ volatile ("" ::: "memory");
        {
            register volatile u8 *wa __asm__("r3") = (volatile u8 *)0x03001780;
            register volatile s16 *cursor __asm__("r2") =
                (volatile s16 *)(wa + 0x574);

            *(volatile u16 *)(packetBase + 24) = (u16)*(volatile u16 *)cursor;
            __asm__ volatile ("" ::: "memory");
            {
                register volatile u8 *fieldBase __asm__("r4") = packet;
                register s32 fieldIndex __asm__("r1");
                // This work-area value is RAM; plain s16 access emits the indexed ldrsh.
                fieldIndex = *(s16 *)cursor;
                *(volatile s16 *)(fieldBase + 26) =
                    (s8)*(volatile u8 *)(wa + fieldIndex * 12 + 48);
            }
            __asm__ volatile ("" ::: "memory");
            {
                register s32 copyIndex __asm__("r1");
                copyIndex = *(s16 *)cursor;
                copySource = (volatile u8 *)(copyIndex * 12 +
                                             (u32)(uintptr_t)wa);
                copyDestination = packet + 28;
                __asm__ volatile ("" : "+r" (copySource),
                                  "+r" (copyDestination));
                copySource += 48;
            }
        }
    }
    {
        struct PacketWords3 { u32 word[3]; };
        *(struct PacketWords3 *)(void *)copyDestination =
            *(const struct PacketWords3 *)(const void *)copySource;
    }
    __asm__ volatile ("" ::: "memory");

    {
        // The ROM builds zero in r0, then copies it into the r8 counter.
        register u32 zeroCount __asm__("r0") = 0;
        __asm__ volatile ("" : "+r" (zeroCount));
        count0 = zeroCount;
    }
    count1 = 0;
    for (row = 0; row <= 3; row = nextRow) {
        col = 0;
        nextRow = row + 1;
        while (col <= 10) {
            if ((u8)sub_08025CF4(0, row, col) == 3) {
                register s32 one __asm__("r1") = 1;
                __asm__ volatile ("" : "+r" (one));
                count0 += one;
            }
            if ((u8)sub_08025CF4(1, row, col) == 3)
                count1++;
            col++;
        }
    }

    {
        register volatile u8 *layerBase __asm__("r0") = (volatile u8 *)0x03001780;
        register u32 layerOffset __asm__("r2") = 0xFF2;
        __asm__ volatile ("" : "+r" (layerBase), "+r" (layerOffset));
        layerBase += layerOffset;
        layer = *(s16 *)layerBase;
        // Keep the r2 offset live and retain the ROM's r3 zero-index register.
        __asm__ volatile ("" : "+r" (layerOffset) : : "r3", "memory");
    }

    if (layer != 0)
        goto tier_nonzero;
    {
        register s32 layerZeroCount __asm__("r4") = count0;
        __asm__ volatile ("" : "+r" (layerZeroCount));
        if (layerZeroCount > 43) {
            tierDestination = packet;
            __asm__ volatile ("" : "+r" (tierDestination));
            tier = 1;
            goto store_tier;
        }
    }
    packet[23] = (u8)layer;
    goto send_packet;
tier_nonzero:
    if (count1 > 43) {
        tierDestination = packet;
        __asm__ volatile ("" : "+r" (tierDestination));
        tier = 3;
        goto store_tier;
    }
    tierDestination = packet;
    __asm__ volatile ("" : "+r" (tierDestination));
    tier = 2;
store_tier:
    tierDestination[23] = tier;
send_packet:
    sub_0800188B0(packet);
}

void _0800B0BC(void) __attribute__((alias("Award_BuildTierPacket")));
void Sub_0800B0BC(void) __attribute__((alias("Award_BuildTierPacket")));
