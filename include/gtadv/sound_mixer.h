#ifndef GTADV_SOUND_MIXER_H
#define GTADV_SOUND_MIXER_H
#include "gba/types.h"

// Mixer 0x0802B888-0x0802BC28 — five traced regions (tools/build_mixer.py)
// Pools: 0x0805DBF4 etc., 0x03000170 IWRAM dispatch (blocked), 0x04000060 DMA
// Each region preserves exact VMA alias, widths, and bx-rN transitions.
// Only regions with proven ARM veneer/kernel and Thumb dispatch are lifted; IWRAM contract remains blocked.

u32 MixerVeneer_2B888(u32 a0, u32 a1); // ARM 0x02B888-0x02B898 (4 insns, bx lr)
void MixerThumb_2B898(void);            // Thumb 0x02B898-0x02B928 (root guard and callbacks)
void MixerArm_2B928(void *ctx);        // ARM 0x02B928-0x02B968 (32-bit mix kernel, bx r3)
void MixerThumb_2B968(void *ctx);      // Thumb 0x02B968-0x02BA8C (voice dispatch, bx r3)
void MixerArm_2BA8C(void *ctx);        // ARM 0x02BA8C-0x02BC28 (tight mix loop, bx r0 -> 0x02BC29 Thumb)

#endif
