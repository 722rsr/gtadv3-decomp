#!/usr/bin/env python3
"""Small C source lexer helpers shared by matching and editing tools.

The corpus is maintained in the C89 dialect the pinned `old_agbcc` accepts.
Tools still need to scan source text without being fooled by braces or quotes
that live in comments and literals, so these helpers live in one place instead
of a compiler-rewrite module whose rewrite half no longer exists.
"""
from __future__ import annotations

import sys

__all__ = ["strip_noncode", "matching_delimiter"]

# (label, source, masked source with all whitespace removed)
# Each entry is a construct that can OPEN A RUN in a comment/string/char
# regex chain. The first two are negative controls: an apostrophe in a comment
# or in a string literal was never broken, because both are masked before the
# char-literal pass could ever see the quote. They are here so a future "fix"
# cannot regress them.
MASK_CASES = (
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


def strip_noncode(text: str) -> str:
    """Blank comments, string literals and char literals in ONE left-to-right pass.

    Four `re.sub` passes in a fixed order cannot do this, because every pass
    scans the ALREADY-masked text: whichever construct is masked first can open
    a run inside a construct of a different kind, and the wrong kind of quote
    then matches all the way to the next same-kind quote.

    A lexer also keeps the mask LINE-PRESERVING, which naive passes do not:
    `" " * len(...)` blanks newlines inside a multi-line run, so code that
    indexes `strip_noncode(text).splitlines()` into `text.splitlines()` would
    move every later line.

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


def self_test() -> int:
    """Check the comment/string/char mask on every construct that opens a run."""
    for label, source, want in MASK_CASES:
        masked = strip_noncode(source)
        # Every caller converts offsets on the mask back to offsets on the
        # source, and some index splitlines() of the mask into splitlines() of
        # the source, so both invariants are load-bearing.
        if len(masked) != len(source):
            print(f"self-test mask fixture {label}: mask is not length-preserving",
                  file=sys.stderr)
            return 1
        if [i for i, ch in enumerate(masked) if ch == "\n"] != \
                [i for i, ch in enumerate(source) if ch == "\n"]:
            print(f"self-test mask fixture {label}: mask moved a newline",
                  file=sys.stderr)
            return 1
        got = "".join(ch for ch in masked if not ch.isspace())
        if got != want:
            print(f"self-test mask fixture {label}: masked {got!r}, want {want!r}",
                  file=sys.stderr)
            return 1
    # The invariant `function_source` depends on: a char literal holding the
    # OTHER quote must not eat the function's closing brace, or
    # `matching_delimiter` returns None and the body is silently reported as
    # not found.
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
    print(f"C source mask self-test: {len(MASK_CASES)}/{len(MASK_CASES)} fixtures PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(self_test())
