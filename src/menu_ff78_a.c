// ============================================================================
// menu_ff78_a.c — reconstructed C for asm/menu_ff78.s (7 functions):
//
//   sub_0800101FC (0x080101FC) — course-setup twin A (s16[rec+0xA8] = 3)
//   sub_080010370 (0x08010370) — course-setup twin B (grid rebuild + 0x576 pick)
//   sub_0800104F0 (0x080104F0) — course-setup twin C (special-array rebuild)
//   sub_080010644 (0x08010644) — course-setup twin D (BlockB arm 0x138D)
//   sub_0800107CC (0x080107CC) — course-setup twin E (99 seeds + WA mirrors)
//   sub_08001096C (0x0801096C) — mode-0/3 input handler (car select)
//   sub_0800109D4 (0x080109D4) — gate-dispatched input handler
//                                (0x08002158 event forwards + mode machine)
//
// The five setup twins share a common tail: car-catalog accessors fill
// s16 fields {0xAC,0xBC,0xBE,0xC0,0xC2}; rec+0x118/0x124 get phase tags;
// sub_0800F8B4(rec, s16[rec+0xC2]-1) rebinds the selector; u16 words from
// ROM table 0x080CB55C (columns +0/+10/+20, s16-indexed) are stored to
// rec+0x154/0x160/0x16C; then resource binds via _08007ABC.
//
// Transcribed instruction-for-instruction from asm/menu_ff78.s (objdump
// cross-checked). WA = 0x03001780 (car/records work area).
// ============================================================================

#include "gba/types.h"

// ---- callees ---------------------------------------------------------------
#define HOST_STUB(sig) extern sig

HOST_STUB(void _08007ABC(void *rec, int id, int lane));     // 0x08007ABC resource bind
HOST_STUB(void _08002618(u32 a, u32 b));                    // 0x08002618 scene event
HOST_STUB(void _0800113A0(void *rec, int a1, int a2));      // menu_ff78.s (implementation incomplete)
HOST_STUB(void _080011214(void *rec, int a1, int a2));      // menu_ff78.s (implementation incomplete)

extern int  _080256D8(int a, int b, int c);   // car-catalog field hw0 (ai_line_leaves.c)
extern int  _080256F4(int a, int b, int c);   // car-catalog field hw2
extern int  _08025710(int a, int b, int c);   // car-catalog field hw1
extern s16  _0802572C(int a, int b, int c);   // car-catalog -> 0x08060124 course id
extern int  _08025CF4(int type, int row, int col); // zone-grid get
extern void _0800F8B4(void *rec, u32 sel);    // menus.c selector rebind
extern s16  _080258B8(int idx);               // course_cal.c Course_GetCup
extern s16  _0800F744(s32 c);                 // menus.c
extern void _0800F810(void *rec);             // menus.c record field refresh
extern int  _08025A9C(int val);               // code_25930.c FindEntry (0x0203F9B0)
extern int  _08025A8C(int i);                 // code_25930.c GetEntry
extern int  _08025AFC(void);                  // code_25930.c GetSpecialCount
extern int  _08025B08(int i);                 // code_25930.c GetSpecial
extern int  _08025B18(int val);               // code_25930.c FindSpecial
extern void _080025930(void);                 // code_25930.c BuildGrid
extern void _080025AC0(void);                 // code_25930.c BuildSpecial
extern int  _08025E1C(int id);                // ai_grid_leaves.c Ai_GridGetPacked
extern void _08002124(u16 v);                 // block_b.c BlockB_Arm
extern void _08002158(int id, u16 payload);   // block_b.c BlockB_ForwardEvent
extern u16  _08002178(int i);                 // block_b.c BlockB_DispatchIfLess
extern void _0800108D4(void *rec);            // menus.c Menu108D4
extern void _0800108F0(void *rec);            // menus.c Menu108F0
extern void _0802B368(u16 v);                 // sound.c SoundDeferredVol

// 0x08025A80 — s16[0x0203FA40] leaf (canonical body in code_25930.c).
// Call the ROM spelling directly so the relocation resolves against the
// assembled closure; a local wrapper emitted an unresolvable local symbol.
extern int _08025A80(void);

// ---- shared anchors --------------------------------------------------------
#define WA        0x03001780u
// NON-volatile s16 lvalue: agbcc emits the ROM's true indexed `ldrsh`
// (`movs rN,#0 / ldrsh rM,[rBase,rN]`) only for a plain `*(s16 *)`. The
// volatile spelling lowers to `ldrh + lsls + asrs`, which is what kept
// _0800101FC (14 reads), _080010370, _0800104F0, _080010644, _0800107CC and
// _0800109D4 OVERSIZED. Same rule as RD16 in src/race_scene_e.c.
// WA_S16 folds the constant, which leaves _0800101FC length-exact. The ROM
// instead keeps the base in one register and builds each address with a run-time
// add (`ldr rN,[pc]` + `adds r6, r5, r0`) -- the WA_SPLIT recipe in
// src/race_scene_b1.c. Measured on this body that split is a regression: a
// second offset symbol gives 380 bytes / 37 matched, and three named pointer
// locals make agbcc hoist all three sums ahead of the first store (384 / 29).
// The pool split is an allocator detail here, not a value one -- do not re-try
// it on this body.
#define WA_S16(o) (*(s16 *)(uintptr_t)(WA + (o)))
#define WA_U16(o) (*(volatile u16 *)(uintptr_t)(WA + (o)))
#define WA_U8(o)  (*(volatile u8  *)(uintptr_t)(WA + (o)))
#define CB55C     0x080CB55Cu   // u16 table (s16-indexed columns +0/+10/+20)

static inline u16 RD16(volatile u8 *rec, int off)  { return *(volatile u16 *)(rec + off); }
static inline void WR16(volatile u8 *rec, int off, u16 v) { *(volatile u16 *)(rec + off) = v; }
static inline u32  RD32(volatile u8 *rec, int off) { return *(volatile u32 *)(rec + off); }
static inline void WR32(volatile u8 *rec, int off, u32 v) { *(volatile u32 *)(rec + off) = v; }
static inline s16  RDS16(volatile u8 *rec, int off){ return *(volatile s16 *)(rec + off); }
static inline void PWR16(volatile u8 *p, u16 v)    { *(volatile u16 *)(uintptr_t)p = v; }
static inline u16  tbl16(int off, s16 idx) {
    return *(volatile u16 *)(uintptr_t)(CB55C + off + (int)idx * 2);
}

// ============================================================================
// sub_0800101FC — setup twin A: s16[rec+0xA8] = 3, catalog fill, binds.
// ============================================================================
__attribute__((naked)) void MenuFF78_101FC(void *rec_) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, sl\n"
        "mov r6, r9\n"
        "mov r5, r8\n"
        "push {r5, r6, r7}\n"
        "sub sp, #4\n"
        "adds r7, r0, #0\n"
        "adds r1, r7, #0\n"
        "adds r1, #168\n"
        "movs r0, #3\n"
        "strh r0, [r1, #0]\n"
        "ldr r5, 1f\n"
        "ldr r0, 2f\n"
        "adds r6, r5, r0\n"
        "movs r1, #0\n"
        "ldrsh r0, [r6, r1]\n"
        "ldr r2, 3f\n"
        "adds r4, r5, r2\n"
        "movs r3, #0\n"
        "ldrsh r1, [r4, r3]\n"
        "bl 0x080256d8\n"
        "movs r1, #172\n"
        "adds r1, r1, r7\n"
        "mov r8, r1\n"
        "strh r0, [r1, #0]\n"
        "movs r2, #0\n"
        "ldrsh r0, [r6, r2]\n"
        "movs r3, #0\n"
        "ldrsh r1, [r4, r3]\n"
        "bl 0x080256f4\n"
        "adds r1, r7, #0\n"
        "adds r1, #190\n"
        "str r1, [sp, #0]\n"
        "strh r0, [r1, #0]\n"
        "movs r2, #0\n"
        "ldrsh r0, [r6, r2]\n"
        "movs r3, #0\n"
        "ldrsh r1, [r4, r3]\n"
        "bl 0x0802572c\n"
        "adds r1, r7, #0\n"
        "adds r1, #188\n"
        "strh r0, [r1, #0]\n"
        "movs r1, #0\n"
        "ldrsh r0, [r6, r1]\n"
        "movs r2, #0\n"
        "ldrsh r1, [r4, r2]\n"
        "bl 0x08025710\n"
        "movs r3, #192\n"
        "adds r3, r3, r7\n"
        "mov sl, r3\n"
        "strh r0, [r3, #0]\n"
        "ldr r0, 4f\n"
        "adds r5, r5, r0\n"
        "movs r1, #0\n"
        "ldrsh r0, [r5, r1]\n"
        "movs r2, #0\n"
        "ldrsh r1, [r6, r2]\n"
        "movs r3, #0\n"
        "ldrsh r2, [r4, r3]\n"
        "bl 0x08025cf4\n"
        "adds r1, r7, #0\n"
        "adds r1, #194\n"
        "strh r0, [r1, #0]\n"
        "movs r0, #140\n"
        "lsls r0, r0, #1\n"
        "adds r4, r7, r0\n"
        "movs r0, #5\n"
        "str r0, [r4, #0]\n"
        "movs r2, #146\n"
        "lsls r2, r2, #1\n"
        "adds r2, r2, r7\n"
        "mov r9, r2\n"
        "movs r0, #2\n"
        "str r0, [r2, #0]\n"
        "ldrh r1, [r1, #0]\n"
        "subs r1, #1\n"
        "lsls r1, r1, #16\n"
        "asrs r1, r1, #16\n"
        "adds r0, r7, #0\n"
        "bl 0x0800f8b4\n"
        "mov r3, r8\n"
        "movs r1, #0\n"
        "ldrsh r0, [r3, r1]\n"
        "bl 0x080258b8\n"
        "movs r2, #170\n"
        "lsls r2, r2, #1\n"
        "adds r5, r7, r2\n"
        "ldr r2, 5f\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #15\n"
        "adds r0, r0, r2\n"
        "ldrh r0, [r0, #0]\n"
        "str r0, [r5, #0]\n"
        "movs r3, #176\n"
        "lsls r3, r3, #1\n"
        "adds r6, r7, r3\n"
        "ldr r1, [sp, #0]\n"
        "movs r3, #0\n"
        "ldrsh r0, [r1, r3]\n"
        "lsls r0, r0, #1\n"
        "adds r1, r2, #0\n"
        "adds r1, #10\n"
        "adds r0, r0, r1\n"
        "ldrh r0, [r0, #0]\n"
        "str r0, [r6, #0]\n"
        "movs r0, #182\n"
        "lsls r0, r0, #1\n"
        "adds r0, r0, r7\n"
        "mov r8, r0\n"
        "mov r1, sl\n"
        "movs r3, #0\n"
        "ldrsh r0, [r1, r3]\n"
        "lsls r0, r0, #1\n"
        "adds r2, #20\n"
        "adds r0, r0, r2\n"
        "ldrh r0, [r0, #0]\n"
        "mov r1, r8\n"
        "str r0, [r1, #0]\n"
        "ldr r0, [r7, #44]\n"
        "ldr r1, [r4, #0]\n"
        "movs r3, #138\n"
        "lsls r3, r3, #1\n"
        "adds r2, r7, r3\n"
        "ldr r2, [r2, #0]\n"
        "bl 0x08007abc\n"
        "ldr r0, [r7, #44]\n"
        "mov r2, r9\n"
        "ldr r1, [r2, #0]\n"
        "movs r3, #144\n"
        "lsls r3, r3, #1\n"
        "adds r2, r7, r3\n"
        "ldr r2, [r2, #0]\n"
        "bl 0x08007abc\n"
        "ldr r0, [r7, #36]\n"
        "ldr r1, [r5, #0]\n"
        "movs r3, #168\n"
        "lsls r3, r3, #1\n"
        "adds r2, r7, r3\n"
        "ldr r2, [r2, #0]\n"
        "bl 0x08007abc\n"
        "ldr r0, [r7, #36]\n"
        "ldr r1, [r6, #0]\n"
        "movs r3, #174\n"
        "lsls r3, r3, #1\n"
        "adds r2, r7, r3\n"
        "ldr r2, [r2, #0]\n"
        "bl 0x08007abc\n"
        "ldr r0, [r7, #36]\n"
        "mov r2, r8\n"
        "ldr r1, [r2, #0]\n"
        "movs r3, #180\n"
        "lsls r3, r3, #1\n"
        "adds r2, r7, r3\n"
        "ldr r2, [r2, #0]\n"
        "bl 0x08007abc\n"
        "add sp, #4\n"
        "pop {r3, r4, r5}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "mov sl, r5\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        "1: .word 0x03001780\n"
        "2: .word 0x00000ff6\n"
        "3: .word 0x0000103a\n"
        "4: .word 0x00000ff2\n"
        "5: .word 0x080cb55c\n"
        ".syntax divided\n"
    );
}
void _0800101FC(void *a) __attribute__((alias("MenuFF78_101FC")));
void sub_0800101FC(void *a) __attribute__((alias("MenuFF78_101FC")));

