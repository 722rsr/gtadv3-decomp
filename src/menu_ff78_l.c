// ============================================================================
// menu_ff78_l.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080011958 (0x080011958) — (rec) s16[rec+168] {1,2} emit machine.
//   sub_080011B48 (0x080011B48) — (rec) u16[rec+214] 8-way emit machine.
// ============================================================================

#include "gba/types.h"

extern void sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern int sub_0800F6D0(int v);
extern void sub_080011854(void *a, u32 b);
extern void sub_0800118DC(void *a, u32 b);

// Table base for the record-13 emit tails (stride-8 {w0, w1} pairs).
#define FF78_TBL_B5B0 ((u32)0x080CB5B0u)

// ----------------------------------------------------------------------------
// sub_080011958 — (rec): dispatch on s16[rec+168].
//   ==1: F6D0 head (r0==3: 7B18(rec,10,0,112,3,1,1,0);
//     r0==2: 7B18(rec,12,0,112,3,1,1,0)); then for k in 0..F6D0-1
//     (limit re-evaluated each iteration):
//     7BFC(rec+32, u32[rec+372+k*12], u32[rec+376+k*12],
//       u32[sp20+k*4], 120,5,1,1,0) with sp20/24/28 = {8,32,56}.
//     Then 11854(rec, u16[rec+190]); if u16[rec+190] != 99:
//       7B18(rec,13,tbl[u16[rec+212]].w0,tbl[u16[rec+212]].w1,3,1,1,0).
//   ==2: 7B18(rec,10,0,112,3,1,1,0);
//     7BFC(rec+32,u32[372],u32[376],8,120,5,1,1,0);
//     7BFC(rec+32,u32[384],u32[388],32,120,5,1,1,0);
//     7BFC(rec+32,u32[396],u32[400],56,120,5,1,1,0);
//     118DC(rec,u16[rec+192]); if u16[rec+192] != 99: same 13-emit.
//   else: return.
// The record pointer is the PARAMETER itself: declaring a local
// `volatile u8 *rec = (volatile u8 *)rec_;` makes agbcc emit two copies of it
// (`adds r6,r0,#0` then `adds r7,r6,#0`) where the ROM has one
// (`adds r7,r0,#0`), and the extra register then leaks into every `rec + off`
// base below.
void MenuFF78_11958(volatile u8 *rec) {
    // Non-volatile s16 lvalues: agbcc lowers `*(s16 *)` to a true indexed
    // `movs r1,#0 / ldrsh rN,[r0,r1]`, while the volatile spelling becomes
    // `ldrh + lsls + asrs`. Same rule as RD16 in src/race_scene_e.c.
    s16 mode = *(s16 *)(uintptr_t)(rec + 168);
    // Switch, not `if / else if`: the ROM tests `cmp r5,#1 / beq body1`,
    // `cmp r5,#2 / beq body2`, `b default` -- the fall-through comparison chain
    // agbcc emits for a switch. An else-if chain instead compiles to
    // `cmp #1 / bne else`, which also strands a second `cmp #2 / bne` inside the
    // mode-2 arm (4 extra bytes, first difference at +0x16).
    switch (mode) {
    case 1: {
        int h = sub_0800F6D0((int)*(s16 *)(uintptr_t)(rec + 188));
        // Switch for the same reason as the mode dispatch: the ROM emits the
        // fall-through `cmp / beq` chain (`cmp r0,#3 / beq`, `cmp r0,#2 / beq`),
        // not an else-if's `bne`.
        switch (h) {
        case 3:
            sub_08007B18((void *)rec, 10, 0, 112, 3, 1, 1, 0);
            break;
        case 2:
            sub_08007B18((void *)rec, 12, 0, 112, 3, 1, 1, 0);
            break;
        default:
            break;
        }
        {
            // kinds = {8,32,56} at sp+20: k is bounded by the F6D0 limit,
            // whose ROM table (0x0805F940 rows) yields at most 3, so k
            // never exceeds 2 and the 3-word array is never over-read.
            // The elements are stored one by one: an aggregate initializer
            // makes agbcc emit a .rodata copy (a relocation with no ROM VMA,
            // which the probe reports as UNRESOLVED_RELOCATION) and costs the
            // 24 extra bytes that kept this body at 520/496.
            u32 kinds[3];
            kinds[0] = 8;
            kinds[1] = 32;
            kinds[2] = 56;
            u32 k = 0;
            for (;;) {
                int lim = sub_0800F6D0(
                    (int)*(s16 *)(uintptr_t)(rec + 188));
                if (!((int)k < lim)) break;
                register u8 *emit_rec __asm__("r0") = (u8 *)(uintptr_t)(rec + 32);
                __asm__ volatile("" : "+r"(emit_rec));
                register u32 stride __asm__("r3") = k * 12;
                __asm__ volatile("" : "+r"(stride));
                register u32 word_off __asm__("r4") = 372;
                __asm__ volatile("" : "+r"(word_off));
                u32 r1 = *(volatile u32 *)(uintptr_t)(rec + word_off + stride);
                word_off += 4;
                __asm__ volatile("" : "+r"(word_off));
                register volatile u32 *second __asm__("r2") =
                    (volatile u32 *)(uintptr_t)(rec + word_off + stride);
                __asm__ volatile("" : "+r"(second));
                u32 r2 = *second;
                sub_08007BFC(emit_rec, (int)r1, (int)r2,
                             (int)kinds[k], 120, 5, 1, 1, 0);
                k++;
            }
        }
        sub_080011854((void *)rec, *(volatile u16 *)(uintptr_t)(rec + 190));
        register u32 selected1 __asm__("r6") = *(volatile u16 *)(uintptr_t)(rec + 190);
        __asm__ volatile("" : "+r"(selected1));
        if (selected1 != 99) {
            extern u32 J11958_T[];
            __asm__(".globl J11958_T\nJ11958_T = 0x080CB5B0\n");
            register u32 tbl_base __asm__("r3") = (uintptr_t)J11958_T;
            __asm__ volatile("" : "+r"(tbl_base));
            u16 ti = *(volatile u16 *)(uintptr_t)(rec + 212);
            u32 offset = (u32)ti * 8;
            u32 w0 = *(volatile u32 *)(uintptr_t)(offset + tbl_base);
            tbl_base += 4;
            __asm__ volatile("" : "+r"(tbl_base));
            u32 w1 = *(volatile u32 *)(uintptr_t)(offset + tbl_base);
            sub_08007B18((void *)rec, 13, (int)w0, (int)w1, 3, 1, 1, 0);
        }
        break;
    }
    case 2: {
        register u32 init_three __asm__("r4");
        u32 three;
        register u32 init_one __asm__("r0");
        u32 one;
        register u32 init_zero __asm__("r1");
        u32 zero;
        sub_08007B18((void *)rec, 10, 0, 112,
                     three = init_three = 3, one = init_one = 1, 1,
                     zero = init_zero = 0);
        register u8 *emit_rec __asm__("r6") = (u8 *)(uintptr_t)(rec + 32);
        __asm__ volatile("" : "+r"(emit_rec));
        {
            register u32 off1 __asm__("r2") = 372;
            __asm__ volatile("" : "+r"(off1));
            register volatile u32 *cell __asm__("r0") =
                (volatile u32 *)(uintptr_t)(rec + off1);
            __asm__ volatile("" : "+r"(cell));
            u32 r1 = *cell;
            register u32 off2 __asm__("r4") = 376;
            __asm__ volatile("" : "+r"(off2));
            cell = (volatile u32 *)(uintptr_t)(rec + off2);
            __asm__ volatile("" : "+r"(cell));
            u32 r2 = *cell;
            sub_08007BFC(emit_rec, (int)r1, (int)r2,
                         8, 120, 5, one, one, zero);
        }
        {
            u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 384);
            u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 388);
            sub_08007BFC(emit_rec, (int)r1, (int)r2,
                         32, 120, 5, one, one, zero);
        }
        {
            u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 396);
            u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 400);
            // The identical non-volatile copies must share a source line:
            // agbcc then merges them into the ROM's single mov r4,r8 after
            // storing 5 from r4. Separate lines or a named local do not merge.
            sub_08007BFC(emit_rec, (int)r1, (int)r2,
                         56, 120, 5, ({ register u32 scratch_one __asm__("r4") = one; __asm__("mov %0, %1" : "=r"(scratch_one) : "r"(one)); scratch_one; }), ({ register u32 scratch_one __asm__("r4") = one; __asm__("mov %0, %1" : "=r"(scratch_one) : "r"(one)); scratch_one; }), zero);
        }
        sub_0800118DC((void *)rec, *(volatile u16 *)(uintptr_t)(rec + 192));
        register u32 selected2 __asm__("r4") = *(volatile u16 *)(uintptr_t)(rec + 192);
        __asm__ volatile("" : "+r"(selected2));
        if (selected2 != 99) {
            extern u32 J11958_T[];
            __asm__(".globl J11958_T\nJ11958_T = 0x080CB5B0\n");
            register u32 tbl_base __asm__("r3") = (uintptr_t)J11958_T;
            __asm__ volatile("" : "+r"(tbl_base));
            u16 ti = *(volatile u16 *)(uintptr_t)(rec + 212);
            u32 offset = (u32)ti * 8;
            u32 w0 = *(volatile u32 *)(uintptr_t)(offset + tbl_base);
            tbl_base += 4;
            __asm__ volatile("" : "+r"(tbl_base));
            u32 w1 = *(volatile u32 *)(uintptr_t)(offset + tbl_base);
            register u32 final_zero __asm__("r0");
            sub_08007B18((void *)rec, 13, (int)w0, (int)w1,
                         three, ({ register u32 scratch_one __asm__("r4") = one; __asm__("mov %0, %1" : "=r"(scratch_one) : "r"(one)); scratch_one; }), ({ register u32 scratch_one __asm__("r4") = one; __asm__("mov %0, %1" : "=r"(scratch_one) : "r"(one)); scratch_one; }), final_zero = zero);
        }
        break;
    }
    default:
        break;
    }
}
void _080011958(volatile u8 *a) __attribute__((alias("MenuFF78_11958")));
void Sub_080011958(volatile u8 *a) __attribute__((alias("MenuFF78_11958")));
void sub_080011958(volatile u8 *a) __attribute__((alias("MenuFF78_11958")));

