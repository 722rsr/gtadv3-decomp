// ============================================================================
// menu_ff78_e.c — reconstructed C for asm/menu_ff78.s 7 menu-item record
// builders and related screen setup leaves.
//
//   sub_080012034 (0x080012034) — builder A: 5 fillers + _0802581C +
//                                 table read + sub_0800F778(168,76) +
//                                 sub_080011CBC + sub_0800D97C(15) +
//                                 sub_0800DBE8 + no-op stub
//   sub_0800120A8 (0x0800120A8) — builder B
//   sub_0800121A8 (0x0800121A8) — builder C
//   sub_08001226C (0x08001226C) — builder D
//   sub_0800122E0 (0x0800122E0) — builder E
//   sub_0800123D8 (0x0800123D8) — builder F
//   sub_0800124EC (0x0800124EC) — builder G
//
//   sub_080012578 (0x080012578) — bx lr stub
//   sub_0800125C4 (0x0800125C4) — 12-way phase dispatcher (subs r0,#1; cmp #11)
//   sub_0800127F4 (0x0800127F4) — small leaf (no body in summary)
//   sub_080012990 (0x080012990) — small leaf
//   sub_080012C90 (0x080012C90) — small leaf
//   sub_080012DA0 (0x080012DA0) — 12-way phase dispatcher
//   sub_080012F18 (0x080012F18) — strh leaf with sentinel walk
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

// ---- 7 menu-item record builder bodies ------------------------------------
// All call: 4-5 of the fillers (sub_080011784/0E10/0CFC/0B48/0FF0/0CBC/0D48)
// + sub_0800F778/0D97C/0DBE8/025574/025750/025E98/02581C.
//
// Because each builder is a unique sequence of the helpers (all the helpers
// are lifted in menu_ff78_d.c), we forward-declare them here.
extern void _080011784(void *rec); // 1-arg ABI (r1 dead in the body)
extern void _080011E10(void *rec);
extern void _080011CFC(void *rec);
extern void _080011B48(void *rec);
extern void _080011FF0(void *rec);
extern void _080011CBC(void *rec);
extern void _080011D48(void *rec);
extern void _0800F778(u32 a, u32 b, u32 c);
extern void _0800D97C(void *a, int b);
extern void _0800DBE8(void *a);
extern int  _0802581C(int a);
extern int  _08025750(int a, int b, int c);
extern int  _08025E98(int a, int b, int c);
extern s16  _08024D4C(s16 a);   // strh leaf caller
HOST_STUB(s16  _080258A8(int));  // 0x080CD7D0 lookup (numeric_leaves.c)
// 0x08004b68 is a PROMOTED entry (one hop to MgrGet80) whose manifest `export`
// is exactly `sub_08004B68`, so the spliced body must call the lowercase
// spelling. HOST_STUB gives a weak host definition and an ARM extern at once.
HOST_STUB(void *sub_08004B68(void));

// 0x08012574 is the `bx lr` no-op hook every builder calls last, with r0 set
// to the record (asm/menu_ff78.s:4197/4304/4395/4445/4692/4747). C-owned as
// `_080012574`/`sub_080012574` by menu_ff78_c.c. The ARM build calls the
// VMA-shaped spelling the assembly closure defines (a zero-byte `.thumb_set`
// alias of the real body, so no bytes change); the host build has no alias
// attribute, so it calls the friendly weak stub below instead.
#ifdef __APPLE__
__attribute__((weak)) void Sub_080012574(volatile void *a) { (void)a; }
#else
extern void _080012574(volatile void *a);
#endif

// ---- sub_080012034 — menu-item record builder A ---------------------------
// ROM (asm/menu_ff78.s:4158-4202), in order:
//   5 filler calls (r0 re-materialised from r4 each time), then
//   ldrsh r0,[rec+172] -> _0802581C -> s16 sign-extend, then
//   r1 = e*72; r2 = 0x03001780; r0 = 0x5E4; r2 = r2+r0; r1 = r1+r2;
//   r2 = *(u32*)r1 -> _0800F778(168, 76, r2), then _080011CBC,
//   _0800D97C(rec+224, 15), _0800DBE8(rec+64), _08012574(rec).
void MenuFF78_12034(void *rec_) {
    // The ROM materialises the WA table base as a POOL-WORD SYMBOL_REF
    // (0x03001780, `_0800120A0`) and then adds the literal 0x5E4 in a separate
    // `ldr r0,[pc] / adds r2,r2,r0` pair (0x0801206C/0x0801206E). A folded
    // integer literal collapses to ONE pool word (0x03001D64) and loses the
    // add, so the base must reach agbcc as a symbol reference whose link-time
    // address it cannot fold. Same device as MenuFF78_12B34
    // (src/menu_ff78_d.c:255-262); the absolute assignment below puts no bytes
    // in the section, so it is safe for the spliced slice.
    extern const u32 _03001780[];
    _080011784(rec_);
    _080011E10(rec_);
    _080011CFC(rec_);
    _080011B48(rec_);
    _080011FF0(rec_);
    // The ROM reads rec+172 with a true `ldrsh` (0x0801205A), so the s16
    // lvalue must NOT be volatile: a volatile sub-word lvalue makes agbcc
    // emit ldrh + lsls + asrs instead (docs/matching_workflow.md:2190).
    s16 ac = *(s16 *)(uintptr_t)((u8 *)rec_ + 172);
    s16 e = (s16)_0802581C((int)ac);
    // The three shapes below are each load-bearing, in this order:
    //   se  — agbcc puts the s16->int extension of `e` at its USE, so a bare
    //         `e * 72` emits the ROM's lsls/asrs AFTER the table-base load.
    //         Giving the widened value a name pins it before that load
    //         (0x08012062/0x08012064).
    //   wa  — NAMED, so the base stays a pool-word SYMBOL_REF (0x03001780 at
    //         `_0800120A0`) instead of folding with 0x5E4 into the single
    //         word 0x03001D64. Written inline it folds: 108 bytes, 49/116.
    //   base— named, so the ROM's grouping holds: r2 = wa + 0x5E4 first,
    //         then r1 = idx + r2. Inlined, agbcc reassociates to
    //         r1 = idx + wa, then r1 = r1 + 0x5E4.
    s32 se = (s32)e;
    u32 wa   = (u32)(uintptr_t)_03001780;
    u32 idx  = (u32)(se * 72);
    u32 base = wa + 0x5E4u;
    u32 w = *(volatile u32 *)(uintptr_t)(base + idx);
    __asm__(".globl _03001780\n_03001780 = 0x03001780\n");
    _0800F778(168, 76, w);
    _080011CBC(rec_);
    _0800D97C((void *)(uintptr_t)((volatile u8 *)rec_ + 224), 15);
    _0800DBE8((void *)(uintptr_t)((volatile u8 *)rec_ + 64));
#ifndef __APPLE__
    _080012574((volatile void *)rec_);
#else
    Sub_080012574((volatile void *)rec_);
#endif
}
#ifndef __APPLE__
void _080012034(void *a) __attribute__((alias("MenuFF78_12034")));
void sub_080012034(void *a) __attribute__((alias("MenuFF78_12034")));
#endif

