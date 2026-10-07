#ifndef GTADV_AI_RACE_H
#define GTADV_AI_RACE_H
#include "gba/types.h"

// Race scene FSM / event ring / collection manager

// ai_raceevt.s 0x023ED0 cluster — ctx is 0xF4-byte scene instance at heap
void Ai_RaceEvtHandler(int ev,int a,int b, void *ctx); // _08023ED0
void Ai_RingReset(void);                  // _08023FE4
void Ai_RingPush(int ev,int arg);         // _08023FF8

// ai_racefsm.s 0x00AA40 — global race phase FSM
void Ai_RaceFsm(void);                    // _0800AA40

// ai_collect.s 0x00B4A8 — collection manager tick
void Ai_CollectTick(void);                // _0800B4A8
s16 Ai_B4A8_GetLimit(void);               // wa+0x1054 s16 adjacency — leaf proved via ldrsh + ramwatch

// ai_phase_pump.s 0x023958 — phase pump
void Ai_PhasePump(void *ctx,int a,int b); // _08023958

// Cross-lane stubs (do not edit other lanes)
void Ai_Sub0800B4A8(void);
void Ai_Sub08025CF4(int,int,int);
void Ai_Sub08023FE4(void);
void Ai_Sub08023FF8(int,int);
void Ai_Sub08002158(int,int);

#endif
