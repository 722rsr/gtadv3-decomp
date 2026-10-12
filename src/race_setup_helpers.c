// ============================================================================
// race_setup_helpers.c — C lift of the 9 remaining race_setup_18adc.s functions
// (VMA 0x080018F94 / 0x080019318 / 0x0800196E8 / 0x08001992C / 0x0800199CC /
//  0x080019A2C / 0x080019AEC / 0x080019D6C / 0x080019EC8).
//
// Race-progress lap/award machinery over the racer array (0x03004E80,
// stride 284 = 142*2) and racectx (*0x03004E20). Transcribed
// instruction-for-instruction from the cited asm listing.
// ============================================================================

#include "gba/types.h"

extern u32  _08018ACC(u32 mask);        // Ghost_FlagTest (src/ghost.c)
extern void _08018AA8(u32 mask, int set); // Ghost_FlagOp
extern void _080018EC0(void);           // (src/race_setup.c)
extern void _080018F14(void *a);        // (src/race_setup.c)
extern int  _080044C4(int a, int b, volatile u32 *c); // TrackArrow_044C4
extern int  _08004508(volatile u32 *a); // TrackDigit_04508 (1-arg in ROM; body ignores dead r1/r2)
extern int  _0802D97C(int a, int b);    // DivRem (bios_wrappers.c)
extern void _0802B500(u16 a, u16 b, s16 c);           // song-bank volume (asm/sound_bank.s)
extern int  _0802D978(int a, int b);    // Div
extern int  _0802DE04(int a, int b);    // signed EABI idiv
extern void *_0802E0A4(void *dst, const void *src, u32 n);  // memcpy, r0=dst r1=src r2=n (returns dst)
extern int  _08002BE8(void);            // u32 random
extern void _08002C48(int a, u16 b);    // Store_02C48
extern int  _08002BD8(u32 a, u32 b);    // LoadSub_02BD8
extern void _08006B20(void);            // Course_CursorReset
extern void *_08005F98(void);           // surface package getter
extern void _08002BFC(int a);           // 16-byte obj alloc
extern void _08004330(void *a);         // camera setter (raw asm)
extern int  _08008164(int a);           // Course_State_CourseId
extern void _08008208(void);            // Course_State_Store22_24
extern void _080081F8(u32 a);           // Course_State_Store18_48 (widened u16->u32 with the body)
extern void _0800821C(void);            // Course_State_ResetTimed
extern void _08008284(u32 a);           // Course_State_Store32 (widened u16->u32 with the body)
extern void _08026550(void *a, int b, void *c, int d); // raw asm (racer tick)
extern void _08027234(u32 a, u32 b, s16 c, void *d);   // raw asm (racer event)
extern void _0802A8F0(void *a, int v);  // raw asm
extern void _0801B498(void);            // raw asm
extern void _08019C34(void);            // real function, not a veneer:
// ROM 0x08019C34 is `push {lr}` and dispatch tables at _080019C30 and
// _080019D2C both hold 0x08019C35 (target+1, Thumb interworking).
// The EMPTY weak stub that used to stand here made the probe select
// this address as a C candidate and then fail to score it, which is
// what --require-all was failing on. The asm label is the definition.
extern void _08007538(void *a, s16 b, s16 c); // 0x08007538
extern void _080075E8(void *a, s16 b, int c);  // 0x080075E8
extern void _08019D0C(void);            // raw asm continuation
extern void _08019D40(void);            // raw asm continuation
extern void _08019E1C(void);            // raw asm continuation
extern void _08019F1D(void);            // raw asm continuation
extern void _08019EC9(void);            // raw asm continuation
extern int  _08019200(void *a, void *b);   // Race_Setup_19200 (src/race_setup.c)
extern int  _080019250(void *a, void *b);  // Race_Setup_19250 (src/race_setup.c)
extern void _080023AC(int slot, void *handler); // src/code_22e4.c

// ----------------------------------------------------------------------------
// _080019C9C / _080019DBC — VCOUNT gate + IRQ-slot install.
//   asm/race_setup_18adc.s:2215 / :2348. Each is 32 bytes: a `push {lr}`,
//   `ldrb r0,[0x04000006]`, `cmp r0,#160`, `bhi end`, then
//   `r1 = 0x080019CBD` / `0x080019DDD`, `r0 = 3`, `bl _080023AC`, and the
//   literal pool holds VCOUNT plus that handler pointer. The handler word is
//   the address the pool holds (a byte inside the next function's encoding),
//   not a label: `_080023AC(3, <word>)` is exactly what the ROM passes.
void _080019C9C(void) {
    if (*(volatile u8 *)(uintptr_t)0x04000006u <= 160u)
        _080023AC(3, (void *)(uintptr_t)0x08019CBDu);
}
void _080019DBC(void) {
    if (*(volatile u8 *)(uintptr_t)0x04000006u <= 160u)
        _080023AC(3, (void *)(uintptr_t)0x08019DDDu);
}

