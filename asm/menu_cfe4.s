@ GT Advance 3 - menu record initializer
@ Region: file offset 0x00CFE4-0x00D048 (VMA 0x0800CFE4-0x0800D048).
@ Pure Thumb; initializer with private literal pool.

.thumb
.type sub_0800CFE4, %function
sub_0800CFE4:
_0800CFE4:
  push {r4, lr}
  adds r4, r0, #0
  bl 0x08004B68
  movs r0, #0
  bl 0x08004CA8
  strh r0, [r4, #0]
  ldr r1, _0800D03C
  movs r0, #0
  movs r2, #6
  bl 0x08007664
  ldr r0, _0800D040
  ldr r1, _0800D044
  adds r0, r0, r1
  ldrh r0, [r0, #0]
  strh r0, [r4, #6]
  bl 0x08026948
  str r0, [r4, #28]
  movs r2, #6
  ldrsh r1, [r4, r2]
  movs r3, #8
  ldrsh r2, [r4, r3]
  bl 0x08026A4C
  ldr r0, [r4, #28]
  movs r1, #1
  bl 0x08026A58
  ldr r0, [r4, #28]
  movs r1, #0
  bl 0x08026A60
  ldr r0, [r4, #28]
  movs r2, #6
  ldrsh r1, [r4, r2]
  bl 0x08026938
  pop {r4}
  pop {r0}
  bx r0
  .short 0
_0800D03C: .word 0x08292B40
_0800D040: .word 0x03001780
_0800D044: .word 0x00000574
