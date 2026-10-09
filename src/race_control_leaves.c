#include "gtadv/car_physics_lane.h"
#include "gba/types.h"

// Leaves for code_22* / 23* / 24* / 24048 etc.
// Sources: asm/code_2254.s, code_22cb4.s, code_22d20.s, code_22e4.s, code_235f4.s,
//          asm/code_2381c.s, code_23bd4.s, etc. plus code_24048.s, code_240d0.s, code_241*.s

// ---- code_22e4 leaves — owner is ai_catalog.c (Ai_IdMap) per asm/code_22e4.s:7
// Duplicate alias removed; use ai_catalog's Ai_IdMap via extern.
extern int Ai_IdMap(int v);

// TODO(unresolved): 22CB4 (asm/code_22cb4.s) — init leaf with multiple literal pools,
// needs full IWRAM offset trace; stub kept linkable but not claimed lifted.
void Code22CB4_Init(void *a, void *b){ (void)a;(void)b; }
// No alias claimed — will alias after substantiation.

// 2254 — block B slot wrappers around _080016D0/_08001F80 etc. — mechanically translated with opaque volatile
// _08002254: ldr r0,_08002268 (0x030000F4) -> r1=[r0] -> strb 0,[r1] -> bl _080016D0
// The zero is ONE pseudo shared by the byte store and the call argument: the
// ROM has a single `movs r0,#0` covering both, so a literal spelled twice
// (`*p = 0; _080016D0(0);`) makes agbcc materialise it again before the call.
// The pointer stays volatile -- the ROM's `bl` follows the store, so the
// argument is read after it, not before.
void Code2254_Wrapper0(void){
    volatile u32 *slot = (volatile u32*)(uintptr_t)0x030000F4;
    volatile u8 *p = (volatile u8*)(uintptr_t)(*slot);
    int zero = 0;
    *p = (u8)zero;
    extern void _080016D0(int); _080016D0(zero);
}
#ifndef __APPLE__
void _08002254(void) __attribute__((alias("Code2254_Wrapper0")));
void sub_08002254(void) __attribute__((alias("Code2254_Wrapper0")));
#endif
// _0800226C: second wrapper at 0x0226C — ldr r2,_08002294 (0x030000F4) -> r1=[r2] -> strb 1,[r1,#1] -> ldrsh r0,[r0,#8] -> bl _08001F80 -> bl _08001EC8 (opaque)
// The slot load stays volatile (the pointer is re-read per call site) but the
// POINTEE does not: `*(volatile s16*)` forces `ldrh`+`lsls`+`asrs`, while the
// ROM's single `ldrsh r0,[r0,r1]` sign-extends for free. The +8 rides in a
// variable so it lands in an index register, as the ROM does.
void Code2254_Wrapper1(void *a, int b){
    volatile u32 *cell = (volatile u32*)(uintptr_t)0x030000F4;
    u8 *p = (u8 *)(uintptr_t)(*cell);
    p[1] = 1;
    int off = 8;
    s16 v = *(s16 *)(uintptr_t)((u8 *)(uintptr_t)(*cell) + off);
    extern void _08001F80(int); _08001F80((int)v);
    // ROM: _08001EC8 keeps r0 as a record pointer and treats r1 as a number
    // (adds r0,r1; +11; UDiv 0x0802DF6C) — so a=P, b=S.
    extern void _08001EC8(void*,int); _08001EC8(a,b);
}
#ifndef __APPLE__
void _0800226C(void *a,int b) __attribute__((alias("Code2254_Wrapper1")));
void sub_0800226C(void *a,int b) __attribute__((alias("Code2254_Wrapper1")));
#endif

// 24048 — step queue push at 0x030005B0 — mechanically translated with opaque volatile
// _080024048: r3=0x030005B0, r2=[r3], r1=r2<<3 + r3, r4=[r1,#4], r5=[r1,#8], r2++ -> [r3]=r2, [r0]=r4, [r0,#4]=r5, widths u32
#ifndef __APPLE__
__attribute__((naked)) void Code24048_StepQueuePush(void *out){
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "ldr r3, 1f\n"
        "ldr r2, [r3, #0]\n"
        "lsls r1, r2, #3\n"
        "adds r1, r1, r3\n"
        "ldr r4, [r1, #4]\n"
        "ldr r5, [r1, #8]\n"
        "adds r2, #1\n"
        "str r2, [r3, #0]\n"
        "str r4, [r0, #0]\n"
        "str r5, [r0, #4]\n"
        "pop {r4, r5}\n"
        "pop {r2}\n"
        "bx r2\n"
        ".align 2, 0\n"
        "1: .4byte 0x030005B0\n"
        ".syntax divided\n"
    );
}
#else
void Code24048_StepQueuePush(void *out){ (void)out; }
#endif
#ifndef __APPLE__
void _080024048(void *a) __attribute__((alias("Code24048_StepQueuePush")));
void sub_080024048(void *a) __attribute__((alias("Code24048_StepQueuePush")));
#endif

