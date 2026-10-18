#include "gtadv/sound.h"
#include "gba/types.h"

// ============================================================================
// sound_voice_apply.s 0x0802C160 — voice envelope apply (uses r4/r5 caller
// state: r4 = voice record, r5 = param source).
// File-scope fixed register declarations keep r4 and r5 out of the saved-register
// mask so old_agbcc emits the 48-byte frame-free leaf straight.
// ============================================================================
register u8 *g_voice_2C160_r4 __asm__("r4");
register u8 *g_param_2C160_r5 __asm__("r5");

void VoiceEnvelopeApply_2C160(void) {
    register u32 r1 __asm__("r1") = g_voice_2C160_r4[18];
    register s32 off __asm__("r0") = 20;
    register s32 r2 __asm__("r2");
    register s32 r3 __asm__("r3");
    // ROM `movs r0, #20 / ldrsb r2, [r4, r0]` (index != destination) is
    // unreachable from C with old_agbcc: the constant path of
    // *extendqisi2_insn reuses the destination as the index, and REG+REG
    // addresses never form. See docs/findings/byte_ldrsb_register_index.md.
    __asm__ volatile ("ldrsb %0, [%1, %2]"
                      : "=r"(r2) : "r"(g_voice_2C160_r4), "r"(off));
    r3 = 128;
    __asm__ volatile ("" : "+r"(r3));
    r3 += r2;
    r3 *= (s32)r1;
    off = g_param_2C160_r5[16];
    __asm__ volatile ("" : "+r"(off));
    off *= r3;
    off >>= 14;
    if ((u32)off > 255) off = 255;
    g_voice_2C160_r4[2] = (u8)off;
    r3 = 127 - r2;
    r3 *= (s32)r1;
    off = g_param_2C160_r5[17];
    __asm__ volatile ("" : "+r"(off));
    off *= r3;
    off >>= 14;
    if ((u32)off > 255) off = 255;
    g_voice_2C160_r4[3] = (u8)off;
}
void _0802C160(void) __attribute__((alias("VoiceEnvelopeApply_2C160")));
void sub_0802C160(void) __attribute__((alias("VoiceEnvelopeApply_2C160")));

// Bounded non-noop leaf: sub_0802BE14 (mov ip,lr; bl 0x0802BCE8; strb r3,[r1,#15]; orrs)
// VMA 0x0802BE14–0x0802BE28 (20 B) in asm/sound_channel_cluster.s:0x02BC84–0x02BE78.
// Pure-Thumb, complete boundary: mov ip,lr at 0x02BE14 / bl 0x0802BCE8 (4 B) / strb r3,[r1,#15] (2 B) / ldrb r3,[r1,#0] (2 B) / movs r2,#12 / orrs r3,r2 / strb r3,[r1,#0] / bx ip at 0x02BE28, no push/pop (ip saves lr).
// Exact per objdump --adjust-vma=0x0802BE14: 4664 mov ip,lr / f7ff fffe bl 0x0802BCE8 / 707b strb r3,[r1,#15] / 780b ldrb r3,[r1,#0] / 4a02 movs r2,#12 / 4313 orrs r3,r2 / 700b strb r3,[r1,#0] / 4674 bx ip
// Helper at 0x0802BCE8: ldr r2,[r1,#64] (u32 cur), adds r3,r2,#1, str r3,[r1,#64], ldrb r3,[r2,#0] (u8), b 0x02BCCE
// Helper return in r3 (nonstandard r3 ABI): BCCE validates via lsrs #25 / ldr 0x080614E0 / cmp / lsrs #14, clears r3 to 0 on else, then bx lr (pop {r0}).
// Caller ABI: channel in r1 (nonstandard r1), r0 ignored, r2/r3 temps. Proven via ROM table at 0x061520 (0x0802BE15 thumb entry at file 0x061520: 15 BE 02 08) within 0x061520 8-entry table, dispatched via r1 channel.
// Do not guess voice layout beyond +15 (u8) / +0 (u8) directly via strb/ldrb/orrs, proven via objdump.

void _0802BE14(void *a, void *b) __attribute__((alias("SoundBE14_Store")));
void sub_0802BE14(void *a, void *b) __attribute__((alias("SoundBE14_Store")));
__attribute__((naked)) void SoundBE14_Store(void *a, void *chan) {
    register u32 frame __asm__("ip");
    register u32 value __asm__("r3");
    register volatile u8 *channel __asm__("r1") = (volatile u8 *)chan;
    __asm__ volatile(".syntax unified\n\tmov %0, lr\n\tbl _0802BCE8\n\t.syntax divided"
                     : "=r"(frame), "=r"(value) : : "lr", "cc", "memory");
    *(volatile u8 *)(channel + 15) = (u8)value;
    register u32 flags __asm__("r3") = channel[0];
    register u32 bits __asm__("r2") = 12;
    __asm__ volatile("" : "+r"(flags), "+r"(bits) : : "cc", "memory");
    flags |= bits;
    channel[0] = (u8)flags;
    __asm__ volatile("bx %0\n\t.short 0x0000" : : "r"(frame) : "cc");
}
