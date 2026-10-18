// ============================================================================
// menu_ff78_o.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use closure symbols or VMA aliases matching the ROM calls.
//
//   sub_080015304 (0x080015304) — (rec) WA+0xFBC 8-way setup + tail.
//   sub_0800154E0 (0x0800154E0) — (rec, dead, ev) event handler.
// ============================================================================

#include "gba/types.h"

extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
extern void Sub_0800DAB8(void *a);
extern void sub_08007614(void *a, int b, int c, int d);
extern void sub_0800798C(void *a, void *b);
extern void sub_08007A58(void *a);
extern void sub_080075E8(void *a, int b, int c);
extern void sub_08007ABC(u32 a, u32 b, u32 c);
extern void sub_0800D77C(void *a, int b, int c);
extern int _08002124(u32 v);
extern int Sub_08024068(void);
extern void Sub_08001549C(void *a);
extern void _08001549C(void *a);
extern void sub_080025BC8(void *a, int b);
extern void sub_0800152A8(void *a);
extern void _0802B368(u32 v);

/* Preserve the ROM's literal WA base/offset loads and scratch registers. */
#define MENU_FF78_O_WA_STORE_R3(src) do { \
    register u32 wa_base asm("r0") = 0x03001780u; \
    register u32 wa_value asm("r1") = *(volatile u16 *)(uintptr_t)(src); \
    register u32 wa_offset asm("r3") = 0x107Cu; \
    register u32 wa_addr asm("r0"); \
    __asm__ volatile("" : "+r"(wa_base), "+r"(wa_value), "+r"(wa_offset) \
                     : : "memory"); \
    wa_addr = wa_base + wa_offset; \
    __asm__ volatile("" : "+r"(wa_addr) : : "cc"); \
    *(volatile u16 *)(uintptr_t)wa_addr = (u16)wa_value; \
} while (0)
#define MENU_FF78_O_WA_STORE_R2(src) do { \
    register u32 wa_base asm("r0") = 0x03001780u; \
    register u32 wa_value asm("r1") = *(volatile u16 *)(uintptr_t)(src); \
    register u32 wa_offset asm("r2") = 0x107Cu; \
    register u32 wa_addr asm("r0"); \
    __asm__ volatile("" : "+r"(wa_base), "+r"(wa_value), "+r"(wa_offset) \
                     : : "memory"); \
    wa_addr = wa_base + wa_offset; \
    __asm__ volatile("" : "+r"(wa_addr) : : "cc"); \
    *(volatile u16 *)(uintptr_t)wa_addr = (u16)wa_value; \
} while (0)
#define MENU_FF78_O_WA_STORE_FROM_REC(rec) do { \
    register u32 wa_base asm("r0"); \
    register u32 wa_value asm("r1"); \
    register u32 wa_offset asm("r3"); \
    register u32 wa_addr asm("r0"); \
    register volatile u8 *wa_src asm("r2"); \
    wa_base = 0x03001780u; \
    __asm__ volatile("" : "+r"(wa_base)); \
    wa_src = (rec) + 144; \
    __asm__ volatile("" : "+r"(wa_src) : : "cc"); \
    wa_value = *(volatile u16 *)(uintptr_t)wa_src; \
    wa_offset = 0x107Cu; \
    __asm__ volatile("" : "+r"(wa_base), "+r"(wa_value), "+r"(wa_offset) \
                     : : "memory"); \
    wa_addr = wa_base + wa_offset; \
    __asm__ volatile("" : "+r"(wa_addr) : : "cc"); \
    *(volatile u16 *)(uintptr_t)wa_addr = (u16)wa_value; \
    tail_cell = wa_src; \
    __asm__ volatile("" : "+r"(tail_cell) : : "cc"); \
} while (0)

// ----------------------------------------------------------------------------
// sub_080015304 — (rec): head = 8-way on s16[WA+0xFBC]:
//   0: 07770(0,tmpl,1,0,4,1), s16[rec+138]=2.
//   1,2,5,7: 07770(0,tmpl,0,0,4,1), s16[rec+138]=0.
//   3: 07770(0,tmpl,0,0,4,1), s16[rec+138]=1.
//   4,6: nothing. (tmpl = 0x082FA8E4.)
// Tail (always):
//   u32[rec+88]=6; u32[rec+100]=12; u32[rec+112]=0; DAB8(rec+16);
//   tile=0x082A798C: 7614(tile,1,0,3); 7614(tile,1,1,4);
//   798C(tmpl,rec); 7A58(rec); 798C(0x082B7410,rec+8); 75E8(tm2,0,5);
//   25BC8(rec+152,5);
//   7ABC(u32[rec+12],u32[rec+156],u32[rec+152]);
//   u32[rec+128]=rec+164; u32[rec+132]=rec+16; u32[rec+16]=0;
//   s16[rec+20]=1; D77C(rec+36,0,-32); D77C(rec+28,0,160);
//   s16[rec+24]=5; s16[rec+26]=6; u32[rec+164]=2;
//   s16[rec+148]=0; s16[rec+150]=0; 02124(0x1392);
//   if s16[rec+138]==1: 152A8(rec).
void MenuFF78_15304(void *rec_) {
    register volatile u8 *rec __asm__("r7") = (volatile u8 *)rec_;
    register u32 sixlo __asm__("r0");
    register u32 sixhi __asm__("r8");
    {
        extern u8 Menu15304WaBase;
        extern u8 Menu15304WaOffset;
        __asm__(".globl Menu15304WaBase\nMenu15304WaBase = 0x03001780\n"
                ".globl Menu15304WaOffset\nMenu15304WaOffset = 0x00000FBC\n");
        s16 *gate = (s16 *)(uintptr_t)((uintptr_t)&Menu15304WaBase +
                                       (uintptr_t)&Menu15304WaOffset);
        int v = gate[0];
        if ((u32)v <= 7u) {
            // Match the ROM table order: 0, 1/2/5/7, 3, then 4/6.
            switch (v) {
            case 0:
                _08007770(0, (void *)(uintptr_t)0x082FA8E4u, 1, 0, 4, 1);
                *(volatile s16 *)(uintptr_t)(rec + 138) = 2;
                break;
            case 1: case 2: case 5: case 7:
                _08007770(0, (void *)(uintptr_t)0x082FA8E4u, 0, 0, 4, 1);
                *(volatile s16 *)(uintptr_t)(rec + 138) = 0;
                break;
            case 3:
                _08007770(0, (void *)(uintptr_t)0x082FA8E4u, 0, 0, 4, 1);
                *(volatile s16 *)(uintptr_t)(rec + 138) = 1;
                break;
            case 4: case 6:
                break;
            default:
                break;
            }
        }
    }
    // The ROM stores a word here and keeps the same value in r8 for +26.
    sixlo = 6;
    sixhi = sixlo;
    __asm__ volatile("" : "+r"(sixlo), "+r"(sixhi));
    *(volatile u32 *)(uintptr_t)(rec + 88) = sixlo;
    *(volatile u32 *)(uintptr_t)(rec + 100) = 12;
    *(volatile u32 *)(uintptr_t)(rec + 112) = 0;
    Sub_0800DAB8((void *)(uintptr_t)(rec + 16));
    {
        register void *tile __asm__("r4") = (void *)(uintptr_t)0x082A798Cu;
        __asm__ volatile("" : "+r"(tile));
        sub_08007614(tile, 1, 0, 3);
        sub_08007614(tile, 1, 1, 4);
    }
    sub_0800798C((void *)(uintptr_t)0x082FA8E4u, rec_);
    sub_08007A58(rec_);
    {
        void *tm2 = (void *)(uintptr_t)0x082B7410u;
        sub_0800798C(tm2, (void *)(uintptr_t)(rec + 8));
        sub_080075E8(tm2, 0, 5);
    }
    sub_080025BC8((void *)(uintptr_t)(rec + 152), 5);
    sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 12),
                 *(volatile u32 *)(uintptr_t)(rec + 156),
                 *(volatile u32 *)(uintptr_t)(rec + 152));
    *(volatile u32 *)(uintptr_t)(rec + 128) = (u32)(uintptr_t)(rec + 164);
    *(volatile u32 *)(uintptr_t)(rec + 132) = (u32)(uintptr_t)(rec + 16);
    *(volatile u32 *)(uintptr_t)(rec + 16) = 0;
    // The ROM uses immediate r7-based STRH forms here; agbcc expands the
    // equivalent byte-offset volatile lvalues into pointer arithmetic.
    __asm__ volatile(".code 16\n\tmovs r0, #1\n\tstrh r0, [r7, #20]"
                     : : : "r0", "cc", "memory");
    sub_0800D77C((void *)(uintptr_t)(rec + 36), 0, -32);
    sub_0800D77C((void *)(uintptr_t)(rec + 28), 0, 160);
    __asm__ volatile(".code 16\n\tmovs r0, #5\n\tstrh r0, [r7, #24]"
                     : : : "r0", "cc", "memory");
    __asm__ volatile(".code 16\n\tmov r1, %0\n\tstrh r1, [r7, #26]"
                     : : "r"(sixhi) : "r1", "cc", "memory");
    *(volatile u32 *)(uintptr_t)(rec + 164) = 2;
    *(volatile s16 *)(uintptr_t)(rec + 148) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 150) = 0;
    _08002124(0x1392u);
    if (*(volatile s16 *)(uintptr_t)(rec + 138) == 1)
        sub_0800152A8(rec_);
}
void _080015304(void *a) __attribute__((alias("MenuFF78_15304")));
void Sub_080015304(void *a) __attribute__((alias("MenuFF78_15304")));
void sub_080015304(void *a) __attribute__((alias("MenuFF78_15304")));

