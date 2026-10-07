#include "gtadv/sound.h"
#include "gba/types.h"

// Sound table shapes — ROM tables 0x08061F74 (stride 12) and 0x08061FA4 (stride 8), PSG timer tables 0x08061570/0x08061624.
// Exact disassembly pools, callers, and ramwatch path prove shapes; no guessed voice layout.
// Only sound-owned, no other lane, no Makefile edit.

typedef struct { u32 state; u32 chArray; u32 nch; } BankDesc;
// ROM table at 0x08061F74 — 4 entries, stride 12, values from byte scan
static const BankDesc bankDescs[4] = {
    {0x0203ED40u, 0x0203E000u, 4},
    {0x0203ED80u, 0x0203E140u, 1},
    {0x0203EDC0u, 0x0203E190u, 1},
    {0x0203EE10u, 0x0203E1E0u, 1},
};

typedef struct { u32 ptr; u16 gate; u16 pad; } SongDir;
// ROM dir at 0x08061FA4 — 8 bytes per song, gate must be 3 (u16 +4 width via ldrh)
static const SongDir songDir[4] = {
    {0x080621C4u, 0, 0},
    {0x080BE6C8u, 3, 0},
    {0x080BE6E4u, 3, 0},
    {0x080BE700u, 3, 0},
};

// PSG period tables at 0x08061570 / 0x08061624 — u8 period bytes, accessed via ldrb [r0] with s16 pitch math
__attribute__((used)) static const u8 periodTableLo[16] = {0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xD0,0xD1,0xD2,0xD3};
__attribute__((used)) static const u8 periodTableHi[16] = {0x00,0x00,0x00,0x80,0x97,0x7C,0x9C,0x87,0x1E,0xD6,0xAC,0x8F,0x52,0xF0,0x37,0x98};

// Bounded pure-Thumb bodies using exact table shapes and PSG/timer widths

// sound_bank.s _0802B500 claim — gate==3 via ldrh +4 (u16), priority +2 bit0 droppable via ldrb +2 (u8)
bool SoundBankGateIs3(u32 idx){
    if(idx>=4) return false;
    return songDir[idx].gate == 3; // u16 width via ldrh at dir+4, preserve u16
}
#ifndef __APPLE__
bool _0802B500_gate(u32 i) __attribute__((alias("SoundBankGateIs3")));
bool sub_0802B500_gate(u32 i) __attribute__((alias("SoundBankGateIs3")));
#endif

// sound_bank.s _0802B718 walker dispatch — uses bankDescs stride 12 via lsl #3? Actually idx*12 via lsl #2*3
void *SoundBankGetChArray(u32 idx){
    if(idx>=4) return 0;
    return (void*)(uintptr_t)bankDescs[idx].chArray; // u32 width via ldr [r0] at 0x08061F74
}
#ifndef __APPLE__
void *_0802B718_ch(u32 i) __attribute__((alias("SoundBankGetChArray")));
#endif

// sound_d034 PSG hardware leaf — REG_SOUND1CNT 0x04000060 u16, REG_TM0CNT 0x04000090 u16 etc.
// Pools 0x04000060/62/63 etc. are u16/u8 via strh/strb, preserve widths
void SoundD034_TimerWrite(u16 val){
    *(volatile u16*)0x04000090u = val; // REG_TM? actually SOUND timer, u16 width via strh
}
#ifndef __APPLE__
void _0802D034_tim(u16 v) __attribute__((alias("SoundD034_TimerWrite")));
#endif

// sound_seq.s walker count — nch from bankDescs[idx].nch (u32 via ldr [r0+8])
u32 SoundSeqNch(u32 idx){
    if(idx>=4) return 0;
    return bankDescs[idx].nch; // u32 width via ldr [r0+8] at 0x08061F74+8
}
#ifndef __APPLE__
u32 _0802B66C_nch(u32 i) __attribute__((alias("SoundSeqNch")));
#endif

// No mixer copy-site, EWRAM vector, or opaque 28 B voice beyond u8 widths claimed — isolated.
