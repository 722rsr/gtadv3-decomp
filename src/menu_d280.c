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
__attribute__((weak)) u32   _08002044(void) { return 0; }
__attribute__((weak)) int   _08001F8C(void) { return 0; }
__attribute__((weak)) u32   _08002140(void) { return 0; }
__attribute__((weak)) u32   _08001E14(int a) { (void)a; return 0; }
__attribute__((weak)) u16   _0800206C(int a) { (void)a; return 0; }
__attribute__((weak)) u16   _08001E5C(int a, int b) { (void)a; return 0; }
__attribute__((weak)) u16   _08002178(int a, int b) { (void)a; (void)b; return 0; }
#else
extern void _08002124(u16 v);   // 0x08002124 block-B arm (src/block_b.c)
extern int  _0800254C(void);    // 0x0800254C screen-tile init; takes no argument
extern void _08003940(int a, u32 b);   // 0x08003940
extern void _08003F18(u32 a, u32 b, const volatile u8 *c, int d); // 0x08003F18 text lane
extern void _080038A4(int a, int b, void *c); // 0x080038A4 frame draw
extern void _08003978(int a, u32 b, int c);   // 0x08003978 sprite space
extern u32  _08002044(void);    // 0x08002044 (src/idle_accessors.c)
extern int  _08001F8C(void);    // 0x08001F8C record count
extern u32  _08002140(void);    // 0x08002140 block-B match
extern u32  _08001E14(int a);   // 0x08001E14 descriptor word
extern u16  _0800206C(int a);   // 0x0800206C record filtered
extern u16  _08001E5C(int a, int b); // 0x08001E5C record halfword
extern u16  _08002178(int a, int b); // 0x08002178; r1 is dead in the ROM callee
#endif

#ifndef __APPLE__
extern const volatile u8 D2B0_Label_F8B0[];
extern const volatile u8 D2B0_Label_F8C0[];
extern const volatile u8 D2B0_Label_F8D0[];
extern const volatile u8 D2B0_Label_F8E0[];
extern const volatile u8 D2B0_Label_F8F0[];
extern const volatile u8 D2B0_Label_F8F4[];
extern const volatile u8 D2B0_Label_F8FC[];
extern const volatile u8 D2B0_Label_F904[];
#else
#define D2B0_Label_F8B0 ((const volatile u8 *)(uintptr_t)0x0805F8B0u)
#define D2B0_Label_F8C0 ((const volatile u8 *)(uintptr_t)0x0805F8C0u)
#define D2B0_Label_F8D0 ((const volatile u8 *)(uintptr_t)0x0805F8D0u)
#define D2B0_Label_F8E0 ((const volatile u8 *)(uintptr_t)0x0805F8E0u)
#define D2B0_Label_F8F0 ((const volatile u8 *)(uintptr_t)0x0805F8F0u)
#define D2B0_Label_F8F4 ((const volatile u8 *)(uintptr_t)0x0805F8F4u)
#define D2B0_Label_F8FC ((const volatile u8 *)(uintptr_t)0x0805F8FCu)
#define D2B0_Label_F904 ((const volatile u8 *)(uintptr_t)0x0805F904u)
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

#ifndef __APPLE__
    __asm__ volatile(
        ".global D2B0_Label_F8B0\nD2B0_Label_F8B0 = 0x0805F8B0\n"
        ".global D2B0_Label_F8C0\nD2B0_Label_F8C0 = 0x0805F8C0\n"
        ".global D2B0_Label_F8D0\nD2B0_Label_F8D0 = 0x0805F8D0\n"
        ".global D2B0_Label_F8E0\nD2B0_Label_F8E0 = 0x0805F8E0\n"
        ".global D2B0_Label_F8F0\nD2B0_Label_F8F0 = 0x0805F8F0\n"
        ".global D2B0_Label_F8F4\nD2B0_Label_F8F4 = 0x0805F8F4\n"
        ".global D2B0_Label_F8FC\nD2B0_Label_F8FC = 0x0805F8FC\n"
        ".global D2B0_Label_F904\nD2B0_Label_F904 = 0x0805F904\n");
#endif

    _08003940(4, (u32)(uintptr_t)D2B0_Label_F8B0);

    _08003F18(40u, 20u, D2B0_Label_F8C0, (int)_08002044());
    _08003F18(40u, 30u, D2B0_Label_F8D0, _08001F8C());
    _08003F18(40u, 40u, D2B0_Label_F8E0, (int)_08002140());
    _08003F18(40u, 50u, D2B0_Label_F8F0, (int)_08001E14(0));

    for (i = 0; i <= 3; i++) {
        int x = i * 50 + 50;
        _08003978(x, 60, (int)(u16)_0800206C(i));
        _08003978(x, 70, (int)(u16)_08001E5C(i, 0));
        {
            u16 state = (u16)_08002178(i, 0);
            __asm__ volatile("" : : : "r1");
            _08003978(x, 80, (int)state);
        }
    }

    _080038A4(40, 130, (void *)(uintptr_t)D2B0_Label_F8F4);
    _080038A4(40, 140, (void *)(uintptr_t)D2B0_Label_F8FC);

    {
        u16 cursor = *(u16 *)((uintptr_t)rec + 6);
        _080038A4(30, (int)cursor * 10 + 130,
                  (void *)(uintptr_t)D2B0_Label_F904);
    }
}
