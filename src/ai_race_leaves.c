#include "gtadv/ai_race.h"
#include "gba/types.h"

// ---- ring helpers ----
void Ai_RingReset(void){
    volatile u32 *base=(volatile u32*)(uintptr_t)0x030005B0u;
    base[0]=0;
    base[1]=0xFFFFFFFFu; // -1
    base[2]=0;
}
void _08023FE4(void) __attribute__((alias("Ai_RingReset")));

void Ai_RingPush(int ev, int arg) {
    register int i __asm__("r4") = 0;
    register int *base __asm__("r0");
    register int *b __asm__("r3");
    register int first __asm__("r1");
    register int neg1 __asm__("r2");
    base = (int *)(uintptr_t)0x030005B0u;
    __asm__("" : "+r"(base));
    first = base[1];
    neg1 = -1;
    b = base;
    if (first != neg1) {
        const int *p = b + 1;
        do {
            p += 2;
            i++;
            if (i > 9)
                break;
        } while (*p != neg1);
    }
    {
        int off = i * 8;
        int *base_ev = b + 1;
        *(int *)((uintptr_t)off + (uintptr_t)base_ev) = ev;
        int *base_arg = b + 2;
        *(int *)((uintptr_t)off + (uintptr_t)base_arg) = arg;
        register int next_off __asm__("r0") = (i + 1) * 8;
        *(int *)((uintptr_t)next_off + (uintptr_t)base_ev) = -1;
        *(int *)((uintptr_t)next_off + (uintptr_t)base_arg) = 0;
    }
}
void _08023FF8(int a,int b) __attribute__((alias("Ai_RingPush")));
void Scene_PostEvent(int a,int b) __attribute__((alias("Ai_RingPush")));

// _08023958 — 121B phase pump, 6 BL sites, tier gate wa+0x10C3
void _08023958(void *ctx, int a, int b) {
    register u8 *c __asm__("r5") = (u8 *)ctx;
    u16 rb = (u16)b;
    u16 *p158 = (u16 *)(c + 158);
    u16 *p164;
    register u16 *p162 __asm__("r2");
    extern void _08002158(int, int);
    extern void _0802B368(int);
    extern u8 AiWaBase[];
    extern u8 AiWaOff[];
    __asm__(".globl AiWaBase\nAiWaBase = 0x03001780\n");
    __asm__(".globl AiWaOff\nAiWaOff = 0x10C3\n");

    if ((u16)(*p158 - 5) > 3) {
        _08002158(4, *p158);
    }
    p164 = (u16 *)(c + 164);
    if (*p158 == 1) {
        if (rb == 2) {
            _0802B368(4);
            {
                s16 *p166 = (s16 *)(c + 166);
                if (*p166 == 0)
                    *p166 = rb;
                *p164 = rb;
            }
        }
        if (rb == 1) {
            register s16 *r0_p __asm__("r0") = (s16 *)(c + 166);
            register int val __asm__("r1");
            register s16 *p166 __asm__("r4");
            __asm__("" : "+r"(r0_p));
            val = *r0_p;
            p166 = r0_p;
            if (val == 0)
                *p166 = 2;
            {
                register int v __asm__("r0") = *(u16 *)p166;
                if (v == 1)
                    _0802B368(1);
                else
                    _0802B368(4);
            }
            *p164 = *(u16 *)p166;
        }
    }
    {
        s16 v158 = *(s16 *)(c + 158);
        if (v158 == 3 || v158 == 10 || ((p162 = (u16 *)(c + 162)), v158 == 11)) {
            u16 r = (u16)(rb - 1);
            __asm__("" : "+r"(c));
            p162 = (u16 *)(c + 162);
            if (r <= 1) {
                register u8 *base __asm__("r0") = AiWaBase;
                register u32 off __asm__("r1") = (uintptr_t)AiWaOff;
                if (*(base + off) == 1)
                    *p162 = 1;
            }
        }
    }
    if (rb == 32)
        *(s16 *)(c + 166) = 2;
    if (rb == 16)
        *(s16 *)(c + 166) = 1;
    _08002158(6, *p162);
    _08002158(5, *p164);
    __asm__("" :: "r"(c));
}

