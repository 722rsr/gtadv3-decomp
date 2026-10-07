#ifndef GTADV_GHOST_H
#define GTADV_GHOST_H
#include "gba/types.h"

// Ghost subsystem — race context + ghost-record manager
// Sources: asm/ghost.s (0x08018A50-0x08018ADC) + asm/ghost2.s (0x080240C4-0x080241C8)
// Assembly reference: asm/ghost.s

#ifndef GHOST_MGR_ADDR
#define GHOST_MGR_ADDR   0x03000610
#endif
#ifndef RACE_CTX_PTR_ADDR
#define RACE_CTX_PTR_ADDR 0x03004E20
#endif
#ifndef RACER_ARRAY_BASE
#define RACER_ARRAY_BASE  0x03004E80
#endif
#define RACER_STRIDE      0x11C
#ifndef WORK_AREA_BASE
#define WORK_AREA_BASE    0x03001780
#endif

typedef struct {
    u32 state;   // +0x00  (0/1; set to 1 by commit)
    u32 recA;    // +0x04  ptr to candidate record
    u32 recP;    // +0x08  ptr to current record
} GhostMgr;

typedef struct {
    u16 valid;      // +0x00
    u16 courseId;   // +0x02
    u32 timeA;      // +0x04  racectx+0x18
    u32 timeB;      // +0x08  racectx+0x2C
    u8  blob[28];   // +0x10
    u8  blob2[28];  // +0x2C  second half (P only)
} GhostRecord; // 56 B

static inline volatile GhostMgr *GhostMgr_Get(void) { return (volatile GhostMgr *)GHOST_MGR_ADDR; }
static inline volatile u32 *RaceCtxPtr(void) { return (volatile u32 *)RACE_CTX_PTR_ADDR; }
static inline u32 RaceCtx(void) { return *RaceCtxPtr(); }

// ghost.s API
void *Ghost_RacerAt(int idx);                 // _08018A50
u8    Ghost_CarRecordField6(s16 idx);         // _08018A6C
void  Ghost_SetRaceCtxU16_74(u32 v);          // _08018A88 (u32 arg: strh consumes low 16)
void  Ghost_RaiseFlag2(void);                 // _08018A98
void  Ghost_FlagOp(u32 mask, int set);        // _08018AA8
u32   Ghost_FlagTest(u32 mask);               // _08018ACC

// RAM replay-sample buffers: bound + zeroed at boot by _08002694 -> 0x0802407C
// (2560 B = 0xA00 each). Ghost_SampleBufPtr (0x08002674, unreferenced) selects
// them by index 0/1. See asm/ghost.s
#define GHOST_BUF_A_ADDR  0x0203D600u
#define GHOST_BUF_P_ADDR  0x0203CC00u
#define GHOST_BUF_SIZE    2560

// ghost2.s API
void  Ghost_BindSampleBuffers(void *ptrA, void *ptrP, int bytes); // _0802407C
void *Ghost_SampleBufPtr(int idx);            // _08002674 (dead accessor)
void *Ghost_GetRecA(void);                    // _080240C4
void *Ghost_GetRecP(void);                    // _080240D0
void  Ghost_RecA_ClearValid(u32 unused);      // _080240DC  (arg ignored, stores 0)
void  Ghost_RecA_SetW08(u32 v);               // _080240EC
void  Ghost_RecA_SetW04(u32 v);               // _080240F8
void  Ghost_RecA_SetH02(u32 v);               // _08024104  strh to recA+0x02; arg is a full word
void  Ghost_RecA_CopyBlob(const void *src);   // _08024110  16B copy to recA+0x10 (asm does 12+4)
void  Ghost_MgrClear(void);                   // _0802412C
void  Ghost_MgrSet(void);                     // _08024138
u32   Ghost_MgrState(void);                   // _08024144
void  Ghost_Invalidate(void *p);              // _08024150
void  Ghost_Commit(void);                     // _08024158  swap A<->P, validate
void  Ghost_Snapshot(void);                   // _0802417C  split P into staging buffers (28+28)

#endif
