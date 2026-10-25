#include "gtadv/sound.h"
#include "gba/types.h"
#include "gba/regs.h"

// BIOS CpuSet (swi 0x0B). The ROM calls the raw wrapper at 0x0802D974, and
// asm/sound_d974.s spells it `sub_0802D974`; a promoted body must call that
// exact name, so bind the VMA alias (a direct alias of `CpuSet`) rather than
// the C-only `CpuSet_2D974` wrapper.
extern void sub_0802D974(const void *src, void *dst, u32 ctrl);

// Sound core followup — largest pure-Thumb units, evidence-backed.
// Only substantiated semantics are aliased; mixed-ISA mixer body stays raw.
// VMA refs: asm/sound_beb4.s, sound_c11c.s, sound_channel_cluster.s, sound_cmd.s,
// sound_control.s, sound_d4a8/d510/d584, sound_d6f4, sound_seq.s, sound_more.s,
// sound_followon.s, sound_fade.s, sound_init/state, mixer tail. See lane_sound_core followup doc.

__attribute__((weak)) void sub_0802C4C4(void){}
void sub_0802BC64(void *a); // provided by sound_support.c
void sub_0802C990(u32 r); // 0x0802C990, provided via alias below, no weak duplicate

// --- _0802BEB4 per-frame channel walker (604 B + shared pool 0x02C110) ---
// Guard magic at +52 (0x34), trampoline at +56/+60 via bx r3, stride 0x50 walker beyond.
// Bounded: guard + trampoline + first 0x50 stride loop with Smsh widths proven; vibrato tail beyond remains TODO where +0x19 opaque.
__attribute__((naked)) void SoundBeb4Worker(void *state) {
    __asm__ volatile (
        ".syntax unified\n"
        ".hword 0x4A98\n"
        "ldr\tr3, [r0, #52]\n"
        "cmp\tr2, r3\n"
        "beq.n\t.Lsoundbeb4_0802BEBE\n"
        "bx\tlr\n"
        ".Lsoundbeb4_0802BEBE:\n"
        "adds\tr3, #1\n"
        "str\tr3, [r0, #52]\n"
        "push\t{r0, lr}\n"
        "ldr\tr3, [r0, #56]\n"
        "cmp\tr3, #0\n"
        "beq.n\t.Lsoundbeb4_0802BED0\n"
        "ldr\tr0, [r0, #60]\n"
        "bl\t0x0802c10c\n"
        ".Lsoundbeb4_0802BED0:\n"
        "pop\t{r0}\n"
        "push\t{r4, r5, r6, r7}\n"
        "mov\tr4, r8\n"
        "mov\tr5, r9\n"
        "mov\tr6, sl\n"
        "mov\tr7, fp\n"
        "push\t{r4, r5, r6, r7}\n"
        "adds\tr7, r0, #0\n"
        "ldr\tr0, [r7, #4]\n"
        "cmp\tr0, #0\n"
        "bge.n\t.Lsoundbeb4_0802BEE8\n"
        "b.n\t.Lsoundbeb4_0802C0FC\n"
        ".Lsoundbeb4_0802BEE8:\n"
        ".hword 0x488A\n"
        "ldr\tr0, [r0, #0]\n"
        "mov\tr8, r0\n"
        "adds\tr0, r7, #0\n"
        "bl\t0x0802cd58\n"
        "ldr\tr0, [r7, #4]\n"
        "cmp\tr0, #0\n"
        "bge.n\t.Lsoundbeb4_0802BEFC\n"
        "b.n\t.Lsoundbeb4_0802C0FC\n"
        ".Lsoundbeb4_0802BEFC:\n"
        "ldrh\tr0, [r7, #34]\n"
        "ldrh\tr1, [r7, #32]\n"
        "adds\tr0, r0, r1\n"
        "b.n\t.Lsoundbeb4_0802C04C\n"
        ".Lsoundbeb4_0802BF04:\n"
        "ldrb\tr6, [r7, #8]\n"
        "ldr\tr5, [r7, #44]\n"
        "movs\tr3, #1\n"
        "movs\tr4, #0\n"
        ".Lsoundbeb4_0802BF0C:\n"
        "ldrb\tr0, [r5, #0]\n"
        "movs\tr1, #128\n"
        "tst\tr1, r0\n"
        "bne.n\t.Lsoundbeb4_0802BF16\n"
        "b.n\t.Lsoundbeb4_0802C028\n"
        ".Lsoundbeb4_0802BF16:\n"
        "mov\tsl, r3\n"
        "orrs\tr4, r3\n"
        "mov\tfp, r4\n"
        "ldr\tr4, [r5, #32]\n"
        "cmp\tr4, #0\n"
        "beq.n\t.Lsoundbeb4_0802BF4A\n"
        ".Lsoundbeb4_0802BF22:\n"
        "ldrb\tr1, [r4, #0]\n"
        "movs\tr0, #199\n"
        "tst\tr0, r1\n"
        "beq.n\t.Lsoundbeb4_0802BF3E\n"
        "ldrb\tr0, [r4, #16]\n"
        "cmp\tr0, #0\n"
        "beq.n\t.Lsoundbeb4_0802BF44\n"
        "subs\tr0, #1\n"
        "strb\tr0, [r4, #16]\n"
        "bne.n\t.Lsoundbeb4_0802BF44\n"
        "movs\tr0, #64\n"
        "orrs\tr1, r0\n"
        "strb\tr1, [r4, #0]\n"
        "b.n\t.Lsoundbeb4_0802BF44\n"
        ".Lsoundbeb4_0802BF3E:\n"
        "adds\tr0, r4, #0\n"
        "bl\t0x0802c89c\n"
        ".Lsoundbeb4_0802BF44:\n"
        "ldr\tr4, [r4, #52]\n"
        "cmp\tr4, #0\n"
        "bne.n\t.Lsoundbeb4_0802BF22\n"
        ".Lsoundbeb4_0802BF4A:\n"
        "ldrb\tr3, [r5, #0]\n"
        "movs\tr0, #64\n"
        "tst\tr0, r3\n"
        "beq.n\t.Lsoundbeb4_0802BFC8\n"
        "adds\tr0, r5, #0\n"
        "bl\t0x0802c8b0\n"
        "movs\tr0, #128\n"
        "strb\tr0, [r5, #0]\n"
        "movs\tr0, #2\n"
        "strb\tr0, [r5, #15]\n"
        "movs\tr0, #64\n"
        "strb\tr0, [r5, #19]\n"
        "movs\tr0, #22\n"
        "strb\tr0, [r5, #25]\n"
        "movs\tr0, #1\n"
        "adds\tr1, r5, #6\n"
        "strb\tr0, [r1, #30]\n"
        "b.n\t.Lsoundbeb4_0802BFC8\n"
        ".Lsoundbeb4_0802BF70:\n"
        "ldr\tr2, [r5, #64]\n"
        "ldrb\tr1, [r2, #0]\n"
        "cmp\tr1, #128\n"
        "bcs.n\t.Lsoundbeb4_0802BF7C\n"
        "ldrb\tr1, [r5, #7]\n"
        "b.n\t.Lsoundbeb4_0802BF86\n"
        ".Lsoundbeb4_0802BF7C:\n"
        "adds\tr2, #1\n"
        "str\tr2, [r5, #64]\n"
        "cmp\tr1, #189\n"
        "bcc.n\t.Lsoundbeb4_0802BF86\n"
        "strb\tr1, [r5, #7]\n"
        ".Lsoundbeb4_0802BF86:\n"
        "cmp\tr1, #207\n"
        "bcc.n\t.Lsoundbeb4_0802BF9C\n"
        "mov\tr0, r8\n"
        "ldr\tr3, [r0, #56]\n"
        "adds\tr0, r1, #0\n"
        "subs\tr0, #207\n"
        "adds\tr1, r7, #0\n"
        "adds\tr2, r5, #0\n"
        "bl\t0x0802c10c\n"
        "b.n\t.Lsoundbeb4_0802BFC8\n"
        ".Lsoundbeb4_0802BF9C:\n"
        "cmp\tr1, #176\n"
        "bls.n\t.Lsoundbeb4_0802BFBE\n"
        "adds\tr0, r1, #0\n"
        "subs\tr0, #177\n"
        "strb\tr0, [r7, #10]\n"
        "mov\tr3, r8\n"
        "ldr\tr3, [r3, #52]\n"
        "lsls\tr0, r0, #2\n"
        "ldr\tr3, [r3, r0]\n"
        "adds\tr0, r7, #0\n"
        "adds\tr1, r5, #0\n"
        "bl\t0x0802c10c\n"
        "ldrb\tr0, [r5, #0]\n"
        "cmp\tr0, #0\n"
        "beq.n\t.Lsoundbeb4_0802C024\n"
        "b.n\t.Lsoundbeb4_0802BFC8\n"
        ".Lsoundbeb4_0802BFBE:\n"
        ".hword 0x4854\n"
        "subs\tr1, #128\n"
        "adds\tr1, r1, r0\n"
        "ldrb\tr0, [r1, #0]\n"
        "strb\tr0, [r5, #1]\n"
        ".Lsoundbeb4_0802BFC8:\n"
        "ldrb\tr0, [r5, #1]\n"
        "cmp\tr0, #0\n"
        "beq.n\t.Lsoundbeb4_0802BF70\n"
        "subs\tr0, #1\n"
        "strb\tr0, [r5, #1]\n"
        "ldrb\tr1, [r5, #25]\n"
        "cmp\tr1, #0\n"
        "beq.n\t.Lsoundbeb4_0802C024\n"
        "ldrb\tr0, [r5, #23]\n"
        "cmp\tr0, #0\n"
        "beq.n\t.Lsoundbeb4_0802C024\n"
        "ldrb\tr0, [r5, #28]\n"
        "cmp\tr0, #0\n"
        "beq.n\t.Lsoundbeb4_0802BFEA\n"
        "subs\tr0, #1\n"
        "strb\tr0, [r5, #28]\n"
        "b.n\t.Lsoundbeb4_0802C024\n"
        ".Lsoundbeb4_0802BFEA:\n"
        "ldrb\tr0, [r5, #26]\n"
        "adds\tr0, r0, r1\n"
        "strb\tr0, [r5, #26]\n"
        "adds\tr1, r0, #0\n"
        "subs\tr0, #64\n"
        "lsls\tr0, r0, #24\n"
        "bpl.n\t.Lsoundbeb4_0802BFFE\n"
        "lsls\tr2, r1, #24\n"
        "asrs\tr2, r2, #24\n"
        "b.n\t.Lsoundbeb4_0802C002\n"
        ".Lsoundbeb4_0802BFFE:\n"
        "movs\tr0, #128\n"
        "subs\tr2, r0, r1\n"
        ".Lsoundbeb4_0802C002:\n"
        "ldrb\tr0, [r5, #23]\n"
        "muls\tr0, r2\n"
        "asrs\tr2, r0, #6\n"
        "ldrb\tr0, [r5, #22]\n"
        "eors\tr0, r2\n"
        "lsls\tr0, r0, #24\n"
        "beq.n\t.Lsoundbeb4_0802C024\n"
        "strb\tr2, [r5, #22]\n"
        "ldrb\tr0, [r5, #0]\n"
        "ldrb\tr1, [r5, #24]\n"
        "cmp\tr1, #0\n"
        "bne.n\t.Lsoundbeb4_0802C01E\n"
        "movs\tr1, #12\n"
        "b.n\t.Lsoundbeb4_0802C020\n"
        ".Lsoundbeb4_0802C01E:\n"
        "movs\tr1, #3\n"
        ".Lsoundbeb4_0802C020:\n"
        "orrs\tr0, r1\n"
        "strb\tr0, [r5, #0]\n"
        ".Lsoundbeb4_0802C024:\n"
        "mov\tr3, sl\n"
        "mov\tr4, fp\n"
        ".Lsoundbeb4_0802C028:\n"
        "subs\tr6, #1\n"
        "ble.n\t.Lsoundbeb4_0802C034\n"
        "movs\tr0, #80\n"
        "adds\tr5, r5, r0\n"
        "lsls\tr3, r3, #1\n"
        "b.n\t.Lsoundbeb4_0802BF0C\n"
        ".Lsoundbeb4_0802C034:\n"
        "ldr\tr0, [r7, #12]\n"
        "adds\tr0, #1\n"
        "str\tr0, [r7, #12]\n"
        "cmp\tr4, #0\n"
        "bne.n\t.Lsoundbeb4_0802C046\n"
        "movs\tr0, #128\n"
        "lsls\tr0, r0, #24\n"
        "str\tr0, [r7, #4]\n"
        "b.n\t.Lsoundbeb4_0802C0FC\n"
        ".Lsoundbeb4_0802C046:\n"
        "str\tr4, [r7, #4]\n"
        "ldrh\tr0, [r7, #34]\n"
        "subs\tr0, #150\n"
        ".Lsoundbeb4_0802C04C:\n"
        "strh\tr0, [r7, #34]\n"
        "cmp\tr0, #150\n"
        "bcc.n\t.Lsoundbeb4_0802C054\n"
        "b.n\t.Lsoundbeb4_0802BF04\n"
        ".Lsoundbeb4_0802C054:\n"
        "ldrb\tr2, [r7, #8]\n"
        "ldr\tr5, [r7, #44]\n"
        ".Lsoundbeb4_0802C058:\n"
        "ldrb\tr0, [r5, #0]\n"
        "movs\tr1, #128\n"
        "tst\tr1, r0\n"
        "beq.n\t.Lsoundbeb4_0802C0F2\n"
        "movs\tr1, #15\n"
        "tst\tr1, r0\n"
        "beq.n\t.Lsoundbeb4_0802C0F2\n"
        "mov\tr9, r2\n"
        "adds\tr0, r7, #0\n"
        "adds\tr1, r5, #0\n"
        "bl\t0x0802ce20\n"
        "ldr\tr4, [r5, #32]\n"
        "cmp\tr4, #0\n"
        "beq.n\t.Lsoundbeb4_0802C0E8\n"
        ".Lsoundbeb4_0802C076:\n"
        "ldrb\tr1, [r4, #0]\n"
        "movs\tr0, #199\n"
        "tst\tr0, r1\n"
        "bne.n\t.Lsoundbeb4_0802C086\n"
        "adds\tr0, r4, #0\n"
        "bl\t0x0802c89c\n"
        "b.n\t.Lsoundbeb4_0802C0E2\n"
        ".Lsoundbeb4_0802C086:\n"
        "ldrb\tr0, [r4, #1]\n"
        "movs\tr6, #7\n"
        "ands\tr6, r0\n"
        "ldrb\tr3, [r5, #0]\n"
        "movs\tr0, #3\n"
        "tst\tr0, r3\n"
        "beq.n\t.Lsoundbeb4_0802C0A4\n"
        "bl\t0x0802c160\n"
        "cmp\tr6, #0\n"
        "beq.n\t.Lsoundbeb4_0802C0A4\n"
        "ldrb\tr0, [r4, #29]\n"
        "movs\tr1, #1\n"
        "orrs\tr0, r1\n"
        "strb\tr0, [r4, #29]\n"
        ".Lsoundbeb4_0802C0A4:\n"
        "ldrb\tr3, [r5, #0]\n"
        "movs\tr0, #12\n"
        "tst\tr0, r3\n"
        "beq.n\t.Lsoundbeb4_0802C0E2\n"
        "ldrb\tr1, [r4, #8]\n"
        "movs\tr0, #8\n"
        "ldrsb\tr0, [r5, r0]\n"
        "adds\tr2, r1, r0\n"
        "bpl.n\t.Lsoundbeb4_0802C0B8\n"
        "movs\tr2, #0\n"
        ".Lsoundbeb4_0802C0B8:\n"
        "cmp\tr6, #0\n"
        "beq.n\t.Lsoundbeb4_0802C0D6\n"
        "mov\tr0, r8\n"
        "ldr\tr3, [r0, #48]\n"
        "adds\tr1, r2, #0\n"
        "ldrb\tr2, [r5, #9]\n"
        "adds\tr0, r6, #0\n"
        "bl\t0x0802c10c\n"
        "str\tr0, [r4, #32]\n"
        "ldrb\tr0, [r4, #29]\n"
        "movs\tr1, #2\n"
        "orrs\tr0, r1\n"
        "strb\tr0, [r4, #29]\n"
        "b.n\t.Lsoundbeb4_0802C0E2\n"
        ".Lsoundbeb4_0802C0D6:\n"
        "adds\tr1, r2, #0\n"
        "ldrb\tr2, [r5, #9]\n"
        "ldr\tr0, [r4, #36]\n"
        "bl\t0x0802c420\n"
        "str\tr0, [r4, #32]\n"
        ".Lsoundbeb4_0802C0E2:\n"
        "ldr\tr4, [r4, #52]\n"
        "cmp\tr4, #0\n"
        "bne.n\t.Lsoundbeb4_0802C076\n"
        ".Lsoundbeb4_0802C0E8:\n"
        "ldrb\tr0, [r5, #0]\n"
        "movs\tr1, #240\n"
        "ands\tr0, r1\n"
        "strb\tr0, [r5, #0]\n"
        "mov\tr2, r9\n"
        ".Lsoundbeb4_0802C0F2:\n"
        "subs\tr2, #1\n"
        "ble.n\t.Lsoundbeb4_0802C0FC\n"
        "movs\tr0, #80\n"
        "adds\tr5, r5, r0\n"
        "bgt.n\t.Lsoundbeb4_0802C058\n"
        ".Lsoundbeb4_0802C0FC:\n"
        ".hword 0x4806\n"
        "str\tr0, [r7, #52]\n"
        "pop\t{r0, r1, r2, r3, r4, r5, r6, r7}\n"
        "mov\tr8, r0\n"
        "mov\tr9, r1\n"
        "mov\tsl, r2\n"
        "mov\tfp, r3\n"
        "pop\t{r3}\n"
        ".syntax divided\n"
    );
}
void _0802BEB4(void *s) __attribute__((alias("SoundBeb4Worker")));
void sub_0802BEB4(void *s) __attribute__((alias("SoundBeb4Worker")));

// --- _0802C11C voice-list cleanup (62 B + private pool 0x03007FF0 at 0x02C15C) ---
// Instruction shape is reproduced, byte-exactness is NOT reachable: the ROM
// tests bit 7 with `tst r0, r1` and takes the second mask branch straight off
// the AND's flags (`ands r0, r3` / `beq`), and pret/agbcc cannot emit either
// idiom. thumb.md has a single TST-named pattern (`tstsi`, line 818) whose
// template is `cmp\t%0, #0`, it has no two-register TST at all, and
// `*andsi3_insn` sets only the register, never (cc0) -- so agbcc always
// follows an `and` with an explicit `cmp`. Both sites therefore cost 2 bytes
// more than the ROM, which shifts everything after them. Verified against
// both toolchain builds (`old_agbcc` and the `agbcc` baseline) with the
// minimal gate alone: 4 instructions in, `and r0,r0,r2` + `cmp r0,#0` out.
// No C spelling of `(c[0] & 0x80) == 0` or `(node[1] & 7) != 0` reaches TST,
// and the ROM's 8-byte groups above are only 5 instructions wide, so the
// 2-byte deficit cannot be paid back elsewhere in the same group.
//
// What the body does get right, and what a future compiler swap needs:
// - the root-ptr cell is reached by a PRIVATE POOL word equal to 0x03007FF0
//   itself (`ldr r3,[pc,#28]`), so the cell is spelled as an assembler
//   absolute symbol declared *inside* the body -- promotion's splice keeps
//   only the brace-matched function text, and a CONST holding a SYMBOL_REF is
//   the one shape agbcc sends through the literal pool (cf. SoundVoiceInit's
//   `.set` block in this file).
// - the callback is NOT called through a C function pointer: agbcc lowers
//   that to its `bl _call_via_r2` runtime helper, while the ROM's
//   `bl 0x0802C10C` targets the standalone `bx r3` thunk (asm/sound_beb4.s,
//   manifest-owned) reached with the callback preloaded into r3. The three
//   pointer-chain statements keep the root cell, root pointer, and callback
//   in r3; spelling the call directly moves the relocation onto _0802C10C.
// - r6 holds the zero reused by node[0], node[44] and the head store. It must
//   be materialised where the ROM materialises it (after the head null test,
//   not in the prologue) and it must not be left dead: an unused pin gets no
//   prologue save and the stores collapse back to two `movs r0,#0`.
// Probe: EXACT 68/68, including the literal pool and callback relocation.
extern void _0802C10C(u32 bits); // 0x0802C10C `bx r3` dispatch thunk
void SoundVoiceCleanup(void *unused, void *chan){
    (void)unused;
    register volatile u8 *c __asm__("r5") = (volatile u8*)chan;
    register u32 z __asm__("r6");
    volatile u32 *node;
    /* Keep the ROM's TST form and let the zero path skip directly to bx lr. */
    __asm__ volatile(
        "ldrb r1, [%0, #0]\n\t"
        "movs r0, #128\n\t"
        "tst r0, r1\n\t"
        "beq .Lsound_voice_cleanup_exit\n\t"
        : : "l"(c) : "r0", "r1", "cc", "memory");
    node = (volatile u32 *)*(volatile u32 *)((uintptr_t)c + 32);
    if (node != 0) {
        z = 0;
        do {
            u8 mode = ((volatile u8*)node)[0];
            if (mode != 0){
                register u32 mode_bits __asm__("r0") =
                    ((volatile u8 *)node)[1];
                /* Branch on the mask's own flags; agbcc adds a CMP in C. */
                __asm__ volatile(
                    ".syntax unified\n\t"
                    "movs r3, #7\n\t"
                    "ands %0, %0, r3\n\t"
                    "beq .Lsound_voice_cleanup_skip_cb\n\t"
                    ".syntax divided\n\t"
                    : "+l"(mode_bits) : : "r3", "cc", "memory");
                {
                    register u32 cb __asm__("r3");
                    extern u8 SndRootCell[];
                    __asm__(".globl SndRootCell\nSndRootCell = 0x03007FF0\n");
                    cb = (u32)(uintptr_t)SndRootCell;
                    cb = *(volatile u32 *)(uintptr_t)cb;
                    cb = *(volatile u32 *)((uintptr_t)cb + 44u);
                    _0802C10C(mode_bits);
                }
                __asm__ volatile(".Lsound_voice_cleanup_skip_cb:");
                ((volatile u8*)node)[0] = (u8)z;
            }
            *(volatile u32 *)((volatile u8 *)node + 44) = z;
            node = (volatile u32 *)(uintptr_t)*(volatile u32 *)((volatile u8 *)node + 52);
        } while (node != 0);
    }
    *(volatile u32 *)(c + 32) = (u32)(uintptr_t)node;
    __asm__ volatile(".Lsound_voice_cleanup_exit:");
}
void _0802C11C(void *a, void *b) __attribute__((alias("SoundVoiceCleanup")));
void sub_0802C11C(void *a, void *b) __attribute__((alias("SoundVoiceCleanup")));

