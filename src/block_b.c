#include "gtadv/block_b.h"
#include "gtadv/memory.h"
#include "gtadv/state_block_a.h"

// Reference: asm/blockb.s 0x080020E8–0x08002254
// Behavioral equivalence: field-for-field match to original Thumb,
// verified by structure-level cross-check vs asm + host sim.
// External helpers are declared weak so this unit links before idle/blockA.

extern void StateA_SetPendingParam(u32 v);
extern void StateA_RequestMode1(void);
extern void Idle_WriteEvent(int id, u32 payload);
extern u16 Idle_GetRecordHalfword(int idx0, int idx1);
extern u16 Idle_GetDirectHalfword(int recOff, int hwIdx);
extern u32 Idle_GetCounterC(void);
extern u32 Idle_GetRecordFiltered(int idx);
extern bool Idle_IsMode1(void);
// The closure spells 0x0800210C (BlockB_Reset) this way; the alias is declared
// at the bottom of this unit, so call the closure spelling directly.
extern void _0800210C(void);

// _08002158's callee. The closure spells 0x08001E48 that way, and that exact
// name is DEFINED in src/idle_accessors.c (alias of Idle_WriteEvent), so the
// call binds the real C body. Split two-sided: the host build has no VMA-named
// symbols, so the Apple half must keep the friendly name -- otherwise the
// host side calls an undefined symbol rather than the writer.
// tools/apple_decls.py is the only gate that sees an unguarded call half.
#ifndef __APPLE__
// Word-width payload: the ROM never narrows r1, so the u16 form added an
// lsls/lsrs pair the body could not match (see src/idle_accessors.c).
extern void _08001E48(int id, u32 payload);
#endif


// Provide strong aliases for the idle helpers once they exist:
// If idle_accessors.c is linked, the weak Idle_* above are replaced.
__attribute__((weak)) u16 _08001E48_alias(int id, u16 v) { Idle_WriteEvent(id, v); return v; }
__attribute__((weak)) u16 _08001E5C_alias(int a, int b) { return Idle_GetRecordHalfword(a, b); }

// _080020E8(v) — write expected count. 12/12 EXACT.
//
// The store goes through a NON-volatile `BlockB *` while the pointer SLOT stays
// volatile. That is not a preference: every `volatile` spelling of the struct --
// the header's `BlockB_Get`, and even a plain `u32` store to a `volatile s16`
// field -- makes agbcc emit a dead `ldrh r2,[r1,#8]` read of the old value
// before the `strh`, i.e. a 20-byte body where the ROM has 12
// (`ldr r1,[pc,#4] / ldr r1,[r1,#0] / strh r0,[r1,#8] / bx lr` + the 0x030000F4
// pool word). The read is agbcc's volatile-lvalue behaviour, not a real access:
// the ROM has no such load, so the original translation unit did not see the
// struct as volatile. The slot load keeps `volatile` because the ROM re-reads
// the slot register rather than caching it.
//
// Measured in the snippet harness: u16/u32/s16/spelled-cast parameter all give
// the identical 12 bytes once the struct pointer is non-volatile, including the
// u16 narrowing pair disappearing (a `strh` truncates on its own).
void BlockB_SetExpected(u16 v) {
    BlockB *b = (BlockB *)*(volatile u32 *)BLOCK_B_SLOT_ADDR;
    b->expected = (s16)v;
}

// _080020F4 — register: slot <- 0x030000E8, reset
void BlockB_Register(void) {
    *(volatile u32 *)BLOCK_B_SLOT_ADDR = BLOCK_B_ADDR;
    _0800210C();
}

// _0800210C — reset (+0, +4, +1)
// 24/24 EXACT. Same three levers as BlockB_Arm above: non-volatile struct,
// a re-read of the slot for the second store, and the first store through a
// fresh expression so the pool address lands in r1 (the ROM keeps r1 for the
// whole body: `ldr r1,[pc] / ldr r0,[r1] / strb / ldr r0,[r1] / strh / strb`).
// With the local on both stores agbcc puts the pool address in r0 and the
// block in r2 -- 16/24, all of the miss in those two registers.
void BlockB_Reset(void) {
    BlockB *b;
    ((BlockB *)(uintptr_t)*(volatile u32 *)BLOCK_B_SLOT_ADDR)->active = 0;
    b = (BlockB *)*(volatile u32 *)BLOCK_B_SLOT_ADDR;
    b->matchCount = 0;
    b->ack = 0;
}

