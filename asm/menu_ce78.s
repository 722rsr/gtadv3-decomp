@ GT Advance 3 - menu record handlers
@ Region: file offset 0x00CE78-0x00CFE4 (VMA 0x0800CE78-0x0800CFE4).
@ Pure Thumb; four ARMCC handlers with private literal pools.

.thumb
.type sub_0800CE78, %function
sub_0800CE78:
_0800CE78:
  push {r4, r5, lr}
  adds r5, r0, #0
  bl 0x08004B68
  movs r0, #0
  bl 0x08004CA8
  strh r0, [r5, #0]
  ldr r0, _0800CEC8
  ldr r1, _0800CECC
  adds r0, r0, r1
  ldrh r0, [r0, #0]
  strh r0, [r5, #6]
  bl 0x08026948
  ldr r2, _0800CED0
  adds r4, r5, r2
  str r0, [r4, #0]
  movs r2, #6
  ldrsh r1, [r5, r2]
  movs r2, #0
  bl 0x08026A4C
  ldr r0, [r4, #0]
  movs r1, #1
  bl 0x08026A58
  ldr r0, [r4, #0]
  movs r1, #0
  bl 0x08026A60
  ldr r0, [r4, #0]
  movs r2, #6
  ldrsh r1, [r5, r2]
  bl 0x08026938
  pop {r4, r5}
  pop {r0}
  bx r0
  .short 0
_0800CEC8: .word 0x03001780
_0800CECC: .word 0x00000574
_0800CED0: .word 0x00001018

.type sub_0800CED4, %function
sub_0800CED4:
_0800CED4:
  push {r4, r5, r6, r7, lr}
  mov r7, r9
  mov r6, r8
  push {r6, r7}
  adds r5, r0, #0
  lsls r2, r2, #16
  lsrs r4, r2, #16
  adds r6, r4, #0
  bl 0x08004B68
  movs r1, #6
  ldrsh r0, [r5, r1]
  mov r8, r0
  movs r7, #0
  movs r2, #2
  mov r9, r2
  adds r0, r4, #0
  ands r0, r2
  cmp r0, #0
  beq.n _0800CF02
  movs r0, #6
  bl 0x08004EA8
_0800CF02:
  movs r0, #32
  ands r0, r4
  cmp r0, #0
  beq.n _0800CF0C
  subs r7, #1
_0800CF0C:
  movs r0, #16
  ands r6, r0
  cmp r6, #0
  beq.n _0800CF16
  adds r7, #1
_0800CF16:
  ldrh r1, [r5, #6]
  adds r0, r1, r7
  strh r0, [r5, #6]
  lsls r0, r0, #16
  cmp r0, #0
  bge.n _0800CF26
  movs r0, #0
  strh r0, [r5, #6]
_0800CF26:
  movs r2, #6
  ldrsh r0, [r5, r2]
  cmp r0, #2
  ble.n _0800CF32
  mov r0, r9
  strh r0, [r5, #6]
_0800CF32:
  movs r2, #6
  ldrsh r1, [r5, r2]
  cmp r1, r8
  beq.n _0800CF50
  ldr r0, _0800CF5C
  adds r4, r5, r0
  ldr r0, [r4, #0]
  movs r2, #0
  bl 0x08026A4C
  ldr r0, [r4, #0]
  movs r2, #6
  ldrsh r1, [r5, r2]
  bl 0x08026938
_0800CF50:
  pop {r3, r4}
  mov r8, r3
  mov r9, r4
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
_0800CF5C: .word 0x00001018

.type sub_0800CF60, %function
sub_0800CF60:
_0800CF60:
  push {r4, lr}
  adds r4, r0, #0
  ldr r2, _0800CF94
  movs r0, #96
  movs r1, #15
  bl 0x080038A4
  ldr r0, _0800CF98
  ldrh r0, [r0, #12]
  lsls r2, r0, #16
  asrs r2, r2, #19
  movs r0, #120
  movs r1, #32
  bl 0x08003978
  ldr r0, _0800CF9C
  adds r4, r4, r0
  ldr r0, [r4, #0]
  movs r1, #120
  movs r2, #100
  bl 0x08026A20
  pop {r4}
  pop {r0}
  bx r0
  .short 0
_0800CF94: .word 0x0805F894
_0800CF98: .word 0x030035C0
_0800CF9C: .word 0x00001018

.type sub_0800CFA0, %function
sub_0800CFA0:
_0800CFA0:
  push {lr}
  cmp r0, #2
  beq.n _0800CFBA
  cmp r0, #2
  bhi.n _0800CFB0
  cmp r0, #1
  beq.n _0800CFDA
  b.n _0800CFE0
_0800CFB0:
  cmp r0, #6
  beq.n _0800CFCA
  cmp r0, #7
  beq.n _0800CFC2
  b.n _0800CFE0
_0800CFBA:
  adds r0, r3, #0
  bl 0x0800CE70
  b.n _0800CFE0
_0800CFC2:
  adds r0, r3, #0
  bl 0x0800CF60
  b.n _0800CFE0
_0800CFCA:
  lsls r1, r1, #16
  lsrs r1, r1, #16
  lsls r2, r2, #16
  lsrs r2, r2, #16
  adds r0, r3, #0
  bl 0x0800CED4
  b.n _0800CFE0
_0800CFDA:
  adds r0, r3, #0
  bl 0x0800CE78
_0800CFE0:
  pop {r0}
  bx r0
menu_ce78_end:
