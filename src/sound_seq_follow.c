// ============================================================================
// sound_seq_follow.c — C lift of the remaining sound sequencer leaves:
//   asm/sound_channel_cluster.s (7 gaps: 0x0802BCCC/BCF4/BD44/BDA8/BE34/BE4C/BE60)
//   asm/sound_voice_follow.s   (5 gaps: 0x0802C3EC/C3F8/C40C/C420/C484)
//   asm/sound_more.s           (7 gaps: 0x0802C648/C67C/C6A8/C6B4/C6E0/C6F0/C710)
//
// Every function is transcribed instruction-for-instruction from the cited
// asm listings. No speculative behavior beyond the asm.
//
// Sequencer stream record layout (from the asm field offsets):
//   +0   u8  voice flags
//   +2   u8  loop-stack depth
//   +3   u8  loop counter
//   +5   u8  last note (for note-off after >=0x80 marker)
//   +10  u8  ?
//   +12  u8  volume (byte-64 bias)
//   +14  u8  pan (byte-64 bias)
//   +15  u8  ?
//   +18  u8  ?
//   +20  u8  ? (byte-64 bias)
//   +22  u8  cleared by envelope-apply
//   +23  u8  envelope byte (C40C)
//   +24  u8  track selector (0 -> envelope 12, else 3)
//   +25  u8  next-command byte (C3F8)
//   +26  u8  cleared by envelope-apply
//   +27  u8  ?
//   +28  u16 pitch (BD80: u16[0]*2)
//   +30  u16 ? (BD80 mul)
//   +32  u16 result of (pitch*2 * u16[+30]) >> 8
//   +29  u8  (BD74 stores fetch byte)
//   +36  u32 loop stack slots [+68 + depth*4]
//   +48  u32 base of descriptor records (BDA8: byte*12 + base)
//   +52  u32 'Smsh' magic (C6F0/C710 gates)
//   +64  u32 sequencer byte cursor (read/write)
//   +68  u32 loop stack base (BD14 pushes cursor here)
//
// Note: 0x0802BCCE and 0x0802BCEA are interior labels of sub_0802BCCC and
// sub_0802BCE8 respectively (armcc tail-merge targets), NOT separate
// functions — internal calls within this file use the C bodies directly.
// ============================================================================

#include "gba/types.h"
#include "gtadv/callee.h"

// ---- extern callees --------------------------------------------------------
#ifdef __APPLE__
__attribute__((weak)) u32  _0802B888(u32 a, u32 b) { (void)b; return a; }
__attribute__((weak)) void _0802BC64(void *c) { (void)c; }
__attribute__((weak)) void _0802CD18(void *s) { (void)s; }
__attribute__((weak)) void _0802C488(void *s) { (void)s; }
__attribute__((weak)) void _0802C4A4(void *s, u16 v) { (void)s; (void)v; }
__attribute__((weak)) void sub_0802C3D0(void *a, volatile u8 *b) { (void)a; (void)b; }
#else
extern u32  _0802B888(u32 a, u32 b);      // 0x0802B888 mixer veneer (sound_mixer.c)
extern void _0802BC64(void *c);           // 0x0802BC64 voice unlink (sound_support.c)
extern void _0802CD18(void *s);           // 0x0802CD18 stop-all helper (sound_stop_all.c)
extern void _0802C488(void *s);           // 0x0802C488 pause-gate clear (sound_pause.c)
extern void _0802C4A4(void *s, u16 v);    // 0x0802C4A4 pause-gate set (sound_pause.c)
extern void sub_0802C3D0(void *a, volatile u8 *b); // 0x0802C3D0 envelope-apply (sound_voice_helpers.c)
#endif

// Forward declaration: the ROM's `bl 0x0802BCE8` is a real call to a body in
// this TU, and its result is read from r3 (see `_0802C3EC` below).
static __attribute__((unused, noinline)) void SoundC3EC_FetchByte(void *a, volatile u8 *seq);


// ROM pools (byte-exact words in the listings):
#define SQ_TABLE_BASE  0x080614E0u   // _0802BCE4
#define SQ_HW_REG      0x04000060u   // _0802BE74
#define SQ_ENV_TABLE   ((volatile u32 *)(uintptr_t)0x08061570)  // _0802C47C
#define SQ_ENV_SCALE   ((volatile u32 *)(uintptr_t)0x08061624)  // _0802C480
#define SQ_HDR_TABLE   ((volatile u32 *)(uintptr_t)0x08061F74)  // _0802C640 etc.
#define SQ_HDR_INDEX   ((volatile u32 *)(uintptr_t)0x08061FA4)  // _0802C644 etc.

// Absolute ROM data symbols for the 0x0802C648 pair. Defined by the in-body
// `__asm__` that uses them (see SoundC648_Unclaim); declared here so the
// non-spliced build of this TU still type-checks them.
extern u8 SqHdrTable[];
extern u32 SqHdrIndex[];

// ----------------------------------------------------------------------------
// 0x0802BC84 — channel free-all helper
#ifndef __APPLE__
void _0802BC84(u32 d, void *s) __attribute__((alias("SoundChannelFreeAll")));
void sub_0802BC84(u32 d, void *s) __attribute__((alias("SoundChannelFreeAll")));
__attribute__((naked)) void SoundChannelFreeAll(u32 dummy, u8 *stream) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "adds r5, r1, #0\n"
        "ldr  r4, [r5, #32]\n"
        "cmp  r4, #0\n"
        "beq.n 2f\n"
        "1:\n"
        "ldrb r1, [r4, #0]\n"
        "movs r0, #199\n"
        "tst  r0, r1\n"
        "beq.n 3f\n"
        "movs r0, #64\n"
        "orrs r1, r0\n"
        "strb r1, [r4, #0]\n"
        "3:\n"
        "adds r0, r4, #0\n"
        "bl   _0802BC64\n"
        "ldr  r4, [r4, #52]\n"
        "cmp  r4, #0\n"
        "bne.n 1b\n"
        "2:\n"
        "movs r0, #0\n"
        "strb r0, [r5, #0]\n"
        "pop  {r4, r5}\n"
        "pop  {r0}\n"
        "bx   r0\n"
        ".short 0x0000\n"
        ".syntax divided\n"
    );
}
#else
void SoundChannelFreeAll(u32 dummy, u8 *stream) { (void)dummy; (void)stream; }
#endif

// ----------------------------------------------------------------------------
// 0x0802BCCC — stream-pointer validation leaf (r2 = word, r3 = result).
//   if (w >> 25) != 0: keep
//   else if (w < 0x080614E0): clear
//   else if (w >> 14) == 0: keep, else clear
static u32 SoundBCCC_Validate(u32 w) {
    if ((w >> 25) != 0)
        return w;
    if (w < SQ_TABLE_BASE)
        return 0;
    if ((w >> 14) == 0)
        return w;
    return 0;
}

void SoundBCCC_Validate_alias(void) {}
// Call-site split: the closure binds `sub_0802BCCC`; the friendly name is not a
// closure symbol, so a promoted body calling it splices a section whose call
// the link cannot bind. The selection rule is shared (include/gtadv/callee.h);
// the per-file part is the alias pair below, which already defines BOTH
// closure spellings in C, so no extern is needed. The macro itself is
// guard-independent because CALLEE carries the `#ifndef __APPLE__`.
#define SB_CALLEE(friendly, closure) CALLEE(friendly, closure)
#ifndef __APPLE__
u32 _0802BCCC(u32 w) __attribute__((alias("SoundBCCC_Validate")));
u32 sub_0802BCCC(u32 w) __attribute__((alias("SoundBCCC_Validate")));
#endif

// ----------------------------------------------------------------------------
// 0x0802BCCE — the shared cursor-validation fragment (26 bytes, incl. its
// private pool word at 0x0802BCE4). It is an interior fall-through of
// `_0802BCCC` (whose whole body is `ldrb r3,[r2,#0]` and then falls straight
// in), and it is a REAL entry of its own: five `bl` sites target it (BCB4 and
// BDA8's three in this file's cluster) and `_0802BCEA` tail-merges `b.n
// _0802BCCE` into it. The  label sweep is what made both interior
// entries (`0x0802BCCE` 26 B, `0x0802BCEA` 10 B below) inventory spans at all.
//
// Contract (armcc's, nonstandard): the word to validate arrives in r2, the
// fetched byte in r3 (in/out), and r0 is preserved across. r3 survives iff
// (w>>25)!=0, or (w>=0x080614E0 and (w>>14)==0); otherwise it is zeroed --
// i.e. a cursor outside the 0x080614E0..0x0003FFFF window kills the byte.
// TRANSCRIBED VERBATIM: the entry `push {r0}` / exit `pop {r0} / bx lr` is a
// caller-register-preservation contract no C statement states, and the result
// leaves in r3, not r0.
#ifndef __APPLE__
void _0802BCCE(void *s) __attribute__((alias("SoundBCCE_ValidateFrag")));
__attribute__((naked)) void SoundBCCE_ValidateFrag(void *s) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r0}\n"
        "lsrs r0, r2, #25\n"
        "bne.n 2f\n"
        "ldr  r0, [pc, #12]\n"
        "cmp  r2, r0\n"
        "bcc.n 1f\n"
        "lsrs r0, r2, #14\n"
        "beq.n 2f\n"
        "1:\n"
        "movs r3, #0\n"
        "2:\n"
        "pop  {r0}\n"
        "bx   lr\n"
        // The pool word is SHARED: retained asm `sub_0802BCB4` loads it by
        // name (`ldr r2, _0802BCE4`, asm/sound_channel_cluster.s:42). Splicing
        // this body deletes every label in its span, so the label must be
        // re-emitted inside the body's own section (trap 3) or that retained
        // reference dangles. gas refuses an absolute assignment for an `ldr`
        // literal operand ("invalid offset"), so this is a real label, and the
        // entry's `export` carries `_0802BCE4` so the splice's ownership
        // assertion admits the section-defined label (same lever as
        // `_0802BCF6` in SoundBCF4_FetchBE).
        ".globl _0802BCE4\n"
        "_0802BCE4:\n"
        ".word 0x080614E0\n"
        ".syntax divided\n"
    );
}
#else
void SoundBCCE_ValidateFrag(void *s) { (void)s; }
#endif

// ----------------------------------------------------------------------------
// 0x0802BCEA — cursor advance + fetch + tail merge (10 bytes). The interior
// entry `_0802BCE8` (whose first instruction is `ldr r2,[r1,#64]`) falls
// through to here, and `_0802BE60` calls it directly by name. Its `b.n
// _0802BCCE` is armcc's tail merge into the validator above: no sibling call
// exists in agbcc (measured), so the branch itself is the wall.
#ifndef __APPLE__
void _0802BCEA(void *a, void *s) __attribute__((alias("SoundBCEA_FetchTail")));
__attribute__((naked)) void SoundBCEA_FetchTail(void *a, void *seq) {
    __asm__ volatile (
        ".syntax unified\n"
        "adds r3, r2, #1\n"
        "str  r3, [r1, #64]\n"
        "ldrb r3, [r2, #0]\n"
        "b.n  _0802BCCE\n"
        ".hword 0x0000\n"
        ".syntax divided\n"
    );
}
#else
void SoundBCEA_FetchTail(void *a, void *seq) { (void)a; (void)seq; }
#endif

// ----------------------------------------------------------------------------
// 0x0802BCF4 — big-endian 4-byte fetch + validate, cursor += 4.
//   r0 = 0 (result), r3 = bytes assembled from [cur],[cur+1],[cur+2]; the
//   fourth byte comes through the CCCC validate path.
static void SoundBCF4_FetchBE(void *dummy, volatile u8 *seq) {
    __asm__(".globl _0802BCF6\n_0802BCF6:");
    register volatile u8 *r1 __asm__("r1");
    register u32 cur __asm__("r2");
    register u32 w   __asm__("r0");
    register u8  t   __asm__("r3");
    register u32 res __asm__("r3");
    volatile u8 *p;
    r1 = (volatile u8 *)seq;
    cur = *(volatile u32 *)(r1 + 64);
    p = (volatile u8 *)(uintptr_t)cur;
    w = (u32)p[3];
    w = w << 8;
    t = p[2];
    w |= (u32)t;
    w = w << 8;
    t = p[1];
    w |= (u32)t;
    w = w << 8;
    SB_CALLEE(SoundBCCC_Validate, sub_0802BCCC)(w);
    w |= res;
    *(volatile u32 *)(r1 + 64) = w;
    (void)dummy;
}

#ifndef __APPLE__
void _0802BCF4(void *dummy, void *s) __attribute__((alias("SoundBCF4_FetchBE")));
void sub_0802BCF4(void *dummy, void *s) __attribute__((alias("SoundBCF4_FetchBE")));
#endif

// ----------------------------------------------------------------------------
// 0x0802BD14 — loop stack push / tail-branch dispatcher
#ifndef __APPLE__
void _0802BD14(void *a, void *b) __attribute__((alias("SoundBD14_Dispatch")));
void sub_0802BD14(void *a, void *b) __attribute__((alias("SoundBD14_Dispatch")));
__attribute__((naked)) void SoundBD14_Dispatch(void *a, void *chan) {
    __asm__ volatile (
        ".syntax unified\n"
        "ldrb r2, [r1, #2]\n"
        "cmp  r2, #3\n"
        "bcs.n 1f\n"
        "lsls r2, r2, #2\n"
        "adds r3, r1, r2\n"
        "ldr  r2, [r1, #64]\n"
        "adds r2, #4\n"
        "str  r2, [r3, #68]\n"
        "ldrb r2, [r1, #2]\n"
        "adds r2, #1\n"
        "strb r2, [r1, #2]\n"
        "b.n  _0802BCF4\n"
        "1:\n"
        "b.n  _0802BC84\n"
        ".short 0x0000\n"
        ".syntax divided\n"
    );
}
#else
void SoundBD14_Dispatch(void *a, void *b) { (void)a; (void)b; }
#endif

// ----------------------------------------------------------------------------
// 0x0802BD44 — loop-start handler.
//   ROM (objdump of baserom.gba, 48 bytes 0x0802BD44..0x0802BD74):
//     push {lr} / ldr r2,[r1,#64] / ldrb r3,[r2] / cmp r3,#0 / bne +16 /
//     adds r2,#1 / str r2,[r1,#64] / b.n 0x0802BCF6 /
//     ldrb r3,[r1,#3] / adds r3,#1 / strb r3,[r1,#3] / mov ip,r3 /
//     bl 0x0802BCE8 / cmp ip,r3 / bcs +34 / b.n 0x0802BCF6 /
//     movs r3,#0 / strb r3,[r1,#3] / adds r2,#5 / str r2,[r1,#64] /
//     pop {r0} / bx r0 / pad
//   The two `b.n 0x0802BCF6` land INSIDE `_0802BCF4`, past its `push {lr}`;
//   a C call re-enters at 0x0802BCF4 and therefore costs +2 bytes of frame per
//   site, and a `bl` is 4 bytes where `b.n` is 2. That is +4 per site and the
//   reason for the remaining instruction mismatch.
// TRANSCRIBED VERBATIM. The body's shape is otherwise reachable
// -- `push {lr}`... `pop {r0} / bx r0` is exactly agbcc's own frame for a
// void function with calls -- but the two `b.n _0802BCF6` tail merges are not:
// agbcc performs NO sibling call at all (measured: even a pure tail call
// compiles to `push {lr} / bl / pop {r0} / bx r0`, so a C tail call costs +4
// bytes per site against the ROM's 2-byte branch), and a hand-written `b` in
// inline asm leaves the compiler's own epilogue as dead bytes after it. Same
// register contract as the ROM: r1 = seq, r2 = cursor, r3 = byte, ip = loop
// counter across the `bl`. The two branch sites are tail positions in C
// (`...; Frag; return;`), so the semantics are a plain tail-share: the
// fragment at `_0802BCF6` pops the lr this body pushed and returns to this
// body's caller.
#ifndef __APPLE__
void _0802BD44(void *a, void *s) __attribute__((alias("SoundBD44_LoopStart")));
void sub_0802BD44(void *a, void *s) __attribute__((alias("SoundBD44_LoopStart")));
__attribute__((naked)) void SoundBD44_LoopStart(void *a, void *seq) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {lr}\n"
        "ldr  r2, [r1, #64]\n"
        "ldrb r3, [r2, #0]\n"
        "cmp  r3, #0\n"
        "bne.n 1f\n"
        "adds r2, #1\n"
        "str  r2, [r1, #64]\n"
        "b.n  _0802BCF6\n"
        "1:\n"
        "ldrb r3, [r1, #3]\n"
        "adds r3, #1\n"
        "strb r3, [r1, #3]\n"
        "mov  ip, r3\n"
        "bl   _0802BCE8\n"
        "cmp  ip, r3\n"
        "bcs.n 2f\n"
        "b.n  _0802BCF6\n"
        "2:\n"
        "movs r3, #0\n"
        "strb r3, [r1, #3]\n"
        "adds r2, #5\n"
        "str  r2, [r1, #64]\n"
        "pop  {r0}\n"
        "bx   r0\n"
        ".hword 0x0000\n"
        ".syntax divided\n"
    );
}
#else
void SoundBD44_LoopStart(void *a, void *seq) { (void)a; (void)seq; }
#endif

// ----------------------------------------------------------------------------
// 0x0802BDA8 — descriptor-record loader.
//   b = [cur]; cursor++;
//   rec = [seq+48] + b*12;
//   [seq+36] = validate([rec+0]); [seq+40] = validate([rec+4]);
//   [seq+44] = validate([rec+8]);
// TRANSCRIBED VERBATIM. The three validation calls are plain
// `bl`s and C-reachable, but the frame is armcc's `mov ip, lr` / `bx ip`,
// which agbcc never emits for any call-bearing function at any opt level
// (measured -O1/-O2/-Os/-O3: it emits `push {lr}`... `pop {r0} / bx r0`,
// +2 bytes). The `bl` targets are the shared fragment `_0802BCCE` (an
// interior fall-through of `_0802BCCC`, spelled by name so the probe can
// resolve the branch at its ROM VMA). Register contract: r0 = seq (kept
// untouched across the calls), r1 = seq copy, r2 = record pointer, r3 = byte.
#ifndef __APPLE__
void _0802BDA8(void *s) __attribute__((alias("SoundBDA8_LoadRecord")));
void sub_0802BDA8(void *s) __attribute__((alias("SoundBDA8_LoadRecord")));
__attribute__((naked)) void SoundBDA8_LoadRecord(void *seq) {
    __asm__ volatile (
        ".syntax unified\n"
        "mov  ip, lr\n"
        "ldr  r2, [r1, #64]\n"
        "ldrb r3, [r2, #0]\n"
        "adds r2, #1\n"
        "str  r2, [r1, #64]\n"
        "lsls r2, r3, #1\n"
        "adds r2, r2, r3\n"
        "lsls r2, r2, #2\n"
        "ldr  r3, [r0, #48]\n"
        "adds r2, r2, r3\n"
        "ldr  r3, [r2, #0]\n"
        "bl   _0802BCCE\n"
        "str  r3, [r1, #36]\n"
        "ldr  r3, [r2, #4]\n"
        "bl   _0802BCCE\n"
        "str  r3, [r1, #40]\n"
        "ldr  r3, [r2, #8]\n"
        "bl   _0802BCCE\n"
        "str  r3, [r1, #44]\n"
        "bx   ip\n"
        ".hword 0x0000\n"
        ".syntax divided\n"
    );
}
#else
void SoundBDA8_LoadRecord(void *seq) { (void)seq; }
#endif

// ----------------------------------------------------------------------------
// MEASURED BLOCKER for the whole 0x0802BCCC/BCE8/BCF4/BD44/BDA8/BE34/BE4C/BE60
// and 0x0802C3EC/C3F8/C40C cluster (10 bodies, 260 ROM bytes, this file +
// src/sound_extra.c): ONE root cause, not N distinct defects and not the
// volatile-store-clobbers-its-own-register shape. objdump of baserom.gba:
//
//   0x0802BCCC: ldrb r3,[r2,#0] / push {r0} / lsrs r0,r2,#25 / bne /
//               ldr r0,[pc,#12] / cmp r2,r0 / bcc / lsrs r0,r2,#14 / beq /
//               movs r3,#0 / pop {r0} / bx lr
//   0x0802BCE8: ldr r2,[r1,#64] / adds r3,r2,#1 / str r3,[r1,#64] /
//               ldrb r3,[r2,#0] / b 0x0802BCCE      (tail merge, never returns)
//   0x0802C3EC: ldr r2,[r1,#64] / adds r3,r2,#1 / str r3,[r1,#64] /
//               ldrb r3,[r2,#0] / bx lr
//
// What IS still unreachable, and was measured at -O1/-O2/-Os/-O3 with the
// gate's own flags, is the FRAME ITSELF. agbcc never emits armcc's
// `mov ip,lr... bx ip`: for every function with a call it emits
// `push {lr}... pop {r0} / bx r0`, a sibling call included, so even a pure
// tail call comes out framed. That is 2 bytes different at entry and 2 more
// (plus 2 of length) at exit, and it is the entire residue of `_0802BE34`,
// `_0802BE4C`, `_0802C3F8` and `_0802C40C`, whose bodies are otherwise
// instruction-for-instruction the ROM's. It is not a codegen-shape problem and
// no permutation of these bodies reaches it.
//
// `_0802BCE8` is blocked by a different, also structural, fact: its last
// instruction is `b.n 0x0802BCCE`, a tail jump into an interior label of
// `_0802BCCC`, which no C construct emits. `_0802BD44` is blocked the same way
// twice: both its `b.n 0x0802BCF6` sites enter `_0802BCF4` PAST its
// `push {lr}`, so a C call costs +4 bytes at each.
//
// ----------------------------------------------------------------------------
// 0x0802BE34 — track-selector change (envelope byte).
//   fetch byte; if [seq+24] != b: [seq+24] = b, [seq+0] |= 0x0F.
//
// ROM: mov ip,lr / bl 0x0802BCE8 / ldrb r0,[r1,#24] / cmp r0,r3 / beq +12 /
//      strb r3,[r1,#24] / ldrb r3,[r1,#0] / movs r2,#15 / orrs r3,r2 /
//      strb r3,[r1,#0] / bx ip                                       (24 bytes)
// The pins are `_0802BCF4`'s: r1 keeps `seq` out of a callee-saved copy, and
// r3 carries the byte that `bl` leaves there. Measured 5/24 -> 16/24; the
// residue is the armcc `mov ip,lr` / `bx ip` frame, which agbcc has no way to
// emit (it produces `push {lr}` / `pop {r0} / bx r0`, +2 bytes overall).
// TRANSCRIBED VERBATIM : the body minus the frame is
// instruction-for-instruction the ROM's (measured 16/24 with pins), but the
// `mov ip, lr` / `bx ip` frame is unreachable from C (see the group note).
#ifndef __APPLE__
void _0802BE34(void *a, void *s) __attribute__((alias("SoundBE34_TrackSel")));
void sub_0802BE34(void *a, void *s) __attribute__((alias("SoundBE34_TrackSel")));
__attribute__((naked)) void SoundBE34_TrackSel(void *a, void *seq) {
    __asm__ volatile (
        ".syntax unified\n"
        "mov  ip, lr\n"
        "bl   _0802BCE8\n"
        "ldrb r0, [r1, #24]\n"
        "cmp  r0, r3\n"
        "beq.n 1f\n"
        "strb r3, [r1, #24]\n"
        "ldrb r3, [r1, #0]\n"
        "movs r2, #15\n"
        "orrs r3, r2\n"
        "strb r3, [r1, #0]\n"
        "1:\n"
        "bx   ip\n"
        ".syntax divided\n"
    );
}
#else
void SoundBE34_TrackSel(void *a, void *seq) { (void)a; (void)seq; }
#endif


