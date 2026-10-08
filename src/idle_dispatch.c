#include "gtadv/idle.h"
#include "gtadv/state_block_a.h"
#include "gtadv/memory.h"
#include "gba/regs.h"
#include "gba/bios.h"

// Idle dispatcher + handlers reference: asm/idle.s 0x08001988-0x080020E8
// without duplicating idle_accessors.c (which owns the pure accessors).
// Behavioral equivalence, not byte-matching.

extern void CpuSet_2D974(const void *src, void *dst, unsigned ctrl); // _08002D974 CpuSet (swi 0x0B)
extern u32  DivRemU_02DFE4(u32 num, u32 den); // sub_0802DFE4 remainder (r0)
extern u32  sub_0802DF6C(u32 n, u32 d); // unsigned quotient (r0)
extern void Warn(u32 a, u32 b); // sub_0800295C
extern void _0800295C(u32 a, u32 b);
extern u32  SessionReadWord(u32 off); // _0800070C
extern void SessionPump(void); // _080006A4
extern void PacketBuild(void *a, u32 b); // _08000848
// sub_08000CA0 (block-B special-mode state stepper) now lifted in runtime_state_dispatch.c
// (SessionStep_CA0); idle mode-8/11 path binds the strong C alias.
extern int  SubCA0(u32 a); // sub_08000CA0

