#include "gtadv/sound.h"
#include "gba/bios.h"

// Sound driver — honest C lift, leaves first.
// Only pure-Thumb substantiated leaves are aliased here; mixer regions live
// in src/sound_mixer.c (blocked interiors noted there).
// VMA refs per asm/* + asm/sound_api.s.

// Weak externs for low-level helpers not yet reconstructed (allow standalone host builds)
__attribute__((weak)) void sub_0802C4C4(void) {}
__attribute__((weak)) void sub_0802C5C0(void) {}
// Declarations for helpers that have real bodies below (ARM) or weak for host
extern void sub_0802CB20(void *a);
extern void sub_0802CB84(void *a);
extern void sub_0802C614(void);
#ifdef __APPLE__
__attribute__((weak)) void sub_0802CB20(void *a) { (void)a; }
__attribute__((weak)) void sub_0802CB84(void *a) { (void)a; }
__attribute__((weak)) void sub_0802C614(void) {}
#endif
__attribute__((weak)) void sub_0802B64C(u32 v) { (void)v; }
__attribute__((weak)) void sub_0802B65C(u32 v) { (void)v; }
__attribute__((weak)) void sub_0802B718(u32 a, u32 b, u32 c) { (void)a;(void)b;(void)c; }
__attribute__((weak)) void sub_0802B74C(u32 a, u32 b, u32 c) { (void)a;(void)b;(void)c; }
__attribute__((weak)) void sub_0802BE78(void) {}
__attribute__((weak)) void sub_0802C53C(void) {}
__attribute__((weak)) void sub_0802C4A4_w(void *s, u16 v) { (void)s;(void)v; }

// Helpers to access master via pointer cell 0x03001764
// The *cell* is volatile -- other code rewrites that live pointer.  The struct
// it yields is NOT: agbcc re-reads a volatile member lvalue to recompute its
// address, emitting a second dead `ldrb`, and the ROM has exactly one byte load
// per read-modify-write.  The cell being volatile is what makes each call site
// below re-read the pointer, which is also what the ROM does across the call.
static inline SoundMaster *soundMaster(void){
    volatile u32 *cell = (volatile u32 *)SOUND_MASTER_PTR;
    u32 p = *cell;
    return (SoundMaster *)(uintptr_t)p;
}

// _0802B04C(state) — driver init, publish ptr, enable, levels |=0xFF
void SoundInit(void *state){
    SoundMaster *m = (SoundMaster *)state;
    register u32 v __asm__("r0");
    sub_0802C4C4();
    *(volatile u32 *)SOUND_MASTER_PTR = (u32)(uintptr_t)m;
    m->flags = 1;
    v = 255;
    m->masterVol |= v;
    m->chanCLevel |= v;
    m->chanDLevel |= v;
}
#ifndef __APPLE__
void _0802B04C(void *s) __attribute__((alias("SoundInit")));
#ifndef __APPLE__
void sub_0802B04C(void *s) __attribute__((alias("SoundInit")));
#endif
#endif

// _0802B07C — VBlank pump when enabled
void SoundVBlank(void){
    volatile SoundMaster *m = soundMaster();
    // Named mask, not a bare literal: agbcc then materialises the 1 into r0
    // *before* the flags load instead of sinking it past it, which is the
    // register split the ROM takes (ldr r1,[r0] / movs r0,#1 / ldrb r1,[r1,#8]).
    u32 mask = 1;
    if ((m->flags & mask)==0) return;
    sub_0802BE78();
}
#ifndef __APPLE__
void _0802B07C(void) __attribute__((alias("SoundVBlank")));
#endif
#ifndef __APPLE__
void sub_0802B07C(void) __attribute__((alias("SoundVBlank")));
#endif

// _0802B098 — VCounter sequencer tick: service, fade engine, re-apply vol + ch C/D
void SoundVCounter(void){
    volatile SoundMaster *m = soundMaster();
    if ((m->flags & 1)==0) return;
    sub_0802C53C();
    m->volChanged = 0;
    u8 flags = m->flags;
    if (flags & 4){
        if ((flags & 32)==0){
            s16 cur = (s16)m->fadeCur;
            u8 step = m->fadeStep;
            s16 next = (s16)(cur - (s16)step);
            m->fadeCur = (u16)next;
            s16 target = (s16)m->fadeTarget;
            if ((s32)next <= (s32)target){
                m->fadeCur = (u16)target;
                m->flags |= 0x10;
                m->flags &= (u8)~4;
                m->flags &= (u8)0xFB; // actually 0xFB? keeps enable etc — preserve width
                if (m->flags2 & 64) sub_0802B65C(m->masterVol); // wait this is StopMute? simplified
                m->flags2 &= 0xBF;
            } else {
                sub_0802B718(m->masterVol, (u32)(s16)m->fadeCur, 255);
            }
        }
    } else if (flags & 8){
        if ((flags & 32)==0){
            s16 cur = (s16)m->fadeCur;
            u8 step = m->fadeStep;
            s16 next = (s16)(cur + (s16)step);
            m->fadeCur = (u16)next;
            s16 target = (s16)m->fadeTarget;
            if ((s32)next >= (s32)target){
                m->fadeCur = (u16)target;
                m->flags |= 0x10;
                m->flags &= (u8)~8;
                m->flags &= 0xF7;
            } else {
                sub_0802B718(m->masterVol, (u32)(s16)m->fadeCur, 255);
            }
        }
    }
    if (m->chanCLevel != 0xFF){
        sub_0802B718(m->chanCLevel, (u32)m->chanC, 255);
        sub_0802B74C(m->chanCLevel, (u32)m->chanD, 255);
    }
}
#ifndef __APPLE__
void _0802B098(void) __attribute__((alias("SoundVCounter")));
#endif
#ifndef __APPLE__
void sub_0802B098(void) __attribute__((alias("SoundVCounter")));
#endif

// Friendly-name wrappers used by foundation_boot.c VBlank/VCounter handlers.
void SoundTick(void) { SoundVBlank(); }
void SoundSeqTick(void) { SoundVCounter(); }

// : exact at 40/40.  Exactly one construct is load-bearing -- the mask's
// register pin -- and the rest of the body is the honest lift:
//   * `mask` is pinned to r0. This is what makes the ROM's register split come
//     out: base r1 / mask r0 / byte temp r2, with the `mov r0,#1` landing
//     BEFORE the flags load. Unpinned, agbcc allocates the mask to r2 in both
//     blocks and the body is 44 bytes (control: 11/40, first difference +0x2).
//   * The base gets NO pin, and that absence is load-bearing rather than
//     incidental. Left to agbcc it lands in r1 on its own, which is what leaves
//     r4 free to hold the 0x03001764 CELL address across the `bl` and produces
//     the ROM's `ldr r4, =0x03001764 ... ldr r1, [r4]` reload on each side of
//     the call. Forcing it is not free: pin the base to r0 and it steals r0
//     from the mask (control: 34/40, first difference +0x4). A pin to r1 is
//     byte-identical to the plain local, so per the group note above it is
//     deliberately absent rather than left in as decoration.
//   * The base is a separate `soundMaster()` call on each side, so the cell is
//     re-read across the call exactly as the ROM reloads it, and it is NOT
//     volatile: a volatile member lvalue costs a dead second `ldrb`
//     (controls on this body: drop the reload and the ROM's post-call
//     `ldr r1, [r4]` is gone -- 22/40, first difference +0x3; make the base
//     volatile and the dead reloads return -- 26/40, candidate 44).
// The accumulate form `mask &= m->flags; m->flags = mask` is NOT load-bearing
// here -- the inline `m->flags &= 0xFE` is byte-identical, unlike on _0802B234
// and _0802B30C where it is. It is kept because it is the form the group note
// above describes for the flag-RMW twins, not because dropping it costs bytes.
// The arg to sub_0802CB20 is the leftover `mask`. The ROM sets up NO argument
// before the `bl`, and it cannot: the callee is `SoundCmdCommit`, whose state
// parameter is `(void)state; // dead in ROM`. So r0 simply carries the guard
// result into the call. Same idiom as sound_core.c:128, and note it is the
// mask PIN that makes the forwarding free -- a literal `0` would emit a
// `mov r0, #0` the ROM does not have.
void SoundOff(void){
    SoundMaster *m;
    register u32 mask __asm__("r0");
    m = soundMaster();
    mask = 1;
    mask &= m->flags;
    if (mask == 0) return;
    sub_0802CB20((void *)(uintptr_t)mask);
    m = soundMaster();
    mask = 0xFE;
    mask &= m->flags;
    m->flags = (u8)mask;
}
#ifndef __APPLE__
void _0802B190(void) __attribute__((alias("SoundOff")));
#endif
#ifndef __APPLE__
void sub_0802B190(void) __attribute__((alias("SoundOff")));
#endif
// : exact at 44/44.  The guard block is _0802B190's verbatim, mask pin and all,
// and the RMW block must NOT reuse the guard's variables. That is the one thing
// load-bearing here, and it is a shape rather than a pin:
//   * the ROM's second block re-loads the master into r0 and holds the mask in
//     r1 -- the MIRROR of the guard block (base r1 / mask r0). Reusing the
//     guard's `m`/`mask` forces that mirrored split onto the guard instead, so
//     the RMW block lands 39/44 with its registers swapped (first difference
//     +0x18). Giving the RMW block its own local lets agbcc allocate the two
//     blocks independently, which is what reproduces the mirror. (control:
//     reuse -> 39/44.)
// The RMW block's plain form is enough and is deliberately left unpinned: the
// only thing that matters is that it does NOT reuse the guard's `m`/`mask`,
// and once it does not, agbcc allocates the mirror on its own. Pinning it
// (`b`->r0 / `k`->r1) is byte-identical, so per the group note above it is
// absent rather than decoration. What IS load-bearing is the guard's mask pin;
// unpin the whole guard and the body spills to 48 bytes (control 20/44, first
// difference +0x2).
// The arg to sub_0802CB84 is the leftover guard `mask`, exactly as on
// _0802B190: the ROM sets up no argument before either `bl`, and this callee
// clobbers r0 on entry (`asm/sound_stop.s` opens with `ldr r0, _0802CBB0`).
void SoundOn(void){
    SoundMaster *m;
    register u32 mask __asm__("r0");
    m = soundMaster();
    mask = 1;
    mask &= m->flags;
    if (mask != 0) return;
    sub_0802C4C4();
    sub_0802CB84((void *)(uintptr_t)mask);
    {
        SoundMaster *b = soundMaster();
        b->flags |= 1;
    }
}
#ifndef __APPLE__
void _0802B1B8(void) __attribute__((alias("SoundOn")));
#endif
#ifndef __APPLE__
void sub_0802B1B8(void) __attribute__((alias("SoundOn")));
#endif

