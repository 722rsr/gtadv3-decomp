#include "gtadv/foundation.h"
#include "gba/types.h"
#include "gba/bios.h"

// Late foundation leaves — code_57d0, 5988, c668, ce2c are small dispatch/flat leaves
// 5b3c math leaves already in foundation_math.c; fa0 and runtime tails remain TODO beyond these.

extern int _0802D9B8(int);
void SaveSlotConfig(int v){
    volatile u32 *slot = (volatile u32 *)0x030003ACu;
    volatile u32 *base = (volatile u32 *)0x03000320u;
    *slot = 0x03000320u;
    base[1] = (u32)v;
    base[0] = 0;
    switch((u32)v){
    case 1:
        base[2] = 0;
        (void)_0802D9B8(4);
        break;
    case 2:
        base[2] = 0;
        (void)_0802D9B8(64);
        break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080057D0(int a) __attribute__((alias("SaveSlotConfig")));
void sub_080057D0(int a) __attribute__((alias("SaveSlotConfig")));
#endif

// code_5988 — 0x08005988: EEPROM slot verify wrapper — owner is save.c (SaveSlotLoad)
// Duplicate alias removed; use save.c's SaveSlotLoad via extern.
extern int SaveSlotVerify(int a,int b); // actually SaveSlotLoad in save.c, keep extern for link
extern int SaveSlotLoad(u32 a, void *b);

// code_c668 — 0x0800C668 dispatch triplet (all via 0x08004EF0 with offsets 0x25E4/0x25E8/0x25EC)
void LateDispatch_C668(int base,int id){
    int v = id-1;
    if((unsigned)v>7) return;
    static const u32 offs[8]={0x25E4,0x25E4,0x25E8,0x25E8,0x25EC,0x25EC,0x25EC,0x25EC};
    extern void _08004EF0(void*);
    u32 off = offs[v &7];
    _08004EF0((void*)(uintptr_t)(base + off));
}
#ifndef __APPLE__
void _0800C668(int a,int b) __attribute__((alias("LateDispatch_C668")));
#endif
#ifndef __APPLE__
void _0800C6CC(int a,int b) __attribute__((alias("LateDispatch_C668")));
#endif
void LateFlagCheck(void *a){ if(*(vu8*)((u8*)a+96)==0) { extern void _08004ED8(int); _08004ED8(1); } }
#ifndef __APPLE__
void _0800C730(void *a) __attribute__((alias("LateFlagCheck")));
void sub_0800C730(void *a) __attribute__((alias("LateFlagCheck")));
#endif

void LateDispatch_CE2C(int id, int a,int b, void *ctx){
    switch((unsigned)id){
        case 2: { extern void _0800CCC8(void*); _0800CCC8(ctx); break; }
        case 7: { extern void _0800CDEC(void*); _0800CDEC(ctx); break; }
        case 6: { extern void _0800CD48(void*,u16,u16); _0800CD48(ctx,(u16)a,(u16)b); break; }
        case 1: { extern void _0800CCEC(void*); _0800CCEC(ctx); break; }
        default: break;
    }
}
#ifndef __APPLE__
void _0800CE2C(int a,int b,int c,void *d) __attribute__((alias("LateDispatch_CE2C")));
#endif

// code_4e6c tail — manager alloc (skipped, TODO) — provide alias guards only
// 5b3c / fa0 lifted elsewhere; no stubs here.
// Owners: _08005BA8 → foundation_math.c (MathHelper_05BA8),
// _08000FA0 → foundation_fa0.c (IntrMain_Dispatch).
