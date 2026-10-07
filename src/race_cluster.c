#include "gtadv/ghost.h"
#include "gba/types.h"
#include "gtadv/memory.h"

// Reference: asm/race_cluster.s 0x0801A008-0x0801A204
// Four leaves + 8-way dispatcher on s16[WA+0x10FC] (table _0801A16C).
// Evidence pools: WA=0x03001780, off 0x10CA,0x10FC, RACE_ARRAY 0x03004E80, RACE_CTX 0x03004E20
// Widths: ldrsh for WA, ldrh for racer+26, flag 32 via _08018ACC.

extern u32 Ghost_FlagTest(u32 mask); // _08018ACC
extern void *Course_0x080263C4(int a, void *b, int c); // 0x080263C4
#ifndef __APPLE__
// Closure spellings for the two callees above. `_08018ACC` and
// `sub_080263C4` are the spellings `arm-none-eabi-nm` reports for 0x08018ACC
// and 0x080263C4, and both are defined as real bodies in src/ghost.c and
// src/runtime_state_dispatch.c -- so the call reaches C rather than a ROM veneer.
extern u32 _08018ACC(u32 mask);
extern void *sub_080263C4(int a, void *b, int c);
#endif
// ROM tail call shape (asm/race_cluster.s _0801A1B8): (u16[racer+26],
// s16[WA+0x1DE4+2i]); its return is a 32-byte tile row copied by _0802D970
// (CpuFastSet, ctrl 8 = 8 words) to 0x03005780 + i*32.
extern void *Course_0x08026520(int a, int b); // 0x08026520
extern void _0802D970(const void *a, void *b, u32 c); // swi 0x0C CpuFastSet

void Race_Cluster_A(void) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    volatile u8 *racers = (volatile u8 *)RACER_ARRAY_BASE;
    s16 cnt = *(volatile s16 *)(wa + 0x10CA);
    for (int i=0;i<cnt;i++) {
        u32 flag = Ghost_FlagTest(32);
        int sel = flag ? (i==0?2:3) : (i==0?1:5);
        u16 v = *(volatile u16 *)(racers + i*0x11C + 26);
        u32 res = (u32)(uintptr_t)Course_0x080263C4(i, (void *)(uintptr_t)v, sel);
        *(volatile u32 *)(racers + i*0x11C + 280) = res;
    }
}
void Race_Cluster_B(void) {
    volatile u8 *racers = (volatile u8 *)RACER_ARRAY_BASE;
    for (int i=0;i<2;i++) {
        u32 flag = Ghost_FlagTest(32);
        int sel = flag ? 2 : 1;
        if (i==1) sel = flag?3:4;
        u16 v = *(volatile u16 *)(racers + i*0x11C + 26);
        u32 res = (u32)(uintptr_t)Course_0x080263C4(i, (void *)(uintptr_t)v, sel);
        *(volatile u32 *)(racers + i*0x11C + 280) = res;
    }
}
void Race_Cluster_C(void) {
    volatile u8 *racers = (volatile u8 *)RACER_ARRAY_BASE;
    // The store offset must be a named local declared BEFORE the call, not the
    // inline `racers + 280`. Inline, the offset is a plain temporary and agbcc
    // gives it the lowest free register (r1); as a variable whose live range
    // spans the call it is rematerialised after the call into r2, which is what
    // the ROM does at 0x0801a0d6. This single line is the whole 44/44 body.
    u32 idx = 280;
#ifndef __APPLE__
    // Closure spellings, chosen as the twin that is BOTH a real closure label
    // and a real C body (src/ghost.c and src/runtime_state_dispatch.c respectively).
    // A bare `extern` for a name nothing in src/ defines passes the byte oracle
    // and silently reaches the ROM body instead of C; the export/call audits
    // catch exactly that. The friendly names live only on the host.
    u32 flag = _08018ACC(32);
    int sel = flag?2:1;
    u16 v = *(volatile u16 *)(racers + 26);
    u32 res = (u32)(uintptr_t)sub_080263C4(0, (void *)(uintptr_t)v, sel);
#else
    u32 flag = Ghost_FlagTest(32);
    int sel = flag?2:1;
    u16 v = *(volatile u16 *)(racers + 26);
    u32 res = (u32)(uintptr_t)Course_0x080263C4(0, (void *)(uintptr_t)v, sel);
#endif
    *(volatile u32 *)(racers + idx) = res;
}
void Race_Cluster_D(void) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    volatile u8 *racers = (volatile u8 *)RACER_ARRAY_BASE;
    u16 ph = *(volatile u16 *)(wa + 0x10FC); // ROM: ldrh
    u16 v = *(volatile u16 *)(racers + 26);
    int sel;
    if (ph == 10) {
        sel = 0;
    } else {
        (void)Ghost_FlagTest(32); // ROM calls _08018ACC on this path, result discarded
        sel = 1;
    }
    u32 res = (u32)(uintptr_t)Course_0x080263C4(0, (void *)(uintptr_t)v, sel);
    *(volatile u32 *)(racers + 280) = res;
}

void Race_Cluster_Dispatch(void) {
    volatile u8 *wa = (volatile u8 *)WORK_AREA_BASE;
    s16 ph = *(volatile s16 *)(wa + 0x10FC);
    if (ph <0 || ph>7) { Race_Cluster_D(); return; }
    switch(ph){
        case 0: Race_Cluster_A(); break;
        case 1: Race_Cluster_D(); break;
        case 2: Race_Cluster_B(); break;
        case 3: Race_Cluster_A(); break;
        case 4: Race_Cluster_C(); break;
        default: Race_Cluster_D(); break;
    }
    // tail loop over WA+0x10CA: course surface tile row per racer (ROM
    // _0801A1B8..0x0801A1EA: 26520(u16[racer+26], s16[WA+0x1DE4+2i]) then
    // CpuFastSet(ret, 0x03005780 + i*32, 8))
    s16 cnt = *(volatile s16 *)(wa + 0x10CA);
    for(int i=0;i<cnt;i++){
        u16 v = *(volatile u16 *)((volatile u8 *)RACER_ARRAY_BASE + i*0x11C + 26);
        s16 n = *(volatile s16 *)(wa + 0x1DE4 + i*2);
        void *row = Course_0x08026520((int)v, (int)n);
        _0802D970(row, (void *)(uintptr_t)(0x03005780u + (u32)i*32u), 8u);
    }
}

#ifndef __APPLE__
void _0801A008(void) __attribute__((alias("Race_Cluster_A")));
void _0801A070(void) __attribute__((alias("Race_Cluster_B")));
void _0801A0BC(void) __attribute__((alias("Race_Cluster_C")));
// The manifest entry for 0x0801a0bc exports `sub_0801A0BC` (asm spells the
// body that way), and the asm branches to it by name -- but nothing in src/
// bound that spelling, so there was no.thumb_set for _export_missing to
// re-emit and the link failed on an undefined reference.
void sub_0801A0BC(void) __attribute__((alias("Race_Cluster_C")));
void _0801A0E8(void) __attribute__((alias("Race_Cluster_D")));
void _0801A12C(void) __attribute__((alias("Race_Cluster_Dispatch")));
#endif

// ROM entry alias.
#ifndef __APPLE__
void RaceScene_Dispatch_A12C(void) __attribute__((alias("Race_Cluster_Dispatch")));
#endif
