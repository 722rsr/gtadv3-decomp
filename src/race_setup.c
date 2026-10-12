#include "gtadv/ghost.h"
void sub_080019860(void) __attribute__((alias("Race_Setup_19860")));  /* rule 6: the splice replaces this label */
#include "gba/types.h"
#include "gtadv/memory.h"
#include <stdint.h>


extern u32 Ghost_FlagTest(u32 mask); // _08018ACC
extern void Ghost_FlagOp(u32 mask,int set); // _08018AA8
extern int _080044C4(u32 a, u32 b, volatile u32 *c);
extern int _08004508(volatile u32 *a);
extern int _08002BE8(void);
extern void _08002C48(int value, u16 course);
// The assembler entry is spelled _080026550; it takes the three live inputs.
extern void _080026550(void *source, u32 delta, volatile u32 *record);
extern void _080023AC(int slot, void *handler);
extern void Course_0x080263E0(u8 *a, int b); // ROM: (r0 = buf, r1 = s16[racectx+132])
// ROM call shapes (asm/race_setup_18adc.s _08018B18/_08018B4A):
//   _080264D8(u32[racer+280], racer[8]-racectx[8]) -> derivation feeding the
//   263E0 blit; _080267FC(u16[racer+26], racer[8]-racectx[8]) -> result goes
//   to _08026868(result, s16[racectx+86]) or zeroes racer[31].
extern void *Course_0x080267FC(int a, u32 b); // 0x080267FC
extern void *_0800264D8(void *a, u32 b);     // 0x080264D8 course byte lookup
extern void  _080026868(u8 *a, int b);       // 0x08026868 palette upload

// 0x08018E28 — initialize one racer record from the live race context. The
// index is sign-extended from its low halfword; the record stride is 284 bytes.
void Race_Setup_18E28(u32 index_arg) {
    register s32 index __asm__("r0") = (s16)index_arg;
    register s32 scaled __asm__("r1");
    register volatile u8 *record __asm__("r4");
    register volatile u32 *inner __asm__("r5");
    register volatile u32 *field_dest __asm__("r2");
    register volatile u8 *volatile *context_cell __asm__("r6");
    register s32 field __asm__("r0");
    register u32 zero __asm__("r3");
    register volatile s32 *score __asm__("r1");
    volatile u8 *context;
    volatile u8 *lookup;
    volatile s32 *random_dest;
    scaled = index * 9;
    scaled <<= 3;
    scaled -= index;
    scaled <<= 2;
    record = (volatile u8 *)(uintptr_t)(0x03004E80u + scaled);
    field_dest = (volatile u32 *)(record + 224);
    __asm__ volatile("" : "+r"(field_dest) : : "memory");
    context_cell = (volatile u8 *volatile *)(uintptr_t)0x03004E20u;
    context = *context_cell;
    index <<= 1;
    __asm__ volatile("" : "+r"(index) : : "memory");
    lookup = context + 132;
    lookup += index;
    zero = 0;
    __asm__ volatile("ldrsh %0, [%1, %2]"
                     : "=r"(field) : "r"(lookup), "r"(zero) : "memory");
    *field_dest = field;
    *(volatile u32 *)(record + 228) = 160 << 7;
    {
        u32 first = *(volatile u32 *)(record + 0);
        u32 second = *(volatile u32 *)(record + 4);
        inner = (volatile u32 *)(record + 200);
        __asm__ volatile("" : "+r"(inner) : : "memory");
        if (_080044C4(first, second, inner) == 0)
            return;
    }
    if (_08004508(inner) == 0)
        return;

    {
        s32 raw_random = _08002BE8();
        random_dest = (volatile s32 *)(record + 220);
        __asm__ volatile("" : "+r"(random_dest) : : "memory");
        s32 random = (s16)raw_random;
        *random_dest = random;
        _08002C48(random, *(volatile u16 *)((volatile u8 *)random_dest - 4));
    }
    {
        score = (volatile s32 *)(record + 208);
        __asm__ volatile("" : "+r"(score) : : "memory");
        s32 value = *score;
        if (value < 0) value += 15;
        *score = value >> 4;
    }
    {
        void *source = (void *)(uintptr_t)*(volatile u32 *)(record + 280);
        volatile u8 *current_context = *context_cell;
        u32 position = *(volatile u32 *)(record + 8);
        u32 origin = *(volatile u32 *)(current_context + 8);
        _080026550(source, position - origin, inner);
    }
}
void _080018E28(u32 index) __attribute__((alias("Race_Setup_18E28")));

// 0x08019BF8 — configure three display halfwords and install the scanline
// callback used by the adjacent VCOUNT handlers.
void Race_Setup_19BF8(void) {
    register volatile u16 *display __asm__("r1") =
        (volatile u16 *)(uintptr_t)0x04000008u;
    __asm__ volatile("" : "+r"(display) : : "memory");
    *display = 0x1C4B;
    display += 2;
    *display = 0xE8C2;
    display -= 6;
    *display = 0x1741;
    _080023AC(3, (void *)(uintptr_t)0x08019C35u);
}
void _080019BF8(void) __attribute__((alias("Race_Setup_19BF8")));

// 0x08019C34 — on the second VCOUNT phase, configure the display scroll
// registers from the active race context and install the next scanline handler.
void Race_Setup_19C34(void) {
    volatile u8 *context;
    u32 position;
    register volatile u16 *scroll __asm__("r2");
    if (*(volatile u8 *)(uintptr_t)0x04000006u <= 159)
        return;
    *(volatile u16 *)(uintptr_t)0x04000000u = 0x1341;
    *(volatile u16 *)(uintptr_t)0x0400000Au = 0x5E49;
    scroll = (volatile u16 *)(uintptr_t)0x04000014u;
    __asm__ volatile("" : "+r"(scroll) : : "memory");
    context = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
    position = *(volatile u32 *)(context + 8);
    *scroll = (u16)(position >> 2);
    scroll++;
    *scroll = 6;
    *(volatile u16 *)(uintptr_t)0x04000010u = (u16)(position >> 3);
    *(volatile u16 *)(uintptr_t)0x04000012u = 0;
    _080023AC(3, (void *)(uintptr_t)0x08019C9Du);
}
void _080019C34(void) __attribute__((alias("Race_Setup_19C34")));

// 0x08019CBC — compare VCOUNT with the signed threshold at 0x0300576A,
// acknowledge the display interrupt, and install the following scanline hook.
void _080019CBC(void) {
    register volatile u8 *vcount_cell __asm__("r0") =
        (volatile u8 *)(uintptr_t)0x04000006u;
    register s32 limit __asm__("r1") = 0x03005760;
    register u32 offset __asm__("r2") = 10;
    u8 vcount;
    __asm__ volatile("" : "+r"(vcount_cell) : : "memory");
    __asm__ volatile("" : "+r"(limit) : : "memory");
    __asm__ volatile("" : "+r"(offset) : : "memory");
    __asm__ volatile("ldrsh %0, [%0, %1]"
                     : "+r"(limit) : "r"(offset) : "memory");
    vcount = *vcount_cell;
    if (vcount < limit)
        return;
    *(volatile u16 *)(uintptr_t)0x04000000u = 0x1641;
    _080023AC(3, (void *)(uintptr_t)0x08019CF5u);
}

// 0x08019DDC — second VCOUNT phase with its own display value and hook.
void _080019DDC(void) {
    register volatile u8 *vcount_cell __asm__("r0") =
        (volatile u8 *)(uintptr_t)0x04000006u;
    register s32 limit __asm__("r1") = 0x03005760;
    register u32 offset __asm__("r2") = 10;
    u8 vcount;
    __asm__ volatile("" : "+r"(vcount_cell) : : "memory");
    __asm__ volatile("" : "+r"(limit) : : "memory");
    __asm__ volatile("" : "+r"(offset) : : "memory");
    __asm__ volatile("ldrsh %0, [%0, %1]"
                     : "+r"(limit) : "r"(offset) : "memory");
    vcount = *vcount_cell;
    if (vcount < limit)
        return;
    *(volatile u16 *)(uintptr_t)0x04000000u = 0x1741;
    _080023AC(3, (void *)(uintptr_t)0x08019E15u);
}

// 0x08019D30 — install the next VCOUNT callback and select its display mode.
void _080019D30(void) {
    register volatile u16 *display __asm__("r1");
    _080023AC(3, (void *)(uintptr_t)0x08019D6Du);
    display = (volatile u16 *)(uintptr_t)0x04000008u;
    __asm__ volatile("" : "+r"(display) : : "memory");
    *display = 0x1C49;
    display += 2;
    *display = 0xE8C3;
    display -= 6;
    *display = 0x1741;
}

// 0x08019E80 — alternate display mode before returning to the racing hook.
void _080019E80(void) {
    register volatile u16 *display __asm__("r1");
    _080023AC(3, (void *)(uintptr_t)0x08019EC9u);
    display = (volatile u16 *)(uintptr_t)0x04000008u;
    __asm__ volatile("" : "+r"(display) : : "memory");
    *display = 0x1CC8;
    display++;
    *display = 0x1E49;
    display++;
    *display = 0xE8C3;
    display -= 6;
    *display = 0x0141;
}

// 0x08019CF4 — after VCOUNT 79, reset scroll state and continue the handler chain.
void _080019CF4(void) {
    volatile u16 *scroll;
    if (*(volatile u8 *)(uintptr_t)0x04000006u <= 79)
        return;
    *(volatile u16 *)(uintptr_t)0x0400000Au = 0x5E48;
    scroll = (volatile u16 *)(uintptr_t)0x04000014u;
    *scroll = 0;
    scroll++;
    *scroll = 0;
    _080023AC(3, (void *)(uintptr_t)0x08019C35u);
}

// 0x08019E14 — reset the display state and derive both scroll coordinates
// from the packed mode word at 0x0300285C.
void _080019E14(void) {
    register volatile u16 *scroll __asm__("r3");
    register volatile u8 *mode_base __asm__("r0");
    register u32 offset __asm__("r1");
    register u32 packed __asm__("r2");
    u32 component_a;
    u32 component_b;
    if (*(volatile u8 *)(uintptr_t)0x04000006u <= 79)
        return;
    *(volatile u16 *)(uintptr_t)0x0400000Au = 0x5E48;
    *(volatile u16 *)(uintptr_t)0x04000014u = 0;
    *(volatile u16 *)(uintptr_t)0x04000016u = 0;
    scroll = (volatile u16 *)(uintptr_t)0x04000010u;
    mode_base = (volatile u8 *)(uintptr_t)0x03001780u;
    offset = 0x10DC;
    __asm__ volatile("" : "+r"(scroll) : : "memory");
    __asm__ volatile("" : "+r"(mode_base) : : "memory");
    __asm__ volatile("" : "+r"(offset) : : "memory");
    mode_base += offset;
    packed = *(volatile u32 *)mode_base;
    component_a = packed & 31;
    component_b = packed & 7;
    *scroll = (u16)(component_a * component_b);
    scroll++;
    component_a = packed & 63;
    packed &= 15;
    *scroll = (u16)(component_a * packed);
    _080023AC(3, (void *)(uintptr_t)0x08019D6Du);
}

// 0x08019F1C — set the blend registers from the current race phase.
void _080019F1C(void) {
    register volatile u16 *blend __asm__("r3");
    register u16 phase __asm__("r2");
    volatile u8 *context;
    if (*(volatile u8 *)(uintptr_t)0x04000006u > 56)
        return;
    if (Ghost_FlagTest(0x40000000u) == 0)
        return;
    *(volatile u16 *)(uintptr_t)0x04000050u = 0x0140;
    blend = (volatile u16 *)(uintptr_t)0x04000052u;
    context = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
    phase = *(volatile u16 *)(context + 116);
    *blend = (u16)(((u32)phase << 8) | (16 - phase));
    _080023AC(3, (void *)(uintptr_t)0x08019EC9u);
}

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

void _080018ADC(void) __attribute__((alias("Race_Setup_18ADC")));

