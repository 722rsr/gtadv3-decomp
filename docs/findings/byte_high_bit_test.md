# A byte's high-bit range test: `cmp rX, #128` / `bcs` needs a named bound

The sound cursor parsers test "is this byte a note id or a command" with a
single load and an unsigned range compare:

```asm
ldrb  r3, [r2]
cmp   r3, #128        @ 0x2B80
bcs   <command arm>   @ unsigned: r3 >= 0x80
strb  r3, [r1, #5]    @ note id path keeps the same byte
```

`SoundVoiceFollow_NoteOff` (`0x0802C390`) and `SoundNoteOn_2C190`
(`0x0802C190`, three of these) both do it. Measured against a scratch TU
(`clang -E` -> `old_agbcc -O2`), the ordinary spellings lower to one of these
instead:

| C spelling | lowering |
| --- | --- |
| `u8 id = *p; if (id < 0x80) …` | `ldrb r2,[r0]` **reload** `ldrb r0,[r0]` `lsl #24` `asr #24` `cmp #0` `blt` |
| same, `>= 0x80`, `128 > id`, `(s8)id < 0`, `signed char id`, `char id` | same sign-extend test, branch polarity flipped |
| `u32/int/u16 id = *p; if (id < 0x80)` | single `ldrb`, `cmp #127`, `bhi` (unsigned) / `bgt` (signed) |
| **`u32 id = *p; u32 limit = 128; if (id < limit)`** | **single `ldrb`, `cmp #128`, `bcs` -- the ROM's form** |
| `if (id & 0x80)`, `if ((id & 0x80) == 0)` | `mov r0,#128` `and r0,r0,rX` `cmp r0,#0` |
| `if ((u8)(id + 0x80) < 0x80)`, `(id >> 7) != 0` | extra `add`, then one of the above |
| `struct { u8 b; }` field read | same as the plain `u8` local: reload + sign test |

The two `u32` spellings differ only in how the bound reaches the compare: a
literal is canonicalised to `#127` (or, in QImode, to a sign test) before
`simplify_comparison` runs, while a bound held in a local is still a register
operand at that point and folds into the compare as `#128` afterwards. So the
pair is reachable; an earlier revision of this note called it out of reach,
which was wrong.

## `tst` bodies: measured spellings and splice route

`thumb.md`'s `tstsi` pattern emits `cmp %0, #0`; the measured straight-C
gate spellings (`(f & m) != 0`, `== 0`, bare `(f & m)`, `!(f & m)`,
`while`, `switch`, `u32`/`u8` mask operands, bitfield read, and
`(u8)(f & m)`) likewise compiled to `and` + `cmp #0` in the tested drafts.
This records those attempts; it does not close the bodies. For a body flagged
by `backlog_census.py --lift-screen`, continue with an explicit gate splice or
a grouped control-flow conversion, keeping C for the surrounding operations.

The earlier C89/C99 declaration-placement experiment only moved declarations
and did not change the tested instruction selection. It does not rule out other
source shapes or splice groupings.

Of the 24 not-yet-decompiled bodies that contain `cmp #128`
(`--idiom cmp128any`), only `SoundVoiceFollow_NoteOff` and `SoundNoteOn_2C190`
use the `bcs` form; the other 22 compare for equality. `--idiom tst` lists the
six bodies that carry the test: the two sound walkers above,
`SoundChannelFreeAll`, the two `DivRem*` runtime routines and
`MixerSetup_2B8AA`. Treat those entries as splice/grouped-control-flow work.
The scanner decodes literal-pool words as data: without that, a pool word such
as `0x4200xxxx` reports a `tst` the body never executes, which is how an
earlier revision of this note arrived at "24 bodies contain the sequence".

`SoundVoiceFollow_NoteOff` (`0x0802C390`, 64 B) now has an exact grouped-splice
source: a named bound, r2/r3 pins, and C for the cursor decode/update and full
linked-list traversal; assembly keeps the frame, mask setup, `tst` gates, key
comparison, and success update. It measures 64/64 bytes exact and classifies as
`partial`. The next lead is to move the key comparison and success update into
C while preserving the inline success arm; see
[sound_voice_follow_noteoff_lift.md](sound_voice_follow_noteoff_lift.md).

Related narrowing already documented: `volatile u16` reads force
`ldrh`+extend while plain `s16` reads take `ldrsh`
(`docs/matching_workflow.md`); this note covers the *compare* that follows the
byte load, not the load itself.
