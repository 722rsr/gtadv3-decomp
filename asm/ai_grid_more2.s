@ GT Advance 3 - packed collection-grid read
@ Region: file offset 0x025E1C-0x025E70 (VMA 0x08025E1C-0x08025E70).
@ Exact Thumb leaf with two literal pools.

.thumb
.type sub_08025E1C, %function
sub_08025E1C:
_08025E1C:
  push {r4, r5, lr}
  sub sp, #4
  adds r4, r0, #0
  ldr r1, _08025E68
  mov r0, sp
  movs r2, #4
  bl 0x0802E0A4
  adds r0, r4, #0
  movs r1, #4
  bl 0x0802D97C
  adds r5, r0, #0
  adds r0, r4, #0
  movs r1, #4
  bl 0x0802D97C
  adds r2, r0, #0
  ldr r1, _08025E6C
  cmp r4, #0
  bge.n _08025E48
  adds r4, #3
_08025E48:
  asrs r0, r4, #2
  adds r1, #24
  adds r0, r0, r1
  mov r3, sp
  adds r1, r3, r5
  ldrb r0, [r0, #0]
  ldrb r1, [r1, #0]
  ands r0, r1
  lsls r1, r2, #1
  asrs r0, r1
  lsls r0, r0, #24
  lsrs r0, r0, #24
  add sp, #4
  pop {r4, r5}
  pop {r1}
  bx r1
_08025E68: .word 0x08060D48
_08025E6C: .word 0x03001780

@ End-of-region anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x08025e1c-0x08025e70), has no `.include`, and ends at
@ 0x08025e70 -- the same address the promoted body at 0x08025e1c ends on
@ (0x08025e70). The boundary is therefore unambiguous and the anchor is safe.
@ Without it promotion_screen refuses the body with "no end marker in ai_grid_more2.s".
ai_grid_more2_end:
