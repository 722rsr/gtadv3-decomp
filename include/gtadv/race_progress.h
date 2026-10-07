#ifndef GTADV_RACE_PROGRESS_H
#define GTADV_RACE_PROGRESS_H
#include "gba/types.h"

// Race progress / award-grid commit helpers
// Source: asm/race_progress.s (0x0800AD84-0x0800B0BC: _0800AD84, _0800AE78, _0800AF84)
// Companion: asm/race_dispatch.s dispatcher, asm/ai_collect.s grid
#define WORK_AREA_BASE 0x03001780

void Race_CommitCell(void);      // _0800AD84
void Race_AdvanceProgress(void); // _0800AE78
// _0800AF84: gate checks + phase dispatch + per-frame ticks. Returns the step
// id the caller (_0800A76C) feeds back into the scene record chain.
int  Race_PerFrameTail(void);

#endif
