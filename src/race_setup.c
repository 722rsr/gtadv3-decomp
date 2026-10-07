#include "gtadv/ghost.h"
void sub_080019860(void) __attribute__((alias("Race_Setup_19860")));  /* rule 6: the splice replaces this label */
#include "gba/types.h"
#include "gtadv/memory.h"
#include <stdint.h>


extern u32 Ghost_FlagTest(u32 mask); // _08018ACC
extern void Ghost_FlagOp(u32 mask,int set); // _08018AA8
extern void Course_0x080263E0(u8 *a, int b); // ROM: (r0 = buf, r1 = s16[racectx+132])
// ROM call shapes (asm/race_setup_18adc.s _08018B18/_08018B4A):
//   _080264D8(u32[racer+280], racer[8]-racectx[8]) -> derivation feeding the
//   263E0 blit; _080267FC(u16[racer+26], racer[8]-racectx[8]) -> result goes
//   to _08026868(result, s16[racectx+86]) or zeroes racer[31].
extern void *Course_0x080267FC(int a, u32 b); // 0x080267FC
extern void *_0800264D8(void *a, u32 b);     // 0x080264D8 course byte lookup
extern void  _080026868(u8 *a, int b);       // 0x08026868 palette upload

void Race_Setup_18ADC(void) {
    volatile u8 *racers = (volatile u8 *)RACER_ARRAY_BASE;
    volatile u8 *racectx = *(volatile u8 *volatile *)RACE_CTX_PTR_ADDR;
    // flag 1024
    u32 f = Ghost_FlagTest(1024);
    if (f) {
        racers[31] = 1;
        Ghost_FlagOp(1024,0);
    } else {
        racers[31] = 0;
    }
    // _08018B06..0x08018B68 (ROM): course-byte derivation feeds the blit and
    // the palette lane.  src = u32[racer+280], delta = racer[8]-racectx[8].
    void *src = *(void *volatile *)(racers + 280);
    u32 delta = *(volatile u32 *)(racers + 8) - *(volatile u32 *)(racectx + 8);
    void *res = _0800264D8(src, delta);
    volatile u32 *rc148 = (volatile u32 *)(racectx + 148);
    if (res != (void *)*rc148) {
        *rc148 = (u32)(uintptr_t)res;
        s16 v = *(volatile s16 *)(racectx + 132);
        Course_0x080263E0((u8 *)res, v);
    }
    if (racers[31] != 0) {
        void *pal = Course_0x080267FC((int)*(volatile u16 *)(racers + 26), delta);
        if (pal == 0) {
            racers[31] = 0;
        } else {
            _080026868((u8 *)pal, *(volatile s16 *)(racectx + 86));
        }
    }
    // ROM tail 0x08018B6A..0x08018C2E (stack-template CpuFastSet pair over
    // pools 0x08018C30/0x08018C34 + the per-record loop) is not yet
    // transcribed — recorded as a body residual.
}

#ifndef __APPLE__
void _080018ADC(void) __attribute__((alias("Race_Setup_18ADC")));
#endif

// _0800191BC: s16[racectx+90] setter gated by table 0x080CBB30 (u32 entries)
// Pools: _0800191D8=0x03004E20, _0800191FC=0x080CBB30; widths: ldrsh/strh, flag table compare
void Race_Setup_191BC(u32 a0) {
    u16 v16 = (u16)(a0 & 0xFFFFu); // lsls #16 / lsrs #16 width
    volatile u8 *racectx = *(volatile u8 *volatile *)0x03004E20;
    volatile s16 *p90 = (volatile s16 *)(racectx + 90);
    s16 cur = *p90;
    if (cur < 0) {
        *p90 = (s16)v16;
        return;
    }
    volatile u32 *tbl = (volatile u32 *)0x080CBB30;
    // lsls #16 / asrs #14 => *4 as signed halfword; values are small positive so direct index
    s16 sv = (s16)v16;
    s16 sc = cur;
    // tbl index as s16 (asm: lsls #2 / adds) — preserve signed, but positive path
    u32 tv = tbl[(int)sv];
    u32 tc = tbl[(int)sc];
    if (tv >= tc) return;
    *p90 = (s16)v16;
}
#ifndef __APPLE__
void _0800191BC(u32 a0) __attribute__((alias("Race_Setup_191BC")));
#endif

// _0800195B8: flag 0x40000 (128<<11) at racectx+92, increment or zero, threshold 19 -> _0801B498
extern void Sub_0801B498(void); // _0801B498
void Race_Setup_195B8(void) {
    u32 t = Ghost_FlagTest(0x40000u); // 128<<11
    volatile u8 *racectx = *(volatile u8 *volatile *)0x03004E20;
    volatile u16 *p92 = (volatile u16 *)(racectx + 92);
    if (t != 0) {
        u16 v = *p92;
        v = (u16)(v + 1);
        *p92 = v;
        s16 sv = (s16)v;
        if (sv > 19) Sub_0801B498();
    } else {
        *p92 = 0;
    }
}
#ifndef __APPLE__
void _0800195B8(void) __attribute__((alias("Race_Setup_195B8")));
#endif

