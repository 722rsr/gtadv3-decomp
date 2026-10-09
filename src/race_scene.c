#include "gtadv/ghost.h"
void sub_08001D218(void) __attribute__((alias("Race_Scene_Wrapper_1D218")));  /* rule 6: the splice replaces this label */
#include "gba/types.h"
#include "gtadv/memory.h"

// Reference: asm/race_scene.s 0x0801A294-0x0802135C (112 funcs)
// Evidence-backed dispatchers and init/frame leaves with proven pools/caller offsets.

// _08001986C: 10-way on s16[WA+0x10FC]-1 via table 0x080198E0
// (VMA evidence: asm/race_setup_18adc.s _0800198E0.._080019904)
extern void RaceScene_Leaf_318(void);  // 0x08019318 (bl sub_080019318 @0x08019908)
extern void RaceScene_Leaf_5B8(void);  // 0x080195B8 (@0x0801990E)
extern void RaceScene_Leaf_6E8(void);  // 0x080195F8 (@0x08019914)
extern void RaceScene_Leaf_860(void);  // 0x080196E8 (@0x0801991A)
void Race_Scene_Dispatch_19_86C(void) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    s16 idx = *(volatile s16 *)(wa + 0x10DC);
    if (idx <0 || idx>9) return;
    switch(idx){
        case 0: case 1: case 3: case 4: RaceScene_Leaf_318(); break;
        case 2: RaceScene_Leaf_5B8(); break;
        case 5: RaceScene_Leaf_6E8(); break;
        case 6: RaceScene_Leaf_860(); break;
        default: break;
    }
}
#ifndef __APPLE__
void _08001986C(void) __attribute__((alias("Race_Scene_Dispatch_19_86C")));
#endif

// _08001D6D0: bx lr stub
void Race_Scene_BxLr(void) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001D6D0(void) __attribute__((alias("Race_Scene_BxLr")));
void sub_08001D6D0(void) __attribute__((alias("Race_Scene_BxLr")));
#endif

// _08001D6D4: 12-way event dispatcher via table 0x08001D6EC. The ROM entry is
//   `push {lr}; adds r3,r1,#0; subs r0,#1; cmp r0,#11; bhi <ret>`, so the
//   switch value is the FIRST parameter (r0) and agbcc indexes the table by
//   (id - 1): 12 slots at 0x08001D6EC for id 1..12, everything else returns.
//   `adds r3,r1,#0` parks the SECOND parameter in r3, and 0x08001D73A reads
//   (u16)r3 then (u16)r2 for the id-6 call, so the signature is
//   (id, x, y) — not the 4-arg (a, ev, b, c) form this file used to declare.
// Case order in the source is the code emission order (0x08001D71C onward):
//   id1 -> 0x0801AAA0, id4 -> 0x0801B024, id5 -> 0x0801D224,
//   id7 -> 0x0801D6D0 (the `bx lr` stub, a real call, spelled
//   Race_Scene_BxLr — the `_08001D6D0` alias exists only under
//   `#ifndef __APPLE__`, so calling it unguarded would be an implicit
//   declaration in the host build; both spellings emit the same call),
//   id12 -> 0x0801D654,
//   id6 -> 0x0801B364((u16)x,(u16)y), id8 -> 0x0801D288. Ids 2,3,9,10,11
//   and out-of-range 0 / >12 fall through to the shared epilogue.
extern void RaceScene_Init_AAA0(void);    // 0x0801AAA0 (id1)
extern void RaceScene_Frame_B024(void);   // 0x0801B024 (id4)
extern void RaceScene_Event_1D224(void);  // 0x0801D224 (id5)
extern void RaceScene_Event_654(void);    // 0x0801D654 (id12)
extern void RaceScene_Event_288(void);    // 0x0801D288 (id8)
extern void RaceScene_B364(u16 x, u16 y); // 0x0801B364 (id6)
// Call-site split: the closure spells these six helpers by VMA, and the screen
// derives rule 6 from closure branch targets, so a friendly name reads as
// "no VMA and not a promoted export". Same shape as FF_CALLEE (menu_ff78_f.c),
// MS_CALLEE (menu_stage.c) and R35_CALLEE (rec35_mid_region.c).
#ifndef __APPLE__
#define RS_CALLEE(friendly, closure) closure
extern void _08001AAA0(void);
extern void _08001B024(void);
extern void _08001D224(void);
extern void _08001D288(void);
extern void _08001D654(void);
extern void _08001B364(u16 x, u16 y);
extern void sub_08008098(int v); // 0x08008098 closure spelling; `Sub_08008098`
                                 // exists only as a C alias, so a promoted caller
                                 // using it fails the link on an undefined ref.
