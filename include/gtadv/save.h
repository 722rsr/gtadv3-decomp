#ifndef GTADV_SAVE_H
#define GTADV_SAVE_H

#include "gba/types.h"

// Save / EEPROM lane — covers asm/save_*.s + asm/saveblock.s
// VMA spans and contracts documented in asm/saveblock.s.
// Preserves field widths, checksum seed, and slot-descriptor invariants.

#define SAVE_CHECKSUM_SEED 0x4E4D4D47u

// IWRAM anchors (see include/gtadv/memory.h)
#define SAVE_CONTROL_ADDR   0x03000320u  // {u32 count; u32 devType; u32 cursor; SaveSlot entries[?]}
#define SAVE_SLOT_TABLE_ADDR 0x0300032Cu // alias SAVE_SLOT_TABLE
#define SAVE_SLOT_ENTRY_SIZE 8u          // {u32 first; u32 size}

#define SAVE_BLOCK1_IDX_ADDR 0x03000620u
#define SAVE_BLOCK1_BUF_ADDR 0x03000628u
#define SAVE_BLOCK2_IDX_ADDR 0x030015CCu
#define SAVE_BLOCK2_BUF_ADDR 0x030013D0u
#define SAVE_BLOCK2_PERSISTED 444u       // meaningful prefix inside 508-B RAM buf
#define SAVE_BLOCK2_RAM_SIZE  508u

#define GARAGE_BASE_ADDR     0x03001780u
#define CAR_RECORD_BASE      0x03001D64u
#define CAR_RECORD_STRIDE    72u
#define CAR_RECORD_COUNT     35u

#define BUMP_ALLOC_ADDR      0x030002D8u
#define TIMER_QUEUE_ADDR     0x030003B0u
#define TIMER_QUEUE_PTR_ADDR 0x030003D4u

// On-media slot table entry
typedef struct {
    u32 firstSector; // byte offset cursor at push time (in bytes, sector = 8B units)
    u32 byteSize;
} SaveSlot;

// Control header at 0x03000320
typedef struct {
    u32 count;
    u32 devType;   // 1..2 = EEPROM present
    u32 cursor;    // next free byte offset
    SaveSlot slots[0];
} SaveControl;

#define SAVE_CONTROL ((volatile SaveControl *)SAVE_CONTROL_ADDR)
#define SAVE_SLOTS   ((volatile SaveSlot  *)SAVE_SLOT_TABLE_ADDR)

// Block #1 layout (3496 B: 16 B bitfields + 3480 B grid)
#define SAVE_BLOCK1_BITFIELD_BYTES 16u
#define SAVE_BLOCK1_GRID_ROWS 58u
#define SAVE_BLOCK1_GRID_COLS 5u
#define SAVE_BLOCK1_GRID_CELL 12u

// C API — behavioral equivalents, aliases preserve original link targets.
// The trailing VMA on each line is the CLOSURE's spelling, not decoration: a
// promoted body is spliced into asm/ as text, so src/save.c calls these
// through the VMA macros at the top of that file (`SAVE_CALL_*`) rather than
// through the friendly names, which no closure file defines. Do not "simplify"
// a `SAVE_CALL_*` back to a direct call — the screen refuses the body
// (tools/promotion_screen.py rule 2) and the spliced `bl` has no definition.

// Bump allocator (asm/save_alloc.s + save_alloc_more.s)
void *SaveAlloc(u32 size);                 // _0800572C
int  SaveAllocFree(u32 size);              // _08005758 (cursor subtraction, returns new cursor)
void SaveAllocWriteByte(u32 off, u32 v);    // _08005768 (word-width `v`: the ROM has no narrowing prologue)
u8   SaveAllocReadByte(u32 off);           // _08005780
int  SaveAllocFreeHigh(u32 size);          // _08005790 (returns new cursor)
void SaveAllocInit(u32 a, u32 b);          // _080057A4 (resource descriptor builder)

// Slot descriptor appender (asm/save_desc.s)
u32 SaveDescAppend(u32 byteSize);          // _0800580C — returns pre-append low byte of count

