// ============================================================================
// menu_ff78_f.c — reconstructed C for asm/menu_ff78.s (15 small leaves).
//
// All bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// except _0800D97C which binds the proven C lift.
// High-register prologues (r8/r9/sl) have no C-visible effect.
//
//   sub_080012E74 (0x080012E74) — ROM table s16 lookup via memcpy.
//   sub_080013E64 (0x080013E64) — countdown tick at rec+0x26C.
//   sub_080015B54 (0x080015B54) — countdown tick at rec+172.
//   sub_08001549C (0x08001549C) — init leaf (2B368 + 24068 branch).
//   sub_0800152A8 (0x0800152A8) — 0x10C3-gated setup leaf.
//   sub_080015230 (0x080015230) — FBC-gated + car-id {27..30}->1 leaf.
//   sub_080015C9C (0x080015C9C) — rec+84 = 6 leaf.
//   sub_08001411C (0x08001411C) — jump-table car-id -> rec+84 leaf.
//   sub_080014E3C (0x080014E3C) — event handler, cell rec+136.
//   sub_080014EA0 (0x080014EA0) — event handler, cell rec+138.
//   sub_080014F14 (0x080014F14) — event handler, cell WA+0x5E0.
//   sub_080014D94 (0x080014D94) — event handler, cell rec+134.
//   sub_080013AA8 (0x080013AA8) — gated 7B18 emit (table 0x080CB6B8).
//   sub_080013AF4 (0x080013AF4) — gated 7B18 pair (kinds 22/23).
//   sub_080011D48 (0x080011D48) — 7B18 + 2x 7BFC triple.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
__attribute__((weak)) void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void Sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) void *Sub_08004B68(void) { return 0; }
__attribute__((weak)) void *sub_08004B68(void) { return 0; }
// `_08002140`/`_08004D4C` are the CLOSURE's spellings for these two callees:
// 0x08002140 is a promoted entry whose `export` carries only `_08002140`, and
// the asm for 0x08004D4C defines `_08004D4C`/`sub_08004D4C`. A body that calls
// the `Sub_` twin would be refused by promotion_screen (and would fail the
// spliced link), so the weak host stubs and the calls below use the same two
// spellings the slice resolves.
__attribute__((weak)) int _08002140(void) { return 0; }
__attribute__((weak)) void _08004D4C(int a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void Sub_08002B368(u32 v) { (void)v; }
__attribute__((weak)) void Sub_08002B384(u32 v) { (void)v; }
// 0x0802b368 is a PROMOTED entry whose manifest `export` is exactly
// ['sub_0802B368'], so a body that splices against it must call the lowercase
// spelling -- `Sub_08002B368` would not survive the splice.
__attribute__((weak)) void sub_0802B368(u32 v) { (void)v; }
__attribute__((weak)) void Sub_08002B214(int v) { (void)v; }
__attribute__((weak)) void sub_0802B214(int v) { (void)v; }
__attribute__((weak)) void Sub_08002B3A4(void) {}
__attribute__((weak)) void sub_0802B3A4(void) {}
// 0x0802b3a4's manifest entry has NO `export` list, so the spliced section
// publishes only its c_name `_0802B3A4`; calling `sub_0802B3A4` leaves the
// slice link with an undefined reference.
__attribute__((weak)) void _0802B3A4(void) {}
__attribute__((weak)) void Sub_08002B234(void) {}
__attribute__((weak)) void sub_0802B234(void) {}
__attribute__((weak)) void Sub_08002618(u32 a, u32 b) { (void)a; (void)b; }
// 0x08002618 is a PROMOTED entry whose manifest `export` is exactly
// ['_08002618', 'sub_08002618']; src/event_dma_queue.c defines both as
// `#ifndef __APPLE__` aliases of ScenePost_2618. A spliced body must call one
// of the exported spellings, not the friendly `Sub_` name.
__attribute__((weak)) void sub_08002618(u32 a, u32 b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_080056F4(void *a, int b, int c) { (void)a; (void)b; (void)c; }
// 0x080056f4 is a PROMOTED entry whose manifest export is exactly
// ['_080056F4'], so a spliced body must call that spelling.
__attribute__((weak)) void _080056F4(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) int Sub_08024068(void) { return 0; }
__attribute__((weak)) void Sub_0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void sub_0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void Sub_08002E0A4(void *d, const void *s, u32 n) { (void)d; (void)s; (void)n; }
__attribute__((weak)) void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f) { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; }
#else
extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i);
// Closure spellings of the two 7Bxx helpers. These are NOT aliases of the
// `Sub_` names above -- the strong bodies live in scene_record_dispatch.c (7B18) and
// course_records.c (7BFC), so they need their own externs here. Declaring the
// friendly name alone is what makes FF_CALLEE resolve to C on the ROM build.
extern void sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i);
extern void *Sub_08004B68(void);
extern void *sub_08004B68(void);
extern int _08002140(void);
extern void _08004D4C(int a, int b, int c);
extern void Sub_08002B368(u32 v);
extern void Sub_08002B384(u32 v);
extern void sub_0802B368(u32 v);
extern void sub_0802B384(u32 v);  // closure spelling (7 digits, per nm)
extern void Sub_08002B214(int v);
extern void sub_0802B214(int v);  // closure spelling (7 digits, per nm)
extern void Sub_08002B3A4(void);
extern void sub_0802B3A4(void);  // closure spelling (7 digits, per nm)
extern void _0802B3A4(void);    // the only name the slice actually publishes
extern void Sub_08002B234(void);
extern void sub_0802B234(void);  // closure spelling (7 digits, per nm)
extern void Sub_08002618(u32 a, u32 b);
extern void sub_08002618(u32 a, u32 b);
extern void Sub_080056F4(void *a, int b, int c);
extern void _080056F4(void *a, int b, int c);
extern int Sub_08024068(void);
extern void Sub_0800D97C(void *a, int b);
extern void sub_0800D97C(void *a, int b);  // closure spelling
extern void Sub_08002E0A4(void *d, const void *s, u32 n);
extern void sub_0802E0A4(void *d, const void *s, u32 n);  // 8 digits: the real symbol
extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
#endif

