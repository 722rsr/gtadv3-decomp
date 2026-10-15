#include "gtadv/ghost.h"
void sub_08001D218(void) __attribute__((alias("Race_Scene_Wrapper_1D218")));  /* rule 6: the splice replaces this label */
#include "gba/types.h"
#include "gtadv/memory.h"
#include "gtadv/course_cal.h"
#include "gtadv/sound.h"

// Reference: asm/race_scene.s 0x0801A294-0x0802135C (112 funcs)
// Evidence-backed dispatchers and init/frame leaves with proven pools/caller offsets.

// _08001986C: 10-way on s16[WA+0x10FC]-1 via table 0x080198E0
// (VMA evidence: asm/race_setup_18adc.s _0800198E0.._080019904)
extern void RaceScene_Leaf_318(void);  // 0x08019318 (bl sub_080019318 @0x08019908)
extern void RaceScene_Leaf_5B8(void);  // 0x080195B8 (@0x0801990E)
extern void RaceScene_Leaf_6E8(void);  // 0x080195F8 (@0x08019914)
extern void RaceScene_Leaf_860(void);  // 0x080196E8 (@0x0801991A)
__attribute__((naked)) void Race_Scene_Dispatch_19_86C(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, lr}\n"
        "ldr r2, .L1986C_wa\n"
        "ldr r1, .L1986C_flag_off\n"
        "adds r0, r2, r1\n"
        "ldr r0, [r0, #0]\n"
        "movs r1, #1\n"
        "ands r0, r1\n"
        "cmp r0, #0\n"
        "beq .L1986C_dispatch\n"
        "movs r5, #0\n"
        "ldr r0, .L1986C_count_off\n"
        "adds r1, r2, r0\n"
        "movs r2, #0\n"
        "ldrsh r0, [r1, r2]\n"
        "cmp r5, r0\n"
        "bge .L1986C_dispatch\n"
        "adds r6, r1, #0\n"
        "ldr r4, .L1986C_records\n"
        ".L1986C_loop:\n"
        "ldr r0, [r4, #0]\n"
        "ldr r1, [r4, #4]\n"
        "adds r2, r5, #0\n"
        "bl 0x080270D8\n"
        "movs r0, #142\n"
        "lsls r0, r0, #1\n"
        "adds r4, r4, r0\n"
        "adds r5, #1\n"
        "movs r1, #0\n"
        "ldrsh r0, [r6, r1]\n"
        "cmp r5, r0\n"
        "blt .L1986C_loop\n"
        ".L1986C_dispatch:\n"
        "ldr r0, .L1986C_wa\n"
        "ldr r2, .L1986C_mode_off\n"
        "adds r0, r0, r2\n"
        "ldrh r0, [r0, #0]\n"
        "subs r0, #1\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #16\n"
        "cmp r0, #9\n"
        "bhi .L1986C_return\n"
        "lsls r0, r0, #2\n"
        "ldr r1, .L1986C_jump_base\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "mov pc, r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L1986C_wa: .word 0x03001780\n"
        ".L1986C_flag_off: .word 0x000010DC\n"
        ".L1986C_count_off: .word 0x000010CA\n"
        ".L1986C_records: .word 0x03004E80\n"
        ".L1986C_mode_off: .word 0x000010FC\n"
        ".L1986C_jump_base: .word 0x080198E0\n"
        ".word 0x08019908\n"
        ".word 0x0801990E\n"
        ".word 0x08019908\n"
        ".word 0x08019908\n"
        ".word 0x08019914\n"
        ".word 0x08019924\n"
        ".word 0x08019924\n"
        ".word 0x0801991A\n"
        ".word 0x08019924\n"
        ".word 0x08019920\n"
        ".L1986C_case0:\n"
        "bl 0x08019318\n"
        "b .L1986C_return\n"
        ".L1986C_case1:\n"
        "bl 0x080195B8\n"
        "b .L1986C_return\n"
        ".L1986C_case2:\n"
        "bl 0x080195F8\n"
        "b .L1986C_return\n"
        ".L1986C_case3:\n"
        "bl 0x080196E8\n"
        "b .L1986C_return\n"
        ".L1986C_case4:\n"
        "bl 0x08019860\n"
        ".L1986C_return:\n"
        "pop {r4, r5, r6}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
}
void _08001986C(void) __attribute__((alias("Race_Scene_Dispatch_19_86C")));

// _08001D6D0: bx lr stub
void Race_Scene_BxLr(void) {}
__asm__(".align 2, 0");
void _08001D6D0(void) __attribute__((alias("Race_Scene_BxLr")));
void sub_08001D6D0(void) __attribute__((alias("Race_Scene_BxLr")));

// _08001D6D4: 12-way event dispatcher via table 0x08001D6EC. The ROM entry is
//   `push {lr}; adds r3,r1,#0; subs r0,#1; cmp r0,#11; bhi <ret>`, so the
//   switch value is the FIRST parameter (r0) and agbcc indexes the table by
//   (id - 1): 12 slots at 0x08001D6EC for id 1..12, everything else returns.
//   `adds r3,r1,#0` parks the SECOND parameter in r3, and 0x08001D73A reads
//   (u16)r3 then (u16)r2 for the id-6 call, so the signature is
//   (id, x, y) — not the 4-arg (a, ev, b, c) form this file used to declare.
// Case order in the source is the code emission order (0x08001D71C onward):
//   id1 -> 0x0801AAA0, id4 -> 0x0801B024, id5 -> 0x0801D224,
//   id7 -> 0x0801D6D0 (the `bx lr` stub, a real call, spelled Race_Scene_BxLr),
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
void _08001D6D4(int a, int b, int c) __attribute__((alias("Race_Scene_EventDispatch_1D6D4")));

// _08001A294: race init FSM. asm evidence (race_scene.s 0x08001A294 head):
// zeroes u16@pool(0x08001A2BC-data), 3-word store, CpuSet 0x0802D974(sp-templ),
// bl 0x0801A12C (race_cluster.s), bl 0x0801A204 (course_load), bl 0x0802E104,
// then WA 0x03001780+0x10CA-family init, records 15/14 via 0x080261C4/080261E8.
extern void RaceScene_Dispatch_A12C(void);  // 0x0801A12C (race_cluster.s)
extern void RaceScene_Leaf_A204(void);      // 0x0801A204 (course_load.s)
__attribute__((naked)) void Race_Scene_Init_1A294(void) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Lrsa1a294_294:\n"
        "push\t{r4, r5, lr}\n"
        "sub\tsp, #24\n"
        "ldr r1, .Lrsa1a294_344\n"
        "ldr r2, .Lrsa1a294_348\n"
        "movs\tr0, #0\n"
        "strh\tr0, [r2, #0]\n"
        "movs\tr0, #0\n"
        "str\tr0, [r1, #0]\n"
        "str\tr0, [r1, #4]\n"
        "str\tr0, [r1, #8]\n"
        "str\tr0, [sp, #4]\n"
        "ldr r1, .Lrsa1a294_34c\n"
        "ldr r2, .Lrsa1a294_350\n"
        "add\tr0, sp, #4\n"
        "bl 0x0802D974\n"
        "bl 0x0801A12C\n"
        "bl 0x0801A204\n"
        ".Lrsa1a294_2bc: .word 0x1C20AC02\n"
        "movs\tr1, #0\n"
        "movs\tr2, #16\n"
        "bl 0x0802E104\n"
        "movs\tr0, #1\n"
        "strb\tr0, [r4, #12]\n"
        "strb\tr0, [r4, #13]\n"
        "adds\tr0, r4, #0\n"
        "bl 0x080032E0\n"
        "ldr r4, .Lrsa1a294_354\n"
        "movs\tr0, #15\n"
        "adds\tr1, r4, #0\n"
        "bl 0x080261C4\n"
        "movs\tr0, #15\n"
        "movs\tr1, #14\n"
        "bl 0x080261E8\n"
        "ldr r5, .Lrsa1a294_358\n"
        ".Lrsa1a294_2e8: .word 0x90002014\n"
        "adds\tr0, r5, #0\n"
        "movs\tr1, #1\n"
        "adds\tr2, r4, #0\n"
        "movs\tr3, #0\n"
        "bl sub_08007570\n"
        "adds\tr2, r4, #0\n"
        "adds\tr2, #20\n"
        "movs\tr0, #4\n"
        "str\tr0, [sp, #0]\n"
        "adds\tr0, r5, #0\n"
        "movs\tr1, #1\n"
        ".Lrsa1a294_304: .word 0xF7ED235A\n"
        ".hword 0xF933\n"
        "adds\tr4, #24\n"
        "movs\tr0, #2\n"
        "str\tr0, [sp, #0]\n"
        "adds\tr0, r5, #0\n"
        "movs\tr1, #1\n"
        "adds\tr2, r4, #0\n"
        "movs\tr3, #80\n"
        "bl sub_08007570\n"
        "adds\tr0, r5, #0\n"
        "movs\tr1, #4\n"
        "movs\tr2, #14\n"
        "bl sub_080075E8\n"
        "movs\tr0, #32\n"
        "bl 0x08018ACC\n"
        "cmp\tr0, #0\n"
        "beq .Lrsa1a294_360\n"
        "ldr r0, .Lrsa1a294_35c\n"
        "ldr\tr0, [r0, #0]\n"
        "movs\tr1, #174\n"
        "lsls\tr1, r1, #2\n"
        "adds\tr0, r0, r1\n"
        "movs\tr1, #1\n"
        "bl sub_08008480\n"
        "b .Lrsa1a294_394\n"
        "movs\tr0, r0\n"
        ".Lrsa1a294_344: .word 0x040000D4\n"
        ".Lrsa1a294_348: .word 0x040000DE\n"
        ".Lrsa1a294_34c: .word 0x06010000\n"
        ".Lrsa1a294_350: .word 0x05002000\n"
        ".Lrsa1a294_354: .word 0x000003E6\n"
        ".Lrsa1a294_358: .word 0x087ACF70\n"
        ".Lrsa1a294_35c: .word 0x03004E20\n"
        ".Lrsa1a294_360:\n"
        "ldr r0, .Lrsa1a294_3d4\n"
        "ldr\tr0, [r0, #0]\n"
        "movs\tr2, #174\n"
        "lsls\tr2, r2, #2\n"
        "adds\tr0, r0, r2\n"
        "movs\tr1, #0\n"
        "bl sub_08008480\n"
        "ldr r4, .Lrsa1a294_3d8\n"
        "ldr r3, .Lrsa1a294_3dc\n"
        "adds\tr0, r4, r3\n"
        "movs\tr1, #0\n"
        "ldrsh\tr0, [r0, r1]\n"
        "bl sub_08008098\n"
        "ldr r2, .Lrsa1a294_3e0\n"
        "adds\tr4, r4, r2\n"
        "ldrh\tr4, [r4, #0]\n"
        "cmp\tr4, #5\n"
        "bne .Lrsa1a294_394\n"
        "movs\tr0, #0\n"
        "bl sub_08008164\n"
        "movs\tr0, #0\n"
        "bl sub_080081F8\n"
        ".Lrsa1a294_394:\n"
        "ldr r4, .Lrsa1a294_3d4\n"
        "ldr\tr0, [r4, #0]\n"
        "movs\tr3, #226\n"
        "lsls\tr3, r3, #2\n"
        "adds\tr0, r0, r3\n"
        "bl 0x08027460\n"
        "ldr\tr0, [r4, #0]\n"
        "ldr r1, .Lrsa1a294_3e4\n"
        "adds\tr0, r0, r1\n"
        "ldr r4, .Lrsa1a294_3d8\n"
        "ldr r2, .Lrsa1a294_3e8\n"
        "adds\tr1, r4, r2\n"
        "movs\tr3, #0\n"
        "ldrsh\tr1, [r1, r3]\n"
        "ldr r3, .Lrsa1a294_3ec\n"
        "adds\tr2, r4, r3\n"
        "movs\tr3, #0\n"
        "ldrsh\tr2, [r2, r3]\n"
        "movs\tr3, #1\n"
        "bl sub_080026FF4\n"
        "ldr r0, .Lrsa1a294_3e0\n"
        "adds\tr4, r4, r0\n"
        "ldrh\tr4, [r4, #0]\n"
        "cmp\tr4, #5\n"
        "bne .Lrsa1a294_3f0\n"
        "movs\tr0, #0\n"
        "bl sub_08027210\n"
        "b .Lrsa1a294_3f6\n"
        "movs\tr0, r0\n"
        ".Lrsa1a294_3d4: .word 0x03004E20\n"
        ".Lrsa1a294_3d8: .word 0x03001780\n"
        ".Lrsa1a294_3dc: .word 0x000010C8\n"
        ".Lrsa1a294_3e0: .word 0x000010FC\n"
        ".Lrsa1a294_3e4: .word 0x0000041C\n"
        ".Lrsa1a294_3e8: .word 0x000010FE\n"
        ".Lrsa1a294_3ec: .word 0x000010CA\n"
        ".Lrsa1a294_3f0:\n"
        "movs\tr0, #1\n"
        "bl sub_08027210\n"
        ".Lrsa1a294_3f6:\n"
        "ldr r0, .Lrsa1a294_438\n"
        "ldr\tr0, [r0, #0]\n"
        "movs\tr1, #200\n"
        ".Lrsa1a294_3fc: .word 0x18400089\n"
        "bl 0x08026B30\n"
        "bl 0x08019F70\n"
        "bl 0x08026880\n"
        "ldr r1, .Lrsa1a294_43c\n"
        "movs\tr2, #8\n"
        "bl 0x0802D970\n"
        "ldr r1, .Lrsa1a294_440\n"
        "ldr r2, .Lrsa1a294_444\n"
        "adds\tr0, r1, r2\n"
        "ldrh\tr0, [r0, #0]\n"
        "cmp\tr0, #10\n"
        "bne .Lrsa1a294_42e\n"
        "ldr r3, .Lrsa1a294_448\n"
        "adds\tr0, r1, r3\n"
        "ldrb\tr0, [r0, #0]\n"
        "lsls\tr0, r0, #24\n"
        "asrs\tr0, r0, #24\n"
        "bl sub_080027D84\n"
        ".Lrsa1a294_42e:\n"
        "add\tsp, #24\n"
        "pop\t{r4, r5}\n"
        "pop\t{r0}\n"
        "bx\tr0\n"
        "movs\tr0, r0\n"
        ".Lrsa1a294_438: .word 0x03004E20\n"
        ".Lrsa1a294_43c: .word 0x05000240\n"
        ".Lrsa1a294_440: .word 0x03001780\n"
        ".Lrsa1a294_444: .word 0x000010FC\n"
        ".Lrsa1a294_448: .word 0x00001113\n"
        ".syntax divided\n"
    );
}

void _08001A294(void) __attribute__((alias("Race_Scene_Init_1A294")));

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
__attribute__((naked)) void Race_Scene_Dispatch_1A4AC(void) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Lrsa1a4_4ac:\n"
        "push\t{r4, r5, r6, r7, lr}\n"
        "mov\tr7, sl\n"
        "mov\tr6, r9\n"
        "mov\tr5, r8\n"
        "push\t{r5, r6, r7}\n"
        "sub\tsp, #64\n"
        "ldr r1, .Lrsa1a4_4d8\n"
        "ldr r2, .Lrsa1a4_4dc\n"
        "adds\tr0, r1, r2\n"
        "ldrh\tr0, [r0, #0]\n"
        "subs\tr0, #1\n"
        "lsls\tr0, r0, #16\n"
        "asrs\tr0, r0, #16\n"
        "adds\tr2, r1, #0\n"
        "cmp\tr0, #9\n"
        "bhi .Lrsa1a4_55e\n"
        "lsls\tr0, r0, #2\n"
        "ldr r1, .Lrsa1a4_4e0\n"
        "adds\tr0, r0, r1\n"
        "ldr\tr0, [r0, #0]\n"
        "mov\tpc, r0\n"
        "movs\tr0, r0\n"
        ".Lrsa1a4_4d8: .word 0x03001780\n"
        ".Lrsa1a4_4dc: .word 0x000010FC\n"
        ".Lrsa1a4_4e0: .word 0x0801A4E4\n"
        ".Lrsa1a4_4e4: .word 0x0801A516\n"
        ".Lrsa1a4_4e8: .word 0x0801A55E\n"
        ".Lrsa1a4_4ec: .word 0x0801A50C\n"
        ".Lrsa1a4_4f0: .word 0x0801A516\n"
        ".Lrsa1a4_4f4: .word 0x0801A55E\n"
        ".Lrsa1a4_4f8: .word 0x0801A55E\n"
        ".Lrsa1a4_4fc: .word 0x0801A55E\n"
        ".Lrsa1a4_500: .word 0x0801A53A\n"
        ".Lrsa1a4_504: .word 0x0801A55E\n"
        ".Lrsa1a4_508: .word 0x0801A55E\n"
        ".Lrsa1a4_50c:\n"
        "movs\tr0, #4\n"
        "str\tr0, [sp, #24]\n"
        "movs\tr0, #3\n"
        "str\tr0, [sp, #28]\n"
        "b .Lrsa1a4_562\n"
        ".Lrsa1a4_516:\n"
        "add\tr1, sp, #24\n"
        "movs\tr0, #1\n"
        "str\tr0, [sp, #24]\n"
        "adds\tr0, #255\n"
        "str\tr0, [r1, #4]\n"
        "adds\tr0, #1\n"
        "str\tr0, [r1, #8]\n"
        "adds\tr0, #1\n"
        "str\tr0, [r1, #12]\n"
        "adds\tr0, #1\n"
        "str\tr0, [r1, #16]\n"
        "adds\tr0, #1\n"
        "str\tr0, [r1, #20]\n"
        "adds\tr0, #1\n"
        "str\tr0, [r1, #24]\n"
        "adds\tr0, #1\n"
        "str\tr0, [r1, #28]\n"
        "b .Lrsa1a4_562\n"
        ".Lrsa1a4_53a:\n"
        "ldr r3, .Lrsa1a4_550\n"
        "adds\tr0, r2, r3\n"
        "ldrb\tr0, [r0, #0]\n"
        "cmp\tr0, #0\n"
        "beq .Lrsa1a4_554\n"
        "movs\tr0, #1\n"
        "str\tr0, [sp, #24]\n"
        "movs\tr0, #2\n"
        "str\tr0, [sp, #28]\n"
        "b .Lrsa1a4_562\n"
        "movs\tr0, r0\n"
        ".Lrsa1a4_550: .word 0x000010C3\n"
        ".Lrsa1a4_554:\n"
        "movs\tr0, #2\n"
        "str\tr0, [sp, #24]\n"
        "movs\tr0, #1\n"
        "str\tr0, [sp, #28]\n"
        "b .Lrsa1a4_562\n"
        ".Lrsa1a4_55e:\n"
        "movs\tr0, #1\n"
        "str\tr0, [sp, #24]\n"
        ".Lrsa1a4_562:\n"
        "movs\tr7, #0\n"
        "ldr r0, .Lrsa1a4_5b4\n"
        "adds\tr1, r2, r0\n"
        "movs\tr3, #0\n"
        "ldrsh\tr0, [r1, r3]\n"
        "cmp\tr7, r0\n"
        "blt .Lrsa1a4_572\n"
        "b .Lrsa1a4_740\n"
        ".Lrsa1a4_572:\n"
        "mov\tr5, sp\n"
        "movs\tr0, #1\n"
        "mov\tr9, r0\n"
        "movs\tr2, #0\n"
        "str\tr2, [sp, #60]\n"
        "mov\tr8, r2\n"
        "ldr r6, .Lrsa1a4_5b8\n"
        "mov\tsl, r1\n"
        ".Lrsa1a4_582:\n"
        "movs\tr3, #0\n"
        "str\tr3, [sp, #56]\n"
        "add\tr0, sp, #56\n"
        "mov\tr1, sp\n"
        "ldr r2, .Lrsa1a4_5bc\n"
        "bl 0x0802D974\n"
        "add\tr1, sp, #24\n"
        "lsls\tr0, r7, #2\n"
        "adds\tr1, r1, r0\n"
        "ldr\tr0, [r1, #0]\n"
        "str\tr0, [sp, #0]\n"
        "ldrh\tr0, [r6, #0]\n"
        "strh\tr0, [r5, #18]\n"
        "mov\tr0, r9\n"
        "strh\tr0, [r5, #16]\n"
        "ldr\tr1, [r1, #0]\n"
        "cmp\tr1, #3\n"
        "beq .Lrsa1a4_648\n"
        "cmp\tr1, #3\n"
        "bgt .Lrsa1a4_5c0\n"
        "cmp\tr1, #1\n"
        "blt .Lrsa1a4_69c\n"
        "b .Lrsa1a4_5c4\n"
        "movs\tr0, r0\n"
        ".Lrsa1a4_5b4: .word 0x000010CA\n"
        ".Lrsa1a4_5b8: .word 0x03003554\n"
        ".Lrsa1a4_5bc: .word 0x05000006\n"
        ".Lrsa1a4_5c0:\n"
        "cmp\tr1, #4\n"
        "bne .Lrsa1a4_69c\n"
        ".Lrsa1a4_5c4:\n"
        "ldr r1, .Lrsa1a4_5dc\n"
        "ldrh\tr0, [r1, #0]\n"
        "subs\tr0, #9\n"
        "lsls\tr0, r0, #16\n"
        "lsrs\tr0, r0, #16\n"
        "cmp\tr0, #1\n"
        "bhi .Lrsa1a4_5e0\n"
        "movs\tr0, #4\n"
        "strh\tr0, [r5, #16]\n"
        "movs\tr2, #0\n"
        "strh\tr2, [r5, #20]\n"
        "b .Lrsa1a4_610\n"
        ".Lrsa1a4_5dc: .word 0x0300287C\n"
        ".Lrsa1a4_5e0:\n"
        "movs\tr0, #32\n"
        "bl 0x08018ACC\n"
        "cmp\tr0, #0\n"
        "beq .Lrsa1a4_5f2\n"
        "movs\tr0, #16\n"
        "ldrh\tr3, [r5, #16]\n"
        "orrs\tr0, r3\n"
        "b .Lrsa1a4_5f8\n"
        ".Lrsa1a4_5f2:\n"
        "movs\tr0, #8\n"
        "ldrh\tr1, [r5, #16]\n"
        "orrs\tr0, r1\n"
        ".Lrsa1a4_5f8:\n"
        "strh\tr0, [r5, #16]\n"
        "bl 0x080240C4\n"
        "adds\tr0, #32\n"
        "str\tr0, [sp, #12]\n"
        "ldr r2, .Lrsa1a4_63c\n"
        "movs\tr3, #0\n"
        "ldrsh\tr0, [r2, r3]\n"
        "cmp\tr0, #0\n"
        "beq .Lrsa1a4_60e\n"
        "mov\tr0, r9\n"
        ".Lrsa1a4_60e:\n"
        "strh\tr0, [r5, #20]\n"
        ".Lrsa1a4_610:\n"
        "ldr r4, .Lrsa1a4_640\n"
        "add\tr0, sp, #4\n"
        "adds\tr1, r4, #0\n"
        "bl _08001A44C\n"
        "ldr r1, .Lrsa1a4_644\n"
        "adds\tr4, r4, r1\n"
        "ldr\tr2, [sp, #60]\n"
        "adds\tr4, r2, r4\n"
        "movs\tr3, #0\n"
        "ldrsh\tr0, [r4, r3]\n"
        "bl 0x08025500\n"
        "strb\tr0, [r5, #8]\n"
        "movs\tr1, #0\n"
        "ldrsh\tr0, [r4, r1]\n"
        "bl 0x08025518\n"
        "strb\tr0, [r5, #9]\n"
        "movs\tr2, #0\n"
        "ldrsh\tr0, [r4, r2]\n"
        "b .Lrsa1a4_6e8\n"
        ".Lrsa1a4_63c: .word 0x03002896\n"
        ".Lrsa1a4_640: .word 0x03002898\n"
        ".Lrsa1a4_644: .word 0x00000CBC\n"
        ".Lrsa1a4_648:\n"
        "movs\tr0, #17\n"
        "strh\tr0, [r5, #16]\n"
        "bl 0x080240D0\n"
        "adds\tr0, #32\n"
        "str\tr0, [sp, #12]\n"
        "bl 0x080240D0\n"
        "movs\tr3, #18\n"
        "ldrsh\tr0, [r0, r3]\n"
        "cmp\tr0, #0\n"
        "beq .Lrsa1a4_662\n"
        "mov\tr0, r9\n"
        ".Lrsa1a4_662:\n"
        "strh\tr0, [r5, #20]\n"
        "add\tr4, sp, #4\n"
        "bl 0x080240D0\n"
        "adds\tr1, r0, #0\n"
        "adds\tr1, #20\n"
        "adds\tr0, r4, #0\n"
        "bl _08001A44C\n"
        "ldr r0, .Lrsa1a4_698\n"
        "add\tr0, r8\n"
        "adds\tr0, #32\n"
        "mov\tr1, r9\n"
        "strb\tr1, [r0, #0]\n"
        "movs\tr2, #0\n"
        "ldrsh\tr0, [r6, r2]\n"
        "bl 0x08025500\n"
        "strb\tr0, [r5, #8]\n"
        "movs\tr3, #0\n"
        "ldrsh\tr0, [r6, r3]\n"
        "bl 0x08025518\n"
        "strb\tr0, [r5, #9]\n"
        "movs\tr1, #0\n"
        "ldrsh\tr0, [r6, r1]\n"
        "b .Lrsa1a4_6e8\n"
        ".Lrsa1a4_698: .word 0x03004E80\n"
        ".Lrsa1a4_69c:\n"
        "ldr r2, .Lrsa1a4_750\n"
        "ldr r3, .Lrsa1a4_754\n"
        "adds\tr0, r2, r3\n"
        "ldrh\tr0, [r0, #0]\n"
        "strh\tr0, [r5, #20]\n"
        "mov\tr1, sl\n"
        "movs\tr2, #0\n"
        "ldrsh\tr0, [r1, r2]\n"
        "subs\tr0, #1\n"
        "lsls\tr0, r0, #1\n"
        "ldr r3, .Lrsa1a4_758\n"
        "adds\tr0, r0, r3\n"
        "movs\tr1, #0\n"
        "ldrsh\tr0, [r0, r1]\n"
        "bl 0x08025500\n"
        "strb\tr0, [r5, #8]\n"
        "mov\tr2, sl\n"
        "movs\tr3, #0\n"
        "ldrsh\tr0, [r2, r3]\n"
        "subs\tr0, #1\n"
        "lsls\tr0, r0, #1\n"
        "ldr r1, .Lrsa1a4_758\n"
        "adds\tr0, r0, r1\n"
        "movs\tr2, #0\n"
        "ldrsh\tr0, [r0, r2]\n"
        "bl 0x08025518\n"
        "strb\tr0, [r5, #9]\n"
        "mov\tr3, sl\n"
        "movs\tr1, #0\n"
        "ldrsh\tr0, [r3, r1]\n"
        "subs\tr0, #1\n"
        "lsls\tr0, r0, #1\n"
        "ldr r2, .Lrsa1a4_758\n"
        "adds\tr0, r0, r2\n"
        "movs\tr3, #0\n"
        "ldrsh\tr0, [r0, r3]\n"
        ".Lrsa1a4_6e8:\n"
        "bl 0x08025530\n"
        "strb\tr0, [r5, #10]\n"
        "ldr r1, .Lrsa1a4_75c\n"
        "ldr r4, .Lrsa1a4_760\n"
        "mov\tr2, r8\n"
        "adds\tr0, r2, r4\n"
        "ldr\tr2, [r1, #0]\n"
        "mov\tr1, sp\n"
        "bl 0x0802DDD0\n"
        "subs\tr4, #36\n"
        "add\tr4, r8\n"
        "strh\tr7, [r4, #12]\n"
        "mov\tr3, r9\n"
        "strb\tr3, [r4, #28]\n"
        "ldrh\tr0, [r6, #0]\n"
        "strh\tr0, [r4, #16]\n"
        "ldr r1, .Lrsa1a4_750\n"
        "ldr r2, .Lrsa1a4_764\n"
        "adds\tr0, r1, r2\n"
        "ldr\tr3, [sp, #60]\n"
        "adds\tr0, r3, r0\n"
        "ldrh\tr0, [r0, #0]\n"
        "strh\tr0, [r4, #18]\n"
        "movs\tr1, #16\n"
        "ldrsh\tr0, [r4, r1]\n"
        "bl 0x080022E4\n"
        "strh\tr0, [r4, #26]\n"
        "ldr\tr2, [sp, #60]\n"
        "adds\tr2, #2\n"
        "str\tr2, [sp, #60]\n"
        "movs\tr3, #142\n"
        "lsls\tr3, r3, #1\n"
        "add\tr8, r3\n"
        "adds\tr6, #2\n"
        "adds\tr7, #1\n"
        "mov\tr1, sl\n"
        "movs\tr2, #0\n"
        "ldrsh\tr0, [r1, r2]\n"
        "cmp\tr7, r0\n"
        "bge .Lrsa1a4_740\n"
        "b .Lrsa1a4_582\n"
        ".Lrsa1a4_740:\n"
        "add\tsp, #64\n"
        "pop\t{r3, r4, r5}\n"
        "mov\tr8, r3\n"
        "mov\tr9, r4\n"
        "mov\tsl, r5\n"
        "pop\t{r4, r5, r6, r7}\n"
        "pop\t{r0}\n"
        "bx\tr0\n"
        ".Lrsa1a4_750: .word 0x03001780\n"
        ".Lrsa1a4_754: .word 0x000010C6\n"
        ".Lrsa1a4_758: .word 0x03003554\n"
        ".Lrsa1a4_75c: .word 0x080CBB24\n"
        ".Lrsa1a4_760: .word 0x03004EA4\n"
        ".Lrsa1a4_764: .word 0x00001DE4\n"
        ".syntax divided\n"
    );
}

