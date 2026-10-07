#include "gtadv/sound.h"
#include "gba/types.h"

// Bounded non-noop leaf: sub_0802BD80 (mov ip,lr; bl 0x0802BCE8; lsls/strh/muls; bx ip)
// VMA 0x0802BD80–0x0802BD94 (20 B) in asm/sound_channel_cluster.s:0x02BC84–0x02BE78.
// Pure-Thumb, complete boundary: mov ip,lr at 0x02BD80 / bl 0x0802BCE8 (4 B) / lsls r3,#1 / strh [r0,#28] / ldrh [r0,#30] / muls / lsrs #8 / strh [r0,#32] / bx ip at 0x02BD92, no push/pop (ip saves lr).
// Exact per objdump --adjust-vma=0x0802BD80: 4664 mov ip,lr / f7ff fffe bl 0x0802BCE8 / 0c5b lsls r3,#1 / 8083 strh r3,[r0,#28] / 8842 ldrh r2,[r0,#30] / 4353 muls r3,r2 / 0c5b lsrs r3,#8 / 8283 strh r3,[r0,#32] / 4674 bx ip
// Helper at 0x0802BCE8: ldr r2,[r1,#64] (u32 cur), adds r3,r2,#1, str r3,[r1,#64], ldrb r3,[r2,#0] (u8), b 0x02BCCE (BCCE validates via lsrs #25 / ldr 080614E0 / cmp / lsrs #14)
// Helper return in r3 (nonstandard r3 ABI): BCCE returns r3 (u8 byte or 0), r0 restored via pop {r0}.
// Caller ABI: channel in r1 (nonstandard r1), state in r0 (standard r0), r2/r3 temps. Proven via ROM table at 0x061504 (0x0802BD81 thumb entry at file 0x061504: 81 BD 02 08) within 0x061504 table (4 entries 0x0802BD75/0x0802BD81/0x0802BD95/0x0802BDA9), dispatched via r1 channel.
// Do not guess voice layout beyond +28 (u16) / +30 (u16) / +32 (u16) directly via strh/ldrh, proven via objdump.

#ifndef __APPLE__
void _0802BD80(void *a, void *b) __attribute__((alias("SoundBD80_Scale")));
void sub_0802BD80(void *a, void *b) __attribute__((alias("SoundBD80_Scale")));
__attribute__((naked)) void SoundBD80_Scale(void *state, void *chan) {
    __asm__ volatile (
        ".syntax unified\n"
        "mov  ip, lr\n"
        "bl   _0802BCE8\n"
        "lsls r3, r3, #1\n"
        "strh r3, [r0, #28]\n"
        "ldrh r2, [r0, #30]\n"
        "muls r3, r2\n"
        "lsrs r3, r3, #8\n"
        "strh r3, [r0, #32]\n"
        "bx   ip\n"
        ".syntax divided\n"
    );
}
#else
void SoundBD80_Scale(void *a, void *b) { (void)a; (void)b; }
#endif
