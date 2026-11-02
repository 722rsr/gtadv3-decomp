# MenuFF78_133E4 lift attempt: the volatile-slot frame recipe

`MenuFF78_133E4` (`0x080133E4`, 1,132 B, 551 instructions, 15 calls to 8
callees) is the largest liftable body left in the ~105-function FF78 menu
family. This note records two sessions' attempts: the source shapes that
reproduce the prologue and the entry block byte for byte, the measured score,
and the remaining differences that keep it from an exact match. The body is
still assembly; nothing here was promoted. The failed shapes are measured
leads, not a verdict on the body.

## What the body does

`(rec, a1, ev)` big event machine. `ev==1` dispatches on `s16[rec+138]`
(0/1); `s16[138]==0` then dispatches on `s16[WA+0xFC2]` (94/95/96), each arm
calling `Sub_0802B368` and running the `WA + row*12 + 48/49` pair writes. Then
`ev==2` resets the panel fields, `a1` bits 32/16 step `u16[rec+136]`,
`u16[rec+140]` decays, `ev` 64/128 step `u16[rec+172]`, the cursor is clamped
against `u32[rec+188 + i*4]`, the key row is rebuilt through
`Sub_080022E4`, and when the row changed the tail re-runs `Sub_0800DA50`,
`Sub_08024C3C`/`Sub_08024C58` + `Sub_08007ABC` pairs, and five unrolled DMA3
palette copies from `0x080CCEEC`/`0x080CD7A8` into `0x05000282..8A`.
`Sub_080026A4C` closes the selection.

## The frame is a set of `volatile u32` locals

The ROM's entry stores are `str` (word), not `strh`, and they land at
`sp+0/4/8/12` in declaration order. A plain `u32` local does not do this:
agbcc keeps it in a register and the frame collapses. `volatile u32` does,
and the order is exactly the declaration order — measured with
`tools/expr_probe.py`:

```
volatile u32 x0 = (u16)a;   ->  lsls/lsrs/str r1,[sp,#0]
volatile u32 x1 = ...;      ->  str r1,[sp,#4]
volatile u32 x2 = ...;      ->  str r1,[sp,#8]
volatile u32 x3 = ...;      ->  str r0,[sp,#12]
```

The ROM's 28-byte frame is then seven such slots, in the ROM's order:

| slot | value |
| --- | --- |
| `sp+0` | `a1` (the key mask word) |
| `sp+4` | saved `u16[rec+174]` |
| `sp+8` | saved `u16[rec+172]` |
| `sp+12` | saved `u16[rec+136]` |
| `sp+16` | `rec+188` pointer |
| `sp+20` | `rec+172` pointer |
| `sp+24` | `(s16)` of the saved `u16[rec+172]` |

## Best draft (`v33`)

`build/lift/133e4/v33.c` (ignored). Shape:

```c
void MenuFF78_133E4(void *rec_, int a1_, int a2_) {
    register volatile u8 *rec __asm__("r6");
    volatile u32 a1;
    register u32 ev __asm__("r9");
    volatile u32 e174, e172, e136, iflag, p172v, saved172;
    u16 *p174;
    register u16 *p136 __asm__("sl");
    register volatile u8 *wa __asm__("r8");
    ...
```

with a short `p174 = rec+174` base (used at entry and in the `c==0` arm) and a
pinned `r7` base for the one-off `rec+172` load.

Measured in the TU: **1,196 B vs 1,132, 141 / 551 instructions aligned**, and
the prologue plus the whole entry block are byte-identical through the second
load:

```
push {r4,r5,r6,r7,lr} / mov r7,sl / mov r6,r9 / mov r5,r8 / push {r5,r6,r7}
sub sp,#28
adds r6,r0,#0
lsls r1,#16 / lsrs r1,#16 / str r1,[sp,#0]
lsls r2,#16 / lsrs r2,#16 / mov r9,r2
adds r4,r6,#0 / adds r4,#174 / ldrh r0,[r4] / str r0,[sp,#4]
```

First difference (byte 38): the ROM loads `ldrh r1,[r7]` directly; the draft
emits `adds r0,r7,#0 / ldrh r1,[r0]`. The pinned-base copy is the same
"unpinned-base" family as `docs/findings/garage_2a764_lift.md`.

## Measured drafts

| draft | shape | candidate | matched | first diff |
| --- | --- | ---: | ---: | ---: |
| v33 | volatile slots + pinned rec/ev/p136/r8/r7 + short p174 | 1,196 | 141 | byte 38 |
| v13 | `int` params, `u16` casts, rec/ev pins, field pointer locals | 1,132 | 195 | byte 26 |
| v9 | `u16` params, rec/ev pins | 1,132 | 185 | byte 12 |
| v20 | plain C, `uintptr_t` casts, unrolled DMA | 1,092 | 127 | byte 12 |
| asmlift seed | advisory | 1,220 | 96 | byte 10 |

`--sweep`/`--spill` over the register and frame variables plateau at 174-178
aligned instructions; no single barrier placement is close.

## Remaining differences

1. **Pinned-base copies.** A pinned base register wins the allocation but
   agbcc still copies it to `r0` for the load (`adds r0,r7,#0`), one
   instruction the ROM does not have.
2. **The `s` load.** ROM: `movs r2,#0; ldrsh r5,[r0,r2]`; draft:
   `movs r1,#0; ldrsh r5,[r0,r1]`. The zero index register differs, and the
   draft's `s` also has to be forced out of `sl`.
3. **Dispatch lowers differently.** `if (s == 0) ... else if (s == 1)` gives a
   `bne` tree; the ROM's `cmp 0/beq, cmp 1/beq, b` is the two-case `switch`
   order. A `switch` spelling restores the tree but drops `r7` from the
   prologue and grows the body by another 20 B (`v34`).
4. **+64 B overall.** The draft is 32 instructions long. The DMA macro and the
   rebuild call sequence account for most of it; the size must be closed before
   the tail can align.

## Tooling notes

- `RAM_DEF_03001780` must stay after every declaration in its block: the macro
  expands to a statement, so a `register ... __asm__` declaration written after
  it is a declaration-after-statement and agbcc rejects the TU.
- An undefined `FF78_DMA_STEP(...)` compiles silently as an implicit
  `int` function call, which scores as a 112-byte-short body. Keep the macro
  definition at file scope inside the TU.

## Next move

Two concrete leads, both cheap to test:

1. Make the one-off `rec+172` base unpinned and instead give `p172` (the
   long-lived tail pointer) the `r7`-then-`sp+20` life the ROM shows, so
   local-alloc chooses `r7` for the entry load instead of copying.
2. Split `s` out of the `sl` pin and give the `ldrsh` its zero index through a
   pinned `r2` temp, then re-run the `--sweep` on the two dispatch branches.
