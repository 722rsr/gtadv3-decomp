#include "gtadv/sound.h"
#include "gba/types.h"

// Bounded non-noop leaf: sub_0802BD30 byte cursor rewind, self-contained body using r1.
// VMA 0x0802BD30-0x0802BD42 (20 B) in asm/sound_channel_cluster.s:0x02BC84-0x02BE78.
// Pure-Thumb, no literal pool, complete boundary: no push / bx lr at 0x02BD42, self-contained via r1.
// Exact per objdump --adjust-vma=0x0802BD30: ldrb r2,[r1,#2] (u8), cmp #0 beq, subs #1 strb, lsls #2 (*4), adds r3,r1,r2, ldr r2,[r3,#68] (u32), str r2,[r1,#64] (u32), bx lr.
// No voice layout guess beyond +2 (u8 cursor idx) / +68 (u32 cursor ptr) / +64 (u32 cursor) directly via ldrb/ldr/str, proven via objdump mnemonics.
// Nonstandard r1 ABI: body uses only r1 (chan* in r1, r0 ignored). Caller at 0x080614EC table dispatch (0x02BD31 thumb entry at 0x0614EC) loads channel in r1 before indirect blx (see 0x080614E0 table, 4 entries 0x0802BC85/0x0802BCF5/0x0802BD15/0x0802BD31). Proven via ROM table dump at 0x0614E0 and pool at 0x02BCE4 (0x080614E0) not used for this leaf's direct call, but leaf's self-contained r1 use is exact per listing.
// Do not guess 28-byte PSG layout (sound_d034 sp+4) or mixer IWRAM 0x03007001.
//
// Two further details are equally load-bearing:
//   * `idx` is declared u32, not u8, so the `strb` truncates implicitly and no
//     masking instructions are emitted. Declaring it u8 costs 4 bytes of
//     lsls/lsrs and drops the body to 12/20.
//   * `idx` is REUSED for the loaded slot value after `base` is formed. agbcc
//     will not recycle the pinned r2 on its own; reassigning it is what turns
//     `ldr r0,[r3,#68]` into `ldr r2,[r3,#68]`. Without that reuse the body is
//     18/20. (The 17/20 variant drops the `base` pin as well and lets agbcc
//     fall back to `adds r2,r1,r2`, which reuses r2 for the address node.)
//   * `base` is itself pinned, and typed as `volatile u8 *` rather than u32:
//     a u32 round-trip through `(u32)(ptr)` / `(volatile u32*)(u32)` matches
//     the ROM equally well but emits two pointer-to-int warnings under the
//     host `tools/apple_decls.py` clang pass. The pointer spelling is exact on
//     ARM and warning-free on the host.
//
// Both compilers for this file were checked directly: `arm-none-eabi-gcc`
// with `-Wall -Wextra -Werror` (clean), and the host
// `clang -D__APPLE__ -fsyntax-only... -Werror=implicit-function-declaration`
// sweep (clean). `register... asm("rN")` is a valid ARM declaration; on the
// host `-fsyntax-only` accepts it, so the pins need no `__APPLE__` guard.

void SoundBD30_Rewind(void *dummy, void *chan){
    register u32 idx asm("r2");
    register volatile u8 *base asm("r3");
    (void)dummy;
    volatile u8 *c = (volatile u8*)chan;
    idx = c[2];
    if(idx == 0) return;
    idx--;
    c[2] = (u8)idx;
    base = c + idx * 4;
    idx = *(volatile u32*)(base + 68);
    *(volatile u32*)(c + 64) = idx;
}
#ifndef __APPLE__
void _0802BD30(void *a, void *b) __attribute__((alias("SoundBD30_Rewind")));
void sub_0802BD30(void *a, void *b) __attribute__((alias("SoundBD30_Rewind")));
#endif
