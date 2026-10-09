@ GT Advance 3 - packed collection-grid update
@ Region: file offset 0x025DBC-0x025E1C (VMA 0x08025DBC-0x08025E1C).
@ Exact Thumb function with two literal pools.

.thumb
.type sub_08025DBC, %function
sub_08025DBC:
_08025DBC:
  push {r4, r5, r6, lr}
  sub sp, #4
  adds r6, r0, #0
  adds r4, r1, #0
  ldr r1, _08025E14
  mov r0, sp
  movs r2, #4
  bl 0x0802E0A4
  movs r0, #3
  ands r4, r0
  adds r0, r6, #0
  movs r1, #4
  bl 0x0802D97C
  adds r2, r0, #0
  ldr r0, _08025E18
  adds r1, r6, #0
  cmp r6, #0
  bge.n _08025DE6
  adds r1, r6, #3
_08025DE6:
  asrs r1, r1, #2
  adds r0, #24
  adds r5, r1, r0
  mov r1, sp
  adds r0, r1, r2
  ldrb r1, [r5, #0]
  ldrb r0, [r0, #0]
  bics r1, r0
  adds r0, r1, #0
  strb r0, [r5, #0]
  adds r0, r6, #0
  movs r1, #4
  bl 0x0802D97C
  lsls r0, r0, #1
  lsls r4, r0
  ldrb r0, [r5, #0]
  orrs r4, r0
  strb r4, [r5, #0]
  add sp, #4
  pop {r4, r5, r6}
  pop {r0}
  bx r0
_08025E14: .word 0x08060D48
_08025E18: .word 0x03001780

@ Byte-neutral end anchor for the splicer. This file declares exactly one
@ `@ Region:` (0x025DBC-0x025E1C) and has no `.include`; the body at
@ 0x08025DBC runs to 0x025E1C, exactly that boundary, so the anchor is safe.
@ Without it the body is refused as 'no end marker in ai_grid_more.s'.
ai_grid_more_end:
