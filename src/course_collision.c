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
extern u8 _08018A6C(s16 idx);
extern void _08006A50(int camX, int camY);
extern void *_08006AEC(void);
extern void *_08005F98(void);
extern int _08006E70(void *a,void *b,void *c,void *d,void *e);

// _08006E70 — 189 L, pool-free, 5 args (r0=sl, r1=r9, r2=ip, r3=?, [sp+68]=r8)
// MEASURED 2026-11-04: the 36-byte frame is one local scratch object, not six
// spills (d 0/4/8, t1 12, t2 16, ep 20, out 24, h 28, t0 32). Spelling it as a
// struct reproduces the frame exactly; best full lift is 360/356 bytes at
// 30/356 equal (draft: docs/findings/course_collision_test/frame36.c). The
// residual is the prologue staging set: the ROM saves r4-r7 and stages
// sl/r9/r8 through r7/r6/r5, the draft saves r4-r6 because r7 is not marked
// live. NEXT: get r7 allocated, not pinned.
__attribute__((naked)) int Course_CollisionTest(void *sl, void *par, void *ip, void *out, void *extra) {
    __asm__ volatile (
        ".syntax unified\n"
        ".L6e70_08006E70:\n"
        "        push {r4, r5, r6, r7, lr}\n"
        "        mov r7, sl\n"
        "        mov r6, r9\n"
        "        mov r5, r8\n"
        "        push {r5, r6, r7}\n"
        "        sub sp, #36\n"
        "        mov sl, r0\n"
        "        mov r9, r1\n"
        "        mov ip, r2\n"
        "        ldr r0, [sp, #68]\n"
        "        mov r8, r0\n"
        "        ldr r2, [r2, #4]\n"
        "        ldr r0, [r1, #4]\n"
        "        subs r2, r2, r0\n"
        "        asrs r2, r2, #4\n"
        "        str r2, [sp, #28]\n"
        "        str r2, [sp, #0]\n"
        "        ldr r2, [r1, #0]\n"
        "        mov r0, ip\n"
        "        ldr r0, [r0, #0]\n"
        "        subs r1, r2, r0\n"
        "        asrs r0, r1, #4\n"
        "        str r0, [sp, #4]\n"
        "        ldr r1, [sp, #28]\n"
        "        cmp r1, #0\n"
        "        bne.n .L6e70_08006EAA\n"
        "        cmp r0, #0\n"
        "        bne.n .L6e70_08006EAA\n"
        "        b.n .L6e70_08006FC0\n"
        ".L6e70_08006EAA:\n"
        "        mov r2, r9\n"
        "        ldr r0, [r2, #0]\n"
        "        mov r1, sl\n"
        "        ldr r1, [r1, #12]\n"
        "        subs r0, r0, r1\n"
        "        asrs r6, r0, #4\n"
        "        ldr r0, [r2, #4]\n"
        "        mov r1, sl\n"
        "        ldr r1, [r1, #16]\n"
        "        subs r2, r0, r1\n"
        "        asrs r7, r2, #4\n"
        "        mov r2, ip\n"
        "        ldr r2, [r2, #0]\n"
        "        mov r0, sl\n"
        "        ldr r0, [r0, #12]\n"
        "        subs r2, r2, r0\n"
        "        str r2, [sp, #32]\n"
        "        asrs r1, r2, #4\n"
        "        str r1, [sp, #12]\n"
        "        mov r2, ip\n"
        "        ldr r0, [r2, #4]\n"
        "        mov r1, sl\n"
        "        ldr r1, [r1, #16]\n"
        "        subs r0, r0, r1\n"
        "        asrs r0, r0, #4\n"
        "        str r0, [sp, #16]\n"
        "        ldr r0, [sp, #0]\n"
        "        muls r0, r6\n"
        "        ldr r1, [sp, #4]\n"
        "        muls r1, r7\n"
        "        adds r0, r0, r1\n"
        "        str r0, [sp, #8]\n"
        "        mov r2, r8\n"
        "        adds r2, #8\n"
        "        str r2, [sp, #20]\n"
        "        adds r0, r3, #0\n"
        "        adds r1, r2, #0\n"
        "        mov r2, sp\n"
        "        str r3, [sp, #24]\n"
        "        bl 0x08005E14\n"
        "        lsls r0, r0, #24\n"
        "        ldr r3, [sp, #24]\n"
        "        cmp r0, #0\n"
        "        beq.n .L6e70_08006F9A\n"
        "        mov r1, r8\n"
        "        ldr r0, [r1, #0]\n"
        "        ldr r2, [sp, #12]\n"
        "        subs r4, r2, r0\n"
        "        ldr r0, [r1, #4]\n"
        "        ldr r1, [sp, #16]\n"
        "        subs r5, r1, r0\n"
        "        mov r0, r8\n"
        "        ldr r2, [r0, #12]\n"
        "        adds r0, r2, #0\n"
        "        muls r0, r5\n"
        "        mov r1, r8\n"
        "        ldr r6, [r1, #8]\n"
        "        adds r1, r6, #0\n"
        "        muls r1, r4\n"
        "        adds r0, r0, r1\n"
        "        cmp r0, #0\n"
        "        ble.n .L6e70_08006FC0\n"
        "        mov r4, r8\n"
        "        movs r5, #20\n"
        "        ldrsh r0, [r4, r5]\n"
        "        subs r0, #16\n"
        "        ldr r1, [r3, #0]\n"
        "        cmp r1, r0\n"
        "        blt.n .L6e70_08006FC0\n"
        "        mov r4, r8\n"
        "        movs r5, #24\n"
        "        ldrsh r0, [r4, r5]\n"
        "        adds r0, #16\n"
        "        cmp r1, r0\n"
        "        bgt.n .L6e70_08006FC0\n"
        "        mov r1, r8\n"
        "        movs r4, #22\n"
        "        ldrsh r0, [r1, r4]\n"
        "        subs r0, #16\n"
        "        ldr r1, [r3, #4]\n"
        "        cmp r1, r0\n"
        "        blt.n .L6e70_08006FC0\n"
        "        mov r5, r8\n"
        "        movs r4, #26\n"
        "        ldrsh r0, [r5, r4]\n"
        "        adds r0, #16\n"
        "        cmp r1, r0\n"
        "        bgt.n .L6e70_08006FC0\n"
        "        str r2, [sp, #0]\n"
        "        negs r1, r6\n"
        "        str r1, [sp, #4]\n"
        "        ldr r5, [sp, #12]\n"
        "        adds r0, r2, #0\n"
        "        muls r0, r5\n"
        "        ldr r2, [sp, #16]\n"
        "        muls r1, r2\n"
        "        adds r0, r0, r1\n"
        "        str r0, [sp, #8]\n"
        "        adds r0, r3, #0\n"
        "        ldr r1, [sp, #20]\n"
        "        mov r2, sp\n"
        "        str r3, [sp, #24]\n"
        "        bl 0x08005E14\n"
        "        lsls r0, r0, #24\n"
        "        ldr r3, [sp, #24]\n"
        "        cmp r0, #0\n"
        "        beq.n .L6e70_08006FC0\n"
        "        ldr r0, [r3, #0]\n"
        "        lsls r0, r0, #4\n"
        "        mov r4, sl\n"
        "        ldr r1, [r4, #12]\n"
        "        adds r0, r0, r1\n"
        "        str r0, [r3, #0]\n"
        "        ldr r0, [r3, #4]\n"
        "        lsls r0, r0, #4\n"
        "        ldr r1, [r4, #16]\n"
        "        adds r0, r0, r1\n"
        "        b.n .L6e70_08006FBA\n"
        ".L6e70_08006F9A:\n"
        "        mov r1, r8\n"
        "        ldr r0, [r1, #0]\n"
        "        subs r4, r6, r0\n"
        "        ldr r0, [r1, #4]\n"
        "        subs r5, r7, r0\n"
        "        ldr r0, [r1, #12]\n"
        "        muls r0, r5\n"
        "        ldr r1, [r1, #8]\n"
        "        muls r1, r4\n"
        "        adds r0, r0, r1\n"
        "        cmp r0, #0\n"
        "        ble.n .L6e70_08006FC0\n"
        "        mov r2, r9\n"
        "        ldr r0, [r2, #0]\n"
        "        str r0, [r3, #0]\n"
        "        ldr r0, [r2, #4]\n"
        ".L6e70_08006FBA:\n"
        "        str r0, [r3, #4]\n"
        "        movs r0, #1\n"
        "        b.n .L6e70_08006FC2\n"
        ".L6e70_08006FC0:\n"
        "        movs r0, #0\n"
        ".L6e70_08006FC2:\n"
        "        add sp, #36\n"
        "        pop {r3, r4, r5}\n"
        "        mov r8, r3\n"
        "        mov r9, r4\n"
        "        mov sl, r5\n"
        "        pop {r4, r5, r6, r7}\n"
        "        pop {r1}\n"
        "        bx r1\n"
        "        .short 0x0000\n"
        ".syntax divided\n"
    );
}

