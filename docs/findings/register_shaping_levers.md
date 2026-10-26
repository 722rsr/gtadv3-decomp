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

## Leave the record pointer unpinned; a local must never land in its register

`_080017788` (`Rec35_Input_17788`, 452 B) reached EXACT in C only with
`rec` as a plain local, not `register u8 *rec __asm__("r7")`. The pin looks
right -- the ROM's `adds r7, r0` is exactly a regvar copy -- but it makes r7
unavailable for the callee-save staging, so agbcc emits
`mov r6,r9 / mov r5,r8 / push {r5,r6}` instead of the ROM's
`mov r7,r9 / mov r6,r8 / push {r6,r7}`. Unpinned, the allocator gives the
pointer r7 on its own and the staging matches.

The pin also decides the *address form* of every sub-word store. agbcc
materialises the address of a pinned base -- `adds r1,r7,#0`, `adds r1,#20`,
`movs r0,#1`, `strh r0,[r1]` -- and folds the offset only for word stores
(`str r0,[r7,#88]`). This is `GO_IF_LEGITIMATE_ADDRESS` rejecting HImode
FP-based addresses before reload (see
[the sub-word fold rule](indexed_halfword_scratch.md#a-pinned-r7-base-can-never-fold-a-sub-word-store-or-load-offset)),
so the splice cannot go while the record pointer stays pinned. The way out is
not to keep the pin and reshaped stores: it is to unpin and steer.
`MenuFF78_15304` (0x08015304, 408/408, 2026-10-24) lands there: `rec` plain
(global-alloc takes r7 on its own), the late call arguments spelled `rec`
instead of `rec_` (keeping the argument pseudo alive stages it in a
callee-saved and cascades into the prologue), and a liveness pin holding r0
across the last store so reload's low-reg copy of the r8 value takes r1.
A pinned *struct* pointer does not help either: the field store folds through
the argument pseudo and uses the incoming register, not the pin.

The same body also shows a wrong-code hazard worth recognising: with
`int t = cond ? 1 : 0;` (a rematerialisable local), agbcc put `t` in **r7**
while `rec` was pinned there, and reused r7 for `rec + 186` afterwards, so the
emitted body was not the C. Two safe spellings: keep the flag's two uses in
per-arm duplicated calls (what the ROM does -- its shared second call is the
compiler's own cross-jump), or make the local's register explicit. Pins on
`r7` and rematerialisable locals do not mix; check the disassembly of any body
that pins r7 before trusting its score.

## Address constants: pinned steps plus a pinned pointer

The ROM materialises `rec + 340/344/512` as `movs rX,#170 / lsls rX,rX,#1` and
`movs rX,#128 / lsls rX,rX,#2`. Three statements on a pinned register with
empty `"+r"` barriers between them (the constant-synthesis recipe above) give
the ROM's scratch registers r2/r3/r1, but the *load* then deploys the address
in that same register (`adds r2,r7,r2`). Pinning a second variable to r0 for
the pointer does not help on its own: copy-coalescing merges it with the
offset. The working shape keeps the offset register alive **through** the
pointer's live range:

```c
register u32 k __asm__("r2");
k = 170u; __asm__ volatile("" : "+r"(k));
k <<= 1;  __asm__ volatile("" : "+r"(k));
register u8 *p __asm__("r0");
p = rec + k;
__asm__ volatile("" : "+r"(p));
sub_08002158(3, RD16P(p));
__asm__ volatile("" : "+r"(k));
```

Both barrier pairs are load-bearing: without the first the constant folds,
without the ones after `p = rec + k` and after the call the two variables
coalesce and the ROM's `adds r0,r7,r2` / `ldrh r1,[r0]` pair becomes
`adds r2,r7,r2` / `ldrh r1,[r2]`.

## `Garage_InitRecord` (0x080026FF4, 228 B): three shapes worth reusing

Measured 2026-10-21 (`--function _080026FF4 --c89 --require-exact` plus
`match_c_slice.py`).

* **Address constants re-dereferenced per use.** The ROM holds `0x03001678`
  itself in a callee-saved register and reloads `[r4]` at each use. A pointer
  *local* (`u32 *slot = (u32*)0x03001678u; *slot = …`) pins the constant for
  the whole body and every later `*slot` becomes a register deref, shifting the
  body. Spelling each access through a macro
  (`#define GARAGE_CUR (*(volatile u32 *)0x03001678u)`) lets the allocator keep
  the *address* and re-deref, which is what the ROM does.
* **A constant store operand costs one extra copy.** `*(volatile u16 *)b =
  0xFFC0;` emits `ldr r1,<pool>` + `adds r0,r1,#0` + `strh r0,[r4]`
  (measured in a two-function scratch TU as well, so it is the store pattern,
  not the surrounding pressure). A `u16 w = 0xFFC0;` local stored through
  `strh r0,[r4]` is the ROM's shape; `w` must be a real variable, and the
  `else` branch should store the flag variable itself (`strh r7,[r4]`).
* **A trailing call's `r3` can be leftover, not an argument.** The ROM ends
  `ldrsh r0,[r1,r2] / movs r3,#12 / ldrsh r2,[r1,r3] / mov r1,r8 / bl`. The
  `12` in `r3` is the halfword index, *not* a fourth argument: declaring the
  callee 3-arg (`extern void f3(u32,u32,u32) __asm__("target")`) is byte-exact,
  while passing a literal `12` makes agbcc rematerialise `movs r3,#12` right
  before the call (+4 B, `OVERSIZED`). Suspect a phantom trailing register
  whenever the last argument equals a constant already used as a load index.

## Leaves that save r4/r5 but not lr: not reachable from this toolchain

`SoundVoiceFollow_NoteOff` (`0x0802C390`, 64 B) is a leaf whose ROM prologue
is `push {r4,r5}` and whose epilogue is `pop {r4,r5}` / `bx lr`: it holds the
`131`/`64` masks in r4/r5 across a linked-list walk and never touches lr.
The C body is small and otherwise reachable (the abort-on-cursor and
two-`tst` gates are a `if (id >= 128)` plus `if (flags & mask)` /
`if (!(flags & busy))` shape), so the frame is the only blocker.

Measured 2026-10-21 with a scratch TU compiled by both shipped compilers
(`build/toolchains/agbcc/{old_agbcc,agbcc}`, `-O2 -ffunction-sections`):

| options | prologue | epilogue |
| --- | --- | --- |
| `-mthumb-interwork` | `push {r4,r5,lr}` | `pop {r4,r5}` / `pop {r0}` / `bx r0` (+4 B) |
| `-mno-thumb-interwork` | `push {r4,r5,lr}` | `pop {r4,r5,pc}` (no `bx lr`) |

A minimal leaf (`u32 a = 131, b = 64;` + the same loop, no calls) reproduces
the lr save, so this is not pressure from the surrounding body. Neither
option set reaches `push {r4,r5}` + `bx lr` (the mechanism was not traced
into the compiler source; the four emitted shapes above are the measurement),
so a body of this class cannot leave its naked-asm owner even when every
instruction in it is understood. Do not re-spend source permutations or
register pins on it;
`SoundVoiceFollow_NoteOff` keeps its asm body and the doc comment above it.

The same limit applies to `Garage_CarPos_27120` (`0x08027120`, 240 B), whose
ROM frame is `push {r4,r5,r6,r7,lr}` + `sub sp,#12` (so lr *is* saved there)
but whose loop head and count reads stayed 16 bytes short of the ROM at
224/240 in the best draft measured: the ROM duplicates the row-count test
(`ldrb`+`lsls`/`asrs`, at both the entry and the back edge) and keeps the
slot *address* in r7 while copying it to r6 for the loop, which agbcc folds
into one register and one rotated test. It remains asm.

## `Race_Setup_19250` (0x08019250, 200 B): the ARMCC r8 slot, measured

Lifted 2026-10-21 as far as plain C goes; the best draft reached **93/200
bytes (candidate 192 B, first diff +0x6)** and was reverted. Measured probe
sequence, in order:

* the two work-area gate reads must be spelled through `ram_blocks.h`
  (`RAM_DEF_03001780;` in the body, `wa = RAM_SYM_03001780; off = 0x1111u;`
  then `*(s8 *)(wa + off)`): with integer literals the address folded to one
  pool word and the body scored 10/200; with the symbol it jumped to 93/200;
* reading the first gate into an `s8` local is what produces the ROM's
  `ldrb` + `lsls`/`asrs` pair; comparing `*(u8 *)gate != 0` folds the
  extension away;
* unlike `code_23a34.c`, this body must **share** one `wa` between both gate
  reads. Giving the second gate its own fresh dead locals (`w2`/`o2`) drops
  the score from 93/200 to 18/200 (188 B) because the shared `wa` is what
  occupies r8 across the calls and therefore produces the ROM's
  `mov r7,r8 / push {r7}` save staging at all.

The residual is the frame. At 93/200 the staging exists but holds the wrong
value: the ROM keeps `p` in r7, `q` in r8 and `d` in r6, whereas the draft
keeps the work-area base constant in r8 (`mov r8,r0` + `add r0,r8`) and
`p`/`q` in r5/r7. Measured alternates, all reverted: pinning `p` to r7 gives
19/200 -- the frame shape returns but every `p + N` sub-word address is
re-materialised (`adds r0,#22 / movs r1,#0 / ldrsh r0,[r0,r1]`); removing
the `r4`/`r5` pins on `dx`/`dy` gives 19/200 with `push {r4,r5,r6,lr}` and
no r7/r8 use at all. `d` pinned r6 is required in every variant. No spelling
tried makes agbcc rank `q` above the work-area base. Re-tried 2026-10-21 with
the explicit pins that worked on `MenuFF78_115E0` -- `register u8 *q
__asm__("r8")` on its own reaches **42/200** (length exact at 200, frame
`push {r4,r6,lr}`), and adding `register u8 *p __asm__("r7")` reaches
**96/200**, still 200 bytes. The frame stays a three-register push because a
pinned r7 is never saved (see "A pinned r7 is never saved") -- so the pins
move the allocation but cannot reproduce the ROM's
`push {r4,r5,r6,r7,lr}`. Treat
96/200 as the ceiling for this body without a compiler-level allocation lever.

**Resolved 2026-10-26: EXACT 200/200, no instruction splice.** The lever is not
a pin on `p`/`q` at all -- it is leaving `a` *unpinned* so that it becomes a
global allocno. Pin `d`->r6, `dx`->r4, `dy`->r5 and hold r4/r5 live from entry
with an empty `+r` barrier; the allocator then has no low register for `a` and
gives it r7, which also makes r7 the r8 save slot (`mov r7,r8` / `push {r7}`)
and restores `pop {r4,r5,r6,r7}`. Because that r7 is an ordinary pseudo rather
than a pinned `frame_pointer_rtx` reference, the field reads come out in the
ROM's indexed form (`movs r0,#22 / ldrsh r1,[r7,r0]`) from plain C -- the
`adds`-form re-materialisation that capped the pinned drafts never appears. The
second operand needs one more lever: hold the r8->low copy live with a barrier
(`u8 *q = pb; __asm__("" : "+r"(q));`) so the offset is not folded into it, and
pin the homes (`hi`->r1, `q`->r2, `lo`->r0).

## `MenuFF78_115E0` (0x080115E0, 252 B): three facts from a fresh lift

Re-derived the cursor handler from its asm on 2026-10-21 and measured it, then
reverted (`menu_ff78_c.c` keeps its asm body). The figure the body's comment
carried for this function -- 246/252 with only the three zero-index `ldrsh`
registers missing -- is **not reproduced by the draft kept on disk**, which
scores **47/252**; the ROM prologue saves r8 (`4647` `mov r7,r8` / `b480`
`push {r7}` / `4680` `mov r8,r0`) while that draft emits
`push {r4,r5,r6,r7,lr}` and no r8 access at all.

The earlier conclusion that this r8 slot is a dead live range the C cannot
express is **wrong**. Pinning the value -- `register u16 prev __asm__("r8")`
with `prev` assigned after the cursor load -- reproduces the ROM staging, and
the body then reaches **252/252 bytes of length** with the same instructions
in nearly the same order (199/252 equal bytes, `PARTIAL`; the one ordering
difference is where the +198 store address is materialised). The size parity
alone disproves the "frame is unreachable" reading; what is left is
temp-register numbering.What made the difference, with only the measurements actually taken:

* `prev` pinned to r8 (above) gives the frame and, with the rest of the draft
  below in place, **106/252** at 260 bytes;
* `s16 *selp` declared **without an initializer** and assigned right before
  the 99 test: the address is no longer hoisted into the prologue, so r7 is
  staged at +0x24 as the ROM does -- **201/252**, first diff +0x2A;
* one `dst` pointer hoisted out of the final if/else removes a duplicated
  `adds r1,#212` -- 256 -> **252** bytes (all the length in the ROM);
* also required, without a score to attribute to each: `_0800F744`
  redeclared `u32` in this TU (asm bodies call it by symbol) so the F700
  argument is a bare `adds r1,r0,#0`; `_0800F6D0` declared `s32` so its
  return is not re-extended before the `cmp`; and
  `prev = *(volatile u16 *)curp` after the F700 store, because the reload
  must not be forwarded into `mov r8,r0` (the ROM reloads through r3).

**Correction (measured 2026-10-25).** The ROM `ldrsh` reads into a call-crossing
register *do* come from plain C; the discriminator is the local's declared
width, not the call. `python3 tools/expr_probe.py --snippet` on
`int q = *(s16 *)(p + 0); H(0); return q;` emits
`movs r1,#0 / ldrsh r4,[r0,r1] / movs r0,#0 / bl H / adds r0,r4,#0`, while
declaring the same local `s16` emits `ldrh r4,[r0,#0]` and the
`lsls #16`/`asrs #16` pair. An earlier reading of this body concluded the
`ldrsh` was unreachable and spliced it; it is not, and no splice is needed.
A natural draft with `int` locals reaches **117/252** at 244 bytes. What is
left is the register each zero-index `ldrsh`'s constant lands in: the ROM
uses a rotating r2/r0/r1/r3 and agbcc always prefers the next free low
register. Pinning that constant does not help (`register u32 z __asm__("r2") = 0`
still allocates r0/r1 -- cse keeps the value, not the home), and a `"+r"`
barrier on the zero adds an `adds rX, base, rX` address computation the ROM
does not have (`ldrsh` then reads `[rX, r0]`). That numbering, not instruction
selection, is what still separates this body from `EXACT`.

Three further facts are worth keeping, because a wrong guess on any
of them costs the whole body:

* `_0800F744` is called with **r0 = 0**, not with the selected index: the ROM
  loads `s16[rec+188]` into r4, zeroes r0 and calls. Spelling the call as
  `_0800F744(sel)` shifts every following register;
* the `_0800F700` second argument is the `_0800F744` return copied with a bare
  `adds r1,r0,#0` -- no `lsls`/`asrs` pair. A 32-bit return type is therefore
  the shape that matches; the file-wide `extern s16 _0800F744(s32)` is only
  used by asm bodies in this TU, so redeclaring it costs nothing elsewhere.
  (The measured 47/252 draft still carried an `lsls`/`asrs` pair, but on the
  *selector* read: `s16 s = *selp;` compiled to `ldrh` + shifts instead of the
  ROM's zero-index `ldrsh`, so that pair is a separate defect, not the call);
* `prev = u16[rec+190]` is in **r8** in the ROM (`mov r8,r0`, then
  `subs r0,#1` re-uses the load). A plain `register u16 prev;` leaves it in
  r4 (agbcc's choice is correct at four live values), but the explicit pin
  `__asm__("r8")` does move it and produces the ROM's `mov r7,r8` /
  `push {r7}` staging -- so this body is not evidence for an unreachable
  frame, only for one that needs an explicit pin.

## A pinned r7 is never saved

Measured 2026-10-21 with a scratch TU, one function per register: each holds
the argument pointer across a call and dereferences it after the call.

```c
typedef unsigned char u8;
int H(int);
int p4(u8 *a) { register u8 *p __asm__("r4") = a; H(0); return p[22]; }
int p5(u8 *a) { register u8 *p __asm__("r5") = a; H(0); return p[22]; }
int p6(u8 *a) { register u8 *p __asm__("r6") = a; H(0); return p[22]; }
int p7(u8 *a) { register u8 *p __asm__("r7") = a; H(0); return p[22]; }
int p8(u8 *a) { register u8 *p __asm__("r8") = a; H(0); return p[22]; }
int p9(u8 *a) { register u8 *p __asm__("r9") = a; H(0); return p[22]; }
```

```sh
build/toolchains/agbcc/old_agbcc -O2 -mthumb-interwork -ffunction-sections \
    -o /tmp/pin.s /tmp/pin.c   # cc1 reads C or preprocessed input; -S/-c are
                                # not accepted by this binary
```

| pin | prologue / epilogue |
| --- | --- |
| `r4` | `push {r4,lr}` / `pop {r4}` |
| `r5` | `push {r5,lr}` / `pop {r5}` |
| `r6` | `push {r6,lr}` / `pop {r6}` |
| `r7` | `push {lr}` / `pop {r1}` -- **no save, and r7 is read after the call** |
| `r8` | `mov r3,r8` / `push {r3}` ... `pop {r3}` / `mov r8,r3` |
| `r9` | same staging pair as r8 |

So r4--r6 are saved normally, r8/r9 through the staging pair, and **r7 not at
all**: `register u8 *p __asm__("r7") = a;` compiled with `p` live across a
`bl` gives `add r7,r0,#0` ... `bl H` ... `add r0,r7,#0` with `push {lr}`
only. That is an ABI-violating body, and the probe still reports a length and
a partial score, so it is easy to miss.

Consequence for this corpus: `Race_Setup_19250`'s ROM keeps `p` in r7 across
both calls, so its `push {r4,r5,r6,r7,lr}` frame is not reproducible from C at
all -- the register itself cannot be saved by a pin. The earlier reading (a
parameter copy is not saved, a value assigned from a load is) was wrong: the
discriminator is register identity, not where the value comes from.
`MenuFF78_115E0`'s r8 pin saves correctly because r8 is a high register.

## r8 needs five simultaneously live call-crossing values

Two bodies this session stalled on an `r8` slot, `Race_Setup_19250` and
`MenuFF78_115E0`. In both, at most **four** values are live across a call, so
agbcc parks them in r4/r5/r6/r7 and never touches r8 -- four for the menu
body, three for the race body (the two cases are separated below). (`SoundVoiceFollow_NoteOff`
is a different limit -- it has no calls at all and is blocked on saving lr;
see the leaf-frame section above.)

Measured threshold on a scratch TU (`old_agbcc -O2 -mthumb-interwork
-ffunction-sections`), each function accumulating `f(k)` results so every
result stays live across the following calls:

```c
int n4(void) { int a=f(0), b=f(1), c=f(2), d=f(3);      return a+b+c+d; }
int n5(void) { int a=f(0), b=f(1), c=f(2), d=f(3), e=f(4);          return a+b+c+d+e; }
int n6(void) { int a=f(0), b=f(1), c=f(2), d=f(3), e=f(4), g=f(5);  return a+b+c+d+e+g; }
```

| body | prologue | r8 use |
| --- | --- | --- |
| `n4` (4 live) | `push {r4,r5,r6,lr}` | none |
| `n5` (5 live) | `push {r4,r5,r6,lr}` / `mov r6,r8` / `push {r6}` | `mov r8,r0` for the second result |
| `n6` (6 live) | `push {r4,r5,r6,lr}` / `mov r6,r9` / `mov r5,r8` / `push {r5,r6}` | r8 and r9 |

Two consequences:

* the staging scratch is not fixed: `n5` uses `mov r6,r8`, `n6` uses
  `mov r5,r8`, and the ROM bodies that do reach r8 use `mov r7,r8`. The scratch
  is whichever register is dead at that point, so a body that must show
  `mov r7,r8` also needs r7 free at function entry;
* a body whose ROM frame saves r8 but whose C has four call-crossing values
  needs an **explicit pin** to reach that frame. `MenuFF78_115E0` is the
  measured case: at the `_0800F744` call the ROM holds `rec` (r6),
  `&s16[+188]` (r7), `&s16[+190]` (r5), the selector temp (r4) and `prev`
  (r8) -- five -- but `prev`'s first value is dead there in C terms (the
  enclosing arm reassigns it right after the branch, `mov r8,r0` at +0x44),
  so the live C values are four and agbcc's r4 choice is correct. Declaring
  `register u16 prev __asm__("r8")` still reproduces the ROM staging, which
  shows the threshold is a default-allocation rule, not a hard limit: when
  the ROM's live range is longer than the C's, the pin makes the frame
  reachable and the body then lands at 252/252 bytes.

`Race_Setup_19250` is *not* that case and must not be filed under it: at its
`Sub_0802A8F0` call the ROM has only `p` (r7), `q` (r8) and `d` (r6) live --
three -- and at `_08006C10` the same three plus a `ctx` that dies in the call.
Three call-crossing values with two of them in r7/r8 is below the measured
agbcc threshold, so no spelling reaches that frame: it is ARMCC allocation
with a pointer-first preference, not a live-range difference.

## A dead stack spill comes back inside a nested block

`0x08007770` (236 B, `src/course_resource_helpers.c`) keeps its DMA3 triple in
a nested block: `volatile u32 *dma = CRH_DMA3;` plus a local `lane` there. The
ROM stores the computed `s1 << 5` to `[sp,#20]` and never reloads it -- a dead
spill -- and agbcc reproduces that store only while the offset has its own block
scope. At function scope the store disappears and the body is 228 B with a 24-byte
frame instead of 28. The rest of the body is the plain 6-arg form of the
`0x0800785C` sibling (same lookup chain, 0748C/074A8 pair per lane, same DMA
triple), so lift a family from its already-matched member before writing one
from scratch.

## A whole-body asm skeleton gets the prologue for free from four `"l"` locals

Measured on `_08001C3E0` (3640 B, `src/race_scene_b2.c`, 2026-10-24), the first
large body in the tree lifted as *skeleton + asm* rather than as a naked block.
While the whole body is still one asm statement the compiler can own the
prologue and epilogue exactly:

```c
u32 v0, v1, v2, v3;                 /* never defined, never read */
__asm__ volatile ("..." : : "l"(v0), "l"(v1), "l"(v2), "l"(v3)
        : "r0", "r1", "r2", "r3", "r8", "r9", "sl", "lr", "cc", "memory");
```

* the four `"l"` inputs are four low-register allocnos; with r0-r3 clobbered the
  allocator takes r4-r7, which is what `regs_ever_live[7]` needs (pins and
  clobbers alone never set it, see "r7 wants an unpinned global allocno"). The
  prologue comes out as the ROM's `push {r4,r5,r6,r7,lr}` / `mov r7,sl` /
  `mov r6,r9` / `mov r5,r8` / `push {r5,r6,r7}`, and the epilogue as
  `pop {r3,r4,r5}` + `mov r8,r3` / `mov r9,r4` / `mov sl,r5` +
  `pop {r4,r5,r6,r7}` / `pop {r0}` / `bx r0`.
* the r8/r9/sl clobbers alone already select the high-register save; the
  operands emit no instruction of their own.
* a compiled frame is *not* free: any local the asm addresses costs a
  `mov r0, sp` before the block, even when the template never mentions it. Keep
  the compiler frame at 0 and put `sub sp,#136` / `add sp,#136` as the block's
  first and last instructions -- the `add sp` then sits immediately before the
  compiler's `pop {r3,...}`, which is where the ROM has it. Both statements
  leave the block by themselves once real C locals produce the frame.

## `SoundSeqInterpreter` (0x0802D6F4, 344 B): two pins and chained stores

A dense 18-way switch (jump table with the base word before the entries, the
arm order = case order) lifted to exact C. Four things decide the body, all
measured in the owning TU:

* pin the pointer that holds the opcode address (`pc0` -> r1) and the byte
  target (`dst` -> r3); `b` needs no pin and lands in r2. Pinning `b` as well
  (`register u32 b __asm__("r2")`) makes the body 360 B.
* leave the `*(u8 **)(seq + 24)` data base as a *reloaded inline expression* in
  each case rather than a cached local. A store through `dst` may alias that
  field, so the ROM reloads it per case; caching it in a local drops those
  loads.
* chain the three program-counter stores: `p1 = pc0 + 1`, `p2 = p1 + 1`,
  `p3 = p2 + 1`, storing each before the next operand byte is read. Writing
  `pc0 + 2` / `pc0 + 3` lets agbcc fold both off `pc0`, which rematerialises
  the addresses, moves the operand reads to the base pointer and makes the
  body 16 B long.
* keep `default:` empty (it falls straight to the epilogue) and send the
  comparison arms to two shared tails with `goto` -- one `_0802DDD0(seq, cur,
  *(u32 *)0x0203EBB4)` call, one `cursor += 4`. That block order is what
  produces the `cmp #17 / bls table / b done` dispatch smoothing; duplicating
  the tails per arm or ordering the cases differently changes it.

## `_0800210BC` (0x080210BC, 396 B): three shapes in a call chain

A call-heavy body (14 `bl`s, four of them `_08007B18` with four stack
arguments) whose setup is otherwise forced. Only three spellings mattered:

* **One `s8` shared by the three repeating blocks.** `s = (s8)(f(a) + g(b));`
  written three times against a single function-scope `s8 s` gives that
  pseudo a callee-saved home (r4), accumulates the sum there, sign-extends in
  place (`asrs r4,r4,#24`) and copies to the argument register at each call
  (`adds r3,r4,#0`). A per-block local instead coalesces with the argument, so
  the sign-extend lands directly in r3 and all three copies disappear.
* **A connected range test needs two `goto`s.** `if (idx >= 0 && idx <= 2)`
  collapses to the unsigned `cmp #2 / bhi`, while the ROM keeps
  `cmp #2 / bgt` + `cmp #0 / blt`. Two separate `if (idx > 2) goto alt;` /
  `if (idx < 0) goto alt;` statements reproduce the signed pair.
* **Bind the base before the field read.** For
  `_08007B18(rec + 16, *(u32 *)(rec + 184), ...)` agbcc derives `rec + 16`
  from the loaded field (`adds r0,#184 / subs r0,#168`); naming the base first
  (`volatile u8 *p16 = rec + 16;`) forms it in r0 before the load, as the ROM
  does. Same lever as "a pointer offset is two instructions only while the
  address stays a pointer" above.
