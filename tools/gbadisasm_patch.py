#!/usr/bin/env python3
"""Patch gbadisasm to emit numeric branch targets instead of asserting.

Upstream gbadisasm asserts when a decoded branch targets an address with
no label; on this ROM that aborts output mid-print for data regions read
as code. Numeric absolute targets reassemble to identical encodings, so
falling back to them keeps round-trip byte-identity.

Usage: python3 gbadisasm_patch.py <gbadisasm-source-dir>
Idempotent: exits 0 without changes if already applied.
"""
import sys
import pathlib

OLD = """            struct Label *label = lookup_label(target);

            assert(label != NULL);  // We should have found this label in the analysis phase
            if (label->name != NULL)"""

NEW = """            struct Label *label = lookup_label(target);

            if (label == NULL)
            {
                // PATCH(gtadv-decomp): no label (misdecoded data or target
                // outside emitted range). Emit an absolute target; gas
                // reassembles it to the identical encoding.
                printf("\\t%s 0x%08X\\n", insn->mnemonic, target);
                return;
            }
            if (label->name != NULL)"""


def main():
    src = pathlib.Path(sys.argv[1]) / "disasm.c"
    text = src.read_text()
    if "PATCH(gtadv-decomp)" in text:
        print("already patched")
        return
    if OLD not in text:
        sys.exit("pattern not found - upstream changed?")
    src.write_text(text.replace(OLD, NEW, 1))
    print("patched", src)


if __name__ == "__main__":
    main()
