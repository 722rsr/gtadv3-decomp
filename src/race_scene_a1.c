// ============================================================================
// race_scene_a1.c — reconstructed C for asm/race_scene.s (10 functions,
// VMA order):
//
//   0x0801A44C (interior tail of sub_08001A294, label _08001A44C to return)
//   0x0801A768 0x0801B068 0x0801B27C 0x0801B364 0x08001B498
//   0x0801B730 (interior of sub_08001B498, label _08001B730 to return)
//   0x0801B77C 0x0801B8DC 0x0801B9AC
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#define HOST_STUB(sig) extern sig

HOST_STUB(void  _08002D974(const void *src, void *dst, u32 ctrl)); // 0x08002D974 CpuSet
HOST_STUB(void  _08002E0A4(void *d, const void *s, u32 n));        // 0x08002E0A4 memcpy
HOST_STUB(void *_08002E104(void *a, int b, u32 c));                // 0x08002E104 memset-like
HOST_STUB(void  _08018AA8(u32 mask, int set));                     // 0x08018AA8 flag set/clear
HOST_STUB(int   _08018ACC(u32 mask));                              // 0x08018ACC flag test
HOST_STUB(int   _08002BE8(int dummy));                             // 0x08002BE8 counter bump, s16 ret
HOST_STUB(void  _08002C48(int a, u16 b));                          // 0x08002C48 table math
HOST_STUB(int   _08002DE04(int n, int d));                         // 0x08002DE04 signed divide
HOST_STUB(u32   _08002DF6C(u32 n, u32 d));                         // 0x08002DF6C unsigned divide, quotient in r0 (R4: NOT remainder)
HOST_STUB(void  _0802DDD0(void *a, void *b, void *c));            // 0x0802DDD0 bx-r2 veneer
HOST_STUB(void  _0802DDCC(void *a, void *b));                     // 0x0802DDCC veneer
HOST_STUB(void  _08004090(void *a, u32 b, void *c, u32 d, int e)); // 0x08004090 5 machine args (r3 select + caller stack word)
HOST_STUB(void  _0800390C(int a, u32 b, const volatile u8 *c));    // 0x0800390C
HOST_STUB(u16   _08026150(int a, int b));                          // 0x08026150 award leaf (strh ret)
HOST_STUB(void  _080261E8(int a, int b));                          // 0x080261E8 award leaf
HOST_STUB(void  _0802612C(void));                                  // 0x0802612C
HOST_STUB(int   _08002A86C(void *rec, int b, int c));              // 0x08002A86C 3-arg record claim
HOST_STUB(void  _080026FF4(void *a, int b, int c, int d));         // 0x080026FF4 record init
HOST_STUB(void  _080270D8(u32 a, u32 b, int c));                   // 0x080270D8
HOST_STUB(void  _08027114(int v));                                 // 0x08027114
HOST_STUB(void  _08027210(int v));                                 // 0x08027210
HOST_STUB(void  _08002B500(int a, int b, int c));                  // 0x08002B500 sound select
HOST_STUB(void  _08002B368(int v));                                // 0x08002B368 sound cue
HOST_STUB(void  _08002B44C(void));                                 // 0x08002B44C
HOST_STUB(void  _08002B234(void));                                 // 0x08002B234
HOST_STUB(void  _08002B3A4(void));                                 // 0x08002B3A4
HOST_STUB(void  _0802B460(void));                                  // 0x0802B460
HOST_STUB(void  _0802B25C(void));                                  // 0x0802B25C
HOST_STUB(void  _0802B474(void));                                  // 0x0802B474
HOST_STUB(void  _0802B280(void));                                  // 0x0802B280
HOST_STUB(void  _080191BC(int v));                                 // 0x080191BC
HOST_STUB(void  _08019AEC(int v));                                 // 0x08019AEC
HOST_STUB(void  _08019A2C(int v));                                 // 0x08019A2C
HOST_STUB(int   _080098C8(u16 a, u16 b));                          // 0x080098C8
HOST_STUB(void  _080096A4(void *p));                               // 0x080096A4
HOST_STUB(void  _0800821C(void));                                  // 0x0800821C timed reset
HOST_STUB(void *_080240C4(void));                                  // 0x080240C4 ghost rec A
HOST_STUB(void *_080240D0(void));                                  // 0x080240D0 ghost rec P
HOST_STUB(void  _080240F8(u32 v));                                 // 0x080240F8
HOST_STUB(void  _080240EC(u32 v));                                 // 0x080240EC
HOST_STUB(void  _08024104(u32 v));                                 // 0x08024104 strh recA+2, word arg
HOST_STUB(void  _08024110(const void *s));                         // 0x08024110
HOST_STUB(void  _08001A4AC(void));                                 // 0x08001A4AC 10-way dispatcher (asm)
HOST_STUB(void  _08001B130(int v));                                // 0x08001B130 (asm)
HOST_STUB(void  _08001B1A4(int v));                                // 0x08001B1A4 6-way dispatcher (asm)
HOST_STUB(void  _08001B410(void));                                 // 0x08001B410 B-handler (asm)

// ---- shared anchors --------------------------------------------------------
#define WORK_AREA 0x03001780u
#define CTX       0x03004E20u
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WORK_AREA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WORK_AREA + (o)))
#define WA_U32(o) (*(volatile u32 *)(uintptr_t)(WORK_AREA + (o)))

// ----------------------------------------------------------------------------
// ---- 0x0801A44C — interior tail of sub_08001A294: flag-byte fold ----
// No pools or calls; 96-byte body.
//   +00 1c03  adds r3, r0, #0; r3 = dst copy, reused as the store base
//   +02 7888  ldrb r0, [r1, #2]; then #6, #4 — IMMEDIATE offset
//...  strb r0, [r3, #0..2]
//   +0E 78da  ldrb r2, [r3, #3]; accumulator lives in r2 for all five bits
//   +10 2007  movs r0, #7; index materialised in r0
//   +12 5608  ldrsb r0, [r1, r0]; SIGNED byte at src+7 — REGISTER offset
//   +14 2800  cmp r0, #0 / +16 d001 beq / +18 2001 movs r0,#1 / +1A 4302 orrs r2,r0
//   +1C 70da  strb r2, [r3, #3]
//   repeated for (src index, bit) = (3,2) (8,4) (9,8) (5,16), ending bx lr at +5E.
//
// Two non-obvious levers, both required for byte-exactness:
//  1. The flag test reads through a *non-volatile* `const s8 *`, while the three
//     header copies read through a non-volatile `const u8 *`. agbcc folds a
//     VOLATILE byte load to the immediate form `ldrb r0,[r1,#7]` (dropping the
//     sign); the plain `const s8 *` subscript is what yields the ROM's
//     two-instruction `movs r0,#7` + `ldrsb r0,[r1,r0]` re-materialisation.
//     The `u8` view must stay plain or the three header copies lose their
//     immediate offsets too. Neither view needs volatile: this is a tuning
//     block, not hardware, and the five indices are distinct so nothing is CSE'd.
//  2. The accumulator is `register... __asm__("r2")`. Without the pin agbcc
//     reloads into r0 and emits `ldrb r0,[r3,#3]` + `adds r2,r0,#0`, which is
//     2 bytes too long per bit block (candidate 108 B vs ROM 96 B). The pin
//     makes the volatile reload land directly in the OR destination.
// ----------------------------------------------------------------------------
void _08001A44C(void *dst_, const void *src_) {
    volatile u8 *dst = (volatile u8 *)dst_;
    const u8 *src = (const u8 *)src_;
    const s8 *cp = (const s8 *)src;
    register u8 b __asm__("r2");
    dst[0] = src[2];
    dst[1] = src[6];
    dst[2] = src[4];
    b = dst[3];
    if (cp[7] != 0)
        b |= 1u;
    dst[3] = (u8)b;
    b = dst[3];
    if (cp[3] != 0)
        b |= 2u;
    dst[3] = (u8)b;
    b = dst[3];
    if (cp[8] != 0)
        b |= 4u;
    dst[3] = (u8)b;
    b = dst[3];
    if (cp[9] != 0)
        b |= 8u;
    dst[3] = (u8)b;
    b = dst[3];
    if (cp[5] != 0)
        b |= 16u;
    dst[3] = (u8)b;
}

// ----------------------------------------------------------------------------
// ---- 0x0801A768 — race-scene setup: fills, flag arms, record loop, ----
// ---- award-lane series, veneer pickup, ghost-gated 28-byte copy ----
// Pools: {0x0500015A, 0x03002864, 0x05000006, WA+0x10BD/0x10FE/0x10C2/
//   0x10FC/0x10CA/0x10DC/0x1114, 0x080CBAA4, 0x03004E80, 0x05000047,
//   0x0805FB9E, 0x0805FBA6, 0x080CBB28, 0x03004EA4, CTX, 0x534,
//   0x080CBB2C, 0x54C}.
// r0=a (record), r1=b (zero-path flag), r2=c (0x8000-arm flag).
// NOTE: r4 holds `a` until the WA+0x10CA loop, then is reused as the loop
// counter whose final value lands in WA+0x10DC (0 here — single pass over
// the loop writes nothing else to r4 before the store).
// ----------------------------------------------------------------------------
__attribute__((naked)) void _08001A768(void *a_, int b, int c) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, sl\n"
        "\tmov\tr6, r9\n"
        "\tmov\tr5, r8\n"
        "\tpush\t{r5, r6, r7}\n"
        "\tsub\tsp, #48\n"
        "\tadds\tr4, r0, #0\n"
        "\tadds\tr5, r1, #0\n"
        "\tmov\tsl, r2\n"
        "\tmovs\tr0, #0\n"
        "\tstr\tr0, [sp, #0]\n"
        "\tldr r2, .L1a768_798\n"
        "\tmov\tr0, sp\n"
        "\tadds\tr1, r4, #0\n"
        "\tbl 0x0802D974\n"
        "\tcmp\tr5, #0\n"
        "\tbeq .L1a768_79c\n"
        "\tmovs\tr0, #32\n"
        "\tmovs\tr1, #1\n"
        "\tbl 0x08018AA8\n"
        "\tb .L1a768_7a8\n"
        "\tmovs\tr0, r0\n"
        ".L1a768_798: .4byte 0x0500015A\n"
        ".L1a768_79c:\n"
        "\tstr\tr5, [sp, #4]\n"
        "\tadd\tr0, sp, #4\n"
        "\tldr r1, .L1a768_a4c\n"
        "\tldr r2, .L1a768_a50\n"
        "\tbl 0x0802D974\n"
        ".L1a768_7a8:\n"
        "\tldr r1, .L1a768_a54\n"
        "\tldr r2, .L1a768_a58\n"
        "\tadds\tr0, r1, r2\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tadd\tr3, sp, #8\n"
        "\tmov\tr8, r3\n"
        "\tadd\tr7, sp, #16\n"
        "\tadds\tr6, r4, #0\n"
        "\tadds\tr6, #132\n"
        "\tstr\tr6, [sp, #32]\n"
        "\tadds\tr2, r4, #0\n"
        "\tadds\tr2, #86\n"
        "\tstr\tr2, [sp, #28]\n"
        "\tadds\tr3, r4, #0\n"
        "\tadds\tr3, #140\n"
        "\tstr\tr3, [sp, #36]\n"
        "\tadds\tr6, #10\n"
        "\tstr\tr6, [sp, #40]\n"
        "\tsubs\tr2, #22\n"
        "\tstr\tr2, [sp, #24]\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .L1a768_836\n"
        "\tldr r3, .L1a768_a5c\n"
        "\tadds\tr2, r1, r3\n"
        "\tmovs\tr6, #0\n"
        "\tldrsh\tr0, [r2, r6]\n"
        "\tcmp\tr0, #15\n"
        "\tble .L1a768_836\n"
        "\tldr r1, .L1a768_a60\n"
        "\tsubs\tr0, #16\n"
        "\tlsls\tr0, r0, #2\n"
        "\tadds\tr0, r0, r1\n"
        "\tldr\tr3, [r0, #0]\n"
        "\tldrb\tr1, [r3, #0]\n"
        "\tadds\tr6, r4, #0\n"
        "\tadds\tr6, #102\n"
        "\tstr\tr6, [sp, #44]\n"
        "\tmovs\tr0, #164\n"
        "\tadds\tr0, r0, r4\n"
        "\tmov\tip, r0\n"
        "\tmovs\tr2, #168\n"
        "\tadds\tr2, r2, r4\n"
        "\tmov\tr9, r2\n"
        "\tcmp\tr1, #0\n"
        "\tbeq .L1a768_820\n"
        "\tadds\tr2, r6, #0\n"
        "\tmovs\tr6, #148\n"
        "\tlsls\tr6, r6, #3\n"
        "\tadds\tr5, r4, r6\n"
        ".L1a768_80a:\n"
        "\tmovs\tr6, #0\n"
        "\tldrsh\tr0, [r2, r6]\n"
        "\tadds\tr0, r5, r0\n"
        "\tstrb\tr1, [r0, #0]\n"
        "\tldrh\tr0, [r2, #0]\n"
        "\tadds\tr0, #1\n"
        "\tstrh\tr0, [r2, #0]\n"
        "\tadds\tr3, #1\n"
        "\tldrb\tr1, [r3, #0]\n"
        "\tcmp\tr1, #0\n"
        "\tbne .L1a768_80a\n"
        ".L1a768_820:\n"
        "\tldr\tr1, [sp, #44]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tlsls\tr0, r0, #2\n"
        "\tmovs\tr1, #120\n"
        "\tsubs\tr1, r1, r0\n"
        "\tmov\tr3, ip\n"
        "\tstr\tr1, [r3, #0]\n"
        "\tmovs\tr0, #60\n"
        "\tmov\tr6, r9\n"
        "\tstr\tr0, [r6, #0]\n"
        ".L1a768_836:\n"
        "\tmov\tr0, sl\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .L1a768_846\n"
        "\tmovs\tr0, #128\n"
        "\tlsls\tr0, r0, #8\n"
        "\tmovs\tr1, #1\n"
        "\tbl 0x08018AA8\n"
        ".L1a768_846:\n"
        "\tldr r5, .L1a768_a54\n"
        "\tldr r1, .L1a768_a64\n"
        "\tadds\tr0, r5, r1\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .L1a768_866\n"
        "\tmovs\tr0, #128\n"
        "\tlsls\tr0, r0, #17\n"
        "\tmovs\tr1, #1\n"
        "\tbl 0x08018AA8\n"
        "\tmovs\tr0, #128\n"
        "\tlsls\tr0, r0, #18\n"
        "\tmovs\tr1, #1\n"
        "\tbl 0x08018AA8\n"
        ".L1a768_866:\n"
        "\tldr r2, .L1a768_a68\n"
        "\tadds\tr0, r5, r2\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r0, r3]\n"
        "\tcmp\tr0, #3\n"
        "\tbeq .L1a768_882\n"
        "\tcmp\tr0, #5\n"
        "\tbne .L1a768_882\n"
        "\tmovs\tr0, #0\n"
        "\tstr\tr0, [r4, #48]\n"
        "\tstr\tr0, [r4, #52]\n"
        "\tadds\tr1, r4, #0\n"
        "\tadds\tr1, #127\n"
        "\tstrb\tr0, [r1, #0]\n"
        ".L1a768_882:\n"
        "\tmovs\tr4, #0\n"
        "\tldr r0, .L1a768_a54\n"
        "\tldr r6, .L1a768_a6c\n"
        "\tadds\tr1, r0, r6\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tcmp\tr4, r0\n"
        "\tbge .L1a768_8b6\n"
        "\tadds\tr6, r1, #0\n"
        "\tmovs\tr5, #0\n"
        ".L1a768_896:\n"
        "\tmovs\tr0, #0\n"
        "\tstr\tr0, [sp, #4]\n"
        "\tldr r1, .L1a768_a70\n"
        "\tadds\tr1, r5, r1\n"
        "\tadd\tr0, sp, #4\n"
        "\tldr r2, .L1a768_a74\n"
        "\tbl 0x0802D974\n"
        "\tmovs\tr3, #142\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr5, r5, r3\n"
        "\tadds\tr4, #1\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r6, r1]\n"
        "\tcmp\tr4, r0\n"
        "\tblt .L1a768_896\n"
        ".L1a768_8b6:\n"
        "\tbl 0x0802612C\n"
        "\tldr r1, .L1a768_a78\n"
        "\tmov\tr0, r8\n"
        "\tmovs\tr2, #8\n"
        "\tbl 0x0802E0A4\n"
        "\tldr r1, .L1a768_a7c\n"
        "\tadds\tr0, r7, #0\n"
        "\tmovs\tr2, #8\n"
        "\tbl 0x0802E0A4\n"
        "\tadds\tr6, r7, #0\n"
        "\tmov\tr4, r8\n"
        "\tldr\tr5, [sp, #32]\n"
        "\tmovs\tr7, #3\n"
        ".L1a768_8d6:\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r4, r2]\n"
        "\tmovs\tr1, #128\n"
        "\tbl 0x08026150\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r4, r3]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r6, r2]\n"
        "\tbl 0x080261E8\n"
        "\tadds\tr6, #2\n"
        "\tadds\tr4, #2\n"
        "\tadds\tr5, #2\n"
        "\tsubs\tr7, #1\n"
        "\tcmp\tr7, #0\n"
        "\tbge .L1a768_8d6\n"
        "\tmovs\tr0, #4\n"
        "\tmovs\tr1, #32\n"
        "\tbl 0x08026150\n"
        "\tmovs\tr4, #0\n"
        "\tldr\tr3, [sp, #28]\n"
        "\tstrh\tr0, [r3, #0]\n"
        "\tmovs\tr0, #4\n"
        "\tmovs\tr1, #2\n"
        "\tbl 0x080261E8\n"
        "\tmovs\tr0, #7\n"
        "\tmovs\tr1, #64\n"
        "\tbl 0x08026150\n"
        "\tldr\tr6, [sp, #36]\n"
        "\tstrh\tr0, [r6, #0]\n"
        "\tmovs\tr0, #7\n"
        "\tmovs\tr1, #0\n"
        "\tbl 0x080261E8\n"
        "\tmovs\tr0, #8\n"
        "\tmovs\tr1, #64\n"
        "\tbl 0x08026150\n"
        "\tldr\tr1, [sp, #40]\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr0, #8\n"
        "\tmovs\tr1, #1\n"
        "\tbl 0x080261E8\n"
        "\tmovs\tr0, #5\n"
        "\tmovs\tr1, #32\n"
        "\tbl 0x08026150\n"
        "\tmovs\tr0, #5\n"
        "\tmovs\tr1, #3\n"
        "\tbl 0x080261E8\n"
        "\tmovs\tr0, #6\n"
        "\tmovs\tr1, #32\n"
        "\tbl 0x08026150\n"
        "\tmovs\tr0, #6\n"
        "\tmovs\tr1, #4\n"
        "\tbl 0x080261E8\n"
        "\tmovs\tr0, #10\n"
        "\tmovs\tr1, #70\n"
        "\tbl 0x08026150\n"
        "\tmovs\tr0, #10\n"
        "\tmovs\tr1, #11\n"
        "\tbl 0x080261E8\n"
        "\tmovs\tr0, #11\n"
        "\tmovs\tr1, #2\n"
        "\tbl 0x08026150\n"
        "\tmovs\tr0, #11\n"
        "\tmovs\tr1, #12\n"
        "\tbl 0x080261E8\n"
        "\tmovs\tr0, #9\n"
        "\tmovs\tr1, #51\n"
        "\tbl 0x08026150\n"
        "\tmovs\tr0, #9\n"
        "\tmovs\tr1, #13\n"
        "\tbl 0x080261E8\n"
        "\tmovs\tr0, #12\n"
        "\tmovs\tr1, #16\n"
        "\tbl 0x08026150\n"
        "\tmovs\tr0, #12\n"
        "\tmovs\tr1, #9\n"
        "\tbl 0x080261E8\n"
        "\tmovs\tr0, #13\n"
        "\tmovs\tr1, #16\n"
        "\tbl 0x08026150\n"
        "\tmovs\tr0, #13\n"
        "\tmovs\tr1, #10\n"
        "\tbl 0x080261E8\n"
        "\tbl sub_08001A4AC\n"
        "\tmovs\tr0, #1\n"
        "\tldr\tr2, [sp, #24]\n"
        "\tstrh\tr0, [r2, #0]\n"
        "\tldr r5, .L1a768_a54\n"
        "\tldr r3, .L1a768_a80\n"
        "\tadds\tr0, r5, r3\n"
        "\tstr\tr4, [r0, #0]\n"
        "\tldr r4, .L1a768_a84\n"
        "\tmov\tr9, r4\n"
        "\tldr r6, .L1a768_a88\n"
        "\tldr r4, .L1a768_a8c\n"
        "\tldr\tr1, [r4, #0]\n"
        "\tmovs\tr7, #163\n"
        "\tlsls\tr7, r7, #3\n"
        "\tadds\tr1, r1, r7\n"
        "\tmov\tr0, r9\n"
        "\tldr\tr2, [r0, #0]\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl 0x0802DDD0\n"
        "\tldr\tr1, [r4, #0]\n"
        "\tldr r2, .L1a768_a90\n"
        "\tmov\tr8, r2\n"
        "\tadds\tr0, r1, r2\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tadds\tr1, #126\n"
        "\tstrb\tr0, [r1, #0]\n"
        "\tldr r3, .L1a768_a94\n"
        "\tadds\tr0, r5, r3\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tsubs\tr0, #94\n"
        "\tlsls\tr0, r0, #16\n"
        "\tlsrs\tr0, r0, #16\n"
        "\tcmp\tr0, #2\n"
        "\tbhi .L1a768_a00\n"
        "\tmovs\tr0, #128\n"
        "\tlsls\tr0, r0, #24\n"
        "\tmovs\tr1, #1\n"
        "\tbl 0x08018AA8\n"
        ".L1a768_a00:\n"
        "\tldr r1, .L1a768_a68\n"
        "\tadds\tr0, r5, r1\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tcmp\tr0, #8\n"
        "\tbne .L1a768_a3a\n"
        "\tldr r0, .L1a768_a98\n"
        "\tldr\tr1, [r0, #0]\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl 0x0802DDCC\n"
        "\tldr\tr1, [r4, #0]\n"
        "\tadds\tr1, r1, r7\n"
        "\tmov\tr3, r9\n"
        "\tldr\tr2, [r3, #0]\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl 0x0802DDD0\n"
        "\tldr\tr1, [r4, #0]\n"
        "\tmov\tr4, r8\n"
        "\tadds\tr0, r1, r4\n"
        "\tldr r6, .L1a768_a9c\n"
        "\tadds\tr1, r1, r6\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tldmia\tr0!, {r2, r3, r4}\n"
        "\tstmia\tr1!, {r2, r3, r4}\n"
        "\tldmia\tr0!, {r2, r3, r6}\n"
        "\tstmia\tr1!, {r2, r3, r6}\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tstr\tr0, [r1, #0]\n"
        ".L1a768_a3a:\n"
        "\tadd\tsp, #48\n"
        "\tpop\t{r3, r4, r5}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tmov\tsl, r5\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".L1a768_a4c: .4byte 0x03002864\n"
        ".L1a768_a50: .4byte 0x05000006\n"
        ".L1a768_a54: .4byte 0x03001780\n"
        ".L1a768_a58: .4byte 0x000010BD\n"
        ".L1a768_a5c: .4byte 0x000010FE\n"
        ".L1a768_a60: .4byte 0x080CBAA4\n"
        ".L1a768_a64: .4byte 0x000010C2\n"
        ".L1a768_a68: .4byte 0x000010FC\n"
        ".L1a768_a6c: .4byte 0x000010CA\n"
        ".L1a768_a70: .4byte 0x03004E80\n"
        ".L1a768_a74: .4byte 0x05000047\n"
        ".L1a768_a78: .4byte 0x0805FB9E\n"
        ".L1a768_a7c: .4byte 0x0805FBA6\n"
        ".L1a768_a80: .4byte 0x000010DC\n"
        ".L1a768_a84: .4byte 0x080CBB28\n"
        ".L1a768_a88: .4byte 0x03004EA4\n"
        ".L1a768_a8c: .4byte 0x03004E20\n"
        ".L1a768_a90: .4byte 0x00000534\n"
        ".L1a768_a94: .4byte 0x00001114\n"
        ".L1a768_a98: .4byte 0x080CBB2C\n"
        ".L1a768_a9c: .4byte 0x0000054C\n"
        ".syntax divided\n"
    );
}

// ----------------------------------------------------------------------------
// ---- 0x0801B068 — start/select arm + record init + racer walk ----
// Pools: {CTX, WA+0x10FE/0x10CA/0x10FC, 0x03004E80}.
// r0=a: nonzero → flag-4 set path (096A4 over [ctx]+860, u16[[ctx]+122]=5);
// zero → flag-4 clear path (026FF4 over ctx+0x41C, 284-stride 270D8 walk,
// 27210 select on u16[WA+0x10FC]==5).
//
// Four constraints, each carried by a distinct lever:
// - then-path: the ROM holds the CTX *address* (pool word 0x03004E20) in r4
//   from its pool load through both reloads, so the `ldr r0,[r4]` / `adds`
//   pair re-reads the cell rather than the memoised value. Left unpinned,
//   agbcc dereferences into r4 and keeps the pooled address in r0, which
//   also moves the u16 store's constant from r1 to r0 and its address off
//   r0 (4 instructions). Pin the cell to r4; re-dereference before +122.
// - else-path: WORK_AREA is reached as base+offset from TWO pool words
//   (`ldr r2,=0x03001780` / `ldr r3,=0x10FE` / `adds r1,r2,r3`), once per
//   offset, with the base live in r2 across the 026FF4/270D8 calls. A
//   folded `WORK_AREA + 0x10FE` pools as the single word 0x0300277E and
//   loses both the adds and the shared base. Each operand is therefore its
//   own assembler absolute symbol (docs/findings/track_car_26180_pool_order.md).
// - the two WA signed-halfword reads use ordered inline assembly. The first
//   reads Wa+0x10FE before loading the 0x10CA offset; both use r3 as the
//   zero-index and the second leaves its address in r4 for the runner loop.
//   This keeps agbcc's reload pass from sinking the first read below the
//   second address calculation.
// - the walk re-reads its bound from *pca in the condition (guard via r4
//   before the body repurposes r4 as the 03004E80 runner, back-edge via the
//   r6 copy the allocator makes), so the bound must stay a memory read, not
//   a cached `s16 n` local. The r6 copy itself needs the source split into
//   `s16 *cnt = pca;` -- with the loop reading `*pca` directly agbcc shares
//   ONE test (entry `b` + back-edge `blt`) instead of the ROM's duplicated
//   guard+back-edge pair, which is 4 instructions of layout.
//
// - the runner's two input words are volatile C reads to keep q[0] before
//   q[1]. Its 284-byte stride is formed in r3 as 142<<1, then added to r4.
//   The final WA+0x10FC read keeps the base in r0 and offset in r3.
//
// The focused corpus probe matches all 200 bytes, including the literal pool.
// ----------------------------------------------------------------------------
void _08001B068(int a) {
    if (a != 0) {
        _08018AA8(4u, 1);
        {
            register u32 cell __asm__("r4");
            cell = CTX;
            _080096A4((void *)(uintptr_t)(*(volatile u32 *)(uintptr_t)cell + 860u));
            *(volatile u16 *)(uintptr_t)(*(volatile u32 *)(uintptr_t)cell + 122u) = 5;
        }
        _0802B460();
        _0802B25C();
    } else {
        extern u8 WaBase[], WaOffFE[], WaOffCA[], WaOffFC[];
        u32 rec;
        register u32 off __asm__("r3");
        register u32 wb __asm__("r2");
        register s16 *pca __asm__("r4");
        register s32 a2 __asm__("r1");
        register int i __asm__("r5");
        __asm__(".globl WaBase\nWaBase = 0x03001780\n");
        __asm__(".globl WaOffFE\nWaOffFE = 0x10FE\n");
        __asm__(".globl WaOffCA\nWaOffCA = 0x10CA\n");
        __asm__(".globl WaOffFC\nWaOffFC = 0x10FC\n");
        _08018AA8(4u, 0);
        /* Argument order is also instruction order here: the ROM builds
         * r0 = [CTX]+0x41C before it pools the WORK_AREA base, so the
         * caller's first argument must be evaluated first. */
        /* The record offset must be a SYMBOL, not the literal 0x41C: a
         * pinned `off = 0x41C` is copy-propagated away and the pool load
         * lands in r2 (the ROM's r3 is only reachable while `off` still has
         * a live range). */
        { extern u8 RecOff41C[];
          __asm__(".globl RecOff41C\nRecOff41C = 0x41C\n");
          rec = *(volatile u32 *)(uintptr_t)CTX;
          off = (u32)(uintptr_t)RecOff41C;
          rec = rec + off; }
        wb = (u32)(uintptr_t)WaBase;
        off = (u32)(uintptr_t)WaOffFE;
        a2 = wb + off;
        __asm__ volatile("" : "+r"(a2));
        off = 0;
        a2 = *(s16 *)(uintptr_t)(a2 + off);
        off = (u32)(uintptr_t)WaOffCA;
        pca = (s16 *)(uintptr_t)(wb + off);
        off = 0;
        wb = (u32)*pca;
        _080026FF4((void *)(uintptr_t)rec, (int)a2, (int)wb, 0);
        /* Written as the rotated form the ROM carries EXACTLY: the entry
         * guard reads the bound through r4 (the register the bound pointer
         * holds from its pool add), then the 03004E80 runner claims r4 for
         * the body and the allocator copies the bound pointer to r6 at the
         * loop entry (`adds r6,r4,#0`), which is what the back-edge test
         * reads. A plain `for` here makes agbcc share one test instead of
         * duplicating it (entry `b` + back-edge `blt`) and the guard read
         * ends up in the same register as the tail. */
        i = 0;
        if (i < (int)*pca) {
            s16 *cnt = pca;
            s32 loopLimit;
            register u32 *q __asm__("r4") = (u32 *)0x03004E80u;
            do {
                u32 x = *(volatile u32 *)(uintptr_t)q;
                u32 y = *(volatile u32 *)(uintptr_t)((u8 *)q + 4);
                _080270D8(x, y, i);
                off = 142;
                __asm__ volatile ("" : "+r"(off));
                off <<= 1;
                q = (u32 *)((u8 *)q + off);
                i++;
                loopLimit = (int)*cnt;
                __asm__ volatile("" : : "r"(off));
            } while (i < loopLimit);
        }
        {
            register u32 wa_base __asm__("r0") = (u32)(uintptr_t)WaBase;
            register u32 wa_offset __asm__("r3") = (u32)(uintptr_t)WaOffFC;
            __asm__ volatile("" : "+r"(wa_base), "+r"(wa_offset)
                             : : "memory");
            wa_base += wa_offset;
            __asm__ volatile("" : "+r"(wa_base) : : "cc");
            if (*(volatile u16 *)(uintptr_t)wa_base == 5)
                _08027210(0);
            else
                _08027210(1);
        }
        _0802B474();
        _0802B280();
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801B27C — key-gated B068/flag driver + counter cell machine ----
// Pools: {WORK_AREA, 0x10C2, CTX}. r0 dead, r1 = u16 key (truncated on entry
// into r4, which stays live for the whole body).
// Flag masks are 0x100000 / 0x200000 for the key==8 arm and 0x1000000 /
// 0x2000000 for the counter-cell arm — each materialised as `movs r0,#128`
// plus a single lsls, which is what fixes the shift amounts at 13/14/17/18.
// The work-area byte is reached as base + 0x10C2 from TWO pool words, so it
// must be an extern object indexed by a constant, not a folded integer.
// The counter cell is at CTX+110, read back signed through a struct field so
// the load is `movs r2,#0 / ldrsh r1,[r1,r2]` rather than ldrh+lsls+asrs.
// ----------------------------------------------------------------------------
void _08001B27C(int unused, u16 key) {
    struct Cell { s16 v; };
    extern volatile u8 RaceWA_1B27C[];
    (void)unused;
    __asm__("RaceWA_1B27C = 0x03001780");
    if (_08018ACC(0x100000u) != 0 && _08018ACC(8u) != 0) {
        _08001B068(1);
    } else if (key == 8 && _08018ACC(8u) != 0) {
        _08018AA8(0x100000u, 1);
        _08018AA8(0x200000u, 1);
    }
    if (((u32)key & 4u) != 0 && RaceWA_1B27C[0x10C2u] != 0) {
        /* The ROM holds the CTX *address* (pool word 0x03004E20) in r0 from the
         * `ldr r0,=CTX` at 0x2D6 through to the reload `ldr r0,[r0]` at 0x308,
         * so r0 is live across the whole cell machine. Left to itself agbcc's
         * local-alloc gives the base the last free register (r2), which also
         * costs the ldrsh its dead-temp index register (r3, not r2) and pushes
         * the switch value into r0. The pin is the same `register T v
         * __asm__("rN")` lever src/agbmain_session.c:110 documents. */
        register volatile u32 *ctxp __asm__("r0") = (volatile u32 *)(uintptr_t)CTX;
        register struct Cell *cell __asm__("r1") =
            (struct Cell *)(uintptr_t)(*ctxp + 110u);
        s16 v;
        *(volatile u16 *)(uintptr_t)cell = (u16)(*(volatile u16 *)(uintptr_t)cell + 1u);
        v = cell->v;
        switch (v) {
        case 0:
        case 3:
            *(volatile u16 *)(uintptr_t)(*ctxp + 110u) = 0;
            _08018AA8(0x1000000u, 1);
            _08018AA8(0x2000000u, 1);
            break;
        case 1:
            _08018AA8(0x1000000u, 0);
            _08018AA8(0x2000000u, 1);
            break;
        case 2:
            _08018AA8(0x1000000u, 0);
            _08018AA8(0x2000000u, 0);
            break;
        default:
            break;
        }
    }
    if (((u32)key & 9u) != 0 && _08018ACC(32u) != 0)
        _08018AA8(2u, 1);
}
__asm__(".align 2, 0");

// The 0x0801B27C span carries a twin label pair in the closure
// (asm/race_scene.s:1803 `sub_08001B27C:` immediately followed by
// `_08001B27C:`), and retained asm still calls the `sub_` spelling at
// asm/race_scene.s:1994 (`bl sub_08001B27C`). The splice deletes both labels
// and re-emits only a `.thumb_set` agbcc actually wrote, so C must define the
// `sub_` spelling or the independent link fails with an undefined reference.
// Alias the real body in ONE hop (chaining through a second alias is a known
// link failure).
void sub_08001B27C(int unused, u16 key) __attribute__((alias("_08001B27C")));
extern void _08001B27C_wide(u32 a, u32 b) __asm__("_08001B27C");
extern int _080098C8_wide(u32 a, u32 b) __asm__("_080098C8");

// ----------------------------------------------------------------------------
// ---- 0x0801B364 — two-u16 event router (ev9 body: 1B1A4/1B27C) ----
// Pools: {WA+0x10FC, CTX}. r0/r1 truncated to u16 on entry.
// WA+0x10FC==8 with positive s16[[ctx]+106] returns at once; else
// ACC(4) picks B1A4 (flag-0x8000 clear or flag-0x4000 set → B130(0)+2;
// else ACC(0x4000) re-test then [ctx]+122 decrement or 098C8) vs B27C.
// ----------------------------------------------------------------------------
void _08001B364(u32 a, u32 b) {
    u32 arg_a;
    u32 arg_b;
    arg_b = (u16)a;
    arg_a = (u16)b;
    register u32 wa_base __asm__("r0") = 0x03001780u;
    register u32 wa_offset __asm__("r1") = 0x10FCu;
    __asm__ volatile ("" : "+r" (wa_base), "+r" (wa_offset));
    register volatile u16 *wa_mode __asm__("r5") =
        (volatile u16 *)(uintptr_t)(wa_base + wa_offset);
    register u32 initial_mode __asm__("r3");
    initial_mode = *wa_mode;
    __asm__ volatile ("" : "+r" (initial_mode));

    if (initial_mode == 8) {
        u8 *ctx = *(u8 **)(uintptr_t)0x03004E20u;
        if (*(s16 *)(ctx + 106) > 0)
            return;
    }
    if (_08018ACC(4) != 0) {
        register int action __asm__("r4") = 0;
        register u32 mode __asm__("r5");
        mode = *wa_mode;
        __asm__ volatile ("" : "+r" (mode));
        if (mode == 8) {
            if (_08018ACC(0x800000u) != 0 || _08018ACC(0x400000u) != 0) {
                _08001B130(0);
                action = 2;
                goto dispatch;
            }
        }
        if (_08018ACC(0x200000u) != 0) {
            register u8 *ctx __asm__("r0") =
                *(u8 **)(uintptr_t)0x03004E20u;
            register volatile u16 *count __asm__("r1") =
                (volatile u16 *)(ctx + 122);
            register u32 count_value __asm__("r2");
            count_value = *count;
            __asm__ volatile ("" : "+r" (count_value));
            register u32 zero __asm__("r3") = 0;
            register int signed_count __asm__("r0");
            signed_count = ((s16 *)(uintptr_t)count)[zero];
            if (signed_count > 0) {
                register int decremented __asm__("r0") = count_value - 1;
                *count = (u16)decremented;
            }
            else {
                register int call_result __asm__("r0");
                call_result = _080098C8_wide(arg_b, arg_a);
                action = call_result;
            }
        }
    dispatch:
        _08001B1A4(action);
    } else {
        _08001B27C_wide(arg_b, arg_a);
    }
}
void sub_08001B364(u32 a, u32 b) __attribute__((alias("_08001B364")));

// ----------------------------------------------------------------------------
// ---- 0x0801B498 — 6-way select dispatcher + case bodies ----
// Pools: {WA+0x10FC, CTX, 0x03004EA4, table 0x0801B550, WA+0x10FE/0x1114/
//   0x10F0}. r0=sel (u16 store to [[ctx]+88]).
// Guards: WA+0x10FC==8 && WA+0x10C3==0 && sel==5 && [[ctx]+128]==0 clears
// flag 1; ACC(1) nonzero returns; else flag 1/128/8 arms, two 2A86C claims
// (8/16), [[ctx]+78]=0, WA+0x10FC==5 with positive [ctx+48] timed reset,
// then sel 0..5 dispatches (switch = table 0x0801B550).
// ----------------------------------------------------------------------------
__attribute__((naked)) void _08001B498(int sel) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, r6, r7, lr}\n"
        "adds	r6, r0, #0\n"
        "ldr r1, .Lrs1b4_53c\n"
        "ldr r2, .Lrs1b4_540\n"
        "adds	r0, r1, r2\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #8\n"
        "bne .Lrs1b4_4ca\n"
        "subs	r2, #57\n"
        "adds	r0, r1, r2\n"
        "ldrb	r0, [r0, #0]\n"
        "cmp	r0, #0\n"
        "bne .Lrs1b4_4ca\n"
        "cmp	r6, #5\n"
        "bne .Lrs1b4_4ca\n"
        "ldr r0, .Lrs1b4_544\n"
        "ldr	r0, [r0, #0]\n"
        "adds	r0, #128\n"
        "ldrb	r0, [r0, #0]\n"
        "cmp	r0, #0\n"
        "bne .Lrs1b4_4ca\n"
        "movs	r0, #1\n"
        "movs	r1, #0\n"
        "bl 0x08018AA8\n"
        ".Lrs1b4_4ca:\n"
        "movs	r0, #1\n"
        "bl 0x08018ACC\n"
        "adds	r7, r0, #0\n"
        "cmp	r7, #0\n"
        "beq .Lrs1b4_4d8\n"
        "b .Lrs1b4_71a\n"
        ".Lrs1b4_4d8:\n"
        "ldr r5, .Lrs1b4_544\n"
        "ldr	r0, [r5, #0]\n"
        "adds	r0, #88\n"
        "strh	r6, [r0, #0]\n"
        "movs	r0, #1\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "movs	r0, #128\n"
        "movs	r1, #0\n"
        "bl 0x08018AA8\n"
        "movs	r0, #8\n"
        "movs	r1, #0\n"
        "bl 0x08018AA8\n"
        "ldr r4, .Lrs1b4_548\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #8\n"
        "movs	r2, #0\n"
        "bl sub_08002A86C\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #16\n"
        "movs	r2, #0\n"
        "bl sub_08002A86C\n"
        "ldr	r5, [r5, #0]\n"
        "adds	r0, r5, #0\n"
        "adds	r0, #78\n"
        "strh	r7, [r0, #0]\n"
        "ldr r0, .Lrs1b4_53c\n"
        "ldr r1, .Lrs1b4_540\n"
        "adds	r0, r0, r1\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #5\n"
        "bne .Lrs1b4_52c\n"
        "ldr	r0, [r5, #48]\n"
        "cmp	r0, #0\n"
        "ble .Lrs1b4_52c\n"
        "bl sub_0800821C\n"
        ".Lrs1b4_52c:\n"
        "cmp	r6, #5\n"
        "bls .Lrs1b4_532\n"
        "b .Lrs1b4_71a\n"
        ".Lrs1b4_532:\n"
        "lsls	r0, r6, #2\n"
        "ldr r1, .Lrs1b4_54c\n"
        "adds	r0, r0, r1\n"
        "ldr	r0, [r0, #0]\n"
        "mov	pc, r0\n"
        ".Lrs1b4_53c: .word 0x03001780\n"
        ".Lrs1b4_540: .word 0x000010FC\n"
        ".Lrs1b4_544: .word 0x03004E20\n"
        ".Lrs1b4_548: .word 0x03004EA4\n"
        ".Lrs1b4_54c: .word 0x0801B550\n"
        ".Lrs1b4_550: .word 0x0801B568\n"
        ".Lrs1b4_554: .word 0x0801B658\n"
        ".Lrs1b4_558: .word 0x0801B698\n"
        ".Lrs1b4_55c: .word 0x0801B698\n"
        ".Lrs1b4_560: .word 0x0801B6B8\n"
        ".Lrs1b4_564: .word 0x0801B6E4\n"
        ".Lrs1b4_568:\n"
        "ldr r4, .Lrs1b4_5c0\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #4\n"
        "movs	r2, #1\n"
        "bl sub_08002A86C\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrs1b4_580\n"
        "b .Lrs1b4_71a\n"
        ".Lrs1b4_580:\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #2\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "bl 0x0802B44C\n"
        "bl 0x0802B234\n"
        "movs	r0, #64\n"
        "bl 0x0802B368\n"
        "movs	r0, #2\n"
        "negs	r0, r0\n"
        "bl 0x08027114\n"
        "ldr r0, .Lrs1b4_5c4\n"
        "ldr r2, .Lrs1b4_5c8\n"
        "adds	r0, r0, r2\n"
        "movs	r1, #0\n"
        "ldrsh	r0, [r0, r1]\n"
        "cmp	r0, #1\n"
        "beq .Lrs1b4_5b2\n"
        "cmp	r0, #4\n"
        "bne .Lrs1b4_5cc\n"
        ".Lrs1b4_5b2:\n"
        "adds	r0, r4, #0\n"
        "subs	r0, #36\n"
        "movs	r2, #14\n"
        "ldrsh	r0, [r0, r2]\n"
        "bl 0x08019AEC\n"
        "b .Lrs1b4_5d2\n"
        ".Lrs1b4_5c0: .word 0x03004EA4\n"
        ".Lrs1b4_5c4: .word 0x03001780\n"
        ".Lrs1b4_5c8: .word 0x000010FC\n"
        ".Lrs1b4_5cc:\n"
        "movs	r0, #1\n"
        "bl 0x08019AEC\n"
        ".Lrs1b4_5d2:\n"
        "ldr r6, .Lrs1b4_644\n"
        "ldr r1, .Lrs1b4_648\n"
        "adds	r0, r6, r1\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #3\n"
        "beq .Lrs1b4_5e0\n"
        "b .Lrs1b4_71a\n"
        ".Lrs1b4_5e0:\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #5\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrs1b4_5ee\n"
        "b .Lrs1b4_71a\n"
        ".Lrs1b4_5ee:\n"
        "bl 0x080240C4\n"
        "adds	r7, r0, #0\n"
        "bl 0x080240D0\n"
        "adds	r5, r0, #0\n"
        "ldr r2, .Lrs1b4_64c\n"
        "adds	r0, r6, r2\n"
        "movs	r1, #0\n"
        "ldrsh	r0, [r0, r1]\n"
        "bl 0x08024104\n"
        "ldr r4, .Lrs1b4_650\n"
        "ldr	r0, [r4, #0]\n"
        "ldr	r0, [r0, #24]\n"
        "bl 0x080240F8\n"
        "ldr	r0, [r4, #0]\n"
        "ldr	r0, [r0, #44]\n"
        "bl 0x080240EC\n"
        "ldr r2, .Lrs1b4_654\n"
        "adds	r0, r6, r2\n"
        "bl 0x08024110\n"
        "ldrh	r0, [r5, #0]\n"
        "cmp	r0, #0\n"
        "beq .Lrs1b4_636\n"
        "ldrh	r0, [r7, #2]\n"
        "ldrh	r1, [r5, #2]\n"
        "cmp	r0, r1\n"
        "bne .Lrs1b4_636\n"
        "ldr	r1, [r7, #4]\n"
        "ldr	r0, [r5, #4]\n"
        "cmp	r1, r0\n"
        "bge .Lrs1b4_71a\n"
        ".Lrs1b4_636:\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #8\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "b .Lrs1b4_71a\n"
        "movs	r0, r0\n"
        ".Lrs1b4_644: .word 0x03001780\n"
        ".Lrs1b4_648: .word 0x000010FC\n"
        ".Lrs1b4_64c: .word 0x000010FE\n"
        ".Lrs1b4_650: .word 0x03004E20\n"
        ".Lrs1b4_654: .word 0x00001114\n"
        ".Lrs1b4_658:\n"
        "ldr r0, .Lrs1b4_694\n"
        "movs	r1, #2\n"
        "movs	r2, #1\n"
        "bl sub_08002A86C\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrs1b4_71a\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #2\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "bl 0x0802B44C\n"
        "bl 0x0802B234\n"
        "movs	r0, #2\n"
        "negs	r0, r0\n"
        "bl 0x08027114\n"
        "movs	r0, #1\n"
        "bl 0x08019AEC\n"
        "movs	r0, #64\n"
        "bl 0x0802B368\n"
        "b .Lrs1b4_71a\n"
        ".Lrs1b4_694: .word 0x03004EA4\n"
        ".Lrs1b4_698:\n"
        "ldr r0, .Lrs1b4_6b4\n"
        "movs	r1, #2\n"
        "movs	r2, #1\n"
        "bl sub_08002A86C\n"
        "bl 0x0802B44C\n"
        "bl 0x0802B234\n"
        "movs	r0, #1\n"
        "negs	r0, r0\n"
        "bl 0x08019AEC\n"
        "b .Lrs1b4_71a\n"
        ".Lrs1b4_6b4: .word 0x03004EA4\n"
        ".Lrs1b4_6b8:\n"
        "ldr r4, .Lrs1b4_6e0\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #4\n"
        "movs	r2, #1\n"
        "bl sub_08002A86C\n"
        "bl 0x0802B44C\n"
        "bl 0x0802B234\n"
        "movs	r0, #64\n"
        "bl 0x0802B368\n"
        "subs	r4, #36\n"
        "movs	r0, #1\n"
        "strh	r0, [r4, #14]\n"
        "bl 0x08019AEC\n"
        "b .Lrs1b4_71a\n"
        "movs	r0, r0\n"
        ".Lrs1b4_6e0: .word 0x03004EA4\n"
        ".Lrs1b4_6e4:\n"
        "ldr r4, .Lrs1b4_720\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #2\n"
        "movs	r2, #1\n"
        "bl sub_08002A86C\n"
        "bl 0x0802B44C\n"
        "bl 0x0802B234\n"
        "bl 0x0802B3A4\n"
        "subs	r4, #36\n"
        "movs	r5, #0\n"
        "movs	r0, #2\n"
        "strh	r0, [r4, #14]\n"
        "bl 0x08019AEC\n"
        "ldr r0, .Lrs1b4_724\n"
        "ldr r2, .Lrs1b4_728\n"
        "adds	r0, r0, r2\n"
        "str	r5, [r0, #0]\n"
        "ldr r0, .Lrs1b4_72c\n"
        "ldr	r0, [r0, #0]\n"
        "adds	r0, #128\n"
        "movs	r1, #1\n"
        "strb	r1, [r0, #0]\n"
        ".Lrs1b4_71a:\n"
        "pop	{r4, r5, r6, r7}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        ".Lrs1b4_720: .word 0x03004EA4\n"
        ".Lrs1b4_724: .word 0x03001780\n"
        ".Lrs1b4_728: .word 0x000010F0\n"
        ".Lrs1b4_72c: .word 0x03004E20\n"
        ".syntax divided\n"
    );
}

// Spelling aliases for the 0x08001B498 event dispatcher (race_scene.c uses
// the Sub_ spelling).
void sub_08001B498(int v) __attribute__((alias("_08001B498")));
void Sub_08001B498(int v) __attribute__((alias("_08001B498")));

// ----------------------------------------------------------------------------
// ---- 0x0801B730 — interior of sub_08001B498: threshold test leaf ----
// No pools. r0=record (word at +12), r1=flag cell (bit 1 tested).
// Returns 1 when the flag bit is set and (empty marker byte at
// [[CTX]+126] or s16[[CTX]+64]-1 below the scaled +12 word).
// ----------------------------------------------------------------------------
int _08001B730(void *rec_, void *cell_) {
    struct Cell { s16 v; };
    volatile u8 *rec = (volatile u8 *)rec_;
    volatile u32 *cell = (volatile u32 *)cell_;
    u32 ctxval;
    int d;
    register int v __asm__("r0");
    register int base __asm__("r1");
    if ((*cell & 2u) == 0)
        return 0;
    ctxval = *(volatile u32 *)(uintptr_t)CTX;
    if (*(volatile u8 *)(uintptr_t)(ctxval + 126u) == 0)
        return 1;
    d = (int)((struct Cell *)(uintptr_t)(ctxval + 64u))->v - 1;
    base = (int)*(volatile u32 *)(uintptr_t)((uintptr_t)rec + 12u);
    v = base + 2048;
    if (v < 0)
        v = base + 0x17FF;
    v >>= 12;
    if (d >= v)
        return 0;
    return 1;
}

