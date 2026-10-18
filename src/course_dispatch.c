#include "gba/types.h"
#include "gba/bios.h"

// Course dispatch — VMA 0x08008AAC–0x080095F0
// Mechanical translation with opaque volatile byte pointers, exact widths, explicit stack args, table strides, direct helper ABIs.

extern int sub_08002BE8(void); // u32 random, s32

__attribute__((naked)) void Course_Dispatch_08AAC(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "adds r4, r0, #0\n"
        "bl 0x08002be8\n"
        "lsls r0, r0, #16\n"
        "asrs r5, r0, #16\n"
        "bl 0x08002be8\n"
        "lsls r0, r0, #16\n"
        "asrs r6, r0, #16\n"
        "ldr r1, 1f\n"
        "ldr r7, 2f\n"
        "ldr r0, [r7, #0]\n"
        "movs r2, #22\n"
        "ldrsh r0, [r0, r2]\n"
        "lsls r0, r0, #1\n"
        "adds r0, r0, r1\n"
        "ldrh r1, [r0, #0]\n"
        "adds r0, r5, #0\n"
        "bl 0x08002c48\n"
        "ldr r1, 3f\n"
        "ldr r0, [r7, #0]\n"
        "movs r3, #22\n"
        "ldrsh r0, [r0, r3]\n"
        "lsls r0, r0, #1\n"
        "adds r0, r0, r1\n"
        "ldrh r1, [r0, #0]\n"
        "adds r0, r6, #0\n"
        "bl 0x08002c48\n"
        "ldr r2, [r7, #0]\n"
        "movs r1, #16\n"
        "ldrsh r0, [r2, r1]\n"
        "cmp r0, #1\n"
        "ble.n .L8aac_after_pool\n"
        "ldr r0, [r2, #84]\n"
        "subs r0, #16\n"
        "movs r1, #255\n"
        "ands r0, r1\n"
        "movs r3, #134\n"
        "lsls r3, r3, #7\n"
        "adds r1, r3, #0\n"
        "orrs r0, r1\n"
        "strh r0, [r4, #0]\n"
        "ldr r0, [r2, #80]\n"
        "subs r0, #20\n"
        "ldr r3, 4f\n"
        "adds r1, r3, #0\n"
        "ands r0, r1\n"
        "ldr r3, 5f\n"
        "adds r1, r3, #0\n"
        "orrs r0, r1\n"
        "lsls r1, r6, #9\n"
        "orrs r0, r1\n"
        "strh r0, [r4, #2]\n"
        "ldrh r1, [r2, #58]\n"
        "lsls r0, r1, #12\n"
        "ldrh r3, [r2, #60]\n"
        "orrs r0, r3\n"
        "strh r0, [r4, #4]\n"
        "adds r4, #8\n"
        "movs r1, #16\n"
        "ldrsh r0, [r2, r1]\n"
        "lsls r3, r5, #9\n"
        "cmp r0, #9\n"
        "ble.n .L8aac_second\n"
        "ldr r1, 6f\n"
        "adds r0, r1, #0\n"
        "strh r0, [r4, #0]\n"
        "ldr r0, 7f\n"
        "adds r1, r0, #0\n"
        "adds r0, r3, #0\n"
        "orrs r0, r1\n"
        "strh r0, [r4, #2]\n"
        "ldrh r0, [r2, #56]\n"
        "adds r0, #16\n"
        "ldrh r2, [r2, #58]\n"
        "lsls r1, r2, #12\n"
        "orrs r0, r1\n"
        "strh r0, [r4, #4]\n"
        "adds r4, #8\n"
        ".L8aac_second:\n"
        "ldr r1, 6f\n"
        "adds r0, r1, #0\n"
        "strh r0, [r4, #0]\n"
        "ldr r2, 8f\n"
        "adds r0, r2, #0\n"
        "orrs r3, r0\n"
        "strh r3, [r4, #2]\n"
        "ldr r1, [r7, #0]\n"
        "ldrh r3, [r1, #58]\n"
        "lsls r0, r3, #12\n"
        "ldrh r1, [r1, #56]\n"
        "orrs r0, r1\n"
        "b.n .L8aac_tail_store\n"
        ".align 2, 0\n"
        "1: .word 0x080cb110\n"
        "2: .word 0x030003e0\n"
        "3: .word 0x080cb132\n"
        "4: .word 0x000001ff\n"
        "5: .word 0xffffc000\n"
        "6: .word 0x0000033a\n"
        "7: .word 0xffff8084\n"
        "8: .word 0xffff8098\n"
        ".L8aac_after_pool:\n"
        "ldr r1, 9f\n"
        "adds r0, r1, #0\n"
        "strh r0, [r4, #0]\n"
        "lsls r0, r5, #9\n"
        "ldr r3, 10f\n"
        "adds r1, r3, #0\n"
        "orrs r0, r1\n"
        "strh r0, [r4, #2]\n"
        "ldrh r1, [r2, #58]\n"
        "lsls r0, r1, #12\n"
        "ldrh r2, [r2, #56]\n"
        "orrs r0, r2\n"
        ".L8aac_tail_store:\n"
        "strh r0, [r4, #4]\n"
        "adds r4, #8\n"
        "adds r0, r4, #0\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r1}\n"
        "bx r1\n"
        ".align 2, 0\n"
        "9: .word 0x0000433a\n"
        "10: .word 0xffffc088\n"
        ".syntax divided\n"
    );
}
void _08008AAC(void *a) __attribute__((alias("Course_Dispatch_08AAC")));
void sub_08008AAC(void *a) __attribute__((alias("Course_Dispatch_08AAC")));

