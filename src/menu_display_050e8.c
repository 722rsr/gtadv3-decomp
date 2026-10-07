#include "gba/types.h"

extern void _0802E0A4(void *dst, const void *src, u32 n);
extern void OAMSlotAttrSet(int slot, u16 value);
extern void OAMSlotParamSet(int slot, u32 value);

// 0x080050E8 — build the display-control word and four BG control words,
// publish the four OAM slots, optionally write the display registers, then
// copy the 68-byte record to the shared menu state block.
void MenuDisplaySetup_050E8(void *record_, int write_display)
{
    volatile u8 *record = (volatile u8 *)record_;
    u16 bg_enable[6];
    u16 bg_priority[4];
    u16 bg_size[4];
    u16 bg_char[4];
    u16 bgcnt[4];

    _0802E0A4(bg_enable, (const void *)0x0805BA80u, 12);
    _0802E0A4(bg_priority, (const void *)0x0805BA8Cu, 8);
    _0802E0A4(bg_size, (const void *)0x0805BA94u, 8);
    _0802E0A4(bg_char, (const void *)0x0805BA9Cu, 8);

    u16 dispcnt = *(volatile u16 *)(uintptr_t)0x04000000u;
    dispcnt = (u16)(dispcnt | 0x0040u | bg_enable[*(volatile u16 *)(record + 0)]);
    for (int i = 0; i < 4; i++) {
        if (record[4 + i * 16] != 0)
            dispcnt = (u16)(dispcnt | bg_priority[i]);
    }

    u16 bg_mode = *(volatile u16 *)(record + 2);
    if (bg_mode != 0)
        dispcnt = (u16)(dispcnt | 0x1000u);

    for (int i = 0; i < 4; i++) {
        volatile u8 *entry = record + i * 16;
        u16 value = (u16)(bg_size[entry[8]] | bg_char[entry[9]] | 0x0040u);
        if (entry[7] != 0)
            value = (u16)(value | 0x0080u);
        value = (u16)(value | ((u16)entry[5] << 2) | ((u16)entry[6] << 8));
        bgcnt[i] = value;
    }

    OAMSlotAttrSet(0, *(volatile u16 *)(record + 12));
    OAMSlotAttrSet(1, *(volatile u16 *)(record + 28));
    OAMSlotAttrSet(2, *(volatile u16 *)(record + 44));
    OAMSlotAttrSet(3, *(volatile u16 *)(record + 60));
    OAMSlotParamSet(0, *(volatile u32 *)(record + 16));
    OAMSlotParamSet(1, *(volatile u32 *)(record + 32));
    OAMSlotParamSet(2, *(volatile u32 *)(record + 48));
    OAMSlotParamSet(3, *(volatile u32 *)(record + 64));

    if (write_display != 0) {
        volatile u16 *regs = (volatile u16 *)(uintptr_t)0x04000008u;
        for (int i = 0; i < 4; i++)
            regs[i] = bgcnt[i];
        *(volatile u16 *)(uintptr_t)0x04000000u = dispcnt;
    }

    _0802E0A4((void *)(uintptr_t)0x03000260u, (const void *)record, 68);
}

#ifndef __APPLE__
void _080050E8(void *record, int write_display)
    __attribute__((alias("MenuDisplaySetup_050E8")));
void sub_080050E8(void *record, int write_display)
    __attribute__((alias("MenuDisplaySetup_050E8")));
#endif
