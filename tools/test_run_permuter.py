#!/usr/bin/env python3
"""Permuter adapter regressions; no ROM or downloaded fork required."""
import base64
import hashlib
import json
import re
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from elftools.elf.elffile import ELFFile

import run_permuter as perm


class SourceTests(unittest.TestCase):
    def test_multiline_body_keeps_real_translation_unit(self):
        source = '''typedef unsigned int u32;
struct S { u32 x; } __attribute__((packed));
static u32 table[] = {1, 2};
static u32 helper(u32 x) { return x + table[0]; }
u32
target(u32 x)
{
    /* a brace in a string must not end the function */
    const char *p = "}";
    return helper(x) + p[0];
}
u32 alias(u32) __attribute__((alias("target")));
'''
        base, before, after = perm.prepare_source(source, "target")
        self.assertIn('static u32 helper(u32 x) { return x + table[0]; }', before)
        self.assertIn('__attribute__((packed))', before)
        self.assertIn('__attribute__((alias("target")))', after)
        self.assertIn('static u32 helper(u32 x) ;', base)
        self.assertIn('const char *p = "}";', base)
        self.assertNotIn('__attribute__', base)

    def test_inline_asm_is_preserved_as_literal(self):
        base, _, _ = perm.prepare_source('void f(void) { __asm__("nop"); }', 'f')
        self.assertIn('#pragma _permuter b64literal', base)

    def test_unsupported_extensions_are_not_silently_dropped(self):
        for source in ('int f(void) { return ({ int x = 1; x; }); }',
                       'int __attribute__((weak)) f(void) { return 1; }',
                       'int f(void) { register int a __asm__("r1"), b; return a; }'):
            with self.subTest(source=source), self.assertRaisesRegex(ValueError, 'unsupported target'):
                perm.prepare_source(source, 'f')

    def test_pinned_register_declarator_is_preserved_verbatim(self):
        source = ('void f(void) {\n'
                  '    register volatile u8 *b __asm__("r1") = *cell;\n'
                  '    b[22] = 16;\n'
                  '}\n')
        base, before, after = perm.prepare_source(source, 'f')
        match = re.search(r'#pragma _permuter b64literal (\S+)', base)
        self.assertIsNotNone(match)
        declaration = base64.b64decode(match.group(1)).decode()
        self.assertEqual(declaration.strip(), 'register volatile u8 *b __asm__("r1") = *cell;')
        # The randomizer needs the type; the compiler sees only the literal.
        self.assertIn('register volatile u8 *b;', base)
        self.assertNotIn('__asm__ ("r1")', base.replace(match.group(0), ''))
        self.assertIn('b[22] = 16;', base)
        self.assertEqual(before, '')
        self.assertEqual(after.strip(), '')

    def test_statement_asm_still_encodes_span_plus_semicolon(self):
        base, _, _ = perm.prepare_source('void f(void) { __asm__("nop"); }', 'f')
        match = re.search(r'#pragma _permuter b64literal (\S+)', base)
        self.assertEqual(base64.b64decode(match.group(1)).decode(), '__asm__("nop");')

    def test_unknown_and_duplicate_body_rejected(self):
        for source in ('int x;', 'int f(void) {return 1;} int f(void) {return 2;}'):
            with self.assertRaisesRegex(ValueError, 'expected one definition'):
                perm.prepare_source(source, 'f')

    def test_nonempty_workspace_preserved(self):
        with tempfile.TemporaryDirectory() as tmp:
            work = Path(tmp)
            (work / 'base.c').write_text('manual edits')
            with self.assertRaisesRegex(ValueError, 'refusing to overwrite'):
                perm.prepare('f', work)
            self.assertEqual((work / 'base.c').read_text(), 'manual edits')

    def test_wrapper_options_after_function_and_fork_separator(self):
        with tempfile.TemporaryDirectory() as tmp:
            with patch.object(perm, 'prepare') as prepare, patch.object(perm.subprocess, 'run') as run:
                run.return_value.returncode = 17
                result = perm.main(['f', '--work-dir', tmp, '--', '--debug', '-j', '1'])
                self.assertEqual(result, 17)
                prepare.assert_called_once_with('f', Path(tmp).resolve())
                self.assertEqual(run.call_args.args[0][-4:], ['--stack-diffs', '--debug', '-j', '1'])
                self.assertEqual(run.call_args.kwargs['cwd'], Path(tmp).resolve())


@unittest.skipUnless(shutil.which('arm-none-eabi-objcopy'), 'optional ARM binutils unavailable')
class ObjectTests(unittest.TestCase):
    def test_real_vma_mapping_and_only_selected_bytes(self):
        with tempfile.TemporaryDirectory() as tmp:
            obj = Path(tmp) / 'target.o'
            blob = bytes.fromhex('0048704780170003')
            perm.diff_object(blob, 0x080026DC, [(0, '$t'), (4, '$d')], obj)
            with obj.open('rb') as stream:
                elf = ELFFile(stream)
                section = elf.get_section_by_name('.text')
                self.assertEqual(section['sh_addr'], 0x080026DC)
                self.assertEqual(section.data(), blob)
            listing = subprocess.check_output(['arm-none-eabi-objdump', '-drz', obj], text=True)
            self.assertIn('bx\tlr', listing)
            self.assertIn('.word\t0x03001780', listing)


@unittest.skipUnless(perm.probe.AGBCC.is_file() and shutil.which('arm-none-eabi-as')
                     and shutil.which('arm-none-eabi-objcopy'), 'optional agbcc toolchain unavailable')
class CompilerTests(unittest.TestCase):
    def test_candidate_changes_survive_and_unknown_calls_fail(self):
        with tempfile.TemporaryDirectory(prefix='permuter space ') as tmp:
            work = Path(tmp)
            (work / 'scratch').mkdir()
            (work / 'before.c').write_text('typedef unsigned int u32;\n')
            (work / 'after.c').write_text('\nu32 other(void) { return 99; }\n')
            meta = dict(body_name='target', vma=0x08004000, symbols={}, aliases={}, hints={},
                        flags=perm.FLAGS,
                        compiler_sha256=hashlib.sha256(perm.probe.AGBCC.read_bytes()).hexdigest(),
                        target_sha256='', target_modes=[])
            (work / 'metadata.json').write_text(json.dumps(meta))
            source, output = work / 'candidate.c', work / 'candidate.o'
            source.write_text('u32 target(void) { return 1; }')
            first = perm.compile_candidate(work, source, output)
            source.write_text('u32 target(void) { return 2; }')
            second = perm.compile_candidate(work, source, output)
            self.assertNotEqual(first, second)
            self.assertEqual(first[:2], b'\x01\x20')
            self.assertEqual(second[:2], b'\x02\x20')
            self.assertLessEqual(len(second), 4)  # unrelated other() is excluded
            source.write_text('u32 target(void) { extern u32 missing(void); return missing(); }')
            with self.assertRaisesRegex(ValueError, 'unresolved candidate relocation'):
                perm.compile_candidate(work, source, output)
            self.assertEqual(list((work / 'scratch').iterdir()), [])


if __name__ == '__main__':
    unittest.main()
