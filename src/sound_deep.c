#include "gtadv/sound.h"
#include "gba/types.h"
#include "gba/bios.h"
#include "gba/regs.h"

// Sound deep followup — remaining pure-Thumb with exact hardware/voice/sequence behavior.
// Only substantiated semantics; opaque state not guessed, no weak no-op aliases.
// Mixed-ISA mixer_2b888.s stays raw unless copy contract proven.


// 0x0802D9B8 — the real entry: u16 normalise prologue (lsls/lsrs) that falls
// through into the 0xD9BC selector body. xref shows no direct callers of
// 0xD9BC (only two bl to 0xD9B8 from 0x080057F8/0x08005802), so the D9BC
// typed entry is a span artifact, not a second function. The ROM layout is
// frameless with three per-arm literal pools plus pad shorts; agbcc emits one
// end pool and frames the body, so plain C cannot place the constants. Naked
// hardware-contract transcription (promoted naked bodies e.g. SoundBCB4_Copy
// establish the precedent). Span for promotion: 0xD9B8..0xD9FC (68 bytes).
int _0802D9B8(u32 s) __attribute__((alias("SoundD9BC_SelectorU16")));
int sub_0802D9B8(u32 s) __attribute__((alias("SoundD9BC_SelectorU16")));
int _0802D9BC(u32 s) __attribute__((alias("SoundD9BC_SelectorU16")));
int sub_0802D9BC(u32 s) __attribute__((alias("SoundD9BC_SelectorU16")));
int SoundD9BC_SelectorU16(u32 sel) {
    extern u32 SelectorCell4[];
    extern u32 SelectorCell64[];
    extern u32 SelectorCellDefault[];
    register u32 result __asm__("r2");

    __asm__(".globl SelectorCell4\nSelectorCell4 = 0x0203FD54\n"
            ".globl SelectorCell64\nSelectorCell64 = 0x0203FD54\n"
            ".globl SelectorCellDefault\nSelectorCellDefault = 0x0203FD54\n");

    sel = (u16)sel;
    result = 0;
    if (sel == 4) {
        *(volatile u32 *)SelectorCell4 = 0x080C4274u;
    } else if (sel == 64) {
        *(volatile u32 *)SelectorCell64 = 0x080C4280u;
    } else {
        *(volatile u32 *)SelectorCellDefault = 0x080C4274u;
        result = 1;
    }
    return (int)result;
}
// D9BC spellings alias the entry: no caller references 0xD9BC directly.

// --- sound_da20.s 0x02DA20 ---
// ROM (asm/sound_da20.s:5-30): int-returning (0 ok / 1 dev>3). dev narrowed
// to u8 (lsls/lsrs into r1); state copied to r2 first. Stores dev to
// [0x03001768], reloads the byte, [0x03001770] = 0x04000100 + (byte<<2),
// [state] = pool 0x0802D9FD (Thumb bit0-set pointer).
int SoundDA20_Setup(u32 dev, void *state){
    u32 st = (u32)(uintptr_t)state;
    u32 d = (u8)dev;
    if (d > 3) return 1;
    *(volatile u8 *)0x03001768u = (u8)d;
    volatile u32 *dst = (volatile u32 *)0x03001770u;
    u32 b = *(volatile u8 *)0x03001768u;
    *dst = 0x04000100u + (b << 2);
    *(volatile u32 *)st = 0x0802D9FDu;
    return 0;
}
int _0802DA20(u32 d, void *s) __attribute__((alias("SoundDA20_Setup")));
int sub_0802DA20(u32 d, void *s) __attribute__((alias("SoundDA20_Setup")));

// --- sound_db24/dba4/dc54/dd30/dd88 sector helpers ---
// Preserve hardware register accesses: REG_DMA3 etc., widths u16/s16, sentinel Smsh check via 0x0203FD54+4 gate
// ROM (asm/sound_db24.s, 128 B, private pools 0x0802DB84-0x0802DBA0):
// latches *0x04000208 into r6, zeroes it, rewrites the halfword at 0x04000204
// as (*0x04000204 & 0xF8FF) | *(u16*)(*(u32*)0x0203FD54 + 6), programs DMA3
// (SAD 0x040000D4 = arg0, DAD 0x040000D8 = arg1, CNT 0x040000DC =
// (u32)(u16)arg2 | 0x80000000), polls CNT bit 15 (0x8000) until clear, then
// restores the latched value. No BIOS call: this is a raw DMA3 sequence, not
// CpuSet. arg2 is read as u16 (lsls #16 / lsrs #16) before the 0x80000000 or.
void SoundDB24_Setup(u32 sad, u32 dad, u16 cnt){
    u16 saved;
    {
        volatile u16 *latch = (volatile u16 *)0x04000208u;
        saved = *latch;
        *latch = 0;
    }
    {
        volatile u16 *mode = (volatile u16 *)0x04000204u;
        register u32 m __asm__("r4") = *mode;
        __asm__ volatile ("" : "+r" (m));
        m &= 0xF8FFu;
        m |= *(volatile u16 *)(*(volatile u32 *)0x0203FD54u + 6);
        *mode = (u16)m;
        *(volatile u32 *)0x040000D4u = sad;
        *(volatile u32 *)0x040000D8u = dad;
    }
    {
        register volatile u32 *cc __asm__("r1") = (volatile u32 *)0x040000DCu;
        *cc = (u32)cnt | 0x80000000u;
        __asm__ volatile ("" : "+r" (cc));
        {
            volatile u16 *st1 = (volatile u16 *)((volatile u8 *)cc + 2);
            register u32 b1 __asm__("r2") = 0x8000u;
            __asm__ volatile ("" : "+r" (b1));
            register u32 t __asm__("r0") = b1;
            __asm__ volatile ("" : "+r" (t));
            register u32 v __asm__("r1") = *st1;
            __asm__ volatile ("" : "+r" (v));
            if ((t & v) != 0) {
                volatile u16 *st = (volatile u16 *)0x040000DEu;
                register u32 b2t __asm__("r0") = (u32)0x80 << 8;
                __asm__ volatile ("" : "+r" (b2t));
                register u32 b2 __asm__("r1") = b2t;
                __asm__ volatile ("" : "+r" (b2));
                while ((b2 & *st) != 0) { }
            }
        }
    }
    *(volatile u16 *)0x04000208u = saved;
}
void _0802DB24(u32 s, u32 d, u16 c) __attribute__((alias("SoundDB24_Setup")));
void sub_0802DB24(u32 s, u32 d, u16 c) __attribute__((alias("SoundDB24_Setup")));

// DBA4 sector reader. ROM (asm/sound_dba4.s, 176 B): int-returning
// (0x80FF gate-fail / 0 ok). Sector narrowed to u16 (into r3); gate
// sector < *(u16*)(desc+4) else return 0x80FF. Desc base 0x0203FD54 stays
// in r6 only across the seed loop (scoped); bit loop reuses r6 for `one`.
// Seed: p=r2 from sp+n*2+2, i=r4; start bits; DB24(208<<20, sp,
// byte8+3) then DB24(same, sp, 0x44); bit loop row=r4, acc=r1,
// bit=r3, q=r2, w=r5. Desc+8 reloads are volatile byte reads each time.
//
// Byte-exact levers (each verified by single-function probe):
//  - gate: the pool word must reach r0 (not r6) while the body loads it
//    again for r6, so the base lives in a pinned r0 for the gate and in a
//    pinned r6 (`descp`) for the loop reloads; `t` (r0) forces ldr/mov/ldr.
//  - buf address: ROM adds `adds r2, r0, r4` (n*2 first, frame in r4), then
//    adds 2. The C sum and `p++` reproduce those adds; a register barrier
//    keeps the second add after the sum.
//  - the three counters are u32 whose `(u8)` narrowing is a separate step;
//    an r0-pinned temp (`ti`/`tb`/`tr`) reproduces agbcc's
//    adds r0,rX,#1 / lsls r0,#24 / lsrs rX,r0,#24 shape (a plain u32
//    narrowing narrows in place; a u8 variable narrows into r0 instead).
//  - the destination-row advance is C (`d += 3`) with a register barrier
//    immediately before it. Without that barrier, loop.c hoists the advance
//    to the loop head and the row tail loses the add. The row counter's `+1`
//    remains asm because the C form also moves it to the loop head.
int SoundDBA4_Read(u32 sector, void *dst){
    register volatile u16 *d __asm__("r5") = (volatile u16 *)dst;
    register u32 sec __asm__("r3") = (u16)sector;
    register u32 *g0 __asm__("r0") = (u32 *)0x0203FD54u;
    if (sec >= *(volatile u16 *)((volatile u8 *)*g0 + 4)) return 0x80FF;
    {
        u16 buf[68];
        register u32 *descp __asm__("r6");
        register volatile u16 *p __asm__("r2");
        register u32 *t __asm__("r0");
        register u32 na __asm__("r0");
        register u32 ba __asm__("r4");
        register u32 ti __asm__("r0");
        register u32 tb __asm__("r0");
        register u32 tr __asm__("r0");
        register u32 i __asm__("r4");
        register u32 n __asm__("r1");
        t = (u32 *)0x0203FD54u;
        descp = t;
        n = ((volatile u8 *)*t)[8];
        na = (u32)n * 2u;
        ba = (u32)(uintptr_t)buf;
        p = (volatile u16 *)(uintptr_t)(na + ba);
        __asm__ volatile ("" : "+r"(p));
        p++;
        i = 0;
        if (i < n) {
            do {
                *p = (u16)sec;
                p--;
                sec >>= 1;
                ti = i + 1u;
                i = (u8)ti;
                n = ((volatile u8 *)*descp)[8];
            } while (i < n);
        }
        *p = 1;
        p--;
        *p = 1;
        {
            u32 base = (u32)208 << 20;
            u8 m = ((volatile u8 *)*(volatile u32 *)0x0203FD54u)[8];
            sub_0802DB24((u32)(uintptr_t)buf, base, (u16)((u32)m + 3u));
            sub_0802DB24(base, (u32)(uintptr_t)buf, 0x44u);
        }
        // `p = buf + 4` lowers to the ROM's `add r2, sp, #8`, but gcse hoists the
        // loop-invariant frame-base address into a callee-saved r7 across the two
        // DMA calls (prologue gains r7, body 54/176). The empty r7 register-clobber
        // discards that cse fact without emitting bytes, so the add is computed
        // fresh here into p's pinned r2. An instruction-splicing `add r2,sp,#8` was
        // previously required here; see docs/findings/indexed_halfword_scratch.md.
        __asm__ volatile ("" : : : "r7");
        p = buf + 4;
        __asm__ volatile ("" : "+r"(d));
        d += 3;
        {
            u32 row;
            register u32 acc __asm__("r1");
            register u32 bit __asm__("r3");
            register u32 one __asm__("r6");
            row = 0;
            one = 1;
            do {
                acc = 0;
                bit = 0;
                do {
                    acc = (u16)(((acc << 17) >> 16) | (*p & one));
                    p++;
                    tb = bit + 1u;
                    bit = (u8)tb;
                } while (bit <= 15);
                *d = (u16)acc;
                d--;
                __asm__ volatile ("" : "+r"(row));
                row = (u8)(row + 1u);
            } while (row <= 3);
        }
    }
    return 0;
}
int _0802DBA4(u32 s, void *d) __attribute__((alias("SoundDBA4_Read")));
int sub_0802DBA4(u32 s, void *d) __attribute__((alias("SoundDBA4_Read")));

extern void sub_0802DA58(volatile u16 *src);   // runtime_record_helpers.c
extern void sub_0802DAE0(void);                // runtime_record_helpers.c

// EEPROM sector writer — write twin of SoundDBA4_Read. The bit stream is built
// DOWNWARD from sp + n, so the halfword at sp is the start bit the DMA reads
// first: one seed zero, four input halfwords each expanded to 16 shifted
// copies, n sector bits, then `*p = 0; p--; *p = 1; p--;`. The DMA count is
// n + 67 because the stream spans 2*(n + 67) bytes from sp. The tail spins on
// the EEPROM status cell until bit 0 sets and returns 0xC001 if the arming
// flag at 0x0300176C went off meanwhile.
//
// Three shapes hold the bytes (all verified by single-function probe):
//  - an empty `"+r"` barrier in front of each wrap-around increment keeps
//    loop.c from moving the `+1` to the loop head;
//  - `dp = *t; goto test;` gives the ROM's rotated sector-bit loop (test at
//    the bottom, `b` over the body) instead of a duplicated top test;
//  - the second masked read is a fresh `two` pinned to r1 with its own load,
//    so agbcc re-materialises `movs r1,#1` instead of hoisting the constant
//    into a callee-saved register (that also frees r5, dropping it from the
//    prologue, and lands the mask in r1 as the ROM has it).
int SoundDC54_Write(u32 sector, void *src){
    register volatile u16 *d __asm__("r5") = (volatile u16 *)src;
    register u32 sec __asm__("r4") = (u16)sector;
    register u32 *g0 __asm__("r0") = (u32 *)0x0203FD54u;
    if (sec >= *(volatile u16 *)((volatile u8 *)*g0 + 4)) return 0x80FF;
    {
        u16 buf[82];
        register u32 *t __asm__("r0");
        register u32 *descp __asm__("r2");
        register volatile u16 *p __asm__("r3");
        register u32 na __asm__("r0");
        register u32 ba __asm__("r1");
        register u32 nb __asm__("r0");
        register u32 dp __asm__("r0");
        u32 i;
        u32 n;

        t = (u32 *)0x0203FD54u;
        na = ((volatile u8 *)*t)[8] * 2u;
        ba = (u32)(uintptr_t)buf;
        p = (volatile u16 *)(uintptr_t)(na + ba);
        __asm__ volatile ("" : "+r"(p));
        p += 66;
        *p = 0;
        p--;
        i = 0;
        do {
            register u32 w __asm__("r2") = *d;
            d++;
            nb = 0;
            do {
                *p = (u16)w;
                p--;
                w >>= 1;
                nb = (u8)(nb + 1u);
            } while (nb <= 15);
            __asm__ volatile ("" : "+r"(i));
            i = (u8)(i + 1u);
        } while (i <= 3);

        i = 0;
        t = (u32 *)0x0203FD54u;
        descp = t;
        dp = *t;
        goto sector_test;
        do {
            *p = (u16)sec;
            p--;
            sec >>= 1;
            __asm__ volatile ("" : "+r"(i));
            i = (u8)(i + 1u);
            dp = *descp;
sector_test:
            ;
        } while (i < ((volatile u8 *)(uintptr_t)dp)[8]);
        *p = 0;
        p--;
        *p = 1;
        p--;
        {
            register u32 base __asm__("r1") = (u32)208 << 20;
            u8 m = ((volatile u8 *)*(volatile u32 *)0x0203FD54u)[8];
            sub_0802DB24((u32)(uintptr_t)buf, base, (u16)((u32)m + 67u));
        }
        sub_0802DA58((volatile u16 *)0x080C428Cu);

        {
            register u32 rc __asm__("r4") = 0;
            register volatile u16 *stat __asm__("r1") = (volatile u16 *)0x0D000000u;
            register u32 one __asm__("r3") = 1;
            register volatile u8 *flag __asm__("r2") = (volatile u8 *)0x0300176Cu;
            for (;;) {
                if ((*stat & one) != 0) break;
                if (*flag != 0) {
                    u16 v = *stat;
                    register u32 two __asm__("r1") = 1;
                    if ((v & two) != 0) break;
                    rc = 0xC001u;
                    break;
                }
            }
            sub_0802DAE0();
            return (int)rc;
        }
    }
}
int _0802DC54(u32 s, void *d) __attribute__((alias("SoundDC54_Write")));
int sub_0802DC54(u32 s, void *d) __attribute__((alias("SoundDC54_Write")));

// dd30 compare helper. ROM (asm/sound_dd30.s): sector narrowed to u16
// (lsls/lsrs into r1); gate: sector < *(u16*)(desc+4) else return 0x80FF;
// calls DBA4(sector, sp) to fill an 8-byte stack buffer, compares 4 u16 lanes
// buf[i] vs stack[i] with a u8 counter (bhi #3 exit); first mismatch sets
// r5 = 0x8000 (movs 128; lsls 8), full match returns 0.
int SoundDD30_Compare(u32 sector, void *buf){
    register void *b __asm__("r4") = buf;
    register u32 sec __asm__("r1") = (u16)sector;
    register u32 rc __asm__("r5") = 0;
    volatile u16 *gate = (volatile u16 *)(*(volatile u32 *)0x0203FD54u + 4);
    if (sec >= *gate) return 0x80FF;
    {
        u16 tmp[4];
        sub_0802DBA4(sec, tmp);
        volatile u16 *p = (volatile u16 *)b;
        volatile u16 *q = (volatile u16 *)tmp;
        u8 i = 0;
        goto body;
        do {
            i = (u8)(i + 1);
test:
            if (i > 3) break;
body:
            u16 x = *p;
            u16 y = *q;
            q++;
            p++;
            if (x != y) { rc = 0x8000u; break; }
        } while (1);
    }
    return (int)rc;
}
// ROM span ends in `00 00`; file-scope align pads the section tail with 0.
__asm__(".align 2, 0");
int _0802DD30(u32 s, void *b) __attribute__((alias("SoundDD30_Compare")));
int sub_0802DD30(u32 s, void *b) __attribute__((alias("SoundDD30_Compare")));

// dd88 retry wrapper. ROM shape (asm/sound_dd88.s): sector narrowed to u16
// (lsls/lsrs into r4); u8 counter r6 looping while <=2 (three tries); each
// try calls write then compare, and EITHER nonzero u16 result retries
// (compare result skipped when write fails); returns the last u16 result
// (0 on success, nonzero after 3 failures — not -1). Register pins are the
// ROM's own allocation: buf r5, sector r4, counter r6.
int SoundDD88_Retry(u32 sector, void *buf){
    register void *b __asm__("r5") = buf;
    register u32 sec __asm__("r4") = (u16)sector;
    register u32 i __asm__("r6") = 0;
    u16 r;
    goto test;
    do {
        register u32 next __asm__("r0") = i + 1u;
        __asm__ volatile ("" : "+r" (next));
        i = (next << 24) >> 24;
test:
        if (i > 2) break;
        r = (u16)sub_0802DC54(sec, b);
        if (r != 0) continue;
        r = (u16)sub_0802DD30(sec, b);
        if (r != 0) continue;
        break;
    } while (1);
    return (int)r;
}
// ROM span ends in `00 00`; file-scope align pads the section tail with 0
// instead of gas's `46c0` Thumb-section-close nop.
__asm__(".align 2, 0");
int _0802DD88(u32 s, void *b) __attribute__((alias("SoundDD88_Retry")));
int sub_0802DD88(u32 s, void *b) __attribute__((alias("SoundDD88_Retry")));

// --- sound_mixer_tail.s 0x02BC28 ---
// Thumb continuation after the ARM loop. This is entered with the mixer stack
// frame and callee-saved register image already active; it must branch back to
// the ARM loop or restore that image and return through its saved PC.
__attribute__((naked)) void SoundMixerTailEpilogue(void *state) {
    (void)state;
    __asm__ volatile (
        ".syntax unified\n"
        "ldr r0, [sp, #4]\n"
        "subs r0, #1\n"
        "ble 1f\n"
        "adds r4, #64\n"
        "b _0802B990\n"
        "1:\n"
        "ldr r0, [sp, #24]\n"
        "ldr r3, 2f\n"
        "str r3, [r0, #0]\n"
        "add sp, #28\n"
        "pop {r0, r1, r2, r3, r4, r5, r6, r7}\n"
        "mov r8, r0\n"
        "mov r9, r1\n"
        "mov sl, r2\n"
        "mov fp, r3\n"
        "pop {r3}\n"
        "bx r3\n"
        ".align 2\n"
        "2: .word 0x68736D53\n"
        ".syntax divided\n"
    );
}
void _0802BC28(void *s) __attribute__((alias("SoundMixerTailEpilogue")));
void sub_0802BC28(void *s) __attribute__((alias("SoundMixerTailEpilogue")));

// --- sound_more/followon/fade/init/state subset ---
// Preserve Smsh guard (u32 +52), stride 0x50 not guessed beyond, u16 +36/38 fade widths

//   base = 0x08061FA4 + ((ch & 7) * 8)      `lsls r0,#16 / lsrs r0,#13` is the
//                                           u16-then-shift form, not a mask
//   e    = u16[base + 4]                    `ldrh r3,[r0,#4]`
//   p    = 0x08061F74 + e * 12              `lsls #1; adds; lsls #2` is agbcc's
//                                           multiply-by-12
//   q    = u32[p]                           `ldr r2,[r1]` — the table holds
//                                           POINTERS, so there are two loads
//   if (q[0] == u32[base]) sub_0802CD18(q);  the `bne` skips the call, and the
//                                           callee's argument is q itself
//
// MEASURED 36/52 (was 4 bytes, NO_OVERLAP). One thing is left: the ROM hoists
// BOTH pool loads to the top -- `ldr r2,[pc,#36]; ldr r1,[pc,#40]` between the
// `lsls` and the `lsrs` -- and puts 0x08061F74's slot before 0x08061FA4's,
// which is the reverse of agbcc's use order and of its pool order. agbcc always
// places a `ldr rX,[pc]` at its use, so the two loads cannot be hoisted here.
extern void _0802CD18(void *p);
extern u8 SqTable614[];
extern u32 SqIndex614[];
void SoundMoreHelper(void *id){
    register u8 *tab __asm__("r2");
    u32 *hidx;
    u32 s, v;
    volatile u32 *hdr;
    u16 rec;
    volatile u8 *entry;
    volatile u32 *e0;
    __asm__(".globl SqTable614\nSqTable614 = 0x08061F74\n.globl SqIndex614\nSqIndex614 = 0x08061FA4\n");
    s = (u32)(uintptr_t)id << 16;
    tab = SqTable614;
    hidx = SqIndex614;
    v = s >> 13;
    hdr = (volatile u32 *)(uintptr_t)((uintptr_t)hidx + v);
    rec = *(volatile u16 *)((volatile u8 *)hdr + 4);
    entry = (volatile u8 *)(uintptr_t)((uintptr_t)(rec * 12) + (uintptr_t)tab);
    e0 = (volatile u32 *)(uintptr_t)*(volatile u32 *)entry;
    if (*e0 == *(volatile u32 *)hdr)
        _0802CD18((void *)(uintptr_t)e0);
}
void _0802C614(void *c) __attribute__((alias("SoundMoreHelper")));
void sub_0802C614(void *c) __attribute__((alias("SoundMoreHelper")));