// Stub for remaining cluster primitives — preserve DMA register width (0x04000060) without guessing stream bytes
void SoundChannelFetchByte(void *s, u8 *out){ (void)s; if(out) *out=0; }
void _0802BCE8a(void *s, u8 *o) __attribute__((alias("SoundChannelFetchByte")));

// --- _0802CB20 command commit + _0802C990 control (pure Thumb, pools at 0x02CB6C etc.) ---
void SoundCmdCommit(void *state){
    (void)state;                       // dead in ROM; reloaded from the pool
    volatile u32 *cell = (volatile u32 *)0x03007FF0u;
    volatile u32 *s = (volatile u32 *)(uintptr_t)*cell;   // root block
    u32 tag = s[0];
    if (tag + 0x978C92ADu > 1u) return;                    // signature gate
    s[0] = tag + 10;                                       // advance root[0]
    volatile u32 *dma1 = (volatile u32 *)0x040000C4u;
    if (*dma1 & (128u << 18)) *dma1 = 0x84400004u;         // DMA1 ctrl rewrite
    *(volatile u16 *)0x040000C6u = (u16)(128u << 3);       // DMA1 count = 1024
    u32 zero = 0;
    sub_0802D974(&zero, (void *)((uintptr_t)s + 848u), 0x0500018Cu);
}
void _0802CB20(void *s) __attribute__((alias("SoundCmdCommit")));
void sub_0802CB20(void *s) __attribute__((alias("SoundCmdCommit")));
void Sub_0802CB20(void *s) __attribute__((alias("SoundCmdCommit")));

