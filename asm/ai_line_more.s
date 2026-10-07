@ GT Advance 3 - AI lineup record accessors
@ Region: file offset 0x0255C4-0x02581C (VMA 0x080255C4-0x0802581C).
@ Pure Thumb; small accessors over the 8-byte lineup catalog records.

.thumb
.type sub_080255C4, %function
sub_080255C4:
_080255C4:
  adds r2, r0, #0
  ldr r3, _0802563C
  movs r1, #2
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  movs r0, #6
  ldrsb r0, [r2, r0]
  lsls r0, r0, #3
  adds r0, r0, r3
  adds r0, #33
  ldrb r1, [r1, #1]
  ldrb r0, [r0, #0]
  adds r0, r1, r0
  movs r1, #4
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #65
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #7
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #97
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #5
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #129
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #9
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #161
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #3
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #193
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #8
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #225
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  lsls r0, r0, #24
  asrs r0, r0, #24
  bx lr
  .short 0
_0802563C: .word 0x080CD6A8

.type sub_08025640, %function
sub_08025640:
_08025640:
  adds r2, r0, #0
  ldr r3, _080256B8
  movs r1, #2
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  movs r0, #6
  ldrsb r0, [r2, r0]
  lsls r0, r0, #3
  adds r0, r0, r3
  adds r0, #32
  ldrb r1, [r1, #0]
  ldrb r0, [r0, #0]
  adds r0, r1, r0
  movs r1, #4
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #64
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #7
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #96
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #5
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #128
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #9
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #160
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #3
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #192
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  movs r1, #8
  ldrsb r1, [r2, r1]
  lsls r1, r1, #3
  adds r1, r1, r3
  adds r1, #224
  ldrb r1, [r1, #0]
  adds r0, r1, r0
  lsls r0, r0, #24
  asrs r0, r0, #24
  bx lr
  .short 0
_080256B8: .word 0x080CD6A8

.type sub_080256BC, %function
sub_080256BC:
_080256BC:
  ldr r3, _080256D4
  lsls r2, r1, #1
  adds r2, r2, r1
  lsls r1, r0, #5
  adds r1, r1, r0
  adds r1, r1, r2
  lsls r1, r1, #3
  adds r1, r1, r3
  movs r2, #20
  ldrsh r0, [r1, r2]
  bx lr
  .short 0
_080256D4: .word 0x0805FCAC

.type sub_080256D8, %function
sub_080256D8:
_080256D8:
  ldr r3, _080256F0
  lsls r2, r1, #1
  adds r2, r2, r1
  lsls r1, r0, #5
  adds r1, r1, r0
  adds r1, r1, r2
  lsls r1, r1, #3
  adds r1, r1, r3
  movs r2, #0
  ldrsh r0, [r1, r2]
  bx lr
  .short 0
_080256F0: .word 0x0805FCAC

.type sub_080256F4, %function
sub_080256F4:
_080256F4:
  ldr r3, _0802570C
  lsls r2, r1, #1
  adds r2, r2, r1
  lsls r1, r0, #5
  adds r1, r1, r0
  adds r1, r1, r2
  lsls r1, r1, #3
  adds r1, r1, r3
  movs r2, #4
  ldrsh r0, [r1, r2]
  bx lr
  .short 0
_0802570C: .word 0x0805FCAC

.type sub_08025710, %function
sub_08025710:
_08025710:
  ldr r3, _08025728
  lsls r2, r1, #1
  adds r2, r2, r1
  lsls r1, r0, #5
  adds r1, r1, r0
  adds r1, r1, r2
  lsls r1, r1, #3
  adds r1, r1, r3
  movs r2, #2
  ldrsh r0, [r1, r2]
  bx lr
  .short 0
_08025728: .word 0x0805FCAC

.type sub_0802572C, %function
sub_0802572C:
_0802572C:
  push {r4, lr}
  ldr r4, _0802574C
  bl 0x080256D8
  lsls r0, r0, #16
  asrs r0, r0, #16
  lsls r1, r0, #1
  adds r1, r1, r0
  lsls r1, r1, #2
  adds r1, r1, r4
  movs r2, #0
  ldrsh r0, [r1, r2]
  pop {r4}
  pop {r1}
  bx r1
  .short 0
_0802574C: .word 0x08060124

.type sub_08025750, %function
sub_08025750:
_08025750:
  push {r4, lr}
  adds r4, r1, #0
  adds r3, r2, #0
  cmp r0, #0
  bne.n _08025770
  ldr r2, _0802576C
  lsls r1, r3, #1
  adds r1, r1, r3
  lsls r0, r4, #5
  adds r0, r0, r4
  adds r0, r0, r1
  lsls r0, r0, #3
  adds r2, #12
  b.n _08025780
_0802576C: .word 0x0805FCAC
_08025770:
  ldr r2, _0802578C
  lsls r1, r3, #1
  adds r1, r1, r3
  lsls r0, r4, #5
  adds r0, r0, r4
  adds r0, r0, r1
  lsls r0, r0, #3
  adds r2, #16
_08025780:
  adds r0, r0, r2
  ldr r0, [r0, #0]
  pop {r4}
  pop {r1}
  bx r1
  .short 0
_0802578C: .word 0x0805FCAC

.type sub_08025790, %function
sub_08025790:
_08025790:
  push {r4, lr}
  adds r4, r1, #0
  adds r3, r2, #0
  cmp r0, #0
  bne.n _080257B4
  ldr r2, _080257B0
  lsls r1, r3, #1
  adds r1, r1, r3
  lsls r0, r4, #5
  adds r0, r0, r4
  adds r0, r0, r1
  lsls r0, r0, #3
  adds r0, r0, r2
  movs r1, #6
  ldrsh r0, [r0, r1]
  b.n _080257C8
_080257B0: .word 0x0805FCAC
_080257B4:
  ldr r2, _080257D0
  lsls r1, r3, #1
  adds r1, r1, r3
  lsls r0, r4, #5
  adds r0, r0, r4
  adds r0, r0, r1
  lsls r0, r0, #3
  adds r0, r0, r2
  movs r1, #8
  ldrsh r0, [r0, r1]
_080257C8:
  pop {r4}
  pop {r1}
  bx r1
  .short 0
_080257D0: .word 0x0805FCAC

.type sub_080257D4, %function
sub_080257D4:
_080257D4:
  ldr r3, _080257EC
  lsls r2, r1, #1
  adds r2, r2, r1
  lsls r1, r0, #5
  adds r1, r1, r0
  adds r1, r1, r2
  lsls r1, r1, #3
  adds r1, r1, r3
  movs r2, #20
  ldrsh r0, [r1, r2]
  bx lr
  .short 0
_080257EC: .word 0x0805FCAC

.type sub_080257F0, %function
sub_080257F0:
_080257F0:
  ldr r3, _08025808
  lsls r2, r1, #1
  adds r2, r2, r1
  lsls r1, r0, #5
  adds r1, r1, r0
  adds r1, r1, r2
  lsls r1, r1, #3
  adds r1, r1, r3
  movs r2, #22
  ldrsh r0, [r1, r2]
  bx lr
  .short 0
_08025808: .word 0x0805FCAC

.type sub_0802580C, %function
sub_0802580C:
_0802580C:
  ldr r1, _08025818
  lsls r0, r0, #1
  adds r0, r0, r1
  movs r1, #0
  ldrsh r0, [r0, r1]
  bx lr
_08025818: .word 0x080600CC
ai_line_more_end:
