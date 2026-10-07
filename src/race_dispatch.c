#include "gtadv/ghost.h"
#include "gtadv/race_dispatch.h"
#include "gba/types.h"
#include "gtadv/memory.h"
#include "gba/bios.h"

/*
 * Race UI packet dispatcher, asm/race_dispatch.s — file offsets
 * 0x00B190-0x00B4A8, ROM VMA 0x0800B190-0x0800B4A8.
 *
 * Structure of the ROM function:
 *
 *   0x0800B190  CpuSet(src = sp+56 (a zero word), dst = sp, 0x0500000E)
 *               -> 32-bit fill of 14 words = zero the packet's 56 bytes.
 *   0x0800B1A2  8-way indirect jump on s16[WA+0x0FBC] (the race phase),
 *               table _0800B1CC.  Value > 7 (unsigned) -> the common tail.
 *   0x0800B2F8  common tail: fills pkt+21/+2/+4, +8/+10 (two sources by
 *               WA+0x0FBC), +12 (two sources by the selector), then the
 *               record block (+24/+26/+28..39) for selectors 6/7 and the
 *               0x0574 default, or the two-catalog-field +24/+26/+29 arm for
 *               selector 2, then pkt+22/+20 and the shared consumer.
 *   0x0800B478  _080188B0(pkt) — the shared UI/race packet consumer.
 *
 * The selector is a *permutation* of the phase, not the phase itself:
 *    phase 0 -> sel 2 (or sel 1 + a 0x08025790 field when WA+0x103A > 2)
 *    phase 1 -> sel 3
 *    phase 2 -> sel 5
 *    phase 3 -> sel 8 + garage-record block (pkt+40/+42/pkt[44..55])
 *    phase 4 -> sel 0 (tail only)
 *    phase 5 -> sel 4 + pkt+6 = 0x0802591C(s16[WA+0x0576])
 *    phase 6 -> sel 0
 *    phase 7 -> sel 6/7 by s16[WA+0x1078] (+ pkt+6 via 0x0802591C when the
 *               value is 1)
 *
 * Signedness is load-for-load: WA+0x103A, 0x0574, 0x0576, 0x0FBC, 0x1078 are
 * `ldrsh` (signed), WA+0x0FEE/0x0FE4/0x0FE6/0x0FC2/0x0FC4/0x0FC6/0x0FF2/
 * 0x0FF6/0x05E0 are `ldrh` (unsigned), WA+0x05E4+72*idx is a 32-bit `ldr`.
 */

/* Work-area image base. */
#define RD_WA ((u8 *)(uintptr_t)WORK_AREA_BASE)

#define WA_U16(o) (*(volatile u16 *)(RD_WA + (o)))
#define WA_S16(o) (*(volatile s16 *)(RD_WA + (o)))
#define WA_U32(o) (*(volatile u32 *)(RD_WA + (o)))

#define PK_U16(o) (*(volatile u16 *)(pkt + (o)))
#define PK_U32(o) (*(volatile u32 *)(pkt + (o)))

/* Car-catalog field readers over the 8-byte stride table at 0x0805FCAC
 * (index = 33*a + 3*b, the third register is dead in every body). */
extern int _080256F4(int a, int b, int c); // 0x080256F4 catalog halfword +4
extern int _08025710(int a, int b, int c); // 0x08025710 catalog halfword +2
extern int _08025750(int a, int b, int c); // 0x08025750 catalog dword +12/+16
extern int _08025790(int a, int b, int c); // 0x08025790 catalog halfword +6/+8
extern int _080257D4(int a, int b, int c); // 0x080257D4 catalog halfword +20
extern int _080257F0(int a, int b, int c); // 0x080257F0 catalog halfword +22
extern int _0802591C(int idx);             // 0x0802591C course event field +10
extern void _080188B0(void *pkt);          // 0x080188B0 shared packet consumer

/* Header contract for the shared consumer (kept for the other TUs). */
__attribute__((weak)) void UiPacket_Consume(void *pkt) { (void)pkt; }

/* The ROM keeps the selector at sp+0 — i.e. the packet's first halfword — and
 * re-reads it from there in the tail (0x0800B376 / 0x0800B3D4 / 0x0800B3F8).
 * Every case stores it with `strh r0,[r1,#0]`, so the packet is the single
 * source of truth and the local below only mirrors it for legibility. */
#define SET_SEL(v) do { sel = (u16)(v); PK_U16(0) = sel; } while (0)

