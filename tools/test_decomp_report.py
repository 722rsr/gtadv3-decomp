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
    @staticmethod
    def fixture_report(manifest, ownership, verified=False, source_classes=None):
        spans = ({report.BASE + 0x100: report.BASE + 0x110} if manifest else {})
        return report.make_report(manifest, ownership, function_spans=spans,
                                  verified=verified, source_classes=source_classes)

    def test_measures_keep_match_and_link_axes_independent(self):
        result = report.measures(
            code=100, matched=60, linked_code=100,
            data=40, matched_data=30, linked_data=40,
            total_functions=5, matched_functions=3,
            total_units=3, complete_units=2,
        )
        self.assertEqual(result['matched_code'], '60')
        self.assertEqual(result['complete_code'], '100')
        self.assertEqual(result['matched_data'], '30')
        self.assertEqual(result['complete_data'], '40')
        self.assertEqual(result['matched_functions'], 3)
        self.assertEqual(result['total_functions'], 5)
        self.assertEqual(result['complete_units'], 2)

    def test_accounting_and_overlap(self):
        manifest = {'0x08000100': {'end_vma': '0x08000110', 'asm_file': 'asm/a.s', 'c_name': 'a'}}
        result = self.fixture_report(manifest, ownership_fixture(), verified=True)
        self.assertEqual(int(result['measures']['matched_code']), 16)
        self.assertEqual(int(result['measures']['complete_code']), report.CODE_SIZE)
        self.assertEqual(result['measures']['matched_functions'], 1)
        self.assertEqual(result['measures']['total_functions'], 1)
        self.assertEqual(sum(int(u['measures']['total_code']) for u in result['units']), report.CODE_SIZE)
        self.assertEqual(sum(int(u['measures']['total_data']) for u in result['units']), report.ROM_SIZE - report.CODE_SIZE)
        manifest['0x08000108'] = dict(manifest['0x08000100'])
        with self.assertRaisesRegex(ValueError, 'overlapping'):
            self.fixture_report(manifest, ownership_fixture())

    def test_treemap_colors_areas_and_drill_down(self):
        # The splice owner can differ from the physical region owner.
        manifest = {'0x08000100': {'end_vma': '0x08000110',
                                 'asm_file': 'asm/passthrough.inc', 'c_name': 'selected'}}
        result = self.fixture_report(manifest, ownership_fixture(), verified=True)
        a, b, data = result['units']
        self.assertEqual(a['name'], 'asm/a.s')
        self.assertEqual(a['measures']['total_code'], '512')
        self.assertEqual(a['measures']['matched_code_percent'], 100 * 16 / 512)
        self.assertEqual(a['measures']['complete_code_percent'], 100.0)
        self.assertEqual(b['measures']['matched_code_percent'], 0)
        total_content = report.CODE_SIZE + report.ROM_SIZE - report.CODE_SIZE
        self.assertEqual(result['measures']['fuzzy_match_percent'], 100 * 16 / total_content)
        for unit in (a, b):
            self.assertEqual(sum(int(item['size']) for item in unit['functions'] + unit.get('sections', [])),
                             int(unit['measures']['total_code']))
        self.assertEqual([item['fuzzy_match_percent'] for item in a['functions']], [100])
        self.assertEqual(len(a['sections']), 2)
        self.assertTrue(all(item['name'].startswith('Non-function source bytes')
                            for item in a['sections']))
        manifest['0x08000100']['end_vma'] = '0x08000210'
        with self.assertRaisesRegex(ValueError, 'crosses ownership'):
            self.fixture_report(manifest, ownership_fixture())

    def test_only_decompiled_c_counts_as_matched(self):
        manifest = {'0x08000100': {'end_vma': '0x08000110',
                                   'asm_file': 'asm/a.s', 'c_name': 'a'}}
        for cls, matched in (('c', 16), ('c_fakematch', 16), ('partial', 0), ('asm', 0)):
            with self.subTest(cls=cls):
                result = self.fixture_report(manifest, ownership_fixture(), verified=True,
                                             source_classes={report.BASE + 0x100: cls})
                self.assertEqual(int(result['measures']['matched_code']), matched)
                self.assertEqual(result['measures']['matched_functions'], int(bool(matched)))
                # Linkage is independent of how the source is written.
                self.assertEqual(result['measures']['complete_code'], str(report.CODE_SIZE))
                for unit in result['units']:
                    self.assertTrue(all(section['fuzzy_match_percent'] == 0.0
                                        for section in unit.get('sections', [])))
        with self.assertRaisesRegex(ValueError, 'source classes'):
            self.fixture_report(manifest, ownership_fixture(), source_classes={})

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
                self.fixture_report({}, rows)

    def test_real_inventory_has_scoped_function_counts_and_excludes_padding(self):
        result = json.loads(report.expected_report(report.ROOT, verified=True))
        self.assertEqual(result['measures']['total_code'], str(report.CODE_SIZE))
        manifest = json.loads((report.ROOT / 'tools/matching_slice_functions.json').read_text())
        expected = sum(int(row['end_vma'], 16) - int(start, 16) for start, row in manifest.items())
        quality = report.source_quality.classify(report.ROOT)
        decompiled = [row for row in quality['functions'].values()
                      if row['class'] in report.source_quality.DECOMPILED]
        self.assertEqual(int(result['measures']['matched_code']),
                         sum(row['size'] for row in decompiled))
        self.assertLess(int(result['measures']['matched_code']), expected)
        self.assertEqual(result['measures']['total_functions'], len(report.corpus_match_probe._rom_functions_scan()))
        self.assertEqual(result['measures']['matched_functions'], len(decompiled))
        ownership = json.loads((report.ROOT / 'docs/data/code_data_ownership.json').read_text())
        content_end = int(ownership['rom']['content_end'], 16)
        data_content_bytes = content_end - report.CODE_SIZE
        self.assertEqual(result['measures']['total_data'], str(data_content_bytes))
        self.assertEqual(result['measures']['matched_data'], str(data_content_bytes))
        self.assertEqual(result['measures']['complete_code'], str(report.CODE_SIZE))
        self.assertEqual(result['measures']['complete_data'], str(data_content_bytes))
        self.assertEqual(result['measures']['total_units'], result['measures']['complete_units'])
        code = [unit for unit in result['units'] if int(unit['measures']['total_code'])]
        self.assertGreater(len(code), 200)
        self.assertTrue(all(unit['name'].startswith('asm/') for unit in code))
        self.assertEqual(sum(int(unit['measures']['total_code']) for unit in code), report.CODE_SIZE)
        self.assertTrue(any(unit['measures']['fuzzy_match_percent'] > 0 for unit in code))
        sections = [section for unit in code for section in unit.get('sections', [])]
        functions = [item for unit in code for item in unit['functions']]
        self.assertEqual(sum(int(section['size']) for section in sections),
                         report.CODE_SIZE - sum(int(item['size']) for item in functions))
        self.assertTrue(all(section['fuzzy_match_percent'] == 0.0
                            for section in sections))
        for unit in code:
            self.assertEqual(sum(int(item['size']) for item in unit['functions'] + unit.get('sections', [])),
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
            data = report.expected_report(root, verified=True)
            (root / report.REPORT).write_bytes(data)
            (root / report.STAMP).write_bytes(report.encoded({
                'version': report.STAMP_VERSION,
                'inputs': report.inputs(root),
                'report_sha256': report.digest(data),
                'verification': report.VERIFICATION,
            }))
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
