#include "gtadv/state_block_a.h"
#include "gtadv/memory.h"
#include "gba/regs.h"
#include "gba/bios.h"

// Reference: asm/blocka.s 0x080015F4–0x08001988
// Block A lives at IWRAM 0x03000008, pointer slot 0x030000E4.
// Record selection uses ROM table at 0x0802E190 with sel = st->sioBits style.
// Each table entry holds a record index; record 16 bytes at +0x70.

static inline volatile StateBlockA *stateA(void) {
    return (volatile StateBlockA *)* (volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
}

// _080015F4(idx) -> u16  record getter, normal select
u16 StateA_GetRecordHalfword(int idx) {
    extern u8 StateARecTbl[];
    StateBlockA *st;
    u8 *tbl;
    u32 entry;
    uintptr_t base;
    __asm__(".globl StateARecTbl\nStateARecTbl = 0x0802E190");
    st = *(StateBlockA **)STATE_BLOCK_A_SLOT_ADDR;
    tbl = StateARecTbl;
    entry = *(u32 *)(((u32)idx << 2) + ((u32)st->sioBits << 4) + tbl);
    entry <<= 4;
    base = (uintptr_t)st + 0x70;
    return *(volatile u16 *)(base + entry);
}

// _08001620(idx0, idx1) -> u16  with halfword offset (idx1+1)*2
u16 StateA_GetRecordHalfwordOffset(int idx0, int idx1) {
    extern u8 StateARecTbl[];
    StateBlockA *st;
    u32 halfOff;
    u8 *tbl;
    u32 entry;
    uintptr_t base;
    __asm__(".globl StateARecTbl\nStateARecTbl = 0x0802E190");
    st = *(StateBlockA **)STATE_BLOCK_A_SLOT_ADDR;
    halfOff = (u32)(idx1 + 1) << 1;
    tbl = StateARecTbl;
    entry = *(u32 *)(((u32)idx0 << 2) + ((u32)st->sioBits << 4) + tbl);
    halfOff += entry << 4;
    base = (uintptr_t)st + 0x70;
    return *(volatile u16 *)(base + halfOff);
}

// _08001650 -> first halfword of normal-select record (entry 0)
u16 StateA_GetFirstHalfword(void) {
    volatile StateBlockA *st = stateA();
    return *(volatile u16 *)((uintptr_t)st + 0x70);
}

// _08001660(idx) -> indexed halfword from normal record
u16 StateA_GetIndexedHalfword(int idx) {
    volatile StateBlockA *st = stateA();
    u32 off = (u32)(idx + 1) << 1;
    uintptr_t base = (uintptr_t)st + 0x70;   // ROM: `adds r1,#0x70` on the slot pointer
    return *(volatile u16 *)(base + off);
}

// _08001674 -> address of normal-select record
void *StateA_GetRecordPtr(void) {
    volatile StateBlockA *st = stateA();
    return (void *)((uintptr_t)st + 0x70);
}

// _08001680 -> first halfword of inverted-select record (sel ^ 1)
u16 StateA_GetRecordHalfwordInverted(void) {
    volatile StateBlockA *st = stateA();
    u16 one = 1;
    u16 sel = (u16)(st->sioBits ^ one);
    u32 off = (u32)sel << 4;
    uintptr_t base = (uintptr_t)st + 0x70;
    return *(volatile u16 *)(base + off);
}

// _08001698(idx) -> indexed halfword, inverted
u16 StateA_GetIndexedHalfwordInverted(int idx) {
    register volatile StateBlockA *st __asm__("r2") = stateA();
    u32 halfOff = (u32)(idx + 1) << 1;
    u16 one = 1;
    u16 sel = (u16)(st->sioBits ^ one);
    u32 scaled = (u32)sel << 4;
    u32 off = halfOff + scaled;
    uintptr_t base = (uintptr_t)st + 0x70;
    return *(volatile u16 *)(base + off);
}

// _080016B8 -> address of inverted-select record
void *StateA_GetRecordPtrInverted(void) {
    volatile StateBlockA *st = stateA();
    u16 sel = 1;
    sel ^= st->sioBits;
    u32 off = (u32)sel << 4;
    return (void *)((uintptr_t)st + 0x70 + off);
}

u16 _080015F4(int idx) __attribute__((alias("StateA_GetRecordHalfword")));
u16 _08001620(int a, int b) __attribute__((alias("StateA_GetRecordHalfwordOffset")));
u16 _08001650(void) __attribute__((alias("StateA_GetFirstHalfword")));
u16 _08001660(int idx) __attribute__((alias("StateA_GetIndexedHalfword")));
void *_08001674(void) __attribute__((alias("StateA_GetRecordPtr")));
u16 _08001680(void) __attribute__((alias("StateA_GetRecordHalfwordInverted")));
u16 _08001698(int idx) __attribute__((alias("StateA_GetIndexedHalfwordInverted")));
void *_080016B8(void) __attribute__((alias("StateA_GetRecordPtrInverted")));
void _080016D0(u32 v) __attribute__((alias("StateA_SetPendingParam")));
void _080016DC(void) __attribute__((alias("StateA_ClearPendingAndScratch")));

// _080016D0(v) — write pending param
//
// True extent is 0x080016D0–0x080016DC = 12 B (4 instructions plus the
// 0x030000E4 pool word at +0x08). The probe's 28 B ROM span runs to 0x080016EC
// and so swallows _080016DC, which asm_vmas does not know as an entry.
void StateA_SetPendingParam(u32 v) {
    volatile StateBlockA *st = stateA();
    *(volatile u16 *)((volatile u8 *)st + 4) = v;
}

// _080016DC — clear +0x04 and +0x08
//
// True extent is 0x080016DC-0x080016EC = 16 B: `ldr r0,=0x030000E4 /
// ldr r1,[r0] / movs r0,#0 / strh r0,[r1,#4] / strh r0,[r1,#8] / bx lr`
// plus the slot pool word.
//
// The two stores must go through SCALAR volatile u16 pointers, not struct
// members: a volatile struct-member store expands in agbcc into a dead
// `ldrh` read before the `strh` (measured 20-byte candidate with
// `ldrh r1,[r1,#4]` at +0x04 and a SECOND pool word, 2/16 matched). The ROM
// has no dead read and one pool word. Same lever as StateA_SetPendingParam.
void StateA_ClearPendingAndScratch(void) {
    volatile u8 *st = (volatile u8 *)stateA();
    *(volatile u16 *)(st + 4) = 0;
    *(volatile u16 *)(st + 8) = 0;
}

// Helpers for the next cluster — forward decls for external session hooks.
// The weak bodies delegate to the real lifted functions when present:
//   session-enter _080005C0 (foundation_agbmain.c: IME=0, IE&=0xFF3F, IF=1),
//   session-exit  _080006BC (asm/agbmain.s:0x6BC: IME=0, IE&=0xFF3F, IF=1,
//     SIOCNT=0x2003, RCNT-style word 0xABFB at 0x0400010C, TM count 0xC0, …).
// NOTE: the blocka.s call sites pass TWO args to _08000BF0
// (`movs r0,#1; ldr r1,[st+0x24]; bl sub_08000BF0`) and the callee's prologue
// moves both (r0→r5, r1→r7) — so its C ABI here is (int flag, u32 arg).
__attribute__((weak)) void SessionEnterHook(int arg) {
    extern void _080005C0(int);
    _080005C0(arg);
}
__attribute__((weak)) void SessionExitHook(void) {
    extern void _080006BC(void);
    _080006BC();
}
__attribute__((weak)) void SessionMode8Hook(u32 arg, int flag) {
    extern void _08000BF0(int, u32);
    _08000BF0(flag, arg);
}

// _08001704 — helper: CpuSet fill of block A (src = stack zero word, dst =
// block A 0x03000008, ctrl 0x05000037 = fill|32bit|55 words = 220 bytes)
void StateA_Fill1754(void) {
    u32 zero = 0;
    CpuSet(&zero, (void *)STATE_BLOCK_A_ADDR, 0x05000037);
}
void _08001704(void) __attribute__((alias("StateA_Fill1754")));

// _080016EC — register block A: slot <- 0x03000008 then 220-byte fill
// (asm: str slot; bl _08001704)
void StateA_Register(void) {
    *(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR = STATE_BLOCK_A_ADDR;
    // The slice links no C object, so a promoted body may only call a
    // spelling the assembly closure defines. The host build has no
    // VMA-named symbol, so it keeps the friendly name.
    extern void _08001704(void);
    _08001704();
}
void _080016EC(void) __attribute__((alias("StateA_Register")));

// _08001724(mode) — set requested mode (also called from 0x8001F9E/0x8001FBA)
// `mode` is u32: the ROM body (12 B) has no lsls/lsrs zero-extend prologue.
// The store is written through a SCALAR volatile u16, not as a struct member:
// agbcc expands a volatile struct-member store of a non-constant value as a
// dead `ldrh` read + `strh`, and the ROM holds the bare `strh r0,[r1,#18]`.
void StateA_SetReqMode(u32 mode) {
    volatile StateBlockA *st = stateA();
    *(volatile u16 *)((volatile u8 *)st + 0x12) = mode;
}
void _08001724(u32 mode) __attribute__((alias("StateA_SetReqMode")));

// _08001730(mode) -> bool game-driven {1..7,10}
//
// True extent is 0x08001730-0x08001744 = 20 B, no pool. The ROM holds the
// answer in a register-sized temporary materialised FIRST (`movs r1,#0`),
// tests the three cases with fall-through polarity (`blt` past, `ble` to the
// set, `bne` past), and copies it back with `adds r0,r1,#0`.
//
// Two measured levers reached that shape and one did not:
//   * the parameter is WORD-width -- `u16` adds a 4-byte `lsls/lsrs`
//     narrowing prologue the ROM does not have (measured 32-byte candidate).
//     The ROM's callers pass already zero-extended values, so nothing
//     narrows; same lever as 0x08001744.
//   * the answer must live in a local, not be returned from each arm.
//     `return false` / `return true` materialises `movs r0,#0` on the
//     fall-through and branches over it (32 bytes, no `adds r0,r1,#0` copy).
// TRUE (`mode` in [1,7] or == 10) comes from a SWITCH, not an `if` chain:
// fold-const rewrites every tree `x < 1` / `x >= 1` with a positive constant
// into `<= 0` / `> 0` (measured nine spellings, all `cmp r0,#0` at +0x02),
// but stmt.c's switch lowering emits case bounds with their original
// constants. This case set makes node [1,7] the tree root with 10 as its
// right child, which emits the ROM's exact dispatch -- `cmp r0,#1 blt` (below
// low bound -> default), `cmp r0,#7 ble` (in range -> body), `cmp r0,#10
// bne` (== 10 -> default), fall into the body. The default arm supplies
// `mov r1,#0`; `r = 1`, with both in r0/r1, keeps the frameless leaf
// (`adds r0,r1,#0 / bx lr`). Promoted from naked asm 2026-10-16.
bool StateA_IsGameMode(int mode) {
    int r = 0;
    switch (mode) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 10:
        r = 1;
    }
    return r;
}
bool _08001730(int mode) __attribute__((alias("StateA_IsGameMode")));

// _08001744(mode) -> bool special {8,11}
//
// True extent is 0x08001744–0x08001754 = 16 B. The probe's span runs on to
// 0x08001758 and swallows _08001754, the empty `bx lr` stub that asm_vmas
// does not know as an entry -- the same span-table gap as _080016D0.
//
// Byte-exact under `old_agbcc` with `u32 mode`. Both halves are measured:
//   * the current agbcc frames EVERY body with a conditional branch, so this
//     16-byte leaf could never match there (see
//     docs/findings/compiler_split.md); old_agbcc is stock-2.95 framing.
//   * the parameter is WORD-width: `u16` adds a 4-byte `lsls/lsrs` narrowing
//     prologue the ROM does not have, and the ROM's only caller
//     (0x08001B7C, `ldrh r0,[r0,#0x10]` then `bl`) passes an already
//     zero-extended value, so nothing narrows. Same lever as 0x08001E48.
bool StateA_IsSpecialMode(u32 mode) {
    return mode == 8 || mode == 11;
}
bool _08001744(u32 mode) __attribute__((alias("StateA_IsSpecialMode")));

// _08001758 — mode-change processor
// Pseudocode documented in asm/blocka.s header and asm/blocka.s
// Called spellings: the ROM's three calls are the real functions
// _080005C0 / _080006BC / _08000BF0 (`bl 0x80005c0`, `bl 0x80006bc`,
// `bl 0x8000bf0`); going through the file-local Session*Hook wrappers leaves
// the relocations unresolved (the wrappers are static in this TU).
// Naming the hooks would be right only if the closure exported them -- it does
// not, so the probe reports UNRESOLVED_RELOCATION and the candidate loses the
// match from +0x62 on.
__attribute__((naked)) void StateA_CommitMode(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "ldr r7, .L1758_slot0\n"
        "ldr r0, [r7]\n"
        "ldrh r1, [r0, #16]\n"
        "ldrh r5, [r0, #18]\n"
        "cmp r5, r1\n"
        "beq .L1758_done\n"
        "adds r0, r1, #0\n"
        "bl 0x08001730\n"
        "adds r6, r0, #0\n"
        "adds r0, r5, #0\n"
        "bl 0x08001730\n"
        "adds r4, r0, #0\n"
        "cmp r6, #0\n"
        "bne .L1758_old_active\n"
        "cmp r4, #0\n"
        "beq .L1758_mode_switch\n"
        "movs r0, #0\n"
        "bl 0x080005C0\n"
        "ldr r0, [r7]\n"
        "strh r6, [r0, #8]\n"
        "b .L1758_mode_switch\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L1758_slot0: .word 0x030000E4\n"
        ".L1758_old_active:\n"
        "cmp r4, #0\n"
        "bne .L1758_mode_switch\n"
        "bl 0x080006BC\n"
        "ldr r0, [r7]\n"
        "strb r4, [r0, #1]\n"
        ".L1758_mode_switch:\n"
        "cmp r5, #6\n"
        "beq .L1758_clear_d0\n"
        "cmp r5, #6\n"
        "bgt .L1758_check_ten\n"
        "cmp r5, #3\n"
        "bgt .L1758_check_range_hi\n"
        "cmp r5, #2\n"
        "blt .L1758_check_range_hi\n"
        "b .L1758_clear_d0\n"
        ".L1758_check_ten:\n"
        "cmp r5, #10\n"
        "bne .L1758_check_range_hi\n"
        ".L1758_clear_d0:\n"
        "ldr r0, .L1758_slot1\n"
        "ldr r0, [r0]\n"
        "adds r0, #208\n"
        "movs r1, #0\n"
        "str r1, [r0]\n"
        ".L1758_check_range_hi:\n"
        "cmp r5, #8\n"
        "beq .L1758_mode_eight\n"
        "cmp r5, #8\n"
        "bgt .L1758_mode_eleven_check\n"
        "cmp r5, #1\n"
        "beq .L1758_mode_one\n"
        "b .L1758_store_mode\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L1758_slot1: .word 0x030000E4\n"
        ".L1758_mode_eleven_check:\n"
        "cmp r5, #11\n"
        "beq .L1758_mode_eleven\n"
        "b .L1758_store_mode\n"
        ".L1758_mode_one:\n"
        "ldr r0, .L1758_slot2\n"
        "ldr r1, [r0]\n"
        "ldr r0, .L1758_const_3eb\n"
        "strh r0, [r1, #4]\n"
        "b .L1758_store_mode\n"
        ".align 2, 0\n"
        ".L1758_slot2: .word 0x030000E4\n"
        ".L1758_const_3eb: .word 0x000003EB\n"
        ".L1758_mode_eight:\n"
        "ldr r0, .L1758_slot3\n"
        "ldr r0, [r0]\n"
        "ldr r1, [r0, #36]\n"
        "movs r0, #1\n"
        "bl 0x08000BF0\n"
        "b .L1758_store_mode\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L1758_slot3: .word 0x030000E4\n"
        ".L1758_mode_eleven:\n"
        "ldr r0, .L1758_slot4\n"
        "ldr r0, [r0]\n"
        "ldr r1, [r0, #40]\n"
        "movs r0, #0\n"
        "bl 0x08000BF0\n"
        ".L1758_store_mode:\n"
        "ldr r0, .L1758_slot4\n"
        "ldr r0, [r0]\n"
        "strh r5, [r0, #16]\n"
        ".L1758_done:\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".L1758_slot4: .word 0x030000E4\n"
        ".syntax divided\n"
    );
}
//. The caller above names `_08001758`, the one spelling
// asm/blocka.s declares. Adding a `sub_08001758` twin would NOT help: the
// closure has no such label, so the screen would then report the mirror-image
// `sub_08001758: closure defines _08001758 at 0x08001758 (rename)`. The fix is
// to name the closure's spelling, not to invent a second one.
void _08001758(void) __attribute__((alias("StateA_CommitMode")));

// _08001818 / _08001824 — requested-mode wrappers (set 1 / set 0)
void StateA_RequestMode1(void) {
    StateA_SetReqMode(1);
}
void StateA_RequestMode0(void) {
    StateA_SetReqMode(0);
}
void _08001818(void) __attribute__((alias("StateA_RequestMode1")));
void _08001824(void) __attribute__((alias("StateA_RequestMode0")));

// _08001754 / _08001830 — empty bx lr stubs (no BL callers found in ROM;
// exported so the cluster's VMA census is complete)
void StateA_Nop_1754(void) {}
__asm__(".align 2, 0");   // trap 6, same as _08001830 below
void StateA_Nop_1830(void) {}
__asm__(".align 2, 0");
void _08001754(void) __attribute__((alias("StateA_Nop_1754")));
void _08001830(void) __attribute__((alias("StateA_Nop_1830")));

// _08001830: empty stub
void StateA_SyncPerFrame(void) {
    volatile StateBlockA *st = stateA();
    u32 siocnt = *(volatile u32 *)(REG_BASE + 0x128);
    u32 bits = (siocnt << 26) >> 30;
    volatile u16 *dst = &st->sioBits;
    *dst = (u16)bits;
    //. Call the CLOSURE's spelling. asm/blocka.s labels this entry
    // `_08001758` and has no `sub_` twin, so `sub_08001758` fails to resolve
    // with `closure defines _08001758 at 0x08001758 (rename)` -- the C-only
    // name has to be replaced by the asm one, and the two are the same routine
    // at the same address, so the scored bytes do not move. Note this is the
    // OPPOSITE of 0x08005988, where the closure declares BOTH spellings.
    _08001758();
}
void _08001834(void) __attribute__((alias("StateA_SyncPerFrame")));

