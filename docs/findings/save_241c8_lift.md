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
