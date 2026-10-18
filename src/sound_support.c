#include "gtadv/sound.h"
#include "gba/bios.h"

// Sound support leaves — substantiated pure-Thumb leaves only.
// Mixer regions live in src/sound_mixer.c (ARM veneer
// 0x02B888, Thumb setup 0x02B898, ARM kernel 0x02B928, Thumb dispatch
// 0x02B968, ARM mix loop 0x02BA8C); remaining sound leaves in src/sound_*.c.

// Host-linkable marker: on ARM the strong sub_0802B898(void) body in
// src/sound_mixer.c wins at link time (weak never shadows strong); the
// zero-arg decl also matches SoundThunk's register-forwarding call below,
// mirroring the ROM push/bl/pop thunk which forwards r0 untouched.
__attribute__((weak)) void sub_0802B898(void) {} // mixer entry marker

// _0802BE78 pump — DMA1 DirectSound A re-arm, preserve register semantics
void SoundPump(void){
    register volatile u32 *rootCell __asm__("r0");
    register volatile u8 *root __asm__("r0");
    register u32 magic __asm__("r2");
    register u32 lock __asm__("r3");
    register u32 countdown __asm__("r1");

    /* The shared pool at 0x0802C114/0x0802C118 is 0x298 bytes ahead. */
    __asm__ volatile (
        ".syntax unified\n"
        ".hword 0x48A6 @ %0\n"
        ".syntax divided\n"
        : "=r"(rootCell));
    root = (volatile u8 *)(uintptr_t)*rootCell;
    __asm__ volatile (
        ".syntax unified\n"
        ".hword 0x4AA6 @ %0\n"
        ".syntax divided\n"
        : "=r"(magic));
    lock = *(volatile u32 *)root;
    lock -= magic;
    if (lock > 1)
        goto pump_done;

    countdown = root[4];
    countdown -= 1;
    root[4] = (u8)countdown;
    /* Consume the signed flags from the decrement, as the ROM's BGT does. */
    __asm__ volatile (".syntax unified\nbgt .Lsound_pump_done\n.syntax divided" : : : "cc");
    countdown = root[11];
    root[4] = (u8)countdown;

    register volatile u32 *dma __asm__("r2") =
        (volatile u32 *)(uintptr_t)0x040000BCu;
    register u32 dmaCount __asm__("r1") = dma[2];
    dmaCount <<= 7;
    __asm__ volatile ("" : "+r"(dmaCount));
    /* BCC consumes the shift's carry (original DMA word bit 25). */
    __asm__ volatile (".syntax unified\nbcc .Lsound_pump_rearm\n.syntax divided" : : : "cc");
    register u32 prime __asm__("r1") = 0x84400004u;
    __asm__ volatile ("" : "+r"(prime));
    *(volatile u32 *)(dma + 2) = prime;
    __asm__ volatile (".syntax unified\n.Lsound_pump_rearm:\n.syntax divided");

    register u32 control __asm__("r1") = 4;
    __asm__ volatile ("" : "+r"(control));
    control <<= 8;
    *(volatile u16 *)((volatile u8 *)dma + 10) = (u16)control;
    control = 182;
    control <<= 8;
    *(volatile u16 *)((volatile u8 *)dma + 10) = (u16)control;
pump_done:
    __asm__ volatile (".syntax unified\n.Lsound_pump_done:\n.syntax divided");
}

void _0802BE78(void) __attribute__((alias("SoundPump")));
void sub_0802BE78(void) __attribute__((alias("SoundPump")));

// _0802C53C thunk -> mixer
void SoundThunk(void){ sub_0802B898(); }
__asm__(".align 2, 0");
void _0802C53C(void) __attribute__((alias("SoundThunk")));
void sub_0802C53C(void) __attribute__((alias("SoundThunk")));

// _0802C488 / _0802C4A4 pause gates (Smsh guard)
//
// WHY A PIN, AND WHY THE SUGGESTED LEVER WAS NOT IT. The obvious reading of the
// residual is that agbcc is reusing the register the zero-extension just freed,
// so the proposed remedy was to declare the `u16` parameter as `u32` and re-narrow
// it with an explicit `(u16)` cast at the stores, regenerating the lsls/lsrs pair
// at the point of use. That premise is wrong, and the disassembly says so. agbcc
// never reuses the freed register for the load: it emits the extension pair
// `lsls r1,r1,#16 / lsrs r3,r1,#16`, i.e. it widens r1 *through* r3 and leaves the
// extended value in r3, so r1 is genuinely free and the load takes it. Widening
// the parameter removes the pair entirely and says nothing about which register
// the load lands in. Note also that _0802C488 has no parameter at all, yet shows
// the identical divergence -- so no parameter-type spelling can be the lever for
// either body. The allocation is decided in gcc-2.95 `local_alloc`, which no C
// source spelling addresses. The control below is the same code with the pin
// removed: it reproduces the 26/28 and 27/32 residuals byte for byte, so the pin
// is load-bearing and the naming is not.
//
// THE PIN. `register u32 g __asm__("r3")` (GNU local register variable, the
// construct already used for this exact class of miss in src/ai_line_leaves.c and
// src/code_22d20.c) forces the ROM register for the guard load. In _0802C4A4 this
// additionally fixes the parameter allocation as a side effect: with r3 bound to
// the guard, the zero-extension pair collapses to the ROM's in-place
// `lsls r1,r1,#16 / lsrs r1,r1,#16`, so both divergent stores fall back to r1.
// One pin per body -- the C89 transform cannot handle two in one declaration
// block. The pin is inert on the host: clang accepts the register name and
// ignores it.
void SoundPauseGateOff(void *state){
    volatile u32 *s = (volatile u32 *)state;
    register u32 g __asm__("r3"); // ROM `ldr r3,[r2,#52]` at +0x02 / `cmp r3,r0`
    g = s[13];
    if (g != SOUND_MAGIC) return; // +0x34 = idx13
    s[1] &= 0x7FFFFFFFu; // +0x04 clear bit31
}
void _0802C488(void *s) __attribute__((alias("SoundPauseGateOff")));
void sub_0802C488(void *s) __attribute__((alias("SoundPauseGateOff")));
void SoundPauseGateOn(void *state, u16 v){
    volatile u32 *s = (volatile u32 *)state;
    u16 fade = 0x100;
    register u32 g __asm__("r3"); // ROM `ldr r3,[r2,#52]` at +0x06 / `cmp r3,r0`
    g = s[13];
    if (g != SOUND_MAGIC) return; // +0x34
    ((volatile u16*)s)[19] = v; // +0x26 (ROM order: 0x26 precedes 0x24)
    ((volatile u16*)s)[18] = v; // +0x24
    ((volatile u16*)s)[20] = fade; // +0x28
}
void _0802C4A4(void *s, u16 v) __attribute__((alias("SoundPauseGateOn")));
void sub_0802C4A4(void *s, u16 v) __attribute__((alias("SoundPauseGateOn")));
// _0802BC64 unlink — doubly-linked active voice list.
// Self-contained leaf, no calls, no pool, frameless `bx lr`.
// ROM: `ldr r3,[r0,#44] / cmp / beq / ldr r1,[r0,#52] / ldr r2,[r0,#48] /
//       cmp / beq / str r1,[r2,#52] / b / str r1,[r3,#32] / cmp r1,#0 /
//       beq / str r2,[r1,#48] / movs r1,#0 / str r1,[r0,#44] / bx lr`.
//
// Three loads are hoisted to the block head, and their ORDER plus their
// destination registers are the whole of the remaining difference. agbcc
// hoists in declaration order, so the locals are declared in the ROM's load
// order (head +0x2C, next +0x34, prev +0x30) and each is pinned to the ROM
// register. With next in r1 and prev in r2 both already live, `chan` stays in
// its own parameter register r0 and the `adds r1,r0,#0` base copy that the
// unpinned body emitted disappears; that copy was the entire 4-byte overshoot.
void SoundUnlinkVoice(void *chan){
    volatile u32 *c = (volatile u32 *)chan;
    register volatile u32 *head asm("r3");
    register volatile u32 *next asm("r1");
    register volatile u32 *prev asm("r2");
    register u32 zero asm("r1");
    head = (volatile u32 *)c[11]; // +0x2C
    if (!head) return;
    next = (volatile u32 *)c[13]; // +0x34
    prev = (volatile u32 *)c[12]; // +0x30
    if (prev) prev[13]= (u32)(uintptr_t)next; else head[8]= (u32)(uintptr_t)next;
    if (next) next[12]= (u32)(uintptr_t)prev;
    zero = 0;
    c[11]=zero;
}
void _0802BC64(void *c) __attribute__((alias("SoundUnlinkVoice")));
void sub_0802BC64(void *c) __attribute__((alias("SoundUnlinkVoice")));

// _0802BC4C clear 64B array, _0802DFE4 divmod
// ROM is a fully unrolled 16-word zero (movs r1-r4,#0 + 4x stmia r0! fenced
// by mov ip,r4 / mov r4,ip, no push, no branches). __builtin_memset emits
// bl memset (UNRESOLVED, closure has no such symbol). Measured 2026-10-13:
// 16 scalar word stores compile to 16x str (36 B, OVERSIZED 0/24), not
// stmia, so no plain-C zeroing reproduces the multi-store; stays asm-owned
// (asm/sound_mixer_leaf.s) with this behavioral spelling.
__attribute__((naked)) void SoundClearVoiceArray(void *base){
    __asm__ volatile(
        ".syntax unified\n"
        ".thumb\n"
        "mov r12, r4\n"
        "movs r1, #0\n"
        "movs r2, #0\n"
        "movs r3, #0\n"
        "movs r4, #0\n"
        "stmia r0!, {r1, r2, r3, r4}\n"
        "stmia r0!, {r1, r2, r3, r4}\n"
        "stmia r0!, {r1, r2, r3, r4}\n"
        "stmia r0!, {r1, r2, r3, r4}\n"
        "mov r4, r12\n"
        "bx lr\n"
        ".syntax divided\n"
    );
}
__asm__(".align 2, 0");
void _0802BC4C(void *b) __attribute__((alias("SoundClearVoiceArray")));
void sub_0802BC4C(void *b) __attribute__((alias("SoundClearVoiceArray")));

// Friendly-name wrapper used by lifted C (idle_dispatch.c mode-3 tail,
// state_block_a.c). _08002DE04 (signed __aeabi_idiv) routes div-by-zero
// through the no-op leaf _08002DE98 then returns 0, so plain C division
// with a zero guard is behaviorally identical on return value.
// (DivModU removed : audit proved its last two callers needed
// the DFE4 remainder / DF6C body instead; zero callers remained, and the
// quotient-wrapper name shadowed the remainder core. Use DivRemU_02DFE4 or
// sub_0802DF6C.)
int DivSI(int num, int den) { return den ? num / den : 0; }
