#include "gtadv/ghost.h"
#include "gba/types.h"
#include "gtadv/memory.h"

// Reference: asm/race_cluster.s 0x0801A008-0x0801A204
// Four leaves + 8-way dispatcher on s16[WA+0x10FC] (table _0801A16C).
// Evidence pools: WA=0x03001780, off 0x10CA,0x10FC, RACE_ARRAY 0x03004E80, RACE_CTX 0x03004E20
// Widths: ldrsh for WA, ldrh for racer+26, flag 32 via _08018ACC.

extern u32 Ghost_FlagTest(u32 mask); // _08018ACC
extern void *Course_0x080263C4(int a, void *b, int c); // 0x080263C4
// Closure spellings for the two callees above. `_08018ACC` and
// `sub_080263C4` are the spellings `arm-none-eabi-nm` reports for 0x08018ACC
// and 0x080263C4, and both are defined as real bodies in src/ghost.c and
// src/runtime_state_dispatch.c -- so the call reaches C rather than a ROM veneer.
extern u32 _08018ACC(u32 mask);
extern void *sub_080263C4(int a, void *b, int c);
// ROM tail call shape (asm/race_cluster.s _0801A1B8): (u16[racer+26],
// s16[WA+0x1DE4+2i]); its return is a 32-byte tile row copied by _0802D970
// (CpuFastSet, ctrl 8 = 8 words) to 0x03005780 + i*32.
extern void *Course_0x08026520(int a, int b); // 0x08026520
extern void _0802D970(const void *a, void *b, u32 c); // swi 0x0C CpuFastSet

void Race_Cluster_A(void) {
    register volatile u8 *racers __asm__("r4") =
        (volatile u8 *)RACER_ARRAY_BASE;
    register s32 i __asm__("r5") = 0;
    register u32 waBase __asm__("r0") = WORK_AREA_BASE;
    register u32 countOffset __asm__("r2") = 0x10CAu;
    __asm__ volatile("" : "+r"(racers), "+r"(i), "+r"(waBase),
                      "+r"(countOffset));
    register volatile s16 *countCell __asm__("r1") =
        (volatile s16 *)(waBase + countOffset);
    __asm__ volatile("" : "+r"(countCell));
    register s32 count __asm__("r0") = *(s16 *)(void *)countCell;
    if (i >= count)
        return;

    register volatile s16 *countTail __asm__("r6") = countCell;
    __asm__ volatile("" : "+r"(countTail));
loop:
    {
        u32 flag;
        register int selector __asm__("r2");
        register u32 result __asm__("r0");
        flag = _08018ACC(32);
        if (flag != 0) {
            selector = 3;
            if (i == 0)
                selector = 2;
        } else {
            selector = 5;
            if (i == 0)
                selector = 1;
        }
        result = (u32)(uintptr_t)sub_080263C4(
            i, (void *)(uintptr_t)*(volatile u16 *)(racers + 26), selector);
        {
            register u32 storeOffset __asm__("r2") = 140;
            __asm__ volatile("" : "+r"(storeOffset));
            storeOffset <<= 1;
            register volatile u32 *target __asm__("r1") =
                (volatile u32 *)(racers + storeOffset);
            *target = result;
        }
    }
    i++;
    {
        register u32 stride __asm__("r0") = 142;
        stride <<= 1;
        racers += stride;
    }
    {
        register s32 nextCount __asm__("r0") =
            *(s16 *)(void *)countTail;
        if (i < nextCount)
            goto loop;
    }
}
__asm__(".align 2, 0");
void Race_Cluster_B(void) {
    register volatile u8 *racers __asm__("r4") =
        (volatile u8 *)RACER_ARRAY_BASE;
    register u32 storeOffset __asm__("r5");
    register u32 stride __asm__("r0");
    u32 flag;
    register int selector __asm__("r2");
    register u32 result __asm__("r0");
    register volatile u32 *target __asm__("r1");

    flag = _08018ACC(32);
    selector = 1;
    if (flag != 0)
        selector = 2;
    result = (u32)(uintptr_t)sub_080263C4(
        0, (void *)(uintptr_t)*(volatile u16 *)(racers + 26), selector);
    storeOffset = 140;
    storeOffset <<= 1;
    target = (volatile u32 *)(racers + storeOffset);
    *target = result;
    stride = 142;
    stride <<= 1;
    racers += stride;

    flag = _08018ACC(32);
    selector = 4;
    if (flag != 0)
        selector = 3;
    result = (u32)(uintptr_t)sub_080263C4(
        1, (void *)(uintptr_t)*(volatile u16 *)(racers + 26), selector);
    target = (volatile u32 *)(racers + storeOffset);
    *target = result;
}
void Race_Cluster_C(void) {
    volatile u8 *racers = (volatile u8 *)RACER_ARRAY_BASE;
    // The store offset must be a named local declared BEFORE the call, not the
    // inline `racers + 280`. Inline, the offset is a plain temporary and agbcc
    // gives it the lowest free register (r1); as a variable whose live range
    // spans the call it is rematerialised after the call into r2, which is what
    // the ROM does at 0x0801a0d6. This single line is the whole 44/44 body.
    u32 idx = 280;
    // Closure spellings, chosen as the twin that is BOTH a real closure label
    // and a real C body (src/ghost.c and src/runtime_state_dispatch.c respectively).
    // A bare `extern` for a name nothing in src/ defines passes the byte oracle
    // and silently reaches the ROM body instead of C; the export/call audits
    // catch exactly that. The friendly names live only on the host.
    u32 flag = _08018ACC(32);
    int sel = flag?2:1;
    u16 v = *(volatile u16 *)(racers + 26);
    u32 res = (u32)(uintptr_t)sub_080263C4(0, (void *)(uintptr_t)v, sel);
    *(volatile u32 *)(racers + idx) = res;
}
void Race_Cluster_D(void) {
    register u8 *racers __asm__("r4") = (u8 *)RACER_ARRAY_BASE;
    // The phase cell is base+pool 0x10FC, not one folded address (ROM
    // `ldr r0,=WA / ldr r1,=0x10FC / adds`). The barrier sits on the base
    // so the offset pool-loads second, in ROM order.
    u32 wabase = WORK_AREA_BASE;
    __asm__("" : "+r"(wabase));
    u8 *cell = (u8 *)(wabase + 0x10FCu);
    int ph = (int)*(u16 *)cell;
    u32 v;
    u32 a0;
    u32 a2;
    // ARGS (measured): both call args are set per-path (`movs r0,#0` lives
    // in each branch, not hoisted to the join), and the call result feeds
    // the store straight from r0 (no `adds r1,r0` copy). The two `a0 = 0`
    // assignments are textually identical so the compiler merges them into
    // one join movs; the barrier makes taken's opaque and unmergeable.
    if (ph == 10) {
        v = (u32)*(u16 *)(racers + 26);
        a0 = 0;
        __asm__("" : "+r"(a0));
        a2 = 0;
    } else {
        (void)_08018ACC(32);
        v = (u32)*(u16 *)(racers + 26);
        a0 = 0;
        a2 = 1;
    }
    u32 result = (u32)(uintptr_t)sub_080263C4((int)a0, (void *)v, (int)a2);
    // The call clobbers r2, so rematerialize the record offset after it.
    register u32 idx __asm__("r2") = 280;
    __asm__("" : "+r"(idx));
    register u32 *target __asm__("r1") = (u32 *)(racers + idx);
    *target = result;
}
// _0801A0E8 occupies 68 ROM bytes: 66 instruction bytes followed by a zero
// halfword. Emit that in its function section instead of gas's Thumb nop fill.
__asm__(".align 2, 0");

