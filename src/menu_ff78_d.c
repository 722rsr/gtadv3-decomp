// ============================================================================
// menu_ff78_d.c — reconstructed C for asm/menu_ff78.s course-record filler
// family (sub_08007B18 callers).
//
//   sub_080011784 (0x080011784) — lifted in scene_record_dispatch.c as Sub_080011784
//   sub_080011854 (0x080011854) — variant with table lookup 0x080CB5A6/AA.
//   sub_0800118DC (0x0800118DC) — variant: if r5==0 / r5!=2 dispatch.
//   sub_080011958 (0x080011958) — variant.
//   sub_080011B48 (0x080011B48) — variant.
//   sub_080011CBC (0x080011CBC) — variant.
//   sub_080011CFC (0x080011CFC) — variant (smaller).
//   sub_080011D48 (0x080011D48) — variant.
//   sub_080011DC4 (0x080011DC4) — variant.
//   sub_080011E10 (0x080011E10) — variant.
//   sub_080011E58 (0x080011E58) — variant.
//   sub_080011EE0 (0x080011EE0) — variant.
//   sub_080011F68 (0x080011F68) — variant.
//   sub_080011FF0 (0x080011FF0) — variant.
//   sub_080012A44 (0x080012A44) — variant.
//   sub_080012AC0 (0x080012AC0) — variant.
//   sub_080012B34 (0x080012B34) — variant.
//   sub_080012B68 (0x080012B68) — variant.
//   sub_080013950 (0x080013950) — variant.
//   sub_0800139F0 (0x0800139F0) — variant.
//   sub_080013AA8 (0x080013AA8) — variant.
//   sub_080013AF4 (0x080013AF4) — variant.
//   sub_0800146D0 (0x0800146D0) — variant.
//   sub_08001479C (0x08001479C) — variant.
//   sub_080014A08 (0x080014A08) — variant.
//   sub_080015930 (0x080015930) — variant.
//   sub_0800159AC (0x0800159AC) — variant.
//
// All are pure-Thumb leaves that read s16[rec+168/192/194/198/216/220/226] and
// call sub_08007B18 with stack args. The shared ABI for _08007B18 is
// (rec, dst_idx, count, offset, a4, a5, a6, a7); we model it as a vararg
// helper below.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

// _08007B18 = sub_08007B18 — course-record placement helper.
// ABI: (rec, dst_idx, count, offset, a4, a5, a6, a7).
// We model the 4 stack args explicitly to keep stack-position semantics
// (the asm pushes {sp, 0/4/8/12} and the builder reads them).
extern void _08007B18(void *rec, int dst, int kind, int off, u32 a4, u32 a5, u32 a6, u32 a7);
extern void _08007BFC(void *dst, int a, int b, int c, int d, int e, int f, int g, int h);
extern u32  _0800F6D0(s32 c);                   // menus.c MenuF6D0
extern void Sub_080011784(void *a);           // scene_record_dispatch.c (lifted 0x080011784)
HOST_STUB(int _08024AC(void));

static inline u16 RD16(volatile u8 *rec, int off)  { return *(volatile u16 *)(rec + off); }
static inline u32 RD32(volatile u8 *rec, int off)  { return *(volatile u32 *)(rec + off); }
static inline s16 RDS16(volatile u8 *rec, int off){ return *(volatile s16 *)(rec + off); }

// Helper to read a s16 from a ROM table.
static inline s16 tbl_s16(u32 base, int idx) {
    return *(volatile s16 *)(uintptr_t)(base + 2 * idx);
}

