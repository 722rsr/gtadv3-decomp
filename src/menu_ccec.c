// ============================================================================
// menu_ccec.c — C lift of asm/menu_ccec.s (VMA 0x0800CCEC–0x0800CE2C, 3 funcs)
//
// Menu record trio : record-open, cursor key handler, and paint
// lane over the block-A record context and the sprite-object API. Every
// function is transcribed instruction-for-instruction from the cited asm.
//
// Record layout (offsets used): +0 u16, +6 u16 cursor value, +0x1018 obj ptr.
// Shared cells: 0x03001780+0x574 (u16 source), cursor garage record field.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void *_08004B68(void) { return (void *)0; }
__attribute__((weak)) int   _08004CA8(int v) { (void)v; return 0; }
__attribute__((weak)) void *_08026948(void) { return (void *)0; }
__attribute__((weak)) void  _08026A4C(void *obj, int slot, int attr) { (void)obj; (void)slot; (void)attr; }
__attribute__((weak)) void  _08026A58(void *obj, int v) { (void)obj; (void)v; }
__attribute__((weak)) void  _08026A60(void *obj, int v) { (void)obj; (void)v; }
__attribute__((weak)) void  _08026938(void *obj, int slot) { (void)obj; (void)slot; }
__attribute__((weak)) void  _08004EA8(int v) { (void)v; }
__attribute__((weak)) void  _08004EC0(int v) { (void)v; }
__attribute__((weak)) void  _080038A4(int a, int b, const void *c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _08003978(int a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _08026A20(void *obj, int x, int y) { (void)obj; (void)x; (void)y; }
#endif

extern void *_08004B68(void);   // 0x08004B68 scene ctx getter
extern int   _08004CA8(int v);  // 0x08004CA8
#ifndef __APPLE__
extern void *_080026948(void);        // 0x08026948 obj alloc (closure spelling)
extern void  _080026938(void *obj, int slot); // 0x08026938 (closure spelling)
#define _08026948 _080026948
#define _08026938 _080026938
#else
extern void *_08026948(void);        // 0x08026948 obj alloc (no argument)
extern void  _08026938(void *obj, int slot);           // 0x08026938
#endif
extern void  _08026A4C(void *obj, int slot, int attr); // 0x08026A4C
extern void  _08026A58(void *obj, int v);              // 0x08026A58
extern void  _08026A60(void *obj, int v);              // 0x08026A60
extern void  _08004EA8(int v);  // 0x08004EA8 manager store+step
extern void  _08004EC0(int v);  // 0x08004EC0
extern void  _080038A4(int a, int b, const void *c);   // 0x080038A4 frame draw
extern void  _08003978(int a, int b, int c);           // 0x08003978 sprite space
extern void  _08026A20(void *obj, int x, int y);       // 0x08026A20 obj position
extern void  _080026A20(void *obj, int x, int y);      // 9-digit closure spelling (0x08026A20 is promoted)

#define WA              0x03001780u
#define WA_CURSOR_OFF   0x0574u
#define REC_OBJ_OFF     0x1018u

// Keypad block at IWRAM 0x030035C0; the field read here is the u16 at +12.
// The struct spelling keeps the address as base+member-offset: agbcc then emits
// `ldr rX,[pc]` for 0x030035C0 and `ldrh rX,[rX,#12]`, matching the ROM pool.
typedef struct { u8 ccec_kp_pad[12]; u16 ccec_kp_w12; } Ccec_Keypad;
#define KEYPAD_W12 (((volatile Ccec_Keypad *)(uintptr_t)0x030035C0)->ccec_kp_w12)

// ----------------------------------------------------------------------------
// 0x0800CCEC sub_0800CCEC(rec)
//   r5=rec
//   _08004B68; r0=_08004CA8(0); [rec+0] = (u16)r0
//   [rec+6] = u16[WA+0x574]
//   obj = _08026948; [rec+0x1018] = obj
//   _08026A4C(obj, (s16)[rec+6], 0)
//   _08026A58(obj, 1); _08026A60(obj, 0); _08026938(obj, (s16)[rec+6])
// ----------------------------------------------------------------------------
void _0800CCEC(void *rec)
{
    // The ROM keeps `rec` in r5 for the whole body and a *pointer to the obj
    // field* in r4, re-loading `ldr r0,[r4]` at each of the four calls. Reading
    // the field through a non-volatile struct member is what makes agbcc
    // re-read memory instead of caching the pointer in a register; a local
    // `void *obj` collapses to `adds r0,r4,#0` copies and costs 8 bytes.
    struct CcecRec { u16 f0; u16 f1; u16 f2; u16 cursor; u8 pad[0x1018 - 8]; void *obj; };
    struct CcecRec *m = (struct CcecRec *)rec;

    // The cursor cell is 0x574 bytes into the 0x03001780 work area, and the ROM
    // materialises the address as base + 0x574 from TWO pool words before the
    // load. A single folded constant (0x03001CF4) drops the second word and
    // the `adds`; going through the extern object plus a typed member offset
    // is what keeps agbcc from folding it.
    struct CcecWA { u8 pad[0x574]; u16 cursor; };
    extern u8 CcecWorkArea[];
    __asm__("CcecWorkArea = 0x03001780");

    _08004B68();
    m->f0 = (u16)_08004CA8(0);
    m->cursor = ((volatile struct CcecWA *)(uintptr_t)CcecWorkArea)->cursor;

    // No argument: the ROM goes straight from `strh` into `bl`, so the C call
    // must not materialise an r0 argument the way `_08026948(0)` did.
    m->obj = _08026948();
    _08026A4C(m->obj, (int)(s16)m->cursor, 0);
    _08026A58(m->obj, 1);
    _08026A60(m->obj, 0);
    _08026938(m->obj, (int)(s16)m->cursor);
}

// Rule 6: asm/menu_ccec.s:6-8 defines BOTH `sub_0800CCEC:` and `_0800CCEC:`
// on this one span, so the splice deletes the `sub_` spelling. C must define
// it, or an export array naming `sub_0800CCEC` fails the independent link.
// Guarded: clang rejects `alias` attributes on darwin.
#ifndef __APPLE__
void sub_0800CCEC(void *rec) __attribute__((alias("_0800CCEC")));
#endif

// ----------------------------------------------------------------------------
// 0x0800CD48 sub_0800CD48(rec, keys, arg2)
//   r5=rec, r4=(u16)arg2, r6=copy of r4 (the &16 test reads r6)
//   prev=(s16)[rec+6] in r8; r7=0 (delta)
//   if (r4 & 2): _08004EA8(1)
//   if (r4 & 1): u16[WA+0x574] = [rec+6]; _08004EC0(6)
//   if (r4 & 32): r7 -= 1
//   if (r6 & 16): r7 += 1
//   r0 = [rec+6] + r7; [rec+6] = r0 (u16)
//     if r0 (s32<<16) < 0: [rec+6] = 0
//   if (s16)[rec+6] > 96: [rec+6] = 96
//   if (s16)[rec+6] != prev:
//     _08026A4C([rec+0x1018], (s16)[rec+6], 0)   <- r1 is the CURSOR, not 0
//     _08026938([rec+0x1018], (s16)[rec+6])      <- reloads the obj each call
//
// `keys`/`arg2` ARE declared at word width. Not to drop a prologue extension
// pair — the ROM HAS one (`lsls r2,r2,#16 / lsrs r4,r2,#16`, the narrowing of
// a u16 param) — but because widening `arg2` gives `r4` and the `r6` copy
// overlapping live ranges that the coalescer can no longer merge. With a u16
// `arg2` agbcc folds the copy away, `delta` takes r6 and `prev` takes r7, and
// the r8 save/restore and `adds r6,r4,#0` both disappear (1/82 -> 35/82).
//
// `prev` must be `int`, not `s16`: a 16-bit local is re-sign-extended at the
// final compare (`lsls/asrs`) instead of living sign-extended in r8.
// The `WA+0x574` store needs the assembler-symbol idiom (two pool words plus an
// `adds`, not one folded literal) — same fold and same fix as MenuFF78_1226C.
// ----------------------------------------------------------------------------
void _0800CD48(void *rec, u32 keys, u32 arg2)
{
    u16 *r = (u16 *)rec;
    u16 r4 = (u16)arg2;    /* keys handled through r2 in asm (r1 unused) */
    u32 r6 = r4;           /* second copy: the ROM keeps one live to the &16 test */
    int prev;
    int delta;
    int nv;

    (void)keys;
    _08004B68();
    prev = (s16)r[3];
    delta = 0;

    if (r4 & 2)
        _08004EA8(1);
    if (r4 & 1) {
        extern u8 MenuCcec_WA_CD48[];
        __asm__("MenuCcec_WA_CD48 = 0x03001780");
        {
            u32 wa = (u32)(uintptr_t)MenuCcec_WA_CD48;
            u32 cv = r[3];
            u32 base = wa + WA_CURSOR_OFF;
            *(volatile u16 *)(uintptr_t)base = (u16)cv;
        }
        _08004EC0(6);
    }
    if (r4 & 32)
        delta -= 1;
    if (r6 & 16)
        delta += 1;

    nv = (int)r[3] + delta;
    r[3] = (u16)nv;
    if ((int)(nv << 16) < 0)
        r[3] = 0;
    if ((s16)r[3] > 96)
        r[3] = 96;

    if ((s16)r[3] != prev) {
        void **pp = (void **)(uintptr_t)((u8 *)rec + REC_OBJ_OFF);
        _08026A4C(*pp, (int)(s16)r[3], 0);
        _08026938(*pp, (int)(s16)r[3]);
    }
}

// ----------------------------------------------------------------------------
// 0x0800CDEC sub_0800CDEC(rec)
//   r4=rec
//   _080038A4(96, 15, 0x0805F88C)
//   r0 = u16[0x030035C0+12]; r2 = (r0<<16)>>19
//   _08003978(120, 32, r2)
//   obj = [rec+0x1018]; _08026A20(obj, 120, 100)
// ----------------------------------------------------------------------------
// Shape copied wholesale from the promoted twin 0x0800CF60 (MenuRec_0800CF60
// in src/menu_records.c): the two spans are identical apart from the `bl`
// immediates and the 0x0805F88C/F894 pool literal, so every register pin in
// that body is load-bearing here too.
void _0800CDEC(void *rec)
{
    volatile u8 *r4 = (volatile u8 *)rec;
    _080038A4(96, 15, (const void *)(uintptr_t)0x0805F88Cu);
    register u16 v __asm__("r0") = KEYPAD_W12;
    register u32 t __asm__("r2");
    t = (u32)v << 16;
    t = (u32)((s32)t >> 19);
    _08003978(120, 32, (int)t);
    void *obj = (void *)(uintptr_t)(*(volatile u32 *)(r4 + REC_OBJ_OFF));
    _080026A20(obj, 120, 100);
}
