#!/usr/bin/env python3
"""Build a complete, local work packet for one remaining assembly body.

A sprint target is more than a VMA and a size: the lift needs the current
source form, the ROM listing, the ABI the bytes imply, every caller and callee,
the nearest already-decompiled sibling, any findings note that already paid for
a failed attempt, and a seed to edit. Assembling that by hand costs a probe
cycle per function and is exactly the context that gets thrown away between
sessions.

This tool collects it into one directory under ignored `build/packets/`:

    packet.md      human/agent-readable brief, in the order a lift uses it
    meta.json      the same facts as structured data
    body.c         current source body (naked asm or partial C), when present
    input.s        normalized agbcc-shaped assembly
    rom.txt        objdump listing of the manifest span
    seed.c         asmlift seed, when asmlift is installed and the body lifts
    score.json     the seed's one-compile advisory score

Commands:

    python3 tools/lift_packet.py 0x0801EFAC
    python3 tools/lift_packet.py _0801EFAC --no-seed
    python3 tools/lift_packet.py --queue --limit 10 --seed
    python3 tools/lift_packet.py --self-test

The packet is discovery only. It does not edit `src/`, select a body, or grant
promotion authority. A seed is a draft; the owning TU and the independent link
remain the acceptance gates.
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

ROOT = Path(__file__).resolve().parents[1]
ROM = ROOT / "baserom.gba"
DEFAULT_OUT = ROOT / "build/packets"
FINDINGS = ROOT / "docs/findings"


# ---------------------------------------------------------------------------
# Small shared helpers
# ---------------------------------------------------------------------------

def _json(path: Path, default):
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return default


_ROM_BYTES: bytes | None = None


def _rom() -> bytes:
    global _ROM_BYTES
    if _ROM_BYTES is None:
        _ROM_BYTES = ROM.read_bytes()
    return _ROM_BYTES


def _short_vma(vma: int) -> str:
    return f"{vma & 0xFFFFFF:06x}"


def _fmt_vma(vma: int) -> str:
    return f"0x{vma:08x}"


def _relative(path: Path) -> str:
    """Repo-relative when possible; absolute `--out` trees are legal too."""
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def _disasm_span(vma: int, size: int, limit: int = 12) -> list[str]:
    """Rows of the first `limit` instructions, without relying on a build."""
    sys.path.insert(0, str(ROOT / "tools"))
    import lift_loop

    blob = _rom()[vma - 0x08000000: vma - 0x08000000 + size]
    rows = lift_loop.disasm(blob, vma)
    return [f"{addr:08x}  {text}" for addr, text in rows[:limit]]


def _xrefs(vma: int) -> dict:
    """Static `bl` callers and literal-word references to a VMA.

    The ROM-wide `bl` scan is the same decoder used by tools/xref.py; literal
    references are exact 4-byte finds, so a data-region coincidence is listed
    as a reference rather than silently promoted to a call.
    """
    import xref

    rom = _rom()
    offset = vma - 0x08000000
    # thumb_bl_targets yields file offsets for both the site and the target.
    calls = [site for site, target in xref.thumb_bl_targets(rom)
             if target == offset]
    words = [struct.pack("<I", value) for value in (vma, vma | 1)]
    literals = set()
    for word in words:
        start = 0
        while True:
            at = rom.find(word, start)
            if at < 0:
                break
            literals.add(at)
            start = at + 1
    return {"calls": calls, "literals": sorted(literals)}


def _spans() -> list[tuple[int, int, str, str]]:
    """(start, end, name, file) for every selected body, for owner lookup."""
    quality = _json(ROOT / "docs/data/source_quality.json", {}).get("functions", {})
    rows = []
    for key, rec in quality.items():
        try:
            vma = int(key, 16)
        except ValueError:
            continue
        size = int(rec.get("size") or 0)
        if size:
            rows.append((vma, vma + size, rec.get("name", ""), rec.get("file", "")))
    rows.sort()
    return rows


def _owner(site: int, spans: list[tuple[int, int, str, str]]) -> tuple[str, str]:
    for start, end, name, file in spans:
        if start <= site < end:
            return name, file
    return "", ""


def _findings(vma: int, name: str) -> list[str]:
    """Findings files that mention this body by address or name."""
    needles = {f"0x{vma:08x}", f"0x{vma:08X}", name}
    out = []
    for path in sorted(FINDINGS.glob("*.md")):
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        if any(needle in text for needle in needles if needle):
            out.append(str(path.relative_to(ROOT)))
    return out


def _arity(vma: int) -> dict:
    """ROM-derived arity and the C declaration's shape, when each exists."""
    import arity_audit

    table = arity_audit.rom_arity_table()
    row = table.get(_short_vma(vma))
    rom_arity = None
    prologue_disp = None
    if row is not None:
        rom_arity, prologue_disp = row[0], row[1]
    return {"rom": rom_arity, "prologue_disp": prologue_disp}


