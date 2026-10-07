#ifndef GTADV_REC35_H
#define GTADV_REC35_H
#include "gba/types.h"

// rec35 scene machinery — runtime-registered event handlers and stage machines
// Sources: asm/rec35_init.s (0x08015CB4-0x08015E28), asm/rec35_helper.s (0x08015E28-0x08015ED0),
//          asm/rec35_stage.s (0x08015ED0-0x08016002 twin machines A/B),
//          asm/rec35_16002.s (0x08016002-0x080163B8), asm/rec35_driver.s (0x080163B8-0x080164D4),
//          asm/rec35_runtime.s (0x080164D4-0x08018A50, 76 funcs, deferred)
void Rec35_Init(void *ctx);          // _08015CB4
void Rec35_HelperA(void *ctx);       // _08015E28
void Rec35_HelperB(void *ctx);       // _08015E7C
void Rec35_StageA(void *ctx);        // _08015ED0
void Rec35_StageB(void *ctx);        // _08015F68
void Rec35_Driver(void *ctx, int ev, int a1, int a2); // _080163B8 event dispatcher
// Runtime 12-way dispatchers and bounded leaves (rec35_runtime.s 0x080164D4–0x08018A50)
void Rec35_RecordFlagSetter(void *a, void *ctx); // _0800164D4
void Rec35_Record49Constructor(void *ctx);       // _08001650C
void Rec35_Leaf_166E8(void *ctx, int r1,int r2); // _0800166E8
void Rec35_PhaseDispatch_16734(void *ctx,int a,int b); // _080016734
void Rec35_Leaf_167C4(void *ctx);                // _0800167C4
void Rec35_Dispatch_1681C(void *ctx,int a,int b,int c); // _08001681C 12-way
void Rec35_Dispatch_16BBC(void *ctx);            // _080016BBC 8-way

#endif
