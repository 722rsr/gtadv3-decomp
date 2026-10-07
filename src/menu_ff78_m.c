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

#ifdef __APPLE__
__attribute__((weak)) void Sub_08002B368(u32 v) { (void)v; }
__attribute__((weak)) void Sub_080015230(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800D854(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800D8E4(void *a) { (void)a; }
__attribute__((weak)) void Sub_080015B54(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800154E0(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080015620(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_08001573C(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080015A0C(void *a) { (void)a; }
__attribute__((weak)) void Sub_080015AB0(void *a) { (void)a; }
__attribute__((weak)) void Sub_080015304(void *a) { (void)a; }
__attribute__((weak)) void Sub_08001529C(void) { }
#else
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
extern void Sub_08001529C(void); // 0-arg ABI (asm/menu_ff78.s:10450 reads no input)
#endif

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
void MenuFF78_13850(void *rec_, u32 dead, u32 ev_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    u16 ev = (u16)ev_;
    if (ev == 1) {
        Sub_08002B368(1);
        *(volatile u32 *)(uintptr_t)(rec + 32) = 10;
        *(volatile s16 *)(uintptr_t)(rec + 36) = 0;
        *(volatile u32 *)(uintptr_t)(rec + 64) = 10;
        s16 s = *(volatile s16 *)(uintptr_t)(rec + 138);
        if (s == 0 || s == 1) {
            u32 cell = (s == 0) ? 0xFC2u : 0x574u;
            *(volatile u16 *)(uintptr_t)(wa + 0xFBEu) =
                *(volatile u16 *)(uintptr_t)(rec + 176);
            *(volatile u16 *)(uintptr_t)(wa + cell) =
                *(volatile u16 *)(uintptr_t)(rec + 174);
            s16 i = *(volatile s16 *)(uintptr_t)(wa + cell);
            u32 off = (((u32)(s32)i << 1) + (u32)(s32)i) << 2; // *12
            *(volatile u8 *)(uintptr_t)(wa + off + 49) =
                (u8)*(volatile u16 *)(uintptr_t)(rec + 172);
            *(volatile u8 *)(uintptr_t)(wa + off + 48) =
                (u8)*(volatile u16 *)(uintptr_t)(rec + 178);
        }
        *(volatile s16 *)(uintptr_t)(rec + 40) = 5;
        *(volatile u32 *)(uintptr_t)(rec + 60) = 1;
    }
    if (ev == 2) {
        Sub_08002B368(4);
        *(volatile s16 *)(uintptr_t)(rec + 186) = 0;
        *(volatile u32 *)(uintptr_t)(rec + 144) = 1;
    }
    if (ev == 32) {
        Sub_08002B368(3);
        if (*(volatile s16 *)(uintptr_t)(rec + 178) == 1)
            *(volatile s16 *)(uintptr_t)(rec + 178) = 0;
    }
    if (ev == 16) {
        Sub_08002B368(3);
        if (*(volatile s16 *)(uintptr_t)(rec + 178) == 0)
            *(volatile s16 *)(uintptr_t)(rec + 178) = 1;
    }
}
#ifndef __APPLE__
void _080013850(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_13850")));
void Sub_080013850(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_13850")));
void sub_080013850(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_13850")));
#endif

// ----------------------------------------------------------------------------
// sub_080015BA0 — (ev, a1, a2, rec): ev-1 indexes a 12-entry table.
//   0->15304(rec), 1->15230(rec,a1), 4->D854(rec+16)+D8E4(rec+120),
//   5->15B54(rec)+[u16[rec+20]!=0] on s16[rec+138]:
//     0->154E0, 1->1573C, 2->15620 (each (rec,(u16)a1,(u16)a2)),
//   6->on s16[rec+138]: 0/1->15A0C(rec), 2->15AB0(rec),
//   11->1529C(rec), else no-op.
void MenuFF78_15BA0(int ev, u32 a1, u32 a2, void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int idx = ev - 1;
    if (idx < 0 || idx > 11) return;
    switch (idx) {
    case 0:
        Sub_080015304(rec_);
        break;
    case 1:
        Sub_080015230(rec_, (void *)(uintptr_t)a1);
        break;
    case 4:
        Sub_0800D854((void *)(uintptr_t)(rec + 16));
        Sub_0800D8E4((void *)(uintptr_t)(rec + 120));
        break;
    case 5: {
        Sub_080015B54(rec_);
        if (*(volatile u16 *)(uintptr_t)(rec + 20) == 0) break;
        s16 s = *(volatile s16 *)(uintptr_t)(rec + 138);
        u16 r5 = (u16)a1, r6 = (u16)a2;
        if (s == 0)
            Sub_0800154E0(rec_, (u32)r5, (u32)r6);
        else if (s == 1)
            Sub_08001573C(rec_, (u32)r5, (u32)r6);
        else if (s == 2)
            Sub_080015620(rec_, (u32)r5, (u32)r6);
        break;
    }
    case 6: {
        s16 s = *(volatile s16 *)(uintptr_t)(rec + 138);
        if (s == 0 || s == 1)
            Sub_080015A0C(rec_);
        else if (s == 2)
            Sub_080015AB0(rec_);
        break;
    }
    case 11:
        Sub_08001529C();
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _080015BA0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_15BA0")));
void sub_080015BA0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_15BA0")));
#endif
