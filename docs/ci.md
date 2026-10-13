# CI and decomp.dev

## Hosted checks

`.github/workflows/ci.yml` runs one Ubuntu job for pull requests, pushes to
`main`, and manual dispatches. It runs `make ci-check` with the runner's Python;
there are no dependency downloads, compiler builds, ROM secrets, caches,
scheduled jobs, or build matrices. The job has a five-minute timeout and cancels
superseded runs for the same branch or pull request. Feature-branch pushes do
not also run a duplicate push workflow.

Documentation changes still run this small job so a required check does not
remain pending because of path filters. No artifact is uploaded for PRs.
Successful default-branch runs upload only `report.json`, named `us_report`,
with seven-day retention. Manual dispatch on `main` republishes an expired
artifact when necessary.

Standard GitHub-hosted runners are [free for public repositories](https://docs.github.com/en/billing/concepts/product-billing/github-actions).
Private repositories consume the owner's allowance. The five-minute job limit
bounds each run, not aggregate monthly use. Repository owners can also configure
an Actions spending budget in GitHub billing.

## Progress measurements

`make progress` verifies the private US ROM against `baserom.sha256`, runs the
full `full-rom-match` gate (which includes `matching-ready`), and writes:

- `docs/data/report.json`: an [objdiff v2 report](https://github.com/encounter/objdiff/blob/main/objdiff-core/protos/report.proto).
- `docs/data/report-inputs.json`: source/build-input hashes, the report hash,
  and the local verification gates passed when the snapshot was refreshed.

CI compares those hashes against the checkout and independently checks that
reported selected spans agree with the manifest. Additions, deletions, and
changes to build inputs require refreshing the snapshot. Documentation outside
the build inventories does not. Fingerprints prevent accidental stale reports;
they are not a signed attestation or a substitute for maintainer build review.

The report keeps exact matching and completion/linkage separate, following
Objdiff's distinct `matched_*` and `complete_*` measures. For the current
snapshot, the identified C candidates are **1,674 / 1,674 exact (100%)**, and
their selected spans total **188,136 / 188,136 exact bytes (100%)**. These are
the C-function progress figures to headline.

- `matched_functions` counts exact C function candidates in the identified
  `.type`/`BL`-target inventory. `matched_code` includes those exact function
  spans plus the source-owned non-function ranges verified by the independent
  C/ASM slice; therefore the current Objdiff `matched_code_percent` is 100%.
- The separate **99.67% diagnostic** is 188,136 selected C-function bytes /
  188,760 bytes in the code slice. Its denominator includes the cartridge
  header, startup, assembly, and non-function bytes, so it is not the C-function
  match rate or overall decompilation progress.
- `complete_code` and `complete_units` mean the entire C/ASM executable slice
  linked and compared byte-for-byte with no `.incbin` or external code boundary.
  This is the code-slice link measure; the separate `full_hybrid_rom_link`
  gate proves the complete 8 MiB C/ASM plus registered-data image.
- `total_data` covers non-padding content only, ending at the audited content end
  in `docs/data/code_data_ownership.json`. `matched_data` counts registered data
  regions whose editable inputs regenerate the exact bytes and whose ownership
  audit agrees. Trailing zero padding is excluded from this denominator.
- `complete_data` records verified content data linked by the reference build.
  It does not claim that compressed payloads or all asset structures have been
  semantically decoded. Raw payloads preserved verbatim are byte-reproducible,
  not semantically reconstructed.

The `.type`/`BL` inventory is cross-checked by `span_audit.py`: all 1,249
push-with-link-register patterns are accounted for, all 1,674 spans are checked,
and no unexplained prologue, shape-only entry, or swallowed entry remains.
Matching counts byte-exact source owners, including documented assembly and
inline assembly; it does not score source readability or semantic completeness.
Report units are ROM ownership groups and generated data regions, not compiler
translation units.

The reference assembly/data ROM build, independent executable C/ASM slice, and
whole-image C/ASM plus registered-data link are proven. Some registered data
inputs are still verbatim or extracted from the reference ROM, so this byte
completion does not claim semantic decoding of every asset. Runtime equivalence
is not run; it is separate evidence and is not needed to establish an exact
full-image match.
Treemap sections represent non-function bytes rather than invented function
boundaries. `objdiff.json` remains the per-function comparison viewer; regenerate
it with `make objdiff-config` when function names, owners, or boundaries change.

## One-time decomp.dev activation

The [official integration guide](https://decomp.wiki/tools/decomp-dev) requires
a report artifact on the default branch before registration.

1. Push the migrated repository and this workflow to `722rsr/gtadv3-decomp` on
   `main`, then confirm that CI passes and exposes `us_report/report.json`.
2. Sign in to [decomp.dev](https://decomp.dev/manage/new) with a GitHub account
   that has administrator access to the repository and add the project.
   Use the repository URL, the game title, Game Boy Advance platform, and
   version `us` when configuring the project.
3. Confirm the first report appears and its code/data totals match the local
   JSON. Add dashboard badges only after the project is registered.
4. Optionally install the decomp.dev GitHub app for this repository. It receives
   workflow completion notifications; without it, decomp.dev polls periodically.

No decomp.dev token is needed in Actions. This workflow publishes progress only
on `main`; it does not supply PR progress reports or enable PR bot comments.
Registration and the first hosted run must be checked separately from local
validation.
