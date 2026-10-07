#!/usr/bin/env python3
"""Screen corpus near-matches for group promotion, and stage the survivors.

`match_c_slice.py` recompiles a whole translation unit per manifest entry and
relinks the whole 188,760-byte slice, so discovering one unresolvable reference
at a time costs a full cycle. This screen applies every promotion rule from
`docs/matching_workflow.md` *statically*, so a whole group can be staged and
linked once.

    python3 tools/promotion_screen.py            # report only, no manifest writes
    python3 tools/promotion_screen.py --stage    # write the survivors
    python3 tools/promotion_screen.py --self-test

The rules it encodes, each of which cost a link cycle to learn:

* The owner index is read from the slice's **include closure**
  (`asm/code.s` -> `asm/passthrough.inc` -> the converted files), never from
  `asm/*.s`. A file that duplicates another's bytes but is included nowhere --
  `asm/__DELETED_GHOST2_TEST_FALLBACK__.s` still carries `0x08024150` -- otherwise looks like a second
  owner and every scan reports a phantom ambiguity (rule 1). Closure membership
  is what counts, so an `.inc` owns an address exactly as a `.s` does -- but a
  `.s` outranks an `.inc` that only names the same address, because there the
  `.inc` entry is a stub before an `.include` and the `.s` carries the bytes.
* **One C body owns one ROM address** (rule 3). `Garage_Noop` is the body of six
  4-byte `bx lr` stubs; staging all six makes the assembler report
  `symbol 'Garage_Noop' is already defined`. Candidates are grouped by the
  report's `alias_of` (falling back to `name`), and the group is seeded from the
  bodies *already* promoted, so a rerun cannot re-add a twin.
* Every assembly spelling branched to inside the spliced span must be re-exported
  (rules 4 and 6). Those go in the entry's `export` list -- but a spelling whose
  label sits *above* the start marker is outside the splice and must not.
* A span may run past its file's region end by the trailing alignment pad and
  nothing else. `promotable()` has already accepted those bytes as zero, so
  refusing them made the two rules contradict each other; overshooting into
  *real* code is still refused, since the slice is one sequential link.
* A callee only has to resolve, not score (rule 2). Names resolve through four
  sources: a VMA-shaped spelling, a C alias declaration, an already-promoted
  export set, and a closure branch target.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import os
import re
import subprocess
import tempfile
import sys
from pathlib import Path

from corpus_match_probe import code_o_staleness
from export_audit import exterior_refs, operand_refs

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "tools/matching_slice_functions.json"
REPORT = ROOT / "build/era-corpus/ready-report.json"
ROM_BASE = 0x08000000            # mirrors corpus_match_probe.ROM_BASE

# A label may carry a trailing `@` comment or trailing whitespace, and the
# converted regions do that constantly: `asm/carphys_tick.s` writes
# `_0800A06C:\t\t\t\t@ record 33 -> 15`. Requiring a bare `:$` silently dropped
# 35 VMA-shaped CODE labels from the owner index, and every body they could own
# then read as "no closure owner" -- three of them were already-exact 4-byte
# bodies (0x0800a06c, 0x0800a070, 0x0800a120) blocked with no diagnosis. The
# `@`-only tail is deliberate: a DATA label such as `_080003B0: .4byte
# 0x087B04C4` must still be rejected, or a constant pool would start owning
# function bodies. 3,987 such data labels exist and 35 code labels did not.
LABEL = re.compile(r"^([A-Za-z_][A-Za-z_0-9]*):\s*(?:@.*)?$")


def index_asm(text: str) -> tuple[dict[int, list[str]], set[str]]:
    """VMA->labels index and branch-target set for one assembly source.

    Module scope, not a method: the self-test must exercise the same function
    `Screen.__init__` calls, or a deleted or reverted rule would still pass.
    """
    index: dict[int, list[str]] = {}
    targets: set[str] = set()
    for line in text.splitlines():
        match = LABEL.match(line)
        if match and (vma := vma_of(match.group(1))) is not None:
            index.setdefault(vma, []).append(match.group(1))
        targets.update(BRANCH.findall(line))
    return index, targets


def _bare(line: str) -> str:
    """`line` with any trailing `@` comment removed, for marker comparison.

    The converted regions annotate labels constantly -- `asm/carphys_tick.s:568`
    is `_0800A06C:\t\t\t@ record 33 -> 15` -- and an exact `line.strip() ==
    "NAME:"` test silently fails on every one of them, so the label was in
    `by_file` but had no findable position and the body read as "no end
    marker". Only an `@` tail is stripped: a DATA label such as
    `_080003B0: .4byte 0x087B04C4` must keep failing to match, or a splice
    could bound itself on a constant-pool word. Kept in step with the
    `marker()` regex in `match_c_slice.replace_body`, which must accept exactly
    the same line shapes or the screen offers a body the splicer refuses.
    """
    s = line.strip()
    at = s.find("@")
    return s[:at].rstrip() if at != -1 else s

INCLUDE = re.compile(r'^\s*\.include\s+"([^"]+)"')
BRANCH = re.compile(
    r"\b(?:bl|b|bne|beq|bgt|blt|bge|ble|bhi|bls|bcc|bcs|bpl|bmi|bx|blx)"
    r"\s+([A-Za-z_][A-Za-z_0-9]*)\b"
)
# A VMA-shaped spelling. Both the 9-digit (`_080021B38`) and 8-digit
# (`_08021B38`) forms name the *same* address -- the corpus spells a VMA both
# ways and gas keeps them distinct symbols -- so strip the shared `080`/`0800`
# prefix rather than parsing the digits whole; the greedy `0*` backtracks onto
# the five-digit tail for either. `Course_0x080075E8` hides it behind a `0x`.
VMA_NAME = re.compile(r"^(?:[A-Za-z_]+?)(?:0x)?0*80([0-9A-Fa-f]{4,7})$")
# The corpus also spells a VMA as a friendly name plus its five hex digits
# (`ObjList_03350` -> 0x08003350, `TimeStr_03D4C` -> 0x08003D4C). Tried only
# after VMA_NAME fails, so a `_0800…` spelling always wins.
FRIENDLY_VMA = re.compile(r"^[A-Za-z_][A-Za-z_0-9]*?(0[0-9A-Fa-f]{4})$")
REGION = re.compile(r"VMA\s+(0x[0-9A-Fa-f]{8})-(0x[0-9A-Fa-f]{8})")
ALIAS_DECL = re.compile(
    r"^([A-Za-z_].*?)\b([A-Za-z_0-9]+)\((.*?)\)\s*__attribute__\(\(alias\("
    r'"([A-Za-z_0-9]+)"\)\)\);\s*$'
)

# agbcc's own runtime, linked verbatim from `lib1thumb.asm` (see the runtime
# provenance table in `docs/compiler_status.md`). These are friendly C names with
# no VMA in the spelling, so the closure cannot resolve them by name.
RUNTIME_NAMES = {
    "memcpy": 0x0802E0A4,
    # The 16-entry armcc interworking veneer pool, read off the ROM at
    # 0x0802DDC8..0x0802DE03: `bx r0`, `bx r1`, ... `bx lr`, each 4 bytes
    # (2 for the bx, `c0 46` for the pad). Only the first was bound, so a body
    # calling through r2 -- a call THROUGH A REGISTER rather than through lr --
    # had no name the screen could resolve, and 0x08004F34 stayed blocked at
    # 52/52. `era_runtime_probe.py` proves `_call_via_rX` byte-for-byte; the
    # other 15 are the same 4-byte pattern at a known offset in the same pool.
    "_call_via_rX": 0x0802DDC8,
    "_call_via_r1": 0x0802DDCC,
    "_call_via_r2": 0x0802DDD0,
    "_call_via_r3": 0x0802DDD4,
    "_call_via_r4": 0x0802DDD8,
    "_call_via_r5": 0x0802DDC,
    "_call_via_r6": 0x0802DDE0,
    "_call_via_r7": 0x0802DDE4,
    "_call_via_r8": 0x0802DDE8,
    "_call_via_r9": 0x0802DDEC,
    "_call_via_sl": 0x0802DDF0,
    "_call_via_fp": 0x0802DDF4,
    "_call_via_ip": 0x0802DDF8,
    "_call_via_sp": 0x0802DDFC,
    "_call_via_lr": 0x0802DE00,
    "__udivsi3": 0x0802DF6C,
    "__umodsi3": 0x0802DFE4,
    "__divsi3": 0x0802DE04,
    "__modsi3": 0x0802DE9C,
    "CpuSet_2D974": 0x0802D974,
    "CpuFastSet": 0x0802D970,
}


def region_end(path: Path) -> int | None:
    """End VMA of a converted file's declared region, from its header comment.

    A candidate's ROM span may not run past the file that owns it. Replacing
    `[start, end)` with a section sized from `end_vma` removes the file's real
    bytes and inserts more, shifting every later address -- which surfaces as
    `invalid offset, target not word aligned` in an unrelated file, because the
    slice is one monolithic sequential link.
    """
    for line in path.read_text(errors="replace").splitlines()[:6]:
        match = REGION.search(line)
        if match:
            return int(match.group(2), 16)
    return None


def friendly_vma(name: str) -> int | None:
    """VMA from the corpus's `Friendly_0XXXX` naming convention."""
    match = FRIENDLY_VMA.match(name)
    return 0x08000000 | int(match.group(1), 16) if match else None


