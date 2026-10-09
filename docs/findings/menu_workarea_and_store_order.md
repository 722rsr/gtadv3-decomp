# Work-area base, linked table entries, and store ordering

Recipes that closed these menu-lane functions to byte-exact (see
`src/menu_ff78_j.c`, `src/menu_ff78_k.c`, `src/menu_ff78_f.c`):

| function | VMA | file |
| --- | --- | --- |
| `_0800123D8` | `0x080123D8` | `src/menu_ff78_j.c` |
| `_080014C88` | `0x08014C88` | `src/menu_ff78_k.c` |
| `_080014F14` | `0x08014F14` | `src/menu_ff78_f.c` |
| `_080013AA8` | `0x08013AA8` | `src/menu_ff78_f.c` |
| `_080014E3C` | `0x08014E3C` | `src/menu_ff78_f.c` |
| `_080014EA0` | `0x08014EA0` | `src/menu_ff78_f.c` |
| `_080014D94` | `0x08014D94` | `src/menu_ff78_f.c` |
| `_080015230` | `0x08015230` | `src/menu_ff78_f.c` |

## 1. Work-area base must not fold with its offset

The IWRAM work area lives at `0x03001780`. Many functions read
`WA + small-offset` where the ROM keeps the base and the offset in **two
separate literal-pool words** and adds them at runtime:

```asm
08012400: ldr  r5, [pc, #0x8c]   ; 0x03001780
08012402: ldr  r2, [pc, #0x90]   ; 0x000010C3
08012404: adds r0, r5, r2
```

A plain C constant folds both into one word (`0x03002843`), which also costs the
extra callee-saved register the fold needs. Make the base an
**assembler-resolved symbol** declared and defined inside the body:

```c
extern u8 J123D8_WA[];
__asm__(".globl J123D8_WA\nJ123D8_WA = 0x03001780\n");
wa = (u8 *)(uintptr_t)J123D8_WA;
```

Then `wa + 0x10C3u` stays a register add. The offset materialises either as a
second pool word (when it needs more than a synthesised immediate, e.g.
`0x10C3`) or as `movs #imm; lsls #n` (when it fits, e.g. `0x5E0 = 0xBC << 3`).

Two placement notes, both load-bearing:

* If a hoisted base load steals an extra callee-saved register (the ROM pushes
  only `{r4,r5,lr}`), scope the `wa` assignment to a nested block at its first
  use. In `_080123D8` and `_080014C88` that is what stops agbcc keeping the
  base live across the whole function.
* The load must sit *after* any preceding call. Assigning `wa` after the call
  (`_080012854`) rather than at the top moves the `ldr` to its ROM position.

`_080014C88` also needs the ternary written as `if (cond == 0) v = 2; else v = 1`
— not `cond != 0 ? 1 : 2` — to get the ROM's `bne` over the out-of-line arm.

## 2. Linked table entries: derive `+4` at runtime

`_080013AA8` reads a two-word ROM table entry at `0x080CB6B8 + idx*8`. The ROM
loads the base once and derives the second word with `adds r3,#4`:

```asm
08013ac4: adds r1, r0, r3   ; off + base
08013ac6: ldr  r2, [r1, #0]
08013ac8: adds r3, #4       ; base + 4
08013aca: adds r0, r0, r3   ; off + base + 4
08013acc: ldr  r3, [r0, #0]
```

Three things are required, each checked by compiling and diffing:

* The base is again an assembler-resolved symbol, so `base + 4` stays a
  register add instead of a second `0x080CB6BC` pool word.
* The `+4` pointer is a **named variable assigned after the first load**:

  ```c
  extern u8 J13AA8_TBL[];
  __asm__(".globl J13AA8_TBL\nJ13AA8_TBL = 0x080CB6B8\n");
  tbl  = (u8 *)(uintptr_t)J13AA8_TBL;
  off  = (u32)(s32)*(s16 *)(uintptr_t)(rec + 172) << 3;
  r2 = *(volatile u32 *)(uintptr_t)(tbl + off);
  tbl4 = tbl + 4;                       /* assigned *after* r2, before r3 */
  r3 = *(volatile u32 *)(uintptr_t)(tbl4 + off);
  ```

  Assigning `tbl4` before `r2` reorders the `adds` and breaks the match. Writing
  the second read as `tbl + off + 4` shares a CSE with the first and collapses to
  `ldr r3,[r1,#4]`; with `tbl` a plain C constant it folds to a second pool word.

