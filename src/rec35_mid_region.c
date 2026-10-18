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
extern void *sub_08004B68(void *a);
extern void *_08004B68(void);   // no-arg spelling used by 0x0800168E8's bare `bl`
typedef struct { u16 pad; s16 courseId; } Rec35_CourseHdr;
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
extern void  sub_08007570(void *a, int b, int c, int d, int e);
extern void  sub_08002ED0(void *a, int b, int c, int d, int e, int f, int g, int h, int i, int j);
extern void  Sub_080075E8(void *a, int b, int c);
extern void *Sub_08005758(int n);
extern void  Sub_08007B18(void *a, int b, int c, int d, int e, int f, int g, int h);
extern void  Sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i);
extern void  sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
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
extern int   Sub_08002178(int id, int sel);
extern int   _08002178(int id, int v);   // true ROM arity (see asm/menu_ebd8.s:20)
extern void  Sub_0800226C(void *a, int b);
extern void  Sub_08002298(void *a);
extern int   Sub_080022C0(void);
extern int   Sub_080022CC(void);
extern void  _080022E4(int v);
extern void  Sub_08002618(int a, int b);
extern void  Sub_08002714(int a, int b, int c);
extern void  Sub_08003978(int a, int b, int c);
extern void  Sub_08004D4C(int a, int b, int c);
extern void *Rec35GhostSlot(int slot) __asm__("_08024BFC");
extern void  Sub_08024C0C(void); // asm body is `bx lr` (baserom 0x24C0C)
extern void  Sub_08024C14(int a, int b); // asm body is `bx lr` (baserom 0x24C14)
extern void  Sub_08024C18(int a);        // asm body is `bx lr` (baserom 0x24C18)
extern void *_08024C74(int a);
extern void *_08024D5C(int a);
extern int   _080258A8(int a);
extern void  Sub_08025BF0(void *a, int b);
extern void  Sub_08024C10(int a);    // asm body is `bx lr` (baserom 0x24C10)
extern void  Sub_08024C1C(void);    // trampoline (no strong body)
extern void  Sub_08024C08(int v); // asm body is `bx lr` (baserom 0x24C08)
extern void *_080240D0(void);       // Ghost_GetRecP (strong, ghost2.c)
extern void  sub_08003BC0(int a, u32 b, int c); // ObjDigits_03BC0 (strong)
extern void  sub_08007A58(void *p); // Course_0x08007A58 (strong)
extern void  Sub_0800D77C(void *a, int b, int c);
extern int   _08025CF4(int a, int b, int c);

// Already-lifted siblings (strong bodies elsewhere in this cluster):
void Rec35_Leaf_16AD4(void *ctx);          // 0x08016AD4 (rec35_runtime.c)
void Rec35_BxLr_16940(void *a);            // 0x08016940 (rec35_runtime.c)
void Rec35_Leaf_168B8(u32 a0);             // 0x080168B8 (rec35_runtime.c)
void Rec35_Dispatch_16F28(void *ctx);      // 0x08016F28 (rec35_runtime.c)
void Rec35_Leaf_173E8(void *ctx);          // 0x080173E8 (rec35_runtime.c)
void Rec35_Leaf_17414(void *ctx);          // 0x08017414 (rec35_runtime.c)
extern void _080017414(void *ctx);         // same-address closure alias
extern void sub_0800168B8(u32 a0);          // assembly closure spelling
extern void sub_080017414(void *ctx);       // assembly closure spelling
void Rec35_BxLr_17D50(void *rec);          // 0x08017D50 (rec35_runtime.c)
void Rec35_BxLr_17F00(void);               // 0x08017F00 (rec35_runtime.c)

// Direct unaligned-capable accesses. Do NOT route these through
// `__builtin_memcpy`: agbcc does not expand it for these sizes, so every
// call site became a real `bl memcpy` and inflated every body in this cluster
// (e.g. _0800168E8 88 B candidate vs 88 B ROM became 132 B). A plain deref
// emits the single ldrh/strh the ROM has.
static inline u16 RD16P(const void *p)   { return *(const u16 *)p; }
static inline void WR16(void *p, u16 v) { *(u16 *)p = v; }
static inline s16 RS16(const void *p)   { return *(const s16 *)p; }
static inline u32 RD32(const void *p)   { return *(const u32 *)p; }
static inline void WR32(void *p, u32 v) { *(u32 *)p = v; }

#define WA   ((u8 *)0x03001780)

// Closure vs friendly callee spelling. A promoted body may only call what the
// slice closure defines; the `_`/`sub_` twin is that spelling, while the host
// build keeps the friendly `Sub_` name (clang rejects the aliases). Defined
// here, not mid-file, so the low-VMA bodies can use it too.
#define R35_CALLEE(friendly, closure) closure
extern void _080056F4(void *a, int b, int c);
extern int  _08002140(void);
extern void sub_08004D4C(int a, int b, int c);
extern int  _080022CC(void);
extern void sub_08003978(int a, int b, int c);
// Closure spellings for the 0x08016AF0 / 0x08016BBC / 0x08016DE4 dispatcher
// family (promotion_screen reports the friendly `Sub_` names as renameblocks).
extern void _080016944(void *rec);
extern void _080016A20(void *rec);
extern void _080016AF0(void *rec);
extern void _080016BBC(void *rec);
extern void _0800226C(void *a, int b);
extern void _08002298(void *a);
extern int  _080022C0(void);
extern void _08002618(int a, int b);
extern void _08007ABC(u32 a, u32 b, u32 c);
extern void sub_0802D974(const void *src, void *dst, u32 ctrl);
extern void _08024C08(int v);
extern void _08024C10(int a);
extern void _08024C14(int a, int b);
extern void _08024C18(int a);
extern void _08024C1C(void);
extern void _08025BF0(void *a, int b);
extern int  _0802B3B8(int v);
// Closure spellings the 0x08017DAC dispatcher needs (same rename blocks).
extern void _080016F28(void *ctx);
extern void _080016FD0(void *rec);
extern void _080017440(void *rec, int a, int id);
extern void _080017528(void *rec, int a, int id);
extern void _080017788(void *rec, int a, int id);
extern void _0800D854(void *a);
extern void _0800D8E4(void *a);

// Forward decls (mutual recursion inside the cluster)
void Rec35_Leaf_16AF0(void *rec);
void Rec35_Leaf_16BBC(void *rec);
void Rec35_Leaf_16DE4(void *rec);
void Rec35_Scene_16FD0(void *rec);
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
// ROM: r5=(1st arg)+192; [r5]=0; if u16[WA+0x0FBC]==3 { rec+88 byte=0;
//   if u8[WA+0x10C3]==0 { _080056F4(rec,1,1); [r5]=1; } }
// then a bare bl to the 0x08004B68 getter (result read at +2, dead) and
// u16[rec+84]=1.
void Rec35_Ev3_168E8(void *ctx, void *rec) {
    u8 *wa;
    void *r5;
    r5 = (u8 *)ctx + 192;      // r5 = 1st arg + 192
    WR16(r5, 0);
    // Symbol form, not a folded constant: agbcc folds `(u8*)0x03001780 + 0x0FBC`
    // into a single pool word, but the ROM keeps the base in r2 and adds each
    // offset. A named absolute symbol is not a foldable constant.
    { extern u8 Rec35Wa[];
      __asm__(".globl Rec35Wa\nRec35Wa = 0x03001780\n");
      wa = Rec35Wa; }
    if (RD16P(wa + 0x0FBC) == 3) {
        *(u8 *)((u8 *)rec + 88) = 0;
        if (*(u8 *)(wa + 0x10C3) == 0) {
            R35_CALLEE(Sub_080056F4, _080056F4)(rec, 1, 1);
            WR16(r5, 1);
        }
    }
    // ROM: bare `bl 0x08004B68` (no argument setup) then a DEAD `ldrh r0,[r0,#2]`
    // whose value the following `movs r0,#1` overwrites. The getter takes no
    // parameter here (same prototype used at other 0x08004B68 call sites);
    // the volatile read keeps the otherwise-dead load in place.
    (void)*(volatile u16 *)((u8 *)_08004B68() + 2);
    WR16((u8 *)rec + 84, 1);
}
void _0800168E8(void *a, void *b) __attribute__((alias("Rec35_Ev3_168E8")));

// ROM: ghost cells -> [+204]/[+206]; selection cell -> [+208]/[+210]
//   sel1 -> [208]=1 [210]=0 | sel2 -> [208]=0 [210]=1 | sel3 -> 0/0 | else: no write.
// Then 0x080CB8CC is a s16[?][2] table read with the pair ([+204], [+208]) for
// the first output and ([+206], [+210]) for the second: the byte maths is
//   tbl + s16[col]*2 + s16[row]*4
// and each result lands in +460 / +472. The matching +456 / +468 cell is
// _08007ABC's *third* argument and the table value its second.
//
// Every one of these spellings is load-bearing:
//  * `dst` has to be a real pointer variable, and the second ghost cell has to
//    go through `WR16(rec + 206)` rather than `dst[1]`. Together they put `z`'s
//    `movs r3,#0` ahead of the first store and give the ROM's
//    `adds r1,#2 / strh r0,[r1]` pointer walk instead of `strh r0,[r1,#2]`.
//  * the `__asm__("r1")` pin on `dst` is LOAD-BEARING: without it agbcc hands
//    the pointer r2 and the ghost value r1 (`ldrh r1` / `adds r2,#204`).
//  * the tail needs the column term (`* 2`) as its own statement ahead of the
//    row term, and the table base added through an explicit byte offset. A
//    plain `tbl[row][col]` subscript evaluates the row address first, picks the
//    scratch zero index as r7 and drags an extra `push/pop {r7}` in.
void Rec35_SyncRec_16944(void *rec) {
    void *r5 = rec;
    void *r4 = Rec35GhostSlot(0);
    void *r0 = Rec35GhostSlot(1);
    u16 a = RD16P(r4);
    register u16 *dst __asm__("r1") = (u16 *)((u8 *)r5 + 204);
    u16 z = 0;
    dst[0] = a;
    u16 b = RD16P(r0);
    WR16((u8 *)r5 + 206, b);
    // A 3-case switch, not an if/else-if chain. agbcc only builds an indirect
    // table from 4 cases up; at 3 it emits the binary decision tree the ROM
    // has (`cmp #2 / beq` then `cmp #2 / bgt` then `cmp #1 / beq`), which no
    // if/else-if spelling produces (that comes out as a linear chain).
    int sel = RS16((u8 *)r5 + 182);
    switch (sel) {
    case 1: WR16((u8 *)r5 + 208, (u16)sel); WR16((u8 *)r5 + 210, z); break;
    case 2: WR16((u8 *)r5 + 208, z); WR16((u8 *)r5 + 210, 1); break;
    case 3: WR16((u8 *)r5 + 208, z); WR16((u8 *)r5 + 210, z); break;
    default: break;
    }
    void *p208 = (u8 *)r5 + 208;
    void *r6 = (u8 *)r5 + 210;
    u32 *out1 = (u32 *)((u8 *)r5 + 460);
    const s16 (*tbl)[2] = (const s16 (*)[2])0x080CB8CC;
    int term1 = RS16(p208) * 2;
    int off1 = term1 + RS16((u8 *)r5 + 204) * 4;
    *out1 = (u32)*(const s16 *)((const u8 *)tbl + off1);
    R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)r5 + 12), *out1, RD32((u8 *)r5 + 456));
    u32 *out2 = (u32 *)((u8 *)r5 + 472);
    int term2 = RS16(r6) * 2;
    int off2 = term2 + RS16((u8 *)r5 + 206) * 4;
    *out2 = (u32)*(const s16 *)((const u8 *)tbl + off2);
    R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)r5 + 12), *out2, RD32((u8 *)r5 + 468));
}
void _080016944(void *a) __attribute__((alias("Rec35_SyncRec_16944")));

