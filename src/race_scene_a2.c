// ============================================================================
// race_scene_a2.c — reconstructed C for asm/race_scene.s (10 functions,
// VMA order):
//
//   0x0801AAA0 0x0801BB14 0x0801BFAC 0x0801BFBC 0x0801C0BC
//   0x0801C0CC 0x0801C0E0 0x0801C27C 0x0801C340 0x0801C3AC
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#define HOST_STUB(sig) extern sig

// External callees (strong lifts or trampolines).
HOST_STUB(void  _08002D974(const void *src, void *dst, u32 ctrl)); // 0x08002D974 CpuSet
HOST_STUB(void  _080024AC(void));                                  // 0x080024AC screen-tile init
HOST_STUB(void  _080055D8(u32 a, u16 b));                          // 0x080055D8
HOST_STUB(void  _08002B00(u32 a, u32 b));                          // 0x08002B00
HOST_STUB(void  _08002B1C(u32 a, u32 b));                          // 0x08002B1C
HOST_STUB(void  _08002B30(u32 a, u32 b));                          // 0x08002B30
HOST_STUB(void  _08001A204(void));                                 // 0x08001A204 course load
HOST_STUB(void  _080023AC(int slot, void *handler));               // 0x080023AC table install
HOST_STUB(void  _08019A2C(int v));                                 // 0x08019A2C
HOST_STUB(void  _08004324(int v));                                 // 0x08004324 camera store
HOST_STUB(void  _08025264(int v));                                 // 0x08025264 lineup builder
HOST_STUB(void *_080240D0(void));                                  // 0x080240D0 record-ptr getter
HOST_STUB(void  _08002B190(void));                                 // 0x08002B190
HOST_STUB(void  _080056B8(int v));                                 // 0x080056B8 queue gate
HOST_STUB(u32   _08018ACC(u32 mask));                              // 0x08018ACC flag test
HOST_STUB(void  _08001A768(void *a, int b, int c));                // 0x08001A768
HOST_STUB(void  _08001A294(void));                                 // 0x08001A294 race init FSM
HOST_STUB(void  _08002B1B8(void));                                 // 0x08002B1B8
HOST_STUB(void  _08002124(u16 v));                                 // 0x08002124 BlockB arm
HOST_STUB(void  _08002254(void));                                  // 0x08002254
HOST_STUB(void  _08018AA8(u32 mask, int set));                     // 0x08018AA8 flag op
HOST_STUB(void *_08004B68(void));                                  // 0x08004B68 scene ctx
HOST_STUB(void  _08004B80(void *p));                               // 0x08004B80
HOST_STUB(void  _08004B90(void *p));                               // 0x08004B90
HOST_STUB(void  _08004BB8(void *p));                               // 0x08004BB8
HOST_STUB(void  _08002B418(int a, int b));                         // 0x08002B418
HOST_STUB(void  _08002B1E4(int v));                                // 0x08002B1E4
HOST_STUB(int   _08001B9AC(void));                                 // 0x08001B9AC helper
HOST_STUB(void  _08027114(int v));                                 // 0x08027114
HOST_STUB(void  _08027210(int v));                                 // 0x08027210
HOST_STUB(void  _08002B3A4(void));                                 // 0x08002B3A4
HOST_STUB(void  _08002B44C(void));                                 // 0x08002B44C
HOST_STUB(void  _08002B30C(int v));                                // 0x08002B30C
HOST_STUB(void  _08002B234(void));                                 // 0x08002B234
HOST_STUB(void  _08024158(void));                                  // 0x08024158 ghost commit
HOST_STUB(int   _08005B5C(int v));                                 // 0x08005B5C abs
HOST_STUB(int   _08005CB4(void *v));                               // 0x08005CB4
HOST_STUB(int   _08005F44(int v));                                 // 0x08005F44 s16 table
HOST_STUB(int   _08002DE04(int n, int d));                         // 0x08002DE04 sdiv
HOST_STUB(void  _08002B500(int a, int b, int c));                  // 0x08002B500
HOST_STUB(int   _08002D9AC(u32 v));                                // 0x08002D9AC sqrt
// 0x0802D9AC (Sqrt) is spelled `sub_0802D9AC` by the promotion closure, and
// bios_wrappers.c already aliases that spelling to the real Sqrt body.
int sub_0802D9AC(u32 v);
HOST_STUB(void *_08006C10(void *a, int b));                        // 0x08006C10
HOST_STUB(void  _0800199B0(void));                                 // 0x0800199B0
HOST_STUB(void  _0800199CC(void));                                 // 0x0800199CC

// ---- shared anchors --------------------------------------------------------
#define WA     0x03001780u
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WA + (o)))
// VOLATILE on purpose: the non-volatile form of this macro was measured on the
// two bodies that read through it and is a regression in both (matched bytes
// 147 -> 134 on _08001AAA0, 90 -> 88 on _08001BB14). Unlike WA_S16 in
// src/menu_ff78_a.c, these reads are not the ROM's `movs rN,#0 / ldrsh` shape --
// do not "fix" it to match.
#define WA_S16(o) (*(volatile s16 *)(uintptr_t)(WA + (o)))
#define WA_S8(o)  (*(volatile s8  *)(uintptr_t)(WA + (o)))
#define WA_U32(o) (*(volatile u32 *)(uintptr_t)(WA + (o)))
#define RC_SLOT   0x03004E20u
#define RC()      ((volatile u8 *)(uintptr_t)(*(volatile u32 *)(uintptr_t)RC_SLOT))

// ----------------------------------------------------------------------------
// ---- 0x0801AAA0 — race init (event 1): zero racectx, VRAM fills, WA lanes,
// ---- 10-way setup dispatch on (s16)(u16[WA+0x10FC]-1) via table 0x0801AC28.
// Pools: {0x03004E20 slot, 0x03005880 racectx, CpuSet ctrls 0x0500015A /
// 0x05000200 / 0x05000400, WA+0x10BE, DISPCNT=0, _080024AC, _080055D8,
// _08002B00/1C/30, _08001A204, WA+0x1104/0x10C2, WA+0x10FC/0x10BC/0x1106 +
// handlers 0x08019E81/0x08019D31/0x08019BF9 via _080023AC(3), IE|=2,
// DISPSTAT|=16, _08019A2C(0), [0x03005760+12]=0x03004420, _08004324,
// lane offsets 0x10CA/0x10C8/0x1114/0x1DD4/0x1119/0x1DE4/0x10BD/0x1100/0x10FE/
// 0x1124/0x1DD6/0x1DE6/0x1129, IME bounce at tail}. Incoming regs dead.
// ----------------------------------------------------------------------------
__attribute__((naked)) void _08001AAA0(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, r6, r7, lr}\n"
        "sub	sp, #12\n"
        "ldr r5, .Lrsa1aa_b1c\n"
        "ldr r1, .Lrsa1aa_b20\n"
        "str	r1, [r5, #0]\n"
        "movs	r4, #0\n"
        "str	r4, [sp, #0]\n"
        "ldr r2, .Lrsa1aa_b24\n"
        "mov	r0, sp\n"
        "bl 0x0802D974\n"
        "ldr r6, .Lrsa1aa_b28\n"
        "ldr r1, .Lrsa1aa_b2c\n"
        "adds	r0, r6, r1\n"
        "strb	r4, [r0, #0]\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #19\n"
        "strh	r4, [r0, #0]\n"
        "bl 0x080024AC\n"
        "ldr	r0, [r5, #0]\n"
        "adds	r0, #172\n"
        "movs	r1, #32\n"
        "bl 0x080055D8\n"
        "str	r4, [sp, #4]\n"
        "add	r0, sp, #4\n"
        "ldr r1, .Lrsa1aa_b30\n"
        "ldr r2, .Lrsa1aa_b34\n"
        "bl 0x0802D974\n"
        "str	r4, [sp, #8]\n"
        "add	r0, sp, #8\n"
        "ldr r1, .Lrsa1aa_b38\n"
        "ldr r2, .Lrsa1aa_b3c\n"
        "bl 0x0802D974\n"
        "ldr r0, .Lrsa1aa_b40\n"
        "movs	r1, #128\n"
        "lsls	r1, r1, #2\n"
        "bl 0x08002B00\n"
        "ldr r0, .Lrsa1aa_b44\n"
        "movs	r1, #128\n"
        "bl 0x08002B1C\n"
        "ldr r0, .Lrsa1aa_b48\n"
        "movs	r1, #128\n"
        "bl 0x08002B30\n"
        "bl 0x0801A204\n"
        "ldr r2, .Lrsa1aa_b4c\n"
        "adds	r0, r6, r2\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #2\n"
        "bne .Lrsa1aa_b54\n"
        "ldr r3, .Lrsa1aa_b50\n"
        "adds	r1, r6, r3\n"
        "movs	r0, #1\n"
        "b .Lrsa1aa_b5a\n"
        "movs	r0, r0\n"
        ".Lrsa1aa_b1c: .word 0x03004E20\n"
        ".Lrsa1aa_b20: .word 0x03005880\n"
        ".Lrsa1aa_b24: .word 0x0500015A\n"
        ".Lrsa1aa_b28: .word 0x03001780\n"
        ".Lrsa1aa_b2c: .word 0x000010BE\n"
        ".Lrsa1aa_b30: .word 0x0600E000\n"
        ".Lrsa1aa_b34: .word 0x05000200\n"
        ".Lrsa1aa_b38: .word 0x0600F000\n"
        ".Lrsa1aa_b3c: .word 0x05000400\n"
        ".Lrsa1aa_b40: .word 0x030035D0\n"
        ".Lrsa1aa_b44: .word 0x03003A20\n"
        ".Lrsa1aa_b48: .word 0x03003C20\n"
        ".Lrsa1aa_b4c: .word 0x00001104\n"
        ".Lrsa1aa_b50: .word 0x000010C2\n"
        ".Lrsa1aa_b54:\n"
        "ldr r4, .Lrsa1aa_b74\n"
        "adds	r1, r6, r4\n"
        "movs	r0, #0\n"
        ".Lrsa1aa_b5a:\n"
        "strb	r0, [r1, #0]\n"
        "ldr r1, .Lrsa1aa_b78\n"
        "ldr r6, .Lrsa1aa_b7c\n"
        "adds	r0, r1, r6\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #10\n"
        "bne .Lrsa1aa_b88\n"
        "ldr r7, .Lrsa1aa_b80\n"
        "adds	r0, r1, r7\n"
        "movs	r1, #1\n"
        "strb	r1, [r0, #0]\n"
        "ldr r1, .Lrsa1aa_b84\n"
        "b .Lrsa1aa_b9c\n"
        ".Lrsa1aa_b74: .word 0x000010C2\n"
        ".Lrsa1aa_b78: .word 0x03001780\n"
        ".Lrsa1aa_b7c: .word 0x000010FC\n"
        ".Lrsa1aa_b80: .word 0x000010BC\n"
        ".Lrsa1aa_b84: .word 0x08019E81\n"
        ".Lrsa1aa_b88:\n"
        "ldr r2, .Lrsa1aa_ba4\n"
        "adds	r0, r1, r2\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #2\n"
        "bne .Lrsa1aa_bb0\n"
        "ldr r3, .Lrsa1aa_ba8\n"
        "adds	r0, r1, r3\n"
        "movs	r1, #1\n"
        "strb	r1, [r0, #0]\n"
        "ldr r1, .Lrsa1aa_bac\n"
        ".Lrsa1aa_b9c:\n"
        "movs	r0, #3\n"
        "bl 0x080023AC\n"
        "b .Lrsa1aa_bc0\n"
        ".Lrsa1aa_ba4: .word 0x00001106\n"
        ".Lrsa1aa_ba8: .word 0x000010BC\n"
        ".Lrsa1aa_bac: .word 0x08019D31\n"
        ".Lrsa1aa_bb0:\n"
        "ldr r4, .Lrsa1aa_c04\n"
        "adds	r0, r1, r4\n"
        "movs	r1, #0\n"
        "strb	r1, [r0, #0]\n"
        "ldr r1, .Lrsa1aa_c08\n"
        "movs	r0, #3\n"
        "bl 0x080023AC\n"
        ".Lrsa1aa_bc0:\n"
        "ldr r2, .Lrsa1aa_c0c\n"
        "ldrh	r0, [r2, #0]\n"
        "movs	r1, #2\n"
        "orrs	r0, r1\n"
        "strh	r0, [r2, #0]\n"
        "ldr r2, .Lrsa1aa_c10\n"
        "ldrh	r0, [r2, #0]\n"
        "movs	r1, #16\n"
        "orrs	r0, r1\n"
        "strh	r0, [r2, #0]\n"
        "movs	r0, #0\n"
        "bl 0x08019A2C\n"
        "ldr r0, .Lrsa1aa_c14\n"
        "ldr r1, .Lrsa1aa_c18\n"
        "str	r1, [r0, #12]\n"
        "bl 0x08004324\n"
        "ldr r0, .Lrsa1aa_c1c\n"
        "ldr r6, .Lrsa1aa_c20\n"
        "adds	r0, r0, r6\n"
        "ldrh	r0, [r0, #0]\n"
        "subs	r0, #1\n"
        "lsls	r0, r0, #16\n"
        "asrs	r0, r0, #16\n"
        "cmp	r0, #9\n"
        "bls .Lrsa1aa_bf8\n"
        "b .Lrsa1aa_f90\n"
        ".Lrsa1aa_bf8:\n"
        "lsls	r0, r0, #2\n"
        "ldr r1, .Lrsa1aa_c24\n"
        "adds	r0, r0, r1\n"
        "ldr	r0, [r0, #0]\n"
        "mov	pc, r0\n"
        "movs	r0, r0\n"
        ".Lrsa1aa_c04: .word 0x000010BC\n"
        ".Lrsa1aa_c08: .word 0x08019BF9\n"
        ".Lrsa1aa_c0c: .word 0x04000200\n"
        ".Lrsa1aa_c10: .word 0x04000004\n"
        ".Lrsa1aa_c14: .word 0x03005760\n"
        ".Lrsa1aa_c18: .word 0x03004420\n"
        ".Lrsa1aa_c1c: .word 0x03001780\n"
        ".Lrsa1aa_c20: .word 0x000010FC\n"
        ".Lrsa1aa_c24: .word 0x0801AC28\n"
        ".Lrsa1aa_c28: .word 0x0801AC50\n"
        ".Lrsa1aa_c2c: .word 0x0801AE84\n"
        ".Lrsa1aa_c30: .word 0x0801ACD4\n"
        ".Lrsa1aa_c34: .word 0x0801AD78\n"
        ".Lrsa1aa_c38: .word 0x0801AE48\n"
        ".Lrsa1aa_c3c: .word 0x0801AF90\n"
        ".Lrsa1aa_c40: .word 0x0801AF90\n"
        ".Lrsa1aa_c44: .word 0x0801ADD8\n"
        ".Lrsa1aa_c48: .word 0x0801AED4\n"
        ".Lrsa1aa_c4c: .word 0x0801AF30\n"
        ".Lrsa1aa_c50:\n"
        "ldr r2, .Lrsa1aa_c78\n"
        "ldr r7, .Lrsa1aa_c7c\n"
        "adds	r1, r2, r7\n"
        "movs	r0, #8\n"
        "strh	r0, [r1, #0]\n"
        "ldr r0, .Lrsa1aa_c80\n"
        "adds	r1, r2, r0\n"
        "movs	r0, #3\n"
        "strh	r0, [r1, #0]\n"
        "ldr r1, .Lrsa1aa_c84\n"
        "adds	r0, r2, r1\n"
        "ldrb	r0, [r0, #0]\n"
        "lsls	r0, r0, #24\n"
        "asrs	r0, r0, #24\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1aa_c88\n"
        "movs	r0, #3\n"
        "bl 0x08025264\n"
        "b .Lrsa1aa_c96\n"
        ".Lrsa1aa_c78: .word 0x03001780\n"
        ".Lrsa1aa_c7c: .word 0x000010CA\n"
        ".Lrsa1aa_c80: .word 0x000010C8\n"
        ".Lrsa1aa_c84: .word 0x00001112\n"
        ".Lrsa1aa_c88:\n"
        "movs	r3, #136\n"
        "lsls	r3, r3, #5\n"
        "adds	r0, r2, r3\n"
        "movs	r4, #0\n"
        "ldrsh	r0, [r0, r4]\n"
        "bl 0x08025264\n"
        ".Lrsa1aa_c96:\n"
        "ldr r0, .Lrsa1aa_cbc\n"
        "ldr r6, .Lrsa1aa_cc0\n"
        "adds	r1, r0, r6\n"
        "ldrh	r2, [r1, #0]\n"
        "ldr r7, .Lrsa1aa_cc4\n"
        "adds	r1, r0, r7\n"
        "strh	r2, [r1, #0]\n"
        "ldr r2, .Lrsa1aa_cc8\n"
        "adds	r1, r0, r2\n"
        "movs	r2, #0\n"
        "ldrsb	r2, [r1, r2]\n"
        "ldr r3, .Lrsa1aa_ccc\n"
        "adds	r1, r0, r3\n"
        "strh	r2, [r1, #0]\n"
        "ldr r4, .Lrsa1aa_cd0\n"
        "adds	r0, r0, r4\n"
        "movs	r1, #1\n"
        "strb	r1, [r0, #0]\n"
        "b .Lrsa1aa_fc4\n"
        ".Lrsa1aa_cbc: .word 0x03001780\n"
        ".Lrsa1aa_cc0: .word 0x00001114\n"
        ".Lrsa1aa_cc4: .word 0x00001DD4\n"
        ".Lrsa1aa_cc8: .word 0x00001119\n"
        ".Lrsa1aa_ccc: .word 0x00001DE4\n"
        ".Lrsa1aa_cd0: .word 0x000010BD\n"
        ".Lrsa1aa_cd4:\n"
        "ldr r4, .Lrsa1aa_d4c\n"
        "ldr r6, .Lrsa1aa_d50\n"
        "adds	r5, r4, r6\n"
        "movs	r0, #1\n"
        "strh	r0, [r5, #0]\n"
        "ldr r7, .Lrsa1aa_d54\n"
        "adds	r1, r4, r7\n"
        "movs	r0, #3\n"
        "strh	r0, [r1, #0]\n"
        "ldr r1, .Lrsa1aa_d58\n"
        "adds	r0, r4, r1\n"
        "ldrh	r1, [r0, #0]\n"
        "ldr r2, .Lrsa1aa_d5c\n"
        "adds	r0, r4, r2\n"
        "strh	r1, [r0, #0]\n"
        "ldr r3, .Lrsa1aa_d60\n"
        "adds	r0, r4, r3\n"
        "movs	r1, #0\n"
        "ldrsb	r1, [r0, r1]\n"
        "ldr r6, .Lrsa1aa_d64\n"
        "adds	r0, r4, r6\n"
        "strh	r1, [r0, #0]\n"
        "bl 0x080240D0\n"
        "adds	r2, r0, #0\n"
        "ldrh	r0, [r2, #0]\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1aa_d3e\n"
        "adds	r7, #54\n"
        "adds	r0, r4, r7\n"
        "ldrh	r1, [r2, #2]\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r1, r0\n"
        "bne .Lrsa1aa_d3e\n"
        "ldr r3, .Lrsa1aa_d68\n"
        "adds	r1, r4, r3\n"
        "adds	r0, r2, #0\n"
        "adds	r0, #16\n"
        "ldmia	r0!, {r3, r6, r7}\n"
        "stmia	r1!, {r3, r6, r7}\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [r1, #0]\n"
        "ldrh	r1, [r2, #16]\n"
        "ldr r6, .Lrsa1aa_d6c\n"
        "adds	r0, r4, r6\n"
        "strh	r1, [r0, #0]\n"
        "movs	r1, #21\n"
        "ldrsb	r1, [r2, r1]\n"
        "ldr r7, .Lrsa1aa_d70\n"
        "adds	r0, r4, r7\n"
        "strh	r1, [r0, #0]\n"
        "movs	r0, #2\n"
        "strh	r0, [r5, #0]\n"
        ".Lrsa1aa_d3e:\n"
        "ldr r0, .Lrsa1aa_d4c\n"
        "ldr r1, .Lrsa1aa_d74\n"
        "adds	r0, r0, r1\n"
        "movs	r1, #0\n"
        "strb	r1, [r0, #0]\n"
        "b .Lrsa1aa_fc4\n"
        "movs	r0, r0\n"
        ".Lrsa1aa_d4c: .word 0x03001780\n"
        ".Lrsa1aa_d50: .word 0x000010CA\n"
        ".Lrsa1aa_d54: .word 0x000010C8\n"
        ".Lrsa1aa_d58: .word 0x00001114\n"
        ".Lrsa1aa_d5c: .word 0x00001DD4\n"
        ".Lrsa1aa_d60: .word 0x00001119\n"
        ".Lrsa1aa_d64: .word 0x00001DE4\n"
        ".Lrsa1aa_d68: .word 0x00001124\n"
        ".Lrsa1aa_d6c: .word 0x00001DD6\n"
        ".Lrsa1aa_d70: .word 0x00001DE6\n"
        ".Lrsa1aa_d74: .word 0x000010BD\n"
        ".Lrsa1aa_d78:\n"
        "ldr r4, .Lrsa1aa_db8\n"
        "ldr r2, .Lrsa1aa_dbc\n"
        "adds	r1, r4, r2\n"
        "movs	r5, #0\n"
        "movs	r0, #8\n"
        "strh	r0, [r1, #0]\n"
        "ldr r3, .Lrsa1aa_dc0\n"
        "adds	r1, r4, r3\n"
        "movs	r0, #3\n"
        "strh	r0, [r1, #0]\n"
        "movs	r6, #136\n"
        "lsls	r6, r6, #5\n"
        "adds	r0, r4, r6\n"
        "movs	r7, #0\n"
        "ldrsh	r0, [r0, r7]\n"
        "bl 0x08025264\n"
        "ldr r1, .Lrsa1aa_dc4\n"
        "adds	r0, r4, r1\n"
        "ldrh	r1, [r0, #0]\n"
        "ldr r2, .Lrsa1aa_dc8\n"
        "adds	r0, r4, r2\n"
        "strh	r1, [r0, #0]\n"
        "ldr r3, .Lrsa1aa_dcc\n"
        "adds	r0, r4, r3\n"
        "movs	r1, #0\n"
        "ldrsb	r1, [r0, r1]\n"
        "ldr r6, .Lrsa1aa_dd0\n"
        "adds	r0, r4, r6\n"
        "strh	r1, [r0, #0]\n"
        "ldr r7, .Lrsa1aa_dd4\n"
        "b .Lrsa1aa_f0e\n"
        ".Lrsa1aa_db8: .word 0x03001780\n"
        ".Lrsa1aa_dbc: .word 0x000010CA\n"
        ".Lrsa1aa_dc0: .word 0x000010C8\n"
        ".Lrsa1aa_dc4: .word 0x00001114\n"
        ".Lrsa1aa_dc8: .word 0x00001DD4\n"
        ".Lrsa1aa_dcc: .word 0x00001119\n"
        ".Lrsa1aa_dd0: .word 0x00001DE4\n"
        ".Lrsa1aa_dd4: .word 0x000010BD\n"
        ".Lrsa1aa_dd8:\n"
        "ldr r1, .Lrsa1aa_e28\n"
        "ldr r0, .Lrsa1aa_e2c\n"
        "adds	r2, r1, r0\n"
        "movs	r3, #0\n"
        "movs	r0, #2\n"
        "strh	r0, [r2, #0]\n"
        "ldr r4, .Lrsa1aa_e30\n"
        "adds	r2, r1, r4\n"
        "movs	r0, #3\n"
        "strh	r0, [r2, #0]\n"
        "ldr r6, .Lrsa1aa_e34\n"
        "adds	r0, r1, r6\n"
        "ldrh	r2, [r0, #0]\n"
        "ldr r7, .Lrsa1aa_e38\n"
        "adds	r0, r1, r7\n"
        "strh	r2, [r0, #0]\n"
        "ldr r2, .Lrsa1aa_e3c\n"
        "adds	r0, r1, r2\n"
        "movs	r2, #0\n"
        "ldrsb	r2, [r0, r2]\n"
        "ldr r4, .Lrsa1aa_e40\n"
        "adds	r0, r1, r4\n"
        "strh	r2, [r0, #0]\n"
        "adds	r6, #16\n"
        "adds	r0, r1, r6\n"
        "ldrh	r2, [r0, #0]\n"
        "adds	r7, #2\n"
        "adds	r0, r1, r7\n"
        "strh	r2, [r0, #0]\n"
        "ldr r2, .Lrsa1aa_e44\n"
        "adds	r0, r1, r2\n"
        "movs	r2, #0\n"
        "ldrsb	r2, [r0, r2]\n"
        "adds	r4, #2\n"
        "adds	r0, r1, r4\n"
        "strh	r2, [r0, #0]\n"
        "subs	r6, #103\n"
        "adds	r1, r1, r6\n"
        "b .Lrsa1aa_fc2\n"
        "movs	r0, r0\n"
        ".Lrsa1aa_e28: .word 0x03001780\n"
        ".Lrsa1aa_e2c: .word 0x000010CA\n"
        ".Lrsa1aa_e30: .word 0x000010C8\n"
        ".Lrsa1aa_e34: .word 0x00001114\n"
        ".Lrsa1aa_e38: .word 0x00001DD4\n"
        ".Lrsa1aa_e3c: .word 0x00001119\n"
        ".Lrsa1aa_e40: .word 0x00001DE4\n"
        ".Lrsa1aa_e44: .word 0x00001129\n"
        ".Lrsa1aa_e48:\n"
        "ldr r1, .Lrsa1aa_e6c\n"
        "ldr r7, .Lrsa1aa_e70\n"
        "adds	r2, r1, r7\n"
        "movs	r3, #0\n"
        "movs	r0, #1\n"
        "strh	r0, [r2, #0]\n"
        "ldr r0, .Lrsa1aa_e74\n"
        "adds	r2, r1, r0\n"
        "movs	r0, #3\n"
        "strh	r0, [r2, #0]\n"
        "ldr r2, .Lrsa1aa_e78\n"
        "adds	r0, r1, r2\n"
        "ldrh	r2, [r0, #0]\n"
        "ldr r4, .Lrsa1aa_e7c\n"
        "adds	r0, r1, r4\n"
        "strh	r2, [r0, #0]\n"
        "ldr r6, .Lrsa1aa_e80\n"
        "b .Lrsa1aa_fb2\n"
        ".Lrsa1aa_e6c: .word 0x03001780\n"
        ".Lrsa1aa_e70: .word 0x000010CA\n"
        ".Lrsa1aa_e74: .word 0x000010C8\n"
        ".Lrsa1aa_e78: .word 0x00001114\n"
        ".Lrsa1aa_e7c: .word 0x00001DD4\n"
        ".Lrsa1aa_e80: .word 0x00001119\n"
        ".Lrsa1aa_e84:\n"
        "ldr r1, .Lrsa1aa_eb8\n"
        "ldr r2, .Lrsa1aa_ebc\n"
        "adds	r0, r1, r2\n"
        "movs	r3, #0\n"
        "movs	r2, #1\n"
        "strh	r2, [r0, #0]\n"
        "ldr r4, .Lrsa1aa_ec0\n"
        "adds	r0, r1, r4\n"
        "strh	r2, [r0, #0]\n"
        "ldr r6, .Lrsa1aa_ec4\n"
        "adds	r0, r1, r6\n"
        "ldrh	r2, [r0, #0]\n"
        "ldr r7, .Lrsa1aa_ec8\n"
        "adds	r0, r1, r7\n"
        "strh	r2, [r0, #0]\n"
        "ldr r2, .Lrsa1aa_ecc\n"
        "adds	r0, r1, r2\n"
        "movs	r2, #0\n"
        "ldrsb	r2, [r0, r2]\n"
        "ldr r4, .Lrsa1aa_ed0\n"
        "adds	r0, r1, r4\n"
        "strh	r2, [r0, #0]\n"
        "subs	r6, #87\n"
        "adds	r1, r1, r6\n"
        "b .Lrsa1aa_fc2\n"
        "movs	r0, r0\n"
        ".Lrsa1aa_eb8: .word 0x03001780\n"
        ".Lrsa1aa_ebc: .word 0x000010CA\n"
        ".Lrsa1aa_ec0: .word 0x000010C8\n"
        ".Lrsa1aa_ec4: .word 0x00001114\n"
        ".Lrsa1aa_ec8: .word 0x00001DD4\n"
        ".Lrsa1aa_ecc: .word 0x00001119\n"
        ".Lrsa1aa_ed0: .word 0x00001DE4\n"
        ".Lrsa1aa_ed4:\n"
        "ldr r4, .Lrsa1aa_f14\n"
        "ldr r7, .Lrsa1aa_f18\n"
        "adds	r1, r4, r7\n"
        "movs	r5, #0\n"
        "movs	r0, #1\n"
        "strh	r0, [r1, #0]\n"
        "ldr r0, .Lrsa1aa_f1c\n"
        "adds	r1, r4, r0\n"
        "movs	r0, #5\n"
        "strh	r0, [r1, #0]\n"
        "ldr r1, .Lrsa1aa_f20\n"
        "adds	r0, r4, r1\n"
        "ldrh	r1, [r0, #0]\n"
        "ldr r2, .Lrsa1aa_f24\n"
        "adds	r0, r4, r2\n"
        "strh	r1, [r0, #0]\n"
        "ldr r3, .Lrsa1aa_f28\n"
        "adds	r0, r4, r3\n"
        "movs	r1, #0\n"
        "ldrsb	r1, [r0, r1]\n"
        "ldr r6, .Lrsa1aa_f2c\n"
        "adds	r0, r4, r6\n"
        "strh	r1, [r0, #0]\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #2\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "subs	r7, #13\n"
        ".Lrsa1aa_f0e:\n"
        "adds	r4, r4, r7\n"
        "strb	r5, [r4, #0]\n"
        "b .Lrsa1aa_fc4\n"
        ".Lrsa1aa_f14: .word 0x03001780\n"
        ".Lrsa1aa_f18: .word 0x000010CA\n"
        ".Lrsa1aa_f1c: .word 0x000010C8\n"
        ".Lrsa1aa_f20: .word 0x00001114\n"
        ".Lrsa1aa_f24: .word 0x00001DD4\n"
        ".Lrsa1aa_f28: .word 0x00001119\n"
        ".Lrsa1aa_f2c: .word 0x00001DE4\n"
        ".Lrsa1aa_f30:\n"
        "ldr r4, .Lrsa1aa_f70\n"
        "ldr r0, .Lrsa1aa_f74\n"
        "adds	r1, r4, r0\n"
        "movs	r5, #0\n"
        "movs	r0, #1\n"
        "strh	r0, [r1, #0]\n"
        "ldr r2, .Lrsa1aa_f78\n"
        "adds	r1, r4, r2\n"
        "movs	r0, #10\n"
        "strh	r0, [r1, #0]\n"
        "ldr r3, .Lrsa1aa_f7c\n"
        "adds	r0, r4, r3\n"
        "ldrh	r1, [r0, #0]\n"
        "ldr r6, .Lrsa1aa_f80\n"
        "adds	r0, r4, r6\n"
        "strh	r1, [r0, #0]\n"
        "ldr r7, .Lrsa1aa_f84\n"
        "adds	r0, r4, r7\n"
        "movs	r1, #0\n"
        "ldrsb	r1, [r0, r1]\n"
        "ldr r2, .Lrsa1aa_f88\n"
        "adds	r0, r4, r2\n"
        "strh	r1, [r0, #0]\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #2\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "ldr r3, .Lrsa1aa_f8c\n"
        "adds	r4, r4, r3\n"
        "strb	r5, [r4, #0]\n"
        "b .Lrsa1aa_fc4\n"
        ".Lrsa1aa_f70: .word 0x03001780\n"
        ".Lrsa1aa_f74: .word 0x000010CA\n"
        ".Lrsa1aa_f78: .word 0x000010C8\n"
        ".Lrsa1aa_f7c: .word 0x00001114\n"
        ".Lrsa1aa_f80: .word 0x00001DD4\n"
        ".Lrsa1aa_f84: .word 0x00001119\n"
        ".Lrsa1aa_f88: .word 0x00001DE4\n"
        ".Lrsa1aa_f8c: .word 0x000010BD\n"
        ".Lrsa1aa_f90:\n"
        "ldr r1, .Lrsa1aa_ff4\n"
        "ldr r4, .Lrsa1aa_ff8\n"
        "adds	r2, r1, r4\n"
        "movs	r3, #0\n"
        "movs	r0, #1\n"
        "strh	r0, [r2, #0]\n"
        "ldr r6, .Lrsa1aa_ffc\n"
        "adds	r2, r1, r6\n"
        "movs	r0, #3\n"
        "strh	r0, [r2, #0]\n"
        "ldr r7, .Lrsa1aa_000\n"
        "adds	r0, r1, r7\n"
        "ldrh	r2, [r0, #0]\n"
        "ldr r4, .Lrsa1aa_004\n"
        "adds	r0, r1, r4\n"
        "strh	r2, [r0, #0]\n"
        "adds	r6, #81\n"
        ".Lrsa1aa_fb2:\n"
        "adds	r0, r1, r6\n"
        "movs	r2, #0\n"
        "ldrsb	r2, [r0, r2]\n"
        "ldr r7, .Lrsa1aa_008\n"
        "adds	r0, r1, r7\n"
        "strh	r2, [r0, #0]\n"
        "ldr r0, .Lrsa1aa_00c\n"
        "adds	r1, r1, r0\n"
        ".Lrsa1aa_fc2:\n"
        "strb	r3, [r1, #0]\n"
        ".Lrsa1aa_fc4:\n"
        "ldr r0, .Lrsa1aa_ff4\n"
        "ldr r1, .Lrsa1aa_010\n"
        "adds	r0, r0, r1\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #8\n"
        "beq .Lrsa1aa_fec\n"
        "ldr r3, .Lrsa1aa_014\n"
        "movs	r0, #0\n"
        "strh	r0, [r3, #0]\n"
        "ldr r1, .Lrsa1aa_018\n"
        "ldrh	r2, [r1, #0]\n"
        "ldr r0, .Lrsa1aa_01c\n"
        "ands	r0, r2\n"
        "strh	r0, [r1, #0]\n"
        "ldrh	r2, [r1, #0]\n"
        "ldr r0, .Lrsa1aa_020\n"
        "ands	r0, r2\n"
        "strh	r0, [r1, #0]\n"
        "movs	r0, #1\n"
        "strh	r0, [r3, #0]\n"
        ".Lrsa1aa_fec:\n"
        "add	sp, #12\n"
        "pop	{r4, r5, r6, r7}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        ".Lrsa1aa_ff4: .word 0x03001780\n"
        ".Lrsa1aa_ff8: .word 0x000010CA\n"
        ".Lrsa1aa_ffc: .word 0x000010C8\n"
        ".Lrsa1aa_000: .word 0x00001114\n"
        ".Lrsa1aa_004: .word 0x00001DD4\n"
        ".Lrsa1aa_008: .word 0x00001DE4\n"
        ".Lrsa1aa_00c: .word 0x000010BD\n"
        ".Lrsa1aa_010: .word 0x000010FC\n"
        ".Lrsa1aa_014: .word 0x04000208\n"
        ".Lrsa1aa_018: .word 0x04000200\n"
        ".Lrsa1aa_01c: .word 0x0000FF7F\n"
        ".Lrsa1aa_020: .word 0x0000FFBF\n"
        ".syntax divided\n"
    );
}

// ----------------------------------------------------------------------------
// ---- 0x0801BB14 — race setup dispatcher on s16[racectx+66] via table
// ---- 0x0801BB3C (10 entries, default _08001BF9C). r3 = RC slot address at
// ---- every arm (set once at head, no call precedes the switch).
// Pools: {0x03004E20, WA+0x10FC/0x10C3/0x10C8/0x1114/0x1118/0x111A/0x1110/
// 0x10BD, 0x04000050/0x04000052, 0x04000000}.
// ----------------------------------------------------------------------------
__attribute__((naked)) void _08001BB14(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, lr}\n"
        "ldr r1, .Lrsa1bb_b34\n"
        "ldr	r0, [r1, #0]\n"
        "adds	r0, #66\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r0, r2]\n"
        "adds	r3, r1, #0\n"
        "cmp	r0, #9\n"
        "bls .Lrsa1bb_b28\n"
        "b .Lrsa1bb_f9c\n"
        ".Lrsa1bb_b28:\n"
        "lsls	r0, r0, #2\n"
        "ldr r1, .Lrsa1bb_b38\n"
        "adds	r0, r0, r1\n"
        "ldr	r0, [r0, #0]\n"
        "mov	pc, r0\n"
        "movs	r0, r0\n"
        ".Lrsa1bb_b34: .word 0x03004E20\n"
        ".Lrsa1bb_b38: .word 0x0801BB3C\n"
        ".Lrsa1bb_b3c: .word 0x0801BB64\n"
        ".Lrsa1bb_b40: .word 0x0801BBCA\n"
        ".Lrsa1bb_b44: .word 0x0801BC64\n"
        ".Lrsa1bb_b48: .word 0x0801BD3C\n"
        ".Lrsa1bb_b4c: .word 0x0801BE0C\n"
        ".Lrsa1bb_b50: .word 0x0801BE86\n"
        ".Lrsa1bb_b54: .word 0x0801BEC8\n"
        ".Lrsa1bb_b58: .word 0x0801BF2C\n"
        ".Lrsa1bb_b5c: .word 0x0801BF48\n"
        ".Lrsa1bb_b60: .word 0x0801BF82\n"
        ".Lrsa1bb_b64:\n"
        "bl 0x0802B190\n"
        "movs	r0, #1\n"
        "bl 0x080056B8\n"
        "ldr r0, .Lrsa1bb_bb4\n"
        "ldr	r5, [r0, #0]\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #2\n"
        "bl 0x08018ACC\n"
        "adds	r4, r0, #0\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #8\n"
        "bl 0x08018ACC\n"
        "adds	r2, r0, #0\n"
        "adds	r0, r5, #0\n"
        "adds	r1, r4, #0\n"
        "bl sub_08001A768\n"
        "bl sub_08001A294\n"
        "bl 0x0802B1B8\n"
        "ldr r0, .Lrsa1bb_bb8\n"
        "ldr r3, .Lrsa1bb_bbc\n"
        "adds	r0, r0, r3\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #8\n"
        "bne .Lrsa1bb_bc4\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #4\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "ldr r0, .Lrsa1bb_bc0\n"
        "bl 0x08002124\n"
        "b .Lrsa1bb_f8a\n"
        ".Lrsa1bb_bb4: .word 0x03004E20\n"
        ".Lrsa1bb_bb8: .word 0x03001780\n"
        ".Lrsa1bb_bbc: .word 0x000010FC\n"
        ".Lrsa1bb_bc0: .word 0x0000138F\n"
        ".Lrsa1bb_bc4:\n"
        "bl 0x08002254\n"
        "b .Lrsa1bb_f8a\n"
        ".Lrsa1bb_bca:\n"
        "ldr r4, .Lrsa1bb_c0c\n"
        "ldr r5, .Lrsa1bb_c10\n"
        "adds	r0, r4, r5\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #8\n"
        "bne .Lrsa1bb_c40\n"
        "movs	r0, #2\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrsa1bb_c40\n"
        "ldr r1, .Lrsa1bb_c14\n"
        "adds	r0, r4, r1\n"
        "ldrb	r0, [r0, #0]\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_c1c\n"
        "ldr r2, .Lrsa1bb_c18\n"
        "ldr	r0, [r2, #0]\n"
        "adds	r0, #124\n"
        "movs	r1, #1\n"
        "strb	r1, [r0, #0]\n"
        "ldr	r1, [r2, #0]\n"
        "adds	r0, r1, #0\n"
        "adds	r0, #125\n"
        "ldrb	r0, [r0, #0]\n"
        "cmp	r0, #0\n"
        "bne .Lrsa1bb_c02\n"
        "b .Lrsa1bb_fa4\n"
        ".Lrsa1bb_c02:\n"
        "adds	r1, #68\n"
        "movs	r0, #30\n"
        "strh	r0, [r1, #0]\n"
        "b .Lrsa1bb_c4a\n"
        "movs	r0, r0\n"
        ".Lrsa1bb_c0c: .word 0x03001780\n"
        ".Lrsa1bb_c10: .word 0x000010FC\n"
        ".Lrsa1bb_c14: .word 0x000010C3\n"
        ".Lrsa1bb_c18: .word 0x03004E20\n"
        ".Lrsa1bb_c1c:\n"
        "ldr r2, .Lrsa1bb_c3c\n"
        "ldr	r1, [r2, #0]\n"
        "adds	r0, r1, #0\n"
        "adds	r0, #124\n"
        "ldrb	r0, [r0, #0]\n"
        "cmp	r0, #0\n"
        "bne .Lrsa1bb_c2c\n"
        "b .Lrsa1bb_fa4\n"
        ".Lrsa1bb_c2c:\n"
        "adds	r1, #125\n"
        "movs	r0, #1\n"
        "strb	r0, [r1, #0]\n"
        "ldr	r0, [r2, #0]\n"
        "adds	r0, #68\n"
        "movs	r1, #31\n"
        "b .Lrsa1bb_c48\n"
        "movs	r0, r0\n"
        ".Lrsa1bb_c3c: .word 0x03004E20\n"
        ".Lrsa1bb_c40:\n"
        "ldr r0, .Lrsa1bb_c60\n"
        "ldr	r0, [r0, #0]\n"
        "adds	r0, #68\n"
        "movs	r1, #30\n"
        ".Lrsa1bb_c48:\n"
        "strh	r1, [r0, #0]\n"
        ".Lrsa1bb_c4a:\n"
        "bl 0x08004B68\n"
        "bl 0x08004B80\n"
        "ldr r2, .Lrsa1bb_c60\n"
        "ldr	r0, [r2, #0]\n"
        "adds	r0, #129\n"
        "movs	r1, #1\n"
        "strb	r1, [r0, #0]\n"
        "ldr	r0, [r2, #0]\n"
        "b .Lrsa1bb_f8e\n"
        ".Lrsa1bb_c60: .word 0x03004E20\n"
        ".Lrsa1bb_c64:\n"
        "ldr	r0, [r3, #0]\n"
        "adds	r1, r0, #0\n"
        "adds	r1, #68\n"
        "ldrh	r2, [r1, #0]\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r1, r3]\n"
        "cmp	r0, #0\n"
        "ble .Lrsa1bb_c7a\n"
        "subs	r0, r2, #1\n"
        "strh	r0, [r1, #0]\n"
        "b .Lrsa1bb_fa4\n"
        ".Lrsa1bb_c7a:\n"
        "ldr r4, .Lrsa1bb_c98\n"
        "ldr r5, .Lrsa1bb_c9c\n"
        "adds	r0, r4, r5\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #10\n"
        "bne .Lrsa1bb_ca0\n"
        "movs	r0, #60\n"
        "bl 0x0802B1E4\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #9\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "b .Lrsa1bb_ce8\n"
        ".Lrsa1bb_c98: .word 0x03001780\n"
        ".Lrsa1bb_c9c: .word 0x000010FC\n"
        ".Lrsa1bb_ca0:\n"
        "movs	r1, #0\n"
        "ldr r0, .Lrsa1bb_d1c\n"
        "adds	r2, r4, r0\n"
        "movs	r0, #4\n"
        "ldrsb	r0, [r2, r0]\n"
        "cmp	r0, #0\n"
        "ble .Lrsa1bb_cb0\n"
        "movs	r1, #1\n"
        ".Lrsa1bb_cb0:\n"
        "movs	r0, #2\n"
        "ldrsb	r0, [r2, r0]\n"
        "cmp	r0, #1\n"
        "ble .Lrsa1bb_cba\n"
        "movs	r1, #2\n"
        ".Lrsa1bb_cba:\n"
        "ldr r2, .Lrsa1bb_d20\n"
        "adds	r0, r4, r2\n"
        "ldrh	r0, [r0, #0]\n"
        "subs	r0, #1\n"
        "lsls	r0, r0, #16\n"
        "asrs	r0, r0, #16\n"
        "bl 0x0802B418\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_ce8\n"
        "ldr r3, .Lrsa1bb_d24\n"
        "adds	r0, r4, r3\n"
        "ldrb	r0, [r0, #0]\n"
        "lsls	r0, r0, #24\n"
        "asrs	r0, r0, #24\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_ce8\n"
        "movs	r0, #58\n"
        "bl 0x0802B1E4\n"
        ".Lrsa1bb_ce8:\n"
        "ldr r0, .Lrsa1bb_d28\n"
        "ldr r5, .Lrsa1bb_d2c\n"
        "adds	r0, r0, r5\n"
        "ldrb	r0, [r0, #0]\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_d34\n"
        "ldr r0, .Lrsa1bb_d30\n"
        "ldr	r2, [r0, #0]\n"
        "adds	r1, r2, #0\n"
        "adds	r1, #100\n"
        "movs	r0, #0\n"
        "strh	r0, [r1, #0]\n"
        "movs	r0, #192\n"
        "lsls	r0, r0, #3\n"
        "str	r0, [r2, #20]\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #12\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #19\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "b .Lrsa1bb_f8a\n"
        ".Lrsa1bb_d1c: .word 0x00001118\n"
        ".Lrsa1bb_d20: .word 0x00001114\n"
        ".Lrsa1bb_d24: .word 0x00001110\n"
        ".Lrsa1bb_d28: .word 0x03001780\n"
        ".Lrsa1bb_d2c: .word 0x000010BD\n"
        ".Lrsa1bb_d30: .word 0x03004E20\n"
        ".Lrsa1bb_d34:\n"
        "movs	r0, #0\n"
        "bl 0x08019A2C\n"
        "b .Lrsa1bb_f8a\n"
        ".Lrsa1bb_d3c:\n"
        "movs	r0, #2\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrsa1bb_d54\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #12\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_d54\n"
        "b .Lrsa1bb_fa4\n"
        ".Lrsa1bb_d54:\n"
        "movs	r0, #64\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_d60\n"
        "b .Lrsa1bb_fa4\n"
        ".Lrsa1bb_d60:\n"
        "ldr r0, .Lrsa1bb_d84\n"
        "ldr r1, .Lrsa1bb_d88\n"
        "adds	r0, r0, r1\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #10\n"
        "bne .Lrsa1bb_d90\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #19\n"
        "ldr r2, .Lrsa1bb_d8c\n"
        "adds	r1, r2, #0\n"
        "strh	r1, [r0, #0]\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #23\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        "b .Lrsa1bb_da2\n"
        "movs	r0, r0\n"
        ".Lrsa1bb_d84: .word 0x03001780\n"
        ".Lrsa1bb_d88: .word 0x000010FC\n"
        ".Lrsa1bb_d8c: .word 0x00001741\n"
        ".Lrsa1bb_d90:\n"
        "ldr r1, .Lrsa1bb_dc4\n"
        "movs	r3, #232\n"
        "lsls	r3, r3, #3\n"
        "adds	r0, r3, #0\n"
        "strh	r0, [r1, #0]\n"
        "adds	r1, #2\n"
        "ldr r5, .Lrsa1bb_dc8\n"
        "adds	r0, r5, #0\n"
        "strh	r0, [r1, #0]\n"
        ".Lrsa1bb_da2:\n"
        "ldr r0, .Lrsa1bb_dcc\n"
        "ldr r1, .Lrsa1bb_dd0\n"
        "adds	r0, r0, r1\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r0, r2]\n"
        "cmp	r0, #10\n"
        "bne .Lrsa1bb_dd8\n"
        "ldr r3, .Lrsa1bb_dd4\n"
        "ldr	r1, [r3, #0]\n"
        "adds	r2, r1, #0\n"
        "adds	r2, #76\n"
        "movs	r0, #0\n"
        "strh	r0, [r2, #0]\n"
        "adds	r1, #68\n"
        "movs	r0, #2\n"
        "strh	r0, [r1, #0]\n"
        "b .Lrsa1bb_e04\n"
        ".Lrsa1bb_dc4: .word 0x04000050\n"
        ".Lrsa1bb_dc8: .word 0x0000040C\n"
        ".Lrsa1bb_dcc: .word 0x03001780\n"
        ".Lrsa1bb_dd0: .word 0x000010FC\n"
        ".Lrsa1bb_dd4: .word 0x03004E20\n"
        ".Lrsa1bb_dd8:\n"
        "cmp	r0, #9\n"
        "bne .Lrsa1bb_df8\n"
        "ldr r4, .Lrsa1bb_df4\n"
        "ldr	r1, [r4, #0]\n"
        "adds	r2, r1, #0\n"
        "adds	r2, #76\n"
        "movs	r3, #0\n"
        "movs	r0, #1\n"
        "strh	r0, [r2, #0]\n"
        "adds	r1, #68\n"
        "strh	r3, [r1, #0]\n"
        "adds	r3, r4, #0\n"
        "b .Lrsa1bb_e04\n"
        "movs	r0, r0\n"
        ".Lrsa1bb_df4: .word 0x03004E20\n"
        ".Lrsa1bb_df8:\n"
        "ldr r0, .Lrsa1bb_e08\n"
        "ldr	r1, [r0, #0]\n"
        "adds	r1, #76\n"
        "movs	r2, #5\n"
        "strh	r2, [r1, #0]\n"
        "adds	r3, r0, #0\n"
        ".Lrsa1bb_e04:\n"
        "ldr	r0, [r3, #0]\n"
        "b .Lrsa1bb_f8e\n"
        ".Lrsa1bb_e08: .word 0x03004E20\n"
        ".Lrsa1bb_e0c:\n"
        "ldr r0, .Lrsa1bb_e38\n"
        "ldr r5, .Lrsa1bb_e3c\n"
        "adds	r0, r0, r5\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #10\n"
        "bne .Lrsa1bb_e40\n"
        "ldr	r1, [r3, #0]\n"
        "adds	r2, r1, #0\n"
        "adds	r2, #114\n"
        "ldrh	r4, [r2, #0]\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r2, r3]\n"
        "cmp	r0, #7\n"
        "bgt .Lrsa1bb_e54\n"
        "adds	r1, #68\n"
        "ldrh	r3, [r1, #0]\n"
        "movs	r5, #0\n"
        "ldrsh	r0, [r1, r5]\n"
        "cmp	r0, #0\n"
        "bgt .Lrsa1bb_ef0\n"
        "adds	r0, r4, #1\n"
        "b .Lrsa1bb_f02\n"
        ".Lrsa1bb_e38: .word 0x03001780\n"
        ".Lrsa1bb_e3c: .word 0x000010FC\n"
        ".Lrsa1bb_e40:\n"
        "bl sub_08001B9AC\n"
        "cmp	r0, #0\n"
        "bne .Lrsa1bb_e54\n"
        "movs	r0, #2\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrsa1bb_e54\n"
        "b .Lrsa1bb_fa4\n"
        ".Lrsa1bb_e54:\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrsa1bb_e66\n"
        "movs	r0, #8\n"
        "movs	r1, #1\n"
        "bl 0x08018AA8\n"
        ".Lrsa1bb_e66:\n"
        "movs	r0, #2\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_e72\n"
        "b .Lrsa1bb_f8a\n"
        ".Lrsa1bb_e72:\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_e7e\n"
        "b .Lrsa1bb_f8a\n"
        ".Lrsa1bb_e7e:\n"
        "movs	r0, #2\n"
        "bl 0x08027114\n"
        "b .Lrsa1bb_f8a\n"
        ".Lrsa1bb_e86:\n"
        "movs	r0, #2\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrsa1bb_e92\n"
        "b .Lrsa1bb_fa4\n"
        ".Lrsa1bb_e92:\n"
        "movs	r0, #8\n"
        "movs	r1, #0\n"
        "bl 0x08018AA8\n"
        "movs	r0, #0\n"
        "bl sub_08027210\n"
        "bl 0x0802B3A4\n"
        "bl 0x0802B44C\n"
        "movs	r0, #32\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_eba\n"
        "movs	r0, #3\n"
        "bl 0x0802B30C\n"
        "b .Lrsa1bb_ebe\n"
        ".Lrsa1bb_eba:\n"
        "bl 0x0802B234\n"
        ".Lrsa1bb_ebe:\n"
        "bl 0x08004B68\n"
        "bl 0x08004B90\n"
        "b .Lrsa1bb_f8a\n"
        ".Lrsa1bb_ec8:\n"
        "ldr r0, .Lrsa1bb_ef8\n"
        "ldr r1, .Lrsa1bb_efc\n"
        "adds	r0, r0, r1\n"
        "ldrh	r0, [r0, #0]\n"
        "cmp	r0, #10\n"
        "bne .Lrsa1bb_f1e\n"
        "ldr	r1, [r3, #0]\n"
        "adds	r2, r1, #0\n"
        "adds	r2, #114\n"
        "ldrh	r4, [r2, #0]\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r2, r3]\n"
        "cmp	r0, #0\n"
        "ble .Lrsa1bb_f0a\n"
        "adds	r1, #68\n"
        "ldrh	r3, [r1, #0]\n"
        "movs	r5, #0\n"
        "ldrsh	r0, [r1, r5]\n"
        "cmp	r0, #0\n"
        "ble .Lrsa1bb_f00\n"
        ".Lrsa1bb_ef0:\n"
        "subs	r0, r3, #1\n"
        "strh	r0, [r1, #0]\n"
        "b .Lrsa1bb_fa4\n"
        "movs	r0, r0\n"
        ".Lrsa1bb_ef8: .word 0x03001780\n"
        ".Lrsa1bb_efc: .word 0x000010FC\n"
        ".Lrsa1bb_f00:\n"
        "subs	r0, r4, #1\n"
        ".Lrsa1bb_f02:\n"
        "strh	r0, [r2, #0]\n"
        "movs	r0, #2\n"
        "strh	r0, [r1, #0]\n"
        "b .Lrsa1bb_fa4\n"
        ".Lrsa1bb_f0a:\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #23\n"
        "movs	r1, #0\n"
        "bl 0x08018AA8\n"
        "movs	r1, #128\n"
        "lsls	r1, r1, #19\n"
        "ldr r2, .Lrsa1bb_f28\n"
        "adds	r0, r2, #0\n"
        "strh	r0, [r1, #0]\n"
        ".Lrsa1bb_f1e:\n"
        "movs	r0, #1\n"
        "bl 0x08019A2C\n"
        "b .Lrsa1bb_f8a\n"
        "movs	r0, r0\n"
        ".Lrsa1bb_f28: .word 0x00000141\n"
        ".Lrsa1bb_f2c:\n"
        "movs	r0, #64\n"
        "bl 0x08018ACC\n"
        "adds	r2, r0, #0\n"
        "cmp	r2, #0\n"
        "bne .Lrsa1bb_fa4\n"
        "ldr r1, .Lrsa1bb_f44\n"
        "ldr	r0, [r1, #0]\n"
        "adds	r0, #129\n"
        "strb	r2, [r0, #0]\n"
        "ldr	r0, [r1, #0]\n"
        "b .Lrsa1bb_f8e\n"
        ".Lrsa1bb_f44: .word 0x03004E20\n"
        ".Lrsa1bb_f48:\n"
        "movs	r0, #16\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "bne .Lrsa1bb_f5e\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #2\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_f70\n"
        ".Lrsa1bb_f5e:\n"
        "ldr r0, .Lrsa1bb_f6c\n"
        "ldr	r0, [r0, #0]\n"
        "adds	r0, #66\n"
        "movs	r1, #0\n"
        "strh	r1, [r0, #0]\n"
        "b .Lrsa1bb_fa4\n"
        "movs	r0, r0\n"
        ".Lrsa1bb_f6c: .word 0x03004E20\n"
        ".Lrsa1bb_f70:\n"
        "movs	r0, #128\n"
        "lsls	r0, r0, #8\n"
        "bl 0x08018ACC\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1bb_f8a\n"
        "bl 0x08024158\n"
        "b .Lrsa1bb_f8a\n"
        ".Lrsa1bb_f82:\n"
        "bl 0x08004B68\n"
        "bl 0x08004BB8\n"
        ".Lrsa1bb_f8a:\n"
        "ldr r0, .Lrsa1bb_f98\n"
        "ldr	r0, [r0, #0]\n"
        ".Lrsa1bb_f8e:\n"
        "adds	r0, #66\n"
        "ldrh	r1, [r0, #0]\n"
        "adds	r1, #1\n"
        "strh	r1, [r0, #0]\n"
        "b .Lrsa1bb_fa4\n"
        ".Lrsa1bb_f98: .word 0x03004E20\n"
        ".Lrsa1bb_f9c:\n"
        "bl 0x08004B68\n"
        "bl 0x08004BB8\n"
        ".Lrsa1bb_fa4:\n"
        "pop	{r4, r5}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        "movs	r0, r0\n"
        ".syntax divided\n"
    );
}

// ----------------------------------------------------------------------------
// ---- 0x0801BFAC — flag-lane store leaf (interior label, heavily BL-called:
// ---- 14 sites in _08001BFBC plus external callers). Pools: {0x03004E20}.
// ----------------------------------------------------------------------------
void _08001BFAC(int v) {
    *(volatile u16 *)(RC() + 120) = (u16)v;
}

// ----------------------------------------------------------------------------
// ---- 0x0801BFBC — key-bit dispatch: one _08001BFAC call per set bit, then
// ---- two direct leaves. Pools: none. Mask order follows ROM (128<<12 is
// ---- tested before 128<<11).
// ----------------------------------------------------------------------------
void _08001BFBC(int flags) {
    if (flags & 32)
        _08001BFAC(1);
    if (flags & 64)
        _08001BFAC(3);
    if (flags & 128)
        _08001BFAC(6);
    if (flags & 256) // 128<<1
        _08001BFAC(2);
    if (flags & 512) // 128<<2
        _08001BFAC(4);
    if (flags & 1024) // 128<<3
        _08001BFAC(7);
    if (flags & 2048) // 128<<4
        _08001BFAC(0);
    if (flags & 4096) // 128<<5
        _08001BFAC(9);
    if (flags & 131072) // 128<<10
        _08001BFAC(10);
    if (flags & 524288) // 128<<12
        _08001BFAC(11);
    if (flags & 262144) // 128<<11
        _08001BFAC(12);
    if (flags & 1048576) // 128<<13
        _08001BFAC(13);
    if (flags & 8388608) // 128<<16
        _08001BFAC(14);
    if (flags & 16777216) // 128<<17
        _08001BFAC(15);
    if (flags & 2097152) // 128<<14
        _0800199B0();
    if (flags & 4194304) // 128<<15
        _0800199CC();
}

// ----------------------------------------------------------------------------
// ---- 0x0801C0BC — lane store leaf (interior label). Pools: {0x03004E20}.
// ----------------------------------------------------------------------------
void _08001C0BC(void) {
    *(volatile u16 *)(RC() + 84) = 6;
}

// ----------------------------------------------------------------------------
// ---- 0x0801C0CC — scene triple store leaf (interior label, 4 BL callers).
// ---- Pools: {0x03004E20}. Third word is (s16)-extended then stored 32-bit.
// ----------------------------------------------------------------------------
void _08001C0CC(int a, int b, int c) {
    volatile u8 *ctx = RC();
    *(volatile u32 *)(ctx + 0) = (u32)a;
    *(volatile u32 *)(ctx + 4) = (u32)b;
    *(volatile u32 *)(ctx + 8) = (u32)(int)(s16)c;
}

// ----------------------------------------------------------------------------
// ---- 0x0801C0E0 — nearest-record scan + heading/scale solve (no incoming
// ---- args). Pools: {WA+0x10CA, 0x03004E80 base, stride 284 (142<<1)}.
// ---- Loop i=1..n-1 (n re-read each pass); best = nearest record with
// ---- |dx|,|dy|<=64 by (u16)sqrt; tail solves the _08002B500(21,.,.) pair.
// ----------------------------------------------------------------------------
__attribute__((naked)) void _08001C0E0(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, r6, r7, lr}\n"
        "mov	r7, sl\n"
        "mov	r6, r9\n"
        "mov	r5, r8\n"
        "push	{r5, r6, r7}\n"
        "sub	sp, #16\n"
        "movs	r0, #0\n"
        "str	r0, [sp, #8]\n"
        "movs	r1, #128\n"
        "lsls	r1, r1, #1\n"
        "str	r1, [sp, #12]\n"
        "movs	r7, #1\n"
        "ldr r0, .Lrsa1c_270\n"
        "ldr r2, .Lrsa1c_274\n"
        "adds	r1, r0, r2\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "cmp	r7, r0\n"
        "bge .Lrsa1c_16e\n"
        "mov	r9, r1\n"
        "movs	r0, #142\n"
        "lsls	r0, r0, #1\n"
        "mov	r8, r0\n"
        ".Lrsa1c_10e:\n"
        "ldr r0, .Lrsa1c_278\n"
        "mov	r1, r8\n"
        "adds	r6, r1, r0\n"
        "adds	r2, r0, #0\n"
        "ldr	r0, [r2, #0]\n"
        "ldr	r1, [r6, #0]\n"
        "subs	r0, r0, r1\n"
        "asrs	r0, r0, #8\n"
        "adds	r4, r0, #0\n"
        "bl 0x08005B5C\n"
        "cmp	r0, #64\n"
        "bgt .Lrsa1c_15c\n"
        "ldr r1, .Lrsa1c_278\n"
        "ldr	r0, [r1, #4]\n"
        "ldr	r1, [r6, #4]\n"
        "subs	r0, r0, r1\n"
        "asrs	r0, r0, #8\n"
        "adds	r5, r0, #0\n"
        "bl 0x08005B5C\n"
        "cmp	r0, #64\n"
        "bgt .Lrsa1c_15c\n"
        "adds	r0, r4, #0\n"
        "muls	r0, r4\n"
        "adds	r1, r5, #0\n"
        "muls	r1, r5\n"
        "adds	r0, r0, r1\n"
        "bl 0x0802D9AC\n"
        "lsls	r0, r0, #16\n"
        "lsrs	r0, r0, #16\n"
        "ldr	r2, [sp, #12]\n"
        "cmp	r0, r2\n"
        "bge .Lrsa1c_15c\n"
        "str	r4, [sp, #0]\n"
        "str	r5, [sp, #4]\n"
        "str	r0, [sp, #12]\n"
        "str	r6, [sp, #8]\n"
        ".Lrsa1c_15c:\n"
        "movs	r0, #142\n"
        "lsls	r0, r0, #1\n"
        "add	r8, r0\n"
        "adds	r7, #1\n"
        "mov	r1, r9\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "cmp	r7, r0\n"
        "blt .Lrsa1c_10e\n"
        ".Lrsa1c_16e:\n"
        "ldr	r0, [sp, #8]\n"
        "cmp	r0, #0\n"
        "beq .Lrsa1c_260\n"
        "mov	r0, sp\n"
        "bl 0x08005CB4\n"
        "adds	r5, r0, #0\n"
        "lsls	r5, r5, #16\n"
        "lsrs	r5, r5, #16\n"
        "movs	r1, #136\n"
        "lsls	r1, r1, #1\n"
        "mov	sl, r1\n"
        "ldr r6, .Lrsa1c_278\n"
        "add	r6, sl\n"
        "adds	r0, r6, #0\n"
        "bl 0x08005CB4\n"
        "adds	r4, r0, #0\n"
        "lsls	r4, r4, #16\n"
        "lsrs	r4, r4, #16\n"
        "ldr	r2, [sp, #8]\n"
        "add	sl, r2\n"
        "mov	r0, sl\n"
        "bl 0x08005CB4\n"
        "mov	r8, r0\n"
        "mov	r0, r8\n"
        "lsls	r0, r0, #16\n"
        "lsrs	r0, r0, #16\n"
        "mov	r8, r0\n"
        "ldr	r0, [r6, #0]\n"
        "adds	r1, r0, #0\n"
        "muls	r1, r0\n"
        "adds	r0, r1, #0\n"
        "movs	r2, #138\n"
        "lsls	r2, r2, #1\n"
        "mov	r9, r2\n"
        "ldr r1, .Lrsa1c_278\n"
        "add	r1, r9\n"
        "ldr	r1, [r1, #0]\n"
        "adds	r2, r1, #0\n"
        "muls	r2, r1\n"
        "adds	r1, r2, #0\n"
        "adds	r0, r0, r1\n"
        "bl 0x0802D9AC\n"
        "adds	r6, r0, #0\n"
        "lsls	r5, r5, #16\n"
        "asrs	r5, r5, #16\n"
        "lsls	r4, r4, #16\n"
        "asrs	r4, r4, #16\n"
        "subs	r4, r5, r4\n"
        "lsls	r4, r4, #16\n"
        "asrs	r4, r4, #16\n"
        "adds	r0, r4, #0\n"
        "bl sub_08005F44\n"
        "lsls	r6, r6, #16\n"
        "lsrs	r6, r6, #16\n"
        "adds	r4, r6, #0\n"
        "muls	r4, r0\n"
        "lsls	r4, r4, #4\n"
        "lsrs	r4, r4, #16\n"
        "mov	r1, sl\n"
        "ldr	r0, [r1, #0]\n"
        "adds	r2, r0, #0\n"
        "muls	r2, r0\n"
        "adds	r0, r2, #0\n"
        "ldr	r1, [sp, #8]\n"
        "add	r9, r1\n"
        "mov	r2, r9\n"
        "ldr	r1, [r2, #0]\n"
        "adds	r2, r1, #0\n"
        "muls	r2, r1\n"
        "adds	r1, r2, #0\n"
        "adds	r0, r0, r1\n"
        "bl 0x0802D9AC\n"
        "adds	r6, r0, #0\n"
        "mov	r0, r8\n"
        "lsls	r0, r0, #16\n"
        "asrs	r0, r0, #16\n"
        "subs	r5, r5, r0\n"
        "lsls	r5, r5, #16\n"
        "asrs	r5, r5, #16\n"
        "adds	r0, r5, #0\n"
        "bl sub_08005F44\n"
        "lsls	r6, r6, #16\n"
        "lsrs	r6, r6, #16\n"
        "muls	r0, r6\n"
        "lsls	r0, r0, #4\n"
        "asrs	r0, r0, #16\n"
        "lsls	r4, r4, #16\n"
        "asrs	r4, r4, #16\n"
        "subs	r0, r0, r4\n"
        "asrs	r0, r0, #1\n"
        "movs	r1, #250\n"
        "lsls	r1, r1, #3\n"
        "adds	r4, r0, r1\n"
        "ldr	r2, [sp, #12]\n"
        "lsls	r0, r2, #1\n"
        "adds	r0, r0, r2\n"
        "lsls	r0, r0, #6\n"
        "movs	r1, #92\n"
        "bl 0x0802DE04\n"
        "movs	r1, #192\n"
        "subs	r1, r1, r0\n"
        "lsls	r1, r1, #16\n"
        "lsrs	r0, r1, #16\n"
        "cmp	r1, #0\n"
        "bge .Lrsa1c_252\n"
        "movs	r0, #0\n"
        ".Lrsa1c_252:\n"
        "lsls	r1, r0, #16\n"
        "asrs	r1, r1, #16\n"
        "lsls	r2, r4, #16\n"
        "asrs	r2, r2, #16\n"
        "movs	r0, #21\n"
        "bl 0x0802B500\n"
        ".Lrsa1c_260:\n"
        "add	sp, #16\n"
        "pop	{r3, r4, r5}\n"
        "mov	r8, r3\n"
        "mov	r9, r4\n"
        "mov	sl, r5\n"
        "pop	{r4, r5, r6, r7}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        ".Lrsa1c_270: .word 0x03001780\n"
        ".Lrsa1c_274: .word 0x000010CA\n"
        ".Lrsa1c_278: .word 0x03004E80\n"
        ".syntax divided\n"
    );
}

