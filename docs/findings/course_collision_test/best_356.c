// Best measured C lift of 0x08006E70 (Course_CollisionTest), 2026-11-04.
// Probe: EXACT length 356 B, 66/356 bytes. Not landed: the body is not
// byte-exact, so src/course_collision.c keeps its asm body.
// Measured with tools/corpus_match_probe.py --function Course_CollisionTest.

typedef struct { u8 pad[12]; s32 x, y; } State;
typedef struct { s32 x, y, dx, dy; s16 minx, miny, maxx, maxy; } Col;
int Course_CollisionTest(void *state0, void *p0, void *q0, void *out0, void *e0)
{
    register State *state __asm__("sl") = (State *)state0;
    s32 *p = (s32 *)p0;
    register s32 *q __asm__("ip") = (s32 *)q0;
    register s32 *out __asm__("r3") = (s32 *)out0;
    register Col *e __asm__("r8") = (Col *)e0;
    s32 d[3];
    s32 h, t0, t1, t2, gx, gy;
    s32 *ep;

    h = (q[1] - p[1]) >> 4;
    d[0] = h;
    d[1] = (p[0] - q[0]) >> 4;
    if (h == 0 && d[1] == 0)
        return 0;
    {
        gx = (p[0] - state->x) >> 4;
        gy = (p[1] - state->y) >> 4;
        t2 = (q[1] - state->y) >> 4;
        t0 = q[0] - state->x;
        t1 = t0 >> 4;
        d[2] = d[0] * gx + d[1] * gy;
    }
    ep = &e->dx;
    if (_08005E14(out, ep, d)) {
        if (e->dx * (t2 - e->y) + e->dy * (t1 - e->x) <= 0)
            return 0;
        if (out[0] < e->minx - 16)
            return 0;
        if (out[0] > e->maxx + 16)
            return 0;
        if (out[1] < e->miny - 16)
            return 0;
        if (out[1] > e->maxy + 16)
            return 0;
        d[0] = e->dx;
        d[1] = -e->dy;
        d[2] = e->dx * t1 + d[1] * t2;
        if (!_08005E14(out, ep, d))
            return 0;
        out[0] = (out[0] << 4) + state->x;;
        out[1] = (out[1] << 4) + state->y;;
        return 1;
    }
    if (e->dx * (gy - e->y) + e->dy * (gx - e->x) <= 0)
        return 0;
    out[0] = p[0];
    out[1] = p[1];
    return 1;
}