// Call-site split for promoted bodies. `promotion_screen.py` accepts a promoted
// body only when every call target is spelled the way the assembly closure
// spells it, which here is `sub_08007Bxx`. The friendly `Sub_...` names are the
// host stubs above, which do not exist on the ROM build. Same split as
// MS_CALLEE in menu_stage.c; spelled separately per file to keep each
// translation unit self-contained.
#ifndef __APPLE__
#define FF_CALLEE(friendly, closure) closure
#else
#define FF_CALLEE(friendly, closure) friendly
#endif

// ----------------------------------------------------------------------------
// sub_080012E74 — copy 22 bytes ROM 0x0805F9A8 -> stack, return s16[idx].
//   r4 = idx; memcpy(sp, 0x0805F9A8, 22); r0 = s16[sp + (idx<<1)].
// The buffer is typed s16[12] (24 B) and indexed plainly: that is what makes
// agbcc reach for the signed halfword load `movs r1,#0; ldrsh r0,[r0,r1]`.
// A `volatile s16 *` cast over a u8 buffer instead yields `ldrh` plus an
// explicit lsls/asrs sign-extension (3 instructions where the ROM has 2).
s16 MenuFF78_12E74(int idx) {
    s16 buf[12];
    FF_CALLEE(Sub_08002E0A4, sub_0802E0A4)(buf, (const void *)(uintptr_t)0x0805F9A8u, 22);
    return buf[idx];
}
#ifndef __APPLE__
s16 _080012E74(int a) __attribute__((alias("MenuFF78_12E74")));
s16 Sub_080012E74(int a) __attribute__((alias("MenuFF78_12E74")));
s16 sub_080012E74(int a) __attribute__((alias("MenuFF78_12E74")));
#endif

// ----------------------------------------------------------------------------
// sub_080013E64 — countdown tick at rec+0x26C (155*4).
//   if u16[WA+0xFBC] != 3 return; if Sub_08002140==2: u32[rec+0x26C]=0
//   else u32[rec+0x26C]++, >180 -> Sub_08004D4C(21,0,0).
extern u8 FF78FWA[];
void MenuFF78_13E64(volatile u8 *rec) {
    int n;
    u8 *wa = (u8 *)(uintptr_t)FF78FWA;
    u32 off = 0xFBC;
    __asm__(".globl FF78FWA\nFF78FWA = 0x03001780\n");
    if (*(volatile u16 *)(uintptr_t)(wa + off) != 3)
        return;
    if (_08002140() != 2) {
        n = (int)*(volatile u32 *)(uintptr_t)(rec + 0x26C) + 1;
        *(volatile u32 *)(uintptr_t)(rec + 0x26C) = (u32)n;
        if (n > 180)
            _08004D4C(21, 0, 0);
    } else {
        *(volatile u32 *)(uintptr_t)(rec + 0x26C) = 0;
    }
}
#ifndef __APPLE__
void _080013E64(volatile u8 *a) __attribute__((alias("MenuFF78_13E64")));
void Sub_080013E64(volatile u8 *a) __attribute__((alias("MenuFF78_13E64")));
void sub_080013E64(volatile u8 *a) __attribute__((alias("MenuFF78_13E64")));
#endif

// ----------------------------------------------------------------------------
// sub_080015B54 — countdown tick at rec+172.
//   Same shape as 13E64 with counter at rec+172.
void MenuFF78_15B54(void *rec_) {
    // FINDING : byte-identical shape to MenuTick_080012D54 — the gate
    // is a TWO-POOL-WORD add, not a folded constant
    // (`ldr r0,=0x03001780; ldr r1,=0x0FBC; adds r0,r0,r1; ldrh r0,[r0] @
    // 0x08015B58..0x08015B5E`), and the `== 2` reset block is OUT OF LINE
    // after the pool (0x08015B90), reached by `beq @ 0x08015B6A`. The counter
    // is a plain (non-volatile) local: `volatile u32 v` spills to the stack.
#ifndef __APPLE__
    extern u8 MenuWaBaseF[] __asm__("MenuWaBaseF");
    __asm__(".globl MenuWaBaseF\nMenuWaBaseF = 0x03001780\n");
    uintptr_t base = (uintptr_t)MenuWaBaseF;
#else
    uintptr_t base = (uintptr_t)(0x03001780u);
#endif
    if (*(volatile u16 *)(uintptr_t)(base + 0xFBCu) == 3) {
        if (_08002140() == 2) goto reset;
        u32 v = *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 172);
        v += 1;
        *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 172) = v;
        // `cmp r0,#180; ble @ 0x08015B78` is a SIGNED compare.
        if ((s32)v > 180) {
            _08004D4C(21, 0, 0);
        }
    }
    return;
reset:
    *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 172) = 0;
}
// The ROM pads the body's last two bytes with 0x0000 (0x08015B9E..0x08015B9F);
// agbcc emits a one-instruction `nop` (0xC046) there instead.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080015B54(void *a) __attribute__((alias("MenuFF78_15B54")));
void Sub_080015B54(void *a) __attribute__((alias("MenuFF78_15B54")));
void sub_080015B54(void *a) __attribute__((alias("MenuFF78_15B54")));
#endif

// ----------------------------------------------------------------------------
// sub_08001549C — init leaf.
//   Sub_08002B368(1); u32[rec+16]=10; s16[rec+20]=0; u32[rec+48]=10;
//   u32[rec+44]=1; v=Sub_08024068; 16->s16[rec+24]=5, 39->s16[rec+24]=1;
//   u16[WA+0x107C]=1.
void MenuFF78_1549C(void *rec_) {
    // FINDING : the ROM keeps TWO value temporaries live rather than
    // rematerialising the immediates. `movs r1,#10` at 0x080154A6 feeds BOTH
    // `str r1,[r4,#16]` and `str r1,[r4,#48]`, and `movs r5,#1` at
    // 0x080154B0 feeds `str r5,[r4,#44]` and — across the `bl 0x08024068` —
    // `strh r5,[r4,#24]` at 0x080154C0. So both immediates need real locals,
    // and `one` must be callee-saved because the call sits between its two
    // uses. A named `rec` local costs an extra pointer copy (the store lands
    // on `r6` instead of `r4`), so the parameter is used directly.
//
    // `cmp r0,#16; beq @ 0x080154B8` jumps FORWARD past the `== 39` test, so
    // the `== 39` arm is the fall-through and `== 16` is the out-of-line one.
    // `sub_0802B368`, not `Sub_08002B368`: 0x0802b368 is a PROMOTED entry whose
    // manifest export is exactly `['sub_0802B368']`. The `Sub_` spelling is a
    // second `.thumb_set` hop whose intermediate the spliced section drops.
    sub_0802B368(1);
    u32 ten = 10;
    *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 16) = ten;
    *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 20) = 0;
    *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 48) = ten;
    u32 one = 1;
    *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 44) = one;
    int v = Sub_08024068();
    if (v == 16) goto set5;
    if (v != 39) goto wa;
    // (reached by fallthrough when v == 39; the label the draft used here was
    // never targeted, and `-Werror=unused-label` rejects it. A label emits no
    // bytes, so dropping it is codegen-neutral.)
    *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 24) = one;
    goto wa;
set5:
    *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 24) = 5;
wa:
#ifndef __APPLE__
    {
        // The WA write is a two-pool-word add (`ldr r0,=0x03001780;
        // ldr r1,=0x107C; adds @ 0x080154C8`). Naming the base absolutely in
        // a block-scoped local keeps both ldrs adjacent and unfolded; folding
        // to 0x030027FC emits a single word.
        extern u8 MenuWaBaseF[] __asm__("MenuWaBaseF");
        uintptr_t base = (uintptr_t)MenuWaBaseF;
        *(volatile u16 *)(uintptr_t)(base + 0x107Cu) = 1;
    }
#else
    *(volatile u16 *)(uintptr_t)(0x03001780u + 0x107Cu) = 1;
#endif
    return;
}
#ifndef __APPLE__
void _08001549C(void *a) __attribute__((alias("MenuFF78_1549C")));
void Sub_08001549C(void *a) __attribute__((alias("MenuFF78_1549C")));
void sub_08001549C(void *a) __attribute__((alias("MenuFF78_1549C")));
#endif

