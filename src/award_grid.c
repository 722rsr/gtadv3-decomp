#include "gtadv/award.h"
#include "gtadv/memory.h"

// asm/award_grid.s 0x0800B9F0 — counts completed cells (value 3) over ids 0..31 via 0x08025E1C per id,
// then grants cars 51/78/89/26 at thresholds 7/15/23/31.

// Extern contract for cell reader at 0x08025E1C: int sub_08025E1C(int id) returns u8 0..3 (low byte)
__attribute__((weak)) int Sub_08025E1C(int id){ (void)id; return 0; }

extern void Award_GrantCar(u8 id);

void Award_GrantThresholdCars(void) {
    int cnt = 0;
    for (int i = 0; i <= 31; i++) {
        int v = Sub_08025E1C(i);
        v = (v << 24) >> 24; // lsls #24 / lsrs #24 normalize as in asm
        v &= 0xFF;
        if (v == 3) cnt++;
    }
    if (cnt > 7)  Award_GrantCar(51);
    if (cnt > 15) Award_GrantCar(78);
    if (cnt > 23) Award_GrantCar(89);
    if (cnt > 31) Award_GrantCar(26);
}

#ifndef __APPLE__
void _0800B9F0(void) __attribute__((alias("Award_GrantThresholdCars")));
#endif
