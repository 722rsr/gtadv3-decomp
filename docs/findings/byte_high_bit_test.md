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

## The `tst` half is a backend-pattern absence, not a source-shape wall

The *other* instruction in these bodies, `tst rX, rY`, is not reachable from C
with the pinned compiler for a different reason: the Thumb machine description
has no two-register `tst` pattern. `thumb.md`'s only `tst`-named pattern is
`tstsi`, which emits `cmp %0, #0`; the `tst`-emitting
`*andsi3_compare0_scratch` pattern belongs to the ARM-mode description, which
`-mthumb` does not use. All five `thumb*.md` copies in the pinned tree agree on
this point.

The sixteen gate spellings measured so far (`(f & m) != 0`, `== 0`, bare
`(f & m)`, `!(f & m)`, `while`, `switch`, a `u32`/`u8` mask operand, a bitfield
read, `(u8)(f & m)`) all expand to `and` + `cmp #0`, as expected from that
description. Compiling the same C with a backend that has the two-register
pattern (modern `arm-none-eabi-gcc -mthumb`, or the ARM-mode description) emits
`tst` immediately, so this is a backend-pattern absence, not a missing source
shape. C89/C99 declaration placement is irrelevant: the old C89 transform only
moved declarations and did not change instruction selection.

A body whose ROM stream contains `tst` is therefore asm/splice-only under the
pinned compiler. Keep it asm, or lift the rest of the body to C and splice the
gate explicitly; `source_quality.py` classifies the latter as `partial`, not
decompiled. Do not switch compilers for one body.

Consequence: a body containing `cmp #128` + `bcs` **and no lift-screen marker**
is worth lifting first with the named-bound spelling above; `python3
tools/thumb_idiom_scan.py --exclude tst` lists exactly those (currently none).
Of the 24 not-yet-decompiled bodies that contain `cmp #128`
(`--idiom cmp128any`), only `SoundVoiceFollow_NoteOff` and `SoundNoteOn_2C190`
use the `bcs` form -- the other 22 compare for equality, which needs no special
spelling. `--idiom tst` lists the six bodies that carry the test: the two sound
walkers above, `SoundChannelFreeAll`, the two `DivRem*` runtime routines and
`MixerSetup_2B8AA`. They are asm/splice-only, not targets for another C shape.
The scanner decodes literal-pool words as data: without that, a pool word such
as `0x4200xxxx` reports a `tst` the body never executes, which is how an
earlier revision of this note arrived at "24 bodies contain the sequence".

`SoundVoiceFollow_NoteOff` (`0x0802C390`, 64 B) is the smallest measured body
where the named bound reproduces the range test. With the bound in a local the
head of the lift emits the ROM's `ldrb` / `cmp #128` / `bcs` sequence (register
homes and the merged load-and-store still differ). The two `tst` gates
(`(flags & 0x83)` / `(flags & 0x40)`) cannot be produced by the pinned Thumb
backend, so the body stays asm-owned or becomes `partial` with an explicit gate
splice.

Related narrowing already documented: `volatile u16` reads force
`ldrh`+extend while plain `s16` reads take `ldrsh`
(`docs/matching_workflow.md`); this note covers the *compare* that follows the
byte load, not the load itself.
