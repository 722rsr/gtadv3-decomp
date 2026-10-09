// ============================================================================
// Runtime record, menu, sound-bank, and course helpers. Assembly owners:
//
//   asm/menu_e650.s              0x0800E7A0 / 0x0800E7A4   (bx lr stubs)
//   asm/menu_d13c.s              0x0800D13C / 0x0800D17C
//   asm/menu_c454.s              0x0800C454 / 0x0800C4AC
//   asm/sound_bank.s             0x0802B64C / 0x0802B65C
//   asm/code_2254.s              0x080022CC / 0x080022D8
//   asm/course_proximity_more.s  0x08007368 / 0x08007484
//   asm/course_stream_tail_more.s 0x08006D9C / 0x08006E1C
//   asm/wrapper_pair.s           0x08026230 / 0x080262A4
//   asm/sound_da20.s             0x0802DA58 / 0x0802DAE0
//   asm/code_279c.s              0x08002844 / 0x08002924
//   asm/code_24e54.s             0x080024E7C / 0x080024EFC
//   asm/code_26068.s             0x080026088 / 0x08002612C
//   asm/code_23bd4.s             0x080023BD4 / 0x080023C54
//
// Transcribed instruction-for-instruction from the cited asm listings.
// ============================================================================

#include "gba/types.h"

// Closure spelling of 0x08002C98 (ObjFlush), defined in src/garage_records.c. The
// body is in another TU, so it needs an extern here; the friendly
// `Sub_08002C98` name is only the host stub over there.
extern void sub_08002C98(void);

