#include "gtadv/sound.h"
#include "gba/types.h"
#include "gba/bios.h"

// Residual pure-Thumb instruction-backed leaves/bodies with exact pools/offsets/widths.
// No invented PSG +0x19/+0x1C 28 B overlap, BE32/DMA, mixer mode/copy, open-bus, allocation/Smsh, or voice layouts beyond proven u8/u16.
// Uses proven 0x08061F74 (stride12 table) / 0x08061FA4 (stride8 dir) pools only where ldr [pc] proves them.
// Opaque volatile byte offsets, exact u8/u16/u32 widths via ldrb/ldrh/ldr/strb/strh, lsls #16/lsrs #16 for u16 s16, direct branches bne/beq/cmp, helper ABIs bl sub_0802D4A8/D510/D584 etc.
// Preserve prior aliases; claim only complete exact CFGs via strong alias; no placeholders/weak.

extern void sub_0802D4A8(void *state, u32 chsel, u32 wid);
extern void sub_0802D510(void *state, u32 chsel, u32 wid);
extern void sub_0802D584(void *state, u32 chsel, u32 wid);
extern void sub_0802D974(const void *src, void *dst, u32 ctrl);
extern void sub_0802C8C4(void *a);
extern void sub_0802C780(void *a);
extern void sub_0802CA34(void *a);
extern void sub_0802CBBC(void *dst, void *src, u32 size);

// --- _0802B718(id,val,chsel) — VOLUME walker -> sub_0802D4A8, pools 0x08061F74/0x08061FA4, stride8 dir +12 stride table ---
// Exact: lsls r0,#16; ldr r4,=0x08061F74; ldr r3,=0x08061FA4; lsrs r0,#13 (id*8); adds r0,r3; ldrh r5,[r0+4] gate; lsls r3 = gate*12; adds r3,r4; ldr r0,[r3] state; bl D4A8
void SoundSeq_VolumeWalker(u32 id, u32 val, u32 chsel){
    volatile u32 *tab = (volatile u32*)0x08061F74u;
    volatile u32 *dir = (volatile u32*)0x08061FA4u;
    (void)tab; (void)dir;
    u32 id8 = (id & 0xFFFFu) * 8u; // lsls #16 / lsrs #13 => id*8 via (id<<16)>>13, preserve u16 width
    volatile u8 *e = (volatile u8*)dir + id8;
    u16 gate = *(volatile u16*)(e + 4); // ldrh [r0+4] u16 gate idx
    u32 off12 = (u32)gate * 12u; // lsls #1 + add + lsls #2 => gate*12 u32
    volatile u8 *rec = (volatile u8*)tab + off12;
    void *state = *(void**)rec; // ldr [r3] u32 state block 0x0203ED40 etc. via ldr [r0] u32
    // helper ABI: r0=state, r1=val u16, r2=chsel u16; preserve u16 widths via lsls/lsrs #16
    u32 v16 = val & 0xFFFFu;
    u32 c16 = chsel & 0xFFFFu;
    sub_0802D4A8(state, c16, v16);
}
#ifndef __APPLE__
void _0802B718(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_VolumeWalker")));
void sub_0802B718(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_VolumeWalker")));
#endif

// --- _0802B74C(id,val,chsel) — PAN walker -> sub_0802D510, same pools, s16 val via asrs ---
void SoundSeq_PanWalker(u32 id, u32 val, u32 chsel){
    volatile u32 *tab = (volatile u32*)0x08061F74u;
    volatile u32 *dir = (volatile u32*)0x08061FA4u;
    u32 id8 = (id & 0xFFFFu) * 8u;
    volatile u8 *e = (volatile u8*)dir + id8;
    u16 gate = *(volatile u16*)(e + 4);
    u32 off12 = (u32)gate * 12u;
    volatile u8 *rec = (volatile u8*)tab + off12;
    void *state = *(void**)rec;
    u32 c16 = chsel & 0xFFFFu;
    s16 sval = (s16)(val & 0xFFFFu); // asrs #16 for s16
    (void)sval;
    sub_0802D510(state, c16, (u32)(u16)sval);
}
#ifndef __APPLE__
void _0802B74C(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_PanWalker")));
void sub_0802B74C(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_PanWalker")));
#endif

// --- _0802B780(id,val,chsel) — third-op walker -> sub_0802D584, s8 val via lsls #24/asrs #24 ---
void SoundSeq_ThirdWalker(u32 id, u32 val, u32 chsel){
    volatile u32 *tab = (volatile u32*)0x08061F74u;
    volatile u32 *dir = (volatile u32*)0x08061FA4u;
    u32 id8 = (id & 0xFFFFu) * 8u;
    volatile u8 *e = (volatile u8*)dir + id8;
    u16 gate = *(volatile u16*)(e + 4);
    u32 off12 = (u32)gate * 12u;
    volatile u8 *rec = (volatile u8*)tab + off12;
    void *state = *(void**)rec;
    u32 c16 = chsel & 0xFFFFu;
    s8 s8v = (s8)(val & 0xFFu); // lsls #24 / asrs #24
    sub_0802D584(state, c16, (u32)(u8)s8v);
}
#ifndef __APPLE__
void _0802B780(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_ThirdWalker")));
void sub_0802B780(u32 a, u32 b, u32 c) __attribute__((alias("SoundSeq_ThirdWalker")));
#endif

// --- _0802C4C4 — driver-wide reset/init, pools 0x0802B91D/0x03007000/0x040000E0/0x0203E230/0x0203EC40/0x0093F800/4/0x08061F74/0x0203EE00 ---
// Exact: push {r4-r6,lr}; ldr r0,[pc+#80]=0x0802B91D & -2 =>0x0802B91C; ldr r1=0x03007000; ldr r2=0x040000E0; bl D974; ldr r0=0x0203E230 bl C8C4; ldr r0=0x0203EC40 bl C780; ldr r0=0x0093F800 bl CA34; ldr r0=4 count; ldr r5=0x08061F74 tab; loop 12 B rec: ldr dst/src, ldrb size, bl CBBC, ldrh [rec+10] strb [dst+11], ldr 0x0203EE00 str [dst+24]
void SoundReset_DriverInit(void){
    const void *srcA = (const void*)0x0802B91Cu; // ldr [pc] 0x0802B91D ands -2 => 0x0802B91C
    void *dstI = (void*)0x03007000u; // ldr [pc] 0x03007000 pool at 0x02C51C
    u32 ctrl = 0x040000E0u; // ldr [pc] 0x040000E0 via ldr [pc+#76] at 0x02C520, u32 mode
    sub_0802D974((const void*)(uintptr_t)srcA, dstI, ctrl);
    sub_0802C8C4((void*)0x0203E230u);
    sub_0802C780((void*)0x0203EC40u);
    sub_0802CA34((void*)0x0093F800u);
    volatile u32 *cntp = (volatile u32*)0x08061F74u; // tab base pool at 0x02C534, not cnt; cnt at 0x02C530 =4
    (void)cntp;
    u32 cnt = 4u; // ldr [pc] 0x02C530 lsls #16/lsrs #16 => 4, cmp #0 beq, u16 width via lsls/lsrs
    if(cnt==0) return;
    volatile u8 *tab = (volatile u8*)0x08061F74u; // ldr [pc] 0x02C534
    const u32 stampAddr = 0x0203EE00u; // ldr [pc] 0x02C538 => 0x0203EE00u str [r4+24]
    for(u32 i=0;i<cnt;i++){
        volatile u8 *rec = tab + i*12u; // adds r5,#12
        void *dst = *(void**)rec; // ldr [r5] u32 dst
        void *src = *(void**)(rec+4); // ldr [r5+4] u32 src
        u32 sz = *(volatile u8*)(rec+8); // ldrb [r5+8] u8 size
        sub_0802CBBC(dst, src, sz);
        u16 hw = *(volatile u16*)(rec+10); // ldrh [r5+10] u16 hw
        *(volatile u8*)((volatile u8*)dst + 11) = (u8)hw; // strb [r4+11] low byte of halfword, preserve hw>>8? exact is strb r0,[r4+11] where r0=ldrh
        *(volatile u32*)((volatile u8*)dst + 24) = stampAddr; // ldr [pc] 0x0203EE00 str [r4+24] u32
    }
}
#ifndef __APPLE__
void _0802C4C4(void) __attribute__((alias("SoundReset_DriverInit")));
void sub_0802C4C4(void) __attribute__((alias("SoundReset_DriverInit")));
#endif
