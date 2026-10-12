#include "gtadv/ai_catalog.h"
#include "gba/types.h"

// 0x080022E4 — id -> catalog id via s16 table 0x080C43EC. The ROM validates
// with `(u16)(id-1) > 97` (agbcc folds it to `(id<<16) + 0xFFFF0000 >> 16`)
// and indexes the table with the SIGNED halfword, so 1..98 is the legal range.
// The previous body returned the identity and never touched the table.
int Ai_IdMap(int id){
    extern s16 L_IdTable[];
    __asm__(".globl L_IdTable\nL_IdTable = 0x080C43EC\n");
    s16 v = (s16)id;
    if ((u16)(v - 1) <= 97)
        return L_IdTable[v];
    return -1;
}
int _080022E4(int a) __attribute__((alias("Ai_IdMap")));
int Sub_080022E4(int a) __attribute__((alias("Ai_IdMap")));

// 0x0800230C — second entry in the same ROM span (typed out of _080022E4 in
// asm/code_22e4.s; two literal words hold it as a function pointer). Same
// 0x080C43EC table, but the -1 sentinel maps to 23 and the out-of-range path
// returns the `(u16)(id-1)` the range check already computed.
int Ai_IdRemap(int id){
    extern s16 L_IdTable[];
    __asm__(".globl L_IdTable\nL_IdTable = 0x080C43EC\n");
    s16 v = (s16)id;
    u16 n = (u16)(v - 1);
    if (n <= 97) {
        s16 t = L_IdTable[v];
        if (t == -1)
            t = 23;
        return t;
    }
    return n;
}
int _0800230C(int a) __attribute__((alias("Ai_IdRemap")));
int Sub_0800230C(int a) __attribute__((alias("Ai_IdRemap")));

/* The catalog accessors narrow both the input and mapped index to signed
 * halfwords. The ROM deliberately indexes -1 when the mapper rejects an id.
 * Keep the shifted value before the absolute data symbol: agbcc then loads
 * the table between lsls #16 and asrs #14, matching the ROM instruction order.
 * These assembler declarations define data addresses, not executable code. */
#define Catalog_IdMap _080022E4
extern int _080022E4(int id);
void *Ai_CatalogA(int id){
    int mapped = Catalog_IdMap((s16)id);
    s32 shifted = (u32)mapped << 16;
    extern u8 CatalogTableA[];
    __asm__(".globl CatalogTableA\nCatalogTableA = 0x080CC640\n");
    uintptr_t base = (uintptr_t)CatalogTableA;
    s32 offset = shifted >> 14;
    return (void *)(uintptr_t)*(const u32 *)(offset + base);
}
void *Ai_CatalogB(int id){
    int mapped = Catalog_IdMap((s16)id);
    s32 shifted = (u32)mapped << 16;
    extern u8 CatalogTableB[];
    __asm__(".globl CatalogTableB\nCatalogTableB = 0x080CC7C4\n");
    uintptr_t base = (uintptr_t)CatalogTableB;
    s32 offset = shifted >> 14;
    return (void *)(uintptr_t)*(const u32 *)(offset + base);
}
void *Ai_CatalogC(int id){
    int mapped = Catalog_IdMap((s16)id);
    s32 shifted = (u32)mapped << 16;
    extern u8 CatalogTableC[];
    __asm__(".globl CatalogTableC\nCatalogTableC = 0x080CC948\n");
    uintptr_t base = (uintptr_t)CatalogTableC;
    s32 offset = shifted >> 14;
    return (void *)(uintptr_t)*(const u32 *)(offset + base);
}
void *_08024C3C(int a) __attribute__((alias("Ai_CatalogA")));
void *sub_08024C3C(int a) __attribute__((alias("Ai_CatalogA")));
void *_08024C58(int a) __attribute__((alias("Ai_CatalogB")));
void *sub_08024C58(int a) __attribute__((alias("Ai_CatalogB")));
void *_08024C74(int a) __attribute__((alias("Ai_CatalogC")));

extern const u32 Table24D78[];
__asm__(".globl Table24D78\nTable24D78 = 0x080CC518\n");

int Ai_CatalogFlatIndex(int id){
    int flat = 0;
    register const s32 * const *groupTbl asm("r4");
    const s32 *list;
    int sentinel = -1;
    groupTbl = (const s32 * const *)(uintptr_t)Table24D78;
    do {
        list = *groupTbl;
        if (*list != sentinel) {
            do {
                if (*list == id) return flat;
                list++;
            } while (*list != -1);
        }
        groupTbl++;
        flat++;
    } while (flat <= 10);
    return 0;
}
__asm__(".align 2, 0");
int _08024C90(int a) __attribute__((alias("Ai_CatalogFlatIndex")));
int sub_08024C90(int a) __attribute__((alias("Ai_CatalogFlatIndex")));
int Sub_08024C90(int a) __attribute__((alias("Ai_CatalogFlatIndex")));

int Ai_CatalogIndexInGroup(int id, int g){
    int idx = 0;
    const s32 *list = (const s32 *)(uintptr_t)Table24D78[g];
    if (*list != -1) {
        do {
            if (id == *list) return idx;
            idx++;
            list++;
        } while (*list != -1);
    }
    return 0;
}
__asm__(".align 2, 0");
int _08024CD0(int a,int b) __attribute__((alias("Ai_CatalogIndexInGroup")));