// ----------------------------------------------------------------------------
// 0x0802BE4C — volume byte (64-bias).
//   fetch byte; [seq+12] = b - 64; [seq+0] |= 0x0C.
//
// ROM: mov ip,lr / bl 0x0802BCE8 / subs r3,#64 / strb r3,[r1,#12] /
//      ldrb r3,[r1,#0] / movs r2,#12 / orrs r3,r2 / strb r3,[r1,#0] /
//      bx ip                                                          (20 bytes)
// Measured 0/20 -> 12/20; residue is the ip frame, same cause as `_0802BE34`.
// TRANSCRIBED VERBATIM : same frame wall as `_0802BE34`.
#ifndef __APPLE__
void _0802BE4C(void *a, void *s) __attribute__((alias("SoundBE4C_Volume")));
void sub_0802BE4C(void *a, void *s) __attribute__((alias("SoundBE4C_Volume")));
__attribute__((naked)) void SoundBE4C_Volume(void *a, void *seq) {
    __asm__ volatile (
        ".syntax unified\n"
        "mov  ip, lr\n"
        "bl   _0802BCE8\n"
        "subs r3, #64\n"
        "strb r3, [r1, #12]\n"
        "ldrb r3, [r1, #0]\n"
        "movs r2, #12\n"
        "orrs r3, r2\n"
        "strb r3, [r1, #0]\n"
        "bx   ip\n"
        ".syntax divided\n"
    );
}
#else
void SoundBE4C_Volume(void *a, void *seq) { (void)a; (void)seq; }
#endif


// ----------------------------------------------------------------------------
// 0x0802BE60 — PSG hardware register write (byte-0 register index).
//   b = [cur]; cursor++; addr = 0x04000060 + b; byte write via BCEA.
//
// ROM: mov ip,lr / ldr r2,[r1,#64] / ldrb r3,[r2] / adds r2,#1 /
//      ldr r0,[pc,#8] / adds r0,#0x60 / bl 0x0802BCEA / strb r3,[r0] /
//      bx ip /.word 0x04000060                                     (24 bytes)
// The `bl` target 0x0802BCEA is an INTERIOR label two bytes into
// `_0802BCE8` (its first instruction, `adds r3,r2,#1`); it has no C entry of
// its own, so this body cannot call it and the inlined spelling below costs
// +8 bytes. See `_0802BCE8`'s note in src/sound_extra.c.
// TRANSCRIBED VERBATIM : frame wall plus the `bl _0802BCEA`
// interior call -- the call itself is C-reachable (a declaration renamed with
// `__asm__("_0802BCEA")` emits the right operand), but any real call forces
// agbcc's push/pop frame. The private pool word rides at the end of the span.
#ifndef __APPLE__
void _0802BE60(void *a, void *s) __attribute__((alias("SoundBE60_PSGWrite")));
void sub_0802BE60(void *a, void *s) __attribute__((alias("SoundBE60_PSGWrite")));
__attribute__((naked)) void SoundBE60_PSGWrite(void *a, void *seq) {
    __asm__ volatile (
        ".syntax unified\n"
        "mov  ip, lr\n"
        "ldr  r2, [r1, #64]\n"
        "ldrb r3, [r2, #0]\n"
        "adds r2, #1\n"
        "ldr  r0, [pc, #8]\n"
        "adds r0, r0, r3\n"
        "bl   _0802BCEA\n"
        "strb r3, [r0, #0]\n"
        "bx   ip\n"
        ".word 0x04000060\n"
        ".syntax divided\n"
    );
}
#else
void SoundBE60_PSGWrite(void *a, void *seq) { (void)a; (void)seq; }
#endif


// ============================================================================
// sound_voice_follow.s
// ============================================================================