// ---------------------------------------------------------------------------
// _080016A20(rec) — two-lap placement emit
// Lap 0/1 picks a value out of each ghost record (sp[0]/sp[1]) and stores it to
// +388/+412/+436 (+12 per lap) while the matching +384/+408/+432 cell goes to
// _08007ABC as its *third* argument (the freshly written value is the second,
// exactly as in Rec35_SyncRec_16944). The +436 value sign-extends
// (`lsls #16 / asrs #16`), so its cast is (s16), not (u16).
//
// The `__asm__("r6")` pin on the record pointer is LOAD-BEARING: without it
// agbcc strength-reduces the lap offset into `rec + 384 + i*12` parked in r6 and
// walked by 12 (`[r6,#4]`-style stores), 40 bytes short of the ROM. With the pin
// the lap offset stays in r7 and rec stays in r6, as the ROM has them.
//
// The destination pointers pin the address association to the ROM's
// `(rec + C) + off`:
//   * the lap offset is a real `off` local walked by 12 (fold-const.c's
//     `associate:` only reassociates `(VAR + CON) + ARG1`, so a genuine
//     variable keeps `(base) + off` intact -- `i * 12` gets strength-reduced
//     into an IV and reassociated to `(off + rec) + C` instead);
//   * each destination base is spelled as a `u8 *pa = (u8 *)r6 + C;`
//     statement, so `pa + off` folds to `PLUS(rec + C, off)` and the constant
//     lands in the register operand of the first add (`adds r0, r6, r3`),
//     exactly as the ROM has it. Inlining `(u8 *)r6 + C + off` re-associates
//     back to `(off + rec) + C`.
//
// Each read-side base (`pb`) is declared after the preceding destination
// store and first call-argument load. This keeps the `subs`/`adds` pair after
// `ldr r0, [r6, #20/28/36]`, matching the ROM's argument evaluation order.
// At the third site, the raw helper result and destination pointer are also
// separate statements so the pointer add precedes the signed cast.
// The focused corpus probe matches all 180 bytes, including the literal pool.
void Rec35_PlacementEmit_16A20(void *rec) {
    u32 sp[2];
    register void *r6 __asm__("r6") = rec;
    __asm__ volatile("" : "+r"(r6));
    // i is zeroed BEFORE the ghost fetches (`mov r0,#0 / mov r8,r0` sits ahead of
    // the first `bl`), so the initialiser belongs to the declaration, not a
    // `for` prologue.
    int i = 0;
    u32 off;
    sp[0] = (u32)Rec35GhostSlot(0);
    sp[1] = (u32)Rec35GhostSlot(1);
    off = 0;
    for (; i <= 1; off += 12, i++) {
        s16 r4 = RS16((u8 *)(u32)sp[i] + 16);
        _080022E4(r4);
        u32 v1 = (u32)_08024D5C(r4);
        u8 *pa = (u8 *)r6 + 388;
        WR32(pa + off, v1);
        u32 arg0 = RD32((u8 *)r6 + 20);
        u8 *pb = (u8 *)r6 + 384;
        u32 arg2 = RD32(pb + off);
        R35_CALLEE(Sub_08007ABC, _08007ABC)(arg0, v1, arg2);
        u32 v2 = (u32)_08024C74(r4);
        u8 *pa2 = (u8 *)r6 + 412;
        WR32(pa2 + off, v2);
        u32 arg0b = RD32((u8 *)r6 + 28);
        u8 *pb2 = (u8 *)r6 + 408;
        u32 arg2b = RD32(pb2 + off);
        R35_CALLEE(Sub_08007ABC, _08007ABC)(arg0b, v2, arg2b);
        int raw3 = _080258A8(RS16((u8 *)(u32)sp[i] + 2));
        u8 *pa3 = (u8 *)r6 + 436;
        u8 *dst3 = pa3 + off;
        s16 v3 = (s16)raw3;
        WR32(dst3, (u32)v3);
        u32 arg0c = RD32((u8 *)r6 + 36);
        u8 *pb3 = (u8 *)r6 + 432;
        u32 arg2c = RD32(pb3 + off);
        R35_CALLEE(Sub_08007ABC, _08007ABC)(arg0c, (u32)v3, arg2c);
    }
}
void _080016A20(void *a) __attribute__((alias("Rec35_PlacementEmit_16A20")));

// ---------------------------------------------------------------------------
// _080016AF0(rec) — 8-way phase dispatcher (indirect table 0x08016B0C)
//
// Phase→body, read out of the ROM table words at 0x08016B0C + 4*phase:
//   p0 -> 0x08016BB0 (exit)   p1 -> 0x08016B2C   p2 -> 0x08016B40
//   p3 -> 0x08016B54          p4 -> 0x08016B66   p5/p6/p7 -> exit
// Bodies (address order == the order agbcc emits them):
//   0x08016B2C  _0800226C(+212, 64); [+196]=3
//   0x08016B40  _08002298(+276);      [+196]=4
//   0x08016B54  v=_080022C0(); if (v==1) [+194]=v
//   0x08016B66  v=_080022C0(); if (v!=1) exit;
//               CpuSet(+276, ghost0, 0x04000008), CpuSet(+308, ghost1,
//               0x04000008), SyncRec_16944, PlacementEmit_16A20, [+194]=v
// The old body had every arm shifted one phase early (its "p1" was really p2),
// carried a (correctly) missing _08002D974 leg out of p3, and pinned the last
// store to 1 instead of the saved _080022C0() result — which is why the ROM
// stashes it in r7 and stores r7.
void Rec35_Leaf_16AF0(void *rec) {
    // Raw s16 selector: `ldrsh`, then `cmp #7 / bhi` — no mask. The table is
    // sized from the case-label span, so case 0 and case 7 must be spelled out
    // for the 8-slot table.
    int p = RS16((u8 *)rec + 196);
    switch (p) {
    case 1:                                     // 0x08016B2C
        R35_CALLEE(Sub_0800226C, _0800226C)((u8 *)rec + 212, 64);
        WR16((u8 *)rec + 196, 3);
        break;
    case 2:                                     // 0x08016B40
        R35_CALLEE(Sub_08002298, _08002298)((u8 *)rec + 276);
        WR16((u8 *)rec + 196, 4);
        break;
    case 3: {                                   // 0x08016B54
        int v = R35_CALLEE(Sub_080022C0, _080022C0)();
        if (v == 1) WR16((u8 *)rec + 194, (u16)v);
        break;
    }
    case 4: {                                   // 0x08016B66
        int v = R35_CALLEE(Sub_080022C0, _080022C0)();
        if (v != 1) break;
        // ROM (asm/rec35_runtime.s:833-849):
        //   r4 = rec + 138*2; bl 0x08024BFC (ghost); r1 = ghost;
        //   r2 = pool _080016BB8 = 0x04000008; r0 = rec+276; bl 0x0802D974
        //   r4 = rec + 154*2; bl 0x08024BFC (ghost); r1 = ghost; r2 = same pool;
        //   r0 = rec+308; bl 0x0802D974
        // i.e. CpuSet(src = rec+276 / rec+308, dst = the ghost record,
        // ctrl = 0x04000008). The old form used dst 0x04000000 and put the ghost
        // pointer in the *ctrl* word, so the count came out of an address.
        R35_CALLEE(Sub_08002D974, sub_0802D974)((const void *)((u8 *)rec + 276), Rec35GhostSlot(0), 0x04000008u);
        R35_CALLEE(Sub_08002D974, sub_0802D974)((const void *)((u8 *)rec + 308), Rec35GhostSlot(1), 0x04000008u);
        R35_CALLEE(Rec35_SyncRec_16944, _080016944)(rec);
        R35_CALLEE(Rec35_PlacementEmit_16A20, _080016A20)(rec);
        WR16((u8 *)rec + 194, (u16)v);
        break;
    }
    case 0:
    case 5:
    case 6:
    case 7:
    default:
        break;
    }
}
void _080016AF0(void *a) __attribute__((alias("Rec35_Leaf_16AF0")));

// ---------------------------------------------------------------------------
// _080016BBC(rec) — 8-way phase dispatcher (indirect table 0x08016BDC)
//
// Phase→body out of the ROM table words at 0x08016BDC + 4*phase:
//   p0 -> 0x08016BFC   p1 -> 0x08016C48   p2 -> 0x08016C8C   p3 -> 0x08016CCC
//   p4 -> 0x08016D02   p6 -> 0x08016D90   p5/p7 -> 0x08016DB8 (exit)
// Corrections vs the previous body: _08002178 takes (id, v) — `movs r0,#1 /
// movs r1,#2` before both calls; p0 ends in _08024C18(s16[rec+340]); p1/p2 do
// NOT add rec to the _080240D0() result (it is `ret+32`, bare) and they run the
// same Sub_08025BF0/_08007ABC tail as p3/p4 (r1 = 4 and 2); p4 passes
// s16[rec+342] to both _08024C14 and the ghost fetch, and its _08024C10 calls
// take 0 and 1.
void Rec35_Leaf_16BBC(void *rec) {
    void *r5 = rec;
    int p = RS16((u8 *)r5 + 198);
    switch (p) {
    case 0: {                                   // 0x08016BFC
        // `int v = (u16)f()` (not `u16 v = (u16)f()`) is what lands the second
        // half of the narrowing pair in v's own register: `lsls r0,r0,#16 /
        // lsrs r4,r0,#16` instead of an extra `adds r4,r0,#0`.
        int v = (u16)_08002178(1, 2);
        if (v != 1) break;
        u8 *wa;
        { extern u8 Rec35Wa[];
          __asm__(".globl Rec35Wa\nRec35Wa = 0x03001780\n");
          wa = Rec35Wa; }
        if (*(wa + 0x10C3) == 1) { WR16((u8 *)r5 + 198, 2); break; }
        Rec35_Leaf_16AD4(r5);
        R35_CALLEE(Sub_08024C18, _08024C18)(RS16((u8 *)r5 + 340));
        WR16((u8 *)r5 + 198, (u16)v);
        break;
    }
    case 1: {                                   // 0x08016C48
        WR32((u8 *)r5 + 508, 0);
        WR32((u8 *)r5 + 512, 0);
        R35_CALLEE(Sub_0800226C, _0800226C)((u8 *)_080240D0() + 32, 0x09E0);
        WR16((u8 *)r5 + 198, 3);
        R35_CALLEE(Sub_08002618, _08002618)(1, 1);
        WR16((u8 *)r5 + 192, 1);
        void *r4 = (u8 *)r5 + 480;
        R35_CALLEE(Sub_08025BF0, _08025BF0)(r4, 4);
        R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)r5 + 44), RD32((u8 *)r5 + 484), RD32(r4));
        break;
    }
    case 2: {                                   // 0x08016C8C
        WR32((u8 *)r5 + 508, 0);
        WR32((u8 *)r5 + 512, 0);
        R35_CALLEE(Sub_08002298, _08002298)((u8 *)_080240D0() + 32);
        WR16((u8 *)r5 + 198, 4);
        R35_CALLEE(Sub_08002618, _08002618)(1, 1);
        WR16((u8 *)r5 + 192, 1);
        void *r4 = (u8 *)r5 + 480;
        R35_CALLEE(Sub_08025BF0, _08025BF0)(r4, 2);
        R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)r5 + 44), RD32((u8 *)r5 + 484), RD32(r4));
        break;
    }
    case 3:                                     // 0x08016CCC
        if (R35_CALLEE(Sub_080022C0, _080022C0)() != 1) break;
        WR16((u8 *)r5 + 198, 6);
        Rec35_Leaf_16AD4(r5);
        R35_CALLEE(Sub_08025BF0, _08025BF0)((u8 *)r5 + 480, 3);
        R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)r5 + 44), RD32((u8 *)r5 + 484), RD32((u8 *)r5 + 480));
        break;
    case 4: {                                   // 0x08016D02
        if (R35_CALLEE(Sub_080022C0, _080022C0)() != 1) break;
        Rec35_Leaf_16AD4(r5);
        void *r4 = (u8 *)r5 + 480;
        R35_CALLEE(Sub_08025BF0, _08025BF0)(r4, 1);
        R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)r5 + 44), RD32((u8 *)r5 + 484), RD32(r4));
        u8 *r6 = (u8 *)r5 + 342;                // ROM keeps this in r6 across calls
        R35_CALLEE(Sub_08024C14, _08024C14)(RS16(r6), (int)(uintptr_t)((u8 *)_080240D0() + 32));
        // ROM (asm/rec35_runtime.s:1049-1057): r4 = rec + s16[rec+340]*32 + 276
        // (the `subs r2, #64` reuses the 340 constant); dst = the ghost record
        // (bl 0x08024BFC), ctrl = pool _080016D8C = 0x04000008.
        // Parenthesised so the offset folds BEFORE the base add: the ROM is
        // `lsls r4,#5 / subs r2,#64 / adds r4,r4,r2 / adds r4,r5,r4`, not
        // `… / adds r4,r5,r4 / subs r2,#64 / adds r4,r4,r2`.
        const void *dst = (const void *)((u8 *)r5 + (RS16((u8 *)r5 + 340) * 32 + 276));
        R35_CALLEE(Sub_08002D974, sub_0802D974)(dst, Rec35GhostSlot(RS16(r6)), 0x04000008u);
        R35_CALLEE(Sub_08024C10, _08024C10)(0);
        R35_CALLEE(Sub_08024C10, _08024C10)(1);
        R35_CALLEE(Sub_08024C1C, _08024C1C)();
        R35_CALLEE(Rec35_SyncRec_16944, _080016944)(r5);
        R35_CALLEE(Rec35_PlacementEmit_16A20, _080016A20)(r5);
        WR16((u8 *)r5 + 198, 6);
        break;
    }
    case 6: {                                   // 0x08016D90
        WR32((u8 *)r5 + 512, 1);
        int v = (u16)_08002178(1, 6);
        if (v != 1) break;
        WR32((u8 *)r5 + 508, (u32)v);
        WR16((u8 *)r5 + 194, 6);
        break;
    }
    case 5:                                     // 0x08016DB8 (fallthrough exit)
    case 7:
    default:
        break;
    }
}
// Match the ROM's two zero pad bytes instead of GAS's Thumb `46 c0` fill.
__asm__(".align 2, 0");
void _080016BBC(void *a) __attribute__((alias("Rec35_Leaf_16BBC")));

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
// `asm/rec35_runtime.s:3132` still branches to `bl sub_080016DC0`, so the
// `sub_` twin must exist in C once this body is promoted (the manifest's
// `export` list only re-emits a label whose VMA equals the entry's).
void _080016DC0(void *c) __attribute__((alias("Rec35_Leaf_16DC0")));
void sub_080016DC0(void *c) __attribute__((alias("Rec35_Leaf_16DC0")));

// ---------------------------------------------------------------------------
// _080016DE4(rec) — 9-way phase dispatcher (indirect table 0x08016E04)
//
// Phase→body, straight from the ROM table (0x08016E04 + 4*phase):
//   p0 -> 0x08016E28   p1 -> 0x08016E44   p3 -> 0x08016EC4
//   p5 -> 0x08016E30   p7 -> 0x08016E8C   p2/p4/p6/p8 -> exit
// The previous body had phases 1 and 5 swapped and several wrong immediates:
// the "p1" arm was really p5, `Sub_0802B368(3)` is not in this function at all,
// the sound test is `R35_CALLEE(Sub_0802B3B8, _0802B3B8)(3)` (and its false arm writes [+60]=0), the
// pair cells are +496/+492 (not +504/+500), the p1 arm ends with [+194]=2, and
// the p3 arm ends with a second SyncRec. agbcc emits case bodies in SOURCE
// order, so the arms are written 0, 5, 1, 7, 3 to lay out at the ROM addresses.
void Rec35_Leaf_16DE4(void *rec) {
    void *r5 = rec;
    int p = RS16((u8 *)r5 + 194);
    switch (p) {
    case 0:                                     // 0x08016E28
        R35_CALLEE(Rec35_Leaf_16AF0, _080016AF0)(r5);
        break;
    case 5:                                     // 0x08016E30
        if ((R35_CALLEE(Sub_0802B3B8, _0802B3B8)(3) << 24) == 0) R35_CALLEE(Rec35_Leaf_16BBC, _080016BBC)(r5);
        break;
    case 1: {                                   // 0x08016E44
        void *r4 = Rec35GhostSlot(0);
        void *r1 = Rec35GhostSlot(1);
        if (RD16P(r4) == 0 && RD16P(r1) == 0) { WR16((u8 *)r5 + 194, 7); break; }
        u8 *q = (u8 *)r5 + 496;
        WR32(q, 0);
        // ROM puts the constant in r1 and the cell in r2 (0x08007ABC's 2nd/3rd
        // parameters are in that order at this call site).
        R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)r5 + 52), 0, RD32(q - 4));
        WR16((u8 *)r5 + 194, 2);
        break;
    }
    case 7: {                                   // 0x08016E8C
        R35_CALLEE(Sub_08002618, _08002618)(1, 1);
        WR16((u8 *)r5 + 192, 1);
        void *r4 = (u8 *)r5 + 480;
        R35_CALLEE(Sub_08025BF0, _08025BF0)(r4, 0);
        R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)r5 + 44), RD32((u8 *)r5 + 484), RD32(r4));
        WR16((u8 *)r5 + 194, 8);
        break;
    }
    case 3: {                                   // 0x08016EC4
        // Written as if/else so agbcc branches OVER the main block (`bne` to the
        // out-of-line [+60]=0 arm); the `if (…) != 0 { … break; }` spelling made
        // it branch the other way.
        if ((R35_CALLEE(Sub_0802B3B8, _0802B3B8)(3) << 24) == 0) {
            WR16((u8 *)r5 + 60, 1);
            WR32((u8 *)r5 + 496, 2);
            R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)r5 + 52), 2, RD32((u8 *)r5 + 492));
            R35_CALLEE(Sub_08024C1C, _08024C1C)();
            R35_CALLEE(Rec35_SyncRec_16944, _080016944)(r5);
            R35_CALLEE(Rec35_PlacementEmit_16A20, _080016A20)(r5);
            WR16((u8 *)r5 + 194, 4);
            WR16((u8 *)r5 + 184, 1);
            WR16((u8 *)r5 + 182, 1);
            WR16((u8 *)r5 + 186, 1);
            R35_CALLEE(Rec35_SyncRec_16944, _080016944)(r5);
        } else {
            WR16((u8 *)r5 + 60, 0);
        }
        break;
    }
    case 2:                                     // 0x08016F20 (exit)
    case 4:
    case 6:
    case 8:
    default:
        break;
    }
}
// Match the ROM's two zero pad bytes instead of GAS's Thumb `46 c0` fill.
__asm__(".align 2, 0");
void _080016DE4(void *a) __attribute__((alias("Rec35_Leaf_16DE4")));