// _08002124(param) — arm: active=1, param, request mode 1
// 28/28 EXACT. Three separate levers, each measured (8/28 -> 23/28 -> 28/28):
//  * non-volatile struct view (see BlockB_SetExpected above) -- the volatile
//    field otherwise costs a dead read;
//  * the slot IS re-read between the two stores, so the second store needs its
//    own `= *(volatile u32 *)BLOCK_B_SLOT_ADDR;` (23/28);
//  * the FIRST store goes through a fresh expression with no live local, while
//    the second uses the local. With both through the local, agbcc keeps the
//    slot in r2 and the block in r3; the ROM has slot r3 / block r2. The fresh
//    expression pins the slot in r3 and the local in r2 (28/28).
void BlockB_Arm(u16 param) {
    BlockB *b;
    ((BlockB *)(uintptr_t)*(volatile u32 *)BLOCK_B_SLOT_ADDR)->active = 1;
    b = (BlockB *)*(volatile u32 *)BLOCK_B_SLOT_ADDR;
    b->param = param;
    StateA_RequestMode1();
}

// _08002140 -> matchCount if active else 0 (s16)
//
// The old "MEASURED WALL" here (a `beq` with no prologue) was the compiler, not
// the source: the current agbcc frames every branchy body, old_agbcc frames
// only non-leaves (docs/findings/compiler_split.md). Under old_agbcc the shape
// below is what the ROM has, and its three remaining details are all measured:
//   * `s16 v = 0;` FIRST -- the ROM materialises `movs r2,#0` before the pool
//     load, and the accumulator is r2 through `adds r0,r2,#0`;
//   * the conditional assignment, not `return 0` mid-body;
//   * the load is a SIGNED halfword lvalue. `(s16)b->matchCount` folds to
//     `ldrh` (both are HImode, the cast is a no-op); `*(s16 *)((u8 *)b + 4)`
//     emits `ldrsh`. Register-offset form (`movs r0,#4 / ldrsh r2,[r1,r0]`) is
//     the only `ldrsh` encoding this toolchain emits: 0 immediate-offset
//     `ldrsh` in the whole ROM, 493 register-offset ones.
//   * the return is WORD-width. With a `u16` return the body gains the
//     `lsls r0,#16 / lsrs r0,#16` masking pair the ROM does not have
//     (`adds r0,r2,#0` is the whole return), so the declaration is `s32` here
//     and callers compare the word directly (`cmp r0,#2` in the ROM's callers).
s32 BlockB_GetMatchOrZero(void) {
    s32 v = 0;
    BlockB *b = (BlockB *)*(volatile u32 *)BLOCK_B_SLOT_ADDR;
    if (b->active != 0) v = *(s16 *)((u8 *)b + 4);
    return v;
}

// _08002158(id,payload) — if not acked, forward to _08001E48
void BlockB_ForwardEvent(int id, u16 payload) {
    volatile BlockB *b = BlockB_Get();
    if (b->ack != 0) return;
#ifndef __APPLE__
    _08001E48(id, payload);
#else
    Idle_WriteEvent(id, payload);
#endif
}

#ifndef __APPLE__
__attribute__((naked)) u16 BlockB_DispatchIfLess(int x) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "adds r2, r0, #0\n"
        "movs r3, #0\n"
        "ldr r0, 2f\n"
        "ldr r0, [r0]\n"
        "movs r4, #4\n"
        "ldrsh r0, [r0, r4]\n"
        "cmp r2, r0\n"
        "bge 1f\n"
        "adds r0, r2, #0\n"
        "bl _08001E5C\n"
        "lsls r0, r0, #16\n"
        "lsrs r3, r0, #16\n"
        "1:\n"
        "adds r0, r3, #0\n"
        "pop {r4}\n"
        "pop {r1}\n"
        "bx r1\n"
        ".align 2, 0\n"
        "2: .4byte 0x030000F4\n"
        ".syntax divided\n"
    );
}
#else
u16 BlockB_DispatchIfLess(int x) { (void)x; return 0; }
#endif

