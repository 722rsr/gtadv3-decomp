// ============================================================================
// menu_ff78_g.c — reconstructed C for asm/menu_ff78.s (10 functions).
//
// All bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080011E58 / 11EE0 / 11F68 — (rec, u16) 7B18 emit twins.
//   sub_08001226C — builder sequence + text + D97C/DBE8/12574 tail.
//   sub_0800124EC — builder sequence + 2581C/F778 + tail.
//   sub_080012A44 — 30-iteration 7B18 loop.
//   sub_080012990 — (rec, dead, ev) event handler, cell rec+128.
//   sub_080012C90 — 10-iteration 7B18 scan + 12A44/12AC0/12B68/12B34 tail.
//   sub_080012B68 — (rec, ignored, v) conditional 7B18 pair.
//   sub_080013024 — (rec) screen init via 12F18/24D0C/22E4.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_080011E10(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011DC4(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011CFC(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011958(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011B48(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011EE0(void *a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_080011F68(void *a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_080011CBC(void *a) { (void)a; }
__attribute__((weak)) void Sub_080011FF0(void *a) { (void)a; }
__attribute__((weak)) void Sub_080012574(volatile void *a) { (void)a; }
__attribute__((weak)) void Sub_080012A44(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080012AC0(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080012B68(void *a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_080012B34(void *a) { (void)a; }
__attribute__((weak)) void Sub_080012F18(void *a) { (void)a; }
__attribute__((weak)) void Sub_08003954(int a, int b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800F778(u32 a, u32 b, u32 c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_0800DBE8(void *a) { (void)a; }
__attribute__((weak)) void Sub_08002B368(u32 v) { (void)v; }
__attribute__((weak)) int Sub_08024DA0(int v) { return v; }
__attribute__((weak)) void Sub_08004BFC(int v) { (void)v; }
__attribute__((weak)) int Sub_0802581C(int v) { return v; }
__attribute__((weak)) int Sub_08024D0C(int a, int b) { (void)b; return a; }
__attribute__((weak)) int Sub_080022E4(int v) { return v; }
__attribute__((weak)) void Sub_0800D77C(void *a, int b, int c) { (void)a; (void)b; (void)c; }
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_080011E10(void *a);
extern void Sub_080011784(void *a);
extern void Sub_080011DC4(void *a);
extern void Sub_080011CFC(void *a);
extern void Sub_080011958(void *a);
extern void Sub_080011B48(void *a);
extern void Sub_080011EE0(void *a, u32 b);
extern void Sub_080011F68(void *a, u32 b);
extern void Sub_080011CBC(void *a);
extern void Sub_080011FF0(void *a);
extern void Sub_080012574(volatile void *a);
extern void Sub_080012A44(void *a, u32 b, u32 c);
extern void Sub_080012AC0(void *a, u32 b, u32 c);
extern void Sub_080012B68(void *a, u32 b, u32 c);
extern void Sub_080012B34(void *a);
extern void Sub_080012F18(void *a);
extern void Sub_08003954(int a, int b, u32 c);
extern void Sub_0800F778(u32 a, u32 b, u32 c);
extern void Sub_0800D97C(void *a, int b);
extern void Sub_0800DBE8(void *a);
extern void Sub_08002B368(u32 v);
extern int Sub_08024DA0(int v);
extern void Sub_08004BFC(int v);
extern int Sub_0802581C(int v);
extern int Sub_08024D0C(int a, int b);
extern int Sub_080022E4(int v);
extern void Sub_0800D77C(void *a, int b, int c);
#endif

// ----------------------------------------------------------------------------
// CALL-SITE SPELLING SPLIT for the two builder bodies below (sub_08001226C /
// sub_0800124EC). Both are byte-exact but the promotion screen refuses to
// stage them: it derives a candidate's `export` set from the real `bl` branch
// targets in the ASSEMBLY closure, and the closure branches to the
// VMA-shaped `sub_0800…` name (asm/menu_ff78.s:4160 `bl sub_080011784`,
// :3033-3035 where `sub_…`/`_…` are the two labels on that span, and the same
// pattern for every helper below). A call to the friendly `Sub_…` spelling
// leaves no `bl` at the ROM address, so the address is never exported and the
// retained assembly has nothing to bind to.
//
// So the ARM build calls the spelling the closure defines. Where the helper
// is already C-owned, `sub_…` is a plain `__attribute__((alias(...)))` of the
// real body (zero-byte `.thumb_set`, so not one emitted byte changes) — see
// MenuFF78_11EE0 / MenuFF78_11F68 below, and the same idiom in
// menu_ff78_d.c, menu_ff78_l.c, menu_ff78_c.c, menus.c and ai_line_more.c.
// Where it is not yet C-owned, `sub_…` IS the closure's own label.
//
// The split has to happen here, at the CALL SITE, not only at the
// declaration: clang rejects `alias` attributes outright on darwin, so the
// `sub_…` name does not exist in the host build, and an unguarded call to it
// is a C89 implicit declaration that only tools/apple_decls.py can catch. The
// `#define` below rewrites the VMA spelling to the friendly weak stub for
// __APPLE__ only; it is an object-like macro rather than a call-site macro so
// the call sites below read as the real spelling.
#ifdef __APPLE__
#define _08007B18  Sub_08007B18
#define sub_080011784 Sub_080011784
#define sub_080011958 Sub_080011958
#define sub_080011B48 Sub_080011B48
#define sub_080011CBC Sub_080011CBC
#define sub_080011CFC Sub_080011CFC
#define sub_080011DC4 Sub_080011DC4
#define sub_080011E10 Sub_080011E10
#define sub_080011EE0 Sub_080011EE0
#define sub_080011F68 Sub_080011F68
#define sub_080011FF0 Sub_080011FF0
#define sub_080012574 Sub_080012574
#define sub_08003954  Sub_08003954
#define sub_0800D97C  Sub_0800D97C
#define sub_0800DBE8  Sub_0800DBE8
#define sub_0800F778  Sub_0800F778
#define sub_0802581C  Sub_0802581C
#else
// Closure spelling of 0x08007B18 (asm defines `_08007B18`/`sub_08007B18` on
// that span): MenuFF78_12A44 below must call it so the spliced link resolves.
// The host macro above rewrites it to the friendly weak stub.
extern void _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void sub_080011784(void *a);
extern void sub_080011958(void *a);
extern void sub_080011B48(void *a);
extern void sub_080011CBC(void *a);
extern void sub_080011CFC(void *a);
extern void sub_080011DC4(void *a);
extern void sub_080011E10(void *a);
extern void sub_080011EE0(void *a, u32 b);
extern void sub_080011F68(void *a, u32 b);
extern void sub_080011FF0(void *a);
extern void sub_080012574(volatile void *a);
// Signature matches the repo-wide extern at src/scene_record_dispatch.c:151.
extern void sub_08003954(int a, u32 b, int c);
extern void sub_0800D97C(void *a, int b);
extern void sub_0800DBE8(void *a);
extern void sub_0800F778(u32 a, u32 b, u32 c);
extern int  sub_0802581C(int a);
#endif

// ROM shape, read from instructions only. All three bodies are identical apart
// from their own branch and `bl` displacements:
//   r4 = rec;  r1 = r7 = (u16)a1   (lsls r1,#16 / lsrs r1,#16 / adds r7,r1,#0)
//   p = rec+168;  f5 = s16[p];  if (f5 != 0) return
//   p = p+16;     r6 = s16[p]
//   if (r1 != 0) { t = 0x080CB5A6; i = s16[rec+226]; v = s16[t + i*2];
//                  7B18(rec+100, v, 72,  72, 3,1,1, f5); }
//   if (r7 != r6){ t = 0x080CB5AA; i = s16[rec+226]; v = s16[t + i*2];
//                  7B18(rec+100, v, 144, 72, 3,1,1, f5); }
// `p` is advanced in place (`adds r0,#168` then `adds r0,#16` on the same
// register), so it is a pointer variable, not two independent `rec +` sums.
//
// LOAD FORM IS LOAD-BEARING: every s16 read is NON-volatile. Thumb-1 has no
// immediate-offset LDRSH, so the ROM's `movs r2,#0 / ldrsh rX,[rY,r2]` is what
// agbcc emits for a plain `*(s16 *)`; a volatile lvalue gives
// `ldrh + lsls #16 + asrs #16` instead — the A/B is recorded under "The sub-word
// `volatile` rule, narrowed" in docs/matching_workflow.md and was re-measured on
// this body.
//
// The two 7B18 calls use `_08007B18`, the spelling the closure binds:
// `Screen.resolve("Sub_08007B18")` is False — "closure defines
// _08007B18/sub_08007B18 at 0x08007b18 (rename)" — and the `#define` at the top
// of this file rewrites it to the friendly weak stub for __APPLE__ only.
//
// LEVER 2 — the two table bases must be assembler symbols, not integer
//   literals. agbcc rematerialises a literal at its point of use, so with
//   `u32 t = 0x080CB5A6u` the `ldr r0,[pc,#..]` was scheduled AFTER the index
//   read, which kept the record pointer live in r0 and let the address fold to
//   a single `adds r0,#42` (one instruction) where the ROM has
//   `adds r1,r4,#0 / adds r1,#226` (two). `extern u8 T; __asm__("T =...")`
//   makes the base a symbol the assembler resolves, exactly as 1226C/124EC
//   do it, so the pool load lands first and the address must be rebuilt from
//   r4. Six other spellings of the constant (declaration order before/after
//   the index read, `t = t + 0`, separate declarations plus assignments,
//   pointer-typed `t`, a named `base` copy, a named `a2` sum) all scored an
//   identical 45/136 — the symbol is the only thing that moves it.
//   REMOVAL CONTROL: same source with the two `__asm__` symbol lines deleted
//   -> 45/136, first difference back at +0x14.
void MenuFF78_11E58(void *rec_, u32 a1) {
    u16 a = (u16)a1;
    s16 *p = (s16 *)(uintptr_t)((u8 *)rec_ + 168);
    s16 f5 = p[0];
    s16 r6;
    if (f5 != 0)
        return;
    p = (s16 *)(uintptr_t)((u8 *)p + 16);
    r6 = p[0];
    s32 r6s = (s32)r6;
    extern u8 T1_11E58[];
    extern u8 T2_11E58[];
    __asm__("T1_11E58 = 0x080CB5A6");
    __asm__("T2_11E58 = 0x080CB5AA");
    if (a != 0) {
        u32 t = (u32)(uintptr_t)T1_11E58;
        s16 i = *(s16 *)(uintptr_t)((u8 *)rec_ + 226);
        s16 v = *(s16 *)(uintptr_t)(t + (u32)(i * 2));
        _08007B18((void *)(uintptr_t)((u8 *)rec_ + 100), (int)v, 72, 72,
                  3, 1, 1, (u32)f5);
    }
    if ((u32)a != (u32)r6s) {
        u32 t = (u32)(uintptr_t)T2_11E58;
        s16 i = *(s16 *)(uintptr_t)((u8 *)rec_ + 226);
        s16 v = *(s16 *)(uintptr_t)(t + (u32)(i * 2));
        _08007B18((void *)(uintptr_t)((u8 *)rec_ + 100), (int)v, 144, 72,
                  3, 1, 1, (u32)f5);
    }
}
#ifndef __APPLE__
void _080011E58(void *a, u32 b) __attribute__((alias("MenuFF78_11E58")));
void Sub_080011E58(void *a, u32 b) __attribute__((alias("MenuFF78_11E58")));
void sub_080011E58(void *a, u32 b) __attribute__((alias("MenuFF78_11E58")));
#endif

void MenuFF78_11EE0(void *rec_, u32 a1) {
    u16 a = (u16)a1;
    s16 *p = (s16 *)(uintptr_t)((u8 *)rec_ + 168);
    s16 f5 = p[0];
    s16 r6;
    if (f5 != 0)
        return;
    p = (s16 *)(uintptr_t)((u8 *)p + 16);
    r6 = p[0];
    s32 r6s = (s32)r6;
    extern u8 T1_11EE0[];
    extern u8 T2_11EE0[];
    __asm__("T1_11EE0 = 0x080CB5A6");
    __asm__("T2_11EE0 = 0x080CB5AA");
    if (a != 0) {
        u32 t = (u32)(uintptr_t)T1_11EE0;
        s16 i = *(s16 *)(uintptr_t)((u8 *)rec_ + 226);
        s16 v = *(s16 *)(uintptr_t)(t + (u32)(i * 2));
        _08007B18((void *)(uintptr_t)((u8 *)rec_ + 100), (int)v, 72, 72,
                  3, 1, 1, (u32)f5);
    }
    if ((u32)a != (u32)r6s) {
        u32 t = (u32)(uintptr_t)T2_11EE0;
        s16 i = *(s16 *)(uintptr_t)((u8 *)rec_ + 226);
        s16 v = *(s16 *)(uintptr_t)(t + (u32)(i * 2));
        _08007B18((void *)(uintptr_t)((u8 *)rec_ + 100), (int)v, 144, 72,
                  3, 1, 1, (u32)f5);
    }
}
#ifndef __APPLE__
void _080011EE0(void *a, u32 b) __attribute__((alias("MenuFF78_11EE0")));
void Sub_080011EE0(void *a, u32 b) __attribute__((alias("MenuFF78_11EE0")));
void sub_080011EE0(void *a, u32 b) __attribute__((alias("MenuFF78_11EE0")));
#endif

void MenuFF78_11F68(void *rec_, u32 a1) {
    u16 a = (u16)a1;
    s16 *p = (s16 *)(uintptr_t)((u8 *)rec_ + 168);
    s16 f5 = p[0];
    s16 r6;
    if (f5 != 0)
        return;
    p = (s16 *)(uintptr_t)((u8 *)p + 16);
    r6 = p[0];
    s32 r6s = (s32)r6;
    extern u8 T1_11F68[];
    extern u8 T2_11F68[];
    __asm__("T1_11F68 = 0x080CB5A6");
    __asm__("T2_11F68 = 0x080CB5AA");
    if (a != 0) {
        u32 t = (u32)(uintptr_t)T1_11F68;
        s16 i = *(s16 *)(uintptr_t)((u8 *)rec_ + 226);
        s16 v = *(s16 *)(uintptr_t)(t + (u32)(i * 2));
        _08007B18((void *)(uintptr_t)((u8 *)rec_ + 100), (int)v, 72, 72,
                  3, 1, 1, (u32)f5);
    }
    if ((u32)a != (u32)r6s) {
        u32 t = (u32)(uintptr_t)T2_11F68;
        s16 i = *(s16 *)(uintptr_t)((u8 *)rec_ + 226);
        s16 v = *(s16 *)(uintptr_t)(t + (u32)(i * 2));
        _08007B18((void *)(uintptr_t)((u8 *)rec_ + 100), (int)v, 144, 72,
                  3, 1, 1, (u32)f5);
    }
}
#ifndef __APPLE__
void _080011F68(void *a, u32 b) __attribute__((alias("MenuFF78_11F68")));
void Sub_080011F68(void *a, u32 b) __attribute__((alias("MenuFF78_11F68")));
void sub_080011F68(void *a, u32 b) __attribute__((alias("MenuFF78_11F68")));
#endif

// ----------------------------------------------------------------------------
// sub_08001226C — builder: 11784 + 11E10 + 11DC4 + 11B48, then
//   11EE0(rec, u16[rec+180]), 11CBC, text draw 03954(232,76,
//   u32[WA + 0x5E4 + (s16[rec+172]+31)*72]), D97C(rec+224,15),
//   DBE8(rec+64), 12574(rec).
//
// THE TWO-WORD POOL IS REACHABLE — give the base an assembler symbol.
//   The ROM emits two pool words and two adds:
//     0x8012298 ldr r2,[pc,#60] -> 0x03001780
//     0x80122aa ldr r1,[pc,#48] -> 0x000005E4
//     0x80122ac adds r2,r2,r1; base + 0x5E4
//     0x80122ae adds r1,r0,r2; + index
//   A literal `0x03001780u + 0x5E4u` is folded in the front end to the
//   single word 0x03001D64, and integer constant folding is mandatory — so
//   no arithmetic spelling recovers it. What recovers it is making the base
//   a SYMBOL the assembler resolves, so the two constants never meet in the
//   front end at all:
//     extern u8 MenuFF78_WA_1226C[];
//     __asm__("MenuFF78_WA_1226C = 0x03001780");
//   `Sym = <addr>` makes it an absolute symbol, so `ldr r2,=Sym` emits the
//   pool word 0x03001780 and the +0x5E4 stays a separate literal. The repo
//   already uses this idiom at src/race_scene_d1.c:316-319. measured
//   18 arithmetic spellings and 2 register pins, all of which leave one
//   literal; it never tried giving the base a name the assembler knows.
//
// Three placement facts, all measured (each one is worth several bytes):
//   1. `u32 wa` must be declared BEFORE `s16 i`, not after. agbcc materialises
//      the load at the declaration; with `wa` after `i` it lands at +0x30
//      instead of +0x2c, i.e. after `adds r0,#172`, and everything after it
//      shifts.
//   2. `u32 base = wa + 0x5E4u;` must be a NAMED local. Written inline as
//      `(wa + 0x5E4u) + off` agbcc reassociates the commutative plus and
//      emits `off + wa` first, then `+ 0x5E4` — the wrong grouping, same
//      instruction count. Naming the intermediate pins the grouping.
//   3. `(i+31)*72` must be spelled `((row<<3) + row) << 3`, not `* 72`: both
//      match here, but the shift form mirrors the ROM and does not depend on
//      agbcc's strength reduction.
void MenuFF78_1226C(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    extern u8 MenuFF78_WA_1226C[];
    __asm__("MenuFF78_WA_1226C = 0x03001780");
    sub_080011784(rec_);
    sub_080011E10(rec_);
    sub_080011DC4(rec_);
    sub_080011B48(rec_);
    sub_080011EE0(rec_, *(volatile u16 *)(uintptr_t)(rec + 180));
    sub_080011CBC(rec_);
    {
        u32 wa = (u32)(uintptr_t)MenuFF78_WA_1226C;
        s16 i = *(s16 *)(uintptr_t)(rec + 172);
        u32 row = (u32)(s32)(i + 31);
        u32 off = ((row << 3) + row) << 3; // *72
        u32 base = wa + 0x5E4u;
        u32 v = *(volatile u32 *)(uintptr_t)(base + off);
        sub_08003954(232, 76, v);
    }
    sub_0800D97C((void *)(uintptr_t)(rec + 224), 15);
    sub_0800DBE8((void *)(uintptr_t)(rec + 64));
    sub_080012574(rec);
}
#ifndef __APPLE__
void _08001226C(void *a) __attribute__((alias("MenuFF78_1226C")));
void sub_08001226C(void *a) __attribute__((alias("MenuFF78_1226C")));
#endif

// ----------------------------------------------------------------------------
// sub_0800124EC — builder: 11784 + 11E10 + 11CFC + 11958 + 11B48, then
//   11F68(rec, u16[rec+180]), 11FF0, 11CBC,
//   F778(168,76, u32[WA+0x5E4 + (s16)2581C(s16[rec+172])*72]),
//   D97C(rec+224,15), DBE8(rec+64), 12574(rec).
//
// Placement, measured (each costs real bytes if you move it):
//   - `u32 wa` is declared AFTER `u32 k = (u32)(s32)v;`, i.e. after the
//     sign-extend, because the ROM loads the base at 0x8012534, between the
//     `asrs` and the index scaling. One line earlier puts the load above the
//     lsls/asrs (candidate 136 but prefix 68); one line later puts it after
//     the index scaling (candidate 136, wrong order).
//   - `u32 base = wa + 0x5E4u;` must be a named local. Inline
//     `(wa + 0x5E4u) + off` reassociates to `off + wa` then `+ 0x5E4` —
//     the right instruction count and the wrong grouping.
void MenuFF78_124EC(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    extern u8 MenuFF78_WA_124EC[];
    __asm__("MenuFF78_WA_124EC = 0x03001780");
    sub_080011784(rec_);
    sub_080011E10(rec_);
    sub_080011CFC(rec_);
    sub_080011958(rec_);
    sub_080011B48(rec_);
    sub_080011F68(rec_, *(volatile u16 *)(uintptr_t)(rec + 180));
    sub_080011FF0(rec_);
    sub_080011CBC(rec_);
    {
        s16 i = *(s16 *)(uintptr_t)(rec + 172);
        s16 v = (s16)sub_0802581C((int)i);
        u32 k = (u32)(s32)v;
        u32 wa = (u32)(uintptr_t)MenuFF78_WA_124EC;
        u32 off = ((k << 3) + k) << 3; // *72
        u32 base = wa + 0x5E4u;
        u32 t = *(volatile u32 *)(uintptr_t)(base + off);
        sub_0800F778(168, 76, t);
    }
    sub_0800D97C((void *)(uintptr_t)(rec + 224), 15);
    sub_0800DBE8((void *)(uintptr_t)(rec + 64));
    sub_080012574(rec);
}
#ifndef __APPLE__
void _0800124EC(void *a) __attribute__((alias("MenuFF78_124EC")));
void sub_0800124EC(void *a) __attribute__((alias("MenuFF78_124EC")));
#endif

// ----------------------------------------------------------------------------
// sub_080012A44 — (rec, r1, r2): 30-iteration emit loop.
//   r5 = 0; r9 = 29; loop: case 2: 7B18(rec+44,7,r5,r7,3,1,0,0);
//   case 3: 7B18(rec+44,7,r5,r7,4,1,0,0); r5 += 8; r9--;
//   continue while r9 >= 0.
//
// The `one`/`zero` locals must be declared BEFORE `r5`/`r9`: they are the
// operands of the stack-passed 7B18 arguments, and agbcc materialises a local
// at its declaration point. The ROM initialises sl=1 and r4=0 first, then
// r5=0, then r9=29; with them declared later they are materialised after
// r5/r9 and the whole prologue diverges.
void MenuFF78_12A44(void *rec_, u32 r1_, u32 r2_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 r7 = r1_;
    u32 r6 = r2_;
    u32 one = 1;
    u32 zero = 0;
    u32 r5 = 0;
    int r9 = 29;
    for (;;) {
        switch (r6) {
        case 2:
            _08007B18((void *)(uintptr_t)(rec + 44), 7, (int)r5, (int)r7,
                      3, (int)one, (int)zero, (int)zero);
            break;
        case 3:
            _08007B18((void *)(uintptr_t)(rec + 44), 7, (int)r5, (int)r7,
                      4, (int)one, (int)zero, (int)zero);
            break;
        default:
            break;
        }
        r5 += 8;
        r9 -= 1;
        if (r9 < 0) break;
    }
}
#ifndef __APPLE__
void _080012A44(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12A44")));
void Sub_080012A44(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12A44")));
void sub_080012A44(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12A44")));
#endif

// ----------------------------------------------------------------------------
// sub_080012990 — (rec, dead, ev): cell = s16[rec+128].
//   ev==2: 2B368(4) + quad reset (u32[+8]=10, s16[+12]=0, u32[+40]=10,
//     u32[+36]=0).
//   ev==1: v=24DA0(s16[cell]); nonzero: 2B368(1), u32[rec+140]=1,
//     s16[rec+12]=0, s16[rec+116]=1, u16[WA+0xFBE]=u16[cell];
//     zero: 2B368(10).
//   ev==64||ev==32: 2B368(2), cell--. ev==128||ev==16: 2B368(2), cell++.
//   Clamp: <0 -> 9; >9 -> 0. Tail: 04BFC(s16[cell]).
void MenuFF78_12990(void *rec_, int dead, u32 ev_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 ev = (u16)ev_;
    volatile u8 *cell = (volatile u8 *)(uintptr_t)(rec + 128);
    if (ev == 2) {
        Sub_08002B368(4);
        *(volatile u32 *)(uintptr_t)(rec + 8) = 10;
        *(volatile s16 *)(uintptr_t)(rec + 12) = 0;
        *(volatile u32 *)(uintptr_t)(rec + 40) = 10;
        *(volatile u32 *)(uintptr_t)(rec + 36) = 0;
    }
    if (ev == 1) {
        int v = Sub_08024DA0((int)*(volatile s16 *)(uintptr_t)cell);
        if (v != 0) {
            Sub_08002B368(1);
            *(volatile u32 *)(uintptr_t)(rec + 140) = 1;
            *(volatile s16 *)(uintptr_t)(rec + 12) = 0;
            *(volatile s16 *)(uintptr_t)(rec + 116) = 1;
            *(volatile u16 *)(uintptr_t)(0x03001780u + 0xFBEu) =
                *(volatile u16 *)(uintptr_t)cell;
        } else {
            Sub_08002B368(10);
        }
    }
    if (ev == 64 || ev == 32) {
        Sub_08002B368(2);
        s16 c = *(volatile s16 *)(uintptr_t)cell;
        *(volatile s16 *)(uintptr_t)cell = (s16)(c - 1);
    }
    if (ev == 128 || ev == 16) {
        Sub_08002B368(2);
        s16 c = *(volatile s16 *)(uintptr_t)cell;
        *(volatile s16 *)(uintptr_t)cell = (s16)(c + 1);
    }
    if (*(volatile s16 *)(uintptr_t)cell < 0)
        *(volatile s16 *)(uintptr_t)cell = 9;
    if (*(volatile s16 *)(uintptr_t)cell > 9)
        *(volatile s16 *)(uintptr_t)cell = 0;
    Sub_08004BFC((int)*(volatile s16 *)(uintptr_t)cell);
}
#ifndef __APPLE__
void _080012990(void *a, int b, u32 c) __attribute__((alias("MenuFF78_12990")));
void sub_080012990(void *a, int b, u32 c) __attribute__((alias("MenuFF78_12990")));
#endif

// ----------------------------------------------------------------------------
// sub_080012C90 — (rec): 10-iteration gated 7B18 scan, then
//   12A44(rec, r4, u32[rec+140]), 12AC0(rec, r6, r4), 12B68(rec, r6, r4),
//   12B34(rec), D97C(rec+132,15), DBE8(rec+8).
//   Scan (r6 = 0..9): r1 = u16[0x080CB678 + r6*2]; if u16[WA+0xFD0]==1
//   and s16[rec+134]==1: 7B18(rec, r1, entryW0, entryW1, 5,1,1,0) with
//   entry from 0x080CB5D8 + r6*8.
//   Table pick: s = s16[rec+128]; r6 = u32[tbl + s*8], r4 = u32[tbl+4+s*8].
void MenuFF78_12C90(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    for (u32 r6 = 0; r6 <= 9; r6++) {
        u16 r1 = *(volatile u16 *)(uintptr_t)(0x080CB678u + (r6 << 1));
        u16 tick = *(volatile u16 *)(uintptr_t)(wa + 0xFD0u + r6 * 2);
        if (tick == 1 && *(volatile s16 *)(uintptr_t)(rec + 134) == 1) {
            u32 w0 = *(volatile u32 *)(uintptr_t)(0x080CB5D8u + r6 * 8);
            u32 w1 = *(volatile u32 *)(uintptr_t)(0x080CB5D8u + 4 + r6 * 8);
            Sub_08007B18(rec_, (int)r1, (int)w0, (int)w1, 5, 1, 1, 0);
        }
    }
    {
        s16 s = *(volatile s16 *)(uintptr_t)(rec + 128);
        u32 off = ((u32)(s32)s) << 3;
        u32 r6 = *(volatile u32 *)(uintptr_t)(0x080CB5D8u + off);
        u32 r4 = *(volatile u32 *)(uintptr_t)(0x080CB5D8u + 4 + off);
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 140);
        Sub_080012A44(rec_, r4, r2);
        Sub_080012AC0(rec_, r6, r4);
        Sub_080012B68(rec_, r6, r4);
    }
    Sub_080012B34(rec_);
    Sub_0800D97C((void *)(uintptr_t)(rec + 132), 15);
    Sub_0800DBE8((void *)(uintptr_t)(rec + 8));
}
#ifndef __APPLE__
void _080012C90(void *a) __attribute__((alias("MenuFF78_12C90")));
void sub_080012C90(void *a) __attribute__((alias("MenuFF78_12C90")));
#endif

// ----------------------------------------------------------------------------
// sub_080012B68 — (rec, ignored, v): conditional 7B18 pair over rec+44.
//   Gate: s16[rec+134]==0 and u32[rec+140] in {2,3} (else return).
//   s = s16[rec+128]:
//     s==0: 7B18(rec+44, 10, 0, v+9, w, 1, 0, 0).
//     s==9: 7B18(rec+44, 9, 0, v-9, w, 1, 0, 0).
//     else: both calls in that order. (w = u32[rec+140].)
void MenuFF78_12B68(void *rec_, u32 ignored, u32 v) {
    (void)ignored;
    volatile u8 *rec = (volatile u8 *)rec_;
    if (*(volatile s16 *)(uintptr_t)(rec + 134) != 0) return;
    u32 w = *(volatile u32 *)(uintptr_t)(rec + 140);
    if (w != 2 && w != 3) return;
    s16 s = *(volatile s16 *)(uintptr_t)(rec + 128);
    if (s == 0) {
        Sub_08007B18((void *)(uintptr_t)(rec + 44), 10, 0, (int)(v + 9),
                     w, 1, 0, 0);
    } else if (s == 9) {
        Sub_08007B18((void *)(uintptr_t)(rec + 44), 9, 0, (int)(v - 9),
                     w, 1, 0, 0);
    } else {
        Sub_08007B18((void *)(uintptr_t)(rec + 44), 10, 0, (int)(v + 9),
                     w, 1, 0, 0);
        Sub_08007B18((void *)(uintptr_t)(rec + 44), 9, 0, (int)(v - 9),
                     w, 1, 0, 0);
    }
}
#ifndef __APPLE__
void _080012B68(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12B68")));
void Sub_080012B68(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12B68")));
void sub_080012B68(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12B68")));
#endif

// ----------------------------------------------------------------------------
// sub_080013024 — (rec) screen init:
//   s16[rec+176] = u16[WA+0xFBE]; 12F18(rec);
//   s16[rec+174] = u16[WA+0x574]; s16[rec+176] = u16[WA+0xFBE] (reload);
//   s16[rec+136] = (s16)24D0C(s16[rec+174], s16[rec+176]);
//   s16[rec+174] = (u16)u32[rec+188 + s16[rec+136]*4];
//   s16[rec+180] = (s16)22E4(s16[rec+174]);
//   s16[rec+172] = (s8)u8[WA + s16[rec+174]*12 + 49];
//   s16[rec+178] = (s8)u8[WA + s16[rec+174]*12 + 48];
//   u32[rec+144] = 2; s16[rec+186] = 0;
//   u16[WA+0xFE0 + s16[rec+176]*2] = 0; D77C(rec+156, 0, -32).
void MenuFF78_13024(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    *(volatile s16 *)(uintptr_t)(rec + 176) =
        *(volatile u16 *)(uintptr_t)(wa + 0xFBEu);
    Sub_080012F18(rec_);
    *(volatile s16 *)(uintptr_t)(rec + 174) =
        *(volatile u16 *)(uintptr_t)(wa + 0x574u);
    *(volatile s16 *)(uintptr_t)(rec + 176) =
        *(volatile u16 *)(uintptr_t)(wa + 0xFBEu);
    {
        s16 a = *(volatile s16 *)(uintptr_t)(rec + 174);
        s16 b = *(volatile s16 *)(uintptr_t)(rec + 176);
        *(volatile s16 *)(uintptr_t)(rec + 136) =
            (s16)Sub_08024D0C((int)a, (int)b);
    }
    {
        s16 i = *(volatile s16 *)(uintptr_t)(rec + 136);
        u32 v = *(volatile u32 *)(uintptr_t)(rec + 188 + ((u32)(s32)i << 2));
        *(volatile s16 *)(uintptr_t)(rec + 174) = (s16)(u16)v;
    }
    *(volatile s16 *)(uintptr_t)(rec + 180) =
        (s16)Sub_080022E4((int)*(volatile s16 *)(uintptr_t)(rec + 174));
    {
        s16 i = *(volatile s16 *)(uintptr_t)(rec + 174);
        u32 off = ((u32)(s32)i << 1) + (u32)(s32)i; // *3
        off <<= 2; // *12
        s8 sb = (s8)*(volatile u8 *)(uintptr_t)(wa + off + 49);
        *(volatile s16 *)(uintptr_t)(rec + 172) = (s16)sb;
    }
    {
        s16 i = *(volatile s16 *)(uintptr_t)(rec + 174);
        u32 off = (((u32)(s32)i << 1) + (u32)(s32)i) << 2; // *12
        s8 sb = (s8)*(volatile u8 *)(uintptr_t)(wa + off + 48);
        *(volatile s16 *)(uintptr_t)(rec + 178) = (s16)sb;
    }
    *(volatile u32 *)(uintptr_t)(rec + 144) = 2;
    *(volatile s16 *)(uintptr_t)(rec + 186) = 0;
    {
        s16 i = *(volatile s16 *)(uintptr_t)(rec + 176);
        *(volatile u16 *)(uintptr_t)(wa + 0xFE0u + (((u32)(s32)i) << 1)) = 0;
    }
    Sub_0800D77C((void *)(uintptr_t)(rec + 156), 0, -32);
}
#ifndef __APPLE__
void _080013024(void *a) __attribute__((alias("MenuFF78_13024")));
void Sub_080013024(void *a) __attribute__((alias("MenuFF78_13024")));
void sub_080013024(void *a) __attribute__((alias("MenuFF78_13024")));
#endif
