# Course_CollisionTest (0x08006E70, 356 B): the frame is a 36-byte scratch object

Still asm-owned. The ROM's frame is one object, not six independent spills:
`d[0..2]` at 0/4/8, then `t1` 12, `t2` 16, `ep` 20, `out` 24, `h` 28, `t0` 32.
Every access is a real load or store — `ep` is reloaded for *both* calls even
though it was just computed, and `d[0]` is reloaded for the muls — and the slot
order equals the field order. A plain-locals lift of the same body allocates 16
bytes; a local struct declared in that order allocates exactly 36 in every pin
set tried.

```c
typedef struct { s32 d0, d1, d2; s32 t1, t2; s32 *ep; s32 *out;
                 s32 h, t0; } Scratch;
```

Parameter copies are assignments, not initializers: `register T *v
__asm__("rN");` with a separate `v = arg;` emits the ROM's entry block
(`mov sl,r0 / mov r9,r1 / mov ip,r2 / ldr r0,[sp,#68] / mov r8,r0`) once three
high registers are pinned. Initializers coalesce with the incoming register and
place the copy at the first clobber instead, which shifts the whole body.

Measured best (`course_collision_test/frame36.c`, via
`tools/shape_search.py _08006E70 ...`): 360 bytes for the 356-byte span,
30/356 equal, frame 36, and the ROM's batched high-register save present.

Next lead: the staging set. agbcc stages that save through the highest saved
low registers — the ROM's `mov r7,sl / mov r6,r9 / mov r5,r8 / push {r5,r6,r7}`
needs r4-r7 all live, and the draft gets `r4,r5,r6` because r7 is not saved.
Pinning `gy` to `r7` does not mark it for saving (the value is used after both
calls and still is not pushed), so the lever is making agbcc allocate r7 on its
own rather than pinning it.
