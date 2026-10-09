#!/usr/bin/env python3
"""Prepare and run an advisory agbcc permuter scratch; never edit game source.

Wrapper options may follow FUNCTION. Pass fork options after `--`, e.g.:
  python3 tools/run_permuter.py _08006C10 --prepare-only
  python3 tools/run_permuter.py _08006C10 -- --debug

The original preprocessed translation unit surrounds each mutated body, retaining
aliases, attributes, helper definitions and globals. Only the selected function's
resolved bytes are exposed to the permuter scorer. A score of zero is NOT the
project's exact-byte or independent-link acceptance gate.
"""
from __future__ import annotations

import argparse
import base64
import hashlib
import json
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

from elftools.elf.elffile import ELFFile

import corpus_match_probe as probe
from agbcc_c89_transform import matching_delimiter, strip_noncode

ROOT = probe.ROOT
PERMUTER_DIR = ROOT / "build/toolchains/decomp-permuter-agbcc"
FLAGS = ["-O2", "-mthumb-interwork", "-ffunction-sections"]


def run(argv, **kwargs):
    return subprocess.run([str(a) for a in argv], check=True, **kwargs)


def extension_spans(source: str):
    masked = strip_noncode(source)
    pattern = r"\b(?:__attribute__|__attribute|__asm__|__asm|asm)\s*(?:volatile\s*)?\("
    for match in re.finditer(pattern, masked):
        start = masked.index("(", match.start(), match.end())
        end = matching_delimiter(masked, start, "(", ")")
        if end is None:
            raise ValueError("unbalanced GNU extension")
        yield match.start(), end + 1


def plain_context(source: str) -> str:
    # Parser-only declarations. Actual compilation uses the untouched TU.
    for start, end in reversed(list(extension_spans(source))):
        source = source[:start] + " " * (end - start) + source[end:]
    return source


def declaration_bounds(masked: str, lo: int) -> tuple[int, int]:
    """Whole-declaration span around an asm declarator at `lo` (masked text).

    Statement positions are separated from declarator positions by the token
    before the extension; a declarator runs from the previous `;`/`{`/`}`
    outside parentheses to the terminating `;` outside parentheses/braces.
    """
    start, paren = 0, 0
    for i in range(lo):
        c = masked[i]
        if c == "(":
            paren += 1
        elif c == ")":
            paren -= 1
        elif paren == 0 and c in ";{}":
            start = i + 1
    paren = brace = 0
    for i in range(lo, len(masked)):
        c = masked[i]
        if c == "(":
            paren += 1
        elif c == ")":
            paren -= 1
        elif c == "{":
            brace += 1
        elif c == "}":
            brace -= 1
        elif paren == 0 and brace == 0:
            if c == ",":
                raise ValueError("unsupported target: multi-declarator asm declaration")
            if c == ";":
                return start, i + 1
    raise ValueError("unterminated asm declarator declaration")


def function_ranges(source: str):
    """Top-level definitions, using balanced delimiters rather than line regexes."""
    masked = strip_noncode(plain_context(source))
    start = pos = 0
    while pos < len(masked):
        if masked[pos] == ";":
            start = pos + 1
        elif masked[pos] == "{":
            end = matching_delimiter(masked, pos, "{", "}")
            if end is None:
                raise ValueError("unbalanced top-level body")
            header = masked[start:pos]
            match = re.search(r"\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*$", header)
            if match and "=" not in header:
                yield match[1], start, pos, end + 1
                start = end + 1
            pos = end
        pos += 1


def prepare_source(source: str, name: str) -> tuple[str, str, str]:
    ranges = list(function_ranges(source))
    selected = [row for row in ranges if row[0] == name]
    if len(selected) != 1:
        raise ValueError(f"expected one definition of {name}, found {len(selected)}")
    _, start, opening, end = selected[0]
    body = source[start:end]
    # The fork's C parser cannot represent register asm declarators or
    # statement expressions, and cannot parse attributed definitions. Do not
    # silently discard constraints: opaque the offending construct as a
    # b64literal pragma (restored verbatim at render time) or reject it.
    masked = strip_noncode(body)
    if re.search(r"\(\s*\{", masked):
        raise ValueError("unsupported target: GNU statement expression")
    pins = []
    for lo, hi in reversed(list(extension_spans(body))):
        if "attribute" in body[lo:hi] or lo < opening - start:
            raise ValueError("unsupported target: attributed/asm function declarator")
        previous = masked[:lo].rstrip()
        if previous and previous[-1] not in ";{}":
            # Pinned register declarator: keep the whole declaration verbatim
            # so the constraint survives every mutation, and publish a
            # type-only declaration to the randomizer's pretend context.
            name = re.search(r"([A-Za-z_]\w*)\s*$", previous)
            reg = re.match(r'\s*__asm__\s*\(\s*"r\d+"\s*\)', body[lo:hi])
            if not name or not reg:
                raise ValueError("unsupported target: asm declarator (e.g. pinned register)")
            decl_start, decl_end = declaration_bounds(masked, lo)
            pins.append(body[decl_start:name.end()] + ";")
            encoded = base64.b64encode(body[decl_start:decl_end].encode()).decode()
            body = (body[:decl_start]
                    + f"#pragma _permuter b64literal {encoded}\n"
                    + body[decl_end:])
            continue
        tail = re.match(r"\s*;", body[hi:])
        if not tail:
            raise ValueError("unsupported target: non-statement asm")
        hi += tail.end()
        encoded = base64.b64encode(body[lo:hi].encode()).decode()
        body = body[:lo] + f"\n#pragma _permuter b64literal {encoded}\n" + body[hi:]
    context = source
    for _, _, lo, hi in reversed(ranges):
        context = context[:lo] + ";" + context[hi:]
    context = plain_context(context)
    if pins:
        # After the preamble: the pin types may use its typedefs.
        context = context + "".join(pin + "\n" for pin in reversed(pins))
    # PERM_PRETEND supplies types to the randomizer, but never to the compiler.
    # The real declarations remain in before.c/after.c, including GNU attributes.
    base = f"PERM_PRETEND(\n{context}\n)\n{body}\n"
    return base, source[:start], source[end:]


def mappings(obj: Path, section: str, start: int, size: int):
    with obj.open("rb") as stream:
        elf = ELFFile(stream)
        index = elf.get_section_index(section)
        symbols = elf.get_section_by_name(".symtab")
        rows = sorted((s["st_value"], s.name[:2]) for s in symbols.iter_symbols()
                      if s["st_shndx"] == index and s.name[:2] in ("$t", "$a", "$d"))
    prior = [kind for offset, kind in rows if offset <= start]
    if not prior:
        raise ValueError(f"missing ARM mapping symbol at {section}+{start:#x}")
    return [(0, prior[-1])] + [(offset - start, kind) for offset, kind in rows
                               if start < offset < start + size]


def diff_object(blob: bytes, vma: int, modes: list, output: Path):
    """One resolved ROM-addressed section, with code/data mapping preserved."""
    with tempfile.TemporaryDirectory(prefix="permuter-object-") as tmp:
        binary = Path(tmp) / "body.bin"
        binary.write_bytes(blob)
        command = ["arm-none-eabi-objcopy", "-I", "binary", "-O", "elf32-littlearm",
                   "-B", "arm", "--rename-section", ".data=.text,alloc,load,readonly,code,contents",
                   "--change-section-vma", f".data={vma:#x}", "--strip-all"]
        for i, (offset, kind) in enumerate(modes):
            command += ["--add-symbol", f"{kind}.{i}=.text:{offset},local"]
        run([*command, binary, output], capture_output=True)


def resolved_body(obj: Path, meta: dict):
    section = ".text." + meta["body_name"]
    sections, relocs = probe.fast_parse_elf(obj)
    if sections is None or section not in sections:
        raise ValueError(f"missing candidate section {section}")
    # Include absolute definitions emitted by function-local asm.
    symbols = dict(meta["symbols"])
    with obj.open("rb") as stream:
        elf = ELFFile(stream)
        for sym in elf.get_section_by_name(".symtab").iter_symbols():
            if sym["st_shndx"] == "SHN_ABS":
                symbols[sym.name] = sym["st_value"]
    resolver = probe.SymbolResolver(symbols, meta["aliases"], meta["hints"])
    if any(sym.startswith(".text") and sym != section
           for _, _, sym, _ in relocs.get(section, [])):
        raise ValueError("cross-section text relocation needs an explicit ROM symbol")
    blob, calls, pools = probe.link_function(sections[section], meta["vma"],
                                           relocs.get(section, []), section, resolver)
    if any(not row.get("resolved", False) for row in calls + pools):
        raise ValueError("unresolved candidate relocation; inspect the full-TU probe")
    return blob, mappings(obj, section, 0, len(blob))


def compile_candidate(work: Path, source: Path, output: Path):
    meta = json.loads((work / "metadata.json").read_text())
    if hashlib.sha256(probe.AGBCC.read_bytes()).hexdigest() != meta["compiler_sha256"]:
        raise ValueError("matching compiler changed; prepare a fresh workspace")
    with tempfile.TemporaryDirectory(prefix="compile-", dir=work / "scratch") as tmp:
        directory = Path(tmp)
        tu, asm, obj = [directory / name for name in ("unit.i", "unit.s", "unit.o")]
        tu.write_text((work / "before.c").read_text() + source.read_text()
                      + (work / "after.c").read_text())
        run([probe.AGBCC, *meta["flags"], tu, "-o", asm], capture_output=True)
        run(["arm-none-eabi-as", "-mcpu=arm7tdmi", asm, "-o", obj], capture_output=True)
        blob, modes = resolved_body(obj, meta)
        # Assembly and compiler mapping symbols can classify identical padding
        # differently (e.g. movs r0,r0 versus .short 0). Identical bytes must
        # not acquire a nonzero score solely from that metadata difference.
        if hashlib.sha256(blob).hexdigest() == meta["target_sha256"]:
            modes = meta["target_modes"]
        diff_object(blob, meta["vma"], modes, output)
    return blob


def fork_modules():
    sys.path.insert(0, str(PERMUTER_DIR))
    try:
        from src.candidate import Candidate
        from src.perm.parse import perm_parse
        from src.perm.perm import EvalState
        from src.scorer import Scorer
        from src.main import get_default_randomization_weights
    except ImportError as exc:
        raise ValueError("permuter dependencies missing/incompatible; run "
                         "python3 -m pip install -r tools/requirements-permuter.txt "
                         "in your Python environment") from exc
    return Candidate, perm_parse, EvalState, Scorer, get_default_randomization_weights


def prepare(function: str, work: Path):
    if work.exists() and any(work.iterdir()):
        raise ValueError(f"refusing to overwrite nonempty workspace: {work}")
    if not probe.AGBCC.is_file():
        raise ValueError("old_agbcc missing; run make toolchain")
    run([ROOT / "tools/build_permuter.sh"], cwd=ROOT)
    Candidate, perm_parse, EvalState, Scorer, weights = fork_modules()
    run(["make", "build-code/code.o"], cwd=ROOT)
    work.mkdir(parents=True, exist_ok=True)
    (work / "scratch").mkdir()
    report = work / "probe.json"
    probe_work = work / "probe"
    aliases = probe.alias_targets()
    probe_name = next((alias for alias, target in aliases.items()
                       if target == function and probe.parse_vma_name(alias)), function)
    run([sys.executable, ROOT / "tools/corpus_match_probe.py", "--function", probe_name,
         "--c89", "--require-all", "--work-dir", probe_work, "--json", report], cwd=ROOT)
    records = json.loads(report.read_text())["results"]
    if len(records) != 1:
        raise ValueError(f"expected one probe result, got {len(records)}")
    record = records[0]
    name = record.get("alias_of") or record["name"]
    source = ROOT / record["source"]
    preprocessed = run(["clang", "-E", "-P", "-x", "c", "-nostdinc", "-undef",
                        probe_work / (source.stem + ".i")], capture_output=True, text=True).stdout
    base, before, after = prepare_source(preprocessed, name)
    (work / "base.c").write_text(base)
    (work / "before.c").write_text(before)
    (work / "after.c").write_text(after)
    vma, size = int(record["vma"], 16), record["rom_bytes"]
    meta = {"function": function, "body_name": name, "source": record["source"],
            "vma": vma, "rom_bytes": size, "flags": FLAGS,
            "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
            "preprocessed_sha256": hashlib.sha256(preprocessed.encode()).hexdigest(),
            "compiler_sha256": hashlib.sha256(probe.AGBCC.read_bytes()).hexdigest(),
            "rom_sha256": hashlib.sha256(probe.ROM.read_bytes()).hexdigest(),
            "permuter_revision": run(["git", "-C", PERMUTER_DIR, "rev-parse", "HEAD"],
                                     capture_output=True, text=True).stdout.strip(),
            "symbols": probe.load_code_symbols(), "aliases": probe.alias_targets(),
            "hints": probe.scan_decl_hints()}
    offset = vma - probe.ROM_BASE
    reference = probe.ROM.read_bytes()[offset:offset + size]
    target_modes = mappings(ROOT / "build-code/code.o", ".text", offset, size)
    if target_modes[0][1] != "$t":
        raise ValueError("only Thumb functions are supported by the matching compiler")
    meta.update(target_sha256=hashlib.sha256(reference).hexdigest(), target_modes=target_modes)
    (work / "metadata.json").write_text(json.dumps(meta, indent=2) + "\n")
    diff_object(reference, vma, target_modes, work / "target.o")
    (work / "settings.toml").write_text(f'func_name = "{name}"\ncompiler_type = "gcc"\n')
    # Exercise the actual fork's parser/generator before claiming a usable seed.
    state = EvalState()
    try:
        expanded = perm_parse(base).evaluate(0, state)
        candidate = Candidate.from_source(expanded, state, name, weights("gcc"), 0)
    except Exception as exc:
        raise ValueError("fork cannot parse this target/context: "
                         + getattr(exc, "message", str(exc))) from exc
    seed = work / "seed.c"
    seed.write_text(candidate.get_source())
    original_blob, _ = resolved_body(probe_work / (source.stem + ".o"), meta)
    seed_blob = compile_candidate(work, seed, work / "seed.o")
    if seed_blob != original_blob:
        raise ValueError("permuter source round-trip changed the baseline bytes; scratch rejected")
    score, _ = Scorer(str(work / "target.o"), stack_differences=True,
                      algorithm="difflib", debug_mode=False).score(str(work / "seed.o"))
    (work / "baseline.json").write_text(json.dumps({
        "probe": record, "seed_preserves_baseline": True, "permuter_score": score,
        "seed_rom_exact": seed_blob == reference,
        "note": "Advisory scratch only; reintegrate and run matching-ready before promotion."
    }, indent=2) + "\n")
    # Publish the runnable entrypoint only after the round-trip check passes.
    wrapper = work / "compile.sh"
    wrapper.write_text("#!/bin/sh\nset -eu\nexec " + shlex.join([
        sys.executable, str(Path(__file__).resolve()), "--compile", str(work)])
        + ' "$1" "$3"\n')
    wrapper.chmod(0o755)
    print(f"Prepared {work}: baseline preserved; permuter score {score} (advisory).", flush=True)


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv and argv[0] == "--compile":
        if len(argv) != 4:
            raise ValueError("internal usage: --compile WORK SOURCE OUTPUT")
        compile_candidate(*map(Path, argv[1:]))
        return 0
    split = argv.index("--") if "--" in argv else len(argv)
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("function")
    parser.add_argument("--work-dir", type=Path)
    parser.add_argument("--prepare-only", action="store_true")
    args = parser.parse_args(argv[:split])
    if not re.fullmatch(r"[A-Za-z_]\w*", args.function):
        parser.error("function must be a C identifier")
    work = (args.work_dir or ROOT / "build/permuter" / args.function).resolve()
    prepare(args.function, work)
    command = [sys.executable, str(PERMUTER_DIR / "permuter.py"), str(work),
               "--stack-diffs", *argv[split + 1:]]
    print("Run/resume from " + str(work) + ": " + shlex.join(command), flush=True)
    if args.prepare_only:
        return 0
    return subprocess.run(command, cwd=work).returncode


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(f"run_permuter: {exc}", file=sys.stderr)
        if isinstance(exc, subprocess.CalledProcessError) and exc.stderr:
            print(exc.stderr.decode() if isinstance(exc.stderr, bytes) else exc.stderr, file=sys.stderr)
        raise SystemExit(1)
