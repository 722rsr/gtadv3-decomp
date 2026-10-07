// ============================================================================
// sound_sequence_init.c — C lift of the remaining sound_init.s gaps
// (VMA 0x0802C898/0x0802C89C/0x0802C8B0)
// and the sound_d584.s gaps (VMA 0x0802D5EC/0x0802D60C/0x0802D680).
//
// Transcribed instruction-for-instruction from the cited asm listings.
// ============================================================================

#include "gba/types.h"

#ifdef __APPLE__
#define HOST_STUB(sig) __attribute__((weak)) sig
#else
#define HOST_STUB(sig) extern sig
#endif

HOST_STUB(void _0802DDCC(void *a, void *b));   // 0x0802DDCC is `bx r1` (0x4708), NOT bx r0

// ----------------------------------------------------------------------------
// 0x0802C898 sub_0802C898 — swi 0x2A (MidiKey2Freq-adjacent; raw BIOS call).
//   svc #42; bx lr
// ----------------------------------------------------------------------------
__attribute__((naked)) u32 _0802C898(u32 a, u32 b, u32 c)
{
    __asm__ volatile ("swi 0x2A\n bx lr\n");
}
#ifndef __APPLE__
u32 sub_0802C898(u32 a, u32 b, u32 c) __attribute__((alias("_0802C898")));
#endif

// ----------------------------------------------------------------------------
// 0x0802C89C sub_0802C89C(a) — VBlank-side tick via the relocated-code veneer:
//   push {lr}; r1 = [0x0203EC38]; bl 0x0802DDCC; return
void _0802C89C(void *a)
{
    void *ctx = *(void **)(uintptr_t)0x0203EC38u;
    _0802DDCC(a, ctx);
}

// ----------------------------------------------------------------------------
// 0x0802C8B0 sub_0802C8B0(self) — VCounter-side tick, same veneer:
//   r1 = 0x0203EC3C; r1 = [r1]; bl 0x0802DDCC
//
// Same shape as _0802C89C above. The veneer at 0x0802DDCC is `bx r1`
// (ROM 0x0802DDCC = 08 47), so the callee has to end up in r1 and r0 has
// to be left alone; that only happens when the first argument is already
// resident in r0 on entry, i.e. when the body has a parameter. Both
// callers agree: asm/sound_tick.s and asm/sound_alloc.s both do
// `adds r0, r4, #0 / bl 0x0802C8B0` before calling. The body never reads
// it, and neither does the ROM — r0 is simply carried through to the
// callback as its first argument.
// ----------------------------------------------------------------------------
void _0802C8B0(void *self)
{
    _0802DDCC(self, *(void **)(uintptr_t)0x0203EC3Cu);
}

// asm/sound_init.s:149-150 defines BOTH `sub_0802C8B0:` and `_0802C8B0:` on this
// one body, so the C replacement must publish both spellings. Callers
// (src/runtime_state_dispatch.c) use the `_`-form; the manifest `export` records both.
#ifndef __APPLE__
void sub_0802C8B0(void *self) __attribute__((alias("_0802C8B0")));
#endif

// ----------------------------------------------------------------------------
// 0x0802D5EC sub_0802D5EC(voice) — set voice "released" state bits:
//   r1=voice; [r1+26]=0; [r1+22]=0
//   r0 = ([r1+24]==0) ? 12 : 3
//   [r1+0] |= r0
//
// The field offsets are what force the ROM's exact shape. Indexed `u8 *`
// arithmetic is NOT equivalent here, and the difference is two instructions,
// not a register choice:
//
//   ROM                             indexed-u8* candidate
//   movs r2, #0   (+2, DEAD)        (absent)
//   movs r0, #0   (+4)              movs r0, #0   (+2)
//... bne.n / movs r0,#12 /... movs r2,#3 / cmp / bne.n /
//       b.n / movs r0,#3                movs r2,#12   (no `b.n`; 3 hoisted)
//
// The dead `movs r2, #0` is the whole story. The ROM materialises the constant
// zero TWICE, into r2 and then r0, and both `strb`s use the second one. agbcc
// common-subexpression-eliminates the two zeros in the indexed form, and then
// schedules the `3` arm into the fallthrough (dropping the `b.n`), which also
// moves the flag from r0 into r2. Typing the parameter as a struct makes agbcc
// keep two independent zero constants and preserve the branch layout, giving
// the ROM's instruction sequence exactly. Measured 9/32 (indexed) -> 30/32.
//
// The remaining 2 bytes are NOT a body difference: the body is byte-identical
// for all 15 instructions (30 bytes). The ROM pads the span with `.short 0`
// (`00 00`) while agbcc emits an alignment `nop` (`c0 46`) at the same offset,
// so `first_diff == matched_bytes == 30` with `rom_bytes - matched == 2` —
// the trailing-alignment-pad case, not a codegen defect.
typedef struct {
    u8 f0;
    u8 pad1[21];
    u8 f22;
    u8 f23;
    u8 f24;
    u8 f25;
    u8 f26;
} SoundVoice;

void _0802D5EC(u8 *voice)
{
    SoundVoice *v = (SoundVoice *)(void *)voice;

    v->f26 = 0;
    v->f22 = 0;
    if (v->f24 == 0)
        v->f0 |= 12u;
    else
        v->f0 |= 3u;
}

__asm__(".align 2, 0");

// asm/sound_d584.s:66-67 defines BOTH `sub_0802D5EC:` and `_0802D5EC:` on this
// one body, so the C replacement must publish both spellings — the same
// requirement as _0802C8B0 above. Both asm call sites branch by address form
// (`bl 0x0802D5EC`), so no caller rename is needed; the manifest `export`
// records both.
#ifndef __APPLE__
void sub_0802D5EC(u8 *voice) __attribute__((alias("_0802D5EC")));
#endif

// ----------------------------------------------------------------------------
// 0x0802D60C sub_0802D60C(root, mask_u16, val_u8) — "Smsh" magic-gated
//   channel-flag writer:
//   r6=root, sl=(u16)mask, r8=(u8)val
//   if [root+52] == 0x68736D53:
//     [root+52] = magic+1
//     r5 = [root+8] (count), r4 = [root+44] (channel array), r7 = 1
//     while r5 > 0:
//       if (mask & r7) and (ch[0] & 128):
//         ch[23] = val
//         if val == 0: _0802D5EC(ch)
//       r5--; r4 += 80; r7 <<= 1
//     [root+52] = magic
//
// Widening `mask`/`val` to word width (the P2 lever) is WRONG here: the ROM
// carries `lsls r1,#16 / lsrs r1,#16` and `lsls r2,#24 / lsrs r2,#24` in the
// prologue, so the u16/u8 declarations are what produce them. Measured:
// 17/116 with u16/u8 vs 6/116 with u32/u32.
//
// Two non-obvious load points, both measured:
//  * The count must be read through a NON-volatile `*(u8 *)` load. As
//    `*(volatile u8 *)` agbcc materialises it in r0 and emits a redundant
//    `adds r5, r0, #0`; the ROM loads straight into r5. 47/116 -> 110/116.
//  * A `volatile u8 *r = root;` local makes agbcc schedule the `root` copy
//    *after* the parameter narrowing instead of before it. Using `root`
//    directly is worth 8 instructions of prologue order. 35/116 -> 47/116.
// `u8 v = val;` inside the guarded do-while is what forces the second high
// register the ROM keeps (r8 and r9 both hold `val`); without it agbcc spills
// `val` to the stack across the _0802D5EC call.
// ----------------------------------------------------------------------------
void _0802D60C(volatile u8 *root, u16 mask, u8 val)
{
    u32 m = *(volatile u32 *)(root + 52);

    if (m != 0x68736D53u)
        return;
    *(volatile u32 *)(root + 52) = m + 1;
    {
        int n = *(u8 *)(root + 8);
        u8 *ch = *(u8 **)(root + 44);
        u32 bit = 1;
        if (n > 0) {
            u8 v = val;
            do {
                if ((mask & bit) && (ch[0] & 128)) {
                    ch[23] = val;
                    if (v == 0)
                        _0802D5EC(ch);
                }
                n--;
                ch += 80;
                bit <<= 1;
            } while (n > 0);
        }
    }
    *(volatile u32 *)(root + 52) = 0x68736D53u;
}

// ----------------------------------------------------------------------------
// 0x0802D680 sub_0802D680(root, mask_u16, val_u8) — twin of _0802D60C
//   writing ch[25] instead of ch[23].
// Verified independently of _0802D60C above (not assumed to follow it): the
// same three source-shape fixes apply and the result is the same 110/116 with
// the same two residual differences. 17/116 at baseline.
// ----------------------------------------------------------------------------
void _0802D680(volatile u8 *root, u16 mask, u8 val)
{
    u32 m = *(volatile u32 *)(root + 52);

    if (m != 0x68736D53u)
        return;
    *(volatile u32 *)(root + 52) = m + 1;
    {
        int n = *(u8 *)(root + 8);
        u8 *ch = *(u8 **)(root + 44);
        u32 bit = 1;
        if (n > 0) {
            u8 v = val;
            do {
                if ((mask & bit) && (ch[0] & 128)) {
                    ch[25] = val;
                    if (v == 0)
                        _0802D5EC(ch);
                }
                n--;
                ch += 80;
                bit <<= 1;
            } while (n > 0);
        }
    }
    *(volatile u32 *)(root + 52) = 0x68736D53u;
}
