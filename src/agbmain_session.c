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
#ifdef __APPLE__
__attribute__((weak)) u32 _0802DDC8(void *a) { (void)a; return 0; }
__attribute__((weak)) void  _0802D974(const void *a, void *b, u32 c) { (void)a; (void)b; (void)c; }
#else
extern u32 _0802DDC8(void *a);                   // 0x0802DDC8 handshake probe (asm): r0=F150, returns probe word
extern void _0802D974(const void *a, void *b, u32 c); // CpuSet (bios_wrappers.c)
#endif

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
#ifndef __APPLE__
void _080006A4(void) __attribute__((alias("SessionPump_080006A4")));
void sub_080006A4(void) __attribute__((alias("SessionPump_080006A4")));
void SessionPump(void) __attribute__((alias("SessionPump_080006A4")));
#endif

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
#ifndef __APPLE__
void _080006BC(void) __attribute__((alias("SessionExit_080006BC")));
void sub_080006BC(void) __attribute__((alias("SessionExit_080006BC")));
#endif

// ----------------------------------------------------------------------------
// _0800070C(buf) -> u16 — session-word reader (per-frame tick).
// The initial 32-bit read spans SIOCNT and the adjacent timer counter, as in
// the ROM's `ldr r7,[r6]`; its low half drives the arming tests and return.
u16 SessionRead_0800070C(void *buf) {
    volatile u32 *siocnt = (volatile u32 *)(uintptr_t)0x04000128;
    u32 r7 = *siocnt;
    volatile u8 *r5 = EF90;
    u8 state = r5[1];
    int run_watchdog = 0;

    if (state == 1) {
        run_watchdog = 1;
    } else if (state == 2) {
        SessionVerify_080008F4(buf);
    } else if (state == 0) {
        if ((r7 & 0x30) != 0) {
            run_watchdog = 1;
        } else if ((r7 & 0x88) == 8) {
            if ((r7 & 4) != 0 || *(volatile u32 *)(r5 + 0x14) != 12) {
                run_watchdog = 1;
            } else {
                *AG_IMEM = 0;
                u16 ie = *AG_IE;
                *AG_IE = (u16)(ie & 0xFF7Fu);
                ie = *AG_IE;
                *AG_IE = (u16)(ie | 0x40u);
                *AG_IMEM = 1;

                volatile u8 *siocnt_hi = (volatile u8 *)(uintptr_t)0x04000129;
                *siocnt_hi = (u8)(*siocnt_hi & (u8)~0x41u);
                *(volatile u32 *)(uintptr_t)0x0400010C = 0xABFBu;
                // 0x04000202 is IF; this acknowledges Timer3 and serial.
                *(volatile u16 *)(uintptr_t)0x04000202 = 0xC0;
                r5[0] = (u8)(r7 & 0x88u);
                run_watchdog = 1;
            }
        }
    }

    if (state == 0 && run_watchdog)
        r5[1] = 1;

    if (run_watchdog) {
        // State 1 retries until the counter has already advanced past seven.
        if ((r5[2] & 0xF0u) == 0 && (r5[6] & 0x40u) == 0) {
            u8 retries = r5[8];
            if (retries > 7)
                r5[1] = 2;
            else
                r5[8] = (u8)(retries + 1);
        }
        SessionVerify_080008F4(buf);
    }

    r5[11] = (u8)(r5[11] + 1);
    u32 status = (u32)(r5[6] & 0x70u) | r5[3];
    status |= (u32)(r5[2] & 0xF0u) << 4;
    if (r5[0] == 8)
        status |= 0x80u;
    if (r5[7] != 0)
        status |= 0x1000u;
    status |= ((u32)r5[8] >> 3) << 15;
    if (((r7 << 26) >> 30) > 3)
        status |= 0x2000u;
    return (u16)status;
}
#ifndef __APPLE__
u16 _0800070C(void *a) __attribute__((alias("SessionRead_0800070C")));
u16 SessionReadWord(void *a) __attribute__((alias("SessionRead_0800070C")));
u16 sub_0800070C(void *a) __attribute__((alias("SessionRead_0800070C")));
#endif

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
void PacketBuild_08000848(void *payload, u32 flag) {
    register u32 sum __asm__("r6") = 0;
    register SessionPacketState *r4 __asm__("r4") =
        (SessionPacketState *)(uintptr_t)0x0203EF90;

    // The first packet pointer is held across its byte read and write.
    register volatile u8 *p0 __asm__("r3") = r4->tx_packet;
    {
        register u32 r1 __asm__("r1") = flag;
        register u32 r2 __asm__("r2") = 1;
        register u32 r5 __asm__("r5");
        r1 &= r2;
        r1 <<= 4;
        r2 = 17;
        r2 = 0 - r2;
        r5 = p0[1];
        r2 &= r5;
        r2 |= r1;
        p0[1] = (u8)r2;
    }

    volatile u8 *p1 = r4->tx_packet;
    u32 flag2 = ((u32)(F150_PKT_SOURCE->byte3 & 1u) << 5);
    u32 b = (p1[1] & ~0x20u) | flag2;
    p1[1] = (u8)b;
    volatile u8 *p2 = r4->tx_packet;
    u32 bit = ((u32)r4->bytes[6] << 25) >> 31;
    b = (p2[1] & ~0x40u) | (bit << 6);
    p2[1] = (u8)b;
    volatile u8 *p3 = r4->tx_packet;
    p3[0] = r4->bytes[11];
    volatile u8 *p4 = r4->tx_packet;
    u32 nib = ((u32)r4->bytes[2] << 28) >> 28;
    u32 x = (u32)r4->bytes[3] ^ nib;
    x &= 0xF;
    b = (p4[1] & ~0x0Fu) | x;
    p4[1] = (u8)b;

    volatile u8 *p5 = r4->tx_packet;
    *(volatile u16 *)(p5 + 2) = 0;
    p5 = r4->tx_packet;
    _0802D974(payload, (void *)(uintptr_t)(p5 + 4), 0x04000004u);

    volatile u16 *p = (volatile u16 *)(uintptr_t)r4->tx_packet;
    for (u32 i = 0; i <= 9; i++) {
        sum += p[i];
    }
    u16 ck = (u16)(~sum - 12);
    p5 = r4->tx_packet;
    *(volatile u16 *)(p5 + 2) = ck;
    r4->bytes[4] = 1;
}
#ifndef __APPLE__
void _08000848(void *a, u32 b) __attribute__((alias("PacketBuild_08000848")));
void sub_08000848(void *a, u32 b) __attribute__((alias("PacketBuild_08000848")));
void PacketBuild(void *a, u32 b) __attribute__((alias("PacketBuild_08000848")));
#endif

// ----------------------------------------------------------------------------
// _080008F4(buf) — verify/receive side. For records 3..0 at [EF90+0x2C]
// (stride 24, descending): sum 10 halfwords, sign-extended; sum == -13 marks
// the record valid (bit r3 in EF90+3), merges its bit5 into EF90+6 low nibble,
// copies 16 bytes from record+4 to buf+r3*16, then zeroes record+4
// (CpuSet fill 0x05000004). Skipped entirely when the handshake probe
// (_0802DDC8(F150)<<24)==0. Tail rebuilds EF90+2 status nibble packing and
// returns EF90[3]. Masks from asm negs: -16=~0x0F, -65=~0x40, -33=~0x20.
int SessionVerify_080008F4(void *buf) {
    u32 probe = _0802DDC8((void *)(uintptr_t)0x0203F110);
    volatile u8 *r5 = EF90;
    r5[3] = 0;
    r5[6] = (u8)(r5[6] & ~0x40u);

    if (((probe << 24) != 0)) {
        u8 *bufBase = (u8 *)buf;
        // descending r3 = 3..0 (asm: movs r3,#3; [sp+8]=r3-1; bge loop)
        for (int r3 = 3; r3 >= 0; r3--) {
            volatile u8 *recs = (volatile u8 *)(uintptr_t)*(volatile u32 *)(r5 + 0x2C);
            volatile u8 *r7 = recs + (u32)r3 * 24u;
            u32 sum = 0;
            volatile u16 *h = (volatile u16 *)(uintptr_t)r7;
            for (u32 i = 0; i <= 9; i++)
                sum += h[i];
            sum = (u32)(s32)(s16)(u16)sum;
            if ((s32)sum == -13) {
                // mark valid
                r5[3] = (u8)(r5[3] | (1u << (u32)r3));
                // merge id nibble into EF90+6 low nibble, high preserved
                u32 v6 = r5[6];
                u32 low = (v6 << 28) >> 28;
                u32 idn = ((u32)r7[1] << 26) >> 31;
                u32 merged = (u32)(((idn << (u32)r3) | low) & 0xFu);
                v6 = (v6 & 0xF0u) | merged;
                r5[6] = (u8)v6;
                // CpuSet(record+4 -> buf+r3*16, ctrl 0x04000004)
                u8 *dst = bufBase + ((u32)r3 << 4);
                _0802D974((const void *)(uintptr_t)(r7 + 4), (void *)(uintptr_t)dst, 0x04000004u);
            }
            // zero the consumed record payload (16 bytes at record+4)
            {
                u32 zero = 0;
                _0802D974((const void *)&zero, (void *)(uintptr_t)(r7 + 4), 0x05000004u);
            }
        }
    }

    // tail: pack EF90+2 nibble with EF90[3]
    {
        volatile u8 *b = EF90;
        u32 v2 = b[2];
        u32 low = (v2 << 28) >> 28;
        u32 v3 = b[3];
        u32 acc = low | v3;
        acc &= 0xF;
        u32 high = v2 & 0xF0u;   // v2 & ~0x0F (asm negs -16)
        acc = high | acc;
        u32 rot = acc >> 4;
        u32 swap = ((acc << 28) >> 28);
        rot = rot | swap;
        rot <<= 4;
        u32 low4 = acc & 0xF;
        u32 out = low4 | rot;
        b[2] = (u8)out;

        // conditional nibble updates on EF90[0]==8 and record[1] bits
        if (v3 & 1) {
            volatile u8 *recs = (volatile u8 *)(uintptr_t)*(volatile u32 *)(b + 0x2C);
            if (recs[0] == 8) {
                u32 bits = b[3] & 3;
                if (bits != 0) {
                    u32 idh = (u32)b[3];
                    u32 recn = ((u32)out >> 4);
                    if (idh == recn) {
                        b[6] = (u8)(0x10 | b[6]);
                    }
                }
            }
            u32 v6 = b[6];
            u32 low6 = (v6 << 28) >> 28;
            u32 rec_hi = ((u32)b[2]) >> 4;
            if (((u32)14 & low6) == ((u32)14 & rec_hi)) {
                b[6] = (u8)(0x40 | v6);
            } else {
                u32 rv = recs[1];
                u32 bit1 = ((u32)rv << 25) >> 31;
                u32 merged = (bit1 << 6);
                b[6] = (u8)((b[6] & ~0x40u) | merged);
            }
            u32 rb = recs[1];
            u32 bit3 = ((u32)rb << 27) >> 31;
            u32 merged2 = bit3 << 5;
            b[6] = (u8)((b[6] & ~0x20u) | merged2);
        }
        return b[3];
    }
}
#ifndef __APPLE__
int _080008F4(void *a) __attribute__((alias("SessionVerify_080008F4")));
int sub_080008F4(void *a) __attribute__((alias("SessionVerify_080008F4")));
#endif