// ----------------------------------------------------------------------------
// sub_080011854 — variant: r5=(u16)a1; r6=_0800F6D0(s16[rec+188])-1.
// if r5 != 0: build stack {3,1,1,0} + table[0x080CB5A6 + 2*s16[rec+226]]
//   sub_08007B18(rec, s16[rec+100], kind=8, count=112,...).
// if r5 != r6: similar with table[0x080CB5AA + 2*idx], kind=16.
// ----------------------------------------------------------------------------
void MenuFF78_11854(void *rec_, int a1) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 r5 = (u16)a1;
    s16 bc = RDS16(rec, 188);
    u32 r6 = _0800F6D0(bc) - 1;
    if (r5 != 0) {
        s16 idx = RDS16(rec, 226);
        s16 tab = tbl_s16(0x080CB5A6u, idx);
        s16 v = RDS16(rec, 100);
        u32 sp[4] = {3, 1, 1, 0};
        _08007B18(rec_, (s32)v, 8, 112, sp[0], sp[1], sp[2], sp[3]);
        (void)tab;
    }
    if (r5 != (u16)r6) {
        s16 idx = RDS16(rec, 226);
        s16 tab = tbl_s16(0x080CB5AAu, idx);
        s16 v = RDS16(rec, 100);
        u32 sp[4] = {3, 1, 1, 0};
        _08007B18(rec_, (s32)v, 16, 112, sp[0], sp[1], sp[2], sp[3]);
        (void)tab;
    }
}
#ifndef __APPLE__
void _080011854(void *a, int b) __attribute__((alias("MenuFF78_11854")));
void Sub_080011854(void *a, int b) __attribute__((alias("MenuFF78_11854")));
void sub_080011854(void *a, int b) __attribute__((alias("MenuFF78_11854")));
#endif

// ----------------------------------------------------------------------------
// sub_0800118DC — variant: r5=(u16)a1.
// if r5 != 0: stack {3,1,1,0}, table 0x080CB5A6 -> s16[rec+100] as kind, off=112
// if r5 != 2: similar w/ table 0x080CB5AA, kind=16, off=112
// ----------------------------------------------------------------------------
void MenuFF78_118DC(void *rec_, int a1) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 r5 = (u16)a1;
    if (r5 != 0) {
        s16 idx = RDS16(rec, 226);
        (void)tbl_s16(0x080CB5A6u, idx);
        s16 v = RDS16(rec, 100);
        u32 sp[4] = {3, 1, 1, 0};
        _08007B18(rec_, (s32)v, 8, 112, sp[0], sp[1], sp[2], sp[3]);
    }
    if (r5 != 2) {
        s16 idx = RDS16(rec, 226);
        (void)tbl_s16(0x080CB5AAu, idx);
        s16 v = RDS16(rec, 100);
        u32 sp[4] = {3, 1, 1, 0};
        _08007B18(rec_, (s32)v, 16, 112, sp[0], sp[1], sp[2], sp[3]);
    }
}
#ifndef __APPLE__
void _0800118DC(void *a, int b) __attribute__((alias("MenuFF78_118DC")));
void Sub_0800118DC(void *a, int b) __attribute__((alias("MenuFF78_118DC")));
void sub_0800118DC(void *a, int b) __attribute__((alias("MenuFF78_118DC")));
#endif

// ============================================================================
// sub_080011CBC — 36B leaf: if u16[rec+168]!=3, call _08007BFC(rec+16,
// u32[rec+252], u32[rec+256], 88, stack {120,8,1,1,0}).
// ============================================================================
void MenuFF78_11CBC(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    if (RD16(rec, 168) == 3) return;
    void *dst = (void *)(uintptr_t)(rec + 16);
    u32 r1 = RD32(rec, 252);
    u32 r2 = RD32(rec, 256);
    _08007BFC(dst, (int)r1, (int)r2, 88, 120, 8, 1, 1, 0);
}
// The body is 62 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080011CBC(void *a) __attribute__((alias("MenuFF78_11CBC")));
void Sub_080011CBC(void *a) __attribute__((alias("MenuFF78_11CBC")));
void sub_080011CBC(void *a) __attribute__((alias("MenuFF78_11CBC")));
#endif

// ============================================================================
// sub_080011CFC — 38B leaf: _08007B18(rec, 9, 152, 88, {3,1,1,0}) +
// _08007BFC(rec+40, u32[rec+288], u32[rec+292], 152, {64,3,1,1,0}).
// ============================================================================
void MenuFF78_11CFC(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    _08007B18(rec_, 9, 152, 88, 3, 1, 1, 0);
    void *dst = (void *)(uintptr_t)(rec + 40);
    u32 r1 = RD32(rec, 288);
    u32 r2 = RD32(rec, 292);
    _08007BFC(dst, (int)r1, (int)r2, 152, 64, 3, 1, 1, 0);
}
#ifndef __APPLE__
void _080011CFC(void *a) __attribute__((alias("MenuFF78_11CFC")));
void Sub_080011CFC(void *a) __attribute__((alias("MenuFF78_11CFC")));
void sub_080011CFC(void *a) __attribute__((alias("MenuFF78_11CFC")));
#endif

// ============================================================================
// sub_080011DC4 — same body as 0x011CFC (38B): _08007B18(rec, 9, 152, 88,
// {3,1,1,0}) + _08007BFC(rec+40, u32[rec+288], u32[rec+292], 152, {64,3,1,1,0}).
// ============================================================================
void MenuFF78_11DC4(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    _08007B18(rec_, 9, 152, 88, 3, 1, 1, 0);
    void *dst = (void *)(uintptr_t)(rec + 40);
    u32 r1 = RD32(rec, 288);
    u32 r2 = RD32(rec, 292);
    _08007BFC(dst, (int)r1, (int)r2, 152, 64, 3, 1, 1, 0);
}
#ifndef __APPLE__
void _080011DC4(void *a) __attribute__((alias("MenuFF78_11DC4")));
void Sub_080011DC4(void *a) __attribute__((alias("MenuFF78_11DC4")));
void sub_080011DC4(void *a) __attribute__((alias("MenuFF78_11DC4")));
#endif

extern void _08026FC4(int a, void *b, int c, int d, int e, int f); // garage.c Garage_FC4
void MenuFF78_11E10(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    void *dst = (void *)(uintptr_t)(rec + 24);
    u32 r1 = RD32(rec, 228);
    u32 r2 = RD32(rec, 232);
    _08007BFC(dst, (int)r1, (int)r2, 80, 36, 10, 1, 1, 0);
    _08026FC4((int)RD32(rec, 240), (void *)(uintptr_t)80, 48, 7, 1, 1);
}
#ifndef __APPLE__
void _080011E10(void *a) __attribute__((alias("MenuFF78_11E10")));
void Sub_080011E10(void *a) __attribute__((alias("MenuFF78_11E10")));
void sub_080011E10(void *a) __attribute__((alias("MenuFF78_11E10")));
#endif

// ============================================================================
// sub_080011FF0 — 36B leaf: if (u16[rec+194]-1) <= 2, call
// _08007BFC(rec+32, u32[rec+0x142], u32[rec+0x146], 208, {120,6,1,1,0}).
// ============================================================================
void MenuFF78_11FF0(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 r = (u16)(RD16(rec, 194) - 1);
    if ((u32)r > 2) return;
    void *dst = (void *)(uintptr_t)(rec + 32);
    u32 r1 = RD32(rec, 0x144);
    u32 r2 = RD32(rec, 0x148);
    _08007BFC(dst, (int)r1, (int)r2, 208, 120, 6, 1, 1, 0);
}
#ifndef __APPLE__
void _080011FF0(void *a) __attribute__((alias("MenuFF78_11FF0")));
void Sub_080011FF0(void *a) __attribute__((alias("MenuFF78_11FF0")));
void sub_080011FF0(void *a) __attribute__((alias("MenuFF78_11FF0")));
#endif