// _0800191BC: s16[racectx+90] setter gated by table 0x080CBB30 (u32 entries)
// Pools: _0800191D8=0x03004E20, _0800191FC=0x080CBB30; widths: ldrsh/strh, flag table compare
void Race_Setup_191BC(u32 a0) {
    u16 v16 = (u16)(a0 & 0xFFFFu); // lsls #16 / lsrs #16 width
    volatile u8 *racectx = *(volatile u8 *volatile *)0x03004E20;
    volatile s16 *p90 = (volatile s16 *)(racectx + 90);
    // Keep the signed halfword access as ldrsh; the store remains volatile.
    s16 cur = *(s16 *)(void *)p90;
    if (cur < 0) {
        *p90 = (s16)v16;
        return;
    }
    volatile u32 *tbl = (volatile u32 *)0x080CBB30;
    // lsls #16 / asrs #14 => *4 as signed halfword table index.
    s16 sv = (s16)v16;
    // Materialize the first table address in r1 before reloading the slot.
    register volatile u32 *tvPtr __asm__("r1") = &tbl[(int)sv];
    __asm__ volatile("" : "+r"(tvPtr));
    // The ROM reloads p90 with ldrsh [r3,r5] after the first index. Keep the
    // zero index in r5 and the second read distinct from the first.
    register s32 zero __asm__("r5") = 0;
    __asm__ volatile("" : "+r"(zero));
    __asm__ volatile("" : : : "memory");
    s32 sc;
    __asm__ volatile("ldrsh %0, [%1, %2]"
                     : "=r"(sc) : "r"(p90), "r"(zero) : "memory");
    // Pin the second address in r0; the ROM compares table values as signed.
    register volatile u32 *tcPtr __asm__("r0") = &tbl[(int)sc];
    s32 tv = (s32)*tvPtr;
    s32 tc = (s32)*tcPtr;
    if (tv >= tc) return;
    *p90 = (s16)v16;
}
void _0800191BC(u32 a0) __attribute__((alias("Race_Setup_191BC")));

// _0800195B8: flag 0x40000 (128<<11) at racectx+92, increment or zero, threshold 19 -> _0801B498(3)
extern void sub_08001B498(int sel); // 0x0801B498 closure entry
void Race_Setup_195B8(void) {
    u32 t = Ghost_FlagTest(0x40000u); // 128<<11
    if (t != 0) {
        volatile u8 *racectx = *(volatile u8 *volatile *)0x03004E20;
        int v = (int)*(volatile u16 *)(racectx + 92) + 1;
        *(volatile u16 *)(racectx + 92) = (u16)v;
        if ((s16)v > 19)
            sub_08001B498(3);
    } else {
        volatile u8 *racectx = *(volatile u8 *volatile *)0x03004E20;
        *(volatile u16 *)(racectx + 92) = (u16)t;
    }
}
void _0800195B8(void) __attribute__((alias("Race_Setup_195B8")));

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
void _0800195F8(void) __attribute__((alias("Race_Setup_195F8")));

// _080019860: direct wrapper for _08027FCC (no pool, bl only)
extern void _080027FCC(void);   // slice-closure spelling; body + `Sub_` alias in src/garage.c
void Race_Setup_19860(void) { _080027FCC(); }
__asm__(".align 2, 0");
void _080019860(void) __attribute__((alias("Race_Setup_19860")));

// _080018EC0: zero-loop at 0x03000598, WA+0x10CA s16 count, 0x03005E48 and racectx+70
// Pools: 0x03001780+0x10CA s16, 0x03005E48, 0x03004E20+70, 0x03000598 array, 0x00FFFFFF
__attribute__((naked)) void Race_Setup_18EC0(void) {
    __asm__ volatile(
        "push {r4, r5, r6, r7, lr}\n\t"
        "movs r1, #0\n\t"
        "ldr r0, .L18ec0_wa\n\t"
        "ldr r3, .L18ec0_count_offset\n\t"
        "add r2, r0, r3\n\t"
        "movs r7, #0\n\t"
        "ldrsh r0, [r2, r7]\n\t"
        "sub r0, r0, #1\n\t"
        "ldr r5, .L18ec0_field\n\t"
        "ldr r6, .L18ec0_ctx\n\t"
        "cmp r1, r0\n\t"
        "bge .L18ec0_done\n\t"
        "movs r4, #0\n\t"
        "ldr r3, .L18ec0_array\n"
        ".L18ec0_loop:\n\t"
        "stmia r3!, {r4}\n\t"
        "add r1, r1, #1\n\t"
        "movs r7, #0\n\t"
        "ldrsh r0, [r2, r7]\n\t"
        "sub r0, r0, #1\n\t"
        "cmp r1, r0\n\t"
        "blt .L18ec0_loop\n"
        ".L18ec0_done:\n\t"
        "ldr r0, .L18ec0_fill\n\t"
        "str r0, [r5, #0]\n\t"
        "ldr r0, [r6, #0]\n\t"
        "add r0, r0, #70\n\t"
        "movs r1, #1\n\t"
        "strh r1, [r0, #0]\n\t"
        "pop {r4, r5, r6, r7}\n\t"
        "pop {r0}\n\t"
        "bx r0\n"
        ".balign 4\n"
        ".L18ec0_wa: .word 0x03001780\n"
        ".L18ec0_count_offset: .word 0x000010CA\n"
        ".L18ec0_field: .word 0x03005E48\n"
        ".L18ec0_ctx: .word 0x03004E20\n"
        ".L18ec0_array: .word 0x03000598\n"
        ".L18ec0_fill: .word 0x00FFFFFF\n");
}
void _080018EC0(void) __attribute__((alias("Race_Setup_18EC0")));

// _080019200: pure compare of two racer structs via +28 pointer and +18/+20/+22 s16 fields.
// d = (s16)[a->28 + 18] - (s16)[b->28 + 18]; d>0 -> 1, d<0 -> 0;
// on a tie: tmp = 06C10(b->28, 0), d4 = a[4]-b[4], d5 = a[8]-b[8],
// res = (s16)tmp[20]*d5 - (s16)tmp[22]*d4; the ROM returns 1 for res<=0 and
// 0 for res>0 (the branch at 0x0801923E is `bgt` to the return-0 block).
extern void *_08006C10(void *a, int b);
int Race_Setup_19200(void *a, void *b) {
    // The ROM keeps a in r6 and b in r7. Pin a; keep b ordinary and reserve
    // r5 at the initial barrier so agbcc allocates the live-across-call b to
    // r7 and records the required callee-save.
    register s32 d4 __asm__("r4");
    register s32 d5 __asm__("r5");
    register u8 *ra __asm__("r6") = (u8 *)a;
    u8 *rb = (u8 *)b;
    __asm__ volatile("" : "+r"(ra), "+r"(rb) : : "r5", "memory");
    u8 *pA = *(u8 **)(ra + 28);
    register s32 hA18 __asm__("r1");
    __asm__ volatile("movs r2, #18\n\tldrsh %0, [%1, r2]"
                     : "=&r"(hA18) : "r"(pA) : "r2", "memory");
    u8 *pB = *(u8 **)(rb + 28);
    s16 hB18 = *(s16 *)(pB + 18);
    int d = (int)hA18 - (int)hB18;
    if (d > 0) return 1;
    if (d != 0) return 0;
    u8 *tmp = (u8 *)_08006C10(pB, 0);
    // ROM keeps the two racer deltas in r4 (a[4]-b[4]) and r5 (a[8]-b[8]).
    register s32 a4 __asm__("r2") = *(s32 *)(ra + 4);
    register s32 b4 __asm__("r1") = *(s32 *)(rb + 4);
    d4 = a4 - b4;
    register s32 a8 __asm__("r2") = *(s32 *)(ra + 8);
    register s32 b8 __asm__("r1") = *(s32 *)(rb + 8);
    d5 = a8 - b8;
    __asm__("" : "+r"(d4), "+r"(d5));
    s32 res = (s32)*(s16 *)(tmp + 20) * d5;
    res -= (s32)*(s16 *)(tmp + 22) * d4; // muls as in asm, s32
    if (res > 0) return 0;
    return 1;
}
int _080019200(void *a, void *b) __attribute__((alias("Race_Setup_19200")));
// The ROM's 80-byte span has a zero halfword after the 78-byte return sequence.
__asm__(".pushsection .text.Race_Setup_19200,\"ax\",%progbits\n\t.short 0\n\t.popsection");

