/* MEASURED 2026-11-04: frame-exact C lift of 0x08006E70 (Course_CollisionTest).
 * Not landed: 360 bytes for the 356-byte span, 30/356 equal. The 36-byte frame
 * and the batched high-register save are reproduced; the residual is the
 * prologue staging set. See docs/findings/course_collision_test_lift.md.
 * Rescore with: python3 tools/shape_search.py _08006E70 <this file>
 */
#include "gba/types.h"

typedef struct { u8 pad[12]; s32 x, y; } State;
typedef struct { s32 x, y, dx, dy, unk; s16 minx, miny, maxx, maxy; } Col;
extern int _08005E14(void *a, void *b, void *c);

typedef struct {
    s32 d0, d1, d2;
    s32 t1, t2;
    s32 *ep;
    s32 *out;
    s32 h, t0;
} Scratch;

int Course_CollisionTest(void *state0, void *p0, void *q0, void *out0, void *e0)
{
    register State *state __asm__("sl");
    register s32 *p __asm__("r9");
    register s32 *q __asm__("r5");
    s32 *out;
    register Col *e __asm__("r8");
    register s32 gx __asm__("r6");
    register s32 gy __asm__("r7");
    Scratch f;

    state = (State *)state0;
    p = (s32 *)p0;
    q = (s32 *)q0;
    out = (s32 *)out0;
    e = (Col *)e0;

    f.h = (q[1] - p[1]) >> 4;
    f.d0 = f.h;
    f.d1 = (p[0] - q[0]) >> 4;
    if (f.h == 0 && f.d1 == 0)
        return 0;
    gx = (p[0] - state->x) >> 4;
    gy = (p[1] - state->y) >> 4;
    f.t0 = q[0] - state->x;
    f.t1 = f.t0 >> 4;
    f.t2 = (q[1] - state->y) >> 4;
    f.d2 = f.d0 * gx + f.d1 * gy;
    f.ep = &e->dx;
    f.out = out0;
    if (_08005E14(f.out, f.ep, &f.d0)) {
        if (e->dy * (f.t2 - e->y) + e->dx * (f.t1 - e->x) <= 0)
            return 0;
        if (f.out[0] < e->minx - 16)
            return 0;
        if (f.out[0] > e->maxx + 16)
            return 0;
        if (f.out[1] < e->miny - 16)
            return 0;
        if (f.out[1] > e->maxy + 16)
            return 0;
        f.d0 = e->dx;
        f.d1 = -e->dy;
        f.d2 = e->dx * f.t1 + f.d1 * f.t2;
        if (!_08005E14(f.out, f.ep, &f.d0))
            return 0;
        f.out[0] = (f.out[0] << 4) + state->x;
        f.out[1] = (f.out[1] << 4) + state->y;
        return 1;
    }
    if (e->dy * (gy - e->y) + e->dx * (gx - e->x) <= 0)
        return 0;
    f.out[0] = p[0];
    f.out[1] = p[1];
    return 1;
}
