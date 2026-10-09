#include "gtadv/foundation.h"
#ifndef __APPLE__
// rule 6: the splice replaces this label. ARM-only: clang on darwin rejects
// alias attributes outright, and no host test compiles this TU, so the
// unguarded form was a latent host-build break no gate could see.
void *sub_08004DF4(void *a) __attribute__((alias("Leaf_04DF4")));
#endif
#include "gtadv/memory.h"
#include "gba/bios.h"
#include "gba/types.h"

// Subsystem loader and manager routines. The C bodies preserve the observed
// manager fields, callback order, and BIOS fill controls from the ROM.

extern void BiosCpuFastSet(const void *s, void *d, u32 m);
extern void Warn(u32 a, u32 b); // sub_0800295C — fatal error reporter (ROM-exact trampoline)
// asm/sound_d974.s labels this address `sub_0802D974` (and only that
// spelling); the C owner is the naked CpuSet (bios_wrappers.c:62). Call the
// closure's spelling so the spliced ROM resolves it. The alias is ARM-only,
// so the host build keeps calling the real body by its friendly name.
#ifndef __APPLE__
extern void sub_0802D974(const void *a, void *b, unsigned c);
#else
extern void CpuSet_2D974(const void *a, void *b, unsigned c);
#endif

// _080049AC — "SCENE MEM ALLOC" bump allocator for subsystem instances.
// Manager block at absolute 0x03000198 (not dereferenced — see the ROM pool
// at 0x080049F8): heap base +0x84, cursor +0x88, size +0x8C, remaining +0x90
// (SubsysStoreConfig / _08004A0C set base/size, HeapReset re-derives them).
// Returns the OLD cursor and zero-fills the block with a CpuSet 32-bit fill
// of (n >> 2) words — the ROM rounds n down to whole words (lsls #9 / lsrs
// #11), leaving the n&3 tail bytes untouched. n == 0 returns 0 without a
// bump. If n exceeds the remaining bytes the ROM reports (tag
// "SCENE MEM ALLOC" 0x0805BA60, deficit n-rem) via sub_0800295C — its fatal
// error handler, which never returns. Body 0x080049AC..0x08004A0A.
void *AllocSlot(int n) {
    volatile u8 *mgr;
    register int size __asm__("r4");
    register volatile u32 *cursor __asm__("r6");
    register volatile u32 *remainingPtr __asm__("r5");
    void *blk;
    register u32 rem __asm__("r1");
    __asm__(".globl AllocMgr_49AC\nAllocMgr_49AC = 0x03000198\n");
    extern u8 AllocMgr_49AC[];
    size = n;
    mgr = AllocMgr_49AC;
    cursor = (volatile u32 *)(mgr + 0x88);
    blk = (void *)(uintptr_t)*cursor;
    if (size != 0) {
        remainingPtr = (volatile u32 *)(mgr + 0x90);
        rem = *remainingPtr;
        __asm__ volatile("" : "+r"(rem));
        if ((u32)size > rem)
            Warn(0x0805BA60, (u32)size - rem); // fatal report (ROM never returns)
        *cursor = *cursor + (u32)size;
        *remainingPtr = *remainingPtr - (u32)size;
        u32 z = 0;
#ifndef __APPLE__
        sub_0802D974(&z, blk, 0x05000000u | (((u32)size << 9) >> 11));
#else
        CpuSet_2D974(&z, blk, 0x05000000u | (((u32)size << 9) >> 11));
#endif
        return blk;
    }
    return 0;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void *_080049AC(int n) __attribute__((alias("AllocSlot")));
void *sub_080049AC(int n) __attribute__((alias("AllocSlot")));
#endif
#define SUBSYS_MGR (*(volatile u32**)0x03000198)

// Manager/loader — substantiated against src/foundation_subsys.c
// _08004A2C(heapBase,size): stores base at mgr+0x84 size at +0x8C then HeapReset (_08004A0C)
void SubsysStoreConfig(u32 base, u32 size) {
    volatile u32 *mgr = (volatile u32*)0x03000198;
    mgr[33] = base; // +132
    mgr[35] = size; // +140
    // The slice links no C object, so a promoted body may only call a
    // spelling the closure defines. That alias lives under `#ifndef __APPLE__`
    // below, so the host build keeps the friendly name -- an undeclared call
    // here is a silent C89 implicit declaration, and a host link with
    // `-undefined dynamic_lookup` binds lazily, so nothing would report it.
#ifndef __APPLE__
    extern void sub_08004A0C(void);
    sub_08004A0C(); // cursor=base free=size
#else
    extern void HeapReset(void); // same file, defined below
    HeapReset();
#endif
}
#ifndef __APPLE__
void _08004A2C(u32 a,u32 b) __attribute__((alias("SubsysStoreConfig")));
void HeapInit(u32 a,u32 b) __attribute__((alias("SubsysStoreConfig")));
#endif

// _08004A48(record,idx): zero 24B at 0x030001A8 via CpuSet, swaps idx/prev in
// +0x00/+0x62, stores record at +0x50, allocs scene RAM at +0x54, optional hdr
// node at +0x4C.
// The ROM epilogue returns the context word in r0; _08004AA4 stores that result
// at mgr+8. The lifted body now follows that return path and the observed
// context fields directly.
void *SubsysCreateInstance(void *record, int idx) {
    volatile u8 *ctx = (volatile u8*)0x030001A8;
    u32 z=0;
#ifndef __APPLE__
    sub_0802D974(&z, (void*)ctx, 0x05000018); // 24 B zero fill (verified ctrl)
#else
    CpuSet_2D974(&z, (void*)ctx, 0x05000018);
#endif
    *(volatile u16*)(ctx+0x00) = (u16)idx;
    u16 prev = *(volatile u16*)(ctx+0x62);
    *(volatile u16*)(ctx+0x02) = prev;
    *(volatile u16*)(ctx+0x62) = (u16)idx;
    *(volatile void**)(ctx+0x50) = record;
    int sz = *(int*)((u8*)record + 8);
#ifndef __APPLE__
    void *blk = _080049AC(sz);
#else
    void *blk = AllocSlot(sz);
#endif
    *(volatile void**)(ctx+0x54) = blk;
    if (*(volatile void**)record) {
#ifndef __APPLE__
        void *node = _080049AC(8);
#else
        void *node = AllocSlot(8);
#endif
        *(volatile void**)(ctx+0x4C) = node;
        // `volatile void *`: the load at record+0 is a volatile read, and
        // reading it into a plain `void *` discards the qualifier. The byte
        // sequence is unchanged -- only the C type of the loaded value.
        volatile void *hdr = *(volatile void**)record;
        *(void**)node = (void *)hdr;
#ifndef __APPLE__
        void *data = _080049AC(*(int*)((u8*)hdr+4));
#else
        void *data = AllocSlot(*(int*)((u8*)hdr+4));
#endif
        *(void**)((u8*)*(void**)(ctx+0x4C)+4) = data;
    }
    return (void *)ctx; // ROM: r0 = r5 = ctx before the bx r1 tail
}
#ifndef __APPLE__
void *_08004A48(void *a,int b) __attribute__((alias("SubsysCreateInstance")));
#endif

// _08004AF4(desc): clear the descriptor table, install the manager callbacks
// and table pointer, clear manager counters, then load the selected instance.
void SubsysInit(void *desc) {
    register volatile u8 *d __asm__("r5");
    register volatile u32 *mgr __asm__("r4");
    register u32 count __asm__("r1");
    u32 zero;
    register u32 clear_bytes __asm__("r0");
    d = (volatile u8 *)desc;
    count = *(volatile u16 *)(d + 2);
    __asm__("" : "+r"(count));
    clear_bytes = count << 3;
    __asm__("" : "+r"(clear_bytes));
    if (clear_bytes != 0) {
        register u32 raw_count __asm__("r0");
        register u32 words __asm__("r2");
        void *dst;
        zero = 0;
#ifndef __APPLE__
        dst = (void *)(uintptr_t)*(volatile u32 *)(d + 12);
        raw_count = *(volatile u16 *)(d + 2);
        __asm__("" : "+r"(raw_count));
        words = raw_count << 1;
        words |= 0x05000000u;
        sub_0802D974(&zero, dst, words);
#else
        CpuSet_2D974(&zero, (void *)(uintptr_t)*(volatile u32 *)(d + 12),
                     0x05000000u | (u32)*(volatile u16 *)(d + 2) << 1);
#endif
    }
    mgr = (volatile u32 *)(uintptr_t)0x03000198u;
    mgr[0] = *(volatile u32 *)(d + 8);
    mgr[1] = *(volatile u32 *)(d + 4);
    mgr[3] = *(volatile u32 *)(d + 12);
#ifndef __APPLE__
    extern void sub_08004CC4(void);
    extern void _08004AA4(int);
    sub_08004CC4();
    zero = 0;
    mgr += 37;
    sub_0802D974(&zero, (void *)mgr, 0x05000008u);
    _08004AA4(*(volatile u16 *)d);
#else
    extern void SubsysLoadInstance(int idx);
    CounterClear();
    CpuSet_2D974(&zero, (void *)(uintptr_t)0x0300022Cu, 0x05000008u);
    SubsysLoadInstance(*(volatile u16 *)d);
#endif
}
#ifndef __APPLE__
void _08004AF4(void *a) __attribute__((alias("SubsysInit")));
#endif

// _08004B50: state = mgr[+8]; fn = mgr[+4]; call fn(state) through the
// 0x0802DDCC `bx r1` veneer; load the returned index as the next subsystem.
//
// The ROM materialises the manager base in r1 and consumes it twice --
// `ldr r0,[r1,#8]` (the argument) then `ldr r1,[r1,#4]` (the function pointer)
// -- because both arguments come from one base register. Writing the base as
// a local is what keeps the pool word 0x03000198 instead of agbcc folding the
// two folded addresses into two separate literals.
void SceneAdvance(void) {
    void *base = (void *)(uintptr_t)0x03000198u;
#ifndef __APPLE__
    extern void *_0802DDCC(void *a, void *b);
    extern void _08004AA4(int a);
    _08004AA4((int)(uintptr_t)_0802DDCC(*(void **)((u8 *)base + 8),
                                             *(void **)((u8 *)base + 4)));
#else
    extern void *RelocatedDispatch(void *a, void *b);
    extern void SubsysLoadInstance(int idx);
    SubsysLoadInstance((int)(uintptr_t)RelocatedDispatch(*(void **)((u8 *)base + 8),
                                                        *(void **)((u8 *)base + 4)));
#endif
}
#ifndef __APPLE__
void _08004B50(void) __attribute__((alias("SceneAdvance")));
void sub_08004B50(void) __attribute__((alias("SceneAdvance")));
void SceneShutdown(void) __attribute__((alias("SceneAdvance")));
#endif

// Four handler groups hang off the manager at +0x4C (optional node), +0x50
// with its payload at +0x54, +0x58 (null-terminated list) and +0x5C
// (optional node). Each broadcasts (a, b, c, payload).
//
// Every broadcast ends in `bl 0x0802DDD8`, which is the
// `__aeabi_call_via_r4` veneer (`bx r4` + a `mov r8,r8` pad) -- NOT a call to
// a function of that address. agbcc emits the `_call_via_rN` name itself for
// a call through a function pointer it has placed in r4, so the handler
// pointer is the callee's register rather than a fifth argument. That is why
// `_0802DDD8` has no C body in this tree and needs none: it is entry 5 of the
// 15-veneer pool at asm/sound_veneer.s (VMA 0x0802DDC8..0x0802DE04).
//
// The first group omits `mov r2,r8` because the prologue's `mov r8,r2` leaves
// the caller's r2 still holding `c`; re-materialising it would be a no-op, so
// the compiler emits nothing. All four groups therefore pass the same four
// arguments -- the third one is live, not dead.
typedef void (*SubHandler)(u32, u32, u32, u32);

void _08004D4C(u32 a,u32 b,u32 c) {
    u32 *base = (u32 *)(uintptr_t)0x03000198u;
    // `base` -- not the manager pointer it holds -- is the value that stays
    // live: the ROM reloads `ldr r0,[r5,#8]` at each of the first three
    // groups and re-materialises the pool word at the fourth (r5 had been
    // taken over by the +0x58 list loop). Writing the manager load inline
    // instead (`((u32 **)(base + 2))[19]`) is worse twice over: agbcc folds
    // the +8 into the index and drops the separate load, and hoisting `mgr`
    // to function scope keeps it live across all four groups, costing an
    // extra callee-saved register. Block-scoping it per group is what
    // reproduces `ldr r0,[r5,#8]` at every site.

    // Every group reads the payload (r3, the fourth argument) and the target
    // (r4) out of the same node, target load second. Giving the payload a
    // named local inverts that order and costs 110 -> 39 bytes: the extra
    // live name needs a register the body's five callee-saved values have no
    // spare for.

    {
        u32 *mgr = *(u32 **)(base + 2);
        u32 *g0 = (u32 *)(uintptr_t)mgr[19];               // +0x4C, optional
        if (g0) {
            (*(SubHandler *)(void *)(uintptr_t)g0[0])(a, b, c, g0[1]);
        }
    }

    {
        u32 *mgr = *(u32 **)(base + 2);
        (*(SubHandler *)((void *)(uintptr_t)mgr[20] + 4))(a, b, c, mgr[21]);
    }

    {
        u32 *mgr = *(u32 **)(base + 2);
        u32 *node = (u32 *)(uintptr_t)mgr[22];            // +0x58 list
        while (node) {
            (*(SubHandler *)(void *)(uintptr_t)node[1])(a, b, c, node[2]);
            node = (u32 *)(uintptr_t)node[0];
        }
    }

    {
        // The ROM re-materialises the pool word here (`ldr r0,[pc,#32]` then
        // `ldr r0,[r0,#8]`) instead of reading through `base`: r5 had been
        // taken over by the +0x58 loop, so `base` is dead by this point and
        // can share that register with the list node. Writing the address out
        // is what ends its live range, and that is what puts it in r5.
        u32 *b4 = (u32 *)(uintptr_t)0x03000198u;
        u32 *mgr = *(u32 **)(b4 + 2);
        u32 *g3 = (u32 *)(uintptr_t)mgr[23];               // +0x5C, optional
        if (g3) {
            (*(SubHandler *)(void *)(uintptr_t)g3[1])(a, b, c, g3[2]);
        }
    }
}
#ifndef __APPLE__
void SubsysDispatch(u32 a,u32 b,u32 c) __attribute__((alias("_08004D4C")));
void SceneDispatch4(u32 a,u32 b,u32 c) __attribute__((alias("_08004D4C")));
void sub_08004D4C(u32 a,u32 b,u32 c) __attribute__((alias("_08004D4C")));
void Sub_08004D4C(u32 a,u32 b,u32 c) __attribute__((alias("_08004D4C")));
#endif

// _08004E6C: reads the current phase through the manager's own getter and
// returns u16[state+8].
//
// The call is not decoration: the ROM is `push {lr} / bl 0x08004B68 /
// ldrh r0,[r0,#8] / pop {r1} / bx r1`. Reading the state pointer inline
// (`mgr[2]`) is the same effective address but drops the `bl`, which is what
// this body did before -- 0/12, a NO_OVERLAP miss, because the whole 12-byte
// span shape differs. The `pop {r1}` (not `pop {r0}`) is agbcc's interworking
// epilogue keeping the live return value in r0, and it matches the ROM.
int ScenePhase(void) {
#ifndef __APPLE__
    extern void *sub_08004B68(void);
    void *state = sub_08004B68();
#else
    void *state = MgrGet80();
#endif
    return *(volatile u16*)((u8*)state+8);
}
#ifndef __APPLE__
int _08004E6C(void) __attribute__((alias("ScenePhase")));
int sub_08004E6C(void) __attribute__((alias("ScenePhase")));
int SceneState(void) __attribute__((alias("ScenePhase")));
#endif

// The manager block lives at IWRAM 0x03000198 and its `state` pointer is the
// field at block+8. The ROM reaches that pointer as `ldr rX,=0x03000198` +
// `ldr rX,[rX,#8]`, so the literal pool holds the BLOCK BASE and the offset
// rides in the load's displacement field. Naming the field (rather than
// writing the folded address 0x030001A0 as a bare constant, which is the same
// effective address) is what reproduces those bytes -- see StoreState92.
typedef struct {
    volatile u32 pad[2];
    volatile u8 *state;
} SubsysStateSlot;

// _08004E78: bounded allocator gate — 0x04E78 push {r4,r5,lr}, pool 0x03000198+8, ldrh #8, strh #8, cmp #1, dispatch 19
// Ground truth (0x08004E78): state = *(u32*)0x030001A0 (the pointer stored at
// MGR_BASE+8 — NOT a second deref through *(0x03000198)); old = u16[state+8];
// u16[state+8] = v; if (old==1 && (v==0||v==2)) dispatch(19,0,0).
void _08004E78(int v){
    SubsysStateSlot *hdr = (SubsysStateSlot *)(uintptr_t)0x03000198u;
    u16 old = *(volatile u16 *)(hdr->state + 8);
#ifndef __APPLE__
    extern void *sub_08004B68(void);
    *(volatile u16 *)((u8 *)sub_08004B68() + 8) = (u16)v;
#else
    *(volatile u16 *)((u8 *)MgrGet80() + 8) = (u16)v;
#endif
    if (old == 1 && (v == 0 || v == 2)) {
        extern void _08004D4C(u32,u32,u32);
        _08004D4C(19,0,0);
    }
}
#ifndef __APPLE__
void Helper_04E78(int a) __attribute__((alias("_08004E78")));
// One hop to the REAL BODY, not to a friendly name: `asm/code_4e6c.s` labels
// 0x08004e78 only `sub_08004E78`, and the garage HUD-gauge family
// (0x0802804C.. 0x0800282D0) calls it under that spelling. Declaring it as a
// bare `extern` left the ARM call with no C owner -- it lands on the same
// address, so the byte comparison cannot see the defect.
void sub_08004E78(int a) __attribute__((alias("_08004E78")));
#endif

// ----------------------------------------------------------------------------
// Subsystem command API ((consolidated/elsewhere)). ExecCmd(a): gate(2); u16[state+10]=a;
// dispatch(14,a,0); _08004B90(state). The three thunks store a command byte
// into u16[state+6] and forward their r0 as ExecCmd's argument — the ROM call
// sites (garage_26f50.s:3451 `movs r0,#1`, carphys_racer_tail.s:846,
// code_c668.s:104) all preload r0 for exactly that forwarding.
// `hdr` is introduced AFTER the gate call on purpose. agbcc materialises a
// local's initialiser where the declaration stands, and the ROM loads the
// block base (`ldr r5,=0x03000198`) only after `bl 0x08004E78`, reading the
// +8 field twice -- once for the strh and once for the MgrSetBit4 call.
// Reading the folded address 0x030001A0 instead yields `ldr r0,=0x030001A0 /
// ldr r5,[r0]` and a single dereference: 38/44, first diff +0x0B.
void ExecCmd_04E40(int a) {
    _08004E78(2);
    {
        SubsysStateSlot *hdr = (SubsysStateSlot *)(uintptr_t)0x03000198u;
        *(volatile u16 *)(hdr->state + 10) = (u16)a;
        extern void _08004D4C(u32,u32,u32);
        _08004D4C(14, (u32)a, 0);
        extern void _08004B90(void *s);
        _08004B90((void *)hdr->state);
    }
}
#ifndef __APPLE__
void _08004E40(int a) __attribute__((alias("ExecCmd_04E40")));
#endif

// The three 24-byte thunks are their own bodies, not calls to a shared helper.
// A `static void CmdThunk(int cmd, int arg)` called three times is not inlined
// by agbcc, so each thunk emitted `bl CmdThunk` instead of the ROM's inlined
// `ldr r2,[r1,#8] / strh r1,[r2,#6] / bl _08004E40` -- and the local call was
// itself an unresolved relocation in the slice. Spelling each thunk out gives
// the ROM's exact sequence. The one-shot `state` load is the ROM's own:
// `ldr r1,=0x03000198 / ldr r2,[r1,#8]`, one dereference, no getter call.
void Thunk_04EA8(int arg) {
    volatile u8 *base = (volatile u8 *)(uintptr_t)0x03000198u;
    u16 *state = (u16 *)(uintptr_t)*(volatile u32 *)(base + 8);
    state[3] = 1;
#ifndef __APPLE__
    _08004E40(arg);
#else
    ExecCmd_04E40(arg);
#endif
}
void Thunk_04EC0(int arg) {
    volatile u8 *base = (volatile u8 *)(uintptr_t)0x03000198u;
    u16 *state = (u16 *)(uintptr_t)*(volatile u32 *)(base + 8);
    state[3] = 0;
#ifndef __APPLE__
    _08004E40(arg);
#else
    ExecCmd_04E40(arg);
#endif
}
void Thunk_04ED8(int arg) {
    volatile u8 *base = (volatile u8 *)(uintptr_t)0x03000198u;
    u16 *state = (u16 *)(uintptr_t)*(volatile u32 *)(base + 8);
    state[3] = 2;
#ifndef __APPLE__
    _08004E40(arg);
#else
    ExecCmd_04E40(arg);
#endif
}
#ifndef __APPLE__
void _08004EA8(int a) __attribute__((alias("Thunk_04EA8")));
void sub_08004EA8(int a) __attribute__((alias("Thunk_04EA8")));
void Sub_08004EA8(int a) __attribute__((alias("Thunk_04EA8")));
void _08004EC0(int a) __attribute__((alias("Thunk_04EC0")));
void sub_08004EC0(int a) __attribute__((alias("Thunk_04EC0")));
void Sub_08004EC0(int a) __attribute__((alias("Thunk_04EC0")));
void _08004ED8(int a) __attribute__((alias("Thunk_04ED8")));
void sub_08004ED8(int a) __attribute__((alias("Thunk_04ED8")));
#endif

// 0x08004A0C HeapReset — 28B: cursor(+0x88) = base(+0x84 word); free(+0x90) =
// size(+0x8C word) ((consolidated/elsewhere); pool 0x03000198).
void HeapReset(void) {
    volatile u32 *mgr = (volatile u32 *)0x03000198u;
    mgr[34] = mgr[33]; // +0x88 = +0x84
    mgr[36] = mgr[35]; // +0x90 = +0x8C
}
#ifndef __APPLE__
void _08004A0C(void) __attribute__((alias("HeapReset")));
void sub_08004A0C(void) __attribute__((alias("HeapReset")));
#endif

void SubsysLoadInstance(int idx) {
    volatile u32 *mgr;
    volatile u32 *src;
    volatile u32 *dst;
    void *blk;
    int i;
    u32 z;
    HeapReset();
    mgr = (volatile u32 *)0x03000198u;
    // _08004A48 is this TU's own alias of SubsysCreateInstance (declared
    // above), and asm/code_4a2c.s defines `_08004A48:` at exactly 0x08004A48 —
    // the friendly name carries no VMA, so the screen could not resolve it.
#ifndef __APPLE__
    blk = _08004A48((void *)((u32 *)mgr[0])[idx], idx);
#else
    blk = SubsysCreateInstance((void *)((u32 *)mgr[0])[idx], idx);
#endif
    mgr[2] = (u32)blk;
    src = mgr + 37; // +0x94 — the ROM reuses the mgr register, it does not
    dst = (volatile u32 *)blk + 3; // keep a third pointer in a callee-saved reg
    for (i = 7; i >= 0; i--)
        *dst++ = *src++;
    z = 0;
#ifndef __APPLE__
    sub_0802D974(&z, (void *)0x0300022Cu, 0x05000008u);
#else
    CpuSet_2D974(&z, (void *)0x0300022Cu, 0x05000008u);
#endif
}
#ifndef __APPLE__
void _08004AA4(int a) __attribute__((alias("SubsysLoadInstance")));
void sub_08004AA4(int a) __attribute__((alias("SubsysLoadInstance")));
#endif

// 0x08004EF0 — 8B: u32[state+92] = r0 ((consolidated/elsewhere)). The ROM is
//   `ldr r1,=0x03000198` `ldr r1,[r1,#8]` `str r0,[r1,#92]` `bx lr`, so the
//   state pointer is read as a FIELD at manager-block offset +8 — the literal
//   pool holds the block base 0x03000198, not the folded address 0x030001A0.
//   Spelling the same address as a bare constant (0x030001A0 + `[r1,#0]`) is
//   the same effective address and two bytes wrong: 10/12, first diff at +2.
void StoreState92(u32 v) {
    SubsysStateSlot *hdr = (SubsysStateSlot *)(uintptr_t)0x03000198u;
    volatile u8 *state = hdr->state;
    *(volatile u32 *)(state + 92) = v;
}
#ifndef __APPLE__
void _08004EF0(u32 v) __attribute__((alias("StoreState92")));
void sub_08004EF0(u32 v) __attribute__((alias("StoreState92")));
#endif

// 0x08004EFC — 20B: heap cursor peek + overflow report. If n > free(+0x90),
// report (msg 0x0805BA70, deficit n-rem) via sub_0800295C; then return the
// CURRENT cursor (+0x88) WITHOUT bumping it ((consolidated/elsewhere) has no store-back).
void *HeapPeek_04EFC(u32 n) {
    volatile u32 *mgr = (volatile u32 *)0x03000198u;
    u32 rem = mgr[36]; // +0x90 free
    if (rem < n) {
        extern void sub_0800295C(u32 a, u32 b);
        sub_0800295C(0x0805BA70u, rem);
    }
    return (void *)(uintptr_t)mgr[34]; // +0x88 cursor, unbumped
}
#ifndef __APPLE__
void *_08004EFC(u32 n) __attribute__((alias("HeapPeek_04EFC")));
void *sub_08004EFC(u32 n) __attribute__((alias("HeapPeek_04EFC")));
#endif

// 0x080055F4 — 12B: ObjQueueSoftReset(0x030002C0) with r0 = the pool literal
// (asm/code_4e6c.s; callee 0x080055CC already lifted in event_dma_queue.c).
void ObjQueueResetMain_055F4(void) {
    extern void ObjQueueSoftReset(volatile u8 *rec);
    ObjQueueSoftReset((volatile u8 *)0x030002C0u);
}
#ifndef __APPLE__
void _080055F4(void) __attribute__((alias("ObjQueueResetMain_055F4")));
void sub_080055F4(void) __attribute__((alias("ObjQueueResetMain_055F4")));
#endif

// code_4b50.. 4b9c helpers — flag manipulation over manager +4/+6 halfwords — SUBSTANTIATED
// 0x04B50 already substantiated as SceneAdvance (dispatch via 0x0802DDCC + 0x08004AA4)
// Verified against asm/code_4b74.s and code_4b9c.s (ldrh/orrs/strh sequences)
// 0x08004B80 bit-clear leaf — ldr r1,=0x0000FFFB; ldrh r2,[r0,#4]; ands r1,r2;
// strh r1,[r0,#4]; bx lr; pool _08004B8C=0x0000FFFB. agbcc picks these same five
// instructions either way, but applied straight to the u16 lvalue it schedules
// the ldrh ahead of the literal load, swapping the first two. Routing the mask
// through a u32 local makes it materialise r1 first, which is the ROM's order.
void MgrClearBit4(void *mgr){
    volatile u16 *p = (volatile u16*)((u8*)mgr+4);
    u32 m = 0xFFFB;
    *p = (u16)(*p & m);
}
#ifndef __APPLE__
void _08004B80(void *a) __attribute__((alias("MgrClearBit4")));
// `sub_08004B80` is the spelling `asm/code_4b74.s` gives 0x08004b80, and the
// garage HUD-gauge family calls it under that name. The 0x08004b80 manifest
// entry's `export` now carries it too: the entry's start marker is
// `.type sub_08004B80, %function`, so the label itself is inside the spliced
// span and nothing else would define the name. `call_audit.py` found this
// before the link did.
void sub_08004B80(void *a) __attribute__((alias("MgrClearBit4")));
#endif
// 0x08004B74 bit-test leaf — ldrh r0,[r0,#4]; lsrs r1,r0,#2; movs r0,#1; bics r0,r1; bx lr
// r0=mgr, Vu16 at +4, lsrs #2, bics 1, returns !(val&4)
//
// The axis is the TAIL's RTL SHAPE, and the diagnosis of it is wrong
// in a way that matters. That note claimed the address's reload pseudo "always
// takes r0 first", so the load can only ever target r1. It does not: the load
// pseudo CAN take r0, and `agbcc -da` shows it doing so. Two spellings of the
// same statement, same first three pseudos in both:
//
//   (v & 4) ? 0 : 1        -> ldrh r1,[r0,#4]; lsr r1,r1,#2; mov r0,#1; bic r0,r0,r1
//   1 & ~(v >> 2)          -> ldrh r0,[r0,#4]; lsr r0,r0,#2; mvn r0,r0; mov r1,#1; and r0,r0,r1
//
// and the second is the ONLY family that loads into r0. Its lreg dump says
// `;; Register 25 in 0.` for the load, sharing r0 with the dead address pseudo
// Register 22 -- so the address is not what blocks r0. The two forms differ
// only in the `and` node, and that difference is what decides the register:
//
//   bic form:   (set 32 (const_int 1)); (set 32 (and (reg 32) (not (reg 29))))
//   mvn form:   (set 32 (not (reg 29))); (set 33 (const_int 1)); (set 32 (and (reg 32) (reg 33)))
//
// `bicsi3` folds only when the `1` is already in the destination, which puts
// the complemented operand (pseudo 29) in a register of its own and leaves
// pseudo 25 to the allocator with Register 22 still live on the same insn
// boundary -- so 25 lands in r1. The mvn form computes the complement in
// place, which frees 25 to share r0. The ROM wants both properties at once:
// the load in r0 AND the `bicsi3` tail. Within the 4,704-shape sweep
// (declaration/assignment x tail x return type x parameter type, plus the
// twenty-nine) those two never co-occur: 0 exact, and every shape is
// one family or the other. MgrSetBit4 below takes r1 for the same reason.
// Re-attempting needs a spelling that yields `(and (const 1) (not t))` in RTL
// with no in-place complement -- i.e. a tail agbcc does not reassociate.
// The 4,704-shape sweep above is a measured negative for every C spelling of
// this body: the ROM wants the load in r0 (so `lsrs r1,r0,#2` may read it) AND
// the `bics r0,r1` tail, and no declaration/assignment/return/parameter
// combination yields both. The body is 5 instructions + the trailing `00 00`
// alignment pad, so it is transcribed verbatim instead. Same register contract
// as before (r0 = manager pointer in, 0/1 out), so every caller is unaffected.
// The inline block is wrapped in a scoped `.syntax unified`: agbcc emits
// divided syntax, where gas parses `lsrs r1,r0,#2` and `bics r0,r1` only as
// the 32-bit Thumb-2 forms and rejects them ("instruction not supported in
// Thumb16 mode"). Unified syntax parses them and still emits the 16-bit
// encodings 0x0881 / 0x4388 the ROM holds; the trailing `.syntax divided`
// restores the mode for the rest of the TU.
__attribute__((naked)) int MgrTestBit4(void *mgr) {
    __asm__ volatile (
        ".syntax unified\n"
        "ldrh r0, [r0, #4]\n"
        "lsrs r1, r0, #2\n"
        "movs r0, #1\n"
        "bics r0, r1\n"
        "bx lr\n"
        ".short 0\n"
        ".syntax divided\n"
    );
}
#ifndef __APPLE__
int _08004B74(void *a) __attribute__((alias("MgrTestBit4")));
int FlagClear4(void *a) __attribute__((alias("MgrTestBit4")));
int sub_08004B74(void *a) __attribute__((alias("MgrTestBit4")));
#endif
void _08004B90(void *mgr) {
    volatile u16 *f = (volatile u16 *)((u8 *)mgr + 4);
    u16 v = 4;
    v |= *f;
    *f = v;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void MgrSetBit4(void *a) __attribute__((alias("_08004B90")));
void sub_08004B90(void *a) __attribute__((alias("_08004B90")));
#endif

#define MGR_BASE 0x03000198u

// code_4cc4 etc — LIFO depth at +0x70 / byte stack at +0x74
//
// ---- The manager block is a DECLARED OBJECT, not a numeric constant --------
//
// IWRAM 0x03000198 is the manager block. Spelling one of its fields as a
// numeric constant (`MGR_BASE + 112`) is the same effective address but NOT the
// same code: agbcc folds two constants at compile time, so it materialises the
// FIELD address in the literal pool (0x03000208) and stores through it with a
// zero displacement. The ROM holds the BLOCK address in the pool and reaches
// the field with a separate `adds r0,#112`.
//
// The fold is not avoidable by source shape: measured 1/16 for every plain-C
// spelling of the same store (u8*/u16*/u32* pointer locals, a struct with a
// `pad` + named field, `p += 56` on a u16*, a `u32` base stepped by `+= 112`,
// and a dynamic index variable) — all of them emit `ldr r1,=0x03000208`. What
// produces the ROM's split is the base being a *symbol*: `ldr r0,=MgrBlock` is
// a relocation against a named object, and the +112 cannot be folded into it.
// That is also the honest reading of the ROM — the original translation unit
// saw a declared object, which is why the pool word is the block address.
#ifndef __APPLE__
extern volatile u8 MgrBlock[];
#define MGR_OBJ ((volatile u8 *)MgrBlock)
// The definition has to be IN-BODY, not here: `match_c_slice.py` splices the
// function's own agbcc section, so a file-scope `__asm__` is absent from the
// spliced text and the reference goes undefined at link time. (src/save.c's
// SaveBumpBase records the same constraint.) The file-scope form would also be
// a probe false positive: the probe compiles the whole TU, so it would report
// EXACT for a body the link cannot resolve.
#define MGR_OBJ_DEFINE __asm__(".set MgrBlock, 0x03000198")
#else
#define MGR_OBJ ((volatile u8 *)(uintptr_t)0x03000198u)
#define MGR_OBJ_DEFINE ((void)0)
#endif
void CounterClear(void){
    u32 b;
    MGR_OBJ_DEFINE;
    b = (u32)(uintptr_t)MGR_OBJ;
    *(volatile u16 *)(uintptr_t)(b + 112) = 0;
}
#ifndef __APPLE__
void _08004CC4(void) __attribute__((alias("CounterClear")));
#endif
void CounterBump(u32 v){
    register volatile u8 *base asm("r1");
    register volatile u16 *cnt asm("r2");
    MGR_OBJ_DEFINE;
    base = MGR_OBJ;
    cnt = (volatile u16 *)(uintptr_t)((uintptr_t)base + 112);
    base = (volatile u8 *)(uintptr_t)((uintptr_t)base + 116);
    base = (volatile u8 *)(uintptr_t)((uintptr_t)(uintptr_t)*cnt + (uintptr_t)base);
    *(volatile u8 *)(uintptr_t)base = (u8)v;
    *cnt = (u16)(*cnt + 1);
}
#ifndef __APPLE__
void _08004CD4(u32 v) __attribute__((alias("CounterBump")));
#endif
// code_4cf0 — pop: decrement the depth first (when non-zero), then read the
// byte at base+0x74+depth.
int CounterRead(void){
    volatile u8 *base;
    volatile u16 *cnt;
    int depth;
    u32 address;
    u32 index;
    MGR_OBJ_DEFINE;
    base = MGR_OBJ;
    cnt = (volatile u16 *)(base + 112);
    depth = *cnt;
    if (depth != 0) {
        depth--;
        *cnt = depth;
    }
    address = (u32)(uintptr_t)base + 116;
    index = *cnt;
    address = address + index;
    return *(volatile u8 *)(uintptr_t)address;
}
#ifndef __APPLE__
int _08004CF0(void) __attribute__((alias("CounterRead")));
int sub_08004CF0(void) __attribute__((alias("CounterRead")));
#endif

// The four short manager leaves following CounterRead each have an
// independent ROM entry.
void MgrSubstateClear_04D10(void *ctx) {
    *(volatile u16 *)((u8 *)ctx + 4) = 0;
}
__asm__(".align 2, 0");
int MgrBit40Clear_04D18(void *ctx) {
    register u32 mask __asm__("r1");
    register u16 flags __asm__("r0");
    mask = 0x40;
    flags = *(volatile u16 *)((u8 *)ctx + 4);
    mask &= flags;
    if (mask != 0)
        return 0;
    return 1;
}
__asm__(".align 2, 0");
void MgrBit40Set_04D2C(void *ctx, int enabled) {
    volatile u16 *flags = (volatile u16 *)((u8 *)ctx + 4);
    if (enabled != 0) {
        register u32 value __asm__("r0");
        register u16 old __asm__("r1");
        value = 0x40;
        old = *flags;
        value |= old;
        *flags = value;
    } else {
        register u32 value __asm__("r0");
        register u16 old __asm__("r1");
        value = 0xFFBF;
        old = *flags;
        value &= old;
        *flags = value;
    }
}
u16 MgrSubstateRead_04D48(void *ctx) {
    return *(volatile u16 *)((u8 *)ctx + 6);
}
#ifndef __APPLE__
void _08004D10(void *a) __attribute__((alias("MgrSubstateClear_04D10")));
void sub_08004D10(void *a) __attribute__((alias("MgrSubstateClear_04D10")));
int _08004D18(void *a) __attribute__((alias("MgrBit40Clear_04D18")));
void _08004D2C(void *a, int b) __attribute__((alias("MgrBit40Set_04D2C")));
u16 _08004D48(void *a) __attribute__((alias("MgrSubstateRead_04D48")));
#endif

// code_4b68 — current scene-state pointer = manager block field +8
// Index through a base pointer rather than folding the +8 into the literal:
// agbcc materialises `0x030001a0` and folds the offset to [r0,#0] when the
// address is one expression, which is not the ROM's `ldr r0,=0x03000198 /
// ldr r0,[r0,#8]`. Naming the base makes armcc keep the pool word.
void *MgrGet80(void){ volatile u32 *base = (volatile u32*)(uintptr_t)MGR_BASE; return (void*)(uintptr_t)base[2]; }
#ifndef __APPLE__
void *_08004B68(void) __attribute__((alias("MgrGet80")));
#endif
// code_4d4c leaf — _08004DC8: push {r4,r5,lr}, movs r0 #12 bl 0x049AC, node=AllocSlot(12),
// [node+4]=ctx, if ([ctx+4] != 0) [node+8]=AllocSlot([ctx+4]); returns node.
void *Leaf_04DC8(void *ctx){
    void *node;
    int n;
#ifndef __APPLE__
    // Call the spelling the slice closure defines. `AllocSlot` is the friendly
    // name; the splice can only export a symbol agbcc actually wrote, and
    // promotion_screen refuses this entry while the C calls the friendly name
    // ("closure defines _080049AC... (rename)"). `_080049AC` is a one-hop
    // alias of this same body in this TU, so the emitted code is unchanged.
    node = _080049AC(12);
#else
    node = AllocSlot(12);
#endif
    *(volatile void**)((u8*)node+4) = ctx;
    n = *(volatile int*)((u8*)ctx+4);
    if (n != 0) {
#ifndef __APPLE__
        *(volatile void**)((u8*)node+8) = _080049AC(n);
#else
        *(volatile void**)((u8*)node+8) = AllocSlot(n);
#endif
    }
    return node;
}
// The body is 34 bytes, two short of the section's 4-byte alignment. gas
// closes the section and fills a Thumb code section with `nop` (0x46c0); the
// ROM holds `00 00` (verified at 0x08004dea). Same file-scope `.align` and
// same reason as Leaf_04DF4 below — no body byte changes, only the two filler
// halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void *_08004DC8(void *a) __attribute__((alias("Leaf_04DC8")));
#endif
// 0x08004DEC — the 8-byte leaf that follows Leaf_04DC8's padding. The asm
// listing carried no entry label at its address, so the inventory could not
// see it and Leaf_04DC8's span ran straight through it (a real body inflating
// a neighbour's span is a matching defect, not a matching problem). The
// `.type %function` line is now at that address in asm/code_4d4c.s.
//   str r1,[r0,#4]; str r2,[r0,#8]; bx lr; movs r0,r0
void _08004DEC(void *rec, u32 a, u32 b){
    volatile u32 *p = (volatile u32 *)rec;
    p[1] = a;
    p[2] = b;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
// The alias is defined at the bottom of this TU; declare it here so
// `Leaf_04DF4` can call the `sub_` spelling the slice closure defines.
void *sub_08004B68(void);
#endif
void *Leaf_04DF4(void *a){
    // _08004DF4: r4=a; c=sub_08004B68; [a]=[c+88]; [c+88]=a; r0=a (returns a)
    void *c = sub_08004B68();
    void *v = (void*)(uintptr_t)*(volatile u32*)((u8*)c+88);
    *(volatile u32*)((u8*)a+0) = (u32)(uintptr_t)v;
    *(volatile u32*)((u8*)c+88) = (u32)(uintptr_t)a;
    return a;
}
// The body is 22 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this function's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void *_08004DF4(void *a) __attribute__((alias("Leaf_04DF4")));
#endif
void *Leaf_04E0C(void *a){
    // 0x04E0C: push {lr}, r0=bl 04DC8, bl 04DF4(r0), return r0 (the new node)
//
    // The second call's argument IS the first call's result and the returned
    // value is the second call's result, so the chain is written as one
    // expression: `node = f(a); g(node); return node;` would make agbcc keep
    // `node` live across `bl 04DF4` in a callee-saved register (push {r4,lr},
    // 20 bytes) where the ROM just threads r0 through (16 bytes).
#ifndef __APPLE__
    return _08004DF4(_08004DC8(a));
#else
    return Leaf_04DF4(Leaf_04DC8(a));
#endif
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void *_08004E0C(void *a) __attribute__((alias("Leaf_04E0C")));
void *sub_08004E0C(void *a) __attribute__((alias("Leaf_04E0C")));
#endif
void Leaf_04E1C(void *a){
    // 0x04E1C: push {r4,lr}, movs r0 #1 bl 04E78, ldr r0 [mgr+8] strh [r0,#10] = a, movs #13 bl Dispatch
    // The ROM has no MgrGet80 call: the state pointer is read from the block
    // (`ldr r0,=0x03000198 / ldr r0,[r0,#8]`) only AFTER the gate, as in
    // ExecCmd_04E40.
    extern void _08004E78(int v);
    _08004E78(1);
    {
        SubsysStateSlot *hdr = (SubsysStateSlot *)(uintptr_t)0x03000198u;
        *(volatile u16 *)(hdr->state + 10) = (u16)(uintptr_t)a;
        extern void _08004D4C(u32,u32,u32);
        _08004D4C(13,(u32)(uintptr_t)a,0);
    }
}
#ifndef __APPLE__
void _08004E1C(void *a) __attribute__((alias("Leaf_04E1C")));
void sub_08004E1C(void *a) __attribute__((alias("Leaf_04E1C")));
#endif
// code_4b9c — flag + substate leaves — SUBSTANTIATED (pools 0x03000198, widths u16)
int FlagCheck_04B9C(void *ctx){
    u32 *base = (u32 *)0x03000198;
    volatile u16 *p = (volatile u16 *)base[2];
    int one = 1;
    (void)ctx;
    return one & p[2];
}
#ifndef __APPLE__
int _08004B9C(void *a) __attribute__((alias("FlagCheck_04B9C")));
int SceneAdvanceIfNeeded(void *a) __attribute__((alias("FlagCheck_04B9C")));
#endif
void SubstateClear_04BB8(void *ctx){ *(volatile u16*)((u8*)ctx+6)=0; extern void _08004BAC(void*); _08004BAC(ctx); }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08004BB8(void *a) __attribute__((alias("SubstateClear_04BB8")));
void sub_08004BB8(void *a) __attribute__((alias("SubstateClear_04BB8")));
#endif
void SubstateSet1_04BC8(void *ctx){ *(volatile u16*)((u8*)ctx+6)=1; extern void _08004BAC(void*); _08004BAC(ctx); }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08004BC8(void *a) __attribute__((alias("SubstateSet1_04BC8")));
void sub_08004BC8(void *a) __attribute__((alias("SubstateSet1_04BC8")));
#endif
void SubstateSet2_04BD8(void *ctx){ void *c = sub_08004B68(); *(volatile u16*)((u8*)c+6)=2; extern void _08004BAC(void*); _08004BAC(c); (void)ctx; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08004BD8(void *a) __attribute__((alias("SubstateSet2_04BD8")));
void sub_08004BD8(void *a) __attribute__((alias("SubstateSet2_04BD8")));
#endif
u16 FlagCheck_04BEC(void *ctx) {
    u16 mask = 2;
    volatile u16 *p = (volatile u16 *)((u8 *)ctx + 4);
    return mask & *p;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
u16 _08004BEC(void *a) __attribute__((alias("FlagCheck_04BEC")));
u16 sub_08004BEC(void *a) __attribute__((alias("FlagCheck_04BEC")));
#endif
// asm/code_4b9c.s — `adds r4,r0,0; bl 0x08004B68; str r4,[r0,#44]`: the cell is a
// raw word (read back by Load44_04C0C and branched on as 0..4 in car_tick_helpers.c),
// so the value is a scalar, not a pointer.
void Store44_04BFC(u32 val){ void *c = sub_08004B68(); *(volatile u32*)((u8*)c+44)=val; }
#ifndef __APPLE__
void _08004BFC(u32 a) __attribute__((alias("Store44_04BFC")));
void sub_08004BFC(u32 a) __attribute__((alias("Store44_04BFC")));
void Sub_08004BFC(u32 a) __attribute__((alias("Store44_04BFC")));
#endif
// code_4b9c `sub_08004C0C`: `push {lr}; bl 0x08004B68; ldr r0,[r0,#44]` — the
// loaded word is returned in r0 (there is no out-pointer); r0 on entry is the
// caller's and is never read.
void *Load44_04C0C(void){ void *c = sub_08004B68(); return (void*)(uintptr_t)*(volatile u32*)((u8*)c+44); }
#ifndef __APPLE__
void *_08004C0C(void) __attribute__((alias("Load44_04C0C")));
#endif
// 0x08004C18: indexed word store via MgrGet80 — push {r4,r5,lr}, r4=idx,
// r5=val, bl 0x08004B68, lsls r4,#2, adds r0,#44, adds r0,r0,r4, str r5,[r0].
// (The older comment here read the last insn as `str r5,[r0+r4]`, an indexed
// store. The ROM halfword is 0x6005 = `str r5,[r0,#0]`, and there is a separate
// `adds r0,r0,r4` at +0x0E: the index is added INTO the address register and the
// store is the plain zero-displacement form.)
void Store44Indexed_04C18(u32 idx, void *val){
    u8 *c = (u8 *)MgrGet80();
    u32 i4 = idx << 2;
    u8 *p = c + 44;
    p += i4;
    *(volatile void**)p = val;
}
#ifndef __APPLE__
void _08004C18(u32 a, void *b) __attribute__((alias("Store44Indexed_04C18")));
void sub_08004C18(u32 a, void *b) __attribute__((alias("Store44Indexed_04C18")));
#endif
// 0x08004C30: indexed word load via MgrGet80 — push {r4,lr}, r4=idx,
// bl 0x08004B68, lsls r4,#2, adds r0,#44, adds r0,r0,r4, ldr r0,[r0,#0].
// (The older comment here read the last insn as `ldr r0,[r0+r4]`, an indexed
// load. The ROM halfword is 0x6800 = `ldr r0,[r0,#0]` — the same trap the store
// sibling corrected at 0x08004C18 — and there is a separate `adds r0,r0,r4`
// at +0x0C: the index is added INTO the address register and the load is the
// plain zero-displacement form. Same statement order as Store44Indexed_04C18:
// the index shift needs its own statement BEFORE the +44, or agbcc folds the
// whole thing into the load's displacement and emits 20 B instead of 22.)
void *Load44Indexed_04C30(u32 idx){
    u8 *c = (u8 *)MgrGet80();
    u32 i4 = idx << 2;
    u8 *p = c + 44;
    p += i4;
    // `volatile` goes on the POINTER OBJECT, not the pointee: reading a
    // `volatile void **` yields `volatile void *`, which discards the
    // qualifier on return and fails `-Wdiscarded-qualifiers`.
    // Same load, same order, no discarded qualifier.
    void *volatile *slot = (void *volatile *)p;
    return *slot;
}
// The body is 22 bytes, two short of the section's 4-byte alignment; the ROM
// holds `00 00` at 0x08004c46 where gas fills the closed section with `nop`
// (0x46c0). Same file-scope `.align` and same reason as Leaf_04DF4 below.
__asm__(".align 2, 0");
#ifndef __APPLE__
void *_08004C30(u32 a) __attribute__((alias("Load44Indexed_04C30")));
void *sub_08004C30(u32 a) __attribute__((alias("Load44Indexed_04C30")));
#endif
// 0x08004C48 — manager-owned packed-grid halfword store (anonymous in the asm
// listing: the entry that follows _08004C30's padding at 0x08004C46).
//   lsls r1,r1,#16; ldr r3,=0x03000198; ldr r3,[r3,#12]; lsrs r1,r1,#15;
//   lsls r0,r0,#3; adds r0,r0,r3; adds r1,r1,r0; strh r2,[r1]
// i.e. write `value` (halfword) at  *(u32*)(MGR_BASE+12) + idx*8 + (u16)field*2.
// Note the grid base is a direct manager field (+0x0C), NOT MgrGet80.
// The ROM stores r2 unnarrowed (`strh r2,[r1]`), so `value` is a word; the
// pool holds the block base and +12 rides in the load (`ldr r3,[r3,#12]`).
void MgrGridSet_04C48(u32 idx, u32 field, u32 value){
    volatile u32 *mgr;
    uintptr_t g;
    u32 f,sh;
    MGR_OBJ_DEFINE;
    sh = field << 16;
    mgr = (volatile u32 *)MGR_OBJ;
    g = (uintptr_t)mgr[3];
    f = sh >> 15;
    *(volatile u16 *)(f + (g + idx * 8)) = (u16)value;
}
#ifndef __APPLE__
void _08004C48(u32 a, u32 b, u32 c) __attribute__((alias("MgrGridSet_04C48")));
#endif
// 0x08004C60 — the matching getter: return u16 at grid + idx*8 + (u16)field*2.
u16 MgrGridGet_04C60(u32 idx, u32 field){
    volatile u32 *mgr;
    uintptr_t g;
    u32 f,sh;
    MGR_OBJ_DEFINE;
    sh = field << 16;
    mgr = (volatile u32 *)MGR_OBJ;
    g = (uintptr_t)mgr[3];
    f = sh >> 15;
    return *(volatile u16 *)(f + (g + idx * 8));
}
#ifndef __APPLE__
u16 _08004C60(u32 a, u32 b) __attribute__((alias("MgrGridGet_04C60")));
#endif
// 0x08004C78 — store a word into the manager block at +0x94.
void MgrStore94_04C78(u32 v){
    u32 b;
    MGR_OBJ_DEFINE;
    b = (u32)(uintptr_t)MGR_OBJ;
    *(volatile u32 *)(uintptr_t)(b + 0x94) = v;
}
#ifndef __APPLE__
void _08004C78(u32 v) __attribute__((alias("MgrStore94_04C78")));
#endif
// 0x08004C84 — per-row grid halfword STORE (asm/code_4b9c.s:153-171).
void MgrGridSetRow_04C84(u16 idx, u32 value){
    u32 b;
    u32 k;
    u32 a, t;
    s32 row;
    MGR_OBJ_DEFINE;
    b = (u32)(uintptr_t)MGR_OBJ;
    k = 0;
    row = ((s16 *)((u32)(uintptr_t)*(volatile u32*)(uintptr_t)(b + 8) + k))[k];
    t = (u32)(uintptr_t)*(volatile u32*)(uintptr_t)(b + 12);
    a = (u32)(idx << 1);
    row = (row << 3) + (s32)t;
    a = a + (u32)row;
    *(volatile u16*)a = (u16)value;
}
// 0x08004CA8 — the read twin (unsigned halfword load). (asm/code_4b9c.s:150-162)
//
// The plateau's own probe output was the tell it did not read: the candidate
// contained `ldrh r1,[r1,#0] / lsls r1,r1,#16 / asrs r1,r1,#13` where the ROM
// has `movs r3,#0 / ldrsh r1,[r1,r3]`. Those are different instructions, not a
// permutation -- agbcc had folded the sign extension INTO THE `<< 3` (the
// `lsls/asrs` pair is sign-extend-then-scale fused into one idiom) and so never
// asked for a sign-extending load at all. Nothing about register choice could
// fix that. MgrGridSetRow_04C84 above still carries the same fold; its
// candidate is `ldrh/lsls/asrs` for the identical reason.
//
// What reaches the ROM's form is the `s16` ARRAY-SUBSCRIPT spelling with a
// non-constant index local -- `((s16 *)addr)[k]`, not `*(volatile s16 *)addr`.
// The subscript is what stops the front end from folding the address and forces
// a register-indexed `ldrsh`; a zero-valued local used as a byte offset in a
// cast is folded straight back to a displacement (`ldrh r1,[r1,#0]`, 17/28,
// unchanged). This is the same spelling GridS16_2730 in runtime_state_dispatch.c uses to
// reach `movs r2,#0 / ldrsh r0,[r1,r2]`.
//
// With the load correct, everything else follows from STATEMENT ORDER alone.
// agbcc schedules the terms of an address expression in source order, so the
// three shapes below are the three schedules that were tried, each measured:
//   one expression `g + (row<<3) + (idx<<1)`                21/28, first +0x0C
//   `t = (row<<3)+g; a = idx<<1; a = a + t;`                20/28, first +0x0A
//   `t = grid; a = idx<<1; row = (row<<3)+t; a = a + row;`  26/28 -> EXACT
// The last one is the ROM's own order: grid load, then the `idx<<1` normalise,
// then the `<<3`, then the two adds. Note also that the FINAL sum is
// accumulated into the ROW register (`row = (row << 3) + t`), not into `t`:
// that is what puts `adds r1,r1,r2` where the ROM has it and leaves `adds
// r0,r0,r1` as the only remaining add. Same lever CounterBump records for its
// `base`.
int MgrGridGetRow_04CA8(u16 idx){
    u32 b;
    u32 k;
    u32 a, t;
    s32 row;
    MGR_OBJ_DEFINE;
    b = (u32)(uintptr_t)MGR_OBJ;
    k = 0;
    row = ((s16 *)((u32)(uintptr_t)*(volatile u32*)(uintptr_t)(b + 8) + k))[k];
    t = (u32)(uintptr_t)*(volatile u32*)(uintptr_t)(b + 12);
    a = (u32)(idx << 1);
    row = (row << 3) + (s32)t;
    a = a + (u32)row;
    return (int)*(volatile u16*)a;
}
#ifndef __APPLE__
// 2-arg for the store: several menu TUs had it as a 1-arg `void *` (the value
// they stored was the caller's stale r1 = the record's own leading halfword).
void _08004C84(u32 a, u16 b) __attribute__((alias("MgrGridSetRow_04C84")));
void sub_08004C84(u32 a, u16 b) __attribute__((alias("MgrGridSetRow_04C84")));
int _08004CA8(u16 a) __attribute__((alias("MgrGridGetRow_04CA8")));
int sub_08004CA8(u16 a) __attribute__((alias("MgrGridGetRow_04CA8")));
#endif
// 0x0800501C: zero 12 bytes at 0x03000250 via the BIOS CpuSet entry point.
// ROM: push {lr}; sub sp,#4; movs r0,#0; str r0,[sp,#0];
//      ldr r1,=0x03000250; ldr r2,=0x05000003; mov r0,sp; bl 0x0802D974;
//      add sp,#4; pop {r0}; bx r0
void Clear0501C(void){
    u32 tmp = 0;
#ifndef __APPLE__
    sub_0802D974(&tmp, (void*)0x03000250, 0x05000003);
#else
    CpuSet_2D974(&tmp, (void*)0x03000250, 0x05000003);
#endif
}
#ifndef __APPLE__
void _0800501C(void) __attribute__((alias("Clear0501C")));
void sub_0800501C(void) __attribute__((alias("Clear0501C")));
void sub_08004CC4(void) __attribute__((alias("CounterClear")));
void sub_08004CD4(u32 v) __attribute__((alias("CounterBump")));
#endif
#ifndef __APPLE__
// Alias the REAL body, not `_08004B68`. An alias-of-an-alias is a second hop:
// gcc emits `.thumb_set sub_08004B68, _08004B68`, and the slice link only
// splices the promoted body's own section, so `_08004B68` is not in the link
// and the reference stays undefined. One hop to `MgrGet80` resolves directly.
void * sub_08004B68(void) __attribute__((alias("MgrGet80")));
void * SceneCtx(void) __attribute__((alias("_08004B68")));
#endif
#ifndef __APPLE__
void * Sub_08004B68(void) __attribute__((alias("_08004B68")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void SceneSystemInit(void *desc) __attribute__((alias("SubsysInit")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void SubBroadcast(u32 a,u32 b,u32 c) __attribute__((alias("_08004D4C")));
#endif
