#!/usr/bin/env python3
"""Predict `match_c_slice.py --all` undefined references, without linking.

Every `match_c_slice.py --all` round recompiles one TU per manifest entry and
relinks the whole 188,760-byte slice, so it costs minutes and stops at the
*first* failure. This tool answers the same question in a second, from the
manifest, the closure's own text and `src/`.

It answers in BOTH directions, because the two have different failure modes
and only one of them was previously covered:

  direction 1 -- what a spliced span REFERENCES. `match_c_slice.py` deletes
    the assembly between an entry's start and end markers, so every label in
    that span leaves the link -- including labels the entry never listed. A
    symbol survives only if a closure label outside every spliced span defines
    it, or a promoted entry really provides it. This tool reports the ones
    that satisfy neither.

  direction 2 -- what a manifest entry PROMISES. Every name in an entry's
    `export` array, and its `c_name`, must be DEFINED in `src/`: a function
    body, a `#ifndef __APPLE__` alias declaration, a `HOST_STUB(...)`, an
    absolute symbol an in-body `__asm__` block emits, or a LABEL an in-body
    `__asm__` block emits beside its `.globl` (the interior-label lever --
    `_0802BCF6` and `_0802BCE4` are exported exactly this way). A bare
    `extern` is not a definition and does not count. This direction exists
    because a name whose VMA is *parseable* resolves in the probe and still
    fails the link; three promotions were lost to it before this direction
    existed.

  direction 3 -- what a spliced span DELETES that live code still reads. The
    dual of direction 1, and the one class no branch-operand scan can see:
    `sub_0802BCB4` loads a pool word by name (`ldr r2, _0802BCE4`) that lives
    inside `_0802BCCE`'s span, so splicing that body deletes the label the
    retained `ldr` binds -- invisible here and to `call_audit`,
    `promotion_screen` and `match_c_slice` until the slice link failed one
    slow round later . A deleted label survives only if the entry's
    own `export`/`c_name` redefines it; anything else referenced from outside
    the span's own lines is reported.


What a splice actually provides
-------------------------------
`match_c_slice.py` deletes the assembly between an entry's start and end
markers, so every label in that span leaves the link -- including labels the
entry never listed. The replacement section defines:

  * the agbcc-compiled body,
  * `.globl`/`.thumb_set` for the entry's `c_name` and `export` names, but only
    for those agbcc *actually* aliased to that body (rule 6: a splice can only
    export a spelling the C declared), and
  * the labels the spliced section itself carries.

Reference extraction (`operand_refs`) deliberately counts EVERY operand form,
not just branches: `bl sub_X` and `ldr r2, _X` both read the label at its
address, and only the first was ever scanned. A label definition line is not a
self-reference; declaration directives (`.type`, `.globl`, `.size`, ...)
name symbols without reading them; binding forms (`.set L, e`, `L = e`,
`.thumb_set L, e`) define `L` and read `e`.
    python3 tools/export_audit.py            # report
    python3 tools/export_audit.py --exports  # list every undefined name
    python3 tools/export_audit.py --self-test

Known limits, all of which make this an assistant to the link rather than a
replacement for it:

  * It reads the *source* manifest and the closure text, so it cannot see a
    spelling agbcc invents or a `.thumb_set` that lands in an unexpected
    section. The linker remains the only authority.
  * Branch operands are filtered against a register-name denylist and against
    names defined in the closure, so ordinary interior labels are not reported.
  * Direction 2 sees `src/*.c` only. A name defined anywhere else is genuinely
    not provided to the slice link, so reporting it is correct -- but a future
    generated source tree outside `src/` would have to be added explicitly.
  * Direction 2 is static. It cannot know that agbcc will or will not emit a
    `.thumb_set` for a name in some unexpected section; that is why an
    `entry_vma_twin` is reported as a latent defect rather than a failure.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "tools/matching_slice_functions.json"
REPORT = ROOT / "build/era-corpus/ready-report.json"

LABEL = re.compile(r"^([A-Za-z_][A-Za-z_0-9]*):$")
INCLUDE = re.compile(r'^\s*\.include\s+"([^"]+)"')
BRANCH = re.compile(
    r"\b(?:bl|b|bne|beq|bgt|blt|bge|ble|bhi|bls|bcc|bcs|bpl|bmi|bx|blx)"
    r"\s+([A-Za-z_][A-Za-z_0-9]*)\b")
# An alias DECLARATION, matched on one line only. The previous `[^;]*` in the
# prototype was a real bug, not a style choice: `[^;]` matches a newline, so
# from a function-BODY line the non-greedy prefix could run forward across
# `#ifndef __APPLE__` and claim a later alias declaration, attributing it to
# the body's name. In src/rec35_runtime.c that silently deleted 13 alias keys
# (`_080016940` among them) and made `alias_map()` answer a different question
# than the one asked. Pinned in the self-test.
ALIAS = re.compile(
    r"^[A-Za-z_][^;\n]*?\b([A-Za-z_][A-Za-z_0-9]*)\s*\([^;\n]*\)\s*"
    r'__attribute__\(\(alias\("([A-Za-z_0-9]+)"\)\)\);')

# --- What counts as a C DEFINITION (the export-definition rule) --------------
# A name is provided to the slice link by exactly four syntactic forms in
# `src/`, and by nothing else. The rule was derived from three promotions that
# were lost to a bare `extern` the probe resolved and the linker refused.

# (1) a function BODY. The parameter list may wrap across lines, and the
#     opening brace may sit on the next line (Allman style -- src/runtime_accessors.c
#     `_08001FD0` is the live instance, and a brace-on-the-same-line-only rule
#     reports 23 manifest `c_name`s and their exports as undefined). `[^;{}]`
#     may cross a newline for the parameter list but the `;` of a prototype
#     still blocks, so `extern void f(void);` is not a body.
C_BODY = re.compile(
    r"^[A-Za-z_][A-Za-z_0-9 \t\*]*?\b([A-Za-z_][A-Za-z_0-9]*)\s*"
    r"\([^;{}]*\)\s*\{", re.M)

# Comments are stripped before any definition form is matched, so a
# commented-out body cannot be read as a definition. `src/` carries both
# spellings.
LINE_COMMENT = re.compile(r"//[^\n]*")
BLOCK_COMMENT = re.compile(r"/\*.*?\*/", re.S)

# (3) `HOST_STUB(sig)`, which expands to `extern sig` on the ROM build and to
#     a weak definition on the host build. It is deliberately the ONLY
#     declaration form that counts: the repo uses it to say "this name is not
#     lifted yet and is provided by the assembly closure", which is exactly
#     the promise an `export` array makes. A bare `extern` makes no such
#     promise and must NOT count.
HOST_STUB = re.compile(r"^HOST_STUB\((.*)\)\s*;")

# (4) an absolute symbol an in-source `__asm__` block emits:
#     `__asm__(".globl SaveBumpBase\nSaveBumpBase = 0x030002D8\n")`. The splice
#     carries the body's own text, so such a symbol IS defined for the entry
#     whose body declares it. Four manifest entries export exactly this shape
#     (`OAMAttrTab`, `OAMParamTab`, `SaveBumpBase`, `SaveTrigTblA/B`) and
#     none of them is a function, so a function-only rule flags all of them.
# (4b) the BLOCK shape: `__asm__` [volatile] `(` then one-or-more adjacent C
#     string literals, then `)`. Multi-literal is load-bearing: the live
#     trap-3/trap-7 lever bodies write their asm as one literal per line
#     behind `__asm__ volatile (` (src/sound_seq_follow.c `_0802BCE4`), and
#     the previous single-literal matcher read none of that block -- a
#     section-defined label then reported as `other_vma` ("a name the entry
#     cannot claim"). Adjacent literals concatenate in C, so the scan joins
#     their contents before splitting asm lines. Extended asm with operands
#     (`__asm__("" : : "r"(x))`) deliberately matches nothing: the operand
#     list breaks the literal run, and its template strings define nothing.
ASM_STR = r'"((?:[^"\\]|\\.)*)"'
_ASM_RUN = r'"(?:[^"\\]|\\.)*"'
# Between adjacent literals the C source carries whitespace and `\`
# continuations -- and, in the live `_0802BCE4` lever, `//` rationale comments.
# Those are handled upstream by `_mask_comments`, so the run regex itself only
# bridges whitespace and continuations.
_ASM_GAP = r'[\s\\]*'
ASM_BLOCK = re.compile(
    r'__asm__\s*(?:volatile\s*)?\(\s*(' + _ASM_RUN + r'(?:' + _ASM_GAP + _ASM_RUN + r')*)\s*\)')


def _mask_comments(text: str) -> str:
    """`text` with every comment blanked to spaces, newlines preserved.

    String and character literals are copied verbatim, so an `__asm__` string
    may carry `//`-looking prose without being eaten -- which is what the
    `//`/`/* */` regex strip (used for bodies) cannot promise, and why the
    block scan runs on THIS. Masking rather than deleting keeps every line
    number valid for anything that correlates positions later.

    Comments must not survive into the block scan in either direction: a
    commented-out `__asm__(".globl X ...")` in a doc comment would otherwise
    register X as defined (under-reporting the audit's own failure class), and
    the live `_0802BCE4` lever carries rationale comments BETWEEN its string
    literals whose own quotes ("invalid offset") would be extracted as a
    literal and glue onto the front of the `.globl` line, hiding the pair.
    """
    out: list[str] = []
    i, n = 0, len(text)
    while i < n:
        ch = text[i]
        if ch == '"' or ch == "'":
            quote = ch
            j = i + 1
            while j < n:
                if text[j] == chr(92):
                    j += 2
                    continue
                if text[j] == quote or text[j] == "\n":
                    j += 1
                    break
                j += 1
            out.append(text[i:j])
            i = j
        elif text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join(c if c == "\n" else " " for c in text[i:j]))
            i = j
        else:
            out.append(ch)
            i += 1
    return "".join(out)
ABS_SYMBOL = re.compile(
    r"^\s*\.globl\s+([A-Za-z_][A-Za-z_0-9]*)\s*$")
ABS_ASSIGN = re.compile(r"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*=")
# (5) a LABEL an in-body `__asm__` emits beside its `.globl`: the trap-3 /
# trap-7 lever, where a spliced body redefines an interior span label inside
# its own section (`__asm__(".globl _0802BCF6\n_0802BCF6:")`). Both parts are
# required -- `_export_missing` admits a name the section defines only through
# its `.globl`, and a bare label would be refused or, worse, re-bound.
ASM_LABEL = re.compile(r"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*:")

# --- reference extraction (direction 3) ------------------------------------
# A label DEFINITION, `name:` at line start with anything after it. A combined
# `L: .word 0x080614E0` line defines L and reads whatever follows the mnemonic.
LABEL_PREFIX = re.compile(r"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*:")
# Directives that NAME a symbol without reading it at its address.
NAME_DECL_RE = re.compile(
    r"^\.(?:type|size|globl|global|weak|thumb_func|section|include|file|"
    r"syntax|eabi_attribute|fnstart|fnend|cantunwind|mov_v|force_thumb|"
    r"ltorg|pool|align|balign|even|end)\b")
# Binding forms: the LHS is defined, the RHS is read. `.thumb_set X, Y` binds
# X to Y, so Y is a genuine reference to whatever Y denotes.
BIND_DIR_RE = re.compile(
    r"^\.(?:set|thumb_set|equ|equiv)\s+([A-Za-z_][A-Za-z_0-9]*)\s*,\s*(.*)$")
BIND_EQ_RE = re.compile(r"^([A-Za-z_][A-Za-z_0-9]*)\s*=\s*(.*)$")
# Numeric literals never name a symbol; stripping them first keeps
# `0x080614E0` from yielding the identifier `x080614E0`.
HEX_RE = re.compile(r"\b0[xX][0-9A-Fa-f]+\b|\b\d+\b")
IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z_0-9]*")

# `static`/`static inline` definitions are file-local: agbcc emits no external
# symbol for them, so they cannot satisfy an `export` entry.
STATIC = re.compile(r"^static\b")
# `bx lr` and friends. A branch operand is not a symbol (rule 9).
NOT_SYMBOLS = {
    "lr", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "sp", "pc", "ip", "sl", "fp",
}


def closure_files() -> list[Path]:
    """The slice's include closure, rooted at `asm/code.s` (rule 1)."""
    seen: set[Path] = set()
    stack = [ROOT / "asm/code.s"]
    out: list[Path] = []
    while stack:
        path = stack.pop()
        if path in seen or not path.exists():
            continue
        seen.add(path)
        out.append(path)
        for line in path.read_text(errors="replace").splitlines():
            match = INCLUDE.match(line)
            if match:
                stack.append(path.parent / match.group(1))
    return out


def src_files() -> list[Path]:
    """The C translation units the slice link sees, in a stable order.

    This is the WHOLE audit scope. A name defined outside it is genuinely not
    provided to the link, so reporting one as missing is correct rather than a
    false positive -- but the scope must be explicit, because widening it
    silently would turn a real defect into a green result.
    """
    return sorted(ROOT.glob("src/*.c"))


def alias_map(files: list[Path] | None = None) -> dict[str, set[str]]:
    """Every `name -> {bodies it is aliased to}` declared in `src/`."""
    out: dict[str, set[str]] = {}
    for path in files if files is not None else src_files():
        for line in path.read_text(errors="replace").splitlines():
            match = ALIAS.match(line)
            if match and not STATIC.match(line):
                out.setdefault(match.group(1), set()).add(match.group(2))
    return out


def name_vma(name: str) -> int | None:
    """The ROM VMA a C symbol name encodes, or None for a friendly name.

    Deliberately the repo's own convention, copied from
    `match_c_slice._name_vma` rather than re-derived: strip an optional
    `sub_`, drop leading zeros, require the remainder to start `80`, then take
    the low six nibbles. That is what makes `sub_08000ED68` and `_0800ED68`
    the same name here -- they are the same name to nm, to the promotion
    screen and to `match_c_slice._export_missing`, and a stricter rule here
    would flag rows the link is entitled to synthesise. Being no MORE
    permissive is the property that matters, so the adversarial cases (a
    friendly name, a non-`80` address, a 7-nibble remainder) are pinned in
    the self-test.
    """
    match = re.search(r"(?:sub_)?_?([0-9A-Fa-f]{6,10})\b", name)
    if not match:
        return None
    digits = match.group(1).lstrip("0").lower()
    if not digits.startswith("80"):
        return None
    digits = digits[2:]
    if len(digits) < 4 or len(digits) > 6:
        return None
    return 0x08000000 | int(digits.zfill(6), 16)


def _stub_name(signature: str) -> str | None:
    """The function named by a `HOST_STUB(sig)` argument."""
    depth = 0
    for i, ch in enumerate(signature):
        if ch == "(":
            if depth == 0:
                head = re.search(r"([A-Za-z_][A-Za-z_0-9]*)\s*$", signature[:i])
                return head.group(1) if head else None
            depth += 1
        elif ch == ")":
            depth -= 1
    return None


def operand_refs(lines: list[str]) -> dict[str, set[int]]:
    """Name -> indices of lines whose OPERANDS read that name at its address.

    Every operand form counts -- branches, `ldr`/`adr` literal operands, data
    words (`.word sub_X`), binding RHSes -- because all of them dangle when the
    label they name is spliced away. near-miss was exactly the gap:
    `BRANCH` sees `bl sub_X` and not `ldr r2, _0802BCE4`.

    Deliberately over-approximate and then filtered by the callers against the
    names they care about: a stray identifier that is not a closure label can
    never be reported, and under-approximating is the failure mode this exists
    to remove. Not counted: `@`-comment prose, register names, numeric
    literals, a label's own definition line, and declaration directives
    (`.type`/`.globl`/`.size` name a symbol; they do not read it).
    """
    out: dict[str, set[int]] = {}
    for i, raw in enumerate(lines):
        code = raw.split("@", 1)[0]
        head = LABEL_PREFIX.match(code)
        rest = code[head.end():] if head else code
        rest = rest.strip()
        if not rest:
            continue
        rhs: str | None
        binding = BIND_DIR_RE.match(rest) or BIND_EQ_RE.match(rest)
        if binding:
            rhs = binding.group(2)
        elif NAME_DECL_RE.match(rest):
            continue
        else:
            parts = rest.split(None, 1)
            rhs = parts[1] if len(parts) > 1 else ""
        for ident in IDENT_RE.findall(HEX_RE.sub(" ", rhs)):
            if ident not in NOT_SYMBOLS:
                out.setdefault(ident, set()).add(i)
    return out


def exterior_refs(refs: dict[str, set[tuple[Path, int]]], deleted: set[str],
                  owner: Path, bounds: tuple[int, int],
                  provided: set[str]) -> dict[str, list[tuple[Path, int]]]:
    """Among `deleted`, the names a line OUTSIDE the span still reads.

    `bounds` is the half-open replaced region `[start, end)` of `owner` --
    exactly `match_c_slice.replace_body`'s region. A reference inside it dies
    with the span and dangles nothing; one anywhere else (retained asm in the
    same file, or any other closure file) binds against a label the splice
    deletes, unless the entry's own `export`/`c_name` redefines it.
    """
    out: dict[str, list[tuple[Path, int]]] = {}
    for name in sorted(deleted):
        if name in provided:
            continue

        sites = sorted((p, i) for p, i in refs.get(name, ())
                       if not (p == owner and bounds[0] <= i < bounds[1]))
        if sites:
            out[name] = sites
    return out


def mine_names(entry: dict, body: str, aliases: dict[str, set[str]]) -> set[str]:
    """Every spelling the deleting entry's own C defines at the deleted address.

    Its `c_name`, its `export` list, its body, and every `alias(body)`
    declaration in `src/`. The alias spelling is not optional: retained asm
    reads `sub_0802B64C` in three places and the entry provides it as
    `__attribute__((alias("_0802B64C")))` (src/runtime_record_helpers.c), which the
    splice binds at the body's address. An alias of a DIFFERENT body is not
    this entry's provision, so the membership test is on `body`.
    """
    mine = {entry["c_name"], *entry.get("export", ()), body}
    mine |= {n for n, bodies in aliases.items() if body in bodies}
    return mine


def scan_definitions(text: str, forms: dict[str, set[str]] | None = None) -> dict[str, set[str]]:
    """Add every definition form in one C translation unit to `forms`.

    Kept text-in / dict-out so the self-test can exercise it on fixtures
    without writing files, and so `c_definitions` stays a plain loop.

    A body is matched against the whole comment-stripped text rather than one
    line, because its brace may sit on the next line (Allman style, which
    src/runtime_accessors.c `_08001FD0` uses) and its parameter list may wrap. The
    `;` of a prototype still blocks the match, so `extern void f(void);` is
    not a body. Aliases and `HOST_STUB`s stay line-anchored: both end in `;`
    and must never run into the next line, which is the bug that `ALIAS`'s
    comment records.
    """
    forms = forms if forms is not None else {
        "body": set(), "alias": set(), "stub": set(), "abs": set(),
        "section_label": set()}
    code = BLOCK_COMMENT.sub(" ", text)
    code = LINE_COMMENT.sub("", code)
    for line in code.splitlines():
        match = ALIAS.match(line)
        if match:
            if not STATIC.match(line):
                forms["alias"].add(match.group(1))
            continue
        match = HOST_STUB.match(line)
        if match:
            name = _stub_name(match.group(1))
            if name:
                forms["stub"].add(name)
    for match in C_BODY.finditer(code):
        if not STATIC.match(match.group(0).lstrip()):
            forms["body"].add(match.group(1))
    # Read from comment-MASKED text: an `__asm__` string may contain a `//`-like
    # sequence that the body's comment stripping would eat (so not `code`), but
    # comments outside strings must still vanish -- see `_mask_comments`.
    for block in ASM_BLOCK.finditer(_mask_comments(text)):
        declared: set[str] = set()
        labelled: set[str] = set()
        joined = "".join(m.group(1) for m in re.finditer(ASM_STR, block.group(1)))
        for asm_line in joined.replace("\\n", "\n").splitlines():
            glob = ABS_SYMBOL.match(asm_line)
            if glob:
                declared.add(glob.group(1))
                continue
            assign = ABS_ASSIGN.match(asm_line)
            if assign:
                if assign.group(1) in declared:
                    forms["abs"].add(assign.group(1))
                continue
            label = ASM_LABEL.match(asm_line)
            if label:
                labelled.add(label.group(1))
        # `.globl L` + `L:` is the interior-label lever; the pair is
        # order-independent because the two lines are written in whichever
        # order the transcription happens to carry them.
        forms["section_label"] |= declared & labelled
    return forms


def c_definitions(files: list[Path] | None = None) -> dict[str, set[str]]:
    """Every name `src/` DEFINIES, mapped to the forms that define it.

    The five keys are the rule stated in the module docstring; the sets are
    unioned by `is_defined`. A name absent from all five is not provided to
    the link, whatever its spelling resembles.
    """
    forms: dict[str, set[str]] = {
        "body": set(), "alias": set(), "stub": set(), "abs": set(),
        "section_label": set()}
    for path in files if files is not None else src_files():
        scan_definitions(path.read_text(errors="replace"), forms)
    return forms


def is_defined(name: str, forms: dict[str, set[str]]) -> bool:
    return any(name in names for names in forms.values())


def undefined_exports(manifest: dict, forms: dict[str, set[str]]) -> list[dict]:
    """Manifest names that no `src/` definition provides, with a verdict.

    Two distinct outcomes are separated here, because they need different
    responses and conflating them is what made this class invisible:

      * `unresolved` -- the name has no definition and carries no VMA the
        entry can claim. `match_c_slice._export_missing` REFUSES to invent an
        alias for it and raises, so the slice link fails outright.
      * `entry_vma_twin` -- the name encodes the entry's OWN address (a
        9-digit `sub_080022CB4` beside an 8-digit `_08022CB4` is the classic
        shape). `match_c_slice` synthesises `.globl`/`.thumb_set` for it and
        the link succeeds, which is exactly why it is dangerous: the manifest
        promises a spelling the C never declared, so any promoted caller that
        binds to it reaches the ROM body through a veneer, or fails closed in
        `build_c.py`'s trampoline gate.
    """
    rows = []
    for vma, entry in sorted(manifest.items()):
        want = list(entry.get("export", ()))
        if entry.get("c_name"):
            want.append(entry["c_name"])
        for name in want:
            if is_defined(name, forms):
                continue
            want_vma = name_vma(name)
            entry_vma = int(vma, 16)
            rows.append({
                "vma": vma, "name": name, "c_name": entry["c_name"],
                "is_c_name": name == entry["c_name"],
                "verdict": ("entry_vma_twin" if want_vma == entry_vma
                            else "unresolved" if want_vma is None
                            else "other_vma"),
            })
    return rows


def span(entry: dict, lines: list[str]) -> tuple[set[str], set[str], tuple[int, int]] | None:
    """(labels deleted, branch operands, replaced line bounds) for one entry.

    None if the markers are unusable. `bounds` is the half-open replaced
    region `[start, end)` -- `match_c_slice.replace_body`'s region -- so
    direction 3 can tell a reference that dies with the span from one that
    outlives it. A combined `L: .word ...` line DELETES `L`, so deletion keys
    on the label prefix, not on a bare `LABEL:` line.
    """
    def at(marker: str) -> list[int]:
        return [i for i, l in enumerate(lines) if l.strip() == marker]

    start, end = at(entry["start_marker"]), at(entry["end_marker"])
    if len(start) != 1 or len(end) != 1 or start[0] >= end[0]:
        return None                      # ambiguous or non-monotonic marker
    deleted, callees = set(), set()
    for i in range(start[0], end[0]):
        # Strip comments before reading the line. A branch operand inside an
        # `@` comment is prose, not a reference: `_0800B190` appears only in
        # comments in asm/carphys_tick.s and was reported as a false positive.
        code = lines[i].split("@", 1)[0]
        label = LABEL_PREFIX.match(code)
        if label:
            deleted.add(label.group(1))
        callees |= {n for n in BRANCH.findall(code) if n not in NOT_SYMBOLS}
    return deleted, callees, (start[0], end[0])


def audit() -> tuple[list[tuple[str, list[str]]], list[dict], int, list[dict]]:
    """(direction-1 problems, direction-3 rows, entry count, undefined names)."""
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    by_name: dict[str, dict] = {}
    if REPORT.exists():
        by_name = {r["name"]: r for r in
                   json.loads(REPORT.read_text(encoding="utf-8"))["results"]}
    aliases = alias_map()
    files = closure_files()
    by_file = {p: p.read_text(errors="replace").splitlines() for p in files}
    closure_labels = {LABEL.match(l.strip()).group(1)
                      for lines in by_file.values() for l in lines
                      if LABEL.match(l.strip())}

    def body_of(entry: dict) -> str:
        rec = by_name.get(entry["c_name"])
        return (rec.get("alias_of") if rec else None) or entry["c_name"]

    # A name an entry really provides: its body, plus each owned spelling the C
    # aliased to that body (rule 6 -- otherwise agbcc wrote no .thumb_set).
    provided: set[str] = set()
    owners: dict[str, str] = {}
    for vma, entry in manifest.items():
        body = body_of(entry)
        provided.add(body)
        owners.setdefault(body, vma)
        for name in [entry["c_name"], *entry.get("export", ())]:
            if name == body or body in aliases.get(name, ()):
                provided.add(name)
                owners.setdefault(name, vma)

    spans: dict[str, tuple[set[str], set[str], tuple[int, int]]] = {}
    for vma, entry in manifest.items():
        lines = by_file.get(ROOT / entry["asm_file"])
        if lines is None:
            continue
        got = span(entry, lines)
        if got:
            spans[vma] = got

    # A closure label is lost when the only spans defining it are spliced away.
    deleted_by: dict[str, set[str]] = {}
    for vma, (deleted, _, _) in spans.items():
        for name in deleted:
            deleted_by.setdefault(name, set()).add(vma)

    problems: dict[str, list[str]] = {}
    for vma, (_deleted, callees, _bounds) in spans.items():
        for name in callees:
            if name in provided:
                continue
            if name in closure_labels and not (deleted_by.get(name, set()) & set(spans)):
                continue                  # survives in untouched assembly
            if name in closure_labels and deleted_by.get(name, set()) <= set(spans):
                # defined only inside spliced spans: survives only if an entry
                # provides it, already checked above.
                continue
            problems.setdefault(name, []).append(vma)

    # Direction 3: every operand-form reference in the closure, then the
    # deleted-but-still-read names per span. `provided` is per-entry here, not
    # global: only the deleting entry's own export/c_name redefines the label
    # at the deleted address.
    refs: dict[str, set[tuple[Path, int]]] = {}
    for path, lines in by_file.items():
        for name, idxs in operand_refs(lines).items():
            for i in idxs:
                refs.setdefault(name, set()).add((path, i))
    dangling: list[dict] = []
    for vma, entry in sorted(manifest.items()):
        got = spans.get(vma)
        if not got:
            continue
        deleted, _callees, bounds = got
        owner = ROOT / entry["asm_file"]
        mine = mine_names(entry, body_of(entry), aliases)
        for name, sites in exterior_refs(refs, deleted, owner, bounds, mine).items():
            dangling.append({
                "vma": vma, "name": name,
                "refs": [[str(p.relative_to(ROOT)), i + 1] for p, i in sites],
            })
    return sorted(problems.items()), dangling, len(manifest), undefined_exports(
        manifest, c_definitions())


def self_test() -> int:
    ok = total = 0

    def check(name: str, got, want) -> None:
        nonlocal ok, total
        total += 1
        if got == want:
            ok += 1
        else:
            print(f"FAIL {name}: got {got!r} want {want!r}", file=sys.stderr)

    check("closure has code.s", (ROOT / "asm/code.s") in closure_files(), True)
    check("closure excludes dead file", (ROOT / "asm/__DELETED_GHOST2_TEST_FALLBACK__.s") in closure_files(), False)
    lines = ["@ x", ".type sub_0800AAAA, %function", "sub_0800AAAA:",
             "movs r0, r0", "bl _0800BBBB", "bx lr",
             ".type sub_0800AAB4, %function", "sub_0800AAB4:"]
    got = span({"start_marker": ".type sub_0800AAAA, %function",
                "end_marker": ".type sub_0800AAB4, %function"}, lines)
    check("span labels deleted", got[0], {"sub_0800AAAA"})
    check("branch operand collected", got[1], {"_0800BBBB"})
    check("bx lr is not a symbol", "lr" in got[1], False)
    check("branch in an @ comment is not a reference",
          "_0800B190" in span(
              {"start_marker": ".type sub_0800AAAA, %function",
               "end_marker": ".type sub_0800AAB4, %function"},
              lines[:5] + ["\t@ prose naming _0800B190"] + lines[5:])[1],
          False)
    dup = lines + ["@ x"]
    check("ambiguous start marker rejected",
          span({"start_marker": "@ x", "end_marker": "sub_0800AAB4:"}, dup), None)
    check("non-monotonic markers rejected",
          span({"start_marker": ".type sub_0800AAB4, %function",
                "end_marker": ".type sub_0800AAAA, %function"}, lines), None)

    # --- the export-definition rule ---------------------------------------
    # `name_vma` decides whether the splice may synthesise the binding, so it
    # must not be stricter than match_c_slice._name_vma -- or looser. Both the
    # permissive cases (9-digit and 8-digit spellings of one address) and the
    # refusals are pinned.
    check("name_vma: 9-digit and 8-digit spellings agree",
          name_vma("sub_080022CB4") == name_vma("_08022CB4"), True)
    check("name_vma: follows the repo's own sub_ convention",
          (name_vma("sub_0800166E8"), name_vma("_080166E8")),
          (0x080166E8, 0x080166E8))
    check("name_vma: a friendly name encodes nothing",
          name_vma("Course_State_Store10"), None)
    check("name_vma: a non-ROM address encodes nothing", name_vma("_02003F00"), None)
    check("name_vma: an over-long address is refused", name_vma("_0800123456"), None)

    # The four definition forms, one positive each. A rule built only on the
    # alias form misses three of them; that mistake was made, so each is
    # pinned separately rather than as one aggregate.
    forms = scan_definitions(
        "int _BodyForm(int a)\n{\n    return a;\n}\n"          # Allman brace
        "void _080019AEC(int mode) __attribute__((alias(\"_BodyForm\")));\n"
        "HOST_STUB(void _StubForm(void *r, int i));\n"
        "void _AbsForm(void)\n{\n"
        "    __asm__(\".globl SaveTbl\\nSaveTbl = 0x030002D8\\n\");\n"
        "}\n")
    check("a function body is a definition (brace on the next line)",
          "_BodyForm" in forms["body"], True)
    check("an alias declaration is a definition",
          "_080019AEC" in forms["alias"], True)
    check("a HOST_STUB is a definition", "_StubForm" in forms["stub"], True)
    check("an in-body __asm__ absolute symbol is a definition",
          "SaveTbl" in forms["abs"], True)

    # (5) the section-label lever: `.globl L` + `L:` in one __asm__ block is
    # how a spliced body redefines an interior span label (
    # `_0802BCF6`, `_0802BCE4`). Both halves are required, pinned separately:
    # a `.globl` alone is refused by `_export_missing`, and a label alone is
    # invisible to it, so counting either alone would validate a spelling the
    # splice does not actually bind.
    esc = chr(92) + "n"          # the two characters of a C string literal's newline escape
    lever = scan_definitions(
        "void _LeverBody(void)\n{\n"
        '    __asm__(".globl _0802BCF6' + esc + '_0802BCF6:' + esc + '");\n'
        "}\n"
        "void _HalfGlob(void)\n{\n"
        '    __asm__(".globl _0800AAAA' + esc + '");\n'
        "}\n"
        "void _HalfLabel(void)\n{\n"
        '    __asm__("_0800BBBB:' + esc + '");\n'
        "}\n")
    check("a .globl + label pair is a section_label definition",
          "_0802BCF6" in lever["section_label"], True)
    check("a .globl without the label is NOT",
          "_0800AAAA" in lever["section_label"], False)
    check("a label without the .globl is NOT",
          "_0800BBBB" in lever["section_label"], False)
    check("and the pair is a definition for is_defined",
          is_defined("_0802BCF6", lever), True)

    # The LIVE lever shape: `__asm__ volatile (` with one C string per asm
    # line (src/sound_seq_follow.c `_0802BCE4`). The previous single-literal
    # matcher read none of this block, so the section-defined label reported
    # as `other_vma` ("a name the entry cannot claim") in the live audit.
    # Built from chr() joins so no escape in this file can mangle the
    # fixture's own backslashes.
    NL = chr(10)
    vol = scan_definitions(
        "void _VolLever(void)" + NL + "{" + NL +
        "    __asm__ volatile (" + NL +
        '        ".syntax unified' + esc + '"' + NL +
        '        ".globl _0802BCE4' + esc + '"' + NL +
        '        "_0802BCE4:' + esc + '"' + NL +
        '        ".word 0x080614E0' + esc + '"' + NL +
        "    );" + NL + "}" + NL)
    check("a volatile multi-literal __asm__ block is one block",
          "_0802BCE4" in vol["section_label"], True)

    # The live block's rationale comments sit BETWEEN literals, one holding
    # quote characters. If the gap cannot swallow them the run splits mid-block
    # and the lever's `.globl`+label pair is never seen -- the exact way the
    # first widening still left `_0802BCE4` reading `other_vma` live.
    volc = scan_definitions(
        "void _VolComment(void)" + NL + "{" + NL +
        "    __asm__ volatile (" + NL +
        '        "push {r0}' + esc + '"' + NL +
        '        // rationale ("invalid offset") naming _0802FAKE' + NL +
        '        ".globl _0802BCE4' + esc + '"' + NL +
        '        "_0802BCE4:' + esc + '"' + NL +
        "    );" + NL + "}" + NL)
    check("comments between literals do not split the block",
          "_0802BCE4" in volc["section_label"], True)
    check("a name inside a rationale comment is not a definition",
          ("_0802FAKE" in volc["section_label"], "_0802FAKE" in volc["abs"]),
          (False, False))
    check("extended asm with operands defines nothing",
          scan_definitions(
              "void _Ext(void)" + NL + "{" + NL +
              '    __asm__("" : : "r"(x) : "memory");' + NL + "}" + NL
          )["section_label"],
          set())

    # --- operand_refs: every operand form, not just branches ---------------
    # The near-miss was `ldr r2, _0802BCE4` -- a literal-pool operand
    # no BRANCH regex sees. Pin the accept class AND the noise class, so the
    # extractor cannot drift toward either a branch-only scan or a grep.
    ref_lines = [
        "\tbl _0800BBBB",                    # branch operand -> reference
        "\tldr r2, _0800BC04",               # literal-pool operand -> reference
        "\tbl 0x0802BCCE",                   # a numeric operand binds no symbol
        "\tbx lr",                           # a register is not a symbol
        "_0800DDDD:",                        # a definition is not a self-read
        "_0800EEEE: .word 0x080614E0",       # defines EEEE; the word is a value
        "_0800FFFF: .word _0800BBBB",        # defines FFFF and reads BBBB
        "\t@ bl _0800CCCC",                  # comment prose
        ".type sub_0800GGGG, %function",     # declarations name, not read
        ".globl _0800HHHH",
        ".size sub_0800GGGG, .-sub_0800GGGG",
        ".thumb_set _0800IIII, _0800BBBB",   # binding RHS is read, LHS defined
    ]
    refs = operand_refs(ref_lines)
    check("operand_refs: a branch operand is a reference",
          0 in refs.get("_0800BBBB", ()), True)
    check("operand_refs: an ldr literal operand is a reference",
          1 in refs.get("_0800BC04", ()), True)
    check("operand_refs: a data-word operand is a reference",
          6 in refs.get("_0800BBBB", ()), True)
    check("operand_refs: a binding RHS is a reference",
          11 in refs.get("_0800BBBB", ()), True)
    check("operand_refs: a label line is not a self-reference",
          refs.get("_0800DDDD", set()), set())
    check("operand_refs: ...and a combined label line reads only its operand",
          (5 in refs.get("_0800EEEE", set()), 6 in refs.get("_0800FFFF", set())),
          (False, False))
    check("operand_refs: a numeric operand is not a reference",
          refs.get("_0802BCCE", set()), set())
    check("operand_refs: comment prose is not a reference",
          refs.get("_0800CCCC", set()), set())
    check("operand_refs: declaration directives name but do not read",
          (refs.get("_0800GGGG", set()), refs.get("_0800HHHH", set())),
          (set(), set()))
    check("operand_refs: a binding LHS is defined, not read",
          refs.get("_0800IIII", set()), set())

    # --- exterior_refs: the pair of shapes, and the control --------
    owner = Path("asm/x.s")
    elsewhere = Path("asm/y.s")
    interior_refs = {
        # `_0802BCF6`: read from ANOTHER span's lines (BD44's two b.n tails)
        "_0802BCF6": {(elsewhere, 10)},
        # `_0802BCE4`: read by retained asm below the span (BCB4's ldr)
        "_0802BCE4": {(owner, 99)},
        # `_0802BE74`: read only from inside the deleting span
        "_0802BE74": {(owner, 21)},
    }
    got = exterior_refs(interior_refs, set(interior_refs), owner, (20, 30), set())
    check("a cross-span branch read is exterior",
          "_0802BCF6" in got, True)
    check("a retained ldr read is exterior",
          "_0802BCE4" in got, True)
    check("a pool label read only inside its own span is NOT exterior",
          "_0802BE74" in got, False)
    check("an export name redefining the label silences it",
          exterior_refs(interior_refs, set(interior_refs), owner, (20, 30),
                        {"_0802BCE4"}),
          {"_0802BCF6": [(elsewhere, 10)]})

    # `mine` must recognise a C `alias(body)` spelling: the live `sub_0802B64C`
    # reads bind through src/runtime_record_helpers.c's alias at the body's address, so
    # counting only c_name/export/body reported them as dangling (first real
    # run of direction 3). An alias of some OTHER body is not this entry's
    # provision.
    aliases = {"sub_0802B64C": {"_0802B64C"}, "sub_Other": {"_08001000"}}
    mine = mine_names({"c_name": "_0802B64C", "export": ["_0802B64E"]},
                      "_0802B64C", aliases)
    check("an alias spelling of the body is provided by the entry",
          "sub_0802B64C" in mine, True)
    check("the export spelling counts as before",
          "_0802B64E" in mine, True)
    check("an alias of another body is NOT this entry's provision",
          "sub_Other" in mine, False)

    # Negative control: the declaration forms that are NOT definitions. A bare
    # `extern` is the trap -- it looks like a definition to a name-based grep
    # and provides the link nothing.
    bare = scan_definitions(
        "extern void _NotDefined(int v);\n"
        "extern void _AlsoNot(int v);   // 0x08001000 closure spelling\n"
        "static void _FileLocal(void) {}\n")
    check("a bare extern declaration is NOT a definition",
          is_defined("_NotDefined", bare), False)
    check("an extern with a closure hint is NOT a definition",
          is_defined("_AlsoNot", bare), False)
    check("a static definition is NOT an exported definition",
          is_defined("_FileLocal", bare), False)

    # The regex regression: `[^;]*` matched a newline, so an alias declaration
    # following a body was attributed to the BODY's name and the alias key was
    # lost. src/rec35_runtime.c lines 338-341 are the live instance.
    cross = scan_definitions(
        "void Rec35_BxLr_16940(void) {}\n"
        "#ifndef __APPLE__\n"
        "void _080016940(void) __attribute__((alias(\"Rec35_BxLr_16940\")));\n"
        "void sub_080016940(void) __attribute__((alias(\"Rec35_BxLr_16940\")));\n"
        "#endif\n")
    check("an alias after a body is not attributed to the body",
          "Rec35_BxLr_16940" in cross["alias"], False)
    check("...and the alias key itself is recovered",
          sorted(cross["alias"]), ["_080016940", "sub_080016940"])

    # THE REGRESSION. Two of the three promotions lost to the export-twin
    # class, reconstructed from the state before commit f22d27e: each manifest
    # entry paired with the C that existed then. The third (`_080019AEC`) was
    # never a manifest defect and is pinned separately below.
    pre_fix = {
        # the manifest had exported sub_08008098 for multiple source revisions; C defined
        # only the `_` and `Sub_` twins.
        "0x08008098": {"c_name": "_08008098",
                       "export": ["_08008098", "sub_08008098"]},
        # the closure spells 0x080022CB4 with NINE hex digits; C defined the
        # 8-digit twins, a different symbol at the same address.
        "0x08022cb4": {"c_name": "_08022CB4",
                       "export": ["_080022CB4", "sub_080022CB4"]},
    }
    pre_src = (
        "void Course_State_Store10(u16 a) { (void)a; }\n"
        "#ifndef __APPLE__\n"
        "void _08008098(u16 a) __attribute__((alias(\"Course_State_Store10\")));\n"
        "void Sub_08008098(u16 a) __attribute__((alias(\"Course_State_Store10\")));\n"
        "#endif\n"
        "void CarRecEventReset_22CB4(void *a, void *b) { (void)a; (void)b; }\n"
        "#ifndef __APPLE__\n"
        "void _08022CB4(void *a, void *b) __attribute__((alias(\"CarRecEventReset_22CB4\")));\n"
        "void sub_08022CB4(void *a, void *b) __attribute__((alias(\"CarRecEventReset_22CB4\")));\n"
        "#endif\n")
    rows = undefined_exports(pre_fix, scan_definitions(pre_src))
    check("REGRESSION: every undefined pre-fix export name is reported",
          sorted(r["name"] for r in rows),
          ["_080022CB4", "sub_080022CB4", "sub_08008098"])
    check("REGRESSION: all three are entry-VMA twins, not link refusals",
          sorted({r["verdict"] for r in rows}), ["entry_vma_twin"])
    # ...and the negative control on the same fixture: what WAS defined then
    # must not be reported. `_08008098` sits at the same address as the
    # missing `sub_08008098`, and `Sub_08008098` is a third spelling of it, so
    # a rule keyed on the ADDRESS rather than on the spelling would pass all
    # three -- which is the exact mistake the 9-digit instance exposes.
    reported = [r["name"] for r in rows]
    check("negative control: a defined body is not reported",
          reported.count("Course_State_Store10"), 0)
    check("negative control: the defined 8-digit twins are not reported",
          reported.count("_08022CB4"), 0)
    check("negative control: _08008098 was defined then and is not reported",
          reported.count("_08008098"), 0)

    # The third instance was never a manifest defect: 0x08019aec is not a
    # manifest entry, so `_080019AEC` belongs to the CALLER direction. Pinned so
    # a future change cannot quietly reclassify it as an export problem.
    live_manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    check("_080019AEC is not an export of any manifest entry",
          "_080019AEC" in {n for e in live_manifest.values()
                           for n in e.get("export", ())}, False)

    # Scope control: the audit scans `src/*.c` and nothing else. Widening that
    # silently would make the audit blind, so the boundary is pinned, and the
    # live scan is asserted against a name the source definition repaired.
    live = c_definitions()
    live_rows = undefined_exports(live_manifest, live)
    check("the audit scope is src/*.c",
          all(p.parent.name == "src" for p in src_files()), True)
    check("live: sub_08008098 was repaired and is no longer reported",
          [r["name"] for r in live_rows if r["name"] == "sub_08008098"], [])
    # `_080019AEC` is the caller-direction gap that this check CANNOT see: it is
    # declared `extern` in src/race_scene.c and defined nowhere, and no manifest
    # entry exports it because 0x08019aec is not a promoted entry at all.
    check("live: _080019AEC has no C definition (caller direction, not this check)",
          is_defined("_080019AEC", live), False)
    check("live: ...but its 8-digit twin IS defined",
          is_defined("_08019AEC", live), True)
    print(f"export_audit self-test: {ok}/{total}")
    return 0 if ok == total else 1


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--exports", action="store_true",
                        help="list every undefined manifest name, not just "
                             "the ones the link would refuse")
    args = parser.parse_args()
    if args.self_test:
        return self_test()
    problems, dangling, entries, exports = audit()
    print(f"manifest entries: {entries}")

    print("\n== direction 1: symbols a spliced span REFERENCES ==")
    print(f"provided by nothing: {len(problems)}")
    for name, vmas in problems:
        print(f"  {name:<20} from {len(vmas)} span(s), e.g. {vmas[0]}")

    print("\n== direction 3: labels a span DELETES that live code still reads ==")
    print(f"dangling (export does not redefine them): {len(dangling)}")
    for row in dangling:
        where = ", ".join(f"{p}:{i}" for p, i in row["refs"][:3])
        print(f"  {row['vma']} {row['name']:<20} read at {where}")

    print("\n== direction 2: manifest names with NO C definition ==")
    buckets: dict[str, list[dict]] = {}
    for row in exports:
        buckets.setdefault(row["verdict"], []).append(row)
    print(f"unresolved (match_c_slice would REFUSE to bind): "
          f"{len(buckets.get('unresolved', ()))}")
    for row in buckets.get("unresolved", ()):
        print(f"  {row['vma']} {row['name']}"
              f"{' (c_name)' if row['is_c_name'] else ''}")
    print(f"other_vma (a name the entry cannot claim): "
          f"{len(buckets.get('other_vma', ()))}")
    for row in buckets.get("other_vma", ()):
        print(f"  {row['vma']} {row['name']} (entry VMA {name_vma(row['name'])})")
    twins = buckets.get("entry_vma_twin", ())
    print(f"entry_vma_twin (link synthesises it; manifest-only spelling, and "
          f"the export-twin trap): {len(twins)} in "
          f"{len({row['vma'] for row in twins})} entries")
    if args.exports:
        for row in twins:
            print(f"  {row['vma']} {row['name']}"
                  f"{' (c_name)' if row['is_c_name'] else ''}")
    print("\n(the linker remains the authority; see this tool's Known limits)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
