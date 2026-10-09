#!/usr/bin/env python3
"""Generate objdiff units and rebuild one ROM-addressed function pair on demand.

Viewer output is advisory, separate from the independent-link/progress gates.
No permuter installation is needed; only its local ELF helpers are shared.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

from elftools.elf.elffile import ELFFile

import corpus_match_probe as probe
from run_permuter import diff_object, mappings, resolved_body, run

ROOT = probe.ROOT
CONFIG = ROOT / "objdiff.json"


def configuration() -> dict:
    spans = probe.rom_functions()
    best = {}
    # Match the corpus probe's ownership ranking, including its stable tie order.
    for source, name, vma, body, weak in probe.src_functions():
        if vma not in spans:
            continue
        rank = (not weak, body)
        if vma not in best or rank > best[vma][0]:
            best[vma] = rank, source, name
    units = []
    for vma, (_, source, name) in sorted(best.items()):
        directory = f"build/objdiff/{vma:08x}"
        units.append({"name": f"{source.stem}/{name}",
                      "target_path": f"{directory}/target.o",
                      "base_path": f"{directory}/candidate.o",
                      "metadata": {"source_path": source.relative_to(ROOT).as_posix()}})
    return {"$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
            "min_version": "3.8.2", "custom_make": "python3",
            "custom_args": ["tools/objdiff_build.py"],
            "build_target": False, "build_base": True,
            "options": {"arm_arch_version": "v4t"},
            "watch_patterns": ["src/**/*.c", "include/**/*.h", "asm/**/*.s", "asm/**/*.inc",
                               "tools/*.py", "tools/matching_slice_functions.json", "Makefile",
                               "baserom.gba", "baserom.sha256", "build/toolchains/agbcc/old_agbcc"],
            "ignore_patterns": ["build/objdiff/**", "build/era-corpus/**", "build-code/**"],
            "units": units}


def named_object(blob: bytes, vma: int, modes: list, name: str, output: Path):
    """Give objdiff a sized Thumb function, rather than an anonymous byte section."""
    diff_object(blob, vma, modes, output)
    # objdiff 3.8.2's ARM decoder recognizes exact $t/$a/$d names only,
    # unlike GNU objdump, which also accepts mapping-symbol suffixes.
    run(["arm-none-eabi-objcopy", "--add-symbol", f"{name}=.text:1,global,function", output],
        capture_output=True)
    # These bytes are already resolved at their ROM VMA. Use executable ELF
    # address semantics (absolute symbols), not ET_REL with a nonzero sh_addr.
    # objcopy cannot set st_size; size includes the pool and alignment tail.
    with output.open("r+b") as stream:
        elf = ELFFile(stream)
        symtab = elf.get_section_by_name(".symtab")
        symbols = list(symtab.iter_symbols())
        strings_offset = elf.get_section(symtab["sh_link"])["sh_offset"]
        section_index = elf.get_section_index(".text")
        for index, sym in enumerate(symbols):
            if sym["st_shndx"] != section_index:
                continue
            offset = symtab["sh_offset"] + index * symtab["sh_entsize"]
            stream.seek(offset + 4)
            stream.write(struct.pack("<II", sym["st_value"] + vma,
                                     len(blob) if sym.name == name else sym["st_size"]))
            if re.fullmatch(r"\$[atd]\.\d+", sym.name):
                # Repeated $t/$d names are valid ELF, but objcopy's rename
                # interface rejects multiple names with the same destination.
                stream.seek(strings_offset + sym["st_name"] + 2)
                stream.write(b"\0")
        stream.seek(16)
        stream.write(struct.pack("<H", 2))  # ET_EXEC: diagnostic image, not a runnable ROM


def configured_unit(path: str) -> dict:
    # objdiff may pass either a relative path or an absolute project path.
    requested = Path(path).resolve()
    config = json.loads(CONFIG.read_text())
    for unit in config["units"]:
        if requested in ((ROOT / unit["base_path"]).resolve(),
                         (ROOT / unit["target_path"]).resolve()):
            return unit
    raise ValueError(f"object is not a configured unit: {path}; run make objdiff-config")


def build(unit: dict):
    target, candidate = (ROOT / unit[key] for key in ("target_path", "base_path"))
    directory = target.parent
    directory.mkdir(parents=True, exist_ok=True)
    # Fail closed: a broken rebuild must not leave yesterday's comparison visible.
    target.unlink(missing_ok=True)
    candidate.unlink(missing_ok=True)
    (directory / "comparison.json").unlink(missing_ok=True)
    function = unit["name"].rsplit("/", 1)[1]
    vma = int(directory.name, 16)
    rom = probe.ROM.read_bytes()
    expected = next(line.split()[0] for line in (ROOT / "baserom.sha256").read_text().splitlines()
                    if re.fullmatch(r"[0-9a-f]{64}\s+\*?baserom\.gba", line.strip()))
    if hashlib.sha256(rom).hexdigest() != expected:
        raise ValueError("baserom.gba does not match baserom.sha256")
    run(["make", "build-code/code.o"], cwd=ROOT)
    with tempfile.TemporaryDirectory(prefix="work-", dir=directory) as tmp:
        work = Path(tmp)
        report = work / "probe.json"
        run([sys.executable, ROOT / "tools/corpus_match_probe.py", "--function", function,
             "--c89", "--require-all", "--work-dir", work / "probe", "--json", report], cwd=ROOT)
        records = json.loads(report.read_text())["results"]
        if len(records) != 1 or int(records[0]["vma"], 16) != vma:
            raise ValueError("function inventory changed; run make objdiff-config")
        record = records[0]
        if record["source"] != unit["metadata"]["source_path"]:
            raise ValueError("function owner changed; run make objdiff-config")
        body_name = record.get("alias_of") or record["name"]
        meta = {"body_name": body_name, "vma": vma,
                "symbols": probe.load_code_symbols(), "aliases": probe.alias_targets(),
                "hints": probe.scan_decl_hints()}
        obj = work / "probe" / (Path(record["source"]).stem + ".o")
        blob, candidate_modes = resolved_body(obj, meta)
        offset, size = vma - probe.ROM_BASE, record["rom_bytes"]
        reference = rom[offset:offset + size]
        target_modes = mappings(ROOT / "build-code/code.o", ".text", offset, size)
        if target_modes[0][1] != "$t":
            raise ValueError("only Thumb functions are supported by old_agbcc")
        if blob == reference:
            candidate_modes = target_modes  # identical padding can have different mapping labels
        named_object(reference, vma, target_modes, body_name, work / "target.o")
        named_object(blob, vma, candidate_modes, body_name, work / "candidate.o")
        (work / "target.o").replace(target)
        (work / "candidate.o").replace(candidate)
        (directory / "comparison.json").write_text(json.dumps({
            "probe": record, "symbol": body_name, "resolved_bytes_exact": blob == reference,
            "rom_sha256": expected,
            "note": "Viewer comparison only; use matching-ready for independent-link acceptance."
        }, indent=2) + "\n")
    print(f"objdiff: {unit['name']} -> {body_name}; exact resolved bytes: {blob == reference}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("object", nargs="?", help="configured target/base object path")
    parser.add_argument("--configure", action="store_true", help="regenerate objdiff.json from source inventory")
    parser.add_argument("--check-config", action="store_true", help="check inventory/config agreement without writing config")
    args = parser.parse_args()
    if args.configure or args.check_config:
        content = json.dumps(configuration(), indent=2) + "\n"
        if args.check_config:
            if CONFIG.read_text() != content:
                raise ValueError("objdiff.json is stale; run make objdiff-config")
        else:
            CONFIG.write_text(content)
        print(f"objdiff: {len(json.loads(content)['units'])} function units configured")
    elif args.object:
        build(configured_unit(args.object))
    else:
        parser.error("provide an object path or --configure")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(f"objdiff_build: {exc}", file=sys.stderr)
        if isinstance(exc, subprocess.CalledProcessError) and exc.stderr:
            print(exc.stderr.decode() if isinstance(exc.stderr, bytes) else exc.stderr, file=sys.stderr)
        raise SystemExit(1)
