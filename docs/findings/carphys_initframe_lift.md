# CarPhysRacer_InitFrame lift: the dispatch is an if-tree, not a switch

`_08021374` (`CarPhysRacer_InitFrame`, VMA `0x08021374`, `0x4B4` = 1204 bytes)
is selected from `asm/carphys_racer.s`. The C lift is **not** in `src/`: the
matching-slice gate splices any C body that defines a selected symbol, so an
inexact draft in `src/car_physics_core.c` makes `make matching-ready` fail at
`match_c_slice: C-owned code slice differs at ROM offset 0x02137E`. The drafts
live here instead, whole-TU copies that compile against the pinned agbcc:

| file | bytes matched | body size |
| --- | --- | --- |
| `best_368.c` | 368 / 1204 | 1208 |
| `best_365.c` | 365 / 1204 | 1208 |
| `switchform_227.c` | 227 / 1204 | 1204 |

`best_368.c` is `best_365.c` plus the `cell` pin (see below).
`python3 tools/corpus_match_probe.py --function _08021374` measures each after
copying it over `src/car_physics_core.c`.

## What moved the body from 137 to 368

### The dispatch is a hand-written if-tree, not a `switch`

The ROM's dispatch has the switch's default body **twice, inside the decision
tree**, before any `case` arm:

```
8021396: movs r2,#192 ; adds r2,r2,r7 ; mov sl,r2      leaf 1 (<=27 path)
802139c: b.n  0x80213e8
80213a6: movs r3,#192 ; adds r3,r3,r7 ; mov sl,r3      leaf 2 (>28 path)
80213ac: b.n  0x80213e8
```

Two copies with **different scratch registers** (`r2`, then `r3`) means two
distinct pseudos, i.e. two expansions of the statement, not one block copied by
the optimiser. `stmt.c`'s `expand_end_case` emits a single `default_label`, so
no `switch` spelling reaches this: `default:` first, `default:` last, or the
assignment moved after the switch all produced **one** copy (227, 271, 227
matched bytes), and adding `__asm__ volatile ("")` between the arms did not stop
the merge either.

An if-tree with `goto` arms does reproduce it:

```c
v = *(s16 *)sub_08004B68();
if (v == 28) goto arm28;
if (v < 29) {
    if (v == 27) goto arm27;
    p = (s16 *)(c + 0xC0); cell = p; goto have_cell;
}
if (v == 29) goto arm29;
if (v == 30) goto arm30;
cell = (s16 *)(c + 0xC0); goto have_cell;
arm27: ... arm28: ... arm29: ... arm30: ...
have_cell:
```

With that spelling the whole dispatch region is instruction-for-instruction
identical to the ROM: `beq +0x48`, `bgt +0x2A`, `beq +0x3A`, both default
copies at `+0x22`/`+0x32`, the shared `strh r0,[r1,#0] / mov sl,r1` store at
`+0x70` that the three branches cross-jump into, and `arm30` falling through
into it at `+0x64`.

Two ordering facts this needed:

* the `27` subtree must be the **then** block (`if (v < 29)`) so it falls
  through and the `29/30` tree is the branch target; writing `if (v > 28)`
  emits the mirrored `ble` and moves the whole region.
* every arm, including the last, must end with `cell = p;`. Dropping it from
  `arm30` (which falls through anyway) makes its tail differ from the other
  three, so jump2 cannot merge it into the shared store block, the body comes
  out 4 bytes long, and `cell` is left unset on that path.

### The 6th argument is `ctx+0x38`, not a literal

All four `sub_08007770` calls pass six arguments; the earlier draft passed `1`:

```
80215fa: movs r0,#4      ; str r0,[sp,#0]
80215fe: mov  r4,r9      ; str r4,[sp,#4]
8021602: movs r0,#0      ; adds r1,r6,#0 ; movs r2,#5 ; movs r3,#0
802160a: bl   0x8007770
```

`r9` is `ctx+0x38` -- built at `802141a` as `movs r6,#56 / adds r6,r6,r7 /
mov r9,r6`, stored to `ctx+0xAC` at `8021564`, never reassigned in between.
Passing `sub38` also moves that value out of a spill slot into `r9`. A spelling
that keeps the literal `1` scores 260/1204 but contradicts this.

## What is left

Frame is 40 bytes against the ROM's 36, and the candidate is 1208 against 1204
-- the same word. Homes:

| value | ROM | `best_368.c` |
| --- | --- | --- |
| `ctx` | `r7` | `r7` |
| `p16` | `r4` | `r4` |
| `sub38` | `r9` | `r9` |
| `cell` (`ctx+0xC0`) | `sl` | `sl` (pinned) |
| `asset` (`0x08368B0C`) | `r6` | `r8` |
| `pF4` (`ctx+0xF4`) | `r8` | stack |

`pF4` on the stack is the spare word in the frame. Pinning it
(`register u32 *pF4 __asm__("r8")`) does not help -- 324/1204, still 1208 B --
so the value has to lose the register race by allocation order rather than take
it by pin. `cell` is the one pin that pays (+3 bytes, 365 -> 368); `sub38` as
`__asm__("r9")` changes nothing.

One cross-jump line remains, `+0x402 -> +0x408` (an arm body the candidate
merges and the ROM keeps twice), separate from the dispatch issue.

## Next leads

* Drop `pF4`'s refs or shorten its live range so it loses the r8 race on the
  refs/live-length comparison instead of being pinned; that is the frame word.
* `asset` sits in `r8` where the ROM has `r6`; the ROM's `r6` is free only
  because `asset` is loaded once at `+0x106` (`mov r8,r4` in the ROM) -- check
  whether the draft's `asset` is live across the four `sub_080075E8` calls in
  a way the ROM's is not.
* The `+0x402` duplicate arm has the same shape as the dispatch duplicate
  (identical tail, one copy in the candidate); try the same `goto`-split there.