def stale_sources(report: Path) -> list[str]:
    """Files newer than the compiled report, repo-relative and sorted.

    Two inputs decide what the report says, and both have to be covered:

    * `src/*.c` -- `call_targets` and every byte come from the *compiled* corpus
      objects, so a report older than the C grades edits that have not happened
      yet.
    * `asm/*.s` and `asm/*.inc` -- the probe resolves every callee through the
      closure, and a *byte-neutral label move* changes those addresses without
      changing a single byte. Moving a misplaced label from 0x08002DC2 up to its
      name's 0x08002DB8 is exactly that, and it silently invalidates every
      `call_targets` entry naming it.

    The failure is silent by construction: the screen returns a confident
    answer about the wrong tree. A missing report is absent, not stale, and
    `REPORT.read_text` already raises for that case.
    """
    if not report.exists():
        return []
    cutoff = report.stat().st_mtime
    watched = ([*_iter(ROOT / "src", "*.c")]
               + [*_iter(ROOT / "asm", "*.s"), *_iter(ROOT / "asm", "*.inc")])
    return sorted(str(p.relative_to(ROOT)) for p in watched
                  if p.stat().st_mtime > cutoff)


def _iter(directory: Path, pattern: str):
    return sorted(directory.glob(pattern)) if directory.is_dir() else []


def _probe():
    spec = importlib.util.spec_from_file_location(
        "corpus_match_probe", ROOT / "tools/corpus_match_probe.py"
    )
    module = importlib.util.module_from_spec(spec)
    sys.path.insert(0, str(ROOT / "tools"))
    spec.loader.exec_module(module)
    return module


def vma_of(name: str) -> int | None:
    match = VMA_NAME.match(name)
    return 0x08000000 | int(match.group(1), 16) if match else None


def closure_files() -> set[Path]:
    """The slice's include closure, starting at `asm/code.s`."""
    seen: set[Path] = set()
    stack = [ROOT / "asm/code.s"]
    out: set[Path] = set()
    while stack:
        path = stack.pop()
        if path in seen or not path.exists():
            continue
        seen.add(path)
        out.add(path)
        for line in path.read_text(errors="replace").splitlines():
            match = INCLUDE.match(line)
            if match:
                stack.append(path.parent / match.group(1))
    return out

def has_end_anchor(lines: list[str], stem: str) -> bool:
    """Is `<stem>_end:` present as a line of its own in `lines`?

    The splicer's own notion of a marker is `^\\s*LABEL\\s*$` (see
    `match_c_slice.replace_body`), so a plain substring test is not enough: it
    would accept the name inside a comment or mid-line, and the splice would
    still raise on a marker that is not really there. This lives at module
    scope, not inside `self_test()`, so the test cases exercise the same
    function `run()` calls -- a private copy inside the test would pass even if
    the guard were deleted outright.
    """
    return any(line.strip() == f"{stem}_end:" for line in lines)


def alias_decls(text: str) -> list[tuple[str, str]]:
    """(alias, definition) for every `__attribute__((alias("...")))` in `text`.

    Module scope so the self-test feeds it synthetic source rather than
    asserting against the live `src/` tree, which would make the case track the
    build instead of the rule.

    Two things this gets right that the previous inline scan did not, and both
    were silent -- the scan simply found fewer names, with no error anywhere:

    * **Strip a `//` comment first.** Comment text was accumulated into the
      pending buffer, so a declaration preceded by a remark matched nothing.
    * **A closing brace ends a function BODY, which is not a declaration.**
      Without clearing on `}`, the `}` was carried into the next alias and the
      whole blob failed `ALIAS_DECL`'s `^[A-Za-z_]` anchor.

    Measured on this tree before the fix: **1177 of 2709 alias declarations
    across 146 files were lost — 43%** — and the affected names then read as
    "no VMA and not a promoted export", which looks like a missing declaration
    rather than a broken scanner. After: 2 rejections, both genuinely
    non-`alias` attribute uses.
    """
    found: list[tuple[str, str]] = []
    pending = ""
    for line in text.splitlines():
        stripped = line.split("//", 1)[0].strip()
        if not stripped or stripped.startswith("#"):
            continue
        if stripped == "}":
            pending = ""
            continue
        pending += stripped
        if ";" not in pending:
            continue              # the __attribute__ is often on its own line
        for decl in pending.split(";"):
            match = ALIAS_DECL.match(decl.strip() + ";")
            if match:
                found.append((match.group(2), match.group(4)))
        pending = ""
    return found

def export_names(own: set[str], spanned: set[str], referenced: set[str],
                 body_name: str, is_local) -> list[str]:
    """Alias spellings the splice must re-export for a promoted body.

    Module scope so the self-test exercises the SAME function `run()` calls.
    A test that recomputes the rule locally passes whether or not the rule
    exists, which is exactly what happened the first time this was pinned.

    `referenced` is EVERY symbolic operand read -- branches AND `ldr`/`adr`
    literals AND `.word`-in-expression -- not just branch targets. The old
    branch-only set missed `_0802BCE4`, read by `ldr r2, _0802BCE4` at
    asm/sound_channel_cluster.s:42, and the export never appeared in the
    staged entry; only `make matching-ready` caught the dangling read. It is
    still narrowed by `is_local`, so self-only pool labels like `_0802BE74`
    are candidates and then filtered, never exported.

    The asymmetry is the point: `own` -- the label(s) at the entry's OWN VMA --
    is deliberately NOT intersected with `referenced`, because that set is
    built by scanning asm/ only. A twin called exclusively from C never enters
    it, and without this the spliced link draws an undefined reference for a
    name the screen had already approved. `spanned` keeps the intersection:
    those are interior span labels, and `is_local` filters the ones that are not
    real entry points.

    Exporting a twin is safe because `_export_missing` re-emits only when
    `_same_vma(name, body)` or `_name_vma(name) == entry_vma`; a name that does
    not denote this body is still refused loudly.
    """
    candidates = ((referenced & spanned) | own) - {body_name}
    return sorted(n for n in candidates if not is_local(n))


def label_is_local(name: str, vma: int, end_vma: int,
                   nm_addrs: dict[str, int],
                   refs: dict[str, set[tuple[Path, int]]],
                   owner: Path, bounds: tuple[int, int]) -> bool:
    """An intra-function label: interior address AND no read from outside.

    Module scope so the self-test pins the rule `run()` applies. Both halves
    matter. The ADDRESS half is the original rule and is wrong alone:
    `_0802BCF6` sits strictly inside `_0802BCF4`'s span yet `sub_0802BD44`
    branches to it twice (trap 7's lever), and `_0802BCE4` sits strictly
    inside `_0802BCCE`'s span yet is read by an `ldr` literal from below --
    calling either local silently drops the export the outside read needs and
    only the slow tier notices. The DIRECTION half is the fix: a label is
    local exactly when every symbolic read of it lies inside the replaced
    region `[bounds[0], bounds[1))` of `owner`, via `exterior_refs` -- the same
    predicate export_audit's direction 3 runs. A label nothing reads is local
    by default: no read can dangle, so demanding an export would only force a
    definition the body may not emit.
    """
    nv = nm_addrs.get(name)
    if nv is None or not (vma < nv < end_vma):
        return False
    return not exterior_refs(refs, {name}, owner, bounds, set())

class Screen:
    def __init__(self) -> None:
        self.files = closure_files()
        # Where each closure label *assembles*. Populated eagerly: an empty map
        # must be loud, because the fallback re-reads the label's spelling and
        # silently produces the wrong verdict for a misplaced label.
        self.nm_addrs = self._nm_addrs()
        if not self.nm_addrs:
            sys.stderr.write(
                "WARNING: no closure object (build-code/code.o); label addresses "
                "fall back to name parsing, which mis-resolves misplaced labels\n")
        # per-file label index: the owner lookup is a dict lookup, never a
        # file-independent predicate (which silently collapses to one file).
        self.by_file: dict[Path, dict[int, list[str]]] = {}
        self.branch_targets: set[str] = set()
        # Every symbolic read, keyed by name to (file, line) sites. One index
        # feeds both `label_is_local`'s direction rule and `export_names`, so
        # the screen and export_audit agree on what a read IS (branch operands
        # are the minority carrier; `ldr`/`adr` literals and `.word`-in-
        # expression spell the same kind of dangling reference).
        self.refs: dict[str, set[tuple[Path, int]]] = {}
        for path in sorted(self.files):
            text = path.read_text(errors="replace")
            index, targets = index_asm(text)
            self.by_file[path] = index
            self.branch_targets |= targets
            for name, idxs in operand_refs(text.splitlines()).items():
                self.refs.setdefault(name, set()).update((path, i) for i in idxs)

        # A branch *operand* is not a symbol. Only names some closure file
        # actually defines as a label can satisfy a reference; otherwise a
        # call under a hybrid-only spelling (`Sub_...`, a friendly name) looks
        # resolvable and costs a link cycle to discover otherwise.
        defined = {n for index in self.by_file.values() for names in index.values()
                   for n in names}
        self.branch_targets &= defined
        self.refs = {n: sites for n, sites in self.refs.items() if n in defined}
        self.promoted = json.loads(MANIFEST.read_text(encoding="utf-8"))
        self.report = json.loads(REPORT.read_text(encoding="utf-8"))
        self.by_vma = {int(r["vma"], 16): r for r in self.report["results"]}
        self.exports, self.bodies = self._promoted_state()
        self.name_to_vma = self._name_index()
        # Deliberately NOT computed here. `__init__` runs inside `--self-test`,
        # whose fixtures are synthetic; reading the live report from it would
        # make the self-test's output depend on build state, and a case that
        # breaks whenever the report is refreshed gets deleted rather than
        # understood. `main()` asks for it explicitly instead.
        self.stale: list[str] = []

    # -- promoted state ---------------------------------------------------
    def body_of(self, vma: int) -> str:
        rec = self.by_vma.get(vma)
        if rec:
            return rec.get("alias_of") or rec["name"]
        return self.promoted[f"{vma:#010x}"].get("c_name", "")

    def _promoted_state(self) -> tuple[set[str], dict[str, int]]:
        exports: set[str] = set()
        bodies: dict[str, int] = {}
        for key, entry in self.promoted.items():
            vma = int(key, 16)
            rec = self.by_vma.get(vma)
            names = {entry.get("c_name"), rec["name"] if rec else None}
            if rec and rec.get("alias_of"):
                names.add(rec["alias_of"])
            exports |= {n for n in names | set(entry.get("export", ())) if n}
            body = self.body_of(vma)
            if body and body not in bodies:          # rule 3: first VMA wins
                bodies[body] = vma
        return exports, bodies

    def _name_index(self) -> dict[str, int]:
        """Friendly C name -> VMA, from every alias declaration under `src/`.

        `ALIAS_DECL` group 1 is the text *before* the name (`"void "`), group 2
        the alias and group 4 the definition it points at; testing group 1
        means `vma_of("void ")` is always None and the index stays empty. The
        attribute is also frequently wrapped onto its own line, so declarations
        are re-joined before matching.
        """
        index: dict[str, int] = dict(RUNTIME_NAMES)
        for path in sorted((ROOT / "src").glob("*.c")):
            for alias, target in alias_decls(path.read_text(errors="replace")):
                vma = vma_of(alias) or friendly_vma(alias)
                if vma is None:
                    continue
                index.setdefault(target, vma)   # friendly body name
                index.setdefault(alias, vma)    # the alias itself
        for key, entry in self.promoted.items():
            for name in (entry.get("c_name"), *entry.get("export", ())):
                if name:
                    index.setdefault(name, int(key, 16))
        return index


    @staticmethod
    def _nm_addrs() -> dict[str, int]:
        """Label -> the address it *assembles* at, from the linked closure object.

        A label's address must come from the assembler, never from its spelling:
        the converter emits the `.type`/label pair *after* an armcc high-register
        prologue preamble, so `sub_08002BB4` can assemble 4 bytes past the address
        its name claims.  `resolve()` below used the name, and recommended
        retargeting a call onto a label that pointed somewhere else -- which
        linked, and then silently moved the call target (4 bytes is 2 in the
        encoded Thumb displacement).  `arm-none-eabi-nm` on the closure object is
        the only sound source; on a relocatable `.o` its values are file offsets,
        hence the `0x08000000` base.
        """
        obj = ROOT / "build-code" / "code.o"
        if not obj.exists():
            return {}
        # Staleness guard, shared with the probe's own `code.o` reads
        # (`corpus_match_probe.code_o_staleness`): one mtime rule and one
        # wording, because after an `asm/*.s` edit the object is out of date
        # until rebuilt and the screen would then grade against pre-edit
        # addresses with no signal -- the same silent-wrong-verdict shape one
        # step removed from "object missing".
        stale = code_o_staleness(obj, closure_files())
        if stale:
            print(stale, file=sys.stderr)
        try:
            out = subprocess.run(
                ["arm-none-eabi-nm", str(obj)],
                capture_output=True, text=True, check=True).stdout
        except (OSError, subprocess.CalledProcessError) as exc:
            print(f"WARNING: arm-none-eabi-nm failed ({exc}); label addresses "
                  "fall back to name parsing", file=sys.stderr)
            return {}
        addrs: dict[str, int] = {}
        for line in out.splitlines():
            parts = line.split()
            if len(parts) == 3:
                try:
                    addrs[parts[2]] = int(parts[0], 16) + 0x08000000
                except ValueError:
                    pass
        return addrs

    # -- resolution -------------------------------------------------------
    def resolve(self, name: str) -> tuple[bool, str]:
        """Classify one callee by its *exact* spelling (rules 1 and 9).

        `_08002A3C` and `sub_08002A3C` name the same address, so a per-VMA test
        passes both and the link then fails on the one the C actually emitted.
        A reference resolves only under a name some closure file defines, an
        already-promoted export, or a runtime name; everything else is reported
        with the fix, so a group is edited once instead of one cycle per symbol.
        """
        if not name:
            return True, ""
        vma = vma_of(name) or friendly_vma(name)
        if vma is None:
            # A friendly C name: the splice may only call what the closure
            # defines, so the call site has to use that spelling.
            vma = self.name_to_vma.get(name)
        if vma is None:
            return False, f"{name}: no VMA and not a promoted export"
        # A promoted export is defined by the spliced C itself, not by the
        # closure asm, so it is exempt from the assembler's address check.
        if name in self.exports:
            return True, ""
        # Trust the assembler, not the spelling (see `_nm_addrs`).
        actual = self.nm_addrs.get(name)
        if actual is not None:
            if actual == vma:
                return True, ""
            return False, (
                f"{name}: assembles at {actual:#010x}, not {vma:#010x} "
 f"(misplaced label; move it, or drop the caller)")
        if name in self.closure_labels:
            return True, ""
        at_vma = sorted({n for f in self.files for n in self.by_file.get(f, {}).get(vma, ())})
        at_vma = [n for n in at_vma if self.nm_addrs.get(n, vma) == vma]
        if not at_vma:
            return False, f"{name}: no closure label at {vma:#010x} (drop the caller)"
        # Same address, different spelling. A compiler-RUNTIME name is the case
        # that matters: agbcc emits `_call_via_r2` for a call through r2, while
        # the closure calls that same veneer `_0802DDD0`. Both denote 0x0802DDD0,
        # so this is a rename of one routine, not two routines -- and demanding a
        # rename here blocked 0x08004F34 at 52/52, a body already byte-exact.
        # Accept ONLY when the runtime table and the closure agree on the
        # address, so this cannot be used to accept a genuine mismatch.
        if RUNTIME_NAMES.get(name) == vma and at_vma:
            return True, ""
        return False, f"{name}: closure defines {'/'.join(at_vma)} at {vma:#010x} (rename)"

    def resolvable(self, name: str) -> bool:
        return self.resolve(name)[0]

    @property
    def closure_labels(self) -> set[str]:
        return {n for f in self.files for names in self.by_file.get(f, {}).values() for n in names}

    def owners(self, vma: int) -> list[Path]:
        """Closure file(s) owning `vma`; `.inc` owns as much as `.s` does.

        The `f.suffix == ".s"` filter this replaced was wrong in both
        directions. It hid the dispatcher: `asm/passthrough.inc` owns a region
        whenever the converted file it includes carries no VMA label of its
        own, so `0x08024138` and `0x08024144` read as "no closure owner"
        although the closure plainly labels them. And dropping the filter
        outright is no better -- dozens of addresses are named by both a `.s`
        and the `.inc`, and there the `.s` holds the bytes while the `.inc`
        entry is a stub before an `.include`, so counting both would turn each
        of those into a phantom `multi-owner` block. A `.s` owner therefore
        outranks a `.inc` one, and the `.inc` owns only where nothing else does.
        A genuine duplicate still returns both, which is what the multi-owner
        guard is for; `asm/__DELETED_GHOST2_TEST_FALLBACK__.s` stays out because `closure_files()` never
        reaches it.
        """
        owners = [f for f in sorted(self.files) if vma in self.by_file.get(f, {})]
        code = [f for f in owners if f.suffix == ".s"]
        return code or owners

    @staticmethod
    def pick_marker(lines: list[str], vma: int, index: dict[int, list[str]]) -> str | None:
        """Outermost unique marker for the label(s) at `vma`.

        A function carries a friendly name, a `sub_`/`_` alias and a `.type`
        line at one VMA. Only the outermost is a safe splice start; a leftover
        label above the replacement collides with the `.globl` agbcc emits.
        """
        best: tuple[int, str] | None = None
        for name in index.get(vma, ()):
            for candidate in (f".type {name}, %function", f"{name}:"):
                if sum(1 for line in lines if _bare(line) == candidate) != 1:
                    continue
                at = next(i for i, line in enumerate(lines) if _bare(line) == candidate)
                if best is None or at < best[0]:
                    best = (at, candidate)
        return best[1] if best else None

    # -- candidates -------------------------------------------------------
    def promotable(self, rec: dict, rom: bytes) -> bool:
        """Is this record good enough to *stage*, independent of linkability?

        Two classes qualify, and the better one used to be missing:

        * `EXACT` -- the compiled C body already equals the ROM span byte for
          byte, pools and alignment included. This is the strongest evidence
          the probe can produce short of a link, so refusing it is strictly
          backwards: gating on `alignment_only` alone discarded every exact
          body and reported an empty queue while 60 of them (1424 bytes on the
           report) sat unpromoted.
        * alignment-only `PARTIAL` -- the whole body matches and the sole miss
          is the trailing 2-byte pad, which the independent link settles.

        Everything else (`OVERSIZED`, `NO_OVERLAP`, `UNRESOLVED_RELOCATION`,
        and any `PARTIAL` with a real instruction difference) is a *matching*
        task for a source edit, not a promotion. Those are counted separately
        so an empty stageable list is never mistaken for an empty backlog.
        """
        return rec["status"] == "EXACT" or self.alignment_only(rec, rom)

    def alignment_only(self, rec: dict, rom: bytes) -> bool:
        """`PARTIAL` whose only miss is the trailing 2-byte alignment pad.

        The gate is `prefix`, NOT `matched_bytes`. `matched_bytes` is a *count*
        of equal bytes over the whole span, so a body with an interior
        instruction difference and a zero tail satisfies
        `matched_bytes + 2 == rom_bytes` just as well as a genuinely
        alignment-only miss does. `prefix` is the length of the matching run
        from the start, so `prefix + 2 == rom_bytes` is the real claim: every
        byte up to the final pad is identical. `_0800CF60` is the instance the
        count admitted -- 62 of 64 bytes equal, first difference at +16, a
        `strh r2,[r0,#2]`/`mov r2,r4` pair where the ROM has
        `strh r2,[r0,#0]`/`mov r2,r0` -- and the link then failed on it.

        The tail check must be bounded to *this function's* span. Slicing the
        whole image from `prefix` onwards reads every later function too,
        rejects every candidate, and makes an empty queue look like a full one.
        """
        if rec["status"] != "PARTIAL":
            return False
        if rec["cand_bytes"] != rec["rom_bytes"]:
            return False
        prefix = rec.get("prefix")
        if prefix is None or prefix + 2 != rec["rom_bytes"]:
            return False
        base = int(rec["vma"], 16) - ROM_BASE
        span = rom[base:base + rec["rom_bytes"]]
        return not any(span[prefix:])

    @staticmethod
    def pad_only(rom: bytes, start_vma: int, end_vma: int) -> bool:
        """Are all the ROM bytes in `[start_vma, end_vma)` zero padding?

        The idiom `alignment_only()` uses, bounded to the overshoot rather than
        to a function's whole tail, plus a length check: a short slice at the
        end of the image would otherwise read as "all zero" and forgive a span
        that ran off the ROM entirely.
        """
        base = start_vma - ROM_BASE
        window = rom[base:end_vma - ROM_BASE]
        return len(window) == end_vma - start_vma and not any(window)

    def run(self) -> tuple[list[dict], list[tuple[str, str]], dict[str, int]]:
        probe = _probe()
        spans = probe.rom_functions()
        rom = probe.ROM.read_bytes()
        staged: list[dict] = []
        blocked: list[tuple[str, str]] = []
        counts = {"considered": 0, "promoted": 0, "not-promotable": 0}
        for rec in self.report["results"]:
            vma = int(rec["vma"], 16)
            counts["considered"] += 1
            if f"{vma:#010x}" in self.promoted:
                counts["promoted"] += 1
                continue
            if not self.promotable(rec, rom):
                counts["not-promotable"] += 1
                continue
            body = rec.get("alias_of") or rec["name"]
            if body in self.bodies:                       # rule 3
                blocked.append((rec["vma"], f"body {body} already owns "
                                             f"{self.bodies[body]:#010x}"))
                continue
            why = [self.resolve(c.get("symbol", ""))[1]
                   for c in rec.get("call_targets", []) if not self.resolvable(c.get("symbol", ""))]
            if why:
                blocked.append((rec["vma"], "; ".join(sorted(set(why)))))
                continue
            owners = self.owners(vma)
            if not owners:
                blocked.append((rec["vma"], "no closure owner"))
                continue
            if len(owners) > 1:
                blocked.append((rec["vma"], f"multi-owner {[p.name for p in owners]}"))
                continue
            end_vma = spans.get(vma)
            path = owners[0]
            lines = path.read_text(errors="replace").splitlines()
            index = self.by_file[path]
            start = self.pick_marker(lines, vma, index)
            if not start or not end_vma:
                blocked.append((rec["vma"], "no unique start/end marker"))
                continue

            # The span must stay inside the file that owns it. With a synthetic
            # end anchor it must end *exactly* at the region boundary, since
            # nothing else in that file is replaced. An overshoot of nothing
            # but the trailing alignment pad is clamped rather than refused:
            # `promotable()` admits that candidate and `alignment_only()`
            # verified those very bytes were zero, so blocking them here made
            # the two rules contradict each other and pinned a 26/28 body
            # forever. The clamp is taken *before* the span walk, so `spanned`
            # and the end marker are both derived from the final span -- which
            # is the ordering trap here, since `end` used to be picked against
            # the unclamped end. Overshooting into real code stays refused; see
            # `region_end()` for why the slice cannot absorb it.
            limit = region_end(path)
            if limit is not None and end_vma > limit:
                if not self.pad_only(rom, limit, end_vma):
                    blocked.append((rec["vma"],
                                    f"span 0x{vma:08x}-0x{end_vma:08x} runs past "
                                    f"{path.name} region end 0x{limit:08x}"))
                    continue
                end_vma = limit

            # `_bare`, not `.strip()`: `pick_marker` may hand back a label the
            # regions wrote with a trailing `@` comment, and an exact match here
            # then raises StopIteration and takes the whole screen down.
            start_at = next(i for i, l in enumerate(lines) if _bare(l) == start)
            # Span labels the splice deletes: everything at [vma, end_vma) whose
            # label line sits *after* the start marker. A label above the marker
            # survives the splice and must not be re-exported.
            spanned: set[str] = set()
            for addr, names in index.items():
                if not vma <= addr < end_vma:
                    continue
                for name in names:
                    line = f"{name}:"
                    if sum(1 for l in lines if _bare(l) == line) == 1 and \
                            next(i for i, l in enumerate(lines) if _bare(l) == line) > start_at:
                        spanned.add(name)
            end = self.pick_marker(lines, end_vma, index)

            if limit is not None and end is None and end_vma != limit:
                blocked.append((rec["vma"],
                                f"no end marker in {path.name} and span ends "
                                f"0x{end_vma:08x} != region end 0x{limit:08x}"))
                continue
            # The `<stem>_end:` fallback must actually be in the file. It is not
            # merely a naming convention: `replace_body` splices from the start
            # marker to this label, so a fallback that does not exist either
            # fails to match or -- worse, once one is appended at EOF -- deletes
            # everything from the body to the end of the region. The common
            # case is a span that ends INSIDE an `.include`, where no textual
            # marker can bound it at all; those are not stageable yet.
            if end is None and not has_end_anchor(lines, path.stem):
                blocked.append((rec["vma"],
                                f"no end marker in {path.name} and no "
                                f"{path.stem}_end: to fall back on (span "
                                f"0x{vma:08x}-0x{end_vma:08x} likely ends "
                                f"inside an .include)"))
                continue
            entry = {
                "c_name": rec["name"],
                "asm_file": f"asm/{path.name}",
                "start_marker": start,
                "end_marker": end or f"{path.stem}_end:",
                "end_vma": f"{end_vma:#010x}",
            }
            # A closure branch target at the entry's OWN VMA needs exporting too,
            # not just one inside the span. `spanned` deliberately skips labels
            # at or above the start marker (they survive the splice), so a
            # differently-spelled twin of the body -- `sub_0802B07C` when the
            # promoted body is `SoundVBlank` -- never lands in the export set,
            # and retained asm that calls it (`asm/boot.s: bl sub_0802B07C`)
            # links against a missing `.thumb_set`. Its section is zero bytes
            # and is not spliced, so the reference is genuinely undefined.
            # The replaced region is `[start_at, end_at)`, exactly what
            # `replace_body` splices out; the direction rule compares read
            # sites against it. The end anchor's line is the exclusive bound,
            # and a marker that cannot be found falls back to EOF rather than
            # taking the whole screen down with StopIteration.
            end_marker = end or f"{path.stem}_end:"
            end_at = next((i for i, l in enumerate(lines)
                           if _bare(l) == end_marker), len(lines))
            own = set(index.get(vma, ()))
            # The one name to exclude is the body actually being spliced, which
            # is `alias_of` when the probe scored an alias and `name` when it
            # scored the real function. Excluding `name` unconditionally is
            # wrong: when the probe names the record `sub_08007A04` with
            # `alias_of` `_08007A04`, that closure spelling is exactly the
            # symbol the retained asm calls, and dropping it leaves
            # `asm/course_leaves_95f0.s: bl sub_08007A04` undefined.
            body_name = rec.get("alias_of") or rec["name"]
            # A branch target whose OWN address lies strictly inside the span is
            # an intra-function label -- a loop back-edge or an internal jump --
            # not an entry point the retained asm calls from outside. The C
            # reproduces it with its own control flow and must not be asked to
            # export it: `_080023DE` inside `_080023C4` is `ble _080023DE` at
            # code_22e4.s:147, nothing outside the span references it, and
            # requiring the splice to bind it is a link failure for a label
            # whose name deliberately does not denote the body.
            def is_local(n: str) -> bool:
                return label_is_local(n, vma, end_vma, self.nm_addrs,
                                      self.refs, path, (start_at, end_at))
            # `own` -- the label(s) at the entry's OWN VMA -- is NOT intersected
            # with `branch_targets`, and that asymmetry is the point.
            # `branch_targets` is built by scanning asm/ only, so a twin that is
            # called exclusively from C never enters it: 0x08005604 staged with
            # `c_name` `_08005604` while src/menus.c calls `sub_08005604`, and the
            # spliced link drew an undefined reference for it. Exporting a twin
            # is safe because `_export_missing` re-emits only when
            # `_same_vma(name, body)` or `_name_vma(name) == entry_vma` -- a
            # name that does not denote this body is still refused loudly.
            # `spanned` keeps the intersection, because those are interior span
            # labels and `is_local` already filters the ones that are not real
            # entry points.
            needed = export_names(set(own), set(spanned), set(self.refs),
                                  body_name, is_local)
            if needed:
                entry["export"] = needed
            elif not end:
                entry["_needs_anchor"] = True
            staged.append({"vma": rec["vma"], "bytes": rec["rom_bytes"], "entry": entry,
                           "body": body})
            self.bodies[body] = vma          # keep the group unique within the group
        return staged, blocked, counts


def stale_verdict(stale: list[str], staging: bool) -> str:
    """`"refuse"`, `"warn"`, or `""` -- the staleness policy, by mode.

    A dry run only *reads* the report, so a stale one costs a wrong answer to a
    question someone is asking anyway: warn, name the files, continue.
    `--stage` *writes* `tools/matching_slice_functions.json` from that report,
    so the same staleness silently commits pre-edit verdicts into the manifest.
    That is the operation that does the damage, so it is the one that fails.

    The split is also what keeps the warning meaningful. With a set of concurrent edits
    editing `src/`, the report can be stale within minutes; a warning that is
    always on is a warning nobody reads.
    """
    if not stale:
        return ""
    return "refuse" if staging else "warn"


def self_test() -> int:
    ok = 0
    total = 0

    def check(name: str, got, want) -> None:
        # `total` is counted, never hardcoded: adding a case then silently
        # breaks the exit status, and the next person "fixes" it by editing
        # the number instead of finding the broken case.
        nonlocal ok, total
        total += 1
        if got == want:
            ok += 1
        else:
            print(f"FAIL {name}: got {got!r} want {want!r}", file=sys.stderr)

    check("9-digit spelling", vma_of("_080021B38"), 0x08021B38)
    check("8-digit spelling names the same address", vma_of("_08021B38"), 0x08021B38)
    check("sub_ prefix", vma_of("sub_080021B38"), 0x08021B38)
    check("vma behind 0x", vma_of("Course_0x080075E8"), 0x080075E8)
    check("friendly name", vma_of("ObjList_03350"), None)
    check("closure has code.s", (ROOT / "asm/code.s") in closure_files(), True)
    check("closure excludes dead file", (ROOT / "asm/__DELETED_GHOST2_TEST_FALLBACK__.s") in closure_files(), False)
    lines = ["@ x", ".type sub_0800AAAA, %function", "sub_0800AAAA:", "movs r0, r0",
             ".type sub_0800AAB4, %function", "sub_0800AAB4:"]
    idx = {0x0800AAAA: ["sub_0800AAAA"], 0x0800AAB4: ["sub_0800AAB4"]}
    check("outermost marker", Screen.pick_marker(lines, 0x0800AAAA, idx),
          ".type sub_0800AAAA, %function")
    # `pick_marker` must find a label the converted regions actually write, and
    # must still refuse a data label. Pin BOTH: a one-sided case passes just as
    # well if the rule simply stopped matching, which is the failure this
    # widening risks. The accept side is the real bug -- the label was in
    # `by_file` but had no findable position, so the body read as
    # "no end marker" while the closure plainly named it.
    cmt = ["_0800A06C:\t\t\t@ record 33 -> 15", "movs r0, #15", "bx lr"]
    check("label with a trailing @ comment is found",
          Screen.pick_marker(cmt, 0x0800A06C, {0x0800A06C: ["_0800A06C"]}),
          "_0800A06C:")
    data = ["_080003B0: .4byte 0x087B04C4", "movs r0, #0"]
    check("data label is NOT a splice marker",
          Screen.pick_marker(data, 0x080003B0, {0x080003B0: ["_080003B0"]}), None)
    # `alias_decls` is the rule that decides which C names the screen can
    # resolve, and it failed SILENTLY: 1177 of 2709 declarations across 146
    # files were lost, and every one then read as "no VMA and not a promoted
    # export" -- which looks like a missing declaration rather than a broken
    # scanner. Synthetic source, never the live tree, so the case tests the rule
    # and not whatever src/ happens to contain today.
    src = ("void f(void) {\n"
           "    (void)0;\n"
           "}\n"
           "// a remark mentioning _08009999 which is prose, not a declaration\n"
           "void _08024A18(void) __attribute__((alias(\"SaveBlock2PreSave\")));\n")
    got = alias_decls(src)
    check("alias after a closing brace is found",
          ("_08024A18", "SaveBlock2PreSave") in got, True)
    check("a remark mentioning a VMA is not a declaration",
          any(a == "_08009999" for a, _ in got), False)
    # Separate the two fixes: this one has NO preceding brace, so it fails only
    # if comment-stripping is removed. Without it, reverting either fix fails the
    # SAME case and neither is individually pinned.
    only_comment = ("// prose mentioning _08008888\n"
                    "void _08024A98(void) __attribute__((alias(\"PostLoad\")));\n")
    check("alias after a comment but no brace is found",
          ("_08024A98", "PostLoad") in alias_decls(only_comment), True)
    # Reject side: a body that is NOT an alias attribute must yield nothing, so
    # a one-sided accept case could not pass a regex matching any attribute.
    check("a non-alias attribute yields nothing",
          alias_decls('__attribute__((weak)) u16 _08001E48_alias(int a){ return 0; }\n'), [])
    # The `own`-twin export rule, pinned BOTH ways. It is currently unpinned and
    # that is a real hole: the manifest now carries
    # `export: ['_08005604', 'sub_08005604']` baked in, so `make matching-slice`
    # passes whether or not the rule exists and reverting it leaves every gate
    # green. Synthetic Screen, synthetic by_file, so the case cannot be satisfied
    # by whatever the live manifest happens to hold.
    own_case = Screen.__new__(Screen)
    own_case.by_file = {Path("asm/x.s"): {0x08005604: ["_08005604", "sub_08005604"]}}
    own_case.branch_targets = {"_08005604"}     # ONLY the c_name is an asm branch target
    own_case.nm_addrs = {"_08005604": 0x08005604, "sub_08005604": 0x08005604}
    own = {"_08005604", "sub_08005604"}
    spanned_set: set[str] = set()
    body_name = "_08005604"
    vma_e, end_vma_e = 0x08005604, 0x08005614
    def _is_local_e(n):
        nv = own_case.nm_addrs.get(n)
        return nv is not None and vma_e < nv < end_vma_e
    wide = export_names(own, spanned_set, own_case.branch_targets, body_name, _is_local_e)
    # What the OLD rule computed: `own` intersected with the asm branch targets.
    narrow = sorted(n for n in ((own | spanned_set) & own_case.branch_targets)
                    if n != body_name and not _is_local_e(n))
    check("a C-only caller twin is exported", "sub_08005604" in wide, True)
    check("the asm-only intersection would have dropped it", "sub_08005604" in narrow, False)
    check("the c_name itself is never re-exported", body_name in wide, False)
    # A compiler-runtime name and a closure label at the SAME address are one
    # routine under two spellings, not a rename to perform. agbcc emits
    # `_call_via_r2` for a call through r2; the closure calls that same veneer
    # `_0802DDD0`. Without this, 0x08004F34 stayed blocked at 52/52.
    synth2 = Screen.__new__(Screen)
    synth2.exports = set()
    # resolve() derives the VMA from name_to_vma, which is seeded from
    # RUNTIME_NAMES in production -- so the fixture must do the same.
    synth2.name_to_vma = {"_call_via_r2": 0x0802DDD0}
    synth2.files = [Path("asm/x.s")]
    synth2.by_file = {Path("asm/x.s"): {0x0802DDD0: ["_0802DDD0"]}}
    synth2.nm_addrs = {"_0802DDD0": 0x0802DDD0}
    r_ok, _ = synth2.resolve("_call_via_r2")
    check("a runtime name and a closure label at ONE address resolve", r_ok, True)
    # It must NOT generalise: at a different address the refusal stands.
    synth2.nm_addrs = {"_0802DDD0": 0x0802DDD4}
    r_bad, r_why = synth2.resolve("_call_via_r2")
    check("a runtime name at a DIFFERENT address still refuses", r_bad, False)
    check("and names the reason", bool(r_why), True)

    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    check("manifest entries unique by VMA",
          len(manifest) == len({int(k, 16) for k in manifest}), True)
    screen = Screen()
    bodies = list(screen.bodies.items())
    check("no promoted body owns two VMAs", len(bodies) == len({b for b, _ in bodies}), True)
    # Pin the assembler-truth rule, in both directions. Without these the whole
    # `nm_addrs` check could be deleted and every gate would still read 10/10:
    # a fix that is not pinned is a fix that can be reverted by accident.
    #
    # The inputs are SYNTHETIC, following the `pick_marker` case above. Asserting
    # against `sub_08003104` and the real `build-code/code.o` would make the case
    # track the build: it would pass or fail depending on whether that object
    # exists and is current, and `sub_08003104` is on the 13-label census
    # backlog -- the day someone moves it, the gate would fail for a legitimate
    # reason and the natural "fix" would be to delete the case.
    synth = Screen.__new__(Screen)          # no build state, no asm scan
    synth.exports = set()
    synth.by_file = {}
    synth.files = []
    synth.name_to_vma = {}
    synth.nm_addrs = {"sub_0800AAAA": 0x0800AAAA,   # at its own VMA
                      "sub_0800AAB4": 0x0800AAB8}   # drifted by 4
    check("label at its own address resolves", synth.resolve("sub_0800AAAA"), (True, ""))
    ok_drift, why_drift = synth.resolve("sub_0800AAB4")
    check("drifted label refused", ok_drift, False)
    check("drifted label names its true address", "0x0800aab8" in why_drift, True)
    check("drifted label is not offered as a rename",
          "(rename)" not in why_drift, True)
    # A spelling the assembler never mentions must still fall through to the
    # name-based closure index, so the nm check did not strand unknown labels.
    synth.nm_addrs = {}
    synth.files = [Path("asm/x.s")]
    synth.by_file = {synth.files[0]: {0x0800AABC: ["sub_0800AABC"]}}
    check("unknown spelling falls back to the closure index",
          synth.resolve("sub_0800AABC")[0], True)
    # `index_asm` is the rule that decides who owns a VMA, so pin it in BOTH
    # directions against synthetic source. The accept side is the bug this
    # change fixes: a code label carrying a trailing `@` comment (how
    # asm/carphys_tick.s writes 35 of them) used to be dropped from the index,
    # so every body it could own read as "no closure owner". The reject side
    # is what keeps the fix honest: a DATA label with the same trailing shape
    # must still be refused, or a constant pool starts owning function bodies.
    # Asserting only the accept side would pass a regex widened to `.+`.
    idx_ok, _ = index_asm(
        "_0800A06C:\t\t\t\t@ record 33 -> 15\n"
        "_0800A070:\n"
        "not_a_label: whatever\n"
    )
    check("code label with a trailing @ comment is indexed",
          0x0800A06C in idx_ok, True)
    check("bare label still indexed", 0x0800A070 in idx_ok, True)
    check("trailing @ comment does not shift the name",
          idx_ok.get(0x0800A06C), ["_0800A06C"])
    idx_no, _ = index_asm(
        "_080003B0: .4byte 0x087B04C4\n"
        "_0800240C0: .4byte 0x03000610 @ pool\n"
    )
    check("data label is NOT indexed as a code owner",
          0x080003B0 in idx_no, False)
    check("data label with a trailing comment is still refused",
          0x080240C0 in idx_no, False)
    # Pin the promotable gate in BOTH directions. `promotable` admits `EXACT`,
    # which `alignment_only` alone refused -- that omission discarded every
    # exact body and made the queue read empty while 1424 bytes of byte-perfect
    # C sat unpromoted. A one-sided case would still pass if the gate were
    # simply `return True`, so the PARTIAL cases matter as much as the EXACT
    # one: alignment-only PARTIAL is admitted, a PARTIAL with a real
    # instruction difference is not. Synthetic records, so the cases test the
    # rule and not the current build report.
    class _G(Screen):
        def __init__(self) -> None:      # no build state, no asm scan
            self.nm_addrs = {}
            self.by_file = {}
            self.files = []
            self.name_to_vma = {}
    gate = _G()
    _rom = bytes(16)
    check("EXACT is promotable", gate.promotable(
        {"status": "EXACT", "cand_bytes": 4, "rom_bytes": 4, "matched_bytes": 4,
         "vma": "0x08000000"}, _rom), True)
    check("alignment-only PARTIAL is promotable", gate.promotable(
        {"status": "PARTIAL", "cand_bytes": 8, "rom_bytes": 8, "matched_bytes": 6,
         "prefix": 6, "vma": "0x08000000"}, _rom), True)
    check("PARTIAL with a real difference is not promotable", gate.promotable(
        {"status": "PARTIAL", "cand_bytes": 8, "rom_bytes": 8, "matched_bytes": 4,
         "prefix": 4, "vma": "0x08000000"}, _rom), False)
    # `prefix` is load-bearing, so a record that omits it must be refused rather
    # than promoted on the strength of the byte count alone.
    check("a PARTIAL with no prefix is not promotable", gate.promotable(
        {"status": "PARTIAL", "cand_bytes": 8, "rom_bytes": 8, "matched_bytes": 6,
         "vma": "0x08000000"}, _rom), False)
    check("OVERSIZED is not promotable", gate.promotable(
        {"status": "OVERSIZED", "cand_bytes": 12, "rom_bytes": 8, "matched_bytes": 8,
         "vma": "0x08000000"}, _rom), False)
    check("UNRESOLVED_RELOCATION is not promotable", gate.promotable(
        {"status": "UNRESOLVED_RELOCATION", "cand_bytes": 8, "rom_bytes": 8,
         "matched_bytes": 8, "vma": "0x08000000"}, _rom), False)
    # Pin the owner rule in both directions, on a SYNTHETIC fixture. Asserting
    # against the real closure would make the case track the build:
    # `0x08024138` is a live promotion candidate, so the day someone promotes
    # it this gate would fail for a legitimate reason and the natural "fix"
    # would be to delete the case. `files` and `by_file` are both populated --
    # `owners()` looks each file up in `by_file`, so a fixture missing the
    # matching `files` entry would report every label absent and fail for a
    # harness reason rather than a rule reason.
    own = Screen.__new__(Screen)          # no build state, no asm scan
    own.files = [Path("asm/passthrough.inc"), Path("asm/leaf_a.s"),
                 Path("asm/leaf_b.s")]
    own.by_file = {
        own.files[0]: {0x0800AAAA: ["sub_0800AAAA"],   # dispatcher label only
                       0x0800AAB4: ["sub_0800AAB4"]},  # .inc and both .s name it
        own.files[1]: {0x0800AAB4: ["sub_0800AAB4"]},
        own.files[2]: {0x0800AAB4: ["sub_0800AAB4"]},
    }
    check("a closure .inc owns a VMA no .s labels",
          [p.name for p in own.owners(0x0800AAAA)], ["passthrough.inc"])
    # The reject side, and the sharper one: the `.inc` must not become a second
    # owner here, or every address the dispatcher re-labels turns into a
    # phantom `multi-owner` block. The two `.s` still come back as two, so the
    # multi-owner guard itself is pinned by the same case.
    check("a .s outranks the .inc that names the same VMA",
          [p.name for p in own.owners(0x0800AAB4)], ["leaf_a.s", "leaf_b.s"])
    check("a VMA no closure file labels has no owner",
          own.owners(0x0800AAE0), [])
    # Pin the padding clamp in both directions too. A zero overshoot is
    # forgiven, a real one is not, and a one-sided case would also pass if the
    # check simply refused everything -- so both go in.
    pad = bytearray(0x40)
    check("zero-pad overshoot is padding",
          Screen.pad_only(bytes(pad), ROM_BASE + 0x30, ROM_BASE + 0x34), True)
    check("two-byte alignment overshoot is padding",
          Screen.pad_only(bytes(pad), ROM_BASE + 0x30, ROM_BASE + 0x32), True)
    pad[0x33] = 0x01
    check("overshoot holding real code is not padding",
          Screen.pad_only(bytes(pad), ROM_BASE + 0x30, ROM_BASE + 0x34), False)
    check("overshoot running off the image is not padding",
          Screen.pad_only(bytes(pad), ROM_BASE + 0x3E, ROM_BASE + 0x42), False)
    # The `<stem>_end:` fallback must exist in the file before the entry is
    # stageable. `replace_body` splices start-marker..end-label, so a fallback
    # that is absent either fails to match or -- if one is appended at EOF to
    # make it match -- deletes the whole region after the body. Pin both
    # directions, and pin the whitespace tolerance, because the splicer's own
    # marker regex is `^\s*LABEL\s*$` and an indented anchor is legitimate.
    # These call the SAME module-level function `run()` calls, not a local copy:
    # a private predicate here would leave all cases green even with the guard
    # deleted from `run()`.
    nolines = [".type sub_08001000, %function", "sub_08001000:",
               "\tmovs r0, #1", "\tbx lr"]
    check("a missing _end: fallback blocks the entry",
          has_end_anchor(nolines, "code_fake"), False)
    check("a present _end: fallback admits the entry",
          has_end_anchor(nolines + ["code_fake_end:"], "code_fake"), True)
    check("an indented _end: fallback is still an anchor",
          has_end_anchor(nolines + ["\tcode_fake_end:"], "code_fake"), True)
    check("a same-named non-marker line is not an anchor",
          has_end_anchor(nolines + ["code_fake_end: .word 0"], "code_fake"),
          False)
    check("an _end: named only in a comment is not an anchor",
          has_end_anchor(nolines + ["@ see code_fake_end: below"],
                         "code_fake"), False)
    # The cases above pin the *predicate*. This one pins the *wiring*: it drives
    # `run()` over a synthetic file on disk, so deleting the `has_end_anchor`
    # call in `run()` turns it red. It asserts on the refusal reason by name and
    # negatively that the same fixture WITH the anchor is staged -- a bare "not
    # staged" would also pass if `run()` refused everything.
    #
    # Everything it depends on is synthetic, on purpose. `run()` calls
    # `probe.rom_functions()` for the span and `probe.ROM` for the bytes; if it
    # used the real ones the case would silently depend on a live label -- 0x08004B90
    # really is an inventory start today, and the day someone moves that label
    # the case would fail for a harness reason and get deleted instead of read.
    # The VMA below is deliberately absent from the real inventory. The file
    # must exist on disk (`run()` does `path.read_text()`) and must be listed in
    # BOTH `files` and `by_file`, or the closure helpers report every label
    # absent and the case fails for a harness reason instead.
    with tempfile.TemporaryDirectory(prefix="screen-anchor-") as tmp:
        VMA, END, ROM_BASE_ = 0x08ABCDE0, 0x08ABCDE8, ROM_BASE
        body_bytes = b"\x00\x70" * 4                # all zero: pad_only holds

        class _FakeProbe:
            ROM = None                              # set below; needs .read_bytes()

            @staticmethod
            def rom_functions():
                return {VMA: END}

        fake_rom = Path(tmp) / "fake.bin"
        fake_rom.write_bytes(bytes((ROM_BASE_ + (END - ROM_BASE_)) - ROM_BASE_))
        _FakeProbe.ROM = fake_rom

        real_probe = _probe

        def drive(anchor: bool):
            fx = Path(tmp) / "code_anchor.s"
            body = [".type sub_08ABCDE0, %function", "sub_08ABCDE0:",
                    "\tpush {r4, lr}", "\tbx r0"]
            if anchor:
                body.append("code_anchor_end:")
            # Region end equals the span end exactly, so the pad clamp is not
            # what the case is testing.
            fx.write_text("@ Region: 0x0ABCDE0-0x0ABCDE8 "
                          "(VMA 0x08ABCDE0-0x08ABCDE8).\n"
                          + "\n".join(body) + "\n", encoding="utf-8")
            scr = Screen.__new__(Screen)          # no build state, no asm scan
            scr.files = [fx]
            scr.by_file = {fx: {VMA: ["sub_08ABCDE0"]}}
            scr.branch_targets = set()
            scr.refs = {}
            scr.promoted = {}
            scr.by_vma = {VMA: {"name": "sub_08ABCDE0"}}
            scr.exports, scr.bodies = set(), {}
            scr.stale = []
            scr.report = {"results": [{
                "vma": f"{VMA:#010x}", "name": "sub_08ABCDE0", "rom_bytes": 8,
                "status": "EXACT", "matched_bytes": 8, "cand_bytes": 8,
                "alias_of": None, "call_targets": [],
                "first_diff": None, "pool_words": [],
            }]}
            return scr.run()

        try:
            globals()["_probe"] = lambda: _FakeProbe
            s_no, b_no, _ = drive(False)
            s_yes, _, _ = drive(True)
        finally:
            globals()["_probe"] = real_probe
        check("run() refuses a body whose file has no end anchor",
              any("no end marker" in r[1] for r in b_no), True)
        check("...and the reason names the missing fallback",
              "code_anchor_end:" in b_no[0][1] if b_no else False, True)
        check("...and it stages nothing", [x["vma"] for x in s_no], [])
        check("run() stages the same body once the anchor exists",
              [x["vma"] for x in s_yes], [f"{VMA:#010x}"])
    # The `is_local` filter on `export`: a branch target whose OWN address lies
    # strictly inside the entry's span is an intra-function label (a loop
    # back-edge, an internal jump), not an entry point the retained asm calls
    # from outside. `_080023DE` is `ble _080023DE` at code_22e4.s:147 and
    # nothing outside 0x080023C4's span references it, yet it sat in `export`
    # for 40 manifest entries -- every one an export requirement nothing could
    # satisfy, which the splicer would then have had to invent. Pin both
    # directions on a synthetic fixture. Without this, deleting the filter
    # restores the over-collection across every future entry and the suite
    # stays green, which is exactly how it shipped in the first place.
    with tempfile.TemporaryDirectory(prefix="screen-local-") as tmp:
        VMA2, END2 = 0x08ABCE00, 0x08ABCE10
        # IN_SPAN is a label inside the span (an intra-function jump target).
        # OWN_TWIN is a second spelling of the body's OWN address, which
        # `is_local` must KEEP -- it uses a strict `<`, and such a name is
        # exactly what retained asm calls from outside.
        IN_SPAN, OWN_TWIN = "_08ABCE04", "_08ABCE00"
        # READ_LDR is the shape: an interior label read by an `ldr`
        # literal from OUTSIDE the span. It is never a branch target, so the
        # old branch-only export rule could not see it and only
        # `make matching-ready` found the dangling read. SELF_POOL is the
        # opposite shape -- read only inside its own span -- and must stay
        # local in the same fixture.
        READ_LDR, SELF_POOL = "_08ABCE08", "_08ABCE0C"
        outside = Path("asm/reader_stub.s")

        class _FakeProbe2:
            ROM = None

            @staticmethod
            def rom_functions():
                return {VMA2: END2}

        f2 = Path(tmp) / "fake.bin"
        f2.write_bytes(bytes(0x40))
        _FakeProbe2.ROM = f2

        def drive_export():
            fx = Path(tmp) / "code_local.s"
            fx.write_text(
                "@ Region: 0x0ABCE00-0x0ABCE10 (VMA 0x08ABCE00-0x08ABCE10).\n"
                ".type sub_08ABCE00, %function\nsub_08ABCE00:\n"
                "_08ABCE00:\n\tpush {r4, lr}\n\tbx r0\n"
                f"{IN_SPAN}:\n\tnop\n"
                f"{READ_LDR}:\n\t.word 0\n"
                f"{SELF_POOL}:\n\t.word 0\n"
                "code_local_end:\n", encoding="utf-8")
            scr = Screen.__new__(Screen)
            scr.files = [fx]
            # `needed` is drawn from the file's own label index, so both
            # candidate names must be declared there; `nm_addrs` then says
            # where each one actually assembles.
            scr.by_file = {fx: {VMA2: ["sub_08ABCE00", OWN_TWIN],
                                 VMA2 + 4: [IN_SPAN],
                                 VMA2 + 8: [READ_LDR],
                                 VMA2 + 12: [SELF_POOL]}}
            scr.branch_targets = {IN_SPAN, OWN_TWIN}
            scr.nm_addrs = {IN_SPAN: VMA2 + 4, OWN_TWIN: VMA2,
                            READ_LDR: VMA2 + 8, SELF_POOL: VMA2 + 12}
            # Reads, by DIRECTION: IN_SPAN and SELF_POOL only from lines inside
            # the replaced region; READ_LDR and OWN_TWIN from another file.
            scr.refs = {IN_SPAN: {(fx, 7)}, SELF_POOL: {(fx, 11)},
                        READ_LDR: {(outside, 5)}, OWN_TWIN: {(outside, 6)}}
            scr.promoted = {}
            scr.by_vma = {VMA2: {"name": "sub_08ABCE00"}}
            scr.exports, scr.bodies = set(), {}
            scr.stale = []
            scr.report = {"results": [{
                "vma": f"{VMA2:#010x}", "name": "sub_08ABCE00", "rom_bytes": 16,
                "status": "EXACT", "matched_bytes": 16, "cand_bytes": 16,
                "alias_of": None, "call_targets": [],
                "first_diff": None, "pool_words": [],
            }]}
            return scr.run()

        real_probe2 = _probe
        try:
            globals()["_probe"] = lambda: _FakeProbe2
            st2, _bl2, _ = drive_export()
        finally:
            globals()["_probe"] = real_probe2
        got = st2[0]["entry"].get("export") or []
        check("an intra-span branch label is NOT exported",
              IN_SPAN in got, False)
        check("a twin at the body's OWN VMA is still exported",
              OWN_TWIN in got, True)
        check("an ldr-read interior label from outside IS exported",
              READ_LDR in got, True)
        check("a pool label read only inside its own span is NOT exported",
              SELF_POOL in got, False)
    # `label_is_local` itself, pinned on the three real shapes. The names are
    # the spellings; `nm_addrs` places all of them inside ONE span, so
    # the address half alone cannot decide and only the direction half can.
    lnm = {"_0802BCF6": 0x0802BCF6, "_0802BCFA": 0x0802BCFA,
           "_0802BD00": 0x0802BD00, "_0802BD04": 0x0802BD04,
           "_0802BCF4": 0x0802BCF4}
    lowner = Path("asm/cluster_stub.s")
    lrefs = {
        "_0802BCF6": {(Path("asm/other_stub.s"), 10)},   # trap 7: branch outside
        "_0802BCFA": {(lowner, 3)},                      # trap 3: ldr outside
        "_0802BD00": {(lowner, 25)},                     # self-only pool read
        "_0802BD04": set(),
    }

    def li(n):
        return label_is_local(n, 0x0802BCF4, 0x0802BD14, lnm, lrefs,
                              lowner, (20, 30))

    check("the _0802BCF6 shape: a branch from outside is not local",
          li("_0802BCF6"), False)
    check("the _0802BCE4 shape: an ldr from outside is not local",
          li("_0802BCFA"), False)
    check("the _0802BE74 shape: a self-only read is local",
          li("_0802BD00"), True)
    check("an unreferenced interior label is local by default",
          li("_0802BD04"), True)
    check("the entry's own VMA is never local", li("_0802BCF4"), False)
    check("a name nm cannot place is never local", li("_08000002"), False)
    # `alignment_only` must key on `prefix`, not on `matched_bytes` -- the latter
    # is a COUNT of equal bytes, so an interior difference with a zero tail
    # satisfies `matched + 2 == rom` just as well as a real pad miss. Pin both
    # directions on synthetic records, or the rule could pass by refusing
    # everything and a genuine pad-only body could regress unnoticed.
    al = Screen.__new__(Screen)
    rom8 = bytes(8)

    def part(**kw):
        base = {"status": "PARTIAL", "cand_bytes": 8, "rom_bytes": 8,
                "matched_bytes": 6, "prefix": 6, "vma": f"{ROM_BASE:#010x}"}
        base.update(kw)
        return base

    check("a genuine trailing-pad miss is alignment-only",
          al.alignment_only(part(), rom8), True)
    # The _0800CF60 shape: two interior bytes differ, so the count is 6 and the
    # prefix is 1, yet the tail is zero. The count said yes; the prefix must.
    check("an interior difference with a zero tail is NOT alignment-only",
          al.alignment_only(part(prefix=1, matched_bytes=6), rom8), False)
    check("a record with no prefix field is refused, not assumed exact",
          al.alignment_only(part(prefix=None, matched_bytes=6), rom8), False)
    check("a non-zero trailing pad is not alignment-only",
          al.alignment_only(part(), bytes(6) + b"\x01\x02"), False)
    check("a size mismatch is refused",
          al.alignment_only(part(cand_bytes=9), rom8), False)
    check("an EXACT record is not routed through the pad test",
          al.alignment_only(part(status="EXACT"), bytes(0xFF) * 8), False)
    # The staleness rule reads the real src/ tree, so pin it against a synthetic
    # report whose mtime is set explicitly -- never by touching src/. A missing
    # report is absent, not stale, and must not be reported as stale.
    with tempfile.TemporaryDirectory(prefix="screen-stale-") as tmp:
        rpt = Path(tmp) / "report.json"
        rpt.write_text("{}", encoding="utf-8")
        check("a report newer than every src file is not stale",
              stale_sources(rpt), [])
        oldest = min(p.stat().st_mtime for p in (ROOT / "src").glob("*.c"))
        os.utime(rpt, (oldest - 60, oldest - 60))
        check("a report older than src is stale", len(stale_sources(rpt)) > 0, True)
        check("stale entries name src/ or asm/ inputs",
              all(p.startswith(("src/", "asm/")) for p in stale_sources(rpt)), True)
        # A byte-neutral asm label move invalidates every call_targets entry
        # naming it, so asm/ has to be watched as well as src/.
        asm_newest = max(p.stat().st_mtime for p in
                         [*_iter(ROOT / "asm", "*.s"), *_iter(ROOT / "asm", "*.inc")])
        os.utime(rpt, (asm_newest - 60, asm_newest - 60))
        check("an asm-only change also stales the report",
              any(p.startswith("asm/") for p in stale_sources(rpt)), True)
        rpt.unlink()
        check("a missing report is absent, not stale", stale_sources(rpt), [])
    # The staleness POLICY, by mode. The read-only path must keep answering --
    # with a set of concurrent edits editing src/, the report goes stale within minutes
    # and a refusal there would make the tool unusable. The mutating path must
    # fail, because it writes the manifest from those verdicts. Pin both, or the
    # policy collapses to whichever side happens to be implemented.
    check("a fresh report needs no verdict", stale_verdict([], staging=False), "")
    check("a fresh report needs no verdict when staging", stale_verdict([], staging=True), "")
    check("a stale dry run warns and continues", stale_verdict(["a.c"], False), "warn")
    check("a stale --stage refuses", stale_verdict(["a.c"], True), "refuse")
    check("many stale files still refuse when staging",
          stale_verdict([f"f{i}.c" for i in range(40)], True), "refuse")
    # The code.o staleness guard `_nm_addrs` consults is the SHARED one -- the
    # probe's own code.o reads must use the same mtime rule and wording -- so
    # pin the binding rather than a copy of the logic: a local redefinition
    # would silently fork the rule one tool at a time.
    import corpus_match_probe
    check("the code.o staleness guard is the probe's shared one",
          code_o_staleness is corpus_match_probe.code_o_staleness, True)
    with tempfile.TemporaryDirectory(prefix="screen-codeo-") as tmp:
        obj = Path(tmp) / "code.o"
        obj.write_text("x", encoding="utf-8")
        os.utime(obj, (100, 100))
        src = Path(tmp) / "a.s"
        src.write_text("y", encoding="utf-8")
        os.utime(src, (50, 50))
        check("the shared guard reads a fresh object as fresh",
              code_o_staleness(obj, [src]), "")
        os.utime(src, (150, 150))
        check("...and a stale one as a named warning",
              "code.o" in code_o_staleness(obj, [src]), True)

    # NOTE: a live "every called spelling is exported" gate was written here and
    # removed before landing. Matching call text against an entry's `c_name` stem
    # conflates distinct bodies that share a stem -- `sub_080022CC` and
    # `_080022CC` are different functions, so a text search for either finds
    # calls to the other, and it reported 245 phantom gaps on a manifest with 2
    # real ones. The defect it hunted is real and was hit in 0x08009F58
    # and 0x080055F4 each needed their `sub_` spelling exported, or the slice link
    # emits a ROM veneer that lands on the same address and leaves every
    # byte-level gate green while the call runs the ROM body. But a text search
    # cannot be the detector. A sound version resolves each `alias("target")` in
    # the owning TU to its body and compares the body's two spellings, instead of
    # searching call sites by name.

    print(f"promotion_screen self-test: {ok}/{total}")
    return 0 if ok == total else 1


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--stage", action="store_true",
                        help="write surviving candidates into the manifest")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        return self_test()

    screen = Screen()
    screen.stale = stale_sources(REPORT)
    verdict = stale_verdict(screen.stale, args.stage)
    if verdict:
        head = ("REFUSING to stage" if verdict == "refuse"
                else "WARNING: report is stale")
        print(f"{head}: build/era-corpus/ready-report.json is older than "
              f"{len(screen.stale)} src/ or asm/ input(s) -- "
              f"{', '.join(screen.stale[:4])}"
              + (" ..." if len(screen.stale) > 4 else ""), file=sys.stderr)
        print("  call_targets come from the COMPILED corpus objects, so this "
              "screen grades pre-edit state and a correct fix reads as no "
              "progress. Refresh it first:\n"
              "  python3 tools/corpus_match_probe.py --c89 --require-all "
              "--work-dir build/era-corpus/ready-work "
              "--json build/era-corpus/ready-report.json", file=sys.stderr)
    staged, blocked, counts = screen.run()
    total = sum(c["bytes"] for c in staged)
    print(f"closure: {len(screen.files)} files, {len(screen.branch_targets)} branch targets")
    print(f"CONSIDERED {counts['considered']} / already-promoted {counts['promoted']} / "
          f"needs-source-match {counts['not-promotable']} / "
          f"survived {counts['considered'] - counts['promoted'] - counts['not-promotable']}")
    print(f"\nSTAGEABLE: {len(staged)} / {total} bytes")
    for cand in staged:
        mark = " [needs end anchor]" if cand["entry"].get("_needs_anchor") else ""
        print(f"  {cand['vma']} {cand['bytes']:>3}B {cand['entry']['asm_file']:<26} "
              f"{cand['entry']['c_name']}"
              + (f"  export={cand['entry']['export']}" if cand["entry"].get("export") else "")
              + mark)
    print(f"\nBLOCKED: {len(blocked)}")
    for vma, why in blocked:
        print(f"  {vma} {why}")

    if not args.stage:
        print("\n(dry run; pass --stage to write the manifest)")
        return 0
    if verdict == "refuse":
        return 1
    for cand in staged:
        entry = {k: v for k, v in cand["entry"].items() if not k.startswith("_")}
        screen.promoted[cand["vma"]] = entry
    screen.promoted = {k: screen.promoted[k]
                       for k in sorted(screen.promoted, key=lambda k: int(k, 16))}
    MANIFEST.write_text(json.dumps(screen.promoted, indent=2) + "\n", encoding="utf-8")
    print(f"\nstaged {len(staged)}; manifest now {len(screen.promoted)} entries / "
          f"{sum(int(v['end_vma'], 16) - int(k, 16) for k, v in screen.promoted.items())} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