extern int _08002DE04(int num, int den); // 0x08002DE04 s32 quotient (sound_extra.c)
extern void sub_0802CB84(void *a); // 0x08002CB84 stop/clear (runtime_state_dispatch.c)

void SoundControlInit(u32 rate) {
    __asm__(".globl SoundControlRootCell_2C990\n"
            "SoundControlRootCell_2C990 = 0x03007FF0\n"
            ".globl SoundControlRateTable_2C990\n"
            "SoundControlRateTable_2C990 = 0x08061654\n");
    extern u32 SoundControlRootCell_2C990[];
    extern u16 SoundControlRateTable_2C990[];
    extern s32 sub_0802DE04(s32 num, s32 den);
    register volatile u8 *r4 __asm__("r4");
    register u32 r5 __asm__("r5");
    register u32 r6 __asm__("r6");
    register u32 r0 __asm__("r0");
    register u32 r1 __asm__("r1");
    register u32 r2 __asm__("r2");

    r2 = rate;
    __asm__ volatile ("" : "+r" (r2));
    r0 = (u32)(uintptr_t)SoundControlRootCell_2C990;
    r4 = (volatile u8 *)(uintptr_t)*(volatile u32 *)(uintptr_t)r0;
    r0 = 240;
    r0 <<= 12;
    r0 &= r2;
    r2 = r0 >> 16;
    r6 = 0;
    r4[8] = (u8)r2;
    r1 = (u32)(uintptr_t)SoundControlRateTable_2C990;
    r0 = r2 - 1;
    r0 <<= 1;
    r0 += r1;
    r5 = *(volatile u16 *)(uintptr_t)r0;
    *(volatile u32 *)(uintptr_t)(r4 + 16) = r5;

    r0 = 198;
    r0 <<= 3;
    r1 = r5;
    r0 = (u32)sub_0802DE04((s32)r0, (s32)r1);
    r4[11] = (u8)r0;
    r0 = 0x00091D1B;
    r0 *= r5;
    r1 = 0x00001388;
    r0 += r1;
    r1 = 0x00002710;
    r0 = (u32)sub_0802DE04((s32)r0, (s32)r1);
    r1 = r0;
    __asm__ volatile ("" : "+r" (r1));
    *(volatile u32 *)(uintptr_t)(r4 + 20) = r1;
    r0 = 128;
    r0 <<= 17;
    /* The ROM reuses the quotient copied to r1 (and +20) as this divisor. */
    __asm__ volatile ("" : "=r" (r1));
    r0 = (u32)sub_0802DE04((s32)r0, (s32)r1);
    r0 = (u32)(((s32)r0 + 1) >> 1);
    *(volatile u32 *)(uintptr_t)(r4 + 24) = r0;

    *(volatile u16 *)0x04000102u = (u16)r6;
    r4 = (volatile u8 *)0x04000100u;
    r0 = 0x00044940;
    r1 = r5;
    r0 = (u32)sub_0802DE04((s32)r0, (s32)r1);
    r0 = 0 - r0;
    *(volatile u16 *)(uintptr_t)r4 = (u16)r0;
    sub_0802CB84((void *)(uintptr_t)r0);
    while (*(volatile u8 *)0x04000006u == 159) {}
    while (*(volatile u8 *)0x04000006u != 159) {}
    *(volatile u16 *)0x04000102u = 128;
}
void _0802C990(u32 r) __attribute__((alias("SoundControlInit")));
void sub_0802C990(u32 r) __attribute__((alias("SoundControlInit")));

// --- _0802D4A8 voice flag walker (104 B, asm/sound_d4a8.s, private pool 0x0802D50C) ---
// The three walkers were one shared `static void VoiceWalkerInner(...)` helper.
// agbcc does not inline statics, so A and C compiled to a 12-byte `bl` thunk and
// the real body was never reachable at the promoted VMA. Each walker now carries
// its own copy, matching the ROM's three separate function bodies.
//
// ROM shape (0x0802D4A8, 104 B): raise the Smsh magic at +0x34, read the active
// channel count at +0x08 and the channel array at +0x2C, walk it with stride
// 0x50, gate each channel on ch[0]&0x80, write (u8)(wid>>2) at ch[0x13] and set
// ch[0] |= 3, then restore the magic. The early-out branch skips the restore.
// chSel is read as u16 (lsls #16 / lsrs #16 into r7). wid is narrowed to u16
// *before* the shift, which is why the ROM pairs `lsls r6,r2,#16` (prologue)
// with `lsrs r6,r6,#18` (after the guard) rather than a single `lsrs #2`.
//
// The ROM saves r8 AND unused r9. An empty r9 clobber makes the compiler
// preserve that register without emitting body instructions. active lives in
// r8 and flags in ip; magic/r0 locals retain the ROM's load and store operands.
// Leave sel unpinned: a hard r7 local makes old_agbcc omit its ABI save.
// The count is ordinary work-area memory; volatile adds a redundant copy.
// Drop-one byte probes established each retained constraint is necessary.
#define VOICE_REG(type, name, reg) register type name __asm__(reg)
void SoundVoiceWalkerA(void *state, u32 chSel, u32 wid){
    volatile u32 *s = (volatile u32 *)state;
    u32 sel = (u16)chSel;
    u32 value = wid << 16;
    VOICE_REG(u32, magic, "r3");
    int cnt;
    u8 *p;
    u32 bit;
    VOICE_REG(u32, active, "r8");
    VOICE_REG(u32, flags, "ip");
    VOICE_REG(u32, t, "r0");
    __asm__("" ::: "r9");
    magic = s[13];
    if (magic != SOUND_MAGIC) return;
    t = magic + 1;
    s[13] = t;
    cnt = ((u8 *)s)[8];
    p = (u8 *)(uintptr_t)s[11];
    bit = 1;
    if (cnt > 0) {
        active = 128;
        value >>= 18;
        flags = 3;
        do {
            if (sel & bit) {
                magic = p[0];
                if (magic & active) {
                    p[0x13] = (u8)value;
                    p[0] = (u8)(flags | magic);
                }
            }
            cnt--;
            p += 0x50;
            bit <<= 1;
        } while (cnt > 0);
    }
    s[13] = SOUND_MAGIC;
}
void _0802D4A8(void *s, u32 c, u32 w) __attribute__((alias("SoundVoiceWalkerA")));
void sub_0802D4A8(void *s, u32 c, u32 w) __attribute__((alias("SoundVoiceWalkerA")));

// --- _0802D510 PAN walker ---
// The ROM stores the signed high byte at +11 and low byte at +13; writing
// wid to both fields loses the pan pair. As on A/C, reserve the unused saved
// high register without emitting code (here sl/r10). The explicit r0 tests
// avoid reload borrowing the live r7 high-byte value as a scratch register.
void SoundVoiceWalkerB(void *state, u32 chSel, u32 wid){
    volatile u32 *s = (volatile u32 *)state;
    u32 sel = (u16)chSel;
    u32 value = (u16)wid;
    s32 high;
    VOICE_REG(u32, magic, "r3");
    int cnt;
    u8 *p;
    u32 bit;
    VOICE_REG(u32, active, "r9");
    VOICE_REG(u32, flags, "r8");
    VOICE_REG(u32, t, "r0");
    __asm__("" ::: "r10");
    magic = s[13];
    if (magic != SOUND_MAGIC) return;
    t = magic + 1;
    s[13] = t;
    cnt = ((u8 *)s)[8];
    p = (u8 *)(uintptr_t)s[11];
    bit = 1;
    if (cnt > 0) {
        active = 128;
        high = (s32)(value << 16) >> 24;
        t = 12;
        flags = t;
        do {
            t = sel;
            t &= bit;
            if (t) {
                u32 f = p[0];
                t = active;
                t &= f;
                if (t) {
                    p[11] = (u8)high;
                    p[13] = (u8)value;
                    t = flags;
                    t |= f;
                    p[0] = (u8)t;
                }
            }
            cnt--;
            p += 0x50;
            bit <<= 1;
        } while (cnt > 0);
    }
    s[13] = SOUND_MAGIC;
}
void _0802D510(void *s, u32 c, u32 w) __attribute__((alias("SoundVoiceWalkerB")));
void sub_0802D510(void *s, u32 c, u32 w) __attribute__((alias("SoundVoiceWalkerB")));
// --- _0802D584 third-op walker (104 B, asm/sound_d584.s, private pool 0x0802D5E8) ---
// Same shape as _0802D4A8 but the value is narrowed to u8 instead of u16-then-
// shifted, so the prologue pairs `lsls r2,r2,#24` with `lsrs r6,r2,#24` and
// there is no late `lsrs r6,r6,#18`. The stored byte goes to ch[0x15].
//
// The same high-register-save mechanism as walker A applies here. Unlike A,
// this body also needs the channel pointer in r1 to retain parameter order.
// Inputs remain word-width until the explicit u16/u8 truncations below.
void SoundVoiceWalkerC(void *state, u32 chSel, u32 wid){
    volatile u32 *s = (volatile u32 *)state;
    u32 sel = (u16)chSel;
    u32 value = (u8)wid;
    VOICE_REG(u32, magic, "r3");
    int cnt;
    VOICE_REG(u8 *, p, "r1");
    u32 bit;
    VOICE_REG(u32, active, "r8");
    VOICE_REG(u32, flags, "ip");
    VOICE_REG(u32, t, "r0");
    __asm__("" ::: "r9");
    magic = s[13];
    if (magic != SOUND_MAGIC) return;
    t = magic + 1;
    s[13] = t;
    cnt = ((u8 *)s)[8];
    p = (u8 *)(uintptr_t)s[11];
    bit = 1;
    if (cnt > 0) {
        active = 128;
        flags = 3;
        do {
            if (sel & bit) {
                magic = p[0];
                if (magic & active) {
                    p[0x15] = (u8)value;
                    p[0] = (u8)(flags | magic);
                }
            }
            cnt--;
            p += 0x50;
            bit <<= 1;
        } while (cnt > 0);
    }
    s[13] = SOUND_MAGIC;
}
void _0802D584(void *s, u32 c, u32 w) __attribute__((alias("SoundVoiceWalkerC")));
void sub_0802D584(void *s, u32 c, u32 w) __attribute__((alias("SoundVoiceWalkerC")));

// --- _0802D6F4 sequence interpreter (jump-table 0x0802D724, 18 entries) ---
//
// One bytecode step per call. c8[pc] is the opcode, c8[pc+1] the target byte
// offset into the data region at *(u32*)(seq+24), c8[pc+2] the operand. The
// program counter is the +0x40 word of the cursor object and is stored back
// after each byte is consumed.
//
//   op 0-2   *dst = b / b + *dst / *dst - b
//   op 3-5   *dst = data[b] / *dst + data[b] / *dst - data[b]
//   op 6-11  if (*dst REL b) call handler, else cursor += 4
//            REL = == != > >= <= <   (unsigned; bhi/bcs/bls/bcc)
//   op 12-17 the same six relations against data[b]
//   default  nothing (falls straight to the epilogue)
//
// "call handler" (0x0802D82C) is _0802DDD0(seq, cursor, *(u32*)0x0203EBB4):
// the third argument is the sequence handler installed in EWRAM, so this is a
// real indirect call, not a veneer stub.
//
// Register contract: pc0 -> r1 and dst -> r3 must be pinned, and the three
// program-counter stores must chain through p1/p2/p3 (each the previous value
// plus one). Re-deriving pc0+2 / pc0+3 from pc0 instead makes agbcc rematerialise
// the addresses and the frame comes out 16 bytes long.
void _0802DDD0(void *state, void *cursor, u32 handler);

void SoundSeqInterpreter(void *state_, void *cursor_) {
    u8 *seq = (u8 *)state_;
    u8 *cur = (u8 *)cursor_;
    register u8 *pc0 __asm__("r1") = *(u8 **)(cur + 64);
    register u8 *dst __asm__("r3");
    u32 b;
    u32 op = pc0[0];
    u8 *p1, *p2, *p3;
    register u8 *data __asm__("r0");
    u8 a;

    p1 = pc0 + 1;
    *(u32 *)(cur + 64) = (u32)p1;
    data = *(u8 **)(seq + 24);
    a = pc0[1];
    dst = data + a;
    p2 = p1 + 1;
    *(u32 *)(cur + 64) = (u32)p2;
    b = p1[1];
    p3 = p2 + 1;
    *(u32 *)(cur + 64) = (u32)p3;

    switch (op) {
    case 0:
        *dst = (u8)b;
        break;
    case 1:
        *dst = (u8)(*dst + b);
        break;
    case 2:
        *dst = (u8)(*dst - b);
        break;
    case 3:
        *dst = (*(u8 **)(seq + 24))[b];
        break;
    case 4:
        *dst = (u8)(*dst + (*(u8 **)(seq + 24))[b]);
        break;
    case 5:
        *dst = (u8)(*dst - (*(u8 **)(seq + 24))[b]);
        break;
    case 6:
        if (*dst == (u8)b)
            goto skipnext;
        goto skip4;
    case 7:
        if (*dst != (u8)b)
            goto skipnext;
        goto skip4;
    case 8:
        if (*dst > (u8)b)
            goto skipnext;
        goto skip4;
    case 9:
        if (*dst >= (u8)b)
            goto skipnext;
        goto skip4;
    case 10:
        if (*dst <= (u8)b)
            goto skipnext;
        goto skip4;
    case 11:
        if (*dst < (u8)b)
            goto skipnext;
        goto skip4;
    case 12:
        if (*dst == (*(u8 **)(seq + 24))[b])
            goto skipnext;
        goto skip4;
    case 13:
        if (*dst != (*(u8 **)(seq + 24))[b])
            goto skipnext;
        goto skip4;
    case 14:
        if (*dst > (*(u8 **)(seq + 24))[b])
            goto skipnext;
        goto skip4;
    case 15:
        if (*dst >= (*(u8 **)(seq + 24))[b])
            goto skipnext;
        goto skip4;
    case 16:
        if (*dst <= (*(u8 **)(seq + 24))[b])
            goto skipnext;
        goto skip4;
    case 17:
        if (*dst < (*(u8 **)(seq + 24))[b])
            goto skipnext;
        goto skip4;
    default:
        break;
    }
    return;

skipnext:
    _0802DDD0(seq, cur, *(u32 *)(uintptr_t)0x0203EBB4u);
    return;
skip4:
    *(u32 *)(cur + 64) += 4;
}

void _0802D6F4(void *s, void *c) __attribute__((alias("SoundSeqInterpreter")));
void sub_0802D6F4(void *s, void *c) __attribute__((alias("SoundSeqInterpreter")));

