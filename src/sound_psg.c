#include "gtadv/sound.h"
#include "gba/types.h"
#include "gba/regs.h"

// Sound PSG tick — readable model of asm/sound_d034.s _0802D034
// (VMA 0x0802D034-0x0802D480, pure Thumb, ~270 insns).
//
// The VMA itself stays owned by asm/sound_d034.s (global sub_0802D034 /
// _0802D034, referenced by src/sound_core.c); this TU claims no alias here.
// What follows is a control-flow-faithful C model: same channel loop
// (1..4, stride 64), same pools/widths, same helper calls, same hardware
// stores. Offsets are facts from the asm (cited per block); fields the ROM
// never names keep offset-stable struct members, not invented semantics.
//
// GBA register facts used (pool words in the asm): ch1 tone
// 0x04000060/62/63/64/65, ch2 tone 0x04000061/68/69/6C/6D, ch3 wave
// 0x04000070/72/73/74/75 + wave RAM 0x04000090, ch4 noise
// 0x04000071/78/79/7C/7D, control 0x04000081/89, wave table 0x08061744.

void SoundVoiceClamp(void *voice);      // _0802CFCC (sound_voice_helpers.c)
void SoundHWMode_CF7C(u32 mode);        // _0802CF7C (sound_hwmode_cf7c.c)
void _0802C89C(void *a);                // sound_sequence_init.c
void _0802C8B0(void *self);             // sound_sequence_init.c

// Channel record, 64 bytes (stride proven by r9 = r4+64 at 0x0802D070 and
// the D464 loop step). Only offsets the ROM touches are named.
typedef struct {
    u8 f0;          // +0 flags (0x80 key-on, 0x40 retrigger-gate, 0x04 release, low 2 bits stage)
    u8 f1;          // +1 (hw-mode gate bit 3 at 0x0802D372)
    u8 p2[2];
    u8 f4;          // +4 rate/level base
    u8 f5;          // +5 sustain level
    u8 f6;          // +6 decay flag
    u8 f7;          // +7 attack rate
    u8 p8;
    u8 f9;          // +9 envelope volume
    u8 f10;         // +10 volume target
    u8 f11;         // +11 envelope tick
    u8 f12;         // +12 volume factor
    u8 f13;         // +13 release counter
    u8 p14[5];
    u8 f19;         // +19 (ch4 marker at 0x0802D3FA..)
    u8 p20[5];
    u8 f25;         // +25 decay ceiling
    u8 f26;         // +26 hw control byte
    u8 f27;         // +27 hw set bits
    u8 f28;         // +28 hw clear bits
    u8 f29;         // +29 dirty bits (1 hw, 2 freq)
    u8 f30;         // +30 note/level
    u8 f31;         // +31 ch1 sweep byte
    u32 f32;        // +32 frequency accumulator
    u32 f36;        // +36 wave pointer
    u32 f40;        // +40 wave pointer shadow
    u8 p44[20];
} PsgCh;

// Per-channel register bundle (pools at 0x0802D0A0/D0B8/D0D8/D144).
typedef struct { volatile u8 *a, *a2, *b, *c, *d; } PsgRegs;
static PsgRegs PsgRegsFor(u32 chn) {
    PsgRegs r;
    if (chn == 1) {
        r.a = (volatile u8 *)0x04000060u; r.a2 = (volatile u8 *)0x04000062u;
        r.b = (volatile u8 *)0x04000063u; r.c = (volatile u8 *)0x04000064u;
        r.d = (volatile u8 *)0x04000065u;
    } else if (chn == 2) {
        r.a = (volatile u8 *)0x04000061u; r.a2 = (volatile u8 *)0x04000068u;
        r.b = (volatile u8 *)0x04000069u; r.c = (volatile u8 *)0x0400006Cu;
        r.d = (volatile u8 *)0x0400006Du;
    } else if (chn == 3) {
        r.a = (volatile u8 *)0x04000070u; r.a2 = (volatile u8 *)0x04000072u;
        r.b = (volatile u8 *)0x04000073u; r.c = (volatile u8 *)0x04000074u;
        r.d = (volatile u8 *)0x04000075u;
    } else {
        r.a = (volatile u8 *)0x04000071u; r.a2 = (volatile u8 *)0x04000078u;
        r.b = (volatile u8 *)0x04000079u; r.c = (volatile u8 *)0x0400007Cu;
        r.d = (volatile u8 *)0x0400007Du;
    }
    return r;
}

void SoundPsgTick(void) {
    volatile u8 *root = (volatile u8 *)(uintptr_t)*(volatile u32 *)0x03007FF0u;
    u8 slot = root[10]; // 0x0802D046 frame divider
    if (slot != 0)
        root[10] = (u8)(slot - 1);
    else
        root[10] = 14;
    {
        PsgCh *ch = (PsgCh *)(uintptr_t)*(volatile u32 *)(root + 28);
        u32 chn;
        for (chn = 1; chn <= 4; chn++, ch++) {
            u8 flags = ch->f0; // 0x0802D064
            if ((flags & 0xC7u) == 0)
                continue; // 0x0802D074 -> D464
            {
                PsgRegs rg = PsgRegsFor(chn);
                u8 frame = root[10]; // 0x0802D0F6 snapshot
                u8 env = *rg.b;      // 0x0802D0FE reg snapshot into r8
                if ((flags & 0x80u) == 0)
                    goto sustain_gate; // 0x0802D1EA
                // Key-on path (0x0802D10C): 0x40 set means retrigger-gated off.
                if ((flags & 0x40u) != 0)
                    goto note_off; // 0x0802D20E
                ch->f0 = 3;
                ch->f29 = 3;
                SoundVoiceClamp((void *)ch); // 0x0802D12E
                if (chn == 2)
                    goto key_common;
                if (chn == 1) { // 0x0802D156
                    *rg.a = ch->f31;
key_common: // 0x0802D15C
                    *rg.a2 = (u8)(ch->f30 + (ch->f36 << 6));
                } else if (chn == 3) { // wave channel, 0x0802D168
                    if (ch->f36 != ch->f40) {
                        volatile u32 *dst = (volatile u32 *)0x04000090u;
                        u32 src = ch->f36;
                        *rg.a = 0x40;
                        dst[0] = ((volatile u32 *)(uintptr_t)src)[0];
                        dst[1] = ((volatile u32 *)(uintptr_t)src)[1];
                        dst[2] = ((volatile u32 *)(uintptr_t)src)[2];
                        dst[3] = ((volatile u32 *)(uintptr_t)src)[3];
                        ch->f40 = src;
                    }
                    *rg.a = 0; // r5 == 0 on this path, 0x0802D190
                    *rg.a2 = ch->f30;
                    if (ch->f30 == 0)
                        ch->f26 = 0x80; // 0x0802D1A8
                    else
                        ch->f26 = 0xC0;
                } else { // ch4, 0x0802D1B0
                    *rg.a2 = ch->f30;
                    *rg.c = (u8)((u32)ch->f36 << 3);
                }
                env = (u8)(ch->f4 + 8u); // 0x0802D1BC
                ch->f26 = (ch->f30 == 0) ? 0 : 0x40;
                ch->f11 = ch->f4;
                if ((ch->f4 & 0xFFu) == 0)
                    goto sustain_decide; // 0x0802D1E0 -> shared 0x0802D322
                ch->f9 = 0;
                goto tick_once; // 0x0802D1E6 -> D350
sustain_gate: // 0x0802D1EA release/sustain gate path
                if ((flags & 4u) == 0) {
                    // Sustain gate (0x0802D21C): 0x40 plus a nonzero stage arms.
                    if ((flags & 0x40u) != 0 && (flags & 3u) != 0) {
                        ch->f0 = (u8)(flags & 0xFCu);
                        ch->f11 = ch->f7;
                        if ((ch->f7 & 0xFFu) == 0)
                            goto vol_recompute; // 0x0802D248
                        ch->f29 |= 1u;
                        if (chn != 3)
                            env = ch->f7;
                        goto tick_once; // 0x0802D25A -> D350
                    }
                    if (ch->f11 != 0)
                        goto tick_once; // 0x0802D25C bne D350
                    if (chn == 3)
                        ch->f29 |= 1u;
                    SoundVoiceClamp((void *)ch); // 0x0802D270
                    goto stage_machine;
                } else {
                    // Release counter (0x0802D1F2): (f13-1) in 1..127 stays.
                    {
                        u8 rel = (u8)(ch->f13 - 1u);
                        ch->f13 = rel;
                        if ((s32)((s32)rel << 24) > 0)
                            goto hw_section; // 0x0802D362
                        goto note_off;
                    }
                }
stage_machine: // 0x0802D274
                {
                    u8 stage = (u8)(ch->f0 & 3u);
                    if (stage != 0) {
                        if (stage == 1) {
                            ch->f9 = ch->f25; // 0x0802D2C6
                            ch->f11 = 7;
                            goto tick_once;
                        }
                        if (stage == 2) { // decay, 0x0802D2CE
                            {
                                u8 v = (u8)(ch->f9 - 1u);
                                ch->f9 = v;
                                if ((s32)((s32)v << 24) > (s32)((s32)ch->f25 << 24)) {
                                    ch->f11 = ch->f5; // 0x0802D30E
                                    goto tick_once;
                                }
                            }
                            if (ch->f6 != 0) {
                                ch->f0 &= 0xFCu;
                                goto vol_recompute; // 0x0802D28E
                            }
                            ch->f0 = (u8)(ch->f0 - 1u);
                            ch->f29 |= 1u;
                            if (chn != 3)
                                env = 8;
                            ch->f9 = ch->f25;
                            ch->f11 = 7;
                            goto tick_once;
                        }
                        // stage 3: rise (0x0802D312)
                        {
                            u8 v = (u8)(ch->f9 + 1u);
                            ch->f9 = v;
                            if (v < ch->f10) {
                                ch->f11 = ch->f4; // 0x0802D34C
                                goto tick_once;
                            }
                        }
                        // sustain-decide (0x0802D322, shared with key-on D1E0)
sustain_decide:
                        ch->f0 = (u8)(ch->f0 - 1u);
                        ch->f11 = ch->f5;
                        if ((ch->f5 & 0xFFu) == 0) {
                            if (ch->f6 == 0) {
                                ch->f0 &= 0xFCu;
                                goto vol_recompute;
                            }
                            ch->f0 = (u8)(ch->f0 - 1u);
                            ch->f29 |= 1u;
                            if (chn != 3)
                                env = 8;
                            ch->f9 = ch->f25;
                            ch->f11 = 7;
                            goto tick_once;
                        }
                        ch->f29 |= 1u;
                        ch->f9 = ch->f10;
                        if (chn != 3)
                            env = ch->f5;
                        goto tick_once;
                    }
                    {
                        u8 v = (u8)(ch->f9 - 1u);
                        ch->f9 = v;
                        if ((s32)((s32)v << 24) > 0) {
                            ch->f11 = ch->f7; // 0x0802D2BE
                            goto tick_once;
                        }
                        goto vol_recompute;
                    }
                }
vol_recompute: // 0x0802D28E (stage-0 and stage-2 fallthrough target)
                {
                    u32 m = (u32)ch->f12 * (u32)ch->f10 + 255u;
                    u8 v = (u8)((s32)m >> 8);
                    ch->f9 = v;
                    if (v == 0)
                        goto note_off;
                    ch->f0 |= 4u;
                    ch->f29 |= 1u;
                    if (chn != 3)
                        env = 8;
                    goto hw_section;
                }
tick_once: // 0x0802D350 envelope tick
                ch->f11 = (u8)(ch->f11 - 1u);
                if (frame == 0) { // 0x0802D360
                    frame--;
                    goto env_loop; // -> 0x0802D25C recheck
                }
                goto hw_section;
env_loop: // 0x0802D25C
                if (ch->f11 != 0)
                    goto tick_once;
                if (chn == 3)
                    ch->f29 |= 1u;
                SoundVoiceClamp((void *)ch);
                goto stage_machine;
hw_section: // 0x0802D362
                if ((ch->f29 & 2u) != 0) {
                    if (chn <= 3 && (ch->f1 & 8u) != 0) {
                        u8 s52 = *(volatile u8 *)0x04000089u; // 0x0802D37A
                        if (s52 <= 63)
                            ch->f32 = (ch->f32 + 2u) & 0x7FCu;
                        else if (s52 <= 127)
                            ch->f32 = (ch->f32 + 1u) & 0x7FEu;
                    }
                    if (chn != 4)
                        *rg.c = (u8)ch->f32; // 0x0802D3AA
                    else
                        *rg.c = (u8)(ch->f32 | (*rg.c & 8u)); // 0x0802D3B4
                    ch->f26 = (u8)((u8)(ch->f26 & 0xC0u) + ((volatile u8 *)ch)[33]); // 0x0802D3C2
                    *rg.d = ch->f26;
                }
                if ((ch->f29 & 1u) != 0) { // 0x0802D3DA
                    volatile u8 *ctl = (volatile u8 *)0x04000081u;
                    *ctl = (u8)((*ctl & (u8)~ch->f28) | ch->f27);
                    if (chn != 3) { // 0x0802D42C square/noise voice
                        u8 e4 = (u8)(env & 15u);
                        env = e4;
                        *rg.b = (u8)((ch->f9 << 4) + e4);
                        *rg.d = (u8)(ch->f26 | 0x80u);
                        if (chn == 1 && (*rg.a & 8u) != 0)
                            *rg.d = (u8)(ch->f26 | 0x80u);
                    } else { // ch3 fallthrough 0x0802D3F6: wave table voice
                        *rg.b = *(volatile u8 *)(uintptr_t)(0x08061744u + ch->f9);
                        if ((ch->f26 & 0x80u) != 0) {
                            *rg.a = 0x80;
                            *rg.d = ch->f26;
                            ch->f26 &= 0x7Fu;
                        }
                    }
                }
                ch->f29 = 0; // 0x0802D460
                continue;
note_off: // 0x0802D20E
                SoundHWMode_CF7C((u32)chn);
                ch->f0 = 0;
                ch->f29 = 0; // falls into the D460 store
            }
        }
    }
}

// --- sound_beb4.s 0x02BF04 walker — bounded group: ldrb +8 count u8,
// ldr +44 array, bit-mask loop with tst 0x80 gate (interior label, no VMA
// alias; exact body stays in asm/sound_beb4.s) ---
void SoundBeb4_ChanWalk(void *state){
    volatile u8 *s = (volatile u8*)state;
    u8 cnt = s[8];
    (void)cnt;
}
#ifndef __APPLE__
void _0802BF04(void *s) __attribute__((alias("SoundBeb4_ChanWalk")));
void sub_0802BF04(void *s) __attribute__((alias("SoundBeb4_ChanWalk")));
#endif

// --- sound_d6f4.s interpreter handler 0x02D728 — owned by
// src/sound_voice_helpers.c as SoundSeqHandler0 (_0802D728); no duplicate
// alias here. ---