// _08023ED0 — 12-way record-42 handler, table at 0x08023EF0
void _08023ED0(int ev, int a, int b, void *ctx) {
    switch (ev) {
        case 2: {
            extern void _08022CB4(void *, int);
            _08022CB4(ctx, a);
            break;
        }
        case 5: {
            extern void _0800D854(void *);
            extern void _0800D8E4(void *);
            extern void sub_080235F4(void *);   /* closure spelling */
            _0800D854((u8 *)ctx + 16);
            _0800D8E4((u8 *)ctx + 120);
            sub_080235F4(ctx);
            break;
        }
        case 7: {
            s16 phase = *(s16 *)((u8 *)ctx + 142);
            switch (phase) {
                case 0: { extern void sub_08023BD4(void *); sub_08023BD4(ctx); break; }
                case 1: { extern void sub_08023D4C(void *); sub_08023D4C(ctx); break; }
                case 2: { extern void sub_08023E0C(void *); sub_08023E0C(ctx); break; }
            }
            break;
        }
        case 6: {
            extern void sub_08023E7C(void *);
            sub_08023E7C(ctx);
            if (*(u16 *)((u8 *)ctx + 20) == 0)
                break;
            {
                s16 phase = *(s16 *)((u8 *)ctx + 142);
                switch (phase) {
                    case 0: { extern void sub_0802381C(void *, int, int); sub_0802381C(ctx, (u16)a, (u16)b); break; }
                    case 1: { _08023958(ctx, (u16)a, (u16)b); break; }
                    case 2: { extern void sub_08023A34(void *, int, int); sub_08023A34(ctx, (u16)a, (u16)b); break; }
                }
            }
            break;
        }
        case 1: {
            extern void sub_08023628(void *);
            sub_08023628(ctx);
            break;
        }
        case 12: {
            extern void _08022D20(void *);
            _08022D20(ctx);
            break;
        }
        default:
            break;
    }
}
__asm__(".align 2, 0");

// _0800AA40 — race FSM, 8-way at +0xFBC — BLOCKED: placeholder 43-event logic incomplete
// Exact asm requires slot field at +0x100A+ b*2+a*8 via ldrsh, tier gate at +0x10E5 (ldrb s8),
// place-change predicates: [sp+8] (slot) vs r8/r9 stacks, wa+0x1074 gate, grid deltas via
// _0800A9A0/_0800A9E0, and 8-way table _0800AAFC (cases 0/1/2/3/5/7 + tails 4/6). Current C
// simplified case 0 fires unconditional _08023FF8(43,0) without wa+0x1074/ slot==10 / r5 vs sl
// checks — not instruction-backed. Leave asm untouched (asm/ai_racefsm.s), no T claim.
// Concrete blocker: need ramwatch of wa+0x1074 s16, wa+0xFF0 one-shot, and stack slot at
// wa+0x28xx before claiming 38 vs 43 vs 49 paths. No weak alias retained.

