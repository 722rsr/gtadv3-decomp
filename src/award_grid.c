#include "gtadv/award.h"
#include "gtadv/callee.h"

// asm/award_grid.s 0x0800B9F0: count completed cells (low byte == 3)
// across ids 0..31, then grant cars at completed counts 8, 16, 24, and 32.
#ifndef __APPLE__
extern int sub_08025E1C(int id);
extern void _0800B990(int id);
#endif

void Award_GrantThresholdCars(void) {
    int cnt = 0;
    int idx = 0;
    for (; idx <= 31; idx++) {
        if ((u8)CALLEE(Ai_GridGetPacked, sub_08025E1C)(idx) == 3) cnt++;
    }
    if (cnt > 7)  CALLEE(Award_GrantCar, _0800B990)(51);
    if (cnt > 15) CALLEE(Award_GrantCar, _0800B990)(78);
    if (cnt > 23) CALLEE(Award_GrantCar, _0800B990)(89);
    if (cnt > 31) CALLEE(Award_GrantCar, _0800B990)(26);
}
// The ROM ends with a zero halfword, not gas's default Thumb nop fill.
__asm__(".align 2, 0");

#ifndef __APPLE__
void _0800B9F0(void) __attribute__((alias("Award_GrantThresholdCars")));
void sub_0800B9F0(void) __attribute__((alias("Award_GrantThresholdCars")));
#endif
