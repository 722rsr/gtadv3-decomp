# Signed narrow loads and arithmetic reassociation

With agbcc, a volatile signed byte or halfword read uses an unsigned load and
separate sign extension; it does not select `ldrsb` or `ldrsh`. A nonvolatile
signed lvalue read into an `int` can use the single sign-extending load when the
operand form permits it.

The compiler can rewrite `a - (b - 48)` as `(a + 48) - b`. If instruction
placement matters, a statement expression with an empty `"+r"` asm constraint
on the intermediate value can inhibit that reassociation while keeping the
read at the expression site.
