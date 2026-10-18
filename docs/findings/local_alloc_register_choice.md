# How agbcc picks a register the C cannot pin

`_0801A204` (`Course_LoadEntry`, `src/course_orch.c`) is the worked example, and
the reason its selector block once sat 2 bytes off at `+0x40`.

## A `register ... __asm__("rN")` pin does not bind an arithmetic destination

The two selector reads want the address formed in `r0` and the zero index in
`r2` in BOTH reads:

```asm
ldr  r4, =0x03001780
ldr  r1, =0x00001106
adds r0, r4, r1        @ address in r0
movs r2, #0            @ index in r2
ldrsh r0, [r0, r2]
...
subs r1, #2
adds r0, r4, r1        @ still r0
movs r2, #0            @ still r2
ldrsh r0, [r0, r2]
```

Spelling either local as a pinned register variable changes **nothing**:

```c
register u32 saddr __asm__("r0");
register u32 zi __asm__("r2");
saddr = wbase + selv;      /* emitted exactly as if both were plain locals */
```

Verified by direct experiment (scratch TU in `build/tst-probe/`): deleting both
pins, or adding a second `__asm__("r0")` variable, produces byte-identical
assembly. The pin is honoured when the value is a *copy* (a parameter move, the
`add r6, r0, #0` style initialisation), but an arithmetic result's destination
is a fresh pseudo the allocator picks.

## What actually decides it

`local-alloc.c`'s `find_free_reg` runs twice. The first pass only considers the
instruction's **suggestion set** -- for `(set X (plus r4 r1))` that is the set's
sources, `{r4, r1}` -- and, among the suggestions that are free, takes the
lowest-numbered register. Only if that pass finds nothing does the fallback run,
which takes the lowest-numbered free register overall.

For the first read `selv` (r1) is still live (the later `subs r1, #2` uses it),
so the suggested pass fails and the fallback lands on `r0`. For the second read
`selv` holds its last value, dies in the instruction, is suggested and is free
-- so r1 wins and the load reads `[r1, r2]`, two bytes off. The same walk is why
the zero index materialises in r1 without a live range to block it.

## The lever: extend the live range with a zero-instruction asm use

`r1` only has to be busy at the second address computation, so keep it live
with an empty asm statement placed *after* the load it would otherwise disturb:

```c
t2 = t2 + *(s16 *)(uintptr_t)(saddr + zi);
__asm__("" : : "r"(selv));     /* r1 live to here -> suggested pass fails */
```

No instruction is emitted (`.code 16` is the only artifact in the listing) and
both reads then match. Placing the use before the load would add nothing but
would pin r1 for the first read as well, which the ROM does not have.

## r7 is reachable through global-alloc, not through pins

`thumb.h` defines `FRAME_POINTER_REGNUM 7`, and r7 is eliminable to sp. That
shapes both halves of register use and is worth stating precisely, because the
first version of this note ("r7 is out of reach") was wrong and parked
`_080019200` unnecessarily:

* **Local allocation can never use r7.** `local-alloc.c`'s `find_free_reg`
  unconditionally marks every eliminable register used:
  `for (i = 0; i < sizeof eliminables / ...; i++) SET_HARD_REG_BIT (used,
  eliminables[i].from);` ("Don't use the frame pointer reg in local-alloc even
  if we may omit the frame pointer..."). A pseudo born and dead inside one
  basic block therefore never lands in r7.
* **Global allocation can.** `global_conflicts` records r7's (permanent)
  frame-pointer liveness against every allocno, but immediately after that
  scan `global.c` clears it again:
  `AND_COMPL_HARD_REG_SET (hard_reg_conflicts[i], eliminable_regset)` -- "If
  we do eliminate the register, the conflict will no longer exist". Any
  *global* allocno (a pseudo that spans basic blocks) is therefore free to
  take r7 whenever r0-r6 conflict.
* **A pseudo allocated to r7 sets `regs_ever_live[7]`.** reload() walks every
  pseudo through `mark_home_live` (reload1.c:2333, "compute which hard
  registers are now in use as homes for pseudo registers"), which sets
  `regs_ever_live[home]`; reload spilling r7 (`used_spill_regs`) sets it too.
  `thumb_function_prologue` builds its push list from `regs_ever_live`, so
  that is what emits `push {r4,r5,r6,r7,lr}` and makes the high-save scratch
  `mov r7, r8 / push {r7}`.
* **A pin is not a pseudo.** `register T x __asm__("r7")` emits raw r7
  references; `flow.c`'s `mark_used_regs` and `mark_set_1` both early-return
  for `FRAME_POINTER_REGNUM` in `elim_reg_set`, so a pinned r7 body never
  sets `regs_ever_live[7]`: the body's uses assemble, the prologue keeps
  `push {r4,r5,r6,lr}` and uses `mov r6,r8 / push {r6}`, and the epilogue
  drops r7. (That is the half the old note got right.)

The working recipe, used by `PacketBuild_08000848`, `Course_Dispatch_08BB8`
and `_080020690`, is an **unpinned temp** made into a global allocno
that conflicts with r0-r6:

* span two basic blocks with the same temp, or extend its last use with an
  empty `"+r"` barrier in a later block (`_080020690` puts the
  barrier after the tail call); a pseudo that spans blocks is left for
  global-alloc;
* keep the registers you want excluded busy inside the temp's live range with
  empty `"+r"` barriers on values already living there (`Course_Dispatch`'s
  `reg1` barrier inside the temp's range is the model);
* when two allocnos compete for the same pair of registers, the
  `allocno_compare` priority decides -- highest
  `floor_log2(refs) * refs / live_length * size` is allocated first, ties
  settled by allocno number (so the order is deterministic);
  `_080020690` gets `table` into r6 and `zero` into r7 because
  table's 5 refs over a 52-insn range outrank zero's 2 refs over 92 insns,
  and `find_reg` then propagates the taken register to every conflicting
  allocno.

Value-known-constant temps interact with this: `reg_equiv_constant` lets
reload rematerialise a constant per use instead of keeping its register live,
which is how a ROM can hold a zero in r2 through a loop and free r2 again for
the reload scratches. A *pin* makes the value opaque and its range real, so
that rematerialisation disappears -- `_08009BF8` is parked on exactly this
(zero-valued `result` plus the const-id cases that must still come out in r2).

`_080019200` (parked at 62/80 under the old note) should be re-examined with
this rule: this session's four exact matches all needed the r7 prologue.

## Loads can sink past earlier statements, and no `-f` flag stops it

`_08001B068` (`src/race_scene_a1.c`) is parked at 183/200 on one shape: the
ROM reads `a2` at `wb+0x10FE` (reusing the dying offset register as the zero
index), then computes `pca = wb+0x10CA` and reads `a3`; agbcc emits the `pca`
group first and then reads `a2` with whatever register is free by then.

The `-dr` dump of that TU (`build/era-corpus/ready-work/race_scene_a1.i.rtl`)
shows the pre-reload insn list in exact source order, so the move is not in
expansion: the `a2` load sinks later. Compiling with `-fno-regmove`,
`-fno-optimize-register-move`, `-fno-caller-saves`,
`-fno-expensive-optimizations`, `-fno-inline` and `-fno-defer-pop` one at a
time leaves the emitted order byte-identical, so it is none of those either --
what is left is reload's placement, and there is no C-level lever. Read the
RTL dump before spending time on a reorder: if the dump already matches the
ROM, the C is not the problem.

## Counters: u32 + `(u8)` narrowing, and the temp must be pinned

`_0802DBA4` (`SoundDBA4_Read`, `src/sound_deep.c`) has three
`x = (u8)(x + 1)` counters, and the ROM spells every one of them as a temp:

```asm
adds r0, r4, #1      @ r4 = counter, r0 = temp
lsls r0, r0, #24
lsrs r4, r0, #24     @ narrow back into r4
```

Three spellings, three different results (all verified in a scratch TU):

| C | emitted |
| --- | --- |
| `u8 i; i = (u8)(i + 1);` | `adds r4,r4,#1` then `lsls r0,r4,#24` / `lsrs r0,r0,#24` (narrows into the temp) |
| `u32 i; i = (u8)(i + 1u);` | all three in place in r4 |
| `register u32 t __asm__("r0"); t = i + 1u; i = (u8)t;` | the ROM shape |

The third one works because the assignment to a *pinned* variable is a copy the
pin is honoured for, even though `t`'s defining instruction is an add. `u32`
counters are also what makes the loop's entry test come out as the ROM's
`cmp r4, r1` / `bcs`: with a `u8` counter cse folds `0 < n` into `cmp r1, #0` /
`beq`, which is the same size but the wrong bytes. Type the counters `u32` and
narrow explicitly.

## Do-while counters get their `+1` hoisted to the loop head

The row loop of `_0802DBA4` ends `row = (u8)(row + 1);` and must keep that
shape. Spelled as C, `loop.c` hoists just the add to the top of the loop and
leaves the narrowing in the tail:

```asm
.L10:
	mov	r1, #0
	mov	r3, #0
	add	r4, r4, #1     @ hoisted
	...
	lsl	r4, r4, #24     @ narrowing stayed behind
	lsr	r4, r4, #24
	cmp	r4, #3
	bls	.L10
```

The same loop's inner counter (`bit`) is *not* hoisted, and neither is a
pre-tested `do/while` wrapper, so the transformation is specific to the do-while
back-edge counter. Spelling the increment as an opaque asm statement keeps it in
place and emits exactly the ROM's instruction:

```c
__asm__("add\t%0, %1, #1" : "=r"(tr) : "r"(row));   /* tr pinned r0 */
row = (u8)tr;
```

## Pointer arithmetic normally puts the pointer in operand 1

For the seed-buffer pointer the ROM forms `adds r2, r0, r4` -- `n * 2` first,
the frame address second. `(volatile u8 *)buf + (u32)n * 2u + 2u` always comes
out the other way round (`adds r2, r4, r0`), and writing the integer operand
first in the pointer sum does not change it: the pointer is normalised to
operand 1. Casting the address to `u32` and summing integers does restore the
order, but then the frame address folds back into `sp` and the add becomes the
2-operand `add rX, rX, sp`, so the register shape is lost anyway.

The working spelling pins both halves and adds them in asm:

```c
na = (u32)n * 2u;            /* lsls r0, r1, #1   (na pinned r0) */
ba = (u32)(uintptr_t)buf;    /* mov  r4, sp       (ba pinned r4) */
__asm__("add\t%0, %1, %2\n\tadd\t%0, #2" : "=r"(p) : "r"(na), "r"(ba));
```

A pinned register variable used as an asm *input* cannot be copy-propagated
away, which is what keeps the `mov r4, sp` alive here.

## `ldr %0, =const` inside an asm statement splits the literal pool

Resisting the temptation to spell a constant load in asm: agbcc emits its own
pool entries at branch targets, and an asm `ldr` gets its own separate pool
block placed later in the function. On `_0802DBA4` that moved the 0x0203FD54
word and every pool `ldr` offset with it. Pin the register whose *value* is the
constant instead and let the compiler emit the load:

```c
register u32 *g0 __asm__("r0") = (u32 *)0x0203FD54u;   /* ldr r0, .Lpool */
```

## Constants are materialised in a scratch and copied (and agbcc folds the copy)

`_08003004` (`RecordSetter_03004`, `src/foundation_runtime.c`) is parked at
107/200 on this shape, and it is the same shape three times over. The ROM
materialises every constant in a scratch register and then copies it in:

```asm
movs r1, #192        @ orval = 0x300
lsls r1, r1, #2
adds r0, r1, #0      @ copy into the phi register r0
...
ldr  r1, _mask       @ base & 0x1FF
adds r0, r1, #0
mov  r3, r8
ands r3, r0
...
movs r1, #0          @ attr12 zero, reused later
```

agbcc folds all of those: it materialises directly into the target register
(`movs r0,#192 / lsls r0,#2`), and in the else arm it even re-uses a live 0xFF
with `adds r0,#1` for 0x100. Because the copy is *folded through* the pin, a
`register u32 v __asm__("r0")` on the variable, or a separate pinned scratch,
does not bring it back -- this is the same rule as "a pinned local whose value
folds to a constant is copy-propagated away". The remaining 4 of the candidate's
196 bytes against the ROM's 200 are exactly those two copies, so the size gap is
a symptom, not the problem.

## Attributes update in place, so give each `|=` its own statement

The same function shows the opposite trap: the ROM updates both attribute words
in place (`orrs r5,r6`, `orrs r4,r3`, `orrs r4,r0`), and a single combined
expression such as

```c
h2 = (u16)((h2 | (u16)(base & 0x1FF)) | (u16)(r9 << 9));
```

makes agbcc evaluate into a temp and copy back into the pinned register. Split
into one statement per `|=` and the `orrs` instructions land in place, in the
ROM's order:

```c
h2 = (u16)(h2 | (u16)(rbase & 0x1FF));
h2 = (u16)(h2 | (u16)(r9 << 9));
```

Same for the branch join: `h0 = h0 | orval; *obj = h0;` in two statements gives
the ROM's `orrs r5,r0` / `strh r5,[r2,#0]`; as one store expression agbcc writes
the result into orval's register instead.

## `(s16)` narrowing: write the shift pair, not the cast

`sub_08004260` (100 B, promoted) sign-extends a parameter once the table
stores are done, and the ROM has the clean two-instruction form:

```
lsls r5, r2, #16
asrs r5, r5, #16
```

`ra = (s16)ang;` cannot produce it. The cast goes through thumb.md's
`extendhisi2` **expander**, which emits

```
rtx temp = gen_reg_rtx (SImode);
operands[1] = force_reg (HImode, operands[1]);
emit_insn (gen_ashlsi3 (temp, operands[1], GEN_INT (16)));
emit_insn (gen_ashrsi3 (operands[0], temp, GEN_INT (16)));
```

so the widening is `ashl TEMP,src,#16` / `ashr DST,TEMP,#16` with TEMP a fresh
SImode pseudo. Agbcc hands TEMP the *source's* own register (the source is dead
after the shift):

```
lsls r2, r2, #16      @ TEMP = r2, not the destination
asrs r5, r2, #16
```

which is the same length but the wrong registers. Writing the two shifts
separately keeps them out of the expander -- combine does not fold them back:

```c
ra = ang << 16;
ra = ra >> 16;
```

`lsls r5, r2, #16 / asrs r5, r5, #16`. Verified against the real TU (EXACT
100/100), and in isolation for the same liveness (`n7` in `build/tst-probe`).
Related: a `(u32)(u16)a` cast around such a value made agbcc keep a second,
zero-extended copy in another register and subtract *that* -- the arithmetic
around it must stay in `int`.

## Loop rotation and strength reduction are a pair

`_080012F18` (84 B) has not been matched. The ROM's loop is the CYGNUS-LOCAL
`expand_end_loop` shape: a single test at the bottom, entered by a forward
`b` to that test, with the store address recomputed every iteration
(`lsls r0,r5,#2` + `adds r1,r1,r0`). Agbcc, given the same `while (*p != -1u)`
body, instead strength-reduces the store index to a pointer (`add r7,r5,#0`,
`stmia r7!,{r0}`), which makes `loop.c` move the invariant base into the
preheader and *duplicate the exit test in front of the loop* to guard it. That
is 2 bytes of overshoot and a different block layout. Every store spelling
tried (`i*4`, `i<<2`, `((u8*)rec+188)+i*4`, a destination pointer `d[i]`, a
value local, `i = i + 1`, pointer-difference index) still reduces; `loop.c`'s
`find_mem_givs`/`strength_reduce` is what has to be avoided, not respelled.

## Halfword reads: volatile or not, and what it prints

Two promotions turned on this pair, one after the other:

* `*(volatile s16 *)p` lowers to `ldrh` **plus an explicit sign extension**
  (`lsls rN,rM,#16` / `asrs rN,rN,#16`). The ROM's `movs r1,#0 / ldrsh r0,[r5,r1]`
  is what a *non-volatile* signed halfword read emits. Same rule the s8 loads
  follow, now confirmed for s16 (`sub_080019A84`, `sub_08001096C`, 104 B each).
* The two qualifiers are a lever, not a style choice. `sub_080019A84` reads
  `rc+0x4A` twice -- once unsigned into a local, once signed for the test -- and
  the ROM really has both `ldrh r2,[r1,#0]` and `ldrsh r0,[r1,r4]`. Keeping the
  *unsigned* read volatile (and the signed reads plain) is what stops cse
  merging them into one load plus an extension; written the other way round,
  agbcc emits `ldrh` + sign extension for the test instead.
* A halfword read into an unpinned `u16`/`u32` local lands in a fresh temp and
  is copied home: `ldrh r0,[r1]` / `adds r2,r0,#0`, a 2-byte overshoot. Pinning
  the destination (`register u32 w __asm__("r2")`) restores the ROM's direct
  `ldrh r2,[r1,#0]`.
* A pinned source dies at its last use and agbcc then reuses its register as the
  arithmetic destination (`subs r2,#1` in place of the ROM's `subs r0,r2,#1`).
  `__asm__("" : : "r"(w));` between the two statements keeps it live and forces
  the fresh temp. That is the same empty-asm lever the DBA4 notes use, here
  constraining the destination rather than the register choice.

## `switch` reproduces the ROM's sparse dispatch

`sub_08001096C` dispatch in the ROM:

```
cmp r0, #0
beq case0
cmp r0, #3
beq case3
b   tail
case0: ...        (ends with `b tail`)
case3: ...        (falls through into tail)
tail: ...
```

An `if (v == 0) { ... } else if (v == 3) { ... }` chain lays case 0 inline and
branches *over* it with `bne`, because gcc orders the then-block as the
fall-through. Writing the same code as a `switch (v) { case 0: ...; break;
case 3: ...; break; }` gives the comparison chain with both case bodies out of
line, exactly as above. Converting the chain to a `switch` was the last change
needed for EXACT 104/104, with no other edit.

## Parameter pseudos vs locals: who gets the callee-saved register

Measured on `_0802CBBC` (`SoundStreamAlloc`, 120 B, EXACT):

```c
void f(void *state, void *chArray, u32 count) {
    volatile u32 *s = (volatile u32 *)state;   /* local copy  */
    volatile u32 *ch = (volatile u8 *)chArray;
```

emits `adds r6,r0,#0` / `adds r7,r1,#0` -- state in r6, chArray in r7 -- and
*every* later access inherits the swap. Dropping the locals and casting at each
use site (`((volatile u32 *)state)[11] = ...`) makes agbcc home the **parameter
pseudos** themselves, which come out `adds r7,r0,#0` / `adds r6,r1,#0` exactly
like the ROM, with no pins and no other change. So when a ROM keeps a parameter
live across calls in a callee-saved register, spell the uses off the parameter,
not off a local.

Pinning those pointers instead is *worse*: `register volatile u32 *s
__asm__("r7");` makes agbcc treat r7 as occupied, spill the frame pointer into a
stack slot and shuffle through r8 (`mov r7,r8 / push {r7} / mov r8,r0 / adds
r7,r1,#0 / mov r7,r8`), growing the body by 20 bytes. Pins alone are not a tool
here; the r7 rules are in [the r7 section above](#r7-is-reachable-through-global-alloc-not-through-pins)
-- r7 wants an unpinned global allocno, not a pin.

Two smaller levers from the same function:

* A field tested and then stored wants a local -- `u32 nxt = root[8]; if
  (nxt != 0) { s[14] = nxt; }` stores the very register that was compared;
  writing `if (root[8] != 0) { s[14] = root[8]; }` re-loads it (+2 bytes).
* Width narrowing is `u8 c = (u8)count;` (`lsls #24 / lsrs #24`), never
  `count & 0xFF` (`movs #255 / ands`, a different 4 bytes), and a do/while
  counter must be re-narrowed after the decrement (`c = (u8)(c - 1);`).

## Keeping an address *live* instead of rematerialising it

`_080012F98` (`src/menu_ff78_e.c`, PARKED 52/140) needs WA `0x03001780` live in
r4 across the body, with the `+0xFC2` read as `ldr r1,=0xFC2 / adds r0,r4,r1`:

* `0x03001780u + 0xFC2u` in one expression is folded by the **front end** into a
  single `0x03002742` pool word -- no add at all, and nothing left to keep live;
* a plain `volatile u8 *wa = (volatile u8 *)(uintptr_t)0x03001780u;` local still
  lets agbcc rematerialise the constant at each of the three uses (`ldr
  r0,[pc,..]` x3, +8 bytes);
* the SYMBOL form `RAM_DEF_03001780; wa = RAM_SYM_03001780;`
  (`include/gtadv/ram_blocks.h`, first used this way in `src/save.c`) is not a
  foldable constant, so the base stays in r4 and the offset is really added.

## A loop's `mov r8,r9` in the preheader cannot be spelled in C

`_080013B68` (`src/menu_ff78_s.c`) is PARKED at **731/740** with the ROM's
exact allocation; the missing 2 bytes are *where the copy lives*, not which
register it uses:

```
ROM                              candidate
cmp r5,r0 / bge .L5              cmp r5,r0 / bge .L5
mov r8,r9        <- preheader    ldr r4,=0x080CB6E0     <- preheader
ldr r4,=0x080CB6E0               .L6:
.L6:  ldr r1,=0x080CB6AC         mov r8,r9            <- loop header
      lsls r0,r5,#1              lsls r0,r5,#1
      adds r0,r0,r1              ldr r1,=0x080CB6AC
      adds r0,r0,r1
```

* A `register u8 *lp __asm__("r8") = base;` pin placed as the loop body's
  first statement is a `reg/v` **user variable** in the RTL
  (`(set (reg/v:SI 8 r8) (reg/v:SI 28))`). `loop.c` will not hoist it, so it
  emits as the loop's first instruction -- the register is right, the block is
  not. Pinning `base` itself to `r9` as well is what keeps the *guard* on r9
  (`add r0,r0,r9`) instead of letting `lp` coalesce into `r8` from the start.
* Hoisting it by hand (`if (guard) { pin; do { ... } while (cond with lp); }`)
  does put the copy in the preheader, immediately before the hoisted `ldr r4`,
  with the guard still on `r9` -- but the pin's live range now starts before
  the loop, so global-alloc can no longer cache `rec+174` in `r8` (it moves to
  `sl`, +4 prologue bytes) and `rec+32` is spilled. 744 B and a different
  prologue.
* The ROM's version is the **allocator's** copy: `rec+174` sits in `r8` for the
  guard, the loop's base copy kills it there, and the loop *rematerialises*
  `rec+174` (`adds r0,r6,#0` / `adds r0,#174`) rather than keeping it live.
  A `u8 *lp = base;` local (no pin) inside the loop is copy-propagated away
  entirely (736 B, no copy at all), and the same local before the loop makes
  `loop.c` hoist a copy of the *index* (`mov r7,r8`) instead of the base.
* Debugging lever: `old_agbcc -O2 -mthumb-interwork -ffunction-sections -dL
  <pre>.i -o out.s` writes `loop.c`'s decisions to `<pre>.i.loop` -- the
  movable list with `savings N not desirable` per insn, the bivs/givs and the
  preheader initialisations. That is how the `ldr r4` in the preheader was
  identified as the strength-reduced table pointer rather than a hoisted load.

## Absolute symbols and reload registers

When matching code needs a constant to reach an add in its destination
register, an assembler absolute symbol can preserve the symbol reference through
reload. An integer literal may instead be loaded into a separate pseudo and
change register assignment. `LaneDriver_02E0` uses this form for `TBL_C4BB0`.

For switches with omitted values, write a missing value as an explicit empty
case when arm order matters; `default` can change the source-order placement of
the case bodies.

## Related traps found the same way

* Two pins aimed at the same register collapse: the second is silently ignored.
* A pinned local whose value folds to a constant is copy-propagated away.
* `agbcc` has no instruction scheduler (`thumb.md` has no `flag_schedule_insns`
  consumers), so a reorder inside one basic block needs a source dependency that
  forces the desired order. `_0800B89C` reaches 244/244 by materializing the
  doubled index before incrementing `selOff`, keeping both values live across
  empty barriers, then forming `scaledIndex + (wa + selOff)`. For stage 2, split
  the stage test from the timer test and clobber `r2` across the branch so the
  timer offset is loaded afterward; keep that offset live through its volatile
  read to preserve the ROM's register choice.
* A promoted function whose ROM span carries a 2-byte zero alignment pad can
  still be staged: `promotion_screen.alignment_only` accepts a `PARTIAL` with
  `cand_bytes == rom_bytes`, `prefix + 2 == rom_bytes` and an all-zero tail
  (`_08002AA30` is the instance), and `match_c_slice.splice_section` pads the
  span itself with a location-counter bump, so the linked slice keeps the
  ROM's `00 00` rather than the assembler's `c0 46` section fill.
* A call in promoted C must use the spelling the closure defines, not a
  friendly alias: `MathRot(...)` is refused with "no VMA and not a promoted
  export" even though `0x08005ba8` is promoted, because the manifest exports
  `sub_08005BA8`. Use the `RM_CALLEE(friendly, closure)` idiom that already
  exists in `src/runtime_hud.c`.
