#include "gtadv/sound.h"
#include "gba/types.h"

// Sound body complete functions instruction-by-instruction, opaque volatile byte offsets, exact widths.
// No invented relocated pointers, mixer mode boundaries, PSG semantics, or open-bus data.
// Each preserves direct branches (cmp #128 bcs etc.) where compare, field offset (+5/+64/+32 etc.), and both targets are proven.
// Mixed-ISA mixer_2b888.s not edited (sound_mixer.c untouched).

// _0802C390 note-off: push {r4,r5}, ldr r2,[r1+64] u32, ldrb [r2] u8 vs 128, strb +5, walk +32 list via ldr [r1+32] u32
void SoundVoiceFollow_NoteOff(void *chan, void *seq){
    volatile u8 *c = (volatile u8*)chan;
    volatile u8 *s = (volatile u8*)seq;
    (void)s;
    volatile u32 *cursor = (volatile u32*)(c + 64);
    volatile u8 *curPtr = (volatile u8*)(uintptr_t)*cursor;
    u8 v = *curPtr;
    if(v < 128){
        c[5] = v;
        (*cursor)++;
    }
    volatile u32 *head = (volatile u32*)(c + 32);
    volatile u32 *node = (volatile u32*)*head;
    while(node){
        u8 fl = ((volatile u8*)node)[0];
        if((fl & 131)==0 || (fl & 64)!=0){ node = (volatile u32*)node[13]; continue; }
        u8 id = ((volatile u8*)node)[17];
        if(id != v) { node = (volatile u32*)node[13]; continue; }
        ((volatile u8*)node)[0] |= 64;
        break;
    }
}
#ifndef __APPLE__
void _0802C390(void *c, void *s) __attribute__((alias("SoundVoiceFollow_NoteOff")));
void sub_0802C390(void *c, void *s) __attribute__((alias("SoundVoiceFollow_NoteOff")));
#endif

// _0802C780 voice initializer — blocked, exact in src/sound_core.c as _0802C780 (SoundVoiceInit)
// No duplicate alias here.

// _0802CED4 table interpolation — blocked, exact leaf now in src/sound_interp_ced4.c as _0802CED4 (see lane_sound_followup3.md)
// No placeholder alias here to avoid duplicate/misleading; prior stub removed.

// 0x0802CA34 (file 0x02CA34): apply packed sound configuration.
// asm/sound_reset_more.s: the argument is flags, not a state pointer.
extern void Sub_0802CB20(void *s); // sound_core.c SoundCmdCommit (arg is dead in ROM)
extern void _0802C990(u32 rate); // sound_core.c SoundControlInit; 0x0802C990
void SoundResetMore(void *config){
    u32 flags = (u32)(uintptr_t)config;
    volatile u8 *root = (volatile u8 *)(uintptr_t)*(volatile u32 *)0x03007FF0u;
    if (*(volatile u32 *)root != 0x68736D53u) return;
    *(volatile u32 *)root = 0x68736D54u;
    if (flags & 0xFFu) root[5] = (u8)(flags & 0x7Fu);
    if (flags & 0xF00u) {
        root[6] = (u8)((flags & 0xF00u) >> 8);
        for (u32 i = 0; i < 12; ++i) root[80 + i * 64] = 0;
    }
    if (flags & 0xF000u) root[7] = (u8)((flags & 0xF000u) >> 12);
    if (flags & 0xB00000u) {
        volatile u8 *bias = (volatile u8 *)0x04000089u;
        *bias = (u8)((*bias & 63u) | ((flags & 0x300000u) >> 14));
    }
    if (flags & 0xF0000u) {
        Sub_0802CB20((void *)0); // r0 dead in ROM (reloaded from 0x03007FF0)
#ifndef __APPLE__
        // The slice links no C object, so a promoted body may only call a
        // spelling the closure defines. `_0802C990` is aliased in
        // sound_core.c under `#ifndef __APPLE__`, so the host build keeps the
        // friendly name -- an undeclared call is a silent C89 implicit
        // declaration, and a host link with `-undefined dynamic_lookup` binds lazily,
        // so nothing would report it.
        _0802C990(flags & 0xF0000u);
#else
        extern void SoundControlInit(u32 rate); // sound_core.c; host-only spelling
        SoundControlInit(flags & 0xF0000u);
#endif
    }
    *(volatile u32 *)root = 0x68736D53u;
}
#ifndef __APPLE__
void _0802CA34(void *s) __attribute__((alias("SoundResetMore")));
void sub_0802CA34(void *s) __attribute__((alias("SoundResetMore")));
#endif
