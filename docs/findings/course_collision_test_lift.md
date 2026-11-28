# `Course_CollisionTest` lift

- Measured 2026-11-28: the complete body is C with empty register and memory barriers. All 175 instruction rows align; `make matching-ready` confirms the fixed 356-byte slice exactly, including both resolved `_08005E14` calls, the 36-byte scratch frame, and the two-byte tail beyond the 354-byte symbol.
- Register-shaped locals and barriers preserve the ROM's instruction and frame layout. There are no instruction-bearing inline assembly groups left in `Course_CollisionTest`.
- Next lead: refresh `backlog_census.py --lift-screen` and select the next eligible raw body.
