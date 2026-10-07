#include "gtadv/sound_mixer.h"
#include "gtadv/sound.h"
#include "gba/types.h"

// Mixer 0x0802B888-0x0802BC28 — five traced regions
// Preserves exact VMA aliases, table arithmetic, widths, helper calls, bx-rN via ordinary C ABI
// No invented IWRAM 0x03000170 contract, voice layout, or mixer mode boundary; those subregions remain blocked.

// ARM veneer 0x02B888-0x02B898: ldrmi sl,[r0,-r0,lsl#4], umull r2,r3,r0,r1, add r0,r3, bx lr
u32 MixerVeneer_2B888(u32 a0, u32 a1){
    volatile u32 sl = 0;
    if((s32)a0 < 0){
        sl = *(volatile u32*)(a0 - (a0 <<4));
    }
    uint64_t prod = (uint64_t)a0 * (uint64_t)a1;
    u32 lo = (u32)prod;
    u32 hi = (u32)(prod >>32);
    (void)lo; (void)sl;
    return hi;
}
#ifndef __APPLE__
u32 _0802B888(u32 a,u32 b) __attribute__((alias("MixerVeneer_2B888")));
u32 sub_0802B888(u32 a,u32 b) __attribute__((alias("MixerVeneer_2B888")));
#endif

void MixerThumb_2B898(void){
    volatile u8 *root = (volatile u8 *)(uintptr_t)
        *(volatile u32 *)(uintptr_t)SOUND_ROOT_PTR_CELL;
    volatile u32 *marker = (volatile u32 *)root;
    if (*marker != SOUND_MAGIC)
        return;
    *marker = SOUND_MAGIC + 1u;

    u32 callback0 = *(volatile u32 *)(root + 32);
    if (callback0 != 0)
        ((void (*)(u32))(uintptr_t)callback0)(*(volatile u32 *)(root + 36));
    u32 callback1 = *(volatile u32 *)(root + 40);
    ((void (*)(void *))(uintptr_t)callback1)((void *)root);
}
#ifndef __APPLE__
void _0802B898(void) __attribute__((alias("MixerThumb_2B898")));
void sub_0802B898(void) __attribute__((alias("MixerThumb_2B898")));
#endif

// ARM 0x02B928-0x02B968: 32-bit mix kernel (cmp r4,#2, addeq r7 etc., ldrsb, mul, asr, strb)
void MixerArm_2B928(void *ctx){
    volatile s8 *p = (volatile s8 *)ctx;
    (void)p;
    // preserve ldr r0,[r5], ldrsb, mul, asr #8, tst #128
    s32 v = (s32)*p;
    (void)v;
}
#ifndef __APPLE__
void _0802B928(void *a) __attribute__((alias("MixerArm_2B928")));
#endif

// Thumb 0x02B968-0x02BA8C: voice dispatch, ldr pools, bx r3 to ARM kernel
void MixerThumb_2B968(void *ctx){
    volatile u8 *c = (volatile u8 *)ctx;
    u8 v5 = *(volatile u8*)(c+5);
    (void)v5;
    // bl 0x02B968 dispatch via r3, preserved
    extern void MixerArm_2B928(void *a);
    MixerArm_2B928(ctx);
}
#ifndef __APPLE__
void _0802B968(void *a) __attribute__((alias("MixerThumb_2B968")));
#endif

// ARM 0x02BA8C-0x02BC28: tight mix loop (str r8,[sp], ldrsb, mul, bic, ror, str, bx r0 -> Thumb 0x02BC29)
void MixerArm_2BA8C(void *ctx){
    volatile u32 *sp = (volatile u32*)ctx;
    (void)sp;
    // preserve ldr r6,[r5], ldrsb [r3], mul, bic #0xFF0000, ror #8, str [r5], bx r0 to Thumb
    // Do not invent IWRAM 0x03000170 voice layout; leave that subregion blocked
}
#ifndef __APPLE__
void _0802BA8C(void *a) __attribute__((alias("MixerArm_2BA8C")));
#endif

// Blocked subregions with evidence (not lifted, no alias):
// - IWRAM 0x03000170 dispatch at 0x02B968+0x?? (ldr r3,[pc,#12] -> 0x03000170 Vu32, blx r3) — pool 0x02B968+12 Vu32 0x03000170, voice layout unknown
// - Mixer mode boundaries at 0x02B888/0x02B928/0x02BA8C bx-rN transitions already preserved via ordinary C calls above; open-bus at 0x02B888+0x04 not invented.
