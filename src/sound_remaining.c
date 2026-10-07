#include "gtadv/sound.h"
#include "gba/types.h"
#include "gba/regs.h"
#include "gba/bios.h"

// Sound remaining pure-Thumb — instruction-by-instruction lifts for bounded units.
// No placeholder/no-op aliases; mixed-ISA mixer body remains isolated.
// VMA cites per asm/sound_*.s, pools and call sites exact, widths preserved.

// --- sound_d034.s PSG 0x02D034 — blocked, not claimed ---
// 540L with 28 B at sp+4 (+10 countdown u8, +8 envelope, +0 flags) and REG_SOUND pools 0x04000060/61/70/71
// with +0x19 vibrato vs +0x1C sentinel 0x68736D53 overlap not proven beyond pools; exact asm remains
// in asm/sound_d034.s without alias here (see asm/sound_api.s). No placeholder alias.

// --- sound_d6f4.s sequence interpreter 0x02D6F4 — dispatch table 0x0802D724 5 entries ---
// Already aliased in sound_core.c as SoundSeqInterpreter (_0802D6F4); full EWRAM vector beyond dispatch remains TODO per that file.
// No duplicate alias here to avoid Werror on redefinition; this note documents the VMA remains covered by prior stub.

// --- sound_beb4.s full walker 0x02BEB4 — guard + trampoline + 0x50 stride walk beyond minimal ---
// Already aliased in sound_core.c as SoundBeb4Worker (_0802BEB4); full 0x50 stride walk beyond trampoline remains TODO where channel struct opaque.
// No duplicate alias here.

// --- sound_note_on.s 0x02C190, sound_seq B66C, sound_voice apply 0x02C160 ---
// Blocked — free-list search via EWRAM 0x0203E000 +32 head and B66C r8/r9/sl high-register spill not proven beyond pools
// without watch; exact asm remains in asm/sound_note_on.s etc. without alias here (see asm/sound_api.s).
// No placeholder aliases emitted.