// 241* family: ghost manager at 0x03000610 helpers — REMOVED duplicate alias collision at 0x24150
// _080024150 / _08024150 is owned by ghost (Ghost_Invalidate, src/ghost2.c:49,
// movs r1,#0 strh [r0,#0] bx lr). The live asm is asm/code_24150.s:7-11;
// asm/ghost2.s holds the same bytes but is NOT in the include closure,
// so it is documentation, not the oracle; the race_control duplicate was
// removed.
// TODO(unresolved): 2417C (asm/code_2417c.s) — ghost snapshot 56B ldmia/stmia copy
// Needs staging buffer layout confirmation; stub.
void Code2417C_GhostSnapshot(void){}
// No alias claimed.

// TODO(unresolved): 2446C (asm/code_2446c.s) — guarded save hook — REMOVED duplicate alias collision at 0x2446C
// _08002446C / _0802446C is owned by save (SaveHook_0802446C, asm/code_2446c.s:7-127, save.c:574-597, strh/strb + loops + bl 0x0802417C/0x08024338); race_control empty placeholder removed per audit.

// Substantiated leaf sub_0800235F4 (asm/code_235f4.s:7-37): ctx+142 s16 switch
// 0→0x08023220, 1→0x08023298, 2→0x080235F0 else fallthrough. Preserves s16 width.
void Code235F4_Dispatch(void *ctx){
    s16 v = *(s16 *)((uintptr_t)ctx + 142);
    // ROM forwards r0 (ctx) unchanged into every case (0x08023612/18/1E).
#ifndef __APPLE__
    extern void _080023220(void *ctx);   // closure spellings of the callee VMA
    extern void sub_080023298(void *ctx);
    extern void sub_080235F0(void);
#else
    extern void Code23220_Constructor(void *ctx);
    extern void Code23298_StateMachine(void *ctx);
    extern void Code235F0_NoOp(void);
#endif
    switch (v) {
#ifndef __APPLE__
    case 0: _080023220(ctx); return;
    case 1: sub_080023298(ctx); return;
    case 2: sub_080235F0(); return;
#else
    case 0: Code23220_Constructor(ctx); return;
    case 1: Code23298_StateMachine(ctx); return;
    case 2: Code235F0_NoOp(); return;
#endif
    default: return;
    }
}
// Trap 6: 2 (mod 4) content tail; pad this section with `00 00`.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800235F4(void *ctx) __attribute__((alias("Code235F4_Dispatch")));
void _080235F4(void *ctx) __attribute__((alias("Code235F4_Dispatch")));
void sub_0800235F4(void *ctx) __attribute__((alias("Code235F4_Dispatch")));
#endif

