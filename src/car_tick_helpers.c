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

extern int  sub_08004CF0(void);   // pop LIFO (src/foundation_subsys.c) — int, not u8:
//   every `return sub_08004CF0;` in this region (0x0800A938, 0x0800A908,
//   0x08009BCC) returns r0 straight out with no u8->int widening, and the
//   whole 0x08008000-0x08010000 window contains no such mask at all.
extern void sub_08004CC4(void);   // reset LIFO
extern void sub_08004CD4(u32 v);  // push LIFO (CounterBump takes u32)
extern int  _0800AA20(void);      // shared racer tick (src/car_tick_dispatch.c)
extern int  _08002340(int idx);   // id lookup (raw asm route)
extern void sub_0800188B0(void *pkt); // UI packet consumer at ROM +0x188B0
extern void _0800D778(void);      // rec10 router helper (raw asm route)
extern void _08001F80(u16 v);     // Idle_SetRecordCount (src/idle_accessors.c)
extern void sub_0802D974(const void *a, void *b, u32 c); // CpuSet wrapper
extern void _0800B190(void);      // Race_Dispatch (src/race_dispatch.c)
// The ARM branch calls the spelling the CLOSURE uses (asm/race_dispatch.s
// labels 0x0800b190 only `sub_0800B190`), so it needs its own declaration.
// Declaring only one of the two spellings leaves an implicit declaration on the
// other side, and `-Wimplicit-function-declaration` is an error under the
// `-Werror` flags -- the hole moves rather than closing.
extern void sub_0800B190(void);
extern void CarTickDispatch_R0(int arg) __asm__("sub_0800B190");
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
    extern u8 CarTickWa[];
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    uintptr_t base = (uintptr_t)CarTickWa;
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
    extern u8 CarTickWa[];
    register u32 slot __asm__("r4") = 0;
    register int result __asm__("r2") = 0;
    u32 key;
    u8 packet[64];

    __asm__ volatile(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    __asm__ volatile("" : "+r" (result), "+r" (slot));
    key = *(const u32 *)((const u8 *)ctx + 0x2Cu);

    switch (key) {
    case 0: {
        /* This is ordinary IWRAM state, not a hardware register. Keep each
         * read separate: a volatile record-byte write can alias WA+0x574
         * when the cursor value is 112. */
        s16 *cursor;

        slot = (u32)CarTickWa;
        cursor = (s16 *)(uintptr_t)(slot + 0x574u);
        {
            /* Keep the volatile record cursor in the ROM's r0 register. */
            register volatile u8 *record __asm__("r0");
            u8 recordByte;

            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            record += 48;
            result = 0;
            record[0] = (u8)result;
            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            recordByte = *(volatile u8 *)(uintptr_t)(slot + 0x1151u);
            record += 49;
            record[0] = recordByte;
            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            record += 50;
            record[0] = (u8)result;
            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            record += 51;
            record[0] = (u8)result;
            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            record += 52;
            record[0] = (u8)result;
            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            record += 53;
            record[0] = (u8)result;
            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            record += 54;
            record[0] = (u8)result;
            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            record += 55;
            record[0] = (u8)result;
            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            record += 56;
            record[0] = (u8)result;
            record = (volatile u8 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            record += 57;
            // The key-0 path keeps result at zero until the shared epilogue.
            record[0] = (u8)result;
        }

        *(u32 *)(packet + 56) = (u32)result;
        sub_0802D974(packet + 56, packet, 0x0500000Eu);
        {
            register u16 *packetWords __asm__("r1");
            register u32 fieldOffset __asm__("r2");
            register u32 fieldOffset113e __asm__("r3");
            u32 headerWord;
            u16 fieldValue;
            u16 fieldValue113e;
            u16 fieldId;
            packetWords = (u16 *)packet;
            headerWord = 0xFFFFu;
            __asm__ volatile ("" : "+r" (headerWord));
            packetWords[0] = (u16)headerWord;
            fieldOffset = 0x576u;
            __asm__ volatile ("" : "+r" (fieldOffset));
            fieldValue = *(volatile u16 *)(uintptr_t)(slot + fieldOffset);
            __asm__ volatile ("" : "+r" (fieldOffset));
            packetWords[1] = fieldValue;
            fieldOffset113e = 0x113Eu;
            __asm__ volatile ("" : "+r" (fieldOffset113e));
            fieldValue113e = *(volatile u16 *)(uintptr_t)(slot + fieldOffset113e);
            __asm__ volatile ("" : "+r" (fieldOffset113e));
            packetWords[5] = fieldValue113e;
            packetWords[4] = *(volatile u16 *)(uintptr_t)(slot + 0x113Cu);
            fieldId = (u16)_08002340((int)(*cursor));
            packetWords = (u16 *)packet;
            packetWords[12] = fieldId;
        }
        {
            /* The ROM keeps the source in r0 for its three-word copy. */
            register const u32 *sourceWords __asm__("r0");
            u32 *destWords;

            sourceWords = (const u32 *)(uintptr_t)((s32)(*cursor) * 12 + slot);
            __asm__ volatile ("" : "+r" (sourceWords) : : "memory");
            destWords = (u32 *)(packet + 28);
            __asm__ volatile ("" : "+r" (destWords) : : "memory");
            sourceWords += 12;
            __asm__ volatile ("" : : "r" (sourceWords) : "memory");
            __builtin_memcpy(destWords, sourceWords, 3 * sizeof(u32));
        }
        goto packet_epilogue;
    }
    case 1:
        result = 1;
        break;
    case 2:
        result = 14;
        break;
    case 3:
        result = 4;
        break;
    case 4:
        result = 5;
        break;
    case 5:
        result = 8;
        break;
    case 9:
        slot++;
    case 8:
        slot++;
    case 7:
        slot++;
    case 6: {
        s16 *cursor;
        const u32 *sourceWords;
        u32 *destWords;

        *(u32 *)(packet + 60) = 0;
        sub_0802D974(packet + 60, packet, 0x0500000Eu);
        ((u16 *)packet)[0] = 10;
        packet[23] = (u8)slot;
        register u32 waBase __asm__("r5");
        waBase = (u32)CarTickWa;
        cursor = (s16 *)(uintptr_t)(waBase + 0x574u);
        ((u16 *)packet)[12] = (u16)_08002340((int)(*cursor));
        sourceWords = (const u32 *)(uintptr_t)((s32)(*cursor) * 12 + waBase);
        __asm__ volatile ("" : "+r" (sourceWords) : : "memory");
        destWords = (u32 *)(packet + 28);
        __asm__ volatile ("" : "+r" (destWords) : : "memory");
        sourceWords += 12;
        __asm__ volatile ("" : : "r" (sourceWords) : "memory");
        __builtin_memcpy(destWords, sourceWords, 3 * sizeof(u32));
        __asm__ volatile ("" : : "r" (waBase) : "memory");
        goto packet_epilogue;
    }
packet_epilogue:
        sub_0800188B0(packet);
        result = 51;
        break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
        break;
    case 100:
        result = 6;
        break;
    case 101:
        _08001F80(1);
        _0800D778();
        result = 11;
        break;
    default:
        break;
    }
    return result;
}
// The ROM's 862-byte body ends with two zero padding bytes, not a Thumb nop.
__asm__(".align 2, 0");
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
//   4 -> {...,22}; else stale (r1 remains live from the caller).
//   The C switch keeps the ROM's five-entry jump table; variant 2's phase
//   checks retain their ordered branch tree. The out-of-range selector returns
//   caller r1 unchanged, captured by the empty output-only asm claim.
// ----------------------------------------------------------------------------
int _0800A254(void)
{
    extern u8 CarTickWa[];
    register int result __asm__("r1");
    register u32 queueArg __asm__("r0");
    u32 variant;
    s16 phase;
    uintptr_t base;

    __asm__ volatile(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    __asm__ volatile("" : "=r" (result));
    base = (uintptr_t)CarTickWa;
    variant = (u32)*(const s16 *)(uintptr_t)(base + 0x107Cu);
    if (variant > 4u)
        return result;

    switch (variant) {
    case 0:
        sub_0800B190();
        result = 49;
        break;
    case 1:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        queueArg = 15;
        goto queue_done;
    case 2:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(15);
        base = (uintptr_t)CarTickWa;
        phase = *(const s16 *)(uintptr_t)(base + 0x0FBCu);
        if (phase == 1)
            goto phase1;
        if (phase > 1)
            goto phase_gt1;
        if (phase == 0)
            goto phase0;
        goto phase_done;
    phase_gt1:
        if (phase == 2)
            goto phase2;
        if (phase == 5)
            goto phase5;
        goto phase_done;
    phase0:
        queueArg = 18;
        goto queue_done;
    phase5:
        queueArg = 17;
        goto queue_done;
    phase1:
        queueArg = 16;
        goto queue_done;
    phase2:
        sub_08004CD4(16);
        goto phase_done;
    case 3:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(15);
        queueArg = 19;
        goto queue_done;
    case 4:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(15);
        queueArg = 22;
    queue_done:
        sub_08004CD4(queueArg);
    phase_done:
        result = _0800AA20();
        break;
    }
    return result;
}

// ----------------------------------------------------------------------------
// 0x0800A344 _0800A344 — record-35 PHASE-0 handler. It uses the same
// variant/phase route as _0800A254, with car id 31 inserted before command 15
// in variants 1-4. The C switch preserves its jump table and phase branch tree;
// out-of-range variants return caller r1 unchanged.
// ----------------------------------------------------------------------------
int _0800A344(void)
{
    extern u8 CarTickWa[];
    register int result __asm__("r1");
    register u32 queueArg __asm__("r0");
    u32 variant;
    s16 phase;
    uintptr_t base;

    __asm__ volatile(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    __asm__ volatile("" : "=r" (result));
    base = (uintptr_t)CarTickWa;
    variant = (u32)*(const s16 *)(uintptr_t)(base + 0x107Cu);
    if (variant > 4u)
        return result;

    switch (variant) {
    case 0:
        sub_0800B190();
        result = 49;
        break;
    case 1:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(31);
        queueArg = 15;
        goto queue_done;
    case 2:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(31);
        sub_08004CD4(15);
        base = (uintptr_t)CarTickWa;
        phase = *(const s16 *)(uintptr_t)(base + 0x0FBCu);
        if (phase == 1)
            goto phase1;
        if (phase > 1)
            goto phase_gt1;
        if (phase == 0)
            goto phase0;
        goto phase_done;
    phase_gt1:
        if (phase == 2)
            goto phase2;
        if (phase == 5)
            goto phase5;
        goto phase_done;
    phase0:
        queueArg = 18;
        goto queue_done;
    phase5:
        queueArg = 17;
        goto queue_done;
    phase1:
        queueArg = 16;
        goto queue_done;
    phase2:
        sub_08004CD4(16);
        goto phase_done;
    case 3:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(31);
        sub_08004CD4(15);
        queueArg = 19;
        goto queue_done;
    case 4:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(31);
        sub_08004CD4(15);
        queueArg = 22;
    queue_done:
        sub_08004CD4(queueArg);
    phase_done:
        result = _0800AA20();
        break;
    }
    return result;
}

// ----------------------------------------------------------------------------
// 0x0800A44C _0800A44C — variant-A race selector.
//   0: sound off (engine cmd), ret 49
//   1: {13,14,42}
//   2: {13,14,42,15} + ev16    3: + ev19    4: + ev22
// ----------------------------------------------------------------------------
int _0800A44C(void)
{
    register int variant __asm__("r0") = (int)WA;
    register int variantOffset __asm__("r2") = 0x107C;
    register int result __asm__("r1");
    __asm__ volatile("" : "+r"(variant), "+r"(variantOffset));
    variant += variantOffset;
    variantOffset = 0;
    variant = *(s16 *)((u8 *)(uintptr_t)variant + variantOffset);

    switch (variant) {
    case 0:
        sub_0800B190();
        result = 49;
        break;
    case 1:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(42);
        result = _0800AA20();
        break;
    case 2:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(42);
        sub_08004CD4(15);
        _08023FF8(16, 0);
        result = _0800AA20();
        break;
    case 3:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(42);
        sub_08004CD4(15);
        _08023FF8(19, 0);
        result = _0800AA20();
        break;
    case 4:
        sub_08004CC4();
        sub_08004CD4(13);
        sub_08004CD4(14);
        sub_08004CD4(42);
        sub_08004CD4(15);
        _08023FF8(22, 0);
        result = _0800AA20();
        break;
    default:
        break;
    }
    return result;
}

// ----------------------------------------------------------------------------
// 0x0800A518 _0800A518 — variant-B race selector.
//   0: sound off, return 49; 1: {13,14,41,44,15}
//   2: {13,14,41,44,15} + ev17 if u16[WA+0x1078]==1 / ev16 if ==2
//   3: same queue + ev20; 4: same queue + ev22; else return caller r1.
//   The C switch preserves the ROM jump table; its final section pad is zero.
// ----------------------------------------------------------------------------
int _0800A518(void) {
    extern u8 CarTickWa[];
    register int result __asm__("r1");
    register int eventId __asm__("r0");
    u32 variant;
    s16 eventChoice;
    uintptr_t base;

    __asm__ volatile(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    __asm__ volatile("" : "=r" (result));
    base = (uintptr_t)CarTickWa;
    variant = (u32)*(const s16 *)(uintptr_t)(base + 0x107Cu);

    if (variant <= 4u) {
        switch (variant) {
        case 0:
            sub_0800B190();
            result = 49;
            break;
        case 1:
            sub_08004CC4();
            sub_08004CD4(13);
            sub_08004CD4(14);
            sub_08004CD4(41);
            sub_08004CD4(44);
            sub_08004CD4(15);
            result = _0800AA20();
            break;
        case 2:
            sub_08004CC4();
            sub_08004CD4(13);
            sub_08004CD4(14);
            sub_08004CD4(41);
            sub_08004CD4(44);
            sub_08004CD4(15);
            base = (uintptr_t)CarTickWa;
            eventChoice = *(const s16 *)(uintptr_t)(base + 0x1078u);
            if (eventChoice == 1)
                goto event17;
            if (eventChoice == 2)
                goto event16;
            goto tick;
        event17:
            eventId = 17;
            goto event_common;
        event16:
                _08023FF8(16, 0);
            goto tick;
        case 3:
            sub_08004CC4();
            sub_08004CD4(13);
            sub_08004CD4(14);
            sub_08004CD4(41);
            sub_08004CD4(44);
            sub_08004CD4(15);
            eventId = 20;
            goto event_common;
        case 4:
            sub_08004CC4();
            sub_08004CD4(13);
            sub_08004CD4(14);
            sub_08004CD4(41);
            sub_08004CD4(44);
            sub_08004CD4(15);
            eventId = 22;
        event_common:
            _08023FF8(eventId, 0);
        tick:
            result = _0800AA20();
            break;
        }
    }
    return result;
}
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// 0x0800A62C _0800A62C(ctx) — record 13 tick (boot-flow decision point).
//   key = [ctx+0x2C]: ==1 -> _08009B60, return 50;
//   >1 -> return 0; ==0 -> request latch u16[WA+0x1086]:
//   ==1 -> clear it, return 49; else return 14.
//   The ROM's dispatch is `cmp #1; beq key_one` then `cmp #1; bcs exit`
//   (0x0800A636/3C). That pair is the case tree of a switch over {0,1}
//   carrying an EMPTY `case 2`: the empty right leaf makes emit_case_nodes
//   branch to the shared exit on the >= form of the reused #1 compare.
//   Measured alternatives: an if-chain's `key > 1` folds to `cmp #1; bhi`
//   and `key >= 1` to `cmp #0; bhi`, and a plain {0,1} switch emits a
//   `cmp #0` the ROM does not have (build/scratch_a62c.c). The ROM passes
//   the key (now 1) rather than ctx to _08009B60; the callee only uses
//   sp/WA (asm/menu_pkt.s), so the value is immaterial.
// ----------------------------------------------------------------------------
int _0800A62C(void *ctx) {
    register int result __asm__("r1");
    register u32 key __asm__("r0");
    result = 0;
    key = *(const u32 *)((const u8 *)ctx + 0x2Cu);

    switch (key) {
    case 2:
        break;
    case 0: {
        extern u8 CarTickWa[];
        __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
        uintptr_t base = (uintptr_t)CarTickWa;
        volatile u16 *request = (volatile u16 *)(uintptr_t)(base + 0x1086u);

        if (*request == 1u) {
            result = 49;
            *request = 0;
        } else
            result = 14;
        break;
    }
    case 1:
        _08009B60((void *)(uintptr_t)key);
        result = 50;
        break;
    }
    return result;
}

// ----------------------------------------------------------------------------
// 0x0800A8FC _0800A8FC — racer-tick veneer (phase-loader path for rec 38).
// ----------------------------------------------------------------------------
int _0800A8FC(void)
{
    return _0800AA20();
}
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// 0x0800A908 _0800A908 — phase loader (record 37): when s16[WA+0x1084]
// ∈ {0,1}, reset the LIFO and enqueue record 13; return the popped id.
// ----------------------------------------------------------------------------
int _0800A908(void)
{
    extern u8 CarTickWa[];
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    uintptr_t base = (uintptr_t)CarTickWa;
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
    register const s16 *p = (const s16 *)ctx;
    register int zero __asm__("r2") = 0;
    register int val __asm__("r1");
    val = p[zero];
    switch (val) {
    case 38:
        return _0800A8FC();
    case 37:
        return _0800A908();
    case 47:
        return _0800A938();
    default:
        return sub_08004CF0();
    }
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
//
// A dense SWITCH, not an if/else chain: the ROM is a 9-entry jump table
// (`cmp r0,#8 / bhi exit / lsls r0,#2 / ldr r1,=<table> / adds / ldr r0,[r0] /
// mov pc,r0`) whose entries are the interior case labels. The if/else spelling
// measured 5/92; a switch with a `default:` arm measured 73/92 because the
// default arm costs an instruction the ROM does not have. The ROM's `bhi`
// jumps STRAIGHT to the shared exit, so the returned value is whatever the
// caller left in r4 -- which is only expressible as a switch with NO default
// over a never-assigned result local (`int r;`). That spelling measures 90/92
// at the ROM's exact 92 bytes and allocates `r` to r4 on its own, opening
// `push {r4,lr}` / `movs r4,#N` / `adds r0,r4,#0` exactly as the ROM has them;
// pinning the register (`register int r __asm__("r4")`) or reading r4 through
// STALE_REG first measure the same 90. STALE_REG is therefore not needed here.
// The last 2 of the 92 bytes are the ROM's own span pad, not code: the body is
// 90 bytes and the file-scope `.align 2, 0` below supplies the `00 00`.
// ----------------------------------------------------------------------------
int Rec9F5C(void *ctx)
{
    u32 key = (u32)(uintptr_t)_08004C0C();
    int r;

    switch (key) {
    case 0:  r = 31; break;
    case 1:
    case 2:
    case 3:
    case 4:  r = 15; break;
    case 5:  r = 41; break;
    case 6:  r = 42; break;
    case 7:  r = 36; break;
    case 8:  r = 23; break;
    }
    (void)ctx;
    return r;
}
int _08009F5C(void *c) __attribute__((alias("Rec9F5C")));
// The body is 90 bytes, so its `-ffunction-sections` section is padded to 92.
// gas closes a Thumb code section with the 2-byte `nop` filler (0x46c0), but the
// ROM pads with `00 00` (asm/carphys_tick.s spans 0x08009F5C-0x08009FB8, and the
// two bytes before 0x08009FB8 are zero). A file-scope `.align 2, 0` is emitted
// after the body's `.size`, i.e. still inside the body's own section, and pads
// with the explicit `0` fill.
__asm__(".align 2, 0");

// CONVERTED to C 2026-10-14. Two levers, both needed (the old PROBE 11/36 note
// below is kept for the record -- it predates both).
//
//  * the stale-register seam. The ROM's default arm never writes r4, so the
//    return is the CALLER's r4 (`adds r0, r4, #0`), and the ROM saves it
//    (`push {r4, lr}`). A LOCAL register variable pinned to r4 plus an empty
//    `__asm__ volatile("" : "=r" (r))` stake claim reads that incoming value
//    while keeping agbcc's own frame. The file-scope STALE_REG macro above is
//    the wrong lever here: it emits `adds r4, r4, #0` before the call and
//    loses the frame (measured 15/36). See
//    docs/findings/r4_input_reachability.md lever B.
//  * the branch tree is a THREE-case switch whose MIDDLE case is empty. That
//    is what emits the ROM's doubled compare against 1 (`cmp r0,#1 / beq exit`
//    then `cmp r0,#1 / bcc case0`): a binary case search whose pivot 1 is
//    itself a case. A two-case switch emits the flat chain `cmp #0 / cmp #2`
//    (13/36), and every `key < 1u` spelling folds to `cmp r0,#0`.
//
// PROBE 11/36 (superseded). The gap is the stale-register seam, not the branch
// tree: the ROM keeps the caller's r4 alive in a callee-saved register and so
// opens `push {r4, lr}`; STALE_REG (above) is the repo's documented stand-in
// and cannot produce that, so agbcc opens `push {lr}` and materialises the
// value in r1. The file's existing Rec9F5C, same idiom, scores 5/92. Four
// source shapes (preloaded r4, inverted key!=1 nest, explicit else-assign) all
// give 11/36, so the shape is not the lever.
// --------------------------------------------------------------------------
int Rec9FFC(void *ctx)
{
    register int r __asm__("r4");
    u32 key;
    __asm__ volatile("" : "=r" (r));
    key = (u32)(uintptr_t)_08004C0C();
    switch (key) {
    case 0:  r = 15; break;
    case 1:  break;
    case 2:  r = 40; break;
    }
    (void)ctx;
    return r;
}
int _08009FFC(void *c) __attribute__((alias("Rec9FFC")));
int sub_08009FFC(void *c) __attribute__((alias("Rec9FFC")));
// Body is 34 bytes; the ROM pads the span to 36 with `00 00`, not gas's 0x46c0
// section-close filler. Same file-scope idiom as Rec9F5C above.
__asm__(".align 2, 0");

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
    // to match it.
    sub_0800B190();
    return 49;
}
int _0800A1A4(void *c) __attribute__((alias("CarTick_Rec_0A1A4")));
int sub_0800A1A4(void *c) __attribute__((alias("CarTick_Rec_0A1A4")));

// ----------------------------------------------------------------------------
// 0x08009FB8 Rec9FB8(ctx) — record 23 ROUTER. key = s16[WA+0x1080]:
//   0 -> 24, 1 -> 34, 2 -> 37 (and mark grid [37][0] = 1), else -> stale r4.
// CONVERTED to C 2026-10-14 — same two levers as Rec9FFC below, plus two
// details this body needs (the old note called the compare chain unreachable;
// it is not):
//   * the stale-r4 tail is the local-pinned-to-r4 + empty-asm stake claim
//     (docs/findings/r4_input_reachability.md lever B).
//   * the compare chain (`cmp #1 / beq`, `cmp #1 / bgt`, `cmp #0 / beq`) is a
//     THREE-case signed switch (pivot 1) — agbcc's binary case search,
//     unchanged by the claim lever. An if-chain scores 15/68.
//   * the key read must be NON-volatile to reach the single indexed `ldrsh`,
//     and the WA base must go through a VARIABLE (`base = (uintptr_t)WA`), as
//     the comment on MgrBlock records: a symbol plus a constant offset folds
//     into the pool word (measured 8/68), a variable does not.
//   * the literal word sits mid-function at entry+0x1C, as in the ROM, and the
//     span's `00 00` pad is the file-scope `.align 2, 0` idiom.
// ----------------------------------------------------------------------------
int Rec9FB8(void *ctx)
{
    extern u8 CarTickWa[];
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    register int r __asm__("r4");
    s32 key;
    uintptr_t base;
    __asm__ volatile("" : "=r" (r));
    base = (uintptr_t)CarTickWa;
    key = *(const s16 *)(uintptr_t)(base + 0x1080u);
    switch (key) {
    case 0:  r = 24; break;
    case 1:  r = 34; break;
    case 2:
        r = 37;
        _08004C48(37, 0, 1);
        break;
    }
    (void)ctx;
    return r;
}
int _08009FB8(void *c) __attribute__((alias("Rec9FB8")));
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// 0x0800A028 CarTick_Rec_0A028(ctx) — record 31 ROUTER.
//   cell = s16[WA + 0x0FD0 + (s16[WA+0xFF6] << 1) + (s16[WA+0xFF2] << 3)];
//   cell == 0 -> store 1 there and return 32; else 15. (No stale tail.)
// ----------------------------------------------------------------------------
int CarTick_Rec_0A028(void *ctx)
{
    extern u8 CarTickWa[];
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    uintptr_t base = (uintptr_t)CarTickWa;
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
int _0800A028(void *c) __attribute__((alias("CarTick_Rec_0A028")));

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
    extern u8 CarTickWa[];
    register int result __asm__("r2");
    u32 key;
    u32 phase;
    uintptr_t base;

    __asm__ volatile(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    __asm__ volatile("" : "=r" (result));
    key = *(const u32 *)((const u8 *)ctx + 0x2Cu);

    switch (key) {
    case 0:
        base = (uintptr_t)CarTickWa;
        phase = (u32)*(const s16 *)(uintptr_t)(base + 0x0FBCu);
        switch (phase) {
        case 0:
            result = 33;
            break;
        case 1:
        case 2:
        case 3:
            result = 16;
            break;
        case 5:
            goto phase17;
        case 6:
            sub_0800B190();
            result = 49;
            break;
        case 7:
            base = (uintptr_t)CarTickWa;
            {
                register int selectorValue __asm__("r0");
                selectorValue = (int)(base + 0x1078u);
                selectorValue = *(const s16 *)(uintptr_t)selectorValue;
                __asm__ volatile ("" : "+r" (result), "+r" (selectorValue));
                if (selectorValue == 1)
                    goto phase17;
            }
            result = 16;
            break;
        phase17:
            result = 17;
            break;
        }
        break;
    case 1: {
        register u32 phaseBase __asm__("r0") = (u32)CarTickWa;
        register u32 phaseOffset __asm__("r1") = 0x0FBCu;

        __asm__ volatile ("" : "+r" (phaseBase), "+r" (phaseOffset));
        phaseBase += phaseOffset;

        result = 19;
        phase = *(const volatile u16 *)(uintptr_t)phaseBase;
        if (phase == 7)
            result = 20;
        break;
    }
    case 2:
        result = 22;
        break;
    case 3:
        result = 36;
        break;
    }
    return result;
}
int _0800A074(void *c) __attribute__((alias("CarTick_Rec_0A074")));

// ----------------------------------------------------------------------------
// 0x0800A124 CarTick_Rec_0A124(ctx) — record 20: drains the manager LIFO once
// when the race phase is 7, twice otherwise, always routing to 15.
// ----------------------------------------------------------------------------
int CarTick_Rec_0A124(void *ctx)
{
    extern u8 CarTickWa[];
    __asm__(".globl CarTickWa\nCarTickWa = 0x03001780\n");
    uintptr_t base = (uintptr_t)CarTickWa;
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
int _0800A124(void *c) __attribute__((alias("CarTick_Rec_0A124")));

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
    register int result __asm__("r1") = 0;
    __asm__ volatile("" : "+r"(result));
    register u32 phaseBase __asm__("r0") = WA;
    register u32 phaseOffset __asm__("r2") = 0x0FBCu;
    __asm__ volatile("" : "+r"(phaseBase), "+r"(phaseOffset));
    register int phase __asm__("r0");
    phaseBase += phaseOffset;
    phaseOffset = 0;
    phase = *(const s16 *)((const u8 *)(uintptr_t)phaseBase + phaseOffset);

    if (phase == 2) goto phase2;
    if (phase > 2) goto phase_above_2;
    if (phase == 1) goto dispatch_once;
    goto finish;

phase_above_2:
    if (phase == 3) goto phase3;
    if (phase == 7) goto phase7;
    goto finish;

phase2:
    {
        register int arg __asm__("r0");
        __asm__ volatile("" : "=r"(arg));
        CarTickDispatch_R0(arg);
    }
phase3:
    {
        register int arg __asm__("r0");
        __asm__ volatile("" : "=r"(arg));
        CarTickDispatch_R0(arg);
    }
phase7:
    {
        register u32 base __asm__("r0") = WA;
        register u32 offset __asm__("r1") = 0x1078u;
        __asm__ volatile("" : "+r"(base), "+r"(offset));
        register int arg __asm__("r0");
        base += offset;
        arg = *(const s16 *)(uintptr_t)base;
        __asm__ volatile("" : "+r"(offset), "+r"(arg));
    }
dispatch_once:
    {
        register int arg __asm__("r0");
        __asm__ volatile("" : "=r"(arg));
        CarTickDispatch_R0(arg);
    }
    result = 49;
finish:
    (void)ctx;
    return result;
}
int _0800A150(void *c) __attribute__((alias("CarTick_Rec_0A150")));

// ----------------------------------------------------------------------------
// 0x0800A1C8 CarTick_Rec_0A1C8(ctx) — record 21 (menu scene): plain racer-style
// tick pump, `push {lr}; bl _0800AA20; bx`.
// ----------------------------------------------------------------------------
int CarTick_Rec_0A1C8(void *ctx)
{
    (void)ctx;
    return _0800AA20();
}
__asm__(".align 2, 0");
int _0800A1C8(void *c) __attribute__((alias("CarTick_Rec_0A1C8")));

// 0x0800D778 — 2B `bx lr` no-op (car_tick_d778.s tail slot; the router's
// `case 10` calls it as a phase placeholder that does nothing).
void CarTickNoop_D778(void) {}
__asm__(".align 2, 0");
void _0800D778(void) __attribute__((alias("CarTickNoop_D778")));
void sub_0800D778(void) __attribute__((alias("CarTickNoop_D778")));
