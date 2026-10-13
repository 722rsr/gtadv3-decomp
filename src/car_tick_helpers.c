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
__attribute__((naked)) int _08009BF8(void *ctx) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "sub sp, #64\n"
        "movs r4, #0\n"
        "movs r2, #0\n"
        "ldr r0, [r0, #44]\n"
        "cmp r0, #101\n"
        "bls .Lct09bf8_08009c08\n"
        "b .Lct09bf8_08009f4c\n"
        ".Lct09bf8_08009c08:\n"
        "lsls r0, r0, #2\n"
        "ldr r1, .Lct09bf8_r0_lit\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "mov pc, r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lct09bf8_r0_lit:\n"
        ".word .Lct09bf8_rec0_table\n"
        ".align 2, 0\n"
        ".Lct09bf8_rec0_table:\n"
        ".word .Lct09bf8_08009db0\n"
        ".word .Lct09bf8_08009ecc\n"
        ".word .Lct09bf8_08009ed0\n"
        ".word .Lct09bf8_08009ed4\n"
        ".word .Lct09bf8_08009ed8\n"
        ".word .Lct09bf8_08009edc\n"
        ".word .Lct09bf8_08009ee6\n"
        ".word .Lct09bf8_08009ee4\n"
        ".word .Lct09bf8_08009ee2\n"
        ".word .Lct09bf8_08009ee0\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f4c\n"
        ".word .Lct09bf8_08009f3c\n"
        ".word .Lct09bf8_08009f40\n"
        ".Lct09bf8_08009db0:\n"
        "ldr r4, .Lct09bf8_ra_lit1\n"
        "ldr r0, .Lct09bf8_ra_lit2\n"
        "adds r5, r4, r0\n"
        "movs r2, #0\n"
        "ldrsh r1, [r5, r2]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "adds r0, #48\n"
        "movs r2, #0\n"
        "strb r2, [r0, #0]\n"
        "movs r3, #0\n"
        "ldrsh r1, [r5, r3]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "ldr r3, .Lct09bf8_ra_lit3\n"
        "adds r1, r4, r3\n"
        "ldrb r1, [r1, #0]\n"
        "adds r0, #49\n"
        "strb r1, [r0, #0]\n"
        "movs r0, #0\n"
        "ldrsh r1, [r5, r0]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "adds r0, #50\n"
        "strb r2, [r0, #0]\n"
        "movs r3, #0\n"
        "ldrsh r1, [r5, r3]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "adds r0, #51\n"
        "strb r2, [r0, #0]\n"
        "movs r0, #0\n"
        "ldrsh r1, [r5, r0]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "adds r0, #52\n"
        "strb r2, [r0, #0]\n"
        "movs r3, #0\n"
        "ldrsh r1, [r5, r3]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "adds r0, #53\n"
        "strb r2, [r0, #0]\n"
        "movs r0, #0\n"
        "ldrsh r1, [r5, r0]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "adds r0, #54\n"
        "strb r2, [r0, #0]\n"
        "movs r3, #0\n"
        "ldrsh r1, [r5, r3]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "adds r0, #55\n"
        "strb r2, [r0, #0]\n"
        "movs r0, #0\n"
        "ldrsh r1, [r5, r0]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "adds r0, #56\n"
        "strb r2, [r0, #0]\n"
        "movs r3, #0\n"
        "ldrsh r1, [r5, r3]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "adds r0, #57\n"
        "strb r2, [r0, #0]\n"
        "str r2, [sp, #56]\n"
        "add r0, sp, #56\n"
        "ldr r2, .Lct09bf8_ra_lit4\n"
        "mov r1, sp\n"
        "bl sub_0802D974\n"
        "mov r1, sp\n"
        "ldr r0, .Lct09bf8_ra_lit5\n"
        "strh r0, [r1, #0]\n"
        "ldr r2, .Lct09bf8_ra_lit6\n"
        "adds r0, r4, r2\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r1, #2]\n"
        "ldr r3, .Lct09bf8_ra_lit7\n"
        "adds r0, r4, r3\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r1, #10]\n"
        "ldr r2, .Lct09bf8_ra_lit8\n"
        "adds r0, r4, r2\n"
        "ldrh r0, [r0, #0]\n"
        "strh r0, [r1, #8]\n"
        "movs r3, #0\n"
        "ldrsh r0, [r5, r3]\n"
        "bl 0x08002340\n"
        "mov r1, sp\n"
        "strh r0, [r1, #24]\n"
        "movs r0, #0\n"
        "ldrsh r1, [r5, r0]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r4\n"
        "add r1, sp, #28\n"
        "adds r0, #48\n"
        "ldmia r0!, {r2, r3, r4}\n"
        "stmia r1!, {r2, r3, r4}\n"
        "b .Lct09bf8_08009f24\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lct09bf8_ra_lit1:\n"
        ".word 0x03001780\n"
        ".Lct09bf8_ra_lit2:\n"
        ".word 0x00000574\n"
        ".Lct09bf8_ra_lit3:\n"
        ".word 0x00001151\n"
        ".Lct09bf8_ra_lit4:\n"
        ".word 0x0500000E\n"
        ".Lct09bf8_ra_lit5:\n"
        ".word 0x0000FFFF\n"
        ".Lct09bf8_ra_lit6:\n"
        ".word 0x00000576\n"
        ".Lct09bf8_ra_lit7:\n"
        ".word 0x0000113E\n"
        ".Lct09bf8_ra_lit8:\n"
        ".word 0x0000113C\n"
        ".Lct09bf8_08009ecc:\n"
        "movs r2, #1\n"
        "b .Lct09bf8_08009f4c\n"
        ".Lct09bf8_08009ed0:\n"
        "movs r2, #14\n"
        "b .Lct09bf8_08009f4c\n"
        ".Lct09bf8_08009ed4:\n"
        "movs r2, #4\n"
        "b .Lct09bf8_08009f4c\n"
        ".Lct09bf8_08009ed8:\n"
        "movs r2, #5\n"
        "b .Lct09bf8_08009f4c\n"
        ".Lct09bf8_08009edc:\n"
        "movs r2, #8\n"
        "b .Lct09bf8_08009f4c\n"
        ".Lct09bf8_08009ee0:\n"
        "adds r4, #1\n"
        ".Lct09bf8_08009ee2:\n"
        "adds r4, #1\n"
        ".Lct09bf8_08009ee4:\n"
        "adds r4, #1\n"
        ".Lct09bf8_08009ee6:\n"
        "movs r0, #0\n"
        "str r0, [sp, #60]\n"
        "add r0, sp, #60\n"
        "ldr r2, .Lct09bf8_rb_lit1\n"
        "mov r1, sp\n"
        "bl sub_0802D974\n"
        "mov r1, sp\n"
        "movs r0, #10\n"
        "strh r0, [r1, #0]\n"
        "mov r0, sp\n"
        "strb r4, [r0, #23]\n"
        "ldr r5, .Lct09bf8_rb_lit2\n"
        "ldr r0, .Lct09bf8_rb_lit3\n"
        "adds r4, r5, r0\n"
        "movs r1, #0\n"
        "ldrsh r0, [r4, r1]\n"
        "bl 0x08002340\n"
        "mov r1, sp\n"
        "strh r0, [r1, #24]\n"
        "movs r2, #0\n"
        "ldrsh r1, [r4, r2]\n"
        "lsls r0, r1, #1\n"
        "adds r0, r0, r1\n"
        "lsls r0, r0, #2\n"
        "adds r0, r0, r5\n"
        "add r1, sp, #28\n"
        "adds r0, #48\n"
        "ldmia r0!, {r2, r3, r4}\n"
        "stmia r1!, {r2, r3, r4}\n"
        ".Lct09bf8_08009f24:\n"
        "mov r0, sp\n"
        "bl 0x080188B0\n"
        "movs r2, #51\n"
        "b .Lct09bf8_08009f4c\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lct09bf8_rb_lit1:\n"
        ".word 0x0500000E\n"
        ".Lct09bf8_rb_lit2:\n"
        ".word 0x03001780\n"
        ".Lct09bf8_rb_lit3:\n"
        ".word 0x00000574\n"
        ".Lct09bf8_08009f3c:\n"
        "movs r2, #6\n"
        "b .Lct09bf8_08009f4c\n"
        ".Lct09bf8_08009f40:\n"
        "movs r0, #1\n"
        "bl 0x08001F80\n"
        "bl 0x0800D778\n"
        "movs r2, #11\n"
        ".Lct09bf8_08009f4c:\n"
        "adds r0, r2, #0\n"
        "add sp, #64\n"
        "pop {r4, r5}\n"
        "pop {r1}\n"
        "bx r1\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
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
//   4 -> {...,22}; else stale (r1 remains live from the caller).
// ----------------------------------------------------------------------------
__attribute__((naked)) int _0800A254(void)
{
    __asm__ volatile (
        ".syntax unified\n"
        "push {lr}\n"
        "ldr r0, .L254_base\n"
        "ldr r2, .L254_off\n"
        "adds r0, r0, r2\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "cmp r0, #4\n"
        "bhi .L254_end\n"
        "lsls r0, r0, #2\n"
        "ldr r1, .L254_lit\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "mov pc, r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L254_base: .word 0x03001780\n"
        ".L254_off: .word 0x0000107C\n"
        ".L254_lit: .word 0x0800A27C\n"
        ".word 0x0800A290\n"
        ".word 0x0800A298\n"
        ".word 0x0800A2AC\n"
        ".word 0x0800A302\n"
        ".word 0x0800A31C\n"
        ".L254_v0:\n"
        "bl sub_0800B190\n"
        "movs r1, #49\n"
        "b .L254_end\n"
        ".L254_v1:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "b .L254_queue_done\n"
        ".L254_v2:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "ldr r0, .L254_phase_base\n"
        "ldr r1, .L254_phase_off\n"
        "adds r0, r0, r1\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "cmp r0, #1\n"
        "beq .L254_phase1\n"
        "cmp r0, #1\n"
        "bgt .L254_phase_gt1\n"
        "cmp r0, #0\n"
        "beq .L254_phase0\n"
        "b .L254_phase_done\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L254_phase_base: .word 0x03001780\n"
        ".L254_phase_off: .word 0x00000FBC\n"
        ".L254_phase_gt1:\n"
        "cmp r0, #2\n"
        "beq .L254_phase2\n"
        "cmp r0, #5\n"
        "beq .L254_phase5\n"
        "b .L254_phase_done\n"
        ".L254_phase0:\n"
        "movs r0, #18\n"
        "b .L254_queue_done\n"
        ".L254_phase5:\n"
        "movs r0, #17\n"
        "b .L254_queue_done\n"
        ".L254_phase1:\n"
        "movs r0, #16\n"
        "b .L254_queue_done\n"
        ".L254_phase2:\n"
        "movs r0, #16\n"
        "bl sub_08004CD4\n"
        "b .L254_phase_done\n"
        ".L254_v3:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "movs r0, #19\n"
        "b .L254_queue_done\n"
        ".L254_v4:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "movs r0, #22\n"
        ".L254_queue_done:\n"
        "bl sub_08004CD4\n"
        ".L254_phase_done:\n"
        "bl _0800AA20\n"
        "adds r1, r0, #0\n"
        ".L254_end:\n"
        "adds r0, r1, #0\n"
        "pop {r1}\n"
        "bx r1\n"
        ".syntax divided\n"
    );
}

// ----------------------------------------------------------------------------
// 0x0800A344 _0800A344 — record-35 PHASE-0 handler (programs queue car id
// 31 instead of command 42).
// ----------------------------------------------------------------------------
__attribute__((naked)) int _0800A344(void)
{
    __asm__ volatile (
        ".syntax unified\n"
        "push {lr}\n"
        "ldr r0, .L344_base\n"
        "ldr r2, .L344_off\n"
        "adds r0, r0, r2\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "cmp r0, #4\n"
        "bhi .L344_end\n"
        "lsls r0, r0, #2\n"
        "ldr r1, .L344_lit\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "mov pc, r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L344_base: .word 0x03001780\n"
        ".L344_off: .word 0x0000107C\n"
        ".L344_lit: .word 0x0800A36C\n"
        ".word 0x0800A380\n"
        ".word 0x0800A388\n"
        ".word 0x0800A3A2\n"
        ".word 0x0800A3FE\n"
        ".word 0x0800A41E\n"
        ".L344_v0:\n"
        "bl sub_0800B190\n"
        "movs r1, #49\n"
        "b .L344_end\n"
        ".L344_v1:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #31\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "b .L344_queue_done\n"
        ".L344_v2:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #31\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "ldr r0, .L344_phase_base\n"
        "ldr r1, .L344_phase_off\n"
        "adds r0, r0, r1\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "cmp r0, #1\n"
        "beq .L344_phase1\n"
        "cmp r0, #1\n"
        "bgt .L344_phase_gt1\n"
        "cmp r0, #0\n"
        "beq .L344_phase0\n"
        "b .L344_phase_done\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L344_phase_base: .word 0x03001780\n"
        ".L344_phase_off: .word 0x00000FBC\n"
        ".L344_phase_gt1:\n"
        "cmp r0, #2\n"
        "beq .L344_phase2\n"
        "cmp r0, #5\n"
        "beq .L344_phase5\n"
        "b .L344_phase_done\n"
        ".L344_phase0:\n"
        "movs r0, #18\n"
        "b .L344_queue_done\n"
        ".L344_phase5:\n"
        "movs r0, #17\n"
        "b .L344_queue_done\n"
        ".L344_phase1:\n"
        "movs r0, #16\n"
        "b .L344_queue_done\n"
        ".L344_phase2:\n"
        "movs r0, #16\n"
        "bl sub_08004CD4\n"
        "b .L344_phase_done\n"
        ".L344_v3:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #31\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "movs r0, #19\n"
        "b .L344_queue_done\n"
        ".L344_v4:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #31\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "movs r0, #22\n"
        ".L344_queue_done:\n"
        "bl sub_08004CD4\n"
        ".L344_phase_done:\n"
        "bl _0800AA20\n"
        "adds r1, r0, #0\n"
        ".L344_end:\n"
        "adds r0, r1, #0\n"
        "pop {r1}\n"
        "bx r1\n"
        ".syntax divided\n"
    );
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
    __asm__ volatile(".code 16\n\tldrsh r0, [r0, r2]"
                     : "+r"(variant) : "r"(variantOffset) : "cc");

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
//   1: {13,14,41,44,15}
//   2: {13,14,41,44,15} + ev17 if u16[WA+0x1078]==1 / ev16 if ==2
//   3: + ev20    4: + ev22
// ----------------------------------------------------------------------------
__attribute__((naked)) int _0800A518(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {lr}\n"
        "ldr r0, .Lct0a518_0800a534\n"
        "ldr r2, .Lct0a518_0800a538\n"
        "adds r0, r0, r2\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "cmp r0, #4\n"
        "bls .Lct0a518_0800a52a\n"
        "b .Lct0a518_0800a624\n"
        ".Lct0a518_0800a52a:\n"
        "lsls r0, r0, #2\n"
        "ldr r1, .Lct0a518_0800a53c\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0]\n"
        "mov pc, r0\n"
        ".align 2, 0\n"
        ".Lct0a518_0800a534: .word 0x03001780\n"
        ".Lct0a518_0800a538: .word 0x0000107C\n"
        ".Lct0a518_0800a53c: .word .Lct0a518_0800a540\n"
        ".Lct0a518_0800a540:\n"
        ".word .Lct0a518_0800a554\n"
        ".word .Lct0a518_0800a55c\n"
        ".word .Lct0a518_0800a580\n"
        ".word .Lct0a518_0800a5ce\n"
        ".word .Lct0a518_0800a5f4\n"
        ".Lct0a518_0800a554:\n"
        "bl sub_0800B190\n"
        "movs r1, #49\n"
        "b .Lct0a518_0800a624\n"
        ".Lct0a518_0800a55c:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #41\n"
        "bl sub_08004CD4\n"
        "movs r0, #44\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "b .Lct0a518_0800a61e\n"
        ".Lct0a518_0800a580:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #41\n"
        "bl sub_08004CD4\n"
        "movs r0, #44\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "ldr r0, .Lct0a518_0800a5b8\n"
        "ldr r1, .Lct0a518_0800a5bc\n"
        "adds r0, r0, r1\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "cmp r0, #1\n"
        "beq .Lct0a518_0800a5c0\n"
        "cmp r0, #2\n"
        "beq .Lct0a518_0800a5c4\n"
        "b .Lct0a518_0800a61e\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lct0a518_0800a5b8: .word 0x03001780\n"
        ".Lct0a518_0800a5bc: .word 0x00001078\n"
        ".Lct0a518_0800a5c0:\n"
        "movs r0, #17\n"
        "b .Lct0a518_0800a618\n"
        ".Lct0a518_0800a5c4:\n"
        "movs r0, #16\n"
        "movs r1, #0\n"
        "bl _08023FF8\n"
        "b .Lct0a518_0800a61e\n"
        ".Lct0a518_0800a5ce:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #41\n"
        "bl sub_08004CD4\n"
        "movs r0, #44\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "movs r0, #20\n"
        "b .Lct0a518_0800a618\n"
        ".Lct0a518_0800a5f4:\n"
        "bl sub_08004CC4\n"
        "movs r0, #13\n"
        "bl sub_08004CD4\n"
        "movs r0, #14\n"
        "bl sub_08004CD4\n"
        "movs r0, #41\n"
        "bl sub_08004CD4\n"
        "movs r0, #44\n"
        "bl sub_08004CD4\n"
        "movs r0, #15\n"
        "bl sub_08004CD4\n"
        "movs r0, #22\n"
        ".Lct0a518_0800a618:\n"
        "movs r1, #0\n"
        "bl _08023FF8\n"
        ".Lct0a518_0800a61e:\n"
        "bl _0800AA20\n"
        "adds r1, r0, #0\n"
        ".Lct0a518_0800a624:\n"
        "adds r0, r1, #0\n"
        "pop {r1}\n"
        "bx r1\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
}

// ----------------------------------------------------------------------------
// 0x0800A62C _0800A62C(ctx) — record 13 tick (boot-flow decision point).
//   key = [ctx+0x2C]: ==1 -> _08009B60, return 50;
//   >1 -> return 0; ==0 -> request latch u16[WA+0x1086]:
//   ==1 -> clear it, return 49; else return 14.
//   Naked transcription: the second head check (cmp #1/bcs) has no C
//   spelling — agbcc folds `key < 1`, `key >= 1`, `key > 0` and `key != 0`
//   alike to cmp #0 (measured t_* battery against old_agbcc; same fold as
//   StateA_IsGameMode in src/state_block_a.c). The ROM passes the key
//   (r0, already holding it — the ctx load clobbers r0) rather than ctx;
//   the callee only uses sp/WA (asm/menu_pkt.s), so the value is
//   immaterial. The host fallback below keeps that shape in C.
// ----------------------------------------------------------------------------
__attribute__((naked)) int _0800A62C(void *ctx) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {lr}\n"
        "movs r1, #0\n"
        "ldr r0, [r0, #44]\n"
        "cmp r0, #1\n"
        "beq 5f\n"
        "cmp r0, #1\n"
        "bcs 6f\n"
        "ldr r0, 3f\n"
        "ldr r1, 4f\n"
        "adds r2, r0, r1\n"
        "ldrh r0, [r2, #0]\n"
        "cmp r0, #1\n"
        "bne 2f\n"
        "movs r1, #49\n"
        "movs r0, #0\n"
        "strh r0, [r2, #0]\n"
        "b 6f\n"
        ".align 2, 0\n"
        "3: .word 0x03001780\n"
        "4: .word 0x00001086\n"
        "2: movs r1, #14\n"
        "b 6f\n"
        "5: bl _08009B60\n"
        "movs r1, #50\n"
        "6: adds r0, r1, #0\n"
        "pop {r1}\n"
        "bx r1\n"
        ".syntax divided\n"
    );
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