`s16` rec reads must stay **non-volatile** (`movs rN,#0; ldrsh`), and an `int`
destination is needed when the value is only compared for equality, otherwise
agbcc drops the sign extension and emits `ldrh`.

## 3. Pinning a store constant's register and order

`_080014F14` needs two stores in one arm where the ROM materialises the second
arm's `0` into `r2` (reusing the dead `ev`) *before* the `2`:

```asm
08014f3a: adds r0, r4, #0
08014f3c: adds r0, #0x8c     ; rec+140
08014f3e: movs r2, #0
08014f40: movs r1, #2
08014f42: strh r1, [r0, #0]
08014f44: strh r2, [r3, #0]  ; cell
```

Two levers get agbcc there:

* Put the rec+140 address in a named `p16` variable so its computation happens
  before the constants are materialised.
* Pin both constants with GNU local register variables:

  ```c
  volatile s16 *p16;
  register s16 zero __asm__("r2");
  register s16 two  __asm__("r1");
  p16 = (volatile s16 *)(uintptr_t)(rec + 140);
  zero = 0;
  two  = 2;
  *p16 = two;
  *(volatile s16 *)(uintptr_t)(wa + 0x5E0u) = zero;
  ```

  Pinning the pointer itself (`register ... *p16 __asm__("r0")`) instead spills
  `rec` to `ip` and breaks the whole allocation — pin only the constants, and let
  `p16` fall into the remaining free register.

### Pinning a sign-extended `s16` load

The same trick, but the pin must be `int`, not `s16`:

```c
register int g __asm__("r4");     /* good:  ldrsh r4,[r0,r2]         */
register s16 g __asm__("r4");     /* bad:   ldrh r4; lsls; asrs r5    */
g = *(s16 *)(uintptr_t)(rec + 166);
```

`_080013AF4` keeps the rec+166 gate value in `r4` and the rec+178 selector in
`r5`; agbcc's default is the reverse (it gives the lower register to the
selector, which has one more reference). Pinning an `s16` makes agbcc load the
raw halfword into the pinned register and sign-extend through a second one; an
`int` destination lets it use the ROM's `ldrsh` form directly.

That function also needs a `switch`, not `if (v == 0) ... else if (v == 1)`: the
ROM lays out a `cmp;beq case0;cmp;beq case1;b end` chain with the case bodies
after it, whereas an if/else chain inlines case 0 and reuses the running
`rec+178` pointer (which the `beq` target cannot).

## 4. Signed `int` vs `s16` reads, and unsigned RMW arms

The event handlers `_080014E3C` / `_080014EA0` / `_080014D94` read one s16
"cell" field several times. The ROM's load form depends on what the value feeds:

* Entry snapshot and the clamp compares are **`ldrsh`**, so the destination must
  be an `int` (`int entry = *(s16 *)(rec + 138);`). An `s16` destination that is
  only tested for equality / magnitude drops the extension: `ldrh` + a copy, or
  `ldrh; lsls; asrs` through a second register.
* The decrement/increment arms are **`ldrh; subs/adds; strh`** with no
  extension at all, i.e. an unsigned read-modify-write:
  `*(u16 *)(rec + 138) = (u16)(*(u16 *)(rec + 138) - 1);`.
* The reads must be **non-volatile** (`u8 *rec`, not `volatile u8 *rec`): a
  volatile read forces a `ldrh`+shift even when the destination is `int`.

The final compare order matters too: the ROM emits `cmp r7, r0` (snapshot
first), which is `if (entry != c)` rather than `if (c != entry)`.

## 5. Range tests: `switch` beats `&&` / `||`

`_080015230` maps car ids `27..30` to `1`, everything else to `6`. The ROM
range-checks with `cmp #30; bgt; cmp #27; blt`:

```asm
cmp r0, #30
bgt six
cmp r0, #27
blt six
```

Every `&&` / `||` formulation folds to a subtract range test
(`ldrh; lsls; ldr r?,#-27; adds; lsrs; cmp #3; bhi`) — and the fold brings the
`ldrh` read with it. A `switch` over the four cases reproduces the ROM chain
verbatim:

```c
switch (car) {
case 27: case 28: case 29: case 30: ... = 1; break;
default: ... = 6; break;
}
```

Placing the `movs r0,#0` lazily is also load-bearing: compute the store address
into a named pointer first, then materialise the constant, then store
(`u8 *p88 = rec + 88; zero = 0; *p88 = zero;`). Assigning the constant first
hoists it above the address computation.

## 6. Table base as a named pointer fixes register roles

`_080014E3C` / `_080014EA0` index a u16 table at `0x080CB7CE` / `0x080CB7E8`. An
inline expression makes agbcc compute the index in `r0` and load the constant
into `r1`; the ROM loads the pool word into `r0` first and shifts the index into
`r1`. Naming the base as its own pointer forces the ROM order:

```c
extern u8 E3C_TBL[];
u8 *tbl;
__asm__(".globl E3C_TBL\nE3C_TBL = 0x080CB7CE\n");
tbl = (u8 *)(uintptr_t)E3C_TBL;
v = *(u16 *)(uintptr_t)(tbl + ((u32)entry << 1));
```

In `_080014EA0` the base assignment must additionally come **before** the reload
that feeds the index — writing the reload first lets agbcc take `r0` as the
reload's zero-index scratch and load the base second.

`_080014EA0`'s ev==1 arm re-reads the cell after the `2B3A4` call, so the index
uses a fresh `ldrsh` rather than the preserved `entry`; keep it a plain re-read
(not `entry`), which is what stops agbcc reusing `r7`.

## 7. Work-area base reused across two gate reads

`_080015230` reads `WA+0xFBC` then `WA+0x10C3`. The ROM loads `0x03001780` into
`r2` **once** and keeps it there across both accesses (offsets as separate pool
words). Model the base as a single `u8 *wa` variable assigned **after** the first
store:

```c
*(u16 *)(uintptr_t)base = 0;      /* base = rec+136, already +136 */
wa = (u8 *)(uintptr_t)MenuWaBaseF;
```

The `movs r0,#0` for the first store finds `r0` only if the base parameter is
consumed first; a `register u32 zero __asm__("r0")` pin there is a trap, because
it forces agbcc to copy the `r0` parameter into `r2` before the `+136` add
(`adds r2,r0,#0`), one instruction too many.

## 8. A manifest entry with no `export` list publishes only its `c_name`

`0x0802b3a4` has no `export` key in `tools/matching_slice_functions.json`, so the
spliced section defines only `_0802B3A4`. A promoted body that calls
`sub_0802B3A4` compiles and probes byte-exact, then **fails the slice link** with
`undefined reference to 'sub_0802B3A4'`. Call the `c_name` spelling instead:

```c
FF_CALLEE(Sub_08002B3A4, _0802B3A4)();
```

Check `export` for every callee before promoting; the byte probe does not catch
this class of error.

## 9. Trailing two-byte `00 00`

When a body's span ends in `00 00` but gas closes the Thumb section with the
`46c0` nop, add a file-scope `__asm__(".align 2, 0");` after the body's `.size`
(still inside its section). See `src/menu_c2c4.c` for the original example.

## 10. Large 16-bit constant store: use a pointer local + a `u16` temp

`_080013FE4` case 0 stores the pooled constant `0xFFD0`. A bare literal store
`*(u16*)(rec+164) = 0xFFD0;` makes agbcc load the 32-bit pool word into a scratch
and copy it down (`ldr r3,=0xFFD0; adds r0,r3,#0; strh r0,[r1]`), one instruction
more than the ROM. Writing the value through a `u16 *` local plus a `u16` temp:

```c
u16 *p = (u16 *)(uintptr_t)(rec + 164);
u16 t = 0xFFD0;
*p = t;
*(u16 *)(uintptr_t)(rec + 166) = 2;
```

makes agbcc compute the address first and `ldr r0,=0xFFD0; strh r0,[r1]` directly.
A `register u16 t __asm__("r0")` pin is a trap here: the store still round-trips
through a scratch and the load hoists above the address computation.

## 11. Two-arm inline `if/else if` -> use `switch`

When the ROM keeps **both** bodies out-of-line behind `beq` tests with a
fall-through `b end` between them, write the two arms as a `switch`, not an
`if/else if`. `if/else if` inlines the first body and skips the extra branch:

- `_080013EB0` case 5 (`s16[rec+186]` -> 0/1 body)
- `_080014AC4` (`s16[WA+0xFF2]` -> 0/1 body)

## 12. Work-area base: local variable, plus a second alias for a non-CSE'd reload

Assigning the assembler-resolved base to a **local** is what keeps `base` and the
offset as two pool words (`ldr r0,=WA; ldr r1,=off; adds r0,r0,r1`); adding the
offset to the raw array expression folds to `base+off`.

When the ROM carries a **second** copy of the base literal (two identical pool
words) because one access must **not** reuse the register the base was cached in
(`sl`), declare a second alias symbol for the very same address:

```c
extern u8 J_WA2[];
__asm__(".globl J_WA2\nJ_WA2 = 0x03001780\n");
u8 *wa2 = (u8 *)(uintptr_t)J_WA2;   /* fresh `ldr r0,=0x03001780` */
```

`_080012854`'s final `WA+0x1058` store needs this; the earlier `WA+0xFBC/0xFBE/0x574`
accesses use the first alias cached in `sl`.

## 13. Ordering a constant/zero against a sibling store

`_080012854` materialises the final `WA+0x1058` store's zero into `r2` **between**
the `rec+128` copy's address computation and its `strh`. Two levers:

```c
register u16 z __asm__("r2");       /* pin so the zero lands in r2, not r1 */
s16 v  = *(s16 *)(uintptr_t)(rec + 130);
s16 *dst = (s16 *)(uintptr_t)(rec + 128);  /* compute dst before the zero */
z = 0;
*dst = v;
```

The destination-pointer local is what pushes the address computation ahead of the
zero; placing `z = 0;` before the `dst` assignment lands the `movs` one
instruction too early.

## 14. A reused pointer field is a struct offset, not a ROM word

`_080012854`'s `... = 2` store reuses `r4`, which the earlier
`*(u32*)(rec+120) = (u32)(rec+140)` left pointing at `rec+140`; the target is
`rec+140`, **not** the `0x082A798C` template held in the pool. Reusing the
register across the two statements is what the ROM does; the pool word belongs to
the three `Sub_08007614` calls.

## 15. One pointer variable, not two

A `void *rec_` parameter cast to `u8 *rec` and then used both as `rec_` (in the
`void *` call arguments) and as `rec` (in `rec + off`) makes agbcc keep **two**
live copies and spill `r8`/`r9`. Call every callee through the one cast pointer:

```c
u8 *rec = (u8 *)rec_;
...
Sub_080014650((void *)rec, 128, 104);   /* not rec_ */
```

With the param. unified, `_080014AC4` dropped from OVERSIZED to a 2-byte pad
difference. Complementarily, writing `(void*)(uintptr_t)(rec + 8)` inline at each
call (rather than a hoisted `u8 *p = rec + 8;`) is what lets agbcc re-derive the
`rec+8` address per branch, as the ROM does.

Also check the arity of the *tail* calls: `_080014AC4`'s final
`Sub_080014BCC((void *)rec)` needs the `rec` arg or the ROM's `adds r0,r5,#0`
before the `bl` is missing.

## 16. `volatile s16` reads do NOT emit `ldrsh` — drop the volatile

An `s16` read written through a `u8 *` base

```c
*(volatile s16 *)(uintptr_t)(rec + 174)
```

does **not** compile to the ROM's one-instruction `ldrsh`. agbcc loads the
volatile halfword with `ldrh` and sign-extends it with two shifts:

```
ldrh r1, [r2, #0]
lsls r1, r1, #16
asrs r1, r1, #16
```

which is 2 bytes longer and lands the first difference at the read. The ROM's
`movs r0, #0` / `ldrsh r1, [r3, r0]` shape comes from the **non-volatile** form:

```c
*(s16 *)(uintptr_t)(rec + 174)
```

This is exactly what `src/menus.c:MenuEcac_0800ECAC` documents, and it is why
`_080013B68` jumps from 275/740 to 332/740 the moment the eight
`*(volatile s16 *)` reads become `*(s16 *)`. agbcc still **reloads** the cell
after every call (it cannot CSE across a call), which is what the ROM does, so
the "read it at each use site" rule and the non-volatile rule are the same edit.
The `u8` / `u32` reads are unaffected (`ldrb` / `ldr`).

## 17. An assembler-resolved base defeats base+imm constant folding

For `_080013B68`'s `0x080CCEEC` lim table, writing the literal

```c
base[off + 8]                     /* base = (u8*)0x080CCEECu */
```

makes agbcc fold `base + 8` into the pool word (`0x080CCEF4`) and emit
`ldrsb r1, [r0, #0]`. The ROM keeps the base in `r9` and emits
`movs r1, #8; ldrsb r1, [r0, r1]`. Declaring the base as an **assembler alias**
and copying it through a local pointer blocks the fold:

```c
extern u8 J13B68_BASE[];
__asm__(".globl J13B68_BASE\nJ13B68_BASE = 0x080CCEEC\n");
u8 *base = (u8 *)(uintptr_t)J13B68_BASE;
```

The same alias trick gives `_080013B68` a single pooled `0x080CCEEC` that agbcc
hoists into `r9`, with the pool words then in byte-identical order
(`0x080CCEEC`, `0x030017B0`, `0x080CB6E0`, `0x080CB6AC`). It reaches **641/740**
with only the loop-entry register copy left: the ROM materialises the loop's
`base` in `r8` with a `mov r8, r9` at loop entry (and a two-instruction
`adds r7,r6,#0 / adds r7,#32` for `rec+32`), while every source spelling tried
(`base2 = base`, `if + do/while`, a `s8 *` copy) either folds the copy into the
pre-loop register `r7` or breaks the prologue. The copy is a register-allocator
artifact of the *loop* occurrence of the condition; a formulation that pins
`rec+32` to `r7` before the copy is claimed would close it.

Both menu-table bases (`0x080CB6AC`, `0x080CB6E0`) are likewise assembler aliases
(`J13B68_E`) so the e-base reload stays inside the loop instead of being
strength-reduced into a pointer that advances by 2.

## 18. `A && B` + a following `if` may be a NESTED `if (A) { if (B) ...; if (C) ...; }`

`_0800139F0` was written as

```c
if (*(s16*)(rec + 186) == 0 && a1 != 0) { ...A... }
if ((int)a1 != *(s16*)(rec + 184))       { ...B... }
```

which compiles to `cmp s16[186],#0; bne <after B>` — i.e. a non-zero `s16[186]`
still runs **B**. The ROM's `bne` at +0x12 goes to the function *epilogue*
(0xb0), so both A and B are skipped, and only an `a1 == 0` falls through to
the `s16[184]` test. The source shape that reproduces it nests them:

```c
if (*(s16*)(rec + 186) == 0) {
    if (a1 != 0)                    { ...A... }
    if ((int)a1 != *(s16*)(rec + 184)) { ...B... }
}
```

With that plus the `volatile`→non-volatile `s16` reads (section 16), the body
is byte-exact at 184/184 and was promoted. Its two-arm `if/else if` also had to
become a `switch` (section 11) so both arms sit out-of-line behind the compare
chain the ROM emits.
