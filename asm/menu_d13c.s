@ GT Advance 3 - menu window renderer and event dispatcher
@ Region: file offset 0x00D13C-0x00D1B4 (VMA 0x0800D13C-0x0800D1B4).
@ Pure Thumb; renderer plus event dispatcher and private literal pool.

.thumb
.type sub_0800D13C, %function
sub_0800D13C:
_0800D13C:
  push {r4, lr}
  adds r4, r0, #0
  ldr r2, _0800D178
  movs r0, #96
  movs r1, #15
  bl 0x080038A4
  ldr r0, [r4, #28]
  movs r1, #120
  movs r2, #100
  bl 0x08026A20
  movs r0, #6
  ldrsh r1, [r4, r0]
  movs r0, #40
  bl 0x08003ADC
  movs r0, #8
  ldrsh r1, [r4, r0]
  movs r0, #50
  bl 0x08003ADC
  movs r0, #60
  movs r1, #97
  bl 0x08003ADC
  pop {r4}
  pop {r0}
  bx r0
  .short 0x0000
_0800D178: .word 0x0805F89C

.type sub_0800D17C, %function
sub_0800D17C:
_0800D17C:
  push {lr}
  cmp r0, #6
  beq.n _0800D198
  cmp r0, #6
  bhi.n _0800D18C
  cmp r0, #1
  beq.n _0800D1A8
  b.n _0800D1AE
_0800D18C:
  cmp r0, #7
  bne.n _0800D1AE
  adds r0, r3, #0
  bl 0x0800D13C
  b.n _0800D1AE
_0800D198:
  lsls r1, r1, #16
  lsrs r1, r1, #16
  lsls r2, r2, #16
  lsrs r2, r2, #16
  adds r0, r3, #0
  bl 0x0800D048
  b.n _0800D1AE
_0800D1A8:
  adds r0, r3, #0
  bl 0x0800CFE4
_0800D1AE:
  pop {r0}
  bx r0
  .short 0x0000
@ Region-final synthetic end label (cf. boot_end:, idle_end:, code_57d0_end:).
@ Byte-neutral: emits no instruction. The region header fixes the span at
@ 0x0800D13C-0x0800D1B4 and sub_0800D17C is the last.type in this file, so the
@ manifest (which has no cross-file end-marker precedent) needs a same-file one.
menu_d13c_end:
