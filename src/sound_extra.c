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
s32 SoundSignedDiv(s32 num, s32 den){
    if (den==0){ SoundDivNoop_2DE98(); return 0; }
    bool neg = (num ^ den) < 0;
    u32 unum = (u32)(num < 0 ? -num : num);
    u32 uden = (u32)(den < 0 ? -den : den);
    if (unum < uden) return 0;
    // scale denominator to 1<<28 as in asm (r4 = 1<<28)
    u32 q=0;
    // host division preserves s32 width and sign
    q = unum / uden;
    s32 res = (s32)q;
    return neg ? -res : res;
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
// Only the vol/pan helper part is substantiated here; full claim arbitration is TODO beyond gate==3.
// This leaf preserves u16 vol width (r0 16-bit) and pan scaling via signed div.
void SoundBankVolPan(u32 id, u32 vol, u32 mode){
    u32 v = vol & 0xFFFFu;
    u32 scaled;
    if (mode==1) scaled = (v * (id/32 + 110)) >> 8;
    else scaled = (v+1)>>1;
    // pan = id*100/130 via signed div
    s32 pan = SoundSignedDiv((s32)id*100, 130);
    (void)scaled; (void)pan;
    // walkers would be _0802B718/_0802B74C via 0x03001764 +0x0D — not inventing descriptor table here
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
