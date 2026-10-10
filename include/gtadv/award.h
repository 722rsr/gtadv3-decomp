#ifndef GTADV_AWARD_H
#define GTADV_AWARD_H
#include "gba/types.h"
#include "gtadv/ai_grid.h"
#ifndef WORK_AREA_BASE
#define WORK_AREA_BASE 0x03001780
#endif

// Award / car-collection helpers
// Sources: asm/car_award.s (0x0800B990-0x0800B9F0), asm/award_grid.s (0x0800B9F0-0x0800BA3C),
//          asm/award_pkt.s (0x0800B0BC-0x0800B190), asm/award_twins.s (0x0800BA3C-0x0800BC08)
// Assembly reference: asm/ai_collect.s

void Award_GrantCar(int carId);                // _0800B990  (sub_0800B990)
void Award_GrantThresholdCars(void);          // _0800B9F0  (sub_0800B9F0)
void Award_BuildTierPacket(void);             // _0800B0BC  (sub_0800B0BC) type-10 packet via _080188B0
void Award_CopyTwinA(int idx, void *dst0, void *dst1); // _0800BA3C
void Award_CopyTwinB(int idx, void *src, void *dst);   // _0800BB0C

// Extern contracts for unavailable lane APIs (AI/physics)
// These are byte-exact external helpers; lifted lane provides weak stubs
// if the AI lane has not yet been lifted.
int  Ai_GridRead(int type, int row, int col); // sub_08025CF4 — returns u8 0..3 (int to match ai_grid.h)
int  Ai_CarPresence(u8 id);                   // sub_08025FAC
int  Ai_CarCatalogIdx(u8 id);                 // _08024C90 / _08024C74 etc
void Ai_GrantEvent(u8 id);                    // sub_08025F78
void Scene_PostEvent(int id, int arg);        // _08023FF8

#endif
