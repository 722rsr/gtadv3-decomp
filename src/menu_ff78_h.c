// ============================================================================
// menu_ff78_h.c — reconstructed C for asm/menu_ff78.s (5 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the closure spellings defined in the asm listings,
// so behavior is identical by construction.
//
//   sub_080014650 (0x080014650) — (rec, a1, a2): 4x6 7BFC grid.
//   sub_080014BD0 (0x080014BD0) — (ev, a1, a2, rec): 12-way dispatcher.
//   sub_080013950 (0x080013950) — (rec, arr, sl, cnt): counted 7B18 loop.
//   sub_0800139F0 (0x0800139F0) — (rec, u16): gated 7B18 pairs.
//   sub_080014078 (0x080014078) — (rec): memcpy + 4x6 grid-sample loop.
// ============================================================================

#include "gba/types.h"

extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void _08007ABC(u32 a, u32 b, u32 c);
extern int _08025CF4(int a, int b, int c);
extern void _0802E0A4(void *d, const void *s, u32 n);
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

// ----------------------------------------------------------------------------
// sub_080014650 — (rec, a1, a2): 4 outer x 6 inner 7BFC grid.
//   outer idx 0..3: base = idx*72; r8v = a2 + idx*8.
//   inner k 0..5: 7BFC(rec+24, u32[rec+248+base+k*12],
//   u32[rec+252+base+k*12], a1, r8v, 11, 1, 0, 0).
void MenuFF78_14650(void *rec_, u32 a1, u32 a2) {
    volatile struct {
        u32 a1;
        u32 a2;
    } args;
    register volatile u8 *rec __asm__("r3") = (volatile u8 *)rec_;
    __asm__ volatile("" : "+r"(rec));
    args.a1 = a1;
    args.a2 = a2;
    register int idx __asm__("r0") = 0;
    register int next_idx __asm__("r9");
    register void *target __asm__("r10") = (void *)(uintptr_t)(rec + 24);
    __asm__ volatile("" : "+r"(target));
    volatile u32 *base1 = (volatile u32 *)(rec + 248);
    volatile u32 *base2 = (volatile u32 *)(rec + 252);
    do {
        __asm__ volatile("" : "+r"(target));
        register u32 lane_off __asm__("r1") = (u32)idx * 8u;
        __asm__ volatile("" : "+r"(lane_off));
        next_idx = idx + 1;
        __asm__ volatile("" : "+r"(next_idx));
        u32 row = (lane_off + (u32)idx) * 8u;
        u32 r8v = args.a2 + lane_off;
        u32 arg = args.a1;
        volatile u32 *p2 = (volatile u32 *)(row + (uintptr_t)base2);
        volatile u32 *p1 = (volatile u32 *)(row + (uintptr_t)base1);
        int k;
        for (k = 5; k >= 0; k--) {
            sub_08007BFC(target, (int)*p1, (int)*p2, (int)arg,
                         r8v, 11, 1, 0, 0);
            arg += 16;
            p2 += 3;
            p1 += 3;
        }
        idx = next_idx;
    } while (idx <= 3);
}
void _080014650(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_14650")));
void Sub_080014650(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_14650")));
void sub_080014650(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_14650")));

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
void _080014BD0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_14BD0")));
void sub_080014BD0(int a, u32 b, u32 c, void *d) __attribute__((alias("MenuFF78_14BD0")));

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
void _080013950(void *a, void *b, u32 c, u32 d) __attribute__((alias("MenuFF78_13950")));
void Sub_080013950(void *a, void *b, u32 c, u32 d) __attribute__((alias("MenuFF78_13950")));
void sub_080013950(void *a, void *b, u32 c, u32 d) __attribute__((alias("MenuFF78_13950")));

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
    if (*(s16 *)(uintptr_t)(rec + 186) == 0) {
        if (a1 != 0) {
            switch (*(s16 *)(uintptr_t)(rec + 166)) {
            case 0:
                sub_08007B18((void *)(uintptr_t)(rec + 68), 11, 112, 80,
                             3, 1, 1, 0);
                break;
            case 1:
                sub_08007B18((void *)(uintptr_t)(rec + 68), 14, 112, 80,
                             3, 1, 1, 0);
                break;
            }
        }
        if ((int)a1 != (int)*(s16 *)(uintptr_t)(rec + 184)) {
            switch (*(s16 *)(uintptr_t)(rec + 166)) {
            case 0:
                sub_08007B18((void *)(uintptr_t)(rec + 68), 15, 120, 80,
                             3, 1, 1, 0);
                break;
            case 1:
                sub_08007B18((void *)(uintptr_t)(rec + 68), 16, 120, 80,
                             3, 1, 1, 0);
                break;
            }
        }
    }
}
void _0800139F0(void *a, u32 b) __attribute__((alias("MenuFF78_139F0")));
void Sub_0800139F0(void *a, u32 b) __attribute__((alias("MenuFF78_139F0")));
void sub_0800139F0(void *a, u32 b) __attribute__((alias("MenuFF78_139F0")));

// ----------------------------------------------------------------------------
// sub_080014078 — (rec): memcpy(spad, 0x0805F9C0, 32); 4 outer x 6 inner:
//   g = 25CF4(s16[0x03002772], outer, inner*2);
//   h = 25CF4(s16[0x03002772], outer, r8) with r8 = 1,3,5,7,9,11;
//   s16v = spad[(h*2 + g*8)];
//   u32[rec+252+outer*72+k*12] = (u32)s16v;
//   7ABC(u32[rec+28], (u32)s16v, u32[rec+248+outer*72+k*12]).
void MenuFF78_14078(void *rec_) {
    register volatile u8 *rec asm("r9") = (volatile u8 *)rec_;
    u8 spad[32];
    volatile u32 * volatile src_base;
    volatile u32 * volatile dst_base;
    int outer;
    _0802E0A4(spad, (const void *)(uintptr_t)0x0805F9C0u, 32);
    outer = 0;
    dst_base = (volatile u32 *)(uintptr_t)(rec + 252);
    {
        register volatile u8 *src_addr asm("r1") = (volatile u8 *)rec;
        src_addr += 248;
        src_base = (volatile u32 *)src_addr;
    }
    while (outer <= 3) {
        int inner = 0;
        register int base asm("r0") = outer << 3;
        __asm__ volatile ("" : "+r" (base));
        register int next_low asm("r2") = outer + 1;
        __asm__ volatile ("" : "+r" (next_low));
        register int next_outer asm("sl") = next_low;
        __asm__ volatile ("" : "+r" (next_outer));
        register int one asm("r1") = 1;
        __asm__ volatile ("" : "+r" (one));
        register int r8 asm("r8") = one;
        __asm__ volatile ("" : "+r" (r8));
        base += outer;
        base <<= 3;
        register u32 src_origin asm("r2") = (u32)src_base;
        __asm__ volatile ("" : "+r" (src_origin));
        register volatile u32 *src asm("r6") =
            (volatile u32 *)(uintptr_t)(base + src_origin);
        __asm__ volatile ("" : "+r" (src));
        register u32 dst_origin asm("r1") = (u32)dst_base;
        __asm__ volatile ("" : "+r" (dst_origin));
        register volatile u32 *dst asm("r5") =
            (volatile u32 *)(uintptr_t)(base + dst_origin);
        __asm__ volatile ("" : "+r" (dst));
        while (inner <= 5) {
            register u32 w0 asm("r0");
            register const s16 *table0 asm("r2") =
                (const s16 *)(uintptr_t)0x03002772u;
            register int gval asm("r0");
            __asm__ volatile ("movs r1, #0\n\tldrsh r0, [r2, r1]"
                              : "=r" (gval) : "r" (table0) : "r1", "memory");
            int g = _08025CF4(gval, outer, inner * 2);
            register const s16 *table1 asm("r2") =
                (const s16 *)(uintptr_t)0x03002772u;
            register int hval asm("r0");
            __asm__ volatile ("movs r1, #0\n\tldrsh r0, [r2, r1]"
                              : "=r" (hval) : "r" (table1) : "r1", "memory");
            int h = _08025CF4(hval, outer, r8);
            u32 idx = (((u32)h) << 1) + (((u32)g) << 3);
            s16 v = *(s16 *)(uintptr_t)(spad + idx);
            *dst = (u32)(s32)v;
            __asm__ volatile ("mov r2, %1\n\tldr %0, [r2, #28]"
                              : "=r" (w0) : "r" (rec) : "r2", "memory");
            u32 w2 = *src;
            _08007ABC(w0, (u32)(s32)v, w2);
            src += 3;
            dst += 3;
            r8 += 2;
            inner++;
        }
        outer = next_outer;
    }
}
void _080014078(void *a) __attribute__((alias("MenuFF78_14078")));
void Sub_080014078(void *a) __attribute__((alias("MenuFF78_14078")));
void sub_080014078(void *a) __attribute__((alias("MenuFF78_14078")));