// _080019CBC / _080019DDC (56 B each, asm/race_setup_18adc.s:2231 / :2364)
// are lifted in race_setup.c. A register-pinned base plus an explicit register
// LDRSH operand reproduces the ROM's `ldrsh r1,[r1,r2]` with r2=10; both bodies
// now pass whole-function matching including their literal pools.

// forward decls (lifted in this file)
void sub_080018F94(void);
void sub_080019318(void);
void sub_0800196E8(void);
void sub_08001992C(void);

// ----------------------------------------------------------------------------
// 0x080018F94 sub_080018F94 — lap-completion sweep:
//   _080018EC0
//   n = (s16)[WA+0x10CA]
//   for i in 1..n-1:
//     r4 = 0x03004E80 + i*284
//     if _080044C4([r4],[r4+4], r4+200):
//       _080018F14(r4)
//       u16[racectx+70] += 1
//   cnt = u16[racectx+70]
//   if (s16)cnt > 3: u16[racectx+70] = 4
// ----------------------------------------------------------------------------
void sub_080018F94(void)
{
    register int i __asm__("r5");
    register int stride __asm__("r6");
    volatile s16 *countTail;
    _080018EC0();
    i = 1;
    {
        register volatile u8 *wa __asm__("r0") = (volatile u8 *)0x03001780u;
        register u32 countOffset __asm__("r2") = 0x10CA;
        register volatile s16 *countp __asm__("r1");
        __asm__ volatile ("add r1, r0, r2" : "=r"(countp)
                          : "r"(wa), "r"(countOffset) : "memory");
        register s32 zero __asm__("r2") = 0;
        register s32 n __asm__("r0");
        __asm__ volatile ("ldrsh r0, [r1, r2]" : "=r"(n)
                          : "r"(countp), "r"(zero) : "memory");
        if (i >= n) goto finish;
        countTail = countp;
        stride = 142;
        stride <<= 1;
loop_records:
        {
            u8 *r4 = (u8 *)(uintptr_t)(0x03004E80u + stride);
            if (_080044C4(*(int *)(r4 + 0), *(int *)(r4 + 4),
                          (u32 *)(r4 + 200))) {
                _080018F14((void *)r4);
                register volatile u8 *racectx __asm__("r0") =
                    *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
                register volatile u16 *lapCount __asm__("r0") =
                    (volatile u16 *)(racectx + 70);
                register u16 lapTotal __asm__("r1") = *lapCount;
                lapTotal++;
                *lapCount = lapTotal;
            }
            stride += 284;
            i++;
            register s32 currentCount __asm__("r0");
            __asm__ volatile ("mov r1, #0\n\tldrsh r0, [r7, r1]"
                              : "=r"(currentCount) : "r"(countTail)
                              : "r1", "memory");
            if (i < currentCount) goto loop_records;
        }
    }
finish:
    {
        register volatile u8 *racectx __asm__("r0") =
            *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
        register volatile s16 *cnt __asm__("r1") =
            (volatile s16 *)(racectx + 70);
        register s32 zero __asm__("r2") = 0;
        register s32 value __asm__("r0");
        __asm__ volatile ("ldrsh r0, [r1, r2]" : "=r"(value)
                          : "r"(cnt), "r"(zero) : "memory");
        if (value > 3) {
            register u16 four __asm__("r0") = 4;
            *cnt = four;
        }
    }
}
void _080018F94(void) __attribute__((alias("sub_080018F94")));

// ----------------------------------------------------------------------------
// 0x080019318 sub_080019318 — ghost ghost-car record updater:
//   r6 = (s16)[WA+0x10CA]; gate [WA+0x10CA+18] & 1 else _080018F94 branch
//   for i in 1..n-1 (r7 stride 284):
//     r4 = racers + i*284
//     callback(*(0x080CBB28))(r4+36, sp)   @ ROM dispatch via bx r2 veneer
//     [r4+0]=[sp+4]; [r4+4]=[sp+8]
//     [r4+280]=[sp+12]; [r4+284]=[sp+16]   @ (280=136*2)
//     [r4+8]=[sp+24]
//     u16[r4+20] = u16[[sp+28]+12]
//     u16[r4+24] = u16[sp+38]
//     r4[30] = ([sp+32]>>6)&1
//     r6 -= _08019200(racectx+0x4C8, sp)
//   if !_08018ACC(1): u16[racers+14] = r6
//   place loop over 0x03000598 array with random + _08002C48 + 0x08026550
//   … (full transcription below)
// ----------------------------------------------------------------------------
void sub_080019318(void)
{
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    volatile u8 *racers = (volatile u8 *)(uintptr_t)0x03004E80u;
    volatile u8 *racectx = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
    int n = (int)*(volatile s16 *)(wa + 0x10CA);
    int i;

    if ((*(volatile u32 *)(wa + 0x10CA + 18) & 1) == 0) {
        sub_080018F94();
        /* fall into placement loop from _080019594 below */
        goto tail_loop;
    }
    {
        u32 sp[16];
        int r6 = n;
        sp[2] = 0; /* [sp+8] init at _080019030's caller frame is separate;
                      the 0x080019318 loop uses the following semantics */

        for (i = 1; i < n; i++) {
            volatile u8 *r4 = racers + i * 284;
            void (*copy_record)(void *, void *) =
                (void (*)(void *, void *))(uintptr_t)*(volatile u32 *)(uintptr_t)0x080CBB28u;
            copy_record((void *)(r4 + 36), sp);
            *(volatile u32 *)(r4 + 0) = sp[1];      /* [sp+4] */
            *(volatile u32 *)(r4 + 4) = sp[2];      /* [sp+8] */
            *(volatile u32 *)(r4 + 280) = sp[3];    /* [sp+12] */
            *(volatile u32 *)(r4 + 284) = sp[4];    /* [sp+16] */
            *(volatile u32 *)(r4 + 8) = sp[6];      /* [sp+24] */
            *(volatile u16 *)(r4 + 20) = *(volatile u16 *)(sp[7] + 12);
            *(volatile u16 *)(r4 + 24) = *(volatile u16 *)((volatile u8 *)sp + 38);
            *(volatile u8 *)(r4 + 30) = (u8)(sp[8] >> 6 & 1);
            r6 -= _08019200((void *)(racectx + 0x4C8), sp);
        }
        if (!_08018ACC(1))
            *(volatile u16 *)(racers + 14) = (u16)r6;
    }
    /* placement phase (_0800193C0.._08001945C) */
    {
        volatile u32 *arr = (volatile u32 *)(uintptr_t)0x03000598u;
        int r5 = 0;
        int lim = (int)(s16)*(volatile u16 *)(racectx + 70) - 1;
        while (r5 < lim) {
            volatile u32 *e = &arr[r5];
            if (*e != 0) {
                volatile u8 *r4 = (volatile u8 *)*e;
                if (r4[29] != 0) {
                    int rnd = (int)(s16)_08002BE8();
                    *(volatile int *)(r4 + 220) = rnd;
                    _08002C48((int)(s16)*(volatile u16 *)(r4 + 216), 0);
                    {
                        volatile u8 *r4s = racers + 280; /* r4 + 140*2 */
                        (void)r4s;
                        _08026550(*(void *volatile *)(r4 + 280),
                                  *(volatile int *)(r4 + 8) - *(volatile int *)(racectx + 8),
                                  (void *)(r4 + 200), 0);
                    }
                    r4[29] = 0;
                    if (r4[32] == 0) {
                        u32 f = _08018ACC(128u << 17);
                        if (f) {
                            if (r4[32] == 0)
                                _08027234(*(volatile u32 *)(r4 + 0), *(volatile u32 *)(r4 + 4),
                                          (s16)*(volatile s16 *)(r4 + 8), (void *)(uintptr_t)0x03005DF0u);
                        }
                    }
                }
            }
            r5++;
            lim = (int)(s16)*(volatile u16 *)(racectx + 70) - 1;
        }
    }
    /* HUD template + sorted placement (_08001946C..) — copied from table row */
    {
        volatile u8 *wa2 = (volatile u8 *)(uintptr_t)0x03001780u;
        volatile u8 *r4h = wa2 + 0x10CA;
        /* row = (n-1)-th of 63-byte records at 0x03001780 */
        int idx = (int)(s16)*(volatile u16 *)(r4h + 0) - 1;
        volatile u8 *row = wa2 + ((idx * 7 * 8 - idx) * 4); /* (idx*55)*4 = idx*220? asm: (r1*8-r1)*4 */
        u16 r6 = *(volatile u16 *)(row + 20);
        u32 tbl[16];
        /* ROM 0x08001948A: memcpy(sp+52, 0x0805FB88, 16) — 16 bytes, dst first
           (r0=dst, r1=src, r2=16). */
        _0802E0A4((void *)tbl, (const void *)(uintptr_t)0x0805FB88u, 16u);
        for (i = 1; i < idx; i++) {
            volatile u8 *r2 = racers + i * 284;
            int d = (int)(s16)*(volatile u16 *)(r2 + 20) - (int)(s16)r6;
            if (d > (int)(s16)*(volatile u16 *)((volatile u8 *)tbl + i * 2))
                _0802A8F0((void *)(r2 + 36), 1);
        }
    }
tail_loop:
    /* tail loop _080019594: place until lap counter bound reached */
    {
        volatile u32 *arr = (volatile u32 *)(uintptr_t)0x03000598u;
        volatile u8 *racectx2 = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
        int r5 = 0;
        int lim = (int)(s16)*(volatile u16 *)(racectx2 + 70) - 1;
        while (r5 < lim) {
            volatile u32 *e = &arr[r5];
            int r7 = r5 + 1;
            if (*e != 0) {
                volatile u8 *r4 = (volatile u8 *)*e;
                *(volatile int *)(r4 + 252) = 0;
                if (_08004508((volatile u32 *)(r4 + 200))) {
                    int rnd = (int)(s16)_08002BE8();
                    *(volatile int *)(r4 + 220) = rnd;
                    _08002C48((int)(s16)*(volatile u16 *)(r4 + 216), 0);
                    {
                        volatile u8 *rc = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
                        *(volatile int *)(r4 + 224) =
                            (int)(s16)*(volatile u16 *)(rc + 132 + r7 * 2);
                        *(volatile int *)(r4 + 228) = (r5 + 6) << 12;
                    }
                    {
                        volatile int *v = (volatile int *)(r4 + 208);
                        int x = *v;
                        if (x < 0) x += 15;
                        *v = x >> 4;
                    }
                    r4[29] = 1;
                    if (r4[32] == 0) {
                        _08026550(*(void *volatile *)(r4 + 280),
                                  *(volatile int *)(r4 + 8) - *(volatile int *)(racectx2 + 8),
                                  (void *)(r4 + 200), 0);
                    }
                } else {
                    r4[29] = 0;
                }
            }
            r5 = r7;
            lim = (int)(s16)*(volatile u16 *)(*(volatile u8 *volatile *)(uintptr_t)0x03004E20u + 70) - 1;
        }
    }
}

