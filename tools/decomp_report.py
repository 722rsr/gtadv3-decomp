#!/usr/bin/env python3
"""Publish a ROM-free objdiff v2 snapshot after local byte verification."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

import corpus_match_probe
import data_regions

ROOT = Path(__file__).resolve().parents[1]
REPORT = Path('docs/data/report.json')
STAMP = Path('docs/data/report-inputs.json')
BASE = 0x08000000
CODE_SIZE = 0x2E158
ROM_SIZE = 0x800000
STAMP_VERSION = 4
VERIFICATION = {
    'command': 'make full-rom-match',
    'reference_rom': 'pass',
    'independent_c_asm_code_slice': 'pass',
    'full_hybrid_rom_link': 'pass',
    'function_entry_census': 'pass',
    'runtime_equivalence': 'not_run',
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def inputs(root):
    # Deliberately independent of git's index: also catches new source files.
    paths = set()
    for directory in ('src', 'include', 'asm'):
        paths.update(p for p in (root / directory).rglob('*') if p.is_file())
    for pattern in ('tools/*.py', 'tools/*.sh', 'tools/*.json',
                    'tools/third_party/**/*', 'docs/data/*.json', 'ldscript*.ld'):
        paths.update(p for p in root.glob(pattern) if p.is_file())
    paths.update(root / p for p in ('Makefile', 'requirements.txt', 'baserom.sha256'))
    return {p.relative_to(root).as_posix(): digest(p.read_bytes())
            for p in sorted(paths)
            if p.relative_to(root) not in (REPORT, STAMP)
            and p.name != '.DS_Store' and '__pycache__' not in p.parts}


def measures(code=0, matched=0, linked_code=0, data=0, matched_data=0,
             linked_data=0, total_functions=0, matched_functions=0,
             total_units=0, complete_units=0):
    """Build Objdiff v2 measures with exact matching and linking kept separate."""
    for label, value, total in (
            ('matched code', matched, code), ('linked code', linked_code, code),
            ('matched data', matched_data, data), ('linked data', linked_data, data),
            ('matched functions', matched_functions, total_functions),
            ('complete units', complete_units, total_units)):
        if value < 0 or value > total:
            raise ValueError(f'{label} ({value}) is outside its total ({total})')

    code_percent = 100.0 * matched / code if code else 0.0
    data_percent = 100.0 * matched_data / data if data else 0.0
    linked_code_percent = 100.0 * linked_code / code if code else 0.0
    linked_data_percent = 100.0 * linked_data / data if data else 0.0
    function_percent = 100.0 * matched_functions / total_functions if total_functions else 0.0
    byte_total = code + data
    fuzzy_percent = 100.0 * (matched + matched_data) / byte_total if byte_total else 0.0
    return {
        'fuzzy_match_percent': fuzzy_percent,
        'total_code': str(code), 'matched_code': str(matched),
        'matched_code_percent': code_percent,
        'complete_code': str(linked_code), 'complete_code_percent': linked_code_percent,
        'total_data': str(data), 'matched_data': str(matched_data),
        'matched_data_percent': data_percent,
        'complete_data': str(linked_data), 'complete_data_percent': linked_data_percent,
        'total_functions': total_functions, 'matched_functions': matched_functions,
        'matched_functions_percent': function_percent,
        'total_units': total_units, 'complete_units': complete_units,
    }


def make_report(manifest, ownership, data_manifest=(), function_spans=None,
                verified=False, code_match_verified=False):
    # Use the audited ROM partition, not the splice file: some manifest entries
    # replace includes in passthrough.inc but belong to a physical .s region.
    regions = sorted(ownership['segments'], key=lambda row: int(row['start'], 16))
    cursor = 0
    for region in regions:
        start, end = int(region['start'], 16), int(region['end'], 16)
        if start != cursor or end <= start or end > ROM_SIZE or region['size'] != end - start:
            raise ValueError('ownership regions must partition the ROM without gaps or overlaps')
        if bool(region['executable']) != (end <= CODE_SIZE) or start < CODE_SIZE < end:
            raise ValueError('ownership executable boundary differs from CODE_SIZE')
        if region['executable'] and len(region['source_owners']) != 1:
            raise ValueError('executable region must have one source owner')
        cursor = end
    if cursor != ROM_SIZE:
        raise ValueError('ownership regions do not cover the full ROM')

    selected = {}
    previous_end = BASE
    for start, entry in sorted(manifest.items(), key=lambda pair: int(pair[0], 16)):
        address, end = int(start, 16), int(entry['end_vma'], 16)
        if not BASE <= address < end <= BASE + CODE_SIZE or address < previous_end:
            raise ValueError(f'invalid or overlapping selected span: {start}')
        previous_end = end
        selected[address] = {'end': end, 'name': entry['c_name']}

    if function_spans is None:
        # Match the address inventory used by corpus matching: .type entries
        # and BL targets known to the source tree. The independent prologue and
        # shape audit in fast-check verifies that this inventory misses no
        # executable entry in the owned code slice.
        function_spans = corpus_match_probe._rom_functions_scan()
    if not set(selected).issubset(function_spans):
        missing = sorted(set(selected) - set(function_spans))
        raise ValueError('selected C entries are missing from the known function inventory: '
                         f'{[hex(vma) for vma in missing[:5]]}')

    groups = {}
    matched_code = 0
    matched_function_count = 0
    linked_code = 0
    for region in regions:
        if not region['executable']:
            continue
        start, end = int(region['start'], 16), int(region['end'], 16)
        name = region['source_owners'][0]
        group = groups.setdefault(name, {
            'size': 0, 'matched': 0, 'linked': 0, 'functions': [],
            'sections': [], 'total_functions': 0, 'matched_functions': 0,
        })
        group['size'] += end - start
        group['linked'] += end - start if verified else 0

        entries = []
        for vma, inferred_end in sorted(function_spans.items()):
            first = vma - BASE
            if not start <= first < end:
                continue
            selected_entry = selected.get(vma)
            last = ((selected_entry['end'] - BASE) if selected_entry
                    else inferred_end - BASE)
            if not first < last <= end:
                raise ValueError(f'function span {vma:#010x}-{BASE + last:#010x} '
                                 f'crosses ownership boundary {BASE + start:#010x}-{BASE + end:#010x}')
            if selected_entry and last > inferred_end - BASE:
                raise ValueError(f'selected span exceeds known ROM function extent: {selected_entry["name"]}')
            entries.append((first, last,
                            selected_entry['name'] if selected_entry else f'_{vma:08X}',
                            selected_entry is not None))

        cursor = start
        for first, last, function, is_matched in entries:
            if cursor < first:
                group['sections'].append(range_item(cursor, first, code_match_verified))
                if code_match_verified:
                    section_size = first - cursor
                    group['matched'] += section_size
                    matched_code += section_size
            if first < cursor:
                raise ValueError(f'overlapping known function span at {function}')
            size = last - first
            group['functions'].append({
                'name': function, 'size': str(size),
                'fuzzy_match_percent': 100.0 if is_matched else 0.0,
                'metadata': {'virtual_address': str(BASE + first)},
            })
            group['total_functions'] += 1
            if is_matched:
                group['matched'] += size
                group['matched_functions'] += 1
                matched_code += size
                matched_function_count += 1
            cursor = last
        if cursor < end:
            group['sections'].append(range_item(cursor, end, code_match_verified))
            if code_match_verified:
                section_size = end - cursor
                group['matched'] += section_size
                matched_code += section_size

    units = []
    for name, group in sorted(groups.items()):
        linked_code += group['linked']
        complete = bool(verified)
        unit = {
            'name': name,
            'measures': measures(
                code=group['size'], matched=group['matched'], linked_code=group['linked'],
                total_functions=group['total_functions'],
                matched_functions=group['matched_functions'],
                total_units=1, complete_units=int(complete)),
            'functions': group['functions'],
            'metadata': {'complete': complete, 'source_path': name},
        }
        if group['sections']:
            unit['sections'] = group['sections']
        units.append(unit)

    registered = {(int(row['start'], 16), int(row['end'], 16)): row for row in data_manifest}
    seen = set()
    content_end = int(ownership.get('rom', {}).get('content_end', hex(ROM_SIZE)), 16)
    if not CODE_SIZE <= content_end <= ROM_SIZE:
        raise ValueError('ownership content_end is outside the data tail')
    data_total = content_end - CODE_SIZE
    represented_data = 0
    matched_data = 0
    linked_data = 0
    for region in regions:
        if region['executable']:
            continue
        key = (int(region['start'], 16), int(region['end'], 16))
        start, end = key
        if start >= content_end:
            continue  # Trailing zero padding is intentionally outside data progress.
        if end > content_end:
            raise ValueError('ownership map must split content bytes from trailing ROM padding')
        size = end - start
        represented_data += size
        row = registered.get(key)
        is_verified = bool(
            row is not None
            and region.get('classification') == 'generated-data'
            and region.get('status') == 'verified-generated-data'
            and region.get('backing') == row['binary']
            and region.get('intended_owner') == row['id'])
        if (row is not None or region.get('classification') == 'generated-data') and not is_verified:
            raise ValueError('generated data ownership is not registered and verified')
        if row is not None:
            seen.add(key)
            name = row['name']
        else:
            name = f'Unverified data 0x{start:06X}-0x{end:06X}'
        matched_bytes = size if is_verified else 0
        linked_bytes = matched_bytes if verified else 0
        matched_data += matched_bytes
        linked_data += linked_bytes
        complete = bool(verified and is_verified)
        units.append({
            'name': name,
            'measures': measures(
                data=size, matched_data=matched_bytes, linked_data=linked_bytes,
                total_units=1, complete_units=int(complete)),
            'metadata': {'complete': complete},
        })
    if represented_data != data_total:
        raise ValueError(f'ownership map covers {represented_data} data bytes before padding, '
                         f'expected {data_total}')
    if seen != set(registered):
        raise ValueError('registered data regions are missing from the ownership map')
    if matched_function_count != len(selected):
        raise ValueError('selected C manifest entries do not map one-to-one to known functions')

    complete_units = sum(int(unit['metadata']['complete']) for unit in units)
    return {
        'version': 2,
        'measures': measures(
            code=CODE_SIZE, matched=matched_code, linked_code=linked_code,
            data=data_total, matched_data=matched_data, linked_data=linked_data,
            total_functions=sum(group['total_functions'] for group in groups.values()),
            matched_functions=matched_function_count,
            total_units=len(units), complete_units=complete_units),
        'units': units,
    }


def range_item(start, end, matched=False):
    # Keep non-function bytes as source-owned sections rather than inventing
    # function entries. The independent full code-slice comparison verifies
    # these spans just as it verifies the selected function bodies.
    return {'name': f'Non-function source bytes 0x{BASE + start:08X}-0x{BASE + end:08X}',
            'size': str(end - start),
            'fuzzy_match_percent': 100.0 if matched else 0.0,
            'metadata': {'virtual_address': str(BASE + start)}}


def encoded(value):
    return (json.dumps(value, indent=2, sort_keys=True) + '\n').encode()


def expected_report(root, verified=False):
    return encoded(make_report(
        json.loads((root / 'tools/matching_slice_functions.json').read_text()),
        json.loads((root / 'docs/data/code_data_ownership.json').read_text()),
        data_regions.load_regions(root),
        function_spans=(corpus_match_probe._rom_functions_scan()
                        if root.resolve() == ROOT.resolve() else {}),
        verified=verified,
        code_match_verified=verified))


def check(root):
    data = (root / REPORT).read_bytes()
    stamp = json.loads((root / STAMP).read_text())
    if stamp != {'version': STAMP_VERSION, 'inputs': inputs(root),
                 'report_sha256': digest(data), 'verification': VERIFICATION}:
        raise ValueError('progress snapshot is stale; run make progress locally and commit both report files')
    if data != expected_report(root, verified=True):
        raise ValueError('progress report differs from the selected C manifest')
    print('progress-check: snapshot and source fingerprints agree (no ROM build performed)')


def refresh(root):
    before = inputs(root)
    subprocess.run(['shasum', '-a', '256', '-c', 'baserom.sha256'], cwd=root, check=True)
    subprocess.run(['make', 'full-rom-match'], cwd=root, check=True)
    if inputs(root) != before:
        raise ValueError('inputs changed during verification; finish edits and rerun make progress')
    data = expected_report(root, verified=True)
    (root / REPORT).write_bytes(data)
    (root / STAMP).write_bytes(encoded({
        'version': STAMP_VERSION,
        'inputs': before,
        'report_sha256': digest(data),
        'verification': VERIFICATION,
    }))
    check(root)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--refresh', action='store_true', help='run full local verification and replace snapshot')
    args = parser.parse_args()
    try:
        refresh(ROOT) if args.refresh else check(ROOT)
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        parser.exit(1, f'decomp_report: {exc}\n')


if __name__ == '__main__':
    main()
