# agbcc recipe transfer probes

These small, original C89 pairs exercise motifs described in the
[recipe cookbook](../../../docs/findings/agbcc_matching_recipes.md).
They contain no extracted game source and do not establish ROM matches.
Each file is a separate translation unit: compile A and B separately.

From the repository root, with the normal matching toolchain available:

```sh
(
set -eu
for recipe in shift packed barrier constant; do
    out="build/experiments/agbcc-recipes/$recipe"
    python3 tools/compiler_microscope.py \
        "tools/fixtures/agbcc_recipes/${recipe}_a.c" \
        "tools/fixtures/agbcc_recipes/${recipe}_b.c" \
        --passes a --out "$out"
    for variant in a b; do
        arm-none-eabi-as -mcpu=arm7tdmi \
            -o "$out/$variant.o" "$out/probes/$variant.s"
        arm-none-eabi-objcopy -O binary --only-section=.text.candidate \
            "$out/$variant.o" "$out/$variant.bin"
    done
    if cmp "$out/a.bin" "$out/b.bin"; then
        echo "$recipe: identical function bytes"
    else
        echo "$recipe: function bytes differ"
    fi
    arm-none-eabi-objdump -r "$out/a.o" "$out/b.o"
done
)
```

Use a fresh output directory after changing a fixture or compiler. On failure,
stop and diagnose before comparing output. `cmp` compares only function-section
bytes; also compare relocation offset, type and target in the printed tables.
The cookbook records the initial results. These are observations, not golden
tests that require future compilers to produce the same output.

For `barrier`, inspect the raw `probes/dumps/b/` files for
`NOTE_INSN_LOOP_BEG` / `NOTE_INSN_LOOP_END`; identical output bytes do not imply
identical intermediate representations. `packed` and `constant` deliberately
include an unresolved `consume` call. This is sufficient to compare codegen
and relocation tuples, but is not a linked program or a runtime-equivalence
test. The right shift of signed `int` in `barrier` uses the pinned compiler's
arithmetic-shift behavior in both variants.
