#include "gtadv/sound.h"
#include "gba/types.h"
#include "gba/regs.h"


__attribute__((weak)) void sub_0802B888(u32 a, u32 b){ (void)a;(void)b; } // ARM veneer helper via mixer

// --- sound_d480.s _0802D480 vol-scale leaf (28 B, pool 0x68736D53) ---
// Push {r4,lr}, lsls r1 #16/lsrs, ldr [r2+52] vs Smsh, strh +30, ldrh +28, muls r4, asrs #8, strh +32
void SoundD480_VolScale(void *state, u16 vol){
    volatile u32 *s = (volatile u32 *)state;
    register u32 g __asm__("r3");
    register s32 base __asm__("r4");
    s32 scaled;
    s32 v = vol;
    g = s[13];
    if (g != SOUND_MAGIC) return; /* +52 */
    ((volatile u16 *)s)[15] = (u16)v; /* +30 */
    base = ((volatile u16 *)s)[14]; /* +28 */
    __asm__("" : "+r" (base));
    scaled = (v * base) >> 8; /* muls + asrs #8 (signed: ROM shifts arithmetically) */
    ((volatile u16 *)s)[16] = (u16)scaled; /* +32 */
}
#ifndef __APPLE__
void _0802D480(void *s, u16 v) __attribute__((alias("SoundD480_VolScale")));
void sub_0802D480(void *s, u16 v) __attribute__((alias("SoundD480_VolScale")));
#endif

void SoundVoiceClamp(void *voice){
    register volatile u8 *p __asm__("r1");
    register u32 a __asm__("r4");
    register u32 b __asm__("r3");
    register u32 ashi __asm__("r2");   // p[2]<<24, live to the a-side halving
    register u32 r0v __asm__("r0");   // ROM's one scratch: p[2], then p[3]<<24,
    u32 s;                            // r2/r3 are dead by the tail, so the pins
                                     // are REUSED there rather than doubled:
                                     // doubling them makes agbcc drop every pin.
    p = (volatile u8*)voice;
    r0v = p[2];                       // ROM: ldrb r0,[r1,#2]; lsls r2,r0,#24
    __asm__("" : "+r"(r0v));
    ashi = r0v << 24;
    a = ashi >> 24;
    b = p[3];                         // ROM: ldrb r3,[r1,#3]; lsls r0,r3,#24
    __asm__("" : "+r"(b));
    r0v = b << 24;
    __asm__("" : "+r"(r0v));
    b = r0v >> 24;
    if (a >= b) {
        r0v = ashi >> 25;             // ROM: lsrs r0, r2, #25
        if (r0v < b)
            goto big;
        p[27] = 0x0f;
        goto join;
    }
    r0v = r0v >> 25;                  // ROM: lsrs r0, r0, #25
    if (r0v >= a) {
        p[27] = 0xf0;
        goto join;
    }
big:
    p[27] = 0xff;
    ashi = p[3];                      // reuse the dead r2 pin: ROM ldrb r2,[r1,#3]
    __asm__("" : "+r"(ashi));
    b = p[2];                         // reuse the dead r3 pin for the addend
    __asm__("" : "+r"(b));
    r0v = (ashi + b) >> 4;            // ROM keeps the sum in r0 across the cmp
    p[10] = (u8)r0v;
    goto tail;
join:
    ashi = p[3];
    __asm__("" : "+r"(ashi));
    b = p[2];
    __asm__("" : "+r"(b));
    r0v = (ashi + b) >> 4;
    p[10] = (u8)r0v;
    if (r0v > 15)
        p[10] = 15;
tail:
    ashi = p[6];                      // reuse the dead r2 pin: ROM ldrb r2,[r1,#6]
    __asm__("" : "+r"(ashi));
    b = p[10];                        // ROM: ldrb r3,[r1,#10]
    __asm__("" : "+r"(b));
    s = ashi * b;
    p[25] = (u8)((s32)(s + 15) >> 4);
    r0v = p[28];                      // ROM loads p[28] FIRST: ldrb r0,[r1,#28]
    ashi = p[27];                     // then p[27]: ldrb r2,[r1,#27]
    r0v &= ashi;
    p[27] = (u8)r0v;
}
#ifndef __APPLE__
void _0802CFCC(void *v) __attribute__((alias("SoundVoiceClamp")));
void sub_0802CFCC(void *v) __attribute__((alias("SoundVoiceClamp")));
#endif

// --- sound_voice_follow.s _0802C390 note-off helper — blocked, exact in src/sound_voice_commands.c as _0802C390
// No duplicate alias here.

// Additional follow helpers preserve u8 widths at +22/+26
//
// LIFTED. The record is addressed through the SECOND parameter, not the first:
// the ROM never touches r0 and does `strb r2,[r1,#22]`, so the prototype here
// was one argument short and the body was writing through an uninitialised
// register. The mode mask is `p[24] ? 3 : 12`, and the ROM's
// `movs r2,#12 / b / movs r2,#3` is the ternary's two materialisations, so it
// is written as one expression rather than as an if/else.
// The mask lives in r2 and the reloaded flag byte in r3 for the whole tail, so
// both are pinned. The mask is MATERIALISED before p[0] is read: the ROM is
// `ldrb r2,[r1,#24]; cmp; bne; movs r2,#12; b; movs r2,#3; ldrb r3,[r1,#0]`.
// Folding the read into `p[0] |= p[24]?3:12` makes agbcc load p[0] first,
// and `m = p[24]==0?12:3` flips which constant is hoisted. The mask in its
// own register with 3 as the default is the shape that survives.
void SoundVoiceFollowA(void *a, void *b){
    volatile u8 *p=(volatile u8*)b;
    register u8 m asm("r2");
    register u8 f asm("r3");
    (void)a;
    m = 0;
    p[22]=m;
    p[26]=m;
    m = p[24];
    if (!m) m = 12; else m = 3;
    f = p[0];
    p[0] = f | m;
}
// `_0802C3D0`'s body is 26 bytes, so its `-ffunction-sections` section is
// padded to 28. Under that flag gas closes a Thumb *code* section with the
// 2-byte `nop` filler (0x46c0), where the ROM holds `00 00` -- which is why
// this scored 26/28 with every instruction already correct. A file-scope
// `.align 2, 0` is emitted after the body's `.size`, i.e. still inside the
// body's own section, and pads with the explicit `0` fill instead.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0802C3D0(void *a, void *b) __attribute__((alias("SoundVoiceFollowA")));
void sub_0802C3D0(void *a, void *b) __attribute__((alias("SoundVoiceFollowA")));
#endif

#ifdef __APPLE__
s32 SoundMixerVeneer(s32 a, s32 b){ (void)a; return 0; }
#else
__attribute__((naked)) s32 SoundMixerVeneer(s32 a, s32 b){
    __asm__ volatile(
        ".arm\n"
        "ldrmi sl, [r0, -r0, lsl #4]\n"
        "umull r2, r3, r0, r1\n"
        "add r0, r3, #0\n"
        "bx lr\n"
        ".thumb\n"
    );
}
#endif
// Alias preserves bx-mode transition width via naked ARM (VMA 0x0802B88C) — ARM only, host stub needs no alias
#ifndef __APPLE__
s32 _0802B88C(s32 a, s32 b) __attribute__((alias("SoundMixerVeneer")));
s32 sub_0802B88C(s32 a, s32 b) __attribute__((alias("SoundMixerVeneer")));
#endif

// --- sound_seq.s sequence interpreter branch 0x02D718 dispatch already via sound_core; extend with one handler ---
// Handler at 0x02D728 etc. is pure Thumb with u8 op widths — minimal handler preserves u8 fetch
void SoundSeqHandler0(void *state, void *cursor){ (void)state; (void)cursor; }
#ifndef __APPLE__
void _0802D728(void *s, void *c) __attribute__((alias("SoundSeqHandler0")));
void sub_0802D728(void *s, void *c) __attribute__((alias("SoundSeqHandler0")));
#endif

// Explicit TODO for remaining interiors where pool/prologue not exact:
// - mixer_2b888.s Thumb 0x02B898–0x02BA88 + ARM 0x02BA8C–0x02BC28 loop beyond veneer (IWRAM bx r3 0x03007001 pool 0x02B90C copy-site unproven)
// - sound_voice_follow full 0x02C390 walk beyond note-off, sound_d034 PSG beyond hardware-reg prefix, sound_d6f4 EWRAM vector 0x08061788
// No weak no-op aliases added for those interiors.