// Forward declarations for cross-module helpers (defined in idle / bios)
void Idle_WriteEvent(int id, u32 val); // _08001E48 (word-width: the ROM never narrows it)
u16  StateA_GetRecordHalfword(int idx);
u16  StateA_GetRecordHalfwordOffset(int idx0, int idx1);
void StateA_SetPendingParam(u32 v);
int  DivSI(int num, int den); // sub_0802DE04
void CpuFastSet(const void *src, void *dst, u32 mode);

// _08001854 — per-frame clock/record updater (caller 0x8001C18).
// Walks records 1..st[+0x0A]: min over getters, tracks a 30-frame
// countdown that commits "current minute" fields, writes mode-ish
// halfwords when on the last record, then drives three calls into
// the helper at 0x8001E48 and a final CpuFastSet copy of a 6-word record.
// See asm/blocka.s:0x1854 and asm/blocka.s.
__attribute__((naked)) void StateA_UpdateClockAndRecords(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, r8\n"
        "push {r7}\n"
        "movs r6, #0\n"
        "ldr r5, .Lstate1854__0800188C\n"
        "movs r7, #1\n"
        "movs r4, #1\n"
        "ldr r0, .Lstate1854__08001890\n"
        "ldr r1, [r0]\n"
        "mov r8, r0\n"
        "ldrh r1, [r1, #0xa]\n"
        "cmp r7, r1\n"
        "bge.n .Lstate1854__080018B2\n"
        ".Lstate1854__0800186E:\n"
        "adds r0, r4, #0\n"
        "bl _080015F4\n"
        "lsls r0, r0, #0x10\n"
        "lsrs r1, r0, #0x10\n"
        "movs r0, #0xfc\n"
        "lsls r0, r0, #2\n"
        "cmp r1, r0\n"
        "beq.n .Lstate1854__08001894\n"
        "adds r0, #1\n"
        "cmp r1, r0\n"
        "bne.n .Lstate1854__080018A6\n"
        "adds r6, #1\n"
        "b.n .Lstate1854__080018A6\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lstate1854__0800188C: .4byte 0x00007FFF\n"
        ".Lstate1854__08001890: .4byte 0x030000E4\n"
        ".Lstate1854__08001894:\n"
        "adds r0, r4, #0\n"
        "movs r1, #0\n"
        "bl _08001620\n"
        "lsls r0, r0, #0x10\n"
        "lsrs r0, r0, #0x10\n"
        "cmp r0, r5\n"
        "bge.n .Lstate1854__080018A6\n"
        "adds r5, r0, #0\n"
        ".Lstate1854__080018A6:\n"
        "adds r4, #1\n"
        "ldr r0, .Lstate1854__080018D8\n"
        "ldr r0, [r0]\n"
        "ldrh r0, [r0, #0xa]\n"
        "cmp r4, r0\n"
        "blt.n .Lstate1854__0800186E\n"
        ".Lstate1854__080018B2:\n"
        "mov r0, r8\n"
        "ldr r1, [r0]\n"
        "movs r2, #0x1c\n"
        "ldrsh r0, [r1, r2]\n"
        "cmp r0, r5\n"
        "bne.n .Lstate1854__080018DC\n"
        "ldrh r0, [r1, #0x1e]\n"
        "adds r0, #1\n"
        "strh r0, [r1, #0x1e]\n"
        "lsls r0, r0, #0x10\n"
        "asrs r0, r0, #0x10\n"
        "cmp r0, #30\n"
        "ble.n .Lstate1854__080018E2\n"
        "movs r0, #0\n"
        "strh r0, [r1, #0x1e]\n"
        "strh r5, [r1, #0x18]\n"
        "movs r7, #0\n"
        "b.n .Lstate1854__080018E2\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lstate1854__080018D8: .4byte 0x030000E4\n"
        ".Lstate1854__080018DC:\n"
        "movs r0, #0\n"
        "strh r0, [r1, #0x1e]\n"
        "strh r5, [r1, #0x1c]\n"
        ".Lstate1854__080018E2:\n"
        "ldr r4, .Lstate1854__08001930\n"
        "ldr r1, [r4]\n"
        "ldrh r0, [r1, #0xa]\n"
        "subs r0, #1\n"
        "cmp r6, r0\n"
        "bne.n .Lstate1854__080018F6\n"
        "movs r0, #4\n"
        "strh r0, [r1, #0x12]\n"
        "movs r0, #0xa\n"
        "strh r0, [r1, #0x14]\n"
        ".Lstate1854__080018F6:\n"
        "cmp r7, #0\n"
        "beq.n .Lstate1854__08001902\n"
        "ldr r1, [r4]\n"
        "ldrh r0, [r1, #0x18]\n"
        "adds r0, #1\n"
        "strh r0, [r1, #0x18]\n"
        ".Lstate1854__08001902:\n"
        "ldr r0, .Lstate1854__08001934\n"
        "bl _080016D0\n"
        "ldr r0, [r4]\n"
        "ldrh r1, [r0, #0x18]\n"
        "movs r0, #0\n"
        "bl _08001E48\n"
        "ldr r2, [r4]\n"
        "movs r3, #0x18\n"
        "ldrsh r0, [r2, r3]\n"
        "cmp r0, #0\n"
        "bne.n .Lstate1854__08001938\n"
        "ldrh r1, [r2, #0x2c]\n"
        "movs r0, #1\n"
        "bl _08001E48\n"
        "ldr r0, [r4]\n"
        "ldrh r1, [r0, #0x16]\n"
        "movs r0, #2\n"
        "bl _08001E48\n"
        "b.n .Lstate1854__08001952\n"
        ".align 2, 0\n"
        ".Lstate1854__08001930: .4byte 0x030000E4\n"
        ".Lstate1854__08001934: .4byte 0x000003ED\n"
        ".Lstate1854__08001938:\n"
        "movs r1, #0x18\n"
        "ldrsh r0, [r2, r1]\n"
        "subs r0, #1\n"
        "lsls r1, r0, #1\n"
        "adds r1, r1, r0\n"
        "lsls r1, r1, #2\n"
        "ldr r0, [r2, #0x24]\n"
        "adds r0, r0, r1\n"
        "adds r1, r2, #0\n"
        "adds r1, #0xc4\n"
        "movs r2, #6\n"
        "bl sub_0802D974\n"
        ".Lstate1854__08001952:\n"
        "ldr r0, .Lstate1854__08001984\n"
        "ldr r2, [r0]\n"
        "movs r3, #0x18\n"
        "ldrsh r0, [r2, r3]\n"
        "movs r3, #0x16\n"
        "ldrsh r1, [r2, r3]\n"
        "cmp r0, r1\n"
        "ble.n .Lstate1854__08001964\n"
        "adds r0, r1, #0\n"
        ".Lstate1854__08001964:\n"
        "adds r4, r2, #0\n"
        "adds r4, #0xd0\n"
        "lsls r0, r0, #12\n"
        "bl sub_0802DE04\n"
        "str r0, [r4]\n"
        "cmp r0, #0\n"
        "bge.n .Lstate1854__08001978\n"
        "movs r0, #0\n"
        "str r0, [r4]\n"
        ".Lstate1854__08001978:\n"
        "pop {r3}\n"
        "mov r8, r3\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lstate1854__08001984: .4byte 0x030000E4\n"
        ".syntax divided\n"
    );
}
void _08001854(void) __attribute__((alias("StateA_UpdateClockAndRecords")));
