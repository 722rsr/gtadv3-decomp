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
pair is reachable; an earlier revision of this note called it impossible.

What stays out of reach is the *other* instruction in these bodies: `tst rX, rY`.
Sixteen gate spellings (`(f & m) != 0`, `== 0`, bare `(f & m)`, `!(f & m)`,
`while`, `switch`, a `u32`/`u8` mask operand, a bitfield read, `(u8)(f & m)`)
all expand to `ands` + `cmp #0`; agbcc has no `tstsi` lowering that survives to
assembly. Both bodies that need `cmp #128`/`bcs` also contain `tst`, so they
stay asm -- for the `tst` reason, not the range test.

Consequence: a body containing `cmp #128` + `bcs` **and no `tst`** is worth
lifting with the named-bound spelling above; `python3
tools/thumb_idiom_scan.py --exclude tst` lists exactly those (currently none).
Of the 24 not-yet-decompiled bodies that contain `cmp #128`
(`--idiom cmp128any`), only `SoundVoiceFollow_NoteOff` and `SoundNoteOn_2C190`
use the `bcs` form -- the other 22 compare for equality, which needs no special
spelling. `--idiom tst` shows the six bodies the missing `tst` lowering blocks
outright: the two sound walkers above, `SoundChannelFreeAll`, the two
`DivRem*` runtime routines and `MixerSetup_2B8AA`. The scanner decodes
literal-pool words as data: without that, a pool word such as `0x4200xxxx`
reports a `tst` the body never executes, which is how an earlier revision of
this note arrived at "24 bodies contain the sequence".

`SoundVoiceFollow_NoteOff` (`0x0802C390`, 64 B) is the smallest measured body
where the named bound reproduces the range test and the `tst r2, r4` for its
`(flags & 0x83)` / `(flags & 0x40)` gates is what still blocks it
(`docs/matching_workflow.md`: agbcc's `tstsi` always expands to `and` +
`cmp #0`). With the bound in a local the head of the lift emits the ROM's
`ldrb` / `cmp #128` / `bcs` sequence (register homes and the merged
load-and-store still differ), so only the two gates keep it asm; the same is
expected of the other byte-gated sound walkers.

Related narrowing already documented: `volatile u16` reads force
`ldrh`+extend while plain `s16` reads take `ldrsh`
(`docs/matching_workflow.md`); this note covers the *compare* that follows the
byte load, not the load itself.