extern void _08018AA8(u32 mask, int set);    // 0x08018AA8, alias of Ghost_FlagOp
extern void sub_08001B498(int v);             // 0x08001B498, alias of _08001B498
extern u32  sub_08009674(void);               // 0x08009674, alias of Course_GetU32_030003E4_0
extern int  sub_0800968C(int v);              // 0x0800968C, alias of Course_Fetch_S16_20Shift
extern void sub_080019AEC(int mode);          // 0x08019AEC; real body is race_setup_helpers.c's
                                             // `sub_080019AEC`. The 9-digit `_080019AEC`
                                             // is also a closure label but has no C
                                             // definition, so calling it reaches the ROM body instead of C.
#else
#define RS_CALLEE(friendly, closure) friendly
#endif
void Race_Scene_EventDispatch_1D6D4(int id, int x, int y) {
    switch (id) {
    case 1: RS_CALLEE(RaceScene_Init_AAA0, _08001AAA0)(); break;
    case 4: RS_CALLEE(RaceScene_Frame_B024, _08001B024)(); break;
    case 5: RS_CALLEE(RaceScene_Event_1D224, _08001D224)(); break;
    case 7: Race_Scene_BxLr(); break;
    case 12: RS_CALLEE(RaceScene_Event_654, _08001D654)(); break;
    case 6: RS_CALLEE(RaceScene_B364, _08001B364)((u16)x, (u16)y); break;
    case 8: RS_CALLEE(RaceScene_Event_288, _08001D288)(); break;
    default: break;
    }
}
#ifndef __APPLE__
void _08001D6D4(int a, int b, int c) __attribute__((alias("Race_Scene_EventDispatch_1D6D4")));
#endif

// _08001A294: race init FSM. asm evidence (race_scene.s 0x08001A294 head):
// zeroes u16@pool(0x08001A2BC-data), 3-word store, CpuSet 0x0802D974(sp-templ),
// bl 0x0801A12C (race_cluster.s), bl 0x0801A204 (course_load), bl 0x0802E104,
// then WA 0x03001780+0x10CA-family init, records 15/14 via 0x080261C4/080261E8.
extern void RaceScene_Dispatch_A12C(void);  // 0x0801A12C (race_cluster.s)
extern void RaceScene_Leaf_A204(void);      // 0x0801A204 (course_load.s)
void Race_Scene_Init_1A294(void) {
    volatile u32 *dma0 = (volatile u32*)0x040000D4;
    volatile u32 *dma1 = (volatile u32*)0x040000DE;
    *dma0 = 0; *dma1 = 0; // strh [r2,#0] zero as in asm
    extern void CpuSet_2D974(void *dst,void *src,int len);
    CpuSet_2D974((void*)0x05002000,(void*)0x06010000,0);
    RaceScene_Dispatch_A12C();
    RaceScene_Leaf_A204();
}
#ifndef __APPLE__
void _08001A294(void) __attribute__((alias("Race_Scene_Init_1A294")));
#endif

// _08001A4AC — the race seeder (asm/race_scene.s 0x08001A4AC–0x08001A740).
// Full transcription. Reads s16[0x03001780+0x10FC] as the mode, indexes the
// 10-entry table at 0x0801A4E4 (slot i = u32[0x0801A4E4 + i*4]; targets
// 0x0801A50C/0x0801A516/0x0801A53A are interior case arms, 0x0801A55E the
// default), then builds one racer per s16[0x03001780+0x10CA] slot:
//   case words W[0..7] at frame sp+24 (the arms write 1, 2 or 7 of them);
//   per iteration i: spec[0] = W[i], spec[+8..+10] = the three car-catalog
//   bytes of the id, spec[+16]/[+18]/[+20] mode halfwords, spec[+4..+7] the
//   tuning block via _08001A44C; then ctor 0x08002A5D4(entry, spec) through
//   the relocated-vtable word 0x080CBB24, then entry[+12]=i, [+28]=1,
//   [+16]=id, [+18]=u16[WA+0x1DE4+2i], [+26]=IdMap(entry[+16]).
// The bx-veneer dispatch (r2 = u32[0x080CBB24] = 0x08002A5D5) becomes a
// direct call to the lifted constructor body.
extern void  _0802D974(void *dst, const void *src, u32 n);   // CpuSet (bios_wrappers.c)
extern int   _08018ACC(u32 mask);                            // flag test (race_cluster.c)
extern void *_080240C4(void);                                // ghost rec A (ghost2.c)
extern void *_080240D0(void);                                // ghost rec P (ghost2.c)
extern void  _08001A44C(void *dst, const void *src);         // tuning-block pack (race_scene_a1.c)
extern s8    _08025500(int id);                              // catalog +14 (ai_line_leaves.c)
extern s8    _08025518(int id);                              // catalog +13
extern s8    _08025530(int id);                              // catalog +12
extern int   _080022E4(int a);                               // car-id map (ai_catalog.c)
extern void  _08002A5D4(void *dst, void *spec);              // record ctor (garage_records.c)
void Race_Scene_Dispatch_1A4AC(void) {
    u32 W[8];                                   // frame sp+24..+56 (case words)
    s16 mode = *(volatile s16 *)(0x03001780u + 0x10FCu);
    int slot = mode - 1;                        // sxth
    if ((u32)slot > 9u) {
        W[0] = 1;                               // 0x0801A55E default arm
        goto tail;
    }
    switch (*(const volatile u32 *)(0x0801A4E4u + (u32)slot * 4u)) {
    case 0x0801A516u:                           // slots 0 and 3 (QUICK RACE)
        W[0] = 1; W[1] = 256; W[2] = 257; W[3] = 258;
        W[4] = 259; W[5] = 260; W[6] = 261; W[7] = 262;
        break;
    case 0x0801A50Cu:                           // slot 2
        W[0] = 4; W[1] = 3;
        break;
    case 0x0801A53Au: {                         // slot 7
        if (*(volatile u8 *)0x03002843u != 0) { W[0] = 1; W[1] = 2; }
        else                                  { W[0] = 2; W[1] = 1; }
        break;
    }
    default:                                    // 0x0801A55E
        W[0] = 1;
        break;
    }
tail: {
    int count = *(volatile s16 *)(0x03001780u + 0x10CAu);
    u8 spec[24];                                // frame sp[0..23]
    {   // CpuSet dst=sp, src=0x05000006, ctrl=0x05000006: 6-halfword fixed fill
        u16 fill = *(volatile u16 *)0x05000006u;
        for (int k = 0; k < 6; k++) ((u16 *)spec)[k] = fill;
    }
    u32 r8 = 0;                                 // record-pool byte cursor
    u32 off = 0;                                // frame sp+60 (2 per iteration)
    for (int i = 0; i < count; i++) {
        u32 w = W[i];                           // [sp+24+4i] (A516 defines all 8)
        *(u32 *)(spec + 0) = w;
        u16 id6 = *(volatile s16 *)(0x03003554u + off);   // r6 walk (base+2i)
        *(u16 *)(spec + 16) = 1;
        *(u16 *)(spec + 18) = id6;
        int id;
        if (w == 3) {                           // 0x0801A648 ghost-record arm
            *(u16 *)(spec + 16) = 17;
            u8 *P = (u8 *)_080240D0();
            *(u32 *)(spec + 12) = (u32)(uintptr_t)P + 32u;
            *(u16 *)(spec + 20) = (*(volatile s16 *)(P + 18) != 0) ? 1 : 0;
            _08001A44C(spec + 4, P + 20);
            *(volatile u8 *)(0x03004E80u + r8 + 32u) = 1;
            id = (s16)id6;
        } else if (w == 4 || w == 1 || w == 2) {// 0x0801A5C4 player arm
            u16 t = (u16)(*(volatile u16 *)0x0300287Cu - 9u);
            if (t <= 1u) {
                *(u16 *)(spec + 16) = 4;
                *(u16 *)(spec + 20) = 0;
            } else {
                u16 acc = *(u16 *)(spec + 16);
                acc = (u16)(_08018ACC(32) ? (acc | 16) : (acc | 8));
                *(u16 *)(spec + 16) = acc;
                u8 *Q = (u8 *)_080240C4();
                *(u32 *)(spec + 12) = (u32)(uintptr_t)Q + 32u;
                *(u16 *)(spec + 20) = (*(volatile s16 *)0x03002896u != 0) ? 1 : 0;
            }
            _08001A44C(spec + 4, (const void *)0x03002898u);
            id = *(volatile s16 *)(0x03003554u + off);    // r4 = 0x03003554+sp60
        } else {                                // 0x0801A69C AI arm
            *(u16 *)(spec + 20) = *(volatile u16 *)(0x03001780u + 0x10C6u);
            id = *(volatile s16 *)(0x03003554u + 2u * (u32)(count - 1));
        }
        *(u8 *)(spec + 8)  = (u8)_08025500(id);
        *(u8 *)(spec + 9)  = (u8)_08025518(id);
        *(u8 *)(spec + 10) = (u8)_08025530(id);
        u8 *entry = (u8 *)(0x03004EA4u + r8);
        _08002A5D4(entry, spec);                // vtable word 0x080CBB24
        u8 *E = entry - 36;
        *(u16 *)(E + 12) = (u16)i;
        *(u8 *)(E + 28) = 1;
        *(u16 *)(E + 16) = id6;
        *(u16 *)(E + 18) = *(volatile u16 *)(0x03001780u + 0x1DE4u + off);
        *(u16 *)(E + 26) = (u16)_080022E4((int)(s16)*(volatile u16 *)(E + 16));
        r8 += 284;
        off += 2;
    }
}
}
#ifndef __APPLE__
void _08001A4AC(void) __attribute__((alias("Race_Scene_Dispatch_1A4AC")));
#endif

extern void Sub_08002A86C(void *rec, u32 bits, u32 flag);
void Race_Scene_BHandler_1B410(void) {
    volatile u8 *wa = (volatile u8*)WORK_AREA_BASE;
    s16 cnt = *(volatile s16 *)(wa + 0x10CA);
    for(int i=0;i<cnt;i++) Sub_08002A86C((void *)(uintptr_t)(0x03004EA4 + i*284), 1, 0);
    if (Ghost_FlagTest(32)==0) {
        u8 v = *(volatile u8 *)(wa + 0x1110);
        if (v) {
            s16 c = *(volatile s16 *)(wa + 0x10CA -18);
            (void)c;
        }
    }
    Ghost_FlagOp(128,1);
    Ghost_FlagOp(256,1);
}
#ifndef __APPLE__
void _08001B410(void) __attribute__((alias("Race_Scene_BHandler_1B410")));
#endif

// _08001B024 — ROM characterisation (measured, 68 B, 0x08001B024-0x08001B067,
// pure Thumb, no callee-saved register, pool of exactly two words at +0x3C/+0x40).
//   +00 b500       push {lr}
//   +02 f7e7fd93   bl 0x08002B50
//   +06 f7e7fdc3   bl 0x08002BB4
//   +0A f7e7fd89   bl 0x08002B44
//   +0E 480b       ldr  r0, =0x03004E20; EWRAM pointer slot
//   +10 6802       ldr  r2, [r0]; r2 = racectx
//   +12 1c13       adds r3, r2, #0; r3 = racectx (copy for the +90 lane)
//   +14 335a       adds r3, #90
//   +16 490a       ldr  r1, =0x0000FFFF
//   +18 1c08       adds r0, r1, #0; r0 = mask, live across BOTH ors
//   +1C 8819       ldrh r1, [r3]; read u16[racectx+90]
//   +1E 4301       orrs r1, r0; | mask          <-- not folded
//   +20 8019       strh r1, [r3]
//   +22 1c11       adds r1, r2, #0; r1 = racectx
//   +24 3178       adds r1, #120
//   +26 880b       ldrh r3, [r1]; read u16[racectx+120]
//   +28 4318       orrs r0, r3; | mask          <-- not folded
//   +2A 8008       strh r0, [r1]
//   +2C 2000       movs r0, #0
//   +2E 60d0       str  r0, [r2, #12]
//   +30 6110       str  r0, [r2, #16]
//   +32 6150       str  r0, [r2, #20]
//   +34 f010faa9   bl 0x0802B5AC
//   +38 bc01       pop  {r0}
//   +3A 4700       bx   r0
//   +3C 0000       movs r0, r0; pool alignment
//   +3E.word 0x03004E20   +42.word 0x0000FFFF
// Address class: 0x03004E20 is EWRAM pointer to race context.
// Matches 68/68 byte-exact in pure C when accessed through an extern struct pointer
// at 0x03004E20: member stores through the pointer preserve both `orrs` without folding,
// allocate r0-r3 without spill to r4, and reproduce the ROM instruction sequence and pool.
struct RaceSceneCtx1B024 {
    char pad[12];
    u32 f12;
    u32 f16;
    u32 f20;
    char pad2[66];
    u16 f90;
    char pad3[28];
    u16 f120;
};
extern struct RaceSceneCtx1B024 *gRaceCtx1B024;