// ----------------------------------------------------------------------------
// sub_0800154E0 — (rec, dead, ev): r6 = rec+144, entry = u16[rec+144].
//   ev==2: 1549C(rec).
//   ev==1 on s = s16[rec+144]:
//     s==1: u16[WA+0x107C]=u16[rec+144]; 1549C(rec).
//     s==0: s16[rec+24]=ev; 2B368(1); u32[16]=10; s16[20]=0; u32[48]=10;
//       u32[44]=ev; u16[WA+0x107C]=u16[rec+144].
//     s in 2..4:
//       s==4 && u16[WA+0xFBC]==7: 2B368(10).
//       else: v=24068; r0 = (v==39) ? 1 : (u16[WA+0xFBC]==1 ? 7 : 5);
//         s16[rec+24]=r0; 2B368(1); u32[16]=10; s16[20]=0; u32[48]=10;
//         u32[44]=1; u16[WA+0x107C]=u16[rec+144].
//   tail on cell u16[r6]: ev==64: cell--; ev==128: cell++;
//   clamp s16 0..4; changed vs entry -> 2B368(2).
void MenuFF78_154E0(void *rec_, u32 dead, u32 ev_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 ev = (u16)ev_;
    register volatile u8 *cell asm("r4") =
        (volatile u8 *)(uintptr_t)(rec + 144);
    u16 entry = *(volatile u16 *)(uintptr_t)cell;
    if (ev == 2)
        _08001549C(rec_);
    register volatile u8 *tail_cell asm("r6") = cell;
    __asm__ volatile("" : "+r"(tail_cell) : : "cc");
    if (ev == 1) {
        register s32 s asm("r4");
        __asm__ volatile("movs r1, #0\n\tldrsh %0, [%1, r1]"
                         : "=l"(s) : "l"(tail_cell)
                         : "r1", "cc", "memory");
        switch (s) {
        case 0:
            *(volatile s16 *)(uintptr_t)(rec + 24) = (s16)ev;
            _0802B368(1);
            *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
            *(volatile s16 *)(uintptr_t)(rec + 20) = (s16)s;
            *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
            *(volatile u32 *)(uintptr_t)(rec + 44) = (u32)ev;
            MENU_FF78_O_WA_STORE_R3(tail_cell);
            break;
        case 1:
            MENU_FF78_O_WA_STORE_R2(tail_cell);
            _08001549C(rec_);
            break;
        case 2:
        case 3:
        case 4: {
            register u32 current_state asm("r6") =
                *(volatile u16 *)(uintptr_t)tail_cell;
            __asm__ volatile("" : "+r"(current_state) : : "memory");
            if (current_state == 4) {
                register u32 wa_base asm("r0") = 0x03001780u;
                register u32 wa_offset asm("r2") = 0x0FBCu;
                register u32 wa_addr asm("r0");
                register u32 gate asm("r0");
                __asm__ volatile("" : "+r"(wa_base), "+r"(wa_offset)
                                 : : "memory");
                wa_addr = wa_base + wa_offset;
                __asm__ volatile("" : "+r"(wa_addr) : : "cc");
                gate = *(volatile u16 *)(uintptr_t)wa_addr;
                if (gate == 7) {
                    _0802B368(10);
                    tail_cell = rec + 144;
                    __asm__ volatile("" : "+r"(tail_cell) : : "cc");
                    break;
                }
            }
            register int rv asm("r0") = Sub_08024068();
            if (rv != 39)
                goto map_gate;
            rv = 1;
            goto map_store;
map_gate:
            {
                register u32 wa_base asm("r0") = 0x03001780u;
                register u32 wa_offset asm("r3") = 0x0FBCu;
                register u32 wa_addr asm("r0");
                register u32 gate asm("r0");
                __asm__ volatile("" : "+r"(wa_base), "+r"(wa_offset)
                                 : : "memory");
                wa_addr = wa_base + wa_offset;
                __asm__ volatile("" : "+r"(wa_addr) : : "cc");
                gate = *(volatile u16 *)(uintptr_t)wa_addr;
                if (gate != 1)
                    goto map_five;
            }
            rv = 7;
            goto map_store;
map_five:
            rv = 5;
map_store:
            *(volatile s16 *)(uintptr_t)(rec + 24) = (s16)rv;
            goto state_common;
        }
        default:
            break;
        }
        goto state_done;
state_common:
        _0802B368(1);
        *(volatile u32 *)(uintptr_t)(rec + 16) = 10;
        *(volatile s16 *)(uintptr_t)(rec + 20) = 0;
        *(volatile u32 *)(uintptr_t)(rec + 48) = 10;
        *(volatile u32 *)(uintptr_t)(rec + 44) = 1;
        MENU_FF78_O_WA_STORE_FROM_REC(rec);
state_done:;
    }
    if (ev == 64) {
        u16 c = *(volatile u16 *)(uintptr_t)tail_cell;
        *(volatile u16 *)(uintptr_t)tail_cell = (u16)(c - 1);
    }
    if (ev == 128) {
        u16 c = *(volatile u16 *)(uintptr_t)tail_cell;
        *(volatile u16 *)(uintptr_t)tail_cell = (u16)(c + 1);
    }
    register volatile u8 *r1 asm("r1") = tail_cell;
    __asm__ volatile("" : "+r"(r1) : : "cc");
    register s32 cur asm("r0");
    cur = *(s16 *)r1;
    if (cur < 0)
        *(volatile u16 *)(uintptr_t)r1 = 0;
    __asm__ volatile("movs r3, #0\n\tldrsh %0, [%1, r3]"
                     : "=l"(cur) : "l"(r1) : "r3", "cc", "memory");
    if (cur > 4)
        *(volatile u16 *)(uintptr_t)r1 = 4;
    register u32 current asm("r6") = *(volatile u16 *)(uintptr_t)tail_cell;
    __asm__ volatile("" : "+r"(current) : : "memory");
    if (entry != current)
        _0802B368(2);
}
#undef MENU_FF78_O_WA_STORE_R3
#undef MENU_FF78_O_WA_STORE_R2
#undef MENU_FF78_O_WA_STORE_FROM_REC
void _0800154E0(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_154E0")));
void Sub_0800154E0(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_154E0")));
void sub_0800154E0(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_154E0")));
