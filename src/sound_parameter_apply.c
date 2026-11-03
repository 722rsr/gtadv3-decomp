#include "gtadv/sound.h"
#include "gba/types.h"


extern void sub_0802D4A8(void *state, u32 chsel, u32 wid);
extern void sub_0802D510(void *state, u32 chsel, u32 wid);
extern void sub_0802D584(void *state, u32 chsel, u32 wid);

// --- _0802B66C(id, selmask, w_pan, w_vol, w_x) — combined multi-op applier, VMA 0x0802B66C–0x0802B700 ---
// Exact in C (172/172, calls resolve). Three blocks share the same shape:
//   state_table[gate_table[id].gate].state, then call(state, selmask, param).
// The two table bases are pinned r0/r2 per block and held by an empty "+r"
// barrier, otherwise the constant initialisers are rematerialised at each use
// and block 1 collapses to a reload after the multiply. A block-scoped
// `u32 s = (u16)vol << 16` is what makes the volume guard compare the shifted
// pair (`lsls r3,#16` / `cmp r3, 0x1000000`); passing `s >> 16` then reuses the
// same register with `lsrs`. Both later blocks stage their call arguments in
// ROM order (sellmask before the converted wid) with an explicit local and an
// empty "+r" barrier on the first, which is what keeps `adds r1,r6,#0` ahead of
// `lsrs r2,r3,#16`.
void SoundB66C_Applier(u32 id, u32 selmask, u32 w_pan, u32 w_vol, u32 w_x) {
    register u32 idv __asm__("r8");
    register u32 sel __asm__("r9");
    register u32 vol __asm__("sl");
    register u32 xv __asm__("r4");
    register s32 pan __asm__("r3");
    u32 idl, sell;

    idl = (u16)id;
    idv = idl;
    sell = (u16)selmask;
    sel = sell;
    vol = (u16)w_vol;
    xv = (u16)w_x;
    pan = (s16)w_pan;

    if (pan != 0) {
        register u8 *gb __asm__("r0");
        register u8 *sb __asm__("r2");
        u16 gate;
        void *state;
        sb = (u8 *)(uintptr_t)0x08061F74u;
        gb = (u8 *)(uintptr_t)0x08061FA4u;
        __asm__ volatile ("" : "+r"(sb), "+r"(gb));
        gate = *(u16 *)(gb + idl * 8 + 4);
        state = *(void **)(sb + gate * 12);
        sub_0802D510(state, sell, (u32)pan);
    }
    {
    u32 s;
    s = (u32)(u16)vol << 16;
    if (s != 0x1000000u) {
        register u8 *gb __asm__("r0");
        register u8 *sb __asm__("r2");
        u16 gate;
        void *state;
        u32 sa, wa;
        sb = (u8 *)(uintptr_t)0x08061F74u;
        gb = (u8 *)(uintptr_t)0x08061FA4u;
        __asm__ volatile ("" : "+r"(sb), "+r"(gb));
        gate = *(u16 *)(gb + idl * 8 + 4);
        state = *(void **)(sb + gate * 12);
        sa = sell;
        __asm__ volatile ("" : "+r"(sa));
        wa = s >> 16;
        sub_0802D4A8(state, sa, wa);
    }
    }
    if (xv != 0u) {
        register u8 *gb __asm__("r0");
        register u8 *sb __asm__("r2");
        u16 gate;
        void *state;
        u32 sa, xa;
        sb = (u8 *)(uintptr_t)0x08061F74u;
        gb = (u8 *)(uintptr_t)0x08061FA4u;
        __asm__ volatile ("" : "+r"(sb), "+r"(gb));
        gate = *(u16 *)(gb + idv * 8 + 4);
        state = *(void **)(sb + gate * 12);
        sa = sel;
        xa = (u32)(s8)xv;
        sub_0802D584(state, sa, xa);
    }
}

void _0802B66C(u32 a, u32 b, u32 c, u32 d, u32 e) __attribute__((alias("SoundB66C_Applier")));
void sub_0802B66C(u32 a, u32 b, u32 c, u32 d, u32 e) __attribute__((alias("SoundB66C_Applier")));
