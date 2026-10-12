#include "gtadv/sound.h"
#include "gba/types.h"
#include "gba/bios.h"

// Residual pure-Thumb instruction-backed leaves/bodies with exact pools/offsets/widths.
// No invented PSG +0x19/+0x1C 28 B overlap, BE32/DMA, mixer mode/copy, open-bus, allocation/Smsh, or voice layouts beyond proven u8/u16.
// Uses proven 0x08061F74 (stride12 table) / 0x08061FA4 (stride8 dir) pools only where ldr [pc] proves them.
// Opaque volatile byte offsets, exact u8/u16/u32 widths via ldrb/ldrh/ldr/strb/strh, lsls #16/lsrs #16 for u16 s16, direct branches bne/beq/cmp, helper ABIs bl sub_0802D4A8/D510/D584 etc.
// Preserve prior aliases; claim only complete exact CFGs via strong alias; no placeholders/weak.

extern void sub_0802D4A8(void *state, u32 chsel, u32 wid);
extern void sub_0802D510(void *state, u32 chsel, u32 wid);
extern void sub_0802D584(void *state, u32 chsel, u32 wid);
extern void sub_0802D974(const void *src, void *dst, u32 ctrl);
extern void sub_0802C8C4(void *a);
extern void sub_0802C780(void *a);
extern void sub_0802CA34(void *a);
extern void sub_0802CBBC(void *dst, void *src, u32 size);

#ifndef __APPLE__
extern const u8 SoundSeqWalkerTable_B718[];
extern const u8 SoundSeqWalkerDirectory_B718[];
extern const u8 SoundSeqWalkerTable_B74C[];
extern const u8 SoundSeqWalkerDirectory_B74C[];
extern const u8 SoundSeqWalkerTable_B780[];
extern const u8 SoundSeqWalkerDirectory_B780[];
#endif

