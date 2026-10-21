# The indexed halfword scratch register, and three other measured walls

Worked while clearing the `partial` class (2026-10-19). Every number here was
measured with the pinned compiler:

```sh
python3 tools/corpus_match_probe.py --function NAME --c89 --require-all --require-exact
```

The `partial` class is the set of functions that are byte-exact but still
carry an instruction-emitting `asm()`. All 19 of them were byte-exact before
this session; the work is removing the splices, not fixing behaviour.

## The indexed signed-halfword load is reachable from C; its scratch register is not

`thumb.md`'s `*extendhisi2_insn` splits a `plus(base, const)` address into a
base and an offset and, when the offset is not a register, emits

```
mov  scratch, #k        @ scratch is the pattern's "=&l" match_scratch
ldrsh DST, [BASE, scratch]
```

so a **non-volatile** signed halfword lvalue produces the ROM's
register-indexed form. Verified in a scratch TU against `old_agbcc`
(`-O2 -mthumb-interwork`):

| C | emitted |
| --- | --- |
| `*(s16 *)(base + k)` | `movs rX,#k / ldrsh rY,[base,rX]` |
| `*(s16 *)base` | `movs rX,#0 / ldrsh rY,[base,rX]` |
| `*(volatile s16 *)` | `ldrh / lsls / asrs` |
| `*(s16 *)(base + idx)` with a runtime `idx` | `adds rX,base,idx / movs r1,#0 / ldrsh rX,[rX,r1]` |

The shape is therefore not the obstacle. **The scratch's hard register is.**
`local-alloc.c`'s `find_free_reg` tries the instruction's suggestion set
first and otherwise takes the lowest free register, and it reuses a register
as soon as the previous pseudo dies. agbcc therefore gives two consecutive
constant-index halfword loads *the same* scratch register, while the ROM's
compiler did not. Measured on three real bodies, each of which is otherwise
byte-exact with the splice replaced by plain C:

| body | bytes | ROM | agbcc | score |
| --- | ---: | --- | --- | --- |
| `_08006050` `Course_Scan` | 156 | `movs r2,#6` | `movs r1,#6` | 154/156 |
| `_08015620` `MenuFF78_15620` | 284 | `movs r2,#0` | `movs r0,#0` | 282/284 |
| `_080154E0` `MenuFF78_154E0` | 320 | `movs r1,#0` | `movs r0,#0` | 318/320 |

The rule is not "the ROM avoids r0": in `MenuFF78_15620` the ROM's scratch is
r2 and r0/r1 are free, and in `MenuFF78_154E0` it is r1 with r0 free. Nor is
it "the ROM avoids reuse": `Course_Scan` reuses nothing while
`_08006A50` `CollectNearby_06A50` reuses r2 for both of its index loads
(0x54 and 0x76) and agbcc gives it r2 then r3. The ROM's choice tracks the
register pressure of the original source, which is not recoverable from the
C spelling.

The zero-index variant of the same pattern *is* steerable: see
[register-shaping levers](register_shaping_levers.md), "C forms for signed
indexed halfword loads" and "In-place halfword load in
`Course_Dispatch_08BB8`" -- keeping the offset live with an empty `"+r"`
barrier stops agbcc from reusing its register for the freshly materialised
zero index. An output-only empty asm on its own (`__asm__ volatile ("" :
"=r"(t));`) does not keep a register busy across the load, and a
hard-register clobber (`__asm__ volatile ("" : : : "r1");`) only forces the
*clobbered* value out of its register, it does not remove the register from
the scratch's candidate set.

### The liveness pin removes a register from the scratch set

`_080020198` (592 B) closes with a zero-byte pin: declare a dummy in the
*unwanted* scratch register, define it with an output-only empty asm, put the
load next, and consume it with an input-only empty asm afterwards.

```c
register int hold __asm__("r3");
__asm__ volatile("" : "=r"(hold));
s28 = *(s16 *)(rec + 28);      // ROM scratch is r1, agbcc picks r3
__asm__ volatile("" : : "r"(hold));
```

The output operand starts `hold`'s live range and the later input ends it, so
r3 is live *across* the load and `find_free_reg` cannot hand it to the
scratch; agbcc takes the next free register, which is the ROM's r1. Both
asms emit no bytes and the allocation shift is local to the load. Applying a
second pin (dummy in r2, load of rec+164, whose ROM scratch is r3) then
matched the rest of the body, 592/592, with no instruction splices. The class
moved `partial` -> `c_fakematch`.

The tick is that the pin must *bracket* the load. An output-only asm alone
leaves `hold` dead immediately, and a clobber cannot reserve the register for
a load that is emitted after it. Measure one scratch at a time: a pin at one
site can move the scratch at the next, so iterate down the listing.

One asm can hold several dummies (`"=r"(h0), "=r"(h1)`); `MenuFF78_15620`
needs both r0 and r1 live to move its zero-index scratch to r2.

The same bracket settles the *zero-index* form. `Course_Scan`'s ROM body
loads `ldrsh r5,[r0,r1]` then `ldrsh r0,[r0,r2]`, and `RaceVM_022624` loads
`[r4,r1]` then `[r4,r2]`; the pin on the first scratch (r1) reproduces the
second. `MenuFF78_154E0` loads the zero index into r1 in a body whose plain
form takes r0, so there the dummy sits in r0.

A body can need a *sequence* of pins. `MenuFF78_15620`'s clamp rereads the
cell twice, through r2 and then r3; pinning r2 around the second load alone
pushes the *first* load to r3, because agbcc sees the later need. Bracket the
first load with a dummy in r3 and the second with a dummy in r2, and both land
where the ROM has them.

**Consequence.** Four `partial` bodies (872 bytes) remain gated on this one
register choice: `_08006A50`, `_0800175BC`, `_0800278F0`, `_080018EC0`. The
liveness pin is the first thing to try on each; `_080020198`,
`_08006050`, `_080022624`, `_0800154E0` and `_080015620` were among them.

`_0800175BC` needs the pin *and* a second fix, and the second one is not a
register choice. Once its four-instruction `+340` store moves to C, the body
is byte-correct except that agbcc expands the far `if (... != 2) return;` at
+0x8C as `beq` over a `b` (`@long jump`, 4 bytes) where the ROM has one
`bne`. That is `shorten_branches`, not allocation: the branch is 246 bytes
from the epilogue, inside the +-254 Thumb range, but agbcc's length estimate
for the span is inflated by its own asm statements (`asm_insn_count` counts
2 bytes per logical line, emitted or not), so the estimate crosses 254
first. The old `.hword` splice hid this: eight emitted bytes counted as two,
a 6-byte *under*-estimate, which is why that body only matched with the
splice. Adding two `.set dummy, 1` statements inside case 2 flips a matching
body to the 4-byte form, so treat any far-conditional branch in a body with
asm barriers as an estimate problem before re-spelling the condition.

The same measurement was run against the comparison `agbcc` binary
(`build/toolchains/agbcc/agbcc`): it allocates identically, so this is not a
difference between the two builds of the compiler.

## A non-volatile lvalue loads into the register the variable already has

`src/agbmain_session.c` `SessionVerify_080008F4` carries the ROM's in-place
`ldrb r7, [r7, #1]` (0x080008F4 + 0x146) as a one-instruction splice, and the
source comment recorded it as a measured wall. It is not:

```c
record = (volatile u8 *)(uintptr_t)*(u8 *)(record + 1);
recordBit = (u32)(uintptr_t)record << 27;
```

The load's destination is the variable that holds its own base, and the
variable is dead immediately afterwards, so agbcc ties the result to the
base's register. The **non-volatile** lvalue is what makes the difference:
the front end then makes the access `(set REG (zero_extend (mem:QI ...)))`
with `REG` the variable itself. A `volatile` lvalue goes through
`load_volatile`, which materialises a per-access pseudo and copies it out
(`ldrb r0,[r7,#1]; adds r7,r0`), and an asm-pinned regvar base cannot fold
`+1` into the addressing mode at all. The body is now 364/364 byte-exact
with no instruction-bearing asm.

This is the "Non-volatile lvalues" lever from
[register-shaping levers](register_shaping_levers.md), applied to a byte
load with a self-referential base.

## `_0802DBA4` `SoundDBA4_Read`: two documented walls

Both of its splices are blocked, and the reasons are different.

* `add r2, sp, #8` (0x0802DBA4 + 0x70) materialises the read cursor from the
  frame base. Spelling it in C as `(u8 *)buf + 8` makes `buf`'s address a
  value the allocator keeps live across the two calls, so it becomes a global
  allocno and takes r7 -- the ROM rematerialises `sp` at each of its four
  uses and never spends a register on it. The body grows by 4 bytes and the
  prologue changes.
* `adds r0, r4, #1` (0x0802DBA4 + 0x96) is the row counter. `tr = row + 1u;`
  is the documented pinned-temp spelling, but `loop.c` hoists the add to the
  head of the do-while back edge and leaves only the narrowing in the tail
  (`adds r4,#1` at the head, `lsls/lsrs` in the tail). The do-while
  counter-hoist rule in
  [local allocation notes](local_alloc_register_choice.md) is the governing
  constraint; only an opaque statement keeps the add in place.

## A halfword store never gets the immediate-offset form

`src/menu_ff78_o.c` `MenuFF78_15304` stores three halfwords with the ROM's
immediate-offset forms (`movs r0,#1 / strh r0,[r7,#20]`). The C equivalent
`*(volatile u16 *)(rec + 20) = 1;` expands to

```
adds r1, r7, #0
adds r1, #20
movs r0, #1
strh r0, [r1, #0]
```

while the same store through a `u32` lvalue keeps the immediate form
(`str r0, [r7, #20]`). The address is copied into a scratch before the
offset is added, and the halfword pattern then has no offsettable address to
print. Casting the base to `u16 *` does not change it, and neither does
making the lvalue non-volatile. Four bytes per store, so the body grows by
twelve.

## What is left, by size

| body | bytes | blocker |
| --- | ---: | --- |
| `_080015304` `MenuFF78_15304` | 408 | halfword immediate-offset store |
| `_0800175BC` `Rec35_Input_175BC` | 400 | indexed halfword scratch, plus a far branch agbcc turns into a 4-byte long jump |
| `_0800278F0` `Garage_Ticker_278F0` | 232 | indexed halfword scratch, plus two naked bodies |
| `_08000BB0C` `Award_CopyTwinB` | 248 | `mov r3,sp` and an add whose operand order the pointer normalisation forbids |
| `_080053DC` `OAMGrid_Build` | 200 | the palette shift hoists out of the loop; 181/200 with the hoist suppressed |
| `_08002DBA4` `SoundDBA4_Read` | 176 | frame-base cursor and the hoisted counter |
| `_08006A50` `CollectNearby_06A50` | 156 | the argument copy wants r7 and the scratch wants r2 |
| `_080018EC0` `Race_Setup_18EC0` | 84 | the zero index wants r7, an unpinned global allocno |
| `_08002BE78` `SoundPump` | 60 | a literal-pool load and two flag-coupled branches |
| `_08002C11C` `SoundVoiceCleanup` | 68 | `tst` is not in the compiler's repertoire |
| `_08002C160` `VoiceEnvelopeApply_2C160` | 48 | register-indexed `ldrsb` |
| `_08001D7B8` | 160 | a half-defined `u64` return |
| `_080015A0` | 24 | `mov r0,pc` |
