#!/usr/bin/env python3
"""Classify each selected function by how much of its source is actually C.

A byte-exact match says nothing about the source that produced it: a naked
function whose body is an `__asm__` instruction listing matches exactly, but it
is not decompiled. Following the convention of other decompilation projects
(INCLUDE_ASM/GLOBAL_ASM placeholders and pret's NAKED functions count as not
done), progress only credits functions whose bytes come from the compiler.

Classes, from least to most decompiled:

  asm          `__attribute__((naked))`, or a body whose inline instruction
               lines outnumber its C statements. Assembly in a .c file.
  partial      C with real instructions spliced in through `asm()`
               (e.g. `mov %0, r8`). Not counted as decompiled.
  c_fakematch  C whose only asm is empty barriers (`asm volatile("")`),
               assembler directives (`.set`, `.align`), or register pins
               (`register x __asm__("r4")`). Counted as decompiled; the
               tricks are listed so they can be cleaned up.
  c            Plain C. Counted as decompiled.

The classifier is pure Python so the ROM- and toolchain-free CI can run it. It
answers only one question -- did the compiler produce these bytes -- and never
reads a byte-match score, because the only committed match data,
`docs/data/report.json`, is a *projection of this classification* (see
`tools/decomp_report.py`: a function is written at 100% exactly when its class is
in `DECOMPILED`). Reading it back would make a freshly lifted body demote itself
on the previous run's label and never return. Byte truth is a separate, measured
fact: `make matching-ready` splices every selected body's *compiled output* and
requires an identical code slice, and `make full-rom-match` requires an identical
image, so a passing gate is the evidence that "this C reproduces the ROM". The
scanner strips comments, keeps string literals intact,
expands asm-bearing macros defined in the scanned files, and resolves each body
to its ROM address through its own name, `alias` declarations, and the
manifest's `c_name`/`export` lists.

Usage: python3 tools/source_quality.py            # check committed snapshot
       python3 tools/source_quality.py --write    # refresh snapshot
       python3 tools/source_quality.py --summary  # print class totals
       python3 tools/source_quality.py --list partial
       python3 tools/source_quality.py --self-test
"""
from __future__ import annotations

import argparse
from collections import defaultdict
import json
from pathlib import Path
import re
import sys
import tempfile
import unittest

import corpus_match_probe

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = Path('tools/matching_slice_functions.json')
SNAPSHOT = Path('docs/data/source_quality.json')
SCAN_DIRS = ('include', 'src')

CLASSES = ('asm', 'partial', 'c_fakematch', 'c')
DECOMPILED = frozenset({'c_fakematch', 'c'})

ASM_KEYWORD = re.compile(r'\b(?:__asm__|__asm|asm)\s*(?:volatile\b|__volatile__\b)?\s*\(')
ALIAS = re.compile(r'\b([A-Za-z_]\w*)\s*\([^;{}()]*\)\s*__attribute__\s*\(\(\s*'
                   r'(?:[a-z_]+\s*,\s*)*alias\s*\(\s*"(\w+)"')
DEFINE = re.compile(r'^[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w*)(\([^)]*\))?(.*)$', re.M)
KEYWORDS = frozenset({'if', 'while', 'for', 'switch', 'return', 'sizeof',
                      '__attribute__', '__asm__', 'asm', 'do', 'else'})


