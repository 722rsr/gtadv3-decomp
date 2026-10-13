# Build verification and byte matching

The matching build uses the pinned `old_agbcc` compiler with
`-O2 -mthumb-interwork -ffunction-sections`. See [setup](../README.md),
[compiler behavior](findings/compiler_split.md), and
[completion criteria](decompilation-roadmap.md).

## Verification commands

```sh
make toolchain
make full-rom-match
git diff --check
```

`matching-ready` checks the reference ROM hash, generates compiler-compatible
C copies, checks original/transformed object equivalence, scores the C corpus,
and links the selected C functions into the independent executable slice.
It also runs an instruction-mutation negative control, runtime-provenance
comparison, and source/data ownership checks.
`full-rom-match` then links that C-spliced code object with the registered data
object, emits source-generated zero padding, and compares the complete 8 MiB
image with the pinned reference. Some registered data inputs remain
verbatim/ROM-derived; the gate proves the exact whole-image build, not semantic
asset reconstruction.

Generated source, objects, reports, captures, and extracted assets belong in
ignored output directories. Never commit the reference ROM or derived binaries.
The checked-in inventories contain metadata and hashes, not replacement code.

For a focused comparison:

```sh
python3 tools/corpus_match_probe.py --function NAME --c89 --require-all --require-exact
make fast-check
```

`fast-check` runs tool regressions and static audits. It does not replace the
whole-image `full-rom-match`. On a fresh checkout, that target runs
`matching-ready` first to generate the corpus report and assembled symbol
closure used by these audits.
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

The advisory `instruction_diff` report aligns reachable Thumb instructions and
separates register choices, immediates/memory offsets, branch/call targets, pool
values and non-code bytes. A local insertion can therefore leave a large aligned
tail even while fixed-offset byte counts decrease. Unsupported indirect control
flow is reported as a limitation; identical mnemonic sequences alone do not prove
operand or control-flow agreement. The ranker requires explicit comparison evidence
for its register-only and instruction-order recommendations. These diagnostics
never relax the whole-span byte gate.
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

## Unresolved-relocation misses (cheapest remaining bucket)

The probe's `UNRESOLVED_RELOCATION` class is the cheapest class to clear: the
body may already be byte-correct and only a symbol reference blocks it. Snapshot
the whole corpus and read the class back:

```sh
python3 tools/corpus_match_probe.py --c89 --limit 0 --json build/lab/full-status.json
python3 - <<'PY'
import json
d = json.load(open('build/lab/full-status.json'))
for f in d['results']:
    if f['status'] != 'UNRESOLVED_RELOCATION':
        continue
    syms = sorted({c['symbol'] for c in f['call_targets'] if not c['resolved']} |
                  {p['symbol'] for p in f['pool_words'] if not p['resolved']})
    print(f"{f['name']:20s} {f['vma']} {f['rom_bytes']:4d}B {f['source']:30s} {syms}")
PY
```

Latest full snapshot (2026-10-12, second run): 1400 EXACT of 1668 selected
bodies; classes `identical mnemonics` 1400, `instruction selection` 253,
`unresolved relocation` 13, `instruction order` 2. The class counts move as
bodies are repaired -- two of the original 15 unresolved relocations were
cleared without changing the EXACT count (`_080011958`, `_08005CB4`; both are
now scorable and carry a much higher matched-byte count). Compare two snapshots
by name rather than by class totals:

```sh
python3 - <<'PY'
import json
a = {f['name']: f for f in json.load(open('build/lab/full-status.json'))['results']}
b = {f['name']: f for f in json.load(open('build/lab/full-status2.json'))['results']}
for n, fb in b.items():
    fa = a.get(n)
    if fa and (fa['status'], fa['matched_bytes'], fa['cand_bytes']) != \
              (fb['status'], fb['matched_bytes'], fb['cand_bytes']):
        print(n, fa['status'], fa['matched_bytes'], fa['cand_bytes'],
              '->', fb['status'], fb['matched_bytes'], fb['cand_bytes'])
PY
```

