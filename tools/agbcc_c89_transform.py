#!/usr/bin/env python3
"""General C89 conformance transform for the lifted C corpus, compiler-driven.

agbcc (GCC 2.95.3) is a C89 compiler; the corpus is C99. Hand-patching 121 files
is not viable and a hand-written C parser is not reliable enough to do it safely.

This tool uses the compiler to locate rejected syntax: compile, take the
diagnostic's exact line, apply the transform that line needs, repeat. Compiler
acceptance is only a syntax gate. `c89_equivalence.py` independently compares
modern ARM object output from the original and generated sources.

Three intended semantics-preserving transforms:

  for    `for (int i = INIT; ...) BODY` -> `{ int i = INIT; for (; ...) BODY }`.
         The new block retains the loop variable's original scope, and the
         initializer still runs exactly once at the same point.

  nest   a declaration after a statement -> wrap the rest of its block in `{ }`.
         The declaration becomes the first thing in a fresh scope that begins
         after the preceding statements, so no initializer is evaluated earlier
         than before. This is the only correct option when the initializer reads
         something an earlier statement wrote.

  extern an `extern` declaration inside a function body -> hoisted to file scope.
         agbcc (GCC 2.95) rejects these even though C89 and C99 both allow them;
         this is a compiler limitation rather than a language one.

The wrap closes inside its original braces, including at a `} else {` line.

`src/` is never modified; output goes to the build directory.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
import threading
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
# Matching compiler (see docs/findings/compiler_split.md): the transformer must
# verify against the build the slice is matched with.
AGBCC = ROOT / "build/toolchains/agbcc/old_agbcc"
SRC = ROOT / "src"
WORK = ROOT / "build/era-corpus/c89"
CACHE_DIR = ROOT / "build/era-corpus/c89-cache"
INCLUDE = [ROOT / "include", ROOT / "asm", ROOT / "build/toolchains/agbcc/ginclude"]

# agbcc reports the *generated* file's path, so the line number is what matters.
DIAG = re.compile(r":(\d+):\s*(.+)$")

FOR_DECL = re.compile(
    r"\bfor\s*\(\s*"
    r"(?P<type>(?:(?:unsigned|signed|const|struct|volatile|register)\s+)*"
    r"(?:int|char|short|long|float|double|unsigned|signed|u8|u16|u32|s8|s16|s32|u64|s64|size_t|vu8|vu16|vu32|vs8|vs16|vs32|u?int\d+_t))"
    r"\s+(?P<ptr>\*+\s*)?(?P<name>[A-Za-z_]\w*)\s*=\s*(?P<init>[^;]*);")

# A diagnostic can point at the first line of a multi-line initializer or a
# declaration sharing its line with a subsequent statement. Only the *start*
# of a declaration matters when we nest the rest of its enclosing block.
DECL_START = re.compile(
    r"^\s*(?:(?:static|register|const|volatile|unsigned|signed)\s+)*"
    r"(?:struct\s+\w+|union\s+\w+|enum\s+\w+|void|char|short|int|long|float|double|unsigned|signed|"
    r"u8|u16|u32|u64|s8|s16|s32|s64|vu8|vu16|vu32|vs8|vs16|vs32|"
    r"bool|bool8|bool32|size_t|uintptr_t|u?int\d+_t|"
    r"(?!if\b|for\b|while\b|return\b|goto\b|else\b|case\b|switch\b|do\b|extern\b)"
    r"[A-Za-z_]\w*)\s+\**\s*[A-Za-z_]\w*")

INLINE_EXTERN = re.compile(r"\bextern\s+[^;{}]*;")

MASK_CASES = (
    # (label, source, masked source with all whitespace removed)
    # Each entry is a construct that can OPEN A RUN in a comment/string/char
    # regex chain. The first two are the negative controls: an apostrophe in a
    # comment or in a string literal was never broken, because both are masked
    # before the char-literal pass could ever see the quote. They are here so a
    # future "fix" cannot regress them while chasing the case below.
    ("apostrophe inside a // comment",
     "int a; // don't panic\nint b;\n", "inta;intb;"),
    ("apostrophe inside a string literal",
     'char *s = "don\'t stop";\nint b;\n', "char*s=;intb;"),
    ("double quote inside a char literal",
     'p[5] = \'\\"\';\nint b;\n', "p[5]=;intb;"),
    ("escaped apostrophe inside a char literal",
     "p[2] = '\\'';\nint b;\n", "p[2]=;intb;"),
    ("// inside a string literal",
     'char *u = "http://x";\nint c = 0;\n', "char*u=;intc=0;"),
    ("/* and */ inside a string literal",
     'char *v = "/* not a comment */";\nint c = 0;\n', "char*v=;intc=0;"),
    ("/* inside a string with a real */ later",
     'char *w = "/* open";\nint c = 0;\nc = c + 1;\n/* close */\nint d = 0;\n',
     "char*w=;intc=0;c=c+1;intd=0;"),
    ("unterminated double quote stops at the newline",
     'int a; "oops\nint b;\n', "inta;intb;"),
)


def run(argv, stdout=None):
    kwargs = {"text": True, "stderr": subprocess.PIPE}
    kwargs["stdout"] = stdout if stdout is not None else subprocess.PIPE
    return subprocess.run([str(a) for a in argv], **kwargs)


def inc_flags():
    out = []
    for d in INCLUDE:
        if d.is_dir():
            out += ["-I", str(d)]
    return out


def strip_noncode(text: str) -> str:
    """Blank comments, string literals and char literals in ONE left-to-right pass.

    Four `re.sub` passes in a fixed order cannot do this, because every pass
    scans the ALREADY-masked text: whichever construct is masked first can open
    a run inside a construct of a different kind, and the wrong kind of quote
    then matches all the way to the next same-kind quote. Measured
     on `src/runtime_hud.c`: the string pass saw the `"` inside the
    char literal `p[5] = '"';` and ran to the `"` of a second `'"'` literal 12
    lines later, blanking 316 bytes of real code including a closing brace --
    163 `{` against 162 `}` -- so `matching_delimiter` returned None and
    `tools/match_context.py` could not extract `TimeStr_03D4C` at all. The
    mirror case, `//` or `/*` inside a `"` string, blanks the rest of the line
    instead. Reordering the four passes only swaps which of the two survives.

    A lexer also keeps the mask LINE-PRESERVING, which the old passes did not:
    `" " * len(...)` blanks newlines inside a multi-line run, and
    `hoist_extern_at` indexes `strip_noncode(text).splitlines()` into
    `text.splitlines()`, so one blanked newline moves every later line.

    The mask is length- and newline-preserving by contract; `self_test` checks
    both, plus one fixture per construct that can open a run.
    """
    out = list(text)
    n = len(text)
    i = 0
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            close = text.find("*/", i + 2)
            end = n if close < 0 else close + 2
        elif c == "/" and i + 1 < n and text[i + 1] == "/":
            close = text.find("\n", i)
            end = n if close < 0 else close
        elif c == '"' or c == "'":
            # A C literal cannot span a raw newline, so stop there: a stray or
            # unterminated quote stays a one-line defect instead of blanking
            # the rest of the file.
            j = i + 1
            while j < n:
                if text[j] == "\\" and j + 1 < n:
                    j += 2
                    continue
                if text[j] == c or text[j] == "\n":
                    j += 1
                    break
                j += 1
            end = j
        else:
            i += 1
            continue
        for k in range(i, end):
            if out[k] != "\n":
                out[k] = " "
        i = end
    return "".join(out)


def compile_probe(text: str, tag: str):
    """Return (ok, line_no, message). `line_no` is where agbcc first objects."""
    WORK.mkdir(parents=True, exist_ok=True)
    tid = threading.get_ident()
    c, i, s = WORK / f"{tag}_{tid}.c", WORK / f"{tag}_{tid}.i", WORK / f"{tag}_{tid}.s"
    c.write_text(text, encoding="utf-8")
    with i.open("w", encoding="utf-8") as h:
        if run(["clang", "-E", "-nostdinc", "-undef", *inc_flags(), str(c)], stdout=h).returncode:
            return False, 0, "preprocess"
    out = run([AGBCC, "-O2", "-mthumb-interwork", str(i), "-o", str(s)])
    if out.returncode == 0:
        return True, 0, "OK"
    for line in (out.stderr or "").splitlines():
        m = DIAG.search(line)
        if m and "warning:" not in line and ("error" in line or "syntax" in line
                                                  or "undeclared" in line):
            return False, int(m.group(1)), m.group(2).strip()
    for line in (out.stderr or "").splitlines():
        m = DIAG.search(line)
        if m and "In function" not in line and "warning:" not in line:
            return False, int(m.group(1)), m.group(2).strip()
    return False, 0, (out.stderr or "").strip().splitlines()[:1][0][:60] if out.stderr else "unknown"


def line_prefix(text: str, offset: int) -> str:
    return text[text.rfind("\n", 0, offset) + 1:offset]


def matching_delimiter(code: str, opening: int, left: str, right: str) -> int | None:
    """Find the matching delimiter in comment/string-masked source."""
    depth = 0
    for pos in range(opening, len(code)):
        if code[pos] == left:
            depth += 1
        elif code[pos] == right:
            depth -= 1
            if depth == 0:
                return pos
    return None


def loop_body_end(code: str, offset: int) -> int | None:
    """Find the end of a loop body, including an unbraced if/else."""
    opening = code.find("(", offset)
    if opening < 0:
        return None
    closing = matching_delimiter(code, opening, "(", ")")
    if closing is None:
        return None
    return statement_end(code, closing + 1)


def statement_end(code: str, pos: int) -> int | None:
    """Find a C statement boundary in masked source; decline unknown syntax."""
    while pos < len(code) and code[pos].isspace():
        pos += 1
    if pos >= len(code):
        return None
    if code[pos] == "{":
        end = matching_delimiter(code, pos, "{", "}")
        return end + 1 if end is not None else None
    if re.match(r"(?:if|for|while|switch)\b", code[pos:]):
        opening = code.find("(", pos)
        if opening < 0:
            return None
        closing = matching_delimiter(code, opening, "(", ")")
        if closing is None:
            return None
        end = statement_end(code, closing + 1)
        if end is None:
            return None
        tail = end
        while tail < len(code) and code[tail].isspace():
            tail += 1
        if code.startswith("else", tail) and not re.match(r"\w", code[tail + 4:tail + 5]):
            return statement_end(code, tail + 4)
        return end
    if re.match(r"do\b", code[pos:]):
        end = statement_end(code, pos + 2)
        if end is None:
            return None
        tail = code.find(";", end)
        return tail + 1 if tail >= 0 else None
    depth = 0
    for i in range(pos, len(code)):
        if code[i] == "(":
            depth += 1
        elif code[i] == ")":
            depth -= 1
        elif code[i] == ";" and depth == 0:
            return i + 1
        elif code[i] in "{}" and depth == 0:
            return None
    return None


def fix_for_at(text: str, offset: int) -> tuple[str, bool]:
    """Wrap a C99 loop in a C89 block without changing its variable scope."""
    m = FOR_DECL.match(text, offset)
    if not m:
        return text, False
    end = loop_body_end(strip_noncode(text), offset)
    if end is None:
        return text, False
    decl = f"{m.group('type')} {m.group('ptr') or ''}{m.group('name')} = {m.group('init')};"
    loop = "for (;"
    prefix = line_prefix(text, m.start())
    indent = re.match(r"[ \t]*", prefix).group(0)
    replacement = f"{{\n{indent}    {decl}\n{indent}    {loop}{text[m.end():end]}\n{indent}}}"
    return text[:offset] + replacement + text[end:], True


def wrap_at(text: str, index: int) -> tuple[str, bool]:
    """Nest the declaration and following statements in their actual block."""
    lines = text.splitlines()
    if index >= len(lines):
        return text, False
    code = strip_noncode(text)
    offset = sum(len(line) + 1 for line in lines[:index])
    stack: list[int] = []
    for pos, char in enumerate(code[:offset]):
        if char == "{":
            stack.append(pos)
        elif char == "}" and stack:
            stack.pop()
    if not stack:
        return text, False
    close = matching_delimiter(code, stack[-1], "{", "}")
    if close is None or close < offset:
        return text, False
    indent = re.match(r"[ \t]*", lines[index]).group(0)
    return (text[:offset] + f"{indent}{{\n" + text[offset:close]
            + f"{indent}}}\n" + text[close:]), True


def hoist_extern_at(text: str, index: int) -> tuple[str, bool]:
    """Move an in-block `extern` declaration to file scope."""
    lines = text.splitlines()
    if index >= len(lines):
        return text, False
    masked = strip_noncode(lines[index])
    matches = list(INLINE_EXTERN.finditer(masked))
    if not matches:
        return text, False
    decls = [lines[index][m.start():m.end()].strip() for m in matches]
    cleaned = lines[index]
    for m in reversed(matches):
        cleaned = cleaned[:m.start()] + cleaned[m.end():]
    lines[index] = cleaned
    code = strip_noncode(text).splitlines()
    # Put declarations after includes but before the first conditional, so an
    # `extern` from an ARM function cannot accidentally land in `#ifdef __APPLE__`.
    # Preserve all other source text, including calls sharing the same line.
    insert_at = 0
    for i, cl in enumerate(code):
        s = cl.strip()
        if s.startswith("#include") or not s:
            insert_at = i + 1
        else:
            break
    return "\n".join(lines[:insert_at] + decls + lines[insert_at:]) + "\n", True


def fix_line(text: str, line_no: int) -> tuple[str, str, bool]:
    """Apply the transform the compiler asked for at `line_no` (1-based)."""
    lines = text.splitlines()
    if not (1 <= line_no <= len(lines)):
        return text, "unaddressable", False
    index = line_no - 1
    line = lines[index]
    # A `for (TYPE name = ...)` is the C99 loop-declaration form, whether it
    # starts the line or shares it with a preceding statement.
    m = FOR_DECL.search(line)
    if m:
        offset = sum(len(l) + 1 for l in lines[:index]) + m.start()
        new, ok = fix_for_at(text, offset)
        if ok:
            return new, "for", True
    if INLINE_EXTERN.search(strip_noncode(line)):
        new, ok = hoist_extern_at(text, index)
        if ok:
            return new, "extern", True
    if DECL_START.match(strip_noncode(line)):
        new, ok = wrap_at(text, index)
        if ok:
            return new, "nest", True
    return text, "unhandled", False


def transform_file(path: Path, max_rounds: int = 300,
                   same_site_tolerance: int = 3,
                   same_message_tolerance: int = 48) -> tuple[str, dict]:
    """Fix-and-recompile until agbcc accepts the file, or the error stops moving.

    Detecting a STALL needs two tests, not one. A single "the message did not
    change" test is a false negative waiting to happen: a real file with two
    for-declarations reports the same `syntax error before 'int'` twice, the
    transform fixes the first, and the next compile reports the SAME text one
    line further down. Measured: that rule dropped the corpus from 152/152 to
    71/152, breaking `ai_line_more.c`, `car_physics_core.c`, `runtime_record_helpers.c`,
    `course_collision.c` and a dozen more.

    Both tests are TOLERANCES, never single-shot bails, because message text
    alone cannot tell "stuck" from "many instances of one diagnostic":

    * The same `(message, line_no)` repeating `same_site_tolerance` times in a
      row means the transform is no longer moving the error.
    * The same message at a DIFFERENT line is progress -- one instance of a
      repeated diagnostic is being fixed at a time -- so it is tolerated up to
      `same_message_tolerance` times.

    Measured while getting this right: a single-shot "message unchanged" bail
    dropped the corpus from 152/152 to **71/152**, breaking `ai_line_more.c`,
    `car_physics_core.c`, `runtime_record_helpers.c`, `course_collision.c` and a dozen
    more; adding a same-line bail recovered it only to 151/152
    (`ai_race_leaves.c`). With both tolerances the corpus is back to 152/152,
    while the genuinely stuck case -- a seed using the file-local
    `Ai_LineRec22` typedef, which appears in no header and so cannot compile at
    all -- still collapses from 300 rounds to a handful and now reports the real
    compiler message instead of "budget exhausted".
    """
    text = path.read_text(encoding="utf-8", errors="replace")
    stats = {"for": 0, "nest": 0, "extern": 0, "rounds": 0, "unhandled": None}
    last_site: tuple[str, int] | None = None
    same_site = 0
    repeated = 0
    for _ in range(max_rounds):
        ok, line_no, msg = compile_probe(text, path.stem)
        stats["rounds"] += 1
        if ok:
            return text, stats
        if line_no == 0:
            stats["unhandled"] = msg
            return text, stats
        new, kind, changed = fix_line(text, line_no)
        if not changed:
            stats["unhandled"] = f"line {line_no}: {msg}"
            return text, stats
        site = (msg, line_no)
        if site == last_site:
            # Same message at the same line again. Repeated CONSECUTIVELY a few
            # times means the transform is not moving the error and this is a
            # diagnostic `fix_line` cannot address -- but a single occurrence is
            # ordinary, because hoisting a declaration can leave the error
            # standing for a round. Measured: bailing on the FIRST such repeat
            # cost the corpus `ai_race_leaves.c`, so this is a tolerance.
            same_site += 1
            if same_site >= same_site_tolerance:
                stats["unhandled"] = (f"line {line_no}: {msg} unchanged by "
                                      f"{same_site} rounds of {kind} transforms")
                return text, stats
        else:
            same_site = 1
        if last_site is not None and msg == last_site[0]:
            # Same message, DIFFERENT line: real progress, one instance of a
            # repeated diagnostic at a time. Tolerated rather than bailed on.
            repeated += 1
            if repeated >= same_message_tolerance:
                stats["unhandled"] = (f"line {line_no}: {msg} still unaddressed "
                                      f"after {repeated} rounds of {kind} transforms")
                return text, stats
        else:
            repeated = 0
        last_site = site
        stats[kind] += 1
        text = new
    stats["unhandled"] = "budget exhausted"
    return text, stats


def cached_transform_file(path: Path) -> tuple[str, dict]:
    """Transform source to C89 with a content-addressed disk cache.

    The key covers the .c bytes and the two tools, but NOT the headers the TU
    includes -- so a result is only as fresh as the last edit to its own file.
    That asymmetry is harmless for a cached SUCCESS (the object cache in
    `corpus_match_probe.py` re-keys on the preprocessed text, so a header edit
    still forces a recompile) and harmful for a cached FAILURE: a failure
    recorded while a header carried a conflicting declaration stays in the
    cache after the header is fixed, and the TU is skipped forever with a
    reason that no longer exists. Measured : an `include/gtadv/
    menus.h` prototype edit repaired `src/menus.c`, the untouched .c kept its
    `line 1061: conflicting types for \`MenuF5A0_0800F5A0'` entry, and every
    later probe of that TU reported the stale error. Failures are therefore
    recomputed rather than cached; they are also the rare case.
    """
    key = None
    try:
        h = hashlib.sha256()
        h.update(path.read_bytes())
        c89_tool = Path(__file__).resolve()
        h.update(str(c89_tool.stat().st_mtime_ns if c89_tool.exists() else 0).encode())
        h.update(str(AGBCC.stat().st_mtime_ns if AGBCC.exists() else 0).encode())
        key = h.hexdigest()[:32]
        cache_entry = CACHE_DIR / key
        transformed_file = cache_entry / "transformed.c"
        stats_file = cache_entry / "stats.json"
        if transformed_file.exists() and stats_file.exists():
            transformed = transformed_file.read_text(encoding="utf-8")
            stats = json.loads(stats_file.read_text(encoding="utf-8"))
            return transformed, stats
    except Exception:
        key = None

    transformed, stats = transform_file(path)
    if key and stats.get("unhandled") is None:
        try:
            cache_entry = CACHE_DIR / key
            cache_entry.mkdir(parents=True, exist_ok=True)
            (cache_entry / "transformed.c").write_text(transformed, encoding="utf-8")
            (cache_entry / "stats.json").write_text(json.dumps(stats), encoding="utf-8")
        except Exception:
            pass
    return transformed, stats


def self_test() -> int:
    """Check the comment/string/char mask, then the rewrites, on real programs."""
    for label, source, want in MASK_CASES:
        masked = strip_noncode(source)
        # Every caller converts offsets on the mask back to offsets on the
        # source, and `hoist_extern_at` indexes splitlines() of the mask into
        # splitlines() of the source, so both invariants are load-bearing.
        if len(masked) != len(source):
            print(f"self-test mask fixture {label}: mask is not length-preserving",
                  file=sys.stderr)
            return 1
        if [i for i, ch in enumerate(masked) if ch == "\n"] != \
                [i for i, ch in enumerate(source) if ch == "\n"]:
            print(f"self-test mask fixture {label}: mask moved a newline",
                  file=sys.stderr)
            return 1
        got = re.sub(r"\s+", "", masked)
        if got != want:
            print(f"self-test mask fixture {label}: masked {got!r}, want {want!r}",
                  file=sys.stderr)
            return 1
    # The invariant `tools/match_context.function_source` depends on: a char
    # literal holding the OTHER quote must not eat the function's closing
    # brace, or `matching_delimiter` returns None and the body is silently
    # reported as not found. Measured on src/runtime_hud.c , where
    # `TimeStr_03D4C` became unextractable for exactly this reason.
    tricky = ("void f(int n) {\n"
              "    if (n) {\n"
              "        p[5] = '\"';\n"
              "        p[0] = '0';\n"
              "        return;\n"
              "    }\n"
              "    p[2] = 0;\n"
              "    p[3] = '\"';\n"
              "}\n")
    closing = tricky.rindex("}")
    tmask = strip_noncode(tricky)
    if tmask[closing] != "}" or matching_delimiter(tmask, tmask.index("{"),
                                                   "{", "}") is None:
        print("self-test mask fixture: a '\"' char literal masked a closing brace",
              file=sys.stderr)
        return 1
    fixtures = [
        ("int f(int n) { int c=0; for (int i=0;i<n;i++) { c+=i; } return c; }\n"
         "int main(void) { return f(5)!=10; }\n", "for"),
        ("int f(int x) { if(x) { x++; int y=x*2; x=y; } else { x=7; } return x; }\n"
         "int main(void) { return f(2)!=6 || f(0)!=7; }\n", "nest"),
        ("int f(int n) { int c=0; for (int i=0;i<n;i++) if(i&1) c+=i; "
         "return c; }\nint main(void) { return f(5)!=4; }\n", "for"),
        ("int g(int x) { return x+3; }\n"
         "int f(int x) { x++; extern int g(int); return g(x); }\n"
         "int main(void) { return f(5)!=9; }\n", "extern"),
    ]
    with tempfile.TemporaryDirectory(prefix="gtadv-c89-test-") as td:
        root = Path(td)
        tasks = []
        for i, (original, kind) in enumerate(fixtures):
            if kind == "for":
                off = original.index("for (")
                converted, ok = fix_for_at(original, off)
            elif kind == "nest":
                # A real diagnostic points at the declaration after x++.
                converted, ok = wrap_at(original.replace(" x++; int y=", " x++;\n int y="), 1)
                original = original.replace(" x++; int y=", " x++;\n int y=")
            else:
                converted, ok = hoist_extern_at(original.replace(" x++; extern", " x++;\n extern"), 2)
                original = original.replace(" x++; extern", " x++;\n extern")
            if not ok:
                print(f"self-test fixture {i}: rewrite declined", file=sys.stderr)
                return 1
            tasks.append((i, original, converted))

        def run_fixture(task):
            idx, orig, conv = task
            for label, source, standard in (("original", orig, "c99"),
                                            ("converted", conv, "c89")):
                src, exe = root / f"{idx}-{label}.c", root / f"{idx}-{label}"
                src.write_text(source, encoding="utf-8")
                compile_result = run(["clang", f"-std={standard}", "-Werror", "-pedantic", src, "-o", exe])
                if compile_result.returncode or run([exe]).returncode:
                    return f"self-test fixture {idx} {label} failed: {compile_result.stderr}"
            return None

        with concurrent.futures.ThreadPoolExecutor(max_workers=len(tasks)) as pool:
            errors = [e for e in pool.map(run_fixture, tasks) if e is not None]
        if errors:
            for err in errors:
                print(err, file=sys.stderr)
            return 1
    print(f"C89 transform self-test: {len(MASK_CASES)}/{len(MASK_CASES)} mask fixtures"
          f" + {len(fixtures)}/{len(fixtures)} rewrite fixtures PASS")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("limit", nargs="?", type=int, help="first N files (legacy pilot interface)")
    ap.add_argument("--out", type=Path, help="write successfully transformed copies here")
    ap.add_argument("--json", type=Path, help="write per-file acceptance report")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        return self_test()
    if not AGBCC.exists():
        print("c89_transform: agbcc not built", file=sys.stderr)
        return 2
    files = sorted(SRC.glob("*.c"))
    if args.limit is not None:
        files = files[:args.limit]

    def process_one(path: Path):
        fixed, stats = cached_transform_file(path)
        record = {"source": str(path.relative_to(ROOT)), **stats}
        return path, fixed, stats, record

    workers = min(os.cpu_count() or 4, len(files), 16)
    if len(files) <= 1:
        results = [process_one(f) for f in files]
    else:
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            results = list(pool.map(process_one, files))

    ok = 0
    totals = {"for": 0, "nest": 0, "extern": 0}
    failures = []
    records = []
    if args.out:
        args.out.mkdir(parents=True, exist_ok=True)
    for path, fixed, stats, record in results:
        records.append(record)
        if stats["unhandled"] is None:
            ok += 1
            for k in totals:
                totals[k] += stats[k]
            if args.out:
                dest = args.out / path.name
                if not dest.is_file() or dest.read_text(encoding="utf-8") != fixed:
                    dest.write_text(fixed, encoding="utf-8")
        else:
            failures.append((path.name, stats["unhandled"]))
    print(f"C89 transform over {len(files)} translation units\n")
    print(f"  compiles with agbcc after transform : {ok}/{len(files)}")
    print(f"  for-declarations hoisted            : {totals['for']}")
    print(f"  block tails wrapped                 : {totals['nest']}")
    print(f"  block-scope externs hoisted         : {totals['extern']}")
    if failures:
        print(f"\n  unresolved ({len(failures)}):")
        for name, why in failures[:20]:
            print(f"    {name:<28} {why}")
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps({"accepted": ok, "total": len(files),
                                         "totals": totals, "files": records},
                                        indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
