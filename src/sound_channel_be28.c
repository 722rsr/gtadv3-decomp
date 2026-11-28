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

void _0802BE28(void *a, void *b) __attribute__((alias("SoundBE28_Store")));
void sub_0802BE28(void *a, void *b) __attribute__((alias("SoundBE28_Store")));
// Preserve the helper's custom r3 result and ip return around the C byte store.
// Keeping dummy `a` live at the store leaves channel unpinned in r1; EXACT 12/12.
__attribute__((naked)) void SoundBE28_Store(void *a, void *chan) {
    register u32 value __asm__("r3");
    register volatile u8 *channel = (volatile u8 *)chan;
    __asm__ volatile (".syntax unified\n\tmov ip, lr\n\tbl _0802BCE8\n\t.syntax divided"
                      : "=r"(value) : : "ip", "lr", "cc", "memory");
    __asm__ volatile ("" : : "r"(a));
    channel[27] = (u8)value;
    __asm__ volatile (".syntax unified\n\tbx ip\n\t.short 0x0000\n\t.syntax divided"
                      : : : "cc");
}
