#include "gtadv/car_physics_lane.h"
#include "gba/types.h"

// Leaves for code_22* / 23* / 24* / 24048 etc.
// Sources: asm/code_2254.s, code_22cb4.s, code_22d20.s, code_22e4.s, code_235f4.s,
//          asm/code_2381c.s, code_23bd4.s, etc. plus code_24048.s, code_240d0.s, code_241*.s

// ---- code_22e4 leaves — owner is ai_catalog.c (Ai_IdMap) per asm/code_22e4.s:7
// Duplicate alias removed; use ai_catalog's Ai_IdMap via extern.
extern int Ai_IdMap(int v);

// 0x08022CB4 is implemented as CarRecEventReset_22CB4 in
// runtime_state_dispatch.c; do not keep the former unreferenced placeholder.

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
void _08002254(void) __attribute__((alias("Code2254_Wrapper0")));
void sub_08002254(void) __attribute__((alias("Code2254_Wrapper0")));
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
void _0800226C(void *a,int b) __attribute__((alias("Code2254_Wrapper1")));
void sub_0800226C(void *a,int b) __attribute__((alias("Code2254_Wrapper1")));

// 24048 — dequeue a two-word step pair from the count-prefixed queue at
// 0x030005B0. The C struct-return ABI supplies the result buffer in r0;
// sub_08024048 is also declared as an explicit output-buffer alias for the
// existing caller so its emitted stack sequence stays unchanged.
typedef struct {
    u32 preceding_word;
    Code24048StepPair value;
} Code24048QueueWindow;

Code24048StepPair Code24048_StepQueuePop(void)
{
    register volatile Code24048QueueWindow *queue __asm__("r3");
    register u32 index __asm__("r2");
    register volatile Code24048QueueWindow *slot __asm__("r1");
    Code24048StepPair result;

    queue = (volatile Code24048QueueWindow *)(uintptr_t)0x030005B0u;
    index = queue->preceding_word;
    slot = (volatile Code24048QueueWindow *)((index << 3) + (uintptr_t)queue);
    result = slot->value;
    queue->preceding_word = index + 1u;
    return result;
}
Code24048StepPair _080024048(void) __attribute__((alias("Code24048_StepQueuePop")));
Code24048StepPair sub_080024048(void) __attribute__((alias("Code24048_StepQueuePop")));

// 241* family: ghost manager at 0x03000610 helpers — REMOVED duplicate alias collision at 0x24150
// _080024150 / _08024150 is owned by ghost (Ghost_Invalidate, src/ghost2.c:49,
// movs r1,#0 strh [r0,#0] bx lr). The live asm is asm/code_24150.s:7-11;
// asm/ghost2.s holds the same bytes but is NOT in the include closure,
// so it is documentation, not the oracle; the race_control duplicate was
// removed.
// 0x0802417C is implemented as Ghost_Snapshot in ghost2.c; the earlier empty
// placeholder is intentionally absent so it cannot be mistaken for coverage.

// 0x0802446C is owned by save (SaveHook_0802446C in save.c); duplicate
// race-control aliases were removed to leave one source owner.

// Substantiated leaf sub_0800235F4 (asm/code_235f4.s:7-37): ctx+142 s16 switch
// 0→0x08023220, 1→0x08023298, 2→0x080235F0 else fallthrough. Preserves s16 width.
void Code235F4_Dispatch(void *ctx){
    s16 v = *(s16 *)((uintptr_t)ctx + 142);
    // ROM forwards r0 (ctx) unchanged into every case (0x08023612/18/1E).
    extern void _080023220(void *ctx);   // closure spellings of the callee VMA
    extern void sub_080023298(void *ctx);
    extern void sub_080235F0(void);
    switch (v) {
    case 0: _080023220(ctx); return;
    case 1: sub_080023298(ctx); return;
    case 2: sub_080235F0(); return;
    default: return;
    }
}
// Trap 6: 2 (mod 4) content tail; pad this section with `00 00`.
__asm__(".align 2, 0");
void _0800235F4(void *ctx) __attribute__((alias("Code235F4_Dispatch")));
void _080235F4(void *ctx) __attribute__((alias("Code235F4_Dispatch")));
void sub_0800235F4(void *ctx) __attribute__((alias("Code235F4_Dispatch")));

