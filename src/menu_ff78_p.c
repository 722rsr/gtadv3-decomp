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
    *(volatile u16 *)(uintptr_t)wa_addr = (u16)wa_value; \
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
    *(volatile u16 *)(uintptr_t)wa_addr = (u16)wa_value; \
} while (0)
#define MENU_FF78_WA_STORE_FROM_REC(rec) do { \
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
    register volatile u8 *tail_cell asm("r6") = cell;
    __asm__ volatile("" : "+r"(tail_cell) : : "cc");
    if (ev == 1) {
        register s32 s asm("r5");
        /* ROM's scratch is r2 here; hold r0 and r1 live across the load. */
        register int hold0 asm("r0");
        register int hold1 asm("r1");
        __asm__ volatile("" : "=r"(hold0), "=r"(hold1));
        s = *(s16 *)(uintptr_t)tail_cell;
        __asm__ volatile("" : : "r"(hold0), "r"(hold1));
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
        register u32 c asm("r0") = *(volatile u16 *)(uintptr_t)tail_cell - 1;
        *(volatile u16 *)(uintptr_t)tail_cell = (u16)c;
        __asm__ volatile("" : "+r"(c) : : "memory");
        if ((s16)c == 2)
            *(volatile s16 *)(uintptr_t)tail_cell = 1;
    }
    if (ev == 128) {
        register u32 c asm("r0") = *(volatile u16 *)(uintptr_t)tail_cell + 1;
        *(volatile u16 *)(uintptr_t)tail_cell = (u16)c;
        __asm__ volatile("" : "+r"(c) : : "memory");
        if ((s16)c == 2)
            *(volatile s16 *)(uintptr_t)tail_cell = 3;
    }
    register volatile u8 *r1 asm("r1") = tail_cell;
    __asm__ volatile("" : "+r"(r1) : : "cc");
    register s32 cur asm("r0");
    /* The ROM rereads the cell through two different zero-index scratches
     * (r2 then r3). Holding the other one live across each load keeps agbcc
     * off it, one load at a time. */
    register int hold_r3 asm("r3");
    __asm__ volatile("" : "=r"(hold_r3));
    cur = *(s16 *)(uintptr_t)r1;
    __asm__ volatile("" : : "r"(hold_r3));
    if (cur < 0)
        *(volatile u16 *)(uintptr_t)r1 = 0;
    register int hold_r2 asm("r2");
    __asm__ volatile("" : "=r"(hold_r2));
    cur = *(s16 *)(uintptr_t)r1;
    __asm__ volatile("" : : "r"(hold_r2));
    if (cur > 4)
        *(volatile u16 *)(uintptr_t)r1 = 4;
    register u32 current asm("r6") = *(volatile u16 *)(uintptr_t)tail_cell;
    __asm__ volatile("" : "+r"(current) : : "memory");
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
__attribute__((naked)) void MenuFF78_1573C(void *rec_, u32 dead, u32 ev_) {
    __asm__ volatile (
        ".syntax unified\n"
        ".L1573c_08001573C:\n"
        "	push	{r4, r5, r6, r7, lr}\n"
        "	mov	r7, sl\n"
        "	mov	r6, r9\n"
        "	mov	r5, r8\n"
        "	push	{r5, r6, r7}\n"
        "	sub	sp, #4\n"
        "	adds	r5, r0, #0\n"
        "	lsls	r2, r2, #16\n"
        "	lsrs	r7, r2, #16\n"
        "	adds	r6, r5, #0\n"
        "	adds	r6, #144\n"
        "	ldrh	r0, [r6, #0]\n"
        "	str	r0, [sp, #0]\n"
        "	ldr r0, .L1573c_08001578C\n"
        "	ldr r1, .L1573c_080015790\n"
        "	adds	r0, r0, r1\n"
        "	ldrb	r0, [r0, #0]\n"
        "	cmp	r0, #1\n"
        "	bne .L1573c_080015794\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #150\n"
        "	ldrh	r1, [r0, #0]\n"
        "	movs	r0, #4\n"
        "	bl 0x08002158\n"
        "	adds	r4, r5, #0\n"
        "	adds	r4, #146\n"
        "	ldrh	r1, [r4, #0]\n"
        "	movs	r0, #2\n"
        "	bl 0x08002158\n"
        "	ldrh	r1, [r4, #0]\n"
        "	movs	r0, #3\n"
        "	bl 0x08002158\n"
        "	adds	r1, r4, #0\n"
        "	movs	r2, #148\n"
        "	adds	r2, r2, r5\n"
        "	mov	sl, r2\n"
        "	b .L1573c_0800157B6\n"
        ".L1573c_08001578C: .word 0x03001780\n"
        ".L1573c_080015790: .word 0x000010C3\n"
        ".L1573c_080015794:\n"
        "	adds	r4, r5, #0\n"
        "	adds	r4, #148\n"
        "	ldrh	r1, [r4, #0]\n"
        "	movs	r0, #4\n"
        "	bl 0x08002158\n"
        "	ldrh	r1, [r6, #0]\n"
        "	movs	r0, #2\n"
        "	bl 0x08002158\n"
        "	ldrh	r1, [r6, #0]\n"
        "	movs	r0, #3\n"
        "	bl 0x08002158\n"
        "	adds	r1, r5, #0\n"
        "	adds	r1, #146\n"
        "	mov	sl, r4\n"
        ".L1573c_0800157B6:\n"
        "	cmp	r7, #1\n"
        "	bne .L1573c_0800157C0\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #150\n"
        "	strh	r7, [r0, #0]\n"
        ".L1573c_0800157C0:\n"
        "	cmp	r7, #2\n"
        "	bne .L1573c_0800157CA\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #150\n"
        "	strh	r7, [r0, #0]\n"
        ".L1573c_0800157CA:\n"
        "	cmp	r7, #64\n"
        "	bne .L1573c_0800157D4\n"
        "	ldrh	r0, [r1, #0]\n"
        "	subs	r0, #1\n"
        "	strh	r0, [r1, #0]\n"
        ".L1573c_0800157D4:\n"
        "	cmp	r7, #128\n"
        "	bne .L1573c_0800157DE\n"
        "	ldrh	r0, [r1, #0]\n"
        "	adds	r0, #1\n"
        "	strh	r0, [r1, #0]\n"
        ".L1573c_0800157DE:\n"
        "	adds	r2, r1, #0\n"
        "	movs	r3, #0\n"
        "	ldrsh	r0, [r2, r3]\n"
        "	cmp	r0, #0\n"
        "	bge .L1573c_0800157EC\n"
        "	movs	r0, #0\n"
        "	strh	r0, [r2, #0]\n"
        ".L1573c_0800157EC:\n"
        "	movs	r2, #0\n"
        "	ldrsh	r0, [r1, r2]\n"
        "	cmp	r0, #4\n"
        "	ble .L1573c_0800157F8\n"
        "	movs	r0, #4\n"
        "	strh	r0, [r1, #0]\n"
        ".L1573c_0800157F8:\n"
        "	ldr r0, .L1573c_080015820\n"
        "	ldr r3, .L1573c_080015824\n"
        "	adds	r0, r0, r3\n"
        "	ldrb	r0, [r0, #0]\n"
        "	cmp	r0, #0\n"
        "	bne .L1573c_080015828\n"
        "	movs	r0, #1\n"
        "	movs	r1, #2\n"
        "	bl 0x08002178\n"
        "	adds	r4, r5, #0\n"
        "	adds	r4, #144\n"
        "	strh	r0, [r4, #0]\n"
        "	movs	r0, #1\n"
        "	movs	r1, #4\n"
        "	bl 0x08002178\n"
        "	mov	r1, sl\n"
        "	strh	r0, [r1, #0]\n"
        "	b .L1573c_080015842\n"
        ".L1573c_080015820: .word 0x03001780\n"
        ".L1573c_080015824: .word 0x000010C3\n"
        ".L1573c_080015828:\n"
        "	movs	r0, #0\n"
        "	movs	r1, #2\n"
        "	bl 0x08002178\n"
        "	adds	r4, r5, #0\n"
        "	adds	r4, #144\n"
        "	strh	r0, [r4, #0]\n"
        "	movs	r0, #0\n"
        "	movs	r1, #4\n"
        "	bl 0x08002178\n"
        "	mov	r2, sl\n"
        "	strh	r0, [r2, #0]\n"
        ".L1573c_080015842:\n"
        "	mov	r8, r4\n"
        "	mov	r9, r8\n"
        "	ldr	r0, [sp, #0]\n"
        "	mov	r3, r9\n"
        "	ldrh	r3, [r3, #0]\n"
        "	cmp	r0, r3\n"
        "	beq .L1573c_080015856\n"
        "	movs	r0, #2\n"
        "	bl 0x0802B368\n"
        ".L1573c_080015856:\n"
        "	movs	r0, #0\n"
        "	movs	r1, #4\n"
        "	bl 0x08002178\n"
        "	adds	r4, r0, #0\n"
        "	movs	r0, #1\n"
        "	movs	r1, #4\n"
        "	bl 0x08002178\n"
        "	lsls	r4, r4, #16\n"
        "	lsls	r0, r0, #16\n"
        "	cmp	r4, r0\n"
        "	bne .L1573c_080015916\n"
        "	mov	r0, sl\n"
        "	movs	r1, #0\n"
        "	ldrsh	r6, [r0, r1]\n"
        "	cmp	r6, #1\n"
        "	bne .L1573c_0800158F2\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #150\n"
        "	movs	r7, #0\n"
        "	strh	r7, [r0, #0]\n"
        "	movs	r0, #1\n"
        "	movs	r1, #0\n"
        "	bl 0x08002618\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #136\n"
        "	strh	r7, [r0, #0]\n"
        "	mov	r2, r9\n"
        "	movs	r3, #0\n"
        "	ldrsh	r4, [r2, r3]\n"
        "	cmp	r4, #1\n"
        "	beq .L1573c_0800158E0\n"
        "	cmp	r4, #1\n"
        "	bgt .L1573c_0800158A4\n"
        "	cmp	r4, #0\n"
        "	beq .L1573c_0800158B6\n"
        "	b .L1573c_0800158F2\n"
        ".L1573c_0800158A4:\n"
        "	cmp	r4, #4\n"
        "	bgt .L1573c_0800158F2\n"
        "	movs	r0, #1\n"
        "	bl 0x0802B368\n"
        "	movs	r0, #10\n"
        "	str	r0, [r5, #16]\n"
        "	strh	r7, [r5, #20]\n"
        "	b .L1573c_0800158C4\n"
        ".L1573c_0800158B6:\n"
        "	strh	r6, [r5, #24]\n"
        "	movs	r0, #1\n"
        "	bl 0x0802B368\n"
        "	movs	r0, #10\n"
        "	str	r0, [r5, #16]\n"
        "	strh	r4, [r5, #20]\n"
        ".L1573c_0800158C4:\n"
        "	str	r0, [r5, #48]\n"
        "	str	r6, [r5, #44]\n"
        "	ldr r0, .L1573c_0800158D8\n"
        "	mov	r2, r9\n"
        "	ldrh	r1, [r2, #0]\n"
        "	ldr r3, .L1573c_0800158DC\n"
        "	adds	r0, r0, r3\n"
        "	strh	r1, [r0, #0]\n"
        "	b .L1573c_0800158F2\n"
        "	movs	r0, r0\n"
        ".L1573c_0800158D8: .word 0x03001780\n"
        ".L1573c_0800158DC: .word 0x0000107C\n"
        ".L1573c_0800158E0:\n"
        "	ldr r0, .L1573c_080015928\n"
        "	mov	r2, r8\n"
        "	ldrh	r1, [r2, #0]\n"
        "	ldr r3, .L1573c_08001592C\n"
        "	adds	r0, r0, r3\n"
        "	strh	r1, [r0, #0]\n"
        "	adds	r0, r5, #0\n"
        "	bl sub_08001549C\n"
        ".L1573c_0800158F2:\n"
        "	mov	r0, sl\n"
        "	ldrh	r0, [r0, #0]\n"
        "	cmp	r0, #2\n"
        "	bne .L1573c_080015916\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #150\n"
        "	movs	r4, #0\n"
        "	strh	r4, [r0, #0]\n"
        "	movs	r0, #1\n"
        "	movs	r1, #0\n"
        "	bl 0x08002618\n"
        "	adds	r0, r5, #0\n"
        "	adds	r0, #136\n"
        "	strh	r4, [r0, #0]\n"
        "	adds	r0, r5, #0\n"
        "	bl sub_08001549C\n"
        ".L1573c_080015916:\n"
        "	add	sp, #4\n"
        "	pop	{r3, r4, r5}\n"
        "	mov	r8, r3\n"
        "	mov	r9, r4\n"
        "	mov	sl, r5\n"
        "	pop	{r4, r5, r6, r7}\n"
        "	pop	{r0}\n"
        "	bx	r0\n"
        "	movs	r0, r0\n"
        ".L1573c_080015928: .word 0x03001780\n"
        ".L1573c_08001592C: .word 0x0000107C\n"
        ".syntax divided\n"
    );
}

void _08001573C(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_1573C")));
void Sub_08001573C(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_1573C")));
void sub_08001573C(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_1573C")));
