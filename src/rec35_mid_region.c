#include "gba/types.h"
#include <stdint.h>

// ============================================================================
// rec35_mid_region.c — C lift of asm/rec35_runtime.s 0x080168E8-0x08018174
// : the 27 remaining mid-region functions — table-driven
// phase dispatchers, twin input/emit/paint/countdown families, and the
// rec35 event dispatchers. 27/27 function VMAs, 0 gaps.
//
// Evidence: instruction-for-instruction from objdump ROM ground truth
// (0x080168E8-0x08018174). Pool words preserved as immediates; armcc u16
// truncation idioms (lsls #16 / lsrs #16) transcribed with (u16)/(s16)
// casts. High-register spills are locals; clamps/loops match exactly.
//
// ⚠️ mov pc dispatchers use an INDIRECT base (first pool word = table start),
// so every case target shifts one slot later than the objdump pseudo-listing
// suggests. True tables (dumped from baserom.gba):
//   0x08016B0C (16AF0): p0->BB0 (fall), p1->B2C, p2->B40, p3->B54,
//                       p4..7->BB0.
//   0x08016BDC (16BBC): p0->BFC, p1->C48, p2->C8C, p3->CCC, p4->D02,
//                       p5->DB8 (fall), p6->D90, p7->DB8 (fall).
//   0x08016E04 (16DE4): p0->E28 (AF0), p1->E44, p2->F20 (fall), p3->EC4,
//                       p4->F20 (fall), p5->E30, p6->F20 (fall), p7->E8C,
//                       p8->F20 (fall).
//   0x08016F44 (16FD0): p0->FC0 (173E8), p1->F58, p2->FC8 (fall),
//                       p3->FBA, p4->FB4, default fall.
//   0x08017DCC (17DAC): ev1->EB6 (FD0), ev2->DFC (168E8), ev3/4->EC4,
//                       ev5->E06, ev6->E5E (17D54), ev7->E2A, ev8..11->EC4,
//                       ev12->EBE (16940).
//   0x08018190 (18174): ev1->81FA (17F04), ev2->81C0, ev3/4->8208 (fall),
//                       ev5->81CA, ev6->81E4 (18064), ev7->81DC (18128),
//                       ev8..11->8208 (fall), ev12->8202 (17F00 bx lr).
//
// Callee ABIs (proven at call sites):
// - _08024BFC(int) returns void* (record fetch; used as CpuSet ctrl in
//   16AF0 case 3 / 16BBC case 3).
// - _08002140(int) (r1 dead); _08002158(id, v) returns int (display list).
// Arities below are the *true* ROM arities, counted from each callee's frame
// displacement (sp+disp == caller stack word 0):
//   _08007B18  20+12+24 = 56 -> reads 56/60/64/68 = 4 stack words = 8 args
//   _08007BFC  20+ 8+24 = 52 -> reads 52/56/60/64/68 = 5 stack words = 9 args
//   _08007C68  20+ 4+24 = 48 -> reads 48/52/56/60/64 = 5 stack words = 9 args
//   _08007770  20+12+28 = 60 -> reads 60/64 = 2 stack words = 6 args
//   _08007570  16+ 0    = 16 -> reads 16 = 1 stack word = 5 args
//   _08002ED0(a,b,c,d, 6 stack slots) — read by the callee's own arg block.
// ============================================================================

extern void *Sub_08004B68(void *a);
extern void  Sub_08002E10(void *dst, void *src, int len);
extern void  Sub_08007ABC(u32 a, u32 b, u32 c);
extern void  Sub_0800D97C(void *a, int b);
extern void  sub_0800D97C(void *a, int b);  // closure spelling
extern void  Sub_0800DBE8(void *a);
extern void  sub_0800DBE8(void *a);  // closure spelling
extern void  Sub_0800D854(void *a);
extern void  Sub_0800D8E4(void *a);
extern void  Sub_08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern void  _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
extern void  Sub_0800DAB8(void *p);
extern void  Sub_0800798C(void *a, void *b);
extern void  Sub_08007570(void *a, int b, int c, int d, int e);
extern void  Sub_080075E8(void *a, int b, int c);
extern void *Sub_08005758(int n);
extern void  Sub_08007B18(void *a, int b, int c, int d, int e, int f, int g, int h);
extern void  Sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i);
extern void  Sub_08007C68(void *a, int b, int c, int d, int e, int f, int g, int h, int i);
extern void  _08007C68(void *a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i); // faithful strong body
// CpuSet wrapper 0x0802D974 = `swi 0x0B; bx lr`, i.e. BIOS order
// (src = r0, dst = r1, ctrl = r2). Callers below pass them positionally, so the
// parameter names are documentary only.
extern void  Sub_08002D974(const void *src, void *dst, u32 ctrl);
extern void  Sub_0802B214(int v);
extern void  Sub_0802B368(int v);
extern int   Sub_0802B3B8(int v);
extern int   Sub_08002060(int v);
extern int   Sub_08002140(void);
extern int   Sub_08002158(int id, int v);
extern int   Sub_08002178(int id);
extern void  Sub_0800226C(void *a, int b);
extern void  Sub_08002298(void *a);
extern int   Sub_080022C0(void);
extern int   Sub_080022CC(void);
extern void  Sub_080022E4(int v);
extern void  Sub_08002618(int a, int b);
extern void  Sub_08002714(int a, int b, int c);
extern void  Sub_08003978(int a, int b, int c);
extern void  Sub_08004D4C(int a, int b, int c);
extern void *Sub_08024BFC(void); // 0-arg: asm body is push/bl/pop/bx, r0 never read
extern void  Sub_08024C0C(void); // asm body is `bx lr` (baserom 0x24C0C)
extern void  Sub_08024C14(void); // asm body is `bx lr` (baserom 0x24C14)
extern void  Sub_08024C18(void); // asm body is `bx lr` (baserom 0x24C18)
extern void *Sub_08024C74(int a);
extern void *Sub_08024D5C(int a);
extern int   Sub_080258A8(int a);
extern void  Sub_08025BF0(void *a, int b);
extern void  Sub_08024C10(void); // asm body is `bx lr` (baserom 0x24C10)
extern void  Sub_08024C1C(void);    // trampoline (no strong body)
extern void  Sub_08024C08(void); // asm body is `bx lr` (baserom 0x24C08)
extern void *_080240D0(void);       // Ghost_GetRecP (strong, ghost2.c)
extern void  sub_08003BC0(int a, u32 b, int c); // ObjDigits_03BC0 (strong)
extern void  sub_08007A58(void *p); // Course_0x08007A58 (strong)
extern void  Sub_0800D77C(void *a, int b, int c);
extern int   _08025CF4(int a, int b, int c);

// Already-lifted siblings (strong bodies elsewhere in this cluster):
void Rec35_Leaf_16AD4(void *ctx);          // 0x08016AD4 (rec35_runtime.c)
void Rec35_BxLr_16940(void);               // 0x08016940 (rec35_runtime.c)
void Rec35_Leaf_168B8(u32 a0);             // 0x080168B8 (rec35_runtime.c)
void Rec35_Dispatch_16F28(void *ctx);      // 0x08016F28 (rec35_runtime.c)
void Rec35_Leaf_173E8(void *ctx);          // 0x080173E8 (rec35_runtime.c)
void Rec35_Leaf_17414(void *ctx);          // 0x08017414 (rec35_runtime.c)
void Rec35_BxLr_17D50(void *rec);          // 0x08017D50 (rec35_runtime.c)
void Rec35_BxLr_17F00(void);               // 0x08017F00 (rec35_runtime.c)

static inline u16 RD16P(const void *p)   { u16 v; __builtin_memcpy(&v, p, 2); return v; }
static inline void WR16(void *p, u16 v) { __builtin_memcpy(p, &v, 2); }
static inline s16 RS16(const void *p)   { s16 v; __builtin_memcpy(&v, p, 2); return v; }
static inline u32 RD32(const void *p)   { u32 v; __builtin_memcpy(&v, p, 4); return v; }
static inline void WR32(void *p, u32 v) { __builtin_memcpy(p, &v, 4); }

#define WA   ((u8 *)0x03001780)

// Forward decls (mutual recursion inside the cluster)
void Rec35_Leaf_16AF0(void *rec);
void Rec35_Leaf_16BBC(void *rec);
void Rec35_Leaf_16DE4(void *rec);
void Rec35_Leaf_16FD0(void *rec);
void Rec35_Input_17440(void *rec, int a, int id);
void Rec35_Input_17528(void *rec, int a, int id);
void Rec35_Input_175BC(void *rec);
void Rec35_Input_17788(void *rec, int a, int id);
void Rec35_Emit_1794C(void *rec);
void Rec35_Emit_17984(void *rec);
void Rec35_Emit_179F0(void *rec);
void Rec35_Emit_17A20(void *rec);
void Rec35_Emit_17B00(void *rec);
void Rec35_Emit_17B84(void *rec);
void Rec35_Paint_17BB8(void *rec);
void Rec35_Paint_17C34(void *rec);
void Rec35_Paint_17C78(void *rec);
void Rec35_Countdown_17D54(void *rec);
void Rec35_EvDispatch_17DAC(int ev, int a, int b, void *rec);
void Rec35_Ev1_17ECC(void *a, void *rec);
void Rec35_Setup_17F04(void *rec);
void Rec35_CountdownGate_18064(void *rec, int a, int b);
void Rec35_RowEmit_18098(void *rec, void *dst, int idx, int arg);
void Rec35_Init_18128(void *rec);
void Rec35_EvDispatch_18174(int ev, int a, int b, void *rec);

// ---------------------------------------------------------------------------
// _0800168E8(ctx, rec) — phase-3 start check (ctx in r1, rec in r0!)
// ROM: r5=rec+192; [r5]=0; if u16[WA+0x1780]==3 { rec+88 byte=0;
//   if u8[WA+0x10C3]==0 { _080056F4(rec,1,1); [r5]=1; } }
// hdr=_08004B68(ctx); u16[rec+84]=1.
void Rec35_Ev3_168E8(void *ctx, void *rec) {
    void *r5 = (u8 *)rec + 192;
    WR16(r5, 0);
    if (RD16P(WA + 0x1780) == 3) {
        *(u8 *)((u8 *)rec + 88) = 0;
        if (*(WA + 0x10C3) == 0) {
            extern void Sub_080056F4(void *a, int b, int c);
            Sub_080056F4(rec, 1, 1);
            WR16(r5, 1);
        }
    }
    Sub_08004B68(ctx);   // hdr discarded by the ROM (result unused)
    WR16((u8 *)rec + 84, 1);
}
#ifndef __APPLE__
void _0800168E8(void *a, void *b) __attribute__((alias("Rec35_Ev3_168E8")));
#endif

void Rec35_SyncRec_16944(void *rec) {
    void *r5 = rec;
    void *r4 = Sub_08024BFC();
    void *r0 = Sub_08024BFC();
    u16 a = RD16P(r4);
    u16 b = RD16P(r0);
    WR16((u8 *)r5 + 204, a);
    WR16((u8 *)r5 + 206, b);
    int sel = RS16((u8 *)r5 + 182);
    if (sel == 1)      { WR16((u8 *)r5 + 208, (u16)sel); WR16((u8 *)r5 + 210, RD16P((u8 *)r5 + 210)); }
    else if (sel == 2) { WR16((u8 *)r5 + 208, 0); WR16((u8 *)r5 + 210, 1); }
    else if (sel == 3) { WR16((u8 *)r5 + 208, 0); WR16((u8 *)r5 + 210, 0); }
    void *r3 = (u8 *)r5 + 208;
    void *r6 = (u8 *)r5 + 210;
    const s16 *tbl = (const s16 *)0x080CB8CC;
    int v = tbl[RS16(r3) * 4 + RS16((u8 *)r5 + 204)];
    WR32((u8 *)r5 + 460, (u32)v);
    Sub_08007ABC(RD32((u8 *)r5 + 12), (u32)v, 0);
    int v2 = tbl[RS16(r6) * 4 + RS16((u8 *)r5 + 206)];
    WR32((u8 *)r5 + 476, (u32)v2);
    Sub_08007ABC(RD32((u8 *)r5 + 12), (u32)v2, 0);
}
#ifndef __APPLE__
void _080016944(void *a) __attribute__((alias("Rec35_SyncRec_16944")));
#endif

// ---------------------------------------------------------------------------
// _080016A20(rec) — two-lap placement emit
void Rec35_PlacementEmit_16A20(void *rec) {
    void *r6 = rec;
    u32 sp[2];
    sp[0] = (u32)Sub_08024BFC();
    sp[1] = (u32)Sub_08024BFC();
    for (int i = 0; i <= 1; i++) {
        s16 r4 = RS16((u8 *)(u32)sp[i] + 16);
        Sub_080022E4(r4);
        WR32((u8 *)r6 + 194 + i * 12, (u32)Sub_08024D5C(r4));
        Sub_08007ABC(RD32((u8 *)r6 + 20), RD32((u8 *)r6 + 188 + i * 12), 0);
        WR32((u8 *)r6 + 206 + i * 12, (u32)Sub_08024C74(r4));
        Sub_08007ABC(RD32((u8 *)r6 + 28), RD32((u8 *)r6 + 200 + i * 12), 0);
        s16 v = (s16)Sub_080258A8(RS16((u8 *)(u32)sp[i] + 2));
        WR32((u8 *)r6 + 218 + i * 12, (u32)(u16)v);
        Sub_08007ABC(RD32((u8 *)r6 + 36), RD32((u8 *)r6 + 212 + i * 12), 0);
    }
}
#ifndef __APPLE__
void _080016A20(void *a) __attribute__((alias("Rec35_PlacementEmit_16A20")));
#endif

// ---------------------------------------------------------------------------
// _080016AF0(rec) — 8-way phase dispatcher (indirect table 0x08016B0C)
//   p1 -> _08002298(+276), [+196]=4;
//   p2 -> [_080022C0==1] -> [+194]=1;
//   p3 -> [_080022C0==1] -> CpuSet(+276, 0x04000000, ctrl=0x08024BFC(0)),
//         CpuSet(+308, 0x04000000, ctrl=0x08024BFC(1)), SyncRec_16944,
//         PlacementEmit_16A20, [+194]=1;
//   p0/p4..7/default -> fallthrough (no writes).
void Rec35_Leaf_16AF0(void *rec) {
    int p = RS16((u8 *)rec + 196);
    switch (p & 7) {
    case 1:
        Sub_08002298((u8 *)rec + 276);
        WR16((u8 *)rec + 196, 4);
        break;
    case 2:
        if (Sub_080022C0() == 1) WR16((u8 *)rec + 194, 1);
        break;
    case 3:
        if (Sub_080022C0() != 1) break;
        // ROM (asm/rec35_runtime.s:833-849):
        //   r4 = rec + 138*2; bl 0x08024BFC (ghost); r1 = ghost;
        //   r2 = pool _080016BB8 = 0x04000008; r0 = rec+276; bl 0x0802D974
        //   r4 = rec + 154*2; bl 0x08024BFC (ghost); r1 = ghost; r2 = same pool;
        //   r0 = rec+308; bl 0x0802D974
        // i.e. CpuSet(src = rec+276 / rec+308, dst = the ghost record,
        // ctrl = 0x04000008). The old form used dst 0x04000000 and put the ghost
        // pointer in the *ctrl* word, so the count came out of an address.
        Sub_08002D974((const void *)((u8 *)rec + 276), Sub_08024BFC(), 0x04000008u);
        Sub_08002D974((const void *)((u8 *)rec + 308), Sub_08024BFC(), 0x04000008u);
        Rec35_SyncRec_16944(rec);
        Rec35_PlacementEmit_16A20(rec);
        WR16((u8 *)rec + 194, 1);
        break;
    default:
        break;
    }
}
#ifndef __APPLE__
void _080016AF0(void *a) __attribute__((alias("Rec35_Leaf_16AF0")));
#endif

// ---------------------------------------------------------------------------
// _080016BBC(rec) — 8-way phase dispatcher (indirect table 0x08016BDC)
void Rec35_Leaf_16BBC(void *rec) {
    void *r5 = rec;
    int p = RS16((u8 *)r5 + 198);
    switch (p & 7) {
    case 0: {                                   // 0x08016BFC
        u16 v = (u16)Sub_08002178(1);
        if (v != 1) break;
        if (*(WA + 0x10C3) == 1) { WR16((u8 *)r5 + 198, 2); break; }
        Rec35_Leaf_16AD4(r5);
        Sub_08024C18();
        WR16((u8 *)r5 + 198, v);
        break;
    }
    case 1: {                                   // 0x08016C48
        WR32((u8 *)r5 + 508, 0);
        WR32((u8 *)r5 + 512, 0);
        int v = (int)_080240D0() + 32;
        Sub_0800226C((u8 *)r5 + v, 0x09E0);
        WR16((u8 *)r5 + 198, 3);
        Sub_08002618(1, 1);
        WR16((u8 *)r5 + 192, 1);
        break;
    }
    case 2: {                                   // 0x08016C8C
        WR32((u8 *)r5 + 508, 0);
        WR32((u8 *)r5 + 512, 0);
        int v = (int)_080240D0() + 32;
        Sub_08002298((u8 *)r5 + v);
        WR16((u8 *)r5 + 198, 4);
        Sub_08002618(1, 1);
        WR16((u8 *)r5 + 192, 1);
        break;
    }
    case 3:                                     // 0x08016CCC
        if (Sub_080022C0() != 1) break;
        WR16((u8 *)r5 + 198, 6);
        Rec35_Leaf_16AD4(r5);
        Sub_08025BF0((u8 *)r5 + 480, 3);
        Sub_08007ABC(RD32((u8 *)r5 + 44), RD32((u8 *)r5 + 484), RD32((u8 *)r5 + 480));
        break;
    case 4: {                                   // 0x08016D02
        if (Sub_080022C0() != 1) break;
        Rec35_Leaf_16AD4(r5);
        Sub_08025BF0((u8 *)r5 + 480, 1);
        Sub_08007ABC(RD32((u8 *)r5 + 44), RD32((u8 *)r5 + 484), RD32((u8 *)r5 + 480));
        s16 r4v = RS16((u8 *)r5 + 342); // ROM ldrsh kept for provenance
        (void)r4v;
        Sub_08024C14();
        // ROM (asm/rec35_runtime.s:1049-1057): r4 = rec + s16[rec+340]*32 + 276
        // (the `subs r2, #64` reuses the 340 constant); dst = the ghost record
        // (bl 0x08024BFC), ctrl = pool _080016D8C = 0x04000008.
        const void *r4p = (const void *)((u8 *)r5 + RS16((u8 *)r5 + 340) * 32 + 276);
        Sub_08002D974(r4p, Sub_08024BFC(), 0x04000008u);
        Sub_08024C10();
        Sub_08024C10();
        Sub_08024C1C();
        Rec35_SyncRec_16944(r5);
        Rec35_PlacementEmit_16A20(r5);
        WR16((u8 *)r5 + 198, 6);
        break;
    }
    case 6: {                                   // 0x08016D90
        WR32((u8 *)r5 + 512, 1);
        u16 v = (u16)Sub_08002178(6);
        if (v != 1) break;
        WR32((u8 *)r5 + 508, 1);
        WR16((u8 *)r5 + 194, 6);
        break;
    }
    case 5:                                     // 0x08016DB8 (fallthrough exit)
    case 7:
    default:
        break;
    }
}
#ifndef __APPLE__
void _080016BBC(void *a) __attribute__((alias("Rec35_Leaf_16BBC")));
#endif

// ---------------------------------------------------------------------------
// _080016DC0(rec) — decrement u16[rec+346]
// The halfword accesses must be DIRECT volatile lvalue reads/writes, not the
// file's RD16P/WR16 memcpy helpers: those lower to a `bl memcpy` with a stack
// temp, which is 76 bytes against the ROM's 36 (measured). A `volatile u16 *`
// lvalue gives the ROM's `ldrh/strh` pair on the same computed pointer.
void Rec35_Leaf_16DC0(void *ctx) {
    volatile u16 *p = (volatile u16 *)((u8 *)ctx + 346);
    u16 v = *p;
    *p = --v;
    // The ROM sign-tests the narrowed value in place: `lsls r0,#16 /
    // cmp r0,#0 / bge`, with no re-narrow pair before the store. That is the
    // non-volatile `(s16)` compare against 0.
    if ((s16)v < 0) {
        Sub_08002060(1);
        *p = 0;
    }
}
#ifndef __APPLE__
// `asm/rec35_runtime.s:3132` still branches to `bl sub_080016DC0`, so the
// `sub_` twin must exist in C once this body is promoted (the manifest's
// `export` list only re-emits a label whose VMA equals the entry's).
void _080016DC0(void *c) __attribute__((alias("Rec35_Leaf_16DC0")));
void sub_080016DC0(void *c) __attribute__((alias("Rec35_Leaf_16DC0")));
#endif

// ---------------------------------------------------------------------------
// _080016DE4(rec) — 9-way phase dispatcher (indirect table 0x08016E04)
void Rec35_Leaf_16DE4(void *rec) {
    void *r5 = rec;
    int p = RS16((u8 *)r5 + 194);
    switch (p & 7) {
    case 0:                                     // 0x08016E28
        Rec35_Leaf_16AF0(r5);
        break;
    case 1:                                     // 0x08016E44
        if ((Sub_0802B3B8(3) << 24) == 0) Rec35_Leaf_16BBC(r5);
        break;
    case 2: {                                   // 0x08016F20 (exit arm)
        break;
    }
    case 3: {                                   // 0x08016EC4
        Sub_0802B368(3);
        if ((Sub_0802B3B8(1) << 24) != 0) break;
        WR16((u8 *)r5 + 60, 1);
        WR32((u8 *)r5 + 504, 2);
        Sub_08007ABC(RD32((u8 *)r5 + 52), RD32((u8 *)r5 + 500), 2);
        Sub_08024C1C();
        Rec35_SyncRec_16944(r5);
        Rec35_PlacementEmit_16A20(r5);
        WR16((u8 *)r5 + 194, 4);
        WR16((u8 *)r5 + 184, 1);
        WR16((u8 *)r5 + 182, 1);
        WR16((u8 *)r5 + 186, 1);
        break;
    }
    case 5: {                                   // 0x08016E30
        void *r4 = Sub_08024BFC();
        void *r1 = Sub_08024BFC();
        if (RD16P(r4) == 0 && RD16P(r1) == 0) { WR16((u8 *)r5 + 194, 7); break; }
        WR32((u8 *)r5 + 504, 0);
        Sub_08007ABC(RD32((u8 *)r5 + 52), RD32((u8 *)r5 + 500), 0);
        break;
    }
    case 7: {                                   // 0x08016E8C
        Sub_08002618(1, 1);
        WR16((u8 *)r5 + 192, 1);
        void *r4 = (u8 *)r5 + 480;
        Sub_08025BF0(r4, 0);
        Sub_08007ABC(RD32((u8 *)r5 + 44), RD32((u8 *)r5 + 484), RD32(r4));
        WR16((u8 *)r5 + 194, 8);
        break;
    }
    case 4:                                     // 0x08016E20 (fallthrough exit)
    case 6:
    case 8:
    default:
        break;
    }
}
#ifndef __APPLE__
void _080016DE4(void *a) __attribute__((alias("Rec35_Leaf_16DE4")));
#endif

// ---------------------------------------------------------------------------
// _080016FD0(rec) — 5-way phase dispatcher (indirect table 0x08016F44)
void Rec35_Leaf_16FD0(void *rec) {
    void *r5 = rec;
    int p = RS16((u8 *)r5 + 200);
    switch (p & 7) {
    case 0:                                     // 0x08016FC0
        Rec35_Leaf_173E8(r5);
        WR16((u8 *)r5 + 200, 2);
        break;
    case 1:                                     // 0x08016F58
        if ((Sub_0802B3B8(3) << 24) != 0) break;
        if (RS16((u8 *)r5 + 182) == 1)      Sub_08024C0C();
        else if (RS16((u8 *)r5 + 182) == 2) Sub_08024C0C();
        else break;
        Rec35_SyncRec_16944(r5);
        Rec35_PlacementEmit_16A20(r5);
        WR16((u8 *)r5 + 200, 4);
        break;
    case 3:                                     // 0x08016FBA
        WR16((u8 *)r5 + 60, 1);
        break;
    case 4:                                     // 0x08016FB4
        WR16((u8 *)r5 + 60, 1);
        break;
    case 2:                                     // 0x08016FC8 (exit)
    default:
        break;
    }
}
#ifndef __APPLE__
void _080016FD0(void *a) __attribute__((alias("Rec35_Leaf_16FD0")));
#endif

// ---------------------------------------------------------------------------
// _080017440(rec, a, id) — selection key handler
void Rec35_Input_17440(void *rec, int a, int id) {
    (void)a;
    void *r5 = rec;
    void *r7 = (u8 *)r5 + 182;
    u16 old = RD16P(r7);
    void *r4 = (u8 *)r5 + 200;
    int mode = RD16P(r4);
    if (mode == 4) {
        if (id == 1) { WR16(r4, 3); return; }
    }
    if (id == 2) {
        Sub_0802B368(4);
        WR32((u8 *)r5 + 56, 10);
        WR16((u8 *)r5 + 60, 0);
        WR32((u8 *)r5 + 88, 10);
        WR32((u8 *)r5 + 84, 0);
    } else if (id == 1) {
        int cur = RS16(r7);
        if (cur == 1 || cur == 2 || cur == 3) {
            Sub_0802B368(1);
            WR16(r4, (u16)id);
            WR16((u8 *)r5 + 60, 0);
            Sub_08025BF0((u8 *)r5 + 480, 9);
            Sub_08007ABC(RD32((u8 *)r5 + 44), RD32((u8 *)r5 + 484), RD32((u8 *)r5 + 480));
            Sub_08002618(1, 1);
            WR16((u8 *)r5 + 192, (u16)id);
        }
    }
    if (id == 64)  { u16 v = RD16P(r7) - 1;  WR16(r7, v); }
    if (id == 128) { u16 v = RD16P(r7) + 1;  WR16(r7, v); }
    if (RS16(r7) < 1) WR16(r7, 1);
    if (RS16(r7) > 3) WR16(r7, 3);
    if (old != RD16P(r7)) {
        Rec35_SyncRec_16944(r5);
        Sub_0802B368(2);
    }
}
#ifndef __APPLE__
void _080017440(void *a, int b, int c) __attribute__((alias("Rec35_Input_17440")));
#endif

// ---------------------------------------------------------------------------
// _080017528(rec, a, id) — alternate selection key handler
void Rec35_Input_17528(void *rec, int a, int id) {
    (void)a;
    void *r4 = rec;
    void *r5 = (u8 *)r4 + 182;
    u16 old = RD16P(r5);
    if (id == 2) Rec35_Leaf_17414(r4);
    if (id == 1) {
        int cur = RS16(r5);
        if (cur == 1 || cur == 2)      Sub_08024C08();
        else if (cur == 3)             Sub_08024C08();
        else goto skip;
        Rec35_Leaf_173E8(r4);
    }
skip:
    void *r2 = (u8 *)r4 + 182;
    if (id == 64)  { u16 v = RD16P(r2) - 1;  WR16(r2, v); }
    if (id == 128) { u16 v = RD16P(r2) + 1;  WR16(r2, v); }
    if (RS16(r2) < 1) WR16(r2, 1);
    if (RS16(r2) > 3) WR16(r2, 3);
    if (old != RD16P(r2)) {
        Rec35_SyncRec_16944(r4);
        Sub_0802B368(2);
    }
}
#ifndef __APPLE__
void _080017528(void *a, int b, int c) __attribute__((alias("Rec35_Input_17528")));
#endif

// ---------------------------------------------------------------------------
// _0800175BC(rec) — mode-2/4 selection FSM
// ROM: r5=_08024BFC(0); r6=_08024BFC(1);
//   if u16[rec+188]==2 && (u16)_08002178(0,5)==2 && (u16)_08002178(1,5)==2
//     { [+190]=0; _080017414(rec); }
//   mode=ldrsh[rec+188]; if mode!=1 → return.
//   r9=0; [rec+190]=0 (halfword store of r9=0);
//   r8=rec+194; sw=ldrsh[r8];
//   sw==2 → if (u16)_08002178(1,4)==2 { sel=ldrsh[rec+182];
//       sel==1: [r5]==1 → _080168B8(1); [rec+340]=r9(0); [+194]=3;
//       sel==2: [r6]==1 → _080168B8(1); [rec+340]=r7; [+194]=3;
//       sel==3: _08017414(rec); else: _080168B8(10); }
//   sw==4 → if (u16)_08002178(1,4)==4 { sel=ldrsh[rec+182];
//       sel==1: [r5]==1 → _080168B8(1); [rec+342]=r9(0);
//       sel==2: [r6]==1 → _080168B8(1); [rec+342]=r7;
//       if u8[WA+0x10C3]==0 { r0=(u16)_08002178(1,3); [rec+340]=r0; }
//       [+194]=5; [rec+198]=r9(0); [rec+344]=r7; }
void Rec35_Input_175BC(void *rec) {
    void *r4 = rec;
    void *r5 = Sub_08024BFC();
    void *r6 = Sub_08024BFC();
    if (RD16P((u8 *)r4 + 188) == 2) {
        if ((u16)Sub_08002178(0) == 2 && (u16)Sub_08002178(1) == 2) {
            WR16((u8 *)r4 + 190, 0);
            Rec35_Leaf_17414(r4);
        }
    }
    int mode = RS16((u8 *)r4 + 188);
    if (mode != 1) return;
    u16 r9 = 0;
    WR16((u8 *)r4 + 190, r9);
    void *r8 = (u8 *)r4 + 194;
    int sw = RS16(r8);
    int sel = RS16((u8 *)r4 + 182);
    if (sw == 2) {
        if ((u16)Sub_08002178(1) != 2) return;
        if (sel == 1) {
            if (RD16P(r5) == 1) { Rec35_Leaf_168B8(1); WR16((u8 *)r4 + 340, r9); }
        } else if (sel == 2) {
            if (RD16P(r6) == 1) { Rec35_Leaf_168B8(1); WR16((u8 *)r4 + 340, (u16)mode); }
        } else if (sel == 3) {
            Rec35_Leaf_17414(r4);
        } else {
            Rec35_Leaf_168B8(10);
        }
        WR16(r8, 3);                            // [+194]=3 (0x08017696)
        return;
    }
    if (sw == 4) {
        if ((u16)Sub_08002178(1) != 4) return;
        if (sel == 1) {
            Rec35_Leaf_168B8(1);
            WR16((u8 *)r4 + 342, r9);
        } else if (sel == 2) {
            Rec35_Leaf_168B8(1);
            WR16((u8 *)r4 + 342, (u16)mode);
        } else if (sel != 3) {
            return;
        }
        if (*(WA + 0x10C3) == 0) {
            u16 one = (u16)Sub_08002178(1);
            WR16((u8 *)r4 + 340, one);
        }
        WR16(r8, 5);                            // [+194]=5
        WR16((u8 *)r4 + 198, r9);
        WR16((u8 *)r4 + 344, (u16)mode);
    }
}
#ifndef __APPLE__
void _0800175BC(void *a) __attribute__((alias("Rec35_Input_175BC")));
#endif

// ---------------------------------------------------------------------------
// _08001774C(rec) — gate leaf (u16[+184] compare after key read)
void Rec35_Leaf_1774C(void *rec) {
    void *r5 = rec;
    u16 a = (u16)Sub_08002178(0);
    u16 b = (u16)Sub_08002178(1);
    if (a != b) return;
    u16 m = RD16P((u8 *)r5 + 188);
    if ((u16)(m - 1) <= 1) Rec35_Leaf_17414(r5);
}
#ifndef __APPLE__
void _08001774C(void *a) __attribute__((alias("Rec35_Leaf_1774C")));
#endif

// ---------------------------------------------------------------------------
// _080017788(rec, a, id) — full-ctrl selection (see ROM register-plumbing note)
void Rec35_Input_17788(void *rec, int a, int id) {
    void *r7 = rec;
    int r9 = (s16)a;
    int r8 = (s16)id;
    void *r5, *r6, *r4, *r1, *r2;
    (void)r9;
    if (*(WA + 0x10C3) == 1) {
        r5 = (u8 *)r7 + 194;
        (void)Sub_08002158(4, RD16P(r5));
        (void)Sub_08002158(5, RD16P((u8 *)r7 + 190));
        r4 = (u8 *)r7 + 184;
        (void)Sub_08002158(1, RD16P(r4));
        (void)Sub_08002158(3, RD16P((u8 *)r7 + 340));
        (void)Sub_08002158(2, RD16P((u8 *)r7 + 344));
        (void)Sub_08002158(6, RD32((u8 *)r7 + 512));
        r6 = r5;                    // 0x080177F8: r6 = r5
        r1 = r4;                    // 0x080177FA: r1 = r4
        r5 = (u8 *)r5 - 6;          // 0x080177FC: r5 -= 6  → +188
        r4 = (u8 *)r4 - 2;          // 0x080177FE: r4 -= 2  → +182
    } else {
        r6 = (u8 *)r7 + 194;
        (void)Sub_08002158(4, RD16P(r6));
        r5 = (u8 *)r7 + 188;
        (void)Sub_08002158(5, RD16P(r5));
        r4 = (u8 *)r7 + 182;
        (void)Sub_08002158(1, RD16P(r4));
        (void)Sub_08002158(3, RD16P((u8 *)r7 + 340));
        (void)Sub_08002158(2, RD16P((u8 *)r7 + 344));
        (void)Sub_08002158(6, RD32((u8 *)r7 + 512));
        r1 = (u8 *)r7 + 184;        // 0x0801785A: r1 = rec+184
        r5 = (u8 *)r5 + 2;          //             r5 = rec+190
        r6 = r5;                    //             r6 = rec+190
        r4 = r1;                    //             r4 = rec+184
    }
    r2 = r1;                        // 0x0801788E: r2 = r1 (+184)
    (void)r2;
    if (r8 == 1) WR16((u8 *)r7 + 190, (u16)r8);
    if (r8 == 2) WR16((u8 *)r7 + 190, (u16)r8);
    if (r8 == 64)  { u16 v = RD16P(r1) - 1;  WR16(r1, v); }
    if (r8 == 128) { u16 v = RD16P(r1) + 1;  WR16(r1, v); }
    if (RS16(r1) < 1) WR16(r1, 1);
    if (RS16(r1) > 3) WR16(r1, 3);
    if (*(WA + 0x10C3) == 0) {
        WR16(r4, (u16)Sub_08002178(1));   // → [+184]
    } else {
        WR16(r4, 0);
    }
    WR16(r5, (u16)Sub_08002178(5));       // → [+190]
    if (RS16(r4) == 0) WR16(r4, RD16P((u8 *)r7 + 186));
    if (RS16(r6) == 2 || RS16(r6) == 4) {
        if (RD16P((u8 *)r7 + 186) != RD16P(r4)) {
            Rec35_SyncRec_16944(r7);
            Sub_0802B368(2);
            Rec35_Leaf_168B8(2);
        }
    }
    WR16((u8 *)r7 + 186, RD16P(r4));
    int cur = RS16(r6);
    if (cur >= 2 && cur <= 4)      Rec35_Input_175BC(r7);
    else if (cur >= 6 && cur <= 8) Rec35_Leaf_1774C(r7);
}
#ifndef __APPLE__
void _080017788(void *a, int b, int c) __attribute__((alias("Rec35_Input_17788")));
#endif

// Call-site split for promoted bodies. `promotion_screen.py` only accepts a
// promoted body whose call targets are spelled the way the assembly closure
// labels them; a friendly C name carries no VMA, so the screen cannot map it
// to an address and reports "no VMA and not a promoted export". The closure
// labels each emitter twice -- `sub_0800179xx` and `_0800179xx` -- and the
// `_0800…` twin is the alias this file already defines, so no extra alias (and
// no second hop for the linker) is needed. On the host build the aliases do
// not exist (clang rejects `__attribute__((alias))`), so the friendly name is
// used there. Same split as MS_CALLEE in menu_stage.c / FF_CALLEE in
// menu_ff78_f.c; spelled separately per file to keep each TU self-contained.
#ifndef __APPLE__
#define R35_CALLEE(friendly, closure) closure
#else
#define R35_CALLEE(friendly, closure) friendly
#endif

// ---------------------------------------------------------------------------
// _08001794C(rec) — two banner-sprite digits (0x08024BFC slot 0 → x=68,
// slot 1 → x=108), each drawn only when its record's u16[+0] is set.
void Rec35_Emit_1794C(void *rec) {
    (void)rec;
    void *r4 = Sub_08024BFC();
    void *r5 = Sub_08024BFC();
    if (RD16P(r4) != 0) sub_08003BC0(152, 68, (int)RD32((u8 *)r4 + 4));
    if (RD16P(r5) != 0) sub_08003BC0(152, 108, (int)RD32((u8 *)r5 + 4));
}
#ifndef __APPLE__
void _08001794C(void *a) __attribute__((alias("Rec35_Emit_1794C")));
#endif

// ---------------------------------------------------------------------------
// _080017984(rec) — two unconditional emitters over rec+8.
// NOTE: no guards in the ROM; both run every call.
void Rec35_Emit_17984(void *rec) {
    void *r8 = (u8 *)rec + 8;
    Sub_08007BFC(r8, (int)RD32((u8 *)rec + 456), (int)RD32((u8 *)rec + 460),
                 0, 48, 5, 1, 1, 0);
    Sub_08007BFC(r8, (int)RD32((u8 *)rec + 468), (int)RD32((u8 *)rec + 472),
                 0, 88, 5, 1, 1, 0);
}
#ifndef __APPLE__
void _080017984(void *a) __attribute__((alias("Rec35_Emit_17984")));
#endif

