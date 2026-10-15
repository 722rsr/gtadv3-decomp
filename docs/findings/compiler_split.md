# Compiler framing and register allocation

The matching tools use `build/toolchains/agbcc/old_agbcc` with
`-O2 -mthumb-interwork -ffunction-sections`. Both `old_agbcc` and the comparison
`agbcc` binary come from the pinned `pret/agbcc` build supplied by
`make toolchain`.

`src/runtime_memset.c` is the one translation-unit exception: its ROM body ends
with `pop {r4,r5,pc}`, which `old_agbcc` emits with `-mno-thumb-interwork`.
Keeping that ABI choice isolated preserves the default interworking behavior
for every other source file. The explicit `.align 2, 0` after the function
reproduces its zero halfword before the next ROM symbol. The shared profile is
defined in `tools/agbcc_options.py`; the assembly boundary is in
`asm/runtime_mem.s`.

## Conditional leaves

For a leaf with conditional control flow, the comparison `agbcc` binary adds
a link-register frame even when the body makes no call. `old_agbcc` can emit
the frameless `bx lr` form present in the ROM.

```c
int f(unsigned a) { int r = 0; if (a == 8u || a == 11u) r = 1; return r; }
```

`old_agbcc` emits the ROM's 16-byte body at VMA `0x08001744`
(file offset `0x001744`):

```text
2100 2808 d001 280b d100 2101 1c08 4770
```

The comparison binary emits a `push {lr}` and an interworking pop/branch
epilogue. Changing source expressions cannot remove a frame imposed by this
compiler behavior. Interworking is still required for the ROM's non-leaf
epilogues; disabling it is not a substitute for the correct compiler.

## Thumb frames and interworking returns

`old_agbcc` saves `lr` whenever a function makes a call or saves a low
callee-saved register. It cannot produce a non-leaf `mov ip, lr` / `bx ip`
frame or a push of low registers without `lr`.

With `-mthumb-interwork`, the epilogue scratch register follows the declared
return width: void and structure returns use `r0`, returns up to four bytes use
`r1`, returns up to eight bytes use `r2`, and wider returns use `r3`. Check
caller behavior before changing a declaration to match an epilogue.

## Source constraints

| ROM VMA | Source | Constraint |
| --- | --- | --- |
| `0x08001744` | `src/state_block_a.c` | Word-width parameter; a `u16` parameter adds narrowing absent from the ROM. |
| `0x08001FD0` | `src/runtime_accessors.c` | Three-case switch preserves the individual comparisons. |
| `0x080014A4` | `src/foundation_fa0.c` | Positive early return and in-section `.align 2, 0`. |
| `0x08002140` | `src/block_b.c` | Word accumulator and return, signed halfword lvalue for `ldrsh`. |
| `0x08007EC4` | `src/course_records.c` | Volatile field load preserves load order; an `r0` accumulator pin preserves add operand order. |

Local register pins are GNU extensions. Their comments must identify the ROM
register and instruction that requires the constraint. They are not ordinary
C89 syntax, even though the source transform supports their use.

## Artifact validity

The slice linker can reuse compiled objects from
`build/era-corpus/ready-work/` when the accompanying report is fresh.
Comparisons using a different compiler must regenerate those objects and the
report. Changing only a diagnostic script's compiler variable does not prove
the reused objects were built with that compiler.

Use `make matching-ready` for the configured compiler and independent-link
check. The [runtime provenance comparison](../compiler_status.md) is separate
from the framing behavior of these C compilers.
