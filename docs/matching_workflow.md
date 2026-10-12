# Build verification and byte matching

The matching build uses the pinned `old_agbcc` compiler with
`-O2 -mthumb-interwork -ffunction-sections`. See [setup](../README.md),
[compiler behavior](findings/compiler_split.md), and
[completion criteria](decompilation-roadmap.md).

## Verification commands

```sh
make toolchain
make matching-ready
git diff --check
```

`matching-ready` checks the reference ROM hash, generates compiler-compatible
C copies, checks original/transformed object equivalence, scores the C corpus,
and links the selected C functions into the independent executable slice.
It also runs an instruction-mutation negative control, runtime-provenance
comparison, and source/data ownership checks.

Generated source, objects, reports, captures, and extracted assets belong in
ignored output directories. Never commit the reference ROM or derived binaries.
The checked-in inventories contain metadata and hashes, not replacement code.

For a focused comparison:

```sh
python3 tools/corpus_match_probe.py --function NAME --c89 --require-all --require-exact
make fast-check
```

`fast-check` runs tool regressions and static audits. It does not replace the
independent link. On a fresh checkout, first run `matching-ready` to generate
the corpus report and assembled symbol closure used by these audits.
`make matching-slice` checks the selected C slice and its
negative control. Assembly edits also require `make independent-slice` and
`make ownership-map` to refresh source hashes before auditing the inventories.
Optional search and synthesis tools have a separate regression target,
`make experimental-check`; it is not part of `fast-check` or the ROM gate.

## Function ownership and relocation

Read the ROM instructions to establish parameter registers, return behavior,
access widths, and control flow. A source name or existing prototype is not
ABI evidence. File offset equals ROM VMA minus `0x08000000`.

Each ROM entry has one canonical C body. Alias spellings of that body do not
create separate ownership. The selected entries and their source paths live in
`tools/matching_slice_functions.json`.

Resolve symbols from the assembled include closure rooted at `asm/code.s`.
Parsing an address out of a VMA-shaped name does not establish that a symbol
exists. Inspect `arm-none-eabi-nm -n build-code/code.o` and the exact C spelling
in `src/`; preserve required ARM aliases.
A selected C export may be valid even if it is absent from the assembly object:
the C section provides it. `CpuSet` is one such example.

The splicer retains a function's section, not its entire translation unit.
Absolute symbols needed by a body must be defined inside that section, or in
retained assembly. A file-scope `__asm__(".set...")` is not sufficient.
An export list must preserve entry aliases without moving an interior label
to the start of its owner.

Branch targets and literal-pool references into a replaced span require special
care. Define an interior label at its exact offset inside the retained C
section, or keep the relevant source assembly. A first-statement inline label
can follow a compiler prologue and therefore land at entry +2 rather than +0.
A naked hardware-contract transcription can place a label precisely. Inspect
all external references, including `ldr` pool operands, before replacement;
static checks alone do not prove the linked references remain valid.

Check end labels in their owning source files. Inventing a missing
`<stem>_end` label or borrowing a neighboring region's boundary can replace
unrelated code.

## Span and alignment checks

Run `tools/span_audit.py` before diagnosing instruction selection. A missing
typed entry can make one function's span swallow another. Bare labels do not
necessarily define function entries; use `.type NAME, %function` where the
ROM establishes one. Do not infer boundaries from name spelling alone.

The probe's `prefix` is the contiguous matching run from the start.
`matched_bytes` counts equal bytes at any offsets, so it cannot establish
that a difference is confined to alignment. Use the predicate implemented by
`promotion_screen.alignment_only`, not a formula based on a total byte count.

For content length congruent to 2 modulo 4, gas may pad a function section
with `c0 46` while the ROM contains `00 00`. The ROM span includes padding:
testing the span length instead of content length reverses this diagnostic.
Compare both tails even when an earlier instruction also differs.
An in-section `__asm__(".align 2, 0");` can reproduce the zero padding.
Do not assume an alignment mismatch is unavoidable, or that every two-byte
residual is alignment-only.