// _080019250 — 200 B (asm/race_setup_18adc.s:943-1041). Delta-angle helper:
// d = s16[a+22] - s16[b+22]; if the work-area flag at 0x03001780+0x1111 is
// set, push a clamped `-d << 3` (0..36, passed as s16) into a+36; then the
// race context's +130 latch decides, and the d == 0 tie case runs the
// 0x08006C10 projection to derive t = (s16[p+20]*dy - s16[p+22]*dx) >> 16 and
// returns 1 for t < 0 outside the +-40 band, else gates on the +0x10C3 flag.
//
// OPEN (45/200): the ROM's first field read is `movs r0,#22 / ldrsh r1,[r7,r0]`
// (base = the PINNED record, offset in a register), but agbcc folds the address
// into the argument copy: `adds r0,#22 / movs r3,#0 / ldrsh r2,[r0,r3]`, which
// shifts every following instruction by one. The index form needs the read's
// base to have no equivalent argument pseudo (see Idle_Mode4Handler in
// src/idle_dispatch.c, which is EXACT and shows both spellings side by side).
// Measured and rejected here: pinned `s32 off asm("r0")=22`, pinned off2=r3,
// `s16 *` pointer variables, `(s16 *)a)[11]`, `u8 *` temporaries, `+r`
// barriers (add an `adds r0,r7,#0` copy) and `"r0"` clobber barriers -- all
// 45/200. Pinning the sum instead measures 195/196 bytes, i.e. worse.
//
// Both work-area addresses are TWO pool words in the ROM
// (`ldr r0,=0x03001780 / ldr r1,=0x00001111 / adds r0,r0,r1`), so they must be
// built from two anchor SYMBOLS -- the folded `0x03001780 + 0x1111` emits one
// word. The `.globl` assignments have to be emitted inside the function
// (match_c_slice splices only this body's section); same rule as WA_SPLIT_DECL
// in src/race_scene_b1.c.
#define RS_WA_DECL                                                            \
    __asm__(".globl RS_SetupWA\nRS_SetupWA = 0x03001780\n"                    \
            ".globl RS_Off1111\nRS_Off1111 = 0x00001111\n"                    \
            ".globl RS_Off10C3\nRS_Off10C3 = 0x000010C3\n")
