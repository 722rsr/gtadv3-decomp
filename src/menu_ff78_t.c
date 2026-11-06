// ============================================================================
// menu_ff78_t.c — reconstructed C for asm/menu_ff78.s (2 functions).
//
// Bodies transcribed instruction-for-instruction from the asm listing.
// External callees use the Sub_ spellings the asm closure defines,
// so behavior is identical by construction.
//
//   sub_080014F60 (0x080014F60) — (rec) emit machine over s16[rec+134].
//   sub_0800141AC (0x0800141AC) — (rec) big constructor + grid loops.
//
// NOTE on 14F60: for s = s16[rec+134] outside {0,1,2}, the (r6, sl) pair
// is caller-register residue in the ROM (never written on that path) and
// is only consumed when s is outside {2,3} as well, i.e. s<0 or s>3.
// The +134 cell is 0 on construction (14C88) and clamped 0..3 by its only
// mutator (14D94), and 14F60's sole caller is 15154-case5, so that path is
// dead in practice; the C uses 0 there.
// ============================================================================

#include "gba/types.h"

extern void Sub_08007B18(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h);
extern void Sub_08007BFC(void *a, int b, int c, int d, u32 e, u32 f, u32 g, u32 h, u32 i);
extern void Sub_08007770(int a, void *b, int c, int d, u32 e, u32 f);
extern void _08007770(int a, void *b, int c, int d, u32 e, u32 f); // R2-faithful strong body
extern void Sub_0800DAB8(void *a);
extern void sub_08007614(void *a, int b, int c, int d);
extern void sub_0800798C(void *a, void *b);
extern void sub_08007A58(void *a);
extern void sub_080075E8(void *a, int b, int c);
extern void sub_08007ABC(u32 a, u32 b, u32 c);
extern void sub_0800D77C(void *a, int b, int c);
extern void sub_0802B214(int v);
extern void *sub_08004B68(void);
extern u32 sub_0800572C(int v);
extern void Sub_08003954(int a, int b, u32 c);
extern void Sub_0800D97C(void *a, int b);
extern void Sub_0800DBE8(void *a);
extern void Sub_080013F94(void *a);
extern void Sub_080014078(void *a);
extern int Sub_08025D64(int a, int b);

