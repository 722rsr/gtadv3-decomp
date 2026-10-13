// ============================================================================
// race_scene_d2.c — reconstructed C for asm/race_scene.s (5 functions):
//
//   _08001EE2C (0x0801EE2C) — key/counter handler: setup latches + lane walk
//   _08001EFAC (0x0801EFAC) — big scene assembly: 7B18/7BFC chains + grid loops
//   _08001FA64 (0x0801FA64) — zero-terminated byte-stream 2ED0 emitter
//   _08001FADC (0x0801FADC) — counted byte-stream 2ED0 emitter (hidden s0 mode)
//   _08001FC14 (0x0801FC14) — scene setup: FBC switch + class branch + ctor chain
//
// Transcribed instruction-for-instruction from asm/race_scene.s (pure Thumb,
// byte-exact via make).

#include "gba/types.h"

#define HOST_STUB(sig) extern sig

#define D2_CALLEE(friendly, closure) closure
extern void *sub_08004B68(void);
extern void sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void sub_0802B214(int v);
extern int sub_0800572C(int v);
extern s16 sub_080258A8(int idx);

HOST_STUB(void _08002B368(u16 v));                                              // 0x08002B368 sound cue
HOST_STUB(void _08002B214(int v));                                              // 0x08002B214 sound wake
HOST_STUB(void *_08004B68(void));                                               // 0x08004B68 scene mgr
HOST_STUB(void _08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h)); // 0x08007B18 emit
HOST_STUB(void Sub_08007BFC(void *a, int b, int c, int d, int e, int f, int g, int h, int i)); // 0x08007BFC exact ROM
HOST_STUB(void _08002ED0(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j)); // 0x08002ED0 10 machine args
HOST_STUB(void _08007770(int a, void *b, int c, int d, u32 e, u32 f));         // 0x08007770 6 machine args
HOST_STUB(void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f));      // 0x08007770 exact ROM body
HOST_STUB(void _080075E8(void *a, int b, int c));                               // 0x080075E8 emit lane
HOST_STUB(void _0800798C(void *a, void *b));                                    // 0x0800798C resource setup
HOST_STUB(void _08007A58(void *p));                                             // 0x08007A58 resource init
HOST_STUB(void _08007ABC(void *a, void *b, int c));                             // 0x08007ABC resource bind
HOST_STUB(int _0800572C(int v));                                                // 0x0800572C IWRAM bump
HOST_STUB(s16 _0800258A8(int idx));                                             // 0x0800258A8 table lookup
HOST_STUB(int _080025F20(int a));                                               // 0x080025F20 grid count
HOST_STUB(void _080026A2C(void *a, s16 b, u16 c));                              // 0x080026A2C sprite XY
HOST_STUB(void _0800DBE8(void *p));                                             // 0x0800DBE8 record setup
HOST_STUB(void _0800DAB8(void *p));                                             // 0x0800DAB8 obj init
HOST_STUB(void _0800D77C(void *a, int b, int c));                               // 0x0800D77C slide setup
HOST_STUB(int _08001F9EC(int x));                                               // 0x08001F9EC s16 table 0x080CBE78
HOST_STUB(int _08001F9FC(int x));                                               // 0x08001F9FC s16 table 0x080CBE82
HOST_STUB(void _08001FA0C(void));                                               // 0x08001FA0C 64-lane template init
HOST_STUB(void _08001FE24(void *rec));                                          // 0x08001FE24 record setup (race_scene_e.c)
HOST_STUB(void _08001FF24(void *rec));                                          // 0x08001FF24 record setup (race_scene_e.c)
HOST_STUB(void _08001FF7C(void *rec));                                          // 0x08001FF7C record setup (race_scene_e.c)

// ---- shared anchors --------------------------------------------------------
#define WA 0x03001780u
static inline u8  R8(volatile u8 *p, u32 o)  { return *(volatile u8 *)(p + o); }
static inline s8  S8(volatile u8 *p, u32 o)  { return *(volatile s8 *)(p + o); }
static inline u16 R16(volatile u8 *p, u32 o) { return *(volatile u16 *)(p + o); }
static inline s16 S16(volatile u8 *p, u32 o) { return *(volatile s16 *)(p + o); }
static inline u32 R32(volatile u8 *p, u32 o) { return *(volatile u32 *)(p + o); }
static inline void W8(volatile u8 *p, u32 o, u8 v)   { *(volatile u8 *)(p + o) = v; }
static inline void W16(volatile u8 *p, u32 o, u16 v) { *(volatile u16 *)(p + o) = v; }
static inline void W32(volatile u8 *p, u32 o, u32 v) { *(volatile u32 *)(p + o) = v; }

// ----------------------------------------------------------------------------
// ---- 0x08001EE2C — key/counter handler: setup latches + lane walk ----
// Pools: {WA 0x03001780, table 0x080CBC24}. r0=rec, r1 dead, r2=(u16)c.
// Entry snapshots: r7=u16[rec+142], r8=u16[rec+136] (compared at the tail).
// ----------------------------------------------------------------------------
__attribute__((naked)) void _08001EE2C(void *rec0, u16 b, u16 c) {
    __asm__ volatile (
        ".syntax unified\n"
        "\tpush\t{r4, r5, r6, r7, lr}\n"
        "\tmov\tr7, r8\n"
        "\tpush\t{r7}\n"
        "\tadds\tr4, r0, #0\n"
        "\tlsls\tr2, r2, #16\n"
        "\tlsrs\tr6, r2, #16\n"
        "\tadds\tr0, #142\n"
        "\tldrh\tr7, [r0, #0]\n"
        "\tadds\tr5, r4, #0\n"
        "\tadds\tr5, #136\n"
        "\tldrh\tr0, [r5, #0]\n"
        "\tmov\tr8, r0\n"
        "\tcmp\tr6, #2\n"
        "\tbne .Lx_08001ee2c_08001EE5A\n"
        "\tmovs\tr1, #10\n"
        "\tstr\tr1, [r4, #24]\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r4, #28]\n"
        "\tstr\tr1, [r4, #56]\n"
        "\tstr\tr0, [r4, #52]\n"
        "\tmovs\tr0, #4\n"
        "\tbl 0x0802B368\n"
        ".Lx_08001ee2c_08001EE5A:\n"
        "\tcmp\tr6, #1\n"
        "\tbne .Lx_08001ee2c_08001EECE\n"
        "\tmovs\tr1, #0\n"
        "\tldrsh\tr0, [r5, r1]\n"
        "\tcmp\tr0, #7\n"
        "\tbgt .Lx_08001ee2c_08001EEB8\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr1, r4, #0\n"
        "\tadds\tr1, #144\n"
        "\tadds\tr1, r1, r0\n"
        "\tldrh\tr0, [r1, #0]\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lx_08001ee2c_08001EEB0\n"
        "\tmovs\tr1, #10\n"
        "\tstr\tr1, [r4, #24]\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r4, #28]\n"
        "\tstr\tr1, [r4, #56]\n"
        "\tstr\tr6, [r4, #52]\n"
        "\tadds\tr1, r4, #0\n"
        "\tadds\tr1, #138\n"
        "\tmovs\tr0, #4\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tldr r1, .Lx_08001ee2c_08001EEA8\n"
        "\tldr r2, .Lx_08001ee2c_08001EEAC\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr0, [r5, r3]\n"
        "\tlsls\tr0, r0, #1\n"
        "\tadds\tr0, r0, r2\n"
        "\tldrh\tr0, [r0, #0]\n"
        "\tmovs\tr2, #252\n"
        "\tlsls\tr2, r2, #4\n"
        "\tadds\tr1, r1, r2\n"
        "\tstrh\tr0, [r1, #0]\n"
        "\tmovs\tr0, #1\n"
        "\tbl 0x0802B368\n"
        "\tb .Lx_08001ee2c_08001EECE\n"
        "\tmovs\tr0, r0\n"
        ".Lx_08001ee2c_08001EEA8: .word 0x03001780\n"
        ".Lx_08001ee2c_08001EEAC: .word 0x080CBC24\n"
        ".Lx_08001ee2c_08001EEB0:\n"
        "\tmovs\tr0, #10\n"
        "\tbl 0x0802B368\n"
        "\tb .Lx_08001ee2c_08001EECE\n"
        ".Lx_08001ee2c_08001EEB8:\n"
        "\tcmp\tr0, #8\n"
        "\tbne .Lx_08001ee2c_08001EECE\n"
        "\tmovs\tr1, #10\n"
        "\tstr\tr1, [r4, #24]\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r4, #28]\n"
        "\tstr\tr1, [r4, #56]\n"
        "\tstr\tr0, [r4, #52]\n"
        "\tmovs\tr0, #1\n"
        "\tbl 0x0802B368\n"
        ".Lx_08001ee2c_08001EECE:\n"
        "\tadds\tr0, r4, #0\n"
        "\tadds\tr0, #136\n"
        "\tmovs\tr3, #0\n"
        "\tldrsh\tr1, [r0, r3]\n"
        "\tadds\tr2, r0, #0\n"
        "\tcmp\tr6, #32\n"
        "\tbne .Lx_08001ee2c_08001EEF0\n"
        "\tldrh\tr0, [r2, #0]\n"
        "\tcmp\tr0, #8\n"
        "\tbne .Lx_08001ee2c_08001EEE8\n"
        "\tmovs\tr0, #3\n"
        "\tstrh\tr0, [r2, #0]\n"
        "\tb .Lx_08001ee2c_08001EEF0\n"
        ".Lx_08001ee2c_08001EEE8:\n"
        "\tsubs\tr1, #4\n"
        "\tcmp\tr1, #0\n"
        "\tblt .Lx_08001ee2c_08001EEF0\n"
        "\tstrh\tr1, [r2, #0]\n"
        ".Lx_08001ee2c_08001EEF0:\n"
        "\tcmp\tr6, #16\n"
        "\tbne .Lx_08001ee2c_08001EF08\n"
        "\tldrh\tr3, [r2, #0]\n"
        "\tcmp\tr3, #8\n"
        "\tbne .Lx_08001ee2c_08001EF00\n"
        "\tmovs\tr0, #7\n"
        "\tstrh\tr0, [r2, #0]\n"
        "\tb .Lx_08001ee2c_08001EF08\n"
        ".Lx_08001ee2c_08001EF00:\n"
        "\tadds\tr1, #4\n"
        "\tcmp\tr1, #7\n"
        "\tbgt .Lx_08001ee2c_08001EF08\n"
        "\tstrh\tr1, [r2, #0]\n"
        ".Lx_08001ee2c_08001EF08:\n"
        "\tadds\tr1, r4, #0\n"
        "\tadds\tr1, #142\n"
        "\tcmp\tr6, #64\n"
        "\tbne .Lx_08001ee2c_08001EF3C\n"
        "\tldrh\tr0, [r2, #0]\n"
        "\tcmp\tr0, #8\n"
        "\tbne .Lx_08001ee2c_08001EF2C\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r1, r4]\n"
        "\tcmp\tr0, #0\n"
        "\tbeq .Lx_08001ee2c_08001EF24\n"
        "\tcmp\tr0, #1\n"
        "\tbeq .Lx_08001ee2c_08001EF28\n"
        "\tb .Lx_08001ee2c_08001EF3C\n"
        ".Lx_08001ee2c_08001EF24:\n"
        "\tmovs\tr0, #3\n"
        "\tb .Lx_08001ee2c_08001EF3A\n"
        ".Lx_08001ee2c_08001EF28:\n"
        "\tmovs\tr0, #7\n"
        "\tb .Lx_08001ee2c_08001EF3A\n"
        ".Lx_08001ee2c_08001EF2C:\n"
        "\tsubs\tr0, #1\n"
        "\tstrh\tr0, [r2, #0]\n"
        "\tlsls\tr0, r0, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tcmp\tr0, #3\n"
        "\tbne .Lx_08001ee2c_08001EF3C\n"
        "\tmovs\tr0, #4\n"
        ".Lx_08001ee2c_08001EF3A:\n"
        "\tstrh\tr0, [r2, #0]\n"
        ".Lx_08001ee2c_08001EF3C:\n"
        "\tcmp\tr6, #128\n"
        "\tbne .Lx_08001ee2c_08001EF52\n"
        "\tldrh\tr0, [r2, #0]\n"
        "\tadds\tr0, #1\n"
        "\tstrh\tr0, [r2, #0]\n"
        "\tlsls\tr0, r0, #16\n"
        "\tasrs\tr0, r0, #16\n"
        "\tcmp\tr0, #4\n"
        "\tbne .Lx_08001ee2c_08001EF52\n"
        "\tmovs\tr0, #8\n"
        "\tstrh\tr0, [r2, #0]\n"
        ".Lx_08001ee2c_08001EF52:\n"
        "\tadds\tr3, r2, #0\n"
        "\tldrh\tr0, [r3, #0]\n"
        "\tcmp\tr0, #3\n"
        "\tbhi .Lx_08001ee2c_08001EF5E\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r1, #0]\n"
        ".Lx_08001ee2c_08001EF5E:\n"
        "\tldrh\tr0, [r3, #0]\n"
        "\tsubs\tr0, #4\n"
        "\tlsls\tr0, r0, #16\n"
        "\tlsrs\tr0, r0, #16\n"
        "\tcmp\tr0, #3\n"
        "\tbhi .Lx_08001ee2c_08001EF6E\n"
        "\tmovs\tr0, #1\n"
        "\tstrh\tr0, [r1, #0]\n"
        ".Lx_08001ee2c_08001EF6E:\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r3, r4]\n"
        "\tcmp\tr0, #0\n"
        "\tbgt .Lx_08001ee2c_08001EF7A\n"
        "\tmovs\tr0, #0\n"
        "\tstrh\tr0, [r3, #0]\n"
        ".Lx_08001ee2c_08001EF7A:\n"
        "\tmovs\tr4, #0\n"
        "\tldrsh\tr0, [r3, r4]\n"
        "\tcmp\tr0, #7\n"
        "\tble .Lx_08001ee2c_08001EF86\n"
        "\tmovs\tr0, #8\n"
        "\tstrh\tr0, [r3, #0]\n"
        ".Lx_08001ee2c_08001EF86:\n"
        "\tldrh\tr1, [r1, #0]\n"
        "\tcmp\tr1, r7\n"
        "\tbeq .Lx_08001ee2c_08001EF94\n"
        "\tmovs\tr0, #3\n"
        "\tbl 0x0802B368\n"
        "\tb .Lx_08001ee2c_08001EFA0\n"
        ".Lx_08001ee2c_08001EF94:\n"
        "\tldrh\tr2, [r2, #0]\n"
        "\tcmp\tr2, r8\n"
        "\tbeq .Lx_08001ee2c_08001EFA0\n"
        "\tmovs\tr0, #2\n"
        "\tbl 0x0802B368\n"
        ".Lx_08001ee2c_08001EFA0:\n"
        "\tpop\t{r3}\n"
        "\tmov\tr8, r3\n"
        "\tpop\t{r4, r5, r6, r7}\n"
        "\tpop\t{r0}\n"
        "\tbx\tr0\n"
        "\tmovs\tr0, r0\n"
        ".syntax divided\n"
    );
}

// ----------------------------------------------------------------------------
// ---- 0x08001EFAC — big scene assembly: 7B18/7BFC chains + grid loops ----
// Pools: {0x080CBBE4 (8-byte entry table), WA 0x03001780 (s8 thresholds)}.
// r0=rec only. armcc high-register frame (r5=3, r6=1, sl=0 staging the
// shared (3,1,1,0) stack tail of the lane emits).
// ----------------------------------------------------------------------------
__attribute__((naked)) void _08001EFAC(void *rec0) {
    __asm__ volatile (
        ".syntax unified\n"
        "push	{r4, r5, r6, r7, lr}\n"
        "mov	r7, sl\n"
        "mov	r6, r9\n"
        "mov	r5, r8\n"
        "push	{r5, r6, r7}\n"
        "sub	sp, #24\n"
        "adds	r7, r0, #0\n"
        "adds	r0, #60\n"
        "movs	r1, #6\n"
        "str	r1, [sp, #0]\n"
        "movs	r5, #3\n"
        "str	r5, [sp, #4]\n"
        "movs	r6, #1\n"
        "str	r6, [sp, #8]\n"
        "movs	r1, #0\n"
        "mov	sl, r1\n"
        "str	r1, [sp, #12]\n"
        "movs	r1, #8\n"
        "movs	r2, #72\n"
        "movs	r3, #88\n"
        "bl 0x08007B18\n"
        "movs	r2, #134\n"
        "lsls	r2, r2, #1\n"
        "adds	r0, r7, r2\n"
        "ldr	r0, [r0, #0]\n"
        "movs	r1, #120\n"
        "movs	r2, #56\n"
        "bl 0x08026A2C\n"
        "adds	r4, r7, #0\n"
        "adds	r4, #16\n"
        "str	r5, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #12]\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #2\n"
        "movs	r2, #8\n"
        "movs	r3, #40\n"
        "bl 0x08007B18\n"
        "str	r5, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r1, sl\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #8\n"
        "movs	r2, #0\n"
        "movs	r3, #64\n"
        "bl 0x08007B18\n"
        "str	r5, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r2, sl\n"
        "str	r2, [sp, #12]\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #6\n"
        "movs	r2, #0\n"
        "movs	r3, #88\n"
        "bl 0x08007B18\n"
        "str	r5, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #12]\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #5\n"
        "movs	r2, #16\n"
        "movs	r3, #112\n"
        "bl 0x08007B18\n"
        "str	r5, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r1, sl\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #3\n"
        "movs	r2, #192\n"
        "movs	r3, #40\n"
        "bl 0x08007B18\n"
        "str	r5, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r2, sl\n"
        "str	r2, [sp, #12]\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #7\n"
        "movs	r2, #200\n"
        "movs	r3, #64\n"
        "bl 0x08007B18\n"
        "str	r5, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #12]\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #4\n"
        "movs	r2, #200\n"
        "movs	r3, #88\n"
        "bl 0x08007B18\n"
        "str	r5, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r1, sl\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r4, #0\n"
        "movs	r1, #1\n"
        "movs	r2, #192\n"
        "movs	r3, #112\n"
        "bl 0x08007B18\n"
        "subs	r4, #8\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #160\n"
        "ldr	r1, [r0, #0]\n"
        "adds	r0, #4\n"
        "ldr	r2, [r0, #0]\n"
        "movs	r0, #40\n"
        "mov	r9, r0\n"
        "str	r0, [sp, #0]\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #168\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "str	r6, [sp, #12]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #16]\n"
        "adds	r0, r4, #0\n"
        "movs	r3, #48\n"
        "bl 0x08007BFC\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #172\n"
        "ldr	r1, [r0, #0]\n"
        "adds	r0, #4\n"
        "ldr	r2, [r0, #0]\n"
        "movs	r0, #64\n"
        "mov	r8, r0\n"
        "str	r0, [sp, #0]\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #180\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "str	r6, [sp, #12]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #16]\n"
        "adds	r0, r4, #0\n"
        "movs	r3, #48\n"
        "bl 0x08007BFC\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #184\n"
        "ldr	r1, [r0, #0]\n"
        "adds	r0, #4\n"
        "ldr	r2, [r0, #0]\n"
        "movs	r0, #88\n"
        "str	r0, [sp, #0]\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #192\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "str	r6, [sp, #12]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #16]\n"
        "adds	r0, r4, #0\n"
        "movs	r3, #48\n"
        "bl 0x08007BFC\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #196\n"
        "ldr	r1, [r0, #0]\n"
        "adds	r0, #4\n"
        "ldr	r2, [r0, #0]\n"
        "movs	r5, #112\n"
        "str	r5, [sp, #0]\n"
        "adds	r0, #4\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "str	r6, [sp, #12]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #16]\n"
        "adds	r0, r4, #0\n"
        "movs	r3, #48\n"
        "bl 0x08007BFC\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #208\n"
        "ldr	r1, [r0, #0]\n"
        "adds	r0, #4\n"
        "ldr	r2, [r0, #0]\n"
        "mov	r0, r9\n"
        "str	r0, [sp, #0]\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #216\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "str	r6, [sp, #12]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #16]\n"
        "adds	r0, r4, #0\n"
        "movs	r3, #168\n"
        "bl 0x08007BFC\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #220\n"
        "ldr	r1, [r0, #0]\n"
        "adds	r0, #4\n"
        "ldr	r2, [r0, #0]\n"
        "mov	r0, r8\n"
        "str	r0, [sp, #0]\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #228\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "str	r6, [sp, #12]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #16]\n"
        "adds	r0, r4, #0\n"
        "movs	r3, #168\n"
        "bl 0x08007BFC\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #232\n"
        "ldr	r1, [r0, #0]\n"
        "adds	r0, #4\n"
        "ldr	r2, [r0, #0]\n"
        "movs	r0, #88\n"
        "str	r0, [sp, #0]\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #240\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "str	r6, [sp, #12]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #16]\n"
        "adds	r0, r4, #0\n"
        "movs	r3, #168\n"
        "bl 0x08007BFC\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #244\n"
        "ldr	r1, [r0, #0]\n"
        "adds	r0, #4\n"
        "ldr	r2, [r0, #0]\n"
        "str	r5, [sp, #0]\n"
        "adds	r0, #4\n"
        "ldr	r0, [r0, #0]\n"
        "str	r0, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "str	r6, [sp, #12]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #16]\n"
        "adds	r0, r4, #0\n"
        "movs	r3, #168\n"
        "bl 0x08007BFC\n"
        "adds	r4, #128\n"
        "ldrh	r1, [r4, #0]\n"
        "cmp	r1, #8\n"
        "beq .L1efac_f200\n"
        "ldr r3, .L1efac_f274\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r4, r2]\n"
        "lsls	r0, r0, #3\n"
        "adds	r1, r0, r3\n"
        "ldr	r2, [r1, #0]\n"
        "adds	r3, #4\n"
        "adds	r0, r0, r3\n"
        "ldr	r3, [r0, #0]\n"
        "movs	r0, #4\n"
        "str	r0, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r0, sl\n"
        "str	r0, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #10\n"
        "bl 0x08007B18\n"
        "ldrh	r4, [r4, #0]\n"
        "cmp	r4, #8\n"
        "bne .L1efac_f21c\n"
        ".L1efac_f200:\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #138\n"
        "ldrh	r1, [r0, #0]\n"
        "movs	r0, #5\n"
        "str	r0, [sp, #0]\n"
        "str	r6, [sp, #4]\n"
        "str	r6, [sp, #8]\n"
        "mov	r2, sl\n"
        "str	r2, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r2, #104\n"
        "movs	r3, #128\n"
        "bl 0x08007B18\n"
        ".L1efac_f21c:\n"
        "movs	r5, #0\n"
        "adds	r0, r7, #0\n"
        "adds	r0, #24\n"
        "str	r0, [sp, #20]\n"
        "movs	r1, #3\n"
        "mov	r9, r1\n"
        "movs	r4, #1\n"
        "movs	r6, #40\n"
        "movs	r2, #130\n"
        "adds	r2, r2, r7\n"
        "mov	sl, r2\n"
        "mov	r8, r5\n"
        ".L1efac_f234:\n"
        "movs	r0, #0\n"
        "bl 0x08025F20\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f294\n"
        "mov	r1, sl\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "lsls	r1, r0, #1\n"
        "adds	r1, r1, r0\n"
        "lsls	r1, r1, #2\n"
        "ldr r0, .L1efac_f278\n"
        "adds	r1, r1, r0\n"
        "adds	r1, #50\n"
        "movs	r0, #0\n"
        "ldrsb	r0, [r1, r0]\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f27c\n"
        "mov	r1, r9\n"
        "str	r1, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r2, r8\n"
        "str	r2, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #11\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #48\n"
        "bl 0x08007B18\n"
        "b .L1efac_f294\n"
        "movs	r0, r0\n"
        ".L1efac_f274: .4byte 0x080CBBE4\n"
        ".L1efac_f278: .4byte 0x03001780\n"
        ".L1efac_f27c:\n"
        "mov	r0, r9\n"
        "str	r0, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r1, r8\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #12\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #48\n"
        "bl 0x08007B18\n"
        ".L1efac_f294:\n"
        "movs	r0, #1\n"
        "bl 0x08025F20\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f2f0\n"
        "mov	r2, sl\n"
        "movs	r1, #0\n"
        "ldrsh	r0, [r2, r1]\n"
        "lsls	r1, r0, #1\n"
        "adds	r1, r1, r0\n"
        "lsls	r1, r1, #2\n"
        "ldr r2, .L1efac_f2d4\n"
        "adds	r1, r1, r2\n"
        "adds	r1, #54\n"
        "movs	r0, #0\n"
        "ldrsb	r0, [r1, r0]\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f2d8\n"
        "mov	r0, r9\n"
        "str	r0, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r1, r8\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #11\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #72\n"
        "bl 0x08007B18\n"
        "b .L1efac_f2f0\n"
        "movs	r0, r0\n"
        ".L1efac_f2d4: .4byte 0x03001780\n"
        ".L1efac_f2d8:\n"
        "mov	r2, r9\n"
        "str	r2, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r0, r8\n"
        "str	r0, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #12\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #72\n"
        "bl 0x08007B18\n"
        ".L1efac_f2f0:\n"
        "movs	r0, #2\n"
        "bl 0x08025F20\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f34c\n"
        "mov	r1, sl\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "lsls	r1, r0, #1\n"
        "adds	r1, r1, r0\n"
        "lsls	r1, r1, #2\n"
        "ldr r0, .L1efac_f330\n"
        "adds	r1, r1, r0\n"
        "adds	r1, #52\n"
        "movs	r0, #0\n"
        "ldrsb	r0, [r1, r0]\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f334\n"
        "mov	r1, r9\n"
        "str	r1, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r2, r8\n"
        "str	r2, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #11\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #96\n"
        "bl 0x08007B18\n"
        "b .L1efac_f34c\n"
        "movs	r0, r0\n"
        ".L1efac_f330: .4byte 0x03001780\n"
        ".L1efac_f334:\n"
        "mov	r0, r9\n"
        "str	r0, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r1, r8\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #12\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #96\n"
        "bl 0x08007B18\n"
        ".L1efac_f34c:\n"
        "movs	r0, #3\n"
        "bl 0x08025F20\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f3a8\n"
        "mov	r2, sl\n"
        "movs	r1, #0\n"
        "ldrsh	r0, [r2, r1]\n"
        "lsls	r1, r0, #1\n"
        "adds	r1, r1, r0\n"
        "lsls	r1, r1, #2\n"
        "ldr r2, .L1efac_f38c\n"
        "adds	r1, r1, r2\n"
        "adds	r1, #55\n"
        "movs	r0, #0\n"
        "ldrsb	r0, [r1, r0]\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f390\n"
        "mov	r0, r9\n"
        "str	r0, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r1, r8\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #11\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #120\n"
        "bl 0x08007B18\n"
        "b .L1efac_f3a8\n"
        "movs	r0, r0\n"
        ".L1efac_f38c: .4byte 0x03001780\n"
        ".L1efac_f390:\n"
        "mov	r2, r9\n"
        "str	r2, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r0, r8\n"
        "str	r0, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #12\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #120\n"
        "bl 0x08007B18\n"
        ".L1efac_f3a8:\n"
        "subs	r6, #8\n"
        "adds	r5, #1\n"
        "cmp	r5, #2\n"
        "bgt .L1efac_f3b2\n"
        "b .L1efac_f234\n"
        ".L1efac_f3b2:\n"
        "movs	r5, #0\n"
        "movs	r1, #3\n"
        "mov	r9, r1\n"
        "movs	r4, #1\n"
        "mov	r8, r5\n"
        "movs	r6, #192\n"
        "movs	r2, #130\n"
        "adds	r2, r2, r7\n"
        "mov	sl, r2\n"
        ".L1efac_f3c4:\n"
        "movs	r0, #4\n"
        "bl 0x08025F20\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f420\n"
        "mov	r1, sl\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "lsls	r1, r0, #1\n"
        "adds	r1, r1, r0\n"
        "lsls	r1, r1, #2\n"
        "ldr r0, .L1efac_f404\n"
        "adds	r1, r1, r0\n"
        "adds	r1, #53\n"
        "movs	r0, #0\n"
        "ldrsb	r0, [r1, r0]\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f408\n"
        "mov	r1, r9\n"
        "str	r1, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r2, r8\n"
        "str	r2, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #11\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #48\n"
        "bl 0x08007B18\n"
        "b .L1efac_f420\n"
        "movs	r0, r0\n"
        ".L1efac_f404: .4byte 0x03001780\n"
        ".L1efac_f408:\n"
        "mov	r0, r9\n"
        "str	r0, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r1, r8\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #12\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #48\n"
        "bl 0x08007B18\n"
        ".L1efac_f420:\n"
        "movs	r0, #5\n"
        "bl 0x08025F20\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f47c\n"
        "mov	r2, sl\n"
        "movs	r1, #0\n"
        "ldrsh	r0, [r2, r1]\n"
        "lsls	r1, r0, #1\n"
        "adds	r1, r1, r0\n"
        "lsls	r1, r1, #2\n"
        "ldr r2, .L1efac_f460\n"
        "adds	r1, r1, r2\n"
        "adds	r1, #57\n"
        "movs	r0, #0\n"
        "ldrsb	r0, [r1, r0]\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f464\n"
        "mov	r0, r9\n"
        "str	r0, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r1, r8\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #11\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #72\n"
        "bl 0x08007B18\n"
        "b .L1efac_f47c\n"
        "movs	r0, r0\n"
        ".L1efac_f460: .4byte 0x03001780\n"
        ".L1efac_f464:\n"
        "mov	r2, r9\n"
        "str	r2, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r0, r8\n"
        "str	r0, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #12\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #72\n"
        "bl 0x08007B18\n"
        ".L1efac_f47c:\n"
        "movs	r0, #6\n"
        "bl 0x08025F20\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f4d8\n"
        "mov	r1, sl\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "lsls	r1, r0, #1\n"
        "adds	r1, r1, r0\n"
        "lsls	r1, r1, #2\n"
        "ldr r0, .L1efac_f4bc\n"
        "adds	r1, r1, r0\n"
        "adds	r1, #51\n"
        "movs	r0, #0\n"
        "ldrsb	r0, [r1, r0]\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f4c0\n"
        "mov	r1, r9\n"
        "str	r1, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r2, r8\n"
        "str	r2, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #11\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #96\n"
        "bl 0x08007B18\n"
        "b .L1efac_f4d8\n"
        "movs	r0, r0\n"
        ".L1efac_f4bc: .4byte 0x03001780\n"
        ".L1efac_f4c0:\n"
        "mov	r0, r9\n"
        "str	r0, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r1, r8\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #12\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #96\n"
        "bl 0x08007B18\n"
        ".L1efac_f4d8:\n"
        "movs	r0, #7\n"
        "bl 0x08025F20\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f534\n"
        "mov	r2, sl\n"
        "movs	r1, #0\n"
        "ldrsh	r0, [r2, r1]\n"
        "lsls	r1, r0, #1\n"
        "adds	r1, r1, r0\n"
        "lsls	r1, r1, #2\n"
        "ldr r2, .L1efac_f518\n"
        "adds	r1, r1, r2\n"
        "adds	r1, #56\n"
        "movs	r0, #0\n"
        "ldrsb	r0, [r1, r0]\n"
        "cmp	r5, r0\n"
        "bge .L1efac_f51c\n"
        "mov	r0, r9\n"
        "str	r0, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r1, r8\n"
        "str	r1, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #11\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #120\n"
        "bl 0x08007B18\n"
        "b .L1efac_f534\n"
        "movs	r0, r0\n"
        ".L1efac_f518: .4byte 0x03001780\n"
        ".L1efac_f51c:\n"
        "mov	r2, r9\n"
        "str	r2, [sp, #0]\n"
        "str	r4, [sp, #4]\n"
        "str	r4, [sp, #8]\n"
        "mov	r0, r8\n"
        "str	r0, [sp, #12]\n"
        "adds	r0, r7, #0\n"
        "movs	r1, #12\n"
        "adds	r2, r6, #0\n"
        "movs	r3, #120\n"
        "bl 0x08007B18\n"
        ".L1efac_f534:\n"
        "adds	r6, #8\n"
        "adds	r5, #1\n"
        "cmp	r5, #2\n"
        "bgt .L1efac_f53e\n"
        "b .L1efac_f3c4\n"
        ".L1efac_f53e:\n"
        "ldr	r0, [sp, #20]\n"
        "bl 0x0800DBE8\n"
        "add	sp, #24\n"
        "pop	{r3, r4, r5}\n"
        "mov	r8, r3\n"
        "mov	r9, r4\n"
        "mov	sl, r5\n"
        "pop	{r4, r5, r6, r7}\n"
        "pop	{r0}\n"
        "bx	r0\n"
        ".syntax divided\n"
    );
}

void _08001FA64(void *dst0, int b, const u8 *tbl0, int d) {
    volatile u8 *dst = (volatile u8 *)dst0;
    int bb = b;
    int dd = d;
    const u8 *tbl = (const u8 *)tbl0;
    const s16 *tab2;
    if (*tbl == 0)
        return;
    tab2 = (const s16 *)0x0203F8F2u;
    for (;;) {
        int s = (int)*(const s16 *)(0x080CBC74u + 2u * (u32)*tbl);
        if (s != -1) {
            int idx = s - 1;
            int w = (int)*(const s16 *)((const u8 *)tab2 + 2u * (u32)idx);
            _08002ED0((void *)dst, bb, w, dd, 1, 0, 1, 0, 0, 1);
            tbl += 1;
            dst += 8;
        }
        if (*tbl == 0)
            break;
    }
}

// asm/race_scene.s:10606-10608 defines BOTH `sub_08001FA64:` and
// `_08001FA64:` on this one body, and asm/race_scene.s:12006,12054,12174,
// 12289 call it as `bl sub_08001FA64`. A promoted entry's export must
// therefore carry both spellings; the `sub_` twin is a direct alias of the
// C body with the same signature (same pattern as _08001F918).
void sub_08001FA64(void *dst0, int b, const u8 *tbl0, int d) __attribute__((alias("_08001FA64")));

void _08001FADC(void *dst0, int b, const u8 *tbl0, int n, int mode) {
    const u8 *tbl = (const u8 *)tbl0;
    volatile u8 *dst;
    const s16 *tab2;
    int i;
    int one;
    int zero;
    if (n <= 0)
        return;
    tab2 = (const s16 *)0x0203F8F2u;
    dst = (volatile u8 *)dst0;
    one = 1;
    zero = 0;
    for (i = n; i != 0; i--) {
        int s = (int)*(const s16 *)(0x080CBC74u + 2u * (u32)*tbl);
        if (s != -1) {
            int idx = s - 1;
            int w = (int)*(const s16 *)((const u8 *)tab2 + 2u * (u32)idx);
            _08002ED0((void *)dst, b, w, mode, one, zero, one, zero, zero, one);
            tbl += 1;
        }
        dst += 8;
    }
}

// ----------------------------------------------------------------------------
// ---- 0x08001FC14 — scene setup: FBC switch + class branch + ctor chain ----
// Pools: {WA+0xFBC (s16 switch key), 0x08344450 (template), 0x082A798C,
// 0x082DFBDC, WA+0x576, 0x08345F84}. r0=rec only.
// FBC table at 0x0801FC40 (mov pc): idx 0/1/4/5/7 -> FC60 (kind 3),
// idx 2 -> FC7C (kind 4), idx 3/6 -> FC90 (empty tail), idx >7 -> FC90.
// Class branch on s16[_08004B68+0]: 24 -> wake+kind7 lane, 25 ->
// kind13 lane (+3 into rec+24 when WA+0xFBC==2), else straight to ctor.
// Tail switch on s16[rec+24] is a ROM cmp-chain (0/2/3), not a table.
// ----------------------------------------------------------------------------
void _08001FC14(void *rec0) {
    volatile u8 *rec = (volatile u8 *)rec0;
    register int fbc __asm__("r0") = (int)WA;
    register int fbcOffset __asm__("r1") = 0xFBC;
    register volatile u32 *p128 __asm__("r1");
    int alloc;
    int w;
    __asm__ volatile("" : "+r"(fbc), "+r"(fbcOffset));
    fbc += fbcOffset;
    fbcOffset = 0;
    __asm__ volatile(".code 16\n\tldrsh r0, [r0, r1]"
                     : "+r"(fbc) : "r"(fbcOffset) : "cc");
    switch (fbc) {
    case 0:
    case 1:
    case 4:
    case 5:
    case 7:
        D2_CALLEE(Sub_08007770, sub_08007770)(0, (void *)(uintptr_t)0x08344450u, 3, 0, 4, 1);
        break;
    case 2:
        D2_CALLEE(Sub_08007770, sub_08007770)(0, (void *)(uintptr_t)0x08344450u, 4, 0, 4, 1);
        break;
    default:
        break;
    }
    W32(rec, 104, 6);
    {
        volatile u8 *clsBase = (volatile u8 *)D2_CALLEE(_08004B68, sub_08004B68)();
        register int cls __asm__("r0");
        register int clsIndex __asm__("r1");
        clsIndex = 0;
        __asm__ volatile(".code 16\n\tldrsh r0, [r0, r1]"
                         : "=r"(cls) : "0"(clsBase), "r"(clsIndex) : "cc");
        switch (cls) {
        case 24:
            D2_CALLEE(_08002B214, sub_0802B214)(51);
            W32(rec, 116, 7);
            p128 = (volatile u32 *)(rec + 128);
            __asm__ volatile("" : "+r"(p128));
            *p128 = 1;
            W16(rec, 24, 0);
            break;
        case 25:
            W32(rec, 116, 13);
            p128 = (volatile u32 *)(rec + 128);
            __asm__ volatile("" : "+r"(p128));
            *p128 = 0;
            W16(rec, 24, 2);
            fbc = (int)WA;
            fbcOffset = 0xFBC;
            __asm__ volatile("" : "+r"(fbc), "+r"(fbcOffset));
            fbc += fbcOffset;
            if (*(volatile u16 *)(uintptr_t)fbc == 2)
                W16(rec, 24, 3);
            break;
        }
    }
    _0800DAB8((void *)(rec + 32));
    _080075E8((void *)(uintptr_t)0x082A798Cu, 0, 7);
    _0800798C((void *)(uintptr_t)0x08344450u, (void *)rec);
    _08007A58((void *)rec);
    _0800798C((void *)(uintptr_t)0x082DFBDCu, (void *)(rec + 8));
    _080075E8((void *)(uintptr_t)0x082DFBDCu, 0, 6);
    alloc = D2_CALLEE(_0800572C, sub_0800572C)(12);
    W32(rec, 176, (u32)alloc);
    fbc = (int)WA;
    fbcOffset = 0x576;
    __asm__ volatile("" : "+r"(fbc), "+r"(fbcOffset));
    fbc += fbcOffset;
    fbcOffset = 0;
    __asm__ volatile(".code 16\n\tldrsh r0, [r0, r1]"
                     : "+r"(fbc) : "r"(fbcOffset) : "cc");
    w = D2_CALLEE(_0800258A8, sub_080258A8)(fbc);
    {
        volatile u32 *out180 = (volatile u32 *)(rec + 180);
        s16 value180 = (s16)w;
        *out180 = (u32)(s32)value180;
        _08007ABC((void *)(uintptr_t)R32(rec, 12), (void *)(uintptr_t)(u32)(s32)value180, (int)R32(rec, 176));
    }
    _0800798C((void *)(uintptr_t)0x08345F84u, (void *)(rec + 16));
    alloc = D2_CALLEE(_0800572C, sub_0800572C)(6);
    W32(rec, 188, (u32)alloc);
    w = _08001F9EC(0);
    {
        volatile u32 *out192 = (volatile u32 *)(rec + 192);
        s16 value192 = (s16)w;
        *out192 = (u32)(s32)value192;
        _08007ABC((void *)(uintptr_t)R32(rec, 20), (void *)(uintptr_t)(u32)(s32)value192, (int)R32(rec, 188));
    }
    _0800798C((void *)(uintptr_t)0x08345F84u, (void *)(rec + 200));
    alloc = D2_CALLEE(_0800572C, sub_0800572C)(1);
    W32(rec, 200, (u32)alloc);
    w = _08001F9FC(0);
    {
        volatile u32 *out204 = (volatile u32 *)(rec + 204);
        s16 value204 = (s16)w;
        *out204 = (u32)(s32)value204;
        _08007ABC((void *)(uintptr_t)R32(rec, 20), (void *)(uintptr_t)(u32)(s32)value204, (int)R32(rec, 200));
    }
    _08001FA0C();
    W32(rec, 32, 0);
    W16(rec, 36, 1);
    _0800D77C((void *)(rec + 52), 0, -32);
    _0800D77C((void *)(rec + 44), 0, 160);
    W16(rec, 40, 6);
    W16(rec, 42, 5);
    {
        register int sel __asm__("r0");
        register int selOffset __asm__("r1") = 24;
        __asm__ volatile(".code 16\n\tldrsh r0, [r6, r1]"
                         : "=r"(sel) : "r"(selOffset), "r"(rec) : "cc");
        switch (sel) {
        case 0:
            _08001FE24((void *)rec);
            break;
        case 2:
            _08001FF24((void *)rec);
            break;
        case 3:
            _08001FF7C((void *)rec);
            break;
        }
    }
}
