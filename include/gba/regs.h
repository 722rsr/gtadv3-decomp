#ifndef GBA_REGS_H
#define GBA_REGS_H

#include "types.h"

#define REG_BASE 0x04000000

// Display
#define REG_DISPCNT    (*(vu16 *)(REG_BASE + 0x000))
#define REG_DISPSTAT   (*(vu16 *)(REG_BASE + 0x004))
#define REG_VCOUNT     (*(vu16 *)(REG_BASE + 0x006))
#define REG_BG0CNT     (*(vu16 *)(REG_BASE + 0x008))
#define REG_BG1CNT     (*(vu16 *)(REG_BASE + 0x00A))
#define REG_BG2CNT     (*(vu16 *)(REG_BASE + 0x00C))
#define REG_BG3CNT     (*(vu16 *)(REG_BASE + 0x00E))

// Input
#define REG_KEYINPUT   (*(vu16 *)(REG_BASE + 0x130))
#define REG_KEYCNT     (*(vu16 *)(REG_BASE + 0x132))

// Serial / SIO
#define REG_SIOCNT     (*(vu16 *)(REG_BASE + 0x128))
#define REG_SIODATA8   (*(vu8  *)(REG_BASE + 0x12A))
#define REG_SIODATA32  (*(vu32 *)(REG_BASE + 0x120))
#define REG_SIOMLT_SEND (*(vu16 *)(REG_BASE + 0x12A))
#define REG_RCNT       (*(vu16 *)(REG_BASE + 0x134))

// Timers
#define REG_TM0CNT_L (*(vu16 *)(REG_BASE + 0x100))
#define REG_TM0CNT_H (*(vu16 *)(REG_BASE + 0x102))
#define REG_TM1CNT_L (*(vu16 *)(REG_BASE + 0x104))
#define REG_TM1CNT_H (*(vu16 *)(REG_BASE + 0x106))
#define REG_TM2CNT_L (*(vu16 *)(REG_BASE + 0x108))
#define REG_TM2CNT_H (*(vu16 *)(REG_BASE + 0x10A))
#define REG_TM3CNT_L (*(vu16 *)(REG_BASE + 0x10C))
#define REG_TM3CNT_H (*(vu16 *)(REG_BASE + 0x10E))

// Interrupts
#define REG_IE   (*(vu16 *)(REG_BASE + 0x200))
#define REG_IF   (*(vu16 *)(REG_BASE + 0x202))
#define REG_IME  (*(vu16 *)(REG_BASE + 0x208))

// DMA
#define REG_DMA0SAD (*(vu32 *)(REG_BASE + 0x0B0))
#define REG_DMA0DAD (*(vu32 *)(REG_BASE + 0x0B4))
#define REG_DMA0CNT_L (*(vu16 *)(REG_BASE + 0x0B8))
#define REG_DMA0CNT_H (*(vu16 *)(REG_BASE + 0x0BA))
#define REG_DMA1SAD (*(vu32 *)(REG_BASE + 0x0BC))
#define REG_DMA1DAD (*(vu32 *)(REG_BASE + 0x0C0))
#define REG_DMA1CNT_L (*(vu16 *)(REG_BASE + 0x0C4))
#define REG_DMA1CNT_H (*(vu16 *)(REG_BASE + 0x0C6))
#define REG_DMA2SAD (*(vu32 *)(REG_BASE + 0x0C8))
#define REG_DMA2DAD (*(vu32 *)(REG_BASE + 0x0CC))
#define REG_DMA2CNT_L (*(vu16 *)(REG_BASE + 0x0D0))
#define REG_DMA2CNT_H (*(vu16 *)(REG_BASE + 0x0D2))
#define REG_DMA3SAD (*(vu32 *)(REG_BASE + 0x0D4))
#define REG_DMA3DAD (*(vu32 *)(REG_BASE + 0x0D8))
#define REG_DMA3CNT_L (*(vu16 *)(REG_BASE + 0x0DC))
#define REG_DMA3CNT_H (*(vu16 *)(REG_BASE + 0x0DE))

// Memory control
#define REG_WAITCNT (*(vu16 *)(REG_BASE + 0x204))

// BIOS helpers — forward decls (implemented via SWI) are in bios.h

// IWRAM/EWRAM base addresses (mirrored bus addresses)
#define IWRAM_BASE 0x03000000
#define EWRAM_BASE 0x02000000
#define VRAM_BASE  0x06000000
#define PAL_BASE   0x05000000
#define OAM_BASE   0x07000000

#endif // GBA_REGS_H
