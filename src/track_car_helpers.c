#include "gtadv/car_physics_lane.h"
#include "gba/types.h"

// Remaining owned spans: code_24ac.s (video init), code_24e54.s, code_25264.s,
// code_25930.s, code_26068.s, code_26180.s, code_261d4.s, code_263de/e0.s
// These are video/DMA setup and car collection helpers. Provide behavioral stubs
// with correct alias and minimal side effects; leaves-to-dispatchers order:
// the DMA setup leaves (24ac) are lifted, the collection dispatchers are stubbed.

// 24AC : video/DMA setup leaf 0x024AC, 72 B, pools 0x04000010/0x04000052/0x05000100/0x040000D4/0x84000100
// Exact strh sequence from disassembly 0x080024AC: ldr r0,=0x04000010; movs r1,#0; strh [r0]; adds #2 ×3; adds #10; 128<<1 → 0x0100 strh
void Code24AC_VideoInit(void){
    // Preserve exact offsets and u16 widths (strh)
    *(volatile uint16_t*)0x04000010 = 0;
    *(volatile uint16_t*)0x04000012 = 0;
    *(volatile uint16_t*)0x04000014 = 0;
    *(volatile uint16_t*)0x04000016 = 0;
    *(volatile uint16_t*)0x04000020 = 0x0100; // 128<<1
    *(volatile uint16_t*)0x04000022 = 0;
    *(volatile uint16_t*)0x04000024 = 0;
    *(volatile uint16_t*)0x04000026 = 0x0100;
    *(volatile uint16_t*)0x04000028 = 0;
    *(volatile uint16_t*)0x0400002A = 0;
    *(volatile uint16_t*)0x0400002C = 0;
    *(volatile uint16_t*)0x0400002E = 0;
    *(volatile uint16_t*)0x0400004C = 0; // +30 from 0x2E → 0x4C (adds r0,#30)
    *(volatile uint16_t*)0x04000050 = 0; // +4
    *(volatile uint16_t*)0x04000052 = 15;
    // CpuSet 0 via 0x0802D974: r0=sp, r1=r4=0x02000000, r2=0x05000100.
    // `src` is declared BEFORE `zero` on purpose: it is live across the
    // CpuSet call (dma[0] below), so agbcc materialises it into r4 ahead of
    // the [sp]=0 store, which is the ROM's order at 0x080024F6.
    extern void sub_0802D974(const void*,void*,uint32_t);
    uint32_t src = 0x02000000;   // 128 << 18
    uint32_t zero=0;
    sub_0802D974(&zero, (void*)src, 0x05000100);
    volatile uint32_t *dma = (volatile uint32_t*)0x040000D4;
    dma[0] = src;
    dma[1] = 0x07000000; // 224 << 19 (OAM destination for DMA3)
    dma[2] = 0x84000100;
    (void)dma[2];
}
#ifndef __APPLE__
void _080024AC(void) __attribute__((alias("Code24AC_VideoInit")));
void sub_080024AC(void) __attribute__((alias("Code24AC_VideoInit")));
#endif