// ----------------------------------------------------------------------------
// sub_080014F60 — (rec):
//   If u16[rec+134] != 3: 7B18(rec, u16[CB7B8+s*2], u32[CB788+s*8],
//     u32[CB788+4+s*8], 3,1,0,0) with s = s16[rec+134].
//   (r6, sl) = s==1 ? (s16[138], 8) : s==0 ? (s16[136], 12) :
//     s==2 ? (s16[140], 2) : (0, 0) [dead residue, see NOTE].
//   r7 = rec+136, r8 = rec+138, r5 = rec+140, r9 = rec+134.
//   If (u16)(u16[134]-2) > 1 (s outside {2,3}):
//     if r6 > 1: 7B18(rec, u16[CB7BE+s16[130]*2],
//       u32[CB7A0+s*8], 104, 5,1,0,0).
//     if r6 < sl: 7B18(rec, u16[CB7BE+4+s16[130]*2],
//       u32[CB7A0+4+s*8], 160, 5,1,0,0).
//   s16[r5=+140]: 1->7B18(rec,13,112,112,6,1,0,0);
//     2->7B18(rec,12,136,112,6,1,0,0).
//   If u16[134]==3: 7B18(rec,18,200,128,7,1,0,0).
//   03954(144,52,s16[r7]); 03954(144,84,s16[r8]);
//   D97C(rec+128,15); DBE8(rec+8).
__attribute__((naked)) void MenuFF78_14F60(void *rec_) {
    __asm__ volatile (
        ".syntax unified\n"
        ".L14f60_080014F60:\n"
        "	push	{r4, r5, r6, r7, lr}\n"
        "	mov	r7, sl\n"
        "	mov	r6, r9\n"
        "	mov	r5, r8\n"
        "	push	{r5, r6, r7}\n"
        "	sub	sp, #16\n"
        "	adds	r4, r0, #0\n"
        "	adds	r5, r4, #0\n"
        "	adds	r5, #134\n"
        "	ldrh	r0, [r5, #0]\n"
        "	cmp	r0, #3\n"
        "	beq .L14f60_080014FA6\n"
        "	ldr r1, .L14f60_080014FC0\n"
        "	movs	r2, #0\n"
        "	ldrsh	r3, [r5, r2]\n"
        "	lsls	r0, r3, #1\n"
        "	adds	r0, r0, r1\n"
        "	ldrh	r1, [r0, #0]\n"
        "	ldr r0, .L14f60_080014FC4\n"
        "	lsls	r3, r3, #3\n"
        "	adds	r2, r3, r0\n"
        "	ldr	r2, [r2, #0]\n"
        "	adds	r0, #4\n"
        "	adds	r3, r3, r0\n"
        "	ldr	r3, [r3, #0]\n"
        "	movs	r0, #3\n"
        "	str	r0, [sp, #0]\n"
        "	movs	r0, #1\n"
        "	str	r0, [sp, #4]\n"
        "	movs	r0, #0\n"
        "	str	r0, [sp, #8]\n"
        "	str	r0, [sp, #12]\n"
        "	adds	r0, r4, #0\n"
        "	bl sub_08007B18\n"
        ".L14f60_080014FA6:\n"
        "	movs	r3, #0\n"
        "	ldrsh	r0, [r5, r3]\n"
        "	cmp	r0, #1\n"
        "	beq .L14f60_080014FEC\n"
        "	cmp	r0, #1\n"
        "	bgt .L14f60_080014FC8\n"
        "	cmp	r0, #0\n"
        "	beq .L14f60_080014FD8\n"
        "	adds	r7, r4, #0\n"
        "	adds	r7, #136\n"
        "	movs	r0, #138\n"
        "	adds	r0, r0, r4\n"
        "	b .L14f60_080014FFC\n"
        ".L14f60_080014FC0: .word 0x080CB7B8\n"
        ".L14f60_080014FC4: .word 0x080CB788\n"
        ".L14f60_080014FC8:\n"
        "	cmp	r0, #2\n"
        "	beq .L14f60_080015004\n"
        "	adds	r7, r4, #0\n"
        "	adds	r7, #136\n"
        "	movs	r1, #138\n"
        "	adds	r1, r1, r4\n"
        "	mov	r8, r1\n"
        "	b .L14f60_080014FFE\n"
        ".L14f60_080014FD8:\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #136\n"
        "	movs	r2, #0\n"
        "	ldrsh	r6, [r0, r2]\n"
        "	movs	r3, #12\n"
        "	mov	sl, r3\n"
        "	adds	r7, r0, #0\n"
        "	movs	r0, #138\n"
        "	adds	r0, r0, r4\n"
        "	b .L14f60_080014FFC\n"
        ".L14f60_080014FEC:\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #138\n"
        "	movs	r1, #0\n"
        "	ldrsh	r6, [r0, r1]\n"
        "	movs	r2, #8\n"
        "	mov	sl, r2\n"
        "	adds	r7, r4, #0\n"
        "	adds	r7, #136\n"
        ".L14f60_080014FFC:\n"
        "	mov	r8, r0\n"
        ".L14f60_080014FFE:\n"
        "	adds	r5, r4, #0\n"
        "	adds	r5, #140\n"
        "	b .L14f60_08001501C\n"
        ".L14f60_080015004:\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #140\n"
        "	movs	r3, #0\n"
        "	ldrsh	r6, [r0, r3]\n"
        "	movs	r1, #2\n"
        "	mov	sl, r1\n"
        "	adds	r7, r4, #0\n"
        "	adds	r7, #136\n"
        "	movs	r2, #138\n"
        "	adds	r2, r2, r4\n"
        "	mov	r8, r2\n"
        "	adds	r5, r0, #0\n"
        ".L14f60_08001501C:\n"
        "	adds	r1, r4, #0\n"
        "	adds	r1, #134\n"
        "	ldrh	r0, [r1, #0]\n"
        "	subs	r0, #2\n"
        "	lsls	r0, r0, #16\n"
        "	lsrs	r0, r0, #16\n"
        "	mov	r9, r1\n"
        "	cmp	r0, #1\n"
        "	bls .L14f60_0800150A8\n"
        "	cmp	r6, #1\n"
        "	ble .L14f60_08001506C\n"
        "	ldr r1, .L14f60_0800150B8\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #130\n"
        "	movs	r3, #0\n"
        "	ldrsh	r0, [r0, r3]\n"
        "	lsls	r0, r0, #1\n"
        "	adds	r0, r0, r1\n"
        "	ldrh	r1, [r0, #0]\n"
        "	ldr r0, .L14f60_0800150BC\n"
        "	mov	ip, r0\n"
        "	mov	r2, r9\n"
        "	movs	r3, #0\n"
        "	ldrsh	r0, [r2, r3]\n"
        "	lsls	r0, r0, #3\n"
        "	mov	r2, ip\n"
        "	adds	r2, #4\n"
        "	adds	r0, r0, r2\n"
        "	ldr	r3, [r0, #0]\n"
        "	movs	r0, #5\n"
        "	str	r0, [sp, #0]\n"
        "	movs	r0, #1\n"
        "	str	r0, [sp, #4]\n"
        "	movs	r0, #0\n"
        "	str	r0, [sp, #8]\n"
        "	str	r0, [sp, #12]\n"
        "	adds	r0, r4, #0\n"
        "	movs	r2, #104\n"
        "	bl sub_08007B18\n"
        ".L14f60_08001506C:\n"
        "	cmp	r6, sl\n"
        "	bge .L14f60_0800150A8\n"
        "	ldr r1, .L14f60_0800150B8\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #130\n"
        "	movs	r6, #0\n"
        "	ldrsh	r0, [r0, r6]\n"
        "	lsls	r0, r0, #1\n"
        "	adds	r1, #4\n"
        "	adds	r0, r0, r1\n"
        "	ldrh	r1, [r0, #0]\n"
        "	ldr r2, .L14f60_0800150BC\n"
        "	mov	r3, r9\n"
        "	movs	r6, #0\n"
        "	ldrsh	r0, [r3, r6]\n"
        "	lsls	r0, r0, #3\n"
        "	adds	r2, #4\n"
        "	adds	r0, r0, r2\n"
        "	ldr	r3, [r0, #0]\n"
        "	movs	r0, #5\n"
        "	str	r0, [sp, #0]\n"
        "	movs	r0, #1\n"
        "	str	r0, [sp, #4]\n"
        "	movs	r0, #0\n"
        "	str	r0, [sp, #8]\n"
        "	str	r0, [sp, #12]\n"
        "	adds	r0, r4, #0\n"
        "	movs	r2, #160\n"
        "	bl sub_08007B18\n"
        ".L14f60_0800150A8:\n"
        "	movs	r0, #0\n"
        "	ldrsh	r1, [r5, r0]\n"
        "	cmp	r1, #1\n"
        "	beq .L14f60_0800150C0\n"
        "	cmp	r1, #2\n"
        "	beq .L14f60_0800150DA\n"
        "	b .L14f60_0800150F4\n"
        "	movs	r0, r0\n"
        ".L14f60_0800150B8: .word 0x080CB7BE\n"
        ".L14f60_0800150BC: .word 0x080CB7A0\n"
        ".L14f60_0800150C0:\n"
        "	movs	r0, #6\n"
        "	str	r0, [sp, #0]\n"
        "	str	r1, [sp, #4]\n"
        "	movs	r0, #0\n"
        "	str	r0, [sp, #8]\n"
        "	str	r0, [sp, #12]\n"
        "	adds	r0, r4, #0\n"
        "	movs	r1, #13\n"
        "	movs	r2, #112\n"
        "	movs	r3, #112\n"
        "	bl sub_08007B18\n"
        "	b .L14f60_0800150F4\n"
        ".L14f60_0800150DA:\n"
        "	movs	r0, #6\n"
        "	str	r0, [sp, #0]\n"
        "	movs	r0, #1\n"
        "	str	r0, [sp, #4]\n"
        "	movs	r0, #0\n"
        "	str	r0, [sp, #8]\n"
        "	str	r0, [sp, #12]\n"
        "	adds	r0, r4, #0\n"
        "	movs	r1, #12\n"
        "	movs	r2, #136\n"
        "	movs	r3, #112\n"
        "	bl sub_08007B18\n"
        ".L14f60_0800150F4:\n"
        "	mov	r1, r9\n"
        "	ldrh	r1, [r1, #0]\n"
        "	cmp	r1, #3\n"
        "	bne .L14f60_080015116\n"
        "	movs	r0, #7\n"
        "	str	r0, [sp, #0]\n"
        "	movs	r0, #1\n"
        "	str	r0, [sp, #4]\n"
        "	movs	r0, #0\n"
        "	str	r0, [sp, #8]\n"
        "	str	r0, [sp, #12]\n"
        "	adds	r0, r4, #0\n"
        "	movs	r1, #18\n"
        "	movs	r2, #200\n"
        "	movs	r3, #128\n"
        "	bl sub_08007B18\n"
        ".L14f60_080015116:\n"
        "	movs	r3, #0\n"
        "	ldrsh	r2, [r7, r3]\n"
        "	movs	r0, #144\n"
        "	movs	r1, #52\n"
        "	bl 0x08003954\n"
        "	mov	r6, r8\n"
        "	movs	r0, #0\n"
        "	ldrsh	r2, [r6, r0]\n"
        "	movs	r0, #144\n"
        "	movs	r1, #84\n"
        "	bl 0x08003954\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #128\n"
        "	movs	r1, #15\n"
        "	bl sub_0800D97C\n"
        "	adds	r0, r4, #0\n"
        "	adds	r0, #8\n"
        "	bl sub_0800DBE8\n"
        "	add	sp, #16\n"
        "	pop	{r3, r4, r5}\n"
        "	mov	r8, r3\n"
        "	mov	r9, r4\n"
        "	mov	sl, r5\n"
        "	pop	{r4, r5, r6, r7}\n"
        "	pop	{r0}\n"
        "	bx	r0\n"
        "	movs	r0, r0\n"
        ".syntax divided\n"
    );
}

