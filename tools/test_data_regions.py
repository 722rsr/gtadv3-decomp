#!/usr/bin/env python3
"""ROM-free regression checks for generated data and progress credit."""
import base64
import copy
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import data_regions as data
import decomp_report as report


def palette_fixture():
    return b''.join(struct.pack('<HHI16H', 6, index, 32,
                                *[(0x8000 if word % 2 else 0) | (index << 10) | word
                                  for word in range(16)]) for index in range(7))


def raw_fixture(group=0, sizes=(5, 3)):
    blob = bytearray()
    for index, size in enumerate(sizes):
        blob.extend(struct.pack('<HHI', group, index, size))
        blob.extend(bytes((index * 7 + k) % 256 for k in range(size)))
    return bytes(blob)


class DataTests(unittest.TestCase):
    def test_roundtrip_preserves_header_channels_and_bit15(self):
        original = palette_fixture()
        editable = data.decode_region(original)
        self.assertEqual(editable['records'][0]['colors'][1]['bit15'], 1)
        self.assertEqual(data.encode_region(editable), original)
        for key, value in [('r5', 32), ('bit15', 2), ('g5', -1)]:
            changed = copy.deepcopy(editable)
            changed['records'][0]['colors'][0][key] = value
            with self.assertRaises(ValueError):
                data.encode_region(changed)
        with self.assertRaises(ValueError):
            data.decode_region(original[:-1])
        wrong_header = bytearray(original)
        wrong_header[0] = 5
        with self.assertRaises(ValueError):
            data.decode_region(wrong_header)

    def test_private_generation_and_negative_control(self):
        original = palette_fixture()
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            row = {'id': 'variant_palette', 'name': 'Fixture', 'format': 'variant-palette-v1',
                   'start': '0x25A118', 'end': '0x25A230', 'sha256': data.sha(original),
                   'editable': 'build/data/variant_palette.json', 'binary': 'build/data/variant_palette.bin'}
            (root / 'tools').mkdir()
            (root / data.MANIFEST).write_text(json.dumps({'version': 1, 'regions': [row]}))
            editable = root / row['editable']
            editable.parent.mkdir(parents=True)
            editable.write_text(json.dumps(data.decode_region(original)))
            # Generation succeeds without baserom.gba; it reads structured fields.
            data.generate(row, root)
            self.assertEqual((root / row['binary']).read_bytes(), original)
            data.verify_outputs(root)
            changed = json.loads(editable.read_text())
            changed['records'][0]['colors'][0]['bit15'] ^= 1
            editable.write_text(json.dumps(changed))
            with patch.object(data, 'reference', side_effect=AssertionError('must not overwrite editable input')):
                data.extract(row, root)
            with self.assertRaisesRegex(ValueError, 'differs from pinned'):
                data.generate(row, root)
            self.assertEqual(json.loads(editable.read_text()), changed)
            self.assertEqual((root / row['binary']).read_bytes(), original)
            with self.assertRaisesRegex(ValueError, 'stale or mismatched'):
                data.verify_outputs(root)
            editable.write_text(json.dumps(data.decode_region(original)))
            (root / row['binary']).write_bytes(original[:-1])
            with self.assertRaises(ValueError):
                data.verify_outputs(root)
            duplicate = {'version': 1, 'regions': [row, row]}
            (root / data.MANIFEST).write_text(json.dumps(duplicate))
            with self.assertRaises(ValueError):
                data.load_regions(root)

    def test_mto_raw_roundtrip_and_negative_controls(self):
        original = raw_fixture(group=0, sizes=(5, 3, 9))
        editable = data.decode_mto_raw(original, 0, 3)
        self.assertEqual(editable['group'], 0)
        self.assertEqual([r['size'] for r in editable['records']], [5, 3, 9])
        self.assertEqual(data.encode_mto_raw(editable), original)
        # Dispatchers agree with the direct codecs.
        row = {'id': 'mto_group0', 'format': 'mto-raw-v1', 'group': 0, 'count': 3}
        self.assertEqual(data.encode_for_row(editable, row), original)
        self.assertEqual(data.decode_for_row(original, row), editable)
        # Wrong group, wrong order, truncated payload, and bad base64 are refused.
        with self.assertRaises(ValueError):
            data.decode_mto_raw(original, 1, 3)
        bad = bytearray(original)
        bad[0] = 1
        with self.assertRaises(ValueError):
            data.decode_mto_raw(bytes(bad), 0, 3)
        with self.assertRaises(ValueError):
            data.decode_mto_raw(original[:-1], 0, 3)
        with self.assertRaises(ValueError):
            data.decode_mto_raw(original + b'\x00', 0, 3)
        changed = copy.deepcopy(editable)
        changed['records'][1]['payload_b64'] = base64.b64encode(b'toolong!').decode('ascii')
        with self.assertRaises(ValueError):
            data.encode_mto_raw(changed)
        changed = copy.deepcopy(editable)
        changed['records'][0]['payload_b64'] = '!!!not-base64!!!'
        with self.assertRaises(ValueError):
            data.encode_mto_raw(changed)
        changed = copy.deepcopy(editable)
        changed['records'][1]['index'] = 5
        with self.assertRaises(ValueError):
            data.encode_mto_raw(changed)
        changed = copy.deepcopy(editable)
        changed['group'] = 1
        with self.assertRaises(ValueError):
            data.encode_for_row(changed, row)

    def test_raw_span_roundtrip_and_negative_controls(self):
        original = bytes(range(256))
        editable = data.decode_raw_span(original)
        self.assertEqual(data.encode_raw_span(editable), original)
        row = {'id': 'span', 'format': 'raw-span-v1'}
        self.assertEqual(data.encode_for_row(editable, row), original)
        self.assertEqual(data.decode_for_row(original, row), editable)
        with self.assertRaises(ValueError):
            data.encode_raw_span({'format': 'raw-span-v1', 'payload_b64': '!!!'})
        with self.assertRaises(ValueError):
            data.encode_for_row({'format': 'raw-span-v1', 'payload_b64': 'xx', 'extra': 1}, row)

    def test_mto_raw_manifest_validation(self):
        base = {'id': 'mto_group0', 'name': 'Fixture', 'format': 'mto-raw-v1',
                'start': '0xCE438', 'end': '0xCE448', 'group': 0, 'count': 1,
                'sha256': '0' * 64, 'editable': 'build/data/a.json', 'binary': 'build/data/a.bin'}
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'tools').mkdir()
            (root / data.MANIFEST).write_text(json.dumps({'version': 1, 'regions': [base]}))
            self.assertEqual(len(data.load_regions(root)), 1)
            for key, value in [('format', 'unknown-v1'), ('group', 11), ('count', 0),
                               ('group', '0'), ('start', '0x16BA2C')]:
                bad = dict(base)
                if key == 'start':
                    bad['start'], bad['end'] = '0x16BA2C', '0xCE438'
                else:
                    bad[key] = value
                (root / data.MANIFEST).write_text(json.dumps({'version': 1, 'regions': [bad]}))
                with self.assertRaises(ValueError):
                    data.load_regions(root)
            # Overlapping ownership is refused even when each span is well-formed.
            first = dict(base, id='first', end='0x16BA2C',
                         editable='build/data/first.json', binary='build/data/first.bin')
            second = {'id': 'second', 'name': 'Second', 'format': 'variant-palette-v1',
                      'start': '0x16BA00', 'end': '0x25A230', 'sha256': '0' * 64,
                      'editable': 'build/data/b.json', 'binary': 'build/data/c.bin'}
            (root / data.MANIFEST).write_text(json.dumps({'version': 1, 'regions': [first, second]}))
            with self.assertRaises(ValueError):
                data.load_regions(root)

    def test_only_verified_registered_data_gets_credit(self):
        start = report.CODE_SIZE
        row = {'id': 'palette', 'name': 'Palette', 'start': hex(start),
               'end': hex(start + 280), 'binary': 'build/data/palette.bin'}
        owned = {'start': hex(start), 'end': hex(start + 280), 'size': 280,
                 'executable': False, 'classification': 'generated-data',
                 'status': 'verified-generated-data', 'backing': row['binary'],
                 'intended_owner': 'palette'}
        inventory = {'segments': [
            {'start': '0x0', 'end': hex(start), 'size': start,
             'executable': True, 'source_owners': ['asm/a.s']}, owned,
            {'start': hex(start + 280), 'end': hex(report.ROM_SIZE),
             'size': report.ROM_SIZE - start - 280, 'executable': False},
        ]}
        result = report.make_report({}, inventory, [row])
        self.assertEqual(result['measures']['matched_data'], '280')
        self.assertEqual(result['measures']['matched_code'], '0')
        self.assertEqual(sum(int(u['measures']['total_data']) for u in result['units']),
                         report.ROM_SIZE - report.CODE_SIZE)
        self.assertAlmostEqual(result['measures']['matched_data_percent'],
                               100 * 280 / (report.ROM_SIZE - report.CODE_SIZE))
        with self.assertRaises(ValueError):
            report.make_report({}, inventory)
        for key, value in [('status', 'unverified'), ('backing', 'baserom.gba'),
                           ('classification', 'asset-data')]:
            changed = copy.deepcopy(inventory)
            changed['segments'][1][key] = value
            with self.assertRaises(ValueError):
                report.make_report({}, changed, [row])


if __name__ == '__main__':
    unittest.main()