// --- _0802B718(id,chsel,val) — volume walker -> sub_0802D4A8. The caller
// places channel mask in r1 and the u16 volume in r2; the ROM forwards them
// unchanged in that order after narrowing each to u16.
void SoundSeq_VolumeWalker(u32 id, u32 chsel, u32 val){
#ifndef __APPLE__
    __asm__(".globl SoundSeqWalkerTable_B718\n"
            "SoundSeqWalkerTable_B718 = 0x08061F74\n"
            ".globl SoundSeqWalkerDirectory_B718\n"
            "SoundSeqWalkerDirectory_B718 = 0x08061FA4\n");
#endif
    register u32 index __asm__("r0") = id << 16;
#ifdef __APPLE__
    register const u8 *table __asm__("r4") = (const u8 *)0x08061F74u;
#else
    register const u8 *table __asm__("r4") = SoundSeqWalkerTable_B718;
#endif
    __asm__ volatile("" : "+r" (table));
#ifdef __APPLE__
    register const u8 *directory __asm__("r3") = (const u8 *)0x08061FA4u;
#else
    register const u8 *directory __asm__("r3") = SoundSeqWalkerDirectory_B718;
#endif
    __asm__ volatile("" : "+r" (directory));
    index >>= 13;
    index += (u32)(uintptr_t)directory;
    register u16 gate __asm__("r5") =
        ((const volatile u16 *)(uintptr_t)index)[2];
    register u32 offset __asm__("r3") = (u32)gate << 1;
    offset += gate;
    offset <<= 2;
    offset += (u32)(uintptr_t)table;
    register void *state __asm__("r0") =
        (void *)(uintptr_t)*(const volatile u32 *)(uintptr_t)offset;
    register u32 c16 __asm__("r1") = (chsel << 16) >> 16;
    register u32 v16 __asm__("r2") = (val << 16) >> 16;
    sub_0802D4A8(state, c16, v16);
}
#ifndef __APPLE__
void _0802B718(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_VolumeWalker")));
void sub_0802B718(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_VolumeWalker")));
#endif

// --- _0802B74C(id,chsel,val) — pan walker -> sub_0802D510; r1 is a u16
// channel mask and r2 is sign-extended from the low s16 before the call.
void SoundSeq_PanWalker(u32 id, u32 chsel, u32 val){
#ifndef __APPLE__
    __asm__(".globl SoundSeqWalkerTable_B74C\n"
            "SoundSeqWalkerTable_B74C = 0x08061F74\n"
            ".globl SoundSeqWalkerDirectory_B74C\n"
            "SoundSeqWalkerDirectory_B74C = 0x08061FA4\n");
#endif
    register u32 index __asm__("r0") = id << 16;
#ifdef __APPLE__
    register const u8 *table __asm__("r4") = (const u8 *)0x08061F74u;
#else
    register const u8 *table __asm__("r4") = SoundSeqWalkerTable_B74C;
#endif
    __asm__ volatile("" : "+r" (table));
#ifdef __APPLE__
    register const u8 *directory __asm__("r3") = (const u8 *)0x08061FA4u;
#else
    register const u8 *directory __asm__("r3") = SoundSeqWalkerDirectory_B74C;
#endif
    __asm__ volatile("" : "+r" (directory));
    index >>= 13;
    index += (u32)(uintptr_t)directory;
    register u16 gate __asm__("r5") =
        ((const volatile u16 *)(uintptr_t)index)[2];
    register u32 offset __asm__("r3") = (u32)gate << 1;
    offset += gate;
    offset <<= 2;
    offset += (u32)(uintptr_t)table;
    register void *state __asm__("r0") =
        (void *)(uintptr_t)*(const volatile u32 *)(uintptr_t)offset;
    register u32 c16 __asm__("r1") = (chsel << 16) >> 16;
    register s32 v16 __asm__("r2") = (s32)(val << 16) >> 16;
    sub_0802D510(state, c16, (u32)v16);
}
#ifndef __APPLE__
void _0802B74C(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_PanWalker")));
void sub_0802B74C(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_PanWalker")));
#endif

// --- _0802B780(id,chsel,val) — third-op walker -> sub_0802D584; r1 is a u16
// channel mask and r2 is sign-extended from the low s8 before the call.
void SoundSeq_ThirdWalker(u32 id, u32 chsel, u32 val){
#ifndef __APPLE__
    __asm__(".globl SoundSeqWalkerTable_B780\n"
            "SoundSeqWalkerTable_B780 = 0x08061F74\n"
            ".globl SoundSeqWalkerDirectory_B780\n"
            "SoundSeqWalkerDirectory_B780 = 0x08061FA4\n");
#endif
    register u32 index __asm__("r0") = id << 16;
#ifdef __APPLE__
    register const u8 *table __asm__("r4") = (const u8 *)0x08061F74u;
#else
    register const u8 *table __asm__("r4") = SoundSeqWalkerTable_B780;
#endif
    __asm__ volatile("" : "+r" (table));
#ifdef __APPLE__
    register const u8 *directory __asm__("r3") = (const u8 *)0x08061FA4u;
#else
    register const u8 *directory __asm__("r3") = SoundSeqWalkerDirectory_B780;
#endif
    __asm__ volatile("" : "+r" (directory));
    index >>= 13;
    index += (u32)(uintptr_t)directory;
    register u16 gate __asm__("r5") =
        ((const volatile u16 *)(uintptr_t)index)[2];
    register u32 offset __asm__("r3") = (u32)gate << 1;
    offset += gate;
    offset <<= 2;
    offset += (u32)(uintptr_t)table;
    register void *state __asm__("r0") =
        (void *)(uintptr_t)*(const volatile u32 *)(uintptr_t)offset;
    register u32 c16 __asm__("r1") = (chsel << 16) >> 16;
    register s32 v8 __asm__("r2") = (s32)(val << 24) >> 24;
    sub_0802D584(state, c16, (u32)v8);
}
#ifndef __APPLE__
void _0802B780(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_ThirdWalker")));
void sub_0802B780(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_ThirdWalker")));
#endif

// --- _0802C4C4 — driver-wide reset/init, pools 0x0802B91D/0x03007000/0x040000E0/0x0203E230/0x0203EC40/0x0093F800/4/0x08061F74/0x0203EE00 ---
// Exact: push {r4-r6,lr}; ldr r0,[pc+#80]=0x0802B91D & -2 =>0x0802B91C; ldr r1=0x03007000; ldr r2=0x040000E0; bl D974; ldr r0=0x0203E230 bl C8C4; ldr r0=0x0203EC40 bl C780; ldr r0=0x0093F800 bl CA34; ldr r0=4 count; ldr r5=0x08061F74 tab; loop 12 B rec: ldr dst/src, ldrb size, bl CBBC, ldrh [rec+10] strb [dst+11], ldr 0x0203EE00 str [dst+24]
void SoundReset_DriverInit(void){
    // The ROM keeps the *tagged* pointer 0x0802B91D in the pool and clears bit
    // 0 at runtime (`movs r1,#2` / `negs r1,r1` / `ands r0,r1`), so the source
    // value cannot be a folded C constant. Same for the 4-entry count: it is
    // materialised from the pool, narrowed with `lsls #16` / `lsrs #16`, and
    // still tested with a runtime `cmp #0`. Assembler-resolved absolutes keep
    // both loads alive (see docs/findings/track_car_26180_pool_order.md).
    u32 srcA;
    u16 cnt;
#ifndef __APPLE__
    extern u8 SoundResetSrcA[];
    extern u8 SoundResetTabCount[];
    __asm__(".globl SoundResetSrcA\nSoundResetSrcA = 0x0802B91D\n");
    __asm__(".globl SoundResetTabCount\nSoundResetTabCount = 4\n");
    srcA = (u32)(uintptr_t)SoundResetSrcA;
#else
    srcA = 0x0802B91Du;
#endif
    sub_0802D974((const void*)(uintptr_t)(srcA & ~1u), (void*)0x03007000u, 0x040000E0u);
    sub_0802C8C4((void*)0x0203E230u);
    sub_0802C780((void*)0x0203EC40u);
    sub_0802CA34((void*)0x0093F800u);
    // Loaded here, not at the top: the ROM materialises the count only after
    // the four init calls (`ldr r0,[pc,#68]` at +0x24).
#ifndef __APPLE__
    cnt = (u16)(uintptr_t)SoundResetTabCount;
#else
    cnt = 4u;
#endif
    if (cnt == 0)
        return;
    {
        // ROM shape: the table base is the walking pointer (r5) and the count
        // is the down counter (r6) - `adds r5,#12` / `subs r6,#1` / `bne`.
        // The counter is a full-width copy of the u16 (`adds r6,r0,#0`);
        // decrementing the u16 itself would re-issue the narrowing pair.
        volatile u8 *rec = (volatile u8*)0x08061F74u;
        int n = cnt;
        do {
            void *dst = *(void**)rec; // ldr [r5] u32 dst
            sub_0802CBBC(dst, *(void**)(rec + 4), *(volatile u8*)(rec + 8));
            *(volatile u8*)((u8*)dst + 11) = *(volatile u16*)(rec + 10);
            *(volatile u32*)((u8*)dst + 24) = 0x0203EE00u;
            rec += 12;
        } while (--n);
    }
}
#ifndef __APPLE__
void _0802C4C4(void) __attribute__((alias("SoundReset_DriverInit")));
void sub_0802C4C4(void) __attribute__((alias("SoundReset_DriverInit")));
#endif
