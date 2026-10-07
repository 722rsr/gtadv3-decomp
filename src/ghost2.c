#include "gtadv/ghost.h"
#include "gtadv/memory.h"
#include "gba/bios.h"

// Reference: asm/ghost2.s 0x080240C4-0x080241C8, plus the two ghost-buffer
// helpers that live next to the manager: asm/code_24048.s 0x0802407C and the
// tail of asm/code_24ac.s 0x08002674.

void *Ghost_GetRecA(void) {
    return (void *)GhostMgr_Get()->recA;
}
void *Ghost_GetRecP(void) {
    return (void *)GhostMgr_Get()->recP;
}
// ROM 0x080240DC-0x080240EC is `ldr r0,=mgr; ldr r1,[r0,#4]; movs r0,#0;
// strh r0,[r1,#0]; bx lr` + pad + pool word, 16 B. Assigning the `u16` FIELD of
// a `volatile GhostRecord *` makes agbcc emit a redundant `ldrh` of the old
// value ahead of the store (an 18-byte body); storing through a `volatile
// u16 *` at the record base emits the ROM's five instructions and nothing else.
void Ghost_RecA_ClearValid(u32 unused) {
    (void)unused;
    volatile u16 *p = (volatile u16 *)(void *)(uintptr_t)GhostMgr_Get()->recA;
    *p = 0;
}
void Ghost_RecA_SetW08(u32 v) {
    volatile GhostRecord *r = (volatile GhostRecord *)GhostMgr_Get()->recA;
    r->timeB = v;
}
void Ghost_RecA_SetW04(u32 v) {
    volatile GhostRecord *r = (volatile GhostRecord *)GhostMgr_Get()->recA;
    r->timeA = v;
}
// ROM 0x08024104 (asm/ghost2.s:81) is `ldr r1,[pc,#4]; ldr r1,[r1,#4];
// strh r0,[r1,#2]; bx lr;.word 0x03000610` — the parameter arrives in r0 and
// is stored UNMODIFIED. A `u16` parameter makes agbcc normalise r0 first
// (`lsls r0,#16 / lsrs r0,#12`), 4 bytes the ROM does not have, so the
// original takes a full word. Storing through `volatile u16 *` also avoids the
// redundant `ldrh r2,[r1,#2]` the struct-field form emits. The sole caller
// (src/race_scene_a1.c) already passes a value narrowed to 16 bits, so the
// `strh` truncation makes the widening behaviourally identical.
void Ghost_RecA_SetH02(u32 v) {
    volatile u16 *p = (volatile u16 *)(void *)((u8 *)GhostMgr_Get()->recA + 2);
    *p = (u16)v;
}
// ROM 0x08024110 (asm/ghost2.s) is 28 B:
typedef struct { int w[4]; } GhostBlob16;

void Ghost_RecA_CopyBlob(const void *src) {
    GhostBlob16 *dst = (GhostBlob16 *)((u8 *)GhostMgr_Get()->recA + 16);
    *dst = *(const GhostBlob16 *)src;
}
void Ghost_MgrClear(void) {
    GhostMgr_Get()->state = 0;
}
void Ghost_MgrSet(void) {
    GhostMgr_Get()->state = 1;
}
u32 Ghost_MgrState(void) {
    return GhostMgr_Get()->state;
}
void Ghost_Invalidate(void *p) {
    *(volatile u16 *)p = 0;
}
// ROM span 0x08024150-0x08024158 is 8 B but the body is only 6 B: agbcc
__asm__(".align 2, 0");
// ROM 0x0802417C-0x080241C8: 60 B of code plus FOUR pool words. Transcribed:
void Ghost_Snapshot(void) {
    struct GhostStage32 { u32 w[8]; };
    extern u8 GhostSnapshotWorkArea[];
    __asm__("GhostSnapshotWorkArea = 0x03001780");
    volatile GhostMgr *m = GhostMgr_Get();
    Ghost_Invalidate((void *)(uintptr_t)m->recA);
    Ghost_Invalidate((void *)(uintptr_t)m->recP);
    {
        // `base` is a NAMED variable, not part of a constant expression: written
        // inline, `GhostSnapshotWorkArea + 0x1DF4` folds to one relocation and
        // gas resolves it to a single pool word (0x03003574). Through a variable
        // the ROM's three instructions survive -- `ldr r2,=0x03001780`,
        // `ldr r0,=0x1DF4`, `adds r1,r2,r0` -- and r2 stays live for the second
        // staging buffer, which is computed from the same base.
        u8 *base = (u8 *)(uintptr_t)GhostSnapshotWorkArea;
        // Each destination is built from the base BEFORE the source is re-read,
        // which is the ROM's order: `ldr r2,=base`, `ldr rX,=offset`,
        // `adds rX, r2, rX`, `ldr r0,[r4,#8]`, then the copy. Left alone agbcc
        // hoists the manager read above the address arithmetic and parks the
        // offset in r3, which then cannot be an LDM temp and the whole tail
        // (including both 2-word LDM/STM pairs) comes out with the wrong
        // register set. The empty `asm` with a "memory" clobber is the
        // scheduling fence that keeps the two in the ROM's order; it emits no
        // instruction.
        struct GhostStage32 *d1 = (struct GhostStage32 *)(base + 0x1DF4);
        __asm__("" : : "r"(d1) : "memory");
        *d1 = *(const struct GhostStage32 *)(uintptr_t)m->recP;
        {
            struct GhostStage32 *d2 = (struct GhostStage32 *)(base + 0x1E14);
            __asm__("" : : "r"(d2) : "memory");
            *d2 = *(const struct GhostStage32 *)(uintptr_t)m->recP;
        }
    }
}

