#include "gtadv/award.h"
#include "gba/types.h"
#include "gtadv/memory.h"


// _0800BA3C(idx, pA, pB) — twin A, 76-byte frame, sorted 5+4 row copies.
__asm__(".set AwardWaBase, 0x03001780");
__asm__(".set AwardRowOff, 0x000005E4");
__asm__(".set AwardCell4C, 0x0300274C");
extern u8 AwardWaBase;
extern u8 AwardRowOff;
extern u8 AwardCell4C;
// The `.globl` assignments must sit inside the promoted body: the slice linker
// only sees this function's own section, and a file-scope `.set` alone leaves
// the symbols file-local, failing the link with "undefined reference to
// `AwardWaBase'" (same recipe as race_scene_b1.c's WA_SPLIT_DECL).
void Award_CopyTwinA(int idx_, void *pA, void *pB) {
    typedef struct { u32 w[3]; } Blk12;
    __asm__(".globl AwardWaBase\nAwardWaBase = 0x03001780\n"
            ".globl AwardRowOff\nAwardRowOff = 0x000005E4\n"
            ".globl AwardCell4C\nAwardCell4C = 0x0300274C\n");
    u8 temp[60];
    register int idx __asm__("r4") = idx_;
    volatile u8 *pa = (u8 *)pA;
    volatile u8 *pb = (u8 *)pB;
    register u32 found __asm__("r8") = 0;
    int count = 0;
    u32 idx8;
    register u32 base __asm__("r9") = (u32)(uintptr_t)&AwardWaBase;
    u32 idx72 = (u32)(((idx << 3) + idx) << 3);
    register u32 off __asm__("sl") = (u32)(uintptr_t)&AwardRowOff;
    u32 rowoff = 0;
    u8 *ptr = (u8 *)(idx72 + (base + off));
    u8 *dst = temp;
    u8 *rowp;
    u32 sum;
    u32 ro;
    register int i __asm__("r4");
    do {
        idx8 = (u32)(idx << 3);
        if (found != 1 && *(u32 *)pa < *(u32 *)ptr) {
            u8 *d = dst;
            __asm__ volatile ("" : "+r" (d));
            register u8 *sA __asm__("r1");
            sA = (u8 *)pa;
            __asm__ volatile ("" : "+r" (sA));
            volatile u16 *cell = (volatile u16 *)(uintptr_t)&AwardCell4C;
            *(Blk12 *)d = *(Blk12 *)sA;
            *cell = (u16)count;
            if ((u16)count == 0)
                *(int *)pb = 1;
            dst += 12;
            count++;
            found = 1;
        }
        if (count <= 4) {
            *(Blk12 *)dst = *(Blk12 *)(u8 *)(((idx8 + idx) << 3) + rowoff + base + off);
            dst += 12;
            count++;
        }
        rowoff += 12;
        ptr += 12;
    } while (count <= 4);
    {
        register u8 *wb __asm__("r1");
        wb = &AwardWaBase;
        __asm__ volatile ("" : "+r" (wb));
        sum = (u32)((idx8 + idx) << 3);
        __asm__ volatile ("" : : "r" (idx));
        dst = temp;
        rowp = (u8 *)(sum + (u32)(uintptr_t)wb);
    }
    ro = (u32)(uintptr_t)&AwardRowOff;
    i = 4;
    do {
        *(Blk12 *)(rowp + ro) = *(Blk12 *)dst;
        dst += 12;
        rowp += 12;
    } while (--i >= 0);
}

// _0800BB0C(idx, src) — twin B, 72-byte frame and the three WA flag cells.
// Entry stores the three flags (0xFC8/0xFCA/0xFCC) from the spilled idx and
// found slots, then the sorted 5+4 row copies run with src held in ip.
__asm__(".set AwardFlagFC8, 0x00000FC8");
__asm__(".set AwardFlagFCA, 0x00000FCA");
__asm__(".set AwardFlagFCC, 0x00000FCC");
extern u8 AwardFlagFC8;
extern u8 AwardFlagFCA;
extern u8 AwardFlagFCC;
void Award_CopyTwinB(int idx_, void *src_, void *unused) {
    typedef struct { u32 w[3]; } Blk12;
    __asm__(".globl AwardWaBase\nAwardWaBase = 0x03001780\n"
            ".globl AwardRowOff\nAwardRowOff = 0x000005E4\n"
            ".globl AwardCell4C\nAwardCell4C = 0x0300274C\n"
            ".globl AwardFlagFC8\nAwardFlagFC8 = 0x00000FC8\n"
            ".globl AwardFlagFCA\nAwardFlagFCA = 0x00000FCA\n"
            ".globl AwardFlagFCC\nAwardFlagFCC = 0x00000FCC\n");
    u8 temp[60];
    int idx = idx_;
    (void)&idx;
    register void *src __asm__("ip") = src_;
    u32 found = 0;
    (void)&found;
    u32 base = (u32)(uintptr_t)&AwardWaBase;
    u8 *flag8 = (u8 *)(base + (u32)(uintptr_t)&AwardFlagFC8);
    *(volatile u16 *)flag8 = (u16)found;
    *(volatile u16 *)(base + (u32)(uintptr_t)&AwardFlagFCA) = (u16)found;
    *(volatile u16 *)(base + (u32)(uintptr_t)&AwardFlagFCC) = (u16)found;
    int count = 0;
    u32 idx72 = (u32)(((idx << 3) + idx) << 3);
    register u32 rowoff __asm__("r8");
    rowoff = 0;
    u8 *ptr = (u8 *)(idx72 + ((u32)(uintptr_t)&AwardRowOff + base));
    u8 *dst = temp;
    u8 *rowp;
    u32 idx8;
    u32 sum, ro;
    register int i __asm__("r4");
    do {
        idx8 = (u32)(idx << 3);
        if (found != 1 && *(u32 *)src > *(u32 *)ptr) {
            *(Blk12 *)dst = *(Blk12 *)src;
            *(volatile u16 *)(uintptr_t)&AwardCell4C = (u16)count;
            if ((u16)count == 0)
                *(volatile u16 *)flag8 = 1;
            dst += 12;
            count++;
            found = 1;
        }
        if (count <= 4) {
            *(Blk12 *)dst = *(Blk12 *)(u8 *)(((idx8 + idx) << 3) + rowoff + base
                                             + (u32)(uintptr_t)&AwardRowOff);
            dst += 12;
            count++;
        }
        rowoff += 12;
        ptr += 12;
    } while (count <= 4);
    {
        register u8 *wb __asm__("r1");
        wb = &AwardWaBase;
        __asm__ volatile ("" : "+r" (wb));
        sum = (u32)((idx8 + idx) << 3);
        __asm__ volatile ("" : : "r" (idx));
        dst = temp;
        rowp = (u8 *)(sum + (u32)(uintptr_t)wb);
    }
    ro = (u32)(uintptr_t)&AwardRowOff;
    i = 4;
    do {
        *(Blk12 *)(rowp + ro) = *(Blk12 *)dst;
        dst += 12;
        rowp += 12;
    } while (--i >= 0);
}

// Original aliases
void _0800BA3C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinA")));
void _0800BB0C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinB")));
void sub_0800BA3C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinA")));
void sub_0800BB0C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinB")));
