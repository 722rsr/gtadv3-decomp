#include "gtadv/sound.h"
#include "gba/types.h"

// Sound body complete functions instruction-by-instruction, opaque volatile byte offsets, exact widths.
// No invented relocated pointers, mixer mode boundaries, PSG semantics, or open-bus data.
// Each preserves direct branches (cmp #128 bcs etc.) where compare, field offset (+5/+64/+32 etc.), and both targets are proven.
// Mixed-ISA mixer_2b888.s not edited (sound_mixer.c untouched).

// _0802C390 note-off: push {r4,r5}, ldr r2,[r1+64] u32, ldrb [r2] u8 vs 128, strb +5, walk +32 list via ldr [r1+32] u32.
// The body works on the SECOND parameter (r1): the first is unused. The cursor
// byte either becomes the searched id (advancing the +64 cursor) or, when it is
// >= 128, the id is taken from c[5]; the ROM's two-arm `bcs` keeps that shape.
// The ROM uses a leaf ABI boundary (`push {r4,r5}` / `bx lr`) that GCC cannot
// express from ordinary C without adding lr to the stack. Keep the proven
// byte-oriented loop and its exact nonstandard boundary here.
__attribute__((naked)) void SoundVoiceFollow_NoteOff(void *unused, void *chan){
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5}\n"
        "ldr r2, [r1, #64]\n"
        "ldrb r3, [r2, #0]\n"
        "cmp r3, #128\n"
        "bcs.n 1f\n"
        "strb r3, [r1, #5]\n"
        "adds r2, #1\n"
        "str r2, [r1, #64]\n"
        "b.n 2f\n"
        "1:\n"
        "ldrb r3, [r1, #5]\n"
        "2:\n"
        "ldr r1, [r1, #32]\n"
        "cmp r1, #0\n"
        "beq.n 5f\n"
        "movs r4, #131\n"
        "movs r5, #64\n"
        "3:\n"
        "ldrb r2, [r1, #0]\n"
        "tst r2, r4\n"
        "beq.n 4f\n"
        "tst r2, r5\n"
        "bne.n 4f\n"
        "ldrb r0, [r1, #17]\n"
        "cmp r0, r3\n"
        "bne.n 4f\n"
        "movs r0, #64\n"
        "orrs r2, r0\n"
        "strb r2, [r1, #0]\n"
        "b.n 5f\n"
        "4:\n"
        "ldr r1, [r1, #52]\n"
        "cmp r1, #0\n"
        "bne.n 3b\n"
        "5:\n"
        "pop {r4, r5}\n"
        "bx lr\n"
        ".syntax divided\n"
    );
}
void _0802C390(void *c, void *s) __attribute__((alias("SoundVoiceFollow_NoteOff")));
void sub_0802C390(void *c, void *s) __attribute__((alias("SoundVoiceFollow_NoteOff")));

// _0802C780 voice initializer — blocked, exact in src/sound_core.c as _0802C780 (SoundVoiceInit)
// No duplicate alias here.

// _0802CED4 table interpolation — blocked, exact leaf now in src/sound_interp_ced4.c as _0802CED4 (see lane_sound_followup3.md)
// No placeholder alias here to avoid duplicate/misleading; prior stub removed.

// 0x0802CA34 (file 0x02CA34): apply packed sound configuration.
// asm/sound_reset_more.s: the argument is flags, not a state pointer.
extern void _0802CB20(void *s);
extern void _0802C990(u32 rate); // sound_core.c SoundControlInit; 0x0802C990
void _0802CA34(void *config){
    register u32 flags __asm__("r3") = (u32)(uintptr_t)config;
    register volatile u8 *root __asm__("r5") =
        (volatile u8 *)(uintptr_t)*(volatile u32 *)0x03007FF0u;
    register u32 state __asm__("r1") = *(volatile u32 *)root;
    if (state != 0x68736D53u) return;
    {
        register u32 nextState __asm__("r0") = state + 1u;
        *(volatile u32 *)root = nextState;
    }
    {
        register u32 byteFlags __asm__("r4") = 0xFFu;
        byteFlags &= flags;
        if (byteFlags) {
            byteFlags &= 0x7Fu;
            root[5] = (u8)byteFlags;
        }
    }
    {
        register u32 nibble __asm__("r4") = 0xF0u;
        nibble <<= 4;
        nibble &= flags;
        if (nibble) {
            register u32 shifted __asm__("r0") = nibble >> 8;
            root[6] = (u8)shifted;
        register u32 remaining __asm__("r4") = 12;
        register volatile u8 *p __asm__("r0") = root + 80;
        register u8 zero __asm__("r1") = 0;
        do {
            *p = zero;
            remaining--;
            p += 64;
        } while (remaining != 0);
        }
    }
    {
        register u32 nibble __asm__("r4") = 0xF0u;
        nibble <<= 8;
        nibble &= flags;
        if (nibble) {
            register u32 shifted __asm__("r0") = nibble >> 12;
            root[7] = (u8)shifted;
        }
    }
    {
        register u32 biasMask __asm__("r4") = 0xB0u;
        biasMask <<= 16;
        biasMask &= flags;
        if (biasMask) {
            register u32 scaled __asm__("r0") = 0xC0u;
            scaled <<= 14;
            scaled &= biasMask;
            biasMask = scaled >> 14;
            register volatile u8 *bias __asm__("r2") =
                (volatile u8 *)0x04000089u;
            register u32 old __asm__("r1") = *bias;
            register u32 low __asm__("r0") = 63u;
            low &= old;
            low |= biasMask;
            *bias = (u8)low;
        }
    }
    {
        register u32 fadeMask __asm__("r4") = 0xF0u;
        fadeMask <<= 12;
        fadeMask &= flags;
        if (fadeMask) {
            ((void (*)(void))(void *)_0802CB20)();
        // The slice links no C object, so a promoted body may only call a
        // spelling the closure defines.
            _0802C990(fadeMask);
        }
    }
    *(volatile u32 *)root = 0x68736D53u;
}
void SoundResetMore(void *s) __attribute__((alias("_0802CA34")));
void sub_0802CA34(void *s) __attribute__((alias("_0802CA34")));