def _c_signature(file: str, name: str) -> str:
    if not file:
        return ""
    import lift_scout

    path = ROOT / file
    if not path.exists():
        return ""
    body = lift_scout.source_body(path, name)
    return lift_scout.signature_of(body) if body else ""


def _parameter_count(signature: str) -> int | None:
    if not signature:
        return None
    import proto_audit

    start = signature.find("(")
    end = signature.rfind(")")
    if start < 0 or end <= start:
        return None
    return len(proto_audit.split_params(signature[start + 1:end]))


# ---------------------------------------------------------------------------
# Similar matched functions
# ---------------------------------------------------------------------------

def _similar(vma: int, size: int, name: str, file: str, count: int = 5) -> list[dict]:
    """Nearest already-decompiled siblings from the family index.

    The index covers bodies the corpus report saw; a naked span has no report
    row, so its fingerprint is built from the ROM here and scored against the
    indexed records directly. Building the index when it is stale is cheap
    (under a second) and keeps the packet self-contained.
    """
    try:
        import match_families as mf

        report = ROOT / "build/era-corpus/ready-report.json"
        index_path = ROOT / "build/matching-assist/families.json"
        try:
            index = mf.load_index(index_path)
        except (OSError, ValueError, KeyError):
            index = mf.build_index(report, index_path)
        target = mf.target_from_rom(vma, size)
        target.update({"name": name, "source": file, "alias_of": None})
        rows = [{
            "vma": hit.get("vma"),
            "name": hit.get("name"),
            "source": hit.get("source"),
            "similarity": hit.get("similarity"),
            "method": "sequence",
            "same_family": bool(hit.get("same_family")),
            "bytes": hit.get("bytes"),
            "class": hit.get("class") or hit.get("twin_class"),
        } for hit in mf.nearest(index, target, count=count)]
        if rows:
            return rows
        # A naked 1.4 KB body often has no sibling inside the sequence-matcher
        # cutoff even when it shares 80% of its instruction vocabulary with a
        # lifted body. The length-tolerant cosine fallback lives in
        # match_families so the CLI and the packet rank siblings identically.
        return [{
            "vma": hit.get("vma"),
            "name": hit.get("name"),
            "source": hit.get("source"),
            "similarity": hit.get("similarity"),
            "method": hit.get("method", "cosine"),
            "same_family": bool(hit.get("same_family")),
            "bytes": hit.get("bytes"),
            "class": hit.get("class") or hit.get("twin_class"),
        } for hit in mf.cosine_siblings(index, target, count=count)]
    except Exception as exc:  # noqa: BLE001 - family retrieval is advisory
        return [{"error": f"{type(exc).__name__}: {exc}"[:200]}]


# ---------------------------------------------------------------------------
# Packet construction
# ---------------------------------------------------------------------------

