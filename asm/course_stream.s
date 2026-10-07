@ GT Advance 3 - course streaming proximity collector
@ Region: file offset 0x006A50-0x006B20 (VMA 0x08006A50-0x08006B20).
@ Pure Thumb; collects nearby course records and iterates the result array.

.thumb
.type sub_08006A50, %function
sub_08006A50:
_08006A50:
  push {r4, r5, r6, r7, lr}
  mov r7, r9
  mov r6, r8
  push {r6, r7}
  adds r7, r0, #0
  adds r6, r1, #0
  ldr r2, _08006ADC
  movs r1, #0
  str r1, [r2, #0]
  ldr r0, _08006AE0
  strh r1, [r0, #0]
  asrs r0, r7, #11
  cmp r0, #0
  bge.n _08006A6E
  adds r0, #15
_08006A6E:
  asrs r7, r0, #4
  asrs r0, r6, #11
  cmp r0, #0
  bge.n _08006A78
  adds r0, #15
_08006A78:
  asrs r6, r0, #4
  movs r5, #0
  ldr r1, _08006AE4
  ldr r0, [r1, #0]
  movs r3, #46
  ldrsh r0, [r0, r3]
  cmp r5, r0
  bge.n _08006ACE
  ldr r0, _08006AE8
  mov r9, r0
  mov r8, r2
_08006A8E:
  ldr r0, [r1, #0]
  lsls r1, r5, #3
  ldr r0, [r0, #16]
  adds r4, r0, r1
  movs r1, #0
  ldrsh r0, [r4, r1]
  subs r0, r7, r0
  bl 0x08005B5C
  cmp r0, #1
  bgt.n _08006AC0
  movs r2, #2
  ldrsh r0, [r4, r2]
  subs r0, r6, r0
  bl 0x08005B5C
  cmp r0, #1
  bgt.n _08006AC0
  mov r3, r8
  ldr r1, [r3, #0]
  lsls r0, r1, #2
  add r0, r9
  str r4, [r0, #0]
  adds r1, #1
  str r1, [r3, #0]
_08006AC0:
  adds r5, #1
  ldr r1, _08006AE4
  ldr r0, [r1, #0]
  movs r2, #46
  ldrsh r0, [r0, r2]
  cmp r5, r0
  blt.n _08006A8E
_08006ACE:
  pop {r3, r4}
  mov r8, r3
  mov r9, r4
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
  .short 0x0000
_08006ADC: .word 0x0203F8A8
_08006AE0: .word 0x0203F874
_08006AE4: .word 0x0203F760
_08006AE8: .word 0x0203F880

.type sub_08006AEC, %function
sub_08006AEC:
_08006AEC:
  ldr r2, _08006B0C
  movs r0, #0
  ldrsh r1, [r2, r0]
  ldr r0, _08006B10
  ldr r0, [r0, #0]
  cmp r1, r0
  bge.n _08006B18
  ldr r0, _08006B14
  lsls r1, r1, #2
  adds r1, r1, r0
  ldr r1, [r1, #0]
  ldrh r0, [r2, #0]
  adds r0, #1
  strh r0, [r2, #0]
  b.n _08006B1A
  .short 0x0000
_08006B0C: .word 0x0203F874
_08006B10: .word 0x0203F8A8
_08006B14: .word 0x0203F880
_08006B18:
  movs r1, #0
_08006B1A:
  adds r0, r1, #0
  bx lr
  .short 0x0000
course_stream_end:
