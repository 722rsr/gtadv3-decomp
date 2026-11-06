# `SoundVoiceFollow_NoteOff` (0x0802C390) is not pinned-agbcc C output

The 64-byte note-off walker is one of the `tst` bodies
`python3 tools/backlog_census.py --lift-screen` separates out. Re-measured
against the pinned compiler; both non-C instructions trace to the compiler
source, not to a missing C spelling.

```text
0802c390  b430        push {r4, r5}
0802c392  6c0a        ldr  r2, [r1, #64]
0802c394  7813        ldrb r3, [r2]
0802c396  2b80        cmp  r3, #128
0802c398  d203        bcs  0802c3a2
...
0802c3b0  4222        tst  r2, r4        @ r4 = 131
0802c3b4  422a        tst  r2, r5        @ r5 = 64
...
0802c3cc  bc30        pop  {r4, r5}
0802c3ce  4770        bx   lr
```

## 1. `tst rX, rY` has no Thumb pattern in the pinned tree

`gcc/thumb.md` has exactly one `tst`-named pattern, `tstsi`
(`define_insn "tstsi"`, line 818), and it prints `cmp %0, #0`. The
two-register `tst` only exists in the ARM-mode description
(`gcc_arm/config/arm/arm.md`, `*andsi3_compare0_scratch`). `git log -S
andsi3_compare0_scratch -- gcc/thumb.md` is empty over the pinned checkout's
history, so this is not a regression in the pin.

Measured on a scratch TU with `old_agbcc -O2 -mthumb-interwork`: the gate
spellings `(f & m) != 0`, `== 0`, bare, `!(f & m)`, `while`, `switch`, `u8`
vs `u32` mask, bitfield read, `(u8)(f & m)` all lower to `and` + `cmp #0`,
never `tst`. The body needs two `tst` instructions, so it needs two splices.

## 2. `push {r4,r5}` / `bx lr` is not a Thumb C frame

ROM frame: `push {r4, r5}` ... `pop {r4, r5}` / `bx lr`.

`gcc_arm/config/arm/thumb.c` `thumb_function_prologue`:

```c
for (regno = 0; regno < 8; regno++)
    if (regs_ever_live[regno] && ! call_used_regs[regno])
      live_regs_mask |= 1 << regno;

if (live_regs_mask || ! leaf_function_p () || far_jump_used_p())
    live_regs_mask |= 1 << 14;
```

r4/r5 are callee-saved, so keeping the `131`/`64` masks in them always adds
lr to the push. `output_return` then takes the `live_regs_mask != 0` arm and,
under `-mthumb-interwork`, emits `pop {r4,r5}` + `thumb_exit(-1)`
(`pop {r0}` / `bx r0`). Both pinned binaries produce:

| options | prologue | epilogue |
| --- | --- | --- |
| `-mthumb-interwork` | `push {r4,r5,lr}` | `pop {r4,r5}` / `pop {r0}` / `bx r0` (+4 B) |
| `-mno-thumb-interwork` | `push {r4,r5,lr}` | `pop {r4,r5,pc}` (no `bx lr`) |

## 3. What a C body does reach

A `naked` body can supply the frame by splicing `push`/`pop`/`bx lr` around
real C. With `u32 limit = 128; if (note < limit)` and pins
`cursor`→r2 / `note`→r3, the ROM head is reproduced instruction-for-instruction
(`ldr r2,[r1,#64]`, `ldrb r3,[r2]`, `cmp r3,#128`, `bcs`, `strb r3,[r1,#5]`,
`adds r2,#1`, `str r2,[r1,#64]`, `b n`, `ldrb r3,[r1,#5]`).

The chain walk is still not C-shaped. With the two gates spliced, `-O2` puts
the success arm out of line (`beq .Lfound` at +0x3C, `.Lfound` at +0x16)
instead of inline after the compare at +0x2C, and the body stays 72 B. The
`tst` gates also remain required.

A `naked` C body does not count as decompiled either:
`tools/source_quality.py` `classify_body` returns `asm` on the `naked`
attribute before it looks at asm/C ratios.

## Consequence

Under the pinned `old_agbcc` the body cannot be classified `c`/`c_fakematch`:
the compiler bytes still need the frame and the two `tst` gates, which only a
splice supplies. `partial` (non-naked C plus splices) is out too: a non-naked
body that keeps the `131`/`64` masks in r4/r5 gets the lr-saving frame, and a
body that keeps the loop in asm to protect the frame is `asm`-dominated by the
classifier (`insn_lines >= max(4, c_statements)`). A `naked` C body is
classified `asm` outright. So the byte-exact source can only be the listing.

`src/sound_voice_commands.c` keeps that listing and carries
`SoundVoiceFollow_NoteOffModel`, a readable C model of the same behavior that
is not gate evidence. Changing this needs a compiler decision under the
AGENTS.md exception (provenance evidence, agreed scope, full corpus
regression), not another C spelling.

## Tracking

The body stays in the `asm` row: not decompiled, and the C model is not
credited. `tools/backlog_census.py` classifies it `raw`, and `--lift-screen`
reports it in the separate `tst (current pattern lowers to and+cmp)` tally
rather than the straight-C lift list. Keep the body comment free of the
census's `regalloc`/`unresolved` keywords (`pinned`, `register alloc`,
`unreachable`, `does not emit`) when recording a compiler-boundary result, or
the row is mis-filed as ordinary allocation work.
