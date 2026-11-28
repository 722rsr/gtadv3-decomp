# CourseOrch_0800628C lift

Measured with pinned `old_agbcc`: 227/227 instructions aligned in the isolated
probe. The full corpus report resolves all five call targets and records
476/476 matched bytes, `EXACT`, with no unresolved bytes. The promoted body is
C with empty register barriers and no instruction-bearing assembly.
`make matching-ready` passed.

Next lead: `Sprite_PlaceEmitters` at `0x08026674` (392 B), which remains raw
assembly on the refreshed lift screen. Continue from its existing candidate;
the concrete lever is matching the ROM's `sp+0/+4` argument homes while
keeping `sp+8/+12` for `pack16` and `colorFlag`.
