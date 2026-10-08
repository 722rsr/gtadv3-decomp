# Contributing

The goal is readable source that builds a byte-identical US version of
*GT Advance 3: Pro Concept Racing*. Matching functions, documenting hardware
behavior, identifying data, and improving build tools are useful contributions.

## Getting started

Fork the repository and work on a branch. Follow the [setup guide](README.md#setup-and-verification)
to install the tools and supply your own US `baserom.gba`. Run `make matching-ready`
before changing game code to establish a working baseline.

Keep each pull request focused. For a large subsystem change, open an issue
first to agree on scope and avoid duplicating work.

## Matching work

Read the [matching workflow](docs/matching_workflow.md) before selecting or
promoting functions. Establish the ABI, access widths, control flow, and span
boundaries from the ROM and reconstructed assembly. Preserve entry aliases and
references into replaced spans.

Use `tools/matching_slice_functions.json` for selected C ownership. An isolated
match or a tool's suggested candidate is not enough to promote a function:
the independent linked slice must remain byte-identical. Do not replace
unmatched code with stubs or original executable bytes to pass a check.

Explain non-obvious compiler constraints beside the source or in a focused
finding. Keep speculative names and interpretations separate from verified
behavior. Do not mix unrelated formatting or renaming into a matching change.

## Before opening a pull request

For changes to source, assembly, headers, build tools, linker scripts, or the
checked-in build inventories:

```sh
make progress
make fast-check
make ci-check
git diff --check
```

`make progress` checks the pinned ROM hash, runs `make matching-ready`, and
refreshes `docs/data/report.json` and `docs/data/report-inputs.json`. Include
both files in the pull request. If assembly ownership changes, follow the
matching guide to refresh the affected inventories before running these gates.
Do not edit build inputs while verification is running.

For documentation-only changes, `make ci-check` and `git diff --check` are
sufficient. Optional search/synthesis changes also need `make experimental-check`.

Describe the affected functions or addresses, what changed, and the commands
and results used to verify it. Include known limitations. Screenshots and runtime
observations can support a change, but do not replace byte verification.

Hosted CI runs ROM-free tool tests and checks the progress snapshot's source
fingerprints. A green CI result does **not** prove that a contributor ran the
full build. Maintainers should review the evidence and rerun the local gates
before merging code changes. See [CI and progress reporting](docs/ci.md).

## Data regions

Follow the [data integration guide](docs/data-integration.md) for the first
working example. Data credit requires a reviewed format, a generator input
consumed by the build, and byte verification of the complete owned range.
Keep extracted editable values and binaries private under `build/`.

## Files and licensing

Do not commit ROMs, extracted assets, save files, compiled binaries, local
compiler trees, or generated build directories. The progress JSON files contain
only names, sizes, and hashes and are intended to be committed.

Contributions to the reconstruction, tooling, and documentation use the
repository's [CC0 dedication](LICENSE). Preserve third-party attribution and
license notices; see [NOTICE](NOTICE). Do not submit material you cannot
contribute under these terms.
