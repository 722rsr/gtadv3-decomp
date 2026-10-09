#!/usr/bin/env python3
"""Pure tests by default; FUNCTION_TRACE_INTEGRATION=1 adds real mGBA runs."""
import argparse
import json
import os
import shutil
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from types import SimpleNamespace

import function_trace as ft
import ramwatch as rw

ROOT = Path(__file__).resolve().parents[1]


class PairTests(unittest.TestCase):
    def call(self, ident, sp=0x03007f00, lr=0x08000101):
        return dict(id=ident, sp=sp, lr=lr, cpu_mode=31)

    def test_recursion_and_mode_guards(self):
        stack = [self.call(1), self.call(2, sp=0x03007efc)]
        regs = [0] * 16
        regs[13] = 0x03007f00
        self.assertEqual(ft.pair_returns(stack, 0x08000100, regs, 63), [])
        regs[13] -= 4
        self.assertEqual(ft.pair_returns(stack, 0x08000100, regs, 31), [])
        self.assertEqual(ft.pair_returns(stack, 0x08000100, regs, 50), [])
        self.assertEqual([c['id'] for c in ft.pair_returns(stack, 0x08000100, regs, 63)], [2])
        regs[13] += 4
        self.assertEqual([c['id'] for c in ft.pair_returns(stack, 0x08000100, regs, 63)], [1])

    def test_arm_and_shared_tail_continuation(self):
        stack = [self.call(1, lr=0x08000100), self.call(2, lr=0x08000100)]
        regs = [0] * 16
        regs[13] = 0x03007f00
        self.assertEqual(len(ft.pair_returns(stack, 0x08000100, regs, 31)), 2)

    def test_regions_exclude_io_and_cross_boundary(self):
        self.assertEqual(ft.region_spec('PAL'), ('PAL', 0x05000000, 1024))
        self.assertEqual(ft.region_spec('state:0x02000004:4'), ('state', 0x02000004, 4))
        for text in ('io:0x04000000:4', 'bad:0x03007fff:2', 'bad:0x02000000:0'):
            with self.assertRaises(argparse.ArgumentTypeError):
                ft.region_spec(text)

    def test_collector_versions_fragmented(self):
        for regions in (rw.REGIONS_V2[:2], rw.REGIONS_V2):
            payload = b''.join(bytes([i + 1]) * size for i, (_, size, _) in enumerate(regions))
            packet = struct.pack('<5I', rw.MAGIC, 7, len(payload) + 8, 0x8000, 0x40000) + payload
            chunks = iter([packet[:13], packet[13:200], packet[200:], b''])
            request = SimpleNamespace(recv=lambda _: next(chunks))
            with tempfile.TemporaryDirectory() as directory:
                rw._Handler(request, None, SimpleNamespace(outdir=Path(directory)))
                self.assertEqual((Path(directory) / 'snap_00000007.bin').read_bytes(), payload)
                meta = json.loads((Path(directory) / 'capture.json').read_text())
                self.assertEqual(len(meta['regions']), len(regions))

    @unittest.skipUnless(shutil.which('lua'), 'Lua interpreter unavailable')
    def test_lua_payload_and_initial_release(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            script = rw.LUA_TEMPLATE % dict(port=46001, every=2, stopat=3,
                                            inputs='{[0]=8,[1]=0}', magic=rw.MAGIC)
            prelude = '''
local output = assert(io.open(arg[1], 'wb'))
socket = {connect=function() return {send=function(_, data) output:write(data) end} end}
console = {log=function() end, error=error}
local current = 0
local keylog = {}
emu = {readRange=function(_, address, size) return string.rep(string.char(math.floor(address / 0x1000000)), size) end,
       setKeys=function(_, mask) table.insert(keylog, mask) end,
       currentFrame=function() return current end}
local callback
callbacks = {add=function(_, name, fn) callback=fn end}
'''
            ending = '''
for frame=1,4 do current=frame; callback() end
assert(keylog[1] == 8 and keylog[2] == 0)
output:close()
'''
            (path / 'run.lua').write_text(prelude + script + ending)
            subprocess.run(['lua', str(path/'run.lua'), str(path/'packets')], check=True)
            data = (path/'packets').read_bytes()
            size = sum(s for _, s, _ in rw.REGIONS_V2)
            expected = b''.join(bytes([b >> 24]) * s for _, s, b in rw.REGIONS_V2)
            for index, frame in enumerate((2, 3)):
                offset = index * (20 + size)
                self.assertEqual(struct.unpack_from('<5I', data, offset),
                                 (rw.MAGIC, frame, size + 8, 0x8000, 0x40000))
                self.assertEqual(data[offset+20:offset+20+size], expected)
            self.assertEqual(len(data), 2 * (20 + size))


@unittest.skipUnless(os.environ.get('FUNCTION_TRACE_INTEGRATION') == '1',
                     'set FUNCTION_TRACE_INTEGRATION=1 with mGBA bindings')
class IntegrationTests(unittest.TestCase):
    def test_recursive_capture_repeat_and_negative_control(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = (ROOT / 'tools/fixtures/function_trace/recursive.s').read_text()
            inputs = root / 'inputs.csv'
            inputs.write_text('frame,keymask\n0,0\n')
            def trace(tag, text, expected_rc=0):
                asm, obj, elf, rom = [root / (tag + suffix) for suffix in ('.s', '.o', '.elf', '.gba')]
                asm.write_text(text)
                for cmd in (
                    ['arm-none-eabi-as', '-o', str(obj), str(asm)],
                    ['arm-none-eabi-ld', '-Ttext=0x08000000', '-o', str(elf), str(obj)],
                    ['arm-none-eabi-objcopy', '-O', 'binary', str(elf), str(rom)],
                ):
                    subprocess.run(cmd, check=True, capture_output=True)
                symbols = subprocess.check_output(['arm-none-eabi-nm', str(elf)], text=True)
                entry = next(line.split()[0] for line in symbols.splitlines() if line.endswith(' candidate'))
                out = root / (tag + '.jsonl')
                completed = subprocess.run([sys.executable, str(ROOT/'tools/function_trace.py'), str(rom), str(inputs),
                                '--entry', '0x'+entry, '--calls', '3', '--frames', '2',
                                '--region', 'result:0x02000000:8', '--out', str(out)],
                               capture_output=True, text=True)
                self.assertEqual(completed.returncode, expected_rc, completed.stderr)
                records = [json.loads(line) for line in out.read_text().splitlines()]
                return records
            a = trace('a', source)
            b = trace('b', source)
            self.assertEqual(a, b)
            events = [r for r in a if r['event'] in ('entry', 'return')]
            self.assertEqual([r['call_id'] for r in events], [1, 2, 3, 3, 2, 1])
            self.assertEqual([r['registers'][0] for r in events[3:]], [1, 2, 3])
            self.assertEqual([int.from_bytes(bytes.fromhex(r['memory'][0]['hex'])[4:], 'little')
                              for r in events], [0, 1, 2, 3, 3, 3])
            self.assertTrue(all(r['elapsed_cycles'] > 0 for r in events[3:]))
            self.assertEqual(a[-1]['incomplete'], 0)
            changed = trace('changed', source.replace('adds r0, #1', 'adds r0, #2'))
            self.assertEqual([r['registers'][0] for r in changed if r['event'] == 'return'], [1, 3, 5])
            incomplete = trace('incomplete', source.replace('bx r1', 'b .'), expected_rc=1)
            self.assertEqual(incomplete[-1]['incomplete'], 3)
            self.assertEqual(len([r for r in incomplete if r['event'] == 'incomplete']), 3)

            # Headless frame captures name the actual terminal frame and declare v2.
            caps = root / 'caps'
            subprocess.run([sys.executable, str(ROOT/'tools/ramwatch.py'), 'run',
                            str(root/'a.gba'), str(inputs), '--frames', '2',
                            '--interval', '0', '--outdir', str(caps)], check=True, capture_output=True)
            self.assertEqual([p.name for p in caps.glob('snap_*.bin')], ['snap_00000002.bin'])
            snap = (caps/'snap_00000002.bin').read_bytes()
            self.assertEqual(len(snap), sum(s for _, s, _ in rw.REGIONS_V2))
            self.assertEqual(struct.unpack_from('<2I', snap, 0x8000), (3, 3))
            self.assertEqual(len(json.loads((caps/'capture.json').read_text())['regions']), 5)

            # A missed target is a failed capture, not a successful empty trace.
            empty = root / 'empty.jsonl'
            missed = subprocess.run([sys.executable, str(ROOT/'tools/function_trace.py'),
                                     str(root/'a.gba'), str(inputs), '--entry', '0x08001000',
                                     '--frames', '1', '--out', str(empty)], capture_output=True)
            self.assertEqual(missed.returncode, 1)
            self.assertEqual(json.loads(empty.read_text().splitlines()[-1])['returns'], 0)


if __name__ == '__main__':
    unittest.main()
