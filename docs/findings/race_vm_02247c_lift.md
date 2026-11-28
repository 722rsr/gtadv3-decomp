# RaceVM_02247C measured C lift

The 424-byte body is now a C lift with grouped asm splices for measured
register and memory-transfer shapes. The canonical source measures 424 bytes,
195/195 instructions aligned, byte exact apart from unresolved call targets;
`make matching-ready` passes. Next lead: replace the 16-instruction default-arm
splice with C while preserving the r5/r4/r6 lanes and 44-byte frame, then
remeasure.
