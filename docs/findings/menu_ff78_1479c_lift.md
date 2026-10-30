# The 0x0801479C lift: a preheader fold and three zero-index liveness brackets

`MenuFF78_1479C` (0x0801479C, 316 B, `src/menu_ff78_n.c`) is now compiled C: a
six-pass loop that clamps `s16[rec+160] + i` between 0 and `s16[rec+158]`,
looks up a grid value (`sub_08025CF4`) and a tier (`_0802572C`), and calls
`sub_08007B18` once or twice per pass. The body is byte-exact in the isolated
probe and in the independent slice:

```sh
python3 tools/corpus_match_probe.py --function _08001479C --c89 \
    --require-all --require-exact     # EXACT 316/316
python3 tools/match_c_slice.py _08001479C
```

Every candidate below was graded from a scratch TU:

```sh
python3 tools/lift_loop.py src/menu_ff78_n.c MenuFF78_1479C \
    --vma 0x0801479C --len 316 --variant <body.c> [<body.c> ...]
```

The body classifies `c_fakematch`. It was landed with 32 register pins and
twelve empty asm statements (four output/input liveness brackets, eight
statements, plus four `"+r"` barriers); a removal sweep then took it to **19
pins and five empty asm statements** -- see
[Shrinking the surface](#shrinking-the-surface-44-tricks-to-28) for what
survives and why.

## The preheader is a pinned four-statement group

The ROM writes the loop's four locals from four *different* registers:

```asm
0x080147c8  movs r0, #1
0x080147ca  negs r0, r0          @ i = -1, left in r0
0x080147cc  mov  r9, r0
0x080147ce  mov  r1, r8          @ p160
0x080147d2  str  r1, [sp, #24]
0x080147d4  mov  r2, r8          @ p158
0x080147d8  str  r2, [sp, #20]
0x080147da  movs r3, #24
0x080147dc  negs r3, r3          @ offA = -24
0x080147de  str  r3, [sp, #28]
0x080147e0  subs r0, #47         @ offB = -48, folded onto the live -1
0x080147e2  str  r0, [sp, #32]
```

Two spellings decide the whole block:

* `offB = -48;` cannot produce `subs r0,#47`. agbcc materialises `-48` from a
  fresh constant and, worse, `subs r0,#48` is not even the shape: writing the
  constant reloads `i` from r9 first (`mov r0,r9 / subs r0,#47`). The fold the
  ROM has is `offB = z - 47`, where `z` is the `-1` still sitting in r0 -- and
  `z` must stay **live** there, which is what pushes `p160`, `p158` and `offA`
  off r0 and onto r1, r2, r3.
* `p158 = rec + 158` must be its own statement with its own pinned temp. Written
  after `p160 = rec + 160` from the same base, agbcc folds the difference
  (`subs r0,#2`); with both pointers named off `rec`, the ROM's `mov r2,r8 /
  adds r2,#158` returns.

The measured block:

```c
{
    register s32 z __asm__("r0");
    register u8 *t1 __asm__("r1");
    register u8 *t2 __asm__("r2");
    register s32 t3 __asm__("r3");
    z = -1;
    i = z;
    t1 = rec + 160;
    p160 = t1;
    t2 = rec + 158;
    p158 = t2;
    t3 = -24;
    offA = t3;
    offB = z - 47;
}
```

## The three zero-index scratches want liveness, not pins

`ldrsh` in this compiler is always `[base, index]` with a *materialised zero*
index, so every `*(s16 *)(p + k)` charges `movs rX,#0` to a scratch register
that `find_free_reg`/reload pick from the live set. `r1`/`r2`/`r3` all hold
something at the three sites this body has, and the ROM's choices are r3 (base
read), r2 (clamp read) and r2 (T2 table read).

A plain C body is 316 B and 154 instructions, i.e. the right length, but lands
the **first** site two instructions off:

```
ROM                               candidate
mov  r0, r8                       mov  r0, r8
adds r0, #164                     adds r0, #164
movs r3, #0                       movs r1, #0
ldrsh r2, [r0, r3]                ldrsh r2, [r0, r1]
```

Pinning the destination (`bt` r2) fixes which register the *value* gets but not
the scratch, and a source-level pinned zero is copied away before it can be
used: `zi = 0; bt = *(s16 *)(pa + zi);` still emits `movs r1,#0 / ldrsh
r2,[r0,r1]`, and making `zi` opaque (`"+r"` barrier between the assignment and
the read) restores `movs r3,#0` but adds an `adds r0,r0,r3`, 320 B.

What works is the [liveness pin](indexed_halfword_scratch.md#the-liveness-pin-removes-a-register-from-the-scratch-set):
start the unwanted register's live range with an output-only empty asm before
the read and end it with an input-only one after. No bytes are emitted.

```c
if (tag <= 3) {
    register s32 hold __asm__("r1");

    __asm__ volatile("" : "=r"(hold));
    bt = *(s16 *)(rec + 164);
    __asm__ volatile("" : : "r"(hold));
    base = bt;
}
```

That is the whole first-site fix: 316 B, 154/154, with the ROM's `movs r3,#0`.

The bracket re-rolls every later scratch in the function, so the other two
sites need their own holders, measured one at a time:

| site | ROM scratch | plain C | with the previous bracket |
| --- | --- | --- | --- |
| `s16[rec+164]` (base) | r3 | r1 | r3 (r1 bracket) |
| `s16[rec+158]` (clamp) | r2 | r2 | r3, fixed by an r3 bracket |
| `s16[T2 + idx*2]` | r2 | r2 | r3, fixed by an r3 bracket |
| `i != 4` copy before `cmp` | r0 | r3 | r3, fixed below |

The T2 site needs the read pulled out of the argument list so the bracket can
surround it; the value itself stays a plain local:

```c
register s32 hold3 __asm__("r3");
s32 tv;

t2 = (s16 *)(uintptr_t)Menu1479C_T2;
__asm__ volatile("" : "=r"(hold3));
tv = *(s16 *)((u8 *)(uintptr_t)(idx * 2) + (uintptr_t)t2);
__asm__ volatile("" : : "r"(hold3));
sub_08007B18(p16, tv, ((oa = offA), oa + pos), 64, 11, 1, 0, 0);
```

## The skip test's `cmp` copy: hold r3 across the `||`

The ROM's `i != -1 && i != 4` sequence copies `i` into r0 for the immediate
compare:

```asm
movs r0, #1
negs r0, r0
cmp  r9, r0
beq  .L89e
mov  r0, r9        @ <- the copy
cmp  r0, #4
beq  .L89e
```

Once the first bracket is in, agbcc copies into r3 instead. The bracket cannot
start before the `||` because the `q = rec` copy in the same condition needs r3
(`mov r3,r8 / ldrh r0,[r3,#36]`); the fix is to read that halfword into a local
first and then hold r3 across the short-circuit test:

```c
u16 s36;
register s32 h3 __asm__("r3");

s36 = *(u16 *)(q + 36);
__asm__ volatile("" : "=r"(h3));
if (s36 != 0 || (i != -1 && i != 4)) {
    register s16 *t1 __asm__("r1");
    __asm__ volatile("" : : "r"(h3));
    ...
```

`r2` is already live there (the clamped tier), so the copy skips r3 and r2 and
lands in r0, exactly as the ROM has it. The read's own codegen is unchanged
(`ldrh r0,[r3,#36] / cmp r0,#0 / bne`).

## Call spelling: the closure label, not the C alias

The 2-argument wrapper at 0x0802572C is `sub_0802572C` / `_0802572C` in
`asm/ai_line_more.s`. Calling the C-only alias `Sub_0802572C` compiles and
matches in the probe (it resolves the alias), but `match_c_slice` links only
the reference assembly, where that spelling does not exist:

```
arm-none-eabi-ld: code.o: in function `MenuFF78_1479C':
(.text+0x14820): undefined reference to `Sub_0802572C'
```

Spelled `_0802572C(*sym, v)` the slice links and the body is byte-identical.

## Measured shapes

All against the fixed 316 B / 154-instruction span:

| shape | bytes | insns | aligned | first |
| --- | ---: | ---: | ---: | --- |
| plain body (`bt` r2 pinned) | 316 | 154 | 152 | 0x080147c2 |
| `zi = 0` pinned r3, used in the address | 316 | 154 | 152 | 0x080147c2 |
| `zi` opaque (`"+r"` barrier) | 320 | 156 | 153 | 0x080147ca |
| `bt` as `s16`/`u16` | 316-320 | 154-156 | 150 | `ldrh` + shift pair |
| liveness bracket r1 (site 1 only) | 316 | 154 | 148 | 0x080147f6 |
| + r3 brackets (sites 2 and 3) | 316 | 154 | 152 | 0x08014844 |
| + r3 held across the skip test | 316 | 154 | **154** | none |

## Shrinking the surface: 44 tricks to 24

Removal sweep over the installed body: a pin is dropped by turning its
declaration into a plain one, a bracket by deleting both asm statements and the
dummy's declaration. Every accepted removal is followed by a re-measure, because
each one re-rolls the live set (the lesson from
[`_0800278F0`](indexed_halfword_scratch.md#_0800278f0-garage_ticker_278f0-closed-by-removing-pins-not-adding-them)).

**13 of the 32 pins and 7 of the 12 empty asm statements went.** The unexpected
part is which ones:

* the rec+158 bracket and the skip-test bracket are **not** load-bearing once
  the pins they were measured against are gone. With `r8`/`r9`, `m`, `p16` and
  the r3 pins removed, plain C hands those two reads the ROM's r2 and r0 by
  itself, 316 B and 154/154;
* the base read's r1 bracket and the T2 read's r3 bracket survive, as does
  `"+r"(inc)`;
* the preheader still cannot drop `z - 47` or the `t1`/`t2`/`t3` temps
  (dropping either pointer temp costs 4 B: 320 B / 149 aligned), but `z` itself,
  `i`, `m`, `p16`, `mt`, `q`, `ic3` and `ic` all lost their pins and still land
  in the ROM's registers.

The remainder is a local optimum: all 20 remaining removal candidates fail
alone, and all 190 pairs fail.

| dropped | bytes | aligned |
| --- | ---: | ---: |
| bracket `hold` r1 (base read) | 320 | 141 |
| bracket `hold3` r3 (T2 read) | 316 | 150 |
| barrier `"+r"(inc)` | 316 | 150 |
| `bt` r2 | 316 | 145 |
| `v` r4 | 316 | 147 |
| `raw` r0 | 316 | 147 |
| `pq` r1 | 316 | 147 |
| `pl` r1 | 316 | 147 |
| `sym2` r3 | 316 | 147 |
| `pos` r5 | 320 | 148 |
| `t1` r1 (preheader) | 320 | 149 |
| `t2` r2 (preheader) | 320 | 149 |
| `wa` r3 | 316 | 149 |
| `sym` r3 | 316 | 149 |
| `cell` r0 | 316 | 150 |
| `t3` r3 (preheader) | 316 | 151 |
| `tb` r1 | 316 | 151 |
| `ob` r3 | 316 | 152 |
| `oa` r3 | 316 | 152 |
| `inc` r2 | 316 | 152 |

The `ob`/`oa`/`inc` rows are the tell that this is allocation, not arithmetic:
dropping any of them still leaves 152 of 154 instructions aligned -- two
instructions move, the length does not.

Reproduce by dropping one trick at a time and re-grading the body:

```sh
python3 tools/lift_loop.py src/menu_ff78_n.c MenuFF78_1479C \
    --vma 0x0801479C --len 316 --variant <body.c>
```