// ----------------------------------------------------------------------------
// ---- 0x0801C27C — record-pair solve leaf (r0=a, r1=b, r2=u16 dst).
// ---- Pools: {0x03004E20}. Writes u16[dst] then u16[racectx+96].
// ----------------------------------------------------------------------------
void _08001C27C(void *a0, void *b0, void *c0) {
    volatile u8 *a = (volatile u8 *)a0;
    volatile u8 *b = (volatile u8 *)b0;
    // r1: the u16 destination pointer. The ROM materialises `mov r1,r9` at
    // 0x0801C30C, i.e. AFTER the `subs` that forms the delta, and keeps r1 live
    // across the negative-clamp branch (0x0801C318 reuses it for the second
    // `strh`). Left unpinned, agbcc keeps the pointer in r0 and must re-emit
    // `mov r1,r9` inside the clamp arm. The `c = c0` copy is placed after the
    // subtraction so the `mov` sinks to 0x0801C30C.
    register volatile u8 *c __asm__("r1");
    int xy[2];
    int dist;
    // r0: `t` holds the _08005F44 result, and the ROM multiplies the scaled
    // product back into that same register (`muls r0,r6` at 0x0801C300). The
    // product must therefore be written back through `t`, not into a fresh
    // `v`; a separate destination costs a `mov` and lands the product in r6.
    register int t __asm__("r0");
    // `p`/`q` are s16, not int. The ROM sign-extends BOTH operands before the
    // subtract (lsls/asrs on r4 and r5 at 0x0801C2E8-EE). With int locals
    // agbcc's range analysis knows the [0,65535] range and proves
    // (s16)((s16)p-(s16)q) == (s16)(p-q), deleting all four mask pairs; this
    // retype restores them.
    s16 p, q;
    register u32 d1 __asm__("r6");
    xy[0] = (int)(*(volatile u32 *)(b + 4) - *(volatile u32 *)(a + 0)) >> 8;
    xy[1] = (int)(*(volatile u32 *)(b + 8) - *(volatile u32 *)(a + 4)) >> 8;
    dist = (int)(u16)sub_0802D9AC((u32)xy[0] * (u32)xy[0] + (u32)xy[1] * (u32)xy[1]);
    p = (s16)(u16)_08005CB4(xy);
    q = (s16)(u16)_08005CB4((void *)(uintptr_t)(b + 12));
    // The y1 load is in a NESTED block so it stays textually after the x1
    // square: the ROM loads b+16 at 0x08001C2D8, i.e. after
    // `adds r1,r0,#0 / muls r1,r0 / adds r0,r1,#0` (0x08001C2D2-D6), while
    // declaring both loads in one block makes agbcc schedule them adjacently
    // and hoist the second one up. Same arithmetic either way — this is load
    // placement, not a semantic change.
    {
        u32 x1 = *(volatile u32 *)(b + 12);
        u32 sq = x1 * x1;
        {
            u32 y1 = *(volatile u32 *)(b + 16);
            d1 = (u32)sub_0802D9AC(sq + y1 * y1);
        }
    }
    t = _08005F44((int)(s16)(p - q));
    // Written back through `t` (pinned r0) and cast back to int so the final
    // `>> 17` is an arithmetic shift: the ROM uses `asrs r0,r0,#17` at
    // 0x0801C304, not the `lsrs` an unsigned right shift would give.
    t = (int)(((u32)(u16)d1 * (u32)t) << 4) >> 17;
    {
        // r0 again: the ROM reuses the now-dead `t` register for the delta
        // (`subs r0,r3,r0` at 0x0801C30A) and then for the s16 sign test
        // (`lsls r0,r0,#16` at 0x0801C310).
        register int dlt __asm__("r0");
        {
            // r3: the ROM writes the loaded halfword OVER the dying copy of
            // `b` (`mov r3,r8` / `ldrh r3,[r3,#38]` at 0x0801C306-0x0801C308),
            // so the address temp and the loaded value share one register.
            // Two pins to the same register work only because the ranges are
            // DISJOINT -- `bp` dies exactly where `h` is born -- and only in
            // separate declaration blocks (two pins in one block make the C89
            // transform bail with "budget exhausted"). See the worked case at
            // src/ai_line_leaves.c `Ai_LineEqual` (`register u16 ha
            // __asm__("r3")`), which closes the same coalescing residual.
            // Control: without the pins this is `ldrh r1,[r3,#38] /
            // subs r0,r1,r0`, 194/196, first difference +0x8C.
            register volatile u8 *bp __asm__("r3");
            bp = b;
            {
                register u16 h __asm__("r3");
                h = *(volatile u16 *)(bp + 38);
                dlt = (int)h - t;
            }
        }
        c = (volatile u8 *)c0;
        *(volatile u16 *)c = (u16)dlt;
        if ((s16)dlt < 0)
            *(volatile u16 *)c = 0;
    }
    if (dist > 255)
        dist = 255;
    {
        // agbcc expands a store's VALUE first and its ADDRESS second, so the
        // ROM's order (base loads, then `asrs`, then `adds r1,#96`, then
        // `strh`) needs the base materialised by its own statement and the
        // value in a named local. Inline `(RC + 96)` gave
        // `adds r1,#96` immediately after `ldr r1,[r0]` and 186/196.
        volatile u8 *rcp = RC();
        int v = 240 - (dist >> 1);
        *(volatile u16 *)(rcp + 96) = (u16)v;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x0801C340 — gate leaf over _08006C10(rec,-1) (single ptr arg, always
// ---- returns 0). Pools: {0x03001780, 0x10C8, 0x17FF}.
// ----------------------------------------------------------------------------
int _08001C340(void *p0) {
    register volatile u8 *p __asm__("r4") = (volatile u8 *)p0;
    register void *q __asm__("r5") = _08006C10((void *)(uintptr_t)p, -1);
    register int threshold __asm__("r2");
    register u32 w __asm__("r1");
    register int t __asm__("r0");
    if (_08018ACC(0x800000) != 0) // 128<<16
        return 0;
    {
        // Constant signed-byte read: Thumb `ldrsb` has no immediate form, so
        // agbcc materialises the offset and -- per the extendqisi2 constant
        // path -- reuses the destination as the index, giving the ROM's
        // `movs r0,#16 / ldrsb r0,[r4,r0]` (0x0801C35A). p (r4) stays live
        // for the later +12 word read, which keeps the base.
        register int signedByte __asm__("r0") = *(s8 *)((u8 *)p + 16);
        if (signedByte <= 0)
            return 0;
    }
    if ((*(volatile u32 *)q & 2u) == 0)
        return 0;
    {
        register u32 waBase __asm__("r0") = 0x03001780u;
        register u32 waOffset __asm__("r1") = 0x10C8u;
        __asm__ volatile("" : "+r"(waBase), "+r"(waOffset));
        waBase += waOffset;
        __asm__ volatile("" : "+r"(waBase));
        // Plain zero-index read: `movs r3,#0 / ldrsh r2,[r0,r3]` (0x0801C372).
        // The barrier on waOffset holds r1 (the just-consumed offset) live
        // through the load, so the freshly materialised zero index cannot
        // reuse r1 and lands in the ROM's r3.
        threshold = *(s16 *)(void *)waBase;
        __asm__ volatile("" : "+r"(waOffset));
    }
    w = *(volatile u32 *)(p + 12);
    __asm__ volatile("" : "+r"(w));
    t = (int)(w + 0x800u); // 128<<4
    __asm__ volatile("" : "+r"(t));
    if (t < 0)
        t = (int)(w + 0x17FFu);
    t >>= 12;
    if (threshold > t)
        return 0;
    _08018AA8(0x400000u, 1); // 128<<15
    return 0;
}

int _08001C3AC(void *a0, void *b0) {
    static const u16 kSet = 0xFFFFu;
    register u8 *b __asm__("r2") = (u8 *)b0;
    register int r __asm__("r1") = 1;
    register u16 keep __asm__("r4");
    if (a0 == 0) {
        int v;
        int nx;
        int ng;
        int o;
        u16 c;
        r = 8;
        v = (int)*(s16 *)(b + r);
        nx = ~v;
        ng = -nx;
        o = ng | nx;
        r = (int)((u32)o >> 31);
        c = kSet;
        *(u16 *)(b + 8) = c;
    } else {
        register u16 f __asm__("r3") = *(u16 *)((u8 *)a0 + 8);
        keep = *(u16 *)(b + 8);
        if (f == keep)
            r = 0;
        *(u16 *)(b + 8) = f;
    }
    return r;
}

// ROM entry alias.
void RaceScene_Init_AAA0(void) __attribute__((alias("_08001AAA0")));
// The spliced span for 0x08001BFBC starts at the asm label `sub_08001BFBC`,
// which the splice deletes, and other promoted C bodies call it under that
// spelling. agbcc only writes the `.thumb_set` for a spelling this TU
// aliases, so declare it here (rule 6).
void sub_08001BFBC(int flags) __attribute__((alias("_08001BFBC")));
