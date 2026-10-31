#!/usr/bin/env python3
"""Exhaustive source-shape sweep for SaveUpdate_080241C8.

Splices each generated body into src/save.c (restoring from git first), scores it
with the acceptance probe, and logs to ledger2.jsonl.  Stops on EXACT, then runs
the real gate.  Always restores src/save.c from git on a failed compile.

Usage: sweep.py [--max N]
"""
from __future__ import annotations

import itertools
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
SRC = ROOT / "src/save.c"
WORK = ROOT / "build/lift/save241c8"
LEDGER = WORK / "ledger2.jsonl"
DONE = WORK / "DONE2"
TEMPLATE = WORK / "template.c"

sys.path.insert(0, str(WORK))
import score as scorer  # noqa: E402


def kzero(gorder, cmpexpr):
    gA = "g = g + (u32)&SaveWAOffNegFFA;"
    gB = "g = g + (u32)&SaveWAOffNegFFA2;"
    cmpA = f"""if ({cmpexpr}) {{
                            o = (u32)&SaveWAOffFF6;
                            pa = o;
                            pa += wa;
                            *(u16 *)(uintptr_t)pa = 3;
                        }}"""
    cmpB = f"""if ({cmpexpr}) {{
                            o = (u32)&SaveWAOffFF6;
                            pa = o;
                            pa += wa;
                            *(u16 *)(uintptr_t)pa = 0;
                            o = (u32)&SaveWAOffFF2;
                            pa = o;
                            pa += wa;
                            *(u16 *)(uintptr_t)pa = 1;
                        }}"""
    if gorder == "before":
        armA = gA + "\n                        " + cmpA
        armB = gB + "\n                        " + cmpB
    else:
        armA = cmpA + "\n                        " + gA
        armB = cmpB + "\n                        " + gB
    return f"""                if (k == 0) {{
                    if (*(s16 *)wastate == 0) {{
                        {armA}
                    }} else {{
                        {armB}
                    }}
                }} else {{
                    if (*wastate == 1 && {cmpexpr}) {{
                        o = (u32)&SaveWAOffFF6;
                        pa = o;
                        pa += wa;
                        *(u16 *)(uintptr_t)pa = 3;
                    }}
                    *wastate = 1;
                    o = (u32)&SaveWAOffFF2;
                    pa = o;
                    pa += wa;
                    *(u16 *)(uintptr_t)pa = 1;
                    g = pa + (u32)&SaveWAOffNegFF2;
                    pa = 0;
                }}"""


def build(t, kdecl, loop, knext, c2p, idxp, cp, countst, gorder, flagv):
    t = t.replace("@KDECL@", '    register s32 k __asm__("r7");' if kdecl == "pin" else "    s32 k;")
    t = t.replace("@C2DECL@", '    register s32 c2 __asm__("r2");' if c2p == "pin" else "    s32 c2;")
    t = t.replace("@IDXDECL@", '    register s32 idx __asm__("r3");' if idxp == "pin" else "    s32 idx;")
    t = t.replace("@CDECL@", '    register s32 c __asm__("r5");' if cp == "pin" else "    s32 c;")

    t = t.replace("@LOOP@", "    while (k <= 1) {" if loop == "while" else "    for (k = 0; k <= 1; k++) {")

    if knext == "struct":
        t = t.replace("@KDECL2@", "    struct { s32 k; u32 pad; } kf;")
        t = t.replace("@KNEXT@", "        kf.k = (s32)(k + 1);")
        kread = "        k = kf.k;"
    elif knext == "plain":
        t = t.replace("@KDECL2@", "    s32 knext;\n    struct { s32 pad0; u32 pad; } kf;")
        t = t.replace("@KNEXT@", "        knext = (s32)(k + 1);")
        kread = "        k = knext;"
    else:
        t = t.replace("@KDECL2@", "    struct { s32 pad0; u32 pad; } kf;")
        t = t.replace("@KNEXT@", "")
        kread = "        k++;"
    if loop == "for":
        t = t.replace("    k = 0;\n", "")
        kread = ""

    if countst == "inline":
        store = ("                *(u16 *)(uintptr_t)((uintptr_t)wa + "
                 "(uintptr_t)&SaveWAOffFF6) = (u16)(c + 1);")
        spill = ""
        c2read = "c2 + k8"
    else:
        store = ("                o = (u32)&SaveWAOffFF6;\n"
                 "                pa = o;\n"
                 "                pa += wa;\n"
                 "                *(u16 *)(uintptr_t)pa = (u16)(c + 1);")
        spill = ""
        c2read = "c2 + k8"
    t = t.replace("@COUNTSTORE@", store)
    t = t.replace("@SPILL@", spill)
    t = t.replace("@C2READ@", c2read)

    if flagv == "var":
        t = t.replace("@FLAGSTORE@",
                      "                onev = 1;\n"
                      "                *(u16 *)(uintptr_t)(idx + pa) = (u16)onev;")
    else:
        t = t.replace("@FLAGSTORE@", "                *(u16 *)(uintptr_t)(idx + pa) = 1;")

    t = t.replace("@KZERO@", kzero(gorder, "c + 1 > 3"))
    t = t.replace("@CREAD@", "            c++;")
    t = t.replace("@KREAD@", kread)
    if flagv == "var":
        t = t.replace("    u32 pa;", "    u32 pa;\n    u32 onev;")
    return t


