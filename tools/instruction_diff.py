#!/usr/bin/env python3
"""Advisory instruction alignment. Exact linked bytes remain the acceptance gate."""
from collections import Counter
from difflib import SequenceMatcher
import re

LINE = re.compile(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{4,8}\s+)+)\s*(\S+)\s*(.*)$')
REG = re.compile(r'\b(?:r(?:1[0-5]|[0-9])|sp|lr|pc|sl|ip|fp)\b')
ALIASES = {'sp': 'r13', 'lr': 'r14', 'pc': 'r15', 'sl': 'r10', 'ip': 'r12', 'fp': 'r11'}


def instructions(text, allowed=None):
    rows = []
    for line in text.splitlines():
        m = LINE.match(line)
        if not m:
            continue
        pc = int(m[1], 16)
        if allowed is not None and pc not in allowed:
            continue
        op = m[3].removesuffix('.n').removesuffix('.w')
        args = m[4].split('@')[0].strip()
        args = REG.sub(lambda m: ALIASES.get(m[0], m[0]), args)
        args = re.sub(r'#(-?(?:0x[0-9a-f]+|\d+))', lambda m: '#' + str(int(m[1], 16 if '0x' in m[1] else 10)), args)
        args = re.sub(r'\s+', '', args)
        rows.append({'pc': pc, 'op': op, 'args': args,
                     'size': len(''.join(m[2].split())) // 2})
    return rows


def register_shape(args):
    return REG.sub('R', args)


def classify_text(a, b):
    """Conservative fallback for callers without reachability/pool evidence."""
    ra, rb = instructions(a), instructions(b)
    ka = [(r['op'], r['args']) for r in ra]
    kb = [(r['op'], r['args']) for r in rb]
    if not ka or not kb:
        return 'instruction selection'
    if ka == kb:
        return 'identical disassembly operands'
    if len(ka) == len(kb) and all(x[0] == y[0] and register_shape(x[1]) == register_shape(y[1])
                                 for x, y in zip(ka, kb)):
        return 'identical mnemonics (register/operand choice)'
    if Counter(ka) == Counter(kb):
        return 'instruction order'
    return 'instruction selection'


def analyze(reference, candidate, vma, reference_text, candidate_text):
    # Import lazily: family decoding itself uses the probe's ROM constants.
    from match_families import flow
    def decode(blob, listing):
        image = b'\0' * (vma - 0x08000000) + blob
        reachable, _, failures = flow(image, vma, vma + len(blob))
        rows = instructions(listing, reachable)
        for row in rows:
            pc, op = row['pc'], row['op']
            h = int.from_bytes(blob[pc-vma:pc-vma+2], 'little')
            row['target'] = None
            row['pool'] = None
            row['pool_address'] = None
            row['pool_displacement'] = None
            if op.startswith('b') and op not in ('bic', 'bics', 'bx', 'blx'):
                match = re.match(r'(?:0x)?([0-9a-f]+)', row['args'])
                if match:
                    row['target'] = int(match[1], 16)
                    row['args'] = ('<local>' if op != 'bl' and vma <= row['target'] < vma + len(blob)
                                   else hex(row['target']))
            if h & 0xf800 == 0x4800:
                pool = ((pc + 4) & ~3) + (h & 255) * 4
                off = pool - vma
                if 0 <= off and off + 4 <= len(blob):
                    row['pool'] = int.from_bytes(blob[off:off+4], 'little')
                    row['pool_address'] = pool
                    row['pool_displacement'] = (h & 255) * 4
                    row['args'] = row['args'].split(',')[0] + ',<pool>'
                else:
                    failures.append(f'literal outside compared span: {pool:#x}')
        if len(rows) != len(reachable):
            failures.append('disassembly did not cover every reachable instruction')
        return rows, failures
    a, fa = decode(reference, reference_text)
    b, fb = decode(candidate, candidate_text)
    def key(r):
        args = '<target>' if r['target'] is not None else register_shape(r['args'])
        return r['op'], re.sub(r'#-?\d+', '#', args)
    matcher = SequenceMatcher(None, [key(r) for r in a], [key(r) for r in b], autojunk=False)
    pairs, edits = [], []
    for tag, ai, aj, bi, bj in matcher.get_opcodes():
        if tag == 'equal':
            pairs.extend(zip(range(ai, aj), range(bi, bj)))
        else:
            edits.append({'kind': tag, 'reference_offsets': [r['pc']-vma for r in a[ai:aj]],
                          'candidate_offsets': [r['pc']-vma for r in b[bi:bj]],
                          'reference': a[ai:aj], 'candidate': b[bi:bj]})
    mapping = {a[i]['pc']: b[j]['pc'] for i, j in pairs}
    differences = []
    agreeing = 0
    for i, j in pairs:
        x, y = a[i], b[j]
        kinds = []
        if x['args'] != y['args']:
            kinds.append('register' if register_shape(x['args']) == register_shape(y['args'])
                         else 'memory-offset' if '[' in x['args'] or '[' in y['args']
                         else 'immediate' if '#' in x['args'] or '#' in y['args'] else 'operand')
        if x['pool'] != y['pool']:
            kinds.append('pool-value')
        if x['pool_address'] != y['pool_address'] or x['pool_displacement'] != y['pool_displacement']:
            kinds.append('pool-placement')
        if x['target'] is not None or y['target'] is not None:
            expected = x['target'] if x['op'] == 'bl' else mapping.get(x['target'], x['target'])
            if expected != y['target']:
                kinds.append('call-target' if x['op'] == 'bl' else 'branch-target')
        if kinds:
            differences.append({'reference_offset': x['pc']-vma, 'candidate_offset': y['pc']-vma,
                                'kinds': kinds, 'reference': x, 'candidate': y})
        else:
            agreeing += 1
    complete = not fa and not fb
    code_a = {r['pc']-vma+k for r in a for k in range(r['size'])}
    code_b = {r['pc']-vma+k for r in b for k in range(r['size'])}
    noncode = [k for k in range(min(len(reference), len(candidate)))
               if k not in code_a and k not in code_b and reference[k] != candidate[k]]
    register_only = (complete and not edits and bool(differences)
                     and all(d['kinds'] == ['register'] for d in differences)
                     and all(a[i]['pc'] == b[j]['pc'] and a[i]['target'] == b[j]['target'] for i, j in pairs)
                     and len(reference) == len(candidate))
    # Matching code is insufficient if padding/unreferenced data differs.
    if register_only:
        # objdump's printed halfwords have the same byte count as the instruction.
        register_only = all(reference[k] == candidate[k] for k in range(len(reference)) if k not in code_a)
    klass = 'identical mnemonics (register/operand choice)' if register_only else 'instruction selection'
    strict = lambda r: (r['op'], r['args'], r['pool'])
    paired_a, paired_b = {i for i, _ in pairs}, {j for _, j in pairs}
    reordered = (complete and bool(edits) and Counter(map(strict, a)) == Counter(map(strict, b))
                 and all(r['target'] is None or i in paired_a for i, r in enumerate(a))
                 and all(r['target'] is None or j in paired_b for j, r in enumerate(b))
                 and not differences)
    if reordered:
        klass = 'instruction order'
    if any('pool-value' in d['kinds'] for d in differences):
        klass = 'literal pool value'
    elif any('call-target' in d['kinds'] or 'branch-target' in d['kinds'] for d in differences):
        klass = 'control-flow target'
    elif any('immediate' in d['kinds'] or 'memory-offset' in d['kinds'] for d in differences):
        klass = 'immediate or memory offset'
    elif any('pool-placement' in d['kinds'] for d in differences):
        klass = 'literal pool placement'
    elif not differences and not edits and noncode:
        klass = 'data or padding'
    return {'class': klass, 'register_only': register_only, 'reordered': reordered, 'complete': complete,
            'limitations': {'reference': fa, 'candidate': fb},
            'reference_instructions': len(a), 'candidate_instructions': len(b),
            'aligned_instructions': len(pairs), 'agreeing_instructions': agreeing,
            'edits': edits, 'differences': differences,
            'noncode_differences': len(noncode), 'first_noncode_difference': noncode[0] if noncode else None,
            'scope': 'advisory alignment; whole-span linked bytes decide exactness'}
