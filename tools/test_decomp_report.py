#!/usr/bin/env python3
"""Regression checks for progress accounting and stale snapshot rejection."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import decomp_report as report


def ownership_fixture():
    return {'segments': [
        {'start': '0x0', 'end': '0x200', 'size': 0x200,
         'executable': True, 'source_owners': ['asm/a.s']},
        {'start': '0x200', 'end': hex(report.CODE_SIZE), 'size': report.CODE_SIZE - 0x200,
         'executable': True, 'source_owners': ['asm/b.s']},
        {'start': hex(report.CODE_SIZE), 'end': hex(report.ROM_SIZE),
         'size': report.ROM_SIZE - report.CODE_SIZE, 'executable': False, 'source_owners': []},
    ]}


class ReportTests(unittest.TestCase):
    def test_accounting_and_overlap(self):
        manifest = {'0x08000100': {'end_vma': '0x08000110', 'asm_file': 'asm/a.s', 'c_name': 'a'}}
        result = report.make_report(manifest, ownership_fixture())
        self.assertEqual(int(result['measures']['matched_code']), 16)
        self.assertEqual(sum(int(u['measures']['total_code']) for u in result['units']), report.CODE_SIZE)
        self.assertEqual(sum(int(u['measures']['total_data']) for u in result['units']), report.ROM_SIZE - report.CODE_SIZE)
        manifest['0x08000108'] = dict(manifest['0x08000100'])
        with self.assertRaisesRegex(ValueError, 'overlapping'):
            report.make_report(manifest, ownership_fixture())

    def test_treemap_colors_areas_and_drill_down(self):
        # The splice owner can differ from the physical region owner.
        manifest = {'0x08000100': {'end_vma': '0x08000110',
                                 'asm_file': 'asm/passthrough.inc', 'c_name': 'selected'}}
        result = report.make_report(manifest, ownership_fixture())
        a, b, data = result['units']
        self.assertEqual(a['name'], 'asm/a.s')
        self.assertEqual(a['measures']['total_code'], '512')
        self.assertEqual(a['measures']['fuzzy_match_percent'], 100 * 16 / 512)
        self.assertEqual(b['measures']['fuzzy_match_percent'], 0)
        self.assertEqual(result['measures']['fuzzy_match_percent'], 100 * 16 / report.CODE_SIZE)
        for unit in (a, b):
            self.assertEqual(sum(int(item['size']) for item in unit['functions']),
                             int(unit['measures']['total_code']))
        self.assertEqual([item['fuzzy_match_percent'] for item in a['functions']], [0, 100, 0])
        self.assertTrue(a['functions'][0]['name'].startswith('Unselected range'))
        manifest['0x08000100']['end_vma'] = '0x08000210'
        with self.assertRaisesRegex(ValueError, 'crosses ownership'):
            report.make_report(manifest, ownership_fixture())

    def test_rejects_bad_ownership_partition(self):
        for modification in ('gap', 'overlap', 'size', 'truncated', 'boundary'):
            rows = ownership_fixture()
            if modification == 'gap':
                rows['segments'][1]['start'] = '0x204'
            elif modification == 'overlap':
                rows['segments'][1]['start'] = '0x1fc'
            elif modification == 'size':
                rows['segments'][0]['size'] = 1
            elif modification == 'truncated':
                rows['segments'].pop()
            else:
                rows['segments'][1]['executable'] = False
            with self.subTest(modification=modification), self.assertRaises(ValueError):
                report.make_report({}, rows)

    def test_real_inventory_preserves_totals_and_has_no_catchall(self):
        result = json.loads(report.expected_report(report.ROOT))
        self.assertEqual(result['measures']['total_code'], str(report.CODE_SIZE))
        # Keep this test independent of growth in selected C ownership.
        manifest = json.loads((report.ROOT / 'tools/matching_slice_functions.json').read_text())
        expected = sum(int(row['end_vma'], 16) - int(start, 16) for start, row in manifest.items())
        self.assertEqual(int(result['measures']['matched_code']), expected)
        code = [unit for unit in result['units'] if int(unit['measures']['total_code'])]
        self.assertGreater(len(code), 200)
        self.assertTrue(all(unit['name'].startswith('asm/') for unit in code))
        self.assertEqual(sum(int(unit['measures']['total_code']) for unit in code), report.CODE_SIZE)
        self.assertTrue(any(unit['measures']['fuzzy_match_percent'] > 0 for unit in code))
        for unit in code:
            self.assertEqual(sum(int(item['size']) for item in unit['functions']),
                             int(unit['measures']['total_code']))

    def test_stale_added_deleted_and_tampered_inputs(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for folder in ('src', 'include', 'asm', 'tools', 'docs/data'):
                (root / folder).mkdir(parents=True)
            for path in ('Makefile', 'requirements.txt', 'baserom.sha256', 'src/a.c'):
                (root / path).write_text('original')
            (root / 'tools/matching_slice_functions.json').write_text('{}')
            (root / 'tools/data_regions.json').write_text(json.dumps({'version': 1, 'regions': []}))
            (root / 'docs/data/code_data_ownership.json').write_text(json.dumps(ownership_fixture()))
            data = report.expected_report(root)
            (root / report.REPORT).write_bytes(data)
            (root / report.STAMP).write_bytes(report.encoded({'version': 1, 'inputs': report.inputs(root), 'report_sha256': report.digest(data)}))
            report.check(root)
            for change in ('modify', 'add', 'delete', 'tamper'):
                with self.subTest(change=change):
                    source = root / 'src/a.c'
                    if change == 'modify':
                        source.write_text('modified')
                    elif change == 'add':
                        (root / 'src/new.c').write_text('new')
                    elif change == 'delete':
                        source.unlink()
                    else:
                        (root / report.REPORT).write_text('{}')
                    with self.assertRaisesRegex(ValueError, 'stale'):
                        report.check(root)
                    source.write_text('original')
                    (root / 'src/new.c').unlink(missing_ok=True)
                    (root / report.REPORT).write_bytes(data)
            # A failed full gate must never overwrite the previous snapshot.
            import subprocess
            with patch.object(report.subprocess, 'run', side_effect=subprocess.CalledProcessError(1, 'make')):
                with self.assertRaises(subprocess.CalledProcessError):
                    report.refresh(root)
            self.assertEqual((root / report.REPORT).read_bytes(), data)


if __name__ == '__main__':
    unittest.main()
