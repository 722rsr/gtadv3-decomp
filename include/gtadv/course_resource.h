#ifndef GTADV_COURSE_RESOURCE_H
#define GTADV_COURSE_RESOURCE_H
#include "gba/types.h"

// Resource accessors from asm/course_resource_access.s 0x0800748C-0x08007664
// Xt = X +12 + [X+8]*4; Xti = X +12+[X+12+4i] etc.
u32   Course_07488(void *p);                  // _08007488 ldr [p+8] Vu32 direct leaf
void *Course_07978(void *X, int i);           // _08007978 Seek(Seek(X,i),0)->ArrBase
void *Course_ArrBase(void *X);                // _0800748C
void *Course_Seek(void *X, int i);            // _08007498
u32   Course_GetU32At4(void *X);              // _080074A8 reads [X+4]
void *Course_GetCountPtr(void *X);            // _08007484 (u16 count at +8)
void  Course_ClearSlot(void *X, int idx);     // _080074AC helper
void  Course_SetupRecords(void *rec);         // _080074D4 uses _0800572C allocator
void  Course_EmitLane(void *a, int b, void *c, int d); // _08007538 etc. simplified

// Resource init / orchestration
void Course_ResourceSetup(void *ctx, void *X, int idx); // _08007664 full setup (DMA desc at 0x040000D4)
void *Course_GetResourcePtr(void *X, int idx); // helpers


// Surface accessors from asm/surface_access.s 0x08005F8C-0x08006050
void Surface_DiffStore(void *out, void *a, void *b); // _08005F8C out = *a - *b dup at +4
void *Surface_GetRoot(void);                          // _08005F98 returns *(0x0203F760)
u8   Surface_Sample(int x, int y);                   // _08005FA4 samples map byte via root+36 + map
void *Surface_GetRecordPtr(void);                     // _08005FD4
void *Surface_IndexRecord(int idx);                   // _08005FE0 *(u8**)(root+4) + 20*idx
int   Surface_Nop0(void);                             // _08005FF8 returns 0
int   Surface_Adjust(int a, int b, int c);            // _08005FFC (s16)(Surface_Nop0 - (s16)(u16)c)
void  Surface_ScatterInit(void *root);                // _08006018 DMA scatter 128x

#endif
