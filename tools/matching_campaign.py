#!/usr/bin/env python3
"""Resumable, bounded full-TU matching experiments. Never modifies game source.

Cases JSON: {"variants": [{"name": "hypothesis", "edits": [{"old": "...",
"new": "..."}]}], "matrix": [{"name": "axis", "old": "...",
"choices": ["...", "..."]}]}. Each needle must occur once in the target body.
Matrix choices form a bounded Cartesian product; each variant starts at the
current source baseline. Results, failures and best full TU persist under build/.
"""
import argparse
import fcntl
import hashlib
import itertools
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import time

import experiment_kit as kit
import corpus_match_probe as probe
from run_permuter import function_ranges


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def atomic_json(path, value):
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')
    temporary.replace(path)


def apply_edits(source, body_name, edits):
    rows = [r for r in function_ranges(source) if r[0] == body_name]
    if len(rows) != 1:
        raise ValueError('target body is absent or ambiguous in current TU')
    _, _, lo, hi = rows[0]
    body = source[lo:hi]
    for edit in edits:
        old, new = edit['old'], edit['new']
        if not isinstance(old, str) or not old or not isinstance(new, str) or body.count(old) != 1:
            raise ValueError('each edit must match exactly once in the target body')
        body = body.replace(old, new, 1)
    return source[:lo] + body + source[hi:]


def cases(spec):
    yield 'baseline', []
    for variant in spec.get('variants', []):
        yield variant['name'], variant['edits']
    axes = spec.get('matrix', [])
    if axes:
        for values in itertools.product(*(axis['choices'] for axis in axes)):
            yield 'matrix:' + ','.join(f'{a["name"]}={i}' for a, i in zip(axes, values)), [
                {'old': a['old'], 'new': value} for a, value in zip(axes, values)]


def rank(record):
    if record.get('status') not in ('EXACT', 'PARTIAL', 'OVERSIZED', 'NO_OVERLAP'):
        return (-1,)
    alignment = record.get('instruction_diff', {})
    return (int(record['status'] == 'EXACT'),
            alignment.get('agreeing_instructions', 0),
            -abs(record['cand_bytes']-record['rom_bytes']),
            record.get('matched_bytes', 0), record.get('prefix', 0))


def worker(job_path):
    job = json.loads(job_path.read_text())
    directory = job_path.parent
    meter = kit.BudgetMeter(kit.Budget(compiles=job['budget'], seconds=job['seconds']))
    adapter = kit.CandidateAdapter(directory / 'work', cache_dir=Path(job['cache']), meter=meter,
                                   flags=tuple(job['flags']), include_dirs=(Path(job['source_parent']),))
    compiled = adapter.compile((directory/'candidate.c').read_text(), job['body_name'], job['vma'])
    record = {'status': 'COMPILE_ERROR', 'error': compiled.error}
    if compiled.ok:
        previous = probe.WORK
        probe.WORK = directory / 'probe'
        try:
            record = probe.compare_function(adapter.rom, job['vma'], job['function'], compiled.blob,
                                            {job['vma']: job['end']}, compiled.calls, compiled.pools,
                                            sym_size=compiled.symbol_size)
        finally:
            probe.WORK = previous
        if record is None:
            record = {'status': 'UNSUPPORTED', 'error': 'span cannot be compared'}
        else:
            (directory/'candidate.bin').write_bytes(compiled.blob)
            record['output_sha256'] = hashlib.sha256(compiled.blob).hexdigest()
    atomic_json(directory/'result.json', {'record': record, 'budget': meter.to_json()})


def bounded_worker(job_path, seconds):
    process = subprocess.Popen([sys.executable, str(Path(__file__).resolve()), '--worker', str(job_path)],
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                               start_new_session=True)
    try:
        output, _ = process.communicate(timeout=seconds)
        if process.returncode:
            raise RuntimeError(output[-4000:])
    finally:
        if process.poll() is None:
            os.killpg(process.pid, signal.SIGTERM)
            try:
                process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
        if process.stdout is not None:
            process.stdout.close()


