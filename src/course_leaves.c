#include "gba/types.h"
#include "gba/bios.h"

// Course leaves mechanical — opaque volatile pointers, exact offsets/widths
// VMA 0x080095F0–0x08009B60 flag-gated field writers and helpers

extern void sub_08007570(void *base, int rec, int field, int v, int sp0);

void CourseLeaves_095F0(void) {
    // _080095F0: r0=0x0828FF08, r1=*(0x030003E0)=rec, r2=u16[rec+50], r3=(s16[rec+12])*4, r1=0, stack=4 -> sub_08007570
    void *base = (void *)0x0828FF08u;
    volatile u8 *rec = *(volatile u8 *volatile *)0x030003E0u;
    u16 f50 = *(volatile u16 *)(rec + 50);
    s16 off = *(s16 *)(rec + 12);
    sub_08007570(base, 0, (int)f50, (int)off * 4, 4);
}
#ifndef __APPLE__
void _080095F0(void) __attribute__((alias("CourseLeaves_095F0")));
void sub_080095F0(void) __attribute__((alias("CourseLeaves_095F0")));
#endif

void CourseLeaves_0961C(void) {
    // _0800961C: same shape, r2=u16[rec+48], r3=(s16[rec+8])*4
    void *base = (void *)0x0828FF08u;
    volatile u8 *rec = *(volatile u8 *volatile *)0x030003E0u;
    u16 f48 = *(volatile u16 *)(rec + 48);
    s16 off = *(s16 *)(rec + 8);
    sub_08007570(base, 0, (int)f48, (int)off * 4, 4);
}
#ifndef __APPLE__
void _0800961C(void) __attribute__((alias("CourseLeaves_0961C")));
void sub_0800961C(void) __attribute__((alias("CourseLeaves_0961C")));
#endif

void CourseLeaves_09648(void) {
    if (*(volatile u8 *)(*(volatile u8 *volatile *)0x030003E0u + 77) != 0)
        CourseLeaves_095F0();
    if (*(volatile u8 *)(*(volatile u8 *volatile *)0x030003E0u + 76) != 0)
        CourseLeaves_0961C();
}
#ifndef __APPLE__
void _08009648(void) __attribute__((alias("CourseLeaves_09648")));
void sub_08009648(void) __attribute__((alias("CourseLeaves_09648")));
#endif

void Course_Leaves_096A4(void *arg) {
    // _080096A4: push {r4,r5,lr} sub sp#4, ldr r4,=0x030003E4; str r1,[r4]; movs r0,#0 str [sp]; ldr r2,=0x0500000B; mov r0,sp; bl 02D974 (CpuSet 44B) — CpuSet/argument translation is only partial (stack 44B via 0x0500000B, template 0x0805F604 lane writes at +24 etc. not proven), tail calls to 0798C (template 0x0805F604) and 261B0/F8 at +14/+16/+18 are blocked on s16 table at 0x03001780+0x10C6 (need watch ldrh +12 s16 vs Vu32)
    // Blocker: exact CFG beyond +18 via ldrh/strh at +14/+16/+18 is proven, but CpuSet stack 44B argument at sp and template 0x0805F604 lane at +24 requires watch of CpuSet src/dst/len and 261B0 table at +12 s16 vs +0 Vu32 — leave blocked, no alias until full CFG proven via objdump ldr/str/CpuSet length and watch.
    volatile u32 *slot = (volatile u32 *)0x030003E4u;
    *slot = (u32)(uintptr_t)arg;
    u32 stack = 0;
    extern void sub_0802D974(const void*,void*,u32);
    sub_0802D974(&stack, (void*)0, 0x0500000Bu);
    // Not claimed — tail blocked, no alias
    (void)stack;
}
// _080096A4 remains unaliased — blocked until full CFG with CpuSet 44B stack and 0798C template proven via watch
// _08009674/_08009680/_0800968C removed — duplicates of course_records.c owner; keep exactly one owner (course_records) to avoid ld -r duplicate