// ROM 0x08024158-0x0802417C (32 B + one pool word) transcribes literally:
//   r1 = mgr->recA; r0 = mgr->recP; swap both; Ghost_Invalidate(old recP);
//   [mgr->recP] = 1; Ghost_MgrSet.
// The zero store and the state store are REAL CALLS in the ROM (`bl 0x08024150`
// / `bl 0x08024138`), not inline stores, so the C calls the two bodies in this
// same file instead of re-writing the stores. `m` is live across the first call
// in r4 (the ROM reloads `ldr r1,[r4,#8]`), which the named local gives.
#undef Ghost_Commit
void Ghost_Commit(void) {
    volatile GhostMgr *m = GhostMgr_Get();
    u32 a = m->recA;
    u32 p = m->recP;
    m->recA = p;
    m->recP = a;
    Ghost_Invalidate((void *)(uintptr_t)p);
    *(volatile u16 *)(uintptr_t)m->recP = 1;
    Ghost_MgrSet();
}

// --- boot-time sample-buffer binder (asm/code_24048.s third body, 0x0802407C) ---
// Sole caller: boot work-area init _08002694, with
// (0x0203D600, 0x0203CC00, 2560). ROM 0x0802407C-0x080240C4, 72 B transcribed
// instruction for instruction:
//   r3 = ptrA; r5 = bytes; r6 = mgr
//   mgr->recA = ptrA; mgr->recP = ptrP
//   r7 = 0 -> [sp+0];  n = (bytes < 0) ? bytes + 3 : bytes
//   r4 = (n << 9) >> 11;  r4 |= 0xA0000000        <- the ROM's literal, NOT
//   0x05000000: `movs r0,#160; lsls r0,#19` is 0xA0000000, OR-ed straight into
//   the count word, and no 0xFFFFF mask appears (bytes 2560 -> count 640).
//   CpuSet(sp+0, ptrA, r4)                <- dst is the REGISTER copy of ptrA
//   r7 = 0 -> [sp+4]; CpuSet(sp+4, mgr->recP, r4)  <- dst is a RELOAD of the
//   manager field, which is why the second call reads m->recP, not ptrP.
// Two separate zero words, not one: the second store sits AFTER the first
// CpuSet, so each `u32 z = 0;` lives in its own block at the point the ROM
// materialises it.
void Ghost_BindSampleBuffers(void *ptrA, void *ptrP, int bytes) {
    register s32 b __asm__("r5") = bytes;
    volatile GhostMgr *m = GhostMgr_Get();
    u32 words;
    m->recA = (u32)(uintptr_t)ptrA;
    m->recP = (u32)(uintptr_t)ptrP;
    {
        // r5 pin: agbcc coalesces a plain `s32 b = bytes;` back onto the
        // parameter pseudo, whose live range ENDS at the copy, so the two
        // values become one and the parameter is mutated in place
        // (`adds r2,#3`). Pinning b keeps them distinct, which restores the
        // ROM's `adds r5,r2,#0 / adds r0,r5,#0 / cmp r5,#0` and pushes mgr and
        // the zero word up into r6/r7. The declaration sits FIRST in the block
        // because agbcc materialises an initialiser where the declaration
        // stands, and the ROM's copy precedes the manager load and the stores.
        u32 z0 = 0;
        register u32 n __asm__("r0") = (u32)b;
        if (b < 0) n = (u32)(b + 3);
        // The `<<9` stays in r0 and only the shifted-down result reaches r4,
        // so the shift chain is written in place; fused into one expression
        // agbcc parks the whole chain in r4 (`lsls r4,r0,#9; lsrs r4,r4,#11`).
        n <<= 9;
        words = n >> 11;
        // 160 << 19, spelled so agbcc builds 0xA0000000 with `lsls #19` (the
        // ROM's form) instead of normalising to `lsls #24`.
        CpuSet(&z0, ptrA, words | (160u << 19));
    }
    {
        u32 z1 = 0;
        CpuSet(&z1, (void *)(uintptr_t)m->recP, words | (160u << 19));
    }
}

// --- sample-buffer selector (asm/code_24ac.s tail, 0x08002674) ---
// UNREFERENCED in the ROM: 0 BL callers and no VMA/literal word holds it (also
// not reachable through the manager's function-pointer slots). Slot 0 -> recA
// buffer, slot 1 -> recP buffer; the asm else-branch returns r1 untouched
// (undefined), C returns NULL instead of a stale register.
void *Ghost_SampleBufPtr(int idx) {
    register void *res __asm__("r1");
    switch (idx) {
    case 0:
        res = (void *)0x0203D600u;
        break;
    case 1:
        res = (void *)0x0203CC00u;
        break;
    }
    return res;
}

// Aliases
#ifndef __APPLE__
void  _0802407C(void *a, void *b, int n) __attribute__((alias("Ghost_BindSampleBuffers")));
void  sub_0802407C(void *a, void *b, int n) __attribute__((alias("Ghost_BindSampleBuffers")));
void *_08002674(int idx) __attribute__((alias("Ghost_SampleBufPtr")));
void *sub_08002674(int idx) __attribute__((alias("Ghost_SampleBufPtr")));
void *_080240C4(void) __attribute__((alias("Ghost_GetRecA")));
void *_080240D0(void) __attribute__((alias("Ghost_GetRecP")));
// The closure spells 0x080240d0 ONLY `sub_080240D0` (asm/passthrough.inc);
// there is no `_080240D0` closure label at all. A `sub_` label in asm is a local
// `t` symbol that no separate C object can resolve, so without this the
// reference is undefined at link time with every other gate green. Aliased to
// the REAL BODY, one hop: gcc emits `.thumb_set sub_080240D0, _080240D0` for an
// alias-of-an-alias, and the slice link splices only the body's own section, so
// an intermediate spelling would stay undefined.
void *sub_080240D0(void) __attribute__((alias("Ghost_GetRecP")));
void _080240DC(u32 v) __attribute__((alias("Ghost_RecA_ClearValid")));
void _080240EC(u32 v) __attribute__((alias("Ghost_RecA_SetW08")));
void _080240F8(u32 v) __attribute__((alias("Ghost_RecA_SetW04")));
void _08024104(u32 v) __attribute__((alias("Ghost_RecA_SetH02")));
void _08024110(const void *s) __attribute__((alias("Ghost_RecA_CopyBlob")));
void sub_08024110(const void *s) __attribute__((alias("Ghost_RecA_CopyBlob")));
void _0802412C(void) __attribute__((alias("Ghost_MgrClear")));
void _08024138(void) __attribute__((alias("Ghost_MgrSet")));
u32  _08024144(void) __attribute__((alias("Ghost_MgrState")));
u32  sub_08024144(void) __attribute__((alias("Ghost_MgrState")));
void _08024150(void *p) __attribute__((alias("Ghost_Invalidate")));
// 9-digit spelling: asm/code_24150.s calls this VMA as `_080024150` from
// `sub_080024158`, and gas treats the two spellings as distinct symbols.
void _080024150(void *p) __attribute__((alias("Ghost_Invalidate")));
void sub_08024150(void *p) __attribute__((alias("Ghost_Invalidate")));
void _08024158(void) __attribute__((alias("Ghost_Commit")));
void _0802417C(void) __attribute__((alias("Ghost_Snapshot")));
void sub_0802417C(void) __attribute__((alias("Ghost_Snapshot")));
#endif

// ----------------------------------------------------------------------------
// 0x08024068 — 12B record-kind probe (asm/code_24048.s, pool 0x030005B0):
//   p = 0x030005B0; entry = p + u32[p]*8; r0 = u32[entry+4]; r1 = u32[entry+8].
// Every caller reads r0 only (`race_scene_c.c` treats it as the record-kind
// id; menu_ff78_f/o/p compare it against 16/39), so the C body returns that
// word; the r1 side-word has no C reader.
int Ghost_Kind_24068(void) {
    volatile u32 *base = (volatile u32 *)0x030005B0u;
    u32 i = *base;
    volatile u32 *entry = (volatile u32 *)((volatile u8 *)base + i * 8u);
    u32 a = entry[1]; // [p + idx*8 + 4] — the returned kind id
    u32 b = entry[2]; // [p + idx*8 + 8] — volatile side read the ROM keeps
    (void)b;
    return (int)a;
}
#ifndef __APPLE__
int _08024068(void) __attribute__((alias("Ghost_Kind_24068")));
int sub_08024068(void) __attribute__((alias("Ghost_Kind_24068")));
int Sub_08024068(void) __attribute__((alias("Ghost_Kind_24068")));
#endif