def _call_graph(vma: int, size: int, input_text: str) -> tuple[list[dict], list[dict]]:
    """Callees from the normalized text, callers from the ROM."""
    import arity_audit

    table = arity_audit.rom_arity_table()
    callees: dict[str, dict] = {}
    for match in re.finditer(r"\b(?:bl|blx)\s+([A-Za-z_][\w]*|0x[0-9A-Fa-f]+)",
                             input_text):
        target = match.group(1)
        target_vma = None
        if target.lower().startswith("0x"):
            target_vma = int(target, 16)
        else:
            target_vma = _parse_symbol(target)
        key = target if target_vma is None else _fmt_vma(target_vma)
        row = callees.setdefault(key, {"symbol": target, "vma": _fmt_vma(target_vma)
                                       if target_vma else "", "calls": 0,
                                       "arity": None, "signature": ""})
        row["calls"] += 1
        if target_vma:
            table_row = table.get(_short_vma(target_vma))
            if table_row and table_row[0] is not None:
                row["arity"] = table_row[0]
    spans = _spans()
    xr = _xrefs(vma)
    callers = []
    for site in xr["calls"]:
        owner_name, owner_file = _owner(site + 0x08000000, spans)
        callers.append({"site": _fmt_vma(site + 0x08000000),
                        "owner": owner_name, "file": owner_file})
    return list(callees.values()), callers


def _parse_symbol(name: str) -> int | None:
    match = re.fullmatch(r"(?:sub_)?_?(?:0*)([0-9A-Fa-f]{6,8})", name)
    if not match:
        return None
    value = int(match.group(1), 16)
    return value | 0x08000000 if value < 0x08000000 else value