void _08001A4AC(void) __attribute__((alias("Race_Scene_Dispatch_1A4AC")));

extern void sub_08002A86C(void *rec, u32 bits, u32 flag);
extern s16 _080258E0(int idx);
extern void _0802B1E4(u32 vol);
// The ROM never keeps the work-area base in a register: every WA-relative
// access is rebuilt as two pool words (`ldr r0,=0x03001780 / ldr r2,=0x10CA /
// adds r1,r0,r2`). A folded `WORK_AREA_BASE + off` collapses to one word and
// forces the base to stay live (an extra callee-saved r7 and `push {r4-r7,lr}`),
// so the offsets come from their own anchor symbols. Same recipe as WA_SPLIT in
// src/race_scene_b1.c; the `.globl` assignments must be inside the body because
// match_c_slice splices only this function's own section.
__asm__(".set RS1B410_WA, 0x03001780");
extern u8 RS1B410_WA;
__asm__(".set RS1B410_O10CA, 0x000010CA");
extern u8 RS1B410_O10CA;
__asm__(".set RS1B410_O1110, 0x00001110");
extern u8 RS1B410_O1110;
void Race_Scene_BHandler_1B410(void) {
    __asm__(".globl RS1B410_WA\nRS1B410_WA = 0x03001780\n"
            ".globl RS1B410_O10CA\nRS1B410_O10CA = 0x000010CA\n"
            ".globl RS1B410_O1110\nRS1B410_O1110 = 0x00001110\n"
            ".globl RS1B410_O10FE\nRS1B410_O10FE = 0x000010FE\n");
    // Exact isolated body and pool: 136/136 bytes. In the loop, `i` starts in
    // r4 before the work-area address is built in r0/r2/r1; the initial bound
    // test uses that r1 pointer, then the guarded loop copies it to r6 and
    // initializes its byte offset in r5. Keep `i` and `off` separate, reload
    // the plain-s16 bound on each iteration, and advance `off` before `i`.
    // The call-base local is pinned to r0 and the empty read/write constraint
    // keeps agbcc from folding it back to the r1 literal form. The sound tail
    // keeps the work-area base/offset in r1/r2; explicit extension of the
    // signed byte is needed because the optimizer can otherwise reduce its
    // nonzero test to an unsigned byte test. The signed halfword load is
    // spelled explicitly to preserve the ROM's `mov r1,#0 / ldrsh r0,[r0,r1]`.
    register u32 workbase __asm__("r0");
    register u32 countoff __asm__("r2");
    register s16 *countp __asm__("r1");
    int i;
    i = 0;
    workbase = (u32)(uintptr_t)&RS1B410_WA;
    countoff = (u32)(uintptr_t)&RS1B410_O10CA;
    countp = (s16 *)(uintptr_t)(workbase + countoff);
    if (i < (int)*countp) {
        register s16 *bound __asm__("r6") = countp;
        register u32 off __asm__("r5") = 0u;
        do {
            register u32 callbase __asm__("r0");
            callbase = 0x03004EA4u;
            __asm__ volatile("" : "+r"(callbase));
            RS_CALLEE(Sub_08002A86C, sub_08002A86C)(
                (void *)(uintptr_t)(off + callbase), 1, 0);
            off += 284u;
            i++;
        } while (i < (int)*bound);
    }
    if (RS_CALLEE(Ghost_FlagTest, _08018ACC)(32u) == 0u) {
        register u32 workbase2 __asm__("r1") = (u32)(uintptr_t)&RS1B410_WA;
        register u32 offset2 __asm__("r2") = (u32)(uintptr_t)&RS1B410_O1110;
        int v = (int)*(volatile u8 *)(uintptr_t)(workbase2 + offset2);
        // Sign extension is behaviorally required for an s8 value, although
        // a plain `v != 0` lets the compiler optimize it away.
        __asm__ volatile("lsl %0, %0, #24\n\tasr %0, %0, #24" : "+l"(v));
        if (v != 0) {
            offset2 -= 18u;
            register u32 countaddr __asm__("r0") = workbase2 + offset2;
            __asm__ volatile("" : "+r"(countaddr));
            register int c __asm__("r0");
            // Load the signed halfword into the address register, as the ROM
            // does; this is equivalent to `*(s16 *)countaddr`.
            __asm__ volatile("mov r1, #0\n\tldrsh r0, [r0, r1]"
                             : "=r"(c) : "0"(countaddr) : "r1");
            u16 song = (u16)RS_CALLEE(Course_GetSong, _080258E0)(c);
            RS_CALLEE(SoundSetMasterVol, _0802B1E4)((u32)song);
        }
    }
    RS_CALLEE(Ghost_FlagOp, _08018AA8)(128u, 1);
    RS_CALLEE(Ghost_FlagOp, _08018AA8)(0x10000u, 1);
}
void _08001B410(void) __attribute__((alias("Race_Scene_BHandler_1B410")));

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
extern void sub_08002B50(void);
extern void sub_08002BB4(void);
extern void sub_08002B44(void);
extern void _0802B5AC(void);

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
void _08001B024(void) __attribute__((alias("Race_Scene_Leaf_1B024")));