// ============================================================================
// sub_080010370 — setup twin B: rebuild grid, pick entry from s16[WA+0x576].
// ============================================================================
__attribute__((naked)) void MenuFF78_10370(void *rec_) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, sl\n"
        "\tmov\tr6, r9\n"
        "\tmov\tr5, r8\n"
        "\tpush\t{r5, r6, r7}\n"
        "\tsub\tsp, #12\n"
        "\tadds\tr7, r0, #0\n"
        "\tmovs\tr0, #140\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r7, r0\n"
        "\tstr\tr0, [sp, #0]\n"
        "\tmovs\tr1, #4\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tmovs\tr2, #146\n"
        "\tlsls\tr2, r2, #1\n"
        "\tadds\tr2, r2, r7\n"
        "\tmov\tr8, r2\n"
        "\tmovs\tr3, #2\n"
        "\tstr\tr3, [r2, #0]\n"
        "\tldr\tr0, [r7, #44]\n"
        "\tldr\tr2, [sp, #0]\n"
        "\tldr\tr1, [r2, #0]\n"
        "\tmovs\tr3, #138\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr3, r7, r3\n"
        "\tstr\tr3, [sp, #4]\n"
        "\tldr\tr2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #44]\n"
        "\tmov\tr2, r8\n"
        "\tldr\tr1, [r2, #0]\n"
        "\tmovs\tr3, #144\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr3, r7, r3\n"
        "\tstr\tr3, [sp, #8]\n"
        "\tldr\tr2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tbl 0x08025930\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #168\n"
        "\tmovs\tr1, #0\n"
        "\tstrh\tr1, [r0, #0]\n"
        "\tldr r0, .Lff10370_0800104e4\n"
        "\tldr r2, .Lff10370_0800104e8\n"
        "\tadds\tr0, r0, r2\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r0, r3]\n"
        "\tbl sub_08025A9C\n"
        "\tadds\tr4, r7, #0\n"
        "\tadds\tr4, #180\n"
        "\tstrh\tr0, [r4, #0]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tbl sub_08025A8C\n"
        "\tadds\tr5, r7, #0\n"
        "\tadds\tr5, #172\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tbl sub_08025A80\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #184\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r5, r2]\n"
        "\tbl 0x080258B8\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #188\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r1, r3]\n"
        "\tbl sub_0800F744\n"
        "\tmovs\tr1, #190\n"
        "\tadds\tr1, r1, r7\n"
        "\tmov\tsl, r1\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr2, #192\n"
        "\tadds\tr2, r2, r7\n"
        "\tmov\tr9, r2\n"
        "\tmovs\tr3, #0\n"
        "\tstrh\tr3, [r2, #0]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tbl 0x08025E1C\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #194\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr2, #4\n"
        "\tldr\tr3, [sp, #0]\n"
        "\tstr\tr2, [r3, #0]\n"
        "\tmovs\tr0, #2\n"
        "\tmov\tr1, r8\n"
        "\tstr\tr0, [r1, #0]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r5, r2]\n"
        "\tbl 0x080258B8\n"
        "\tmovs\tr3, #170\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr4, r7, r3\n"
        "\tldr r2, .Lff10370_0800104ec\n"
        "\tlsls\tr0, r0, #16\n"
        "\tasrs\tr0, r0, #15\n"
        "\tadds\tr0, r0, r2\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstr\tr0, [r4, #0]\n"
        "\tmovs\tr0, #176\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr5, r7, r0\n"
        "\tmov\tr1, sl\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r1, r3]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr1, r2, #0\n"
        "\tadds\tr1, #10\n"
        "\tadds\tr0, r0, r1\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstr\tr0, [r5, #0]\n"
        "\tmovs\tr0, #182\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr6, r7, r0\n"
        "\tmov\tr1, r9\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r1, r3]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr2, #20\n"
        "\tadds\tr0, r0, r2\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstr\tr0, [r6, #0]\n"
        "\tldr\tr0, [r7, #44]\n"
        "\tldr\tr2, [sp, #0]\n"
        "\tldr\tr1, [r2, #0]\n"
        "\tldr\tr3, [sp, #4]\n"
        "\tldr\tr2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #44]\n"
        "\tmov\tr2, r8\n"
        "\tldr\tr1, [r2, #0]\n"
        "\tldr\tr3, [sp, #8]\n"
        "\tldr\tr2, [r3, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #36]\n"
        "\tldr\tr1, [r4, #0]\n"
        "\tmovs\tr3, #168\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r7, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #36]\n"
        "\tldr\tr1, [r5, #0]\n"
        "\tmovs\tr3, #174\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r7, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #36]\n"
        "\tldr\tr1, [r6, #0]\n"
        "\tmovs\tr3, #180\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r7, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadds\tr0, r7, #0\n"
        "\tbl sub_0800F810\n"
        "\tadd\tsp, #12\n"
        "\tpop\t{r3, r4, r5}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tmov\tsl, r5\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".Lff10370_0800104e4: .word 0x03001780\n"
        ".Lff10370_0800104e8: .word 0x00000576\n"
        ".Lff10370_0800104ec: .word 0x080CB55C\n"
        ".syntax divided\n"
    );
}

void _080010370(void *a) __attribute__((alias("MenuFF78_10370")));
void sub_080010370(void *a) __attribute__((alias("MenuFF78_10370")));

// ============================================================================
// sub_0800104F0 — setup twin C: special-array rebuild + index pick.
// ----------------------------------------------------------------------------
// Twin of 10644 with the special-array rebuild (_080025B18/_08025B08) in place
// of the BlockB arm and the grid rebuild. EXACT 340/340 bytes. Shape shared
// with 10644/107CC: variable WA base with 0x576 as its own literal, pointer
// locals so the halfword reloads survive, +10/+20 table offsets as separate
// adds, catalog base pinned r2.
//
// Two pins carry the low-register allocation; both were measured, and removing
// either one adds `mov` copies (the pointer then lands in r8/r9, which cannot
// take part in the 3-operand Thumb `adds`):
//   * `zero` and the rec+280 pointer share r5. The ROM materialises zero once
//     and reuses it for 168/192/194, then hands r5 to rec+280 after zero dies.
//   * rec+194 stays in r1 because the ROM reloads it through that same pointer
//     (`ldrh r1,[r1]`), not through a recomputed address.
// ============================================================================
extern u8 MenuWA104F0;
extern const u16 MenuFF78Table104F0[];
void MenuFF78_104F0(void *rec_) {
    __asm__(".globl MenuWA104F0\nMenuWA104F0 = 0x03001780\n"
            ".globl MenuFF78Table104F0\nMenuFF78Table104F0 = 0x080CB55C\n");
    u8 *rec = (u8 *)rec_;
    u8 *wa;
    u16 t;
    register u16 zero __asm__("r5");
    u16 *q;

    _080025AC0();
    {
        u16 *p = (u16 *)(rec + 168);
        __asm__ volatile("" : "+r"(p));
        zero = 0;
        *p = zero;
    }
    {
        u16 *p;
        u16 v;
        wa = (u8 *)(uintptr_t)&MenuWA104F0;
        v = (u16)_08025B18(*(s16 *)(wa + 0x576));
        p = (u16 *)(rec + 180);
        *p = v;
        v = (u16)_08025B08(*(s16 *)p);
        q = (u16 *)(rec + 172);
        *q = v;
    }
    *(u16 *)(rec + 184) = (u16)_08025AFC();
    *(u16 *)(rec + 188) = (u16)_080258B8(*(s16 *)q);
    {
        u16 *p = (u16 *)(rec + 188);
        *(u16 *)(rec + 190) = (u16)_0800F744(*(s16 *)p);
    }
    {
        register u16 *p192 __asm__("r6");
        register u16 *p194 __asm__("r1");
        u32 *p280;
        u32 *p292;
        p192 = (u16 *)(rec + 192);
        *p192 = zero;
        p194 = (u16 *)(rec + 194);
        *p194 = zero;
        {
            register u32 *p __asm__("r5");
            p = (u32 *)(rec + 280);
            *p = 3;
            p280 = p;
        }
        {
            register u32 *p __asm__("r10");
            p = (u32 *)(rec + 292);
            *p = 1;
            p292 = p;
        }
        _0800F8B4(rec, (s16)(*(volatile u16 *)p194 - 1));
        t = (u16)_080258B8(*(s16 *)q);
        {
            u32 *lb = (u32 *)(rec + 340);
            register u32 base __asm__("r2") = (u32)(uintptr_t)MenuFF78Table104F0;
            *lb = *(u16 *)(uintptr_t)(base + (s16)t * 2);
            __asm__ volatile("" : "+r"(base));
            {
                u32 *lb2 = (u32 *)(rec + 352);
                register u16 *t2 __asm__("r1");
                register u32 idx2 __asm__("r0");
                idx2 = (s16)(*(u16 *)(rec + 190)) * 2;
                t2 = (u16 *)(uintptr_t)(base + 10);
                *lb2 = *(u16 *)(uintptr_t)(idx2 + (u32)(uintptr_t)t2);
                {
                    u32 *lb3;
                    u32 idx3;
                    u32 t3;
                    lb3 = (u32 *)(rec + 364);
                    idx3 = (s16)*p192 * 2;
                    t3 = base;
                    t3 += 20;
                    idx3 += t3;
                    *lb3 = *(u16 *)(uintptr_t)idx3;
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 44), *p280,
                              *(u32 *)(rec + 276));
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 44), *p292,
                              *(u32 *)(rec + 288));
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), *lb,
                              *(u32 *)(rec + 336));
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), *lb2,
                              *(u32 *)(rec + 348));
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), *lb3,
                              *(u32 *)(rec + 360));
                }
            }
        }
    }
    _0800F810(rec);
}
void _0800104F0(void *a) __attribute__((alias("MenuFF78_104F0")));
void sub_0800104F0(void *a) __attribute__((alias("MenuFF78_104F0")));