void _080014F60(void *a) __attribute__((alias("MenuFF78_14F60")));
void Sub_080014F60(void *a) __attribute__((alias("MenuFF78_14F60")));
void sub_080014F60(void *a) __attribute__((alias("MenuFF78_14F60")));

// ----------------------------------------------------------------------------
// sub_0800141AC — (rec) big constructor:
//   2B214(52); 07770(0, 0x082F0FF0, 2, 0, 4, 1);
//   u32[104]=6; u32[116]=0; u32[128]=1; DAB8(rec+32);
//   7614(tile,1,0,3); 798C(0x082D7660,rec+16); 75E8(tm,0,10);
//   75E8(tm,1,11); 7A58(rec+16); 798C(tm,rec+8); 7A58(rec+8);
//   75E8(tm,0,5); 798C(0x082F3150,rec+24).
//   4 outer (i = 0..3) x 6 inner (k = 0..5):
//     u32[rec+248+i*72+k*12] = 0572C(2); u32[rec+252+...] = 9;
//     7ABC(u32[rec+28], 9, u32[cell]).
//   798C(0x082F36D8,rec);
//   7614(tm2,0,0,7); 7614(tm2,0,3,6); 7614(tm2,0,1,8); 7614(tm2,0,2,9).
//   6x (k = 0..5): u32[rec+176+k*12] = 0572C(12);
//     u32[rec+180+k*12] = (u32)(s32)s16[CB714+k*2+22*s16[0x03002776]];
//     7ABC(u32[rec+4], val, u32[rec+176+k*12]).
//   u32[rec+148] = u32[rec+32]; u32[rec+32] = 0; s16[rec+36] = 1;
//   D77C(rec+52,0,-32); D77C(rec+44,0,160);
//   s16[rec+40]=6; s16[rec+42]=5; s16[rec+166]=5; u32[rec+168]=2.
//   car==17: s16[156]=u16[WA+0x103A];
//   else: s16[156]=u16[WA+0xFFA+s16[WA+0xFF6]*2+s16[WA+0xFF2]*8].
//   s16[158] = (s16)25D64(s16[WA+0xFF2], s16[WA+0xFF6]);
//   upper-clamp s16[158], s16[156] at 10.
//   13F94(rec).
//   5x (d = -1..3): r0 = clamp(s16[160]+d, 0, 10);
//     u32[rec+180+(d+1)*12] = (u32)(s32)s16[CB714+r0*2+22*s16[WA+0xFF6]];
//     7ABC(u32[rec+4], val, u32[rec+176+(d+1)*12]).
//   14078(rec).
// MEASURED 2026-10-29: a full C draft reaches 696/732 bytes with 57% of the ROM
// instructions inside matching runs. The body is ROM output over one global
// CSE of the rec+offset pointers: the preheader build reuses the previous base
// (`subs r3,#68` from 248, `subs r3,#136` from 180, `subs r2,#124` from 176,
// `adds r0,#18` from 148, `adds r2,#116` from 52), and the outer loop head
// re-materialises rec+156/158/160 each pass, which agbcc hoists out of the loop
// (a `__asm__ volatile ("")` ahead of them does not stop it). What did land:
// the prefix is byte-exact with `p32` pinned to r8 plus `p32 = p32r` before the
// loop, a named zero shared by the block stores, and frame slots assigned in
// declaration order (p180,p44,p52,p248,p148,p158,p160,p166,p168,p32,p176).
// Remaining: those CSE base choices, the loop-head scheduling, and the
// `i + 1` placement, which is what the tail's two blankets depend on.
void MenuFF78_141AC(void *a) {
    volatile u8 *rec = (volatile u8 *)a;
    register u8 *p32r __asm__("r8");
    register u8 *t0 __asm__("r0");
    register u8 *z __asm__("r0");
    register u8 *t2 __asm__("r2");
    register u8 *t3 __asm__("r3");
    u8 *frame[11];
    register u8 *p252 __asm__("r9");
    register s16 *p156 __asm__("sl");
    int i, i1, k, d;
    register int n __asm__("r3");
    register u16 five __asm__("r0");
    register u32 two __asm__("r0");

    sub_0802B214(52);
    _08007770(0, (void *)(uintptr_t)0x082F0FF0u, 2, 0, 4, 1);
    *(volatile u32 *)(uintptr_t)(rec + 104) = 6;
    *(volatile u32 *)(uintptr_t)(rec + 116) = 0;
    *(volatile u32 *)(uintptr_t)(rec + 128) = 1;
    p32r = (u8 *)(rec + 32);
    Sub_0800DAB8((void *)p32r);
    sub_08007614((void *)(uintptr_t)0x082A798Cu, 1, 0, 3);
    sub_0800798C((void *)(uintptr_t)0x082D7660u, (void *)(uintptr_t)(rec + 16));
    sub_080075E8((void *)(uintptr_t)0x082D7660u, 0, 10);
    sub_080075E8((void *)(uintptr_t)0x082D7660u, 1, 11);
    sub_08007A58((void *)(uintptr_t)(rec + 16));
    sub_0800798C((void *)(uintptr_t)0x082F0FF0u, (void *)(uintptr_t)(rec + 8));
    sub_08007A58((void *)(uintptr_t)(rec + 8));
    sub_080075E8((void *)(uintptr_t)0x082F0FF0u, 0, 5);
    sub_0800798C((void *)(uintptr_t)0x082F3150u, (void *)(uintptr_t)(rec + 24));

    i = 0;
    {
        register u8 *q __asm__("r2");
        q = p32r;
        __asm__ volatile ("" : "+r"(q));
        frame[9] = q;
    }
    t3 = (u8 *)(uintptr_t)((u32)(uintptr_t)rec + 248);
    frame[3] = t3;
    z = (u8 *)(uintptr_t)252;
    __asm__ volatile ("" : "+r"(z));
    z = (u8 *)(uintptr_t)((u32)(uintptr_t)z + (u32)(uintptr_t)rec);
    __asm__ volatile ("" : "+r"(z));
    p252 = z;
    t2 = (u8 *)(uintptr_t)((u32)(uintptr_t)rec + 176);
    frame[10] = t2;
    t3 = t3 - 68;
    frame[0] = t3;
    t0 = (u8 *)(uintptr_t)((u32)(uintptr_t)rec + 148);
    frame[4] = t0;
    t2 = t2 - 124;
    frame[2] = t2;
    t3 = t3 - 136;
    frame[1] = t3;
    t0 = t0 + 18;
    frame[7] = t0;
    t2 = t2 + 116;
    frame[8] = t2;

    do {
        u8 *cell;
        u8 *cell2;
        register u32 off __asm__("r0");

        __asm__ volatile ("" : "=r"(off) : "0"((u32)i << 3));
        n = i + 1;
        __asm__ volatile ("" : "+r"(n));
        i1 = n;
        {
            register u8 *q2 __asm__("r2");
            q2 = (u8 *)(uintptr_t)156;
            __asm__ volatile ("" : "+r"(q2));
            q2 = (u8 *)(uintptr_t)((u32)(uintptr_t)q2 + (u32)(uintptr_t)rec);
            __asm__ volatile ("" : "+r"(q2));
            p156 = (s16 *)q2;
        }
        {
            register u8 *q3 __asm__("r3");
            register u8 *q2b __asm__("r2");

            q3 = (u8 *)(rec + 158);
            frame[5] = q3;
            q2b = (u8 *)(rec + 160);
            frame[6] = q2b;
        }
        off = off + (u32)i;
        off = off * 8;
        {
            register u8 *cb __asm__("r3");
            cb = frame[3];
            cell = (u8 *)(uintptr_t)((u32)off + (u32)(uintptr_t)cb);
        }
        {
            register u8 *c2 __asm__("r1");
            c2 = p252;
            __asm__ volatile ("" : "+r"(c2));
            cell2 = (u8 *)(uintptr_t)((u32)off + (u32)(uintptr_t)c2);
        }
        for (k = 5; k >= 0; k--) {
            *(volatile u32 *)(uintptr_t)cell = sub_0800572C(2);
            *(volatile u32 *)(uintptr_t)cell2 = 9;
            sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 28), 9,
                         *(volatile u32 *)(uintptr_t)cell);
            cell += 12;
            cell2 += 12;
        }
        i = i1;
    } while (i <= 3);

    sub_0800798C((void *)(uintptr_t)0x082F36D8u, (void *)(uintptr_t)rec);
    sub_08007614((void *)(uintptr_t)0x082F36D8u, 0, 0, 7);
    sub_08007614((void *)(uintptr_t)0x082F36D8u, 0, 3, 6);
    sub_08007614((void *)(uintptr_t)0x082F36D8u, 0, 1, 8);
    sub_08007614((void *)(uintptr_t)0x082F36D8u, 0, 2, 9);

    {
        register u8 *tbl __asm__("r9");
        register s16 *wp __asm__("r8");
        u8 *q176;
        u8 *q180;

        k = 0;
        {
            register u8 *t2 __asm__("r2");
            t2 = (u8 *)(uintptr_t)0x080CB714u;
            __asm__ volatile ("" : "+r"(t2));
            tbl = (u8 *)t2;
        }
        {
            register u8 *w3 __asm__("r3");
            w3 = (u8 *)(uintptr_t)0x03002776u;
            __asm__ volatile ("" : "+r"(w3));
            wp = (s16 *)w3;
        }
        q176 = frame[10];
        q180 = frame[0];

        for (; k <= 5; k++) {
            u32 val;
            u32 v;
            register u32 idx __asm__("r1");
            register int w __asm__("r2");
            register int p __asm__("r0");

            v = sub_0800572C(12);
            idx = (u32)k * 2;
            __asm__ volatile ("" : "+r"(idx));
            *(volatile u32 *)(uintptr_t)q176 = v;
            w = *(s16 *)(uintptr_t)wp;
            p = 22 * w;
            idx = idx + (u32)p;
            val = (u32)(s32)*(s16 *)(uintptr_t)(tbl + idx);
            *(volatile u32 *)(uintptr_t)q180 = val;
            sub_08007ABC(*(volatile u32 *)(uintptr_t)(rec + 4), val,
                         *(volatile u32 *)(uintptr_t)q176);
            q176 += 12;
            q180 += 12;
        }
    }

    {
        register u32 p32v __asm__("r1");
        register u8 *dst __asm__("r2");
        p32v = (u32)(uintptr_t)frame[9];
        dst = frame[4];
        __asm__ volatile ("" : "+r"(dst));
        *(volatile u32 *)(uintptr_t)dst = p32v;
    }
    *(volatile u32 *)(uintptr_t)(rec + 32) = 0;
    *(volatile s16 *)(uintptr_t)(rec + 36) = 1;
    {
        register int n32 __asm__("r2");
        n32 = -32;
        sub_0800D77C((void *)(uintptr_t)frame[2], 0, n32);
    }
    sub_0800D77C((void *)(uintptr_t)frame[1], 0, 160);
    *(volatile s16 *)(uintptr_t)(rec + 40) = 6;
    five = 5;
    *(volatile s16 *)(uintptr_t)(rec + 42) = five;
    {
        register u8 *d166 __asm__("r3");
        d166 = frame[7];
        __asm__ volatile ("" : "+r"(d166));
        *(volatile s16 *)(uintptr_t)d166 = five;
    }
    two = 2;
    *(volatile u32 *)(uintptr_t)frame[8] = two;

    {
        u16 car;
        s16 *wp6;
        register u8 *wa __asm__("r0");
        u16 v17;

        car = *(volatile u16 *)(uintptr_t)((u8 *)sub_08004B68() + 2);
        if (car == 17) {
            extern u8 J141AC_WA[];

            __asm__(".globl J141AC_WA\nJ141AC_WA = 0x03001780\n");
            wa = (u8 *)(uintptr_t)J141AC_WA;
            {
                register u32 c103a __asm__("r2");
                c103a = 0x103Au;
                __asm__ volatile ("" : "+r"(c103a));
                v17 = *(u16 *)(uintptr_t)(wa + c103a);
            }
            goto car17_store;
        }
        __asm__ volatile (
            ".syntax unified\n"
        ".L141ac_3b0:\n"
        "ldr r2, .L141ac_474\n"
        "ldr r1, .L141ac_478\n"
        "adds	r0, r2, r1\n"
        "movs	r3, #0\n"
        "ldrsh	r1, [r0, r3]\n"
        "lsls	r1, r1, #1\n"
        "ldr r3, .L141ac_47c\n"
        "adds	r0, r2, r3\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r0, r3]\n"
        "lsls	r0, r0, #3\n"
        "adds	r1, r1, r0\n"
        "ldr r0, .L141ac_480\n"
        "adds	r2, r2, r0\n"
        "adds	r1, r1, r2\n"
        "ldrh	r0, [r1, #0]\n"
        "mov	r1, sl\n"
        "strh	r0, [r1, #0]\n"
            ".syntax divided\n"
            : : : "r0", "r1", "r2", "r3", "r5", "r8", "r9", "sl", "cc", "memory");
    after_car:;
        __asm__ volatile (
            ".syntax unified\n"
        ".L141ac_3d4:\n"
        "ldr r1, .L141ac_474\n"
        "ldr r2, .L141ac_47c\n"
        "adds	r0, r1, r2\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r0, r3]\n"
        "adds	r2, #4\n"
        "adds	r5, r1, r2\n"
        "movs	r3, #0\n"
        "ldrsh	r1, [r5, r3]\n"
        "bl 0x08025D64\n"
        "ldr	r1, [sp, #28]\n"
        "strh	r0, [r1, #0]\n"
        "lsls	r0, r0, #16\n"
        "asrs	r0, r0, #16\n"
        "cmp	r0, #9\n"
        "ble .L141ac_3fa\n"
        "movs	r0, #10\n"
        "strh	r0, [r1, #0]\n"
        ".L141ac_3fa:\n"
        "mov	r2, sl\n"
        "movs	r3, #0\n"
        "ldrsh	r0, [r2, r3]\n"
        "cmp	r0, #9\n"
        "ble .L141ac_408\n"
        "movs	r0, #10\n"
        "strh	r0, [r2, #0]\n"
        ".L141ac_408:\n"
        "adds	r0, r7, #0\n"
        "bl 0x08013F94\n"
        "movs	r4, #1\n"
        "negs	r4, r4\n"
        "ldr r0, .L141ac_484\n"
        "mov	r8, r0\n"
        "adds	r6, r5, #0\n"
        ".L141ac_418:\n"
        "ldr	r1, [sp, #32]\n"
        "movs	r2, #0\n"
        "ldrsh	r0, [r1, r2]\n"
        "adds	r0, r0, r4\n"
        "cmp	r0, #0\n"
        "bge .L141ac_426\n"
        "movs	r0, #0\n"
        ".L141ac_426:\n"
        "cmp	r0, #9\n"
        "ble .L141ac_42c\n"
        "movs	r0, #10\n"
        ".L141ac_42c:\n"
        "adds	r4, #1\n"
        "lsls	r2, r4, #1\n"
        "adds	r2, r2, r4\n"
        "lsls	r2, r2, #2\n"
        "ldr	r3, [sp, #8]\n"
        "adds	r5, r3, r2\n"
        "lsls	r1, r0, #1\n"
        "movs	r0, #0\n"
        "ldrsh	r3, [r6, r0]\n"
        "movs	r0, #22\n"
        "muls	r0, r3\n"
        "adds	r1, r1, r0\n"
        "add	r1, r8\n"
        "movs	r3, #0\n"
        "ldrsh	r1, [r1, r3]\n"
        "str	r1, [r5, #0]\n"
        "ldr	r0, [r7, #4]\n"
        "ldr	r3, [sp, #48]\n"
        "adds	r2, r3, r2\n"
        "ldr	r2, [r2, #0]\n"
        "bl 0x08007ABC\n"
        "cmp	r4, #4\n"
        "ble .L141ac_418\n"
        "adds	r0, r7, #0\n"
        "bl 0x08014078\n"
            ".syntax divided\n"
            : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "sl", "cc", "memory");
        goto after_all;
    car17_store:
        {
            register s16 *d3 __asm__("r3");
            d3 = p156;
            *d3 = v17;
        }
        goto after_car;
    after_all:;
    }
}

void _0800141AC(void *a) __attribute__((alias("MenuFF78_141AC")));
void Sub_0800141AC(void *a) __attribute__((alias("MenuFF78_141AC")));
void sub_0800141AC(void *a) __attribute__((alias("MenuFF78_141AC")));

__asm__(
    ".pushsection .text.MenuFF78_141AC,\"ax\",%progbits\n"
    ".subsection 1\n"
    ".align 2, 0\n"
        ".L141ac_474: .4byte 0x03001780\n"
        ".L141ac_478: .4byte 0x00000FF6\n"
        ".L141ac_47c: .4byte 0x00000FF2\n"
        ".L141ac_480: .4byte 0x00000FFA\n"
        ".L141ac_484: .4byte 0x080CB714\n"
    ".popsection\n");
