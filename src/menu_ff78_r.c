// ============================================================================
// menu_ff78_r.c — reconstructed C for asm/menu_ff78.s (1 function).
//
// Body transcribed from the asm listing, then shaped until agbcc reproduces
// it byte for byte (728/728 bytes including the literal pool).
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080013108 (0x080013108) — (rec) full setup + DMA table fill.
// ============================================================================

#include "gba/types.h"
#include "gtadv/ram_blocks.h"

extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
extern void Sub_0800DAB8(void *a);
extern void Sub_080075E8(void *a, int b, int c);
extern void Sub_0800798C(void *a, void *b);
extern void Sub_08007A58(void *a);
extern void Sub_08007ABC(u32 a, u32 b, u32 c);
extern void Sub_0800D77C(void *a, int b, int c);
extern int Sub_08002124(u32 v);
extern void Sub_08002B214(int v);
extern void Sub_080012E5C(int v);
extern int Sub_080012E74(int v);
extern void Sub_080012F98(void *a);
extern void Sub_080013024(void *a);
extern u32 Sub_0800572C(int v);
extern int Sub_08024C3C(int v);
extern int Sub_08024C58(int v);
extern void Sub_0800D9A4(void *a, int b, int c);

// ----------------------------------------------------------------------------
// sub_080013108 — (rec):
//   [u16[WA+0xFBC]==3] 02124(0x1393); s16[rec+138] = (u16==7) ? 0 : 1.
//   2B214(49); 07770(0, 0x082EE524, 3, 0, 4, 1);
//   u32[104]=6; u32[116]=4; u32[128]=1; DAB8(rec+32);
//   tile=0x082A798C: 75E8(tile,0,3); 75E8(tile,2,7);
//   798C(0x082EE524,rec); 75E8(tm,0,4); 75E8(tm,1,6); 75E8(tm,2,8);
//   7A58(rec); s16[138]: 0->12F98(rec), 1->13024(rec).
//   75E8(0x082C4228,0,5); 798C(0x082ECAE4,rec+8);
//   12E5C(s16[176]); u32[584]=0572C(16); u32[588]=(u32)12E74(s16[176]);
//   7ABC(u32[12], e74, u32[584]);
//   798C(0x082C5040,rec+16); u32[596]=0572C(14); u32[600]=5;
//   7ABC(u32[20],5,u32[596]);
//   798C(0x082D0DC0,rec+24); u32[608]=0572C(7); u32[612]=50;
//   7ABC(u32[28],50,u32[608]);
//   u32[32]=0; s16[36]=1; D77C(52,0,-32); D77C(44,0,160);
//   s16[40]=6; s16[42]=5;
//   D9A4(rec+580, s16[174], s16[172]);
//   u32[600]=24C3C(s16[174]); 7ABC(u32[20],u32[600],u32[596]);
//   u32[612]=24C58(s16[174]); 7ABC(u32[28],u32[612],u32[608]);
//   DMA: for k in 1..5: [0x040000D4] = (u32)(0x080CD7A8 +
//     (s8)u8[0x080CCEEC + s16[174]*20 + k]*2);
//     [..+4] = 0x05000280+k*2; [..+8] = 0x80000001 (+ dummy read).
//
// Shaping notes (the last walls, measured with corpus_match_probe --diff):
//
// * The DMA control word is the ROM's pooled 0x80000001, written from r3 with
//   one `ldr r3,[pc,..]` hoisted ahead of the five unrolled stores.  Writing
//   the literal in each step lets agbcc rematerialise a `movs r3,#1`-style
//   immediate instead; spelling the full 0x80000001 (which no Thumb immediate
//   can build) makes it pool the word and CSE the five loads into the ROM's
//   single hoisted load + five stores.  `(void)dma[2]` keeps the ROM's dummy
//   read-back after each store.
// * The rec+600 pointer (`r9v`) must land in r9, but its address is computed
//   and used inside one basic block, so local-alloc allocates it — and takes
//   r8, the first free register at that point — giving `mov r8,r0` where the
//   ROM has `mov r9,r0` and pushing rec+612 (the ROM's r8) to r9.  The two
//   empty barriers around the definition occupy r8 for exactly the window in
//   which local-alloc scores that quantity (`ghost8` is a pinned dummy; no
//   template text, so no bytes), which makes the first free register r9.  The
//   window closes before rec+612's definition, so global-alloc can still give
//   rec+612 = r8, as the ROM does.
//   (Measured alternatives: pinning either pointer to r8/r9 adds a copy and
//   changes the tail scratch rotation; extending r9v's liveness instead makes
//   it sort ahead of the rec+138 pointer and swaps r8/r9 the other way.)
// * `rec + N` addresses stay pointer arithmetic (`*(volatile u8 *)(rec + N)`),
//   never `(uintptr_t)(rec + N)`: the pointer form keeps the ROM's separate
//   `adds rX,r7,#0 / adds rX,#N` pair.
// ----------------------------------------------------------------------------
#define FF78_DMA_STEP(K, DEST)                                             \
    do {                                                                   \
        volatile u8 *row_;                                                 \
        s32 byte_;                                                         \
        __asm__ volatile ("" : : : "memory");                              \
        idx_ = (s32)*(s16 *)(uintptr_t)r6v;                                \
        row_ = (volatile u8 *)(uintptr_t)((u32)(idx_ * 5) * 4 + r5v);      \
        byte_ = (s32)((volatile s8 *)row_)[K];                             \
        __asm__ volatile ("" : "+r"(byte_));                               \
        dma[0] = 0x080CD7A8u + (u32)(byte_ * 2);                           \
        dma[1] = (DEST);                                                   \
        dma[2] = 0x80000001u;                                              \
        (void)dma[2];                                                      \
    } while (0)

