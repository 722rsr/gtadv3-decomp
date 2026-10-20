# Register-shaping levers measured on the last non-exact entries

Worked while clearing the six non-EXACT manifest entries (2026-10-16). Four
became byte-exact with the recipes below -- `0x08000848`
(`PacketBuild_08000848`), `0x08008BB8` (`Course_Dispatch_08BB8`), `0x08001D974`
(`_08001D974`, 84 B) and `0x08020690` (`_080020690`) -- and two advanced:
`0x0800B89C` (`GoStart_Tick`) and `0x08009BF8`. Both now match their complete
244-byte and 864-byte bodies, respectively (2026-10-17).
Every claim here was checked with
`python3 tools/corpus_match_probe.py --function NAME --c89 --require-exact`
plus `python3 tools/match_c_slice.py NAME`, and each touched TU's siblings were
re-probed afterwards. The matching commits are `a46575a`, `3a972ed`, `dd36b43`,
`644e6c1` and `f362679`.

## r7 wants an unpinned global allocno

The compiler mechanics live in
[local_alloc_register_choice.md](local_alloc_register_choice.md#r7-is-reachable-through-global-alloc-not-through-pins).
The three bodies that needed it, and the shapes that worked:

| site | shape |
| --- | --- |
| `PacketBuild_08000848` mid-body byte/halfword temp | one function-scope `u32 temp` with one def per site, each def kept live to its use with `__asm__ volatile ("" : "+r"(temp));` so combine cannot fold the load into the `and`; the loads read through non-volatile lvalues (below) |
| `Course_Dispatch_08BB8` r7 scratch | `word7` pinned r7 for the ROM's `adds r1,r7,#0` copies, plus a second, block-spanning `u32 temp7` whose def-use pair is anchored by barriers on both sides: the allocno reaches global-alloc, takes r7, and `mark_home_live` restores `push {r4,r5,r6,r7,lr}` and the `mov r7,r8 / push {r7}` high-save |
| `_080020690` zero word | unpinned `int zero`, extended past the tail calls with one `"+r"` barrier; the allocno takes r7 (the value is `0`, so reload may also rematerialise it) and the push/save follow |

Two ordering facts from the same work:

* `allocno_compare` prioritises `floor_log2(refs) * refs / live_length * size`
  (ties by allocno number), so a temp with more refs over a shorter range is
  allocated first; `_080020690` gets `table` into r6 and `zero` into r7 on
  exactly that comparison.
* after a register is assigned, `find_reg` copies it into every conflicting
  allocno's set, so an allocno that loses the race also loses the register.

A pin is not a substitute: `register T x __asm__("r7")` emits raw r7 uses that
`flow.c` early-returns on, so `regs_ever_live[7]` stays clear and the prologue
never saves r7.

## Non-volatile lvalues: the direct form loads into the register you pinned

`v = *(volatile u16 *)p;` goes through `load_volatile`: the access materialises
a per-access `HI`/`QI` pseudo (a local, so never r7) and the value is copied
out afterwards. `v = *(u16 *)p;` instead lets the front end emit
`(set REG (zero_extend:SI (mem:HI ...)))` -- the `ldrh REG,[p]` /
`ldrb REG,[p]` pattern -- into whatever the destination is: a pin, or another
allocno. `PacketBuild_08000848` needs this at all three byte/halfword sites;
with the volatile spelling the byte temps are local QI pseudos that land in r8
and the body is 12 bytes long.

Caveat: once the access is non-volatile, combine may fold the load into a
following ALU op as `(subreg:SI (mem:QI) 0)`. The register that then feeds the
op is a **reload scratch**, chosen per instruction -- which is the desired
answer when the ROM's register is r7 and r0-r6 are all live at that point
(`PacketBuild_08000848`'s block 2 keeps `ldrb r7,[r5,#1] / ands r1,r7` that
way), but a temp that is folded at *every* site never becomes an allocno, so it
never triggers `mark_home_live` and never pushes r7. `Course_Dispatch_08BB8`
sits exactly between the two cases: block 2 gets r7 as a scratch, block 3 stays
on r5 until `temp7` is a real global allocno.

## Blocking combine's copy-rewire needs a *second* use

`(set T (asm [in]))` followed by `(set Y T)` is rewired by combine into
`(set Y (asm [in]))` -- the asm's output moves to `Y` and `T` disappears --
whenever `T` dies at the copy. A single `"+r"(T)` barrier between the def and
the copy is therefore not enough: the barrier's own output is what gets
rewired (the dump shows the barrier's `=r` destination rewritten to r1 and the
constant folded into its input). Keeping `T` live *past* the copy -- a second
`"+r"(T)` barrier after it -- stops the rewrite, and the copy renders as the
ROM's `adds r1,r7,#0`. `Course_Dispatch_08BB8` spells both `temp7` sites that
way.

## extendhisi2's zero scratch is where `movs rX,#0` rotations come from

`thumb.md`'s `*extendhisi2_insn` expands a bare-register address as

```
mov  scratch, #0
ldrsh DST, [BASE, scratch]
```

because the pattern sets `ops[2] = const0_rtx` and `const0_rtx` is not a REG,
so it takes the `mov`/`ldrsh` branch even for offset zero. `scratch` is the
pattern's `"&l"` match_scratch, allocated like any short-lived pseudo, so the
`movs rX,#0` rotation registers in a ROM body are those scratches, not source
variables. Steering them means steering the surrounding pressure: the
`_0800B89C` stage loads had to be pinned to the ROM's r1/r2/r0/r4 rotation
before the scratch/value registers lined up.

## Address association: write the operand the ROM's destination register implies

* `idx * 72 + table` emits `adds r3,r3,r6` (destination = the index term),
  while `table + idx * 72` emits the mirrored `adds r3,r6,r3`.
  `_080020690` and `_0800B89C` both need the index-term-first spelling.
* constant terms are normalised to the end, so `wa + off + 2*idx` never
  materialises `wa + (off + 2)`; keep `off` a runtime value (a variable) and
  update it in place (`off += 2u`) when the ROM bumps the offset register
  (`adds r2,#2`).
* a pointer plus an assembler symbol stays a real add, while two integer
  literals fold into one pool word -- the established `RAM_SYM_...` recipe in
  `local_alloc_register_choice.md` ("Keeping an address *live*").

## Constant synthesis: do the arithmetic in steps on a pinned register

`_080020690`'s else arm materialises 78 as
`movs r2,#2 / lsls r2,r2,#4 / adds r2,#46` instead of `movs r2,#78`. A C
expression like `(2 << 4) + 46` folds (front end), and a pinned variable
assigned that expression also folds (combine rewires the hard-register chain).
Three statements with empty `"+r"` barriers between them keep every step:

```c
register u32 idParts __asm__("r2");
idParts = 2u;
__asm__ volatile ("" : "+r" (idParts));
idParts <<= 4;
__asm__ volatile ("" : "+r" (idParts));
idParts += 46u;
```

The barriers occupy no bytes from the following statement onward; the fold is
what the C has to prevent.

## Pinned-variable initialization is load-bearing

`register u32 z __asm__("r0");` with no initializer lets agbcc reuse whatever
was already in r0: `_08001D974` stored r0's leftover address value where the
ROM stores a fresh zero. Assigning `zero = 0;` immediately before each store
restores the ROM's two `movs r0,#0` materialisations. The pinned *assignment*
is honoured (it is a copy); the pin alone is not a value.

## Declaration order sets the parameter-copy order

The prologue copies pinned pointer variables in declaration order:
`_08001D974` declares `pb` (`r4`) before `pa` (`r5`) to get the ROM's
`adds r4,r1,#0 / adds r5,r0,#0`, where the argument order (`a_`, `b_`) emits
the r5 copy first. Related: a parameter used directly can beat a local copy
(the `SoundStreamAlloc` note in `local_alloc_register_choice.md`).

## `bx lr` stubs still take the record in r0

`_080020878` is a `bx lr` stub, but all three ROM call sites set
`adds r0,rN,#0` first. Declaring the parameter (`void _080020878(void *a);`)
and calling `_080020878((void *)rec);` reproduces the `adds r0,r5,#0` before
the `bl`; calling it as `void` compiles to a bare `bl` and comes out 2 bytes
short. `_080020690` needed exactly this.

## Resolved: `_0800B89C` emitted order

`_0800B89C` now matches all 244 bytes and its isolated source-only slice passes.
For stage 1, materialize `scaledIndex` before incrementing the pinned `selOff`,
then add `scaledIndex + (wa + selOff)` with the base in `r1`; this places both
the offset bump and base materialization after the `ldrsh`/`lsls` group. For
stage 2, a separate stage check and an `r2` clobber keep the timer literal load
after the branch, while keeping its offset live through the volatile read gets
the ROM's `ldr r2` / `adds r0,r5,r2` pair. Calls use the closure spelling
`sub_0800B82C`, which lets the independent slice resolve the grant-delay calls.
The emitted C uses empty barriers and register pins only.

## Resolved: `_08009BF8` packet fields

The record stores, switch cases, copy paths, and zero-filled tail padding now
match as a complete 864-byte body, and the independent 188,760-byte slice also
matches. In the case-0 packet block, an `r1` packet pointer, `r2` and `r3`
field-offset values kept live through their reads, and a materialized header
word make GCC 2.95 choose the ROM's register order. Reassigning the packet
pointer after `_08002340` emits the ROM's second `mov r1,sp`. These are empty
compiler barriers and register pins only; there is no instruction-bearing asm
in the lifted body.

## C forms for signed indexed halfword loads

`_0800A150`'s first work-area phase read no longer needs its three-instruction
splice. Keeping the base and byte offset in pinned `r0`/`r2`, adding them in C,
zeroing the offset, then reading through a byte pointer as
`*(const s16 *)((const u8 *)base + offset)` makes old_agbcc emit the ROM's
`movs r2,#0 / ldrsh r0,[r0,r2]`. A direct volatile halfword dereference instead
emits `ldrh` plus sign extension. The 84-byte body remains exact and its
isolated source-only slice passes. The phase-7 read is C too: after adding the
`0x1078` offset, keep that `r1` value live through the signed load with an
empty barrier on `offset` and the `r0` result. This prevents agbcc from
reusing `r1` for the zero-index register and preserves the loaded value in
`r0` for the following void-dispatch call. The full function now has no
instruction-bearing asm.

The case-1 read in `_0800A074` is also C now: keeping the base and `0x0FBC`
offset pinned in `r0`/`r1`, adding them in C, assigning result 19, and reading
the volatile halfword reproduces its 168-byte body exactly. The phase-7 read
is C too: keep `selectorValue` pinned to `r0` and the return value live in
`r2` across the signed load; this makes agbcc select the ROM's zero-index
register `r3`. Calling the dispatcher through its closure name
`sub_0800B190` lets the focused slice resolve it. The 168-byte function has no
instruction-bearing asm and passes the isolated slice.

## C expressions for packed resource-row tests

`_080262A4` now expresses both signed byte-index calculations, both `7 & i`
masks, and both `one = 1` assignments in C. Keeping the `i + 7` adjustment
inside the negative case preserves the arithmetic shift sequence. The first
`CpuFastSet` copy is also a C call with `r0`/`r1`/`r2` arguments held through
empty barriers; saving/restoring the VRAM cursor and advancing the destination
in C reproduces the original instructions. The second copy block remains
instruction asm because converting both calls makes agbcc rematerialize the
one-bit mask and grows the body by four bytes. Calling the decompressor through
`sub_0802D988` keeps the same `0x0802D988` target while allowing the independent
source-only slice to resolve its closure symbol. The full 288-byte body is
exact and `_080262A4` passes the independent 188,760-byte slice link.

## In-place halfword load in `Course_Dispatch_08BB8`

The branch-local `ldrh r2,[r2,#58]` is C now:
`rec = (volatile u8 *)(uintptr_t)*(volatile u16 *)(rec + 58)`. An empty
`"+r"(rec)` barrier with a memory clobber keeps agbcc's result in the pinned
`r2`; without it, the compiler chooses `r7`. The 232-byte body and its
independent slice remain exact. The two signed register-indexed `ldrsh` loads
still use asm: spelling one as `value = *(volatile s16 *)(rec + reg1)` makes
agbcc add the base and index, then use `ldrh` plus sign extension, growing the
body by four bytes.

## Closure names and same-register loads in `Course_ResourceSetup`

The body calls `sub_08007498`, `sub_0800748C`, `sub_080056C4`, `sub_0802D984`,
and `sub_0802D988` for the original assembly closure labels. It uses
`_08007658` and `_080074A8` for helpers that are also replaced by C in the
all-function slice; those spellings remain exported by their C owners. These
same-address names let the 268-byte function pass both its focused slice and
the all-function link. Its two `ldrh r7,[r7,#2]` splices remain asm: a C
assignment expands one load into `add r0,r7,#2; ldrh r0,[r0]; add r7,r0`,
adding four bytes because agbcc does not tie the memory result to its base
register.

## C selector and byte load in `_080028C54`

The masked selector stays in pinned `r1` through an empty barrier; a signed
temporary pinned to `r0` and another empty barrier preserve the C copy before
the arithmetic shift. This emits the ROM's `add r0,r1,#0 / asr r1,r0,#4`.
The table byte is C too: `b = *tableAddr` followed by a `"+r"(b)` memory
barrier keeps the result in `r1` for the following arithmetic. Replacing the
palette copy call with its `sub_0802D974` closure spelling makes the 328-byte
body's focused source-only slice resolve. An input-only empty barrier holds
`firstSource` in `r0` before `lane <<= 12`, which emits the ROM's `lsl r5,r5,#12`.
`_080028C54` now has no instruction-bearing asm, matches all 328 bytes, and
passes the independent slice link.

## C signed reads and clamp in `MenuFF78_12990`

Reading `cell` through a non-volatile `s16` lvalue into pinned `r0` emits the
ROM's zero-index `ldrsh` before the `0x08024DA0` call. The tail is C as well:
`clamp_cell` pinned to `r1`, signed halfword reads, `< 0` and `> 9` conditions,
and volatile stores reproduce both clamp branches and the final `r6`-based
read before `sub_08004BFC`. The absolute WA symbol remains an assembler
directive, but no instruction-bearing asm remains in the function. Its 180-byte
body is exact and passes the independent slice.

## C signed reads and cursor update in `MenuRec_0800CED4`

The four signed cursor reads are C `s16` lvalues with the offsets pinned to
the ROM's index registers. Using `volatile s16` makes agbcc form an address,
load a halfword, then sign-extend; the non-volatile lvalue emits the required
register-indexed `ldrsh` directly. For the cursor update, a halfword load into
pinned `r1`, followed by an empty `"+r"` barrier and the C addition into `r0`,
emits `ldrh r1,[r5,#6] / add r0,r1,r7`. The barrier carries no instruction.
The calls use the exported closure names `sub_08004B68` and `_080026938` so the
isolated source-only link resolves. `_0800CED4` is now C with only register
pins and an empty barrier; all 140 bytes match and its focused slice is
byte-identical.

## C result scatter in `MenuF9A0_Scatter`

All five work-area destinations use C pointer arithmetic. Pinning the WA base
to `r4` preserves the ROM's `add destination,r4,r2` operand order, including
the two destinations derived by incrementing and decrementing the offset. The
two coordinate reads are C signed-halfword lvalues with their zero indices
pinned to `r1` and `r2`, which emits the ROM's register-offset `ldrsh` forms.
Calls through the exported `sub_0800F700` and `sub_0800F7C0` closure names keep
the focused source-only link resolvable. The 132-byte body now uses only C and
register pins, matches exactly, and passes its independent slice.

## C halfword read and grid stride in `_080025930`

The 264-byte loop advance is C: a stride pinned to `r0` starts at 132, shifts
left once, then advances the `r8` byte address and `r5`. The signed halfword
read is also C. A pointer pinned to `r2` and dereferenced into the `r0` result
emits `mov r2,r8 / movs r3,#0 / ldrsh r0,[r2,r3]`; an empty input barrier after
the read keeps the address live so it cannot overlap the result. Keeping the
grid output pointer `r4` live through the final fallback with another empty
barrier preserves that block's `r3` load scratch. These barriers carry no
instructions. The full 336-byte function is exact, has no instruction-bearing
asm, and passes its independent source-only slice.

## C word-pointer copies in `RuntimeMemcpy`

The 16-byte loop is four `*word_dst++ = *s++` expressions, with the 4-byte
remainder loop using one. Declaring both cursors as word pointers makes agbcc
emit the ROM's single-register `ldmia`/`stmia` post-increment pairs; converting
the advanced source cursor back to `const u8 *` keeps the byte tail unchanged.
`RuntimeMemcpy` is now C with only register pins and an empty barrier, matches
all 96 bytes, and passes its independent slice. Its `RuntimeMemset` sibling
also remains exact at 84 bytes.

## C copy for the sprite mask in `_080026898`

An empty `"+r"` barrier on the `r8` mask value before assigning it to pinned
`r2` preserves the ROM's `mov r2,r8` as compiler-generated C. Without the
barrier agbcc reuses the constant already in `r2` and removes that copy. The
160-byte body and independent slice remain exact; the two stack-adjustment
splices still make the function `partial`.

## C same-register halfword load in `SaveGarageUnpack`

The trailing `ldrh r2,[r2,#46]` is C now: a non-volatile halfword load through
the byte-offset pointer plus a `"+r"(blk)` barrier keeps the result in pinned
`r2`. The six mask-test calls use `_0802476C`, the exported same-address name
that lets the target-only slice resolve the C helper. The opening signed
`ldrsh r4,[r0,r3]` is C too. Keep the `0x574` work-area offset live in pinned
`r1` and reserve the later `block2` register `r2` across the read with empty
barriers; agbcc then uses the pinned zero index in `r3` instead of reusing
`r1` or `r2`. The complete 192-byte body has no instruction-bearing asm and
passes its independent slice.

## C literal loads in `_080023BD4`

The table pointer and the `0x03001780`/`0x10C3` address components now come
from pinned C variables initialized from absolute assembler symbols defined by
`.equ` directives. Keep the assignments after their preceding helper calls:
declaring initialized register variables at function entry lets agbcc move
loads before calls or fold the address sum. The packet-send call uses
`_080022D44`, the spelling present in the assembly closure; its promoted C
owner now exports that spelling in the matching-slice manifest as well. The
128-byte body has no instruction-bearing asm, matches exactly, and passes both
the focused and aggregate source-only slice links.

## A switch's default can share a case body

`0x080022920` (`RaceVM_022920`, 244 B) dispatches on a work-area value with

```
cmp r1, #1
beq case1
cmp r1, #2
beq case2
case1:            <- the fall-through for every other value
```

so an unmatched value runs case 1's body. That is a `switch` whose `default`
label sits on the same body as `case 1`, and the source order is `default`
first:

```c
switch (value) {
default:
case 1:
    *(volatile u16 *)(uintptr_t)(rec + 132) = 0;
    break;
case 2:
    *(volatile u16 *)(uintptr_t)(rec + 132) = 1;
    break;
}
```

Written the other way round (`case 1:` then `default:`) the body is the same,
but writing `default: break;` as a separate empty arm puts both bodies out of
line and adds a branch to the end -- 241 of 244 bytes. The empty `default`
also changes the source-order placement, so it is not interchangeable with an
omitted `default` (an omitted one measures the same 241).

## The switch's controlling value is copied explicitly

The same body copies the value into r1 before the dispatch
(`mov r1, r9 / cmp r1, #1`) because the case bodies want r0. Left to itself
agbcc puts the copy in r0 -- the lowest free register -- and the comparison
then reads the wrong register. Making the copy explicit and pinned fixes it:

```c
register s32 r1v __asm__("r1");
r1v = r9v;
switch (r1v) { ... }
```

An unpinned copy does not work (the same 241): the register has to be pinned,
because the pinned variable's own home is the register the comparison reads.

## A stack argument can be an assignment

`0x080022920` passes a value that is also kept in another register, and the
ROM materialises it once:

```
movs r0, #4
str  r0, [sp, #0]
movs r1, #1
mov  sl, r1
str  r1, [sp, #4]
```

The copy into sl and the stack store share one materialisation, so the
argument is the assignment itself:

```c
rslv = 1;
_08007770(0, (void *)(uintptr_t)r8v, 0, 0, 4, (rslv = 1));
```

Passing the variable instead stores sl, and passing the literal materialises
the 1 twice (once for sl, once for the argument).

## A pointer offset is two instructions only while the address stays a pointer

`0x08002A764` (196 B, measured but not yet landed) is the clearest case:
every `rec + N` in the ROM is `adds rX, r7, #0 / adds rX, #N`, and that is
what a *pointer* expression gives. The same expression with a `(uintptr_t)`
cast in the middle becomes an integer sum and the compiler folds it to
`movs rX, #N / adds rX, rX, r7` -- one instruction fewer, and the body stops
matching. The same applies to the signed reads: `*(s16 *)ptr` (a plain
lvalue) is the form that gives the ROM's register-indexed
`movs rX, #0 / ldrsh rY, [base, rX]`, and a `(uintptr_t)` cast turns it into
a copy of the base plus the same index form.
