// ============================================================================
// src/race_phase_vms.c — C lift of asm/carphys_racer_tail.s
// (0x08021C94–0x08022CB4, VMA 0x08021C94–0x08022CB4) plus the presence-test
// leaf _08026068 from asm/code_26068.s.
//
// The cluster is four parallel race-phase mini-VMs (constructor / event-
// handler / dispatcher / tick families) over the course-cursor grids at
// IWRAM 0x03001780+0xFF2/0xFF6/0x0FD0..0x0FD8/0x0FC2/0x1076/0x1078 and the
// ROM record tables 0x080CC078..0x080CC168. Transcribed instruction-faithful
// from the byte-exact asm; register names kept in comments where the mapping
// is non-obvious. armcc fall-through entries 0x08022240/0x08022438 inside
// 0x08022348's body are external BL targets (xref.py: 0x02239E→0x08022240,
// 0x02254C→0x08022438). The 0x08022C2C label is the tail dispatcher's in-body
// `mov pc,r0` instruction.
//
// Builder _08007B18 ABI (asm/course_resource_leaf_more.s): 4 reg args
// (rec, dst, idx, sel) + 3 stack words (a4..a6) read at [sp,#60/64/68]
// after its 0x38-byte frame; [sp,#56] (the 5th incoming word) is forwarded
// to _08002ED0 as its 1st arg. Builders _08007770 (4 reg + 2 stack words)
// and _0800DBE8 (r0 only: incoming r1 never read, all stack traffic local)
// follow the same AAPCS mapping.
// ============================================================================

#include "gtadv/foundation.h"

// ---- external callees (C lifts or asm trampolines) -------------------------
extern void *_08004B68(void);                       // car-record base getter
#ifndef __APPLE__
extern void *sub_08004B68(void);                     // closure spelling of _08004B68
#endif
extern void _0802B214(u32 vol);                     // sound master vol cond
extern void _0802B368(u16 vol);                     // sound deferred vol
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f); // 0x08007770 exact ROM body
extern void _0800DAB8(void *p);
extern void _08007614(void *a, int b, int c, int d);
extern void _0800798C(void *base, void *dst);
extern void _08007A58(void *p);
extern void _080075E8(void *base, int idx, int n);
extern void _0800D77C(void *a, int b, int c);
extern int  _08025D90(int type);                    // grid rows-full count
extern void _08007B18(void *rec, int dst, int idx, int sel, u32 a4, u32 a5, u32 a6, u32 a7);
extern void _0800D97C(void *rec, int reload);       // countdown cell dec
extern void _0800D854(void *rec);
extern void _0800D8E4(void *rec);
extern void _0800DBE8(void *rec);
extern void _0802E0A4(void *dst, const void *src, u32 n); // RuntimeMemcpy
extern void _08004BFC(int v);
extern void _08004EC0(int v);
extern int  _0800572C(int n);                       // save-alloc
extern void Gap_021C94_Select(void *ctx, void *rec);
// Call-site split. asm/save_alloc.s binds `sub_0800572C` at 0x0800572C (see the
// note in src/ai_award_leaves.c, which records the same rename), and a
// promoted body is spliced into a link where only the closure spelling is
// bound -- calling `_0800572C` here passes every per-function probe and then
// fails with `undefined reference`. The host build keeps the weak
// `_0800572C` stub below, so the split is mandatory.
#ifndef __APPLE__
#define PV_CALLEE(friendly, closure) closure
extern int sub_0800572C(int n);
extern void sub_080021C94(void *ctx, void *rec);
extern void sub_080021DC0(void *rec, int r1, u32 v);
extern void sub_08002205C(void *rec);
#else
#define PV_CALLEE(friendly, closure) friendly
#endif
extern void _08007664(void *a, void *b, int c);     // course resource setup
extern u8 RaceVM_WA[];

// ---- forward decls (this cluster) ------------------------------------------
void RaceVM_021CC0(void *rec);
void RaceVM_021CC4(void *rec);
void RaceVM_021DC0(void *rec, int r1, u32 v);
void RaceVM_021F14(void *rec, int x, int y);
void RaceVM_021F88(void *rec);
void RaceVM_02205C(void *rec);
void RaceVM_022240(void *ctx);
void RaceVM_02227C(void *a, int b);
void RaceVM_022280(void *rec);
void RaceVM_02231C(void *a);
void RaceVM_022320(void *ctx, int r1, u32 v);
void RaceVM_022348(void *a);
void RaceVM_02234C(u32 ev, u32 p1, u32 p2, void *p3);
u16  RaceVM_0223C0(int n);
void RaceVM_0223F0(void *rec);
int  RaceVM_022438(int x);
void RaceVM_022454(void *ctx, void *rec);
void RaceVM_022478(void *a);
void RaceVM_02247C(void *rec);
void RaceVM_022624(void *rec, int r1, u32 v);
void RaceVM_0226D4(void *rec, int r1, u32 v);
void RaceVM_022750(void *rec, int x, int y);
void RaceVM_0227C0(void *rec);
void RaceVM_021A4(u32 ev, u32 p1, u32 p2, void *p3);
void RaceVM_02285C(u32 ev, u32 p1, u32 p2, void *p3);
void RaceVM_0228F8(void *ctx, void *rec);
void RaceVM_02291C(void *rec);
void RaceVM_022920(void *rec);
void RaceVM_022A14(void *rec, int r1, u32 v);
void RaceVM_022ACC(void *rec, int r1, u32 v);
void RaceVM_022B48(void *rec, int x, int y);
#ifndef __APPLE__
void sub_080022B48(void *rec, int x, int y);
#endif
void RaceVM_022BBC(void *rec);
void RaceVM_022C18(u32 ev, u32 p1, u32 p2, void *p3);
int  RaceVM_026068(int i);

// ============================================================================
// code_26068.s _08026068 — presence test: u8[0x03001780 + 0x057C + i] == 1.
int RaceVM_026068(int i) {
    return *(volatile u8 *)(0x03001780u + 0x057Cu + (u32)i) == 1;
}
#ifndef __APPLE__
int _08026068(int a) __attribute__((alias("RaceVM_026068")));
int sub_08026068(int a) __attribute__((alias("RaceVM_026068")));
#endif

// ============================================================================
// sub_080021CC0 (0x080021CC0, 2 B) — `bx lr` no-op leaf (dispatcher slot 6).
void RaceVM_021CC0(void *rec) { (void)rec; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080021CC0(void *rec) __attribute__((alias("RaceVM_021CC0")));
void sub_080021CC0(void *rec) __attribute__((alias("RaceVM_021CC0")));
#endif

// ============================================================================
// sub_080021CC4 (0x080021CC4, 0xE8 B) — results constructor #1 (template
// 0x08376500): object builder + obj init, palette lanes, base/dst wiring,
// two D77C clears, place cells from WA+0xFF6/0xFF2.
void RaceVM_021CC4(void *rec) {
    volatile u8 *r7 = (volatile u8 *)rec;
    _0802B214(51);
    const u32 *r5 = (const u32 *)0x08376500;
    _08007770(0, (void *)r5, 4, 0, 4u, 1u);
    *(volatile u32 *)(r7 + 80) = 6;
    *(volatile u32 *)(r7 + 92) = 1;
    *(volatile u32 *)(r7 + 104) = 1;
    _0800DAB8((void *)(r7 + 8));
    const u32 *r4 = (const u32 *)0x082A798C;
    _08007614((void *)r4, 1, 0, 3);
    _08007614((void *)r4, 1, 1, 4);
    _0800798C((void *)r5, (void *)rec);
    _08007A58((void *)rec);
    _080075E8((void *)r5, 1, 6);
    _080075E8((void *)r5, 2, 7);
    _080075E8((void *)r5, 0, 8);
    _080075E8((void *)r5, 3, 5);
    // r4 = r7+160; [rec+120] = rec+160; [rec+124] = rec+8
    *(volatile u32 *)(r7 + 120) = (u32)(uintptr_t)(r7 + 160);
    *(volatile u32 *)(r7 + 124) = (u32)(uintptr_t)(r7 + 8);
    *(volatile u32 *)(r7 + 8) = 0;
    *(volatile u16 *)(r7 + 12) = 1;
    _0800D77C((void *)(r7 + 28), 0, -32);
    _0800D77C((void *)(r7 + 20), 0, 160);
    *(volatile u16 *)(r7 + 16) = 1;
    *(volatile u16 *)(r7 + 18) = 5;
    *(volatile u32 *)(r7 + 160) = 2;
    // Both loads are plain (non-volatile): a volatile s16 load becomes
    // ldrh+lsls+asrs, and a volatile u16 load is sunk into the store so the
    // address register is reused for the value.  The ROM has a real ldrsh
    // (ARMv4T needs a register index) issued *before* the store address, so
    // the u16 read has to stay in its own statement.
    volatile u8 *WA = (volatile u8 *)0x03001780;
    u16 v = *(u16 *)(WA + 0xFF6);
    *(volatile u16 *)(r7 + 156) = v;
    s16 ff2 = *(s16 *)(WA + 0xFF2);
    // Both loads are plain (non-volatile): a volatile s16 load becomes
    // ldrh+lsls+asrs, and a volatile u16 load is sunk into the store so the
    // address register is reused for the value.  The ROM has a real ldrsh
    // (ARMv4T needs a register index) issued *before* the store address, so
    // the u16 read has to stay in its own statement.
    *(volatile u16 *)(r7 + 158) = (u16)_08025D90(ff2);
}
#ifndef __APPLE__
void _080021CC4(void *a) __attribute__((alias("RaceVM_021CC4")));
void sub_080021CC4(void *a) __attribute__((alias("RaceVM_021CC4")));
#endif

// ============================================================================
// sub_080021DC0 (0x080021DC0, 0x154 B) — results event handler (v = phase
// bits): 2=enter/reset {+8=10,+12=0,+40=10,+36=0}, 1=commit (grid1 vote via
// 0x080CC120, WA+0xFF6 lookup, master-cell test at WA+0x0FD0+idx*2 → +16 =
// v or 6), 64/128 = counter dec/inc at rec+156, 32/16 = WA+0x0FD2 cursor
// resets + rows-full stamp at rec+158, sound 3 on cursor change; tail
// clamps rec+156 to [0, rec+158] and beeps 2 twice on any change.
void RaceVM_021DC0(void *rec, int r1, u32 v) {
    (void)r1;
    volatile u8 *r4 = (volatile u8 *)rec;
    volatile u8 *r6 = r4 + 156;
    u16 saved = *(volatile u16 *)r6;                    // mov r9, r0
    u16 r5 = (u16)v;
    if (r5 == 2) {
        _0802B368(4);
        *(volatile u32 *)(r4 + 8) = 10;
        *(volatile u16 *)(r4 + 12) = 0;
        *(volatile u32 *)(r4 + 40) = 10;
        *(volatile u32 *)(r4 + 36) = 0;
    }
    if (r5 == 1) {
        _0802B368(1);
        *(volatile u16 *)(r4 + 12) = 0;
        *(volatile u16 *)(r4 + 116) = (u16)r5;
        volatile u8 *WA = (volatile u8 *)0x03001780;
        volatile u16 *cellFF6 = (volatile u16 *)(WA + 0xFF6);
        s16 idx = *(volatile s16 *)r6;
        u16 tbl = *(const volatile u16 *)((const u8 *)0x080CC120 + ((u32)(s32)idx << 1));
        *cellFF6 = tbl;
        s16 c0 = (s16)*cellFF6;                          // ldrsh r1,[r0]
        s16 c1 = *(volatile s16 *)(WA + 0xFF2);          // ldrsh r0
        // addr = WA + 0x0FD0 + (c0<<1) + (c1<<3)  → s16 element c0 + c1*4
        s16 master = *(volatile s16 *)((volatile u8 *)WA + 0x0FD0u
                                       + ((u32)c0 << 1) + ((u32)c1 << 3));
        if (master == 0) {
            *(volatile u16 *)(r4 + 16) = (u16)r5;
        } else {
            *(volatile u16 *)(r4 + 16) = 6;
        }
    }
    if (r5 == 64) {
        u16 t = *(volatile u16 *)r6;
        *(volatile u16 *)r6 = (u16)(t - 1);
    }
    if (r5 == 128) {
        u16 t = *(volatile u16 *)r6;
        *(volatile u16 *)r6 = (u16)(t + 1);
    }
    volatile u8 *WA = (volatile u8 *)0x03001780;
    s16 m0 = *(volatile s16 *)(WA + 0x0FD0);
    volatile u16 *r4b = (volatile u16 *)(r4 + 158);      // adds r4, #158
    if (m0 != 0) {
        volatile u16 *cur = (volatile u16 *)(WA + 0x0FD2);
        u16 old = *cur;                                  // mov r8, r0
        if (r5 == 32) {
            *cur = 0;
            *(volatile u16 *)(WA + 0x0FD6) = 0;          // adds r3,#4
            *r4b = (u16)_08025D90(m0);
        }
        if (r5 == 16) {
            *cur = 1;
            *(volatile u16 *)(WA + 0x0FF6) = 0;
            *r4b = (u16)_08025D90(1);
        }
        if (old != *cur) {
            _0802B368(3);
        }
    }
    volatile u16 *r2 = (volatile u16 *)(r4 + 156);       // adds r2, r6
    if ((s16)*r2 < 0) {
        *r2 = 0;
    }
    if ((s16)*r2 > (s16)*r4b) {
        *r2 = *r4b;
    }
    s16 orig = (s16)saved;                               // asrs r4, r0, #16
    if (orig != (s16)*r2) {
        _0802B368(2);
        if (orig != (s16)*r2) {
            _0802B368(2);
        }
    }
}
#ifndef __APPLE__
void _080021DC0(void *a, int b, u32 c) __attribute__((alias("RaceVM_021DC0")));
void sub_080021DC0(void *a, int b, u32 c) __attribute__((alias("RaceVM_021DC0")));
#endif

// ============================================================================
// sub_080021F14 (0x080021F14, 0x6C B) — results page: place one row via
// _08007B18(rec, x, y, 0, a4, 1, 0); a4 = {mode==1||mode==3 ? 4 : 3} with
// mode = u32[rec+160], row sel = s16[rec+156] (dead-loads preserved in asm).
void RaceVM_021F14(void *rec, int x, int y) {
    volatile u8 *r3 = (volatile u8 *)rec;
    s16 cur = *(volatile s16 *)(r3 + 156);
    u32 mode = *(volatile u32 *)(r3 + 160);
    (void)cur; (void)mode; // loaded in asm, only mode selects a4
    u32 a4 = (mode == 1 || mode == 3) ? 4u : 3u;
    _08007B18(rec, (int)x, y, 0, a4, 1u, 0u, 0u);
}
#ifndef __APPLE__
void _080021F14(void *a, int b, int c) __attribute__((alias("RaceVM_021F14")));
void sub_080021F14(void *a, int b, int c) __attribute__((alias("RaceVM_021F14")));
#endif

// ============================================================================
// sub_080021F88 (0x080021F88, 0xD0 B) — results rows: for r6 in 0..3:
// when r6 == s16[rec+156], r7 = {mode==0→1, 1/2/3→2, 4→0, else→2} (jump
// table 0x08021FB8, slots {1,2,2,2,0}); sel = u16[0x080CC108 + (cur*3+r7)*2],
// (w0,w1) = u32 pair at 0x080CC0A8 + (cur*3+r7)*8; else sel/pair at index
// r6*3. Both arms: _08007B18(rec, w0, w1, 0, 6, 1, 0).
void RaceVM_021F88(void *rec) {
    volatile u8 *r5 = (volatile u8 *)rec;
    for (int r6 = 0; r6 <= 3; r6++) {
        volatile u8 *r2 = r5 + 156;
        s16 cur = *(volatile s16 *)r2;
        int r7;
        if (r6 == cur) {
            u32 mode = *(volatile u32 *)(r2 + 4);
            switch (mode) {
                case 0: r7 = 1; break;                  // 0x08021FD0
                case 1: case 2: case 3: r7 = 2; break;  // 0x08021FD4
                case 4: r7 = 0; break;                  // 0x08021FCC
                default: r7 = 2; break;                 // bhi default
            }
            u32 i = (u32)cur * 3u + (u32)r7;
            u16 sel = *(const volatile u16 *)((const u8 *)0x080CC108 + i * 2u);
            u32 w0 = *(const volatile u32 *)((const u8 *)0x080CC0A8 + i * 8u);
            u32 w1 = *(const volatile u32 *)((const u8 *)0x080CC0A8 + i * 8u + 4u);
            _08007B18(rec, (int)w0, (int)w1, (int)sel, 6u, 1u, 0u, 0u);
        } else {
            u32 i = (u32)r6 * 3u;
            u16 sel = *(const volatile u16 *)((const u8 *)0x080CC108 + i * 2u);
            u32 w0 = *(const volatile u32 *)((const u8 *)0x080CC0A8 + i * 8u);
            u32 w1 = *(const volatile u32 *)((const u8 *)0x080CC0A8 + i * 8u + 4u);
            _08007B18(rec, (int)w0, (int)w1, (int)sel, 6u, 1u, 0u, 0u);
        }
    }
}
#ifndef __APPLE__
void _080021F88(void *a) __attribute__((alias("RaceVM_021F88")));
void sub_080021F88(void *a) __attribute__((alias("RaceVM_021F88")));
#endif

// ============================================================================
// sub_08002205C (0x08002205C, 0x148 B) — results report refresh: dec
// s16[rec+152] (reload 15); when master WA+0x0FD0==1, page c1=WA+0x0FD2:
// c1==0 → rows (14,8),(6,64),(11,72) a4 {7,8,7}; c1==1 → (13,8),(12,64),
// (5,72) same a4 — all via _08007B18(rec, 8, y, x, a4, m0, c1). Then for
// rows 0..3 (num/img pairs 0x080CC078+r*8, gate u16 0x080CC0A0+r*2): when
// r > s16[rec+158] emit (5,1,0,0). Finally rows[s16[rec+156]] via 0x021F14,
// 0x021F88, and _0800DBE8(rec+8).
void RaceVM_02205C(void *rec) {
    volatile u8 *r7 = (volatile u8 *)rec;
    _0800D97C((void *)(r7 + 152), 15);
    volatile u8 *WA = (volatile u8 *)0x03001780;
    s16 m0 = *(volatile s16 *)(WA + 0x0FD0);
    if (m0 == 1) {
        s16 c1 = *(volatile s16 *)(WA + 0x0FD2);
        if (c1 == 0) {
            _08007B18(rec, 8, 14, 8, 7u, (u32)m0, (u32)c1, (u32)c1);
            _08007B18(rec, 8, 6, 64, 8u, (u32)m0, (u32)c1, (u32)c1);
            _08007B18(rec, 8, 11, 72, 7u, (u32)m0, (u32)c1, (u32)c1);
        } else if (c1 == 1) {
            _08007B18(rec, 8, 13, 8, 7u, (u32)c1, 0u, 0u);
            _08007B18(rec, 8, 12, 64, 8u, (u32)c1, 0u, 0u);
            _08007B18(rec, 8, 5, 72, 7u, (u32)c1, 0u, 0u);
        }
    }
    for (int r5 = 0; r5 <= 3; r5++) {
        u32 img = *(const volatile u32 *)((const u8 *)0x080CC078 + r5 * 8u + 4u);
        u16 gate = *(const volatile u16 *)((const u8 *)0x080CC0A0 + r5 * 2u);
        s16 cur = *(volatile s16 *)(r7 + 158);
        if (r5 > cur) {
            _08007B18(rec, (int)img, (int)gate, 0, 5u, 1u, 0u, 0u);
        }
    }
    s16 cur = *(volatile s16 *)(r7 + 156);
    u32 w0 = *(const volatile u32 *)((const u8 *)0x080CC078 + (u32)cur * 8u);
    u32 w1 = *(const volatile u32 *)((const u8 *)0x080CC078 + (u32)cur * 8u + 4u);
    RaceVM_021F14(rec, (int)w0, (int)w1);
    RaceVM_021F88(rec);
    _0800DBE8((void *)(r7 + 8));
}
#ifndef __APPLE__
void _08002205C(void *a) __attribute__((alias("RaceVM_02205C")));
void sub_08002205C(void *a) __attribute__((alias("RaceVM_02205C")));
#endif

// ============================================================================
// sub_080022240 (0x080022240, 0x3C B) — countdown leaf (BL target from the
// slot-6 arm of 0x08002234C @0x02239E): u16[+8] phase {0→(+10=200, 3),
// 1→(+12=1, 2), 3→dec +10 until 0, 2, else none}.
void RaceVM_022240(void *ctx) {
    volatile u8 *r1 = (volatile u8 *)ctx;
    u16 phase = *(volatile u16 *)(r1 + 8);
    switch (phase) {
        case 0:
            *(volatile u16 *)(r1 + 10) = 200;
            phase = 3;
            break;
        case 3: {
            int t = *(volatile u16 *)(r1 + 10) - 1;
            *(volatile u16 *)(r1 + 10) = (u16)t;
            if ((t << 16) > 0) return;
            phase = 1;
            break;
        }
        case 1:
            *(volatile u16 *)(r1 + 12) = 1;
            phase = 2;
            break;
        case 2:
            return;
        default:
            return;
    }
    *(volatile u16 *)(r1 + 8) = phase;
}
#ifndef __APPLE__
void _080022240(void *a) __attribute__((alias("RaceVM_022240")));
void sub_080022240(void *a) __attribute__((alias("RaceVM_022240")));
#endif

// ============================================================================
// sub_08002227C (0x08002227C, 2 B) — `bx lr` no-op leaf (dispatcher slot 2).
void RaceVM_02227C(void *a, int b) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08002227C(void *a, int b) __attribute__((alias("RaceVM_02227C")));
void sub_08002227C(void *a, int b) __attribute__((alias("RaceVM_02227C")));
#endif

// ============================================================================
// sub_080022280 (0x080022280, 0x94 B) — trophy constructor (template
// 0x0837BBB0): alloc 19, {+24:5}, lane (0,3), bind _08007ABC(u32[rec+4],
// u32[rec+24], u32[rec+20]), then _08007664(0, 0x0837BBB0, {m0==0→2,
// 1→3, 2→1, 3→4}); u16[rec+8]=0.
extern void _08007ABC(void *a, int b, int c);
void RaceVM_022280(void *rec) {
    volatile u8 *r4 = (volatile u8 *)rec;
    _0802B214(67);
    const u32 *r5 = (const u32 *)0x0837BBB0;
    _0800798C((void *)r5, (void *)rec);
    *(volatile u32 *)(r4 + 20) = (u32)PV_CALLEE(_0800572C, sub_0800572C)(19);
    *(volatile u32 *)(r4 + 24) = 5;
    _080075E8((void *)r5, 0, 3);
    _08007ABC((void *)(uintptr_t)*(volatile u32 *)(r4 + 4),
              (int)*(volatile u32 *)(r4 + 24),
              (int)*(volatile u32 *)(r4 + 20));
    // plain (non-volatile) read → a real ldrsh (a volatile s16 load is
    // lowered to ldrh+lsls+asrs).  One call site per arm, and the default
    // arm makes no call at all — that is what the ROM does.
    __asm__(".globl RaceVM_WA\nRaceVM_WA = 0x03001780\n");
    u32 base = (uintptr_t)RaceVM_WA;
    u16 off = 0x0FF6;
    s16 m0 = *(s16 *)(base + off);
    switch (m0) {                      // one call site per arm; default: no call
        case 0: _08007664((void *)0, (void *)r5, 2); break;
        case 1: _08007664((void *)0, (void *)r5, 3); break;
        case 2: _08007664((void *)0, (void *)r5, 1); break;
        case 3: _08007664((void *)0, (void *)r5, 4); break;
        default: break;
    }
    *(volatile u16 *)(r4 + 8) = 0;
}
// The body is 154 bytes, two short of the section's 4-byte alignment, so gas
// closes the section with a `nop` (0x46c0) where the ROM holds `00 00`. This
// file-scope `.align` lands after the body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument. No body byte changes.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080022280(void *a) __attribute__((alias("RaceVM_022280")));
void sub_080022280(void *a) __attribute__((alias("RaceVM_022280")));
#endif

// ============================================================================
// sub_08002231C (0x08002231C, 2 B) — `bx lr` no-op leaf (dispatcher slot 5,
// first half).
void RaceVM_02231C(void *a) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08002231C(void *a) __attribute__((alias("RaceVM_02231C")));
void sub_08002231C(void *a) __attribute__((alias("RaceVM_02231C")));
#endif

// ============================================================================
// sub_080022320 (0x080022320, 0x28 B) — mode commit gate: tag = u16[car+2];
// if (u16)(v-1) <= 1 → _08004BFC(14) + _08004EC0(1).
void RaceVM_022320(void *ctx, int r1, u32 v) {
    (void)ctx; (void)r1;
    u16 trunc = (u16)v;                  // lsls/lsrs issued *before* the call
#ifndef __APPLE__
    (void)sub_08004B68();                // closure spelling; car-record base read, unused
#else
    (void)_08004B68();                   // car-record base read; result unused
#endif
    u16 tag = trunc - 1;                 // subs/lsls/lsrs
    if (tag <= 1) {
        _08004BFC(14);
        _08004EC0(1);
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080022320(void *a, int b, u32 c) __attribute__((alias("RaceVM_022320")));
void sub_080022320(void *a, int b, u32 c) __attribute__((alias("RaceVM_022320")));
#endif

// ============================================================================
// sub_080022348 (0x080022348, 4 B) — `bx lr` no-op leaf sitting at the head
// of the dispatcher below (BL target of its slot-7 arm).
void RaceVM_022348(void *a) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080022348(void *a) __attribute__((alias("RaceVM_022348")));
void sub_080022348(void *a) __attribute__((alias("RaceVM_022348")));
#endif

// ============================================================================
// sub_08002234C (0x08002234C, 0x70 B) — 7-way event dispatcher (1-based
// table at 0x08022368, pool base 0x08022364; slots 3/4 = exit): 1→022280
// (trophy ctor), 2→02227C (noop), 5→02231C+022240 (countdown), 6→022320
// (p3, (u16)p1, (u16)p2), 7→022348 leaf (no-op).
void RaceVM_02234C(u32 ev, u32 p1, u32 p2, void *p3) {
    if (ev == 0 || ev > 7) return;
    switch (ev) {
        // armcc emitted the case bodies in source order 2, 1, 5, 7, 6 —
        // match that layout, it is what fixes every table entry address.
        case 2: RaceVM_02227C(p3, (int)p1); break;
        case 1:
#ifndef __APPLE__
            _080022280(p3);             // closure spelling
#else
            RaceVM_022280(p3);
#endif
            break;
        case 5:
            RaceVM_02231C(p3);
#ifndef __APPLE__
            _080022240(p3);             // closure spelling
#else
            RaceVM_022240(p3);
#endif
            break;
        case 7: RaceVM_022348(p3); break;
        case 6:
#ifndef __APPLE__
            _080022320(p3, (int)(u16)p1, (u16)p2);   // closure spelling
#else
            RaceVM_022320(p3, (int)(u16)p1, (u16)p2);
#endif
            break;
        default: break;
    }
}
#ifndef __APPLE__
void _08002234C(u32 a, u32 b, u32 c, void *d) __attribute__((alias("RaceVM_02234C")));
void sub_08002234C(u32 a, u32 b, u32 c, void *d) __attribute__((alias("RaceVM_02234C")));
#endif

// ============================================================================
// sub_0800223C0 (0x0800223C0, 0x30 B) — stack template u16[3] from ROM
// 0x0805FC64 (RuntimeMemcpy 6 B); index clamp n to [0,2]; return u16[n].
u16 RaceVM_0223C0(int n) {
    u32 tmp[2];   // 8-byte frame keeps the memcpy 4-aligned like sp+8
    _0802E0A4(tmp, (const void *)0x0805FC64, 6);
    int r4 = n;
    if (r4 > 1) r4 = 2;
    if (r4 <= 0) r4 = 0;
    return ((volatile u16 *)tmp)[r4];
}
#ifndef __APPLE__
u16 _0800223C0(int a) __attribute__((alias("RaceVM_0223C0")));
u16 sub_0800223C0(int a) __attribute__((alias("RaceVM_0223C0")));
#endif

// ============================================================================
// sub_0800223F0 (0x0800223F0, 0x34 B) — results row collection: rows
// u16[rec+186..] = i where _08026068(i)==1 (i in 0..2); count-1 (clamped ≥0)
// → u16[rec+184].
void RaceVM_0223F0(void *rec) {
    volatile u8 *r7 = (volatile u8 *)rec;
    int r6 = 0;                                         // movs r6, #0
    int r4 = 0;                                         // movs r4, #0
    volatile u16 *out = (volatile u16 *)(r7 + 186);     // adds r5, r7 / adds r5, #186
    while (r4 <= 2) {
        if (_08026068(r4) == 1) {
            *out++ = (u16)r4;
            r6++;
        }
        r4++;
    }
    int v = r6 - 1;
    volatile u16 *cnt = (volatile u16 *)(r7 + 184);
    *cnt = (u16)v;
    if ((s32)((u32)v << 16) <= 0) {
        *cnt = 0;
    }
}
// The body is 58 bytes, two short of the section's 4-byte alignment. Under
// -ffunction-sections gas closes the section itself and fills a Thumb code
// section with `nop` (0x46c0); the ROM holds `00 00`. This file-scope
// `.align` is emitted after this body's `.size` -- still inside its own
// section -- so it pads with the `0` fill argument instead. No body byte
// changes; only the two filler halfwords.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800223F0(void *a) __attribute__((alias("RaceVM_0223F0")));
void sub_0800223F0(void *a) __attribute__((alias("RaceVM_0223F0")));
#endif

// ============================================================================
// sub_08002242C (0x08002242C, 0x0C B) — leaf sitting between _0800223F0 and
// _080022438: `lsls r1,r1,#1; adds r0,#186; adds r0,r0,r1; movs r1,#0;
// ldrsh r0,[r0,r1]; bx lr`, i.e. s16[rec + 186/2 + i]. It has no BL caller in
// the closure (tools/xref.py), so it only became visible as a function start
// once it was named — an unnamed entry is discoverable in the ROM but not
// liftable, and the probe requires a C candidate for every named start.
//
// The one-expression form was 12 B of `ldrh`+`lsls`+`asrs` against the ROM's
// single `ldrsh`, with the base in the wrong register. EXACT needs the ROM's
// *build* order: declare the scaled index first, then the base, and add the
// base as the FIRST operand so agbcc writes the add's destination into r0
// (rec) rather than r1 (index) — the same first-operand rule as the
// _080256BC functions.
s16 RaceVM_02242C(void *rec, int i) {
    int off = i * 2;
    u8 *base = (u8 *)rec + 186;
    return *(s16 *)(base + off);
}
#ifndef __APPLE__
s16 _08002242C(void *rec, int i) __attribute__((alias("RaceVM_02242C")));
s16 sub_08002242C(void *rec, int i) __attribute__((alias("RaceVM_02242C")));
#endif

// ============================================================================
// sub_080022438 (0x080022438, 0x1C B incl. the `00 00` align filler) — BL
// target from 0x0802254C: map {95→1, 96→2, else 0}.
//
// The ROM keeps a DECISION TREE, not the obvious if/else-if chain:
//
//     movs r1,#0 / cmp #95 / beq A / cmp #95 / ble END
//     cmp #96 / beq B / b END
//     A: movs r1,#1 / b END
//     B: movs r1,#2
//     END: adds r0,r1,#0 / bx lr
//
// The redundant-looking middle `cmp #95 / ble END` (x<=95 is already covered
// by the `beq A` miss) is the low-side test of a switch tree, and both case
// arms stay OUT OF LINE with their own trailing `b END`. agbcc's if/else
// expansion inverts the first test to `bne` and threads the arms inline
// (measured 6/28), and a 2-case `switch` emits the right layout but drops the
// low-side node (24 B). The `case 94: break;` label is what makes agbcc build
// the three-node tree; it contributes no code of its own.
int RaceVM_022438(int x) {
    int r1 = 0;
    switch (x) {
    case 94: break;
    case 95: r1 = 1; break;
    case 96: r1 = 2; break;
    }
    return r1;
}
// 26 B of body, two short of the section's 4-byte alignment; the ROM holds
// `00 00` where gas would fill a Thumb code section with `46 c0`. Same
// file-scope zero-fill trick as RaceVM_0223F0 above.
__asm__(".align 2, 0");
#ifndef __APPLE__
int _080022438(int a) __attribute__((alias("RaceVM_022438")));
int sub_080022438(int a) __attribute__((alias("RaceVM_022438")));
#endif

// ============================================================================
// sub_080022454 (0x080022454, 0x20 B) — place setter: u16[rec+84] =
// (u16[car+2]==44) ? 5 : 6.
void RaceVM_022454(void *ctx, void *rec) {
    (void)ctx;
    volatile u16 *hw = (volatile u16 *)_08004B68();
    volatile u8 *r4 = (volatile u8 *)rec;
    if (hw[1] == 44)
        *(volatile u16 *)(r4 + 84) = 5;
    else
        *(volatile u16 *)(r4 + 84) = 6;
}
#ifndef __APPLE__
void _080022454(void *a, void *b) __attribute__((alias("RaceVM_022454")));
void sub_080022454(void *a, void *b) __attribute__((alias("RaceVM_022454")));
#endif

// ============================================================================
// sub_080022478 (0x080022478, 2 B) — `bx lr` no-op leaf (dispatcher slot 12).
void RaceVM_022478(void *a) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080022478(void *a) __attribute__((alias("RaceVM_022478")));
void sub_080022478(void *a) __attribute__((alias("RaceVM_022478")));
#endif

// ============================================================================
// sub_08002247C (0x08002247C, 0x158 B) — results constructor #2 (template
// 0x0839CFE0): [0x030027F6]=0; object builder (1 lane); {+80:6, +92:20,
// +104:1}; obj init rec+8; palette lanes {d=4,e=5}; base/dst bind; lane
// (0,6); [rec+120]=rec+196, [rec+124]=rec+8, [rec+8]=0, [rec+12]=1;
// D77C(rec+28, 0, -32) + D77C(rec+20, 0, 160); {+16:6, +18:5};
// 0x0223F0(rec); u32[rec+196]=2; [0x030027F6]=0; map u16[0x03001780+0xFC2]
// → u16[rec+132]; memcpy 24 B from 0x0805FC6C to the stack words sp+16
// (dead per-row defaults, preserved by the asm as loaded live registers);
// for col 0..2: rows u16[rec+132+col*4] = id or 8, u16[rec+136+col*4] =
// tag or 9, u32[rec+152+col*8] = word pair from 0x080CC140+id*6 (ldr pair)
// or 88, u32[rec+156+col*8] = sp+20 words or u32[rec+196].
void RaceVM_02247C(void *rec) {
    volatile u8 *r7 = (volatile u8 *)rec;
    volatile u8 *WA = (volatile u8 *)0x03001780;
    *(volatile u16 *)(WA + 0x1078) = 0;                  // mov r9 = 0 store
    _0802B214(51);
    const u32 *r5 = (const u32 *)0x0839CFE0;
    _08007770(0, (void *)r5, 1, 0, 4u, 1u);
    *(volatile u32 *)(r7 + 80) = 6;
    *(volatile u32 *)(r7 + 92) = 20;
    *(volatile u32 *)(r7 + 104) = 1;
    _0800DAB8((void *)(r7 + 8));
    const u32 *r4 = (const u32 *)0x082A798C;
    _08007614((void *)r4, 1, 0, 4);
    _08007614((void *)r4, 1, 1, 5);
    _0800798C((void *)r5, (void *)rec);
    _08007A58((void *)rec);
    *(volatile u32 *)(r7 + 120) = (u32)(uintptr_t)(r7 + 196);
    *(volatile u32 *)(r7 + 124) = (u32)(uintptr_t)(r7 + 8);
    *(volatile u32 *)(r7 + 8) = 0;
    *(volatile u16 *)(r7 + 12) = 1;
    _0800D77C((void *)(r7 + 28), 0, -32);
    _0800D77C((void *)(r7 + 20), 0, 160);
    *(volatile u16 *)(r7 + 16) = 6;
    *(volatile u16 *)(r7 + 18) = 5;
    RaceVM_0223F0(rec);
    *(volatile u32 *)(r7 + 196) = 2;
    *(volatile u16 *)(WA + 0x1076) = 0;
    s16 variant = *(volatile s16 *)(WA + 0x0FC2);
    *(volatile u16 *)(r7 + 132) = (u16)RaceVM_022438((int)variant);
    // dead stack defaults: 12-byte template 0x0805FC64 → sp+8, 24-byte
    // table 0x0805FC6C → sp+16 (both read by the loop's fallback arms).
    u32 tmplA[3], tmplB[6];
    _0802E0A4(tmplA, (const void *)0x0805FC64, 6);
    _0802E0A4(tmplB, (const void *)0x0805FC6C, 24);
    (void)tmplA;
    u32 rowdef[3] = { tmplB[0], tmplB[1], tmplB[2] };    // sp+16.. sp+27
    u32 gatedef[3] = { tmplB[3], tmplB[4], tmplB[5] };   // sp+20.. sp+31
    (void)rowdef;
    for (int col = 0; col <= 2; col++) {
        int id = _08026068(col);
        volatile u16 *p_id = (volatile u16 *)(r7 + 132 + col * 4);
        volatile u16 *p_tag = (volatile u16 *)(r7 + 136 + col * 4);
        volatile u32 *p_num = (volatile u32 *)(r7 + 152 + col * 8);
        volatile u32 *p_gate = (volatile u32 *)(r7 + 156 + col * 8);
        if (id != 0) {
            u32 base = 0x080CC140u + (u32)(col * 2);
            *p_id = *(const volatile u16 *)base;
            *p_tag = *(const volatile u16 *)(base + 6u);
            *p_num = *(const volatile u32 *)base;
            *p_gate = gatedef[col];
        } else {
            *p_id = 8;
            *p_tag = 9;
            *p_num = 88;
            *p_gate = *(volatile u32 *)(rowdef + (u32)col);
        }
    }
}
#ifndef __APPLE__
void _08002247C(void *a) __attribute__((alias("RaceVM_02247C")));
void sub_08002247C(void *a) __attribute__((alias("RaceVM_02247C")));
#endif

// ============================================================================
// sub_080022624 (0x080022624, 0xAC B) — results event handler #2 (v = phase
// bits): 2 = enter/reset {+8=10,+12=0,+40=10,+36=0}; 1 = commit when
// _08026068(s16[rec+132])==1: place word via 0x0223C0 → u16[0x03001780+
// 0xFC2], sound 1, u32[rec+196]=v, u16[rec+116]=0 (rec+196-80), u16[rec+12]=v
// — wait: strh r4(0) [rec+12], u32[rec+36]=v; sound 10 otherwise; 64/128 =
// counter dec/inc; clamp [0,2]; sound 2 on change.
void RaceVM_022624(void *rec, int r1, u32 v) {
    (void)r1;
    u8 *r5 = (u8 *)rec;
    u16 r6 = (u16)v;
    u8 *r4 = r5 + 132;
    u16 saved = *(u16 *)r4;
    if (r6 == 2) {
        _0802B368(4);
        u32 ten = 10;
        *(volatile u32 *)(r5 + 8) = ten;
        u16 zero = 0;
        *(u16 *)(r5 + 12) = zero;
        *(volatile u32 *)(r5 + 40) = ten;
        *(volatile u32 *)(r5 + 36) = zero;
    }
    if (r6 == 1) {
        s16 cur = *(s16 *)r4;
        if (_08026068(cur) == 1) {
            u16 place = RaceVM_0223C0(cur);
            __asm__(".globl RaceVM_WA\nRaceVM_WA = 0x03001780\n");
            u32 wa = (uintptr_t)RaceVM_WA;
            u16 cell = 0x0FC2;
            *(u16 *)(wa + cell) = place;
            _0802B368(1);
            *(volatile u32 *)(r5 + 196) = (u32)r6;
            *(u16 *)(r5 + 12) = 0;                      // strh r4 (0)
            *(u16 *)(r5 + 116) = (u16)r6;               // rec+196-80
            *(volatile u32 *)(r5 + 36) = (u32)r6;
        } else {
            _0802B368(10);
        }
    }
    // The ROM recomputes the cell pointer here (r2), after the r4 value was
    // consumed, and clamps through a copy in r1 with a zero index register.
    u8 *r2 = r5 + 132;
    if (r6 == 64) {
        u16 t = *(u16 *)r2;
        *(u16 *)r2 = (u16)(t - 1);
    }
    if (r6 == 128) {
        u16 t = *(u16 *)r2;
        *(u16 *)r2 = (u16)(t + 1);
    }
    // named `p1`, not `r1`: this function's second parameter is already `r1`,
    // and a local with the same name is a hard redeclaration error.
    u8 *p1 = r2;
    if (*(s16 *)p1 < 0) *(u16 *)p1 = 0;
    if (*(s16 *)p1 > 2) *(u16 *)p1 = 2;
    if (saved != *(u16 *)r2) {
        _0802B368(2);
    }
}
#ifndef __APPLE__
void _080022624(void *a, int b, u32 c) __attribute__((alias("RaceVM_022624")));
void sub_080022624(void *a, int b, u32 c) __attribute__((alias("RaceVM_022624")));
#endif

// ============================================================================
// sub_0800226D4 (0x0800226D4, 0x7C B) — rows 0..29 builder: id 2 → a4=4,
// id 3 → a4=5; _08007B18(rec+44, 7, r5, r1, a4, 1, 0, 0).
void RaceVM_0226D4(void *rec_, int r1_, u32 v) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 r7 = (u32)r1_;
    u32 r6 = v;
    u32 one = 1;
    u32 zero = 0;
    u32 r5 = 0;
    int r9 = 29;
    for (;;) {
        switch (r6) {
        case 2:
            _08007B18((void *)(uintptr_t)(rec + 44), 7, (int)r5, (int)r7,
                      4, (int)one, (int)zero, (int)zero);
            break;
        case 3:
            _08007B18((void *)(uintptr_t)(rec + 44), 7, (int)r5, (int)r7,
                      5, (int)one, (int)zero, (int)zero);
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
void _0800226D4(void *a, int b, u32 c) __attribute__((alias("RaceVM_0226D4")));
void sub_0800226D4(void *a, int b, u32 c) __attribute__((alias("RaceVM_0226D4")));
#endif

// ============================================================================
// sub_080022750 (0x080022750, 0x6C B) — results page twin of 0x021F14:
// sel = s16[rec+132], mode = u32[rec+196]; a4 = {1||3 ? 5 : 4}.
void RaceVM_022750(void *rec, int x, int y) {
    volatile u8 *r3 = (volatile u8 *)rec;
    s16 cur = *(volatile s16 *)(r3 + 132);
    u32 mode = *(volatile u32 *)(r3 + 196);
    (void)cur;
    u32 a4 = (mode == 1 || mode == 3) ? 5u : 4u;
    _08007B18(rec, (int)x, y, 0, a4, 1u, 0u, 0u);
}
#ifndef __APPLE__
void _080022750(void *a, int b, int c) __attribute__((alias("RaceVM_022750")));
void sub_080022750(void *a, int b, int c) __attribute__((alias("RaceVM_022750")));
#endif

// ============================================================================
// sub_0800227C0 (0x0800227C0, 0x94 B) — paint: dec s16[rec+128] (reload 15);
// rows 0..2: (w1,w3) = u32 pair at rec+152+row*8 / rec+156+row*8 →
// _08007B18(rec, w1, w3, 0, 6, 0, 0); then rows[s16[rec+132]] via 0x0226D4 +
// 0x022750; _0800DBE8(rec+8).
void RaceVM_0227C0(void *rec) {
    volatile u8 *r5 = (volatile u8 *)rec;
    _0800D97C((void *)(r5 + 128), 15);
    for (int r4 = 0; r4 <= 2; r4++) {
        u32 w1 = *(volatile u32 *)(r5 + 152 + (u32)r4 * 8u);
        u32 w3 = *(volatile u32 *)(r5 + 156 + (u32)r4 * 8u);
        _08007B18(rec, (int)w1, (int)w3, 0, 6u, 0u, 0u, 0u);
    }
    s16 cur = *(volatile s16 *)(r5 + 132);
    u32 w1 = *(volatile u32 *)(r5 + 152 + (u32)cur * 8u);
    u32 w3 = *(volatile u32 *)(r5 + 156 + (u32)cur * 8u);
    RaceVM_0226D4(rec, (int)w1, w3);
    RaceVM_022750(rec, (int)w1, (int)w3);
    _0800DBE8((void *)(r5 + 8));
}
#ifndef __APPLE__
void _0800227C0(void *a) __attribute__((alias("RaceVM_0227C0")));
void sub_0800227C0(void *a) __attribute__((alias("RaceVM_0227C0")));
#endif

// ============================================================================
// _0800221A4 — parallel 12-way phase dispatcher (1-based table): 1->021CC4,
// 2->021C94, 5->D854+D8E4, 6->u16[rec+12]!=0 ->021DC0, 7->02205C,
// 12->021CC0. Slots 3/4/8/9/10/11 exit.
void RaceVM_021A4(u32 ev, u32 p1, u32 p2, void *p3) {
    if (ev == 0 || ev > 12) return;
    switch (ev) {
        // Keep the armcc case layout: 2, 5, 7, 6, 1, 12.
        case 2: PV_CALLEE(Gap_021C94_Select, sub_080021C94)(p3, (void *)(uintptr_t)p1); break;
        case 5: _0800D854((u8 *)p3 + 8); _0800D8E4((u8 *)p3 + 112); break;
        case 7: PV_CALLEE(RaceVM_02205C, sub_08002205C)(p3); break;
        case 6:
            if (*(volatile u16 *)((u8 *)p3 + 12) != 0)
                PV_CALLEE(RaceVM_021DC0, sub_080021DC0)(p3, (int)(u16)p1, (u16)p2);
            break;
        case 1: RaceVM_021CC4(p3); break;
        case 12: RaceVM_021CC0(p3); break;
        default: break;
    }
}
// The ROM pads this dispatcher to its next 4-byte boundary with 0x0000.
// agbcc's function section uses 0x46C0 by default, so preserve the literal
// Thumb alignment halfword in this section.
__asm__(".pushsection .text.RaceVM_021A4,\"ax\",%progbits\n\t.align 2, 0\n\t.popsection");
#ifndef __APPLE__
void _0800221A4(u32 ev, u32 p1, u32 p2, void *p3) __attribute__((alias("RaceVM_021A4")));
#endif

// ============================================================================
// sub_08002285C (0x08002285C, 0x98 B) — 12-way event dispatcher (1-based
// table at 0x08022878, pool base 0x08022874; slots 3/4/8/9/10/11 = exit):
// 1→02247C (ctor), 2→022454 (setter), 5→D854+D8E4, 6→u16[+12]!=0 → 022624,
// 7→0227C0 (paint), 12→022478 (noop).
void RaceVM_02285C(u32 ev, u32 p1, u32 p2, void *p3) {
    if (ev == 0 || ev > 12) return;
    switch (ev) {
        // armcc emitted the case bodies in source order 2, 5, 7, 6, 1, 12 —
        // that layout is what fixes every jump-table entry address.
        case 2:
#ifndef __APPLE__
            _080022454((void *)p3, (void *)(uintptr_t)p1);    // closure spelling
#else
            RaceVM_022454((void *)p3, (void *)(uintptr_t)p1);
#endif
            break;
        case 5: _0800D854((u8 *)p3 + 8); _0800D8E4((u8 *)p3 + 112); break;
        case 7:
#ifndef __APPLE__
            _0800227C0(p3);             // closure spelling
#else
            RaceVM_0227C0(p3);
#endif
            break;
        case 6:
            if (*(volatile u16 *)((u8 *)p3 + 12) != 0) {
#ifndef __APPLE__
                _080022624(p3, (int)(u16)p1, (u16)p2);       // closure spelling
#else
                RaceVM_022624(p3, (int)(u16)p1, (u16)p2);
#endif
            }
            break;
        case 1:
#ifndef __APPLE__
            _08002247C(p3);             // closure spelling
#else
            RaceVM_02247C(p3);
#endif
            break;
        case 12: RaceVM_022478(p3); break;
        default: break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08002285C(u32 a, u32 b, u32 c, void *d) __attribute__((alias("RaceVM_02285C")));
void sub_08002285C(u32 a, u32 b, u32 c, void *d) __attribute__((alias("RaceVM_02285C")));
#endif

// ============================================================================
// sub_0800228F8 (0x0800228F8, 0x20 B) — place setter twin: u16[rec+84] =
// (u16[car+2]==15) ? 5 : 6.
void RaceVM_0228F8(void *ctx, void *rec) {
    (void)ctx;
    volatile u16 *hw = (volatile u16 *)_08004B68();
    volatile u8 *r4 = (volatile u8 *)rec;
    if (hw[1] == 15)
        *(volatile u16 *)(r4 + 84) = 5;
    else
        *(volatile u16 *)(r4 + 84) = 6;
}
#ifndef __APPLE__
void _0800228F8(void *a, void *b) __attribute__((alias("RaceVM_0228F8")));
void sub_0800228F8(void *a, void *b) __attribute__((alias("RaceVM_0228F8")));
#endif

// ============================================================================
// sub_08002291C (0x08002291C, 2 B) — `bx lr` no-op leaf (tail slot 12).
void RaceVM_02291C(void *rec) { (void)rec; }
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08002291C(void *rec) __attribute__((alias("RaceVM_02291C")));
void sub_08002291C(void *rec) __attribute__((alias("RaceVM_02291C")));
#endif

// ============================================================================
// sub_080022920 (0x080022920, 0xE4 B) — results constructor #3 (template
// 0x0839E9C0): variant = s16[WA+0x1078]; [0x030027F6]=0; object builder
// (0 lanes); {+80:6, +92:20, +104:1}; obj init; palette lanes {d=4,e=5};
// base/dst; [rec+120]=rec+136, [rec+124]=rec+8, [rec+8]=0, [rec+12]=1;
// D77C x2; {+16:6, +18:5}; u32[rec+136]=2; [0x030027F6]=0; u16[rec+132] =
// (variant==2) ? 1 : 0.
void RaceVM_022920(void *rec) {
    volatile u8 *r7 = (volatile u8 *)rec;
    volatile u8 *WA = (volatile u8 *)0x03001780;
    volatile u16 *cell = (volatile u16 *)(WA + 0x1078);
    s16 variant = *(volatile s16 *)cell;                 // mov r9, r0
    _0802B214(51);
    *cell = 0;
    const u32 *r8 = (const u32 *)0x0839E9C0;
    _08007770(0, (void *)r8, 0, 0, 4u, 1u);
    *(volatile u32 *)(r7 + 80) = 6;
    *(volatile u32 *)(r7 + 92) = 20;
    *(volatile u32 *)(r7 + 104) = 1;
    _0800DAB8((void *)(r7 + 8));
    const u32 *r4 = (const u32 *)0x082A798C;
    _08007614((void *)r4, 1, 0, 4);
    _08007614((void *)r4, 1, 1, 5);
    _0800798C((void *)r8, (void *)rec);
    _08007A58((void *)rec);
    *(volatile u32 *)(r7 + 120) = (u32)(uintptr_t)(r7 + 136);
    *(volatile u32 *)(r7 + 124) = (u32)(uintptr_t)(r7 + 8);
    *(volatile u32 *)(r7 + 8) = 0;
    *(volatile u16 *)(r7 + 12) = 1;
    _0800D77C((void *)(r7 + 28), 0, -32);
    _0800D77C((void *)(r7 + 20), 0, 160);
    *(volatile u16 *)(r7 + 16) = 6;
    *(volatile u16 *)(r7 + 18) = 5;
    *(volatile u32 *)(r7 + 136) = 2;
    *(volatile u16 *)0x030027F6 = 0;
    if (variant == 2) {
        *(volatile u16 *)(r7 + 132) = 1;
    } else {
        *(volatile u16 *)(r7 + 132) = 0;
    }
}
#ifndef __APPLE__
void _080022920(void *a) __attribute__((alias("RaceVM_022920")));
void sub_080022920(void *a) __attribute__((alias("RaceVM_022920")));
#endif

// ============================================================================
// sub_080022A14 (0x080022A14, 0xB4 B) — variant event handler (v = phase
// bits): 2 = enter/reset; 1 = commit (variant cell select: 0→cell=v,
// 1→cell=2; sound 1; u32[rec+136]=1; u16[rec+12]=0; u16[rec+116]=1;
// u32[rec+36]=1); 64/128 dec/inc; clamp [0,1]; sound 2 on change.
void RaceVM_022A14(void *rec, int r1, u32 v) {
    (void)r1;
    volatile u8 *r4 = (volatile u8 *)rec;
    volatile u8 *r6 = r4 + 132;
    u16 saved = *(volatile u16 *)r6;
    u16 r5 = (u16)v;
    if (r5 == 2) {
        _0802B368(4);
        *(volatile u32 *)(r4 + 8) = 10;
        *(volatile u16 *)(r4 + 12) = 0;
        *(volatile u32 *)(r4 + 40) = 10;
        *(volatile u32 *)(r4 + 36) = 0;
    }
    if (r5 == 1) {
        s16 cur = *(volatile s16 *)r6;
        if (cur == 0) {
            *(volatile u16 *)(0x03001780u + 0x1078u) = (u16)r5;
        } else if (cur == 1) {
            *(volatile u16 *)(0x03001780u + 0x1078u) = 2;
        }
        _0802B368(1);
        *(volatile u32 *)(r4 + 136) = 1;
        *(volatile u16 *)(r4 + 12) = 0;
        *(volatile u16 *)(r4 + 116) = 1;
        *(volatile u32 *)(r4 + 36) = 1;
    }
    if (r5 == 64) {
        u16 t = *(volatile u16 *)r6;
        *(volatile u16 *)r6 = (u16)(t - 1);
    }
    if (r5 == 128) {
        u16 t = *(volatile u16 *)r6;
        *(volatile u16 *)r6 = (u16)(t + 1);
    }
    if ((s16)*(volatile u16 *)r6 < 0) *r6 = 0;
    if ((s16)*(volatile u16 *)r6 > 1) *r6 = 1;
    if (saved != *(volatile u16 *)r6) {
        _0802B368(2);
    }
}
#ifndef __APPLE__
void _080022A14(void *a, int b, u32 c) __attribute__((alias("RaceVM_022A14")));
void sub_080022A14(void *a, int b, u32 c) __attribute__((alias("RaceVM_022A14")));
#endif

// ============================================================================
// sub_080022ACC (0x080022ACC, 0x7C B) — rows 0..29 builder twin of 0x0226D4:
// byte-identical ROM span, only the `bl` displacement differs, so it is the
// same MenuFF78_12A44 recipe body (see the note above it).
void RaceVM_022ACC(void *rec_, int r1_, u32 v) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u32 r7 = (u32)r1_;
    u32 r6 = v;
    u32 one = 1;
    u32 zero = 0;
    u32 r5 = 0;
    int r9 = 29;
    for (;;) {
        switch (r6) {
        case 2:
            _08007B18((void *)(uintptr_t)(rec + 44), 7, (int)r5, (int)r7,
                      4, (int)one, (int)zero, (int)zero);
            break;
        case 3:
            _08007B18((void *)(uintptr_t)(rec + 44), 7, (int)r5, (int)r7,
                      5, (int)one, (int)zero, (int)zero);
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
void _080022ACC(void *a, int b, u32 c) __attribute__((alias("RaceVM_022ACC")));
void sub_080022ACC(void *a, int b, u32 c) __attribute__((alias("RaceVM_022ACC")));
#endif

// ============================================================================
// sub_080022B48 (0x080022B48, 0x6C B) — page twin: sel = s16[rec+132],
// mode = u32[rec+136]; a4 = {1||3 ? 5 : 4}.
void RaceVM_022B48(void *rec, int x, int y) {
    volatile u8 *r3 = (volatile u8 *)rec;
    s16 cur = *(volatile s16 *)(r3 + 132);
    u32 mode = *(volatile u32 *)(r3 + 136);
    (void)cur;
    u32 a4 = (mode == 1 || mode == 3) ? 5u : 4u;
    _08007B18(rec, (int)x, y, 0, a4, 1u, 0u, 0u);
}
#ifndef __APPLE__
void _080022B48(void *a, int b, int c) __attribute__((alias("RaceVM_022B48")));
void sub_080022B48(void *a, int b, int c) __attribute__((alias("RaceVM_022B48")));
#endif

// ============================================================================
// sub_080022BBC (0x080022BBC, 0x5C B) — paint twin: dec s16[rec+128]
// (reload 15); r8 = 0x080CC158+4. The 0x022ACC call receives
// table[s16[rec+132]].word1 and rec[+136]; 0x022B48 receives table.word0
// and table.word1; finish with _0800DBE8(rec+8).
void RaceVM_022BBC(void *rec) {
    register volatile u8 *r4 __asm__("r4") = (volatile u8 *)rec;
    _0800D97C((void *)(r4 + 128), 15);
    register const u32 *r5 __asm__("r5") = (const u32 *)0x080CC158;
    __asm__ volatile("" : "+r" (r5));
    register volatile u8 *r6 __asm__("r6") = r4 + 132;
    register u32 r1 __asm__("r1") = 0;
    register s32 index __asm__("r0") = *(const s16 *)(r6 + r1);
    index <<= 3;
    r1 = (u32)(uintptr_t)r5 + 4;
    register const u8 *r8 __asm__("r8") = (const u8 *)(uintptr_t)r1;
    index += (u32)(uintptr_t)r8;
    r1 = *(const u32 *)(uintptr_t)index;
    register u32 modeAddress __asm__("r0") = (u32)(uintptr_t)r4 + 136;
    register u32 mode __asm__("r2") = *(volatile u32 *)(uintptr_t)modeAddress;
    RaceVM_022ACC((void *)r4, (int)r1, mode);
    r1 = 0;
    index = *(const s16 *)(r6 + r1);
    index <<= 3;
    r5 = (const u32 *)((u32)index + (u32)(uintptr_t)r5);
    r1 = r5[0];
    index += (u32)(uintptr_t)r8;
    mode = *(const u32 *)(uintptr_t)index;
#ifndef __APPLE__
    sub_080022B48((void *)r4, (int)r1, (int)mode);
#else
    RaceVM_022B48((void *)r4, (int)r1, (int)mode);
#endif
    r4 += 8;
    _0800DBE8((void *)r4);
}
#ifndef __APPLE__
void _080022BBC(void *a) __attribute__((alias("RaceVM_022BBC")));
void sub_080022BBC(void *a) __attribute__((alias("RaceVM_022BBC")));
#endif

// ============================================================================
// _080022C18 (0x080022C18, 0x9C B) — tail dispatcher (12-way
// 1-based table, pool base word → 0x08022C34; slots 3/4/8..11 = exit): twin
// of 0x02285C over the 0x08022xxx family — 1→022920 (ctor), 2→0228F8
// (setter), 5→D854+D8E4, 6→u16[+12]!=0 → 022A14, 7→022BBC (paint),
// 12→02291C (noop).
void RaceVM_022C18(u32 ev, u32 p1, u32 p2, void *p3) {
    if (ev == 0 || ev > 12) return;
    switch (ev) {
        // armcc laid the case bodies out in source order 2, 5, 7, 6, 1, 12.
        case 2: PV_CALLEE(RaceVM_0228F8, sub_0800228F8)((void *)p3, (void *)(uintptr_t)p1); break;
        case 5: _0800D854((u8 *)p3 + 8); _0800D8E4((u8 *)p3 + 112); break;
        case 7: PV_CALLEE(RaceVM_022BBC, sub_080022BBC)(p3); break;
        case 6:
            if (*(volatile u16 *)((u8 *)p3 + 12) != 0) {
                PV_CALLEE(RaceVM_022A14, sub_080022A14)(p3, (int)(u16)p1, (u16)p2);
            }
            break;
        case 1: PV_CALLEE(RaceVM_022920, sub_080022920)(p3); break;
        case 12: PV_CALLEE(RaceVM_02291C, sub_08002291C)(p3); break;
        default: break;
    }
}
// Match the ROM's zero Thumb alignment halfword after the return.
__asm__(".pushsection .text.RaceVM_022C18,\"ax\",%progbits\n\t.align 2, 0\n\t.popsection");
#ifndef __APPLE__
void _080022C18(u32 a, u32 b, u32 c, void *d) __attribute__((alias("RaceVM_022C18")));
#endif

// ============================================================================
// Host-test weak stubs (menu_stage.c already provides _08004EA8/EC0/BFC/
// ED8/2140/2178/3F18) — the ARM build uses C-lifted aliases / trampolines.
// ============================================================================
#ifdef __APPLE__
__attribute__((weak)) void *_08004B68(void) { return 0; }
__attribute__((weak)) void _0802B214(u32 v) { (void)v; }
__attribute__((weak)) void _0802B368(u16 v) { (void)v; }
__attribute__((weak)) void _08007770(int a, void *b, int c, int d, u32 e, u32 f) {
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f;
}
__attribute__((weak)) void _0800DAB8(void *p) { (void)p; }
__attribute__((weak)) void _08007614(void *a, int b, int c, int d) {
    (void)a; (void)b; (void)c; (void)d;
}
__attribute__((weak)) void _080075E8(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) void _0800D77C(void *a, int b, int c) { (void)a; (void)b; (void)c; }
__attribute__((weak)) int _08025D90(int a) { (void)a; return 0; }
__attribute__((weak)) void _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h) {
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h;
}
__attribute__((weak)) void _0800D97C(void *a, int b) { (void)a; (void)b; }
__attribute__((weak)) void _0800D854(void *a) { (void)a; }
__attribute__((weak)) void _0800D8E4(void *a) { (void)a; }
__attribute__((weak)) void _0800DBE8(void *a) { (void)a; }
__attribute__((weak)) void _0802E0A4(void *a, const void *b, u32 c) {
    (void)a; (void)b; (void)c;
}
__attribute__((weak)) int _0800572C(int a) { (void)a; return 0; }
__attribute__((weak)) void _08007664(void *a, void *b, int c) { (void)a; (void)b; (void)c; }
#endif