def variants():
    base = TEMPLATE.read_text()
    for kdecl, loop, knext, c2p, idxp, cp, countst, gorder, flagv in itertools.product(
        ["pin", "plain"],
        ["while", "for"],
        ["struct", "plain", "none"],
        ["pin", "plain"],
        ["pin", "plain"],
        ["pin", "plain"],
        ["inline", "opa"],
        ["after", "before"],
        ["lit", "var"],
    ):
        key = "|".join([kdecl, loop, knext, c2p, idxp, cp, countst, gorder, flagv])
        yield key, build(base, kdecl, loop, knext, c2p, idxp, cp, countst, gorder, flagv)


def probe_line():
    out = subprocess.run(
        ["python3", "tools/corpus_match_probe.py", "--function",
         "SaveUpdate_080241C8", "--c89"],
        cwd=ROOT, capture_output=True, text=True).stdout
    return next((l for l in out.splitlines() if "0x080241c8" in l and "bytes" in l), "")


def parse(line):
    m = re.search(r":\s+([A-Z_]+)\s+(\d+)/(\d+) bytes, candidate (\d+) bytes, first difference (\S+)", line)
    if not m:
        return -1, 0, 0, "ERR"
    status, matched, span, clen, first = m.groups()
    fd = 0x170 if first == "none" else (int(first[1:], 16) if first.startswith("+") else -1)
    return fd, int(clen), int(matched), status


def main():
    seen = set()
    if LEDGER.exists():
        for line in LEDGER.read_text().splitlines():
            try:
                seen.add(json.loads(line)["key"])
            except Exception:
                pass
    limit = None
    if "--max" in sys.argv:
        limit = int(sys.argv[sys.argv.index("--max") + 1])
    n = 0
    for key, body in variants():
        if key in seen:
            continue
        if limit is not None and n >= limit:
            break
        (WORK / "cand.c").write_text(body)
        scorer.splice(body)
        line = probe_line()
        fd, clen, matched, status = parse(line)
        rec = {"key": key, "first": fd, "len": clen, "matched": matched, "status": status}
        with LEDGER.open("a") as h:
            h.write(json.dumps(rec) + "\n")
        print(json.dumps(rec), flush=True)
        n += 1
        if status == "EXACT":
            DONE.write_text(key + "\n")
            print("EXACT:", key, flush=True)
            gate = subprocess.run(["make", "matching-slice"], cwd=ROOT,
                                  capture_output=True, text=True)
            (WORK / "gate.log").write_text(gate.stdout + gate.stderr)
            print("gate rc", gate.returncode, flush=True)
            if gate.returncode == 0:
                (WORK / "gate.ok").write_text(key + "\n")
            return 0 if gate.returncode == 0 else 3
    subprocess.run(["git", "checkout", "--", "src/save.c"], cwd=ROOT)
    rows = []
    for line in LEDGER.read_text().splitlines():
        try:
            rows.append(json.loads(line))
        except Exception:
            pass
    rows.sort(key=lambda r: (-r["first"], abs(r["len"] - 368)))
    for r in rows[:10]:
        print("BEST", json.dumps(r), flush=True)
    return 1


if __name__ == "__main__":
    sys.exit(main())
