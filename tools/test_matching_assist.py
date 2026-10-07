#!/usr/bin/env python3
"""Regression tests for advisory matching tools (no ROM/toolchain required)."""
import json
from pathlib import Path
import struct
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import match_families as families
import leaf_synth as synth

BASE = families.probe.ROM_BASE


def blob(*halfwords):
    return struct.pack('<' + 'H' * len(halfwords), *halfwords)


class FamilyTests(unittest.TestCase):
    def test_pool_cannot_seed_thumb2_it_decoder_state(self):
        data = blob(0xBF82, 0x2001, 0x4770)
        image = families.instruction_image(data, {BASE+2: 2, BASE+4: 2})
        self.assertEqual(image, blob(0, 0x2001, 0x4770))

    def test_return_excludes_literal_pool_and_padding(self):
        rom = blob(0x4800, 0x4770, 0x1234, 0x0800, 0)
        seen, _, errors = families.flow(rom, BASE, BASE + len(rom))
        self.assertEqual(list(seen), [BASE, BASE + 2])
        self.assertFalse(errors)

    def test_branch_topology_not_just_opcode_histogram(self):
        def fp(branch):
            rom = blob(0x2800, branch, 0x2001, 0x4770)
            listing = {BASE: ('cmp', 'r0, #0'), BASE+2: ('beq', '8000006'),
                       BASE+4: ('movs', 'r0, #1'), BASE+6: ('bx', 'lr')}
            return families.fingerprint(rom, BASE, BASE + len(rom), listing)
        a, b = fp(0xD000), fp(0xD0FF)
        self.assertTrue(a['complete'])
        self.assertNotEqual(a['fingerprint'], b['fingerprint'])

    def test_constant_variants_group_but_widths_do_not(self):
        def fp(imm, mnemonic='ldrb'):
            return families.fingerprint(blob(0x7800, 0x4770), BASE, BASE + 4,
                                        {BASE: (mnemonic, f'r0, [r0, #{imm}]'), BASE+2: ('bx', 'lr')})
        self.assertEqual(fp(0)['fingerprint'], fp(4)['fingerprint'])
        self.assertNotEqual(fp(0)['fingerprint'], fp(0, 'ldrh')['fingerprint'])

    def test_unresolved_control_flow_fails_closed(self):
        for data in (blob(0x4700), blob(0xE7FC), blob(0xF000, 0), blob(0xDF00)):
            self.assertTrue(families.flow(data, BASE, BASE + len(data))[2])

    def test_interworking_return_and_call(self):
        rom = blob(0xB500, 0xF000, 0xF800, 0xBC01, 0x4700)
        seen, _, errors = families.flow(rom, BASE, BASE+len(rom))
        self.assertFalse(errors)
        self.assertEqual(seen[BASE+2][1:], (4, 'call'))
        self.assertEqual(seen[BASE+8][2], 'return')

    def test_pool_code_overlap_rejected(self):
        self.assertTrue(families.flow(blob(0x4800, 0x2000, 0x4770, 0), BASE, BASE+8)[2])

    def test_stale_rom_or_report_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            rom, report, index = [root / n for n in ('rom', 'report', 'index')]
            rom.write_bytes(b'rom')
            report.write_text('{}')
            data = {'version': families.VERSION, 'rom_sha256': families.digest(rom),
                    'inventory_sha256': families.inventory_digest(),
                    'report': str(report), 'report_sha256': families.digest(report), 'records': []}
            index.write_text(json.dumps(data))
            with patch.object(families.probe, 'ROM', rom):
                self.assertEqual(families.load_index(index), data)
                report.write_text('{"changed": true}')
                with self.assertRaisesRegex(ValueError, 'report changed'):
                    families.load_index(index)
                rom.write_bytes(b'changed')
                with self.assertRaisesRegex(ValueError, 'ROM/schema'):
                    families.load_index(index)

    def test_nearest_deduplicates_body_and_rejects_incomplete(self):
        base = {'source': 'src/a.c', 'name': 'a', 'alias_of': None, 'vma': hex(BASE),
                'complete': True, 'tokens': ['mov', 'return'], 'fingerprint': 'a',
                'status': 'EXACT', 'promoted': False}
        sibling = dict(base, name='b', vma=hex(BASE+4))
        alias = dict(sibling, name='alias', alias_of='b')
        incomplete = dict(base, name='c', vma=hex(BASE+8), complete=False)
        result = families.nearest({'records': [base, sibling, alias, incomplete]}, base, recipes=True)
        self.assertEqual([r['name'] for r in result], ['b'])

    def test_recipe_never_reuses_a_failed_or_changed_probe(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / 'example.c'
            source.write_text('unsigned example(void) { return 7; }')
            target = {'source': 'target.c', 'name': 'target', 'vma': hex(BASE),
                      'complete': True, 'tokens': ['mov', 'return'], 'fingerprint': 'a'}
            rec = dict(target, source=str(source), name='example', vma=hex(BASE+4),
                       status='EXACT', promoted=False, parameters=[])
            index = {'records': [rec]}
            for status, code, mutate, accepted in [('EXACT', 0, False, True),
                                                   ('PARTIAL', 0, False, False),
                                                   ('EXACT', 1, False, False),
                                                   ('EXACT', 0, True, False)]:
                def run(command, **kwargs):
                    report = Path(command[command.index('--json')+1])
                    report.write_text(json.dumps({'results': [dict(rec, status=status, rom_bytes=4)]}))
                    if mutate:
                        source.write_text(source.read_text() + '\n/* changed during probe */')
                    return SimpleNamespace(returncode=code, stdout='', stderr='')
                with patch.object(families.subprocess, 'run', side_effect=run):
                    families.verified_recipes(index, target, root, 1)
                result = json.loads((root / 'recipe-verification.json').read_text())
                self.assertEqual(result[0]['accepted'], accepted)

    def test_instruction_set_mode_boundary(self):
        modes = [(BASE, 'a'), (BASE+4, 't'), (BASE+8, 'd')]
        self.assertEqual(families.mode_at(modes, BASE+6), 't')
        self.assertEqual(families.mode_at(modes, BASE+8), 'd')


class SynthTests(unittest.TestCase):
    def lift(self, *h, memory='ordinary'):
        data = blob(*h)
        return synth.lift(data, BASE, BASE + len(data), memory)

    def test_constants_and_parameter_reads(self):
        model = self.lift(0x2007, 0x4770)
        self.assertFalse(model.inputs)
        self.assertEqual(model.nodes[0].expression, '7u')
        self.assertIn('candidate(void)', next(synth.variants(model, 'u32')))
        model = self.lift(0x3103, 0x1C08, 0x4770)
        self.assertEqual(model.inputs, {1})
        self.assertIn('u32 p0, u32 p1', next(synth.variants(model, 'u32')))

    def test_literal_pool_value_and_width(self):
        model = self.lift(0x4800, 0x4770, 0x5678, 0x1234)
        self.assertEqual(model.nodes[0].expression, '0x12345678u')
        for op, ty in ((0x6800, 'u32'), (0x8800, 'u16'), (0x7800, 'u8')):
            self.assertIn(f'*({ty} *)', self.lift(op, 0x4770).nodes[0].expression)
        self.assertIn('*(volatile u8 *)', self.lift(0x7800, 0x4770, memory='volatile').nodes[0].expression)

    def test_signed_loads_and_shift_32(self):
        self.assertIn('*(s8 *)', self.lift(0x5608, 0x4770).nodes[0].expression)
        self.assertIn('*(s16 *)', self.lift(0x5E08, 0x4770).nodes[0].expression)
        self.assertEqual(self.lift(0x0800, 0x4770).nodes[0].expression, '0u')
        self.assertIn('>> 31', self.lift(0x1000, 0x4770).nodes[0].expression)

    def test_store_width_and_order_preserved(self):
        model = self.lift(0x7808, 0x7008, 0x4770, memory='volatile')
        for source in synth.variants(model, 'void'):
            self.assertEqual(source.count('volatile u8'), 2)
            self.assertLess(source.index('(u32)(*'), source.index('= (u8)'))

    def test_duplicate_volatile_load_not_inlined(self):
        model = self.lift(0x7800, 0x0040, 0x4770, memory='volatile')
        for source in synth.variants(model, 'u32'):
            self.assertEqual(source.count('volatile u8'), 1)

    def test_reject_unsupported_abi_and_control_flow(self):
        for data in ((0xB500, 0x4770), (0xD000, 0x4770), (0xF000, 0xF800),
                     (0x2401, 0x4770), (0x4620, 0x4770), (0xDF00, 0x4770)):
            with self.assertRaises(synth.Unsupported):
                self.lift(*data)

    def test_budget_and_determinism(self):
        model = self.lift(0x3001, 0x4770)
        a = list(synth.variants(model, 'u32', 2))
        self.assertEqual(len(a), 2)
        self.assertEqual(a, list(synth.variants(model, 'u32', 2)))
        self.assertEqual(len(a), len(set(a)))

    def test_arm_bytes_are_never_synthesized_as_thumb(self):
        with self.assertRaisesRegex(synth.Unsupported, 'ARM/data'):
            synth.lift(blob(0x2001, 0x4770), BASE, BASE+4, modes=[(BASE, 'a')])

    def test_unsupported_rerun_clears_old_winner(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            rom = root / 'rom'
            rom.write_bytes(blob(0xB500, 0x4770))
            (root / 'winner.c').write_text('old winner')
            def run(command, **kwargs):
                (root / 'baseline.json').write_text(json.dumps({'results': [{'vma': hex(BASE), 'rom_bytes': 4}]}))
                return SimpleNamespace(returncode=0, stdout='', stderr='')
            with patch.object(synth.subprocess, 'run', side_effect=run), \
                 patch.object(synth.probe, 'ROM', rom), patch.object(synth.probe, 'AGBCC', rom), \
                 patch.object(synth, 'assembled_modes', return_value=[(BASE, 't')]):
                self.assertEqual(synth.synthesize('example', root, 'void', 'ordinary', 1), 2)
            self.assertFalse((root / 'winner.c').exists())
            self.assertEqual(json.loads((root / 'summary.json').read_text())['status'], 'UNSUPPORTED')


if __name__ == '__main__':
    unittest.main()