// ============================================================================
// sub_080012B34 — 36B leaf: _08007B18(rec, u16[0x080CB68C + 2*s16[rec+128]],
// 104, 32, {6,1,0,0}).
//
// Two source shapes are load-bearing here, and both were pinned by compiling
// this body against the ROM bytes (every other formulation is 39-44/52):
//
// 1. The record field must be read through a *typed* `s16 *`, not through
//    `RDS16`/`*(volatile s16 *)`. Over a `volatile` cast agbcc emits
//    `ldrh` + `lsls #16` + `asrs #15` (three instructions); through a plain
//    `s16 *` it emits the ROM's `movs r3,#0` + `ldrsh`. Same trick as the
//    `s16 buf[12]` in menu_ff78_f.c's MenuFF78_12E74.
// 2. The table base must be a SYMBOL_REF leaf, not an integer literal. With
//    `tbl_s16(0x080CB68C, idx)` agbcc constant-folds the address and issues
//    the pool `ldr` at its use, so the scratch for the `ldrsh` takes r2 and
//    the `ldr` then reuses r2. Through the symbol the base is materialised
//    first, occupies r2, and the scratch falls to r3 — the ROM's allocation.
//    (Same reason as the SaveTrigTbl* symbols in save.c.) The `__asm__` must
//    sit inside this body: the per-body splice extracts only the
//    brace-matched body, so a file-scope definition would leave the
//    reference undefined. It follows the declarations because C89 forbids a
//    declaration after the `__asm__` statement.
// ============================================================================
void MenuFF78_12B34(void *rec_) {
    extern const s16 _080CB68C[];
    const s16 *tab = _080CB68C;
    s16 *q = (s16 *)((u8 *)rec_ + 128);
    s16 idx = *q;
    u16 v = (u16)tab[idx];
    __asm__(".globl _080CB68C\n_080CB68C = 0x080CB68C\n");
    _08007B18(rec_, (int)v, 104, 32, 6, 1, 0, 0);
}
#ifndef __APPLE__
void _080012B34(void *a) __attribute__((alias("MenuFF78_12B34")));
void Sub_080012B34(void *a) __attribute__((alias("MenuFF78_12B34")));
void sub_080012B34(void *a) __attribute__((alias("MenuFF78_12B34")));
#endif

// ============================================================================
// sub_080012AC0 — switch on u32[rec+140]: case 0/2 -> stack {3,1,0,0}
// kind=r1; case 1/3 -> stack {4,1,0,0} kind=r1. r3 = arg passed.
// u16 idx = u16[0x080CB678 + 2*s16[rec+128]] (read before switch).
// ============================================================================
void MenuFF78_12AC0(void *rec_, int a1, int a2) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 s128 = RDS16(rec, 128);
    u16 v = (u16)tbl_s16(0x080CB678u, (int)s128);
    u32 sel = RD32(rec, 140);
    u32 sp[4];
    if (sel == 1 || sel == 3) {
        sp[0] = 4; sp[1] = 1; sp[2] = 0; sp[3] = 0;
        _08007B18(rec_, (int)v, (int)a1, (int)a2, sp[0], sp[1], sp[2], sp[3]);
    } else {  // 0 or 2 or other
        sp[0] = 3; sp[1] = 1; sp[2] = 0; sp[3] = 0;
        _08007B18(rec_, (int)v, (int)a1, (int)a2, sp[0], sp[1], sp[2], sp[3]);
    }
}
#ifndef __APPLE__
void _080012AC0(void *a, int b, int c) __attribute__((alias("MenuFF78_12AC0")));
void Sub_080012AC0(void *a, int b, int c) __attribute__((alias("MenuFF78_12AC0")));
void sub_080012AC0(void *a, int b, int c) __attribute__((alias("MenuFF78_12AC0")));
#endif

// ============================================================================
// sub_0800146D0 — 5-builder filler (with high-register ldrh/strh).
// r8 = s16[rec+164] if s16[rec+166] in [0..3] else 0.
// r6 = u16[rec+36]. r7 = rec+8.
// if r6==1: _08007B18(rec+8, 5, r8-24, 64, {5,1,0,0}) + r8+216.
// always: _08007B18(rec+8, 5, r8+24/+72/+120/+168, 64, {5,1,0,0}).
// ============================================================================
void MenuFF78_146D0(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 s166 = RDS16(rec, 166);
    s16 r8v = 0;
    if (s166 >= 0 && s166 <= 3) r8v = RDS16(rec, 164);
    u16 r6 = RD16(rec, 36);
    void *r7 = (void *)(uintptr_t)(rec + 8);
    if (r6 == 1) {
        _08007B18(r7, 5, (int)r8v - 24, 64, 5, 1, 0, 0);
        _08007B18(r7, 5, (int)r8v + 216, 64, 5, 1, 0, 0);
    }
    _08007B18(r7, 5, (int)r8v + 24,   64, 5, 1, 0, 0);
    _08007B18(r7, 5, (int)r8v + 72,   64, 5, 1, 0, 0);
    _08007B18(r7, 5, (int)r8v + 120,  64, 5, 1, 0, 0);
    _08007B18(r7, 5, (int)r8v + 168,  64, 5, 1, 0, 0);
}
#ifndef __APPLE__
void _0800146D0(void *a) __attribute__((alias("MenuFF78_146D0")));
void Sub_0800146D0(void *a) __attribute__((alias("MenuFF78_146D0")));
void sub_0800146D0(void *a) __attribute__((alias("MenuFF78_146D0")));
#endif