void *Course_Dispatch_08BB8(void *out) {
    register volatile u16 *cursor __asm__("r3") = (volatile u16 *)out;
    register volatile u8 * volatile *root __asm__("ip") =
        (volatile u8 * volatile *)0x030003e0u;
    register volatile u8 *rec __asm__("r2") = *root;
    register u32 reg1 __asm__("r1");
    register s32 value __asm__("r0");
    register u32 word4 __asm__("r4");
    register u32 word5 __asm__("r5");
    register u32 mask __asm__("r6");
    // word7/temp7 both encode the ROM's r7 scratch use: word7 (pinned) carries
    // the shifted/loaded temps whose copies to r1 are real instructions, and
    // temp7 spans two blocks so global-alloc must place it in r7, whose
    // mark_home_live also restores the ROM's r7 prologue save.
    register u32 word7 __asm__("r7");
    register u32 temp7;
    register u32 mask8 __asm__("r8");

    value = *(s16 *)(rec + 16);
    if (value > 1) {
        word5 = *(volatile u32 *)(rec + 84);
        word4 = 255;
        __asm__ volatile ("" : "+r"(word4));
        mask8 = word4;
        __asm__ volatile ("" : "+r"(mask8));
        value = word5;
        value &= word4;
        word7 = 128;
        word7 <<= 7;
        __asm__ volatile ("" : "+r"(word7));
        reg1 = word7;
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[0] = (u16)value;

        word4 = *(volatile u32 *)(rec + 80);
        value = word4;
        value += 12;
        reg1 = 0x1ff;
        __asm__ volatile ("" : "+r"(reg1));
        mask = reg1;
        __asm__ volatile ("" : "+r"(mask));
        value &= mask;
        temp7 = 0xffffc000u;
        __asm__ volatile ("" : "+r"(reg1));
        __asm__ volatile ("" : "+r"(temp7));
        reg1 = temp7;
        __asm__ volatile ("" : "+r"(temp7));
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[1] = (u16)value;

        reg1 = *(volatile u16 *)(rec + 58);
        __asm__ volatile ("" : "+r"(reg1));
        value = reg1 << 12;
        word7 = *(u16 *)(rec + 60);
        __asm__ volatile ("" : "+r"(word7));
        value |= word7;
        cursor[2] = (u16)value;
        cursor = (volatile u16 *)((volatile u8 *)cursor + 8);

        value = *(s16 *)(rec + 16);
        if (value > 9) {
            value = word5 - 6;
            word7 = mask8;
            __asm__ volatile ("" : "+r"(word7));
            value &= word7;
            cursor[0] = (u16)value;

            value = word4 - 20;
            value &= mask;
            word4 = 0xffff8000u;
            __asm__ volatile ("" : "+r"(word4));
            reg1 = word4;
            __asm__ volatile ("" : "+r"(reg1));
            value |= reg1;
            cursor[1] = (u16)value;

            value = *(volatile u16 *)(rec + 56);
            value += 16;
            rec = (volatile u8 *)(uintptr_t)*(volatile u16 *)(rec + 58);
            __asm__ volatile ("" : "+r"(rec) : : "memory");
            reg1 = (u32)(uintptr_t)rec << 12;
            value |= reg1;
            cursor[2] = (u16)value;
            cursor = (volatile u16 *)((volatile u8 *)cursor + 8);
        }

        word7 = (u32)(uintptr_t)root;
        __asm__ volatile ("" : "+r"(word7));
        rec = *(volatile u8 **)(uintptr_t)word7;
        value = *(volatile u32 *)(rec + 84);
        value -= 6;
        reg1 = mask8;
        __asm__ volatile ("" : "+r"(reg1));
        value &= reg1;
        cursor[0] = (u16)value;
        value = *(volatile u32 *)(rec + 80);
        value &= mask;
        word4 = 0xffff8000u;
        __asm__ volatile ("" : "+r"(word4));
        reg1 = word4;
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[1] = (u16)value;
        word7 = *(u16 *)(rec + 58);
        value = word7 << 12;
        rec = (volatile u8 *)(uintptr_t)*(u16 *)(rec + 56);
        value |= (u32)(uintptr_t)rec;
    } else {
        value = *(volatile u32 *)(rec + 84);
        value -= 6;
        reg1 = 255;
        __asm__ volatile ("" : "+r"(reg1));
        value &= reg1;
        word4 = 128;
        word4 <<= 7;
        __asm__ volatile ("" : "+r"(word4));
        reg1 = word4;
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[0] = (u16)value;

        value = *(volatile u32 *)(rec + 80);
        reg1 = 0x1ff;
        __asm__ volatile ("" : "+r"(reg1));
        value &= reg1;
        temp7 = 0xffffc000u;
        __asm__ volatile ("" : "+r"(temp7));
        reg1 = temp7;
        __asm__ volatile ("" : "+r"(temp7));
        __asm__ volatile ("" : "+r"(reg1));
        value |= reg1;
        cursor[1] = (u16)value;

        reg1 = *(volatile u16 *)(rec + 58);
        __asm__ volatile ("" : "+r"(reg1));
        value = reg1 << 12;
        rec = (volatile u8 *)(uintptr_t)*(u16 *)(rec + 56);
        value |= (u32)(uintptr_t)rec;
    }

    cursor[2] = (u16)value;
    cursor = (volatile u16 *)((volatile u8 *)cursor + 8);
    return (void *)cursor;
}
void *_08008BB8(void *a) __attribute__((alias("Course_Dispatch_08BB8")));
void *sub_08008BB8(void *a) __attribute__((alias("Course_Dispatch_08BB8")));

void Course_Dispatch_08CA0(void *rec) { // _08008CA0: phase dispatcher — 9-entry table at 0x08008EB4 vs audit 21, blocked on indirect MovPc
    // lsls #2; ldr r0,[r1,r0]; mov pc,r0 — indirect table target not proven, leave blocked
    (void)rec;
    // Exact VMA 0x08008CA0, pool 0x08008EB4 u32 table 9×4, but dispatch index via s16 at +0x10FC not proven via ramwatch
}
// No alias for 08CA0 — indirect MovPc 21 vs 9 remains blocked, not guessed

// ROM entry alias.
void RomEmit8AAC(void *rec) __attribute__((alias("Course_Dispatch_08AAC")));
void *RomEmit8BB8(void *out) __attribute__((alias("Course_Dispatch_08BB8")));
