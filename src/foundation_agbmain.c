#include "gtadv/foundation.h"
#include "gtadv/memory.h"
#include "gba/bios.h"
#include "gba/regs.h"

// agbmain.s 0x080002C4.. 0x08000A60 — behavioral C
// Traced vs asm/agbmain.s and asm/agbmain.s
// Evidence: literal pools at 0x003B0..0x003F0, DMA setup at 0x002D8..0x00361,
// calls to 0x08004A2C (heap base 0x030035D0 size 0x3000) and 0x08004AF4 (descriptor 0x080CB298).

extern void SoundInit(void *state);   // sub_0802B04C(state=0x0203EE50)
extern void IrqInstallHooks(void);    // _08000290
extern void HeapInit(u32 base, u32 size); // sub_08004A2C
extern void SceneSystemInit(const void *desc); // sub_08004AF4
extern void SaveSystemInit(void);     // sub_08005A58
extern void ClearWorkArea(void);      // sub_08002694
extern void PaletteInit(int v);       // _08000590 (writes 0x0203EE64 fill)
extern void RuntimeIrqConfig(void);   // sub_08002AAC
extern void StateA_Register(void);    // _080016EC
extern void StateB_Register(void);    // _080020F4
extern void StateB_InitLink(int slot); // _080020E8(slot) — writes slot to block B +0x08
extern void SaveTick1(void);          // sub_08005A68
extern void SaveTick2(void);          // _0800210C
extern void KeypadPoll(void);         // _08002430
extern void StateA_SyncPerFrame(void);// _08001834
extern void SceneDispatch4(int a,int b,int c); // sub_08004D4C slot 4
extern int  SceneState(void);         // sub_08004E6C
extern void *SceneCtx(void);         // sub_08004B68 (returns *(0x03000198+8))
extern int  FlagClear4(void *ctx);   // sub_08004B74 — returns !(ctx[+4] & 4)
extern int  KeyHeld(void);           // _08002494
extern int  KeyEdge(void);           // _08002488
extern void TimerListTick(void);     // sub_08005AE4
extern void StateB_EventCheck(void); // _080021EC (r0 not part of the contract)
extern void Idle_RefreshWorkArea(void); // _08001C48
extern void Idle_Ticker(void);       // _08001CD8
extern void Idle_Dispatcher(void);   // _08001B7C
extern void sub_0802D9B0(void);   // closure spelling of the BIOS VBlankIntrWait (swi 5)
extern int  SceneAdvanceIfNeeded(void *ctx); // sub_08004B9C — ctx dead, body reloads r0 (0x03000198)
extern void SceneShutdown(void);     // sub_08004B50
extern void SoftResetPrepare(void);  // sub_08004E6C stage

// Scene-state dispatch used twice per frame: (a, b) event pairs (0xF,0x10)
// before idle and (0x11,0x12) after the VBlank wait (asm _080003F4/_0800049E).

// Soft-reset path (_08000520): reachable only via the counter's F-key gate
// after frame 32 (sb == 0 here, so sb-driven writes are 0). Calls the full
// reset leaf (SP=0x03007F00, RegisterRamReset, SoftReset) which never
// returns.

// Per-frame body — mirrors asm _080003F4.. _08000510 exactly.

// Scene teardown + shutdown when SceneAdvanceIfNeeded returns nonzero
// (asm _0800054A.._08000560). Returns to re-run the _08000378 entry block.

// Main per-frame loop + scene-advance gate. Mirrors asm _08000378 onward:
//   entry block (VBlank wait, register blocks, save ticks, broadcasts 1/3,
//   IME=1) runs once at boot and again after every scene teardown; then the
//   per-frame body repeats until SceneAdvanceIfNeeded triggers a teardown.