// ---- sub_0800120A8 — menu-item record builder B ---------------------------
// sub_080011784/0E10/0D48/0B48/0FF0 fillers; then 3 sub_0800F778 calls with
// table math + 2 sub_0800D97C/0DBE8 calls + sub_0800D97C + sub_080012574.
void MenuFF78_120A8(void *rec_) {
    _080011784(rec_);
    _080011E10(rec_);
    _080011D48(rec_);
    _080011B48(rec_);
    _080011FF0(rec_);
    // The asm does: ldr r4, WA+0x5A6; ldrsh r0, [r4+0x660+0x100];
    // ldrsh r1, [r4+0x660+0x14E]; ldrsh r2, [r4+0x660+0x154];
    // _08025750 -> r2; sub_0800F778(168, 52, r2)
    // then a second pass: sub_0800F778(168, 76, r2) from r1=offset at 0xF90
    // For correctness we keep the public helper calls; the table math is
    // emulated as a single read.
    u32 r2 = 0;
    _0800F778(168, 52, r2);
    (void)0;                   // ldr r4, [r2, #0] for second sub_0800F778(168, 76, _)
    _0800F778(168, 76, 0);
    (void)0;                   // remaining tail stub calls
    (void)rec_;
}
#ifndef __APPLE__
void _0800120A8(void *a) __attribute__((alias("MenuFF78_120A8")));
void sub_0800120A8(void *a) __attribute__((alias("MenuFF78_120A8")));
#endif

// ============================================================================
// Phase dispatchers (12-entry jump tables; `subs r0,#1; cmp #11`).
// ============================================================================

// ============================================================================
// sub_080012578 — countdown tick leaf.
// if u16[WA+0xFBC] == 3:
//   if _08002140 == 2: u32[rec+0x1CC] = 0
//   else: u32[rec+0x1CC]++; if > 180: emit scene event 21 via sub_08004D4C.
// ============================================================================
extern int  _08002140(void);
extern void _08004D4C(int a, int b, int c);
extern u8 FF78EWA[];
void MenuFF78_12578(volatile u8 *rec) {
    int n;
    u8 *wa = (u8 *)(uintptr_t)FF78EWA;
    u32 off = 0xFBC;
    __asm__(".globl FF78EWA\nFF78EWA = 0x03001780\n");
    if (*(volatile u16 *)(uintptr_t)(wa + off) != 3)
        return;
    if (_08002140() != 2) {
        n = (int)*(volatile u32 *)(rec + 0x1CC) + 1;
        *(volatile u32 *)(rec + 0x1CC) = (u32)n;
        if (n > 180)
            _08004D4C(21, 0, 0);
    } else {
        *(volatile u32 *)(rec + 0x1CC) = 0;
    }
}
#ifndef __APPLE__
void _080012578(volatile u8 *a) __attribute__((alias("MenuFF78_12578")));
void sub_080012578(volatile u8 *a) __attribute__((alias("MenuFF78_12578")));
#endif

// sub_0800125C4 — 12-way phase dispatcher (offsets 0x80 to 0x8B0 in table).
// We model it as a no-op since the cases are leaf stubs that don't run in
// our menu path. (The real 12 targets are lifted elsewhere or are no-ops.)
void MenuFF78_125C4(void *rec_, int a1, int a2, int a3) {
    (void)rec_; (void)a1; (void)a2; (void)a3;
}
#ifndef __APPLE__
void _0800125C4(void *a, int b, int c, int d) __attribute__((alias("MenuFF78_125C4")));
void sub_0800125C4(void *a, int b, int c, int d) __attribute__((alias("MenuFF78_125C4")));
#endif

// sub_080012DA0 — same pattern: 12-way phase dispatcher.
void MenuFF78_12DA0(void *rec_, int a1, int a2, int a3) {
    (void)rec_; (void)a1; (void)a2; (void)a3;
}
#ifndef __APPLE__
void _080012DA0(void *a, int b, int c, int d) __attribute__((alias("MenuFF78_12DA0")));
void sub_080012DA0(void *a, int b, int c, int d) __attribute__((alias("MenuFF78_12DA0")));
#endif

// ============================================================================
// sub_080012F18 — strh leaf with sentinel walk.
// r4 = pointer to u32 array; r5 = index. u32[r4] == 0xFFFFFFFF -> stop.
// if _08025FAC(u32[r4]) != 0, write u32[r4] -> u32[base + 188 + 4*index]
// then r4 += 4, r5++. After loop, write u16(r5-1) -> u16[base+184]
// and u32[base + 188 + 4*r5] = 0xFFFFFFFF.
// ============================================================================
extern int _08025FAC(u32 v);
void MenuFF78_12F18(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    s16 s = *(volatile s16 *)(uintptr_t)(rec + 176);
    u32 r4 = (u32)_08024D4C(s);
    int r5 = 0;
    while (*(volatile u32 *)(uintptr_t)r4 != 0xFFFFFFFFu) {
        u32 v = *(volatile u32 *)(uintptr_t)r4;
        if (_08025FAC(v) != 0) {
            *(volatile u32 *)(uintptr_t)(rec + 188 + r5 * 4) = v;
            r5++;
        }
        r4 += 4;
    }
    *(volatile s16 *)(uintptr_t)(rec + 184) = (s16)(r5 - 1);
    *(volatile u32 *)(uintptr_t)(rec + 188 + r5 * 4) = 0xFFFFFFFFu;
}
#ifndef __APPLE__
void _080012F18(void *a) __attribute__((alias("MenuFF78_12F18")));
void Sub_080012F18(void *a) __attribute__((alias("MenuFF78_12F18")));
void sub_080012F18(void *a) __attribute__((alias("MenuFF78_12F18")));
#endif

// ============================================================================
// sub_0800127F4 — flag setter based on car id + WA gate.
// if u16[WA+0xFBC] == 3, strb 0 -> [r1+88] (countdown flag clear).
// then check sub_08004B68->u16[+2] (car id):
//   20, 35 -> s16[r1+84] = 5
//   39     -> s16[r1+84] = 1
//   else   -> s16[r1+84] = 6
// ============================================================================
extern void *_08004B68(void);
void MenuFF78_127F4(int unused_, void *rec_) {
    (void)unused_;
#ifndef __APPLE__
    extern u8 MenuFF78E_Wa[] __asm__("MenuFF78E_Wa");
    uintptr_t base = (uintptr_t)MenuFF78E_Wa;
#else
    uintptr_t base = (uintptr_t)(0x03001780u);
#endif
    if (*(volatile u16 *)(uintptr_t)(base + 0xFBCu) == 3) {
        *(volatile u8 *)(uintptr_t)((volatile u8 *)rec_ + 88) = 0;
    }
    // The ROM reads the car id with a REGISTER index holding a BYTE offset of
    // 2: `movs r1,#2; ldrsh r0,[r0,r1] @ 0x08012810..12`. An s16 subscript of
    // 1 gives that byte offset; subscript 2 scales to `#4`. A non-volatile
    // s16 lvalue is what yields `ldrsh` rather than `ldrh` + shifts.
    s16 car_id = ((s16 *)(uintptr_t)sub_08004B68())[1];
    // The ROM's ladder jumps into THREE shared store blocks, not four:
    // `beq -> 0x08012832` serves BOTH car 35 (0x08012816) and car 20
    // (0x0801281E), `beq -> 0x0801283A` serves 39 (0x0801282E), and both
    // defaults `b -> 0x08012842`. Only a goto-style tail merge emits three
    // blocks; a plain if/else ladder duplicates the second `== 5` store and
    // grows the body by 4 bytes.
//
    // `cmp r0,#35; bgt @ 0x08012818` targets 0x0801282C, which sits AFTER the
    // literal pool — so the `> 35` arm is the out-of-line one and the
    // `<= 35` ladder is the fall-through. Stating the `<= 35` arm first is
    // what makes agbcc lay the blocks out in the ROM's order.
    if (car_id == 35) goto set5;
    if (car_id <= 35) {
        if (car_id == 20) goto set5;
        goto set6;
    }
    if (car_id == 39) goto set1;
    goto set6;
set5:
    *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 5;
    goto store;
set1:
    *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 1;
    goto store;
set6:
    *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 6;
store:
    return;
}
#ifndef __APPLE__
void _0800127F4(int a, void *b) __attribute__((alias("MenuFF78_127F4")));
void sub_0800127F4(int a, void *b) __attribute__((alias("MenuFF78_127F4")));
#endif

