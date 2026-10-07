# GT Advance 3 reference and independent-slice builds.
# Requires ARM binutils on PATH; matching also needs the pinned old_agbcc.

AS      := arm-none-eabi-as
LD      := arm-none-eabi-ld
OBJCOPY := arm-none-eabi-objcopy
SHA     := shasum -a 256

BUILD   := build
TARGET  := $(BUILD)/gtadv3.gba

ASFLAGS := -mcpu=arm7tdmi -Iasm

.PHONY: all verify clean distclean toolchain
all: verify

$(BUILD):
	@mkdir -p $(BUILD)

# Both code and data includes are inputs to the reference object.
ASM_SOURCES := $(wildcard asm/*.s) $(wildcard asm/*.inc) $(wildcard asm/macros/*.inc)

$(BUILD)/rom.o: asm/rom.s $(ASM_SOURCES) baserom.gba | $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/rom.elf: $(BUILD)/rom.o ldscript.ld
	$(LD) -T ldscript.ld -Map $(BUILD)/rom.map $(BUILD)/rom.o -o $@

$(TARGET): $(BUILD)/rom.elf
	$(OBJCOPY) -O binary $< $@

# Depend on the output so parallel make cannot verify before linking.
# Check the pinned US ROM first; missing files and command failures are fatal.
verify: $(TARGET)
	@$(SHA) -c baserom.sha256
	@cmp baserom.gba $(TARGET)
	@echo "OK: $(TARGET) is byte-identical to the pinned US ROM"

# ---------------------------------------------------------------------------
# Independently assembled objects for the decomp link.
#
# code.o is the same executable-region top level the reference `make` build
# and the independent slice consume (asm/code.s); data.o is the cataloged
# data/asset tail at its real address.  Neither copies reference code bytes,
# and neither is build/rom.o.
# ---------------------------------------------------------------------------

CODEBUILD := build-code


.PHONY: code-objects
code-objects: $(CODEBUILD)/code.o $(CODEBUILD)/data.o

$(CODEBUILD):
	@mkdir -p $(CODEBUILD)

$(CODEBUILD)/code.o: asm/code.s $(ASM_SOURCES) | $(CODEBUILD)
	$(AS) $(ASFLAGS) asm/code.s -o $@

$(CODEBUILD)/data.o: asm/data_tail.s baserom.gba | $(CODEBUILD)
	$(AS) $(ASFLAGS) asm/data_tail.s -o $@

# Fast static audits and tool regression checks.
# The independent byte comparison is provided by matching-slice/matching-ready.
.PHONY: fast-check
fast-check:
	@test -f build/era-corpus/ready-report.json -a -f build-code/code.o || { \
	    echo "fast-check: missing corpus report or symbol closure; run make toolchain && make matching-ready first" >&2; exit 1; }
	@python3 tools/apple_decls.py --self-test
	@python3 tools/apple_decls.py
	@python3 tools/promotion_screen.py --self-test
	@python3 tools/match_c_slice.py --self-test
	@python3 tools/corpus_match_probe.py --self-test
	@python3 tools/lift_scout.py --self-test
	@python3 tools/test_matching_assist.py
	@python3 tools/export_audit.py --self-test
	@python3 tools/export_audit.py
	@python3 tools/call_audit.py --self-test
	@python3 tools/call_audit.py
	@python3 tools/label_census.py --self-test
	@python3 tools/label_census.py
	@python3 tools/span_audit.py --self-test
	@python3 tools/span_audit.py

# Optional local verification before recording changes.
.PHONY: experimental-check
experimental-check:
	@python3 tools/test_experiment_strategies.py

.PHONY: pre-commit
pre-commit:
	@$(MAKE) fast-check
	@echo "pre-commit: fast-check PASS -- static audits and tool regressions passed; full build not run"

# ---------------------------------------------------------------------------
# Runtime evidence (future independent-ROM gate): drive baserom.gba and the
# independent build through identical scripted input and compare IWRAM +
# EWRAM + VRAM + PAL + OAM with tools/ramwatch.py + tools/ramdiff.py.
# Needs the libmgba Python bindings: run tools/build_mgba_python.sh once.
# There is currently no Makefile wrapper; invoke the tools directly with
# inputs.csv. Byte identity against baserom.sha256 remains the final gate.
# ---------------------------------------------------------------------------

# Runtime provenance: the ROM links a toolchain's hand-written compiler runtime
# (the 32-bit division cores and the interworking call veneers).  Those routines
# are not compiler output, so matching them byte-for-byte identifies the build's
# toolchain without any source reconstruction.  Needs two runtime files: the
# pret/agbcc one (`make toolchain`) and the vanilla GCC 2.95.3 baseline, vendored
# at tools/third_party/gcc-2.95.3/lib1thumb.asm so the target needs no GCC
# checkout.  A missing file is reported SKIP and still fails the target, because
# an incomplete comparison is not a clean one -- not because the routine
# mismatched.
.PHONY: era-runtime-probe
era-runtime-probe:
	@python3 tools/era_runtime_probe.py --self-test
	@python3 tools/era_runtime_probe.py --json build/era-corpus/report.json
	@cmp build/era-corpus/report.json docs/data/era_runtime_probe.json

# Full-ROM source/data ownership inventory.  The audit verifies that the
# checked-in map still describes every byte and every raw .incbin span.
# ownership-check is intentionally strict: it must fail while executable
# bytes or a reference-build artifact remain in the independent-link path.
.PHONY: independent-slice independent-audit
# Independent link slice: the entire executable region (header, crt0/IntrMain,
# AgbMain and every reconstructed engine region) assembled from reconstructed
# source only, with no build/rom.o and no .incbin.  The build must come out
# byte-identical to baserom.gba over 0x000000-0x02E158.
# `independent-slice` refreshes the report; `independent-audit` is the gate.
# Rebuild the assembled closure before deriving source hashes and boundaries.
independent-slice: code-objects
	@python3 tools/independent_slice.py --write

independent-audit:
	@python3 tools/independent_slice.py --self-test
	@python3 tools/independent_slice.py --check

.PHONY: ownership-map ownership-audit ownership-check
# The ownership report describes the current assembled closure.
ownership-map: code-objects
	@python3 tools/ownership_map.py --write

ownership-audit:
	@python3 tools/ownership_map.py --self-test
	@python3 tools/ownership_map.py --check

# Refresh source hashes before checking the pinned ownership inventory.
ownership-check: ownership-map
	@python3 tools/ownership_map.py --self-test
	@python3 tools/ownership_map.py --check --strict

# Repeatable starting gate for matching C source. All generated copies, reports,
# and temporary C-owned assembly remain under ignored build/.
.PHONY: matching-ready matching-slice
matching-slice: code-objects
	@python3 tools/match_c_slice.py --all
	@python3 tools/match_c_slice.py _080028250 --negative-control

# Resolve probe symbols from a freshly assembled code object.
matching-ready: all code-objects
	@python3 tools/agbcc_c89_transform.py --self-test
	@python3 tools/agbcc_c89_transform.py --out build/era-corpus/c89/generated --json build/era-corpus/c89/transform_report.json
	@python3 tools/c89_equivalence.py --json build/era-corpus/c89/equivalence_report.json
	@python3 tools/corpus_match_probe.py --self-test
	@python3 tools/corpus_match_probe.py --c89 --require-all --work-dir build/era-corpus/ready-work --json build/era-corpus/ready-report.json > build/era-corpus/ready-summary.txt
	@python3 tools/promotion_screen.py --self-test
	@python3 tools/match_c_slice.py --self-test
	@$(MAKE) matching-slice
	@$(MAKE) era-runtime-probe independent-audit ownership-check
	@echo "matching-ready: PASS (reports in build/era-corpus; C-owned slice in build/matching-slice)"

.PHONY: decomp-tools
decomp-tools:
	@tools/build_gbafix.sh
	@tools/build_gbagfx.sh
	@tools/build_asm_differ.sh
	@tools/build_permuter.sh

# Provision the matching compiler. Load-bearing for every matching target
# (`matching-ready`, the corpus/era probes); `make` does not
# need it. Serial by design -- see tools/build_agbcc.sh.
.PHONY: toolchain
toolchain:
	@tools/build_agbcc.sh

# Triage the unpromoted backlog into a ranked, dispatchable work queue.
# Discovery only: it grants no promotion authority, and the independent link
# stays the only authority. Reads the corpus report matching-ready refreshes
# plus baserom prologue/epilogue bytes and the asm typed-entry set.
.PHONY: lift-scout
lift-scout:
	@python3 tools/lift_scout.py --self-test
	@python3 tools/lift_scout.py --json build/lift-scout/queue.json

.PHONY: treemap
treemap:
	@python3 tools/treemap.py --mode independent --out build/treemap/treemap_independent.png --html build/treemap/independent.html
	@python3 tools/treemap.py --mode subsystem --out build/treemap/treemap_subsystem.png --html build/treemap/subsystem.html

# `rm -rf build` would take build/toolchains/ with it: the cloned, built agbcc
# tree the matching tools read (a ~10 minute rebuild), plus the optional era
# toolchains. Everything else under build/ is a regenerable output, so `clean`
# keeps the toolchains and `distclean` is the explicit way to drop them.
# Auxiliary object trees live beside build/ and must be cleared by both.
AUX_BUILD_DIRS := $(CODEBUILD) build-slice build-c build-s
clean:
	@if [ -d $(BUILD) ]; then \
	    find $(BUILD) -mindepth 1 -maxdepth 1 ! -name toolchains -exec rm -rf {} + ; \
	fi
	@rm -rf $(AUX_BUILD_DIRS)
	@echo "clean: removed build outputs (kept $(BUILD)/toolchains)"

distclean:
	rm -rf $(BUILD) $(AUX_BUILD_DIRS)

# Cheap, ROM-free hosted CI. Full byte verification remains a local gate.
.PHONY: ci-check progress progress-check
ci-check: progress-check
	@python3 tools/apple_decls.py --self-test
	@python3 tools/export_audit.py --self-test
	@python3 tools/call_audit.py --self-test
	@python3 tools/agbcc_c89_transform.py --self-test
	@python3 tools/test_matching_assist.py
	@python3 tools/test_decomp_report.py
	@python3 tools/test_makefile.py

progress-check:
	@python3 tools/decomp_report.py

# Never stamp a snapshot from an unchecked manifest or a stale build.
progress:
	@python3 tools/decomp_report.py --refresh