`OVERSIZED` means only that the candidate exceeds the ROM span. A small
length difference does not imply a nearly matching instruction stream.
Establish the correct length and cause before tuning register allocation.
See [candidate-length diagnostics](findings/oversized_plus4_class.md).

## Compiler constraints

- A volatile signed halfword lvalue can emit `ldrh; lsl; asr` where a
  non-volatile read emits `mov rN,#0; ldrsh`. Distinguish a volatile pointer
  cell from a volatile pointee. IWRAM work areas are not hardware registers;
  preserve actual hardware volatility and access counts.
- Parameter widths follow the ROM. A narrow C parameter can introduce
  truncation absent from an already word-width calling convention.
- Named locals and their declaration positions can preserve load ordering.
  An inline expression may be folded or hoisted differently.
- A shared named constant can preserve one materialization used by a store
  and a call. Duplicating a literal can introduce a second instruction.
- GNU local register variables constrain a specific pseudo. Pinning a base
  pointer is not equivalent to pinning a loaded cell word. Document the
  required ROM register; verify each body's output independently.
- An empty read/write asm constraint such as `__asm__("" : "+r"(value))`
  can prevent rematerialization. Combined with a pinned register and later
  reuse, it preserves the source-to-return copy in the runtime entry writers
  at `0x0800315C`, `0x08003194`, and `0x08003210`.
- An assembler-resolved absolute base can preserve a pool load before address
  arithmetic. Keep its definition in the function section. See
  [the track-award example](findings/track_car_26180_pool_order.md).
- A `static` helper introduces a call unless the compiler actually inlines it.
  If the ROM has no such call, express the operation in the owning body. The
  symptom is worth recognising: `_080015A0C`/`_080015AB0` (twins of one shared
  body, 164 B each) reported UNRESOLVED_RELOCATION at 1/164 with a 32-byte
  candidate -- because a static helper called *twice* is never inlined at -O2,
  so each wrapper was a call stub whose callee had no ROM address. Turning the
  body into a `do { ... } while (0)` MACRO fixed the relocations and moved the
  pair to 27/164 and 19/164 (the rest is the pool word below).
- Declare the machine arity the CALLER uses, not the one the callee reads.
  `Sub_0800159AC` touches r0-r3 only, but the ROM caller still stores a fifth
  word into the outgoing-argument slot (`mov r4,r8 / ldr r0,[r4] / str r0,[sp]`
  at 0x080015A68), so a four-parameter declaration drops that store and the
  body measures 140 bytes instead of 164. Check with `tools/arity_audit.py`
  before concluding a body is too short.
- A base that the ROM materialises at run time (`adds r6,r4,#4`) must not be
  spelled as a folded constant: `(tbl) + 4 + off` with a literal `tbl` folds
  `tbl+4` into a SECOND pool word (0x080CB81C) and loses 4 bytes, while the ROM
  pools `tbl` alone and adds 4 at run time. `tblp[1 + s*2]` (a `u32 *` pointer
  variable) produces the ROM form, but it also reallocates the whole body; on
  `_080015A0C` every spelling of that base was measured before settling.
- Switches, conditional expressions, and if/else chains can produce different
  instruction forms. Semantically equal C need not be byte-equivalent.
- Not every ROM index expression was strength reduced. agbcc *drops* a
  marginal giv at `loop.c:4597`
  (`! flag_reduce_all_givs && v->lifetime * threshold * benefit < insn_count`
  sets `v->ignore`), and the original build dropped some that this one keeps.
  `0x0802494C`/`0x080249AC` are the measured cases: the ROM re-derives
  `base + 0xA4 + i*8` per iteration (`lsls r1,r4,#3 / mov r0,ip / adds r0,#164
  / adds r1,r1,r0`) and therefore keeps `i` live, which is separately what
  stops the counter being rewritten as a countdown. Every spelling of that
  destination that reaches the giv pass -- `ioff` in its own statement,
  `(i<<3)`, `i*8`, a `u32 *` subscript, a `u32`-casted local, a pointer local
  assigned in the loop, with and without `volatile` -- gives the same body with
  the destination reduced into a fourth rolling register. Making the index
  non-affine (`u16 i`) does drop the giv but wraps the counter and stops `i*72`
  from rolling, which is worse. The same wall governs how a non-immediate
  offset (`0x5E8`) is carried: as a running register in the ROM, rematerialised
  from the literal pool here.

- Two constants added at different levels do not always fuse, and the
  encoding depends on *which* level. `cell8 - 36 + 14` and
  `((u16 *)(cell8 - 36))[7]` both fold to one `sub #22 / ldrh [r0]`, but a
  struct cast with the field at +14 (`((struct S *)(cell8 - 36))->v`) keeps
  `sub #36 / ldrh [r0,#14]` **and** leaves the ternary's `movs r2,#0` first,
  while a named local keeps the split but emits the `sub` ahead of the
  default. Eight spellings were measured on the pinned compiler in a scratch
  TU (`build/foldprobe.c`, the same `-O2 -mthumb-interwork` pipeline the probe
  uses) before touching the real body: replicate that when a single expression
  has to produce a specific two-instruction pair.
- Absolute literals that the ROM derives from a value already in a register
  must be written as that value plus an offset, not as a fresh constant. The
  `0x08001D288` pool holds neither `0x03004E80` nor `0x03004E8E`: the first is
  the cell minus 36 (`subs r5,#36`) and the second is that plus 14
  (`ldrh r5,[r5,#14]`). Likewise `0x03001780 + 0x10C3` stays two pool words
  (`ldr / ldr / adds`) -- see `WA_SPLIT`, which `_08001DB0C` already links
  EXACT with, whereas `WA_U8(o)`'s folded `WA + o` collapses to one word.
- Pool order follows the *first reference in the emitted code*, so an argument
  that has to be pooled ahead of another must be stated first, and a deref that
  must not sit between two pool loads has to be a separate statement. Lifting
  all three of `ldr r0,=TABLE / ldr r5,=CELL / ldr r2,[r0] / adds r0,r5,#0` into
  one call expression or one statement loses one of the three.

Compiler RTL dumps (`-da`) help distinguish instruction selection,
scheduling, and register allocation. A failed finite search establishes a
limit of the tested forms, not impossibility of every C representation.

For additional controlled experiments, see the
[newer agbcc recipe cookbook](findings/agbcc_matching_recipes.md): narrow-mode
locals, loop-note barriers, allocator diagnostics, and reproducible transfer
probes. It separates upstream observations from locally reproduced results.

## Selecting C sections

Use a fresh `build/era-corpus/ready-report.json` and its `ready-work/` objects.
Other corpus directories may contain stale output. Screen candidates with
`tools/promotion_screen.py`, then run `tools/call_audit.py` and
`tools/export_audit.py` before the independent link. The screen accepts exact
and verified alignment-only candidates; the link remains authoritative.

Do not edit source while its full verification is running. Isolated diagnostic
output can be selected with the probe's `--work-dir` and `--json` arguments.
Review the resulting manifest diff and rerun `make matching-ready` after
changing selected functions, exports, paths, or assembly boundaries.

## Register pins: the last 1-2 instructions

When a body matches everything except one or two instructions whose *only*
difference is which register participates, stop re-spelling the expression and
pin the register instead. This is what closed `_0800121A8` (194 -> 196) and
`_08009B60` (98 -> 108):

* pin the destination of a repeated load to the ROM's register
  (`register uintptr_t woffs __asm__("r3")`);
* pin a base pointer to materialise `mov rX,sp` where the ROM has it, and
  declare that pin *before* the hoisted constant to control which of the two
  lands first (`pkt0` before `zero`);
* pin the intermediate of the arithmetic chain so the final value lands in the
  ROM's register (`register int floored __asm__("r0")`).

Two pitfalls:

* Pinning the *sum* (`register uintptr_t wsum __asm__("r2") = a + b;`) makes
  agbcc encode the add with the operands swapped (`adds r2,r3,r2`). Pin the
  first operand instead and leave the sum plain.