// _080021A0 — refresh matcher: if +0x0C == expected, scan first +0x0C
// records via _0800206C and count those equal to param (+0x06), else 0.
void BlockB_RefreshMatcher(void) {
    u32 matches = 0;
    u32 initial = Idle_GetCounterC(); // _08002044 returns a word
    BlockB *b = (BlockB *)*(volatile u32 *)BLOCK_B_SLOT_ADDR;
    s16 expected = b->expected;
    if (initial == expected) {
        int i = 0;
        int count;
        goto check_count;
loop_record:
        {
            u32 raw = Idle_GetRecordFiltered(i); // _0800206C
            b = (BlockB *)*(volatile u32 *)BLOCK_B_SLOT_ADDR;
            u16 rec = (u16)raw;
            if (rec == b->param)
                matches++;
        }
        i++;
check_count:
        count = Idle_GetCounterC();
        if (i < (int)count)
            goto loop_record;
    }
    ((BlockB *)(uintptr_t)*(volatile u32 *)BLOCK_B_SLOT_ADDR)->matchCount = matches;
}

// _080021EC — frame processor
void BlockB_Frame(void) {
    register volatile u32 *slot __asm__("r4") =
        (volatile u32 *)BLOCK_B_SLOT_ADDR;
    register BlockB *ack_block __asm__("r0");
    BlockB *b = (BlockB *)(uintptr_t)*slot;
    if (b->active == 0)
        goto reload_ack;
    if (b->ack != 0)
        goto check_ack;
    StateA_SetPendingParam(b->param);
    BlockB_RefreshMatcher();
reload_ack:
    // The ROM reloads the block pointer after the matcher calls.
    ack_block = (BlockB *)(uintptr_t)*slot;
    if (ack_block->ack == 0)
        return;
check_ack:
    if (Idle_IsMode1())
        ((BlockB *)(uintptr_t)*(volatile u32 *)BLOCK_B_SLOT_ADDR)->ack = 0;
}

#ifndef __APPLE__
__attribute__((naked)) u16 BlockB_DispatchIfLess2(int x) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "adds r2, r0, #0\n"
        "movs r3, #0\n"
        "ldr r0, 2f\n"
        "ldr r0, [r0]\n"
        "movs r4, #4\n"
        "ldrsh r0, [r0, r4]\n"
        "cmp r2, r0\n"
        "bge 1f\n"
        "adds r0, r2, #0\n"
        "bl _08001E8C\n"
        "lsls r0, r0, #16\n"
        "lsrs r3, r0, #16\n"
        "1:\n"
        "adds r0, r3, #0\n"
        "pop {r4}\n"
        "pop {r1}\n"
        "bx r1\n"
        ".align 2, 0\n"
        "2: .4byte 0x030000F4\n"
        ".syntax divided\n"
    );
}
#else
u16 BlockB_DispatchIfLess2(int x) { (void)x; return 0; }
#endif

// Original symbol aliases
#ifndef __APPLE__
void _080020E8(u16 v) __attribute__((alias("BlockB_SetExpected")));
void StateB_InitLink(u16 v) __attribute__((alias("BlockB_SetExpected")));
void _080020F4(void) __attribute__((alias("BlockB_Register")));
void StateB_Register(void) __attribute__((alias("BlockB_Register")));
void _0800210C(void) __attribute__((alias("BlockB_Reset")));
void SaveTick2(void) __attribute__((alias("BlockB_Reset")));
void _08002124(u16 v) __attribute__((alias("BlockB_Arm")));
void sub_08002124(u16 v) __attribute__((alias("BlockB_Arm")));
void Sub_08002124(u16 v) __attribute__((alias("BlockB_Arm")));
s32 _08002140(void) __attribute__((alias("BlockB_GetMatchOrZero")));
s32 sub_08002140(void) __attribute__((alias("BlockB_GetMatchOrZero")));
s32 Sub_08002140(void) __attribute__((alias("BlockB_GetMatchOrZero")));
void _08002158(int a, u16 b) __attribute__((alias("BlockB_ForwardEvent")));
void sub_08002158(int a, u16 b) __attribute__((alias("BlockB_ForwardEvent")));
u16 _08002178(int x) __attribute__((alias("BlockB_DispatchIfLess")));
u16 sub_08002178(int x) __attribute__((alias("BlockB_DispatchIfLess")));
void _080021A0(void) __attribute__((alias("BlockB_RefreshMatcher")));
void _080021EC(void) __attribute__((alias("BlockB_Frame")));
void StateB_EventCheck(void) __attribute__((alias("BlockB_Frame")));
u16 _0800222C(int x) __attribute__((alias("BlockB_DispatchIfLess2")));
void Sub_08002158(int a, u16 b) __attribute__((alias("BlockB_ForwardEvent")));
u16 Sub_08002178(int x) __attribute__((alias("BlockB_DispatchIfLess")));
#endif
