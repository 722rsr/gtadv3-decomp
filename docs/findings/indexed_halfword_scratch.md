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

**Consequence.** Two `partial` bodies remain gated on this one
register choice: `_08006A50`, `_0800278F0`. The
liveness pin is the first thing to try on each; `_080020198`,
`_08006050`, `_080022624`, `_0800154E0`, `_080015620` and `_080018EC0` were
among them.

A scratch in a *high* register needs one dummy per free register below it, and
they can share a single bracket. `_080018EC0` (84 B) wants `movs r7,#0` before
its first `ldrsh`; agbcc takes r3, then r4, r5, r6 as each is pinned, so the
bracket names all four (`"=r"` on `scratch`/r3 plus three dummies in r4/r5/r6,
one input-only asm after the load). The holders may be the body's *own*
pinned regvars as long as their live ranges start after the load: r5/r6 there
belong to the `field`/`ctx_slot` pointers, which are only assigned below it.
Dropping any one of the four dummies returns the scratch to r3-r6, 82/84.

`_0800175BC` (400/400, 2026-10-24) closed **without** a liveness pin; the
scratch was never its own blocker. Two measured facts from the landing are
worth keeping:

* The case-2 `+340`/`+342` stores need the constant synthesis in r2
  (`movs r2,#170 / lsls r2,#1`), which a pinned `byteOffset __asm__("r2")`
  with one `"+r"` barrier delivers. The pin binds only through an asm
  operand -- the same spelling without a barrier lets cse fold the constant
  past the pin and re-synthesise it in a free register. Removing those two
  constant reloads then moved reload's round-robin `last_spill_reg` cursor,
  and the state-store copy (`mov r1,r8`), the second pool word (`ldr r1`)
  and the ldrsh zero-scratch all fell into place with no further spelling.
  When one register choice is off by a cursor position, fix the *first* wrong
  site and re-measure the rest before pinning each one.
* The far `if (... != 2) return;` at +0x8C is 246 bytes from the epilogue,
  and `shorten_branches` prices every asm statement at 2 bytes per logical
  line (`asm_insn_count`, emitted or not), so its span holds at most **4**
  one-line asm statements (246 + 8 = 254). The old `.hword` splices hid
  this: eight emitted bytes counted as two, a 6-byte *under*-estimate, which
  is why the body only matched with the splice. Adding two `.set dummy, 1`
  statements inside case 2 flips a matching body to the 4-byte `beq` over a
  `b` form, so treat any far-conditional branch in a body with asm barriers
  as an estimate problem before re-spelling the condition.

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

## `_0802DBA4` `SoundDBA4_Read`: fully decompiled

* `adds r0, r4, #1` (0x0802DBA4 + 0x96) is the row counter. `tr = row + 1u;`
  is the documented pinned-temp spelling, but `loop.c` hoists the add to the
  head of the do-while back edge and leaves only the narrowing in the tail
  (`adds r4,#1` at the head, `lsls/lsrs` in the tail). **An empty `"+r"`
  barrier placed *before* the update is what keeps it in place**:
  `__asm__ volatile ("" : "+r"(row)); row = (u8)(row + 1u);` reproduces
  `adds r0,r4,#1 / lsls r0,#24 / lsrs r4,r0,#24` with no splice. Placing the
  barrier after the update (the mirrored order) does not -- that form is what
  was measured as blocked. The same shape closed `SoundDC54_Write`'s two
  wrap-around counters.
* `add r2, sp, #8` (0x0802DBA4 + 0x70) materialises the read cursor from the
  frame base. Spelling it in C as `buf + 4` makes the address a global cse
  target: agbcc hoists it to the top of the frame setup and keeps it in a
  callee-saved r7 across the two DMA calls, so the prologue gains r7 and the
  body scores 54/176. Empty barriers, `&buf[4]`, `(u8 *)buf + 8` and a
  `volatile` local all reproduce the hoist. An **empty register-clobber** just
  before the assignment does not: `__asm__ volatile ("" : : : "r7");` discards
  cse's fact that r7 holds the frame base without emitting any bytes, so the
  `add` is computed fresh into the pinned `r2` and the body is 176/176 with no
  instruction splice. Prefer this over an `add r2,sp,#8` splice whenever an
  address is parked in a callee-saved across calls; it should also unblock
  hoist walls like `OAMGrid_Build`'s palette shift.

## A pinned r7 base can never fold a sub-word store (or load) offset

`src/menu_ff78_o.c` `MenuFF78_15304` stores three halfwords with the ROM's
immediate-offset forms (`movs r0,#1 / strh r0,[r7,#20]`). This section first
recorded the symptom as "a halfword store never gets the immediate-offset
form"; that diagnosis was wrong and led nowhere. The real mechanism, read out
of the compiler (`thumb.h:808` `GO_IF_LEGITIMATE_ADDRESS`): the first clause
rejects **every HImode/QImode address mentioning the frame pointer** (r7 is
`FRAME_POINTER_REGNUM`) until reload starts, and `movhi`/`movqi`'s expands
answer an illegitimate address with `copy_to_reg` -- which is the
`adds r1,r7,#0 / adds r1,#20` pair. SImode skips that clause, so the same
spelling through a `u32` lvalue folds to `str r0,[r7,#20]` and halfwords do
not. Casting the base to `u16 *`, or making the lvalue non-volatile, changes
nothing: it is the **pin** that is fatal. A pinned r7 regvar is hard-reg r7 at
expand time; an *unpinned* pseudo passes the check, survives to reload with
its offset folded, and reload keeps the fold when the pseudo's home is r7.
So a ROM body containing `strh r0,[r7,#off]` (off <= 62) is proof the base
was a pseudo that global-alloc gave r7 -- never a regvar spelling.

`MenuFF78_15304` (408/408, 2026-10-24) lands exactly that way, and with no
instruction-bearing asm:

* `rec` is a plain local. Global-alloc gives it r7 on its own (the
  [r7 recipe](register_shaping_levers.md#r7-wants-an-unpinned-global-allocno));
  every `rec + N` copy form (`adds rX,r7,#0 / adds rX,#N`) still appears, and
  `mark_home_live` restores the `push {r4,r5,r6,r7,lr}` prologue.
* The three late `rec_` arguments are spelled `rec`, not `rec_`: the ROM
  re-derives them from the r7 copy (`adds r1,r7,#0` before `bl`), and keeping
  the incoming argument pseudo alive stages it in a callee-saved (measured:
  r5, which in turn pushed the shared zero constant from r5 into r9 and cost
  a `mov r6,r8 / push {r6,r7}` high-save).
* The last store's source is the r8-pinned `sixhi`, and `strh` sources must
  be low registers, so reload inserts `mov r1,r8`. A liveness pin holding r0
  across the store (output-only empty asm before, input-only after) moves that
  copy to the ROM's r1; without it reload reuses r0.

## What is left, by size

The liveness pin closed `Race_Setup_18EC0` (84 B) as described above, and the
unpinned-global-allocno route closed `MenuFF78_15304` (408 B). What is left
of the `partial` class, by size (2026-10-24, from
`python3 tools/source_quality.py --list partial`):

| body | bytes | blocker |
| --- | ---: | --- |
| `_08000BB0C` `Award_CopyTwinB` | 248 | `mov r3,sp` and an add whose operand order the pointer normalisation forbids |
| `_0800278F0` `Garage_Ticker_278F0` | 232 | indexed halfword scratch, plus two naked bodies |
| `_080053DC` `OAMGrid_Build` | 200 | the palette shift hoists out of the loop; 181/200 with the hoist suppressed |
| `_08001D7B8` | 160 | a half-defined `u64` return |
| `_08006A50` `CollectNearby_06A50` | 156 | the argument copy wants r7 and the scratch wants r2 |
| `_08002C11C` `SoundVoiceCleanup` | 68 | `tst` is not in the compiler's repertoire |
| `_08002BE78` `SoundPump` | 60 | a literal-pool load and two flag-coupled branches |
| `_08002C160` `VoiceEnvelopeApply_2C160` | 48 | register-indexed `ldrsb` |
| `_080015A0` | 24 | `mov r0,pc` |
