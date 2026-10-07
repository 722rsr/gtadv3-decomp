#ifndef GTADV_AI_CATALOG_H
#define GTADV_AI_CATALOG_H
#include "gba/types.h"

// Car catalog 0x08024C3C cluster — ROM tables 0x080CC640/C7C4/C948 + group list 0x080CC518

void *Ai_CatalogA(int id);          // _08024C3C
void *Ai_CatalogB(int id);          // _08024C58
void *Ai_CatalogC(int id);          // _08024C74
int   Ai_CatalogFlatIndex(int id);  // _08024C90 — flat slot of id
int   Ai_CatalogIndexInGroup(int id,int g); // _08024CD0
int   Ai_CatalogOwnedIndexInGroup(int id,int g); // _08024D0C
void *Ai_CatalogGroupPtr(int g);    // _08024D4C
void *Ai_CatalogGroupOfId(int id);  // _08024D5C
int   Ai_CatalogMaxId(void);        // _08024D74 ->97
int   Ai_CatalogGroupSize(int g);   // _08024D78
int   Ai_CatalogOwnedInGroup(int g);// _08024DA0
int   Ai_CatalogGroupCount(void);   // _08024DD8 ->11
void *Ai_CatalogBindA(int g);       // _08024DDC — the ROM returns 0748C's result
void *Ai_CatalogBindB(int g);       // _08024E00 — every caller discards it
void  Ai_CatalogStoreHelper(void *p, int v); // _08024E24
int   Ai_CatalogOwnedTotal(void);   // _08024E34

int Ai_IdMap(int id); // _080022E4 — bounds-checked mapper (-1 if >97)

#endif