#ifdef __APPLE__
__attribute__((weak)) void  _080038A4(int a, int b, const void *c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _08026A20(void *o, int x, int y) { (void)o; (void)x; (void)y; }
__attribute__((weak)) void  _08003ADC(int a, int b) { (void)a; (void)b; }
__attribute__((weak)) void  _0800CFE4(void *a) { (void)a; }
__attribute__((weak)) void  _0800D048(void *a, u16 b, u16 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _08007770(int a, void *b, int c, int d, u32 e, u32 f) {
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f;
}
__attribute__((weak)) void  Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f) {
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f;
}
__attribute__((weak)) u32   _080056DC(int a) { (void)a; return 0; }
__attribute__((weak)) u32   _080056C4(int a) { (void)a; return 0; }
__attribute__((weak)) void  _0802C548(u32 ch) { (void)ch; }
__attribute__((weak)) void  _0802C614(u32 ch) { (void)ch; }
__attribute__((weak)) int   _08001CB4(void) { return 0; }
__attribute__((weak)) int   _08001E14(void) { return 0; }
__attribute__((weak)) void *_08006C10(void *s, int d) { (void)s; (void)d; return 0; }
__attribute__((weak)) int   _08006C94(void *s, void *p) { (void)s; (void)p; return 0; }
__attribute__((weak)) void  _08005BA8(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void  _08005DA4(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) int   _08005E14(void *a, void *b, void *c) { (void)a; (void)b; (void)c; return 0; }
__attribute__((weak)) void *_08007498(void *x, int i) { (void)x; (void)i; return 0; }
__attribute__((weak)) void *_0800748C(void *x) { (void)x; return 0; }
__attribute__((weak)) void  _0802D988(const void *s, void *d) { (void)s; (void)d; }
__attribute__((weak)) void  _0802D970(const void *s, void *d, u32 c) { (void)s; (void)d; (void)c; }
__attribute__((weak)) void  _0802D974(const void *s, void *d, u32 c) { (void)s; (void)d; (void)c; }
__attribute__((weak)) s16   _080261B0(int a) { (void)a; return 0; }
__attribute__((weak)) s16   _080261F8(int a) { (void)a; return 0; }
__attribute__((weak)) void  _0800798C(void *a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void  _08007ABC(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _080075E8(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _08007CD0(void *a, int b, int c, int d, int e, int f, int g) {
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g;
}
__attribute__((weak)) void  _08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i) {
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i;
}
__attribute__((weak)) void  _080056FC(void) {}
__attribute__((weak)) void  _080056B8(int a) { (void)a; }
// Apple-only no-op counterparts of the closure-spelling callees below. The ARM
// build calls sub_08003104 / sub_08002B50 / sub_08002BB4 (the names asm/ defines);
// the host build keeps the old no-op behaviour through these stubs.
__attribute__((weak)) void  _08003104(void *a) { (void)a; }
__attribute__((weak)) void  _080032E0(void *a) { (void)a; }
__attribute__((weak)) void  _08002B50(void) {}
__attribute__((weak)) void  _08002BB4(void) {}
__attribute__((weak)) void  _08002B44(void) {}
__attribute__((weak)) void  _0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void  _080023AE4(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _080023B60(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _0800DBE8(void *a) { (void)a; }
__attribute__((weak)) void  _08022D44(void *a, void *b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _08023E78(void *a) { (void)a; }
__attribute__((weak)) void  _08002E0A4(void *d, const void *s, u32 n) { (void)d; (void)s; (void)n; }
__attribute__((weak)) void  _08003940(int a, u32 b) { (void)a; (void)b; }
#else
extern void  _080038A4(int a, int b, const void *c);
extern void  _08026A20(void *o, int x, int y);
extern void  _08003ADC(int a, int b);
extern void  _0800CFE4(void *a);                         // 0x0800CFE4
extern void  _0800D048(void *a, u16 b, u16 c);            // 0x0800D048
extern u32   _08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void  Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f); // 0x08007770 exact ROM trampoline
extern u32   _080056DC(int a);
extern u32   _080056C4(int a);
extern void  _0802C548(u32 ch);
extern void  _0802C614(u32 ch);
extern int   _08001CB4(void);
extern int   _08001E14(void);
extern void *_08006C10(void *s, int d);
extern int   _08006C94(void *s, void *p);
extern void  _08005BA8(void *a, int b);
extern void  _08005DA4(void *a, void *b);
extern int   _08005E14(void *a, void *b, void *c);
extern void *_08007498(void *x, int i);
extern void *_0800748C(void *x);
extern void  _0802D988(const void *s, void *d);
extern void  _0802D970(const void *s, void *d, u32 c);
extern void  _0802D974(const void *s, void *d, u32 c);
extern s16   _080261B0(int a);
extern s16   _080261F8(int a);
extern void  _0800798C(void *a, void *b);
extern void  _08007ABC(void *a, int b, int c);
extern void  _080075E8(void *a, int b, int c);
extern void  _08007CD0(void *a, int b, int c, int d, int e, int f, int g);
extern void  _08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i);
extern void  _080056FC(void);
extern void  _0800D97C(void *a, int b);
extern void  _080023AE4(void *a, int b, int c);
extern void  _080023B60(void *a, int b, int c);
extern void  _0800DBE8(void *a);
extern void  _08022D44(void *a, void *b, int c);
extern void  _08023E78(void *a);
extern void  _08002E0A4(void *d, const void *s, u32 n);
extern void  _08003940(int a, u32 b);
extern void  _080056B8(int a);                            // 0x080056B8
extern void  sub_08003104(void *a);                      // 0x08003104 closure spelling
#ifndef __APPLE__
extern void  sub_080032E0(void *a);                      // 0x080032E0, the closure spelling
#else
extern void  _080032E0(void *a);                         // host weak no-op above
#endif
extern void  sub_08002B50(void);                         // 0x08002B50 closure spelling
extern void  sub_08002BB4(void);                         // 0x08002BB4 closure spelling
extern void  _08002B44(void);                            // 0x08002B44
#endif

// ============================================================================
// menu_e650.s — bx lr stubs
// ============================================================================

void _0800E7A0(void) { }
__asm__(".align 2, 0");
void _0800E7A4(void) { }
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_0800E7A0(void) __attribute__((alias("_0800E7A0")));
void sub_0800E7A4(void) __attribute__((alias("_0800E7A4")));
#endif

// ============================================================================
// menu_d13c.s — renderer + 4-way event dispatcher
// ============================================================================

void _0800D13C(void *rec)
{
    volatile u8 *r = (volatile u8 *)rec;
    // The two field reads are NOT volatile (trap 5): a `volatile s16` read at a
    // constant offset expands to `ldrh; lsls #16; asrs #16` (6 B) where the
    // ROM has the 4-byte register-offset form `movs r0,#imm; ldrsh r1,[r4,r0]`
    // -- which is exactly what a plain `const s16 *` read yields (same lever as
    // TrackDigit_043B8). The ROM's read is decisive; rec is a work record, not
    // a hardware register. Removal control: volatile back on -> 24/64.
    const u8 *rc = (const u8 *)rec;
    _080038A4(96, 15, (const void *)(uintptr_t)0x0805F89Cu);
    _08026A20((void *)(uintptr_t)*(volatile u32 *)(r + 28), 120, 100);
    _08003ADC(40, (int)*(const s16 *)(rc + 6));
    _08003ADC(50, (int)*(const s16 *)(rc + 8));
    _08003ADC(60, 97);
}
#ifndef __APPLE__
void sub_0800D13C(void *a) __attribute__((alias("_0800D13C")));
#endif

void _0800D17C(int ev, int a1, int a2, void *rec)
{
    switch ((u32)ev) {
    case 7:
        _0800D13C(rec);
        break;
    case 6:
        _0800D048(rec, (u16)a1, (u16)a2);
        break;
    case 1:
        _0800CFE4(rec);
        break;
    }
}
// The last two bytes are the section-alignment filler, not unreachable code:
// the body is 54 bytes, so under `-ffunction-sections` its section pads to 56
// and gas closes a Thumb *code* section with the 2-byte nop (0x46c0) where the
// ROM holds `00 00`. This file-scope `.align 2, 0` is emitted after the body's
// `.size`, still inside the body's own section, and pads with the explicit `0`
// fill instead -- the same one-liner `_0800D1EC` uses in src/menu_d1b4.c.
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_0800D17C(int a, int b, int c, void *d) __attribute__((alias("_0800D17C")));
#endif

// ============================================================================
// menu_c454.s — tilemap fill helpers over resource 0x08292B40
// ============================================================================

void _0800C454(void)
{
    // ROM sub_0800C454 consumes 07770's r0 (`adds r4, r0, #0` → tile fill),
    // and ROM 07770 exits with r0 = r9 = _080050D0(ctx,...) (return ruling
    // : the strong body returns it, so this calls C-to-C).
    extern u32 _08007770(int a, void *b, int c, int d, u32 e, u32 f);
    u32 zero = 0;
    u32 fill = (u16)_08007770(0, (void *)(uintptr_t)0x08292B40u, 8, 0, zero, zero);
    void *base = (void *)(uintptr_t)_080056DC(0);
    (void)_080056C4(0);
    volatile u16 *lo = (volatile u16 *)base;
    volatile u16 *hi = (volatile u16 *)((u8 *)base + 0x800);
    for (int row = 0; row <= 31; row++) {
        volatile u16 *a = lo + (row << 5);
        volatile u16 *b = hi + (row << 5);
        for (int col = 31; col >= 0; col--) {
            *a++ = fill;
            *b++ = fill;
        }
    }
}
#ifndef __APPLE__
void sub_0800C454(void) __attribute__((alias("_0800C454")));
#endif

void _0800C4AC(void)
{
    // ROM sub_0800C4AC discards r0; calls the strong body C-to-C (same
    // ruling as _0800C454 above).
    extern u32 _08007770(int a, void *b, int c, int d, u32 e, u32 f);
    _08007770(1, (void *)(uintptr_t)0x08292B40u, 12, 0, 0u, 1u);
}
#ifndef __APPLE__
void sub_0800C4AC(void) __attribute__((alias("_0800C4AC")));
#endif

// 0x0800C4CC — a real 2-byte leaf (`bx lr`), not alignment filler: the two
void _0800C4CC(void) { }
__asm__(".align 2, 0");

// ============================================================================
// sound_bank.s — start/pause wrappers (u16 truncate → sequencer)
// ============================================================================

void _0802B64C(u32 ch)
{
    _0802C548((u16)ch);
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_0802B64C(u32 c) __attribute__((alias("_0802B64C")));
#endif

void _0802B65C(u32 ch)
{
    _0802C614((u16)ch);
}
// 14 bytes of code in a 4-aligned section: gas closes the section with a
// 2-byte `nop` (0x46c0) where the ROM holds `00 00` at 0x0802B66A. Emitted after
// this body's `.size`, still inside its own section, so it pads with the `0`
// fill argument instead. No instruction changes.
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_0802B65C(u32 c) __attribute__((alias("_0802B65C")));
#endif

// ============================================================================
// code_2254.s — thin wrappers over idle getters
// ============================================================================

int _080022CC(void)
{
    return _08001CB4();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
int sub_080022CC(void) __attribute__((alias("_080022CC")));
int Sub_080022CC(void) __attribute__((alias("_080022CC")));
#endif

int _080022D8(void)
{
    return _08001E14();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
int sub_080022D8(void) __attribute__((alias("_080022D8")));
#endif

// ============================================================================
// course_proximity_more.s — identity return + proximity search
// ============================================================================

// 0x08007484 — bx lr (return r0 unchanged). Callers treat the result as
// either a pointer or an int; both share the same Thumb ABI in r0.
void *_08007484(void *x)
{
    return x;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void *sub_08007484(void *x) __attribute__((alias("_08007484")));
#endif

// 0x08007368 — proximity hit test (exact control flow from asm)
int _08007368(void *state, void *pos, u32 heading_u16, void *out)
{
    void *row = _08006C10(state, 0);
    s32 dx0 = *(volatile s32 *)pos - *(volatile s32 *)((u8 *)row + 12);
    s32 dy0 = *(volatile s32 *)((u8 *)pos + 4) - *(volatile s32 *)((u8 *)row + 16);
    s32 delta[2];
    delta[0] = dx0;
    delta[1] = dy0;

    for (int pass = 0; pass <= 1; pass++) {
        s32 box[2];
        box[0] = (pass == 0) ? 32 : -32;
        box[1] = 64;
        _08005BA8(box, (s16)heading_u16);
        s32 tmp[2];
        _08005DA4(tmp, delta);

        volatile u8 *slot = (volatile u8 *)row + 40;
        s32 sl_off = 0;
        for (int k = 0; k <= 1; k++) {
            s32 hit[2];
            if (_08005E14(hit, tmp, (void *)slot)) {
                s32 hx = hit[0];
                s32 hy = hit[1];
                s32 ax = hx >> 8;
                s32 ay = hy >> 8;
                if (*(volatile s16 *)(slot + 12) <= ax &&
                    *(volatile s16 *)(slot + 16) >= ax &&
                    *(volatile s16 *)(slot + 14) <= ay &&
                    *(volatile s16 *)(slot + 18) >= ay) {
                    s32 ddx = hx - delta[0];
                    s32 ddy = hy - delta[1];
                    if (ddx * ddx + ddy * ddy <= 25) {
                        volatile s32 *o = (volatile s32 *)out;
                        o[0] = *(volatile s32 *)((u8 *)row + 12) + hx;
                        o[1] = *(volatile s32 *)((u8 *)row + 16) + hy;
                        o[2] = -*(volatile s32 *)((u8 *)row + 40 + (u32)sl_off);
                        o[3] = -*(volatile s32 *)((u8 *)row + 44 + (u32)sl_off);
                        return 1;
                    }
                }
            }
            slot += 28;
            sl_off += 28;
        }
    }
    return 0;
}
#ifndef __APPLE__
int sub_08007368(void *a, void *b, u32 c, void *d) __attribute__((alias("_08007368")));
#endif

// ============================================================================
// course_stream_tail_more.s — movement predicates
// ============================================================================

int _08006D9C(void *state, void *vec)
{
    register u8 *vv asm("r9");
    register u8 *st asm("r8");
    st = (u8 *)state;
    vv = (u8 *)vec;
    void *cur = _08006C10(st, 0);
    if (!cur)
        return 0;
    void *prev = _08006C10(st, -1);
    s32 dx, dy;
    if (prev) {
        void *again = _08006C10(st, 0);
        s32 a = *(s16 *)((u8 *)again + 28) + *(s32 *)((u8 *)again + 12);
        s32 b = *(s16 *)((u8 *)prev + 28) + *(s32 *)((u8 *)prev + 12);
        dx = a - b;
        s32 c = *(s16 *)((u8 *)again + 30) + *(s32 *)((u8 *)again + 16);
        s32 d = *(s16 *)((u8 *)prev + 30) + *(s32 *)((u8 *)prev + 16);
        dy = c - d;
    } else {
        dx = *(s16 *)((u8 *)cur + 24);
        dy = *(s16 *)((u8 *)cur + 26);
    }
    s32 dot = *(s32 *)vv * dx + *(s32 *)(vv + 4) * dy;
    return (dot < 0) ? 1 : 0;
}
#ifndef __APPLE__
int sub_08006D9C(void *a, void *b) __attribute__((alias("_08006D9C")));
#endif

void *_08006E1C(void *state, void *pos)
{
    void *cur = _08006C10(state, 0);
    if (!cur)
        return 0;
    if (_08006C94(cur, pos) >= 0) {
        void *prev = _08006C10(state, -1);
        if (!prev)
            goto ret_cur;
        if (_08006C94(prev, pos) < 0)
            goto ret_cur;
        return prev;
    } else {
        return _08006C10(state, 1);
    }
ret_cur:
    return cur;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void *sub_08006E1C(void *a, void *b) __attribute__((alias("_08006E1C")));
#endif

// ============================================================================
// wrapper_pair.s — resource wrap A/B
// ============================================================================

void *_08026230(int idx, void *ptr)
{
    u32 *mgr = *(u32 **)(uintptr_t)0x03001670u;
    u32 *slot = (u32 *)((u8 *)mgr + (idx << 3) + 8);
    u32 m1;
    u32 accum;
    void *arr;
    slot[0] = (u32)(uintptr_t)ptr;
    m1 = mgr[1];
    slot[1] = m1;
    accum = m1;
    arr = _0800748C(_08007498((void *)(uintptr_t)0x0879984Cu,
                              (int)(uintptr_t)ptr));
    {
        u8 *ab = (u8 *)arr;
        int i;
        for (i = 0; i <= 255; i++) {
            u8 b = *(ab + (i >> 3));
            if (((b >> (i & 7)) & 1) != 0)
                accum += 0x900; /* 144<<4 */
        }
    }
    mgr[1] = accum;
    return (void *)slot;
}
#ifndef __APPLE__
void *sub_08026230(int a, void *b) __attribute__((alias("_08026230")));
#endif

void _080262A4(void *rec, int sel)
{
    // asm/wrapper_pair.s 0x080262A4 (2-arg: r0=rec, r1=caller's second
    // argument): t0 = Seek(0x083D7BE8, r1) stored RAW to [sp] (no ArrBase
    // here — ArrBase is applied on each re-read below); mask =
    // ArrBase(Seek(0x0879984C, 7)); bits = ArrBase(Seek(0x0879984C,
    // u32[rec])); then per 128-row half: src = ArrBase(Seek(t0, 0/1)),
    // LZ77UnCompWram(src, 0x02000000), and the bit loops copy 0x240 words
    // (CpuFastSet, len r2=144<<2) from EWRAM to the rec+4 cursor for set
    // bits[] entries while advancing the source cursor 0x900 per set
    // mask[] entry. Cursor is read-modify-written through r8 (rec+4).
    u32 cursor = *(volatile u32 *)((u8 *)rec + 4);
    void *t0 = _08007498((void *)(uintptr_t)0x083D7BE8u, sel);
    void *mask = _0800748C(_08007498((void *)(uintptr_t)0x0879984Cu, 7));
    void *bits = _0800748C(_08007498((void *)(uintptr_t)0x0879984Cu,
                                     (int)*(volatile u32 *)rec));
    void *src0 = _0800748C(_08007498(t0, 0));
    _0802D988(src0, (void *)(uintptr_t)0x02000000u);

    u32 vram = 0x02000000u;
    for (int i = 0; i <= 127; i++) {
        int byte = i >= 0 ? (i >> 3) : ((i + 7) >> 3);
        int bit = i & 7;
        if (((*(volatile u8 *)((u8 *)bits + byte) >> bit) & 1) != 0) {
            _0802D970((const void *)(uintptr_t)vram,
                      (void *)(uintptr_t)cursor, 0x240); /* 144<<2 */
            cursor += 0x900;
        }
        if (((*(volatile u8 *)((u8 *)mask + byte) >> bit) & 1) != 0)
            vram += 0x900;
    }

    void *src1 = _0800748C(_08007498(t0, 1));
    _0802D988(src1, (void *)(uintptr_t)0x02000000u);
    vram = 0x02000000u;
    for (int i = 128; i <= 255; i++) {
        int byte = i >= 0 ? (i >> 3) : ((i + 7) >> 3);
        int bit = i & 7;
        if (((*(volatile u8 *)((u8 *)bits + byte) >> bit) & 1) != 0) {
            _0802D970((const void *)(uintptr_t)vram,
                      (void *)(uintptr_t)cursor, 0x240);
            cursor += 0x900;
        }
        if (((*(volatile u8 *)((u8 *)mask + byte) >> bit) & 1) != 0)
            vram += 0x900;
    }
}
#ifndef __APPLE__
void sub_080262A4(void *a, int b) __attribute__((alias("_080262A4")));
#endif

// ============================================================================
// sound_da20.s — IRQ/timer arm + disarm
// ============================================================================

void _0802DA58(volatile u16 *src)
{
    *(volatile u16 *)0x03001774u = *(volatile u16 *)0x04000208u; /* IME save */
    *(volatile u16 *)0x04000208u = 0;
    volatile u16 *tm = *(volatile u16 **)(uintptr_t)0x03001770u;
    tm[1] = 0;
    u8 sel = *(volatile u8 *)0x03001768u;
    *(volatile u16 *)0x04000202u = (u16)(8u << sel);           /* IF ack */
    *(volatile u16 *)0x04000200u |= (u16)(8u << sel);          /* IE or */
    *(volatile u8 *)0x0300176Cu = 0;
    *(volatile u16 *)0x0300176Au = src[0];
    tm[0] = src[1];
    *(volatile u16 **)(uintptr_t)0x03001770u = tm + 1;
    tm[1] = src[2];
    *(volatile u16 **)(uintptr_t)0x03001770u = tm;
    *(volatile u16 *)0x04000208u = 1;
}
#ifndef __APPLE__
void sub_0802DA58(volatile u16 *a) __attribute__((alias("_0802DA58")));
#endif

void _0802DAE0(void)
{
    *(volatile u16 *)0x04000208u = 0;
    volatile u16 *tm = *(volatile u16 **)(uintptr_t)0x03001770u;
    tm[0] = 0;
    *(volatile u16 **)(uintptr_t)0x03001770u = tm + 1;
    tm[1] = 0;
    *(volatile u16 **)(uintptr_t)0x03001770u = tm;
    u8 sel = *(volatile u8 *)0x03001768u;
    *(volatile u16 *)0x04000200u &= (u16)~(8u << sel);
    *(volatile u16 *)0x04000208u = *(volatile u16 *)0x03001774u;
}
#ifndef __APPLE__
void sub_0802DAE0(void) __attribute__((alias("_0802DAE0")));
#endif

// ============================================================================
// code_279c.s — BG/affine reset + title paint

void _08002844(void)
{
    // Preserve exact offsets and u16 widths (strh)
    *(volatile u16 *)0x04000010u = 0;
    *(volatile u16 *)0x04000012u = 0;
    *(volatile u16 *)0x04000014u = 0;
    *(volatile u16 *)0x04000016u = 0;
    *(volatile u16 *)0x04000020u = 0x0100;
    *(volatile u16 *)0x04000022u = 0;
    *(volatile u16 *)0x04000024u = 0;
    *(volatile u16 *)0x04000026u = 0x0100;
    *(volatile u16 *)0x04000028u = 0;
    *(volatile u16 *)0x0400002Au = 0;
    *(volatile u16 *)0x0400002Cu = 0;
    *(volatile u16 *)0x0400002Eu = 0;
    *(volatile u16 *)0x0400004Cu = 0;
    *(volatile u16 *)0x04000050u = 0;
    *(volatile u16 *)0x04000052u = 15;
    extern void sub_0802D974(const void *, void *, u32);
    u32 src = 0x02000000;
    u32 zero = 0;
    sub_0802D974(&zero, (void *)src, 0x05000100);
    volatile u32 *dma = (volatile u32 *)0x040000D4u;
    dma[0] = src;
    dma[1] = 0x07000000;
    dma[2] = 0x84000100;
    (void)dma[2];
}
#ifndef __APPLE__
void sub_08002844(void) __attribute__((alias("_08002844")));
#endif

void _08002924(void)
{
    _08003940(70, 0x0802E1F4u);
    volatile u32 *pair = (volatile u32 *)0x030035D0u;
    _08003940(90, pair[0]);
    _08003ADC(110, (int)pair[1]);
}
#ifndef __APPLE__
void sub_08002924(void) __attribute__((alias("_08002924")));
#endif

// 0x080032E0 is now spelled `sub_080032E0`, the closure's spelling. It was a
// real call all along -- asm/code_279c.s:174 is the matching `bl 0x080032E0` and
// the ROM there is real code (`push {r4, r5, r6, lr}`) -- so promotion_screen's
// "drop the caller" advice was correctly refused. The real defect was a DRIFTED
// label: asm/runtime_2aac.s had `.type sub_080032E0, %function` mid-function,
// assembling at 0x0800331C, and nothing at 0x080032E0 at all. The pair now sits
// at the entry, verified with `nm`, so the closure names the address and the
// call site follows it. Byte-neutral: a label emits nothing.
typedef struct { u32 w0, w1, w2, w3; } Runtime28CCArgs;

void _080028CC(void)
{
    Runtime28CCArgs args;
    u16 disp;
    _08002844();
    disp = (u16)(0x82u << 5);
    *(volatile u16 *)(0x80u << 19) = disp;
    _080056FC();
    _080056B8(0);
#ifndef __APPLE__
    sub_08003104((void *)0x030035D8u);
#else
    _08003104((void *)0x030035D8u);
#endif
    args = *(const Runtime28CCArgs *)0x0802E1E4u;
#ifndef __APPLE__
    sub_080032E0((void *)&args);   // the closure spelling; see the note above
#else
    _080032E0((void *)&args);      // host weak no-op
#endif
}
#ifndef __APPLE__
void sub_080028CC(void) __attribute__((alias("_080028CC")));
#endif

// 0x08002910 — `push {lr} / bl x3 / pop {r0} / bx r0` plus a 2-byte `movs r0,r0`
void _08002910(void)
{
#ifndef __APPLE__
    sub_08002B50();
    sub_08002BB4();
#else
    _08002B50();
    _08002BB4();
#endif
    _08002B44();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_08002910(void) __attribute__((alias("_08002910")));
#endif


// 2950 — thin wrapper: `push {lr} / bl ObjFlush_02C98 / pop {r0} / bx r0`,
// 10 B including the trailing pad. Labelled in asm/code_279c.s, which made it
// a known start, so the probe now needs a C candidate for it. A leaf that makes
// one call is exactly the shape agbcc frames, so this should be EXACT.
void Code2950_ObjFlush(void)
{
    sub_08002C98();
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08002950(void) __attribute__((alias("Code2950_ObjFlush")));
void sub_08002950(void) __attribute__((alias("Code2950_ObjFlush")));
#endif

// ============================================================================
// code_24e54.s — table scan + IWRAM wipe
// ============================================================================

int _080024E7C(u16 key)
{
    u32 rows[7];
    const u32 *src = (const u32 *)(uintptr_t)0x0805FC90u;
    for (int i = 0; i < 7; i++)
        rows[i] = src[i];
    s16 want = (s16)key;
    for (int r = 0; r <= 6; r++) {
        volatile u16 *p = (volatile u16 *)(uintptr_t)rows[r];
        if ((s16)p[0] == -1)
            continue;
        for (;;) {
            if ((s16)*p == want)
                return r;
            p++;
            if ((s16)*p == -1)
                break;
        }
    }
    return -1;
}
#ifndef __APPLE__
int sub_080024E7C(u16 k) __attribute__((alias("_080024E7C")));
int Sub_080024E7C(u16 k) __attribute__((alias("_080024E7C")));
#endif

void _080024EFC(void)
{
    u32 zero = 0;
    _0802D974(&zero, (void *)(uintptr_t)0x030015D8u, 0x05000003u);
}
#ifndef __APPLE__
void sub_080024EFC(void) __attribute__((alias("_080024EFC")));
#endif

// ============================================================================
// code_26068.s — award-table placement + OBJ wipe
// ============================================================================

void _080026088(int a, int b, int row)
{
    (void)a; (void)b;
    s16 v4 = _080261B0(12);
    s16 v5 = _080261F8(12);
    volatile u16 *ent = (volatile u16 *)(uintptr_t)(0x080CD9E4u + (u32)row * 12);
    u32 scratch[2];
    _0800798C((void *)(uintptr_t)*(volatile u32 *)(ent + 4), scratch);
    _08007ABC((void *)(uintptr_t)scratch[1], (int)(s16)ent[2], (int)v4);
    _080075E8((void *)(uintptr_t)*(volatile u32 *)(ent + 4),
              (int)(s16)ent[3], (int)v5);
    if ((s16)ent[0] == -1) {
        _08007CD0(scratch, (int)v4, (int)(s16)ent[2], (int)(s16)ent[1],
                  (int)v5, 0, 0);
    } else {
        _08007BFC(scratch, (int)v4, (int)(s16)ent[2], (int)(s16)ent[0],
                  (int)(s16)ent[1], (int)v5, 0, 0, 0);
    }
}
#ifndef __APPLE__
void sub_080026088(int a, int b, int c) __attribute__((alias("_080026088")));
#endif

void _08002612C(void)
{
    u32 zero = 0;
    _0802D974(&zero, (void *)(uintptr_t)0x030015F0u, 0x05000020u);
    _080056FC();
}
#ifndef __APPLE__
void sub_08002612C(void) __attribute__((alias("_08002612C")));
#endif

// ============================================================================
// code_23bd4.s — race presentation tick + BG bind
// ============================================================================

void _080023BD4(void *rec)
{
    volatile u8 *r = (volatile u8 *)rec;
    _0800D97C((void *)(r + 136), 15);
    s16 idx = *(volatile s16 *)(r + 140);
    const u32 *tbl = (const u32 *)(uintptr_t)0x080CC178u;
    _080023AE4(rec, (int)tbl[idx * 2 + 1],
               (int)*(volatile u32 *)(r + 168));
    _080023B60(rec, (int)tbl[idx * 2], (int)tbl[idx * 2 + 1]);
    _0800DBE8((void *)(r + 16));
    if (*(volatile u8 *)(0x03001780u + 0x10C3u) == 0 &&
        *(volatile u16 *)(r + 144) == 1) {
        _08022D44(rec, (void *)(r + 176), 48);
    }
    _08023E78(rec);
}
#ifndef __APPLE__
void sub_080023BD4(void *a) __attribute__((alias("_080023BD4")));
void _08023BD4(void *a) __attribute__((alias("_080023BD4")));
#endif

void _080023C54(void *rec, int mode)
{
    u16 tA[2], tB[2];
    _08002E0A4(tA, (const void *)(uintptr_t)0x0805FC84u, 4);
    _08002E0A4(tB, (const void *)(uintptr_t)0x0805FC88u, 4);
    if (mode == 2)
        mode = 0;
    volatile u32 *gate = (volatile u32 *)(uintptr_t)0x030005ACu;
    volatile u32 *flag = (volatile u32 *)(uintptr_t)0x080CC198u;
    volatile u8 *r = (volatile u8 *)rec;
    volatile u32 *p212 = (volatile u32 *)(r + 212);
    volatile u32 *p224 = (volatile u32 *)(r + 224);
    volatile u32 *p208 = (volatile u32 *)(r + 208);
    volatile u32 *p220 = (volatile u32 *)(r + 220);
    if (mode != (int)*gate || *flag == 0) {
        p212[0] = tA[mode];
        p224[0] = tB[mode];
        _08007ABC((void *)(uintptr_t)*(volatile u32 *)(r + 12),
                  (int)p212[0], (int)p208[0]);
        _08007ABC((void *)(uintptr_t)*(volatile u32 *)(r + 12),
                  (int)p224[0], (int)p220[0]);
    }
    void *base = (void *)(r + 8);
    _08007BFC(base, (int)p208[0], (int)p212[0], 168,
              96, 6, 1, 0, 0);
    _08007BFC(base, (int)p220[0], (int)p224[0], 40,
              96, 6, 1, 0, 0);
    *gate = (u32)mode;
    *flag = 1;
}
#ifndef __APPLE__
void sub_080023C54(void *a, int b) __attribute__((alias("_080023C54")));
void _08023C54(void *a, int b) __attribute__((alias("_080023C54")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void _08026088(int a, int b, int row) __attribute__((alias("_080026088")));
void _0802612C(void) __attribute__((alias("_08002612C")));
#endif
