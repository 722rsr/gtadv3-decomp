# `SoundVoiceFollow_NoteOff` (0x0802C390)

The fresh `backlog_census.py --lift-screen` marks this 64-byte body for its
`tst` instructions. Treat that marker as a splice route, not as a compiler
limit.

The promoted source in `src/sound_voice_commands.c` measures **64/64 bytes
exact** with `tools/lift_loop.py` and the pinned `old_agbcc`. C emits the
cursor decode/update, child pointer load, null check, next-link load, and loop
back edge. Assembly keeps the paired `push/pop {r4,r5}` frame, mask setup,
`tst` gates, key comparison, and success update. The compiler emits the final
`bx lr`. Source quality classifies the body as `partial`, so it is not yet
credited as decompiled.

An all-C loop draft with separate gate splices put the success arm out of line
and aligned only 26 bytes. Grouping the gates, key comparison, and success arm
while leaving link traversal in C produced the exact body. MEASURED
2026-11-28: splitting the `tst` gates into a splice and returning the flag byte
in r2 lets C emit the key comparison and update, but the best shape aligns only
28/64 bytes; the success update lands after the loop and the key branch points
to that out-of-line block. Next lead: shape the C loop so a key mismatch branches
to the next-link step while the matching update stays inline before the loop
exit, then re-measure the fixed 64-byte span.
