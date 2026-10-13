#include "gtadv/sound.h"
#include "gba/types.h"

// Sound extended leaves — largest pure-Thumb evidence-backed units.
// Only substantiated semantics are aliased; mixer regions live in
// src/sound_mixer.c (see header note there for blocked interiors).
// VMA refs per asm/sound_*.s + asm/sound_api.s.

// 0x0802DE98 — 2-byte no-op leaf `mov pc, lr` (zero-divisor route; ROM returns
__attribute__((naked)) void SoundDivNoop_2DE98(void) {
    __asm__ volatile (
        "mov pc, lr\n"
        ".short 0x0000\n"
    );
}
void _0802DE98(void) __attribute__((alias("SoundDivNoop_2DE98")));
void sub_0802DE98(void) __attribute__((alias("SoundDivNoop_2DE98")));
void Sub_0802DE98(void) __attribute__((alias("SoundDivNoop_2DE98")));
void _08002DE98(void) __attribute__((alias("SoundDivNoop_2DE98")));  // 9-digit corpus spelling

// --- Signed EABI div (sound_aeabi_idiv.s 0x02DE04) ---
// s32 __aeabi_idiv(s32 num, s32 den) — shift/subtract quotient core, handles sign via ip.
// 0x0802DE04 — __aeabi_idiv. NOT PROMOTABLE: ARMCC runtime (returns `mov pc, lr`,
// lazy `push {r4}` frame), unlike every agbcc body. Byte-exact via
// asm/sound_aeabi_idiv.s; see build/scan_armcc.py for the marker scan.
__attribute__((naked)) s32 SoundSignedDiv(s32 num, s32 den) {
    __asm__ volatile (
        ".syntax unified\n"
        "\n"
        "cmp r1, #0\n"
        "beq.n .Lsoundde04_0802DE8C\n"
        "push {r4}\n"
        "adds r4, r0, #0\n"
        "eors r4, r1\n"
        "mov ip, r4\n"
        "movs r3, #1\n"
        "movs r2, #0\n"
        "cmp r1, #0\n"
        "bpl.n .Lsoundde04_0802DE1A\n"
        "negs r1, r1\n"
        ".Lsoundde04_0802DE1A:\n"
        "cmp r0, #0\n"
        "bpl.n .Lsoundde04_0802DE20\n"
        "negs r0, r0\n"
        ".Lsoundde04_0802DE20:\n"
        "cmp r0, r1\n"
        "bcc.n .Lsoundde04_0802DE7E\n"
        "movs r4, #1\n"
        "lsls r4, r4, #28\n"
        ".Lsoundde04_0802DE28:\n"
        "cmp r1, r4\n"
        "bcs.n .Lsoundde04_0802DE36\n"
        "cmp r1, r0\n"
        "bcs.n .Lsoundde04_0802DE36\n"
        "lsls r1, r1, #4\n"
        "lsls r3, r3, #4\n"
        "b.n .Lsoundde04_0802DE28\n"
        ".Lsoundde04_0802DE36:\n"
        "lsls r4, r4, #3\n"
        ".Lsoundde04_0802DE38:\n"
        "cmp r1, r4\n"
        "bcs.n .Lsoundde04_0802DE46\n"
        "cmp r1, r0\n"
        "bcs.n .Lsoundde04_0802DE46\n"
        "lsls r1, r1, #1\n"
        "lsls r3, r3, #1\n"
        "b.n .Lsoundde04_0802DE38\n"
        ".Lsoundde04_0802DE46:\n"
        "cmp r0, r1\n"
        "bcc.n .Lsoundde04_0802DE4E\n"
        "subs r0, r0, r1\n"
        "orrs r2, r3\n"
        ".Lsoundde04_0802DE4E:\n"
        "lsrs r4, r1, #1\n"
        "cmp r0, r4\n"
        "bcc.n .Lsoundde04_0802DE5A\n"
        "subs r0, r0, r4\n"
        "lsrs r4, r3, #1\n"
        "orrs r2, r4\n"
        ".Lsoundde04_0802DE5A:\n"
        "lsrs r4, r1, #2\n"
        "cmp r0, r4\n"
        "bcc.n .Lsoundde04_0802DE66\n"
        "subs r0, r0, r4\n"
        "lsrs r4, r3, #2\n"
        "orrs r2, r4\n"
        ".Lsoundde04_0802DE66:\n"
        "lsrs r4, r1, #3\n"
        "cmp r0, r4\n"
        "bcc.n .Lsoundde04_0802DE72\n"
        "subs r0, r0, r4\n"
        "lsrs r4, r3, #3\n"
        "orrs r2, r4\n"
        ".Lsoundde04_0802DE72:\n"
        "cmp r0, #0\n"
        "beq.n .Lsoundde04_0802DE7E\n"
        "lsrs r3, r3, #4\n"
        "beq.n .Lsoundde04_0802DE7E\n"
        "lsrs r1, r1, #4\n"
        "b.n .Lsoundde04_0802DE46\n"
        ".Lsoundde04_0802DE7E:\n"
        "adds r0, r2, #0\n"
        "mov r4, ip\n"
        "cmp r4, #0\n"
        "bpl.n .Lsoundde04_0802DE88\n"
        "negs r0, r0\n"
        ".Lsoundde04_0802DE88:\n"
        "pop {r4}\n"
        "mov pc, lr\n"
        ".Lsoundde04_0802DE8C:\n"
        "push {lr}\n"
        "bl 0x0802DE98\n"
        "movs r0, #0\n"
        "pop {pc}\n"
        ".short 0\n"
        ".syntax divided\n"
    );
}
s32 _0802DE04(s32 a, s32 b) __attribute__((alias("SoundSignedDiv")));
s32 sub_0802DE04(s32 a, s32 b) __attribute__((alias("SoundSignedDiv")));
s32 __aeabi_idiv(s32 a, s32 b) __attribute__((alias("SoundSignedDiv")));


