#include "gtadv/sound.h"
#include "gba/types.h"

// EXPERIMENT: does a callee-side r3 pin reproduce the ROM's r3 result ABI,
// and does it survive `bx lr` without an `adds r0,r3,#0` fixup?
extern u32 _0802BCE8(void *dummy, void *chan);
u32 SoundBCE8_Test(void *dummy, void *chan){
    register u32 v asm("r3");
    volatile u8 *c = (volatile u8*)chan;
    u32 cur = *(volatile u32*)(c + 64);
    *(volatile u32*)(c + 64) = cur + 1;
    v = *(volatile u8*)(uintptr_t)cur;
    return v;
}

#ifndef __APPLE__
void _0802BE28(void *a, void *b) __attribute__((alias("SoundBE28_Store")));
void sub_0802BE28(void *a, void *b) __attribute__((alias("SoundBE28_Store")));
__attribute__((naked)) void SoundBE28_Store(void *a, void *chan) {
    __asm__ volatile (
        ".syntax unified\n"
        "mov  ip, lr\n"
        "bl   _0802BCE8\n"
        "strb r3, [r1, #27]\n"
        "bx   ip\n"
        ".short 0x0000\n"
        ".syntax divided\n"
    );
}
#else
void SoundBE28_Store(void *a, void *b) { (void)a; (void)b; }
#endif
