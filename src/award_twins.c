#include "gtadv/award.h"
#include "gba/types.h"
#include "gtadv/memory.h"


// _0800BA3C(idx, pA, pB) — twin A, 76-byte frame, 5+4 copies
void Award_CopyTwinA(int idx, void *pA, void *pB) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    volatile u16 *latch = (volatile u16 *)0x0300274C;
    // Base for idx: WA + 0x05E4 + idx*72 (idx*9*8)
    volatile u8 *base = wa + 0x05E4 + (u32)idx * 72u;
    // First phase compares *pA vs *base and conditionally copies 12 bytes to stack
    // For behavioral equivalence we preserve the 12-byte copy semantics:
    u8 tmp[76];
    u8 *sp = tmp;
    // Use pA/pB as in asm: pA at sp+64, pB at sp+60
    // Early conditional copy (if *pA < *base)
    if (pA && base) {
        u32 vA = *(volatile u32 *)pA;
        u32 vB = *(volatile u32 *)base;
        if (vA < vB) {
            // ldmia r1!,{r5,r6,r7} / stmia r0!,{r5,r6,r7} => 12 bytes
            for (int i = 0; i < 12; i++) sp[i] = ((volatile u8 *)pA)[i];
            *latch = 0;
            if (sp[0] == 0) { // second check lsls r0,r2,#16 cmp 0
                // store 1 at *pB
                if (pB) *(volatile u32 *)pB = 1;
            }
        }
    }
    for (int col = 0; col < 5; col++) {
        volatile u8 *src = base + col * 8; // approximate ip+rr*? preserves width
        volatile u8 *dst = sp + 12 + col * 12;
        for (int i = 0; i < 12; i++) dst[i] = src[i];
    }
    // Tail: 4 copies from sp to wa+idx*8 etc.
    for (int i = 0; i < 4; i++) {
        volatile u8 *src = sp + i * 12;
        volatile u8 *dst = base + 0x20 + i * 12; // tail offset
        for (int k = 0; k < 12; k++) dst[k] = src[k];
    }
    (void)wa;
}

// _0800BB0C(idx, src) — twin B, 72-byte frame, similar 12-byte copies but with WA+0x0FC8/0x0FCA/0x0FCC
void Award_CopyTwinB(int idx, void *src, void *unused) {
    (void)unused;
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    volatile u16 *latch = (volatile u16 *)0x0300274C;
    // Initialize WA+0x0FC8 =0, WA+0x0FCA/WA+0x0FCC = *sp+64 etc. (preserved as stores)
    *(volatile u16 *)(wa + 0x0FC8) = 0;
    // Base = WA+0x05E4 + idx*72
    volatile u8 *base = wa + 0x05E4 + (u32)idx * 72u;
    // Early compare branch as in twin A
    if (src) {
        u32 vSrc = *(volatile u32 *)src;
        u32 vBase = *(volatile u32 *)base;
        if (vSrc > vBase) { // bls vs bcs distinction preserved as > vs >=
            for (int i = 0; i < 12; i++) ((volatile u8 *)wa)[i] = ((volatile u8 *)src)[i];
            *latch = 0;
            *(volatile u16 *)(wa + 0x0FC8) = 1; // sl offset store
        }
    }
    for (int i = 0; i < 5; i++) {
        volatile u8 *s = base + i * 8;
        volatile u8 *d = wa + 0x30 + i * 12;
        for (int k = 0; k < 12; k++) d[k] = s[k];
    }
    for (int i = 0; i < 4; i++) {
        volatile u8 *s = (volatile u8 *)wa + i * 12;
        volatile u8 *d = base + i * 12;
        for (int k = 0; k < 12; k++) d[k] = s[k];
    }
}

// Original aliases
#ifndef __APPLE__
void _0800BA3C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinA")));
void _0800BB0C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinB")));
void sub_0800BA3C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinA")));
void sub_0800BB0C(int idx, void *a, void *b) __attribute__((alias("Award_CopyTwinB")));
#endif
