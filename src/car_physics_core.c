#include "gtadv/car_physics_lane.h"
#include "gba/types.h"
#include "gba/bios.h"

// Lane core: carphys_racer, go_start, grant_delay
// Sources: asm/carphys_racer.s, asm/go_start.s, asm/grant_delay.s, asm/carphys_racer.s

// Forward decls of external helpers (provided by other lanes or ROM)
extern void *_08004B68(void);
extern void sub_0802B1E4(int);
extern void sub_0800DAB8(void *p);
extern void sub_080075E8(void *base,int idx,int n);
extern void sub_0800798C(void *base, void *dst);
extern void sub_08007A58(void *p);
extern void *sub_0800572C(u32 s);  // save.c:813 alias of _0800572C: `void *(u32)`, not void/int
extern int  _0800572C(int);
extern int  _08005758(int);
extern void sub_08007ABC(void *a,int b,int c);
extern void sub_0800279C(int);
extern void sub_0800D77C(void *a,int b,int c);
extern int  sub_08007570(void *base,int v,int n,int m,int k); // 5-arg ROM ABI; decl-only here
extern int  sub_08025CF4(int,int,int);
extern int  _080025CF4(int,int,int);    // _08007770 true arity 6 (2 stack words); the two car-id sites pass {4, r9=1}.
    extern void sub_08007770(int,void*,int,int,int,int);
extern int  sub_08024C3C(int);
extern int  sub_08024D5C(int);
extern int  sub_08024C58(int);
extern int  sub_08025F78(int);
extern int  sub_08025FAC(int);
extern int  sub_08024C90(int);
extern void _08023FF8(int,int);
extern int  sub_08026004(int);
extern int  sub_08025FF0(int);
extern int  sub_080022E4(int);
extern void _0802B368(u16 v);   // closure spelling; was sub_08002B368, a
                               // 9-digit form the screen cannot match

// IWRAM base
#define WA 0x03001780u

// ---- grant_delay : _0800B82C ----
void GrantDelay_Tick(int carId, int delay)
{
    extern u8 GrantDelayWa[];
    register int id __asm__("r5") = carId;
    register volatile u8 *wa __asm__("r6") = (volatile u8 *)GrantDelayWa;
    register u32 offset __asm__("r2") = 0x10F8u;
    register uintptr_t slot __asm__("r4");
    register u32 index __asm__("r0");
    register volatile u8 *store_addr __asm__("r1");

    __asm__(".globl GrantDelayWa\nGrantDelayWa = 0x03001780\n");
    if (*(volatile u32 *)(wa + offset) < (u32)delay)
        return;
    if (((u32)sub_08025FAC(id) << 16) != 0)
        return;

    offset = 0x1058u;
    {
        volatile u16 *flag = (volatile u16 *)(wa + offset);
        __asm__ volatile ("" : : : "memory");
        slot = 1;
        *flag = (u16)slot;
    }
    index = (u32)sub_08024C90(id);
    index <<= 1;
    offset = 131;
    offset <<= 5;
    store_addr = wa + offset;
    index += (u32)(uintptr_t)store_addr;
    *(volatile u16 *)(uintptr_t)index = (u16)slot;

    slot = (uintptr_t)(wa + 0x103Cu);
    {
        register u32 work __asm__("r1") = 0;
        register s32 count __asm__("r0");
        // Keep this non-volatile signed-halfword read into an int accumulator:
        // agbcc then emits the ROM's `movs r1,#0; ldrsh r0,[r4,r1]` directly.
        count = *(s16 *)(uintptr_t)(slot + work);
        offset -= 34;
        work = (u32)(wa + offset);
        count += work;
        *(volatile u8 *)(uintptr_t)count = (u8)id;
    }

    sub_08025F78(id);
    _08023FF8(28, 0);
    *(volatile u16 *)slot = *(volatile u16 *)slot + 1;
}

void _0800B82C(int a0,int a1) __attribute__((alias("GrantDelay_Tick")));
void sub_0800B82C(int a0,int a1) __attribute__((alias("GrantDelay_Tick")));