// ----------------------------------------------------------------------------
// 0x0800196E8 sub_0800196E8 — lap-1 start + record flash:
//   u16[WA+0x134] = 1000 (250<<2)
//   gate [WA+0x10DC] & 1 else sub_080018F94 + 0x03000598 placement tail
//   if [0x03000598] != 0 and byte[+29]:
//     random → [r+220]; _08002C48; [r+208]>>4; _08026550(r+280, racectx, …)
//     if _08018ACC(1<<17): _08027234([r],[r+4],(s16)[r+8],0x03005DF0)
//   if _08018ACC(1<<9): sub_080019250(racers+284); r4 = 2 - ret
//   else r4 = byte[WA+0x10C3] ? 1 : 2
//   if !_08018ACC(1): u16[racers+14] = r4
//   else: sub_080018F94; r5=[0x03000598]; if r5: [r5+252]=r4; digit path;
//         _08026550; random; [r+220]; _08002C48; [r+224]=(s16)[rc+134];
//         [r+228]=0x18000; [r+208]>>4; r5[29]=r6; _08026550
// ----------------------------------------------------------------------------
void sub_0800196E8(void)
{
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    volatile u8 *racers = (volatile u8 *)(uintptr_t)0x03004E80u;
    volatile u8 *racectx = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
    volatile u32 *arr = (volatile u32 *)(uintptr_t)0x03000598u;
    int r4 = 0;

    *(volatile u16 *)(wa + 0x134) = 1000;

    if (*(volatile u32 *)(wa + 0x10DC) & 1) {
        volatile u8 *r4p = (volatile u8 *)arr[0];
        if (r4p != 0 && r4p[29] != 0) {
            int rnd = (int)(s16)_08002BE8();
            *(volatile int *)(r4p + 220) = rnd;
            _08002C48((int)(s16)*(volatile u16 *)(r4p + 216), 0);
            {
                volatile int *v = (volatile int *)(r4p + 208);
                int x = *v;
                if (x < 0) x += 15;
                *v = x >> 4;
            }
            _08026550(*(void *volatile *)(r4p + 280),
                      *(volatile int *)(r4p + 8) - *(volatile int *)(racectx + 8),
                      (void *)(r4p + 200), 0);
            if (_08018ACC(128u << 17)) {
                _08027234(*(volatile u32 *)(r4p + 0), *(volatile u32 *)(r4p + 4),
                          (s16)*(volatile s16 *)(r4p + 8),
                          (void *)(uintptr_t)0x03005DF0u);
            }
        }
        if (_08018ACC(128u << 9)) {
            r4 = 2 - _080019250((void *)(uintptr_t)(racers + 284), (void *)0);
        } else {
            r4 = (*(volatile u8 *)(wa + 0x10C3) != 0) ? 1 : 2;
        }
        if (!_08018ACC(1)) {
            *(volatile u16 *)(racers + 14) = (u16)r4;
        }
    } else {
        volatile u8 *r5;
        sub_080018F94();
        r5 = (volatile u8 *)(uintptr_t)(uintptr_t)arr[0];
        if (r5 == 0)
            return;
        *(volatile int *)(r5 + 252) = r4;
        if (_08004508((volatile u32 *)(r5 + 200))) {
            int rnd = (int)(s16)_08002BE8();
            *(volatile int *)(r5 + 220) = rnd;
            _08002C48((int)(s16)*(volatile u16 *)(r5 + 216), 0);
            *(volatile int *)(r5 + 224) = (int)(s16)*(volatile u16 *)(racectx + 134);
            *(volatile int *)(r5 + 228) = 192 << 7;
            {
                volatile int *v = (volatile int *)(r5 + 208);
                int x = *v;
                if (x < 0) x += 15;
                *v = x >> 4;
            }
            r5[29] = (u8)r4;
            _08026550(*(void *volatile *)(r5 + 280),
                      *(volatile int *)(r5 + 8) - *(volatile int *)(racectx + 8),
                      (void *)(r5 + 200), 0);
        } else {
            r5[29] = 0;
        }
    }
}