* Association controls which register receives the result. `off + (base +
  offs)` accumulates into the dead offset register (`adds r1,r1,r2`), while the
  left-associative `base + offs + off` overwrites the base (170 vs 172 bytes).

## Suppressing loop strength reduction (the address giv)

When a loop's body in the ROM *recomputes* an indexed address every iteration
(`lsls r0,r5,#2 / adds r1,r6,#0 / adds r1,#188 / adds r1,r1,r0`) but agbcc
instead emits an auto-increment pointer (`stmia r7!,{r0}`) and a wider
callee-saved set (`push {r4,r5,r6,r7,lr}`), the difference is agbcc's
`DEST_ADDR` giv. On ARM `AUTO_INC_DEC` is defined, so the benefit test in
`loop.c:4560-4584` *adds back* `add_cost * biv_count` for any address giv whose
multiplier equals the access size, and the drop test at `loop.c:4597`
(`lifetime * threshold * benefit < insn_count`) almost never fires. Re-spelling
the loop (`while`/`do-while`/`for(;;)`, subscript vs pointer vs offset) does
not help -- agbcc canonicalises all of them to the same giv.

Two devices defeat it, both verified on `_080012F18` (17 -> 84/84 bytes):

* **Pin the scaled index and keep the base plain.**
  `register u32 idx_ __asm__("r0") = r5 * 4;` followed by a plain
  `u8 *b_ = rec + 188;` and `*(u32 *)(b_ + idx_) = *r4;`. The pinned index is
  an asm operand, so the address source is no longer recognised as a giv; the
  base must NOT be pinned too or the constant add moves ahead of the `lsls`
  (pin base to r1 as well and the order flips, 67 vs 70 matched bytes).
* **An `__asm__ volatile("" : "+r"(count));` barrier on the loop count inside
  the loop** works as well (`+r` on the count), but the pin above is enough on
  its own, so prefer it -- it also fixes the instruction order.

State the *declaration order* to control instruction order: the ROM computes
the value (`subs r1,r5,#1`) before the base copy (`adds r0,r6,#0`), so declare
the value pin first. Chaining a pointer across statements (`e_ += 4;`) is what
reproduces the ROM's register reuse (`adds r0,#4` reusing the `rec+184`
register); recomputing the address from scratch costs the match.

## Constant addresses: symbol or cast constant

A ROM table address used as a pointer decides where the pool load lands, and
that is a 2-3 instruction difference:

* `u16 *tab = (u16 *)0x080CC098;` lets agbcc rematerialise the address at its
  use, so the `ldr rX,[pc]` is emitted just before the `adds` and the register
  it would have taken is free for something else (the `ldrsh` zero index moved
  from r2 to r1 in `_080021F14`).
* Giving the address a symbol (`__asm__(".globl T\nT = 0x080CC098\n");` plus
  `extern u16 T[];`, with the `.globl` inside the function as in
  `WA_SPLIT_DECL`) keeps the load at its definition point, which is what the
  ROM has. The assignment emits no bytes, so no other measurement moves.

Same file, same body: symbol 116/116 exact, cast constant 104/116.

## Switch dispatch shape

* The dispatch tree over `{0,1,2,3}` is a balanced binary search; the arms are
  laid out in **source order**, so write `case 1: case 3:` before `case 0:
  case 2:` when the ROM has the {1,3} arm first.
* The mode word's signedness is visible: `bgt` means the source read it as
  `s32`, `bcc` means `u32` (`_080021F14`: `*(s32 *)(rec + 160)`).

## Halfword loads: keep the value's width

`int v = s16tab[i];` produces the ROM's single `ldrsh`. Declaring the result
`s16` instead makes agbcc load with `ldrh` and emit a `lsls #16 / asrs #16`
sign-extend at each use (twice, when both switch arms pass it)
(`_080022750`).

## Indexed halfword loads (`movs rX,#imm` + `ldrsh rY,[rZ,rX]`)

agbcc has two spellings for `*(s16 *)((u8 *)base + imm)` and the ROM picks one:

* **Fold** — `adds rX, base, #imm` / `movs rZ, #0` / `ldrsh rY, [rX, rZ]`. This
  appears when the base is the incoming argument register and it is the last
  use (the register is dead right after), e.g. the first field read of
  `_080019250`.
* **Index** — `movs rX, #imm` / `ldrsh rY, [base, rX]`. This appears when the
  base register stays live, so the constant gets its own register. Reference
  oracle: `Idle_Mode4Handler` (`_08001A74`, already EXACT) reads one field
  twice -- `ldrh r2,[r1,#0x14]` (volatile `u16`) then `movs r3,#0x14` /
  `ldrsh r0,[r1,r3]` (plain `s16`); the plain read takes the index form
  *because* `r1` is live for the next statement.

Consequence: a body whose first field read is `ldrsh rY,[r7,r0]` with `r7` a
pinned copy of the argument needs the pinned variable to be the only home of
that value (an argument copy in `r0` cannot be reused). Pinning the parameter
(`register void *a __asm__("r7") = a_;`) is *not* enough -- agbcc still folds
through the argument pseudo. Measured on `_080019250`: 6 spellings (pinned
offset, `s16 *` pointer variables, `s16[]` indexing, `u8 *` temporaries,
`+r` barriers, `"r0"` clobber barriers) all stay at 45/200; the barrier forms
also add a `adds r0,r7,#0` copy that the ROM does not have.

## Loop-invariant hoisting (keep the ROM's per-iteration recompute)

agbcc's loop optimiser hoists pure subexpressions out of loops even when the
body contains calls. `_08002F68` recomputes `(z << 12)` inside the loop from
the spilled parameter at `[sp,#0]`; writing `((u32)z << 12) | ry` produced a
hoisted `lsls r2,r7,#12 / str r2,[sp,#4]` instead and moved `h2` from `[sp,#4]`
to `[sp,#0]` (31/160 vs the ROM's 156 bytes). Taking `z`'s address (`int *zp =
&z;` then `*zp`) suppresses the hoist but costs the spill shape a different way
(15/164). Same family: `_080013B68`'s `mov r8,r9` / `ldr r4,[pc]` reorder, which
the scheduler fixes independently of the C (11 spellings measured, all 735/740).

## Spans ending in zero padding

A promoted span that ends in `00 00` right after `bx lr` needs
`__asm__(".align 2, 0");` after the body: gas otherwise fills the tail with the
2-byte Thumb NOP (`46 c0`) and the last two bytes never match
(`_080021F14`, `_080022B48`; same trick as `Course_Iter_07CD0`).

## Absolute anchors and raw-region symbol names

Two naming traps that block an otherwise exact body at link time:

* A folded constant that the ROM loads as two pool words needs two *anchors*
  used as symbols, not a literal sum: `0x03001780u + 0x5E4u` folds to one word
  (0x03001D64), while `(uintptr_t)&Base + (uintptr_t)&Off` keeps two. The
  `.globl X\nX = 0x...` assignment must be emitted *inside* the function (a
  file-scope `.set` does not reach the spliced TU); see `WA_SPLIT` in
  `src/race_scene_b1.c`.
* Raw ROM regions carry 9-digit closure names: the routine at VMA 0x080188B0 is
  `sub_0800188B0`/`_0800188B0` (asm/rec35_runtime.s), so the 8-digit
  `sub_080188B0` never resolves. Add `#define sub_080188B0 sub_0800188B0`.
* An asm file whose region ends at a file boundary needs a `<stem>_end:` label
  for the screen to derive `end_marker` (`menu_pkt_end:` in asm/menu_pkt.s).

## Unresolved-call defects (fix these first)

A body whose candidate carries an unresolved `bl` can never be exact, so the
first move on any `UNRESOLVED_RELOCATION` record is to make the call resolve.
Three recurring causes, all found with `--function NAME --c89` (the `call`
lines name the symbol the probe could not place):

* **`static` helper called twice or more.** agbcc `-O2` never inlines it, so the
  candidate emits `bl Helper` where the ROM has the statements inline at every
  site. Rewrite the helper as a macro (`#define WA_PAIR(...) do { ... } while
  (0)`), keeping distinctive temp names (`_wp_off`) so sites cannot collide.
  Examples: `WA_PAIR` in `src/menu_ff78_u.c` (`_0800133E4`, 43 -> 101 matched),
  `_080015A0C`/`_080015AB0` in `src/menu_ff78_i.c`.
* **`static` helper that duplicates a real ROM function.** If the helper's name
  suffix is a VMA (`RaceSpriteNode_27308`), check `tools/xref.py baserom.gba
  0x08027308`: a `bl` caller means the routine already exists in the ROM (and
  usually already has C elsewhere). Delete the duplicate and call the closure
  symbol (`src/race_scene_helpers.c` -> `_08027308`, defined in
  `asm/garage_26f50.s`).
* **Shared tail reached by `b`, not `bl`.** `asm/ai_racefsm.s` reaches
  `_0800AD00`/`_0800AD26` with `b` from cases 5 and 7, so the ROM shares one
  copy of the tail. A `static` that both callers invoke emits `bl` instead;
  the faithful shape is a label plus `goto` (the tail then lays out as its own
  block, exactly as the ROM has it).

Checking the `static`s in a file for a hex name suffix is cheap:

```sh
grep -n '^\s*static.*_[0-9A-Fa-f]\{4,\}\s*(' src/*.c
```

## Bodies that are not agbcc output (do not re-lift)

`build/scan_armcc.py` flags unmatched bodies carrying non-agbcc markers. Two
facts measured against the pinned compiler:

* agbcc always saves `lr` on the entry path when the body calls (`push {r4,
  lr}`, `push {r4, r5, r6, r7, lr}`); it never emits a bare `push {rN}` with a
  `bl` in the body, and never defers the frame past the early exits.
* returns are `bx lr` / `pop {r0}; bx r0` -- never `mov pc, lr` or `pop {pc}`.

0 of 1396 promoted bodies contain `mov pc, lr` (the single exception is a
`__attribute__((naked))` inline-asm leaf, `SoundDivNoop_2DE98`). The four ROM
spans that do are the EABI division runtime, taken from ARM's library, and stay
asm-owned:

| VMA | bytes | asm |
|-----|-------|-----|
| 0x0802DE04 | 148 | `asm/sound_aeabi_idiv.s` |
| 0x0802DE9C | 208 | `asm/sound_aeabi_idiv_core.s` |
| 0x0802DF6C | 120 | `asm/sound_uaeabi_uidiv.s` |
| 0x0802DFE4 | 192 | `asm/sound_divmod.s` |

Same class as `_0802C390` (proved earlier by its `push` lacking the `lr` save).
Their inline core uses the Thumb ALU rotate (`rors r3, r4`), which agbcc does
produce for `(v >> n) | (v << (32 - n))` with a constant `n`, so the rotates are
not the blocker -- the frame/return idioms are.

## Optional matching diagnostics

```sh
python3 tools/match_families.py --build
python3 tools/match_context.py NAME
python3 tools/strategy_router.py --function NAME
```

Family matching compares instruction and branch patterns with consistent
register normalization, while retaining load/store widths. It is a retrieval
heuristic, not semantic-equivalence evidence. Fresh example probes validate
recipes against current source; stale indexes must be rebuilt.

`leaf_synth.py` supports bounded straight-line Thumb leaves and requires
ROM-backed choices for return type and memory qualification. It rejects
unsupported control flow, stack use, calls, and ambiguous live-ins. Its outputs
distinguish `EXACT_DRAFT`, `NO_EXACT_DRAFT`, and `UNSUPPORTED`.
Exit 0 means the search completed, not that it matched. Recheck a draft in its
actual translation unit and independent link before selecting it.

See the [tool index](../tools/README.md) for synthesis, disassembly, asset,
and runtime comparison utilities.
