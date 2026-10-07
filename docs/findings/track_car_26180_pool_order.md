# Pool-load ordering in the track-award helper

`_080026180`, at ROM VMA `0x08026180` (file offset `0x026180`), has a
48-byte span. Its implementation is in `src/track_car_helpers.c`.

## Required instruction order

```asm
08026190: bl   0x08005758
08026194: ldr  r1, =0x030015F0
08026196: lsls r4, r4, #16
08026198: asrs r4, r4, #13
0802619a: adds r4, r4, r1
0802619c: strh r0, [r4, #0]
```

The constant's only use is the add at +0x1A, but its pool load precedes the
scale pair. A folded C constant tends to be loaded at its first use, after
that arithmetic. An assembler-resolved absolute symbol keeps the base as a
separate literal:

```c
extern u8 TrackAwardRecBase[];
__asm__(".globl TrackAwardRecBase\nTrackAwardRecBase = 0x030015F0\n");
base = (uintptr_t)TrackAwardRecBase;
```

The declaration and definition belong inside the function body. The independent
link retains that function's section; a file-scope absolute definition may
otherwise be omitted.

## Width and signedness

The offset must stay full width as `(u32)((iw << 16) >> 13)`, with `iw` a
named `s32`. Assigning to an `s16` offset adds truncation and produces a
different shift pair. Plain multiplication of the zero-extended input also
loses the ROM's wrapping signed interpretation: input 65535 must yield -8,
not 524280.

Both the assembler-resolved base and the full-width offset are required for
the 48-byte match. Similar-looking bodies require their own ABI and byte
checks; this example does not establish their semantics.
