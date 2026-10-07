#ifndef GTADV_AI_AWARD_H
#define GTADV_AI_AWARD_H
#include "gba/types.h"

// Collection bitset / award record leaves
// ROM tables
#define AI_COLLECTION_BM_BASE  0x03001780u  // +0x20 = 0x030017A0 bitmask base
#define AI_MASK_TABLE          0x080CD9D4u // byte masks for bitset
#define AI_AWARD_REC_BASE      0x030015F0u // 8-byte? actually 4-byte stride 8 record array
#define AI_MASK_TBL2           0x08060D4Cu // 6-byte table for 26020

void Ai_AwardSetBit(int id);                 // _08025F78
int  Ai_OwnedTest(int id);                   // _08025FAC
void Ai_SetOwnedFlag(int slot);     // _08025FF0 — slot+base+1400 writes 1
int  Ai_GetOwnedFlag(int slot);     // _08026004 — read flag byte
void Ai_ClaimSlotAndSetBit(int slot);        // _08026020
void Ai_AwardLeafSet(int idx, int a, int b); // _08026150 — writes pair at 0x030015F0 stride 8
s16  Ai_AwardLeafGet(int idx);               // _080261B0
void Ai_AwardLeafPut(int idx, int v);        // _080261C4

// low-level helpers used by grid (cross-lane)
extern void BitUnPackWram(const void *src, void *dst, const void *tbl);
extern int  Ai_PackedShift(int idx, int bits);

#endif
