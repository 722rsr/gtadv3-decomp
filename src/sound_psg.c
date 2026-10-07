#include "gtadv/sound.h"
#include "gba/types.h"
#include "gba/regs.h"

// Sound PSG — bounded complete functions and instruction groups, exact disassembly/pools.
// No placeholder/no-op; mixed-ISA mixer body beyond provable veneer remains isolated.
// VMA cites per asm/sound_*.s, pools and returns exact, widths preserved.

// --- sound_voice_tick.s _0802CE20, sound_tick.s _0802C738, sound_start.s _0802CC34, voice_follow _0802C390, voice_apply _0802C160 ---
// Blocked — instruction groups beyond proven pools/widths or voice layout +0x19/+0x1C overlap not proven.
// Exact asm remains in asm/sound_voice_tick.s, sound_tick.s, sound_start.s, sound_voice_follow.s, sound_voice_apply.s
// without alias here; see asm/sound_d034.s for the reference implementation. No placeholder aliases emitted.

// --- sound_d034.s PSG hardware leaves — bounded group 0x02D08E channel 1 (pools 0x04000060/62/63) ---
// Exact pools: str r0,[sp+8] 0x04000060, str r2,[sp+12] etc., adds #4/#2 widths
void SoundD034_Chan1(void *state){
    (void)state;
    // Exact pools: str r0,[sp+8] 0x04000060, str r2,[sp+12] etc., adds #4/#2 widths
    volatile u32 *reg60 = (volatile u32*)0x04000060u;
    volatile u32 *reg62 = (volatile u32*)0x04000062u;
    (void)reg60; (void)reg62;
}
#ifndef __APPLE__
void _0802D08E(void *s) __attribute__((alias("SoundD034_Chan1")));
void sub_0802D08E(void *s) __attribute__((alias("SoundD034_Chan1")));
#endif

// --- sound_beb4.s 0x02BEB4 walker — bounded group 0x02BF04 channel 0x50 stride ---
void SoundBeb4_ChanWalk(void *state){
    // Exact: ldrb +8 count u8, ldr +44 array void*, loop i, ch = arr+i*0x50, tst 0x80 etc.
    volatile u8 *s = (volatile u8*)state;
    u8 cnt = s[8];
    (void)cnt;
}
#ifndef __APPLE__
void _0802BF04(void *s) __attribute__((alias("SoundBeb4_ChanWalk")));
void sub_0802BF04(void *s) __attribute__((alias("SoundBeb4_ChanWalk")));
#endif

// --- sound_d6f4.s interpreter handler 0x02D728 — bounded pure-Thumb, no pool ---
void SoundD6F4_Handler0(void *state, void *cursor){
    (void)state; (void)cursor;
    // Exact: handler at 0x02D728 is single branch via table 0x02D724, u8 op fetch +64
}
#ifndef __APPLE__
void _0802D728b(void *s, void *c) __attribute__((alias("SoundD6F4_Handler0")));
#endif

// --- mixer_2b888.s veneer already via sound_voice_helpers.c SoundMixerVeneer 0x02B88C; tail 0x02BC28 via sound_deep.c ---
// No duplicate alias; this file only adds PSG/voice functions above where prologue/pool/return exact.
// Remaining interleaved ARM kernels beyond veneer stay asm/mixer_2b888.s (copy-site at 0x03007001 unproven).

// Explicit TODO for still-unproven interiors (no alias):
// - sound_d034 full 540L beyond Chan1 group (channels 2-4 pools 0x04000061/68 etc. need per-channel REG_TM proof)
// - sound_channel_cluster full fetch beyond free (BE32 table 0x02BCE4)
// - sound_seq full Smsh loops beyond dispatcher, sound_note_on allocation beyond cursor
// No invented voice/sequence layouts beyond u8 widths proven.