// _0800195F8: dual-flag handler at racectx+94 (flags 0x2000 and 0x40000)
// Pools: _080019620/_0800196D8=0x03004E20, widths: ldrh/ldrsh/strh, flag tests
void Race_Setup_195F8(void) {
    u32 t1 = Ghost_FlagTest(0x2000u); // 128<<6 = 8192
    volatile u8 *racectx = *(volatile u8 *volatile *)0x03004E20;
    volatile u16 *p94 = (volatile u16 *)(racectx + 94);
    if (t1 != 0) {
        u16 v = *p94;
        v = (u16)(v + 1);
        *p94 = v;
        s16 sv = (s16)v;
        if (sv > 120) {
            *p94 = 120;
        }
    } else {
        // ldrsh path: if *(s16*)(p94) >0 then *p94 -=3
        s16 cur = *(volatile s16 *)p94;
        if (cur > 0) {
            u16 raw = *p94;
            raw = (u16)(raw - 3);
            *p94 = raw;
        }
    }
    u32 t2 = Ghost_FlagTest(0x40000u); // 128<<11 = 0x40000
    if (t2 != 0) {
        u16 v = *p94;
        v = (u16)(v - 3);
        *p94 = v;
    }
    // Tail at _080019652: clamp if <=0 then Ghost_FlagOp etc. and _08008284 scaled call
    // Beyond small-leaf scope: leave exact blocker — needs full racectx+94 scaled math proof
    s16 cur2 = *(volatile s16 *)p94;
    if (cur2 <= 0) {
        *p94 = 0;
        // Ghost_FlagOp(1<<22) / (1<<7) and Sub_08008284 scaled path omitted here — blocked
    }
    // Scaled _0802DE04 / _08008284 path is open-bus data (>64KB racectx), left TODO
}
#ifndef __APPLE__
void _0800195F8(void) __attribute__((alias("Race_Setup_195F8")));
#endif

// _080019860: direct wrapper for _08027FCC (no pool, bl only)
extern void _080027FCC(void);   // slice-closure spelling; body + `Sub_` alias in src/garage.c
void Race_Setup_19860(void) { _080027FCC(); }
#ifndef __APPLE__
void _080019860(void) __attribute__((alias("Race_Setup_19860")));
#endif

// _080018EC0: zero-loop at 0x03000598, WA+0x10CA s16 count, 0x03005E48 and racectx+70
// Pools: 0x03001780+0x10CA s16, 0x03005E48, 0x03004E20+70, 0x03000598 array, 0x00FFFFFF
void Race_Setup_18EC0(void) {
    volatile u8 *wa = (volatile u8 *)0x03001780;
    s16 cnt = *(volatile s16 *)(wa + 0x10CA);
    s16 n = (s16)(cnt - 1);
    volatile u32 *arr = (volatile u32 *)0x03000598;
    for (int i = 0; i < n; i++) {
        arr[i] = 0; // stmia r3!,{r4} where r4=0, width u32
    }
    *(volatile u32 *)0x03005E48 = 0x00FFFFFFu;
    volatile u8 *racectx = *(volatile u8 *volatile *)0x03004E20;
    *(volatile u16 *)(racectx + 70) = 1; // movs #1 strh
}
#ifndef __APPLE__
void _080018EC0(void) __attribute__((alias("Race_Setup_18EC0")));
#endif

// _080019200: pure compare of two racer structs via +28 pointer and +18/+20/+22 s16 fields
// Returns 1 if d>0 or tie-break via mul, else 0; helper 0x08006C10
extern void *Sub_08006C10(void *a);
int Race_Setup_19200(void *a, void *b) {
    void *pA = *(void *volatile *)((volatile u8 *)a + 28);
    void *pB = *(void *volatile *)((volatile u8 *)b + 28);
    s16 hA18 = *(volatile s16 *)((volatile u8 *)pA + 18);
    s16 hB18 = *(volatile s16 *)((volatile u8 *)pB + 18);
    int d = (int)hA18 - (int)hB18;
    if (d > 0) return 1;
    if (d != 0) return 0;
    void *tmp = Sub_08006C10(pB); // bl with r0=pB, r1=0 in asm
    s32 t20 = (s32)*(volatile s16 *)((volatile u8 *)tmp + 20);
    s32 t22 = (s32)*(volatile s16 *)((volatile u8 *)tmp + 22);
    // Second tie-break uses racer deltas at +4/+8 (u32)
    u32 a4 = *(volatile u32 *)((volatile u8 *)a + 4);
    u32 b4 = *(volatile u32 *)((volatile u8 *)b + 4);
    u32 a8 = *(volatile u32 *)((volatile u8 *)a + 8);
    u32 b8 = *(volatile u32 *)((volatile u8 *)b + 8);
    s32 d4 = (s32)(a4 - b4);
    s32 d5 = (s32)(a8 - b8);
    s32 res = t20 * d5 - t22 * d4; // muls as in asm, s32
    if (res > 0) return 1;
    return 0;
}
#ifndef __APPLE__
int _080019200(void *a, void *b) __attribute__((alias("Race_Setup_19200")));
#endif

