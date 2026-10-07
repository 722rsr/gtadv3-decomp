#!/usr/bin/env python3
"""Publish a ROM-free objdiff v2 snapshot after local byte verification."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
REPORT = Path('docs/data/report.json')
STAMP = Path('docs/data/report-inputs.json')
BASE = 0x08000000
CODE_SIZE = 0x2E158
ROM_SIZE = 0x800000


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


def measures(code=0, matched=0, data=0):
    percent = 100.0 * matched / code if code else 0.0
    return {'total_code': str(code), 'matched_code': str(matched),
            'matched_code_percent': percent, 'complete_code': str(matched),
            'complete_code_percent': percent, 'total_data': str(data),
            'matched_data': '0', 'complete_data': '0'}


def make_report(manifest):
    groups = {}
    previous_end = BASE
    matched = 0
    for start, entry in sorted(manifest.items(), key=lambda pair: int(pair[0], 16)):
        address, end = int(start, 16), int(entry['end_vma'], 16)
        if not BASE <= address < end <= BASE + CODE_SIZE or address < previous_end:
            raise ValueError(f'invalid or overlapping selected span: {start}')
        previous_end = end
        size = end - address
        matched += size
        groups.setdefault(entry['asm_file'], []).append({
            'name': entry['c_name'], 'size': str(size), 'fuzzy_match_percent': 100.0,
            'metadata': {'virtual_address': str(address)}})
    units = []
    for name, functions in sorted(groups.items()):
        size = sum(int(f['size']) for f in functions)
        units.append({'name': name + ' (selected C spans)',
                      'measures': measures(size, size), 'functions': functions})
    units.extend([
        {'name': 'Remaining executable slice (includes header and padding)',
         'measures': measures(CODE_SIZE - matched)},
        {'name': 'Unreconstructed data tail (includes ROM padding)',
         'measures': measures(data=ROM_SIZE - CODE_SIZE)},
    ])
    # Function totals and fuzzy scores are intentionally omitted: the manifest
    # is a selection, not an exhaustive function census or a partial-match scan.
    return {'version': 2, 'measures': measures(CODE_SIZE, matched, ROM_SIZE - CODE_SIZE),
            'units': units}


def encoded(value):
    return (json.dumps(value, indent=2, sort_keys=True) + '\n').encode()


def expected_report(root):
    return encoded(make_report(json.loads((root / 'tools/matching_slice_functions.json').read_text())))


def check(root):
    data = (root / REPORT).read_bytes()
    stamp = json.loads((root / STAMP).read_text())
    if stamp != {'version': 1, 'inputs': inputs(root), 'report_sha256': digest(data)}:
        raise ValueError('progress snapshot is stale; run make progress locally and commit both report files')
    if data != expected_report(root):
        raise ValueError('progress report differs from the selected C manifest')
    print('progress-check: snapshot and source fingerprints agree (no ROM build performed)')


def refresh(root):
    before = inputs(root)
    subprocess.run(['shasum', '-a', '256', '-c', 'baserom.sha256'], cwd=root, check=True)
    subprocess.run(['make', 'matching-ready'], cwd=root, check=True)
    if inputs(root) != before:
        raise ValueError('inputs changed during verification; finish edits and rerun make progress')
    data = expected_report(root)
    (root / REPORT).write_bytes(data)
    (root / STAMP).write_bytes(encoded({'version': 1, 'inputs': before,
                                      'report_sha256': digest(data)}))
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
