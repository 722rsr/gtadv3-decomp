#ifndef GTADV_SOUND_H
#define GTADV_SOUND_H

#include "gba/types.h"
#include "gba/regs.h"

// Sound driver lane — EWRAM 0x0203EE50 master block, stream banks 0x0203ED40...
// VMA spans 0x0802B04C-0x0802DDC8 + mixer 0x0802B888-0x0802BC28 (mixed ISA)
// Companion: asm/sound_api.s

#define SOUND_MASTER_ADDR   0x0203EE50u
#define SOUND_MASTER_PTR    0x03001764u
#define SOUND_BANK0_ADDR    0x0203ED40u
#define SOUND_BANK1_ADDR    0x0203ED80u
#define SOUND_BANK2_ADDR    0x0203EDC0u
#define SOUND_BANK3_ADDR    0x0203EE10u
#define SOUND_ROOT_PTR_CELL 0x03007FF0u
#define SOUND_MAGIC         0x68736D53u // "Smsh"

// Master block at 0x0203EE50 — field map verified from sound_api.s
typedef struct {
    u16 fadeTarget;   // +0x00
    u16 fadeCur;      // +0x02
    s16 chanC;        // +0x04
    s16 chanD;        // +0x06
    u8  flags;        // +0x08 bit0 enable, bit2 fadeDown, bit3 fadeUp, bit4 latch, bit5 pauseGate
    u8  masterVol;    // +0x09 0xFF muted
    u8  secVol;       // +0x0A
    u8  _0B;
    u8  fadeStep;     // +0x0C
    u8  songSpeed;    // +0x0D from ROM 0x080613B8
    u8  flags2;       // +0x0E bit6 reapply
    u8  chanCLevel;   // +0x0F
    u8  chanDLevel;   // +0x10
    u8  volChanged;   // +0x11
} SoundMaster;
#define SOUND_MASTER ((volatile SoundMaster *)SOUND_MASTER_ADDR)
#define SOUND_MASTER_PTR_VAL (*(volatile u32 *)SOUND_MASTER_PTR)

// C API — behavioral equivalents, aliases preserve original BL targets.
// Only substantiated pure-Thumb leaves are implemented; mixed-ISA mixer is TODO.

// Driver init + tick entries (sound_api.s 0x02B04C-0x02B488)
void SoundInit(void *state);          // _0802B04C
void SoundVBlank(void);               // _0802B07C -> _0802BE78 pump when enabled
void SoundVCounter(void);             // _0802B098 fade engine + chan apply
void SoundOff(void);                  // _0802B190
void SoundOn(void);                   // _0802B1B8
void SoundSetMasterVol(u32 vol);      // _0802B1E4
void SoundSetMasterVolCond(u16 vol);  // _0802B214
void SoundStopMute(void);             // _0802B234
void SoundPause(void);                // _0802B25C
void SoundResume(void);               // _0802B280
void SoundArmFade(u16 target, u8 step); // _0802B2A4
void SoundFadeOut(u8 step);           // _0802B30C
s16 SoundFadeRemaining(void);         // _0802B330
void _0802B368(u16 vol);              // was SoundDeferredVol; the closure spelling is the definition
void SoundSetSecVol(u16 v);           // _0802B384
void SoundApplySecVol(void);          // _0802B3A4
bool SoundIsBankPlaying(u32 bank);    // _0802B3B8
void SoundSetSongSpeed(u16 a, u16 b); // _0802B418
void SoundReapplySpeedA(void);        // _0802B44C
void SoundReapplySpeedB(void);        // _0802B460
void SoundReapplySpeedC(void);        // _0802B474

// Sample pump + thunk (sound_pump.s / sound_thunk.s)
void SoundPump(void);                 // _0802BE78 DMA1 DirectSound A re-arm
void SoundThunk(void);                // _0802C53C bx mixer's _0802B898

// Pause gates (sound_pause.s)
void SoundPauseGateOff(void *state);         // _0802C488
void SoundPauseGateOn(void *state, u16 v);   // _0802C4A4

// Voice unlink (sound_unlink.s)
void SoundUnlinkVoice(void *chan);    // _0802BC64

// Mixer leaf (sound_mixer_leaf.s) + premix/divmod (sound_premix.s / sound_divmod.s)
void SoundClearVoiceArray(void *base); // _0802BC4C
// (SoundDivMod removed _08002DFE4 returns the remainder in r0 —
// see DivRemU_02DFE4 in src/scene_record_dispatch.c.)

// Extended pure-Thumb leaves (sound_extra.c)
s32 SoundSignedDiv(s32 num, s32 den);           // _0802DE04 __aeabi_idiv
s32 SoundSignedDivCore(s32 num, s32 den);       // _0802DE9C
void SoundStreamAlloc(void *state, void *ch, u32 cnt); // _0802CBBC
void SoundBankVolPan(u32 id, u32 vol, u32 mode); // _0802B488 vol/pan helper part
u8  SoundFetchByte(void *stream);                // _0802BD14/fetch
u32 SoundFetchBE32(void *stream);                // _0802BCE8
void *SoundPremixGetPtr(void *obj);              // _0802B7B4
void *SoundPremixPitch(void *obj, u32 *freq);  // _0802B7B8 non-void: agbcc pops lr into r1

// Voice helpers and PSG functions (sound_voice_helpers.c / sound_psg.c)
void SoundD480_VolScale(void *state, u16 vol); // _0802D480
void SoundVoiceClamp(void *voice);             // _0802CFCC
void SoundVoiceNoteOff(void *chan, void *seq); // _0802C390
s32 SoundMixerVeneer(s32 a, s32 b);           // _0802B88C ARM veneer
// Additional PSG functions
void SoundD034_Chan1(void *state);             // _0802D08E channel 1 hardware regs
void SoundBeb4_ChanWalk(void *state);          // _0802BF04 stride 0x50
void SoundPsgTick(void);                       // readable model of _0802D034 (asm-owned VMA)
// (0x0802D728 handler owned by src/sound_voice_helpers.c as SoundSeqHandler0)

// Table shapes (sound_table.c) — ROM 0x08061F74 stride 12, 0x08061FA4 stride 8, PSG 0x08061570/0x08061624
bool SoundBankGateIs3(u32 idx);               // _0802B500 gate==3 via ldrh +4 u16
void *SoundBankGetChArray(u32 idx);           // bankDescs chArray via ldr +4 u32
void SoundD034_TimerWrite(u16 val);           // REG 0x04000090 u16
u32 SoundSeqNch(u32 idx);                     // nch via ldr +8 u32
// Body complete exact bodies (sound_voice_commands.c)
void SoundVoiceFollow_NoteOff(void *chan, void *seq); // _0802C390
void SoundInit_Voice(void *voice);                   // _0802C780
void SoundInterp(void *a, u8 x, u8 y, u8 z);         // _0802CED4
void SoundResetMore(void *state);                    // _0802CA34

// Weak stubs for bank vol/pan walkers (sound_seq.s) — implemented where substantiated
void SoundVolWalker(u32 id, u32 val, u32 chSel); // _0802B718 via _0802D4A8
void SoundPanWalker(u32 id, u32 val, u32 chSel); // _0802B74C via _0802D510

#endif // GTADV_SOUND_H
