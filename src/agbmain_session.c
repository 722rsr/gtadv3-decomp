// ============================================================================
// agbmain_session.c — C lift of the agbmain.s session-cluster gaps:
//   _080006A4  soft-IRQ pump (16 B)
//   _080006BC  session-exit teardown
//   _0800070C  session-word reader / serial state machine (0x13C ROM bytes)
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
extern u32 sub_0802DDC8(void *a);
extern void _0802D974(const void *a, void *b, u32 c); // CpuSet (bios_wrappers.c)
extern void sub_0802D974(const void *a, void *b, u32 c);

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
// sub_08008F4 — raw ASM callee used by the independently linked session lift.
int sub_08008F4(void *buf);

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
// _0800070C(buf) -> u32 — session-word reader (per-frame tick). The idle
// caller stores the full return register at +0xD8, and the retry byte can
// contribute bits above bit 15.
// The initial 32-bit read spans SIOCNT and the adjacent timer counter, as in
// the ROM's `ldr r7,[r6]`; its low half drives the arming tests and return.
u32 SessionRead_0800070C(void *buf)
{
    register void *savedBuf __asm__("ip") = buf;
    register volatile u8 *serial __asm__("r6") = (volatile u8 *)(uintptr_t)0x04000128;
    u32 sio = *(volatile u32 *)serial;
    register volatile u8 *base __asm__("r0") = EF90;
    register int fsm __asm__("r1") = base[1];
    register volatile u8 *session __asm__("r5") = base;
    register u32 sampledState __asm__("r4");
    register u32 statusTemp __asm__("r0");
    register u32 statusMask __asm__("r1");
    u32 bit4Value;
    u8 retry;
    u8 serialValue;
    u32 serialMask;
    register volatile u8 *watchBase __asm__("r1");
    register u32 watchMask __asm__("r0");
    u8 watchByte;

    if (fsm == 1)
        goto watchdog;
    if (fsm > 1)
        goto high_state;
    if (fsm == 0)
        goto state_zero;
    goto compose_status;

high_state:
    if (fsm == 2)
        goto call_verify;
    goto compose_status;

state_zero:
    {
        statusMask = 0x30u;
        statusTemp = sio;
        __asm__ volatile ("" : "+r" (statusTemp));
        statusTemp &= statusMask;
        if (statusTemp == 0) {
            sampledState = sio & 0x88u;
            if (sampledState != 8u)
                goto compose_status;

            statusMask = 4u;
            statusTemp = sio;
            __asm__ volatile ("" : "+r" (statusTemp));
            statusTemp &= statusMask;
            __asm__ volatile ("" : "+r" (statusTemp));
            statusTemp <<= 24;
            bit4Value = statusTemp >> 24;
            if (bit4Value != 0)
                goto state_zero_ready;
            if (*(volatile u32 *)(session + 0x14) == 12u) {
                *AG_IMEM = 0;
                *AG_IE = (u16)(*AG_IE & 0xFF7Fu);
                *AG_IE = (u16)(*AG_IE | 0x40u);
                *AG_IMEM = 1;
                serialValue = serial[1];
                serialMask = 0x41u;
                __asm__ volatile ("" : "+r" (serialMask));
                serialMask = 0u - serialMask;
                serialMask &= serialValue;
                serial[1] = (u8)serialMask;
                *(volatile u32 *)(uintptr_t)0x0400010C = 0x0000ABFBu;
                *(volatile u16 *)(uintptr_t)0x04000202 = 0x00C0u;
                session[0] = (u8)sampledState;
            }
        }
state_zero_ready:
        session[1] = 1;
        goto watchdog;
    }

watchdog:
    watchBase = EF90;
    __asm__ volatile ("" : "+r" (watchBase));
    watchMask = 0xF0u;
    watchByte = watchBase[2];
    watchMask &= watchByte;
    if (watchMask == 0)
        goto call_verify;
    watchMask = 0x40u;
    watchByte = watchBase[6];
    watchMask &= watchByte;
    if (watchMask != 0)
        goto call_verify;
    retry = watchBase[8];
    if (retry > 7)
        goto retry_expired;
    watchBase[8] = (u8)(retry + 1);
    goto call_verify;

retry_expired:
    watchBase[1] = 2;

call_verify:
    sub_08008F4(savedBuf);

compose_status:
    {
        register volatile u8 *composeBase __asm__("r2") = EF90;
        register u32 value __asm__("r1");
        register u32 temp __asm__("r0");
        register u32 accum __asm__("r3");
        register volatile u8 *tail __asm__("r5");

        temp = composeBase[11];
        temp++;
        composeBase[11] = (u8)temp;

        value = composeBase[6];
        accum = 0x10u;
        accum &= value;
        temp = composeBase[3];
        accum |= temp;
        temp = 0x20u;
        temp &= value;
        accum |= temp;
        temp = 0x40u;
        temp &= value;
        accum |= temp;

        value = composeBase[2];
        __asm__ volatile ("" : "+r" (value));
        temp = value >> 4;
        temp <<= 8;
        tail = composeBase;
        {
            register u32 state __asm__("r2") = tail[0];
            __asm__ volatile ("" : "+r" (state));
            if (state == 8u) {
                value = 0x80u;
                value |= temp;
                value |= accum;
            } else {
                value = accum;
                value |= temp;
            }
        }

        if (tail[7] != 0) {
            temp = 0x80u;
            temp <<= 5;
            value |= temp;
        }

        {
            register u32 state __asm__("r5") = tail[8];
            __asm__ volatile ("" : "+r" (state));
            temp = state >> 3;
        }
        {
            register u32 high __asm__("r2") = temp << 15;
            temp = (sio << 26) >> 30;
            if (temp > 3u) {
                temp = 0x80u;
                temp <<= 6;
                temp |= high;
                temp |= value;
            } else {
                temp = value;
                temp |= high;
            }
            return temp;
        }
    }
}
__asm__(".align 2, 0");