void Code2534_ByteCopy(const uint8_t *src, uint8_t *dst, uint32_t len){
    const uint8_t *s = src;
    if (len != 0) {
        do {
            *dst = *s;
            s++;
            dst++;
        } while (--len);
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_08002534(const uint8_t *src, uint8_t *dst, uint32_t len) __attribute__((alias("Code2534_ByteCopy")));
void _08002534(const uint8_t *src, uint8_t *dst, uint32_t len) __attribute__((alias("Code2534_ByteCopy")));
#endif

void Code24E54_Strb(void *base, uint32_t val){
    uintptr_t addr = (uintptr_t)base + 0x030015D8;
    *(volatile uint8_t*)addr = (uint8_t)val;
}
#ifndef __APPLE__
void _08024F1C(void *a, uint32_t b) __attribute__((alias("Code24E54_Strb")));
#endif
// 0x08024F28: ldr r1,=0x030015D8; adds r0,r1; ldrb r0,[r0] (u8)
uint32_t Code24E54_Ldrb(void *base){
    return (uint32_t)*(volatile uint8_t*)((uintptr_t)base + 0x030015D8);
}
#ifndef __APPLE__
uint32_t _08024F28(void *a) __attribute__((alias("Code24E54_Ldrb")));
#endif

// 24E54 : table lookup into the 7-word aggregate at 0x0805FC90.
//
// ROM 0x080024E54..0x080024E77 (40 B) + pool word 0x080024E78 = 0x0805FC90:
//   push {r4,r5,lr} / sub sp,#28 / mov r2,sp / ldr r1,=0x0805FC90
//   ldmia r1!,{r3,r4,r5}; stmia r2!,{r3,r4,r5}      (twice)
//   ldr r1,[r1,#0]; str r1,[r2,#0]                   (28th byte)
//   lsls r0,#16 / asrs r0,#14 / add r0,sp / ldr r0,[r0,#0]
//   add sp,#28 / pop {r4,r5} / pop {r1} / bx r1
//
// The two ldmia/stmia pairs plus the trailing word are a 28-byte *struct*
// assignment, not an array index: the whole object is copied to the frame
// first, and only then is r0 (shifted by the u32 element size) added to sp.
// An array local would instead index the literal address directly, so the
// aggregate type is load-bearing here.
typedef struct { u32 w[7]; } Code24E54_Row;

int Code24E54_Lookup(int idx){
    Code24E54_Row row = *(const Code24E54_Row *)0x0805FC90;
    s16 i = idx;
    return (int)row.w[i];
}
#ifndef __APPLE__
int _080024E54(int idx) __attribute__((alias("Code24E54_Lookup")));
int sub_080024E54(int idx) __attribute__((alias("Code24E54_Lookup")));
#endif
// sub_080024E7C remains TODO here (ip/r8 high regs); sub_080024EFC is C-owned in
// src/runtime_record_helpers.c, not here.

s16 Code26180_Full(int a0, int a1){
    extern int sub_08005758(int);
    u32 ia = (u32)a0 << 16;
    u32 r4 = ia >> 16;
    u32 ta = (u32)a1 << 16;
    u32 r5 = ta >> 16;
    s32 r0 = (s32)ta >> 16;
    s32 ret;
    uintptr_t base;
    u32 off;
    s32 iw;
    volatile u16 *p;
    ret = sub_08005758(r0);
#ifndef __APPLE__
    { extern u8 TrackAwardRecBase[];
      __asm__(".globl TrackAwardRecBase\nTrackAwardRecBase = 0x030015F0\n");
      base = (uintptr_t)TrackAwardRecBase; }
#else
    base = (uintptr_t)0x030015F0u;
#endif
    iw = (s32)r4;
    off = (u32)((iw << 16) >> 13);
    p = (volatile u16 *)(uintptr_t)(off + base);
    p[0] = (u16)ret;
    p[1] = (u16)r5;
    return *(s16 *)p;
}
#ifndef __APPLE__
s16 _080026180(int a,int b) __attribute__((alias("Code26180_Full")));
s16 sub_080026180(int a,int b) __attribute__((alias("Code26180_Full")));
#endif

// What makes this work, all three parts measured:
//   - Hoist the RESULT into a register-pinned local initialised to 0 BEFORE the
//     address computation. That is what keeps agbcc branch-shaped (rather than
//     folding to branchless `neg; orr; lsr`) and drops the frame. Declaring
//     the result local after the loads is what the survey measured, and that is
//     exactly why it generalised to 20/21.
//   - The ROM has TWO pool loads -- `ldr r1,[pc]` = 0x03001780 and
//     `ldr r3,[pc]` = 0x0000057C, added at runtime -- so the address must be
//     spelled as the sum of two absolute asm symbols. A folded `0x030017FC`
//     literal collapses to one pool word: 3/32.
//   - The index add must be written `(uintptr_t)r0 + a`, index FIRST, so the
//     result lands in r0 (`adds r0, r0, r1`). Writing `a + r0` lets agbcc pick
//     r1 as the destination (`adds r1, r1, r0`): 30/32.
int Code26068_Tiny(int r0){
    extern u8 TrackTickWa[];
    extern u8 TrackTickOff57C[];
    __asm__(".globl TrackTickWa\nTrackTickWa = 0x03001780\n");
    __asm__(".globl TrackTickOff57C\nTrackTickOff57C = 0x0000057C\n");
    register int res __asm__("r2") = 0;
    uintptr_t a = (uintptr_t)TrackTickWa + (uintptr_t)TrackTickOff57C;
    uint8_t v = *(volatile uint8_t *)((uintptr_t)r0 + a);
    if (v == 1) {
        res = 1;
    }
    return res;
}
#ifndef __APPLE__
// Entry twins `_08026068` / `sub_08026068` are NOT declared here: they already
// exist as aliases of RaceVM_026068 in src/race_phase_vms.c, which is the C
// owner of asm/code_26068.s. Redeclaring them would be a duplicate-symbol link
// error. This file's `_080026068` spelling does not collide with either.
int _080026068(int r0) __attribute__((alias("Code26068_Tiny")));
#endif

// 0x080261D4 (file offset 0x0261D4), 20 bytes: signed halfword at
// record +2. Sign-extend the index before scaling; a halfword offset wraps.
// The absolute data symbol preserves the pool load before the index shifts.
// Non-volatile signed reads compile to the ROM ldrsh, rather than ldrh plus
// an explicit sign extension. Each body retains its own symbol definition.
s16 Code261D4_Get(s16 idx){
#ifndef __APPLE__
    extern u8 TrackAwardRecBase[];
    __asm__(".globl TrackAwardRecBase\nTrackAwardRecBase = 0x030015F0\n");
    uintptr_t base = (uintptr_t)TrackAwardRecBase;
#else
    uintptr_t base = 0x030015F0u;
#endif
    s32 off = (s32)((u32)idx << 16) >> 13;
    return *(s16 *)(base + off + 2);
}
#ifndef __APPLE__
s16 _0800261D4(s16 idx) __attribute__((alias("Code261D4_Get")));
s16 _080261D4(s16 idx) __attribute__((alias("Code261D4_Get")));
s16 sub_080261D4(s16 idx) __attribute__((alias("Code261D4_Get")));
#endif
// 0x261E4: strh r1,[r0+4] where r0 = 0x030015F0 + s16*8
void Code261D4_Set(s16 idx, uint32_t val){
    volatile uint8_t *base = (volatile uint8_t*)0x030015F0;
    s16 off = idx * 8;
    *(volatile uint16_t*)(base + off + 4) = (uint16_t)val;
}
#ifndef __APPLE__
void _0800261E4(s16 idx, uint32_t val) __attribute__((alias("Code261D4_Set")));
#endif
// 0x26208: ldr r1,=0x030015F0; lsls r0#16 asrs #13; adds r0,r1; ldrsh [r0+4] (s16) — same base+4
s16 Code261D4_Get4(s16 idx){
    volatile uint8_t *base = (volatile uint8_t*)0x030015F0;
    s16 off = idx * 8;
    return *(volatile s16*)(base + off + 4);
}
#ifndef __APPLE__
s16 _080026208(s16 idx) __attribute__((alias("Code261D4_Get4")));
#endif
// 0x2621C: push lr; ldr r2,=0x03001670; str r0,[r2]; str r1,[r0]; bl _08026220 (slot store)
void Code261D4_StoreSlot(void *a0, void *a1){
    *(volatile uintptr_t*)0x03001670u = (uintptr_t)a0;
    *(volatile uintptr_t*)(uintptr_t)a0 = (uintptr_t)a1;
    extern void _08026220(void);
    _08026220();
}
#ifndef __APPLE__
void _08002621C(void *a, void *b) __attribute__((alias("Code261D4_StoreSlot")));
#endif
// 0x26220: ldr r0,=0x03001670; ldr r1,[r0]; ldr r0,[r1]; str r0,[r1,4] (u32 copy)
void _08026220(void){
    uintptr_t p = *(volatile uintptr_t*)0x03001670u;
    *(volatile uintptr_t*)(p + 4) = *(volatile uintptr_t*)p;
}
#ifndef __APPLE__
void Code261D4_CopySlot(void) __attribute__((alias("_08026220")));
#ifndef __APPLE__
void sub_08026220(void) __attribute__((alias("_08026220")));
#endif
#endif
// Remaining larger helpers — 25264 (lineup builder), 25930 (grid scan),
// 263E0 (1.3k dispatcher) — all lifted elsewhere; no stubs here.
// Owners: _08025264 → scene_record_dispatch.c (AiLineupBuilder_25264),
// _08025930 → code_25930.c (Code25930_BuildGrid),
// _080263E0 → sprite_obj_263e0.c (Sprite_BlitTiles).

// 263DE : pad word only — no code

// 0x0802620C — 36B record-base registration ((consolidated/elsewhere); slot 0x03001670,
// the same slot 0x08026230/0x08026220 manage):
//   push {lr}; slot = *(0x03001670); slot[0] = rec; rec[0] = val;
//   bl 0x08026220 (Code261D4_CopySlot: rec[1] = rec[0]); pop; bx.
// sprite_obj_263e0.c calls it as (rec, val) — matches r0/r1 exactly.
void RegisterBase_2620C(void *rec, void *val) {
    volatile void **slot = (volatile void **)0x03001670u;
    *slot = rec;
    *(volatile void **)rec = val;
    _08026220();
}
#ifndef __APPLE__
void _0802620C(void *a, void *b) __attribute__((alias("RegisterBase_2620C")));
void sub_0802620C(void *a, void *b) __attribute__((alias("RegisterBase_2620C")));
#endif

// 0x080261E8 (file offset 0x0261E8), 16 bytes, asm/code_261d4.s.
// The ROM consumes r1 at word width and narrows only at the halfword store:
//   u16[0x030015F0 + idx*8 + 4] = v   (r0<<16>>13 == idx*8, signed lane).
// Callers (race_scene_a1.c:199/203) pass (word, halfword).
void PoolStore_261E8(int idx, int v){
#ifndef __APPLE__
    extern u8 TrackAwardRecBase[];
    __asm__(".globl TrackAwardRecBase\nTrackAwardRecBase = 0x030015F0\n");
    uintptr_t base = (uintptr_t)TrackAwardRecBase;
#else
    uintptr_t base = 0x030015F0u;
#endif
    s32 off = (s32)((u32)idx << 16) >> 13;
    *(volatile u16 *)(base + off + 4) = v;
}
#ifndef __APPLE__
void _080261E8(int a, int b) __attribute__((alias("PoolStore_261E8")));
void sub_080261E8(int a, int b) __attribute__((alias("PoolStore_261E8")));
#endif

// 0x080261F8 (file offset 0x0261F8), 20 bytes, asm/code_261d4.s:
//   return s16[0x030015F0 + idx*8 + 4].
s16 PoolLoad_261F8(int idx){
#ifndef __APPLE__
    extern u8 TrackAwardRecBase[];
    __asm__(".globl TrackAwardRecBase\nTrackAwardRecBase = 0x030015F0\n");
    uintptr_t base = (uintptr_t)TrackAwardRecBase;
#else
    uintptr_t base = 0x030015F0u;
#endif
    s32 off = (s32)((u32)idx << 16) >> 13;
    return *(s16 *)(base + off + 4);
}
#ifndef __APPLE__
s16 _080261F8(int a) __attribute__((alias("PoolLoad_261F8")));
s16 sub_080261F8(int a) __attribute__((alias("PoolLoad_261F8")));
#endif