// _0800B4A8 — collection manager: triples via _08024F4C/_08024F74/_08024F9C, grants via _08024FC4/_08024FE8, grid scans
__attribute__((naked)) void _0800B4A8(void) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Lai_b4a8_4a8:\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r9\n"
        "mov r6, r8\n"
        "push {r6, r7}\n"
        "ldr r7, .Lai_b4a8_54c_lit\n"
        "ldr r0, .Lai_b4a8_550_lit\n"
        "adds r5, r7, r0\n"
        "movs r2, #0\n"
        "ldrsh r1, [r5, r2]\n"
        "lsls r3, r1, #1\n"
        "ldr r6, .Lai_b4a8_554_lit\n"
        "adds r4, r7, r6\n"
        "movs r2, #0\n"
        "ldrsh r0, [r4, r2]\n"
        "lsls r2, r0, #3\n"
        "adds r3, r3, r2\n"
        "adds r6, #24\n"
        "adds r2, r7, r6\n"
        "adds r3, r3, r2\n"
        "movs r6, #0\n"
        "ldrsh r2, [r3, r6]\n"
        "mov r8, r2\n"
        "bl sub_08024F4C\n"
        "lsls r0, r0, #16\n"
        "asrs r6, r0, #16\n"
        "movs r1, #0\n"
        "ldrsh r0, [r4, r1]\n"
        "movs r2, #0\n"
        "ldrsh r1, [r5, r2]\n"
        "mov r2, r8\n"
        "bl sub_08024F74\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #16\n"
        "mov r9, r0\n"
        "movs r3, #0\n"
        "ldrsh r0, [r4, r3]\n"
        "movs r2, #0\n"
        "ldrsh r1, [r5, r2]\n"
        "mov r2, r8\n"
        "bl sub_08024F9C\n"
        "lsls r0, r0, #16\n"
        "asrs r1, r0, #16\n"
        "ldr r3, .Lai_b4a8_558_lit\n"
        "adds r0, r7, r3\n"
        "movs r3, #0\n"
        "ldrsh r2, [r0, r3]\n"
        "movs r0, #1\n"
        "negs r0, r0\n"
        "cmp r6, r0\n"
        "beq .Lai_b4a8_5c4\n"
        "ldr r3, .Lai_b4a8_55c_lit\n"
        "adds r0, r7, r3\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "cmp r0, r1\n"
        "blt .Lai_b4a8_5c4\n"
        "cmp r2, r1\n"
        "bge .Lai_b4a8_5c4\n"
        "cmp r6, #2\n"
        "bhi .Lai_b4a8_58c\n"
        "ldr r0, .Lai_b4a8_560_lit\n"
        "adds r4, r7, r0\n"
        "strh r6, [r4, #0]\n"
        "adds r0, r6, #0\n"
        "bl sub_08025F20\n"
        "adds r0, #1\n"
        "ldr r1, .Lai_b4a8_564_lit\n"
        "adds r2, r7, r1\n"
        "strh r0, [r2, #0]\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #16\n"
        "cmp r0, #3\n"
        "ble .Lai_b4a8_568\n"
        "movs r0, #3\n"
        "strh r0, [r2, #0]\n"
        "b .Lai_b4a8_5c4\n"
        ".align 2, 0\n"
        ".Lai_b4a8_54c_lit: .word 0x03001780\n"
        ".Lai_b4a8_550_lit: .word 0x00000FF8\n"
        ".Lai_b4a8_554_lit: .word 0x00000FF2\n"
        ".Lai_b4a8_558_lit: .word 0x00001054\n"
        ".Lai_b4a8_55c_lit: .word 0x000010E5\n"
        ".Lai_b4a8_560_lit: .word 0x00001046\n"
        ".Lai_b4a8_564_lit: .word 0x00001048\n"
        ".Lai_b4a8_568:\n"
        "ldr r3, .Lai_b4a8_588_lit\n"
        "adds r1, r7, r3\n"
        "movs r0, #1\n"
        "strh r0, [r1, #0]\n"
        "movs r6, #0\n"
        "ldrsh r0, [r4, r6]\n"
        "movs r3, #0\n"
        "ldrsh r1, [r2, r3]\n"
        "bl sub_08025EC0\n"
        "movs r0, #27\n"
        "movs r1, #0\n"
        "bl _08023FF8\n"
        "b .Lai_b4a8_5c4\n"
        ".align 2, 0\n"
        ".Lai_b4a8_588_lit: .word 0x0000105A\n"
        ".Lai_b4a8_58c:\n"
        "adds r0, r6, #0\n"
        "bl sub_08025F20\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #16\n"
        "cmp r0, r9\n"
        "bge .Lai_b4a8_5c4\n"
        "ldr r0, .Lai_b4a8_804_lit\n"
        "adds r3, r7, r0\n"
        "strh r6, [r3, #0]\n"
        "ldr r1, .Lai_b4a8_808_lit\n"
        "adds r2, r7, r1\n"
        "mov r6, r9\n"
        "strh r6, [r2, #0]\n"
        "adds r0, #20\n"
        "adds r1, r7, r0\n"
        "movs r0, #1\n"
        "strh r0, [r1, #0]\n"
        "movs r1, #0\n"
        "ldrsh r0, [r3, r1]\n"
        "movs r3, #0\n"
        "ldrsh r1, [r2, r3]\n"
        "bl sub_08025EC0\n"
        "movs r0, #27\n"
        "movs r1, #0\n"
        "bl _08023FF8\n"
        ".Lai_b4a8_5c4:\n"
        "ldr r7, .Lai_b4a8_80c_lit\n"
        "ldr r6, .Lai_b4a8_810_lit\n"
        "adds r5, r7, r6\n"
        "movs r1, #0\n"
        "ldrsh r0, [r5, r1]\n"
        "ldr r2, .Lai_b4a8_814_lit\n"
        "adds r4, r7, r2\n"
        "movs r3, #0\n"
        "ldrsh r1, [r4, r3]\n"
        "mov r2, r8\n"
        "bl sub_08024FC4\n"
        "lsls r0, r0, #16\n"
        "asrs r6, r0, #16\n"
        "movs r1, #0\n"
        "ldrsh r0, [r5, r1]\n"
        "movs r2, #0\n"
        "ldrsh r1, [r4, r2]\n"
        "mov r2, r8\n"
        "bl sub_08024FE8\n"
        "lsls r0, r0, #16\n"
        "asrs r4, r0, #16\n"
        "movs r0, #1\n"
        "negs r0, r0\n"
        "cmp r6, r0\n"
        "beq .Lai_b4a8_652\n"
        "adds r0, r6, #0\n"
        "bl sub_08025FAC\n"
        "lsls r0, r0, #16\n"
        "cmp r0, #0\n"
        "bne .Lai_b4a8_652\n"
        "ldr r3, .Lai_b4a8_818_lit\n"
        "adds r0, r7, r3\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "cmp r0, r4\n"
        "blt .Lai_b4a8_652\n"
        "ldr r1, .Lai_b4a8_81c_lit\n"
        "adds r0, r7, r1\n"
        "movs r4, #1\n"
        "strh r4, [r0, #0]\n"
        "adds r0, r6, #0\n"
        "bl sub_08024C90\n"
        "lsls r0, r0, #1\n"
        "movs r2, #131\n"
        "lsls r2, r2, #5\n"
        "adds r1, r7, r2\n"
        "adds r0, r0, r1\n"
        "strh r4, [r0, #0]\n"
        "ldr r3, .Lai_b4a8_820_lit\n"
        "adds r4, r7, r3\n"
        "movs r1, #0\n"
        "ldrsh r0, [r4, r1]\n"
        "subs r2, #34\n"
        "adds r1, r7, r2\n"
        "adds r0, r0, r1\n"
        "strb r6, [r0, #0]\n"
        "adds r0, r6, #0\n"
        "bl sub_08025F78\n"
        "movs r0, #28\n"
        "movs r1, #0\n"
        "bl _08023FF8\n"
        "ldrh r0, [r4, #0]\n"
        "adds r0, #1\n"
        "strh r0, [r4, #0]\n"
        ".Lai_b4a8_652:\n"
        "movs r6, #0\n"
        "movs r7, #0\n"
        "movs r3, #0\n"
        "mov r8, r3\n"
        "movs r5, #0\n"
        ".Lai_b4a8_65c:\n"
        "movs r4, #0\n"
        ".Lai_b4a8_65e:\n"
        "movs r0, #0\n"
        "adds r1, r5, #0\n"
        "adds r2, r4, #0\n"
        "bl sub_08025CF4\n"
        "lsls r0, r0, #24\n"
        "cmp r0, #0\n"
        "beq .Lai_b4a8_682\n"
        "movs r0, #0\n"
        "adds r1, r5, #0\n"
        "adds r2, r4, #0\n"
        "bl sub_08025CF4\n"
        "lsls r0, r0, #24\n"
        "lsrs r0, r0, #24\n"
        "cmp r0, #3\n"
        "bhi .Lai_b4a8_682\n"
        "adds r6, #1\n"
        ".Lai_b4a8_682:\n"
        "movs r0, #1\n"
        "adds r1, r5, #0\n"
        "adds r2, r4, #0\n"
        "bl sub_08025CF4\n"
        "lsls r0, r0, #24\n"
        "cmp r0, #0\n"
        "beq .Lai_b4a8_6a6\n"
        "movs r0, #1\n"
        "adds r1, r5, #0\n"
        "adds r2, r4, #0\n"
        "bl sub_08025CF4\n"
        "lsls r0, r0, #24\n"
        "lsrs r0, r0, #24\n"
        "cmp r0, #3\n"
        "bhi .Lai_b4a8_6a6\n"
        "adds r7, #1\n"
        ".Lai_b4a8_6a6:\n"
        "adds r4, #1\n"
        "cmp r4, #10\n"
        "ble .Lai_b4a8_65e\n"
        "adds r5, #1\n"
        "cmp r5, #3\n"
        "ble .Lai_b4a8_65c\n"
        "cmp r6, #43\n"
        "ble .Lai_b4a8_6f8\n"
        "movs r0, #94\n"
        "bl sub_08025FAC\n"
        "lsls r0, r0, #16\n"
        "cmp r0, #0\n"
        "bne .Lai_b4a8_6f8\n"
        "movs r0, #0\n"
        "bl sub_08026020\n"
        "ldr r1, .Lai_b4a8_80c_lit\n"
        "ldr r6, .Lai_b4a8_824_lit\n"
        "adds r2, r1, r6\n"
        "movs r0, #1\n"
        "strh r0, [r2, #0]\n"
        "ldr r0, .Lai_b4a8_820_lit\n"
        "adds r4, r1, r0\n"
        "movs r2, #0\n"
        "ldrsh r0, [r4, r2]\n"
        "ldr r3, .Lai_b4a8_828_lit\n"
        "adds r1, r1, r3\n"
        "adds r0, r0, r1\n"
        "movs r1, #94\n"
        "strb r1, [r0, #0]\n"
        "movs r0, #94\n"
        "bl sub_08025F78\n"
        "movs r0, #28\n"
        "movs r1, #0\n"
        "bl _08023FF8\n"
        "ldrh r0, [r4, #0]\n"
        "adds r0, #1\n"
        "strh r0, [r4, #0]\n"
        ".Lai_b4a8_6f8:\n"
        "cmp r7, #43\n"
        "ble .Lai_b4a8_73e\n"
        "movs r0, #95\n"
        "bl sub_08025FAC\n"
        "lsls r0, r0, #16\n"
        "cmp r0, #0\n"
        "bne .Lai_b4a8_73e\n"
        "movs r0, #1\n"
        "bl sub_08026020\n"
        "ldr r1, .Lai_b4a8_80c_lit\n"
        "ldr r6, .Lai_b4a8_824_lit\n"
        "adds r2, r1, r6\n"
        "movs r0, #1\n"
        "strh r0, [r2, #0]\n"
        "ldr r0, .Lai_b4a8_820_lit\n"
        "adds r4, r1, r0\n"
        "movs r2, #0\n"
        "ldrsh r0, [r4, r2]\n"
        "ldr r3, .Lai_b4a8_828_lit\n"
        "adds r1, r1, r3\n"
        "adds r0, r0, r1\n"
        "movs r1, #95\n"
        "strb r1, [r0, #0]\n"
        "movs r0, #95\n"
        "bl sub_08025F78\n"
        "movs r0, #28\n"
        "movs r1, #0\n"
        "bl _08023FF8\n"
        "ldrh r0, [r4, #0]\n"
        "adds r0, #1\n"
        "strh r0, [r4, #0]\n"
        ".Lai_b4a8_73e:\n"
        "movs r5, #0\n"
        ".Lai_b4a8_740:\n"
        "movs r4, #0\n"
        "adds r6, r5, #1\n"
        ".Lai_b4a8_744:\n"
        "movs r0, #1\n"
        "adds r1, r5, #0\n"
        "adds r2, r4, #0\n"
        "bl sub_08025CF4\n"
        "lsls r0, r0, #24\n"
        "lsrs r0, r0, #24\n"
        "cmp r0, #3\n"
        "bne .Lai_b4a8_75a\n"
        "movs r0, #1\n"
        "add r8, r0\n"
        ".Lai_b4a8_75a:\n"
        "adds r4, #1\n"
        "cmp r4, #10\n"
        "ble .Lai_b4a8_744\n"
        "adds r5, r6, #0\n"
        "cmp r5, #3\n"
        "ble .Lai_b4a8_740\n"
        "mov r1, r8\n"
        "cmp r1, #43\n"
        "ble .Lai_b4a8_7ae\n"
        "movs r0, #96\n"
        "bl sub_08025FAC\n"
        "lsls r0, r0, #16\n"
        "cmp r0, #0\n"
        "bne .Lai_b4a8_7ae\n"
        "movs r0, #2\n"
        "bl sub_08026020\n"
        "ldr r1, .Lai_b4a8_80c_lit\n"
        "ldr r3, .Lai_b4a8_824_lit\n"
        "adds r2, r1, r3\n"
        "movs r0, #1\n"
        "strh r0, [r2, #0]\n"
        "ldr r6, .Lai_b4a8_820_lit\n"
        "adds r4, r1, r6\n"
        "movs r2, #0\n"
        "ldrsh r0, [r4, r2]\n"
        "subs r3, #56\n"
        "adds r1, r1, r3\n"
        "adds r0, r0, r1\n"
        "movs r1, #96\n"
        "strb r1, [r0, #0]\n"
        "movs r0, #96\n"
        "bl sub_08025F78\n"
        "movs r0, #28\n"
        "movs r1, #0\n"
        "bl _08023FF8\n"
        "ldrh r0, [r4, #0]\n"
        "adds r0, #1\n"
        "strh r0, [r4, #0]\n"
        ".Lai_b4a8_7ae:\n"
        "movs r6, #0\n"
        "movs r5, #0\n"
        ".Lai_b4a8_7b2:\n"
        "movs r4, #0\n"
        ".Lai_b4a8_7b4:\n"
        "adds r7, r5, #1\n"
        ".Lai_b4a8_7b6:\n"
        "movs r0, #0\n"
        "adds r1, r5, #0\n"
        "adds r2, r4, #0\n"
        "bl sub_08025CF4\n"
        "lsls r0, r0, #24\n"
        "lsrs r0, r0, #24\n"
        "cmp r0, #3\n"
        "bne .Lai_b4a8_7ca\n"
        "adds r6, #1\n"
        ".Lai_b4a8_7ca:\n"
        "adds r4, #1\n"
        "cmp r4, #10\n"
        "ble .Lai_b4a8_7b6\n"
        "adds r5, r7, #0\n"
        "cmp r5, #3\n"
        "ble .Lai_b4a8_7b2\n"
        "cmp r6, #43\n"
        "ble .Lai_b4a8_7f6\n"
        "ldr r0, .Lai_b4a8_80c_lit\n"
        "movs r6, #255\n"
        "lsls r6, r6, #4\n"
        "adds r1, r0, r6\n"
        "movs r2, #0\n"
        "ldrsh r0, [r1, r2]\n"
        "cmp r0, #0\n"
        "bne .Lai_b4a8_7f6\n"
        "movs r0, #1\n"
        "strh r0, [r1, #0]\n"
        "movs r0, #29\n"
        "movs r1, #0\n"
        "bl _08023FF8\n"
        ".Lai_b4a8_7f6:\n"
        "pop {r3, r4}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".Lai_b4a8_804_lit: .word 0x00001046\n"
        ".Lai_b4a8_808_lit: .word 0x00001048\n"
        ".Lai_b4a8_80c_lit: .word 0x03001780\n"
        ".Lai_b4a8_810_lit: .word 0x00000FF2\n"
        ".Lai_b4a8_814_lit: .word 0x00000FF8\n"
        ".Lai_b4a8_818_lit: .word 0x000010E5\n"
        ".Lai_b4a8_81c_lit: .word 0x00001058\n"
        ".Lai_b4a8_820_lit: .word 0x0000103C\n"
        ".Lai_b4a8_824_lit: .word 0x00001076\n"
        ".Lai_b4a8_828_lit: .word 0x0000103E\n"
        ".syntax divided\n"
    );
}


// Leaf proved for wa+0x1054 s16 adjacency to _0800B4A8 — exact asm plus ramwatch
// Asm: ldr r3,=0x00001054; adds r0, r7, r3; movs r3,#0; ldrsh r2,[r0,r3] (s16, offset 0) then cmp r2,r1 (id2 s16 via lsls16/asrs16) with bge (signed)
// Ramwatch headless 2500 frames (interval 100, baserom.gba inputs.csv → /tmp/caps1054b): IWRAM 0x030027D4 (wa+0x1054) stable 0x0000 s16 0 / tier 0, width s16 via ldrsh not ldrh, no racectx +0x490/+0x4E0 invented
s16 Ai_B4A8_GetLimit(void){
    volatile u8 *wa = (volatile u8*)(uintptr_t)0x03001780u;
    return *(volatile s16*)(wa+0x1054);
}

// ROM entry alias.
void _080023FE4(void) __attribute__((alias("Ai_RingReset")));

// ROM entry alias.
void _080023FF8(int ev,int arg) __attribute__((alias("Ai_RingPush")));
