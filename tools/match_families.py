#!/usr/bin/env python3
"""ROM instruction families and freshly verified agbcc recipe retrieval.

Discovery only: neither fingerprints nor recipes grant promotion authority.
Uses stdlib and the existing local ARM toolchain. Outputs belong under build/.
"""
from __future__ import annotations

import argparse
from bisect import bisect_right
from collections import defaultdict
from difflib import SequenceMatcher
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

import corpus_match_probe as probe

ROOT = probe.ROOT
DEFAULT_INDEX = ROOT / 'build/matching-assist/families.json'
REPORT = ROOT / 'build/era-corpus/ready-report.json'
VERSION = 1


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def inventory_digest():
    """Invalidate advisory spans/ownership when their source inputs change."""
    paths = sorted(p for p in (ROOT / 'asm').rglob('*') if p.suffix in ('.s', '.inc'))
    paths += [ROOT / 'tools' / n for n in ('coverage.py', 'corpus_match_probe.py',
                                         'matching_slice_functions.json', 'match_families.py')]
    h = hashlib.sha256()
    for path in paths:
        h.update(str(path.relative_to(ROOT)).encode() + b'\0')
        h.update(path.read_bytes())
    return h.hexdigest()


def sign(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def assembled_modes(work):
    """Get ARM/Thumb/data mappings from the current assembled include closure."""
    obj = work / 'mode-map.o'
    subprocess.run(['arm-none-eabi-as', '-mcpu=arm7tdmi', '-I' + str(ROOT / 'asm'),
                    str(ROOT / 'asm/code.s'), '-o', str(obj)], check=True, capture_output=True)
    text = subprocess.run(['arm-none-eabi-readelf', '-sW', str(obj)],
                          check=True, capture_output=True, text=True).stdout
    modes = []
    for line in text.splitlines():
        fields = line.split()
        if len(fields) == 8 and fields[6] == '1' and re.fullmatch(r'\$[atd](?:\..*)?', fields[7]):
            modes.append((probe.ROM_BASE + int(fields[1], 16), fields[7][1]))
    if not modes:
        raise ValueError('assembled closure has no instruction-set mapping symbols')
    return sorted(modes)


def mode_at(modes, pc):
    pos = bisect_right(modes, (pc, 'z')) - 1
    return modes[pos][1] if pos >= 0 else None


def flow(blob, start, end):
    """Conservative Thumb-1 reachability; pools are never walked after returns.

    Unknown indirect transfers, out-of-span branches and non-Thumb-1 opcodes
    make a record ineligible for grouping, rather than guessing a boundary.
    """
    pending, seen, edges, pools = [start], {}, [], set()
    failures = []
    def half(pc):
        off = pc - probe.ROM_BASE
        if off < 0 or off + 2 > len(blob):
            raise ValueError('instruction outside ROM')
        return int.from_bytes(blob[off:off + 2], 'little')
    while pending:
        pc = pending.pop()
        if pc in seen:
            continue
        if not start <= pc < end or pc + 2 > end:
            failures.append(f'branch/fallthrough outside span: {pc:#x}')
            continue
        h = half(pc)
        size, kind, targets = 2, 'next', [pc + 2]
        if h & 0xF800 == 0xF000:
            if pc + 4 > end or half(pc + 2) & 0xF800 != 0xF800:
                failures.append(f'invalid Thumb-1 BL: {pc:#x}')
                continue
            size, kind, targets = 4, 'call', [pc + 4]
        elif h & 0xF800 == 0xE000:
            kind, targets = 'branch', [pc + 4 + sign(h & 0x7FF, 11) * 2]
        elif h & 0xF000 == 0xD000:
            cond = (h >> 8) & 15
            if cond < 14:
                kind, targets = f'cond{cond}', [pc + 2, pc + 4 + sign(h & 255, 8) * 2]
            else:
                failures.append(f'SWI/undefined instruction: {pc:#x}')
                continue
        elif h & 0xFF87 == 0x4700:
            reg = (h >> 3) & 15
            # agbcc interworking epilogue: pop {rN}; bx rN.
            popped = pc > start and half(pc - 2) == (0xBC00 | (1 << reg)) if reg < 8 else False
            if reg == 14 or popped:
                kind, targets = 'return', []
            else:
                failures.append(f'indirect transfer r{reg}: {pc:#x}')
                continue
        elif h & 0xFF00 == 0xBD00:
            kind, targets = 'return', []
        elif h & 0xFC00 == 0x4400 and ((h & 7) | ((h >> 4) & 8)) == 15 and ((h >> 8) & 3) != 1:
            failures.append(f'write to PC: {pc:#x}')
            continue
        elif h >= 0xE800 or h & 0xFF00 in (0xBE00, 0xBF00) or h & 0xFF80 == 0x4780:
            failures.append(f'unsupported instruction: {pc:#x}')
            continue
        if h & 0xF800 == 0x4800:
            pool = ((pc + 4) & ~3) + (h & 255) * 4
            pools.update((pool, pool + 2))
        seen[pc] = (h, size, kind)
        edges.extend((pc, dest) for dest in targets)
        pending.extend(targets)
    if pools.intersection(seen):
        failures.append('reachable instruction overlaps literal pool')
    occupied = set()
    for pc, (_, size, _) in seen.items():
        for pos in range(pc, pc + size, 2):
            if pos in occupied:
                failures.append('overlapping instructions')
            occupied.add(pos)
    return seen, edges, sorted(set(failures))


def instruction_image(rom, instructions):
    """Zero non-code at its original VMA so pool words cannot seed Thumb-2 IT state."""
    data = bytearray(min(len(rom), probe.CODE_END - probe.ROM_BASE))
    for pc, size in instructions.items():
        off = pc - probe.ROM_BASE
        data[off:off + size] = rom[off:off + size]
    return bytes(data)


def disassemble(rom, work, instructions):
    """One process over reachable instructions, retaining original VMAs.

    objdump is bounded to the window the caller actually asked about. The
    image is ROM-base-indexed (because `flow` reads `rom[pc - ROM_BASE]`), so
    an unbounded run disassembles every instruction from the base to the end
    of the highest requested address -- tens of thousands of them -- even when
    the caller passed a forty-byte function. That is what made a single
    instruction signature take milliseconds of objdump work and a
    twelve-candidate search take minutes.
    """
    path = work / 'code.bin'
    path.write_bytes(instruction_image(rom, instructions))
    argv = ['arm-none-eabi-objdump', '-D', '-b', 'binary', '-m', 'armv4t',
            '-M', 'force-thumb', f'--adjust-vma={probe.ROM_BASE}']
    if instructions:
        argv += [f'--start-address={min(instructions):#x}',
                 f'--stop-address={max(pc + size for pc, size in instructions.items()):#x}']
    result = subprocess.run(argv + [str(path)], text=True, capture_output=True, check=True)
    out = {}
    for line in result.stdout.splitlines():
        m = re.match(r'^\s*([0-9a-f]+):\s+(?:[0-9a-f]{4}\s+){1,2}\s*([a-z][a-z0-9.]*)\s*(.*?)\s*$', line)
        if m:
            out[int(m[1], 16)] = (m[2], m[3].split('@')[0].strip())
    return out


def fingerprint(rom, start, end, listing):
    seen, edges, failures = flow(rom, start, end)
    order = {pc: i for i, pc in enumerate(sorted(seen))}
    registers, tokens, parameters = {}, [], []
    def reg(m):
        name = {'ip': 'r12', 'sl': 'r10', 'fp': 'r11', 'sb': 'r9'}.get(m[0], m[0])
        if name in ('sp', 'lr', 'pc'):
            return name
        return registers.setdefault(name, f'v{len(registers)}')
    for pc in sorted(seen):
        h, size, kind = seen[pc]
        mnemonic, operands = listing.get(pc, ('?', ''))
        if mnemonic == '?' or mnemonic.startswith('.'):
            failures.append(f'undecoded instruction: {pc:#x}')
        parameters.append({'offset': pc - start, 'instruction': f'{mnemonic} {operands}'.strip()})
        if kind in ('call', 'branch') or kind.startswith('cond'):
            token = kind
        elif kind == 'return':
            token = 'return:' + mnemonic
        else:
            operands = re.sub(r'\b(?:r\d+|sp|lr|pc|ip|sl|fp|sb)\b', reg, operands)
            operands = re.sub(r'#-?(?:0x[0-9a-f]+|\d+)', '#K', operands)
            token = mnemonic + ' ' + re.sub(r'\s+', '', operands)
        tokens.append(token)
    cfg = sorted((order[a], order.get(b, -1)) for a, b in edges)
    key = json.dumps([tokens, cfg], separators=(',', ':'))
    return {'fingerprint': hashlib.sha256(key.encode()).hexdigest()[:20],
            'tokens': tokens, 'edges': cfg, 'parameters': parameters,
            'complete': not failures, 'limitations': sorted(set(failures))}


def build_index(report, out):
    data = json.loads(report.read_text())
    rom = probe.ROM.read_bytes()
    spans = probe.rom_functions()
    promoted = probe._promoted_vmas()
    records, seen = [], set()
    with tempfile.TemporaryDirectory(prefix='families-') as tmp:
        modes = assembled_modes(Path(tmp))
        instructions = {}
        for rec in data['results']:
            vma = int(rec['vma'], 16)
            if vma not in spans or not 0 < spans[vma] - vma <= 1024:
                continue
            reachable, _, _ = flow(rom, vma, spans[vma])
            instructions.update({pc: item[1] for pc, item in reachable.items() if mode_at(modes, pc) == 't'})
        listing = disassemble(rom, Path(tmp), instructions)
        for rec in data['results']:
            vma = int(rec['vma'], 16)
            identity = (rec['source'], rec.get('alias_of') or rec['name'], vma)
            if identity in seen or vma not in spans:
                continue
            seen.add(identity)
            end = spans[vma]
            if not 0 < end - vma <= 1024:
                continue
            record = {key: rec.get(key) for key in ('name', 'alias_of', 'source', 'vma', 'status')}
            record.update(end_vma=hex(end), promoted=vma in promoted)
            record.update(fingerprint(rom, vma, end, listing))
            for p in record['parameters']:
                if mode_at(modes, vma + p['offset']) != 't':
                    record['complete'] = False
                    record['limitations'].append('reachable address is ARM/data in assembled closure')
                    break
            records.append(record)
    owners = defaultdict(set)
    for rec in records:
        owners[(rec['source'], rec.get('alias_of') or rec['name'])].add(rec['vma'])
    for rec in records:
        if len(owners[(rec['source'], rec.get('alias_of') or rec['name'])]) > 1:
            rec['complete'] = False
            rec['limitations'].append('one C body is attributed to multiple ROM addresses; adjudicate ownership')
    index = {'version': VERSION, 'rom_sha256': digest(probe.ROM), 'inventory_sha256': inventory_digest(),
             'report': str(report.resolve()), 'report_sha256': digest(report),
             'records': records}
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(index, indent=2) + '\n')
    groups = defaultdict(list)
    for rec in records:
        if rec['complete']:
            groups[rec['fingerprint']].append(rec)
    families = [g for g in groups.values() if len(g) > 1]
    lines = ['# ROM sibling families', '', 'Discovery only. Constants, ABI and actual ROM spans require review.', '']
    for group in sorted(families, key=lambda g: (-sum(not r['promoted'] for r in g), -len(g), g[0]['vma'])):
        if not any(not r['promoted'] for r in group):
            continue
        lines += [f"## {group[0]['fingerprint']} ({len(group)} members)", '']
        lines += [f"- `{r['name']}` at {r['vma']} (file offset {int(r['vma'],16)-probe.ROM_BASE:#x}); "
                  f"{'promoted' if r['promoted'] else 'unpromoted'}; recorded {r['status']}" for r in group]
        lines.append('')
    out.with_suffix('.md').write_text('\n'.join(lines))
    print(f'{len(records)} records; {sum(r["complete"] for r in records)} complete CFGs; '
          f'{len(families)} repeated families; {out}')
    return index