// ---------------------------------------------------------------------------
// _0800179F0(rec) — single _08007B18 emit when u16[+182]==3
void Rec35_Emit_179F0(void *rec) {
    extern void sub_08007B18(void *a, int b, int c, int d, int e, int f, int g, int h);
    if (*(u16 *)((u8 *)rec + 182) == 3) {
        sub_08007B18(rec, 4, 104, 128, 6, 1, 1, 0);
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _0800179F0(void *a) __attribute__((alias("Rec35_Emit_179F0")));
#endif

// ---------------------------------------------------------------------------
// _080017A20(rec) — FOUR emitters in two guarded pairs.
// Guard on slot 0: (rec+16, [384],[388], d=9,  e=72)  + (rec+24, [408],[412], d=76, e=72)
// Guard on slot 1: (rec+16, [396],[400], d=9,  e=112) + (rec+24, [420],[424], d=76, e=112)
void Rec35_Emit_17A20(void *rec) {
    void *r7 = rec;
    if (RD16P(Sub_08024BFC()) != 0) {
        Sub_08007BFC((u8 *)r7 + 16, (int)RD32((u8 *)r7 + 384), (int)RD32((u8 *)r7 + 388),
                     9, 72, 4, 1, 1, 0);
        Sub_08007BFC((u8 *)r7 + 24, (int)RD32((u8 *)r7 + 408), (int)RD32((u8 *)r7 + 412),
                     76, 72, 4, 1, 1, 0);
    }
    if (RD16P(Sub_08024BFC()) != 0) {
        Sub_08007BFC((u8 *)r7 + 16, (int)RD32((u8 *)r7 + 396), (int)RD32((u8 *)r7 + 400),
                     9, 112, 4, 1, 1, 0);
        Sub_08007BFC((u8 *)r7 + 24, (int)RD32((u8 *)r7 + 420), (int)RD32((u8 *)r7 + 424),
                     76, 112, 4, 1, 1, 0);
    }
}
#ifndef __APPLE__
void _080017A20(void *a) __attribute__((alias("Rec35_Emit_17A20")));
#endif

// ---------------------------------------------------------------------------
// _080017B00(rec) — two guarded emitters over rec+32 (e=60 / e=100)
void Rec35_Emit_17B00(void *rec) {
    void *r5 = rec;
    if (RD16P(Sub_08024BFC()) != 0) {
        Sub_08007BFC((u8 *)r5 + 32, (int)RD32((u8 *)r5 + 432), (int)RD32((u8 *)r5 + 436),
                     16, 60, 7, 1, 1, 0);
    }
    if (RD16P(Sub_08024BFC()) != 0) {
        Sub_08007BFC((u8 *)r5 + 32, (int)RD32((u8 *)r5 + 444), (int)RD32((u8 *)r5 + 448),
                     16, 100, 7, 1, 1, 0);
    }
}
#ifndef __APPLE__
void _080017B00(void *a) __attribute__((alias("Rec35_Emit_17B00")));
#endif

// ---------------------------------------------------------------------------
// _080017B84(rec) — single _08007BFC emitter (d=0, e=32, f=3)
// The ROM's r3 slot is clobbered to 0 by the last stack store before the call.
void Rec35_Emit_17B84(void *rec) {
    Sub_08007BFC((u8 *)rec + 48, (int)RD32((u8 *)rec + 492), (int)RD32((u8 *)rec + 496),
                 0, 32, 3, 1, 1, 0);
}
#ifndef __APPLE__
void _080017B84(void *a) __attribute__((alias("Rec35_Emit_17B84")));
#endif

// ---------------------------------------------------------------------------
// _080017BB8(rec) — paint
void Rec35_Paint_17BB8(void *rec) {
    void *r4 = rec;
    sub_0800D97C((u8 *)r4 + 176, 15);
    Rec35_Emit_1794C(r4);
    Rec35_Emit_17984(r4);
    Rec35_Emit_179F0(r4);
    Rec35_Emit_17A20(r4);
    Rec35_Emit_17B00(r4);
    Rec35_Emit_17B84(r4);
    if (RS16((u8 *)r4 + 192) == 1) {
        _08007C68((u8 *)r4 + 40, (u32)(int)RD32((u8 *)r4 + 480), 0x38,
                     (int)RD32((u8 *)r4 + 484), 64, 8, 1, 0, 0);
    }
    sub_0800DBE8((u8 *)r4 + 56);
    Rec35_BxLr_17D50(r4);
}
#ifndef __APPLE__
void _080017BB8(void *a) __attribute__((alias("Rec35_Paint_17BB8")));
#endif

// ---------------------------------------------------------------------------
// _080017C34(rec) — paint variant without the +192 lap branch
void Rec35_Paint_17C34(void *rec) {
    void *r4 = rec;
    sub_0800D97C((u8 *)r4 + 176, 15);
    R35_CALLEE(Rec35_Emit_1794C, _08001794C)(r4);
    R35_CALLEE(Rec35_Emit_17984, _080017984)(r4);
    R35_CALLEE(Rec35_Emit_179F0, _0800179F0)(r4);
    R35_CALLEE(Rec35_Emit_17A20, _080017A20)(r4);
    R35_CALLEE(Rec35_Emit_17B00, _080017B00)(r4);
    R35_CALLEE(Rec35_Emit_17B84, _080017B84)(r4);
    sub_0800DBE8((u8 *)r4 + 56);
    Rec35_BxLr_17D50(r4);
}
#ifndef __APPLE__
void _080017C34(void *a) __attribute__((alias("Rec35_Paint_17C34")));
// The closure spells this VMA `sub_080017C34` (asm/rec35_runtime.s:2905) and
// the entry exports it, so the owning TU must define that exact name. One hop
// to the real body -- an alias-of-an-alias is not in the slice link.
void sub_080017C34(void *a) __attribute__((alias("Rec35_Paint_17C34")));
#endif

// ---------------------------------------------------------------------------
// _080017C78(rec) — paint variant gated by u8[WA+0x10C3] and s16[+194]
void Rec35_Paint_17C78(void *rec) {
    void *r4 = rec;
    Sub_0800D97C((u8 *)r4 + 176, 15);
    if (*(WA + 0x10C3) == 1 && RS16((u8 *)r4 + 194) != 0) {
        Rec35_Emit_1794C(r4);
        Rec35_Emit_17984(r4);
        if ((u16)(RD16P((u8 *)r4 + 194) - 6) > 2) Rec35_Emit_179F0(r4);
        Rec35_Emit_17A20(r4);
        Rec35_Emit_17B00(r4);
        Rec35_Emit_17B84(r4);
    }
    if (RS16((u8 *)r4 + 192) == 1) {
        _08007C68((u8 *)r4 + 40, (u32)(int)RD32((u8 *)r4 + 480), 0x14,
                     (int)RD32((u8 *)r4 + 484), 72, 8, 1, 0, 0);
    }
    if (RS16((u8 *)r4 + 198) >= 3 && RS16((u8 *)r4 + 198) <= 4) {
        int v = Sub_080022CC() * 100;
        if (v < 0) v += 0xFFF;
        Sub_08003978(128, 96, 100 - (v >> 12));
    }
    Sub_0800DBE8((u8 *)r4 + 56);
    Rec35_BxLr_17D50(r4);
}
#ifndef __APPLE__
void _080017C78(void *a) __attribute__((alias("Rec35_Paint_17C78")));
#endif

// ---------------------------------------------------------------------------
// _080017D54(rec) — countdown
void Rec35_Countdown_17D54(void *rec) {
    void *r4 = rec;
    if (RD16P(WA + 0x1780) != 3) return;
    if (RD32((u8 *)r4 + 508) != 1) return;
    if (Sub_08002140() == 2) { WR32((u8 *)r4 + 504, 0); return; }
    u32 v = RD32((u8 *)r4 + 504) + 1;
    WR32((u8 *)r4 + 504, v);
    if ((int)v > 180) Sub_08004D4C(21, 0, 0);
}
#ifndef __APPLE__
void _080017D54(void *a) __attribute__((alias("Rec35_Countdown_17D54")));
#endif

// ---------------------------------------------------------------------------
// _080017DAC(ev, a, b, rec) — 12-way event dispatcher
// (indirect table 0x08017DCC; ev1 → Rec35_Leaf_16FD0!)
void Rec35_EvDispatch_17DAC(int ev, int a, int b, void *rec) {
    void *r4 = rec;
    void *r5 = (void *)(u32)a;
    void *r6 = (void *)(u32)b;
    switch (ev - 1) {
    case 0:                                     // 0x08017EB6 → 0x08016FD0
        Rec35_Leaf_16FD0(r4);
        break;
    case 1:                                     // 0x08017DFC → 0x080168E8
        Rec35_Ev3_168E8(r4, r5);
        break;
    case 4:                                     // 0x08017E06
        Sub_0800D854((u8 *)r4 + 56);
        Sub_0800D8E4((u8 *)r4 + 160);
        Rec35_Leaf_16DE4(r4);
        Rec35_Dispatch_16F28(r4);
        Rec35_Leaf_16DC0(r4);
        break;
    case 5: {                                   // 0x08017E5E → 0x08017D54 + rec+60 gate
        Rec35_Countdown_17D54(r4);
        if (RD16P((u8 *)r4 + 60) == 0) break;
        int m = RS16((u8 *)r4 + 180);
        if (m == 0)      Rec35_Input_17440(r4, (int)(u16)(u32)r5, (int)(u16)(u32)r6);
        else if (m == 1) Rec35_Input_17528(r4, (int)(u16)(u32)r5, (int)(u16)(u32)r6);
        else if (m == 3) Rec35_Input_17788(r4, (int)(u16)(u32)r5, (int)(u16)(u32)r6);
        break;
    }
    case 6: {                                   // 0x08017E2A
        int mode = RS16((u8 *)r4 + 180);
        if (mode == 0)      Rec35_Paint_17BB8(r4);
        else if (mode == 1) Rec35_Paint_17C34(r4);
        else if (mode == 3) Rec35_Paint_17C78(r4);
        break;
    }
    case 11:                                    // 0x08017EBE → 0x08016940
        Rec35_BxLr_16940();
        break;
    case 2:                                     // 0x08017EC4 (exit)
    case 3:
    case 7:
    case 8:
    case 9:
    case 10:
    default:
        break;
    }
}
#ifndef __APPLE__
void _080017DAC(int ev, int a, int b, void *rec) __attribute__((alias("Rec35_EvDispatch_17DAC")));
#endif

// ---------------------------------------------------------------------------
// _080017ECC(a, rec) — ev1 flag setter
void Rec35_Ev1_17ECC(void *a, void *rec) {
    void *hdr = Sub_08004B68(a);
    s16 v = RS16((u8 *)hdr + 2);
    u16 out;
    if (v == 21) out = 1;
    else if (v < 21) out = 6;
    else if (v > 30) out = 6;
    else if (v < 27) out = 6;
    else out = 1;
    WR16((u8 *)rec + 84, out);
}
#ifndef __APPLE__
void _080017ECC(void *a, void *b) __attribute__((alias("Rec35_Ev1_17ECC")));
#endif

// ---------------------------------------------------------------------------
// _080017F04(rec) — scene constructor
void Rec35_Setup_17F04(void *rec) {
    void *r7 = rec;
    Sub_0802B214(65);
    void *r4 = (void *)0x0802C31C;
    _08007770(0, r4, 1, 0, 4, 1); // R2 C body (was Sub_ veneer)
    WR32((u8 *)r7 + 80, 6);
    WR32((u8 *)r7 + 92, 23);
    WR32((u8 *)r7 + 104, 0);
    Sub_0800DAB8((u8 *)r7 + 8);
    Sub_0800798C(r4, r7);
    sub_08007A58(r7);
    Sub_080075E8(r4, 0, 3);
    int sl = 3;
    int r5 = 0;
    // The ROM re-reads the SAME two cells every iteration (r3 = WA+0xFF4 and
    // r9 = WA+0xFF8 are loop-invariant); only the column index i varies.
    const s16 *rowT = (const s16 *)(WA + 0x0FF4);
    const s16 *colT = (const s16 *)(WA + 0x0FF8);
    for (int i = 0; i <= 2; i++) {
        int res = _08025CF4(RS16(rowT), RS16(colT), i);
        if ((u32)(res - 1) <= 2) r5 = res;
        if (r5 <= sl) sl = r5;
    }
    WR16((u8 *)r7 + 132, (u16)sl);
    Sub_08002714(RS16(rowT), RS16(rowT + 2), sl);
    void *allocated = Sub_08005758(64);
    WR16((u16 *)0x0203F8EA, (u16)(u32)allocated);
    WR16((u16 *)0x0203F8E8, 0);
    int sel = RS16((u8 *)r7 + 132) - 1;
    Sub_08007570((void *)0x08306B54, RS16((u8 *)0x080CB8E0 + 2 * sel),
                 RS16((u16 *)0x0203F8EA), 64, 64);
    Sub_080075E8((void *)0x08306B54, RS16((u8 *)0x080CB8E6 + 2 * sel), 4);
    WR32((u8 *)r7 + 120, (u32)((u8 *)r7 + 136));
    WR32((u8 *)r7 + 124, (u32)((u8 *)r7 + 8));
    WR32((u8 *)r7 + 8, 0);
    WR16((u8 *)r7 + 12, 1);
    Sub_0800D77C((u8 *)r7 + 28, 0, -32);
    Sub_0800D77C((u8 *)r7 + 20, 0, 160);
    WR16((u8 *)r7 + 16, 6);
    WR16((u8 *)r7 + 18, 5);
    WR32((u8 *)r7 + 136, 2);
}
#ifndef __APPLE__
void _080017F04(void *a) __attribute__((alias("Rec35_Setup_17F04")));
#endif

// ---------------------------------------------------------------------------
// _080018064(rec, a, b) — countdown gate
// ROM: r2=ldrsh[rec+132]; r2--; if (u16)r2>1 → return; else 0x0802B368(1);
//   [rec+136]=1; [rec+12](u16)=0; [rec+116](u16)=1; [rec+36]=1.
void Rec35_CountdownGate_18064(void *rec, int a, int b) {
    (void)a; (void)b;
    void *r4 = rec;
    s16 r2 = RS16((u8 *)r4 + 132);
    r2 = (s16)(r2 - 1);
    if ((u16)r2 > 1) return;
    Sub_0802B368(1);
    WR32((u8 *)r4 + 136, 1);
    WR16((u8 *)r4 + 12, 0);
    WR16((u8 *)r4 + 116, 1);
    WR32((u8 *)r4 + 36, 1);
}
#ifndef __APPLE__
void _080018064(void *a, int b, int c) __attribute__((alias("Rec35_CountdownGate_18064")));
#endif

// ---------------------------------------------------------------------------
// _080018098(rec, dst, idx, arg) — row emit
void Rec35_RowEmit_18098(void *rec, void *dst, int idx, int arg) {
    void *r6 = rec;
    void *r7 = dst;
    int r1 = idx;
    int r8 = arg;
    u32 *ctr = (u32 *)0x03000590;
    *ctr = *ctr - 1;
    u16 *cursor = (u16 *)0x0203F8E8;
    if ((int)*ctr <= 0) { *cursor = (u16)(*cursor + 1); *ctr = 5; }
    if (RS16(cursor) > 15) *cursor = 0;
    int v = RS16((u8 *)cursor + 2);
    Sub_08007570((void *)0x08306B54, RS16((u8 *)0x080CB8E0 + 2 * r1), v,
                 (int)RS16(cursor) << 6, 64);
    Sub_08002ED0(r6, (int)(uintptr_t)r7, v, r8, 1, 3, 1, 0, 0, 1);
}
#ifndef __APPLE__
void _080018098(void *a, void *b, int c, int d) __attribute__((alias("Rec35_RowEmit_18098")));
#endif

// ---------------------------------------------------------------------------
// _080018128(rec) — init
void Rec35_Init_18128(void *rec) {
    u8 *r4 = (u8 *)rec;
    Sub_0800D97C(r4 + 128, 15);
    Sub_08007B18(r4, 2, 64, 64, 3, 1, 0, 0);
    Rec35_RowEmit_18098(0, (void *)72, *(const s16 *)(r4 + 132) - 1, 4);
    r4 += 8;
    Sub_0800DBE8(r4);
}
#ifndef __APPLE__
void _080018128(void *a) __attribute__((alias("Rec35_Init_18128")));
void sub_080018128(void *a) __attribute__((alias("Rec35_Init_18128")));
#endif

// ---------------------------------------------------------------------------
// _080018174(ev, a, b, rec) — 12-way event dispatcher
// (indirect table 0x08018190; ev1 → Rec35_Setup_17F04!)
extern void _080017ECC(void *a, void *b);
extern void _0800D854(void *a);
extern void _0800D8E4(void *a);
extern void _080018128(void *a);
extern void _080018064(void *a, int b, int c);
extern void _080017F04(void *a);
extern void _080017F00(void *a);

void Rec35_EvDispatch_18174(int ev, int a, int b, void *rec) {
    register int r5 __asm__("r5") = a;
    u8 *r4 = (u8 *)rec;
    switch (ev - 1) {
    case 1:
        _080017ECC(r4, (void *)(uintptr_t)r5);
        break;
    case 4:
        _0800D854(r4 + 8);
        _0800D8E4(r4 + 112);
        break;
    case 6:
        _080018128(r4);
        break;
    case 5:
        if (*(volatile u16 *)(r4 + 12) == 0) break;
        _080018064(r4, (u16)r5, (u16)b);
        break;
    case 0:
        _080017F04(r4);
        break;
    case 11:
        _080017F00(r4);
        break;
    default:
        break;
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080018174(int ev, int a, int b, void *rec) __attribute__((alias("Rec35_EvDispatch_18174")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void Rec35_Leaf_16944(void *rec) __attribute__((alias("Rec35_SyncRec_16944")));
void Rec35_Leaf_16A20(void *rec) __attribute__((alias("Rec35_PlacementEmit_16A20")));
#endif
