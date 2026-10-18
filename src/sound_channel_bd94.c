#include "gtadv/sound.h"
#include "gba/types.h"

// Bounded non-noop leaf: sub_0802BD94 (mov ip,lr; bl 0x0802BCE8; strb r3,[r1,#10]; orrs)
// VMA 0x0802BD94–0x0802BDA8 (20 B) in asm/sound_channel_cluster.s:0x02BC84–0x02BE78.
// Pure-Thumb, complete boundary: mov ip,lr at 0x02BD94 / bl 0x0802BCE8 (4 B) / strb r3,[r1,#10] (2 B) / ldrb r3,[r1,#0] (2 B) / movs r2,#12 / orrs r3,r2 / strb r3,[r1,#0] / bx ip at 0x02BD9E, no push/pop (ip saves lr).
// Exact per objdump --adjust-vma=0x0802BD94: 4664 mov ip,lr / f7ff fffe bl 0x0802BCE8 / 7073 strb r3,[r1,#10] / 780b ldrb r3,[r1,#0] / 4a02 movs r2,#12 / 4313 orrs r3,r2 / 700b strb r3,[r1,#0] / 4674 bx ip
// Helper at 0x0802BCE8: ldr r2,[r1,#64] (u32 cur), adds r3,r2,#1, str r3,[r1,#64], ldrb r3,[r2,#0] (u8), b 0x02BCCE
// Helper return in r3 (nonstandard r3 ABI): BCCE validates via lsrs #25 / ldr 0x080614E0 / cmp / lsrs #14, clears r3 to 0 on else, then bx lr (pop {r0}).
// Caller ABI: channel in r1 (nonstandard r1), r0 ignored, r2/r3 temps. Proven via ROM table at 0x061504 (0x0802BD95 thumb entry at file 0x061506: 95 BD 02 08) within 0x061504 table (4 entries 0x0802BD75/0x0802BD81/0x0802BD95/0x0802BDA9), dispatched via r1 channel.
// Do not guess voice layout beyond +10 (u8) / +0 (u8) directly via strb/ldrb/orrs, proven via objdump.

void _0802BD94(void *a, void *b) __attribute__((alias("SoundBD94_Store")));
void sub_0802BD94(void *a, void *b) __attribute__((alias("SoundBD94_Store")));
__attribute__((naked)) void SoundBD94_Store(void *a, void *chan) {
    register u32 frame __asm__("ip");
    register u32 value __asm__("r3");
    register volatile u8 *channel __asm__("r1") = (volatile u8 *)chan;
    __asm__ volatile(".syntax unified\n\tmov %0, lr\n\tbl _0802BCE8\n\t.syntax divided"
                     : "=r"(frame), "=r"(value) : : "lr", "cc", "memory");
    *(volatile u8 *)(channel + 10) = (u8)value;
    register u32 flags __asm__("r3") = channel[0];
    register u32 mask __asm__("r2") = 12;
    __asm__ volatile("" : "+r"(flags), "+r"(mask) : : "cc", "memory");
    flags |= mask;
    channel[0] = (u8)flags;
    __asm__ volatile("bx %0\n\t.short 0x0000" : : "r"(frame) : "cc");
}