def strip_comments(text: str) -> str:
    """Drop comments and join continued lines; string and char literals survive."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == '\\' and i + 1 < n and text[i + 1] == '\n':
            out.append(' ')
            i += 2
        elif text.startswith('//', i):
            while i < n and text[i] != '\n':
                i += 1
        elif text.startswith('/*', i):
            end = text.find('*/', i + 2)
            end = n if end < 0 else end + 2
            # A single space, even for multi-line comments: a newline here would
            # cut a backslash-continued #define short.
            out.append(' ')
            i = end
        elif c in '"\'':
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == '\\' else 1
            out.append(text[i:j + 1])
            i = j + 1
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def skip_literal(text: str, i: int) -> int:
    """Index just past the string/char literal starting at text[i]."""
    quote, j = text[i], i + 1
    while j < len(text) and text[j] != quote:
        j += 2 if text[j] == '\\' else 1
    return j + 1


def matching(text: str, i: int, open_ch: str, close_ch: str) -> int:
    """Index just past the bracket matching text[i], skipping literals."""
    depth = 0
    while i < len(text):
        c = text[i]
        if c in '"\'':
            i = skip_literal(text, i)
            continue
        if c == open_ch:
            depth += 1
        elif c == close_ch:
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return len(text)


def asm_statements(text: str) -> list[tuple[str, str]]:
    """(kind, template) for each asm() in text.

    kind is 'pin' for `register ... __asm__("rN")`, 'barrier' for an empty
    template, 'directive' for templates made only of assembler directives or
    labels, and 'insn' for anything that emits an instruction.
    """
    found = []
    for m in ASM_KEYWORD.finditer(text):
        close = matching(text, m.end() - 1, '(', ')')
        inner = text[m.end():close - 1]
        head = _before_operands(inner)
        template = ''.join(re.findall(r'"((?:[^"\\]|\\.)*)"', head))
        stringified = re.search(r'#\s*[A-Za-z_]', head) is not None
        before = text[max(0, m.start() - 200):m.start()]
        statement_start = max(before.rfind(';'), before.rfind('{'), before.rfind('}'))
        declarator = before[statement_start + 1:].strip()
        if (declarator and not declarator.endswith(':')
                and not re.match(r'(?:if|else|while|for|do|switch)\b', declarator)):
            # asm() inside a declaration: a register pin or a symbol rename
            # (`extern T f(void) __asm__("_08002BFC")`). Neither emits code.
            found.append(('pin' if re.search(r'\bregister\b', declarator) else 'directive',
                          template))
            continue
        lines = [ln.strip() for ln in re.split(r'\\n|;|\\t', template)]
        lines = [ln for ln in lines if ln]
        if not lines and not stringified:
            found.append(('barrier', template))
        elif not stringified and all(_is_directive(ln) for ln in lines):
            found.append(('directive', template))
        else:
            found.append(('insn', template))
    return found


def _before_operands(inner: str) -> str:
    """The template part of asm(...): everything before the first `:` outside a literal."""
    i = 0
    while i < len(inner):
        if inner[i] == '"':
            i = skip_literal(inner, i)
            continue
        if inner[i] == ':':
            return inner[:i]
        i += 1
    return inner


def _is_directive(line: str) -> bool:
    """Assembler directive, label, comment, or `sym = expr`: no instruction."""
    return (line.startswith(('.', '@')) or line.endswith(':')
            or re.fullmatch(r'[A-Za-z_.$][\w.$]*\s*=.*', line) is not None)


def instruction_lines(template: str) -> int:
    lines = [ln.strip() for ln in re.split(r'\\n|;', template)]
    return sum(1 for ln in lines if ln and not _is_directive(ln))


def asm_macros(texts: list[str]) -> dict[str, list[tuple[str, str]]]:
    """Macros whose expansion contains asm, keyed by name.

    A macro defined differently per #ifdef branch keeps every branch's
    statements, so an ARM-only instruction is never hidden by its host stub.
    Macros that define whole functions are handled by `macro_functions`.
    """
    macros: dict[str, list[tuple[str, str]]] = defaultdict(list)
    for text in texts:
        for m in DEFINE.finditer(text):
            body = m.group(3)
            if re.search(r'\)\s*\{', body) and m.group(2):
                continue
            stmts = asm_statements(body)
            if stmts:
                macros[m.group(1)].extend(stmts)
    return dict(macros)


def macro_functions(text: str):
    """Yield (name, prefix, body) for functions stamped out by a #define."""
    for m in DEFINE.finditer(text):
        params = m.group(2)
        body = m.group(3)
        if not params:
            continue
        names = [p.strip() for p in params[1:-1].split(',')]
        fn = re.search(r'([A-Za-z_]\w*)\s*\([^)]*\)\s*\{', body)
        if not fn or fn.group(1) not in names:
            continue
        slot = names.index(fn.group(1))
        open_brace = body.index('{', fn.start())
        fbody = body[open_brace + 1:matching(body, open_brace, '{', '}') - 1]
        prefix = body[:fn.start()]
        for call in re.finditer(r'^[ \t]*' + re.escape(m.group(1)) + r'\s*\(([^)]*)\)', text, re.M):
            args = [a.strip() for a in call.group(1).split(',')]
            if len(args) == len(names):
                yield args[slot], prefix, fbody


def functions(text: str):
    """Yield (name, prefix, body) for each top-level function definition."""
    depth = 0
    i, n = 0, len(text)
    last_boundary = 0
    while i < n:
        c = text[i]
        if c in '"\'':
            i = skip_literal(text, i)
            continue
        if c == '#' and (i == 0 or text[i - 1] == '\n'):
            i = text.find('\n', i)
            i = n if i < 0 else i
            last_boundary = i
            continue
        if c in ';}' and depth == 0:
            last_boundary = i + 1
        if c == '{':
            if depth == 0:
                head = text[last_boundary:i]
                close = matching(text, i, '{', '}')
                m = re.search(r'([A-Za-z_]\w*)\s*\(', _strip_attributes(head))
                if m and m.group(1) not in KEYWORDS and head.rstrip().endswith(')') \
                        and '=' not in head.split('(')[0]:
                    yield m.group(1), head, text[i + 1:close - 1]
                i = close
                last_boundary = i
                continue
            depth += 1
        elif c == '}':
            depth -= 1
        i += 1


def _strip_attributes(head: str) -> str:
    out = head
    while True:
        m = re.search(r'__attribute__\s*\(', out)
        if not m:
            return out
        end = matching(out, m.end() - 1, '(', ')')
        out = out[:m.start()] + ' ' + out[end:]


def classify_body(prefix: str, body: str, macros: dict) -> tuple[str, dict]:
    stmts = asm_statements(body)
    for name, expansion in macros.items():
        hits = len(re.findall(r'\b' + re.escape(name) + r'\b', body))
        stmts.extend(expansion * hits)
    kinds = defaultdict(int)
    for kind, _ in stmts:
        kinds[kind] += 1
    insn_lines = sum(instruction_lines(t) for k, t in stmts if k == 'insn')
    c_statements = body.count(';') - len(stmts)
    naked = re.search(r'\bnaked\b', prefix) is not None
    detail = {k: kinds[k] for k in ('insn', 'barrier', 'directive', 'pin') if kinds[k]}
    if insn_lines:
        detail['insn_lines'] = insn_lines
    if naked:
        detail['naked'] = True
        return 'asm', detail
    if kinds['insn'] and insn_lines >= max(4, c_statements):
        return 'asm', detail
    if kinds['insn']:
        return 'partial', detail
    if kinds['barrier'] or kinds['directive'] or kinds['pin']:
        return 'c_fakematch', detail
    return 'c', detail


def parse_vma(name: str, valid) -> int | None:
    # The probe's parser accepts both `_08028250` and `_080028250` spellings.
    vma = corpus_match_probe.parse_vma_name(name)
    return vma if vma in valid else None


def classify(root: Path = ROOT) -> dict:
    manifest = json.loads((root / MANIFEST).read_text())
    spans = {int(k, 16): int(v['end_vma'], 16) - int(k, 16) for k, v in manifest.items()}
    by_name = {}
    for k, v in manifest.items():
        for name in (v['c_name'], *v.get('export', ())):
            by_name[name] = int(k, 16)

    files = {}
    for directory in SCAN_DIRS:
        for path in sorted((root / directory).rglob('*')):
            if path.suffix in ('.c', '.h') and path.is_file():
                files[path.relative_to(root).as_posix()] = strip_comments(
                    path.read_text(encoding='utf-8', errors='replace'))
    macros = asm_macros(list(files.values()))

    best: dict[int, dict] = {}
    is_weak: dict[int, bool] = {}
    for rel, text in files.items():
        if not rel.startswith('src/') or not rel.endswith('.c'):
            continue
        aliases = defaultdict(set)
        for alias, target in ALIAS.findall(text):
            aliases[target].add(alias)
        local = {k: v for k, v in macros.items() if re.search(r'\b' + re.escape(k) + r'\b', text)}
        bodies = list(functions(text)) + list(macro_functions(text))
        for name, prefix, body in bodies:
            vma = None
            for candidate in (name, *sorted(aliases[name])):
                vma = by_name.get(candidate) or parse_vma(candidate, spans)
                if vma in spans:
                    break
                vma = None
            if vma is None:
                continue
            weak = re.search(r'\bweak\b', prefix) is not None
            cls, detail = classify_body(prefix, body, local)
            row = {'name': name, 'file': rel, 'size': spans[vma], 'class': cls}
            if detail:
                row['detail'] = detail
            # The strong body is the one that links; a weak placeholder loses.
            if vma not in best or (is_weak[vma] and not weak):
                best[vma] = row
                is_weak[vma] = weak

    missing = sorted(set(spans) - set(best))
    if missing:
        names = ', '.join(manifest[f'0x{vma:08x}']['c_name'] for vma in missing[:8])
        raise ValueError(f'{len(missing)} selected functions have no definition in src/: {names}')
    weak_only = sorted(vma for vma, weak in is_weak.items() if weak)
    if weak_only:
        raise ValueError(f'selected functions defined only as weak placeholders: '
                         f'{[hex(v) for v in weak_only[:8]]}')

    totals = {cls: {'functions': 0, 'bytes': 0} for cls in CLASSES}
    for row in best.values():
        totals[row['class']]['functions'] += 1
        totals[row['class']]['bytes'] += row['size']
    return {
        'version': 1,
        'definitions': {
            'asm': 'naked function or asm-dominated body; not decompiled',
            'partial': 'C with instructions spliced in through asm(); not decompiled',
            'c_fakematch': 'C with only empty barriers, directives, or register pins; decompiled',
            'c': 'plain C; decompiled',
        },
        'totals': totals,
        'functions': {f'0x{vma:08x}': best[vma] for vma in sorted(best)},
    }


def source_classes(root: Path = ROOT) -> dict[int, str]:
    return {int(k, 16): row['class'] for k, row in classify(root)['functions'].items()}


def encoded(value) -> bytes:
    return (json.dumps(value, indent=1, sort_keys=True) + '\n').encode()


def summary_lines(result: dict) -> list[str]:
    totals = result['totals']
    all_fns = sum(t['functions'] for t in totals.values())
    all_bytes = sum(t['bytes'] for t in totals.values())
    lines = [f'{"class":12s} {"functions":>10s} {"bytes":>9s} {"share":>7s}']
    for cls in CLASSES:
        t = totals[cls]
        lines.append(f'{cls:12s} {t["functions"]:10,d} {t["bytes"]:9,d} '
                     f'{100 * t["bytes"] / all_bytes:6.2f}%')
    done = sum(totals[c]['bytes'] for c in DECOMPILED)
    done_fns = sum(totals[c]['functions'] for c in DECOMPILED)
    lines.append(f'decompiled   {done_fns:10,d} {done:9,d} {100 * done / all_bytes:6.2f}%'
                 f'  (of {all_fns:,} functions / {all_bytes:,} bytes)')
    return lines


def check(root: Path = ROOT) -> None:
    path = root / SNAPSHOT
    if not path.is_file() or path.read_bytes() != encoded(classify(root)):
        raise ValueError(f'{SNAPSHOT} is stale; run python3 tools/source_quality.py --write')
    print('source-quality: snapshot agrees with src/')


class SelfTest(unittest.TestCase):
    def run_fixture(self, source: str, names: list[str],
                    percent: list[float] | None = None) -> dict:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'tools').mkdir()
            (root / 'src').mkdir()
            (root / 'src/a.c').write_text(source)
            manifest = {f'0x{0x08000100 + 0x10 * i:08x}': {
                'c_name': name, 'end_vma': f'0x{0x08000110 + 0x10 * i:08x}'}
                for i, name in enumerate(names)}
            (root / MANIFEST).write_text(json.dumps(manifest))
            if percent is not None:
                functions = [{'metadata': {'virtual_address': 0x08000100 + 0x10 * i},
                              'fuzzy_match_percent': p} for i, p in enumerate(percent)]
                report = root / 'docs/data/report.json'
                report.parent.mkdir(parents=True, exist_ok=True)
                report.write_text(json.dumps({'units': [{'functions': functions}]}))
            result = classify(root)
        return {row['name']: row['class'] for row in result['functions'].values()}

    def test_classes(self):
        source = r'''
#define BARRIER(v) __asm__ volatile ("" : "+r" (v))
#ifdef __arm__
#define STALE(d) __asm__ volatile ("mov %0, r8" : "=r" (d))
#else
#define STALE(d) do { (d) = 0; } while (0)
#endif
#define VENEER(name, reg) \
    __attribute__((naked)) void name(void) { __asm__ volatile ("bx " reg); }
/* int Commented(void) { return 0; } */
int Plain(int a) { if (a) { return a + 1; } return 0; }
__attribute__((naked)) void Naked(void) { __asm__ volatile ("push {lr}\n" "pop {pc}\n"); }
int Pinned(void) { register int x __asm__("r4") = 1; return x; }
int Barrier(int v) { BARRIER(v); return v; }
int Directive(void) { __asm__(".set Foo, 0x03000000"); return 1; }
int Partial(void) { int d; STALE(d); d += 1; d *= 3; return d; }
int Listing(void) { int a; __asm__ volatile ("mov r0, #1\n" "add r0, r0\n" "lsl r0, #2\n" "bx lr\n"); return a; }
const char *Str(void) { return "{ not a brace }"; }
int Clobber(int v) { __asm__ volatile ("" ::: "memory"); return v; }
int Rename(void) { extern int f(void) __asm__("_08002BFC"); return f(); }
int SymDef(void) { __asm__(".globl Tbl\\nTbl = 0x080C4BA0\\n"); return 0; }
int Labelled(int v) { if (v) goto L; return 0; L: __asm__ volatile ("ldrsh %0, [%1, %2]" : "=r" (v) : "r" (v), "r" (v)); return v; }
VENEER(Veneer_2DDC8, "r0")
void _08000190(void) __attribute__((alias("Str")));
'''
        got = self.run_fixture(source, ['Plain', 'Naked', 'Pinned', 'Barrier', 'Directive',
                                        'Partial', 'Listing', 'Str', 'Veneer_2DDC8',
                                        'Clobber', 'Rename', 'SymDef', 'Labelled'])
        self.assertEqual(got, {
            'Plain': 'c', 'Naked': 'asm', 'Pinned': 'c_fakematch', 'Barrier': 'c_fakematch',
            'Directive': 'c_fakematch', 'Partial': 'partial', 'Listing': 'asm', 'Str': 'c',
            'Veneer_2DDC8': 'asm', 'Clobber': 'c_fakematch', 'Rename': 'c_fakematch',
            'SymDef': 'c_fakematch', 'Labelled': 'partial'})

    def test_alias_and_vma_names_resolve(self):
        source = ('int Friendly(void) { return 1; }\n'
                  'int _08000100(void) __attribute__((alias("Friendly")));\n'
                  'int _08000110(void) { return 2; }\n')
        got = self.run_fixture(source, ['sub_08000100', 'sub_08000110'])
        self.assertEqual(got, {'Friendly': 'c', '_08000110': 'c'})

    def test_strong_body_beats_weak_placeholder(self):
        source = ('__attribute__((weak)) int _08000100(void) { return 0; }\n'
                  '__attribute__((naked)) int Strong(void) { __asm__("bx lr"); }\n'
                  'int sub_08000100(void) __attribute__((alias("Strong")));\n')
        got = self.run_fixture(source, ['_08000100'])
        self.assertEqual(got, {'Strong': 'asm'})

    def test_report_is_not_match_evidence(self):
        # report.json is written *from* this classification, so its per-function
        # percent carries no information about the bytes. A body must keep its
        # source-form verdict even when the report beside it says 0%.
        source = 'int Exact(void) { return 1; }\nint Wip(void) { return 2; }\n'
        names = ['Exact', 'Wip']
        self.assertEqual(self.run_fixture(source, names), {'Exact': 'c', 'Wip': 'c'})
        self.assertEqual(self.run_fixture(source, names, [100.0, 0.0]),
                         {'Exact': 'c', 'Wip': 'c'})
        self.assertEqual(self.run_fixture(source, names, [100.0]),
                         {'Exact': 'c', 'Wip': 'c'})

    def test_missing_definition_fails(self):
        with self.assertRaisesRegex(ValueError, 'no definition'):
            self.run_fixture('int Other(void) { return 0; }\n', ['Missing'])


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--write', action='store_true', help='refresh the committed snapshot')
    parser.add_argument('--summary', action='store_true', help='print class totals')
    parser.add_argument('--list', choices=CLASSES, help='list functions in one class, largest first')
    parser.add_argument('--self-test', action='store_true')
    args = parser.parse_args()
    if args.self_test:
        unittest.main(argv=[sys.argv[0]], defaultTest='SelfTest', verbosity=1)
        return
    try:
        if args.list:
            rows = classify(ROOT)['functions']
            for vma, row in sorted(rows.items(), key=lambda kv: -kv[1]['size']):
                if row['class'] == args.list:
                    print(f'{vma} {row["size"]:6d} {row["file"]}:{row["name"]} {row.get("detail", {})}')
        elif args.summary:
            print('\n'.join(summary_lines(classify(ROOT))))
        elif args.write:
            result = classify(ROOT)
            (ROOT / SNAPSHOT).write_bytes(encoded(result))
            print('\n'.join(summary_lines(result)))
        else:
            check(ROOT)
    except (OSError, ValueError) as exc:
        parser.exit(1, f'source_quality: {exc}\n')


if __name__ == '__main__':
    main()
