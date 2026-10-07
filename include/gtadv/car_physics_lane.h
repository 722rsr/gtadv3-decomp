#ifndef GTADV_CAR_PHYSICS_LANE_H
#define GTADV_CAR_PHYSICS_LANE_H
#include "gba/types.h"

// Lane: CAR PHYSICS and numeric support — owns listed asm files only.
// Headers provide C API matching original VMA symbols with CamelCase wrappers
// and original alias guarantees. See asm/*.s headers for field maps.

// IWRAM anchors used by this lane
#define CAR_WORK_BASE     0x03001780u
#define GARAGE_RECORD_BASE (CAR_WORK_BASE + 0x30u)

// ---- numeric / table leaves (code_24f* family) ----
s16 Code24F4C_TableLookup(int a0, int a1, int a2); // _080024F4C (off 0)
s16 Code24F74_TableLookup(int a0, int a1, int a2); // _080024F74 (off 2)
s16 Code24F9C_TableLookup(int a0, int a1, int a2); // _080024F9C (off 4)
s16 Code24FC4_TableLookup(int a0, int a1, int a2); // _080024FC4 (176 *a0)
void Code24F34_Clear10(void *dst);                 // raw 10-byte zero (no symbol)
s16 Code258A8_SimpleLookup(int idx);               // 0x080258A8 leaf

// ---- garage / collection numeric (code_26*) ----
s16 Code26F8_GetIWRAM_S16(int r0, int r1);
void Code26F8_SetIWRAM_S16(int r0, int r1, int r2);
s16 Code2730_GetIWRAM_S16(int r0, int r1);
void Code2730_2730_etc(void);
void Code279C_SetFlag0(int v); // _0800279C
int  Code279C_IsNonZero_105C(void);
int  Code279C_Get_4770_Entry(int idx);

// ---- race control leaves (code_22*, 23* 24048 etc.) ----
int  Code22E4_Clamp97(int v); // _080022E4 -> -1 if >97 else halfword
int  Code23X_RaceHelper(int a0,int a1,int a2); // placeholder for 23628 family
void Code23E0C(void *ctx); // _080023E0C: +140 s16 *8 0x080CC178 + ctx+144 u16 ==1 + 23E78 (62 B, proven via 6000-frame trace WA+0xFBC=5, race+140 s16)
void Code23D4C(void *ctx); // _080023D4C: +140 s16 *8 0x080CC178 + ctx+144/158 + 0x08022D44/022CC/03978 (97 B, proven via 6000-frame race+140 s16)
void Code24048_StepQueuePush(void *out); // _080024048
void Code24150_ClearSlot(void *slot);
void Code2417C_GhostSnapshot(void);
void Code2446C_GuardedSaveHook(void);

// ---- car physics cores ----
void CarPhysRacer_InitFrame(void *ctx); // _08021374
void CarPhysRacer_MarkScene(void *ctx, void *rec); // _0802135C
void CarPhysRacer_Nop(void);            // _08021370
void CarPhysRacer_RaceStartLatch(void *ctx, int b, int c); // _08021828
void CarPhysRacer_Dispatcher(int ev, int b, int c, void *ctx); // _08021BF8
void GoStart_Tick(void);   // _0800B89C
void GrantDelay_Tick(int carId, int delay); // _0800B82C
void Code2694_InitWork(void); // _08002694
void *Code26F30_Alloc(int a0,int a1); // _080026F30
// ---- follow-up dispatchers (substantiated) ----
int  CarTick_Dispatcher(void *ctx); // _0800A668 52-entry tick table
void Gap_021C94_Select(void *a, void *b); // sub_080021C94 ctx+84 = 5/6 (ROM: void, r0 destroyed by epilogue)
void Code235F4_Dispatch(void *ctx); // sub_0800235F4 ctx+142 switch

#endif
