#ifndef GTADV_FOUNDATION_H
#define GTADV_FOUNDATION_H
#include "gba/types.h"

// Foundation lane — behavioral C headers for boot/scene/runtime clusters.
// Each asm span is represented by a C API with original VMA alias.

// --- code_295c : 0x0800295C..0x08002A3C — SUBSTANTIATED (register trace vs asm/code_295c.s) ---
void Foundation_InitCommon(u32 a0, u32 a1); // sub_0800295C
void Foundation_SaveIrqSnapshot(void);      // sub_080029D8
void Foundation_RestoreIrqSnapshot(void);   // tail at 0x02A0C

// code_2a3c : IRQ vector install helper 0x08002A3C(r0=slot, r1=handler) — SUBSTANTIATED
void IrqInstall(int slot, void *handler); // _08002A3C
void IrqResetSlot0(void);                 // _08002A68
void IrqInstallTable(void);               // _08002A80

// code_2dd86 etc — padding / no-ops — SUBSTANTIATED as no code (verified.hword 0)
void Foundation_Pad_2DD86(void); // 0x02DD86: 2 bytes padding, no function
void Foundation_Pad_2DDC6(void); // 0x02DDC6: 2 bytes padding
void Foundation_Pad_2DE96(void); // 0x02DE96+3 padding
void Foundation_Pad_2DF6A(void); // 0x02DF6A: 2 bytes padding

// --- subsystem loader block 0x08004A2C.. 0x08004E6C ---
// Status: tiny flag/counter leaves at 4B68/4B74/4CC4/4CD4/4CF0 are SUBSTANTIATED;
// large managers at 4A2C/4AF4/4D4C/4E6C remain TODO in asm/code_4*.s (see src/foundation_subsys.c)
void MgrClearBit4(void *mgr); // _08004B80: exact ldr r1,=0xFFFB; ldrh r2,[r0,#4]; ands/strh [r0,#4], r0=mgr (VMA 0x08004B80)
void MgrSetBit4(void *mgr);   // _08004B90: exact ldrh r2,[r0,#4]/orrs/strh [r0,#4], r0=mgr (VMA 0x08004B90)
void CounterClear(void); // _08004CC4
void CounterBump(u32 v);  // _08004CD4 — u32, not u8: agbcc masks a u8 PARAMETER with
                        // lsls #24 / lsrs #24 on entry, and the ROM's 0x08004CD4
                        // opens straight into `ldr r1,=MgrBlock` with no masking.
                        // Same argument the CounterRead note below makes for a
                        // u8 return. The body itself stores (u8)v, so the byte
                        // width at +0x74 is unchanged.
int CounterRead(void);   // _08004CF0 — int, not u8: the value is a ldrb 0..255, and every
                        // call site (car_tick_helpers.c, car_tick_dispatch.c) declares it int;
                        // a u8 definition against int declarations is UB, and agbcc
                        // widens a u8 result with lsls/lsrs #24 at each call — the ROM
                        // emits no such masking in 0x08008000-0x08010000.
void *MgrGet80(void);    // _08004B68
void *Load44Indexed_04C30(u32 idx); // _08004C30: exact lsls #2, adds #44, ldr Vu32, r0=idx (VMA 0x08004C30)
void Store44Indexed_04C18(u32 idx, void *val); // _08004C18: exact lsls #2, adds #44, str Vu32 (VMA 0x08004C18)
void Clear0501C(void); // _0800501C: exact CpuFastSet zero to 0x03000250, ctrl 0x05000003 (VMA 0x0800501C)

// code_57d0 etc — save/mode helpers — SUBSTANTIATED for 57D0/5988/c668/ce2c leaves
void SaveSlotConfig(int v); // _080057D0
int SaveSlotVerify(int a,int b); // _08005988
void LateDispatch_C668(int base,int id); // _0800C668/_0800C6CC
void LateFlagCheck(void *a); // _0800C730
void LateDispatch_CE2C(int id,int a,int b,void *ctx); // _0800CE2C
// code_5b3c — only single-instruction math leaves are SUBSTANTIATED (see foundation_math.c)
// Small math leaves are exposed for testing:
int  MathAbs(int v);               // at 0x05B5C — SUBSTANTIATED
int  MathSign(int v);              // at 0x05B6E — SUBSTANTIATED
int  MathAbs16(int v);             // at 0x05B8A — SUBSTANTIATED
void MathHelper_05BA8(void *p,int angle); // at 0x05BA8 — SUBSTANTIATED (muls/asrs#12, 0x0805BAF0/0x0805CAF0)

// --- boot / agbmain / runtime ---
u8   SessionRxPoll(void);          // _0800021C (ARM stub returns old +5) — SUBSTANTIATED
void FrameCounter_004BC(void);      // _080004BC frame counter bits at 0x03001780+0x108/+0x10B0 — SUBSTANTIATED (opaque IWRAM)
void IrqInstallHooks(void);        // _08000290 installs VBlank/VCounter — SUBSTANTIATED
void RuntimeIrqConfig(void);       // _08002AAC DISPSTAT/IE setup — SUBSTANTIATED
void RuntimeMemcpy(void *dst, const void *src, u32 n); // _0802E0A4 — SUBSTANTIATED (verified memcpy)
void RuntimeMemset(void *dst, int c, u32 n);            // _0802E104 — SUBSTANTIATED (verified memset)
// AgbMain (0x02C4) remains TODO — early init sequence not yet traced against agbmain.s
// handlers / softirq — EWRAM ISRs and VBlank kicker (handlers.s/softirq.s)
void EWRAM_Handler_BlobA(void); // _08000AF4
void EWRAM_Handler_BlobB(void); // _08000E80
void InstallEWRAMHandlers(int checksumMode, u32 inputBase); // _08000BF0
void SoftIrqKicker(void); // _08000A60
// code_c668 / ce2c / fa0 — c668/ce2c SUBSTANTIATED, fa0 entry SUBSTANTIATED, remainder TODO
void IntrMain_Dispatch(void *ctx,int v); // _08000FA0
int Helper_014A4(void *p); // _080014A4
void Helper_014B4(void *p); // _080014B4
void Helper_013BC(void *p); // _080013BC
void Helper_030CC(void); // _080030CC
// runtime_2aac small stores + placement helpers — SUBSTANTIATED
u32 Place_x4(u32 x); // _08002AF4 lsls #2 *4 + 0x0203F170
void Store_02C48(int idx, u16 val); // _08002C48 exact: ldr base 0x03000134, lsls r0#5, strh r1@+6/+30, u16 val via lsls#16/lsrs#16
// _08002C60: word-typed parameters, unlike its neighbour _08002C48. The ROM
// at 0x08002C48 has `lsls r1,#16 / lsrs r1,#16` before the store (a u16
// parameter narrowed in the prologue); 0x08002C60 has NO such pair for either
// argument, so the VMA owner takes words and `strh` drops the high half.
void Store_02C60(int idx, u32 v1, u32 v2);
void Store_02C74(int idx, int val); // _08002C74 exact: ldr base 0x03000134, lsls r0#5, strh r1@+22
void Wrap_02C84(int idx); // _08002C84 exact: ldr base 0x03000134, lsls r0#5, ldrh/negs/strh @+6
u32 Get_03130(void); // _08003130 exact: ldr r0,=0x03000134; ldr r0,[r0]; bx lr; pool _08003138 Vu32
void Store_0313C(u32 val); // _0800313C exact: ldr r1,=0x03000138; str r0,[r1]; bx lr; pool _08003144 Vu32 (typed entry despite objdump order)
void Store_02B00(u32 a,u32 b); // _08002B00
void Store_02B1C(u32 a,u32 b); // _08002B1C
void Store_02B30(u32 a,u32 b); // _08002B30
void Store_02B44(void); // _08002B44
void Store_02BB4(void); // _08002BB4
int LoadSub_02BD8(void); // _08002BD8
int Inc_02BE8(int dummy); // _08002BE8 post-inc s16 at 0x03000158 — SUBSTANTIATED (ldrh/strh + lsls#16/asrs#16, pool 0x03000158 at _08002BF8; arg ignored)
void Insert_02C34(int idx, void *node); // _08002C34 heap slot insert at 0x03000140+idx*4 — SUBSTANTIATED (ldr Vu32, lsls#2 *4, str Vu32 +8)

#endif
