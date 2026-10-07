// ============================================================================
// menu_ff78_h.c — reconstructed C for asm/menu_ff78.s (5 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080014650 (0x080014650) — (rec, a1, a2): 4x6 7BFC grid.
//   sub_080014BD0 (0x080014BD0) — (ev, a1, a2, rec): 12-way dispatcher.
//   sub_080013950 (0x080013950) — (rec, arr, sl, cnt): counted 7B18 loop.
//   sub_0800139F0 (0x0800139F0) — (rec, u16): gated 7B18 pairs.
//   sub_080014078 (0x080014078) — (rec): memcpy + 4x6 grid-sample loop.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
// The lower-case twin, needed because MenuFF78_13950 (0x08013950, promoted) now
// calls 0x08007b18 under the CLOSURE's spelling: the spliced link holds
// asm/code.s plus that one body and no C object, so `Sub_` has nothing to bind
// to. Both arms need the split, not just the declaration: a bare `extern` under
// `#ifndef __APPLE__` leaves the host build with an undefined symbol that
// `-undefined dynamic_lookup` binds lazily, so the suite still reads green.
__attribute__((weak)) void sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) void Sub_08007ABC(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) int Sub_08025CF4(int a, int b, int c) { (void)a; (void)b; (void)c; return 0; }
__attribute__((weak)) void Sub_08002E0A4(void *d, const void *s, u32 n) { (void)d; (void)s; (void)n; }
__attribute__((weak)) void Sub_0800141AC(void *a) { (void)a; }
__attribute__((weak)) void Sub_08001411C(int a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800D854(void *a) { (void)a; }
__attribute__((weak)) void Sub_0800D8E4(void *a) { (void)a; }
__attribute__((weak)) void Sub_080013FE4(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014AC4(void *a) { (void)a; }
__attribute__((weak)) void Sub_080014488(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800141A8(void *a) { (void)a; }
// Closure spellings for the MenuFF78_14BD0 (0x08014BD0, promoted) call sites:
// the spliced link holds asm/code.s plus that one body and no C object, so a
// `Sub_` call has nothing to bind to. Host gets weak no-ops (same as the
// Sub_ twins above); ARM gets externs resolving to the real bodies.
__attribute__((weak)) void sub_08001411C(int a, void *b) { (void)a; (void)b; }
__attribute__((weak)) void sub_0800D854(void *a) { (void)a; }
__attribute__((weak)) void sub_0800D8E4(void *a) { (void)a; }
__attribute__((weak)) void _080013FE4(void *a) { (void)a; }
__attribute__((weak)) void sub_080014AC4(void *a) { (void)a; }
__attribute__((weak)) void sub_080014488(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void sub_0800141AC(void *a) { (void)a; }
__attribute__((weak)) void sub_0800141A8(void *a) { (void)a; }
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void Sub_08007ABC(u32 a, u32 b, u32 c);
extern int Sub_08025CF4(int a, int b, int c);
extern void Sub_08002E0A4(void *d, const void *s, u32 n);
extern void Sub_0800141AC(void *a);
extern void Sub_08001411C(int a, void *b);
extern void Sub_0800D854(void *a);
extern void Sub_0800D8E4(void *a);
extern void Sub_080013FE4(void *a);
extern void Sub_080014AC4(void *a);
extern void Sub_080014488(void *a, u32 b, u32 c);
extern void Sub_0800141A8(void *a);
// Closure spellings called by the promoted MenuFF78_14BD0 body (see above).
extern void sub_08001411C(int a, void *b);
extern void sub_0800D854(void *a);
extern void sub_0800D8E4(void *a);
extern void _080013FE4(void *a);
extern void sub_080014AC4(void *a);
extern void sub_080014488(void *a, u32 b, u32 c);
extern void sub_0800141AC(void *a);
extern void sub_0800141A8(void *a);
#endif

// ----------------------------------------------------------------------------
// sub_080014650 — (rec, a1, a2): 4 outer x 6 inner 7BFC grid.
//   outer idx 0..3: base = idx*72; r8v = a2 + idx*8.
//   inner k 0..5: 7BFC(rec+24, u32[rec+248+base+k*12],
//   u32[rec+252+base+k*12], a1, r8v, 11, 1, 0, 0).
void MenuFF78_14650(void *rec_, u32 a1, u32 a2) {
    volatile u8 *rec = (volatile u8 *)rec_;
    for (u32 idx = 0; idx <= 3; idx++) {
        u32 base = idx * 72;
        u32 r8v = a2 + idx * 8;
        for (u32 k = 0; k < 6; k++) {
            u32 w1 = *(volatile u32 *)(uintptr_t)(rec + 248 + base + k * 12);
            u32 w2 = *(volatile u32 *)(uintptr_t)(rec + 252 + base + k * 12);
            Sub_08007BFC((void *)(uintptr_t)(rec + 24), (int)w1, (int)w2,
                         (int)a1, r8v, 11, 1, 0, 0);
        }
    }
}
#ifndef __APPLE__
void _080014650(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_14650")));
void Sub_080014650(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_14650")));
void sub_080014650(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_14650")));
#endif

// ----------------------------------------------------------------------------
// sub_080014BD0 — (ev, a1, a2, rec): ev-1 indexes a 12-entry table.
//   0->141AC(rec), 1->1411C(rec,a1), 4->D854(rec+32)+D8E4(rec+136)+13FE4(rec),
//   5->14488(rec,(u16)a1,(u16)a2) if u16[rec+36]!=0, 6->14AC4(rec),
//   11->141A8(rec), else no-op.
void MenuFF78_14BD0(int ev, u32 a1, u32 a2, void *rec) {
    int idx = ev - 1;
    if (idx < 0 || idx > 11) return;
    switch (idx) {
    case 1:
        sub_08001411C((int)(uintptr_t)rec, (void *)(uintptr_t)a1);
        break;
    case 4:
        sub_0800D854((void *)(uintptr_t)((u8 *)rec + 32));
        sub_0800D8E4((void *)(uintptr_t)((u8 *)rec + 136));
        _080013FE4(rec);
        break;
    case 6:
        sub_080014AC4(rec);
        break;
    case 5:
        if (*(volatile u16 *)(uintptr_t)((u8 *)rec + 36) != 0)
            sub_080014488(rec, (u16)a1, (u16)a2);
        break;
    case 0:
        sub_0800141AC(rec);
        break;
    case 11:
        sub_0800141A8(rec);
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _080014BD0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_14BD0")));
void sub_080014BD0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_14BD0")));
#endif

// ----------------------------------------------------------------------------
// sub_080013950 — (rec, arr, sl, cnt): u8 n = (u8)cnt clamped to 0..32;
//   m = (s8)n, q = m/2, rem = (s8)(m%2). Loop i in 0..q-1 calls
//   7B18(rec,25,arr+i*8,sl,3,1,1,0); if rem==1 then
//   7B18(rec,24,arr+i*8,sl,3,1,1,0) — the SAME i variable, so the loop
//   induction variable is live past the loop and agbcc emits its do-while
//   rotation (i=0; cmp i,q; bge; i=q; body; --i).
void MenuFF78_13950(void *rec_, void *arr_, u32 sl_, u32 cnt_) {
    void *rec = rec_;
    volatile u8 *arr = (volatile u8 *)arr_;
    u32 s = sl_;
    s8 m;
    int q;
    int rem;
    int i;
    u8 n = (u8)cnt_;
    if ((s8)cnt_ > 31) n = 32;
    if ((s8)n <= 0) n = 0;
    m = (s8)n;
    q = m / 2;
    rem = (s8)(m % 2);
    // `sub_08007B18`, not `Sub_08007B18`: the closure's spelling at 0x08007b18 is
    // the lower-case `sub_` form, and the spliced link contains asm/code.s plus
    // this body's section and NO C object, so a `Sub_` call has nothing to bind
    // to. Both spellings are one-hop aliases of `CourseEmit7B18` in the full C
    // build, so only these two call sites in THIS body move; the sibling bodies
    // below keep `Sub_`, which their own not-yet-promoted status still resolves.
    for (i = 0; i < q; i++) {
        sub_08007B18(rec, 25, (int)(uintptr_t)(arr + (u32)i * 8), (int)s,
                     3, 1, 1, 0);
    }
    if (rem == 1) {
        sub_08007B18(rec, 24, (int)(uintptr_t)(arr + (u32)i * 8), (int)s,
                     3, 1, 1, 0);
    }
}
// The body is 158 bytes, two short of its section's 4-byte alignment; gas
// closes a Thumb code section with `nop` (0x46c0) where the ROM holds
// `00 00`. Same file-scope pad as MenuFF78_11CBC in menu_ff78_d.c.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080013950(void *a, void *b, u32 c, u32 d) __attribute__((alias("MenuFF78_13950")));
void Sub_080013950(void *a, void *b, u32 c, u32 d) __attribute__((alias("MenuFF78_13950")));
void sub_080013950(void *a, void *b, u32 c, u32 d) __attribute__((alias("MenuFF78_13950")));
#endif

// ----------------------------------------------------------------------------
// sub_0800139F0 — (rec, u16 a1):
//   if s16[rec+186]==0 and a1!=0:
//     r2 = s16[rec+166]; r2==0: 7B18(rec+68,11,112,80,3,1,1,0);
//     r2==1: 7B18(rec+68,14,112,80,3,1,1,0).
//   if (u16)a1 != s16[rec+184]:
//     r2 = s16[rec+166]; r2==0: 7B18(rec+68,15,120,80,3,1,1,0);
//     r2==1: 7B18(rec+68,16,120,80,3,1,1,0).
void MenuFF78_139F0(void *rec_, u32 a1_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 a1 = (u16)a1_;
    if (*(volatile s16 *)(uintptr_t)(rec + 186) == 0 && a1 != 0) {
        s16 r2 = *(volatile s16 *)(uintptr_t)(rec + 166);
        if (r2 == 0) {
            Sub_08007B18((void *)(uintptr_t)(rec + 68), 11, 112, 80,
                         3, 1, 1, 0);
        } else if (r2 == 1) {
            Sub_08007B18((void *)(uintptr_t)(rec + 68), 14, 112, 80,
                         3, 1, 1, 0);
        }
    }
    if ((int)a1 != (int)*(volatile s16 *)(uintptr_t)(rec + 184)) {
        s16 r2 = *(volatile s16 *)(uintptr_t)(rec + 166);
        if (r2 == 0) {
            Sub_08007B18((void *)(uintptr_t)(rec + 68), 15, 120, 80,
                         3, 1, 1, 0);
        } else if (r2 == 1) {
            Sub_08007B18((void *)(uintptr_t)(rec + 68), 16, 120, 80,
                         3, 1, 1, 0);
        }
    }
}
#ifndef __APPLE__
void _0800139F0(void *a, u32 b) __attribute__((alias("MenuFF78_139F0")));
void Sub_0800139F0(void *a, u32 b) __attribute__((alias("MenuFF78_139F0")));
void sub_0800139F0(void *a, u32 b) __attribute__((alias("MenuFF78_139F0")));
#endif

// ----------------------------------------------------------------------------
// sub_080014078 — (rec): memcpy(spad, 0x0805F9C0, 32); 4 outer x 6 inner:
//   g = 25CF4(s16[0x03002772], outer, inner*2);
//   h = 25CF4(s16[0x03002772], outer, r8) with r8 = 1,3,5,7,9,11;
//   s16v = spad[(h*2 + g*8)];
//   u32[rec+252+outer*72+k*12] = (u32)s16v;
//   7ABC(u32[rec+28], (u32)s16v, u32[rec+248+outer*72+k*12]).
void MenuFF78_14078(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u8 spad[32];
    Sub_08002E0A4(spad, (const void *)(uintptr_t)0x0805F9C0u, 32);
    for (u32 outer = 0; outer <= 3; outer++) {
        u32 base = outer * 72;
        u32 r8 = 1;
        for (u32 inner = 0; inner <= 5; inner++) {
            int cell = (int)*(volatile s16 *)(uintptr_t)0x03002772u;
            int g = Sub_08025CF4(cell, (int)outer, (int)(inner * 2));
            int h = Sub_08025CF4(cell, (int)outer, (int)r8);
            u32 idx = (((u32)h) << 1) + (((u32)g) << 3);
            s16 v = *(volatile s16 *)(uintptr_t)(spad + idx);
            *(volatile u32 *)(uintptr_t)(rec + 252 + base + inner * 12) =
                (u32)(s32)v;
            u32 w0 = *(volatile u32 *)(uintptr_t)(rec + 28);
            u32 w2 = *(volatile u32 *)(uintptr_t)(rec + 248 + base + inner * 12);
            Sub_08007ABC(w0, (u32)(s32)v, w2);
            r8 += 2;
        }
    }
}
#ifndef __APPLE__
void _080014078(void *a) __attribute__((alias("MenuFF78_14078")));
void Sub_080014078(void *a) __attribute__((alias("MenuFF78_14078")));
void sub_080014078(void *a) __attribute__((alias("MenuFF78_14078")));
#endif
