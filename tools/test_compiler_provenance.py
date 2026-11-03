#!/usr/bin/env python3
"""Guard the one-compiler policy and the known asm/splice-only bodies.

`old_agbcc`'s Thumb machine description has no two-register `tst` pattern
(`tstsi` emits `cmp %0, #0`), so a ROM body containing `tst` cannot be pure C
output from the pinned compiler. These six bodies are hand-written assembly or
explicit splices; promoting one to plain C would claim a match the compiler
cannot produce.

The VMA list is pinned here so the check runs without baserom.gba. Regenerate it
with `python3 tools/thumb_idiom_scan.py --idiom tst` when the ROM inventory
changes.
"""
import unittest

import source_quality

TST_BODIES = {
    0x0802B8AA: "MixerSetup_2B8AA",
    0x0802BC84: "SoundChannelFreeAll",
    0x0802C190: "SoundNoteOn_2C190",
    0x0802C390: "SoundVoiceFollow_NoteOff",
    0x0802DE9C: "DivRemS_02DE9C",
    0x0802DFE4: "DivRemU_02DFE4",
}


class CompilerProvenanceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rows = source_quality.classify(source_quality.ROOT)["functions"]

    def test_tst_bodies_stay_asm_or_partial(self):
        for vma, name in sorted(TST_BODIES.items()):
            key = f"0x{vma:08x}"
            with self.subTest(vma=key, name=name):
                row = self.rows.get(key)
                self.assertIsNotNone(
                    row, f"{name} {key} is not in the selected manifest"
                )
                self.assertEqual(row["name"], name)
                self.assertIn(
                    row["class"],
                    ("asm", "partial"),
                    f"{name} {key} is {row['class']}; a body containing `tst` is "
                    "asm/splice-only under old_agbcc's Thumb backend",
                )


if __name__ == "__main__":
    unittest.main()
