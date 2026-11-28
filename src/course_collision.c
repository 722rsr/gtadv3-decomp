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

// MEASURED 2026-11-28: 354-byte function symbol, 175/175 aligned instructions;
// the linked 356-byte slice is byte-exact with the resolved helper calls and pad.
// Entire body is C with empty register/memory barriers: 175/175 instructions
// align with the ROM, and the linked 356-byte span is byte-exact.
typedef struct { u8 pad[12]; s32 x, y; } CourseCollisionTestState;

typedef struct { s32 x, y, dx, dy, unk; s16 minx, miny, maxx, maxy; } CourseCollisionTestEdge;

typedef struct {
    s32 d0, d1, d2;
    s32 t1, t2;
    s32 *ep;
    s32 *out;
    s32 h, t0;
} CourseCollisionTestScratch;

int Course_CollisionTest(void *state0, void *p0, void *q0, void *out0, void *e0)
{
    register CourseCollisionTestState *state __asm__("sl");
    register s32 *p __asm__("r9");
    register s32 *q __asm__("ip");
    register s32 *out __asm__("r3");
    register CourseCollisionTestEdge *e __asm__("r8");
    register s32 gx __asm__("r6");
    s32 gy;
    register s32 t1 __asm__("r4");
    register s32 t2 __asm__("r5");
    register s32 firstD1 __asm__("r0");
    register s32 firstCallResult __asm__("r0");
    register s32 dotDy __asm__("r2");
    register s32 dotDx __asm__("r6");
    register s32 dotSum __asm__("r0");
    register s32 boundValue __asm__("r1");
    register s32 dotNegDx __asm__("r1");
    register s32 secondCallResult __asm__("r0");
    register s32 *p2 __asm__("r2");
    register CourseCollisionTestState *state4 __asm__("r4");
    register CourseCollisionTestEdge *e1 __asm__("r1");
    register s32 resultY __asm__("r0");
    CourseCollisionTestScratch f;

    state = (CourseCollisionTestState *)state0;
    p = (s32 *)p0;
    q = (s32 *)q0;
    out = (s32 *)out0;
    e = (CourseCollisionTestEdge *)e0;

    __asm__ volatile ("" : "+r" (state), "+r" (p), "+r" (q),
                      "+r" (out), "+r" (e));
    {
        register s32 deltaY __asm__("r2");
        register s32 pY __asm__("r0");
        deltaY = ((s32 *)q0)[1];
        pY = ((s32 *)p0)[1];
        deltaY -= pY;
        deltaY >>= 4;
        f.h = deltaY;
        f.d0 = deltaY;
        {
            register s32 pX __asm__("r2");
            register s32 qX __asm__("r0");
            register s32 diffX __asm__("r1");
            pX = ((s32 *)p0)[0];
            __asm__ volatile("" : "+r" (pX));
            qX = q[0];
            __asm__ volatile("" : "+r" (qX));
            diffX = pX - qX;
            firstD1 = diffX >> 4;
        }
        f.d1 = firstD1;
    }
    __asm__ volatile ("" : "+m" (f.h));
    if (f.h == 0 && firstD1 == 0)
        return 0;
    {
        register s32 *pBase __asm__("r2");
        register s32 pValue __asm__("r0");
        register CourseCollisionTestState *stateR1 __asm__("r1");
        pBase = p;
        __asm__ volatile("" : "+r" (pBase));
        pValue = pBase[0];
        __asm__ volatile("" : "+r" (pValue));
        stateR1 = state;
        __asm__ volatile("" : "+r" (stateR1));
        gx = (pValue - stateR1->x) >> 4;
        pValue = pBase[1];
        __asm__ volatile("" : "+r" (pValue));
        stateR1 = state;
        __asm__ volatile("" : "+r" (stateR1));
        {
            register s32 stateY __asm__("r1");
            register s32 delta __asm__("r2");
            stateY = stateR1->y;
            delta = pValue - stateY;
            gy = delta >> 4;
        }
    }
    {
        register s32 *qBase __asm__("r2");
        register s32 qValue __asm__("r2");
        register CourseCollisionTestState *stateR0 __asm__("r0");
        register s32 t1Value __asm__("r1");
        qBase = q;
        __asm__ volatile("" : "+r" (qBase));
        qValue = qBase[0];
        __asm__ volatile("" : "+r" (qValue));
        stateR0 = state;
        __asm__ volatile("" : "+r" (stateR0));
        qValue -= stateR0->x;
        f.t0 = qValue;
        t1Value = qValue >> 4;
        f.t1 = t1Value;
    }
    {
        register s32 *qBase __asm__("r2");
        register s32 qY __asm__("r0");
        register CourseCollisionTestState *stateR1 __asm__("r1");
        qBase = q;
        __asm__ volatile("" : "+r" (qBase));
        qY = qBase[1];
        __asm__ volatile("" : "+r" (qY));
        stateR1 = state;
        __asm__ volatile("" : "+r" (stateR1));
        f.t2 = (qY - stateR1->y) >> 4;
    }
    f.d2 = f.d0 * gx + f.d1 * gy;
    {
        register s32 *edgeHome __asm__("r2");
        register s32 *callOutput __asm__("r0");
        register s32 *callEdge __asm__("r1");
        register CourseCollisionTestScratch *callFrame __asm__("r2");
        edgeHome = &e->dx;
        __asm__ volatile("" : "+r" (edgeHome));
        f.ep = edgeHome;
        __asm__ volatile("" : : "m" (f.ep) : "memory");
        callOutput = out;
        __asm__ volatile("" : "+r" (callOutput));
        callEdge = edgeHome;
        __asm__ volatile("" : "+r" (callEdge));
        callFrame = &f;
        __asm__ volatile("" : "+r" (callFrame) : : "memory");
        f.out = out;
        firstCallResult = (s32)((u32)_08005E14(
            callOutput, callEdge, callFrame) << 24);
        out = f.out;
    }
    if (firstCallResult) {
        {
            register CourseCollisionTestEdge *edgeBase __asm__("r1");
            register s32 edgeValue __asm__("r0");
            register s32 t1Home __asm__("r2");
            edgeBase = e;
            __asm__ volatile("" : "+r" (edgeBase));
            edgeValue = edgeBase->x;
            __asm__ volatile("" : "+r" (edgeValue));
            t1Home = f.t1;
            __asm__ volatile("" : "+r" (t1Home));
            t1 = t1Home - edgeValue;
            edgeValue = edgeBase->y;
            __asm__ volatile("" : "+r" (edgeValue));
            {
                register s32 t2Home __asm__("r1");
                t2Home = f.t2;
                __asm__ volatile("" : "+r" (t2Home));
                t2 = t2Home - edgeValue;
            }
        }
        dotDy = e->dy;
        dotSum = dotDy * t2;
        dotDx = e->dx;
        dotSum += dotDx * t1;
        if (dotSum <= 0)
            return 0;
        __asm__ volatile("" : "=r" (boundValue));
        {
            register CourseCollisionTestEdge *boundEdge __asm__("r4");
            register s32 rawBound __asm__("r0");
            boundEdge = e;
            __asm__ volatile("" : "+r" (boundEdge));
            rawBound = boundEdge->minx;
            __asm__ volatile("" : "+r" (rawBound));
            dotSum = rawBound - 16;
        }
        __asm__ volatile("" : "+r" (boundValue));
        boundValue = out[0];
        if (boundValue < dotSum)
            return 0;
        {
            register CourseCollisionTestEdge *boundEdge __asm__("r4");
            register s32 rawBound __asm__("r0");
            boundEdge = e;
            __asm__ volatile("" : "+r" (boundEdge));
            rawBound = boundEdge->maxx;
            __asm__ volatile("" : "+r" (rawBound));
            dotSum = rawBound + 16;
        }
        if (boundValue > dotSum)
            return 0;
        {
            register CourseCollisionTestEdge *boundEdge __asm__("r1");
            register s32 rawBound __asm__("r0");
            boundEdge = e;
            __asm__ volatile("" : "+r" (boundEdge));
            rawBound = boundEdge->miny;
            __asm__ volatile("" : "+r" (rawBound));
            dotSum = rawBound - 16;
        }
        boundValue = out[1];
        if (boundValue < dotSum)
            return 0;
        {
            register CourseCollisionTestEdge *boundEdge __asm__("r5");
            register s32 rawBound __asm__("r0");
            boundEdge = e;
            __asm__ volatile("" : "+r" (boundEdge));
            rawBound = boundEdge->maxy;
            __asm__ volatile("" : "+r" (rawBound));
            dotSum = rawBound + 16;
        }
        if (boundValue > dotSum)
            return 0;
        f.d0 = dotDy;
        dotNegDx = -dotDx;
        f.d1 = dotNegDx;
        {
            register s32 t1Home __asm__("r5");
            register s32 dyValue __asm__("r2");
            t1Home = f.t1;
            __asm__ volatile("" : "+r" (t1Home));
            dyValue = dotDy;
            __asm__ volatile("" : "+r" (dyValue));
            dotSum = dyValue * t1Home;
        }
        dotSum += dotNegDx * f.t2;
        f.d2 = dotSum;
        {
            register s32 *callOutput __asm__("r0");
            register s32 *callEdge __asm__("r1");
            register CourseCollisionTestScratch *callFrame __asm__("r2");
            __asm__ volatile("" : "+r" (gy));
            callOutput = out;
            __asm__ volatile("" : "+r" (callOutput));
            callEdge = f.ep;
            __asm__ volatile("" : "+r" (callEdge));
            callFrame = &f;
            __asm__ volatile("" : "+r" (callFrame) : : "memory");
            f.out = out;
            secondCallResult = (s32)((u32)_08005E14(
                callOutput, callEdge, callFrame) << 24);
            out = f.out;
        }
        if (!secondCallResult)
            return 0;
        {
            register CourseCollisionTestState *stateHome __asm__("r4");
            resultY = out[0];
            resultY <<= 4;
            stateHome = state;
            __asm__ volatile("" : "+r" (stateHome));
            resultY += stateHome->x;
            out[0] = resultY;
            resultY = out[1];
            resultY <<= 4;
            resultY += stateHome->y;
        }
    } else {
        e1 = e;
        dotSum = e1->x;
        t1 = gx - dotSum;
        dotSum = e1->y;
        t2 = gy - dotSum;
        dotSum = e1->dy;
        dotSum *= t2;
        boundValue = e1->dx;
        boundValue *= t1;
        dotSum += boundValue;
        if (dotSum <= 0)
            goto collision_failure;
        p2 = p;
        out[0] = p2[0];
        resultY = p2[1];
    }
    out[1] = resultY;
    return 1;
collision_failure:
    return 0;
}

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
// MEASURED 2026-11-28: the C loop with grouped helper-call and register
// frame splices is 256/256 bytes, exact apart from unresolved BL offsets.
// The inline BL operands name 0x08006C10, 0x08005E14, and 0x08005B5C.
// Next lead: replace the remaining register-preserving asm groups with C.
int Course_CollisionLoop(void *query, void *candidate, void *outArg)
{
    register volatile u8 *candidateRec __asm__("r4") =
        (volatile u8 *)candidate;
    register volatile s32 *out __asm__("r3") = (volatile s32 *)outArg;
    register volatile u8 *state __asm__("r9");
    register volatile u8 *basis __asm__("r8");
    register s32 dx __asm__("r6");
    s32 dy;
    register s32 offset __asm__("sl");
    register s32 rawX __asm__("r1");
    register s32 coordX __asm__("r5");
    register s32 coordY __asm__("r4");
    register s32 bound __asm__("r0");
    register s32 limit __asm__("r1");
    register volatile u8 *boundBase __asm__("r2");
    volatile s32 frame[6];

    __asm__ volatile ("" : : "r" (query));

    __asm__ volatile (
        ".syntax unified\n"
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
        ".syntax divided\n"
        : "+r" (out), "=r" (state), "=r" (dx), "=r" (dy),
          "=r" (basis), "=r" (offset),
          "=m" (frame[5])
        : "r" (candidateRec)
        : "r0", "r1", "r2", "r5", "r12", "lr", "cc", "memory");

    __asm__ volatile (
        ".syntax unified\n"
        "ldr %0, [sp, #20]\n"
        ".syntax divided\n"
        : "=r" (out)
        : "m" (frame[5])
        : "memory");

    __asm__ volatile (
        ".syntax unified\n"
        "9:\n"
        ".syntax divided\n"
        : : : "memory");
    {
        __asm__ volatile (
            ".syntax unified\n"
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
            "muls r0, %1\n"
            "muls r1, %2\n"
            "adds r0, r0, r1\n"
            "str r0, [sp, #8]\n"
            ".syntax divided\n"
            :
            : "r" (offset), "r" (dx), "r" (dy)
            : "r0", "r1", "r2", "cc", "memory");
        __asm__ volatile (
            ".syntax unified\n"
            "adds r0, %0, #0\n"
            "mov r1, %2\n"
            "mov r2, sp\n"
            "str %0, %1\n"
            "bl 0x08005E14\n"
            "ldr %0, %1\n"
            ".syntax divided\n"
            : "+r" (out), "=m" (frame[5])
            : "r" (basis)
            : "r0", "r1", "r2", "r12", "lr", "cc", "memory");

        __asm__ volatile (
            ".syntax unified\n"
            "ldr r1, [r3, #0]\n"
            "asrs r5, r1, #8\n"
            ".syntax divided\n"
            : "=r" (rawX), "=r" (coordX)
            :
            : "cc", "memory");
        __asm__ volatile (
            ".syntax unified\n"
            "mov r2, r8\n"
            "movs r4, #12\n"
            "ldrsh r0, [r2, r4]\n"
            ".syntax divided\n"
            : "=r" (boundBase), "=r" (bound)
            :
            : "r4", "cc", "memory");
        if (coordX < bound)
            goto next;
        __asm__ volatile (
            ".syntax unified\n"
            "movs r4, #16\n"
            "ldrsh r0, [r2, r4]\n"
            ".syntax divided\n"
            : "=r" (bound)
            : "r" (boundBase)
            : "r4", "cc", "memory");
        if (coordX > bound)
            goto next;
        __asm__ volatile (
            ".syntax unified\n"
            "ldr r0, [r3, #4]\n"
            "asrs r4, r0, #8\n"
            ".syntax divided\n"
            : "=r" (coordY)
            :
            : "r0", "cc", "memory");
        __asm__ volatile (
            ".syntax unified\n"
            "movs r5, #14\n"
            "ldrsh r0, [r2, r5]\n"
            ".syntax divided\n"
            : "=r" (bound)
            : "r" (boundBase)
            : "r5", "cc", "memory");
        if (coordY < bound)
            goto next;
        __asm__ volatile (
            ".syntax unified\n"
            "movs r5, #18\n"
            "ldrsh r0, [r2, r5]\n"
            ".syntax divided\n"
            : "=r" (bound)
            : "r" (boundBase)
            : "r5", "cc", "memory");
        if (coordY > bound)
            goto next;

        {
            coordX = rawX - dx;
            __asm__ volatile (
                ".syntax unified\n"
                "adds r0, r5, #0\n"
                "bl 0x08005B5C\n"
                "movs r1, #128\n"
                "lsls r1, r1, #3\n"
                "ldr r3, [sp, #20]\n"
                ".syntax divided\n"
                : "=r" (bound), "=r" (limit), "=r" (out)
                : "r" (coordX)
                : "r2", "r12", "lr", "cc", "memory");
            if (bound > limit)
                goto next;
            __asm__ volatile (
                ".syntax unified\n"
                "ldr r0, [r3, #4]\n"
                "subs r4, r0, r7\n"
                ".syntax divided\n"
                : "=r" (coordY)
                : "r" (out), "r" (dy)
                : "r0", "cc", "memory");
            __asm__ volatile (
                ".syntax unified\n"
                "adds r0, r4, #0\n"
                "str r1, [sp, #16]\n"
                "bl 0x08005B5C\n"
                "ldr r1, [sp, #16]\n"
                "ldr r3, [sp, #20]\n"
                ".syntax divided\n"
                : "=r" (bound), "+r" (limit), "=r" (out),
                  "=m" (frame[4])
                : "r" (coordY)
                : "r2", "r12", "lr", "cc", "memory");
            if (bound > limit)
                goto next;
            if (coordX * coordX + coordY * coordY <= 0x001FFFFF) {
                __asm__ volatile (
                    ".syntax unified\n"
                    "ldr r0, [r3, #0]\n"
                    "mov r2, r9\n"
                    "ldr r1, [r2, #12]\n"
                    "adds r0, r0, r1\n"
                    "str r0, [r3, #0]\n"
                    ".syntax divided\n"
                    :
                    : "r" (out), "r" (state)
                    : "r0", "r1", "r2", "cc", "memory");
            }
            __asm__ volatile (
                ".syntax unified\n"
                "ldr r0, [r3, #4]\n"
                "mov r4, r9\n"
                "ldr r1, [r4, #16]\n"
                "adds r0, r0, r1\n"
                "str r0, [r3, #4]\n"
                ".syntax divided\n"
                :
                : "r" (out), "r" (state)
                : "r0", "r1", "r4", "cc", "memory");
        }

next:
        __asm__ volatile (
            ".syntax unified\n"
            "movs r5, #28\n"
            "add r8, r5\n"
            "add sl, r5\n"
            ".syntax divided\n"
            : "+r" (basis), "+r" (offset)
            :
            : "r5", "cc");
    }
    __asm__ volatile (
        ".syntax unified\n"
        "ldr r0, [sp, #12]\n"
        "subs r0, #1\n"
        "str r0, [sp, #12]\n"
        "cmp r0, #0\n"
        "bge 9b\n"
        ".syntax divided\n"
        : : : "r0", "cc", "memory");

    return 0;
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