// --- Stream allocation helper (sound_alloc.s 0x02CBBC) ---
// u32 sub_0802CBBC(state, channels, count) — validates count<=16, Smsh lock at *0x03007FF0==0x68736D53
void SoundStreamAlloc(void *state, void *chArray, u32 count){
    // Frame-exact transcription of asm/sound_alloc.s. Widths matter:
    //  * the count is narrowed to `u8` (`lsls #24 / lsrs #24`) and re-narrowed
    //    after each decrement -- `count & 0xFF` gives `movs #255 / ands`
    //    instead, which is a different 4 bytes;
    //  * `bl 0x0802C8B0` takes the state pointer in r0;
    //  * the driver's lock word is bumped to MAGIC+1 before that call, and the
    //    tail link fixes root[8]=0x0802BEB5 (the driver's next-tick entry),
    //    root[0]=MAGIC and state[13]=MAGIC sharing one constant in r0.
    // The two pointer parameters are used DIRECTLY (casts at the use sites)
    // rather than copied into locals: agbcc then homes the parameter pseudos
    // themselves, which is what puts state in r7 and chArray in r6 like the
    // ROM -- with separate locals it swaps them (state->r6, chArray->r7) and
    // the whole body comes out in the wrong registers.
    u8 c;
    u32 lock;
    volatile u32 *cell;
    volatile u32 *root;
    c = (u8)count;
    if (c == 0) return;
    if (c > 16) c = 16;
    cell = (volatile u32 *)SOUND_ROOT_PTR_CELL;
    root = (volatile u32 *)(uintptr_t)*cell;
    // Read the lock once: the ROM bumps the register it compared, it does not
    // re-load root[0].
    lock = root[0];
    if (lock != SOUND_MAGIC) return;
    root[0] = lock + 1;
    {
        extern void _0802C8B0(void *self);
        _0802C8B0(state);
    }
    ((volatile u32 *)state)[11] = (u32)(uintptr_t)chArray;   // +0x2C
    ((volatile u8 *)state)[8] = c;
    ((volatile u32 *)state)[1] = 0x80000000u;                // +0x04
    while (c != 0) {
        *(volatile u8 *)chArray = 0;
        c = (u8)(c - 1);
        chArray = (void *)((u8 *)chArray + 80);
    }
    {
        // Tested through a local: the ROM stores the very value it compared
        // (`str r0,[r7,#56]` with root[8] still in r0), not a fresh load.
        u32 nxt = root[8];
        if (nxt != 0) {
            ((volatile u32 *)state)[14] = nxt;               // +56
            ((volatile u32 *)state)[15] = root[9];           // +60
            root[8] = 0;
        }
    }
    root[9] = (u32)(uintptr_t)state;   // +36
    root[8] = 0x0802BEB5u;             // +32  driver next-tick entry
    root[0] = SOUND_MAGIC;
    ((volatile u32 *)state)[13] = SOUND_MAGIC;               // +52
}
void _0802CBBC(void *a, void *b, u32 c) __attribute__((alias("SoundStreamAlloc")));
void sub_0802CBBC(void *a, void *b, u32 c) __attribute__((alias("SoundStreamAlloc")));

// --- Bank claim layer (sound_bank.s 0x02B488) —
// The volume/pan leaf is a complete ROM body.  Its two walker calls carry the
// scaled volume and signed pan values into the sound descriptor layer; keeping
// the literal pool in place preserves the ARMCC register split at the call
// boundary.
__attribute__((naked)) void SoundBankVolPan(u32 id, u32 vol, u32 mode){
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "lsls r0, r0, #16\n"
        "lsrs r3, r0, #16\n"
        "lsls r1, r1, #16\n"
        "lsrs r1, r1, #16\n"
        "lsls r2, r2, #16\n"
        "asrs r2, r2, #16\n"
        "cmp r2, #1\n"
        "bne.n .Lsbvp_half\n"
        "lsls r0, r1, #16\n"
        "asrs r0, r0, #16\n"
        "lsls r2, r3, #16\n"
        "asrs r1, r2, #21\n"
        "adds r1, #110\n"
        "muls r0, r1\n"
        "adds r5, r2, #0\n"
        "cmp r0, #0\n"
        "bge.n .Lsbvp_scaled\n"
        "adds r0, #255\n"
        ".Lsbvp_scaled:\n"
        "lsls r0, r0, #8\n"
        "lsrs r1, r0, #16\n"
        "b.n .Lsbvp_apply\n"
        ".Lsbvp_half:\n"
        "lsls r0, r1, #16\n"
        "asrs r1, r0, #16\n"
        "lsrs r0, r0, #31\n"
        "adds r1, r1, r0\n"
        "lsls r1, r1, #15\n"
        "lsrs r1, r1, #16\n"
        "lsls r5, r3, #16\n"
        ".Lsbvp_apply:\n"
        "ldr r4, 1f\n"
        "ldr r0, [r4, #0]\n"
        "ldrb r0, [r0, #13]\n"
        "lsls r2, r1, #16\n"
        "asrs r2, r2, #16\n"
        "movs r1, #255\n"
        "bl _0802B718\n"
        "ldr r1, 2f\n"
        "adds r0, r5, r1\n"
        "asrs r0, r0, #16\n"
        "movs r1, #100\n"
        "muls r0, r1\n"
        "movs r1, #130\n"
        "bl _0802DE04\n"
        "adds r2, r0, #0\n"
        "ldr r0, [r4, #0]\n"
        "ldrb r0, [r0, #13]\n"
        "lsls r2, r2, #16\n"
        "asrs r2, r2, #16\n"
        "movs r1, #255\n"
        "bl _0802B74C\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2\n"
        "1: .word 0x03001764\n"
        "2: .word 0xEE6C0000\n"
        ".syntax divided\n"
    );
}
void _0802B488(u32 a, u32 b, u32 c) __attribute__((alias("SoundBankVolPan")));
void sub_0802B488(u32 a, u32 b, u32 c) __attribute__((alias("SoundBankVolPan")));

// --- Channel cluster fetch primitives (sound_channel_cluster.s) —
// Provide minimal fetch byte / BE32 / loop-stack stubs preserving u8/u32 widths.
// Full 328-line walker is TODO; these primitives are leaves with clear widths.
u8 SoundFetchByte(void *stream){ return *(volatile u8*)((uintptr_t)stream); }
// BCE8 is a two-byte entry prefix. It loads the cursor and falls through into
// BCEA's shared fetch tail, which advances the cursor and returns via BCCE.
__attribute__((naked)) u32 SoundBCE8_CursorFetch(void *dummy, void *seq) {
    __asm__ volatile (
        ".syntax unified\n"
        "ldr r2, [r1, #64]\n"
        ".syntax divided\n"
    );
}
u32 _0802BCE8(void *dummy, void *s) __attribute__((alias("SoundBCE8_CursorFetch")));
u32 sub_0802BCE8(void *dummy, void *s) __attribute__((alias("SoundBCE8_CursorFetch")));


// --- Premix leaves (sound_premix.s 0x02B7B4–0x02B888) — accessor/pitch stubs ---
// The ROM is `ldr r0,[r0,#8] / bx lr` -- it LOADS the word at +8 and returns
// that, it does not return the address of +8. Spelling this as
// `(void*)((uintptr_t)obj+8)` compiled to `adds r0,#8`, which is 2/4 and was
// reading as a padding miss when it is a semantic one: the two differ in what
// the caller receives, not only in the instruction.
void *SoundPremixGetPtr(void *obj){ return *(void **)((uintptr_t)obj+8); }
void *_0802B7B4(void *o) __attribute__((alias("SoundPremixGetPtr")));
void *SoundPremixPitch(void *obj, u32 *freq){
    volatile u32 *p = *(volatile u32 **)((uintptr_t)obj+8);
    p[0] = (freq[0] << 13) + 0x20000;
    p[1] = 0x18000;
    p[3] = freq[0];
    return obj;
}
void *_0802B7B8(void *o, u32 *f) __attribute__((alias("SoundPremixPitch")));

// --- Mixer tail (sound_mixer_tail.s) — termination stub, pure Thumb ---
// Removed: the `_0802BC4C_tail` alias of a no-op stub claimed VMA 0x0802BC4C,
// which is really the 64-byte clear owned by SoundClearVoiceArray
// (src/sound_support.c). Two C bodies for one ROM entry made the probe score
// the stub instead of the real body.


// ROM entry alias.
void _08002B488(u32 id, u32 vol, u32 mode) __attribute__((alias("SoundBankVolPan")));

// ROM entry alias.
void * _08002B7B4(void *obj) __attribute__((alias("SoundPremixGetPtr")));

// ROM entry alias.
void *_08002B7B8(void *obj, u32 *freq) __attribute__((alias("SoundPremixPitch")));

// ROM entry alias.
s32 _08002DE04(s32 num, s32 den) __attribute__((alias("SoundSignedDiv")));
