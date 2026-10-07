#!/usr/bin/env python3
"""Compiler microscope: diagnostics for an unwanted instruction decision.

Answer one question: WHICH compiler pass makes an unwanted instruction decision?

Method: compile two controlled source variants of the same body with the pinned
agbcc and its own RTL dumps enabled, parse a small subset of each dump, and
report the first pass (in a fixed, documented order) at which the two variants'
normalized operation ordering or register use diverges.

Scope limits, both deliberate:
  * Diagnostic wrapper only. The "instrumented agbcc" second version of the
    strategy document is a separate project and is NOT implemented here.
  * This is an observation about THESE TWO VARIANTS. It is not a reconstruction
    of the original compilation of a ROM body: the original source is unknown,
    so the tool compares a candidate against a sibling candidate, never against
    history.

The `-d<letter>` set below was verified against the installed toolchain
(`build/toolchains/agbcc/agbcc`, gcc 2.9) by compiling a scratch body once per
letter and listing the emitted files. A letter that is not in the table is
refused with kit.Unsupported rather than silently producing no dump: an empty
dump set is indistinguishable from a pass that did nothing, and this tool's
whole output is a claim about what a pass did.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import experiment_kit as kit
import corpus_match_probe as probe

STRATEGY = "compiler-microscope"

# --------------------------------------------------------------------------
# Dump letters. (letter, suffix, pass name) in the tool's default comparison
# order, which follows the GCC 2.x pass pipeline as this compiler emits them:
# combine runs inside expand, then expand's RTL dump, the two CSE sweeps, regmove
# between the local and global allocators, flow analysis 1, gcse, loop analysis,
# the two jump passes, the final machine RTL, and addressof.
#
# The order is a claim about the toolchain and is overridable with --passes;
# the raw dumps are always kept so a caller can re-order without recompiling.
# --------------------------------------------------------------------------
PASSES: list[tuple[str, str, str]] = [
    ("c", "combine", "combine"),
    ("r", "rtl", "expand (rtl)"),
    ("s", "cse", "cse1"),
    ("t", "cse2", "cse2"),
    ("N", "regmove", "regmove"),
    ("l", "lreg", "local allocator"),
    ("g", "greg", "global allocator"),
    ("f", "flow", "flow1"),
    ("G", "gcse", "gcse"),
    ("L", "loop", "loop (flow2)"),
    ("j", "jump", "jump"),
    ("J", "jump2", "jump2"),
    ("M", "mach", "machine rtl"),
]
DEFAULT_LETTERS = "".join(letter for letter, _, _ in PASSES)

# `-da` is not one pass; it is every dump this compiler knows, including the two
# with no letter of their own.
ALL_LETTER = "a"
ALL_SUFFIXES = ["addressof", "combine", "cse", "cse2", "flow", "gcse", "greg",
                "jump", "jump2", "loop", "lreg", "mach", "regmove", "rtl"]

PASS_BY_LETTER = {letter: (suffix, name) for letter, suffix, name in PASSES}
PASS_BY_SUFFIX = {suffix: name for _, suffix, name in PASSES}

#: Insn heads this tool understands. Anything else is counted, kept in the raw
#: dump, and reported as an unknown record rather than guessed at.
INSN_KINDS = frozenset({
    "insn", "jump_insn", "call_insn", "call_insn_1", "call_insn_2",
    "insn_list", "code_label", "barrier", "note",
})

_INT_RE = re.compile(r"-?\d+")


# --------------------------------------------------------------------------
# S-expression reader
# --------------------------------------------------------------------------
def tokenize(text: str) -> list[str]:
    """Split a GCC dump into parens, strings, symbols and numbers.

    `;;` line comments and the free prose GCC prints between dumps (register
    usage summaries, basic-block banners) are dropped here: this tool reads the
    insn stream, not the prose.
    """
    out: list[str] = []
    i, n = 0, len(text)
    while i < n:
        ch = text[i]
        if ch in " \t\r\n":
            i += 1
        elif text.startswith(";;", i):
            end = text.find("\n", i)
            i = n if end < 0 else end + 1
        elif ch == '"':
            end = i + 1
            while end < n and text[end] != '"':
                end += 2 if text[end] == "\\" else 1
            out.append(text[i:min(end + 1, n)])
            i = end + 1
        elif ch in "()":
            out.append(ch)
            i += 1
        else:
            end = i
            while end < n and text[end] not in ' \t\r\n()"' and not text.startswith(";;", end):
                end += 1
            out.append(text[i:end])
            i = end
    return out


def parse_forms(text: str) -> list[list]:
    """Top-level s-expressions in a dump, in file order."""
    forms: list[list] = []
    stack: list[list] = []
    for tok in tokenize(text):
        if tok == "(":
            node: list = []
            if stack:
                stack[-1].append(node)
            stack.append(node)
        elif tok == ")":
            if not stack:
                continue
            node = stack.pop()
            if not stack:
                forms.append(node)
        elif stack:
            stack[-1].append(tok)
        else:
            # A bare atom at top level (prose that survived tokenizing).
            forms.append([tok])
    return forms


def render(node) -> str:
    """Serialize a parsed form back to one whitespace-normalized line."""
    if isinstance(node, str):
        return node
    return "(" + " ".join(render(item) for item in node) + ")"


# --------------------------------------------------------------------------
# Normalization
# --------------------------------------------------------------------------
def _scrub(node, keep_insn_list: bool = False, *, numbers: bool = True) -> str:
    """Normalized rendering of an RTL subtree.

    With `numbers=False` the subtree is rendered verbatim: that is the mode used
    for insn patterns, where integers ARE the decision under study (register
    numbers, `const_int` values, memory offsets) and must stay visible.

    With `numbers=True` -- the mode used for header slots, cost slots and note
    payloads -- integers are replaced by `#`. Those integers are compiler-
    assigned (instruction uids, `low`, insn length, cost) or record where the
    scratch file was written (a note's line-marker code), and they shift
    whenever anything above them moves.

    `(insn_list ...)` is dropped unless `keep_insn_list=True`: it lists
    instruction uids and moves with every insertion, so keeping it would report
    a difference in every pass and hide the one that matters.
    `(expr_list:REG_* ...)` is ALWAYS dropped -- REG_DEAD / REG_EQUAL /
    REG_UNUSED is allocator bookkeeping rather than an instruction, and a
    `parallel` body is collapsed for the same reason.
    """
    if isinstance(node, str):
        return _INT_RE.sub("#", node) if numbers else node
    if not node:
        return "()"
    head = node[0] if isinstance(node[0], str) else "("
    # The token carries its REG_ kind as a prefix (`expr_list:REG_DEAD`), so the
    # prefix is what identifies the node -- an exact match never fires.
    if head.startswith("expr_list"):
        return "(expr_list)"
    if head == "insn_list":
        if not keep_insn_list:
            return "(insn_list ...)"
        return "(" + " ".join(_scrub(item, keep_insn_list, numbers=numbers) for item in node) + ")"
    if head.rstrip("[").startswith("parallel") and not keep_insn_list:
        return "(parallel ...)"
    return "(" + " ".join(_scrub(item, keep_insn_list, numbers=numbers) for item in node) + ")"


@dataclass
class Record:
    """One normalized dump record."""
    kind: str
    label: str          # display line, ids replaced
    key: str            # comparison key: label plus kept meta

    def to_json(self) -> dict:
        return {"kind": self.kind, "label": self.label, "key": self.key}


def parse_dump(text: str, *, keep_insn_list: bool = False) -> list[Record]:
    """Parse the small RTL subset: `(insn ...)` / `(note ...)` and friends.

    Top-level forms are flat token lists whose head is the node kind
    (`["insn", uid, low, len, pattern, meta, ...]`); children are nested lists.
    """
    records: list[Record] = []
    for head in parse_forms(text):
        if not head or not isinstance(head[0], str):
            continue  # bare prose between dumps
        kind = head[0]
        if kind not in INSN_KINDS:
            records.append(Record("other", render(head), _scrub(head, keep_insn_list)))
            continue
        if kind == "note":
            # (note UID LOW LEN CODE NAME)
            code = head[4] if len(head) > 4 else ""
            name = head[5] if len(head) > 5 else ""
            label = f"(note {name} {_scrub(code, numbers=True)})"
            records.append(Record(kind, label, label))
            continue
        if kind in ("barrier", "code_label"):
            label = _scrub(head, keep_insn_list)
            records.append(Record(kind, label, label))
            continue
        pattern = head[4] if len(head) > 4 else []
        label = f"({kind} {render(pattern) if pattern else '(nil)'})"
        meta = _scrub(head[5:], keep_insn_list, numbers=True).strip()
        key = label if meta in ("", "()") else label + "  " + meta
        records.append(Record(kind, label, key))
    return records


def display_dump(text: str, *, keep_insn_list: bool = False) -> str:
    """Human-readable normalized dump: one record per line, ids replaced."""
    return "\n".join(r.label for r in parse_dump(text, keep_insn_list=keep_insn_list))



# --------------------------------------------------------------------------
# Pass comparison
# --------------------------------------------------------------------------
@dataclass
class PassDiff:
    pass_name: str
    suffix: str
    letter: str
    index: int | None
    kind: str            # "identical" | "operand" | "ordering" | "count"
    a: dict | None
    b: dict | None
    detail: str

    def to_json(self) -> dict:
        return {
            "pass": self.pass_name, "suffix": self.suffix, "letter": self.letter,
            "index": self.index, "kind": self.kind, "detail": self.detail,
            "variant_a": self.a, "variant_b": self.b,
        }


def compare_pass(a_text: str, b_text: str, *, keep_insn_list: bool = False,
                 letter: str = "", suffix: str = "", pass_name: str = "") -> PassDiff:
    """First record index at which two dumps of one pass differ."""
    a = parse_dump(a_text, keep_insn_list=keep_insn_list)
    b = parse_dump(b_text, keep_insn_list=keep_insn_list)
    name = pass_name or PASS_BY_SUFFIX.get(suffix, suffix)
    limit = min(len(a), len(b))
    for i in range(limit):
        if a[i].key != b[i].key:
            same_multiset = sorted(r.key for r in a) == sorted(r.key for r in b)
            return PassDiff(name, suffix, letter, i,
                            "ordering" if same_multiset else "operand",
                            a[i].to_json(), b[i].to_json(),
                            f"record {i} of {len(a)} (A) / {len(b)} (B)")
    if len(a) != len(b):
        longer, tag = (a, "A") if len(a) > len(b) else (b, "B")
        i = limit
        rec = longer[i].to_json() if i < len(longer) else None
        return PassDiff(name, suffix, letter, i, "count", a[i].to_json() if i < len(a) else None,
                        rec, f"A has {len(a)} records, B has {len(b)}; first extra in {tag}")
    return PassDiff(name, suffix, letter, None, "identical", None, None,
                    f"all {len(a)} normalized records match")


# --------------------------------------------------------------------------
# Compilation
# --------------------------------------------------------------------------
def preprocess(source: str, work: Path, tag: str) -> tuple[Path | None, str | None]:
    """C89-transform if needed, then `clang -E` with the project's closure."""
    work.mkdir(parents=True, exist_ok=True)
    c_file = work / f"{tag}.c"
    c_file.write_text(source, encoding="utf-8")
    compile_input = c_file
    if "#include" in source:
        import agbcc_c89_transform as c89_transform
        transformed, stats = c89_transform.cached_transform_file(c_file)
        if stats.get("unhandled"):
            return None, f"C89 transform: {stats['unhandled']}"
        c89_file = work / f"{tag}.c89.c"
        c89_file.write_text(transformed, encoding="utf-8")
        compile_input = c89_file
    i_file = work / f"{tag}.i"
    with i_file.open("w", encoding="utf-8") as handle:
        done = subprocess.run(
            ["clang", "-E", "-nostdinc", "-undef", *probe.inc_flags(), str(compile_input)],
            text=True, stdout=handle, stderr=subprocess.PIPE)
    if done.returncode:
        return None, f"preprocess: {(done.stderr or '').strip()[:1500]}"
    return i_file, None


def dump_files(base: Path, exclude: set[Path]) -> dict[str, Path]:
    """Suffix -> path for every dump agbcc emitted next to `base`.

    Two traps this guards, both observed:

    * A bare `<base>.*` glob also matches the scratch `.c`, `.i` and `.s` files
      the harness itself wrote. `exclude` removes those explicitly, because
      guessing from the extension is not enough: `.s` is also the assembler
      output, and if a future letter ever writes `<base>.c` the scratch source
      would shadow a real dump.
    * Only known dump suffixes are accepted. An unrecognised suffix means the
      letter table is out of date, which is a tool bug -- and silently
      registering it as a pass would put a non-dump in the comparison.
    """
    known = set(ALL_SUFFIXES)
    found: dict[str, Path] = {}
    for path in sorted(base.parent.glob(base.name + ".*")):
        if path in exclude:
            continue
        suffix = path.suffix[1:] if path.suffix.startswith(".") else ""
        if suffix in known:
            found[suffix] = path
    return found




def compile_variant(source: str, tag: str, letters: str, work: Path) -> dict:
    """Preprocess + agbcc with dumps. Returns a record dict; never raises on a
    compiler error, it reports it (`error` key) so the caller can record it."""
    # agbcc runs with cwd=work, so every path handed to it must be absolute or
    # it will be resolved against the scratch directory instead of the repo.
    work = Path(work).resolve()
    work.mkdir(parents=True, exist_ok=True)
    prepped = kit.with_preamble(source)
    preamble_file = work / f"{tag}.preamble.c"
    preamble_file.write_text(prepped, encoding="utf-8")
    i_file, error = preprocess(prepped, work, tag)
    if i_file is None:
        return {"tag": tag, "ok": False, "stage": "preprocess", "error": error}
    base = work / tag
    s_file = work / f"{tag}.s"
    argv = [str(probe.AGBCC), *kit.AGBCC_FLAGS, f"-d{letters}", "-dumpbase", str(base),
            str(i_file), "-o", str(s_file)]
    done = subprocess.run(argv, text=True, capture_output=True, cwd=str(work))
    warning = (done.stderr or done.stdout or "").strip()
    scratch = {preamble_file, i_file, s_file, work / f"{tag}.c", work / f"{tag}.c89.c"}
    dumps = dump_files(base, scratch)
    if not dumps:
        return {"tag": tag, "ok": False, "stage": "agbcc", "argv": argv,
                "error": warning[:1500] or "agbcc emitted no dump files"}
    return {"tag": tag, "ok": True, "argv": argv, "warning": warning[:600],
            "i_file": str(i_file), "s_file": str(s_file) if s_file.exists() else None,
            "dumps": {suffix: str(path) for suffix, path in dumps.items()}}



# --------------------------------------------------------------------------
# Explanation
# --------------------------------------------------------------------------
def explain(diff: PassDiff, ctx: dict) -> list[str]:
    """Rule-based, evidence-tied reading of the first observed difference.

    Every clause names the string it was derived from, so a reader can check the
    claim against the normalized dumps on disk without trusting the tool.
    """
    if diff.kind == "identical":
        return [f"Through pass '{diff.pass_name}' the two variants produced identical "
                f"normalized RTL, so no pass up to and including this one distinguishes them."]
    lines = [
        f"First divergence: pass '{diff.pass_name}' (dump suffix .{diff.suffix}, "
        f"index {diff.index}); A has {ctx.get('a_records')} records, B has "
        f"{ctx.get('b_records')}.",
    ]
    a_key = (diff.a or {}).get("key", "")
    b_key = (diff.b or {}).get("key", "")
    if diff.kind == "ordering":
        lines.append("Both variants emit the SAME SET of normalized records, in a different "
                     "order: this is an operation-ordering decision, not an operand decision.")
    elif diff.kind == "operand":
        lines.append("The normalized record sets differ, so this is an operand/register decision "
                     "rather than a reordering.")
    elif diff.kind == "count":
        lines.append("The record counts differ at this pass, so one variant has already produced "
                     "operations the other has not.")

    a_folded = "const_int" in a_key
    b_folded = "const_int" in b_key
    if a_folded and not b_folded:
        lines.append("A carries the address as a folded integer (`const_int` is present in A's "
                     f"record: {a_key[:160]}), while B does not "
                     f"(B's record: {b_key[:160]}). A constant operand is materializable by the "
                     "combiner/CSE at any point, so A's literal is available before the index "
                     "arithmetic; B's is not folded at this point and must be materialized later.")
    elif b_folded and not a_folded:
        lines.append("B carries the address as a folded integer (`const_int` present in B's "
                     f"record: {b_key[:160]}) while A does not (A's record: {a_key[:160]}).")
    if "asm_input" in a_key or "asm_input" in b_key:
        lines.append("One variant's RTL contains an `asm_input` at this point: the source-level "
                     "assembler text is opaque to the RTL passes, so the optimizer cannot see the "
                     "address as a constant and cannot fold it. That is the mechanism by which an "
                     "assembler-resolved absolute data symbol changes pool-load placement.")
    if not (a_folded != b_folded or "asm_input" in a_key or "asm_input" in b_key):
        lines.append("No folded-literal or asm_input signature is present at this point, so the "
                     "difference is reported as observed without a mechanism claim.")
    return lines


# --------------------------------------------------------------------------
# Textual mutation
# --------------------------------------------------------------------------
def mutate(source: str, spec: str) -> str:
    """Documented textual mutation: `OLD==>NEW`, first occurrence replaced.

    Substitution only -- no AST rewriting, no template matching. It exists so a
    user can produce a controlled pair without hand-writing two files, not so
    the tool can invent variants. The `==>` separator (not `=`) is deliberate:
    the mutation almost always rewrites a C initializer or an assembler
    `symbol = addr` line, both of which contain `=`.

    The needle must occur exactly once. Anything else -- absent, ambiguous,
    malformed -- is an explicit refusal, because a pair that does not differ by
    exactly the one intended edit cannot support a pass-level claim.
    """
    if "==>" not in spec:
        raise kit.Unsupported(f"--mutate expects OLD==>NEW, got {spec!r}")
    old, _, new = spec.partition("==>")
    if not old:
        raise kit.Unsupported("--mutate needs a non-empty OLD")
    if source.count(old) != 1:
        raise kit.Unsupported(
            f"--mutate needle {old!r} occurs {source.count(old)} times in the source; "
            "it must occur exactly once so the pair differs by exactly that edit")
    return source.replace(old, new, 1)


# --------------------------------------------------------------------------
# The run
# --------------------------------------------------------------------------
@dataclass
class Run:
    a_source: str
    b_source: str
    function: str | None
    letters: str
    out_root: Path
    keep_insn_list: bool = False
    writer: kit.EvidenceWriter | None = None
    extras: dict = field(default_factory=dict)


def run_microscope(run: Run) -> dict:
    """Compile both variants, compare every requested pass, return the summary."""
    # Validate the letter set BEFORE anything is written: an unsupported letter
    # must refuse the run, not produce an empty dump set.
    letters = normalize_letters(run.letters)
    resolve_suffixes(letters)
    kit.require_toolchain()

    # `run.writer` is already set when `--out` was given (see main), so this
    # fallback is the no-`--out` case and wants the default timestamped run
    # directory under build/experiments/<strategy>/.
    writer = run.writer or kit.out_writer(
        STRATEGY, None,
        extra={"variants": ["a", "b"], "dump_letters": letters,
               "agbcc": str(probe.AGBCC), "agbcc_flags": list(kit.AGBCC_FLAGS),
               "function": run.function}).open()

    writer.save_candidate("variant_a", run.a_source)
    writer.save_candidate("variant_b", run.b_source)

    scratch = writer.path("probes", "scratch")
    scratch.mkdir(parents=True, exist_ok=True)
    # One dumpbase per variant, so the two never collide in the same directory.
    compiled = {
        "a": compile_variant(run.a_source, "mic_a", letters, scratch),
        "b": compile_variant(run.b_source, "mic_b", letters, scratch),
    }

    for tag, info in compiled.items():
        writer.record({"name": f"variant_{tag}", "status": "COMPILED" if info["ok"] else "TOOL_FAILURE",
                       "matched_bytes": 0, "rom_bytes": len(info.get("dumps", {})),
                       "detail": info.get("error") or ""})
        if not info["ok"]:
            continue
        if info.get("s_file"):
            writer.path("probes", f"{tag}.s").write_text(
                Path(info["s_file"]).read_text(errors="replace"), encoding="utf-8")
        for suffix, path in sorted(info["dumps"].items()):
            dst = writer.path("probes", "dumps", tag, f"{Path(path).stem}.{suffix}")
            dst.parent.mkdir(parents=True, exist_ok=True)
            dst.write_text(Path(path).read_text(errors="replace"), encoding="utf-8")

    failed = [tag for tag, info in compiled.items() if not info["ok"]]
    if failed:
        raise kit.Unsupported(
            "variant(s) " + ",".join(failed) + " did not compile or produced no dumps: "
            + "; ".join(compiled[t]["error"] or "" for t in failed))

    diffs: list[PassDiff] = []
    for letter in letters:
        if letter == ALL_LETTER:
            requested = [(ALL_LETTER, suffix, PASS_BY_SUFFIX.get(suffix, suffix))
                         for suffix in ALL_SUFFIXES]
        else:
            suffix, name = PASS_BY_LETTER[letter]
            requested = [(letter, suffix, name)]
        for letter_, suffix, name in requested:
            a_path = Path(compiled["a"]["dumps"].get(suffix, ""))
            b_path = Path(compiled["b"]["dumps"].get(suffix, ""))
            if not a_path.is_file() or not b_path.is_file():
                diffs.append(PassDiff(name, suffix, letter_, None, "missing", None, None,
                                      "one variant did not emit this dump"))
                continue
            diffs.append(compare_pass(a_path.read_text(errors="replace"),
                                      b_path.read_text(errors="replace"),
                                      keep_insn_list=run.keep_insn_list,
                                      letter=letter_, suffix=suffix, pass_name=name))

    # Normalized dumps for the first differing pass, and for every pass when the
    # pair is identical, so an "identical" verdict is backed by the artifact too.
    first = next((d for d in diffs if d.kind not in ("identical", "missing")), None)
    shown = {first.suffix} if first else {d.suffix for d in diffs if d.kind == "identical"}
    for tag, info in compiled.items():
        for suffix in sorted(shown):
            src = info["dumps"].get(suffix)
            if not src:
                continue
            dst = writer.path("probes", "normalized", f"{tag}.{suffix}.txt")
            dst.parent.mkdir(parents=True, exist_ok=True)
            dst.write_text(display_dump(Path(src).read_text(errors="replace"),
                                        keep_insn_list=run.keep_insn_list) + "\n", encoding="utf-8")

    ctx: dict = {}
    if first is not None:
        ctx = {
            "a_records": len(parse_dump(Path(compiled["a"]["dumps"][first.suffix])
                                        .read_text(errors="replace"))),
            "b_records": len(parse_dump(Path(compiled["b"]["dumps"][first.suffix])
                                        .read_text(errors="replace"))),
        }
        lines = explain(first, ctx)
        lines.append(f"Raw dump for the differing pass: probes/dumps/a/*.{first.suffix} vs "
                     f"probes/dumps/b/*.{first.suffix}; normalized forms at "
                     f"probes/normalized/a.{first.suffix}.txt and b.{first.suffix}.txt.")
    else:
        lines = ["No pass reported a difference: for these two variants the normalized RTL is "
                 "identical at every requested pass."]
    lines.append("SCOPE: this is an observation about THESE TWO VARIANTS as compiled by the "
                 "pinned agbcc. It is not a reconstruction of the original compilation of any "
                 "ROM body, and it cannot promote a byte: no draft was assembled, integrated or "
                 "linked.")

    writer.write_json("passes.json", {
        "order": [{"letter": l, "suffix": s, "pass": n}
                  for l, s, n in PASSES if l in letters or l == ALL_LETTER],
        "diffs": [d.to_json() for d in diffs],
        "first_differing_pass": first.to_json() if first else None,
        "scope": "observation about these two variants only",
    })
    writer.record({"name": "first_differing_pass",
                   "status": first.kind.upper() if first else "IDENTICAL",
                   "matched_bytes": 0, "rom_bytes": len(diffs),
                   "detail": first.detail if first else ""})

    explanation = "\n".join(lines) + "\n"
    writer.path("diagnostics", "explanation.md").write_text(explanation, encoding="utf-8")

    summary = {
        "draft_status": "NOT_APPLICABLE_DIAGNOSTIC",
        "integrated_status": "not run",
        "link_accepted": "not run",
        "first_differing_pass": first.pass_name if first else "none",
        "difference_kind": first.kind if first else "identical",
        "passes_compared": len(diffs),
        "notes": lines,
        "results": [{"name": f"variant_{tag}", "status": "COMPILED", "matched_bytes": 0,
                     "rom_bytes": len(info["dumps"]), "detail": ""}
                    for tag, info in compiled.items()],
    }
    writer.finish(summary)
    return {"summary": summary, "writer": writer, "diffs": diffs, "first": first,
            "explanation": explanation, "compiled": compiled}


def print_report(result: dict) -> None:
    writer: kit.EvidenceWriter = result["writer"]
    diffs: list[PassDiff] = result["diffs"]
    print(f"compiler-microscope run: {writer.relative(writer.dir)}")
    print(f"  dumps compared: {len(diffs)}")
    for diff in diffs:
        mark = {"identical": "=", "missing": "?"}.get(diff.kind, "!")
        print(f"  {mark} {diff.letter or 'a'} .{diff.suffix:<10} {diff.pass_name:<20} {diff.kind:<9} {diff.detail}")
    print()
    for line in result["explanation"].rstrip().splitlines():
        print(line)


# --------------------------------------------------------------------------
# Self-test
# --------------------------------------------------------------------------
def self_test() -> int:
    total = passed = 0

    def check(label: str, condition: bool) -> None:
        nonlocal total, passed
        total += 1
        ok = bool(condition)
        passed += 1 if ok else 0
        print(f"{'PASS' if ok else 'FAIL'}: {label}")

    # 1. Normalization.
    dump = """
;; Function candidate

(insn 4 2 6 (set (reg:SI 23)
        (reg:SI 0 r0)) -1 (nil) {*movsi_insn} (insn_list 4 (nil))
    (expr_list:REG_DEAD (reg:SI 0 r0)
        (nil)))

(note 11 10 12 "" NOTE_INSN_FUNCTION_BEG)
"""
    shifted = dump.replace("(insn 4 2 6", "(insn 9 7 6").replace("(note 11 10 12", "(note 14 13 12")
    check("uid/note-number normalization is stable",
          display_dump(dump) == display_dump(shifted))
    check("normalization keeps register choices visible",
          "(reg:SI 23)" in display_dump(dump))
    check("normalization keeps modes visible", ":SI" in display_dump(dump))
    default_key = parse_dump(dump)[0].key
    kept_key = parse_dump(dump, keep_insn_list=True)[0].key
    check("uid-bearing insn_list and REG_ bookkeeping are collapsed by default",
          "(insn_list ...)" in default_key and "REG_DEAD" not in default_key
          and "REG_DEAD" not in display_dump(dump))
    check("--keep-insn-list restores insn_list uids but never REG_ bookkeeping",
          "(insn_list #" in kept_key and "REG_DEAD" not in kept_key)
    check("the insn pattern is still visible with --keep-insn-list",
          "(reg:SI 23)" in display_dump(dump, keep_insn_list=True))
    check("the cost name survives normalization", "{*movsi_insn}" in default_key)
    linemarker = "(note 11 10 12 42 NOTE_INSN_FUNCTION_BEG)\n"
    other = "(note 11 10 12 77 NOTE_INSN_FUNCTION_BEG)\n"
    check("line-marker note codes normalize", display_dump(linemarker) == display_dump(other))
    check("note kind stays visible", "NOTE_INSN_FUNCTION_BEG" in display_dump(linemarker))

    # 2. Pass comparison on fixed text.
    const_a = "(insn 26 23 28 (set (reg:SI 35) (plus:SI (reg/v:SI 28) (const_int 50337266))) -1 (nil) (nil))"
    const_b = "(insn 31 28 33 (set (reg:SI 35) (plus:SI (reg/v:SI 33) (reg:HI 40))) -1 (nil) (nil))"
    same = compare_pass(const_a + "\n" + const_b, const_a + "\n" + const_b)
    check("identical pass text compares identical", same.kind == "identical" and same.index is None)
    mixed = compare_pass(const_a + "\n" + const_b, const_b + "\n" + const_a)
    check("reordered records are reported as ordering", mixed.kind == "ordering" and mixed.index == 0)
    swapped = compare_pass(const_a + "\n" + const_b, const_a + "\n" + const_b.replace("(reg:SI 35)", "(reg:SI 36)"))
    check("register change is reported as operand, not ordering",
          swapped.kind == "operand" and "35" in swapped.a["label"] and "36" in swapped.b["label"])

    # 3. Negative control: an unsupported -d letter must be refused, not ignored.
    def unsupported_letter() -> bool:
        try:
            run_microscope(Run("int f(void){return 1;}\n", "int f(void){return 1;}\n", None,
                               "r,Z", writer=None,
                               out_root=Path("/tmp/compiler-microscope-never")))
        except kit.Unsupported:
            return True
        return False
    check("unsupported -d letter is refused (negative control)", unsupported_letter())

    def unknown_suffix() -> bool:
        try:
            resolve_suffixes("ZZZ")
        except kit.Unsupported:
            return True
        return False
    check("unknown dump suffix is refused (negative control)", unknown_suffix())

    # 4. Mutation: exactly one textual edit, refused when the needle is absent.
    src = "u32 base = 0x030015F0u;\n"
    mutated = mutate(src, "0x030015F0u==>TrackAwardRecBase")
    check("mutation applies exactly one edit", "TrackAwardRecBase" in mutated and src.count("0x030015F0u") == 1)
    check("mutation refuses an absent needle", kit.raises(kit.Unsupported, lambda: mutate(src, "nope==>x")))
    check("mutation refuses an ambiguous needle",
          kit.raises(kit.Unsupported, lambda: mutate("x\nx\n", "x==>y")))
    check("mutation refuses a malformed spec", kit.raises(kit.Unsupported, lambda: mutate(src, "noarrow")))

    # 5. Missing toolchain must be DependencyMissing, never a silent fallback.
    saved = probe.AGBCC
    try:
        probe.AGBCC = Path("/nonexistent/agbcc-does-not-exist")
        check("missing agbcc yields DependencyMissing",
              kit.raises(kit.DependencyMissing, kit.require_toolchain))
        check("missing agbcc yields DependencyMissing on a real run (negative control)",
              kit.raises(kit.DependencyMissing, lambda: run_microscope(Run(
                  "int f(void){return 1;}\n", "int f(void){return 1;}\n", None, "r",
                  out_root=Path("/tmp/compiler-microscope-never"),
                  writer=kit.EvidenceWriter(STRATEGY, "selftest-dep-missing",
                                            root=Path("/tmp/compiler-microscope-never")).open()))))
    finally:
        probe.AGBCC = saved

    # 6. Real compiler: identical variants and folded-constant variants.
    identical = "typedef unsigned int u32;\ns16 candidate(s16 idx) {\nu32 base = 0x030015F0u;\n" \
                "s32 off = (s32)((u32)idx << 16) >> 13;\nreturn *(s16 *)(base + off + 2);\n}\n"
    symbolised = mutate(identical,
                        "u32 base = 0x030015F0u;==>extern u8 TrackAwardRecBase[];\n"
                        "__asm__(\".globl TrackAwardRecBase\\nTrackAwardRecBase = 0x030015F0\\n\");\n"
                        "u32 base = (u32)TrackAwardRecBase;")
    try:
        # Under a self-test subdirectory: a real run's evidence directory must
        # not be polluted by regression runs that reuse fixed run ids.
        out_root = Path("build/experiments") / STRATEGY / "self-test"
        same_run = run_microscope(Run(identical, identical, None, "r", out_root=out_root,
                                      writer=kit.EvidenceWriter(STRATEGY, "selftest-identical", root=out_root).open()))
        check("two identical variants produce NO pass difference",
              same_run["first"] is None and all(d.kind == "identical" for d in same_run["diffs"]))
        diff_run = run_microscope(Run(identical, symbolised, None, "c,r,s,f", out_root=out_root,
                                      writer=kit.EvidenceWriter(STRATEGY, "selftest-folded", root=out_root).open()))
        check("two variants differing only in a folded constant DO differ",
              diff_run["first"] is not None)
        check("first differing pass is reported with its records",
              diff_run["first"] is not None and diff_run["first"].a is not None
              and diff_run["first"].b is not None)
        check("explanation states the two-variant scope limit",
              "not a reconstruction" in diff_run["explanation"])
        check("explanation never claims a promotion",
              "cannot promote a byte" in diff_run["explanation"])
        check("every requested pass was compared", len(diff_run["diffs"]) == 4)
    except kit.DependencyMissing as exc:
        check("real-compiler self-test (agbcc present)", False)
        print(f"      skipped: {exc}")

    print(f"\nself-test: {passed}/{total} passed")
    return 0 if passed == total else 1


def normalize_letters(text: str) -> str:
    """Accept `r,c,f` and `rcf` alike; the comparison order is the argument's."""
    return "".join(ch for ch in text if ch not in ", \t")


def resolve_suffixes(letters: str) -> list[str]:
    """Expand a `-d` letter set to the dump suffixes it can produce."""
    out: list[str] = []
    for letter in normalize_letters(letters):
        if letter == ALL_LETTER:
            out.extend(ALL_SUFFIXES)
        elif letter in PASS_BY_LETTER:
            out.append(PASS_BY_LETTER[letter][0])
        else:
            raise kit.Unsupported(
                f"-d{letter} is not a dump this toolchain supports "
                f"(supported: {DEFAULT_LETTERS}a); refusing rather than reporting an "
 "empty dump set that is indistinguishable from a pass that did nothing")
    return out


# --------------------------------------------------------------------------
# CLI
# --------------------------------------------------------------------------
def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="compiler microscope (diagnostic wrapper; no instrumented agbcc)")
    ap.add_argument("variant_a", nargs="?", type=Path, help="first scratch C89 variant")
    ap.add_argument("variant_b", nargs="?", type=Path,
                    help="second variant; omit with --mutate")
    ap.add_argument("--function", help="record the lifted function name in the evidence")
    ap.add_argument("--passes", default=DEFAULT_LETTERS,
                    help=f"dump letters to compare, in order (default {DEFAULT_LETTERS}; "
                         f"add 'a' for every dump)")
    ap.add_argument("--mutate", metavar="OLD==>NEW",
                    help="derive variant B from variant A by one textual substitution "
                         "(first occurrence; must occur exactly once)")
    ap.add_argument("--out", type=Path, help="run directory (default build/experiments/"
                                            + STRATEGY + "/<run-id>)")
    ap.add_argument("--json", type=Path, help="copy the run summary here")
    ap.add_argument("--keep-insn-list", action="store_true",
                    help="keep uid-bearing (insn_list ...) bookkeeping in the comparison key")
    ap.add_argument("--self-test", action="store_true", help="run the self-test and exit")
    return ap


