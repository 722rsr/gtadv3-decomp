#!/usr/bin/env python3
"""Build reviewed data regions from private, editable inputs and verify linkage."""
import argparse
import base64
import binascii
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CODE_END = 0x2E158
ROM_SIZE = 0x800000
MANIFEST = 'tools/data_regions.json'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def load_regions(root=ROOT):
    manifest = json.loads((root / MANIFEST).read_text())
    if manifest['version'] != 1:
        raise ValueError('unsupported data-region manifest version')
    regions = manifest['regions']
    end = CODE_END
    ids, paths = set(), set()
    for row in regions:
        start, stop = int(row['start'], 16), int(row['end'], 16)
        if not end <= start < stop <= ROM_SIZE:
            raise ValueError('data regions overlap, are unsorted, or extend outside the data tail')
        fmt = row.get('format')
        if fmt == 'variant-palette-v1':
            if stop - start != 7 * 40:
                raise ValueError('unsupported data format or size')
        elif fmt == 'mto-raw-v1':
            group, count = row.get('group'), row.get('count')
            if type(group) is not int or not 0 <= group <= 10:
                raise ValueError('mto-raw region needs an integer group 0..10')
            if type(count) is not int or count <= 0 or count > 256:
                raise ValueError('mto-raw region needs a positive record count')
            if stop - start <= 8 * count:
                raise ValueError('mto-raw region span is too small for its headers')
        elif fmt == 'raw-span-v1':
            if stop - start <= 0 or stop - start > ROM_SIZE:
                raise ValueError('raw-span region needs a positive span')
            if 'group' in row or 'count' in row:
                raise ValueError('raw-span region takes no group or count')
        else:
            raise ValueError('unsupported data format or size')
        if row['id'] in ids or not re.fullmatch(r'[a-z][a-z0-9_]*', row['id']):
            raise ValueError('invalid or duplicate data-region id')
        ids.add(row['id'])
        if not re.fullmatch(r'[0-9a-f]{64}', row['sha256']):
            raise ValueError('invalid data-region digest')
        for key in ('editable', 'binary'):
            path = Path(row[key])
            if path.parts[:2] != ('build', 'data') or '..' in path.parts or row[key] in paths:
                raise ValueError('data artifacts must have unique paths under build/data')
            paths.add(row[key])
        end = stop
    return regions


def reference(root=ROOT):
    rom = (root / 'baserom.gba').read_bytes()
    expected = (root / 'baserom.sha256').read_text().splitlines()[-1].split()[0]
    if len(rom) != ROM_SIZE or sha(rom) != expected:
        raise ValueError('reference ROM does not match baserom.sha256')
    return rom


def decode_region(blob):
    if len(blob) != 280:
        raise ValueError('variant palette family must contain seven 40-byte records')
    records = []
    for index in range(7):
        group, stored_index, size = struct.unpack_from('<HHI', blob, index * 40)
        if (group, stored_index, size) != (6, index, 32):
            raise ValueError('variant palette record header differs from reviewed layout')
        words = struct.unpack_from('<16H', blob, index * 40 + 8)
        records.append({'index': index, 'colors': [
            {'r5': word & 31, 'g5': (word >> 5) & 31,
             'b5': (word >> 10) & 31, 'bit15': word >> 15} for word in words]})
    return {'format': 'variant-palette-v1', 'records': records}


def encode_region(editable):
    if editable['format'] != 'variant-palette-v1' or len(editable['records']) != 7:
        raise ValueError('invalid editable palette family')
    result = bytearray()
    for index, record in enumerate(editable['records']):
        if record['index'] != index or len(record['colors']) != 16:
            raise ValueError('invalid palette record index or color count')
        result.extend(struct.pack('<HHI', 6, index, 32))
        for color in record['colors']:
            for key, maximum in (('r5', 31), ('g5', 31), ('b5', 31), ('bit15', 1)):
                if type(color[key]) is not int or not 0 <= color[key] <= maximum:
                    raise ValueError(f'invalid {key} value')
            word = color['r5'] | color['g5'] << 5 | color['b5'] << 10 | color['bit15'] << 15
            result.extend(struct.pack('<H', word))
    return bytes(result)


def decode_mto_raw(blob, group, count):
    """Split a raw MTO chain span into header-validated base64 payloads."""
    if type(group) is not int or type(count) is not int:
        raise ValueError('mto-raw decode needs an integer group and count')
    records = []
    offset = 0
    for index in range(count):
        if offset + 8 > len(blob):
            raise ValueError('mto-raw span is truncated at a record header')
        rec_group, rec_index, size = struct.unpack_from('<HHI', blob, offset)
        if (rec_group, rec_index) != (group, index):
            raise ValueError('mto-raw record header differs from reviewed layout')
        if size <= 0 or size > 0x20000 or offset + 8 + size > len(blob):
            raise ValueError('mto-raw record size is implausible or overruns the span')
        payload = blob[offset + 8:offset + 8 + size]
        records.append({'index': index, 'size': size,
                        'payload_b64': base64.b64encode(payload).decode('ascii')})
        offset += 8 + size
    if offset != len(blob):
        raise ValueError('mto-raw span has trailing bytes past its record chain')
    return {'format': 'mto-raw-v1', 'group': group, 'records': records}


def encode_mto_raw(editable):
    """Rebuild a raw MTO chain span from editable base64 payloads."""
    if editable.get('format') != 'mto-raw-v1':
        raise ValueError('invalid mto-raw family')
    group = editable.get('group')
    records = editable.get('records')
    if type(group) is not int or not 0 <= group <= 10:
        raise ValueError('invalid mto-raw group')
    if type(records) is not list or not records:
        raise ValueError('invalid mto-raw record list')
    result = bytearray()
    for index, record in enumerate(records):
        if record.get('index') != index:
            raise ValueError('invalid mto-raw record index or order')
        size = record.get('size')
        payload_b64 = record.get('payload_b64')
        if type(size) is not int or size <= 0 or size > 0x20000:
            raise ValueError('invalid mto-raw record size')
        if type(payload_b64) is not str:
            raise ValueError('invalid mto-raw payload')
        try:
            payload = base64.b64decode(payload_b64.encode('ascii'), validate=True)
        except (ValueError, binascii.Error, UnicodeEncodeError):
            raise ValueError('invalid mto-raw base64 payload')
        if len(payload) != size:
            raise ValueError('mto-raw payload length differs from its header size')
        result.extend(struct.pack('<HHI', group, index, size))
        result.extend(payload)
    return bytes(result)


def decode_raw_span(blob):
    return {'format': 'raw-span-v1', 'payload_b64': base64.b64encode(blob).decode('ascii')}


def encode_raw_span(editable):
    if editable.get('format') != 'raw-span-v1':
        raise ValueError('invalid raw-span family')
    payload_b64 = editable.get('payload_b64')
    if type(payload_b64) is not str:
        raise ValueError('invalid raw-span payload')
    try:
        return base64.b64decode(payload_b64.encode('ascii'), validate=True)
    except (ValueError, binascii.Error, UnicodeEncodeError):
        raise ValueError('invalid raw-span base64 payload')


def encode_for_row(editable, row):
    fmt = row.get('format')
    if fmt == 'variant-palette-v1':
        return encode_region(editable)
    if fmt == 'mto-raw-v1':
        data = encode_mto_raw(editable)
        if editable.get('group') != row.get('group') or len(editable.get('records', [])) != row.get('count'):
            raise ValueError('mto-raw editable group or count differs from the registry')
        return data
    if fmt == 'raw-span-v1':
        if set(editable.keys()) != {'format', 'payload_b64'}:
            raise ValueError('invalid raw-span editable')
        return encode_raw_span(editable)
    raise ValueError('unsupported data format')


def decode_for_row(blob, row):
    fmt = row.get('format')
    if fmt == 'variant-palette-v1':
        return decode_region(blob)
    if fmt == 'mto-raw-v1':
        return decode_mto_raw(blob, row['group'], row['count'])
    if fmt == 'raw-span-v1':
        return decode_raw_span(blob)
    raise ValueError('unsupported data format')


def extract(row, root=ROOT):
    target = root / row['editable']
    if target.exists():
        print(f'Keeping existing editable input: {row["editable"]}')
        return
    rom = reference(root)
    data = rom[int(row['start'], 16):int(row['end'], 16)]
    if sha(data) != row['sha256']:
        raise ValueError('selected data-region digest differs from manifest')
    # Cross-check the boundary against the existing resource-package reader.
    import track_dump
    _, _, records = track_dump.read_package(rom)
    if row.get('format') == 'variant-palette-v1':
        selected = [(offset, group, index, size) for offset, group, index, size, _ in records if group == 6]
        expected = [(int(row['start'], 16) + index * 40, 6, index, 32) for index in range(7)]
        if selected != expected:
            raise ValueError('resource-package inventory differs from selected palette region')
    elif row.get('format') == 'mto-raw-v1':
        selected = [(offset, group, index, size) for offset, group, index, size, _ in records
                    if group == row['group']]
        if len(selected) != row['count']:
            raise ValueError('resource-package inventory differs from selected raw region')
        start = int(row['start'], 16)
        cursor = start
        for offset, group, index, size in selected:
            if offset != cursor or group != row['group']:
                raise ValueError('resource-package inventory differs from selected raw region')
            cursor += 8 + size
        if cursor != int(row['end'], 16):
            raise ValueError('resource-package inventory differs from selected raw region')
    elif row.get('format') == 'raw-span-v1':
        pass
    else:
        raise ValueError('unsupported data format')
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(decode_for_row(data, row), indent=2) + '\n')


def generate(row, root=ROOT):
    # Intentionally no ROM read here: editable fields are the generator input.
    data = encode_for_row(json.loads((root / row['editable']).read_text()), row)
    if len(data) != int(row['end'], 16) - int(row['start'], 16) or sha(data) != row['sha256']:
        raise ValueError('generated data differs from pinned span; editable input preserved, output not replaced')
    target = root / row['binary']
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(data)


def verify_outputs(root=ROOT):
    regions = load_regions(root)
    for row in regions:
        data = (root / row['binary']).read_bytes()
        rebuilt = encode_for_row(json.loads((root / row['editable']).read_text()), row)
        if data != rebuilt or len(data) != int(row['end'], 16) - int(row['start'], 16) or sha(data) != row['sha256']:
            raise ValueError(f'data output is stale or mismatched: {row["id"]}')
    return regions


def check_object(root=ROOT):
    regions = verify_outputs(root)
    rom = reference(root)
    with tempfile.TemporaryDirectory() as temp:
        output = Path(temp) / 'data.bin'
        subprocess.run(['arm-none-eabi-objcopy', '-O', 'binary', str(root / 'build-code/data.o'), str(output)], check=True)
        data = output.read_bytes()
    if data != rom[CODE_END:]:
        raise ValueError('assembled data object differs from the complete reference tail')
    if (root / 'build/gtadv3.gba').read_bytes() != rom:
        raise ValueError('reference ROM build differs from the pinned ROM')
    print(f'data-check: {sum(int(r["end"],16)-int(r["start"],16) for r in regions)} generated bytes; complete data object and ROM byte-identical')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument('--extract', metavar='REGION')
    action.add_argument('--generate', metavar='REGION')
    action.add_argument('--check', action='store_true')
    args = parser.parse_args()
    try:
        if args.check:
            check_object()
        else:
            name = args.extract or args.generate
            row = next((r for r in load_regions() if r['id'] == name), None)
            if row is None:
                raise ValueError(f'unknown data region: {name}')
            (extract if args.extract else generate)(row)
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as exc:
        parser.exit(1, f'data_regions: {exc}\n')


if __name__ == '__main__':
    main()
