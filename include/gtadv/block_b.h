#ifndef GTADV_BLOCK_B_H
#define GTADV_BLOCK_B_H

#include "gba/types.h"

// State block B at IWRAM 0x030000E8; pointer slot 0x030003F4.
// Layout from asm/blockb.s (see asm/agbmain.s)
typedef struct {
    u8  active;     // +0x00
    u8  ack;        // +0x01
    u8  _02[2];
    u16 matchCount; // +0x04 written by _080021A0
    u16 param;      // +0x06 set by _08002124, forwarded to block A
    s16 expected;   // +0x08 set by _080020E8
    u8  _0A[6];
} BlockB;

#define BLOCK_B_SLOT_ADDR 0x030000F4
#define BLOCK_B_ADDR      0x030000E8
#define BLOCK_B_SLOT      (*(BlockB **)BLOCK_B_SLOT_ADDR)
#define BLOCK_B           (*(volatile BlockB *)BLOCK_B_ADDR)

static inline volatile BlockB *BlockB_Get(void) {
    return (volatile BlockB *)*(volatile u32 *)BLOCK_B_SLOT_ADDR;
}

// C API — behavioral equivalent of asm/blockb.s
void BlockB_SetExpected(u16 v);          // _080020E8
void BlockB_Register(void);              // _080020F4
void BlockB_Reset(void);                 // _0800210C
void BlockB_Arm(u16 param);              // _08002124
s32  BlockB_GetMatchOrZero(void);        // _08002140 (word return: `adds r0,r2,#0`, no u16 mask)
void BlockB_ForwardEvent(int id, u16 payload); // _08002158
u16  BlockB_DispatchIfLess(int idx);     // _08002178 via _08001E5C
void BlockB_RefreshMatcher(void);        // _080021A0
void BlockB_Frame(void);                 // _080021EC
u16  BlockB_DispatchIfLess2(int idx);    // _0800222C via _08001E8C

#endif // GTADV_BLOCK_B_H
