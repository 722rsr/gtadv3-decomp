@ GT Advance 3 - course streaming record helpers
@ Region: file offset 0x006B30-0x006CB8 (VMA 0x08006B30-0x08006CB8).
@ The next body, sub_08006CB8, belongs to asm/course_stream_tail.s.
@ Keep sub_08006C94's 36-byte span within this region.
@ Exact Thumb helpers and their private literal pools.

.thumb
.type sub_08006B30, %function
sub_08006B30:
_08006B30:
  push {r4, r5, r6, r7, lr}
  mov r7, r9
  mov r6, r8
  push {r6, r7}
  ldr r0, _08006BA8
  mov ip, r0
  ldr r0, _08006BAC
  ldr r0, [r0, #0]
  mov r9, r0
  ldr r1, _08006BB0
  mov r8, r1
_08006B46:
  mov r4, ip
  ldrh r1, [r4, #0]
  adds r1, #1
  strh r1, [r4, #0]
  lsls r1, r1, #16
  mov r5, r9
  ldrh r5, [r5, #46]
  lsls r0, r5, #16
  cmp r0, r1
  ble.n _08006BB8
  movs r7, #0
  ldrsh r0, [r4, r7]
  lsls r0, r0, #3
  mov r4, r9
  ldr r1, [r4, #16]
  adds r6, r1, r0
  ldr r1, _08006BB4
  movs r7, #0
  ldrsh r5, [r6, r7]
  ldr r0, [r1, #0]
  cmp r0, r5
  bgt.n _08006B46
  movs r7, #2
  ldrsh r4, [r6, r7]
  ldr r1, [r1, #4]
  cmp r1, r4
  bgt.n _08006B46
  adds r0, #16
  cmp r0, r5
  ble.n _08006B46
  adds r0, r1, #0
  adds r0, #16
  cmp r0, r4
  ble.n _08006B46
  mov r1, r8
  ldr r0, [r1, #0]
  subs r2, r5, r0
  ldr r0, [r1, #4]
  subs r3, r4, r0
  adds r0, r2, #0
  muls r0, r2
  adds r1, r3, #0
  muls r1, r3
  adds r0, r0, r1
  cmp r0, #50
  bgt.n _08006B46
  adds r0, r6, #0
  b.n _08006BBA
  .short 0x0000
_08006BA8: .word 0x0203F874
_08006BAC: .word 0x0203F760
_08006BB0: .word 0x0203F8E0
_08006BB4: .word 0x0203F8B0
_08006BB8:
  movs r0, #0
_08006BBA:
  pop {r3, r4}
  mov r8, r3
  mov r9, r4
  pop {r4, r5, r6, r7}
  pop {r1}
  bx r1
  .short 0x0000

.type sub_08006BC8, %function
sub_08006BC8:
_08006BC8:
  push {r4, r5, lr}
  adds r4, r0, #0
  adds r5, r1, #0
  bl 0x08005F98
  adds r1, r0, #0
  ldrh r2, [r5, #12]
  movs r3, #12
  ldrsh r0, [r5, r3]
  cmp r0, #0
  blt.n _08006BE2
  strh r2, [r4, #10]
  b.n _08006BE6
_08006BE2:
  movs r0, #0
  strh r0, [r4, #10]
_08006BE6:
  ldr r0, [r1, #20]
  str r0, [r4, #0]
  ldr r0, [r1, #24]
  str r0, [r4, #4]
  ldrh r0, [r1, #48]
  movs r1, #0
  movs r2, #0
  strh r0, [r4, #8]
  movs r3, #16
  ldrsh r0, [r5, r3]
  str r0, [r4, #12]
  strb r1, [r4, #16]
  ldrh r0, [r5, #14]
  strh r0, [r4, #18]
  str r2, [r4, #24]
  strb r1, [r4, #17]
  strh r2, [r4, #20]
  pop {r4, r5}
  pop {r0}
  bx r0
  .short 0x0000

.type sub_08006C10, %function
sub_08006C10:
_08006C10:
  push {r4, lr}
  adds r4, r0, #0
  movs r2, #10
  ldrsh r0, [r4, r2]
  adds r2, r0, r1
  ldr r0, [r4, #0]
  ldrb r0, [r0, #0]
  cmp r0, #0
  beq.n _08006C3E
  cmp r2, #0
  bge.n _08006C30
  movs r1, #8
  ldrsh r0, [r4, r1]
_08006C2A:
  adds r2, r2, r0
  cmp r2, #0
  blt.n _08006C2A
_08006C30:
  movs r0, #8
  ldrsh r1, [r4, r0]
  adds r0, r2, #0
  bl 0x0802DE9C
  adds r2, r0, #0
  b.n _08006C4E
_08006C3E:
  cmp r2, #0
  blt.n _08006C4A
  movs r1, #8
  ldrsh r0, [r4, r1]
  cmp r0, r2
  bgt.n _08006C4E
_08006C4A:
  movs r0, #0
  b.n _08006C58
_08006C4E:
  movs r0, #88
  adds r1, r2, #0
  muls r1, r0
  ldr r0, [r4, #4]
  adds r0, r0, r1
_08006C58:
  pop {r4}
  pop {r1}
  bx r1
  .short 0x0000

.type sub_08006C60, %function
sub_08006C60:
_08006C60:
  push {r4, lr}
  movs r1, #0
  bl 0x08006C10
  adds r4, r0, #0
  cmp r4, #0
  beq.n _08006C8A
  movs r1, #10
  ldrsh r0, [r4, r1]
  cmp r0, #0
  blt.n _08006C8A
  bl 0x08005F98
  movs r1, #10
  ldrsh r2, [r4, r1]
  lsls r1, r2, #1
  adds r1, r1, r2
  lsls r1, r1, #2
  ldr r0, [r0, #8]
  adds r0, r0, r1
  b.n _08006C8C
_08006C8A:
  movs r0, #0
_08006C8C:
  pop {r4}
  pop {r1}
  bx r1
  .short 0x0000

.type sub_08006C94, %function
sub_08006C94:
_08006C94:
  push {r4, r5, lr}
  adds r2, r0, #0
  ldr r3, [r1, #0]
  ldr r0, [r2, #12]
  subs r4, r3, r0
  ldr r1, [r1, #4]
  ldr r0, [r2, #16]
  subs r5, r1, r0
  movs r1, #20
  ldrsh r0, [r2, r1]
  muls r0, r5
  movs r3, #22
  ldrsh r1, [r2, r3]
  muls r1, r4
  subs r0, r0, r1
  pop {r4, r5}
  pop {r1}
  bx r1
course_stream_more_end:
