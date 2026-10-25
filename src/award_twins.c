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
    volatile int idx = idx_;
    register void *src __asm__("ip") = src_;
    volatile u32 found = 0;

    register u32 base0 __asm__("r1") = (u32)(uintptr_t)&AwardWaBase;
    register u32 fc8c __asm__("r2") = (u32)(uintptr_t)&AwardFlagFC8;
    register u32 f8p __asm__("r4") = base0 + fc8c;
    *(volatile u16 *)(uintptr_t)f8p = 0;
    register u32 fcac __asm__("r5") = (u32)(uintptr_t)&AwardFlagFCA;
    register u32 fap __asm__("r0") = base0 + fcac;
    register u16 *f6 __asm__("r6") = (u16 *)((u8 *)&found);
    register u16 f6v __asm__("r6") = *f6;
    *(volatile u16 *)(uintptr_t)fap = f6v;
    register u32 fccc __asm__("r7") = (u32)(uintptr_t)&AwardFlagFCC;
    register u32 fccp __asm__("r3") = base0 + fccc;
    *(volatile u16 *)(uintptr_t)fccp =
        *(volatile u16 *)((u8 *)&found);

    register int count __asm__("r5") = 0;
    register u32 idxv __asm__("r2") = (u32)idx;
    register u32 idx72 __asm__("r0") =
        (u32)(((idxv << 3) + idxv) << 3);
    register u32 baseoff __asm__("r6") =
        (u32)(uintptr_t)&AwardRowOff;
    baseoff += base0;
    register u32 rowoff __asm__("r8") = 0;
    register u8 *ptr __asm__("r3") = (u8 *)(idx72 + baseoff);
    register u8 *dst __asm__("r2") = temp;
    register u8 *flag8 __asm__("sl");
    register u32 base __asm__("r9");
    flag8 = (u8 *)(uintptr_t)f8p;
    base = base0;
    u32 idx8;
    u8 *rowp;
    u32 sum, ro;
    register int i __asm__("r4");

    do {
        register u32 t7 __asm__("r7") = (u32)idx;
        t7 <<= 3;
        idx8 = t7;
        if (found != 1) {
            register u8 *s4 __asm__("r4") = (u8 *)src;
            if (*(u32 *)s4 > *(u32 *)ptr) {
                register u8 *d __asm__("r0") = dst;
                register u8 *sB __asm__("r1");
                sB = (u8 *)src;
                __asm__ volatile ("" : "+r" (sB));
                *(Blk12 *)d = *(Blk12 *)sB;
                register u16 *cell6 __asm__("r6") =
                    (u16 *)(uintptr_t)&AwardCell4C;
                *cell6 = (u16)count;
                if ((u16)count == 0) {
                    register u32 hold1 __asm__("r1");
                    register u32 hold4 __asm__("r4");
                    register u32 hold6 __asm__("r6");
                    __asm__ volatile ("" : "=r" (hold1), "=r" (hold4),
                                      "=r" (hold6));
                    register u16 one __asm__("r0") = 1;
                    *(volatile u16 *)flag8 = one;
                    __asm__ volatile ("" : "+r" (hold1), "+r" (hold4),
                                      "+r" (hold6));
                }
                dst += 12;
                count++;
                found = 1;
            }
        }
        if (count <= 4) {
            register u32 a1 __asm__("r1") = idx8;
            __asm__ volatile ("" : "+r" (a1));
            register u32 a4 __asm__("r4") = (u32)idx;
            register u32 a0 __asm__("r0") = a1 + a4;
            a0 <<= 3;
            a0 += rowoff;
            a0 += base;
            register u8 *d1 __asm__("r1") = dst;
            __asm__ volatile ("" : "+r" (d1));
            register u32 off6 __asm__("r6") =
                (u32)(uintptr_t)&AwardRowOff;
            a0 += off6;
            *(Blk12 *)d1 = *(Blk12 *)(uintptr_t)a0;
            dst += 12;
            count++;
        }
        register u32 twelve __asm__("r7") = 12;
        /* r7 is agbcc's frame pointer: once cse learns `r7 == 12` it folds
           every later frame-address materialisation (the tail's
           `tsrc = temp`) to a constant. The barrier keeps twelve opaque. */
        __asm__ volatile ("" : "+r" (twelve));
        rowoff += twelve;
        ptr += 12;
    } while (count <= 4);

    // The tail's shape is all pins/barriers now (no spliced instructions):
    // cse folds `tsrc = temp` to a constant unless twelve's r7 is opaque
    // (r7 is agbcc's frame pointer -- see the barrier in the first loop),
    // trow must land in r2 and the row-offset copy/add in r5/r1 or agbcc
    // coalesces them into the dead t0/wb registers.
    {
        register u8 *wb __asm__("r1") = (u8 *)(uintptr_t)&AwardWaBase;
        register u32 t2 __asm__("r2") = idx8;
        __asm__ volatile ("" : "+r" (t2));
        register u32 t3 __asm__("r3") = (u32)idx;
        register u32 t0 __asm__("r0") = (u32)((t2 + t3) << 3);
        register u8 *tsrc __asm__("r3") = temp;
        __asm__ volatile ("" : "+r" (tsrc));
        register u8 *trow __asm__("r2") = (u8 *)(t0 + (u32)(uintptr_t)wb);
        register u32 offv __asm__("r4") = (u32)(uintptr_t)&AwardRowOff;
        __asm__ volatile ("" : "+r" (offv));
        register u32 ro8 __asm__("r8") = offv;
        __asm__ volatile ("" : "+r" (ro8));
        i = 4;
        do {
            /* ROM: `mov r5, r8 / adds r1, r2, r5` -- r8 is high, so the
               3-operand Thumb add needs the offset copied down first. */
            register u32 ro __asm__("r5") = ro8;
            __asm__ volatile ("" : "+r" (ro));
            register u8 *dstp __asm__("r1") = trow + ro;
            __asm__ volatile ("" : "+r" (dstp));
            *(Blk12 *)dstp = *(Blk12 *)tsrc;
            tsrc += 12;
            trow += 12;
        } while (--i >= 0);
    }
}

// Original aliases
void _0800BA3C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinA")));
void _0800BB0C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinB")));
void sub_0800BA3C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinA")));
void sub_0800BB0C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinB")));