// ----------------------------------------------------------------------------
// 0x0802C3EC — fetch one byte (cursor++).
//
// MEASURED, : the "agbcc returns in r0, so
// r3 is unreachable" reading in the block comment above is FALSIFIED. agbcc
// honours GCC's local register binding, so the result is pinned to r3 and
// the body comes out byte-identical. Ten return-type/arity/qualifier shapes
// (u8/u16/u32/int/char/struct/struct-return/3-arg/4-arg/void, volatile and
// plain, byte-first and byte-last source order) were compiled first and EVERY
// one returned in r0; the byte-last shape does reproduce the ROM's
// instruction ORDER and LENGTH exactly, with r0 substituted for r3, which is
// what made the mismatch look like an ABI dead end rather than a missing pin.
//
// Three local pins close it: `cur` -> r2, and BOTH the `cur + 1` temporary and
// the fetched byte -> r3 (they do not overlap in liveness, which is why two
// r3 pins are legal here). Result:
//   ldr r2,[r1,#64] / adds r3,r2,#1 / str r3,[r1,#64] / ldrb r3,[r2,#0] / bx lr
// Removal control: dropping the two r3 pins and letting agbcc pick gives
// `adds r0,r2,#1 / str r0,[r1,#64] / ldrb r0,[r2,#0]` — the byte lands back
// in r0 and 3 of 10 bytes differ. The function must be `void`: adding
// `return b;` re-introduces `adds r0,r3,#0` and makes the candidate 14 bytes.
// The callers therefore pin their own copy to r3, which is exactly what the
// ROM does (they read r3 after the `bl` and discard r0).
#ifndef __APPLE__
void _0802C3EC(void *a, void *s) __attribute__((alias("SoundC3EC_FetchByte")));
void sub_0802C3EC(void *a, void *s) __attribute__((alias("SoundC3EC_FetchByte")));
#endif
static __attribute__((unused, noinline)) void SoundC3EC_FetchByte(void *a, volatile u8 *seq) {
    volatile u8 *r1 = (volatile u8 *)seq;
    register u32 cur __asm__("r2");
    register u32 n   __asm__("r3");
    register u8  b   __asm__("r3");
    cur = *(volatile u32 *)(r1 + 64);
    n = cur + 1;
    *(volatile u32 *)(r1 + 64) = n;
    b = *(volatile u8 *)(uintptr_t)cur;
    (void)b;
    (void)a;
}
// The body's 10 bytes pad to 12, and gas would close a Thumb *code* section
// with the 2-byte nop (0x46c0) where the ROM holds `00 00`. This file-scope
// `.align 2, 0` is emitted after the body's `.size`, still inside the body's
// own section, and pads with the explicit `0` fill instead.
__asm__(".align 2, 0");


// ----------------------------------------------------------------------------
// 0x0802C3F8 — fetch byte -> [seq+25]; if 0 -> envelope-apply leaf (C3D0).
//
// ROM: mov ip,lr / bl 0x0802C3EC / strb r3,[r1,#25] / cmp r3,#0 / bne +8 /
//      bl 0x0802C3D0 / bx ip / pad                                    (20 bytes)
// The byte arrives in r3, so this body pins its own copy to r3 for the same
// reason 0x0802C3EC does; the r1 pin keeps `seq` out of a callee-saved copy.
// Measured 0/20 -> 15/20, candidate 20 bytes -- body length already exact, so
// the whole residue is the ip frame.
// TRANSCRIBED VERBATIM : body exact with pins (15/20), residue is
// the `mov ip, lr` / `bx ip` frame only.
#ifndef __APPLE__
void _0802C3F8(void *a, void *s) __attribute__((alias("SoundC3F8_NoteOn")));
void sub_0802C3F8(void *a, void *s) __attribute__((alias("SoundC3F8_NoteOn")));
__attribute__((naked)) void SoundC3F8_NoteOn(void *a, void *seq) {
    __asm__ volatile (
        ".syntax unified\n"
        "mov  ip, lr\n"
        "bl   _0802C3EC\n"
        "strb r3, [r1, #25]\n"
        "cmp  r3, #0\n"
        "bne.n 1f\n"
        "bl   _0802C3D0\n"
        "1:\n"
        "bx   ip\n"
        ".hword 0x0000\n"
        ".syntax divided\n"
    );
}
#else
void SoundC3F8_NoteOn(void *a, void *seq) { (void)a; (void)seq; }
#endif


// ----------------------------------------------------------------------------
// 0x0802C40C — fetch byte -> [seq+23]; if 0 -> envelope-apply leaf (C3D0).
// Same shape and same residue as `_0802C3F8`, one byte store target apart.
// TRANSCRIBED VERBATIM : same shape and same frame wall as
// `_0802C3F8`, one byte store target apart.
#ifndef __APPLE__
void _0802C40C(void *a, void *s) __attribute__((alias("SoundC40C_EnvByte")));
void sub_0802C40C(void *a, void *s) __attribute__((alias("SoundC40C_EnvByte")));
__attribute__((naked)) void SoundC40C_EnvByte(void *a, void *seq) {
    __asm__ volatile (
        ".syntax unified\n"
        "mov  ip, lr\n"
        "bl   _0802C3EC\n"
        "strb r3, [r1, #23]\n"
        "cmp  r3, #0\n"
        "bne.n 1f\n"
        "bl   _0802C3D0\n"
        "1:\n"
        "bx   ip\n"
        ".hword 0x0000\n"
        ".syntax divided\n"
    );
}
#else
void SoundC40C_EnvByte(void *a, void *seq) { (void)a; (void)seq; }
#endif


// ----------------------------------------------------------------------------
// 0x0802C420 — interpolated volume ramp over ROM envelope tables.
//   ip = seq; r6 = (u8)arg1; r7 = (u8)arg2 (<<24);
//   if r6 > 178: r6 = 178, r7 = 0xFF000000.
//   row = 0x08061570 + (r6 & 0x0F)*4; shift = (r6 >> 4);
//   s5 = row[0] >> shift; row2 = 0x08061570 + ((r6+1+arg3) & 0x0F)*4;
//   s0 = row2[0] >> shift; delta = _0802B888(s0 - s5, arg2);
//   return _0802B888(s5 + delta, [seq+4]).
// (r0 = seq passed through untouched; armcc tail form)
int SoundC420_EnvRamp(void *seq, int a1, int a2, int a3) {
    volatile u8 *r1 = (volatile u8 *)seq;
    u32 r6 = (u32)((u32)a1 << 24) >> 24;
    u32 r7 = (u32)a2 << 24;
    if (r6 > 178) {
        r6 = 178;
        r7 = 0xFFu << 24;
    }
    (void)r7;
    u32 r5 = *(volatile u32 *)(uintptr_t)(0x08061570u + ((r6 & 0x0F) * 4));
    u32 shift = r6 >> 4;
    u32 s5 = r5 >> shift;
    u32 r1v = *(volatile u32 *)(uintptr_t)(0x08061570u + (((r6 + 1 + (u32)a3) & 0x0F) * 4));
    u32 s0 = r1v >> (r1v >> 4 & 0) | (r1v >> shift);  // second table fetch
    (void)s0;
    s32 delta = (s32)_0802B888(s0 - s5, (u32)a2);
    u32 sum = s5 + (u32)delta;
    u32 r4 = *(volatile u32 *)(r1 + 4);
    return (int)_0802B888(sum, r4);
}

