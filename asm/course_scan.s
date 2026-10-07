@ GT Advance 3 - course record scan helper
@ Region: file offset 0x006050-0x0060EC (VMA 0x08006050-0x080060EC).
@ Pure Thumb; ARMCC high-register function with no literal pool.

.thumb
.type sub_08006050, %function
sub_08006050:
_08006050:
  push {r4, r5, r6, r7, lr}
  mov r7, sl
  mov r6, r9
  mov r5, r8
  push {r5, r6, r7}
  mov r9, r0
  mov sl, r1
  ldr r0, [r0, #0]
  mov r2, r9
  ldr r1, [r2, #4]
  bl 0x08006A50
  b.n _080060D2
_0800606A:
  mov r0, r8
  movs r1, #4
  ldrsh r5, [r0, r1]
  movs r2, #6
  ldrsh r0, [r0, r2]
  cmp r0, r5
  blt.n _080060D2
  lsls r0, r5, #1
  adds r0, r0, r5
  lsls r7, r0, #2
_0800607E:
  bl 0x08005F98
  ldr r0, [r0, #12]
  adds r4, r0, r7
  movs r1, #8
  ldrsh r0, [r4, r1]
  bl 0x08018A6C
  lsls r0, r0, #24
  cmp r0, #0
  beq.n _080060C4
  ldr r0, [r4, #0]
  mov r2, r9
  ldr r1, [r2, #0]
  subs r0, r0, r1
  mov r1, sl
  str r0, [r1, #0]
  bl 0x08005B5C
  movs r6, #128
  lsls r6, r6, #3
  cmp r0, r6
  bgt.n _080060C4
  ldr r0, [r4, #4]
  mov r2, r9
  ldr r1, [r2, #4]
  subs r0, r0, r1
  mov r1, sl
  str r0, [r1, #4]
  bl 0x08005B5C
  cmp r0, r6
  bgt.n _080060C4
  movs r0, #12
  b.n _080060DE
_080060C4:
  adds r7, #12
  adds r5, #1
  mov r2, r8
  movs r1, #6
  ldrsh r0, [r2, r1]
  cmp r0, r5
  bge.n _0800607E
_080060D2:
  bl 0x08006AEC
  mov r8, r0
  cmp r0, #0
  bne.n _0800606A
  movs r0, #0
_080060DE:
  pop {r3, r4, r5}
  mov r8, r3
  mov r9, r4
  mov sl, r5
  pop {r4, r5, r6, r7}
  pop {r1}
  bx r1