def run(args):
    kit.require_toolchain()
    functions = probe.src_functions(filter_wanted=[args.function])
    wanted = probe.parse_vma_name(args.function)
    rows = [r for r in functions if r[1] == args.function or wanted is not None and r[2] == wanted]
    if not rows:
        raise ValueError('no current source owner for requested target')
    source_path, function, vma, _, _ = max(rows, key=lambda r: (not r[4], r[3]))
    source_path = Path(source_path)
    source = source_path.read_text()
    body_name = probe.alias_targets().get(function, function)
    apply_edits(source, body_name, [])  # validate current ownership, not a stale report
    spans = probe.rom_functions()
    if vma not in spans:
        raise ValueError('target has no authoritative ROM boundary')
    out = (args.out or probe.ROOT / 'build/experiments/campaigns' / f'{vma:08x}').resolve()
    # A campaign must never overwrite tracked source or accept a destructive output root.
    if out == probe.ROOT or probe.ROOT / 'src' == out or probe.ROOT / 'asm' == out:
        raise ValueError('output must be a dedicated experiment directory')
    if out.is_relative_to(probe.ROOT) and not out.is_relative_to(probe.ROOT/'build'):
        raise ValueError('repository outputs must be under ignored build/')
    out.mkdir(parents=True, exist_ok=True)
    with (out/'campaign.lock').open('a') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        flags = probe.agbcc_options(source_path.name)
        headers = {str(p.relative_to(probe.ROOT)): kit.digest_path(p)
                   for parent in ('include', 'src', 'asm')
                   for p in sorted((probe.ROOT/parent).rglob('*')) if p.suffix in ('.h', '.inc')}
        _, _, body_lo, body_hi = next(r for r in function_ranges(source) if r[0] == body_name)
        environment = digest({'compiler': kit.digest_path(probe.AGBCC), 'rom': kit.digest_path(probe.ROM),
                              'tu_context': digest(source[:body_lo] + source[body_hi:]),
                              'flags': flags, 'headers': headers, 'symbols': probe.load_code_symbols(),
                              'aliases': probe.alias_targets(), 'hints': probe.scan_decl_hints(),
                              'vma': vma, 'end': spans[vma], 'body': body_name,
                              'source_parent': str(source_path.parent),
                              'tools': {name: kit.digest_path(probe.ROOT/'tools'/name) for name in
                                        ('matching_campaign.py', 'experiment_kit.py', 'corpus_match_probe.py',
                                         'instruction_diff.py', 'agbcc_c89_transform.py', 'agbcc_options.py')}})
        ledger = out/'ledger.jsonl'
        records = [json.loads(line) for line in ledger.read_text().splitlines()] if ledger.exists() else []
        prior = {r['key']: r for r in records if r['environment'] == environment and r.get('reusable')}
        best = max((r for r in prior.values() if rank(r['record'])[0] >= 0),
                   key=lambda r: rank(r['record']), default=None)
        spec = json.loads(args.cases.read_text()) if args.cases else {'variants': [
            {'name': s, 'edits': [{'old': s.split('==>', 1)[0], 'new': s.split('==>', 1)[1]}]}
            for s in args.mutate]}
        started, used, reused, duplicates, generated = time.monotonic(), 0, 0, 0, 0
        outputs = {r['record'].get('output_sha256') for r in prior.values()}
        stop = 'exhausted candidate set'
        for name, edits in itertools.islice(cases(spec), args.max_variants):
            remaining = args.seconds - (time.monotonic()-started)
            if used >= args.budget or remaining <= 0:
                stop = 'budget exhausted'
                break
            generated += 1
            try:
                candidate = apply_edits(source, body_name, edits)
            except (ValueError, KeyError, TypeError) as exc:
                print(f'{name}: INVALID_EDIT: {exc}')
                continue
            key = digest({'source': candidate, 'environment': environment})
            if key in prior:
                row = prior[key]
                reused += 1
                print(f'{name}: reused {row["record"]["status"]}')
                continue
            directory = out/'candidates'/key
            directory.mkdir(parents=True, exist_ok=True)
            (directory/'candidate.c').write_text(candidate)
            job = {'body_name': body_name, 'function': function, 'vma': vma, 'end': spans[vma],
                   'flags': flags, 'source_parent': str(source_path.parent),
                   'budget': args.budget-used, 'seconds': remaining, 'cache': str(out/'cache')}
            atomic_json(directory/'job.json', job)
            try:
                bounded_worker(directory/'job.json', remaining)
                result = json.loads((directory/'result.json').read_text())
                used += result['budget']['compiles_used']
                record = result['record']
            except subprocess.TimeoutExpired:
                stop = 'time budget exhausted'
                record = {'status': 'TIMEOUT'}
            except RuntimeError as exc:
                # A failed worker may have started a compile; charge it conservatively.
                used += 1
                record = {'status': 'TOOL_FAILURE', 'error': str(exc)}
            output_hash = record.get('output_sha256')
            duplicate = output_hash is not None and output_hash in outputs
            duplicates += int(duplicate)
            outputs.add(output_hash)
            row = {'name': name, 'key': key, 'environment': environment, 'record': record,
                   'candidate': str(directory/'candidate.c'), 'duplicate_output': duplicate,
                   'reusable': record['status'] not in ('TIMEOUT', 'TOOL_FAILURE'),
                   'seconds': round(time.monotonic()-started, 3)}
            with ledger.open('a') as stream:
                stream.write(json.dumps(row, sort_keys=True)+'\n')
                stream.flush()
                os.fsync(stream.fileno())
            prior[key] = row
            print(f'{name}: {record["status"]} {record.get("matched_bytes", 0)}/{record.get("rom_bytes", 0)}')
            if rank(record)[0] >= 0 and (best is None or rank(record) > rank(best['record'])):
                best = row
            if record['status'] == 'TIMEOUT':
                break
        if generated == args.max_variants:
            stop = 'generation cap reached'
        if best:
            (out/'best.c').write_text(Path(best['candidate']).read_text())
            atomic_json(out/'best.json', best)
        elif (out/'best.json').exists():
            # Keep old artifact but make its stale provenance explicit in this run.
            stop += '; no current-environment best'
        summary = {'environment': environment, 'baseline_sha256': kit.digest_bytes(source.encode()),
                   'generated': generated, 'compiles': used, 'reused': reused,
                   'duplicate_outputs': duplicates, 'stop': stop,
                   'best': best, 'integrated_status': 'not run', 'accepted_bytes': 0}
        atomic_json(out/'summary.json', summary)
        print(f'Campaign: {out}; compiles={used}, reused={reused}, duplicate outputs={duplicates}')
        return summary


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('function', nargs='?')
    ap.add_argument('--cases', type=Path)
    ap.add_argument('--mutate', action='append', default=[], metavar='OLD==>NEW')
    ap.add_argument('--out', type=Path)
    ap.add_argument('--budget', type=int, default=48)
    ap.add_argument('--seconds', type=float, default=900)
    ap.add_argument('--max-variants', type=int, default=64)
    ap.add_argument('--worker', type=Path, help=argparse.SUPPRESS)
    args = ap.parse_args()
    if args.worker:
        worker(args.worker)
        return 0
    if not args.function or args.budget < 1 or args.seconds <= 0 or args.max_variants < 1:
        ap.error('function and positive budgets are required')
    if args.cases and args.mutate or any('==>' not in s for s in args.mutate):
        ap.error('choose --cases or OLD==>NEW mutations')
    try:
        run(args)
    except (ValueError, OSError, kit.DependencyMissing) as exc:
        ap.error(str(exc))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