// _08006FD4 — 166 L, 4 args (r0=sl,r1=r9,r2=r8,r3=r5), pool-free, s32 diff>>4, origin 0x0805DBF4
// Complete CFG with explicit stack temporaries at sp+0/4/8/12, high-reg r8/sl/r9, s16 widths, proven helpers
int Course_CollisionMore(void *state0, void *par0, void *pos0, void *out0) {
    void *state = state0;
    register void *par asm("r9") = par0;
    register void *u asm("r8") = pos0;
    register s32 *out asm("r5") = (s32 *)out0;
    register void *rec asm("r6");
    register void *result asm("sl") = 0;
    s32 v[4];
    void *arg;
    register s32 px asm("ip");
    register s32 py asm("r3");
    s32 *rp;
    s32 p2;
    register s32 reserved asm("r4");

    rec = _08006C10(state, 0);
    if (rec == 0) goto done;
    __asm__ volatile("" : "=r"(reserved));
    {
        register s32 *p1 asm("r1");
        register s32 t0 asm("r2");
        p1 = (s32 *)u;
        v[0] = (px = p1[0]) - (t0 = *(s32 *)((u8 *)rec + 12));
    }
    __asm__ volatile("" : "+r"(reserved));
    {
        register s32 t1 asm("r1");
        reserved = (s32)u;
        v[1] = (py = ((s32 *)reserved)[1]) - (t1 = *(s32 *)((u8 *)rec + 16));
    }
    {
        register s32 t2 asm("r0");
        t2 = *(s16 *)((u8 *)rec + 26);
        __asm__ volatile("" : : "r"(t2));
        u = (void *)t2;
        __asm__ volatile("" : : "r"(u));
    }
    u = (void *)((s32)u * v[0]);
    __asm__ volatile("" : : "r"(u));
    {
        register s32 t3 asm("r0");
        t3 = *(s16 *)((u8 *)rec + 24);
        __asm__ volatile("" : : "r"(t3));
        p2 = t3 * v[1];
        __asm__ volatile("" : : "r"(p2) : "r2");
    }
    {
        register s32 v2 asm("r2");
        reserved = (s32)u;
        v2 = reserved - p2;
        v[2] = v2;
        u = 0;
        reserved = (s32)px;
        if (v2 <= 0) {
            u = (void *)-1;
            if (v2 < 0)
                u = (void *)1;
        }
    }
    v[0] = (s32)reserved;
    v[1] = py;
    if (*(u8 *)(*(void **)state) == 0) {
        if (_08006C10(state, -1) == 0) {
            arg = (u8 *)(*(void **)state) + 4;
            if ((s32)(_08006E70(rec, par, v, out, arg) << 24) > 0) {
                result = (u8 *)(*(void **)state) + 12;
                v[0] = out[0];
                v[1] = out[1];
            }
        }
        if (_08006C10(state, 1) == 0) {
            arg = (u8 *)(*(void **)state) + 32;
            if ((s32)(_08006E70(rec, par, v, out, arg) << 24) > 0) {
                result = (u8 *)(*(void **)state) + 40;
                v[0] = out[0];
                v[1] = out[1];
            }
        }
    }
    if ((s32)u >= 0) {
        register s32 off asm("r0") = (s32)u * 28 + 32;
        register void *arg4 asm("r4") = (u8 *)rec + off;
        if ((s32)(_08006E70(rec, par, v, out, arg4) << 24) > 0) {
            result = (u8 *)arg4 + 8;
            v[0] = out[0];
            v[1] = out[1];
        }
    }
done:
    if (result == 0)
        return 0;
    out[0] = v[0];
    out[1] = v[1];
    rp = (s32 *)result;
    out[2] = rp[0];
    out[3] = rp[1];
    return 1;
}
int _08006FD4(void *a,void *b,void *c,void *d) __attribute__((alias("Course_CollisionMore")));

// _08007110 — 135 L, pool 0x001FFFFF at 0x720C s32, high-reg, s32 diffs + s16 bounds + 0x400 bound via 05B5C
//
// MEASURED 2026-10-29: a full C body reaches the exact 256 B span and 58/123
// aligned rows. dx/dy/basis/ip pin cleanly to the ROM's r6/r7/r8/r9, so the
// whole residual is one variable: `out`. The ROM spills it to sp+20 and
// reloads r3 after every call; agbcc keeps it in a callee-saved register.
// Measured spellings: a plain `s32 *o` (r5), `s32 *volatile o` (double store,
// 276 B), a one-element array home (r7, which then clobbers the dy pin and
// miscompiles), and an `asm("" : "+m"(o))` barrier (two stores, 260 B).
// Pinning `o` to r3 is not safe: agbcc treats a hard register as callee-saved,
// so it would not reload after the `bl`s. Closing this needs a lever that
// forces a stack home without an extra store.
__attribute__((naked)) int Course_CollisionLoop(void *query, void *candidate, void *out) {
    __asm__ volatile (
        ".syntax unified\n"
        "push {r4, r5, r6, r7, lr}\n"
        "mov r7, sl\n"
        "mov r6, r9\n"
        "mov r5, r8\n"
        "push {r5, r6, r7}\n"
        "sub sp, #24\n"
        "adds r4, r1, #0\n"
        "adds r3, r2, #0\n"
        "movs r1, #0\n"
        "str r3, [sp, #20]\n"
        "bl 0x08006C10\n"
        "mov r9, r0\n"
        "ldr r1, [r4, #0]\n"
        "ldr r0, [r0, #12]\n"
        "subs r6, r1, r0\n"
        "ldr r1, [r4, #4]\n"
        "mov r2, r9\n"
        "ldr r0, [r2, #16]\n"
        "subs r7, r1, r0\n"
        "movs r4, #40\n"
        "add r4, r9\n"
        "mov r8, r4\n"
        "movs r5, #0\n"
        "mov sl, r5\n"
        "movs r0, #1\n"
        "str r0, [sp, #12]\n"
        "ldr r3, [sp, #20]\n"
        ".Lcourse7110_08007148:\n"
        "mov r0, r9\n"
        "adds r0, #44\n"
        "add r0, sl\n"
        "ldr r2, [r0, #0]\n"
        "str r2, [sp, #0]\n"
        "mov r0, r9\n"
        "adds r0, #40\n"
        "add r0, sl\n"
        "ldr r1, [r0, #0]\n"
        "negs r1, r1\n"
        "str r1, [sp, #4]\n"
        "adds r0, r2, #0\n"
        "muls r0, r6\n"
        "muls r1, r7\n"
        "adds r0, r0, r1\n"
        "str r0, [sp, #8]\n"
        "adds r0, r3, #0\n"
        "mov r1, r8\n"
        "mov r2, sp\n"
        "str r3, [sp, #20]\n"
        "bl 0x08005E14\n"
        "ldr r3, [sp, #20]\n"
        "ldr r1, [r3, #0]\n"
        "asrs r5, r1, #8\n"
        "mov r2, r8\n"
        "movs r4, #12\n"
        "ldrsh r0, [r2, r4]\n"
        "cmp r5, r0\n"
        "blt.n .Lcourse7110_080071EA\n"
        "movs r4, #16\n"
        "ldrsh r0, [r2, r4]\n"
        "cmp r5, r0\n"
        "bgt.n .Lcourse7110_080071EA\n"
        "ldr r0, [r3, #4]\n"
        "asrs r4, r0, #8\n"
        "movs r5, #14\n"
        "ldrsh r0, [r2, r5]\n"
        "cmp r4, r0\n"
        "blt.n .Lcourse7110_080071EA\n"
        "movs r5, #18\n"
        "ldrsh r0, [r2, r5]\n"
        "cmp r4, r0\n"
        "bgt.n .Lcourse7110_080071EA\n"
        "subs r5, r1, r6\n"
        "adds r0, r5, #0\n"
        "bl 0x08005B5C\n"
        "movs r1, #128\n"
        "lsls r1, r1, #3\n"
        "ldr r3, [sp, #20]\n"
        "cmp r0, r1\n"
        "bgt.n .Lcourse7110_080071EA\n"
        "ldr r0, [r3, #4]\n"
        "subs r4, r0, r7\n"
        "adds r0, r4, #0\n"
        "str r1, [sp, #16]\n"
        "bl 0x08005B5C\n"
        "ldr r1, [sp, #16]\n"
        "ldr r3, [sp, #20]\n"
        "cmp r0, r1\n"
        "bgt.n .Lcourse7110_080071EA\n"
        "adds r0, r5, #0\n"
        "muls r0, r5\n"
        "adds r1, r4, #0\n"
        "muls r1, r4\n"
        "adds r0, r0, r1\n"
        "ldr r1, .Lcourse7110_0800720C\n"
        "cmp r0, r1\n"
        "bgt.n .Lcourse7110_080071E0\n"
        "ldr r0, [r3, #0]\n"
        "mov r2, r9\n"
        "ldr r1, [r2, #12]\n"
        "adds r0, r0, r1\n"
        "str r0, [r3, #0]\n"
        ".Lcourse7110_080071E0:\n"
        "ldr r0, [r3, #4]\n"
        "mov r4, r9\n"
        "ldr r1, [r4, #16]\n"
        "adds r0, r0, r1\n"
        "str r0, [r3, #4]\n"
        ".Lcourse7110_080071EA:\n"
        "movs r5, #28\n"
        "add r8, r5\n"
        "add sl, r5\n"
        "ldr r0, [sp, #12]\n"
        "subs r0, #1\n"
        "str r0, [sp, #12]\n"
        "cmp r0, #0\n"
        "bge.n .Lcourse7110_08007148\n"
        "movs r0, #0\n"
        "add sp, #24\n"
        "pop {r3, r4, r5}\n"
        "mov r8, r3\n"
        "mov r9, r4\n"
        "mov sl, r5\n"
        "pop {r4, r5, r6, r7}\n"
        "pop {r1}\n"
        "bx r1\n"
        ".Lcourse7110_0800720C: .word 0x001FFFFF\n"
        ".syntax divided\n"
    );
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
    {
        u32 rot[2] = {0, 0};
        _08005BA8(rot, (s16)(u16)(uintptr_t)c);
        (void)rot;
        // Remainder (05DA4 2-pass loop + 05E14 hit test) stays untranscribed.
        (void)b; (void)d;
        return 0;
    }
}

// _08006050 — scan helper, 78 instructions, no pool. Record bounds and car
// indices are signed 16-bit fields; the coordinate pair is in 20.12 units.
int Course_Scan(void *outPair, void *inPair) {
    register volatile u8 *out __asm__("r9") = (volatile u8 *)outPair;
    register volatile u8 *delta __asm__("sl") = (volatile u8 *)inPair;
    register volatile u8 *record __asm__("r8");
    register volatile u8 *recordLow __asm__("r0");
    register volatile u8 *entry __asm__("r4");
    register volatile u8 *second __asm__("r2");
    register volatile u8 *deltaLow __asm__("r1");
    register s32 index __asm__("r5");
    register s32 limit __asm__("r6");
    register s32 absValue __asm__("r0");
    register s32 last __asm__("r0");
    s32 offset;
    s32 camX;
    s32 camY;
    s32 dx;
    s32 dy;

    /* Keep the callee-save and loop registers aligned with the Thumb body. */
    camX = *(volatile s32 *)(out + 0);
    second = out;
    camY = *(volatile s32 *)(second + 4);
    _08006A50(camX, camY);
    record = (volatile u8 *)_08006AEC();
    while (record != 0) {
        recordLow = record;
        index = *(s16 *)(recordLow + 4);
        /* ROM gives the second signed halfword offset its own scratch (r2);
         * keeping r1 live across the load keeps agbcc off it. */
        {
            register int hold __asm__("r1");
            __asm__ volatile("" : "=r"(hold));
            last = *(s16 *)(recordLow + 6);
            __asm__ volatile("" : : "r"(hold));
            if (last >= index) {
                offset = index * 12;
                __asm__ volatile ("" : "+r"(offset));
                do {
                    recordLow = (volatile u8 *)_08005F98();
                    recordLow = (volatile u8 *)*(volatile u32 *)(recordLow + 12);
                    __asm__ volatile ("" : "+r"(recordLow));
                    entry = recordLow + offset;
                    if (((u32)_08018A6C(*(s16 *)(entry + 8)) << 24) != 0u) {
                        dx = *(volatile s32 *)(entry + 0);
                        __asm__ volatile ("" : : : "memory");
                        second = out;
                        dx -= *(volatile s32 *)(second + 0);
                        deltaLow = delta;
                        *(volatile s32 *)(deltaLow + 0) = dx;
                        absValue = _08005B5C(dx);
                        limit = 128;
                        limit <<= 3;
                        if (absValue <= limit) {
                            dy = *(volatile s32 *)(entry + 4);
                            __asm__ volatile ("" : : : "memory");
                            second = out;
                            dy -= *(volatile s32 *)(second + 4);
                            deltaLow = delta;
                            *(volatile s32 *)(deltaLow + 4) = dy;
                            absValue = _08005B5C(dy);
                            if (absValue <= limit)
                                return 12;
                        }
                    }
                    offset += 12;
                    index++;
                    second = record;
                } while (*(s16 *)(second + 6) >= index);
            }
            record = (volatile u8 *)_08006AEC();
        }
    }
    return 0;
}
void Course_StreamTail(void) {
    // stream_more / tail_more placeholders — DMA tail helpers, not yet lifted
}
int _08006E70(void *a,void *b,void *c,void *d,void *e) __attribute__((alias("Course_CollisionTest")));
int _08007110(void *a,void *b,void *c) __attribute__((alias("Course_CollisionLoop")));
int _08006050(void *a,void *b) __attribute__((alias("Course_Scan")));
// Remaining 06FD4/07210/07368 left blocked with exact evidence: field map +12/+16 vs header nA*20/nB*12, rect +24/+4/+8/+12, 32-entry loop over [sp+40] needs indirect _08005DA4 table beyond +12 — no alias until pointer roots proven via watchpoint on sl/_08007664:r1
