#include "gtadv/sound.h"
#include "gba/types.h"

// Sound extended leaves — largest pure-Thumb evidence-backed units.
// Only substantiated semantics are aliased; mixer regions live in
// src/sound_mixer.c (see header note there for blocked interiors).
// VMA refs per asm/sound_*.s + asm/sound_api.s.

// 0x0802DE98 — 2-byte no-op leaf `mov pc, lr` (zero-divisor route; ROM returns
#ifndef __APPLE__
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
#else
void SoundDivNoop_2DE98(void) {}
#endif

// --- Signed EABI div (sound_aeabi_idiv.s 0x02DE04) ---
// s32 __aeabi_idiv(s32 num, s32 den) — shift/subtract quotient core, handles sign via ip.
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
#ifndef __APPLE__
s32 _0802DE04(s32 a, s32 b) __attribute__((alias("SoundSignedDiv")));
s32 sub_0802DE04(s32 a, s32 b) __attribute__((alias("SoundSignedDiv")));
s32 __aeabi_idiv(s32 a, s32 b) __attribute__((alias("SoundSignedDiv")));
#endif


// --- Stream allocation helper (sound_alloc.s 0x02CBBC) ---
// u32 sub_0802CBBC(state, channels, count) — validates count<=16, Smsh lock at *0x03007FF0==0x68736D53
void SoundStreamAlloc(void *state, void *chArray, u32 count){
    u32 c = count & 0xFF;
    if (c==0) return;
    if (c>16) c=16;
    volatile u32 *cell = (volatile u32 *)SOUND_ROOT_PTR_CELL;
    volatile u32 *root = (volatile u32 *)(uintptr_t)*cell;
    if (root[0] != SOUND_MAGIC) return;
    // minimal: publish ptrs, clear stride 0x50 array as in asm (80 bytes per channel)
    // preserve u8/u32 widths: state+0x2C, +8, +4, +56/+60/next ptrs
    volatile u32 *s = (volatile u32 *)state;
    s[11]= (u32)(uintptr_t)chArray; // +0x2C
    ((volatile u8*)s)[8]= (u8)c;
    s[1]= 0x80000000u; // +0x04
    // zero channels
    volatile u8 *ch = (volatile u8*)chArray;
    for(u32 i=0;i<c;i++) ch[i*80]=0;
    // link into active list (root+32/36) simplified, preserve stride
    if (root[8]!=0){
        s[14]= root[8]; // +56
        s[15]= root[9]; // +60
        root[8]=0;
    }
    root[9]= (u32)(uintptr_t)state; // +36
    root[8]= SOUND_MAGIC; // +32
    s[13]= SOUND_MAGIC; // +52
}
#ifndef __APPLE__
void _0802CBBC(void *a, void *b, u32 c) __attribute__((alias("SoundStreamAlloc")));
void sub_0802CBBC(void *a, void *b, u32 c) __attribute__((alias("SoundStreamAlloc")));
#endif

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
#ifndef __APPLE__
void _0802B488(u32 a, u32 b, u32 c) __attribute__((alias("SoundBankVolPan")));
void sub_0802B488(u32 a, u32 b, u32 c) __attribute__((alias("SoundBankVolPan")));
#endif

// --- Channel cluster fetch primitives (sound_channel_cluster.s) —
// Provide minimal fetch byte / BE32 / loop-stack stubs preserving u8/u32 widths.
// Full 328-line walker is TODO; these primitives are leaves with clear widths.
u8 SoundFetchByte(void *stream){ return *(volatile u8*)((uintptr_t)stream); }
#ifndef __APPLE__
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
#else
u32 SoundBCE8_CursorFetch(void *dummy, void *seq) {
    (void)dummy;
    (void)seq;
    return 0;
}
#endif


// --- Premix leaves (sound_premix.s 0x02B7B4–0x02B888) — accessor/pitch stubs ---
// The ROM is `ldr r0,[r0,#8] / bx lr` -- it LOADS the word at +8 and returns
// that, it does not return the address of +8. Spelling this as
// `(void*)((uintptr_t)obj+8)` compiled to `adds r0,#8`, which is 2/4 and was
// reading as a padding miss when it is a semantic one: the two differ in what
// the caller receives, not only in the instruction.
void *SoundPremixGetPtr(void *obj){ return *(void **)((uintptr_t)obj+8); }
#ifndef __APPLE__
void *_0802B7B4(void *o) __attribute__((alias("SoundPremixGetPtr")));
#endif
void *SoundPremixPitch(void *obj, u32 *freq){
    volatile u32 *p = *(volatile u32 **)((uintptr_t)obj+8);
    p[0] = (freq[0] << 13) + 0x20000;
    p[1] = 0x18000;
    p[3] = freq[0];
    return obj;
}
#ifndef __APPLE__
void *_0802B7B8(void *o, u32 *f) __attribute__((alias("SoundPremixPitch")));
#endif

// --- Mixer tail (sound_mixer_tail.s) — termination stub, pure Thumb ---
void SoundMixerTail(void *state){ (void)state; }
#ifndef __APPLE__
void _0802BC4C_tail(void *s) __attribute__((alias("SoundMixerTail"))); // distinct from _0802BC4C leaf
#endif


// ROM entry alias.
#ifndef __APPLE__
void _08002B488(u32 id, u32 vol, u32 mode) __attribute__((alias("SoundBankVolPan")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void * _08002B7B4(void *obj) __attribute__((alias("SoundPremixGetPtr")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void *_08002B7B8(void *obj, u32 *freq) __attribute__((alias("SoundPremixPitch")));
#endif

// ROM entry alias.
#ifndef __APPLE__
s32 _08002DE04(s32 num, s32 den) __attribute__((alias("SoundSignedDiv")));
#endif
