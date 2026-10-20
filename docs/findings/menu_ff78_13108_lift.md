# Lifting `MenuFF78_13108` (0x08013108, 728 B) to C

Worked 2026-10-19/20. The body is still `naked` in `src/menu_ff78_r.c`;
this note records a C transcription that reaches **708 of 728 bytes** and the
two differences that are left, so the work is resumable rather than repeated.

Measured with:

```sh
python3 tools/corpus_match_probe.py --function _080013108 --c89 --work-dir build/lab/ff78
```

## Status

| | |
| --- | --- |
| ROM body | 728 bytes, 20 calls, 5 unrolled DMA steps |
| C transcription | **708 / 728 bytes**, same length, 16 differing offsets |
| Residual group A | the r8/r9 assignment of two long-lived pointers (4 offsets) |
| Residual group B | the DMA control word's constant `1` is a pool load in the ROM and an immediate in the candidate (12 offsets, mostly the pool layout) |

Everything else matches: the prologue with its high-register save, the
work-area gate, all twenty calls, the switch, the three record groups, and
all five DMA steps.

## The transcription

Replace the `__attribute__((naked))` body of `MenuFF78_13108` with the
following. The declarations and the DMA macro are the load-bearing part; the
rest is a straight transcription of the pseudo-code already in the file's
header comment.

```c
#define FF78_DMA_STEP(K, DEST)                                             \
    do {                                                                   \
        volatile u8 *row_;                                                 \
        s32 byte_;                                                        \
        __asm__ volatile ("" : : : "memory");                              \
        idx_ = (s32)*(s16 *)(uintptr_t)r6v;                                \
        row_ = (volatile u8 *)(uintptr_t)                                  \
            ((u32)(idx_ * 5) * 4 + r5v);                                 \
        byte_ = (s32)((volatile s8 *)row_)[K];                              \
        __asm__ volatile ("" : "+r"(byte_));                              \
        dma[0] = 0x080CD7A8u + (u32)(byte_ * 2);                           \
        dma[1] = (DEST);                                                   \
        dma[2] = 1u;                                                       \
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
    r9v = (volatile u32 *)(uintptr_t)(rec + 600);
    *r9v = 5;
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
```

## The recipes, and what each one is worth

### A pointer offset is two instructions, not one

The ROM forms every `rec + N` as

```
adds r1, r7, #0
adds r1, #138
```

and agbcc produces that **only** when the address stays a pointer. Writing
`*(volatile s16 *)(uintptr_t)(rec + 138)` turns the sum into an integer sum
and the compiler folds it to `movs r2,#138 / adds r2, r2, r7`. Measured in a
scratch TU: `*(volatile u8 *)(r + 138) = 1;` gives the ROM's two-instruction
form, and the same expression with a `(uintptr_t)` cast does not.

This is the "address association" rule in
[register-shaping levers](register_shaping_levers.md) seen from the other
side: the *pointer* keeps the offset as a separate add.

### A `switch`, not an if/else chain

The ROM's `cmp r1,#0 / beq case0 / cmp r1,#1 / beq case1 / b default` with
both case bodies out of line is what a `switch` lowers to. The `if (v == 0)
{...} else if (v == 1) {...}` chain lays the first body inline and branches
over it, which is two bytes and a branch out of place.

### The last two call arguments are literals

The ROM materialises the fifth argument in r4 and the sixth in r6 *between the
two stack stores*:

```
movs r4, #4
str  r4, [sp, #0]
movs r6, #1
str  r6, [sp, #4]
```

That is the signature of two literal arguments, each materialised at its own
store. Passing variables instead moves the `movs` before the stores.

### The work-area address is two pool words

`ldr r0,=0x03001780 / ldr r1,=0x0FBC / adds r4, r0, r1` needs the
`RAM_DEF_03001780` / `RAM_SYM_03001780` recipe and a *variable* offset; two
integer literals fold into one pool word.

### A call's result is stored straight from r0

`bl 0572C` followed by `str r0, [r5, #0]` means the result is stored without
a copy. Writing `val = Sub_0800572C(16); *dst = val;` adds a copy. The
destination address has to be a sub-expression of the store, not a prior
assignment, or the compiler computes the address *before* the call.

### The DMA loop is five unrolled steps

The ROM's tail is five copies of one step, and agbcc does not unroll. The
step's address arithmetic is `(v*4 + v) * 4`, not `v*20`:
`lsls r0,r1,#2 / adds r0,r0,r1 / lsls r0,r0,#2` is `(v*5)*4`; `v*20` would be
`lsls r0,r1,#4 / adds r0,r0,r1`.

### A signed read needs the destination pinned

The ROM's `ldrsh r1, [r6, r3]` puts the loaded value in r1. An unpinned
destination lands in a fresh temp and is copied home, which shifts the whole
tail. `register s32 idx_ __asm__("r1");` restores the ROM's register, and
with it the index scratch, the DMA base and the row table all fall into
place.

### The five re-reads need a memory clobber

The ROM re-reads `s16[rec+174]` in every step. A non-volatile read is
common-subexpression-eliminated across the five steps, and a volatile read
changes the instruction shape (`ldrh` plus a sign extension instead of the
ROM's `movs r3,#0 / ldrsh r1,[r6,r3]`). An empty `asm` with a memory clobber
before the read keeps the ROM's shape and stops the elimination.

### The sign extension must stay separate from the doubling

`ldrb r0,[r0,#1] / lsls r0,r0,#24 / asrs r0,r0,#24 / lsls r0,r0,#1` is a
sign extension followed by a shift. `byte_ * 2` folds the shift into the
extension (`asrs r0,r0,#23`); an empty `"+r"(byte_)` barrier between the read
and the multiply keeps them apart.

## The two residual differences

### A: r8 and r9 carry the two long-lived pointers the other way round

The ROM holds `rec+138`/`rec+612` in r8 and `rec+600` in r9. The candidate
holds `rec+600` in r8 and `rec+138`/`rec+612` in r9. Both are global
allocnos that live across the whole body, and `global.c`'s `allocno_compare`
(`floor_log2(refs) * refs / live_length * size`) should prefer the r8 one --
4-5 refs over a 418-insn range against 2 refs over a 138-insn range -- but
agbcc gives it the other register.

Tried and measured: pinning either pointer to r8 or r9 (each adds a copy and
drops the score to 426-650); splitting `rec+138` and `rec+612` into two
variables (four high registers instead of three, so the prologue changes);
declaring the pointers in the other order; moving the `rec+600` assignment
earlier. None lands.

### B: the DMA control word's constant 1 comes from the literal pool

The ROM writes the control word from r3, which it loaded from the pool:

```
ldr r3, [pc, #180] @ (0x2c4)      @ the pool word is 0x00000001
str r3, [r2, #8]
```

agbcc rematerialises the same constant as `movs r3, #1`. The pool word is
also what shifts the rest of the pool by four bytes, so group B accounts for
12 of the 16 differing offsets.

Tried and measured: the literal `1u`; a `const u32` local; a `static const`;
an enum; a pinned `register u32 one __asm__("r3") = 1u;` (which moves the
materialisation to the top of the function and re-orders the tail); and a
pinned variable assigned inside the first step (which hoists the assignment
above the byte load). agbcc always rematerialises a small constant with an
immediate, and the pool is only used for values that do not fit one.

The same shape appears in the ROM's pool at 0x2c0-0x2d4, where the four
remaining DMA destination words are `0x05000282` .. `0x0500028A` -- those do
not fit an immediate and both compilers pool them, so only the `1` differs.
