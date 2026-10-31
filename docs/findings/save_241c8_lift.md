# SaveUpdate_080241C8 lift attempt: 340/368 with the WA_SPLIT pool recipe

`SaveUpdate_080241C8` (`0x080241C8`, 368 B, 174 instructions, 5 callees) is the
last assembly body in `src/save.c`. It is a 2x4 grid updater. This note records
the measured drafts, the semantics as the ROM actually computes them, and the
one wall left. The body is still assembly; nothing here was promoted.

## What the body does (corrected)

```
for (k = 0; k <= 1; k++)
  for (c = 0; c <= 3; c++) {
    v = _08025D64(k, c);
    idx = k*8 + c*2;                      // BYTE offset, not a halfword index
    *(u16 *)(WA + 0xFFA + idx) = v;       // grid 1
    if ((s16)v == 11) {
      *(u16 *)(WA + 0x101A + idx) = 1;    // grid 2
      *(u16 *)(WA + 0xFF6) = c + 1;       // count
      if (k == 0) {
        if (*(s16 *)(WA + 0xFF0) == 0) { if (c+1 > 3) *(u16 *)(WA+0xFF6) = 3; }
        else                           { if (c+1 > 3) { WA[0xFF6] = 0; WA[0xFF2] = 1; } }
      } else {
        if (*(u16 *)(WA + 0xFF0) == 1 && c+1 > 3) *(u16 *)(WA+0xFF6) = 3;
        *(u16 *)(WA + 0xFF0) = 1;
        *(u16 *)(WA + 0xFF2) = 1;
      }
    } else {
      *(u16 *)(WA + 0x101A + idx) = 0;
    }
    if (*(s16 *)(WA + 0xFFA + idx) > 2) *(u16 *)(WA + 0x102A + idx) = 1;
    else                                *(u16 *)(WA + 0x102A + idx) = 0;
    _08002714(k, c, _0800A9A0(k, c));
    _080026DC(k, c, _0800A9E0(k, c));
  }
```

**The index is a byte offset.** The ROM computes `c*2` (`lsls r2,r5,#1`),
`k*8` (`lsls r2,r7,#3`) and adds them; the grid rows are 8 bytes (4 cells).
Writing it as a u16 index (`k*4 + c`) compiled to the same address but a
shorter body, and writing `(k*8 + c*2)*2` is simply wrong.

## The pool recipe (`WA_SPLIT`)

The ROM keeps the base and each displacement in separate literal-pool words
(`ldr r2,=0x03001780; ... ldr r6,=0x00000FFA; add r6,r8`). A plain
`(uintptr_t)(0x03001780 + off)` folds into one constant and collapses to a
single `ldr`, which is most of the length shortfall. Naming the base and each
offset as *absolute symbols* restores the split:

```c
__asm__(".globl SaveWABase\nSaveWABase = 0x03001780\n"
        ".globl SaveWAOffFFA\nSaveWAOffFFA = 0x00000FFA\n" ...);
```

with `extern u8 SaveWABase;` etc. at file scope. This is the `WA_SPLIT`
recipe from `src/race_scene_b1.c`, already documented for the same WA fields
in `src/race_progress.c`. Two details measured there and confirmed here:

- `0xFF0` has **no** pool word in the ROM: it is synthesised as
  `movs r1,#255; lsls r1,#4`. Spelling it with a symbol adds a pool word and
  two instructions, so it stays a plain constant.
- Each `(wa + off)` must be materialised **before** `+ idx`; as one expression
  agbcc re-associates and shares the `wa + idx` sum across sites.

## Measured drafts

Drafts under ignored `build/lift/save241c8/`. The in-source numbers use the
same TU and gates as the acceptance probe.

| draft | shape | candidate | aligned |
| --- | --- | ---: | ---: |
| v59 | sibling-style `o; pa = o; pa += wa;` + `sl` state pointer | 352 B | 76/174 |
| v41 / v47 | absolute-symbol pools, byte index, `p = wa + off` per site | 340 B | 107/174 |
| v48 | two base symbols, branch re-anchor `p = WA` | 344 B | 88 |
| v44 | same + a single `idx` reused | 324 B | 86 |
| v40 | byte index, symbol pools, inline `c2 + k8` | 340 B | 66 |
| v28 | halfword index (2x address, wrong) | 364 B | 72 |
| v24 | `struct SaveWork24` fields | 320 B | 28 |

The tail is byte-exact (all four calls, the `r2` return copies, the loop
increments and tests, and the epilogue), and the setup is byte-exact through
`sl = WA + 0xFF0` in v41.

## The remaining wall (28 bytes)

Every grid store in the ROM is `ldr rX,=off; add rX,r8; adds rX,idx,rX`
(two adds, base in `r8`). The draft instead copies the base to a scratch
(`mov r4,r8; adds r1,r4,r0; adds r0,r1,r3`) or recomputes `base + c2 + k8`
whenever the index is not kept live, and it never reproduces the three
`ldr r2,=0xFFFFF006; adds r6,r6,r2` pointer resets that re-anchor the grid
pointer on `WA`. Pinning the roles (`idx`→r3, base→r6, `k8`→r9, `c2`→r2) makes
the body shorter (300-324 B), not longer, so the ROM's extra instructions come
from a source shape that keeps an anchor pointer *and* rebuilds the index from
a spilled `c2`, not from pinning.

`--sweep`/`--spill` over `k, c, k8, c2, idx, v, p, wa` plateau at 340/107.

## Barrier-sweep progress (this session)

Best exact-length barrier sets found by `--sweep` (in-place, 368 B):

- `v117`: `+r(c2)` after the grid-1 store, elsewhere — **118/174 aligned**;
  `+p71/c` (`+r(c)` after the `k!=0` flag store) is the probe that pays.
- Compounding `p60/c + p71/c` etc. reaches 376 B / 121 aligned — **loses the exact
  length**.
- `r3p54/c` (+r(c) after the `count` store) reaches **368 B / 121 aligned**.
- `body_diff` against the v117 ROM window: the setup through `str r0,[sp,#0]`
  is **byte-identical**; the alignment metric overstates the remaining work.

The remaining wall is unchanged: the ROM spills `c2` **after** the grid store
(`str r2,[sp,#4]` at 0x08024206), where the drafts spill at its definition; and
the `tst r3,#1`-style constant copy emits an extra `mov ip,r6` the ROM lacks.
`body_diff` confirms 138/174 instructions currently differ over the full
0x80241c8..0x8024338 span included pool disassembly confusion.

## Measured lever: `pa = o; pa += wa;`

`src/race_progress.c` (`Race_AdvanceProgress`, EXACT) already solves this field set
and shows the expression shape: name each pooled displacement with its own
absolute symbol, keep the base in a register, and build the address as
`offset first, then base`. Written as `pa = wa + o` agbcc emits
`mov rX,r8; adds rX,rX,rO` (a base copy plus a 3-operand add); written as

```c
o = (u32)&SaveWAOffFFA;
pa = o;
pa += wa;
```

it emits the ROM's `ldr r1,=0x00000FFA; add r1,r8` — the 2-operand `add`.
With that, the per-site shape matches the ROM instruction for instruction
(`ldr offset; add base; adds idx,base; strh`), and the body reaches 352/368.

`src/race_progress.c` also documents the complementary levers: a displacement
without its own pool word must be derived from one that has it
(`o4 = o4 - 16u; pa = w2 + o4;`), and a base that the ROM materialises twice
needs two distinct symbols (`PftWa`/`PftWb`/`PftWc`) or agbcc CSEs the base
into one long-lived register.

## Next move

Write the branch bodies as `p = (u8 *)WA;` and the grid stores as
`p + (0xFFA + c2 + k8)`, with `c2` forced to the frame, so the compiler has to
re-anchor `p` and rebuild the index at `0x080242B4` the way the ROM does. The
three `-0xFFA` pool words and the `adds r6,r6,r2` resets are the visible
signature of that shape.

## Progress: exact length reached (368 B)

The strongest drafts now reproduce the ROM's **exact 368-byte length** and 96 of
174 instructions:

- `build/lift/save241c8/v93.c` / `v97.c` — the base pointer `g` re-anchors per
  arm (`g = g + &NegFFA` in each `k==0` sub-path, `g = _flag + &NegFF2` in the
  `k!=0` arm), which restores all three `-0xFFA` pool words and the `-0xFF2`
  word. Pool-word count is 13 vs the ROM's 14.
- `build/lift/save241c8/v47.c` — best instruction alignment (107/174) at 340 B,
  from the barrier sweep.
- The `-0xFFA` re-anchor only survives when the offset is an **absolute symbol**
  (`extern u8 SaveWAOffNegFFA;` with `.set ... = 0xFFFFF006`); as a plain
  `0xFFFFF006u` literal agbcc constant-folds it away and the body shrinks by 12 B.

### The remaining wall: `k` lives on the stack

The ROM keeps the outer counter in `r7` (`movs r7,#0`, `cmp r7,#1`, and
`adds r0,r7,#0` at each call). Every draft spills it to `sp+0`
(`movs r0,#0; str r0,[sp,#0]`, then `ldr r0,[sp,#0]` per use), because the other
seven callee-saved registers are occupied by `wa` (r8), the state pointer (sl),
`k8` (r9), `c` (r5), the grid base (r6) and the `c+1` scratch (r4).

What was measured:

- Register pins on `k` (`__asm__("r7")`) are overridden by the loop optimizer.
- A `+r` barrier on `k` *does* put it in a register, but it also hides `k = 0`
  from the loop test, so agbcc emits a pre-test (`cmp r7,#1; ble.n; b exit`)
  that the ROM does not have. With a `do { } while` loop the pre-test is gone
  but the ROM's `adds r0,r7,#1; str r0,[sp,#0]` increment hoist disappears too.
