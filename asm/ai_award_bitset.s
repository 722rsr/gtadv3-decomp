@ GT Advance 3 - collection bitset setter
@ Region: file offset 0x025F78-0x025FAC (VMA 0x08025F78-0x08025FAC).
@ Exact Thumb leaf with private base/table pools.

.thumb
.type sub_08025F78, %function
sub_08025F78:
_08025F78:
  adds r1, r0, #0
  ldr r2, _08025FA4
  cmp r1, #0
  bge.n _08025F82
  adds r0, r1, #7
_08025F82:
  asrs r0, r0, #3
  adds r2, #32
  adds r2, r0, r2
  ldr r3, _08025FA8
  lsls r0, r0, #3
  subs r0, r1, r0
  movs r1, #7
  ands r0, r1
  lsls r0, r0, #1
  adds r0, r0, r3
  ldrb r1, [r2, #0]
  ldrb r0, [r0, #0]
  orrs r1, r0
  adds r0, r1, #0
  strb r0, [r2, #0]
  bx lr
  .short 0x0000
_08025FA4: .word 0x03001780
_08025FA8: .word 0x080CD9D4
