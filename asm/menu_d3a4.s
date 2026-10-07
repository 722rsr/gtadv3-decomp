@ GT Advance 3 - menu selection handler
@ Region: file offset 0x00D3A4-0x00D470 (VMA 0x0800D3A4-0x0800D470).
@ Exact Thumb function; no literal pool in this slice.

.thumb
.type sub_0800D3A4, %function
sub_0800D3A4:
_0800D3A4:
  push {r4, r5, r6, r7, lr}
  mov r7, r8
  push {r7}
  adds r5, r0, #0
  lsls r1, r1, #16
  lsrs r1, r1, #16
  mov r8, r1
  lsls r2, r2, #16
  lsrs r7, r2, #16
  movs r0, #123
  bl 0x080016D0
  movs r0, #2
  bl 0x08001EA4
  strh r0, [r5, #6]
  movs r6, #0
  b.n _0800D430
_0800D3C8:
  adds r0, r6, #0
  movs r1, #0
  bl 0x08001E8C
  adds r0, r6, #0
  movs r1, #1
  bl 0x08001E8C
  lsls r0, r0, #16
  lsrs r4, r0, #16
  bl 0x08001E30
  cmp r0, #0
  beq.n _0800D408
  movs r0, #64
  ands r0, r4
  cmp r0, #0
  beq.n _0800D3F6
  ldrh r0, [r5, #4]
  cmp r0, #0
  beq.n _0800D3F6
  subs r0, #1
  strh r0, [r5, #4]
_0800D3F6:
  movs r0, #128
  ands r0, r4
  cmp r0, #0
  beq.n _0800D408
  ldrh r0, [r5, #4]
  cmp r0, #2
  bhi.n _0800D408
  adds r0, #1
  strh r0, [r5, #4]
_0800D408:
  movs r0, #1
  ands r0, r4
  cmp r0, #0
  beq.n _0800D41E
  movs r0, #1
  bl 0x08004EC0
  ldrh r0, [r5, #6]
  bl 0x08004BFC
  b.n _0800D438
_0800D41E:
  movs r0, #2
  ands r0, r4
  cmp r0, #0
  beq.n _0800D42E
  movs r0, #1
  bl 0x08004EA8
  b.n _0800D438
_0800D42E:
  adds r6, #1
_0800D430:
  bl 0x08001F8C
  cmp r6, r0
  blt.n _0800D3C8
_0800D438:
  movs r0, #128
  lsls r0, r0, #1
  ands r0, r7
  cmp r0, #0
  beq.n _0800D44E
  movs r0, #1
  bl 0x08004EC0
  movs r0, #0
  bl 0x08004BFC
_0800D44E:
  movs r0, #0
  mov r1, r8
  bl 0x08001E48
  movs r0, #1
  adds r1, r7, #0
  bl 0x08001E48
  ldrh r1, [r5, #4]
  movs r0, #2
  bl 0x08001E48
  pop {r3}
  mov r8, r3
  pop {r4, r5, r6, r7}
  pop {r0}
  bx r0
menu_d3a4_end:
