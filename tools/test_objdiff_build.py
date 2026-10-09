#!/usr/bin/env python3
"""Objdiff inventory and ELF regressions, without private ROM inputs."""
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from elftools.elf.elffile import ELFFile

import objdiff_build as viewer


class ConfigTests(unittest.TestCase):
    def test_deduplicates_aliases_and_prefers_strong_owner(self):
        a = viewer.ROOT / 'src/a.c'
        b = viewer.ROOT / 'src/b.c'
        rows = [(a, 'weak', 0x08002000, 20, True),
                (b, 'real', 0x08002000, 4, False),
                (b, 'alias', 0x08002000, 4, False),
                (a, 'unknown_span', 0x08003000, 4, False)]
        with patch.object(viewer.probe, 'rom_functions', return_value={0x08002000: 0x08002008}), \
             patch.object(viewer.probe, 'src_functions', return_value=rows):
            config = viewer.configuration()
        self.assertEqual(len(config['units']), 1)
        self.assertEqual(config['units'][0]['name'], 'b/real')
        self.assertEqual(config['units'][0]['metadata'], {'source_path': 'src/b.c'})
        self.assertEqual(config['custom_args'], ['tools/objdiff_build.py'])
        self.assertTrue(config['build_base'])
        self.assertFalse(config['build_target'])  # the base build publishes both objects

    def test_unknown_object_path_refused(self):
        with self.assertRaisesRegex(ValueError, 'not a configured unit'):
            viewer.configured_unit('/tmp/not-an-objdiff-unit.o')

    def test_failed_build_removes_stale_pair(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            directory = root / 'build/objdiff/08002000'
            directory.mkdir(parents=True)
            for name in ('target.o', 'candidate.o', 'comparison.json'):
                (directory / name).write_text('stale')
            unit = {'name': 'a/f', 'target_path': 'build/objdiff/08002000/target.o',
                    'base_path': 'build/objdiff/08002000/candidate.o'}
            with patch.object(viewer, 'ROOT', root), patch.object(viewer.probe, 'ROM', root / 'missing.gba'):
                with self.assertRaises(FileNotFoundError):
                    viewer.build(unit)
            self.assertEqual(list(directory.iterdir()), [])


@unittest.skipUnless(shutil.which('arm-none-eabi-objcopy'), 'ARM binutils unavailable')
class ElfTests(unittest.TestCase):
    def test_sized_function_and_exact_mapping_names_at_rom_address(self):
        with tempfile.TemporaryDirectory() as tmp:
            obj = Path(tmp) / 'target.o'
            data = bytes.fromhex('00487047801700037047')
            viewer.named_object(data, 0x08002000, [(0, '$t'), (4, '$d'), (8, '$t')], 'function', obj)
            with obj.open('rb') as stream:
                elf = ELFFile(stream)
                self.assertEqual(elf['e_type'], 'ET_EXEC')
                self.assertEqual(elf.get_section_by_name('.text').data(), data)
                rows = list(elf.get_section_by_name('.symtab').iter_symbols())
                symbols = {s.name: s.entry for s in rows}
                self.assertEqual(symbols['function']['st_size'], 10)
                self.assertEqual(symbols['function']['st_value'], 0x08002001)
                self.assertEqual([s['st_value'] for s in rows if s.name == '$t'], [0x08002000, 0x08002008])
                self.assertEqual(symbols['$d']['st_value'], 0x08002004)

    def test_real_objdiff_decodes_thumb_and_detects_changed_operand(self):
        cli = shutil.which('objdiff-cli') or viewer.ROOT / 'build/toolchains/objdiff/objdiff-cli'
        if not Path(cli).is_file():
            self.skipTest('optional objdiff-cli unavailable')
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            target, base, report = (root / n for n in ('target.o', 'base.o', 'diff.json'))
            viewer.named_object(bytes.fromhex('01207047'), 0x08002000, [(0, '$t')], 'f', target)
            for value in (1, 2):
                viewer.named_object(bytes([value, 0x20, 0x70, 0x47]), 0x08002000, [(0, '$t')], 'f', base)
                subprocess.run([str(cli), 'diff', '-1', str(target), '-2', str(base), '-o', str(report)],
                               check=True, capture_output=True)
                data = json.loads(report.read_text())
                function = next(s for s in data['left']['symbols'] if s['name'] == 'f')
                instructions = [i['instruction'] for i in function['instructions']]
                self.assertEqual(instructions[0]['size'], 2)
                self.assertTrue(instructions[0]['formatted'].startswith('mov'))
                self.assertEqual(function['match_percent'] == 100, value == 1)


if __name__ == '__main__':
    unittest.main()
