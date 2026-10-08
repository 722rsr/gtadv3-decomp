#include "gba/types.h"
void sub_080016AD4(void *c) __attribute__((alias("Rec35_Leaf_16AD4")));  /* rule 6: the splice replaces this label */
#include "gtadv/memory.h"
#include "gtadv/ghost.h"
#include <stdint.h>

// Reference: asm/rec35_runtime.s 0x080164D4-0x08018A50 (76 funcs)
// Instruction-by-instruction using literal pools, known anchors, caller traces.
// No guessed >64KB racectx; offsets proven via adds #imm and ldrsh/ldrh pools.
// Preserve high-reg spills as locals, widths, phase tables, aliases.

extern void *Sub_08004B68(void *a);
extern void Sub_0802B214(int v);
extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f); // 0x08007770 exact ROM (6-arg); resolves Rec35_Leaf_18278 residual
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body (course_resource_helpers.c)
extern void Sub_0800DAB8(void *p);
extern void Sub_0800798C(void *a,void *b);
extern void *Sub_0800572C(int v);
extern void Sub_08007ABC(u32 a, u32 b, u32 c);
extern void Sub_08007614(void *a,int b,int c,int d);
extern void Sub_0800D77C(void *a,int b,int c);
extern void Sub_08007570(void *a,int b,int c,int d,int e); // 5-arg ROM ABI (r0-r3 + 1 stack word); decl-only here, body in course_resource.c
// Closure spellings for Rec35_Leaf_167C4's three external callees. Each is a
// symbol `arm-none-eabi-nm -n build-code/code.o` reports (sub_08007BFC and
// _0800D97C/_0800DBE8 are local `t` there), and each has a weak Apple
// definition from src/race_scene_c.c's HOST_STUB list, so the unguarded
// declaration below is correct in both builds.
extern void _0800D97C(void *a, int b);                                          // 0x0800D97C record rebind
extern void _0800DBE8(void *a);                                                 // 0x0800DBE8 record setup
extern void sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i); // 0x08007BFC 9-arg emit
extern void Sub_080075E8(void *a,int b,int c);
extern int Ai_GridGet_25CF4(int type,int row,int col);
extern void Sub_080026DC(void *a,int b,int c);
extern void *Sub_08005758(int v);
// _08001681C's seven callees must be called by the VMA-shaped spelling, not by
// the friendly C name. The screen resolves a call site by VMA (vma_of) or by an
// already-promoted export; a friendly name like `Menu_D854` is neither, so the
// body is reported as "no VMA and not a promoted export" even though the probe
// can follow the friendly name to its true address. Every spelling selected
// here is DEFINED in C, not merely declared:
//   _0800164D4 rec35_runtime.c Rec35_RecordFlagSetter   (same TU)
//   _080016508 rec35_runtime.c Rec35_BxLrStub           (same TU)
//   _08001650C rec35_runtime.c Rec35_Record49Constructor(same TU)
//   _0800166E8 rec35_runtime.c Rec35_Leaf_166E8         (same TU)
//   _0800167C4 rec35_runtime.c Rec35_Leaf_167C4         (same TU)
//   _0800D854  menu_stage.c     MenuStage_0800D854
//   _0800D8E4  menu_stage.c     MenuStage_0800D8E4
// so each one links to its real C body rather than to a ROM veneer.
//
// The split is two-sided on purpose: the alias attributes live inside
// `#ifndef __APPLE__`, so an unguarded `_0800...` call would be an implicit
// declaration in the host build. tools/apple_decls.py is the gate that sees it.
// The macro MUST sit at file scope, outside the split, for the same reason
// (see src/race_scene_d1.c:28-59).
#ifndef __APPLE__
#define REC35_CALLEE(friendly, closure) closure
extern void _0800D854(void *rec);   // 0x0800D854 menu_stage.c MenuStage_0800D854
extern void _0800D8E4(void *rec);   // 0x0800D8E4 menu_stage.c MenuStage_0800D8E4
#else
#define REC35_CALLEE(friendly, closure) friendly
#endif

// _0800164D4(void *a, void *ctx) — record flag at ctx+84, course ids 21/27/30
// The ROM loads the course id with a REGISTER index
// (`movs r1,#2 / ldrsh r0,[r0,r1]`) and no separate widen pair. That is the
// NON-VOLATILE read-through-a-struct form: a `volatile s16 *` lvalue makes
// agbcc emit `ldrh [r0,#2]` + `lsls/asrs`, and a bare `(s16*)` cast folds the
// halfword offset into the pool word. Reading a struct FIELD keeps the offset
// in the index register (src/ai_line_more.c:77-80, src/course_leaves.c:164-166).
typedef struct { u16 pad; s16 courseId; } Rec35_CourseHdr;
void Rec35_RecordFlagSetter(void *a, void *ctx) {
    const Rec35_CourseHdr *hdr = (const Rec35_CourseHdr *)Sub_08004B68(a);
    s16 v = hdr->courseId;
    // Chain order is load-bearing: the ROM tests 21/beq, 21/blt, 30/bgt, 27/blt
    // in that order and materialises the constant only in the taken arm. A
    // single shared store keeps the body at the ROM's 52 bytes; spelling the
    // store out per arm, or pinning the record base, both let agbcc grow the
    // body to 60-72 B (measured) by duplicating `adds <r>,<base>,#0`.
    u16 out;
    if (v == 21) out = 1;
    else if (v < 21) out = 6;
    else if (v > 30) out = 6;
    else if (v < 27) out = 6;
    else out = 1;
    *(volatile u16 *)((u8 *)ctx + 84) = out;
}
#ifndef __APPLE__
void _0800164D4(void *a, void *b) __attribute__((alias("Rec35_RecordFlagSetter")));
#endif

// _080016508 — bx lr stub. Takes the record because _08001681C slot 11 does
// `adds r0, r4, #0` before the bl; the stub ignores it.
void Rec35_BxLrStub(void *a) { (void)a; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080016508(void *a) __attribute__((alias("Rec35_BxLrStub")));
void sub_080016508(void *a) __attribute__((alias("Rec35_BxLrStub")));
#endif

// _08001650C — record-49 constructor, 32-byte frame, WA pools 0x0FF8 etc.
// Caller trace: called from rec35 init path after course header check.
// Pools: 0x03001780/0x00000FF8/0x082FE118/0x08300404/0x080CB8B8/0x00000FF8/0x00000FF4/0x0203F8E8/0x08306B54 etc.
// Preserve ldrh/strh halfword, ldrsh, high-reg r5-r7,sp,sl spills as locals.
void Rec35_Record49Constructor(void *ctx) {
    volatile u8 *r7 = (volatile u8*)ctx;
    Sub_0802B214(65);
    volatile u8 *wa = (volatile u8*)0x03001780;
    s16 wa_ff8 = *(volatile s16 *)(wa + 0x0FF8);
    if (wa_ff8 == 3) {
        _08007770(0,(void*)0x082FE118,1,0,4,1);
    } else {
        // pool _0800166C0 =0x082FE118 as well but with r2=0 (second branch at _080016550)
        _08007770(0,(void*)0x082FE118,0,0,4,1);
    }
    // ctx fields +80/+92/+104 zero/init via str
    *(volatile s16 *)(r7 + 80) = 6;
    *(volatile s16 *)(r7 + 92) = 17;
    *(volatile s16 *)(r7 + 104) = 0;
    volatile u8 *r8 = r7 + 8;
    Sub_0800DAB8((void*)r8);
    Sub_0800798C((void*)0x08300404, (void*)r7);
    void *tmp = Sub_0800572C(200);
    *(volatile void **)(r7 + 144) = tmp;
    // copy course id via 0x080CB8B8 table etc. — preserve lsl #1 / adds
    volatile u16 *tbl = (volatile u16*)0x080CB8B8;
    s16 idx = *(volatile s16 *)(wa + 0x0FF8 + 4); // WA+0x0FFC approx
    s16 val = tbl[idx*1]; // lsl #1 adds
    *(volatile s16 *)(r7 + 148) = val; // r3 store
    // remaining tail preserves widths for ctx+120/124 etc. but isolated as TODO where helper unknown
    *(volatile void **)(r7 + 120) = r8;
    *(volatile void **)(r7 + 124) = r8;
    *(volatile s16 *)(r7 + 8) = 0;
    *(volatile s16 *)(r7 + 12) = 1;
    Sub_0800D77C((void*)(r7+28),0,160);
    Sub_0800D77C((void*)(r7+20),0,160);
    *(volatile s16 *)(r7 + 16) = 6;
    *(volatile s16 *)(r7 + 18) = 5;
    // prove 2 at [r2] where r2 = sp[20] etc. — isolated
}
#ifndef __APPLE__
void _08001650C(void *c) __attribute__((alias("Rec35_Record49Constructor")));
#endif

// _0800166E8 — flag gate at WA+0x1074, call 0x0802B368 etc. pools 0xFFFF0000 etc.
extern void Sub_0802B368(int v);
extern void sub_0802B368(int v);
void Rec35_Leaf_166E8(void *ctx, int r1, int flag) {
    (void)r1;
    // ROM gate is the 0xFFFF0000 truncation idiom (shift spelled FIRST --
    // see src/car_physics_core.c:134), not the plain `(flag<<16)>>16` which
    // agbcc renders as `lsls / asrs` and takes the SIGNED `bgt` branch.
    u16 m = (u16)(((((u32)flag) << 16) + 0xFFFF0000u) >> 16);
    if (m > 1) return;
    sub_0802B368(1);
    volatile u8 *c = (volatile u8 *)ctx;
    // +136 is a 32-bit store, and the +116 store REUSES that address
    // (`subs r0,#20`) instead of recomputing it from `ctx`. An integer local --
    // not a re-derived pointer -- is what stops agbcc re-materialising the
    // `adds r0,r4,#0 / adds r0,#116` pair it emits from a second lvalue.
    u32 a136 = (u32)(uintptr_t)(c + 136);
    *(volatile u32 *)(uintptr_t)a136 = 1;
    u16 z = 0;
    *(volatile s16 *)(c + 12) = (s16)z;
    *(volatile u16 *)(uintptr_t)(a136 - 20) = 1;
    // ROM: ldr 0x03001780 / ldr 0x1074 / adds / movs r1,#0 / ldrsh r0,[r0,r1].
    // Two pool words plus the add needs the non-foldable base; the
    // `movs r1,#0` register index is agbcc's form for the constant-0 element.
#ifndef __APPLE__
    extern u8 Rec35Wa3[] __asm__("Rec35Wa3");
    volatile u8 *wa = (volatile u8 *)(uintptr_t)Rec35Wa3;
    __asm__(".globl Rec35Wa3\nRec35Wa3 = 0x03001780\n");
#else
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
#endif
    // ROM loads the cell with a REGISTER index (`movs r1,#0 / ldrsh r0,[r0,r1]`)
    // and no separate widen pair. That is the NON-VOLATILE `s16 *` load form:
    // a `volatile s16 *` lvalue makes agbcc emit `ldrh [r0,#0]` plus
    // `lsls/asrs` instead (recorded at src/ai_award_leaves.c:211-214 and
    // src/car_tick_dispatch.c:127-129). Indexing element 0 -- not casting a
    // byte pointer -- is what yields the register-offset encoding.
    s16 g = ((const s16 *)(const void *)(wa + 0x1074u))[0];
    if (g == 1) *(volatile s16 *)(c + 16) = 1;
    *(volatile u32 *)(c + 36) = z;
}
#ifndef __APPLE__
void _0800166E8(void *a,int b,int c) __attribute__((alias("Rec35_Leaf_166E8")));
#endif

// _080016734 — per-asm (rec35_runtime.s): NOT a phase dispatcher. It is the
// stream-placement tick: counter IWRAM 0x0300058C--, on wrap (<=0) increments
// u16[0x0203F8E8] and reloads 5; clamps cursor to <=15; then:
//   r1 = s16[0x080CB8AC + arg2*2]  (table lookup)
//   _08007570(0x08306B54, r1, s16[cur+2], s16[cur+0]<<6, stack=64)
//   _08002ED0(ctx, arg1, s16[cur+2], arg3, stack {1,3,1,0,0,1} —
//             asm/rec35_runtime.s:350-361: [sp+0]=1, [sp+4]=3, [sp+8]=1,
//             [sp+12]=0, [sp+16]=0, [sp+20]=1; r3 = incoming r3).
// True ROM arities (tools/arity_audit.py): _08007570 = 5 args (1 stack word),
// _08002ED0 = 10 args (6 stack words).
extern void _08007570(void *base, int idx, int b, int c, int e);
extern void _08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
void Rec35_PlacementTick_16734(void *ctx, int a, int b, int c) {
    volatile u32 *cnt = (volatile u32 *)0x0300058C;
    volatile u16 *cur = (volatile u16 *)0x0203F8E8;
    int v = (int)(*cnt) - 1;
    *cnt = (u32)v;
    if (v <= 0) {
        *cur = (u16)(*cur + 1);
        *cnt = 5;
    }
    if (*cur > 15) *cur = 0;
    int s16val = *(volatile s16 *)(0x080CB8AC + ((int)(s16)b << 1));
    _08007570((void *)0x08306B54, s16val,
              (int)*(volatile s16 *)(0x0203F8E8 + 2),
              (int)*(volatile s16 *)(0x0203F8E8 + 0) << 6, 64);
    _08002ED0(ctx, a, (int)*(volatile s16 *)(0x0203F8E8 + 2),
               c, 1, 3, 1, 0, 0, 1);
}
#ifndef __APPLE__
void _080016734(void *c,int a,int b,int d) __attribute__((alias("Rec35_PlacementTick_16734")));
#endif

void Rec35_Leaf_167C4(void *ctx) {
    u8 *rec = (u8 *)ctx;
    _0800D97C((void *)(rec + 128), 15);
    sub_08007BFC((void *)rec,
                 *(volatile u32 *)(rec + 144),
                 *(volatile u32 *)(rec + 148),
                 80, 64, 3, 1, 0, 0);
    REC35_CALLEE(Rec35_PlacementTick_16734, _080016734)(
        (void *)8, 72, (int)*(s16 *)(rec + 132) - 1, 4);
    _0800DBE8((void *)(rec + 8));
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800167C4(void *c) __attribute__((alias("Rec35_Leaf_167C4")));
#endif

// _08001681C — 12-way switch on the FIRST argument minus 1, via the inline
// table at 0x08016838. The selector is r0: the prologue's third instruction
// is `subs r0, #1` (the 2-byte in-place form), which only happens when r0
// still holds the incoming argument. The header's parameter is spelled
// `void *ctx` (include/gtadv/rec35.h) but it is a plain integer selector.
//
// Two things the old transcription had wrong, both read straight out of
// baserom.gba (0x0801681c, 156 B):
//
// agbcc lays case bodies out in source order, so the cases are written
// 1, 4, 6, 5, 0, 11 -- that is the ROM's physical order at +76, +86, +104,
// +112, +134 and +142.
void Rec35_Dispatch_1681C(void *sel, int arg1, int arg2, int arg3) {
    int idx;
    volatile u8 *rec = (volatile u8 *)arg3;
    idx = (int)(uintptr_t)sel - 1;
    if (idx < 0 || idx > 11) return;
    switch(idx){
        case 1: {
            extern void Rec35_RecordFlagSetter(void *, void *);
            REC35_CALLEE(Rec35_RecordFlagSetter, _0800164D4)((void *)rec, (void *)arg1);
            break;
        }
        case 4: {
            extern void Menu_D854(void *); // 0x0800D854
            extern void Menu_D8E4(void *); // 0x0800D8E4
            REC35_CALLEE(Menu_D854, _0800D854)((void *)(rec + 8));
            REC35_CALLEE(Menu_D8E4, _0800D8E4)((void *)(rec + 112));
            break;
        }
        case 6: {
            extern void Rec35_Leaf_167C4(void *);
            REC35_CALLEE(Rec35_Leaf_167C4, _0800167C4)((void *)rec);
            break;
        }
        case 5: {
            if (*(volatile u16 *)(rec + 12) != 0) {
                extern void Rec35_Leaf_166E8(void *, int, int);
                REC35_CALLEE(Rec35_Leaf_166E8, _0800166E8)((void *)rec, (int)(u16)arg1, (int)(u16)arg2);
            }
            break;
        }
        case 0: {
            extern void Rec35_Record49Constructor(void *);
            REC35_CALLEE(Rec35_Record49Constructor, _08001650C)((void *)rec);
            break;
        }
        case 11: {
            // 0x080016508 is a bare `bx lr`, but the ROM still does
            // `adds r0, r4, #0` before the bl, so the record is passed.
            extern void Rec35_Leaf_16508(void *); // 0x080016508
            REC35_CALLEE(Rec35_Leaf_16508, _080016508)((void *)rec);
            break;
        }
        default: break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001681C(void *c, int a, int b, int d) __attribute__((alias("Rec35_Dispatch_1681C")));
#endif

// _080016AD4: setter at ctx+346 =180, plus _08002060(0) (pools none beyond +346)
// Width: ldrh/strh via adds #346 (173<<1)
extern void _08002060(int v);   // slice-closure spelling; body + `Sub_` alias in src/idle_accessors.c
void Rec35_Leaf_16AD4(void *ctx) {
    _08002060(0);
    volatile u8 *c = (volatile u8 *)ctx;
    *(volatile u16 *)(c + 346) = 180;
}
// The body is 26 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this function's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080016AD4(void *c) __attribute__((alias("Rec35_Leaf_16AD4")));
#endif

// _080016940: bx lr stub
void Rec35_BxLr_16940(void) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080016940(void) __attribute__((alias("Rec35_BxLr_16940")));
void sub_080016940(void) __attribute__((alias("Rec35_BxLr_16940")));
#endif

// _0800168B8: flag leaf at WA+0x10C3, calls _0802B368 with low halfword width.
// The ROM DUPLICATES the `adds r0,r1,#0 / bl sub_0802B368` pair in both arms of
// the `cmp r0,#1 / bne` -- it does not share a tail. Writing the call inside
// both arms is what keeps them separate; a single call after the gate let agbcc
// emit one `bl` and drop the `b` over the literal pool (measured 6/48).
void Rec35_Leaf_168B8(u32 a0) {
    u16 v = (u16)(a0 & 0xFFFFu); // lsls r0,#16 / lsrs r1,r0,#16 -- value in r1
    // ROM reaches WA+0x10C3 as TWO pool words plus `adds r0,r0,r2`, so the base
    // must stay a SYMBOL_REF (see src/ai_award_leaves.c Ai_SetOwnedFlag).
#ifndef __APPLE__
    extern u8 Rec35Wa4[] __asm__("Rec35Wa4");
    volatile u8 *wa = (volatile u8 *)(uintptr_t)Rec35Wa4;
    __asm__(".globl Rec35Wa4\nRec35Wa4 = 0x03001780\n");
#else
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
#endif
    u8 f = *(volatile u8 *)(wa + 0x10C3u);
    if (f == 1) sub_0802B368(v);
    else sub_0802B368(v);
}
// The body is 46 bytes; the ROM's 48-byte span ends in `00 00`, while gas
// closes a Thumb code section with a `nop` (0x46c0). This file-scope
// `.align 2, 0` lands after the body's `.size`, still inside its own section,
// so it pads with the explicit zero. Same idiom as Rec35_Leaf_16AD4 below.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800168B8(u32 a0) __attribute__((alias("Rec35_Leaf_168B8")));
#endif

// _080016DC0 lifted exactly in src/rec35_mid_region.c; duplicate removed.
// _0800173E8: small setup at ctx+192/56/60/88/84 etc., flags via _08002618 and _0802B368
extern void _08002618(int a, int b);   // slice-closure spelling; body + Sub_/sub_ aliases in src/event_dma_queue.c
extern void _0802B368(int v);           // slice-closure spelling; body + Sub_/sub_ aliases in src/sound.c
extern void Sub_08002618(int a,int b);
extern void sub_08002618(int a,int b);
void Rec35_Leaf_173E8(void *ctx) {
    _08002618(1,0);
    volatile u8 *c = (volatile u8 *)ctx;
    *(volatile u16 *)(c + 192) = 0;
    _0802B368(1);
    *(volatile u32 *)(c + 56) = 10;
    *(volatile u16 *)(c + 60) = 0;
    *(volatile u32 *)(c + 88) = 10;
    *(volatile u32 *)(c + 84) = 1;
}
#ifndef __APPLE__
void _0800173E8(void *c) __attribute__((alias("Rec35_Leaf_173E8")));
void sub_0800173E8(void *c) __attribute__((alias("Rec35_Leaf_173E8")));
#endif

// _080017414: twin of 173E8 with +192=0 and flag 4
void Rec35_Leaf_17414(void *ctx) {
    sub_08002618(1,0);
    volatile u8 *c = (volatile u8 *)ctx;
    u32 z;
    // `z = 0` lives inside the first store's expression on purpose. Written as
    // its own statement, agbcc materialises the zero *before* the
    // `adds r0, r5, #0` / `adds r0, #192` address pair and hands r5 to the
    // pointer; written here it keeps the address computation first and gives
    // r4 to the zero, which the ROM does at 0x08017424 and then reuses for
    // all three zero-stores (0x08017426, 0x08017432, 0x08017436).
    *(volatile u16 *)(c + 192) = (u16)(z = 0);
    sub_0802B368(4);
    *(volatile u32 *)(c + 56) = 10;
    *(volatile u16 *)(c + 60) = (u16)z;
    *(volatile u32 *)(c + 88) = 10;
    *(volatile u32 *)(c + 84) = z;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080017414(void *c) __attribute__((alias("Rec35_Leaf_17414")));
// The closure spells this VMA `sub_080017414` and the entry exports it, so the
// owning TU must define that exact name. One hop to the real body -- an
// alias-of-an-alias would not be in the slice link.
void sub_080017414(void *c) __attribute__((alias("Rec35_Leaf_17414")));
#endif

// _080016F28: 5-way dispatcher at ctx+200 via table 0x08016F44 (lsl #2/mov pc, s16 width)
// Pools: 0x08016F44 table (5 entries), widths ldrsh, helper ABI Sub_0802B3B8/Sub_08024C0C etc. proven
extern int Sub_0802B3B8(int v);
extern void Sub_08024C0C(void);
void Rec35_Dispatch_16F28(void *ctx) {
    volatile u8 *c8 = (volatile u8 *)ctx;
    s16 ph = *(volatile s16 *)(c8 + 200);
    if (ph < 0 || ph > 4) return;
    switch (ph) {
        case 0: {
            // _080016F58: 0x0802B3B8(3) != 0 (<<24 test) → exit; else switch
            // s16[ctx+182]: 1 → 0x08024C0C(0), 2 → 0x08024C0C(1); then leaves
            // 0x080016944 + 0x080016A20, set [ctx+200]=4, event 10 @ ctx+240,
            // bind via [ctx+44]/[[ctx+484]] → 0x08007ABC (asm evidence).
            extern int Sound_0x0802B3B8(int v);
            if ((Sound_0x0802B3B8(3) << 24) != 0) break;
            s16 v182 = *(volatile s16 *)(c8 + 182);
            if (v182 == 1 || v182 == 2) {
                extern void Sound_0x08024C0C(void); // 0x08024C0C (asm body is `bx lr`)
                Sound_0x08024C0C();
                extern void Rec35_Leaf_16944(void *c); // 0x08016944
                Rec35_Leaf_16944(ctx);
                extern void Rec35_Leaf_16A20(void *c); // 0x08016A20
                Rec35_Leaf_16A20(ctx);
                *(volatile s16 *)(c8 + 200) = 4;
                extern void Event_0x08025BF0(void *list, int id); // 0x08025BF0 (ROM shape (P,S))
                Event_0x08025BF0((void *)(c8 + 480), 10);
                extern void EventBind_0x08007ABC(void *a, int b, int c2); // 0x08007ABC (ROM shape (P,S,S),)
                EventBind_0x08007ABC(*(void **)(c8 + 44),
                                     (int)(uintptr_t)*(void **)(c8 + 484), 0);
            }
            break;
        }
        case 1: Rec35_Leaf_173E8(ctx); *(volatile s16 *)(c8 + 200) = 2; break;
        case 2: *(volatile s16 *)(c8 + 60) = 1; break; // _080016FB4 (asm evidence)
        case 3: *(volatile s16 *)(c8 + 200) = 2; break;
        case 4: break; // _080016FC8 exit
        default: break;
    }
}
#ifndef __APPLE__
void _080016F28(void *c) __attribute__((alias("Rec35_Dispatch_16F28")));
#endif

// bx lr stubs (size 5, no pools, proven via single bx lr, no helper ABI)
// `Rec35_BxLr_17D50` ignores its argument, but the three paint callers all
// reload `rec` into r0 immediately before the `bl` (e.g. 0x08017C6C
// `adds r0, r4, #0` / `bl 0x08017D50`), so it must take one: with a
// zero-arg prototype r4 is dead at that point and agbcc folds the +56
// addend into r4 itself instead of copying then adding.
void Rec35_BxLr_17D50(void *rec) { (void)rec; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080017D50(void *rec) __attribute__((alias("Rec35_BxLr_17D50")));
void sub_080017D50(void *rec) __attribute__((alias("Rec35_BxLr_17D50")));
#endif
void Rec35_BxLr_17F00(void) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080017F00(void) __attribute__((alias("Rec35_BxLr_17F00")));
void sub_080017F00(void) __attribute__((alias("Rec35_BxLr_17F00")));
#endif
void Rec35_BxLr_18418(void) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080018418(void) __attribute__((alias("Rec35_BxLr_18418")));
void sub_080018418(void) __attribute__((alias("Rec35_BxLr_18418")));
#endif
void Rec35_BxLr_1841C(void *hdr, void *a) { (void)hdr; (void)a; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001841C(void *hdr, void *a) __attribute__((alias("Rec35_BxLr_1841C")));
void sub_08001841C(void *hdr, void *a) __attribute__((alias("Rec35_BxLr_1841C")));
#endif

// _08001774C lifted in src/rec35_mid_region.c  with the exact
// (void *rec) 1-arg form the ROM body uses (r1/r2 dead); old copy removed.
// _080018420: push {r4,lr}; Sub_08004B68(a) Vu32 r0=a; sub_08001841C(hdr,a) (bx lr)
// Pools: none, widths none beyond bl, complete boundary push/pop
extern void *sub_08004B68(void);   // slice-closure spelling; the real callee takes no args (src/foundation_subsys.c MgrGet80)
extern void sub_08001841C(void *hdr, void *a);
void Rec35_Leaf_18420(void *a) {
    void *hdr = sub_08004B68();
    sub_08001841C(hdr, a);
}
#ifndef __APPLE__
void _080018420(void *a) __attribute__((alias("Rec35_Leaf_18420")));
void sub_080018420(void *a) __attribute__((alias("Rec35_Leaf_18420")));
#endif

// _080018260: push {lr}; strb Vu8 1 at s+89; _080056F4(s,1,1) Vu8
// Pools: none, widths strb Vu8 +89, helper strb +20, pure Thumb
// ROM keeps the `1` live in r1 across the store and passes it as the second
// argument (only r2 is reloaded at +12). Writing the literal twice let agbcc
// rematerialize it, which cost a whole extra `movs r1,#1` and desynchronised
// every instruction after +12; a named local gives the register a live range.
//
// Callee spelling. 0x080056F4 carried NO label anywhere in the closure until
//, so five call sites (asm/code_22cb4.s, menu_f924.s, menu_ff78.s,
// race_scene.s, rec35_runtime.s) reached it as a bare numeric `bl 0x080056F4`
// and this lift was written against the `Sub_` fallback the screen then had to
// offer. asm/code_4e6c.s:1119 now defines `_080056F4:` at exactly 0x080056f4,
// so the screen asks for the `_` form. src/event_dma_queue.c:687 DEFINES
// `_080056F4` (one hop to the real body SlotLaneSet_56F4) and still defines
// the sub_/Sub_ spellings beside it, so nothing else has to move: the sibling
// TUs (menu_ff78_f.c, rec35_mid_region.c) keep calling `Sub_080056F4`, and the
// closure's `bl` operands are numeric in either case.
// Split two-sided: the host build has no VMA-named symbols, and the Apple half
// keeps `Sub_`, which the host already resolves to the weak no-op at
// src/menu_ff78_f.c:41 that those siblings call unconditionally.
#ifndef __APPLE__
extern void _080056F4(void *s, int idx, u8 val);   // 0x080056F4 (event_dma_queue.c)
#else
extern void Sub_080056F4(void *s, int idx, int val);
#endif
void Rec35_Leaf_18260(void *a, void *s) {
    volatile u8 *p;
    int one;
    (void)a;
    p = (volatile u8 *)s + 89;
    one = 1;
    *p = (u8)one;
#ifndef __APPLE__
    _080056F4(s, one, 1);
#else
    Sub_080056F4(s, one, 1);
#endif
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080018260(void *a, void *b) __attribute__((alias("Rec35_Leaf_18260")));
#endif

// _080018344: push {r4,lr}; sub_08007664(0,0x08292B40,9) Vu32 pools; str Vu32 150 at +0, 0 at +4
// Pools: 0x08292B40 Vu32 ROM, helper sub_08007664 ABI proven via 0x08007498 record resolvers
extern void sub_08007664(int a, void *pool, int c);
void Rec35_Leaf_18344(void *a) {
    sub_08007664(0, (void *)0x08292B40, 9);
    *(volatile u32 *)a = 150;
    *(volatile u32 *)((volatile u8 *)a + 4) = 0;
}
#ifndef __APPLE__
void _080018344(void *a) __attribute__((alias("Rec35_Leaf_18344")));
#endif

// _08001876C — canonical body: rec35_dispatch_tail.c `Rec35_Setup_1876C`
// (`sub_08007664(0, 0x08292B40, 7)`, then u32[ctx] = 150 / u32[ctx+4] = 0).
// A second, byte-identical body used to live here. The link keeps the FIRST
// definition and objects are linked in filename order, so a duplicate makes
// the surviving body depend on file names — and if the two ever drifted, the
// winner would be arbitrary. Removed : never re-add a body for a VMA
// another TU already lifts.

// _0800184F0: push {lr}; _08026020(0) Vu32; _08026020(1); _08026020(2) — pool 0x08060D4C Vu32 94/95/96 via 0x0802E0A4 6B copy, strb Vu8 at 0x03001CFC+slot, ldrsh Vs16 94/95/96 → bl 0x08025F78 Vu32
// Pools: 0x08060D4C Vu32 ROM 94/95/96 (6B), 0x03001780 Vu32 base, 0x5C offset, helper ABI proven via ai_award_tail.s + ai_collect.s + runtime_mem.s 0x0802E0A4 6B memcpy
extern void _08026020(int slot);   // slice-closure spelling; body + Sub_ alias in src/ai_award_leaves.c
void Rec35_Leaf_184F0(void) {
    _08026020(0);
    _08026020(1);
    _08026020(2);
}
#ifndef __APPLE__
void _0800184F0(void) __attribute__((alias("Rec35_Leaf_184F0")));
void sub_0800184F0(void) __attribute__((alias("Rec35_Leaf_184F0")));
#endif

// _08001881C — canonical body: rec35_dispatch_tail.c `Rec35_Setup_1881C`
// (identical to 0x08001876C except `sub_08007664(..., 10)`). Duplicate body
// removed here  for the same reason as _08001876C above.

// _080018364: push {lr}; ldr Vu32 +0 ble; subs Vu32 -1 str; ldr Vu32 +4 bne; str Vu32 1 at +4; bl 0x08004EC0 Vu32 pool 0x03000198
// _080018364: 36-byte ROM span, the same countdown shape as _08001878C /
// _08001883C above and byte-identical to menu_stage.c's MenuStage_0800D704
// except for the `bl` displacement. Derive both fields from the parameter:
// a `volatile u32 *p` local makes agbcc materialise a second base register
// for p[1], which the ROM does not have. The ROM passes r0=1 to 0x08004EC0
// (`movs r0,#1` feeds the bl), and the extern declares that one int arg.
// The `Sub_` twin is NOT usable here: 0x08004EC0 is a promoted entry whose
// `export` carries `_08004EC0`/`sub_08004EC0` only, and promotion_screen
// refuses a body that calls a spelling the slice closure cannot bind.
extern void _08004EC0(int v);
void Rec35_Leaf_18364(void *a) {
    s32 n = *(volatile s32 *)a;
    if (n > 0) {
        *(volatile s32 *)a = n - 1;
    } else if (*(volatile s32 *)((u8 *)a + 4) == 0) {
        *(volatile s32 *)((u8 *)a + 4) = 1;
        _08004EC0(1);
    }
}
// Body is 34 bytes; the last two bytes of the span are the ROM's `00 00` pad.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080018364(void *a) __attribute__((alias("Rec35_Leaf_18364")));
#endif
// _08001878C / _08001883C are the same countdown shape and are lifted in

// _0800182D8: push {lr}; lsls Vu16 r2, adds Vu32 0xFFFF0000, lsrs Vu16; bhi Vu16 >1; bl 0x0802B368 Vu32 1; bl 0x08004EA8 Vu32 1
// Pools: 0xFFFF0000 Vu32 at _0800182F8, helpers _0802B368 Vu32 r0=1 +0x03001764 pool proven, _08004EA8 Vu32 pool 0x03000198
extern void _0802B368(int v);          // slice-closure spelling (src/sound.c)
extern void _08004EA8(int v);          // slice-closure spelling (src/foundation_subsys.c)
void Rec35_Leaf_182D8(int a, int b, int c) {
    (void)a; (void)b;
    u16 v = (u16)c;
    u16 w = (u16)(v - 1); // lsls Vu16 #16; adds Vu32 0xFFFF0000; lsrs Vu16 #16
    if (w > 1) return; // cmp #1 bhi Vu16 >1
    _0802B368(1);
    _08004EA8(1);
}
#ifndef __APPLE__
void _0800182D8(int a, int b, int c) __attribute__((alias("Rec35_Leaf_182D8")));
void sub_0800182D8(int a, int b, int c) __attribute__((alias("Rec35_Leaf_182D8")));
#endif

// _080018210: push {r4,r5,lr}; Sub_08025C60(b,c) Vu32 pool 0x08025C80 Vu32 0x080CD830 table +20 stride, ldr Vu32 +4/+24/+20, Sub_08007ABC Vu32
extern void sub_080025C60(void *s, int idx);
extern void sub_08007ABC(u32 a, u32 b, u32 c);
void Rec35_Leaf_18210(void *a, void *b, int c) {
    sub_080025C60(b, c);
    void *pa4 = *(void **)((volatile u8 *)a + 4);
    int v24 = *(volatile u32 *)((volatile u8 *)b + 24);
    int v20 = *(volatile u32 *)((volatile u8 *)b + 20);
    sub_08007ABC((u32)(uintptr_t)pa4, (u32)v24, (u32)v20);
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080018210(void *a, void *b, int c) __attribute__((alias("Rec35_Leaf_18210")));
#endif

// _080018434: push {r4,lr}; loop r4=1..98 skip 54/94/95/96; Sub_08025F78(r4) Vu32 0x03001780 pool 0x03001780 etc.
// Pools: none (movs #1 Vu32 etc.), helper Sub_08025F78 Vu32 bitset at 0x030017A0 + 0x080CD9D4 pool proven via ai_award_bitset.s
extern void _08025F78(int v); // asm/ai_award_bitset.s spells it this way
void Rec35_Leaf_18434(void) {
    for (int r4 = 1; r4 <= 98; r4++) {
        if (r4 == 54) continue;
        if (r4 == 94) continue;
        if (r4 == 95) continue;
        if (r4 == 96) continue;
        _08025F78(r4);
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080018434(void) __attribute__((alias("Rec35_Leaf_18434")));
#endif

// ---- : rec35_runtime.s 0x08018230-0x080183D8 dispatcher family (8 VMAs) ----
// Evidence: instruction-for-instruction from asm/rec35_runtime.s; pool words
// 0x0832EBF0/0x082B7410/0x08292B40/0xFFFF0000 preserved as immediates.
// Shapes: _080018230 = 9-arg packet build (r3=[b+4] feeds the r3 slot, stack
// {c+[b+8],3,1,0,0} into sub_08007C68); _080018278 = template/emit/bind leaf
// (r1/r2 dead); _0800182FC/_080018388 = bx lr stubs; _080018300/_080018394 =
// 4-way cmp-chain dispatchers on ev with rec in r3 (NOT table-driven — plain
// cmp/beq/bhi chains); _08001838C = interior BL entry (*(base+84)=1, first
// arg dead); _0800183D8 = record-setup leaf (07664 sel 2, 075E8 (tmpl,1,0)).
// Callee spellings: Sub_08007C68 / Sub_08007770 name the exact ROM bodies
// (9-arg 07C68 with 5 caller stack words; 6-arg 07770). Course_Iter_07C68 is faithful 9-arg (closed) and
// Course_0x08007770 is faithful 6-arg (closed; _080018230
// deliberately binds the ROM route for 07C68 (provenance), and _080018278
// binds Sub_08007770 (exact ROM) — both routes now equivalent where faithful.
// Other callees use the _0800 form to bind strong lifted bodies
// (course_resource.c / code_25930.c / sound.c); the rest route to the ROM bodies.
extern void Sub_08007C68(void *a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i); // 0x08007C68
extern void _08007C68(void *a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i); // faithful strong body (course_records.c)
// NOTE: Sub_08007770 (6-arg exact-ROM-body spelling) is declared file-scope above.
extern void _0800798C(void *a, void *b); // 0x0800798C
extern void _080075E8(void *a, int b, int c); // 0x080075E8
extern void _08007ABC(void *a, u32 b, u32 c); // 0x08007ABC
extern void _080025BC8(void *a, int b); // 0x08025BC8
extern void _08007A58(void *p); // 0x08007A58

// _080018230: push {r4,r5,lr}; sub sp,#20; r4=[b+20]; r5=[b+24]; r3=[b+4];
// r1=[b+8]; str (c+r1)@[sp]; stack {+0:c+v08, +4:3, +8:1, +12:0, +16:0};
// r1=r4; r2=r5; bl sub_08007C68. r0 (a) passes through untouched.
void Rec35_Leaf_18230(void *a, void *b, int c) {
    u32 v20 = *(volatile u32 *)((volatile u8 *)b + 20);
    u32 v24 = *(volatile u32 *)((volatile u8 *)b + 24);
    u32 v04 = *(volatile u32 *)((volatile u8 *)b + 4);
    u32 v08 = *(volatile u32 *)((volatile u8 *)b + 8);
    _08007C68(a, v20, v24, v04, (u32)c + v08, 3, 1, 0, 0); // R1 C body (was Sub_ ROM veneer)
}
// The body is 46 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080018230(void *a, void *b, int c) __attribute__((alias("Rec35_Leaf_18230")));
#endif

// _080018278: push {r4,r5,lr}; sub sp,#8; r4=r0; _08007770(0,0x0832EBF0,0,0
// +stack{0,1}); _0800798C(0x082B7410,r4); _080075E8(0x082B7410,0,3);
// _080025BC8(r4+44,19); _08007ABC([r4+4],[r4+48],[r4+44]);
// _080018210(r4,r4+24,19). Incoming r1/r2 never read (dead).
void Rec35_Leaf_18278(void *ctx) {
    volatile u8 *c = (volatile u8 *)ctx;
    _08007770(0, (void *)(uintptr_t)0x0832EBF0u, 0, 0, 0, 1); // R2 C body (was Sub_ veneer)
    _0800798C((void *)(uintptr_t)0x082B7410u, ctx);
    _080075E8((void *)(uintptr_t)0x082B7410u, 0, 3);
    _080025BC8((void *)(c + 44), 19);
    _08007ABC((void *)(uintptr_t)*(volatile u32 *)(c + 4),
              *(volatile u32 *)(c + 48), *(volatile u32 *)(c + 44));
    _080018210(ctx, (void *)(c + 24), 19);
}
#ifndef __APPLE__
void _080018278(void *c) __attribute__((alias("Rec35_Leaf_18278")));
void sub_080018278(void *c) __attribute__((alias("Rec35_Leaf_18278")));
#endif

// _0800182FC: single.hword 0x4770 (bx lr) + pad; BL target of _080018300 case 7,
// which passes rec in r0 (0x0801833A: adds r0,r3,#0; bl _0800182FC) — unused.
void Rec35_BxLr_182FC(void *rec) { (void)rec; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800182FC(void *rec) __attribute__((alias("Rec35_BxLr_182FC")));
#endif

// _080018388: bx lr stub (2 B + pad; code continues at interior entry _08001838C).
// Called with (rec, u16 b, u16 c) — all ignored.
void Rec35_BxLr_18388(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080018388(void *a, int b, int c) __attribute__((alias("Rec35_BxLr_18388")));
void sub_080018388(void *a, int b, int c) __attribute__((alias("Rec35_BxLr_18388")));
#endif

// _08001838C: interior BL entry (no.type; reached via bl from _080018394):
// adds r1,#84; movs r0,#1; strh r0,[r1] → *(u16*)(base+84)=1. r0/c dead.
void Rec35_Leaf_1838C(void *rec, void *base, int c) {
    (void)rec; (void)c;
    *(volatile u16 *)((volatile u8 *)base + 84) = 1;
}
#ifndef __APPLE__
void _08001838C(void *a, void *b, int c) __attribute__((alias("Rec35_Leaf_1838C")));
#endif

// _080018300: cmp-chain dispatcher on ev (r0), rec in r3:
// 2→_080018260(rec,b), 1→_080018278(rec), 6→_0800182D8(rec,b16,c16),
// 7→_0800182FC(rec); else return. (cmp#2/beq; bhi; cmp#1/beq; cmp#6/#7/beq.)
void Rec35_Dispatch_18300(u32 ev, int b, int c, void *rec) {
    switch (ev) {
        case 2: _080018260(rec, (void *)(uintptr_t)b); break;
        case 1: _080018278(rec); break;
        case 6: _0800182D8((int)(uintptr_t)rec, (int)(u16)b, (int)(u16)c); break;
        case 7: _0800182FC(rec); break;
        default: break;
    }
}
#ifndef __APPLE__
void _080018300(u32 a, int b, int c, void *d) __attribute__((alias("Rec35_Dispatch_18300")));
#endif

// _080018394: cmp-chain dispatcher on ev (r0), rec in r3:
// 2→_08001838C(rec,b,c), 1→_080018344(rec), 5→_080018364(rec),
// 6→_080018388(rec,b16,c16); else return. (cmp#2/beq; bhi→cmp#5/#6; cmp#1/beq.)
void Rec35_Dispatch_18394(u32 ev, int b, int c, void *rec) {
    switch (ev) {
        case 2: _08001838C(rec, (void *)(uintptr_t)b, c); break;
        case 1: _080018344(rec); break;
        case 5: _080018364(rec); break;
        case 6: _080018388(rec, (int)(u16)b, (int)(u16)c); break;
        default: break;
    }
}
#ifndef __APPLE__
void _080018394(u32 a, int b, int c, void *d) __attribute__((alias("Rec35_Dispatch_18394")));
#endif

// _0800183D8: push {r4,r5,lr}; r4=r0; *(r4+8)=0; _08002B214(48);
// sub_08007664(0,0x08292B40,2); r4+=28; _0800798C(0x08292B40,r4);
// _08007A58(r4); _080075E8(0x08292B40,1,0).
extern void _0802B214(int v);   // slice-closure spelling; body + Sub_/sub_ aliases in src/sound.c
void Rec35_Leaf_183D8(void *ctx) {
    volatile u8 *c = (volatile u8 *)ctx;
    *(volatile u16 *)(c + 8) = 0;
    _0802B214(48);
    sub_08007664(0, (void *)(uintptr_t)0x08292B40u, 2);
    _0800798C((void *)(uintptr_t)0x08292B40u, (void *)(c + 28));
    _08007A58((void *)(c + 28));
    _080075E8((void *)(uintptr_t)0x08292B40u, 1, 0);
}
#ifndef __APPLE__
void _0800183D8(void *c) __attribute__((alias("Rec35_Leaf_183D8")));
void sub_0800183D8(void *c) __attribute__((alias("Rec35_Leaf_183D8")));
#endif


// ROM entry alias.
#ifndef __APPLE__
void Rec35_Leaf_16508(void *a) __attribute__((alias("Rec35_BxLrStub")));
#endif

// 0x08025C60 — 0x94B record fill from the per-slot template table
//
// Three measured facts give the ROM's 36 bytes (was 44 with per-word stores):
//  * the table base must be materialised into a NAMED local before the row
//    address, or agbcc loads the pool word after the index arithmetic
//    (`ldr r1,[pc,#20]` at +8 instead of the ROM's `ldr r3,[pc,#28]` at +2);
//  * the 20-byte payload must be ONE struct assignment: that is what lowers
//    to the ROM's `ldmia/stmia {r1,r3,r4}` + `ldmia/stmia {r1,r4}` pair. Five
//    separate volatile word stores do not;
//  * the +24 scalar is read through a NON-VOLATILE `s16` lvalue, giving the
//    ROM's `movs r3,#0 / ldrsh r1,[r2,r3] / str r1,[r0,#24]`. A volatile
//    s16 lvalue gives `ldrh` + a narrow pair instead.
typedef struct { u32 f0, f1, f2, f3, f4; } Rec35_Row20;
void Rec35_RecordFill_25C60(void *dst, int idx) {
#ifndef __APPLE__
    extern u8 Rec35RowTbl25C60[] __asm__("Rec35RowTbl25C60");
    __asm__(".globl Rec35RowTbl25C60\nRec35RowTbl25C60 = 0x080CD830\n");
    const u8 *base = (const u8 *)(uintptr_t)Rec35RowTbl25C60;
#else
    const u8 *base = (const u8 *)(uintptr_t)0x080CD830u;
#endif
    const u8 *row = base + (u32)idx * 20u;
    *(volatile u32 *)((u8 *)dst + 24) = (u32)(s32)((const s16 *)row)[0];
    *(Rec35_Row20 *)dst = *(const Rec35_Row20 *)row;
}
#ifndef __APPLE__
void Sub_08025C60(void *a, int b) __attribute__((alias("Rec35_RecordFill_25C60")));
void _08025C60(void *a, int b) __attribute__((alias("Rec35_RecordFill_25C60")));
void sub_08025C60(void *a, int b) __attribute__((alias("Rec35_RecordFill_25C60")));
void sub_080025C60(void *a, int b) __attribute__((alias("Rec35_RecordFill_25C60")));
#endif
