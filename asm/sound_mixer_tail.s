@ GT Advance 3 - sound mixer Thumb epilogue
@ Region: file offset 0x02BC28-0x02BC4C (VMA 0x0802BC28-0x0802BC4C).
@ Exact Thumb continuation after the ARM mixer loop; private Smsh pool.

.thumb
.type sub_0802BC28, %function
sub_0802BC28:
_0802BC28:
  ldr r0, [sp, #4]
  subs r0, #1
  ble.n _0802BC32
  adds r4, #64
  b _0802B990
_0802BC32:
  ldr r0, [sp, #24]
  ldr r3, _0802BC48
  str r3, [r0, #0]
  add sp, #28
  pop {r0, r1, r2, r3, r4, r5, r6, r7}
  mov r8, r0
  mov r9, r1
  mov sl, r2
  mov fp, r3
  pop {r3}
  bx r3
_0802BC48: .word 0x68736D53