// The one escape found: `volatile SoundMaster *m` makes _0802B30C emit the
// ROM's EXACT three registers (base r1 / mask r0 / temp r2) — but the
// volatile member lvalue then costs a second `ldrb r3,[r1,#14]`, 38 bytes
// against the ROM's 36.  Splitting the compound assignment into an explicit
// `u8 f` temp does NOT remove it, and neither does routing the RMW through a
// non-volatile `u8 *`/`SoundMaster *` alias of `m` (which loses the good
// allocation again).  So on this body the register split and the byte count
// are jointly unreachable from C: 32/36 with correct registers costs 2 bytes
// too many, and 36 bytes with the correct count has the wrong registers.
// ----------------------------------------------------------------------------

// _0802B1E4 / _0802B214 / _0802B234 / _0802B25C / _0802B280
// : exact at 48/48. Three shapes are load-bearing, and
// only one of them is a register pin:
//   * `m` is a GCC LOCAL REGISTER VARIABLE pinned to r1. The ROM keeps the
//     master base in r1 across the three stores and the AND; agbcc otherwise
//     splits it across r1/r2 (control: drop this pin -> 44/48, prefix 26).
//   * the `volChanged` store SHARES r1 with the `fadeCur` store, so `m` is
//     assigned once for the pair rather than re-read per access.
//   * the callee argument is re-read from the master byte (the ROM's
//     `ldr r0,[r4,#0] / ldrb r0,[r0,#9]` before `bl sub_0802B64C`), not the
//     `vol` parameter -- worth 15 -> 23 bytes.
// The `mask op= m->flags; m->flags = mask` accumulate form is also
// load-bearing (control: `m->flags &= 0xFB` -> 44/48, prefix 28), but a
// `register u32 mask __asm__("r0")` pin is NOT: it is byte-identical to the
// plain `u32 mask` below, so it is deliberately absent.  See the group note
// above on why one `__asm__` register declaration per block.
void SoundSetMasterVol(u32 vol){
    register SoundMaster *m __asm__("r1");
    u32 mask;
    soundMaster()->masterVol = (u8)vol;
    m = soundMaster();
    m->fadeCur = 0xFF;
    m->volChanged = 1;
    sub_0802B64C(soundMaster()->masterVol);
    m = soundMaster();
    mask = 0xFB;
    mask &= m->flags;
    m->flags = (u8)mask;
}
#ifndef __APPLE__
void _0802B1E4(u32 v) __attribute__((alias("SoundSetMasterVol")));
#endif
#ifndef __APPLE__
void sub_0802B1E4(u32 v) __attribute__((alias("SoundSetMasterVol")));
#endif
void SoundSetMasterVolCond(u16 vol){
    SoundMaster *m = soundMaster();
    if (m->masterVol == vol) return;
#ifndef __APPLE__
    sub_0802B1E4(vol);
#else
    SoundSetMasterVol(vol);
#endif
}
#ifndef __APPLE__
void _0802B214(u16 v) __attribute__((alias("SoundSetMasterVolCond")));
void Sub_0802B214(u16 v) __attribute__((alias("SoundSetMasterVolCond")));
#endif
#ifndef __APPLE__
void sub_0802B214(u16 v) __attribute__((alias("SoundSetMasterVolCond")));
#endif
// Each master access re-invokes soundMaster so the cell is re-read, matching
// the ROM's `ldr rN,[r4]` after every call.  `mask` is a SEPARATE assignment
// after the master load: an inline literal makes agbcc sink the `movs` past
// the byte load and allocate the mask register to the base pointer instead.
//
// : `m` is a GCC LOCAL REGISTER VARIABLE pinned to r1.
// The four RMW twins here all need the same hard-register split --
// base = r1, mask = r0, byte temp = r2 -- and ~50 source spellings could not
// reach it, because the choice is made in agbcc's `local_alloc`/reload from
// live-range and reference-count order rather than from anything visible in
// the C.  `register T v __asm__("rN")` is the one construct that addresses
// that decision directly, and it is what makes _0802B234 and _0802B30C
// byte-exact.  The accumulate form (`mask op= m->flags; m->flags = mask;`)
// is equally load-bearing: with `m->flags op= mask` the OR/AND result takes a
// NEW register (r2) instead of staying in the pinned r0, which is the last
// two bytes of _0802B234 and _0802B30C.  See the group note above.
void SoundStopMute(void){
    register SoundMaster *m __asm__("r1");
    register u32 mask __asm__("r0");
    sub_0802B65C(soundMaster()->masterVol);
    soundMaster()->masterVol = 0xFF;
    m = soundMaster();
    mask = 0xD3;
    mask &= m->flags;
    m->flags = (u8)mask;
}
#ifndef __APPLE__
void _0802B234(void) __attribute__((alias("SoundStopMute")));
#endif
#ifndef __APPLE__
void sub_0802B234(void) __attribute__((alias("SoundStopMute")));
#endif
// : exact at 36/36, and this is the body that shows the
// `register... __asm__` pin reaching a base pointer that agbcc's own
// allocator will not put in r1.  The ROM loads the master cell into r1 for
// BOTH the `masterVol` guard and the post-call `flags` RMW; agbcc gave the
// first load r0 and the second r1, i.e. the same 35/36 body with the registers
// swapped between two blocks.  Pinning the *cell word* (not a `SoundMaster *`)
// in r1 is what forces it: the control is sharp in both directions --
// drop the pin -> 30/36 prefix 4; pin it to r0 instead -> 29/36 prefix 4.
// The accumulate form IS load-bearing here, as on _0802B234: the inline
// `|= 32` scores 32/36 prefix 18, while `mask |= cell; cell = mask` is EXACT.
// See the group note above on why one `__asm__` register declaration here.
void SoundPause(void){
    register u32 p __asm__("r1");
    u8 mask;
    p = *(volatile u32 *)SOUND_MASTER_PTR;
    if (*(volatile u8 *)(p + 9) == 0xFF) return;
    sub_0802C614();
    p = *(volatile u32 *)SOUND_MASTER_PTR;
    mask = 32;
    mask |= *(volatile u8 *)(p + 8);
    *(volatile u8 *)(p + 8) = mask;
}
#ifndef __APPLE__
void _0802B25C(void) __attribute__((alias("SoundPause")));
#endif
#ifndef __APPLE__
void sub_0802B25C(void) __attribute__((alias("SoundPause")));
#endif
// : exact at 36/36. Same two-block register split as
// _0802B25C above, and the same fix: the cell word is pinned to r1 so the
// guard's `ldr r1,[r4,#0]` and the post-call `ldr r1,[r4,#0]` agree, and the
// `movs r0,#0xDF / ldrb r2,[r1,#8] / ands r0,r2` mask lands in r0/r2.
// The accumulate form is kept (see _0802B234: it is load-bearing there).
void SoundResume(void){
    register u32 p __asm__("r1");
    u32 mask;
    p = *(volatile u32 *)SOUND_MASTER_PTR;
    if (*(volatile u8 *)(p + 9) == 0xFF) return;
    sub_0802C5C0();
    p = *(volatile u32 *)SOUND_MASTER_PTR;
    mask = 0xDF;
    mask &= *(volatile u8 *)(p + 8);
    *(volatile u8 *)(p + 8) = (u8)mask;
}
#ifndef __APPLE__
void _0802B280(void) __attribute__((alias("SoundResume")));
#endif
#ifndef __APPLE__
void sub_0802B280(void) __attribute__((alias("SoundResume")));
#endif

