@ GT Advance 3 - sound voice follow-on helpers
@ Region: file offset 0x02C390-0x02C488 (VMA 0x0802C390-0x0802C488).
@ Pure Thumb; note-off, cursor, wrapper, and interpolation helpers.

.thumb
.type sub_0802C390, %function
sub_0802C390:
_0802C390:
  push {r4, r5}
  ldr r2, [r1, #64]
  ldrb r3, [r2, #0]
  cmp r3, #128
  bcs.n _0802C3A2
  strb r3, [r1, #5]
  adds r2, #1
  str r2, [r1, #64]
  b.n _0802C3A4
_0802C3A2:
  ldrb r3, [r1, #5]
_0802C3A4:
  ldr r1, [r1, #32]
  cmp r1, #0
  beq.n _0802C3CC
  movs r4, #131
  movs r5, #64
_0802C3AE:
  ldrb r2, [r1, #0]
  tst r2, r4
  beq.n _0802C3C6
  tst r2, r5
  bne.n _0802C3C6
  ldrb r0, [r1, #17]
  cmp r0, r3
  bne.n _0802C3C6
  movs r0, #64
  orrs r2, r0
  strb r2, [r1, #0]
  b.n _0802C3CC
_0802C3C6:
  ldr r1, [r1, #52]
  cmp r1, #0
  bne.n _0802C3AE
_0802C3CC:
  pop {r4, r5}
  bx lr

.type sub_0802C3D0, %function
sub_0802C3D0:
_0802C3D0:
  movs r2, #0
  strb r2, [r1, #22]
  strb r2, [r1, #26]
  ldrb r2, [r1, #24]
  cmp r2, #0
  bne.n _0802C3E0
  movs r2, #12
  b.n _0802C3E2
_0802C3E0:
  movs r2, #3
_0802C3E2:
  ldrb r3, [r1, #0]
  orrs r3, r2
  strb r3, [r1, #0]
  bx lr
  .short 0

.type sub_0802C3EC, %function
sub_0802C3EC:
_0802C3EC:
  ldr r2, [r1, #64]
  adds r3, r2, #1
  str r3, [r1, #64]
  ldrb r3, [r2, #0]
  bx lr
  .short 0

.type sub_0802C3F8, %function
sub_0802C3F8:
_0802C3F8:
  mov ip, lr
  bl 0x0802C3EC
  strb r3, [r1, #25]
  cmp r3, #0
  bne.n _0802C408
  bl 0x0802C3D0
_0802C408:
  bx ip
  .short 0

.type sub_0802C40C, %function
sub_0802C40C:
_0802C40C:
  mov ip, lr
  bl 0x0802C3EC
  strb r3, [r1, #23]
  cmp r3, #0
  bne.n _0802C41C
  bl 0x0802C3D0
_0802C41C:
  bx ip
  .short 0

.type sub_0802C420, %function
sub_0802C420:
_0802C420:
  push {r4, r5, r6, r7, lr}
  mov ip, r0
  lsls r1, r1, #24
  lsrs r6, r1, #24
  lsls r7, r2, #24
  cmp r6, #178
  bls.n _0802C434
  movs r6, #178
  movs r7, #255
  lsls r7, r7, #24
_0802C434:
  ldr r3, _0802C47C
  adds r0, r6, r3
  ldrb r5, [r0, #0]
  ldr r4, _0802C480
  movs r2, #15
  adds r0, r5, #0
  ands r0, r2
  lsls r0, r0, #2
  adds r0, r0, r4
  lsrs r1, r5, #4
  ldr r5, [r0, #0]
  lsrs r5, r1
  adds r0, r6, #1
  adds r0, r0, r3
  ldrb r1, [r0, #0]
  adds r0, r1, #0
  ands r0, r2
  lsls r0, r0, #2
  adds r0, r0, r4
  lsrs r1, r1, #4
  ldr r0, [r0, #0]
  lsrs r0, r1
  mov r1, ip
  ldr r4, [r1, #4]
  subs r0, r0, r5
  adds r1, r7, #0
  bl 0x0802B888	@ direct; sub_0802B888's body is ARM, a symbol bl would throw an interworking thunk at the boundary
  adds r1, r0, #0
  adds r1, r5, r1
  adds r0, r4, #0
  bl 0x0802B888	@ direct; sub_0802B888's body is ARM, a symbol bl would throw an interworking thunk at the boundary
  pop {r4, r5, r6, r7}
  pop {r1}
  bx r1
_0802C47C: .word 0x08061570
_0802C480: .word 0x08061624

.type sub_0802C484, %function
sub_0802C484:
_0802C484:
  bx lr
  .short 0

sound_voice_follow_end:
