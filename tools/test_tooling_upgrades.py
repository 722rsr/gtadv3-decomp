#!/usr/bin/env python3
"""Operand diagnostics, isolated full-TU edits, and persistent campaign regressions."""
import argparse
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import corpus_match_probe as probe
import instruction_diff as diff
import matching_campaign as campaign


class Diagnostics(unittest.TestCase):
    def compare(self, a, b):
        vma = 0x08004000
        with tempfile.TemporaryDirectory() as tmp, patch.object(probe, 'WORK', Path(tmp)):
            return diff.analyze(a, b, vma, probe.objdump_text(a, vma, 'a'),
                                probe.objdump_text(b, vma, 'b'))

    def test_register_only_proof_and_immediate_negative(self):
        a = bytes.fromhex('01207047')
        r = self.compare(a, bytes.fromhex('01217047'))
        self.assertTrue(r['register_only'])
        json.dumps(r)  # reports must serialize without binary objects
        changed = self.compare(a, bytes.fromhex('02207047'))
        self.assertFalse(changed['register_only'])
        self.assertEqual(changed['class'], 'immediate or memory offset')

    def test_pool_is_data_and_values_are_checked(self):
        result = self.compare(bytes.fromhex('0048704780170003'), bytes.fromhex('0048704784170003'))
        self.assertEqual(result['reference_instructions'], 2)
        self.assertEqual(result['class'], 'literal pool value')
        self.assertFalse(result['register_only'])

    def test_inserted_instruction_does_not_hide_matching_tail(self):
        r = self.compare(bytes.fromhex('01207047'), bytes.fromhex('c04601207047'))
        self.assertEqual(r['agreeing_instructions'], 2)
        self.assertEqual(r['candidate_instructions'], 3)
        self.assertTrue(r['edits'])

    def test_width_and_padding_are_not_register_changes(self):
        width = self.compare(bytes.fromhex('00687047'), bytes.fromhex('00887047'))
        self.assertFalse(width['register_only'])
        self.assertTrue(width['edits'])
        padding = self.compare(bytes.fromhex('012070470000'), bytes.fromhex('012170470100'))
        self.assertFalse(padding['register_only'])
        self.assertEqual(padding['first_noncode_difference'], 4)

    def test_changed_call_target(self):
        a = probe.encode_thumb_bl(0x08004000, 0x08006c10) + bytes.fromhex('7047')
        b = probe.encode_thumb_bl(0x08004000, 0x08006c60) + bytes.fromhex('7047')
        r = self.compare(a, b)
        self.assertEqual(r['class'], 'control-flow target')

    def test_equal_pool_values_at_different_slots_are_not_register_only(self):
        a = bytes.fromhex('004870470100000001000000')
        b = bytes.fromhex('014970470100000001000000')
        r = self.compare(a, b)
        self.assertFalse(r['register_only'])
        self.assertEqual(r['class'], 'literal pool placement')

    def test_recursive_call_keeps_absolute_entry_target_after_insertion(self):
        vma = 0x08004000
        a = bytes.fromhex('0120') + probe.encode_thumb_bl(vma+2, vma) + bytes.fromhex('7047')
        b = bytes.fromhex('c0460120') + probe.encode_thumb_bl(vma+4, vma) + bytes.fromhex('7047')
        r = self.compare(a, b)
        self.assertFalse(any('call-target' in d['kinds'] for d in r['differences']))


class CampaignUnit(unittest.TestCase):
    def test_edits_are_confined_to_body_and_never_ambiguous(self):
        source = 'int other(void){return 1;}\nint target(void){return 1;}\n'
        edited = campaign.apply_edits(source, 'target', [{'old': 'return 1;', 'new': 'return 2;'}])
        self.assertEqual(edited, source.replace('target(void){return 1;', 'target(void){return 2;'))
        with self.assertRaises(ValueError):
            campaign.apply_edits(source, 'target', [{'old': 'absent', 'new': ''}])

    def test_matrix_is_reproducible_and_baseline_is_first(self):
        spec = {'matrix': [{'name': 'x', 'old': 'X', 'choices': ['A', 'B']},
                           {'name': 'y', 'old': 'Y', 'choices': ['C', 'D']}]}
        rows = list(campaign.cases(spec))
        self.assertEqual(len(rows), 5)
        self.assertEqual(rows[0], ('baseline', []))
        self.assertEqual(rows, list(campaign.cases(spec)))

    def test_exactness_outranks_any_diagnostic_score(self):
        exact = {'status': 'EXACT', 'cand_bytes': 4, 'rom_bytes': 4, 'matched_bytes': 4}
        partial = dict(exact, status='PARTIAL', instruction_diff={'agreeing_instructions': 1000})
        self.assertGreater(campaign.rank(exact), campaign.rank(partial))


@unittest.skipUnless(probe.AGBCC.is_file() and probe.ROM.is_file(), 'matching toolchain/ROM absent')
class CampaignIntegration(unittest.TestCase):
    def test_full_tu_baseline_resume_and_source_preservation(self):
        with tempfile.TemporaryDirectory() as tmp:
            args = argparse.Namespace(function='_080013B68', out=Path(tmp), budget=2, seconds=60,
                                      max_variants=8, cases=None, mutate=[])
            paths = [r[0] for r in probe.src_functions(filter_wanted=[args.function])]
            before = {p: p.read_bytes() for p in paths}
            first = campaign.run(args)
            self.assertEqual(first['best']['record']['status'], 'EXACT')
            self.assertEqual(first['compiles'], 1)
            second = campaign.run(args)
            self.assertEqual(second['compiles'], 0)
            self.assertEqual(second['reused'], 1)
            self.assertEqual({p: p.read_bytes() for p in paths}, before)
            self.assertEqual(len((Path(tmp)/'ledger.jsonl').read_text().splitlines()), 1)

    def test_duplicate_outputs_failed_hypotheses_and_matrix_resume(self):
        with tempfile.TemporaryDirectory() as tmp:
            out, spec = Path(tmp)/'campaign', Path(tmp)/'cases.json'
            spec.write_text(json.dumps({'variants': [
                {'name': 'comment', 'edits': [{'old': '(loop_rec + 174)',
                                              'new': '(loop_rec + 174) /* unchanged */'}]},
                {'name': 'invalid', 'edits': [{'old': '(loop_rec + 174)', 'new': '(???)'}]}],
                'matrix': [{'name': 'spelling', 'old': '(loop_rec + 174)',
                            'choices': ['(loop_rec + 174)', '(loop_rec + 174u)']}]}))
            args = argparse.Namespace(function='_080013B68', out=out, budget=8, seconds=60,
                                      max_variants=8, cases=spec, mutate=[])
            first = campaign.run(args)
            self.assertGreaterEqual(first['duplicate_outputs'], 1)
            records = [json.loads(s) for s in (out/'ledger.jsonl').read_text().splitlines()]
            self.assertTrue(any(r['record']['status'] == 'COMPILE_ERROR' for r in records))
            second = campaign.run(args)
            self.assertEqual(second['compiles'], 0)
            self.assertEqual(second['reused'], 5)
            self.assertEqual(len((out/'ledger.jsonl').read_text().splitlines()), len(records))

    def test_worker_timeout_stops_the_child_group(self):
        with tempfile.TemporaryDirectory() as tmp:
            job = Path(tmp)/'job.json'
            job.write_text('{}')
            # Even a worker that cannot finish starting must be terminated and reaped.
            with self.assertRaises(__import__('subprocess').TimeoutExpired):
                campaign.bounded_worker(job, 0.000001)


if __name__ == '__main__':
    unittest.main()
