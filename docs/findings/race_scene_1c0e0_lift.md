# _08001C0E0 measured C attempt

The 412-byte nearest-record scan remains raw. The structured C candidate
measures 108 matched bytes in a 388-byte body (first difference +0x0C). A new
early-guard/do-while version with r4-r9 pins measures 68 matched bytes; a
register sweep on that version found one +1-instruction placement, while a
register sweep and a forced-stack-home sweep on the structured candidate
found no improvement (92 instructions aligned in both sweeps). Next lead:
splice the nearest-record loop as one grouped block and resume C at the
heading/scale tail.