// Checksum + sector I/O wrappers (asm/save_checksum.s)
u32 SaveChecksum(const void *buf, u32 byteSize); // _08005860 seed 0x4E4D4D47
int SaveReadSectors(u32 firstSector, void *dst, u32 byteSize);  // _08005884
int SaveWriteSectors(u32 firstSector, const void *src, u32 byteSize); // _080058D0 ≤10 retries

// Slot API (asm/save_slot_api.s)
int SaveSlotSave(u32 slotIdx, const void *src); // _080059F0 / sub_080059F0
int SaveSlotLoad(u32 slotIdx, void *dst);       // _08005988 (referenced by saveblock wrappers)

// Timer queue (asm/save_timer_queue.s + save_delay.s)
void SaveTimerQueueInit(void);              // _08005A58
void SaveTimerQueueZero(void);              // _08005A68 (CpuSet zero helper wrapper)
bool SaveTimerQueueIsZero(void);            // _08005A8C predicate (count==0)
void SaveTimerQueueArm(u16 time, u8 id);    // _08005AA4 (slot search + count++)
void SaveTimerQueueDrain(void);             // _08005AE4 (decrement, compact, event 11)

// Trigger accessors (asm/save_trigger_accessors.s)
s16 SaveTriggerA(u32 arg); // _08005F2C  base 0x0805CAF0 mask 0x0FFE
s16 SaveTriggerB(u32 arg); // _08005F44  base 0x0805BAF0
s16 SaveTriggerC(u32 arg); // _08005F5C  base 0x0805CAF0
s16 SaveTriggerD(u32 arg); // _08005F74  base 0x0805BAF0

// Save-block #1 bitfield + grid (asm/saveblock.s)
void SaveBlock1SetBit(u32 bit, int set); // _0802466C
u32  SaveBlock1TestBit(u32 bit);         // _080246BC
u16  SaveBlock1GetHWord(void);           // _080246DC @+0x0C
void SaveBlock1SetHWord(u16 v);          // _080246E8
void *SaveBlock1GridPtr(u32 row, u32 col); // _080246F4 = 0x03000638 + row*60 + col*12
void SaveBlock1Init(void);               // _08024568 (full clear + slot pick + load + seed + audit)
int  SaveBlock1Load(void);               // _08024634
void SaveBlock1Save(void);               // _08024650 — the ROM's `pop {r0}` destroys r0

// Save-block #2 pack/unpack and guarded ops
u32  SaveGarageMaskTest(u32 idx, u32 k);       // _0802476C
void SaveGarageMaskSet(u32 val, u32 idx, u32 k); // _08024740 (val==1 → OR mask k into packed[idx])
void SaveGaragePack(void);                // _08024794
void SaveGarageUnpack(void);              // _0802488C
void SaveCarRecordsPack(void);            // _0802494C 35×8
void SaveCarRecordsUnpack(void);          // _080249AC asymmetric (b0 sign-extend halfword, +11=0)
void SaveBlock2PreSave(void);             // _08024A18
void SaveBlock2PostLoad(void);            // _08024A98
int  SaveBlock2Load(void);                // _08024B24
// The four guarded/block2 SAVE entries below are `void` because the ROM
// epilogue is `pop {r0}; bx r0` — r0 is NOT preserved across the call, so no
// caller can be reading a return value. Declaring them `int` keeps r0 live
// into the epilogue and agbcc then emits `pop {r1}; bx r1` instead, which is
// a 2-byte miss in every one of the four bodies. src/rec35_stage.c:8-9 already
// declared the two friendly wrappers `extern void`; this block is the odd
// one out, and these four lines bring it into agreement.
void SaveBlock2Save(void);                // _08024B54
void SaveGuardedFull(void);               // _08024B70 zero→reload→hook→rebuild→write→reload
void SaveGuardedSaveOnly(void);           // _08024BC0
void SaveGuardedLoadOnly(void);           // _08024BD8
void SaveGhostBulkSave(void);             // _0802471C slot0 dual save (ghost staging)
void SaveHook_0802446C(void);             // sub_0802446C: session-progress reset + ghost snapshot + zone rebuild
void SaveUpdate_080241C8(void);
void SaveRebuild_08024338(void);

#endif // GTADV_SAVE_H
