#include "gtadv/idle.h"
#include "gtadv/memory.h"
#include "gba/types.h"

// Lifted small accessors from asm/idle.s 0x08001988–0x080020E8
// Each is a direct translation of the Thumb asm; all offsets are byte
// offsets from State Block A base (IWRAM 0x03000008 via slot 0x030000E4).

// Callees reached from the lifted bodies below, each spelled as the label the
// slice closure `asm/` actually defines at its true address, so every call
// resolves under the spliced ROM (asm/ `sub_`/`_` labels are LOCAL `t`
// symbols; only a spelling some closure file defines can be called):
//   _08001724     = StateA_SetReqMode           (state_block_a.c)   0x08001724
//   _08001FE0     = state-block special-mode test(runtime_accessors.c)     0x08001FE0
//   sub_0802D974  = CpuSet, swi 0x0B              (bios_wrappers.c)  0x0802d974
//   _0800295C     = Warn / Foundation_InitCommon (foundation_boot.c) 0x0800295c
// Each is defined in C (sub_0802D974 is bios_wrappers.c:62, an alias of the
// naked CpuSet), so these calls land on the maintained C owner rather than on
// a ROM veneer.
// `_08001724`, `sub_0802D974` and `_0800295C` are aliases that exist only in
// the ARM build; the host build keeps calling the real bodies by name.
#ifndef __APPLE__
extern void _08001724(u16 mode);                      // StateA_SetReqMode
extern void sub_0802D974(const void *src, void *dst, u32 ctrl); // CpuSet (swi 0x0B)
extern void _0800295C(u32 a, u32 b);                   // Warn (foundation_boot.c)
#else
extern void StateA_SetReqMode(u16 mode);               // 0x08001724
extern void CpuSet_2D974(const void *a, void *b, unsigned c);
extern void Warn(u32 a, u32 b);                        // 0x0800295c
#endif
extern int  _08001FE0(int m);                         // 0x08001FE0 (runtime_accessors.c)
extern void _08001F80(u32 v);                          // Idle_SetRecordCount (below)
extern void _08001F08(u32 a);                          // Idle_SetupMode3 (idle_dispatch.c)
extern int  Ai_IdMap(int a);                           // _080022E4 (ai_catalog.c)

