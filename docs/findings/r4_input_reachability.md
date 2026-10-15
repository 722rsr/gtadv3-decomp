# Incoming r4 and r5 values

A function can read an incoming callee-saved register through a fixed-register
variable. A file-scope register variable can keep `r4` available to a leaf
without adding it to the function's saved-register set.

When the function must preserve `r4`, use a local register variable and an
empty output constraint to expose the incoming value while keeping normal
save/restore behavior:

```c
register u32 acc __asm__("r4");
__asm__ volatile ("" : "=r" (acc));
```

The constraint emits no instruction. This technique applies to the `r4`/`r5`
input cases; other register conventions require separate evidence.