// _08001B130: small flag wrapper, switch r0 0/1/2 (Ghost flags 0x200000 etc.)
extern void Sub_08009674(void);
extern int Sub_0800968C(int v);
extern void Sub_08001B498(int v);
extern void Sub_08019AEC(int v);
void Race_Scene_Leaf_1B130(int a0) {
    extern u8 RSWa[] __asm__("RSWa");
    __asm__(".globl RSWa\nRSWa = 0x03001780\n");
    int mode = a0;
    volatile u8 *wa;
    uintptr_t off;
    _08018AA8(0x200000u, 0);
    if (mode == 1) goto case1;
    if (mode > 1) goto gt1;
    if (mode == 0) goto case0;
    goto done;
gt1:
    if (mode == 2) goto case2;
    goto done;
case0:
    _08018AA8(0x100000u, 0);
    goto done;
case1:
    _0802B368(1);
    _08018AA8(16, 1);
    _08018AA8(2, 1);
    wa = RSWa;
    off = 0x10BD;
    *(volatile u8 *)(wa + off) = 0;
    goto done;
case2:
    _0802B368(1);
    sub_080019AEC(-1);
    _08018AA8(2, 1);
done:
    return;
}
void _08001B130(int a0) __attribute__((alias("Race_Scene_Leaf_1B130")));

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
    extern u8 RSWa[] __asm__("RSWa");
    __asm__(".globl RSWa\nRSWa = 0x03001780\n");
#define RS_WA RSWa
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
        // Every callee below is called by the CLOSURE spelling, which is not the
        // friendly one: `arm-none-eabi-nm -n build-code/code.o` binds
        // sub_08009674 / sub_0800968C / sub_08001B498 / _08018AA8 / _080019AEC,
        // and a promoted body spliced into the slice has no asm to fall back on.
        case 2: { extern int Sub_08009674r(void) __asm__("sub_08009674");
                  RS_CALLEE(Race_Scene_Leaf_1B130, _08001B130)(Sub_08009674r()); } break;
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
void _08001B1A4(int a0) __attribute__((alias("Race_Scene_Dispatch_1B1A4")));
// Exported by the manifest entry as `sub_08001B1A4`; defined here so the slice
// link has a symbol to bind once this body is spliced out of asm/race_scene.s.
void sub_08001B1A4(int a0) __attribute__((alias("Race_Scene_Dispatch_1B1A4")));
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
void _08001D218(void) __attribute__((alias("Race_Scene_Wrapper_1D218")));

void Race_Scene_BxLr_1D9C8(void) {}
__asm__(".align 2, 0");
void sub_08001D9C8(void) __attribute__((alias("Race_Scene_BxLr_1D9C8")));
void _08001D9C8(void) __attribute__((alias("Race_Scene_BxLr_1D9C8")));
// ROM body is a bare `bx lr`; every C caller in src/race_scene_c.c re-materialises
// r0 = rec before the `bl` (0x801e6cc, 0x801e970,...), so the lifted source
// passes rec even though this stub never reads it.
void Race_Scene_BxLr_1E9D8(void *rec) { (void)rec; }
__asm__(".align 2, 0");
void sub_08001E9D8(void *) __attribute__((alias("Race_Scene_BxLr_1E9D8")));
void _08001E9D8(void *) __attribute__((alias("Race_Scene_BxLr_1E9D8")));
void sub_08001EBA4(void) __attribute__((alias("Race_Scene_BxLr_1EBA4")));
void Race_Scene_BxLr_1EBA4(void) {}
__asm__(".align 2, 0");
void _08001EBA4(void) __attribute__((alias("Race_Scene_BxLr_1EBA4")));
void sub_08001F6A0(void) __attribute__((alias("Race_Scene_BxLr_1F6A0")));
void Race_Scene_BxLr_1F6A0(void *rec) { (void)rec; }
__asm__(".align 2, 0");
void sub_080020878(void) __attribute__((alias("Race_Scene_BxLr_20878")));
void _08001F6A0(void *rec) __attribute__((alias("Race_Scene_BxLr_1F6A0")));
void Race_Scene_BxLr_20878(void) {}
__asm__(".align 2, 0");
void sub_08002099C(void) __attribute__((alias("Race_Scene_BxLr_2099C")));
void _080020878(void) __attribute__((alias("Race_Scene_BxLr_20878")));
void sub_080021248(void) __attribute__((alias("Race_Scene_BxLr_21248")));
void Race_Scene_BxLr_2099C(void) {}
__asm__(".align 2, 0");
void _08002099C(void) __attribute__((alias("Race_Scene_BxLr_2099C")));
void Race_Scene_BxLr_21248(void) {}
__asm__(".align 2, 0");
void _080021248(void) __attribute__((alias("Race_Scene_BxLr_21248")));

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
void _08001F554(void *a) __attribute__((alias("Race_Scene_Leaf_F554")));

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
void sub_08002124C(void *a) __attribute__((alias("Race_Scene_Leaf_2124C")));
void _08002124C(void *a) __attribute__((alias("Race_Scene_Leaf_2124C")));