static inline volatile u8 *st_u8(int off) {
    volatile u32 *slot = (volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    volatile u8 *base = (volatile u8 *)(uintptr_t)*slot;
    return base + off;
}
static inline volatile u16 *st_u16(int off) { return (volatile u16 *)st_u8(off); }
static inline volatile u32 *st_u32(int off) { return (volatile u32 *)st_u8(off); }

// _08001E48(id, val) — write payload at +0xC0 + (id+1)*2
//
// Two levers, both measured on this body :
//
// 1. `payload` is WORD-width, and that is a ROM fact, not a preference. The
//    ROM's `strh r1,[r2,#0]` stores the incoming r1 untouched -- there is no
//    `lsls r1,#16 / lsrs r1,#16` narrowing pair, and every u16-typed body
//    emits one (4 bytes the ROM does not have). Callers all pass a halfword
//    value, which zero-extends into the wider parameter exactly as the u16
//    contract did, so the wider signature is behaviour-preserving.
// 2. The +0xC0 must be ITS OWN statement. `st_u16(0xC0 + ((id+1) << 1))`
//    folds all three constants at compile time into `adds r0,#194` and then
//    adds the base ONCE (24 bytes, OVERSIZED at 2/20); the ROM adds the base
//    twice -- `adds r2,#192` then `adds r2,r2,r0`. Written as
//    `base += 0xC0;` before the offset add, the two `adds` separate and the
//    body is byte-exact: 20/20 EXACT including the 0x030000E4 pool word.
void Idle_WriteEvent(int id, u32 payload) {
    u8 *base = (u8 *)(uintptr_t)*(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    u32 off = (u32)(id + 1) << 1;
    base += 0xC0;
    *(volatile u16 *)(base + off) = (u16)payload;
}

// _08001E5C(idx0, idx1) -> u16 — record halfword (current sel)
// ROM order: table base load, idx0<<2, slot load, st load, sel ldrh,
// sel<<4, two adds, table ldr, (idx1+1)<<1, entry<<4, sum, st+0x70, add, ldrh.
// The table base is an absolute symbol (course_cal.c pattern): as a folded
// integer constant the reload pass sinks the load to its point of use.
// Measured 11/48 -> 44/48, exact length, first diff +0x0.
// The three register pins close the rest (44/48 -> 45/48 -> 48/48 EXACT):
// local-alloc assigns shortest-lived quantities the lowest regs, so sel
// (live for 2 insns) always takes r2 and tbl/st pack into r3/r2. Pinning
// sel->r5 and st->r3 forces the ROM's r5/r3 layout (tbl then takes r4, the
// only free reg overlapping its range); pinning selOff->r2 stops it
// coalescing into sel (`lsls r5,r5,#4`) and forces `lsls r2,r5,#4`.
extern const u8 RecTable1E5C[];
u16 Idle_GetRecordHalfword(int idx0, int idx1) {
    const u8 *tbl;
    u32 idxOff;
    volatile u32 *slot;
    register volatile u8 *st __asm__("r3");
    register u16 sel __asm__("r5");
    register u32 selOff __asm__("r2");
    u32 entry;
    u32 halfOff;
    u32 off;
    volatile u8 *p;
    __asm__(".globl RecTable1E5C\nRecTable1E5C = 0x0802E190\n");
    tbl = RecTable1E5C;
    idxOff = (u32)idx0 << 2;
    slot = (volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    st = (volatile u8 *)(uintptr_t)*slot;
    sel = *(volatile u16 *)(st + 6);
    selOff = (u32)sel << 4;
    idxOff += selOff;
    entry = *(const u32 *)(tbl + idxOff);
    halfOff = (u32)(idx1 + 1) << 1;
    off = halfOff + (entry << 4);
    p = st + 0x70;
    return *(volatile u16 *)(p + off);
}

// _08001E8C(recOff, hwIdx) -> u16 — direct
// Both edits below are load-bearing; each alone scores 19/24 or 22/24.
//  * `halfOff + (recOff << 4)` operand order. agbcc evaluates the add with the
//    LEFT operand in the destination, so the sum lands in r1 (halfOff's own
//    register) rather than r0, which is what the ROM does at 0x08001E96
//    (`adds r1,r1,r0`). The reverse order puts the sum in r0.
//  * `p = st + 0x70` as a named temp. agbcc folds a pointer+constant chain
//    `st + 0x70 + off` into a single address expression and reassociates it
//    into r0 (`adds r0,r0,r2` / `adds r0,#112`); naming the biased base keeps
//    the ROM's two steps at 0x08001E98/0x08001E9A, in r2.
u16 Idle_GetDirectHalfword(int recOff, int hwIdx) {
    volatile u8 *st = st_u8(0);
    u32 halfOff = (u32)(hwIdx + 1) << 1;
    u32 off = halfOff + ((u32)recOff << 4);
    volatile u8 *p = st + 0x70;
    return *(volatile u16 *)(p + off);
}

// _08001EA4(hwIdx) -> u16 — st[0x70 + (hwIdx+1)*2]
// All THREE intermediates must be named, and in the ROM's own order. agbcc
// materialises each initialiser where its declaration stands, so:
//   * one inline `st + 0x70 + off` folds the whole chain into one address
//     expression and reassociates it into r0 (`adds r0,r0,r1; adds r0,#0x70`);
//   * naming only the biased base `p` is not enough either -- with `p` declared
//     first, `adds r1,#0x70` is emitted straight after the pool load, before
//     the index is computed;
//   * declaring `p` after `off` hoists the index computation above the pool
//     load instead.
// `st`, then `off`, then `p = st + 0x70` reproduces all three ROM steps in
// order (0x08001EA4/0x08001EA6 load, 0x08001EA8/0x08001EAA index,
// 0x08001EAC bias, 0x08001EAE add), all in r1.
u16 Idle_GetIndexedHalfword2(int hwIdx) {
    volatile u8 *st = st_u8(0);
    u32 off = (u32)(hwIdx + 1) << 1;
    volatile u8 *p = st + 0x70;
    return *(volatile u16 *)(p + off);
}

bool Idle_IsMode1(void) {
    bool r1 = 0;
    volatile u32 *slot = (volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    volatile u8 *base = (volatile u8 *)(uintptr_t)*slot;
    if (*(volatile u16 *)(base + 0x10) == 1)
        r1 = 1;
    return r1;
}

void *Idle_GetRecordPtr(int idx) {
    register const u8 *tbl __asm__("r3");
    u32 idxOff;
    volatile u32 *slot;
    register volatile u8 *st __asm__("r2");
    register u16 sel __asm__("r4");
    register u32 selOff __asm__("r1");
    u32 entry;
    u32 off;
    __asm__(".globl RecTable1E5C\nRecTable1E5C = 0x0802E190\n");
    tbl = RecTable1E5C;
    idxOff = (u32)idx << 2;
    slot = (volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    st = (volatile u8 *)(uintptr_t)*slot;
    sel = *(volatile u16 *)(st + 6);
    selOff = (u32)sel << 4;
    idxOff += selOff;
    idxOff += (uintptr_t)tbl;
    entry = *(const u32 *)idxOff;
    off = (entry << 4) + 0x70;
    st += off;
    return (void *)st;
}

// 0x08001F24 / 0x08001F30 — 10-byte thunks onto _08001F3C (asm/idle.s:838-852).
// ROM: `push {lr}; bl _08001F3C; pop {r1}; bx r1` + `movs r0,r0` pad, i.e. an
// unmodified tail call, exactly like the promoted 0x080022C0 forwarder. `int`
// return (not bool) so agbcc does not insert a narrowing sequence after the
// call; the callee already leaves 0/1 in r0 and the tail call forwards it.
#ifdef __APPLE__
int Idle_Wrap_1F24(void) { return Idle_IsMode1(); }
int Idle_Wrap_1F30(void) { return Idle_IsMode1(); }
#else
bool _08001F3C(void);
int Idle_Wrap_1F24(void) { return _08001F3C(); }
// Each 10-byte body ends in a 12-byte span whose last halfword the ROM holds
// as `00 00`. gas closes a section with the 2-byte nop (0x46c0) by default, so
// fill the alignment explicitly with zero (same fix as Idle_ArenaRegister).
__asm__(".align 2, 0");
int Idle_Wrap_1F30(void) { return _08001F3C(); }
__asm__(".align 2, 0");
#endif

// _08001CB4 -> u32 — raw word at st+0xD0 (asm/idle.s:455).
u32 Idle_GetD0Word(void) {
    return *st_u32(0xD0);
}

// _08001CC4 -> s32 — st+0xD0 halved, rounding toward zero (asm/idle.s:467):
//   lsrs r1,r0,#31; adds r0,r0,r1; asrs r0,r0,#1
s32 Idle_GetD0WordHalved(void) {
    u32 v = *st_u32(0xD0);
    s32 t = (s32)(v + (v >> 31));
    return t >> 1;
}

// _08001F98(arg) — request mode 6 then latch arg at st+0x24 (asm/idle.s:918).
// Twin of Idle_RequestMode10 below; the ROM pair is byte-identical apart from
// the immediate (`movs r0,#6` / `str r4,[r0,#0x24]` vs `#10` / `#0x28`).
void Idle_RequestMode6(u32 arg) {
#ifndef __APPLE__
    _08001724(6);
#else
    StateA_SetReqMode(6);
#endif
    *st_u32(0x24) = arg;
}

// _08001FB4(arg) — request mode 10 then latch arg at st+0x28 (asm/idle.s:930).
void Idle_RequestMode10(u32 arg) {
#ifndef __APPLE__
    _08001724(10);
#else
    StateA_SetReqMode(10);
#endif
    *st_u32(0x28) = arg;
}

// _08002014 -> bool — special-mode test over BOTH the current (+0x10) and
// the requested (+0x12) mode (asm/idle.s:991): the ROM ORs _08001FE0 of each,
// i.e. false only when both are in {10,11}.
int Idle_IsSpecialModeAny(void) {
    int a = _08001FE0(*st_u16(0x10));
    int b = _08001FE0(*st_u16(0x12));
    return a | b;
}

// _0800207C(handle, region, len) — arena bookkeeping helper (asm/idle.s:1063):
// CpuSet 32-bit zero-fill of `region` (word count = (len with a +3 bias while
// negative) >> 2, ctrl 0x05000000 = 32-bit | fill), then latch {region, len}
// into the 2-word handle at `handle`.
void Idle_ArenaRegister(void *handle, void *region, int len) {
    u32 zero = 0;
    int n = len;
    if (n < 0) n += 3;
    u32 ctrl = 0x05000000u | (((u32)n << 9) >> 11);
#ifndef __APPLE__
    sub_0802D974(&zero, region, ctrl);
#else
    CpuSet_2D974(&zero, region, ctrl);
#endif
    ((u32 *)handle)[0] = (u32)(uintptr_t)region;
    ((u32 *)handle)[1] = (u32)len;
}
// The body is 50 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");

// _080020B0(handle, amt) — 2-word arena consume (asm/idle.s tail, unlabelled
// `.type`; every caller uses the numeric `bl 0x080020B0` form — 0x08005080 and
// the menu/save allocators). Returns the *old* head:
//   old = h[0]; if (amt == 0) return 0;
//   have = h[1]; if ((s32)have < (s32)amt) _0800295C(0x0802E1DC, amt - have);
//   h[0] = h[0] + amt; h[1] -= amt; return old;
// `have` caches h[1] so the compare and the Warn argument share ONE cell load
// (the ROM has a single `ldr r1,[r4,#4]` feeding both), the compare is SIGNED
// (the ROM branches `bge`, not `bcs`), and h[0] is re-read for the store rather
// than reusing `old` -- the ROM reloads it after the _0800295C call.
void *ArenaConsume(void *handle, u32 amt) {
    volatile u32 *h = (volatile u32 *)handle;
    u32 old = h[0];
    u32 have;
    if (amt == 0) return 0;
    have = h[1];
#ifndef __APPLE__
    if ((s32)have < (s32)amt) _0800295C(0x0802E1DCu, amt - have);
#else
    if ((s32)have < (s32)amt) Warn(0x0802E1DCu, amt - have);
#endif
    h[0] = h[0] + amt;
    h[1] = h[1] - amt;
    return (void *)(uintptr_t)old;
}
#ifndef __APPLE__
void *_080020B0(void *a, u32 b) __attribute__((alias("ArenaConsume")));
void *sub_080020B0(void *a, u32 b) __attribute__((alias("ArenaConsume")));
#endif

// _08001F80(v) / _08001F8C — record count +0x0A
//
// The VMA owner is WORD-typed and the `u16` entry point is a wrapper, because
// the ROM's argument is a word. agbcc's `assign_parms` converts a sub-word
// parameter to Pmode in the prologue (gcc_arm/function.c), and GCC 2.95 only
// deletes that conversion when the parameter is entirely unused -- so a `u16`
// declaration costs a real `lsls r0,#16 / lsrs r0,#16` pair. asm/idle.s:900-903
// has no such pair: `ldr r1,=0x030000E4 / ldr r1,[r1] / strh r0,[r1,#10] /
// bx lr` over a 0x030000E4 pool, 12 B, so the incoming argument is a word and
// `strh` is what drops its high half. Measured in isolation: with `u32 v` or
// `int v` the body is those four instructions and the pool word, byte for
// byte; with `u16 v` it is the same plus the two shifts (16 B).
// The wrapper keeps the declared `u16` contract that include/gtadv/idle.h and
// src/car_tick_helpers.c both rely on, so no header or caller moves, and the
// two entry points are the same machine behaviour: a halfword store of the
// low half of r0. The `extern _08001F80(u16)` at the top of this file is
// likewise untouched, so Idle_2298_ReloadCurrent's call site keeps the
// `u16`-declared argument it is byte-matched against today.
void Idle_SetRecordCountW(u32 v) { *st_u16(0x0A) = v; }
void Idle_SetRecordCount(u16 v) { Idle_SetRecordCountW(v); }
// Returns `int`, not `u16`. Two promoted callers compare the result against a
// SIGNED loop bound, and with a `u16` declaration agbcc zero-extends r0 before
// the compare, which the ROM does not do (its caller is a bare `cmp r6,r0`).
// The value is 0-65535 by construction, so widening is lossless, and a `u16`
// here with `int` at the call sites was a cross-TU mismatch -- undefined
// behaviour, and the header had to move with it.
int Idle_GetRecordCount(void) { return (int)*st_u16(0x0A); }

// _08002038 -> current mode
u16 Idle_GetCurrentMode(void) { return *st_u16(0x10); }
// _08002044 -> +0x0C
u16 Idle_GetCounterC(void) { return *st_u16(0x0C); }
// _08002050 -> +0xD8
u16 Idle_GetD8Word(void) { return *st_u16(0xD8); }
// _08002060(f) set +0x02 flag. Same word-argument argument as _08001F80 (see
// above): asm/idle.s:1068-1071 is `ldr r1,=0x030000E4 / ldr r1,[r1] /
// strb r0,[r1,#2] / bx lr` with no zero-extend of r0, so the VMA owner takes
// a word and the `u8` prototype in include/gtadv/idle.h is kept as a wrapper.
void Idle_SetFlag2W(u32 v) { *st_u8(0x02) = v; }
void Idle_SetFlag2(u8 v) { Idle_SetFlag2W(v); }

// _0800206C -- wrapper over _080015F4, u16. The name is deliberately NOT
// followed by '(' : tools/corpus_match_probe.py's VMA_DEF treats a VMA-shaped
// name immediately followed by '(' as a C DEFINITION site, and a comment
// saying `_0800206C(idx)` would steal the attribution from the real owner,
// src/menu_d280.c:21, and make the body unscorable.
u16 Idle_GetRecordFiltered(int idx) {
    // ROM (16 B): push {lr} / bl _080015F4 / lsls r0,#16 / lsrs r0,#16 /
#ifndef __APPLE__
    extern u32 _080015F4(int idx);
#else
    extern u16 StateA_GetRecordHalfword(int idx);
#endif
#ifndef __APPLE__
    return (u16)_080015F4(idx);
#else
    return (u16)StateA_GetRecordHalfword(idx);
#endif
}
// 16-byte body in a 16-byte span: gas closes the section with the 2-byte nop
// (0x46c0) where the ROM holds `00 00` at 0x0800207A. Emitted after the body's
// `.size`, still inside its own section, so it pads with the explicit `0` fill.
__asm__(".align 2, 0");

// Aliases
#ifndef __APPLE__
void _08001E48(int a, u32 b) __attribute__((alias("Idle_WriteEvent")));
u16 _08001E5C(int a, int b) __attribute__((alias("Idle_GetRecordHalfword")));
u16 _08001E8C(int a, int b) __attribute__((alias("Idle_GetDirectHalfword")));
u16 _08001EA4(int a) __attribute__((alias("Idle_GetIndexedHalfword2")));
bool _08001F3C(void) __attribute__((alias("Idle_IsMode1")));
bool sub_08001F3C(void) __attribute__((alias("Idle_IsMode1")));
void *_08001F54(int idx) __attribute__((alias("Idle_GetRecordPtr")));
void *sub_08001F54(int idx) __attribute__((alias("Idle_GetRecordPtr")));
u32  _08001CB4(void) __attribute__((alias("Idle_GetD0Word")));
s32  _08001CC4(void) __attribute__((alias("Idle_GetD0WordHalved")));
int _08001F24(void) __attribute__((alias("Idle_Wrap_1F24")));
int _08001F30(void) __attribute__((alias("Idle_Wrap_1F30")));
void _08001FB4(u32 v) __attribute__((alias("Idle_RequestMode10")));
void _08001F98(u32 v) __attribute__((alias("Idle_RequestMode6")));
int  _08002014(void) __attribute__((alias("Idle_IsSpecialModeAny")));
void _0800207C(void *h, void *region, int len) __attribute__((alias("Idle_ArenaRegister")));
void _08001F80(u32 v) __attribute__((alias("Idle_SetRecordCountW")));
int _08001F8C(void) __attribute__((alias("Idle_GetRecordCount")));
u16 _08002038(void) __attribute__((alias("Idle_GetCurrentMode")));
u16 _08002044(void) __attribute__((alias("Idle_GetCounterC")));
u16 _08002050(void) __attribute__((alias("Idle_GetD8Word")));
void _08002060(u32 v) __attribute__((alias("Idle_SetFlag2W")));
void Sub_08002060(u32 v) __attribute__((alias("Idle_SetFlag2W")));
u16 _0800206C(int a) __attribute__((alias("Idle_GetRecordFiltered")));
#endif

// 0x080022C0 — 4B forwarder: `bl 0x08001F3C; bx lr` (pop {r1}; bx r1 tail).
// All call sites read it as `mode == 1`.
#ifndef __APPLE__
int Idle_22C0_IsMode1(void) { return _08001F3C(); }
#else
int Idle_22C0_IsMode1(void) { return Idle_IsMode1(); }
#endif

// 0x08002298 — 20B: `B = *(u32*)0x030000F4; u8[B+1] = 1; r0 = s16[B+8];
// _08001F80(r0); _08001F08(rec)` ((consolidated/elsewhere): state-block-B arm — mark
// flag 1, forward s16[B+8] to the setter, run the record's setup arm).
#ifndef __APPLE__
__attribute__((naked)) void Idle_2298_ReloadCurrent(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "adds r4, r0, #0\n"
        "ldr r2, 1f\n"
        "ldr r1, [r2, #0]\n"
        "movs r0, #1\n"
        "strb r0, [r1, #1]\n"
        "ldr r0, [r2, #0]\n"
        "movs r1, #8\n"
        "ldrsh r0, [r0, r1]\n"
        "bl _08001F80\n"
        "adds r0, r4, #0\n"
        "bl _08001F08\n"
        "pop {r4}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        "1: .4byte 0x030000F4\n"
        ".syntax divided\n"
    );
}
#else
void Idle_2298_ReloadCurrent(void *rec) { (void)rec; }
#endif

// 0x08002340 — 8B..0x68: linear search `for (i = 0; i <= 98; i++)
// if (s16[Ai_IdMap(i)] == v) return i; return -1` over the car-id map
// ((consolidated/elsewhere)). Callers (asm/carphys_tick.s:249/332) store the result
// halfword into the packet.
// ROM: the counter is a word (r5) and each probe id is its s16 narrowing
// (`lsls/asrs` into r4), which is both the call argument and the result.
int Idle_IdSearch(int v) {
    s16 want = (s16)v;
    int i;
#ifndef __APPLE__
    extern int _080022E4(int a);  // closure spelling of Ai_IdMap
#endif
    for (i = 0; i <= 98; i++) {
        s16 id = (s16)i;
#ifndef __APPLE__
        if ((s16)_080022E4(id) == want) return id;
#else
        if ((s16)Ai_IdMap(id) == want) return id;
#endif
    }
    return -1;
}
// Trap 6: content is 2 (mod 4); pad the body's own section with `00 00`.
__asm__(".align 2, 0");

// 0x08002370 — 20B: `return &0x080C44B2[v * 7]` — row getter into the
// per-mode 28-byte table (asm/code_22e4.s; pool 0x08002380).
// Armcc register allocation: r0 sign-extended, r1 = r0*7, r0 = pool, r1 += r0, r0 = r1.
__attribute__((naked)) void *Idle_ModeRow(s16 v) {
    __asm__ volatile (
        ".syntax unified\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #16\n"
        "lsls r1, r0, #3\n"
        "subs r1, r1, r0\n"
        "ldr  r0, 1f\n"
        "adds r1, r1, r0\n"
        "adds r0, r1, #0\n"
        "bx   lr\n"
        "1: .word 0x080C44B2\n"
        ".syntax divided\n"
    );
}

// 0x08002384 — 4B: empty stub leaf (bx lr + nop pad)
__attribute__((naked)) void Idle_Stub_2384(void) {
    __asm__ volatile (
        "bx lr\n"
        ".short 0\n"
    );
}

#ifndef __APPLE__
int   _08002340(int v) __attribute__((alias("Idle_IdSearch")));
int   sub_08002340(int v) __attribute__((alias("Idle_IdSearch")));
void *_08002370(s16 v) __attribute__((alias("Idle_ModeRow")));
void *sub_08002370(s16 v) __attribute__((alias("Idle_ModeRow")));
void  _08002384(void) __attribute__((alias("Idle_Stub_2384")));
void  sub_08002384(void) __attribute__((alias("Idle_Stub_2384")));
void  _08002298(void *a) __attribute__((alias("Idle_2298_ReloadCurrent")));
void  sub_08002298(void *a) __attribute__((alias("Idle_2298_ReloadCurrent")));
void  Sub_08002298(void *a) __attribute__((alias("Idle_2298_ReloadCurrent")));
int   _080022C0(void) __attribute__((alias("Idle_22C0_IsMode1")));
int   sub_080022C0(void) __attribute__((alias("Idle_22C0_IsMode1")));
int   Sub_080022C0(void) __attribute__((alias("Idle_22C0_IsMode1")));
#endif