// _0802B2A4 arm fade, _0802B30C fadeOut, _0802B330 remaining
void SoundArmFade(u16 target, u8 step){
    volatile SoundMaster *m = soundMaster();
    m->flags2 &= 0xBF;
    if (m->masterVol == 0xFF) return;
    s16 cur = (s16)m->fadeCur;
    if ((s32)cur >= (s32)(s16)target){
        m->fadeTarget = target; m->fadeStep = step; m->flags |= 4;
    } else {
        m->fadeTarget = target; m->fadeStep = step; m->flags |= 8;
    }
    m->flags &= 0xEF;
}
#ifndef __APPLE__
void _0802B2A4(u16 t, u8 s) __attribute__((alias("SoundArmFade")));
#endif
#ifndef __APPLE__
void sub_0802B2A4(u16 t, u8 s) __attribute__((alias("SoundArmFade")));
#endif
void SoundFadeOut(u8 step){
    register SoundMaster *m __asm__("r1");
    register u8 mask __asm__("r0");
#ifndef __APPLE__
    sub_0802B2A4(0, step);
#else
    SoundArmFade(0, step);
#endif
    m = soundMaster();
    mask = 64;
    mask |= m->flags2;
    m->flags2 = (u8)mask;
}
#ifndef __APPLE__
void _0802B30C(u8 s) __attribute__((alias("SoundFadeOut")));
#endif
#ifndef __APPLE__
void sub_0802B30C(u8 s) __attribute__((alias("SoundFadeOut")));
#endif
s16 SoundFadeRemaining(void){
    volatile SoundMaster *m = soundMaster();
    if (m->masterVol==0xFF) return 0;
    u8 f = m->flags;
    if ((f & 8) || (f & 4)){
        s16 t = (s16)m->fadeTarget;
        s16 c = (s16)m->fadeCur;
        return (s16)(t - c);
    }
    return 0;
}
#ifndef __APPLE__
s16 _0802B330(void) __attribute__((alias("SoundFadeRemaining")));
#endif
#ifndef __APPLE__
s16 sub_0802B330(void) __attribute__((alias("SoundFadeRemaining")));
#endif

// _0802B368 deferred vol, _0802B384/_0802B3A4 sec vol
//
// : exact at 28/28. The base is NOT volatile here. With
// `volatile SoundMaster *m` agbcc re-reads the member lvalue to recompute its
// address and emits a dead `ldrb r1,[r2,#17]` at +0x0A, which the ROM does not
// have (it goes straight from `ldr r2,[r1,#0]` to `movs r1,#1` to `strb`).
// A plain `SoundMaster *` drops it: 15/28 -> 28/28. The base lands in r2
// either way, so no register pin is needed or justified on this body.
void _0802B368(u16 vol){
    SoundMaster *m = soundMaster();
    m->volChanged = 1;
    sub_0802B64C(vol);
}
#ifndef __APPLE__
void Sub_0802B368(u16 v) __attribute__((alias("_0802B368")));
#endif
#ifndef __APPLE__
void sub_0802B368(u16 v) __attribute__((alias("_0802B368")));
#endif
void SoundSetSecVol(u16 v){
    soundMaster()->secVol = (u8)v;
    soundMaster()->volChanged = 1;
    sub_0802B64C(v);
}
#ifndef __APPLE__
void _0802B384(u16 v) __attribute__((alias("SoundSetSecVol")));
#endif
#ifndef __APPLE__
void sub_0802B384(u16 v) __attribute__((alias("SoundSetSecVol")));
#endif
void SoundApplySecVol(void){
    volatile SoundMaster *m = soundMaster();
    sub_0802B65C(m->secVol);
}
#ifndef __APPLE__
void _0802B3A4(void) __attribute__((alias("SoundApplySecVol")));
#endif
#ifndef __APPLE__
void sub_0802B3A4(void) __attribute__((alias("SoundApplySecVol")));
#endif

// _0802B3B8 bank probe, _0802B418 speed
bool SoundIsBankPlaying(u32 bank){
    volatile SoundMaster *m = soundMaster();
    if ((m->flags & 1)==0) return false;
    u32 addr;
    if (bank==0) addr=SOUND_BANK0_ADDR;
    else if (bank==1) addr=SOUND_BANK1_ADDR;
    else if (bank==2) addr=SOUND_BANK2_ADDR;
    else addr=SOUND_BANK3_ADDR;
    u32 w = *(volatile u32 *)(addr+4);
    if (m->volChanged==1) return true;
    if ((w & 0xFFFF)==0) return false;
    if ((s32)w < 0) return false;
    return true;
}
#ifndef __APPLE__
bool _0802B3B8(u32 b) __attribute__((alias("SoundIsBankPlaying")));
bool Sub_0802B3B8(u32 b) __attribute__((alias("SoundIsBankPlaying")));
#endif
#ifndef __APPLE__
bool sub_0802B3B8(u32 b) __attribute__((alias("SoundIsBankPlaying")));
#endif
void SoundSetSongSpeed(u16 a, u16 b){
    SoundMaster *m = soundMaster();
    u32 off = (u32)a + (u32)b*98 + 0x080613B8u;
    u8 v = *(volatile u8 *)off;
    m->songSpeed = (u8)(v + 24);
    sub_0802B64C(m->songSpeed);
}
#ifndef __APPLE__
void _0802B418(u16 a, u16 b) __attribute__((alias("SoundSetSongSpeed")));
#endif
#ifndef __APPLE__
void sub_0802B418(u16 a, u16 b) __attribute__((alias("SoundSetSongSpeed")));
#endif
void SoundReapplySpeedA(void){ volatile SoundMaster *m=soundMaster(); sub_0802B65C(m->songSpeed); }
#ifndef __APPLE__
void _0802B44C(void) __attribute__((alias("SoundReapplySpeedA")));
#endif
#ifndef __APPLE__
void sub_0802B44C(void) __attribute__((alias("SoundReapplySpeedA")));
#endif
void SoundReapplySpeedB(void){ volatile SoundMaster *m=soundMaster(); sub_0802B65C(m->songSpeed); }
#ifndef __APPLE__
void _0802B460(void) __attribute__((alias("SoundReapplySpeedB")));
#endif
#ifndef __APPLE__
void sub_0802B460(void) __attribute__((alias("SoundReapplySpeedB")));
#endif
void SoundReapplySpeedC(void){ volatile SoundMaster *m=soundMaster(); sub_0802B64C(m->songSpeed); }
#ifndef __APPLE__
void _0802B474(void) __attribute__((alias("SoundReapplySpeedC")));
#endif
#ifndef __APPLE__
void sub_0802B474(void) __attribute__((alias("SoundReapplySpeedC")));
#endif


