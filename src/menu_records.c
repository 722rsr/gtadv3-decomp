// ============================================================================
// menu_records.c — C lift of menu record-handler clusters
//   asm/menu_ce78.s  (VMA 0x0800CE78–0x0800CFE4, 4 funcs)
//   asm/menu_d9a4.s  (VMA 0x0800D9A4–0x0800DAB8, 4 funcs)
//
// Every function is transcribed instruction-for-instruction from the cited
// asm listings (labels = bare VMAs; all control flow, widths and call ABI
// preserved). This file contains NO speculative behavior beyond the asm.
//
// Menu-record layout (work-area record, from asm field offsets):
//   +0      u32 sprite-object handle (D9A4 family) / u16 state (CE78)
//   +6      s16 cursor slot (clamped 0..2 by the CE78 key handler)
//   +0x1018 u32 sprite-object handle (CE78 family; 4-byte aligned offset)
//
// The sprite/text-render object API lives in asm/code_263e0.s (not yet
// reconstructed in C); its entry points are reached via the established
// `extern NAME(...); // 0x0800XXXX` decl-comment hints, which resolve to the
// real asm bodies. Extern decls that match a strong C-defined symbol
// (e.g. _080022E4 in ai_catalog.c) bind to it directly:
//   _08026948  obj allocate + default state
//   _080269AC  obj allocate (0x1400-byte pool variant)
//   _08026A4C  (obj, slot, attr) — set attr slot
//   _08026A58  (obj, 1) — store 1 -> [obj+4]
//   _08026A60  (obj, 0) — store 0 -> [obj+6]
//   _08026A20  (obj, x, y) — set center position
//   _08026938  (obj, slot) — request redraw (walks [r0+8] -> [+92] -> 262A4)
//   _080022E4  car id -> index map (table 0x080C43EC, -1 sentinel)
//   _080038A4  frame draw (template, x, y)
//   _08004B68  block-B manager pointer (result discarded by callers)
//   _08004CA8  manager state read (r0 = which)
//   _08004EA8  manager store+step (r0 = arg)
// ============================================================================

#include "gtadv/menus.h"
#include "gba/types.h"

typedef struct { u8 crh_kp_pad[12]; u16 crh_kp_w12; } CRH_Keypad;
#define WA_CURSOR_CELL (*(volatile u16 *)((uintptr_t)0x03001780 + 0x574))
#define KEYPAD_W12     (((volatile CRH_Keypad *)(uintptr_t)0x030035C0)->crh_kp_w12)

