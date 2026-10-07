// ============================================================================
// race_scene_b3.c — C lift of asm/race_scene.s single function:
//
//   0x0801DC8C — record setup driver (phase switcher + lane/award wiring)
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make), lines ~6890-7526 (.type sub_08001DC8C to
//.type sub_08001E1E0).
//
// Shape (prologue `push {r4-r7,lr}` + high-reg save, `sub sp,#16`; only r0
// is live at entry, so the signature is void(void *rec)):
//  - Switch 1 on s16[WA+0xFBC] via table 0x0801DCC0 (r2 = WA base kept for
//    the case arms): case 0 probes a grid cell at
//    WA+0x100A + s16[WA+0xFF8]*2 + s16[WA+0xFF4]*8 (>2 → rec+156 = 1 else 3);
//    cases 1/2/3/5 store {0,2,5,4}; case 7 stores 4/0 on s16[WA+0x1078]
//    == 1/2; cases 4/6 store nothing.
//  - sub_08001D858(rec), then switch 2 on s16[rec+156] via table 0x0801DD7C
//    (sound select via _08002B214; every arm sets r8 = rec+146).
//  - Switch 3 on s16[rec+156] via table 0x0801DE2C: _08007770 template
//    setup (0, 0x083397D8, {4,5,6}, 0, 4, 1); default (>5) skips.
//  - Long setup tail: obj/lane/resource binds (_0800DAB8, _080075E8,
//    _0800798C, _08007A58, _08007614, _08007ABC), IWRAM bump allocs
//    (_0800572C), s8[WA+0x10E5] award-lane seed (r9 = rec+158), key-lane
//    clamp via _08001DC20, _080025BC8 probe, _08001D750 remap +
//    _08001D7B8 solve, _08002124(0x1391), and a final cmp-chain tail on
//    s16[rec+156] (3 → [rec+158] = 1; 2 → _08001DB0C; 5 → _08001DB4C).

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

// External callees (strong lifts or trampolines).
HOST_STUB(void _08001D858(void *rec));                                   // 0x08001D858 record-lane reset
HOST_STUB(int  _08001D750(int a));                                       // 0x08001D750 id remap (s16 result)
HOST_STUB(void _08001D7B8(void *out, int a, int b, int c));              // 0x08001D7B8 paired-lane solve
HOST_STUB(void _08001DB0C(void));                                        // 0x08001DB0C (asm passes rec in dead r0)
HOST_STUB(void _08001DB4C(void *rec));                                   // 0x08001DB4C gate leaf
HOST_STUB(void _08001DC20(void *rec, int x));                            // 0x08001DC20 index clamp + bind
HOST_STUB(void _08002B214(int v));                                       // 0x08002B214 sound wake
HOST_STUB(void _08002124(u16 v));                                        // 0x08002124 BlockB arm
HOST_STUB(void _080025BC8(void *rec, int v));                            // 0x08025BC8 record probe
HOST_STUB(void _080075E8(void *a, int b, int c));                        // 0x080075E8 emit lane
HOST_STUB(void _08007770(int a, void *b, int c, int d, u32 e, u32 f));   // 0x08007770 template setup
HOST_STUB(void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f));    // 0x08007770 exact ROM body
HOST_STUB(void _0800798C(void *a, void *b));                             // 0x0800798C resource setup
HOST_STUB(void _08007A58(void *a));                                      // 0x08007A58 resource init
HOST_STUB(void _08007614(void *a, int b, int c, int d));                 // 0x08007614 lane emit
HOST_STUB(int  _0800572C(int v));                                        // 0x0800572C IWRAM bump alloc
HOST_STUB(void _08007ABC(void *a, void *b, int c));                      // 0x08007ABC resource bind
HOST_STUB(void _0800DAB8(void *a));                                      // 0x0800DAB8 obj init
HOST_STUB(void _0800D77C(void *a, int b, int c));                        // 0x0800D77C slide setup

// ---- shared anchors --------------------------------------------------------
#define WA        0x03001780u
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WA + (o)))
#define WA_U32(o) (*(volatile u32 *)(uintptr_t)(WA + (o)))