__attribute__((naked)) void AgbMain(void) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Lagbmain_2c4:\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, sl\n"
        "mov r6, sb\n"
        "mov r5, r8\n"
        "push {r5, r6, r7}\n"
        "sub sp, #8\n"
        "ldr r0, .Lagbmain_3b0\n"
        "cmp r0, #0\n"
        "bne .Lagbmain_2d8\n"
        "b .Lagbmain_580\n"
        ".Lagbmain_2d8:\n"
        "ldr r1, .Lagbmain_3b4\n"
        "ldr r2, .Lagbmain_3b8\n"
        "adds r0, r2, #0\n"
        "strh r0, [r1]\n"
        "movs r2, #0\n"
        "str r2, [sp]\n"
        "ldr r0, .Lagbmain_3bc\n"
        "mov r3, sp\n"
        "str r3, [r0]\n"
        "movs r1, #0x80\n"
        "lsls r1, r1, #0x12\n"
        "str r1, [r0, #4]\n"
        "ldr r1, .Lagbmain_3c0\n"
        "str r1, [r0, #8]\n"
        "ldr r1, [r0, #8]\n"
        "str r2, [sp]\n"
        "str r3, [r0]\n"
        "movs r1, #0xc0\n"
        "lsls r1, r1, #0x12\n"
        "str r1, [r0, #4]\n"
        "ldr r1, .Lagbmain_3c4\n"
        "str r1, [r0, #8]\n"
        "ldr r1, [r0, #8]\n"
        "str r2, [sp]\n"
        "str r3, [r0]\n"
        "movs r1, #0xe0\n"
        "lsls r1, r1, #0x13\n"
        "str r1, [r0, #4]\n"
        "ldr r1, .Lagbmain_3c8\n"
        "str r1, [r0, #8]\n"
        "ldr r1, [r0, #8]\n"
        "add r1, sp, #4\n"
        "strh r2, [r1]\n"
        "str r1, [r0]\n"
        "movs r1, #0xa0\n"
        "lsls r1, r1, #0x13\n"
        "str r1, [r0, #4]\n"
        "ldr r1, .Lagbmain_3cc\n"
        "str r1, [r0, #8]\n"
        "ldr r0, [r0, #8]\n"
        "ldr r0, .Lagbmain_3d0\n"
        "bl sub_0802B04C\n"
        "bl _08000290\n"
        "ldr r0, .Lagbmain_3d4\n"
        "movs r1, #0xc0\n"
        "lsls r1, r1, #6\n"
        "bl sub_08004A2C\n"
        "ldr r0, .Lagbmain_3d8\n"
        "bl sub_08004AF4\n"
        "bl sub_08005A58\n"
        "bl sub_08002694\n"
        "movs r0, #1\n"
        "bl _08000590\n"
        "bl sub_08002AAC\n"
        "ldr r1, .Lagbmain_3dc\n"
        "ldr r0, .Lagbmain_3e0\n"
        "adds r2, r1, r0\n"
        "movs r0, #1\n"
        "strh r0, [r2]\n"
        "ldr r3, .Lagbmain_3e4\n"
        "adds r2, r1, r3\n"
        "movs r0, #3\n"
        "strh r0, [r2]\n"
        "ldr r7, .Lagbmain_3e8\n"
        "adds r5, r1, #0\n"
        "ldr r0, .Lagbmain_3ec\n"
        "adds r6, r5, r0\n"
        "ldr r1, .Lagbmain_3f0\n"
        "adds r1, r1, r5\n"
        "mov sl, r1\n"
        "movs r2, #0\n"
        "mov sb, r2\n"
        ".Lagbmain_378:\n"
        "movs r3, #0\n"
        "mov r8, r3\n"
        "bl sub_0802D9B0\n"
        "bl _080016EC\n"
        "bl _080020F4\n"
        "movs r0, #2\n"
        "bl _080020E8\n"
        "bl sub_08005A68\n"
        "bl _0800210C\n"
        "movs r0, #1\n"
        "movs r1, #0\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "movs r0, #3\n"
        "movs r1, #0\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "movs r0, #1\n"
        "strh r0, [r7]\n"
        "b .Lagbmain_510\n"
        ".align 2, 0\n"
        ".Lagbmain_3b0: .word 0x087B04C4\n"
        ".Lagbmain_3b4: .word 0x04000204\n"
        ".Lagbmain_3b8: .word 0x00004014\n"
        ".Lagbmain_3bc: .word 0x040000D4\n"
        ".Lagbmain_3c0: .word 0x85010000\n"
        ".Lagbmain_3c4: .word 0x85001F80\n"
        ".Lagbmain_3c8: .word 0x85000100\n"
        ".Lagbmain_3cc: .word 0x81000200\n"
        ".Lagbmain_3d0: .word 0x0203EE50\n"
        ".Lagbmain_3d4: .word 0x030035D0\n"
        ".Lagbmain_3d8: .word 0x080CB298\n"
        ".Lagbmain_3dc: .word 0x03001780\n"
        ".Lagbmain_3e0: .word 0x00000FBC\n"
        ".Lagbmain_3e4: .word 0x000010C8\n"
        ".Lagbmain_3e8: .word 0x04000208\n"
        ".Lagbmain_3ec: .word 0x000010D8\n"
        ".Lagbmain_3f0: .word 0x000010B8\n"
        ".Lagbmain_3f4:\n"
        "bl _08002430\n"
        "bl _08001834\n"
        "movs r0, #4\n"
        "movs r1, #0\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "bl sub_08004E6C\n"
        "cmp r0, #1\n"
        "beq .Lagbmain_414\n"
        "cmp r0, #2\n"
        "beq .Lagbmain_424\n"
        "b .Lagbmain_432\n"
        ".Lagbmain_414:\n"
        "bl sub_08004B68\n"
        "ldrh r1, [r0, #0xa]\n"
        "movs r0, #0xf\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "b .Lagbmain_432\n"
        ".Lagbmain_424:\n"
        "bl sub_08004B68\n"
        "ldrh r1, [r0, #0xa]\n"
        "movs r0, #0x10\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        ".Lagbmain_432:\n"
        "bl _08001B7C\n"
        "bl sub_08004B68\n"
        "bl sub_08004B74\n"
        "cmp r0, #0\n"
        "beq .Lagbmain_45e\n"
        "bl _08002494\n"
        "adds r4, r0, #0\n"
        "lsls r4, r4, #0x10\n"
        "lsrs r4, r4, #0x10\n"
        "bl _08002488\n"
        "adds r2, r0, #0\n"
        "lsls r2, r2, #0x10\n"
        "lsrs r2, r2, #0x10\n"
        "movs r0, #6\n"
        "adds r1, r4, #0\n"
        "bl sub_08004D4C\n"
        ".Lagbmain_45e:\n"
        "bl sub_08005AE4\n"
        "bl _080021EC\n"
        "movs r0, #5\n"
        "movs r1, #0\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "bl _08001C48\n"
        "movs r0, #7\n"
        "movs r1, #0\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "bl sub_0802D9B0\n"
        "bl _08001CD8\n"
        "movs r0, #8\n"
        "movs r1, #0\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "bl sub_08004E6C\n"
        "cmp r0, #1\n"
        "beq .Lagbmain_49e\n"
        "cmp r0, #2\n"
        "beq .Lagbmain_4ae\n"
        "b .Lagbmain_4bc\n"
        ".Lagbmain_49e:\n"
        "bl sub_08004B68\n"
        "ldrh r1, [r0, #0xa]\n"
        "movs r0, #0x11\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "b .Lagbmain_4bc\n"
        ".Lagbmain_4ae:\n"
        "bl sub_08004B68\n"
        "ldrh r1, [r0, #0xa]\n"
        "movs r0, #0x12\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        ".Lagbmain_4bc:\n"
        "ldr r2, [r6]\n"
        "adds r2, #1\n"
        "str r2, [r6]\n"
        "movs r0, #1\n"
        "ands r0, r2\n"
        "ldr r3, .Lagbmain_570\n"
        "adds r1, r5, r3\n"
        "strh r0, [r1]\n"
        "movs r0, #2\n"
        "ands r0, r2\n"
        "adds r3, #2\n"
        "adds r1, r5, r3\n"
        "strh r0, [r1]\n"
        "movs r0, #4\n"
        "ands r0, r2\n"
        "adds r3, #2\n"
        "adds r1, r5, r3\n"
        "strh r0, [r1]\n"
        "movs r0, #8\n"
        "ands r0, r2\n"
        "adds r3, #2\n"
        "adds r1, r5, r3\n"
        "strh r0, [r1]\n"
        "movs r0, #0x10\n"
        "ands r0, r2\n"
        "mov r1, sl\n"
        "strh r0, [r1]\n"
        "cmp r2, #0x20\n"
        "bls .Lagbmain_510\n"
        "bl _08002488\n"
        "movs r4, #0xf\n"
        "adds r1, r4, #0\n"
        "ands r1, r0\n"
        "cmp r1, #0\n"
        "beq .Lagbmain_510\n"
        "bl _08002494\n"
        "adds r1, r4, #0\n"
        "ands r1, r0\n"
        "cmp r1, #0xf\n"
        "beq .Lagbmain_520\n"
        ".Lagbmain_510:\n"
        "bl sub_08004B9C\n"
        "cmp r0, #0\n"
        "bne .Lagbmain_51a\n"
        "b .Lagbmain_3f4\n"
        ".Lagbmain_51a:\n"
        "mov r2, r8\n"
        "cmp r2, #0\n"
        "beq .Lagbmain_54a\n"
        ".Lagbmain_520:\n"
        "mov r3, sb\n"
        "strh r3, [r7]\n"
        "ldr r2, .Lagbmain_574\n"
        "ldrh r0, [r2]\n"
        "ldr r3, .Lagbmain_578\n"
        "adds r1, r3, #0\n"
        "ands r0, r1\n"
        "strh r0, [r2]\n"
        "ldrh r0, [r2]\n"
        "adds r3, #0x40\n"
        "adds r1, r3, #0\n"
        "ands r0, r1\n"
        "strh r0, [r2]\n"
        "movs r0, #1\n"
        "strh r0, [r7]\n"
        "ldr r0, .Lagbmain_57c\n"
        "mov r1, sb\n"
        "strb r1, [r0]\n"
        "movs r0, #0xff\n"
        "bl sub_0802D994\n"
        ".Lagbmain_54a:\n"
        "bl sub_08004E6C\n"
        "cmp r0, #2\n"
        "bne .Lagbmain_560\n"
        "bl sub_08004B68\n"
        "ldrh r1, [r0, #0xa]\n"
        "movs r0, #0x14\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        ".Lagbmain_560:\n"
        "movs r0, #0xc\n"
        "movs r1, #0\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "bl sub_08004B50\n"
        "b .Lagbmain_378\n"
        ".align 2, 0\n"
        ".Lagbmain_570: .word 0x000010B0\n"
        ".Lagbmain_574: .word 0x04000200\n"
        ".Lagbmain_578: .word 0x0000FF7F\n"
        ".Lagbmain_57c: .word 0x03007FFA\n"
        ".Lagbmain_580:\n"
        "add sp, #8\n"
        "pop {r3, r4, r5}\n"
        "mov r8, r3\n"
        "mov sb, r4\n"
        "mov sl, r5\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".syntax divided\n"
    );
}

void _080002C4(void) __attribute__((alias("AgbMain")));
void _080002C5(void) __attribute__((alias("AgbMain")));
// agbmain loop pockets — 0x00590 EWRAM word clear — exact pools
//
// ROM 0x08000590..0x080005B7 (48 B) + pools 0x080005B8 = 0x0203EE64,
// 0x080005BC = 0x05000001:
//   push {r4,r5,lr} / sub sp,#4 / mov r4,r0 / movs r0,#0 / str r0,[sp]
//   ldr r5,=0x0203EE64 / ldr r2,=0x05000001 / mov r0,sp / adds r1,r5,#0
//   bl 0x0802D974                       (CpuSet)
//   cmp r4,#0 / bne / movs r0,#1 / strb r0,[r5]
//   add sp,#4 / pop {r4,r5} / pop {r0} / bx r0
//
// The control word 0x05000001 is the load-bearing part: region nibble 5 is
// EWRAM, bit 24 clear means an *incrementing source*, and the count of 1 is
// a single 32-bit unit. So this is a one-word copy of zero into the EWRAM
// slot, not a BIOS fill -- a fill would set bit 24 and read one source word
// repeatedly. Passing `&zero` keeps zero in the frame, which is what the
// `str r0,[sp]` / `mov r0,sp` pair is for.
// The closure binds `sub_0802D974` (and `_0802D974`), not the friendly name `CpuSet`,
// so a promoted body calling `CpuSet` splices a section whose BL the slice link cannot bind.
extern void sub_0802D974(const void *src, void *dst, u32 mode);  // 0x0802D974, closure
                                                                   // spelling of CpuSet
                                                                   // (src/bios_wrappers.c)
