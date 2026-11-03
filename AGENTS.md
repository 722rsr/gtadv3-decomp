# Agent notes

## Matching work

No remaining body is closed. A finding records what one pass measured, not a
compiler limit. If an earlier note says a shape is impossible, unreachable, or
"walled", treat it as a stale attempt and reopen it with a new lever; re-measure
with `make matching-ready` before repeating or accepting the claim. Keep new
notes to the measured result plus the next concrete lead.

Before lifting an assembly body, run `python3 tools/backlog_census.py
--lift-screen`. A body carrying a lift-screen marker (`tst`, `mov pc, lr`,
`pop {pc}`, a bare `push {rN}` with a `bl`, an `.arm` body, `swi`, or a
fall-through tail) is asm/splice-only under the pinned `old_agbcc`; keep it asm
or lift the rest and splice the marker. That is not a closed body, but the next
lever is a splice or a grouped control-flow conversion, not another C spelling
and not a compiler switch.

## Compiler changes

Do not recommend a compiler patch as a way to close a matching body. The
project's result depends on the pinned `old_agbcc`: a patched compiler that
emits the missing instructions would make the match meaningless as evidence
that the C source reproduces the ROM. Treat the pinned compiler as fixed and
keep searching for the source shape.

`old_agbcc_instr` is a diagnostic build only. It may be used to inspect
allocation and RTL, but its output is never selected source or gate evidence,
and it must not replace `old_agbcc` in a build.

The one exception is a documented discovery: evidence that the ROM was built by
a different compiler, or that the pinned compiler is demonstrably wrong for a
body. That needs concrete evidence, agreement on the scope, and a full corpus
regression. "This one function does not match" is not evidence, and a proposed
compiler change that only helps one body is a red flag.
