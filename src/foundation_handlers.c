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
// MEASURED 2026-10-22: a full C lift reaches 163/252 with the candidate length
// exact (`SioBlock` copy out of 0x04000120, non-volatile SIO halves so the
// stores are plain `strh`, and a memory barrier between the row loop and the
// `cnt24 == 9` re-read). Every remaining difference is a register choice:
// the session pointer's first pseudo (ROM `r2`, then `adds r4,r2`), the zero
// shared by three stores (ROM `movs r5,#0`), and the `cnt24`/`cnt20` reloads
// (ROM `r0`, agbcc homes them in r5/r6 because the loop keeps them live).
// Pinning the session pointer to r4 costs the `adds r4,r2` copy and drops the
// score; pinning a zero-valued local is folded away. Leaving the body naked
// until a shape splits those three homes.
__attribute__((naked)) void EWRAM_Handler_BlobA(void)
{
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "sub sp, #8\n"
        "ldr r0, .L0af4_p0\n"
        "ldr r1, [r0, #4]\n"
        "ldr r0, [r0, #0]\n"
        "str r0, [sp, #0]\n"
        "str r1, [sp, #4]\n"
        "ldr r2, .L0af4_p1\n"
        "ldr r0, .L0af4_p2\n"
        "ldr r0, [r0, #0]\n"
        "lsls r0, r0, #25\n"
        "lsrs r0, r0, #31\n"
        "movs r5, #0\n"
        "strb r0, [r2, #7]\n"
        "mov r1, sp\n"
        "ldr r0, .L0af4_p3\n"
        "adds r4, r2, #0\n"
        "ldrh r1, [r1, #0]\n"
        "cmp r1, r0\n"
        "bne .L0af4_mid\n"
        "ldr r0, [r4, #24]\n"
        "cmp r0, #9\n"
        "ble .L0af4_mid\n"
        "movs r0, #1\n"
        "negs r0, r0\n"
        "str r0, [r4, #24]\n"
        "ldr r1, [r4, #40]\n"
        "ldr r0, [r4, #36]\n"
        "str r0, [r4, #40]\n"
        "str r1, [r4, #36]\n"
        "ldrb r0, [r4, #4]\n"
        "cmp r0, #0\n"
        "beq .L0af4_ime\n"
        "ldr r1, [r4, #32]\n"
        "ldr r0, [r4, #28]\n"
        "str r0, [r4, #32]\n"
        "str r1, [r4, #28]\n"
        "strb r5, [r4, #4]\n"
        "str r5, [r4, #20]\n"
        ".L0af4_ime:\n"
        "ldr r3, .L0af4_p4\n"
        "strh r5, [r3, #0]\n"
        "ldr r2, .L0af4_p5\n"
        "ldrh r0, [r2, #0]\n"
        "movs r1, #128\n"
        "orrs r0, r1\n"
        "strh r0, [r2, #0]\n"
        "movs r0, #1\n"
        "strh r0, [r3, #0]\n"
        ".L0af4_mid:\n"
        "ldr r0, [r4, #20]\n"
        "cmp r0, #9\n"
        "bgt .L0af4_after_rx\n"
        "ldr r2, .L0af4_p2\n"
        "ldr r1, [r4, #32]\n"
        "lsls r0, r0, #1\n"
        "adds r0, r0, r1\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r2, #2]\n"
        ".L0af4_after_rx:\n"
        "ldr r0, [r4, #20]\n"
        "cmp r0, #10\n"
        "bgt .L0af4_after_cnt14\n"
        "adds r0, #1\n"
        "str r0, [r4, #20]\n"
        ".L0af4_after_cnt14:\n"
        "ldr r0, [r4, #24]\n"
        "cmp r0, #0\n"
        "blt .L0af4_after_tx\n"
        "ldr r1, [r4, #36]\n"
        "mov r2, sp\n"
        "lsls r0, r0, #1\n"
        "adds r1, r1, r0\n"
        "movs r3, #3\n"
        ".L0af4_tx_loop:\n"
        "ldrh r0, [r2, #0]\n"
        "strh r0, [r1, #0]\n"
        "adds r2, #2\n"
        "adds r1, #24\n"
        "subs r3, #1\n"
        "cmp r3, #0\n"
        "bge .L0af4_tx_loop\n"
        "ldr r0, [r4, #24]\n"
        "cmp r0, #9\n"
        "bne .L0af4_after_tx\n"
        "movs r0, #1\n"
        "strb r0, [r4, #5]\n"
        ".L0af4_after_tx:\n"
        "ldr r0, [r4, #24]\n"
        "cmp r0, #10\n"
        "bgt .L0af4_after_latch\n"
        "adds r0, #1\n"
        "str r0, [r4, #24]\n"
        ".L0af4_after_latch:\n"
        "ldrb r2, [r4, #0]\n"
        "cmp r2, #0\n"
        "beq .L0af4_serial\n"
        "ldr r1, .L0af4_p6\n"
        "movs r0, #0\n"
        "strh r0, [r1, #0]\n"
        ".L0af4_serial:\n"
        "ldr r0, [r4, #20]\n"
        "cmp r0, #10\n"
        "bgt .L0af4_done_serial\n"
        "cmp r2, #0\n"
        "beq .L0af4_done_serial\n"
        "ldr r2, .L0af4_p2\n"
        "ldrh r0, [r2, #0]\n"
        "movs r1, #128\n"
        "orrs r0, r1\n"
        "strh r0, [r2, #0]\n"
        "ldr r1, .L0af4_p6\n"
        "movs r0, #192\n"
        "strh r0, [r1, #0]\n"
        ".L0af4_done_serial:\n"
        "movs r0, #1\n"
        "strb r0, [r4, #9]\n"
        "add sp, #8\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".L0af4_p0: .4byte 0x04000120\n"
        ".L0af4_p1: .4byte 0x0203ef90\n"
        ".L0af4_p2: .4byte 0x04000128\n"
        ".L0af4_p3: .4byte 0x0000fefe\n"
        ".L0af4_p4: .4byte 0x04000208\n"
        ".L0af4_p5: .4byte 0x03007ff8\n"
        ".L0af4_p6: .4byte 0x0400010e\n"
        ".syntax divided\n"
    );
}
void _08000AF4(void) __attribute__((alias("EWRAM_Handler_BlobA")));

// Blob B — _08000E80 (SIO-normal polling, 0x0203F150 state)
typedef struct { vu8 f00; vu8 _01[3]; vu32 p04; vu32 _08; vu32 cnt08; vu32 _0C; } EwramF150;
#define F150 ((EwramF150*)0x0203F150)
// MEASURED 2026-10-22: the C lift reaches 192/200; every remaining difference
// is a store agbcc cross-jumps and the ROM keeps. Both modes end by writing
// SIODATA32 (`str r0,[r2]` at +0x3C, +0x56, +0x60) and the three blocks are
// identical in that last instruction, so agbcc merges them into one shared
// copy and skips 8 bytes. Nothing in the C spelling separates them: the stored
// values differ, the destination does not.
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
