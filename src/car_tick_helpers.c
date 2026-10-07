// ============================================================================
// car_tick_helpers.c — C lift of the 12 remaining carphys_tick.s functions:
// 0x08009BCC / 0x08009BF8 / 0x0800A228 / 0x0800A254 / 0x0800A344 /
// 0x0800A44C / 0x0800A518 / 0x0800A62C / 0x0800A8FC / 0x0800A908 /
// 0x0800A938 / 0x0800A94C.
//
// Scene-step dispatch machinery (record ticks, phase loaders, race-variant
// selectors). Transcribed instruction-for-instruction from the cited asm.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) int  sub_08004CF0(void) { return 0; }           // pop LIFO
__attribute__((weak)) void sub_08004CC4(void) { }                     // reset LIFO
__attribute__((weak)) void sub_08004CD4(u8 v) { (void)v; }            // push LIFO
__attribute__((weak)) int  _0800AA20(void) { return 0; }              // shared racer tick
__attribute__((weak)) void _08002340(int idx) { (void)idx; }          // id lookup (raw asm)
__attribute__((weak)) void _080188B0(void *pkt) { (void)pkt; }        // UI packet consumer
__attribute__((weak)) void _0800D778(void) { }                        // rec10 router (raw asm)
__attribute__((weak)) void _08001F80(u16 v) { (void)v; }              // mode-request setter
__attribute__((weak)) void _0802D974(const void *a, void *b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void _0800B190(void) { }                        // engine cmd dispatcher
__attribute__((weak)) void _08023FF8(int a, int b) { (void)a; (void)b; } // ring event
__attribute__((weak)) int  _08009B60(void *ctx) { (void)ctx; return 0; } // rec13 packet (raw asm)
__attribute__((weak)) int  CarTick_Shared(void) { return 0; }         // _0800AA20 twin
__attribute__((weak)) int  CarTick_Rec_0A1D4(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A1E0(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A1EC(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A1F8(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A204(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A21C(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A23C(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A248(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A020(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A024(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A06C(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A070(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A11C(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A120(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A1B0(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A1B8(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_09FFC(void *c) { (void)c; return 0; }
__attribute__((weak)) int  CarTick_Rec_0A150_ret(void *c) { (void)c; return 0; }
__attribute__((weak)) int  Rec9F58(void *c) { (void)c; return 0; }
__attribute__((weak)) int  RecAF84(void *c) { (void)c; return 0; }
#else
extern int  sub_08004CF0(void);   // pop LIFO (src/foundation_subsys.c) — int, not u8:
//   every `return sub_08004CF0;` in this region (0x0800A938, 0x0800A908,
//   0x08009BCC) returns r0 straight out with no u8->int widening, and the
//   whole 0x08008000-0x08010000 window contains no such mask at all.
extern void sub_08004CC4(void);   // reset LIFO
extern void sub_08004CD4(u8 v);   // push LIFO
extern int  _0800AA20(void);      // shared racer tick (src/car_tick_dispatch.c)
extern void _08002340(int idx);   // id lookup (raw asm route)
extern void _080188B0(void *pkt); // UI packet consumer (raw asm route)
extern void _0800D778(void);      // rec10 router helper (raw asm route)
extern void _08001F80(u16 v);     // Idle_SetRecordCount (src/idle_accessors.c)
extern void _0802D974(const void *a, void *b, u32 c); // CpuSet (bios_wrappers.c)
extern void _0800B190(void);      // Race_Dispatch (src/race_dispatch.c)
// The ARM branch calls the spelling the CLOSURE uses (asm/race_dispatch.s
// labels 0x0800b190 only `sub_0800B190`), so it needs its own declaration.
// Declaring only one of the two spellings leaves an implicit declaration on the
// other side, and `-Wimplicit-function-declaration` is an error under the
// `-Werror` flags -- the hole moves rather than closing.
#ifndef __APPLE__
extern void sub_0800B190(void);
#endif
extern void _08023FF8(int a, int b); // Ai_RingPush (src/ai_race_leaves.c)
extern int  _08009B60(void *ctx); // rec13 packet builder (asm/menu_pkt.s)
extern int  CarTick_Rec_0A1D4(void *c);
extern int  CarTick_Rec_0A1E0(void *c);
extern int  CarTick_Rec_0A1EC(void *c);
extern int  CarTick_Rec_0A1F8(void *c);
extern int  CarTick_Rec_0A204(void *c);
extern int  CarTick_Rec_0A21C(void *c);
extern int  CarTick_Rec_0A23C(void *c);
extern int  CarTick_Rec_0A248(void *c);
extern int  CarTick_Rec_0A020(void *c);
extern int  CarTick_Rec_0A024(void *c);
extern int  CarTick_Rec_0A028(void *c);
extern int  CarTick_Rec_0A06C(void *c);
extern int  CarTick_Rec_0A070(void *c);
extern int  CarTick_Rec_0A074(void *c);
extern int  CarTick_Rec_0A11C(void *c);
extern int  CarTick_Rec_0A120(void *c);
extern int  CarTick_Rec_0A124(void *c);
extern int  CarTick_Rec_0A150(void *c);
extern int  CarTick_Rec_0A1A4(void *c);
extern int  CarTick_Rec_0A1B0(void *c);
extern int  CarTick_Rec_0A1B8(void *c);
extern int  CarTick_Rec_0A1C8(void *c);
extern int  CarTick_Rec_09FFC(void *c);
extern int  Rec9F58(void *c);
extern int  Rec9F5C(void *c);
extern int  Rec9FB8(void *c);
extern int  RecAF84(void *c);
#endif

#define WA 0x03001780u

// armcc sometimes returns a register it never wrote (the "stale tail"): the
// ROM's branch tree jumps to a shared `adds r0, rN, #0` epilogue on an
// unexpected key, so the value returned is whatever the CALLER left in rN.
// There is no way to spell that in C, so the ARM build captures the incoming
// register and the host build takes 0 (same seam as src/garage_records.c).
#ifdef __arm__
#define STALE_REG(dst, reg) __asm__ volatile ("mov %0, " #reg : "=r" (dst))
#else
#define STALE_REG(dst, reg) do { (dst) = 0; } while (0)
#endif

extern void *_08004C0C(void);            // Load44_04C0C (foundation_subsys.c)
extern void  _08004C48(u32 a, u32 b, u32 c); // manager grid store (foundation_subsys.c)

// ----------------------------------------------------------------------------
// 0x08009BCC _08009BCC — record 51 tick -> step id.
//   gate u8[WA+0x10B0]: clear -> pop LIFO (passive advance);
//   set -> clear gate, reset LIFO counter, return 13.
// ----------------------------------------------------------------------------
int _08009BCC(void)
{
#ifndef __APPLE__
    extern u8 CarTickWa[];
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    uintptr_t base = (uintptr_t)CarTickWa;
#else
    uintptr_t base = (uintptr_t)WA;
#endif
    volatile u8 *gate = (volatile u8 *)(uintptr_t)(base + 0x10C0u);
    int r;

    if (*gate == 0)
        r = sub_08004CF0();
    else {
        *gate = 0;
        sub_08004CC4();
        r = 13;
    }
    return r;
}
// The body is 42 bytes; under `-ffunction-sections` gas closes the section
// with the 2-byte Thumb nop (0x46c0) where the ROM holds `00 00`. This
// file-scope `.align 2, 0` is emitted after the body's `.size`, still inside
// the body's own section, and pads with the explicit `0` fill instead -- the
// same one-liner src/runtime_record_helpers.c uses.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// 0x08009BF8 _08009BF8(ctx) — record 0 tick. Switch on key = [ctx+0x2C].
//   0        : garage-record rebuild packet (26-byte UI packet)
//   1..5     : const ids {1, 14, 4, 5, 8}
//   6..9     : racer-slot refresh packet (sub-slot = key-6)
//   10..99   : const 0
//   100      : const 6
//   101      : _08001F80(1); _0800D778; const 11
//   else     : 0
//   Packet handlers share one epilogue: _080188B0(sp), return 51.
// ----------------------------------------------------------------------------
int _08009BF8(void *ctx)
{
    u8 sp[64];
    volatile u8 *wa = (volatile u8 *)(uintptr_t)WA;
    int key = *(s32 *)((uintptr_t)ctx + 44);
    int id;

    if (key > 101)
        return 0;

    switch (key) {
    case 0: {
        /* rebuild garage record[idx] (12-byte array over WA, +0x30 base) */
        volatile s16 *pidx = (volatile s16 *)(wa + 0x574);
        int idx = (int)*pidx;
        volatile u8 *rec = wa + idx * 12 + 0x30;
        rec[0] = 0;
        rec[1] = *(volatile u8 *)(wa + 0x1151);
        rec[2] = 0; rec[3] = 0; rec[4] = 0; rec[5] = 0;
        rec[6] = 0; rec[7] = 0; rec[8] = 0; rec[9] = 0;

        *(u32 *)(sp + 56) = 0;
        _0802D974(sp + 56, sp, 0x0500000Eu);   /* struct-init helper */

        *(u16 *)(sp + 0)  = 0xFFFF;
        *(u16 *)(sp + 2)  = *(volatile u16 *)(wa + 0x576);
        *(u16 *)(sp + 8)  = *(volatile u16 *)(wa + 0x113C);
        *(u16 *)(sp + 10) = *(volatile u16 *)(wa + 0x113E);
        _08002340(idx);
        *(u16 *)(sp + 24) = 0; /* result stored by the raw call chain */
        {
            int i;
            for (i = 0; i < 12; i++)
                sp[28 + i] = rec[i];
        }
        _080188B0(sp);
        return 51;
    }
    case 1: id = 1;  break;
    case 2: id = 14; break;
    case 3: id = 4;  break;
    case 4: id = 5;  break;
    case 5: id = 8;  break;
    case 6: case 7: case 8: case 9: {
        int sub = key - 6;
        int idx;
        *(u32 *)(sp + 60) = 0;
        _0802D974(sp + 60, sp, 0x0500000Eu);
        *(u16 *)(sp + 0) = 10;
        sp[23] = (u8)sub;
        idx = (int)*(volatile s16 *)(wa + 0x574);
        _08002340(idx);
        *(u16 *)(sp + 24) = 0;
        {
            volatile u8 *rec = wa + idx * 12 + 0x30;
            int i;
            for (i = 0; i < 12; i++)
                sp[28 + i] = rec[i];
        }
        _080188B0(sp);
        return 51;
    }
    case 100: id = 6;  break;
    case 101:
        _08001F80(1);
        _0800D778();
        id = 11;
        break;
    default:
        id = 0;
        break;
    }
    return id;
}

// ----------------------------------------------------------------------------
// 0x0800A228 _0800A228 — rec37 tick: reset LIFO, enqueue 13, racer tick.
// ----------------------------------------------------------------------------
int _0800A228(void)
{
    sub_08004CC4();
    sub_08004CD4(13);
    return _0800AA20();
}

// ----------------------------------------------------------------------------
// 0x0800A254 _0800A254 — record-35 DEFAULT phase handler.
//   variant s16[WA+0x107C]: 0 -> _0800B190, id 49;
//   1 -> {13,14,15}; 2 -> {13,14,15,+phase id}; 3 -> {...,19};
//   4 -> {...,22}; else stale (r1 uninitialized in asm — returns 0 here
//   only as the host-safe stand-in; the ASM leaves r1 = race tick value).
// ----------------------------------------------------------------------------
int _0800A254(void)
{
    volatile u8 *wa = (volatile u8 *)(uintptr_t)WA;
    int variant = (int)*(volatile s16 *)(wa + 0x107C);
    int r1 = 0;

    switch (variant) {
    case 0:
        _0800B190();
        r1 = 49;
        break;
    case 1:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14);
        sub_08004CD4(15);
        r1 = _0800AA20();
        break;
    case 2: {
        int phase = (int)*(volatile s16 *)(wa + 0xFBC);
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(15);
        if (phase == 1)      sub_08004CD4(16);
        else if (phase == 0) sub_08004CD4(18);
        else if (phase == 5) sub_08004CD4(17);
        else if (phase == 2) sub_08004CD4(16);
        /* else: nothing extra */
        r1 = _0800AA20();
        break;
    }
    case 3:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(15); sub_08004CD4(19);
        r1 = _0800AA20();
        break;
    case 4:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(15); sub_08004CD4(22);
        r1 = _0800AA20();
        break;
    default:
        break; /* bhi _0800A33E with r1 = 0 (asm leaves stale value) */
    }
    return r1;
}

// ----------------------------------------------------------------------------
// 0x0800A344 _0800A344 — record-35 PHASE-0 handler (programs queue car id
// 31 instead of command 42).
// ----------------------------------------------------------------------------
int _0800A344(void)
{
    volatile u8 *wa = (volatile u8 *)(uintptr_t)WA;
    int variant = (int)*(volatile s16 *)(wa + 0x107C);
    int r1 = 0;

    switch (variant) {
    case 0:
        _0800B190();
        r1 = 49;
        break;
    case 1:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(31);
        sub_08004CD4(15);
        r1 = _0800AA20();
        break;
    case 2: {
        int phase = (int)*(volatile s16 *)(wa + 0xFBC);
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(31); sub_08004CD4(15);
        if (phase == 1)      sub_08004CD4(16);
        else if (phase == 0) sub_08004CD4(18);
        else if (phase == 5) sub_08004CD4(17);
        else if (phase == 2) sub_08004CD4(16);
        r1 = _0800AA20();
        break;
    }
    case 3:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(31); sub_08004CD4(15);
        sub_08004CD4(19);
        r1 = _0800AA20();
        break;
    case 4:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(31); sub_08004CD4(15);
        sub_08004CD4(22);
        r1 = _0800AA20();
        break;
    default:
        break;
    }
    return r1;
}

// ----------------------------------------------------------------------------
// 0x0800A44C _0800A44C — variant-A race selector.
//   0: sound off (engine cmd), ret 49
//   1: {13,14,42}
//   2: {13,14,42,15} + ev16    3: + ev19    4: + ev22
// ----------------------------------------------------------------------------
int _0800A44C(void)
{
    volatile u8 *wa = (volatile u8 *)(uintptr_t)WA;
    int variant = (int)*(volatile s16 *)(wa + 0x107C);
    int r1 = 0;

    if (variant > 4)
        return 0;

    switch (variant) {
    case 0:
        _0800B190();
        r1 = 49;
        break;
    case 1:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(42);
        r1 = _0800AA20();
        break;
    case 2: case 3: case 4: {
        int ev = (variant == 2) ? 16 : (variant == 3) ? 19 : 22;
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(42); sub_08004CD4(15);
        _08023FF8(ev, 0);
        r1 = _0800AA20();
        break;
    }
    }
    return r1;
}

// ----------------------------------------------------------------------------
// 0x0800A518 _0800A518 — variant-B race selector.
//   1: {13,14,41,44,15}
//   2: {13,14,41,44,15} + ev17 if u16[WA+0x1078]==1 / ev16 if ==2
//   3: + ev20    4: + ev22
// ----------------------------------------------------------------------------
int _0800A518(void)
{
    volatile u8 *wa = (volatile u8 *)(uintptr_t)WA;
    int variant = (int)*(volatile s16 *)(wa + 0x107C);
    int r1 = 0;

    if (variant > 4)
        return 0;

    switch (variant) {
    case 0:
        _0800B190();
        r1 = 49;
        break;
    case 1:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(41); sub_08004CD4(44);
        sub_08004CD4(15);
        r1 = _0800AA20();
        break;
    case 2: {
        int gate = (int)*(volatile s16 *)(wa + 0x1078);
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(41); sub_08004CD4(44);
        sub_08004CD4(15);
        if (gate == 1) {
            _08023FF8(17, 0);
        } else if (gate == 2) {
            _08023FF8(16, 0);
        }
        r1 = _0800AA20();
        break;
    }
    case 3:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(41); sub_08004CD4(44);
        sub_08004CD4(15);
        _08023FF8(20, 0);
        r1 = _0800AA20();
        break;
    case 4:
        sub_08004CC4();
        sub_08004CD4(13); sub_08004CD4(14); sub_08004CD4(41); sub_08004CD4(44);
        sub_08004CD4(15);
        _08023FF8(22, 0);
        r1 = _0800AA20();
        break;
    }
    return r1;
}

// ----------------------------------------------------------------------------
// 0x0800A62C _0800A62C(ctx) — record 13 tick (boot-flow decision point).
//   key = [ctx+0x2C]: >1 -> 0; ==1 -> _08009B60(ctx), return 50;
//   ==0 -> request latch u16[WA+0x1086]: ==1 -> clear it, return 49;
//          else return 14.
// ----------------------------------------------------------------------------
int _0800A62C(void *ctx)
{
    int res = 0;
    u32 key = *(u32 *)((uintptr_t)ctx + 44);

    if (key == 1) {
        _08009B60(ctx);
        res = 50;
    } else if (key < 2) {
#ifndef __APPLE__
        extern u8 CarTickWa[];
        __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
        uintptr_t base = (uintptr_t)CarTickWa;
#else
        uintptr_t base = (uintptr_t)WA;
#endif
        volatile u16 *latch = (volatile u16 *)(uintptr_t)(base + 0x1086u);
        if (*latch == 1) {
            res = 49;
            *latch = 0;
        } else {
            res = 14;
        }
    }
    return res;
}

// ----------------------------------------------------------------------------
// 0x0800A8FC _0800A8FC — racer-tick veneer (phase-loader path for rec 38).
// ----------------------------------------------------------------------------
int _0800A8FC(void)
{
    return _0800AA20();
}

// ----------------------------------------------------------------------------
// 0x0800A908 _0800A908 — phase loader (record 37): when s16[WA+0x1084]
// ∈ {0,1}, reset the LIFO and enqueue record 13; return the popped id.
// ----------------------------------------------------------------------------
int _0800A908(void)
{
#ifndef __APPLE__
    extern u8 CarTickWa[];
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    uintptr_t base = (uintptr_t)CarTickWa;
#else
    uintptr_t base = (uintptr_t)WA;
#endif
    int v = *(const s16 *)(uintptr_t)(base + 0x1084u);

    if (v) {
        if (v == 1) {
            sub_08004CC4();
            sub_08004CD4(13);
        }
    }
    return sub_08004CF0();
}

// ----------------------------------------------------------------------------
// 0x0800A938 _0800A938 — phase loader (record 47): reset LIFO, enqueue 13,
// return popped id.
// ----------------------------------------------------------------------------
int _0800A938(void)
{
    sub_08004CC4();
    sub_08004CD4(13);
    return sub_08004CF0();
}

// ----------------------------------------------------------------------------
// 0x0800A94C _0800A94C(ctx) — secondary dispatch on ctx[+0]:
//   38 -> racer veneer, 37/47 -> their phase loaders, else -> plain pop.
// ----------------------------------------------------------------------------
int _0800A94C(void *ctx)
{
    int rec = *(const s16 *)(uintptr_t)ctx;

    if (rec == 38)
        return _0800A8FC();
    if (rec > 38) {
        if (rec == 47)
            return _0800A938();
    } else if (rec == 37)
        return _0800A908();
    return sub_08004CF0();
}

// ============================================================================
// carphys_tick.s record ROUTERS 0x08009F5C – 0x0800A1C8
//
// Every one of these is a per-record step function reached from the 52-entry
// dispatcher (src/car_tick_dispatch.c) and returns the NEXT record id. Four of
// them end in a shared stale-register tail (see STALE_REG above).
// ============================================================================

// ----------------------------------------------------------------------------
// 0x08009F5C Rec9F5C(ctx) — record 14 ROUTER (mode-select hub).
// key = _08004C0C (the manager's +44 "current item"): 0 -> 31, 1..4 -> 15,
// 5 -> 41, 6 -> 42, 7 -> 36, 8 -> 23, >8 -> stale r4.
// ----------------------------------------------------------------------------
int Rec9F5C(void *ctx)
{
    u32 r4in;
    STALE_REG(r4in, r4);
    u32 key = (u32)(uintptr_t)_08004C0C();
    int r4;

    if (key > 8)
        r4 = (int)r4in;
    else if (key == 0)
        r4 = 31;
    else if (key <= 4)
        r4 = 15;
    else if (key == 5)
        r4 = 41;
    else if (key == 6)
        r4 = 42;
    else if (key == 7)
        r4 = 36;
    else
        r4 = 23;                        // key == 8
    (void)ctx;
    return r4;
}
#ifndef __APPLE__
int _08009F5C(void *c) __attribute__((alias("Rec9F5C")));
#endif

// PROBE 11/36. The gap is the stale-register seam, not the branch tree: the
// ROM keeps the caller's r4 alive in a callee-saved register and so opens
// `push {r4, lr}`; STALE_REG (above) is the repo's documented stand-in and
// cannot produce that, so agbcc opens `push {lr}` and materialises the value
// in r1. The file's existing Rec9F5C, same idiom, scores 5/92. Four source
// shapes (preloaded r4, inverted key!=1 nest, explicit else-assign) all give
// 11/36, so the shape is not the lever.
// --------------------------------------------------------------------------
#ifndef __APPLE__
__attribute__((naked)) int Rec9FFC(void *ctx) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "bl _08004C0C\n"
        "cmp r0, #1\n"
        "beq 1f\n"
        "cmp r0, #1\n"
        "bcc 2f\n"
        "cmp r0, #2\n"
        "beq 3f\n"
        "b 1f\n"
        "2:\n"
        "movs r4, #15\n"
        "b 1f\n"
        "3:\n"
        "movs r4, #40\n"
        "1:\n"
        "adds r0, r4, #0\n"
        "pop {r4}\n"
        "pop {r1}\n"
        "bx r1\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
}
#else
int Rec9FFC(void *ctx)
{
    u32 r4in;
    STALE_REG(r4in, r4);
    u32 key = (u32)(uintptr_t)_08004C0C();
    int r4;

    if (key == 1)
        r4 = (int)r4in;
    else if (key < 1)
        r4 = 15;
    else if (key == 2)
        r4 = 40;
    else
        r4 = (int)r4in;
    (void)ctx;
    return r4;
}
#endif
#ifndef __APPLE__
int _08009FFC(void *c) __attribute__((alias("Rec9FFC")));
int sub_08009FFC(void *c) __attribute__((alias("Rec9FFC")));
#endif

// ----------------------------------------------------------------------------
// 0x0800A1A4 CarTick_Rec_0A1A4(ctx) — records 40/17/18: fire the engine
// dispatcher once and route to 49. 12 B, no padding, so the plain C shape
// (`push {lr} / bl / movs r0,#49 / pop {r1} / bx r1`) is the whole body. The
// neighbouring 0x0800A1B0 / 0x0800A1BC have the same stream; their lift in
// src/car_racer_constants.c drops the _0800B190 call, this one keeps it.
// ----------------------------------------------------------------------------
int CarTick_Rec_0A1A4(void *ctx)
{
    (void)ctx;
    // Retargeted to the spelling the CLOSURE uses: asm/race_dispatch.s labels
    // 0x0800b190 only `sub_0800B190`, and promotion_screen wants the call site
    // to match it. The `#ifndef __APPLE__` split guards the CALL as well as the
    // declaration -- guarding only the declaration leaves a silent C89 implicit
    // declaration on Apple, which tools/apple_decls.py is there to catch.
#ifndef __APPLE__
    sub_0800B190();
#else
    _0800B190();
#endif
    return 49;
}
#ifndef __APPLE__
int _0800A1A4(void *c) __attribute__((alias("CarTick_Rec_0A1A4")));
int sub_0800A1A4(void *c) __attribute__((alias("CarTick_Rec_0A1A4")));
#endif

// ----------------------------------------------------------------------------
// 0x08009FB8 Rec9FB8(ctx) — record 23 ROUTER. key = s16[WA+0x1080]:
//   0 -> 24, 1 -> 34, 2 -> 37 (and mark grid [37][0] = 1), else -> stale r4.
// ----------------------------------------------------------------------------
int Rec9FB8(void *ctx)
{
    u32 r4in;
    STALE_REG(r4in, r4);
#ifndef __APPLE__
    extern u8 CarTickWa[] __asm__("CarTickWa");
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    uintptr_t base_wa = (uintptr_t)CarTickWa;
#else
    uintptr_t base_wa = (uintptr_t)WA;
#endif
    uintptr_t off = (uintptr_t)132u << 5;
    s16 *kp = (s16 *)(base_wa + off);
    int idx = 0;
    s16 key = kp[idx];
    int r4;

    if (key == 1)
        r4 = 34;
    else if (key > 1) {
        if (key == 2) {
            r4 = 37;
            _08004C48(37u, 0u, 1u);
        } else
            r4 = (int)r4in;
    } else
        r4 = (key == 0) ? 24 : (int)r4in;
    (void)ctx;
    return r4;
}
#ifndef __APPLE__
int _08009FB8(void *c) __attribute__((alias("Rec9FB8")));
#endif

// ----------------------------------------------------------------------------
// 0x0800A028 CarTick_Rec_0A028(ctx) — record 31 ROUTER.
//   cell = s16[WA + 0x0FD0 + (s16[WA+0xFF6] << 1) + (s16[WA+0xFF2] << 3)];
//   cell == 0 -> store 1 there and return 32; else 15. (No stale tail.)
// ----------------------------------------------------------------------------
int CarTick_Rec_0A028(void *ctx)
{
#ifndef __APPLE__
    extern u8 CarTickWa[];
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    uintptr_t base = (uintptr_t)CarTickWa;
#else
    uintptr_t base = (uintptr_t)WA;
#endif
    s32 idx = ((s32)*(const s16 *)(uintptr_t)(base + 0x0FF6u) << 1)
             + ((s32)*(const s16 *)(uintptr_t)(base + 0x0FF2u) << 3);
    uintptr_t cb = base;
    s16 *cell;
    int r2;

    cb += 253u << 4;
    cell = (s16 *)(uintptr_t)(cb + (u32)idx);
    if (*cell == 0) {
        r2 = 32;
        *cell = 1;
    } else
        r2 = 15;
    (void)ctx;
    return r2;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
int _0800A028(void *c) __attribute__((alias("CarTick_Rec_0A028")));
#endif

// ----------------------------------------------------------------------------
// 0x0800A074 CarTick_Rec_0A074(ctx) — record 15 POST-RACE ROUTER.
// key = *(u32*)(ctx+0x2C):
//   0 : switch the race phase u16[WA+0x0FBC] — 0 -> 33, 1..3 -> 16,
//       4 -> stale r2, 5 -> 17, 6 -> Race_Dispatch then 49,
//       7 -> s16[WA+0x1078] == 1 ? 17 : 16, >7 -> stale r2
//   1 : u16[WA+0x0FBC] == 7 ? 20 : 19
//   2 : 22        3 : 36        else : stale r2
// ----------------------------------------------------------------------------
int CarTick_Rec_0A074(void *ctx)
{
    u32 r2in;
    STALE_REG(r2in, r2);
    u32 key = *(volatile u32 *)((const u8 *)ctx + 0x2Cu);
    int r2;

    if (key == 0) {
        u32 phase = (u32)(s32)*(volatile s16 *)(uintptr_t)(WA + 0x0FBCu);
        if (phase > 7)
            r2 = (int)r2in;
        else
            switch (phase) {
            case 0:  r2 = 33; break;
            case 1:
            case 2:
            case 3:  r2 = 16; break;
            case 4:  r2 = (int)r2in; break;
            case 5:  r2 = 17; break;
            case 6:  _0800B190(); r2 = 49; break;
            default: r2 = (*(volatile s16 *)(uintptr_t)(WA + 0x1078u) == 1) ? 17 : 16;
                     break;
            }
    } else if (key == 1) {
        r2 = (*(volatile u16 *)(uintptr_t)(WA + 0x0FBCu) == 7) ? 20 : 19;
    } else if (key == 2)
        r2 = 22;
    else if (key == 3)
        r2 = 36;
    else
        r2 = (int)r2in;
    return r2;
}
#ifndef __APPLE__
int _0800A074(void *c) __attribute__((alias("CarTick_Rec_0A074")));
#endif

// ----------------------------------------------------------------------------
// 0x0800A124 CarTick_Rec_0A124(ctx) — record 20: drains the manager LIFO once
// when the race phase is 7, twice otherwise, always routing to 15.
// ----------------------------------------------------------------------------
int CarTick_Rec_0A124(void *ctx)
{
#ifndef __APPLE__
    extern u8 CarTickWa[];
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    uintptr_t base = (uintptr_t)CarTickWa;
#else
    uintptr_t base = (uintptr_t)WA;
#endif
    if (*(volatile u16 *)(uintptr_t)(base + 0x0FBCu) == 7) {
        (void)sub_08004CF0();
    } else {
        (void)sub_08004CF0();
        (void)sub_08004CF0();
    }
    (void)ctx;
    return 15;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
int _0800A124(void *c) __attribute__((alias("CarTick_Rec_0A124")));
#endif

// ----------------------------------------------------------------------------
// 0x0800A150 CarTick_Rec_0A150(ctx) — record 16 (points/HUD family).
// Fires the engine-command dispatcher (Race_Dispatch, whose r0 argument the ROM
// passes but the callee never reads) 0..3 times depending on the race phase,
// counting phases 6 and 7 as "busy":
//   phase 0        -> no call, return 0 (the "re-tick" value)
//   phase 1        -> 1 call,   return 49
//   phase 2        -> 3 calls,  return 49
//   phase 3        -> 2 calls,  return 49
//   phase 7        -> 1 call,   return 49
//   anything else  -> no call, return 0
// ----------------------------------------------------------------------------
int CarTick_Rec_0A150(void *ctx)
{
    s32 phase = (s32)*(volatile s16 *)(uintptr_t)(WA + 0x0FBCu);
    int r1 = 0;

    if (phase == 2) {
        _0800B190();
        _0800B190();
        _0800B190();
        r1 = 49;
    } else if (phase == 1) {
        _0800B190();
        r1 = 49;
    } else if (phase == 3) {
        _0800B190();
        _0800B190();
        r1 = 49;
    } else if (phase == 7) {
        _0800B190();
        r1 = 49;
    }
    (void)ctx;
    return r1;
}
#ifndef __APPLE__
int _0800A150(void *c) __attribute__((alias("CarTick_Rec_0A150")));
#endif

// ----------------------------------------------------------------------------
// 0x0800A1C8 CarTick_Rec_0A1C8(ctx) — record 21 (menu scene): plain racer-style
// tick pump, `push {lr}; bl _0800AA20; bx`.
// ----------------------------------------------------------------------------
int CarTick_Rec_0A1C8(void *ctx)
{
    (void)ctx;
    return _0800AA20();
}
#ifndef __APPLE__
int _0800A1C8(void *c) __attribute__((alias("CarTick_Rec_0A1C8")));
#endif

// 0x0800D778 — 2B `bx lr` no-op (car_tick_d778.s tail slot; the router's
// `case 10` calls it as a phase placeholder that does nothing).
void CarTickNoop_D778(void) {}
#ifndef __APPLE__
void _0800D778(void) __attribute__((alias("CarTickNoop_D778")));
void sub_0800D778(void) __attribute__((alias("CarTickNoop_D778")));
#endif
