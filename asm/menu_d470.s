@ GT Advance 3 - menu event dispatcher
@ Region: file offset 0x00D470-0x00D4EA (VMA 0x0800D470-0x0800D4EA).
@ Exact Thumb dispatcher, handler table, and local stubs.

.thumb
.type sub_0800D470, %function
sub_0800D470:
_0800D470:
  push {r4, lr}
  adds r4, r1, #0
  subs r0, #1
  cmp r0, #10
  bhi.n _0800D4E4
  lsls r0, r0, #2
  ldr r1, _0800D484
  adds r0, r0, r1
  ldr r0, [r0, #0]
  mov pc, r0
_0800D484: .word 0x0800D488
  .word 0x0800D4BE
  .word 0x0800D4B4
  .word 0x0800D4E4
  .word 0x0800D4E4
  .word 0x0800D4D6
  .word 0x0800D4C6
  .word 0x0800D4DE
  .word 0x0800D4E4
  .word 0x0800D4E4
  .word 0x0800D4E4
  .word 0x0800D4E4
_0800D4B4:
  adds r0, r3, #0
  adds r1, r4, #0
  bl 0x0800D280
  b.n _0800D4E4
_0800D4BE:
  adds r0, r3, #0
  bl 0x0800D298
  b.n _0800D4E4
_0800D4C6:
  lsls r1, r4, #16
  lsrs r1, r1, #16
  lsls r2, r2, #16
  lsrs r2, r2, #16
  adds r0, r3, #0
  bl 0x0800D3A4
  b.n _0800D4E4
_0800D4D6:
  adds r0, r3, #0
  bl 0x0800D2AC
  b.n _0800D4E4
_0800D4DE:
  adds r0, r3, #0
  bl 0x0800D2B0
_0800D4E4:
  pop {r4}
  pop {r0}
  bx r0
menu_d470_end:
