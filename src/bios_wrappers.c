#include "gba/types.h"
#include "gba/bios.h"

/*
 * BIOS/SWI wrappers. The original ROM invokes these
 * SWIs directly; lifted C calls the SDK-named wrappers below. Each wrapper is
 * a thin `swi n; bx lr` so behavior is identical to the original call.
 *
 * SWI numbers (GBA BIOS):
 *   0x00 SoftReset   0x05 VBlankIntrWait  0x06 Div
 *   0x08 Sqrt        0x0B CpuSet          0x0C CpuFastSet
 *   0x10 BitUnpack   0x11 LZ77UnCompWram  0x12 LZ77UnCompVram
 *   0x13 HuffUnComp  0x0E BgAffineSet     0x0F ObjAffineSet
 */

#define SWI(n) __asm__ volatile ("swi " #n ::: "r0","r1","r2","r3","r12","lr")

__attribute__((naked)) void SoftReset(u32 flags) { SWI(0x00); __asm__("bx lr"); }
__attribute__((naked)) void VBlankIntrWait(void) { SWI(0x05); __asm__("bx lr"); }

__attribute__((naked)) int Div(int num, int den) { SWI(0x06); __asm__("bx lr"); }

__attribute__((naked)) unsigned DivMod(unsigned num, unsigned den, unsigned *rem) {
    __asm__ volatile (
        "swi 0x06\n"
        "str r1, [r2]\n"
        "bx lr\n"
    );
}

__attribute__((naked)) int Sqrt(u32 num) { SWI(0x08); __asm__("bx lr"); }

__attribute__((naked)) void CpuSet(const void *src, void *dst, u32 mode) { SWI(0x0B); __asm__("bx lr"); }
__attribute__((naked)) void CpuFastSet(const void *src, void *dst, u32 mode) { SWI(0x0C); __asm__("bx lr"); }

__attribute__((naked)) void BitUnPack(const void *src, void *dst, const void *tbl) { SWI(0x10); __asm__("bx lr"); }
__attribute__((naked)) void LZ77UnCompWram(const void *src, void *dst) { SWI(0x11); __asm__("bx lr"); }
__attribute__((naked)) void LZ77UnCompVram(const void *src, void *dst) { SWI(0x12); __asm__("bx lr"); }
__attribute__((naked)) void HuffUnComp(const void *src, void *dst) { SWI(0x13); __asm__("bx lr"); }

__attribute__((naked)) void BgAffineSet(const void *src, void *dst, int num) { SWI(0x0E); __asm__("bx lr"); }
__attribute__((naked)) void ObjAffineSet(const void *src, void *dst, int num, int offset) { SWI(0x0F); __asm__("bx lr"); }

// Friendly-name wrapper used by lifted C (race_scene.c etc.).
// sub_0802D974 is exactly `swi 0x0B; bx lr`, so register pass-through is identical.
void CpuSet_2D974(const void *a, void *b, unsigned c) { CpuSet(a, b, (u32)c); }

// VMA aliases for SWI wrappers proven byte-identical to asm/sound_d974.s:
//   sub_0802D974 = swi 0x0B (CpuSet), sub_0802D978 = swi 0x06 (Div quotient),
//   sub_0802D97C = swi 0x06 + mov r0,r1 (Div remainder),
//   sub_0802D984 = swi 0x12 (LZ77UnCompVram), sub_0802D988 = swi 0x11
//   (LZ77UnCompWram), sub_0802D9AC = swi 8 (Sqrt).
// Spelling matches callers/asm (sub_0802D974 8-digit form); both _ and sub_
// spellings provided since lifted C uses both.
// sub_0802D98C (swi 0x25 with r1=1) intentionally left in asm.
__attribute__((naked)) int DivRem(int num, int den) {
    __asm__ volatile ("swi 0x06\n mov r0, r1\n bx lr\n");
}
__asm__(".align 2, 0");
// The ROM's VBlank wait entry is NOT the bare `swi 5` wrapper above: the bytes
// at 0x0802D9B0 are `movs r2,#0; swi 5; bx lr` (8 bytes,.short 0 pad at
// 0x0802D9B6 that belongs to neither body). It needs its own definition, not
// an alias of VBlankIntrWait, which compiles to 4 bytes and cannot overlap.
__attribute__((naked)) void VBlankIntrWaitR2(void) {
    __asm__ volatile (
        "movs r2, #0\n"
        "swi 5\n"
        "bx lr\n"
        ".short 0\n"
    );
}
#ifndef __APPLE__
void _0802D974(const void *a, void *b, u32 c) __attribute__((alias("CpuSet")));
void sub_0802D974(const void *a, void *b, u32 c) __attribute__((alias("CpuSet")));
int _0802D978(int a, int b) __attribute__((alias("Div")));
int sub_0802D978(int a, int b) __attribute__((alias("Div")));
int _0802D97C(int a, int b) __attribute__((alias("DivRem")));
int sub_0802D97C(int a, int b) __attribute__((alias("DivRem")));
void _0802D984(const void *a, void *b) __attribute__((alias("LZ77UnCompVram")));
void sub_0802D984(const void *a, void *b) __attribute__((alias("LZ77UnCompVram")));
void _0802D988(const void *a, void *b) __attribute__((alias("LZ77UnCompWram")));
void sub_0802D988(const void *a, void *b) __attribute__((alias("LZ77UnCompWram")));
int _0802D9AC(u32 a) __attribute__((alias("Sqrt")));
int sub_0802D9AC(u32 a) __attribute__((alias("Sqrt")));
// _0802D970 = swi 0x0C (CpuFastSet), per asm/sound_d6f4.s tail.
void _0802D970(const void *a, void *b, u32 c) __attribute__((alias("CpuFastSet")));
void sub_0802D970(const void *a, void *b, u32 c) __attribute__((alias("CpuFastSet")));
void _0802D9B0(void) __attribute__((alias("VBlankIntrWaitR2")));
void sub_0802D9B0(void) __attribute__((alias("VBlankIntrWaitR2")));
void _08002D9B0(void) __attribute__((alias("VBlankIntrWaitR2")));

// The 8-digit spellings. The forms above are the *7*-digit ones, and the
// passing-2.5 veneer list kept showing _08002D974 / _08002D978 / _08002D97C /
// _08002D970 / _08002D9AC / Sub_08002D974 undefined: every caller that spelled
// the VMA with its leading zero was jumping through an absolute veneer into the
// same `swi`, and the `Sub_` spelling had no definition at all. Same SWIs, same
// shapes, so these are plain aliases to the wrappers above.
void _08002D974(const void *a, void *b, u32 c) __attribute__((alias("CpuSet")));
void Sub_08002D974(const void *a, void *b, u32 c) __attribute__((alias("CpuSet")));
int  _08002D978(int a, int b) __attribute__((alias("Div")));
int  _08002D97C(int a, int b) __attribute__((alias("DivRem")));
void _08002D970(const void *a, void *b, u32 c) __attribute__((alias("CpuFastSet")));
int  _08002D9AC(u32 a) __attribute__((alias("Sqrt")));
#endif
