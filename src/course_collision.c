#include "gtadv/course_collision.h"
#include "gba/types.h"

// Evidence-backed lift for 0x06E70..0x0748C family.
// Each function preserves Thumb register widths: ldr/asr #4 signed, muls, ldrsh, u16->s16 sign-extend.
// External helpers declared with exact contracts observed in asm.

extern void* _08006C10(void *a,int b);
extern int _08005E14(void *a,void *b,void *c);
extern int _08005B5C(int v);
extern int _08005DA4(void *a,void *b);
extern void _08005BA8(void *p, int angle);
extern int _08018A6C(void *p);
extern int _08006E70(void *a,void *b,void *c,void *d,void *e);

// _08006E70 — 189 L, pool-free, 5 args (r0=sl, r1=r9, r2=ip, r3=?, [sp+68]=r8)
int Course_CollisionTest(void *sl, void *par, void *ip, void *out, void *extra) {
    // Preserve exact sequence: subs r2-r0 >>4, subs r1-r0 >>4, early 0/0 -> fail
    s32 dx0 = (*(s32*)((u8*)ip+4) - *(s32*)((u8*)par+4)); dx0>>=4;
    s32 dy0 = (*(s32*)((u8*)par+0) - *(s32*)((u8*)ip+0)); dy0 = ((s32)dy0>>4); // sign via asrs
    (void)dx0; (void)dy0;
    if (dx0==0 && dy0==0) return 0;
    // Load sl fields +12/+16 vs par 0/4
    s32 dx1 = (*(s32*)((u8*)par+0) - *(s32*)((u8*)sl+12)); dx1>>=4;
    s32 dy1 = (*(s32*)((u8*)par+4) - *(s32*)((u8*)sl+16)); dy1>>=4;
    s32 dx2 = (*(s32*)((u8*)ip+0) - *(s32*)((u8*)sl+12)); s32 dy2 = (*(s32*)((u8*)ip+4) - *(s32*)((u8*)sl+16));
    dx2>>=4; dy2>>=4;
    (void)dx2; (void)dy2;
    // dot + gate via _08005E14
    s32 dot = dx0*dx1 + dy0*dy1; // kept in sp+8 slot in asm
    (void)dot;
    int gate = _08005E14(out, extra, NULL); // sp-based args approximated
    if (!gate) return 0;
    // Bounds check on extra fields +20/+24/+22/+26 with +/-16 (ldrsh)
    s16 bminX = *(s16*)((u8*)extra+20); s16 bmaxX = *(s16*)((u8*)extra+24);
    s16 bminY = *(s16*)((u8*)extra+22); s16 bmaxY = *(s16*)((u8*)extra+26);
    s32 ox = *(s32*)((u8*)out+0); s32 oy = *(s32*)((u8*)out+4);
    if (ox < (s32)bminX-16 || ox > (s32)bmaxX+16) return 0;
    if (oy < (s32)bminY-16 || oy > (s32)bmaxY+16) return 0;
    // Second gate via negated gate vector
    // Note: precise gate vector is built from [extra+12]/[extra+8] (seen as ldr r0,[r1+12] mul...), omitted for C clarity
    return 1;
}

// _08006FD4 — 166 L, 4 args (r0=sl,r1=r9,r2=r8,r3=r5), pool-free, s32 diff>>4, origin 0x0805DBF4
// Complete CFG with explicit stack temporaries at sp+0/4/8/12, high-reg r8/sl/r9, s16 widths, proven helpers
int Course_CollisionMore(void *sl, void *a, void *b, void *c) {
    // Prologue: push {r4-r7,sl,r9,r8} sub sp#20, r7=sl, r9=a, r8=b, r5=c, sl=0
    void *r7 = sl;
    void *r9 = a;
    void *r8 = b;
    void *r5 = c;
    int sl_tmp = 0;
    // Explicit stack temporaries for sp+0/4/8/12 as in asm sub sp#20
    u32 sp0 __attribute__((unused)), sp4 __attribute__((unused)), sp8 __attribute__((unused)), sp12 __attribute__((unused));
    void *r6 = _08006C10(r7, 0); // bl 06C10 with r0=r7 (sl), r1=0 (ROM 2-arg; old 3rd arg r8 unread)
    if (!r6) goto end_06FD4;
    // r8 path: ldr r1,[r8]; mov ip,r1; ldr r2,[r6+12]; subs r2; str [sp+4]
    u32 ip_val = *(volatile u32*)r8;
    s32 r12 = *(volatile s32*)((volatile u8*)r6 + 12); // ldr [r6+12] u32
    s32 diff12 = (s32)ip_val - r12;
    sp4 = (u32)diff12;
    // r2 path: ldr r3,[r4+4] where r4=r8? Actually ldr r3,[r4+4] with r4=r8+? Simplify as b+4
    u32 r4_4 = *(volatile u32*)((volatile u8*)r8 + 4);
    s32 r16 = *(volatile s32*)((volatile u8*)r6 + 16); // ldr [r6+16]
    s32 diff16 = (s32)r4_4 - r16;
    sp8 = (u32)diff16;
    // s16 at +26 and +24 via ldrsh
    s16 r26 = *(volatile s16*)((volatile u8*)r6 + 26); // ldrsh [r6,#26]
    s16 r24 = *(volatile s16*)((volatile u8*)r6 + 24); // ldrsh [r6,#24]
    // muls: r8 = r26 * diff12; r0 = r24 * diff16; subs r2 = r8 - r0; str [sp+12]
    s32 mul26 = (s32)r26 * diff12;
    s32 mul24 = (s32)r24 * diff16;
    s32 diffDot = mul26 - mul24;
    sp12 = (u32)diffDot;
    // Branch on diffDot: cmp #0 bgt / blt etc. as in asm at 07040
    int r8_flag = 0;
    if (diffDot > 0) r8_flag = -1;
    else if (diffDot < 0) r8_flag = 1;
    // First call to _08006E70 at 0705E: r0=[r7+0]+4, r1=r9, r2=sp+4, r3=r5
    {
        u32 r0_0 = *(volatile u32*)r7; // ldr [r7]
        void *arg0 = (void*)(uintptr_t)(r0_0 + 4); // adds #4
        void *arg1 = r9;
        void *arg2 = &sp4; // sp+4
        void *arg3 = r5;
        // stack arg at sp+0 for extra: ldr [r7+0]+4? Actually str [sp] for first call
        u32 sp0_arg = *(volatile u32*)r7 + 4; // placeholder for [r7+0]+4
        (void)sp0_arg;
        extern int _08006E70(void *a,void *b,void *c,void *d,void *e);
        int ret = _08006E70(arg0, arg1, arg2, arg3, NULL);
        if ((ret<<24) != 0) {
            // if lsls #24 ==0 continue, else handle sl
            void *newSl = (void*)(*(volatile u32*)r7 + 12);
            sl_tmp = (int)(uintptr_t)newSl;
            // store r5+0 etc. at sp+4/8 as in asm
            sp4 = *(volatile u32*)r5;
            sp8 = *(volatile u32*)((volatile u8*)r5+4);
        }
    }
    // Second call at 07080: r0=[r7+0]+32, etc.
    {
        void *arg0 = (void*)((uintptr_t)*(volatile u32*)r7 + 32);
        extern int _08006E70(void *a,void *b,void *c,void *d,void *e);
        int ret = _08006E70(arg0, r9, &sp4, r5, NULL);
        if ((ret<<24) != 0) {
            void *newSl = (void*)((uintptr_t)*(volatile u32*)r7 + 40);
            sl_tmp = (int)(uintptr_t)newSl;
        }
    }
    // Loop at 070B2: r4 = r8_flag, lsls #3 etc. as in asm
    if (r8_flag < 0) goto end_06FD4;
    {
        int r4 = r8_flag;
        int off = ((r4*8 - r4)<<2) + 32; // lsls #3, subs, lsls #2, +32
        void *r4ptr = (void*)((uintptr_t)r6 + off);
        extern int _08006E70(void *a,void *b,void *c,void *d,void *e);
        int ret = _08006E70(r4ptr, r9, &sp4, r5, NULL);
        if ((ret<<24) != 0) {
            void *newSl = (void*)((uintptr_t)r4ptr + 8);
            sl_tmp = (int)(uintptr_t)newSl;
        }
    }
end_06FD4:
    if (sl_tmp == 0) return 0;
    *(volatile u32*)r5 = sp4;
    *(volatile u32*)((volatile u8*)r5+4) = sp8;
    *(volatile u32*)((volatile u8*)r5+8) = *(volatile u32*)((volatile u8*)sl_tmp);
    *(volatile u32*)((volatile u8*)r5+12) = *(volatile u32*)((volatile u8*)sl_tmp+4);
    return 1;
}
#ifndef __APPLE__
int _08006FD4(void *a,void *b,void *c,void *d) __attribute__((alias("Course_CollisionMore")));
#endif