static inline volatile u8 *st8(void) { return (volatile u8 *)(uintptr_t)*(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR; }
static inline volatile u16 *st16(int off) { return (volatile u16 *)(st8()+off); }
static inline volatile u32 *st32(int off) { return (volatile u32 *)(st8()+off); }

// Forward decls of helpers owned by other files
u16  StateA_GetRecordHalfwordInverted(void);
// The closure spells these two `_0800…`; call them by that spelling so the
// spliced body resolves without a veneer. Declared here as well as by their
// friendly names above, which other bodies in this file still use.
u16  _08001680(void);
void _080016D0(u32 v);
u16  StateA_GetIndexedHalfwordInverted(int idx);
void *StateA_GetRecordPtrInverted(void);
void StateA_SetPendingParam(u32 v);
bool StateA_IsGameMode(int mode);
bool StateA_IsSpecialMode(u32 mode);

// _08001988 — mode 3 handler
void Idle_Mode3Handler(void) {
    u16 head = StateA_GetRecordHalfwordInverted();
    if (head == 0x03ED) {
        u16 v1 = StateA_GetIndexedHalfwordInverted(0);
        s16 cur = (s16)*st16(0x1A);
        if ((s16)v1 == cur) {
            if (v1 != 0) {
                s16 v16 = (s16)*st16(0x16);
                if ((s16)v1 == v16) {
                    u32 payload = *st32(0x2C);
                    // asm uses r0 of sub_0802DFE4 = REMAINDER (r2 = quotient)
                    unsigned div = DivRemU_02DFE4(payload, 12);
                    if (div == 0) div = 12;
                    *st16(0x12) = 5;
                    void *src = StateA_GetRecordPtrInverted();
                    src = (u8*)src + 4;
                    u32 dstWord = *st32(0x20);
                    int words = (int)div;
                    int adj = (words >> 31);
                    int half = (words + adj) >> 1;
                    CpuSet_2D974(src, (void*)(uintptr_t)dstWord, (unsigned)half);
                    *st32(0x20) = dstWord + (((u32)div >> 1) << 1);
                } else {
                    // div remains 12 path
                }
                // tail copy path already handled above in == case
            } else {
                u16 v0 = StateA_GetIndexedHalfwordInverted(0);
                *st32(0x2C) = v0;
                u16 v2 = StateA_GetIndexedHalfwordInverted(2);
                *st16(0x16) = v2;
                if (v2 == 0) Warn(0x0802E1D0, 0);
            }
        }
        // tick +1 at 0x1A
        u16 t = *st16(0x1A);
        *st16(0x1A) = t + 1;
    }
    StateA_SetPendingParam(0x03FC);
    u16 t = *st16(0x1A);
    Idle_WriteEvent(0, t);
    s16 a = (s16)*st16(0x1A);
    s16 b = (s16)*st16(0x16);
    int v = a;
    if (a > b) v = b;
    // signed divide helper sub_0802DE04 — clamp negative to 0
    extern int DivSI(int n, int d);
    int res = DivSI(v,1);
    *st32(0xD0) = res < 0 ? 0 : (u32)res;
}
#ifndef __APPLE__
void _08001988(void) __attribute__((alias("Idle_Mode3Handler")));
#endif

// _08001A74 — mode 4 countdown
void Idle_Mode4Handler(void) {
    u8 *base;
    register u16 uv __asm__("r2");
    s16 sv;
    _080016D0(0x03EE);
    base = (u8 *)*(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    uv = *(volatile u16 *)(base + 0x14);
    sv = *(s16 *)(base + 0x14);
    if (sv > 0) {
        register u16 tmp __asm__("r0");
        tmp = uv - 1;
        *(volatile u16 *)(base + 0x14) = tmp;
    } else {
        *(volatile u16 *)(base + 0x12) = 1;
    }
}
#ifndef __APPLE__
void _08001A74(void) __attribute__((alias("Idle_Mode4Handler")));
#endif

// _08001AA0 — mode5
void Idle_Mode5Handler(void) {
    if (_08001680() == 0x03EE) *st16(0x12)=1;
    _080016D0(0x03F1);
}
#ifndef __APPLE__
void _08001AA0(void) __attribute__((alias("Idle_Mode5Handler")));
#endif

// _08001ACC — mode6
void Idle_Mode6Handler(void) {
    u16 cnt = *st16(0x0A);
    int r5=1, r4=1;
    if (1 < (int)cnt) {
        for (r4=1; r4 < (int)cnt; r4++) {
            if (StateA_GetRecordHalfword(r4) != 0x03F4) r5++;
        }
    }
    if (r5 == (int)cnt) { *st16(0x12)=7; *st16(0x14)=3; }
    StateA_SetPendingParam(0x03F2);
}
#ifndef __APPLE__
void _08001ACC(void) __attribute__((alias("Idle_Mode6Handler")));
#endif

// _08001B20 — mode7
void Idle_Mode7Handler(void) {
    u8 *base;
    register u16 uv __asm__("r2");
    s16 sv;
    base = (u8 *)*(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    uv = *(volatile u16 *)(base + 0x14);
    sv = *(s16 *)(base + 0x14);
    if (sv > 0) {
        register u16 tmp __asm__("r0");
        tmp = uv - 1;
        *(volatile u16 *)(base + 0x14) = tmp;
    } else {
        *(volatile u16 *)(base + 0x12) = 8;
    }
#ifndef __APPLE__
    _080016D0(0x03F3);
#else
    StateA_SetPendingParam(0x03F3);
#endif
}
#ifndef __APPLE__
void _08001B20(void) __attribute__((alias("Idle_Mode7Handler")));
#endif

// _08001B50 — mode10
void Idle_Mode10Handler(void) {
    if (StateA_GetRecordHalfwordInverted()==0x03F3) *st16(0x12)=11;
#ifndef __APPLE__
    _080016D0(0x03F4);
#else
    StateA_SetPendingParam(0x03F4);
#endif
}
#ifndef __APPLE__
void _08001B50(void) __attribute__((alias("Idle_Mode10Handler")));
#endif

// _08001B7C — per-frame mode dispatcher
void Idle_Dispatcher(void) {
    u16 cur = *st16(0x10);
    if (StateA_IsSpecialMode(cur)) {
        u32 d0 = *st32(0xD0);
        if (SubCA0(d0) != 0) *st16(0x12)=1;
    }
    // refresh work areas at +0x30 — asm uses sub_0802D974 = CpuSet (swi 0x0B),
    // NOT CpuFastSet: for ctrl 0x10 (16 words, a multiple of 8) the two SWIs
    // agree, but keep the exact asm op for fidelity.
    {
        volatile u8 *st = st8();
        CpuSet((const void*)(st+0x70), (void*)(st+0x30), 0x04000010);
        u32 zero=0;
        CpuSet(&zero, (void*)(st+0x30), 0x05000010);
    }
    u16 req = *st16(0x10);
    int idx = (int)req -1;
    if (idx <0 || idx >=10) return;
    switch(idx) {
        case 0: StateA_SetPendingParam(0x03EB); break;
        case 1: StateA_UpdateClockAndRecords(); break;
        case 2: Idle_Mode3Handler(); break;
        case 3: Idle_Mode4Handler(); break;
        case 4: Idle_Mode5Handler(); break;
        case 5: Idle_Mode6Handler(); break;
        case 6: Idle_Mode7Handler(); break;
        case 7: break;
        case 8: break;
        case 9: Idle_Mode10Handler(); break;
    }
}
#ifndef __APPLE__
void _08001B7C(void) __attribute__((alias("Idle_Dispatcher")));
#endif

// _08001C48 — idle work-area refresh
void Idle_RefreshWorkArea(void) {
    volatile u8 *st = st8();
    // asm _08001C48: r0=st+0xC0 (src), r1=st+0xB0 (dst) — C0->B0, not B0->C0.
    CpuSet((const void*)(st+0xC0), (void*)(st+0xB0), 0x04000004);
    s16 cnt = *(volatile s16*)(st+8);
    if (cnt==0) {
        u32 z=0; CpuSet(&z, (void*)(st+0xC0), 0x05000004);
    } else {
        *(volatile s16*)(st+8) = cnt-1;
    }
    u16 cur = *st16(0x10);
    if (StateA_IsGameMode(cur)) {
        u16 p = *st16(4);
        *(volatile u16*)(st+0xC0) = p;
        PacketBuild((void*)(st+0xB0), 0);
    }
}
#ifndef __APPLE__
void _08001C48(void) __attribute__((alias("Idle_RefreshWorkArea")));
#endif

// _08001CD8 — input/ticker
void Idle_Ticker(void) {
    volatile u8 *st = st8();
    *st16(0x0C)=0;
    u16 cur = *st16(0x10);
    if (!StateA_IsGameMode(cur)) return;
    u32 w = SessionReadWord((u32)(uintptr_t)(st+0x30));
    *st32(0xD8)= w;
    SessionPump();
    u8 b2 = *(volatile u8*)(st+2);
    if (b2 !=0) {
        u32 d8 = *st32(0xD8);
        bool bump=false;
        if ((d8 & 0xC000)==0) {
            if (d8 & 0x8000) {
                u32 low = d8 & 0xF;
                u32 high = (d8>>12)&0xF;
                if (low != high) bump=true;
            }
        } else bump=true;
        if (bump) {
            u16 e = *st16(0x0E);
            e++;
            if (e > 60) { e=0; extern void SubBroadcast(u32 a,u32 b,u32 c); SubBroadcast(w,21,0); } // asm evidence: sub_08004D4C(0x030000E4-block, 21, 0)
            *st16(0x0E)=e;
        } else {
            // store 0/1? original stores r3 (0 or 1) into +0x0E when not bumping
        }
        // extra pump gates on bit15 and latch
        u32 d = *st32(0xD8);
        if (d & 0x80) {
            if (*(volatile u8*)(st+1)==0) SessionPump();
            // second gate
            d = *st32(0xD8);
            if (d & 0x100) *(volatile u8*)(st+1)=1;
        }
        // bits 0,1,2,3 -> +0x0C increments
        d = *st32(0xD8);
        if (d & 1)  (*st16(0x0C))++;
        d = *st32(0xD8);
        if (d & 2)  (*st16(0x0C))++;
        d = *st32(0xD8);
        if (d & 4)  (*st16(0x0C))++;
        d = *st32(0xD8);
        if (d & 8)  (*st16(0x0C))++;
    }
}
#ifndef __APPLE__
void _08001CD8(void) __attribute__((alias("Idle_Ticker")));
#endif

// Remaining small thunks/accessors not in idle_accessors.c
// _08001E14 — 28 B with TWO pool words. asm/idle.s:654-669 is
//   ldr r2,=0x0802E190 / lsls r0,#2 / ldr r1,=0x030000E4 / ldr r1,[r1] /
//   ldrh r1,[r1,#6] / lsls r1,#4 / adds r0,r0,r1 / adds r0,r0,r2 /
//   ldr r0,[r0] / bx lr.
// agbcc materialises each initialiser where its declaration stands, so both
// intermediates are NAMED locals declared in the ROM's order: the table
// pointer first, then the index shift, then the halfword load.
u32 Idle_GetDescriptorWord(int idx) {
#ifndef __APPLE__
    extern u8 RecDescTbl[] __asm__("RecDescTbl");
    __asm__(".globl RecDescTbl\nRecDescTbl = 0x0802E190\n");
    u32 tbl = (u32)(uintptr_t)RecDescTbl;
#else
    u32 tbl = 0x0802E190u;
#endif
    u32 off = (u32)idx << 2;
    u16 sel = *st16(6);
    u32 sum = off + ((u32)sel << 4);
    return *(u32 *)((uintptr_t)sum + (uintptr_t)tbl);
}
#ifndef __APPLE__
u32 _08001E14(int a) __attribute__((alias("Idle_GetDescriptorWord")));
#endif
// _08001E30 -> int — 1 when block A +6 == 0 (normal select active).
// Word-typed return, and the `int r = 0` must be a NAMED local declared BEFORE
// the load: agbcc materialises each initialiser where its declaration stands,
// so an unnamed `return x == 0;` sinks `movs r1,#0` behind the two pool loads
// (candidate `ldr r0,[pc,#20]; ldr r0,[r0]; movs r1,#0; ldrh...`). With the
// local first the ROM's own order falls out: `movs r1,#0` at 0x08001E30, the
// pool load at 0x08001E32.
//
// NOT byte-exact, and the two missing halfwords are not a C-shape problem.
// asm/idle.s:673-686 is 24 B with no frame (`adds r0,r1,#0; bx lr` at 0x1e3e);
// this body compiles to the identical ten instructions plus `push {lr}` at
// 0x1e30 and `pop {r1}; bx r1` instead of the bare `bx lr` — 28 B, and
// `push {lr}`/`pop {r1}` is an aligned 2-byte insert so even the 10 real
// instructions sit at shifted addresses. The frame comes from agbcc's Thumb
// prologue policy, not from the C: `thumb_function_prologue` pushes lr when
// `live_regs_mask || !leaf_function_p || far_jump_used_p`
// (gcc_arm/config/arm/thumb.c:1134) and `output_return` falls to
// `thumb_exit(-1)` → `pop {r1}; bx r1` (thumb.c:1025-1034). Measured across
// ~60 leaf shapes (int/u32/long/short/char/unsigned/pointer returns; if/else,
// ?:, switch, do-while, goto, two-armed and three-armed accumulators; named
// and unnamed initialisers; `register`): EVERY one that keeps a conditional
// branch gets `push {lr}` + `pop {r1}; bx r1`, and every one that drops the
// branch is frameless. A branchless rewrite (`neg/orr/lsr #31`, or
// `mov r0,#1`) is frameless but does not reproduce the ROM's
// `movs r1,#0 … movs r1,#1 … adds r0,r1,#0` either, so there is no C that
// yields these 24 bytes. Consistent with the rest of the backlog: the ROM has
// several frameless branchy leaves (asm/carphys_racer_tail.s:983-999 at
// 0x080022438, asm/garage_26f50.s:1720 and :4567) and none is promoted;
// src/race_phase_vms.c:565 records the same wall for 0x08002242C.
// It is not a flag choice either: the same body frames at -O0/-O1/-O2/-O3/-Os
// and with or without -mthumb-interwork (only the epilogue spelling changes:
// `pop {pc}` instead of `pop {r1}; bx r1` when interwork is off, and
// -fno-jump-tables does not help).
// `bool` was also wrong independently: as a u8 return it gives the same
// frame, and the ROM leaves the flag in r0 as a word.
int Idle_IsSelZero(void) {
    int r = 0;
    if (*st16(6) == 0) r = 1;
    return r;
}
#ifndef __APPLE__
int _08001E30(void) __attribute__((alias("Idle_IsSelZero")));
#endif
// _08001EB8(v) — `v-1` into block A +8. The ROM decrements the ARGUMENT
// (`subs r0,#1`, asm/idle.s:774), not a re-read of the cell, so the word-typed
// parameter replaces the old reload-and-decrement; the store is `strh`, so the
// upper bits of the argument are dropped exactly as the ROM drops them.
void Idle_DecScratch08(u32 v) { *st16(8) = v - 1; }
#ifndef __APPLE__
void _08001EB8(u32 v) __attribute__((alias("Idle_DecScratch08")));
#endif
void Idle_SetupMode2(u32 a, u32 b); // _08001EC8 etc. — stub behavioral
#ifndef __APPLE__
void _08001EC8(u32 a,u32 b) __attribute__((alias("Idle_SetupMode2")));
#endif
void Idle_SetupMode2(u32 a, u32 b) {
    u32 *slot = (u32 *)STATE_BLOCK_A_SLOT_ADDR;
    u8 *base = (u8 *)*slot;
    u32 savea = a;
    u32 zero = 0;
    u32 q;
    *(volatile u16 *)(base + 0x12) = 2;
    *(volatile u32 *)(base + 0x2C) = b;
    q = sub_0802DF6C(b + 11, 12);
    *(volatile u16 *)(base + 0x16) = (u16)q;
    if ((q << 16) == 0)
        _0800295C(0x0802E1D0, 0);
    {
        u8 *p = (u8 *)*slot;
        *(volatile u16 *)(p + 0x18) = (u16)zero;
        *(volatile u16 *)(p + 0x1C) = (u16)zero;
        *(volatile u32 *)(p + 0x24) = savea;
    }
}
// _08001F08 — one cached state-block pointer, and the zero materialised
void Idle_SetupMode3(u32 a) {
    volatile u8 *p = st8();
    u16 zero = 0;
    *(volatile u16 *)(p + 0x12) = 3;
    *(volatile u16 *)(p + 0x1A) = zero;
    { u16 c = 0x80 << 5; *(volatile u16 *)(p + 0x16) = c; }
    *(volatile u32 *)(p + 0x20) = a;
}
#ifndef __APPLE__
void _08001F08(u32 a) __attribute__((alias("Idle_SetupMode3")));
#endif