u32 _0800070C(void *a) __attribute__((alias("SessionRead_0800070C")));
u32 SessionReadWord(void *a) __attribute__((alias("SessionRead_0800070C")));
u32 sub_0800070C(void *a) __attribute__((alias("SessionRead_0800070C")));

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
//
// EXACT 172/172. The three byte/halfword temps are one function-scope temp
// kept as a global allocno: non-volatile lvalues make the front end load
// zero-extended straight into it, and the empty "+r" barriers after each def
// stop combine folding the load into the following and/add. That lets
// global-alloc place it in r7 (local-alloc never uses the eliminable frame
// pointer register), so marked-home r7 also restores the ROM's
// push {r4,r5,r6,r7,lr} and the mov r7,r8 high-save. The checksum add reads
// temp first -- sum = temp + sum -- to match the ROM's adds r6,r7,r6.
void PacketBuild_08000848(void *payload, u32 flag)
{
    register u32 sum __asm__("r6");
    register volatile u8 *base __asm__("r4");
    register u32 value __asm__("r1");
    register u32 one __asm__("r3");
    u32 temp;
    sum = 0;
    __asm__ volatile ("" : "+r" (sum));
    base = EF90;

    {
        register volatile u8 *packet __asm__("r3");
        register u32 mask __asm__("r2");
        register u32 oldHeader __asm__("r5");
        packet = *(volatile u8 * volatile *)(base + 0x1C);
        value = flag;
        mask = 1;
        value &= mask;
        value <<= 4;
        mask = 17;
        __asm__ volatile ("" : "+r" (mask));
        mask = 0u - mask;
        oldHeader = packet[1];
        mask &= oldHeader;
        mask |= value;
        packet[1] = (u8)mask;
    }

    {
        register u8 *packet __asm__("r5");
        register volatile u8 *source __asm__("r1");
        register u32 bits __asm__("r2");
        packet = *(volatile u8 * volatile *)(base + 0x1C);
        source = (volatile u8 *)(uintptr_t)0x0203F150;
        one = 1;
        __asm__ volatile ("" : "+r" (one));
        bits = one;
        __asm__ volatile ("" : "+r" (bits));
        {
            register u32 sourceByte __asm__("r1") = source[3];
            __asm__ volatile ("" : "+r" (sourceByte));
            bits &= sourceByte;
        }
        bits <<= 5;
        value = 33;
        __asm__ volatile ("" : "+r" (value));
        value = 0u - value;
        temp = packet[1];
        __asm__ volatile ("" : "+r" (temp));
        value &= temp;
        value |= bits;
        packet[1] = (u8)value;
    }

    {
        register u8 *packet __asm__("r2");
        register u32 sioByte __asm__("r5");
        register u32 bit __asm__("r1");
        packet = *(volatile u8 * volatile *)(base + 0x1C);
        sioByte = base[6];
        __asm__ volatile ("" : "+r" (sioByte));
        bit = sioByte << 25;
        __asm__ volatile ("" : "+r" (bit));
        bit >>= 31;
        __asm__ volatile ("" : "+r" (bit));
        one &= bit;
        one <<= 6;
        bit = 65;
        __asm__ volatile ("" : "+r" (bit));
        bit = 0u - bit;
        temp = packet[1];
        __asm__ volatile ("" : "+r" (temp));
        bit &= temp;
        bit |= one;
        packet[1] = (u8)bit;
    }

    {
        register volatile u8 *packet __asm__("r2");
        packet = *(volatile u8 * volatile *)(base + 0x1C);
        value = base[11];
        packet[0] = (u8)value;
    }

    {
        register volatile u8 *packet __asm__("r3");
        register u32 nibble __asm__("r2");
        register u32 mask __asm__("r1");
        register u32 oldHeader __asm__("r5");
        packet = *(volatile u8 * volatile *)(base + 0x1C);
        nibble = base[2];
        __asm__ volatile ("" : "+r" (nibble));
        value = (nibble << 28) >> 28;
        nibble = base[3];
        nibble ^= value;
        mask = 15;
        nibble &= mask;
        mask = 16;
        __asm__ volatile ("" : "+r" (mask));
        mask = 0u - mask;
        oldHeader = packet[1];
        mask &= oldHeader;
        mask |= nibble;
        packet[1] = (u8)mask;
    }

    {
        register volatile u8 *packet __asm__("r1");
        packet = *(volatile u8 * volatile *)(base + 0x1C);
        *(volatile u16 *)(packet + 2) = (u16)sum;
    }

    {
        register volatile u8 *packet __asm__("r1");
        register u32 control __asm__("r2");
        packet = *(volatile u8 * volatile *)(base + 0x1C);
        packet += 4;
        control = 0x04000004u;
        sub_0802D974(payload, (void *)packet, control);
    }

    {
        register u16 *words __asm__("r0");
        value = 0;
        words = (u16 *)*(volatile u8 * volatile *)(base + 0x1C);
        do {
            temp = *words;
            __asm__ volatile ("" : "+r" (temp));
            sum = temp + sum;
            words++;
            value++;
        } while (value <= 9);
    }

    {
        register volatile u8 *packet __asm__("r0");
        packet = *(volatile u8 * volatile *)(base + 0x1C);
        value = ~sum;
        value -= 12;
        *(volatile u16 *)(packet + 2) = (u16)value;
    }

    {
        register u32 ready __asm__("r0") = 1;
        base[4] = (u8)ready;
    }
}
__asm__(".align 2, 0");
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
int SessionVerify_080008F4(void *buf)
{
    register void *output __asm__("r9") = buf;
    register s32 invalidChecksum __asm__("sl");
    register volatile u8 *stage __asm__("r8");
    register volatile u8 *initialState __asm__("r2");
    register s32 tempR6 __asm__("r6");
    volatile u8 *record;
    volatile u32 zeroWords[2];
    volatile s32 nextIndex;
    register void *probeArg __asm__("r0") = (void *)(uintptr_t)0x0203F110;
    u32 probe;

    __asm__ volatile ("" : "+r" (probeArg));
    zeroWords[0] = 0;
    probe = sub_0802DDC8(probeArg);
    probe <<= 24;
    {
        register u32 clearMask __asm__("r1");
        register u32 oldStatus __asm__("r3");
        initialState = EF90;
        initialState[3] = 0;
        clearMask = 65;
        __asm__ volatile ("" : "+r" (clearMask));
        clearMask = 0u - clearMask;
        oldStatus = initialState[6];
        clearMask &= oldStatus;
        initialState[6] = (u8)clearMask;
    }

    if (probe != 0) {
        register volatile u8 *session __asm__("r5");
        register u32 index __asm__("r3") = 3;
        register u32 sum __asm__("r0");
        register volatile u8 *recordArea __asm__("r1");
        register u32 wordIndex __asm__("r2");
        register volatile u8 *copySource __asm__("r4");
        {
            register volatile u8 *stageLow __asm__("r4");
            stageLow = (volatile u8 *)&zeroWords[1];
            __asm__ volatile ("" : "+r" (stageLow));
            stage = stageLow;
        }
        session = initialState;
        tempR6 = 13;
        tempR6 = 0 - tempR6;
        __asm__ volatile ("" : "+r" (tempR6));
        invalidChecksum = tempR6;
        do {
            sum = index << 1;
            sum += index;
            sum <<= 3;
            recordArea = (volatile u8 *)(uintptr_t)
                *(volatile u32 *)(session + 0x2C);
            record = recordArea + sum;
            sum = 0;
            wordIndex = 0;
            nextIndex = (s32)index - 1;
            recordArea = record;
            do {
                u32 word = *(volatile u16 *)recordArea;
                sum += word;
                recordArea += 2;
                wordIndex++;
            } while (wordIndex <= 9);
            sum = (u32)(s32)(s16)sum;
            __asm__ volatile ("" : "+r" (sum));
            copySource = record + 4;

            if ((s32)sum == invalidChecksum) {
                register u32 validBit __asm__("r0") = 1;
                register u32 serialStatus __asm__("r2");
                register u32 statusNibble __asm__("r1");
                validBit <<= index;
                tempR6 = session[3];
                __asm__ volatile ("" : "+r" (tempR6));
                validBit |= tempR6;
                session[3] = (u8)validBit;

                serialStatus = session[6];
                statusNibble = serialStatus << 28;
                statusNibble >>= 28;
                tempR6 = record[1];
                __asm__ volatile ("" : "+r" (tempR6));
                validBit = tempR6 << 26;
                validBit >>= 31;
                validBit <<= index;
                validBit |= statusNibble;
                statusNibble = 15;
                validBit &= statusNibble;
                tempR6 = 16;
                __asm__ volatile ("" : "+r" (tempR6));
                tempR6 = 0u - tempR6;
                __asm__ volatile ("" : "+r" (tempR6));
                statusNibble = tempR6;
                __asm__ volatile ("" : "+r" (statusNibble));
                serialStatus &= statusNibble;
                serialStatus |= validBit;
                session[6] = (u8)serialStatus;

                recordArea = (volatile u8 *)output;
                recordArea += index << 4;
                sub_0802D974((const void *)copySource,
                             (void *)recordArea,
                             0x04000004u);
            }

            zeroWords[1] = 0;
            sub_0802D974((const void *)stage, (void *)copySource,
                         0x05000004u);
            index = (u32)nextIndex;
        } while ((s32)index >= 0);
    }

    {
        register volatile u8 *tailState __asm__("r3") = EF90;
        register volatile u8 *returnState __asm__("r5");
        register u32 statusWork __asm__("r1");
        register u32 statusByte __asm__("r2");
        register u32 temp __asm__("r0");
        register u32 packedStatus __asm__("r4");
        register u32 one __asm__("r1");
        register u32 serial __asm__("r4");
        register u32 serialNibble __asm__("r2");
        register u32 fourteen __asm__("r0");
        register u32 stateByte __asm__("r6");
        register u32 stateHigh __asm__("r3");
        register u32 compareLeft __asm__("r1");

        statusByte = tailState[2];
        temp = statusByte << 28;
        temp >>= 28;
        tempR6 = tailState[3];
        temp |= tempR6;
        statusWork = 15;
        temp &= statusWork;
        statusWork = 16;
        __asm__ volatile ("" : "+r" (statusWork));
        statusWork = 0u - statusWork;
        statusWork &= statusByte;
        statusWork |= temp;
        statusByte = statusWork >> 4;
        temp = statusWork << 28;
        temp >>= 28;
        statusByte |= temp;
        statusByte <<= 4;
        packedStatus = 15;
        packedStatus &= statusWork;
        packedStatus |= statusByte;
        tailState[2] = (u8)packedStatus;
        one = 1;
        temp = one;
        temp &= tempR6;
        returnState = tailState;

        if (temp != 0) {
            if (returnState[0] == 8) {
                temp = tempR6 & 3u;
                if (temp != 0) {
                    temp = tempR6 << 24;
                __asm__ volatile ("" : "+r" (temp));
                    temp >>= 24;
                    statusWork = packedStatus >> 4;
                    if (temp == statusWork) {
                        temp = 16;
                        serial = returnState[6];
                        temp |= serial;
                        returnState[6] = (u8)temp;
                    }
                }

                serial = returnState[6];
                serialNibble = serial << 28;
                serialNibble >>= 28;
                fourteen = 14;
                stateByte = returnState[2];
                __asm__ volatile ("" : "+r" (stateByte));
                stateHigh = stateByte >> 4;
                compareLeft = fourteen;
                compareLeft &= serialNibble;
                fourteen &= stateHigh;
                if (compareLeft == fourteen) {
                    temp = 0x40;
                    temp |= serial;
                    returnState[6] = (u8)temp;
                }
            } else {
                register u32 recordByte __asm__("r2");
                register u32 recordBit __asm__("r0");
                register u32 stateOld __asm__("r3");
                recordByte = record[1];
                __asm__ volatile ("" : "+r" (recordByte));
                recordBit = recordByte << 25;
                recordBit >>= 31;
                statusWork &= recordBit;
                statusWork <<= 6;
                recordBit = 65;
                __asm__ volatile ("" : "+r" (recordBit));
                recordBit = 0u - recordBit;
                stateOld = returnState[6];
                __asm__ volatile ("" : "+r" (stateOld));
                recordBit &= stateOld;
                recordBit |= statusWork;
                returnState[6] = (u8)recordBit;
            }
            {
                register u32 bit5Header __asm__("r7") =
                    (u32)(uintptr_t)record;
                register u32 recordBit __asm__("r1");
                register u32 stateMask __asm__("r0");
                register u32 stateOld __asm__("r4");
                __asm__ volatile ("ldrb %0, [%0, #1]"
                                  : "+l" (bit5Header));
                recordBit = bit5Header << 27;
                recordBit >>= 31;
                recordBit <<= 5;
                stateMask = 33;
                __asm__ volatile ("" : "+r" (stateMask));
                stateMask = 0u - stateMask;
                stateOld = returnState[6];
                __asm__ volatile ("" : "+r" (stateOld));
                stateMask &= stateOld;
                stateMask |= recordBit;
                returnState[6] = (u8)stateMask;
            }
        }

        return returnState[3];
    }
}

int _080008F4(void *a) __attribute__((alias("SessionVerify_080008F4")));
int sub_080008F4(void *a) __attribute__((alias("SessionVerify_080008F4")));