def load_index(path):
    data = json.loads(path.read_text())
    if data.get('version') != VERSION or data['rom_sha256'] != digest(probe.ROM):
        raise ValueError('stale family index: ROM/schema changed; rebuild it')
    report = Path(data['report'])
    if not report.exists() or data['report_sha256'] != digest(report):
        raise ValueError('stale family index: corpus report changed; rebuild it')
    if data.get('inventory_sha256') != inventory_digest():
        raise ValueError('stale family index: inventory/tool/manifest changed; rebuild it')
    return data


def find_record(index, name):
    records = [r for r in index['records'] if name in (r['name'], r.get('alias_of'), r['vma'])]
    if len(records) != 1:
        raise ValueError(f'expected one indexed body for {name}, found {len(records)}')
    return records[0]


def nearest(index, target, count=3, recipes=False):
    if not target['complete']:
        return []
    candidates = []
    seen = set()
    for rec in index['records']:
        key = (rec['source'], rec.get('alias_of') or rec['name'])
        if (not rec['complete'] or rec['vma'] == target['vma'] or key in seen or
            key == (target['source'], target.get('alias_of') or target['name'])):
            continue
        if recipes and not (rec['status'] == 'EXACT' or rec['promoted']):
            continue
        seen.add(key)
        score = SequenceMatcher(None, target['tokens'], rec['tokens'], autojunk=False).ratio()
        if score < 0.45:
            continue
        candidates.append(dict(rec, similarity=round(score, 4), same_family=rec['fingerprint'] == target['fingerprint']))
    return sorted(candidates, key=lambda r: (-r['same_family'], -r['similarity'], r['vma']))[:count]


