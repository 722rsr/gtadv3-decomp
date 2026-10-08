// ============================================================================
// menu_d280.c — C lift of asm/menu_d280.s (VMA 0x0800D280–0x0800D3A4)
// remaining three functions: 0x0800D298 / 0x0800D2AC / 0x0800D2B0.
//
// Menu init helpers + garage grid renderer. Transcribed
// instruction-for-instruction from the cited asm listing.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void  _08002124(u16 v) { (void)v; }
__attribute__((weak)) void  _08003940(int a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void  _08003F18(u32 a, u32 b, const volatile u8 *c, int d) { (void)a; (void)b; (void)c; (void)d; }
__attribute__((weak)) void  _080038A4(int a, int b, void *c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _08003978(int a, u32 b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) u16   _08002044(void) { return 0; }
__attribute__((weak)) u16   _08001F8C(void) { return 0; }
__attribute__((weak)) u32   _08002140(void) { return 0; }
__attribute__((weak)) u32   _08001E14(int a) { (void)a; return 0; }
__attribute__((weak)) u16   _0800206C(int a) { (void)a; return 0; }
__attribute__((weak)) u16   _08001E5C(int a, int b) { (void)a; return 0; }
__attribute__((weak)) u16   _08002178(int a) { (void)a; return 0; }
#else
extern void _08002124(u16 v);   // 0x08002124 block-B arm (src/block_b.c)
extern int  _0800254C(void);    // 0x0800254C screen-tile init; takes no argument
extern void _08003940(int a, u32 b);   // 0x08003940
extern void _08003F18(u32 a, u32 b, const volatile u8 *c, int d); // 0x08003F18 text lane
extern void _080038A4(int a, int b, void *c); // 0x080038A4 frame draw
extern void _08003978(int a, u32 b, int c);   // 0x08003978 sprite space
extern u16  _08002044(void);    // 0x08002044 (src/idle_accessors.c)
extern u16  _08001F8C(void);    // 0x08001F8C record count
extern u32  _08002140(void);    // 0x08002140 block-B match
extern u32  _08001E14(int a);   // 0x08001E14 descriptor word
extern u16  _0800206C(int a);   // 0x0800206C record filtered
extern u16  _08001E5C(int a, int b); // 0x08001E5C record halfword
extern u16  _08002178(int a);   // 0x08002178 block-B read (1-arg)
#endif

// ----------------------------------------------------------------------------
// 0x0800D298 sub_0800D298 — menu init: _08002124(0x1398); _0800254C
// ----------------------------------------------------------------------------
void _0800D298(void *rec)
{
    (void)rec;
    _08002124(0x1398);
    _0800254C();
}

// ----------------------------------------------------------------------------
// 0x0800D2AC sub_0800D2AC — no-op
// ----------------------------------------------------------------------------
void _0800D2AC(void *rec)
{
    (void)rec;
}
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// 0x0800D2B0 sub_0800D2B0(rec) — garage grid renderer:
//   r6=rec
//   _08003940(4, 0x0805F8B0)
//   _08003F18(40, 20, 0x0805F8C0, r3=_08002044)
//   _08003F18(40, 30, 0x0805F8D0, r3=_08001F8C)
//   _08003F18(40, 40, 0x0805F8E0, r3=_08002140)
//   _08003F18(40, 50, 0x0805F8F0, r3=_08001E14(0))
//   for r5 in 0..3:
//     r4 = r5*50 + 50
//     _08003978(r4, 60, (u16)_0800206C(r5))
//     _08003978(r4, 70, (u16)_08001E5C(r5, 0))
//     _08003978(r4, 80, (u16)_08002178(r5))
//   _080038A4(40, 130, 0x0805F8F4)
//   _080038A4(40, 140, 0x0805F8FC)
//   r0 = u16[rec+6]; r1 = r0*5*2 + 130 = r0*10 + 130
//   _080038A4(30, r1, 0x0805F904)
// ----------------------------------------------------------------------------
void _0800D2B0(void *rec)
{
    int i;

    _08003940(4, 0x0805F8B0u);

    _08003F18(40u, 20u, (const volatile u8 *)(uintptr_t)0x0805F8C0u, (int)(u16)_08002044());
    _08003F18(40u, 30u, (const volatile u8 *)(uintptr_t)0x0805F8D0u, (int)(u16)_08001F8C());
    _08003F18(40u, 40u, (const volatile u8 *)(uintptr_t)0x0805F8E0u, (int)_08002140());
    _08003F18(40u, 50u, (const volatile u8 *)(uintptr_t)0x0805F8F0u, (int)_08001E14(0));

    for (i = 0; i <= 3; i++) {
        int x = i * 50 + 50;
        _08003978(x, 60, (int)(u16)_0800206C(i));
        _08003978(x, 70, (int)(u16)_08001E5C(i, 0));
        _08003978(x, 80, (int)(u16)_08002178(i));
    }

    _080038A4(40, 130, (void *)(uintptr_t)0x0805F8F4u);
    _080038A4(40, 140, (void *)(uintptr_t)0x0805F8FCu);

    {
        u16 cursor = *(u16 *)((uintptr_t)rec + 6);
        _080038A4(30, (int)cursor * 10 + 130, (void *)(uintptr_t)0x0805F904u);
    }
}