// ----------------------------------------------------------------------------
// ---- 0x08001DC8C — record setup driver ----
// Pools: {WA, +0xFBC, table 0x0801DCC0} / {+0xFF8} / {+0x1078} /
//   {table 0x0801DD7C} / {table 0x0801DE2C} / {0x083397D8 x4} /
//   {WA, +0x10E5} / {0x083397D8, 0x08336CA0, WA, +0x10F8, 0x082B7410,
//   0x082D7660, 0x0833D4E0, 0x080CBB7C} / {0x083397D8} / {0x00001391}.
// ----------------------------------------------------------------------------
void _08001DC8C(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *lane156 = rec + 156; // sl: phase word (rec+156)
    volatile u8 *lane158;             // r9: award lane (rec+158)
    volatile u8 *lane146;             // r8: key lane (rec+146)

    // ---- switch 1: s16[WA+0xFBC] via table 0x0801DCC0 ----
    switch (*(volatile s16 *)(uintptr_t)(WA + 0xFBCu)) {
    case 0: {
        // Grid-cell probe: s16[WA+0x100A + s16[WA+0xFF8]*2 + s16[WA+0xFF4]*8]
        s16 qa = *(volatile s16 *)(uintptr_t)(WA + 0xFF8u);
        s16 qb = *(volatile s16 *)(uintptr_t)(WA + 0xFF4u);
        s16 cell = *(volatile s16 *)((uintptr_t)(WA + 0x100Au) + (int)qa * 2 + (int)qb * 8);
        *(volatile u16 *)lane156 = (cell > 2) ? 1 : 3;
        break;
    }
    case 1:
        *(volatile u16 *)lane156 = 0;
        break;
    case 2:
        *(volatile u16 *)lane156 = 2;
        break;
    case 3:
        *(volatile u16 *)lane156 = 5;
        break;
    case 5:
        *(volatile u16 *)lane156 = 4;
        break;
    case 7: {
        s16 w = *(volatile s16 *)(uintptr_t)(WA + 0x1078u);
        if (w == 1)
            *(volatile u16 *)lane156 = 4;
        else if (w == 2)
            *(volatile u16 *)lane156 = 0;
        break;
    }
    case 4:
    case 6:
    default:
        break;
    }

    _08001D858((void *)(uintptr_t)rec);

    // ---- switch 2: s16[rec+156] via table 0x0801DD7C (sound select) ----
    lane146 = rec + 146;
    switch (*(volatile s16 *)lane156) {
    case 0:
    case 2: {
        u16 a = *(volatile u16 *)(rec + 182);
        u16 b = *(volatile u16 *)(rec + 184);
        _08002B214((a == 1 || b == 1) ? 61 : 59);
        break;
    }
    case 3:
        _08002B214((*(volatile u16 *)(rec + 186) == 1) ? 62 : 63);
        break;
    case 1:
    case 4: {
        u16 w = (u16)(*(volatile u16 *)(rec + 146) - 1); // asm masks to u16
        _08002B214((w > 2) ? 63 : 62);
        break;
    }
    default:
        _08002B214(59);
        break;
    }

    // ---- switch 3: s16[rec+156] via table 0x0801DE2C (template setup) ----
    switch (*(volatile s16 *)lane156) {
    case 3:
        Sub_08007770(0, (void *)0x083397D8u, 6, 0, 4, 1);
        break;
    case 2:
        Sub_08007770(0, (void *)0x083397D8u, 5, 0, 4, 1);
        break;
    case 0:
    case 1:
    case 4:
    case 5:
        Sub_08007770(0, (void *)0x083397D8u, 4, 0, 4, 1);
        break;
    default:
        break;
    }

    // ---- setup tail ----
    *(volatile u32 *)(rec + 112) = 6;
    *(volatile u32 *)(rec + 124) = 8;
    *(volatile u32 *)(rec + 136) = 0;
    _0800DAB8((void *)(uintptr_t)(rec + 40));
    _080075E8((void *)0x083397D8u, 0, 3);

    // Award-lane seed from s8[WA+0x10E5]; r9 = rec+158 on every path.
    lane158 = rec + 158;
    {
        s8 v = *(volatile s8 *)(uintptr_t)(WA + 0x10E5u);
        if (v == 1)
            *(volatile u16 *)lane158 = 1;
        else if (v == 2)
            *(volatile u16 *)lane158 = 2; // asm stores r1 (== 2)
    }

    _0800798C((void *)0x083397D8u, (void *)(uintptr_t)rec);
    _08007A58((void *)0x083397D8u);
    _080075E8((void *)0x083397D8u, 2, 4);
    _080075E8((void *)0x083397D8u, 2, 5);
    _080075E8((void *)0x083397D8u, 0, 7);
    _0800798C((void *)0x08336CA0u, (void *)(uintptr_t)(rec + 16));

    *(volatile u32 *)(rec + 292) = (u32)_0800572C(18); // 146*2
    *(volatile u32 *)(rec + 296) = 17;                 // 148*2
    _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 20),
              (void *)(uintptr_t)17u,
              (int)*(volatile u32 *)(rec + 292));

    *(volatile u32 *)(rec + 264) = (u32)_0800572C(22); // 132*2
    *(volatile u32 *)(rec + 268) = (u32)_08001D750((int)WA_U32(0x10F8u)); // 134*2
    {
        u32 out[2];
        _08001D7B8((void *)out, (int)WA_U32(0x10F8u), 112, 104);
        *(volatile u32 *)(rec + 276) = out[0]; // 138*2
        *(volatile u32 *)(rec + 280) = out[1];
    }
    _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 20),
              (void *)(uintptr_t)*(volatile u32 *)(rec + 268),
              (int)*(volatile u32 *)(rec + 264));

    *(volatile u32 *)(rec + 304) = (u32)_0800572C(18); // 152*2
    *(volatile u32 *)(rec + 308) = 11;                 // 154*2 (asm keeps 11 in r5)
    _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 20),
              (void *)(uintptr_t)11u,
              (int)*(volatile u32 *)(rec + 304));

    _0800798C((void *)0x082B7410u, (void *)(uintptr_t)(rec + 24));
    _080075E8((void *)0x082B7410u, 0, 9);
    _080025BC8((void *)(uintptr_t)(rec + 216), 5);
    _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 28),
              (void *)(uintptr_t)*(volatile u32 *)(rec + 220),
              (int)*(volatile u32 *)(rec + 216));

    *(volatile u32 *)(rec + 40) = 0;
    *(volatile u16 *)(rec + 44) = 1;
    _0800D77C((void *)(uintptr_t)(rec + 60), 0, -32);
    _0800D77C((void *)(uintptr_t)(rec + 52), 0, 160);
    *(volatile u16 *)(rec + 48) = 6;
    *(volatile u16 *)(rec + 50) = 5;
    _0800798C((void *)0x082D7660u, (void *)(uintptr_t)(rec + 32));
    *(volatile u32 *)(rec + 252) = (u32)_0800572C(16);
    {
        // asm: ldrh; subs #1; lsls #16; asrs #16 (sign-extended u16-1)
        s16 q = (s16)((int)*(volatile u16 *)lane146 - 1);
        _08001DC20((void *)(uintptr_t)rec, q);
    }
    _0800798C((void *)0x0833D4E0u, (void *)(uintptr_t)(rec + 8));
    *(volatile u32 *)(rec + 192) = (u32)_0800572C(4);
    {
        u16 key = *(volatile u16 *)lane146;
        int t = (int)key - 1;
        u32 w = *(volatile u32 *)((uintptr_t)(0x080CBB7Cu + (u32)(t * 4)));
        u32 slot192 = *(volatile u32 *)(rec + 192);
        *(volatile u32 *)(rec + 196) = w;
        _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 12),
                  (void *)(uintptr_t)w,
                  (int)*(volatile u32 *)(uintptr_t)slot192);
    }
    *(volatile u32 *)(rec + 204) = (u32)_0800572C(2);
    {
        // Key cmp chain on u16[rec+146]: 2 → 10; >2: 3 → 12 else 13;
        // <2: 1 → 11 (asm stores r5 == 11) else 13.
        u16 key = *(volatile u16 *)lane146;
        u32 v;
        if (key == 2)
            v = 10;
        else if (key > 2)
            v = (key == 3) ? 12 : 13;
        else
            v = (key == 1) ? 11 : 13;
        *(volatile u32 *)(rec + 208) = v;
    }
    {
        u32 slot204 = *(volatile u32 *)(rec + 204);
        _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 12),
                  (void *)(uintptr_t)*(volatile u32 *)(rec + 208),
                  (int)*(volatile u32 *)(uintptr_t)slot204);
    }

    // rec+158 branch: 2 → 8-variant; 1 and else share the 9-variant body.
    {
        s16 w = *(volatile s16 *)lane158; // r9
        u32 id;
        if (w == 2) {
            _08007614((void *)0x083397D8u, 3, 1, 8);
            *(volatile u32 *)(rec + 228) = (u32)_0800572C(80);
            *(volatile u32 *)(rec + 232) = 8;
            id = 8u;
        } else {
            _08007614((void *)0x083397D8u, 3, 0, 8);
            *(volatile u32 *)(rec + 228) = (u32)_0800572C(80);
            *(volatile u32 *)(rec + 232) = 9;
            id = 9u;
        }
        {
            u32 slot228 = *(volatile u32 *)(rec + 228);
            _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 12),
                      (void *)(uintptr_t)id,
                      (int)*(volatile u32 *)(uintptr_t)slot228);
        }
    }

    *(volatile u32 *)(rec + 240) = (u32)_0800572C(28);
    *(volatile u32 *)(rec + 244) = 14;
    {
        u32 slot240 = *(volatile u32 *)(rec + 240);
        _08007ABC((void *)(uintptr_t)*(volatile u32 *)(rec + 12),
                  (void *)(uintptr_t)14u,
                  (int)*(volatile u32 *)(uintptr_t)slot240);
    }
    _08002124(0x1391);
    *(volatile u16 *)(rec + 168) = 0;
    *(volatile u16 *)(rec + 170) = 0;

    // Final cmp-chain tail on s16[rec+156] (no table in asm).
    {
        s16 w = *(volatile s16 *)lane156;
        if (w == 3)
            *(volatile u16 *)lane158 = 1;
        else if (w == 2)
            _08001DB0C(); // NOTE: asm passes rec in r0, but the callee overwrites r0 first (dead)
        else if (w == 5)
            _08001DB4C((void *)(uintptr_t)rec);
    }
}
