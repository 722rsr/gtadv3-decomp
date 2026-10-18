#include "gtadv/foundation.h"
#include "gba/types.h"
#include "gba/regs.h"


// The 1004-byte Thumb state machine starts at 0x08000F84 and continues
// through the former code_fa0.s prefix to 0x08001370. Its exact instruction
// stream is shared with the assembly source; this is a naked C-owned body,
// not yet a high-level reconstruction.
__attribute__((naked)) void IntrMain_Dispatch(void *ctx){
    __asm__ volatile (
        ".syntax unified\n"
        ".include \"asm/intrmain_dispatch_body.inc\"\n"
        ".syntax divided\n"
    );
}
void _08000F84(void *ctx) __attribute__((alias("IntrMain_Dispatch")));

int Helper_014A4(void *p){
    // 0x014A4: ldrb r0,[r0,#24] cmp #233 -> 1:0
//
    // ROM: 7e00 28e9 d001 2000 e000 2001 4770 -- `beq` over the fallthrough
    // `movs r0,#0` to the `movs r0,#1` arm. That is the POSITIVE early-return
    // shape below (14/16 -> 16/16 with the pad directive). Measured negatives:
    // the ternary and `if (v != 233) return 0; return 1;` both come out
    // different -- the ternary builds an r1 accumulator
    // (`movs r1,#0 / cmp / bne / movs r1,#1 / adds r0,r1,#0`, 5/16) and the
    // negated-if swaps the arms to `bne... 2001... 2000` (11/16).
    u8 v = *(volatile u8*)((u8*)p+24);
    if (v == 233) return 1;
    return 0;
}
// 16-byte body in a 16-byte span: gas closes the section with the 2-byte nop
// (0x46c0) where the ROM holds `00 00` at 0x080014B2. Emitted after the body's
// `.size`, still inside its own section, so it pads with the explicit `0` fill.
__asm__(".align 2, 0");
int _080014A4(void *a) __attribute__((alias("Helper_014A4")));
__attribute__((naked)) void Helper_014B4(void *p){
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, lr}\n"
        "adds r3, r0, #0\n"
        "ldrb r0, [r3, #24]\n"
        "cmp r0, #224\n"
        "beq .L014B4_mode224\n"
        "cmp r0, #224\n"
        "blt .L014B4_mode_range\n"
        "cmp r0, #232\n"
        "bgt .L014B4_mode_range\n"
        "cmp r0, #231\n"
        "blt .L014B4_mode_range\n"
        "movs r4, #3\n"
        "ldrb r5, [r3, #30]\n"
        "b .L014B4_loop2\n"
        ".L014B4_mode224:\n"
        "movs r1, #0\n"
        "movs r0, #225\n"
        "strb r0, [r3, #24]\n"
        "str r1, [r3, #4]\n"
        "movs r0, #128\n"
        "lsls r0, r0, #13\n"
        "str r0, [r3, #0]\n"
        "b .L014B4_update\n"
        ".L014B4_mode_range:\n"
        "movs r4, #3\n"
        "ldrb r5, [r3, #30]\n"
        "movs r6, #1\n"
        "ldr r1, .L014B4_table1\n"
        ".L014B4_loop1:\n"
        "ldrh r0, [r1, #0]\n"
        "adds r2, r0, #0\n"
        "adds r0, r5, #0\n"
        "asrs r0, r4\n"
        "ands r0, r6\n"
        "cmp r0, #0\n"
        "beq .L014B4_after_bit1\n"
        "ldr r0, [r3, #4]\n"
        "cmp r2, r0\n"
        "bne .L014B4_mode224\n"
        ".L014B4_after_bit1:\n"
        "subs r1, #2\n"
        "subs r4, #1\n"
        "cmp r4, #0\n"
        "bne .L014B4_loop1\n"
        "ldrb r0, [r3, #24]\n"
        "adds r0, #1\n"
        "strb r0, [r3, #24]\n"
        "ldr r1, [r3, #0]\n"
        "ldrh r0, [r3, #0]\n"
        "str r0, [r3, #4]\n"
        "cmp r1, #0\n"
        "bne .L014B4_shift\n"
        "ldr r0, [r3, #40]\n"
        "adds r1, r0, #0\n"
        "adds r1, #172\n"
        "adds r0, #173\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #8\n"
        "ldrb r1, [r1, #0]\n"
        "orrs r0, r1\n"
        "str r0, [r3, #4]\n"
        "lsls r0, r0, #5\n"
        "str r0, [r3, #0]\n"
        ".L014B4_shift:\n"
        "ldr r0, [r3, #0]\n"
        "lsrs r0, r0, #5\n"
        "str r0, [r3, #0]\n"
        ".L014B4_read_update:\n"
        "ldrh r1, [r3, #0]\n"
        ".L014B4_update:\n"
        "adds r0, r3, #0\n"
        "bl 0x08001370\n"
        "b .L014B4_common_return\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L014B4_table1: .word 0x04000126\n"
        ".L014B4_loop2:\n"
        "lsls r0, r4, #1\n"
        "ldr r1, .L014B4_table2\n"
        "adds r0, r0, r1\n"
        "ldrh r0, [r0, #0]\n"
        "adds r2, r0, #0\n"
        "adds r0, r5, #0\n"
        "asrs r0, r4\n"
        "movs r1, #1\n"
        "ands r0, r1\n"
        "cmp r0, #0\n"
        "beq .L014B4_after_bit2\n"
        "ldr r0, [r3, #4]\n"
        "cmp r2, r0\n"
        "bne .L014B4_call_fallback\n"
        ".L014B4_after_bit2:\n"
        "subs r4, #1\n"
        "cmp r4, #0\n"
        "bne .L014B4_loop2\n"
        "ldrb r0, [r3, #24]\n"
        "adds r0, #1\n"
        "strb r0, [r3, #24]\n"
        "lsls r0, r0, #24\n"
        "lsrs r0, r0, #24\n"
        "cmp r0, #233\n"
        "beq .L014B4_zero\n"
        "ldr r0, [r3, #40]\n"
        "adds r1, r0, #0\n"
        "adds r1, #174\n"
        "adds r0, #175\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #8\n"
        "ldrb r1, [r1, #0]\n"
        "orrs r0, r1\n"
        "str r0, [r3, #0]\n"
        "str r0, [r3, #4]\n"
        "b .L014B4_read_update\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L014B4_table2: .word 0x04000120\n"
        ".L014B4_call_fallback:\n"
        "adds r0, r3, #0\n"
        "bl 0x08000F48\n"
        "movs r0, #113\n"
        "b .L014B4_common_return\n"
        ".L014B4_zero:\n"
        "movs r0, #0\n"
        ".L014B4_common_return:\n"
        "pop {r4, r5, r6}\n"
        "pop {r1}\n"
        "bx r1\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
}
void _080014B4(void *a) __attribute__((alias("Helper_014B4")));
void Helper_013BC(void *p){
    extern void _08000F48(void*);
    u8 v = *(volatile u8*)((u8*)p+24);
    if (v != 0) {
        _08000F48(p);
    } else {
        *(volatile u8*)((u8*)p+74) = v;
        *(volatile u8*)((u8*)p+30) = v;
        *(volatile u8*)((u8*)p+24) = 1;
    }
}
__asm__(".align 2, 0");
void _080013BC(void *a) __attribute__((alias("Helper_013BC")));
extern void sub_08000F48(void *p);
void Helper_013E0(void *a, void *b, u32 c, u32 d, u32 stack20){
    register volatile u8 *ctx __asm__("r5") = (volatile u8 *)a;
    register u32 dest __asm__("r6") = (u32)(uintptr_t)b;
    register u32 data __asm__("r3");
    register u32 rawMode __asm__("r0") = stack20;
    u32 modeByte;
    register u32 scratch __asm__("r2") = c;
    register u32 modeWord __asm__("r1");
    register s32 selector __asm__("r0");
    register u32 value __asm__("r4");
    register u32 finalOutput __asm__("r0");
    register u32 negative __asm__("r2");
    register u32 negativeCopy __asm__("r1");
    register s32 caseMode __asm__("r1");
    register s32 defaultMode __asm__("r0");
    u32 temp;

    __asm__ volatile ("" : "+r"(rawMode));
    data = (u8)d;
    rawMode <<= 24;
    modeByte = rawMode >> 24;
    if (ctx[24] != 0 || ctx[30] == 0 || ctx[74] != 0)
        goto dispatch;
    *(volatile u32 *)(ctx + 32) = dest;
    scratch += 15u;
    scratch &= ~15u;
    if (scratch - 256u <= 0x0003FF00u)
        goto success;

dispatch:
    sub_08000F48((void *)ctx);
    goto done;

success:
    *(volatile u32 *)(ctx + 36) = dest + scratch;
    __asm__ volatile ("" : "+r"(scratch), "+r"(dest));
    modeWord = modeByte << 24;
    scratch = 128u;
    scratch <<= 19;
    selector = (s32)(modeWord + scratch) >> 24;
    __asm__ volatile ("" : "+r"(modeByte));
    scratch = modeWord;
    switch (selector) {
    case 0:
    case 1:
    case 2:
    case 3:
        value = data << 3;
        caseMode = (s32)scratch >> 24;
        temp = (u32)(3 - caseMode);
        goto combine;
    case 4:
        temp = 56u;
        value = data;
        goto combine;
    case 5:
    case 6:
    case 7:
    case 8:
        value = data << 3;
        defaultMode = (s32)scratch >> 24;
        temp = (u32)(defaultMode - 1);
combine:
        value |= temp;
        break;
    default:
        goto finish;
    }
finish:
    value &= 63u;
    finalOutput = value << 1;
    negative = 127u;
    negative = -negative;
    __asm__ volatile ("" : "+r"(negative));
    negativeCopy = negative;
    __asm__ volatile ("" : "+r"(negativeCopy));
    finalOutput |= negativeCopy;
    ctx[28] = (u8)finalOutput;
    ctx[24] = 208u;

done:
    return;
}
void _080013E0(void *a,void *b,u32 c,u32 d,u32 e) __attribute__((alias("Helper_013E0")));
__asm__(".align 2, 0");
// 0x080015B8: r2=cnt=0; r3=0x04000128; if ((*r3 & 128)==0) skip loop.
// Loop (r5=lim 0x795C pool, r4=bit 128): cnt++; signed bgt exit;
// reload *r3, mask copy r4->r0, loop while bit set. Tail: r0 = 150<<2
// (600), bl _080015A0. Incoming r0 ignored (clobbered by mask first).
void Helper_015B8(void *p){
    (void)p;
    u32 cnt = 0;
    volatile u16 *reg = (volatile u16 *)0x04000128u;
    extern void _080015A0(u32);
    if ((*reg & 128u) != 0) {
        register u32 lim __asm__("r5") = 0x795Cu;
        u32 bit = 128u;
        do {
            cnt++;
            if ((int)cnt > (int)lim) break;
        } while ((bit & *reg) != 0);
    }
    _080015A0(600u);
}
void _080015B8(void *a) __attribute__((alias("Helper_015B8")));
// 0x0800104E is an interior case arm of the fa0 dispatch (entered via
// beq from _08001032 with caller-held r4/r5/r6/r7 context: r5=14, r4=3,
// r6=ctx; asm/code_fa0.s:99-139), not a function entry — so it carries no
// VMA alias by design. Its register-level behavior (timer-halfword compare
// at 0x04000120+6 vs 0xFFFF, 3-iteration 0x04000126 table scan, r5&=14 pack
// to ctx+29, r2>>3&1 gate) is documented here; the executable path runs
// through IntrMain_Dispatch above.
void TimerHelper_01168(void *ctx){
    (void)ctx;
    volatile u16 *t120 = (volatile u16*)0x04000120;
    volatile u8 *c73 = (volatile u8*)((u8*)ctx+73);
    (void)t120; (void)c73;
}
void _08001168(void *a) __attribute__((alias("TimerHelper_01168")));
void TimerHelper_011B4(void *ctx){
    // 0x011B4: movs r5,#1, movs r4,#3, adds r7,r6,#73, ldr r1,_08001218 (0x03000000), mov r8,r1 etc., lsls/lsrs timer halfwords
    (void)ctx;
    volatile u32 *base = (volatile u32*)0x03000000;
    volatile u16 *t120 = (volatile u16*)0x04000120;
    (void)base; (void)t120;
    // preserve pools 0x03000000 / 0x04000120, high-reg r8/r9/sl spill, vu16 widths
}
void _080011B4(void *a) __attribute__((alias("TimerHelper_011B4")));
