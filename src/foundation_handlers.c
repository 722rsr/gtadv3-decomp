#include "gtadv/foundation.h"
#include "gba/types.h"
#include "gba/regs.h"
#include "gba/bios.h"

// EWRAM serial handlers — ROM 0x08000AF4/0x08000E80 copied to 0x0203EE70 via CpuFastSet 72 words (0x04000048)
// Executed via IntrMain slot0 combo 0xC0 (Timer3+Serial). Behavioral C mirrors asm/handlers.s

// State at 0x0203EF90 (see asm/agbmain.s / asm/handlers.s)
typedef struct { vu8 _00; vu8 fsm; vu8 _02; vu8 _03; vu8 ready; vu8 c5; vu8 flags; vu8 siocnt6; vu8 _08; vu8 latch9; vu8 _0A[4]; vu32 cnt14; vu32 cnt18; vu32 ptr1C; vu32 ptr20; vu32 ptr24; vu32 ptr28; } EwramSess;
#define SESS ((EwramSess*)0x0203EF90)

// Blob A — _08000AF4 (MultiSio 32-bit burst, timer3+serial).
// Exact C. Three homes the ROM splits and agbcc merges by default: the session
// pointer is materialised in r2 and copied to r4 once (`t` pinned to r2, then
// `st = t`), the zero shared by the three clears is a named local, and the
// count reloads use r0 (pinned `n`), with volatile reloads where the ROM
// re-reads a cell the loop could otherwise keep live. `bp`/`k` order the
// compare's two materialisations (buf address, then 0xFEFE) ahead of the copy.
void EWRAM_Handler_BlobA(void)
{
    u16 buf[4];
    volatile u32 *sio;
    u8 *st;
    register u8 *t __asm__("r2");
    register s32 n __asm__("r0");
    u32 v;
    u32 hi;
    u32 lo;
    u32 zero;
    s32 i;
    u16 *dst;
    u16 *src;
    u16 *bp;
    u32 k;
    u8 busy;
    volatile u16 *cnt;

    sio = (volatile u32 *)(uintptr_t)0x04000120u;
    hi = sio[1];
    lo = sio[0];
    *(u32 *)&buf[0] = lo;
    *(u32 *)&buf[2] = hi;
    t = (u8 *)(uintptr_t)0x0203EF90u;
    v = (*(volatile u32 *)(uintptr_t)0x04000128u << 25) >> 31;
    zero = 0;
    t[7] = (u8)v;
    bp = buf;
    k = 0xFEFEu;
    st = t;
    if (*bp == (u16)k) {
        if (*(s32 *)(st + 24) > 9) {
            *(s32 *)(st + 24) = -1;
            hi = *(u32 *)(st + 40);
            lo = *(u32 *)(st + 36);
            *(u32 *)(st + 40) = lo;
            *(u32 *)(st + 36) = hi;
            if (st[4] != 0) {
                hi = *(u32 *)(st + 32);
                lo = *(u32 *)(st + 28);
                *(u32 *)(st + 32) = lo;
                *(u32 *)(st + 28) = hi;
                st[4] = zero;
                *(u32 *)(st + 20) = zero;
            }
            *(volatile u16 *)(uintptr_t)0x04000208u = zero;
            *(volatile u16 *)(uintptr_t)0x03007FF8u |= 0x80;
            *(volatile u16 *)(uintptr_t)0x04000208u = 1;
        }
    }
    n = *(s32 *)(st + 20);
    if (n <= 9) {
        cnt = (volatile u16 *)(uintptr_t)0x04000128u;
        cnt[1] = *(u16 *)(*(u32 *)(st + 32) + n * 2);
    }
    n = *(volatile s32 *)(st + 20);
    if (n <= 10)
        *(s32 *)(st + 20) = n + 1;
    n = *(s32 *)(st + 24);
    if (n >= 0) {
        u32 base;

        base = *(u32 *)(st + 36);
        src = buf;
        dst = (u16 *)(base + n * 2);
        for (i = 0; i <= 3; i++) {
            *dst = *src;
            src++;
            dst = (u16 *)((u8 *)dst + 24);
        }
        if (*(volatile s32 *)(st + 24) == 9)
            st[5] = 1;
    }
    n = *(volatile s32 *)(st + 24);
    if (n <= 10)
        *(s32 *)(st + 24) = n + 1;
    busy = st[0];
    if (busy != 0)
        *(volatile u16 *)(uintptr_t)0x0400010Eu = 0;
    if (*(volatile s32 *)(st + 20) <= 10) {
        if (busy != 0) {
            *(volatile u16 *)(uintptr_t)0x04000128u |= 0x80;
            *(volatile u16 *)(uintptr_t)0x0400010Eu = 192;
        }
    }
    st[9] = 1;
}
void _08000AF4(void) __attribute__((alias("EWRAM_Handler_BlobA")));

// Blob B — _08000E80 (SIO-normal polling, 0x0203F150 state).
// The record fields used here are the mode byte at +0, the descriptor pointer
// at +4, the signed receive count at +8, and the overflow slot at +12.
typedef struct {
    vu8 f00;
    vu8 _01[3];
    vu32 p04;
    vu32 count08;
    vu32 overflow0C;
} EwramF150;
#define F150 ((EwramF150 *)(uintptr_t)0x0203F150)

// The ROM keeps RCNT in r2 only on the mode-1 edge; the normal arm reuses r2
// for its signed count. Splitting the count into path-local variables models
// that disjoint liveness without a spill. r3 holds the received word, r5 the
// original record, and r4 the working copy.
//
// The tautological guard in mode1_over is load-bearing for block layout. It
// gives the later `neg` block a second CFG predecessor while merge_blocks runs,
// so agbcc keeps the ROM's normal/mode1/.../neg/store order. A later pass folds
// `x | ~x` to -1 and emits the ROM's unconditional tail branch. A plain
// `goto tail` lets merge_blocks move `neg` upward and deletes that branch.
void EWRAM_Handler_BlobB(void) {
    register volatile u32 *rcnt __asm__("r2") =
        (volatile u32 *)0x04000120u;
    register u32 rx __asm__("r3") = *rcnt;
    register EwramF150 *st __asm__("r5") = F150;
    register EwramF150 *rec __asm__("r4") = st;
    register s32 normal_v __asm__("r2");
    s32 mode_v;
    s32 tail_v;

    if (st->f00 == 1)
        goto mode1;
    *(volatile u16 *)0x04000128u |= 0x80u;
    normal_v = (s32)rec->count08;
    if (normal_v >= 0)
        goto store_rx;
    goto neg;

mode1:
    *(volatile u16 *)0x0400010Eu = 0;
    mode_v = (s32)rec->count08;
    if (mode_v >= 0)
        goto mode1_count;
    *rcnt = 0xFEFEFEFEu;
    goto tail;

mode1_count:
    if (mode_v > 0x1FFF)
        goto mode1_over;
    *rcnt = *(volatile u32 *)((u8 *)rec->p04 + mode_v * 4);
    goto tail;

mode1_over:
    *rcnt = rec->overflow0C;
    if ((mode_v | ~mode_v) == -1)
        goto tail;

neg:
    if (rx == 0xFEFEFEFEu)
        goto tail;
    {
        register u32 dec __asm__("r0") = (u32)normal_v - 1u;
        st->count08 = dec;
    }
    goto tail;

store_rx:
    if (normal_v > 0x1FFF)
        goto store_over;
    {
        register volatile u32 *table __asm__("r1") =
            (volatile u32 *)rec->p04;
        register u32 off __asm__("r0") = (u32)normal_v * 4u;
        *(volatile u32 *)(off + (u32)(uintptr_t)table) = rx;
    }
    goto tail;

store_over:
    rec->overflow0C = rx;

tail:
    tail_v = (s32)rec->count08;
    if (tail_v > 0x2002)
        goto done;
    rec->count08 = (u32)(tail_v + 1);
    {
        register u8 mode __asm__("r4") = rec->f00;
        if (mode == 1) {
            *(volatile u16 *)0x04000128u |= 0x80u;
            *(volatile u16 *)0x0400010Eu = 0xC0u;
        }
    }

done:
    ;
}
void _08000E80(void) __attribute__((alias("EWRAM_Handler_BlobB")));

void InstallEWRAMHandlers(int checksumMode, u32 inputBase) {
    extern void sub_0802D974(const void *src, void *dst, u32 ctrl);
    register int mode __asm__("r5") = checksumMode;
    const volatile u32 *input;
    register u32 sum __asm__("r6");
    register u32 one __asm__("r8");
    register volatile u16 *ime __asm__("r3");
    register volatile u16 *ie __asm__("r2");
    register volatile u32 *handlers __asm__("r4");
    u32 zero;

    __asm__ volatile ("" : "+r" (mode), "=r" (input)
                      : "1" ((const volatile u32 *)(uintptr_t)inputBase));
    sum = 0;
    ime = (volatile u16 *)(uintptr_t)0x04000208u;
    *ime = (u16)sum;
    __asm__ volatile ("" ::: "memory");
    ie = (volatile u16 *)(uintptr_t)0x04000200u;
    *ie &= 0xFF3Fu;
    one = 1;
    *ime = 1;
    zero = sum;
    handlers = (volatile u32 *)(uintptr_t)0x0203F150u;
    sub_0802D974(&zero, (void *)handlers, 0x05000006u);
    sub_0802D974((const void *)(uintptr_t)0x08000E81u,
                 (void *)(uintptr_t)0x0203EE70u, 0x04000048u);
    *(volatile u32 *)(uintptr_t)0x04000128u = 0x00002003u;
    handlers[1] = (u32)(uintptr_t)input;
    handlers[2] = ~0u;
    if (mode != 0) {
        register volatile u32 *checksum __asm__("r0") =
            (volatile u32 *)(uintptr_t)0x0400010Cu;
        register u32 byteOne __asm__("r2");
        *checksum = (u32)sum;
        __asm__ volatile ("" ::: "r2", "memory");
        byteOne = one;
        *(volatile u8 *)handlers = (u8)byteOne;
        {
            register const volatile u32 *sumInput __asm__("r1") = input;
            u32 count = 0x2000u;
            do {
                sum += *sumInput++;
                count--;
            } while (count != 0);
        }
        {
            register u32 complement __asm__("r0") = ~sum;
            register volatile u16 *vectorIE __asm__("r1");
            register u32 vectorValue __asm__("r2");
            register u32 vectorStore __asm__("r0");
            handlers[3] = complement;
            __asm__ volatile ("" ::: "r1", "r2", "memory");
            vectorIE = (volatile u16 *)(uintptr_t)0x04000128u;
            vectorValue = 0x1000u;
            vectorStore = vectorValue;
            *vectorIE = (u16)vectorStore;
            vectorValue++;
            __asm__ volatile ("" : "+r" (vectorValue));
            vectorStore = vectorValue;
            *vectorIE = (u16)vectorStore;
        }
    }
}
void _08000BF0(int checksumMode, u32 inputBase)
    __attribute__((alias("InstallEWRAMHandlers")));