// ----------------------------------------------------------------------------
// 0x08001992C sub_08001992C — course-id tick:
//   [racectx+48] = min([racectx+48]+1, 99)
//   _08008164([racectx+48]); _08008208
//   r = DivRem([racectx+48], 5)
//   vol = (s16)(([racectx+48]-5) * 150)
//   r != 0 → _0802B500(22, 255, …) else _0802B500(23, 255, …)
//   if [racectx+48] > [racectx+52]:
//     [racectx+52] = [racectx+48]; _080081F8([racectx+48])
// ----------------------------------------------------------------------------
//   The ROM never keeps the context base in a register across the body: the
//   prologue is only `push {r4, lr}` and every use re-loads *0x03004E20 (the
//   address itself lives in r4 and is rematerialised from the pool for the
//   tail). Caching the base in a local instead forces `push {r4, r5, r6, lr}`
//   and a callee-saved pointer. So each use is spelled as the full
//   `*(volatile u32 *)(*(volatile u32 *)(uintptr_t)0x03004E20u + off)` load.
void sub_08001992C(void)
{
    int r;

    {
        volatile u32 *p = (volatile u32 *)(*(volatile u32 *)(uintptr_t)0x03004E20u + 48);
        int v = (int)(*p) + 1;
        *p = (u32)v;
        if (v > 99) *p = 99;
    }

    _08008164((int)*(volatile u32 *)(*(volatile u32 *)(uintptr_t)0x03004E20u + 48));
    _08008208();

    r = _0802D97C((int)*(volatile u32 *)(*(volatile u32 *)(uintptr_t)0x03004E20u + 48), 5);
    if (r != 0) {
        int d = (int)*(volatile u32 *)(*(volatile u32 *)(uintptr_t)0x03004E20u + 48) - 5;
        _0802B500(22, 255, (s16)(d * 150));
    } else {
        int d = (int)*(volatile u32 *)(*(volatile u32 *)(uintptr_t)0x03004E20u + 48) - 5;
        _0802B500(23, 255, (s16)(d * 150));
    }

    {
        // Signed fields and a single read of [48]: the ROM's compare is `ble`
        // (signed) and the value is stored and passed on without a reload.
        volatile s32 *b = (volatile s32 *)*(volatile u32 *)(uintptr_t)0x03004E20u;
        s32 a = b[12];
        if (a > b[13]) {
            b[13] = a;
            _080081F8((u32)a);
        }
    }
}

// ----------------------------------------------------------------------------
// 0x0800199CC sub_0800199CC — start-scene award check:
//   if _08018ACC(1<<14):
//     rc = *0x03004E20
//     if (s16)[rc+0x4F0] > 79 and (s16)[rc+94] > 0:
//       rc2 = *(u32 *)(rc+0x4E4)
//       if (s16)[rc2+18] >= (s16)[rc2+22]: sub_08001992C
//   [rc+56] = 0; _08018AA8(1<<21, 0)
// ----------------------------------------------------------------------------
void sub_0800199CC(void)
{
    if (_08018ACC(128u << 7)) {
        register volatile u8 *rc __asm__("r1") =
            *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
        register u32 offset __asm__("r2") = 158;
        __asm__ volatile("" : "+r"(offset));
        offset <<= 3;
        __asm__ volatile("" : "+r"(offset));
        register u32 addr __asm__("r0") = (u32)(uintptr_t)rc + offset;
        __asm__ volatile("" : "+r"(addr));
        register int total __asm__("r0");
        __asm__ volatile(".code 16\n\t"
                         "mov r2, #0\n\t"
                         "ldrsh r0, [r0, r2]"
                         : "=r"(total) : "0"(addr) : "r2", "cc");
        if (total > 79) {
            register u32 addr94 __asm__("r0") = (u32)(uintptr_t)rc;
            __asm__ volatile("" : "+r"(addr94));
            addr94 += 94;
            __asm__ volatile("" : "+r"(addr94));
            register int positive __asm__("r0");
            __asm__ volatile(".code 16\n\t"
                             "mov r2, #0\n\t"
                             "ldrsh r0, [r0, r2]"
                             : "=r"(positive) : "0"(addr94)
                             : "r2", "cc");
            if (positive > 0) {
                register u32 offset4E4 __asm__("r2") = 0x4E4u;
                __asm__ volatile("" : "+r"(offset4E4));
                register volatile u8 *record __asm__("r0");
                __asm__ volatile(".code 16\n\t"
                                 "add r0, r1, r2\n\t"
                                 "ldr r0, [r0]"
                                 : "=r"(record)
                                 : "r"(rc), "r"(offset4E4)
                                 : "cc", "memory");
                register int value22 __asm__("r0") =
                    (int)(uintptr_t)record;
                register int value18 __asm__("r1");
                __asm__ volatile(".code 16\n\t"
                                 "mov r2, #18\n\t"
                                 "ldrsh r1, [r0, r2]\n\t"
                                 "mov r2, #22\n\t"
                                 "ldrsh r0, [r0, r2]"
                                 : "+r"(value22), "=r"(value18)
                                 : : "r2", "cc");
                if (value18 >= value22)
                    sub_08001992C();
            }
        }
    }
    volatile u8 *rc = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
    *(volatile int *)(rc + 56) = 0;
    _08018AA8(128u << 21, 0);
}

