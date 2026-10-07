#ifndef GTADV_COURSE_ORCH_H
#define GTADV_COURSE_ORCH_H
#include "gba/types.h"

// Course orchestrator cluster 0x080060EC-0x08006650
void Course_VariantCopy(void *src); // _080060EC CpuFastSet 1 word to 0x0203F758
void *Course_Group0Payload12(int idx, void *base); // _08006100 idx+ base -> payload+12
void Course_VariantZero(void); // _08006114 zero fill variant cell
void Course_Orchestrator(void *out, int course, void *base); // _08006138 full pipeline
void Course_VariantGfx(void *out, int idx, void *base); // _080061B8 groups 5/6/7
void Course_BigGfx(void *out, int idx, void *base);     // _0800628C groups 8/9/10
void Course_ThemeGfx(void *out, int idx, void *base); // _08006468 groups 2/3/4
void Course_ParseHeader(void *out, int idx, void *base); // _080064EC group0 header -> out struct
void Course_SurfaceLoad(void *a, int b, void *c); // _08006574 group1 ->0x02000000
                                               // arg2 is the course id (int) and
                                               // arg3 the base pointer, per the
                                               // caller at _08006138:0x619E; arg1
                                               // is passed in r0 and unused.
void *Course_SeekRes(void *base, int grp, int idx); // _08006590 seeker
int Course_AttrLookup(int x, int y, int v); // _080065D4 table 0x080C8FE4

// Load entry 0x0801A204 COURSE LOAD (no args, uses global 0x03002886/0x03002884/0x0300287E)
void Course_LoadEntry(void); // _0801A204

#endif
