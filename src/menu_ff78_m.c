// ============================================================================
// menu_ff78_m.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Both bodies are C transcriptions; external callees use the Sub_ spellings
// the asm closure defines, so behaviour is identical by construction.
//
//   sub_080013850 (0x080013850) — (rec, dead, ev) event handler.
//   sub_080015BA0 (0x080015BA0) — (ev, a1, a2, rec) 12-way dispatcher.
// ============================================================================

#include "gba/types.h"

extern void _0802B368(u16 v);   // 0x0802B368 sound entry
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
void MenuFF78_13850(void *rec_, u32 dead, u32 ev_) {
    u8 *rec = (u8 *)rec_;
    u16 ev = (u16)ev_;

    (void)dead;
    if (ev == 1) {
        // Assembler-resolved work-area base: keeps 0x03001780 and each offset
        // as separate pool words (the ROM keeps them apart and adds at runtime).
        extern u8 M13850_WA[];
        u8 *wa;
        u16 t;
        u16 v;
        u32 off;
        u8 *p49;
        u8 *p48;
        __asm__(".globl M13850_WA\nM13850_WA = 0x03001780\n");

        _0802B368(1);
        *(u32 *)(rec + 32) = 10;
        *(s16 *)(rec + 36) = 0;
        *(u32 *)(rec + 64) = 10;
        switch (*(s16 *)(rec + 138)) {
        case 0:
            wa = (u8 *)(uintptr_t)M13850_WA;
            t = *(u16 *)(rec + 176);
            *(u16 *)(wa + 0xFBE) = t;
            v = *(u16 *)(rec + 174);
            off = 0xFC2;
            break;
        case 1:
            wa = (u8 *)(uintptr_t)M13850_WA;
            t = *(u16 *)(rec + 176);
            *(u16 *)(wa + 0xFBE) = t;
            v = *(u16 *)(rec + 174);
            off = 0x574;
            break;
        default:
            goto tail;
        }
        *(u16 *)(wa + off) = v;
        // The record base is the *second* addend (index*12 + wa) so the sum
        // accumulates into the index register, as the ROM does.
        p49 = (u8 *)(uintptr_t)((u32)(*(s16 *)(wa + off) * 12) + (u32)(uintptr_t)wa);
        t = *(u16 *)(rec + 172);
        p49 += 49;
        *p49 = (u8)t;
        p48 = (u8 *)(uintptr_t)((u32)(*(s16 *)(wa + off) * 12) + (u32)(uintptr_t)wa);
        t = *(u16 *)(rec + 178);
        p48 += 48;
        *p48 = (u8)t;
    tail:
        *(s16 *)(rec + 40) = 5;
        *(u32 *)(rec + 60) = 1;
    }
    if (ev == 2) {
        _0802B368(4);
        *(s16 *)(rec + 186) = 0;
        *(u32 *)(rec + 144) = 1;
    }
    if (ev == 32) {
        _0802B368(3);
        if (*(u16 *)(rec + 178) == 1)
            *(s16 *)(rec + 178) = 0;
    }
    if (ev == 16) {
        _0802B368(3);
        if (*(s16 *)(rec + 178) == 0)
            *(s16 *)(rec + 178) = 1;
    }
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
