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
full `matching-ready` gate, and writes:

- `docs/data/report.json`: an [objdiff v2 report](https://github.com/encounter/objdiff/blob/main/objdiff-core/protos/report.proto).
- `docs/data/report-inputs.json`: source/build-input hashes and the report hash.

CI compares those hashes against the checkout and independently checks that
reported selected spans agree with the manifest. Additions, deletions, and
changes to build inputs require refreshing the snapshot. Documentation outside
the build inventories does not. Fingerprints prevent accidental stale reports;
they are not a signed attestation or a substitute for maintainer build review.

Code progress counts only selected C-owned spans that passed the independent
link, including any padding within those spans. Its conservative denominator
is the entire 188,760-byte executable slice, including startup, header, literal
pools, and alignment. Reconstructed assembly alone receives no C-match credit.
The data denominator is 8,199,848 bytes, including trailing ROM padding.
Verified generated regions from `tools/data_regions.json` receive data credit
only when the ownership audit records the same registered range and generator
input. The first region covers [280 palette bytes](data-integration.md); all
remaining reference-backed data and padding stay unmatched. This is not a claim that the full independent C ROM is complete.

The report uses the audited ownership map to group the entire executable slice
by assembly source region. Every byte contributes to its region's area, including
unselected code. The `fuzzy_match_percent` field used by decomp.dev for color is
a conservative byte-weighted score: verified selected spans contribute 100%,
and all other bytes contribute zero. It equals the C-owned byte percentage;
it does not estimate instruction similarity for unmatched functions.

Drill-down lists include selected functions and explicitly named unselected byte
ranges so their areas still cover the whole region. These ranges may include
multiple functions, literal pools, header bytes, or padding; they are not a
function census. Aggregate function counts remain omitted. The report does not
claim that all selected bodies are free of inline assembly. The existing `objdiff.json`
is a placeholder with an empty `units` list, not a configured comparison
workspace or the source of these metrics.

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
