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

// Blob B — _08000E80 (SIO-normal polling, 0x0203F150 state)
typedef struct { vu8 f00; vu8 _01[3]; vu32 p04; vu32 _08; vu32 cnt08; vu32 _0C; } EwramF150;
#define F150 ((EwramF150*)0x0203F150)
// MEASURED 2026-10-29: C reaches 196/200 with the three SIODATA32 stores kept
// apart by a bare `__asm__ volatile ("")` after the first two -- the earlier
// 192-byte reading was agbcc's cross-jump merging them into one shared copy.
// What is left is BLOCK LAYOUT only: the ROM puts the mode-0 negative arm
// (`*[r5+8] = v-1`) after the mode-1 blocks at 0x08000EEA, i.e.
// [head][mode1][neg][store chain]. Four spellings were measured -- if/else-if,
// `if (v >= 0) goto store; goto neg;`, a `goto`-only else, and a `switch` with
// `default` first -- and agbcc emits [head][neg][mode1][store] for all of them:
// it places a trailing `goto`'s target block immediately after the block that
// jumps to it, so the else block is always pushed past the join. The head's
// two-branch dispatch (`bge store; b neg`) is what armcc produced; agbcc never
// emits both arms out of line. Do not re-derive the 196-byte body.
__attribute__((naked)) void EWRAM_Handler_BlobB(void)
{
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "ldr r2, .L0e80_p0\n"
        "ldr r3, [r2, #0]\n"
        "ldr r5, .L0e80_p1\n"
        "adds r4, r5, #0\n"
        "ldrb r0, [r5, #0]\n"
        "cmp r0, #1\n"
        "beq .L0e80_mode1\n"
        "ldr r0, .L0e80_p2\n"
        "ldrh r1, [r0, #0]\n"
        "movs r2, #128\n"
        "orrs r1, r2\n"
        "strh r1, [r0, #0]\n"
        "ldr r2, [r4, #8]\n"
        "cmp r2, #0\n"
        "bge .L0e80_store_rx\n"
        "b .L0e80_rx_negative\n"
        ".align 2, 0\n"
        ".L0e80_p0: .4byte 0x04000120\n"
        ".L0e80_p1: .4byte 0x0203F150\n"
        ".L0e80_p2: .4byte 0x04000128\n"
        ".L0e80_mode1:\n"
        "ldr r1, .L0e80_p3\n"
        "movs r0, #0\n"
        "strh r0, [r1, #0]\n"
        "ldr r1, [r4, #8]\n"
        "cmp r1, #0\n"
        "bge .L0e80_mode1_count\n"
        "ldr r0, .L0e80_p4\n"
        "str r0, [r2, #0]\n"
        "b .L0e80_tail\n"
        ".align 2, 0\n"
        ".L0e80_p3: .4byte 0x0400010E\n"
        ".L0e80_p4: .4byte 0xFEFEFEFE\n"
        ".L0e80_mode1_count:\n"
        "ldr r0, .L0e80_p5\n"
        "cmp r1, r0\n"
        "bgt .L0e80_mode1_over\n"
        "ldr r0, [r4, #4]\n"
        "lsls r1, r1, #2\n"
        "adds r1, r1, r0\n"
        "ldr r0, [r1, #0]\n"
        "str r0, [r2, #0]\n"
        "b .L0e80_tail\n"
        ".align 2, 0\n"
        ".L0e80_p5: .4byte 0x00001FFF\n"
        ".L0e80_mode1_over:\n"
        "ldr r0, [r4, #12]\n"
        "str r0, [r2, #0]\n"
        "b .L0e80_tail\n"
        ".L0e80_rx_negative:\n"
        "ldr r0, .L0e80_p6\n"
        "cmp r3, r0\n"
        "beq .L0e80_tail\n"
        "subs r0, r2, #1\n"
        "str r0, [r5, #8]\n"
        "b .L0e80_tail\n"
        ".align 2, 0\n"
        ".L0e80_p6: .4byte 0xFEFEFEFE\n"
        ".L0e80_store_rx:\n"
        "ldr r0, .L0e80_p7\n"
        "cmp r2, r0\n"
        ".L0e80_store_rx_cmp:\n"
        "bgt .L0e80_store_over\n"
        "ldr r1, [r4, #4]\n"
        "lsls r0, r2, #2\n"
        "adds r0, r0, r1\n"
        "str r3, [r0, #0]\n"
        "b .L0e80_tail\n"
        ".align 2, 0\n"
        ".L0e80_p7: .4byte 0x00001FFF\n"
        ".L0e80_store_over:\n"
        "str r3, [r4, #12]\n"
        ".L0e80_tail:\n"
        "ldr r1, [r4, #8]\n"
        "ldr r0, .L0e80_p8\n"
        "cmp r1, r0\n"
        "bgt .L0e80_done\n"
        "adds r0, r1, #1\n"
        "str r0, [r4, #8]\n"
        "ldrb r4, [r4, #0]\n"
        "cmp r4, #1\n"
        "bne .L0e80_done\n"
        "ldr r2, .L0e80_p9\n"
        "ldrh r0, [r2, #0]\n"
        "movs r1, #128\n"
        "orrs r0, r1\n"
        "strh r0, [r2, #0]\n"
        "ldr r1, .L0e80_p10\n"
        "movs r0, #192\n"
        "strh r0, [r1, #0]\n"
        ".L0e80_done:\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".L0e80_p8: .4byte 0x00002002\n"
        ".L0e80_p9: .4byte 0x04000128\n"
        ".L0e80_p10: .4byte 0x0400010E\n"
        ".syntax divided\n"
    );
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