#ifdef __APPLE__
__attribute__((weak)) void *_08026948(int a) { (void)a; return (void *)0; }
__attribute__((weak)) void *_080269AC(int a) { (void)a; return (void *)0; }
__attribute__((weak)) void  _08026A4C(void *obj, int slot, int attr) { (void)obj; (void)slot; (void)attr; }
__attribute__((weak)) void  _08026A58(void *obj, int v) { (void)obj; (void)v; }
__attribute__((weak)) void  _08026A60(void *obj, int v) { (void)obj; (void)v; }
__attribute__((weak)) void  _080026A20(void *obj, int x, int y) { (void)obj; (void)x; (void)y; }
__attribute__((weak)) void  _08026938(void *obj, int slot) { (void)obj; (void)slot; }
__attribute__((weak)) int   _080022E4(int id) { return id >= 0 && id <= 97 ? 23 : -1; }
__attribute__((weak)) void  _080038A4(int a, int b, void *c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void  _08003978(int a, u32 b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void *_08004B68(void) { return (void *)0; }
__attribute__((weak)) int   _08004CA8(int v) { (void)v; return 0; }
__attribute__((weak)) void  _08004EA8(int v) { (void)v; }
__attribute__((weak)) void  _0800CE70(void *rec) { (void)rec; }
#else
extern void *_08026948(int a);                          // 0x08026948
extern void *_080269AC(int a);                          // 0x080269AC
extern void  _08026A4C(void *obj, int slot, int attr);  // 0x08026A4C
extern void  _08026A58(void *obj, int v);               // 0x08026A58
extern void  _08026A60(void *obj, int v);               // 0x08026A60
extern void  _080026A20(void *obj, int x, int y);        // 0x08026A20 (asm/code_263e0.s)
extern void  _08026938(void *obj, int slot);            // 0x08026938
extern int   _080022E4(int id);                         // 0x080022E4
extern void  _080038A4(int a, int b, void *c);          // 0x080038A4
extern void  _08003978(int a, u32 b, int c);            // 0x08003978 (runtime_hud.c)
extern void *_08004B68(void);                           // 0x08004B68
extern int   _08004CA8(int v);                          // 0x08004CA8
extern void  _08004EA8(int v);                          // 0x08004EA8
#endif

// ============================================================================
// menu_ce78.s (VMA 0x0800CE78–0x0800CFE4)
// ============================================================================

// ----------------------------------------------------------------------------
// sub_0800CE78 — record open.
//   [rec+0](u16) = _08004CA8(0)
//   [rec+6](u16) = *(u16*)(0x03001780+0x574)      (garage cursor s16)
//   obj = _08026948; [rec+0x1018](u32) = obj
//   _08026A4C(obj, s16[rec+6], 0); _08026A58(obj, 1); _08026A60(obj, 0)
//   _08026938(obj, s16[rec+6])
struct MenuRec_CE78_Rec { u16 f0; u16 f1; u16 f2; u16 cursor; u8 pad[0x1018 - 8]; void *obj; };
struct MenuRec_CE78_WA  { u8 pad[0x574]; u16 cursor; };
extern void *_080026948(void);          // 0x08026948, closure spelling
extern void  _080026938(void *obj, int slot);
extern void *_0800269AC(int a);        // 0x080269AC, closure spelling (takes the pool size)
extern u8 MenuRecWA[];
// 0x08026938 is a PROMOTED entry whose manifest export is exactly
// `['_080026938','sub_080026938']`. The ARM build must therefore call
// `_080026938`: the `_08026938` spelling is a second `.thumb_set` hop whose
// intermediate the spliced section drops, leaving the reference undefined at
// link even with every other gate green.
#define MENUREC_REDRAW _080026938

void MenuRec_0800CE78(void *rec) {
    struct MenuRec_CE78_Rec *m = (struct MenuRec_CE78_Rec *)rec;

    _08004B68();
    m->f0 = (u16)_08004CA8(0);
    __asm__("MenuRecWA = 0x03001780");
    m->cursor = ((volatile struct MenuRec_CE78_WA *)(uintptr_t)MenuRecWA)->cursor;
    m->obj = _080026948();
    _08026A4C(m->obj, (int)(s16)m->cursor, 0);
    _08026A58(m->obj, 1);
    _08026A60(m->obj, 0);
    _080026938(m->obj, (int)(s16)m->cursor);
}
#ifndef __APPLE__
void _0800CE78(void *a) __attribute__((alias("MenuRec_0800CE78")));
void sub_0800CE78(void *a) __attribute__((alias("MenuRec_0800CE78")));
#endif

// ----------------------------------------------------------------------------
// sub_0800CED4 — key input on the cursor slot.
//   r4 = r6 = u16(arg2 keys); r8 = s16[rec+6] (saved cursor); r7 = 0;
//   r9 = 2.
//   keys&2  -> _08004EA8(6)
//   keys&32 -> r7 -= 1; keys&16 -> r7 += 1
//   cur = (u16)(u16[rec+6] + r7); store u16[rec+6] = cur;
//   if ((s16)cur < 0) u16[rec+6] = 0;
//   if (s16[rec+6] > 2) u16[rec+6] = r9 (2);
//   cur = s16[rec+6]; if (cur != r8):
//     obj = *(u32*)(rec+0x1018); _08026A4C(obj, cur, 0); _08026938(obj, cur)
void MenuRec_0800CED4(void *rec, int a1, int a2) {
    (void)a1;
    volatile u8 *r5 = (volatile u8 *)rec;
    u16 r4 = (u16)a2;
    u16 r6 = r4;
    s16 r7 = 0;
    const s16 r9 = 2;

    _08004B68();
    s16 r8 = *(volatile s16 *)(r5 + 6);

    if (r4 & 2)
        _08004EA8(6);
    if (r4 & 32)
        r7 = (s16)(r7 - 1);
    if (r6 & 16)
        r7 = (s16)(r7 + 1);

    u16 sum = (u16)(*(volatile u16 *)(r5 + 6) + (u16)r7);
    *(volatile u16 *)(r5 + 6) = sum;
    if ((s16)sum < 0)
        *(volatile u16 *)(r5 + 6) = (u16)0;

    if (*(volatile s16 *)(r5 + 6) > 2)
        *(volatile u16 *)(r5 + 6) = (u16)r9;

    s16 cur = *(volatile s16 *)(r5 + 6);
    if (cur != r8) {
        void *obj = (void *)(uintptr_t)*(volatile u32 *)(r5 + 0x1018);
        _08026A4C(obj, cur, 0);
        _08026938(obj, cur);
    }
}
#ifndef __APPLE__
void _0800CED4(void *a, int b, int c) __attribute__((alias("MenuRec_0800CED4")));
void sub_0800CED4(void *a, int b, int c) __attribute__((alias("MenuRec_0800CED4")));
#endif

// ----------------------------------------------------------------------------
// sub_0800CF60 — paint/measure lane.
//   _080038A4(96, 15, 0x0805F894)
//   r2 = ((int)(u16[0x030035C0+12] << 16)) >> 19    (arithmetic >>3)
//   _08003978(120, 32, r2)
//   _08026A20(*(u32*)(rec+0x1018), 120, 100)
//
// : exact at 64/64. This body was on the "unreachable"
// list in docs/matching_workflow.md ("the destination dies at the consuming
// instruction, so no source shape reaches it; 345 generated variants produced
// zero hits"). That conclusion was correct about SOURCE SHAPES and wrong as a
// statement about reachability: it was drawn before `register T v
// __asm__("rN")` existed as a lever, and no source shape can pin a hard
// register. The register pin reaches it, and the mechanism it exploits is
// worth naming -- the shift must be a TWO-STATEMENT chain through a `u32`,
// so that agbcc's combine_reloads has an intervening statement to break on
// and cannot fold `ldrh` into the same register as its consumer.
//
//   ROM:  ldrh r0,[r0,#12] / lsls r2,r0,#16 / asrs r2,r2,#19
//   was:  ldrh r2,[r0,#12] / lsls r2,r2,#16 / asrs r2,r2,#19
//
// Controls, each measured with a fresh probe work dir:
//   * drop the `v` -> r0 pin            -> 62/64 prefix 16 (load folds back)
//   * drop the `t` -> r2 pin            -> 33/64 prefix 19
//   * drop both pins                    -> 62/64 prefix 16
//   * pin `t` to r3 instead of r2       -> 38/64 prefix 18
//   * one-statement shift, `t` pinned    -> 62/64 prefix 16
// Every control fails, so both pins and the split are each load-bearing.
// ----------------------------------------------------------------------------
void MenuRec_0800CF60(void *rec) {
    volatile u8 *r4 = (volatile u8 *)rec;
    _080038A4(96, 15, (void *)(uintptr_t)0x0805F894);
    register u16 v __asm__("r0") = KEYPAD_W12;
    register u32 t __asm__("r2");
    t = (u32)v << 16;
    t = (u32)((s32)t >> 19);
    _08003978(120, 32, (int)t);
    void *obj = (void *)(uintptr_t)(*(volatile u32 *)(r4 + 0x1018));
    _080026A20(obj, 120, 100);
}
#ifndef __APPLE__
void _0800CF60(void *a) __attribute__((alias("MenuRec_0800CF60")));
void sub_0800CF60(void *a) __attribute__((alias("MenuRec_0800CF60")));
#endif

// ----------------------------------------------------------------------------
// sub_0800CFA0 — event dispatcher (r0 = ev, r3 = record):
//   ev 1 -> _0800CE78(r3)
//   ev 2 -> _0800CE70(r3)
//   ev 6 -> _0800CED4(r3, u16(r1), u16(r2))
//   ev 7 -> _0800CF60(r3)
//   default: return (ev 0, 3, 4, 5, >7)
#ifndef __APPLE__
extern void _0800CE70(void *rec);   // 0x0800CE70 (menus.c MenuCE70_0800CE70)
// 0x0800CED4 is NOT a promoted span, so the assembled closure keeps its asm
// labels: the call must use `_0800CED4`/`sub_0800CED4`, not the C name.
extern void _0800CED4(void *rec, int a1, int a2);
#endif
// The span is byte-identical to 0x0800CE2C (src/foundation_late.c,
// LateDispatch_CE2C) apart from the four `bl` immediates, so the promoted
// recipe's shape is used wholesale: `switch ((unsigned)ev)` gives the
// cmp #2 / beq / cmp #2 / bhi chain and the case order 2,7,6,1 lays the
// out-of-line arms at +0x1a/+0x22/+0x2a/+0x3a exactly as the ROM has them.
void MenuRec_0800CFA0(int ev, int a1, int a2, void *rec) {
    switch ((unsigned)ev) {
        case 2: { _0800CE70(rec); break; }
        case 7: { MenuRec_0800CF60(rec); break; }
        case 6: {
#ifdef __APPLE__
            MenuRec_0800CED4(rec, (int)(u16)a1, (int)(u16)a2);
#else
            _0800CED4(rec, (int)(u16)a1, (int)(u16)a2);
#endif
            break;
        }
        case 1: { MenuRec_0800CE78(rec); break; }
        default: break;
    }
}
#ifndef __APPLE__
void _0800CFA0(int a, int b, int c, void *d) __attribute__((alias("MenuRec_0800CFA0")));
void sub_0800CFA0(int a, int b, int c, void *d) __attribute__((alias("MenuRec_0800CFA0")));
#endif

// ============================================================================
// menu_d9a4.s (VMA 0x0800D9A4–0x0800DAB8)
// ============================================================================

// shared id -> index map: _080022E4(s16 id), -1 sentinel -> 23
//
// FINDING : this CANNOT stay a `static` helper. agbcc does not inline
// it, so every call site emits a real `bl MenuRec_D9_IdMap` and the probe
// reports UNRESOLVED_RELOCATION. The ROM has the four-instruction sequence
// (`lsls/lsrs` narrow, `bl 0x080022E4`, `cmp r4,#-1`, `movs r4,#23`) inline
// in each of D9A4/D9F8/DA50, and each body needs its OWN `s16` temporary, so
// the sequence is spelled out at every use site.

// ----------------------------------------------------------------------------
// sub_0800D9A4 — obj alloc + attrs by car id.
//   obj = _08026948; [rec+0](u32) = obj
//   idx = idmap(s16 a1); _08026A4C(obj, idx, s16 a2)
//   _08026A58(obj, 1); _08026A60(obj, 0); _08026938(obj, idx)
void MenuRec_0800D9A4(void *rec, int a1, int a2) {
    // FINDING : the ROM narrows BOTH id parameters up front —
    // `lsls r4,#16; lsrs r4,#16` for a1 (0x0800D9AA) and
    // `lsls r2,#16; lsrs r6,r2,#16` for a2 (0x0800D9AE) — before the first
    // call, then RE-narrows the already-narrowed values at their use sites
    // (`lsls r4,#16; asrs r4,#16` @ 0x0800D9B8, the same for r6 @ 0x0800D9D0).
    // So each needs a `u16` local assigned before the call and re-widened
    // where it is consumed. The handle is also RELOADED from `[rec+0]` before
    // each call (`ldr r0,[r5,#0]` x4) rather than held in a register.
    volatile u8 *r5 = (volatile u8 *)rec;
    u16 id = (u16)a1;
    u16 attr = (u16)a2;
#ifndef __APPLE__
    *(volatile u32 *)(r5 + 0) = (u32)(uintptr_t)_080026948();
#else
    *(volatile u32 *)(r5 + 0) = (u32)(uintptr_t)_08026948(0);
#endif
    int r4 = (int)(s16)(u16)id;
    r4 = _080022E4(r4);
    if (r4 == -1) {
        r4 = 23;
    }
    _08026A4C(*(void **)(r5 + 0), r4, (int)(s16)(u16)attr);
    _08026A58(*(void **)(r5 + 0), 1);
    _08026A60(*(void **)(r5 + 0), 0);
    // `_080026938`, not `_08026938`: 0x08026938 is a PROMOTED entry whose
    // manifest export is exactly `['_080026938','sub_080026938']`. The
    // `_08026938` spelling is a second `.thumb_set` hop whose intermediate
    // the spliced section drops, so the reference stays undefined at link.
    MENUREC_REDRAW(*(void **)(r5 + 0), r4);
}
#ifndef __APPLE__
void _0800D9A4(void *a, int b, int c) __attribute__((alias("MenuRec_0800D9A4")));
void Sub_0800D9A4(void *a, int b, int c) __attribute__((alias("MenuRec_0800D9A4")));
void sub_0800D9A4(void *a, int b, int c) __attribute__((alias("MenuRec_0800D9A4")));
#endif

// ----------------------------------------------------------------------------
// sub_0800D9F8 — same as D9A4 but allocates via the 0x1400-byte pool variant
// (_080269AC(160 << 5) = _080269AC(5120)).
void MenuRec_0800D9F8(void *rec, int a1, int a2) {
    // FINDING : same shape as MenuRec_0800D9A4 — both id parameters
    // narrowed up front into callee-saved registers and re-narrowed at their
    // use sites, the handle reloaded from `[rec+0]` before each call, and
    // the allocator invoked with NO argument
    // (`movs r0,#160; lsls r0,#5; bl` @ 0x0800DA06 sets r0 itself).
    volatile u8 *r5 = (volatile u8 *)rec;
    u16 id = (u16)a1;
    u16 attr = (u16)a2;
    // 160 << 5 == 5120. ARM spells the callee `_0800269AC` (the closure
    // spelling at asm/code_263e0.s): 0x080269ac's only asm label is the
    // 9-digit twin, so the 8-digit alias is a second `.thumb_set` hop the
    // splice drops.
    *(volatile u32 *)(r5 + 0) = (u32)(uintptr_t)_0800269AC(5120);
    int r4 = (int)(s16)(u16)id;
    r4 = _080022E4(r4);
    if (r4 == -1) {
        r4 = 23;
    }
    _08026A4C(*(void **)(r5 + 0), r4, (int)(s16)(u16)attr);
    _08026A58(*(void **)(r5 + 0), 1);
    _08026A60(*(void **)(r5 + 0), 0);
    MENUREC_REDRAW(*(void **)(r5 + 0), r4);
}
#ifndef __APPLE__
void _0800D9F8(void *a, int b, int c) __attribute__((alias("MenuRec_0800D9F8")));
void sub_0800D9F8(void *a, int b, int c) __attribute__((alias("MenuRec_0800D9F8")));
#endif

// ----------------------------------------------------------------------------
// sub_0800DA50 — set attr on the existing obj handle at [rec+0] (no enable).
//   idx = idmap(s16 a1); _08026A4C([rec+0], idx, s16 a2); _08026938([rec+0], idx)
void MenuRec_0800DA50(void *rec, int a1, int a2) {
    // FINDING : here `a1` is narrowed to `s16` immediately and stays
    // in r0 (`adds r0,r1,#0; lsls r0,#16; asrs r0,#16` @ 0x0800DA54), while
    // `a2` is narrowed to `u16` into r5 (`lsls r2,#16; lsrs r5,r2,#16` @
    // 0x0800DA56) and re-widened at its use (`lsls r2,r5,#16; asrs` @
    // 0x0800DA70). The handle is reloaded from `[rec+0]` before each call.
    volatile u8 *r6 = (volatile u8 *)rec;
    u16 attr = (u16)a2;
    int r4 = _080022E4((int)(s16)(u16)a1);
    if (r4 == -1) {
        r4 = 23;
    }
    _08026A4C(*(void **)(r6 + 0), r4, (int)(s16)(u16)attr);
    MENUREC_REDRAW(*(void **)(r6 + 0), r4);
}
#ifndef __APPLE__
void _0800DA50(void *a, int b, int c) __attribute__((alias("MenuRec_0800DA50")));
void Sub_0800DA50(void *a, int b, int c) __attribute__((alias("MenuRec_0800DA50")));
void sub_0800DA50(void *a, int b, int c) __attribute__((alias("MenuRec_0800DA50")));
#endif

// ----------------------------------------------------------------------------
// sub_0800DA88 — attr-only setter; the record pointer itself is the obj arg
// (no [rec+0] deref, no enable calls, no redraw).
void MenuRec_0800DA88(void *rec, int a1, int a2) {
    u16 t = (u16)a2;
    int r4 = _080022E4((int)(s16)(u16)a1);
    if (r4 == -1)
        r4 = 23;
    _08026A4C(rec, r4, (int)(s16)t);
}
// The ROM span at 0x0800DA88 is 48 bytes and its last instruction is the 2-byte
// `00 47` (`bx r0`) at +0x2C. The trailing two bytes are `00 00` (verified from
// baserom.gba), not code, and this body compiles to 46 bytes. Under
// -ffunction-sections gas closes a Thumb *code* section with the 2-byte `nop`
// filler (0x46c0), so this scored 46/48 with every instruction already
// byte-correct. A file-scope `.align 2, 0` is emitted after the body's `.size`,
// i.e. still inside the body's own section, and pads with the explicit `0`
// fill instead.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800DA88(void *a, int b, int c) __attribute__((alias("MenuRec_0800DA88")));
void sub_0800DA88(void *a, int b, int c) __attribute__((alias("MenuRec_0800DA88")));
#endif
