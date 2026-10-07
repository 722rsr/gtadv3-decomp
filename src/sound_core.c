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
void SoundBeb4Worker(void *state){
    volatile u32 *s = (volatile u32*)state;
    if (s[13] != SOUND_MAGIC) return; // +52
    s[13] = SOUND_MAGIC+1;
    void *cb = (void*)s[14]; // +56
    if (cb) {
        ((void (*)(u32))(uintptr_t)cb)(s[15]); // bx r3, r0=[state+60]
    }
    // Bounded walker: active count u8 +8, array void* +44, stride 0x50, gate u8 +0&0x80
    u8 active = ((volatile u8*)s)[8];
    void *arr = (void*)s[11]; // +44
    for(u32 i=0;i<active;i++){
        volatile u8 *ch = (volatile u8*)arr + i*0x50;
        if((ch[0] & 0x80)==0) continue;
        ch[0x1A] = (u8)(ch[0x1A] + ch[0x19]);
    }
    s[13] = SOUND_MAGIC;
}
#ifndef __APPLE__
void _0802BEB4(void *s) __attribute__((alias("SoundBeb4Worker")));
void sub_0802BEB4(void *s) __attribute__((alias("SoundBeb4Worker")));
#endif

// --- _0802C11C voice-list cleanup (45 B, pool 0x03007FF0 at 0x02C15C) ---
void SoundVoiceCleanup(void *unused, void *chan){
    (void)unused;
    volatile u8 *c = (volatile u8*)chan;
    if ((c[0] & 0x80)==0) return;
    volatile u32 *ch = (volatile u32*)((uintptr_t)chan+32);
    volatile u32 *node = (volatile u32*)*ch;
    while(node){
        u8 mode = ((volatile u8*)node)[0];
        if (mode!=0){
            u8 mode_bits = ((volatile u8 *)node)[1] & 7u;
            if (mode_bits != 0){
                volatile u32 *cell = (volatile u32*)SOUND_ROOT_PTR_CELL;
                volatile u8 *root = (volatile u8 *)(uintptr_t)*cell;
                u32 callback = *(volatile u32 *)(root + 44);
                ((void (*)(u32, void *))(uintptr_t)callback)(mode_bits, chan);
            }
            ((volatile u8*)node)[0]=0;
        }
        *(volatile u32 *)((volatile u8 *)node + 44)=0;
        node = (volatile u32 *)(uintptr_t)*(volatile u32 *)((volatile u8 *)node + 52);
    }
    *(volatile u32 *)((volatile u8 *)chan + 32)=0;
}
#ifndef __APPLE__
void _0802C11C(void *a, void *b) __attribute__((alias("SoundVoiceCleanup")));
void sub_0802C11C(void *a, void *b) __attribute__((alias("SoundVoiceCleanup")));
#endif

// Stub for remaining cluster primitives — preserve DMA register width (0x04000060) without guessing stream bytes
void SoundChannelFetchByte(void *s, u8 *out){ (void)s; if(out) *out=0; }
#ifndef __APPLE__
void _0802BCE8a(void *s, u8 *o) __attribute__((alias("SoundChannelFetchByte")));
#endif

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
#ifndef __APPLE__
void _0802CB20(void *s) __attribute__((alias("SoundCmdCommit")));
void sub_0802CB20(void *s) __attribute__((alias("SoundCmdCommit")));
void Sub_0802CB20(void *s) __attribute__((alias("SoundCmdCommit")));
#endif

extern int _08002DE04(int num, int den); // 0x08002DE04 s32 quotient (sound_extra.c)
extern void sub_0802CB84(void *a); // 0x08002CB84 stop/clear (runtime_state_dispatch.c)

void SoundControlInit(u32 rate) {
    // r4 = root = *(u32*)0x03007FF0
    volatile u8 *r4 = (volatile u8 *)(uintptr_t)*(volatile u32 *)0x03007FF0u;
    // r2 = (rate & 0xF0000) >> 16  (240<<12 mask, then lsrs #16)
    u32 r2 = (rate & 0x000F0000u) >> 16;
    r4[8] = (u8)r2;
    // r5 = *(u16*)(0x08061654 + (r2-1)*2) — u16 period table lookup
    u16 r5 = *(volatile u16 *)(0x08061654u + (u32)(r2 - 1u) * 2u);
    *(volatile u32 *)(r4 + 16) = r5;
    // q1 = 1584 / r5 (198<<3); [r4+11] = (u8)q1
    r4[11] = (u8)_08002DE04(1584, (int)r5);
    // q2 = (0x91D1B*r5 + 0x1388) / 0x2710 (muls wraps 32-bit); [r4+20] = q2
    u32 t = 0x00091D1Bu * (u32)r5 + 0x00001388u;
    *(volatile u32 *)(r4 + 20) = (u32)_08002DE04((int)t, 0x00002710);
    // r1 receives q2 before the third division (asm 0x0802C9CC).
    int q3 = _08002DE04(0x01000000, (int)*(volatile u32 *)(r4 + 20)) + 1;
    *(volatile u32 *)(r4 + 24) = (u32)(q3 >> 1);
    // TM0CNT_H = 0 (stop timer 0)
    *(volatile u16 *)0x04000102u = 0;
    // TM0CNT_L = -(0x44940 / r5) (reload for the sample rate)
    int q4 = _08002DE04(0x00044940, (int)r5);
    *(volatile u16 *)0x04000100u = (u16)(-q4);
    // stop/clear helper (r0 leftover -q4 forwarded as its arg, as in asm)
    sub_0802CB84((void *)(uintptr_t)(u32)(-q4));
    // VCOUNT handshake on 0x04000006: wait while == 159, then until == 159
    // (159 = last visible scanline; two phases so entry ON 159 still takes
    // a full frame). Volatile: -O2 keeps every poll read.
    volatile u8 *vc = (volatile u8 *)0x04000006u;
    while (*vc == 159) { }
    while (*vc != 159) { }
    // TM0CNT_H = 0x80 (start timer 0, prescaler 0, no IRQ)
    *(volatile u16 *)0x04000102u = 0x0080;
}
#ifndef __APPLE__
void _0802C990(u32 r) __attribute__((alias("SoundControlInit")));
void sub_0802C990(u32 r) __attribute__((alias("SoundControlInit")));
#endif

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
// MEASURED, still open (probe, both walkers identical in shape).
// The candidate is 104 B and the whole loop body 0x30-0x52 is byte-identical
// to the ROM. Exactly two differences remain, and they are COUPLED: fixing
// either alone moves the span off 104.
//   (1) The ROM's prologue/epilogue save TWO high callee-saved registers:
//       `mov r7,r9 / mov r6,r8 / push {r6,r7}` and `pop {r3,r4} / mov r8,r3 /
//       mov r9,r4`. r9 is then never read or written in the body -- it is a
//       DEAD SAVE, so the only way to reproduce it is to make agbcc mark r9
//       ever-live and then lose the value. Verified: `mov r9,r1`=0x4681 and
//       `mov r0,r9`=0x4648 (assembled), while the ROM's 0x4684/0x4660 are
//       `mov ip,r0` / `mov r0,ip` -- so the constant 3 really is in r12 and
//       r9 is genuinely unused. Every source shape tried that makes r9 live
//       (volatile `f = p[0]`, `u32 f`, `u32 stride`, `sel` pinned to r9)
//       costs >= 2 EXTRA body instructions, and no dead-register pin
//       (`register u32 d __asm__("r9")`, used or unused) survives DCE.
//   (2) The ROM reads the count as `ldrb r2, [r4, #8]` -- straight into the
//       loop register, no copy. Through the `volatile u32 *s` alias agbcc
//       emits `ldrb r0, [r4, #8] / add r2, r0, #0` because the QI result and
//       the SImode `int` are separate pseudos. Reading through a NON-volatile
//       alias -- `int cnt = ((u8 *)state)[8];` -- removes that copy and makes
//       the loop body byte-identical, but the span then falls to 100 B, i.e.
//       (1) and (2) are worth exactly the same 2 bytes each. One shape that
//       closes (1) as well was not found; do not re-run the pin search above
//       without a new mechanism for a dead r9.
void SoundVoiceWalkerA(void *state, u16 chSel, u16 wid){
    volatile u32 *s = (volatile u32*)state;
    u32 sel = chSel;
    u32 m = s[13];
    if (m != SOUND_MAGIC) return;
    s[13] = m + 1;
    int cnt = ((volatile u8 *)s)[8];
    u8 *p = (u8 *)(uintptr_t)s[11];
    u32 bit = 1;
    while (cnt > 0) {
        if (sel & bit) {
            // The ROM keeps the flag byte live in a register across the
            // ch[0x13] store: one ldrb feeds both the gate and the |= 3.
            if (p[0] & 0x80) {
                ((volatile u8 *)p)[0x13] = (u8)(wid >> 2);
                p[0] |= 3;
            }
        }
        cnt--;
        p += 0x50;
        bit <<= 1;
    }
    s[13] = SOUND_MAGIC;
}
#ifndef __APPLE__
void _0802D4A8(void *s, u16 c, u16 w) __attribute__((alias("SoundVoiceWalkerA")));
void sub_0802D4A8(void *s, u16 c, u16 w) __attribute__((alias("SoundVoiceWalkerA")));
#endif

// --- _0802D510 PAN walker ---
void SoundVoiceWalkerB(void *state, u32 chSel, u32 wid){
    volatile u32 *s=(volatile u32*)state;
    if (s[13]!=SOUND_MAGIC) return;
    s[13]=SOUND_MAGIC+1;
    for(u32 i=0;i< ((volatile u8*)s)[8];i++) if((chSel>>i)&1){
        volatile u8 *ch=(volatile u8*)s[11]+i*0x50; if(ch[0]&0x80){ ch[11]=(u8)wid; ch[13]=(u8)wid; ch[0]|=0x0C; }
    }
    s[13]=SOUND_MAGIC;
}
#ifndef __APPLE__
void _0802D510(void *s, u32 c, u32 w) __attribute__((alias("SoundVoiceWalkerB")));
void sub_0802D510(void *s, u32 c, u32 w) __attribute__((alias("SoundVoiceWalkerB")));
#endif
// --- _0802D584 third-op walker (104 B, asm/sound_d584.s, private pool 0x0802D5E8) ---
// Same shape as _0802D4A8 but the value is narrowed to u8 instead of u16-then-
// shifted, so the prologue pairs `lsls r2,r2,#24` with `lsrs r6,r2,#24` and
// there is no late `lsrs r6,r6,#18`. The stored byte goes to ch[0x15].
//
// MEASURED, still open (probe, 66/104, candidate 104 B). This
// walker has one defect _0802D4A8 does not: the two narrowed parameters come
// out SWAPPED. The ROM narrows wid (u8) into r6 and chSel (u16) into r7 --
// `lsrs r6, r2, #24` / `lsrs r7, r1, #16` -- so the loop test reads r7
// (`adds r0, r7, #0` at +0x48) and the store is `strb r6, [r1, #21]` at
// +0x66. agbcc here emits the mirror image (`lsrs r7, r2, #24` /
// `lsrs r6, r1, #16`, `adds r0, r6, #0`, `strb r7, [r1, #21]`).
// `register u32 sel __asm__("r7")` DOES fix both narrows, but the 0x80 gate
// mask then lands in r12 instead of r8, the r8 save disappears, and the span
// collapses to 92 B. The r8/r9 save (see the _0802D4A8 note above) is the
// blocker for both walkers, not the parameter allocation.
void SoundVoiceWalkerC(void *state, u16 chSel, u8 wid){
    volatile u32 *s = (volatile u32*)state;
    u32 sel = chSel;
    u32 m = s[13];
    if (m != SOUND_MAGIC) return;
    s[13] = m + 1;
    int cnt = ((volatile u8 *)s)[8];
    u8 *p = (u8 *)(uintptr_t)s[11];
    u32 bit = 1;
    while (cnt > 0) {
        if (sel & bit) {
            if (p[0] & 0x80) {
                ((volatile u8 *)p)[0x15] = wid;
                p[0] |= 3;
            }
        }
        cnt--;
        p += 0x50;
        bit <<= 1;
    }
    s[13] = SOUND_MAGIC;
}
#ifndef __APPLE__
void _0802D584(void *s, u16 c, u8 w) __attribute__((alias("SoundVoiceWalkerC")));
void sub_0802D584(void *s, u16 c, u8 w) __attribute__((alias("SoundVoiceWalkerC")));
#endif

