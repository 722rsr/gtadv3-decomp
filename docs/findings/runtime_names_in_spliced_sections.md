# Runtime calls in section splices

The matching splicer links one compiled function section at a time. Compiler
runtime calls such as `__divsi3` therefore need to resolve to the corresponding
closure label within that section.

`match_c_slice` retargets recognized runtime call operands using the shared
`RUNTIME_VMAS` table. Do not add a C alias, `--defsym`, or `.set` for these
calls: aliases in other sections are absent from the splice, absolute symbols
can create an interworking veneer, and section definitions can collide with
closure symbols.