// ---- go_start : _0800B89C ----
void GoStart_Tick(void) {
    register volatile u8 *wa __asm__("r5") = (volatile u8 *)(uintptr_t)WA;
    register volatile u16 *stage __asm__("r4") = (volatile u16 *)(wa + 0x576);

    if (*stage == 1) {
        GrantDelay_Tick(86, 10);
        GrantDelay_Tick(56, 30);
    }
    if (*stage == 2) {
        GrantDelay_Tick(55, 10);
        GrantDelay_Tick(31, 30);
    }
    if (*stage == 3) {
        GrantDelay_Tick(14, 10);
        GrantDelay_Tick(98, 30);
    }

    if (*stage == 1 && *(volatile u32 *)(wa + 0x10F8) > 19u &&
        (u16)sub_08026004(1) == 0) {
        sub_08025FF0(1);
        volatile s16 *selection = (volatile s16 *)(wa + 0x104A);
        s16 index = *(s16 *)(uintptr_t)selection;
        *(volatile u16 *)(wa + 0x104C + 2 * (s32)index) = 2;
        _08023FF8(30, 0);
        *selection = (u16)(*selection + 1);
    }

    if (*stage == 2 && *(volatile u32 *)(wa + 0x10F8) > 19u &&
        (u16)sub_08026004(2) == 0) {
        sub_08025FF0(2);
        volatile s16 *selection = (volatile s16 *)(wa + 0x104A);
        s16 index = *(s16 *)(uintptr_t)selection;
        *(volatile u16 *)(wa + 0x104C + 2 * (s32)index) = 3;
        _08023FF8(30, 0);
        *selection = (u16)(*selection + 1);
    }
}
void _0800B89C(void) __attribute__((alias("GoStart_Tick")));
void sub_0800B89C(void) __attribute__((alias("GoStart_Tick")));

void CarPhysRacer_MarkScene(void *ctx, void *rec){
    (void)ctx;
    extern void *sub_08004B68(void);
    (void)sub_08004B68();
    *(volatile u16 *)((uintptr_t)rec + 0x54) = 1;
}
void _0802135C(void *a, void *b) __attribute__((alias("CarPhysRacer_MarkScene")));

void CarPhysRacer_Nop(void){}
__asm__(".align 2, 0");
void _08021370(void) __attribute__((alias("CarPhysRacer_Nop")));

void CarPhysRacer_RaceStartLatch(void *ctx,int b,int c){
    // _08021828: r2=c, t = (u16)c -1; if <=1 then latch.
    // The 16-bit truncation is written in the ROM's own shape, not as an
    // arithmetic add of 0xFFFF: `((c<<16) + 0xFFFF0000) >> 16`. Written as
    // `(u16)((u16)c + 0xFFFF)` agbcc instead reassociates to
    // `ldr r0,=0xFFFF0000 / adds r2,r2,r0 / lsls r2,#16 / lsrs r2,#16` -- the
    // same four operations with the constant load hoisted ABOVE the shift
    // (47/56, first diff +0x04). Spelling the shift first reproduces the ROM's
    // `lsls r2,#16 / ldr r0,=0xFFFF0000 / adds r2,r2,r0 / lsrs r2,#16` exactly,
    // and still takes the unsigned `cmp r2,#1; bhi`, which only an unsigned
    // `t` produces. 56/56.
    u16 t = (u16)(((((u32)c) << 16) + 0xFFFF0000u) >> 16);
    if (t > 1) return;
    _0802B368(1);
    *(volatile u32 *)((uintptr_t)ctx + 0xC8) = 1;
    *(volatile u16 *)((uintptr_t)ctx + 0x3C) = 0;
    *(volatile u16 *)((uintptr_t)ctx + 0xA4) = 1;
    *(volatile u16 *)((uintptr_t)ctx + 0x40) = 1;
    (void)b;
}
void _08021828(void *a,int b,int c) __attribute__((alias("CarPhysRacer_RaceStartLatch")));

__attribute__((naked)) void CarPhysRacer_InitFrame(void *ctx) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, sl\n"
        "mov r6, r9\n"
        "mov r5, r8\n"
        "push {r5, r6, r7}\n"
        "sub sp, #36\n"
        "adds r7, r0, #0\n"
        "bl sub_08004B68\n"
        "movs r1, #0\n"
        "ldrsh r0, [r0, r1]\n"
        "cmp r0, #28\n"
        "beq.n .Lcar21374__080213BC\n"
        "cmp r0, #28\n"
        "bgt.n .Lcar21374__0802139E\n"
        "cmp r0, #27\n"
        "beq.n .Lcar21374__080213AE\n"
        "movs r2, #192\n"
        "adds r2, r2, r7\n"
        "mov sl, r2\n"
        "b.n .Lcar21374__080213E8\n"
        ".Lcar21374__0802139E:\n"
        "cmp r0, #29\n"
        "beq.n .Lcar21374__080213CA\n"
        "cmp r0, #30\n"
        "beq.n .Lcar21374__080213D8\n"
        "movs r3, #192\n"
        "adds r3, r3, r7\n"
        "mov sl, r3\n"
        "b.n .Lcar21374__080213E8\n"
        ".Lcar21374__080213AE:\n"
        "movs r0, #66\n"
        "bl sub_0802B1E4\n"
        "adds r1, r7, #0\n"
        "adds r1, #192\n"
        "movs r0, #0\n"
        "b.n .Lcar21374__080213E4\n"
        ".Lcar21374__080213BC:\n"
        "movs r0, #66\n"
        "bl sub_0802B1E4\n"
        "adds r1, r7, #0\n"
        "adds r1, #192\n"
        "movs r0, #1\n"
        "b.n .Lcar21374__080213E4\n"
        ".Lcar21374__080213CA:\n"
        "movs r0, #66\n"
        "bl sub_0802B1E4\n"
        "adds r1, r7, #0\n"
        "adds r1, #192\n"
        "movs r0, #2\n"
        "b.n .Lcar21374__080213E4\n"
        ".Lcar21374__080213D8:\n"
        "movs r0, #66\n"
        "bl sub_0802B1E4\n"
        "adds r1, r7, #0\n"
        "adds r1, #192\n"
        "movs r0, #3\n"
        ".Lcar21374__080213E4:\n"
        "strh r0, [r1, #0]\n"
        "mov sl, r1\n"
        ".Lcar21374__080213E8:\n"
        "ldr r4, .Lcar21374__080215BC\n"
        "ldr r5, .Lcar21374__080215C0\n"
        "adds r0, r4, r5\n"
        "ldrh r0, [r0, #0]\n"
        "adds r6, r7, #0\n"
        "adds r6, #188\n"
        "str r6, [sp, #8]\n"
        "strh r0, [r6, #0]\n"
        "ldr r1, .Lcar21374__080215C4\n"
        "adds r0, r4, r1\n"
        "ldrh r0, [r0, #0]\n"
        "adds r2, r7, #0\n"
        "adds r2, #190\n"
        "str r2, [sp, #12]\n"
        "strh r0, [r2, #0]\n"
        "adds r0, r7, #0\n"
        "adds r0, #128\n"
        "movs r3, #6\n"
        "str r3, [r0, #0]\n"
        "adds r0, #12\n"
        "movs r4, #16\n"
        "str r4, [r0, #0]\n"
        "adds r0, #12\n"
        "movs r5, #0\n"
        "str r5, [r0, #0]\n"
        "movs r6, #56\n"
        "adds r6, r6, r7\n"
        "mov r9, r6\n"
        "mov r0, r9\n"
        "bl sub_0800DAB8\n"
        "ldr r0, .Lcar21374__080215C8\n"
        "movs r1, #2\n"
        "movs r2, #5\n"
        "bl sub_080075E8\n"
        "ldr r0, .Lcar21374__080215CC\n"
        "movs r1, #0\n"
        "movs r2, #6\n"
        "bl sub_080075E8\n"
        "ldr r0, .Lcar21374__080215D0\n"
        "adds r1, r7, #0\n"
        "adds r1, #24\n"
        "bl sub_0800798C\n"
        "movs r0, #7\n"
        "bl sub_0800572C\n"
        "adds r1, r7, #0\n"
        "adds r1, #220\n"
        "str r1, [sp, #16]\n"
        "str r0, [r1, #0]\n"
        "adds r2, r7, #0\n"
        "adds r2, #224\n"
        "str r2, [sp, #20]\n"
        "movs r3, #2\n"
        "str r3, [r2, #0]\n"
        "ldr r0, [r7, #28]\n"
        "ldr r2, [r1, #0]\n"
        "movs r1, #2\n"
        "bl sub_08007ABC\n"
        "ldr r0, .Lcar21374__080215D4\n"
        "adds r1, r7, #0\n"
        "adds r1, #32\n"
        "bl sub_0800798C\n"
        "movs r0, #14\n"
        "bl sub_0800572C\n"
        "movs r4, #244\n"
        "adds r4, r4, r7\n"
        "mov r8, r4\n"
        "str r0, [r4, #0]\n"
        "adds r5, r7, #0\n"
        "adds r5, #248\n"
        "str r5, [sp, #24]\n"
        "movs r6, #5\n"
        "str r6, [r5, #0]\n"
        "ldr r0, [r7, #36]\n"
        "ldr r2, [r4, #0]\n"
        "movs r1, #5\n"
        "bl sub_08007ABC\n"
        "ldr r0, .Lcar21374__080215D8\n"
        "adds r1, r7, #0\n"
        "adds r1, #40\n"
        "bl sub_0800798C\n"
        "movs r0, #7\n"
        "bl sub_0800572C\n"
        "movs r1, #128\n"
        "lsls r1, r1, #1\n"
        "adds r1, r7, r1\n"
        "str r1, [sp, #28]\n"
        "str r0, [r1, #0]\n"
        "movs r2, #130\n"
        "lsls r2, r2, #1\n"
        "adds r2, r7, r2\n"
        "str r2, [sp, #32]\n"
        "movs r0, #50\n"
        "str r0, [r2, #0]\n"
        "ldr r0, [r7, #44]\n"
        "ldr r2, [r1, #0]\n"
        "movs r1, #50\n"
        "bl sub_08007ABC\n"
        "ldr r6, .Lcar21374__080215DC\n"
        "adds r0, r6, #0\n"
        "adds r1, r7, #0\n"
        "bl sub_0800798C\n"
        "adds r0, r7, #0\n"
        "bl sub_08007A58\n"
        "adds r0, r6, #0\n"
        "movs r1, #0\n"
        "movs r2, #3\n"
        "bl sub_080075E8\n"
        "adds r0, r6, #0\n"
        "movs r1, #1\n"
        "movs r2, #4\n"
        "bl sub_080075E8\n"
        "adds r0, r6, #0\n"
        "movs r1, #2\n"
        "movs r2, #7\n"
        "bl sub_080075E8\n"
        "adds r0, r6, #0\n"
        "movs r1, #3\n"
        "movs r2, #10\n"
        "bl sub_080075E8\n"
        "ldr r0, .Lcar21374__080215E0\n"
        "adds r1, r7, #0\n"
        "adds r1, #8\n"
        "bl sub_0800798C\n"
        "movs r0, #65\n"
        "bl sub_0800572C\n"
        "adds r3, r7, #0\n"
        "adds r3, #232\n"
        "str r0, [r3, #0]\n"
        "movs r4, #236\n"
        "adds r4, r4, r7\n"
        "mov ip, r4\n"
        "ldr r2, .Lcar21374__080215E4\n"
        "ldr r5, [sp, #12]\n"
        "movs r1, #0\n"
        "ldrsh r0, [r5, r1]\n"
        "lsls r0, r0, #1\n"
        "ldr r4, [sp, #8]\n"
        "movs r5, #0\n"
        "ldrsh r1, [r4, r5]\n"
        "lsls r1, r1, #3\n"
        "adds r0, r0, r1\n"
        "adds r0, r0, r2\n"
        "movs r2, #0\n"
        "ldrsh r1, [r0, r2]\n"
        "mov r4, ip\n"
        "str r1, [r4, #0]\n"
        "ldr r0, [r7, #12]\n"
        "ldr r2, [r3, #0]\n"
        "bl sub_08007ABC\n"
        "ldr r5, .Lcar21374__080215E8\n"
        "adds r4, r7, #0\n"
        "adds r4, #16\n"
        "adds r0, r5, #0\n"
        "adds r1, r4, #0\n"
        "bl sub_0800798C\n"
        "adds r0, r4, #0\n"
        "bl sub_08007A58\n"
        "adds r0, r5, #0\n"
        "movs r1, #0\n"
        "movs r2, #9\n"
        "bl sub_080075E8\n"
        "adds r0, r7, #0\n"
        "adds r0, #168\n"
        "adds r4, #184\n"
        "str r4, [r0, #0]\n"
        "adds r0, #4\n"
        "mov r5, r9\n"
        "str r5, [r0, #0]\n"
        "movs r0, #0\n"
        "str r0, [r7, #56]\n"
        "movs r1, #1\n"
        "mov r9, r1\n"
        "mov r2, r9\n"
        "strh r2, [r7, #60]\n"
        "adds r0, r7, #0\n"
        "adds r0, #76\n"
        "movs r2, #32\n"
        "negs r2, r2\n"
        "movs r1, #0\n"
        "bl sub_0800D77C\n"
        "adds r0, r7, #0\n"
        "adds r0, #68\n"
        "movs r1, #0\n"
        "movs r2, #160\n"
        "bl sub_0800D77C\n"
        "adds r0, r7, #0\n"
        "adds r0, #64\n"
        "movs r3, #6\n"
        "strh r3, [r0, #0]\n"
        "adds r0, #2\n"
        "movs r5, #5\n"
        "strh r5, [r0, #0]\n"
        "movs r0, #2\n"
        "str r0, [r4, #0]\n"
        "movs r0, #1\n"
        "bl sub_0800279C\n"
        "mov r2, sl\n"
        "movs r3, #0\n"
        "ldrsh r1, [r2, r3]\n"
        "cmp r1, #1\n"
        "beq.n .Lcar21374__08021690\n"
        "cmp r1, #1\n"
        "bgt.n .Lcar21374__080215EC\n"
        "cmp r1, #0\n"
        "beq.n .Lcar21374__080215FA\n"
        "b.n .Lcar21374__08021806\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lcar21374__080215BC: .4byte 0x03001780\n"
        ".Lcar21374__080215C0: .4byte 0x00001046\n"
        ".Lcar21374__080215C4: .4byte 0x00001048\n"
        ".Lcar21374__080215C8: .4byte 0x082A798C\n"
        ".Lcar21374__080215CC: .4byte 0x082C4228\n"
        ".Lcar21374__080215D0: .4byte 0x082C4458\n"
        ".Lcar21374__080215D4: .4byte 0x082C5040\n"
        ".Lcar21374__080215D8: .4byte 0x082D0DC0\n"
        ".Lcar21374__080215DC: .4byte 0x08368B0C\n"
        ".Lcar21374__080215E0: .4byte 0x0836EF7C\n"
        ".Lcar21374__080215E4: .4byte 0x080CBFAC\n"
        ".Lcar21374__080215E8: .4byte 0x08342B60\n"
        ".Lcar21374__080215EC:\n"
        "cmp r1, #2\n"
        "bne.n .Lcar21374__080215F2\n"
        "b.n .Lcar21374__0802174C\n"
        ".Lcar21374__080215F2:\n"
        "cmp r1, #3\n"
        "bne.n .Lcar21374__080215F8\n"
        "b.n .Lcar21374__08021762\n"
        ".Lcar21374__080215F8:\n"
        "b.n .Lcar21374__08021806\n"
        ".Lcar21374__080215FA:\n"
        "movs r0, #4\n"
        "str r0, [sp, #0]\n"
        "mov r4, r9\n"
        "str r4, [sp, #4]\n"
        "movs r0, #0\n"
        "adds r1, r6, #0\n"
        "movs r2, #5\n"
        "movs r3, #0\n"
        "bl sub_08007770\n"
        "movs r0, #16\n"
        "bl sub_08005758\n"
        "ldr r3, .Lcar21374__08021680\n"
        "strh r0, [r3, #2]\n"
        "movs r5, #0\n"
        "strh r5, [r3, #0]\n"
        "ldr r6, [sp, #12]\n"
        "movs r0, #0\n"
        "ldrsh r1, [r6, r0]\n"
        "cmp r1, #0\n"
        "bne.n .Lcar21374__08021628\n"
        "b.n .Lcar21374__08021806\n"
        ".Lcar21374__08021628:\n"
        "cmp r1, #0\n"
        "bge.n .Lcar21374__0802162E\n"
        "b.n .Lcar21374__08021806\n"
        ".Lcar21374__0802162E:\n"
        "cmp r1, #3\n"
        "ble.n .Lcar21374__08021634\n"
        "b.n .Lcar21374__08021806\n"
        ".Lcar21374__08021634:\n"
        "ldr r4, .Lcar21374__08021684\n"
        "ldr r2, .Lcar21374__08021688\n"
        "lsls r1, r1, #1\n"
        "ldr r5, [sp, #8]\n"
        "movs r6, #0\n"
        "ldrsh r0, [r5, r6]\n"
        "lsls r0, r0, #3\n"
        "adds r1, r1, r0\n"
        "adds r1, r1, r2\n"
        "movs r0, #0\n"
        "ldrsh r1, [r1, r0]\n"
        "movs r5, #2\n"
        "ldrsh r2, [r3, r5]\n"
        "movs r6, #16\n"
        "str r6, [sp, #0]\n"
        "adds r0, r4, #0\n"
        "movs r3, #16\n"
        "bl sub_08007570\n"
        "ldr r2, .Lcar21374__0802168C\n"
        "ldr r1, [sp, #12]\n"
        "movs r3, #0\n"
        "ldrsh r0, [r1, r3]\n"
        "lsls r0, r0, #1\n"
        "ldr r5, [sp, #8]\n"
        "movs r6, #0\n"
        "ldrsh r1, [r5, r6]\n"
        "lsls r1, r1, #3\n"
        "adds r0, r0, r1\n"
        "adds r0, r0, r2\n"
        "movs r2, #0\n"
        "ldrsh r1, [r0, r2]\n"
        "adds r0, r4, #0\n"
        "movs r2, #8\n"
        "bl sub_080075E8\n"
        "b.n .Lcar21374__08021806\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lcar21374__08021680: .4byte 0x0203F990\n"
        ".Lcar21374__08021684: .4byte 0x08349754\n"
        ".Lcar21374__08021688: .4byte 0x080CBFEC\n"
        ".Lcar21374__0802168C: .4byte 0x080CC02C\n"
        ".Lcar21374__08021690:\n"
        "movs r0, #4\n"
        "str r0, [sp, #0]\n"
        "mov r3, r9\n"
        "str r3, [sp, #4]\n"
        "movs r0, #0\n"
        "adds r1, r6, #0\n"
        "movs r2, #4\n"
        "movs r3, #0\n"
        "bl sub_08007770\n"
        "ldr r4, .Lcar21374__08021740\n"
        "ldr r5, .Lcar21374__08021744\n"
        "adds r1, r4, r5\n"
        "ldrh r0, [r1, #0]\n"
        "subs r0, #1\n"
        "strh r0, [r1, #0]\n"
        "lsls r0, r0, #16\n"
        "cmp r0, #0\n"
        "bgt.n .Lcar21374__080216BA\n"
        "movs r6, #0\n"
        "strh r6, [r1, #0]\n"
        ".Lcar21374__080216BA:\n"
        "movs r2, #0\n"
        "ldrsh r0, [r1, r2]\n"
        "ldr r3, .Lcar21374__08021740\n"
        "ldr r4, .Lcar21374__08021748\n"
        "adds r1, r3, r4\n"
        "adds r0, r0, r1\n"
        "ldrb r0, [r0, #0]\n"
        "lsls r0, r0, #24\n"
        "asrs r0, r0, #24\n"
        "adds r4, r7, #0\n"
        "adds r4, #182\n"
        "strh r0, [r4, #0]\n"
        "movs r5, #0\n"
        "ldrsh r0, [r4, r5]\n"
        "bl sub_080022E4\n"
        "adds r1, r7, #0\n"
        "adds r1, #184\n"
        "strh r0, [r1, #0]\n"
        "adds r0, r7, #0\n"
        "adds r0, #180\n"
        "movs r6, #0\n"
        "strh r6, [r0, #0]\n"
        "adds r0, #24\n"
        "movs r2, #0\n"
        "ldrsh r1, [r4, r2]\n"
        "movs r2, #0\n"
        "bl sub_0800D9A4\n"
        "movs r3, #0\n"
        "ldrsh r0, [r4, r3]\n"
        "bl sub_08024C3C\n"
        "adds r1, r0, #0\n"
        "ldr r5, [sp, #24]\n"
        "str r1, [r5, #0]\n"
        "ldr r0, [r7, #36]\n"
        "mov r6, r8\n"
        "ldr r2, [r6, #0]\n"
        "bl sub_08007ABC\n"
        "movs r1, #0\n"
        "ldrsh r0, [r4, r1]\n"
        "bl sub_08024D5C\n"
        "adds r1, r0, #0\n"
        "ldr r2, [sp, #20]\n"
        "str r1, [r2, #0]\n"
        "ldr r0, [r7, #28]\n"
        "ldr r3, [sp, #16]\n"
        "ldr r2, [r3, #0]\n"
        "bl sub_08007ABC\n"
        "movs r5, #0\n"
        "ldrsh r0, [r4, r5]\n"
        "bl sub_08024C58\n"
        "adds r1, r0, #0\n"
        "ldr r6, [sp, #32]\n"
        "str r1, [r6, #0]\n"
        "ldr r0, [r7, #44]\n"
        "ldr r3, [sp, #28]\n"
        "ldr r2, [r3, #0]\n"
        "bl sub_08007ABC\n"
        "b.n .Lcar21374__08021806\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lcar21374__08021740: .4byte 0x03001780\n"
        ".Lcar21374__08021744: .4byte 0x0000103C\n"
        ".Lcar21374__08021748: .4byte 0x0000103E\n"
        ".Lcar21374__0802174C:\n"
        "movs r0, #4\n"
        "str r0, [sp, #0]\n"
        "mov r4, r9\n"
        "str r4, [sp, #4]\n"
        "movs r0, #0\n"
        "adds r1, r6, #0\n"
        "movs r2, #7\n"
        "movs r3, #0\n"
        "bl sub_08007770\n"
        "b.n .Lcar21374__08021806\n"
        ".Lcar21374__08021762:\n"
        "ldr r5, .Lcar21374__08021818\n"
        "ldr r0, .Lcar21374__0802181C\n"
        "adds r1, r5, r0\n"
        "ldrh r0, [r1, #0]\n"
        "subs r0, #1\n"
        "strh r0, [r1, #0]\n"
        "lsls r0, r0, #16\n"
        "cmp r0, #0\n"
        "bgt.n .Lcar21374__08021778\n"
        "movs r2, #0\n"
        "strh r2, [r1, #0]\n"
        ".Lcar21374__08021778:\n"
        "movs r3, #0\n"
        "ldrsh r0, [r1, r3]\n"
        "lsls r0, r0, #1\n"
        "ldr r4, .Lcar21374__08021818\n"
        "ldr r5, .Lcar21374__08021820\n"
        "adds r1, r4, r5\n"
        "adds r0, r0, r1\n"
        "ldrh r0, [r0, #0]\n"
        "adds r5, r7, #0\n"
        "adds r5, #194\n"
        "strh r0, [r5, #0]\n"
        "movs r0, #4\n"
        "str r0, [sp, #0]\n"
        "mov r0, r9\n"
        "str r0, [sp, #4]\n"
        "movs r0, #0\n"
        "adds r1, r6, #0\n"
        "movs r2, #6\n"
        "movs r3, #0\n"
        "bl sub_08007770\n"
        "ldr r4, .Lcar21374__08021824\n"
        "adds r1, r7, #0\n"
        "adds r1, #48\n"
        "adds r0, r4, #0\n"
        "bl sub_0800798C\n"
        "adds r0, r4, #0\n"
        "movs r1, #0\n"
        "movs r2, #3\n"
        "bl sub_080075E8\n"
        "movs r0, #12\n"
        "bl sub_0800572C\n"
        "movs r1, #134\n"
        "lsls r1, r1, #1\n"
        "adds r4, r7, r1\n"
        "str r0, [r4, #0]\n"
        "movs r2, #0\n"
        "ldrsh r0, [r5, r2]\n"
        "bl sub_080258A8\n"
        "adds r1, r0, #0\n"
        "movs r3, #136\n"
        "lsls r3, r3, #1\n"
        "adds r0, r7, r3\n"
        "lsls r1, r1, #16\n"
        "asrs r1, r1, #16\n"
        "str r1, [r0, #0]\n"
        "ldr r0, [r7, #52]\n"
        "ldr r2, [r4, #0]\n"
        "bl sub_08007ABC\n"
        "movs r6, #142\n"
        "lsls r6, r6, #1\n"
        "adds r4, r7, r6\n"
        "movs r1, #0\n"
        "ldrsh r0, [r5, r1]\n"
        "str r0, [r4, #0]\n"
        "movs r1, #4\n"
        "bl sub_08026F30\n"
        "movs r2, #140\n"
        "lsls r2, r2, #1\n"
        "adds r1, r7, r2\n"
        "str r0, [r1, #0]\n"
        "ldr r1, [r4, #0]\n"
        "movs r2, #4\n"
        "bl sub_08026F50\n"
        ".Lcar21374__08021806:\n"
        "add sp, #36\n"
        "pop {r3, r4, r5}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "mov sl, r5\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r0}\n"
        "bx r0\n"
        "movs r0, r0\n"
        ".align 2, 0\n"
        ".Lcar21374__08021818: .4byte 0x03001780\n"
        ".Lcar21374__0802181C: .4byte 0x0000104A\n"
        ".Lcar21374__08021820: .4byte 0x0000104C\n"
        ".Lcar21374__08021824: .4byte 0x082DFBDC\n"
        ".syntax divided\n"
    );
}
void _08021374(void *ctx) __attribute__((alias("CarPhysRacer_InitFrame")));