The unresolved relocations split into three fix directions:

* **Libgcc name where the ROM has its own helper.** `src/foundation_math.c`
  `MathHelper_05CB4` declared `extern int __aeabi_idiv(int,int)`, but the ROM's
  two call sites are `bl 0x0802DE04` -- the game's own signed divider listed in
the table above. Pointing the C at `_08002DE04` resolves both relocations;
  the body is still `PARTIAL 13/192` because the ROM re-reads `p`'s two fields
  from the record pointer (`ldr r3,[r6,#0]` then `ldr r5,[r6,#4]`) instead of
  caching them in locals, so the pointer stays live in `r6`. Same class:
  `_0802B888` (`__muldi3`; the ROM span is a 4-byte Thumb interworking veneer
  `add r2,pc,#0 / bx r2` plus a 12-byte ARM `umull r2,r3,r0,r1 / add r0,r3,#0`
  helper, not a libgcc call) and `_0802BC4C` (`memset`; the ROM unrolls four
  `stmia r0!,{r1,r2,r3,r4}` after `mov ip,r4`, which no plain-C aggregate form
  reproduced -- `__builtin_memset` emits a call, struct assignment emits
  `stmia`+`str` triples).
* **`static` helper the ROM inlined.** The body calls a file-local function that
  is a real ROM span only as inlined code, so the symbol has no VMA:
  `_080002C4` (`AgbMain_Run`), `_08000CA0` (`D0_rec` + `CA0_State0..4`),
  `_08008CA0` (`phase_case*`, 2384 B), `_0800AA40` (`race_lap_tail`,
  `race_progress_check`), `_08023628` (`CourseSetup7770_Static`), `_08001758`
  (`SessionEnter*`/`Exit*`/`Mode8Hook`), `_0802B898` (`MixerEnter`). Either
  inline the helper into the caller or accept the miss.
* **TU-local constant where the ROM uses an absolute table.** The pool word is
  a `.rodata`/`.data` section symbol with no ROM VMA: `_080011958`,
  `_0800DD9C`, `_08024338`, `_080263E0`, `_0802D6F4`. Reference the table by its
  absolute address the way `src/foundation_math.c` does for `0x0805DAF0`.
  The most common source is an **aggregate initializer**: `u32 kinds[3] = {8,32,
  56};` makes agbcc emit a `.rodata` copy, while storing the three elements one
  by one emits plain immediates. That one change took `_080011958` from
  `UNRESOLVED_RELOCATION 36/520` to `OVERSIZED 80/508` (the remaining 12 bytes
  are the record pointer being re-read from `r6` instead of cached).

`_0802B898` additionally has a **mis-split span**: the ROM's guard ends
`str r3,[r0,#0]` with no `bx lr`, i.e. its success path falls through into the
next function at 0x0802B8AA, while the manifest's `end_marker` cuts the span at
18 bytes. The body cannot be reproduced until the `asm/sound_mixer.s` markers are
corrected.

Two near-misses keep the `instruction order` class alive and are documented in
place rather than re-attempted: `_080013B68` (735/740; the loop preheader's
`mov r8,r9` precedes the hoisted pool `ldr r4,[pc]` in the ROM -- agbcc's
scheduler hoists the load regardless of C order, 11 variants measured) and
`_080021018` (148/164; the loop-body call site materialises the outgoing stack
constants in `r0` before `mov r0,r9 / adds r0,#8`, while the same C shape at the
odd-branch site matches).

## The volatile-s16 lever (instruction-selection class)

A `volatile s16` lvalue lowers to `ldrh + lsls + asrs`; the ROM's indexed read is
the true `movs rN,#0 / ldrsh rM,[rBase,rN]`, which agbcc only emits for a **plain**
`*(s16 *)`. SW-`volatile` is also why a large work-area offset can never use the
`ldrsh` immediate (it only reaches +62). Flipping one macro in
`src/menu_ff78_a.c` (`WA_S16`) improved every body that reads through it and cost
nothing:

| VMA | before | after |
|-----|--------|-------|
| 0x080101FC | OVERSIZED 29/396 | PARTIAL 39/372 (length-exact) |
| 0x08010370 | OVERSIZED 97/408 | OVERSIZED 97/404 |
| 0x080104F0 | OVERSIZED 38/356 | OVERSIZED 50/352 |
| 0x08010644 | OVERSIZED 35/396 | PARTIAL 41/392 (length-exact) |
| 0x080107CC | OVERSIZED 18/276 | OVERSIZED 43/276 |
| 0x080109D4 | PARTIAL 51/- | PARTIAL 51/672 |

Executable to find the rest of this vein (it reads the per-function work bins, so
rebuild a bin with `match_context.py` before trusting a hit):

```sh
python3 - <<'PY'
import os, re, subprocess
for d in sorted(os.listdir('build/match')):
    w = os.path.join('build/match', d, 'work')
    if not os.path.isdir(w): continue
    refs = [f for f in os.listdir(w) if f.endswith('_context_ref.bin')]
    if not refs: continue
    stem = refs[0][:-len('_context_ref.bin')]
    dis = lambda p: [re.match(r"\s*[0-9a-f]+:\t[0-9a-f ]+\t(.*)", l).group(1).strip()
                     for l in subprocess.run(['arm-none-eabi-objdump', '-D', '-b', 'binary',
                         '-m', 'arm', '-M', 'force-thumb', p], capture_output=True,
                         text=True).stdout.splitlines()
                     if re.match(r"\s*[0-9a-f]+:\t[0-9a-f ]+\t", l)]
    a = dis(os.path.join(w, stem + '_context_ref.bin'))
    b = dis(os.path.join(w, stem + '_context_cand.bin'))
    nref = sum('ldrsh' in t for t in a); nc = sum('ldrsh' in t for t in b)
    sig = sum(1 for i, t in enumerate(b) if t.startswith('ldrh') and i + 2 < len(b)
              and b[i+1].startswith('lsls') and b[i+2].startswith('asrs'))
    if nref > nc and sig: print(d, 'rom_ldrsh=%d cand_ldrsh=%d truncated=%d' % (nref, nc, sig))
PY
```

Two counter-examples worth not repeating: the same flip is a **regression** in
`src/race_dispatch.c` (`WA_S16`, `_0800B190`: 69 -> 67 matched) and
`src/race_scene_a2.c` (`_08001AAA0`: 147 -> 134, `_08001BB14`: 90 -> 88); both are
reverted, with the measurement left in a comment above the macro. The `WA_SPLIT`
two-symbol anchor is likewise a regression on `_0800101FC` (see the comment above
`WA_S16` in `src/menu_ff78_a.c`) even though it is right for `_08001B410` in
`src/race_scene_b1.c`. Measure per body; do not generalise either recipe.

## The jumptable switch lever

A switch the ROM lowers with an `ldr r0,[r0] / mov pc,r0` jump table (the
DD9C pattern: `cmp r0,#27 / bhi default`, `lsls r0,r0,#2`, `ldr r1,[pc]`,
`adds r0,r0,r1`, `ldr r0,[r0,#0]`, `mov pc,r0`, then the table) is
reproducible in plain C, but three details matter and each one was worth a
step on `_0800DD9C` (0 -> 132 -> 176 EXACT):

1. **Store in every arm, not a shared store after the switch.** The ROM
   emits one full copy of the address expression and the store in each arm
   (`adds r1,r4,#0 / adds r1,#84 / movs r0,#5 / b`, then again with
   `movs r0,#1`). Assigning a `u16 out` in each arm and storing once after
   the switch tail-merges both arms (174/176).
2. **Sign-extension order on the index.** The ROM emits `subs #15` FIRST,
   then `lsls/asrs`; writing the read directly as
   `(int)(s16)((int)*(u16 *)ptr - 15)` matches that. `(s16)(u16 - x)
   inverts the order and mis-scores.
3. **The default arm's address lands where the case bodies do, so the
   whole function length is right but the pool table is off by 4 bytes** if
   a `u16 out` variable is left in scope. Removing it drops the extra
   `movs r1,#5 / b / movs r1,#1` merge block and re-spaces the literal pool
   to the ROM layout (174 -> 176).

The tail-anchor idiom also applies to a jumptable function: a zero halfword
at the ROM span end reproduces the literal pool padding the assembler would
otherwise fill with `46 c0`. Same `.pushsection .short 0` form as
`Race_Setup_19200`.

## Two shape levers measured on `_080011958` (496 B)

That body went from `UNRESOLVED_RELOCATION 36/520` to `OVERSIZED 306/500` in
three steps, none of which changed behaviour:

1. **Take the record pointer as the parameter, not as a local copy.**
   `void f(void *rec_) { volatile u8 *rec = (volatile u8 *)rec_; ... }` costs two
   register copies (`adds r6,r0,#0` then `adds r7,r6,#0`) where the ROM has one
   (`adds r7,r0,#0`), and the extra live register then leaks into every `rec +
   off` base in the body. `void f(volatile u8 *rec)` (update the alias
   declarations to match) plus `(void *)rec` at the call sites: 80 -> 205
   matched. Watch for the same double copy wherever a `void *` parameter is
   immediately re-cast.
2. **`switch` where the ROM has a fall-through `cmp/beq` chain.** An
   `if / else if` compiles to `cmp #1 / bne else`, which also strands a second
   `cmp / bne` inside the first arm. The ROM's `cmp r5,#1 / beq body1`,
   `cmp r5,#2 / beq body2`, `b default` is the shape agbcc emits for a
   `switch`. Converting the outer dispatch: 209 -> 276; converting the inner
   `h` dispatch the same way: 276 -> 306.

Everything after 0x76 is register permutation only (same operations, `r0/r2/r3`
vs the ROM's `r3/r4/r1`), so the body is allocation-bound from there.

## Promoting a body whose asm region has no end marker

The screen refuses a body with `no end marker in <region>.s and no <region>_end:
to fall back on`. The fix is the anchor label the other region files already
carry, appended at the end of the region file -- but only when the file declares
exactly one `@ Region:` and contains no `.include` (otherwise the boundary is
ambiguous and the anchor is not safe):

```asm
@ End-of-region anchor for the splicer. ...
code_241c8_end:
```

`asm/code_241c8.s` gained one this way for `0x08024338` (308 B), which then
staged in one pass. Note the screen grades the **compiled** corpus report, not
the working tree: refresh `build/era-corpus/ready-report.json` after *every*
source change, and again after fixing a call-name spelling, or a correct fix
reads as `BLOCKED` on stale data.

## Unresolved-relocation triage order that works

The class went 15 -> 7 bodies in one session with three different fix
patterns, in the order that finds the cheapest first:

1. **Wrong spelling for a real callee** (`_08023628`): the C wrapped the call
   in a file-local static, but the ROM's `bl` lands on a real span (here
   `0x08007770`). Check the ROM's `bl` targets FIRST -- any that match an
   exported function need the closure spelling only, and the score may jump
   straight to OVERSIZED/PARTIAL with no body rewrite.
2. **Helper the ROM inlined** (`_0800AA40`, `_0800DD9C`, `_08000CA0`): if the
   ROM span has no `bl` where the C calls a helper, the helper must become a
   statement-LEVEL macro (`do { ... } while (0)` for void bodies) so the body
   is textually replicated. Statement-expression macros with `goto` labels
   also work but need declared variables that the case body uses.
3. **Genuinely separate functions** (leave alone): when the ROM DOES have the
   `bl` and the helper has a real span of its own, the C is already right and
   the miss is elsewhere.

**A tooling hazard:** in `src/scene_record_dispatch.c`, three
static-helpers-turned-macros were attempted with a regex body-extraction
script. The bodies' nested `if/else` flatten against the `\{(.*?)\n\}\n`
match and the resulting TU does not compile ("`cc` undeclared here"). Only
closing braces at column 0 matched, so the whole file had to be reverted by
`git stash` and the surviving good edits re-applied with `str_replace`. Edit
multi-case bodies with explicit text replacements, not pattern extraction.

A per-function reminder: `build/match/*/work` bins are stale snapshots. Four
bodies that looked unfixed there (`_080017984`, `_080017B00`, `_080016DE4`,
`_08002AA30`) re-measure EXACT -- always re-run the probe before editing.

## Full-inline into one ROM body (measured 2026-10-12)

`_080002C4` (716 B, `foundation_agbmain.c`) sat in UNRESOLVED_RELOCATION
because `static void AgbMain_Run` compiled to a real `bl` into its own
section, and nothing in the fixture's 1669-symbol closure maps
`AgbMain_Run` (the ROM's whole setup+loop is ONE body; only the next
inventory start at 0x08000590 bounds the span). Introspective fact: the
inventory has no known start between 0x080002C4 and 0x08000590 (verified
directly against `rom_functions()`), so NO alias spelling can fix this --
the loop must be inline, per the old rule at the top of this file (a ROM
body whose helper-call sites branch locally).

Recipe (measured with the kit's `CandidateAdapter.evaluate`): splice each
static helper into braces at its call site, recursively (`AgbMain_Run` ->
`AgbMain_PerFrameBody` -> `AgbMain_SceneStateDispatch(0x0F,0x10)` twice,
`AgbMain_SoftReset`, `AgbMain_Teardown`), excise the five definitions,
and spell the BIOS `VBlankIntrWait` as the closure's `sub_0802D9B0`
(binding `VBlankIntrWait` fails -- the pocket leaf `_080003F4` is
`VBlankIntrWait_Pocket`, a different symbol). Result: **UNRESOLVED →
PARTIAL 73/716, zero unresolved relocations, class instruction-selection**;
corpus-wide diff confirmed exactly one entry moved and `make matching-slice`
kept both PASS lines. Remaining delta: the ROM stages `r7←sl, r6←r9,
r5←r8` and pushes `{r6,r7}` where the candidate pushes `{r5,r6,r7}` -- a
register-frame grind, same family as `_0800AA40`.

## `_080021018` prepared battery re-measured (2026-10-12)

The 13 prepared case variants (`build/cases_21018c.py`) were run through
the kit (`transform_file` -> `evaluate`). All 12 compilable ones confirm
the source comment: the statement-expression family stays at 148/164,
the plain-`rec+8` family collapses the loop prologue to 74-76/164 (the
r0 pin is load-bearing), none EXACT. X4 (`pin nine=r0`) fails the
transform helper itself (`six` undeclared -- a `_x()` substitution bug
in the prepared file, not a compiler verdict), so it remains unmeasured.
The 8 missing bytes are the ROM's stack-arg stores using r0 as scratch
BEFORE `mov r0,r9 / adds r0,#8`, with r1-scratch after -- the scheduler
never inverts that order in the prepared shapes.

## Tiny sound bodies (still unresolved, measured in work-dir)

- `_0802BC4C` (24 B): ROM is a fully-inline 16-word zero
  (`movs r1..r4,#0` + 4x `stmia r0!,{r1-r4}` fenced by `mov ip,r4` /
  `mov r4,ip`); the C's `__builtin_memset` emits `bl memset`, which the
  closure cannot bind. Byte search found NO other occurrence of that
  shape in ROM, so there is no existing EXACT body to mirror -- needs a
  hand-crafted idiom (struct-copy or asm-shaped leaf).
- `_0802B888` (16 B): ROM is a thumb->ARM veneer (`add r2,pc,#0; bx r2`)
  wrapping the 64-bit multiply; the C reaches for `bl __muldi3`.
- `_0802B898` (18 B): ROM is `MixerThumb` wrapper; candidate reaches
  `MixerEnter` (unresolved) + two `_call_via_r1` gadget calls.

These three remain the documented mixed-ISA/veneer class.

See the [tool index](../tools/README.md) for synthesis, disassembly, asset,
and runtime comparison utilities.
