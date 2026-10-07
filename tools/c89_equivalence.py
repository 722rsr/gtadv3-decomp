#!/usr/bin/env python3
"""Compare modern ARM compiler output before and after the C89 source rewrite.

This is an independent negative control for the generated-copy transform: both
sources compile with the existing modern C compiler, and all loadable sections
and relocations must agree. It catches accidental changes to the code or data
the transform emits. Equal objects are evidence, not a proof of all C semantics.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path

from elftools.elf.constants import SH_FLAGS
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

ROOT = Path(__file__).resolve().parents[1]
GENERATED = ROOT / "build/era-corpus/c89/generated"
WORK = ROOT / "build/era-corpus/c89/equivalence"
EQUIV_CACHE = ROOT / "build/era-corpus/c89/equivalence_cache"


def compile_arm(source: Path, output: Path) -> tuple[bool, str]:
    command = [
        "arm-none-eabi-gcc", "-mthumb", "-mthumb-interwork", "-mcpu=arm7tdmi",
        "-O2", "-ffreestanding", "-fno-builtin", "-ffunction-sections",
        "-I", str(ROOT / "include"), "-I", str(ROOT / "asm"),
        "-c", str(source), "-o", str(output),
    ]
    proc = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    return proc.returncode == 0, proc.stderr.strip()


def fingerprint(path: Path) -> tuple[dict, dict]:
    """Return loadable section bytes and symbolic relocations from an ELF .o."""
    with path.open("rb") as stream:
        elf = ELFFile(stream)
        sections = {}
        relocations = {}
        for section in elf.iter_sections():
            if section["sh_flags"] & SH_FLAGS.SHF_ALLOC:
                sections[section.name] = (
                    section["sh_size"] if section["sh_type"] == "SHT_NOBITS" else section.data()
                )
            if not isinstance(section, RelocationSection):
                continue
            target = elf.get_section(section["sh_info"]).name
            symtab = elf.get_section(section["sh_link"])
            rows = []
            for reloc in section.iter_relocations():
                sym = symtab.get_symbol(reloc["r_info_sym"])
                name = sym.name
                if not name and isinstance(sym["st_shndx"], int):
                    name = elf.get_section(sym["st_shndx"]).name
                rows.append((reloc["r_offset"], reloc["r_info_type"], name,
                             reloc["r_addend"] if reloc.is_RELA() else None))
            relocations[target] = rows
        return sections, relocations


def get_header_tag() -> str:
    h = hashlib.sha256()
    for p in sorted((ROOT / "include").rglob("*.[ch]")):
        h.update(str(p.stat().st_mtime_ns).encode())
    for p in sorted((ROOT / "asm").rglob("*.inc")):
        h.update(str(p.stat().st_mtime_ns).encode())
    gcc_path = shutil.which("arm-none-eabi-gcc")
    if gcc_path:
        h.update(str(Path(gcc_path).stat().st_mtime_ns).encode())
    return h.hexdigest()[:16]


def check_source(source: Path, generated_dir: Path, header_tag: str) -> dict:
    generated = generated_dir / source.name
    record = {"source": str(source.relative_to(ROOT)), "status": ""}
    if not generated.is_file():
        record["status"] = "MISSING_GENERATED"
        return record

    cache_file = None
    try:
        h = hashlib.sha256()
        h.update(source.read_bytes())
        h.update(b"\0")
        h.update(generated.read_bytes())
        h.update(b"\0")
        h.update(header_tag.encode())
        cache_key = h.hexdigest()[:32]
        cache_file = EQUIV_CACHE / f"{cache_key}.json"
        if cache_file.exists():
            return json.loads(cache_file.read_text(encoding="utf-8"))
    except Exception:
        pass

    orig_obj = WORK / f"{source.stem}.original.o"
    c89_obj = WORK / f"{source.stem}.c89.o"
    ok_orig, diag_orig = compile_arm(source, orig_obj)
    ok_c89, diag_c89 = compile_arm(generated, c89_obj)
    if not ok_orig or not ok_c89:
        record["status"] = "COMPILE_FAIL"
        record["diagnostic"] = (diag_orig if not ok_orig else diag_c89).splitlines()[:2]
    else:
        orig_sections, orig_relocs = fingerprint(orig_obj)
        c89_sections, c89_relocs = fingerprint(c89_obj)
        different_sections = sorted(set(orig_sections) ^ set(c89_sections) |
                                    {name for name in orig_sections.keys() & c89_sections.keys()
                                     if orig_sections[name] != c89_sections[name]})
        different_relocs = sorted(set(orig_relocs) ^ set(c89_relocs) |
                                  {name for name in orig_relocs.keys() & c89_relocs.keys()
                                   if orig_relocs[name] != c89_relocs[name]})
        record["status"] = "EXACT" if not different_sections and not different_relocs else "MISMATCH"
        if different_sections:
            record["different_sections"] = different_sections
        if different_relocs:
            record["different_relocations"] = different_relocs

    if cache_file and record["status"] == "EXACT":
        try:
            EQUIV_CACHE.mkdir(parents=True, exist_ok=True)
            cache_file.write_text(json.dumps(record), encoding="utf-8")
        except Exception:
            pass
    return record


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--generated", type=Path, default=GENERATED)
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--json", type=Path)
    args = ap.parse_args()
    sources = sorted((ROOT / "src").glob("*.c"))
    if args.limit:
        sources = sources[:args.limit]
    WORK.mkdir(parents=True, exist_ok=True)
    header_tag = get_header_tag()
    workers = min(os.cpu_count() or 4, len(sources), 16)
    if len(sources) <= 1:
        records = [check_source(s, args.generated, header_tag) for s in sources]
    else:
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            records = list(pool.map(lambda s: check_source(s, args.generated, header_tag), sources))
    failures = [r for r in records if r["status"] != "EXACT"]
    print(f"C89 modern ARM output equivalence: {len(records) - len(failures)}/{len(records)} exact")
    for record in failures:
        print(f"  {record['source']}: {record['status']} "
              f"{record.get('different_sections', record.get('diagnostic', ''))}")
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps({"total": len(records), "exact": len(records) - len(failures),
                                         "results": records}, indent=2, sort_keys=True) + "\n")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