def verified_recipes(index, target, out, count=3):
    """Recompile selected examples now; never attach stale source to old scores."""
    from match_context import function_source
    lines = ['## Closest agbcc recipes', '',
             'Similarity is structural retrieval, not semantic equivalence. Examples below passed a fresh scoped EXACT probe.', '']
    accepted, attempts = 0, []
    for rec in nearest(index, target, count * 2, recipes=True):
        work = out / 'recipes' / rec['name']
        work.mkdir(parents=True, exist_ok=True)
        report = work / 'report.json'
        source_hash = digest(ROOT / rec['source'])
        result = subprocess.run([sys.executable, str(ROOT / 'tools/corpus_match_probe.py'), '--function', rec['name'],
                                 '--c89', '--require-all', '--require-exact', '--work-dir', str(work), '--json', str(report)],
                                text=True, capture_output=True)
        (work / 'probe.log').write_text(result.stdout + result.stderr)
        fresh = json.loads(report.read_text())['results'] if result.returncode == 0 and report.exists() else []
        good = [r for r in fresh if source_hash == digest(ROOT / rec['source']) and
                r['status'] == 'EXACT' and r['vma'] == rec['vma'] and
                r['source'] == rec['source'] and (r.get('alias_of') or r['name']) == (rec.get('alias_of') or rec['name'])]
        attempts.append({'name': rec['name'], 'accepted': len(good) == 1, 'returncode': result.returncode,
                         'source_sha256': source_hash, 'report': str(report), 'similarity': rec['similarity']})
        if len(good) != 1:
            continue
        r = good[0]
        snippet = function_source(ROOT / r['source'], r.get('alias_of') or r['name'], 100)
        lines += [f"### `{r['name']}` ({rec['similarity']:.0%} structural similarity)", '',
                  f"Source: `{r['source']}`; VMA {r['vma']}; file offset {int(r['vma'],16)-probe.ROM_BASE:#x}.",
                  f"Fresh isolated EXACT: {r['rom_bytes']} bytes. This is not a new independent-link result.", '',
                  '```c', snippet, '```', '', 'ROM instruction pattern:', '```asm',
                  *[p['instruction'] for p in rec['parameters']], '```', '']
        accepted += 1
        if accepted == count:
            break
    if not accepted:
        lines += ['No candidate passed fresh exact verification within the retrieval budget.', '']
    (out / 'recipe-verification.json').write_text(json.dumps(attempts, indent=2) + '\n')
    return lines


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--build', action='store_true')
    ap.add_argument('--report', type=Path, default=REPORT)
    ap.add_argument('--index', type=Path, default=DEFAULT_INDEX)
    ap.add_argument('--function')
    ap.add_argument('--top', type=int, default=10)
    args = ap.parse_args()
    if args.top < 1:
        ap.error('--top must be positive')
    try:
        index = build_index(args.report, args.index) if args.build else load_index(args.index)
        if args.function:
            target = find_record(index, args.function)
            print(json.dumps({'target': target, 'siblings': nearest(index, target, args.top)}, indent=2))
        elif not args.build:
            ap.error('supply --build or --function')
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(f'match_families: {exc}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