def body(args) -> int:
    if args.self_test:
        return self_test()
    if args.variant_a is None:
        raise kit.Unsupported("a variant source is required (positional or --mutate with one source)")
    a_text = Path(args.variant_a).read_text(encoding="utf-8")
    if args.mutate:
        if args.variant_b is not None:
            raise kit.Unsupported("--mutate and a second source file are mutually exclusive")
        b_text = mutate(a_text, args.mutate)
    elif args.variant_b is not None:
        b_text = Path(args.variant_b).read_text(encoding="utf-8")
    else:
        raise kit.Unsupported("need a second variant: pass VARIANT_B.c or --mutate OLD==>NEW")

    out_root = Path(args.out) if args.out else None
    run = Run(a_text, b_text, args.function, args.passes, out_root or kit.EXPERIMENTS_ROOT / STRATEGY,
              keep_insn_list=args.keep_insn_list)
    if out_root:
        # `--out` IS the run directory. `root=out_root` with an auto-generated
        # run id wrote to a timestamped CHILD of the path the caller named.
        run.writer = kit.out_writer(STRATEGY, out_root,
                                    extra={"dump_letters": args.passes,
                                           "agbcc": str(probe.AGBCC),
                                           "agbcc_flags": list(kit.AGBCC_FLAGS),
                                           "function": args.function}).open()
    result = run_microscope(run)
    print_report(result)
    if args.json:
        payload = {
            "run_dir": str(result["writer"].dir),
            "summary": result["summary"],
            "passes": [d.to_json() for d in result["diffs"]],
            "explanation": result["explanation"],
        }
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print(f"\nwrote {args.json}")
    return 0


def main() -> int:
    return kit.run_tool(STRATEGY, body, ap=build_parser())


if __name__ == "__main__":
    sys.exit(main())
