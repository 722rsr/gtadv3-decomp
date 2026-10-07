# Candidate length and instruction differences

The corpus probe reports `OVERSIZED` when a candidate is longer than its ROM
span. This status says nothing about instruction similarity. A candidate
four bytes too long can differ throughout its body.

## Interpreting the comparison

The relevant measurements are distinct:

- Candidate length determines whether the body can fit its owning span.
- `prefix` and `first_diff` locate the first byte mismatch.
- `matched_bytes` counts equal bytes across the span, including unrelated
  offsets that happen to agree.
- A relocation-aware instruction comparison distinguishes extra operations,
  register choices, instruction order, and pool placement.

Check the span itself before changing source. A missing function entry can
make the comparison include multiple ROM functions. The probe's `alias_of`
field is not an ownership defect: it can simply connect a VMA alias to its
canonical C body. Manifest ownership is keyed by VMA.

## Causes to inspect

A length difference may come from an unnecessary call argument, repeated
constant materialization, an uninlined helper, incorrect memory qualification,
an ABI width mismatch, a frame, or a literal-pool difference. Establish the
ROM's actual argument registers and memory operations before removing any
candidate instruction.

For example, a volatile signed halfword read may require three instructions
where a non-volatile read can use `ldrsh`. This is a qualification question,
not permission to remove volatility from hardware registers. Likewise, a
compiler that frames conditional leaves cannot reproduce a frameless leaf
merely by changing expression spelling.

Length buckets are not interchangeable diagnostic classes. A cause observed
at +4 bytes does not explain candidates at +8 or +12. Compare actual
instructions and use a controlled change that tests the suspected cause.

## Reproduction

```sh
python3 tools/corpus_match_probe.py --function NAME --c89 \
    --work-dir build/scratch/length-check --json build/scratch/length-check.json
```

Read the probe's emitted assembly and relocation-resolved comparison.
Generated candidate binaries from unrelated or stale runs are not evidence.
See [build verification](../matching_workflow.md) for independent-link checks.
