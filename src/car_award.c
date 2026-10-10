#include "gtadv/award.h"
#include "gtadv/memory.h"
#include "gtadv/callee.h"

// Reference: asm/car_award.s 0x0800B990-0x0800B9F0.
// The ROM preserves the word-width car id through the calls, truncating only
// the owned-list byte store. Presence is tested through its low halfword.
#ifndef __APPLE__
extern int sub_08025FAC(int id);
extern int _08024C90(int id);
extern void sub_08025F78(int id);
extern void _08023FF8(int id, int arg);
extern u8 AwardWorkBase[];
#else
extern int Ai_OwnedTest(int id);
extern int Ai_CatalogFlatIndex(int id);
extern void Ai_AwardSetBit(int id);
#endif

void Award_GrantCar(int carId) {
    u8 *base;
    s16 *cursor;
    int idx;
    u32 off;
    int pos;
    if ((u16)CALLEE(Ai_OwnedTest, sub_08025FAC)(carId)) return;
#ifndef __APPLE__
    // A symbol base keeps the pool load separate from the field offsets.
    __asm__(".globl AwardWorkBase\nAwardWorkBase = 0x03001780\n");
    base = AwardWorkBase;
#else
    base = (u8 *)WORK_AREA_BASE;
#endif
    *(u16 *)(base + 0x1058) = 1;
    idx = CALLEE(Ai_CatalogFlatIndex, _08024C90)(carId);
    idx <<= 1;
    off = 0x1060;
    *(u16 *)(base + off + idx) = 1;
    cursor = (s16 *)(base + 0x103C);
    // Read before moving the base to the owned list. Reusing off reproduces
    // the ROM's subtract-34 instruction rather than a fourth pool constant.
    pos = *cursor;
    off -= 34;
    base += off;
    *(u8 *)(pos + base) = (u8)carId;
    CALLEE(Ai_AwardSetBit, sub_08025F78)(carId);
    CALLEE(Scene_PostEvent, _08023FF8)(28, 0);
    *(u16 *)cursor = *(u16 *)cursor + 1;
}

#ifndef __APPLE__
void _0800B990(int id) __attribute__((alias("Award_GrantCar")));
void sub_0800B990(int id) __attribute__((alias("Award_GrantCar")));
#endif
