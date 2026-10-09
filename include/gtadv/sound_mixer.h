#ifndef GTADV_SOUND_MIXER_H
#define GTADV_SOUND_MIXER_H
#include "gba/types.h"

// Mixer 0x0802B888-0x0802BC28 — exact bytes in asm/mixer_2b888.s (+ tail /
// leaf files). Pools: root cell 0x03007FF0 @B904, Smsh @B908, IWRAM vector
// 0x03007001 @B90C, VCOUNT 0x04000006 @B910/@B9AC, mix buffer +848 @B914,
// frame 1584 @B918. Vectors [root+32](arg [root+36]) and [root+40](arg
// root) run through the shared `bx r3` gadget at 0x0802BC46. The ARM mix
// kernels and the IWRAM-fed voice loop stay asm-owned (mixed ISA).

u32 MixerVeneer_2B888(u32 a0, u32 a1); // ARM 0x02B888-0x02B898: umull high word
void MixerThumb_2B898(void);          // Thumb 0x02B898+: Smsh guard + vector calls

#endif
