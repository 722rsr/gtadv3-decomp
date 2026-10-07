#!/usr/bin/env python3
"""Cross-cutting regressions for the experimental strategy tools.

Each strategy tool self-tests its own algorithm. This suite checks the
contracts that span the whole set and that no single tool can verify alone --
which is exactly where this kind of tooling fails quietly: seven tools that
each work, disagreeing about what a status means, one of them importing a
third-party library the repository forbids, one of them quietly writing into
`src/`.

Checks, all static or ROM-free so the suite runs in the fast tier:

  * every strategy tool is stdlib-only;
  * none of them can write into `src/`, `asm/`, or the promotion manifest;
  * every one passes its own `--self-test`;
  * every one is documented in `tools/README.md`;
  * none redefines the pinned agbcc flag set;
  * the shared status vocabulary is the only vocabulary in use;
  * no tool runs a slow gate on its own.
"""
from __future__ import annotations

import ast
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / "tools"

#: The shared foundation plus every strategy tool and the router.
KIT = "experiment_kit"
ROUTER = "strategy_router"
STRATEGIES = {
    "compiler_microscope": "compiler microscope",
    "recipe_miner": "learned C grammar",
    "branch_synth": "branch-aware synthesis",
    "egraph_search": "equality saturation",
    "novelty_search": "novelty search",
    "cross_rom_archaeology": "cross-ROM archaeology",
    "asset_pipeline": "asset pipeline",
}
ALL_TOOLS = [KIT, ROUTER] + sorted(STRATEGIES)

#: Third-party modules that must never appear. The repository's Python is
#: stdlib-only, and an optional solver or e-graph engine has to be an
#: explicitly configured external executable, not an import.
FORBIDDEN_IMPORTS = {
    "z3", "cvc5", "sympy", "numpy", "scipy", "egglib", "egglog", "networkx",
    "pulp", "ortools", "requests", "yaml", "pytest",
}

#: Paths a strategy tool must never WRITE. Reading `baserom.gba` is the byte
#: oracle and reading the manifest is how a tool knows what is already done,
#: so the rule is about writes specifically.
PROTECTED = ("matching_slice_functions.json", "ldscript", "baserom.gba",
             "src/", "asm/", "Makefile")

#: Gates that are too expensive to appear in a research tool's own code.
SLOW_GATES = ("matching-ready", "match_c_slice.py --all", "make matching")

def is_local(name: str) -> bool:
    """Is this import one of this repository's own tools?"""
    return (TOOLS / f"{name}.py").exists()


def imports_of(tree: ast.AST) -> set[str]:
    names: set[str] = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Import):
            names.update(alias.name.split(".")[0] for alias in node.names)
        elif isinstance(node, ast.ImportFrom):
            if node.level == 0 and node.module:
                names.add(node.module.split(".")[0])
    return names


def docstring_nodes(tree: ast.AST) -> set[int]:
    """ids() of Constant nodes that are docstrings."""
    out: set[int] = set()
    for node in ast.walk(tree):
        if isinstance(node, (ast.Module, ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
            body = getattr(node, "body", [])
            if body and isinstance(body[0], ast.Expr) and \
                    isinstance(body[0].value, ast.Constant) and \
                    isinstance(body[0].value.value, str):
                out.add(id(body[0].value))
    return out


def code_strings(tree: ast.AST) -> list[str]:
    """String literals that are real code, not documentation."""
    skip = docstring_nodes(tree)
    return [node.value for node in ast.walk(tree)
            if isinstance(node, ast.Constant) and isinstance(node.value, str)
            and id(node) not in skip]


def subprocess_arguments(tree: ast.AST) -> list[str]:
    """String literals that are actually handed to a subprocess."""
    out: list[str] = []
    for node in ast.walk(tree):
        if not isinstance(node, ast.Call):
            continue
        func = node.func
        name = getattr(func, "attr", None) or getattr(func, "id", None)
        owner = getattr(func, "value", None)
        if name is None or (owner is None and name not in
                            ("system", "popen", "run", "check_call", "check_output")):
            continue
        if owner is not None and getattr(owner, "id", "") != "subprocess":
            continue
        for arg in list(node.args) + [kw.value for kw in node.keywords]:
            out.extend(child.value for child in ast.walk(arg)
                       if isinstance(child, ast.Constant) and isinstance(child.value, str))
    return out


def write_arguments(tree: ast.AST) -> list[str]:
    """String literals handed to a call that WRITES a file.

    Reading `baserom.gba` is the whole point of a byte oracle, and reading
    the promotion manifest is how a tool knows what is already done. Flagging
    a read would force authors to stop using the repository's own evidence.
    What must never happen is a WRITE to `src/`, `asm/`, or the manifest.
    """
    writers = {"write_text", "write_bytes", "open", "writelines"}
    modes = {"w", "wb", "a", "ab", "x", "r+", "w+"}
    out: list[str] = []
    for node in ast.walk(tree):
        if not isinstance(node, ast.Call):
            continue
        name = getattr(node.func, "attr", None) or getattr(node.func, "id", None)
        if name not in writers:
            continue
        arguments = list(node.args) + [kw.value for kw in node.keywords]
        literals = [child.value for arg in arguments
                    for child in ast.walk(arg)
                    if isinstance(child, ast.Constant) and isinstance(child.value, str)]
        if name != "open" or any(m in modes for m in literals):
            out.extend(literals)
    return out

def main() -> int:
    failures: list[str] = []
    checked = 0

    def check(label: str, ok: bool, detail: str = "") -> None:
        nonlocal checked
        checked += 1
        if ok:
            print(f"  [PASS] {label}")
        else:
            failures.append(f"{label}{': ' + detail if detail else ''}")
            print(f"  [FAIL] {label}" + (f" -- {detail}" if detail else ""), file=sys.stderr)

    readme = (ROOT / "tools/README.md").read_text()

    for name in ALL_TOOLS:
        path = TOOLS / f"{name}.py"
        if not path.exists():
            check(f"{name}.py exists", False)
            continue
        check(f"{name}.py exists", True)
        text = path.read_text()
        tree = ast.parse(text)
        imported = imports_of(tree)

        # -- stdlib, or one of this repository's own tools ----------------
        external = {m for m in imported
                    if m not in FORBIDDEN_IMPORTS and not is_local(m)
                    and m not in sys.stdlib_module_names}
        check(f"{name} imports only stdlib and local tools", not external,
              ", ".join(sorted(external)))
        banned = {m for m in imported if m in FORBIDDEN_IMPORTS}
        check(f"{name} imports no optional third-party module", not banned,
              ", ".join(sorted(banned)))

        # -- cannot WRITE to a protected path ------------------------------
        # Only literals actually handed to a writing call count. A docstring
        # saying "never edits the manifest" is the opposite of a violation,
        # and flagging it would push authors to stop documenting the rule.
        writes = write_arguments(tree)
        protected = sorted({p for p in PROTECTED for s in writes if p in s})
        check(f"{name} never writes to a protected path", not protected,
              ", ".join(protected))

        # -- does not redefine the pinned flags ---------------------------
        if name != KIT:
            redefines = re.search(r'\bFLAGS\s*=\s*\([^)]*"-O[012s]', text)
            check(f"{name} does not redefine the pinned flag set", redefines is None)
        else:
            check("kit owns the pinned flag set",
                  'AGBCC_FLAGS = ("-O2", "-mthumb-interwork", "-ffunction-sections")'
                  in text.replace("\n", " "))

        # -- does not shell out to a slow gate ----------------------------
        # Again only invocations count: telling the user to run
        # `make matching-ready` is the opposite of running it.
        invoked = subprocess_arguments(tree)
        slow = sorted({g for g in SLOW_GATES for s in invoked if g in s})
        check(f"{name} never invokes a slow gate", not slow, ", ".join(slow))

        # -- documented ---------------------------------------------------
        check(f"{name} is documented in tools/README.md", name in readme)

        # -- own self-test passes ----------------------------------------
        result = subprocess.run([sys.executable, str(path), "--self-test"],
                                text=True, capture_output=True)
        check(f"{name} --self-test exits 0", result.returncode == 0,
              (result.stderr or result.stdout).strip().splitlines()[-1]
              if (result.stderr or result.stdout).strip() else "")

    # -- shared vocabulary ------------------------------------------------
    kit_text = (TOOLS / f"{KIT}.py").read_text()
    for name in STRATEGIES:
        text = (TOOLS / f"{name}.py").read_text()
        # A strategy may invent its own STATUS for a domain the C vocabulary
        # does not cover (asset bytes). What it must not do is re-declare a
        # core status under a different spelling.
        shadowed = [s for s in ("EXACT_DRAFT", "NO_EXACT_DRAFT", "TOOL_FAILURE")
                    if re.search(rf'["\']?{s}\s*=\s*["\']', text)]
        check(f"{name} does not redefine a core status", not shadowed,
              ", ".join(shadowed))
        uses_kit = "experiment_kit" in text or "import kit" in text
        check(f"{name} draws its vocabulary from the shared kit", uses_kit)

    # -- the kit's own statuses are the canonical ones --------------------
    for status in ("EXACT_DRAFT", "NO_EXACT_DRAFT", "UNSUPPORTED_CONTRACT",
                   "DEPENDENCY_MISSING", "SOLVER_TIMEOUT", "BUDGET_EXHAUSTED",
                   "TOOL_FAILURE", "COMPILE_ERROR"):
        check(f"kit defines {status}", status in kit_text)

    # -- `--out` names the run directory, and it must actually be filled ----
    # `EvidenceWriter` lays out `<root>/<run-id>/`, so `root=out.parent` with
    # an auto-generated run id writes to a SIBLING of the directory the caller
    # named, leaving the named one empty and reporting no error. That happened:
    # `--out build/experiments/recipe-pilot` filled a timestamped directory
    # instead. Pinned here so the three tools cannot drift back.
    def relocates(text: str) -> list[int]:
        """`EvidenceWriter(...)` calls that take a `root=` with no run id.

        AST rather than a substring: a text rule matches the comments that
        explain the rule, and it cannot tell a genuine defect from
        `EvidenceWriter(S, out.name, root=out.parent)`, which passes an
        explicit run id and so lands on `out` correctly.
        """
        hits = []
        for node in ast.walk(ast.parse(text)):
            if not isinstance(node, ast.Call):
                continue
            func = node.func
            called = func.attr if isinstance(func, ast.Attribute) else getattr(func, "id", "")
            if called != "EvidenceWriter":
                continue
            # Signature is (strategy, run_id=None, *, root=None).
            if len(node.args) >= 2:
                continue
            if any(kw.arg == "root" for kw in node.keywords):
                hits.append(node.lineno)
        return hits

    moved = [f"{name}:{line}" for name in STRATEGIES
             for line in relocates((TOOLS / f"{name}.py").read_text())]
    check("no strategy relocates evidence out of its own --out", not moved,
          ", ".join(moved))
    sys.path.insert(0, str(TOOLS))
    import experiment_kit as kit  # noqa: E402
    with tempfile.TemporaryDirectory() as tmp:
        target = Path(tmp) / "named-by-the-caller"
        writer = kit.run_writer("selftest", target)
        check("run_writer's run directory IS the --out path",
              writer.dir == target, f"{writer.dir} != {target}")

    # -- the router covers every triage band ------------------------------
    router_text = (TOOLS / f"{ROUTER}.py").read_text()
    sys.path.insert(0, str(TOOLS))
    import lift_scout  # noqa: E402
    missing = [band for band in lift_scout.BANDS if band not in router_text]
    check("router references every lift_scout band", not missing, ", ".join(missing))

    print(f"\n{checked - len(failures)}/{checked} passed")
    if failures:
        print("\nFAILURES:", file=sys.stderr)
        for item in failures:
            print(f"  - {item}", file=sys.stderr)
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
