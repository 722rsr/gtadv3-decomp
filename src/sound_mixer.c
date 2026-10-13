#include "gtadv/sound_mixer.h"
#include "gtadv/sound.h"
#include "gba/types.h"

// Mixer 0x0802B888-0x0802BC28 — exact bytes in asm/mixer_2b888.s (pools are
// rendered as round-tripping mnemonics in that listing), epilogue in
// asm/sound_mixer_tail.s, voice-array clear in asm/sound_mixer_leaf.s.
// Pools proven from ROM: root cell 0x03007FF0 @B904, Smsh @B908, IWRAM
// vector 0x03007001 @B90C, VCOUNT 0x04000006 @B910/@B9AC, mix buffer +848
// (0x350) @B914, frame samples 1584 (0x630) @B918.
//
// Dispatch facts: [root+32] is a callback invoked with r0=[root+36], and
// [root+40] a callback invoked with r0=root, both through the shared `bx
// r3` gadget at 0x0802BC46 (the epilogue's return halfword: `bl BC46`
// sets lr, `bx r3` forwards to the vector, the callee returns to the
// mixer). The voice loop calls relocated IWRAM Thumb code at 0x03007001
// (pool @B90C) via `bx r3`. The two ARM islands (mix kernels at 0x02B928
// and 0x02BA8C) stay asm-owned: mixed ISA in one span has no plain-C form,
// and the IWRAM copy-site contract is only proven as the pool word.

// ARM veneer 0x02B888-0x02B898: ldrmi sl,[r0,-r0,lsl#4], umull r2,r3,r0,r1,
// add r0,r3, bx lr. High word of the unsigned product; the conditional
// word load's result is discarded by the ROM (dead timing read).
__attribute__((naked)) u32 MixerVeneer_2B888(u32 a0, u32 a1){
    __asm__ volatile(
        ".arm\n"
        "ldrmi sl, [r0, -r0, lsl #4]\n"
        "umull r2, r3, r0, r1\n"
        "add r0, r3, #0\n"
        "bx lr\n"
        ".thumb\n"
    );
}
u32 _0802B888(u32 a,u32 b) __attribute__((alias("MixerVeneer_2B888")));
u32 sub_0802B888(u32 a,u32 b) __attribute__((alias("MixerVeneer_2B888")));

// Thumb entry 0x02B898-0x02B8AA: Smsh guard. root = *0x03007FF0; if
// *root != Smsh return; *root = Smsh+1 and fall into the setup.
static volatile u8 *MixerEnter(void){
    volatile u8 *root = (volatile u8 *)(uintptr_t)*(volatile u32 *)0x03007FF0u;
    if (*(volatile u32 *)root != SOUND_MAGIC)
        return 0;
    *(volatile u32 *)root = SOUND_MAGIC + 1u;
    return root;
}

// Thumb setup 0x02B8AA-0x02B928 (interior label _0802B8AA, no VMA alias):
// VCOUNT-gated mix bound at [sp,#20] (root[12] + VCOUNT + 228 while
// VCOUNT < 160), the two vector calls above, volume preload into r8,
// mix buffer root+848, voice count root[4]-1, per-voice rate table walk.
// Ends with the IWRAM vector load (pool @B90C) and `bx r3`.
__attribute__((naked)) void MixerThumb_2B898(void){
    __asm__ volatile (
        ".syntax unified\n"
        ".thumb\n"
        "\tldr\tr0, [pc, #104]\t@ (0x802b904)\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tldr\tr2, [pc, #104]\t@ (0x802b908)\n"
        "\tldr\tr3, [r0, #0]\n"
        "\tcmp\tr2, r3\n"
        "\tbeq.n\t.Lsoundb898_equal\n"
        "\tbx\tlr\n"
        ".Lsoundb898_equal:\n"
        "\tadds\tr3, #1\n"
        "\tstr\tr3, [r0, #0]\n"
        ".syntax divided\n"
    );
}
void _0802B898(void) __attribute__((alias("MixerThumb_2B898")));
void sub_0802B898(void) __attribute__((alias("MixerThumb_2B898")));

// Blocked subregions (asm-owned, no alias here):
// - Thumb voice dispatch 0x02B968-0x02BA8C: 0x50-stride voice envelope
//   state machine (voice init/mix-level/select, VCOUNT re-gate @B9AC),
//   ends with adr/bx into the ARM island. Interior labels only.
// - ARM 32-bit mix kernel 0x02B928-0x02B968 (cmp r4,#2 / ldrsb / mul /
//   asr #8 / strb) and ARM tight mix loop 0x02BA8C-0x02BC28 (ldrsb / mul
//   / bic #0xFF0000 / ror / str, IWRAM-fed sample pointers). Mixed ISA,
//   no plain-C transcription; bytes exact in asm/mixer_2b888.s.
// - Epilogue 0x02BC28-0x02BC4C in asm/sound_mixer_tail.s (loop-back to
//   0x02B990 or restore-and-return, Smsh pool @BC48); its `bx r3`
//   halfword at 0x0802BC46 doubles as the vector-call gadget above.
// - Voice-array clear 0x02BC4C-0x02BC62 in asm/sound_mixer_leaf.s,
//   C-owned as SoundClearVoiceArray (src/sound.c).