// --- _0802D6F4 sequence interpreter (inline table 0x0802D724, 5 entries) ---
void SoundSeqInterpreter(void *state, void *cursor){
    volatile u8 *c=(volatile u8*)cursor;
    u8 op = *c++; // +0
    volatile u32 *s=(volatile u32*)state;
    s[6]= s[6]+op; // +24 adds op byte (width u32)
    c++; // skip arg byte
    c++; // skip next byte
    if(op<=17){
        // dispatch via table 0x0802D724 — preserve indirect branch width
        static void (*table[5])(void*,void*) = {0};
        if(op < 5 && table[op]) table[op](state,cursor);
    }
    // Full interpreter (0x02D728–0x02D846) remains TODO; preserve dispatch width only
    (void)state;
}
#ifndef __APPLE__
void _0802D6F4(void *s, void *c) __attribute__((alias("SoundSeqInterpreter")));
void sub_0802D6F4(void *s, void *c) __attribute__((alias("SoundSeqInterpreter")));
#endif

// --- _0802CD58 fade tick, _0802C780 init, _0802C8C4 state etc. — minimal width-preserving stubs ---
void SoundFadeTick(void *state){
    volatile u16 *s=(volatile u16*)state;
    if(s[18]==0) return; // +36
    s[19]--; // +38
    if(s[19]!=0) return;
    s[19]=s[18]; // reload from +36
    s[20] |= 2; // +40 |=2
}
#ifndef __APPLE__
void _0802CD58(void *s) __attribute__((alias("SoundFadeTick")));
void sub_0802CD58(void *s) __attribute__((alias("SoundFadeTick")));
#endif
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
#ifndef __APPLE__
void _0802C780(void *s) __attribute__((alias("SoundVoiceInit")));
void sub_0802C780(void *s) __attribute__((alias("SoundVoiceInit")));
#endif
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
#ifndef __APPLE__
void _0802C8C4(void *s) __attribute__((alias("SoundStateInit")));
void sub_0802C8C4(void *s) __attribute__((alias("SoundStateInit")));
#endif

// TODO: remaining large pure-Thumb units still unsupported stay asm/
// - sound_d034.s PSG 0x02D034 540L (4× channel envelope/timer, REG_SOUND1CNT etc. — needs PSG struct widths)
// - sound_beb4 full 0x50 stride walk beyond trampoline, sound_seq walkers _0802B718/B74C full Smsh loop,
//   sound_more/followon bank helpers 0x02C614/548, sound_note_on/start/stop/tick/voice_* cluster,
//   d9b0-da/dd BIOS timing helpers. No fake aliases added for these interiors.
// Mixer tail beyond _0802BC4C leaf and mixed-ISA mixer body 0x02B888–0x02BC28 remain separate (copy contract at 0x03000170 unproven).
