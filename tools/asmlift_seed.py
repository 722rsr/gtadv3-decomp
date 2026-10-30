#!/usr/bin/env python3
"""Generate agbcc-shaped C seeds for still-assembly bodies with asmlift.

The remaining decompilation backlog is 130 naked bodies plus 4 bodies with
instructions spliced in through `asm()`. They are already byte-exact, so the
corpus report cannot rank them and the permuter has nothing to permute. The
first move is still a C draft, and writing that draft from a 1,400-byte
listing is the expensive part of a sprint.

`asmlift` (https://github.com/macabeus/asmlift, MIT) is a matching decompiler
for agbcc Thumb: assembly in, C out, aimed at source that recompiles to the
same bytes. This adapter feeds it the ROM span, the project's real callee
arities, and a types context, then scores the seed against the fixed ROM span.
A seed is *not* a promotion: it is an advisory draft that still has to be
reviewed in its owning translation unit and pass the matching gates.

Why the adapter is not a one-line `npx asmlift`:

* The ROM's own `asm/*.s` text is a byte transcription, not agbcc output. Its
  labels are `_0800XXXX` aliases, branch mnemonics carry `.n`/`.w`, and calls
  are numeric (`bl 0x0802B368`). asmlift reads agbcc `.s`, so the adapter
  rewrites labels to local `.L` names, drops alignment directives that make
  the base alignment ambiguous in a text-only input, and resolves numeric
  calls to symbols.
* asmlift cannot know a callee's arity from a bare `bl`. The project already
  derives ROM arities in `arity_audit.py` and C declarations in
  `proto_audit.py`; the adapter merges both into a `--proto` sidecar so
  outgoing stack arguments lift.
* A seed is compiled by asmlift's own wrapper only when `--rank` is used.
  The cheap, project-shaped score is one compile of the seed as a standalone
  TU scored by `lift_loop.evaluate` against the manifest span. That measures
  the same fixed span the acceptance gates use and costs one compile, not
  asmlift's 2,000-candidate fan.

Usage:
    python3 tools/asmlift_seed.py _0801EFAC
    python3 tools/asmlift_seed.py 0x0801EFAC --rank --jobs 8
    python3 tools/asmlift_seed.py --queue --wall raw --limit 20
    python3 tools/asmlift_seed.py --queue --wall raw --json build/lift-seeds/queue.json
    python3 tools/asmlift_seed.py --install          # npm install into build/tools
    python3 tools/asmlift_seed.py --self-test

Outputs live under ignored `build/lift-seeds/`:

    <vma>/input.s      normalized agbcc-shaped assembly sent to asmlift
    <vma>/seed.c       asmlift output (structs renamed to AsmLift_*)
    <vma>/meta.json    resolution, normalization flags, seed score, commands
    <vma>/log.txt      asmlift stderr (declines are preserved verbatim)
    queue.json         one row per attempted body, sorted by aligned fraction
    queue.md           the same queue as a short human table

Nothing under `src/` is written. A seed is not selected source and never
changes the promotion manifest.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

ROOT = Path(__file__).resolve().parents[1]
ROM = ROOT / "baserom.gba"
QUALITY = ROOT / "docs/data/source_quality.json"
MANIFEST = ROOT / "tools/matching_slice_functions.json"
DEFAULT_OUT = ROOT / "build/lift-seeds"
TOOLS_BUILD = ROOT / "build/tools"
ASMLIFT_VERSION = "0.10.0"
ASMLIFT_REL = TOOLS_BUILD / "node_modules/.bin/asmlift"
PROTO_NAME = "proto.json"
CTX_NAME = "ctx.h"
CONFIG_NAME = "decomp.yaml"
COMPILE_NAME = "compile.sh"

# A VMA-shaped label: `sub_0801EFAC`, `_0801EFAC`, `0801EFAC`, and the
# `_lit` spelling tools/objdump2gas.py uses for a literal-pool word.
HEX_LABEL = re.compile(r"(?:sub_)?_?(0*(?:08|09)[0-9A-Fa-f]{5,7})(_lit)?$")
LABEL_LINE = re.compile(r"^([A-Za-z_][\w]*)\s*:(.*)$")
BRANCH = r"(?:b|bl|blx|beq|bne|bcs|bcc|bhi|bls|bge|blt|bgt|ble|bhs|blo)"
BRANCH_SUFFIX = re.compile(r"\b(" + BRANCH + r")\.(?:n|w)\b")
BRANCH_OPERAND = re.compile(
    r"(\b(?:ldr\s+r\d+\s*,\s*|" + BRANCH + r"\s+))"
    r"([A-Za-z_][\w]*|0x[0-9A-Fa-f]+)")
ALIGN_LINE = re.compile(r"^\s*\.(?:align|balign|p2align)\b")
PAD_LINE = re.compile(r"^\.(?:short|hword|2byte)\s+(?:0x0+|0)$")
PAD_INSN = {"movs r0, r0", "mov r0, r0", "nop"}
NUMERIC_CALL = re.compile(r"\b(bl|blx)\s+0x0?([0-9A-Fa-f]{6,8})\b")
ASMLIFT_ERROR = "ASMLIFT_ERROR"
STRUCT_NAME = re.compile(r"\b(?:struct|union)\s+(Struct\d+|Elem\d+)\b")
TYPE_WORD = re.compile(r"\b(Struct\d+|Elem\d+)\b")


# ---------------------------------------------------------------------------
# Inventory and resolution
# ---------------------------------------------------------------------------

def _load_json(path: Path, default):
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return default


def load_quality() -> dict:
    doc = _load_json(QUALITY, {})
    return doc.get("functions", {}) if isinstance(doc, dict) else {}


def load_manifest() -> dict:
    return _load_json(MANIFEST, {})


def parse_vma(value: str | int) -> int | None:
    """Accept `0x0801EFAC`, `_0801EFAC`, `sub_0801EFAC`, `801EFAC`."""
    if isinstance(value, int):
        return value if value >= 0x08000000 else None
    text = str(value).strip()
    match = re.fullmatch(r"(?:0x)?(?:sub_)?_?([0-9A-Fa-f]{6,9})", text)
    if not match:
        return None
    number = int(match.group(1), 16)
    if number < 0x08000000:
        number |= 0x08000000
    return number if 0x08000000 <= number < 0x0A000000 else None


def resolve(needle: str) -> dict:
    """Resolve a VMA or a source/body name to one backlog record.

    `source_quality.json` is the authoritative selected-entry inventory. A name
    may be the curated body name (`CarPhysRacer_InitFrame`) or a VMA alias
    (`_08021374`); both are accepted. A VMA missing from the snapshot falls
    back to the assembled inventory so a newly added body still resolves.
    """
    quality = load_quality()
    manifest = load_manifest()
    wanted_vma = parse_vma(needle)
    lowered = needle.lower()
    for key, rec in quality.items():
        try:
            vma = int(key, 16)
        except ValueError:
            continue
        names = {rec.get("name", ""), key, f"0x{vma:08x}", f"{vma:x}", f"{vma:X}"}
        entry = manifest.get(key) or {}
        names.update(entry.get("export") or [])
        names.update(entry.get("c_name") or [])
        if wanted_vma is not None:
            hit = vma == wanted_vma
        else:
            hit = lowered in {str(n).lower() for n in names if n}
        if not hit:
            continue
        return _record(vma, rec, entry)
    if wanted_vma is not None:
        for key, entry in manifest.items():
            try:
                vma = int(key, 16)
            except ValueError:
                continue
            if vma != wanted_vma:
                continue
            return _record(vma, {"name": entry.get("c_name") or entry.get("export", [""])[0],
                                 "file": entry.get("source", ""),
                                 "size": 0, "class": "asm"}, entry)
    raise SystemExit(
        f"asmlift_seed: no selected body matches {needle!r}; "
        "try a VMA from `python3 tools/wall_census.py` or a name from "
        "`python3 tools/source_quality.py --list asm`")


def _record(vma: int, rec: dict, entry: dict) -> dict:
    size = int(rec.get("size") or 0)
    if not size and entry.get("end_vma"):
        size = int(entry["end_vma"], 16) - vma
    aliases = list(dict.fromkeys((entry.get("export") or []) +
                                ([entry["c_name"]] if entry.get("c_name") else [])))
    return {
        "vma": vma,
        "size": size,
        "name": rec.get("name") or entry.get("c_name") or f"_0800{vma & 0xFFFFFF:06X}",
        "file": rec.get("file") or entry.get("source") or "",
        "class": rec.get("class") or "asm",
        "aliases": aliases,
        "manifest": entry,
    }


def backlog(wall: str = "raw", klass: str = "all",
            census_rows: list[dict] | None = None) -> list[dict]:
    """Selected-but-assembly bodies with their wall class, largest first.

    `census_rows` lets a queue pay the live source scan once and share it.
    The live census is the default because a stale snapshot can name a body
    that was promoted since the last `make progress`.
    """
    import wall_census

    rows = census_rows if census_rows is not None else wall_census.census(snapshot=False)
    quality = load_quality()
    manifest = load_manifest()
    out = []
    for row in rows:
        if klass != "all" and row.get("class") != klass:
            continue
        if wall not in ("all", row.get("wall")):
            continue
        key = f"0x{int(row['vma']):08x}"
        rec = quality.get(key, {})
        out.append(_record(int(row["vma"]), rec, manifest.get(key, {})))
    out.sort(key=lambda r: (-r["size"], r["vma"]))
    return out


# ---------------------------------------------------------------------------
# asmlift provisioning and configuration
# ---------------------------------------------------------------------------

def asmlift_path(explicit: str | None = None) -> Path | None:
    candidates = []
    if explicit:
        candidates.append(Path(explicit))
    env = os.environ.get("ASMLIFT")
    if env:
        candidates.append(Path(env))
    candidates.append(ASMLIFT_REL)
    found = shutil.which("asmlift")
    if found:
        candidates.append(Path(found))
    for path in candidates:
        if path.is_file() and os.access(path, os.X_OK):
            return path
    return None


def install_asmlift(version: str = ASMLIFT_VERSION) -> int:
    npm = shutil.which("npm")
    if not npm:
        print("asmlift_seed: npm is required to install asmlift; "
              "install Node.js (https://nodejs.org/) or pass --asmlift PATH",
              file=sys.stderr)
        return 1
    TOOLS_BUILD.mkdir(parents=True, exist_ok=True)
    package = f"@asmlift/cli@{version}"
    print(f"asmlift_seed: npm install --prefix {TOOLS_BUILD.relative_to(ROOT)} {package}")
    result = subprocess.run([npm, "install", "--prefix", str(TOOLS_BUILD), package])
    if result.returncode:
        return result.returncode
    path = asmlift_path()
    if path is None:
        print("asmlift_seed: install finished but the executable was not found",
              file=sys.stderr)
        return 1
    print(f"asmlift_seed: {path} ({_version(path)})")
    return 0


def _version(path: Path) -> str:
    try:
        out = subprocess.run([str(path), "--version"], capture_output=True,
                             text=True, timeout=30).stdout.strip()
        return out or "unknown"
    except (OSError, subprocess.SubprocessError):
        return "unknown"


def ensure_config(out_dir: Path) -> dict:
    """Write the small generated asmlift project files; return their paths.

    Paths in `decomp.yaml` are absolute: asmlift runs the compiler from a
    temporary directory, so a path relative to the YAML would resolve against
    the wrong cwd. The compile wrapper resolves the repo root from its own
    location, so the generated tree stays relocatable.
    """
    out_dir.mkdir(parents=True, exist_ok=True)
    ctx = out_dir / CTX_NAME
    if not ctx.exists() or "#include \"gba/types.h\"" not in ctx.read_text():
        ctx.write_text('#include "gba/types.h"\n', encoding="utf-8")
    script = out_dir / COMPILE_NAME
    script.write_text(
        "#!/bin/sh\n"
        "set -e\n"
        f"root='{ROOT}'\n"
        'in=$1\n'
        'out=$2\n'
        "shift 2\n"
        'clang -E -nostdinc -undef -I"$root/include" -I"$root/asm" '
        '-I"$root/build/toolchains/agbcc/ginclude" "$in" -o "$out.i"\n'
        '"$root/build/toolchains/agbcc/old_agbcc" "$@" "$out.i" -o "$out.s"\n'
        'arm-none-eabi-as -mcpu=arm7tdmi -mthumb-interwork "$out.s" -o "$out"\n',
        encoding="utf-8")
    script.chmod(0o755)
    config = out_dir / CONFIG_NAME
    config.write_text(
        "name: GT Advance 3 seed lifting\n"
        "platform: gba\n"
        "versions:\n"
        "  - name: us\n"
        "    fullname: US\n"
        "    paths:\n"
        f"      target: {ROOT / 'baserom.gba'}\n"
        f"      build_dir: {ROOT / 'build'}\n"
        f"      map: {ROOT / 'build/rom.map'}\n"
        f"      compiled_target: {ROOT / 'build/gtadv3.gba'}\n"
        "tools:\n"
        "  asmlift:\n"
        "    target: agbcc\n"
        "    compiler: |\n"
        "      set -e\n"
        f"      {script} {{{{inputPath}}}} {{{{outputPath}}}} {{{{flags}}}}\n",
        encoding="utf-8")
    return {"config": config, "context": ctx, "compile": script}


def proto_sidecar(out_dir: Path, refresh: bool = False) -> Path:
    """Merge ROM arities and C declarations into asmlift's `--proto` JSON.

    asmlift reads a callee's arity to decide whether a `str rX, [sp, #N]` is
    an outgoing stack argument. `arity_audit.rom_arity_table()` derives that
    from the callee's own prologue; `proto_audit.collect()` supplies the C
    parameter shapes where the tree has a single consistent declaration.
    The ROM arity wins on conflicts because the seed must reflect the bytes,
    not a possibly stale prototype.
    """
    path = out_dir / PROTO_NAME
    sig_path = out_dir / (PROTO_NAME + ".sig")
    signature = _proto_signature()
    if path.exists() and sig_path.exists() and not refresh:
        try:
            if sig_path.read_text(encoding="utf-8").strip() == signature:
                return path
        except OSError:
            pass
    for module in ("proto_audit", "arity_audit"):
        if module not in sys.modules:
            __import__(module)
    import proto_audit
    import arity_audit

    proto: dict[str, dict] = {}
    found, _ = proto_audit.collect([str(ROOT / "src"), str(ROOT / "include")])
    for name, entries in found.items():
        shapes = collections.Counter((e[2], e[3]) for e in entries)
        if len(shapes) != 1:
            continue
        ret, params = next(iter(shapes))
        entry = {"params": list(params)}
        if ret == "void":
            entry["returnsVoid"] = True
        else:
            entry["returns"] = ret
        proto[name] = entry
    for norm, row in arity_audit.rom_arity_table().items():
        arity = row[0]
        if arity is None or arity < 0:
            continue
        for spelling in arity_audit.spellings(row[4]):
            if spelling not in proto:
                proto[spelling] = {"params": min(arity, 16)}
    path.write_text(json.dumps(proto, indent=0) + "\n", encoding="utf-8")
    sig_path.write_text(signature + "\n", encoding="utf-8")
    return path


def _proto_signature() -> str:
    digest = hashlib.sha256()
    for base in ("src", "include", "asm"):
        directory = ROOT / base
        if not directory.is_dir():
            continue
        for file in sorted(directory.rglob("*")):
            if not file.is_file() or file.suffix not in (".c", ".h", ".s", ".inc"):
                continue
            try:
                stat = file.stat()
            except OSError:
                continue
            digest.update(f"{file.relative_to(ROOT)}:{stat.st_size}:{stat.st_mtime_ns}\n".encode())
    return digest.hexdigest()


# ---------------------------------------------------------------------------
# Assembly normalization
# ---------------------------------------------------------------------------

def canonical_label(name: str) -> str | None:
    """`.L0801EFAC` for a VMA-shaped label; None otherwise."""
    match = HEX_LABEL.fullmatch(name)
    if not match:
        return None
    return ".L" + match.group(1) + (match.group(2) or "")


_ROM_BYTES: bytes | None = None


def _rom_bytes() -> bytes:
    """Read the ROM once per process.

    A queue reads the same 8 MiB image for every body; re-reading it per
    slice was measurable in the 68-body sweep and needless.
    """
    global _ROM_BYTES
    if _ROM_BYTES is None:
        _ROM_BYTES = ROM.read_bytes()
    return _ROM_BYTES


def _rom_slice(vma: int, size: int) -> bytes:
    offset = vma - 0x08000000
    data = _rom_bytes()
    if offset < 0 or size <= 0 or offset + size > len(data):
        raise SystemExit(f"asmlift_seed: span {vma:#010x}+{size} is outside the ROM")
    return data[offset:offset + size]


def extracted_asm(vma: int, size: int, work: Path) -> str:
    """ROM span -> byte-exact gas text via the proven objdump2gas path."""
    work.mkdir(parents=True, exist_ok=True)
    blob = work / f"{vma:08x}.bin"
    blob.write_bytes(_rom_slice(vma, size))
    listing = work / f"{vma:08x}.lst"
    with listing.open("w", encoding="utf-8") as handle:
        subprocess.run(["arm-none-eabi-objdump", "-D", "-b", "binary", "-m", "arm",
                        "-M", "force-thumb", f"--adjust-vma={vma:#x}", str(blob)],
                       stdout=handle, check=True)
    out = work / f"{vma:08x}.raw.s"
    result = subprocess.run([sys.executable, str(ROOT / "tools/objdump2gas.py"),
                             "--start", hex(vma - 0x08000000),
                             "--end", hex(vma - 0x08000000 + size),
                             "--lst", str(listing), "--out", str(out)],
                            capture_output=True, text=True)
    if result.returncode:
        raise SystemExit("asmlift_seed: objdump2gas failed: " +
                         (result.stderr.strip().splitlines() or ["?"])[-1][:160])
    return out.read_text(encoding="utf-8")


def normalize_asm(text: str, *, vma: int, size: int, name: str,
                  aliases: list[str], tail_calls: bool = False,
                  entries: set[int] | None = None) -> tuple[str, dict]:
    """Turn a ROM transcription into agbcc-shaped text asmlift can read.

    The returned text always starts at a single global function label named
    `name`, so an asmlift seed's section and the owning translation unit agree.
    Labels are local `.L` names because that is the one label shape asmlift's
    Thumb frontend accepts for an intra-function block. Alignment directives
    are dropped: a text input has no base address, and `.align 2, 0` inside the
    span makes the frontend report two consistent decodes at different bases.
    """
    meta = {"align_removed": 0, "pads_removed": 0, "numeric_calls": 0,
            "labels_renamed": 0, "tail_calls": 0}
    lines = []
    for raw in text.splitlines():
        stripped = raw.strip()
        if not stripped or stripped.startswith("@"):
            continue
        if stripped.startswith((".type", ".global", ".section", ".size")):
            continue
        if ALIGN_LINE.match(stripped):
            meta["align_removed"] += 1
            continue
        lines.append(raw.rstrip())

    primary = {name, *aliases}
    primary_canon = {canonical_label(p) for p in primary if canonical_label(p)}
    known: set[str] = set()
    for line in lines:
        match = LABEL_LINE.match(line.strip())
        if not match:
            continue
        canon = canonical_label(match.group(1))
        if canon and canon not in primary_canon:
            known.add(canon)

    body: list[str] = []
    emitted: set[str] = set()

    def rewrite(match: re.Match) -> str:
        prefix, target = match.group(1), match.group(2)
        if target.lower().startswith("0x"):
            value = int(target, 16)
            if prefix.split()[0] in ("bl", "blx"):
                meta["numeric_calls"] += 1
                return prefix + "sub_%08X" % value
            return match.group(0)
        canon = canonical_label(target)
        if canon and canon in known:
            return prefix + canon
        return match.group(0)

    for line in lines:
        stripped = line.strip()
        match = LABEL_LINE.match(stripped)
        if match:
            label, rest = match.group(1), match.group(2).strip()
            is_primary = label in primary or canonical_label(label) in primary_canon
            if is_primary:
                if rest:
                    body.append("\t" + rest)
                continue
            canon = canonical_label(label) or ".L_" + re.sub(r"\W", "_", label)
            if canon in emitted:
                if rest:
                    body.append("\t" + rest)
                continue
            emitted.add(canon)
            meta["labels_renamed"] += 1
            line = f"{canon}:" + (("\t" + rest) if rest else "")
        line = BRANCH_SUFFIX.sub(r"\1", line)
        line = BRANCH_OPERAND.sub(rewrite, line)
        line = NUMERIC_CALL.sub(
            lambda m: f"{m.group(1)} sub_{int(m.group(2), 16):08X}", line)
        if tail_calls and entries and re.match(r"^\s*b\s+", line):
            target = line.split()[-1]
            target_vma = parse_vma(target) if target.lower().startswith("0x") else None
            if target_vma and target_vma in entries and not (vma <= target_vma < vma + size):
                meta["tail_calls"] += 1
                body.extend(["\tbl sub_%08X" % target_vma, "\tbx lr"])
                continue
        body.append(line)

    while body:
        last = body[-1].strip()
        if last in PAD_INSN or PAD_LINE.match(last):
            body.pop()
            meta["pads_removed"] += 1
            continue
        break
    header = [".text", ".code 16", ".global " + name,
              ".type " + name + ", %function", ".thumb_func", name + ":"]
    return "\n".join(header + body) + "\n", meta


def rename_seed_types(seed: str) -> str:
    """Prefix asmlift's synthetic struct names so a splice cannot collide."""
    seed = STRUCT_NAME.sub(lambda m: f"struct AsmLift_{m.group(1)}", seed)
    seed = TYPE_WORD.sub(lambda m: f"AsmLift_{m.group(1)}", seed)
    return seed


def seed_has_error(seed: str) -> bool:
    return ASMLIFT_ERROR in seed


# ---------------------------------------------------------------------------
# asmlift invocation and scoring
# ---------------------------------------------------------------------------

def _cflags(record: dict) -> str:
    """The owning TU's agbcc flags, including the interworking exception."""
    from agbcc_options import options_for

    return " ".join(options_for(record["file"] or "default.c"))


def run_asmlift(executable: Path, input_path: Path, out_dir: Path, record: dict,
                config: dict, proto: Path, *, rank: bool = False,
                jobs: int = 1, tail_calls: bool = False) -> dict:
    """Run asmlift once; return its seed and verbatim diagnostics.

    `--score-against` is asmlift's own fan-out (thousands of candidate
    compiles). It is opt-in as `--rank`; the default score is one compile of
    the returned seed through `lift_loop`, which is faster and uses the same
    fixed ROM span as the project gates.
    """
    target = None
    if rank:
        target = build_target_object(record, out_dir)
    args = [str(executable), str(input_path), "--name", record["name"],
            "--config", str(config["config"]), "--context", str(config["context"]),
            "--proto", str(proto), "--target", "agbcc",
            "--cflags", _cflags(record)]
    if target is not None:
        args += ["--score-against", str(target), "--jobs", str(max(1, jobs))]
    started = time.time()
    try:
        result = subprocess.run(args, capture_output=True, text=True,
                                timeout=600, cwd=str(ROOT))
    except subprocess.TimeoutExpired:
        return {"status": "timeout", "seed": "", "stdout": "", "stderr": "",
                "seconds": round(time.time() - started, 2)}
    seed = rename_seed_types(result.stdout)
    # An empty stdout is a decline too. Treating it as "ok" wrote a zero-byte
    # seed and then scored an empty object, which reads as a compiler fault
    # rather than the diagnostic it is.
    status = "declined" if (not seed.strip() or seed_has_error(seed)) else "ok"
    return {"status": status, "seed": seed, "stdout": result.stdout,
            "stderr": result.stderr, "seconds": round(time.time() - started, 2),
            "returncode": result.returncode, "target": str(target) if target else ""}


def build_target_object(record: dict, out_dir: Path) -> Path:
    """A ROM-addressed ELF object for asmlift's own `--score-against`."""
    from run_permuter import mappings
    import objdiff_build

    code = ROOT / "build-code/code.o"
    if not code.exists():
        raise SystemExit("asmlift_seed: --rank needs build-code/code.o; "
                         "run `make code-objects`")
    offset = record["vma"] - 0x08000000
    size = record["size"]
    modes = mappings(code, ".text", offset, size)
    if not modes or modes[0][1] != "$t":
        raise SystemExit(f"asmlift_seed: {record['vma']:#x} is not a Thumb span")
    target = out_dir / "target.o"
    objdiff_build.named_object(_rom_slice(record["vma"], size), record["vma"],
                               modes, record["name"], target)
    return target


def score_seed(record: dict, seed: str, work: Path) -> dict:
    """Compile the seed as a standalone TU and compare it with the ROM span.

    The seed is self-contained (asmlift emits raw `u8`/`s32`/pointer access and
    numeric calls), so it does not need the owning TU's headers. That isolation
    is deliberate: a project header may declare the function with a narrower
    prototype than the ROM uses, and that conflict would reject a seed whose
    *shape* is exactly what the sprint needs to inspect.
    """
    import lift_loop

    work.mkdir(parents=True, exist_ok=True)
    tu = '#include "gba/types.h"\n' + seed
    try:
        result = lift_loop.evaluate(tu, Path(record["file"] or "seed.c"),
                                    record["name"], record["vma"],
                                    record["size"], work)
    except SystemExit as exc:
        return {"kind": "error", "msg": str(exc)[:300]}
    except Exception as exc:  # noqa: BLE001 - one bad seed must not end a queue run
        return {"kind": "error", "msg": f"{type(exc).__name__}: {exc}"[:300]}
    out = {"kind": result["kind"], "candidate_bytes": result["size"],
           "aligned": result["aligned"]}
    if result["aligned"] >= 0:
        estimate = max(1, record["size"] // 2)
        out["aligned_percent"] = round(100 * result["aligned"] / estimate, 1)
    if result.get("msg"):
        out["error"] = result["msg"][:300]
    try:
        from lift_loop import byte_score, rom_slice
        matched, first = byte_score(result["blob"],
                                    rom_slice(record["vma"], record["size"] or len(result["blob"])))
        out["bytes"] = matched
        out["first_diff"] = first
    except Exception:  # noqa: BLE001 - byte counts are advisory detail
        pass
    return out


# ---------------------------------------------------------------------------
# One body and the queue
# ---------------------------------------------------------------------------

def generate(record: dict, out_root: Path, executable: Path, *,
             rank: bool = False, score: bool = True, jobs: int = 1,
             tail_calls: bool = False, force: bool = False) -> dict:
    vma = record["vma"]
    work = out_root / f"{vma:08x}"
    if work.exists() and not force:
        meta_path = work / "meta.json"
        if meta_path.exists():
            cached = _load_json(meta_path, {})
            if cached.get("tool") == "asmlift_seed" and cached.get("vma") == f"0x{vma:08x}":
                return cached
    work.mkdir(parents=True, exist_ok=True)
    config = ensure_config(out_root)
    proto = proto_sidecar(out_root)
    entries = {int(key, 16) for key in load_quality()}
    started = time.time()
    meta = {
        "tool": "asmlift_seed",
        "vma": f"0x{vma:08x}",
        "name": record["name"],
        "file": record["file"],
        "class": record["class"],
        "size": record["size"],
        "aliases": record["aliases"],
    }
    try:
        raw = extracted_asm(vma, record["size"], work / "asm")
        text, flags = normalize_asm(raw, vma=vma, size=record["size"],
                                    name=record["name"], aliases=record["aliases"],
                                    tail_calls=tail_calls, entries=entries)
    except (OSError, subprocess.SubprocessError, SystemExit) as exc:
        meta.update(status="input-error", error=str(exc)[:300])
        _write_meta(work, meta)
        return meta
    input_path = work / "input.s"
    input_path.write_text(text, encoding="utf-8")
    meta["normalization"] = flags
    result = run_asmlift(executable, input_path, work, record, config, proto,
                         rank=rank, jobs=jobs, tail_calls=tail_calls)
    meta["asmlift_seconds"] = result.get("seconds")
    meta["status"] = result["status"]
    if result.get("target"):
        meta["target_object"] = result["target"]
    if result["status"] != "ok":
        meta["error"] = _decline_line(result.get("stderr", ""))
        (work / "log.txt").write_text(result.get("stderr", ""), encoding="utf-8")
        _write_meta(work, meta)
        return meta
    seed = result["seed"]
    (work / "seed.c").write_text(seed, encoding="utf-8")
    (work / "log.txt").write_text(result.get("stderr", ""), encoding="utf-8")
    if score:
        meta["score"] = score_seed(record, seed, work / "score")
    meta["seconds"] = round(time.time() - started, 2)
    meta["commands"] = packet_commands(record, work)
    _write_meta(work, meta)
    return meta


def _decline_line(stderr: str) -> str:
    lines = [l for l in stderr.splitlines() if "[lift]" in l or "[internal]" in l]
    if lines:
        return lines[-1].split(": ", 1)[-1][:300]
    lines = [l for l in stderr.splitlines() if l.strip()]
    return (lines[-1] if lines else "asmlift declined")[:300]


def _write_meta(work: Path, meta: dict) -> None:
    (work / "meta.json").write_text(json.dumps(meta, indent=2) + "\n",
                                    encoding="utf-8")


def packet_commands(record: dict, work: Path) -> dict:
    try:
        rel = work.relative_to(ROOT)
    except ValueError:
        rel = work
    return {
        "rescore_seed": f"python3 tools/asmlift_seed.py 0x{record['vma']:08x} --force",
        "splice_and_score": f"python3 tools/lift_loop.py {record['file']} "
                            f"{record['name']} --variant {rel}/seed.c --diff",
        "context": f"python3 tools/lift_packet.py 0x{record['vma']:08x}",
        "wall": "python3 tools/wall_census.py",
        "accept": "make matching-ready && make full-rom-match",
    }


def queue(out_root: Path, executable: Path, *, wall: str = "raw",
          klass: str = "all", limit: int = 0, jobs: int = 1,
          rank: bool = False, score: bool = True, tail_calls: bool = False,
          force: bool = False) -> list[dict]:
    rows = backlog(wall=wall, klass=klass)
    if limit:
        rows = rows[:limit]
    if not rows:
        return []
    # The asmlift runs are independent processes. Each one writes its own
    # body directory, so a thread pool is safe; protoc/config writes happen
    # once before the pool starts.
    ensure_config(out_root)
    proto_sidecar(out_root)
    results = []
    with ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        futures = {
            pool.submit(generate, record, out_root, executable, rank=rank,
                        score=score, jobs=1, tail_calls=tail_calls,
                        force=force): record
            for record in rows
        }
        for future in as_completed(futures):
            record = futures[future]
            try:
                results.append(future.result())
            except Exception as exc:  # noqa: BLE001 - report, do not abort the queue
                results.append({"tool": "asmlift_seed", "vma": f"0x{record['vma']:08x}",
                                "name": record["name"], "file": record["file"],
                                "size": record["size"], "status": "error",
                                "error": f"{type(exc).__name__}: {exc}"[:300]})
    # Best-first by the fraction of the span's instructions the seed already
    # aligns, then by payoff. A 1.4 KB seed at 27% is a better next target than
    # a 300 B seed at 10%; size alone would invert that.
    def rank(row: dict) -> tuple:
        score = row.get("score") or {}
        aligned = score.get("aligned") or 0
        if aligned < 0:
            aligned = 0
        estimate = max(1, (row.get("size") or 0) // 2)
        return (row.get("status") != "ok", -(aligned / estimate),
                -(row.get("size") or 0), row.get("vma", ""))

    results.sort(key=rank)
    _write_queue(out_root, results)
    return results


def _write_queue(out_root: Path, rows: list[dict]) -> None:
    out_root.mkdir(parents=True, exist_ok=True)
    (out_root / "queue.json").write_text(json.dumps({
        "tool": "asmlift_seed",
        "generated": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "rows": rows,
    }, indent=2) + "\n", encoding="utf-8")
    ok = [r for r in rows if r.get("status") == "ok"]
    lines = ["# asmlift seed queue", "",
             f"{len(ok)} seeds from {len(rows)} bodies "
             f"({sum(r.get('size', 0) for r in ok):,} ROM bytes).", "",
             "| ROM | body | seed result | score |",
             "| ---: | --- | --- | --- |"]
    for row in rows:
        score = row.get("score") or {}
        state = row.get("status", "?")
        if score and score.get("kind") != "error" and (score.get("aligned") or 0) >= 0:
            aligned = score.get("aligned") or 0
            estimate = max(1, (row.get("size") or 0) // 2)
            detail = (f"{score.get('kind', '?')}, {aligned} aligned "
                      f"({aligned / estimate:.0%}), "
                      f"{score.get('candidate_bytes', '?')} B")
        elif score:
            detail = "seed did not compile: " + str(score.get("error") or score.get("msg") or "")[:60]
        else:
            detail = ""
        if state != "ok":
            detail = row.get("error", "")[:80]
        lines.append(f"| {row.get('size', 0):,} | `{row.get('name', '?')}` "
                     f"`{row.get('vma', '?')}` | {state} | {detail} |")
    (out_root / "queue.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


# ---------------------------------------------------------------------------
# Self-test
# ---------------------------------------------------------------------------

def self_test() -> int:
    checks = []

    def check(name: str, got, want) -> None:
        checks.append((name, got == want, got, want))

    check("parse 0x", parse_vma("0x0801EFAC"), 0x0801EFAC)
    check("parse underscore", parse_vma("_0801EFAC"), 0x0801EFAC)
    check("parse sub_", parse_vma("sub_0801EFAC"), 0x0801EFAC)
    check("parse short", parse_vma("801EFAC"), 0x0801EFAC)
    check("canonical label", canonical_label("_0801EFAC"), ".L0801EFAC")
    check("canonical lit", canonical_label("_0800B54C_lit"), ".L0800B54C_lit")
    check("canonical friendly", canonical_label("Race_Dispatch"), None)

    sample = """
@ comment
\t.type sub_08001000, %function
sub_08001000:
_08001000:
\tpush\t{r4, lr}
\tldr\tr0, _08001020
\tcmp\tr0, #0
\tbne.n\t_08001010
\tbl\t0x08002000
\t.align 2, 0
_08001010:
\tb.n\t_08001018
_08001018:
\tpop\t{r4}
\tpop\t{r0}
\tbx\tr0
_08001020: .4byte 0x12345678
\t.short 0x0000
"""
    text, flags = normalize_asm(sample, vma=0x08001000, size=0x24,
                                name="_08001000", aliases=["sub_08001000"])
    check("primary label", text.count("_08001000:"), 1)
    check("local branch", ".L08001010" in text, True)
    check("numeric call", re.search(r"\bbl\s+sub_08002000\b", text) is not None, True)
    check("align removed", flags["align_removed"], 1)
    check("pad removed", flags["pads_removed"], 1)
    check("pool kept", "0x12345678" in text, True)
    check("no suffix", ".n " not in text and ".w " not in text, True)

    seed = "struct Struct0 { u8 x; };\nvoid f(struct Struct0 *a) {}\n"
    renamed = rename_seed_types(seed)
    check("struct renamed", "struct AsmLift_Struct0" in renamed, True)
    check("error marker", seed_has_error("/* ASMLIFT_ERROR('x') */"), True)

    failed = [c for c in checks if not c[1]]
    for name, ok, got, want in checks:
        print(f"  {'PASS' if ok else 'FAIL'} {name}: got {got!r} want {want!r}")
    print(f"asmlift_seed self-test: {'PASS' if not failed else 'FAIL'} "
          f"({len(checks) - len(failed)}/{len(checks)})")
    return 0 if not failed else 1


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("function", nargs="?", help="VMA or body name to seed")
    ap.add_argument("--queue", action="store_true",
                    help="attempt every backlog body in the selected wall/class")
    ap.add_argument("--wall", default="raw",
                    choices=["raw", "measured", "regalloc", "unreachable",
                             "veneer", "unexplained", "all"],
                    help="wall class to include in --queue (default: raw)")
    ap.add_argument("--class", dest="klass", default="all",
                    choices=["asm", "partial", "all"],
                    help="source-quality class to include in --queue")
    ap.add_argument("--limit", type=int, default=0,
                    help="max bodies in --queue (0 = all)")
    ap.add_argument("--jobs", type=int, default=0,
                    help="parallel asmlift processes (0 = min(8, cpu))")
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT,
                    help=f"output root (default: {DEFAULT_OUT.relative_to(ROOT)})")
    ap.add_argument("--rank", action="store_true",
                    help="let asmlift fan out and rank candidates (slower, higher quality)")
    ap.add_argument("--no-score", action="store_true",
                    help="skip the one-compile standalone seed score")
    ap.add_argument("--tail-calls", action="store_true",
                    help="rewrite an external branch to a known entry as bl+bx lr")
    ap.add_argument("--force", action="store_true", help="overwrite an existing body directory")
    ap.add_argument("--asmlift", metavar="PATH", help="explicit asmlift executable")
    ap.add_argument("--score-file", type=Path, metavar="FILE",
                    help="score an edited seed file against the selected span; "
                         "does not need asmlift")
    ap.add_argument("--install", action="store_true", help="npm install asmlift into build/tools")
    ap.add_argument("--version", default=ASMLIFT_VERSION,
                    help=f"asmlift npm version to install (default {ASMLIFT_VERSION})")
    ap.add_argument("--json", type=Path, help="write the single-body result or queue here")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)

    if args.self_test:
        return self_test()
    if args.install:
        return install_asmlift(args.version)
    if args.score_file:
        if not args.function:
            ap.error("--score-file needs a function or VMA")
        record = resolve(args.function)
        try:
            seed = (Path("-") if str(args.score_file) == "-" else args.score_file)
            text = sys.stdin.read() if str(seed) == "-" else seed.read_text(encoding="utf-8")
        except OSError as exc:
            print(f"asmlift_seed: cannot read seed: {exc}", file=sys.stderr)
            return 1
        out = args.out if args.out.is_absolute() else ROOT / args.out
        result = score_seed(record, rename_seed_types(text),
                            out / f"{record['vma']:08x}" / "edited-score")
        payload = {"vma": f"0x{record['vma']:08x}", "name": record["name"],
                   "size": record["size"], "score": result}
        print(json.dumps(payload, indent=2))
        if args.json:
            args.json.parent.mkdir(parents=True, exist_ok=True)
            args.json.write_text(json.dumps(payload, indent=2) + "\n",
                                 encoding="utf-8")
        return 0 if result.get("kind") not in ("error", None) else 1
    executable = asmlift_path(args.asmlift)
    if executable is None:
        print("asmlift_seed: asmlift was not found. Install it with:\n"
              "  python3 tools/asmlift_seed.py --install\n"
              "or pass --asmlift PATH.", file=sys.stderr)
        return 1
    jobs = args.jobs or max(1, min(8, os.cpu_count() or 2))
    out = args.out if args.out.is_absolute() else ROOT / args.out
    if args.queue:
        rows = queue(out, executable, wall=args.wall, klass=args.klass,
                     limit=args.limit, jobs=jobs, rank=args.rank,
                     score=not args.no_score, tail_calls=args.tail_calls,
                     force=args.force)
        ok = [r for r in rows if r.get("status") == "ok"]
        print(f"asmlift_seed: {len(ok)}/{len(rows)} seeds, "
              f"{sum(r.get('size', 0) for r in ok):,} ROM bytes")
        print(f"queue: {out / 'queue.md'}")
        if args.json:
            args.json.parent.mkdir(parents=True, exist_ok=True)
            args.json.write_text(json.dumps({"rows": rows}, indent=2) + "\n",
                                 encoding="utf-8")
        return 0
    if not args.function:
        ap.error("provide a function, or use --queue")
    record = resolve(args.function)
    result = generate(record, out, executable, rank=args.rank,
                      score=not args.no_score, jobs=jobs,
                      tail_calls=args.tail_calls, force=args.force)
    print(json.dumps(result, indent=2))
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    return 0 if result.get("status") == "ok" else 1


if __name__ == "__main__":
    sys.exit(main())
