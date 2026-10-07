// ============================================================================
// runtime_accessors.c — final 2-gap stragglers across four asm clusters:
//
//   asm/idle.s               0x08001FD0 / 0x08001FE0  (mode-range tests)
//   asm/sound_d974.s         0x0802D98C / 0x0802D994  (SWI 0x25 + reset leaf)
//   asm/code_fa0.s           0x08001370 / 0x080015A0  (serial RC / wait leaf)
//   asm/course_records_7bfc.s 0x08008164 / 0x0800821C (course-id / timed reset)
//
// Transcribed instruction-for-instruction from the cited asm listings.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void sub_08000F48(void *p) { (void)p; }
__attribute__((weak)) void _080023AC(int slot, void *h) { (void)slot; (void)h; }
__attribute__((weak)) void _08007ABC(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void _08007538(void *a, int b, void *c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void _08007614(void *a, int b, int c, int d) { (void)a; (void)b; (void)c; (void)d; }
__attribute__((weak)) int _08002D978(int a, int b) { (void)a; return 0; }
__attribute__((weak)) int _08002D97C(int a, int b) { (void)a; return 0; }
__attribute__((weak)) int _08002DE04(int a, int b) { (void)a; return 0; }
#endif

extern void sub_08000F48(void *p);              // 0x08000F48 (raw asm route)
extern void _080023AC(int slot, void *h);       // 0x080023AC (src/code_22e4.c)
extern void _08007ABC(void *a, int b, int c);   // EventBind
extern void _08007538(void *a, int b, void *c); // 0x08007538 lane emit (course_resource.c)
extern void _08007614(void *a, int b, int c, int d); // 0x08007614 lane emit (course_resource.c)
extern int _08002D978(int a, int b);            // SWI Div quotient (bios_wrappers.c)
extern int _08002D97C(int a, int b);            // SWI DivRem remainder (bios_wrappers.c)
extern int _08002DE04(int a, int b);            // signed __aeabi_idiv quotient (sound_extra.c)

// ============================================================================
// idle.s — mode-range predicates
// ============================================================================

// ----------------------------------------------------------------------------
// 0x08001FD0 _08001FD0(m) -> bool — true unless m in {6..8} ("race modes").
// ----------------------------------------------------------------------------
int _08001FD0(int m)
{
    int r1 = 1;
    switch (m) {
    case 6:
    case 7:
    case 8:
        r1 = 0;
        break;
    }
    return r1;
}

// ----------------------------------------------------------------------------
// 0x08001FF0 _08001FF0 -> u32 — two idle mode predicates OR'd.
//   ROM: r5 = 0x030000E4; a = _08001FD0(*(u16*)(*r5 + 16));
//        b = _08001FD0(*(u16*)(*r5 + 18));  (the slot is RE-READ for b,
//        `ldr r0,[r5]` appears twice); return a | b.
// ----------------------------------------------------------------------------
u32 _08001FF0(void)
{
    volatile u32 *slot = (volatile u32 *)(uintptr_t)0x030000E4u;
    u32 a = (u32)_08001FD0((int)*(volatile u16 *)((const volatile u8 *)(uintptr_t)*slot + 16));
    u32 b = (u32)_08001FD0((int)*(volatile u16 *)((const volatile u8 *)(uintptr_t)*slot + 18));
    return a | b;
}

// ----------------------------------------------------------------------------
// 0x08001FE0 _08001FE0(m) -> bool — true unless m in {10, 11}.
// ----------------------------------------------------------------------------
int _08001FE0(int m)
{
    int r1 = 1;
    switch (m) {
    case 10:
    case 11:
        r1 = 0;
        break;
    }
    return r1;
}

// ============================================================================
// sound_d974.s — SWI leaves (sound BIOS/reset tail)
// ============================================================================

// ----------------------------------------------------------------------------
// 0x0802D98C sub_0802D98C — swi 0x25 (SoundBias: r1 = 1) with r0 preserved.
// ----------------------------------------------------------------------------
__attribute__((naked)) void _0802D98C(void)
{
    __asm__ volatile (
        "movs r1, #1\n"
        "swi 0x25\n"
        "bx lr\n"
        ".short 0\n"
    );
}

// ----------------------------------------------------------------------------
// 0x0802D994 sub_0802D994 — full reset: IME=0, SP=0x03007F00, RegisterRamReset
// (swi 1) then SoftReset (swi 0).
// ----------------------------------------------------------------------------
__attribute__((naked)) void _0802D994(void)
{
    __asm__ volatile (
        "ldr r3, 1f\n"
        "movs r2, #0\n"
        "strb r2, [r3]\n"
        "ldr r1, 2f\n"
        "mov sp, r1\n"
        "swi 1\n"
        "swi 0\n"
        ".short 0\n"
        "1: .word 0x04000208\n"
        "2: .word 0x03007F00\n"
    );
}
#ifndef __APPLE__
void sub_0802D98C(void) __attribute__((alias("_0802D98C")));
void sub_0802D994(void) __attribute__((alias("_0802D994")));
#endif

// ============================================================================
// code_fa0.s — serial RC gate + wait leaf
// ============================================================================

// ----------------------------------------------------------------------------
// 0x08001370 _08001370(ctx, v_u16) — SIO RC register write gate:
//   r2=ctx, r1=(u16)v
//   RCNT(0x04000128): r4 = u16[RCNT] & 140
//   if r4 == 8:                       @ GPIO mode 8 = SIO on RC
//     RCNT+2 (0x0400012A) = r1
//     RCNT = 0x2083
//     ctx[72] = 1
//     return 0
//   else:
//     _08000F48(ctx)
//     r4 ^= 8
//     return r4
// ----------------------------------------------------------------------------
int _08001370(void *ctx, u16 v)
{
    volatile u16 *rcnt = (volatile u16 *)(uintptr_t)0x04000128u;
    u16 r4 = (u16)(*rcnt & 140);

    if (r4 != 8) {
        sub_08000F48(ctx);
        r4 = (u16)(r4 ^ 8);
        return r4;
    }

    *(volatile u16 *)(uintptr_t)0x0400012Au = v;
    *rcnt = 0x2083;
    *(volatile u8 *)((uintptr_t)ctx + 72) = 1;
    return 0;
}

__asm__(".align 2, 0");

// Rule 6: asm/code_fa0.s:515-517 defines BOTH `sub_08001370:` and
// `_08001370:` on this span, and :334/:362/:488/:758 still `bl sub_08001370`.
// The splice can only export a spelling agbcc wrote, so C must define both.
#ifndef __APPLE__
int sub_08001370(void *ctx, u16 v) __attribute__((alias("_08001370")));
#endif

// ----------------------------------------------------------------------------
// 0x080015A0 _080015A0(delay) — mode-clocked wait:
//   r2 = (u8)((pc >> 24) & 0xFF)   @ 0x08 on GBA ROM (IWRAM = 0x02)
//   r1 = (r2 == 2) ? 12 : (r2 == 8) ? 13 : 4
//   do { delay -= r1 } while (delay > 0)
// ----------------------------------------------------------------------------
__attribute__((naked)) void _080015A0(int delay)
{
    __asm__ volatile (
        ".syntax unified\n"
        "mov r2, pc\n"
        "lsrs r2, r2, #24\n"
        "movs r1, #12\n"
        "cmp r2, #2\n"
        "beq 1f\n"
        "movs r1, #13\n"
        "cmp r2, #8\n"
        "beq 1f\n"
        "movs r1, #4\n"
        "1:\n"
        "subs r0, r0, r1\n"
        "bgt 1b\n"
        "bx lr\n"
        ".syntax divided\n"
    );
}
#ifndef __APPLE__
void sub_080015A0(int delay) __attribute__((alias("_080015A0")));
#endif

// ============================================================================
// course_records_7bfc.s — course-id handler + timed reset
// (course-state block behind the global pointer slot 0x030003E0)
// ============================================================================

void _08008164(int id)
{
    volatile u8 *base = *(volatile u8 *volatile *)(uintptr_t)0x030003E0u;

    *(volatile u16 *)(base + 16) = (u16)id;
    *(volatile u16 *)(base + 28) = 0;
    if (id > 1) {
        if (id > 9) {
            int q = _08002D978(id, 10);
            u16 tq = *(volatile u16 *)(uintptr_t)(0x080CB17Cu + (u32)q * 2u);
            _08007538((void *)(uintptr_t)0x0828A35Cu, (int)tq,
                      (void *)(uintptr_t)(u32)(*(volatile u16 *)(base + 56) + 16u));
        }
        {
            int r = _08002D97C(id, 10);
            u16 tr = *(volatile u16 *)(uintptr_t)(0x080CB17Cu + (u32)r * 2u);
            _08007538((void *)(uintptr_t)0x0828A35Cu, (int)tr,
                      (void *)(uintptr_t)(u32)*(volatile u16 *)(base + 56));
        }
    } else {
        _08007538((void *)(uintptr_t)0x0828C420u, 11,
                  (void *)(uintptr_t)(u32)*(volatile u16 *)(base + 56));
    }
    _08007614((void *)(uintptr_t)0x0828A35Cu, 1, 0,
              (int)*(volatile u16 *)(base + 58));
    *(volatile u8 *)(base + 79) = 1;
}

void _0800821C(void)
{
    volatile u8 *base = *(volatile u8 *volatile *)(uintptr_t)0x030003E0u;

    *(volatile u16 *)(base + 22) = 16;
    *(volatile u16 *)(base + 24) = 3;
    *(volatile u8 *)(base + 79) = 1;
    {
        int q = _08002DE04((int)*(volatile s16 *)(base + 16), 5);
        *(volatile s16 *)(base + 30) = (s16)q;
        if ((s16)q > 10)
            *(volatile s16 *)(base + 30) = 10;
    }
    {
        s16 slot = *(volatile s16 *)(base + 30);
        s16 tv = *(volatile s16 *)(uintptr_t)(0x080CB074u + (u32)((s32)slot * 8) + 4u);
        if (tv == -1)
            return;
        _08007ABC((void *)(uintptr_t)*(volatile u32 *)(base + 100),
                  (int)tv, (int)*(volatile u16 *)(base + 62));
        *(volatile u16 *)(base + 28) = 60;
        *(volatile u8 *)(base + 79) = 1;
    }
}

// ----------------------------------------------------------------------------
// 0x08000F48 — 0x66B record-header reset ((replaced: asm/code_fa0.s)): zeroes u8[rec+24],
// u8[rec+29], u8[rec+30], u8[rec+72], s16[rec+22]; stores 15 at u8[rec+74]
// (a countdown seed — Helper_013BC's caller `_080014A4` decrements it while
// >15 and rolls it back otherwise); then three halfword constants through
// literal pools:
//   u16[0x04000134] = 0        (pool 0x08000F74)
//   u16[0x04000128] = 0x0003   (pool 0x08000F7C, via r3=0x04000000 + 0x128)
//   u16[0x0400012A] = 0        (pool 0x08000F80)
// ROM: `strh r1,[r0,#0]` with r0 = pool 0x08000F78 word = 0x04000134;
// `strh r0,[r2,#0]` with r2 = 0x04000134 pool... decoded from the slice dump:
// pools are (0x04000134, 0x04000128, 0x0003, 0x0400012A).
void RecReset_0F48(void *rec) {
    volatile u8 *b = (volatile u8 *)rec;
    b[30] = 0; b[24] = 0; b[29] = 0; b[72] = 0;
    *(volatile s16 *)(b + 22) = 0;
    b[74] = 15;
    *(volatile u16 *)0x04000134u = 0;
    *(volatile u16 *)0x04000128u = 0x0003;
    *(volatile u16 *)0x0400012Au = 0;
}
#ifndef __APPLE__
void _08000F48(void *p) __attribute__((alias("RecReset_0F48")));
void sub_08000F48(void *p) __attribute__((alias("RecReset_0F48")));
#endif