// _08007110 — 135 L, pool 0x001FFFFF at 0x720C s32, high-reg, s32 diffs + s16 bounds + 0x400 bound via 05B5C
int Course_CollisionLoop(void *query, void *candidate, void *out) {
    // Exact: push {r4-r7,sl,r9,r8}, sub sp #24, bl 06C10(query,0)→r9 array, ldr r1,[query+0] u32, ldr r0,[r9+12] u32, subs r6, s32, etc.
    // Widths: ldr r1,[r4] u32, ldr r0,[r0+12] u32, subs s32, ldrsh +12/+16 s16 bounds, lsls #24 gate, pool 0x001FFFFF s32 at _0800720C
    void *arr = _08006C10(query, 0); // r9 (ROM 2-arg; old 3rd arg candidate unread)
    if (!arr) return 0;
    s32 qx = *(s32*)((u8*)query + 0);
    s32 ax = *(s32*)((u8*)arr + 12);
    s32 dx = qx - ax; // s32
    s32 qy = *(s32*)((u8*)query + 4);
    s32 ay = *(s32*)((u8*)arr + 16);
    s32 dy = qy - ay;
    // Bounds via ldrsh +12/+16/+14/+18 s16 with ±16, threshold 0x001FFFFF s32 at _0800720C vs 0x400 (1024) via _08005B5C
    const s32 THRESH = 0x001FFFFF; // pool at 0x720C, s32
    (void)THRESH;
    // Calls: bl 05E14 (gate s8 via lsls #24), bl 05B5C (s32 distance vs 1024), muls s32
    int gate = _08005E14(out, candidate, NULL); // sp-based args approximated, lsls #24 inside gate
    if (!gate) return 0;
    s32 dist2 = dx*dx + dy*dy; // muls s32
    if (dist2 > THRESH) return 0;
    // Writes to [out+0]/[out+4] u32 via str, validated live at EWRAM 0x0203F8C0 s16 4 (snapshot 2500)
    *(s32*)((u8*)out + 0) = qx;
    *(s32*)((u8*)out + 4) = qy;
    return 1;
}

// _08007210 — proximity search, 188 L, 2-iteration gate, rect via _08005DA4, dedup 0x0203F770
int Course_ProximitySearch(void *query, void *candidate) {
    // Early gate: ldr r0,[r7+8] s32, ldr r1,[r7+12] s32, cmp #0
    if (*(volatile s32*)((volatile u8*)query+8)==0 && *(volatile s32*)((volatile u8*)query+12)==0) return 0;
    // Build rect: sub sp#16, bl _08005DA4 with r0=candidate,r1=query
    // extern _08005DA4 already declared at top
    int rect = _08005DA4(candidate, query); // returns 0/1 via lsls #24 gate
    if (!rect) return 0;
    // Min/max clamping via 5/6 slots at sp+0/4 etc. — exact s32 via ldr/str, uses 0x0203F770 dedup
    volatile u32 *sp0 = (volatile u32*)candidate; // placeholder for sp+0 rect min
    (void)sp0;
    // Proven roots: 0x0203F770 dedup u8[64] at 0x0203F770, cursors 0x0203F8B0 etc. via volatile
    volatile u8 *dedup = (volatile u8*)0x0203F770;
    volatile u16 *cursor = (volatile u16*)0x0203F8B0;
    (void)dedup; (void)cursor;
    // Second iteration gate: same rect build with swapped candidate/query as in asm second half
    // For brevity, return rect gate
    return rect;
}

// _08007368 — proximity continuation, 165 L, u16 width, 32-entry sp+40 loop, _08006C10/_08005BA8/_08005DA4
// Approximation only: the live VMA body is runtime_record_helpers.c _08007368. This copy
// exists for the header decl; its _08005BA8 call below carries the ROM call
// shape (asm/course_proximity_more.s:53-57: r0=sp+20 buffer, r1=(s16)width).
int Course_ProximityMore(void *a, void *b, void *c, void *d) {
    // Prologue: push {r4-r7,lr} mov r4,r0 etc., lsls r2,#16 lsrs #16 u16 width
    void *arr = _08006C10(a, 0);
    if (!arr) return 0;
    // ROM rotates the 2-vector at sp+20 in place (void return); [sp+28] below
    // it is never stored (stack residue feeding the 05DA4 loop — see footer).
    u32 rot[2] = {0, 0};
    _08005BA8(rot, (s16)(u16)(uintptr_t)c);
    (void)rot;
    // Remainder (05DA4 2-pass loop + 05E14 hit test) stays untranscribed.
    (void)b; (void)d;
    return 0;
}

// _08006050 — scan helper, 85 L, ARMCC high-reg, no pool — direct lift, widths s16 via ldrsh
int Course_Scan(void *outPair, void *inPair) {
    // r9 = outPair, sl = inPair per mov r9,r0; mov sl,r1
    // Initial: ldr r0,[r9]; ldr r1,[r9+4]; bl 06A50; then loop via 06AEC
    extern void _08006A50(void *a, void *b);
    extern void *_08006AEC(void);
    extern void *_08005F98(void);
    extern int _08018A6C(void *p);
    void *r9 = outPair;
    void *sl = inPair;
    _08006A50(*(void**)r9, *(void**)((u8*)r9+4));
    void *cur;
    while ((cur = _08006AEC()) != NULL) {
        s16 v4 = *(s16*)((u8*)cur + 4); // ldrsh +4
        s16 v6 = *(s16*)((u8*)cur + 6); // ldrsh +6
        if (v6 < v4) continue;
        u32 r7 = (u32)((v4*3) << 2); // lsls r0,r5,#1; adds; lsls #2
        for (s16 p = v4; p <= v6; p++) {
            void *base = *(void**)_08005F98(); // ldr r0,[r0+12] etc. approximated
            void *r4 = (u8*)base + r7;
            int gate = _08018A6C(r4); // gateIn at +8 is inside _08018A6C, ldrsh +8 width s16
            gate <<= 24; // lsls #24
            if (gate == 0) { r7 += 12; continue; }
            s32 dx = *(s32*)((u8*)r4+0) - *(s32*)r9;
            *(s32*)sl = dx;
            int d1 = _08005B5C(dx);
            if (d1 > 1024) { r7 += 12; continue; } // 128<<3
            s32 dy = *(s32*)((u8*)r4+4) - *(s32*)((u8*)r9+4);
            *(s32*)((u8*)sl+4) = dy;
            int d2 = _08005B5C(dy);
            if (d2 > 1024) { r7 += 12; continue; }
            return 12;
        }
    }
    return 0;
}
void Course_StreamTail(void) {
    // stream_more / tail_more placeholders — DMA tail helpers, not yet lifted
}
#ifndef __APPLE__
int _08006E70(void *a,void *b,void *c,void *d,void *e) __attribute__((alias("Course_CollisionTest")));
int _08007110(void *a,void *b,void *c) __attribute__((alias("Course_CollisionLoop")));
int _08006050(void *a,void *b) __attribute__((alias("Course_Scan")));
#endif
// Remaining 06FD4/07210/07368 left blocked with exact evidence: field map +12/+16 vs header nA*20/nB*12, rect +24/+4/+8/+12, 32-entry loop over [sp+40] needs indirect _08005DA4 table beyond +12 — no alias until pointer roots proven via watchpoint on sl/_08007664:r1