#define RS_WA(o) ((uintptr_t)(o) + (uintptr_t)&RS_SetupWA)
extern u8 RS_SetupWA;
extern u8 RS_Off1111;
extern u8 RS_Off10C3;
extern int Sub_0802A8F0(void *a, int v);
extern void *_08006C10(void *state, int d);
int Race_Setup_19250(void *a_, void *b_) {
    RS_WA_DECL;
    // r7 = a, r8 = b: the ROM's `mov r7,r8 / push {r7}` prologue only appears
    // when r8 is live across the body.
    register void *a __asm__("r7") = a_;
    register void *b __asm__("r8") = b_;
    // NON-volatile halfword reads: a volatile s16 dereference makes agbcc emit
    // `ldrh` plus a `lsls #24 / asrs #24` sign-extend; the ROM has `ldrsh`.
    s16 d22a = *(s16 *)((u8 *)a + 22);
    s16 d22b = *(s16 *)((u8 *)b + 22);
    int d = (int)d22a - (int)d22b;
    s8 flag = *(volatile s8 *)(RS_WA(&RS_Off1111)); // ldrb + lsls #24 asrs #24
    if (flag != 0) {
        int neg = -d;
        int v = neg << 3; // lsls #3
        if (v > 36) v = 36;
        if (v < 0) v = 0;
        Sub_0802A8F0((volatile u8 *)a + 36, (s16)v);
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
    // d == 0 tie case: project through 0x08006C10(racectx + 0x4FC, 0).
    {
        u8 *rec = (u8 *)_08006C10((void *)(racectx + 0x4FC), 0);
        int dx = *(s32 *)a - *(s32 *)b;
        int dy = *(s32 *)((u8 *)a + 4) - *(s32 *)((u8 *)b + 4);
        int t = (*(s16 *)(rec + 20) * dy - *(s16 *)(rec + 22) * dx) >> 16;
        if ((u32)(t + 40) > 79u) {
            if (t < 0) return 1;
            return 0;
        }
        if (*(volatile u8 *)RS_WA(&RS_Off10C3) == 0)
            return 0;
        return 1;
    }
}
int _080019250(void *a, void *b) __attribute__((alias("Race_Setup_19250")));

// _080018F14: sorted insert at 0x03000598 by word at +204 vs 0x03005E48 head
// Pools: 0x03005E48 Vu32 gv, 0x03000598 Vu32 arr[3], widths ldr/str Vu32, ble/bge signed
void Race_Setup_18F14(void *a) {
    register void *record __asm__("r5") = a;
    register volatile u32 *global __asm__("r1") =
        (volatile u32 *)0x03005E48;
    register volatile u32 *keyp __asm__("r0") =
        (volatile u32 *)((volatile u8 *)a + 204);
    register u32 limit __asm__("r2") = *global;
    register u32 key __asm__("r0") = *keyp;
    volatile u32 *global_tail = global;
    if ((s32)limit <= (s32)key) return;

    register s32 index __asm__("r4") = 0;
    register volatile u32 *firstp __asm__("r0") =
        (volatile u32 *)0x03000598;
    register u32 first __asm__("r1") = *firstp;
    register volatile u32 *records __asm__("r6") = firstp;
    if (first == 0) {
        *records = (u32)(uintptr_t)record;
        goto tail;
    }
compare_records:
    {
        __asm__ volatile ("" : "+r"(record) : : "memory");
        register volatile u32 *recordKeyp __asm__("r1") =
            (volatile u32 *)((volatile u8 *)record + 204);
        register s32 offset __asm__("r2") = index * 4;
        register volatile u32 *slotp __asm__("r0");
        __asm__ volatile ("add r0, r2, r6" : "=r"(slotp)
                          : "r"(offset), "r"(records) : "memory");
        u32 slot = *slotp;
        register volatile u32 *slotKeyp __asm__("r0") =
            (volatile u32 *)((volatile u8 *)(uintptr_t)slot + 204);
        u32 currentKey = *recordKeyp;
        u32 slotKey = *slotKeyp;
        if ((s32)currentKey < (s32)slotKey) {
            register s32 move __asm__("r3") = 1;
            if (move >= index) {
                register volatile u32 *shift __asm__("r1") = records + 1;
                do {
                    shift[1] = shift[0];
                    shift--;
                    move--;
                } while (move >= index);
            }
            __asm__ volatile ("add r0, r2, r6\n\tstr r5, [r0]"
                              : : : "r0", "memory");
            goto tail;
        }
    }
    index++;
    if (index > 2) goto tail;
    {
        register volatile u32 *nextp __asm__("r1");
        __asm__ volatile ("lsl r0, r4, #2\n\tadd r1, r0, r6"
                          : "=r"(nextp) : "r"(index), "r"(records)
                          : "r0", "memory");
        u32 next = *nextp;
        if (next != 0) goto compare_records;
        *nextp = (u32)(uintptr_t)record;
        goto tail;
    }
tail:
    {
        u32 last = records[2];
        if (last != 0) {
            u32 lastKey = *(volatile u32 *)((volatile u8 *)(uintptr_t)last + 204);
            *global_tail = lastKey;
        }
    }
}
__asm__(".pushsection .text.Race_Setup_18F14,\"ax\",%progbits\n\t.short 0\n\t.popsection");
void _080018F14(void *a) __attribute__((alias("Race_Setup_18F14")));
void sub_080018F14(void *a) __attribute__((alias("Race_Setup_18F14")));

// All seven formerly unowned typed entries in race_setup_18adc.s now have
// substantive C bodies above: 0x080019CBC, 0x080019CF4, 0x080019D30,
// 0x080019DDC, 0x080019E14, 0x080019E80, and 0x080019F1C.

// ROM entry alias.
void RaceScene_Leaf_5B8(void) __attribute__((alias("Race_Setup_195B8")));
void RaceScene_Leaf_6E8(void) __attribute__((alias("Race_Setup_195F8")));
void _08018ADC(void) __attribute__((alias("Race_Setup_18ADC")));
void _080191BC(u32 a0) __attribute__((alias("Race_Setup_191BC")));
int _08019200(void *a, void *b) __attribute__((alias("Race_Setup_19200")));

// ----------------------------------------------------------------------------
// 0x0802A8F0 — 4B draft/proximity trim setter (asm/garage_26f50.s, tucked
// behind 0x0802A8EC's epilogue): *(u16*)(rec+148) = v; return 0. The sole
// writer of the +148 trim consumed by sub_080029A74 (car_dynamics.md §9).
int RaceDraftTrim_2A8F0(void *rec, int v) {
    *(volatile u16 *)((volatile u8 *)rec + 148) = (u16)v;
    return 0;
}
int _0802A8F0(void *a, int v) __attribute__((alias("RaceDraftTrim_2A8F0")));
int sub_0802A8F0(void *a, int v) __attribute__((alias("RaceDraftTrim_2A8F0")));
int Sub_0802A8F0(void *a, int v) __attribute__((alias("RaceDraftTrim_2A8F0")));