void CourseLeaves_09748(u16 a, u16 sel) {
    // _08009748: VMA 0x08009748 pure Thumb, pools 0x030003E4/0xFFFF/0x03001780+0x10CA etc.
    (void)a;
    u16 sel_m = sel;
    register u32 delta __asm__("r2") = 0;
    if (sel_m == 32) delta = 0xFFFFu;
    else if (sel_m == 16) delta = 1;
    volatile u32 *slot = (volatile u32 *)0x030003E4u;
    register volatile u8 *blk __asm__("r1") = *(volatile u8 * volatile *)slot; // unconditional deref: asm has no if (!blk)
    u32 phase = *(volatile u32 *)(blk + 4);
    if (phase == 1) goto ph1;
    if (phase > 1) {
        if (phase == 2) goto ph2;
        return;
    }
    if (phase == 0) goto ph0;
    return;
ph0: {
        s16 d = (s16)delta;
        u16 cur = *(volatile u16 *)(blk + 20);
        s32 sum = (s32)cur + (s32)d;
        u16 sum_u = (u16)sum;
        *(volatile u16 *)(blk + 20) = sum_u;
        s16 sum_s = (s16)sum_u;
        if (sum_s <= 0) { *(volatile u16 *)(blk + 20) = 1; return; }
        volatile u16 *p_max_u = (volatile u16 *)0x0300284Au;
        volatile s16 *p_bound = (volatile s16 *)0x0300284Au;
        u16 max_u = *p_max_u;
        s16 bound = *p_bound;
        if (sum_s > bound) *(volatile u16 *)(blk + 20) = max_u;
        return;
    }
ph1: {
        s16 d = (s16)delta;
        u16 cur = *(volatile u16 *)(blk + 22);
        s32 sum = (s32)cur + (s32)d;
        *(volatile u16 *)(blk + 22) = (u16)sum;
        return;
    }
ph2: {
        s16 d = (s16)delta;
        u16 cur = *(volatile u16 *)(blk + 24);
        s32 sum = (s32)cur + (s32)d;
        u16 sum_u = (u16)sum;
        *(volatile u16 *)(blk + 24) = sum_u;
        s16 sum_s = (s16)sum_u;
        if (sum_s <= 0) { *(volatile u16 *)(blk + 24) = 1; return; }
        if (sum_s > 3) { *(volatile u16 *)(blk + 24) = 3; return; }
        return;
    }
}
#ifndef __APPLE__
void _08009748(u16 a, u16 b) __attribute__((alias("CourseLeaves_09748")));
void sub_08009748(u16 a, u16 b) __attribute__((alias("CourseLeaves_09748")));
#endif

int _080097E8(u16 a, u16 sel) {
    // _080097E8: VMA 0x080097E8 116 B pure Thumb, smallest remaining leaf with no external BL.
    // Fidelity: asm at 097F2/09800/09814 is ldr r0,=0x030003E4 / ldr r0,[r0] / str/ldr [r0,#0] with no null guard (09734 ldr r0,[r0] then str r3,[r0]; 09800 ldr r2,[r0] etc.). Slot is always valid (0x03005B38 from 2280 per course_header_trace); C dereferences unconditionally to match exact asm control flow. No silent safety branch added.
    (void)a; // r0 incoming ignored per asm (r1 is selector; r0 overwritten with slot load)
    u16 k = sel; // lsls #16 lsrs #16 Vu16
    int r3 = 0; // movs r3,#0
    if (k == 8) { // cmp #8
        volatile u32 *slot = (volatile u32 *)0x030003E4u;
        volatile u32 *blk = *(volatile u32 * volatile *)slot; // unconditional: asm ldr r0,[r0] / str r3,[r0,#0] Vu32
        *blk = (u32)r3; // r3=0 stored, then r3=2
        r3 = 2; // movs r3,#2
    }
    if (k == 1) { // cmp #1
        r3 = 2;
    }
    if (k == 64) { // cmp #64
        volatile u32 *slot = (volatile u32 *)0x030003E4u;
        volatile u32 *blk = *(volatile u32 * volatile *)slot; // unconditional deref
        u32 v = *blk; // ldr Vu32 at [blk+0]
        if ((s32)v > 0) { // cmp #0 ble
            r3 = 1; // movs r3,#1
            *blk = v - 1; // subs #1 str Vu32
        }
    }
    if (k == 128) { // cmp #128
        volatile u32 *slot = (volatile u32 *)0x030003E4u;
        volatile u32 *blk = *(volatile u32 * volatile *)slot; // unconditional
        u32 v = *blk; // ldr Vu32
        if ((s32)v <= 1) { // cmp #1 bgt skip
            r3 = 1;
            *blk = v + 1; // adds #1 str
        }
    }
    return r3; // adds r0,r3
}
#ifndef __APPLE__
int CourseLeaves_097E8(u16 a, u16 b) __attribute__((alias("_080097E8")));
int sub_080097E8(u16 a, u16 b) __attribute__((alias("_080097E8")));
#endif

