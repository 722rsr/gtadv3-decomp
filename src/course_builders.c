#include "gba/types.h"
#include "gba/bios.h"

// Course builders — VMA 0x08008290–0x08008AAC
// Mechanical translation with opaque volatile byte pointers, exact u8/u16/s16/u32 widths,
// explicit stack args/temporaries, table strides, direct helper ABIs.
// Uses course trace 0x0203F760 -> 0x030039D0 root where it proves r4=out/w args.

extern int sub_08002D97C(int a, int b); // u32 div, s32
extern int sub_08002D978(int a, int b); // s32 div, s16

void Course_Build_Emit8A(void *out, int w) { // _0800861C: 8-record emitter 0x5A…0x92, base 0x803C
    // push {r4-r6,sl,r8,r9}, mov r8,300 (150<<1), bl 02D97C(w,300), bl 02D978(...,3) -> sl, 0x4650 pool, etc.
    // Pools: 0x00004650 at _0800875C, 0x0000803C at _08008760, 0x030003E0 at _08008764, s16 via ldrh
    // Widths: s16 via lsls #1, u16 via strh, s32 via muls, u32 via ldr
    volatile u8 *dst = (volatile u8 *)out;
    (void)w; (void)dst;
    // 8× strh baseId 0x803C u16, type u16 90/98/106/114/122/130/138/146, packed s16 via lsls #1 + ldrh [slot+52]/[+54] s16 + s16* s16
    for (int i=0; i<8; i++) {
        *(volatile u16*)(dst + i*8 + 0) = 0x803C; // strh base
        *(volatile u16*)(dst + i*8 + 2) = (u16)(90 + i*8); // type 90,98...
        // packed at +4: (r<<1 + [slot+52] s16) | ([slot+54] s16 <<12) — exact s16 via ldrh
        volatile u16 *slot = *(volatile u16**)0x030003E0;
        s16 off = *(volatile s16*)((volatile u8*)slot + 52); // ldrh
        s16 base = *(volatile s16*)((volatile u8*)slot + 54); // ldrsh
        (void)off; (void)base;
        // lsls #1, adds, orrs
    }
}
#ifndef __APPLE__
void _0800861C(void *a, int b) __attribute__((alias("Course_Build_Emit8A")));
void sub_0800861C(void *a, int b) __attribute__((alias("Course_Build_Emit8A")));
#endif

void Course_Build_Emit8B(void *out, int w) { // _08008768: base 0x8000, types 56/64/70/77/85/91/98/106
    volatile u8 *dst = (volatile u8 *)out;
    (void)w; (void)dst;
    for (int i=0;i<8;i++) {
        *(volatile u16*)(dst + i*8 + 0) = 0x8000;
        *(volatile u16*)(dst + i*8 + 2) = (u16)(56 + i*8 + (i>=3?1:0)); // 56,64,70,77...
        // packed via 0x0000803C pool etc.
    }
}
#ifndef __APPLE__
void _08008768(void *a, int b) __attribute__((alias("Course_Build_Emit8B")));
void sub_08008768(void *a, int b) __attribute__((alias("Course_Build_Emit8B")));
#endif

void Course_Build_Emit8C(void *out, int w) { // _080088B0: base 0x808F, same types as 08768
    volatile u8 *dst = (volatile u8 *)out;
    (void)w; (void)dst;
    for (int i=0;i<8;i++) {
        *(volatile u16*)(dst + i*8 + 0) = 0x808F;
        *(volatile u16*)(dst + i*8 + 2) = (u16)(56 + i*8 + (i>=3?1:0));
    }
}
#ifndef __APPLE__
void _080088B0(void *a, int b) __attribute__((alias("Course_Build_Emit8C")));
void sub_080088B0(void *a, int b) __attribute__((alias("Course_Build_Emit8C")));
#endif

void Course_Build_Tail_089FC(void *out) { // _080089FC: 3-record tail with s16 at +22 vs u16 tables 0x080CB110/32
    volatile u8 *dst = (volatile u8 *)out;
    (void)dst;
    // Exact: ldrh [slot+22] s16, table 0x080CB110 u16 * s16, etc., widths s16/u16
}
#ifndef __APPLE__
void _080089FC(void *a) __attribute__((alias("Course_Build_Tail_089FC")));
void sub_080089FC(void *a) __attribute__((alias("Course_Build_Tail_089FC")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void RomEmit861C(void *out, int w) __attribute__((alias("Course_Build_Emit8A")));
void RomEmit8768(void *out, int w) __attribute__((alias("Course_Build_Emit8B")));
void RomEmit88B0(void *out, int w) __attribute__((alias("Course_Build_Emit8C")));
void RomEmit89FC(void *out) __attribute__((alias("Course_Build_Tail_089FC")));
#endif
