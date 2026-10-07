#ifndef GTADV_AI_GRID_H
#define GTADV_AI_GRID_H
#include "gba/types.h"

// Packed 2-bit collection grid + row counters
// Base IWRAM 0x03001780; mask table ROM 0x08060D48; helper tables
#define AI_GRID_BASE   0x03001780u
#define AI_GRID_MASK_TBL 0x08060D48u

int Ai_GridGet(int type, int row, int col);          // _08025CF4
int Ai_GridCountCol(int type, int col);              // _08025D64 — count set cells in col? actually type,row? check asm: 25D64(type,row)
int Ai_GridRowsFull(int type);                       // _08025D90
void Ai_GridSetPacked(int type, int value);          // _08025DBC — 2-bit update via BitUnPack
int Ai_GridGetPacked(int type);                      // _08025E1C
void Ai_GridWriteRecord(u32 a,int b,int c, u32 val); // _08025E70 — stores r3 at computed addr
u32  Ai_GridReadRecord(u32 a,int b,int c);           // _08025E98
void Ai_GridHalfwordSet(int id, int v);              // _08025EC0
int  Ai_GridHalfwordGet(int id);                     // _08025F20

#endif
