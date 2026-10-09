// Every function is transcribed instruction-for-instruction from the cited
// asm listings. The cluster is the runtime-registered rec35 record-49
// transition/tail machinery: twin event handlers on u16[rec+140], sound-
// driven clears, 30-iteration course-record placement (sub_08007B18), the
// table-driven emitter dispatch (0x080CB89C / 0x080CB88C stride-8 records)
// and the record-49 tail driver that uploads a 48-mode packet via
// sub_08007C68 and ticks the menu machinery (sub_0800DBE8).
// ============================================================================

#include "gtadv/menus.h"
#include "gba/types.h"

// ----------------------------------------------------------------------------
// Extern callees. All of them resolve to real bodies on ARM: _0802B368 and
// _08002618 trampoline to asm; sub_08007B18/_08007C68 trampoline to asm;
// sub_0800DBE8 binds the menus.c strong lift; sub_0800D97C binds the strong
// lift below. Host weak no-ops mirror the usual override model.
// ----------------------------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) void _0802B368(int v) { (void)v; }
__attribute__((weak)) void _08002618(int a, int b) { (void)a; (void)b; }
__attribute__((weak)) void sub_08007B18(void *a, int b, int c, int d, int e, int f, int g, int h)
    { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; }
__attribute__((weak)) void sub_08007C68(void *a, u32 b, u32 c, int d, int e, int f, int g, int h, int i)
    { (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i; }
__attribute__((weak)) void sub_0800DBE8(void *a) { (void)a; }
#else
extern void _0802B368(int v);                    // 0x0802B368 sound (asm)
extern void _08002618(int a, int b);             // 0x08002618 (asm)
extern void sub_08007B18(void *a, int b, int c, int d, int e, int f, int g, int h); // 0x08007B18 (asm)
extern void sub_08007C68(void *a, u32 b, u32 c, int d, int e, int f, int g, int h, int i); // 0x08007C68 (asm)
extern void sub_0800DBE8(void *a);               // 0x0800DBE8 (menus.c)
#endif

#define WA_1084 (*(volatile u16 *)((uintptr_t)0x03001780 + 0x1084))

// Non-foldable WA base. Several ROM bodies in this cluster reach a work-area
// cell as TWO pool words plus an `adds` (`ldr 0x03001780 / ldr <off> / adds`),
// which only happens when the base is a SYMBOL_REF rather than a CONST_INT:
// simplify_rtx folds the folded form into a single pool word at expand time
// and the whole `ldr/adds` pair disappears. Same lever as src/ai_award_leaves.c
// (Ai_SetOwnedFlag) and src/car_tick_helpers.c (CarTickWa).
#ifndef __APPLE__
extern u8 Rec35Wa[] __asm__("Rec35Wa");
#define REC35_WA_BASE ((volatile u8 *)(uintptr_t)Rec35Wa)
// The `.globl` DEFINITION is emitted inside Rec35_RecordClear2 -- see there.
#else
#define REC35_WA_BASE ((volatile u8 *)(uintptr_t)0x03001780u)
#endif

void Rec35_Noop(void *rec); // fwd (defined below, called by Rec35_Record49Tail)

// ============================================================================
// sub_0800D97C (0x0800D97C, 0x26 B, from asm/menu_d8e4.s) — countdown cell:
//   u16[rec] -= 1;  if ((s16)new < 0): u16[rec] = reload and toggle s16[rec+2]
//   between 0/1 (only when it was exactly 0 or 1).
void Rec35_CountdownDec(void *rec, int reload) {
    volatile u16 *cell = (volatile u16 *)rec;
    u16 nv = (u16)(*cell - 1);
    *cell = nv;
    if ((s16)nv < 0) {
        *cell = (u16)reload;
        s16 st = (s16)*(volatile u16 *)((u8 *)rec + 2);
        if (st == 0) {
            *(volatile u16 *)((u8 *)rec + 2) = 1;
        } else if (st == 1) {
            *(volatile u16 *)((u8 *)rec + 2) = 0;
        }
    }
}
#ifndef __APPLE__
/* VMA exports intentionally omitted — see the ownership note above. */
#endif

// ============================================================================
// sub_080016004 (0x08016004, 0xC0 B) — event handler (twin A) on
// u16[rec+140].  (rec, unused, mode):
//   mode 2: sound 4; [rec+16]=10; [rec+20]=0; [rec+48]=10; [rec+44]=0;
//           WA+0x1084 = 0
//   mode 1: cur = s16[rec+140];
//     cur==0: sound 1; [rec+144]=0; [rec+150]=1
//     cur==1: sound 1; [rec+16]=10; [rec+20]=0; [rec+48]=10; [rec+44]=0;
//             WA+0x1084 = 0
//   mode 64: [rec+140]-=1;  mode 128: [rec+140]+=1
//   clamp s16[rec+140] to [0,1]; if u16[rec+140] changed: sound 2
// ============================================================================
void Rec35_EventA(void *rec, int unused, int mode) {
    (void)unused;
    u8 *r5 = (u8 *)rec;
    u16 r6 = (u16)mode;
    volatile u16 *p140 = (volatile u16 *)(r5 + 140);
    u16 r7 = *p140;
    if (r6 == 2) {
        _0802B368(4);
        *(volatile u32 *)(r5 + 16) = 10;
        *(volatile u16 *)(r5 + 20) = 0;
        *(volatile u32 *)(r5 + 48) = 10;
        *(volatile u16 *)(r5 + 44) = 0;
        WA_1084 = 0;
    }
    if (r6 == 1) {
        int cur = (s16)*p140;
        if (cur == 0) {
            _0802B368(1);
            *(volatile u16 *)(r5 + 144) = 0;
            *(volatile u16 *)(r5 + 150) = r6;
        } else if (cur == 1) {
            _0802B368(1);
            *(volatile u32 *)(r5 + 16) = 10;
            *(volatile u16 *)(r5 + 20) = 0;
            *(volatile u32 *)(r5 + 48) = 10;
            *(volatile u16 *)(r5 + 44) = 0;
            WA_1084 = 0;
        }
    }
    if (r6 == 64) {
        *p140 = (u16)(*p140 - 1);
    }
    if (r6 == 128) {
        *p140 = (u16)(*p140 + 1);
    }
    if ((s16)*p140 < 0) *p140 = 0;
    if ((s16)*p140 > 1) *p140 = 1;
    if (*p140 != r7) _0802B368(2);
}
#ifndef __APPLE__
void _080016004(void *a, int b, int c) __attribute__((alias("Rec35_EventA")));
void sub_080016004(void *a, int b, int c) __attribute__((alias("Rec35_EventA")));
#endif

// ============================================================================
// sub_0800160C4 (0x080160C4, 0xA0 B) — twin B of 0x16004. Differences:
//   mode 2 does NOT clear WA+0x1084; mode 1 cur==0 writes [rec+148] not
//   [rec+150]; mode 1 cur==1 does not clear WA+0x1084.
// ============================================================================
void Rec35_EventB(void *rec, int unused, int mode) {
    (void)unused;
    u8 *r5 = (u8 *)rec;
    u16 r6 = (u16)mode;
    volatile u16 *p140 = (volatile u16 *)(r5 + 140);
    u16 r7 = *p140;
    if (r6 == 2) {
        _0802B368(4);
        *(volatile u32 *)(r5 + 16) = 10;
        *(volatile u16 *)(r5 + 20) = 0;
        *(volatile u32 *)(r5 + 48) = 10;
        *(volatile u16 *)(r5 + 44) = 0;
    }
    if (r6 == 1) {
        int cur = (s16)*p140;
        if (cur == 0) {
            _0802B368(1);
            *(volatile u16 *)(r5 + 144) = 0;
            *(volatile u16 *)(r5 + 148) = r6;
        } else if (cur == 1) {
            _0802B368(1);
            *(volatile u32 *)(r5 + 16) = 10;
            *(volatile u16 *)(r5 + 20) = 0;
            *(volatile u32 *)(r5 + 48) = 10;
            *(volatile u16 *)(r5 + 44) = 0;
        }
    }
    if (r6 == 64) {
        *p140 = (u16)(*p140 - 1);
    }
    if (r6 == 128) {
        *p140 = (u16)(*p140 + 1);
    }
    if ((s16)*p140 < 0) *p140 = 0;
    if ((s16)*p140 > 1) *p140 = 1;
    if (*p140 != r7) _0802B368(2);
}
#ifndef __APPLE__
void _0800160C4(void *a, int b, int c) __attribute__((alias("Rec35_EventB")));
void sub_0800160C4(void *a, int b, int c) __attribute__((alias("Rec35_EventB")));
#endif

// ============================================================================
// sub_080016164 (0x08016164, 0x3C B) — sound-driven record clear.
//   Executes when (u16)mode - 1 <= 1 (mode 1 or 2, unsigned compare):
//     [rec+144]=0; _08002618(1,0); sound 1;
//     [rec+16]=10; [rec+20]=0; [rec+48]=10; [rec+44]=0.
// ============================================================================
void Rec35_RecordClear(void *rec, int unused, int mode) {
    (void)unused;
    u8 *r5 = (u8 *)rec;
    // ROM gate: lsls r2,#16 / ldr r0,=0xFFFF0000 / adds r2,r2,r0 / lsrs r2,#16
    // / cmp r2,#1 / bhi. That is a plain (u16) truncation of `mode` -- NOT
    // `(u16)mode - 1`, which agbcc renders with a leading `subs r2,#1` that
    // displaced every later instruction (measured 8/60). The shift must be
    // spelled FIRST: agbcc reassociates `(u16)((u16)x + 0xFFFF)` into
    // `ldr 0xFFFF0000 / adds / lsls / lsrs`, hoisting the constant above the
    // shift. Proven shape: src/car_physics_core.c:134.
    u16 m = (u16)(((((u32)mode) << 16) + 0xFFFF0000u) >> 16);
    if (m > 1) return;
    // ROM holds ONE zero for all three clear stores: `movs r4,#0` at +0x14
    // feeds `strh r4,[r0]` (+144), `strh r4,[r5,#20]` and `str r4,[r5,#44]`.
    // The named local is what keeps agbcc on r4; three separate literal zeros
    // rematerialise `movs r0,#0` before the +44 store. The ASSIGNMENT lives
    // INSIDE the first store's expression (same lever as Rec35_Leaf_17414 in
    // rec35_runtime.c): as its own statement agbcc hoists `movs r4,#0` ABOVE
    // the `adds r0,r5,#0 / adds r0,#144` address pair, where the ROM
    // materialises it after.
    u32 z;
    *(volatile u16 *)((u8 *)r5 + 144) = (u16)(z = 0);
    _08002618(1, 0);
    _0802B368(1);
    *(volatile u32 *)((u8 *)r5 + 16) = 10;
    *(volatile u16 *)((u8 *)r5 + 20) = (u16)z;
    *(volatile u32 *)((u8 *)r5 + 48) = 10;
    *(volatile u32 *)((u8 *)r5 + 44) = z;
}
#ifndef __APPLE__
void _080016164(void *a, int b, int c) __attribute__((alias("Rec35_RecordClear")));
void sub_080016164(void *a, int b, int c) __attribute__((alias("Rec35_RecordClear")));
#endif

// ============================================================================
// sub_0800161A0 (0x080161A0, 0x50 B) — twin of 0x16164, plus:
//   [rec+26]=1; [rec+24]=1; WA+0x1084 = 1.
// ============================================================================
void Rec35_RecordClear2(void *rec, int unused, int mode) {
    (void)unused;
    u8 *r5 = (u8 *)rec;
    // Same (u16)-truncation gate as 0x08016164 -- see the note above.
    u16 m = (u16)(((((u32)mode) << 16) + 0xFFFF0000u) >> 16);
    if (m > 1) return;
    // ONE zero for all three clear stores, assigned inside the first store --
    // see the note in Rec35_RecordClear above.
    u32 z;
    *(volatile u16 *)((u8 *)r5 + 144) = (u16)(z = 0);
    _08002618(1, 0);
    _0802B368(1);
    *(volatile u32 *)((u8 *)r5 + 16) = 10;
    *(volatile u16 *)((u8 *)r5 + 20) = (u16)z;
    *(volatile u32 *)((u8 *)r5 + 48) = 10;
    *(volatile u32 *)((u8 *)r5 + 44) = z;
    // ROM keeps `1` live in r1 across all three stores and reaches WA+0x1084
    // as TWO pool words plus an `adds r0,r0,r2`, so the base must stay a
    // SYMBOL_REF. The `.globl` definition goes INSIDE the function, as in
    // src/ai_award_leaves.c (Ai_SetOwnedFlag): agbcc expands a file-scope
    // constant definition early enough that simplify_rtx folds it back into
    // one pool word 0x03002804.
    u16 one = 1;
    *(volatile u16 *)((u8 *)r5 + 26) = one;
    *(volatile u16 *)((u8 *)r5 + 24) = one;
    volatile u8 *wa = REC35_WA_BASE;
    __asm__(".globl Rec35Wa\nRec35Wa = 0x03001780\n");
    *(volatile u16 *)(wa + 0x1084u) = one;
}
#ifndef __APPLE__
void _0800161A0(void *a, int b, int c) __attribute__((alias("Rec35_RecordClear2")));
void sub_0800161A0(void *a, int b, int c) __attribute__((alias("Rec35_RecordClear2")));
#endif

// ============================================================================
// sub_0800161F0 (0x080161F0, 0x7C B) — 30-iteration course-record placement:
//   for i in 29..0 (off = i*8): mode==2 -> 07B18(rec+52, 7, off, arg1,
//   4, one, one, zero); mode==3 -> 07B18(rec+52, 7, off, arg1, 5, one,
//   one, zero).
//
// Two source shapes are load-bearing here, and neither is a width:
//   * the two `1` stack args and the `0` are hoisted into callee-saved
//     registers by the ROM (r5 = one, sl = zero), so they must be *named*
//     locals — passing the literals 1/1/0 straight through gives a
//     different allocation;
//   * the dispatch is a `switch`, not an `if/else if` chain. The `switch`
//     lowering emits `cmp/beq; cmp/beq; b end; body; b end; body; end`
//     (bodies out of line); `if/else if` emits `cmp/bne; body;...` with
//     the bodies in line. That single difference is 37 -> 124.
// The declaration order (R, a1, md, one, zero, off) is also load-bearing:
// it is what maps rec->r8, arg1->r7, mode->r6, one->r5, off->r4 as the
// ROM does. Reordering `off` before `zero` permutes r4/r5/sl.
// ============================================================================
void Rec35_PlaceRecords(void *rec, int arg1, int mode) {
    u8 *R = (u8 *)rec;
    int a1 = arg1;
    int md = mode;
    int one = 1;
    int zero = 0;
    int off = 0;
    for (int i = 29; i >= 0; i--) {
        switch (md) {
        case 2:
            sub_08007B18(R + 52, 7, off, a1, 4, one, one, zero);
            break;
        case 3:
            sub_08007B18(R + 52, 7, off, a1, 5, one, one, zero);
            break;
        default:
            break;
        }
        off += 8;
    }
}
#ifndef __APPLE__
void _0800161F0(void *a, int b, int c) __attribute__((alias("Rec35_PlaceRecords")));
void sub_0800161F0(void *a, int b, int c) __attribute__((alias("Rec35_PlaceRecords")));
#endif

// ============================================================================
// sub_08001626C (0x0801626C, 0x74 B) — table-driven emitter dispatch.
//   t = u16[0x080CB89C + s16[rec+140]*2];  m = u32[rec+152]
//   m==1 or m==3: 07B18(rec, t, arg1, arg2, 5, 1, 1, 0)
//   m==0 or m==2: 07B18(rec, t, arg1, arg2, 4, 1, 1, 0)
// ============================================================================
void Rec35_PlaceByTable(void *rec, int arg1, int arg2) {
    u16 *tbase = (u16 *)0x080CB89Cu;
    s16 idx = *(s16 *)((u8 *)rec + 140);
    u16 t = tbase[idx];
    int m = *(int *)((u8 *)rec + 152);
    switch (m) {
    case 1:
    case 3:
        sub_08007B18(rec, (int)t, arg1, arg2, 5, 1, 1, 0);
        break;
    case 0:
    case 2:
        sub_08007B18(rec, (int)t, arg1, arg2, 4, 1, 1, 0);
        break;
    }
}
#ifndef __APPLE__
void _08001626C(void *a, int b, int c) __attribute__((alias("Rec35_PlaceByTable")));
void sub_08001626C(void *a, int b, int c) __attribute__((alias("Rec35_PlaceByTable")));
#endif

// ============================================================================
// sub_0800162E0 (0x080162E0, 0xD4 B) — record-49 tail driver.
//   _0800D97C(rec+136, 15)
//   if (s16[rec+144] == 0):
//     if (s16[rec+142] == 0): 07B18(rec, 5, 0, 32, 3, 1, 1, 0)
//     if (s16[rec+142] == 1): 07B18(rec, 4, 0, 32, 3, 1, 1, 0)
//     (r1, r2) = (u32[0x080CB88C + s16[rec+140]*8 + 4], u32[rec+152]);
//     Rec35_PlaceRecords(rec, r1, r2)
//     (r1, r2) = (u32[0x080CB88C + s16[rec+140]*8], u32[...+4]);
//     Rec35_PlaceByTable(rec, r1, r2)
//   if (s16[rec+144] == 1):
//     sub_08007C68(rec+8, u32[rec+160], u32[rec+164], 48, 64, 6, 1, 0, 0)
//   sub_0800DBE8(rec+16); sub_0800163B4(rec)
// ============================================================================
void Rec35_Record49Tail(void *rec) {
    u8 *r7 = (u8 *)rec;
    Rec35_CountdownDec(r7 + 136, 15);
    if ((s16)*(volatile u16 *)(r7 + 144) == 0) {
        int sel = (s16)*(volatile u16 *)(r7 + 142);
        if (sel == 0) {
            sub_08007B18(r7, 5, 0, 32, 3, 1, 1, 0);
        } else if (sel == 1) {
            sub_08007B18(r7, 4, 0, 32, 3, 1, 1, 0);
        }
        u32 base = 0x080CB88Cu + (u32)((s16)*(volatile u16 *)(r7 + 140)) * 8;
        u32 r1a = *(volatile u32 *)(base + 4);
        u32 r2a = *(volatile u32 *)(r7 + 152);
        Rec35_PlaceRecords(r7, (int)r1a, (int)r2a);
        u32 r1b = *(volatile u32 *)(base + 0);
        u32 r2b = *(volatile u32 *)(base + 4);
        Rec35_PlaceByTable(r7, (int)r1b, (int)r2b);
    }
    if ((s16)*(volatile u16 *)(r7 + 144) == 1) {
        sub_08007C68(r7 + 8,
                     *(volatile u32 *)(r7 + 160),
                     *(volatile u32 *)(r7 + 164),
                     48, 64, 6, 1, 0, 0);
    }
    sub_0800DBE8(r7 + 16);
    Rec35_Noop(r7);
}
#ifndef __APPLE__
void _0800162E0(void *a) __attribute__((alias("Rec35_Record49Tail")));
void sub_0800162E0(void *a) __attribute__((alias("Rec35_Record49Tail")));
#endif

// ============================================================================
// sub_0800163B4 (0x080163B4, 0x4 B) — `bx lr` no-op stub.
// ============================================================================
void Rec35_Noop(void *rec) { (void)rec; }
// `bx lr` alone is 2 bytes; the ROM holds `00 00` in the two pad halfwords at
// 0x080163B6. gas closes a 2-mod-4 Thumb section with its own alignment NOP
// (0x46c0), so the file-scope `.align 2, 0` below pads with the ROM's zeros
// instead. No instruction changes.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800163B4(void *a) __attribute__((alias("Rec35_Noop")));
void sub_0800163B4(void *a) __attribute__((alias("Rec35_Noop")));
#endif
