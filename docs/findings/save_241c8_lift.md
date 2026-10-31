# SaveUpdate_080241C8 lift attempt: near-miss at 352/368

`SaveUpdate_080241C8` (`0x080241C8`, 368 B, 174 instructions, 5 callees) is the
last assembly body in `src/save.c`. It is a 2x4 grid updater. This note records
the measured drafts and the two walls that stopped an exact match. The body is
still assembly; nothing here was promoted.

## What the body does

```
for (k = 0; k <= 1; k++)
  for (c = 0; c <= 3; c++) {
    v = _08025D64(k, c);
    idx = k*8 + c*2;
    WA[0xFFA + idx] = v;                  // grid 1
    if (v == 11) {
      WA[0x101A + idx] = 1;               // grid 2
      WA[0xFF6] = c + 1;                  // count
      if (k == 0) {
        if ((s16)WA[0xFF0] == 0) { if (c+1 > 3) WA[0xFF6] = 3; }
        else                     { if (c+1 > 3) { WA[0xFF6] = 0; WA[0xFF2] = 1; } }
      } else {
        if (WA[0xFF0] == 1 && c+1 > 3) WA[0xFF6] = 3;
        WA[0xFF0] = 1; WA[0xFF2] = 1;
      }
    } else {
      WA[0x101A + idx] = 0;
    }
    if ((s16)WA[0xFFA + idx] > 2) WA[0x102A + idx] = 1;   // grid 3
    else                          WA[0x102A + idx] = 0;
    _08002714(k, c, _0800A9A0(k, c));
    _080026DC(k, c, _0800A9E0(k, c));
  }
```

The two `_0800A9xx` calls return the value their `_08002714`/`_080026DC`
partner stores, which is why the ROM copies `r0` to `r2` between them.

## Measured drafts

All drafts are under ignored `build/lift/save241c8/`.

| draft | shape | candidate | aligned | first diff |
| --- | --- | ---: | ---: | ---: |
| v17 | sweep best: pinned `r8` base, `sl` state, barrier on `idx`/`v` | 352 B | 104/174 | byte 12 |
| v14 | + `k`/`k8` pins (ignored by the loop pass) | 344 B | 91 | byte 12 |
| v13 | pinned `r8`/`sl`, natural counters | 344 B | 36 | byte 12 |
| v6 | plain C, `s16` reads, `k8`/`c2` locals | 308 B | 57 | byte 10 |
| v24 | `struct SaveWork24` field accesses | 320 B | 28 | byte 10 |

The best draft reproduces the **entire tail byte for byte**: the four calls in
order, the `r2` return copies, `c++` through `r4`, the two loop tests, and the
whole epilogue. The setup matches through `sl = WA + 0xFF0` in v13/v14. The
remaining 16 bytes (4 instructions) are all inside the inner body.

## Findings

- **`volatile s16` is wrong here.** `*(volatile s16 *)p` compiles to
  `ldrh` + `lsls/asrs`; a plain `*(s16 *)p` compiles to `ldrsh` with the
  zero-index idiom `movs rX,#0; ldrsh rY,[base,rX]`. The ROM uses `ldrsh`
  for the grid read at `0x080242B4` and for `state` in the `k==0` arm, and
  `ldrh` for `state` in the `k!=0` arm — so the source mixed `s16` and `u16`
  lvalues, and the `volatile` spelling costs two instructions per read.
- **Call order comes first.** The ROM calls `_08025D64` before computing
  `c*2`/`idx`; a draft that computes the index first hoists it above the call
  and loses the alignment at `0x080241EC`. Keeping `v = f(k,c)` before
  `idx = ...` is required.
- **The ROM pools each `WA+offset` separately** (14 literal words, including
  the same `0xFFA` and `-0xFFA` three times each). A shared `u16 *wa` base
  lets agbcc CSE the offsets into `subs`/`adds` differences and drop several
  pool words, which is most of the 16-byte shortfall. Separate pointer
  locals (`g1`, `g2`, `g3`, ...) make it worse, not better: agbcc derives them
  from one anchor (`movs r1,#10; negs r1,r1; add r1,r9`).
- **`k` must land in `r7`.** In the ROM `r7`=k, `r9`=k*8, `r5`=c, `sp+0`=k+1,
  `sp+4`=c*2. Natural C gives `r6`=k. Pinning `k` is ignored by the loop pass
  (`register ... __asm__("r7")` is overridden after loop optimization); a
  `+r` barrier on a *later* variable (`k8`) moved `k` into `r7` and fixed the
  `sp+0` spill, but left `k8` spilled to `sp+4` instead of `r9`.

## Next move

The tail and setup are done; only the inner body's base arithmetic remains.
The most promising lead is forcing the `WA+offset` literals to stay separate
by using a *different symbol per field* (like `SaveBlock1Flags_Set`/
`SaveBlock1Masks_Set` in this file) instead of one `RAM_SYM` base plus
offsets. Each symbol reference becomes a relocation the compiler cannot fold
into another, which should restore the ROM's separate pool words and the
`WA + const` address adds.
