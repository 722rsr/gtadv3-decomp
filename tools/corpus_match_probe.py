#!/usr/bin/env python3
"""Corpus byte-match probe: does agbcc reproduce the *real* lifted C?

The de-risk probes (`tools/derisk_probe.py`, `tools/derisk_pool_probe.py`) tested
hand-written reconstructions of a dozen leaves. That is evidence the compiler can
match, but it is not the question that gates the migration. The question is what
happens when the project's own C -- written to be *behaviourally* faithful, not
shaped for GCC 2.95's register allocator -- is compiled and compared byte-for-byte
against the ROM.

This tool does that for the translation units agbcc accepts today, i.e. those
with no C99 constructs needing a transform. That is a deliberate, self-limiting
scope: the C89 transform is unfinished, so rather than wait for it, this measures
the byte-match rate on the subset that compiles, which is enough to decide whether
the migration has a payoff.

Method, per function:
  * compile the TU with agbcc, `-ffunction-sections`, at the function's real ROM VMA
  * resolve that function's section, follow the R_ARM_THM_CALL relocations against
    the ROM so calls and their literal loads land at the real addresses
  * compare against the ROM span the function occupies
  * classify the miss: register allocation, instruction order/selection, size

A function counts as a match only on an exact whole-body comparison including its
literal pool. Anything else is a miss, classified rather than dismissed.
"""
from __future__ import annotations

import argparse
import collections
import concurrent.futures
import importlib.util
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from collections.abc import Iterable
from functools import lru_cache
from pathlib import Path

import agbcc_c89_transform as c89
import export_audit

ROOT = Path(__file__).resolve().parents[1]
ROM = ROOT / "baserom.gba"
WORK = ROOT / "build/era-corpus/corpus"
# Adopted : the ROM's 90 frameless branchy bodies prove stock-2.95
# framing, and the whole 690/26538 promoted slice is byte-identical under this
# build (docs/findings/compiler_split.md). `agbcc` next to it is the newer build
# that frames every branchy body; keep it only as the comparison baseline.
AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"
INCLUDE = [ROOT / "include", ROOT / "asm", ROOT / "build/toolchains/agbcc/ginclude"]
OPTS = ("-O1", "-O2", "-Os")
ROM_BASE = 0x08000000
CODE_END = 0x0802E158

# ---------------------------------------------------------------------------
# Translation-unit compile cache.
#
# `match_c_slice.py` runs this probe once per *promoted body*, in a private
# work dir, and each of those runs redid the C89 transform, the clang
# preprocess, the agbcc compile and the assemble -- for the whole TU, whose
# output is byte-identical for every body in it. Measured on a 268-body
# manifest: 275 compiles for 56 distinct TUs, 4.9x redundant, and that
# redundancy *is* the 13-minute link.
#
# Only the agbcc + `as` half is cached, and it is keyed on the **preprocessed**
# text rather than the .c file. That distinction is the whole correctness of
# the cache: every TU here `#include`s from `include/` and `asm/`, so a header
# edit leaves the .c bytes untouched. Keyed on the .c, a header change would
# serve pre-edit objects for every affected TU -- and a stale object still
# scores cleanly, so the failure is a confident wrong answer, not a crash.
# `clang -E` is the cheap step and runs unconditionally; agbcc is the
# expensive one. The key also folds in agbcc's own identity and the flag
# string, so a toolchain change misses rather than serving old output.
TU_CACHE = ROOT / "build/era-corpus/tu-cache"

# The path field of a clang linemarker. Normalising it is what makes the key
# work-dir independent; see _tu_cache_key.
_LINEMARKER_PATH = re.compile(rb'(?m)^(# \d+ ")[^"]*(")')

def _tu_cache_key(pre: Path, use_c89: bool) -> str:
    """Content address for one TU's agbcc output.

    `pre` is the preprocessed source, so every `#include` is already folded in.
    The linemarker *paths* are normalised out and nothing else: clang writes
    `# 1 "<abs path of the input>"` at every file boundary, and match_c_slice
    gives each body its own `--work-dir`, so every one of the 275 runs would
    otherwise get a different key from the same bytes. Line numbers are left
    alone -- agbcc consumes them, and they are part of the input. Header
    *content* still lands in the key, so a header edit misses as it must.
    """
    h = hashlib.sha256()
    h.update(_LINEMARKER_PATH.sub(rb"\1<P>\2", pre.read_bytes()))
    h.update(b"\0c89=" + str(use_c89).encode())
    h.update(b"\0opts=-O2 -mthumb-interwork -ffunction-sections")
    for tool in (AGBCC, Path(__file__), ROOT / "tools/agbcc_c89_transform.py"):
        h.update(b"\0" + str(tool.name).encode() + b"=")
        h.update(str(tool.stat().st_mtime_ns if tool.exists() else 0).encode())
    return h.hexdigest()[:32]


def tu_cache_lookup(pre: Path, use_c89: bool, dest_s: Path, dest_o: Path) -> bool:
    """Copy a cached TU's agbcc output into `dest_s`/`dest_o`; True on a hit."""
    entry = TU_CACHE / _tu_cache_key(pre, use_c89)
    if not (entry / "out.s").exists() or not (entry / "out.o").exists():
        return False
    dest_s.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(entry / "out.s", dest_s)
    shutil.copyfile(entry / "out.o", dest_o)
    return True


def tu_cache_store(pre: Path, use_c89: bool, src: Path, obj: Path) -> None:
    entry = TU_CACHE / _tu_cache_key(pre, use_c89)
    entry.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(src, entry / "out.s")
    shutil.copyfile(obj, entry / "out.o")

cached_transform_file = c89.cached_transform_file

# A function definition in src/: a VMA-shaped name bound to a body.
VMA_DEF = re.compile(
    r"^(?:(?:static|extern|inline|const|volatile|unsigned|signed|void|char|short|int|long|u8|u16|u32|s8|s16|s32|bool|bool8|bool32)\s+|__attribute__\(\([^)]+\)\)\s+)+"
    r"\*?\s*(?P<name>(?:[Ss]ub_)?_?0*80[0-9A-Fa-f]{4,7}(?:_[A-Za-z0-9]+)?)\s*\(", re.M)

VMA_TOKEN = re.compile(r"(?<![0-9A-Fa-f])0*80([0-9A-Fa-f]{4,7})(?![0-9A-Fa-f])")


@lru_cache(maxsize=4096)
def parse_vma_name(name: str) -> int | None:
    """Normalize both `_08028250` and `_080028250` to the same real VMA."""
    match = VMA_TOKEN.search(name)
    if not match:
        return None
    offset = int(match.group(1), 16)
    return ROM_BASE + offset if 0 <= offset <= CODE_END - ROM_BASE else None

# A VMA may have several definitions across the corpus; the one that matters is
# the strong body, not a `__attribute__((weak))` placeholder that returns a
# constant. Measuring the weak one reports a miss the real body does not have.
WEAK_DEF = re.compile(r"__attribute__\(\(weak\)\)")

RUNTIME_VMAS: dict[str, int] = {
    "_call_via_r0": 0x0802DDC8,
    "_call_via_r1": 0x0802DDCC,
    "_call_via_r2": 0x0802DDD0,
    "_call_via_r3": 0x0802DDD4,
    "_call_via_r4": 0x0802DDD8,
    "_call_via_r5": 0x0802DDDC,
    "_call_via_r6": 0x0802DDE0,
    "_call_via_r7": 0x0802DDE4,
    "_call_via_r8": 0x0802DDE8,
    "_call_via_r9": 0x0802DDEC,
    "_call_via_sl": 0x0802DDEC,
    "_call_via_r10": 0x0802DDF0,
    "_call_via_fp": 0x0802DDF0,
    "_call_via_r11": 0x0802DDF4,
    "_call_via_ip": 0x0802DDF4,
    "_call_via_r12": 0x0802DDF8,
    "_call_via_sp": 0x0802DDFC,
    "_call_via_lr": 0x0802DE00,
    "__divsi3": 0x0802DE04,
    "DivSI": 0x0802DE04,
    "__divsi3_from_thumb": 0x0802DE04,
    "__div0": 0x0802DE98,
    "__modsi3": 0x0802DE9C,
    "ModSI": 0x0802DE9C,
    "__modsi3_from_thumb": 0x0802DE9C,
    "__udivsi3": 0x0802DF6C,
    "__udivsi3_from_thumb": 0x0802DF6C,
    "__umodsi3": 0x0802DFE4,
    "__umodsi3_from_thumb": 0x0802DFE4,
    "memcpy": 0x0802E0A4,
    "_0802E0A4": 0x0802E0A4,
}

MANUAL_HINTS: dict[str, str] = {
    "SubBroadcast": "08004D4C",
    "EventPost": "08025BF0",
    "EventBind": "08007ABC",
    "HeapReset": "08004A0C",
    "Course_0x08025CF4_gap": "08025CF4",
    "Course_0x08007770": "08007770",
    "Course_0x0800798C": "0800798C",
    "Course_0x08007A58": "08007A58",
    "Course_0x0800DAB8": "0800DAB8",
    "Course_0x080263E0": "080263E0",
    "Course_0x080267FC": "080267FC",
    "Tick_0x0800AA20": "0800AA20",
    "Tick_0x0800AA40": "0800AA40",
    "Rec35_Leaf_16508": "08016508",
    "Sound_0x0802B3B8": "0802B3B8",
    "RaceScene_B364": "0801B364",
    "UiPacket_Consume": "080188B0",
    "Rec35_StageA": "08015ED0",
    "Rec35_StageB": "08015F68",
    "GuardedSaveOnly": "08024BC0",
    "GuardedFullSave": "08024B70",
    "ScenePost": "08002618",
    "Sound_Cmd3": "0802B3B8",
}

# One C body has no VMA-shaped alias because it is used only inside its TU.
FRIENDLY_SOURCE_VMAS = {"CarDisplay_21860": 0x08021860}


def run(argv, stdout=None, stderr=subprocess.PIPE):
    kwargs = {"text": True, "stderr": stderr}
    kwargs["stdout"] = stdout if stdout is not None else subprocess.PIPE
    return subprocess.run([str(a) for a in argv], **kwargs)


def inc_flags():
    out = []
    for d in INCLUDE:
        if d.is_dir():
            out += ["-I", str(d)]
    return out


@lru_cache(maxsize=1)
def alias_targets() -> dict[str, str]:
    """VMA-named symbol -> the friendly name it is an alias of.

    `int _0802D974(void) __attribute__((alias("CpuSet_2D974")));` makes the VMA
    name an alias of the friendly body, and agbcc emits the section under the
    friendly name. Without this map most VMA-named functions look missing.
    """
    out: dict[str, str] = {}
    alias_re = re.compile(
        r"(_?0*80[0-9A-Fa-f]{4,7})\s*\([^)]*\)\s*__attribute__\(\(alias\(\"([A-Za-z_]\w*)\"\)\)\)")
    for f in sorted((ROOT / "src").glob("*.c")):
        text = f.read_text(encoding="utf-8", errors="replace")
        if "alias" not in text:
            continue
        for m in alias_re.finditer(text):
            out.setdefault(m.group(1), m.group(2))
    return out


@lru_cache(maxsize=1)
def scan_decl_hints() -> dict[str, int]:
    """Scan src/*.c and include/**/*.h for `extern NAME(...); // <VMA>` decl comments."""
    hints: dict[str, int] = {}
    decl_re = re.compile(r"extern\s+[^;]*?\**(\w+)\s*\([^;]*\)\s*;\s*//\s*(.+)$")
    vma_re = re.compile(r"(?:sub_|Sub_|SUB_)?_?(0*80[0-9a-fA-F]{4,7})\b")
    search_dirs = [ROOT / "src", ROOT / "include"]
    for sdir in search_dirs:
        if not sdir.is_dir():
            continue
        for f in sorted(sdir.rglob("*.[ch]")):
            text = f.read_text(encoding="utf-8", errors="replace")
            if "//" not in text or "extern" not in text:
                continue
            for line in text.splitlines():
                if "//" not in line or "extern" not in line:
                    continue
                m = decl_re.search(line)
                if not m:
                    continue
                name, comment = m.group(1), m.group(2)
                vm = vma_re.search(comment)
                if vm:
                    vma = parse_vma_name(vm.group(1))
                    if vma is not None:
                        hints.setdefault(name, vma)
    for name, hex_str in MANUAL_HINTS.items():
        hints.setdefault(name, int(hex_str, 16))
    return hints


def closure_sources() -> list[Path]:
    """What `build-code/code.o` is built from: the `asm/code.s` include closure.

    Reused from `export_audit` rather than re-globbed so the staleness rule and
    the closure rule cannot disagree -- `(deleted: asm/ghost2.s)` is dead weight the build
    never includes, and aging the object on it would be a warning that is a lie.
    """
    return export_audit.closure_files()


def code_o_staleness(obj: Path, sources: Iterable[Path]) -> str:
    """The staleness warning for a closure object, or `""` when it is fresh.

    `build-code/code.o` is a build artifact rebuilt from `asm/`, and several
    tools resolve symbol addresses through it: this probe's call-target
    resolution and `promotion_screen`'s label placement both read it. After an
    `asm/*.s` edit it is out of date until rebuilt, and every one of those
    readers silently answers with pre-edit addresses -- the confident wrong
    verdict this repo has paid for most often. One shared guard, called with
    whatever closure each tool considers its inputs, keeps the mtime rule and
    the wording from drifting apart.

    A missing object answers `""`: the callers have their own, more specific
    "no closure object" path, and duplicating it here would only be noise.
    """
    try:
        if not obj.exists():
            return ""
        stale = obj.stat().st_mtime < max(
            (f.stat().st_mtime for f in sources), default=0.0)
    except OSError:
        return ""
    if not stale:
        return ""
    shown = obj
    try:
        shown = obj.relative_to(ROOT)
    except ValueError:
        pass
    return (f"WARNING: {shown} is older than the newest closure file; run "
            f"`make build-code/code.o` or symbol addresses are pre-edit")


@lru_cache(maxsize=1)
def load_code_symbols() -> dict[str, int]:
    """Read symbols and addresses from build-code/code.o or build/rom.o."""
    syms: dict[str, int] = {}
    for obj_path in (ROOT / "build-code/code.o", ROOT / "build/rom.o"):
        if obj_path.exists():
            warn = code_o_staleness(obj_path, closure_sources())
            if warn:
                print(warn, file=sys.stderr)
            out = run(["arm-none-eabi-nm", str(obj_path)]).stdout or ""
            for line in out.splitlines():
                parts = line.split()
                if len(parts) >= 3:
                    try:
                        addr = int(parts[0], 16)
                        # nm prints address relative to section start (0) for relocatable object
                        syms[parts[2]] = ROM_BASE + addr
                    except ValueError:
                        pass
            if syms:
                break
    return syms


class SymbolResolver:
    """Resolves any symbol name (cross-TU, alias, runtime, hint) to its ROM VMA."""

    def __init__(self, code_syms: dict[str, int], aliases: dict[str, str], hints: dict[str, int]):
        self.code_syms = code_syms
        self.aliases = aliases
        self.inv_aliases = {v: k for k, v in aliases.items()}
        self.hints = hints
        self._cache: dict[str, int | None] = {}

    def resolve(self, sym: str) -> int | None:
        if not sym:
            return None
        if sym in self._cache:
            return self._cache[sym]
        v = self._resolve_uncached(sym)
        self._cache[sym] = v
        return v

    def _resolve_uncached(self, sym: str) -> int | None:
        # 1. Symbol table from code.o / rom.o
        if sym in self.code_syms:
            return self.code_syms[sym]
        # 2. Known runtime symbols
        if sym in RUNTIME_VMAS:
            return RUNTIME_VMAS[sym]
        # 3. Dynamic _call_via_rX
        if sym.startswith("_call_via_"):
            reg = sym[len("_call_via_"):]
            reg_map = {"r0": 0, "r1": 1, "r2": 2, "r3": 3, "r4": 4, "r5": 5, "r6": 6,
                       "r7": 7, "r8": 8, "r9": 9, "sl": 9, "r10": 10, "fp": 10,
                       "r11": 11, "ip": 11, "r12": 12, "sp": 13, "lr": 14}
            if reg in reg_map:
                return 0x0802DDC8 + reg_map[reg] * 4
        # 4. Decl-comment hints
        if sym in self.hints:
            return self.hints[sym]
        # 5. Inverted alias lookup (e.g. friendly name -> VMA name)
        if sym in self.inv_aliases:
            vma_name = self.inv_aliases[sym]
            if vma_name in self.code_syms:
                return self.code_syms[vma_name]
            v = self._parse_vma(vma_name)
            if v is not None:
                return v
        # 6. Direct alias lookup
        if sym in self.aliases:
            target_name = self.aliases[sym]
            if target_name in self.code_syms:
                return self.code_syms[target_name]
            if target_name in self.hints:
                return self.hints[target_name]
        # 7. Embedded VMA in name
        return self._parse_vma(sym)

    @staticmethod
    def _parse_vma(sym: str) -> int | None:
        return parse_vma_name(sym)


def _rom_functions_scan() -> dict[int, int]:
    """VMA -> end, for every function entry the project knows about.

    The end is the next known entry, but only when that is plausible: an alias
    declaration is not a real body, so a 4-byte "function" whose next neighbour
    starts 4 bytes later must be given the extent of the *real* function it
    belongs to. An independently typed or called entry exactly two bytes later
    is different: the earlier entry can be a one-instruction prefix that falls
    through into a shared tail, and extending it across the next entry makes
    those two bodies overlap.
    """
    # Reuse the project's stronger function inventory: typed entry labels and
    # labels with real BL callers. Typed-only scanning misses short untyped
    # functions such as _080028250 and can silently understate their span.
    spec = importlib.util.spec_from_file_location("gtadv_coverage", ROOT / "tools/coverage.py")
    assert spec and spec.loader
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    # Deliberately NOT filtered by prologue here. A `bx lr` leaf with no push is
    # a legitimate 4-byte function and several are promoted: _08004324,
    # _080022D20, and the six Garage stubs. The push-with-lr test belongs in the
    # unlabelled-entries CENSUS (which is looking for something to label), not
    # in the inventory, which must not shrink a promoted span. A `bl` target
    # that is really an interior branch point is handled by the scoped
    # `known <= sourced` check instead -- see self_test.
    starts = {ROM_BASE + int(v, 16) for values in module.asm_vmas(str(ROOT / "asm")).values()
              for v in values}
    ordered = sorted(starts)
    spans: dict[int, int] = {}
    for i, vma in enumerate(ordered):
        end = ordered[i + 1] if i + 1 < len(ordered) else CODE_END
        minimum = 2 if end == vma + 2 else 4
        spans[vma] = max(end, vma + minimum)
    return spans


# Both of these rescan the whole tree on every call: `src_functions` re-reads all
# 152 C files (0.635 s) and `rom_functions` re-scans all of `asm/` (0.098 s).
# That is 0.73 s of the 0.86 s fixed per-process cost, and it is pure waste
# inside one process -- nothing between calls can change the answer. Split into
# a scan and a cached public wrapper. The wrapper hands back a COPY because
# callers treat the result as theirs; returning the cached list itself would let
# one caller mutate the cache for every later caller.
def rom_functions() -> dict[int, int]:
    """VMA -> end for every known function entry. Memoised: see the note above."""
    return dict(_rom_functions_cached())


def src_functions(filter_wanted: list[str] | None = None) -> list[tuple[Path, str, int, int, bool]]:
    """(file, name, vma, body_lines, weak) per VMA-named src/ function. Memoised."""
    if filter_wanted:
        return _src_functions_filtered(filter_wanted)
    return list(_src_functions_cached())


@lru_cache(maxsize=1)
def _rom_functions_cached() -> dict[int, int]:
    cache_file = ROOT / "build/era-corpus/rom_spans.json"
    asm_files = sorted(Path(ROOT / "asm").glob("*.s")) + sorted(Path(ROOT / "asm").glob("*.inc"))
    newest_mtime = max((f.stat().st_mtime_ns for f in asm_files), default=0)
    total_size = sum(f.stat().st_size for f in asm_files)
    cov_tool = ROOT / "tools/coverage.py"
    cov_mtime = cov_tool.stat().st_mtime_ns if cov_tool.exists() else 0
    probe_mtime = Path(__file__).stat().st_mtime_ns
    sig = f"{len(asm_files)}:{total_size}:{newest_mtime}:{cov_mtime}:{probe_mtime}"

    if cache_file.exists():
        try:
            cached_data = json.loads(cache_file.read_text(encoding="utf-8"))
            if cached_data.get("sig") == sig:
                return {int(k): v for k, v in cached_data["spans"].items()}
        except Exception:
            pass

    spans = _rom_functions_scan()
    try:
        cache_file.parent.mkdir(parents=True, exist_ok=True)
        cache_file.write_text(json.dumps({"sig": sig, "spans": spans}), encoding="utf-8")
    except Exception:
        pass
    return spans


@lru_cache(maxsize=1)
def _src_functions_cached() -> list[tuple[Path, str, int, int, bool]]:
    return _src_functions_scan()


ALIAS_DEF = re.compile(
    r"(?<![A-Za-z0-9_])((?:[Ss]ub_)?_?0*80[0-9A-Fa-f]{4,7}(?:_[A-Za-z0-9]+)?)\s*\([^)]*\)\s*__attribute__\(\(alias\(\"([A-Za-z_]\w*)\"\)\)\)")


def _find_target_fast(text: str, target: str) -> int | None:
    """Find position right after '{' of function definition 'target(...) {'."""
    tlen = len(target)
    start = 0
    t_len_text = len(text)
    while True:
        idx = text.find(target, start)
        if idx == -1:
            return None
        start = idx + tlen
        # check left boundary
        if idx > 0 and (text[idx - 1].isalnum() or text[idx - 1] == "_"):
            continue
        # check right boundary
        if start < t_len_text and (text[start].isalnum() or text[start] == "_"):
            continue
        # scan for '('
        p = start
        while p < t_len_text and text[p] in " \t\r\n":
            p += 1
        if p >= t_len_text or text[p] != "(":
            continue
        p2 = text.find(")", p + 1)
        if p2 == -1:
            continue
        p3 = p2 + 1
        while p3 < t_len_text and text[p3] in " \t\r\n":
            p3 += 1
        if p3 < t_len_text and text[p3] == "{":
            return p3 + 1
    return None


def _src_functions_scan(files: list[Path] | None = None) -> list[tuple[Path, str, int, int, bool]]:
    """(file, name, vma, body_lines, weak) for every VMA-named function in src/.

    `body_lines` is the function's own statement count, used to separate real
    transcriptions from placeholder stubs. Many VMAs in the corpus are one-line
    stand-ins (`int CarRacer_Return49(void){ return 49; }`) sitting where the ROM
    holds a substantial function; those cannot byte-match by construction and
    would drag the hit rate down for a reason that says nothing about agbcc.
    """
    out = []
    if files is None:
        files = sorted((ROOT / "src").glob("*.c"))
    friendly_patterns = {
        name: (vma, re.compile(r"^[^\n;{}]*\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*\{", re.M))
        for name, vma in FRIENDLY_SOURCE_VMAS.items()
    }
    for f in files:
        text = f.read_text(encoding="utf-8", errors="replace")
        # 1. Alias definitions: void _0800B190(void) __attribute__((alias("Race_Dispatch")));
        if "alias" in text:
            target_cache: dict[str, int] = {}
            for m in ALIAS_DEF.finditer(text):
                vma_name = m.group(1)
                target = m.group(2)
                vma = parse_vma_name(vma_name)
                if vma is not None and vma > ROM_BASE:
                    if target in target_cache:
                        bsize = target_cache[target]
                    else:
                        tend = _find_target_fast(text, target)
                        bsize = _body_size(text, tend - 1) if tend is not None else 1
                        target_cache[target] = bsize
                    out.append((f, vma_name, vma, bsize, False))

        # 2. Direct VMA function definitions
        for m in VMA_DEF.finditer(text):
            vma = parse_vma_name(m.group("name"))
            if vma is None or vma == ROM_BASE:
                continue
            # Skip extern declarations
            prefix_line = text[max(0, m.start() - 30):m.start()]
            if "extern" in prefix_line:
                continue
            bsize = _body_size(text, m.end())
            if bsize == 0:
                continue
            # VMA_DEF consumes leading attributes, so weak may be inside the
            # match itself rather than in the preceding source window.
            weak = bool(WEAK_DEF.search(text[max(0, m.start() - 80):m.end()]))
            out.append((f, m.group("name"), vma, bsize, weak))

        # 3. Friendly names
        for name, (vma, pattern) in friendly_patterns.items():
            if name in text:
                for m in pattern.finditer(text):
                    bsize = _body_size(text, m.end() - 1)
                    out.append((f, name, vma, bsize, False))
    return out


def _src_functions_filtered(wanted: list[str]) -> list[tuple[Path, str, int, int, bool]]:
    aliases = alias_targets()
    inv_aliases = {v: k for k, v in aliases.items()}
    needles = set()
    for w in wanted:
        needles.add(w)
        needles.add(w.lstrip("_"))
        vma = parse_vma_name(w)
        if vma:
            needles.add(f"{vma:X}")
            needles.add(f"{vma:08X}")
            needles.add(f"{vma:x}")
            needles.add(f"{vma:08x}")
            for fname, fvma in FRIENDLY_SOURCE_VMAS.items():
                if fvma == vma:
                    needles.add(fname)
        if w in aliases:
            needles.add(aliases[w])
        if w in inv_aliases:
            needles.add(inv_aliases[w])
        if w in FRIENDLY_SOURCE_VMAS:
            needles.add(w)

    byte_needles = [n.lower().encode("ascii", errors="ignore") for n in needles if n]
    candidates = []
    for f in sorted((ROOT / "src").glob("*.c")):
        raw = f.read_bytes().lower()
        if any(bn in raw for bn in byte_needles):
            candidates.append(f)
    if not candidates:
        candidates = sorted((ROOT / "src").glob("*.c"))
    return _src_functions_scan(candidates)


def _body_size(text: str, start: int, cap: int = 40) -> int:
    """Rough statement count of a function body, from the brace after `start`.

    Returns 0 if this is an extern declaration (a semicolon appears before the
    opening brace).
    """
    open_idx = text.find("{", start)
    if open_idx < 0:
        return 0
    semi_idx = text.find(";", start)
    if 0 <= semi_idx < open_idx:
        return 0
    depth = 0
    body = 0
    chunk = text[open_idx:open_idx + 4000]
    for ch in chunk:
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                break
        elif ch == ";":
            body += 1
            if body > cap:
                break
    return max(body, 1)


def encode_thumb_bl(insn_vma: int, target_vma: int) -> bytes:
    """Encode a 32-bit Thumb-1 (ARMv4T) `bl` instruction calling `target_vma`."""
    disp = target_vma - (insn_vma + 4)
    high = 0xF000 | ((disp >> 12) & 0x7FF)
    low = 0xF800 | ((disp >> 1) & 0x7FF)
    return (high | (low << 16)).to_bytes(4, "little")


def encode_thumb_b(insn_vma: int, target_vma: int) -> bytes:
    """Encode a 16-bit Thumb-1 unconditional `b` branching to `target_vma`.

    armcc's tail merges emit this as `b.n <interior label>` (relocation
    R_ARM_THM_JUMP11, ELF type 102). agbcc performs no sibling calls at all,
    so a body that tail-branches can only spell the branch in a transcribed
    body -- and the probe must resolve the displacement like it does `bl`.
    """
    disp = target_vma - (insn_vma + 4)
    if disp & 1 or not -4096 <= disp <= 4094:
        raise ValueError(f"b target {target_vma:#x} out of range from {insn_vma:#x}")
    return (0xE000 | ((disp >> 1) & 0x7FF)).to_bytes(2, "little")


def thumb_bl_target(halfwords: list[int], pc_index: int, base_vma: int = 0) -> int | None:
    """Decode a 32-bit Thumb-1 (ARMv4T) `bl` at halfword index `pc_index`."""
    if pc_index + 1 >= len(halfwords):
        return None
    h1, h2 = halfwords[pc_index], halfwords[pc_index + 1]
    if (h1 & 0xF800) != 0xF000 or (h2 & 0xF800) != 0xF800:
        return None
    imm11_high = h1 & 0x7FF
    imm11_low = h2 & 0x7FF
    imm = (imm11_high << 12) | (imm11_low << 1)
    if imm & 0x00400000:
        imm -= 0x00800000  # 23-bit sign extension
    return base_vma + (pc_index * 2 + 4) + imm


def analyze(blob: bytes, vma: int) -> dict:
    """Resolve the section's `bl` targets to absolute VMAs."""
    halfwords = [int.from_bytes(blob[i:i + 2], "little") for i in range(0, len(blob) - 1, 2)]
    targets = []
    idx = 0
    while idx < len(halfwords) - 1:
        t = thumb_bl_target(halfwords, idx, vma)
        if t is not None:
            targets.append({"offset": idx * 2, "target_vma": f"{t:#010x}"})
            idx += 2
        else:
            idx += 1
    return {"size": len(blob), "halfwords": halfwords, "bl_targets": targets}


def classify(rom_ins: str, cand_ins: str) -> str:
    """Name *how* a candidate differs, from objdump mnemonic sequences."""
    def mnemonics(text: str) -> list[str]:
        return [ln.split("\t")[2].strip() for ln in text.splitlines()
                if ln.count("\t") >= 3 and ":" in ln]
    r, c = mnemonics(rom_ins), mnemonics(cand_ins)
    if r == c:
        return "identical mnemonics (register/operand choice)"
    if sorted(r) == sorted(c):
        return "instruction order"
    return "instruction selection"


_ELF_CACHE: dict[Path, tuple[dict[str, bytes], dict[str, list[tuple[int, str, str, int]]]]] = {}

RELOC_TYPES = {
    0: "R_ARM_NONE",
    1: "R_ARM_PC24",
    2: "R_ARM_ABS32",
    10: "R_ARM_THM_CALL",
    40: "R_ARM_V4BX",
    102: "R_ARM_THM_JUMP11",
}


def fast_parse_elf(obj_path: Path) -> tuple[dict[str, bytes] | None, dict[str, list[tuple[int, str, str, int]]] | None]:
    if obj_path in _ELF_CACHE:
        return _ELF_CACHE[obj_path]
    try:
        data = obj_path.read_bytes()
        if len(data) < 52 or data[:4] != b"\x7fELF":
            return None, None
        e_shoff, = struct.unpack_from("<I", data, 32)
        e_shentsize, e_shnum, e_shstrndx = struct.unpack_from("<HHH", data, 46)
        if e_shstrndx >= e_shnum or e_shentsize < 40:
            return None, None
        shstr_offset = e_shoff + e_shstrndx * e_shentsize
        shstr_sh_offset, shstr_sh_size = struct.unpack_from("<II", data, shstr_offset + 16)
        shstrtab = data[shstr_sh_offset:shstr_sh_offset + shstr_sh_size]

        sec_headers = []
        sections = {}
        symtab_off = symtab_size = strtab_off = 0

        for i in range(e_shnum):
            off = e_shoff + i * e_shentsize
            sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link, sh_info = struct.unpack_from("<IIIIIIII", data, off)
            name_end = shstrtab.find(b"\0", sh_name)
            sec_name = shstrtab[sh_name:name_end].decode("ascii", errors="replace")
            sec_headers.append((sec_name, sh_type, sh_offset, sh_size, sh_link, sh_info))
            if sec_name.startswith(".text"):
                sections[sec_name] = data[sh_offset:sh_offset + sh_size]
            elif sec_name == ".symtab":
                symtab_off, symtab_size = sh_offset, sh_size
            elif sec_name == ".strtab":
                strtab_off = sh_offset

        symbols = []
        if symtab_off and strtab_off:
            for i in range(0, symtab_size, 16):
                st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack_from("<IIIBBH", data, symtab_off + i)
                name_end = data.find(b"\0", strtab_off + st_name)
                sym_name = data[strtab_off + st_name:name_end].decode("ascii", errors="replace")
                if not sym_name and st_shndx < len(sec_headers):
                    sym_name = sec_headers[st_shndx][0]
                symbols.append(sym_name)

        all_relocs: dict[str, list[tuple[int, str, str, int]]] = {}
        for sec_name, sh_type, sh_offset, sh_size, sh_link, sh_info in sec_headers:
            if sh_type in (4, 9):  # SHT_RELA (4) or SHT_REL (9)
                is_rela = (sh_type == 4)
                entry_size = 12 if is_rela else 8
                relocs = []
                for i in range(0, sh_size, entry_size):
                    if is_rela:
                        r_offset, r_info, r_addend = struct.unpack_from("<III", data, sh_offset + i)
                    else:
                        r_offset, r_info = struct.unpack_from("<II", data, sh_offset + i)
                        r_addend = 0
                    sym_idx = r_info >> 8
                    r_type = r_info & 0xff
                    sym_name = symbols[sym_idx] if sym_idx < len(symbols) else ""
                    if not sym_name:
                        continue
                    reloc_type_str = RELOC_TYPES.get(r_type, str(r_type))
                    relocs.append((r_offset, reloc_type_str, sym_name, r_addend))
                target_name = sec_headers[sh_info][0] if sh_info < len(sec_headers) else ""
                all_relocs[sec_name] = relocs
                if target_name:
                    all_relocs[target_name] = relocs

        res = (sections, all_relocs)
        _ELF_CACHE[obj_path] = res
        return res
    except Exception:
        return None, None


def objdump_text(blob: bytes, vma: int, tag: str) -> str:
    WORK.mkdir(parents=True, exist_ok=True)
    p = WORK / f"{tag}.bin"
    if not p.exists() or p.read_bytes() != blob:
        p.write_bytes(blob)
    return run(["arm-none-eabi-objdump", "-D", "-b", "binary", "-m", "arm", "-M",
                "force-thumb", f"--adjust-vma={vma:#x}", str(p)]).stdout or ""


def section_map(obj: Path) -> dict[str, bytes]:
    """section name -> bytes, for every `.text.*` section of a relocatable object.

    A relocatable object has every section at VMA 0, so the section's *contents*
    are read directly rather than through a linked address; the caller links the
    function at its real ROM VMA when comparing.
    """
    sections, _ = fast_parse_elf(obj)
    if sections is not None:
        return sections

    WORK.mkdir(parents=True, exist_ok=True)
    listing = run(["arm-none-eabi-objdump", "-h", str(obj)]).stdout or ""
    result: dict[str, bytes] = {}
    for line in listing.splitlines():
        parts = line.split()
        if len(parts) < 6 or not parts[0].isdigit():
            continue
        name = parts[1]
        if not name.startswith(".text"):
            continue
        target = WORK / f"{name.replace('.', '_')}.sec.bin"
        if run(["arm-none-eabi-objcopy", "-O", "binary", "--only-section", name,
                str(obj), str(target)], stderr=subprocess.DEVNULL).returncode == 0 and target.exists():
            result[name] = target.read_bytes()
    if not result and ".text" in listing:
        raise RuntimeError(
            f"section_map extracted no .text sections from {obj} although objdump "
            f"reports some; WORK={WORK} may be missing or unwritable")
    return result


def relocations(obj: Path, section: str) -> list[tuple[int, str, str, int]]:
    """Return [(offset, reloc_type, symbol_name, addend)] inside `section`."""
    _, all_relocs = fast_parse_elf(obj)
    if all_relocs is not None:
        return all_relocs.get(section, [])

    out = []
    text = run(["arm-none-eabi-readelf", "-r", "-W", str(obj)]).stdout or ""
    current = None
    target_sections = {section, f".rel{section}", f".rela{section}"}
    for line in text.splitlines():
        m = re.match(r"Relocation section '(\S+)'", line)
        if m:
            current = m.group(1)
            continue
        if current not in target_sections:
            continue
        parts = line.split()
        if len(parts) >= 5 and re.fullmatch(r"[0-9A-Fa-f]+", parts[0]):
            try:
                offset = int(parts[0], 16)
            except ValueError:
                continue
            reloc_type = parts[2]
            sym_name = parts[4]
            addend = 0
            if len(parts) >= 7 and parts[5] == "+":
                try:
                    addend = int(parts[6], 0)
                except ValueError:
                    addend = 0
            out.append((offset, reloc_type, sym_name, addend))
    return out


def link_function(blob: bytes, vma: int, relocs: list[tuple[int, str, str, int]],
                  section_name: str, resolver: SymbolResolver) -> tuple[bytes, list[dict], list[dict]]:
    """Return `blob` with cross-TU calls and absolute pool relocations resolved at `vma`."""
    data = bytearray(blob)
    call_targets: list[dict] = []
    pool_words: list[dict] = []

    for offset, reloc_type, sym_name, addend in relocs:
        if reloc_type == "R_ARM_THM_CALL":
            target_vma = resolver.resolve(sym_name)
            if target_vma is not None and 0 <= offset <= len(data) - 4:
                insn_vma = vma + offset
                encoded = encode_thumb_bl(insn_vma, target_vma)
                data[offset:offset + 4] = encoded
                call_targets.append({
                    "offset": offset,
                    "symbol": sym_name,
                    "target_vma": f"{target_vma:#010x}",
                    "resolved": True,
                })
            else:
                call_targets.append({
                    "offset": offset,
                    "symbol": sym_name,
                    "target_vma": None,
                    "resolved": False,
                })
        elif reloc_type == "R_ARM_THM_JUMP11":
            target_vma = resolver.resolve(sym_name)
            if target_vma is not None and 0 <= offset <= len(data) - 2:
                insn_vma = vma + offset
                data[offset:offset + 2] = encode_thumb_b(insn_vma, target_vma)
                call_targets.append({
                    "offset": offset,
                    "symbol": sym_name,
                    "target_vma": f"{target_vma:#010x}",
                    "resolved": True,
                    "size": 2,
                })
            else:
                call_targets.append({
                    "offset": offset,
                    "symbol": sym_name,
                    "target_vma": None,
                    "resolved": False,
                    "size": 2,
                })
        elif reloc_type == "R_ARM_ABS32":
            if 0 <= offset <= len(data) - 4:
                raw_val = int.from_bytes(data[offset:offset + 4], "little")
                if sym_name == section_name or sym_name.startswith(".text"):
                    target = vma + raw_val + addend
                    resolved = True
                else:
                    sym_vma = resolver.resolve(sym_name)
                    target = (sym_vma if sym_vma is not None else 0) + raw_val + addend
                    resolved = sym_vma is not None
                data[offset:offset + 4] = (target & 0xFFFFFFFF).to_bytes(4, "little")
                pool_words.append({
                    "offset": offset,
                    "symbol": sym_name,
                    "target_value": f"{(target & 0xFFFFFFFF):#010x}",
                    "resolved": resolved,
                })
        elif reloc_type != "R_ARM_NONE":
            pool_words.append({"offset": offset, "symbol": sym_name,
                               "target_value": None, "resolved": False,
                               "relocation_type": reloc_type})

    return bytes(data), call_targets, pool_words

def unresolved_windows(calls: list[dict], pools: list[dict], blob_len: int) -> list[tuple[int, int]]:
    """`[start, end)` spans of the candidate the LINKER never produced.

    Two shapes reach a comparison, and neither one is a byte agbcc emitted
    for this body at a real address:

    * an unresolved `R_ARM_THM_CALL` leaves whatever `as` put in the call
      slot. Measured at every such site in `build/era-corpus/ready-work/`
      (45 rows, 53 sites) that filler is the 4 bytes `ff f7 fe ff`, and its
      `ff f7` halfword is a real `bl`'s own prefix -- so it earns credit
      exactly when the call the C wrote is genuinely there.
    * an unresolved `R_ARM_ABS32` is not left alone: `link_function` line 848
      OVERWRITES the word with `raw_val + addend` on a zero symbol base,
      which for a `.rodata`/`.data` section symbol is a small integer such
      as `00 00 00 00`.

    Both are windows at the relocation's own offset (4 bytes, or 2 for a
    16-bit `R_ARM_THM_JUMP11` tail branch; the entry's `size` says which).
    Overlapping
    windows are merged so a byte is never counted twice, and a window that
    runs past the compared region is dropped rather than clipped.
    """
    spans: list[tuple[int, int]] = []
    for entry in calls + pools:
        if entry.get("resolved", True):
            continue
        off = entry.get("offset")
        size = entry.get("size", 4)
        if not isinstance(off, int) or off < 0 or off + size > blob_len:
            continue
        spans.append((off, off + size))
    if not spans:
        return []
    spans.sort()
    merged = [spans[0]]
    for start, end in spans[1:]:
        last_start, last_end = merged[-1]
        if start <= last_end:
            merged[-1] = (last_start, max(last_end, end))
        else:
            merged.append((start, end))
    return merged



def symbol_sizes(obj: Path) -> dict[str, int]:
    """name -> `st_size`, for the sized function symbols of one object."""
    out: dict[str, int] = {}
    text = run(["arm-none-eabi-nm", "-S", "--defined-only", str(obj)]).stdout or ""
    for line in text.splitlines():
        parts = line.split()
        # `addr size type name` for sized symbols; `addr type name` otherwise.
        if len(parts) == 4:
            try:
                out[parts[3]] = int(parts[1], 16)
            except ValueError:
                continue
    return out


def trim_section_fill(blob: bytes, sym_size: int | None, ref_len: int) -> bytes:
    """Drop gas's section-end alignment fill from the candidate.

    agbcc opens each function section with `.align 2, 0`, so the section's
    alignment is 4 and gas rounds the *section* size up to it -- a body whose
    content is 2 (mod 4) reads 2 bytes long in an isolated section object,
    filled with the code nop `c0 46`. Those bytes are not the body's: the
    splice cuts the text at the body's own `.size` (match_c_slice.asm_section)
    and pads the span itself, so the fill never reaches the slice. Sizing the
    body by the section therefore overstated it -- `_0802BCCE` measured
    OVERSIZED 26/28 for a body whose 26 symbol bytes were all correct.

    Bound the candidate by `max(symbol size, ROM span)`: the symbol is the
    body's real extent, and a ROM span longer than the symbol (a body plus its
    own alignment pad, where the section legitimately carries the pad) is
    never clipped. Only bytes beyond BOTH are trimmed.
    """
    if sym_size is None:
        return blob
    bound = max(sym_size, ref_len)
    return blob[:bound] if len(blob) > bound else blob


def compare_function(rom: bytes, vma: int, name: str, blob: bytes, spans: dict[int, int],
                     calls: list[dict], pools: list[dict],
                     sym_size: int | None = None) -> dict | None:
    """Compare one function's linked agbcc output against its ROM span."""
    if len(blob) < 2:
        return None
    if vma not in spans:
        return None
    end = spans[vma]
    ref = rom[vma - ROM_BASE:end - ROM_BASE]
    blob = trim_section_fill(blob, sym_size, len(ref))
    rec: dict = {
        "name": name,
        "vma": f"{vma:#010x}",
        "rom_bytes": len(ref),
        "cand_bytes": len(blob),
        "status": "?",
        "class": None,
        "call_targets": calls,
        "pool_words": pools,
    }

    # The disassembly dumps are written BEFORE the early return below, on
    # purpose. They used to sit after it, so any EXACT run left the previous
    # run's `<name>_rom.bin` / `<name>_cand.bin` in the work dir -- and a later
    # reader of those files saw a confident, wrong answer. The JSON score is
    # always correct; the dumped blobs were not. Keep the dumps unconditional so
    # the files always describe the run that just happened.
    WORK.mkdir(parents=True, exist_ok=True)
    p_rom = WORK / f"{name}_rom.bin"
    p_cand = WORK / f"{name}_cand.bin"
    p_rom.write_bytes(ref)
    p_cand.write_bytes(blob)
    # Bytes the linker never produced are not measurements of anything, so
    # they are excluded from `matched_bytes` rather than counted as a match.
    # Counting them was not a neutral choice: `ff f7 fe ff` shares its first
    # halfword with a genuine nearby `bl`, so the count was biased UPWARD in
    # proportion to how correct the unresolved call actually was. Excluding
    # them keeps the field an int -- `experiment_kit.Score.from_record`
    # (tools/experiment_kit.py:723) copies it into `Score.matched_bytes: int`
    # and `tools/novelty_search.py:536` sorts on `-c.score.matched_bytes`, so
    # a `null` here is a TypeError there, not a better number. `prefix` and
    # `first_diff` are deliberately left alone: they answer a different
    # question (docs/matching_workflow.md, "prefix is the sound signal") and
    # for this class they usefully point AT the unresolved call site.
    compared = min(len(blob), len(ref))
    holes = unresolved_windows(calls, pools, compared)
    rec["unresolved_bytes"] = sum(end - start for start, end in holes)


    if blob == ref and all(x.get("resolved", True) for x in calls + pools):
        rec["status"] = "EXACT"
        rec["first_diff"] = None
        rec["prefix"] = len(blob)
        rec["matched_bytes"] = len(blob)
        rec["class"] = "identical mnemonics (register/operand choice)"
        return rec

    diffs = [i for i in range(min(len(blob), len(ref))) if blob[i] != ref[i]]
    if diffs:
        rec["first_diff"] = diffs[0]
        rec["prefix"] = diffs[0]
    elif len(blob) != len(ref):
        rec["first_diff"] = min(len(blob), len(ref))
        rec["prefix"] = min(len(blob), len(ref))
    else:
        rec["first_diff"] = None
        rec["prefix"] = len(blob)

    matched = 0
    at = 0
    for start, end in holes:
        matched += sum(1 for i in range(at, start) if blob[i] == ref[i])
        at = end
    matched += sum(1 for i in range(at, compared) if blob[i] == ref[i])
    rec["matched_bytes"] = matched


    if any(not x.get("resolved", True) for x in calls + pools):
        rec["status"] = "UNRESOLVED_RELOCATION"
        rec["class"] = "unresolved relocation"
        # `matched_bytes` above is a count of the bytes that WERE linked, over
        # a region `unresolved_bytes` bytes smaller than the candidate. It is
        # the number to read for triage; the status still says the candidate
        # was never compared in full, so no promotion may be inferred from it.
        return rec

    if len(blob) > len(ref):
        rec["status"] = "OVERSIZED"
    elif rec["prefix"] > 0 or matched > 0:
        rec["status"] = "PARTIAL"
    else:
        rec["status"] = "NO_OVERLAP"

    rom_text = objdump_text(ref, vma, f"{name}_rom")
    blob_text = objdump_text(blob, vma, f"{name}_cand")
    rec["class"] = classify(rom_text, blob_text)
    return rec


def _promoted_vmas() -> set[int]:
    """VMAs the manifest has already promoted. Unpromoted bodies are the
    normal state of this project, so they carry no obligation."""
    import json as _json
    path = ROOT / "tools" / "matching_slice_functions.json"
    if not path.exists():
        return set()
    data = _json.loads(path.read_text())
    out = set()
    for key in data:
        v = parse_vma_name(key)
        if v:
            out.add(v)
    return out


def self_test() -> int:
    global AGBCC, WORK
    """Self-test suite for cross-TU call resolution and scorer verification."""
    print("running corpus_match_probe self-test...")
    passed = 0
    total = 0

    def check(name: str, condition: bool):
        nonlocal passed, total
        total += 1
        if condition:
            passed += 1
            print(f"  [PASS] {name}")
        else:
            print(f"  [FAIL] {name}", file=sys.stderr)

    # 1. Roundtrip Thumb-1 BL encoding and decoding
    test_cases = [
        (0x08006C36, 0x0802DE9C),  # forward large
        (0x08006BCE, 0x08005F98),  # backward small
        (0x08006C64, 0x08006C10),  # backward tiny
        (0x0800B1B0, 0x080188B0),  # forward medium
    ]
    bl_roundtrip_ok = True
    for insn_vma, target_vma in test_cases:
        b = encode_thumb_bl(insn_vma, target_vma)
        hw = [int.from_bytes(b[:2], "little"), int.from_bytes(b[2:], "little")]
        decoded = thumb_bl_target(hw, 0, insn_vma)
        if decoded != target_vma:
            bl_roundtrip_ok = False
            break
    check("Thumb-1 BL encode/decode roundtrip", bl_roundtrip_ok)

    # 2. Known forward displacement matches ROM baserom bytes exactly
    # 0x08006C36 -> 0x0802DE9C in ROM is 27 f0 31 f9
    b_6c36 = encode_thumb_bl(0x08006C36, 0x0802DE9C)
    check("BL 0x08006C36 -> 0x0802DE9C matches ROM bytes", b_6c36 == bytes.fromhex("27 f0 31 f9"))

    # 3. Known backward displacement matches ROM baserom bytes exactly
    # 0x08006BCE -> 0x08005F98 in ROM is ff f7 e3 f9
    b_6bce = encode_thumb_bl(0x08006BCE, 0x08005F98)
    check("BL 0x08006BCE -> 0x08005F98 matches ROM bytes", b_6bce == bytes.fromhex("ff f7 e3 f9"))

    # 3b. Thumb-1 unconditional `b` (R_ARM_THM_JUMP11) known ROM bytes:
    # the two `b.n _0802BCF6` tail merges in sub_0802BD44 and the
    # `b.n _0802BCCE` tail jump at the end of sub_0802BCE8.
    check("b 0x0802BD52 -> 0x0802BCF6 matches ROM bytes",
          encode_thumb_b(0x0802BD52, 0x0802BCF6) == bytes.fromhex("d0 e7"))
    check("b 0x0802BD64 -> 0x0802BCF6 matches ROM bytes",
          encode_thumb_b(0x0802BD64, 0x0802BCF6) == bytes.fromhex("c7 e7"))
    check("b 0x0802BCF0 -> 0x0802BCCE matches ROM bytes",
          encode_thumb_b(0x0802BCF0, 0x0802BCCE) == bytes.fromhex("ed e7"))
    b_jump11 = encode_thumb_b(0x0802BD52, 0x0802BCF6)
    hw_jump11 = int.from_bytes(b_jump11, "little") & 0x7FF
    if hw_jump11 & 0x400:
        hw_jump11 -= 0x800
    check("b encode/decode roundtrip",
          0x0802BD52 + 4 + (hw_jump11 << 1) == 0x0802BCF6)
    try:
        encode_thumb_b(0x0802BD52, 0x0802BD52 + 0x2000)
        out_of_range = False
    except ValueError:
        out_of_range = True
    check("b rejects an out-of-range target", out_of_range)

    # 3c. trim_section_fill: gas rounds the SECTION up to its 4-byte
    # alignment, so an isolated body whose content is 2 (mod 4) measures 2
    # bytes long with `c0 46` fill. The splice cuts at the body's `.size`, so
    # the fill is a measurement artifact and must not fail a match. Bound by
    # max(symbol size, ROM span) -- never clip real content.
    check("section fill beyond the symbol size is trimmed",
          trim_section_fill(bytes(26) + bytes.fromhex("c0 46"), 26, 26) == bytes(26))
    check("a ROM span longer than the symbol is not clipped",
          trim_section_fill(bytes(28), 26, 28) == bytes(28))
    check("content inside the bound is untouched",
          trim_section_fill(bytes(30), 32, 32) == bytes(30))
    check("an unsized symbol is not trimmed",
          trim_section_fill(bytes(28), None, 26) == bytes(28))

    # 3d. code_o_staleness: the shared staleness guard. Pinned on synthetic
    # files with pinned mtimes -- the rule is a bare timestamp comparison and
    # the failure it guards against is silent, so the comparison itself must
    # not drift. A missing object and an empty source set both answer "",
    # because the callers own their own "no closure object" wording and a
    # second message there would be noise.
    with tempfile.TemporaryDirectory(prefix="cmp-codeo-") as tmp:
        t = Path(tmp)
        obj, src = t / "code.o", t / "cluster.s"
        obj.write_text("obj", encoding="utf-8")
        src.write_text("src", encoding="utf-8")
        os.utime(obj, (200, 200))
        os.utime(src, (100, 100))
        check("a closure object newer than every source is fresh",
              code_o_staleness(obj, [src]) == "")
        os.utime(src, (300, 300))
        stale_msg = code_o_staleness(obj, [src])
        check("an object older than the newest source is flagged",
              "older than the newest closure file" in stale_msg)
        check("...and the warning names the object",
              "code.o" in stale_msg)
        check("a missing object is left to the caller's own message",
              code_o_staleness(t / "absent.o", [src]) == "")
        check("an empty source set is vacuously fresh",
              code_o_staleness(obj, []) == "")

    # 4. SymbolResolver tests
    resolver = SymbolResolver(
        code_syms={"_08006C10": 0x08006C10, "_0802DE9C": 0x0802DE9C},
        aliases={"_0802D974": "CpuSet"},
        hints={"EventPost": 0x08025BF0},
    )
    check("resolver: direct VMA", resolver.resolve("_08006C10") == 0x08006C10)
    check("resolver: runtime routine", resolver.resolve("_call_via_r2") == 0x0802DDD0)
    check("resolver: alias target", resolver.resolve("CpuSet") == 0x0802D974)
    check("resolver: decl hint", resolver.resolve("EventPost") == 0x08025BF0)
    check("VMA aliases with an extra zero normalize to one address",
          parse_vma_name("_080028250") == parse_vma_name("_08028250") == 0x08028250)
    check("VMA parser rejects a trailing hex digit", parse_vma_name("_080282500") is None)
    check("untyped called leaf receives its real 8-byte span", rom_functions().get(0x08028250) == 0x08028258)
    check("adjacent shared-tail entry keeps its 2-byte prefix span",
          rom_functions().get(0x0802BCE8) == 0x0802BCEA)
    check("adjacent validator entry keeps its 2-byte prefix span",
          rom_functions().get(0x0802BCCC) == 0x0802BCCE)
    # A `known` interior branch point is reached by `bl` but is not a function:
    # 0x0802C10C is `bx r3` and must not be given a candidate. The register
    # branch veneers at 0x0802DDC8..0x0802DDEC used to be treated as having no C
    # bodies. They now have exact naked C definitions in runtime_state_dispatch.c
    # (verified through the C89 path), so they are ordinary promoted candidates
    # and must not remain in this exemption set. 0x0802BCCE USED to be listed
    # here ("2 bytes into a body"); it is now a real, C-owned entry of its own
    # -- the label sweep made the inventory see `bl`-targeted interior entries,
    # and its 26-byte validate fragment is promoted from src/sound_seq_follow.c.
    # The exemption had to go with it, or `no promoted body is silently
    # exempted` would have gone red the moment it was staged.
    #
    # 0x0800A020 and 0x0800A024 were listed here as "`bx r7; nop`". That was
    # WRONG, and the error survived because the entry was never re-measured. The
    # ROM holds `movs r0,#44; bx lr` and `movs r0,#15; bx lr` -- two ordinary
    # 4-byte leaf functions, since promoted as such. The "no promoted body is
    # silently exempted" check below is what caught it: promoting them made the
    # two classifications contradict each other, which is the only reason a
    # stale exemption ever announces itself.
    NO_C_BODY = {0x0802C10C}
    known = set(rom_functions())
    sourced = {entry[2] for entry in src_functions()}
    # Pin the BL_RE fix itself: a numeric `bl 0x0800XXXX` operand must reach the
    # inventory, which it cannot if the capture class starts with [A-Za-z_].
    check("a numeric bl target is a known start (BL_RE sees 0x operands)",
          0x08009FFC in known)
    # Scope the obligation to bodies the project has actually PROMOTED.
    # Asserting it over every known start was wrong the moment added 59
    # real ROM entries that have no C body yet -- and 974 unpromoted bodies is
    # the normal state of this project, not a defect. The old form could only be
    # satisfied by growing NO_C_BODY by hand, which is the same hand-maintained
    # list that let the label census rot from 2 to a claimed 9.
    #
    # The invariant that actually matters: a body in the manifest is spliced
    # from C, so it MUST have a C candidate, and it must not be exempted.
    promoted = set(_promoted_vmas())
    required = (known & promoted) - NO_C_BODY
    check("every PROMOTED body has a C candidate", required <= sourced)
    check("the promoted set is non-empty, so the check is not vacuous",
          bool(promoted))
    check("no promoted body is silently exempted",
          not ((known & promoted) & NO_C_BODY))
    # Reject-side pin. The check above is a set subtraction, so a NO_C_BODY that
    # grows too wide would make it pass vacuously -- and it would keep passing.
    # Take a start that IS legitimately required, drop just that one from
    # `sourced`, and assert the containment now fails. Cheap, uses only live data,
    # and fails by name if the check ever stops being able to say no.
    _required = sorted(required & sourced)
    check("control: a real obligation is not silently exempted", bool(_required))
    if _required:
        check("control: dropping one required candidate fails the containment",
              not (required <= (sourced - {_required[0]})))
    check("prefixed and suffixed VMA spellings normalize",
          parse_vma_name("sub_08002388") == 0x08002388
          and parse_vma_name("_0802D84C_command") == 0x0802D84C)

    # 5. Near-match verification on _08006C10 (80 B candidate with cross-TU call)
    if ROM.exists():
        rom_data = ROM.read_bytes()
        # Compile the fixture into a PRIVATE temp dir, every single run.
        # The old code reused build/era-corpus/corpus/course_stream_more.o
        # whenever that file existed, so this self-test's verdict depended on
        # leftover build residue: it reported green against a 3-day-old
        # pre-fix object for byte-identical source. A self-test that can be
        # green or red for the same source is not a test. Always rebuild.
        fixture_dir = Path(tempfile.mkdtemp(prefix="probe-selftest-"))
        csm_obj = fixture_dir / "course_stream_more.o"
        src_file = ROOT / "src/course_stream_more.c"
        pre = fixture_dir / "course_stream_more.i"
        sfile = fixture_dir / "course_stream_more.s"
        with pre.open("w", encoding="utf-8") as h:
            run(["clang", "-E", "-nostdinc", "-undef", *inc_flags(), str(src_file)], stdout=h)
        run([AGBCC, "-O2", "-mthumb-interwork", "-ffunction-sections", str(pre), "-o", str(sfile)])
        run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-o", str(csm_obj), str(sfile)])

        if csm_obj.exists():
            secs = section_map(csm_obj)
            sec_name = ".text._08006C10"
            if sec_name in secs:
                rel = relocations(csm_obj, sec_name)
                linked_blob, calls, pools = link_function(secs[sec_name], 0x08006C10, rel, sec_name, resolver)
                spans = {0x08006C10: 0x08006C60}
                rec = compare_function(rom_data, 0x08006C10, "_08006C10", linked_blob, spans, calls, pools)
                # The body is byte-exact over its full 80-byte ROM span. These
                # assertions used to pin the PRE-fix state (78/80 matched, first
                # diff at +0x4E) as though the trailing 2-byte Thumb nop filler
                # were the expected result. Commit 1fa62bd applied the
                # `.align 2, 0` pad fix, so that halfword is now `00 00` and
                # there is no difference at all. This is a TIGHTENING, not a
                # relaxation: the old expectation tolerated a 2-byte defect,
                # this one demands zero difference over the whole span. The
                # numbers below were verified against baserom.gba itself.
                check("_08006C10: call to _0802DE9C resolved", len(calls) == 1 and calls[0]["target_vma"] == "0x0802de9c")
                check("_08006C10: 80/80 matched bytes (byte-exact)", rec is not None and rec.get("matched_bytes") == 80 and rec.get("rom_bytes") == 80)
                check("_08006C10: no first difference", rec is not None and rec.get("first_diff") is None)
                # Negative control, and the reason the previous version of this
                # check was wrong: a byte-exactness assertion is only meaningful
                # if it CAN fail. Reintroduce the pre-fix Thumb nop filler
                # (0x46c0) in the trailing halfword and require the comparison to
                # catch it. If this ever passes, the comparison has gone blind.
                nop_tail = bytearray(linked_blob)
                nop_tail[0x4E:0x50] = b"\xc0\x46"
                rec_nop = compare_function(rom_data, 0x08006C10, "_08006C10", bytes(nop_tail), spans, calls, pools)
                check("_08006C10: control - Thumb nop tail IS detected", rec_nop is not None and rec_nop.get("first_diff") == 0x4E)

            # 6. Deliberately wrong candidate (calling wrong callee)
            # Create a mutated candidate that calls _08005F98 instead of _0802DE9C at offset 0x26
            if sec_name in secs:
                mutated_rel = [(38, "R_ARM_THM_CALL", "_08005F98", 0)]
                wrong_blob, wrong_calls, _ = link_function(secs[sec_name], 0x08006C10, mutated_rel, sec_name, resolver)
                # The call at offset 0x26 must differ from ROM!
                call_matches_rom = (wrong_blob[38:42] == rom_data[0x6C10 + 38 : 0x6C10 + 42])
                check("deliberately wrong candidate: call mismatch detected (not falsely matched)", not call_matches_rom)

            # 7. Exact-instructions verification on _08006C60 (52 B candidate with 2 calls)
            sec_name_60 = ".text._08006C60"
            if sec_name_60 in secs:
                rel_60 = relocations(csm_obj, sec_name_60)
                linked_60, calls_60, _ = link_function(secs[sec_name_60], 0x08006C60, rel_60, sec_name_60, resolver)
                spans_60 = {0x08006C60: 0x08006C94}
                rec_60 = compare_function(rom_data, 0x08006C60, "_08006C60", linked_60, spans_60, calls_60, [])
                # Byte-exact over the full 52-byte span, for the same reason as
                # _08006C10 above: the pad fix landed in 1fa62bd and these
                # assertions still described the pre-fix defect.
                check("_08006C60: 2/2 calls resolved", len(calls_60) == 2 and all(c["resolved"] for c in calls_60))
                check("_08006C60: 52/52 matched bytes (byte-exact)", rec_60 is not None and rec_60.get("matched_bytes") == 52 and rec_60.get("rom_bytes") == 52)
                check("_08006C60: no first difference", rec_60 is not None and rec_60.get("first_diff") is None)
                # Negative control: the nop tail must be detectable here too.
                nop_tail_60 = bytearray(linked_60)
                nop_tail_60[0x32:0x34] = b"\xc0\x46"
                rec_nop_60 = compare_function(rom_data, 0x08006C60, "_08006C60", bytes(nop_tail_60), spans_60, calls_60, [])
                check("_08006C60: control - Thumb nop tail IS detected", rec_nop_60 is not None and rec_nop_60.get("first_diff") == 0x32)

    # The TU cache is only sound if a HEADER edit invalidates it. Every TU
    # here #includes from include/ and asm/, so a key built from the .c file
    # alone would serve pre-edit objects for every affected TU -- and a stale
    # object still scores cleanly, so that is a confident wrong answer rather
    # than a crash. Assert the key moves when the preprocessed text moves.
    with tempfile.TemporaryDirectory() as td:
        tdp = Path(td)
        hdr = tdp / "hdr.h"
        hdr.write_text("#define N 1\n", encoding="utf-8")
        body = tdp / "tu.c"
        body.write_text('#include "hdr.h"\nint f(void){return N;}\n', encoding="utf-8")
        for out in ("a.i", "b.i"):
            with (tdp / out).open("w") as h:
                run(["clang", "-E", "-nostdinc", "-undef", "-I", str(tdp), str(body)], stdout=h)
        check("TU cache key is stable for identical preprocessed text",
              _tu_cache_key(tdp / "a.i", True) == _tu_cache_key(tdp / "b.i", True))
        # The property match_c_slice actually depends on: it gives every
        # promoted body its own --work-dir, and clang stamps that directory
        # into every linemarker. Without normalising the path field the key
        # differs per run and the cache can NEVER hit, which is the same
        # failure as having no cache at all -- and the counters would look
        # like a slow tool rather than a broken one.
        d1, d2 = tdp / "wd1", tdp / "wd2"
        for d in (d1, d2):
            d.mkdir()
            (d / "c89").mkdir()
            (d / "c89" / "tu.c").write_text((tdp / "tu.c").read_text(encoding="utf-8"), encoding="utf-8")
            with (d / "tu.i").open("w") as h:
                run(["clang", "-E", "-nostdinc", "-undef", "-I", str(tdp),
                     str(d / "c89" / "tu.c")], stdout=h)
        check("TU cache key ignores the work dir (linemarkers normalised)",
              _tu_cache_key(d1 / "tu.i", True) == _tu_cache_key(d2 / "tu.i", True))
        check("the two work dirs really did produce different raw bytes",
              (d1 / "tu.i").read_bytes() != (d2 / "tu.i").read_bytes())
        hdr.write_text("#define N 2\n", encoding="utf-8")
        with (tdp / "c.i").open("w") as h:
            run(["clang", "-E", "-nostdinc", "-undef", "-I", str(tdp), str(body)], stdout=h)
        check("a header edit changes the TU cache key",
              _tu_cache_key(tdp / "a.i", True) != _tu_cache_key(tdp / "c.i", True))
        # Negative control: the key must NOT be the .c content alone, or the
        # case above would pass for the wrong reason.
        check("TU cache key is not just the source file bytes",
              _tu_cache_key(tdp / "a.i", True) != hashlib.sha256(body.read_bytes()).hexdigest()[:32])
        # And a toolchain change must miss too, or a rebuilt agbcc would serve
        # every previously-cached object. Monkeypatched rather than faked with
        # a parameter, so the production key path is the one under test.
        real_agbcc = AGBCC
        k_before = _tu_cache_key(tdp / "a.i", True)
        fake = tdp / "agbcc-fake"
        fake.write_text("not really agbcc\n", encoding="utf-8")
        import os as _os
        _os.utime(fake, (1, 1))
        AGBCC = fake
        try:
            check("a toolchain change invalidates the TU cache key",
                  _tu_cache_key(tdp / "a.i", True) != k_before)
        finally:
            AGBCC = real_agbcc

        # Regression: the disassembly dumps must describe the run that just
        # happened, INCLUDING an EXACT one. They used to be written after the
        # EXACT early return, so an EXACT run left the PREVIOUS run's blobs in
        # the work dir -- and the same source text then read back as a confident,
        # wrong score. objdump_text writes into the module-level WORK, so the
        # assertions read from there rather than from a local temp dir.
        # compare_function slices ref = rom[vma-ROM_BASE : end-ROM_BASE], so
        # the payload must sit at the FILE OFFSET, not at the VMA. Offsetting by
        # the VMA here puts it 8 MB past the end and the slice reads zeros.
        _BLOFF = 0x08001000 - ROM_BASE
        _BLROM = bytearray(_BLOFF + 8)
        _BLROM[_BLOFF:_BLOFF + 4] = b"\x01\x02\x04\x05"
        _BLSPAN = {0x08001000: 0x08001004}
        _real_work = WORK
        WORK = tdp / "blobs"
        try:
            WORK.mkdir(exist_ok=True)
            cand_bin = WORK / "x_cand.bin"
            cand_bin.write_bytes(b"\xff\xff")          # seed a WRONG blob
            exact_rec = compare_function(bytes(_BLROM), 0x08001000, "x",
                                         b"\x01\x02\x04\x05", _BLSPAN, [], [])
            check("an EXACT run overwrites a stale candidate blob",
                  exact_rec is not None and exact_rec["status"] == "EXACT"
                  and cand_bin.read_bytes() == b"\x01\x02\x04\x05")
            partial_rec = compare_function(bytes(_BLROM), 0x08001000, "x",
                                           b"\x01\x02\x09\x05", _BLSPAN, [], [])
            check("a later PARTIAL run overwrites the candidate blob",
                  partial_rec is not None and partial_rec["status"] == "PARTIAL"
                  and cand_bin.read_bytes() == b"\x01\x02\x09\x05")
            # Negative control: the EXACT case must be the one that catches the
            # ordering. Re-seed and run a PARTIAL first -- a PARTIAL always
            # wrote the blob even before the fix, so only the EXACT assertion
            # distinguishes the two orderings.
            cand_bin.write_bytes(b"\xff\xff")
            compare_function(bytes(_BLROM), 0x08001000, "x",
                             b"\x01\x02\x09\x05", _BLSPAN, [], [])
            check("only the EXACT path was reordered (control: PARTIAL always wrote)",
                  cand_bin.read_bytes() == b"\x01\x02\x09\x05")
        finally:
            WORK = _real_work

        # `matched_bytes` must not count bytes the LINKER never produced. The
        # fixture is built by hand rather than read from a live report, so the
        # case cannot start failing for a reason that has nothing to do with the
        # rule: 16 ROM bytes whose +4..+7 window is `ff f7 fe ff`, i.e. a real
        # nearby `bl`'s own prefix, which is exactly the placeholder `as`
        # leaves behind and exactly the case that used to earn free credit.
        # The candidate then differs from the ROM only at +11, so `prefix` has
        # a non-trivial value and the two fields can be compared.
        _UOFF = 0x08002000 - ROM_BASE
        _UROM = bytearray(_UOFF + 16)
        _UROM[_UOFF:_UOFF + 16] = (b"\x01\x02\x03\x04\xff\xf7\xfe\xff"
                                    b"\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10")
        _USPAN = {0x08002000: 0x08002010}
        _UCAND = (b"\x01\x02\x03\x04\xff\xf7\xfe\xff"
                  b"\x09\x0a\x0b\x99\x0d\x0e\x0f\x10")
        _UCALL_U = [{"offset": 4, "symbol": "StaticHelper", "target_vma": None,
                     "resolved": False}]
        _UCALL_R = [{"offset": 4, "symbol": "StaticHelper",
                     "target_vma": "0x08002000", "resolved": True}]
        WORK = tdp / "unres"
        try:
            WORK.mkdir(exist_ok=True)
            check("unresolved_windows finds the 4-byte window at the offset",
                  unresolved_windows(_UCALL_U, [], 16) == [(4, 8)])
            check("overlapping unresolved windows merge instead of double counting",
                  unresolved_windows([{"offset": 4, "resolved": False},
                                      {"offset": 6, "resolved": False}], [], 16)
                  == [(4, 10)])
            check("an unresolved window past the compared region is dropped",
                  unresolved_windows([{"offset": 14, "resolved": False}], [], 16) == [])
            # Reject side of the first case: if this passed for the wrong reason
            # -- say because the loop never ran -- the accept case above would
            # be vacuous.
            check("a resolved relocation contributes no window",
                  unresolved_windows(_UCALL_R, [], 16) == []
                  and unresolved_windows([], [{"offset": 4, "symbol": ".rodata",
                                              "target_value": "0x08001234",
                                              "resolved": True}], 16) == [])
            hole_rec = compare_function(bytes(_UROM), 0x08002000, "u",
                                        _UCAND, _USPAN, _UCALL_U, [])
            check("a placeholder matching the ROM by chance is not counted",
                  hole_rec is not None and hole_rec["matched_bytes"] == 11
                  and hole_rec["unresolved_bytes"] == 4)
            # The second shape of fabricated byte, reproduced from a measured
            # live case: link_function line 848 overwrites an unresolvable
            # `.rodata` pool word with `raw_val + addend` on a zero base, so
            # the candidate carries `00 00 00 00` where the ROM carries
            # `08 00 de 34`. Here both streams hold zeros at +4..+7, which is
            # the shape of `_08025548` and `_0800DD9C`.
            _UROM2 = bytearray(_UROM)
            _UROM2[_UOFF + 4:_UOFF + 8] = b"\x00\x00\x00\x00"
            pool_rec = compare_function(
                bytes(_UROM2), 0x08002000, "u",
                b"\x01\x02\x03\x04\x00\x00\x00\x00\x09\x0a\x0b\x99\x0d\x0e\x0f\x10",
                _USPAN, [], [{"offset": 4, "symbol": ".rodata",
                              "target_value": "0x00000000", "resolved": False}])
            check("a section-relative pool word the linker filled with zero is excluded too",
                  pool_rec is not None and pool_rec["matched_bytes"] == 11
                  and pool_rec["unresolved_bytes"] == 4)
            # Control: the SAME bytes with the call resolved must score exactly
            # as before the change. If this moved, the exclusion would be a
            # blanket one and every real body would lose 4 bytes of credit.
            full_rec = compare_function(bytes(_UROM), 0x08002000, "u",
                                        _UCAND, _USPAN, _UCALL_R, [])
            check("control: the same blob with the call resolved counts every byte",
                  full_rec is not None and full_rec["matched_bytes"] == 15
                  and full_rec["unresolved_bytes"] == 0)
            check("the exclusion leaves prefix and first_diff untouched",
                  hole_rec is not None and full_rec is not None
                  and hole_rec["prefix"] == full_rec["prefix"] == 11
                  and hole_rec["first_diff"] == full_rec["first_diff"] == 11)
            check("a placeholder row is not reported as a normal PARTIAL",
                  hole_rec is not None
                  and hole_rec["status"] == "UNRESOLVED_RELOCATION"
                  and full_rec is not None and full_rec["status"] == "PARTIAL")
            # Negative control: a RESOLVED call to the wrong address must still
            # fail, with the exclusion switched off. Without this the case
            # above could pass for the wrong reason -- a tool that hid every
            # 4-byte call window would also report 11 here.
            _wrong_blob, _wrong_calls, _wrong_pools = link_function(
                b"\x01\x02\x03\x04\x00\x00\x00\x00\x09\x0a\x0b\x99\x0d\x0e\x0f\x10",
                0x08002000, [(4, "R_ARM_THM_CALL", "_08006C10", 0)],
                ".text.u", resolver)
            wrong_rec = compare_function(bytes(_UROM), 0x08002000, "u",
                                       _wrong_blob, _USPAN, _wrong_calls, _wrong_pools)
            check("a resolved but wrong call target still fails",
                  wrong_rec is not None
                  and _wrong_blob[4:8] != bytes(_UROM[_UOFF + 4:_UOFF + 8])
                  and wrong_rec["status"] != "EXACT"
                  and wrong_rec["unresolved_bytes"] == 0
                  and wrong_rec["matched_bytes"] == 15 - 4)
        finally:
            WORK = _real_work

    print(f"\nself-test: {passed}/{total} checks PASS")
    return 0 if passed == total else 1


def main() -> int:
    global WORK
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--json", type=Path)
    ap.add_argument("--limit", type=int, default=0, help="max functions (0 = all)")
    ap.add_argument("--function", type=str, action="append",
                    help="target specific function name(s) (e.g. _08006C10); repeatable, so one "
                         "run can score every promoted body in a TU instead of one run each")
    ap.add_argument("--work-dir", type=Path, help="private scratch directory for this run")
    ap.add_argument("--include-unknown", action="store_true", help="include source VMAs outside the known ASM function inventory")
    ap.add_argument("--c89", action="store_true", help="compile validated C89 copies of source TUs")
    ap.add_argument("--require-all", action="store_true", help="fail if any selected TU cannot compile")
    ap.add_argument("--require-exact", action="store_true", help="fail unless every selected function is byte-exact")
    ap.add_argument("--self-test", action="store_true", help="run self-test suite")
    args = ap.parse_args()

    if args.work_dir:
        WORK = args.work_dir.resolve()
    c89.WORK = WORK / "c89-transform"

    if args.self_test:
        return self_test()

    if not AGBCC.exists():
        print("corpus_match: agbcc not built", file=sys.stderr)
        return 2
    if not ROM.exists():
        print("corpus_match: no ROM", file=sys.stderr)
        return 2

    rom = ROM.read_bytes()
    spans = rom_functions()
    alias_target = alias_targets()
    code_syms = load_code_symbols()
    decl_hints = scan_decl_hints()
    resolver = SymbolResolver(code_syms, alias_target, decl_hints)

    if args.function:
        wanted = list(args.function)
        requested = {parse_vma_name(n) for n in wanted}
        requested.discard(None)
        funcs = src_functions(filter_wanted=wanted)
        funcs = [f for f in funcs
                 if f[1] in wanted
                 or f[1].lstrip("_") in {n.lstrip("_") for n in wanted}
                 or (requested and f[2] in requested)]
        if not funcs:
            print(f"corpus_match: none of {wanted} found in src/", file=sys.stderr)
            return 1
    elif args.limit:
        funcs = src_functions()[:args.limit]
    else:
        funcs = src_functions()
    if not args.include_unknown and not args.function:
        funcs = [f for f in funcs if f[2] in spans]

    # group by file so each TU is compiled once
    by_file: dict[Path, list[tuple[str, int, int, bool]]] = collections.defaultdict(list)
    best: dict[int, tuple] = {}
    for f, name, vma, body, weak in funcs:
        # Keep one definition per VMA: the strong body wins over a weak stub,
        # and a longer body wins over a shorter one at the same strength.
        rank = (0 if weak else 1, body)
        if vma not in best or rank > best[vma][0]:
            best[vma] = (rank, f, name, body, weak)
    for vma, (_, f, name, body, weak) in best.items():
        by_file[f].append((name, vma, body, weak))

    results = []
    compiled_files = 0
    tu_cache_hits = 0
    tu_cache_compiles = 0
    skipped_files = 0
    skipped_reasons: list[dict[str, str]] = []
    unscored: list[dict[str, str]] = []

    def process_one_tu(item: tuple[Path, list[tuple[str, int, int, bool]]]):
        path, tu_items = item
        pre = WORK / (path.stem + ".i")
        src = WORK / (path.stem + ".s")
        obj = WORK / (path.stem + ".o")
        compile_path = path
        hit = 0
        comp = 0
        unsc = []
        res = []

        if args.c89:
            transformed, stats = cached_transform_file(path)
            if stats["unhandled"] is not None:
                return (hit, comp, {"source": str(path.relative_to(ROOT)),
                                    "reason": str(stats["unhandled"])}, unsc, res, 0)
            compile_path = WORK / "c89" / path.name
            compile_path.parent.mkdir(parents=True, exist_ok=True)
            compile_path.write_text(transformed, encoding="utf-8")

        with pre.open("w", encoding="utf-8") as h:
            if run(["clang", "-E", "-nostdinc", "-undef", *inc_flags(), str(compile_path)], stdout=h).returncode:
                return (hit, comp, {"source": str(path.relative_to(ROOT)), "reason": "preprocess"}, unsc, res, 0)

        # Preprocess first, then consult the cache. `pre` is agbcc's real
        # input, so it already folds in every #include -- a header edit
        # changes it. Keying on the .c file instead would serve pre-edit
        # objects for every TU that includes the changed header, and a stale
        # object still produces a confident, byte-exact-looking score.
        if tu_cache_lookup(pre, bool(args.c89), src, obj):
            hit = 1
        else:
            comp = 1
            # One compile per opt level would triple the work; -O2 is the default the
            # project builds C with, so measure that and report it as such.
            if run([AGBCC, "-O2", "-mthumb-interwork", "-ffunction-sections", str(pre), "-o", str(src)]).returncode:
                return (hit, comp, {"source": str(path.relative_to(ROOT)), "reason": "agbcc compile"}, unsc, res, 0)
            if run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-o", str(obj), str(src)]).returncode:
                return (hit, comp, {"source": str(path.relative_to(ROOT)), "reason": "assembler"}, unsc, res, 0)
            tu_cache_store(pre, bool(args.c89), src, obj)

        sections, all_relocs = fast_parse_elf(obj)
        if sections is None:
            sections = section_map(obj)
        source_text = path.read_text(encoding="utf-8", errors="replace")
        local_alias = {m.group(1): m.group(2) for m in ALIAS_DEF.finditer(source_text)}
        for name, vma, body, weak in tu_items:
            keys = [f".text.{name}"]
            # Alias spellings can be repeated in several TUs. The global
            # resolver is useful for calls, but the section belongs to this
            # particular source file's alias target.
            target = local_alias.get(name)
            if target:
                keys.append(f".text.{target}")
            blob = None
            calls = []
            pools = []
            for key in keys:
                if key in sections:
                    if all_relocs is not None:
                        relocs = all_relocs.get(key, [])
                    else:
                        relocs = relocations(obj, key)
                    blob, calls, pools = link_function(sections[key], vma, relocs, key, resolver)
                    break
            sizes = symbol_sizes(obj)
            if blob is None:
                unsc.append({"name": name, "vma": f"{vma:#010x}",
                             "reason": "compiled function section missing"})
                continue
            if vma not in spans:
                unsc.append({"name": name, "vma": f"{vma:#010x}",
                             "reason": "ROM function span not independently known"})
                continue
            rec = compare_function(rom, vma, name, blob, spans, calls, pools,
                                   sizes.get(name) or sizes.get(target or ""))
            if rec:
                rec["source"] = str(path.relative_to(ROOT))
                rec["body_stmts"] = body
                rec["alias_of"] = target
                rec["weak_only"] = weak
                res.append(rec)
        return (hit, comp, None, unsc, res, 1)

    tu_list = list(by_file.items())
    WORK.mkdir(parents=True, exist_ok=True)
    if len(tu_list) <= 1:
        tu_outputs = [process_one_tu(item) for item in tu_list]
    else:
        workers = min(os.cpu_count() or 4, len(tu_list), 12)
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            tu_outputs = list(pool.map(process_one_tu, tu_list))

    for hit, comp, skip, unsc, res, comp_f in tu_outputs:
        tu_cache_hits += hit
        tu_cache_compiles += comp
        if skip:
            skipped_files += 1
            skipped_reasons.append(skip)
        unscored.extend(unsc)
        results.extend(res)
        compiled_files += comp_f

    tally = collections.Counter(r["status"] for r in results)
    classes = collections.Counter(r["class"] for r in results if r.get("class"))
    total = len(results)
    exact = tally.get("EXACT", 0)
    real = [r for r in results if r.get("body_stmts", 0) >= 3]
    stubs = [r for r in results if r.get("body_stmts", 0) < 3]
    real_exact = sum(1 for r in real if r["status"] == "EXACT")
    stub_exact = sum(1 for r in stubs if r["status"] == "EXACT")

    # Near matches: real functions matching >= 90% bytes or with diff <= 4 bytes
    near_matches = [
        r for r in real
        if r["status"] != "EXACT"
        and r.get("cand_bytes") == r.get("rom_bytes")
        and (r.get("matched_bytes", 0) >= 0.9 * r.get("rom_bytes", 1)
             or (r.get("rom_bytes", 0) - r.get("matched_bytes", 0) <= 4))
    ]

    print(f"corpus byte-match probe: {total} src functions in {compiled_files} agbcc-compiled TUs "
          f"({skipped_files} skipped{' after C89 transform' if args.c89 else ': C99 constructs agbcc rejects'})\n")
    if tu_cache_hits or tu_cache_compiles:
        print(f"  TU cache: {tu_cache_hits} hit(s), {tu_cache_compiles} agbcc compile(s) "
              f"-- a hit is byte-identical output, keyed on the preprocessed source")
    print(f"  EXACT (whole body + pool) : {exact}/{total}  ({100*exact/total if total else 0:.0f}%)")
    print(f"    of which real bodies (>=3 statements) : {real_exact}/{len(real)}")
    print(f"    of which one-line stubs               : {stub_exact}/{len(stubs)}")
    if near_matches:
        print(f"    near matches (>=90% matched instructions): {len(near_matches)}/{len(real)}")
        for nm in sorted(near_matches, key=lambda x: -x.get("matched_bytes", 0)):
            print(f"      {nm['name']:<15} {nm['vma']} match={nm['matched_bytes']}/{nm['rom_bytes']} B "
                  f"first_diff=+0x{nm['first_diff']:02X} calls={len(nm['call_targets'])}")
    print()
    for status, n in tally.most_common():
        if status != "EXACT":
            print(f"  {status:<28} {n}")
    if classes:
        print("\nmiss classification:")
        for c, n in classes.most_common():
            print(f"  {c:<40} {n}")

    if args.function:
        for rec in results:
            diff = rec["first_diff"]
            where = "none" if diff is None else f"+0x{diff:X}"
            print(f"\n{rec['name']} {rec['vma']} {rec['source']}: {rec['status']} "
                  f"{rec['matched_bytes']}/{rec['rom_bytes']} bytes, "
                  f"candidate {rec['cand_bytes']} bytes, first difference {where}")
            for call in rec["call_targets"]:
                print(f"  call +0x{call['offset']:X}: {call['symbol']} -> "
                      f"{call['target_vma'] or 'UNRESOLVED'}")
            for pool in rec["pool_words"]:
                print(f"  pool +0x{pool['offset']:X}: {pool['symbol']} -> {pool['target_value']}")
    for skipped in skipped_reasons:
        print(f"  SKIPPED {skipped['source']}: {skipped['reason']}")
    if unscored:
        print(f"  UNSCORED functions: {len(unscored)}")
        if args.function:
            for item in unscored:
                print(f"    {item['name']} {item['vma']}: {item['reason']}")

    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(
            {"total": total, "exact": exact, "compiled_files": compiled_files,
             "skipped_files": skipped_files,
             "tu_cache_hits": tu_cache_hits,
             "tu_cache_compiles": tu_cache_compiles,
             "skipped_reasons": skipped_reasons,
             "selected_functions": len(best), "unscored": unscored,
             "real_bodies": len(real), "real_exact": real_exact,
             "stubs": len(stubs), "stub_exact": stub_exact,
             "near_matches": len(near_matches),
             "by_status": dict(tally), "by_class": dict(classes),
             "results": results}, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print(f"\nJSON: {args.json}")
    if args.function and not results:
        return 1
    if args.require_all and (skipped_files or unscored):
        return 1
    if args.require_exact and (not results or any(rec["status"] != "EXACT" for rec in results)):
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
