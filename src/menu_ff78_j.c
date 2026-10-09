// ============================================================================
// menu_ff78_j.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_0800122E0 (0x0800122E0) — (rec, u16): selector-cell + 7BFC pair.
//   sub_0800123D8 (0x0800123D8) — builder with 122E0/7C68 branch + tail.
//
// NOTE on 122E0: it reads u32[0x080CB5C8] as a gate and stores 1 there.
// 0x080CB5C8 is a ROM word (value 0), so on hardware the store is ignored
// by the bus and the gate reads constant 0. The lift replicates the exact
// accesses (volatile), which behave identically under the emulator.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) void Sub_08007C68(void *a, u32 b, u32 c, int d, u32 e, u32 f, u32 g, u32 h, u32 i) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) void Sub_08007ABC(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_08002E0A4(void *d, const void *s, u32 n) { (void)d; (void)s; (void)n; }
__attribute__((weak)) int Sub_0802581C(int v) { return v; }
__attribute__((weak)) void Sub_080011E10(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011CFC(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011B48(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011E58(void *a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800122E0(void *a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800F778(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800DBE8(void *a) { (void)a; }
__attribute__((weak)) void Sub_080012574(volatile void *a) { (void)a; }
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void Sub_08007C68(void *a, u32 b, u32 c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void _08007C68(void *a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i); // faithful strong body
extern void Sub_08007ABC(u32 a, u32 b, u32 c);
extern void Sub_08002E0A4(void *d, const void *s, u32 n);
// Closure spellings for the three callees _0800122E0 calls. The friendly
// `Sub_` names above are host-only weak stubs, so on the ROM build a call to
// one of them binds to nothing and the screen blocks the body with
// "closure defines _0802E0A4/sub_0802E0A4 at 0x0802e0a4 (rename)".
// The digit count is load-bearing: arm-none-eabi-nm reports `sub_0802E0A4`
// (7 hex digits). Spelling it `sub_08002E0A4` normalises to the same address
// and still fails, because the screen compares names, not addresses. Declaring
// the friendly name alone is what makes J_CALLEE resolve to C on the ROM build
// -- the same reason FF_CALLEE does in menu_ff78_f.c:80. Spelled per file so
// each translation unit stays self-contained.
extern void sub_0802E0A4(void *d, const void *s, u32 n);  // 7 digits, per nm
extern void sub_08007ABC(u32 a, u32 b, u32 c);
extern void sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern int Sub_0802581C(int v);
extern void Sub_080011E10(void *a);
extern void Sub_080011784(void *a);
extern void Sub_080011CFC(void *a);
extern void Sub_080011B48(void *a);
extern void Sub_080011E58(void *a, u32 b);
extern void Sub_0800122E0(void *a, u32 b);
extern void Sub_0800F778(u32 a, u32 b, u32 c);
extern void Sub_0800D97C(void *a, int b);
extern void Sub_0800DBE8(void *a);
extern void Sub_080012574(volatile void *a);
// Closure spellings for the _0800123D8 callees. On the ROM build each call must
// name a label the closure actually defines (see J_CALLEE); the friendly `Sub_`
// spellings above are host-only weak stubs and the promotion screen rejects them
// with "closure defines ... (rename)".
extern void sub_080011784(void *a);
extern void sub_080011E10(void *a);
extern void sub_080011CFC(void *a);
extern void sub_080011B48(void *a);
extern void sub_080011E58(void *a, u32 b);
extern int sub_0802581C(int v);
extern void sub_0800F778(u32 a, u32 b, u32 c);
extern void sub_0800D97C(void *a, int b);
extern void sub_0800DBE8(void *a);
extern void sub_080012574(volatile void *a);
#endif

// Same split as FF_CALLEE (menu_ff78_f.c) and D1_CALLEE (race_scene_d1.c):
// friendly name on the host build, closure spelling on the ROM build.
//
// This macro MUST sit at file scope, below the __APPLE__ split above, not
// inside its `#else`. Defined inside the ROM branch it is undefined on the
// host build, and tools/apple_decls.py reports that as a "call to undeclared
// 'J_CALLEE' under __APPLE__" -- the check exists for exactly this.
#ifndef __APPLE__
#define J_CALLEE(friendly, closure) closure
#else
#define J_CALLEE(friendly, closure) friendly
#endif

// ----------------------------------------------------------------------------
// sub_0800122E0 — (rec, u32 a1):
//   memcpy(sp+20, 0x0805F9A0, 4); memcpy(sp+24, 0x0805F9A4, 4).
//   if a1 != u32[0x03000588] or u32[0x080CB5C8]==0:
//     u32[rec+424] = u16[sp+20 + a1*2]; u32[rec+436] = u16[sp+24 + a1*2];
//     7ABC(u32[rec+52], u32[rec+424], u32[rec+420]);
//     7ABC(u32[rec+52], u32[rec+436], u32[rec+432]).
//   7BFC(rec+48,u32[rec+420],u32[rec+424],168,96,11,1,0,0);
//   7BFC(rec+48,u32[rec+432],u32[rec+436],40,96,11,1,0,0);
//   u32[0x03000588] = a1; u32[0x080CB5C8] = 1 (ROM-ignored store).
//
// Four source levers, each load-bearing, each checked by compiling this TU
// with agbcc and diffing against baserom.gba (248/248 exact). Do not tidy any
// of them away — each one costs bytes:
//
//   * `u32 a1`, NOT `u16 a1 = (u16)a1_`. The truncation is hoisted out of the
//     body into the prologue as `lsls r1,r1,#16; lsrs r1,r1,#16`, where the
//     ROM has a bare `mov sl, r1`, and the ROM keeps the full 32-bit value
//     through the `cmp` and the store to 0x03000588. (143/248)
//   * `volatile u16 tmp[2]` / `tmp2[2]`, NOT a `u8 spad[28]` with pointers
//     into it. Two 4-byte locals is what makes the frame `sub sp, #28`
//     instead of `sub sp, #48`, and an *array* is what makes the indexed
//     read address base-first (`mov r0,sp; adds r0,r0,r1`) rather than
//     index-first (`adds r0,r1,#0; add r0,sp`). The volatile is what makes
//     it a single `ldrh`. (68/248)
//   * The reads are INLINED into the stores — no `u16 v0` temporary. agbcc
//     schedules a named temporary's RHS as its own statement and emits the
//     source address before the destination address; the ROM emits the
//     destination first. (26 -> 217/248)
//   * The FIRST 7BFC reads its arguments inline, the SECOND keeps the
//     `u32 r1/r2` temporaries. That asymmetry is real: inlining the first
//     is what puts `rec+48` into r9 ahead of the rec[420]/rec[424] loads,
//     and inlining the second does not reproduce its sibling's schedule.
void MenuFF78_122E0(void *rec_, u32 a1_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 a1 = a1_;
    volatile u16 tmp[2];
    volatile u16 tmp2[2];
    J_CALLEE(Sub_08002E0A4, sub_0802E0A4)((void *)&tmp[0],
                  (const void *)(uintptr_t)0x0805F9A0u, 4);
    J_CALLEE(Sub_08002E0A4, sub_0802E0A4)((void *)&tmp2[0],
                  (const void *)(uintptr_t)0x0805F9A4u, 4);
    if ((u32)a1 != *(volatile u32 *)(uintptr_t)0x03000588u ||
        *(volatile u32 *)(uintptr_t)0x080CB5C8u == 0) {
        *(volatile u32 *)(uintptr_t)(rec + 424) = (u32)(tmp[a1]);
        *(volatile u32 *)(uintptr_t)(rec + 436) = (u32)(tmp2[a1]);
        J_CALLEE(Sub_08007ABC, sub_08007ABC)(*(volatile u32 *)(uintptr_t)(rec + 52),
                     *(volatile u32 *)(uintptr_t)(rec + 424),
                     *(volatile u32 *)(uintptr_t)(rec + 420));
        J_CALLEE(Sub_08007ABC, sub_08007ABC)(*(volatile u32 *)(uintptr_t)(rec + 52),
                     *(volatile u32 *)(uintptr_t)(rec + 436),
                     *(volatile u32 *)(uintptr_t)(rec + 432));
    }
    J_CALLEE(Sub_08007BFC, sub_08007BFC)((void *)(uintptr_t)(rec + 48),
                 *(volatile u32 *)(uintptr_t)(rec + 420),
                 *(volatile u32 *)(uintptr_t)(rec + 424),
                 168, 96, 11, 1, 0, 0);
    {
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 432);
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 436);
        J_CALLEE(Sub_08007BFC, sub_08007BFC)((void *)(uintptr_t)(rec + 48), (int)r1, (int)r2,
                     40, 96, 11, 1, 0, 0);
    }
    *(volatile u32 *)(uintptr_t)0x03000588u = (u32)a1;
    *(volatile u32 *)(uintptr_t)0x080CB5C8u = 1;
}
#ifndef __APPLE__
void _0800122E0(void *a, u32 b) __attribute__((alias("MenuFF78_122E0")));
void Sub_0800122E0(void *a, u32 b) __attribute__((alias("MenuFF78_122E0")));
void sub_0800122E0(void *a, u32 b) __attribute__((alias("MenuFF78_122E0")));
#endif

// ----------------------------------------------------------------------------
// sub_0800123D8 — (rec):
//   11784(rec); if s16[rec+202] <= 0:
//     11E10, 11CFC, 11B48; if u8[WA+0x10C3]==1: 11E58(rec,u16[rec+180]);
//     F778(168,76,u32[WA+0x5E4+(s16)2581C(s16[rec+172])*72]).
//   if s16[rec+216]==1:
//     if u8[WA+0x10C3]==1: 122E0(rec, s16[WA+0xFEE]);
//       7C68(rec+48,u32[rec+408],u32[rec+412],32,72,11,1,0,0).
//     else: 7C68(rec+48,u32[rec+408],u32[rec+412],20,112,11,1,0,0).
//   D97C(rec+224,15); DBE8(rec+64); 12574(rec).
// Reconstructed from asm/menu_ff78.s: every `rec+offset` selector read is a
// NON-volatile s16 (`adds r0,#imm; movs r1,#0; ldrsh r0,[r0,r1]`); a volatile
// s16 lvalue emits `ldrh;lsls`. The work-area base 0x03001780 is NOT kept in
// a local: the ROM reloads it from its own pool word on each use
// (`ldr r5,_080012490` then `ldr r1,_080012490`) and the offset 0x10C3 comes
// from a second pool word, so the two are kept as separate constants to stop
// agbcc folding them into one address (which costs the extra callee-saved r7).
void MenuFF78_123D8(void *rec_) {
    u8 *rec = (u8 *)rec_;
    // Assembler-resolved work-area base (docs/findings/track_car_26180_pool_order.md):
    // a folded C constant would merge base+offset into one pool word, but the ROM
    // keeps 0x03001780 and each offset in separate pools and adds them.
    extern u8 J123D8_WA[];
    u32 o10C3 = 0x10C3u;
    u32 o5E4 = 0x5E4u;
    u32 oFEE = 0xFEEu;
    __asm__(".globl J123D8_WA\nJ123D8_WA = 0x03001780\n");
    J_CALLEE(Sub_080011784, sub_080011784)((void *)rec);
    if (*(s16 *)(rec + 202) <= 0) {
        J_CALLEE(Sub_080011E10, sub_080011E10)((void *)rec);
        J_CALLEE(Sub_080011CFC, sub_080011CFC)((void *)rec);
        J_CALLEE(Sub_080011B48, sub_080011B48)((void *)rec);
        if (*(volatile u8 *)((u8 *)(uintptr_t)J123D8_WA + o10C3) == 1)
            J_CALLEE(Sub_080011E58, sub_080011E58)((void *)rec, *(volatile u16 *)(rec + 180));
        {
            s16 i = *(s16 *)(rec + 172);
            s16 v = (s16)J_CALLEE(Sub_0802581C, sub_0802581C)((int)i);
            u32 off = (u32)((((s32)v << 3) + (s32)v) << 3); // *72
            u8 *wa5 = (u8 *)(uintptr_t)J123D8_WA + o5E4;
            u32 t = *(volatile u32 *)(wa5 + off);
            J_CALLEE(Sub_0800F778, sub_0800F778)(168, 76, t);
        }
    }
    if (*(s16 *)(rec + 216) == 1) {
        if (*(volatile u8 *)((u8 *)(uintptr_t)J123D8_WA + o10C3) == 1) {
            s16 w = *(s16 *)((u8 *)(uintptr_t)J123D8_WA + oFEE);
            J_CALLEE(Sub_0800122E0, sub_0800122E0)((void *)rec, (u32)(s32)w);
            _08007C68((void *)(rec + 48),
                         *(u32 *)(rec + 408), *(u32 *)(rec + 412),
                         32, 72, 11, 1, 0, 0);
        } else {
            _08007C68((void *)(rec + 48),
                         *(u32 *)(rec + 408), *(u32 *)(rec + 412),
                         20, 112, 11, 1, 0, 0);
        }
    }
    J_CALLEE(Sub_0800D97C, sub_0800D97C)((void *)(rec + 224), 15);
    J_CALLEE(Sub_0800DBE8, sub_0800DBE8)((void *)(rec + 64));
    J_CALLEE(Sub_080012574, sub_080012574)(rec);
}
// The ROM's 276-byte span ends in `00 00`. gas closes a Thumb code section
// with the `46c0` nop; this file-scope `.align 2, 0` (after the body's `.size`,
// still inside its section) pads with the explicit `0` fill instead.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800123D8(void *a) __attribute__((alias("MenuFF78_123D8")));
void sub_0800123D8(void *a) __attribute__((alias("MenuFF78_123D8")));
#endif
