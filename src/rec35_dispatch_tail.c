// ============================================================================
// rec35_dispatch_tail.c — C lift of asm/rec35_runtime.s 0x080186A8-0x080188B0
// : the rec35 runtime-registered scene dispatch tail —
// gate leaf, 12-way event dispatcher, setup/countdown/dispatch twins, and
// the menu record-type switch. 9/9 function VMAs, 0 gaps.
//
// Evidence: instruction-for-instruction from asm/rec35_runtime.s; pool words
// 0x03001780/0x000010D8/0x08018708/0x08292B40/0x0805FB66/0x00002328/
// 0x000010C1/0x000010C6/0x000010FC preserved as immediates.
//
//   _0800186A8 gate leaf: x=u32[WA+0x10D8]; Div(DivRem(x,48),24)!=0 emits one
//     8-arg _08007B18(rec+28,13,88,112,0,0,0,0). (Div=quotient _0802D978,
//     DivRem=remainder _0802D97C, both naked swi 0x06 in bios_wrappers.c.)
//   _0800186F0 12-way event dispatcher (subs r0,#1; cmp #11; bhi default;
//     table 0x08018708): ev1->183D8, ev5->18420, ev6->18540 (u16 a,b),
//     ev7->186A8, ev{2,3,4,8,9,10,11,12}->18418 (default arm falls into the
//     same 18418 call). Signature (ev,a,b,rec) with rec in r3.
//   _08001876C/_08001881C setup twins: sub_08007664(0,0x08292B40,7/10);
//     u32[ctx]=150; u32[ctx+4]=0.
//   _08001878C/_08001883C countdown twins: s32[ctx]>0 ? s32[ctx]-- :
//     (s32[ctx+4]==0 ? (s32[ctx+4]=1, _08004EC0(1))). The 4EC0 call carries
//     r0=1 in ROM; the strong c_trampolines.s trampoline shadows the weak
//     no-ops, so this is the exact ROM body.
//   _0800187B0/_080018860 bx lr stubs (2 B + pad; case-6 args dead).
//   _0800187B4 flag-test leaf: r3=134<<5=0x10C0; v=(u8[WA+0x10C0]!=0 ?
//     (clear,1) : 2); u16[base+84]=v where base=dispatcher's r1.
//   _080018864 join leaf: u16[base+84]=1.
//   _0800187D8/_08001886C 6-way cmp-chain dispatchers (NOT table-driven):
//     2->7B4/864 leaf, 1->76C/81C setup, 5->78C/83C countdown,
//     6->7B0/860 stub (u16 a,b, dead); else return. rec in r3.
//   _0800188B0 menu record-type switch (cf. _0800188B0 consumer note):
//     s16[rec]==-1 ? template path (memcpy 30 B of 0x0805FB66 to sp frame,
//     idx=s16[rec+2], v=(idx>14 ? u16[sp+28] : u16[sp+idx*2]),
//     s16[rec]=v, u32[rec+12]=0x2328, u8[WA+0x10C1]=1)
//     : (u8[WA+0x10C1]=0, u16[WA+0x10C6]=u16[rec+6]); then on s16[rec]:
//     4->{s16[rec+4]=Sub_080024E7C(s16[rec+24])}, 0->{[rec+2]=[rec+8]=0},
//     9->random-car pick (08014/05B5C random source, (r&31)+16 lane,
//     Course_GetCup==1 -> [rec+8]=2, DivRem(.,98) loop excluding bonus/
//     special ids {86,56,55,31,14,98,54,94,95,96} + Ai_IdMap==-1 retry,
//     Ai_LineGet0 s8 divisor, u8[rec+29]=DivRem), 10->{flag 0,[rec+8]=0,
//     [rec+2]=48,[rec+26]=0}, 6->{[rec]=3}, 7->{[rec]=4 + 24E7C path};
//     tail: Ai_IdMap(s16[rec+24])==-1 -> ([rec+24]=10,[rec+29]=0);
//     memcpy(WA+0x10FC,rec,56).
//
// Callee routing: _0800 spellings bind strong lifted bodies
// (bios_wrappers/course_records/code_5b3c_math/course_cal/ai_catalog/
// ai_line_leaves/foundation_runtime/); Sub_080024E7C and
// Sub_08008014(int) follow the int-prototype precedent (_08008014's
// strong body is void-typed but the callers consume r0;
// scene_record_dispatch.c:1068/1113 already rely on this).
// Residual: none in this span — all 9 VMAs evidence-backed.
// ============================================================================

#include "gba/types.h"
#include <stdint.h>

// ----------------------------------------------------------------------------
// Callees (HOST_STUB = weak decl on Apple host builds, extern on ARM; strong
// bodies win on the ARM link).
// ----------------------------------------------------------------------------
#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

HOST_STUB(int _0802D978(int a, int b));                                 // swi Div (quotient)
HOST_STUB(int _0802D97C(int a, int b));                                 // swi DivRem (remainder)
HOST_STUB(void _08007B18(void *r, int k, int x, int y,
                         u32 a, u32 b, u32 c, u32 d));                  // 8-arg emitter
HOST_STUB(void sub_08007664(void *a, void *b, int c));                  // Course_ResourceSetup
HOST_STUB(void _08004EC0(int v));                                       // trampoline -> ROM
HOST_STUB(int _08008014(void));                                         // int-prototype precedent
HOST_STUB(int _08005B5C(int v));                                        // MathLeaf (abs)
HOST_STUB(s16 _080258B8(int idx));                                      // Course_GetCup
HOST_STUB(int _080022E4(int id));                                       // Ai_IdMap
HOST_STUB(s8 _080254E8(int a));                                         // Ai_LineGet0
HOST_STUB(void _08002E0A4(void *d, const void *s, u32 n));              // RuntimeMemcpy
HOST_STUB(int Sub_080024E7C(u16 key));                                    // 0x080024E7C trampoline -> ROM
HOST_STUB(void _0800183D8(void *c));
HOST_STUB(void _080018418(void *a));
HOST_STUB(void _080018420(void *a));
HOST_STUB(void _080018540(void *c, int a, int b));
HOST_STUB(void _0800186A8(void *rec));
HOST_STUB(void _08001876C(void *ctx));
HOST_STUB(void _08001878C(void *ctx));
HOST_STUB(void _0800187B0(void *a, int b, int c));
HOST_STUB(void _0800187B4(void *rec, void *base, int c));
HOST_STUB(void _08001881C(void *ctx));
HOST_STUB(void _08001883C(void *ctx));
HOST_STUB(void _080018860(void *a, int b, int c));
HOST_STUB(void _080018864(void *rec, void *base, int c));

// _0800186A8: x=u32[WA+0x10D8]; Div(DivRem(x,48),24)!=0 ->
// _08007B18(rec+28,13,88,112,0,0,0,0). sp frame 16 B, r4=rec.
void Rec35_Gate_186A8(void *rec) {
    // ROM reaches WA+0x10D8 as TWO pool words plus an `adds r0,r0,r1`
    // (`ldr 0x03001780 / ldr 0x10D8 / adds r0,r0,r1 / ldr r0,[r0]`), so the
    // base must stay a SYMBOL_REF. With a folded CONST_INT base the whole pair
    // collapses into one pool word 0x03002858 (measured). Same lever as
    // src/ai_award_leaves.c (Ai_SetOwnedFlag); the `.globl` definition goes
    // INSIDE the function because agbcc folds a file-scope one away.
#ifndef __APPLE__
    extern u8 Rec35Wa2[] __asm__("Rec35Wa2");
    volatile u8 *wa = (volatile u8 *)(uintptr_t)Rec35Wa2;
    __asm__(".globl Rec35Wa2\nRec35Wa2 = 0x03001780\n");
#else
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
#endif
    u32 x = *(volatile u32 *)(wa + 0x10D8);
    if (_0802D978(_0802D97C((int)x, 48), 24) != 0)
        _08007B18((void *)((volatile u8 *)rec + 28), 13, 88, 112, 0, 0, 0, 0);
}
#ifndef __APPLE__
void _0800186A8(void *rec) __attribute__((alias("Rec35_Gate_186A8")));
#endif

