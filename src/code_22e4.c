// ============================================================================
// code_22e4.c — C lift of asm/code_22e4.s remaining functions:
// 0x08002388 / 0x080023AC / 0x080023B8 / 0x080023C4.
//
// IRQ enable + dispatcher reinstall + full IRQ/timer/sound reset.
// Transcribed instruction-for-instruction from the cited asm listing.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void sub_08000A60(void) { }
__attribute__((weak)) void sub_08002A3C(int a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void sub_08002A80(void) { }
__attribute__((weak)) void _08002AAC(void) { }
#else
extern void sub_08000A60(void);    // 0x08000A60 soft-IRQ kicker (src/foundation_softirq.c)
extern void sub_08002A3C(int slot, void *handler); // 0x08002A3C IRQ install (foundation_boot.c)
extern void sub_08002A80(void);    // 0x08002A80 IRQ table reinstall
extern void _08002AAC(void);    // 0x08002AAC IRQ/DISPSTAT config (foundation_runtime.c)
#endif

// ----------------------------------------------------------------------------
// 0x08002388 sub_08002388 — enable IRQs + arm master enable flag:
//   sub_08000A60
//   IME(0x04000208) = 0
//   [0x03007FF8] |= 1
//   IME = 1
// ----------------------------------------------------------------------------
void sub_08002388(void)
{
    sub_08000A60();
    *(volatile u16 *)(uintptr_t)0x04000208u = 0;
    *(volatile u16 *)(uintptr_t)0x03007FF8u |= 1;
    *(volatile u16 *)(uintptr_t)0x04000208u = 1;
}

// ----------------------------------------------------------------------------
// 0x080023AC sub_080023AC(slot, handler) — IRQ dispatcher install thunk
// ----------------------------------------------------------------------------
void _080023AC(int slot, void *handler)
{
    sub_08002A3C(slot, handler);
}

// ----------------------------------------------------------------------------
// 0x080023B8 sub_080023B8 — IRQ table reinstall thunk
// ----------------------------------------------------------------------------
void _080023B8(void)
{
    sub_08002A80();
}
#ifndef __APPLE__
// asm/code_22e4.s branches to the `sub_` spelling; the body is defined under
// the `_` spelling, so the splice needs the alias to re-export it (rule 6).
void _08002388(void) __attribute__((alias("sub_08002388")));
void sub_080023AC(int slot, void *handler) __attribute__((alias("_080023AC")));
void sub_080023B8(void) __attribute__((alias("_080023B8")));
#endif

// ----------------------------------------------------------------------------
// 0x080023C4 sub_080023C4 — full IRQ/timer/sound reset.
// Ground truth (baserom.gba @ 0x080023C4), decoded instruction for instruction:
//   push {r4, lr}
//   ldr r0,=0x04000208; movs r1,#0; strh r1,[r0]      IME  = 0
//   ldr r0,=0x04000004; strh r1,[r0]                   TM0  = 0
//   ldr r0,=0x04000200; strh r1,[r0]                   DISPSTAT = 0
//   ldr r1,=0x04000202; ldr r2,=0xFFFF; strh r0,[r1]   DISPSTAT_EN = 0xFFFF
//   for slot in 0..9: _080023AC(slot, 0)
//   _080023AC(0, 0x0203EE71)     @ ISR blob A slot
//   _080023AC(1, 0x08002389)     @ this fn's slot-1 continuation
//   _080023B8
void _080023C4(void)
{
    int slot;
    volatile u16 *ime = (volatile u16 *)(uintptr_t)0x04000208u;
    *ime = 0;
    volatile u16 *tm0 = (volatile u16 *)(uintptr_t)0x04000004u;
    *tm0 = 0;
    volatile u16 *dstat = (volatile u16 *)(uintptr_t)0x04000200u;
    *dstat = 0;
    volatile u16 *dstat_en = (volatile u16 *)(uintptr_t)0x04000202u;
    *dstat_en = 0xFFFF;

    for (slot = 0; slot <= 9; slot++)
        _080023AC(slot, (void *)0);
    _080023AC(0, (void *)(uintptr_t)0x0203EE71u);
    _080023AC(1, (void *)(uintptr_t)0x08002389u);
    _080023B8();
}

// 0x08002424 sub_08002424 — 12 B thin wrapper: `push {lr} / bl 0x08002AAC /
// pop {r0} / bx r0` + 2 pad. Labelled in asm/code_22e4.s, which made it a known
// start, so the probe requires a C candidate. A leaf that makes exactly one
// call is the shape agbcc frames, so it should be byte-exact.
// 0x08002AAC, defined in foundation_agbmain.c. The closure spells it
// `sub_08002AAC`, so call that on the ROM build; the friendly name is the
// host stub. Same split as FF_CALLEE / MS_CALLEE / R35_CALLEE elsewhere.
extern void sub_08002AAC(void);
#ifdef __APPLE__
extern void RuntimeIrqConfig(void);
#define CAL_2AAC RuntimeIrqConfig
#else
#define CAL_2AAC sub_08002AAC
#endif
void Code2424_IrqConfigReset(void)
{
    CAL_2AAC();
}
#ifndef __APPLE__
void _08002424(void) __attribute__((alias("Code2424_IrqConfigReset")));
void sub_08002424(void) __attribute__((alias("Code2424_IrqConfigReset")));
#endif