void Race_Dispatch(void) {
    u8 pkt[60] __attribute__((aligned(4)));
    u16 sel;
    int rec_off;
    int i;

    /* 0x0800B190 — CpuSet(src = sp+56 (a zero word), dst = sp,
     * 0x0500000E): fill mode, 32-bit units, 14 words = 56 bytes.  This is also
     * where the selector starts out as 0 for the two paths that never store
     * one (phase 4 and the out-of-table branch), which mirrors the ROM. */
    for (i = 0; i < 56; ++i) pkt[i] = 0;
    sel = PK_U16(0);

    /* 0x0800B1A2 — jump table _0800B1CC on the signed phase cell, entered
     * only while (u32)(s32)phase <= 7 (cmp #7 + bls). */
    {
        s16 phase = WA_S16(0x0FBC);
        if ((u32)phase > 7u) goto tail;
        switch ((u16)phase) {
        case 0:  goto c01F4;
        case 1:  goto c0254;
        case 2:  goto c025C;
        case 3:  goto c02AC;
        case 4:  goto tail;
        case 5:  goto c023C;
        case 6:  goto c01EC;
        default: goto c0264;
        }
    }

c01EC: /* 0x0800B1EC */
    SET_SEL(0);
    goto tail;

c01F4: /* 0x0800B1F4 */
    if (WA_S16(0x103A) > 2) {            /* cmp #2 + bgt (signed) */
        SET_SEL(1);
        PK_U16(6) = (u16)_08025790(WA_S16(0x0FF2), WA_S16(0x0FF6),
                                   WA_S16(0x103A));
    } else {
        SET_SEL(2);
    }
    goto tail;

c023C: /* 0x0800B23C */
    SET_SEL(4);
    PK_U16(6) = (u16)_0802591C(WA_S16(0x0576));
    goto tail;

c0254: /* 0x0800B254 */
    SET_SEL(3);
    goto tail;

c025C: /* 0x0800B25C */
    SET_SEL(5);
    goto tail;

c0264: { /* 0x0800B264 */
    s16 v = WA_S16(0x1078);
    if (v == 1) {                        /* beq 0x0800B288 */
        SET_SEL(7);
        PK_U16(6) = (u16)_0802591C(WA_S16(0x0576));
    } else if (v == 2) {                 /* bne -> 0x0800B2A4 */
        SET_SEL(6);
    } else {
        SET_SEL(7);
    }
    goto tail;
}

c02AC: { /* 0x0800B2AC */
    s16 idx;
    const u8 *rec;
    SET_SEL(8);
    PK_U16(40) = WA_U16(0x0FC4);
    idx = WA_S16(0x0574);
    rec = RD_WA + 0x30 + (s32)idx * 12;  /* ldrb/lsls #24/asrs #24 */
    PK_U16(42) = (s16)*(volatile s8 *)rec;
    for (i = 0; i < 12; ++i) pkt[44 + i] = rec[i];
    pkt[45] = (u8)WA_U16(0x0FC6);        /* strb at sp+45 overwrites the copy */
    goto tail;
}

tail: /* 0x0800B2F8 */
    sel = PK_U16(0);                     /* the ROM re-reads sp+0 here */
    pkt[21] = (u8)WA_U16(0x0FEE);
    PK_U16(2) = WA_U16(0x0576);
    PK_U16(4) = WA_U16(0x0FF6);
    if (WA_S16(0x0FBC) == 0) {
        PK_U16(8)  = (u16)_080256F4(WA_S16(0x0FF6), WA_S16(0x103A), 0);
        PK_U16(10) = (u16)_08025710(WA_S16(0x0FF6), WA_S16(0x103A), 0);
    } else {
        PK_U16(8)  = WA_U16(0x0FE6);
        PK_U16(10) = WA_U16(0x0FE4);
    }

    /* 0x0800B3D0 — pkt+12, before the selector checks. */
    if (sel == 2) {                      /* ldrh + cmp #2 */
        PK_U32(12) = (u32)(u16)_08025750(WA_S16(0x0FF2), WA_S16(0x0FF6),
                                         WA_S16(0x103A));
    } else {
        s16 idx = WA_S16(0x0576);
        PK_U32(12) = WA_U32(0x05E4 + (s32)idx * 72);
    }

    /* 0x0800B3D6 — (u16)(sel-6) <= 1, else the signed sel == 2 re-test. */
    if (sel == 6 || sel == 7) {
        rec_off = 0x0FC2;
    } else if (sel == 2) {
        /* 0x0800B400 — the i/j arm: it replaces the *record* block
         * (+24/+26/+28..39) with two catalog fields and a clear. */
        PK_U16(24) = (u16)_080257D4(WA_S16(0x0FF6), WA_S16(0x103A), 0);
        pkt[29] = (u8)_080257F0(WA_S16(0x0FF6), WA_S16(0x103A), 0);
        PK_U16(26) = 0;
        goto finish;
    } else {
        rec_off = 0x0574;                /* 0x0800B440 */
    }

    /* 0x0800B446 — index a 12-byte record by the u16 at WA+rec_off. */
    {
        u16 v = WA_U16(rec_off);
        const u8 *rec;
        PK_U16(24) = v;
        rec = RD_WA + 0x30 + (s32)(s16)v * 12;
        PK_U16(26) = (s16)*(volatile s8 *)rec;
        for (i = 0; i < 12; ++i) pkt[28 + i] = rec[i];
    }

finish: /* 0x0800B478 */
    pkt[22] = (u8)WA_U16(0x0FF2);
    pkt[20] = (u8)WA_U16(0x05E0);        /* ldrh at WA + (188 << 3) */
    _080188B0(pkt);
}

#ifndef __APPLE__
void _0800B190(void) __attribute__((alias("Race_Dispatch")));
// The closure spells this address ONLY `sub_0800B190` (asm/race_dispatch.s),
// and promotion_screen wants a call site to use the spelling the closure uses.
// Aliased to the REAL BODY, one hop, never to `_0800B190` above: gcc emits
// `.thumb_set sub_0800B190, _0800B190` for an alias-of-an-alias, and the slice
// link splices only the promoted body's own section, so an intermediate
// spelling is left undefined at link time with every other gate green.
// src/foundation_subsys.c:548-551 is the precedent for this exact shape.
void sub_0800B190(void) __attribute__((alias("Race_Dispatch")));
#endif
