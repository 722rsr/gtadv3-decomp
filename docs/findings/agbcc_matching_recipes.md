# Transferring newer agbcc matching recipes

This cookbook adapts selected observations from FacundoMainere's
[GBA matching decomp skill](https://github.com/FacundoMainere/gba-matching-decomp-skill/blob/cf9978add0b38d7b7040ef2a4090694c39cfd873/SKILL.md),
specifically its [matching tips](https://github.com/FacundoMainere/gba-matching-decomp-skill/blob/cf9978add0b38d7b7040ef2a4090694c39cfd873/reference/matching_tips.md)
and [strategy notes](https://github.com/FacundoMainere/gba-matching-decomp-skill/blob/cf9978add0b38d7b7040ef2a4090694c39cfd873/reference/STRATEGY.md).
Pinned revision: `cf9978add0b38d7b7040ef2a4090694c39cfd873`.
Adapted material is MIT licensed, copyright (c) 2026 FacundoMainere;
see the retained [license](../third_party/gba-matching-decomp-skill/LICENSE).

Those observations come from Medabots AX. Here they are experiments to try
against GT Advance 3's pinned `old_agbcc`, not evidence of original source or
newly matched functions. Keep the compiler choice and production flags in
[compiler_split.md](compiler_split.md). Do not feed these unverified motifs
into `recipe_miner.py` as verified templates: that tool requires multiple
freshly exact ROM examples.

## Choose an experiment from the mismatch

| Mismatch | Upstream tip | Controlled source experiment | Conditions and limits |
| --- | --- | --- | --- |
| Missing or misplaced sign extension near a call | T135 | Put an intermediate in a full-width local, or compare a separate assignment with an assignment expression inside the argument. | Keep the ABI parameter type fixed. Prove equivalence of conversions and evaluation order; avoid signed overflow. An explicit cast alone may be narrowed away. |
| A byte/halfword value loses a later re-extension | T313, T344, T414 | Compare a scalar local with a fully initialized packed single-field local; inspect whether RTL retains a QI/HI pseudo. | This can change stack use and alignment. Use a scratch local, not an invented packed view of game memory. For halfwords, test the required alignment explicitly. |
| Truncation is correct but shifts/ORs use the wrong registers | T421, T426, T427 | Compare `x <<= 3; result = x | mask;` for a narrow unsigned local with a wide shift inside the expression. | Equivalent only where the observable result has the same narrowing. A wide return can make these different programs. Check shift counts and promotion rules. |
| Correct operations, wrong two-address destination | T422, T429 | Insert `do { } while (0);` between the defining operation and its next use; inspect regmove input/output. | Upstream reports loop notes can stop a regmove scan. Notes are reproduced locally; allocation improvement is not. Try removal as the negative control. |
| Constant rematerialized instead of kept across a call | T347, T412, T420, T423 | Wrap the assignment in `do { value = constant; } while (0);`; compare a legitimate second assignment where the algorithm needs one. | Check ref counts, live length and `REG_EQUIV` in raw dumps. Do not add uninitialized reads or assume an empty loop forces a register. |
| Register-only mismatch after an unsigned halfword load | T339 | Try `value &= 0xffffu;` immediately after a known zero-extending load. | Upstream reports a removed operation can still affect allocator bookkeeping. Only redundant when the value really is bounded; compare RTL and final bytes. |
| Load/address evaluation order differs | T13, T24, T155, T208 | Compare a proven struct member access with the current pointer arithmetic and separate address calculation from the load. | Recover the actual layout and aliases first. Do not invent incompatible declarations or add volatile to ordinary RAM merely to constrain scheduling. |
| Indirect-call setup or copies differ | T22, T311, T318 | Compare a correctly typed function-pointer call with the already proven call-through-register veneer representation. | Preserve the real ABI, arguments and return type. Check relocations and linked call targets, not just opcode shape. |
| Loop exit or common-tail shape differs | T10, T53, T342, T428 | Compare equivalent `while`/`goto` forms, a shared tail versus duplicated statements, or moved loop initialization. | Preserve side effects and zero-iteration behavior. Follow the transformation through reload and final jump cleanup: an early difference can disappear later. |

These recipes complement existing local register pins and empty read/write asm
constraints; they do not establish that such constraints are necessary or
unnecessary. A finite failed search is not proof that C cannot express a match.

## Diagnose before adding permutations

1. Check the function boundary and ABI from callers and callees. T308/T333
   describe an apparently spare register actually carrying another argument.
   T40 warns that a Thumb `bl` can be an internal long jump. Neither observation
   alone justifies changing a boundary or signature.
2. Save the full-TU baseline and choose one transformation. Use
   `compiler_microscope.py` to compare RTL; inspect raw dumps as well as the
   normalized report, which can omit bookkeeping details.
3. For allocator differences, inspect `.greg` references, live lengths,
   conflicts and preferences, then local allocation/reload output. Final
   register differences can originate much earlier than reload. Do not infer
   causation from the first different dump alone.
4. Record the tip ID, exact variant, compiler hash/flags, full-body byte score,
   and whether removing the change reverses the effect. Keep failed attempts
   in the experiment record so the next search can avoid or invert them.
5. Recompile the real full TU and run the existing exact-match, promotion and
   independent-link gates before claiming a game-function match. A microscope
   result or permuter score is diagnostic evidence only.

If raw dumps cannot explain a persistent allocator discrepancy, upstream's
next step is a separate diagnostic compiler instrumenting `find_reg` in
`global.c` and `allocate_reload_reg` in `reload1.c`. Keep any such compiler
separate from the pinned production executable and verify unchanged output;
no instrumented compiler is introduced here.

## Local transfer probes (2026-10-08)

The original, small C89 fixtures in
[`tools/fixtures/agbcc_recipes`](../../tools/fixtures/agbcc_recipes/README.md)
exercise four motifs with the existing microscope. All eight variants compiled
and assembled with `-O2 -mthumb-interwork -ffunction-sections`.
The tested `old_agbcc` executable SHA-256 was
`a4469f0d563a3e47104b4c9d2c614e1b4db535e719788fb5de9ac3ca92e6fe53`.

| Pair | Function section size A / B | Function bytes and relocation tuples |
| --- | --- | --- |
| Narrow shift before OR | 12 / 12 | Identical |
| Scalar byte versus packed local | 20 / 20 | Identical |
| Empty loop between definition and use | 12 / 12 | Identical |
| Constant assignment inside a one-shot loop | 12 / 12 | Identical |

The empty-loop variant does emit `NOTE_INSN_LOOP_BEG` and
`NOTE_INSN_LOOP_END`, including in the local-allocation dump. That confirms
the syntactic mechanism survives this compiler, but does not reproduce the
upstream register-allocation benefit. These low-pressure fixtures are useful
negative results, not a rejection of the motifs in a larger function. No ROM
matching improvement is claimed. Raw output lives under the ignored
`build/experiments/agbcc-recipes/` directory and can be regenerated below.

## Deliberately excluded from the transfer

- The upstream project's compiler-wide `-fprologue-bugfix` choice, padding
  conventions, and progress accounting: those need this game's own evidence.
- Uninitialized-value tricks, empty non-void returns, and conflicting extern
  types to influence compilation. They do not preserve a defensible C program.
- Blanket claims that pointer-type changes cannot affect code generation;
  alias analysis and alignment can matter.
- The upstream autonomous agent, automatic commit/push, and integration
  workflow. This is a recipe reference, not an installed automation policy.