// Substantiated gap leaf sub_080021C94: ctx = r1, slot = _08004B68->+2
// if *(s16*)(slot+2)==15||20 → *(u16*)(ctx+84)=5 else 6 (asm/carphys_racer_tail.s:7-21)
// The ROM epilogue is `pop {r4}; pop {r0}; bx r0` (B70 at 0x08021CB8), which
// DESTROYS r0 — so the routine is void in the ROM and no caller can observe the
// 5/6 it just stored. Returning int makes agbcc pick a different pop scratch
// (`pop {r1}; bx r1`), which is the only remaining byte difference.
void Gap_021C94_Select(void *a, void *b){
    volatile u16 *dst;
    int out;
    s16 v;
    (void)a;
    v = *(const s16 *)((uintptr_t)_08004B68() + 2);
    if (v == 15 || v == 20) {
        dst = (volatile u16 *)((uintptr_t)b + 84);
        out = 5;
    } else {
        dst = (volatile u16 *)((uintptr_t)b + 84);
        out = 6;
    }
    *dst = (u16)out;
}
__asm__(".align 2, 0");
void _080021C94(void *a, void *b) __attribute__((alias("Gap_021C94_Select")));
void sub_080021C94(void *a, void *b) __attribute__((alias("Gap_021C94_Select")));

// Dispatcher _08021BF8 : 12-entry jump table (ev-1)
void CarPhysRacer_Dispatcher(int ev,int b,int c,void *ctx){
    // Mirrors asm table at 0x08021C14; slot = ev-1
    extern void _08021374(void *x);
    extern void NopCtx21370(void *x) __asm__("_08021370");
    unsigned slot = (unsigned)(ev - 1);
    if (slot > 11)
        return;
    switch(slot){
        // ROM block order is 1,4,6,5,0,11 (table at 0x08021C14); source order
        // sets layout order, so list them in ROM order.
        // ROM 0x08021C44: `adds r0,r4; adds r1,r5; bl 0x0802135C` -- the
        // second argument (r1/b) is the record pointer MarkScene stores into.
        case 1: CarPhysRacer_MarkScene(ctx, (void *)(uintptr_t)b); break;
        case 4: { extern void sub_0800D854(void*); extern void sub_0800D8E4(void*); sub_0800D854((u8*)ctx+0x38); sub_0800D8E4((u8*)ctx+0xA0); break; }
        case 6: { extern void _08021BA0(void*); _08021BA0(ctx); break; }
        case 5: { if (*(volatile u16 *)((uintptr_t)ctx+0x3C)!=0) CarPhysRacer_RaceStartLatch(ctx,(int)(u16)b,(int)(u16)c); break; }
        case 0: _08021374(ctx); break;
        case 11: NopCtx21370(ctx); break;
        default: break;
    }
}
__asm__(".align 2, 0");
void _08021BF8(int ev,int b,int c,void *ctx) __attribute__((alias("CarPhysRacer_Dispatcher")));

// Remaining gap helpers (0x21CC4-0x22CB4, ~280 labels) — TODO pending BL target audit.
// Only sub_080021C94 above is substantiated; rest stay as explicit TODO without alias.