// ---------------------------------------------------------------------------
// _080016FD0(rec) — race-scene constructor (1048 B).
//
// This VMA is NOT the 5-way phase dispatcher. That body belongs to 0x08016F28
// (`Rec35_Dispatch_16F28`, src/rec35_runtime.c), which ends at 0x08016FCE — the
// dispatcher used to be bound here as well, which is why this function probed as
// `PARTIAL 5/1048 bytes, candidate 140`.
//
// ROM shape (asm/rec35_runtime.s:1351-1823):
//   * two sub_08007770 resource binds (0x0831F244 / 0x082B7410), [+128]=6;
//   * the 0x04B68 lookup whose s16 result (39 / 40) picks the record kind when
//     WA+0xFBC is not 3; the kind cell is +180 and r9 keeps its address;
//   * seven binds (0x0831F244, 0x08320060, 0x08324F00, 0x082DFBDC,
//     0x082C4228, 0x082C4458, 0x08325A7C) and three 2-lap sub_0800572C →
//     sub_08007ABC pair loops — cells +456/+460 (arg [rec+12]), +432/+436
//     (arg [rec+36], value signed-narrowed), +384/+388 + +408/+412 (args
//     [rec+20] / [rec+28]) — the same lap layout `Rec35_PlacementEmit_16A20`
//     walks, plus the 0x08025BC8(rec+480, 5) bind feeding [rec+44]/[rec+484];
//   * WA mirrors: [+364] = WA+0xFBE, [+362] = WA+0x574, [+348] = 20;
//   * a 3-way switch on RS16(rec+180) (1 / 0 / 3, default) before the shared
//     tail ([+344]=0, [+508]=1, [+512]=1).
//
// The WA base must stay a SYMBOL: the front end folds `0x03001780 + 0xFBC`
// into one pool word, while the ROM really loads 0x03001780, loads 0x00000FBC
// and adds them (same for 0xFBE / 0x574 / 0x10C3).
__attribute__((naked)) void Rec35_Scene_16FD0(void *rec_) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, sl\n"
        "\tmov\tr6, r9\n"
        "\tmov\tr5, r8\n"
        "\tpush\t{r5, r6, r7}\n"
        "\tsub\tsp, #56\n"
        "\tadds\tr6, r0, #0\n"
        "\tldr r1, .Lr_080016fd0_08001702C\n"
        "\tmovs\tr0, #4\n"
        "\tstr\tr0, [sp, #0]\n"
        "\tmovs\tr5, #1\n"
        "\tstr\tr5, [sp, #4]\n"
        "\tmovs\tr0, #0\n"
        "\tmovs\tr2, #2\n"
        "\tmovs\tr3, #0\n"
        "\tbl sub_08007770\n"
        "\tldr r1, .Lr_080016fd0_080017030\n"
        "\tmovs\tr4, #0\n"
        "\tstr\tr4, [sp, #0]\n"
        "\tmovs\tr0, #3\n"
        "\tstr\tr0, [sp, #4]\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr2, #1\n"
        "\tmovs\tr3, #0\n"
        "\tbl sub_08007770\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #128\n"
        "\tmovs\tr0, #6\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tldr r0, .Lr_080016fd0_080017034\n"
        "\tldr r1, .Lr_080016fd0_080017038\n"
        "\tadds\tr0, r0, r1\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr2, [r0, r1]\n"
        "\tcmp\tr2, #3\n"
        "\tbne .Lr_080016fd0_08001703C\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #140\n"
        "\tmovs\tr1, #19\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tadds\tr0, #40\n"
        "\tstrh\tr2, [r0, #0]\n"
        "\tb .Lr_080016fd0_08001706C\n"
        "\tmovs\tr0, r0\n"
        ".Lr_080016fd0_08001702C: .word 0x0831F244\n"
        ".Lr_080016fd0_080017030: .word 0x082B7410\n"
        ".Lr_080016fd0_080017034: .word 0x03001780\n"
        ".Lr_080016fd0_080017038: .word 0x00000FBC\n"
        ".Lr_080016fd0_08001703C:\n"
        "\tbl 0x08004B68\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r0, r2]\n"
        "\tcmp\tr0, #39\n"
        "\tbeq .Lr_080016fd0_080017060\n"
        "\tmovs\tr1, #180\n"
        "\tadds\tr1, r1, r6\n"
        "\tmov\tr9, r1\n"
        "\tcmp\tr0, #40\n"
        "\tbne .Lr_080016fd0_08001706E\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #140\n"
        "\tmovs\tr0, #19\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tmov\tr2, r9\n"
        "\tstrh\tr5, [r2, #0]\n"
        "\tb .Lr_080016fd0_08001706E\n"
        ".Lr_080016fd0_080017060:\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #140\n"
        "\tmovs\tr1, #18\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tadds\tr0, #40\n"
        "\tstrh\tr4, [r0, #0]\n"
        ".Lr_080016fd0_08001706C:\n"
        "\tmov\tr9, r0\n"
        ".Lr_080016fd0_08001706E:\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #152\n"
        "\tmovs\tr0, #1\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tadds\tr5, r6, #0\n"
        "\tadds\tr5, #56\n"
        "\tadds\tr0, r5, #0\n"
        "\tbl sub_0800DAB8\n"
        "\tldr r4, .Lr_080016fd0_0800172E0\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr1, r6, #0\n"
        "\tbl sub_0800798C\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl sub_08007A58\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #3\n"
        "\tbl sub_080075E8\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #1\n"
        "\tmovs\tr2, #6\n"
        "\tbl sub_080075E8\n"
        "\tldr r4, .Lr_080016fd0_0800172E4\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #8\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_0800798C\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #5\n"
        "\tbl sub_080075E8\n"
        "\tstr\tr5, [sp, #36]\n"
        "\tadds\tr7, r6, #0\n"
        "\tadds\tr7, #48\n"
        "\tmovs\tr0, #32\n"
        "\tadds\tr0, r0, r6\n"
        "\tmov\tr8, r0\n"
        "\tmovs\tr1, #16\n"
        "\tadds\tr1, r1, r6\n"
        "\tmov\tsl, r1\n"
        "\tadds\tr2, r6, #0\n"
        "\tadds\tr2, #24\n"
        "\tstr\tr2, [sp, #28]\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #40\n"
        "\tstr\tr0, [sp, #32]\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #168\n"
        "\tstr\tr1, [sp, #8]\n"
        "\tadds\tr2, #148\n"
        "\tstr\tr2, [sp, #12]\n"
        "\tadds\tr0, #36\n"
        "\tstr\tr0, [sp, #52]\n"
        "\tsubs\tr1, #100\n"
        "\tstr\tr1, [sp, #48]\n"
        "\tsubs\tr2, #108\n"
        "\tstr\tr2, [sp, #40]\n"
        "\tsubs\tr0, #10\n"
        "\tstr\tr0, [sp, #44]\n"
        "\tadds\tr1, #114\n"
        "\tstr\tr1, [sp, #16]\n"
        "\tadds\tr2, #124\n"
        "\tstr\tr2, [sp, #20]\n"
        "\tadds\tr0, #124\n"
        "\tstr\tr0, [sp, #24]\n"
        "\tmovs\tr4, #0\n"
        "\tmovs\tr5, #1\n"
        ".Lr_080016fd0_080017102:\n"
        "\tmovs\tr0, #150\n"
        "\tbl sub_0800572C\n"
        "\tmovs\tr1, #228\n"
        "\tlsls\tr1, r1, #1\n"
        "\tadds\tr2, r6, r1\n"
        "\tadds\tr2, r2, r4\n"
        "\tstr\tr0, [r2, #0]\n"
        "\tadds\tr1, #4\n"
        "\tadds\tr0, r6, r1\n"
        "\tadds\tr0, r0, r4\n"
        "\tmovs\tr1, #1\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r6, #12]\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadds\tr4, #12\n"
        "\tsubs\tr5, #1\n"
        "\tcmp\tr5, #0\n"
        "\tbge .Lr_080016fd0_080017102\n"
        "\tldr r0, .Lr_080016fd0_0800172E8\n"
        "\tadds\tr1, r7, #0\n"
        "\tbl sub_0800798C\n"
        "\tmovs\tr0, #30\n"
        "\tbl sub_0800572C\n"
        "\tmovs\tr1, #246\n"
        "\tlsls\tr1, r1, #1\n"
        "\tadds\tr2, r6, r1\n"
        "\tstr\tr0, [r2, #0]\n"
        "\tmovs\tr0, #248\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr1, r6, r0\n"
        "\tmovs\tr0, #0\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tldr\tr0, [r6, #52]\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tmovs\tr1, #0\n"
        "\tbl sub_08007ABC\n"
        "\tldr r4, .Lr_080016fd0_0800172EC\n"
        "\tadds\tr0, r4, #0\n"
        "\tmov\tr1, r8\n"
        "\tbl sub_0800798C\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #7\n"
        "\tbl sub_080075E8\n"
        "\tldr r1, .Lr_080016fd0_0800172F0\n"
        "\tmov\tr8, r1\n"
        "\tmovs\tr5, #0\n"
        "\tmovs\tr7, #1\n"
        ".Lr_080016fd0_080017172:\n"
        "\tmovs\tr0, #12\n"
        "\tbl sub_0800572C\n"
        "\tmovs\tr2, #216\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr4, r6, r2\n"
        "\tadds\tr4, r4, r5\n"
        "\tstr\tr0, [r4, #0]\n"
        "\tmov\tr1, r8\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tbl 0x080258A8\n"
        "\tadds\tr1, r0, #0\n"
        "\tmovs\tr2, #218\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr0, r6, r2\n"
        "\tadds\tr0, r0, r5\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r6, #36]\n"
        "\tldr\tr2, [r4, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadds\tr5, #12\n"
        "\tsubs\tr7, #1\n"
        "\tcmp\tr7, #0\n"
        "\tbge .Lr_080016fd0_080017172\n"
        "\tldr r0, .Lr_080016fd0_0800172F4\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #4\n"
        "\tbl sub_080075E8\n"
        "\tldr r0, .Lr_080016fd0_0800172F8\n"
        "\tmov\tr1, sl\n"
        "\tbl sub_0800798C\n"
        "\tldr r0, .Lr_080016fd0_0800172FC\n"
        "\tldr\tr1, [sp, #28]\n"
        "\tbl sub_0800798C\n"
        "\tmovs\tr4, #0\n"
        "\tmovs\tr7, #1\n"
        ".Lr_080016fd0_0800171CA:\n"
        "\tmovs\tr0, #7\n"
        "\tbl sub_0800572C\n"
        "\tmovs\tr1, #192\n"
        "\tlsls\tr1, r1, #1\n"
        "\tadds\tr2, r6, r1\n"
        "\tadds\tr2, r2, r4\n"
        "\tstr\tr0, [r2, #0]\n"
        "\tadds\tr1, #4\n"
        "\tadds\tr0, r6, r1\n"
        "\tadds\tr0, r0, r4\n"
        "\tmovs\tr1, #4\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r6, #20]\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tmovs\tr0, #7\n"
        "\tbl sub_0800572C\n"
        "\tmovs\tr1, #204\n"
        "\tlsls\tr1, r1, #1\n"
        "\tadds\tr2, r6, r1\n"
        "\tadds\tr2, r2, r4\n"
        "\tstr\tr0, [r2, #0]\n"
        "\tadds\tr1, #4\n"
        "\tadds\tr0, r6, r1\n"
        "\tadds\tr0, r0, r4\n"
        "\tmovs\tr1, #50\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r6, #28]\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadds\tr4, #12\n"
        "\tsubs\tr7, #1\n"
        "\tcmp\tr7, #0\n"
        "\tbge .Lr_080016fd0_0800171CA\n"
        "\tldr r4, .Lr_080016fd0_080017300\n"
        "\tadds\tr0, r4, #0\n"
        "\tldr\tr1, [sp, #32]\n"
        "\tbl sub_0800798C\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #8\n"
        "\tbl sub_080075E8\n"
        "\tmovs\tr2, #240\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr4, r6, r2\n"
        "\tadds\tr0, r4, #0\n"
        "\tmovs\tr1, #5\n"
        "\tbl 0x08025BC8\n"
        "\tldr\tr0, [r6, #44]\n"
        "\tmovs\tr2, #242\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr1, r6, r2\n"
        "\tldr\tr1, [r1, #0]\n"
        "\tldr\tr2, [r4, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tmovs\tr0, #176\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr4, r6, r0\n"
        "\tldr\tr1, [sp, #8]\n"
        "\tstr\tr4, [r1, #0]\n"
        "\tldr\tr2, [sp, #36]\n"
        "\tldr\tr0, [sp, #12]\n"
        "\tstr\tr2, [r0, #0]\n"
        "\tmovs\tr7, #0\n"
        "\tstr\tr7, [r6, #56]\n"
        "\tmovs\tr5, #1\n"
        "\tstrh\tr5, [r6, #60]\n"
        "\tmovs\tr2, #32\n"
        "\tnegs\tr2, r2\n"
        "\tldr\tr0, [sp, #52]\n"
        "\tmovs\tr1, #0\n"
        "\tbl sub_0800D77C\n"
        "\tldr\tr0, [sp, #48]\n"
        "\tmovs\tr1, #0\n"
        "\tmovs\tr2, #160\n"
        "\tbl sub_0800D77C\n"
        "\tldr\tr1, [sp, #40]\n"
        "\tstrh\tr5, [r1, #0]\n"
        "\tldr\tr2, [sp, #44]\n"
        "\tstrh\tr5, [r2, #0]\n"
        "\tldr\tr0, [sp, #16]\n"
        "\tstrh\tr5, [r0, #0]\n"
        "\tmovs\tr1, #2\n"
        "\tmov\tr8, r1\n"
        "\tstr\tr1, [r4, #0]\n"
        "\tldr\tr2, [sp, #20]\n"
        "\tstrh\tr7, [r2, #0]\n"
        "\tldr\tr0, [sp, #24]\n"
        "\tstrh\tr7, [r0, #0]\n"
        "\tmov\tr1, r9\n"
        "\tldrh\tr1, [r1, #0]\n"
        "\tcmp\tr1, #3\n"
        "\tbeq .Lr_080016fd0_08001729E\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl sub_080016944\n"
        ".Lr_080016fd0_08001729E:\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl sub_080016A20\n"
        "\tldr r4, .Lr_080016fd0_080017304\n"
        "\tldr r2, .Lr_080016fd0_080017308\n"
        "\tadds\tr0, r4, r2\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tmovs\tr2, #182\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr0, r6, r2\n"
        "\tstrh\tr1, [r0, #0]\n"
        "\tldr r1, .Lr_080016fd0_08001730C\n"
        "\tadds\tr0, r4, r1\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tsubs\tr2, #2\n"
        "\tadds\tr1, r6, r2\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr0, #174\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr1, r6, r0\n"
        "\tmovs\tr0, #20\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmov\tr1, r9\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tcmp\tr0, #1\n"
        "\tbeq .Lr_080016fd0_08001733C\n"
        "\tcmp\tr0, #1\n"
        "\tbgt .Lr_080016fd0_080017310\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lr_080016fd0_080017316\n"
        "\tb .Lr_080016fd0_0800173B8\n"
        "\tmovs\tr0, r0\n"
        ".Lr_080016fd0_0800172E0: .word 0x0831F244\n"
        ".Lr_080016fd0_0800172E4: .word 0x08320060\n"
        ".Lr_080016fd0_0800172E8: .word 0x08324F00\n"
        ".Lr_080016fd0_0800172EC: .word 0x082DFBDC\n"
        ".Lr_080016fd0_0800172F0: .word 0x03001CF6\n"
        ".Lr_080016fd0_0800172F4: .word 0x082C4228\n"
        ".Lr_080016fd0_0800172F8: .word 0x082C4458\n"
        ".Lr_080016fd0_0800172FC: .word 0x08325A7C\n"
        ".Lr_080016fd0_080017300: .word 0x082B7410\n"
        ".Lr_080016fd0_080017304: .word 0x03001780\n"
        ".Lr_080016fd0_080017308: .word 0x00000FBE\n"
        ".Lr_080016fd0_08001730C: .word 0x00000574\n"
        ".Lr_080016fd0_080017310:\n"
        "\tcmp\tr0, #3\n"
        "\tbeq .Lr_080016fd0_080017356\n"
        "\tb .Lr_080016fd0_0800173B8\n"
        ".Lr_080016fd0_080017316:\n"
        "\tbl 0x0802412C\n"
        "\tmovs\tr1, #248\n"
        "\tlsls\tr1, r1, #1\n"
        "\tadds\tr0, r6, r1\n"
        "\tmov\tr2, r8\n"
        "\tstr\tr2, [r0, #0]\n"
        "\tldr\tr0, [r6, #52]\n"
        "\tmovs\tr2, #246\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr1, r6, r2\n"
        "\tldr\tr2, [r1, #0]\n"
        "\tmovs\tr1, #2\n"
        "\tbl sub_08007ABC\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #200\n"
        "\tstrh\tr7, [r0, #0]\n"
        "\tb .Lr_080016fd0_0800173B8\n"
        ".Lr_080016fd0_08001733C:\n"
        "\tmovs\tr1, #248\n"
        "\tlsls\tr1, r1, #1\n"
        "\tadds\tr0, r6, r1\n"
        "\tstr\tr5, [r0, #0]\n"
        "\tldr\tr0, [r6, #52]\n"
        "\tmovs\tr2, #246\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr1, r6, r2\n"
        "\tldr\tr2, [r1, #0]\n"
        "\tmovs\tr1, #1\n"
        "\tbl sub_08007ABC\n"
        "\tb .Lr_080016fd0_0800173B8\n"
        ".Lr_080016fd0_080017356:\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #194\n"
        "\tstrh\tr7, [r0, #0]\n"
        "\tldr r0, .Lr_080016fd0_080017378\n"
        "\tbl 0x08002124\n"
        "\tldr r1, .Lr_080016fd0_08001737C\n"
        "\tadds\tr0, r4, r1\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tcmp\tr0, #1\n"
        "\tbne .Lr_080016fd0_080017380\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #196\n"
        "\tmov\tr2, r8\n"
        "\tstrh\tr2, [r0, #0]\n"
        "\tb .Lr_080016fd0_0800173B8\n"
        "\tmovs\tr0, r0\n"
        ".Lr_080016fd0_080017378: .word 0x00001390\n"
        ".Lr_080016fd0_08001737C: .word 0x000010C3\n"
        ".Lr_080016fd0_080017380:\n"
        "\tbl 0x08024C1C\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl sub_080016944\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl sub_080016A20\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #196\n"
        "\tstrh\tr5, [r0, #0]\n"
        "\tmovs\tr0, #0\n"
        "\tbl 0x08024BFC\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #212\n"
        "\tldr r4, .Lr_080016fd0_0800173E4\n"
        "\tadds\tr2, r4, #0\n"
        "\tbl 0x0802D974\n"
        "\tmovs\tr0, #1\n"
        "\tbl 0x08024BFC\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #244\n"
        "\tadds\tr2, r4, #0\n"
        "\tbl 0x0802D974\n"
        ".Lr_080016fd0_0800173B8:\n"
        "\tmovs\tr0, #172\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr1, r6, r0\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr1, #254\n"
        "\tlsls\tr1, r1, #1\n"
        "\tadds\tr0, r6, r1\n"
        "\tmovs\tr1, #1\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tmovs\tr2, #128\n"
        "\tlsls\tr2, r2, #2\n"
        "\tadds\tr0, r6, r2\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tadd\tsp, #56\n"
        "\tpop\t{r3, r4, r5}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tmov\tsl, r5\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        ".Lr_080016fd0_0800173E4: .word 0x04000008\n"
        ".syntax divided\n"
    );
}
void _080016FD0(void *a) __attribute__((alias("Rec35_Scene_16FD0")));

// ---------------------------------------------------------------------------
// _080017440(rec, a, id) — selection key handler
// Shares the entry narrowing + tail (64/128 step, clamp, sync) with
// Rec35_Input_17528; the 4-state early-out replaces the whole tail.
void Rec35_Input_17440(void *rec, int a, int id) {
    (void)a;
    int nid = (u16)id;
    void *r2;
    void *p480;
    void *r7 = (u8 *)rec + 182;
    int old = RD16P(r7);
    void *r4 = (u8 *)rec + 200;
    // Inverted guard: the ROM branches *away* to the state-4 fixup, so the
    // state-4 block is emitted last instead of first.
    if (RD16P(r4) != 4) {
        if (nid == 2) {
            Sub_0802B368(4);
            WR32((u8 *)rec + 56, 10);
            WR16((u8 *)rec + 60, 0);
            WR32((u8 *)rec + 88, 10);
            WR32((u8 *)rec + 84, 0);
        }
        if (nid == 1) {
            // 3-arm switch. Cases 1 and 2 must be *separate* case labels with
            // duplicated bodies: a grouped `case 1: case 2:` becomes one range
            // node (`cmp #1 / blt / ble`) instead of the ROM's balanced
            // `cmp #2 / beq ... cmp #2 / bgt ... cmp #1 / beq` tree, which is
            // then cross-jumped to the shared block.
            switch (RS16(r7)) {
            case 1: {
                // ROM materialises the 0 *before* the rec+200 store and then
                // reuses the register (`movs r0,#0 / strh r6,[r4] /
                // strh r0,[r5,#60]`); an explicit zero variable keeps that
                // order instead of rematerialising it at the store.
                u16 z;
                Sub_0802B368(1);
                z = 0;
                WR16(r4, nid);
                WR16((u8 *)rec + 60, z);
                p480 = (u8 *)rec + 480;
                R35_CALLEE(Sub_08025BF0, _08025BF0)(p480, 9);
                R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)rec + 44), RD32((u8 *)rec + 484), RD32(p480));
                R35_CALLEE(Sub_08002618, _08002618)(1, 1);
                WR16((u8 *)rec + 192, nid);
                break;
            }
            case 2: {
                u16 z;
                Sub_0802B368(1);
                z = 0;
                WR16(r4, nid);
                WR16((u8 *)rec + 60, z);
                p480 = (u8 *)rec + 480;
                R35_CALLEE(Sub_08025BF0, _08025BF0)(p480, 9);
                R35_CALLEE(Sub_08007ABC, _08007ABC)(RD32((u8 *)rec + 44), RD32((u8 *)rec + 484), RD32(p480));
                R35_CALLEE(Sub_08002618, _08002618)(1, 1);
                WR16((u8 *)rec + 192, nid);
                break;
            }
            case 3:
                Rec35_Leaf_17414(rec);
                break;
            default:
                break;
            }
        }
        r2 = (u8 *)rec + 182;
        if (nid == 64)  { u16 v = RD16P(r2) - 1;  WR16(r2, v); }
        if (nid == 128) { u16 v = RD16P(r2) + 1;  WR16(r2, v); }
        {
            // Same clamp shape as Rec35_Input_17528 (pointer in r2, clamp base
            // copied to r1 with an r3=0 index scratch).
            register void *q __asm__("r1") = (u8 *)rec + 182;
            __asm__ volatile("" : "+r"(q));
            if (RS16(q) < 1) WR16(q, 1);
            if (RS16(q) > 3) WR16(q, 3);
        }
        if (old != RD16P(r2)) {
            R35_CALLEE(Rec35_SyncRec_16944, _080016944)(rec);
            Sub_0802B368(2);
        }
    } else if (nid == 1) {
        WR16(r4, 3);
    }
}
void _080017440(void *a, int b, int c) __attribute__((alias("Rec35_Input_17440")));

// ---------------------------------------------------------------------------
// _080017528(rec, a, id) — alternate selection key handler
// ROM narrows the (int) id at entry (`lsls r2,#16; lsrs r6,r2,#16`) and keys
// every branch off the narrowed value; `rec` keeps the r0 value for the first
// call but gets a callee-saved home (r4) for the rest.
void Rec35_Input_17528(void *rec, int a, int id) {
    (void)a;
    int nid = (u16)id;
    void *r5 = (u8 *)rec + 182;
    u16 old = RD16P(r5);
    void *p;
    if (nid == 2) Rec35_Leaf_17414(rec);
    if (nid == 1) {
        // 3-arm switch on the raw s16: agbcc emits the ROM's balanced
        // `cmp #2 / beq; cmp #2 / bgt; cmp #1 / beq` decision tree only here.
        switch (RS16(r5)) {
        case 1: R35_CALLEE(Sub_08024C08, _08024C08)(0); Rec35_Leaf_173E8(rec); break;
        case 2: R35_CALLEE(Sub_08024C08, _08024C08)(1); Rec35_Leaf_173E8(rec); break;
        case 3: Rec35_Leaf_173E8(rec); break;
        default: goto skip;
        }
    }
skip:
    p = (u8 *)rec + 182;
    if (nid == 64)  { u16 v = RD16P(p) - 1;  WR16(p, v); }
    if (nid == 128) { u16 v = RD16P(p) + 1;  WR16(p, v); }
    {
        // ROM keeps the tail pointer in r2 and the clamp on a *copy* in r1
        // with a r3=0 index scratch (`adds r1,r2,#0; movs r3,#0;
        // ldrsh r0,[r1,r3]`). Pinning the clamp base to r1 forces agbcc to
        // push `p` off r1 and materialise the copy.
        register void *q __asm__("r1") = (u8 *)rec + 182;
        __asm__ volatile("" : "+r"(q));
        if (RS16(q) < 1) WR16(q, 1);
        if (RS16(q) > 3) WR16(q, 3);
    }
    if (old != RD16P(p)) {
        Rec35_SyncRec_16944(rec);
        Sub_0802B368(2);
    }
}
void _080017528(void *a, int b, int c) __attribute__((alias("Rec35_Input_17528")));

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
    // ROM parks the two ghost-slot pointers in r5/r6 and the later state in
    // r7/r8/r9; agbcc prefers the high callee-saved pair for them, so pin.
    register void *r5 __asm__("r5") = Rec35GhostSlot(0);
    __asm__ volatile("" : "+r"(r5));
    register void *r6 __asm__("r6") = Rec35GhostSlot(1);
    __asm__ volatile("" : "+r"(r6));
    u16 zero;                                   // ROM keeps 0 in r9
    void *r8;                                   // r8 = rec+194 (the FSM cell)
    int mode;
    if (RD16P((u8 *)rec + 188) == 2) {
        if ((u16)_08002178(0, 5) == 2 && (u16)_08002178(1, 5) == 2) {
            WR16((u8 *)rec + 190, 0);
            sub_080017414(rec);
        }
    }
    register u8 *statusp __asm__("r0") = (u8 *)rec + 188;
    mode = RS16(statusp);
    if (mode != 1) return;
    statusp += 2;
    __asm__ volatile("" : "+r"(statusp));
    zero = 0;                                   // 0x0801761C: movs r2,#0; mov r9,r2
    WR16(statusp, zero);
    r8 = (u8 *)rec + 194;
    // Two-case dispatch is a plain chain; the inner sel dispatch is a 3-arm
    // switch (balanced `cmp #2 / beq; cmp #2 / bgt; cmp #1 / beq` tree).
    switch (RS16(r8)) {
    case 2:
        if ((u16)_08002178(1, 4) != 2) return;
        register u16 *slotp __asm__("r0");
        switch (RS16((u8 *)rec + 182)) {
        case 1:
            slotp = (u16 *)r5;
            __asm__ volatile("" : "+r"(slotp));
            if (RD16P(slotp) == 1) {
                sub_0800168B8(1);
                WR16((u8 *)rec + 340, zero);
            } else {
                goto mode2_slot_fail;
            }
            goto mode2_done;
        case 2:
            slotp = (u16 *)r6;
            __asm__ volatile("" : "+r"(slotp));
            if (RD16P(slotp) == 1) {
                sub_0800168B8(1);
                /* ROM uses r2 for the scaled +340 offset. */
                __asm__ volatile(".hword 0x22AA, 0x0052, 0x18A0, 0x8007");
            } else {
                goto mode2_slot_fail;
            }
            /* Fall through to the shared [+194]=3 update. */
mode2_done:
            register u16 state_value __asm__("r0") = 3;
            register u16 *state_dest __asm__("r1") = (u16 *)r8;
            __asm__ volatile("" : "+r"(state_value), "+r"(state_dest));
            *state_dest = state_value;
            return;
mode2_slot_fail:
            sub_0800168B8(10);
            return;
        case 3:
            goto mode2_sel3;
        default:
            return;
        }
mode2_sel3:
        sub_080017414(rec);
        return;
    case 4: {
        u8 *wa;
        if ((u16)_08002178(1, 4) != 4) return;
        register s32 sel __asm__("r0") =
            (s32)(uintptr_t)((u8 *)rec + 182);
        // C supplies the ROM's zero index in r2; the signed indexed load stays asm.
        register int zeroIndex __asm__("r2") = 0;
        __asm__ volatile("" : : "r"(zeroIndex));
        __asm__ volatile("ldrsh %0, [%0, r2]"
                         : "+r"(sel) : : "memory");
        switch (sel) {
        case 1:
            sub_0800168B8(1);
            *(volatile u16 *)((u8 *)rec + 342) = zero;
            goto mode4_done;
        case 2:
            sub_0800168B8(1);
            {
                register int byteOffset __asm__("r2") = 171;
                byteOffset <<= 1;
                __asm__ volatile("" : "+r"(byteOffset));
                register volatile u16 *dest __asm__("r0") =
                    (volatile u16 *)((u8 *)rec + byteOffset);
                *dest = (u16)mode;
            }
            /* Fall through to the shared mode-4 updates. */
mode4_done:
        // Symbol form keeps the base in a register and adds the offset, like
        // the ROM's two pool words (`ldr r0,=0x03001780; ldr r1,=0x10C3`).
        { extern u8 Rec35Wa[];
          __asm__(".globl Rec35Wa\nRec35Wa = 0x03001780\n");
          wa = Rec35Wa; }
        if (*(wa + 0x10C3) == 0) {
            WR16((u8 *)rec + 340, (u16)_08002178(1, 3));
        }
        WR16(r8, 5);                            // [+194]=5
        WR16((u8 *)rec + 198, zero);
        WR16((u8 *)rec + 344, (u16)mode);
        return;
        case 3:
            goto mode4_sel3;
        default:
            return;
        }
mode4_sel3:
        sub_080017414(rec);
        return;
    }
    default:
        return;
    }
}
// The ROM has two zero pad bytes after the return; use zero fill instead of
// GAS's Thumb alignment NOP at 0x0801774A.
__asm__(".align 2, 0");
void _0800175BC(void *a) __attribute__((alias("Rec35_Input_175BC")));
// ---------------------------------------------------------------------------
// _08001774C(rec) — gate leaf (u16[+184] compare after key read)
void Rec35_Leaf_1774C(void *rec) {
    register u8 *r5 __asm__("r5") = (u8 *)rec;
    u32 a = (u32)((int(*)(int, int))(void *)_08002178)(0, 4);
    u32 b = (u32)((int(*)(int, int))(void *)_08002178)(1, 4);
    u32 m;
    if (((a << 16) >> 16) != ((b << 16) >> 16)) return;
    m = *(u16 *)(r5 + 188);
    m = ((m - 1) << 16) >> 16;
    if (m > 1) return;
    Rec35_Leaf_17414(r5);
}
__asm__(".align 2, 0");
void _08001774C(void *a) __attribute__((alias("Rec35_Leaf_1774C")));

// ---------------------------------------------------------------------------
// _080017788(rec, a, id) — full-ctrl selection (see ROM register-plumbing note)
__attribute__((naked)) void Rec35_Input_17788(void *rec, int a, int id) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, r6, r7, lr}\n"
        "mov	r7, r9\n"
        "mov	r6, r8\n"
        "push	{r6, r7}\n"
        "adds	r7, r0, #0\n"
        "lsls	r1, r1, #16\n"
        "lsrs	r1, r1, #16\n"
        "mov	r9, r1\n"
        "lsls	r2, r2, #16\n"
        "lsrs	r2, r2, #16\n"
        "mov	r8, r2\n"
        "ldr r0, .Lr35i_pool_804\n"
        "ldr r1, .Lr35i_pool_808\n"
        "adds	r0, r0, r1\n"
        "ldrb	r0, [r0, #0]\n"
        "cmp	r0, #1\n"
        "bne .Lr35i_80c\n"
        "adds	r5, r7, #0\n"
        "adds	r5, #194\n"
        "ldrh	r1, [r5, #0]\n"
        "movs	r0, #4\n"
        "bl 0x08002158\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #190\n"
        "ldrh	r1, [r0, #0]\n"
        "movs	r0, #5\n"
        "bl 0x08002158\n"
        "adds	r4, r7, #0\n"
        "adds	r4, #184\n"
        "ldrh	r1, [r4, #0]\n"
        "movs	r0, #1\n"
        "bl 0x08002158\n"
        "movs	r2, #170\n"
        "lsls	r2, r2, #1\n"
        "adds	r0, r7, r2\n"
        "ldrh	r1, [r0, #0]\n"
        "movs	r0, #3\n"
        "bl 0x08002158\n"
        "movs	r3, #172\n"
        "lsls	r3, r3, #1\n"
        "adds	r0, r7, r3\n"
        "ldrh	r1, [r0, #0]\n"
        "movs	r0, #2\n"
        "bl 0x08002158\n"
        "movs	r1, #128\n"
        "lsls	r1, r1, #2\n"
        "adds	r0, r7, r1\n"
        "ldrh	r1, [r0, #0]\n"
        "movs	r0, #6\n"
        "bl 0x08002158\n"
        "adds	r6, r5, #0\n"
        "adds	r1, r4, #0\n"
        "subs	r5, #6\n"
        "subs	r4, #2\n"
        "b .Lr35i_85e\n"
        "movs	r0, r0\n"
        ".Lr35i_pool_804: .word 0x03001780\n"
        ".Lr35i_pool_808: .word 0x000010C3\n"
        ".Lr35i_80c:\n"
        "adds	r6, r7, #0\n"
        "adds	r6, #194\n"
        "ldrh	r1, [r6, #0]\n"
        "movs	r0, #4\n"
        "bl 0x08002158\n"
        "adds	r5, r7, #0\n"
        "adds	r5, #188\n"
        "ldrh	r1, [r5, #0]\n"
        "movs	r0, #5\n"
        "bl 0x08002158\n"
        "adds	r4, r7, #0\n"
        "adds	r4, #182\n"
        "ldrh	r1, [r4, #0]\n"
        "movs	r0, #1\n"
        "bl 0x08002158\n"
        "movs	r2, #170\n"
        "lsls	r2, r2, #1\n"
        "adds	r0, r7, r2\n"
        "ldrh	r1, [r0, #0]\n"
        "movs	r0, #3\n"
        "bl 0x08002158\n"
        "movs	r3, #172\n"
        "lsls	r3, r3, #1\n"
        "adds	r0, r7, r3\n"
        "ldrh	r1, [r0, #0]\n"
        "movs	r0, #2\n"
        "bl 0x08002158\n"
        "movs	r1, #128\n"
        "lsls	r1, r1, #2\n"
        "adds	r0, r7, r1\n"
        "ldrh	r1, [r0, #0]\n"
        "movs	r0, #6\n"
        "bl 0x08002158\n"
        "adds	r1, r7, #0\n"
        "adds	r1, #184\n"
        ".Lr35i_85e:\n"
        "mov	r2, r8\n"
        "cmp	r2, #1\n"
        "bne .Lr35i_86a\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #190\n"
        "strh	r2, [r0, #0]\n"
        ".Lr35i_86a:\n"
        "mov	r3, r8\n"
        "cmp	r3, #2\n"
        "bne .Lr35i_876\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #190\n"
        "strh	r3, [r0, #0]\n"
        ".Lr35i_876:\n"
        "mov	r0, r8\n"
        "cmp	r0, #64\n"
        "bne .Lr35i_882\n"
        "ldrh	r0, [r1, #0]\n"
        "subs	r0, #1\n"
        "strh	r0, [r1, #0]\n"
        ".Lr35i_882:\n"
        "mov	r2, r8\n"
        "cmp	r2, #128\n"
        "bne .Lr35i_88e\n"
        "ldrh	r0, [r1, #0]\n"
        "adds	r0, #1\n"
        "strh	r0, [r1, #0]\n"
        ".Lr35i_88e:\n"
        "adds	r2, r1, #0\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r2, r3]\n"
        "cmp	r0, #0\n"
        "bgt .Lr35i_89c\n"
        "movs	r0, #1\n"
        "strh	r0, [r2, #0]\n"
        ".Lr35i_89c:\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "cmp	r0, #3\n"
        "ble .Lr35i_8a8\n"
        "movs	r0, #3\n"
        "strh	r0, [r1, #0]\n"
        ".Lr35i_8a8:\n"
        "ldr r0, .Lr35i_pool_8c4\n"
        "ldr r3, .Lr35i_pool_8c8\n"
        "adds	r0, r0, r3\n"
        "ldrb	r0, [r0, #0]\n"
        "cmp	r0, #0\n"
        "bne .Lr35i_8cc\n"
        "movs	r0, #1\n"
        "movs	r1, #1\n"
        "bl 0x08002178\n"
        "strh	r0, [r4, #0]\n"
        "movs	r0, #1\n"
        "b .Lr35i_8d8\n"
        "movs	r0, r0\n"
        ".Lr35i_pool_8c4: .word 0x03001780\n"
        ".Lr35i_pool_8c8: .word 0x000010C3\n"
        ".Lr35i_8cc:\n"
        "movs	r0, #0\n"
        "movs	r1, #1\n"
        "bl 0x08002178\n"
        "strh	r0, [r4, #0]\n"
        "movs	r0, #0\n"
        ".Lr35i_8d8:\n"
        "movs	r1, #5\n"
        "bl 0x08002178\n"
        "strh	r0, [r5, #0]\n"
        "movs	r1, #0\n"
        "ldrsh	r0, [r4, r1]\n"
        "adds	r5, r7, #0\n"
        "adds	r5, #186\n"
        "cmp	r0, #0\n"
        "bne .Lr35i_8f0\n"
        "ldrh	r0, [r5, #0]\n"
        "strh	r0, [r4, #0]\n"
        ".Lr35i_8f0:\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r6, r2]\n"
        "cmp	r0, #2\n"
        "beq .Lr35i_8fc\n"
        "cmp	r0, #4\n"
        "bne .Lr35i_910\n"
        ".Lr35i_8fc:\n"
        "ldrh	r3, [r5, #0]\n"
        "ldrh	r0, [r4, #0]\n"
        "cmp	r3, r0\n"
        "beq .Lr35i_910\n"
        "adds	r0, r7, #0\n"
        "bl sub_080016944\n"
        "movs	r0, #2\n"
        "bl sub_0800168B8\n"
        ".Lr35i_910:\n"
        "ldrh	r0, [r4, #0]\n"
        "strh	r0, [r5, #0]\n"
        "movs	r1, #0\n"
        "ldrsh	r5, [r6, r1]\n"
        "cmp	r5, #2\n"
        "blt .Lr35i_93e\n"
        "cmp	r5, #4\n"
        "bgt .Lr35i_92c\n"
        "adds	r0, r7, #0\n"
        "mov	r1, r9\n"
        "mov	r2, r8\n"
        "bl sub_0800175BC\n"
        "b .Lr35i_93e\n"
        ".Lr35i_92c:\n"
        "cmp	r5, #8\n"
        "bgt .Lr35i_93e\n"
        "cmp	r5, #6\n"
        "blt .Lr35i_93e\n"
        "adds	r0, r7, #0\n"
        "mov	r1, r9\n"
        "mov	r2, r8\n"
        "bl sub_08001774C\n"
        ".Lr35i_93e:\n"
        "pop	{r3, r4}\n"
        "mov	r8, r3\n"
        "mov	r9, r4\n"
        "pop	{r4, r5, r6, r7}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        "movs	r0, r0\n"
        ".syntax divided\n"
    );
}
void _080017788(void *a, int b, int c) __attribute__((alias("Rec35_Input_17788")));

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
// (R35_CALLEE itself is defined near the top of this file.)

// ---------------------------------------------------------------------------
// _08001794C(rec) — two banner-sprite digits (0x08024BFC slot 0 → x=68,
// slot 1 → x=108), each drawn only when its record's u16[+0] is set.
void Rec35_Emit_1794C(void *rec) {
    // SLOT ARGS (measured): the ROM is `movs r0,#0 / bl / adds r4,r0 /
    // movs r0,#1 / bl` -- the 0x08024BFC veneer passes r0 through to the
    // 0x080240D0 getter. The TU extern binds the slot index under a local
    // name with the closure's `_08024BFC` spelling; twin-call sites pass
    // 0/1 in order.
    register void *r4 __asm__("r4");
    register void *r5 __asm__("r5");
    (void)rec;
    r4 = Rec35GhostSlot(0);
    r5 = Rec35GhostSlot(1);
    if (*(u16 *)r4 != 0) sub_08003BC0(152, 68, *(int *)((u8 *)r4 + 4));
    if (*(u16 *)r5 != 0) sub_08003BC0(152, 108, *(int *)((u8 *)r5 + 4));
}
void _08001794C(void *a) __attribute__((alias("Rec35_Emit_1794C")));

// ---------------------------------------------------------------------------
// _080017984(rec) — two unconditional emitters over rec+8.
// NOTE: no guards in the ROM; both run every call.
void Rec35_Emit_17984(void *rec) {
    void *r8 = (u8 *)rec + 8;
    R35_CALLEE(Sub_08007BFC, sub_08007BFC)(r8, (int)RD32((u8 *)rec + 456), (int)RD32((u8 *)rec + 460),
                 0, 48, 5, 1, 1, 0);
    R35_CALLEE(Sub_08007BFC, sub_08007BFC)(r8, (int)RD32((u8 *)rec + 468), (int)RD32((u8 *)rec + 472),
                 0, 88, 5, 1, 1, 0);
}
// Match the ROM's two zero pad bytes instead of GAS's Thumb `46 c0` fill.
__asm__(".align 2, 0");
void _080017984(void *a) __attribute__((alias("Rec35_Emit_17984")));

// ---------------------------------------------------------------------------
// _0800179F0(rec) — single _08007B18 emit when u16[+182]==3
void Rec35_Emit_179F0(void *rec) {
    extern void sub_08007B18(void *a, int b, int c, int d, int e, int f, int g, int h);
    if (*(u16 *)((u8 *)rec + 182) == 3) {
        sub_08007B18(rec, 4, 104, 128, 6, 1, 1, 0);
    }
}
__asm__(".align 2, 0");
void _0800179F0(void *a) __attribute__((alias("Rec35_Emit_179F0")));

// ---------------------------------------------------------------------------
// _080017A20(rec) — FOUR emitters in two guarded pairs.
// Guard on slot 0: (rec+16, [384],[388], d=9,  e=72)  + (rec+24, [408],[412], d=76, e=72)
// Guard on slot 1: (rec+16, [396],[400], d=9,  e=112) + (rec+24, [420],[424], d=76, e=112)
void Rec35_Emit_17A20(void *rec) {
    // The ROM issues BOTH ghost-slot fetches before the first guard test
    // (slot0 -> r4, slot1 -> r9); fetching inside each `if` makes the whole
    // prologue and both blocks diverge.
    void *s0 = Rec35GhostSlot(0);
    void *s1 = Rec35GhostSlot(1);
    if (RD16P(s0) != 0) {
        // Argument expressions in ROM order (address first, then the loads).
        u8 *d16 = (u8 *)rec + 16;
        u32 a384 = RD32((u8 *)rec + 384);
        u32 a388 = RD32((u8 *)rec + 388);
        R35_CALLEE(Sub_08007BFC, sub_08007BFC)(d16, (int)a384, (int)a388,
                     9, 72, 4, 1, 1, 0);
        u8 *d24 = (u8 *)rec + 24;
        u32 a408 = RD32((u8 *)rec + 408);
        u32 a412 = RD32((u8 *)rec + 412);
        R35_CALLEE(Sub_08007BFC, sub_08007BFC)(d24, (int)a408, (int)a412,
                     76, 72, 4, 1, 1, 0);
    }
    if (RD16P(s1) != 0) {
        u8 *d16 = (u8 *)rec + 16;
        u32 a396 = RD32((u8 *)rec + 396);
        u32 a400 = RD32((u8 *)rec + 400);
        R35_CALLEE(Sub_08007BFC, sub_08007BFC)(d16, (int)a396, (int)a400,
                     9, 112, 4, 1, 1, 0);
        u8 *d24 = (u8 *)rec + 24;
        u32 a420 = RD32((u8 *)rec + 420);
        u32 a424 = RD32((u8 *)rec + 424);
        R35_CALLEE(Sub_08007BFC, sub_08007BFC)(d24, (int)a420, (int)a424,
                     76, 112, 4, 1, 1, 0);
    }
}
void _080017A20(void *a) __attribute__((alias("Rec35_Emit_17A20")));

// ---------------------------------------------------------------------------
// _080017B00(rec) — two guarded emitters over rec+32 (e=60 / e=100)
void Rec35_Emit_17B00(void *rec) {
    // Both ghost-slot fetches are hoisted ahead of the guards, as in the ROM.
    // No local copy of `rec`: the parameter pseudo itself is homed to a
    // callee-saved register across the two fetches (one copy, not two).
    void *s0 = Rec35GhostSlot(0);
    void *s1 = Rec35GhostSlot(1);
    if (RD16P(s0) != 0) {
        // Argument expressions in ROM order: the rec+32 address first, then the
        // two loads; inlining them lets agbcc sink the address past the loads.
        u8 *dst = (u8 *)rec + 32;
        u32 a = RD32((u8 *)rec + 432);
        u32 b = RD32((u8 *)rec + 436);
        R35_CALLEE(Sub_08007BFC, sub_08007BFC)(dst, (int)a, (int)b,
                     16, 60, 7, 1, 1, 0);
    }
    if (RD16P(s1) != 0) {
        u8 *dst = (u8 *)rec + 32;
        u32 a = RD32((u8 *)rec + 444);
        u32 b = RD32((u8 *)rec + 448);
        R35_CALLEE(Sub_08007BFC, sub_08007BFC)(dst, (int)a, (int)b,
                     16, 100, 7, 1, 1, 0);
    }
}
// Match the ROM's two zero pad bytes instead of GAS's Thumb `46 c0` fill.
__asm__(".align 2, 0");
void _080017B00(void *a) __attribute__((alias("Rec35_Emit_17B00")));

// ---------------------------------------------------------------------------
// _080017B84(rec) — single _08007BFC emitter (d=0, e=32, f=3)
// The ROM's r3 slot is clobbered to 0 by the last stack store before the call.
void Rec35_Emit_17B84(void *rec) {
    register u8 *base __asm__("r2") = (u8 *)rec;
    register void *dst __asm__("r0") = base + 48;
    register u32 offset __asm__("r3") = 492;
    register u32 x __asm__("r1") = *(u32 *)(base + offset);
    offset += 4;
    u32 y = *(u32 *)(base + offset);
    sub_08007BFC(dst, (int)x, (int)y,
                 0, 32, 3, 1, 1, 0);
}
// ROM inventory span ends with the zero-filled alignment halfword at +0x32.
__asm__(".align 2, 0");
void _080017B84(void *a) __attribute__((alias("Rec35_Emit_17B84")));
void sub_080017B84(void *a) __attribute__((alias("Rec35_Emit_17B84")));

// ---------------------------------------------------------------------------
// _080017BB8(rec) — paint
void Rec35_Paint_17BB8(void *rec) {
    sub_0800D97C((u8 *)rec + 176, 15);
    R35_CALLEE(Rec35_Emit_1794C, _08001794C)(rec);
    R35_CALLEE(Rec35_Emit_17984, _080017984)(rec);
    R35_CALLEE(Rec35_Emit_179F0, _0800179F0)(rec);
    R35_CALLEE(Rec35_Emit_17A20, _080017A20)(rec);
    R35_CALLEE(Rec35_Emit_17B00, _080017B00)(rec);
    R35_CALLEE(Rec35_Emit_17B84, _080017B84)(rec);
    if (RS16((u8 *)rec + 192) == 1) {
        // ROM order is dst, [480], [484], 0x38 — not 0x38/[484] as before.
        u8 *dst = (u8 *)rec + 40;
        u32 a480 = RD32((u8 *)rec + 480);
        u32 a484 = RD32((u8 *)rec + 484);
        _08007C68(dst, (u32)(int)a480, (u32)(int)a484,
                     0x38, 64, 8, 1, 0, 0);
    }
    sub_0800DBE8((u8 *)rec + 56);
    Rec35_BxLr_17D50(rec);
}
void _080017BB8(void *a) __attribute__((alias("Rec35_Paint_17BB8")));

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
void _080017C34(void *a) __attribute__((alias("Rec35_Paint_17C34")));
// The closure spells this VMA `sub_080017C34` (asm/rec35_runtime.s:2905) and
// the entry exports it, so the owning TU must define that exact name. One hop
// to the real body -- an alias-of-an-alias is not in the slice link.
void sub_080017C34(void *a) __attribute__((alias("Rec35_Paint_17C34")));

// ---------------------------------------------------------------------------
// _080017C78(rec) — paint variant gated by u8[WA+0x10C3] and s16[+194]
void Rec35_Paint_17C78(void *rec) {
    void *r4 = rec;
    u8 *wa;
    sub_0800D97C((u8 *)r4 + 176, 15);
    // WA+0x10C3 as base+offset (the ROM loads 0x03001780 and 0x10C3 as two
    // pool words and adds them), assigned after the 0x0800D97C call.
    { extern u8 Rec35Wa[];
      __asm__(".globl Rec35Wa\nRec35Wa = 0x03001780\n");
      wa = Rec35Wa; }
    if (*(wa + 0x10C3) == 1 && RS16((u8 *)r4 + 194) != 0) {
        R35_CALLEE(Rec35_Emit_1794C, _08001794C)(r4);
        R35_CALLEE(Rec35_Emit_17984, _080017984)(r4);
        if ((u16)(RD16P((u8 *)r4 + 194) - 6) > 2) R35_CALLEE(Rec35_Emit_179F0, _0800179F0)(r4);
        R35_CALLEE(Rec35_Emit_17A20, _080017A20)(r4);
        R35_CALLEE(Rec35_Emit_17B00, _080017B00)(r4);
        R35_CALLEE(Rec35_Emit_17B84, _080017B84)(r4);
    }
    if (RS16((u8 *)r4 + 192) == 1) {
        // ROM order: dst, [480], [484], 20 — the two 0x14/[484] slots are the
        // OTHER way round, and the address is evaluated before the loads.
        u8 *dst = (u8 *)r4 + 40;
        u32 a480 = RD32((u8 *)r4 + 480);
        u32 a484 = RD32((u8 *)r4 + 484);
        _08007C68(dst, (u32)(int)a480, (u32)(int)a484,
                     0x14, 72, 8, 1, 0, 0);
    }
    // The ROM tests the high bound first (`cmp #4 / bgt`) and then the low one
    // as `cmp #3 / blt`; a `switch` over the two in-range values reproduces that
    // range check where `>= 3 && <= 4` canonicalises to `cmp #2 / ble`.
    switch ((int)RS16((u8 *)r4 + 198)) {
    case 3:
    case 4: {
        int v = R35_CALLEE(Sub_080022CC, _080022CC)() * 100;
        if (v < 0) v += 0xFFF;
        R35_CALLEE(Sub_08003978, sub_08003978)(128, 96, 100 - (v >> 12));
        break;
    }
    default:
        break;
    }
    sub_0800DBE8((u8 *)r4 + 56);
    Rec35_BxLr_17D50(r4);
}
void _080017C78(void *a) __attribute__((alias("Rec35_Paint_17C78")));

// ---------------------------------------------------------------------------
// _080017D54(rec) — countdown
void Rec35_Countdown_17D54(void *rec) {
    // The phase gate is WA+0x0FBC (pool word 0x00000FBC), and the ROM keeps
    // WA in r0 and adds the offset, so use the non-foldable symbol spelling.
    { extern u8 Rec35Wa[];
      u8 *wa;
      __asm__(".globl Rec35Wa\nRec35Wa = 0x03001780\n");
      wa = Rec35Wa;
      if (RD16P(wa + 0x0FBC) != 3) return; }
    if (RD32((u8 *)rec + 508) != 1) return;
    // ROM layout is `cmp/beq <zero-block>`: the countdown arm falls through and
    // the zero arm sits past the literal pool, so the condition is written
    // inverted with the zero-write in the else arm (a `return` form puts the
    // zero block inline and moves the pool).
    if (R35_CALLEE(Sub_08002140, _08002140)() != 2) {
        u32 v = RD32((u8 *)rec + 504) + 1;
        WR32((u8 *)rec + 504, v);
        if ((int)v > 180) R35_CALLEE(Sub_08004D4C, sub_08004D4C)(21, 0, 0);
    } else {
        WR32((u8 *)rec + 504, 0);
    }
}
void _080017D54(void *a) __attribute__((alias("Rec35_Countdown_17D54")));

