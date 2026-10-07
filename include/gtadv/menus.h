#ifndef GTADV_MENUS_H
#define GTADV_MENUS_H

#include "gba/types.h"


#define MENU_WA_BASE 0x03001780u
#define MENU_SLOT_BASE 0x03001680u

// Packet consumer in raw region (shared UI packet handler)
extern void MenuPacketConsumer(void *pkt); // _080188B0 / 0x080188B0

// Leaves — packet builders, pollers, and small helpers (port leaves first)
void MenuPkt_08009B60(void *ctx);                 // menu_pkt.s type-9 builder
void MenuPoller_0800C814(void *ctx);              // menu_c814.s record poller
void MenuHandler_0800C884(void *ctx, u32 a, u32 b); // menu_c884.s high-reg handler
void MenuRenderer_0800CAE4(void *ctx);            // menu_cae4.s
void MenuHelper_0800CC38(void);                   // no-op leaf
void MenuDispatch_0800CC3C(u32 mode);             // menu_cc38.s 4-way dispatch
void MenuTrio_0800CCEC(void);                     // menu_ccec.s 3-handler trio
void MenuFamily_0800CE78(void);                   // menu_ce78.s 4-handler family
void MenuInit_0800CFE4(void *rec);                // menu_cfe4.s
void MenuUpdater_0800D048(void *rec, u32 a);      // menu_d048.s
void MenuSetup_0800C0F0(void);                    // menu_c0f0.s helpers
void MenuDispatch_C168(int id, u32 arg, int slot, void *rec); // menu_dispatch.s 14-entry ctor A (defined in src/runtime_state_dispatch.c)
void MenuCtor_0800C1E4(void *rec);                // menu_ctor2.s
void MenuC2C4_0800C2C4(void);                     // menu_c2c4.s 4 leaves
void MenuC340_0800C340(void);                     // menu_c340.s setup
void MenuC454_0800C454(void);                     // menu_c454.s
void MenuC4D0_0800C4D0(void *rec);                // menu_c4d0.s REC20 ctor
void MenuC604_0800C604(void *rec);                     // menu_c604.s mode toggle/teardown
void MenuC7F4_0800C7F4(void *rec);                     // menu_c7f4.s init
void MenuC814Alias(void *ctx);                   // alias for _0800C814
void MenuCE70_0800CE70(void *rec, void *ctx);     // menu_ce70.s mode-6 setter
// Two parameters: the ROM's `adds r1,#84` puts the base in r1, and the caller
// (asm/menu_ce78.s:170) sets only r0. One-arg form compiles the base into r0.

// Larger menu clusters (record/menu dispatchers — port after leaves)
void MenuRecordUpdate_0800BCD4(void *a0, u32 a1, void *a2, u32 a3, void *sp24, void *sp28, void *sp32, void *sp36); // menu_record_update.s
void MenuSetup_0800BC08(void *a0, void *a1, void *a2, void *a3); // menu_setup.s
// The 0x0800BD40 / 0x0800BE74 bodies live in src/scene_record_dispatch.c under these
// names (the earlier MenuProgress_0800BD40 / MenuPlace_0800BE74 spellings were
// declarations with no definition, which silently became no-op link stubs).
void MenuProgressUpdate_0BD40(void);              // menu_progress_update.s
void MenuRecordApply_0800BE20(void *rec);         // menu_record_apply.s
void MenuPlaceUpdate_0BE74(void);                 // menu_place.s placement/tier
void MenuCtor_0800C010(void *rec);                // menu_ctor.s clean ctor
void MenuD1B4_0800D1B4(void);                     // menu_d1b4.s cursor-box family
void MenuD280_0800D280(void *unused, void *rec);     // menu_d280.s leaf 0xD280 (true ABI: dead r0, rec r1)
void MenuD280b_0800D298(void); // 0xD298 tiny bl 0x02124/0x0254C
void MenuD3A4_0800D3A4(void);                     // menu_d3a4.s family
void MenuD470_0800D470(void);
void MenuD4EA_0800D4EA(void);
void MenuD8E4_0800D8E4(void);
void MenuD9A4_0800D9A4(void);
void MenuDAB8_0800DAB8(void);
void MenuDBE8_0800DBE8(void *rec);
void MenuE650_0800E650(void *rec, void *arg);                     // 34-entry record selector
void MenuEBD8_0800EBD8(void *rec, u32 cmd);
void MenuF040_0800F040(void *rec, u32 a1, u32 type);
void MenuF0BC_0800F0BC(void *rec, u32 a1, u32 a2);
void MenuF134_0800F134(void *rec, u32 a1, u32 a2);
void MenuF22C_0800F22C(void *rec);
void MenuF5A0_0800F5A0(volatile u8 *rec, u32 a2, u32 a3, u32 a4);                // 0xFBC countdown ticker
                                                                                 // (the 12-entry dispatcher is MenuF5EC @0x0800F5EC)
u32 MenuF6D0_0800F6D0(s32 idx);
u32 MenuF700_0800F700(s32 a, s32 b);
s16 MenuF744_0800F744(s32 idx);
void MenuF778_0800F778(u32 a, u32 b, u32 c);
void MenuF794_0800F794(void *rec);
void MenuF7C0_0800F7C0(void *rec);
s16 MenuF7EC_0800F7EC(s32 a, s32 b);
void MenuF810_0800F810(void *rec);
void MenuF8B4_0800F8B4(void *rec, u32 sel);
void MenuF924_0800F924(void *a, void *b);
void MenuFA24_0800FA24(void *rec);                     // record-49 setup
void MenuFF78_0800FF78(void *rec);                // big dispatcher + 117 funcs (menu_ff78.s)
void MenuD13C_0800D13C(void);
void MenuD7C4_0800D7C4(void *rec); // menu_d4ea.s tiny leaf *(u32*)rec=11, VMA 0x0800D7C4
void Menu12C7C_080012C7C(void); // menu_ff78.s tiny wrapper push/bl 0x04B68/ldrh/bl 0x02158, VMA 0x080012C7C
void Menu13E4C_080013E4C(void); // menu_ff78.s tiny wrapper push/bl 0x04B68/ldrh/bl 0x02158, VMA 0x080013E4C (next after 012C7C)
void Menu12E5C_080012E5C(int v); // menu_ff78.s tiny wrapper push/bl 0x07614, VMA 0x080012E5C (r0 forwarded into 07614's int 3rd arg; pool 0x082ECAE4)
void Menu14C70_080014C70(void *a, void *rec); // menu_ff78.s tiny wrapper push/bl 0x04B68/adds #84/strh #6, VMA 0x080014C70 (next smallest after 013E4C)
void Menu108D4_0800108D4(void *rec); // menu_ff78.s tiny leaf push/movs/str/strh/bl 0x02B368, VMA 0x0800108D4 (next after 012E5C, no pool)
void Menu108F0_0800108F0(void *rec); // menu_ff78.s tiny leaf push/movs/str/strh/bl 0x02B368/bl 0x02B234, VMA 0x0800108F0 (sibling to 0108D4, +92=1)
void Menu010914_080010914(void *rec, u32 dummy, u16 arg); // menu_ff78.s wrapper lsls/lsrs rec+168 cmp 3, VMA 0x080010914 (next after 0108F0, r2 arg)
void Menu010940_080010940(void *rec, u32 dummy, u16 arg); // menu_ff78.s wrapper same family as 010914, VMA 0x080010940 (next after 010914)
void Menu1529C_08001529C(void); // menu_ff78.s tiny wrapper bl 0x02B234, VMA 0x08001529C

// Menu pocket already in carphys_tick.s but owned by this lane's docs
void MenuRec13_0800A62C(void *ctx);
void MenuRec0_08009BF8(void *ctx);
void MenuRec51_08009BCC(void *ctx);

// Save/menus bridge helpers (used by menu ROM via numeric BL)
void MenuSaveBridge_0800D77C(void);
void MenuSaveBridge_0800D95C(void *dst, const void *src, u32 n);

#endif // GTADV_MENUS_H
