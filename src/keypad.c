#include "gtadv/keypad.h"
#include "gba/regs.h"

// Reference: asm/keypad.s 0x08002430–0x080024AC
// Behavioral equivalence: verified by structure-level matching and
// mGBA RAM-diff harness (KEYPAD_STATE at 0x030035C0).
//
// Original Thumb:
//   _08002430: poll KEYINPUT (~REG_KEYINPUT), update cur/edge/repeat,
//             countdown and frameCounter.
//   _08002488/_08002494/_080024A0: trivial getters.

void KeypadPoll(void) {
    // Original asm: ldrh + mvns + lsls/lsrs #16 keeps the FULL 16-bit complement
    // (~KEYINPUT). Idle (KEYINPUT=0x03FF) therefore stores cur=0xFC00 — the top
    // six bits are always-set garbage, and consumers test low bits only. Must NOT
    // mask to 0x03FF: the state block is byte-compared by the RAM harness.
    u16 keys = (u16)~REG_KEYINPUT;

    volatile KeypadState *st = KEYPAD_STATE;
    u16 prevHeld = st->cur;
    u16 edge = (u16)(keys & ~prevHeld);

    // Store edge and current
    st->edge = edge;
    st->cur = keys;
    st->repeat = 0; // cleared, conditionally re-set below

    // Frame counter increments every poll (u16 wrap)
    st->frameCounter++;

    u16 lastHeld = st->lastHeld;
    if (keys != lastHeld) {
        // Key state changed: latch new held value, set repeat = current,
        // reload countdown to 24 (initial repeat delay)
        st->lastHeld = keys;
        st->repeat = keys;
        st->countdown = 24;
    } else {
        // Held unchanged: tick countdown
        u16 cd = st->countdown;
        if (cd != 0) {
            cd--;
            st->countdown = cd;
        } else {
            // Countdown expired: repeat fires every other frame (bit0 of frameCounter)
            if ((st->frameCounter & 1) == 0) {
                // No repeat on even frames
            } else {
                st->repeat = lastHeld;
            }
        }
    }
}

u16 KeypadGetEdge(void) {
    return KEYPAD_STATE->edge;
}

u16 KeypadGetHeld(void) {
    return KEYPAD_STATE->cur;
}

u16 KeypadGetRepeat(void) {
    return KEYPAD_STATE->repeat;
}

// Original asm symbols — alias so existing BLs (e.g. from agbmain/idle)
// resolve to the C implementation once linked.
#ifndef __APPLE__
void _08002430(void) __attribute__((alias("KeypadPoll")));
u16 _08002488(void) __attribute__((alias("KeypadGetEdge")));
u16 _08002494(void) __attribute__((alias("KeypadGetHeld")));
u16 _080024A0(void) __attribute__((alias("KeypadGetRepeat")));
#endif

// Friendly-name wrappers used by lifted C callers (foundation_agbmain.c).
// Values are full 16-bit complements of KEYINPUT (idle = 0xFC00); callers test
// individual bits (KEY_*). u16 vs int return is ABI-compatible (r0).
int KeyEdge(void) { return (int)KeypadGetEdge(); }
int KeyHeld(void) { return (int)KeypadGetHeld(); }
