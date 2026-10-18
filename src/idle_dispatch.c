#include "gtadv/idle.h"
#include "gtadv/state_block_a.h"
#include "gtadv/memory.h"
#include "gba/regs.h"
#include "gba/bios.h"

// Idle dispatcher + handlers reference: asm/idle.s 0x08001988-0x080020E8
// without duplicating idle_accessors.c (which owns the pure accessors).
// Idle_Dispatcher, Idle_Mode3Handler, and Idle_Ticker below are C lifts with
// exact ROM matches.

extern void CpuSet_2D974(const void *src, void *dst, unsigned ctrl); // _08002D974 CpuSet (swi 0x0B)
extern u32  DivRemU_02DFE4(u32 num, u32 den); // sub_0802DFE4 remainder (r0)
extern u32  sub_0802DF6C(u32 n, u32 d); // unsigned quotient (r0)
extern void Warn(u32 a, u32 b); // sub_0800295C
extern void _0800295C(u32 a, u32 b);
extern u32  SessionReadWord(u32 off); // _0800070C
extern void SessionPump(void); // _080006A4
extern void PacketBuild(void *a, u32 b); // _08000848
extern void _08000848(void *a, u32 b);
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

// _08001988 — mode-3 handler, lifted to C with an exact 236-byte match.
void Idle_Mode3Handler(void)
{
    extern u16 _08001698(int idx);
    extern void *_080016B8(void);
    extern void _08001E48(int id, u32 payload);
    extern u32 sub_0802DFE4(u32 a, u32 b);
    extern void sub_0802D974(const void *src, void *dst, u32 ctrl);
    extern s32 sub_0802DE04(s32 a, s32 b);
    register u32 slotOrBlocks __asm__("r5");
    register volatile u8 *record __asm__("r4");
    register s16 *minuteState __asm__("r4");
    register s32 currentMinute __asm__("r0");
    register u32 minute __asm__("r1");
    if (_08001680() == 0x03ED) {
        minute = _08001698(0);
        slotOrBlocks = STATE_BLOCK_A_SLOT_ADDR;
        record = (volatile u8 *)(uintptr_t)*(volatile u32 *)(uintptr_t)slotOrBlocks;
        minuteState = (s16 *)record;
        currentMinute = minuteState[13];
        if (minute == currentMinute) {
            if (minute == 0) {
                *(volatile u32 *)((volatile u8 *)(uintptr_t)
                    *(volatile u32 *)(uintptr_t)slotOrBlocks + 44) = _08001698(1);
                {
                    register u16 result __asm__("r0") = _08001698(2);
                    *(volatile u16 *)((volatile u8 *)(uintptr_t)
                        *(volatile u32 *)(uintptr_t)slotOrBlocks + 22) = result;
                    if (((u32)result << 16) == 0)
                        _0800295C(0x0802E1D0, 0);
                }
            } else {
                minuteState = (s16 *)record;
                currentMinute = minuteState[11];
                if (minute == currentMinute) {
                    slotOrBlocks = sub_0802DFE4(*(volatile u32 *)(record + 44), 12);
                    if (slotOrBlocks == 0)
                        slotOrBlocks = 12;
                    *(volatile u16 *)(record + 18) = 5;
                } else {
                    slotOrBlocks = 12;
                }
                {
                    u32 source = (u32)(uintptr_t)_080016B8() + 4;
                    record = (volatile u8 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
                    {
                        u32 destination = *(volatile u32 *)((volatile u8 *)(uintptr_t)
                            *(volatile u32 *)record + 32);
                        register u32 control __asm__("r2") = slotOrBlocks;
                        control = (slotOrBlocks + (slotOrBlocks >> 31)) << 10;
                        control >>= 11;
                        sub_0802D974((const void *)(uintptr_t)source,
                                     (void *)(uintptr_t)destination, control);
                    }
                    {
                        register volatile u32 *advanceState __asm__("r2") =
                            (volatile u32 *)(uintptr_t)*(volatile u32 *)record;
                        register u32 evenCount __asm__("r1") = slotOrBlocks >> 1;
                        evenCount <<= 1;
                        advanceState[8] += evenCount;
                    }
                }
            }
            {
                register volatile u8 *lateRecord __asm__("r1") =
                    (volatile u8 *)(uintptr_t)*(volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
                *(volatile u16 *)(lateRecord + 26) = *(volatile u16 *)(lateRecord + 26) + 1;
            }
        }
    }
    _080016D0(0x03F0);
    {
        register volatile u8 *state __asm__("r4") = (volatile u8 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
        register volatile u8 *eventState __asm__("r0") =
            (volatile u8 *)(uintptr_t)*(volatile u32 *)state;
        u16 eventMinute = *(volatile u16 *)(eventState + 26);
        _08001E48(0, eventMinute);
        state = (volatile u8 *)(uintptr_t)*(volatile u32 *)state;
        {
            register s32 current __asm__("r0");
            register s32 limit __asm__("r1");
            current = ((s16 *)state)[13];
            limit = ((s16 *)state)[11];
            if (current > limit)
                current = limit;
            state += 208;
            current = (s32)((u32)current << 12);
            {
                s32 result = sub_0802DE04(current, limit);
                *(volatile s32 *)state = result;
                if (result < 0)
                    *(volatile s32 *)state = 0;
            }
        }
    }
}

void _08001988(void) __attribute__((alias("Idle_Mode3Handler")));

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
void _08001A74(void) __attribute__((alias("Idle_Mode4Handler")));

// _08001AA0 — mode5
void Idle_Mode5Handler(void) {
    if (_08001680() == 0x03EE) *st16(0x12)=1;
    _080016D0(0x03F1);
}
void _08001AA0(void) __attribute__((alias("Idle_Mode5Handler")));

// _08001ACC — mode6
void Idle_Mode6Handler(void) {
    register int matches __asm__("r5") = 1;
    register int i __asm__("r4") = 1;
    register volatile u32 *slot __asm__("r0") =
        (volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    register volatile u8 *initial_base __asm__("r1") =
        (volatile u8 *)(uintptr_t)*slot;
    register volatile u32 *saved_slot __asm__("r6") = slot;
    register u16 count __asm__("r1") =
        *(volatile u16 *)(initial_base + 0x0A);

    // The assembly rereads the record count after each record lookup and
    // again before the final comparison; callers can update this shared block.
    if (matches < count) {
        do {
            register u32 record __asm__("r0") = StateA_GetRecordHalfword(i);
            register u32 expected __asm__("r1") = 0xFD;
            record <<= 16;
            record >>= 16;
            expected <<= 2;
            if (record == expected)
                matches++;
            i++;
            u16 current_count = *(volatile u16 *)((volatile u8 *)(uintptr_t)
                *(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR + 0x0A);
            if (i >= current_count)
                break;
        } while (1);
    }

    register volatile u8 *base __asm__("r1") =
        (volatile u8 *)(uintptr_t)*saved_slot;
    u16 final_count = *(volatile u16 *)(base + 0x0A);
    if (matches == final_count) {
        *(volatile u16 *)(base + 0x12) = 7;
        *(volatile u16 *)(base + 0x14) = 3;
    }
    StateA_SetPendingParam(0x03F2);
}
void _08001ACC(void) __attribute__((alias("Idle_Mode6Handler")));

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
    _080016D0(0x03F3);
}
void _08001B20(void) __attribute__((alias("Idle_Mode7Handler")));

// _08001B50 — mode10
void Idle_Mode10Handler(void) {
    if (StateA_GetRecordHalfwordInverted()==0x03F3) *st16(0x12)=11;
    _080016D0(0x03F4);
}
void _08001B50(void) __attribute__((alias("Idle_Mode10Handler")));

// _08001B7C — snapshot/clear work buffers, then dispatch modes 1 through 10.
// Special modes 8 and 11 can first redirect the pending mode to 1.
void Idle_Dispatcher(void)
{
    extern int _08001744(u32 mode);
    extern int sub_08000CA0(u32 state);
    extern void sub_0802D974(const void *src, void *dst, u32 ctrl);
    extern void _08001854(void);
    extern void _08001988(void);
    extern void _08001A74(void);
    extern void _08001AA0(void);
    extern void _08001ACC(void);
    extern void _08001B20(void);
    extern void _08001B50(void);
    u32 zero;
    register volatile u32 *slot __asm__("r4") =
        (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
    if (_08001744(*(volatile u16 *)((volatile u8 *)(uintptr_t)*slot + 16))) {
        register u32 modeState __asm__("r0") = (u32)(uintptr_t)*slot + 208;
        if (sub_08000CA0(modeState)) {
            register volatile u8 *writeState __asm__("r1") =
                (volatile u8 *)(uintptr_t)*slot;
            *(volatile u16 *)(writeState + 18) = 1;
        }
    }
    slot = (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
    {
        register volatile u8 *state __asm__("r1") =
            (volatile u8 *)(uintptr_t)*slot;
        sub_0802D974((const void *)(state + 48), (void *)(state + 112), 0x04000010);
    }
    zero = 0;
    {
        register volatile u8 *state __asm__("r1") =
            (volatile u8 *)(uintptr_t)*slot;
        sub_0802D974(&zero, (void *)(state + 48), 0x05000010);
    }
    switch (*(volatile u16 *)((volatile u8 *)(uintptr_t)*slot + 16)) {
    case 1:
        _080016D0(0x03EB);
        break;
    case 2:
        _08001854();
        break;
    case 3:
        _08001988();
        break;
    case 4:
        _08001A74();
        break;
    case 5:
        _08001AA0();
        break;
    case 6:
        _08001ACC();
        break;
    case 7:
        _08001B20();
        break;
    case 10:
        _08001B50();
        break;
    default:
        break;
    }
}
void _08001B7C(void) __attribute__((alias("Idle_Dispatcher")));

// _08001C48 — idle work-area refresh
void Idle_RefreshWorkArea(void) {
    register volatile u32 *slot __asm__("r4") =
        (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
    volatile u8 *first = (volatile u8 *)(uintptr_t)*slot;
    // asm _08001C48: r0=st+0xC0 (src), r1=st+0xB0 (dst) — C0->B0, not B0->C0.
    CpuSet((const void*)(first+0xC0), (void*)(first+0xB0), 0x04000004);
    volatile u8 *st = (volatile u8 *)(uintptr_t)*slot;
    register u16 rawCount __asm__("r0") = *(volatile u16 *)(st+8);
    register u32 countOff __asm__("r3") = 8;
    register s32 signedCount __asm__("r2");
    signedCount = *(s16 *)(st + countOff);
    if (signedCount==0) {
        u32 z=(u32)signedCount; CpuSet(&z, (void*)(st+0xC0), 0x05000004);
    } else {
        *(volatile u16 *)(st+8) = rawCount-1;
    }
    slot = (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
    volatile u8 *modeState = (volatile u8 *)(uintptr_t)*slot;
    u16 cur = *(volatile u16 *)(modeState+0x10);
    if (StateA_IsGameMode(cur)) {
        register volatile u8 *state __asm__("r0") =
            (volatile u8 *)(uintptr_t)*slot;
        register u16 p __asm__("r2") = *(volatile u16 *)(state+4);
        *(volatile u16*)(state+0xC0) = p;
        _08000848((void*)(state+0xB0), 0);
    }
}
void _08001C48(void) __attribute__((alias("Idle_RefreshWorkArea")));

// _08001CD8 — input/ticker processor, reconstructed in C with an exact 316-byte match.
void Idle_Ticker(void)
{
    extern int _08001730(int mode);
    extern u32 _0800070C(void *buffer);
    extern void _080006A4(void);
    extern void sub_08004D4C(u32 a, u32 b, u32 c);
    register volatile u32 *slot __asm__("r4") =
        (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
    u32 returned;
    {
        register volatile u8 *initialState __asm__("r1") =
            (volatile u8 *)(uintptr_t)*slot;
        *(volatile u16 *)(initialState + 12) = 0;
        if (!_08001730(*(volatile u16 *)(initialState + 16)))
            goto done;
    }
    returned = _0800070C((void *)((volatile u8 *)(uintptr_t)*slot + 48));
    *(volatile u32 *)((volatile u8 *)(uintptr_t)*slot + 216) = returned;
    _080006A4();
    {
        register volatile u8 *activeState __asm__("r1") =
            (volatile u8 *)(uintptr_t)*slot;
        if (*(volatile u8 *)(activeState + 2) != 0) {
            register u32 trigger __asm__("r3") = 0;
            register volatile u8 *flagsField __asm__("r0") = activeState;
            flagsField += 216;
            register u32 flags __asm__("r2") =
                *(volatile u32 *)flagsField;
            if ((flags & 0x3000) != 0)
                goto update_timer;
            if ((flags & 0x8000) != 0) {
                register u32 lowNibble __asm__("r1") = (flags << 28) >> 28;
                register u32 inputNibble __asm__("r0") = (flags << 20) >> 28;
                if (lowNibble != inputNibble)
                    trigger = 1;
            }
            if (trigger == 0)
                goto reset_timer;
update_timer:
            {
                register volatile u32 *timerSlot __asm__("r0") =
                    (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
                activeState = (volatile u8 *)(uintptr_t)*timerSlot;
            }
            {
                register u32 timer __asm__("r0") = *(volatile u16 *)(activeState + 14);
                timer += 1;
                *(volatile u16 *)(activeState + 14) = timer;
                timer = (u16)timer;
                if (timer <= 60)
                    goto after_timer;
                *(volatile u16 *)(activeState + 14) = 0;
                {
                    register volatile u8 *callField __asm__("r0") = activeState;
                    register u32 context __asm__("r1");
                    callField += 216;
                    context = *(volatile u32 *)callField;
                    sub_08004D4C(21, context, 0);
                }
                goto after_timer;
            }
reset_timer:
            {
                register volatile u32 *resetSlot __asm__("r0") =
                    (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
                resetSlot = (volatile u32 *)(uintptr_t)*resetSlot;
                *(volatile u16 *)((volatile u8 *)resetSlot + 14) = trigger;
            }
        }
    }
after_timer:
    {
        register volatile u32 *statusSlot __asm__("r4") =
            (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
        register u32 statusMask __asm__("r5");
        register u32 statusFlagMask __asm__("r1");
        register volatile u8 *state __asm__("r1") =
            (volatile u8 *)(uintptr_t)*statusSlot;
        register u32 flags __asm__("r0") = *(volatile u32 *)(state + 216);
        statusMask = 0x80;
        if ((flags & statusMask) != 0) {
            if (*(volatile u8 *)(state + 1) != 0)
                goto status_done;
            _080006A4();
        }
        state = (volatile u8 *)(uintptr_t)*statusSlot;
        if (*(volatile u8 *)(state + 1) == 0) {
            flags = (u32)(uintptr_t)state;
            flags += 216;
            flags = *(volatile u32 *)(uintptr_t)flags;
            if ((flags & statusMask) != 0)
                _080006A4();
            {
                register volatile u8 *flagState __asm__("r2") =
                    (volatile u8 *)(uintptr_t)*statusSlot;
                flags = (u32)(uintptr_t)flagState;
                flags += 216;
                flags = *(volatile u32 *)(uintptr_t)flags;
                statusFlagMask = 0x100;
                flags &= statusFlagMask;
                if (flags != 0)
                    *(volatile u8 *)(flagState + 1) = 1;
            }
        }
    }
status_done:
    {
        register volatile u32 *lateSlot __asm__("r3") =
            (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
        register volatile u8 *counterState __asm__("r2");
        register u32 flags __asm__("r0");
        register u32 mask __asm__("r1");
        counterState = (volatile u8 *)(uintptr_t)*lateSlot;
        flags = (u32)(uintptr_t)counterState;
        flags += 216;
        flags = *(volatile u32 *)(uintptr_t)flags;
        mask = 1;
        if ((flags & mask) != 0) {
            *(volatile u16 *)(counterState + 12) = *(volatile u16 *)(counterState + 12) + 1;
        }
        counterState = (volatile u8 *)(uintptr_t)*lateSlot;
        flags = (u32)(uintptr_t)counterState;
        flags += 216;
        flags = *(volatile u32 *)(uintptr_t)flags;
        mask = 2;
        if ((flags & mask) != 0) {
            *(volatile u16 *)(counterState + 12) = *(volatile u16 *)(counterState + 12) + 1;
        }
        counterState = (volatile u8 *)(uintptr_t)*lateSlot;
        flags = (u32)(uintptr_t)counterState;
        flags += 216;
        flags = *(volatile u32 *)(uintptr_t)flags;
        mask = 4;
        if ((flags & mask) != 0) {
            *(volatile u16 *)(counterState + 12) = *(volatile u16 *)(counterState + 12) + 1;
        }
        counterState = (volatile u8 *)(uintptr_t)*lateSlot;
        flags = (u32)(uintptr_t)counterState;
        flags += 216;
        flags = *(volatile u32 *)(uintptr_t)flags;
        mask = 8;
        if ((flags & mask) != 0) {
            *(volatile u16 *)(counterState + 12) = *(volatile u16 *)(counterState + 12) + 1;
        }
    }
done:
    return;
}



void _08001CD8(void) __attribute__((alias("Idle_Ticker")));

// Remaining small thunks/accessors not in idle_accessors.c
// _08001E14 — 28 B with TWO pool words. asm/idle.s:654-669 is
//   ldr r2,=0x0802E190 / lsls r0,#2 / ldr r1,=0x030000E4 / ldr r1,[r1] /
//   ldrh r1,[r1,#6] / lsls r1,#4 / adds r0,r0,r1 / adds r0,r0,r2 /
//   ldr r0,[r0] / bx lr.
// agbcc materialises each initialiser where its declaration stands, so the
// table base, index shift, and halfword load stay in the ROM's order.
u32 Idle_GetDescriptorWord(int idx) {
    register u32 tbl __asm__("r2") = 0x0802E190;
    /* Force the pinned table base into r2 before the index shift. */
    __asm__ volatile ("" : "+r" (tbl));
    u32 off = (u32)idx << 2;
    u16 sel = *st16(6);
    u32 sum = off + ((u32)sel << 4);
    return *(u32 *)((uintptr_t)sum + (uintptr_t)tbl);
}
u32 _08001E14(int a) __attribute__((alias("Idle_GetDescriptorWord")));
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
int _08001E30(void) __attribute__((alias("Idle_IsSelZero")));
// _08001EB8(v) — `v-1` into block A +8. The ROM decrements the ARGUMENT
// (`subs r0,#1`, asm/idle.s:774), not a re-read of the cell, so the word-typed
// parameter replaces the old reload-and-decrement; the store is `strh`, so the
// upper bits of the argument are dropped exactly as the ROM drops them.
void Idle_DecScratch08(u32 v) { *st16(8) = v - 1; }
void _08001EB8(u32 v) __attribute__((alias("Idle_DecScratch08")));
void Idle_SetupMode2(u32 a, u32 b); // _08001EC8 etc. — stub behavioral
void _08001EC8(u32 a,u32 b) __attribute__((alias("Idle_SetupMode2")));
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
void _08001F08(u32 a) __attribute__((alias("Idle_SetupMode3")));