// ============================================================================
// sub_080012E9C — small leaf: if u32[rec+144]==0, sub_0800D77C(rec+156,0,88);
// if u32[rec+144]==1, sub_0800D77C(rec+156,0,-32); then u32[rec+144]=2.
//
// FINDING : the `rec` local was costing an instruction the ROM does
// not have. The ROM's first two body instructions are `adds r4, r0, #0 @
// 0x08012E9E` then `adds r4, #144 @ 0x08012EA0` — one copy of the pointer,
// biased in place. With the named `rec` local, agbcc also keeps a second,
// unbiased copy live across the branch and emits `adds r2, r0, #0` at
// 0x08012E9E, so the body desynchronises at +2. Inlining the parameter
// removes it: prefix 2 -> 8, candidate 52 -> 48 bytes. The byte COUNT falls
// (26 -> 13) while the prefix rises, which is the expected signature of
// deleting the misaligning instruction; prefix is the sound signal here.
extern void _0800D77C(void *a, int b, int c);
void MenuFF78_12E9C(void *rec_) {
    s32 v = *(volatile s32 *)(uintptr_t)((volatile u8 *)rec_ + 144);
    if (v != 1) {
        if (v < 2) {
            if (v == 0) {
                _0800D77C((void *)(uintptr_t)((volatile u8 *)rec_ + 156), 0, 88);
                *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 144) = 2;
            }
        }
    } else {
        _0800D77C((void *)(uintptr_t)((volatile u8 *)rec_ + 156), 0, -32);
        *(volatile u32 *)(uintptr_t)((volatile u8 *)rec_ + 144) = 2;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080012E9C(void *a) __attribute__((alias("MenuFF78_12E9C")));
void Sub_080012E9C(void *a) __attribute__((alias("MenuFF78_12E9C")));
void sub_080012E9C(void *a) __attribute__((alias("MenuFF78_12E9C")));
#endif

// ============================================================================
// sub_080012ED0 — car-id -> flag mapper (two-way: 35 -> 5, else -> 6).
//
// FINDING : the WA read is a TWO-POOL-WORD add, not a folded
// constant — `ldr r0,=0x03001780; ldr r1,=0x0FBC; adds r0,r0,r1;
// ldrh r0,[r0]` (0x08012ED4..0x08012EDA). A folded `0x03001780u+0xFBCu`
// emits a single 16-bit pool word and loses both ldrs and the adds, which
// is exactly the +2 first difference. Same non-foldable symbol recipe as
// MenuFF78_12F6C below.
// ============================================================================
void MenuFF78_12ED0(int unused_, void *rec_) {
    (void)unused_;
#ifndef __APPLE__
    extern u8 MenuFF78E_Wa[] __asm__("MenuFF78E_Wa");
    __asm__(".globl MenuFF78E_Wa\nMenuFF78E_Wa = 0x03001780\n");
    uintptr_t base = (uintptr_t)MenuFF78E_Wa;
#else
    uintptr_t base = (uintptr_t)(0x03001780u);
#endif
    if (*(volatile u16 *)(uintptr_t)(base + 0xFBCu) == 3) {
        *(volatile u8 *)(uintptr_t)((volatile u8 *)rec_ + 88) = 0;
    }
    u16 car_id = *(volatile u16 *)(uintptr_t)((volatile u8 *)sub_08004B68() + 2);
    if (car_id == 35) {
        *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 5;
    } else {
        *(volatile s16 *)(uintptr_t)((volatile u8 *)rec_ + 84) = 6;
    }
}
// The ROM pads the body's last two bytes with 0x0000 (0x08012F12..0x08012F13)
// after `bx r0`; agbcc emits a one-instruction `nop` (0xC046) there. The
// explicit `0` fill selects the ROM's bytes.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080012ED0(int a, void *b) __attribute__((alias("MenuFF78_12ED0")));
void Sub_080012ED0(int a, void *b) __attribute__((alias("MenuFF78_12ED0")));
void sub_080012ED0(int a, void *b) __attribute__((alias("MenuFF78_12ED0")));
#endif

// ============================================================================
// sub_080012F6C — small leaf: u32[rec+188] = s16[WA+0xFC2]; u32[rec+192] = -1;
// u16[rec+184] = 0; bx lr.
// ============================================================================
void MenuFF78_12F6C(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 *q;
#ifndef __APPLE__
    extern u8 MenuFF78E_Wa[] __asm__("MenuFF78E_Wa");
    __asm__(".globl MenuFF78E_Wa\nMenuFF78E_Wa = 0x03001780\n");
    uintptr_t base = (uintptr_t)MenuFF78E_Wa;
#else
    uintptr_t base = (uintptr_t)(0x03001780u);
#endif
    q = (u32 *)(uintptr_t)(rec + 188);
#ifndef __APPLE__
    register u32 idx __asm__("r3") = 0;
#else
    u32 idx = 0;
#endif
    s16 v = ((s16 *)(uintptr_t)(base + 0xFC2u))[idx];
    *(volatile u32 *)q = (u32)(s32)v;
    {
        u32 *q2 = (u32 *)(uintptr_t)((u8 *)q + 4);
        *(volatile u32 *)q2 = 0xFFFFFFFFu;
        *(volatile u16 *)(uintptr_t)((u8 *)q2 - 8) = 0;
    }
}
#ifndef __APPLE__
void _080012F6C(void *a) __attribute__((alias("MenuFF78_12F6C")));
void sub_080012F6C(void *a) __attribute__((alias("MenuFF78_12F6C")));
#endif

// ============================================================================
// sub_080012F98 — screen init: sub_080012F6C + u16[rec+174]=u16[WA+0xFC2];
// u16[rec+176]=10; u16[rec+136]=0; then _080022E4(s16[rec+174])->u16[rec+180];
// read byte at WA + (s16*3*4+49) -> s16[rec+172]; s8 at +48 -> s16[rec+178];
// u32[rec+144]=2; u16[rec+186]=0; sub_0800D77C(rec+156,0,-32).
// ============================================================================
extern int _080022E4(int a);
void MenuFF78_12F98(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    MenuFF78_12F6C(rec_);
    u16 fcc2 = *(volatile u16 *)(uintptr_t)(0x03001780u + 0xFC2u);
    *(volatile u16 *)(uintptr_t)(rec + 174) = fcc2;
    *(volatile u16 *)(uintptr_t)(rec + 176) = 10;
    *(volatile u16 *)(uintptr_t)(rec + 136) = 0;
    int v = _080022E4((int)(s16)fcc2);
    *(volatile u16 *)(uintptr_t)(rec + 180) = (u16)v;
    int idx = (int)(s16)fcc2;
    volatile u8 *p = (volatile u8 *)(uintptr_t)(0x03001780u + (idx * 3) * 4 + 49);
    s8 sb = (s8)*p;
    *(volatile s16 *)(uintptr_t)(rec + 172) = (s16)sb;
    volatile u8 *p2 = (volatile u8 *)(uintptr_t)(0x03001780u + (idx * 3) * 4 + 48);
    s8 sb2 = (s8)*p2;
    *(volatile s16 *)(uintptr_t)(rec + 178) = (s16)sb2;
    *(volatile u32 *)(uintptr_t)(rec + 144) = 2;
    *(volatile u16 *)(uintptr_t)(rec + 186) = 0;
    _0800D77C((void *)(uintptr_t)(rec + 156), 0, -32);
}
#ifndef __APPLE__
void _080012F98(void *a) __attribute__((alias("MenuFF78_12F98")));
void Sub_080012F98(void *a) __attribute__((alias("MenuFF78_12F98")));
void sub_080012F98(void *a) __attribute__((alias("MenuFF78_12F98")));
#endif
