@ GT Advance 3 - AI lineup tail accessors
@ Region: file offset 0x02581C-0x0258A8 (VMA 0x0802581C-0x080258A8).
@ Pure Thumb; lookup helpers over the lineup tables.

.thumb
.type sub_0802581C, %function
sub_0802581C:
_0802581C:
  push {r4, lr}
  adds r3, r0, #0
  movs r1, #0
  ldr r2, _08025834
_08025824:
  movs r4, #0
  ldrsh r0, [r2, r4]
  cmp r3, r0
  bne.n _08025838
  lsls r0, r1, #16
  asrs r0, r0, #16
  b.n _08025842
  .short 0
_08025834: .word 0x080600CC
_08025838:
  adds r2, #2
  adds r1, #1
  cmp r1, #31
  ble.n _08025824
  movs r0, #0
_08025842:
  pop {r4}
  pop {r1}
  bx r1

.type sub_08025848, %function
sub_08025848:
_08025848:
  movs r0, #32
  bx lr

.type sub_0802584C, %function
sub_0802584C:
_0802584C:
  ldr r1, _08025858
  lsls r0, r0, #3
  adds r0, r0, r1
  movs r1, #0
  ldrsh r0, [r0, r1]
  bx lr
_08025858: .word 0x0806010C

.type sub_0802585C, %function
sub_0802585C:
_0802585C:
  ldr r1, _08025868
  lsls r0, r0, #3
  adds r0, r0, r1
  movs r1, #2
  ldrsh r0, [r0, r1]
  bx lr
_08025868: .word 0x0806010C

.type sub_0802586C, %function
sub_0802586C:
_0802586C:
  ldr r1, _08025878
  lsls r0, r0, #3
  adds r0, r0, r1
  movs r1, #4
  ldrsh r0, [r0, r1]
  bx lr
_08025878: .word 0x0806010C

.type sub_0802587C, %function
sub_0802587C:
_0802587C:
  push {r4, lr}
  adds r3, r0, #0
  movs r1, #0
  ldr r2, _08025894
_08025884:
  movs r4, #0
  ldrsh r0, [r2, r4]
  cmp r3, r0
  bne.n _08025898
  lsls r0, r1, #16
  asrs r0, r0, #16
  b.n _080258A2
  .short 0
_08025894: .word 0x0806010C
_08025898:
  adds r2, #8
  adds r1, #1
  cmp r1, #2
  ble.n _08025884
  movs r0, #0
_080258A2:
  pop {r4}
  pop {r1}
  bx r1

@ Region end 0x0258A8. The _08002587C span runs to here exactly, and this
@ file has no.include, so the marker bounds it truthfully.
ai_line_tail_end:
