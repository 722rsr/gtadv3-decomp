#include "gtadv/ai_catalog.h"
#include "gba/types.h"

int Ai_IdMap(int id){
    int v = (s16)id;
    if(v>97) return -1;
    if(v<0) return -1;
    return v;
}
#ifndef __APPLE__
int _080022E4(int a) __attribute__((alias("Ai_IdMap")));
int Sub_080022E4(int a) __attribute__((alias("Ai_IdMap")));
#endif

/* The catalog accessors narrow both the input and mapped index to signed
 * halfwords. The ROM deliberately indexes -1 when the mapper rejects an id.
 * Keep the shifted value before the absolute data symbol: agbcc then loads
 * the table between lsls #16 and asrs #14, matching the ROM instruction order.
 * These assembler declarations define data addresses, not executable code. */
#ifndef __APPLE__
#define Catalog_IdMap _080022E4
extern int _080022E4(int id);
#else
#define Catalog_IdMap Ai_IdMap
#endif
void *Ai_CatalogA(int id){
    int mapped = Catalog_IdMap((s16)id);
    s32 shifted = (u32)mapped << 16;
#ifndef __APPLE__
    extern u8 CatalogTableA[];
    __asm__(".globl CatalogTableA\nCatalogTableA = 0x080CC640\n");
    uintptr_t base = (uintptr_t)CatalogTableA;
#else
    uintptr_t base = 0x080CC640u;
#endif
    s32 offset = shifted >> 14;
    return (void *)(uintptr_t)*(const u32 *)(offset + base);
}
void *Ai_CatalogB(int id){
    int mapped = Catalog_IdMap((s16)id);
    s32 shifted = (u32)mapped << 16;
#ifndef __APPLE__
    extern u8 CatalogTableB[];
    __asm__(".globl CatalogTableB\nCatalogTableB = 0x080CC7C4\n");
    uintptr_t base = (uintptr_t)CatalogTableB;
#else
    uintptr_t base = 0x080CC7C4u;
#endif
    s32 offset = shifted >> 14;
    return (void *)(uintptr_t)*(const u32 *)(offset + base);
}
void *Ai_CatalogC(int id){
    int mapped = Catalog_IdMap((s16)id);
    s32 shifted = (u32)mapped << 16;
#ifndef __APPLE__
    extern u8 CatalogTableC[];
    __asm__(".globl CatalogTableC\nCatalogTableC = 0x080CC948\n");
    uintptr_t base = (uintptr_t)CatalogTableC;
#else
    uintptr_t base = 0x080CC948u;
#endif
    s32 offset = shifted >> 14;
    return (void *)(uintptr_t)*(const u32 *)(offset + base);
}
#ifndef __APPLE__
void *_08024C3C(int a) __attribute__((alias("Ai_CatalogA")));
void *sub_08024C3C(int a) __attribute__((alias("Ai_CatalogA")));
#endif
#ifndef __APPLE__
void *_08024C58(int a) __attribute__((alias("Ai_CatalogB")));
void *sub_08024C58(int a) __attribute__((alias("Ai_CatalogB")));
#endif
#ifndef __APPLE__
void *_08024C74(int a) __attribute__((alias("Ai_CatalogC")));
#endif

int Ai_CatalogFlatIndex(int id){
    int target=(s16)id;
    const u32 *groupTbl=(const u32*)(uintptr_t)0x080CC518u;
    int flat=0;
    for(int g=0;g<=10;++g){
        const u32 *list=(const u32*)(uintptr_t)groupTbl[g];
        if(list==NULL) { flat++; continue; }
        for(int i=0;;++i){ if((int)list[i]==-1) break; if((int)list[i]==target) return flat; }
        flat++;
    }
    return 0;
}
#ifndef __APPLE__
int _08024C90(int a) __attribute__((alias("Ai_CatalogFlatIndex")));
int sub_08024C90(int a) __attribute__((alias("Ai_CatalogFlatIndex")));
int Sub_08024C90(int a) __attribute__((alias("Ai_CatalogFlatIndex")));
#endif

// Friendly-name wrapper used by car_award.c (u8 car id; VMA _08024C90).
int Ai_CarCatalogIdx(u8 id) { return Ai_CatalogFlatIndex((int)id); }

int Ai_CatalogIndexInGroup(int id, int g){
    const s32 * const *groupTbl = (const s32 * const *)0x080CC518;
    const s32 *list = groupTbl[g];
    int idx = 0;
    if (*list != -1) {
        do {
            if (*list == id) return idx;
            idx++;
            list++;
        } while (*list != -1);
    }
    return 0;
}
#ifndef __APPLE__
int _08024CD0(int a,int b) __attribute__((alias("Ai_CatalogIndexInGroup")));
#endif

int Ai_CatalogOwnedIndexInGroup(int id,int g){
    const u32 *groupTbl=(const u32*)(uintptr_t)0x080CC518u;
    const u32 *list=(const u32*)(uintptr_t)groupTbl[g &0xF];
    extern int _08025FAC(int);
    int ownedCnt=0;
    for(int i=0;;++i){
        int cur=(int)list[i]; if(cur==-1) break;
        if(_08025FAC(cur)==0) continue;
        if(cur==id) return ownedCnt;
        ownedCnt++;
    }
    return 0;
}
#ifndef __APPLE__
int _08024D0C(int a,int b) __attribute__((alias("Ai_CatalogOwnedIndexInGroup")));
int Sub_08024D0C(int a,int b) __attribute__((alias("Ai_CatalogOwnedIndexInGroup")));
#endif

void *Ai_CatalogGroupPtr(int g){
    const u32 *t=(const u32*)(uintptr_t)0x080CC518u;
    return (void*)(uintptr_t)(t[g] + t[0] - t[0]);
}
#ifndef __APPLE__
void *_08024D4C(int a) __attribute__((alias("Ai_CatalogGroupPtr")));
#endif

#ifndef __APPLE__
extern const u32 Table24D5C[];
extern int _08024C90(int a);
#define Call_FlatIndex _08024C90
#else
#define Call_FlatIndex Ai_CatalogFlatIndex
#endif
void *Ai_CatalogGroupOfId(int id){
#ifndef __APPLE__
    const u32 *t;
    __asm__(".globl Table24D5C\nTable24D5C = 0x080CC614\n");
    t = Table24D5C;
    return (void*)(uintptr_t)t[Call_FlatIndex(id)];
#else
    const u32 *t = (const u32 *)(uintptr_t)0x080CC614u;
    return (void*)(uintptr_t)t[Call_FlatIndex(id)];
#endif
}
#ifndef __APPLE__
void *_08024D5C(int a) __attribute__((alias("Ai_CatalogGroupOfId")));
void *sub_08024D5C(int a) __attribute__((alias("Ai_CatalogGroupOfId")));
#endif

int Ai_CatalogMaxId(void){ return 97; }
#ifndef __APPLE__
int _08024D74(void) __attribute__((alias("Ai_CatalogMaxId")));
#endif
int Ai_CatalogGroupCount(void){ return 11; }
#ifndef __APPLE__
int _08024DD8(void) __attribute__((alias("Ai_CatalogGroupCount")));
#endif

int Ai_CatalogGroupSize(int g){
#ifndef __APPLE__
    extern const u32 Table24D78[];
    __asm__(".globl Table24D78\nTable24D78 = 0x080CC518\n");
    const u32 *t = Table24D78;
#else
    const u32 *t = (const u32*)(uintptr_t)0x080CC518u;
#endif
    const u32 *list=(const u32*)(uintptr_t)t[g];
    int c=0; while(*list!=(u32)~0u){ c++; list++; } return c;
}
#ifndef __APPLE__
int _08024D78(int a) __attribute__((alias("Ai_CatalogGroupSize")));
#endif

int Ai_CatalogOwnedInGroup(int g){
    const u32 *t=(const u32*)(uintptr_t)0x080CC518u;
    const u32 *list=(const u32*)(uintptr_t)t[g &0xF];
    extern int _08025FAC(int);
    int c=0;
    for(int i=0;;++i){ int v=(int)list[i]; if(v==-1) break; if(_08025FAC(v)) c++; }
    return c;
}
#ifndef __APPLE__
int _08024DA0(int a) __attribute__((alias("Ai_CatalogOwnedInGroup")));
int Sub_08024DA0(int a) __attribute__((alias("Ai_CatalogOwnedInGroup")));
#endif