// ============================================================================
// sub_080015930 — 29-iteration loop calling _08007B18 with r4+=8 each iter.
// if r6==2: stack {3,1,1,0}; if r6==3: stack {4,1,1,0}.
// rec arg = r8+52, kind=7, off=r4, count=r7.
// ============================================================================
void MenuFF78_15930(void *rec_, int a1, int a2) {
    volatile u8 *rec = (volatile u8 *)rec_;
    int r7 = a1;
    int r6 = a2;
    int one = 1;
    int zero = 0;
    for (int r4 = 0, r9 = 29; r9 >= 0; r4 += 8, r9--) {
        switch (r6) {
        case 2:
            _08007B18((void *)(uintptr_t)(rec + 52), 7, r4, r7, 3, one, one, zero);
            break;
        case 3:
            _08007B18((void *)(uintptr_t)(rec + 52), 7, r4, r7, 4, one, one, zero);
            break;
        }
    }
}
#ifndef __APPLE__
void _080015930(void *a, int b, int c) __attribute__((alias("MenuFF78_15930")));
void Sub_080015930(void *a, int b, int c) __attribute__((alias("MenuFF78_15930")));
void sub_080015930(void *a, int b, int c) __attribute__((alias("MenuFF78_15930")));
#endif

// ============================================================================
// sub_0800159AC — switch on u32[rec+164]: case 0/2 -> {3,1,1,0}; case 1/3 -> {4,1,1,0}.
// rec arg = r4, kind=r1, off=r5, count=r6.
// ============================================================================
void MenuFF78_159AC(void *rec_, int a1, int a2, int a3) {
    // FINDING : the ROM's dispatch is a hand-rolled ladder, not a
    // two-armed if/else. `cmp #1; beq @ 0x080159D2` jumps forward to the
    // `{4,1,1,0}` block; `cmp #1; bgt @ 0x080159C2` puts the `> 1` sub-ladder
    // out of line; both `== 0` (0x080159C6) and `== 2` (0x080159CC) branch to
    // the shared `{3,1,1,0}` block at 0x080159EC; and `== 3` FALLS THROUGH
    // into the `{4,...}` block, so `sel == 3` must be the last test.
    // `bgt @ 0x080159C2` is a SIGNED compare, so `sel` is `int`, not `u32`
    // (a `u32` test emits `bhi`). The call passes (r0,r1,r2,r3) =
    // (rec, a3, a1, a2): the ROM copies a3 into r1 up front
    // (`adds r1,r3,#0 @ 0x080159B6`) and reloads r0/r2/r3 from r4/r5/r6 at
    // each call site.
#ifndef __APPLE__
    // The ROM keeps `sel` in r0 for the WHOLE ladder (`cmp r0,#1` at
    // 0x080159BC..0x080159CE all read r0); unpinned, agbcc copies it to r2
    // and emits an `adds r2,r0,#0` the ROM does not have.
    register int sel __asm__("r0") = (int)RD32((volatile u8 *)rec_, 164);
#else
    int sel = (int)RD32((volatile u8 *)rec_, 164);
#endif
    if (sel == 1) goto four;
    if (sel <= 1) {
        if (sel == 0) goto three;
        goto end;
    }
    if (sel == 2) goto three;
    if (sel != 3) goto end;
four:
    _08007B18(rec_, a3, a1, a2, 4, 1, 1, 0);
    goto end;
three:
    _08007B18(rec_, a3, a1, a2, 3, 1, 1, 0);
end:
    return;
}
#ifndef __APPLE__
void _0800159AC(void *a, int b, int c, int d) __attribute__((alias("MenuFF78_159AC")));
void sub_0800159AC(void *a, int b, int c, int d) __attribute__((alias("MenuFF78_159AC")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void Sub_0800159AC(void *rec_, int a1, int a2, int a3) __attribute__((alias("MenuFF78_159AC")));
#endif