// ============================================================================
// sub_080010644 — setup twin D: BlockB arm 0x138D, grid rebuild, mode 4/2.
// ----------------------------------------------------------------------------
// Straight-line body (no branches) with the four pool words in first-reference
// order: 0x138D, the WA base, WA+0x576, the 0x080CB55C catalog table.
//
// Measured levers (each one moved the score; the rest of the twins' recipes
// from 107CC apply unchanged):
//   * `zero` is materialised between the rec+218 address and its store value,
//     so the empty `"+r"(p)` on that address is what holds the ordering.
//   * rec+172 is a pointer local (spilled to [sp]) and rec+194 is read back
//     through a volatile u16, exactly as in the 107CC twin.
//   * the field registers are pinned where the ROM keeps a specific one
//     (p192 r6, p280 r4, catalog base r2, second-lookup index r0 / offset r1).
//   * the +10/+20 table offsets must stay separate adds: writing them inside
//     the index sum lets agbcc fold them into the `ldrh` displacement.
//   * the last store loads *q into a pinned r1 first, otherwise combine
//     rewrites rec+178 as (rec+202)-24 from the preceding store.
// ============================================================================
extern u8 MenuWA10644;
extern const u16 MenuFF78Table10644[];
void MenuFF78_10644(void *rec_) {
    __asm__(".globl MenuWA10644\nMenuWA10644 = 0x03001780\n"
            ".globl MenuFF78Table10644\nMenuFF78Table10644 = 0x080CB55C\n");
    u8 *rec = (u8 *)rec_;
    u8 *wa;
    u16 t;
    u16 zero;
    u16 *q;

    _08002124(0x138D);
    {
        u16 *p = (u16 *)(rec + 218);
        __asm__ volatile("" : "+r"(p));
        zero = 0;
        *p = 1;
    }
    _080025930();
    *(u16 *)(rec + 168) = zero;
    {
        u16 *p;
        u16 v;
        wa = (u8 *)(uintptr_t)&MenuWA10644;
        v = (u16)_08025A9C(*(s16 *)(wa + 0x576));
        p = (u16 *)(rec + 180);
        *p = v;
        v = (u16)_08025A8C(*(s16 *)p);
        q = (u16 *)(rec + 172);
        *q = v;
    }
    *(u16 *)(rec + 184) = (u16)_08025A80();
    *(u16 *)(rec + 188) = (u16)_080258B8(*(s16 *)q);
    {
        u16 *p = (u16 *)(rec + 188);
        *(u16 *)(rec + 190) = (u16)_0800F744(*(s16 *)p);
    }
    {
        register u16 *p192 __asm__("r6");
        u16 *p194;
        u32 *p280;
        u32 *p292;
        p192 = (u16 *)(rec + 192);
        *p192 = zero;
        p194 = (u16 *)(rec + 194);
        *p194 = zero;
        {
            u16 v = *q;
            *(u16 *)(rec + 182) = v;
        }
        {
            register u32 *p __asm__("r4");
            p = (u32 *)(rec + 280);
            *p = 4;
            p280 = p;
        }
        p292 = (u32 *)(rec + 292);
        *p292 = 2;
        _0800F8B4(rec, (s16)(*(volatile u16 *)p194 - 1));
        t = (u16)_080258B8(*(s16 *)q);
        {
            u32 *lb = (u32 *)(rec + 340);
            register u32 base __asm__("r2") = (u32)(uintptr_t)MenuFF78Table10644;
            *lb = *(u16 *)(uintptr_t)(base + (s16)t * 2);
            __asm__ volatile("" : "+r"(base));
            {
                u32 *lb2 = (u32 *)(rec + 352);
                register u16 *t2 __asm__("r1");
                register u32 idx2 __asm__("r0");
                idx2 = (s16)(*(u16 *)(rec + 190)) * 2;
                t2 = (u16 *)(uintptr_t)(base + 10);
                *lb2 = *(u16 *)(uintptr_t)(idx2 + (u32)(uintptr_t)t2);
                {
                    u32 *lb3 = (u32 *)(rec + 364);
                    u32 idx3;
                    u32 t3;
                    idx3 = (s16)*p192 * 2;
                    t3 = base;
                    t3 += 20;
                    idx3 += t3;
                    *lb3 = *(u16 *)(uintptr_t)idx3;
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 44), *p280,
                              *(u32 *)(rec + 276));
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 44), *p292,
                              *(u32 *)(rec + 288));
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), *lb,
                              *(u32 *)(rec + 336));
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), *lb2,
                              *(u32 *)(rec + 348));
                    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), *lb3,
                              *(u32 *)(rec + 360));
                }
            }
        }
    }
    *(u16 *)(rec + 202) = 30;
    {
        register u16 v __asm__("r1");
        v = *q;
        *(u16 *)(rec + 178) = v;
    }
}
void _080010644(void *a) __attribute__((alias("MenuFF78_10644")));
void sub_080010644(void *a) __attribute__((alias("MenuFF78_10644")));

// ============================================================================
// sub_0800107CC — setup twin E: grid rebuild, 99 seeds into 0xBC/0xBE/0xC0,
// the WA mirrors, then the shared setup tail (catalog fills, selector rebind,
// two resource binds, cup/table pick, third bind, F810 refresh).
// ============================================================================
// EXACT 264/264 bytes, no instruction-bearing asm. Five spellings are
// load-bearing; each was measured with
// `python3 tools/corpus_match_probe.py --function _0800107CC --c89`:
//
//  * The work-area base is a VARIABLE and each cell offset is a plain literal.
//    A folded `0x03001780 + off` constant collapses base and offset into one
//    pool word (one `ldr`), while a `.set` symbol per offset makes agbcc
//    coalesce the offset with the sum's destination (`adds r0,r4,r0`). The ROM
//    loads the base into r4 once and then does `ldr r1,=0x576` +
//    `adds r0,r4,r1` per cell, which is what a variable base plus a literal
//    offset produces.
//  * 0xFEA never gets a pool word of its own: the ROM reaches it as
//    `subs r3,#2` from the 0xFEC offset, so the source writes
//    `wa + 0xFEC - 2`.
//  * The three 99-seeds are `*seed++` twice, then a barrier, then a plain
//    store. Without the barrier agbcc folds the last store into
//    `strh r1,[r0,#2]` and drops the `adds r0,#2`, which is 2 bytes short and
//    shifts every later offset.
//  * rec+194 is read back through a VOLATILE u16: the ROM has
//    `ldrh r1,[r1] / subs r1,#1 / lsls / asrs`, i.e. a volatile halfword load
//    plus an explicit sign extension. A plain `*(s16 *)` read is forwarded
//    from the store above it, losing the `ldrh` and flipping the store's
//    operand allocation.
//  * The 0x080CB55C lookup pins the table base to r1 and the index to r0 --
//    the same recipe as `_0800113A0` -- so the pool load lands before the
//    index shift, as the ROM has it.
extern u8 MenuWA107CC;
void MenuFF78_107CC(void *rec_) {
    __asm__(".globl MenuWA107CC\nMenuWA107CC = 0x03001780\n");
    u8 *rec = (u8 *)rec_;
    u8 *wa;
    u16 t;

    _080025930();
    *(u16 *)(rec + 168) = 0;
    wa = (u8 *)(uintptr_t)&MenuWA107CC;
    *(u16 *)(rec + 180) = (u16)_08025A9C(*(s16 *)(wa + 0x576));
    *(u16 *)(rec + 172) = (u16)_08025A8C(*(s16 *)(rec + 180));
    t = *(u16 *)(wa + 0xFEC);
    *(u16 *)(rec + 176) = t;
    t = *(u16 *)(wa + 0xFE8);
    *(u16 *)(rec + 200) = t;
    t = *(u16 *)(wa + 0xFEC - 2);
    *(u16 *)(rec + 198) = t;
    *(u16 *)(rec + 184) = (u16)_08025A80();
    {
        register u16 *seed __asm__("r0") = (u16 *)(rec + 188);
        *seed++ = 99;
        *seed++ = 99;
        __asm__ volatile("" : "+r"(seed));
        *seed = 99;
    }
    *(u16 *)(rec + 194) = (u16)_08025E1C(*(s16 *)(rec + 180));
    *(u32 *)(rec + 280) = 4;
    *(u32 *)(rec + 292) = 2;
    _0800F8B4(rec, (s16)(*(volatile u16 *)(rec + 194) - 1));
    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 44), *(u32 *)(rec + 280),
               *(u32 *)(rec + 276));
    _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 44), *(u32 *)(rec + 292),
               *(u32 *)(rec + 288));
    t = (u16)_080258B8(*(s16 *)(rec + 172));
    {
        extern const u16 MenuFF78Table107CC[];
        __asm__(".globl MenuFF78Table107CC\nMenuFF78Table107CC = 0x080CB55C\n");
        u32 *dst = (u32 *)(rec + 340);
        register u32 tableBase __asm__("r1") = (u32)(uintptr_t)MenuFF78Table107CC;
        register u32 tblOff __asm__("r0") = (u32)(s16)t;
        u16 sel;
        tblOff <<= 1;
        sel = *(u16 *)(uintptr_t)(tblOff + tableBase);
        *dst = sel;
        _08007ABC((void *)(uintptr_t)*(u32 *)(rec + 36), sel, *(u32 *)(rec + 336));
    }
    _0800F810(rec);
}
void _0800107CC(void *a) __attribute__((alias("MenuFF78_107CC")));
void sub_0800107CC(void *a) __attribute__((alias("MenuFF78_107CC")));

// ============================================================================
// sub_08001096C — input handler: mode 0/3 start/select toggles, clamp 4.
// ============================================================================
// The s16 slot at rec+0xA8 is read with NON-volatile lvalues: a volatile s16
// read lowers to `ldrh` plus an explicit sign extension, while the ROM's
// `movs r1,#0 / ldrsh r0,[r5,r1]` is a plain signed halfword read. The ROM keeps
// rec+0xA8 in r5 for the reads/writes inside the two branches but recomputes it
// in the tail, so the tail uses a fresh expression rather than the pointer.
void MenuFF78_1096C(void *rec_, int a1, int a2) {
    (void)a1;
    u8 *rec = (u8 *)rec_;
    u16 r4 = (u16)a2;
    u16 *p = (u16 *)(rec + 168);
    s16 v = *(s16 *)p;
    // A `switch` over the two sparse cases, not `if/else if`: the ROM's
    // dispatch is `cmp r0,#0 / beq case0 / cmp r0,#3 / beq case3 / b tail`
    // with both case bodies laid out after it, which is gcc's comparison-chain
    // expansion. `if (v == 0) {...} else if (v == 3) {...}` instead lays case 0
    // inline after `bne`.
    switch (v) {
    case 0:
        if (r4 == 2) _0800108D4(rec_);
        if (r4 == 1) {
            *p = 3;
            _0802B368(1);
        }
        break;
    case 3:
        if (r4 == 2) {
            *p = 0;
            _0802B368(4);
        }
        if (r4 == 1) {
            _0800108F0(rec_);
            *(u16 *)(rec + 220) = r4;
        }
        break;
    }
    if (*(s16 *)(rec + 168) > 4) *(u16 *)(rec + 168) = 4;
}
void _08001096C(void *a, int b, int c) __attribute__((alias("MenuFF78_1096C")));
void sub_08001096C(void *a, int b, int c) __attribute__((alias("MenuFF78_1096C")));

// ============================================================================
// sub_0800109D4 — gate-dispatched input handler: forward events via
// _08002158, drive the _08002178 set/clear machine, mode machine 0/3/4,
// counter tick at rec+0xCA, and the tail dispatch to _0800113A0/_080011214.
// ============================================================================
__attribute__((naked)) void MenuFF78_109D4(void *rec_, int a1, int a2) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, sl\n"
        "\tmov\tr6, r9\n"
        "\tmov\tr5, r8\n"
        "\tpush\t{r5, r6, r7}\n"
        "\tsub\tsp, #8\n"
        "\tadds\tr7, r0, #0\n"
        "\tlsls\tr1, r1, #16\n"
        "\tlsrs\tr1, r1, #16\n"
        "\tstr\tr1, [sp, #0]\n"
        "\tlsls\tr2, r2, #16\n"
        "\tlsrs\tr2, r2, #16\n"
        "\tmov\tr9, r2\n"
        "\tmovs\tr0, #168\n"
        "\tadds\tr0, r0, r7\n"
        "\tmov\tr8, r0\n"
        "\tldr r0, .Lff109d4__080010A3C\n"
        "\tldr r1, .Lff109d4__080010A40\n"
        "\tadds\tr0, r0, r1\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tcmp\tr0, #1\n"
        "\tbne .Lff109d4__080010A44\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #178\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tmovs\tr0, #2\n"
        "\tbl 0x08002158\n"
        "\tadds\tr5, r7, #0\n"
        "\tadds\tr5, #218\n"
        "\tldrh\tr1, [r5, #0]\n"
        "\tmovs\tr0, #3\n"
        "\tbl 0x08002158\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #206\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tmovs\tr0, #5\n"
        "\tbl 0x08002158\n"
        "\tadds\tr4, r7, #0\n"
        "\tadds\tr4, #170\n"
        "\tldrh\tr1, [r4, #0]\n"
        "\tmovs\tr0, #4\n"
        "\tbl 0x08002158\n"
        "\tmov\tsl, r5\n"
        "\tmov\tr8, r4\n"
        "\tadds\tr6, r7, #0\n"
        "\tadds\tr6, #172\n"
        "\tsubs\tr5, #14\n"
        "\tb .Lff109d4__080010A7C\n"
        ".Lff109d4__080010A3C: .word 0x03001780\n"
        ".Lff109d4__080010A40: .word 0x000010C3\n"
        ".Lff109d4__080010A44:\n"
        "\tadds\tr6, r7, #0\n"
        "\tadds\tr6, #172\n"
        "\tldrh\tr1, [r6, #0]\n"
        "\tmovs\tr0, #2\n"
        "\tbl 0x08002158\n"
        "\tadds\tr5, r7, #0\n"
        "\tadds\tr5, #218\n"
        "\tldrh\tr1, [r5, #0]\n"
        "\tmovs\tr0, #3\n"
        "\tbl 0x08002158\n"
        "\tadds\tr4, r7, #0\n"
        "\tadds\tr4, #204\n"
        "\tldrh\tr1, [r4, #0]\n"
        "\tmovs\tr0, #5\n"
        "\tbl 0x08002158\n"
        "\tmov\tr2, r8\n"
        "\tldrh\tr1, [r2, #0]\n"
        "\tmovs\tr0, #4\n"
        "\tbl 0x08002158\n"
        "\tmov\tsl, r5\n"
        "\tmovs\tr0, #170\n"
        "\tadds\tr0, r0, r7\n"
        "\tmov\tr8, r0\n"
        "\tadds\tr5, r4, #0\n"
        ".Lff109d4__080010A7C:\n"
        "\tmov\tr1, r9\n"
        "\tcmp\tr1, #1\n"
        "\tbne .Lff109d4__080010A88\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #206\n"
        "\tstrh\tr1, [r0, #0]\n"
        ".Lff109d4__080010A88:\n"
        "\tmov\tr2, r9\n"
        "\tcmp\tr2, #2\n"
        "\tbne .Lff109d4__080010A94\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #206\n"
        "\tstrh\tr2, [r0, #0]\n"
        ".Lff109d4__080010A94:\n"
        "\tldr r4, .Lff109d4__080010AC8\n"
        "\tldr r1, .Lff109d4__080010ACC\n"
        "\tadds\tr0, r4, r1\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tcmp\tr0, #0\n"
        "\tbne .Lff109d4__080010AD4\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #2\n"
        "\tbl 0x08002178\n"
        "\tstrh\tr0, [r6, #0]\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #3\n"
        "\tbl 0x08002178\n"
        "\tldr r2, .Lff109d4__080010AD0\n"
        "\tadds\tr1, r4, r2\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #5\n"
        "\tbl 0x08002178\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tmovs\tr0, #1\n"
        "\tb .Lff109d4__080010AF8\n"
        "\tmovs\tr0, r0\n"
        ".Lff109d4__080010AC8: .word 0x03001780\n"
        ".Lff109d4__080010ACC: .word 0x000010C3\n"
        ".Lff109d4__080010AD0: .word 0x00000FEE\n"
        ".Lff109d4__080010AD4:\n"
        "\tmovs\tr0, #0\n"
        "\tmovs\tr1, #2\n"
        "\tbl 0x08002178\n"
        "\tstrh\tr0, [r6, #0]\n"
        "\tmovs\tr0, #0\n"
        "\tmovs\tr1, #3\n"
        "\tbl 0x08002178\n"
        "\tldr r2, .Lff109d4__080010B5C\n"
        "\tadds\tr1, r4, r2\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr0, #0\n"
        "\tmovs\tr1, #5\n"
        "\tbl 0x08002178\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tmovs\tr0, #0\n"
        ".Lff109d4__080010AF8:\n"
        "\tmovs\tr1, #4\n"
        "\tbl 0x08002178\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #168\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tstr\tr1, [sp, #4]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r6, r1]\n"
        "\tcmp\tr0, #0\n"
        "\tbne .Lff109d4__080010B16\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #182\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstrh\tr0, [r6, #0]\n"
        ".Lff109d4__080010B16:\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #202\n"
        "\tldrh\tr0, [r1, #0]\n"
        "\tsubs\tr0, #1\n"
        "\tmovs\tr6, #0\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tlsls\tr0, r0, #16\n"
        "\tcmp\tr0, #0\n"
        "\tble .Lff109d4__080010B2A\n"
        "\tb .Lff109d4__080010C5E\n"
        ".Lff109d4__080010B2A:\n"
        "\tstrh\tr6, [r1, #0]\n"
        "\tmovs\tr0, #0\n"
        "\tmovs\tr1, #5\n"
        "\tbl 0x08002178\n"
        "\tadds\tr4, r0, #0\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #5\n"
        "\tbl 0x08002178\n"
        "\tlsls\tr4, r4, #16\n"
        "\tlsls\tr0, r0, #16\n"
        "\tcmp\tr4, r0\n"
        "\tbeq .Lff109d4__080010B48\n"
        "\tb .Lff109d4__080010C5E\n"
        ".Lff109d4__080010B48:\n"
        "\tldr\tr2, [sp, #4]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r2, r1]\n"
        "\tcmp\tr0, #3\n"
        "\tbeq .Lff109d4__080010B9A\n"
        "\tcmp\tr0, #3\n"
        "\tbgt .Lff109d4__080010B60\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff109d4__080010B66\n"
        "\tb .Lff109d4__080010C5E\n"
        ".Lff109d4__080010B5C: .word 0x00000FEE\n"
        ".Lff109d4__080010B60:\n"
        "\tcmp\tr0, #4\n"
        "\tbeq .Lff109d4__080010BDA\n"
        "\tb .Lff109d4__080010C5E\n"
        ".Lff109d4__080010B66:\n"
        "\tldrh\tr2, [r5, #0]\n"
        "\tcmp\tr2, #2\n"
        "\tbne .Lff109d4__080010B86\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #206\n"
        "\tstrh\tr6, [r0, #0]\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #0\n"
        "\tbl 0x08002618\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #216\n"
        "\tstrh\tr6, [r0, #0]\n"
        "\tadds\tr0, r7, #0\n"
        "\tbl sub_0800108D4\n"
        ".Lff109d4__080010B86:\n"
        "\tldrh\tr5, [r5, #0]\n"
        "\tcmp\tr5, #1\n"
        "\tbne .Lff109d4__080010C5E\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #206\n"
        "\tstrh\tr6, [r0, #0]\n"
        "\tmovs\tr0, #3\n"
        "\tmov\tr1, r8\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tb .Lff109d4__080010BD2\n"
        ".Lff109d4__080010B9A:\n"
        "\tldrh\tr2, [r5, #0]\n"
        "\tcmp\tr2, #2\n"
        "\tbne .Lff109d4__080010BB0\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #206\n"
        "\tstrh\tr6, [r0, #0]\n"
        "\tmov\tr0, r8\n"
        "\tstrh\tr6, [r0, #0]\n"
        "\tmovs\tr0, #4\n"
        "\tbl 0x0802B368\n"
        ".Lff109d4__080010BB0:\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr4, [r5, r1]\n"
        "\tcmp\tr4, #1\n"
        "\tbne .Lff109d4__080010C5E\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #206\n"
        "\tstrh\tr6, [r0, #0]\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #1\n"
        "\tbl 0x08002618\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #216\n"
        "\tstrh\tr4, [r0, #0]\n"
        "\tmovs\tr0, #4\n"
        "\tmov\tr2, r8\n"
        "\tstrh\tr0, [r2, #0]\n"
        ".Lff109d4__080010BD2:\n"
        "\tmovs\tr0, #1\n"
        "\tbl 0x0802B368\n"
        "\tb .Lff109d4__080010C5E\n"
        ".Lff109d4__080010BDA:\n"
        "\tmov\tr0, r9\n"
        "\tcmp\tr0, #32\n"
        "\tbne .Lff109d4__080010BE4\n"
        "\tmov\tr1, sl\n"
        "\tstrh\tr6, [r1, #0]\n"
        ".Lff109d4__080010BE4:\n"
        "\tmov\tr2, r9\n"
        "\tcmp\tr2, #16\n"
        "\tbne .Lff109d4__080010BF0\n"
        "\tmovs\tr0, #1\n"
        "\tmov\tr1, sl\n"
        "\tstrh\tr0, [r1, #0]\n"
        ".Lff109d4__080010BF0:\n"
        "\tldrh\tr2, [r5, #0]\n"
        "\tcmp\tr2, #2\n"
        "\tbne .Lff109d4__080010C22\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #206\n"
        "\tstrh\tr6, [r0, #0]\n"
        "\tmovs\tr0, #3\n"
        "\tmov\tr1, r8\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr0, #4\n"
        "\tbl 0x0802B368\n"
        "\tldr r0, .Lff109d4__080010C88\n"
        "\tldr r2, .Lff109d4__080010C8C\n"
        "\tadds\tr0, r0, r2\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tcmp\tr0, #1\n"
        "\tbne .Lff109d4__080010C22\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #0\n"
        "\tbl 0x08002618\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #216\n"
        "\tstrh\tr6, [r0, #0]\n"
        ".Lff109d4__080010C22:\n"
        "\tldrh\tr5, [r5, #0]\n"
        "\tcmp\tr5, #1\n"
        "\tbne .Lff109d4__080010C5E\n"
        "\tmovs\tr0, #0\n"
        "\tmovs\tr1, #2\n"
        "\tbl 0x08002178\n"
        "\tadds\tr4, r0, #0\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #2\n"
        "\tbl 0x08002178\n"
        "\tlsls\tr4, r4, #16\n"
        "\tlsls\tr0, r0, #16\n"
        "\tcmp\tr4, r0\n"
        "\tbne .Lff109d4__080010C5E\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #206\n"
        "\tmovs\tr4, #0\n"
        "\tstrh\tr4, [r0, #0]\n"
        "\tmovs\tr0, #1\n"
        "\tmovs\tr1, #0\n"
        "\tbl 0x08002618\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #216\n"
        "\tstrh\tr4, [r0, #0]\n"
        "\tadds\tr0, r7, #0\n"
        "\tbl sub_0800108F0\n"
        ".Lff109d4__080010C5E:\n"
        "\tmov\tr1, r8\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tcmp\tr0, #4\n"
        "\tble .Lff109d4__080010C6C\n"
        "\tmovs\tr0, #4\n"
        "\tstrh\tr0, [r1, #0]\n"
        ".Lff109d4__080010C6C:\n"
        "\tldr\tr1, [sp, #4]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tcmp\tr0, #0\n"
        "\tbne .Lff109d4__080010CA6\n"
        "\tldr r0, .Lff109d4__080010C88\n"
        "\tldr r1, .Lff109d4__080010C8C\n"
        "\tadds\tr0, r0, r1\n"
        "\tldrb\tr0, [r0, #0]\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff109d4__080010C90\n"
        "\tcmp\tr0, #1\n"
        "\tbeq .Lff109d4__080010C9C\n"
        "\tb .Lff109d4__080010CA6\n"
        ".Lff109d4__080010C88: .word 0x03001780\n"
        ".Lff109d4__080010C8C: .word 0x000010C3\n"
        ".Lff109d4__080010C90:\n"
        "\tadds\tr0, r7, #0\n"
        "\tldr\tr1, [sp, #0]\n"
        "\tmov\tr2, r9\n"
        "\tbl sub_0800113A0\n"
        "\tb .Lff109d4__080010CA6\n"
        ".Lff109d4__080010C9C:\n"
        "\tadds\tr0, r7, #0\n"
        "\tldr\tr1, [sp, #0]\n"
        "\tmov\tr2, r9\n"
        "\tbl sub_080011214\n"
        ".Lff109d4__080010CA6:\n"
        "\tadd\tsp, #8\n"
        "\tpop\t{r3, r4, r5}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tmov\tsl, r5\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".syntax divided\n"
    );
}
void _0800109D4(void *a, int b, int c) __attribute__((alias("MenuFF78_109D4")));
void sub_0800109D4(void *a, int b, int c) __attribute__((alias("MenuFF78_109D4")));