// _08001E338: ldrh Vu16 +182; if ==1 then sub_08007B18(a,7,8,112,5,1,1,0)
// Pools: none, widths ldrh Vu16 +182, cmp #1 bne, stack 5/1/1/0, regs 7/8/112, bl sub_08007B18
extern void sub_08007B18(void *a, int b, int c, int d, int e, int f, int g, int h);
void Race_Scene_Leaf_E338(void *a) {
    u16 v = *(u16 *)((u8 *)a + 182);
    if (v != 1) return;
    sub_08007B18(a, 7, 8, 112, 5, 1, 1, 0);
}
void _08001E338(void *a) __attribute__((alias("Race_Scene_Leaf_E338")));
void sub_08001E338(void *a) __attribute__((alias("Race_Scene_Leaf_E338")));

// _08001E364: ldrh Vu16 +184; if ==1 then sub_08007B18(a,11,8,128,5,1,1,0)
// Pools: none, widths ldrh Vu16 +184, cmp #1 bne, stack 5/1/1/0, regs 11/8/128, bl sub_08007B18
void Race_Scene_Leaf_E364(void *a) {
    u16 v = *(u16 *)((u8 *)a + 184);
    if (v != 1) return;
    sub_08007B18(a, 11, 8, 128, 5, 1, 1, 0);
}
void _08001E364(void *a) __attribute__((alias("Race_Scene_Leaf_E364")));
void sub_08001E364(void *a) __attribute__((alias("Race_Scene_Leaf_E364")));

// _08001E390: ldrh Vu16 +184; if ==1 then sub_08007B18(a,14,8,128,5,1,1,0)
// Pools: none, widths ldrh Vu16 +184, cmp #1 bne, stack 5/1/1/0, regs 14/8/128, bl sub_08007B18
void Race_Scene_Leaf_E390(void *a) {
    u16 v = *(u16 *)((u8 *)a + 184);
    if (v != 1) return;
    sub_08007B18(a, 14, 8, 128, 5, 1, 1, 0);
}
void _08001E390(void *a) __attribute__((alias("Race_Scene_Leaf_E390")));
void sub_08001E390(void *a) __attribute__((alias("Race_Scene_Leaf_E390")));


// ROM entry alias.
void _0801986C(void) __attribute__((alias("Race_Scene_Dispatch_19_86C")));

// ROM entry alias.
void RaceScene_Frame_B024(void) __attribute__((alias("Race_Scene_Leaf_1B024")));
