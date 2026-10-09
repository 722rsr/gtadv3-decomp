@ GT Advance 3 - course streaming state update
@ Region: file offset 0x006CB8-0x006D68 (VMA 0x08006CB8-0x08006D68).
@ Exact Thumb continuation with two inline literal pools.

.thumb
.type sub_08006CB8, %function
sub_08006CB8:
_08006CB8:
  push {r4, r5, r6, lr}
  adds r4, r0, #0
  adds r6, r1, #0
  movs r1, #0
  bl 0x08006C10
  adds r5, r0, #0
  movs r0, #0
  strb r0, [r4, #16]
  cmp r5, #0
  beq.n _08006D5C
  adds r0, r5, #0
  adds r1, r6, #0
  bl 0x08006C94
  cmp r0, #0
  blt.n _08006D1C
  movs r1, #1
  negs r1, r1
  adds r0, r4, #0
  bl 0x08006C10
  adds r5, r0, #0
  cmp r5, #0
  beq.n _08006D5C
  adds r1, r6, #0
  bl 0x08006C94
  cmp r0, #0
  blt.n _08006D5C
  ldrh r0, [r4, #10]
  subs r0, #1
  strh r0, [r4, #10]
  ldrh r0, [r4, #18]
  subs r0, #1
  strh r0, [r4, #18]
  ldrb r0, [r4, #16]
  subs r0, #1
  strb r0, [r4, #16]
  movs r0, #8
  ldrsh r1, [r5, r0]
  ldr r0, [r4, #12]
  subs r0, r0, r1
  str r0, [r4, #12]
  ldr r1, _08006D18
  cmp r0, r1
  bge.n _08006D5C
  b.n _08006D5A
_08006D18: .word 0xFFFF8000
_08006D1C:
  adds r0, r4, #0
  movs r1, #1
  bl 0x08006C10
  cmp r0, #0
  beq.n _08006D5C
  ldrh r0, [r4, #10]
  adds r0, #1
  strh r0, [r4, #10]
  ldrh r2, [r4, #18]
  adds r2, #1
  strh r2, [r4, #18]
  ldrb r3, [r4, #16]
  adds r3, #1
  strb r3, [r4, #16]
  lsls r1, r2, #16
  ldrh r6, [r4, #22]
  lsls r0, r6, #16
  cmp r0, r1
  bge.n _08006D4A
  adds r0, r3, #1
  strb r0, [r4, #16]
  strh r2, [r4, #22]
_08006D4A:
  movs r0, #8
  ldrsh r1, [r5, r0]
  ldr r0, [r4, #12]
  adds r0, r0, r1
  str r0, [r4, #12]
  ldr r1, _08006D64
  cmp r0, r1
  ble.n _08006D5C
_08006D5A:
  str r1, [r4, #12]
_08006D5C:
  pop {r4, r5, r6}
  pop {r0}
  bx r0
  .short 0x0000
_08006D64: .word 0x00007FFF
course_stream_tail_end:
