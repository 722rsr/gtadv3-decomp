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
extern void VBlankIntrWait(void);
extern int  SceneAdvanceIfNeeded(void *ctx); // sub_08004B9C — ctx dead, body reloads r0 (0x03000198)
extern void SceneShutdown(void);     // sub_08004B50
extern void SoftResetPrepare(void);  // sub_08004E6C stage

// Scene-state dispatch used twice per frame: (a, b) event pairs (0xF,0x10)
// before idle and (0x11,0x12) after the VBlank wait (asm _080003F4/_0800049E).
static void AgbMain_SceneStateDispatch(u16 ev1, u16 ev2) {
    int st = SceneState(); // sub_08004E6C
    if (st == 1) {
        void *ctx = SceneCtx();
        u16 phase = *(volatile u16*)((u8*)ctx + 0x0A);
        SceneDispatch4(ev1, phase, 0);
    } else if (st == 2) {
        void *ctx = SceneCtx();
        u16 phase = *(volatile u16*)((u8*)ctx + 0x0A);
        SceneDispatch4(ev2, phase, 0);
    }
}

// Soft-reset path (_08000520): reachable only via the counter's F-key gate
// after frame 32 (sb == 0 here, so sb-driven writes are 0). Calls the full
// reset leaf (SP=0x03007F00, RegisterRamReset, SoftReset) which never
// returns.
static void AgbMain_SoftReset(void) {
    volatile u16 *ime = (volatile u16*)0x04000208;
    volatile u16 *ie = (volatile u16*)0x04000200;
    *ime = 0;
    *ie &= 0xFF7F;
    *ie &= 0xFFBF;
    *ime = 1;
    *(volatile u8*)0x03007FFA = 0;
    extern void _0802D994(void); // full reset leaf (runtime_accessors.c)
    _0802D994();                 // never returns
}

// Per-frame body — mirrors asm _080003F4.. _08000510 exactly.
static void AgbMain_PerFrameBody(void) {
    KeypadPoll();            // _08002430
    StateA_SyncPerFrame();   // _08001834
    SceneDispatch4(4,0,0);
    AgbMain_SceneStateDispatch(0x0F, 0x10);
    Idle_Dispatcher();       // _08001B7C
    void *fctx = SceneCtx(); // sub_08004B68 (unconditional, as in asm)
    if (FlagClear4(fctx)) {  // sub_08004B74(ctx) nonzero = gate open
        u16 held = (u16)KeyHeld(); // _08002494, lsls/lsrs #16
        u16 edge = (u16)KeyEdge(); // _08002488
        SceneDispatch4(6, held, edge);
    }
    TimerListTick();         // sub_08005AE4
    StateB_EventCheck(); // _080021EC
    SceneDispatch4(5,0,0);
    Idle_RefreshWorkArea();  // _08001C48
    SceneDispatch4(7,0,0);
    VBlankIntrWait();        // sub_0802D9B0 = swi 5
    Idle_Ticker();           // _08001CD8
    SceneDispatch4(8,0,0);
    AgbMain_SceneStateDispatch(0x11, 0x12);
    // _080004BC: frame counter + bit mirrors, then the >0x20 F-key gate
    {
        volatile u8 *wa = (volatile u8*)0x03001780;
        volatile u32 *cnt = (volatile u32*)(wa + 0x10D8);
        u32 v = *cnt + 1;
        *cnt = v;
        *(volatile u16*)(wa + 0x10B0) = (u16)(v & 1);
        *(volatile u16*)(wa + 0x10B2) = (u16)(v & 2);
        *(volatile u16*)(wa + 0x10B4) = (u16)(v & 4);
        *(volatile u16*)(wa + 0x10B6) = (u16)(v & 8);
        *(volatile u16*)(wa + 0x10B8) = (u16)(v & 0x10);
        if (v > 0x20) {
            if ((KeyEdge() & 0xF) != 0 && (KeyHeld() & 0xF) == 0xF)
                AgbMain_SoftReset(); // never returns
        }
    }
}

// Scene teardown + shutdown when SceneAdvanceIfNeeded returns nonzero
// (asm _0800054A.._08000560). Returns to re-run the _08000378 entry block.
static void AgbMain_Teardown(void) {
    if (SceneState() == 2) {
        void *ctx = SceneCtx();
        u16 phase = *(volatile u16*)((u8*)ctx + 0x0A);
        SceneDispatch4(0x14, phase, 0);
    }
    SceneDispatch4(0x0C, 0, 0);
    SceneShutdown();         // sub_08004B50
}

// Main per-frame loop + scene-advance gate. Mirrors asm _08000378 onward:
//   entry block (VBlank wait, register blocks, save ticks, broadcasts 1/3,
//   IME=1) runs once at boot and again after every scene teardown; then the
//   per-frame body repeats until SceneAdvanceIfNeeded triggers a teardown.
static void AgbMain_Run(void) {
    for (;;) {
        // _08000378 entry: VBlankIntrWait + register block A/B + link slots +
        // save ticks + broadcasts 1 and 3 + IME=1, then jump to _08000510.
        VBlankIntrWait();
        StateA_Register();   // _080016EC
        StateB_Register();   // _080020F4
        StateB_InitLink(2);  // _080020E8 expected count 2
        SaveTick1();         // sub_08005A68
        SaveTick2();         // _0800210C
        SceneDispatch4(1,0,0);
        SceneDispatch4(3,0,0);
        *(volatile u16*)0x04000208 = 1; // IME = 1 (r7 = 0x04000208)
        // _08000510: advance gate is checked before the first per-frame body
        // (the entry block jumps here) and after every body thereafter.
        while (SceneAdvanceIfNeeded((void *)0) == 0) { // sub_08004B9C
            AgbMain_PerFrameBody();
        }
        // advance triggered: r8 is always 0 on this path, so the F-key
        // soft reset (_08000520) only runs from the counter gate inside the
        // body; here we always take the teardown (_0800054A).
        AgbMain_Teardown();
        // loop back to the _08000378 entry block
    }
}

void AgbMain(void) { // _080002C4
    {
        // asm 0x002D8: strh 0x4014 -> WAITCNT (0x04000204)
        REG_WAITCNT = 0x4014;
        // Fill sources live on the C stack (asm uses [sp]/[sp,#4]); values
        // are zero, so only the DMA timing matters.
        volatile u32 zero32 = 0;
        volatile u16 zero16 = 0;
        // DMA3 SAD/DAD at 0x040000D4/D8, CNT as one 32-bit word at 0x040000DC
        // (the asm str r1,[r0,#8] stores CNT_L|CNT_H together), then a
        // discard read-back of CNT as the completion barrier (asm ldr r1,
        // [r0,#8]).
        // 1) EWRAM 0x02000000 <- 0, 0x10000 words (CNT_L=0 => 0x10000) = 256 KB
        *(vu32 *)0x040000D4 = (u32)(uintptr_t)&zero32;
        *(vu32 *)0x040000D8 = 0x02000000;
        *(vu32 *)0x040000DC = 0x85010000;
        (void)*(vu32 *)0x040000DC;
        // 2) IWRAM 0x03000000 <- 0, 0x1F80 words = 0x7E00 bytes (leaves stack/IRQ area intact)
        *(vu32 *)0x040000D4 = (u32)(uintptr_t)&zero32;
        *(vu32 *)0x040000D8 = 0x03000000;
        *(vu32 *)0x040000DC = 0x85001F80;
        (void)*(vu32 *)0x040000DC;
        // 3) OAM 0x07000000 <- 0, 0x100 words = 1 KB
        *(vu32 *)0x040000D4 = (u32)(uintptr_t)&zero32;
        *(vu32 *)0x040000D8 = 0x07000000;
        *(vu32 *)0x040000DC = 0x85000100;
        (void)*(vu32 *)0x040000DC;
        *(vu32 *)0x040000D4 = (u32)(uintptr_t)&zero16;
        *(vu32 *)0x040000D8 = 0x05000000;
        *(vu32 *)0x040000DC = 0x81000200;
        (void)*(vu32 *)0x040000DC;
    }
    SoundInit((void*)0x0203EE50); // sub_0802B04C r0 from pool 0x003D0
    IrqInstallHooks();            // _08000290 — SECOND, before heap init
    HeapInit(0x030035D0, 0x3000); // sub_08004A2C
    SceneSystemInit((void*)0x080CB298); // sub_08004AF4
    SaveSystemInit();
    ClearWorkArea();
    PaletteInit(1);
    RuntimeIrqConfig(); // sub_08002AAC DISPSTAT 0x2838 IE 5
    // Work-area seeds at 0x03001780+0xFBC/+0x10C8
    *(volatile u16*)(0x03001780 + 0x0FBC) = 1;
    *(volatile u16*)(0x03001780 + 0x10C8) = 3;
    AgbMain_Run();
    // unreachable — original loops forever until the reset path
}
#ifndef __APPLE__
void _080002C4(void) __attribute__((alias("AgbMain")));
#endif
#ifndef __APPLE__
void _080002C5(void) __attribute__((alias("AgbMain")));
#endif
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
// Call-site split. The closure binds `sub_0802D974` (and `_0802D974`), not the
// friendly name `CpuSet`, so a promoted body calling `CpuSet` splices a section
// whose BL the slice link cannot bind. `sub_0802D974` is itself declared only
// under `#ifndef __APPLE__` (src/bios_wrappers.c), while `CpuSet` is defined
// unconditionally, so the host build needs the friendly name. Same shape as
// `RM_CALLEE` in src/runtime_hud.c and the split at ObjTwo_03810.
#ifndef __APPLE__
extern void sub_0802D974(const void *src, void *dst, u32 mode);  // 0x0802D974, closure
                                                                   // spelling of CpuSet
                                                                   // (src/bios_wrappers.c)
#endif
void PaletteFill_00590(int v){
    u32 zero = 0;
#ifndef __APPLE__
    sub_0802D974(&zero, (void *)0x0203EE64, 0x05000001u);
#else
    CpuSet(&zero, (void *)0x0203EE64, 0x05000001u);
#endif
    if (v == 0) *(volatile u8 *)0x0203EE64 = 1;
}
#ifndef __APPLE__
void _08000590(int a) __attribute__((alias("PaletteFill_00590")));
#endif
// Session entry: 0x080005C0 (file 0x0005C0), asm/agbmain.s.
// Preserve the ROM source addresses for the copied executable IRQ handlers.
void SessionHook_005C0(int arg){
    volatile u16 *ime = (volatile u16 *)0x04000208u;
    volatile u16 *ie = (volatile u16 *)0x04000200u;
    volatile u16 *siocnt = (volatile u16 *)0x04000128u;
    volatile u8 *session = (volatile u8 *)0x0203EF90u;
    *ime = 0;
    *ie &= 0xFF3Fu;
    *ime = 1;
    *(volatile u16 *)0x04000134u = 0;
    *siocnt = 0x2000;
    *siocnt |= 0x4003;
    u32 zero = 0;
    CpuSet(&zero, (void *)session, 0x05000060u);
    CpuSet((const void *)0x0800021Cu, (void *)0x0203F110u, 0x04000010u);
    CpuSet((const void *)0x08000AF5u, (void *)0x0203EE70u, 0x04000048u);
    session[2] = (u8)((session[2] & 0x0Fu) | ((u32)arg << 4));
    *(volatile u32 *)(session + 0x14) = 12;
    *(volatile u32 *)(session + 0x18) = 12;
    *(volatile u32 *)(session + 0x1C) = 0x0203EFC0u;
    *(volatile u32 *)(session + 0x20) = 0x0203EFD8u;
    *(volatile u32 *)(session + 0x24) = 0x0203EFF0u;
    *(volatile u32 *)(session + 0x28) = 0x0203F050u;
    *(volatile u32 *)(session + 0x2C) = 0x0203F0B0u;
    *ime = 0;
    *ie |= 0x80;
    *ime = 1;
}

#ifndef __APPLE__
void _080005C0(int a) __attribute__((alias("SessionHook_005C0")));
#endif
void VBlankIntrWait_Pocket(void){
    volatile u16 *vblank = (volatile u16*)0x04000208;
    (void)vblank;
}
#ifndef __APPLE__
void _080003F4(void) __attribute__((alias("VBlankIntrWait_Pocket")));
#endif
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
#ifndef __APPLE__
void _080004BC(void) __attribute__((alias("FrameCounter_004BC")));
#endif
#ifndef __APPLE__
void sub_080004BC(void) __attribute__((alias("FrameCounter_004BC")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void PaletteInit(int v) __attribute__((alias("PaletteFill_00590")));
#endif
