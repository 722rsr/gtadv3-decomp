// ============================================================================
// agbmain_session.c — C lift of the agbmain.s session-cluster gaps:
//   _080006A4  soft-IRQ pump (16 B)
//   _080006BC  session-exit teardown
//   _0800070C  session-word reader / serial state machine (0x80 B)
//   _08000848  packet builder (0xAC B)
//   _080008F4  packet verify/receive side (0x108 B)
//
// Transcribed instruction-for-instruction from asm/agbmain.s. Session block
// EWRAM base = 0x0203EF90 (EF90); RX record area = [[EF90+0x2C]]; staging
// copy runs at 0x0203F110 (see asm/agbmain.s).
//
// Field map (EF90 offsets): +0 state word low byte (kicker gate), +1 FSM
// state, +2 status byte, +3 valid-record bits, +4 packet-ready, +6 SIOCNT
// bit snapshot / id nibble, +7 burst flag, +8 retry counter, +0x14 arming
// counter, +0x1C TX packet pointer, +0x2C RX record-area pointer.
// ============================================================================

#include "gba/types.h"

// ---- extern callees --------------------------------------------------------
extern u32 _0802DDC8(void *a);                   // 0x0802DDC8 handshake probe (asm): r0=F150, returns probe word
extern void _0802D974(const void *a, void *b, u32 c); // CpuSet (bios_wrappers.c)

// Hardware registers (byte-exact pool words):
#define AG_IMEM   ((volatile u16 *)(uintptr_t)0x04000208)  // IME
#define AG_IE     ((volatile u16 *)(uintptr_t)0x04000200)  // IE
#define AG_SIOCNT ((volatile u16 *)(uintptr_t)0x04000128)
#define AG_TM3CNT ((volatile u16 *)(uintptr_t)0x0400010C)  // SIOCNT-0x1C
#define AG_DMA3S  ((volatile u32 *)(uintptr_t)0x040000D4)

// Session block (byte-exact pools):
#define EF90 ((volatile u8 *)(uintptr_t)0x0203EF90)
// Session-block +6 byte, spelled as a base+member access: agbcc then emits
// `ldr rX,[pc]` for the 0x0203EF90 pool word and `ldrb rY,[rX,#6]`, which is
// the ROM form. A subscripted constant base folds to 0x0203EF96 + `#0` instead.
typedef struct { u8 ef90_lo[6]; u8 ef90_f6; } EF90_Blk;
#define EF90_F6 (((EF90_Blk *)(uintptr_t)0x0203EF90)->ef90_f6)
#define F110 ((volatile u8 *)(uintptr_t)0x0203F110)  // Verify probe arg / thunk copy dst
#define F150 ((volatile u8 *)(uintptr_t)0x0203F110)  // legacy alias (Verify path)

typedef struct {
    u8 bytes[0x1C];
    volatile u8 * volatile tx_packet;
} SessionPacketState;
typedef struct { volatile u8 reserved[3]; volatile u8 byte3; } PacketFlagSource;
#define F150_PKT_SOURCE ((volatile PacketFlagSource *)(uintptr_t)0x0203F150)

// ----------------------------------------------------------------------------
// _080008F4 — forward decl (defined below).
int SessionVerify_080008F4(void *buf);

// ----------------------------------------------------------------------------
// _080006A4 — soft-IRQ pump. If EF90[0] != 0, set bit7 of EF90[6].
//
// Two measured levers:
//   * the mask must be a NAMED local. Inlining `0x80 | r1[6]` materialises
//     `movs r0,#128` AFTER the `ldrb`; binding it hoists the constant above
//     the load, which is the ROM's order. Same lever as
//     StateA_GetRecordHalfwordInverted in src/state_block_a.c.
//   * the EF90 base must be r1 and the loaded byte r2. Left unpinned agbcc
//     gives base r2 and the byte r1 (measured candidate 16/24, every operand
//     register off by one). The `__asm__("r1")` pin is LOAD-BEARING: the
//     unpinned body is r2/r1, the pinned body is r1/r2 = the ROM's.
void SessionPump_080006A4(void) {
    register volatile u8 *r1 __asm__("r1") = EF90;
    if (r1[0] != 0) {
        u8 mask = 0x80;
        r1[6] = (u8)(mask | r1[6]);
    }
}
void _080006A4(void) __attribute__((alias("SessionPump_080006A4")));
void sub_080006A4(void) __attribute__((alias("SessionPump_080006A4")));
void SessionPump(void) __attribute__((alias("SessionPump_080006A4")));

// ----------------------------------------------------------------------------
// _080006BC — session-exit teardown:
//   IME=0; IE &= 0xFF3F; IF=1 (ack all); SIOCNT=0x2003; 0x0400010C=0xABFB
//   (via word store at SIOCNT-0x1C); TM3CNT_H (SIOCNT+0xF6-0x1C+...) = 0xC0
//   halfword; EF90[6] &= 0x7F.
void SessionExit_080006BC(void) {
    *AG_IMEM = 0;
    u16 ie = *AG_IE;
    *AG_IE = (u16)(ie & 0xFF3Fu);
    *AG_IMEM = 1;
    *AG_SIOCNT = 0x2003;
    // subs r1,#0x1c from SIOCNT = 0x0400010C, word store 0xABFB
    *(volatile u32 *)(uintptr_t)0x0400010C = 0xABFBu;
    // adds r1,#0xf6 from 0x0400010C = 0x04000202, halfword 0xC0
    *(volatile u16 *)(uintptr_t)0x04000202 = 0xC0;
    // EF90[6] &= 0x7F, with the base in r1 and the mask in r0 (see above)
    {
        register volatile u8 *b __asm__("r1") = (volatile u8 *)(uintptr_t)0x0203EF90;
        register u8 m __asm__("r0") = 0x7F;
        m = (u8)(m & b[6]);
        b[6] = m;
    }
}
void _080006BC(void) __attribute__((alias("SessionExit_080006BC")));
void sub_080006BC(void) __attribute__((alias("SessionExit_080006BC")));

// ----------------------------------------------------------------------------
// _0800070C(buf) -> u16 — session-word reader (per-frame tick).
// The initial 32-bit read spans SIOCNT and the adjacent timer counter, as in
// the ROM's `ldr r7,[r6]`; its low half drives the arming tests and return.
__attribute__((naked)) u16 SessionRead_0800070C(void *buf) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Lsession070c_70c:\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov ip, r0\n"
        "ldr r6, .Lsession070c_728\n"
        "ldr r7, [r6]\n"
        "ldr r0, .Lsession070c_72c\n"
        "ldrb r1, [r0, #1]\n"
        "adds r5, r0, #0\n"
        "cmp r1, #1\n"
        "beq.n .Lsession070c_794\n"
        "cmp r1, #1\n"
        "bgt.n .Lsession070c_730\n"
        "cmp r1, #0\n"
        "beq.n .Lsession070c_736\n"
        "b.n .Lsession070c_7da\n"
        ".align 2, 0\n"
        ".Lsession070c_728: .word 0x04000128\n"
        ".Lsession070c_72c: .word 0x0203EF90\n"
        ".Lsession070c_730:\n"
        "cmp r1, #2\n"
        "beq.n .Lsession070c_7d4\n"
        "b.n .Lsession070c_7da\n"
        ".Lsession070c_736:\n"
        "movs r1, #0x30\n"
        "adds r0, r7, #0\n"
        "ands r0, r1\n"
        "cmp r0, #0\n"
        "bne.n .Lsession070c_790\n"
        "movs r0, #0x88\n"
        "adds r4, r7, #0\n"
        "ands r4, r0\n"
        "cmp r4, #8\n"
        "bne.n .Lsession070c_7da\n"
        "movs r1, #4\n"
        "adds r0, r7, #0\n"
        "ands r0, r1\n"
        "lsls r0, r0, #24\n"
        "lsrs r1, r0, #24\n"
        "cmp r1, #0\n"
        "bne.n .Lsession070c_790\n"
        "ldr r0, [r5, #0x14]\n"
        "cmp r0, #12\n"
        "bne.n .Lsession070c_790\n"
        "ldr r3, .Lsession070c_7b8\n"
        "strh r1, [r3, #0]\n"
        "ldr r2, .Lsession070c_7bc\n"
        "ldrh r1, [r2, #0]\n"
        "ldr r0, .Lsession070c_7c0\n"
        "ands r0, r1\n"
        "strh r0, [r2, #0]\n"
        "ldrh r0, [r2, #0]\n"
        "movs r1, #0x40\n"
        "orrs r0, r1\n"
        "strh r0, [r2, #0]\n"
        "movs r0, #1\n"
        "strh r0, [r3, #0]\n"
        "ldrb r1, [r6, #1]\n"
        "movs r0, #0x41\n"
        "negs r0, r0\n"
        "ands r0, r1\n"
        "strb r0, [r6, #1]\n"
        "ldr r1, .Lsession070c_7c4\n"
        "ldr r0, .Lsession070c_7c8\n"
        "str r0, [r1, #0]\n"
        "adds r1, #0xf6\n"
        "movs r0, #0xc0\n"
        "strh r0, [r1, #0]\n"
        "strb r4, [r5, #0]\n"
        ".Lsession070c_790:\n"
        "movs r0, #1\n"
        "strb r0, [r5, #1]\n"
        ".Lsession070c_794:\n"
        "ldr r1, .Lsession070c_7cc\n"
        "movs r0, #0xf0\n"
        "ldrb r2, [r1, #2]\n"
        "ands r0, r2\n"
        "cmp r0, #0\n"
        "beq.n .Lsession070c_7d4\n"
        "movs r0, #0x40\n"
        "ldrb r2, [r1, #6]\n"
        "ands r0, r2\n"
        "cmp r0, #0\n"
        "bne.n .Lsession070c_7d4\n"
        "ldrb r0, [r1, #8]\n"
        "cmp r0, #7\n"
        "bhi.n .Lsession070c_7d0\n"
        "adds r0, #1\n"
        "strb r0, [r1, #8]\n"
        "b.n .Lsession070c_7d4\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lsession070c_7b8: .word 0x04000208\n"
        ".Lsession070c_7bc: .word 0x04000200\n"
        ".Lsession070c_7c0: .word 0x0000FF7F\n"
        ".Lsession070c_7c4: .word 0x0400010C\n"
        ".Lsession070c_7c8: .word 0x0000ABFB\n"
        ".Lsession070c_7cc: .word 0x0203EF90\n"
        ".Lsession070c_7d0:\n"
        "movs r0, #2\n"
        "strb r0, [r1, #1]\n"
        ".Lsession070c_7d4:\n"
        "mov r0, ip\n"
        "bl sub_08008F4\n"
        ".Lsession070c_7da:\n"
        "ldr r2, .Lsession070c_810\n"
        "ldrb r0, [r2, #11]\n"
        "adds r0, #1\n"
        "strb r0, [r2, #11]\n"
        "ldrb r1, [r2, #6]\n"
        "movs r3, #16\n"
        "ands r3, r1\n"
        "ldrb r0, [r2, #3]\n"
        "orrs r3, r0\n"
        "movs r0, #32\n"
        "ands r0, r1\n"
        "orrs r3, r0\n"
        "movs r0, #0x40\n"
        "ands r0, r1\n"
        "orrs r3, r0\n"
        "ldrb r1, [r2, #2]\n"
        "lsrs r0, r1, #4\n"
        "lsls r0, r0, #8\n"
        "adds r5, r2, #0\n"
        "ldrb r2, [r5, #0]\n"
        "cmp r2, #8\n"
        "bne.n .Lsession070c_814\n"
        "movs r1, #0x80\n"
        "orrs r1, r0\n"
        "orrs r1, r3\n"
        "b.n .Lsession070c_818\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lsession070c_810: .word 0x0203EF90\n"
        ".Lsession070c_814:\n"
        "adds r1, r3, #0\n"
        "orrs r1, r0\n"
        ".Lsession070c_818:\n"
        "ldrb r0, [r5, #7]\n"
        "cmp r0, #0\n"
        "beq.n .Lsession070c_824\n"
        "movs r0, #0x80\n"
        "lsls r0, r0, #5\n"
        "orrs r1, r0\n"
        ".Lsession070c_824:\n"
        "ldrb r5, [r5, #8]\n"
        "lsrs r0, r5, #3\n"
        "lsls r2, r0, #15\n"
        "lsls r0, r7, #26\n"
        "lsrs r0, r0, #30\n"
        "cmp r0, #3\n"
        "bls.n .Lsession070c_83c\n"
        "movs r0, #0x80\n"
        "lsls r0, r0, #6\n"
        "orrs r0, r2\n"
        "orrs r0, r1\n"
        "b.n .Lsession070c_840\n"
        ".Lsession070c_83c:\n"
        "adds r0, r1, #0\n"
        "orrs r0, r2\n"
        ".Lsession070c_840:\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r1}\n"
        "bx r1\n"
        "movs r0, r0\n"
        ".syntax divided\n"
    );
}

u16 _0800070C(void *a) __attribute__((alias("SessionRead_0800070C")));
u16 SessionReadWord(void *a) __attribute__((alias("SessionRead_0800070C")));
u16 sub_0800070C(void *a) __attribute__((alias("SessionRead_0800070C")));

// ----------------------------------------------------------------------------
// _08000848(payload, flag) — packet builder.
//   pkt = [EF90+0x1C]:
//     pkt[1] = (pkt[1] & ~0x10) | ((flag & 1) << 4)
//     pkt[1] = (pkt[1] & ~0x20) | ((F150[3] & 1) << 5)
//     pkt[1] = (pkt[1] & ~0x40) | (((EF90[6]>>6) & 1) << 6)
//     pkt[0] = EF90[11]
//     pkt[1] = (pkt[1] & ~0x0F) | (((EF90[2] & 0xF) ^ EF90[3]) & 0x0F)
//     u16[pkt+2] = 0
//     CpuSet(payload, pkt+4, 0x04000004)  — 4 words of payload from r0
//     checksum = ~(sum of first 10 halfwords) - 12 -> u16[pkt+2]
//     EF90[4] = 1
// Masks from asm negs: -17=~0x10, -33=~0x20, -65=~0x40, -16=~0x0F.
__attribute__((naked)) void PacketBuild_08000848(void *payload, u32 flag)
{
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "movs r6, #0\n"
        "ldr r4, .L0848_p0\n"
        "ldr r3, [r4, #28]\n"
        "movs r2, #1\n"
        "ands r1, r2\n"
        "lsls r1, r1, #4\n"
        "movs r2, #17\n"
        "negs r2, r2\n"
        "ldrb r5, [r3, #1]\n"
        "ands r2, r5\n"
        "orrs r2, r1\n"
        "strb r2, [r3, #1]\n"
        "ldr r5, [r4, #28]\n"
        "ldr r1, .L0848_p1\n"
        "movs r3, #1\n"
        "adds r2, r3, #0\n"
        "ldrb r1, [r1, #3]\n"
        "ands r2, r1\n"
        "lsls r2, r2, #5\n"
        "movs r1, #33\n"
        "negs r1, r1\n"
        "ldrb r7, [r5, #1]\n"
        "ands r1, r7\n"
        "orrs r1, r2\n"
        "strb r1, [r5, #1]\n"
        "ldr r2, [r4, #28]\n"
        "ldrb r5, [r4, #6]\n"
        "lsls r1, r5, #25\n"
        "lsrs r1, r1, #31\n"
        "ands r3, r1\n"
        "lsls r3, r3, #6\n"
        "movs r1, #65\n"
        "negs r1, r1\n"
        "ldrb r7, [r2, #1]\n"
        "ands r1, r7\n"
        "orrs r1, r3\n"
        "strb r1, [r2, #1]\n"
        "ldr r2, [r4, #28]\n"
        "ldrb r1, [r4, #11]\n"
        "strb r1, [r2, #0]\n"
        "ldr r3, [r4, #28]\n"
        "ldrb r2, [r4, #2]\n"
        "lsls r1, r2, #28\n"
        "lsrs r1, r1, #28\n"
        "ldrb r2, [r4, #3]\n"
        "eors r2, r1\n"
        "movs r1, #15\n"
        "ands r2, r1\n"
        "movs r1, #16\n"
        "negs r1, r1\n"
        "ldrb r5, [r3, #1]\n"
        "ands r1, r5\n"
        "orrs r1, r2\n"
        "strb r1, [r3, #1]\n"
        "ldr r1, [r4, #28]\n"
        "strh r6, [r1, #2]\n"
        "ldr r1, [r4, #28]\n"
        "adds r1, #4\n"
        "ldr r2, .L0848_p2\n"
        "bl 0x0802D974\n"
        "movs r1, #0\n"
        "ldr r0, [r4, #28]\n"
        ".L0848_sum_loop:\n"
        "ldrh r7, [r0, #0]\n"
        ".hword 0x19be\n"
        "adds r0, #2\n"
        "adds r1, #1\n"
        "cmp r1, #9\n"
        "bls.n .L0848_sum_loop\n"
        "ldr r0, [r4, #28]\n"
        "mvns r1, r6\n"
        "subs r1, #12\n"
        "strh r1, [r0, #2]\n"
        "movs r0, #1\n"
        "strb r0, [r4, #4]\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L0848_p0: .4byte 0x0203EF90\n"
        ".L0848_p1: .4byte 0x0203F150\n"
        ".L0848_p2: .4byte 0x04000004\n"
        ".syntax divided\n"
    );
}
void _08000848(void *a, u32 b) __attribute__((alias("PacketBuild_08000848")));
void sub_08000848(void *a, u32 b) __attribute__((alias("PacketBuild_08000848")));
void PacketBuild(void *a, u32 b) __attribute__((alias("PacketBuild_08000848")));

// ----------------------------------------------------------------------------
// _080008F4(buf) — verify/receive side. For records 3..0 at [EF90+0x2C]
// (stride 24, descending): sum 10 halfwords, sign-extended; sum == -13 marks
// the record valid (bit r3 in EF90+3), merges its bit5 into EF90+6 low nibble,
// copies 16 bytes from record+4 to buf+r3*16, then zeroes record+4
// (CpuSet fill 0x05000004). Skipped entirely when the handshake probe
// (_0802DDC8(F150)<<24)==0. Tail rebuilds EF90+2 status nibble packing and
// returns EF90[3]. Masks from asm negs: -16=~0x0F, -65=~0x40, -33=~0x20.
__attribute__((naked)) int SessionVerify_080008F4(void *buf) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, sl\n"
        "mov r6, r9\n"
        "mov r5, r8\n"
        "push {r5, r6, r7}\n"
        "sub sp, #12\n"
        "mov r9, r0\n"
        "ldr r0, .Lsession08f4_a14\n"
        "movs r4, #0\n"
        "str r4, [sp]\n"
        "bl sub_0802DDC8\n"
        "lsls r0, r0, #24\n"
        "ldr r2, .Lsession08f4_a18\n"
        "strb r4, [r2, #3]\n"
        "movs r1, #0x41\n"
        "negs r1, r1\n"
        "ldrb r3, [r2, #6]\n"
        "ands r1, r3\n"
        "strb r1, [r2, #6]\n"
        "cmp r0, #0\n"
        "beq.n .Lsession08f4_9a2\n"
        "movs r3, #3\n"
        "add r4, sp, #4\n"
        "mov r8, r4\n"
        "adds r5, r2, #0\n"
        "movs r6, #13\n"
        "negs r6, r6\n"
        "mov sl, r6\n"
        ".Lsession08f4_92e:\n"
        "lsls r0, r3, #1\n"
        "adds r0, r0, r3\n"
        "lsls r0, r0, #3\n"
        "ldr r1, [r5, #0x2c]\n"
        "adds r7, r1, r0\n"
        "movs r0, #0\n"
        "movs r2, #0\n"
        "subs r1, r3, #1\n"
        "str r1, [sp, #8]\n"
        "adds r1, r7, #0\n"
        ".Lsession08f4_942:\n"
        "ldrh r4, [r1, #0]\n"
        "adds r0, r4, r0\n"
        "adds r1, #2\n"
        "adds r2, #1\n"
        "cmp r2, #9\n"
        "bls.n .Lsession08f4_942\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #16\n"
        "adds r4, r7, #4\n"
        "cmp r0, sl\n"
        "bne.n .Lsession08f4_98e\n"
        "movs r0, #1\n"
        "lsls r0, r3\n"
        "ldrb r6, [r5, #3]\n"
        "orrs r0, r6\n"
        "strb r0, [r5, #3]\n"
        "ldrb r2, [r5, #6]\n"
        "lsls r1, r2, #28\n"
        "lsrs r1, r1, #28\n"
        "ldrb r6, [r7, #1]\n"
        "lsls r0, r6, #26\n"
        "lsrs r0, r0, #31\n"
        "lsls r0, r3\n"
        "orrs r0, r1\n"
        "movs r1, #15\n"
        "ands r0, r1\n"
        "movs r6, #16\n"
        "negs r6, r6\n"
        "adds r1, r6, #0\n"
        "ands r2, r1\n"
        "orrs r2, r0\n"
        "strb r2, [r5, #6]\n"
        "lsls r1, r3, #4\n"
        "add r1, r9\n"
        "adds r0, r4, #0\n"
        "ldr r2, .Lsession08f4_a1c\n"
        "bl sub_0802D974\n"
        ".Lsession08f4_98e:\n"
        "movs r0, #0\n"
        "str r0, [sp, #4]\n"
        "mov r0, r8\n"
        "adds r1, r4, #0\n"
        "ldr r2, .Lsession08f4_a20\n"
        "bl sub_0802D974\n"
        "ldr r3, [sp, #8]\n"
        "cmp r3, #0\n"
        "bge.n .Lsession08f4_92e\n"
        ".Lsession08f4_9a2:\n"
        "ldr r3, .Lsession08f4_a18\n"
        "ldrb r2, [r3, #2]\n"
        "lsls r0, r2, #28\n"
        "lsrs r0, r0, #28\n"
        "ldrb r6, [r3, #3]\n"
        "orrs r0, r6\n"
        "movs r1, #15\n"
        "ands r0, r1\n"
        "movs r1, #16\n"
        "negs r1, r1\n"
        "ands r1, r2\n"
        "orrs r1, r0\n"
        "lsrs r2, r1, #4\n"
        "lsls r0, r1, #28\n"
        "lsrs r0, r0, #28\n"
        "orrs r2, r0\n"
        "lsls r2, r2, #4\n"
        "movs r4, #15\n"
        "ands r4, r1\n"
        "orrs r4, r2\n"
        "strb r4, [r3, #2]\n"
        "movs r1, #1\n"
        "adds r0, r1, #0\n"
        "ands r0, r6\n"
        "adds r5, r3, #0\n"
        "cmp r0, #0\n"
        "beq.n .Lsession08f4_a4e\n"
        "ldrb r0, [r5, #0]\n"
        "cmp r0, #8\n"
        "bne.n .Lsession08f4_a24\n"
        "movs r0, #3\n"
        "ands r0, r6\n"
        "cmp r0, #0\n"
        "beq.n .Lsession08f4_9f8\n"
        "lsls r0, r6, #24\n"
        "lsrs r0, r0, #24\n"
        "lsrs r1, r4, #4\n"
        "cmp r0, r1\n"
        "bne.n .Lsession08f4_9f8\n"
        "movs r0, #16\n"
        "ldrb r1, [r5, #6]\n"
        "orrs r0, r1\n"
        "strb r0, [r5, #6]\n"
        ".Lsession08f4_9f8:\n"
        "ldrb r4, [r5, #6]\n"
        "lsls r2, r4, #28\n"
        "lsrs r2, r2, #28\n"
        "movs r0, #14\n"
        "ldrb r6, [r5, #2]\n"
        "lsrs r3, r6, #4\n"
        "adds r1, r0, #0\n"
        "ands r1, r2\n"
        "ands r0, r3\n"
        "cmp r1, r0\n"
        "bne.n .Lsession08f4_a3a\n"
        "movs r0, #0x40\n"
        "orrs r0, r4\n"
        "b.n .Lsession08f4_a38\n"
        ".align 2, 0\n"
        ".Lsession08f4_a14: .word 0x0203F110\n"
        ".Lsession08f4_a18: .word 0x0203EF90\n"
        ".Lsession08f4_a1c: .word 0x04000004\n"
        ".Lsession08f4_a20: .word 0x05000004\n"
        ".Lsession08f4_a24:\n"
        "ldrb r2, [r7, #1]\n"
        "lsls r0, r2, #25\n"
        "lsrs r0, r0, #31\n"
        "ands r1, r0\n"
        "lsls r1, r1, #6\n"
        "movs r0, #0x41\n"
        "negs r0, r0\n"
        "ldrb r3, [r5, #6]\n"
        "ands r0, r3\n"
        "orrs r0, r1\n"
        ".Lsession08f4_a38:\n"
        "strb r0, [r5, #6]\n"
        ".Lsession08f4_a3a:\n"
        "ldrb r7, [r7, #1]\n"
        "lsls r1, r7, #27\n"
        "lsrs r1, r1, #31\n"
        "lsls r1, r1, #5\n"
        "movs r0, #0x21\n"
        "negs r0, r0\n"
        "ldrb r4, [r5, #6]\n"
        "ands r0, r4\n"
        "orrs r0, r1\n"
        "strb r0, [r5, #6]\n"
        ".Lsession08f4_a4e:\n"
        "ldrb r0, [r5, #3]\n"
        "add sp, #12\n"
        "pop {r3, r4, r5}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "mov sl, r5\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r1}\n"
        "bx r1\n"
        ".syntax divided\n"
    );
}

int _080008F4(void *a) __attribute__((alias("SessionVerify_080008F4")));
int sub_080008F4(void *a) __attribute__((alias("SessionVerify_080008F4")));
