# Register-indexed byte loads with a distinct destination

`VoiceEnvelopeApply_2C160` (`src/sound_channel_be14.c`, ROM `0x0802C160`) is
the only place in the ROM where a signed byte read uses a register index that
is not also the destination:

```text
a17c      ldrb  r1, [r4, #18]
1420      movs  r0, #20
2256      ldrsb r2, [r4, r0]      <- index r0, destination r2
```

The pinned `old_agbcc` cannot select this pair from C. Two independent limits
combine:

1. **The constant-offset lowering reuses the destination as the index.**
   `*extendqisi2_insn` in `thumb.md` emits
   `mov %0, %2; ldrsb %0, [%1, %0]` when the address is base + constant, so
   the best constant-index candidate is
   `movs r2, #20; ldrsb r2, [r4, r2]` (46/48 bytes, `PARTIAL` at +0x3).
   The direct `ldrsb %0, [%1, %2]` template is only reached when the RTL
   address is a genuine `(plus base index)` with a register index.
2. **REG+REG memory addresses never form in this compiler.** `thumb.h`
   `GO_IF_LEGITIMATE_ADDRESS` admits `(plus reg reg)` only once
   `reload_completed`, and no post-reload pass creates the form after the
   fact (`reload_cse_regs` compares loaded values, not addresses;
   `thumb_reorg` only places constant pools). A scratch TU with an opaque
   index proves it for both shipped binaries:

   ```c
   typedef unsigned char u8;
   u32 F_byte(u8 *p, u32 i) { return (u32)((s8 *)p)[i]; }
   ```

   emits `add r0, r0, r1` followed by `ldrb r0, [r0, #0]` (plus the
   `lsl #24 / asr #24` extension), never `ldrsb rX, [rB, rI]`.

Register-index lemma: a register that holds the constant 20 at the load is
either a folded constant (limit 1, destination reused) or an opaque value
whose address is first copied to a register (limit 2, extra `add`). Shape
experiments along both routes -- a pinned `r0` offset with a leading `"=r"`
barrier, a `"+r"` barrier before or after the read, and a volatile register
local -- all regress or stay destination-indexed; the committed one-asm
splice is the only byte-exact representation. Corpus check: of 102 `ldrsb`
instructions in `asm/*.s`, 101 have index == destination; this is the only
exception. The one reg+reg word `ldr` in the generated corpus comes from the
naked asm body `SoundBeb4Worker`, not from C.

Keep this function's byte read as the existing single-instruction `partial`
splice; do not re-spend source permutations or register pins on it. Pins can
move which register plays which role, but not the destination-is-index
structure of the constant path.

## A plain `if (*(s8 *)p == 0)` collapses to the shift pair

A signed byte read used only in a zero test is folded to a QImode compare, so
agbcc emits `ldrb r0,[p] / lsls r0,#24 / asrs r0,#24 / cmp r0,#0` where the
ROM (armcc) has the shorter `movs r0,#0 / ldrsb r0,[p,r0] / cmp r0,#0`.
Spelling the load into an `int`, an `s8`, a `!(...)`, a `volatile` or an empty
`"+r"` barrier on the value all still produce the shift pair.

What flips it is **pinning the base pointer** so it stays live across the
load. Measured on `_0800BD40` (224 B, results progress updater), whose two
`*(s8 *)(wa + 0x10E4)` flag checks need the ROM form:

```c
register u8 *p __asm__("r3") = wa + 0x10E4;   /* ROM's address register */
rec = p;                                      /* keeps the pointer live */
if (*(s8 *)p == 0) goto clear;
```

That emits `mov r3, ... / add r3, r8 / mov r9, r3 / movs r0, #0 / ldrsb
r0, [r3, r0]`. Without the pin, the address is a throwaway pseudo and the
load keeps the shift pair. The index is the destination here, so this is the
same destination-is-index path as above, not the distinct-destination case.

## Reproduction

```sh
python3 tools/corpus_match_probe.py --function _0802C160 --c89 \
    --require-all --require-exact
```

The probe reports `PARTIAL 46/48` for plain-C read shapes and `EXACT 48/48`
with the retained asm splice. For the REG+REG argument, compile the scratch
snippet above with `build/toolchains/agbcc/old_agbcc -O2 -mthumb-interwork`
(or the comparison `agbcc` binary; both agree) and inspect the emitted
address sequence.
