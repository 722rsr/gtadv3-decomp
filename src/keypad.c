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
    u32 keys = (u16)~REG_KEYINPUT;

    // The state block is ordinary IWRAM. The ROM does not perform volatile
    // readbacks after its stores; only the KEYINPUT hardware register is
    // volatile.
    register KeypadState *st __asm__("r3") =
        (KeypadState *)(uintptr_t)0x030035C0;
    u32 prevHeld = st->cur;
    u32 edge = keys & ~prevHeld;

    // Store edge and current
    st->edge = edge;
    st->cur = keys;
    st->repeat = 0; // cleared, conditionally re-set below

    // Frame counter increments every poll (u16 wrap)
    register u32 frame __asm__("r1") = (u32)st->frameCounter + 1;
    __asm__("" : "+r"(frame));
    *(volatile u16 *)((u8 *)st + 0x0A) = frame;

    register u16 lastHeld __asm__("r4") = st->lastHeld;
    if (keys == lastHeld) {
        // Held unchanged: tick countdown
        int cd = st->countdown;
        if (cd == 0) {
            // Countdown expired: repeat fires every other frame (bit0 of frameCounter)
            frame &= 1;
            if (frame != 0)
                st->repeat = lastHeld;
        } else {
            cd--;
            st->countdown = cd;
        }
    } else {
        // Key state changed: latch new held value, set repeat = current,
        // reload countdown to 24 (initial repeat delay)
        st->lastHeld = keys;
        st->repeat = keys;
        st->countdown = 24;
    }
}
#ifndef __APPLE__
// The ROM's function section ends with a zero halfword after `bx lr`.
__asm__(".pushsection .text.KeypadPoll,\"ax\",%progbits\n.align 2, 0\n.popsection");
#endif

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