def build_packet(needle: str, out_root: Path, *, seed: bool = True,
                 force: bool = False, census_rows: dict | None = None) -> dict:
    import asmlift_seed

    record = asmlift_seed.resolve(needle)
    vma, size = record["vma"], record["size"]
    work = out_root / f"{vma:08x}"
    if work.exists() and not force and (work / "meta.json").exists():
        cached = _json(work / "meta.json", {})
        if cached.get("tool") == "lift_packet" and cached.get("vma") == _fmt_vma(vma):
            return cached
    work.mkdir(parents=True, exist_ok=True)

    # Current source body, notes, wall class.
    import lift_scout
    import wall_census

    body = ""
    if record["file"]:
        path = ROOT / record["file"]
        if path.exists():
            body = lift_scout.source_body(path, record["name"])
    # A queue pays the ~0.8 s live census once and shares it; a single packet
    # pays it once for itself. Re-deriving it per row made a ten-row queue
    # spend most of its wall time re-reading the same source comments.
    census = census_rows or {row["vma"]: row for row in wall_census.census(snapshot=False)}
    wall_row = census.get(vma, {})
    wall = wall_row.get("wall", "unknown")
    note = wall_row.get("note", "")

    # The normalized agbcc-shaped text is also the input a seed uses.
    entries = {int(key, 16) for key in asmlift_seed.load_quality()}
    try:
        raw = asmlift_seed.extracted_asm(vma, size, work / "asm")
        input_text, norm = asmlift_seed.normalize_asm(
            raw, vma=vma, size=size, name=record["name"],
            aliases=record["aliases"], entries=entries)
    except (OSError, SystemExit, subprocess.SubprocessError) as exc:
        input_text, norm = "", {"error": str(exc)[:300]}
    if input_text:
        (work / "input.s").write_text(input_text, encoding="utf-8")
        (work / "rom.txt").write_text(
            "\n".join(_disasm_span(vma, size, limit=max(12, size // 2))) + "\n",
            encoding="utf-8")
    if body:
        (work / "body.c").write_text(body + "\n", encoding="utf-8")

    metadata = {
        "tool": "lift_packet",
        "vma": _fmt_vma(vma),
        "size": size,
        "name": record["name"],
        "file": record["file"],
        "class": record["class"],
        "aliases": record["aliases"],
        "body_present": bool(body),
        "wall": wall,
        "wall_note": note,
        "prologue": _disasm_span(vma, min(size, 24), limit=8),
        "arity": _arity(vma),
        "c_signature": _c_signature(record["file"], record["name"]),
        "normalization": norm,
        "callers": [],
        "callees": [],
        "literals": [],
        "similar": [],
        "findings": _findings(vma, record["name"]),
        "seed": {"status": "not-requested"},
        "commands": {},
    }
    if metadata["c_signature"]:
        metadata["arity"]["c_params"] = _parameter_count(metadata["c_signature"])
    if input_text:
        callees, callers = _call_graph(vma, size, input_text)
        metadata["callees"] = callees
        metadata["callers"] = callers
        xr = _xrefs(vma)
        spans = _spans()
        metadata["literals"] = [
            {"site": _fmt_vma(off + 0x08000000),
             "owner": _owner(off + 0x08000000, spans)[0],
             "file": _owner(off + 0x08000000, spans)[1]}
            for off in xr["literals"]]
    metadata["similar"] = _similar(vma, size, record["name"], record["file"])

    if seed:
        executable = asmlift_seed.asmlift_path()
        if executable is None:
            metadata["seed"] = {
                "status": "unavailable",
                "hint": "python3 tools/asmlift_seed.py --install",
            }
        else:
            # Reuse a seed this body already has; otherwise generate one.
            seed_meta = work / "seed.c"
            try:
                generated = asmlift_seed.generate(
                    record, out_root, executable, score=True, force=False)
                seed_file = work / "seed.c"
                metadata["seed"] = {
                    "status": generated.get("status"),
                    "path": (_relative(seed_file)
                             if seed_file.exists() else ""),
                    "score": generated.get("score"),
                    "error": generated.get("error", ""),
                }
            except Exception as exc:  # noqa: BLE001 - seed is optional
                metadata["seed"] = {"status": "error",
                                    "error": f"{type(exc).__name__}: {exc}"[:200]}
            _ = seed_meta

    seed_path = (work / "seed.c")
    splice_command = (f"python3 tools/lift_loop.py {record['file']} "
                      f"{record['name']} --variant {_relative(seed_path)} --diff"
                      if seed_path.exists() else
                      f"# no seed; python3 tools/asmlift_seed.py 0x{vma:08x} --force")
    metadata["commands"] = {
        "rescore_seed": f"python3 tools/asmlift_seed.py 0x{vma:08x} --force",
        "splice_and_score": splice_command,
        "wave_check": "python3 tools/wall_census.py",
        "accept": "make matching-ready && make full-rom-match",
    }
    (work / "meta.json").write_text(json.dumps(metadata, indent=2) + "\n",
                                    encoding="utf-8")
    (work / "packet.md").write_text(render(metadata), encoding="utf-8")
    return metadata


# ---------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------

def _table(rows: list[list[str]], headers: list[str]) -> list[str]:
    if not rows:
        return []
    widths = [len(h) for h in headers]
    for row in rows:
        for i, cell in enumerate(row):
            widths[i] = max(widths[i], len(cell))
    out = ["| " + " | ".join(h.ljust(widths[i]) for i, h in enumerate(headers)) + " |",
           "| " + " | ".join("-" * widths[i] for i in range(len(headers))) + " |"]
    for row in rows:
        out.append("| " + " | ".join(cell.ljust(widths[i])
                                     for i, cell in enumerate(row)) + " |")
    return out


def render(meta: dict) -> str:
    vma = meta["vma"]
    end = int(vma, 16) + meta["size"]
    lines = [
        f"# {meta['name']} ({vma}, {meta['size']:,} B)",
        "",
        f"- Source: `{meta['file']}`",
        f"- Source class: `{meta['class']}`; wall: `{meta['wall']}`",
        f"- ROM span: `{vma}`..`0x{end:08x}`",
    ]
    if meta.get("aliases"):
        lines.append("- Aliases: " + ", ".join(f"`{a}`" for a in meta["aliases"]))
    if meta.get("wall_note"):
        lines += ["", "> " + meta["wall_note"].strip().replace("\n", "\n> ")]
    lines += ["", "## ABI", ""]
    prologue = meta.get("prologue") or []
    if prologue:
        lines += ["First instructions:", "", "```"]
        lines += prologue
        lines += ["```", ""]
    arity = meta.get("arity") or {}
    lines.append(f"- ROM arity: **{arity.get('rom') if arity.get('rom') is not None else 'unknown'}**"
                 f" (prologue displacement `{arity.get('prologue_disp')}`)")
    c_params = arity.get("c_params")
    if c_params is not None:
        lines.append(f"- C declaration parameters: **{c_params}**")
    if meta.get("c_signature"):
        lines += ["", "Current C signature:", "", "```c", meta["c_signature"], "```"]
    if not meta.get("body_present", True) and meta.get("file"):
        lines += ["", "_No body text was found for this name in the owning file; "
                  "inspect the source around the VMA._"]

    lines += ["", "## Callees", ""]
    callees = [c for c in meta.get("callees", []) if c.get("calls")]
    lines += _table(
        [[c.get("symbol", ""), c.get("vma", ""),
          str(c.get("arity") if c.get("arity") is not None else "?"),
          str(c.get("calls", ""))] for c in callees],
        ["symbol", "VMA", "arity", "calls"]) or ["_none found in the normalized body_"]
    lines += ["", "## Callers", ""]
    lines += _table(
        [[c.get("site", ""), c.get("owner", ""), c.get("file", "")]
         for c in meta.get("callers", [])],
        ["site", "owner", "file"]) or ["_none; this body may be reached through a table or a computed branch_"]
    if meta.get("literals"):
        lines += ["", "## Literal-word references", ""]
        lines += _table(
            [[r.get("site", ""), r.get("owner", ""), r.get("file", "")]
             for r in meta["literals"]],
            ["word site", "owner", "file"])

    lines += ["", "## Similar decompiled bodies", ""]
    rows = []
    for hit in meta.get("similar", []):
        if hit.get("error"):
            lines.append(f"_family retrieval failed: {hit['error']}_")
            break
        rows.append([str(hit.get("similarity", "")), hit.get("vma", ""),
                     hit.get("name", ""), hit.get("source", ""),
                     hit.get("method", ""),
                     "same" if hit.get("same_family") else ""])
    else:
        if rows:
            lines += _table(rows, ["similarity", "VMA", "name", "source",
                                   "method", "family"])
        else:
            lines.append("_no near sibling in the indexed C corpus_")

    findings = meta.get("findings") or []
    if findings:
        lines += ["", "## Existing findings", ""]
        lines += [f"- [{f}](../../../{f})" for f in findings]

    lines += ["", "## Seed", ""]
    seed = meta.get("seed") or {}
    status = seed.get("status", "not-requested")
    lines.append(f"- status: **{status}**")
    if seed.get("path"):
        lines.append(f"- file: `{seed['path']}`")
    score = seed.get("score") or {}
    if score:
        detail = (f"- score: `{score.get('kind', '?')}`, "
                  f"`{score.get('aligned', '?')}` aligned instructions")
        if score.get("aligned_percent") is not None:
            detail += f" ({score['aligned_percent']}%)"
        detail += f", `{score.get('candidate_bytes', '?')}` candidate bytes"
        if score.get("bytes") is not None:
            detail += f", `{score.get('bytes')}` matching bytes"
        lines.append(detail)
    if seed.get("error"):
        lines.append(f"- decline: `{seed['error']}`")
    if seed.get("hint"):
        lines.append(f"- {seed['hint']}")

    lines += ["", "## Commands", ""]
    for label, command in (meta.get("commands") or {}).items():
        lines.append(f"- `{label}`: `{command}`")

    lines += [
        "",
        "## Promotion checklist",
        "",
        "1. Re-check ABI, access widths, and call order against the ROM rows.",
        "2. Replace the seed with readable C in the owning translation unit; "
        "keep aliases and required interiors intact.",
        "3. Run the focused probe: "
        f"`python3 tools/corpus_match_probe.py --function {meta['name']} --c89 --require-all --require-exact`.",
        "4. Run `make matching-ready` and then `make full-rom-match`.",
        "5. Refresh `source_quality`, the report, and the README progress block "
        "with `make update-progress`.",
        "",
        "_Discovery packet only: it does not grant promotion authority._",
        "",
    ]
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Queue and self-test
# ---------------------------------------------------------------------------

def queue(out_root: Path, *, wall: str, limit: int, seed: bool,
          force: bool) -> list[dict]:
    import asmlift_seed
    import wall_census

    live = wall_census.census(snapshot=False)
    rows = asmlift_seed.backlog(wall=wall, census_rows=live)
    if limit:
        rows = rows[:limit]
    census_rows = {row["vma"]: row for row in live}
    out = []
    for record in rows:
        try:
            out.append(build_packet(_fmt_vma(record["vma"]), out_root,
                                    seed=seed, force=force,
                                    census_rows=census_rows))
        except Exception as exc:  # noqa: BLE001 - one packet must not end the queue
            out.append({"tool": "lift_packet", "vma": _fmt_vma(record["vma"]),
                        "name": record["name"], "error":
                        f"{type(exc).__name__}: {exc}"[:300]})
    return out


def self_test() -> int:
    checks = []

    def check(name, got, want):
        checks.append((name, got == want, got, want))

    check("short vma", _short_vma(0x0801EFAC), "01efac")
    check("parse symbol", _parse_symbol("sub_0801EFAC"), 0x0801EFAC)
    check("parse literal symbol", _parse_symbol("_0801EFAC"), 0x0801EFAC)
    check("parse friendly", _parse_symbol("Race_Dispatch"), None)
    check("parameter count", _parameter_count("void f(void *a, int b)"), 2)
    check("parameter count void", _parameter_count("void f(void)"), 0)
    check("owner lookup", _owner(0x08001004, [(0x08001000, 0x08001020, "f", "s.c")]),
          ("f", "s.c"))
    check("owner miss", _owner(0x08000000, [(0x08001000, 0x08001020, "f", "s.c")]),
          ("", ""))
    failed = [c for c in checks if not c[1]]
    for name, ok, got, want in checks:
        print(f"  {'PASS' if ok else 'FAIL'} {name}: got {got!r} want {want!r}")
    print(f"lift_packet self-test: {'PASS' if not failed else 'FAIL'} "
          f"({len(checks) - len(failed)}/{len(checks)})")
    return 0 if not failed else 1


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("function", nargs="?", help="VMA or body name")
    ap.add_argument("--queue", action="store_true", help="build packets for a backlog slice")
    ap.add_argument("--wall", default="raw",
                    choices=["raw", "measured", "regalloc", "unreachable",
                             "veneer", "unexplained", "all"])
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--no-seed", action="store_true", help="skip asmlift seed generation")
    ap.add_argument("--seed", dest="seed", action="store_true", default=True,
                    help="generate an asmlift seed when available (default)")
    ap.add_argument("--force", action="store_true")
    ap.add_argument("--json", type=Path)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)
    if args.self_test:
        return self_test()
    out = args.out if args.out.is_absolute() else ROOT / args.out
    if args.queue:
        rows = queue(out, wall=args.wall, limit=args.limit,
                     seed=args.seed and not args.no_seed, force=args.force)
        ok = [r for r in rows if not r.get("error")]
        print(f"lift_packet: {len(ok)}/{len(rows)} packets; "
              f"{sum(r.get('size', 0) for r in ok):,} ROM bytes")
        print(f"packets: {out}")
        if args.json:
            args.json.parent.mkdir(parents=True, exist_ok=True)
            args.json.write_text(json.dumps({"packets": rows}, indent=2) + "\n",
                                 encoding="utf-8")
        return 0
    if not args.function:
        ap.error("provide a function, or use --queue")
    result = build_packet(args.function, out, seed=not args.no_seed,
                          force=args.force)
    print(f"lift_packet: {result['vma']} {result['name']} -> "
          f"{out / result['vma'][2:]} / packet.md")
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
