// ============================================================================
// menu_ff78_p.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External calls use symbols provided by the asm closure or aliases at the
// same ROM VMA.
//
//   sub_080015620 (0x080015620) — (rec, dead, ev) event handler.
//   sub_08001573C (0x08001573C) — (rec, dead, ev) event handler.
// ============================================================================

#include "gba/types.h"

/* Preserve the literal WA base-plus-offset accesses and scratch registers. */
#define MENU_FF78_WA_STORE_R3(src) do { \
    register u32 wa_base asm("r0") = 0x03001780u; \
    register u32 wa_value asm("r1") = *(volatile u16 *)(uintptr_t)(src); \
    register u32 wa_offset asm("r3") = 0x107Cu; \
    register u32 wa_addr asm("r0"); \
    __asm__ volatile("" : "+r"(wa_base), "+r"(wa_value), "+r"(wa_offset) \
                     : : "memory"); \
    wa_addr = wa_base + wa_offset; \
    __asm__ volatile("" : "+r"(wa_addr) : : "cc"); \
    __asm__ volatile("strh %1, [%0]" : : "l"(wa_addr), "l"(wa_value) \
                     : "memory"); \
} while (0)
#define MENU_FF78_WA_STORE_R2(src) do { \
    register u32 wa_base asm("r0") = 0x03001780u; \
    register u32 wa_value asm("r1") = *(volatile u16 *)(uintptr_t)(src); \
    register u32 wa_offset asm("r2") = 0x107Cu; \
    register u32 wa_addr asm("r0"); \
    __asm__ volatile("" : "+r"(wa_base), "+r"(wa_value), "+r"(wa_offset) \
                     : : "memory"); \
    wa_addr = wa_base + wa_offset; \
    __asm__ volatile("" : "+r"(wa_addr) : : "cc"); \
    __asm__ volatile("strh %1, [%0]" : : "l"(wa_addr), "l"(wa_value) \
                     : "memory"); \
} while (0)
#define MENU_FF78_WA_STORE_FROM_REC(rec) do { \
    register u32 wa_base asm("r0"); \
    register u32 wa_value asm("r1"); \
    register u32 wa_offset asm("r3"); \
    register u32 wa_addr asm("r0"); \
    register volatile u8 *wa_src asm("r2"); \
    wa_base = 0x03001780u; \
    __asm__ volatile("" : "+r"(wa_base)); \
    __asm__ volatile("add r2, %1, #0\n\tadd r2, #144" \
                     : "=r"(wa_src) : "l"(rec) : "cc"); \
    wa_value = *(volatile u16 *)(uintptr_t)wa_src; \
    wa_offset = 0x107Cu; \
    __asm__ volatile("" : "+r"(wa_base), "+r"(wa_value), "+r"(wa_offset) \
                     : : "memory"); \
    wa_addr = wa_base + wa_offset; \
    __asm__ volatile("" : "+r"(wa_addr) : : "cc"); \
    __asm__ volatile("strh %1, [%0]" : : "l"(wa_addr), "l"(wa_value) \
                     : "memory"); \
    __asm__ volatile("add r6, r2, #0" : : : "cc"); \
} while (0)

extern void Sub_08002B368(u32 v);
extern void _0802B368(u32 v);
extern int Sub_08024068(void);
extern void Sub_08001549C(void *a);
extern void _08001549C(void *a);
extern void Sub_08002158(int cmd, u32 val);
extern int Sub_08002178(u32 a);
extern void Sub_08002618(u32 a, u32 b);

// ----------------------------------------------------------------------------
// sub_080015620 — (rec, dead, ev): r6 = rec+144, entry = u16[rec+144].
//   ev==2: 1549C(rec).
//   ev==1 on s = s16[rec+144]:
//     s==0: s16[rec+24]=ev; 2B368(1); u32[16]=10; s16[20]=0; u32[48]=10;
//       u32[44]=ev; u16[WA+0x107C]=u16[rec+144].
//     s==1: u16[WA+0x107C]=u16[rec+144]; 1549C(rec).
//     s in 2..4: v=24068; v==16 stores 5 and v==39 stores 1 to s16[24];
//       other v values leave s16[24] unchanged. Then 2B368(1), u32[16]=10;
//       s16[20]=0; u32[48]=10;
//       u32[44]=1; u16[WA+0x107C]=u16[rec+144].
//   tail on cell u16[r6]: ev==64: cell--, then if ==2: cell=1;
//   ev==128: cell++, then if ==2: cell=3.
//   clamp s16 0..4; changed (u16) vs entry -> 2B368(2).
void MenuFF78_15620(void *rec_, u32 dead, u32 ev_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 ev = (u16)ev_;
    register volatile u8 *cell asm("r5") =
        (volatile u8 *)(uintptr_t)(rec + 144);
    u16 entry = *(volatile u16 *)(uintptr_t)cell;
    if (ev == 2)
        _08001549C(rec_);
    register volatile u8 *tail_cell asm("r6");
    __asm__ volatile("add r6, r5, #0"
                     : "=r"(tail_cell) : "r"(cell) : "cc");
    if (ev == 1) {
        register s32 s asm("r5");
        __asm__ volatile("movs r2, #0\n\tldrsh %0, [%1, r2]"
                         : "=l"(s) : "l"(tail_cell)
                         : "r2", "cc", "memory");
        register int rv asm("r0");
        switch (s) {
        case 0:
            *(volatile s16 *)(uintptr_t)(rec + 24) = (s16)ev;
            _0802B368(1);
            *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
            *(volatile s16 *)(uintptr_t)(rec + 20) = (s16)s;
            *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
            *(volatile u32 *)(uintptr_t)(rec + 44) = (u32)ev;
            MENU_FF78_WA_STORE_R3(tail_cell);
            break;
        case 1:
            MENU_FF78_WA_STORE_R2(tail_cell);
            _08001549C(rec_);
            break;
        case 2:
        case 3:
        case 4: {
            rv = Sub_08024068();
            if (rv == 16)
                goto map_five;
            if (rv != 39)
                goto state_common;
            goto map_one;
        }
        default:
            break;
        }
        goto state_done;
map_one:
        rv = 1;
        goto map_store;
map_five:
        rv = 5;
map_store:
        *(volatile s16 *)(uintptr_t)(rec + 24) = (s16)rv;
state_common:
            _0802B368(1);
            *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
            *(volatile s16 *)(uintptr_t)(rec + 20) = 0;
            *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
            *(volatile u32 *)(uintptr_t)(rec + 44) = 1;
            MENU_FF78_WA_STORE_FROM_REC(rec);
state_done:;
    }
    if (ev == 64) {
        register s32 c asm("r0");
        __asm__ volatile("ldrh %0, [%1]\n\tsub %0, %0, #1\n\t"
                         "strh %0, [%1]\n\tlsl %0, %0, #16\n\t"
                         "asr %0, %0, #16"
                         : "=&l"(c) : "l"(tail_cell) : "cc", "memory");
        if (c == 2)
            *(volatile s16 *)(uintptr_t)tail_cell = 1;
    }
    if (ev == 128) {
        register s32 c asm("r0");
        __asm__ volatile("ldrh %0, [%1]\n\tadd %0, %0, #1\n\t"
                         "strh %0, [%1]\n\tlsl %0, %0, #16\n\t"
                         "asr %0, %0, #16"
                         : "=&l"(c) : "l"(tail_cell) : "cc", "memory");
        if (c == 2)
            *(volatile s16 *)(uintptr_t)tail_cell = 3;
    }
    register volatile u8 *r1 asm("r1");
    __asm__ volatile("add r1, r6, #0"
                     : "=r"(r1) : "r"(tail_cell) : "cc");
    register s32 cur asm("r0");
    __asm__ volatile("movs r2, #0\n\tldrsh %0, [%1, r2]"
                     : "=l"(cur) : "l"(r1) : "r2", "cc", "memory");
    if (cur < 0)
        *(volatile u16 *)(uintptr_t)r1 = 0;
    __asm__ volatile("movs r3, #0\n\tldrsh %0, [%1, r3]"
                     : "=l"(cur) : "l"(r1) : "r3", "cc", "memory");
    if (cur > 4)
        *(volatile u16 *)(uintptr_t)r1 = 4;
    register u32 current asm("r6");
    __asm__ volatile("ldrh r6, [r6]" : "=r"(current) : : "memory");
    if (entry != current)
        _0802B368(2);
}
#undef MENU_FF78_WA_STORE_R3
#undef MENU_FF78_WA_STORE_R2
#undef MENU_FF78_WA_STORE_FROM_REC
void _080015620(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_15620")));
void Sub_080015620(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_15620")));
void sub_080015620(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_15620")));

// ----------------------------------------------------------------------------
// sub_08001573C — (rec, dead, ev): entry = u16[rec+144].
//   Head: if u8[WA+0x10C3]==1: 02158(u16[150],4); 02158(u16[146],2);
//     02158(u16[146],3); else: 02158(u16[148],4); 02158(u16[144],2);
//     02158(u16[144],3). r1 = rec+146, sl = rec+148 either way.
//   ev==1: s16[rec+150]=1. ev==2: s16[rec+150]=2.
//   ev==64: u16[r1]--. ev==128: u16[r1]++. Clamp s16[r1] 0..4.
//   Gate = u8[WA+0x10C3]:
//     0: 02178(1,2)->s16[rec+144]; 02178(1,4)->s16[sl].
//     nonzero: 02178(0,2)->s16[rec+144]; 02178(0,4)->s16[sl].
//   If (u16)02178(gate,4) != (u16)02178(gate^1,4):
//     (recompute r4 = rec+144, r9 = u16[rec+144].)
//     slv = s16[sl];
//     slv==1: s16[rec+150]=0; 02618(1,0); s16[rec+136]=0;
//       q = s16[rec+144];
//       q==1: u16[WA+0x107C]=u16[rec+144]; 1549C(rec).
//       q==0: s16[rec+24]=slv; 2B368(1); u32[16]=10; s16[20]=0;
//         u32[48]=10; u32[44]=slv; u16[WA+0x107C]=u16[rec+144].
//       q in 2..4: 2B368(1); u32[16]=10; s16[20]=0 (then same tail:
//         u32[48]=10; u32[44]=slv; u16[WA+0x107C]=u16[rec+144]).
//     slv==2: s16[rec+150]=0; 02618(1,0); s16[rec+136]=0; 1549C(rec).
void MenuFF78_1573C(void *rec_, u32 dead, u32 ev_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    u16 ev = (u16)ev_;
    u16 entry = *(volatile u16 *)(uintptr_t)(rec + 144);
    u8 gate = *(volatile u8 *)(uintptr_t)(wa + 0x10C3u);
    if (gate == 1) {
        Sub_08002158(4, *(volatile u16 *)(uintptr_t)(rec + 150));
        Sub_08002158(2, *(volatile u16 *)(uintptr_t)(rec + 146));
        Sub_08002158(3, *(volatile u16 *)(uintptr_t)(rec + 146));
    } else {
        Sub_08002158(4, *(volatile u16 *)(uintptr_t)(rec + 148));
        Sub_08002158(2, *(volatile u16 *)(uintptr_t)(rec + 144));
        Sub_08002158(3, *(volatile u16 *)(uintptr_t)(rec + 144));
    }
    volatile u8 *r1 = (volatile u8 *)(uintptr_t)(rec + 146);
    volatile u8 *sl = (volatile u8 *)(uintptr_t)(rec + 148);
    if (ev == 1)
        *(volatile s16 *)(uintptr_t)(rec + 150) = 1;
    if (ev == 2)
        *(volatile s16 *)(uintptr_t)(rec + 150) = 2;
    if (ev == 64) {
        u16 c = *(volatile u16 *)(uintptr_t)r1;
        *(volatile u16 *)(uintptr_t)r1 = (u16)(c - 1);
    }
    if (ev == 128) {
        u16 c = *(volatile u16 *)(uintptr_t)r1;
        *(volatile u16 *)(uintptr_t)r1 = (u16)(c + 1);
    }
    if (*(volatile s16 *)(uintptr_t)r1 < 0)
        *(volatile s16 *)(uintptr_t)r1 = 0;
    if (*(volatile s16 *)(uintptr_t)r1 > 4)
        *(volatile s16 *)(uintptr_t)r1 = 4;
    {
        int g = (gate == 0) ? 1 : 0;
        int v0 = Sub_08002178((u32)g);
        *(volatile s16 *)(uintptr_t)(rec + 144) = (s16)(u16)v0;
        int v1 = Sub_08002178((u32)g);
        *(volatile s16 *)(uintptr_t)sl = (s16)(u16)v1;
        if (entry != *(volatile u16 *)(uintptr_t)(rec + 144))
            Sub_08002B368(2);
        int w0 = Sub_08002178(0);
        int w1 = Sub_08002178(1);
        if ((u16)w0 != (u16)w1) {
            s16 slv = *(volatile s16 *)(uintptr_t)sl;
            if (slv == 1) {
                *(volatile s16 *)(uintptr_t)(rec + 150) = 0;
                Sub_08002618(1, 0);
                *(volatile s16 *)(uintptr_t)(rec + 136) = 0;
                s16 q = *(volatile s16 *)(uintptr_t)(rec + 144);
                if (q == 1) {
                    *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                        *(volatile u16 *)(uintptr_t)(rec + 144);
                    Sub_08001549C(rec_);
                } else if (q == 0) {
                    *(volatile s16 *)(uintptr_t)(rec + 24) = slv;
                    Sub_08002B368(1);
                    *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
                    *(volatile s16 *)(uintptr_t)(rec + 20) = 0;
                    *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
                    *(volatile u32 *)(uintptr_t)(rec + 44) = (u32)(u16)slv;
                    *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                        *(volatile u16 *)(uintptr_t)(rec + 144);
                } else if (q >= 2 && q <= 4) {
                    Sub_08002B368(1);
                    *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
                    *(volatile s16 *)(uintptr_t)(rec + 20) = 0;
                    *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
                    *(volatile u32 *)(uintptr_t)(rec + 44) = (u32)(u16)slv;
                    *(volatile u16 *)(uintptr_t)(wa + 0x107Cu) =
                        *(volatile u16 *)(uintptr_t)(rec + 144);
                }
            } else if (slv == 2) {
                *(volatile s16 *)(uintptr_t)(rec + 150) = 0;
                Sub_08002618(1, 0);
                *(volatile s16 *)(uintptr_t)(rec + 136) = 0;
                Sub_08001549C(rec_);
            }
        }
    }
}
void _08001573C(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_1573C")));
void Sub_08001573C(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_1573C")));
void sub_08001573C(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_1573C")));
