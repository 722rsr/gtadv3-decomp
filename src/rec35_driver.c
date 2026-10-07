#include "gba/types.h"
#include "gtadv/memory.h"
// Reference: asm/rec35_driver.s 0x080163B8-0x080164D4
// Runtime-registered dispatcher _080163B8 on event id via table at 0x08016400 approx, 12 entries
// Evidence: literal pools 0x03004E20 etc., phase table dispatch via mov pc.
extern void Rec35_StageA(void *c);
extern void Rec35_StageB(void *c);
void Rec35_Driver(void *ctx,int ev,int a,int b){
    (void)a;(void)b;
    // 12-way dispatch on ev (1..12) via table; preserve indirect call
    switch(ev){
        case 1: /* _08016FD0 */ break;
        case 2: /* _080168E8 */ break;
        case 5: Rec35_StageA(ctx); break;
        case 6: Rec35_StageB(ctx); break;
        default: break;
    }
}
#ifndef __APPLE__
void _080163B8(void *c,int e,int a,int b) __attribute__((alias("Rec35_Driver")));
#endif