// ---------------------------------------------------------------------------
// _080017DAC(ev, a, b, rec) — 12-way event dispatcher, table at 0x08017DCC:
//   [1] 0x08017DFC   [4] 0x08017E06   [6] 0x08017E2A   [5] 0x08017E5E
//   [0] 0x08017EB6   [11] 0x08017EBE  all other entries -> the shared epilogue.
// The range check comes out as `cmp #11 / bls <table> / b <exit>` only because
// the epilogue sits past Thumb's +-256-byte conditional-branch reach; the two
// mode dispatches are 3-arm switches (see Rec35_SyncRec_16944). The ROM's
// trailing two bytes are zero padding; the explicit fill after this body below
// matches them in place of GAS's Thumb NOP filler.
void Rec35_EvDispatch_17DAC(int ev, int a, int b, void *rec) {
    void *r5 = (void *)(u32)a;
    void *r6 = (void *)(u32)b;
    void *r4 = rec;
    // Case bodies are emitted in SOURCE order, and the ROM lays them out as
    // 0x08017DFC (case 1), 0x08017E06 (4), 0x08017E2A (6), 0x08017E5E (5),
    // 0x08017EB6 (0), 0x08017EBE (11) -- so the source has to list them in that
    // order, not numerically.
    switch (ev - 1) {
    case 1:                                     // 0x08017DFC → 0x080168E8
        Rec35_Ev3_168E8(r4, r5);
        break;
    case 4:                                     // 0x08017E06
        R35_CALLEE(Sub_0800D854, _0800D854)((u8 *)r4 + 56);
        R35_CALLEE(Sub_0800D8E4, _0800D8E4)((u8 *)r4 + 160);
        Rec35_Leaf_16DE4(r4);
        R35_CALLEE(Rec35_Dispatch_16F28, _080016F28)(r4);
        Rec35_Leaf_16DC0(r4);
        break;
    case 6: {                                   // 0x08017E2A
        // 3-arm switch, not an if/else-if chain: agbcc builds the ROM's
        // `cmp #1 / beq; cmp #1 / bgt; cmp #0 / beq` decision tree only for a
        // switch (see Rec35_SyncRec_16944).
        switch (RS16((u8 *)r4 + 180)) {
        case 0: Rec35_Paint_17BB8(r4); break;
        case 1: Rec35_Paint_17C34(r4); break;
        case 3: Rec35_Paint_17C78(r4); break;
        default: break;
        }
        break;
    }
    case 5: {                                   // 0x08017E5E → 0x08017D54 + rec+60 gate
        Rec35_Countdown_17D54(r4);
        if (RD16P((u8 *)r4 + 60) == 0) break;
        switch (RS16((u8 *)r4 + 180)) {
        case 0: R35_CALLEE(Rec35_Input_17440, _080017440)(r4, (int)(u16)(u32)r5, (int)(u16)(u32)r6); break;
        case 1: R35_CALLEE(Rec35_Input_17528, _080017528)(r4, (int)(u16)(u32)r5, (int)(u16)(u32)r6); break;
        case 3: R35_CALLEE(Rec35_Input_17788, _080017788)(r4, (int)(u16)(u32)r5, (int)(u16)(u32)r6); break;
        default: break;
        }
        break;
    }
    case 0:                                     // 0x08017EB6 → 0x08016FD0
        R35_CALLEE(Rec35_Scene_16FD0, _080016FD0)(r4);
        break;
    case 11:                                    // 0x08017EBE → 0x08016940
        Rec35_BxLr_16940(r4);
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
__asm__(".align 2, 0");
void _080017DAC(int ev, int a, int b, void *rec) __attribute__((alias("Rec35_EvDispatch_17DAC")));

// ---------------------------------------------------------------------------
// _080017ECC(a, rec) — ev1 flag setter
void Rec35_Ev1_17ECC(void *a, void *rec) {
    register u8 *dst __asm__("r4") = (u8 *)rec;
    const Rec35_CourseHdr *hdr = (const Rec35_CourseHdr *)sub_08004B68(a);
    register volatile u16 *p __asm__("r1");
    u16 out;
    switch (hdr->courseId) {
    case 21:
    case 27:
    case 28:
    case 29:
    case 30:
        p = (volatile u16 *)(dst + 84);
        out = 1;
        break;
    default:
        p = (volatile u16 *)(dst + 84);
        out = 6;
    }
    *p = out;
}
__asm__(".align 2, 0");
void _080017ECC(void *a, void *b) __attribute__((alias("Rec35_Ev1_17ECC")));

// ---------------------------------------------------------------------------
// _080017F04(rec) — scene constructor
__attribute__((naked)) void Rec35_Setup_17F04(void *rec) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, r6, r7, lr}\n"
        "mov	r7, sl\n"
        "mov	r6, r9\n"
        "mov	r5, r8\n"
        "push	{r5, r6, r7}\n"
        "sub	sp, #12\n"
        "adds	r7, r0, #0\n"
        "movs	r0, #65\n"
        "bl 0x0802B214\n"
        "ldr r4, .Lr35f_pool_044\n"
        "movs	r0, #4\n"
        "str	r0, [sp, #0]\n"
        "movs	r0, #1\n"
        "str	r0, [sp, #4]\n"
        "movs	r0, #0\n"
        "adds	r1, r4, #0\n"
        "movs	r2, #1\n"
        "movs	r3, #0\n"
        "bl sub_08007770\n"
        "movs	r0, #6\n"
        "str	r0, [r7, #80]\n"
        "movs	r0, #23\n"
        "str	r0, [r7, #92]\n"
        "movs	r0, #0\n"
        "str	r0, [r7, #104]\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #8\n"
        "bl sub_0800DAB8\n"
        "adds	r0, r4, #0\n"
        "adds	r1, r7, #0\n"
        "bl sub_0800798C\n"
        "adds	r0, r7, #0\n"
        "bl sub_08007A58\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #0\n"
        "movs	r2, #3\n"
        "bl sub_080075E8\n"
        "movs	r0, #3\n"
        "mov	sl, r0\n"
        "movs	r5, #0\n"
        "movs	r4, #0\n"
        "ldr r1, .Lr35f_pool_048\n"
        "mov	r8, r1\n"
        "ldr r2, .Lr35f_pool_04c\n"
        "add	r2, r8\n"
        "mov	r9, r2\n"
        ".Lr35f_f6c:\n"
        "ldr r3, .Lr35f_pool_050\n"
        "add	r3, r8\n"
        "movs	r6, #0\n"
        "ldrsh	r0, [r3, r6]\n"
        "mov	r2, r9\n"
        "movs	r6, #0\n"
        "ldrsh	r1, [r2, r6]\n"
        "adds	r2, r4, #0\n"
        "str	r3, [sp, #8]\n"
        "bl 0x08025CF4\n"
        "adds	r1, r0, #0\n"
        "subs	r0, r1, #1\n"
        "ldr	r3, [sp, #8]\n"
        "cmp	r0, #2\n"
        "bhi .Lr35f_f8e\n"
        "adds	r5, r1, #0\n"
        ".Lr35f_f8e:\n"
        "cmp	r5, sl\n"
        "bgt .Lr35f_f94\n"
        "mov	sl, r5\n"
        ".Lr35f_f94:\n"
        "adds	r4, #1\n"
        "cmp	r4, #2\n"
        "ble .Lr35f_f6c\n"
        "adds	r4, r7, #0\n"
        "adds	r4, #132\n"
        "movs	r0, #0\n"
        "mov	r8, r0\n"
        "mov	r1, sl\n"
        "strh	r1, [r4, #0]\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r3, r2]\n"
        "movs	r6, #4\n"
        "ldrsh	r1, [r3, r6]\n"
        "mov	r2, sl\n"
        "bl sub_08002714\n"
        "movs	r0, #64\n"
        "bl sub_08005758\n"
        "ldr r2, .Lr35f_pool_054\n"
        "strh	r0, [r2, #2]\n"
        "mov	r0, r8\n"
        "strh	r0, [r2, #0]\n"
        "ldr r5, .Lr35f_pool_058\n"
        "ldr r1, .Lr35f_pool_05c\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r4, r3]\n"
        "subs	r0, #1\n"
        "lsls	r0, r0, #1\n"
        "adds	r0, r0, r1\n"
        "movs	r6, #0\n"
        "ldrsh	r1, [r0, r6]\n"
        "movs	r0, #2\n"
        "ldrsh	r2, [r2, r0]\n"
        "movs	r0, #64\n"
        "str	r0, [sp, #0]\n"
        "adds	r0, r5, #0\n"
        "movs	r3, #64\n"
        "bl sub_08007570\n"
        "ldr r1, .Lr35f_pool_060\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r4, r2]\n"
        "subs	r0, #1\n"
        "lsls	r0, r0, #1\n"
        "adds	r0, r0, r1\n"
        "movs	r3, #0\n"
        "ldrsh	r1, [r0, r3]\n"
        "adds	r0, r5, #0\n"
        "movs	r2, #4\n"
        "bl sub_080075E8\n"
        "adds	r4, #4\n"
        "str	r4, [r7, #120]\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #8\n"
        "str	r0, [r7, #124]\n"
        "mov	r6, r8\n"
        "str	r6, [r7, #8]\n"
        "movs	r0, #1\n"
        "strh	r0, [r7, #12]\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #28\n"
        "movs	r2, #32\n"
        "negs	r2, r2\n"
        "movs	r1, #0\n"
        "bl sub_0800D77C\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #20\n"
        "movs	r1, #0\n"
        "movs	r2, #160\n"
        "bl sub_0800D77C\n"
        "movs	r0, #6\n"
        "strh	r0, [r7, #16]\n"
        "movs	r0, #5\n"
        "strh	r0, [r7, #18]\n"
        "movs	r0, #2\n"
        "str	r0, [r4, #0]\n"
        "add	sp, #12\n"
        "pop	{r3, r4, r5}\n"
        "mov	r8, r3\n"
        "mov	r9, r4\n"
        "mov	sl, r5\n"
        "pop	{r4, r5, r6, r7}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        ".Lr35f_pool_044: .word 0x0832C31C\n"
        ".Lr35f_pool_048: .word 0x03001780\n"
        ".Lr35f_pool_04c: .word 0x00000FF8\n"
        ".Lr35f_pool_050: .word 0x00000FF4\n"
        ".Lr35f_pool_054: .word 0x0203F8E8\n"
        ".Lr35f_pool_058: .word 0x08306B54\n"
        ".Lr35f_pool_05c: .word 0x080CB8E0\n"
        ".Lr35f_pool_060: .word 0x080CB8E6\n"
        ".syntax divided\n"
    );
}
void _080017F04(void *a) __attribute__((alias("Rec35_Setup_17F04")));

// ---------------------------------------------------------------------------
// _080018064(rec, a, b) — countdown gate
// ROM takes the counter in r2 (third arg b): (b<<16)-0x10000, lsr 16;
// if (u32)r2>1 → return; else 0x0802B368(1);
//   [rec+136]=1; [rec+12](u16)=0; [rec+116](u16)=1; [rec+36]=1.
void Rec35_CountdownGate_18064(void *rec, int a, int b) {
    register u8 *r4 __asm__("r4") = (u8 *)rec;
    int r2 = b;
    r2 = (r2 << 16) + -0x10000;
    r2 = (int)((u32)r2 >> 16);
    if ((u32)r2 > 1) return;
    (void)a;
    Sub_0802B368(1);
    *(u32 *)(r4 + 136) = 1;
    *(u16 *)(r4 + 12) = 0;
    *(u16 *)(r4 + 116) = 1;
    *(u32 *)(r4 + 36) = 1;
}
void _080018064(void *a, int b, int c) __attribute__((alias("Rec35_CountdownGate_18064")));

// ---------------------------------------------------------------------------
// _080018098(rec, dst, idx, arg) — row emit
// ROM byte-for-byte twin of Rec35_PlacementTick_16734 (rec35_runtime.c), which
// is EXACT 144/144: only the counter cell (0x03000590 vs 0x0300058C) and the
// row table (0x080CB8E0 vs 0x080CB8AC) differ. Every pinned register below is
// load-bearing — the `adds r2,r5,#0` clamp copy, the idx relocation out of r2
// (`adds r1,r2,#0`), the pool-word order and the r8-save prologue all follow
// from these pins. The pinned body only exists off-Apple (clang rejects the
// absolute aliases); the host build keeps the plain spelling.
void Rec35_RowEmit_18098(void *rec, void *dst, int idx, int arg) {
    register int index __asm__("r1") = idx;
    __asm__ volatile("" : "+r"(index));
    register volatile u32 *cnt __asm__("r2") = (volatile u32 *)0x03000590;
    __asm__ volatile("" : "+r"(cnt));
    int v = (int)(*cnt) - 1;
    *cnt = (u32)v;
    register s16 *cur __asm__("r5") = (s16 *)0x0203F8E8;
    __asm__ volatile("" : "+r"(cur));
    if (v <= 0) {
        *cur = (u16)(*cur + 1);
        *cnt = 5;
    }
    register s16 *cur0 __asm__("r2") = cur;
    __asm__ volatile("" : "+r"(cur0));
    if (*cur0 > 15) *cur0 = 0;
    register void *base __asm__("r0") = (void *)0x08306B54;
    __asm__ volatile("" : "+r"(base));
    register const s16 *table __asm__("r2") = (const s16 *)0x080CB8E0;
    __asm__ volatile("" : "+r"(table));
    int s16val = table[index];
    sub_08007570(base, s16val, (int)cur[1], (int)cur[0] << 6, 64);
    sub_08002ED0(rec, (int)(uintptr_t)dst, (int)cur[1], arg, 1, 3, 1, 0, 0, 1);
}
void _080018098(void *a, void *b, int c, int d) __attribute__((alias("Rec35_RowEmit_18098")));

// ---------------------------------------------------------------------------
// _080018128(rec) — init
void Rec35_Init_18128(void *rec) {
    extern void sub_08007B18(void *a, int b, int c, int d, int e, int f, int g, int h);
    u8 *r4 = (u8 *)rec;
    sub_0800D97C(r4 + 128, 15);
    sub_08007B18(r4, 2, 64, 64, 3, 1, 0, 0);
    R35_CALLEE(Rec35_RowEmit_18098, _080018098)(0, (void *)72, *(const s16 *)(r4 + 132) - 1, 4);
    r4 += 8;
    sub_0800DBE8(r4);
}
void _080018128(void *a) __attribute__((alias("Rec35_Init_18128")));
void sub_080018128(void *a) __attribute__((alias("Rec35_Init_18128")));

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
void _080018174(int ev, int a, int b, void *rec) __attribute__((alias("Rec35_EvDispatch_18174")));

// ROM entry alias.
void Rec35_Leaf_16944(void *rec) __attribute__((alias("Rec35_SyncRec_16944")));
void Rec35_Leaf_16A20(void *rec) __attribute__((alias("Rec35_PlacementEmit_16A20")));