int Ai_CatalogOwnedIndexInGroup(int id, int g){
    int ownedCnt = 0;
    const s32 *list = (const s32 *)(uintptr_t)Table24D78[g];
    extern int _08025FAC(int);
    while (*list != -1) {
        if (_08025FAC(*list)) {
            if (id == *list) return ownedCnt;
            ownedCnt++;
        }
        list++;
    }
    return 0;
}
__asm__(".align 2, 0");
int _08024D0C(int a,int b) __attribute__((alias("Ai_CatalogOwnedIndexInGroup")));
int Sub_08024D0C(int a,int b) __attribute__((alias("Ai_CatalogOwnedIndexInGroup")));

void *Ai_CatalogGroupPtr(int g){
    const u32 *t=(const u32*)(uintptr_t)0x080CC518u;
    return (void*)(uintptr_t)(t[g] + t[0] - t[0]);
}
void *_08024D4C(int a) __attribute__((alias("Ai_CatalogGroupPtr")));

extern const u32 Table24D5C[];
extern int _08024C90(int a);
#define Call_FlatIndex _08024C90
void *Ai_CatalogGroupOfId(int id){
    const u32 *t;
    __asm__(".globl Table24D5C\nTable24D5C = 0x080CC614\n");
    t = Table24D5C;
    return (void*)(uintptr_t)t[Call_FlatIndex(id)];
}
void *_08024D5C(int a) __attribute__((alias("Ai_CatalogGroupOfId")));
void *sub_08024D5C(int a) __attribute__((alias("Ai_CatalogGroupOfId")));

int Ai_CatalogMaxId(void){ return 97; }
int _08024D74(void) __attribute__((alias("Ai_CatalogMaxId")));
int Ai_CatalogGroupCount(void){ return 11; }
int _08024DD8(void) __attribute__((alias("Ai_CatalogGroupCount")));

int Ai_CatalogGroupSize(int g){
    extern const u32 Table24D78[];
    __asm__(".globl Table24D78\nTable24D78 = 0x080CC518\n");
    const u32 *t = Table24D78;
    const u32 *list=(const u32*)(uintptr_t)t[g];
    int c=0; while(*list!=(u32)~0u){ c++; list++; } return c;
}
int _08024D78(int a) __attribute__((alias("Ai_CatalogGroupSize")));

int Ai_CatalogOwnedInGroup(int g){
    const u32 *t=(const u32*)(uintptr_t)0x080CC518u;
    __asm__("" : "+r"(t));
    const u32 *cur=(const u32*)(uintptr_t)t[g];
    extern int _08025FAC(int);
    int c=0;
    if ((int)*cur != -1) {
        do {
            int v = (int)*cur;
            if (_08025FAC(v)) c++;
            cur++;
        } while ((int)*cur != -1);
    }
    return c;
}
int _08024DA0(int a) __attribute__((alias("Ai_CatalogOwnedInGroup")));
int Sub_08024DA0(int a) __attribute__((alias("Ai_CatalogOwnedInGroup")));

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
void *_08024DDC(int a) __attribute__((alias("Ai_CatalogBindA")));
void *Ai_CatalogBindB(int g){
    extern void *_08007498(void*,int); extern void *_0800748C(void*);
    void *dst=(void*)(uintptr_t)0x083D7BE8u;
    const u32 *t=(const u32*)(uintptr_t)0x080CC1E4u;
    return _0800748C(_08007498(dst,(int)(t[g]+t[0]-t[0])));
}
void *_08024E00(int a) __attribute__((alias("Ai_CatalogBindB")));

// 0x08024E24 — 16B: strh r1, [r0, #0]; adds r0, #4; bl sub_08024F34
void Ai_CatalogStoreHelper(void *p, int v){
    *(volatile u16*)p = (u16)v;
    extern void sub_08024F34(void*);
    sub_08024F34((u8*)p+4);
}
__asm__(".align 2, 0");
void _08024E24(void *a, int b) __attribute__((alias("Ai_CatalogStoreHelper")));
void sub_08024E24(void *a, int b) __attribute__((alias("Ai_CatalogStoreHelper")));

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
void _08024F34(void *a) __attribute__((alias("Ai_CatalogClear10")));
void sub_08024F34(void *a) __attribute__((alias("Ai_CatalogClear10")));

int Ai_CatalogOwnedTotal(void){
    extern int _08025FAC(int);
    int c=0; for(int i=1;i<=98;++i) if(_08025FAC(i)) c++; return c;
}
int _08024E34(void) __attribute__((alias("Ai_CatalogOwnedTotal")));

// ROM entry alias.
void * Sub_08024C3C(int id) __attribute__((alias("Ai_CatalogA")));
void * Sub_08024C58(int id) __attribute__((alias("Ai_CatalogB")));
void * Sub_08024C74(int id) __attribute__((alias("Ai_CatalogC")));
void * Sub_08024D5C(int id) __attribute__((alias("Ai_CatalogGroupOfId")));
