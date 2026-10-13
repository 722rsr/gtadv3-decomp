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
void MenuFF78_11B48(void *rec_) {
    volatile u8 *rec = (volatile u8 *)rec_;
    u16 v = *(volatile u16 *)(uintptr_t)(rec + 214);
    if (v > 7) return;
    if (v == 5 || v == 6) {
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 336);
        u32 r2 = *(volatile u32 *)(uintptr_t)(rec + 340);
        sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)r1, (int)r2,
                     160, 96, 5, 1, 1, 0);
        if (*(volatile u16 *)(uintptr_t)(rec + 190) != 99) {
            u32 a = *(volatile u32 *)(uintptr_t)(rec + 348);
            u32 b = *(volatile u32 *)(uintptr_t)(rec + 352);
            sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)a, (int)b,
                         184, 96, 5, 1, 1, 0);
        }
        if (*(volatile u16 *)(uintptr_t)(rec + 192) != 99) {
            u32 a = *(volatile u32 *)(uintptr_t)(rec + 360);
            u32 b = *(volatile u32 *)(uintptr_t)(rec + 364);
            sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)a, (int)b,
                         208, 96, 5, 1, 1, 0);
        }
    } else {
        u32 r1 = *(volatile u32 *)(uintptr_t)(rec + 336);
        u32 r2 = (v <= 1) ? *(volatile u32 *)(uintptr_t)(rec + 340)
                          : *(volatile u32 *)(uintptr_t)(rec + 344);
        sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)r1, (int)r2,
                     160, 96, 5, 1, 1, 0);
        {
            u32 a = *(volatile u32 *)(uintptr_t)(rec + 348);
            u32 b = *(volatile u32 *)(uintptr_t)(rec + 352);
            sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)a, (int)b,
                         184, 96, 5, 1, 1, 0);
        }
        {
            u32 a = *(volatile u32 *)(uintptr_t)(rec + 360);
            u32 b = *(volatile u32 *)(uintptr_t)(rec + 364);
            sub_08007BFC((void *)(uintptr_t)(rec + 32), (int)a, (int)b,
                         208, 96, 5, 1, 1, 0);
        }
    }
}
void _080011B48(void *a) __attribute__((alias("MenuFF78_11B48")));
void Sub_080011B48(void *a) __attribute__((alias("MenuFF78_11B48")));
void sub_080011B48(void *a) __attribute__((alias("MenuFF78_11B48")));
