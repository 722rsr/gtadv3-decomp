// ============================================================================
// menu_ff78_m.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080013850 (0x080013850) — (rec, dead, ev) event handler.
//   sub_080015BA0 (0x080015BA0) — (ev, a1, a2, rec) 12-way dispatcher.
// ============================================================================

#include "gba/types.h"

extern void Sub_08002B368(u32 v);
extern void Sub_080015230(void *a, void *b);
extern void Sub_0800D854(void *a);
extern void Sub_0800D8E4(void *a);
extern void Sub_080015B54(void *a);
extern void Sub_0800154E0(void *a, u32 b, u32 c);
extern void Sub_080015620(void *a, u32 b, u32 c);
extern void Sub_08001573C(void *a, u32 b, u32 c);
extern void Sub_080015A0C(void *a);
extern void Sub_080015AB0(void *a);
extern void Sub_080015304(void *a);
extern void Sub_08001529C(void *a); // 1-arg caller ABI (adds r0, r4, #0)

// ----------------------------------------------------------------------------
// sub_080013850 — (rec, dead, ev):
//   ev==1: 2B368(1); u32[rec+32]=10; s16[rec+36]=0; u32[rec+64]=10;
//     s = s16[rec+138];
//     s==0: u16[WA+0xFBE]=u16[rec+176]; u16[WA+0xFC2]=u16[rec+174];
//     s==1: u16[WA+0xFBE]=u16[rec+176]; u16[WA+0x574]=u16[rec+174];
//     (s==0: cell = WA+0xFC2; s==1: cell = WA+0x574)
//     i = s16[u16cell]; u8[WA+i*12+49] = (u8)u16[rec+172];
//     u8[WA+i*12+48] = (u8)u16[rec+178];
//     (s other: skip to tail.)
//     tail: s16[rec+40]=5; u32[rec+60]=1.
//   ev==2: 2B368(4); s16[rec+186]=0; u32[rec+144]=1.
//   ev==32: 2B368(3); if s16[rec+178]==1: s16[rec+178]=0.
//   ev==16: 2B368(3); if s16[rec+178]==0: s16[rec+178]=1.
__attribute__((naked)) void MenuFF78_13850(void *rec_, u32 dead, u32 ev_) {
    __asm__ volatile (
        ".syntax unified\n"
        ".L13850_080013850:\n"
        "	push	{r4, r5, lr}\n"
        "	adds	r4, r0, #0\n"
        "	lsls	r2, r2, #16\n"
        "	lsrs	r5, r2, #16\n"
        "	cmp	r5, #1\n"
        "	bne .L13850_0800138F2\n"
        "	movs	r0, #1\n"
        "	bl 0x0802B368\n"
        "	movs	r1, #10\n"
        "	str	r1, [r4, #32]\n"
        "	movs	r0, #0\n"
        "	strh	r0, [r4, #36]\n"
        "	str	r1, [r4, #64]\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #138\n"
        "	movs	r1, #0\n"
        "	ldrsh	r0, [r0, r1]\n"
        "	cmp	r0, #0\n"
        "	beq .L13850_08001387E\n"
        "	cmp	r0, #1\n"
        "	beq .L13850_0800138A4\n"
        "	b .L13850_0800138EA\n"
        ".L13850_08001387E:\n"
        "	ldr r3, .L13850_080013898\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #176\n"
        "	ldrh	r1, [r0, #0]\n"
        "	ldr r2, .L13850_08001389C\n"
        "	adds	r0, r3, r2\n"
        "	strh	r1, [r0, #0]\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #174\n"
        "	ldrh	r0, [r0, #0]\n"
        "	ldr r1, .L13850_0800138A0\n"
        "	b .L13850_0800138BA\n"
        "	movs	r0, r0\n"
        ".L13850_080013898: .word 0x03001780\n"
        ".L13850_08001389C: .word 0x00000FBE\n"
        ".L13850_0800138A0: .word 0x00000FC2\n"
        ".L13850_0800138A4:\n"
        "	ldr r3, .L13850_080013944\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #176\n"
        "	ldrh	r1, [r0, #0]\n"
        "	ldr r2, .L13850_080013948\n"
        "	adds	r0, r3, r2\n"
        "	strh	r1, [r0, #0]\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #174\n"
        "	ldrh	r0, [r0, #0]\n"
        "	ldr r1, .L13850_08001394C\n"
        ".L13850_0800138BA:\n"
        "	adds	r2, r3, r1\n"
        "	strh	r0, [r2, #0]\n"
        "	movs	r0, #0\n"
        "	ldrsh	r1, [r2, r0]\n"
        "	lsls	r0, r1, #1\n"
        "	adds	r0, r0, r1\n"
        "	lsls	r0, r0, #2\n"
        "	adds	r0, r0, r3\n"
        "	adds	r1, r4, #0\n"
        "	adds	r1, #172\n"
        "	ldrh	r1, [r1, #0]\n"
        "	adds	r0, #49\n"
        "	strb	r1, [r0, #0]\n"
        "	movs	r0, #0\n"
        "	ldrsh	r1, [r2, r0]\n"
        "	lsls	r0, r1, #1\n"
        "	adds	r0, r0, r1\n"
        "	lsls	r0, r0, #2\n"
        "	adds	r0, r0, r3\n"
        "	adds	r1, r4, #0\n"
        "	adds	r1, #178\n"
        "	ldrh	r1, [r1, #0]\n"
        "	adds	r0, #48\n"
        "	strb	r1, [r0, #0]\n"
        ".L13850_0800138EA:\n"
        "	movs	r0, #5\n"
        "	strh	r0, [r4, #40]\n"
        "	movs	r0, #1\n"
        "	str	r0, [r4, #60]\n"
        ".L13850_0800138F2:\n"
        "	cmp	r5, #2\n"
        "	bne .L13850_08001390A\n"
        "	movs	r0, #4\n"
        "	bl 0x0802B368\n"
        "	adds	r1, r4, #0\n"
        "	adds	r1, #186\n"
        "	movs	r0, #0\n"
        "	strh	r0, [r1, #0]\n"
        "	subs	r1, #42\n"
        "	movs	r0, #1\n"
        "	str	r0, [r1, #0]\n"
        ".L13850_08001390A:\n"
        "	cmp	r5, #32\n"
        "	bne .L13850_080013922\n"
        "	movs	r0, #3\n"
        "	bl 0x0802B368\n"
        "	adds	r1, r4, #0\n"
        "	adds	r1, #178\n"
        "	ldrh	r2, [r1, #0]\n"
        "	cmp	r2, #1\n"
        "	bne .L13850_080013922\n"
        "	movs	r0, #0\n"
        "	strh	r0, [r1, #0]\n"
        ".L13850_080013922:\n"
        "	cmp	r5, #16\n"
        "	bne .L13850_08001393C\n"
        "	movs	r0, #3\n"
        "	bl 0x0802B368\n"
        "	adds	r1, r4, #0\n"
        "	adds	r1, #178\n"
        "	movs	r2, #0\n"
        "	ldrsh	r0, [r1, r2]\n"
        "	cmp	r0, #0\n"
        "	bne .L13850_08001393C\n"
        "	movs	r0, #1\n"
        "	strh	r0, [r1, #0]\n"
        ".L13850_08001393C:\n"
        "	pop	{r4, r5}\n"
        "	pop	{r0}\n"
        "	bx	r0\n"
        "	movs	r0, r0\n"
        ".L13850_080013944: .word 0x03001780\n"
        ".L13850_080013948: .word 0x00000FBE\n"
        ".L13850_08001394C: .word 0x00000574\n"
        ".syntax divided\n"
    );
}

void _080013850(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_13850")));
void Sub_080013850(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_13850")));
void sub_080013850(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_13850")));

// ----------------------------------------------------------------------------
// sub_080015BA0 — (ev, a1, a2, rec): ev-1 indexes a 12-entry table.
//   0->15304(rec), 1->15230(rec,a1), 4->D854(rec+16)+D8E4(rec+120),
//   5->15B54(rec)+[u16[rec+20]!=0] on s16[rec+138]:
//     0->154E0, 1->1573C, 2->15620 (each (rec,(u16)a1,(u16)a2)),
//   6->on s16[rec+138]: 0/1->15A0C(rec), 2->15AB0(rec),
//   11->1529C(rec), else no-op.
void MenuFF78_15BA0(int ev, u32 a1, u32 a2, void *rec_) {
    u8 *r4 = (u8 *)rec_;
    switch (ev) {
    case 2:
        Sub_080015230(r4, (void *)(uintptr_t)a1);
        break;
    case 5:
        Sub_0800D854(r4 + 16);
        Sub_0800D8E4(r4 + 120);
        break;
    case 7:
        switch (*(s16 *)(r4 + 138)) {
        case 0:
        case 1:
            Sub_080015A0C(r4);
            break;
        case 2:
            Sub_080015AB0(r4);
            break;
        }
        break;
    case 6:
        Sub_080015B54(r4);
        if (*(volatile u16 *)(r4 + 20) != 0) {
            switch (*(s16 *)(r4 + 138)) {
            case 0:
                Sub_0800154E0(r4, (u16)a1, (u16)a2);
                break;
            case 2:
                Sub_080015620(r4, (u16)a1, (u16)a2);
                break;
            case 1:
                Sub_08001573C(r4, (u16)a1, (u16)a2);
                break;
            }
        }
        break;
    case 1:
        Sub_080015304(r4);
        break;
    case 12:
        Sub_08001529C(r4);
        break;
    }
}
__asm__(".align 2, 0");
void _080015BA0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_15BA0")));
void sub_080015BA0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_15BA0")));