// ----------------------------------------------------------------------------
// sub_0800152A8 — u8[WA+0x10C3]-gated setup leaf.
//   0: Sub_08002618(1,1); s16[rec+136]=1;
//      Sub_08007770(1, 0x082B7410, 4, 0, 0, 3).
//   else: Sub_08002618(1,0); s16[rec+136]=0.
void MenuFF78_152A8(void *rec_) {
    // FINDING : the gate is a TWO-POOL-WORD add
    // (`ldr r0,=0x03001780; ldr r1,=0x10C3; adds; ldrb r4,[r0] @
    // 0x080152AE..0x080152B4`), and the gate byte is passed through as the
    // 5th argument — it lives in a callee-saved register (r4) across both
    // calls and lands at `sp,#0`. The `!= 0` arm is the fall-through
    // (`bne @ 0x080152B8` jumps forward to 0x080152EC).
#ifndef __APPLE__
    extern u8 MenuWaBaseF[] __asm__("MenuWaBaseF");
    uintptr_t base = (uintptr_t)MenuWaBaseF;
#else
    uintptr_t base = (uintptr_t)(0x03001780u);
#endif
#ifndef __APPLE__
    // The ROM loads the gate byte straight into r4
    // (`ldrb r4,[r0] @ 0x080152B4`), so the local is pinned to r4; unpinned,
    // agbcc loads via r0 and adds a copy.
    register u32 gate __asm__("r4") = *(volatile u8 *)(uintptr_t)(base + 0x10C3u);
#else
    u32 gate = *(volatile u8 *)(uintptr_t)(base + 0x10C3u);
#endif
    if (gate == 0) {
        sub_08002618(1, 1);
        *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 136) = 1;
        // 0x082B7410 is reached by `ldr r1,[pc,#28] @ 0x080152CA`, emitted
        // before the stack slots because it is the 2nd argument.
        _08007770(1, (void *)(uintptr_t)0x082B7410u, 4, 0, gate, 3);
    } else {
        sub_08002618(1, 0);
        *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 136) = 0;
    }
}
#ifndef __APPLE__
void _0800152A8(void *a) __attribute__((alias("MenuFF78_152A8")));
void Sub_0800152A8(void *a) __attribute__((alias("MenuFF78_152A8")));
void sub_0800152A8(void *a) __attribute__((alias("MenuFF78_152A8")));
#endif

// ----------------------------------------------------------------------------
// sub_080015230 — (base, rec): s16[base+136]=0; if u16[WA+0xFBC]==3 and
//   u8[WA+0x10C3]==0: u8[rec+88]=0, Sub_080056F4(rec,1,1), s16[base+136]=1.
//   Then car-id map: 27..30 -> s16[rec+84]=1 else 6.
void MenuFF78_15230(void *base_, void *rec_) {
    // The ROM keeps the work-area base in r2 across BOTH gate reads and loads
    // 0x0FBC/0x10C3 as separate pool words (`ldr r2,=0x03001780;
    // ldr r1,=0x0FBC; adds`), so the base must be a runtime pointer assigned
    // after the first store -- a folded literal would emit one 0x030027FC word.
    register u8 *base __asm__("r5");
    u8 *rec = (u8 *)rec_;
    extern u8 MenuWaBaseF[] __asm__("MenuWaBaseF");
    u8 *wa;
    register u32 zero __asm__("r0");
    int car;
    __asm__(".globl MenuWaBaseF\nMenuWaBaseF = 0x03001780\n");
    base = (u8 *)base_ + 136;
    zero = 0;
    *(u16 *)(uintptr_t)base = (u16)zero;
    wa = (u8 *)(uintptr_t)MenuWaBaseF;
    if (*(u16 *)(uintptr_t)(wa + 0xFBC) == 3) {
        {
            u8 *p88 = (u8 *)(uintptr_t)(rec + 88);
            zero = 0;
            *p88 = (u8)zero;
        }
        if (*(u8 *)(uintptr_t)(wa + 0x10C3) == 0) {
            FF_CALLEE(Sub_080056F4, _080056F4)(rec_, 1, 1);
            *(u16 *)(uintptr_t)base = 1;
        }
    }
    // Signed s16 car id. Two separate `goto`s are what keep agbcc on
    // `cmp #30;bgt;cmp #27;blt`; a combined `&&`/`||` range test folds to
    // `(u32)(car-27) <= 3` with an ldrh+shift read instead of ldrsh.
    car = *(s16 *)(uintptr_t)((u8 *)FF_CALLEE(Sub_08004B68, sub_08004B68)() + 2);
    switch (car) {
    case 27: case 28: case 29: case 30:
        *(u16 *)(uintptr_t)(rec + 84) = 1;
        break;
    default:
        *(u16 *)(uintptr_t)(rec + 84) = 6;
        break;
    }
}
#ifndef __APPLE__
void _080015230(void *a, void *b) __attribute__((alias("MenuFF78_15230")));
void Sub_080015230(void *a, void *b) __attribute__((alias("MenuFF78_15230")));
void sub_080015230(void *a, void *b) __attribute__((alias("MenuFF78_15230")));
#endif
// 106-byte body, two short of the section's 4-byte alignment: gas pads with
// `nop` (0x46c0) where the ROM holds `00 00`.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// sub_080015C9C — (dead, rec): s16[rec+84] = 6 (car id fetched, unused).
void MenuFF78_15C9C(int dead, void *rec_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    sub_08004B68();
    *(volatile s16 *)(uintptr_t)(rec + 84) = 6;
}
#ifndef __APPLE__
void _080015C9C(int a, void *b) __attribute__((alias("MenuFF78_15C9C")));
void sub_080015C9C(int a, void *b) __attribute__((alias("MenuFF78_15C9C")));
#endif

// ----------------------------------------------------------------------------
// sub_0800152A8 alias block is above; 1411C below.
// sub_08001411C — (dead, rec): car-id jump table -> s16[rec+84].
//   v = (s16)(u16[Sub_08004B68+2] - 16); 5 when v in {0..4,6,19} else 6.
void MenuFF78_1411C(int dead, void *rec_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    // `sub_08004B68`, not `Sub_08004B68`: 0x08004b68 is a PROMOTED body and its
    // manifest entry exports exactly `sub_08004B68` (one hop to `MgrGet80`).
    // `Sub_08004B68` is an alias OF `_08004B68`, itself an alias of `MgrGet80`,
    // so it is a second `.thumb_set` hop whose intermediate the spliced section
    // does not carry -- the reference stayed undefined with every other gate
    // green. Only the call site in THIS body moves; the sibling bodies keep the
    // `Sub_` spelling, which their own not-yet-promoted status still resolves.
    u16 car = *(volatile u16 *)(uintptr_t)((volatile u8 *)sub_08004B68() + 2);
    int v = (int)(s16)(car - 16);
    switch (v) {
    case 0: case 1: case 2: case 3: case 4: case 6: case 19:
        *(volatile s16 *)(uintptr_t)(rec + 84) = 5;
        break;
        // Eight case labels over 0..19 are what push agbcc to a jump table;
        // 18 shares the default arm (the table entry is 0x8014198).
    case 18:
    default:
        // Out-of-range (>19 unsigned) also yields 6 (bhi default arm).
        *(volatile s16 *)(uintptr_t)(rec + 84) = 6;
        break;
    }
}
// The body is 138 bytes, two short of its section's 4-byte alignment; gas
// closes a Thumb code section with `nop` (0x46c0) where the ROM holds
// `00 00`. Same file-scope pad as MenuFF78_11CBC in menu_ff78_d.c.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001411C(int a, void *b) __attribute__((alias("MenuFF78_1411C")));
void Sub_08001411C(int a, void *b) __attribute__((alias("MenuFF78_1411C")));
void sub_08001411C(int a, void *b) __attribute__((alias("MenuFF78_1411C")));
#endif

