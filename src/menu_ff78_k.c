// ============================================================================
// menu_ff78_k.c — reconstructed C for asm/menu_ff78.s (6 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080012854 (0x080012854) — full screen setup (07770/DAB8/7614/...).
//   sub_080013EB0 (0x080013EB0) — (ev, a1, a2, rec) 12-way dispatcher.
//   sub_080013F94 (0x080013F94) — (rec) s16[rec+160] clamp machine.
//   sub_080013FE4 (0x080013FE4) — (rec) 6-way jump table on s16[rec+166].
//   sub_080014AC4 (0x080014AC4) — setup + 14650/146D0/1479C/148D8 chain.
//   sub_080014C88 (0x080014C88) — full screen setup twin of 12854.
//
// NOTES:
// - 12854 stores 2 to u32[rec+140], reusing the pointer from the
//   rec+120 store. The ROM template 0x082A798C is only a call argument.
// - 14C88's s16[rec+140] store on the WA+0x5E0==0 path uses r1 leftover
//   from "movs r1,#2" two stores earlier (no intervening call clobbers
//   r1: only strh/ldr/arith follow). Value is deterministically 2; the
//   nonzero path stores 1.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; }
__attribute__((weak)) void Sub_0800DAB8(void *a) { (void)a; }
__attribute__((weak)) void Sub_08007614(void *a, int b, int c, int d) { (void)a; (void)b; (void)c; (void)d; }
__attribute__((weak)) void Sub_0800798C(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_08007A58(void *a) { (void)a; }
__attribute__((weak)) void Sub_080075E8(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800D77C(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void *Sub_08004B68(void) { return 0; }
__attribute__((weak)) int Sub_08002124(u32 v) { (void)v; return 0; }
__attribute__((weak)) void Sub_08002B214(int v) { (void)v; }
__attribute__((weak)) void Sub_08002B234(void) {}
__attribute__((weak)) int Sub_08024C90(int v) { return v; }
__attribute__((weak)) void Sub_080013108(void *a) { (void)a; }
__attribute__((weak)) void Sub_080012ED0(void *a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800D854(void *a) { (void)a; }
__attribute__((weak)) void Sub_080012E9C(void *a) { (void)a; }
__attribute__((weak)) void Sub_080013E4C(void *a) { (void)a; }
__attribute__((weak)) void Sub_080013B68(void *a) { (void)a; }
__attribute__((weak)) void Sub_080013E64(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800133E0(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800133E4(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080013850(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080012F14(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800DBE8(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014650(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800146D0(void *a) { (void)a; }
__attribute__((weak)) void Sub_08001479C(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800148D8(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014A08(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014BCC(void *a) { (void)a; }
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
extern void Sub_0800DAB8(void *a);
extern void Sub_08007614(void *a, int b, int c, int d);
extern void Sub_0800798C(void *a, void *b);
extern void Sub_08007A58(void *a);
extern void Sub_080075E8(void *a, int b, int c);
extern void Sub_0800D77C(void *a, int b, int c);
extern void *Sub_08004B68(void);
extern int Sub_08002124(u32 v);
extern void Sub_08002B214(int v);
extern void Sub_08002B234(void);
extern int Sub_08024C90(int v);
extern void Sub_080013108(void *a);
extern void Sub_080012ED0(void *a, u32 b);
extern void Sub_0800D854(void *a);
extern void Sub_080012E9C(void *a);
extern void Sub_080013E4C(void *a); // call site sets r0=rec (asm:13F1E)
extern void Sub_080013B68(void *a);
extern void Sub_080013E64(void *a);
extern void Sub_0800133E0(void *a, u32 b, u32 c);
extern void Sub_0800133E4(void *a, u32 b, u32 c);
extern void Sub_080013850(void *a, u32 b, u32 c);
extern void Sub_080012F14(void *a);
extern void Sub_0800D97C(void *a, int b);
extern void Sub_0800DBE8(void *a);
extern void Sub_080014650(void *a, u32 b, u32 c);
extern void Sub_0800146D0(void *a);
extern void Sub_08001479C(void *a);
extern void Sub_0800148D8(void *a);
extern void Sub_080014A08(void *a);
extern void Sub_080014BCC(void *a);
// Closure spellings for the _080014C88 callees. On the ROM build each call must
// name a label the closure actually defines (see K_CALLEE); the friendly `Sub_`
// spellings above are host-only weak stubs and the promotion screen rejects them
// with "closure defines ... (rename)".
extern void sub_0800DAB8(void *a);
extern void sub_08007614(void *a, int b, int c, int d);
extern void sub_0800798C(void *a, void *b);
extern void sub_08007A58(void *a);
extern void sub_080075E8(void *a, int b, int c);
extern void sub_0800D77C(void *a, int b, int c);
extern void sub_0802B234(void);
// Closure spellings for the _080013EB0 callees (see K_CALLEE).
extern void sub_080013108(void *a);
extern void sub_080012ED0(void *a, u32 b);
extern void sub_0800D854(void *a);
extern void sub_080012E9C(void *a);
extern void sub_080013E4C(void *a);
extern void sub_080013B68(void *a);
extern void sub_080013E64(void *a);
extern void sub_0800133E0(void *a, u32 b, u32 c);
extern void sub_0800133E4(void *a, u32 b, u32 c);
extern void sub_080013850(void *a, u32 b, u32 c);
extern void sub_080012F14(void *a);
// Closure spellings for the _080012854 callees not shared with _080014C88.
extern void sub_0802B214(int v);
extern void _08002124(u32 v);
extern void *sub_08004B68(void);
extern int sub_08024C90(int v);
// Closure spellings for the _080014AC4 callees.
extern void sub_0800D97C(void *a, int b);
extern void sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void sub_080014650(void *a, u32 b, u32 c);
extern void sub_0800146D0(void *a);
extern void sub_08001479C(void *a);
extern void sub_0800148D8(void *a);
extern void sub_080014A08(void *a);
extern void sub_0800DBE8(void *a);
extern void sub_080014BCC(void *a);
#endif

// Call-site split for promoted bodies: friendly name on the host build, closure
// spelling on the ROM build. Same split as FF_CALLEE/J_CALLEE; kept per file so
// each translation unit stays self-contained. Must sit below the __APPLE__
// split so tools/apple_decls.py does not report it as undeclared on the host.
#ifndef __APPLE__
#define K_CALLEE(friendly, closure) closure
#else
#define K_CALLEE(friendly, closure) friendly
#endif

// ----------------------------------------------------------------------------
// sub_080012854 — (rec) full screen setup.
//   2B214(49); if u16[WA+0xFBC]==3: 02124(0x1393).
//   07770(0, 0x082E7DD8, 1, 0, 4, 1);
//   u16[rec+80]=6; u32[rec+92]=10; u32[rec+104]=1; DAB8(rec+8);
//   tile = 0x082A798C: 7614(tile,1,0,3); 7614(tile,1,1,4); 7614(tile,1,2,5);
//   798C(tmpl,rec); 7A58(rec); 75E8(tmpl,0,6);
//   u32[rec+120]=rec+140; u32[rec+124]=rec+8; u32[rec+8]=0; s16[rec+12]=1;
//   D77C(rec+28,0,-32); D77C(rec+20,0,160);
//   s16[rec+16]=6; s16[rec+18]=5;
//   car==20: s16[rec+130]=u16[WA+0xFBE];
//   else: s16[rec+130]=(s16)24C90(s16[WA+0x574]);
//   s16[rec+128]=s16[rec+130]; u32[rec+140]=2; u16[WA+0x1058]=0.
void MenuFF78_12854(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    // Assembler-resolved work-area base so the 0xFBC/0xFBE/0x574/0x1058
    // offsets stay separate pool words added at runtime (the ROM keeps the base
    // in sl and adds each offset), instead of folding base+offset together.
    extern u8 J12854_WA[];
    volatile u8 *wa;
    __asm__(".globl J12854_WA\nJ12854_WA = 0x03001780\n");
    K_CALLEE(Sub_08002B214, sub_0802B214)(49);
    wa = (volatile u8 *)(uintptr_t)J12854_WA;
    if (*(volatile u16 *)(uintptr_t)(wa + 0xFBCu) == 3)
        K_CALLEE(Sub_08002124, _08002124)(0x1393u);
    _08007770(0, (void *)(uintptr_t)0x082E7DD8u, 1, 0, 4, 1); // R2 C body (was Sub_ veneer)
    *(volatile u32 *)(uintptr_t)(rec + 80) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 92) = 10;
    *(volatile u32 *)(uintptr_t)(rec + 104) = 1;
    K_CALLEE(Sub_0800DAB8, sub_0800DAB8)((void *)(uintptr_t)(rec + 8));
    K_CALLEE(Sub_08007614, sub_08007614)((void *)(uintptr_t)0x082A798Cu, 1, 0, 3);
    K_CALLEE(Sub_08007614, sub_08007614)((void *)(uintptr_t)0x082A798Cu, 1, 1, 4);
    K_CALLEE(Sub_08007614, sub_08007614)((void *)(uintptr_t)0x082A798Cu, 1, 2, 5);
    K_CALLEE(Sub_0800798C, sub_0800798C)((void *)(uintptr_t)0x082E7DD8u, rec_);
    K_CALLEE(Sub_08007A58, sub_08007A58)(rec_);
    K_CALLEE(Sub_080075E8, sub_080075E8)((void *)(uintptr_t)0x082E7DD8u, 0, 6);
    *(volatile u32 *)(uintptr_t)(rec + 120) = (u32)(uintptr_t)(rec + 140);
    *(volatile u32 *)(uintptr_t)(rec + 124) = (u32)(uintptr_t)(rec + 8);
    *(volatile u32 *)(uintptr_t)(rec + 8) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 12) = 1;
    K_CALLEE(Sub_0800D77C, sub_0800D77C)((void *)(uintptr_t)(rec + 28), 0, -32);
    K_CALLEE(Sub_0800D77C, sub_0800D77C)((void *)(uintptr_t)(rec + 20), 0, 160);
    *(volatile s16 *)(uintptr_t)(rec + 16) = 6;
    *(volatile s16 *)(uintptr_t)(rec + 18) = 5;
    {
        u16 car = *(volatile u16 *)(uintptr_t)((volatile u8 *)K_CALLEE(Sub_08004B68, sub_08004B68)() + 2);
        if (car == 20) {
            u16 v = *(u16 *)(uintptr_t)(wa + 0xFBEu);
            *(s16 *)(uintptr_t)(rec + 130) = v;
        } else {
            s16 w = *(s16 *)(uintptr_t)(wa + 0x574u);
            *(volatile s16 *)(uintptr_t)(rec + 130) = (s16)K_CALLEE(Sub_08024C90, sub_08024C90)((int)w);
        }
    }
    {
        // The ROM materialises the final WA store's zero into r2 early (before
        // the rec+128 copy) and reuses it at the end, so the pinned zero is
        // assigned here rather than at the WA store.
        register u16 z __asm__("r2");
        {
            s16 v = *(s16 *)(uintptr_t)(rec + 130);
            s16 *dst = (s16 *)(uintptr_t)(rec + 128);
            z = 0;
            *dst = v;
        }
        // The ROM keeps `rec + 140` in r4 from the rec+120 pointer store above
        // and reuses it here; the target is rec+140 (not the ROM word).
        *(volatile u32 *)(uintptr_t)(rec + 140) = 2;
        {
            // A second alias for the same work-area base forces the ROM's
            // second `ldr r0,=0x03001780` pool word (the base is otherwise
            // CSE'd into sl or folded into 0x030027D8).
            extern u8 J12854_WA2[];
            __asm__(".globl J12854_WA2\nJ12854_WA2 = 0x03001780\n");
            u8 *wa2 = (u8 *)(uintptr_t)J12854_WA2;
            *(volatile u16 *)(uintptr_t)(wa2 + 0x1058u) = z;
        }
    }
}
#ifndef __APPLE__
void _080012854(void *a) __attribute__((alias("MenuFF78_12854")));
void sub_080012854(void *a) __attribute__((alias("MenuFF78_12854")));
#endif

// ----------------------------------------------------------------------------
// sub_080013EB0 — (ev, a1, a2, rec): ev-1 indexes a 12-entry table.
//   0->13108(rec), 1->12ED0(rec,a1),
//   4->D854(rec+32)+12E9C(rec)+[u16[WA+0xFBC]==3]13E4C(rec),
//   5->13B68(rec),
//   6->13E64(rec)+[u16[rec+36]!=0]133E0(rec,(u16)a1,(u16)a2)+
//     on s16[rec+186]: 0->133E4, 1->13850 (each (rec,(u16)a1,(u16)a2)),
//   11->12F14(rec), else no-op.
void MenuFF78_13EB0(int ev, u32 a1, u32 a2, void *rec_) {
    u8 *rec = (u8 *)rec_;
    int idx = ev - 1;
    if (idx < 0 || idx > 11) return;
    switch (idx) {
    // Source order mirrors the ROM body layout (1,4,6,5,0,11): agbcc emits
    // switch-arm blocks in source order when no reordering pass runs.
    case 1:
        K_CALLEE(Sub_080012ED0, sub_080012ED0)((void *)rec, a1);
        break;
    case 4:
        K_CALLEE(Sub_0800D854, sub_0800D854)((void *)(uintptr_t)(rec + 32));
        K_CALLEE(Sub_080012E9C, sub_080012E9C)((void *)rec);
        {
            // Assembler-resolved work-area base assigned to a local keeps
            // 0x03001780 and 0xFBC as two pool words (`ldr r0,=WA;
            // ldr r1,=0xFBC; adds r0,r0,r1`); adding the offset to the array
            // expression folds to the single literal 0x0300273C.
            extern u8 J13EB0_WA[];
            __asm__(".globl J13EB0_WA\nJ13EB0_WA = 0x03001780\n");
            u8 *wa = (u8 *)(uintptr_t)J13EB0_WA;
            if (*(volatile u16 *)(uintptr_t)(wa + 0xFBCu) == 3)
                K_CALLEE(Sub_080013E4C, sub_080013E4C)((void *)rec);
        }
        break;
    case 6:
        K_CALLEE(Sub_080013B68, sub_080013B68)((void *)rec);
        break;
    case 5:
        // ROM table slot 5 -> 0x08013F38 (13E64 body); the truncated a1/a2 are
        // written back into the parameter registers so the later calls reuse
        // r5/r6 instead of spilling them to r7/r8.
        K_CALLEE(Sub_080013E64, sub_080013E64)((void *)rec);
        if (*(u16 *)(uintptr_t)(rec + 36) != 0) {
            a1 = (u32)(u16)a1;
            a2 = (u32)(u16)a2;
            K_CALLEE(Sub_0800133E0, sub_0800133E0)((void *)rec, a1, a2);
            {
                // Two-arm `switch` (not if/else): the ROM keeps BOTH bodies
                // out-of-line behind `beq` tests, with the fall-through
                // `b end` between them; an if/else if inlines the first body.
                s16 s = *(s16 *)(uintptr_t)(rec + 186);
                switch (s) {
                case 0:
                    K_CALLEE(Sub_0800133E4, sub_0800133E4)((void *)rec, a1, a2);
                    break;
                case 1:
                    K_CALLEE(Sub_080013850, sub_080013850)((void *)rec, a1, a2);
                    break;
                }
            }
        }
        break;
    case 0:
        K_CALLEE(Sub_080013108, sub_080013108)((void *)rec);
        break;
    case 11:
        K_CALLEE(Sub_080012F14, sub_080012F14)((void *)rec);
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _080013EB0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_13EB0")));
void sub_080013EB0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_13EB0")));
#endif

// ----------------------------------------------------------------------------
// sub_080013F94 — (rec) s16[rec+160] clamp machine:
//   if s16[160]-1 >= s16[156]: s16[160] = u16[156].
//   if s16[160]+4 <= s16[156]: s16[160] = u16[156]-3.
//   clamp 0..7.
// The ROM's skips are `blt` / `bgt` over the bodies, so the source conditions
// are the negations of what the branch mnemonics suggest at a glance, and the
// copies are u16 reads (`ldrh`), not s16 (`ldrsh`).
void MenuFF78_13F94(void *rec_) {
    u8 *rec = (u8 *)rec_;
    if (*(s16 *)(uintptr_t)(rec + 160) - 1 >= *(s16 *)(uintptr_t)(rec + 156))
        *(u16 *)(uintptr_t)(rec + 160) = *(u16 *)(uintptr_t)(rec + 156);
    if (*(s16 *)(uintptr_t)(rec + 160) + 4 <= *(s16 *)(uintptr_t)(rec + 156))
        *(u16 *)(uintptr_t)(rec + 160) =
            (u16)(*(u16 *)(uintptr_t)(rec + 156) - 3);
    if (*(s16 *)(uintptr_t)(rec + 160) < 0)
        *(u16 *)(uintptr_t)(rec + 160) = 0;
    if (*(s16 *)(uintptr_t)(rec + 160) > 7)
        *(u16 *)(uintptr_t)(rec + 160) = 7;
}
#ifndef __APPLE__
void _080013F94(void *a) __attribute__((alias("MenuFF78_13F94")));
void Sub_080013F94(void *a) __attribute__((alias("MenuFF78_13F94")));
void sub_080013F94(void *a) __attribute__((alias("MenuFF78_13F94")));
#endif
// 78-byte body, two short of the section's 4-byte alignment: gas pads with
// `nop` (0x46c0) where the ROM holds `00 00`.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// sub_080013FE4 — (rec) 6-way jump table on s16[rec+166] (else no-op):
//   0: [164]=0xFFD0,[166]=2; 1: [164]=48,[166]=3;
//   2: [164]+=12, if >0: [166]=4; 3: [164]-=12, if <0: [166]=4;
//   4: [164]=0,[166]=5; 5: no-op.
void MenuFF78_13FE4(void *rec_) {
    // No explicit `s < 0 || s > 5` guard: that emits TWO range checks plus a
    // shift-extended `ldrh` read. A bare `switch (int s)` lets agbcc emit the
    // ROM's single signed `ldrsh` + `cmp #5; bhi default`, and an explicit empty
    // `case 5` extends the jump table to six entries (index 5 -> default).
    u8 *rec = (u8 *)rec_;
    int s = *(s16 *)(uintptr_t)(rec + 166);
    switch (s) {
    case 0: {
        // `0xFFD0` (not `(s16)-48`): the ROM keeps 0xFFD0 in the pool
        // rather than synthesising it with `movs #48; negs`. Writing the
        // value through a `u16 *` local + a `u16` temp makes agbcc compute
        // the address first and `ldr r0` straight into the halfword store
        // register; a bare literal instead emits `ldr r3; adds r0,r3,#0`.
        u16 *p = (u16 *)(uintptr_t)(rec + 164);
        u16 t = 0xFFD0;
        *p = t;
        *(u16 *)(uintptr_t)(rec + 166) = 2;
        break;
    }
    case 1:
        *(u16 *)(uintptr_t)(rec + 164) = 48;
        *(u16 *)(uintptr_t)(rec + 166) = 3;
        break;
    case 2: {
        // Unsigned read-modify-write; the `(s16)` test is what makes agbcc
        // reuse the stored halfword with `lsls r0,r0,#16; cmp r0,#0`.
        int v = *(u16 *)(uintptr_t)(rec + 164) + 12;
        *(u16 *)(uintptr_t)(rec + 164) = v;
        if ((s16)v > 0)
            *(u16 *)(uintptr_t)(rec + 166) = 4;
        break;
    }
    case 3: {
        int v = *(u16 *)(uintptr_t)(rec + 164) - 12;
        *(u16 *)(uintptr_t)(rec + 164) = v;
        if ((s16)v < 0)
            *(u16 *)(uintptr_t)(rec + 166) = 4;
        break;
    }
    case 4:
        *(u16 *)(uintptr_t)(rec + 164) = 0;
        *(u16 *)(uintptr_t)(rec + 166) = 5;
        break;
    case 5:
        break;
    }
}
#ifndef __APPLE__
void _080013FE4(void *a) __attribute__((alias("MenuFF78_13FE4")));
void Sub_080013FE4(void *a) __attribute__((alias("MenuFF78_13FE4")));
void sub_080013FE4(void *a) __attribute__((alias("MenuFF78_13FE4")));
#endif
// 146-byte body, one word short of the section's 4-byte alignment: gas pads
// with `nop` (0x46c0) where the ROM holds `00 00`.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// sub_080014AC4 — (rec):
//   D97C(rec+152,15); r1 = s16[WA+0xFF2];
//   r1==0: 7B18(rec+8,4,16,32,5,1,0,0), r6 = rec+8;
//   r1==1: 7B18(rec+8,3,16,32,5,1,0,0), r6 = rec+8;
//   else: r6 = rec+8.
//   7B18(r6,8,s16[rec+160]*8+144,32,5,1,0,0);
//   14650(rec,128,104); 146D0(rec); 1479C(rec); 148D8(rec);
//   if u16[rec+36]==1:
//     7B18(r6,9,-40,48,5,1,0,0); 7B18(r6,9,216,48,5,1,0,0).
//   14A08(rec); DBE8(rec+32); 14BCC(rec).
void MenuFF78_14AC4(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 r1;
    K_CALLEE(Sub_0800D97C, sub_0800D97C)((void *)(uintptr_t)(rec + 152), 15);
    {
        // Assembler-resolved work-area base keeps 0x03001780 and 0xFF2 as two
        // pool words (`ldr r0,=WA; ldr r1,=0xFF2; adds r0,r0,r1`).
        extern u8 J14AC4_WA[];
        __asm__(".globl J14AC4_WA\nJ14AC4_WA = 0x03001780\n");
        u8 *wa = (u8 *)(uintptr_t)J14AC4_WA;
        r1 = *(s16 *)(uintptr_t)(wa + 0xFF2u);
    }
    // Two-arm `switch` keeps both bodies out-of-line behind `beq` tests
    // (an if/else if inlines the first body and reorders the else path).
    switch (r1) {
    case 0:
        K_CALLEE(Sub_08007B18, sub_08007B18)((void *)(uintptr_t)(rec + 8), 4, 16, 32, 5, 1, 0, 0);
        break;
    case 1:
        K_CALLEE(Sub_08007B18, sub_08007B18)((void *)(uintptr_t)(rec + 8), 3, 16, 32, 5, 1, 0, 0);
        break;
    }
    {
        s16 w = *(s16 *)(uintptr_t)(rec + 160);
        K_CALLEE(Sub_08007B18, sub_08007B18)((void *)(uintptr_t)(rec + 8), 8,
                     (int)w * 8 + 144, 32, 5, 1, 0, 0);
    }
    K_CALLEE(Sub_080014650, sub_080014650)((void *)rec, 128, 104);
    K_CALLEE(Sub_0800146D0, sub_0800146D0)((void *)rec);
    K_CALLEE(Sub_08001479C, sub_08001479C)((void *)rec);
    K_CALLEE(Sub_0800148D8, sub_0800148D8)((void *)rec);
    if (*(u16 *)(uintptr_t)(rec + 36) == 1) {
        K_CALLEE(Sub_08007B18, sub_08007B18)((void *)(uintptr_t)(rec + 8), 9, -40, 48, 5, 1, 0, 0);
        K_CALLEE(Sub_08007B18, sub_08007B18)((void *)(uintptr_t)(rec + 8), 9, 216, 48, 5, 1, 0, 0);
    }
    K_CALLEE(Sub_080014A08, sub_080014A08)((void *)rec);
    K_CALLEE(Sub_0800DBE8, sub_0800DBE8)((void *)(uintptr_t)(rec + 32));
    K_CALLEE(Sub_080014BCC, sub_080014BCC)((void *)rec);
}
#ifndef __APPLE__
void _080014AC4(void *a) __attribute__((alias("MenuFF78_14AC4")));
void Sub_080014AC4(void *a) __attribute__((alias("MenuFF78_14AC4")));
void sub_080014AC4(void *a) __attribute__((alias("MenuFF78_14AC4")));
#endif
// 262-byte body, one word short of the section's 4-byte alignment: gas pads
// with `nop` (0x46c0) where the ROM holds `00 00`.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// sub_080014C88 — (rec) full screen setup twin of 12854:
//   07770(0, 0x082F8348, 3, 0, 4, 1);
//   u16[rec+80]=6; u32[rec+92]=11; u32[rec+104]=1; DAB8(rec+8);
//   tile = 0x082A798C: 7614(tile,1,0,3); 7614(tile,1,1,4);
//   798C(tmpl,rec); 7A58(rec);
//   75E8(tmpl,0,6); 75E8(tmpl,1,5); 75E8(tmpl,2,7);
//   u32[rec+120]=rec+144; u32[rec+124]=rec+8; u32[rec+8]=0; s16[rec+12]=1;
//   D77C(rec+28,0,-32); D77C(rec+20,0,160);
//   s16[rec+16]=6; s16[rec+18]=5; u32[rec+144]=2;
//   s16[rec+136]=1; s16[rec+138]=1;
//   s16[rec+140] = (s16[WA+0x5E0] != 0) ? 1 : 2 (stale-r1 proof, see NOTE);
//   s16[rec+134]=0; 2B234.
void MenuFF78_14C88(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    void *tmpl = (void *)(uintptr_t)0x082F8348u;
    // Assembler-resolved work-area base so the 0x5E0 offset stays a separate
    // constant (the ROM synthesises it as `movs #0xbc; lsls #3`) instead of
    // folding base+0x5E0 into one pool word.
    extern u8 J14C88_WA[];
    __asm__(".globl J14C88_WA\nJ14C88_WA = 0x03001780\n");
    _08007770(0, tmpl, 3, 0, 4, 1); // R2 C body (was Sub_ veneer)
    *(volatile u32 *)(uintptr_t)(rec + 80) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 92) = 11;
    *(volatile u32 *)(uintptr_t)(rec + 104) = 1;
    K_CALLEE(Sub_0800DAB8, sub_0800DAB8)((void *)(uintptr_t)(rec + 8));
    K_CALLEE(Sub_08007614, sub_08007614)((void *)(uintptr_t)0x082A798Cu, 1, 0, 3);
    K_CALLEE(Sub_08007614, sub_08007614)((void *)(uintptr_t)0x082A798Cu, 1, 1, 4);
    K_CALLEE(Sub_0800798C, sub_0800798C)(tmpl, rec_);
    K_CALLEE(Sub_08007A58, sub_08007A58)(rec_);
    K_CALLEE(Sub_080075E8, sub_080075E8)(tmpl, 0, 6);
    K_CALLEE(Sub_080075E8, sub_080075E8)(tmpl, 1, 5);
    K_CALLEE(Sub_080075E8, sub_080075E8)(tmpl, 2, 7);
    *(volatile u32 *)(uintptr_t)(rec + 120) = (u32)(uintptr_t)(rec + 144);
    *(volatile u32 *)(uintptr_t)(rec + 124) = (u32)(uintptr_t)(rec + 8);
    *(volatile u32 *)(uintptr_t)(rec + 8) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 12) = 1;
    K_CALLEE(Sub_0800D77C, sub_0800D77C)((void *)(uintptr_t)(rec + 28), 0, -32);
    K_CALLEE(Sub_0800D77C, sub_0800D77C)((void *)(uintptr_t)(rec + 20), 0, 160);
    *(volatile s16 *)(uintptr_t)(rec + 16) = 6;
    *(volatile s16 *)(uintptr_t)(rec + 18) = 5;
    *(volatile u32 *)(uintptr_t)(rec + 144) = 2;
    *(volatile s16 *)(uintptr_t)(rec + 136) = 1;
    *(volatile s16 *)(uintptr_t)(rec + 138) = 1;
    {
        u8 *wa = (u8 *)(uintptr_t)J14C88_WA;
        if (*(s16 *)(wa + 0x5E0u) == 0)
            *(volatile s16 *)(uintptr_t)(rec + 140) = 2;
        else
            *(volatile s16 *)(uintptr_t)(rec + 140) = 1;
    }
    *(volatile s16 *)(uintptr_t)(rec + 134) = 0;
    K_CALLEE(Sub_08002B234, sub_0802B234)();
}
// The ROM's 268-byte span ends in `00 00`; this file-scope `.align 2, 0`
// (after the body's `.size`) pads with the explicit `0` fill instead of gas's
// `46c0` Thumb-section-close nop.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080014C88(void *a) __attribute__((alias("MenuFF78_14C88")));
void Sub_080014C88(void *a) __attribute__((alias("MenuFF78_14C88")));
void sub_080014C88(void *a) __attribute__((alias("MenuFF78_14C88")));
#endif
