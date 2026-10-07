#ifndef GTADV_STATE_BLOCK_A_H
#define GTADV_STATE_BLOCK_A_H

#include "gba/types.h"

// State block A at IWRAM 0x03000008; pointer slot 0x030000E4.
// Field map (see asm/blocka.s, asm/blocka.s)
typedef struct {
    u8  _00[4];
    u16 pendingParam;   // +0x04 set by _080016D0, consumed by mode-commit
    u16 sioBits;        // +0x06 SIOCNT bits 4-5 snapshot (per-frame by _08001834)
    u16 scratch08;      // +0x08 set on session enter
    u16 _0A;            // +0x0A number of records? (loop bound for _08001854)
    u8  _0C[4];
    u16 curMode;        // +0x10 current mode
    u16 reqMode;        // +0x12 requested mode
    u8  _14[4];
    s16 lastMin;        // +0x18
    s16 curMin;         // +0x1A
    u16 _1C;            // +0x1C
    u16 secCounter;     // +0x1E wraps at 30
    u8  _20[4];
    u32 argMode8;       // +0x24 arg for mode-8 hook
    u32 argMode11;      // +0x28 arg for mode-11 hook
    u8  _2C[0x44];
    u8  recordArea[0];  // +0x70 16-byte records selected by table at 0x0802E190
} StateBlockA;

#define STATE_BLOCK_A_SLOT (*(StateBlockA **)0x030000E4)
#define STATE_BLOCK_A      (*(volatile StateBlockA *)0x03000008)

// ROM descriptor table for record selection
#define RECORD_TABLE ((const u32 *)0x0802E190)

// C API — behavioral equivalent of asm/blocka.s
u16  StateA_GetRecordHalfword(int idx);                    // _080015F4
u16  StateA_GetRecordHalfwordOffset(int idx0, int idx1);   // _08001620
u16  StateA_GetFirstHalfword(void);                        // _08001650
u16  StateA_GetIndexedHalfword(int idx);                   // _08001660
void *StateA_GetRecordPtr(void);                           // _08001674
u16  StateA_GetRecordHalfwordInverted(void);               // _08001680
u16  StateA_GetIndexedHalfwordInverted(int idx);           // _08001698
void *StateA_GetRecordPtrInverted(void);                   // _080016B8
void StateA_SetPendingParam(u32 v);                        // _080016D0
void StateA_ClearPendingAndScratch(void);                  // _080016DC
void StateA_Register(void);                                // _080016EC
void StateA_SetReqMode(u32 mode);                          // _08001724
bool StateA_IsGameMode(int mode);                          // _08001730 (word-width: the ROM never narrows; signed so `>= 1` stays `cmp #1; blt`)
bool StateA_IsSpecialMode(u32 mode);                       // _08001744 (word-width: the ROM never narrows)
void StateA_CommitMode(void);                              // _08001758
void StateA_RequestMode1(void);                            // _08001818
void StateA_RequestMode0(void);                            // _08001824
void StateA_SyncPerFrame(void);                            // _08001834
void StateA_UpdateClockAndRecords(void);                   // _08001854

#endif // GTADV_STATE_BLOCK_A_H
