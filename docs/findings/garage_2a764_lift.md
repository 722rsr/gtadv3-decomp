# Lifting `_08002A764` (0x08002A764, 196 B): the unpinned base and two walls

Worked 2026-10-20. The body is still `naked` in `src/garage_records.c`; this
note records a C transcription that reaches **the exact ROM length (196 bytes,
196 candidate bytes) with 127 of those bytes matching**, plus the two things
that are left and the recipes that got it that far.

## The one big lever: an unpinned base is read through directly

The ROM reads the same record field three times through one register:

```
adds r7, r6, #0
adds r7, #110
movs r0, #0
ldrsh r1, [r7, r0]     <- read 1
...
movs r0, #0
ldrsh r2, [r7, r0]     <- read 2
...
ldrh r7, [r7, #0]      <- read 3 (the base dies here)
```

agbcc reproduces that shape only when the base is an **unpinned** local. With
`register u8 *p __asm__("r7")` the reload copies the base before every load
(`adds r0, r7, #0 / movs r1, #0 / ldrsh r0, [r0, r1]`), because a pinned
variable is a user variable and the load's address cannot reuse its register.
With a plain `u8 *p` the load reads through the base's own register:

```
adds r7, r0, #0
adds r7, #110
movs r1, #0
ldrsh r0, [r7, r1]
```

Measured in a scratch TU (`build/scratch/rd2.c`, `rd5.c`): a pinned base
copies, an unpinned one does not. The same is true of the address form -- a
*pointer* expression gives the ROM's two-instruction `adds r7, r6, #0 /
adds r7, #110`, while the same expression with a `(uintptr_t)` cast in the
middle gives the integer form `movs r0, #110 / adds r0, r0, r6`.

The four long-lived values (the two record pointers and the two field
pointers) are the ROM's r4-r7, and the unpinned locals are what put them
there; the candidate reaches the same four registers with no pins at all.

## The transcription

Replace the `__attribute__((naked))` body of `_08002A764` with this. It
compiles, it is the ROM's length, and 127 of 196 bytes match.

```c
int _08002A764(void *src_, void *dst_) {
    u8 *src = (u8 *)src_;
    u8 *dst = (u8 *)dst_;
    u8 *p110;
    u8 *p136;
    s32 a;
    s32 b;
    s32 c;
    s32 v;
    s32 w;
    s32 div_;

    p110 = src + 110;
    v = *(s16 *)p110;
    a = (s32)v * 2;
    a = (s32)*(u32 *)(src + 28) + a;
    a = a >> 4;
    *(u32 *)(dst + 20) = (u32)a;

    p136 = src + 136;
    w = *(s16 *)p136;
    b = (s32)w * 16;
    b = (s32)*(u32 *)(src + 32) + b;
    c = (s32)*(s16 *)p110;
    b = b + (c * 8 + c);
    b = b >> 4;
    *(u32 *)(dst + 24) = (u32)b;

    *(u32 *)(dst + 4) = *(u32 *)(src + 4);
    *(u32 *)(dst + 8) = *(u32 *)(src + 8);
    *(u32 *)(dst + 12) = *(u32 *)(src + 20);
    *(u32 *)(dst + 16) = *(u32 *)(src + 24);
    *(u16 *)(dst + 36) = *(u16 *)(src + 108);
    *(u32 *)(dst + 28) = (u32)(uintptr_t)(src + 36);
    *(u32 *)(dst + 32) = *(u32 *)(src + 92);
    div_ = (s32)*(s16 *)(src + 140);
    *(volatile u16 *)(dst + 40) = (u16)_08002DE04(div_, 5);
    *(u16 *)(dst + 38) = *(u16 *)(src + 128);
    *(u16 *)(dst + 42) = *(u16 *)(src + 116);
    {
        u16 lo_ = *(u16 *)p110;
        u16 hi_ = *(u16 *)p136;
        s32 sum_ = (s32)lo_ + (s32)hi_;
        *(u16 *)(dst + 44) = (u16)sum_;
        v = (s16)sum_;
    }
    if (v > 200) {
        *(u16 *)(dst + 44) = 200;
    } else if (v < -200) {
        *(u16 *)(dst + 44) = (u16)(-200);
    }
    if (*(volatile u8 *)(src + 156) == 0) {
        *(volatile u8 *)(dst + 48) = 1;
    } else {
        *(volatile u8 *)(dst + 48) = *(volatile u8 *)(src + 156);
    }
    *(volatile u8 *)(dst + 49) = *(volatile u8 *)(src + 158);
    return 0;
}
```

## The shapes that fall out of it

* **The read's width decides its form.** `*(s16 *)p` is the ROM's
  `movs rX,#0 / ldrsh rY,[base,rX]` (a non-volatile signed lvalue);
  `*(volatile s16 *)p` becomes `ldrh` plus a sign extension, and
  `*(s16 *)p` where `p` is already a `s16 *` becomes `ldrh` plus a sign
  extension too -- the cast from a `u8 *` is what keeps the signed form.
* **A volatile access computes its address; a plain one folds into the
  addressing mode.** The ROM's `adds r0, r6, #0 / adds r0, #156 / ldrb r1,
  [r0, #0]` is the volatile shape and `ldr r0, [r6, #28]` is the plain one.
  The byte reads at +156/+158 have to be volatile for that reason; the
  halfword reads at +108/+128/+116 do not, because their offsets are past
  the halfword immediate range and the address is computed anyway.
* **The value's live range steers the register.** Declaring the two signed
  reads as `s32` instead of `s16` moves them into the ROM's r1 and r0 and
  is worth 3 bytes. Reusing a variable later (the same `a` for the division
  argument, the same `v` for the clamp) keeps it alive and changes the
  allocation; giving the division argument its own variable is worth 1 byte
  more.

## What is left

Two things, both about *which* register rather than which instruction.

1. **The commutative add's operand order.** The ROM has
   `ldr r0,[r6,#28] / adds r0, r0, r1` -- the destination is also the first
   source -- and the candidate has `adds r0, r1, r0`. Both source orders
   (`a = word + a` and `a = a + word`) produce the candidate's form, so this
   is the compiler's canonicalisation, not the source.
2. **The clamp's two stores are merged.** The ROM stores 200 in one arm and
   -200 in the other (`strh r0,[r5,#44]` at +0x88 and `strh r1,[r5,#44]` at
   +0x94); the candidate if-converts them into one store after the join.
   An empty `asm` with a memory clobber between the arms does not separate
   them.

The remaining 44 differing offsets are all one of those two or the same
swap seen at a different point: the ROM's first read puts the scratch in r0
and the value in r1, the candidate the other way round, and the same swap
reappears at the byte reads in the tail.