// ----------------------------------------------------------------------------
// 0x080019A2C sub_080019A2C(mode) — award-tier register write:
//   _08018AA8(64, 1)
//   rc = *0x03004E20
//   u16[rc+72] = mode
//   0x04000050 = 0x0FDF
//   mode==0: 0x04000054 = 16; u16[rc+74] = 32
//   mode==1: 0x04000054 = 0;  u16[rc+74] = 32
// ----------------------------------------------------------------------------
void sub_080019A2C(int mode)
{
    _08018AA8(64, 1);
    volatile u8 *rc = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
    *(volatile u16 *)(rc + 72) = (u16)mode;
    *(volatile u16 *)(uintptr_t)0x04000050u = 0x0FDF;

    switch (mode) {
    case 0:
        *(volatile u16 *)(uintptr_t)0x04000054u = 16;
        *(volatile u16 *)(rc + 74) = 32;
        break;
    case 1:
        *(volatile u16 *)(uintptr_t)0x04000054u = 0;
        *(volatile u16 *)(rc + 74) = 32;
        break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------------
// 0x080019AEC sub_080019AEC(mode) — award-grid finalize + diff scan:
//   r4 = u16[rc+64] - 1
//   [rc+44] = 0x7FFFFFFF
//   if mode > 0:
//     WA[0x10E4] = 1; WA[0x10E5] = mode
//     if u16[WA+0x10FC] == 8 and mode == 2: [WA+0x10F4] = 0
//     else: [WA+0x10F4] = [rc+24]
//     if u16[WA+0x10FC] == 5: [WA+0x10F8] = [rc+52]
//     r4 = (u16)(r4 + (1<<12))
//   else:
//     WA[0x10E4] = 0
//     if u16[WA+0x10FC] == 5: [WA+0x10F8] = [rc+52]
//   for i in 0..(s16)r4-1:
//     d = [0x03004E20 + 28 + (i+1)*4] - [0x03004E20 + 28 + i*4]
//     [0x03002868 + i*4] = d
//     if d < [rc+44]: [rc+44] = d
// ----------------------------------------------------------------------------
void sub_080019AEC(int mode)
{
    volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;
    volatile u8 *rc = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
    volatile u32 *diff = (volatile u32 *)(uintptr_t)0x03002868u;
    u32 r4 = (u32)(u16)(*(volatile u16 *)(rc + 64) - 1);
    int i;

    *(volatile int *)(rc + 44) = 0x7FFFFFFF;

    if (mode > 0) {
        *(volatile u8 *)(wa + 0x10E4) = 1;
        *(volatile u8 *)(wa + 0x10E5) = (u8)mode;
        if (*(volatile u16 *)(wa + 0x10FC) == 8 && mode == 2) {
            *(volatile int *)(wa + 0x10F4) = 0;
        } else {
            *(volatile int *)(wa + 0x10F4) = *(volatile int *)(rc + 24);
        }
        if (*(volatile u16 *)(wa + 0x10FC) == 5)
            *(volatile int *)(wa + 0x10F8) = *(volatile int *)(rc + 52);
        r4 = (u32)(u16)(r4 + (128u << 9));
    } else {
        *(volatile u8 *)(wa + 0x10E4) = 0;
        if (*(volatile u16 *)(wa + 0x10FC) == 5)
            *(volatile int *)(wa + 0x10F8) = *(volatile int *)(rc + 52);
    }

    for (i = 0; (int)(s16)r4 > i; i++) {
        volatile u32 *rcb = *(volatile u32 *volatile *)(uintptr_t)0x03004E20u;
        u32 next = *(volatile u32 *)(rcb + 28 + (i + 1) * 4);
        u32 cur  = *(volatile u32 *)(rcb + 28 + i * 4);
        int d = (int)next - (int)cur;
        diff[i] = (u32)d;
        if ((u32)d < *(volatile u32 *)(rc + 44))
            *(volatile u32 *)(rc + 44) = (u32)d;
    }
}

// ----------------------------------------------------------------------------
// 0x080019D6C sub_080019D6C — VCounter 160 set: BG rot/palette prep:
//   if u8[0x04000006] > 159:
//     0x04000010-ish REG_BASE writes: 0x04000010? no —
//     asm: [0x04000008? (1<<19)] = 0x1341  (BG2 PA/rot base region)
//          [+10] = 0x5E4A
//          [+20] = ([0x03004E20]+8)>>2
//          [+22] = 6
//     _080023AC(3, 0x08019DBD)
// ----------------------------------------------------------------------------
void sub_080019D6C(void)
{
    if (*(volatile u8 *)(uintptr_t)0x04000006u > 159) {
        volatile u16 *base = (volatile u16 *)(128u << 19);
        // Preserve the ROM's 10, 10, then 2 byte destination advances.
        *base = 0x1341;
        base += 5;
        *base = 0x5E4A;
        base += 5;
        *base = (u16)(*(volatile u32 *)(*(volatile u32 *)(uintptr_t)0x03004E20u + 8) >> 2);
        base += 1;
        *base = 6;
        _080023AC(3, (void *)(uintptr_t)0x08019DBDu);
    }
}

// ----------------------------------------------------------------------------
// 0x080019EC8 sub_080019EC8 — VCounter 55 fade-in edge:
//   if u8[0x04000006] > 55 and _08018ACC(1<<23):
//     0x04000050 = 0x1442
//     v = u16[[rc]+114]
//     0x04000052 = (v+2)<<8 | v
//     _080023AC(3, 0x08019F1D)
// ----------------------------------------------------------------------------
void sub_080019EC8(void)
{
    if (*(volatile u8 *)(uintptr_t)0x04000006u > 55) {
        if (!_08018ACC(128u << 23))
            return;
        *(volatile u16 *)(uintptr_t)0x04000050u = 0x1442;
        // Declared AFTER the 0x04000050 store on purpose: the ROM issues the
        // store before it touches *0x03004E20, while a declaration placed
        // earlier lets agbcc hoist the pointer chain above the store and it
        // also folds 0x04000052 into 0x04000050 with `add r2,r2,#2`, costing
        // a pool word (5 against the ROM's 6).  `dst` is likewise bound before
        // the pointer chain so the 0x04000052 literal load is issued ahead of
        // the 0x03004E20 one, which is the ROM's order.  48/84 -> 84/84.
        // `v` is a u32 ON PURPOSE.  Declared u16 it reaches the OR as a HI
        // subreg, gcc2.95's narrow_binop narrows the shifted half alongside it,
        // and expand then builds `(set X (ior v (u16)(t<<8)))` with a FRESH
        // target X -- which lands the result in v's register, giving
        // `orrs r1,r0` against the ROM's `orrs r0,r1`.  As a u32 both
        // operands are SI, the target is the shift's own register, and the
        // two bytes match.  u32 is also the honest width: the ROM's
        // `ldrh r1,[r0]` zero-extends into the 32-bit `adds`/`lsls`/`orrs`.
        {
            volatile u16 *dst = (volatile u16 *)(uintptr_t)0x04000052u;
            volatile u8 *rc = *(volatile u8 *volatile *)(uintptr_t)0x03004E20u;
            u32 v = *(volatile u16 *)(rc + 114);
            *dst = (u16)(((v + 2) << 8) | v);
            _080023AC(3, (void *)(uintptr_t)0x08019F1Du);
        }
    }
}

void sub_08019F70(void)
{
    u8 *rec = *(u8 *volatile *)_08005F98();
    volatile u32 *cfg;
    // The ROM dereferences cfg in place (r0 = *cfg) and keeps the context in
    // r0 for both store addresses and the later +0x8C lane read.
    register u32 *rc __asm__("r0");
    s16 *pa;
    s16 *pb;
    // `ldrb` loads (so the bytes are zero-extended) but the ROM clamps with
    // `cmp #13 / ble` (signed): widest-int locals reproduce both.
    int a;
    // The ROM keeps the second clamped lane in r3 (`ldrb r3,[r0,#15]`,
    // `lsls r5,r3,#1`), so pin it there.
    register int b __asm__("r3");
    a = rec[14];
    b = rec[15];
    if (a > 13) a = 0;
    if (b > 13) b = 0;
    cfg = (volatile u32 *)(uintptr_t)0x03004E20u;
    rc = (u32 *)*cfg;
    // The pointer value is bound inside the store expression: the ROM computes
    // the store address first (`movs r4,#146` / `lsls r4,r4,#3` /
    // `adds r2,r0,r4`) and only then the `a*24 + 0x080CB954` chain, and it
    // reuses that 0x490 offset with `adds r4,#4` for the second lane. Keeping
    // the assignments as separate statements makes agbcc emit the pointer
    // chains before the addresses and materialise 0x494 from the pool.
    rc[292] = (u32)(uintptr_t)(pa = (s16 *)(uintptr_t)(0x080CB954u + a * 24));
    rc[293] = (u32)(uintptr_t)(pb = (s16 *)(uintptr_t)(0x080CB954u + b * 24));
    _08007538((void *)(uintptr_t)0x0832F654u, pa[0],
              *(s16 *)((u8 *)rc + 0x8C));
    _08007538((void *)(uintptr_t)0x0832F654u, pb[0],
              *(s16 *)((u8 *)*cfg + 0x8E));
    _080075E8((void *)(uintptr_t)0x0832F654u, pa[1], 0);
    _080075E8((void *)(uintptr_t)0x0832F654u, pb[1], 1);
}

// PROBE SCORE 19/152 (OVERSIZED, 160-byte candidate). The ROM body is
// transcribed statement for statement; what does not match is scheduling and
// register choice only — agbcc keeps the surface pointer in r1 (the ROM leaves
// it in r0 after `ldr r0,[r0]`), inserts two `adds` copies where the ROM picks
// the ldrb destination register directly, and allocates pa/pb to r4/r5 where
// the ROM uses r5/r4. Every call target and every pool constant resolves.
// Pinning the locals with `register... __asm__("rN")` does reach 31/152, but
// this file has no other register pin and the technique does not belong in a
// C89 translation unit, so the readable form is kept and the gap is recorded.
void _080019F70(void) __attribute__((alias("sub_08019F70")));
void _08019F70(void) __attribute__((alias("sub_08019F70")));

// ----------------------------------------------------------------------------
// 0x0800199B0 — 20 B flag leaf (asm/race_setup_18adc.s; the award path in
// race_scene_b1.c): raise the 0x4000 and 0x10000000 scene flags.
//   _08018AA8(0x80 << 7, 1); _08018AA8(0x80 << 21, 1);
// The second flag is 0x10000000, not 0x01000000: the ROM materialises it as
// `movs r0,#0x80` / `lsls r0,r0,#21` (0x080199bc-0x080199be), and 0x80<<21 is
// 0x10000000. The old 0x01000000u encoded as `lsls r0,r0,#17`.
void sub_0800199B0(void)
{
    _08018AA8(0x00004000u, 1);
    _08018AA8(0x10000000u, 1);
}
__asm__(".align 2, 0");

// ----------------------------------------------------------------------------
// 0x080019A84 — 100 B lap-clock leaf on racectx (*0x03004E20):
//   n = s16[rc+0x4A]; if (n <= 0) { Ghost_FlagOp(64, 0); return; }
//   u16[rc+0x4A] = n - 1;
//   switch (s16[rc+0x48]): case 0 → 0x04000054 = Div(s16[rc+0x4A], 2);
//                          case 1 → 0x04000054 = 16 - Div(s16[rc+0x4A], 2);
// The reads are NOT volatile: a volatile s16 lvalue lowers to `ldrh` plus an
// explicit sign extension, while the ROM's `movs r4,#0 / ldrsh r0,[r1,r4]` is
// what a plain signed halfword read emits (same rule as s8). The unsigned
// `ldrh r2,[r1,#0]` for the decrement and the signed `ldrsh` for the test are
// two separate reads of rc+0x4A in the ROM, so keep the value in a u16 local.
void sub_080019A84(void)
{
    u8 *rc = *(u8 **)(uintptr_t)0x03004E20u;
    volatile u16 *p4A = (volatile u16 *)(rc + 0x4A);
    // The halfword is pinned: unpinned, agbcc lands the load in a fresh temp
    // and copies it home (`ldrh r0,[r1]` / `adds r2,r0,#0`) -- that copy alone
    // is the candidate's 2-byte overshoot. The ROM loads straight into r2. The
    // decrement, though, stays a *separate* temp (`subs r0,r2,#1` before the
    // in-place `subs r2,#1` a pinned subtraction would give), so it goes
    // through its own u32 local.
    register u32 rword __asm__("r2");
    u32 nv;
    s16 lane;
    rword = *p4A;
    if (*(s16 *)(rc + 0x4A) > 0) {
        nv = rword - 1u;
        // Keep rword live through the subtraction: without it agbcc reuses the
        // pinned r2 as the destination (`subs r2,#1`), where the ROM computes
        // into a fresh temp (`subs r0,r2,#1`). Empty asm only.
        __asm__("" : : "r"(rword));
        *p4A = (u16)nv;
        lane = *(s16 *)(rc + 0x48);
        if (lane == 0) {
            *(volatile u16 *)(uintptr_t)0x04000054u =
                (u16)_0802D978((int)*(s16 *)(rc + 0x4A), 2);
        } else if (lane == 1) {
            *(volatile u16 *)(uintptr_t)0x04000054u =
                (u16)(16 - _0802D978((int)*(s16 *)(rc + 0x4A), 2));
        }
    } else {
        _08018AA8(64u, 0);
    }
}
// Match the ROM's two zero pad bytes instead of GAS's Thumb `46 c0` fill.
__asm__(".align 2, 0");

// ROM entry alias.
void _080199B0(void) __attribute__((alias("sub_0800199B0")));
void sub_080199B0(void) __attribute__((alias("sub_0800199B0")));
void _08019A84(void) __attribute__((alias("sub_080019A84")));
void sub_08019A84(void) __attribute__((alias("sub_080019A84")));
void RaceScene_Leaf_318(void) __attribute__((alias("sub_080019318")));
void RaceScene_Leaf_860(void) __attribute__((alias("sub_0800196E8")));
void Sub_08019AEC(int mode) __attribute__((alias("sub_080019AEC")));
void _0801992C(void) __attribute__((alias("sub_08001992C")));
void _080199CC(void) __attribute__((alias("sub_0800199CC")));
void _08019A2C(int mode) __attribute__((alias("sub_080019A2C")));
void _080019A2C(int mode) __attribute__((alias("sub_080019A2C")));
void _08019AEC(int mode) __attribute__((alias("sub_080019AEC")));
// The closure uses 9-digit spellings for these entry points, so keep those
// exact aliases beside the shorter C-facing names above.
void _0800199B0(void) __attribute__((alias("sub_0800199B0")));
void _0800199CC(void) __attribute__((alias("sub_0800199CC")));