// ----------------------------------------------------------------------------
// ---- 0x0801B77C — grid paint: div-derived sweep over +0x4B4 column ----
// Pools: {CTX, 0x0805FBB0, 0x4B4}. No incoming args.
// Count = DivSI(s16[[ctx]+100]*s16[[ctx]+102], 80)-1; four BE8/C48 pairs
// feed the sp+4 select words; each live lane picks a select word via the
// DF6C remainder and calls 04090 (5 machine args: r3 select + stack 1)
// or 0390C. Tail writes the 0x10000/1536-scaled pair to [[ctx]+16/+20].
// ----------------------------------------------------------------------------
__attribute__((naked)) void _08001B77C(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, r6, r7, lr}\n"
        "mov	r7, sl\n"
        "mov	r6, r9\n"
        "mov	r5, r8\n"
        "push	{r5, r6, r7}\n"
        "sub	sp, #40\n"
        "ldr r0, .Lrs1b7_868\n"
        "ldr	r0, [r0, #0]\n"
        "adds	r1, r0, #0\n"
        "adds	r1, #102\n"
        "movs	r2, #0\n"
        "ldrsh	r1, [r1, r2]\n"
        "adds	r0, #100\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r0, r3]\n"
        "muls	r0, r1\n"
        "movs	r1, #80\n"
        "bl 0x0802DE04\n"
        "adds	r6, r0, #0\n"
        "add	r5, sp, #20\n"
        "adds	r1, r5, #0\n"
        "ldr r0, .Lrs1b7_86c\n"
        "ldmia	r0!, {r2, r3, r4}\n"
        "stmia	r1!, {r2, r3, r4}\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [r1, #0]\n"
        "add	r4, sp, #36\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #0\n"
        "movs	r2, #2\n"
        "bl 0x0802E104\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrs1b7_88e\n"
        "subs	r6, #1\n"
        "mov	sl, r6\n"
        "add	r4, sp, #4\n"
        "movs	r6, #3\n"
        ".Lrs1b7_7d0:\n"
        "bl 0x08002BE8\n"
        "lsls	r0, r0, #16\n"
        "asrs	r0, r0, #16\n"
        "stmia	r4!, {r0}\n"
        "ldmia	r5!, {r1}\n"
        "lsls	r1, r1, #16\n"
        "lsrs	r1, r1, #16\n"
        "bl 0x08002C48\n"
        "subs	r6, #1\n"
        "cmp	r6, #0\n"
        "bge .Lrs1b7_7d0\n"
        "mov	r6, sl\n"
        "cmp	r6, #0\n"
        "blt .Lrs1b7_88e\n"
        "ldr r4, .Lrs1b7_868\n"
        "mov	r9, r4\n"
        "ldr r0, .Lrs1b7_870\n"
        "mov	r8, r0\n"
        "lsls	r7, r6, #3\n"
        ".Lrs1b7_7fa:\n"
        "mov	r1, r9\n"
        "ldr	r0, [r1, #0]\n"
        "add	r0, r8\n"
        "add	r0, sl\n"
        "movs	r1, #12\n"
        "strb	r1, [r0, #0]\n"
        "mov	r2, r9\n"
        "ldr	r0, [r2, #0]\n"
        "add	r0, r8\n"
        "adds	r1, r0, r6\n"
        "ldrb	r0, [r1, #0]\n"
        "cmp	r0, #0\n"
        "beq .Lrs1b7_818\n"
        "subs	r0, #1\n"
        "strb	r0, [r1, #0]\n"
        ".Lrs1b7_818:\n"
        "mov	r3, r9\n"
        "ldr	r1, [r3, #0]\n"
        "movs	r4, #148\n"
        "lsls	r4, r4, #3\n"
        "adds	r0, r1, r4\n"
        "adds	r0, r0, r6\n"
        "ldrb	r0, [r0, #0]\n"
        "add	r2, sp, #36\n"
        "strb	r0, [r2, #0]\n"
        "mov	r3, r8\n"
        "adds	r0, r1, r3\n"
        "adds	r2, r0, r6\n"
        "ldrb	r0, [r2, #0]\n"
        "cmp	r0, #0\n"
        "beq .Lrs1b7_874\n"
        "adds	r0, r1, #0\n"
        "adds	r0, #164\n"
        "ldr	r4, [r0, #0]\n"
        "adds	r4, r4, r7\n"
        "subs	r4, #4\n"
        "adds	r0, #4\n"
        "ldr	r5, [r0, #0]\n"
        "subs	r5, #8\n"
        "ldrb	r0, [r2, #0]\n"
        "movs	r1, #3\n"
        "bl 0x0802DF6C\n"
        "lsls	r0, r0, #24\n"
        "lsrs	r0, r0, #22\n"
        "add	r0, sp\n"
        "adds	r0, #4\n"
        "ldr	r3, [r0, #0]\n"
        "movs	r0, #1\n"
        "str	r0, [sp, #0]\n"
        "adds	r0, r4, #0\n"
        "adds	r1, r5, #0\n"
        "add	r2, sp, #36\n"
        "bl 0x08004090\n"
        "b .Lrs1b7_886\n"
        ".Lrs1b7_868: .word 0x03004E20\n"
        ".Lrs1b7_86c: .word 0x0805FBB0\n"
        ".Lrs1b7_870: .word 0x000004B4\n"
        ".Lrs1b7_874:\n"
        "adds	r0, r1, #0\n"
        "adds	r0, #164\n"
        "ldr	r0, [r0, #0]\n"
        "adds	r0, r0, r7\n"
        "adds	r1, #168\n"
        "ldr	r1, [r1, #0]\n"
        "add	r2, sp, #36\n"
        "bl 0x0800390C\n"
        ".Lrs1b7_886:\n"
        "subs	r7, #8\n"
        "subs	r6, #1\n"
        "cmp	r6, #0\n"
        "bge .Lrs1b7_7fa\n"
        ".Lrs1b7_88e:\n"
        "ldr r0, .Lrs1b7_8d8\n"
        "ldr	r5, [r0, #0]\n"
        "adds	r4, r5, #0\n"
        "adds	r4, #100\n"
        "ldrh	r1, [r4, #0]\n"
        "lsls	r0, r1, #16\n"
        "movs	r1, #160\n"
        "bl 0x0802DE04\n"
        "movs	r1, #128\n"
        "lsls	r1, r1, #9\n"
        "subs	r1, r1, r0\n"
        "str	r1, [r5, #16]\n"
        "movs	r2, #0\n"
        "ldrsh	r1, [r4, r2]\n"
        "lsls	r0, r1, #1\n"
        "adds	r0, r0, r1\n"
        "lsls	r0, r0, #9\n"
        "movs	r1, #160\n"
        "bl 0x0802DE04\n"
        "movs	r1, #192\n"
        "lsls	r1, r1, #3\n"
        "subs	r1, r1, r0\n"
        "str	r1, [r5, #20]\n"
        "cmp	r1, #0\n"
        "bge .Lrs1b7_8c8\n"
        "movs	r0, #0\n"
        "str	r0, [r5, #20]\n"
        ".Lrs1b7_8c8:\n"
        "add	sp, #40\n"
        "pop	{r3, r4, r5}\n"
        "mov	r8, r3\n"
        "mov	r9, r4\n"
        "mov	sl, r5\n"
        "pop	{r4, r5, r6, r7}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        ".Lrs1b7_8d8: .word 0x03004E20\n"
        ".syntax divided\n"
    );
}

// ----------------------------------------------------------------------------
// ---- 0x0801B8DC — +100/+104 state machine over B77C ----
// Pools: {CTX}. s16[[ctx]+100] ticks up while <= 159; s16[[ctx]+104]
// selects: 0 → ACC(32)-gated 19A2C(0); 1 → B77C + miteinander 79/ACC(32)
// gate to 19A2C(0); 2 → B77C + ACC(64)-gated u16[[ctx]+68]=40 +
// flag 0x04000000; 3 → B77C + u16[[ctx]+68] tick-down or flag 0x00080000.
// Every taken arm (except the pure tick-down) bumps u16[[ctx]+104].
// ----------------------------------------------------------------------------
__attribute__((naked)) void _08001B8DC(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "ldr r4, .L1b8dc_90c\n"
        "ldr r0, [r4, #0]\n"
        "adds r1, r0, #0\n"
        "adds r1, #100\n"
        "ldrh r2, [r1, #0]\n"
        "movs r3, #0\n"
        "ldrsh r0, [r1, r3]\n"
        "cmp r0, #159\n"
        "bgt .L1b8dc_8f4\n"
        "adds r0, r2, #1\n"
        "strh r0, [r1, #0]\n"
        ".L1b8dc_8f4:\n"
        "ldr r0, [r4, #0]\n"
        "adds r0, #104\n"
        "movs r1, #0\n"
        "ldrsh r0, [r0, r1]\n"
        "cmp r0, #1\n"
        "beq .L1b8dc_92c\n"
        "cmp r0, #1\n"
        "bgt .L1b8dc_910\n"
        "cmp r0, #0\n"
        "beq .L1b8dc_91a\n"
        "b .L1b8dc_9a4\n"
        "movs r0, r0\n"
        ".L1b8dc_90c: .4byte 0x03004E20\n"
        ".L1b8dc_910:\n"
        "cmp r0, #2\n"
        "beq .L1b8dc_94e\n"
        "cmp r0, #3\n"
        "beq .L1b8dc_976\n"
        "b .L1b8dc_9a4\n"
        ".L1b8dc_91a:\n"
        "movs r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp r0, #0\n"
        "beq .L1b8dc_99a\n"
        "movs r0, #0\n"
        "bl 0x08019A2C\n"
        "b .L1b8dc_99a\n"
        ".L1b8dc_92c:\n"
        "bl 0x0801B77C\n"
        "ldr r0, [r4, #0]\n"
        "adds r0, #100\n"
        "movs r2, #0\n"
        "ldrsh r0, [r0, r2]\n"
        "cmp r0, #79\n"
        "ble .L1b8dc_9a4\n"
        "movs r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp r0, #0\n"
        "bne .L1b8dc_99a\n"
        "movs r0, #0\n"
        "bl 0x08019A2C\n"
        "b .L1b8dc_99a\n"
        ".L1b8dc_94e:\n"
        "bl 0x0801B77C\n"
        "movs r0, #64\n"
        "bl 0x08018ACC\n"
        "cmp r0, #0\n"
        "bne .L1b8dc_9a4\n"
        "ldr r1, [r4, #0]\n"
        "adds r0, r1, #0\n"
        "adds r0, #100\n"
        "movs r3, #0\n"
        "ldrsh r0, [r0, r3]\n"
        "cmp r0, #159\n"
        "ble .L1b8dc_9a4\n"
        "adds r1, #68\n"
        "movs r0, #40\n"
        "strh r0, [r1, #0]\n"
        "movs r0, #128\n"
        "lsls r0, r0, #19\n"
        "b .L1b8dc_994\n"
        ".L1b8dc_976:\n"
        "bl 0x0801B77C\n"
        "ldr r0, [r4, #0]\n"
        "adds r1, r0, #0\n"
        "adds r1, #68\n"
        "ldrh r2, [r1, #0]\n"
        "movs r3, #0\n"
        "ldrsh r0, [r1, r3]\n"
        "cmp r0, #0\n"
        "ble .L1b8dc_990\n"
        "subs r0, r2, #1\n"
        "strh r0, [r1, #0]\n"
        "b .L1b8dc_9a4\n"
        ".L1b8dc_990:\n"
        "movs r0, #128\n"
        "lsls r0, r0, #12\n"
        ".L1b8dc_994:\n"
        "movs r1, #0\n"
        "bl 0x08018AA8\n"
        ".L1b8dc_99a:\n"
        "ldr r0, [r4, #0]\n"
        "adds r0, #104\n"
        "ldrh r1, [r0, #0]\n"
        "adds r1, #1\n"
        "strh r1, [r0, #0]\n"
        ".L1b8dc_9a4:\n"
        "pop {r4}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".hword 0x0000\n"
        ".syntax divided\n"
    );
}

