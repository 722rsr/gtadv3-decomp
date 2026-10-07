@ GT Advance 3 - menu initialization and garage grid renderer
@ Region: file offset 0x00D280-0x00D3A4 (VMA 0x0800D280-0x0800D3A4).
@ Pure Thumb; four handlers with private literal pools.

.thumb
.type sub_0800D280, %function
sub_0800D280:
_0800D280:
  ldr r2, _0800D294
  ldrb r0, [r2, #0]
  cmp r0, #0
  beq.n _0800D292
  movs r0, #0
  strb r0, [r2, #0]
  adds r1, #84
  movs r0, #2
  strh r0, [r1, #0]
_0800D292:
  bx lr
_0800D294: .word 0x0203EE64

.type sub_0800D298, %function
sub_0800D298:
_0800D298:
  push {lr}
  ldr r0, _0800D2A8
  bl 0x08002124
  bl 0x0800254C
  pop {r0}
  bx r0
_0800D2A8: .word 0x00001398

.type sub_0800D2AC, %function
sub_0800D2AC:
_0800D2AC:
  bx lr
  .short 0x0000

.type sub_0800D2B0, %function
sub_0800D2B0:
_0800D2B0:
  push {r4, r5, r6, lr}
  adds r6, r0, #0
  ldr r1, _0800D384
  movs r0, #4
  bl 0x08003940
  ldr r4, _0800D388
  bl 0x08002044
  adds r3, r0, #0
  movs r0, #40
  movs r1, #20
  adds r2, r4, #0
  bl 0x08003F18
  ldr r4, _0800D38C
  bl 0x08001F8C
  adds r3, r0, #0
  movs r0, #40
  movs r1, #30
  adds r2, r4, #0
  bl 0x08003F18
  ldr r4, _0800D390
  bl 0x08002140
  adds r3, r0, #0
  movs r0, #40
  movs r1, #40
  adds r2, r4, #0
  bl 0x08003F18
  ldr r4, _0800D394
  movs r0, #0
  bl 0x08001E14
  adds r3, r0, #0
  movs r0, #40
  movs r1, #50
  adds r2, r4, #0
  bl 0x08003F18
  movs r5, #0
_0800D308:
  movs r0, #50
  adds r4, r5, #0
  muls r4, r0
  adds r4, #50
  adds r0, r5, #0
  bl 0x0800206C
  adds r2, r0, #0
  lsls r2, r2, #16
  lsrs r2, r2, #16
  adds r0, r4, #0
  movs r1, #60
  bl 0x08003978
  adds r0, r5, #0
  movs r1, #0
  bl 0x08001E5C
  adds r2, r0, #0
  lsls r2, r2, #16
  lsrs r2, r2, #16
  adds r0, r4, #0
  movs r1, #70
  bl 0x08003978
  adds r0, r5, #0
  movs r1, #0
  bl 0x08002178
  adds r2, r0, #0
  lsls r2, r2, #16
  lsrs r2, r2, #16
  adds r0, r4, #0
  movs r1, #80
  bl 0x08003978
  adds r5, #1
  cmp r5, #3
  ble.n _0800D308
  ldr r2, _0800D398
  movs r0, #40
  movs r1, #130
  bl 0x080038A4
  ldr r2, _0800D39C
  movs r0, #40
  movs r1, #140
  bl 0x080038A4
  ldrh r0, [r6, #6]
  lsls r1, r0, #2
  adds r1, r1, r0
  lsls r1, r1, #1
  adds r1, #130
  ldr r2, _0800D3A0
  movs r0, #30
  bl 0x080038A4
  pop {r4, r5, r6}
  pop {r0}
  bx r0
  .short 0x0000
_0800D384: .word 0x0805F8B0
_0800D388: .word 0x0805F8C0
_0800D38C: .word 0x0805F8D0
_0800D390: .word 0x0805F8E0
_0800D394: .word 0x0805F8F0
_0800D398: .word 0x0805F8F4
_0800D39C: .word 0x0805F8FC
_0800D3A0: .word 0x0805F904
