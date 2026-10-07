#include "gtadv/sound.h"
#include "gba/types.h"


extern void sub_0802D4A8(void *state, u32 chsel, u32 wid);
extern void sub_0802D510(void *state, u32 chsel, u32 wid);
extern void sub_0802D584(void *state, u32 chsel, u32 wid);

// --- _0802B66C(id, selmask, w_pan, w_vol, w_x) — combined multi-op applier, VMA 0x0802B66C–0x0802B700 ---
// Exact: push {r4-r7,lr} + r8/r9/sl spill, ldr r4,[sp+32] w_x, lsls/lsrs #16 for id/selmask/w_vol u16, asrs #16 for s16 w_pan,
// cmp #0 beq, then each non-zero section: ldr r2=0x08061F74, ldr r0=0x08061FA4, lsls r1=id*8, adds, ldrh gate [r1+4] u16, gate*12, adds, ldr state [r0], bl helper
// Pools at 0x02B6BC 0x08061F74 and 0x02B6C0 0x08061FA4 proven via ldr [pc]; helper ABIs r0=state r1=selmask r2=wid preserved with u16/s16 widths.
void SoundB66C_Applier(u32 id, u32 selmask, u32 w_pan, u32 w_vol, u32 w_x){
    // Preserve stack arg for w_x via volatile sp+32 access — caller passes 5th arg on stack as u32
    // Widths: id u16 via lsls #16/lsrs #16, selmask u16, w_vol u16, w_pan s16 via asrs #16, w_x u16 via lsls #16/lsrs #16 then s8 via lsls #24/asrs #24
    u32 id_u16 = id & 0xFFFFu;         // lsls #16 / lsrs #16 at 0x02B678/0x02B67A
    u32 sel_u16 = selmask & 0xFFFFu;   // lsls #16 / lsrs #16 at 0x02B67E/0x02B680
    u32 vol_u16 = w_vol & 0xFFFFu;     // lsls #16 / lsrs #16 at 0x02B684/0x02B686 -> sl
    s16 pan_s16 = (s16)(w_pan & 0xFFFFu); // lsls #16 / asrs #16 at 0x02B68E/0x02B690, cmp r3,#0 at 0x02B692
    u32 x_u16 = w_x & 0xFFFFu;         // ldr [sp+32] at 0x02B676 + lsls #16/lsrs #16 at 0x02B68A/0x02B68C, cmp #0 at 0x02B6DC
    // Branch 1: w_pan s16 !=0 -> D510 (cmp r3,#0 beq at 0x02B694)
    if(pan_s16 != 0){
        // Pools at 0x0802B710=0x08061F74 and 0x0802B714=0x08061FA4 via ldr [pc] at 0x02B696/0x02B698 — volatile pool words
        volatile u32 *poolTab = (volatile u32*)0x0802B710u;
        volatile u32 *poolDir = (volatile u32*)0x0802B714u;
        volatile u32 *tab = (volatile u32*)(uintptr_t)*poolTab; // ldr r2,[pc] pool load
        volatile u32 *dir = (volatile u32*)(uintptr_t)*poolDir;
        u32 id8 = id_u16 * 8u; // lsls r1,r5,#3 at 0x02B69A
        volatile u8 *e = (volatile u8*)dir + id8; // adds r1,r0 at 0x02B69C
        u16 gate = *(volatile u16*)(e + 4); // ldrh [r1+4] at 0x02B69E, volatile
        u32 off12 = (u32)gate * 12u; // lsls #1 / adds / lsls #2 at 0x02B6A0/0x02B6A2/0x02B6A6, gate*12
        volatile u8 *rec = (volatile u8*)tab + off12; // adds r0,r2
        void *state = (void*)(uintptr_t)*(volatile u32*)rec; // ldr [r0] volatile state load at 0x02B6AA
        sub_0802D510(state, sel_u16, (u32)(s32)pan_s16); // r2=pan s16 sign-extended via asrs, preserve s16 width
    }
    // Branch 2: w_vol u16 !=0x100 -> D4A8 (lsls r3,#16 cmp 0x01000000 beq at 0x02B6B8/0x02B6BE)
    if(vol_u16 != 0x0100u){
        volatile u32 *poolTab = (volatile u32*)0x0802B710u;
        volatile u32 *poolDir = (volatile u32*)0x0802B714u;
        volatile u32 *tab = (volatile u32*)(uintptr_t)*poolTab;
        volatile u32 *dir = (volatile u32*)(uintptr_t)*poolDir;
        u32 id8 = id_u16 * 8u;
        volatile u8 *e = (volatile u8*)dir + id8;
        u16 gate = *(volatile u16*)(e + 4);
        u32 off12 = (u32)gate * 12u;
        volatile u8 *rec = (volatile u8*)tab + off12;
        void *state = (void*)(uintptr_t)*(volatile u32*)rec;
        u32 vol_arg = vol_u16; // lsrs r2,r3,#16 at 0x02B6D6, zero-extended u16
        sub_0802D4A8(state, sel_u16, vol_arg);
    }
    // Branch 3: w_x u16 !=0 -> D584  s8 via lsls #24/asrs #24 at 0x02B6F8/0x02B6FA, cmp r4,#0 beq at 0x02B6DE
    if(x_u16 != 0){
        volatile u32 *poolTab = (volatile u32*)0x0802B710u;
        volatile u32 *poolDir = (volatile u32*)0x0802B714u;
        volatile u32 *tab = (volatile u32*)(uintptr_t)*poolTab;
        volatile u32 *dir = (volatile u32*)(uintptr_t)*poolDir;
        u32 id8 = id_u16 * 8u;
        volatile u8 *e = (volatile u8*)dir + id8;
        u16 gate = *(volatile u16*)(e + 4);
        u32 off12 = (u32)gate * 12u;
        volatile u8 *rec = (volatile u8*)tab + off12;
        void *state = (void*)(uintptr_t)*(volatile u32*)rec;
        s8 sx = (s8)(x_u16 & 0xFFu); // low byte only, but asm does lsls #24/asrs #24 on full w_x u16 low byte sign-extended
        // Actually asm: lsls r2,r4,#24; asrs r2,#24 -> s8 of low byte of w_x (u16 low 8 bits sign-extended)
        s32 sx32 = (s32)sx; // sign-extended s8
        sub_0802D584(state, sel_u16, (u32)sx32);
    }
}
#ifndef __APPLE__
void _0802B66C(u32 a, u32 b, u32 c, u32 d, u32 e) __attribute__((alias("SoundB66C_Applier")));
void sub_0802B66C(u32 a, u32 b, u32 c, u32 d, u32 e) __attribute__((alias("SoundB66C_Applier")));
#endif
