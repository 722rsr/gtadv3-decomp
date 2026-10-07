@ GT Advance 3 - menu record updater
@ Region: file offset 0x00D048-0x00D13C (VMA 0x0800D048-0x0800D13C).
@ Pure Thumb; ARMCC high-register updater with private literal pool.

.thumb
.type sub_0800D048, %function
sub_0800D048:
_0800D048:
  push {r4, r5, r6, r7, lr}
  mov r7, r9
  mov r6, r8
  push {r6, r7}
  adds r4, r0, #0
  lsls r2, r2, #16
  lsrs r5, r2, #16
  bl 0x08004B68
  movs r1, #6
  ldrsh r0, [r4, r1]
  mov r8, r0
  movs r7, #0
  movs r6, #0
  movs r3, #8
  ldrsh r2, [r4, r3]
  mov r9, r2
  movs r0, #2
  ands r0, r5
  cmp r0, #0
  beq.n _0800D078
  movs r0, #1
  bl 0x08004EA8
_0800D078:
  movs r0, #1
  ands r0, r5
  cmp r0, #0
  beq.n _0800D098
  ldr r1, _0800D130
  ldrh r2, [r4, #6]
  ldr r3, _0800D134
  adds r0, r1, r3
  strh r2, [r0, #0]
  ldrh r0, [r4, #8]
  ldr r2, _0800D138
  adds r1, r1, r2
  strb r0, [r1, #0]
  movs r0, #1
  bl 0x08004EA8
_0800D098:
  movs r0, #32
  ands r0, r5
  cmp r0, #0
  beq.n _0800D0A2
  subs r7, #1
_0800D0A2:
  movs r0, #16
  ands r0, r5
  cmp r0, #0
  beq.n _0800D0AC
  adds r7, #1
_0800D0AC:
  movs r0, #64
  ands r0, r5
  cmp r0, #0
  beq.n _0800D0B6
  subs r6, #1
_0800D0B6:
  movs r0, #128
  ands r0, r5
  cmp r0, #0
  beq.n _0800D0C0
  adds r6, #1
_0800D0C0:
  ldrh r3, [r4, #8]
  adds r0, r3, r6
  movs r1, #0
  strh r0, [r4, #8]
  lsls r0, r0, #16
  cmp r0, #0
  bge.n _0800D0D2
  movs r0, #4
  strh r0, [r4, #8]
_0800D0D2:
  movs r2, #8
  ldrsh r0, [r4, r2]
  cmp r0, #4
  ble.n _0800D0DC
  strh r1, [r4, #8]
_0800D0DC:
  ldrh r3, [r4, #6]
  adds r0, r3, r7
  strh r0, [r4, #6]
  lsls r0, r0, #16
  cmp r0, #0
  bge.n _0800D0EA
  strh r1, [r4, #6]
_0800D0EA:
  movs r1, #6
  ldrsh r0, [r4, r1]
  cmp r0, #96
  ble.n _0800D0F6
  movs r0, #96
  strh r0, [r4, #6]
_0800D0F6:
  movs r2, #6
  ldrsh r1, [r4, r2]
  cmp r1, r8
  beq.n _0800D112
  ldr r0, [r4, #28]
  movs r3, #8
  ldrsh r2, [r4, r3]
  bl 0x08026A4C
  ldr r0, [r4, #28]
  movs r2, #6
  ldrsh r1, [r4, r2]
  bl 0x08026938
_0800D112:
  movs r3, #8
  ldrsh r2, [r4, r3]
  cmp r2, r9
  beq.n _0800D124
  ldr r0, [r4, #28]
  movs r3, #6
  ldrsh r1, [r4, r3]
  bl 0x08026A4C
_0800D124:
  pop {r3, r4}
  mov r8, r3
  mov r9, r4
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
_0800D130: .word 0x03001780
_0800D134: .word 0x00000574
_0800D138: .word 0x00001151
