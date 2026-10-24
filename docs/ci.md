# CI and progress reporting

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

The report keeps decompilation and completion/linkage separate, following
Objdiff's distinct `matched_*` and `complete_*` measures. The README's
generated progress block quotes the current figures.

- `matched_functions` and `matched_code` count only functions decompiled to C:
  the `c` and `c_fakematch` classes in `docs/data/source_quality.json`, which
  `tools/source_quality.py` produces. Naked or asm-dominated bodies (`asm`) and
  C with spliced instructions (`partial`) match byte-for-byte but are reported
  at 0%, as other projects report `INCLUDE_ASM`/`NAKED` placeholders.
  Non-function bytes (header, startup, pools) are never matched code.
- `total_functions` is the whole `.type`/`BL` inventory, including identified
  functions that have not been selected for C at all.
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

The `.type`/`BL` inventory is cross-checked by `span_audit.py`: every
push-with-link-register pattern is accounted for, and no unexplained prologue,
shape-only entry, or swallowed entry remains. The census proves the inventory
is complete; it says nothing about how much of it is decompiled.
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

## recomp.fyi

The default-branch workflow uploads the objdiff v2 report as the `us_report`
artifact. [recomp.fyi](https://recomp.fyi/game/gt-advance-3-pro-concept-racing-gba)
reads this report for the project's matched-code progress. The root
[`.recomp.json`](../.recomp.json) records the game, platform, release, project
name, and current status; the board reads it daily. See the
[manifest specification](https://recomp.fyi/spec) for the available fields.