int CourseLeaves_098C8(u16 a, u16 sel) {
    // _080098C8: VMA 0x080098C8 40 B pure Thumb dispatcher.
    u16 r2 = a; // lsls r0,#16 lsrs r2,#16 Vu16
    u16 r1 = sel; // lsls/lsrs Vu16 (already u16 param)
    volatile u32 *slot = (volatile u32 *)0x030003E4u;
    volatile u8 *blk = *(volatile u8 * volatile *)slot; // unconditional: asm ldr r0,[r0] with no cmp/beq
    // The mode read is a PLAIN s16 load, not `volatile s16`: the volatile
    // qualifier forces `ldrh` + `lsls #16` + `asrs #16`, while the plain read
    // takes the ROM's `movs r3,#12` / `ldrsh r0,[r0,r3]` -- including the
    // register-offset form agbcc only emits for the non-volatile spelling.
    // The 2-way dispatch is a `switch`: that is what puts the case-0 body
    // OUT OF LINE behind the literal pool (`cmp #0 / beq case0`), the way the
    // ROM has it. The `if / else if / else` chain scores 31/56, the same
    // branches as a `switch` score 54/56 -- contiguous prefix 54 with the
    // ROM's `00 00` align tail, i.e. alignment-only.
    s16 mode = *(s16 *)(blk + 12);
    switch (mode) {
    case 0: {
        extern int _080097E8(u16, u16);
        return _080097E8(r2, r1); // adds r0,r2 / bl 097E8
    }
    case 1: {
        extern int sub_08009830(u16, u16);
        return sub_08009830(r2, r1); // adds r0,r2 / bl 09830, still asm leaf
    }
    default:
        return 0; // movs r0,#0
    }
}
__asm__(".align 2, 0");
#ifndef __APPLE__
int _080098C8(u16 a, u16 b) __attribute__((alias("CourseLeaves_098C8")));
int sub_080098C8(u16 a, u16 b) __attribute__((alias("CourseLeaves_098C8")));
#endif

void CourseLeaves_09B28(void) {
    extern void sub_08009900(void);
    extern void sub_080099D0(void);
    // _08009B28: s16 at [*(0x030003E4)+12]: 0 -> sub_08009900, 1 -> sub_080099D0, else nothing
    // switch (not else-if) is required: agbcc emits cmp/beq chains for switch,
    // while else-if emits bne. An explicit default: break is required so the
    // trailing b.n reaches the same join point.
    volatile u8 *blk = *(volatile u8 *volatile *)0x030003E4u;
    s16 mode = *(s16 *)(blk + 12);
    switch (mode) {
    case 0:
        sub_08009900();
        break;
    case 1:
        sub_080099D0();
        break;
    default:
        break;
    }
}
__asm__(".space 2, 0");
#ifndef __APPLE__
void _08009B28(void) __attribute__((alias("CourseLeaves_09B28")));
void sub_08009B28(void) __attribute__((alias("CourseLeaves_09B28")));
#endif

u16 CourseLeaves_09B50(void) {
    // _08009B50: push {lr} bl _08002494 lsls r0,#16 lsrs r0,#16 pop {r1} bx r1
    extern u16 _08002494(void);
    u16 v = _08002494();
    return (u16)v;
}
__asm__(".align 2, 0");
#ifndef __APPLE__
u16 _08009B50(void) __attribute__((alias("CourseLeaves_09B50")));
u16 sub_08009B50(void) __attribute__((alias("CourseLeaves_09B50")));
void Sub_08009B28(void) __attribute__((alias("CourseLeaves_09B28")));
#endif