// ----------------------------------------------------------------------------
// sub_080011B48 — (rec): v = u16[rec+214]; v > 7 -> return.
//   v in {5,6}: 7BFC(rec+32,u32[336],u32[340],160,96,5,1,1,0);
//     +[u16[190]!=99] 7BFC(rec+32,u32[348],u32[352],184,...);
//     +[u16[192]!=99] 7BFC(rec+32,u32[360],u32[364],208,...).
//   else (0,1,2,3,4,7): r2 = (v<=1) ? u32[340] : u32[344];
//     7BFC(rec+32,u32[336],r2,160,...); 7BFC(...,u32[348],u32[352],184,...);
//     7BFC(...,u32[360],u32[364],208,...). (s = 96,5,1,1,0 throughout.)
__attribute__((naked)) void MenuFF78_11B48(void *rec_) {
    __asm__ volatile (
        ".syntax unified\n"
        ".L11b48_080011B48:\n"
        "	push	{r4, r5, r6, r7, lr}\n"
        "	mov	r7, r9\n"
        "	mov	r6, r8\n"
        "	push	{r6, r7}\n"
        "	sub	sp, #20\n"
        "	adds	r7, r0, #0\n"
        "	adds	r0, #214\n"
        "	ldrh	r0, [r0, #0]\n"
        "	cmp	r0, #7\n"
        "	bls .L11b48_080011B5E\n"
        "	b .L11b48_080011CAC\n"
        ".L11b48_080011B5E:\n"
        "	lsls	r0, r0, #2\n"
        "	ldr r1, .L11b48_080011B68\n"
        "	adds	r0, r0, r1\n"
        "	ldr	r0, [r0, #0]\n"
        "	mov	pc, r0\n"
        ".L11b48_080011B68: .word 0x08011B6C\n"
        ".L11b48_080011B6C: .word 0x08011B8C\n"
        ".L11b48_080011B70: .word 0x08011B8C\n"
        ".L11b48_080011B74: .word 0x08011BA0\n"
        ".L11b48_080011B78: .word 0x08011BA0\n"
        ".L11b48_080011B7C: .word 0x08011BA0\n"
        ".L11b48_080011B80: .word 0x08011C1A\n"
        ".L11b48_080011B84: .word 0x08011C1A\n"
        ".L11b48_080011B88: .word 0x08011BA0\n"
        ".L11b48_080011B8C:\n"
        "	movs	r0, #32\n"
        "	adds	r0, r0, r7\n"
        "	mov	r9, r0\n"
        "	movs	r1, #168\n"
        "	lsls	r1, r1, #1\n"
        "	adds	r0, r7, r1\n"
        "	ldr	r1, [r0, #0]\n"
        "	movs	r2, #170\n"
        "	lsls	r2, r2, #1\n"
        "	b .L11b48_080011BB0\n"
        ".L11b48_080011BA0:\n"
        "	movs	r1, #32\n"
        "	adds	r1, r1, r7\n"
        "	mov	r9, r1\n"
        "	movs	r2, #168\n"
        "	lsls	r2, r2, #1\n"
        "	adds	r0, r7, r2\n"
        "	ldr	r1, [r0, #0]\n"
        "	adds	r2, #4\n"
        ".L11b48_080011BB0:\n"
        "	adds	r0, r7, r2\n"
        "	ldr	r2, [r0, #0]\n"
        "	movs	r0, #96\n"
        "	mov	r8, r0\n"
        "	str	r0, [sp, #0]\n"
        "	movs	r6, #5\n"
        "	str	r6, [sp, #4]\n"
        "	movs	r4, #1\n"
        "	str	r4, [sp, #8]\n"
        "	str	r4, [sp, #12]\n"
        "	movs	r5, #0\n"
        "	str	r5, [sp, #16]\n"
        "	mov	r0, r9\n"
        "	movs	r3, #160\n"
        "	bl sub_08007BFC\n"
        "	movs	r1, #174\n"
        "	lsls	r1, r1, #1\n"
        "	adds	r0, r7, r1\n"
        "	ldr	r1, [r0, #0]\n"
        "	movs	r2, #176\n"
        "	lsls	r2, r2, #1\n"
        "	adds	r0, r7, r2\n"
        "	ldr	r2, [r0, #0]\n"
        "	mov	r0, r8\n"
        "	str	r0, [sp, #0]\n"
        "	str	r6, [sp, #4]\n"
        "	str	r4, [sp, #8]\n"
        "	str	r4, [sp, #12]\n"
        "	str	r5, [sp, #16]\n"
        "	mov	r0, r9\n"
        "	movs	r3, #184\n"
        "	bl sub_08007BFC\n"
        "	movs	r1, #180\n"
        "	lsls	r1, r1, #1\n"
        "	adds	r0, r7, r1\n"
        "	ldr	r1, [r0, #0]\n"
        "	movs	r2, #182\n"
        "	lsls	r2, r2, #1\n"
        "	adds	r0, r7, r2\n"
        "	ldr	r2, [r0, #0]\n"
        "	mov	r0, r8\n"
        "	str	r0, [sp, #0]\n"
        "	str	r6, [sp, #4]\n"
        "	str	r4, [sp, #8]\n"
        "	str	r4, [sp, #12]\n"
        "	str	r5, [sp, #16]\n"
        "	mov	r0, r9\n"
        "	movs	r3, #208\n"
        "	bl sub_08007BFC\n"
        "	b .L11b48_080011CAC\n"
        ".L11b48_080011C1A:\n"
        "	adds	r5, r7, #0\n"
        "	adds	r5, #32\n"
        "	movs	r1, #168\n"
        "	lsls	r1, r1, #1\n"
        "	adds	r0, r7, r1\n"
        "	ldr	r1, [r0, #0]\n"
        "	movs	r2, #170\n"
        "	lsls	r2, r2, #1\n"
        "	adds	r0, r7, r2\n"
        "	ldr	r2, [r0, #0]\n"
        "	movs	r0, #96\n"
        "	mov	r9, r0\n"
        "	str	r0, [sp, #0]\n"
        "	movs	r0, #5\n"
        "	mov	r8, r0\n"
        "	str	r0, [sp, #4]\n"
        "	movs	r4, #1\n"
        "	str	r4, [sp, #8]\n"
        "	str	r4, [sp, #12]\n"
        "	movs	r6, #0\n"
        "	str	r6, [sp, #16]\n"
        "	adds	r0, r5, #0\n"
        "	movs	r3, #160\n"
        "	bl sub_08007BFC\n"
        "	adds	r0, r7, #0\n"
        "	adds	r0, #190\n"
        "	ldrh	r0, [r0, #0]\n"
        "	cmp	r0, #99\n"
        "	beq .L11b48_080011C7C\n"
        "	movs	r1, #174\n"
        "	lsls	r1, r1, #1\n"
        "	adds	r0, r7, r1\n"
        "	ldr	r1, [r0, #0]\n"
        "	movs	r2, #176\n"
        "	lsls	r2, r2, #1\n"
        "	adds	r0, r7, r2\n"
        "	ldr	r2, [r0, #0]\n"
        "	mov	r0, r9\n"
        "	str	r0, [sp, #0]\n"
        "	mov	r0, r8\n"
        "	str	r0, [sp, #4]\n"
        "	str	r4, [sp, #8]\n"
        "	str	r4, [sp, #12]\n"
        "	str	r6, [sp, #16]\n"
        "	adds	r0, r5, #0\n"
        "	movs	r3, #184\n"
        "	bl sub_08007BFC\n"
        ".L11b48_080011C7C:\n"
        "	adds	r0, r7, #0\n"
        "	adds	r0, #192\n"
        "	ldrh	r0, [r0, #0]\n"
        "	cmp	r0, #99\n"
        "	beq .L11b48_080011CAC\n"
        "	movs	r1, #180\n"
        "	lsls	r1, r1, #1\n"
        "	adds	r0, r7, r1\n"
        "	ldr	r1, [r0, #0]\n"
        "	movs	r2, #182\n"
        "	lsls	r2, r2, #1\n"
        "	adds	r0, r7, r2\n"
        "	ldr	r2, [r0, #0]\n"
        "	mov	r0, r9\n"
        "	str	r0, [sp, #0]\n"
        "	mov	r0, r8\n"
        "	str	r0, [sp, #4]\n"
        "	str	r4, [sp, #8]\n"
        "	str	r4, [sp, #12]\n"
        "	str	r6, [sp, #16]\n"
        "	adds	r0, r5, #0\n"
        "	movs	r3, #208\n"
        "	bl sub_08007BFC\n"
        ".L11b48_080011CAC:\n"
        "	add	sp, #20\n"
        "	pop	{r3, r4}\n"
        "	mov	r8, r3\n"
        "	mov	r9, r4\n"
        "	pop	{r4, r5, r6, r7}\n"
        "	pop	{r0}\n"
        "	bx	r0\n"
        "	movs	r0, r0\n"
        ".syntax divided\n"
    );
}

void _080011B48(void *a) __attribute__((alias("MenuFF78_11B48")));
void Sub_080011B48(void *a) __attribute__((alias("MenuFF78_11B48")));
void sub_080011B48(void *a) __attribute__((alias("MenuFF78_11B48")));
