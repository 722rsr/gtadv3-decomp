# Reading agbcc's allocator for SaveUpdate_080241C8

The remaining gap on this function is not structure — it is agbcc's greedy
register choice. To stop guessing, build an **instrumented** copy of the pinned
compiler (never replace the pinned binary):

```sh
cd build/toolchains/agbcc
cp old_agbcc old_agbcc.pinned

# in gcc/global.c, immediately before `retval = reload (get_insns (), 1, file);`
# inside global_alloc(), print the finished assignment:
#   fprintf (stderr, ";; ALLOC pseudo=%d hard=%d refs=%d live=%d size=%d\n",
#            regno, reg_renumber[regno], allocno_n_refs[order],
#            allocno_live_length[order], allocno_size[order]);

make -C gcc old -j4          # ~2 min
cp gcc/old_agbcc old_agbcc_instr
cp old_agbcc.pinned old_agbcc   # restore the pinned binary
```

Then compile a scratch TU that carries the body plus the `.set` symbol block and
the `u8/u16/u32/s16/s32/uintptr_t` typedefs, and read the dump:

```sh
build/toolchains/agbcc/old_agbcc_instr -O2 -mthumb-interwork \
    -ffunction-sections build/rtl/tu.c -o build/rtl/tu.s 2>&1 | grep ALLOC
```

`-dr` (RTL after reload) is what maps pseudo numbers back to source expressions.

## Result for the 307/368 draft

Pseudo -> source, read off `build/rtl/tu.c.rtl`, and the hard register the
allocator chose:

| pseudo | source | hard reg | ROM wants |
| ---: | --- | ---: | ---: |
| 25 | `k` | r7 | r7 |
| 26 | `kf` (8-byte struct: `k+1` at sp+0, `c2` at sp+4) | memory | memory |
| 27 | `c` | r5 | r5 |
| 30 | `c2 = c << 1` | r2 | r2 |
| 31 | `idx = c2 + k8` | r3 | r3 |
| 35 | `cnt = c + 1` | r4 | r4 |
| 33 | grid offset constant (`0x101A` etc.) | r0 | r0 |
| 46 | `idx + offset` address | r0 | r0 |
| **47** | **grid-2 flag constant `1`** | **r2** | **r3** |
| 76 | grid-3 constant `1` | memory | r1 |

So every allocation the ROM makes is reproduced, **except** which scratch
register carries the constant `1` at the grid-2 store (and the grid-3
constants). Both `r2` and `r3` are dead at that point; the ROM's compiler picked
`r3`, ours picks `r2`. It is a tie-break in the sorted allocation order, not a
liveness difference.

## Why the attempted C spellings did not force it

* A liveness pin holding `r2` across that store costs the frame: the body goes
  to 376 B (or 364 B) because `c2` then has nowhere to live.
* Making the constant a named variable pinned to `r3` oversizes to 380 B for the
  same reason.
* 72 pin combinations and a greedy pin sweep over 21 anchors x 5 registers were
  measured; the best remains 307/368.

That is the precise statement of the open difference: the last gap is an
allocator tie-break between two equally free registers, and every source-level
attempt measured so far displaces `c2` and changes the frame. The dump above
makes this a targeted problem -- the pseudo and both candidate registers are
known, so a liveness or allocation-order change is the next lever. Do not treat
the failed pin combinations as a proof that the tie cannot be broken.
