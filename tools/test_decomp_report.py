#!/usr/bin/env python3
"""Regression checks for progress accounting and stale snapshot rejection."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import decomp_report as report


class ReportTests(unittest.TestCase):
    def test_accounting_and_overlap(self):
        manifest = {'0x08000100': {'end_vma': '0x08000110', 'asm_file': 'asm/a.s', 'c_name': 'a'}}
        result = report.make_report(manifest)
        self.assertEqual(int(result['measures']['matched_code']), 16)
        self.assertEqual(sum(int(u['measures']['total_code']) for u in result['units']), report.CODE_SIZE)
        self.assertEqual(sum(int(u['measures']['total_data']) for u in result['units']), report.ROM_SIZE - report.CODE_SIZE)
        manifest['0x08000108'] = dict(manifest['0x08000100'])
        with self.assertRaisesRegex(ValueError, 'overlapping'):
            report.make_report(manifest)

    def test_stale_added_deleted_and_tampered_inputs(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for folder in ('src', 'include', 'asm', 'tools', 'docs/data'):
                (root / folder).mkdir(parents=True)
            for path in ('Makefile', 'requirements.txt', 'baserom.sha256', 'src/a.c'):
                (root / path).write_text('original')
            (root / 'tools/matching_slice_functions.json').write_text('{}')
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
