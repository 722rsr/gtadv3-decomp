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
// The body is the ROM-derived high-register transfer loop; its pool follows
// the epilogue exactly as in asm/award_twins.s.
__attribute__((naked)) void Award_CopyTwinB(int idx, void *src, void *unused) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, sl\n"
        "mov r6, r9\n"
        "mov r5, r8\n"
        "push {r5, r6, r7}\n"
        "sub sp, #72\n"
        "str r0, [sp, #60]\n"
        "mov ip, r1\n"
        "movs r0, #0\n"
        "str r0, [sp, #64]\n"
        "ldr r1, 8f\n"
        "ldr r2, 9f\n"
        "adds r4, r1, r2\n"
        "strh r0, [r4, #0]\n"
        "ldr r5, 10f\n"
        "adds r0, r1, r5\n"
        "add r6, sp, #64\n"
        "ldrh r6, [r6, #0]\n"
        "strh r6, [r0, #0]\n"
        "ldr r7, 11f\n"
        "adds r3, r1, r7\n"
        "add r0, sp, #64\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r3, #0]\n"
        "movs r5, #0\n"
        "ldr r2, [sp, #60]\n"
        "lsls r0, r2, #3\n"
        "adds r0, r0, r2\n"
        "lsls r0, r0, #3\n"
        "ldr r6, 12f\n"
        "adds r6, r6, r1\n"
        "mov r8, r5\n"
        "adds r3, r0, r6\n"
        "mov r2, sp\n"
        "mov sl, r4\n"
        "mov r9, r1\n"
        "1:\n"
        "ldr r7, [sp, #60]\n"
        "lsls r7, r7, #3\n"
        "str r7, [sp, #68]\n"
        "ldr r0, [sp, #64]\n"
        "cmp r0, #1\n"
        "beq.n 2f\n"
        "mov r4, ip\n"
        "ldr r1, [r4, #0]\n"
        "ldr r0, [r3, #0]\n"
        "cmp r1, r0\n"
        "bls.n 2f\n"
        "adds r0, r2, #0\n"
        "mov r1, ip\n"
        "ldmia r1!, {r4, r6, r7}\n"
        "stmia r0!, {r4, r6, r7}\n"
        "ldr r6, 13f\n"
        "strh r5, [r6, #0]\n"
        "lsls r0, r5, #16\n"
        "cmp r0, #0\n"
        "bne.n 3f\n"
        "movs r0, #1\n"
        "mov r7, sl\n"
        "strh r0, [r7, #0]\n"
        "3:\n"
        "adds r2, #12\n"
        "adds r5, #1\n"
        "movs r0, #1\n"
        "str r0, [sp, #64]\n"
        "2:\n"
        "cmp r5, #4\n"
        "bgt.n 4f\n"
        "5:\n"
        "ldr r1, [sp, #68]\n"
        "ldr r4, [sp, #60]\n"
        "adds r0, r1, r4\n"
        "lsls r0, r0, #3\n"
        "add r0, r8\n"
        "add r0, r9\n"
        "adds r1, r2, #0\n"
        "ldr r6, 12f\n"
        "adds r0, r0, r6\n"
        "ldmia r0!, {r4, r6, r7}\n"
        "stmia r1!, {r4, r6, r7}\n"
        "adds r2, #12\n"
        "adds r5, #1\n"
        "4:\n"
        "movs r7, #12\n"
        "add r8, r7\n"
        "adds r3, #12\n"
        "cmp r5, #4\n"
        "ble.n 1b\n"
        "6:\n"
        "ldr r1, 8f\n"
        "ldr r2, [sp, #68]\n"
        "ldr r3, [sp, #60]\n"
        "adds r0, r2, r3\n"
        "lsls r0, r0, #3\n"
        "mov r3, sp\n"
        "adds r2, r0, r1\n"
        "ldr r4, 12f\n"
        "mov r8, r4\n"
        "movs r4, #4\n"
        "7:\n"
        "mov r5, r8\n"
        "adds r1, r2, r5\n"
        "adds r0, r3, #0\n"
        "ldmia r0!, {r5, r6, r7}\n"
        "stmia r1!, {r5, r6, r7}\n"
        "adds r3, #12\n"
        "adds r2, #12\n"
        "subs r4, #1\n"
        "cmp r4, #0\n"
        "bge.n 7b\n"
        "add sp, #72\n"
        "pop {r3, r4, r5}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "mov sl, r5\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".short 0x0000\n"
        ".align 2\n"
        "8: .word 0x03001780\n"
        "9: .word 0x00000FC8\n"
        "10: .word 0x00000FCA\n"
        "11: .word 0x00000FCC\n"
        "12: .word 0x000005E4\n"
        "13: .word 0x0300274C\n"
        ".syntax divided\n"
    );
}

// Original aliases
void _0800BA3C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinA")));
void _0800BB0C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinB")));
void sub_0800BA3C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinA")));
void sub_0800BB0C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinB")));
