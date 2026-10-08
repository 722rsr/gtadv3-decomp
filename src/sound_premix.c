// ============================================================================
// sound_premix.c — C lift of the 4 remaining sound_premix.s leaves
// (VMA 0x0802B7D8 / 0x0802B7DC / 0x0802B7E0 / 0x0802B818).
//
// Transcribed instruction-for-instruction from asm/sound_premix.s.
// The two 0x…7D8/0x…7DC leaves are sound-effect dispatch sinks that the
// parent game code calls with effect-specific register payloads; in the ROM
// they are exactly `bx lr` (the effect tables were stubbed at build time).
// ============================================================================

#include "gba/types.h"

// ----------------------------------------------------------------------------
// sub_08002B7D8 — effect sink A: `bx lr` (no-op with 2-arg ABI).
void Sfx_08002B7D8(int a, int b) {
    (void)a; (void)b;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08002B7D8(int a, int b) __attribute__((alias("Sfx_08002B7D8")));
void sub_08002B7D8(int a, int b) __attribute__((alias("Sfx_08002B7D8")));
#endif

// ----------------------------------------------------------------------------
// sub_08002B7DC — effect sink B: `bx lr` (no-op with 2-arg ABI).
void Sfx_08002B7DC(int a, int b) {
    (void)a; (void)b;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08002B7DC(int a, int b) __attribute__((alias("Sfx_08002B7DC")));
void sub_08002B7DC(int a, int b) __attribute__((alias("Sfx_08002B7DC")));
#endif

#ifndef __APPLE__
__attribute__((naked)) void Sfx_08002B7E0(int id, int a1, int a2, int a3) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "adds r4, r2, #0\n"
        "adds r2, r3, #0\n"
        "cmp r0, #5\n"
        "beq 1f\n"
        "cmp r0, #6\n"
        "beq 2f\n"
        "b 3f\n"
        "1:\n"
        "adds r0, r2, #0\n"
        "bl sub_08002B7D8\n"
        "b 3f\n"
        "2:\n"
        "lsls r0, r1, #16\n"
        "lsrs r0, r0, #16\n"
        "lsls r1, r4, #16\n"
        "lsrs r1, r1, #16\n"
        "bl sub_08002B7DC\n"
        "3:\n"
        "pop {r4}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
}
#else
void Sfx_08002B7E0(int id, int a1, int a2, int a3) {
    switch ((u32)id) {
    case 5:
        Sfx_08002B7D8(a3, 0);
        break;
    case 6:
        Sfx_08002B7DC((int)(u16)a1, (int)(u16)a2);
        break;
    }
}
#endif
#ifndef __APPLE__
void _08002B7E0(int a, int b, int c, int d) __attribute__((alias("Sfx_08002B7E0")));
void sub_08002B7E0(int a, int b, int c, int d) __attribute__((alias("Sfx_08002B7E0")));
#endif

extern void _08005D74(void *p, int y, int angle);
void Sfx_08002B818(void *obj, int arg1) {
    register volatile u32 *st __asm__("r4");  // ROM holds the state block in r4
    register u32 v __asm__("r2");             // and the u16 mask in r2
    register u32 n __asm__("r0");             // r0 is the working value
    register u32 t __asm__("r1");             // r1 is the 1600 / sign scratch
    volatile u32 out[2];
    v = (u32)(arg1 << 16) >> 16;      // ROM: lsls r1,#16; lsrs r2,r1,#16
    st = (volatile u32 *)(uintptr_t)*(volatile u32 *)((volatile u8 *)obj + 8);
    if (v & 1) {
        n = (s32)(u32)(st[4] + 16);
        st[4] = (u32)n;
        t = (u32)(200 << 3);
        if ((s32)n > (s32)t)  // ROM: cmp r0,r1; ble — a SIGNED compare
            st[4] = t;
    } else {
        n = st[4];
        t = n >> 31;                 // ROM: lsrs r1, r0, #31
        n = n + t;                   // ROM: adds r0, r0, r1
        n = (u32)((s32)n >> 1);      // ROM: asrs r0, r0, #1 (signed)
        st[4] = n;
    }
    if (v & 32)
        st[2] = st[2] - 32;
    if (v & 16)
        st[2] = st[2] + 32;
    t = (u32)(s32)st[4];
    _08005D74((void *)out, (int)(s32)t,
              (int)*(s16 *)(st + 2));   // Non-volatile signed load preserves ldrsh.
    st[0] = st[0] + out[0];
    st[1] = st[1] + out[1];
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08002B818(void *a, int b) __attribute__((alias("Sfx_08002B818")));
void sub_08002B818(void *a, int b) __attribute__((alias("Sfx_08002B818")));
#endif
