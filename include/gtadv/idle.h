#ifndef GTADV_IDLE_H
#define GTADV_IDLE_H

#include "gba/types.h"

// Idle path cluster 0x08001988–0x080020E8 — helpers over State Block A.
// See asm/idle.s and asm/blocka.s. Many are simple
// halfword/byte accessors; the dispatcher and handlers remain in asm
// pending full C lift, but the pure accessors are lifted here.

// Event area at +0xC0: u16 slots for payloads id 0..n via _08001E48.
// `payload` is deliberately WORD-width: the ROM stores r1 with a bare `strh`
// and never narrows it, so a u16 parameter adds a `lsls/lsrs` pair the ROM does
// not have (and the body then cannot match). Every caller passes a halfword
// value, which zero-extends into u32 identically.
void Idle_WriteEvent(int id, u32 payload);            // _08001E48
u16 Idle_GetRecordHalfword(int idx0, int idx1);       // _08001E5C
u16 Idle_GetDirectHalfword(int recOff, int hwIdx);    // _08001E8C
u16 Idle_GetIndexedHalfword2(int hwIdx);              // _08001EA4
bool Idle_IsMode1(void);                              // _08001F3C (current mode==1)
void *Idle_GetRecordPtr(int idx);                      // _08001F54
void Idle_SetRecordCount(u16 v);                      // _08001F80
int Idle_GetRecordCount(void);                         // _08001F8C (was u16; see idle_accessors.c)
u16 Idle_GetCurrentMode(void);                        // _08002038
u32 Idle_GetCounterC(void);                           // _08002044 (+0x0C; word ABI)
u16 Idle_GetD8Word(void);                             // _08002050 (+0xD8)
void Idle_SetFlag2(u8 v);                             // _08002060 (+0x02)
u32 Idle_GetRecordFiltered(int idx);                  // _0800206C wrapper over _080015F4; word ABI
u32 Idle_GetD0Word(void);                             // _08001CB4 (+0xD0 raw word)
s32 Idle_GetD0WordHalved(void);                      // _08001CC4 (+0xD0 halved, toward zero)
void Idle_RequestMode10(u32 arg);                     // _08001FB4 (request mode 10, +0x28 = arg)
int Idle_IsSpecialModeAny(void);                      // _08002014 (special-mode test over both)
void Idle_ArenaRegister(void *handle, void *region, int len); // _0800207C

#endif // GTADV_IDLE_H