- Declaration order, `register` without a pin, `k = 0` before/after the WA
  setup, `k < 2`, `k != 2`, and `--sweep` over every variable all leave `k` on
  the stack.
- The `k8` shift scratch was pinned to `r2` (the ROM's) with a `+r` barrier; it
  does not free `r7` for `k`.

The permuter cannot help on this draft: `--prepare-only` reports
`unresolved candidate relocation` for `SaveWAOffNegFFA`/`SaveWAOffNegFF2`,
because the absolute `.set` symbols the pool recipe depends on are not visible
to its relocation resolver. A symbol-free draft (the 340 B `v41`) permutes but
plateaus at an advisory score of 5575.

## Latest state: exact length, exact setup, 199 matched bytes

`build/lift/save241c8/v108.c` (and `v109`) now reproduce the ROM's setup
**instruction for instruction**:

```
movs r7,#0                  k = 0 in r7
ldr r0,=WA / mov r8,r0      wa
movs r1,#255 / lsls #4 / add r1,r8 / mov sl,r1   state pointer
movs r5,#0                  c = 0
lsls r2,r7,#3 / mov r9,r2   k8 (r2 scratch, r9 home)
adds r0,r7,#1 / str r0,[sp,#0]    k+1 spill
adds r0,r7,#0 / adds r1,r5,#0 / bl
```

It is 368 B (exact), has the ROM's 14 literal-pool words, and matches 199
bytes / 114 of 174 instructions. The levers that got there:

- `knext = k + 1; __asm__ volatile ("" : "+m"(knext));` — the explicit
  increment temporary with a memory barrier is what reproduces the ROM's
  `adds r0,r7,#1; str r0,[sp,#0]` hoist; a plain `for (...; k++)` does not.
- `k8` computed through a pinned `r2` scratch (`kt`) with a `+r` barrier, then
  moved to `r9`.
- The `k+1` spill must be declared/assigned before `c = 0` in the loop body,
  otherwise the compiler reorders `movs r5,#0` above the `lsls`.
- `SaveWAOffNegFFA2` (a second symbol with the same value) is required so the
  two `k==0` re-anchors do not merge into one pool word.

### What is left

The only structural difference left is the `c2` spill point. The ROM keeps
`c2 = c<<1` in `r2` through the grid store and spills it afterwards
(`str r2,[sp,#4]` at `0x08024206`); every draft spills at the definition
(`str r1,[sp,#4]` right after `lsls r1,r5,#1`) because the `+m` barrier that
creates the slot is satisfied by homing the variable from the start. Removing
the barrier keeps `c2` in a register, drops the frame to 4 B and the body to
356 B. A `+r` barrier at the definition plus `+m` later emits the store twice.

The remaining register differences (`r1` vs `r2` for `c2`, `r0` vs `r3` in a
few scratch slots) follow from that one spill decision.

## Near-complete draft: `v131` (368 B, exact structure, 170/368 bytes)

`build/lift/save241c8/v131.c` is the closest draft so far:

- **368 bytes, the ROM's exact length**, 14 literal-pool words in the ROM's
  five pools with the same word order.
- The instruction sequence matches block for block: same ops, same count.
- `corpus_match_probe --c89` reports `PARTIAL 170/368 bytes, candidate 368`,
  and the `--diff` listing shows the残り differences are **register choices
  only** (`r0`/`r1`/`r2`/`r3` picks) plus pool-placement shifts that follow
  from them — no missing or extra operations.

Levers that built v131 (on top of the earlier ones):

- `c2st = (u32)c2;` **after** the grid store plus
  `__asm__ volatile("" : "+m"(c2st));` produces the ROM's late
  `str r2,[sp,#4]` and the `ldr r2,[sp,#4]` re-read.
- `knext = k + 1;` with a `+m` barrier produces the `adds r0,r7,#1;
  str r0,[sp,#0]` increment hoist.
- The three `-0xFFA` re-anchors must be **defined** in the function's own
  `__asm__(".globl ...")` block (`SaveWAOffNegFFA`, `SaveWAOffNegFFA2`,
  `SaveWAOffNegFF2`). Without the `.globl` definitions the assembler emits
  `R_ARM_ABS32` relocations that the probe cannot resolve
  (`pool +0x8C: SaveWAOffNegFFA -> 0x00000000`); with them the words are raw
  bytes and the probe resolves the body.

### Remaining

Register allocation only. The ROM has `c` in `r5`, `k` in `r7`, `idx` in `r3`,
`c2` in `r2`, the grid base in `r6`, `WA` in `r8`, `k8` in `r9`, the state
pointer in `sl`; the draft rotates several of those. Pins on `k`/`c` are
overridden by the loop pass, and `--sweep` barriers trade length for alignment.
decomp-permuter now ingests the draft (no unresolved relocations) and is
grinding on the register choices from an advisory score of ~3900.