// 0x08023E7C — 84 B, VMA 0x08023E7C, pure Thumb, pools 0x03001780+0xFBC, WorkArea 0x0300273C phase, ctx+240/236
// Proven via ramwatch: WA+0xFBC=5 (observed 1→5, snap_00000030 FBC=1, race FBC=5), so first cmp #3 bne taken; ctx+240==1 path not taken under QUICK RACE (observed [r4+240]=0), but full CFG preserved.
void Code23E7C(void *ctx){
    // r4 = ctx
    register uintptr_t waAddr __asm__("r0");
    register u32 waOffset __asm__("r1");
    waAddr = 0x03001780u;
    __asm__("" : "+r" (waAddr));
    waOffset = 0x0FBCu;
    __asm__("" : "+r" (waOffset));
    waAddr += waOffset;
    __asm__("" : "+r" (waAddr));
    u16 phase = *(volatile u16 *)waAddr;
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
void _080023E7C(void *a) __attribute__((alias("Code23E7C")));
void _08023E7C(void *a) __attribute__((alias("Code23E7C")));
void sub_080023E7C(void *a) __attribute__((alias("Code23E7C")));
// 22D20 / 23628 lifted elsewhere; no stubs here.
// Owners: _08022D20-cluster → code_22d20.c (7 funcs),
// _08023628 → scene_record_dispatch.c (RaceSceneCtor_23628).
// 0x0802381C — 161 B, VMA 0x0802381C, pure Thumb, pools WA+0x10C3 (0x03001780+0x10C3), ctx+140 s16, ctx+20/16 etc.
// Proven via 6000-frame trace: WA+0x10C3=0 (observed 0 all 200 snaps, so ldrb [r7]==1 branch not taken, bne to 0x2385A), race+140 s16 varying -26624..31650 (proves s16), but full CFG preserved.
__attribute__((naked)) void Code2381C(void *ctx, int a1, int a2) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, r6, r7, lr}\n"
        "mov	r7, r8\n"
        "push	{r7}\n"
        "adds	r5, r0, #0\n"
        "lsls	r1, r1, #16\n"
        "lsrs	r1, r1, #16\n"
        "lsls	r2, r2, #16\n"
        "lsrs	r6, r2, #16\n"
        "adds	r4, r5, #0\n"
        "adds	r4, #140\n"
        "ldrh	r0, [r4, #0]\n"
        "mov	r8, r0\n"
        "ldr r0, .Lcode2381c_080023880\n"
        "ldr r2, .Lcode2381c_080023884\n"
        "adds	r7, r0, r2\n"
        "ldrb	r0, [r7, #0]\n"
        "cmp	r0, #1\n"
        "bne .Lcode2381c_08002385A\n"
        "movs	r0, #0\n"
        "bl 0x08002158\n"
        "movs	r0, #1\n"
        "adds	r1, r6, #0\n"
        "bl 0x08002158\n"
        "adds	r0, r5, #0\n"
        "adds	r0, #162\n"
        "ldrh	r1, [r0, #0]\n"
        "movs	r0, #6\n"
        "bl 0x08002158\n"
        ".Lcode2381c_08002385A:\n"
        "ldrh	r1, [r4, #0]\n"
        "movs	r0, #2\n"
        "bl 0x08002158\n"
        "adds	r0, r5, #0\n"
        "adds	r0, #232\n"
        "ldrb	r1, [r0, #0]\n"
        "movs	r0, #3\n"
        "bl 0x08002158\n"
        "ldrb	r0, [r7, #0]\n"
        "cmp	r0, #0\n"
        "bne .Lcode2381c_080023888\n"
        "movs	r0, #1\n"
        "movs	r1, #0\n"
        "bl 0x08002178\n"
        "movs	r0, #1\n"
        "b .Lcode2381c_080023892\n"
        ".Lcode2381c_080023880: .word 0x03001780\n"
        ".Lcode2381c_080023884: .word 0x000010C3\n"
        ".Lcode2381c_080023888:\n"
        "movs	r0, #0\n"
        "movs	r1, #0\n"
        "bl 0x08002178\n"
        "movs	r0, #0\n"
        ".Lcode2381c_080023892:\n"
        "movs	r1, #1\n"
        "bl 0x08002178\n"
        "adds	r6, r0, #0\n"
        "cmp	r6, #2\n"
        "bne .Lcode2381c_0800238BE\n"
        "movs	r0, #4\n"
        "bl 0x0802B368\n"
        "movs	r0, #10\n"
        "str	r0, [r5, #16]\n"
        "movs	r4, #0\n"
        "strh	r4, [r5, #20]\n"
        "str	r0, [r5, #48]\n"
        "str	r4, [r5, #44]\n"
        "movs	r0, #1\n"
        "movs	r1, #0\n"
        "bl 0x08002618\n"
        "adds	r0, r5, #0\n"
        "adds	r0, #144\n"
        "strh	r4, [r0, #0]\n"
        ".Lcode2381c_0800238BE:\n"
        "adds	r7, r5, #0\n"
        "adds	r7, #140\n"
        "cmp	r6, #1\n"
        "bne .Lcode2381c_080023914\n"
        "movs	r1, #0\n"
        "ldrsh	r4, [r7, r1]\n"
        "cmp	r4, #1\n"
        "beq .Lcode2381c_080023902\n"
        "cmp	r4, #1\n"
        "bgt .Lcode2381c_080023914\n"
        "cmp	r4, #0\n"
        "bne .Lcode2381c_080023914\n"
        "movs	r0, #1\n"
        "bl 0x0802B368\n"
        "adds	r0, r5, #0\n"
        "adds	r0, #168\n"
        "str	r6, [r0, #0]\n"
        "strh	r4, [r5, #20]\n"
        "subs	r0, #44\n"
        "strh	r6, [r0, #0]\n"
        "str	r6, [r5, #44]\n"
        "movs	r0, #1\n"
        "movs	r1, #0\n"
        "bl 0x08002618\n"
        "adds	r0, r5, #0\n"
        "adds	r0, #144\n"
        "strh	r4, [r0, #0]\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r7, r2]\n"
        "bl 0x08004BFC\n"
        "b .Lcode2381c_080023914\n"
        ".Lcode2381c_080023902:\n"
        "movs	r0, #1\n"
        "bl 0x0802B368\n"
        "adds	r0, r5, #0\n"
        "adds	r0, #142\n"
        "movs	r1, #0\n"
        "strh	r6, [r0, #0]\n"
        "adds	r0, #16\n"
        "strh	r1, [r0, #0]\n"
        ".Lcode2381c_080023914:\n"
        "cmp	r6, #64\n"
        "bne .Lcode2381c_08002391E\n"
        "ldrh	r0, [r7, #0]\n"
        "subs	r0, #1\n"
        "strh	r0, [r7, #0]\n"
        ".Lcode2381c_08002391E:\n"
        "cmp	r6, #128\n"
        "bne .Lcode2381c_080023928\n"
        "ldrh	r0, [r7, #0]\n"
        "adds	r0, #1\n"
        "strh	r0, [r7, #0]\n"
        ".Lcode2381c_080023928:\n"
        "adds	r1, r7, #0\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "cmp	r0, #0\n"
        "bge .Lcode2381c_080023936\n"
        "movs	r0, #0\n"
        "strh	r0, [r1, #0]\n"
        ".Lcode2381c_080023936:\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "cmp	r0, #1\n"
        "ble .Lcode2381c_080023942\n"
        "movs	r0, #1\n"
        "strh	r0, [r1, #0]\n"
        ".Lcode2381c_080023942:\n"
        "ldrh	r7, [r7, #0]\n"
        "cmp	r8, r7\n"
        "beq .Lcode2381c_08002394E\n"
        "movs	r0, #2\n"
        "bl 0x0802B368\n"
        ".Lcode2381c_08002394E:\n"
        "pop	{r3}\n"
        "mov	r8, r3\n"
        "pop	{r4, r5, r6, r7}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        ".syntax divided\n"
    );
}
void _08002381C(void *a, int b, int c) __attribute__((alias("Code2381C")));
void _0802381C(void *a, int b, int c) __attribute__((alias("Code2381C")));
void sub_08002381C(void *a, int b, int c) __attribute__((alias("Code2381C")));
// 23A34 / 23BD4 lifted elsewhere; no stubs here.
// Owners: _08023A34 → code_23a34.c, _08023BD4 → runtime_record_helpers.c.
// 0x08023D4C — 192-byte exact C lift, pools 0x080CC178 + s16[ctx+140]*8
// and 0x00000FFF; WA+0xFBC is not needed, ctx+144/158 gate the phase work.
// Proven via 6000-frame trace: race+140 s16 -26624..31650 (proves ldrsh), race+144 38912 (u16 38912, s16 -26624) etc., race+158 -1 (0xFFFF) proves s16 -1 vs u16 65535, so cmp #12 bne taken, cmp #1 bne taken, but full CFG preserved.
void Code23D4C(void *ctx){
    u8 *context = (u8 *)ctx;
    u8 *slot158;
    u32 value;
    s32 product;

    extern void _0800D97C(void *a, int b);
    extern void sub_080023AE4(void *a, int b, int c);
    extern void sub_080023B60(void *a, int b, int c);
    extern void _0800DBE8(void *a);
    extern void sub_080023C54(void *a, int b);
    extern void _080022D44(void *a, void *b, int c);
    extern int _080022CC(void);
    extern void _08003978(int a, int b, int c);
    extern void sub_08023144(void *a);
    extern void sub_080023E78(void *a);

    _0800D97C(context + 136, 15);
    u8 *table = (u8 *)(uintptr_t)0x080CC178u;
    u8 *record140 = context + 140;
    int offset = ((s16 *)record140)[0] << 3;
    u8 *table4 = table + 4;
    u32 first = *(u32 *)(table4 + offset);
    u32 second = *(u32 *)(context + 168);
    sub_080023AE4(context, (int)first, (int)second);

    int offset2 = ((s16 *)record140)[0] << 3;
    u32 secondTableFirst = *(u32 *)(table + offset2);
    u32 secondTableNext = *(u32 *)(table4 + offset2);
    sub_080023B60(context, (int)secondTableFirst, (int)secondTableNext);
    _0800DBE8(context + 16);

    if (*(volatile u16 *)(context + 144) == 1) {
        slot158 = context + 158;
        if (*(volatile u16 *)slot158 == 1)
            sub_080023C54(context, ((s16 *)(context + 166))[0]);
        _080022D44(context, context + 176, 64);
        value = (u16)(*(volatile u16 *)slot158 - 5);
        if (value <= 3) {
            product = _080022CC() * 100;
            if (product < 0)
                product += 0xFFF;
            _08003978(128, 96, 100 - (product >> 12));
        }
    }

    if (*(volatile u16 *)(context + 158) == 12)
        sub_08023144(context);
    sub_080023E78(context);
}
void _080023D4C(void *a) __attribute__((alias("Code23D4C")));
void _08023D4C(void *a) __attribute__((alias("Code23D4C")));
void sub_080023D4C(void *a) __attribute__((alias("Code23D4C")));
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
void _080023E0C(void *a) __attribute__((alias("Code23E0C")));
void _08023E0C(void *a) __attribute__((alias("Code23E0C")));
void sub_080023E0C(void *a) __attribute__((alias("Code23E0C")));

// 240D0 / 24138 etc. tiny ghost flag helpers — REMOVED duplicate alias collision at 0x240D0
// _0800240D0 / _080240D0 is owned by ghost (Ghost_GetRecP, asm/ghost2.s:36-45, ldr r0,=0x03000610 ldr r0,[r0,#8] bx lr; also asm/code_240d0.s:7-11); race_control empty helper removed per audit.
void sub_08024048(void *out) __attribute__((alias("Code24048_StepQueuePop")));

// ROM entry alias.
void Sub_0800226C(void *a, int b) __attribute__((alias("Code2254_Wrapper1")));
