#include "gtadv/award.h"
#include "gtadv/memory.h"

// Reference: asm/car_award.s 0x0800B990-0x0800B9F0
// work area WA = 0x03001780

extern int Ai_CarPresence(u8 id);      // sub_08025FAC
extern int Ai_CarCatalogIdx(u8 id);    // _08024C90
extern void Ai_GrantEvent(u8 id);      // sub_08025F78
extern void Scene_PostEvent(int id, int arg); // _08023FF8

void Award_GrantCar(u8 carId) {
    if (Ai_CarPresence(carId)) return;
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    // u16[wa+0x1058] =1  (changed flag)
    *(volatile u16 *)(wa + 0x1058) = 1;
    int idx = Ai_CarCatalogIdx(carId);
    // catalog slot at wa+0x1060 + idx*2
    *(volatile u16 *)(wa + 0x1060 + (idx << 1)) = 1;
    // owned-list append: cursor s16[wa+0x103C], base wa+0x103E
    s16 cursor = *(volatile s16 *)(wa + 0x103C);
    *(volatile u8 *)(wa + 0x103E + cursor) = carId;
    Ai_GrantEvent(carId);
    Scene_PostEvent(28, 0);
    // cursor++
    s16 cur = *(volatile s16 *)(wa + 0x103C);
    *(volatile s16 *)(wa + 0x103C) = cur + 1;
}

#ifndef __APPLE__
void _0800B990(u8 id) __attribute__((alias("Award_GrantCar")));
#endif
