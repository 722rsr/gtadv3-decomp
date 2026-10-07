#ifndef GTADV_MEMORY_H
#define GTADV_MEMORY_H

#include "gba/types.h"

// Verified memory layout anchors (from cheat DB + trace).
// Addresses are absolute IWRAM/EWRAM bus addresses.

// Keypad state at IWRAM 0x030035C0 (keypad.s)
#define KEYPAD_STATE_ADDR 0x030035C0

// State block A at IWRAM 0x03000008; pointer slot 0x030000E4
#define STATE_BLOCK_A_ADDR      0x03000008
#define STATE_BLOCK_A_SLOT_ADDR 0x030000E4

// State block B at IWRAM 0x030000F4 slot
#define STATE_BLOCK_B_SLOT_ADDR 0x030000F4

// Subsystem manager / instance array
#define SUBSYS_MGR_ADDR   0x03000198
#define SUBSYS_INST_ARRAY 0x030003E8
#define SUBSYS_INST_COUNT 45
#define SUBSYS_INST_SIZE  8

// Save / EEPROM map
#define EEPROM_STATE_ADDR 0x03000260
#define SAVE_SLOT_TABLE   0x0300032C
#define SAVE_BLOCK1_IDX   0x03000620
#define SAVE_BLOCK1_BUF   0x03000628
#define SAVE_BLOCK2_IDX   0x030015CC
#define SAVE_BLOCK2_BUF   0x030013D0
#define GARAGE_BITFIELDS  0x03001780

// Global frame / phase
#define FRAME_COUNTER_ADDR 0x03002858
#define GLOBAL_PHASE_ADDR  0x0300273C

// Race / ghost
#define RACE_CTX_PTR_ADDR 0x03004E20
#define GHOST_MGR_ADDR    0x03000610

// Stack tops (from crt0)
#define IRQ_STACK_TOP    0x03007FA0
#define SYSTEM_STACK_TOP 0x03007F00
#define IRQ_HANDLER_PTR  0x03007FFC
#define INTR_WAIT_FLAG   0x03007FF8

#endif // GTADV_MEMORY_H
