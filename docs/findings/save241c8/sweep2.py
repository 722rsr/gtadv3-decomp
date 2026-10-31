#!/usr/bin/env python3
"""Second sweep for SaveUpdate_080241C8: the c2 spill at function offset 0x3E.

Varies how the value of `c2` is kept across the if/else (the ROM stores it at
sp+4 and reloads it at 0xEC), together with the register pins and the count
store form.  Scores with the acceptance probe, logs ledger3.jsonl, stops on
EXACT and runs the real gate.

Usage: sweep2.py [--max N]
"""
from __future__ import annotations

import itertools
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
WORK = ROOT / "build/lift/save241c8"
LEDGER = WORK / "ledger3.jsonl"
DONE = WORK / "DONE3"
TEMPLATE = WORK / "template.c"

sys.path.insert(0, str(WORK))
import score as scorer  # noqa: E402
from sweep import kzero  # noqa: E402


def build(t, kdecl, knext, c2p, spill, countst, gorder, flagv):
    t = t.replace("@KDECL@", '    register s32 k __asm__("r7");' if kdecl == "pin" else "    s32 k;")
    t = t.replace("@C2DECL@", '    register s32 c2 __asm__("r2");' if c2p == "pin" else "    s32 c2;")
    t = t.replace("@IDXDECL@", "    s32 idx;")
    t = t.replace("@CDECL@", "    s32 c;")
    t = t.replace("@LOOP@", "    while (k <= 1) {")

    if knext == "struct":
        t = t.replace("@KDECL2@", "    struct { s32 k; u32 pad; } kf;")
        t = t.replace("@KNEXT@", "        kf.k = (s32)(k + 1);")
        kread = "        k = kf.k;"
    else:
        t = t.replace("@KDECL2@", "    s32 knext;\n    struct { s32 pad0; u32 pad; } kf;")
        t = t.replace("@KNEXT@", "        knext = (s32)(k + 1);")
        kread = "        k = knext;"

    if spill == "none":
        t = t.replace("@SPILL@", "")
        c2read = "c2 + k8"
    elif spill == "struct":
        t = t.replace("    struct { s32 k; u32 pad; } kf;", "    struct { s32 k; u32 c2; } kf;")
        t = t.replace("@SPILL@", "            kf.c2 = (u32)c2;")
        c2read = "(s32)kf.c2 + k8"
    elif spill == "scalar":
        t = t.replace("    struct { s32 k; u32 pad; } kf;", "    struct { s32 k; } kf;\n    u32 c2s;")
        t = t.replace("@SPILL@", "            c2s = (u32)c2;")
        c2read = "(s32)c2s + k8"
    else:  # volatile
        t = t.replace("    struct { s32 k; u32 pad; } kf;", "    struct { s32 k; } kf;\n    volatile u32 c2v;")
        t = t.replace("@SPILL@", "            c2v = (u32)c2;")
        c2read = "(s32)c2v + k8"
    t = t.replace("@C2READ@", c2read)

    if countst == "inline":
        store = ("                *(u16 *)(uintptr_t)((uintptr_t)wa + "
                 "(uintptr_t)&SaveWAOffFF6) = (u16)(c + 1);")
    else:
        store = ("                o = (u32)&SaveWAOffFF6;\n"
                 "                pa = o;\n"
                 "                pa += wa;\n"
                 "                *(u16 *)(uintptr_t)pa = (u16)(c + 1);")
    t = t.replace("@COUNTSTORE@", store)

    if flagv == "var":
        t = t.replace("@FLAGSTORE@",
                      "                onev = 1;\n"
                      "                *(u16 *)(uintptr_t)(idx + pa) = (u16)onev;")
        t = t.replace("    u32 pa;", "    u32 pa;\n    u32 onev;")
    else:
        t = t.replace("@FLAGSTORE@", "                *(u16 *)(uintptr_t)(idx + pa) = 1;")

    t = t.replace("@KZERO@", kzero(gorder, "c + 1 > 3"))
    t = t.replace("@CREAD@", "            c++;")
    t = t.replace("@KREAD@", kread)
    return t


def variants():
    base = TEMPLATE.read_text()
    for kdecl, knext, c2p, spill, countst, gorder, flagv in itertools.product(
        ["pin", "plain"],
        ["struct", "plain"],
        ["pin", "plain"],
        ["none", "struct", "scalar", "volatile"],
        ["inline", "opa"],
        ["after", "before"],
        ["lit", "var"],
    ):
        key = "|".join([kdecl, knext, c2p, spill, countst, gorder, flagv])
        yield key, build(base, kdecl, knext, c2p, spill, countst, gorder, flagv)


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
        (WORK / "cand3.c").write_text(body)
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
            (WORK / "gate3.log").write_text(gate.stdout + gate.stderr)
            print("gate rc", gate.returncode, flush=True)
            if gate.returncode == 0:
                (WORK / "gate3.ok").write_text(key + "\n")
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