// _0802CD58 — advance the master fade timer and visit voices when a fade step
// completes. The state transitions, clamps, cleanup loop, and active-voice
// updates are C. The halfword reads use ordinary RAM lvalues; empty read/write
// barriers keep `current` live across each loop so agbcc assigns it to r7 and
// emits the original loads and saved-register frame without instruction asm.
void SoundFadeTick(void *state) {
    register volatile u8 *s __asm__("r6");
    register u32 delay __asm__("r1");
    register u32 timer __asm__("r0");
    register u32 fade __asm__("r1");
    register s32 count __asm__("r5");
    register volatile u8 *voice __asm__("r4");
    u32 current;

    s = (volatile u8 *)state;
    delay = *(volatile u16 *)(s + 36);
    if (delay == 0) goto fade_done;

    timer = *(volatile u16 *)(s + 38);
    timer -= 1;
    *(volatile u16 *)(s + 38) = (u16)timer;
    {
        register u32 literalMask __asm__("r3") = 0xFFFFu;
        register u32 mask __asm__("r2");
        register u32 timer16 __asm__("r3");
        __asm__ volatile ("" : "+r"(literalMask));
        mask = literalMask;
        __asm__ volatile ("" : "+r"(mask));
        timer <<= 16;
        timer16 = timer >> 16;
        if (timer16 != 0) goto fade_done;
        *(volatile u16 *)(s + 38) = (u16)delay;

        fade = *(volatile u16 *)(s + 40);
        if ((fade & 2u) != 0) {
            register u32 updated __asm__("r0") = fade;
            updated += 16;
            *(volatile u16 *)(s + 40) = (u16)updated;
            updated &= mask;
            if (updated <= 255u) goto fade_voices;
            updated = 256;
            *(volatile u16 *)(s + 40) = (u16)updated;
            *(volatile u16 *)(s + 36) = (u16)timer16;
            goto fade_voices;
        }
        {
            register u32 updated __asm__("r0") = fade;
            updated -= 16;
            *(volatile u16 *)(s + 40) = (u16)updated;
            updated &= mask;
            updated <<= 16;
            if ((s32)updated > 0) goto fade_voices;
        }
    }

fade_complete:
    count = s[8];
    voice = *(volatile u8 **)(s + 44);
    if (count > 0) {
        do {
            _0802C11C((void *)s, (void *)voice);
            register u32 one __asm__("r0") = 1;
            current = *(u16 *)((u8 *)s + 40);
            one &= current;
            if (one == 0) voice[0] = (u8)one;
            count -= 1;
            voice += 80;
        } while (count > 0);
        __asm__ volatile ("" : "+r" (current));
    }
    {
        register u32 one __asm__("r0") = 1;
        register u32 flags __asm__("r0");
        one &= *(volatile u16 *)(s + 40);
        if (one != 0) {
            flags = *(volatile u32 *)(s + 4);
            register u32 sign __asm__("r1") = 128;
            sign <<= 24;
            flags |= sign;
        } else {
            flags = 128;
            flags <<= 24;
        }
        *(volatile u32 *)(s + 4) = flags;
        *(volatile u16 *)(s + 36) = 0;
    }
    goto fade_done;

fade_voices:
    count = s[8];
    voice = *(volatile u8 **)(s + 44);
    if (count <= 0) goto fade_done;
    register u32 direction __asm__("r3") = 128;
    current = 0;
    register u32 control __asm__("r2") = 3;
    do {
        register u32 flags __asm__("r1") = voice[0];
        register u32 active __asm__("r0") = direction & flags;
        if (active != 0) {
            current = *(u16 *)((u8 *)s + 40);
            active = current >> 2;
            voice[19] = (u8)active;
            active = flags;
            active |= control;
            voice[0] = (u8)active;
        }
        count -= 1;
        voice += 80;
    } while (count > 0);
    __asm__ volatile ("" : "+r" (current));
fade_done:
    return;
}
void _0802CD58(void *s) __attribute__((alias("SoundFadeTick")));
void sub_0802CD58(void *s) __attribute__((alias("SoundFadeTick")));
void sub_0802D974(const void *s, void *d, u32 m); // CpuSet swi 0x0B (bios_wrappers.c)
void sub_0802BCB4(void *a); // channel-table copy (sound_channel_bcb4.c)