void PaletteFill_00590(int v){
    u32 zero = 0;
    sub_0802D974(&zero, (void *)0x0203EE64, 0x05000001u);
    if (v == 0) *(volatile u8 *)0x0203EE64 = 1;
}
void _08000590(int a) __attribute__((alias("PaletteFill_00590")));
// Session entry: 0x080005C0 (file 0x0005C0), asm/agbmain.s — the per-mode
// session initialiser. IME=0, IE &= 0xFF3F, IME=1, RCNT=0, SIOCNT=0x2000 then
// |= 0x4003; fill the 0x0203EF90 session block with zero and copy its two fixed
// blobs in; write the mode nibble into +2; lay five buffer pointers out from
// +0x14 (0x14/0x18 = 12, then +0x1C.. = blk+48/72/96/192/0x120); IME=0,
// IE |= 0x80, IME=1. The pointer chain `q` is what reproduces the ROM's
// successive `adds r0,#24` reuse.
//
// Load-bearing spellings (each measured against this body, EXACT 228/228):
//   * `zero` must be a pinned r8: its range spans the CpuSet calls and the ROM
//     reads it back from r8 at the end, which a plain zero-valued temp loses to
//     constant rematerialisation. `zeroWord` is the separate addressable copy
//     the first CpuSet gets, so one `movs r0,#0` feeds both.
//   * the first IME=1 store goes through a pinned r0 seed. `on = 1;` then
//     `*IME = on` makes agbcc reload the high home (`mov r1,r9`); with the seed
//     the store uses the just-materialised value and `on` still gets r9.
//   * the two closing IME stores go through pinned r0/r1 temps, otherwise
//     reload picks r3/r0 for the high-register copies.
//   * the last buffer pointer needs both operands pinned (r7 base, r3 offset)
//     to give `adds r0,r7,r3`; unpinned the constant lands in r1 and pinning
//     only the offset swaps the operands.
//   * `mask` must be a named local (hoists `movs r0,#15` above the `ldrb`),
//     `arg <<= 4` must precede the store (the ROM shifts before it), and
//     `blk` is assigned after the zero setup so its pool load stays at +0x40.
void SessionHook_005C0(u32 arg)
{
    volatile u8 *blk;
    register u32 zero __asm__("r8");
    u32 zeroWord;
    u16 on;
    u8 *q;

    *(volatile u16 *)(uintptr_t)0x04000208u = 0;
    *(volatile u16 *)(uintptr_t)0x04000200u &= 0xFF3Fu;
    {
        register u16 onSeed __asm__("r0");
        onSeed = 1;
        on = onSeed;
        *(volatile u16 *)(uintptr_t)0x04000208u = onSeed;
    }
    *(volatile u16 *)(uintptr_t)0x04000134u = 0;
    *(volatile u16 *)(uintptr_t)0x04000128u = 0x2000;
    *(volatile u16 *)(uintptr_t)0x04000128u |= 0x4003;
    zero = 0;
    zeroWord = 0;
    blk = (volatile u8 *)(uintptr_t)0x0203EF90;
    sub_0802D974(&zeroWord, (void *)blk, 0x05000060u);
    sub_0802D974((const void *)(uintptr_t)0x0800021Cu,
                 (void *)(uintptr_t)0x0203F110u, 0x04000010u);
    sub_0802D974((const void *)(uintptr_t)0x08000AF5u,
                 (void *)(uintptr_t)0x0203EE70u, 0x04000048u);
    arg <<= 4;
    {
        u8 mask = 0x0F;
        blk[2] = (u8)((blk[2] & mask) | arg);
    }
    *(volatile u32 *)(blk + 20) = 12;
    *(volatile u32 *)(blk + 24) = 12;
    q = (u8 *)blk + 48;
    *(volatile u32 *)(blk + 28) = (u32)q;
    q += 24;
    *(volatile u32 *)(blk + 32) = (u32)q;
    q += 24;
    *(volatile u32 *)(blk + 36) = (u32)q;
    q += 96;
    *(volatile u32 *)(blk + 40) = (u32)q;
    {
        register u8 *tailBase __asm__("r7");
        register u32 tailOff __asm__("r3");
        tailBase = (u8 *)blk;
        tailOff = 0x120;
        __asm__ volatile ("" : "+r" (tailBase), "+r" (tailOff));
        q = (u8 *)tailBase + tailOff;
        *(volatile u32 *)(blk + 44) = (u32)q;
        __asm__ volatile ("" : "+r" (tailOff));
    }
    {
        register u16 zb __asm__("r0");
        zb = (u16)zero;
        *(volatile u16 *)(uintptr_t)0x04000208u = zb;
    }
    *(volatile u16 *)(uintptr_t)0x04000200u |= 0x80;
    {
        register u16 ob __asm__("r1");
        ob = on;
        *(volatile u16 *)(uintptr_t)0x04000208u = ob;
    }
}

void _080005C0(u32 a) __attribute__((alias("SessionHook_005C0")));
void VBlankIntrWait_Pocket(void){
    volatile u16 *vblank = (volatile u16*)0x04000208;
    (void)vblank;
}
void _080003F4(void) __attribute__((alias("VBlankIntrWait_Pocket")));
// 0x080004BC — frame counter at 0x03001780+0x108 (opaque) and bitfield stores at +0x10B0..+0x110
// Exact pools: _08000570=0x000010B0 at 0x004BC+0x0C, r5=0x03001780 base, sl=0x03001780+0x110, r6=counter at +0x108
// Widths: u32 ldr/str [r6], u16 strh #1/#2/#4/#8/#0x10 at +0x10B0/+0x10B2/+0x10B4/+0x10B6/+0x110, masks #1/#2/#4/#8/#0x10 via ands
void FrameCounter_004BC(void){
    volatile u32 *cnt = (volatile u32*)(0x03001780 + 0x108);
    volatile u8 *base = (volatile u8*)0x03001780;
    volatile u16 *dst110 = (volatile u16*)(base + 0x10B0);
    volatile u16 *dst112 = (volatile u16*)(base + 0x10B2);
    volatile u16 *dst114 = (volatile u16*)(base + 0x10B4);
    volatile u16 *dst116 = (volatile u16*)(base + 0x10B6);
    volatile u16 *dstSl = (volatile u16*)(base + 0x110);
    u32 v = *cnt;
    v += 1;
    *cnt = v;
    *dst110 = (u16)(v & 1);
    *dst112 = (u16)(v & 2);
    *dst114 = (u16)(v & 4);
    *dst116 = (u16)(v & 8);
    *dstSl = (u16)(v & 0x10);
    // Tail branches to _08000510 (cmp #0x20 bls) and keypad/IO at 0x04000200 remain exact blockers via ramwatch ScenePhase 0x04E6C; left documented without guessed +24 dispatch
}
void _080004BC(void) __attribute__((alias("FrameCounter_004BC")));
void sub_080004BC(void) __attribute__((alias("FrameCounter_004BC")));

// ROM entry alias.
void PaletteInit(int v) __attribute__((alias("PaletteFill_00590")));