// sound_alloc.s CBBC, sound_cmd.s CB20, sound_control.s C990, sound_channel_cluster.s BC84 — blocked
// CBBC alloc sentinel 0x03007FF0 0x68736D53, CB20 dispatcher, C990 control, BC84 stream helper remain
// asm/sound_alloc.s etc. without alias here; exact versions in src/sound_extra.c / core already cover
// complete CFGs where proven (see asm/sound_api.s). No duplicate aliases emitted.
// Remaining pure-Thumb channel/allocation bodies — blocked, not claimed.
// BCB4/BCCC/BCE8/BCF4/BD14/BD30/BD44/BD74/BD80/BD94/BDA8/BDD8/BDEC/BE00/BE14/BE28/BE34/BE4C/BE60,
// sound_note_on C190, sound_start CC34, sound_stop CB84, sound_tick C738, sound_seq B66C
// remain asm/sound_channel_cluster.s etc. — BE32 0x080614E0, DMA 0x04000060, +64 cursor 0x40,
// and 0x50 stride voice layouts not proven beyond pools; left blocked without alias (see asm/sound_api.s).
// No placeholder aliases emitted here.
// sound_followon.s _0802C614 — blocked, exact in src/sound_deep.c as _0802C614 (SoundMoreHelper)
// No duplicate alias here.

// TODO: remaining mixed-ISA mixer_2b888.s body remains blocked (needs mode-boundary trace, not guessed).

// ROM entry alias.
#ifndef __APPLE__
bool Sound_0x0802B3B8(u32 bank) __attribute__((alias("SoundIsBankPlaying")));
bool Sound_Cmd3(u32 bank) __attribute__((alias("SoundIsBankPlaying")));
void Sub_08002B214(u16 vol) __attribute__((alias("SoundSetMasterVolCond")));
void Sub_08002B234(void) __attribute__((alias("SoundStopMute")));
void Sub_08002B368(u16 vol) __attribute__((alias("_0802B368")));
void Sub_08002B384(u16 v) __attribute__((alias("SoundSetSecVol")));
void Sub_08002B3A4(void) __attribute__((alias("SoundApplySecVol")));
void _08002B190(void) __attribute__((alias("SoundOff")));
void _08002B1B8(void) __attribute__((alias("SoundOn")));
void _08002B1E4(u32 vol) __attribute__((alias("SoundSetMasterVol")));
void _08002B214(u16 vol) __attribute__((alias("SoundSetMasterVolCond")));
void _08002B234(void) __attribute__((alias("SoundStopMute")));
void _08002B25C(void) __attribute__((alias("SoundPause")));
void _08002B280(void) __attribute__((alias("SoundResume")));
void _08002B30C(u8 step) __attribute__((alias("SoundFadeOut")));
void _08002B368(u16 vol) __attribute__((alias("_0802B368")));
void _08002B3A4(void) __attribute__((alias("SoundApplySecVol")));
void _08002B418(u16 a, u16 b) __attribute__((alias("SoundSetSongSpeed")));
void _08002B44C(void) __attribute__((alias("SoundReapplySpeedA")));
void _08002B460(void) __attribute__((alias("SoundReapplySpeedB")));
void _08002B474(void) __attribute__((alias("SoundReapplySpeedC")));
void sub_08002B368(u16 vol) __attribute__((alias("_0802B368")));
#endif