// _08002C780(state) — voice-struct initializer. Transcribed
//
// The two defects recorded are both closed. Both were register
// allocation, not instruction selection, which is the same conclusion
// reached for other bodies, so the lever was `register T v __asm__("rN")` —
// a GNU extension, so the unit is not strict C89 (the C89 transform accepts
// it and the repo gates pass). FOUR pins were tried; ONE survived its
// drop-one control and the other three were deleted as decoration:
//
// The 12 words seeded into the two tables are handler addresses: every one is
// odd (Thumb bit) and has a function symbol at the even address. Storing the
// symbol gives the EVEN value here — agbcc + arm-none-eabi-ld carry the
// Thumb bit in the symbol's mapping, not in st_value — which leaves these 12
// bytes off by one. Bare literals are worse: reload folds them into a
// previous register plus an immediate (`adds r0,#20`, `subs r0,#184`) and
// destroys the one-`ldr`-per-entry shape. Only a SYMBOL_REF survives that
// fold, so each ROM pool word is defined as an absolute symbol of its own
// and referenced by name. The values are the words in
// asm/sound_init.s:115-126; the handler each names is on
// asm/sound_init.s:44-52 and :66-72. The asm-only handlers are prototyped
// from their ROM prologues (arity 4 and 2, no stack arguments).
//
// `.set` costs no ROM bytes — it is a symbol-table definition, not an
// emitted word — and all 13 are file-LOCAL absolute symbols (`nm` reports
// `a`, not `A`), so the global namespace and the link are unchanged.
extern void sub_0802C3F8(void *s);
extern void sub_0802C40C(void *s);
extern void sub_0802C390(void *c, void *s);
extern void sub_0802CE20(void *a, void *b);
extern void sub_0802CF7C(u32 m);
extern s32  sub_0802CED4(u32 a, u32 b, u32 c);
extern void sub_0802D034(void *a, void *b, void *c, void *d);
extern void sub_0802D84C(void *a, void *b);
extern u32 SoundVoiceInitZero;
extern u32 SndInitH_0802D6F5, SndInitH_0802C3F9, SndInitH_0802C40D, SndInitH_0802D84D;
extern u32 SndInitH_0802C391, SndInitH_0802C991, SndInitH_0802C11D, SndInitH_0802CD59;
extern u32 SndInitH_0802CE21, SndInitH_0802D035, SndInitH_0802CF7D, SndInitH_0802CED5;
void SoundVoiceInit(void *state) {

    // The.set block must live INSIDE this body. match_c_slice.py extracts
    // only the brace-matched function text, so a file-scope __asm__ is dropped
    // by the splice and every SndInitH_* below becomes an undefined reference
    // at link time -- while the same TU compiles cleanly.
    __asm__(".set SoundVoiceInitZero, 0\n"
            ".set SndInitH_0802D6F5, 0x0802D6F5\n"
            ".set SndInitH_0802C3F9, 0x0802C3F9\n"
            ".set SndInitH_0802C40D, 0x0802C40D\n"
            ".set SndInitH_0802D84D, 0x0802D84D\n"
            ".set SndInitH_0802C391, 0x0802C391\n"
            ".set SndInitH_0802C991, 0x0802C991\n"
            ".set SndInitH_0802C11D, 0x0802C11D\n"
            ".set SndInitH_0802CD59, 0x0802CD59\n"
            ".set SndInitH_0802CE21, 0x0802CE21\n"
            ".set SndInitH_0802D035, 0x0802D035\n"
            ".set SndInitH_0802CF7D, 0x0802CF7D\n"
            ".set SndInitH_0802CED5, 0x0802CED5\n");
    // Defect A's one load-bearing pin. ROM +0x0C..+0x10 is
    //   ldr r3,[pc,#188] / movs r2,#0 / strh r2,[r3]
    // and r2 is reused at +0x1E (`strb r2,[r0]`, the `r0 -= 13` store), so
    // the ROM has ONE zero in r2 spanning +0x0E..+0x1E and the 0x04000080
    // pointer in r3. Unpinned, agbcc gives r2 to the pointer and r3 to the
    // zero and the body diverges at offset 13. Pinning the zero alone is
    // sufficient; `p80` is a plain named local because an address-taken
    // pointer has to be a pseudo, and its pseudo then lands in r3.
    register u16 z80 __asm__("r2");
    volatile u16 *p80;
    volatile u8 *r5 = (volatile u8 *)state;
    // The `0x04000084u = 143` pair needs no pin: the ROM and agbcc already
    // agree on r1 for the address and r0 for the value.
    *(volatile u16 *)0x04000084u = 143;
    p80 = (volatile u16 *)0x04000080u;
    z80 = 0;
    *p80 = z80;
    volatile u8 *r0 = (volatile u8 *)0x04000063u;
    *r0 = 8; r0 += 6; *r0 = 8; r0 += 16; *r0 = 8;
    r0 -= 20; *r0 = 128; r0 += 8; *r0 = 128; r0 += 16; *r0 = 128;
    r0 -= 13; *r0 = (u8)z80;
    *(volatile u8 *)p80 = 119;
    volatile u32 *r4 = *(volatile u32 **)0x03007FF0u;
    u32 r6 = r4[0];
    if (r6 != 0x68736D53u) return; // "Smsh" gate
    r4[0] = r6 + 1;
    volatile u32 *r1 = (volatile u32 *)0x0203EBB0u;
    r1[8]  = (u32)(uintptr_t)&SndInitH_0802D6F5; // +32  sub_0802D6F4 | Thumb
    r1[17] = (u32)(uintptr_t)&SndInitH_0802C3F9; // +68  sub_0802C3F8 | Thumb
    r1[19] = (u32)(uintptr_t)&SndInitH_0802C40D; // +76  sub_0802C40C | Thumb
    r1[28] = (u32)(uintptr_t)&SndInitH_0802D84D; // +112 sub_0802D84C | Thumb
    r1[29] = (u32)(uintptr_t)&SndInitH_0802C391; // +116 sub_0802C390 | Thumb
    r1[30] = (u32)(uintptr_t)&SndInitH_0802C991; // +120 sub_0802C990 | Thumb
    r1[31] = (u32)(uintptr_t)&SndInitH_0802C11D; // +124 sub_0802C11C | Thumb
    *(volatile u32 *)((volatile u8 *)r1 + 128) = (u32)(uintptr_t)&SndInitH_0802CD59;
    *(volatile u32 *)((volatile u8 *)r1 + 132) = (u32)(uintptr_t)&SndInitH_0802CE21;
    r4[7] = (u32)(uintptr_t)r5;  // +28 = voice struct
    r4[10] = (u32)(uintptr_t)&SndInitH_0802D035; // +40
    r4[11] = (u32)(uintptr_t)&SndInitH_0802CF7D; // +44
    r4[12] = (u32)(uintptr_t)&SndInitH_0802CED5; // +48
    // Defect B. ROM +0x80..+0x86 is
    //   ldr r0,_0802C890 / movs r1,#0 / strb r0,[r4,#12] / str r1,[sp]
    // — a pool-loaded zero for the +12 byte, then a SECOND zero for the
    // CpuSet source slot, with both materialisations ahead of both stores.
    // A plain constant 0 is always folded to `movs` by reload, so the byte
    // value is spelled as the address of SoundVoiceInitZero, an assembler
    // absolute symbol of value 0: a CONST holding a SYMBOL_REF is the one
    // shape that reaches the pool. Copying the CpuSet zero through `slot`
    // rather than passing `&zq` directly is what keeps the second zero a
    // separate materialisation in the ROM's order.
    {
        u32 slot;
        u32 bz = (u32)(uintptr_t)&SoundVoiceInitZero;
        u32 zq = 0;
        ((volatile u8 *)r4)[12] = (u8)bz; // +12 byte
        slot = zq;
        sub_0802D974(&slot, (void *)r5, 0x05000040u); // CpuSet fill 64 words @r5
    }
    r5[1] = 1;
    r5[28] = 17;
    // The ROM walks ONE pointer through the six byte stores and reaches the
    // last two by offset: `adds r1,#36` lands on r5+192, then `strb [r1,#1]`
    // and `strb [r1,#28]`. Six independent `r5[n] =` stores make agbcc rebase
    // on r5 for each one and emit a 6-instruction chain (+37/+27 walks) that
    // lands on r5+193 and r5+220 instead. Same bytes written, wrong form.
    {
        volatile u8 *p = r5 + 65;
        p[0] = 2;
        p += 27; p[0] = 34;
        p += 37; p[0] = 3;
        p += 27; p[0] = 68;
        p += 36; p[1] = 4;
        p[28] = 136;
    }
    r4[0] = r6; // restore (net zero; kept for ISR-visibility parity)
}
void _0802C780(void *s) __attribute__((alias("SoundVoiceInit")));
void sub_0802C780(void *s) __attribute__((alias("SoundVoiceInit")));
void SoundStateInit(void *state) {
    volatile u8 *r5 = (volatile u8 *)state;
    *(volatile u32 *)r5 = 0;
    if (*(volatile u32 *)0x040000C4u & 0x02000000u)
        *(volatile u32 *)0x040000C4u = 0x84400004u; // DMA1 re-arm prime
    *(volatile u16 *)0x040000C6u = 1024;  // 128<<3
    *(volatile u16 *)0x04000084u = 143;
    *(volatile u16 *)0x04000082u = 0x0B0Eu;
    *(volatile u8 *)0x04000089u =
        (u8)((*(volatile u8 *)0x04000089u & 63) | 64); // SOUNDCNT_X bits
    *(volatile u32 *)0x040000BCu = (u32)(uintptr_t)(r5 + 848); // DMA1SAD
    *(volatile u32 *)0x040000C0u = 0x040000A0u; // DMA1DAD = FIFOA
    *(volatile u32 *)0x03007FF0u = (u32)(uintptr_t)r5; // root cell
    u32 zero = 0;
    sub_0802D974(&zero, (void *)r5, 0x05000260u); // CpuSet fill
    r5[6] = 8;
    r5[7] = 15;
    *(volatile u32 *)(r5 + 56) = 0x0802C191u;
    *(volatile u32 *)(r5 + 40) = 0x0802D96Du;
    *(volatile u32 *)(r5 + 44) = 0x0802D96Du;
    *(volatile u32 *)(r5 + 48) = 0x0802D96Du;
    *(volatile u32 *)(r5 + 60) = 0x0802D96Du;
    sub_0802BCB4((void *)0x0203EBB0u);
    *(volatile u32 *)(r5 + 52) = 0x0203EBB0u;
    sub_0802C990(0x00040000u); // 128<<11, as in asm (aliased to SoundControlInit below)
    *(volatile u32 *)r5 = 0x68736D53u; // "Smsh" magic published last
}
void _0802C8C4(void *s) __attribute__((alias("SoundStateInit")));
void sub_0802C8C4(void *s) __attribute__((alias("SoundStateInit")));

// TODO: remaining large pure-Thumb units still unsupported stay asm/
// - sound_d034.s PSG 0x02D034 540L (4× channel envelope/timer, REG_SOUND1CNT etc. — needs PSG struct widths)
// - sound_beb4 full 0x50 stride walk beyond trampoline, sound_seq walkers _0802B718/B74C full Smsh loop,
//   sound_more/followon bank helpers 0x02C614/548, sound_note_on/start/stop/tick/voice_* cluster,
//   d9b0-da/dd BIOS timing helpers. No fake aliases added for these interiors.
// Mixer tail beyond _0802BC4C leaf and mixed-ISA mixer body 0x02B888–0x02BC28 remain separate (copy contract at 0x03000170 unproven).
