#include "gtadv/sound.h"
#include "gba/types.h"

// Bounded non-noop leaf: sub_0802BCB4 table copy via shared pool at _0802BCE4
// VMA 0x0802BCB4–0x0802BCCC (24 B) in asm/sound_channel_cluster.s:0x02BC84–0x02BE78.
// Pure-Thumb, mov ip,lr / bl _0802BCCE / bx ip.

#ifndef __APPLE__
void _0802BCB4(void *a) __attribute__((alias("SoundBCB4_Copy")));
void sub_0802BCB4(void *a) __attribute__((alias("SoundBCB4_Copy")));
__attribute__((naked)) void SoundBCB4_Copy(void *dst) {
    __asm__ volatile (
        ".syntax unified\n"
        "mov  ip, lr\n"
        "movs r1, #36\n"
        "ldr  r2, [pc, #40]\n"
        "1:\n"
        "ldr  r3, [r2, #0]\n"
        "bl   _0802BCCE\n"
        "stmia r0!, {r3}\n"
        "adds r2, #4\n"
        "subs r1, #1\n"
        "bgt.n 1b\n"
        "bx   ip\n"
        ".short 0x0000\n"
        ".syntax divided\n"
    );
}
#else
void SoundBCB4_Copy(void *dst) { (void)dst; }
#endif