// ----------------------------------------------------------------------------
// sub_080014E3C — (rec, dead, ev): cell = s16[rec+136].
//   ev==1: Sub_08002B214(u16[0x080CB7CE + entry*2]).
//   ev==32: cell--; ev==16: cell++. Clamp 1..12.
//   Changed vs entry -> Sub_08002B384(3).
void MenuFF78_14E3C(void *rec_, int dead, u32 ev_) {
    // FINDING : signed reads need an `int` destination (an `s16` destination is
    // only compared for equality, so agbcc skips the sign extension and loads
    // `ldrh` + copies). The decrement/increment arms store the cell through a
    // `u16` read (`ldrh;subs/adds;strh`), i.e. unsigned -- a `s16` read there
    // inserts a redundant `lsls/asrs` pair. The final compare is `cmp r7,r0`
    // (entry first), so the source orders it `entry != c`.
    (void)dead;
    u8 *rec = (u8 *)rec_;
    u16 ev = (u16)ev_;
    int entry = *(s16 *)(uintptr_t)(rec + 136);
    int c;
    if (ev == 1) {
        // Assembler-resolved table base: the ROM loads the pool word into r0
        // and shifts `entry` into r1 (`ldr r0,=0x080CB7CE; lsls r1,r7,#1;
        // adds r1,r1,r0`). A plain literal makes agbcc compute the index first
        // in r0 and load the constant second.
        extern u8 E3C_TBL[];
        u8 *tbl;
        u16 v;
        __asm__(".globl E3C_TBL\nE3C_TBL = 0x080CB7CE\n");
        tbl = (u8 *)(uintptr_t)E3C_TBL;
        v = *(u16 *)(uintptr_t)(tbl + ((u32)entry << 1));
        FF_CALLEE(Sub_08002B214, sub_0802B214)(v);
    }
    if (ev == 32) {
        *(u16 *)(uintptr_t)(rec + 136) =
            (u16)(*(u16 *)(uintptr_t)(rec + 136) - 1);
    }
    if (ev == 16) {
        *(u16 *)(uintptr_t)(rec + 136) =
            (u16)(*(u16 *)(uintptr_t)(rec + 136) + 1);
    }
    c = *(s16 *)(uintptr_t)(rec + 136);
    if (c <= 0)
        *(u16 *)(uintptr_t)(rec + 136) = 1;
    c = *(s16 *)(uintptr_t)(rec + 136);
    if (c > 12)
        *(u16 *)(uintptr_t)(rec + 136) = 12;
    c = *(s16 *)(uintptr_t)(rec + 136);
    if (entry != c)
        FF_CALLEE(Sub_08002B384, sub_0802B384)(3);
}
#ifndef __APPLE__
void _080014E3C(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14E3C")));
void Sub_080014E3C(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14E3C")));
void sub_080014E3C(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14E3C")));
#endif

// ----------------------------------------------------------------------------
// sub_080014EA0 — (rec, dead, ev): cell = s16[rec+138].
//   ev==1: Sub_08002B3A4 then Sub_08002B384(u16[0x080CB7E8+entry*2]).
//   ev==32: cell-- + 2B3A4; ev==16: cell++ + 2B3A4. Clamp 1..8.
//   Changed vs entry -> Sub_08002B384(3).
void MenuFF78_14EA0(void *rec_, int dead, u32 ev_) {
    // Same shape as MenuFF78_14E3C (signed `int` reads, unsigned RMW arms) with
    // a 2B3A4 side-effect per arm and cell rec+138 (clamp 1..8). The ev==1 arm
    // re-reads the cell AFTER the 2B3A4 call, so the table index uses a fresh
    // `ldrsh` rather than the preserved `entry` -- a plain re-read (not
    // `entry`) is what stops agbcc reusing r7.
    (void)dead;
    u8 *rec = (u8 *)rec_;
    u16 ev = (u16)ev_;
    int entry = *(s16 *)(uintptr_t)(rec + 138);
    int c;
    if (ev == 1) {
        extern u8 EA0_TBL[];
        u8 *tbl;
        u16 v;
        FF_CALLEE(Sub_08002B3A4, _0802B3A4)();
        __asm__(".globl EA0_TBL\nEA0_TBL = 0x080CB7E8\n");
        // Base load FIRST, then the reload: the ROM reserves r0 for the pool
        // word, so the reload's zero index lands in r2. Writing the reload
        // before the base assignment makes agbcc take r0 as the index scratch
        // and load the base second.
        tbl = (u8 *)(uintptr_t)EA0_TBL;
        c = *(s16 *)(uintptr_t)(rec + 138);
        v = *(u16 *)(uintptr_t)(tbl + ((u32)c << 1));
        FF_CALLEE(Sub_08002B384, sub_0802B384)(v);
    }
    if (ev == 32) {
        *(u16 *)(uintptr_t)(rec + 138) =
            (u16)(*(u16 *)(uintptr_t)(rec + 138) - 1);
        FF_CALLEE(Sub_08002B3A4, _0802B3A4)();
    }
    if (ev == 16) {
        *(u16 *)(uintptr_t)(rec + 138) =
            (u16)(*(u16 *)(uintptr_t)(rec + 138) + 1);
        FF_CALLEE(Sub_08002B3A4, _0802B3A4)();
    }
    if (*(s16 *)(uintptr_t)(rec + 138) <= 0)
        *(u16 *)(uintptr_t)(rec + 138) = 1;
    if (*(s16 *)(uintptr_t)(rec + 138) > 8)
        *(u16 *)(uintptr_t)(rec + 138) = 8;
    if (entry != *(s16 *)(uintptr_t)(rec + 138))
        FF_CALLEE(Sub_08002B384, sub_0802B384)(3);
}
#ifndef __APPLE__
void _080014EA0(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14EA0")));
void Sub_080014EA0(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14EA0")));
void sub_080014EA0(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14EA0")));
#endif

// ----------------------------------------------------------------------------
// sub_080014F14 — (rec, dead, ev): cell = s16[WA+0x5E0].
//   ev==32: s16[rec+140]=1, cell=1. ev==16: s16[rec+140]=2, cell=0.
//   Cell changed vs entry -> Sub_08002B384(1).
void MenuFF78_14F14(void *rec_, int dead, u32 ev_) {
    (void)dead;
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 ev = (u16)ev_;
    // Assembler-resolved work-area base so the 0x5E0 offset stays a separate
    // constant (the ROM synthesises it as `movs #0xbc; lsls #3`) added at
    // runtime, instead of folding base+0x5E0 into one pool word.
    extern u8 J14F14_WA[];
    volatile u8 *wa;
    int entry;
    __asm__(".globl J14F14_WA\nJ14F14_WA = 0x03001780\n");
    wa = (volatile u8 *)(uintptr_t)J14F14_WA;
    // Signed int destination keeps the ROM's `movs r0,#0; ldrsh r5,[r3,r0]`
    // (a plain s16 compare would let agbcc use `ldrh`, which needs no sign
    // extension for an equality test).
    entry = *(s16 *)(uintptr_t)(wa + 0x5E0u);
    if (ev == 32) {
        *(volatile s16 *)(uintptr_t)(rec + 140) = 1;
        *(volatile s16 *)(uintptr_t)(wa + 0x5E0u) = 1;
    }
    if (ev == 16) {
        // The ROM computes the rec+140 store address (into r0), then
        // materialises the `0` into r2 (reusing the now-dead `ev`) and the `2`
        // into r1, then stores both. Pin the zero so agbcc emits it in r2 at
        // this point rather than folding it into a fresh register late.
        volatile s16 *p16;
        register s16 zero __asm__("r2");
        register s16 two __asm__("r1");
        p16 = (volatile s16 *)(uintptr_t)(rec + 140);
        zero = 0;
        two = 2;
        *p16 = two;
        *(volatile s16 *)(uintptr_t)(wa + 0x5E0u) = zero;
    }
    if (*(s16 *)(uintptr_t)(wa + 0x5E0u) != entry)
        FF_CALLEE(Sub_08002B384, sub_0802B384)(1);
}
#ifndef __APPLE__
void _080014F14(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14F14")));
void Sub_080014F14(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14F14")));
void sub_080014F14(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14F14")));
#endif

// ----------------------------------------------------------------------------
// sub_080014D94 — (rec, dead, ev): cell = s16[rec+134].
//   ev==2: 2B368(4) + reset quad (u32[+8]=10, s16[+12]=0, u32[+40]=10,
//     u32[+36]=0). cell==3 && ev==1: 2B368(1) + same reset.
//   ev==64: D97C(rec+128,15), cell--; ev==128: D97C(rec+128,15), cell++.
//   Clamp 0..3. Changed vs entry -> 2B384(2); new==1 -> 2B234.
void MenuFF78_14D94(void *rec_, int dead, u32 ev_) {
    // Signed `int` `entry` read (ldrsh), unsigned RMW arms (ldrh), and the
    // reset quad materialised as two reused temporaries (`ten` in r1, `zero`
    // in r0) rather than four independent immediates.
    (void)dead;
    u8 *rec = (u8 *)rec_;
    u16 ev = (u16)ev_;
    int entry = *(s16 *)(uintptr_t)(rec + 134);
    if (ev == 2) {
        FF_CALLEE(Sub_08002B368, sub_0802B368)(4);
        register u32 ten __asm__("r1");
        register u32 zero __asm__("r0");
        ten = 10;
        *(u32 *)(uintptr_t)(rec + 8) = ten;
        zero = 0;
        *(u16 *)(uintptr_t)(rec + 12) = (u16)zero;
        *(u32 *)(uintptr_t)(rec + 40) = ten;
        *(u32 *)(uintptr_t)(rec + 36) = zero;
    }
    if (*(u16 *)(uintptr_t)(rec + 134) == 3 && ev == 1) {
        FF_CALLEE(Sub_08002B368, sub_0802B368)(1);
        {
            register u32 ten __asm__("r1");
            register u32 zero __asm__("r0");
            ten = 10;
            *(u32 *)(uintptr_t)(rec + 8) = ten;
            zero = 0;
            *(u16 *)(uintptr_t)(rec + 12) = (u16)zero;
            *(u32 *)(uintptr_t)(rec + 40) = ten;
            *(u32 *)(uintptr_t)(rec + 36) = zero;
        }
    }
    if (ev == 64) {
        FF_CALLEE(Sub_0800D97C, sub_0800D97C)((void *)(uintptr_t)(rec + 128), 15);
        *(u16 *)(uintptr_t)(rec + 134) =
            (u16)(*(u16 *)(uintptr_t)(rec + 134) - 1);
    }
    if (ev == 128) {
        FF_CALLEE(Sub_0800D97C, sub_0800D97C)((void *)(uintptr_t)(rec + 128), 15);
        *(u16 *)(uintptr_t)(rec + 134) =
            (u16)(*(u16 *)(uintptr_t)(rec + 134) + 1);
    }
    if (*(s16 *)(uintptr_t)(rec + 134) <= 0)
        *(u16 *)(uintptr_t)(rec + 134) = 0;
    if (*(s16 *)(uintptr_t)(rec + 134) > 3)
        *(u16 *)(uintptr_t)(rec + 134) = 3;
    if (entry != *(s16 *)(uintptr_t)(rec + 134)) {
        FF_CALLEE(Sub_08002B384, sub_0802B384)(2);
        if (*(u16 *)(uintptr_t)(rec + 134) == 1)
            FF_CALLEE(Sub_08002B234, sub_0802B234)();
    }
}
#ifndef __APPLE__
void _080014D94(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14D94")));
void Sub_080014D94(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14D94")));
void sub_080014D94(void *a, int b, u32 c) __attribute__((alias("MenuFF78_14D94")));
#endif
// 166-byte body, two short of the section's 4-byte alignment: gas pads with
// `nop` (0x46c0) where the ROM holds `00 00`.
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// sub_080013AA8 — gated 7B18 emit over table 0x080CB6B8 (stride 8).
//   Gate: s16[rec+186]==0 && s16[rec+170]==1.
//   r2 = u32[0x080CB6B8 + s16[rec+172]*8], r3 = u32[...+4]:
//   Sub_08007B18(rec, 6, r2, r3, 4, 1, 1, 0).
void MenuFF78_13AA8(void *rec_) {
    u8 *rec = (u8 *)rec_;
    if (*(s16 *)(uintptr_t)(rec + 186) != 0) return;
    if (*(s16 *)(uintptr_t)(rec + 170) != 1) return;
    // Assembler-resolved table base: the ROM keeps 0x080CB6B8 in a register and
    // derives the second entry with `adds r3,#4` one pool word, rather than
    // folding base+4 into a second 0x080CB6BC literal.
    extern u8 J13AA8_TBL[];
    u8 *tbl = (u8 *)(uintptr_t)J13AA8_TBL;
    u8 *tbl4;
    u32 off;
    u32 r2, r3;
    __asm__(".globl J13AA8_TBL\nJ13AA8_TBL = 0x080CB6B8\n");
    off = (u32)(s32)*(s16 *)(uintptr_t)(rec + 172) << 3;
    r2 = *(volatile u32 *)(uintptr_t)(tbl + off);
    tbl4 = tbl + 4;
    r3 = *(volatile u32 *)(uintptr_t)(tbl4 + off);
    FF_CALLEE(Sub_08007B18, sub_08007B18)(rec_, 6, (int)r2, (int)r3, 4, 1, 1, 0);
}
#ifndef __APPLE__
void _080013AA8(void *a) __attribute__((alias("MenuFF78_13AA8")));
void Sub_080013AA8(void *a) __attribute__((alias("MenuFF78_13AA8")));
void sub_080013AA8(void *a) __attribute__((alias("MenuFF78_13AA8")));
#endif

// ----------------------------------------------------------------------------
// sub_080013AF4 — gated 7B18 pair.
//   Gate: u16[rec+186]==1 && s16[rec+166]==1. r5 = s16[rec+178].
//   r5==0: 7B18(rec,22,u32[rec+156],u32[rec+160]+8,6,1,1,0).
//   r5==1: 7B18(rec,23,u32[rec+156]+24,u32[rec+160]+8,6,1,1,0).
void MenuFF78_13AF4(void *rec_) {
    u8 *rec = (u8 *)rec_;
    register int g __asm__("r4");
    s16 v;
    if (*(volatile u16 *)(uintptr_t)(rec + 186) != 1) return;
    g = *(s16 *)(uintptr_t)(rec + 166);
    if (g != 1) return;
    v = *(s16 *)(uintptr_t)(rec + 178);
    // A switch, not `if (v == 0) ... else if (v == 1) ...`: the ROM lays out a
    // compare-and-branch chain (`cmp;beq case0;cmp;beq case1;b end`) with the
    // case bodies after it. An if/else chain instead inlines case0 and reuses
    // the running rec+178 pointer, which the beq target cannot.
    switch (v) {
    case 0: {
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 156);
        u32 r3 = *(volatile u32 *)(uintptr_t)(rec + 160) + 8;
        FF_CALLEE(Sub_08007B18, sub_08007B18)(rec_, 22, (int)r2, (int)r3, 6, g, g, v);
        break;
    }
    case 1: {
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 156) + 24;
        u32 r3 = *(volatile u32 *)(uintptr_t)(rec + 160) + 8;
        FF_CALLEE(Sub_08007B18, sub_08007B18)(rec_, 23, (int)r2, (int)r3, 6, v, v, 0);
        break;
    }
    }
}
#ifndef __APPLE__
void _080013AF4(void *a) __attribute__((alias("MenuFF78_13AF4")));
void Sub_080013AF4(void *a) __attribute__((alias("MenuFF78_13AF4")));
void sub_080013AF4(void *a) __attribute__((alias("MenuFF78_13AF4")));
#endif

// ----------------------------------------------------------------------------
// sub_080011D48 — 7B18 + two 7BFC records.
//   7B18(rec,9,152,88,3,1,1,0);
//   7BFC(rec+40,u32[rec+276],u32[rec+280],152,40,3,1,1,0);
//   7BFC(rec+40,u32[rec+288],u32[rec+292],152,64,3,1,1,0).
void MenuFF78_11D48(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    // rec+40 is live across the second 7BFC call, so it must be materialised
    // into a callee-saved register (r8) *before* the record-word loads -- the
    // ROM's order. Inlining the expression as the argument makes agbcc sink
    // the rematerialisation past both loads.
    void *dst;
    FF_CALLEE(Sub_08007B18, sub_08007B18)(rec_, 9, 152, 88, 3, 1, 1, 0);
    dst = (void *)(uintptr_t)(rec + 40);
    {
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 276);
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 280);
        FF_CALLEE(Sub_08007BFC, sub_08007BFC)(dst, (int)r1, (int)r2, 152, 40, 3, 1, 1, 0);
    }
    {
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 288);
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 292);
        FF_CALLEE(Sub_08007BFC, sub_08007BFC)(dst, (int)r1, (int)r2, 152, 64, 3, 1, 1, 0);
    }
}
#ifndef __APPLE__
void _080011D48(void *a) __attribute__((alias("MenuFF78_11D48")));
void sub_080011D48(void *a) __attribute__((alias("MenuFF78_11D48")));
#endif
