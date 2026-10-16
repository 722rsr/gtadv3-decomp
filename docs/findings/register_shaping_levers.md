# Register-shaping levers measured on the last non-exact entries

Worked while clearing the six non-EXACT manifest entries (2026-10-16). Four
became byte-exact with the recipes below -- `0x08000848`
(`PacketBuild_08000848`), `0x08008BB8` (`Course_Dispatch_08BB8`), `0x08001D974`
(`_08001D974`, 84 B) and `0x08020690` (`_080020690`) -- and two advanced:
`0x0800B89C` (`GoStart_Tick`) to 220/244 and `0x08009BF8` (796/864, unchanged).
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

## What still resists

* **Emitted-order reorders.** `_0800B89C` is at 220/244 with the whole body
  shape right except that the `adds r2,#2` bump and the `wa+off` base
  materialisation are lifted above each `ldrsh`/`lsls` group, where the ROM
  places them after it. Statement order, an in-place `selOff += 2` inside the
  address expression, and an operand-less `__asm__("")` fence between the load
  and the bump all leave the emitted order byte-identical.
* **Rematerialisation versus a pin.** `_08009BF8` needs its `result` temp to
  stay a constant-known pseudo so reload frees r2 for the loop's zero
  scratches; pinned, r2 stays live through the loop and the scratches rotate
  r3/r6 instead (and the extra r6 forces the `push {r4,r5,r6,lr}`). Unpinned,
  the const-id cases lose r2 to other allocnos. Parked at 796/864 with the
  machinery above still to be combined.
