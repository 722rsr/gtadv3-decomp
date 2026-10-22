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
#include "gtadv/ram_blocks.h"

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
extern void _080012F18(void *a);
extern void Sub_08003954(int a, int b, u32 c);
extern void Sub_0800F778(u32 a, u32 b, u32 c);
extern void Sub_0800D97C(void *a, int b);
extern void Sub_0800DBE8(void *a);
extern void sub_0802B368(u32 v);
extern int _08024DA0(int v);
extern void sub_08004BFC(int v);
extern int Sub_0802581C(int v);
extern int _08024D0C(int a, int b);
extern int _080022E4(int v);
extern void _0800D77C(void *a, int b, int c);

#define MFFG_12990_B368 sub_0802B368
#define MFFG_12990_24DA0 _08024DA0

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
// Closure spelling of 0x08007B18 (asm defines `_08007B18`/`sub_08007B18` on
// that span): MenuFF78_12A44 below must call it so the spliced link resolves.
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
// _08007B18/sub_08007B18 at 0x08007b18 (rename)".
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
void _080011E58(void *a, u32 b) __attribute__((alias("MenuFF78_11E58")));
void Sub_080011E58(void *a, u32 b) __attribute__((alias("MenuFF78_11E58")));
void sub_080011E58(void *a, u32 b) __attribute__((alias("MenuFF78_11E58")));

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
void _080011EE0(void *a, u32 b) __attribute__((alias("MenuFF78_11EE0")));
void Sub_080011EE0(void *a, u32 b) __attribute__((alias("MenuFF78_11EE0")));
void sub_080011EE0(void *a, u32 b) __attribute__((alias("MenuFF78_11EE0")));

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
void _080011F68(void *a, u32 b) __attribute__((alias("MenuFF78_11F68")));
void Sub_080011F68(void *a, u32 b) __attribute__((alias("MenuFF78_11F68")));
void sub_080011F68(void *a, u32 b) __attribute__((alias("MenuFF78_11F68")));

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
void _08001226C(void *a) __attribute__((alias("MenuFF78_1226C")));
void sub_08001226C(void *a) __attribute__((alias("MenuFF78_1226C")));

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
void _0800124EC(void *a) __attribute__((alias("MenuFF78_124EC")));
void sub_0800124EC(void *a) __attribute__((alias("MenuFF78_124EC")));

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
void _080012A44(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12A44")));
void Sub_080012A44(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12A44")));
void sub_080012A44(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12A44")));

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
    if (ev == 2) {
        MFFG_12990_B368(4);
        register u32 ten __asm__("r1") = 10;
        __asm__ volatile("" : "+r"(ten));
        *(volatile u32 *)(uintptr_t)(rec + 8) = ten;
        {
            register u32 zero __asm__("r0") = 0;
            __asm__ volatile("" : "+r"(zero));
            *(volatile s16 *)(uintptr_t)(rec + 12) = (s16)zero;
            *(volatile u32 *)(uintptr_t)(rec + 40) = ten;
            *(volatile u32 *)(uintptr_t)(rec + 36) = zero;
        }
    }
    register volatile u8 *cell __asm__("r6") =
        (volatile u8 *)(uintptr_t)(rec + 128);
    if (ev == 1) {
        register int cell_s16 __asm__("r0");
        cell_s16 = *(const s16 *)(uintptr_t)cell;
        int v = MFFG_12990_24DA0(cell_s16);
        if (v != 0) {
            MFFG_12990_B368(1);
            *(volatile u32 *)(uintptr_t)(rec + 140) = 1;
            *(volatile s16 *)(uintptr_t)(rec + 12) = 0;
            *(volatile s16 *)(uintptr_t)(rec + 116) = 1;
            __asm__(".globl MenuFF78Wa12990\nMenuFF78Wa12990 = 0x03001780");
            extern u8 MenuFF78Wa12990[];
            register u32 wa_base __asm__("r0") =
                (u32)(uintptr_t)MenuFF78Wa12990;
            register u16 cell_value __asm__("r1") =
                *(volatile u16 *)(uintptr_t)cell;
            register u32 wa_offset __asm__("r2") = 0xFBEu;
            __asm__ volatile("" : "+r"(wa_base));
            __asm__ volatile("" : "+r"(cell_value));
            __asm__ volatile("" : "+r"(wa_offset));
            *(volatile u16 *)(uintptr_t)(wa_base + wa_offset) = cell_value;
        } else {
            MFFG_12990_B368(10);
        }
    }
    if (ev == 64 || ev == 32) {
        MFFG_12990_B368(2);
        u16 c = *(volatile u16 *)(uintptr_t)cell;
        *(volatile u16 *)(uintptr_t)cell = (u16)(c - 1);
    }
    if (ev == 128 || ev == 16) {
        MFFG_12990_B368(2);
        u16 c = *(volatile u16 *)(uintptr_t)cell;
        *(volatile u16 *)(uintptr_t)cell = (u16)(c + 1);
    }
    /* Signed non-volatile reads select ldrsh; keep the clamped stores volatile. */
    register volatile s16 *clamp_cell __asm__("r1") = (volatile s16 *)cell;
    register int clamp_value __asm__("r0");
    clamp_value = *(const s16 *)(uintptr_t)clamp_cell;
    if (clamp_value < 0) {
        clamp_value = 9;
        *clamp_cell = (s16)clamp_value;
    }
    clamp_value = *(const s16 *)(uintptr_t)clamp_cell;
    if (clamp_value > 9) {
        clamp_value = 0;
        *clamp_cell = (s16)clamp_value;
    }
    clamp_value = *(const s16 *)(uintptr_t)cell;
    sub_08004BFC(clamp_value);
}
// The ROM has a zero halfword after bx r0; use data-fill alignment instead
// of the Thumb section's 0x46C0 nop filler.
__asm__(".align 2, 0");
void _080012990(void *a, int b, u32 c) __attribute__((alias("MenuFF78_12990")));
void sub_080012990(void *a, int b, u32 c) __attribute__((alias("MenuFF78_12990")));

// ----------------------------------------------------------------------------
// sub_080012C90 — (rec): 10-iteration gated 7B18 scan, then
//   12A44(rec, r4, u32[rec+140]), 12AC0(rec, r6, r4), 12B68(rec, r6, r4),
//   12B34(rec), D97C(rec+132,15), DBE8(rec+8).
//   Scan (r6 = 0..9): r1 = u16[0x080CB678 + r6*2]; if u16[WA+0xFD0]==1
//   and s16[rec+134]==1: 7B18(rec, r1, entryW0, entryW1, 5,1,1,0) with
//   entry from 0x080CB5D8 + r6*8.
//   Table pick: s = s16[rec+128]; r6 = u32[tbl + s*8], r4 = u32[tbl+4+s*8].
void MenuFF78_12C90(void *rec_)
{
    extern const u8 MenuFF78Records12C90[];
    extern const u16 MenuFF78Index12C90[];
    extern void MenuFF78_12AC0_4(void *a, u32 b, u32 c, u32 d)
        __asm__("_080012AC0");
    extern void MenuFF78_12B68_4(void *a, u32 b, u32 c)
        __asm__("_080012B68");
    extern void MenuFF78_12B34_4(void *a) __asm__("_080012B34");
    extern void MenuFF78_0D97C_4(void *a, int b) __asm__("_0800D97C");
    extern void MenuFF78_0DBE8_4(void *a) __asm__("_0800DBE8");
    volatile u8 *rec = (volatile u8 *)rec_;
    register volatile u8 *gate __asm__("r8");
    register volatile u8 *work __asm__("r0");
    register volatile u8 *entries __asm__("r5");
    register s32 i __asm__("r6");
    register u32 gateOffset __asm__("r1");

    __asm__(".globl MenuFF78Records12C90\nMenuFF78Records12C90 = 0x080CB5D8\n"
            ".globl MenuFF78Index12C90\nMenuFF78Index12C90 = 0x080CB678");
    RAM_DEF_03001780;
    i = 0;
    work = RAM_SYM_03001780;
    gateOffset = 131;
    __asm__ volatile ("" : "+r" (gateOffset));
    gateOffset <<= 5;
    gate = (volatile u8 *)(uintptr_t)(gateOffset + (u32)(uintptr_t)work);
    entries = (volatile u8 *)MenuFF78Records12C90;
    do {
        register u32 first __asm__("r2");
        register u32 second __asm__("r3");
        register u32 savedSecond __asm__("ip");
        const u16 *indexTable;
        register u32 kind __asm__("r1");
        register s32 gateValue __asm__("r4");
        first = *(volatile u32 *)(entries + 0);
        second = *(volatile u32 *)(entries + 4);
        __asm__ volatile ("" : "+r" (second));
        savedSecond = second;
        indexTable = MenuFF78Index12C90;
        __asm__ volatile ("" : "+r" (indexTable));
        kind = indexTable[i];
        if (*(volatile u16 *)gate == 1) {
            gateValue = *(const s16 *)(uintptr_t)(rec + 134);
            __asm__ volatile ("" : "+r" (gateValue));
            if (gateValue != 1) goto next_12c90;
            _08007B18((void *)rec, (int)kind, (int)first,
                      (int)savedSecond, 5, (u32)gateValue,
                      (u32)gateValue, 0);
        }
next_12c90:
        {
            register u32 step __asm__("r0") = 2;
            __asm__ volatile ("" : "+r" (step));
            gate += step;
        }
        entries += 8;
        i += 1;
    } while (i <= 9);

    {
        const u8 *records = MenuFF78Records12C90;
        s16 index = *(const s16 *)(uintptr_t)(rec + 128);
        u32 offset = (u32)(s32)index << 3;
        u32 r6 = *(const u32 *)(records + offset);
        records += 4;
        u32 r4 = *(const u32 *)(records + offset);
        {
            register u32 progress __asm__("r2");
            progress = *(const u32 *)(uintptr_t)(rec + 140);
            MenuFF78_12A44((void *)rec, r4, progress);
        }
        {
            register u32 mode __asm__("r3");
            mode = *(const u32 *)(uintptr_t)(rec + 140);
            MenuFF78_12AC0_4((void *)rec, r6, r4, mode);
        }
        MenuFF78_12B68_4((void *)rec, r6, r4);
    }
    MenuFF78_12B34_4((void *)rec);
    MenuFF78_0D97C_4((void *)(uintptr_t)(rec + 132), 15);
    MenuFF78_0DBE8_4((void *)(uintptr_t)(rec + 8));
}
void _080012C90(void *a) __attribute__((alias("MenuFF78_12C90")));
void sub_080012C90(void *a) __attribute__((alias("MenuFF78_12C90")));

// ----------------------------------------------------------------------------
// sub_080012B68 — (rec, ignored, v): conditional 7B18 pair over rec+44.
//   Gate: s16[rec+134]==0 and u32[rec+140] in {2,3} (else return).
//   Each mode has its own s16[rec+128] dispatch. Both pass 2 as the fifth
//   helper argument; mode 2 forwards the stored mode, while mode 3 uses 2.
//   s==0: one call at v+9; s==9: one call at v-9; otherwise both in order.
void MenuFF78_12B68(void *rec_, u32 ignored, u32 v) {
    (void)ignored;
    u8 *rec = (u8 *)rec_;
    s16 *mode_field;
    void *helper_rec;
    int y_plus;
    s16 s;
    s16 gate;
    register u32 mode asm("r6");
    register u32 v_reg asm("r5") = v;
    register u32 one asm("r8");
    register u32 two asm("r9");

    mode_field = (s16 *)(uintptr_t)(rec + 134);
    gate = mode_field[0];
    if (gate != 0) return;
    mode_field = (s16 *)(uintptr_t)((u8 *)mode_field + 6);
    mode = *(u32 *)mode_field;
    if (mode == 2) goto mode_two;
    if (mode != 3) return;

    s = *((s16 *)(uintptr_t)((u8 *)mode_field - 12));
    if (s == 0) {
        _08007B18((void *)(uintptr_t)(rec + 44), 10, 0,
                     (int)(v_reg + 9), 2, 1, 0, 0);
    } else if (s == 9) {
        _08007B18((void *)(uintptr_t)(rec + 44), 9, 0,
                     (int)(v_reg - 9), 2, 1, 0, 0);
    } else {
        mode = (u32)(uintptr_t)(rec + 44);
        y_plus = (int)(v_reg + 9);
        two = 2;
        _08007B18((void *)(uintptr_t)mode, 10, 0, y_plus, 2,
                     (one = 1), 0, 0);
        _08007B18((void *)(uintptr_t)mode, 9, 0,
                     (int)(v_reg - 9), two, one, 0, 0);
    }
    goto done;

mode_two:
    s = *((s16 *)(uintptr_t)(rec + 128));
    if (s == 0) {
        _08007B18((void *)(uintptr_t)(rec + 44), 10, 0,
                     (int)(v_reg + 9), mode, 1, 0, 0);
    } else if (s == 9) {
        _08007B18((void *)(uintptr_t)(rec + 44), 9, 0,
                     (int)(v_reg - 9), mode, 1, 0, 0);
    } else {
        helper_rec = (void *)(uintptr_t)(rec + 44);
        y_plus = (int)(v_reg + 9);
        _08007B18(helper_rec, 10, 0, y_plus, mode, (one = 1), 0, 0);
        _08007B18(helper_rec, 9, 0,
                     (int)(v_reg - 9), mode, one, 0, 0);
    }

done:
    return;
}
void _080012B68(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12B68")));
void Sub_080012B68(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12B68")));
void sub_080012B68(void *a, u32 b, u32 c) __attribute__((alias("MenuFF78_12B68")));

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
//   u16[WA+0x1060 + s16[rec+176]*2] = 0; D77C(rec+156, 0, -32).
// The pinned registers and the empty-asm `off` feed exist only to reproduce
// agbcc's own operand choices (174 materialised in r1 before the base add,
// rec+188 formed in r0 before the scaled index is folded in).
void MenuFF78_13024(void *rec_)
{
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u8 *wa;
    register volatile u8 *pbe __asm__("r5");
    register volatile u16 *p176 __asm__("r6");
    register volatile u16 *p174 __asm__("r9");
    register u32 zero __asm__("r8");
    volatile u16 *p;
    u16 h;
    u16 h2;
    s16 ti;
    RAM_DEF_03001780;
    wa = RAM_SYM_03001780;
    pbe = (volatile u8 *)(uintptr_t)(wa + 0xFBEu);
    h = *(volatile u16 *)pbe;
    p176 = (volatile u16 *)(uintptr_t)(rec + 176);
    zero = 0;
    *p176 = h;
    _080012F18(rec_);
    h2 = *(volatile u16 *)(uintptr_t)(wa + 0x574u);
    {
        register u32 off __asm__("r1");
        __asm__("" : "=r"(off) : "0"(174));
        p = (volatile u16 *)(uintptr_t)(off + (uintptr_t)rec);
    }
    p174 = p;
    *p = h2;
    *p176 = *(volatile u16 *)pbe;
    *(volatile u16 *)(uintptr_t)(rec + 136) =
        (u16)_08024D0C((int)*(s16 *)(uintptr_t)p,
                          (int)*(s16 *)(uintptr_t)p176);
    {
        register s32 idx4 __asm__("r1");
        register volatile u8 *a188 __asm__("r0");
        register u32 v __asm__("r0");
        idx4 = (s32)*(s16 *)(uintptr_t)(rec + 136);
        idx4 = idx4 * 4;
        a188 = (volatile u8 *)(uintptr_t)(rec + 188);
        v = *(volatile u32 *)(uintptr_t)(a188 + idx4);
        *p174 = (u16)v;
    }
    {
        register volatile u16 *q __asm__("r1");
        q = p174;
        *(volatile u16 *)(uintptr_t)(rec + 180) =
            (u16)_080022E4((int)*(s16 *)(uintptr_t)q);
    }
    {
        register s32 b1 __asm__("r1") = 0;
        s32 b2;
        b1 = *(s8 *)((u8 *)wa + ((int)*(s16 *)(uintptr_t)p174 * 3) * 4 + 49);
        *(volatile u16 *)(uintptr_t)(rec + 172) = (u16)b1;
        b2 = *(volatile s8 *)(uintptr_t)
            ((u8 *)wa + ((int)*(s16 *)(uintptr_t)p174 * 3) * 4 + 48);
        *(volatile u16 *)(uintptr_t)(rec + 178) = (u16)b2;
    }
    *(volatile u32 *)(uintptr_t)(rec + 144) = 2;
    *(volatile u16 *)(uintptr_t)(rec + 186) = (u16)zero;
    ti = *(s16 *)(uintptr_t)p176;
    {
        u32 off = (u32)(ti * 2);
        wa = (volatile u8 *)(uintptr_t)((u8 *)wa + 0x1060);
        *(volatile u16 *)(uintptr_t)((u8 *)wa + off) = (u16)zero;
    }
    _0800D77C((void *)(uintptr_t)(rec + 156), 0, -32);
}

void _080013024(void *a) __attribute__((alias("MenuFF78_13024")));
void Sub_080013024(void *a) __attribute__((alias("MenuFF78_13024")));
void sub_080013024(void *a) __attribute__((alias("MenuFF78_13024")));