void MenuFF78_13108(void *rec_) {
    RAM_DEF_03001780;
    register u32 r4v __asm__("r4");
    register u32 r5v __asm__("r5");
    register u32 r6v __asm__("r6");
    volatile u8 *rec;
    volatile u32 *r9v;
    volatile u32 *r8v;
    register u32 ghost8 __asm__("r8");
    register u32 wa_base __asm__("r0");
    register u32 wa_off __asm__("r1");
    register u32 mode __asm__("r2");
    register u32 val;
    register s32 idx_ __asm__("r1");
    volatile u32 *dma;

    rec = (volatile u8 *)rec_;

    wa_base = (u32)RAM_SYM_03001780;
    wa_off = 0x0FBCu;
    r4v = wa_base + wa_off;
    mode = (u32)*(u16 *)(uintptr_t)r4v;
    if (mode == 3)
        Sub_08002124(0x1393u);
    r4v = (u32)*(u16 *)(uintptr_t)r4v;
    if (r4v == 7)
        *(volatile s16 *)(uintptr_t)(rec + 138) = 0;
    else
        *(volatile s16 *)(uintptr_t)(rec + 138) = 1;

    Sub_08002B214(49);

    r5v = 0x082EE524u;
    Sub_08007770(0, (void *)(uintptr_t)r5v, 3, 0, 4, 1);

    *(volatile u32 *)(uintptr_t)(rec + 104) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 116) = 4;
    { volatile u8 *p1 = rec + 128; *(volatile u32 *)(uintptr_t)p1 = 1; }
    { volatile u8 *p2 = rec + 32; Sub_0800DAB8((void *)(uintptr_t)p2); }

    r4v = 0x082A798Cu;
    Sub_080075E8((void *)(uintptr_t)r4v, 0, 3);
    Sub_080075E8((void *)(uintptr_t)r4v, 2, 7);
    Sub_0800798C((void *)(uintptr_t)r5v, (void *)(uintptr_t)rec);
    Sub_080075E8((void *)(uintptr_t)r5v, 0, 4);
    Sub_080075E8((void *)(uintptr_t)r5v, 1, 6);
    Sub_080075E8((void *)(uintptr_t)r5v, 2, 8);
    Sub_08007A58((void *)(uintptr_t)rec);

    r8v = (volatile u32 *)(uintptr_t)(rec + 138);
    switch ((int)*(s16 *)(uintptr_t)r8v) {
    case 0: Sub_080012F98((void *)(uintptr_t)rec); break;
    case 1: Sub_080013024((void *)(uintptr_t)rec); break;
    default: break;
    }

    Sub_080075E8((void *)(uintptr_t)0x082C4228u, 0, 5);
    Sub_0800798C((void *)(uintptr_t)0x082ECAE4u, (void *)(uintptr_t)(rec + 8));

    r4v = (u32)(uintptr_t)(rec + 176);
    Sub_080012E5C((int)*(s16 *)(uintptr_t)r4v);
    *(volatile u32 *)(uintptr_t)(rec + 584) = Sub_0800572C(16);
    val = (u32)Sub_080012E74((int)*(s16 *)(uintptr_t)r4v);
    *(volatile u32 *)(uintptr_t)(rec + 588) = val;
    Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 12), val,
                 *(volatile u32 *)(uintptr_t)(rec + 584));

    Sub_0800798C((void *)(uintptr_t)0x082C5040u, (void *)(uintptr_t)(rec + 16));
    *(volatile u32 *)(uintptr_t)(rec + 596) = Sub_0800572C(14);
    // Pinned-r8 window around the rec+600 pointer: keeps local-alloc off r8 so
    // the address lands in r9 as in the ROM.  Emits no instructions.
    __asm__ volatile ("" : "=r"(ghost8));
    r9v = (volatile u32 *)(uintptr_t)(rec + 600);
    *r9v = 5;
    __asm__ volatile ("" : : "r"(ghost8));
    Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 20), 5,
                 *(volatile u32 *)(uintptr_t)(rec + 596));

    Sub_0800798C((void *)(uintptr_t)0x082D0DC0u, (void *)(uintptr_t)(rec + 24));
    *(volatile u32 *)(uintptr_t)(rec + 608) = Sub_0800572C(7);
    r8v = (volatile u32 *)(uintptr_t)(rec + 612);
    *r8v = 50;
    Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 28), 50,
                 *(volatile u32 *)(uintptr_t)(rec + 608));

    *(volatile u32 *)(uintptr_t)(rec + 32) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 36) = 1;
    Sub_0800D77C((void *)(uintptr_t)(rec + 52), 0, -32);
    Sub_0800D77C((void *)(uintptr_t)(rec + 44), 0, 160);
    *(volatile s16 *)(uintptr_t)(rec + 40) = 6;
    *(volatile s16 *)(uintptr_t)(rec + 42) = 5;

    { volatile u8 *p3 = rec + 580;
      r6v = (u32)(uintptr_t)(rec + 174);
      Sub_0800D9A4((void *)(uintptr_t)p3, (int)*(s16 *)(uintptr_t)r6v,
                   (int)*(s16 *)(uintptr_t)(rec + 172)); }
    val = (u32)Sub_08024C3C((int)*(s16 *)(uintptr_t)r6v);
    *r9v = val;
    Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 20), val,
                 *(volatile u32 *)(uintptr_t)(rec + 596));
    val = (u32)Sub_08024C58((int)*(s16 *)(uintptr_t)r6v);
    *r8v = val;
    Sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 28), val,
                 *(volatile u32 *)(uintptr_t)(rec + 608));

    dma = (volatile u32 *)0x040000D4u;
    r5v = 0x080CCEECu;
    FF78_DMA_STEP(1, 0x05000282u);
    FF78_DMA_STEP(2, 0x05000284u);
    FF78_DMA_STEP(3, 0x05000286u);
    FF78_DMA_STEP(4, 0x05000288u);
    FF78_DMA_STEP(5, 0x0500028Au);
}

void _080013108(void *a) __attribute__((alias("MenuFF78_13108")));
void Sub_080013108(void *a) __attribute__((alias("MenuFF78_13108")));
void sub_080013108(void *a) __attribute__((alias("MenuFF78_13108")));
