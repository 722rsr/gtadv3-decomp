#ifndef GTADV_AI_LINE_H
#define GTADV_AI_LINE_H
#include "gba/types.h"

// Collection-line accessors & helpers

// Byte accessors over ROM 0x080CCEEC (20-byte records)
s8 Ai_LineGet0(int id);   // _080254E8 off 0
s8 Ai_LineGet1(int id);   // _08025500 off 14
s8 Ai_LineGet2(int id);   // _08025518 off 13
s8 Ai_LineGet3(int id);   // _08025530 off 12

// Bit probe/write over IWRAM 0x030015E8 + ROM 0x080C4768
int  Ai_LineProbe(int id);   // _08025214
void Ai_LineWrite(int id);   // _08025248 — CpuFastSet style 0x05000006

// Record helpers over 0x080CCD8C / 12-byte arrays
s16 Ai_LineHelperLoad(int a,int b,int c); // _08024FE8  — ldrsh [base + a*176 + b*44 + c*4 +2]
int Ai_LineFindSlot(int id, const void *base); // _0802500C / _08025084
int Ai_LineEqual(const void *a, const void *b); // _080250FC

void Ai_LineCopyInsert(void *dst, const void *src); // _08025028
void Ai_LineCmpInsert(void *dst, const void *src);   // same but second variant
void Ai_LineTimingHelper(const void *src); // _080251FC — bl 0x0802D974, r0 forwarded
void Ai_LineClearAndCompact(void *a, void *b); // _08025198
void Ai_LineUpdateAndCompact(void *a, void *b); // _08025130

// Catalog sums over 0x080CD6A8
int Ai_LineScoreSum(const void *rec);   // _08025548
int Ai_LineScoreSum2(const void *rec);  // _080255C4 / 25640 family — two sums
int Ai_LineCatalogField0(const void *rec); // _080256BC family
int Ai_LineTierLookup(int a,int b,int c);  // _0802572C variant

// Tail lookups
int Ai_LineFindCourse(int id); // _0802581C — scan 0x080600CC
int Ai_LineFieldU8(int idx,int off); // wrappers

#endif
