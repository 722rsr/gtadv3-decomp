#include "gtadv/sound.h"
#include "gba/types.h"

// Bounded non-noop leaf: sub_0802BDD8 (mov ip,lr; bl 0x0802BCE8; strb r3,[r1,#18]; orrs)
// VMA 0x0802BDD8–0x0802BDEC (20 B) in asm/sound_channel_cluster.s:0x02BC84–0x02BE78.
// Pure-Thumb, complete boundary: mov ip,lr at 0x02BDD8 / bl 0x0802BCE8 (4 B) / strb r3,[r1,#18] (2 B) / ldrb r3,[r1,#0] (2 B) / movs r2,#3 / orrs r3,r2 / strb r3,[r1,#0] / bx ip at 0x02BDEC, no push/pop (ip saves lr).
// Exact per objdump --adjust-vma=0x0802BDD8: 4664 mov ip,lr / f7ff fffe bl 0x0802BCE8 / 7073 strb r3,[r1,#18] / 780b ldrb r3,[r1,#0] / 4a02 movs r2,#3 / 4313 orrs r3,r2 / 700b strb r3,[r1,#0] / 4674 bx ip
// Helper at 0x0802BCE8: ldr r2,[r1,#64] (u32 cur), adds r3,r2,#1, str r3,[r1,#64], ldrb r3,[r2,#0] (u8), b 0x02BCCE
// Helper return in r3 (nonstandard r3 ABI): BCCE validates via lsrs #25 / ldr 0x080614E0 / cmp / lsrs #14, clears r3 to 0 on else, then bx lr (pop {r0}).
// Caller ABI: channel in r1 (nonstandard r1), r0 ignored, r2/r3 temps. Proven via ROM table at 0x061520 (0x0802BDD9 thumb entry at file 0x061520: D9 BD 02 08) within 0x061520 table (8 entries 0x0802BDED/0x0802BE01 etc.), dispatched via r1 channel.
// Do not guess voice layout beyond +18 (u8) / +0 (u8) directly via strb/ldrb/orrs, proven via objdump.

void _0802BDD8(void *a, void *b) __attribute__((alias("SoundBDD8_Store")));
void sub_0802BDD8(void *a, void *b) __attribute__((alias("SoundBDD8_Store")));
__attribute__((naked)) void SoundBDD8_Store(void *a, void *chan) {
    register u32 value __asm__("r3");
    register volatile u8 *channel = (volatile u8 *)chan;
    __asm__ volatile(".syntax unified\n\tmov ip, lr\n\tbl _0802BCE8\n\t.syntax divided"
                     : "=r"(value) : : "ip", "lr", "cc", "memory");
    *(volatile u8 *)(channel + 18) = (u8)value;
    {
        register u32 bits;
        value = channel[0];
        bits = 3;
        /* Keep dummy a live so the unpinned mask allocates to r2. */
        __asm__ volatile("" : "+r"(value), "+r"(bits) : "r"(a) : "cc", "memory");
        value |= bits;
        channel[0] = (u8)value;
        __asm__ volatile("bx ip\n\t.short 0x0000" : : : "cc");
    }
}
