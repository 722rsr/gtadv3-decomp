#ifndef GTADV_KEYPAD_H
#define GTADV_KEYPAD_H

#include "gba/types.h"

// Layout of IWRAM 0x030035C0 — see asm/keypad.s and asm/agbmain.s
typedef struct {
    u16 cur;         // +0x00 current held keys (~KEYINPUT)
    u16 edge;        // +0x02 newly pressed this frame (cur & ~prev)
    u16 repeat;      // +0x04 repeat keys (fires while held, see below)
    u16 countdown;   // +0x06 hold countdown (reloaded to 24 on change)
    u16 lastHeld;    // +0x08 last held value (for change detection)
    u16 frameCounter;// +0x0A increments per poll
} KeypadState;

#define KEYPAD_STATE ((volatile KeypadState *)0x030035C0)

// GBA key bits (as read from ~KEYINPUT, i.e. 1 = pressed)
#define KEY_A      (1 << 0)
#define KEY_B      (1 << 1)
#define KEY_SELECT (1 << 2)
#define KEY_START  (1 << 3)
#define KEY_RIGHT  (1 << 4)
#define KEY_LEFT   (1 << 5)
#define KEY_UP     (1 << 6)
#define KEY_DOWN   (1 << 7)
#define KEY_R      (1 << 8)
#define KEY_L      (1 << 9)

// C API — behavioral equivalent of asm/keypad.s
void KeypadPoll(void);      // _08002430
u16  KeypadGetEdge(void);   // _08002488
u16  KeypadGetHeld(void);   // _08002494
u16  KeypadGetRepeat(void); // _080024A0

#endif // GTADV_KEYPAD_H
