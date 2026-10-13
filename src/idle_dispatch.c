#include "gtadv/idle.h"
#include "gtadv/state_block_a.h"
#include "gtadv/memory.h"
#include "gba/regs.h"
#include "gba/bios.h"

// Idle dispatcher + handlers reference: asm/idle.s 0x08001988-0x080020E8
// without duplicating idle_accessors.c (which owns the pure accessors).
// Behavioral equivalence, not byte-matching.

extern void CpuSet_2D974(const void *src, void *dst, unsigned ctrl); // _08002D974 CpuSet (swi 0x0B)
extern u32  DivRemU_02DFE4(u32 num, u32 den); // sub_0802DFE4 remainder (r0)
extern u32  sub_0802DF6C(u32 n, u32 d); // unsigned quotient (r0)
extern void Warn(u32 a, u32 b); // sub_0800295C
extern void _0800295C(u32 a, u32 b);
extern u32  SessionReadWord(u32 off); // _0800070C
extern void SessionPump(void); // _080006A4
extern void PacketBuild(void *a, u32 b); // _08000848
extern void _08000848(void *a, u32 b);
// sub_08000CA0 (block-B special-mode state stepper) now lifted in runtime_state_dispatch.c
// (SessionStep_CA0); idle mode-8/11 path binds the strong C alias.
extern int  SubCA0(u32 a); // sub_08000CA0

static inline volatile u8 *st8(void) { return (volatile u8 *)(uintptr_t)*(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR; }
static inline volatile u16 *st16(int off) { return (volatile u16 *)(st8()+off); }
static inline volatile u32 *st32(int off) { return (volatile u32 *)(st8()+off); }

// Forward decls of helpers owned by other files
u16  StateA_GetRecordHalfwordInverted(void);
// The closure spells these two `_0800…`; call them by that spelling so the
// spliced body resolves without a veneer. Declared here as well as by their
// friendly names above, which other bodies in this file still use.
u16  _08001680(void);
void _080016D0(u32 v);
u16  StateA_GetIndexedHalfwordInverted(int idx);
void *StateA_GetRecordPtrInverted(void);
void StateA_SetPendingParam(u32 v);
bool StateA_IsGameMode(int mode);
bool StateA_IsSpecialMode(u32 mode);

// _08001988 — mode 3 handler
__attribute__((naked)) void Idle_Mode3Handler(void)
{
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, lr}\n"
        "bl 0x08001680\n"
        "lsls r0, r0, #16\n"
        "ldr r1, .L1988_p0\n"
        "cmp r0, r1\n"
        "bne .L1988_increment\n"
        "movs r0, #0\n"
        "bl 0x08001698\n"
        "lsls r0, r0, #16\n"
        "lsrs r1, r0, #16\n"
        "ldr r5, .L1988_p1\n"
        "ldr r4, [r5, #0]\n"
        "movs r2, #26\n"
        "ldrsh r0, [r4, r2]\n"
        "cmp r1, r0\n"
        "bne .L1988_increment\n"
        "cmp r1, #0\n"
        "bne .L1988_nonzero\n"
        "movs r0, #1\n"
        "bl 0x08001698\n"
        "ldr r1, [r5, #0]\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "str r0, [r1, #44]\n"
        "movs r0, #2\n"
        "bl 0x08001698\n"
        "ldr r1, [r5, #0]\n"
        "strh r0, [r1, #22]\n"
        "lsls r0, r0, #16\n"
        "cmp r0, #0\n"
        "bne .L1988_post_copy\n"
        "ldr r0, .L1988_p2\n"
        "movs r1, #0\n"
        "bl 0x0800295c\n"
        "b .L1988_post_copy\n"
        ".align 2, 0\n"
        ".L1988_p0: .4byte 0x03ed0000\n"
        ".L1988_p1: .4byte 0x030000e4\n"
        ".L1988_p2: .4byte 0x0802e1d0\n"
        ".L1988_nonzero:\n"
        "movs r2, #22\n"
        "ldrsh r0, [r4, r2]\n"
        "cmp r1, r0\n"
        "bne .L1988_default_div\n"
        "ldr r0, [r4, #44]\n"
        "movs r1, #12\n"
        "bl 0x0802dfe4\n"
        "adds r5, r0, #0\n"
        "cmp r5, #0\n"
        "bne .L1988_have_div\n"
        "movs r5, #12\n"
        ".L1988_have_div:\n"
        "movs r0, #5\n"
        "strh r0, [r4, #18]\n"
        "b .L1988_after_div\n"
        ".L1988_default_div:\n"
        "movs r5, #12\n"
        ".L1988_after_div:\n"
        "bl 0x080016b8\n"
        "adds r0, #4\n"
        "ldr r4, .L1988_p3\n"
        "ldr r1, [r4, #0]\n"
        "ldr r1, [r1, #32]\n"
        "lsrs r2, r5, #31\n"
        "adds r2, r5, r2\n"
        "lsls r2, r2, #10\n"
        "lsrs r2, r2, #11\n"
        "bl 0x0802d974\n"
        "ldr r2, [r4, #0]\n"
        "lsrs r1, r5, #1\n"
        "lsls r1, r1, #1\n"
        "ldr r0, [r2, #32]\n"
        "adds r0, r0, r1\n"
        "str r0, [r2, #32]\n"
        ".L1988_post_copy:\n"
        "ldr r0, .L1988_p3\n"
        "ldr r1, [r0, #0]\n"
        "ldrh r0, [r1, #26]\n"
        "adds r0, #1\n"
        "strh r0, [r1, #26]\n"
        ".L1988_increment:\n"
        "movs r0, #252\n"
        "lsls r0, r0, #2\n"
        "bl 0x080016d0\n"
        "ldr r4, .L1988_p3\n"
        "ldr r0, [r4, #0]\n"
        "ldrh r1, [r0, #26]\n"
        "movs r0, #0\n"
        "bl 0x08001e48\n"
        "ldr r4, [r4, #0]\n"
        "movs r1, #26\n"
        "ldrsh r0, [r4, r1]\n"
        "movs r2, #22\n"
        "ldrsh r1, [r4, r2]\n"
        "cmp r0, r1\n"
        "ble .L1988_store_result\n"
        "adds r0, r1, #0\n"
        ".L1988_store_result:\n"
        "adds r4, #208\n"
        "lsls r0, r0, #12\n"
        "bl 0x0802de04\n"
        "str r0, [r4, #0]\n"
        "cmp r0, #0\n"
        "bge .L1988_done\n"
        "movs r0, #0\n"
        "str r0, [r4, #0]\n"
        ".L1988_done:\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".L1988_p3: .4byte 0x030000e4\n"
        ".syntax divided\n"
    );
}
void _08001988(void) __attribute__((alias("Idle_Mode3Handler")));

// _08001A74 — mode 4 countdown
void Idle_Mode4Handler(void) {
    u8 *base;
    register u16 uv __asm__("r2");
    s16 sv;
    _080016D0(0x03EE);
    base = (u8 *)*(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    uv = *(volatile u16 *)(base + 0x14);
    sv = *(s16 *)(base + 0x14);
    if (sv > 0) {
        register u16 tmp __asm__("r0");
        tmp = uv - 1;
        *(volatile u16 *)(base + 0x14) = tmp;
    } else {
        *(volatile u16 *)(base + 0x12) = 1;
    }
}
void _08001A74(void) __attribute__((alias("Idle_Mode4Handler")));

// _08001AA0 — mode5
void Idle_Mode5Handler(void) {
    if (_08001680() == 0x03EE) *st16(0x12)=1;
    _080016D0(0x03F1);
}
void _08001AA0(void) __attribute__((alias("Idle_Mode5Handler")));

// _08001ACC — mode6
void Idle_Mode6Handler(void) {
    register int matches __asm__("r5") = 1;
    register int i __asm__("r4") = 1;
    register volatile u32 *slot __asm__("r0") =
        (volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    register volatile u8 *initial_base __asm__("r1") =
        (volatile u8 *)(uintptr_t)*slot;
    register volatile u32 *saved_slot __asm__("r6") = slot;
    register u16 count __asm__("r1") =
        *(volatile u16 *)(initial_base + 0x0A);

    // The assembly rereads the record count after each record lookup and
    // again before the final comparison; callers can update this shared block.
    if (matches < count) {
        do {
            register u32 record __asm__("r0") = StateA_GetRecordHalfword(i);
            register u32 expected __asm__("r1") = 0xFD;
            record <<= 16;
            record >>= 16;
            expected <<= 2;
            if (record == expected)
                matches++;
            i++;
            u16 current_count = *(volatile u16 *)((volatile u8 *)(uintptr_t)
                *(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR + 0x0A);
            if (i >= current_count)
                break;
        } while (1);
    }

    register volatile u8 *base __asm__("r1") =
        (volatile u8 *)(uintptr_t)*saved_slot;
    u16 final_count = *(volatile u16 *)(base + 0x0A);
    if (matches == final_count) {
        *(volatile u16 *)(base + 0x12) = 7;
        *(volatile u16 *)(base + 0x14) = 3;
    }
    StateA_SetPendingParam(0x03F2);
}
void _08001ACC(void) __attribute__((alias("Idle_Mode6Handler")));

// _08001B20 — mode7
void Idle_Mode7Handler(void) {
    u8 *base;
    register u16 uv __asm__("r2");
    s16 sv;
    base = (u8 *)*(volatile u32 *)STATE_BLOCK_A_SLOT_ADDR;
    uv = *(volatile u16 *)(base + 0x14);
    sv = *(s16 *)(base + 0x14);
    if (sv > 0) {
        register u16 tmp __asm__("r0");
        tmp = uv - 1;
        *(volatile u16 *)(base + 0x14) = tmp;
    } else {
        *(volatile u16 *)(base + 0x12) = 8;
    }
    _080016D0(0x03F3);
}
void _08001B20(void) __attribute__((alias("Idle_Mode7Handler")));

// _08001B50 — mode10
void Idle_Mode10Handler(void) {
    if (StateA_GetRecordHalfwordInverted()==0x03F3) *st16(0x12)=11;
    _080016D0(0x03F4);
}
void _08001B50(void) __attribute__((alias("Idle_Mode10Handler")));

// _08001B7C — per-frame mode dispatcher
__attribute__((naked)) void Idle_Dispatcher(void)
{
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, lr}\n"
        "sub sp, #4\n"
        "ldr r4, .L1b7c_p0\n"
        "ldr r0, [r4, #0]\n"
        "ldrh r0, [r0, #16]\n"
        "bl 0x08001744\n"
        "cmp r0, #0\n"
        "beq.n .L1b7c_refresh\n"
        "ldr r0, [r4, #0]\n"
        "adds r0, #208\n"
        "bl 0x08000CA0\n"
        "cmp r0, #0\n"
        "beq.n .L1b7c_refresh\n"
        "ldr r1, [r4, #0]\n"
        "movs r0, #1\n"
        "strh r0, [r1, #18]\n"
        ".L1b7c_refresh:\n"
        "ldr r4, .L1b7c_p0\n"
        "ldr r1, [r4, #0]\n"
        "adds r0, r1, #0\n"
        "adds r0, #48\n"
        "adds r1, #112\n"
        "ldr r2, .L1b7c_p1\n"
        "bl 0x0802D974\n"
        "movs r0, #0\n"
        "str r0, [sp, #0]\n"
        "ldr r1, [r4, #0]\n"
        "adds r1, #48\n"
        "ldr r2, .L1b7c_p2\n"
        "mov r0, sp\n"
        "bl 0x0802D974\n"
        "ldr r0, [r4, #0]\n"
        "ldrh r0, [r0, #16]\n"
        "subs r0, #1\n"
        "cmp r0, #9\n"
        "bhi.n .L1b7c_done\n"
        "lsls r0, r0, #2\n"
        "ldr r1, .L1b7c_p3\n"
        "adds r0, r0, r1\n"
        "ldr r0, [r0, #0]\n"
        "mov pc, r0\n"
        ".align 2, 0\n"
        ".L1b7c_p0: .4byte 0x030000E4\n"
        ".L1b7c_p1: .4byte 0x04000010\n"
        ".L1b7c_p2: .4byte 0x05000010\n"
        ".L1b7c_p3: .4byte 0x08001BE4\n"
        ".4byte 0x08001C0C\n"
        ".4byte 0x08001C18\n"
        ".4byte 0x08001C1E\n"
        ".4byte 0x08001C24\n"
        ".4byte 0x08001C2A\n"
        ".4byte 0x08001C30\n"
        ".4byte 0x08001C36\n"
        ".4byte 0x08001C40\n"
        ".4byte 0x08001C40\n"
        ".4byte 0x08001C3C\n"
        ".L1b7c_case0:\n"
        "ldr r0, .L1b7c_case0_pool\n"
        "bl 0x080016D0\n"
        "b.n .L1b7c_done\n"
        ".align 2, 0\n"
        ".L1b7c_case0_pool: .4byte 0x000003EB\n"
        ".L1b7c_case1:\n"
        "bl 0x08001854\n"
        "b.n .L1b7c_done\n"
        ".L1b7c_case2:\n"
        "bl 0x08001988\n"
        "b.n .L1b7c_done\n"
        ".L1b7c_case3:\n"
        "bl 0x08001A74\n"
        "b.n .L1b7c_done\n"
        ".L1b7c_case4:\n"
        "bl 0x08001AA0\n"
        "b.n .L1b7c_done\n"
        ".L1b7c_case5:\n"
        "bl 0x08001ACC\n"
        "b.n .L1b7c_done\n"
        ".L1b7c_case6:\n"
        "bl 0x08001B20\n"
        "b.n .L1b7c_done\n"
        ".L1b7c_case10:\n"
        "bl 0x08001B50\n"
        ".L1b7c_done:\n"
        "add sp, #4\n"
        "pop {r4}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".syntax divided\n"
    );
}
void _08001B7C(void) __attribute__((alias("Idle_Dispatcher")));

// _08001C48 — idle work-area refresh
void Idle_RefreshWorkArea(void) {
    register volatile u32 *slot __asm__("r4") =
        (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
    volatile u8 *first = (volatile u8 *)(uintptr_t)*slot;
    // asm _08001C48: r0=st+0xC0 (src), r1=st+0xB0 (dst) — C0->B0, not B0->C0.
    CpuSet((const void*)(first+0xC0), (void*)(first+0xB0), 0x04000004);
    volatile u8 *st = (volatile u8 *)(uintptr_t)*slot;
    register u16 rawCount __asm__("r0") = *(volatile u16 *)(st+8);
    register u32 countOff __asm__("r3") = 8;
    register s32 signedCount __asm__("r2");
    __asm__ volatile ("ldrsh %0, [%1, %2]"
                      : "=r" (signedCount) : "r" (st), "r" (countOff));
    if (signedCount==0) {
        u32 z=(u32)signedCount; CpuSet(&z, (void*)(st+0xC0), 0x05000004);
    } else {
        *(volatile u16 *)(st+8) = rawCount-1;
    }
    slot = (volatile u32 *)(uintptr_t)STATE_BLOCK_A_SLOT_ADDR;
    volatile u8 *modeState = (volatile u8 *)(uintptr_t)*slot;
    u16 cur = *(volatile u16 *)(modeState+0x10);
    if (StateA_IsGameMode(cur)) {
        register volatile u8 *state __asm__("r0") =
            (volatile u8 *)(uintptr_t)*slot;
        register u16 p __asm__("r2") = *(volatile u16 *)(state+4);
        *(volatile u16*)(state+0xC0) = p;
        _08000848((void*)(state+0xB0), 0);
    }
}
void _08001C48(void) __attribute__((alias("Idle_RefreshWorkArea")));

// _08001CD8 — input/ticker
__attribute__((naked)) void Idle_Ticker(void) {
    __asm__ volatile (
        ".syntax unified\n"
        ".Lidle1cd8_cd8:\n"
        "push {r4, r5, lr}\n"
        "ldr r4, .Lidle1cd8_d5c\n"
        "ldr r1, [r4]\n"
        "movs r0, #0\n"
        "strh r0, [r1, #0xc]\n"
        "ldrh r0, [r1, #0x10]\n"
        "bl _08001730\n"
        "cmp r0, #0\n"
        "bne.n .Lidle1cd8_cee\n"
        "b.n .Lidle1cd8_e0a\n"
        ".Lidle1cd8_cee:\n"
        "ldr r0, [r4]\n"
        "adds r0, #0x30\n"
        "bl _0800070C\n"
        "ldr r1, [r4]\n"
        "adds r1, #0xd8\n"
        "str r0, [r1]\n"
        "bl _080006A4\n"
        "ldr r1, [r4]\n"
        "ldrb r0, [r1, #2]\n"
        "cmp r0, #0\n"
        "beq.n .Lidle1cd8_d66\n"
        "movs r3, #0\n"
        "adds r0, r1, #0\n"
        "adds r0, #0xd8\n"
        "ldr r2, [r0]\n"
        "movs r0, #0xc0\n"
        "lsls r0, r0, #6\n"
        "ands r0, r2\n"
        "cmp r0, #0\n"
        "bne.n .Lidle1cd8_d36\n"
        "movs r0, #0x80\n"
        "lsls r0, r0, #8\n"
        "ands r0, r2\n"
        "cmp r0, #0\n"
        "beq.n .Lidle1cd8_d32\n"
        "lsls r1, r2, #28\n"
        "lsrs r1, r1, #28\n"
        "lsls r0, r2, #20\n"
        "lsrs r0, r0, #28\n"
        "cmp r1, r0\n"
        "beq.n .Lidle1cd8_d32\n"
        "movs r3, #1\n"
        ".Lidle1cd8_d32:\n"
        "cmp r3, #0\n"
        "beq.n .Lidle1cd8_d60\n"
        ".Lidle1cd8_d36:\n"
        "ldr r0, .Lidle1cd8_d5c\n"
        "ldr r1, [r0]\n"
        "ldrh r0, [r1, #0xe]\n"
        "adds r0, #1\n"
        "strh r0, [r1, #0xe]\n"
        "lsls r0, r0, #16\n"
        "lsrs r0, r0, #16\n"
        "cmp r0, #60\n"
        "bls.n .Lidle1cd8_d66\n"
        "movs r0, #0\n"
        "strh r0, [r1, #0xe]\n"
        "adds r0, r1, #0\n"
        "adds r0, #0xd8\n"
        "ldr r1, [r0]\n"
        "movs r0, #21\n"
        "movs r2, #0\n"
        "bl sub_08004D4C\n"
        "b.n .Lidle1cd8_d66\n"
        ".align 2, 0\n"
        ".Lidle1cd8_d5c: .word 0x030000E4\n"
        ".Lidle1cd8_d60:\n"
        "ldr r0, .Lidle1cd8_e10\n"
        "ldr r0, [r0]\n"
        "strh r3, [r0, #0xe]\n"
        ".Lidle1cd8_d66:\n"
        "ldr r4, .Lidle1cd8_e10\n"
        "ldr r1, [r4]\n"
        "adds r0, r1, #0\n"
        "adds r0, #0xd8\n"
        "ldr r0, [r0]\n"
        "movs r5, #0x80\n"
        "ands r0, r5\n"
        "cmp r0, #0\n"
        "beq.n .Lidle1cd8_d82\n"
        "ldrb r0, [r1, #1]\n"
        "cmp r0, #0\n"
        "bne.n .Lidle1cd8_db0\n"
        "bl _080006A4\n"
        ".Lidle1cd8_d82:\n"
        "ldr r1, [r4]\n"
        "ldrb r0, [r1, #1]\n"
        "cmp r0, #0\n"
        "bne.n .Lidle1cd8_db0\n"
        "adds r0, r1, #0\n"
        "adds r0, #0xd8\n"
        "ldr r0, [r0]\n"
        "ands r0, r5\n"
        "cmp r0, #0\n"
        "beq.n .Lidle1cd8_d9a\n"
        "bl _080006A4\n"
        ".Lidle1cd8_d9a:\n"
        "ldr r2, [r4]\n"
        "adds r0, r2, #0\n"
        "adds r0, #0xd8\n"
        "ldr r0, [r0]\n"
        "movs r1, #0x80\n"
        "lsls r1, r1, #1\n"
        "ands r0, r1\n"
        "cmp r0, #0\n"
        "beq.n .Lidle1cd8_db0\n"
        "movs r0, #1\n"
        "strb r0, [r2, #1]\n"
        ".Lidle1cd8_db0:\n"
        "ldr r3, .Lidle1cd8_e10\n"
        "ldr r2, [r3]\n"
        "adds r0, r2, #0\n"
        "adds r0, #0xd8\n"
        "ldr r0, [r0]\n"
        "movs r1, #1\n"
        "ands r0, r1\n"
        "cmp r0, #0\n"
        "beq.n .Lidle1cd8_dc8\n"
        "ldrh r0, [r2, #0xc]\n"
        "adds r0, #1\n"
        "strh r0, [r2, #0xc]\n"
        ".Lidle1cd8_dc8:\n"
        "ldr r2, [r3]\n"
        "adds r0, r2, #0\n"
        "adds r0, #0xd8\n"
        "ldr r0, [r0]\n"
        "movs r1, #2\n"
        "ands r0, r1\n"
        "cmp r0, #0\n"
        "beq.n .Lidle1cd8_dde\n"
        "ldrh r0, [r2, #0xc]\n"
        "adds r0, #1\n"
        "strh r0, [r2, #0xc]\n"
        ".Lidle1cd8_dde:\n"
        "ldr r2, [r3]\n"
        "adds r0, r2, #0\n"
        "adds r0, #0xd8\n"
        "ldr r0, [r0]\n"
        "movs r1, #4\n"
        "ands r0, r1\n"
        "cmp r0, #0\n"
        "beq.n .Lidle1cd8_df4\n"
        "ldrh r0, [r2, #0xc]\n"
        "adds r0, #1\n"
        "strh r0, [r2, #0xc]\n"
        ".Lidle1cd8_df4:\n"
        "ldr r2, [r3]\n"
        "adds r0, r2, #0\n"
        "adds r0, #0xd8\n"
        "ldr r0, [r0]\n"
        "movs r1, #8\n"
        "ands r0, r1\n"
        "cmp r0, #0\n"
        "beq.n .Lidle1cd8_e0a\n"
        "ldrh r0, [r2, #0xc]\n"
        "adds r0, #1\n"
        "strh r0, [r2, #0xc]\n"
        ".Lidle1cd8_e0a:\n"
        "pop {r4, r5}\n"
        "pop {r0}\n"
        "bx r0\n"
        ".align 2, 0\n"
        ".Lidle1cd8_e10: .word 0x030000E4\n"
        ".syntax divided\n"
    );
}

void _08001CD8(void) __attribute__((alias("Idle_Ticker")));

// Remaining small thunks/accessors not in idle_accessors.c
// _08001E14 — 28 B with TWO pool words. asm/idle.s:654-669 is
//   ldr r2,=0x0802E190 / lsls r0,#2 / ldr r1,=0x030000E4 / ldr r1,[r1] /
//   ldrh r1,[r1,#6] / lsls r1,#4 / adds r0,r0,r1 / adds r0,r0,r2 /
//   ldr r0,[r0] / bx lr.
// agbcc materialises each initialiser where its declaration stands, so both
// intermediates are NAMED locals declared in the ROM's order: the table
// pointer first, then the index shift, then the halfword load.
u32 Idle_GetDescriptorWord(int idx) {
    extern u8 RecDescTbl[] __asm__("RecDescTbl");
    __asm__(".globl RecDescTbl\nRecDescTbl = 0x0802E190\n");
    u32 tbl = (u32)(uintptr_t)RecDescTbl;
    u32 off = (u32)idx << 2;
    u16 sel = *st16(6);
    u32 sum = off + ((u32)sel << 4);
    return *(u32 *)((uintptr_t)sum + (uintptr_t)tbl);
}
u32 _08001E14(int a) __attribute__((alias("Idle_GetDescriptorWord")));
// _08001E30 -> int — 1 when block A +6 == 0 (normal select active).
// Word-typed return, and the `int r = 0` must be a NAMED local declared BEFORE
// the load: agbcc materialises each initialiser where its declaration stands,
// so an unnamed `return x == 0;` sinks `movs r1,#0` behind the two pool loads
// (candidate `ldr r0,[pc,#20]; ldr r0,[r0]; movs r1,#0; ldrh...`). With the
// local first the ROM's own order falls out: `movs r1,#0` at 0x08001E30, the
// pool load at 0x08001E32.
//
// NOT byte-exact, and the two missing halfwords are not a C-shape problem.
// asm/idle.s:673-686 is 24 B with no frame (`adds r0,r1,#0; bx lr` at 0x1e3e);
// this body compiles to the identical ten instructions plus `push {lr}` at
// 0x1e30 and `pop {r1}; bx r1` instead of the bare `bx lr` — 28 B, and
// `push {lr}`/`pop {r1}` is an aligned 2-byte insert so even the 10 real
// instructions sit at shifted addresses. The frame comes from agbcc's Thumb
// prologue policy, not from the C: `thumb_function_prologue` pushes lr when
// `live_regs_mask || !leaf_function_p || far_jump_used_p`
// (gcc_arm/config/arm/thumb.c:1134) and `output_return` falls to
// `thumb_exit(-1)` → `pop {r1}; bx r1` (thumb.c:1025-1034). Measured across
// ~60 leaf shapes (int/u32/long/short/char/unsigned/pointer returns; if/else,
// ?:, switch, do-while, goto, two-armed and three-armed accumulators; named
// and unnamed initialisers; `register`): EVERY one that keeps a conditional
// branch gets `push {lr}` + `pop {r1}; bx r1`, and every one that drops the
// branch is frameless. A branchless rewrite (`neg/orr/lsr #31`, or
// `mov r0,#1`) is frameless but does not reproduce the ROM's
// `movs r1,#0 … movs r1,#1 … adds r0,r1,#0` either, so there is no C that
// yields these 24 bytes. Consistent with the rest of the backlog: the ROM has
// several frameless branchy leaves (asm/carphys_racer_tail.s:983-999 at
// 0x080022438, asm/garage_26f50.s:1720 and :4567) and none is promoted;
// src/race_phase_vms.c:565 records the same wall for 0x08002242C.
// It is not a flag choice either: the same body frames at -O0/-O1/-O2/-O3/-Os
// and with or without -mthumb-interwork (only the epilogue spelling changes:
// `pop {pc}` instead of `pop {r1}; bx r1` when interwork is off, and
// -fno-jump-tables does not help).
// `bool` was also wrong independently: as a u8 return it gives the same
// frame, and the ROM leaves the flag in r0 as a word.
int Idle_IsSelZero(void) {
    int r = 0;
    if (*st16(6) == 0) r = 1;
    return r;
}
int _08001E30(void) __attribute__((alias("Idle_IsSelZero")));
// _08001EB8(v) — `v-1` into block A +8. The ROM decrements the ARGUMENT
// (`subs r0,#1`, asm/idle.s:774), not a re-read of the cell, so the word-typed
// parameter replaces the old reload-and-decrement; the store is `strh`, so the
// upper bits of the argument are dropped exactly as the ROM drops them.
void Idle_DecScratch08(u32 v) { *st16(8) = v - 1; }
void _08001EB8(u32 v) __attribute__((alias("Idle_DecScratch08")));
void Idle_SetupMode2(u32 a, u32 b); // _08001EC8 etc. — stub behavioral
void _08001EC8(u32 a,u32 b) __attribute__((alias("Idle_SetupMode2")));
void Idle_SetupMode2(u32 a, u32 b) {
    u32 *slot = (u32 *)STATE_BLOCK_A_SLOT_ADDR;
    u8 *base = (u8 *)*slot;
    u32 savea = a;
    u32 zero = 0;
    u32 q;
    *(volatile u16 *)(base + 0x12) = 2;
    *(volatile u32 *)(base + 0x2C) = b;
    q = sub_0802DF6C(b + 11, 12);
    *(volatile u16 *)(base + 0x16) = (u16)q;
    if ((q << 16) == 0)
        _0800295C(0x0802E1D0, 0);
    {
        u8 *p = (u8 *)*slot;
        *(volatile u16 *)(p + 0x18) = (u16)zero;
        *(volatile u16 *)(p + 0x1C) = (u16)zero;
        *(volatile u32 *)(p + 0x24) = savea;
    }
}
// _08001F08 — one cached state-block pointer, and the zero materialised
void Idle_SetupMode3(u32 a) {
    volatile u8 *p = st8();
    u16 zero = 0;
    *(volatile u16 *)(p + 0x12) = 3;
    *(volatile u16 *)(p + 0x1A) = zero;
    { u16 c = 0x80 << 5; *(volatile u16 *)(p + 0x16) = c; }
    *(volatile u32 *)(p + 0x20) = a;
}
void _08001F08(u32 a) __attribute__((alias("Idle_SetupMode3")));