// _080019250: delta-angle helper with WA+0x1111 s8 flag and racectx+130 byte, calls 0x0802A8F0
// Pools: 0x03001780+0x1111, 0x03004E20+130, helper 0x0802A8F0 (s16*8 clamp)
extern int Sub_0802A8F0(void *a, int v);
int Race_Setup_19250(void *a, void *b) {
    s16 d22a = *(volatile s16 *)((volatile u8 *)a + 22);
    s16 d22b = *(volatile s16 *)((volatile u8 *)b + 22);
    int d = (int)d22a - (int)d22b;
    volatile u8 *wa = (volatile u8 *)0x03001780;
    s8 flag = *(volatile s8 *)(wa + 0x1111); // ldrb + lsls #24 asrs #24
    if (flag != 0) {
        int neg = -d;
        int v = neg << 3; // lsls #3
        if (v > 36) v = 36;
        if (v < 0) v = 0;
        Sub_0802A8F0(a, v);
    }
    volatile u8 *racectx = *(volatile u8 *volatile *)0x03004E20;
    volatile u8 *p130 = racectx + 130;
    u8 cur130 = *p130;
    if (cur130 != 0) {
        *p130 = 0;
        return 1;
    }
    if (d > 0) return 1;
    if (d != 0) return 0;
    // tie case uses Sub_08006C10 and WA+0x10C3 etc. — needs racer +0/+4 layout proven, leave blocked
    // For bounded pure, return 0 where tie and flag 0
    return 0;
}
#ifndef __APPLE__
int _080019250(void *a, void *b) __attribute__((alias("Race_Setup_19250")));
#endif

// _080018F14: sorted insert at 0x03000598 by word at +204 vs 0x03005E48 head
// Pools: 0x03005E48 Vu32 gv, 0x03000598 Vu32 arr[3], widths ldr/str Vu32, ble/bge signed
void Race_Setup_18F14(void *a) {
    volatile u8 *pa = (volatile u8 *)a;
    volatile u32 *pGlobal = (volatile u32 *)0x03005E48;
    volatile u32 *arr = (volatile u32 *)0x03000598;
    u32 key = *(volatile u32 *)(pa + 204);
    u32 gv = *pGlobal;
    if ((s32)gv <= (s32)key) return;
    if (arr[0] == 0) {
        arr[0] = (u32)(uintptr_t)a;
        goto tail;
    }
    for (int r4 = 0; r4 <= 2; r4++) {
        u32 slot = arr[r4];
        if (slot == 0) {
            arr[r4] = (u32)(uintptr_t)a;
            goto tail;
        }
        u32 slotKey = *(volatile u32 *)((volatile u8 *)(uintptr_t)slot + 204);
        if ((s32)key >= (s32)slotKey) {
            continue;
        }
        for (int i = 2; i > r4; i--) arr[i] = arr[i-1];
        arr[r4] = (u32)(uintptr_t)a;
        goto tail;
    }
tail:
    {
        u32 v2 = arr[2];
        if (v2 != 0) {
            u32 k2 = *(volatile u32 *)((volatile u8 *)(uintptr_t)v2 + 204);
            *pGlobal = k2;
        }
    }
}
#ifndef __APPLE__
void _080018F14(void *a) __attribute__((alias("Race_Setup_18F14")));
#endif

// Remaining 10 funcs in span remain TODO with exact VMA blockers:
// 0x080018F94 (sorted array drain), 0x080019318 (large copy via 0x0802DDD0),
// 0x08001992C/0x0800199CC/0x080019A2C/0x080019AEC/0x080019D6C/0x080019EC8 etc. — left blocked (needs racer +28 layout etc.)

// ROM entry alias.
#ifndef __APPLE__
void RaceScene_Leaf_5B8(void) __attribute__((alias("Race_Setup_195B8")));
void RaceScene_Leaf_6E8(void) __attribute__((alias("Race_Setup_195F8")));
void _08018ADC(void) __attribute__((alias("Race_Setup_18ADC")));
void _080191BC(u32 a0) __attribute__((alias("Race_Setup_191BC")));
int _08019200(void *a, void *b) __attribute__((alias("Race_Setup_19200")));
#endif

// ----------------------------------------------------------------------------
// 0x0802A8F0 — 4B draft/proximity trim setter (asm/garage_26f50.s, tucked
// behind 0x0802A8EC's epilogue): *(u16*)(rec+148) = v; return 0. The sole
// writer of the +148 trim consumed by sub_080029A74 (car_dynamics.md §9).
int RaceDraftTrim_2A8F0(void *rec, int v) {
    *(volatile u16 *)((volatile u8 *)rec + 148) = (u16)v;
    return 0;
}
#ifndef __APPLE__
int _0802A8F0(void *a, int v) __attribute__((alias("RaceDraftTrim_2A8F0")));
int sub_0802A8F0(void *a, int v) __attribute__((alias("RaceDraftTrim_2A8F0")));
int Sub_0802A8F0(void *a, int v) __attribute__((alias("RaceDraftTrim_2A8F0")));
#endif