// 0x0802C548 — LIFTED from asm/sound_start.s-adjacent listing (36 B body, 2-word
// pool). Same index walk as SoundMoreHelper (0x0802C614) but WITHOUT the
// `if (e0[0] == hdr[0])` gate: the ROM loads both and calls unconditionally.
//   base = 0x08061FA4 + ((u16)ch >> 13)     lsls r0,#16 / lsrs #13
//   rec  = u16[base+4]                      ldrh r3,[r0,#4]
//   p    = 0x08061F74 + rec*12              lsls#1/adds/lsls#2
//   e0   = u32[p]                           ldr r2,[r1]  (table holds POINTERS)
//   _0802CC34(e0, base[0])                  ldr r1,[r0]; adds r0,r2,#0; bl
// The epilogue is `pop {r0}; bx r0` (void), so the C stays void. The three
// levers proven on the 0xC614 twin all transfer: the r2 pin on the 0x08061F74
// base, the `lsls`/`lsrs` pair split across two statements so both pool `ldr`s
// land between them, and `rec * 12 + tab` so `adds r1,r1,r2` puts r1 in Rn.
// 0x802c548-0x802c56a is 34 bytes; the 8-byte pool at 0x802c56c closes the
// section at 44, which is the ROM's real function size.
extern void _0802CC34(void *row, void *arg); // src/runtime_state_dispatch.c SoundStart_2CC34
void SoundFollowOnCopy(u32 ch){
    register u8 *tab __asm__("r2");
    u32 *hidx;
    u32 s, v;
    volatile u32 *hdr;
    u16 rec;
    volatile u8 *entry;
    volatile u32 *e0;
    s = ch << 16;
    tab = SqTable614;
    hidx = SqIndex614;
    v = s >> 13;
    hdr = (volatile u32 *)(uintptr_t)((uintptr_t)hidx + v);
    rec = *(volatile u16 *)((volatile u8 *)hdr + 4);
    entry = (volatile u8 *)(uintptr_t)((uintptr_t)(rec * 12) + (uintptr_t)tab);
    e0 = (volatile u32 *)(uintptr_t)*(volatile u32 *)entry;
    _0802CC34((void *)(uintptr_t)e0, (void *)(uintptr_t)*(volatile u32 *)hdr);
}
void _0802C548(u32 c) __attribute__((alias("SoundFollowOnCopy")));
void sub_0802C548(u32 c) __attribute__((alias("SoundFollowOnCopy")));

// TODO: remaining large PSG/voice/seq full interpreters stay asm/:
// - sound_d034.s PSG 0x02D034 540L (REG_SOUND*, timer, envelope — opaque 28B struct at sp+4 needs bit-exact proof)
// - sound_d6f4.s seq interpreter beyond dispatch (378L, vector 0x08061788, EWRAM 0x0203EBB0/B4 indirect dispatch — needs EWRAM proof)
// - sound_channel_cluster full walker beyond free (0x02BCE4 table walk + DMA1SAD register streams)
// - sound_bank full claim 0x02B500 (priority byte +2 bit0), sound_seq vol/pan walkers full Smsh loops
// - sound_note_on 0x02C190 285L, sound_voice_* apply/clamp/follow/tick, d4a8/d510/d584 full Smsh loops already partially via core but hosts raw
// - mixer_2b888.s body 0x02B888–0x02BC28 (ARM veneer + Thumb + ARM kernels + IWRAM 0x03000170 copy-site unproven)
// No weak no-op aliases added for these; they stay byte-identical via make.