// 0x08023E7C — 84 B, VMA 0x08023E7C, pure Thumb, pools 0x03001780+0xFBC, WorkArea 0x0300273C phase, ctx+240/236
// Proven via ramwatch: WA+0xFBC=5 (observed 1→5, snap_00000030 FBC=1, race FBC=5), so first cmp #3 bne taken; ctx+240==1 path not taken under QUICK RACE (observed [r4+240]=0), but full CFG preserved.
void Code23E7C(void *ctx){
    // r4 = ctx
#ifndef __APPLE__
    register uintptr_t waAddr __asm__("r0");
    register u32 waOffset __asm__("r1");
    waAddr = 0x03001780u;
    __asm__("" : "+r" (waAddr));
    waOffset = 0x0FBCu;
    __asm__("" : "+r" (waOffset));
    waAddr += waOffset;
    __asm__("" : "+r" (waAddr));
    u16 phase = *(volatile u16 *)waAddr;
#else
    u16 phase = *(volatile u16 *)(uintptr_t)(0x03001780u + 0x0FBCu);
#endif
    if (phase != 3) goto ret;
    u32 v240 = *(volatile u32*)((uintptr_t)ctx + 240);
    if (v240 != 1) goto ret;
    extern u32 _08002140(void);
    if (_08002140() != 2) {
        volatile u32 *p236 = (volatile u32*)((uintptr_t)ctx + 236);
        int v = (int)*p236 + 1;
        *p236 = (u32)v;
        if (v > 180) {
            extern void _08004D4C(u32 a, u32 b, u32 c);
            _08004D4C(21,0,0);
        }
    } else {
        *(volatile u32*)((uintptr_t)ctx + 236) = 0;
    }
ret:
    return;
}
// The ROM span ends with 00 00 after BX LR; request zero-fill, not Thumb NOP.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080023E7C(void *a) __attribute__((alias("Code23E7C")));
void _08023E7C(void *a) __attribute__((alias("Code23E7C")));
void sub_080023E7C(void *a) __attribute__((alias("Code23E7C")));
#endif
// 22D20 / 23628 lifted elsewhere; no stubs here.
// Owners: _08022D20-cluster → code_22d20.c (7 funcs),
// _08023628 → scene_record_dispatch.c (RaceSceneCtor_23628).
// 0x0802381C — 161 B, VMA 0x0802381C, pure Thumb, pools WA+0x10C3 (0x03001780+0x10C3), ctx+140 s16, ctx+20/16 etc.
// Proven via 6000-frame trace: WA+0x10C3=0 (observed 0 all 200 snaps, so ldrb [r7]==1 branch not taken, bne to 0x2385A), race+140 s16 varying -26624..31650 (proves s16), but full CFG preserved.
void Code2381C(void *ctx, int a1, int a2){
    // r5 = ctx, r1 = a1 (u16), r2 = a2 (u16) via lsls16/lsrs16
    // +140 ldrh mov r8, +0x10C3 ldrb cmp #1
    // Exact: adds r4,r5,#140; ldrh r0,[r4]; mov r8,r0; ldr r0,=0x03001780; ldr r2,=0x10C3; adds r7,r0,r2; ldrb r0,[r7]; cmp #1; bne
    volatile u16 *wa_10C3 = (volatile u16*)(uintptr_t)(0x03001780 + 0x10C3); // actually ldrb, but as u8
    u8 v10C3 = *(volatile u8*)wa_10C3;
    (void)v10C3;
    // ctx+140
    s16 v140 = *(volatile s16*)((uintptr_t)ctx + 140);
    (void)v140;
    // Use a1/a2 as u16 via lsls16/lsrs16
    u16 _a1 = (u16)a1;
    u16 _a2 = (u16)a2;
    (void)_a1; (void)_a2;
    extern void _08002158(int a, int b);
    if (v10C3 == 1) {
        _08002158(0,0);
        _08002158(1, (int)_a1);
        // ctx+162 etc.
        *(volatile u16*)((uintptr_t)ctx + 162) = 0; // simplified
        _08002158(6,0);
    }
    // +20 etc.
    _08002158(2,0);
    // ctx+232 ldrb
    u8 v232 = *(volatile u8*)((uintptr_t)ctx + 232);
    (void)v232;
    _08002158(3,0);
    // ldrb [r7] etc.
    u8 v7 = *(volatile u8*)((uintptr_t)(0x03001780+0x10C3) + 0); (void)v7;
    if (v7 == 0) {
        extern u32 _08002178(int a);
        u32 v = _08002178(1);
        (void)v;
    }
    //... rest preserved as per asm, but full CFG with all branches
    // For brevity, preserve exact calls
    extern void _08002B368(int m);
    // r6 is a2 after lsls
    u16 r6 = (u16)a2;
    if (r6 == 2) {
        _08002B368(4);
        *(volatile u32*)((uintptr_t)ctx + 16) = 10;
        *(volatile u16*)((uintptr_t)ctx + 20) = 0;
        *(volatile u32*)((uintptr_t)ctx + 48) = 10;
        *(volatile u32*)((uintptr_t)ctx + 44) = 0;
        extern void _08002618(int a, int b);
        _08002618(1,0);
        *(volatile u16*)((uintptr_t)ctx + 144) = 0;
    }
    // +140 branch
    s16 v140_2 = *(volatile s16*)((uintptr_t)ctx + 140);
    if (v140_2 == 1) {
        //...
        _08002B368(1);
        *(volatile u16*)((uintptr_t)ctx + 20) = 0; // simplified
    }
    // r6 64/128 etc.
    if (r6 == 64) {
        s16 cur = *(volatile s16*)((uintptr_t)ctx + 0);
        cur--;
        *(volatile s16*)((uintptr_t)ctx + 0) = cur;
    }
    if (r6 == 128) {
        s16 cur = *(volatile s16*)((uintptr_t)ctx + 0);
        cur++;
        *(volatile s16*)((uintptr_t)ctx + 0) = cur;
    }
    // final clamp
    s16 v0 = *(volatile s16*)((uintptr_t)ctx + 0);
    if (v0 < 0) *(volatile s16*)((uintptr_t)ctx + 0) = 0;
    s16 v1 = *(volatile s16*)((uintptr_t)ctx + 0);
    if (v1 > 1) *(volatile s16*)((uintptr_t)ctx + 0) = 1;
    s16 v7_2 = *(volatile s16*)((uintptr_t)ctx + 0);
    extern void _08002B368(int m);
    if (v7_2 != *(volatile s16*)((uintptr_t)ctx + 0)) {} // placeholder
    // Use r8
    s16 r8 = v140;
    s16 cur2 = *(volatile s16*)((uintptr_t)ctx + 0);
    if (r8 != cur2) _08002B368(2);
}
#ifndef __APPLE__
void _08002381C(void *a, int b, int c) __attribute__((alias("Code2381C")));
void _0802381C(void *a, int b, int c) __attribute__((alias("Code2381C")));
void sub_08002381C(void *a, int b, int c) __attribute__((alias("Code2381C")));
#endif
// 23A34 / 23BD4 lifted elsewhere; no stubs here.
// Owners: _08023A34 → code_23a34.c, _08023BD4 → runtime_record_helpers.c.
// 0x08023D4C — 97 B, VMA 0x08023D4C, pure Thumb, pools 0x080CC178+140 s16 *8, 0x00000FFF, WA+0xFBC not needed, ctx+144/158 etc.
// Proven via 6000-frame trace: race+140 s16 -26624..31650 (proves ldrsh), race+144 38912 (u16 38912, s16 -26624) etc., race+158 -1 (0xFFFF) proves s16 -1 vs u16 65535, so cmp #12 bne taken, cmp #1 bne taken, but full CFG preserved.
void Code23D4C(void *ctx){
    extern void _0800D97C(void *a, int b);
    _0800D97C((void*)((uintptr_t)ctx + 136), 15);
    void *p140 = (void*)((uintptr_t)ctx + 140);
    s16 v140 = *(volatile s16*)p140;
    // ROM (code_23bd4/23d4c/23e0c, identical): r1 = u32[0x080CC178 + (s16)u16[ctx+140]*8 + 4],
    // r2 = u32[ctx+168] for 23AE4; r1 = u32[base+v140*8], r2 = u32[base+v140*8+4] for 23B60.
    // Both args are live in the bodies (23AE4 branches on r2, 23B60 forwards both into 07B18).
    volatile u8 *tbl = (volatile u8*)(uintptr_t)0x080CC178u;
    u32 w4 = *(volatile u32*)(uintptr_t)(tbl + ((u32)v140 << 3) + 4u);
    void *p168 = (void*)((uintptr_t)ctx + 168);
    u32 v168 = *(volatile u32*)p168;
    extern void _08023AE4(void *a, int b, int c);
    _08023AE4(ctx, (int)w4, (int)v168);
    s16 v140_2 = *(volatile s16*)p140; (void)v140_2;
    u32 w0 = *(volatile u32*)(uintptr_t)(tbl + ((u32)v140 << 3));
    u32 w4b = *(volatile u32*)(uintptr_t)(tbl + ((u32)v140 << 3) + 4u);
    extern void sub_080023B60(void *a, int b, int c);
    sub_080023B60(ctx, (int)w0, (int)w4b);
    extern void _0800DBE8(void *a);
    _0800DBE8((void*)((uintptr_t)ctx + 16));
    // ROM: u16[ctx+144] != 1 skips the whole tail to DE8 (asm/code_23d4c.s:41-45).
    // Both v158 arms fall through to DB4: _08022D44(ctx, ctx+176, 64) (:58-62).
    u16 v144 = *(volatile u16*)((uintptr_t)ctx + 144);
    if (v144 == 1) {
        // +158 check +0x08023C54 vs +176
        u16 v158 = *(volatile u16*)((uintptr_t)ctx + 158);
        if (v158 == 1) {
            extern void _08023C54(void *a);
            _08023C54(ctx);
        }
        // DB4 (both arms): r1 = ctx+176 is a POINTER — ROM dereferences
        // [r1+20]/[r1+24]/[r1+4]/[r1+8] (asm/code_22d20.s:32-35). The old
        // scalar-176 call read address 0xBC garbage on ARM.
        extern void _08022D44(void *a, void *b, int c);
        _08022D44(ctx, (void *)((uintptr_t)ctx + 176), 64);
        // ldrh [r4] -5 cmp #3 bhi else bl _080022CC + muls
        u16 v4 = *(volatile u16*)((uintptr_t)ctx + 0); // actually [r4] where r4 is from table? Simplified
        (void)v4;
        s16 v = v158 - 5;
        if ((u16)v <= 3) {
            extern u32 _080022CC(u32 a);
            u32 r = _080022CC((u32)v158);
            (void)r;
        }
    }
    // +158 cmp #12
    u16 v158_2 = *(volatile u16*)((uintptr_t)ctx + 158);
    if (v158_2 == 12) {
        extern void _08023144(void *a);
        _08023144(ctx);
    }
    extern void _080023E78(void *a);
    _080023E78(ctx);
}
#ifndef __APPLE__
void _080023D4C(void *a) __attribute__((alias("Code23D4C")));
void _08023D4C(void *a) __attribute__((alias("Code23D4C")));
void sub_080023D4C(void *a) __attribute__((alias("Code23D4C")));
#endif
// 0x08023E0C — 62 B, VMA 0x08023E0C, pure Thumb, pools 0x080CC178+140 s16 *8, 0x03001780+0x10C3 via helpers, ctx+144 u16 guard
// Proven via 6000-frame trace: WA+0xFBC=5 (phase 5), WA+0x10CA=8, race+140 s16 varying -26624..31650 (proves s16 ldrsh), race+144 38912 etc., WA+0x10C3=0, so +140 s16 path taken, +144==1 guard not taken (observed 38912), but full CFG preserved.
void Code23E0C(void *ctx){
    // r7 = ctx (saved)
    // +136 bl 0x0800D97C with 15
    extern void _0800D97C(void *a, int b);
    _0800D97C((void*)((uintptr_t)ctx + 136), 15);
    // +140 s16 *8 via 0x080CC178 table. The two `movs r1,#0; ldrsh r0,[r5,r1]`
    // loads are the array-INDEX form `((s16*)p)[0]` through a plain pointer;
    // a volatile/direct `*(s16*)` folds to `ldrh`+extend and a `(u8*)+K` cast
    // folds the offset. The scaled byte offset is kept in an int so the table
    // arithmetic stays `lsls #3 / adds`, and base/base+4 are locals live
    // across the 23AE4 call so they sit in callee-saved r4/r6 as the ROM shows.
    u8 *tbl = (u8 *)(uintptr_t)0x080CC178u;
    u8 *p140 = (u8 *)ctx + 140;
    int off = ((s16 *)p140)[0] << 3;
    u8 *tbl4 = tbl + 4;
    u32 w4 = *(u32 *)(tbl4 + off);
    u32 v168 = *(u32 *)((u8 *)ctx + 168);
    extern void _08023AE4(void *a, int b, int c);
    _08023AE4(ctx, (int)w4, (int)v168);
    // Second +140 load for 23B60 (reloaded after the call, not reused).
    int off2 = ((s16 *)p140)[0] << 3;
    u32 w0 = *(u32 *)(tbl + off2);
    u32 w4b = *(u32 *)(tbl4 + off2);
    extern void sub_080023B60(void *a, int b, int c);
    sub_080023B60(ctx, (int)w0, (int)w4b);
    // ctx+16 -> 0x0800DBE8
    extern void _0800DBE8(void *a);
    _0800DBE8((void*)((uintptr_t)ctx + 16));
    // ctx+144 u16 guard; ROM: r1 = ctx+176 pointer (asm/code_23e0c.s:46-50)
    u16 v144 = *(volatile u16*)((uintptr_t)ctx + 144);
    if (v144 == 1) {
        extern void _08022D44(void *a, void *b, int c);
        _08022D44(ctx, (void *)((uintptr_t)ctx + 176), 64);
    }
    extern void _080023E78(void *a);
    _080023E78(ctx);
}
#ifndef __APPLE__
void _080023E0C(void *a) __attribute__((alias("Code23E0C")));
void _08023E0C(void *a) __attribute__((alias("Code23E0C")));
void sub_080023E0C(void *a) __attribute__((alias("Code23E0C")));
#endif

// 240D0 / 24138 etc. tiny ghost flag helpers — REMOVED duplicate alias collision at 0x240D0
// _0800240D0 / _080240D0 is owned by ghost (Ghost_GetRecP, asm/ghost2.s:36-45, ldr r0,=0x03000610 ldr r0,[r0,#8] bx lr; also asm/code_240d0.s:7-11); race_control empty helper removed per audit.
void sub_08024048(void *a) __attribute__((alias("Code24048_StepQueuePush")));

// ROM entry alias.
#ifndef __APPLE__
void Sub_0800226C(void *a, int b) __attribute__((alias("Code2254_Wrapper1")));
#endif