// 0x08024DDC / 0x08024E00 — both are the same 36B body: the table word goes in
// r1 and the constant 0x083D7BE8 in r0, then _08007498 and _0800748C are
// chained (the ROM passes 07498's RETURN to 0748C — there is no reload between
// the two `bl`s). 0748C takes ONE argument (asm/course_resource_access.s:8),
// so nothing is live across a call and the frame stays `push {lr}`.
//
// The return type is `void *`, not `void`, and that is a byte difference rather
// than a style one: the ROM's epilogue is `pop {r1}; bx r1`, which is the form
// agbcc emits when the function returns a value, against `pop {r0}; bx r0` for
// one that does not. Every caller discards the result, so nothing observable
// changes; include/gtadv/ai_catalog.h had to move with it, because a lone
// `void *` here is a hard agbcc "conflicting types" error and the C89
// transform then skips the whole TU.
void *Ai_CatalogBindA(int g){
    extern void *_08007498(void*,int); extern void *_0800748C(void*);
    void *dst=(void*)(uintptr_t)0x083D7BE8u;
    const u32 *t=(const u32*)(uintptr_t)0x080CC1E4u;
    return _0800748C(_08007498(dst,(int)(t[g]+t[0]-t[0])));
}
#ifndef __APPLE__
void *_08024DDC(int a) __attribute__((alias("Ai_CatalogBindA")));
#endif
void *Ai_CatalogBindB(int g){
    extern void *_08007498(void*,int); extern void *_0800748C(void*);
    void *dst=(void*)(uintptr_t)0x083D7BE8u;
    const u32 *t=(const u32*)(uintptr_t)0x080CC1E4u;
    return _0800748C(_08007498(dst,(int)(t[g]+t[0]-t[0])));
}
#ifndef __APPLE__
void *_08024E00(int a) __attribute__((alias("Ai_CatalogBindB")));
#endif

// 0x08024E24 — 16B: strh r1, [r0, #0]; adds r0, #4; bl sub_08024F34
void Ai_CatalogStoreHelper(void *p, int v){
    *(volatile u16*)p = (u16)v;
    extern void sub_08024F34(void*);
    sub_08024F34((u8*)p+4);
}
#ifndef __APPLE__
void _08024E24(void *a, int b) __attribute__((alias("Ai_CatalogStoreHelper")));
void sub_08024E24(void *a, int b) __attribute__((alias("Ai_CatalogStoreHelper")));
#endif

// 0x08024F34 — 16B ten-byte zero-fill (asm/code_24f34.s): strb #0 at
// [r0+0..9] — clears the 10 catalog bytes after a store (ROM 0x08024E24's
// `adds r0,#4` tail zeroes bytes 4..13 of the record; SaveHook_0802446C uses
// it for the garage-record wipe loop).
void Ai_CatalogClear10(void *rec) {
    u8 *b = (u8 *)rec;
    b[0] = 0;
    b[1] = 0;
    b[2] = 0;
    b[3] = 0;
    b[4] = 0;
    b[5] = 0;
    b[6] = 0;
    b[7] = 0;
    b[8] = 0;
    b[9] = 0;
}
#ifndef __APPLE__
void _08024F34(void *a) __attribute__((alias("Ai_CatalogClear10")));
void sub_08024F34(void *a) __attribute__((alias("Ai_CatalogClear10")));
#endif

int Ai_CatalogOwnedTotal(void){
    extern int _08025FAC(int);
    int c=0; for(int i=1;i<=98;++i) if(_08025FAC(i)) c++; return c;
}
#ifndef __APPLE__
int _08024E34(void) __attribute__((alias("Ai_CatalogOwnedTotal")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void * Sub_08024C3C(int id) __attribute__((alias("Ai_CatalogA")));
void * Sub_08024C58(int id) __attribute__((alias("Ai_CatalogB")));
void * Sub_08024C74(int id) __attribute__((alias("Ai_CatalogC")));
void * Sub_08024D5C(int id) __attribute__((alias("Ai_CatalogGroupOfId")));
#endif
