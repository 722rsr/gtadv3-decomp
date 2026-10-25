// ============================================================================
// menu_ff78_c.c — reconstructed C for asm/menu_ff78.s no-op `bx lr` stubs +
// screen-setup cursor handlers.
//
//   sub_080012574 (0x080012574) — bx lr no-op (4B)
//   sub_080012850 (0x080012850) — bx lr no-op (4B)
//   sub_080012F14 (0x080012F14) — bx lr no-op (4B)
//   sub_0800133E0 (0x0800133E0) — bx lr no-op (4B)
//   sub_080013E60 (0x080013E60) — bx lr no-op (4B)
//   sub_080014074 (0x080014074) — bx lr no-op (4B; tail of sub_080014072)
//   sub_0800141A8 (0x0800141A8) — bx lr no-op (4B)
//   sub_080014BCC (0x080014BCC) — bx lr no-op (4B)
//   sub_080014C84 (0x080014C84) — bx lr no-op (4B)
//   sub_080015CB0 (0x080015CB0) — bx lr no-op (4B; rec35_driver.s `bl 0x08015CB0`)
//
//   sub_080010F3C (0x080010F3C) — cursor-move handler w/ s16[rec+0xC2]=0
//                                  (mirrors 0x11214 but uses _08025A8C/0x258B8)
// ============================================================================

#include "gba/types.h"

#define HOST_STUB(sig) extern sig

// ---- no-op stubs ----------------------------------------------------------
void _080012574(volatile void *rec) { (void)rec; }
__asm__(".align 2, 0");
void _080012850(void) { }
__asm__(".align 2, 0");
void _080012F14(void) { }
__asm__(".align 2, 0");
void _0800133E0(void) { }
__asm__(".align 2, 0");
void _080013E60(void) { }
__asm__(".align 2, 0");
void _080014074(void) { }
__asm__(".align 2, 0");
void _0800141A8(void) { }
__asm__(".align 2, 0");
void _080014BCC(void) { }
__asm__(".align 2, 0");
void _080014C84(void) { }
__asm__(".align 2, 0");
void _080015CB0(void) { }
__asm__(".align 2, 0");

void sub_080012574(volatile void *rec) __attribute__((alias("_080012574")));
void sub_080012850(void) __attribute__((alias("_080012850")));
void sub_080012F14(void) __attribute__((alias("_080012F14")));
void sub_0800133E0(void) __attribute__((alias("_0800133E0")));
void sub_080013E60(void) __attribute__((alias("_080013E60")));
void sub_080014074(void) __attribute__((alias("_080014074")));
void sub_0800141A8(void) __attribute__((alias("_0800141A8")));
void sub_080014BCC(void) __attribute__((alias("_080014BCC")));
void sub_080014C84(void) __attribute__((alias("_080014C84")));
void sub_080015CB0(void) __attribute__((alias("_080015CB0")));
void Sub_080012574(volatile void *rec) __attribute__((alias("_080012574")));
void Sub_080012850(void) __attribute__((alias("_080012850")));
void Sub_080012F14(void) __attribute__((alias("_080012F14")));
void Sub_0800133E0(void) __attribute__((alias("_0800133E0")));
void Sub_080013E60(void) __attribute__((alias("_080013E60")));
void Sub_080014074(void) __attribute__((alias("_080014074")));
void Sub_0800141A8(void) __attribute__((alias("_0800141A8")));
void Sub_080014BCC(void) __attribute__((alias("_080014BCC")));
void Sub_080014C84(void) __attribute__((alias("_080014C84")));
void Sub_080015CB0(void) __attribute__((alias("_080015CB0")));

// ---- sub_080010F3C: cursor-move handler ----------------------------------
extern int  _08025A8C(int i);            // code_25930.c GetEntry
extern s16  _080258B8(int idx);          // course_cal.c Course_GetCup
extern s16  _0800F744(s32 c);            // menus.c
extern int  _08025E1C(int id);           // ai_grid_leaves.c Ai_GridGetPacked
extern void _0800F810(void *rec);        // menus.c record field refresh
extern s16  _0800258A8(int idx);         // numeric_leaves.c Code258A8
extern void _08007ABC(void *rec, int id, int lane); // resource bind
extern void _0802B368(u16 v);            // sound.c SoundDeferredVol
extern void _080026F50(u32 a, u32 b, u32 c, void *d); // garage.c Garage_PlaceRecord
extern s16  _0800F7EC(s32 a, s32 b);     // menus.c

#define CB55C 0x080CB55Cu

static inline u16 RD16(volatile u8 *rec, int off)  { return *(volatile u16 *)(rec + off); }
static inline void WR16(volatile u8 *rec, int off, u16 v) { *(volatile u16 *)(rec + off) = v; }
static inline u32  RD32(volatile u8 *rec, int off) { return *(volatile u32 *)(rec + off); }
static inline void WR32(volatile u8 *rec, int off, u32 v) { *(volatile u32 *)(rec + off) = v; }
static inline s16  RDS16(volatile u8 *rec, int off){ return *(volatile s16 *)(rec + off); }
static inline u16  tbl16(int off, s16 idx) {
    return *(volatile u16 *)(uintptr_t)(CB55C + off + (int)idx * 2);
}

// ============================================================================
// sub_080010F3C — cursor-move handler with s16[rec+0xC2]=0 after apply.
// r0 = rec, r1 = keymask (16/32 = up/down). Same epilogue shape as 0x11214
// but uses _08025A8C (vs 0x11214's _08025A8C, identical) and adds a
// s16[rec+0xC2]=0 write + the 0xE8/0xF4 rec placement sequence.
// ============================================================================
__attribute__((naked)) void MenuFF78_10F3C(void *rec_, int a1, int a2) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, r8\n"
        "\tpush\t{r7}\n"
        "\tadds\tr6, r0, #0\n"
        "\tlsls\tr1, r1, #16\n"
        "\tlsrs\tr1, r1, #16\n"
        "\tadds\tr3, r6, #0\n"
        "\tadds\tr3, #180\n"
        "\tldrh\tr5, [r3, #0]\n"
        "\tmovs\tr0, #32\n"
        "\tands\tr0, r1\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff10f3c_080010f6a\n"
        "\tadds\tr2, r6, #0\n"
        "\tadds\tr2, #186\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r2, r4]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff10f3c_080010f6a\n"
        "\tsubs\tr0, r5, #1\n"
        "\tstrh\tr0, [r3, #0]\n"
        "\tmovs\tr0, #10\n"
        "\tstrh\tr0, [r2, #0]\n"
        ".Lff10f3c_080010f6a:\n"
        "\tmovs\tr0, #16\n"
        "\tands\tr0, r1\n"
        "\tadds\tr7, r6, #0\n"
        "\tadds\tr7, #180\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #186\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff10f3c_080010f8c\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff10f3c_080010f8c\n"
        "\tldrh\tr0, [r7, #0]\n"
        "\tadds\tr0, #1\n"
        "\tstrh\tr0, [r7, #0]\n"
        "\tmovs\tr0, #10\n"
        "\tstrh\tr0, [r1, #0]\n"
        ".Lff10f3c_080010f8c:\n"
        "\tldrh\tr0, [r1, #0]\n"
        "\tsubs\tr0, #1\n"
        "\tmovs\tr4, #0\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tlsls\tr0, r0, #16\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff10f3c_080010f9c\n"
        "\tstrh\tr4, [r1, #0]\n"
        ".Lff10f3c_080010f9c:\n"
        "\tadds\tr2, r7, #0\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #184\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r2, r3]\n"
        "\tldrh\tr3, [r0, #0]\n"
        "\tmov\tr8, r3\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r0, r3]\n"
        "\tcmp\tr1, r0\n"
        "\tblt .Lff10f3c_080010fb6\n"
        "\tmov\tr0, r8\n"
        "\tstrh\tr0, [r2, #0]\n"
        ".Lff10f3c_080010fb6:\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r2, r1]\n"
        "\tcmp\tr0, #0\n"
        "\tbge .Lff10f3c_080010fc0\n"
        "\tstrh\tr4, [r2, #0]\n"
        ".Lff10f3c_080010fc0:\n"
        "\tlsls\tr0, r5, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r7, r2]\n"
        "\tcmp\tr0, r1\n"
        "\tbeq .Lff10f3c_080011092\n"
        "\tcmp\tr1, #99\n"
        "\tbeq .Lff10f3c_080011092\n"
        "\tmovs\tr0, #3\n"
        "\tbl 0x0802B368\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r7, r3]\n"
        "\tbl sub_08025A8C\n"
        "\tadds\tr4, r6, #0\n"
        "\tadds\tr4, #172\n"
        "\tstrh\tr0, [r4, #0]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tbl 0x080258B8\n"
        "\tadds\tr5, r6, #0\n"
        "\tadds\tr5, #188\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r5, r2]\n"
        "\tbl sub_0800F744\n"
        "\tmovs\tr3, #190\n"
        "\tadds\tr3, r3, r6\n"
        "\tmov\tr8, r3\n"
        "\tstrh\tr0, [r3, #0]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r7, r1]\n"
        "\tbl 0x08025E1C\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #194\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl sub_0800F810\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r4, r2]\n"
        "\tbl 0x080258A8\n"
        "\tadds\tr1, r0, #0\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #232\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r6, #28]\n"
        "\tadds\tr2, r6, #0\n"
        "\tadds\tr2, #228\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #244\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r4, r3]\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tsubs\tr0, #4\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tmovs\tr2, #7\n"
        "\tbl 0x08026F50\n"
        "\tmovs\tr4, #170\n"
        "\tlsls\tr4, r4, #1\n"
        "\tadds\tr2, r6, r4\n"
        "\tldr r1, .Lff10f3c_08001109c\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r5, r3]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r0, r1\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tstr\tr1, [r2, #0]\n"
        "\tldr\tr0, [r6, #36]\n"
        "\tsubs\tr4, #4\n"
        "\tadds\tr2, r6, r4\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r5, r1]\n"
        "\tmov\tr2, r8\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r2, r3]\n"
        "\tbl sub_0800F7EC\n"
        "\tadds\tr1, r0, #0\n"
        "\tadds\tr4, #16\n"
        "\tadds\tr0, r6, r4\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r6, #36]\n"
        "\tmovs\tr3, #174\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r6, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        ".Lff10f3c_080011092:\n"
        "\tpop\t{r3}\n"
        "\tmov\tr8, r3\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        ".Lff10f3c_08001109c: .word 0x080CB55C\n"
        ".syntax divided\n"
    );
}

void _080010F3C(void *a, int b, int c) __attribute__((alias("MenuFF78_10F3C")));
void sub_080010F3C(void *a, int b, int c) __attribute__((alias("MenuFF78_10F3C")));

// ============================================================================
// sub_0800110A0 — cursor-move handler w/ _0802584C (vs 0x10F3C's _08025A8C)
// and the _0800F8B4 selector rebind step. Same epilogue shape.
// r0 = rec, r1 = keymask (16/32 = up/down).
// ============================================================================
extern s16  _0802584C(int a);             // ai_line_more.c Ai_LineLookupA
extern void _0800F8B4(void *rec, u32 sel); // menus.c selector rebind
__attribute__((naked)) void MenuFF78_110A0(void *rec_, int a1, int a2) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, r9\n"
        "\tmov\tr6, r8\n"
        "\tpush\t{r6, r7}\n"
        "\tadds\tr6, r0, #0\n"
        "\tlsls\tr1, r1, #16\n"
        "\tlsrs\tr1, r1, #16\n"
        "\tadds\tr3, r6, #0\n"
        "\tadds\tr3, #180\n"
        "\tldrh\tr5, [r3, #0]\n"
        "\tmovs\tr0, #32\n"
        "\tands\tr0, r1\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff110a0_0800110d0\n"
        "\tadds\tr2, r6, #0\n"
        "\tadds\tr2, #186\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r2, r4]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff110a0_0800110d0\n"
        "\tsubs\tr0, r5, #1\n"
        "\tstrh\tr0, [r3, #0]\n"
        "\tmovs\tr0, #10\n"
        "\tstrh\tr0, [r2, #0]\n"
        ".Lff110a0_0800110d0:\n"
        "\tmovs\tr0, #16\n"
        "\tands\tr0, r1\n"
        "\tadds\tr4, r6, #0\n"
        "\tadds\tr4, #180\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #186\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff110a0_0800110f2\n"
        "\tmovs\tr7, #0\n"
        "\tldrsh\tr0, [r1, r7]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff110a0_0800110f2\n"
        "\tldrh\tr0, [r4, #0]\n"
        "\tadds\tr0, #1\n"
        "\tstrh\tr0, [r4, #0]\n"
        "\tmovs\tr0, #10\n"
        "\tstrh\tr0, [r1, #0]\n"
        ".Lff110a0_0800110f2:\n"
        "\tldrh\tr0, [r1, #0]\n"
        "\tsubs\tr0, #1\n"
        "\tmovs\tr2, #0\n"
        "\tmov\tr8, r2\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tlsls\tr0, r0, #16\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff110a0_080011106\n"
        "\tmov\tr3, r8\n"
        "\tstrh\tr3, [r1, #0]\n"
        ".Lff110a0_080011106:\n"
        "\tadds\tr2, r4, #0\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #184\n"
        "\tmovs\tr7, #0\n"
        "\tldrsh\tr1, [r2, r7]\n"
        "\tldrh\tr3, [r0, #0]\n"
        "\tmovs\tr7, #0\n"
        "\tldrsh\tr0, [r0, r7]\n"
        "\tcmp\tr1, r0\n"
        "\tble .Lff110a0_08001111c\n"
        "\tstrh\tr3, [r2, #0]\n"
        ".Lff110a0_08001111c:\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r2, r1]\n"
        "\tcmp\tr0, #0\n"
        "\tbge .Lff110a0_080011128\n"
        "\tmov\tr3, r8\n"
        "\tstrh\tr3, [r2, #0]\n"
        ".Lff110a0_080011128:\n"
        "\tlsls\tr0, r5, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tmovs\tr7, #0\n"
        "\tldrsh\tr1, [r4, r7]\n"
        "\tcmp\tr0, r1\n"
        "\tbeq .Lff110a0_080011204\n"
        "\tcmp\tr1, #99\n"
        "\tbeq .Lff110a0_080011204\n"
        "\tmovs\tr0, #3\n"
        "\tbl 0x0802B368\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tbl 0x0802584C\n"
        "\tadds\tr4, r6, #0\n"
        "\tadds\tr4, #172\n"
        "\tstrh\tr0, [r4, #0]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r4, r2]\n"
        "\tbl 0x080258B8\n"
        "\tadds\tr5, r6, #0\n"
        "\tadds\tr5, #188\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r5, r3]\n"
        "\tbl sub_0800F744\n"
        "\tmovs\tr7, #190\n"
        "\tadds\tr7, r7, r6\n"
        "\tmov\tr9, r7\n"
        "\tstrh\tr0, [r7, #0]\n"
        "\tadds\tr7, r6, #0\n"
        "\tadds\tr7, #194\n"
        "\tmov\tr0, r8\n"
        "\tstrh\tr0, [r7, #0]\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl sub_0800F810\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tbl 0x080258A8\n"
        "\tadds\tr1, r0, #0\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #232\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r6, #28]\n"
        "\tadds\tr2, r6, #0\n"
        "\tadds\tr2, #228\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldrh\tr1, [r7, #0]\n"
        "\tsubs\tr1, #1\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl sub_0800F8B4\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #244\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r4, r2]\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tsubs\tr0, #4\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tmovs\tr2, #7\n"
        "\tbl 0x08026F50\n"
        "\tmovs\tr3, #170\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr2, r6, r3\n"
        "\tldr r1, .Lff110a0_080011210\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r5, r4]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r0, r1\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tstr\tr1, [r2, #0]\n"
        "\tldr\tr0, [r6, #36]\n"
        "\tmovs\tr7, #168\n"
        "\tlsls\tr7, r7, #1\n"
        "\tadds\tr2, r6, r7\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r5, r1]\n"
        "\tmov\tr2, r9\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r2, r3]\n"
        "\tbl sub_0800F7EC\n"
        "\tadds\tr1, r0, #0\n"
        "\tmovs\tr4, #176\n"
        "\tlsls\tr4, r4, #1\n"
        "\tadds\tr0, r6, r4\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r6, #36]\n"
        "\tadds\tr7, #12\n"
        "\tadds\tr2, r6, r7\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        ".Lff110a0_080011204:\n"
        "\tpop\t{r3, r4}\n"
        "\tmov\tr8, r3\n"
        "\tmov\tr9, r4\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        ".Lff110a0_080011210: .word 0x080CB55C\n"
        ".syntax divided\n"
    );
}

void _0800110A0(void *a, int b, int c) __attribute__((alias("MenuFF78_110A0")));
void sub_0800110A0(void *a, int b, int c) __attribute__((alias("MenuFF78_110A0")));

// ============================================================================
// sub_080011484 — cursor-move handler w/ _0802581C (Ai_LineFindCourse) for
// s16[rec+0xC2]; otherwise similar epilogue (F8B4 rebind + 0xE8/0xF4 placement
// + 0x154/0x160/0x168 from 0x080CB55C, etc.).
// ============================================================================
extern int  _0802581C(int a);             // ai_line_more.c Ai_LineFindCourse
__attribute__((naked)) void MenuFF78_11484(void *rec_, int a1, int a2) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, r8\n"
        "\tpush\t{r7}\n"
        "\tadds\tr7, r0, #0\n"
        "\tlsls\tr1, r1, #16\n"
        "\tlsrs\tr1, r1, #16\n"
        "\tadds\tr3, r7, #0\n"
        "\tadds\tr3, #180\n"
        "\tldrh\tr6, [r3, #0]\n"
        "\tmovs\tr0, #32\n"
        "\tands\tr0, r1\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff11484_0800114b2\n"
        "\tadds\tr2, r7, #0\n"
        "\tadds\tr2, #186\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r2, r4]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff11484_0800114b2\n"
        "\tsubs\tr0, r6, #1\n"
        "\tstrh\tr0, [r3, #0]\n"
        "\tmovs\tr0, #10\n"
        "\tstrh\tr0, [r2, #0]\n"
        ".Lff11484_0800114b2:\n"
        "\tmovs\tr0, #16\n"
        "\tands\tr0, r1\n"
        "\tadds\tr5, r7, #0\n"
        "\tadds\tr5, #180\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #186\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lff11484_0800114d4\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r1, r2]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff11484_0800114d4\n"
        "\tldrh\tr0, [r5, #0]\n"
        "\tadds\tr0, #1\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tmovs\tr0, #10\n"
        "\tstrh\tr0, [r1, #0]\n"
        ".Lff11484_0800114d4:\n"
        "\tldrh\tr0, [r1, #0]\n"
        "\tsubs\tr0, #1\n"
        "\tmovs\tr4, #0\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tlsls\tr0, r0, #16\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lff11484_0800114e4\n"
        "\tstrh\tr4, [r1, #0]\n"
        ".Lff11484_0800114e4:\n"
        "\tadds\tr2, r5, #0\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #184\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r2, r3]\n"
        "\tldrh\tr3, [r0, #0]\n"
        "\tmov\tr8, r3\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r0, r3]\n"
        "\tcmp\tr1, r0\n"
        "\tblt .Lff11484_0800114fe\n"
        "\tmov\tr0, r8\n"
        "\tstrh\tr0, [r2, #0]\n"
        ".Lff11484_0800114fe:\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r2, r1]\n"
        "\tcmp\tr0, #0\n"
        "\tbge .Lff11484_080011508\n"
        "\tstrh\tr4, [r2, #0]\n"
        ".Lff11484_080011508:\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r5, r2]\n"
        "\tbl sub_08025A8C\n"
        "\tadds\tr4, r7, #0\n"
        "\tadds\tr4, #172\n"
        "\tstrh\tr0, [r4, #0]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r4, r3]\n"
        "\tbl 0x080258B8\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #188\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tbl 0x0802581C\n"
        "\tlsls\tr0, r0, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tbl 0x08025E1C\n"
        "\tadds\tr1, r7, #0\n"
        "\tadds\tr1, #194\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tlsls\tr0, r6, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr2, [r5, r3]\n"
        "\tmov\tr8, r4\n"
        "\tadds\tr4, r1, #0\n"
        "\tcmp\tr0, r2\n"
        "\tbeq .Lff11484_08001155a\n"
        "\tcmp\tr2, #99\n"
        "\tbeq .Lff11484_08001155a\n"
        "\tmovs\tr0, #3\n"
        "\tbl 0x0802B368\n"
        "\tadds\tr0, r7, #0\n"
        "\tbl sub_0800F810\n"
        ".Lff11484_08001155a:\n"
        "\tldrh\tr1, [r4, #0]\n"
        "\tsubs\tr1, #1\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tadds\tr0, r7, #0\n"
        "\tbl sub_0800F8B4\n"
        "\tmov\tr4, r8\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r4, r1]\n"
        "\tbl 0x080258A8\n"
        "\tadds\tr4, r7, #0\n"
        "\tadds\tr4, #232\n"
        "\tlsls\tr0, r0, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tstr\tr0, [r4, #0]\n"
        "\tmov\tr2, r8\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r2, r3]\n"
        "\tbl 0x080258B8\n"
        "\tmovs\tr1, #170\n"
        "\tlsls\tr1, r1, #1\n"
        "\tadds\tr5, r7, r1\n"
        "\tldr r1, .Lff11484_0800115dc\n"
        "\tlsls\tr0, r0, #16\n"
        "\tasrs\tr0, r0, #15\n"
        "\tadds\tr0, r0, r1\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tstr\tr0, [r5, #0]\n"
        "\tadds\tr6, r7, #0\n"
        "\tadds\tr6, #244\n"
        "\tmov\tr2, r8\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r2, r3]\n"
        "\tstr\tr0, [r6, #0]\n"
        "\tldr\tr0, [r7, #28]\n"
        "\tldr\tr1, [r4, #0]\n"
        "\tadds\tr2, r7, #0\n"
        "\tadds\tr2, #228\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tldr\tr0, [r7, #36]\n"
        "\tldr\tr1, [r5, #0]\n"
        "\tmovs\tr4, #168\n"
        "\tlsls\tr4, r4, #1\n"
        "\tadds\tr2, r7, r4\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadds\tr0, r7, #0\n"
        "\tadds\tr0, #240\n"
        "\tldr\tr0, [r0, #0]\n"
        "\tldr\tr1, [r6, #0]\n"
        "\tmovs\tr2, #7\n"
        "\tbl 0x08026F50\n"
        "\tpop\t{r3}\n"
        "\tmov\tr8, r3\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".Lff11484_0800115dc: .word 0x080CB55C\n"
        ".syntax divided\n"
    );
}

void _080011484(void *a, int b, int c) __attribute__((alias("MenuFF78_11484")));
void sub_080011484(void *a, int b, int c) __attribute__((alias("MenuFF78_11484")));

// ============================================================================
// sub_0800115E0 — list-cursor handler w/ u16[rec+190] cursor over the record
// list. Uses _0800F744/F700/F6D0 for clamp + select logic. Same epilogue shape
// (s16[rec+212] = selected idx).
// ROM frame: `push {r4,r5,r6,r7,lr}` + `mov r7,r8` / `push {r7}` /
// `mov r8,r0` -- the u16[rec+190] cursor lives in r8. `register u16 prev
// __asm__("r8")` reproduces that staging; with it a natural draft scores
// 117/252 at 244 bytes. The `ldrsh` reads need no splice: a signed halfword
// read into an `int` local compiles to `ldrsh` even when the value crosses a
// call (only an `s16` local gets `ldrh` + `lsls`/`asrs`). What is left is the
// register each zero-index `ldrsh`'s constant lands in -- the ROM rotates
// r2/r0/r1/r3 while agbcc takes the next free low register, and pinning that
// constant does not move it. The body stays asm until that numbering is
// reachable; measurements in docs/findings/register_shaping_levers.md.
// ============================================================================
extern u32 _0800F6D0(s32 idx);            // menus.c
extern u32 _0800F700(s32 a, s32 b);       // menus.c
__attribute__((naked)) void MenuFF78_115E0(void *rec_, int a1, int a2) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, r8\n"
        "\tpush\t{r7}\n"
        "\tadds\tr6, r0, #0\n"
        "\tlsls\tr2, r2, #16\n"
        "\tlsrs\tr2, r2, #16\n"
        "\tadds\tr5, r6, #0\n"
        "\tadds\tr5, #190\n"
        "\tldrh\tr0, [r5, #0]\n"
        "\tmov\tr8, r0\n"
        "\tcmp\tr2, #32\n"
        "\tbne .Lff115e0_0800115fc\n"
        "\tsubs\tr0, #1\n"
        "\tstrh\tr0, [r5, #0]\n"
        ".Lff115e0_0800115fc:\n"
        "\tcmp\tr2, #16\n"
        "\tbne .Lff115e0_080011606\n"
        "\tldrh\tr0, [r5, #0]\n"
        "\tadds\tr0, #1\n"
        "\tstrh\tr0, [r5, #0]\n"
        ".Lff115e0_080011606:\n"
        "\tadds\tr7, r6, #0\n"
        "\tadds\tr7, #188\n"
        "\tldrh\tr1, [r5, #0]\n"
        "\tcmp\tr1, #99\n"
        "\tbne .Lff115e0_080011628\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr4, [r7, r2]\n"
        "\tmovs\tr0, #0\n"
        "\tbl sub_0800F744\n"
        "\tadds\tr1, r0, #0\n"
        "\tadds\tr0, r4, #0\n"
        "\tbl sub_0800F700\n"
        "\tstrh\tr0, [r5, #0]\n"
        "\tldrh\tr3, [r5, #0]\n"
        "\tmov\tr8, r3\n"
        ".Lff115e0_080011628:\n"
        "\tmovs\tr0, #0\n"
        "\tldrsh\tr4, [r5, r0]\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r7, r1]\n"
        "\tbl sub_0800F6D0\n"
        "\tcmp\tr4, r0\n"
        "\tblt .Lff115e0_08001164a\n"
        "\tldrh\tr2, [r5, #0]\n"
        "\tcmp\tr2, #99\n"
        "\tbeq .Lff115e0_08001164a\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r7, r3]\n"
        "\tbl sub_0800F6D0\n"
        "\tsubs\tr0, #1\n"
        "\tstrh\tr0, [r5, #0]\n"
        ".Lff115e0_08001164a:\n"
        "\tadds\tr0, r6, #0\n"
        "\tadds\tr0, #190\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r0, r2]\n"
        "\tadds\tr4, r0, #0\n"
        "\tcmp\tr1, #0\n"
        "\tbge .Lff115e0_08001165c\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r4, #0]\n"
        ".Lff115e0_08001165c:\n"
        "\tldrh\tr0, [r4, #0]\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #198\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmov\tr3, r8\n"
        "\tlsls\tr0, r3, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r4, r2]\n"
        "\tcmp\tr0, r1\n"
        "\tbeq .Lff115e0_0800116a8\n"
        "\tcmp\tr1, #99\n"
        "\tbeq .Lff115e0_0800116a8\n"
        "\tmovs\tr0, #3\n"
        "\tbl 0x0802B368\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r7, r3]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r4, r2]\n"
        "\tbl sub_0800F7EC\n"
        "\tadds\tr1, r0, #0\n"
        "\tmovs\tr3, #176\n"
        "\tlsls\tr3, r3, #1\n"
        "\tadds\tr0, r6, r3\n"
        "\tlsls\tr1, r1, #16\n"
        "\tasrs\tr1, r1, #16\n"
        "\tstr\tr1, [r0, #0]\n"
        "\tldr\tr0, [r6, #36]\n"
        "\tsubs\tr3, #4\n"
        "\tadds\tr2, r6, r3\n"
        "\tldr\tr2, [r2, #0]\n"
        "\tbl sub_08007ABC\n"
        "\tadds\tr0, r6, #0\n"
        "\tbl sub_0800F810\n"
        ".Lff115e0_0800116a8:\n"
        "\tadds\tr0, r7, #0\n"
        "\tldrh\tr1, [r0, #0]\n"
        "\tcmp\tr1, #1\n"
        "\tbne .Lff115e0_0800116c0\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr0, [r0, r2]\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r4, r3]\n"
        "\tbl sub_0800F700\n"
        "\tsubs\tr0, #1\n"
        "\tb .Lff115e0_0800116cc\n"
        ".Lff115e0_0800116c0:\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r7, r1]\n"
        "\tmovs\tr2, #0\n"
        "\tldrsh\tr1, [r4, r2]\n"
        "\tbl sub_0800F700\n"
        ".Lff115e0_0800116cc:\n"
        "\tadds\tr1, r6, #0\n"
        "\tadds\tr1, #212\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tpop\t{r3}\n"
        "\tmov\tr8, r3\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        ".syntax divided\n"
    );
}

void _0800115E0(void *a, int b, int c) __attribute__((alias("MenuFF78_115E0")));
void sub_0800115E0(void *a, int b, int c) __attribute__((alias("MenuFF78_115E0")));

// ============================================================================
// sub_0800116DC — small cursor handler w/ u16[rec+192] (clamped 0..2, 99→0),
// mirrors to u16[rec+200], and the same epilogue 0x15C bind + s16[rec+212].
// ============================================================================
void MenuFF78_116DC(void *rec_, int unused, int a2) {
    (void)unused;
    __asm__(".globl MenuFF78Table116DC\nMenuFF78Table116DC = 0x080CB55C\n");
    extern const u16 MenuFF78Table116DC[];
    register u8 *r4 __asm__("r4") = (u8 *)rec_;
    register u32 mode __asm__("r2") = (u32)a2;
    mode = (u16)mode;
    register volatile u8 *r1 __asm__("r1") = r4;
    r1 += 192;
    register u16 old __asm__("r3") = *(volatile u16 *)r1;

    if (mode == 32) {
        register u16 dec __asm__("r0") = (u16)(old - 1);
        *(volatile u16 *)r1 = dec;
    }
    if (mode == 16) {
        register u16 inc __asm__("r0") = *(volatile u16 *)r1;
        inc = (u16)(inc + 1);
        *(volatile u16 *)r1 = inc;
    }
    if (*(volatile u16 *)r1 == 99) {
        *(volatile u16 *)r1 = 0;
        old = 0;
    }
    {
        register u32 zeroClamp __asm__("r2") = 0;
        register s32 clampValue __asm__("r0");
        clampValue = *(s16 *)((u8 *)r1 + zeroClamp);
        if (clampValue > 2 && clampValue != 99)
            *(volatile u16 *)r1 = 2;
    }
    {
        register u8 *p0 __asm__("r0") = r4;
        p0 += 192;
        register u32 zeroCursor __asm__("r2") = 0;
        register s32 signedValue __asm__("r1");
        signedValue = *(s16 *)(p0 + zeroCursor);
        register u8 *r5 __asm__("r5") = p0;
        if (signedValue < 0)
            *(volatile u16 *)r5 = 0;
        register u16 mirror __asm__("r1") = *(volatile u16 *)r5;
        register u8 *p200 __asm__("r0") = r4;
        p200 += 200;
        *(volatile u16 *)p200 = mirror;

        register s32 previous __asm__("r0") = (s16)old;
        register u32 zeroCompare __asm__("r3") = 0;
        register s32 currentValue __asm__("r1");
        currentValue = *(s16 *)(r5 + zeroCompare);
        if (previous != currentValue && currentValue != 99) {
            _0802B368(3);
            register u32 destOffset __asm__("r0") = 182;
            __asm__ volatile ("" : "+r"(destOffset));
            destOffset <<= 1;
            register u32 dest __asm__("r2") =
                (u32)(uintptr_t)r4 + destOffset;
            __asm__ volatile ("" : "+r"(dest));

            register u32 tableBase __asm__("r1") =
                (u32)(uintptr_t)MenuFF78Table116DC;
            __asm__ volatile ("" : "+r"(tableBase));
            register u32 tableZero __asm__("r3") = 0;
            register s32 row __asm__("r0");
            row = *(s16 *)(r5 + tableZero);
            row <<= 1;
            tableBase += 20;
            row += tableBase;
            register u32 selected __asm__("r1") =
                *(volatile u16 *)(uintptr_t)row;
            *(volatile u32 *)(uintptr_t)dest = selected;

            register u32 firstArg __asm__("r0") =
                *(volatile u32 *)(r4 + 36);
            register u32 laneOffset __asm__("r3") = 180;
            __asm__ volatile ("" : "+r"(laneOffset));
            laneOffset <<= 1;
            register u32 lane __asm__("r2") =
                (u32)(uintptr_t)r4 + laneOffset;
            __asm__ volatile ("" : "+r"(lane));
            lane = *(volatile u32 *)(uintptr_t)lane;
            _08007ABC((void *)(uintptr_t)firstArg, (int)selected, (int)lane);
            _0800F810(r4);
        }
        register u16 finalValue __asm__("r1") = *(volatile u16 *)r5;
        register u8 *endDest __asm__("r0") = r4;
        endDest += 212;
        *(volatile u16 *)endDest = finalValue;
    }
}
void _0800116DC(void *a, int b, int c) __attribute__((alias("MenuFF78_116DC")));
void sub_0800116DC(void *a, int b, int c) __attribute__((alias("MenuFF78_116DC")));