// PROBE 11/36. The gap is the stale-register seam, not the branch tree: the
// ROM keeps the caller's r4 alive in a callee-saved register and so opens
// `push {r4, lr}`; STALE_REG (above) is the repo's documented stand-in and
// cannot produce that, so agbcc opens `push {lr}` and materialises the value
// in r1. The file's existing Rec9F5C, same idiom, scores 5/92. Four source
// shapes (preloaded r4, inverted key!=1 nest, explicit else-assign) all give
// 11/36, so the shape is not the lever.
// --------------------------------------------------------------------------
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
int _08009FFC(void *c) __attribute__((alias("Rec9FFC")));
int sub_08009FFC(void *c) __attribute__((alias("Rec9FFC")));

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
//   Naked transcription (same seam as the sibling Rec9FFC just below): the
//   stale-r4 tail keeps the caller's r4 alive with zero instructions, and
//   the compare chain (cmp #1/beq, cmp #1/bgt, cmp #0/beq) has no C
//   spelling — agbcc folds every range-check form and zeroes the stale
//   value. The literal word sits mid-function at entry+0x1C, as in the ROM.
// ----------------------------------------------------------------------------
__attribute__((naked)) int Rec9FB8(void *ctx) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "ldr r0, 5f\n"
        "movs r1, #132\n"
        "lsls r1, r1, #5\n"
        "adds r0, r0, r1\n"
        "movs r1, #0\n"
        "ldrsh r0, [r0, r1]\n"
        "cmp r0, #1\n"
        "beq 2f\n"
        "cmp r0, #1\n"
        "bgt 4f\n"
        "cmp r0, #0\n"
        "beq 3f\n"
        "b 1f\n"
        ".align 2, 0\n"
        "5: .word 0x03001780\n"
        "4: cmp r0, #2\n"
        "beq 6f\n"
        "b 1f\n"
        "3: movs r4, #24\n"
        "b 1f\n"
        "2: movs r4, #34\n"
        "b 1f\n"
        "6: movs r4, #37\n"
        "movs r0, #37\n"
        "movs r1, #0\n"
        "movs r2, #1\n"
        "bl _08004C48\n"
        "1: adds r0, r4, #0\n"
        "pop {r4}\n"
        "pop {r1}\n"
        "bx r1\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
}
int _08009FB8(void *c) __attribute__((alias("Rec9FB8")));

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
__attribute__((naked)) int CarTick_Rec_0A074(void *ctx)
{
    __asm__ volatile (
        ".syntax unified\n"
        "push {lr}\n"
        "ldr r0, [r0, #44]\n"
        "cmp r0, #1\n"
        "beq 9f\n"
        "cmp r0, #1\n"
        "bcc 1f\n"
        "cmp r0, #2\n"
        "beq 10f\n"
        "cmp r0, #3\n"
        "beq 11f\n"
        "b 12f\n"
        "1:\n"
        "ldr r1, 2f\n"
        "ldr r3, 3f\n"
        "adds r0, r1, r3\n"
        "movs r3, #0\n"
        "ldrsh r0, [r0, r3]\n"
        "adds r3, r1, #0\n"
        "cmp r0, #7\n"
        "bhi 12f\n"
        "lsls r0, r0, #2\n"
        "ldr r1, 4f\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "mov pc, r0\n"
        ".align 2, 0\n"
        "2: .word 0x03001780\n"
        "3: .word 0x00000FBC\n"
        "4: .word 0x0800A0B0\n"
        ".word 0x0800A0D0\n"
        ".word 0x0800A0E8\n"
        ".word 0x0800A0E8\n"
        ".word 0x0800A0E8\n"
        ".word 0x0800A116\n"
        ".word 0x0800A0F0\n"
        ".word 0x0800A0D4\n"
        ".word 0x0800A0DC\n"
        "13:\n"
        "movs r2, #33\n"
        "b 12f\n"
        "14:\n"
        "bl _0800B190\n"
        "movs r2, #49\n"
        "b 12f\n"
        "15:\n"
        "ldr r1, 5f\n"
        "adds r0, r3, r1\n"
        "movs r3, #0\n"
        "ldrsh r0, [r0, r3]\n"
        "cmp r0, #1\n"
        "beq 6f\n"
        "16:\n"
        "movs r2, #16\n"
        "b 12f\n"
        ".align 2, 0\n"
        "5: .word 0x00001078\n"
        "6:\n"
        "movs r2, #17\n"
        "b 12f\n"
        "9:\n"
        "ldr r0, 7f\n"
        "ldr r1, 8f\n"
        "adds r0, r0, r1\n"
        "movs r2, #19\n"
        "ldrh r0, [r0, #0]\n"
        "cmp r0, #7\n"
        "bne 12f\n"
        "movs r2, #20\n"
        "b 12f\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        "7: .word 0x03001780\n"
        "8: .word 0x00000FBC\n"
        "10:\n"
        "movs r2, #22\n"
        "b 12f\n"
        "11:\n"
        "movs r2, #36\n"
        "12:\n"
        "adds r0, r2, #0\n"
        "pop {r1}\n"
        "bx r1\n"
        ".syntax divided\n"
    );
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
    __asm__ volatile(".code 16\n\t"
                     "add r0, r0, r2\n\t"
                     "movs r2, #0\n\t"
                     "ldrsh r0, [r0, r2]"
                     : "=r"(phase), "+r"(phaseOffset)
                     : "0"(phaseBase) : "cc", "memory");

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
        __asm__ volatile(".code 16\n\t"
                         "add r0, r0, r1\n\t"
                         "movs r2, #0\n\t"
                         "ldrsh r0, [r0, r2]"
                         : "=r"(arg), "+r"(offset)
                         : "0"(base) : "cc", "memory");
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