// ----------------------------------------------------------------------------
// Round-six transcription — sound-bank song load/claim + channel swap
// (asm/sound_bank.s; master block pointer cell 0x03001764, bank table
// 0x08061FA4, 8 bytes per bank entry: [0]=song-struct ptr, [4]=state gate,
// struct+2 = priority byte).
//
// 0x0802B500(idx, w4, w6) — SONG LOAD / CHANNEL CLAIM (3-arg, all halfwords):
//   gate = u16[0x08061FA4 + idx*8 + 4]; if (gate != 3) return;
//   m = *(0x03001764);
//   if (m->C == 0xFF) { if (m->D == 0xFF) { m->D = idx; commit; }
//     else { if (prio(D) < prio(new)) { m->D = idx; commit; } } return; }
//   if (prio(C) < prio(new)) return;            // C >= new ? drop
//   if (m->D != 0xFF && prio(D) >= prio(new)) return;
//   if (m->D == 0xFF) { m->D = idx; commit (from saved ip/r9); }
//   else { m->D = idx; commit; }
//   commit: u16[Dsong+4] = w4; u16[Dsong+6] = w6  (Dsong = bank[idx].ptr)
void SoundBankClaim_2B500(u32 idx, u32 w4, u32 w6) {
    volatile u8 *bankRow = (volatile u8 *)(0x08061FA4u + (u32)(s16)(u16)idx * 8u);
    if (*(volatile u16 *)(bankRow + 4) != 3) return;
    volatile u8 *m = *(volatile u8 *volatile *)0x03001764u;
    u8 newPrio = *(volatile u8 *)(*(volatile u32 *)bankRow + 2);
    u8 c = m[15], d = m[16];
    if (c == 0xFF) {
        if (d == 0xFF || *(volatile u8 *)(*(volatile u32 *)(0x08061FA4u + (u32)d * 8u) + 2) < newPrio) {
            m[16] = (u8)idx;
            volatile u8 *ds = *(volatile u8 *volatile *)(0x08061FA4u + (u32)idx * 8u);
            *(volatile u16 *)(ds + 4) = (u16)w4;
            *(volatile u16 *)(ds + 6) = (u16)w6;
        }
        return;
    }
    if (*(volatile u8 *)(*(volatile u32 *)(0x08061FA4u + (u32)c * 8u) + 2) < newPrio) return; // C >= new ? drop
    if (d != 0xFF && *(volatile u8 *)(*(volatile u32 *)(0x08061FA4u + (u32)d * 8u) + 2) >= newPrio) return;
    m[16] = (u8)idx;
    volatile u8 *ds = *(volatile u8 *volatile *)(0x08061FA4u + (u32)idx * 8u);
    *(volatile u16 *)(ds + 4) = (u16)w4;
    *(volatile u16 *)(ds + 6) = (u16)w6;
}
#ifndef __APPLE__
void _0802B500(u32 a, u32 b, u32 c) __attribute__((alias("SoundBankClaim_2B500")));
void sub_0802B500(u32 a, u32 b, u32 c) __attribute__((alias("SoundBankClaim_2B500")));
void _08002B500(u32 a, u32 b, u32 c) __attribute__((alias("SoundBankClaim_2B500")));
#endif

// 0x0802B5AC — CHANNEL SWAP / TEARDOWN: promote D into C (or stop C).
//   m = *(0x03001764); D = m[16];
//   if (D != 0xFF) {
//     if (prio(D) & 1) ch = D;  else ch = m[15];   // _0802B5E0 arm
//     if (ch != 0xFF) _0802B65C(ch);               // pause old C
//     m[15] = D; m[17] = 1;                        // promote + volume re-apply
//     _0802B64C(m[15]);                            // start new C
//   } else if (m[15] != 0xFF) {
//     if (prio(C) & 1) ch = C;
//     else { if (_0802B3B8(3)) return; ch = m[15]; }
//     _0802B65C(ch); m[15] = 0xFF;                 // pause + free C
//   }
//   m[16] = 0xFF;                                  // clear D owner
void SoundBankSwap_2B5AC(void) {
    volatile u8 *m = *(volatile u8 *volatile *)0x03001764u;
    extern void sub_0802B64C(u32 v);
    extern void sub_0802B65C(u32 v);
    extern bool SoundIsBankPlaying(u32 bank);
    u8 d = m[16];
    if (d != 0xFF) {
        u8 prioD = *(volatile u8 *)(*(volatile u32 *)(0x08061FA4u + (u32)d * 8u) + 2);
        u8 ch = (prioD & 1) ? d : m[15];
        if (ch != 0xFF) sub_0802B65C(ch);
        m[15] = d;
        m[17] = 1;
        sub_0802B64C(m[15]);
    } else if (m[15] != 0xFF) {
        u8 c = m[15];
        u8 prioC = *(volatile u8 *)(*(volatile u32 *)(0x08061FA4u + (u32)c * 8u) + 2);
        u8 ch;
        if (prioC & 1) ch = c;
        else { if (SoundIsBankPlaying(3)) { m[16] = 0xFF; return; } ch = c; }
        sub_0802B65C(ch);
        m[15] = 0xFF;
    }
    m[16] = 0xFF;
}
#ifndef __APPLE__
void _0802B5AC(void) __attribute__((alias("SoundBankSwap_2B5AC")));
void sub_0802B5AC(void) __attribute__((alias("SoundBankSwap_2B5AC")));
void Sub_0802B5AC(void) __attribute__((alias("SoundBankSwap_2B5AC")));
void _08002B5AC(void) __attribute__((alias("SoundBankSwap_2B5AC")));  // 9-digit corpus spelling
#endif