// _0800186F0: 12-way table dispatcher on ev-1 (table 0x08018708; bhi ->
// default). ev1->183D8, ev5->18420, ev6->18540(u16 a,b), ev7->186A8,
// rest->18418. r4=saved r1 (used only for the u16 truncation on ev6).
void Rec35_Dispatch_186F0(int ev, int a, int b, void *rec) {
    switch (ev - 1) {
        case 0: _0800183D8(rec); break;
        case 4: _080018420(rec); break;
        case 6: _0800186A8(rec); break;
        case 5: _080018540(rec, (int)(u16)a, (int)(u16)b); break;
        case 11: _080018418(rec); break;
        default: break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800186F0(int ev, int a, int b, void *rec)
    __attribute__((alias("Rec35_Dispatch_186F0")));
#endif

// _08001876C: sub_08007664(0,0x08292B40,7); u32[ctx]=150; u32[ctx+4]=0.
void Rec35_Setup_1876C(void *ctx) {
    volatile u8 *c = (volatile u8 *)ctx;
    sub_08007664(0, (void *)(uintptr_t)0x08292B40u, 7);
    *(volatile u32 *)(c + 0) = 150;
    *(volatile u32 *)(c + 4) = 0;
}
#ifndef __APPLE__
void _08001876C(void *ctx) __attribute__((alias("Rec35_Setup_1876C")));
#endif

// _08001878C: s32 countdown + _08004EC0(1) latch (r0=1 at the ROM call).
//
// Derived from `ctx` directly, with no `volatile u8 *` base local. The named
// byte pointer makes agbcc materialise a SECOND base register for the ctx+4
// lvalue (`adds r2,r1,#0`; `ldr r0,[r2,#4]`), while the ROM reaches both
// fields off the one register it copied out of r0 (`ldr r0,[r1,#0]` /
// `ldr r0,[r1,#4]`). `src/menu_stage.c`'s MenuStage_0800D704 is the same
// 36-byte ROM span byte for byte and is the recipe this body follows.
void Rec35_Countdown_1878C(void *ctx) {
    s32 n = *(volatile s32 *)ctx;
    if (n > 0) {
        *(volatile s32 *)ctx = n - 1;
    } else if (*(volatile s32 *)((u8 *)ctx + 4) == 0) {
        *(volatile s32 *)((u8 *)ctx + 4) = 1;
        _08004EC0(1);
    }
}
// The body is 34 bytes; the last two bytes of the 36-byte inventory span are
// the ROM's `00 00` inter-function pad. Same file-scope `.align 2, 0` idiom as
// src/menu_stage.c: it lands after the body's `.size`, inside the body's own
// section, and pads with the explicit `0` fill where gas would close a Thumb
// code section with a `nop` (0x46c0).
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001878C(void *ctx) __attribute__((alias("Rec35_Countdown_1878C")));
#endif

// _0800187B0: bx lr stub (incoming rec/u16/u16 all dead).
void Rec35_BxLr_187B0(void *a, int b, int c) { (void)a; (void)b; (void)c; }
#ifndef __APPLE__
void _0800187B0(void *a, int b, int c) __attribute__((alias("Rec35_BxLr_187B0")));
void sub_0800187B0(void *a, int b, int c) __attribute__((alias("Rec35_BxLr_187B0")));
#endif

// _0800187B4: v=(u8[WA+0x10C0]!=0 ? (clear,1) : 2); u16[base+84]=v.
//
// Two measured facts drive this shape:
//  * the WA base must stay a SYMBOL_REF (same lever as Rec35_Gate_186A8
//    above), else the whole address folds to one pool word;
//  * the ROM computes the cell as `movs r3,#134 / lsls r3,#5 / adds r2,r0,r3`,
//    so the offset is a shifted value in its own local, not the literal
//    0x10C0 (same idiom as src/ai_award_leaves.c Ai_SetOwnedFlag).
void Rec35_Leaf_187B4(void *rec, void *base, int c) {
#ifndef __APPLE__
    extern u8 Rec35Wa187B4[] __asm__("Rec35Wa187B4");
    volatile u8 *wa = (volatile u8 *)(uintptr_t)Rec35Wa187B4;
    __asm__(".globl Rec35Wa187B4\nRec35Wa187B4 = 0x03001780\n");
#else
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
#endif
    u32 off = 134;
    off = off << 5;
    volatile u8 *p = wa + off;
    volatile u8 *b = (volatile u8 *)base;
    (void)rec; (void)c;
    if (*p != 0) {
        *p = 0;
        *(volatile u16 *)(b + 84) = 1;
    } else {
        *(volatile u16 *)(b + 84) = 2;
    }
}
#ifndef __APPLE__
void _0800187B4(void *a, void *b, int c) __attribute__((alias("Rec35_Leaf_187B4")));
#endif

// _0800187D8: cmp-chain dispatcher (2->7B4, 1->76C, 5->78C, 6->7B0).
// ev is u32: agbcc then picks the ROM's unsigned `bhi` for the switch's
// ev>2 range test (int gives `bgt`). Same construct as Rec35_Dispatch_18394.
void Rec35_Dispatch_187D8(u32 ev, int a, int b, void *rec) {
    switch (ev) {
        case 2: _0800187B4(rec, (void *)(uintptr_t)a, b); break;
        case 1: _08001876C(rec); break;
        case 5: _08001878C(rec); break;
        case 6: _0800187B0(rec, (int)(u16)a, (int)(u16)b); break;
        default: break;
    }
}
#ifndef __APPLE__
void _0800187D8(u32 ev, int a, int b, void *rec)
    __attribute__((alias("Rec35_Dispatch_187D8")));
#endif

// _08001881C: sub_08007664(0,0x08292B40,10); u32[ctx]=150; u32[ctx+4]=0.
void Rec35_Setup_1881C(void *ctx) {
    volatile u8 *c = (volatile u8 *)ctx;
    sub_08007664(0, (void *)(uintptr_t)0x08292B40u, 10);
    *(volatile u32 *)(c + 0) = 150;
    *(volatile u32 *)(c + 4) = 0;
}
#ifndef __APPLE__
void _08001881C(void *ctx) __attribute__((alias("Rec35_Setup_1881C")));
#endif

// _08001883C: countdown twin of _08001878C — same 36-byte ROM span, same
// parameter-derived shape (see the note above it).
void Rec35_Countdown_1883C(void *ctx) {
    s32 n = *(volatile s32 *)ctx;
    if (n > 0) {
        *(volatile s32 *)ctx = n - 1;
    } else if (*(volatile s32 *)((u8 *)ctx + 4) == 0) {
        *(volatile s32 *)((u8 *)ctx + 4) = 1;
        _08004EC0(1);
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001883C(void *ctx) __attribute__((alias("Rec35_Countdown_1883C")));
#endif

// _080018860: bx lr stub (incoming rec/u16/u16 all dead).
void Rec35_BxLr_18860(void *a, int b, int c) { (void)a; (void)b; (void)c; }
#ifndef __APPLE__
void _080018860(void *a, int b, int c) __attribute__((alias("Rec35_BxLr_18860")));
void sub_080018860(void *a, int b, int c) __attribute__((alias("Rec35_BxLr_18860")));
#endif

// _080018864: interior BL entry — u16[base+84]=1 (r0/c dead).
void Rec35_Leaf_18864(void *rec, void *base, int c) {
    (void)rec; (void)c;
    *(volatile u16 *)((volatile u8 *)base + 84) = 1;
}
#ifndef __APPLE__
void _080018864(void *a, void *b, int c) __attribute__((alias("Rec35_Leaf_18864")));
#endif

// _08001886C: cmp-chain dispatcher twin (2->864, 1->81C, 5->83C, 6->860).
// u32 ev -> `bhi` range test, matching the ROM and its 187D8 twin.
void Rec35_Dispatch_1886C(u32 ev, int a, int b, void *rec) {
    switch (ev) {
        case 2: _080018864(rec, (void *)(uintptr_t)a, b); break;
        case 1: _08001881C(rec); break;
        case 5: _08001883C(rec); break;
        case 6: _080018860(rec, (int)(u16)a, (int)(u16)b); break;
        default: break;
    }
}
#ifndef __APPLE__
void _08001886C(u32 ev, int a, int b, void *rec)
    __attribute__((alias("Rec35_Dispatch_1886C")));
#endif

// _0800188B0: menu record-type switch over s16[rec]. sp frame 32 B (r5=rec).
void Rec35_RecordSwitch_188B0(void *rec) {
    volatile u8 *c = (volatile u8 *)rec;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    if (*(volatile s16 *)(c + 0) == -1) {
        u8 spb[32];
        _08002E0A4(spb, (const void *)(uintptr_t)0x0805FB66u, 30);
        s16 idx = *(volatile s16 *)(c + 2);
        u16 v = (idx > 14) ? *(volatile u16 *)(spb + 28)
                           : *(volatile u16 *)(spb + idx * 2);
        *(volatile s16 *)(c + 0) = (s16)v;
        *(volatile u32 *)(c + 12) = 0x2328u;
        *(volatile u8 *)(wa + 0x10C1) = 1;
    } else {
        *(volatile u8 *)(wa + 0x10C1) = 0;
        *(volatile u16 *)(wa + 0x10C6) = *(volatile u16 *)(c + 6);
    }
    s16 t = *(volatile s16 *)(c + 0);
    if (t == 4) {
        s16 w = *(volatile s16 *)(c + 24);
        *(volatile s16 *)(c + 4) = (s16)Sub_080024E7C((u16)w);
    } else if (t == 0) {
        *(volatile u16 *)(c + 2) = 0;
        *(volatile u16 *)(c + 8) = 0;
    } else if (t == 9) {
        int r = _08005B5C(_08008014());
        u16 lane = (u16)((r & 31) + 16);
        *(volatile u16 *)(c + 2) = lane;
        s16 cup = _080258B8((int)lane);
        if (cup == 1)
            *(volatile u16 *)(c + 8) = 2;
        for (;;) {
            int car = _0802D97C(_08005B5C(_08008014()), 98);
            *(volatile s16 *)(c + 24) = (s16)car;
            s16 v = (s16)car;
            if (v == 86 || v == 56 || v == 55 || v == 31 || v == 14 ||
                v == 98 || v == 54 || v == 94 || v == 95 || v == 96)
                continue;
            if ((s16)_080022E4((int)v) == -1)
                continue;
            break;
        }
        int r4 = _08005B5C(_08008014());
        s16 lv = (s16)_080254E8((int)*(volatile s16 *)(c + 24));
        *(volatile u8 *)(c + 29) = (u8)_0802D97C(r4, (int)lv);
    } else if (t == 10) {
        *(volatile u8 *)(wa + 0x10C1) = 0;
        *(volatile u16 *)(c + 8) = 0;
        *(volatile u16 *)(c + 2) = 48;
        *(volatile u16 *)(c + 26) = 0;
    } else if (t == 6) {
        *(volatile s16 *)(c + 0) = 3;
    } else if (t == 7) {
        *(volatile s16 *)(c + 0) = 4;
        s16 w = *(volatile s16 *)(c + 24);
        *(volatile s16 *)(c + 4) = (s16)Sub_080024E7C((u16)w);
    }
    if ((s16)_080022E4((int)*(volatile s16 *)(c + 24)) == -1) {
        *(volatile u16 *)(c + 24) = 10;
        *(volatile u8 *)(c + 29) = 0;
    }
    _08002E0A4((void *)(wa + 0x10FC), rec, 56);
}
#ifndef __APPLE__
void _0800188B0(void *rec) __attribute__((alias("Rec35_RecordSwitch_188B0")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void UiPacket_Consume(void *rec) __attribute__((alias("Rec35_RecordSwitch_188B0")));
void _080188B0(void *rec) __attribute__((alias("Rec35_RecordSwitch_188B0")));
void sub_080188B0(void *rec) __attribute__((alias("Rec35_RecordSwitch_188B0")));
#endif