__attribute__((naked)) void Race_Cluster_Dispatch(void) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "ldr r0, .L1a12c_158\n"
        "ldr r1, .L1a12c_15c\n"
        "bl 0x0802620C\n"
        "bl 0x08026220\n"
        "ldr r0, .L1a12c_160\n"
        "ldr r1, .L1a12c_164\n"
        "adds r0, r0, r1\n"
        "ldrh r0, [r0, #0]\n"
        "subs r0, #1\n"
        "lsls r0, r0, #16\n"
        "asrs r0, r0, #16\n"
        "cmp r0, #7\n"
        "bhi .L1a12c_19e\n"
        "lsls r0, r0, #2\n"
        "ldr r1, .L1a12c_168\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "mov pc, r0\n"
        "movs r0, r0\n"
        ".L1a12c_158: .4byte 0x03004E30\n"
        ".L1a12c_15c: .4byte 0x02010000\n"
        ".L1a12c_160: .4byte 0x03001780\n"
        ".L1a12c_164: .4byte 0x000010FC\n"
        ".L1a12c_168: .4byte 0x0801A16C\n"
        ".L1a12c_16c: .4byte 0x0801A18C\n"
        ".L1a12c_170: .4byte 0x0801A19E\n"
        ".L1a12c_174: .4byte 0x0801A192\n"
        ".L1a12c_178: .4byte 0x0801A18C\n"
        ".L1a12c_17c: .4byte 0x0801A198\n"
        ".L1a12c_180: .4byte 0x0801A19E\n"
        ".L1a12c_184: .4byte 0x0801A19E\n"
        ".L1a12c_188: .4byte 0x0801A192\n"
        ".L1a12c_18c:\n"
        "bl 0x0801A008\n"
        "b .L1a12c_1a2\n"
        ".L1a12c_192:\n"
        "bl 0x0801A070\n"
        "b .L1a12c_1a2\n"
        ".L1a12c_198:\n"
        "bl 0x0801A0BC\n"
        "b .L1a12c_1a2\n"
        ".L1a12c_19e:\n"
        "bl 0x0801A0E8\n"
        ".L1a12c_1a2:\n"
        "movs r4, #0\n"
        "ldr r2, .L1a12c_1f0\n"
        "ldr r3, .L1a12c_1f4\n"
        "adds r1, r2, r3\n"
        "movs r3, #0\n"
        "ldrsh r0, [r1, r3]\n"
        "cmp r4, r0\n"
        "bge .L1a12c_1ea\n"
        "adds r7, r2, #0\n"
        "adds r6, r1, #0\n"
        "movs r5, #0\n"
        ".L1a12c_1b8:\n"
        "ldr r0, .L1a12c_1f8\n"
        "adds r0, r5, r0\n"
        "ldrh r0, [r0, #26]\n"
        "lsls r1, r4, #1\n"
        "ldr r3, .L1a12c_1fc\n"
        "adds r2, r7, r3\n"
        "adds r1, r1, r2\n"
        "movs r2, #0\n"
        "ldrsh r1, [r1, r2]\n"
        "bl 0x08026520\n"
        "lsls r1, r4, #5\n"
        "ldr r2, .L1a12c_200\n"
        "adds r1, r1, r2\n"
        "movs r2, #8\n"
        "bl 0x0802D970\n"
        "movs r3, #142\n"
        "lsls r3, r3, #1\n"
        "adds r5, r5, r3\n"
        "adds r4, #1\n"
        "movs r1, #0\n"
        "ldrsh r0, [r6, r1]\n"
        "cmp r4, r0\n"
        "blt .L1a12c_1b8\n"
        ".L1a12c_1ea:\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".L1a12c_1f0: .4byte 0x03001780\n"
        ".L1a12c_1f4: .4byte 0x000010CA\n"
        ".L1a12c_1f8: .4byte 0x03004E80\n"
        ".L1a12c_1fc: .4byte 0x00001DE4\n"
        ".L1a12c_200: .4byte 0x03005780\n"
        ".syntax divided\n"
    );
}

void _0801A008(void) __attribute__((alias("Race_Cluster_A")));
void _0801A070(void) __attribute__((alias("Race_Cluster_B")));
void _0801A0BC(void) __attribute__((alias("Race_Cluster_C")));
// The manifest entry for 0x0801a0bc exports `sub_0801A0BC` (asm spells the
// body that way), and the asm branches to it by name -- but nothing in src/
// bound that spelling, so there was no.thumb_set for _export_missing to
// re-emit and the link failed on an undefined reference.
void sub_0801A0BC(void) __attribute__((alias("Race_Cluster_C")));
void _0801A0E8(void) __attribute__((alias("Race_Cluster_D")));
void sub_0801A0E8(void) __attribute__((alias("Race_Cluster_D")));
void _0801A12C(void) __attribute__((alias("Race_Cluster_Dispatch")));

// ROM entry alias.
void RaceScene_Dispatch_A12C(void) __attribute__((alias("Race_Cluster_Dispatch")));