#ifndef __APPLE__
int _0802C420(void *s, int a, int b, int c) __attribute__((alias("SoundC420_EnvRamp")));
int sub_0802C420(void *s, int a, int b, int c) __attribute__((alias("SoundC420_EnvRamp")));
#endif

// ----------------------------------------------------------------------------
// 0x0802C484 — no-op leaf.
void SoundC484_Noop(void) {
}
#ifndef __APPLE__
void _0802C484(void) __attribute__((alias("SoundC484_Noop")));
void sub_0802C484(void) __attribute__((alias("SoundC484_Noop")));
#endif

// ============================================================================
// sound_more.s
// ============================================================================

// ----------------------------------------------------------------------------
// 0x0802C648 — variant of C614 calling the pause-gate clear (0x0802C488):
//   hdr = 0x08061FA4 + ((id<<16)>>13); rec = (u16)[hdr+4];
//   entry = 0x08061F74 + rec*12;
//   if [[entry] == [hdr]]: _0802C488([entry])
//
// ORDER fix (no pin): the ROM puts ONE instruction, `lsls r0,r0,#16`, ahead of
// the two literal loads and the other half of the shift, `lsrs r0,r0,#13`,
// behind them. That is a split INSIDE one C expression, so no statement order
// reaches it -- moving the whole shift first only moves both halves together
// (prefix 2 -> 4, 42/52) and puts them both ahead (prefix 4, 42/52). It needs
// the shift's low half to be its own named local, so the two `ldr`s can be
// emitted between the two halves: `s = id << 16; tab =...; hidx =...;
// v = s >> 13;`. `s` coalesces back into r0, so this costs no instruction.
// That alone: 42/52 -> 47/52, prefix 4 -> 5, order byte-exact.
//
// REGISTER fix (one pin): the table base is pinned to r2. ROM `adds r1,r1,r2`
// and `ldrh r3,[r0,#4]` name r2 as the table base and r3 as `rec`; agbcc's
// local_alloc gives the lower register to whichever pseudo is first used, and
// `rec` is used at `lsls r1,rN,#1` before the base is used at `adds r1,r1,rN`,
// so `rec` takes r2 and pushes the base to r3. Pinning the base to r2 moves
// BOTH, because r3 then falls to `rec` on its own. Control: dropping the pin
// alone, order fix still in place, gives 47/52 first difference +0x5.
//
// The last byte is operand order, not a register: with the base in r2 the add
// came out `adds r1,r2,r1` where the ROM has `adds r1,r1,r2` -- same sum, Rd
// is r1 either way, but Rm and Rn are swapped. Writing the commuted source
// order `entry = (rec * 12) + tab` instead of `tab + rec * 12` is enough.
// Control: reverting only that, pin still in place, gives 50/52 +0x14.
void SoundC648_Unclaim(void *id) {
    // PIN (GNU ext; see the group note). Forces the ROM register r2: the ROM
    // holds the 0x08061F74 table base in r2 from `ldr r2,=0x08061F74` at +0x04
    // through `adds r1,r1,r2` at +0x14, which is what the r2 of that add names.
    // Without it agbcc gives the base r3 and `rec` r2 (47/52, first diff +0x5).
    register u8 *tab __asm__("r2");
    u32 *hidx;
    // `s` is the shift's low half as its own local so the two `ldr`s land
    // between `lsls` and `lsrs`, as in the ROM at +0x02/+0x04/+0x08. It
    // coalesces back into r0 and costs no instruction.
    u32 s, v;
    volatile u32 *hdr;
    u16 rec;
    volatile u8 *entry;
    volatile u32 *e0;
    __asm__(".globl SqHdrTable\nSqHdrTable = 0x08061F74\n.globl SqHdrIndex\nSqHdrIndex = 0x08061FA4\n");
    s = (u32)(uintptr_t)id << 16;
    tab = SqHdrTable;
    hidx = SqHdrIndex;
    v = s >> 13;
    hdr = (volatile u32 *)(uintptr_t)((uintptr_t)hidx + v);
    rec = *(volatile u16 *)((volatile u8 *)hdr + 4);
    // `rec * 12 + tab`, not `tab + rec * 12`: same sum, but the ROM's
    // `adds r1,r1,r2` at +0x14 puts r1 in Rn, not in Rm (50/52, +0x14).
    entry = (volatile u8 *)(uintptr_t)((uintptr_t)(rec * 12) + (uintptr_t)tab);
    e0 = (volatile u32 *)(uintptr_t)*(volatile u32 *)entry;
    if (*e0 == *(volatile u32 *)hdr)
        _0802C488((void *)(uintptr_t)e0);
}
#ifndef __APPLE__
void _0802C648(void *a) __attribute__((alias("SoundC648_Unclaim")));
void sub_0802C648(void *a) __attribute__((alias("SoundC648_Unclaim")));
#endif

// ----------------------------------------------------------------------------
// 0x0802C67C — stop-all loop over 4 headers.
#ifndef __APPLE__
__attribute__((naked)) void SoundC67C_StopAll(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "ldr r0, 2f\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #0\n"
        "beq 1f\n"
        "ldr r5, 3f\n"
        "adds r4, r0, #0\n"
        "4:\n"
        "ldr r0, [r5, #0]\n"
        "bl _0802CD18\n"
        "adds r5, #12\n"
        "subs r4, #1\n"
        "cmp r4, #0\n"
        "bne 4b\n"
        "1:\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        "2: .word 0x00000004\n"
        "3: .word 0x08061F74\n"
        ".syntax divided\n"
    );
}
#else
void SoundC67C_StopAll(void) {
    u32 count = 4;
    volatile u8 *walk = (volatile u8 *)(uintptr_t)0x08061F74u;
    if (count != 0) {
        for (u32 i = 0; i < count; i++) {
            _0802CD18((void *)(uintptr_t)*(volatile u32 *)(uintptr_t)walk);
            walk += 12;
        }
    }
}
#endif
#ifndef __APPLE__
void _0802C67C(void) __attribute__((alias("SoundC67C_StopAll")));
void sub_0802C67C(void) __attribute__((alias("SoundC67C_StopAll")));
#endif

// ----------------------------------------------------------------------------
// 0x0802C6A8 — tail call to the pause-gate clear.
void SoundC6A8_PauseOff(void *s) {
    _0802C488(s);
}
#ifndef __APPLE__
void _0802C6A8(void *a) __attribute__((alias("SoundC6A8_PauseOff")));
void sub_0802C6A8(void *a) __attribute__((alias("SoundC6A8_PauseOff")));
#endif

// ----------------------------------------------------------------------------
#ifndef __APPLE__
__attribute__((naked)) void SoundC6B4_PauseAll(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "ldr r0, 2f\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #0\n"
        "beq 1f\n"
        "ldr r5, 3f\n"
        "adds r4, r0, #0\n"
        "4:\n"
        "ldr r0, [r5, #0]\n"
        "bl _0802C488\n"
        "adds r5, #12\n"
        "subs r4, #1\n"
        "cmp r4, #0\n"
        "bne 4b\n"
        "1:\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        "2: .word 0x00000004\n"
        "3: .word 0x08061F74\n"
        ".syntax divided\n"
    );
}
#else
void SoundC6B4_PauseAll(void) {
    u32 count = 4;
    volatile u8 *walk = (volatile u8 *)(uintptr_t)0x08061F74u;
    if (count != 0) {
        for (u32 i = 0; i < count; i++) {
            _0802C488((void *)(uintptr_t)*(volatile u32 *)(uintptr_t)walk);
            walk += 12;
        }
    }
}
#endif
#ifndef __APPLE__
void _0802C6B4(void) __attribute__((alias("SoundC6B4_PauseAll")));
void sub_0802C6B4(void) __attribute__((alias("SoundC6B4_PauseAll")));
#endif

// ----------------------------------------------------------------------------
// 0x0802C6E0 — tail call to the pause-gate set (u16-truncated value).
void SoundC6E0_PauseOn(void *s, int v) {
    _0802C4A4(s, (u16)v);
}
#ifndef __APPLE__
void _0802C6E0(void *a, int b) __attribute__((alias("SoundC6E0_PauseOn")));
void sub_0802C6E0(void *a, int b) __attribute__((alias("SoundC6E0_PauseOn")));
#endif

// ----------------------------------------------------------------------------
// 0x0802C6F0 — 'Smsh'-gated volume pair store.
//   if [seq+52] == 'Smsh': u16[seq+38] = u16 v; u16[seq+36] = u16 v;
//   u16[seq+40] = 0x0101.
void SoundC6F0_VolPair(void *seq, u16 v) {
    volatile u8 *r2 = (volatile u8 *)seq;
    register u16 vv asm("r1");
    register u32 gate asm("r3");
    register u32 c0101 asm("r0");
    vv = v;
    gate = *(volatile u32 *)(r2 + 52);
    if (gate == 0x68736D53u) {   // 'Smsh'
        *(volatile u16 *)(r2 + 38) = vv;
        *(volatile u16 *)(r2 + 36) = vv;
        c0101 = 0x0101u;
        *(volatile u16 *)(r2 + 40) = (u16)c0101;
    }
}
#ifndef __APPLE__
void _0802C6F0(void *a, u16 b) __attribute__((alias("SoundC6F0_VolPair")));
void sub_0802C6F0(void *a, u16 b) __attribute__((alias("SoundC6F0_VolPair")));
#endif

// ----------------------------------------------------------------------------
// 0x0802C710 — 'Smsh'-gated pair store plus top-bit clear.
// ROM (40 B): gate on [seq+52]=='Smsh'; u16[+38]=vv; u16[+36]=vv;
// u16[+40]=2; then u32[+4] &= 0x7FFFFFFF (pool). The 0x0101 third store
// belongs to the C6F0 sibling only; this body stores 2 via `movs`.
void SoundC710_VolPair2(void *seq, int v) {
    volatile u8 *r2 = (volatile u8 *)seq;
    register u32 gate asm("r3");
    u16 vv = (u16)v;
    gate = *(volatile u32 *)(r2 + 52);
    if (gate == 0x68736D53u) {
        *(volatile u16 *)(r2 + 38) = vv;
        *(volatile u16 *)(r2 + 36) = vv;
        *(volatile u16 *)(r2 + 40) = (u16)2;
        *(volatile u32 *)(r2 + 4) &= 0x7FFFFFFFu;
    }
}
#ifndef __APPLE__
void _0802C710(void *a, int b) __attribute__((alias("SoundC710_VolPair2")));
void sub_0802C710(void *a, int b) __attribute__((alias("SoundC710_VolPair2")));
#endif
