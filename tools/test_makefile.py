#!/usr/bin/env python3
"""ROM-free regressions for Make's verification and incremental build rules."""
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class MakefileTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        shutil.copyfile(ROOT / 'Makefile', self.root / 'Makefile')
        for folder in ('build', 'build-code', 'asm/macros'):
            (self.root / folder).mkdir(parents=True)
        self.reference = b'synthetic reference, not ROM data'
        (self.root / 'baserom.gba').write_bytes(self.reference)
        (self.root / 'build/gtadv3.gba').write_bytes(self.reference)
        (self.root / 'baserom.sha256').write_text(
            hashlib.sha256(self.reference).hexdigest() + '  baserom.gba\n')

    def make(self, *args):
        return subprocess.run(['make', '--no-print-directory', *args],
                              cwd=self.root, text=True, capture_output=True)

    def verify(self, *args):
        # Suppress rebuilding only in these fixtures to exercise the real
        # verification recipe with missing/corrupt output and no ARM tools.
        return self.make('-o', 'build/gtadv3.gba', 'verify', *args)

    def test_valid_reference_and_output(self):
        result = self.verify()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_missing_inputs_fail(self):
        for names in [('baserom.gba',), ('build/gtadv3.gba',),
                      ('baserom.gba', 'build/gtadv3.gba'), ('baserom.sha256',)]:
            with self.subTest(names=names):
                saved = {name: (self.root / name).read_bytes() for name in names}
                for name in names:
                    (self.root / name).unlink()
                result = self.verify()
                self.assertNotEqual(result.returncode, 0)
                self.assertNotIn('byte-identical', result.stdout)
                for name, data in saved.items():
                    (self.root / name).write_bytes(data)

    def test_matching_but_wrong_rom_fails(self):
        for name in ('baserom.gba', 'build/gtadv3.gba'):
            (self.root / name).write_bytes(b'identical, but not the pinned reference')
        self.assertNotEqual(self.verify().returncode, 0)

    def test_different_output_and_failed_hash_command_fail(self):
        (self.root / 'build/gtadv3.gba').write_bytes(b'wrong output')
        self.assertNotEqual(self.verify().returncode, 0)
        (self.root / 'build/gtadv3.gba').write_bytes(self.reference)
        self.assertNotEqual(self.verify('SHA=false').returncode, 0)

    def test_incremental_dependencies(self):
        sources = ('asm/rom.s', 'asm/code.s', 'asm/data_tail.s',
                   'asm/macros/function.inc', 'asm/extra.s')
        for name in sources:
            (self.root / name).write_text('@ synthetic source\n')
        for name in (*sources, 'baserom.gba'):
            os.utime(self.root / name, (1_000_000_000, 1_000_000_000))
        for target in ('build/rom.o', 'build-code/code.o', 'build-code/data.o'):
            (self.root / target).write_bytes(b'fixture')
            os.utime(self.root / target, (1_000_000_100, 1_000_000_100))
            self.assertEqual(self.make('-q', target).returncode, 0)
        cases = [('build/rom.o', name) for name in (*sources, 'baserom.gba')]
        cases += [('build-code/data.o', 'baserom.gba'),
                  ('build-code/data.o', 'asm/data_tail.s'),
                  ('build-code/code.o', 'asm/macros/function.inc')]
        for target, source in cases:
            with self.subTest(target=target, source=source):
                result = self.make('-q', '-W', source, target)
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)

    def test_cleanup_targets(self):
        for target in ('clean', 'distclean'):
            with self.subTest(target=target):
                outputs = ('build/temp.o', 'build-code/code.o',
                           'build-slice/code.elf', 'build-c/output.o', 'build-s/output.o')
                preserved = ('baserom.gba', 'src/example.c', 'tools/asm-differ/diff.py')
                compiler = 'build/toolchains/agbcc/old_agbcc'
                for name in (*outputs, *preserved, compiler):
                    path = self.root / name
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text('fixture')
                result = self.make(target)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                for name in outputs:
                    self.assertFalse((self.root / name).exists(), name)
                for name in preserved:
                    self.assertTrue((self.root / name).is_file(), name)
                self.assertEqual((self.root / compiler).exists(), target == 'clean')

    def test_verify_builds_before_hash_check(self):
        # No assembler is invoked in this dry run. A standalone verify must
        # include the complete build before the hash check, even with -j.
        for name in ('asm/rom.s', 'asm/code.s', 'asm/data_tail.s', 'ldscript.ld'):
            (self.root / name).write_text('fixture\n')
        (self.root / 'build/gtadv3.gba').unlink()
        result = self.make('-n', '-j4', 'verify')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertLess(result.stdout.index('arm-none-eabi-objcopy'),
                        result.stdout.index('shasum -a 256 -c'))


if __name__ == '__main__':
    unittest.main()