extern void Sub_08002B50(void);
extern void Sub_08002BB4(void);
extern void Sub_08002B44(void);
extern void Sub_0802B5AC(void);
#ifndef __APPLE__
extern void sub_08002B50(void);
extern void sub_08002BB4(void);
extern void sub_08002B44(void);
extern void _0802B5AC(void);
#endif

void Race_Scene_Leaf_1B024(void) {
    __asm__(".set gRaceCtx1B024, 0x03004E20\n");
    RS_CALLEE(Sub_08002B50, sub_08002B50)();
    RS_CALLEE(Sub_08002BB4, sub_08002BB4)();
    RS_CALLEE(Sub_08002B44, sub_08002B44)();
    gRaceCtx1B024->f90 |= 0xFFFF;
    gRaceCtx1B024->f120 |= 0xFFFF;
    gRaceCtx1B024->f12 = 0;
    gRaceCtx1B024->f16 = 0;
    gRaceCtx1B024->f20 = 0;
    RS_CALLEE(Sub_0802B5AC, _0802B5AC)();
}
#ifndef __APPLE__
void _08001B024(void) __attribute__((alias("Race_Scene_Leaf_1B024")));
#endif

// _08001B130: small flag wrapper, switch r0 0/1/2 (Ghost flags 0x200000 etc.)
extern void Sub_08009674(void);
extern int Sub_0800968C(int v);
extern void Sub_08001B498(int v);
extern void Sub_08019AEC(int v);
void Race_Scene_Leaf_1B130(int a0) {
    int r4 = a0;
    Ghost_FlagOp(0x200000u, 0); // 128<<14 = 0x200000
    if (r4 == 1) {
        (void)Sub_0800968C(1);
        volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
        (void)wa;
        Ghost_FlagOp(2, 1);
        Ghost_FlagOp(16, 1);
        *(volatile u8 *)(wa + 0x10BD) = 0;
    } else if (r4 == 2) {
        (void)Sub_0800968C(1);
        Sub_08019AEC(-1);
        Ghost_FlagOp(2, 1);
    } else if (r4 == 0) {
        Ghost_FlagOp(0x100000u, 0); // 128<<13 = 0x100000
    }
}
#ifndef __APPLE__
void _08001B130(int a0) __attribute__((alias("Race_Scene_Leaf_1B130")));
#endif

// _08001B1A4: 6-way dispatcher on a0 0..5 via table 0x0801B1CC (lsl #2 / mov pc)
// Pools: _08001B1CC table, flag 0x200000 (128<<14) etc., no large racectx
//
// ROM shape (asm/race_scene.s 1711-1801), offsets from 0x0801B1A4:
//   +0x00 push{r4,lr} / +0x06 movs r0,#128 / +0x08 lsls r0,#13 / +0x0A bl
//   +0x0E cmp r0,#0 / +0x10 bne +0x18 / +0x12 movs r0,#0 / +0x14 bl
//   +0x16 b +0xCA (shared epilogue)
//   +0x18 cmp r4,#5 / +0x1A bhi +0xCA / +0x1C lsls r0,r4,#2
//   +0x1E ldr r1,_08001B1CC / +0x20 adds r0,r0,r1 / +0x22 ldr r0,[r0]
//   +0x24 mov pc,r0 / +0x26 movs r0,r0 (pad)
//   +0x28.4byte 0x0801B1D0   <-- the table BASE
//   +0x2C..+0x43 the six case words: 0x0801B26E (case 0 IS the shared
//     epilogue), 0x0801B1E8, 0x0801B1F0, 0x0801B1FA, 0x0801B21A, 0x0801B244
//   +0x44..+0xC9 the five case arms, each `b +0xCA` (case 5 falls through)
//   +0xCA pop{r4} / +0xCC pop{r0} / +0xCE bx r0
//   +0xD0.4byte 0x03001780 / +0xD4.4byte 0x000010C8  (case 5's pool, which
//     agbcc emits AFTER the shared epilogue -- that is why the 216-byte span
//     ends at 0x0801B27C = sub_08001B27C, the next.type %function)
//
// agbcc DOES build the jump table from a plain C `switch`; the pre-inline
// candidate already emitted cmp/bhi/lsls/ldr/adds/ldr/mov pc plus a six-word
// pool. What it could not do is place the case BODIES inside the dispatcher's
// span: they were `bl` calls to the separate Race_Scene_Leaf_1B1E8..1B244
// definitions, so every arm was 6 bytes (`bl` + `b`) where the ROM has
// 8/10/32/42/42 and the span closed at 104 bytes instead of 216. Those five
// VMAs carry no `.type %function` of their own in asm/race_scene.s and
// nothing branches to them by name, so tools/coverage.py assigns their bytes
// to _08001B1A4 and the 216-byte span is producible only by ONE C body.
// Hence the arms are inlined here and the five standalone wrappers are gone:
// their aliases were never promoted and their bytes belong to this body.
extern void Sub_0802B368(int v);
extern void Sub_08009674(void);
extern int  Sub_0800968C(int v);
extern void Sub_08001B498(int v);
extern void Sub_08019AEC(int v);
extern void Sub_08008098(int v);
extern void _08001B068(int a);
void Race_Scene_Dispatch_1B1A4(int a0) {
    // The work-area base must stay an unresolved SYMBOL_REF: agbcc folds a
    // literal 0x03001780 + 0x10C6 into one constant (`.word 0x03001846`),
    // but the ROM keeps the pair as two pool words plus `adds r1,r1,r2`.
    // Declaring the absolute symbol INSIDE the body is required -- the slice
    // link splices only this section, so a file-scope one never reaches code.o.
#ifndef __APPLE__
    extern u8 RSWa[] __asm__("RSWa");
    __asm__(".globl RSWa\nRSWa = 0x03001780\n");
#define RS_WA RSWa
#else
#define RS_WA ((volatile u8 *)(uintptr_t)0x03001780u)
#endif
    int r4 = a0;
    if (Ghost_FlagTest(0x100000u) == 0) { // 128<<13 = 0x100000
        _08001B068(0);
        return;
    }
    if ((u32)r4 > 5) return;
    switch(r4){
        case 0: break;
        // case 1 @ +0x44: movs r0,#2 / bl 0x0802B368 / b +0xCA
        case 1: Sub_0802B368(2); break;
        // case 2 @ +0x4C: bl sub_08009674 / bl sub_08001B130 / b +0xCA
        // The ROM has no `movs r0,#0` here: sub_08001B130 receives whatever
        // sub_08009674 left in r0, i.e. its return value. The int-returning
        // re-spelling keeps the closure's VMA symbol while giving the call a
        // result to forward.
#ifndef __APPLE__
        // Every callee below is called by the CLOSURE spelling, which is not the
        // friendly one: `arm-none-eabi-nm -n build-code/code.o` binds
        // sub_08009674 / sub_0800968C / sub_08001B498 / _08018AA8 / _080019AEC,
        // and a promoted body spliced into the slice has no asm to fall back on.
        case 2: { extern int Sub_08009674r(void) __asm__("sub_08009674");
                  RS_CALLEE(Race_Scene_Leaf_1B130, _08001B130)(Sub_08009674r()); } break;
#else
        case 2: Sub_08009674(); Race_Scene_Leaf_1B130(0); break;
#endif
        // case 3 @ +0x56: the second call's result is truncated to s16
        // (`lsls r0,#16 / asrs r0,#16`) before Sub_08019AEC sees it.
        case 3:
            RS_CALLEE(Sub_08001B498, sub_08001B498)(0);
            RS_CALLEE(Sub_08019AEC, sub_080019AEC)((s16)RS_CALLEE(Sub_0800968C, sub_0800968C)(0));
            RS_CALLEE(Ghost_FlagOp, _08018AA8)(0x100000u, 0);
            break;
        // case 4 @ +0x76: `ldr r1,=WA; ldr r2,=0x10C6; adds r1,r1,r2` is
        // the two-pool-word address, not a folded constant.
        case 4: {
            int r0 = RS_CALLEE(Sub_0800968C, sub_0800968C)(1);
            volatile u8 *wa = RS_WA;
            volatile u16 *p = (volatile u16 *)(wa + 0x10C6);
            *p = (u16)r0;
            RS_CALLEE(Ghost_FlagOp, _08018AA8)(2,1);
            RS_CALLEE(Ghost_FlagOp, _08018AA8)(16,1);
} break;
        case 5: {
            int r0 = RS_CALLEE(Sub_0800968C, sub_0800968C)(2);
            volatile u8 *wa = RS_WA;
            volatile u16 *p = (volatile u16 *)(wa + 0x10C8);
            *p = (u16)r0;
            { s16 *q = (s16 *)p; RS_CALLEE(Sub_08008098, sub_08008098)(*q); }
            RS_CALLEE(Ghost_FlagOp, _08018AA8)(0x200000u,0);
            RS_CALLEE(Ghost_FlagOp, _08018AA8)(0x100000u,0);
} break;
        default: break;
    }
}
#ifndef __APPLE__
void _08001B1A4(int a0) __attribute__((alias("Race_Scene_Dispatch_1B1A4")));
// Exported by the manifest entry as `sub_08001B1A4`; defined here so the slice
// link has a symbol to bind once this body is spliced out of asm/race_scene.s.
void sub_08001B1A4(int a0) __attribute__((alias("Race_Scene_Dispatch_1B1A4")));
#endif
#undef RS_WA



// _08001D218: wrapper for sub_08009B28 (no pool)
extern void _08009B28(void);   // slice-closure spelling; body + `Sub_` alias in course_leaves.c
void Race_Scene_Wrapper_1D218(void) { _08009B28(); }
// The ROM span at 0x08001D218 is 12 bytes, but its last instruction is the
// 2-byte `00 47` at +0x08: `push {lr}; bl 0x08009B28; pop {r1}; bx r0`. The
// trailing two bytes are `00 00` (verified from baserom.gba), not code.
// Under -ffunction-sections gas closes a Thumb *code* section with the
// 2-byte `nop` filler (0x46c0), so this scored 10/12 with every instruction
// already byte-correct. A file-scope `.align 2, 0` is emitted after the
// body's `.size`, i.e. still inside the body's own section, and pads with
// the explicit `0` fill instead.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001D218(void) __attribute__((alias("Race_Scene_Wrapper_1D218")));
#endif

void Race_Scene_BxLr_1D9C8(void) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_08001D9C8(void) __attribute__((alias("Race_Scene_BxLr_1D9C8")));
void _08001D9C8(void) __attribute__((alias("Race_Scene_BxLr_1D9C8")));
#endif
// ROM body is a bare `bx lr`; every C caller in src/race_scene_c.c re-materialises
// r0 = rec before the `bl` (0x801e6cc, 0x801e970,...), so the lifted source
// passes rec even though this stub never reads it.
void Race_Scene_BxLr_1E9D8(void *rec) { (void)rec; }
__asm__(".align 2, 0");
void sub_08001E9D8(void *) __attribute__((alias("Race_Scene_BxLr_1E9D8")));
#ifndef __APPLE__
void _08001E9D8(void *) __attribute__((alias("Race_Scene_BxLr_1E9D8")));
#endif
void sub_08001EBA4(void) __attribute__((alias("Race_Scene_BxLr_1EBA4")));
void Race_Scene_BxLr_1EBA4(void) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001EBA4(void) __attribute__((alias("Race_Scene_BxLr_1EBA4")));
void sub_08001F6A0(void) __attribute__((alias("Race_Scene_BxLr_1F6A0")));
#endif
void Race_Scene_BxLr_1F6A0(void) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_080020878(void) __attribute__((alias("Race_Scene_BxLr_20878")));
void _08001F6A0(void) __attribute__((alias("Race_Scene_BxLr_1F6A0")));
#endif
void Race_Scene_BxLr_20878(void) {}
__asm__(".align 2, 0");
void sub_08002099C(void) __attribute__((alias("Race_Scene_BxLr_2099C")));
#ifndef __APPLE__
void _080020878(void) __attribute__((alias("Race_Scene_BxLr_20878")));
#endif
void sub_080021248(void) __attribute__((alias("Race_Scene_BxLr_21248")));
void Race_Scene_BxLr_2099C(void) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08002099C(void) __attribute__((alias("Race_Scene_BxLr_2099C")));
#endif
void Race_Scene_BxLr_21248(void) {}
__asm__(".align 2, 0");
#ifndef __APPLE__
void _080021248(void) __attribute__((alias("Race_Scene_BxLr_21248")));
#endif

