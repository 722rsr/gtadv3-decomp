# The 0x080148D8 lift: a dead clamp and a named offset

`MenuFF78_148D8` (0x080148D8, 304 B, `src/menu_ff78_n.c`) is now plain C: a
six-iteration loop over `rec` that picks two table words
(`u32[176+j*12]`, `u32[180+j*12]`) and calls `0x08007BFC` with mode 8, 6 or 7.
The body is byte-exact in the isolated probe and in the independent slice.

Two spellings were load-bearing, and neither is about arithmetic:

* the then-arm's `if (v < 0) v = 0;` is **dead** (`v` is not one of the call's
  arguments there) and agbcc deletes it, while the ROM keeps it;
* the `+24` in the address argument must be a **named value**, or combine
  reassociates it into the `i * 48` scale.

Every number below was measured with the scratch-TU variant grader:

```sh
python3 tools/lift_loop.py src/menu_ff78_n.c MenuFF78_148D8 \
    --vma 0x080148D8 --len 304 --variant <body.c> [<body.c> ...]
```

The installed body measures `150/150 instructions, 304/304 bytes,
BYTE-EXACT apart from unresolved call targets`, and classifies `c_fakematch`
(one empty barrier, ten register pins).

## A dead clamp: an input-only `"r"` read after the assignment

The ROM's then-arm (the `v == s16[rec+156]` case) is

```
0x08014932  cmp   r1, r0
0x08014934  bne.n 0x8014968
0x08014936  cmp   r1, #0
0x08014938  bge.n 0x801493C
0x0801493A  movs  r1, #0
0x0801493C  adds  r4, r5, #1      @ table index, i + 1
```

`v` (r1) is not used after the clamp in that arm: the call takes `rec`, the two
table words, `i * 48 + base + 24`, and literals. agbcc's dead-code pass removes
all three instructions, and the candidate's `bne.n` branches straight into the
table loads: **300 B** (298 B of code plus the alignment nop) with **101 of
304 bytes** aligned.

An input-only read after the clamp keeps it:

```c
if (v < 0)
    v = 0;
__asm__ volatile("" : : "r"(v));
```

The asm emits no bytes; it makes the clamp's result live, so the three
instructions survive. `"r"` is enough -- no `"+r"`, no memory clobber.

The **position is load-bearing**. The same read placed *before* the clamp
measures the same as deleting it (300 B, 101/304): the clamp is still dead,
because nothing reads its result. Only a read on the clamp's far side keeps it.

The else-arm needs nothing: `v` there feeds the `v >= 10` / `v <= 2` tests, so
its clamp is live and falls out of the C by itself.

## A constant added to a scaled index must be a named value

The ROM forms the fourth argument after the `i * 48` scale:

```
lsls r3, r5, #1
adds r3, r3, r5
lsls r3, r3, #4        @ i * 48
mov  r0, r8
adds r0, #24           @ base + 24, in the ROM's r0
adds r3, r3, r0        @ i * 48 + (base + 24)
```

Spelling the offset inline as a constant lets combine fold it into the scale:

```c
_08007BFC(rec, a1, a2, i * 48 + (base + 24), 48, 6, 1, 0, 0);
```

```
adds r3, #24
add  r3, r8
```

Two instructions where the ROM has three, 2 B shorter (302 B of code, 304 B
with the alignment nop), and **139 of 304 bytes** aligned -- the whole tail
below the address is displaced.

Naming the offset keeps it out of the reassociation:

```c
s32 pos;

pos = base + 24;
...
_08007BFC(rec, a1, a2, i * 48 + pos, 48, 6, 1, 0, 0);
```

and the ROM's `mov r0,r8 / adds r0,#24 / adds r3,r3,r0` comes back. Measured
exact (304/304) with `pos` a plain local, with the assignment as its own
statement **or** inside the argument list -- the register it lands in is the
allocator's choice, and the assignment placement is free *as long as the
variable is not pinned*.

### With `pos` pinned, the assignment must stay in the argument list

The installed body pins `pos` to r0 (the ROM's register for it) and assigns it
inside the argument list. Moving that assignment out does not just change the
layout:

```c
register s32 pos __asm__("r0");

pos = base + 24;                  /* own statement, before the table loads */
...
_08007BFC(rec, a1, a2, i * 48 + pos, 48, 6, 1, 0, 0);
```

measures **300 B / 138 of 304**, and it is a miscompile rather than a layout
difference: the arm's argument is computed from `rec + 180` instead of
`base + 24`. `pos`'s live range now spans the table temp `t`, which is also
pinned to r0, and the temp's `adds r0,#180` overwrites the offset before the
add that consumes it:

```
0x08014944  adds r0, r6, #0
0x08014946  adds r0, #176
0x08014948  adds r0, r0, r2
0x0801494A  ldr  r1, [r0, #0]
0x0801494C  adds r0, r6, #0
0x0801494E  adds r0, #180
0x08014950  adds r2, r0, r2       @ t's home is clobbered: r0 is rec+180 now
0x08014952  ldr  r2, [r2, #0]
...
0x0801495A  adds r3, r3, r0       @ i*48 + (rec + 180), not i*48 + base + 24
```

Two regvars pinned to one register with overlapping live ranges are outside
what agbcc keeps apart. Inside the argument list `pos` is live only after
`t`'s last use, the ranges never overlap, and the body is exact. Dropping the
`pos` pin removes the hazard outright: both placements then measure exact.

Measured spellings of the fourth argument (code bytes, then bytes aligned of
the 304 B span; the extracted blob adds a 2 B alignment nop where the code
length is not a multiple of four):

| spelling | code B | aligned | result |
| --- | ---: | ---: | --- |
| `i * 48 + (base + 24)` | 302 | 139 | `+24` folds into the scale |
| `i * 48 + pos`, `pos` plain, own statement | 304 | 304 | exact |
| `i * 48 + pos`, `pos` plain, in the argument | 304 | 304 | exact |
| `i * 48 + (pos = base + 24)`, `pos` pinned r0 | 304 | 304 | installed body |
| `i * 48 + pos`, `pos` pinned r0, own statement | 300 | 138 | wrong argument |

## What the landed body still needs

Removal sweep over the installed body (`--variant` grading, one pin dropped at
a time):

| pin dropped | bytes | aligned | first difference |
| --- | ---: | ---: | --- |
| `t` r0 (per arm) | 304 | 298 | 0x08014950 |
| `base` r8 | 304 | 304 | -- |
| `off` r2 (per arm) | 304 | 304 | -- |
| `pos` r0 (per arm) | 304 | 304 | -- |

Only the temp pin is load-bearing in the final spelling. With `t` a plain
local, the `+180` table word cannot be written in place: the ROM chains the
pointer in r0 (`adds r0, #180`, then `adds r0, r0, r2 / ldr r2, [r0]`), while
an unpinned pseudo's last use is the load, so the sum gets a fresh pseudo and
the add writes elsewhere (`adds r2, r0, r2 / ldr r2, [r2]`).

`base` r8 and `off` r2 remain in the source from earlier spellings of this
loop; on the final expression forms each can be dropped and the body still
measures exact. That is the same lesson as the pin sweep in
[the indexed halfword scratch](indexed_halfword_scratch.md): once the
expression spelling settles, re-measure by removing pins, not only by adding
them.
