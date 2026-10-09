#!/usr/bin/env python3
"""Bounded, headless GBA entry/return captures using mGBA 0.10.5's debugger.

Outputs private JSONL evidence, never modifies ROM/source or promotes matches.
See docs/function_captures.md for timing, coverage and pairing limitations.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import time

from ramwatch import REGIONS_V2, load_inputs


def region_spec(text):
    """Only canonical, side-effect-free RAM ranges; exclude IO and mirrors."""
    for name, size, base in REGIONS_V2:
        if text.upper() == name:
            return (name, base, size)
    try:
        name, address, length = text.split(':')
        address, length = int(address, 0), int(length, 0)
        if not name or length <= 0:
            raise ValueError()
        if not any(base <= address and address + length <= base + size
                   for _, size, base in REGIONS_V2):
            raise ValueError()
        return name, address, length
    except ValueError:
        raise argparse.ArgumentTypeError(
            'region must be IWRAM/EWRAM/VRAM/PAL/OAM or NAME:ADDRESS:SIZE '
            'wholly inside one canonical RAM region') from None


def pair_returns(stack, address, regs, cpsr):
    """LIFO return continuation + restored SP + CPU mode/state guard.

    Multiple tail-entered calls can share the same continuation and SP.
    Do not search past an unmatched inner call: nonlocal unwinds are unresolved.
    """
    paired = []
    while stack:
        call = stack[-1]
        lr = call['lr']
        if (address != (lr & ~1) or regs[13] != call['sp']
                or bool(cpsr & 32) != bool(lr & 1)
                or (cpsr & 31) != call['cpu_mode']):
            break
        paired.append(stack.pop())
    return paired


class Capture:
    def __init__(self, core, ffi, lib, args, emit):
        self.core, self.ffi, self.lib = core, ffi, lib
        self.args, self.emit = args, emit
        self.stack = []
        self.points = {}
        self.entries = self.returns = 0
        self.error = None
        self.debug = ffi.new('struct mDebugger *')
        self.callback = ffi.callback(
            'void(struct mDebugger *, enum mDebuggerEntryReason, struct mDebuggerEntryInfo *)',
            self.entered)
        self.debug.entered = self.callback
        self.debug.type = lib.DEBUGGER_CUSTOM
        lib.mDebuggerAttach(self.debug, core._core)
        self.sync_points()

    def sync_points(self):
        desired = {c['lr'] & ~1 for c in self.stack}
        if self.entries < self.args.calls:
            desired.add(self.args.entry)
        platform = self.debug.platform
        for address in set(self.points) - desired:
            platform.clearBreakpoint(platform, self.points.pop(address))
        for address in desired - set(self.points):
            point = self.ffi.new('struct mBreakpoint *')
            point.address, point.segment = address, -1
            point.type = self.lib.BREAKPOINT_HARDWARE
            ident = platform.setBreakpoint(platform, point)
            if ident < 0:
                raise RuntimeError(f'cannot set breakpoint at {address:#x}')
            self.points[address] = ident

    def memory(self, regs):
        regions = list(self.args.region)
        if self.args.stack_bytes:
            sp = regs[13]
            for name, size, base in REGIONS_V2[:2]:
                if base <= sp < base + size:
                    regions.append(('stack', sp, min(self.args.stack_bytes, base + size - sp)))
                    break
            else:
                raise RuntimeError(f'SP {sp:#x} is outside canonical CPU RAM')
        result = []
        for name, address, size in regions:
            # Direct memory avoids bus reads with IO side effects.
            parent = next(r for r in REGIONS_V2 if r[2] <= address < r[2] + r[1])
            available = self.ffi.new('size_t *')
            raw = self.lib.mCoreGetMemoryBlock(self.core._core, parent[2], available)
            offset = address - parent[2]
            if raw == self.ffi.NULL or offset + size > available[0]:
                raise RuntimeError(f'cannot map {name} at {address:#x}')
            data = bytes(self.ffi.buffer(self.ffi.cast('uint8_t *', raw) + offset, size))
            result.append(dict(name=name, address=address, size=size, hex=data.hex()))
        return result

    def entered(self, debugger, reason, info):
        # Exceptions must not escape through CFFI (which would only log them).
        try:
            if reason != self.lib.DEBUGGER_ENTER_BREAKPOINT or info == self.ffi.NULL:
                raise RuntimeError(f'unexpected debugger entry reason {reason}')
            cpu = self.core.cpu._native
            regs = [int(cpu.gprs[i]) & 0xffffffff for i in range(16)]
            cpsr = int(cpu.cpsr.packed) & 0xffffffff
            address = int(info.address)
            cycles = int(self.lib.mTimingGlobalTime(self.core._core.timing))
            common = dict(frame=int(self.core.frame_counter), cycles=cycles,
                          address=address, registers=regs, cpsr=cpsr,
                          memory=self.memory(regs))
            for call in pair_returns(self.stack, address, regs, cpsr):
                self.returns += 1
                self.emit(dict(event='return', call_id=call['id'],
                               elapsed_cycles=cycles-call['cycles'], **common))
            if address == self.args.entry and self.entries < self.args.calls:
                if bool(cpsr & 32) != (self.args.mode == 'thumb'):
                    raise RuntimeError('entry breakpoint reached in the wrong instruction set')
                self.entries += 1
                call = dict(id=self.entries, lr=regs[14], sp=regs[13],
                            cpu_mode=cpsr & 31, cycles=cycles)
                self.stack.append(call)
                self.emit(dict(event='entry', call_id=call['id'],
                               return_address=regs[14] & ~1, **common))
            self.sync_points()
        except Exception as exc:
            self.error = str(exc)
            for ident in self.points.values():
                self.debug.platform.clearBreakpoint(self.debug.platform, ident)
            self.points.clear()
        finally:
            # RunFrame must reach the frame boundary even after a capture error.
            debugger.state = self.lib.DEBUGGER_RUNNING

    def close(self):
        self.core._core.detachDebugger(self.core._core)


def run(args):
    import mgba.core
    import mgba.image
    import mgba.log
    from mgba._pylib import ffi, lib
    mgba.log.silence()
    for symbol in ('mDebuggerAttach', 'mDebuggerRunFrame', 'mTimingGlobalTime'):
        if not hasattr(lib, symbol):
            raise RuntimeError(f'mGBA bindings lack {symbol}; rebuild with debugger support')
    core = mgba.core.load_path(str(args.rom))
    if core is None or not hasattr(core, 'cpu'):
        raise RuntimeError('cannot load GBA ROM')
    cfg = mgba.core.Config()
    core.load_config(cfg)
    screen = mgba.image.Image(*core.desired_video_dimensions())
    core.set_video_buffer(screen)
    core.reset()
    inputs = load_inputs(args.inputs)
    if any(frame < 0 or mask < 0 or mask > 1023 for frame, mask in inputs.items()):
        raise ValueError('input frames must be nonnegative and key masks in 0..1023')
    # Match ramwatch's completed-frame convention. Frame zero is initial state.
    core.set_keys(raw=inputs.get(0, 0))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open('x') as stream:
        def emit(record):
            stream.write(json.dumps(record, sort_keys=True) + '\n')
            stream.flush()
        emit(dict(event='metadata', schema='gtadv3.function-trace.v1',
                  rom_sha256=hashlib.sha256(args.rom.read_bytes()).hexdigest(),
                  inputs_sha256=hashlib.sha256(args.inputs.read_bytes()).hexdigest(),
                  mgba_version=ffi.string(lib.projectVersion).decode(),
                  entry=args.entry, mode=args.mode, regions=args.region,
                  stack_bytes=args.stack_bytes, frames=args.frames, calls=args.calls,
                  seconds=args.seconds, initial_keys=inputs.get(0, 0),
                  input_timing='row 0 before execution; row N after completed frame N',
                  pc_semantics='registers[15] is raw pipeline PC; address is breakpoint address',
                  coverage='selected snapshots only; not a memory-access trace',
                  bios='mGBA built-in HLE', save='fresh; no save file loaded'))
        capture = Capture(core, ffi, lib, args, emit)
        deadline = time.monotonic() + args.seconds
        reason = 'frame_limit'
        try:
            while core.frame_counter < args.frames:
                lib.mDebuggerRunFrame(capture.debug)
                if capture.error:
                    raise RuntimeError(capture.error)
                core.set_keys(raw=inputs.get(core.frame_counter, core._core.getKeys(core._core)))
                if capture.entries >= args.calls and not capture.stack:
                    reason = 'call_limit'
                    break
                if time.monotonic() >= deadline:
                    reason = 'time_limit'
                    break
        except (Exception, KeyboardInterrupt) as exc:
            reason = 'error'
            emit(dict(event='error', message=str(exc) or 'interrupted'))
        finally:
            for call in capture.stack:
                emit(dict(event='incomplete', call_id=call['id'],
                          return_address=call['lr'] & ~1, reason=reason))
            emit(dict(event='summary', reason=reason, entries=capture.entries,
                      returns=capture.returns, incomplete=len(capture.stack),
                      frame=int(core.frame_counter)))
            capture.close()
    print(f'{capture.entries} entries, {capture.returns} returns ({reason}) -> {args.out}')
    return 0 if reason != 'error' and capture.returns and not capture.stack else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('inputs', type=Path)
    parser.add_argument('--entry', required=True, type=lambda s: int(s, 0))
    parser.add_argument('--mode', choices=('thumb', 'arm'), default='thumb')
    parser.add_argument('--out', required=True, type=Path, help='new JSONL file (never overwritten)')
    parser.add_argument('--region', action='append', type=region_spec, default=[])
    parser.add_argument('--stack-bytes', type=int, default=64)
    parser.add_argument('--calls', type=int, default=8)
    parser.add_argument('--frames', type=int, default=600)
    parser.add_argument('--seconds', type=float, default=60)
    args = parser.parse_args()
    if args.mode == 'thumb':
        args.entry &= ~1
    if not (0x08000000 <= args.entry < 0x0a000000) or (args.mode == 'arm' and args.entry % 4):
        parser.error('entry must be an aligned address in the primary ROM window')
    if min(args.calls, args.frames, args.seconds) <= 0 or not 0 <= args.stack_bytes <= 4096:
        parser.error('positive limits required; stack bytes must be 0..4096')
    if sum(r[2] for r in args.region) > 1024 * 1024:
        parser.error('selected regions exceed 1 MiB per event')
    try:
        return run(args)
    except ImportError as exc:
        parser.exit(2, f'function_trace: {exc}; run tools/build_mgba_python.sh, '
                       'export its PYTHONPATH, and use build/pyenv/bin/python\n')
    except (OSError, ValueError, RuntimeError) as exc:
        parser.exit(2, f'function_trace: {exc}\n')


if __name__ == '__main__':
    raise SystemExit(main())