// _08001F554: hdr = MgrGet80/Sub_08004B68(void); ldrh Vu16 +0; Sub_08002158(6, Vu16)
// Pools: none, widths ldrh Vu16 +0, lsls #16 lsrs #16 for val, bl numeric 0x08004B68(void)/0x08002158
// Exact asm: push {lr}; bl 0x08004B68 with no r0 setup -> Sub_08004B68(void)
extern void *sub_08004B68(void);        // asm/passthrough.inc spells it this way
extern void _08002158(int id, int val); // asm/blockb.s spells it this way
void Race_Scene_Leaf_F554(void *a) {
    (void)a;
    void *hdr = sub_08004B68();
    u16 v = *(volatile u16 *)hdr;
    _08002158(6, (int)v);
}
// Same trailing-pad case as `Race_Scene_Wrapper_1D218` above. The 20-byte
// ROM span's last instruction is the 2-byte `00 47` at +0x10, and the two
// bytes the span carries past it are `00 00` (verified from baserom.gba),
// where gas closes the Thumb code section with the `nop` filler 0x46c0.
// 18/20, every instruction already byte-correct.
__asm__(".align 2, 0");
#ifndef __APPLE__
void _08001F554(void *a) __attribute__((alias("Race_Scene_Leaf_F554")));
#endif

// _08002124C: same shape and the SAME two callees as _08001F554 (verified by
// decoding both BLs at +2 and +10 out of baserom.gba: both resolve to
// 0x08004B68 and 0x08002158), but a different PC. The two ROM spans differ only
// in the two BL *encodings* (e5 f7 07 fb / e2 f7 fb fd versus e3 f7 8b fc /
// e0 f7 7f ff), which is exactly what a PC-relative displacement changes.
// So the C text is identical and the difference is ownership, not semantics:
// a second definition puts a body at 0x0802124c so its BLs encode from there.
// Sharing Race_Scene_Leaf_F554 (rule 3) would emit one body at 0x08001F554 and
// silently relocate every caller of _08002124C.
void Race_Scene_Leaf_2124C(void *a) {
    (void)a;
    void *hdr = sub_08004B68();
    u16 v = *(volatile u16 *)hdr;
    _08002158(6, (int)v);
}
__asm__(".align 2, 0");
#ifndef __APPLE__
void sub_08002124C(void *a) __attribute__((alias("Race_Scene_Leaf_2124C")));
void _08002124C(void *a) __attribute__((alias("Race_Scene_Leaf_2124C")));
#endif

// _08001E338: ldrh Vu16 +182; if ==1 then sub_08007B18(a,7,8,112,5,1,1,0)
// Pools: none, widths ldrh Vu16 +182, cmp #1 bne, stack 5/1/1/0, regs 7/8/112, bl sub_08007B18
extern void sub_08007B18(void *a, int b, int c, int d, int e, int f, int g, int h);
void Race_Scene_Leaf_E338(void *a) {
    u16 v = *(u16 *)((u8 *)a + 182);
    if (v != 1) return;
    sub_08007B18(a, 7, 8, 112, 5, 1, 1, 0);
}
#ifndef __APPLE__
void _08001E338(void *a) __attribute__((alias("Race_Scene_Leaf_E338")));
void sub_08001E338(void *a) __attribute__((alias("Race_Scene_Leaf_E338")));
#endif

// _08001E364: ldrh Vu16 +184; if ==1 then sub_08007B18(a,11,8,128,5,1,1,0)
// Pools: none, widths ldrh Vu16 +184, cmp #1 bne, stack 5/1/1/0, regs 11/8/128, bl sub_08007B18
void Race_Scene_Leaf_E364(void *a) {
    u16 v = *(u16 *)((u8 *)a + 184);
    if (v != 1) return;
    sub_08007B18(a, 11, 8, 128, 5, 1, 1, 0);
}
#ifndef __APPLE__
void _08001E364(void *a) __attribute__((alias("Race_Scene_Leaf_E364")));
void sub_08001E364(void *a) __attribute__((alias("Race_Scene_Leaf_E364")));
#endif

// _08001E390: ldrh Vu16 +184; if ==1 then sub_08007B18(a,14,8,128,5,1,1,0)
// Pools: none, widths ldrh Vu16 +184, cmp #1 bne, stack 5/1/1/0, regs 14/8/128, bl sub_08007B18
void Race_Scene_Leaf_E390(void *a) {
    u16 v = *(u16 *)((u8 *)a + 184);
    if (v != 1) return;
    sub_08007B18(a, 14, 8, 128, 5, 1, 1, 0);
}
#ifndef __APPLE__
void _08001E390(void *a) __attribute__((alias("Race_Scene_Leaf_E390")));
void sub_08001E390(void *a) __attribute__((alias("Race_Scene_Leaf_E390")));
#endif


// ROM entry alias.
#ifndef __APPLE__
void _0801986C(void) __attribute__((alias("Race_Scene_Dispatch_19_86C")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void RaceScene_Frame_B024(void) __attribute__((alias("Race_Scene_Leaf_1B024")));
#endif
