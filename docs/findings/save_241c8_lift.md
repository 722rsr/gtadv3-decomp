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

## Real-gate evidence: 4 bytes from exact

`make matching-slice` (the actual acceptance gate) now reports:

```
match_c_slice: C-owned code slice differs at ROM offset 0x0241D6: got 1b, expected 1a
```

`0x0241D6` is function offset `0xE`, the first literal-pool load. Everything
before it — prologue, frame, `movs r7,#0`, the WA/state setup — is byte-exact.
The cause is a single spurious `movs r7,#1` that every draft emits just before
`movs r5,#0` (the ROM's `c = 0`), which pushes the pool four bytes late.

Two exact-length drafts reach this point:

- `v131`/`v146` (while + `knext`): frame 8, `c2` at `sp+4`, `k+1` hoist
  `adds r0,r7,#1; str r0,[sp,#0]` — plus the spurious `movs r7,#1`.
- `v163` (struct `{ k; c2; }` frame slots, `k` initialized in its declaration):
  same, and it also fixes the `movs r7,#0` position.

Tried and measured against the gate, all landing at `0x241D2`–`0x241D6`:
`+m` barriers on `knext`/`c2st`, `volatile` locals, a `u32 kst[1]` array, a
struct with a `volatile` field, `for`/`while`/`while(1)+break` loop forms,
`k++` vs a hoisted `knext`, and removing the `+r(kt)` barrier. The `movs r7,#1`
appears in every variant that also produces the ROM's `k+1` spill; the variants
without it lose the `sp+0` slot and fail at `0x241D2` (frame 4 instead of 8).

Note: the `+m` barrier does **not** allocate a stack slot — a draft using it
alone writes `sp+4` inside a 4-byte frame. The struct form is the legitimate
slot allocator.

Next lead: find the source shape that yields `adds r0,r7,#1; str r0,[sp,#0]`
without the `movs r7,#1`. The `movs` is the compiler materialising `k` at the
loop head from the spilled `k+1`; it appears whenever the increment value is
kept in `k`'s register. A shape where the spill is read into a *different*
register (the ROM reads it back into `r7` only at the loop *bottom*) should
remove it.

## Best C draft: first difference at +0x3E (real gate)

Progress this session, all measured on `make matching-slice` (the acceptance
gate) and reproduced by `python3 tools/corpus_match_probe.py --function
SaveUpdate_080241C8 --c89`:

| draft | gate first diff | candidate | note |
| --- | --- | --- | --- |
| earlier best | `0x0241D6` (+0x0E) | 368 B | pool 4 B late, spurious `movs r7,#1` |
| `v172` | `0x0241F4` (+0x2C) | 368 B | `kf.c2` spill added; `c2` still in `r7` |
| **`v173`** | **`0x024206` (+0x3E)** | **368 B** | **`c2` pinned to `r2`** |

`v173` (`build/lift/save241c8/v173.c`) is the best draft: prologue, frame,
`movs r7,#0`, the WA/state setup, the `k<<3` / `k+1` hoist, the `_08025D64`
call, `lsls r2,r5,#1` (`c2` in `r2`), `adds r3,r2,r1` (`idx` in `r3`), the
grid-1 store and the `lsls/asrs` narrowing are all byte-identical. The first
difference is at function offset `0x3E`, where the ROM has

```
3e: 9201   str r2,[sp,#4]      ; spill c2
40: 280b   cmp r0,#11
```

and the draft has `cmp` directly — the `c2` spill is missing. The ROM reloads
it at `0xEC` (`ldr r2,[sp,#4]; add r2,r9`), so the value is live across the
whole `if/else`.

### Why the spill is missing, and what was measured

`agbcc` spills `c2` in the ROM because `r2` is *also* used inside the
`if/else` (`mov r2,sl` at 0x5C, `ldr r2,[pc]` at 0x6A/0x90, `mov r2,sl` at
0xC2). Pinning `c2` to `r2` (`register s32 c2 __asm__("r2")`) is what makes
the *first* half match — but it also removes `r2` from the allocator's scratch
set, so nothing clobbers it and no spill is needed.

Every C spelling tried to force the spill, with measured results:

| spelling | first diff | len |
| --- | --- | --- |
| flush `c2` into `kf.c2` right after the grid store, read back for the grid-3 block | +0x0E | 364 |
| same, but keep `c2` in `r2` **and** write `kf.pad` | +0x0E | 364 |
| separate `u32 c2st` scalar instead of a struct field | +0x0E | 372 (oversized) |
| two single-member structs (`{k}` + `{c2}`) | compile error | - |
| address-taken alias through `&kf + 4` | compile error | - |
| `volatile u32 c2m[1]` | +0x0E | 296 |
| `u32 c2m[1]` | +0x0A | 372 |
| named `s16 vs = (s16)v` before the compare (no spill) | +0x3E | 368 |
| plain `knext` / `c2sp` scalars (no struct) | +0x2C | 364 |
| `idx` pinned to `r3`, `c2` free | +0x3E | 364 |

The struct form emits the right `str r2,[sp,#4]` but agbcc **rotates the outer
loop** — it puts `mov r7,#1` before `.L265` and branches into the rotated
head, which both costs 2 bytes at `0x1A` and drops 4 bytes overall (364 B).
That rotation is the current blocker, not the store itself.

### Tooling left in place

- `build/lift/save241c8/score.py PATH.c [...]` — splice a body into
  `src/save.c`, run the acceptance probe, print
  `first=+0xNN len=NNN matched=NN status`. It restores `src/save.c` from git
  before every splice, so it is safe to run in a sweep.
- `build/lift/save241c8/gen*/` — the swept variants (loop form × k-init ×
  k-spill × c2-spill × count-store × constant × register pins).
- `build/lift/save241c8/v173.c` — the current best body.

Next lead: stop the loop rotation that the two-slot struct triggers. The ROM
keeps `k+1` in `sp+0` and reloads it only at `0x152`; agbcc instead rotates
the loop when both halves of `kf` are live. A shape where the `k+1` store and
its reload are the *same* memory object but the `c2` slot is reached through a
different lvalue may keep the spill without enabling rotation.

## Second pass: r2 pin moves the wall from 0x2C to 0x3E

The compile-order recipe (`__asm__(".set NAME, 0x...")` at file scope for every
WA offset, then the C body) is what makes the acceptance probe agree with the
real gate. With that in place the search became measurable per variant.

Pinning `c2 = c << 1` to r2 is the single highest-value lever found so far:

```c
register s32 kt __asm__("r2");
register s32 c2 __asm__("r2");
...
c2 = c << 1;
idx = c2 + k8;
```

`make matching-slice` then reports

```
match_c_slice: C-owned code slice differs at ROM offset 0x024206: got 0b, expected 01
```

`0x024206` is function offset `0x3E`. Bytes 0x00-0x3D are now exact: prologue,
8-byte frame, `movs r7,#0`, the WA load, the `0xFF0` synthesis, `mov sl,r1`,
`movs r5,#0`, `lsls r2,r7,#3`, `mov r9,r2`, `adds r0,r7,#1`, `str r0,[sp]`,
`bl`, `lsls r2,r5,#1`, `mov r1,r9`, `adds r3,r2,r1`, the grid-1 base load,
`strh r0,[r1]`, and the `lsls`/`asrs` narrowing. Probe: `PARTIAL 260/368,
first difference +0x3E`.

What is missing at 0x3E is the ROM's `str r2,[sp,#4]` - the spill of `c2`. The
ROM stores it there and reloads it at `0xEC` (`ldr r2,[sp,#4]; add r2,r9`),
because its if/else arms use r2 as scratch. agbcc with `c2` pinned to r2 has r2
free, so it never spills.

### Attempts measured against the gate/probe after the r2 pin

| variant | idea | result |
| --- | --- | --- |
| `kf.c2 = (u32)c2` in `struct { s32 k; u32 c2; }` | force the spill | spill appears, but at `0x3C`, plus a spurious `movs r7,#1` at the loop head; 364 B |
| `kf.pad = ...` in `struct { s32 k; u32 pad; }` | same, other field name | identical to the above |
| two single-member structs (`{k}` + `{c2}`) | avoid the rotated loop | does not compile cleanly through the C89 transform |
| byte-offset alias `*(u32 *)((u8 *)&kf + 4)` | spill without a second member | probe preprocess failure |
| plain `s32 knext; u32 c2sp;` | two scalars | c2 lands in r7 again; `+0x2C`, 364 B |
| `volatile u32 c2m[1]` | volatile slot | 296 B, `+0x0E` |
| `u32 c2m[1]` | array slot | 372 B (oversized) |
| `register s32 idx __asm__("r3")` | force idx to r3 | 364 B, `+0x0E`/`+0x3E` depending on the rest |
| asm splice `str %0,[sp,#4]` / `ldr %0,[sp,#4]` | exact by construction | breaks the C89 transform at `src/save.c:232` |

The `movs r7,#1` that appears whenever the struct is fully used is agbcc's loop
rotation: it duplicates the loop head and materialises `k = 1` on the second
entry path. That is why the struct form is 4 bytes short even though it has the
right spill.

### Harness left behind

* `build/lift/save241c8/score.py` - splices a body (with the file-scope `.set`
  block) into `src/save.c`, restores from git first, runs the acceptance probe,
  and prints `first=+0xNN len=N matched=M status`.
* `build/lift/save241c8/gen*/` - the measured variant families.
* `build/lift/save241c8/v173.c` - best known body (260/368, first difference
  `+0x3E`).
* `build/lift/save241c8/ledger.jsonl` - 333 variants from the first sweep.

Always restore with `git checkout -- src/save.c`; `make matching-slice` is green
on the committed naked body.

## Third pass: 305/368, correct program, register allocation only

The decisive change was to stop pinning `k`.  A pin on `k` is not honoured after
agbcc's loop pass: the generated code materialised the flag constant into `r7`
and then compared it (`movs r7,#1; ...; cmp r7,#0`), which makes the `k == 0`
arm unreachable.  The draft was the right length but a different program.

With `k` left to the allocator, the body stays correct and the following shape
reaches **368 bytes, 174 instructions (same as the ROM), 305/368 bytes matched,
first difference `+0x20`**:

* `struct { s32 k; u32 c2; } kf;` - forces the 8-byte frame and the two ROM
  slots (`k+1` at `sp+0`, `c2` at `sp+4`).  With `k` unpinned agbcc emits the
  ROM's `str r2,[sp,#4]` / `ldr [sp,#4]` spill and reload by itself.
* `s32 cnt;` plus `if ((s16)cnt > 3)` - this is what produces the ROM's
  `lsls rN,#16; asrs rN,#16` narrowing before each `cmp #3`.  A 16-bit `cnt`
  type instead adds zero-extensions at every store and is 4 bytes too long.
* `u16 *pcnt` hoisted out of the branch - the ROM computes `wa+0xFF6` once
  (`ldr r1,=0xFF6; add r1,r8`) and reuses `r1` for the later `= 3` / `= 0`
  stores.  Recomputing the pointer per store costs two instructions.
* the `g += -0xFFA` adjustment must be emitted **before** the `cnt > 3`
  comparison (the ROM does `adds r6,r6,r2` then `cmp r0,#3`).  Moving it after
  the branch loses 18 bytes of alignment (`287` -> `305`).

`docs/findings/save241c8/best_305.c` is that body (`v371` in the work tree).

### What is left

All remaining differences are register choices, not structure:

| ROM | draft | site |
| --- | --- | --- |
| `adds r0,r7,#1` | `adds r3,r7,#1` | `k+1` hoist |
| `str r2,[sp,#4]` at `0x3E` | at `0x3A` | `c2` spill scheduled two instructions early |
| `movs r3,#1` | `movs r2,#1` | grid-2 flag |
| `adds r0,r5,#1` + `adds r4,r0,#0` | `adds r4,r5,#1` | counter |
| `mov r2,sl` / `ldrsh r2,[r2,r0]` | `mov r0,sl` / `ldrsh r0,[r0,r3]` | state read |
| `ldr r2,[sp,#4]; add r2,r9` | `ldr r3,[sp,#4]; add r3,r9` | grid-3 index |
| `movs r2,#0` | `movs r1,#0` | grid-2 clear |
| `movs r1,#1` / `movs r2,#0` | `movs r3,#1` / `movs r0,#0` | grid-3 store |

Measured attempts that did **not** help: pinning `idx` to `r2`, `cnt` to `r4`,
`c2` to `r2`, the state read into an `r2`/`r0`/plain temp, computing `pcnt`
through `o`, reordering the counter store, and an explicit `kf.c2` spill.

The permuter runs on `best_305.c` with an advisory baseline of **1785**
(previously 5300 on the earlier drafts).

### Final measured state

`docs/findings/save241c8/best_305.c` is the best correct draft:

```
_080241C8 0x080241c8 src/save.c: PARTIAL 305/368 bytes, candidate 368 bytes, first difference +0x20
rom 174 instructions / 368 B, candidate 174 instructions / 368 B, 134 instructions identical
```

A sweep of 72 register-pin combinations (`c2`, `idx`, `c`, `cnt` pinned to
`r2`/`r3`/`r4`/`r5`, flag constant as a pinned variable) found **no combination
that moves the first difference past `+0x20`**; every one either stays at
`+0x20`/368 B or drops to 364 B.  The `+0x20` difference is
`adds r0,r7,#1` (ROM) versus `adds r3,r7,#1` (draft) - a pure scratch-register
choice for the `k+1` hoist, and pinning that scratch (`register s32 kn
__asm__("r0")`) is ignored by agbcc.

The permuter, seeded from `best_305.c`, has an advisory baseline of **1785**
(down from 5495 on the earlier wrong-length drafts) and its best mutation so far
scores **1555**, from cosmetic rewrites (`0x0FF0u ^ 0`, an extra `u8 *`
temporary), not a structural breakthrough.

What remains is 40 instructions of register-choice difference across:

* the `k+1` scratch (`r0` vs `r3`),
* the `c2` spill being scheduled two instructions before the sign-extension,
* the indexed-halfword scratch for `*(s16 *)wastate` (ROM `r2`/`r0`, draft `r0`/`r3`),
* which low register holds the grid-2 flag, the grid-2 clear, and the grid-3 constants,
* the reloaded `c2` for the grid-3 index (ROM `r2`, draft `r3`).

These are exactly the cases `docs/findings/indexed_halfword_scratch.md` and
`register_shaping_levers.md` call allocator-dependent; the repo's own note says
the choice "tracks the register pressure of the original source, which is not
recoverable from the C spelling" for the halfword scratch.

## Fourth pass: 307/368 via a liveness pin

`docs/findings/save241c8/best_307.c` is the new best:

```
PARTIAL 307/368 bytes, candidate 368 bytes, first difference +0x3A
```

The `+0x20` wall (`adds r0,r7,#1` in the ROM, `adds r3,r7,#1` in the draft) is
closed with the liveness-pin recipe from
`docs/findings/indexed_halfword_scratch.md`: pin a dummy to the *unwanted*
scratch register across the statement that allocates it.

```c
register s32 hold __asm__("r3");
...
__asm__ volatile ("" : "=r"(hold));
kf.k = (s32)(k + 1);          /* now allocates r0, as the ROM does */
__asm__ volatile ("" : : "r"(hold));
```

Measured: no pin -> `+0x20`/305; pin `r3` -> `+0x3A`/307; pins on `r0`, `r1`
-> no change.

### The new wall

`0x3A` is the `c2` spill.  The ROM emits

```
3a: lsls r0,r0,#16
3c: asrs r0,r0,#16
3e: str r2,[sp,#4]
```

while the draft emits `str r2,[sp,#4]` at `0x3A` - the spill is scheduled two
instructions before the sign-extension.  A greedy sweep over 21 statement
anchors x 5 registers found **no single liveness pin that improves on 307**, and
naming the sign extension (`s16 vs = (s16)v;`) costs a stack slot and drops the
body to 364 B.

### Search tooling

* `build/lift/save241c8/drive.py` - bounded-round mutation search, persists the
  best draft, verifies with the probe and runs `make matching-slice` on a
  scratch-grader 368/368.
* `build/lift/save241c8/pins.py` - greedy liveness-pin search.
* Single-mutation sweeps over 12 mutator kinds (types, pins, declaration order,
  statement order, counter order, flag/state/grid-3 temps, loop form) all return
  exactly 305 on the previous best - the neighbourhood is flat, which is why the
  pin recipe was needed.

## Fifth pass: instrumented agbcc, and the exact nature of the last wall

I built an instrumented copy of the pinned compiler (patch in
`docs/findings/save241c8/allocator_dump.md`) and dumped `reg_renumber` for every
allocno.  The result is definitive:

**Every allocation the ROM makes is reproduced** - `k`->r7, `kf`->sp, `c`->r5,
`c2`->r2, `idx`->r3, `cnt`->r4, the grid offset constants->r0, the address
arithmetic->r0.

The only remaining differences are which *scratch* register carries a constant
that two dead registers could equally hold:

| site | ROM | draft |
| --- | --- | --- |
| grid-2 flag `1` (pseudo 47) | r3 | r2 |
| grid-3 constants (pseudo 76) | r1 | memory |

Both registers are dead at that point, so this is a tie-break in the sorted
allocation order, not a liveness or structure difference.  Pinning `r2` across
the store, or naming the constant and pinning it to `r3`, both displace `c2` and
change the frame (376 B / 364 B / 380 B).  72 pin combinations and a greedy pin
sweep over 21 anchors x 5 registers were measured; 307/368 stands.

This is the honest boundary: the remaining 61 bytes are an allocator tie-break
that the C spelling does not reach.