// ----------------------------------------------------------------------------
// ---- 0x0801B9AC — 6-way scene-state dispatcher (table 0x0801B9D4) ----
// Pools: {CTX}. No incoming args; returns out (1 only when case 0 finds
// non-positive s16[[ctx]+68]). s16[[ctx]+76] picks the arm; arms 1/2/3
// share the decrement-vs-sound shape (100/60/60 via B410 for case 1),
// arm 4 falls into the arm-5 store shape on empty, arm 5 stores 40.
// The LBB02 store path also ticks u16[[ctx]+76] down by one.
// ----------------------------------------------------------------------------
__attribute__((naked)) int _08001B9AC(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, lr}\n"
        "movs	r5, #0\n"
        "ldr r1, .Lrs1b9_9cc\n"
        "ldr	r0, [r1, #0]\n"
        "adds	r0, #76\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r0, r2]\n"
        "adds	r4, r1, #0\n"
        "cmp	r0, #5\n"
        "bls .Lrs1b9_9c2\n"
        "b .Lrs1b9_b0c\n"
        ".Lrs1b9_9c2:\n"
        "lsls	r0, r0, #2\n"
        "ldr r1, .Lrs1b9_9d0\n"
        "adds	r0, r0, r1\n"
        "ldr	r0, [r0, #0]\n"
        "mov	pc, r0\n"
        ".Lrs1b9_9cc: .word 0x03004E20\n"
        ".Lrs1b9_9d0: .word 0x0801B9D4\n"
        ".Lrs1b9_9d4: .word 0x0801B9EC\n"
        ".Lrs1b9_9d8: .word 0x0801BA18\n"
        ".Lrs1b9_9dc: .word 0x0801BA58\n"
        ".Lrs1b9_9e0: .word 0x0801BA94\n"
        ".Lrs1b9_9e4: .word 0x0801BAD0\n"
        ".Lrs1b9_9e8: .word 0x0801BAFA\n"
        ".Lrs1b9_9ec:\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrs1b9_9fc\n"
        "movs	r0, #3\n"
        "bl 0x080191BC\n"
        ".Lrs1b9_9fc:\n"
        "ldr r0, .Lrs1b9_a14\n"
        "ldr	r0, [r0, #0]\n"
        "adds	r1, r0, #0\n"
        "adds	r1, #68\n"
        "ldrh	r2, [r1, #0]\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r1, r3]\n"
        "cmp	r0, #0\n"
        "bgt .Lrs1b9_ae0\n"
        "movs	r5, #1\n"
        "b .Lrs1b9_b0c\n"
        "movs	r0, r0\n"
        ".Lrs1b9_a14: .word 0x03004E20\n"
        ".Lrs1b9_a18:\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrs1b9_a28\n"
        "movs	r0, #2\n"
        "bl 0x080191BC\n"
        ".Lrs1b9_a28:\n"
        "ldr r4, .Lrs1b9_a54\n"
        "ldr	r0, [r4, #0]\n"
        "adds	r1, r0, #0\n"
        "adds	r1, #68\n"
        "ldrh	r2, [r1, #0]\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r1, r3]\n"
        "cmp	r0, #0\n"
        "bgt .Lrs1b9_ae0\n"
        "movs	r0, #8\n"
        "movs	r1, #255\n"
        "movs	r2, #0\n"
        "bl 0x0802B500\n"
        "bl sub_08001B410\n"
        "ldr	r0, [r4, #0]\n"
        "adds	r2, r0, #0\n"
        "adds	r2, #68\n"
        "movs	r1, #100\n"
        "b .Lrs1b9_b02\n"
        "movs	r0, r0\n"
        ".Lrs1b9_a54: .word 0x03004E20\n"
        ".Lrs1b9_a58:\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrs1b9_a68\n"
        "movs	r0, #1\n"
        "bl 0x080191BC\n"
        ".Lrs1b9_a68:\n"
        "ldr r4, .Lrs1b9_a90\n"
        "ldr	r0, [r4, #0]\n"
        "adds	r1, r0, #0\n"
        "adds	r1, #68\n"
        "ldrh	r2, [r1, #0]\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r1, r3]\n"
        "cmp	r0, #0\n"
        "bgt .Lrs1b9_ae0\n"
        "movs	r0, #7\n"
        "movs	r1, #255\n"
        "movs	r2, #0\n"
        "bl 0x0802B500\n"
        "ldr	r0, [r4, #0]\n"
        "adds	r2, r0, #0\n"
        "adds	r2, #68\n"
        "movs	r1, #60\n"
        "b .Lrs1b9_b02\n"
        "movs	r0, r0\n"
        ".Lrs1b9_a90: .word 0x03004E20\n"
        ".Lrs1b9_a94:\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrs1b9_aa4\n"
        "movs	r0, #0\n"
        "bl 0x080191BC\n"
        ".Lrs1b9_aa4:\n"
        "ldr r4, .Lrs1b9_acc\n"
        "ldr	r0, [r4, #0]\n"
        "adds	r1, r0, #0\n"
        "adds	r1, #68\n"
        "ldrh	r2, [r1, #0]\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r1, r3]\n"
        "cmp	r0, #0\n"
        "bgt .Lrs1b9_ae0\n"
        "movs	r0, #6\n"
        "movs	r1, #255\n"
        "movs	r2, #0\n"
        "bl 0x0802B500\n"
        "ldr	r0, [r4, #0]\n"
        "adds	r2, r0, #0\n"
        "adds	r2, #68\n"
        "movs	r1, #60\n"
        "b .Lrs1b9_b02\n"
        "movs	r0, r0\n"
        ".Lrs1b9_acc: .word 0x03004E20\n"
        ".Lrs1b9_ad0:\n"
        "ldr	r0, [r4, #0]\n"
        "adds	r1, r0, #0\n"
        "adds	r1, #68\n"
        "ldrh	r2, [r1, #0]\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r1, r3]\n"
        "cmp	r0, #0\n"
        "ble .Lrs1b9_ae6\n"
        ".Lrs1b9_ae0:\n"
        "subs	r0, r2, #1\n"
        "strh	r0, [r1, #0]\n"
        "b .Lrs1b9_b0c\n"
        ".Lrs1b9_ae6:\n"
        "movs	r0, #5\n"
        "movs	r1, #255\n"
        "movs	r2, #0\n"
        "bl 0x0802B500\n"
        "ldr	r0, [r4, #0]\n"
        "adds	r2, r0, #0\n"
        "adds	r2, #68\n"
        "movs	r1, #60\n"
        "b .Lrs1b9_b02\n"
        ".Lrs1b9_afa:\n"
        "ldr	r0, [r4, #0]\n"
        "adds	r2, r0, #0\n"
        "adds	r2, #68\n"
        "movs	r1, #40\n"
        ".Lrs1b9_b02:\n"
        "strh	r1, [r2, #0]\n"
        "adds	r0, #76\n"
        "ldrh	r1, [r0, #0]\n"
        "subs	r1, #1\n"
        "strh	r1, [r0, #0]\n"
        ".Lrs1b9_b0c:\n"
        "adds	r0, r5, #0\n"
        "pop	{r4, r5}\n"
        "pop	{r1}\n"
        "bx	r1\n"
        ".syntax divided\n"
    );
}

// ROM entry alias.
void RaceScene_B364(u16 a, u16 b) __attribute__((alias("_08001B364")));
void Sub_0801B498(int sel) __attribute__((alias("_08001B498")));
