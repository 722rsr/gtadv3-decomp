#ifndef GTADV_RACE_DISPATCH_H
#define GTADV_RACE_DISPATCH_H
#include "gba/types.h"

// Race/menu command dispatchers
// Sources: asm/race_dispatch.s (0x0800B190-0x0800B4A8, 8-way on s16[WA+0x0FBC])
//          asm/race_cluster.s  (0x0801A008-0x0801A204, 8-way dispatcher with resource callers)
//          asm/race_setup_18adc.s (0x08018ADC-0x0801A008, setup leaves)
//          asm/race_scene.s (0x0801A294-0x0802135C, 112 funcs, deferred)
void Race_Dispatch(void);        // _0800B190  (sub_0800B190)
void Race_ClusterDispatch(void); // _0801A008 family dispatcher (approx)

// Extern contract for shared UI packet consumer
void UiPacket_Consume(void *packet); // _080188B0

#endif
