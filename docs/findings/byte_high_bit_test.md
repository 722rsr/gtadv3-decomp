# A byte's high-bit range test: `cmp rX, #128` / `bcs` is not reachable

The sound cursor parsers test "is this byte a note id or a command" with a
single load and an unsigned range compare:

```asm
ldrb  r3, [r2]
cmp   r3, #128        @ 0x2B80
bcs   <command arm>   @ unsigned: r3 >= 0x80
strb  r3, [r1, #5]    @ note id path keeps the same byte
```

`SoundVoiceFollow_NoteOff` (`0x0802C390`) and `SoundNoteOn_2C190`
(`0x0802C190`, three of these) both do it. The pinned `old_agbcc` never emits
that pair for a range test. Measured against a scratch TU
(`build/snip.c` -> `clang -E` -> `old_agbcc -O2`), every spelling lowers to one
of these three instead:

| C spelling | lowering |
| --- | --- |
| `u8 id = *p; if (id < 0x80) …` | `ldrb r2,[r0]` **reload** `ldrb r0,[r0]` `lsl #24` `asr #24` `cmp #0` `blt` |
| same, `>= 0x80`, `128 > id`, `(s8)id < 0`, `signed char id`, `char id` | same sign-extend test, branch polarity flipped |
| `u32/int/u16 id = *p; if (id < 0x80)` | single `ldrb`, `cmp #127`, `bhi` (unsigned) / `bgt` (signed) |
| `if (id & 0x80)`, `if ((id & 0x80) == 0)` | `mov r0,#128` `and r0,r0,rX` `cmp r0,#0` |
| `if ((u8)(id + 0x80) < 0x80)`, `(id >> 7) != 0` | extra `add`, then one of the above |
| `struct { u8 b; }` field read | same as the plain `u8` local: reload + sign test |

So the compiler has exactly two byte-range canonical forms (`cmp #127` plus a
direction, or an `lsl/asr` sign test), and the ROM's `cmp #128` + `bcs` is a
third one it cannot produce. `cmp rX, #128` *does* appear in C-derived bodies in
this tree -- `_080097E8` (`if (k == 128)`), `RaceVM_022C18`,
`SoundChannel_WalkFree` -- but only as an **equality** compare, which is a
different operation and does lower correctly.

Consequence: a leftover body containing the `ldrb` / `cmp #128` / `bcs`
sequence is not a source-shaping problem. Do not spend permutations on it; keep
the naked transcription. 24 leftover bodies contain the instruction (1-3 times
each), e.g. `MenuFF78_133E4`, `MenuFF78_1573C`, `Rec35_Input_17788`,
`MenuE15C_0800E15C`, `_08001EE2C`.

`SoundVoiceFollow_NoteOff` (`0x0802C390`, 64 B) needs two things agbcc cannot
emit in the same body: the range test above *and* `tst r2, r4` for its
`(flags & 0x83)` / `(flags & 0x40)` gates (`docs/matching_workflow.md`: agbcc's
`tstsi` always expands to `and` + `cmp #0`). A 64-byte body that needs both is
the smallest measured instance of that pair; expect the same for
the other byte-gated sound walkers.

Related narrowing already documented: `volatile u16` reads force
`ldrh`+extend while plain `s16` reads take `ldrsh`
(`docs/matching_workflow.md`); this note covers the *compare* that follows the
byte load, not the load itself.
