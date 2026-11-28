# `Course_CollisionLoop` lift

- Measured 2026-11-28: `lift_loop.py` reports 256/256 bytes and 123/123 aligned instructions, byte-exact apart from unresolved helper `BL` offsets. `make matching-ready` passes with the independent 188,760-byte code slice byte-identical and no raw executable bytes.
- Next lead: replace the remaining grouped asm blocks one at a time with C, preserving the `out` stack home at `sp+20` across helper calls and remeasuring the 256-byte body.
